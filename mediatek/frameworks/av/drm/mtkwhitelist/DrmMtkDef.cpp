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

#define LOG_NDEBUG 1
#define LOG_TAG "DrmMtkUtil/WhiteList"
#include <utils/Log.h>

#include <drm/DrmMtkDef.h>
#include <utils/String8.h>

using namespace android;

const int DrmTrustedApp::TRUSTED_APP_CNT;
const char* DrmTrustedApp::TRUSTED_APP[TRUSTED_APP_CNT] = {
    "com.android.gallery3d",
    "com.mediatek.videoplayer",
    "com.mediatek.videoplayer2",
    "com.android.music",
    "com.android.phone",
    "com.android.settings",
    "com.android.deskclock",
    "android.process.media",
    "com.android.systemui",
    "com.mediatek.drmframeworktest", // added for drm test case
    "com.google.android.xts.media",
    "com.widevine.demo",
    "com.discretix.drmassist",  // add for PlayReady
};

bool DrmTrustedApp::IsDrmTrustedApp(const String8& procName) {
    bool result = false;
    for (int i = 0; i < DrmTrustedApp::TRUSTED_APP_CNT; i++) {
        ALOGD("IsDrmTrustedApp: compare [%s] with [%s].",
                DrmTrustedApp::TRUSTED_APP[i],
                procName.string());

        if (0 == strcmp(DrmTrustedApp::TRUSTED_APP[i], procName.string())) {
            ALOGD("IsDrmTrustedApp: accepted.");
            result = true;
            break;
        }
    }
    return result;
}

const int DrmTrustedClient::TRUSTED_PROC_CNT;
const char* DrmTrustedClient::TRUSTED_PROC[TRUSTED_PROC_CNT] = {
    "com.android.gallery", // gallery 2d
    "com.android.gallery:CropImage",
    "com.cooliris.media",  // gallery 3d (2.3)
    "android.process.media",
    "com.mediatek.drmfileinstaller",
    "com.android.phone",   // ringtone playing
    "com.android.gallery3d", // gallery (4.0)
    "com.android.gallery3d:crop",
    "com.mediatek.drmframeworktest", // added for drm test case
    "com.google.android.xts.media",
    "com.widevine.demo",
    "com.android.launcher3:wallpaper_chooser",  //KK add
    "com.discretix.drmassist",// add for PlayReady
};

bool DrmTrustedClient::IsDrmTrustedClient(const String8& procName) {
    bool result = false;
    for (int i = 0; i < DrmTrustedClient::TRUSTED_PROC_CNT; i++) {
        ALOGD("IsDrmTrustedClient: compare [%s] with [%s].",
                DrmTrustedClient::TRUSTED_PROC[i],
                procName.string());

        if (0 == strcmp(DrmTrustedClient::TRUSTED_PROC[i], procName.string())) {
            ALOGD("IsDrmTrustedClient: accepted.");
            result = true;
            break;
        }
    }
    return result;
}

const int DrmTrustedVideoApp::TRUSTED_VIDEO_APP_CNT;
const char* DrmTrustedVideoApp::TRUSTED_VIDEO_APP[TRUSTED_VIDEO_APP_CNT] = {
    "com.android.gallery3d",
    "com.mediatek.videoplayer",
    "com.mediatek.videoplayer2"
};

bool DrmTrustedVideoApp::IsDrmTrustedVideoApp(const String8& procName) {
    bool result = false;
    for (int i = 0; i < DrmTrustedVideoApp::TRUSTED_VIDEO_APP_CNT; i++) {
        ALOGD("IsDrmTrustedVideoApp: compare [%s] with [%s].",
                DrmTrustedVideoApp::TRUSTED_VIDEO_APP[i],
                procName.string());

        if (0 == strcmp(DrmTrustedVideoApp::TRUSTED_VIDEO_APP[i], procName.string())) {
            ALOGD("DrmTrustedVideoApp: accepted.");
            result = true;
            break;
        }
    }
    return result;
}

const char* DrmSntpServer::NTP_SERVER_1 = "hshh.org";
const char* DrmSntpServer::NTP_SERVER_2 = "t1.hshh.org";
const char* DrmSntpServer::NTP_SERVER_3 = "t2.hshh.org";
const char* DrmSntpServer::NTP_SERVER_4 = "t3.hshh.org";
const char* DrmSntpServer::NTP_SERVER_5 = "clock.via.net";

