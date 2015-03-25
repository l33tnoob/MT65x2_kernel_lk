#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <string.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <arch/ops.h>
#include <target.h>
#include <platform.h>
#include <platform/mt_reg_base.h>
#include <platform/boot_mode.h>
#include <platform/mtk_wdt.h>
#include <platform/mt_rtc.h>
#include <platform/bootimg.h>
#include <mt_partition.h>
#include <platform/mtk_nand.h>

/*For image write*/
#include "sparse_format.h"
#include "download_commands.h"
#include "sparse_state_machine.h"
#include <platform/mmc_core.h>
#include <platform/mt_gpt.h>

#include "fastboot.h"


#define MODULE_NAME "FASTBOOT_DOWNLOAD"
#define MAX_RSP_SIZE 64


extern void *download_base;
extern unsigned download_max;
extern unsigned download_size;
extern unsigned fastboot_state;

/*LXO: !Download related command*/

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
#define INVALID_PTN -1
/*#define DBG_LV SPEW*/
#define DBG_LV 0
#define DBG_DCACHE 2
//For test: Display info on boot screen
#define DISPLAY_INFO_ON_LCM

extern unsigned int memory_size();
extern void video_printf (const char *fmt, ...);
extern int video_get_rows(void);
extern void video_set_cursor(int row, int col);
extern void video_clean_screen(void);
extern int nand_write_img(u32 addr, void *data, u32 img_sz,u32 partition_size,bool partition_type);
extern u32 gpt4_tick2time_ms (u32 tick);

unsigned start_time_ms;
#define TIME_STAMP gpt4_tick2time_ms(gpt4_get_current_tick())
#define TIME_START {start_time_ms = gpt4_tick2time_ms(gpt4_get_current_tick());}
#define TIME_ELAPSE (gpt4_tick2time_ms(gpt4_get_current_tick()) - start_time_ms)

extern int usb_write(void *buf, unsigned len);
extern int usb_read(void *buf, unsigned len);
static void init_display_xy();
static void display_info(const char* msg);
static void display_progress(const char* msg_prefix, unsigned size, unsigned totle_size);
static void display_speed_info(const char* msg_prefix, unsigned size);
static void fastboot_fail_wrapper(const char* msg);
static void fastboot_ok_wrapper(const char* msg, unsigned size);
static unsigned hex2unsigned(const char *x);



#define BOOT_LIKE_IMAGE_SIZE 	18*1024*1024  //assume that is 18M, now is about 5M.
#define CACHE_PADDING_SIZE 	BLK_SIZE

u32 IMAGE_TRUNK_SIZE  = (16*1024*1024);    //16M default
void set_image_trunk_size(u32 size)
{
	IMAGE_TRUNK_SIZE = size;
}

#define CACHE_PAGE_SIZE 		(IMAGE_TRUNK_SIZE)
#define DCACHE_SIZE 			(2*CACHE_PAGE_SIZE+2*CACHE_PADDING_SIZE)
#define MEMORY_SIZE_REQ 		(DCACHE_SIZE>BOOT_LIKE_IMAGE_SIZE ? DCACHE_SIZE+BOOT_LIKE_IMAGE_SIZE : 2*BOOT_LIKE_IMAGE_SIZE)
#define BOOT_IMG_ADDR_OFFSET 	(DCACHE_SIZE>BOOT_LIKE_IMAGE_SIZE ? DCACHE_SIZE : BOOT_LIKE_IMAGE_SIZE)
/*dual cache pattern:     | PADDING1 | CACHE1 |PADDING2 | CACHE2 |+ boot image temporal */

#define SIGNAL_RESCHEDULE 0  //true; active. 0 passive.

#if defined(MTK_EMMC_SUPPORT)  //use another macro.
#define EMMC_TYPE
#endif

#ifdef MTK_SECURITY_SW_SUPPORT    
extern BOOL sec_is_signature_file(u8* data, u32 length);
extern BOOL sec_lib_security_check(u8** pdata, u32* plength,  u64 image_offset, int dev_type, const char *arg);
extern unsigned int sec_error();
extern u64 sec_lib_security_get_img_total_size();
#endif

typedef struct cache
{
	u8* padding_buf;
	u8* cache_buf;
	u32 padding_length;  //sparse image boundary problem.
	u32 content_length;  //if this is 0, indicate this the last package.
	event_t content_available;
	event_t cache_available;
	u32 check_sum;   //reserved.
}cache_t;

typedef struct boot_like_image_info
{
	u8* boot_like_image_address;
	BOOL is_boot_like_image;
	u32 offset;
}boot_like_image_info_t;

typedef struct engine_context
{
	cache_t  dual_cache[2];
	u32 flipIdxR;     //receive buffer shift
	u32 flipIdxW;    //write buffer shift
	event_t thr_end_ev; // recieve thread exit sync.
	u32 b_error ; // something is wrong, should exit.
	boot_like_image_info_t boot_like_info;
}engine_context_t;

static engine_context_t ctx;

void cmd_install_sig(const char *arg, void *data, unsigned sz)
{
    fastboot_fail_wrapper("Signature command not supported");
}

void init_engine_context(engine_context_t* tx)
{
	u8* cache_base = (u8*)download_base;  // use absolute address.

	tx->dual_cache[0].padding_buf = cache_base;
	tx->dual_cache[0].cache_buf = cache_base+CACHE_PADDING_SIZE;
	tx->dual_cache[1].padding_buf = cache_base+CACHE_PADDING_SIZE+CACHE_PAGE_SIZE;
	tx->dual_cache[1].cache_buf = cache_base+CACHE_PADDING_SIZE+CACHE_PAGE_SIZE+CACHE_PADDING_SIZE;
	tx->boot_like_info.boot_like_image_address = cache_base+BOOT_IMG_ADDR_OFFSET;

	tx->dual_cache[0].padding_length = tx->dual_cache[1].padding_length = 0;

	event_init(&tx->dual_cache[0].content_available, 0, EVENT_FLAG_AUTOUNSIGNAL);//no data in cache
	event_init(&tx->dual_cache[1].content_available, 0, EVENT_FLAG_AUTOUNSIGNAL);//no data in cache

	event_init(&tx->dual_cache[0].cache_available, 1, EVENT_FLAG_AUTOUNSIGNAL);    //can receive from usb
	event_init(&tx->dual_cache[1].cache_available, 1, EVENT_FLAG_AUTOUNSIGNAL);    //can receive from usb

	event_init(&tx->thr_end_ev, 0, EVENT_FLAG_AUTOUNSIGNAL);//do not end.
	tx->b_error = 0;
	tx->flipIdxR = tx->flipIdxW = 0;
	tx->boot_like_info.is_boot_like_image = FALSE;
	tx->boot_like_info.offset = 0;

}

inline u32 cache_shift(u32 pre)
{
	return pre ^ 0x01;
}



typedef struct storage_info
{
#if defined(EMMC_TYPE)
	int is_sparse_image;
	unsparse_status_t unsparse_status;
#endif
	int first_run;
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    u32 part_id;
#endif
	u64 image_base_addr;
	u64 bulk_image_offset;
	u32 to_write_data_len;
	u8 partition_name[32];
	BOOL checksum_enabled;
}storage_info_t;

