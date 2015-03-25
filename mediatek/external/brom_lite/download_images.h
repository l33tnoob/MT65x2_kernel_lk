/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*******************************************************************************
* Filename:
* ---------
*  download_images.h
*
* Project:
* --------
*   Standalone Flash downloader sample code
*
* Description:
* ------------
*   This module contains download procedure header
*
* Author:
* -------
*  Kevin Lim (mtk60022)
*
*==============================================================================
*           HISTORY
* Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*------------------------------------------------------------------------------
* $Revision$
* $Modtime$
* $Log$
*
* 04 12 2013 monster.huang
* [STP100006902]  [FlashTool] v5.1312/v5.1314 maintenance
* [BROM LITE] Support combo memory and automatic generate eppParam.
* Fetch bmt address by acquiring linux partitions.
*
* 02 07 2013 stanley.song
* [STP100006748]  Update Brom Lite for SV5
* Update BROM Lite to support MT6255
 *
 * 10 01 2010 kevin.lim
 * [STP100004187]  Upload the BROM-lite source code on P4 server.
 * 10 01 2010 Kevin Lim
 * [STP100004187] BROM Lite v1.1037.2 release
*
*
*------------------------------------------------------------------------------
* Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*==============================================================================
*******************************************************************************/
#ifndef _DOWNLOAD_IMAGES_H
#define _DOWNLOAD_IMAGES_H

#include "image.h"
#include "_External/include/mtk_status.h"
#include "rom_info_internal.h"
#include "_External/include/mtk_mcu.h"
#include "_External/include/SW_TYPES.H"

//#define MT6235_NAND
//#define MT6251_SERIALFLASH
#define MT6276_NAND
//#define MT6236_NAND
//#define MT6223_NOR


// The current maximum number of MCPs supported in
// a Combo-MCP-enabled MAUI load is around 5
#define MAX_NUM_FLASH_INFO_ENTRIES              8
#define MAX_NUM_EXTERNAL_RAM_SETTING_ENTRIES    MAX_NUM_FLASH_INFO_ENTRIES


struct Image;

typedef struct
{
    _BOOL               m_valid;    // _TRUE -> dram setting is valid
    //NOT Support
} DRAM_SETTING;


/* BOOTLOADER INFO */
#define NEW_ROM_INFO_LOCATE_RANGE 0x100000

#define MTK_BLOADER_INFO_BEGIN    "MTK_BLOADER_INFO_v"

#define EMI_INFO_ID            "MTK_EMI_INFO"
#define EMI_INFO_ID_LEN        12

typedef struct {
    char            m_identifier[16];   // "MTK_EMI_INFO"
    unsigned int    m_ver;              // 0x0001

    unsigned int    m_emi_con_l;
    unsigned int    m_emi_con_i;
    unsigned int    m_emi_con_i_ext;
    unsigned int    m_emi_con_k;

    unsigned int    m_emi_gen_a;
    unsigned int    m_emi_gen_b;
} EMIInfo_v01_ST;    // for MT6228 || MT6225

typedef struct {
    char            m_identifier[16];   // "MTK_EMI_INFO"
    unsigned int    m_ver;              // 0x0002

    unsigned int    m_emi_con_l;
    unsigned int    m_emi_con_i;
    unsigned int    m_emi_con_i_ext;
    unsigned int    m_emi_con_k;

    unsigned int    m_emi_gen_a;
    unsigned int    m_emi_gen_b;
    unsigned int    m_emi_gen_c;
} EMIInfo_v02_ST;    // for MT6229 || MT6230

typedef struct {
    char            m_identifier[16];   // "MTK_EMI_INFO"
    unsigned int    m_ver;              // 0x0003

    unsigned int    m_emi_gen_a;
    unsigned int    m_emi_gen_b;
    unsigned int    m_emi_gen_c;
    unsigned int    m_emi_gen_d;

    unsigned int    m_emi_con_i;
    unsigned int    m_emi_con_j;
    unsigned int    m_emi_con_k;
    unsigned int    m_emi_con_l;
    unsigned int    m_emi_con_m;
    unsigned int    m_emi_con_n;
} EMIInfo_v03_ST;    // for MT6238 || MT6235

