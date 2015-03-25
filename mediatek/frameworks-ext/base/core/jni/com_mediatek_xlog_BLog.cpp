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

#include <stdio.h>
#include <stdint.h>

#include "jni.h"
#include "cutils/log.h"

#define XLOG_BLOG_DEBUG 0

#if XLOG_BLOG_DEBUG
#define XLOG_LOG(fmt, ...) \
    __android_log_print(ANDROID_LOG_DEBUG, "xlog", fmt, __VA_ARGS__)
#else
#define XLOG_LOG(fmt, ...)
#endif

static const char *bLogPackerClassPathName = "com/mediatek/xlog/BLogPacker";
jclass g_bLogPackerClass;
jfieldID g_bLogPacker_binBufField;
jfieldID g_bLogPacker_indexField;

static int ale_output_hash(JNIEnv *env, int prio, jstring tag, jint value)
{
    const char* chars = env->GetStringUTFChars(tag, NULL);
    uint32_t hash_value = value;

    XLOG_LOG("prio:%d %s: hashValue %x\n", prio, __PRETTY_FUNCTION__, hash_value);

    ale_log_output_binary(prio, chars, &hash_value, sizeof(hash_value));
    env->ReleaseStringUTFChars(tag, chars);
    return 1;
}

static jint
v1(JNIEnv *env, jobject thiz, jstring tag, jint hashValue) {
    return ale_output_hash(env, ANDROID_LOG_VERBOSE, tag, hashValue);
}

static jint
d1(JNIEnv *env, jobject thiz, jstring tag, jint hashValue) {
    return ale_output_hash(env, ANDROID_LOG_DEBUG, tag, hashValue);
}

static jint
i1(JNIEnv *env, jobject thiz, jstring tag, jint hashValue) {
    return ale_output_hash(env, ANDROID_LOG_INFO, tag, hashValue);
}

static jint
w1(JNIEnv *env, jobject thiz, jstring tag, jint hashValue) {
    return ale_output_hash(env, ANDROID_LOG_WARN, tag, hashValue);
}

static jint
e1(JNIEnv *env, jobject thiz, jstring tag, jint hashValue) {
    return ale_output_hash(env, ANDROID_LOG_ERROR, tag, hashValue);
}

static int ale_output_java_binary(JNIEnv *env, int prio, jstring tag, jint value, jobject packer)
{
    const char* chars = env->GetStringUTFChars(tag, NULL);
    uint32_t hash_value = value;

    jlong pointer = env->GetLongField(packer, g_bLogPacker_binBufField);
    jint index = env->GetIntField(packer, g_bLogPacker_indexField);

    XLOG_LOG("prio:%d %s: hashValue %x pointer %llx index %d\n", prio, __PRETTY_FUNCTION__, hash_value, pointer, index);
    
    uint8_t *binBuf = (uint8_t *)(pointer - 4);
    memcpy(binBuf, &hash_value, sizeof(hash_value));

    ale_log_output_binary(prio, chars, binBuf, index + sizeof(hash_value));

    env->ReleaseStringUTFChars(tag, chars);
    return 1;
}

static jint
bLog_v2(JNIEnv *env, jobject thiz, jstring tag, jint hashValue, jobject obj) {
    return ale_output_java_binary(env, ANDROID_LOG_VERBOSE, tag, hashValue, obj);
}

static jint
d2(JNIEnv *env, jobject thiz, jstring tag, jint hashValue, jobject obj) {
    return ale_output_java_binary(env, ANDROID_LOG_DEBUG, tag, hashValue, obj);
}

static jint
i2(JNIEnv *env, jobject thiz, jstring tag, jint hashValue, jobject obj) {
    return ale_output_java_binary(env, ANDROID_LOG_INFO, tag, hashValue, obj);
}

static jint
w2(JNIEnv *env, jobject thiz, jstring tag, jint hashValue, jobject obj) {
    return ale_output_java_binary(env, ANDROID_LOG_WARN, tag, hashValue, obj);
}

static jint
e2(JNIEnv *env, jobject thiz, jstring tag, jint hashValue, jobject obj) {
    return ale_output_java_binary(env, ANDROID_LOG_ERROR, tag, hashValue, obj);
}

