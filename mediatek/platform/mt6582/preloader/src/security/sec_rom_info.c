/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "sec_platform.h"

/* import rom_info variable */
#include "sec_rom_info.h"

/* import partition table */
#include "partition_define.h"

/* import custom info */
#include "cust_bldr.h"
#include "cust_sec_ctrl.h"
#include "KEY_IMAGE_AUTH.h"
#include "KEY_SML_AUTH.h"
#include "KEY_SML_ENCODE.h"
#include "PL_CFG.h"

/**************************************************************************
*  MACRO
**************************************************************************/ 
#define MOD                             "ROM_INFO"

#define yes                             (1)
#define no                              (0)

/******************************************************************************
 * DEBUG
 ******************************************************************************/
#define SEC_DEBUG                       (FALSE)
#define SMSG                            print
#if SEC_DEBUG
#define DMSG                            print
#else
#define DMSG 
#endif


/**************************************************************************
 *  GLOBAL VARIABLES
 **************************************************************************/

/**************************************************************************
 *  LOCAL VARIABLES
 **************************************************************************/

/**************************************************************************
 *  EXTERNAL VARIABLES
 **************************************************************************/

/**************************************************************************
 *  EXTERNAL FUNCTIONS
 **************************************************************************/

/**************************************************************************
 *  DO NOT MODIFY THIS !
 **************************************************************************/