static storage_info_t sto_info;
void init_sto_info(storage_info_t* s, BOOL checksum_enabled)
{
#if defined(EMMC_TYPE)
	s->is_sparse_image = 0;
	mmc_init_unsparse_status(&(s->unsparse_status), 0, checksum_enabled);
#endif
	s->first_run = 1;
 	s->image_base_addr = 0;
	s->bulk_image_offset = 0;
	s->to_write_data_len = 0;
	memset(s->partition_name, 0, 32);
	s->checksum_enabled = checksum_enabled;
}

void abort_engine(engine_context_t* tx)
{
	tx->b_error = 1;

	event_signal(&tx->dual_cache[0].cache_available, SIGNAL_RESCHEDULE);
	event_signal(&tx->dual_cache[1].cache_available, SIGNAL_RESCHEDULE);
	event_signal(&tx->dual_cache[0].content_available, SIGNAL_RESCHEDULE);
	event_signal(&tx->dual_cache[1].content_available, SIGNAL_RESCHEDULE);
	event_signal(&tx->thr_end_ev, SIGNAL_RESCHEDULE);
}

void destroy_engine(engine_context_t* tx)
{
	event_destroy(&tx->dual_cache[0].cache_available);
	event_destroy(&tx->dual_cache[1].cache_available);
	event_destroy(&tx->dual_cache[0].content_available);
	event_destroy(&tx->dual_cache[1].content_available);
	event_destroy(&tx->thr_end_ev);
}

BOOL is_sparse_image(u8* data, u32 length)
{
	sparse_header_t *sparse_header;
	//u32* magic_number = (u32*) data;

	sparse_header = (sparse_header_t *) data;

	dprintf(DBG_LV, "is_sparse_image(), data:0x%x,, n sparse_header->magic = 0x%x\n",*(int*)data, sparse_header->magic );

	return (sparse_header->magic == SPARSE_HEADER_MAGIC) ? TRUE : FALSE;
}
int get_mountpoint_from_image(void *data, unsigned sz, char *mountpoint,char *i_type);
//interface
int get_partition_name(u8* data, u32 length, u8* buf)
{

#ifdef MTK_SECURITY_SW_SUPPORT    
	if(sec_is_signature_file(data, length))
	{
		strcpy((char*)buf, (char*)"signatureFile");
		return 0;
	}
#endif

	return get_mountpoint_from_image((void*)data, length, (char*)buf,(char*)NULL);
}

#if defined(EMMC_TYPE)
BOOL write_to_emmc(u8* data, u32 length)
{
	u64 paritition_size = 0;
	u64 size_wrote = 0;
	int next_flip = 0;
	u32 index;
	u32 pre_chksum = 0;
	u32 post_chksum = 0;
	int r;

	while(sto_info.first_run)
	{
		r = get_partition_name(data, length, sto_info.partition_name);
		if(r < 0)
		{
			display_info("\nget_partition_name() Fail");
			return FALSE;
		}

		if((!strncmp((char*)sto_info.partition_name, (char*)"signatureFile", 16))
			|| (!strncmp((char*)sto_info.partition_name, (char*)"boot", 8)))
		{
			//this do not need subsequent codes for normal partition.
			ctx.boot_like_info.is_boot_like_image = TRUE;
			ctx.boot_like_info.offset = 0;
			sto_info.first_run = 0;
			break;
		}

		index = partition_get_index((char*)sto_info.partition_name);
		if(index == (u32)(-1))
		{
			display_info("\nBrick phone??");
			return FALSE;
		}

		if(!is_support_flash(index))
		{
			display_info((char*)sto_info.partition_name);
			display_info("\nDont support partition");
			return FALSE;
		}

		paritition_size = partition_get_size(index);
		dprintf(DBG_LV, "[index:%d]-[downSize:%d]\n", index,  sto_info.to_write_data_len);

		if (ROUND_TO_PAGE(sto_info.to_write_data_len,511) > paritition_size)
		{
			display_info("\nsize too large, space small.");
			dprintf(DBG_LV, "size too large, space small.");
			return FALSE;
		}

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
		sto_info.part_id = partition_get_region(index);
		sto_info.unsparse_status.part_id = sto_info.part_id;
#endif
		sto_info.image_base_addr = partition_get_offset(index);
		sto_info.unsparse_status.image_base_addr = sto_info.image_base_addr;
		sto_info.is_sparse_image = is_sparse_image(data, length);
		sto_info.first_run = 0;
	}

	//boot like image do not need write to image at this function. it is in flash function.
	if(ctx.boot_like_info.is_boot_like_image)
	{
		dprintf(DBG_LV, "boot like img: len: %d\n", length);
		dprintf(DBG_LV, "data: %08X\n", (u32)data);
		//dprintf(DBG_LV, "ctx.boot_like_info.boot_like_image_address: %08X, ctx.boot_like_info.offset %u, \n", ctx.boot_like_info.boot_like_image_address , ctx.boot_like_info.offset);

		memcpy(ctx.boot_like_info.boot_like_image_address + ctx.boot_like_info.offset, data, length);
		ctx.boot_like_info.offset += length;
		return TRUE;
	}

	if(sto_info.is_sparse_image)
	{
		next_flip = cache_shift(ctx.flipIdxR);

		sto_info.unsparse_status.buf = data;
		sto_info.unsparse_status.size = length;
		mmc_write_sparse_data(&sto_info.unsparse_status);

		if(sto_info.unsparse_status.handle_status == S_DA_SDMMC_SPARSE_INCOMPLETE)
		{
			ctx.dual_cache[next_flip].padding_length = sto_info.unsparse_status.size;
			memcpy(ctx.dual_cache[next_flip].padding_buf +(CACHE_PADDING_SIZE-sto_info.unsparse_status.size)
				, sto_info.unsparse_status.buf
				, sto_info.unsparse_status.size);
		}
		else if (sto_info.unsparse_status.handle_status== S_DONE)
		{
			ctx.dual_cache[next_flip].padding_length = 0;
		}
		else
		{
			//some error
			dprintf(DBG_LV, "write_to_emmc() Failed. handle_status(%d)\n", sto_info.unsparse_status.handle_status);
			display_info("\nError in write sparse image in EMMC.");
			return FALSE;
		}
	}
	else
	{
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
		size_wrote = emmc_write(sto_info.part_id, sto_info.image_base_addr+sto_info.bulk_image_offset , (void*)data, length);
#else
		size_wrote = emmc_write(sto_info.image_base_addr+sto_info.bulk_image_offset , (void*)data, length);
#endif
		if (size_wrote  != length)
		{
			dprintf(DBG_LV, "write_to_emmc() Failed. act(%d) != want(%d)\n", (u32)size_wrote, length);
			display_info("\nError in write bulk in EMMC.");
			return FALSE;
		}
		if(sto_info.checksum_enabled)
		{
			pre_chksum = calc_checksum(data, (u32)length);
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
			if(length != emmc_read(sto_info.part_id, sto_info.image_base_addr+sto_info.bulk_image_offset, data,  length))
#else
			if(length != emmc_read(sto_info.image_base_addr+sto_info.bulk_image_offset, data,  length))
#endif
			{
				dprintf(DBG_LV, "emmc_read() Failed.\n");
				display_info("\nError in Read bulk EMMC.");
				return FALSE;
			}

			post_chksum = calc_checksum(data, (u32)length);

			if(post_chksum != pre_chksum)
			{
				dprintf(DBG_LV, "write_to_emmc() Failed. checksum error\n");
				display_info("\nWrite bulk in EMMC. Checksum Error");
				return FALSE;
			}
		}

		sto_info.bulk_image_offset += size_wrote;
	}
	return TRUE;
}

