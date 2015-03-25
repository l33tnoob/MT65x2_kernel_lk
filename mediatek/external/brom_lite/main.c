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
*  main.c
*
* Project:
* --------
*   Standalone Flash downloader sample code
*
* Description:
* ------------
*   This module contains main procedure
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
* 04 17 2013 monster.huang
* [STP100006902]  [BROMLite] v5.1312/v5.1314 maintenance
* Add EMI settings for 
* Hynix H9DA1GH25HAMMR_4EM
* ESMT FM64D1G56A_5BAGE     
* Micorn MT29C1G12MAADVAMD_5IT
* 
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
#include <assert.h>
//#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "interface.h"
#include "download_images.h"
#if defined(__GNUC__)
#include "GCC_Utility.h"
#endif
//jone.wang@2013-4
#include "mt_gpio.h"
#include <fcntl.h>

/* for LOG */
int enable_log = 1;
int PSIZ = 1024;

//6280
/*
[EXTMemorySetting]
valid
BBCHIP_TYPE
CFG_Type_Version
PMIC_Controller

ExternalRAMSetting version
ExternalRAMSetting numValidEntries
ExtRamType
*/

//added by xiaohui 4-19
//come from interface.c
//pipe_fd,used for updating progress bar in recovery
//pipe_buf,buffer used for pipe_fd
//extern char dev_path[64];
//extern int pipe_fd;
//extern char *pipe_buf;

//
static const unsigned char externalMemorySetting[] =
{
    //valid            , BBCHIP_TYPE        , CFG_Type_Version   , PMIC_Controller
    0x01,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x03,0x00,0x00,0x00, 0x02,0x00,0x00,0x00,
    //ExtRam version   , ExtRam Entries     , ExtRam Type
    0x06,0x00,0x00,0x00, 0x03,0x00,0x00,0x00, /*0x04,0x00,0x00,0x00,*/ 0x03, 0x00, 0x00,0x00,
    //# EMI 1
    0x04,0x00,0x00,0x00, 0x40,0x40,0x32,0x00, 0x87,0x24,0x10,0x40, 0x00,0x20,0x0B,0x00,
    0x05,0x50,0x72,0x00, 0x00,0x00,0x5A,0x00, 0x77,0x77,0x77,0x77, 0x00,0x00,0x77,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x55,0x05,0x05,0x55,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x03,0x10,
    //# EMI 2
    0x03,0x00,0x00,0x00, 0x40,0x40,0x32,0x00, 0x45,0x12,0x0c,0x00, 0x00,0x10,0x0B,0x00,
    0x05,0x30,0x72,0x00, 0x00,0x00,0x5A,0x00, 0x77,0x77,0x77,0x77, 0x00,0x00,0x77,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x55,0x05,0x05,0x55,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x03,0x10,
    //# EMI 3
    0x04,0x00,0x00,0x00, 0x40,0x40,0x32,0x00, 0x87,0x24,0x11,0x40, 0x00,0x20,0x0B,0x00,
    0x05,0x50,0x31,0x00, 0x00,0x00,0x4A,0x00, 0x77,0x77,0x77,0x77, 0x00,0x00,0x77,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x55,0x05,0x05,0x55,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x03,0x10,
    //# EMI 4
    0x04,0x00,0x00,0x00, 0x40,0x40,0x32,0x00, 0x87,0x24,0x09,0x40, 0x00,0x20,0x0B,0x00,
    0x05,0x50,0x76,0x00, 0x00,0x00,0x4A,0x00, 0x77,0x77,0x77,0x77, 0x00,0x00,0x77,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x55,0x05,0x05,0x55,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x03,0x10,
    //# EMI 5
    0x04,0x00,0x00,0x00, 0x40,0x40,0x32,0x00, 0x87,0x24,0x11,0x40, 0x00,0x20,0x0B,0x00,
    0x05,0x50,0x31,0x00, 0x00,0x00,0x5A,0x00, 0x77,0x77,0x77,0x77, 0x00,0x00,0x77,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x55,0x05,0x05,0x55,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x03,0x10,
    //# EMI 6
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    //# EMI 7
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    //# EMI 8
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    //FlashInfo version, FlashInfo Entries  , Flash type
    0x01,0x00,0x00,0x00, 0x03,0x00,0x00,0x00, 0x01,0x00,0x00,0x00,
    
    //# FlashID 1
    0x05,0x00,0x00,0x00, 0x2C,0xB1,0x80,0x55, 0x02,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    //# FlashID 2
    0x05,0x00,0x00,0x00, 0x2C,0xB1,0x80,0x55, 0x04,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    //# FlashID 3
    0x04,0x00,0x00,0x00, 0xAD,0xB1,0x00,0x55, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    //# FlashID 4
    0x05,0x00,0x00,0x00, 0xC8,0xB1,0x80,0x55, 0x40,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    //# FlashID 5
    0x04,0x00,0x00,0x00, 0xAD,0xB1,0x00,0x55, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    //# FlashID 6
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    //# FlashID 7
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    //# FlashID 8
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
};