typedef struct {
    char            m_identifier[16];   // MTK_EMI_INFO
    unsigned int    m_ver;              // 0x0004

    unsigned int    m_emi_gen_a;
    unsigned int    m_emi_gen_b;
    unsigned int    m_emi_gen_c;
    unsigned int    m_emi_gen_d;

    unsigned int    m_emi_con_i;
    unsigned int    m_emi_con_j;
    unsigned int    m_emi_con_k;
    unsigned int    m_emi_con_l;
    unsigned int    m_emi_con_m;
    unsigned int    m_emi_con_n;

    unsigned int    m_emi_del_a;
    unsigned int    m_emi_del_b;
    unsigned int    m_emi_del_c;
    unsigned int    m_emi_del_d;
    unsigned int    m_emi_del_e;
    unsigned int    m_emi_del_f;
    unsigned int    m_emi_del_g;
    unsigned int    m_emi_del_h;
    unsigned int    m_emi_del_i;
    unsigned int    m_emi_del_j;

    unsigned int    m_emi_arb_a;
    unsigned int    m_emi_arb_b;
    unsigned int    m_emi_arb_c;
    unsigned int    m_emi_arb_d;
    unsigned int    m_emi_arb_e;
    unsigned int    m_emi_arb_f;
    unsigned int    m_emi_arb_g;
    unsigned int    m_emi_arb_h;
    unsigned int    m_emi_arb_i;
} EMIInfo_v04_ST;   // for MT6268

typedef union {
    EMIInfo_v01_ST    m_v01;
    EMIInfo_v02_ST    m_v02;
    EMIInfo_v03_ST    m_v03;
    EMIInfo_v04_ST    m_v04;
} EMIInfo_U;

#define ROM_SUPPORT_SYSTEM_DRIVE_ON_NAND    0x00000002        //bit1
#define ROM_SUPPORT_FOTA                    0x00000010        //bit4
#define ROM_SUPPORT_MBA                     0x00000020        //bit5

typedef struct
{
    unsigned int m_start_address;
    unsigned int m_length;    // 0 for an invalid range
} Range;

//
// External RAM setting
//
typedef enum DRAMType
{
    DRAMType_Invalid = 0,
    DRAMType_DDR,
    DRAMType_DDR2,
    DRAMType_DDR_166M,
    DRAMType_DDR_200M,
    DRAMType_DDR2_166M,
    DRAMType_DDR2_200M,
    DRAMType_End = 0x42424242

} DRAMType;


typedef struct DRAMSetting_v03
{
    DRAMType ramType;
    unsigned int EMI_CONI_Value;
    unsigned int EMI_CONJ_Value;
    unsigned int EMI_CONK_Value;
    unsigned int EMI_CONL_Value;
    unsigned int EMI_CONN_Value;
    unsigned int EMI_DQSA_Value;
    unsigned int EMI_DRVA_Value;
    unsigned int EMI_DRVB_Value;
    unsigned int EMI_ODLA_Value;
    unsigned int EMI_ODLB_Value;
    unsigned int EMI_ODLC_Value;
    unsigned int EMI_ODLD_Value;
    unsigned int EMI_ODLE_Value;
    unsigned int EMI_ODLG_Value;
} DRAMSetting_v03;

typedef struct DRAMSetting_v05
{
    DRAMType ramType;
    unsigned int EMI_CONI_Value;
    unsigned int EMI_CONJ_Value;
    unsigned int EMI_CONK_Value;
    unsigned int EMI_CONL_Value;
    unsigned int EMI_CONN_Value;
    // remove DQSA
    unsigned int EMI_DRVA_Value;
    unsigned int EMI_DRVB_Value;
    unsigned int EMI_ODLA_Value;
    unsigned int EMI_ODLB_Value;
    unsigned int EMI_ODLC_Value;
    unsigned int EMI_ODLD_Value;
    unsigned int EMI_ODLE_Value;
    unsigned int EMI_ODLF_Value; //New
    unsigned int EMI_ODLG_Value;
    // new 6 register
    unsigned int EMI_DUTA_Value;
    unsigned int EMI_DUTB_Value;
    unsigned int EMI_DUTC_Value;
    unsigned int EMI_DUCA_Value;
    unsigned int EMI_DUCB_Value;
    unsigned int EMI_DUCE_Value;
    unsigned int EMI_IOCL_Value;
} DRAMSetting_v05;
typedef struct DRAMSetting_v06
{
    DRAMType ramType;
    unsigned int EMI_CONI_Value;
    unsigned int EMI_CONJ_Value;
    unsigned int EMI_CONK_Value;
    unsigned int EMI_CONL_Value;
    unsigned int EMI_CONN_Value;

    unsigned int EMI_DRVA_Value;
    unsigned int EMI_DRVB_Value;
//    unsigned int EMI_ODLA_Value;
//    unsigned int EMI_ODLB_Value;
//    unsigned int EMI_ODLC_Value;
//    unsigned int EMI_ODLD_Value;
    unsigned int EMI_ODLE_Value;
    unsigned int EMI_ODLF_Value;
    unsigned int EMI_ODLG_Value;
    unsigned int EMI_ODLH_Value;
    //New 7 register (to v05)
    unsigned int EMI_ODLI_Value;
    unsigned int EMI_ODLJ_Value;
    unsigned int EMI_ODLK_Value;
    unsigned int EMI_ODLL_Value;
    unsigned int EMI_ODLM_Value;
    unsigned int EMI_ODLN_Value;

    unsigned int EMI_DUTA_Value;
    unsigned int EMI_DUTB_Value;
    unsigned int EMI_DUTC_Value;
    unsigned int EMI_DUCA_Value;
    unsigned int EMI_DUCB_Value;
    unsigned int EMI_DUCE_Value;
    unsigned int EMI_IOCL_Value;
} DRAMSetting_v06;