#else


BOOL write_to_nand(u8* data, u32 length, u32 img_total_len)
{
	static u64 partition_size = 0;
	int next_flip = 0;
	u32 index;
	static int img_type = UNKOWN_IMG;
	s8* p_type;
	u32 w_length =0;
	//u32 pre_chksum = 0;
	//u32 post_chksum = 0;
	int r;

	if(sto_info.first_run)
	{
		r = get_partition_name(data, length, sto_info.partition_name);
		if(r < 0)
		{
			display_info("\nGet_partition_name() Fail");
			return FALSE;
		}

		index = partition_get_index(sto_info.partition_name);  //
		if(index == -1)
		{
			display_info("\nBrick phone??");
			return FALSE;
		}

		if(!is_support_flash(index))
		{
			display_info("\nDont support partition.");
			return FALSE;
		}

		partition_size = partition_get_size(index);
		dprintf(DBG_LV, "[index:%d]-[partitionSize:%lld]-[downSize:%d]\n", index, partition_size, sto_info.to_write_data_len);

		if (ROUND_TO_PAGE(sto_info.to_write_data_len,511) > partition_size)
		{
			display_info("size too large, space small.");
			dprintf(DBG_LV, "size too large, space small.");
			return FALSE;
		}

		{
			char i_type[20] = {0};
			get_image_type(data,length,(char *)i_type);
					
			partition_get_type(index,&p_type);
			
			if(strcmp(i_type,p_type)){
					display_info("[warning]image type is not match with partition type\n");
					dprintf(DBG_LV, "[warning]image type'%s' is not match with partition type'%s'",i_type,p_type);
			}
			printf("image type %s\n",i_type);

			if(!strcmp(i_type,"raw data")){
				img_type = RAW_DATA_IMG;	
			}else if(!strcmp(i_type,"yaffs2")){
				img_type = YFFS2_IMG;	
			}else if(!strcmp(i_type,"ubifs")){
				img_type = UBIFS_IMG;	
			}else{
					dprintf(DBG_LV, "image type '%s' unkown\n",i_type);
					display_info("\nimage type unkown");
					return FALSE;
			}
		}		
		sto_info.image_base_addr = partition_get_offset(index);

		//NAND has no sparse image.
		//sto_info.unsparse_status.image_base_addr = sto_info.image_base_addr;
		//sto_info.is_sparse_image = is_sparse_image(data, length);
		sto_info.first_run = 0;
	}

	if (0 != nand_write_img_ex((u32)(sto_info.image_base_addr+sto_info.bulk_image_offset), (void*)data, length,img_total_len?(img_total_len):(u32)(sto_info.to_write_data_len), &w_length, (u32)(sto_info.image_base_addr),(u32)partition_size, img_type))
	{
		dprintf(DBG_LV, "nand_write_img() Failed.\n");
		display_info("Error in write bulk in NAND.");
		return FALSE;
	}
	if(sto_info.checksum_enabled)
	{
		//NAND do not support read() now.
	}

	sto_info.bulk_image_offset += w_length;

	return TRUE;
}

#endif
inline BOOL write_data(u8* data, u32 length, u32 img_total_len)
{
	if(data == 0 ||length==0)
	{
		return TRUE;
	}
#if defined(EMMC_TYPE)
	return write_to_emmc(data, length);
#else
	// Open NAND support
	printf("write_to_nand OPEN\n");
	return write_to_nand(data, length, img_total_len);
#endif

}

//interface
BOOL security_check(/*IN OUT*/u8** pdata, /*IN OUT*/ u32* plength, /*IN*/ u64 image_offset, const char *arg)
{

#if defined(EMMC_TYPE)
    int dev_type = 1;   //EMMC
#else
    int dev_type = 0;   //NAND
    //return FALSE;//Open NAND
#endif

    return sec_lib_security_check(pdata, plength, image_offset, dev_type, arg);
}

int write_storage_proc(void *arg)
{
	u8* data = 0;
	u32 data_len = 0;
	char msg[128];
	u64 image_offset = 0;
    u32 round = 0;

	//dprintf(DBG_DCACHE, "   --[%d] Enter write_storage_proc \n", TIME_STAMP);
	for (;;)
	{
		dprintf(DBG_DCACHE, "   --[%d]Wait ID:%d  cache to read, \n", TIME_STAMP, ctx.flipIdxR);
		event_wait(&(ctx.dual_cache[ctx.flipIdxR].content_available));
		dprintf(DBG_DCACHE, "   --[%d]Obtain ID:%d  cache to read, \n", TIME_STAMP, ctx.flipIdxR);
		if(ctx.b_error)
		{
			sprintf(msg, "\nError USB?\n");
			goto error;
		}
		//if has something to write
		if(ctx.dual_cache[ctx.flipIdxR].content_length !=0  || ctx.dual_cache[ctx.flipIdxR].padding_length !=0)
		{
			data = (u8*)(ctx.dual_cache[ctx.flipIdxR].cache_buf);
			data_len = ctx.dual_cache[ctx.flipIdxR].content_length;

		#ifdef MTK_SECURITY_SW_SUPPORT    
			if(!security_check(&data, &data_len, image_offset, NULL))
			{
				//security error.
				sprintf(msg, "\nSecurity deny - Err:0x%x \n", sec_error());
				goto error;
			}
		#endif
			
			image_offset += ctx.dual_cache[ctx.flipIdxR].content_length;

			data -=  ctx.dual_cache[ctx.flipIdxR].padding_length;
			data_len += ctx.dual_cache[ctx.flipIdxR].padding_length;
			//if data_len==0, secure img tail met.

			dprintf(DBG_DCACHE, "   --[%d]Write ID:%d  to EMMC \n", TIME_STAMP, ctx.flipIdxR);
        #ifdef MTK_SECURITY_SW_SUPPORT    			
			if(!write_data(data, data_len, sec_lib_security_get_img_total_size()))
	    #else
            if(!write_data(data, data_len, 0))
	    #endif
			{
				//error
				sprintf(msg, "\nWrite data error. \n");
				goto error;
			}
		}

		//last package, should return;
		if (ctx.dual_cache[ctx.flipIdxR].content_length == 0)
		{
			dprintf(DBG_DCACHE, "  --[%d]Write EMMC Fin\n", TIME_STAMP);

			data_len = 0;
			
		#ifdef MTK_SECURITY_SW_SUPPORT    
            security_check(&data, &data_len, image_offset, NULL); //notify security check that is the end.
        #endif

			if(ctx.boot_like_info.is_boot_like_image)
			{
				//boot image need download_size to flash.
				download_size = sto_info.to_write_data_len;
				//cache using is over, so copy boot image to download_base
				memcpy(download_base, ctx.boot_like_info.boot_like_image_address, download_size);
			}

			event_signal(&ctx.dual_cache[0].cache_available, SIGNAL_RESCHEDULE);//prevent from dead lock.
			event_signal(&ctx.dual_cache[1].cache_available, SIGNAL_RESCHEDULE);
			event_signal(&ctx.thr_end_ev, SIGNAL_RESCHEDULE);
			return 0;
		}

        round++;
		dprintf(DBG_DCACHE, "   --[%d]Notify ID:%d cache writeable\n", TIME_STAMP, ctx.flipIdxR);
		event_signal(&ctx.dual_cache[ctx.flipIdxR].cache_available, SIGNAL_RESCHEDULE); //make this cache writeable again.

		ctx.flipIdxR = cache_shift(ctx.flipIdxR); //change next buffer.
	}
	return 0;
error:
	dprintf(DBG_LV, msg);
	display_info(msg);
	abort_engine(&ctx);
	return (-1);
}



