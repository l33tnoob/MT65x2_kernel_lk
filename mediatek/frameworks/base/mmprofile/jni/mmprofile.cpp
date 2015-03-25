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

#define LOG_TAG "MMProfile"

#include "JNIHelp.h"
#include "jni.h"
#include <utils/Log.h>
#include <linux/mmprofile_internal.h>

jint MMProfileRegisterEvent_J(JNIEnv *env, jobject thiz, jint parent, jstring name)
{
    jint id;
    const char *_name = name ? env->GetStringUTFChars(name, NULL) : NULL;
    if (!_name)
    {
        return 0;
    }
    id = MMProfileRegisterEvent((MMP_Event)parent, _name);
    env->ReleaseStringUTFChars(name, _name);
    return id;
}

jint MMProfileFindEvent_J(JNIEnv *env, jobject thiz, jint parent, jstring name)
{
    jint id;
    const char *_name = name ? env->GetStringUTFChars(name, NULL) : NULL;
    if (!_name)
    {
        return 0;
    }
    id = MMProfileFindEvent((MMP_Event)parent, _name);
    env->ReleaseStringUTFChars(name, _name);
    return id;
}

void MMProfileEnableEvent_J(JNIEnv *env, jobject thiz, jint event, jint enable)
{
    MMProfileEnableEvent((MMP_Event)event, (int)enable);
}

void MMProfileEnableEventRecursive_J(JNIEnv *env, jobject thiz, jint event, jint enable)
{
    MMProfileEnableEventRecursive((MMP_Event)event, (int)enable);
}

jint MMProfileQueryEnable_J(JNIEnv *env, jobject thiz, jint event)
{
    return (jint)MMProfileQueryEnable((MMP_Event)event);
}

void MMProfileLog_J(JNIEnv *env, jobject thiz, jint event, jint type)
{
    MMProfileLog((MMP_Event)event, (MMP_LogType)type);
}

void MMProfileLogEx_J(JNIEnv *env, jobject thiz, jint event, jint type, jint data1, jint data2)
{
    MMProfileLogEx((MMP_Event)event, (MMP_LogType)type, (unsigned int)data1, (unsigned int)data2);
}

jint MMProfileLogMetaString_J(JNIEnv *env, jobject thiz, jint event, jint type, jstring str)
{
    jint ret;
    const char *_str = str ? env->GetStringUTFChars(str, NULL) : NULL;
    if (!_str)
    {
        return -1;
    }
    ret = MMProfileLogMetaString((MMP_Event)event, (MMP_LogType)type, _str);
    env->ReleaseStringUTFChars(str, _str);
    return ret;
}

jint MMProfileLogMetaStringEx_J(JNIEnv *env, jobject thiz, jint event, jint type, jint data1, jint data2, jstring str)
{
    jint ret;
    const char *_str = str ? env->GetStringUTFChars(str, NULL) : NULL;
    if (!_str)
    {
        return -1;
    }
    ret = MMProfileLogMetaStringEx((MMP_Event)event, (MMP_LogType)type, (unsigned int)data1, (unsigned int)data2, _str);
    env->ReleaseStringUTFChars(str, _str);
    return ret;
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
	{"MMProfileRegisterEvent", "(ILjava/lang/String;)I", (void*)MMProfileRegisterEvent_J},
    {"MMProfileFindEvent", "(ILjava/lang/String;)I", (void*)MMProfileFindEvent_J},
    {"MMProfileEnableEvent", "(II)V", (void*)MMProfileEnableEvent_J},
    {"MMProfileEnableEventRecursive", "(II)V", (void*)MMProfileEnableEventRecursive_J},
    {"MMProfileQueryEnable", "(I)I", (void*)MMProfileQueryEnable_J},
	{"MMProfileLog", "(II)V", (void*)MMProfileLog_J},
    {"MMProfileLogMetaString", "(IILjava/lang/String;)I", (void*)MMProfileLogMetaString_J},
    {"MMProfileLogMetaStringEx", "(IIIILjava/lang/String;)I", (void*)MMProfileLogMetaStringEx_J},
};

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed!");
        return result;
    }
    if (!env)
    {
        ALOGE("Could not retrieve the env!");
    }
    jniRegisterNativeMethods(env, "com/mediatek/mmprofile/MMProfile", sMethods, NELEM(sMethods));
    return JNI_VERSION_1_4;
}
