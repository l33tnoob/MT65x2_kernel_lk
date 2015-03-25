/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
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
int (*perfUserScnReg)(int, int) = NULL;
int (*perfUserScnUnreg)(int) = NULL;
int (*perfUserScnEnable)(int) = NULL;
int (*perfUserScnDisable)(int) = NULL;
int (*perfUserScnResetAll)(void) = NULL;
int (*perfUserScnDisableAll)(void) = NULL;

typedef int (*ena)(int);
typedef int (*disa)(int);
typedef int (*notify)(const char*, const char*, int);
typedef int (*user_reg)(int, int);
typedef int (*user_unreg)(int);
typedef int (*user_enable)(int);
typedef int (*user_disable)(int);
typedef int (*user_reset_all)(void);
typedef int (*user_disable_all)(void);

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

	func = dlsym(handle, "perfUserScnReg");
	perfUserScnReg = reinterpret_cast<user_reg>(func);

	if (perfUserScnReg == NULL) {
        ALOGE("perfUserScnReg error: %s", dlerror());
		perfUserScnReg = NULL;
		dlclose(handle);
		return;
	}

	func = dlsym(handle, "perfUserScnUnreg");
	perfUserScnUnreg = reinterpret_cast<user_unreg>(func);

	if (perfUserScnUnreg == NULL) {
        ALOGE("perfUserScnUnreg error: %s", dlerror());
		perfUserScnUnreg = NULL;
		dlclose(handle);
		return;
	}

	func = dlsym(handle, "perfUserScnEnable");
	perfUserScnEnable = reinterpret_cast<user_enable>(func);

	if (perfUserScnEnable == NULL) {
		ALOGE("perfUserScnEnable error: %s", dlerror());
		perfUserScnEnable = NULL;
		dlclose(handle);
		return;
	}

	func = dlsym(handle, "perfUserScnDisable");
	perfUserScnDisable = reinterpret_cast<user_disable>(func);

	if (perfUserScnDisable == NULL) {
		ALOGE("perfUserScnDisable error: %s", dlerror());
		perfUserScnDisable = NULL;
		dlclose(handle);
		return;
	}

	func = dlsym(handle, "perfUserScnResetAll");
	perfUserScnResetAll = reinterpret_cast<user_reset_all>(func);

	if (perfUserScnResetAll == NULL) {
		ALOGE("perfUserScnResetAll error: %s", dlerror());
		perfUserScnResetAll = NULL;
		dlclose(handle);
		return;
	}

	func = dlsym(handle, "perfUserScnDisableAll");
	perfUserScnDisableAll = reinterpret_cast<user_disable_all>(func);

	if (perfUserScnDisableAll == NULL) {
		ALOGE("perfUserScnDisableAll error: %s", dlerror());
		perfUserScnDisableAll = NULL;
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
		const char *nativeApp = env->GetStringUTFChars(packName, 0);
		const char *nativeCom = env->GetStringUTFChars(className, 0);

		if(nativeApp != NULL) {
			//ALOGI("android_server_PerfNotifyAppState: %s %s %d", nativeApp, nativeCom, state);
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

static int
android_server_PerfUserScnReg(JNIEnv *env, jobject thiz,
										jint scn_core, jint scn_freq)
{
	if (!inited)
		init();

	if (perfUserScnReg)
		return perfUserScnReg(scn_core, scn_freq);

	ALOGE("perfUserScnReg bypassed!");
    return -1;
}

static int
android_server_PerfUserScnUnreg(JNIEnv *env, jobject thiz,
										jint handle)
{
	if (!inited)
		init();

	if (perfUserScnUnreg)
		return perfUserScnUnreg(handle);

	ALOGE("perfUserScnUnreg bypassed!");
    return -1;
}

static int
android_server_PerfUserScnEnable(JNIEnv *env, jobject thiz,
										jint handle)
{
	if (!inited)
		init();

	if (perfUserScnEnable)
		return perfUserScnEnable(handle);

	ALOGE("perfBoostEnable bypassed!");
	return -1;
}

static int
android_server_PerfUserScnDisable(JNIEnv *env, jobject thiz,
										jint handle)
{
	if (!inited)
		init();

	if (perfUserScnDisable)
		return perfUserScnDisable(handle);

	ALOGE("perfBoostDisable bypassed!");
	return -1;
}

static int
android_server_PerfUserScnResetAll(JNIEnv *env, jobject thiz)
{
	if (!inited)
		init();

	if (perfUserScnResetAll)
		return perfUserScnResetAll();

	ALOGE("perfUserScnResetAll bypassed!");
	return -1;
}

static int
android_server_PerfUserScnDisableAll(JNIEnv *env, jobject thiz)
{
	if (!inited)
		init();

	if (perfUserScnDisableAll)
		return perfUserScnDisableAll();

	ALOGE("perfUserScnDisableAll bypassed!");
	return -1;
}

static JNINativeMethod sMethods[] = {
	{"nativePerfBoostEnable",    "(I)I",   (int *)android_server_PerfBoostEnable},
	{"nativePerfBoostDisable",   "(I)I",   (int *)android_server_PerfBoostDisable},
	{"nativePerfNotifyAppState", "(Ljava/lang/String;Ljava/lang/String;I)I", (int *)android_server_PerfNotifyAppState},
	{"nativePerfUserScnReg",     "(II)I",  (int *)android_server_PerfUserScnReg},
	{"nativePerfUserScnUnreg",   "(I)I",   (int *)android_server_PerfUserScnUnreg},
	{"nativePerfUserScnEnable",  "(I)I",   (int *)android_server_PerfUserScnEnable},
	{"nativePerfUserScnDisable", "(I)I",   (int *)android_server_PerfUserScnDisable},
	{"nativePerfUserScnResetAll",  "()I",  (int *)android_server_PerfUserScnResetAll},
	{"nativePerfUserScnDisableAll","()I",  (int *)android_server_PerfUserScnDisableAll},
};

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed!\n");
        return result;
    }
    if (!env)
    {
        ALOGE("Could not retrieve the env!\n");
    }
    jniRegisterNativeMethods(env, "com/mediatek/perfservice/PerfServiceManagerImpl", sMethods, NELEM(sMethods));
    return JNI_VERSION_1_4;
}

}