BOOL read_usb_proc(void *arg)
{
	char msg[128];
	u32 bytes_already_read = 0;
	u32 data_length = *((u32*)arg);
	u32 bytes_read = 0;
	int bytes_read_once = 0;
	u32 bytes_left = 0;

	dprintf(DBG_DCACHE, "++[%d]Enter read_usb_proc\n", TIME_STAMP);

	while (bytes_already_read  < data_length)
	{
		dprintf(DBG_DCACHE, "++[%d]Wait ID:%d  cache to write\n", TIME_STAMP, ctx.flipIdxW);
		event_wait(&(ctx.dual_cache[ctx.flipIdxW].cache_available));
		dprintf(DBG_DCACHE, "++[%d]Obtain ID:%d  cache to write, \n", TIME_STAMP, ctx.flipIdxW);
		if(ctx.b_error)
		{
			sprintf(msg, "\nError Write?\n");
			goto error;
		}

		bytes_read = 0;
		bytes_left = data_length - bytes_already_read;
		bytes_left = bytes_left >= CACHE_PAGE_SIZE ? CACHE_PAGE_SIZE : bytes_left;

		dprintf(DBG_DCACHE, "++[%d]READ USB to ID:%d\n", TIME_STAMP, ctx.flipIdxW);
		while(bytes_left > 0)
		{
			bytes_read_once = usb_read(ctx.dual_cache[ctx.flipIdxW].cache_buf + bytes_read,  bytes_left);

			if (bytes_read_once < 0)
			{
				abort_engine(&ctx);
				dprintf(DBG_LV, "Read USB error.\n");
				display_info("\nRead USB error\n");
				fastboot_state = STATE_ERROR;
				return FALSE;
			}
			bytes_left -= bytes_read_once;
			bytes_read += bytes_read_once;
		}

		ctx.dual_cache[ctx.flipIdxW].content_length = bytes_read;
		bytes_already_read += bytes_read;

		dprintf(DBG_DCACHE, "++[%d]Notify ID:%d cache readable\n", TIME_STAMP, ctx.flipIdxW);
		event_signal(&ctx.dual_cache[ctx.flipIdxW].content_available, SIGNAL_RESCHEDULE);

		ctx.flipIdxW = cache_shift(ctx.flipIdxW); //change next buffer.

		display_progress("\rTransfer Data", bytes_already_read, data_length);
	}

	if(bytes_already_read  != data_length)
	{
		dprintf(DBG_LV, "ASSERT error.  bytes_already_read  != data_length\n");
		//cause assert.
		*((int*)0x00) = 0;
	}

	dprintf(DBG_DCACHE, "++[%d]USB read Fin\n", TIME_STAMP);
	//last package.
         //must wait for this can write again.
	event_wait(&(ctx.dual_cache[ctx.flipIdxW].cache_available));
	ctx.dual_cache[ctx.flipIdxW].content_length = 0;
	event_signal(&ctx.dual_cache[ctx.flipIdxW].content_available, SIGNAL_RESCHEDULE);

	return TRUE;
error:
	dprintf(DBG_LV, msg);
	display_info(msg);
	abort_engine(&ctx);
	return FALSE;
}

int is_use_ex_download(u32 length)
{
#if defined(ORIGN_NORMAL_DOWNLOAD)
	dprintf(DBG_LV, "ORIGN_NORMAL_DOWNLOAD defined use normal download\n");
	if(TRUE)return 0;   //remove warnings.
#endif
	if(length < CACHE_PAGE_SIZE)
	{
		return 0;
	}
#if defined(EMMC_TYPE)
	return 1;
#else
	return 1; //Open NAND support
#endif
}

int download_ex(u32 data_length)//Big image and parallel transfer.
{
	thread_t *thr;

	init_engine_context(&ctx);
	init_sto_info(&sto_info, FALSE);  //no checksum enabled.
	sto_info.to_write_data_len = data_length;

	thr = thread_create("fastboot", write_storage_proc, 0, DEFAULT_PRIORITY, 16*1024);
	if (!thr)
	{
		return -1;
	}
	thread_resume(thr);

	TIME_START;

	read_usb_proc(&data_length);

	//wait for write thread end.
	event_wait(&ctx.thr_end_ev);

	destroy_engine(&ctx);

	if(ctx.b_error)
	{
		fastboot_fail_wrapper("\n@DOWNLOAD ERROR@\nPlease re-plug your USB cable\n");
		fastboot_state = STATE_ERROR;
	}else
	{
		fastboot_okay("");
	}
	return 0;
}

int download_standard(u32 data_length)
{
	int r;
	display_info("USB Transferring... ");
	TIME_START;
	r = usb_read(download_base, data_length);

	if ((r < 0) || ((unsigned) r != data_length))
	{
		dprintf(DBG_LV, "Read USB error.\n");
		display_speed_info("Read USB error", data_length);
        		fastboot_fail_wrapper("Read USB error");
		fastboot_state = STATE_ERROR;
		return -1;
	}
	download_size = data_length;

	dprintf(DBG_LV, "read OK.  data:0x%x\n", *((int*)download_base));
	fastboot_ok_wrapper("USB Transmission OK", data_length);
	return 0;
}

int __check_mountpoint(void *data, int count, char *mountpoint,char *i_type);

int get_mountpoint_from_image(void *data, unsigned sz, char *mountpoint,char *i_type)
{
	sparse_header_t *sparse_header;
	chunk_header_t *chunk_header;
	unsigned int chunk_data_sz = 0;

	if(sz < CHECK_SIZE){
		printf("[get_mountpoint_from_image]sz must be more than CHECK SIZE(%dB)\n",CHECK_SIZE);
		return -1;
	}
	sparse_header = (sparse_header_t *) data;
	printf("[get_mountpoint_from_image]data %p, mageic %x\n",data,sparse_header->magic);
	if (sparse_header->magic == SPARSE_HEADER_MAGIC)
	{
			/* Read and skip over sparse image header */
			data += sizeof(sparse_header_t);
			printf("[get_mountpoint_from_image]sparse header size %d, file_hdr_sz: %d\n",sizeof(sparse_header_t), sparse_header->file_hdr_sz);
			if(sparse_header->file_hdr_sz > sizeof(sparse_header_t))
			{
				/* Skip the remaining bytes in a header that is longer than
				* we expected.
				*/
				data += (sparse_header->file_hdr_sz - sizeof(sparse_header_t));
			}


			/* Read and skip over chunk header */
			chunk_header = (chunk_header_t *) data;
			data += sizeof(chunk_header_t);

			if(sparse_header->chunk_hdr_sz > sizeof(chunk_header_t))
			{
				/* Skip the remaining bytes in a header that is longer than
				* we expected.
				*/
				data += (sparse_header->chunk_hdr_sz - sizeof(chunk_header_t));
			}

			chunk_data_sz = sparse_header->blk_sz * chunk_header->chunk_sz;

			if(chunk_header->chunk_type == CHUNK_TYPE_RAW || chunk_data_sz >= 4*1024){
				if(chunk_header->total_sz != (sparse_header->chunk_hdr_sz + chunk_data_sz))
				{
					fastboot_fail_wrapper("Bogus chunk size for chunk type Raw");
					return -1;
				}
				return __check_mountpoint((void*)data,CHECK_SIZE,mountpoint,i_type);
			}else{
				printf("[get_mountpoint_from_image]the first chunk is empty or chunk size %d is littler than 4KB\n",chunk_data_sz);
				return -1;
			}
	}
	else
	{
			return __check_mountpoint((void*)data,CHECK_SIZE,mountpoint,i_type);
	}
}

