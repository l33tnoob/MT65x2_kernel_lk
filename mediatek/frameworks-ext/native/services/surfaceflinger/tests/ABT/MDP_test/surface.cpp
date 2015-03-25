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


using namespace android;

struct FRAME {
    char name[128];     // file name
    uint32_t w;         // width
    uint32_t s;         // stride
    uint32_t h;         // height
    uint32_t fmt;       // format
    uint32_t api;       // api connection type
};

#define TEST_FRAMES 1
FRAME test_frames[TEST_FRAMES] = {
    {"/data/prv03.bin", 640, 640, 480, HAL_PIXEL_FORMAT_I420, NATIVE_WINDOW_API_CAMERA},
};

status_t doCheck(uint32_t layer_id, const FRAME &f) {
    status_t err = NO_ERROR;
    int SIZE = f.w * f.h * 5;  // UYVY

    printf("file size:%d\n", SIZE);

    uint8_t *gold = NULL, *sample = NULL;
    FILE *file = NULL;
    char cmds[256];
    int i = 0;

    sprintf(cmds, "/data/gold.i420");
    printf("gold file:%s\n", cmds);

    file = fopen(cmds, "rb");

    if(NULL != file) {
        gold = (uint8_t *)malloc(sizeof(uint8_t) * SIZE);
        fread(gold, SIZE, 1, file);
        fclose(file);
    } else {
        printf("file open fail:%s\n", strerror(errno));
        free(gold);
        return BAD_VALUE;
    }

    printf("read gold pattern done\n");

    sprintf(cmds, "/data/SF_dump/[mdp]%u.i420", layer_id);
    printf("output path:%s\n", cmds);

    file = fopen(cmds, "rb");

    if(NULL != file) {
        sample = (uint8_t *)malloc(sizeof(uint8_t) * SIZE);
        fread(sample, SIZE, 1, file);
        fclose(file);
    } else {
        printf("file open fail:%s\n", strerror(errno));
        free(sample);
        return BAD_VALUE;
    }

    printf("read output pattern done\n");

    uint8_t *f1 = gold, *f2 = sample;

    while (i++ < SIZE) {
        if(*f1 != *f2) {
            err = BAD_VALUE;
            break;
        }
        f1++;
        f2++;
    }

    free(gold);
    free(sample);

    return err;
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
    native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90);

    // set buffer queue lenghth
    //native_window_set_buffer_count(w, 8);

    ANativeWindowBuffer *buf;
    sp<GraphicBuffer>   gb;
    void                *ptr;
    const Rect          rect(f.w, f.h);

    w->dequeueBuffer(w, &buf);                                                                          // dequeue to get buffer handle
    gb = new GraphicBuffer(buf, false);
    gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr);
    {
        FILE *file = fopen(f.name, "rb");                // read file into buffer
        {
            fread(ptr, f.s * f.h * 3 / 2, 1, file);      // bpp is 1.5
        }
        fclose(file);
    }
    gb->unlock();
    w->queueBuffer(w, buf);                                    // queue to display

    char cmds[256];

    sprintf(cmds, "setprop debug.sf.layerdump.raw 0");
    system(cmds);
    sprintf(cmds, "setprop debug.sf.layerdump -1");
    system(cmds);
    sprintf(cmds, "dumpsys SurfaceFlinger");
    system(cmds);
    printf("layerdump done, surface ID:%d\n", layer_id);

    sleep(1);

    // disconnect as unregister
    native_window_api_disconnect(w, f.api);

    return NO_ERROR;
}


status_t main(int argc, char** argv) {

    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    uint32_t dispw = client->getDisplayWidth(0);
    uint32_t disph = client->getDisplayHeight(0);

    sp<SurfaceControl> surfaceControl = client->createSurface(
            String8("test-surface"), 0, 480, 640, PIXEL_FORMAT_RGB_565);
    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setPosition(0, 0);
    surfaceControl->setLayer(100000);
    SurfaceComposerClient::closeGlobalTransaction();

    // pretend it went cross-process
    Parcel parcel;
    SurfaceControl::writeSurfaceToParcel(surfaceControl, &parcel);
    parcel.setDataPosition(0);
    sp<Surface> surface = Surface::readFromParcel(parcel);
    ANativeWindow* window = surface.get();

    uint32_t layerID = surfaceControl->getIdentity();

    int32_t i = 0;
    char value[20];
    if(true) {
        i = 0;

        printf("[test] %d ... ", i);
        showTestFrame(window, test_frames[i], layerID);

        // check Kimono
        if (!strcmp("/data/prv03.bin", test_frames[i].name)) {


            printf("start check\n");

            status_t err = doCheck(layerID, test_frames[i]);

            printf("check done\n\n");

            if(NO_ERROR != err) {
                printf("test case fail\n");
            } else {
                printf("test case pass\n");
            }
        }

        if (i >= TEST_FRAMES) {
            printf("\n... loop again ...\n");
            i = 0;
        }
    }

    return NO_ERROR;
}
