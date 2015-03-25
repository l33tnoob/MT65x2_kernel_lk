/*******************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of MediaTek Inc. (C) 2006
 *
 ******************************************************************************/

/*******************************************************************************
 * Filename:
 * ---------
 *  rom_info_internal.h
 *
 * Project:
 * --------
 *  DA
 *
 * Description:
 * ------------
 *  Security routines. (CAUTION!! Internal USE Only!)
 *
 * Author:
 * -------
 *  Amos Hsu (mtk00539)
 *
 *==============================================================================
 *           HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 * 10 01 2010 kevin.lim
 * [STP100004187]  Upload the BROM-lite source code on P4 server.
 * 10 01 2010 Kevin Lim
 * [STP100004187] BROM Lite v1.1037.2 release
 *
 * 10 01 2010 kevin.lim
 * [STP100004187]  Upload the BROM-lite source code on P4 server.
 * 10 01 2010 yh.sung
 * [STP100004187]  Release BROM_Lite v1.1037.2
 *
 * 01 08 2010 owen.chu
 * [STP100003662][FlashTool] v3.0952.00 release 
 * .
 *
 * Feb 24 2009 MTK02172
 * [STP100001912] [FlashTool] v3.0908.00 release
 * 
 *
 * Dec 1 2008 mtk01413
 * [STP100001857] [FlashTool] v3.08480 release
 * 
 *
 * Jan 31 2008 mtk01413
 * [STP100001630] [FlashTool] v3.0804.0 release
 * New features:
 *  1. USB Download for End User
 * Enhancements:
 *  1. [DA] Download Optimization with 921k on MT6225/MT6229/MT6230/MT6235/MT6238
 *  2. [DA] Provide two download methodology : Best Effort Erase (default) and Sequential Erase
 *  3. [DA] New CheckID algorithm for all SPANSION MCP series
 *
 * Jan 2 2007 mtk01413
 * [STP100001195] FlashTool v3.1.02 release
 * New features:
 *  1. [BROM_DLL/DA] Support FOTA(Firmware update Over The Air) download.
 *  2. [BROM_DLL/DA] Support MT6223  download.
 *  3. [BROM_DLL/DA] Support MT6268T download.
 *  4. [DA] Supports new NOR Flash device
 *         [SPANSION] S29GL128NH
 *         [SPANSION] S29GL128NL
 *         [SPANSION] S29GL256NH
 *         [SPANSION] S29GL256NL
 *         [SPANSION] S29GL512NH
 *         [SPANSION] S29GL512NL
 * 
 * Enhancements:
 *  1. [DA] Support BBC(Blank Block Check) for NOR Flash Format.
 *  2. [BROM_DLL/DA] Support Retransmit mechanism for NAND Readback operation.
 *  3. [BROM_DLL] Enhance E-NFB Check.
 * 
 *
 * Aug 14 2006 mtk00539
 * [STP100001057] FlashTool v3.1.00 release
 * 
 * 
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *==============================================================================
 *******************************************************************************/
#ifndef _ROM_INFO_INTERNAL_H_
#define _ROM_INFO_INTERNAL_H_

typedef enum {
     MTK_ROM_INFO_VER_00 = 0
    ,MTK_ROM_INFO_VER_01
    ,MTK_ROM_INFO_VER_02
    ,MTK_ROM_INFO_VER_03
    ,MTK_ROM_INFO_VER_04
    ,MTK_ROM_INFO_VER_05
    ,MTK_ROM_INFO_VER_06
    ,MTK_ROM_INFO_VER_07
} MTK_ROM_INFO_VER;

#define NEW_ROM_INFO_BEGIN            "MTK_ROMINFO_v"
#define NEW_ROM_INFO_BEGIN_LEN        13
#define NEW_ROM_INFO_LOCATE_RANGE    0x100000

#define MAX_ROM_PLATFORM_ID_LENGTH        64
#define MAX_ROM_PROJECT_ID_LENGTH        64

typedef struct {
    unsigned short        m_manu_id;
    unsigned short        m_dev_id;
    unsigned short        m_ext_dev_id1;
    unsigned short        m_ext_dev_id2;
    unsigned int        m_fat_begin_addr;
    unsigned int        m_fat_length;
} FlashDevInfo_ST;


typedef struct {
    char                m_identifier[16];
    char                m_platform_id[128];
    char                m_project_id[64];
    unsigned short        m_nfb_identifier;
    unsigned short        m_flash_dev_cnt;
    FlashDevInfo_ST        m_flash_info[6];
    unsigned int        m_sb_crc;
    unsigned int        m_sb_addr;
    unsigned int        m_sb_length;
} MTK_ROMInfo_v04_ST;

typedef struct {
    char                m_identifier[16];
    char                m_platform_id[128];
    char                m_project_id[64];
    unsigned short        m_nfb_identifier;
    unsigned short        m_flash_dev_cnt;
    FlashDevInfo_ST        m_flash_info[6];
    unsigned int        m_sb_crc;
    unsigned int        m_sb_addr;
    unsigned int        m_sb_length;
//----------------------------------------    
    unsigned int        m_cust_para_addr;
    unsigned int        m_cust_para_len;
    unsigned int        m_bit_ctrl;
} MTK_ROMInfo_v05_ST;

