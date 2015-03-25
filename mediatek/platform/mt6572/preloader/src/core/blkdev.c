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
#include "blkdev.h"

static blkdev_t *blkdev_list = NULL;

int blkdev_register(blkdev_t *bdev)
{
    blkdev_t *tail = blkdev_list;

    bdev->next = NULL;

    while (tail && tail->next) {
        tail = tail->next;
    }

    if (tail) {
        tail->next = bdev;
    } else {
        blkdev_list = bdev;
    }

    return 0;
}

blkdev_t *blkdev_get(u32 type)
{
    blkdev_t *bdev = blkdev_list;

    while (bdev) {
        if (bdev->type == type)
            break;
        bdev = bdev->next;
    }
    return bdev;
}

int blkdev_bread(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf)
{
    return bdev->bread(bdev, blknr, blks, buf);
}

int blkdev_bwrite(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf)
{
    return bdev->bwrite(bdev, blknr, blks, buf);
}

int blkdev_read(blkdev_t *bdev, u64 src, u32 size, u8 *dst)
{
    u8 *buf = (u8*)bdev->blkbuf;
    u32 blksz = bdev->blksz;
    u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    u32 blknr, blks;

    if (!bdev) {
        return -1;
    }

    if (size == 0) 
        return 0;

    end = src + size;

    part_start    = src &  (blksz - 1);
    part_end      = end &  (blksz - 1);
    aligned_start = src & ~(blksz - 1);
    aligned_end   = end & ~(blksz - 1);
 
	if (part_start) {
        blknr = aligned_start / blksz;	
        part_len = part_start + size > blksz ? blksz - part_start : size;
        if ((bdev->bread(bdev, blknr, 1, buf)) != 0)
            return -1;
        memcpy(dst, buf + part_start, part_len);
        dst  += part_len;
        src  += part_len;
        size -= part_len;
    }

    if (size >= blksz) {
        aligned_start = src & ~(blksz - 1);
        blknr  = aligned_start / blksz;
        blks = (aligned_end - aligned_start) / blksz;

        if (blks && 0 != bdev->bread(bdev, blknr, blks, dst))
            return -1;

        src  += (blks * blksz);
        dst  += (blks * blksz);
        size -= (blks * blksz);
    }
    if (size && part_end && src < end) {
        blknr = aligned_end / blksz;	
        if ((bdev->bread(bdev, blknr, 1, buf)) != 0)
            return -1;
        memcpy(dst, buf, part_end);
    }
    return 0;    
}

int blkdev_write(blkdev_t *bdev, u64 dst, u32 size, u8 *src)
{
    u8 *buf = (u8*)bdev->blkbuf;
    u32 blksz = bdev->blksz;
    u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    u32 blknr, blks;

    if (!bdev) {
        return -1;
    }

    if (size == 0) 
        return 0;

    end = dst + size;

    part_start    = dst &  (blksz - 1);
    part_end      = end &  (blksz - 1);
    aligned_start = dst & ~(blksz - 1);
    aligned_end   = end & ~(blksz - 1);
 
    if (part_start) {
        blknr = aligned_start / blksz;	
        part_len = part_start + size > blksz ? blksz - part_start : size;
        if ((bdev->bread(bdev, blknr, 1, buf)) != 0)
            return -1;
        memcpy(buf + part_start, src, part_len);
        if ((bdev->bwrite(bdev, blknr, 1, buf)) != 0)
            return -1;
        dst  += part_len;
        src  += part_len;
        size -= part_len;
    }

    if (size >= blksz) {
        aligned_start = dst & ~(blksz - 1);
        blknr  = aligned_start / blksz;
        blks = (aligned_end - aligned_start) / blksz;

        if (blks && 0 != bdev->bwrite(bdev, blknr, blks, src))
            return -1;

        src  += (blks * blksz);
        dst  += (blks * blksz);
        size -= (blks * blksz);
    }
    
    if (size && part_end && dst < end) {
        blknr = aligned_end / blksz;	
        if ((bdev->bread(bdev, blknr, 1, buf)) != 0)
            return -1;
        memcpy(buf, src, part_end);
        if ((bdev->bwrite(bdev, blknr, 1, buf)) != 0)
            return -1;		
    }
    return 0;
}