int __check_mountpoint(void *data, int count, char *mountpoint,char *i_type)
{
	struct ext4_super_block *sb;
	char buf[64] = {0};
#ifndef MTK_EMMC_SUPPORT
	part_dev_t *dev =  mt_part_get_device();
	struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
	int page_size,spare_size;
#endif
	if (0 == memcmp((void *)data, BOOT_MAGIC, strlen(BOOT_MAGIC))){
		if(mountpoint != NULL)
			strcpy(mountpoint,"boot");
		if(i_type != NULL)
			strcpy(i_type,"raw data");
		return 0;
	}
#ifdef MTK_EMMC_SUPPORT
	if((unsigned int)count < sizeof(struct ext4_super_block)){
		printf("[__check_mountpoint] %d littler than ext4 super block size\n",count);
	  return -1;
	}

	data += 1024;

	sb = (struct ext4_super_block *)data;

	if(sb->s_magic != EXT4_SUPER_MAGIC){
		printf("[__check_mountpoint]ext4 magic num error, %x data %p\n",sb->s_magic,data);
		return -1;
	}
	memcpy(buf,sb->s_last_mounted,sizeof(sb->s_last_mounted));
	if(strncmp(buf,"/",1)){
		printf("[__check_mountpoint]mountpoint  %s is not support\n",buf);
		return -1;
	}
	printf("[__check_mountpoint]mountpoint is %s\n",buf);
	if(i_type != NULL)
			strcpy(i_type,"ext4");
	if(mountpoint != NULL){
	if(!strcmp(&buf[1],"data")){
			strcpy(mountpoint,"userdata");
	}else{
		memcpy(mountpoint,&buf[1],strlen(buf));
	}
	}
#else
	page_size = nand->page_size;
	if(page_size == 2048){
		spare_size = 64;
	}else if(page_size == 4096){
		spare_size = 128;
	}else{
		printf("[__check_mountpoint]page size %d is not support\n",page_size);
		return -1;
	}
//check if ubi image first
{
	
	char *tp = data;
	struct ubi_ec_hdr *ec_hdr = (struct ubi_ec_hdr *)tp;
	struct ubi_vid_hdr *vid_hdr = NULL;
	struct ubi_vtbl_record *vtbl = NULL;
	int name_len = 0;
	int leb_start = 0;
	/*1. check first page for ec hdr*/
	if(be32_to_cpu(ec_hdr->magic) == UBI_EC_HDR_MAGIC){
		printf("[__check_mountpoint]find UBI EC HDR\n");
	/*2. check 2nd page for vid hdr*/
		tp += be32_to_cpu(ec_hdr->vid_hdr_offset);
		leb_start = be32_to_cpu(ec_hdr->data_offset);
		vid_hdr = (struct ubi_vid_hdr *)tp;
		if(be32_to_cpu(vid_hdr->magic) == UBI_VID_HDR_MAGIC){
				printf("[__check_mountpoint]find UBI VID HDR\n");
	/*3. check 3rd page for layout volume*/
				if(be32_to_cpu(vid_hdr->vol_id) == UBI_LAYOUT_VOLUME_ID){
						printf("[__check_mountpoint]find UBI LAYOUT VOLUME\n");
						tp = data + leb_start;
						vtbl = (struct ubi_vtbl_record *)tp;
						name_len = be16_to_cpu(vtbl->name_len);
						printf("[__check_mountpoint]volume name len %d\n",name_len);
						if(name_len <= UBI_VOL_NAME_MAX){
								memcpy(buf,vtbl->name,name_len);
								if(i_type != NULL)
											strcpy(i_type,"ubifs");
								goto find;
						}else{
								printf("[__check_mountpoint]volume name len more than %d\n",UBI_VOL_NAME_MAX);

						}
				}else{
						printf("[__check_mountpoint]can't find UBI LAYOUT VOLUME\n");

				}
		}else{
				printf("[__check_mountpoint]can't find UBI VID HDR, %x\n",ec_hdr->magic);
		}
			
	}else{
		printf("[__check_mountpoint]can not find UBI EC HDR, check another fs image\n");
	}

}
//check if ubi image 
	data += (page_size+spare_size);

	if(strcmp((char *)(data-YAFFS_TAG_OFFSET),"yaffs2")){
		printf("[__check_mountpoint]yaffs tag can not find in %d\n",page_size+spare_size-YAFFS_TAG_OFFSET);
		return -1;
	}
	if(i_type != NULL)
			strcpy(i_type,"yaffs2");	
	memcpy(buf,data-PARTITION_INFO_OFFSET,PARTITION_INFO_OFFSET-YAFFS_TAG_OFFSET);
find:
	printf("[__check_mountpoint]mountpoint is %s\n",buf);
	if(mountpoint != NULL){
	if(!strcmp(buf,"data")){
		strcpy(mountpoint,"userdata");
	}else{
		memcpy(mountpoint,buf,strlen(buf)+1);
	}
	}
#endif
	return 0;
}

void get_image_type(void *data,unsigned sz,char *i_type)
{
	int r = 0;
	r = get_mountpoint_from_image(data, sz, NULL,i_type);
	if(r<0)
		strcpy(i_type,"raw data");
	
	return;
}

void cmd_download(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	unsigned len = hex2unsigned(arg);
	u32 available_memory=0;
	//int r;

	init_display_xy();
	download_size = 0;
	//available_memory = memory_size()-(u32)download_base;
	// Real code should be: available_memory = memory_size()-((u32)download_base - MEMBASE);
	// download_base - MEMBASE is maximum of nearly 64M, that is more smaller than real RAM size like 1G, so use whole memory for approximation.
	available_memory = memory_size();	

	dprintf(DBG_LV, "Enter cmd_download Data Length:%d, available_memory:%d\n", len, available_memory);

	if (len > download_max)
	{
		dprintf(DBG_LV, "Data is larger than all partitions size in target.\n");
		fastboot_fail_wrapper("Data is larger than all partitions size in target");
		return;
	}

	if(is_use_ex_download(len))
	{
		if(available_memory < MEMORY_SIZE_REQ)
		{
			dprintf(DBG_LV, "Insufficient memory for DCACHE\n");
			fastboot_fail_wrapper("Insufficient memory for DCACHE");
			return;
		}
	}
	else
	{
		if (len > available_memory)
		{
			dprintf(DBG_LV, "Insufficient memory for whole image\n");
			fastboot_fail_wrapper("Insufficient memory for whole image");
			return;
		}
	}


	snprintf(response, MAX_RSP_SIZE, "DATA%08x", len);
	if (usb_write(response, strlen(response)) < 0)
	{
	    dprintf(DBG_LV, "cmd_download -- usb write fail\n");
		return;
	}

	if(is_use_ex_download(len))
	{
		//use ex download
		download_ex(len);
	}
	else
	{
		//use normal download
		download_standard(len);
	}

	return;
}


