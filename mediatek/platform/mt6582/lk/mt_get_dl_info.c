#include "mt_partition.h"
#include <printf.h>
#include <malloc.h>
#include <string.h>

typedef struct{
	u32 image_index;
	u32 pc_checksum;
	u32 da_checksum;
	char checksum_status[8];
}CHECKSUM_INFO_EMMC;

typedef struct{
	char magic_num[32];
	CHECKSUM_INFO_EMMC part_info[PART_MAX_COUNT];
	char ram_checksum[16];
	char download_status[16];
}DL_STATUS_EMMC;

#define DL_INFO_SIZE ((u64)2048)
#define DL_NOT_FOUND 2
#define DL_PASS 0
#define DL_FAIL 1
extern u64 g_emmc_size;

int mmc_get_dl_info(void)
{
	
	DL_STATUS_EMMC download_info;
	u64 dl_addr = g_emmc_size - DL_INFO_SIZE;
	part_dev_t *dev = mt_part_get_device();
	int i,ret;
	u8 *dl_buf = malloc(DL_INFO_SIZE);
	printf("get dl info from 0x%llx\n",dl_addr);
	dev->read(dev,dl_addr,(u8 *)dl_buf,DL_INFO_SIZE);
	memcpy(&download_info,dl_buf,sizeof(download_info));
	if(memcmp(download_info.magic_num,"DOWNLOAD INFORMATION!!",22)){
		printf("DL INFO NOT FOUND\n");
		ret = DL_NOT_FOUND;
	}

	if(!memcmp(download_info.download_status,"DL_DONE",7)||!memcmp(download_info.download_status,"DL_CK_DONE",10))
	{
		printf("dl done. status = %s\n",download_info.download_status);	
		printf("dram checksum : %s\n",download_info.ram_checksum);
		for(i=0;i<PART_MAX_COUNT;i++)
		{
			if(download_info.part_info[i].image_index!=0){
				printf("image_index:%d, checksum: %s\n",download_info.part_info[i].image_index,download_info.part_info[i].checksum_status);
			}
		}
		ret = DL_PASS;
	}else
	{
		printf("dl error. status = %s\n",download_info.download_status);
		printf("dram checksum : %s\n",download_info.ram_checksum);
		for(i=0;i<PART_MAX_COUNT;i++)
		{
			if(download_info.part_info[i].image_index!=0){
				printf("image_index:%d, checksum: %s\n",download_info.part_info[i].image_index,download_info.part_info[i].checksum_status);
			}
		}
		ret = DL_FAIL;
	}

	free(dl_buf);
	return ret;
}
