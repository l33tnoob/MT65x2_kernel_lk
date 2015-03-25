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

#define LOG_TAG "test-fence"

//#define DEBUG_ACQUIRE_FENCE_FD
#define DEBUG_RELEASE_FENCE_FD

#include <cutils/memory.h>

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
#include <gui/ISurface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <cutils/properties.h>

#include <ui/DisplayInfo.h>

#include <sync/sync.h>

using namespace android;

struct FRAME {
    char name[128];     // file name
    uint32_t w;         // width
    uint32_t s;         // stride
    uint32_t h;         // height
    uint32_t fmt;       // format
    uint32_t api;       // api connection type
};

FRAME test_frames[] = {
    {"/data/LGE.yv12", 400, 416, 240, HAL_PIXEL_FORMAT_YV12, NATIVE_WINDOW_API_MEDIA},
};

const static int TEST_FRAMES = sizeof(test_frames) / sizeof(struct FRAME);

//
// use FRAME data to dispay with an ANativeWindow
// as we postBuffer before
//
status_t showTestFrame(ANativeWindow *w, const FRAME &f) {
    char value[PROPERTY_VALUE_MAX];

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

    // set buffer rotation
    native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90);

    // set buffer count
    native_window_set_buffer_count(w, 3);

    ANativeWindowBuffer *buf;
    sp<GraphicBuffer>   gb;
    void                *ptr;
    const Rect          rect(f.w, f.h);

    int err;
    int fd = -1;

    int sync_timeline_fd = -1;

    // create timeline
    sync_timeline_fd = sw_sync_timeline_create();
    if (sync_timeline_fd < 0) {
        perror("can't create sw_sync_timeline");
        return INVALID_OPERATION;
    }

    while (true) {
        // releaseFenceFd (from HWC)
        {
            // the buffer can be dequeued before the fence signals
            err = w->dequeueBuffer(w, &buf, &fd);    // dequeue to get buffer handle
            ALOGD("dequeue fd(%d)", fd);
#ifdef DEBUG_RELEASE_FENCE_FD
            sp<Fence> fence(new Fence(fd));
            // the producer is responsible for delaying writes until it signals
            fence->waitForever(1000, LOG_TAG);
            if(err) {
                ALOGE("%s", strerror(-err));
            }
#endif

            fd = -1;
        }

        gb = new GraphicBuffer(buf, false);
        gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr);
        {
            FILE *file = fopen(f.name, "rb");                // read file into buffer

            if (file != NULL)
            {
                fread(ptr, f.s * f.h * 3 / 2, 1, file);      // bpp is 1.5
            }
            else
            {
                ALOGE("open file %s failed", f.name);
            }

            fclose(file);
        }
        gb->unlock();

        // acquireFenceFd (to HWC)
        {
#ifdef DEBUG_ACQUIRE_FENCE_FD
            // create fence & fill sync data
            static int fence_count = 1;
            fd = sw_sync_fence_create(sync_timeline_fd, "queueFence", fence_count);
            if (fd < 0) {
                printf("can't create sync point %d: %s", fence_count, strerror(errno));
                return INVALID_OPERATION;
            }
            sp<Fence> fence(new Fence(fd));
#endif
            err = w->queueBuffer(w, buf, fd);   // queue to display
#ifdef DEBUG_ACQUIRE_FENCE_FD
            ALOGD("queue fd(%d) fence_count(%d)", fd, fence_count);

            fence_count = fence_count + 1;
#endif
            fd = -1;
        }

        property_get("debug.sftest.sleep", value, "1000");
        int delay = atoi(value);
        usleep(delay * 1000);

#ifdef DEBUG_ACQUIRE_FENCE_FD
        // signal sync fence
        {
            static int signal_count = 1;

            ALOGD("increase timeline to %d", signal_count);

            int err = sw_sync_timeline_inc(sync_timeline_fd, 1);
            if (err < 0) {
                perror("can't increment sync obj:");
                return INVALID_OPERATION;
            }

            signal_count = signal_count + 1;
        }
#endif
    }

    // disconnect as unregister
    native_window_api_disconnect(w, f.api);

    // close timeline
    if (sync_timeline_fd != -1)
        close(sync_timeline_fd);

    return NO_ERROR;
}


status_t main(int argc, char** argv) {

    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    DisplayInfo dinfo;
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain);
    SurfaceComposerClient::getDisplayInfo(display, &dinfo);
    uint32_t dispw = dinfo.w;
    uint32_t disph = dinfo.h;

    ALOGD("display (w,h):(%d,%d)", dispw, disph);

    sp<SurfaceControl> surfaceControl = client->createSurface(
        String8("test-fence"), dispw - 100, disph - 100, PIXEL_FORMAT_RGB_565);
    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setPosition(50, 50);
    surfaceControl->setLayer(100000);
    SurfaceComposerClient::closeGlobalTransaction();

    // pretend it went cross-process
    Parcel parcel;
    SurfaceControl::writeSurfaceToParcel(surfaceControl, &parcel);
    parcel.setDataPosition(0);
    sp<Surface> surface = Surface::readFromParcel(parcel);
    ANativeWindow* window = surface.get();

    int i = 0;
    while(true) {
        printf("[test] %d ... ", i);
        showTestFrame(window, test_frames[i]);
        printf("done\n");

        i = i + 1;
        if (i >= TEST_FRAMES) {
            printf("\n... loop again ...\n");
            i = 0;
        }
    }

    printf("test complete. CTRL+C to finish.\n");
    IPCThreadState::self()->joinThreadPool();
    return NO_ERROR;
}
