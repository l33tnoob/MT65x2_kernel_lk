#ifndef DRAM_BUFFER_H 
#define DRAM_BUFFER_H

#include "nand_core.h"
#include "platform.h"
#include "cust_part.h"

#include "pmt.h"

#if CFG_TRUSTONIC_TEE_SUPPORT
#include "trustzone.h"
#endif

/*bmt.c*/
//#define BMT_BUFFER_SIZE     0x10000
//#define MAX_MAIN_SIZE                (0x1000) /*nand_core.h*/
//#define MAX_SPAR_SIZE                (0x80) /*nand_core.h*/
//#define BMT_DAT_BUFFER_SIZE         (MAX_MAIN_SIZE + MAX_SPAR_SIZE) /*nand_core.h*/
/*nand.c*/
//#define NFI_BUF_MAX_SIZE             (0x10000)         /*nand_core.h*/
//#define NAND_NFI_BUFFER_SIZE        (NFI_BUF_MAX_SIZE) /*nand_core.h*/


/*download.c*/
#define PART_HDR_BUF_SIZE 512

/*mmc_common_inter.c*/
#define STORAGE_BUFFER_SIZE 0x10000

/*part.c*/
#define IMG_HDR_BUF_SIZE 512

/*partition.c*/
#define EMMC_PMT_BUFFER_SIZE             (0x20000)

/*print.c*/
#define LOG_BUFFER_MAX_SIZE             (0x10000)

/*sec.c*/
#define DRAM_SEC_SECRO_BUFFER_LENGTH     (0x3000)   /*buffer.h*/
#define DRAM_SEC_WORKING_BUFFER_LENGTH   0x2000  
#define DRAM_SEC_UTIL_BUFFER_LENGTH      0x1000   
#define DRAM_SEC_LIB_HEAP_LENGTH         0x8000   
#define DRAM_SEC_IMG_BUFFER_LENGTH       0x400    
#define DRAM_SEC_CHUNK_BUFFER_LENGTH     0x100000 

/******************************************************************************
 * DRAM buffer
 ******************************************************************************/
typedef struct{
	/*bmt.c*/
	//u8 bmt_buf[BMT_BUFFER_SIZE];
	//u8 bmt_dat_buf[BMT_DAT_BUFFER_SIZE];
    /*nand.c*/
	//u8 nand_nfi_buf[NAND_NFI_BUFFER_SIZE];
	
    /*download.c*/
	part_hdr_t part_hdr_buf[PART_HDR_BUF_SIZE];  
    /*mmc_common_inter.c*/
	unsigned char storage_buffer[STORAGE_BUFFER_SIZE];
	/*part.c*/
	u8 img_hdr_buf[IMG_HDR_BUF_SIZE];
	unsigned int part_num;
	part_hdr_t   part_info[PART_MAX_NUM];
	/*partition.c*/
	u8 emmc_pmt_buf[EMMC_PMT_BUFFER_SIZE];
	pt_resident new_part[PART_MAX_COUNT];
    pt_resident lastest_part[PART_MAX_COUNT];
	u8  pmt_dat_buf[PMT_DAT_BUFFER_SIZE];
	u8  pmt_read_buf[PMT_READ_BUFFER_SIZE];
	/*platform.c*/
	boot_arg_t bootarg; 
	/*print.c*/
	u8 log_dram_buf[LOG_BUFFER_MAX_SIZE];
	/*sec.c*/
	u8  sec_secro_buf[DRAM_SEC_SECRO_BUFFER_LENGTH];
	u8  sec_working_buf[DRAM_SEC_WORKING_BUFFER_LENGTH];/*This dram Buffer not used for security concern*/
	u8  sec_util_buf[DRAM_SEC_UTIL_BUFFER_LENGTH];
	u8  sec_lib_heap_buf[DRAM_SEC_LIB_HEAP_LENGTH];
	u8  sec_img_buf[DRAM_SEC_IMG_BUFFER_LENGTH];        /*This dram Buffer not used for security concern*/
	u8  sec_chunk_buf[DRAM_SEC_CHUNK_BUFFER_LENGTH];
	/* tee_init.c */
#if CFG_TRUSTONIC_TEE_SUPPORT
	tee_arg_t  teearg;
#endif
} dram_buf_t;

/******************************************************************************
 * SECURE sram buffer
 ******************************************************************************/

typedef struct{
    u8  sram_sec_working_buf[DRAM_SEC_WORKING_BUFFER_LENGTH];
    u8  sram_sec_img_buf[DRAM_SEC_IMG_BUFFER_LENGTH];
} sec_buf_t;


typedef struct{
    u32 l1_page_table[4096];
}dram_page_table;

void init_dram_buffer();

extern dram_buf_t *g_dram_buf;
extern dram_page_table *g_page_table; 

extern sec_buf_t  g_sec_buf;


#endif /*DRAM_BUFFER_H*/