typedef enum ExtRamType
{
    RAMType_Invalid = 0,
    RAMType_DDR,         // MCP DRAM
    RAMType_DDR2,
    RAMType_DDR_166M,
    RAMType_DDR_200M,
    RAMType_DDR2_166M,
    RAMType_DDR2_200M,
    RAMType_DDR3_125M,
    RAMType_DDR_166M_SIP = 0x1000,   // SIP DRAM
    RAMType_DDR_200M_SIP,
    RAMType_DDR2_166M_SIP,
    RAMType_DDR2_200M_SIP,
    RAMType_SRAM_Normal = 0x10000,       // PSRAM
    RAMType_End = 0x42424242

} ExtRamType;

typedef struct ExternalRAMSetting
{
    unsigned int version;
    unsigned int numValidEntries;
    ExtRamType ramType;

    union
    {
        DRAM_SETTING    v00[MAX_NUM_EXTERNAL_RAM_SETTING_ENTRIES];
        DRAMSetting_v03 v03[MAX_NUM_EXTERNAL_RAM_SETTING_ENTRIES];
        DRAMSetting_v05 v05[MAX_NUM_EXTERNAL_RAM_SETTING_ENTRIES];
        DRAMSetting_v06 v06[MAX_NUM_EXTERNAL_RAM_SETTING_ENTRIES];
    } u;

} ExternalRAMSetting;

typedef struct flashID
{
    unsigned int idNumber; // max = 8
    unsigned char id[8];
}flashID;

// NOR
typedef struct FlashInfo_v01
{
    flashID NOR_ID;
    struct { int dummy; } NOR_geometry;

} FlashInfo_v01;


// NAND
typedef struct FlashInfo_v02
{
    flashID NAND_ID;
    struct { int dummy; } NAND_Geometry;

} FlashInfo_v02;

//
// Flash information
//

typedef enum FLASHType
{
    FLASHType_Invalid = 0,
    FLASHType_NAND,
    FLASHType_NOR,
    FLASHType_SERIAL_NOR_FLASH,
    FLASHType_End = 0x42424242

} FLASHType;

typedef enum CFG_Type_Version
{
    CFGType_Invalid = 0,
    CFGType_ALPHA3,
    CFGType_V1,
    CFGType_V2,
    CFGType_End = 0x42424242
}CFG_Type_Version;

typedef enum PMIC_Controller
{
    PMIC_Dev_Invalid = 0x0,
    PMIC_MT6321,
    PMIC_MT6327,
    PMIC_Dev_End = 0x42424242
}PMIC_Controller;



typedef struct FlashInfo
{
    unsigned int version;
    unsigned int numValidEntries;
    FLASHType flashType;

    union
    {
        FlashInfo_v01 v01[MAX_NUM_FLASH_INFO_ENTRIES];
        FlashInfo_v02 v02[MAX_NUM_FLASH_INFO_ENTRIES];
    } u;

} FlashInfo;


//
// External memory setting
//
typedef struct ExternalMemorySetting
{
    _BOOL               valid;
    BBCHIP_TYPE         bbchipType;
    CFG_Type_Version    CFGVersion;
    PMIC_Controller     PMICController;
    ExternalRAMSetting  ramSetting;
    FlashInfo           flashInfo;  // Required for Combo MCP

} ExternalMemorySetting;

//
// EPP PARAM
//

typedef union
{
    DRAMSetting_v03 v03;
    DRAMSetting_v05 v05;
    DRAMSetting_v06 v06;
} DRAMSetting_U;