static const char *classPathName = "com/mediatek/xlog/BLog";

static JNINativeMethod bLogMethods[] = {
  {"v", "(Ljava/lang/String;I)I", (void*)v1 },
  {"d", "(Ljava/lang/String;I)I", (void*)d1 },
  {"i", "(Ljava/lang/String;I)I", (void*)i1 },
  {"w", "(Ljava/lang/String;I)I", (void*)w1 },
  {"e", "(Ljava/lang/String;I)I", (void*)e1 },
  {"v", "(Ljava/lang/String;ILcom/mediatek/xlog/BLogPacker;)I", (void*)bLog_v2 },
  {"d", "(Ljava/lang/String;ILcom/mediatek/xlog/BLogPacker;)I", (void*)d2 },
  {"i", "(Ljava/lang/String;ILcom/mediatek/xlog/BLogPacker;)I", (void*)i2 },
  {"w", "(Ljava/lang/String;ILcom/mediatek/xlog/BLogPacker;)I", (void*)w2 },
  {"e", "(Ljava/lang/String;ILcom/mediatek/xlog/BLogPacker;)I", (void*)e2 },
};

static jlong bLogPacker_initBinaryBuffer(JNIEnv *env, jobject thiz) 
{
    void *binBuf = malloc(LOGGER_ALE_MSG_SIZE + 4);
    XLOG_LOG("%s %p\n", __func__, binBuf);
    return (jlong) binBuf + 4;
}

static jint bLogPacker_writeInt(JNIEnv *env, jlong pointer, jint index, jint jintValue) 
{
    uint8_t *binBuf = (uint8_t *) pointer;
    int32_t intValue = jintValue;

    XLOG_LOG("%s %lld %d intValue:%d\n", __func__, pointer, index, intValue);
    if (index + sizeof(intValue) < LOGGER_ALE_MSG_SIZE) {
        memcpy(binBuf + index, &intValue, sizeof(intValue));
        return index + sizeof(intValue);
    }
    else {
        return index;
    }
}

static jint bLogPacker_writeChar(JNIEnv *env, jlong pointer, jint index, jchar jcharValue)
{
    uint8_t *binBuf = (uint8_t *) pointer;
    char output[4] = {0, 0, 0, 0};
    uint16_t charValue = (uint16_t) jcharValue;

    XLOG_LOG("%s %lld %d charValue:%x\n", __func__, pointer, index, jcharValue);
    if (index + sizeof(charValue) < LOGGER_ALE_MSG_SIZE) {
        memcpy(binBuf + index, &charValue, sizeof(charValue));
        return index + sizeof(charValue);
    }
    else {
        return index;
    }
}

static jint bLogPacker_writeConstString(jlong pointer, jint index, const char *strValue) {
    uint8_t *binBuf = (uint8_t *) pointer;
    int strValueLen = strlen(strValue);

    XLOG_LOG("%s %llx %d strValue:%s\n", __func__, pointer, index, strValue);
    if (index + strValueLen >= LOGGER_ALE_MSG_SIZE) {
        strValueLen = LOGGER_ALE_MSG_SIZE - index - 1;
    }
    memcpy(binBuf + index, strValue, strValueLen);
    binBuf[index + strValueLen] = 0;

    return index + strValueLen + 1;
}

static jint bLogPacker_writeString(JNIEnv *env, jlong pointer, jint index, jstring jstrValue) 
{
    uint8_t *binBuf = (uint8_t *) pointer;
    if (jstrValue != NULL) {
      const char* strValue = env->GetStringUTFChars(jstrValue, NULL);
      int strValueLen = strlen(strValue);
      
      XLOG_LOG("%s %llx %d strValue:%s\n", __func__, pointer, index, strValue);
      if (index + strValueLen >= LOGGER_ALE_MSG_SIZE) {
        strValueLen = LOGGER_ALE_MSG_SIZE - index - 1;
      }
      memcpy(binBuf + index, strValue, strValueLen);
      env->ReleaseStringUTFChars(jstrValue, strValue);
      binBuf[index + strValueLen] = 0;
      
      return index + strValueLen + 1;
    }
    else {
      return bLogPacker_writeConstString(pointer, index, "null");
    }
}

