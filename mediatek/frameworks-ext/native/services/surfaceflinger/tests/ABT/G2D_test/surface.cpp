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

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/memory.h>

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
#include <gui/ISurface.h>
#include <gui/SurfaceComposerClient.h>

#include <SkImageEncoder.h>
#include <SkImageDecoder.h>
#include <SkBitmap.h>
#include <binder/IMemory.h>
#include <gui/ISurfaceComposer.h>
#include <string.h>
#include <unistd.h>
#include "../common/surfaceTestLib.h"

using namespace android;


struct FRAME {
    char name[128];     // file name
    uint32_t w;         // width
    uint32_t s;         // stride
    uint32_t h;         // height
    uint32_t fmt;       // format
    uint32_t api;       // api connection type
};

//#define PRODUCE_GOLDEN_MODE

#define LAYER_BASE_START 200000
#define DISPLAY_HANDLE  0

#define DRAW_FHD_W 1080
#define DRAW_FHD_H 1920


#define CAPTURE_BY_LCD

FRAME test_frames[] = {
     {"/data/doorknob480_338.png",           480,  480,  338, HAL_PIXEL_FORMAT_RGBA_8888, NATIVE_WINDOW_API_EGL},
     {"/data/ball512_512.png",               512,  512,  512, HAL_PIXEL_FORMAT_RGBA_8888, NATIVE_WINDOW_API_EGL},       
};

const static int TEST_FRAMES = sizeof(test_frames)/sizeof(FRAME);
const static int FRAME_COUNT = TEST_FRAMES + 1;
//#define FRAME_COUNT 3
//#define TEST_FRAMES 2

char cmd_path[128]={"/data/g2d_test"};

static void usage(const char* pname)
{
    fprintf(stderr,
            "usage: %s [-g] [FILENAME]\n"
            "   -g: save golden data\n"
            "If FILENAME is not given, the results will be saved to /data/g2d_test_gold.bgra.\n",
            pname
    );
}

//
// use FRAME data to dispay with an ANativeWindow
// as we postBuffer before
//
status_t showTestFrame(ANativeWindow *w, const FRAME &f, uint32_t layer_id) {

    // set api connection type as register
    native_window_api_connect(w, f.api);

    // set buffer size
    native_window_set_buffers_dimensions(w, f.w, f.h);

    // set format
    native_window_set_buffers_format(w, f.fmt);

    // set usage software write-able and hardware texture bind-able
    native_window_set_usage(w, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);

    // set scaling to match window display size
    native_window_set_scaling_mode(w, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

    // set display rotation
    // g2d no rotation native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90);

    // set buffer queue lenghth
    //native_window_set_buffer_count(w, 8);

    ANativeWindowBuffer *buf;
    sp<GraphicBuffer>   gb;
    void                *ptr;
    const Rect          rect(f.w, f.h);
    SkBitmap            hbitmap;

    w->dequeueBuffer(w, &buf);                                                                          // dequeue to get buffer handle
    gb = new GraphicBuffer(buf, false);
    gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr);
    {
        if (false == SkImageDecoder::DecodeFile(f.name, &hbitmap)) {
            printf("fail load file %s",f.name);
            return INVALID_OPERATION;
        }

        if(hbitmap.width() != (int)f.s || hbitmap.height() != (int)f.h || hbitmap.bytesPerPixel() !=4)
        {
            printf("file %s format error, w=%d, h=%d, byterperpixel=%d!, ",f.name, hbitmap.width(), hbitmap.height(), hbitmap.bytesPerPixel());
            return INVALID_OPERATION;
        }

        memcpy(ptr, hbitmap.getPixels(), hbitmap.width() * hbitmap.height() * hbitmap.bytesPerPixel());
            
    }
    gb->unlock();
    w->queueBuffer(w, buf);                                    // queue to display
#if 0
    char cmds[256];

    sprintf(cmds, "setprop debug.sf.layerdump.raw 0");
    system(cmds);
    sprintf(cmds, "setprop debug.sf.layerdump -1");
    system(cmds);
    sprintf(cmds, "dumpsys SurfaceFlinger");
    system(cmds);
    printf("layerdump done, surface ID:%d\n", layer_id);
#endif
    sleep(1);

    // disconnect as unregister
    native_window_api_disconnect(w, f.api);

    return NO_ERROR;
}

