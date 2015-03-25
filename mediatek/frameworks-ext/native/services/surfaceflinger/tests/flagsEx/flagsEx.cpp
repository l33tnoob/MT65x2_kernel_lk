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


status_t main(int argc, char** argv) {

    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    uint32_t dispw = client->getDisplayWidth(0);
    uint32_t disph = client->getDisplayHeight(0);
    
    sp<SurfaceControl> surfaceControl = client->createSurface(
        String8("test-flagsEx"), 0, dispw - 100, disph - 100, PIXEL_FORMAT_RGBA_8888);
    SurfaceComposerClient::openGlobalTransaction();
    {
        surfaceControl->setPosition(50, 50);
        surfaceControl->setLayer(100000);
    }
    SurfaceComposerClient::closeGlobalTransaction();

    sp<Surface> surface = surfaceControl->getSurface();
    Surface::SurfaceInfo i;
    surface->lock(&i);
    {
        memset(i.bits, 0x80, i.s * i.h * 4);    // draw to grey
    }
    surface->unlockAndPost();

    SurfaceComposerClient::openGlobalTransaction();
    {
        surfaceControl->setFlagsEx(0xaaaaaaEF, 0x000000ff);
    }
    SurfaceComposerClient::closeGlobalTransaction();
    printf("--- [ BYTE 0 ] ---\n");
    system("dumpsys SurfaceFlinger");
    sleep(3);

    SurfaceComposerClient::openGlobalTransaction();
    {
        surfaceControl->setFlagsEx(0xbbbbBEbb, 0x0000ff00);
    }
    SurfaceComposerClient::closeGlobalTransaction();
    printf("--- [ BYTE 1 ] ---\n");
    system("dumpsys SurfaceFlinger");
    sleep(3);

    SurfaceComposerClient::openGlobalTransaction();
    {
        surfaceControl->setFlagsEx(0xccADcccc, 0x00ff0000);
    }
    SurfaceComposerClient::closeGlobalTransaction();
    printf("--- [ BYTE 2 ] ---\n");
    system("dumpsys SurfaceFlinger");
    sleep(3);

    SurfaceComposerClient::openGlobalTransaction();
    {
        surfaceControl->setFlagsEx(0xDEdddddd, 0xff000000);
    }
    SurfaceComposerClient::closeGlobalTransaction();
    printf("--- [ BYTE 3 ] ---\n");
    system("dumpsys SurfaceFlinger");
    sleep(3);

    SurfaceComposerClient::openGlobalTransaction();
    {
        surfaceControl->setFlagsEx(0xFACEBAAD, 0xffffffff);
    }
    SurfaceComposerClient::closeGlobalTransaction();
    printf("--- [ BYTE ALL ] ---\n");
    system("dumpsys SurfaceFlinger");
    sleep(3);



    printf("test complete. CTRL+C to finish.\n");
    IPCThreadState::self()->joinThreadPool();
    return NO_ERROR;
}