/************** BIN Setting for NAND linux partition image *******************/
//xiaohui modify begin 1 4-22
static const char *LINUX_PARTITION_IMAGE_FILE_NAME[] =
{
    NULL,/*MD_BL*/
    NULL,/*MDAP_BL*/
    NULL,/*BBM*/
    NULL,/*NVRAM*/
    "uboot.bin",/*UBOOT*/
    "uboot.bin",/*UBOOT2*/
    "boot.img",/*Linux*/
    "ful.bin",/*FUL*/
    "fc-hosted.bin",/*FactoryConfig*/
    "configpack.bin",/*ConfigPack1*/
    "configpack.bin",/*ConfigPack2*/
    "system.img",/*SYSTEM*/
    "userdata.img",/*USERDATA*/
    NULL,/*CDROM*/
    NULL,/*FWU*/
    NULL,/*MODEM*/
    "modem.img",/*MODEM2*/
    "SECURE_RO",/*SecurePartition*/
    NULL,/*LOGO*/
    NULL/*_BMTPOOL*/
};
static char *LINUX_PARTITION_IMAGE_FILE_PATH[20];
//xiaohui modify end 1


static const unsigned int LINUX_PARTITION_FILE_ADDR[] =
{
    0x0,      /*MD_BL*/
    0x20000,  /*MDAP_BL*/
    0x40000,  /*BBM*/
    0x100000, /*NVRAM*/
    0x400000, /*UBOOT*/
    0x480000, /*UBOOT2*/
    0x500000, /*Linux*/
    0x880000, /*FUL*/
    0x8c0000, /*FactoryConfig*/
    0x900000, /*ConfigPack1*/
    0x940000, /*ConfigPack2*/
    0x980000, /*SYSTEM*/
    0x2580000, /*USERDATA*/
    0x3780000, /*CDROM*/
    0x4580000, /*FWU*/
    0x6880000, /*MODEM*/
    0x6e80000, /*MODEM2*/
    0x7480000, /*SecurePartition*/
    0x74A0000, /*LOGO*/
    0x7B00000  /*_BMTPOOL*/
};
static const char *LINUX_PARTITION_IMAGE_NAME[] =
{
    "MD_BL",
    "MDAP_BL",
    "BBM",
    "NVRAM",
    "UBOOT",
    "UBOOT2",
    "Linux",
    "FUL",
    "FactoryConfig",
    "ConfigPack1",
    "ConfigPack2",
    "SYSTEM",
    "USERDATA",
    "CDROM",
    "FWU",
    "MODEM",
    "MODEM2",
    "SecurePartition",
    "LOGO",
    "_BMTPOOL"
};

static const unsigned int LINUX_PARTITION_TYPE[] =
{
    RESERVED, /*MD_BL*/
    RESERVED, /*MDAP_BL*/
    RESERVED, /*BBM*/
    RESERVED, /*NVRAM*/
    RAW, /*UBOOT*/
    RAW, /*UBOOT2*/
    RAW, /*Linux*/
    RAW, /*FUL*/
    RAW, /*FactoryConfig*/
    RAW, /*ConfigPack1*/
    RAW, /*ConfigPack2*/
    YAFFS, /*SYSTEM*/
    YAFFS, /*USERDATA*/
    RESERVED, /*CDROM*/
    RESERVED, /*FWU*/
    RESERVED, /*MODEM*/
    RAW, /*MODEM2*/
    RAW, /*SecurePartition*/
    RESERVED, /*LOGO*/
    RESERVED  /*_BMTPOOL*/
};

