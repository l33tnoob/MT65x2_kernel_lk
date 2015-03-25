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

/*
 * Copyright (C) 2007 The Android Open Source Project
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


#include <android_runtime/AndroidRuntime.h>
#include "jni.h"
#include "JNIHelp.h"
#include "custom_prop.h"

//#include <utils/Log.h>
#define MAX_PROP_VALUE        256

namespace android {

static jstring com_mediatek_custom_CustomProperties_get_string(JNIEnv* env, jobject clazz,
                                                               jstring moduleJ, jstring keyJ, jstring defaultJ)
{
    const char *key = NULL;
    const char *module = NULL;
    const char *defaultValue = NULL;
    char value[MAX_PROP_VALUE];
    jstring result;
    int len;


    if (moduleJ)
        module = env->GetStringUTFChars(moduleJ, NULL);
    if (keyJ)
        key = env->GetStringUTFChars(keyJ, NULL);
    if (defaultJ)
        defaultValue = env->GetStringUTFChars(defaultJ, NULL);

//    LOGD("[CustomProp]->module_p: %d,key_p: %d",module, key);
//    if (module)
//        LOGD("[CustomProp]->module: %s",module);
//    if (key)
//        LOGD("[CustomProp]->key: %s",key);
//    if (defaultValue)
//        LOGD("[CustomProp]->default: %s",defaultValue);


    len = custom_get_string(module, key, value, defaultValue);

    if (len <= 0)
        result = env->NewStringUTF("");
    else
        result = env->NewStringUTF(value);

    if (moduleJ)
        env->ReleaseStringUTFChars(moduleJ, module);
    if (keyJ)
        env->ReleaseStringUTFChars(keyJ, key);
    if (defaultJ)
        env->ReleaseStringUTFChars(defaultJ, defaultValue);

    return result;
}

// ----------------------------------------------------------------------------

/*
 * JNI registration.
 */
static JNINativeMethod gNotify[] = {
    {"native_get_string", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
            (void*)com_mediatek_custom_CustomProperties_get_string},

};

int register_com_mediatek_custom_CustomProperties(JNIEnv* env)
{
    int res = jniRegisterNativeMethods(env, "com/mediatek/custom/CustomProperties", gNotify, NELEM(gNotify));

    return res;
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }
    
    register_com_mediatek_custom_CustomProperties(env);

    return JNI_VERSION_1_4;
}

extern "C" JNIEnv* getJNIEnv()
{
    JNIEnv *env;

    if (!AndroidRuntime::getJavaVM())
        return 0;

    env = AndroidRuntime::getJNIEnv();

    return env;
}

}

