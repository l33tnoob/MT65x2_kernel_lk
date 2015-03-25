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


using namespace android;

// # of buffer
#define FRAME_COUNT    5

rgba_t color[FRAME_COUNT];

static void initFrames()
{
    for (int i = 0; i < FRAME_COUNT; i++) {
        color[i].r = 255;
        color[i].g = i * 10;
        color[i].b = i * 20;
        color[i].a = 255;
    }
}


status_t main(int argc, char** argv)
{
    // random
    srand(time(NULL));

    // create our thread pool
    sp<ProcessState> proc = ProcessState::self();
    ProcessState::self()->startThreadPool();
    // one layer per client
    sp<SurfaceComposerClient> client[FRAME_COUNT];
    for (int i = 0; i < FRAME_COUNT; i++) {
        client[i] = new SurfaceComposerClient();
    }
    ALOGD("end create client");


    sp<SurfaceControl> surfaceControls[FRAME_COUNT];
    // surface 1 (LayerDim)
    surfaceControls[0] = client[0]->createSurface(
            String8("test-surface2"),
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT,
            PIXEL_FORMAT_RGBA_8888,
            ISurfaceComposerClient::eFXSurfaceDim & ISurfaceComposerClient::eFXSurfaceMask);
    // surface 2 ~
    for (int i = 1; i < FRAME_COUNT; i++) {
        surfaceControls[i] = client[i]->createSurface(
            String8("test-surface2"),
            LAYER_WIDTH,
            LAYER_HEIGHT,
            PIXEL_FORMAT_RGBA_8888);
    }
    ALOGD("end create surface");

    SurfaceComposerClient::openGlobalTransaction();
    // surface 1 (LayerDim)
    surfaceControls[0]->setLayer(100000);
    surfaceControls[0]->setPosition(0, 0);
    surfaceControls[0]->setAlpha(0.6f);    // MUST HAVE
    // surace 2 - 5
    for (int i = 1; i < FRAME_COUNT; i++) {
        surfaceControls[i]->setLayer(101000 + i * 10);
        surfaceControls[i]->setPosition(100 + i * 20, 100 + i * 40);
    }
    SurfaceComposerClient::closeGlobalTransaction();
    ALOGD("end transaction");

    Parcel parcels[FRAME_COUNT]; // i = 1 means that we exclude LayerDim
    sp<Surface> surfaces[FRAME_COUNT];  // 1 = 1 means that exclude LayerDim
    // pretend it went cross-process
    // surface 2 ~
    for (int i = 1; i < FRAME_COUNT; i++) {
        SurfaceControl::writeSurfaceToParcel(surfaceControls[i], &parcels[i]);
        parcels[i].setDataPosition(0);
        surfaces[i] = Surface::readFromParcel(parcels[i]);
    }
    ALOGD("end IPC");

    // surface 2 ~
    SurfaceUtils surface_utils[FRAME_COUNT];  // i = 1 means that we exclude LayerDim
    ANativeWindow *windows[FRAME_COUNT];    // i = 1 means that we exclude LayerDim
    for (int i = 1; i < FRAME_COUNT; i++) {
        windows[i] = surfaces[i].get();
        ALOGD("window = %p\n", windows[i]);
        surface_utils[i].setWindow(windows[i]);
    }

    // set api connection type
    // exclude LayerDim
    for (int i = 1; i < FRAME_COUNT; i++) {
        surface_utils[i].connectAPI(NATIVE_WINDOW_API_CPU);
    }

    // initialize input frames
    initFrames();

    // display
    int loop = 0;
    float dynamic_alpha = (rand() % 11) / 10.0f;
    float stepping = 0.02f;
    while (true) {
        // surface 2 ~
        for (int i = 1; i < FRAME_COUNT; i++) {
            surface_utils[i].setColor(&color[i]);
            surface_utils[i].setAlpha(Rect(10, 10, 200, 40), 0.3f);  // translucent

            if (dynamic_alpha + stepping > 1.0f || dynamic_alpha + stepping < 0.0f) {
                stepping = -stepping;
            }
            dynamic_alpha = dynamic_alpha + stepping;
            surface_utils[i].setAlpha(Rect(110, 120, LAYER_WIDTH - 20, 160), dynamic_alpha); // dynamic alpha
            surface_utils[i].setAlpha(Rect(80, 180, LAYER_WIDTH - 120, LAYER_WIDTH - 20), 0.0f); // transparent
            // show frame
            surface_utils[i].showTestFrame(i);
        }
        usleep(16667); // fsp = 60

        loop = loop + 1;
        if (loop == FRAME_COUNT) {
           // printf("\nloop again...\n");
            loop = 0;
        }
    };

    printf("test complete. CTRL+C to finish.\n");
    IPCThreadState::self()->joinThreadPool();
    return NO_ERROR;
}
