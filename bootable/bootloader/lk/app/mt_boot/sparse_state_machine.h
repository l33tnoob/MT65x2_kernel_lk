#ifndef 	_SPARSE_STATE_MACHINE_H_
#define 	_SPARSE_STATE_MACHINE_H_

typedef enum STATUS_E_t
{
	S_DONE = 0,
	S_DA_SDMMC_WRITE_FAILED,
	S_DA_SDMMC_READ_FAILED,
	S_DA_SDMMC_SPARSE_INCOMPLETE,
	S_DA_SDMMC_CHECKSUM_ERR,
} STATUS_E;

typedef enum unsparse_wait_phase
{
    UNSPARSE_WAIT_SPARSE_HEADER,
    UNSPARSE_WAIT_CHUNK_HEADER,
    UNSPARSE_WAIT_CHUNK_DATA,
} unsparse_phase_t;

typedef struct unsparse_status
{
   STATUS_E handle_status;
   unsparse_phase_t wait_phase;
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
   u32 part_id;
#endif
   u64 image_base_addr;
   u32 size;
   u8 *buf;
   BOOL checksum_enabled;
} unsparse_status_t;

typedef struct unsparse_data
{
     sparse_header_t sparse_hdr;
     chunk_header_t chunk_hdr;
     u64 chunk_remain_data_size;
     u64 image_address_offset;
     u32 unhandle_buf_size;
     u8  unhandle_buf[sizeof(chunk_header_t)];
} unsparse_data_t;


void mmc_init_unsparse_status(unsparse_status_t* status, u64 image_base_addr, BOOL enable_chk);
void mmc_write_sparse_data(unsparse_status_t* status);

inline u32 calc_checksum(u8* data, u32 length)
{
	register u32 chksum = 0;
	u8* ptr = data;
	u32 i = 0;

	for(i=0;i<(length&(~3));i +=sizeof(int))
	{
		chksum += (*(u32*)(data+i));
	}
	if(i < length)//can not aligned by 4
	{
		ptr += i;
		for(i = 0; i<(length&3); i++)/// remain
		{
			chksum += ptr[i];
		}
	}

	return chksum;
}

#endif

