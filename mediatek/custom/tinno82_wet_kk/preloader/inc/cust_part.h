/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef CUST_PART_H
#define CUST_PART_H

#include "cust_bldr.h"
#include "partition_define.h"

#define PART_PRELOADER              "PRELOADER"
#define PART_DSP_BL                 "DSP_BL"
#define PART_MBOOT                  "MBOOT"
#define PART_MBL                    "MBL"
#define PART_MBR                    "MBR"
#define PART_EBR1                   "EBR1"
#define PART_EBR2                   "EBR2"
#define PART_UBOOT                  "UBOOT"
#define PART_SECURE                 "SECCNFG"
#define PART_LOGO                   "LOGO"
#define PART_CUST                   "CUSTOME"
#define PART_BACKUP                 "BACKUP"
#define PART_IMAGE                  "IMAGE"
#define PART_BOOTIMG                "BOOTIMG"
#define PART_USER                   "USER"
#define PART_ANDSYSIMG              "ANDSYSIMG"
#define PART_CACHE                  "CACHE"
#define PART_MISC                   "MISC"
#define PART_NVRAM                  "NVRAM"
#define PART_RECOVERY               "RECOVERY"
#define PART_EXPDB                  "EXPDB"
#define PART_AUTHEN_FILE            "AUTHEN_FILE"
#define PART_SECSTATIC              "SECSTATIC"
#define PART_KERNEL                 "KERNEL"
#define PART_ROOTFS                 "ROOTFS"
#define PART_AP_ROM                 "AP_ROM"
#define PART_MD_2G_ROM              "MD_2G_ROM"
#define PART_MD_3G_ROM              "MD_3G_ROM"
#define PART_DSP_2G_ROM             "DSP_2G_ROM"
#define PART_DSP_3G_ROM             "DSP_3G_ROM"
#define PART_MD_FS                  "MD_FS"
#define PART_PMT                    "PMT"

#define PART_MAX_COUNT              20

#define PART_FLAG_NONE              0
#define PART_FLAG_LEFT              0x1
#define PART_FLAG_END               0x2
#define PART_MAGIC                  0x58881688

/*=======================================================================*/
/* Partitions                                                            */
/*=======================================================================*/
#define CFG_USE_MBL_PARTITION

#ifdef SLT_BOOTLOADER
#define CFG_USE_AP_ROM_PARTITION
#define CFG_USE_DSP_3G_ROM_PARTITION
#define CFG_USE_MD_3G_ROM_PARTITION
#define CFG_USE_DSP_2G_ROM_PARTITION
#define CFG_USE_MD_2G_ROM_PARTITION
#define CFG_USE_MD_FS_PARTITION
#else
#define CFG_USE_UBOOT_PARTITION
#define CFG_USE_SECURE_PARTITION
#define CFG_USE_LOGO_PARTITION
#define CFG_USE_BOOTIMG_PARTITION
#define CFG_USE_RECOVERY_PARTITION
#define CFG_USE_SECSTATIC_PARTITION
#define CFG_USE_MISC_PARTITION
#define CFG_USE_NVRAM_PARTITION
#define CFG_USE_CACHE_PARTITION
#define CFG_USE_ANDROID_SYSIMG_PARTITION
#define CFG_USE_EXPDB_PARTITION
#define CFG_USE_USER_PARTITION

#ifdef MTK_EMMC_SUPPORT
#define CFG_USE_MBR_PARTITION
#define CFG_USE_EBR1_PARTITION
#define CFG_USE_EBR2_PARTITION
#define CFG_USE_PMT_PARTITION
#endif

#endif

typedef union
{
    struct
    {
        unsigned int magic;     /* partition magic */
        unsigned int dsize;     /* partition data size */
        char name[32];          /* partition name */    
        unsigned int maddr;     /* partition memory address */
    } info;
    unsigned char data[512];
} part_hdr_t;

#ifdef MTK_EMMC_SUPPORT
typedef struct
{
    unsigned char *name;        /* partition name */
    unsigned long startblk;     /* partition start blk */
    unsigned long long size;         /* partition size */
    unsigned long blks;         /* partition blks */
    unsigned long flags;        /* partition flags */
} part_t;
#else
typedef struct
{
    unsigned char *name;        /* partition name */
    unsigned long startblk;     /* partition start blk */
    unsigned long size;         /* partition size */
    unsigned long blks;         /* partition blks */
    unsigned long flags;        /* partition flags */
} part_t;
#endif

extern void cust_part_init(void);
extern part_t *cust_part_tbl(void);

#endif /* CUST_PART_H */
