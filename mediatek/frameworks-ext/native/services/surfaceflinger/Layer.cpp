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

#include <cutils/xlog.h>
#include <cutils/properties.h>

#include <SkImageEncoder.h>
#include <SkBitmap.h>

#include <gui/ISurfaceComposer.h>
#include <gui/BufferQueue.h>

#include "SurfaceFlinger.h"
#include "Layer.h"

#include "RenderEngine/RenderEngine.h"

#ifndef EMULATOR_SUPPORT
#include "ui/gralloc_extra.h"
#include "ui/GraphicBufferExtra.h"
#endif

#define ALIGN_CEIL(x,a) (((x) + (a) - 1L) & ~((a) - 1L))

namespace android {

void Layer::drawProtectedImage(const sp<const DisplayDevice>& hw, const Region& clip) const
{
    const State& s(getDrawingState());
    const Transform tr(hw->getTransform() * s.transform);
    const uint32_t hw_h = hw->getHeight();
    Rect win(s.active.w, s.active.h);
    if (!s.active.crop.isEmpty()) {
        win.intersect(s.active.crop, &win);
    }

    int w = win.getWidth();
    int h = win.getHeight();
    if (w > h) {
        win.left += ((w - h) / 2);
        win.right = win.left + h;
    } else {
        win.top += ((h - w) / 2);
        win.bottom = win.top + w;
    }

    Mesh::VertexArray<vec2> position(mMesh.getPositionArray<vec2>());
    position[0] = tr.transform(win.left,  win.top);
    position[1] = tr.transform(win.left,  win.bottom);
    position[2] = tr.transform(win.right, win.bottom);
    position[3] = tr.transform(win.right, win.top);
    for (size_t i=0 ; i<4 ; i++) {
        position[i].y = hw_h - position[i].y;
    }

    Mesh::VertexArray<vec2> texCoords(mMesh.getTexCoordArray<vec2>());
    texCoords[0] = vec2(0, 0);
    texCoords[1] = vec2(0, 1);
    texCoords[2] = vec2(1, 1);
    texCoords[3] = vec2(1, 0);

    RenderEngine& engine(mFlinger->getRenderEngine());
    engine.setupLayerProtectedImage();
    engine.drawMesh(mMesh);
    engine.disableBlending();
}

// dump current using buffer in Layer
void Layer::dumpActiveBuffer() const {
    XLOGV("[dumpActiveBuffer] + id=%p", this);

    if (mActiveBuffer != NULL) {
        char     value[PROPERTY_VALUE_MAX];
        bool     raw;
        const void*    identity;

        property_get("debug.sf.layerdump.raw", value, "0");
        raw = (0 != atoi(value));
        identity = this;

        char             fname[128];
        void*            ptr;
        float            bpp;
        SkBitmap         b;
        SkBitmap::Config c;

        int inputFormat = mActiveBuffer->format;
        int dumpHeight = mActiveBuffer->height;

#ifndef EMULATOR_SUPPORT
        // check private format
        if (inputFormat == HAL_PIXEL_FORMAT_YUV_PRIVATE) {
            gralloc_buffer_info_t buffInfo;
            GraphicBufferExtra::get().getBufInfo(mActiveBuffer->handle, &buffInfo);
            int fillFormat = (buffInfo.status & GRALLOC_EXTRA_MASK_CM);
            switch (fillFormat) {
                case GRALLOC_EXTRA_BIT_CM_YV12:
                    inputFormat = HAL_PIXEL_FORMAT_YV12;
                    break;
                case GRALLOC_EXTRA_BIT_CM_NV12_BLK:
                    inputFormat = HAL_PIXEL_FORMAT_NV12_BLK;
                    dumpHeight = ALIGN_CEIL(mActiveBuffer->height, 32);
                    break;
                case GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM:
                    inputFormat = HAL_PIXEL_FORMAT_NV12_BLK_FCM;
                    dumpHeight = ALIGN_CEIL(mActiveBuffer->height, 32);
                    break;
                default:
                    XLOGD("unexpected format for dumpping clear motion: 0x%x", fillFormat);
                    return;
            }
        }
#endif

        bpp = 1.0f;
        c = SkBitmap::kNo_Config;
        switch (inputFormat) {
            case PIXEL_FORMAT_RGBA_8888:
            case PIXEL_FORMAT_RGBX_8888:
                if (false == raw) {
                    c = SkBitmap::kARGB_8888_Config;
                    sprintf(fname, "/data/SF_dump/%p.png", identity);
                } else {
                    bpp = 4.0;
                    sprintf(fname, "/data/SF_dump/%p.RGBA", identity);
                }
                break;
            case PIXEL_FORMAT_BGRA_8888:
            case 0x1ff:                     // tricky format for SGX_COLOR_FORMAT_BGRX_8888 in fact
                if (false == raw) {
                    c = SkBitmap::kARGB_8888_Config;
                    sprintf(fname, "/data/SF_dump/%p(RBswapped).png", identity);
                } else {
                    bpp = 4.0;
                    sprintf(fname, "/data/SF_dump/%p.BGRA", identity);
                }
                break;
            case PIXEL_FORMAT_RGB_565:
                if (false == raw) {
                    c = SkBitmap::kRGB_565_Config;
                    sprintf(fname, "/data/SF_dump/%p.png", identity);
                } else {
                    bpp = 2.0;
                    sprintf(fname, "/data/SF_dump/%p.RGB565", identity);
                }
                break;
            case HAL_PIXEL_FORMAT_I420:
                bpp = 1.5;
                sprintf(fname, "/data/SF_dump/%p.i420", identity);
                break;
            case HAL_PIXEL_FORMAT_NV12_BLK:
                bpp = 1.5;
                sprintf(fname, "/data/SF_dump/%p.nv12_blk", identity);
                break;
            case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
                bpp = 1.5;
                sprintf(fname, "/data/SF_dump/%p.nv12_blk_fcm", identity);
                break;
            case HAL_PIXEL_FORMAT_YV12:
                bpp = 1.5;
                sprintf(fname, "/data/SF_dump/%p.yv12", identity);
                break;
            default:
                XLOGE("[%s] cannot dump format:%p for identity:%d",
                      __func__, mActiveBuffer->format, identity);
                return;
        }

        {
            //Mutex::Autolock _l(mDumpLock);
            mActiveBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &ptr);
            {
                XLOGI("[%s] %s", __func__, getName().string());
                XLOGI("    %s (config:%d, stride:%d, height:%d, ptr:%p)",
                    fname, c, mActiveBuffer->stride, dumpHeight, ptr);

                if (SkBitmap::kNo_Config != c) {
                    b.setConfig(c, mActiveBuffer->stride, dumpHeight);
                    b.setPixels(ptr);
                    SkImageEncoder::EncodeFile(fname, b, SkImageEncoder::kPNG_Type,
                                               SkImageEncoder::kDefaultQuality);
                } else {
                    uint32_t size = mActiveBuffer->stride * dumpHeight * bpp;

                    // correction for YV12 case, pending for VU planes should also pending to 16
                    if (HAL_PIXEL_FORMAT_YV12 == inputFormat) {
                        uint32_t u_remainder = (mActiveBuffer->stride / 2) % 16;
                        if (0 != u_remainder) {
                            size += (16 - u_remainder) * dumpHeight;
                        }
                    }

                    FILE *f = fopen(fname, "wb");
                    fwrite(ptr, size, 1, f);
                    fclose(f);
                }
            }
            mActiveBuffer->unlock();
        }
    }
    XLOGV("[dumpActiveBuffer] - id=%p", this);
}

};