status_t createEnv(sp<SurfaceComposerClient>& client, sp<SurfaceControl>(& surfaceControls)[FRAME_COUNT], int dispw, int disph)
{
    int i=0;
    // surface 1 (LayerDim)
    surfaceControls[0] = client->createSurface(
            String8("layer_test_surface"),
            DISPLAY_HANDLE,
            DRAW_FHD_W,
            DRAW_FHD_H,
            PIXEL_FORMAT_RGBX_8888);
    
    for (i = 1; i < FRAME_COUNT; i++) {
        surfaceControls[i] = client->createSurface(
            String8("layer_test_surface"),
            DISPLAY_HANDLE,
            test_frames[i-1].w,
            test_frames[i-1].h,
            PIXEL_FORMAT_RGBA_8888);
    }
    printf("end create surface\n");

    SurfaceComposerClient::openGlobalTransaction();
    // surface 1 (background)
    surfaceControls[0]->setLayer(LAYER_BASE_START);
    surfaceControls[0]->setPosition(0, 0);
    surfaceControls[0]->setAlpha(1.0);
    
    // surace 2 - 4
    for (i = 1; i < FRAME_COUNT; i++) {
        surfaceControls[i]->setLayer(LAYER_BASE_START + i * 10);
        surfaceControls[i]->setPosition(200 - i * 40, 200 - i * 40);
        //hw no alpha surfaceControls[i]->setAlpha(0.9f-0.2f*i);
    }
    SurfaceComposerClient::closeGlobalTransaction();
    printf("end transaction\n");

    sp<Surface> s = surfaceControls[0]->getSurface();    // fill background
    Surface::SurfaceInfo      info;
    s->lock(&info);
    {        
        //memset(info.bits, 0, DRAW_FHD_W * DRAW_FHD_W * 4);
        memset(info.bits, 0, DRAW_FHD_W * DRAW_FHD_W * 4);
        /*char *ptemp = (char*)info.bits+3;
        for(i=0;i<DRAW_FHD_W * DRAW_FHD_W;i++)
        {
            *ptemp=0xFF;
            ptemp+=4;
        }*/
            
    }
    s->unlockAndPost();

    return NO_ERROR;
}

status_t getSurfaceFromCtrl(sp<SurfaceControl> (&surfaceControls)[FRAME_COUNT], sp<Surface> (&surfaces)[FRAME_COUNT])
{
    
    Parcel parcels[FRAME_COUNT]; // i = 1 means that we exclude LayerDim
    // pretend it went cross-process
    // surface 2 ~
    for (int i = 1; i < FRAME_COUNT; i++) {
        SurfaceControl::writeSurfaceToParcel(surfaceControls[i], &parcels[i]);
        parcels[i].setDataPosition(0);
        surfaces[i] = Surface::readFromParcel(parcels[i]);
    }
    return NO_ERROR;
}

status_t main(int argc, char** argv) {

    // set up the thread-pool
    const char* pname = argv[0];
    int c;
    bool saveGolden = false;
    while ((c = getopt(argc, argv, "gh")) != -1) {
        switch (c) {
            case 'g':
                saveGolden = true;
                break;
            case '?':
            case 'h':
                usage(pname);
                return 1;
        }
    }
    printf("saveGolden =%d,argc=%d,optind=%d\n",saveGolden,argc,optind);
    argc -= optind;
    argv += optind;
    
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    uint32_t dispw = client->getDisplayWidth(0);
    uint32_t disph = client->getDisplayHeight(0);
    int i=0;

    sp<SurfaceControl> surfaceControls[FRAME_COUNT];

    createEnv(client, surfaceControls, dispw, disph);

    sp<Surface> surfaces[FRAME_COUNT];  // 1 = 1 means that exclude LayerDim

    getSurfaceFromCtrl(surfaceControls, surfaces);

    for (i = 0; i < TEST_FRAMES; i++) {

        printf("display %d ... ", i);
        showTestFrame(surfaces[i+1].get(), test_frames[i], surfaceControls[i+1]->getIdentity());
    }
    printf("\n");

    captureScreen(saveGolden, argv[0], cmd_path);
    
    if(!saveGolden)
    {
        bool result = doCheck(dispw*disph*4, cmd_path);
        printf("check done\n\n");

        if(true != result) {
            printf("test case fail\n");
        } else {
            printf("test case pass\n");
        }
    }

    return NO_ERROR;
}