__attribute__((section(".data.rom_info")))
AND_ROMINFO_T  g_ROM_INFO = 
{
    /* ================================================================ */
    /* ROM_INFO                                                         */
    /* ================================================================ */

                                                            /* MTK */    
    .m_identifier                       = ROM_INFO_NAME                           

                                                            /* MTK */
    ,.m_rom_info_ver                    = PRELOADER_SEC_VERSION               

                                                            /* CUSTOMER */
    ,.m_platform_id                     = PLATFORM_NAME                             

                                                            /* CUSTOMER */
    ,.m_project_id                      = PROJECT_NAME                    

    /* bit to indicate whether SEC_RO_IMG does exist */     /* MTK */
    ,.m_sec_ro_exist                    = ROM_INFO_SEC_RO_EXIST                

                                                            /* MTK */     
#if (CFG_BOOT_DEV == BOOTDEV_SDMMC) & (!CFG_FPGA_PLATFORM)
    ,.m_sec_ro_offset                   = PART_OFFSET_SEC_RO
#else
    ,.m_sec_ro_offset                   = PART_OFFSET_SEC_RO
#endif
    /* sec_ro_length */                                     /* MTK */    
    /* SEC_RO IMG encrypted size */
    ,.m_sec_ro_length                   = 0x2C00

    /* ac_offset from ROM_INFO */                           /* MTK */
    ,.m_ac_offset                       = ROM_INFO_ANTI_CLONE_OFFSET 

    /* ac_length */                                         /* MTK */
    ,.m_ac_length                       = ROM_INFO_ANTI_CLONE_LENGTH 

    /* sec_cfg_offset */                                    /* MTK */
#if (CFG_BOOT_DEV == BOOTDEV_SDMMC) & (!CFG_FPGA_PLATFORM)
    ,.m_sec_cfg_offset                  = PART_OFFSET_SECCFG 
#else
    ,.m_sec_cfg_offset                  = PART_OFFSET_SECCFG
#endif

    /* sec_cfg_length */                                    /* MTK */
    ,.m_sec_cfg_length                  = PART_SIZE_SECCFG           


    /* ================================================================ */
    /* FLASHTOOL_SECCFG_T                                               */
    /* ================================================================ */
#if defined(FLASHTOOL_SEC_CFG)    
                                                            /* MTK */    
    ,.m_flashtool_cfg.m_magic_num       = FLASHTOOL_CFG_MAGIC

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[0].m_img_name = BYPASS_CHECK_IMAGE_0_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[0].m_img_offset = BYPASS_CHECK_IMAGE_0_OFFSET

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[0].m_img_length = BYPASS_CHECK_IMAGE_0_LENGTH

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[1].m_img_name = BYPASS_CHECK_IMAGE_1_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[1].m_img_offset = BYPASS_CHECK_IMAGE_1_OFFSET

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[1].m_img_length = BYPASS_CHECK_IMAGE_1_LENGTH

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[2].m_img_name = BYPASS_CHECK_IMAGE_2_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[2].m_img_offset = BYPASS_CHECK_IMAGE_2_OFFSET

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[2].m_img_length = BYPASS_CHECK_IMAGE_2_LENGTH   
#elif defined(FLASHTOOL_SEC_CFG_64)
                                                        /* MTK */    
    ,.m_flashtool_cfg.m_magic_num       = FLASHTOOL_CFG_MAGIC_64

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[0].m_img_name = BYPASS_CHECK_IMAGE_0_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[0].m_img_offset_high = ((BYPASS_CHECK_IMAGE_0_OFFSET & 0xFFFFFFFF00000000) >> 32)

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[0].m_img_offset_low =  (BYPASS_CHECK_IMAGE_0_OFFSET & 0x00000000FFFFFFFF)

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[1].m_img_name = BYPASS_CHECK_IMAGE_1_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[1].m_img_offset_high = ((BYPASS_CHECK_IMAGE_1_OFFSET & 0xFFFFFFFF00000000) >> 32)

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[1].m_img_offset_low =  (BYPASS_CHECK_IMAGE_1_OFFSET & 0x00000000FFFFFFFF)

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[2].m_img_name = BYPASS_CHECK_IMAGE_2_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[2].m_img_offset_high = ((BYPASS_CHECK_IMAGE_2_OFFSET & 0xFFFFFFFF00000000) >> 32)
    
                                                            /* CUSTOMER */
    ,.m_flashtool_cfg.m_bypass_check_img[2].m_img_offset_low =  (BYPASS_CHECK_IMAGE_2_OFFSET & 0x00000000FFFFFFFF)

#else
    ,.m_flashtool_cfg                   = {0x0}
#endif


    /* ================================================================ */
    /* FLASHTOOL_FORBID_DOWNLOAD_T                                               */
    /* ================================================================ */
#if defined(FLASHTOOL_FORBID_DL_NSLA_CFG)                                             /* MTK */    
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_magic_num       = FLASHTOOL_NON_SLA_FORBID_MAGIC

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[0].m_img_name = FORBID_DL_IMAGE_0_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[0].m_img_offset = FORBID_DL_IMAGE_0_OFFSET

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[0].m_img_length = FORBID_DL_IMAGE_0_LENGTH

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[1].m_img_name = FORBID_DL_IMAGE_1_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[1].m_img_offset = FORBID_DL_IMAGE_1_OFFSET

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[1].m_img_length = FORBID_DL_IMAGE_1_LENGTH
#elif defined(FLASHTOOL_FORBID_DL_NSLA_CFG_64)                                             /* MTK */    
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_magic_num       =   FLASHTOOL_NON_SLA_FORBID_MAGIC_64

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[0].m_img_name = FORBID_DL_IMAGE_0_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[0].m_img_offset_high = ((FORBID_DL_IMAGE_0_OFFSET & 0xFFFFFFFF00000000) >> 32)
    
                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[0].m_img_offset_low = (FORBID_DL_IMAGE_0_OFFSET & 0x00000000FFFFFFFF)

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[1].m_img_name = FORBID_DL_IMAGE_1_NAME

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[1].m_img_offset_high = ((FORBID_DL_IMAGE_1_OFFSET & 0xFFFFFFFF00000000) >> 32)

                                                            /* CUSTOMER */
    ,.m_flashtool_forbid_dl_nsla_cfg.m_forbid_dl_nsla_img[1].m_img_offset_low = (FORBID_DL_IMAGE_1_OFFSET & 0x00000000FFFFFFFF)


#else
    ,.m_flashtool_forbid_dl_nsla_cfg                   = {0x0}
#endif
   
    /* ================================================================ */
    /* AND_SECCTRL_T                                                    */
    /* ================================================================ */
    
                                                            /* MTK */
    ,.m_SEC_CTRL.m_identifier           = ROM_INFO_SEC_CTRL_ID        

                                                            /* MTK */
    ,.m_SEC_CTRL.m_sec_cfg_ver          = ROM_INFO_SEC_CTRL_VER      

                                                            /* CUSTOMER */
    ,.m_SEC_CTRL.m_sec_usb_dl           = SEC_USBDL_CFG               

                                                            /* CUSTOMER */
    ,.m_SEC_CTRL.m_sec_boot             = SEC_BOOT_CFG                  
    
                                                            /* CUSTOMER */                                                            
    ,.m_SEC_CTRL.m_sec_modem_auth       = MTK_SEC_MODEM_AUTH                  

                                                            /* CUSTOMER */
    ,.m_SEC_CTRL.m_sec_sds_en           = 0x0
                                                            /* CUSTOMER */
#ifdef SECCFG_ANTICLONE_DIS
    ,.m_SEC_CTRL.m_seccfg_ac_en         = 0x0
#else
    ,.m_SEC_CTRL.m_seccfg_ac_en         = 0x1
#endif    

                                                            /* CUSTOMER */
#ifdef SML_CUSTOM_AES_256
    ,.m_SEC_CTRL.m_sec_aes_legacy       = 0x1
#else
    ,.m_SEC_CTRL.m_sec_aes_legacy       = 0x0
#endif

                                                            /* CUSTOMER */
#ifndef MTK_SEC_SECRO_AC_SUPPORT
    ,.m_SEC_CTRL.m_secro_ac_en          = 0x0
#else                                                            
    ,.m_SEC_CTRL.m_secro_ac_en          = MTK_SEC_SECRO_AC_SUPPORT
#endif 

                                                            /* CUSTOMER */
#ifdef SML_AES_KEY_ANTICLONE_EN
    ,.m_SEC_CTRL.m_sml_aes_key_ac_en    = 0x1
#else
    ,.m_SEC_CTRL.m_sml_aes_key_ac_en    = 0x0
#endif

                                                            /* MTK */
    ,.m_SEC_CTRL.reserve                = {0x0}

                                                            /* MTK */
    ,.m_reserve2                        = {0x0}


    /* ================================================================ */
    /* AND_SECBOOT_ENABLE_PART_T                                        */
    /* ================================================================ */
    
    ,.m_SEC_BOOT_CHECK_PART.name[0]     = SBOOT_PART_UBOOT 
    ,.m_SEC_BOOT_CHECK_PART.name[1]     = SBOOT_PART_LOGO  
    ,.m_SEC_BOOT_CHECK_PART.name[2]     = SBOOT_PART_BOOTIMG  
    ,.m_SEC_BOOT_CHECK_PART.name[3]     = SBOOT_PART_RECOVERY 
    ,.m_SEC_BOOT_CHECK_PART.name[4]     = SBOOT_PART_ANDSYSIMG 

    #if MTK_SEC_SECRO_AC_SUPPORT
        ,.m_SEC_BOOT_CHECK_PART.name[5] = SBOOT_PART_SECSTATIC      
    #else
        ,.m_SEC_BOOT_CHECK_PART.name[5] = {0x0}
    #endif


    ,.m_SEC_BOOT_CHECK_PART.name[6]     = {0x0}
#ifdef CUSTOMIZED_SECURE_PARTITION_SUPPORT    
    ,.m_SEC_BOOT_CHECK_PART.name[7]     = SBOOT_CUST_PART1      /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[8]     = SBOOT_CUST_PART2      /* CUSTOMER */
#else
    ,.m_SEC_BOOT_CHECK_PART.name[7]     = {0x0}                 /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[8]     = {0x0}                 /* CUSTOMER */
#endif

    /* ================================================================ */
    /* AND_SECKEY_T                                                     */
    /* ================================================================ */

                                                            /* MTK */
    ,.m_SEC_KEY.m_identifier            = ROM_INFO_SEC_KEY_ID            

                                                            /* MTK */
    ,.m_SEC_KEY.m_sec_key_ver           = ROM_INFO_SEC_KEY_VER           

                                                            /* CUSTOMER */
    ,.m_SEC_KEY.img_auth_rsa_n          = IMG_CUSTOM_RSA_N                

                                                            /* CUSTOMER */
    ,.m_SEC_KEY.img_auth_rsa_e          = IMG_CUSTOM_RSA_E                

                                                            /* MTK */
#ifdef SML_AES_KEY_ANTICLONE_EN
    /* SML AES key is moved to SEC_RO Image Anti-Clone Region */
    ,.m_SEC_KEY.sml_aes_key             = 0
#else    
    /* SML AES key is stored in ROM INFO area */
    #ifdef SML_CUSTOM_AES_256
    ,.m_SEC_KEY.sml_aes_key             = SML_CUSTOM_AES_256
    #endif
    
    #ifdef SML_CUSTOM_AES_128
    ,.m_SEC_KEY.sml_aes_key             = SML_CUSTOM_AES_128
    #endif
#endif

                                                            /* CUSTOMER */
    ,.m_SEC_KEY.crypto_seed             = CUSTOM_CRYPTO_SEED             

                                                            /* CUSTOMER */
    ,.m_SEC_KEY.sml_auth_rsa_n          = SML_CUSTOM_RSA_N            

                                                            /* CUSTOMER */
    ,.m_SEC_KEY.sml_auth_rsa_e          = SML_CUSTOM_RSA_E            
    
};

void sec_rom_info_init (void)
{
    COMPILE_ASSERT(AND_ROM_INFO_SIZE == sizeof(AND_ROMINFO_T));
    SMSG("[%s] 'v%d','0x%x','0x%x','0x%x','0x%x'\n",
                                            MOD,  g_ROM_INFO.m_rom_info_ver,
                                            g_ROM_INFO.m_sec_cfg_offset,
                                            g_ROM_INFO.m_sec_cfg_length,
                                            g_ROM_INFO.m_sec_ro_offset,
                                            g_ROM_INFO.m_sec_ro_length);
}
