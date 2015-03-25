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

#ifndef __DRM_MTK_DEF_H__
#define __DRM_MTK_DEF_H__

#include <utils/String8.h>
#define NTP_SERVER_CNT 5

namespace android {

const unsigned int OMADrmFlag = 0x80; // 128

/*
* this defines those process that can access directly the decrypt content
* of OMA DRM v1 protected (by encryption) files.
*/
class DrmTrustedClient {
private:
    DrmTrustedClient();

public:
    static bool IsDrmTrustedClient(const String8& procName);

public:
    static const int TRUSTED_PROC_CNT = 13;
    static const char* TRUSTED_PROC[TRUSTED_PROC_CNT];
};

/*
* this defines those process that can launch playback for those protected content
* of OMA DRM v1 while using default mediaplayer.
* the process which need to play OMA DRM v1 content (video/audio), and is using
* MediaPlayer instance, should be added to this white list. e.g. com.android.music
*/
class DrmTrustedApp {
private:
    DrmTrustedApp();

public:
    static bool IsDrmTrustedApp(const String8& procName);

public:
    static const int TRUSTED_APP_CNT = 13;
    static const char* TRUSTED_APP[TRUSTED_APP_CNT];
};

/*
* this is defined for Mediatek default video playback applications (videoplayer)
* Note: normally you should not modify these
*/
class DrmTrustedVideoApp {
private:
    DrmTrustedVideoApp();

public:
    static bool IsDrmTrustedVideoApp(const String8& procName);

public:
    static const int TRUSTED_VIDEO_APP_CNT = 3;
    static const char* TRUSTED_VIDEO_APP[TRUSTED_VIDEO_APP_CNT];
};

/*
* this is defined for those servers which SNTP time synchronization will use
* Note: these are deprecated. The actuall definition is in DrmProvider
*/
class DrmSntpServer {
private:
    DrmSntpServer();

public:
    static const char* NTP_SERVER_1;
    static const char* NTP_SERVER_2;
    static const char* NTP_SERVER_3;
    static const char* NTP_SERVER_4;
    static const char* NTP_SERVER_5;
};

/*
* defined for OMA DRM v1 file's meta-data keys
*/
class DrmMetaKey {
private:
    DrmMetaKey();

public:
    static const char* META_KEY_IS_DRM;
    static const char* META_KEY_CONTENT_URI;
    static const char* META_KEY_OFFSET;
    static const char* META_KEY_DATALEN;
    static const char* META_KEY_RIGHTS_ISSUER;
    static const char* META_KEY_CONTENT_NAME;
    static const char* META_KEY_CONTENT_DESCRIPTION;
    static const char* META_KEY_CONTENT_VENDOR;
    static const char* META_KEY_ICON_URI;
    static const char* META_KEY_METHOD;
    static const char* META_KEY_MIME;
};

}

#endif // __DRM_MTK_DEF_H__