//for MT6280
// for 80 tranditional

/*
static const char *BOOT_LOADER_FILE_PATH = "D:\\brom_lite_test_load\\6280\\tdongle\\MT6280_MP_R7R8_HSPA_BOOTLOADER_V005_MT6280_WR8_W13_01_P1.bin";
static const char *EXT_BOOT_LOADER_FILE_PATH = "D:\\brom_lite_test_load\\6280\\tdongle\\EXT_BOOTLOADER";
static const char *DSP_BOOT_LOADER_FILE_PATH = "";

static const char *ROM_FILE_PATH = "D:\\brom_lite_test_load\\6280\\tdongle\\MT6280_MP_R7R8_HSPA_PCB01_MT6280_S01.WR8_W13_01_P1.bin";
static const char *SECONDARY_ROM_FILE_PATH = "";
static const char *DSP_ROM_FILE_PATH = "";
static const char *DEMAND_PAGING_ROM0_FILE_PATH = "";
*/

//xiaohui modify begin 2 4-22,for host
//name
static const char *BOOT_LOADER_FILE_NAME = "md_bl.bin";
static const char *EXT_BOOT_LOADER_FILE_NAME = "preloader.bin";
static const char *DSP_BOOT_LOADER_FILE_NAME = NULL;

static const char *ROM_FILE_NAME = NULL;
static const char *SECONDARY_ROM_FILE_NAME = NULL;
static const char *DSP_ROM_FILE_NAME = NULL;
static const char *DEMAND_PAGING_ROM0_FILE_NAME = NULL;

//path
static char *BOOT_LOADER_FILE_PATH = NULL;
static char *EXT_BOOT_LOADER_FILE_PATH = NULL;
static char *DSP_BOOT_LOADER_FILE_PATH = NULL;

static char *ROM_FILE_PATH = NULL;
static char *SECONDARY_ROM_FILE_PATH = NULL;
static char *DSP_ROM_FILE_PATH = NULL;
static char *DEMAND_PAGING_ROM0_FILE_PATH = NULL;
//xiaohui modify end 2

/*
// for 80 hosted
static const char *BOOT_LOADER_FILE_PATH = "D:\\brom_lite_test_load\\6280\\rndis\\md_bl.bin";
static const char *EXT_BOOT_LOADER_FILE_PATH = "D:\\brom_lite_test_load\\6280\\rndis\\preloader.bin";
static const char *DSP_BOOT_LOADER_FILE_PATH = "";

static const char *ROM_FILE_PATH = "";
static const char *SECONDARY_ROM_FILE_PATH = "";
static const char *DSP_ROM_FILE_PATH = "";
static const char *DEMAND_PAGING_ROM0_FILE_PATH = "";
*/

//for ROM iamge (NAND)
static const unsigned int IMAGE_LOAD_ADDR[] =
{
    0x00000000
};


/*****************************************************************/
//for MT6280
static const unsigned int DOWNLOAD_AGENT_LOAD_ADDR = 0x70008000;
//static const unsigned int DOWNLOAD_AGENT_TCM_LOAD_ADDR = 0x00020000;
static const unsigned int DOWNLOAD_EPP_LOAD_ADDR = 0x70008000  ;


// MTK USB Driver
//xiaohui modify begin 3
//name
static const char *DOWNLOAD_AGENT_FILE_NAME = "INT_SYSRAM";
//static const char *DOWNLOAD_AGENT_TCM_FILE_NAME = "EXT_RAM";
static const char *NOR_FLASH_TABLE_FILE_NAME = "NOR_FLASH_TABLE";
//static const char *NOR_FLASH_TABLE_FILE_NAME = NULL;
static const char *NAND_FLASH_TABLE_FILE_NAME = "NAND_FLASH_TABLE";
static const char *DOWNLOAD_EPP_FILE_NAME = "EPP";