/*PROTECTED

void cmd_download(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	unsigned len = hex2unsigned(arg);
	u32 available_memory=0;
	int r;

	init_display_xy();
	download_size = 0;
	available_memory = memory_size()-(u32)download_base;

	dprintf(DBG_LV, "Enter cmd_download. Length:%d, available_memory:%d\n", len, available_memory);

	if (len > download_max || len > available_memory)
	{
		dprintf(DBG_LV, "data too large.len > download_max.\n");
		fastboot_fail_wrapper("data too large");
		return;
	}

	snprintf(response, MAX_RSP_SIZE, "DATA%08x", len);

	if (usb_write(response, strlen(response)) < 0)
	{
		return;
	}

	display_info("USB Transferring... ");
	TIME_START;
	r = usb_read(download_base, len);

	if ((r < 0) || ((unsigned) r != len))
	{
		dprintf(DBG_LV, "Read USB error.\n");
		display_speed_info("Read USB error", len);
        fastboot_fail_wrapper("Read USB error");
		fastboot_state = STATE_ERROR;
		return;
	}
	download_size = len;

	dprintf(DBG_LV, "read OK.  data:0x%x\n", *((int*)download_base));
	fastboot_ok_wrapper("USB Transmission OK", len);
}
*/

BOOL cmd_flash_emmc_img(const char *arg, void *data, unsigned sz)
{
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    unsigned int part_id;
#endif
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	unsigned long long size_wrote = 0;
	int index = INVALID_PTN;
	u32 pre_chksum = 0;
	u32 post_chksum = 0;
	char msg[256];
	dprintf(DBG_LV, "Function cmd_flash_img()\n");
	//dprintf(DBG_LV, "EMMC Offset[0x%x], Length[0x%x], data In[0x%x]\n", arg, sz, data);
	TIME_START;

	if (!strcmp(arg, "partition"))
	{
		dprintf(DBG_LV, "Attempt to write partition image.(MBR, GPT?)\n");
		dprintf(DBG_LV, "Not supported, return.\n");
		fastboot_fail_wrapper("Not supported 'partition'.\n");
		return FALSE;
		/*if (write_partition(sz, (unsigned char *) data)) {
		fastboot_fail_wrapper("failed to write partition");
		return FALSE;
		}*/
	}
	else
	{
		index = partition_get_index(arg);
		if(index == -1)
		{
			fastboot_fail_wrapper("partition table doesn't exist");
			return FALSE;
		}
		if(!is_support_flash(index)){
			sprintf(msg,"\npartition '%s' not support flash",arg);
			fastboot_fail_wrapper(msg);
			return FALSE;
		}

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        part_id = partition_get_region(index);
#endif
		ptn = partition_get_offset(index);

		//dprintf(DBG_LV, "[arg:%s]-[index:%d]-[ptn(offset):0x%x]\n", arg, index, ptn);

		if (!strcmp(arg, "boot") || !strcmp(arg, "recovery"))
		{
			if (memcmp((void *)data, BOOT_MAGIC, strlen(BOOT_MAGIC)))
			{
				fastboot_fail_wrapper("\nimage is not a boot image");
				return FALSE;
			}
		}

		size = partition_get_size(index);
		//dprintf(DBG_LV, "[index:%d]-[partitionSize:%lld]-[downSize:%lld]\n", index, size, sz);

		if (ROUND_TO_PAGE(sz,511) > size)
		{
			fastboot_fail_wrapper("size too large");
			dprintf(DBG_LV, "size too large");
			return FALSE;
		}

		display_info("\nWriting Flash ... ");

		pre_chksum = calc_checksum(data,  (u32)sz);

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
		size_wrote = emmc_write(part_id, ptn , data, sz);
#else
		size_wrote = emmc_write(ptn , data, sz);
#endif
		if (size_wrote  != sz)
		{
			//dprintf(DBG_LV, "emmc_write() Failed. act(%lld) != want(%lld)\n", size_wrote, sz);
			fastboot_fail_wrapper("\nFlash write failure");
			return FALSE;
		}

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
		if(sz != emmc_read(part_id, ptn,  data,  sz))
#else
		if(sz != emmc_read(ptn,  data,  sz))
#endif
		{
			dprintf(DBG_LV, "emmc_read() Failed.\n");
			fastboot_fail_wrapper("\nRead EMMC error.");
			return FALSE;
		}

		post_chksum = calc_checksum(data, (u32)sz);
		if(post_chksum != pre_chksum)
		{
			dprintf(DBG_LV, "write_to_emmc() Failed. checksum error\n");
			fastboot_fail_wrapper("\nChecksum Error.");
			return FALSE;
		}

		fastboot_ok_wrapper("OK", sz);
	}
    
	return TRUE;
}

