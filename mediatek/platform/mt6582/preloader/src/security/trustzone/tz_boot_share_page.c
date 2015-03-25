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
#include "typedefs.h"
#include "tz_boot_share_page.h"
#include "mt6582.h"

#define MOD "[BT_SD_PG]"

void tz_boot_share_page_init(void)
{
    /* assume D-cache is diabled */
    /* 1. clear boot share page */
    memset((void*)BOOT_SHARE_BASE, 0, BOOT_SHARE_SIZE);

    /* 2. Fill magic number */
    *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_MAGIC1_OFST) = BOOT_SHARE_MAGIC;
    *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_MAGIC2_OFST) = BOOT_SHARE_MAGIC;
    
    /* 3. Fill device information */
    *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_DEV_INFO_OFST+0x00000000) = DRV_Reg32(APHW_CODE);
    *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_DEV_INFO_OFST+0x00000004) = DRV_Reg32(APHW_SUBCODE);
    *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_DEV_INFO_OFST+0x00000008) = DRV_Reg32(APHW_VER);
    *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_DEV_INFO_OFST+0x0000000c) = DRV_Reg32(APSW_VER);
#if 0
    print("%s device info 0x%x 0x%x 0x%x 0x%x\n", MOD, DRV_Reg32(APHW_CODE), DRV_Reg32(APHW_SUBCODE), DRV_Reg32(APHW_VER), DRV_Reg32(APSW_VER));    
    print("%s device info 0x%x 0x%x 0x%x 0x%x\n", MOD, *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_DEV_INFO_OFST+0x00000000), 
            *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_DEV_INFO_OFST+0x00000004), 
            *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_DEV_INFO_OFST+0x00000008), 
            *(unsigned int*)(BOOT_SHARE_BASE+BOOT_SHARE_DEV_INFO_OFST+0x0000000c));
#endif
}