//path
static char *DOWNLOAD_AGENT_FILE_PATH = NULL;
//static const char *DOWNLOAD_AGENT_TCM_FILE_PATH = NULL;
static char *NOR_FLASH_TABLE_FILE_PATH = NULL;
static char *NAND_FLASH_TABLE_FILE_PATH = NULL;
static char *DOWNLOAD_EPP_FILE_PATH = NULL;
//xiaohui modify end 3
/*
static const char *DOWNLOAD_AGENT_FILE_PATH = "D:\\brom_lite_test_load\\6280\\INT_SYSRAM";
//static const char *DOWNLOAD_AGENT_TCM_FILE_PATH = "D:\\brom_lite_test_load\\EXT_RAM";
static const char *NOR_FLASH_TABLE_FILE_PATH = "D:\\brom_lite_test_load\\6280\\NOR_FLASH_TABLE";
static const char *NAND_FLASH_TABLE_FILE_PATH = "D:\\brom_lite_test_load\\6280\\NAND_FLASH_TABLE";
static const char *DOWNLOAD_EPP_FILE_PATH = "D:\\brom_lite_test_load\\6280\\EPP";
*/

//generate_all_image_path()
//generate all the paths of the images to be download
static int generate_all_image_path(char *path)
{
    int i;
    const char * file_name = NULL;

    memset(LINUX_PARTITION_IMAGE_FILE_PATH, 0, sizeof(LINUX_PARTITION_IMAGE_FILE_PATH));
    printf("image file count is %d\n", sizeof(LINUX_PARTITION_IMAGE_FILE_PATH)/sizeof(LINUX_PARTITION_IMAGE_FILE_PATH[0]));

    printf("Modem image list:    num, image_name, image_file_name, download_addr\n");
    for(i=0;i<20;i++)
    {
        if(LINUX_PARTITION_IMAGE_FILE_NAME[i])
        {
            file_name = LINUX_PARTITION_IMAGE_FILE_NAME[i];

            LINUX_PARTITION_IMAGE_FILE_PATH[i] = (char *)malloc(strlen(path) + strlen(file_name) + 1);
            memset(LINUX_PARTITION_IMAGE_FILE_PATH[i], 0, strlen(path) + strlen(file_name) + 1);
            strcpy(LINUX_PARTITION_IMAGE_FILE_PATH[i], path);
            LINUX_PARTITION_IMAGE_FILE_PATH[i] = strcat(LINUX_PARTITION_IMAGE_FILE_PATH[i], file_name);
            if (access(LINUX_PARTITION_IMAGE_FILE_PATH[i], R_OK) != 0) {
                fprintf(stderr, "path '%s` is invalid. %d, %s\n", LINUX_PARTITION_IMAGE_FILE_PATH[i], errno, strerror(errno));
                return -1;
            }

        }
        printf("%d,%s,%s,%x\n", i, LINUX_PARTITION_IMAGE_NAME[i], LINUX_PARTITION_IMAGE_FILE_PATH[i], LINUX_PARTITION_FILE_ADDR[i]);
    }
    printf("\n\n");



    if(BOOT_LOADER_FILE_NAME)
    {
        file_name = (char *)BOOT_LOADER_FILE_NAME;
        
        BOOT_LOADER_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(BOOT_LOADER_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(BOOT_LOADER_FILE_PATH, path);
        BOOT_LOADER_FILE_PATH = strcat(BOOT_LOADER_FILE_PATH, file_name);
    }

    if(BOOT_LOADER_FILE_PATH)
        printf("%s\n",BOOT_LOADER_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");



    if(EXT_BOOT_LOADER_FILE_NAME)
    {
        file_name = (char *)EXT_BOOT_LOADER_FILE_NAME;        
        EXT_BOOT_LOADER_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(EXT_BOOT_LOADER_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(EXT_BOOT_LOADER_FILE_PATH, path);
        EXT_BOOT_LOADER_FILE_PATH = strcat(EXT_BOOT_LOADER_FILE_PATH, file_name);
    }
    if(EXT_BOOT_LOADER_FILE_PATH)
        printf("%s\n",EXT_BOOT_LOADER_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");



    if(DSP_BOOT_LOADER_FILE_NAME)
    {
        file_name = (char *)DSP_BOOT_LOADER_FILE_NAME;
        DSP_BOOT_LOADER_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(DSP_BOOT_LOADER_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(DSP_BOOT_LOADER_FILE_PATH, path);
        DSP_BOOT_LOADER_FILE_PATH = strcat(DSP_BOOT_LOADER_FILE_PATH, file_name);
    }
    if(DSP_BOOT_LOADER_FILE_PATH)
        printf("%s\n",DSP_BOOT_LOADER_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");

    
    if(ROM_FILE_NAME)
    {
        file_name = (char *)ROM_FILE_NAME;
        ROM_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(ROM_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(ROM_FILE_PATH, path);
        ROM_FILE_PATH = strcat(ROM_FILE_PATH, file_name);
    }
    if(ROM_FILE_PATH)
        printf("%s\n",ROM_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");



    if(SECONDARY_ROM_FILE_NAME)
    {
        file_name = (char *)SECONDARY_ROM_FILE_NAME;
        SECONDARY_ROM_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(SECONDARY_ROM_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(SECONDARY_ROM_FILE_PATH, path);
        SECONDARY_ROM_FILE_PATH = strcat(SECONDARY_ROM_FILE_PATH, file_name);
    }
    if(SECONDARY_ROM_FILE_PATH)
        printf("%s\n",SECONDARY_ROM_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");




    if(DSP_ROM_FILE_NAME)
    {
        file_name = (char *)DSP_ROM_FILE_NAME;
        DSP_ROM_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(DSP_ROM_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(DSP_ROM_FILE_PATH, path);
        DSP_ROM_FILE_PATH = strcat(DSP_ROM_FILE_PATH, file_name);
    }
    if(DSP_ROM_FILE_PATH)
        printf("%s\n",DSP_ROM_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");




    if(DEMAND_PAGING_ROM0_FILE_NAME)
    {
        file_name = (char *)DEMAND_PAGING_ROM0_FILE_NAME;
        DEMAND_PAGING_ROM0_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(DEMAND_PAGING_ROM0_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(DEMAND_PAGING_ROM0_FILE_PATH, path);
        DEMAND_PAGING_ROM0_FILE_PATH = strcat(DEMAND_PAGING_ROM0_FILE_PATH, file_name);
    }
    if(DEMAND_PAGING_ROM0_FILE_PATH)
        printf("%s\n",DEMAND_PAGING_ROM0_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");




    if(DOWNLOAD_AGENT_FILE_NAME)
    {
        file_name = (char *)DOWNLOAD_AGENT_FILE_NAME;
        DOWNLOAD_AGENT_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(DOWNLOAD_AGENT_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(DOWNLOAD_AGENT_FILE_PATH, path);
        DOWNLOAD_AGENT_FILE_PATH = strcat(DOWNLOAD_AGENT_FILE_PATH, file_name);
    }
    if(DOWNLOAD_AGENT_FILE_PATH)
        printf("%s\n",DOWNLOAD_AGENT_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");




    if(NOR_FLASH_TABLE_FILE_NAME)
    {
        file_name = (char *)NOR_FLASH_TABLE_FILE_NAME;
        NOR_FLASH_TABLE_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(NOR_FLASH_TABLE_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(NOR_FLASH_TABLE_FILE_PATH, path);
        NOR_FLASH_TABLE_FILE_PATH = strcat(NOR_FLASH_TABLE_FILE_PATH, file_name);
    }
    if(NOR_FLASH_TABLE_FILE_PATH)
        printf("%s\n",NOR_FLASH_TABLE_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");


    if(NAND_FLASH_TABLE_FILE_NAME)
    {
        file_name = (char *)NAND_FLASH_TABLE_FILE_NAME;
        NAND_FLASH_TABLE_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(NAND_FLASH_TABLE_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(NAND_FLASH_TABLE_FILE_PATH, path);
        NAND_FLASH_TABLE_FILE_PATH = strcat(NAND_FLASH_TABLE_FILE_PATH, file_name);
    }
    if(NAND_FLASH_TABLE_FILE_PATH)
        printf("%s\n",NAND_FLASH_TABLE_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");



    if(DOWNLOAD_EPP_FILE_NAME)
    {
        file_name = (char *)DOWNLOAD_EPP_FILE_NAME;
        DOWNLOAD_EPP_FILE_PATH = (char *)malloc(strlen(path) + strlen(file_name) + 1);
        memset(DOWNLOAD_EPP_FILE_PATH, 0, strlen(path) + strlen(file_name) + 1);
        strcpy(DOWNLOAD_EPP_FILE_PATH, path);
        DOWNLOAD_EPP_FILE_PATH = strcat(DOWNLOAD_EPP_FILE_PATH, file_name);
    }
    if(DOWNLOAD_EPP_FILE_PATH)
        printf("%s\n",DOWNLOAD_EPP_FILE_PATH);
    //else
    //    printf("%s\n","path point is NULL");

    return 0;
}

static int load_image(struct image *image, const char *file_path,
                       const unsigned int load_addr)
{
    FILE *file;

    assert(image != NULL);
    if(!file_path)
    {
        image->buf = NULL;
        image->len = 0;
        image->load_addr = load_addr;
        return 0;
    }

    file = fopen(file_path, "rb");
    if (file == NULL) {
        printf("open %s error, %d, %s\n", file_path, errno, strerror(errno));
        return -1;
    }
    //assert(file != NULL);

#if defined(_MSC_VER)
    image->len = _filelength(fileno(file));
#elif defined(__GNUC__)
    {
        fseek(file, 0L, SEEK_END);
        image->len = ftell(file);
        fseek(file, 0L, SEEK_SET);
    }
#endif
    assert(image->len != 0);

    image->buf = malloc(image->len);
    assert(image->buf != NULL);

    fread(image->buf, 1, image->len, file);
    fclose(file);

    image->load_addr = load_addr;

    return 0;
}

static void load_image_info(struct image *image, const char *name,
                       PartitionType type)
{
    memcpy(image->name,name,MAX_PARTITION_NAME_LEN);
    image->type= type;
}


static void release_image(struct image *image)
{
    assert(image != NULL);

    free(image->buf);
    image->buf = NULL;
}


static int acquire_linux_images(   struct image **images,
                                    unsigned int *num_images,
                                    unsigned int *bmt_address)
{
    const unsigned int NUM_IMAGES = sizeof(LINUX_PARTITION_IMAGE_FILE_PATH) /
        sizeof(LINUX_PARTITION_IMAGE_FILE_PATH[0]);
    /*const unsigned int NUM_IMAGES_NAME = sizeof(LINUX_PARTITION_IMAGE_NAME) /
        sizeof(LINUX_PARTITION_IMAGE_NAME[0]);*/
    unsigned int i;

    //assert(NUM_IMAGES == NUM_IMAGES_NAME);

    assert(images != NULL);
    *images = (struct image *)malloc(sizeof(struct image) * NUM_IMAGES);
    memset(*images, 0, sizeof(struct image) * NUM_IMAGES);
    assert(*images != NULL);

    assert(num_images != NULL);
    *num_images = NUM_IMAGES;

    for (i=0; i<NUM_IMAGES; ++i)
    {
        struct image *image_i = &(*images)[i];
        if (load_image(image_i, LINUX_PARTITION_IMAGE_FILE_PATH[i], LINUX_PARTITION_FILE_ADDR[i]) != 0)
            return -1;
        load_image_info(image_i, LINUX_PARTITION_IMAGE_NAME[i], LINUX_PARTITION_TYPE[i]);
        if(!strcmp(LINUX_PARTITION_IMAGE_NAME[i], "_BMTPOOL"))
        {
            *bmt_address = LINUX_PARTITION_FILE_ADDR[i];
        }
    }
    return 0;
}


static void release_images(struct image **images, unsigned int num_images)
{
    unsigned int i;

    assert(images != NULL);
    assert(*images != NULL);

    for (i=0; i<num_images; ++i)
    {
        struct image *image_i = &(*images)[i];
        release_image(image_i);
    }

    free(*images);
    *images = NULL;
}


static int acquire_download_agent(struct image *download_agent,const char *file_path, unsigned int load_addr)
{
    return load_image(download_agent, file_path, load_addr);
}


static void release_download_agent(struct image *download_agent)
{
    release_image(download_agent);
}


static int acquire_flash_table(struct image *flash_table,
                                const char *file_path)
{
    return load_image(flash_table, file_path, 0);
}

static void release_flash_table(struct image *flash_table)
{
    release_image(flash_table);
}

static int acquire_image(struct image *imageBuf,
                          const char *file_path, unsigned int load_addr)
{
    return load_image(imageBuf, file_path, load_addr);
}

//jone.wang@
/*static void set_modem_dl_and_reset(int *store_data)
{
    int fd;
    
    fd = open("/dev/mtgpio", O_RDWR);
    if (fd < 0) {
        LOGE( "Can't open /dev/mtgpio");
    }

    store_data[0] = ioctl(fd, GPIO99,GPIO_IOCQDIR);
    if (store_data[0] == GPIO_DIR_IN)
        store_data[1] = ioctl(fd, GPIO99,GPIO_IOCQDATAIN);

    LOGE("gpio99 state is dir=%d, value=%d\n", store_data[0], store_data[1]);

    ioctl(fd, GPIO99, GPIO_IOCTMODE0);
    ioctl(fd, GPIO99, GPIO_IOCSDIROUT);
    ioctl(fd, GPIO99, GPIO_IOCSDATALOW);


    store_data[2] = ioctl(fd, GPIO95,GPIO_IOCQDIR);
    if (store_data[2] == GPIO_DIR_IN)
        store_data[3] = ioctl(fd, GPIO95,GPIO_IOCQDATAIN);
    LOGE("gpio95 state is dir=%d, value=%d\n", store_data[3], store_data[4]);

    ioctl(fd, GPIO95, GPIO_IOCTMODE0);    
    ioctl(fd, GPIO95, GPIO_IOCSDIROUT);
    ioctl(fd, GPIO95, GPIO_IOCSDATALOW);    
    sleep(2);
    ioctl(fd, GPIO95, GPIO_IOCSDATAHIGH);    
    close(fd);
}*/

/*static const char *TEMPORARY_LOG_FILE = "/tmp/recovery.log";*/

int main(int argc,char *argv[])
{
    //struct image *norImages = NULL;
    //unsigned int num_nor_images = 0;
    struct image download_agent = { NULL, 0, 0, "", 0 };
    struct image download_agent_TCM = { NULL, 0, 0, "", 0 };
    struct image download_EPP = { NULL, 0, 0, "", 0 };
    struct image nor_flash_table = { NULL, 0, 0, "", 0 };
    struct image nand_flash_table = { NULL, 0, 0, "", 0 };
    struct image *linux_images = NULL;
    //unsigned int num_images = 0;
    unsigned int num_linux_images = 0;
    unsigned int bmt_address = 0;
    //int i = 0;

    struct image boot_loader = { NULL, 0, 0, "", 0 };
    struct image ext_boot_loader = { NULL, 0, 0, "", 0 };
    //struct image dsp_boot_loader = { NULL, 0, 0, "", 0 };
    struct image rom_image = { NULL, 0, 0, "", 0 };
    //struct image secondary_rom_image = { NULL, 0, 0, "", 0 };
    //struct image dsp_rom_image = { NULL, 0, 0, "", 0 };
    //struct image demand_paging_image = { NULL, 0, 0, "", 0 };

    //jone.wang@

    //freopen(TEMPORARY_LOG_FILE, "a", stdout); setbuf(stdout, NULL);
    //freopen(TEMPORARY_LOG_FILE, "a", stderr); setbuf(stderr, NULL);
    //fflush(stdout);
    //fflush(stderr);
    //setbuf(stdout, NULL);
    //setbuf(stderr, NULL);
    //fprintf(stdout, "Brom Ite main =================\n");
    //added by xiaohui 4-19

    if (argc < 2) {
        fprintf(stderr, "Usage: %s filepath [write_block_size] [trace_log]\n", basename(argv[0]));
        return -1;
    }

    char path[BUFSIZ] = "";
    strcpy(path, argv[1]);

    /* "/system/etc/firmware/modem" normally */
    if (access(path, R_OK) != 0) {
        fprintf(stderr, "path '%s` is invalid. %d, %s\n", path, errno, strerror(errno));
        return -1;
    }

    path[strlen(path)] = '/';
    LOG("path: %s\n", path);

    if (generate_all_image_path(path) != 0)
        return -1;

    //strcpy(dev_path,argv[2]);
    //pipe_fd = atoi(argv[3]);
    //pipe_buf = (char *)malloc(PIPE_BUF_SIZE);
    //if(!pipe_buf)
        //return 0;
    //printf("dev_path=%s\n",dev_path);
    //printf("pipe fd=%d\n",pipe_fd);

    log_feedback("begin to feedback!!!\n");
    //printf("%s\n",pipe_buf);
    //return 0;
    //added by xiaohui 4-19    

    if (argc > 2) {
        int psiz = atoi(argv[2]);
        if (psiz > 0) {
            PSIZ = psiz;
        }
    }
    LOG("PSIZ = %d\n", PSIZ);

    int trace_log = 0;
    if (argc > 3) {
        if (argv[3][0] == '0')
            trace_log = 0;
        else
            trace_log = 1;
    }
    if (trace_log)
        LOG("enable log tracing\n");
    enable_log = trace_log;

    int rc = -1;

    do {
        // Acquire pointers to DA, flash tables, and images
        if ((rc = acquire_download_agent(&download_agent,DOWNLOAD_AGENT_FILE_PATH,DOWNLOAD_AGENT_LOAD_ADDR)) != 0)
            break;
        //acquire_download_agent(&download_agent_TCM,DOWNLOAD_AGENT_TCM_FILE_PATH, DOWNLOAD_AGENT_TCM_LOAD_ADDR);
        if ((rc = acquire_download_agent(&download_EPP,DOWNLOAD_EPP_FILE_PATH, DOWNLOAD_EPP_LOAD_ADDR)) != 0)
            break;
        if ((rc = acquire_flash_table(&nor_flash_table, NOR_FLASH_TABLE_FILE_PATH)) != 0)
            break;
        if ((rc = acquire_flash_table(&nand_flash_table, NAND_FLASH_TABLE_FILE_PATH)) != 0)
            break;

        //Acquire Boot loader
        if ((rc = acquire_image(&boot_loader, BOOT_LOADER_FILE_PATH,0)) != 0)
            break;
        if ((rc = acquire_image(&ext_boot_loader, EXT_BOOT_LOADER_FILE_PATH,0)) != 0)
            break;
        //acquire_image(&dsp_boot_loader, DSP_BOOT_LOADER_FILE_PATH,0);
        //Acquire ROM images
        if ((rc = acquire_image(&rom_image, ROM_FILE_PATH,0)) != 0)
            break;
        //acquire_image(&secondary_rom_image, SECONDARY_ROM_FILE_PATH,0);
        //acquire_image(&dsp_rom_image, DSP_ROM_FILE_PATH,0);
        //acquire_image(&demand_paging_image, DEMAND_PAGING_ROM0_FILE_PATH,0);

        //Acquire linux partition images
        if ((rc = acquire_linux_images(&linux_images, &num_linux_images, &bmt_address)) != 0)
            break;
    } while (0);

    if (rc != 0)
        return rc;

    rc = download_images(&download_agent,
                         &download_agent_TCM,
                         &nor_flash_table,
                         &nand_flash_table,
                         &download_EPP,
                         &boot_loader,
                         &ext_boot_loader,
                         NULL, //&dsp_boot_loader,
                         &rom_image,
                         NULL, //&secondary_rom_image,
                         NULL, //&dsp_rom_image,
                         NULL, //&demand_paging_image,
                         linux_images,
                         (const struct ExternalMemorySetting *)&externalMemorySetting,
                         sizeof(LINUX_PARTITION_IMAGE_FILE_NAME) / sizeof(LINUX_PARTITION_IMAGE_FILE_NAME[0]), //num_linux_images,modified by xiaohui 4-22
                         bmt_address,
                         1, /*0:UART, 1:USB*/
                         1 /*0:NOR, 1:NAND*/);
    if (rc != S_DONE) {
        printf("download failed, rc %d\n", rc);
    } else {
        //jone.wang
        printf("download done\n");
    }

    release_images(&linux_images, num_linux_images);

    release_image(&boot_loader);
    release_image(&ext_boot_loader);
    release_image(&rom_image);
    //release_image(&secondary_rom_image);
    //release_image(&demand_paging_image);

    release_flash_table(&nand_flash_table);
    release_flash_table(&nor_flash_table);
    release_download_agent(&download_agent);
    release_download_agent(&download_agent_TCM);
    release_download_agent(&download_EPP);

    //added by xiaohui 4-19
    //close(pipe_fd);
    //free(pipe_buf);

    printf("brom_lite end, rc %d\n", rc);
    return rc;
}

