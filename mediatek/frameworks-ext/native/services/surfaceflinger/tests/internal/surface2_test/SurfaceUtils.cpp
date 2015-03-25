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

#include "SurfaceUtils.h"

namespace android {
    SurfaceUtils::SurfaceUtils()
    {
        // random
        srand(time(NULL));
        mDynamicAlpha = (rand() % 11) / 10.0f;
        mStepping = 0.02f;
    }
    // ref: android_view_Surface.cpp
    static inline SkBitmap::Config convertPixelFormat(PixelFormat format)
    {
        /* note: if PIXEL_FORMAT_RGBX_8888 means that all alpha bytes are 0xFF, then
           we can map to SkBitmap::kARGB_8888_Config, and optionally call
           bitmap.setIsOpaque(true) on the resulting SkBitmap (as an accelerator)
         */
        switch (format) {
            case PIXEL_FORMAT_RGBX_8888:    return SkBitmap::kARGB_8888_Config;
            case PIXEL_FORMAT_RGBA_8888:    return SkBitmap::kARGB_8888_Config;
            case PIXEL_FORMAT_RGBA_4444:    return SkBitmap::kARGB_4444_Config;
            case PIXEL_FORMAT_RGB_565:      return SkBitmap::kRGB_565_Config;
            case PIXEL_FORMAT_A_8:          return SkBitmap::kA8_Config;
            default:                        return SkBitmap::kNo_Config;
        }
    }

    void SurfaceUtils::connectAPI(int api) {
        native_window_api_connect(mWindow, api);
    }

    void SurfaceUtils::disconnectAPI(int api) {
        native_window_api_disconnect(mWindow, api);
    }
    void SurfaceUtils::setBufferCount(size_t bufferCount) {
        native_window_set_buffer_count(mWindow, bufferCount);
    }

    void SurfaceUtils::setBuffersFormat(int format) {
        native_window_set_buffers_format(mWindow, format);
    }

    void SurfaceUtils::setBuffersDimensions(int w, int h) {
        native_window_set_buffers_dimensions(mWindow, w, h);
    }

    void SurfaceUtils::setBuffersTransform(int transform) {
        native_window_set_buffers_transform(mWindow, transform);
    }

    void SurfaceUtils::setScalingMode(int mode) {
        native_window_set_scaling_mode(mWindow, mode);
    }

    void SurfaceUtils::setCrop(android_native_rect_t const * crop) {
        native_window_set_crop(mWindow, crop);
    }

    void SurfaceUtils::setUsage(int usage) {
        native_window_set_usage(mWindow, usage);
    }

    void SurfaceUtils::showTestFrame(const frame_t *f, int idx) {
        ANativeWindowBuffer *buf;
        void *ptr;
        const Rect rect(LAYER_WIDTH, LAYER_HEIGHT);

        // set buffer size
        setBuffersDimensions(LAYER_WIDTH, LAYER_HEIGHT);

        // set usage software write-able and hardware texture bind-able
        setUsage(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);

        // set scaling to match window display size
        setScalingMode(NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

        // set transform
        switch(idx) {
            case 0:
        //        LOGD("layer no rotation");
                break;
            case 1:
        //        LOGD("layer %d: ROT_90", idx);
                setBuffersTransform(HAL_TRANSFORM_ROT_90);
                break;
            case 2:
        //        LOGD("layer %d: ROT_180", idx);
                setBuffersTransform(HAL_TRANSFORM_ROT_180);
                break;
            default:
        //        LOGD("layer %d: ROT_270", idx);
                setBuffersTransform(HAL_TRANSFORM_ROT_270);
        }

        // get the buffer handle
        mWindow->dequeueBuffer(mWindow, &buf);
        // get the buffer addr
        // LOGD("lock GraphicBuffer: %d", idx_frame);
        GraphicBufferMapper::getInstance().lock(buf->handle,
                GRALLOC_USAGE_SW_WRITE_OFTEN,
                rect,
                &ptr);
        /*
        LOGD("(w,h,s,f)=(%d,%d,%d,%d)",
                buf->width, buf->height, buf->stride,
                buf->format);
         */
        // fill buffer
        //LOGD("load bitmap");
        SkBitmap bitmap;
        ssize_t bpr = buf->stride * BPP;
        bitmap.setConfig(convertPixelFormat(buf->format), buf->width, buf->height, bpr);
        SkImageDecoder::DecodeFile(f->name, &bitmap,
                SkBitmap::kARGB_8888_Config,
                SkImageDecoder::kDecodePixels_Mode, NULL);
        int w = bitmap.width();
        int h = bitmap.height();
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                memcpy((uint8_t *)ptr + (j * bpr) + (i * BPP),
                        bitmap.getAddr32(i, j),
                        sizeof(uint32_t));
            }
        }

