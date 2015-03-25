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

#include <gui/Surface.h>
#include <gui/ISurface.h>
#include <gui/SurfaceComposerClient.h>

#define DISPLAY_WIDTH       480
#define DISPLAY_HEIGHT      800

#define LAYER_MIN_WIDTH     160
#define LAYER_MIN_HEIGHT    240

#define LAYER_MAX_WIDTH     320
#define LAYER_MAX_HEIGHT    480

#define RATIO               (((float)LAYER_MIN_HEIGHT)/LAYER_MIN_WIDTH)

#define FPS                 ((int)((1/60.0f)*1000000))
#define STEPPING            2

using namespace android;

status_t main(int argc, char** argv)
{
    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();

    // create surface
    sp<SurfaceControl> surfaceControl = client->createSurface(
            String8("test-resize2"),
            0,
            LAYER_MIN_WIDTH,
            LAYER_MIN_HEIGHT,
            PIXEL_FORMAT_RGB_565);

    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setLayer(100000);
    SurfaceComposerClient::closeGlobalTransaction();

    Parcel parcel;
    SurfaceControl::writeSurfaceToParcel(surfaceControl, &parcel);
    parcel.setDataPosition(0);
    sp<Surface> surface = Surface::readFromParcel(parcel);

    LOGD("RATIO:%f", RATIO);

    Surface::SurfaceInfo info;
    ssize_t bpr;
    while(true) {
        // zoom in
        {
            int width = LAYER_MIN_WIDTH, height;
            int x, y;
            Region *dirty = new Region();
            while(width <= LAYER_MAX_WIDTH) {
                SurfaceComposerClient::openGlobalTransaction();
                width = width + STEPPING;
                height = (int)(width * RATIO);
                surfaceControl->setSize(width, height);
                x = (int)((DISPLAY_WIDTH - width) / 2);
                y = (int)((DISPLAY_HEIGHT - height) / 2);
                surfaceControl->setPosition(x, y);
                SurfaceComposerClient::closeGlobalTransaction();

                // set dirty region before lock
                dirty->set(Rect(x, y, x + width, y + height));
                //surface->lock(&info);
                surface->lock(&info, dirty);
                bpr = info.s * bytesPerPixel(info.format);
                android_memset16((uint16_t*)info.bits, 0x07E0, bpr*info.h);
                // LOGD("(w,h,x,y,s,h,bpr):(%d,%d,%d,%d,%d,%d,%d)",
                //         width, height,
                //         x, y,
                //         info.s, info.h, (int)bpr);
                surface->unlockAndPost();

                usleep(FPS);
            }

            delete dirty;
        }

        // zoom out
        {
            int width = LAYER_MAX_WIDTH, height;
            int x, y;
            Region *dirty = new Region();
            while (width >= LAYER_MIN_WIDTH) {
                SurfaceComposerClient::openGlobalTransaction();
                width = width - STEPPING;
                height = (int)(width * RATIO);
                surfaceControl->setSize(width, height);
                x = (int)((DISPLAY_WIDTH - width) / 2);
                y = (int)((DISPLAY_HEIGHT - height) / 2);
                surfaceControl->setPosition(x, y);
                SurfaceComposerClient::closeGlobalTransaction();

                // set dirty region before lock
                dirty->set(Rect(x, y, x + width, y + height));
                //surface->lock(&info);
                surface->lock(&info, dirty);
                bpr = info.s * bytesPerPixel(info.format);
                android_memset16((uint16_t*)info.bits, 0x07E0, bpr*info.h);
                // LOGD("(w,h,x,y,s,h,bpr):(%d,%d,%d,%d,%d,%d,%d)",
                //         width, height,
                //         x, y,
                //         info.s, info.h, (int)bpr);
                surface->unlockAndPost();

                usleep(FPS);
            }

            delete dirty;
        }
        printf("\n... loop again ...\n");
    }

    IPCThreadState::self()->joinThreadPool();
    return NO_ERROR;
}   // main