BOOL cmd_flash_emmc_sparse_img(const char *arg, void *data, unsigned sz)
{
	unsigned int chunk;
	unsigned int chunk_data_sz;
	sparse_header_t *sparse_header;
	chunk_header_t *chunk_header;
	u32 total_blocks = 0;
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    unsigned int part_id;
#endif
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	unsigned long long size_wrote = 0;
	int index = INVALID_PTN;
	char msg[256];
	dprintf(DBG_LV, "Enter cmd_flash_sparse_img()\n");
	//dprintf(DBG_LV, "EMMC Offset[0x%x], Length[%d], data In[0x%x]\n", arg, sz, data);

	index = partition_get_index(arg);
	if(index == -1)
	{
		fastboot_fail_wrapper("partition table doesn't exist");
		return FALSE;
	}

	if(!is_support_flash(index)){
		sprintf(msg,"partition '%s' not support flash\n",arg);
		fastboot_fail_wrapper(msg);
		return FALSE;
	}

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    part_id = partition_get_region(index);
#endif
	ptn = partition_get_offset(index);

	size = partition_get_size(index);
	if (ROUND_TO_PAGE(sz,511) > size)
	{
		fastboot_fail_wrapper("size too large");
		return FALSE;
	}

	/* Read and skip over sparse image header */
	sparse_header = (sparse_header_t *) data;
	data += sparse_header->file_hdr_sz;
	if(sparse_header->file_hdr_sz > sizeof(sparse_header_t))
	{
		/* Skip the remaining bytes in a header that is longer than
		* we expected.
		*/
		data += (sparse_header->file_hdr_sz - sizeof(sparse_header_t));
	}

	dprintf (DBG_LV, "=== Sparse Image Header ===\n");
	dprintf (DBG_LV, "magic: 0x%x\n", sparse_header->magic);
	dprintf (DBG_LV, "major_version: 0x%x\n", sparse_header->major_version);
	dprintf (DBG_LV, "minor_version: 0x%x\n", sparse_header->minor_version);
	dprintf (DBG_LV, "file_hdr_sz: %d\n", sparse_header->file_hdr_sz);
	dprintf (DBG_LV, "chunk_hdr_sz: %d\n", sparse_header->chunk_hdr_sz);
	dprintf (DBG_LV, "blk_sz: %d\n", sparse_header->blk_sz);
	dprintf (DBG_LV, "total_blks: %d\n", sparse_header->total_blks);
	dprintf (DBG_LV, "total_chunks: %d\n", sparse_header->total_chunks);

	TIME_START;
	display_info("Writing Flash ... ");
	/* Start processing chunks */
	for (chunk=0; chunk<sparse_header->total_chunks; chunk++)
	{
		/* Read and skip over chunk header */
		chunk_header = (chunk_header_t *) data;
		data += sizeof(chunk_header_t);

		dprintf (INFO, "=== Chunk Header ===\n");
		dprintf (INFO, "chunk_type: 0x%x\n", chunk_header->chunk_type);
		dprintf (INFO, "chunk_data_sz: 0x%x\n", chunk_header->chunk_sz);
		dprintf (INFO, "total_size: 0x%x\n", chunk_header->total_sz);

		if(sparse_header->chunk_hdr_sz > sizeof(chunk_header_t))
		{
			/* Skip the remaining bytes in a header that is longer than
			* we expected.
			*/
			data += (sparse_header->chunk_hdr_sz - sizeof(chunk_header_t));
		}

		chunk_data_sz = sparse_header->blk_sz * chunk_header->chunk_sz;
		switch (chunk_header->chunk_type)
		{
		case CHUNK_TYPE_RAW:
			if(chunk_header->total_sz != (sparse_header->chunk_hdr_sz +
				chunk_data_sz))
			{
				fastboot_fail_wrapper("Bogus chunk size for chunk type Raw");
				return FALSE;
			}


			//dprintf(INFO, "[Flash Base Address:0x%llx offset:0x%llx]-[size:%d]-[DRAM Address:0x%x]\n",
			//	ptn , ((uint64_t)total_blocks*sparse_header->blk_sz), chunk_data_sz, data);

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
			size_wrote = emmc_write(part_id, ptn + ((uint64_t)total_blocks*sparse_header->blk_sz),
				(unsigned int*)data, chunk_data_sz);
#else
			size_wrote = emmc_write(ptn + ((uint64_t)total_blocks*sparse_header->blk_sz),
				(unsigned int*)data, chunk_data_sz);
#endif

			dprintf(INFO, "[wrote:%lld]-[size:%d]\n", size_wrote ,chunk_data_sz);

			if(size_wrote != chunk_data_sz)
			{
				fastboot_fail_wrapper("flash write failure");
				return FALSE;
			}
			total_blocks += chunk_header->chunk_sz;
			data += chunk_data_sz;
			break;

		case CHUNK_TYPE_DONT_CARE:
			total_blocks += chunk_header->chunk_sz;
			break;

		case CHUNK_TYPE_CRC:
			if(chunk_header->total_sz != sparse_header->chunk_hdr_sz)
			{
				fastboot_fail_wrapper("Bogus chunk size for chunk type Dont Care");
				return FALSE;
			}
			total_blocks += chunk_header->chunk_sz;
			data += chunk_data_sz;
			break;

		default:
			fastboot_fail_wrapper("Unknown chunk type");
			return FALSE;
		}
	}

	dprintf(DBG_LV, "Wrote %d blocks, expected to write %d blocks\n",
		total_blocks, sparse_header->total_blks);

	if(total_blocks != sparse_header->total_blks)
	{
		fastboot_fail_wrapper("sparse image write failure");
        return FALSE;
	}

    fastboot_ok_wrapper("Write Flash OK", sz);
	
	return TRUE;;
}
void cmd_flash_emmc(const char *arg, void *data, unsigned sz)
{
	sparse_header_t *sparse_header;
	/* 8 Byte Magic + 2048 Byte xml + Encrypted Data */
	//unsigned int *magic_number = (unsigned int *) data;
	BOOL write_ret = TRUE;
	char msg[128] = {0};
#if 0
	if (magic_number[0] == DECRYPT_MAGIC_0 &&
		magic_number[1] == DECRYPT_MAGIC_1)
	{
#ifdef SSD_ENABLE
		ret = decrypt_scm((u32 **) &data, &sz);
#endif
		if (ret != 0)
		{
			dprintf(CRITICAL, "ERROR: Invalid secure image\n");
			return;
		}
	}
	else if (magic_number[0] == ENCRYPT_MAGIC_0 &&
		magic_number[1] == ENCRYPT_MAGIC_1)
	{
#ifdef SSD_ENABLE
		ret = encrypt_scm((u32 **) &data, &sz);
#endif
		if (ret != 0)
		{
			dprintf(CRITICAL, "ERROR: Encryption Failure\n");
			return;
		}
	}
#endif
	if(sz  == 0)
	{
		fastboot_okay("");
		return;
	}

#ifdef MTK_SECURITY_SW_SUPPORT    
    //[Security] Please DO NOT get any data for reference if security check is not passed
    if(!security_check((u8**)&data, &sz, 0, arg))
    {
        sprintf(msg, "\nSecurity deny - Err:0x%x \n", sec_error());
    	dprintf(DBG_LV, msg);
    	fastboot_fail_wrapper((char*)msg);
        return;
    }
#endif

	sparse_header = (sparse_header_t *) data;

	dprintf(DBG_LV, "cmd_flash_emmc, data:0x%x,, n sparse_header->magic = 0x%x\n",*(int*)data, sparse_header->magic );

	if (sparse_header->magic != SPARSE_HEADER_MAGIC)
	{
		write_ret = cmd_flash_emmc_img(arg, data, sz);
	}
	else
	{
		write_ret = cmd_flash_emmc_sparse_img(arg, data, sz);
	}

    if( write_ret )
    {
        //[Security] Notify security check that is the end.
        sz = 0;
    #ifdef MTK_SECURITY_SW_SUPPORT    
        security_check((u8**)&data, &sz, IMAGE_TRUNK_SIZE, arg); 
    #endif
    }
}

void cmd_erase_emmc(const char *arg, void *data, unsigned sz)
{
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    unsigned int part_id;
#endif
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	int index = INVALID_PTN;
	int erase_ret = MMC_ERR_NONE;
	char msg[256];

	init_display_xy();
	dprintf (DBG_LV, "Enter cmd_erase()\n");

	index = partition_get_index(arg);
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    part_id = partition_get_region(index);
#endif
	ptn = partition_get_offset(index);

	if(index == -1) {
		fastboot_fail_wrapper("Partition table doesn't exist");
		return;
	}
	if(!is_support_erase(index)){
		sprintf(msg,"partition '%s' not support erase\n",arg);
		fastboot_fail_wrapper(msg);
		return;
	}

	TIME_START;
	size = partition_get_size(index);
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
	erase_ret = emmc_erase(part_id, ptn, partition_get_size(index));
#else
	erase_ret = emmc_erase(ptn, partition_get_size(index));
#endif

	if(erase_ret  == MMC_ERR_NONE)
	{
		dprintf (DBG_LV, "emmc_erase() OK\n");
		fastboot_ok_wrapper("Erase EMMC", size);
	}
	else
	{
		dprintf (DBG_LV, "emmc_erase() Fail\n");
		snprintf(msg, sizeof(msg), "Erase error. code:%d", erase_ret);
		fastboot_fail_wrapper(msg);
	}


	return;
}

