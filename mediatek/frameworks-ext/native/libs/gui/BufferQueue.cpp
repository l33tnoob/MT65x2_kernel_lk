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

#include <gui/BufferQueue.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>

#include <math.h>

namespace android {

status_t BufferQueue::DrawDebugLineToGraphicBuffer(sp<GraphicBuffer> gb, uint32_t cnt, uint8_t val) {
    if (gb == NULL) {
        return INVALID_OPERATION;
    }

    status_t lockret;
    uint8_t *ptr;

    char value[PROPERTY_VALUE_MAX];
    property_get("debug.bq.linecnt", value, "-1");
    int linecnt = atoi(value);
    if (linecnt >= 0)
        cnt = linecnt;

    property_get("debug.bq.line_wp", value, "-1");
    uint32_t line_wp = atoi(value);
    if (line_wp > 5)
        line_wp = 5;
    uint32_t line_w = pow(2, 5);
    
    lockret = gb->lock(LOCK_FOR_SW, (void**)&ptr);
    if (NO_ERROR != lockret) {
        XLOGE("[%s] buffer lock fail: %s (gb:%p, handle:%p)",
            __func__, strerror(lockret), gb.get(), gb->handle);
        return INVALID_OPERATION;
    } else {
        uint32_t bsize; // block size, will split intrested plane to 32 parts

        cnt &= (line_w - 1);    // mod count by 32
        switch (gb->format) {

            // for 32bit format
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
                bsize = (gb->stride * gb->height * 4) >> line_wp;
                memset(ptr + (bsize * cnt), val, bsize);
                break;

            // for 16bits format
            case HAL_PIXEL_FORMAT_RGB_565:
                bsize = (gb->stride * gb->height * 2) >> line_wp;
                memset(ptr + (bsize * cnt), val, bsize);
                break;

            // for general YUV format, draw Y plane only
            case HAL_PIXEL_FORMAT_I420:
            case HAL_PIXEL_FORMAT_NV12_BLK:
            case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
            case HAL_PIXEL_FORMAT_YV12:
            case HAL_PIXEL_FORMAT_YUV_PRIVATE:
                bsize = (gb->stride * gb->height) >> line_wp;
                memset(ptr + (bsize * cnt), val, bsize);
                break;

            default:
                XLOGE("[%s] unsupported format:%d", __func__, gb->format);
        }
    }
    gb->unlock();

    return NO_ERROR;
}

status_t BufferQueue::getProcessName(int pid, String8& name) {
    FILE *fp = fopen(String8::format("/proc/%d/cmdline", pid), "r");
    if (NULL != fp) {
        const size_t size = 64;
        char proc_name[size];
        fgets(proc_name, size, fp);
        fclose(fp);

        name = proc_name;
        return NO_ERROR;
    }

    return INVALID_OPERATION;
}

}