static jint bLogPacker_writeDouble(JNIEnv *env, jlong pointer, jint index, jdouble jdoubleValue) 
{
    uint8_t *binBuf = (uint8_t *) pointer;
    double doubleValue = jdoubleValue;
    
    XLOG_LOG("%s %llx %d doubleValue:%f\n", __func__, pointer, index, doubleValue);
    if (index + sizeof(doubleValue) < LOGGER_ALE_MSG_SIZE) {
        memcpy(binBuf + index, &doubleValue, sizeof(doubleValue));
        return index + sizeof(doubleValue);
    }
    else {
        return index;
    }
}

static jint bLogPacker_writeBoolean(JNIEnv *env, jlong pointer, jint index, jboolean jboolValue) 
{
    uint8_t *binBuf = (uint8_t *) pointer;
    const char* strValue = jboolValue ? "true" : "false";
    int strValueLen = strlen(strValue);

    XLOG_LOG("%s %llx %d strValue:%s\n", __func__, pointer, index, strValue);
    if (index + strValueLen >= LOGGER_ALE_MSG_SIZE) {
        strValueLen = LOGGER_ALE_MSG_SIZE - index - 1;
    }
    memcpy(binBuf + index, strValue, strValueLen);
    binBuf[index + strValueLen] = 0;

    return index + strValueLen + 1;
}

static jint bLogPacker_writeLong(JNIEnv *env, jlong pointer, jint index, jlong jlongValue) 
{
    uint8_t *binBuf = (uint8_t *) pointer;
    int64_t longValue = jlongValue;

    XLOG_LOG("%s %lld %d longValue:%lld\n", __func__, pointer, index, longValue);
    if (index + sizeof(longValue) < LOGGER_ALE_MSG_SIZE) {
        memcpy(binBuf + index, &longValue, sizeof(longValue));
        return index + sizeof(longValue); 
    }
    else {
        return index;
    }
}

static void bLogPacker_finalize(JNIEnv *env, jobject thiz)
{
    jlong pointer = env->GetLongField(thiz, g_bLogPacker_binBufField);
    XLOG_LOG("%s pointer %llx\n", __func__, pointer);
    free((void *) (pointer - 4));
}

static JNINativeMethod bLogPackerMethods[] = {
    {"initBinaryBuffer", "()J", (void*)bLogPacker_initBinaryBuffer },
    {"writeChar", "(JIC)I", (void*)bLogPacker_writeChar },
    {"writeInt", "(JII)I", (void*)bLogPacker_writeInt },
    {"writeString", "(JILjava/lang/String;)I", (void*)bLogPacker_writeString },
    {"writeDouble", "(JID)I", (void*)bLogPacker_writeDouble },
    {"writeBoolean", "(JIZ)I", (void*)bLogPacker_writeBoolean },
    {"writeLong", "(JIJ)I", (void*)bLogPacker_writeLong },
    {"finalize", "()V", (void*)bLogPacker_finalize },
};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register xlog methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
int register_com_mediatek_xlog_BLog(JNIEnv* env)
{
    jclass packerClass = env->FindClass(bLogPackerClassPathName);
    g_bLogPackerClass = (jclass) env->NewGlobalRef(packerClass);
    g_bLogPacker_binBufField = env->GetFieldID(g_bLogPackerClass, "binBuf", "J");
    g_bLogPacker_indexField = env->GetFieldID(g_bLogPackerClass, "index", "I");

    if (!registerNativeMethods(env, classPathName,
                               bLogMethods, sizeof(bLogMethods) / sizeof(bLogMethods[0]))) {
        return JNI_FALSE;
    }
    
    if (!registerNativeMethods(env, bLogPackerClassPathName,
                               bLogPackerMethods, sizeof(bLogPackerMethods) / sizeof(bLogPackerMethods[0]))) {
        return JNI_FALSE;
    }
    
  return JNI_TRUE;
}