BOOL cmd_flash_nand_img(const char *arg, void *data, unsigned sz)
{
	int index;
	u64 offset,size;
	int img_type;
	char *p_type;
	char msg[256];

    index = partition_get_index(arg);
    if(index == -1){
        fastboot_fail_wrapper("partition get index fail");
        return FALSE;
    }
    if(!is_support_flash(index)){
        sprintf(msg,"partition '%s' not support flash\n",arg);
        fastboot_fail_wrapper(msg);
        return FALSE;
    }

    offset = partition_get_offset(index);
    if(offset == (u64)(-1)){
        fastboot_fail_wrapper("partition get offset fail");
        return FALSE;
    }else{
        printf("get offset: 0x%llx\n",offset);
    }
    size = partition_get_size(index);
    if(size == (u64)(-1)){
        fastboot_fail_wrapper("partition get size fail");
        return FALSE;
    }else{
        printf("get size: 0x%llx\n",size);
    }

    if (!strcmp(arg, "boot") || !strcmp(arg, "recovery"))
    {
        if (memcmp((void *)data, BOOT_MAGIC, strlen(BOOT_MAGIC)))
        {
            fastboot_fail_wrapper("image is not a boot image");
            return FALSE;
        }
    }
    {
			char i_type[20] = {0};
			
			get_image_type(data,sz,(char *)i_type);
					
			partition_get_type(index,&p_type);
			
			if(strcmp(i_type,p_type)){
					display_info("[warning]image type is not match with partition type\n");
					dprintf(DBG_LV, "[warning]image type'%s' is not match with partition type'%s'",i_type,p_type);
			}
			if(!strcmp(i_type,"raw data")){
				img_type = RAW_DATA_IMG;	
			}else if(!strcmp(i_type,"yaffs2")){
				img_type = YFFS2_IMG;	
			}else if(!strcmp(i_type,"ubifs")){
				img_type = UBIFS_IMG;	
			}else{
					dprintf(DBG_LV, "image type '%s' unkown\n",i_type);
					display_info("\nimage type unkown");
					return FALSE;
			}
		}
    TIME_START;
    display_info("write flash ....");
    printf("writing %d bytes to '%s' img_type %d\n", sz, arg,img_type);
    if (nand_write_img((u32)offset, (char*)data, sz,(u32)size,img_type)) {
        fastboot_fail_wrapper("nand  write image failure");
        return FALSE;
    }
    printf("partition '%s' updated\n", arg);
    fastboot_ok_wrapper("write flash sucess",sz);

    return TRUE;
}


void cmd_flash_nand(const char *arg, void *data, unsigned sz)
{
	char msg[128] = {0};

	if(sz  == 0)
	{
		fastboot_okay("");
		return;
	}
	
#ifdef MTK_SECURITY_SW_SUPPORT    
    //Please DO NOT get any data for reference if security check is not passed
    if(!security_check((u8**)&data, &sz, 0, arg))
    {
        sprintf(msg, "\nSecurity deny - Err:0x%x \n", sec_error());
    	dprintf(DBG_LV, msg);
    	fastboot_fail_wrapper(msg);
        return;
    }
#endif

	dprintf(DBG_LV, "cmd_flash_nand, data:0x%x\n",*(int*)data);

    if(cmd_flash_nand_img(arg,data,sz))
    {
        //[Security] Notify security check that is the end.
        sz = 0;
    #ifdef MTK_SECURITY_SW_SUPPORT    
        security_check((u8**)&data, &sz, IMAGE_TRUNK_SIZE, arg); 
    #endif
    }
}

void cmd_erase_nand(const char *arg, void *data, unsigned sz)
{

	int index;
	u64 offset,size;
	char msg[256];
	index = partition_get_index(arg);
	if(index == -1){
		fastboot_fail_wrapper("partition get index fail");
		return;
	}
	if(!is_support_erase(index)){
		sprintf(msg,"partition '%s' not support erase\n",arg);
		fastboot_fail_wrapper(msg);
		return;
	}

	offset = partition_get_offset(index);
	if(offset == (u64)(-1)){
		fastboot_fail_wrapper("partition get offset fail");
		return;
	}else{
		printf("get offset: 0x%llx\n",offset);
	}
	size = partition_get_size(index);
	if(size == (u64)(-1)){
		fastboot_fail_wrapper("partition get size fail");
		return;
	}else{
		printf("get size: 0x%llx\n",size);
	}

	TIME_START;
	display_info("erase flash ....");

	if(nand_erase(offset,size)!=0){
		fastboot_fail_wrapper("failed to erase partition");
		return;
	}

	fastboot_ok_wrapper("erase flash sucess",sz);

	return;
}


static void init_display_xy()
{
#if defined(DISPLAY_INFO_ON_LCM)
	video_clean_screen();
	video_set_cursor(video_get_rows()/2, 0);
	//video_set_cursor(1, 0);
#endif
}

static void display_info(const char* msg)
{
#if defined(DISPLAY_INFO_ON_LCM)
	if(msg == 0)
	{
		return;
	}
	video_printf("%s\n", msg);
#endif
}

static void display_progress(const char* msg_prefix, unsigned size, unsigned totle_size)
{
#if defined(DISPLAY_INFO_ON_LCM)
	unsigned vel = 0;
	unsigned prog = 0;
	unsigned time = TIME_ELAPSE;

	if(msg_prefix == 0)
	{
		msg_prefix = "Unknown";
	}

	if(time != 0)
	{
		vel = (unsigned)(size / time); //approximate  1024/1000
		time /= 1000;
	}
	if(totle_size != 0)
	{
		prog = (unsigned)((size*100.0f)/totle_size);
	}
	video_printf("%s > %3d%% Time:%4ds Vel:%5dKB/s", msg_prefix, prog, time, vel);
#endif
}

static void display_speed_info(const char* msg_prefix, unsigned size)
{
#if defined(DISPLAY_INFO_ON_LCM)
	unsigned vel = 0;
	unsigned time = TIME_ELAPSE;

	if(msg_prefix == 0)
	{
		msg_prefix = "Unknown";
	}

	if(time != 0)
	{
		vel = (unsigned)(size / time); //approximate  1024/1000
	}
	video_printf("%s  Time:%dms Vel:%dKB/s \n", msg_prefix, time, vel);
#endif
}

static void fastboot_fail_wrapper(const char* msg)
{
	display_info(msg);
	fastboot_fail(msg);
}

static void fastboot_ok_wrapper(const char* msg, unsigned data_size)
{
	display_speed_info(msg, data_size);
	fastboot_okay("");
}

static unsigned hex2unsigned(const char *x)
{
    unsigned n = 0;

    while(*x) {
        switch(*x) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            n = (n << 4) | (*x - '0');
            break;
        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
            n = (n << 4) | (*x - 'a' + 10);
            break;
        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
            n = (n << 4) | (*x - 'A' + 10);
            break;
        default:
            return n;
        }
        x++;
    }

    return n;
}

/*LXO: END!Download related command*/
