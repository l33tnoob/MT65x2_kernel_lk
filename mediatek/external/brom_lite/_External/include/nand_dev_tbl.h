/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2004
*
*******************************************************************************/

/*******************************************************************************
 *
 * Filename:
 * ---------
 *      nand_dev_tbl.h
 *
 * Project:
 * --------
 *    FlashTool Download Agent
 *
 * Description:
 * ------------
 *    NAND flash device table
 *
 * Author:
 * -------
 *      Amos Hsu
 *
 *******************************************************************************/
#ifndef _NAND_DEV_TBL_H_
#define _NAND_DEV_TBL_H_

#include "SW_TYPES.H"
#include "DOWNLOAD.H"

#pragma pack(push) /* push current alignment to stack */
#pragma pack(4) /* set alignment to 4 byte boundary */


#ifdef __cplusplus
extern "C" {
#endif

#define NAND_MAX_SPARE_SIZE_BYTE        64
#define NAND_MAX_OTP_REGION_PER_DIE        4

// DO NOT modify the IO BITS value, because it must be the same as BootROM definition
#define NAND_IO_8BITS                0
#define NAND_IO_16BITS                1

typedef struct {
    uint16        m_maker_code;
    uint16        m_device_code;
    // 4k-todo::
    uint16        m_ext_code_1;
    uint16        m_ext_code_2;
} NAND_ID_S;

typedef struct {
    uint16        m_enable;
    uint16        m_cmd;
} NAND_Command_S;

typedef struct {
    NAND_Command_S            m_read_id;
    NAND_Command_S            m_status;
    NAND_Command_S            m_reset;
    NAND_Command_S            m_read;
    NAND_Command_S            m_read_spare;
    NAND_Command_S            m_read_cnf;
    NAND_Command_S            m_program_1st_half_page;
    NAND_Command_S            m_program;
    NAND_Command_S            m_program_cnf;
    NAND_Command_S            m_erase;
    NAND_Command_S            m_erase_cnf;
} NAND_CommandSet_S;



struct _NAND_DeviceInfo_S;
typedef struct _NAND_DeviceInfo_S  NAND_DeviceInfo_S;
typedef STATUS_E  (*FP_CB_NAND_READ_ID_T)(uint16  *p_maker_code, uint16  *p_device_code, uint16  *p_ext_code1, uint16  *p_ext_code2);
typedef STATUS_E  (*FP_CB_NAND_RESET_T)(void);
typedef STATUS_E  (*FP_CB_NAND_PRE_PROCESS_T)(void);
typedef STATUS_E  (*FP_CB_NAND_READ_STATUS_T)(void);
typedef STATUS_E  (*FP_CB_NAND_BLOCK_ERASE_T)(const uint32  row_addr);
typedef STATUS_E  (*FP_CB_NAND_BAD_BLOCK_SYMBOL_CHECK_T)(const uint32  *p_spare32);
typedef STATUS_E  (*FP_CB_NAND_BAD_BLOCK_SYMBOL_SET_T)(uint32  *p_spare32);
typedef STATUS_E  (*FP_CB_NAND_PAGE_READ_T)(const uint32 row_addr, uint32 *p_data32, uint32  ecc_parity_from_reg[8]);
typedef STATUS_E  (*FP_CB_NAND_PAGE_PROGRAM_T)(const uint32 row_addr, const uint32 *p_data32, uint32  ecc_parity_from_reg[8], const uint32 fdm_format_option);
typedef STATUS_E  (*FP_CB_NAND_SPARE_READ_T)(const uint32 row_addr, uint32 *p_spare32);
typedef STATUS_E  (*FP_CB_NAND_SPARE_PROGRAM_T)(const uint32 row_addr, const uint32 *p_spare32);
typedef STATUS_E  (*FP_CB_NAND_PAGE_SPARE_PROGRAM_T)(uint32 row_addr, const uint32 *p_page_spare);

typedef struct {
    NAND_CMD_Callback_ID_E                    m_cb_gid;
    FP_CB_NAND_READ_ID_T                    m_cb_read_id;
    FP_CB_NAND_RESET_T                        m_cb_reset;
    FP_CB_NAND_PRE_PROCESS_T                m_cb_pre_process;
    FP_CB_NAND_READ_STATUS_T                m_cb_read_status;
    FP_CB_NAND_BLOCK_ERASE_T                m_cb_block_erase;
    FP_CB_NAND_BAD_BLOCK_SYMBOL_CHECK_T        m_cb_bad_block_symbol_check;
    FP_CB_NAND_BAD_BLOCK_SYMBOL_SET_T        m_cb_bad_block_symbol_set;
    FP_CB_NAND_PAGE_READ_T                    m_cb_page_read;
    FP_CB_NAND_PAGE_PROGRAM_T                m_cb_page_program;
    FP_CB_NAND_SPARE_READ_T                    m_cb_spare_read;
    FP_CB_NAND_SPARE_PROGRAM_T                m_cb_spare_program;
    FP_CB_NAND_PAGE_SPARE_PROGRAM_T            m_cb_page_spare_program;
} NAND_CMD_Callback_S;

// OTP

typedef STATUS_E  (*FP_CB_NAND_OTP_ENTER_T)(void);
typedef STATUS_E  (*FP_CB_NAND_OTP_PAGE_READ_T)(const uint32 row_addr, uint32 *p_data32, uint32  ecc_parity_from_reg[8]);
typedef STATUS_E  (*FP_CB_NAND_OTP_PAGE_PROGRAM_T)(const uint32 row_addr, const uint32 *p_data32, uint32  ecc_parity_from_reg[8]);
typedef STATUS_E  (*FP_CB_NAND_OTP_LOCK_T)(void);
typedef STATUS_E  (*FP_CB_NAND_OTP_LOCK_CHECKSTATUS_T)(void);
typedef STATUS_E  (*FP_CB_NAND_OTP_EXIT_T)(void);

typedef struct {
    FP_CB_NAND_OTP_ENTER_T                    m_cb_otp_enter;
    FP_CB_NAND_OTP_PAGE_READ_T                m_cb_otp_page_read;
    FP_CB_NAND_OTP_PAGE_PROGRAM_T            m_cb_otp_page_program;
    FP_CB_NAND_OTP_LOCK_T                    m_cb_otp_lock;
    FP_CB_NAND_OTP_LOCK_CHECKSTATUS_T        m_cb_otp_lock_checkstatus;
    FP_CB_NAND_OTP_EXIT_T                    m_cb_otp_exit;
} NAND_OTP_CMD_Callback_S;

typedef struct {
    // sector layout
    uint32        m_region_base;
    uint32        m_page_count;
    uint32        m_region_offset;
} NAND_SectorRegion_S;

typedef struct {
    // total size (in pages) on this die
    uint32        m_total_pages;

    // grouped sector region map
    NAND_SectorRegion_S    m_region_map[NAND_MAX_OTP_REGION_PER_DIE];

} NAND_OTP_Layout_S;

typedef struct {

    // nand flash H/W manufacture id and device code
    NAND_ID_S    m_id;

    // total size in MB
    uint32        m_total_size_in_mb;
    // pages per block
    uint32        m_pages_per_block;
    // page size in Byte
    uint32        m_page_size;
      // spare size in Byte
    uint32        m_spare_size;
    // I/O interface: 8bits or 16bits
    uint32        m_io_interface;
    // address cycle
    uint32        m_addr_cycle;

} NAND_HW_Info_S;

typedef struct {
    // nand flash id
    NAND_DeviceID_E            m_id;
    // NAND flash device H/W info
    NAND_HW_Info_S                m_hw_info;

    // command set
    uint32 m_cmd_set; //const NAND_CommandSet_S*
    // callback function set
    uint32 m_cb_func_set; //const NAND_CMD_Callback_S *

    //OTP layout
    uint32 m_otp_layout; //const NAND_OTP_Layout_S*
    // OTP callback function set
    uint32 m_otp_cb_func_set; //const NAND_OTP_CMD_Callback_S*
} NAND_Device_S;

struct _NAND_DeviceInfo_S {

    // total page count
    uint32                m_total_pages;
    // total block count
    uint32                m_total_blocks;
    // block size in Byte
    uint32                m_block_size;
    // spare size in Byte
    uint32                m_spare_size;
    // page addr shift bits
    uint32                m_page_addr_shift_bits;
    // block addr shift bits
    uint32                m_block_addr_shift_bits;

    // NAND flash H/W info
    uint32 m_dev; //const NAND_Device_S   
};

typedef struct {
    uint8    m_data[512];
    uint8    m_spare[16];
} NAND_Page512_8_S;

typedef struct {
    uint16    m_data[256];
    uint16    m_spare[8];
} NAND_Page512_16_S;

typedef struct {
    uint32    m_data[128];
    uint32    m_spare[4];
} NAND_Page512_32_S;

typedef struct {
    uint8    m_data[2048];
    uint8    m_spare[64];
} NAND_Page2048_8_S;

typedef struct {
    uint16    m_data[1024];
    uint16    m_spare[32];
} NAND_Page2048_16_S;

typedef struct {
    uint32    m_data[512];
    uint32    m_spare[16];
} NAND_Page2048_32_S;

typedef union {
    uint8                m_raw8[512+16];
    uint16                m_raw16[256+8];
    uint32                m_raw32[128+4];

    // NFIv1, v2, v3, v4
    NAND_Page512_8_S    m_pagespare8;
    NAND_Page512_16_S    m_pagespare16;
    NAND_Page512_32_S    m_pagespare32;
} NAND_Page512_U;

typedef union {
    uint8                m_raw8[2048+64];
    uint16                m_raw16[1024+32];
    uint32                m_raw32[512+16];

    // PAGE: NFIv1, v2
    NAND_Page2048_8_S    m_pagespare8;
    NAND_Page2048_16_S    m_pagespare16;
    NAND_Page2048_32_S    m_pagespare32;

    // SECTOR: NFIv3, v4
    NAND_Page512_8_S    m_sectorspare8[4];
    NAND_Page512_16_S    m_sectorspare16[4];
    NAND_Page512_32_S    m_sectorspare32[4];
} NAND_Page2048_U;

typedef union {
    NAND_Page512_U        m_512;
    NAND_Page2048_U        m_2048;
} NAND_PageBuffer_U;

typedef struct {
    NAND_PageBuffer_U    m_page[64];
    bool                m_page_dirty[64];
} NAND_BlockBuffer_S;

typedef union {
    uint8    d8[4];
    uint16    d16[2];
    uint32    d32;
} UnionData_U;

extern         NAND_Device_S  g_NandFlashDev;
extern const NAND_Device_S     g_NandFlashDevTbl_Internal[];
extern          NAND_Device_S     *g_NandFlashDevTbl;
extern NAND_DeviceInfo_S    g_NandFlashInfo;
extern NAND_DeviceInfo_S    *g_pNandInfo;

#pragma pack(pop) /* restore original alignment from stack */

#ifdef __cplusplus
}
#endif

#endif

