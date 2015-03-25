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

#ifndef MTK_SURFACE_UTILS_H
#define MTK_SURFACE_UTILS_H

#include <time.h>

#include <ui/GraphicBufferMapper.h>

#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>

#include <gui/ISurface.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#include <utils/RefBase.h>
#include "SkImageDecoder.h"

#undef  LOG_TAG
#define LOG_TAG "surface_test"

#define DISPLAY_HANDLE  0
// WVGA
#define DISPLAY_WIDTH   480
#define DISPLAY_HEIGHT  800
// layer size
#define LAYER_WIDTH     300
#define LAYER_HEIGHT    300

#define BPP             4 // byte per pixel


using namespace android;

typedef struct block {
    Rect rect;
    float alpha;
} block_t;

typedef struct frame {
    int     api_type;   // type of client API
    char    name[128];  // file name

    block_t blk[2];     // transparent and translucent blocks
} frame_t;

typedef struct rgba {
    int r;
    int g;
    int b;
    int a;
} rgba_t;


namespace android {

    class SurfaceUtils : public virtual RefBase
    {
        static const int IMG_ARRAY_SIZE = ((LAYER_WIDTH * LAYER_HEIGHT) * BPP);
        ANativeWindow *mWindow;

        uint8_t mRGBAImg[LAYER_HEIGHT][LAYER_WIDTH * BPP];
        float mDynamicAlpha;
        float mStepping;

    public:
        SurfaceUtils();

        void setWindow(ANativeWindow *window) {  mWindow = window;  };

        void connectAPI(int api);
        void disconnectAPI(int api);

        void setBufferCount(size_t bufferCount);

        void setBuffersFormat( int format);
        void setBuffersDimensions(int w, int h);
        void setBuffersTransform(int transform);

        void setScalingMode(int mode);
        void setCrop(android_native_rect_t const * crop);
        void setUsage(int usage);

        void setColor(rgba_t *color);
        void setAlpha(uint8_t *addr, const block_t *blk, ANativeWindowBuffer *buf, bool isDynamic = false);

        void showTestFrame(const frame_t *f, int idx = 0);
    }; // class SurfaceUtils

}; // namespace android

#endif // MTK_SURFACE_UTILS_H