typedef struct EPP_PARAM
{
    CFG_Type_Version cfg_version;
    ExtRamType       m_ExtRamType;
    PMIC_Controller  m_PMIC_Controller;
    DRAMSetting_U    MTK_DRAM_Setting;

    unsigned char reserved[256 -
                   sizeof(unsigned int) -
                   sizeof(ExtRamType) -
                   sizeof(PMIC_Controller) -
                   sizeof(DRAMSetting_U)];
} EPP_PARAM;


//GFH Information
typedef enum {
     GFH_FILE_NONE          = 0x0000                //Recognized by BootROM
    ,ARM_BL                 = 0x0001
    ,ARM_EXT_BL             = 0x0002
    ,DUALMAC_DSP_BL         = 0x0003
    ,SCTRL_CERT             = 0x0004
    ,TOOL_AUTH              = 0x0005
    ,FILE_MTK_RESERVED1     = 0x0006
    ,EPP                    = 0x0007
    ,FILE_MTK_RESERVED2     = 0x0008
    ,FILE_MTK_RESERVED3     = 0x0009
    ,ROOT_CERT              = 0x000a

    ,V_MAUI_BINARY          = 0x0100                 //MAUI binary group
    ,PRIMARY_MAUI           = V_MAUI_BINARY
    ,SECONDARY_MAUI         = 0x0101
    ,ON_DEMAND_PAGING       = 0x0102
    ,THIRD_ROM              = 0x0103
    ,DSP_ROM                = 0x0104
    ,CACHED_DSP_ROM         = 0x0105
    ,V_MAUI_BINARY_END      = 0x017f

    ,V_RESOURCE_BINARY      = 0x0180                //Resource binary group
    ,CUSTOM_PACK            = V_RESOURCE_BINARY
    ,LANGUAGE_PACK          = 0x0181
    ,JUMP_TABLE             = 0x0182
    ,V_RESOURCE_BINARY_END  = 0x01ff

    ,V_MISC_BINARY          = 0x0200                //Binary not belonging to MAUI
    ,FOTA_UE                = V_MISC_BINARY
    ,V_MISC_BINARY_END

    ,V_SECURE_RO            = 0x0300                //Secure structure group
    ,SECURE_RO_S            = V_SECURE_RO
    ,SECURE_RO_ME           = 0x0301
    ,V_SECURE_RO_END

    ,V_EXTERNAL_FILE        = 0x0400
    ,CARD_DOWNLOAD_PACKAGE  = V_EXTERNAL_FILE
    ,CONFIDENTIAL_BINARY    = 0x0401
    ,V_EXTERNAL_FILE_END

    ,V_FILE_SYSTEM_BINARY   = 0x0480                //File System
    ,FILE_SYSTEM            = V_FILE_SYSTEM_BINARY
    ,V_FILE_SYSTEM_BINARY_END

    ,BOOT_CERT              = 0x0500

    ,GFH_FILE_END           = (U16)0xffff
} GFH_FILE_TYPE;

typedef enum {

     GFH_FILE_INFO          = 0x0000
    ,GFH_BL_INFO            = 0x0001
    ,GFH_ANTI_CLONE         = 0x0002
    ,GFH_BL_SEC_KEY         = 0x0003
    ,GFH_SCTRL_CERT         = 0x0004
    ,GFH_TOOL_AUTH          = 0x0005
    ,GFH_MTK_RESERVED1      = 0x0006
    ,GFH_BROM_CFG           = 0x0007
    ,GFH_BROM_SEC_CFG       = 0x0008
    ,GFH_MTK_RESERVED2      = 0x0009
    ,GFH_MTK_RESERVED3      = 0x000a
    ,GFH_ROOT_CERT          = 0x000b
    ,GFH_EXP_CHK            = 0x000c
    ,GFH_EPP_PARAM          = 0x000d
    ,GFH_CHIP_VER           = 0x000e
    ,GFH_MTK_RESERVED4      = 0x000f

    ,GFH_EPP_INFO           = 0x0100
    ,GFH_EMI_LIST           = 0x0101
    ,GFH_CMEM_ID_INFO       = 0x0102
    ,GFH_CMEM_NOR_INFO      = 0x0103

    ,GFH_MAUI_INFO          = 0x0200
    ,GFH_MAUI_SEC           = 0x0201
    ,GFH_MAUI_CODE_KEY      = 0x0202    //MAUI_SEC_KEY for code part
    ,GFH_MAUI_SECURE_RO_KEY = 0x0203    //MAUI_SEC_KEY for secure ro part
    ,GFH_MAUI_RESOURCE_KEY  = 0x0204    //MAUI_SEC_KEY for resource part

    ,GFH_SECURE_RO_INFO     = 0x0205

    ,GFH_DL_PACKAGE_INFO    = 0x0206
    ,GFH_FLASH_INFO         = 0x0207
    ,GFH_MACR_INFO          = 0x0208
    ,GFH_ARM_BL_INFO        = 0x0209
    ,GFH_EMMC_BOOTING_INFO  = 0x020A
    ,GFH_FOTA_INFO          = 0x020B
    ,GFH_CBR_RECORD_INFO    = 0x020C
    ,GFH_CONFIDENTIAL_BIN_INFO = 0x020D
    ,GFH_CBR_INFO           = 0x020E

    ,GFH_BOOT_CERT          = 0x0300

    ,GFH_TYPE_NUM
    ,GFH_TYPE_END           = (U16)0xffff
} GFH_TYPE;

