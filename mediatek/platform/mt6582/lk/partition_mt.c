#include <mt_partition.h>
#include <stdint.h>
#include <printf.h>
#include <string.h>
#include <platform/errno.h>
#include "pmt.h"
#include <platform/mtk_nand.h>
#include <target.h>
#define PMT 1


//common
//BLK_SIZE is 512, block_size is from flash is 128K
#ifndef MTK_EMMC_SUPPORT
static u32 block_size;
static u32 page_size;
extern flashdev_info devinfo;
#endif
extern pt_resident lastest_part[PART_MAX_COUNT];
extern part_t partition_layout[];

extern int total_size;
extern struct NAND_CMD g_kCMD;
static pt_info pi;

//if used malloc func ,the pdata = (uchar*)malloc(sizeof(uchar)*size);
// in recovery_check_command_trigger will return 0
//static char *page_buf;  

unsigned char page_buf[4096+128];
unsigned char backup_buf[4096];
#ifdef MTK_EMMC_SUPPORT
#define CFG_EMMC_PMT_SIZE 0xc00000
extern int g_user_virt_addr;
extern u64 g_emmc_size;
pt_resident32 lastest_part32[PART_MAX_COUNT];
#endif

#ifdef PMT
void get_part_tab_from_complier(void)
{
#ifdef MTK_EMMC_SUPPORT
	int index=0;
	printf("get_pt_from_complier\n");
	while (partition_layout[index].flags != PART_FLAG_END) {
		memcpy(lastest_part[index].name,partition_layout[index].name,MAX_PARTITION_NAME_LEN);
		lastest_part[index].size = (u64)partition_layout[index].blknum * BLK_SIZE ;
		lastest_part[index].offset = (u64)partition_layout[index].startblk * BLK_SIZE;
		if (lastest_part[index].size == 0) {
			lastest_part[index].size = target_get_max_flash_size() - lastest_part[index].offset - partition_reserve_size(); 
		}
		lastest_part[index].mask_flags = partition_layout[index].flags;  //this flag in kernel should be fufilled even though in flash is 0.
		printf("get_ptr  %s %016llx %016llx\n",lastest_part[index].name,lastest_part[index].offset,lastest_part[index].size);
		index++;
	}
#else
	int index=0;
	printf("get_pt_from_complier \n");
	while(partition_layout[index].flags!= PART_FLAG_END)
	{
    		
		memcpy(lastest_part[index].name,partition_layout[index].name,MAX_PARTITION_NAME_LEN);
		lastest_part[index].size = partition_layout[index].blknum*BLK_SIZE ;
		lastest_part[index].offset = partition_layout[index].startblk * BLK_SIZE;
		if(lastest_part[index].size == 0){
			lastest_part[index].size = total_size - lastest_part[index].offset;		
		}
		lastest_part[index].mask_flags =  partition_layout[index].flags;  //this flag in kernel should be fufilled even though in flash is 0.
		printf ("get_ptr  %s %lx %lx\n",lastest_part[index].name,lastest_part[index].offset,lastest_part[index].size);
		index++;
	}
#endif
}

#ifndef MTK_EMMC_SUPPORT
bool find_mirror_pt_from_bottom(int *start_addr,part_dev_t *dev)
{
	int mpt_locate;
	int mpt_start_addr;
	int current_start_addr=0;
	char pmt_spare[4];
	mpt_start_addr = total_size+block_size;
	//mpt_start_addr=MPT_LOCATION*block_size-page_size;
	for(mpt_locate=(block_size/page_size);mpt_locate>0;mpt_locate--)
	{
		memset(pmt_spare,0xFF,PT_SIG_SIZE);
		
		current_start_addr = mpt_start_addr+mpt_locate*page_size;
		if(!dev->read(dev,current_start_addr, page_buf,page_size))
		{
			printf ("find_mirror read  failed %x %x \n",current_start_addr,mpt_locate);
		}
		memcpy(&page_buf[page_size],g_kCMD.au1OOB,16);
		memcpy(pmt_spare,&page_buf[page_size] ,PT_SIG_SIZE);
		//need enhance must be the larget sequnce number
		
		if(is_valid_mpt(page_buf)&&is_valid_mpt(&pmt_spare))
		{
		      //if no pt, pt.has space is 0;
			pi.sequencenumber = page_buf[PT_SIG_SIZE+page_size];
			printf ("find_mirror find valid pt at %x sq %x \n",current_start_addr,pi.sequencenumber);
			break;
		}
		else
		{
			continue;
		}
	}
	if(mpt_locate==0)
	{
		printf ("no valid mirror page\n");
		pi.sequencenumber =  0;
		return FALSE;
	}
	else
	{
		*start_addr = current_start_addr;
		return TRUE;
	}
}
#endif