typedef struct {
    char                m_identifier[16];
    char                m_platform_id[128];
    char                m_project_id[64];
    unsigned short        m_nfb_identifier;
    unsigned short        m_flash_dev_cnt;
    FlashDevInfo_ST        m_flash_info[6];
    unsigned int        m_sb_crc;
    unsigned int        m_sb_addr;
    unsigned int        m_sb_length;
    unsigned int        m_cust_para_addr;
    unsigned int        m_cust_para_len;
    unsigned int        m_bit_ctrl;
//----------------------------------------    
    char                  m_nand_fdm_dal_ver[8];
} MTK_ROMInfo_v06_ST;

//
// ROM INFO v7, start from 08B/Maintrunk w0909
//
typedef struct {
    char                m_identifier[16];
    char                m_platform_id[128];
    char                m_project_id[64];
    unsigned short        m_nfb_identifier;
    unsigned short        m_flash_dev_cnt;
    FlashDevInfo_ST        m_flash_info[6];
    unsigned int        m_sb_crc;
    unsigned int        m_sb_addr;
    unsigned int        m_sb_length;
    unsigned int        m_cust_para_addr;
    unsigned int        m_cust_para_len;
    unsigned int        m_bit_ctrl;
    char                  m_nand_fdm_dal_ver[8];
//----------------------------------------    
    unsigned int        m_extsram_size;
    unsigned int        m_bl_maui_paired_ver;     //Allow update MAUI only when matched with BL
    unsigned int        m_feature_combination;    //Increase m_bl_maui_paired_ver if supported feature increases    
} MTK_ROMInfo_v07_ST;

typedef union {
    MTK_ROMInfo_v04_ST    m_v04;
    MTK_ROMInfo_v05_ST    m_v05;
    MTK_ROMInfo_v06_ST    m_v06;
    MTK_ROMInfo_v07_ST    m_v07;
} MTK_ROMInfo_U;


//--------------------------------------------------------------
// BOOT LOADER Related INFO
//--------------------------------------------------------------

/* BOOTLOADER INFO */
#define MTK_BLOADER_INFO_BEGIN    "MTK_BLOADER_INFO_v"

typedef enum {
     MTK_BLOADER_INFO_VER_UNKNOWN = 0
    ,MTK_BLOADER_INFO_VER_01
    ,MTK_BLOADER_INFO_VER_02
    ,MTK_BLOADER_INFO_VER_03
} MTK_BLOADER_INFO_VER;

typedef struct {
    char            m_identifier[24];
    char            m_filename[64];
    char            m_version[4];
    unsigned int    m_chksum_seed;
    unsigned int    m_start_addr;
    char            m_bin_identifier[8];
} BLoaderInfo_v01_ST;

typedef struct {
    char            m_identifier[24];
    char            m_filename[64];
    char            m_version[4];
    unsigned int    m_chksum_seed;
    unsigned int    m_start_addr;
    unsigned int    m_emi_gen_a;
    unsigned int    m_emi_gen_b;
    unsigned int    m_emi_con_i;
    unsigned int    m_emi_con_i_ext;
    unsigned int    m_emi_con_k;
    unsigned int    m_emi_con_l;
    char            m_bin_identifier[8];
} BLoaderInfo_v02_ST;

typedef struct {
    char            m_identifier[24];
    char            m_filename[64];
    char            m_version[4];
    unsigned int    m_chksum_seed;
    unsigned int    m_start_addr;
    unsigned int    m_emi_gen_a;
    unsigned int    m_emi_gen_b;
    unsigned int    m_emi_gen_c;
    unsigned int    m_emi_con_i;
    unsigned int    m_emi_con_i_ext;
    unsigned int    m_emi_con_k;
    unsigned int    m_emi_con_l;
    char            m_bin_identifier[8];
} BLoaderInfo_v03_ST;

typedef union {
    BLoaderInfo_v01_ST    m_v01;
    BLoaderInfo_v02_ST    m_v02;
    BLoaderInfo_v03_ST    m_v03;
} BLoaderInfo_U;

/* EXT BOOTLOADER INFO */
#define EXT_BLOADER_INFO_ID            "MTK_EBL_INFO_v"
#define EXT_BLOADER_INFO_ID_LEN        14

typedef enum {
     MTK_EXT_BLOADER_INFO_VER_UNKNOWN = 0
    ,MTK_EXT_BLOADER_INFO_VER_01
} MTK_EXT_BLOADER_INFO_VER;

typedef struct {
    char            m_identifier[16];
    unsigned int    m_bl_maui_paired_ver;
    unsigned int    m_feature_combination;
    unsigned char    m_fdm_major_ver;
    unsigned char    m_fdm_minor_ver1;
    unsigned char    m_fdm_minor_ver2;
    unsigned char    m_dal_ver;

    unsigned int    m_reserved[8];

} ExtBLoaderInfo_v01_ST;

typedef union {
    ExtBLoaderInfo_v01_ST    m_v01;
} ExtBLoaderInfo_U;

#endif

