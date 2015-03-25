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

#include "typedefs.h"
#include "platform.h"
#include "download.h"

#if CFG_LEGACY_USB_DOWNLOAD
#include "mtk_wdt.h"
#include "mt6577_rtc.h"


#define MOD                      "<DM>"

/**************************************************************************
*  MACRO DEFINITION
**************************************************************************/
#if DM_DBG_LOG
#define DM_ASSERT(expr)      {  if ((expr)==FALSE){  \
    print("%s : [ASSERT] at %s #%d %s\n       %s\n       above expression is not TRUE\n", MOD, __FILE__, __LINE__, __FUNCTION__, #expr); \
    while(1){};  }                    \
                                 }
#define DM_STATE_LOG(state)  print("%s : state %s\n", MOD, (state))
#define DM_ENTRY_LOG()       print("%s : enter %s\n", MOD, __FUNCTION__)
#define DM_LOG               print
#else
#define DM_ASSERT(expr)
#define DM_STATE_LOG(state)
#define DM_ENTRY_LOG()
#define DM_LOG
#endif

/**************************************************************************
*  EXTERNAL DECLARATION
**************************************************************************/
extern DM_CONTEXT dm_ctx;

/**************************************************************************
 *  Return Image Format
 **************************************************************************/
u8 * get_img_fmt (DM_IMG_FORMAT fmt)
{
    switch (fmt)
    {
    case DM_IMG_FORMAT_FAT:
        return "FAT";
    case DM_IMG_FORMAT_YAFFS2:
        return "YAFFS2";
    case DM_IMG_FORMAT_UNKNOWN:
    default:
        return "UNKNOWN";
    }
}


/**************************************************************************
 *  Return Image Type
 **************************************************************************/
u8 * get_img_type (DM_IMG_TYPE type)
{
    switch (type)
    {
    case DM_IMG_TYPE_LOGO:
        return "LOGO";
    case DM_IMG_TYPE_BOOTIMG:
        return "ANDROID_BOOTING";
    case DM_IMG_TYPE_RECOVERY:
        return "RECOVERY";
    case DM_IMG_TYPE_ANDROID:
        return "AND SYSTEM";
    case DM_IMG_TYPE_USRDATA:
        return "USRDATA";
    case DM_IMG_TYPE_UBOOT:
        return "UBOOT";
    case DM_IMG_TYPE_AUTHEN_FILE:
        return "AUTHEN_FILE";
    case DM_IMG_TYPE_SECSTATIC:
        return "SEC_RO";
    case DM_IMG_TYPE_EU_FT_INFORM:
        return "EU_FT_INFORM";
    case DM_IMG_TYPE_FT_LOCK_INFORM:
        return "FT_LOCK_INFORM";
    case DM_IMG_TYPE_PT_TABLE_INFORM:
        return "PT_TABLE_INFORM";

    case DM_IMG_TYPE_CUST_IMAGE1:
        return "CUST_IMAGE1";
    case DM_IMG_TYPE_CUST_IMAGE2:
        return "CUST_IMAGE2";
    case DM_IMG_TYPE_CUST_IMAGE3:
        return "CUST_IMAGE3";
    case DM_IMG_TYPE_CUST_IMAGE4:
        return "CUST_IMAGE4";
    case DM_IMG_TYPE_UNKNOWN:
    default:
        print ("'%s : image type = %d' ", MOD, type);
        return "UNKNOWN";
    }
}

/**************************************************************************
 *  Debug
 **************************************************************************/
#if DM_DBG_LOG
void dump_dm_descriptor (void)
{
    DM_LOG ("%s : ------------------ DM ---------------\n", MOD);
    DM_LOG ("%s : DM state  = %d err = 0x%x   pag off = 0x%x\n", MOD, dm_ctx.dm_status, dm_ctx.dm_err, dm_ctx.page_off);
    DM_LOG ("%s : DM Blk sz = 0x%x \t cur off = 0x%x\n", MOD, dm_ctx.block_size, dm_ctx.curr_off);
    DM_LOG ("%s : DM Pag sz = 0x%x \t pkt cnt = 0x%x\n", MOD, dm_ctx.page_size, dm_ctx.pkt_cnt);
    DM_LOG ("%s : DM Spr sz = 0x%x \t cur cnt = 0x%x\n", MOD, dm_ctx.spare_size, dm_ctx.curr_cnt);
    DM_LOG ("%s : --------------------------------------\n", MOD);
    DM_LOG ("%s : DM range  = 0x%x ~ 0x%x\n", MOD, dm_ctx.img_info.addr_off, dm_ctx.img_info.addr_off + dm_ctx.part_range);
    DM_LOG ("%s : --------------------------------------\n\n", MOD);
}

void dump_spare_data (u8 * buf)
{
    u32 i = 0;
    print ("Dump spare data :\n");
    for (i = 2049; i < 2048 + 64; i++)
    {
        print ("USB buf[%d] : 0x%x\n", i, buf[i]);
    }
}
#endif


/**************************************************************************
 *  Reset Descriptor
 **************************************************************************/
