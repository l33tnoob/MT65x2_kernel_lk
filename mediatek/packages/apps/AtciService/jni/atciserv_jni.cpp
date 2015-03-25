/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "ATCISERV_JNI"

#include <jni.h>
#include <cutils/jstring.h>
#include <utils/Log.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern "C" int writeBTAddr(unsigned char *addr);
extern "C" int readBTAddr(unsigned char *addr);
extern "C" int enableTestMode(int enable, unsigned char *addr);
extern "C" int queryTestMode();
/*****************************************************
*   Bluetooth JNI interface
*****************************************************/
static void addrToString(const unsigned char *addr, char *str){
    if(addr && str){
        sprintf(str, "%02X%02X%02X%02X%02X%02X", 
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    }
}

static void stringToAddr(const char *str, unsigned char *addr){
    char tmp[3] = {0,0,0};
    int i;
    if(str && addr){
        for(i = 0;i < 6;i++){
            tmp[0] = *(str++);
            tmp[1] = *(str++);
            addr[i] = (unsigned char)strtol(tmp, NULL, 16);
        }
    }
}

static jboolean writeBTAddress(JNIEnv* env, jobject object, jstring bdaddr){
    int ret = -1;
    const char *c_addr = NULL;
    unsigned char addr[6];
#ifdef __MTK_BT_SUPPORT__ 
    ALOGD("writeBTAddress");
    if(bdaddr != NULL){
        c_addr = env->GetStringUTFChars(bdaddr, NULL);
        if(c_addr){
            ALOGD("address=%s", c_addr);
            stringToAddr(c_addr, addr);
            env->ReleaseStringUTFChars(bdaddr, c_addr);
            ret = writeBTAddr(addr);
        }else{
            ALOGE("GetStringUTFChars failed");
        }
    }
#endif
    return (ret < 0 ) ? JNI_FALSE : JNI_TRUE;
}

static jstring readBTAddress(JNIEnv* env, jobject object){
    char c_addr[13];
    unsigned char addr[6];
#ifdef __MTK_BT_SUPPORT__
    if(readBTAddr(addr) < 0){
        return NULL;
    }
    addrToString(addr, c_addr);
    return env->NewStringUTF(c_addr);
#else
    return NULL;
#endif
}

static jboolean queryBTTestMode(JNIEnv* env, jobject object){
#ifdef __MTK_BT_SUPPORT__
    return (queryTestMode()) ? JNI_TRUE : JNI_FALSE;
#else
    return JNI_FALSE;
#endif
}

static jboolean enterBTTestMode(JNIEnv* env, jobject object, jstring bdaddr){
    int ret = -1;
    unsigned char addr[6];
    const char *c_addr;
#ifdef __MTK_BT_SUPPORT__
    c_addr = env->GetStringUTFChars(bdaddr, NULL);
    if(c_addr){
        stringToAddr(c_addr, addr);
        env->ReleaseStringUTFChars(bdaddr, c_addr);
        ret = enableTestMode(1, addr);
    }
#endif
    return (ret < 0) ? JNI_FALSE : JNI_TRUE;
}

static jboolean leaveBTTestMode(JNIEnv* env, jobject object){
    int ret = -1;
#ifdef __MTK_BT_SUPPORT__
    ret = enableTestMode(0, NULL);
#endif
    return (ret < 0) ? JNI_FALSE : JNI_TRUE;
}

static JNINativeMethod sMethods[] = {
    {"writeBTAddressNative", "(Ljava/lang/String;)Z",      (void *)writeBTAddress},
    {"readBTAddressNative", "()Ljava/lang/String;",         (void *)readBTAddress},
    {"queryBTTestModeNative", "()Z",                            (void *)queryBTTestMode},
    {"enterBTTestModeNative", "(Ljava/lang/String;)Z",    (void *)enterBTTestMode},
    {"leaveBTTestModeNative", "()Z",                             (void *)leaveBTTestMode},
    /*
    {"initServiceNative", "()Z", (void *)initServiceNative},
    {"cleanupNativeDataNative", "()V", (void *)cleanupNativeDataNative},
    {"enableNative", "()Z", (void *)enableNative},
    {"disableNative", "()V", (void *)disableNative},
    {"disconnectNative", "(I)V", (void *)disconnectNative},
    {"authorizeRspNative", "(Z)V", (void *)authorizeRspNative},
    {"selectSIMNative", "(I)Z", (void *)selectSIMNative},
    {"startListenNative", "()Z", (void *)startListenNative},
    {"stopListenNative", "()V", (void *)stopListenNative},
    {"prepareListentoSocketNative", "()Z", (void*) prepareListentoSocketNative},
    {"sendSIMUnaccessibleIndNative", "()V", (void *)sendSIMUnaccessibleIndNative}
    */
};


static int registerNativeMethods(JNIEnv* env, const char* className,
	JNINativeMethod* methods, int numMethods) 
{
    jclass clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'\n", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'\n", className);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static int registerNatives(JNIEnv* env) {
    if (!registerNativeMethods(env, "com/mediatek/atci/service/AtciService",
                                            sMethods, sizeof(sMethods) / sizeof(sMethods[0])))
    {
        ALOGE("[ATSERV_JNI] registerNativeMethods failed");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


/*
 * When library is loaded by Java by invoking "loadLibrary()".
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    ALOGI("[ATSERV_JNI] JNI_OnLoad [+]");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (!registerNatives(env)) {
        ALOGE("ERROR: failed to register natives\n");
        goto bail;
    }
    result = JNI_VERSION_1_4;

bail:
    ALOGI("[ATSERV_JNI] JNI_OnLoad [-]");
    return result;;
}

