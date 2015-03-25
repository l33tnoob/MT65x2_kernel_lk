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

#define LOG_TAG "PerfService"
#include "utils/Log.h"

#include <stdio.h>
#include <dlfcn.h>

#include <unistd.h>

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"

namespace android
{

static int inited = false;

int (*perfBoostEnable)(int) = NULL;
int (*perfBoostDisable)(int) = NULL;
int (*perfNotifyAppState)(const char*, const char*, int) = NULL;

typedef int (*ena)(int);
typedef int (*disa)(int);
typedef int (*notify)(const char*, const char*, int);

#define LIB_FULL_NAME "/system/lib/libperfservice.so"

void init()
{
	void *handle, *func;

	// only enter once
	inited = true;

	handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
	if (handle == NULL) {
		ALOGE("Can't load library: %s", dlerror());
		return;
	}

	func = dlsym(handle, "perfBoostEnable");
	perfBoostEnable = reinterpret_cast<ena>(func);

	if (perfBoostEnable == NULL) {
        ALOGE("perfBoostEnable error: %s", dlerror());
		dlclose(handle);
		return;
	}

	func = dlsym(handle, "perfBoostDisable");
	perfBoostDisable = reinterpret_cast<disa>(func);

	if (perfBoostDisable == NULL) {
        ALOGE("perfBoostDisable error: %s", dlerror());
		perfBoostEnable = NULL;
		dlclose(handle);
		return;
	}

	func = dlsym(handle, "perfNotifyAppState");
	perfNotifyAppState = reinterpret_cast<notify>(func);

	if (perfNotifyAppState == NULL) {
        ALOGE("perfNotifyAppState error: %s", dlerror());
		perfNotifyAppState = NULL;
		dlclose(handle);
		return;
	}
}

static int
android_server_PerfBoostEnable(JNIEnv *env, jobject thiz,
										jint scenario)
{
	if (!inited)
		init();

	if (perfBoostEnable)
		return perfBoostEnable(scenario);

	ALOGE("perfBoostEnable bypassed!");
    return -1;
}

static int
android_server_PerfBoostDisable(JNIEnv *env, jobject thiz,
										jint scenario)
{
	if (!inited)
		init();

	if (perfBoostDisable)
		return perfBoostDisable(scenario);

	ALOGE("perfBoostDisable bypassed!");
	return -1;
}

static int
android_server_PerfNotifyAppState(JNIEnv *env, jobject thiz, jstring packName, jstring className,
										jint state)
{
	if (!inited)
		init();

	if (perfNotifyAppState) {
		const char *nativeApp = (packName) ? env->GetStringUTFChars(packName, 0) : NULL;
		const char *nativeCom = (className) ? env->GetStringUTFChars(className, 0) : NULL;

		if(nativeApp != NULL) {
			perfNotifyAppState(nativeApp, nativeCom, state);
			if(nativeCom != NULL)
				env->ReleaseStringUTFChars(className, nativeCom);
			env->ReleaseStringUTFChars(packName, nativeApp);
			return 0;
		}
		else
			return -1;
	}

	ALOGE("perfNotifyAppState bypassed!");
	return -1;
}

static JNINativeMethod method_table[] = {
	{"nativePerfBoostEnable",    "(I)I",   (int *)android_server_PerfBoostEnable},
	{"nativePerfBoostDisable",   "(I)I",   (int *)android_server_PerfBoostDisable},
	{"nativePerfNotifyAppState", "(Ljava/lang/String;Ljava/lang/String;I)I", (int *)android_server_PerfNotifyAppState},
};

int register_android_service_PerfService(JNIEnv *env)
{
    return AndroidRuntime::registerNativeMethods(env, "com/android/server/PerfService",
            method_table, NELEM(method_table));
}

}
