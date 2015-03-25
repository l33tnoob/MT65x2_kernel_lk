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

#define LOG_TAG "ShutdownManager"
#include "utils/Log.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"

#include <media/AudioSystem.h>


namespace android
{

static int
android_media_AudioSystem_SetMasterMute(JNIEnv *env, jobject thiz, jboolean mute)
{
	if(NO_ERROR == AudioSystem::setMasterMute(mute))
		return 0;
	else
		return -1;
}

static int
android_media_AudioSystem_GetMasterMute(JNIEnv *env, jobject thiz)
{
	bool muted;
	if(NO_ERROR == AudioSystem::getMasterMute(&muted))
		return muted?1:0;
	return -1;
}

static int
android_media_AudioSystem_SetStreamMute(JNIEnv *env, jobject thiz,
										jint streamType, jboolean mute)
{
	if(NO_ERROR == AudioSystem::setStreamMute((audio_stream_type_t)streamType, mute))
		return 0;
	else
		return -1;
}

static int
android_media_AudioSystem_GetStreamMute(JNIEnv *env, jobject thiz,
										jint streamType)
{
	bool muted;
	if(NO_ERROR == AudioSystem::getStreamMute((audio_stream_type_t)streamType, &muted))
		return muted?1:0;
	return -1;
}

static JNINativeMethod method_table[] = {
	{"SetMasterMute",	"(Z)I",	   (void *)android_media_AudioSystem_SetMasterMute},
	{"GetMasterMute",	"()I",	   (void *)android_media_AudioSystem_GetMasterMute},
	{"SetStreamMute",	"(IZ)I",   (void *)android_media_AudioSystem_SetStreamMute},
	{"GetStreamMute",	"(I)I",	   (void *)android_media_AudioSystem_GetStreamMute},
};

int register_com_android_internal_app_ShutdownManager(JNIEnv *env)
{
    return AndroidRuntime::registerNativeMethods(env, "com/android/internal/app/ShutdownManager",
            method_table, NELEM(method_table));
}

}