void reset_dm_descriptor (void)
{
    dm_ctx.img_info.img_format = DM_IMG_FORMAT_UNKNOWN;
    dm_ctx.img_info.img_type = DM_IMG_TYPE_UNKNOWN;
    dm_ctx.img_info.img_size = 0;
    dm_ctx.img_info.addr_off = 0;
    dm_ctx.img_info.pkt_size = 0;

    dm_ctx.curr_off = 0;
    dm_ctx.page_off = 0;
    dm_ctx.curr_cnt = 0;
    dm_ctx.pkt_cnt = 0;
    dm_ctx.dm_err = DM_ERR_OK;
    dm_ctx.part_range = 0;

#if DM_CAL_CKSM_FROM_USB_BUFFER
    dm_ctx.chk_sum = 0;
#endif
}


/**************************************************************************
 *  Return Partition Name
 **************************************************************************/
u8 * get_part_name (DM_IMG_TYPE type)
{
    switch (type)
    {
    case DM_IMG_TYPE_LOGO:
        return PART_LOGO;
    case DM_IMG_TYPE_BOOTIMG:
        return PART_BOOTIMG;
    case DM_IMG_TYPE_RECOVERY:
        return PART_RECOVERY;
    case DM_IMG_TYPE_ANDROID:
        return PART_ANDSYSIMG;
    case DM_IMG_TYPE_USRDATA:
        return PART_USER;
    case DM_IMG_TYPE_UBOOT:
        return PART_UBOOT;
    case DM_IMG_TYPE_AUTHEN_FILE:
        return PART_AUTHEN_FILE;
    case DM_IMG_TYPE_SECSTATIC:
        return PART_SECSTATIC;
    case DM_IMG_TYPE_EU_FT_INFORM:
        return "EU_FT_INFORM";
    case DM_IMG_TYPE_FT_LOCK_INFORM:
        return "FT_LOCK_INFORM";
    case DM_IMG_TYPE_PT_TABLE_INFORM:
        return "PT_TABLE_INFORM";

    case DM_IMG_TYPE_CUST_IMAGE1:
        return "CUST_IMAGE1";
    case DM_IMG_TYPE_CUST_IMAGE2:
        return "CUST_IMAGE2";
    case DM_IMG_TYPE_CUST_IMAGE3:
        return "CUST_IMAGE3";
    case DM_IMG_TYPE_CUST_IMAGE4:
        return "CUST_IMAGE4";
    case DM_IMG_TYPE_UNKNOWN:
    default:
        return NULL;
    }
}

/**************************************************************************
 *  Return Partition Range
 **************************************************************************/
#ifndef FEATURE_DOWNLOAD_BOUNDARY_CHECK
u32 get_part_range (DM_IMG_TYPE img_type)
{
    part_t *part;
    blkdev_t *blkdev = blkdev_get(CFG_BOOT_DEV);
    u8 *name = get_part_name (img_type);

    DM_ASSERT (name);
    if (name == NULL)
        return 0;

    part = part_get(name);

    DM_ASSERT (part);

    return ((part->pgnum) * blkdev->blksz);
}
#endif // #ifndef FEATURE_DOWNLOAD_BOUNDARY_CHECK

/**************************************************************************
 *  Reboot
 **************************************************************************/
void do_reboot (char mode)
{
    mtk_arch_reset (mode);

    return;
}

/**************************************************************************
 *  Checksum (Received Buffer)
 **************************************************************************/
#if DM_CAL_CKSM_FROM_USB_BUFFER
u32 cal_chksum_per_pkt (u8 * pkt_buf, u32 pktsz)
{
    blkdev_t *blkdev = blkdev_get(CFG_BOOT_DEV);
    u32 i, chk_sum = dm_ctx.chk_sum;

    // skip spare because FAT format image doesn't have any spare region    
    for (i = 0; i < blkdev->blksz; i++)
        chk_sum ^= *pkt_buf++;

    dm_ctx.chk_sum = chk_sum;
    return dm_ctx.chk_sum;

}
#endif

/**************************************************************************
 *  Check Package Type
 **************************************************************************/
DM_PKT_TYPE judge_pkt_type (const void *buf)
{
    if (memcmp (buf, (const void *) DM_STR_REBOOT, DM_SZ_REBOOT_STR) == 0)
        return DM_PKT_REBT;

    if (memcmp (buf, (const void *) DM_STR_ATBOOT, DM_SZ_ATBOOT_STR) == 0)
        return DM_PKT_AUTO;

    if (memcmp (buf, (const void *) DM_STR_UPDATE, DM_SZ_UPDATE_STR) == 0)
        return DM_PKT_UPDT;
        
    if (memcmp (buf, (const void *) DM_STR_IMGP, 4) == 0)
        return DM_PKT_IMGP;

    if (memcmp (buf, (const void *) DM_STR_PLIP, 4) == 0)
        return DM_PKT_PLIP;

    if (memcmp (buf, (const void *) DM_STR_CKSM, 4) == 0)
        return DM_PKT_CKSM;

    if (memcmp (buf, (const void *) DM_STR_ERRP, 4) == 0)
        return DM_PKT_ERRP;

    if (memcmp (buf, (const void *) DM_STR_PCMD, 4) == 0)
        return DM_PKT_PCMD;

    return DM_PKT_DATA;
}


#endif /* CFG_LEGACY_USB_DOWNLOAD */