#ifdef MTK_EMMC_SUPPORT

#define PMT_REGION_SIZE     (0x1000)
#define PMT_REGION_OFFSET   (0x100000)

#define PMT_VER_V1          ("1.0")
#define PMT_VER_SIZE        (4)

static int load_pt_from_fixed_addr(u8 *buf, part_dev_t *dev)
{
    int reval = ERR_NO_EXIST;
    u64 pt_start; 
    u64 mpt_start; 
    int pt_size = PMT_REGION_SIZE; 
    int buffer_size = pt_size;
    //unsigned char *head = &page_buf[0];
    unsigned char *head = page_buf;
    unsigned char *tail = page_buf + pt_size - PT_SIG_SIZE;

    pt_start = g_emmc_size - PMT_REGION_OFFSET;
    mpt_start = pt_start + PMT_REGION_SIZE;

    printf("============func=%s===scan pmt from %llx=====\n", __func__, pt_start);
    /* try to find the pmt at fixed address, signature:0x50547631 */

    dev->read(dev, pt_start, (u8*)page_buf, buffer_size); 
    if (is_valid_pt(head)) {
        if (!memcmp(page_buf + PT_SIG_SIZE, PMT_VER_V1, PMT_VER_SIZE)) {
            if (is_valid_pt(tail)) {
                printf("find pt at %llx\n", pt_start); 
                memcpy(buf, page_buf + PT_SIG_SIZE + PMT_VER_SIZE, PART_MAX_COUNT * sizeof(pt_resident));
                reval = DM_ERR_OK;
                return reval;
            } else {
                printf("invalid tail pt format\n");
                reval = ERR_NO_EXIST;
            }
        } else {
            printf("invalid pt version %s\n", page_buf + PT_SIG_SIZE);
            reval = ERR_NO_EXIST;
        }
    }

    
    dev->read(dev, mpt_start, (u8*)page_buf, buffer_size); 
    if (is_valid_mpt(head)) { 
        if (!memcmp(page_buf + PT_SIG_SIZE, PMT_VER_V1, PMT_VER_SIZE)) {
            if (is_valid_mpt(tail)) {
                printf("find mpt at %llx\n", mpt_start); 
                memcpy(buf, page_buf + PT_SIG_SIZE + PMT_VER_SIZE, PART_MAX_COUNT * sizeof(pt_resident));
                reval = DM_ERR_OK;
                return reval;
            } else {
                printf("invalid tail mpt format\n");
                reval = ERR_NO_EXIST;
            }
        } else {
            printf("invalid mpt version %s\n", page_buf + PT_SIG_SIZE);
            reval = ERR_NO_EXIST;
        }
    }

	return reval;      
}
#endif
int load_exist_part_tab(u8 *buf,part_dev_t *dev)
{
#ifndef MTK_EMMC_SUPPORT
	int pt_start_addr;
	int pt_cur_addr;
	int pt_locate;
	int reval=DM_ERR_OK;
	int mirror_address;
	char pmt_spare[PT_SIG_SIZE];

	block_size= devinfo.blocksize*1024;
	page_size = devinfo.pagesize;
	
	//page_buf = malloc(page_size);	 

	pt_start_addr = total_size;
	printf("load_pt from 0x%x \n",pt_start_addr);
	//pt_start_addr=PT_LOCATION*block_size;
	for(pt_locate=0;pt_locate<(block_size/page_size);pt_locate++)
	{
		pt_cur_addr = pt_start_addr+pt_locate*page_size;
		memset(pmt_spare,0xFF,PT_SIG_SIZE);

		if(!dev->read(dev,pt_cur_addr, page_buf,page_size))
		{
			printf ("load_pt read pt failded: %x\n",pt_cur_addr);
		}
          	 memcpy(&page_buf[page_size],g_kCMD.au1OOB,16);

		memcpy(pmt_spare,&page_buf[page_size] ,PT_SIG_SIZE); //skip bad block flag
		if(is_valid_pt(page_buf)&&is_valid_pt(pmt_spare))
		{
			pi.sequencenumber = page_buf[PT_SIG_SIZE+page_size];
			printf("load_pt find valid pt at %x sq %x \n",pt_start_addr,pi.sequencenumber);
			break;
		}
		else
		{
			continue;
		}
	}
	//for test 
	//pt_locate==(block_size/page_size);
	if(pt_locate==(block_size/page_size))
	{
		//first download or download is not compelte after erase or can not download last time
		printf ("load_pt find pt failed \n");
		pi.pt_has_space = 0; //or before download pt power lost
		
		if(!find_mirror_pt_from_bottom(&mirror_address,dev))
		{
			printf ("First time download \n");
			reval=ERR_NO_EXIST;
			return reval;
		}
		else
		{
			//used the last valid mirror pt, at lease one is valid.
			dev->read(dev,mirror_address, page_buf,page_size);
		}
	}
	memcpy(buf,&page_buf[PT_SIG_SIZE],sizeof(lastest_part));

	return reval;
#else
    return ERR_NO_EXIST;
#endif
}
void part_init_pmt(unsigned long totalblks,part_dev_t *dev)
{
#ifdef MTK_EMMC_SUPPORT
    part_t *part = &partition_layout[0];
	unsigned long lastblk;
	int retval=0;
	int i=0;
	printf ("mt6577_part_init_pmt \n");
	if (!totalblks) return;

	/* updater the number of blks of first part. */
	if (totalblks <= part->blknum)
	    part->blknum = totalblks;

	totalblks -= part->blknum;
	lastblk = part->startblk + part->blknum;

	while (totalblks) {
		part++;
		if (!part->name)
		    break;

		if (part->flags & PART_FLAG_LEFT || totalblks <= part->blknum)
		    part->blknum = totalblks;

		part->startblk = lastblk;
		totalblks -= part->blknum;
		lastblk = part->startblk + part->blknum;
	}
	
	memset(&pi,0xFF,sizeof(pi));
	memset(&lastest_part, 0, PART_MAX_COUNT * sizeof(pt_resident));
	retval = load_pt_from_fixed_addr((u8 *)&lastest_part, dev);
	if (retval == ERR_NO_EXIST) {
        //first run preloader before dowload 
		//and valid mirror last download or first download 
		printf("no pt \n");
		get_part_tab_from_complier(); //get from complier
	} else {
		printf("Find pt \n");
		for (i = 0; i < PART_MAX_COUNT; i++) {	
			if (lastest_part[i].size == 0) {
				lastest_part[i].size = target_get_max_flash_size() - lastest_part[i].offset - partition_reserve_size(); 
				printf("partition %s size %016llx %016llx \n",lastest_part[i].name,lastest_part[i].offset,lastest_part[i].size);
				break;
			}
			if (!strcmp((char *)lastest_part[i].name, PMT_END_NAME)) {
				lastest_part[i].size = target_get_max_flash_size() - lastest_part[i].offset - partition_reserve_size(); 
				printf("partition %s size %016llx %016llx \n",lastest_part[i].name,lastest_part[i].offset,lastest_part[i].size);
				break;
			}
			printf ("partition %s size %016llx %016llx \n",lastest_part[i].name,lastest_part[i].offset,lastest_part[i].size);
		}
	}
#else
part_t *part = &partition_layout[0];
	unsigned long lastblk;
	int retval=0;
	int i=0;
	printf ("mt6577_part_init_pmt \n");
	if (!totalblks) return;

	/* updater the number of blks of first part. */
	if (totalblks <= part->blknum)
	part->blknum = totalblks;

	totalblks -= part->blknum;
	lastblk = part->startblk + part->blknum;

	while(totalblks) 
	{
		part++;
		if (!part->name)
		break;

		if (part->flags & PART_FLAG_LEFT || totalblks <= part->blknum)
		part->blknum = totalblks;

		part->startblk = lastblk;
		totalblks -= part->blknum;
		lastblk = part->startblk + part->blknum;
	}
	
	memset(&pi,0xFF,sizeof(pi));
	memset(&lastest_part,0,PART_MAX_COUNT*sizeof(pt_resident));
	retval=load_exist_part_tab((u8 *)&lastest_part,dev);
	if (retval==ERR_NO_EXIST) //first run preloader before dowload
	{
		//and valid mirror last download or first download 
		printf ("no pt \n");
		get_part_tab_from_complier(); //get from complier
	}
	else
	{
		printf ("Find pt \n");
		for(i=0;i<PART_MAX_COUNT;i++)
		{	
			if(lastest_part[i].size == 0){
				lastest_part[i].size = total_size - lastest_part[i].offset;	
				printf ("partition %s size %lx %lx \n",lastest_part[i].name,lastest_part[i].offset,lastest_part[i].size);
				break;
			}
			printf ("partition %s size %lx %lx \n",lastest_part[i].name,lastest_part[i].offset,lastest_part[i].size);
		}
	}
#endif
}

#endif
