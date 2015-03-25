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

#ifndef __MMC_COMMON_INTER_H__
#define __MMC_COMMON_INTER_H__

#include "msdc_cfg.h"
#include "mmc_types.h"

#if defined(MMC_MSDC_DRV_PRELOADER)
extern u32 mmc_init_device (void);

#if CFG_LEGACY_USB_DOWNLOAD
extern u32 mmc_read_data (u8 * buf, u32 offset);
extern u32 mmc_write_data (u8 * buf, u32 offset);
extern bool mmc_erase_data (u32 offset, u32 offset_limit, u32 size);
extern void mmc_wait_ready (void);
extern u32 mmc_find_safe_block (u32 offset);

extern u32 mmc_chksum_per_file (u32 mmc_offset, u32 img_size);
extern u32 mmc_chksum_body (u32 chksm, char *buf, u32 pktsz);
extern u32 mmc_get_device_id(u8 *id, u32 len,u32 *fw_len);
#endif
#endif

#if defined(MMC_MSDC_DRV_LK)
int mmc_legacy_init(int verbose);
unsigned long mmc_wrap_bread(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *dst);
unsigned long mmc_wrap_bwrite(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *src);
#endif

extern int mmc_do_erase(int dev_num,u64 start_addr,u64 len);

#endif /* __MMC_COMMON_INTER_H__ */
