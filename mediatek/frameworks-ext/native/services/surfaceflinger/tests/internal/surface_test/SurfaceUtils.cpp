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
        int i, j;
        for (i = 0; i < LAYER_HEIGHT; i++) {
            for (j = 0; j < (LAYER_WIDTH * BPP); j++) {
                memset(static_cast<void *>(&mRGBAImg[i][j]),
                        (int)0x00,
                        (size_t)(sizeof(uint8_t)));
            }
        }
        ALOGD("buffer size: %d", sizeof(uint32_t) * LAYER_WIDTH * LAYER_HEIGHT);
        ALOGD("!Surfaceutils(i, j)=(%d,%d)",i,j);
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

    void SurfaceUtils::showTestFrame(int idx) {
        ANativeWindowBuffer *buf;
        void *ptr;
        const Rect rect(LAYER_WIDTH, LAYER_HEIGHT);

        // set buffer size
        //native_window_set_buffers_dimensions(window, frame.w, frame.h);
        setBuffersDimensions(LAYER_WIDTH, LAYER_HEIGHT);
        setBufferCount(10);

        // set format only to YV12
        //native_window_set_buffers_format(window, HAL_PIXEL_FORMAT_YV12);

        // set usage software write-able and hardware texture bind-able
        setUsage(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);

        // set scaling to match window display size
        //setScalingMode(NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

        // set transform
        switch(idx) {
            case 0:
                ALOGD("layer no rotation");
                break;
            case 1:
                ALOGD("layer %d: ROT_90", idx);
                setBuffersTransform(HAL_TRANSFORM_ROT_90);
                break;
            case 2:
                ALOGD("layer %d: ROT_180", idx);
                setBuffersTransform(HAL_TRANSFORM_ROT_180);
                break;
            default:
                ALOGD("layer %d: ROT_270", idx);
                setBuffersTransform(HAL_TRANSFORM_ROT_270);
        }

        // get the buffer handle
        int fence;
        mWindow->dequeueBuffer(mWindow, &buf, &fence);
        // get the buffer addr
        // ALOGD("lock GraphicBuffer: %d", idx_frame);
        GraphicBufferMapper::getInstance().lock(buf->handle,
                GRALLOC_USAGE_SW_WRITE_OFTEN,
                rect,
                &ptr);
        /*
           ALOGD("(w,h,s,f)=(%d,%d,%d,%d)",
           buf->width, buf->height, buf->stride,
           buf->format);
         */
        // fill buffer
        for (int j = 0; j < LAYER_HEIGHT; j++) {
            //for (int i = 0; i < LAYER_WIDTH; i++) {

                memcpy(ptr, static_cast<void *>(mRGBAImg[j]),
                        (size_t)(sizeof(uint32_t) * LAYER_WIDTH));
            //}
            ptr = (uint8_t*)ptr + buf->stride * BPP; // stride * bpp

        }
        // call unlock after finishing changes against the buffer
        //ALOGD("unlock GraphicBuffer");
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        // transfer the ownership back to the server
        mWindow->queueBuffer(mWindow, buf, fence);
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
        //ALOGD("setColor: (%d,%d,%d,%d)", color->r, color->g, color->b, color->a);
        //ALOGD("!setColor(i, j)=(%d,%d)",i,j);
    }

    void SurfaceUtils::setAlpha(const Rect rect, float alpha) {
        int i = 0, j = 0;
        int _alpha = (alpha * 255) < 255 ? alpha * 255 : 255;
        const int W = rect.width();
        const int H = rect.height();
        uint8_t *addr;
        const int X = rect.leftTop().x;
        const int Y = rect.leftTop().y;
        /*
        ALOGD("(x,y,w,h,alpha)=(%d,%d,%d,%d,%d)",
        X, Y,
        W, H,
        _alpha);
        */

        for (i = 0; i < H; i++) {
            for (j = 0; j < (W * BPP); j = j + BPP) {
//                addr = &mRGBAImg[i+leftTop.y][j+leftTop.x * 4];
                addr = &mRGBAImg[Y + i][X * BPP + j] + 3;
                //memset(static_cast<void *>(addr + 3),
                //        (int)0x11,
                //        (size_t)(sizeof(uint8_t)));
                *addr = _alpha;
            }
        }

        //ALOGD("!setAlpha(i, j, alpha)=(%d,%d,%d)",i,j,_alpha);
        /*
           for (i = 0; i < LAYER_WIDTH * BPP; i++) {
           ALOGD("pixel value:0x%x", mRGBAImg[0][i]);
           }
         */
    }
}; // namespace android