typedef enum {
     FLASH_DEV_NONE         = 0
    ,F_NOR                  = 1
    ,F_NAND_SEQUENTIAL      = 2
    ,F_NAND_TTBL            = 3
    ,F_NAND_FDM50           = 4
    ,F_EMMC_BOOT_REGION     = 5
    ,F_EMMC_DATA_REGION     = 6
    ,F_SF                   = 7
    ,FLASH_DEV_END          = (U8)255
} GFH_FLASH_DEV;

typedef enum {
     SIG_NONE               = 0
    ,SIG_PHASH              = 1
    ,SIG_SINGLE             = 2
    ,SIG_SINGLE_AND_PHASH   = 3
    ,SIG_MULTI              = 4
    ,SIG_TYPE_NUM
    ,SIG_TYPE_END           = (U8)255
} GFH_SIG_TYPE;

typedef struct GFH_Header {
    U32                 m_magic_ver;
    U16                 m_size;
    #if defined(WIN32) || defined(__GNUC__)
    U16                 m_type;
    #else
    GFH_TYPE            m_type;
    #endif
} GFH_Header;

typedef struct GFH_FILE_INFO_v1 {
    GFH_Header      m_gfh_hdr;
    char            m_identifier[12];       // including '\0'
    U32             m_file_ver;
#if defined(WIN32) || defined(__GNUC__)
    U16             m_file_type;
    U8              m_flash_dev;
    U8              m_sig_type;
#else
    GFH_FILE_TYPE   m_file_type;
    GFH_FLASH_DEV   m_flash_dev;
    GFH_SIG_TYPE    m_sig_type;
#endif
    U32             m_load_addr;
    U32             m_file_len;
    U32             m_max_size;
    U32             m_content_offset;
    U32             m_sig_len;
    U32             m_jump_offset;
    U32             m_attr;
} GFH_FILE_INFO_v1;

typedef struct GFH_BL_INFO_v1 {
    GFH_Header      m_gfh_hdr;
    U32             m_bl_attr;
} GFH_BL_INFO_v1;

typedef struct GFH_CBR_INFO_v1 {
    GFH_Header      m_gfh_hdr;
    U32             m_cbr_normal_block_num;
    U32             m_cbr_spare_block_num;
    U32             m_reserved[4];
} GFH_CBR_INFO_v1;

typedef struct GFH_CBR_INFO_v2 {
    GFH_Header      m_gfh_hdr;
    U32             m_cbr_normal_block_num;
    U32             m_cbr_spare_block_num;
    U32             m_cbr_normal_region_size;
    U32             m_cbr_spare_region_size;
    U32             m_reserved[2];
} GFH_CBR_INFO_v2;

STATUS_E download_images(const struct image *download_agent,
                         const struct image *download_agent_TCM,
                         const struct image *nor_flash_table,
                         const struct image *nand_flash_table,
                         const struct image *download_EPP,
                         const struct image *bootLoader,
                         const struct image *extBootLoader,
                         const struct image *dspBootLoader,
                         const struct image *rom,
                         const struct image *secondRom,
                         const struct image *dspRom,
                         const struct image *demand_paging_rom,
                         struct image *linux_images,
                         const struct ExternalMemorySetting *externalMemorySetting,
                         unsigned int num_linux_images,
                         unsigned int bmt_address,
                         int isUSB,
                         int isNFB);

#endif /* _DOWNLOAD_IMAGES_H */