        // set transparent/translucent region
        setAlpha((uint8_t *)ptr, &f->blk[0], buf, false);
        setAlpha((uint8_t *)ptr, &f->blk[1], buf, true);
        //LOGD("end fill buffer");

        // call unlock after finishing changes against the buffer
        //LOGD("unlock GraphicBuffer");
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        // transfer the ownership back to the server
        mWindow->queueBuffer(mWindow, buf);
    }

    void SurfaceUtils::setColor(rgba_t *color) {
        int i, j;
        uint8_t *addr;

        for (i = 0; i < LAYER_HEIGHT; i++) {
            for (j = 0; j < (LAYER_WIDTH * BPP); j = j + BPP) {
                addr = &mRGBAImg[i][j];
                memset(static_cast<void *>(addr),
                        color->r,
                        (size_t)(sizeof(uint8_t)));
                memset(static_cast<void *>(addr + 1),
                        color->g,
                        (size_t)(sizeof(uint8_t)));
                memset(static_cast<void *>(addr + 2),
                        color->b,
                        (size_t)(sizeof(uint8_t)));
                memset(static_cast<void *>(addr + 3),
                        color->a,
                        (size_t)(sizeof(uint8_t)));
            }
        }
        //LOGD("setColor: (%d,%d,%d,%d)", color->r, color->g, color->b, color->a);
        //LOGD("!setColor(i, j)=(%d,%d)",i,j);
    }

    void SurfaceUtils::setAlpha(uint8_t *addr, const block_t *blk, ANativeWindowBuffer *buf, bool isDynamic) {
        int _alpha = (blk->alpha * 255) < 255 ? blk->alpha * 255 : 255;
        const int W = blk->rect.width();
        const int H = blk->rect.height();
        uint8_t *addr_alpha;
        const int X = blk->rect.leftTop().x;
        const int Y = blk->rect.leftTop().y;
        const int BPR = buf->stride * BPP;  // bytes per row
        /*
        LOGD("(x,y,w,h,alpha)=(%d,%d,%d,%d,%d)",
        X, Y,
        W, H,
        _alpha);
        */
        if (isDynamic) {
            if (mDynamicAlpha + mStepping > 1.0f || mDynamicAlpha + mStepping < 0.0f) {
                mStepping = -mStepping;
            }
            mDynamicAlpha = (mDynamicAlpha + mStepping);
            //        LOGD("mDynamicAlpha:%f", mDynamicAlpha);
        }

        for (int j = 0; j < H; j++) {
            for (int i = 0; i < W; i++) {
                if (mDynamicAlpha == 0.0f || _alpha == 0) {
                    // set all color channels to 0
                    addr_alpha = addr + ((Y + j) * BPR) + ((X + i) * BPP);
                    memset(addr_alpha, 0, sizeof(uint32_t));
                }
                else {
                    addr_alpha = addr + ((Y + j) * BPR) + ((X + i) * BPP) + 3;
                    if (isDynamic) {
                        *addr_alpha = (int)(mDynamicAlpha * 255);
                    }
                    else {
                        *addr_alpha = _alpha;
                    }
                }
            }
        }

    }
}; // namespace android
