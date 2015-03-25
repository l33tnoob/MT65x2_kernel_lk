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

#define LOG_TAG "DRM_DCF_DECODER_JNI"
#include <utils/Log.h>
#include <cutils/xlog.h>

#include <jni.h>


#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "SkBitmap.h"
#include "GraphicsJNI.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkImageDecoder.h"
#include "GraphicsJNI.h"

#include <drm_framework_common.h>
#include <DrmManagerClient.h>
#include <DrmMtkUtil.h>

#define DECRYPT_BUF_LEN 4096

using namespace android;

static const char *classPathName = "com/mediatek/dcfdecoder/DcfDecoder";
static jclass dcfDecoder_class;
static jmethodID dcfDecoder_constructorMethodID;
static jfieldID dcfDecoder_nativeInstanceID;

jclass options_class;
jfieldID options_justBoundsFieldID;
jfieldID options_sampleSizeFieldID;
jfieldID options_preferSizeFieldID;
jfieldID options_configFieldID;
jfieldID options_ditherFieldID;
jfieldID options_purgeableFieldID;
jfieldID options_widthFieldID;
jfieldID options_heightFieldID;
jfieldID options_mimeFieldID;
jfieldID options_mCancelID;

static jclass make_globalref(JNIEnv* env, const char classname[]) {
    jclass c = env->FindClass(classname);
    SkASSERT(c);
    return (jclass)env->NewGlobalRef(c);
}

static jfieldID getFieldIDCheck(JNIEnv* env, jclass clazz,
        const char fieldname[], const char type[]) {
    jfieldID id = env->GetFieldID(clazz, fieldname, type);
    SkASSERT(id);
    return id;
}

jstring getMimeTypeString(JNIEnv* env, SkImageDecoder::Format format) {
    static const struct {
        SkImageDecoder::Format fFormat;
        const char*            fMimeType;
    } gMimeTypes[] = {
        { SkImageDecoder::kBMP_Format,  "image/bmp" },
        { SkImageDecoder::kGIF_Format,  "image/gif" },
        { SkImageDecoder::kICO_Format,  "image/x-ico" },
        { SkImageDecoder::kJPEG_Format, "image/jpeg" },
        { SkImageDecoder::kPNG_Format,  "image/png" },
        { SkImageDecoder::kWBMP_Format, "image/vnd.wap.wbmp" }
    };

    const char* cstr = NULL;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gMimeTypes); i++) {
        if (gMimeTypes[i].fFormat == format) {
            cstr = gMimeTypes[i].fMimeType;
            break;
        }
    }

    jstring jstr = 0;
    if (NULL != cstr) {
        jstr = env->NewStringUTF(cstr);
    }
    return jstr;
}

static unsigned char* decryptDcfSource(JNIEnv* env,
                                       DrmManagerClient *drmManagerClient,
                                       sp<DecryptHandle> decryptHandle,
                                       int* length) {
    if (NULL == env) {
        XLOGE("decryptDcfSource: invalid JNIEnv.");
        return NULL;
    }
    if (NULL == drmManagerClient || NULL == decryptHandle.get()) {
        XLOGE("decryptDcfSource: drmManagerClient or decryptHandle is NULL");
        return NULL;
    }

    int fileLength = decryptHandle->decryptInfo->decryptBufferLength;
    XLOGD("decryptDcfSource: file length %d Bytes", fileLength);
    if (fileLength <= 0) {
        XLOGE("decryptDcfSource: illegal file length.");
        return NULL;
    }

    //allocate buffer to hold decrypted data
    // just use the length of the encrypted data
    unsigned char *decryptedBuf = (unsigned char*)malloc(fileLength);
    memset(decryptedBuf, 0, fileLength);

    unsigned char buffer[DECRYPT_BUF_LEN];
    memset(buffer, 0, sizeof(buffer));

    int readSize = 0;
    int resultSize = 0; // also the offset of data
    while (readSize < fileLength) {
        memset(buffer, 0, sizeof(buffer));
        int size = (fileLength - readSize) > DECRYPT_BUF_LEN ?
            DECRYPT_BUF_LEN : (fileLength - readSize);
        int readLength = drmManagerClient->pread(decryptHandle, buffer, size, resultSize);
        if (readLength <= 0) {
            XLOGE("decryptDcfSource: failed to read valid decrypted data.");
            break;
        }
        memcpy(decryptedBuf + resultSize, buffer, readLength);
        readSize += size;
        resultSize += readLength;
    }
    if (resultSize <= 0) {
        XLOGE("decryptDcfSource: failed to read decrypted data.");
        free(decryptedBuf);
        return NULL;
    }

    if (NULL != length) {
    *length = resultSize;
    XLOGD("decryptDcfSource: returned buffer length %d", resultSize);
    }
    return decryptedBuf;
}

static jobject decodeDcfSource(JNIEnv* env, jobject options,
                               DrmManagerClient *drmManagerClient,
                               sp<DecryptHandle> decryptHandle ) {
    if (NULL == env) {
        XLOGE("decodeDcfSource: invalid JNIEnv.");
        return NULL;
    }
    if (NULL == drmManagerClient || NULL == decryptHandle.get()) {
        XLOGE("decodeDcfSource: drmManagerClient or decryptHandle is NULL");
        return NULL;
    }

    int length = 0;
    unsigned char* decryptedBuf = decryptDcfSource(env, drmManagerClient, decryptHandle, &length);
    if (NULL == decryptedBuf) {
        XLOGE("decodeDcfSource: failed to decrypt DCF.");
        return NULL;
    }

    //as decrypted data are in buffer, wrap it into SkMemoryStream
    SkMemoryStream memStream(decryptedBuf, length);

    int sampleSize = 1;
    //int preferSize = 0; // deprecated
    SkImageDecoder::Mode mode = SkImageDecoder::kDecodePixels_Mode;
    SkBitmap::Config prefConfig = SkBitmap::kNo_Config;
    bool doDither = true;
    bool isPurgeable =
            options != NULL
            && env->GetBooleanField(options, options_purgeableFieldID);

    if (NULL != options) {
        sampleSize = env->GetIntField(options, options_sampleSizeFieldID);
        //preferSize = env->GetIntField(options, options_preferSizeFieldID); // deprecated
        if (env->GetBooleanField(options, options_justBoundsFieldID)) {
            mode = SkImageDecoder::kDecodeBounds_Mode;
        }
        // initialize these, in case we fail later on
        env->SetIntField(options, options_widthFieldID, -1);
        env->SetIntField(options, options_heightFieldID, -1);
        env->SetObjectField(options, options_mimeFieldID, 0);

        jobject jconfig = env->GetObjectField(options, options_configFieldID);
        prefConfig = GraphicsJNI::getNativeBitmapConfig(env, jconfig);
        doDither = env->GetBooleanField(options, options_ditherFieldID);
    }

    SkImageDecoder* decoder = SkImageDecoder::Factory(&memStream);
    if (NULL == decoder) {
        XLOGE("decodeDcfSource: SkImageDecoder-Factory() failed, return NULL");
        free(decryptedBuf);
        return NULL;
    }

    // To fix the race condition in case "requestCancelDecode"
    // happens earlier than AutoDecoderCancel object is added
    // to the gAutoDecoderCancelMutex linked list.
    if (NULL != options && env->GetBooleanField(options, options_mCancelID)) {
        XLOGE("decodeDcfSource: Decoding is cancelled, return NULL");
        free(decryptedBuf);
        return NULL;
    }

    SkImageDecoder::Mode decodeMode = mode;
    if (isPurgeable) {
        decodeMode = SkImageDecoder::kDecodeBounds_Mode;
    }

    SkBitmap* bitmap = new SkBitmap;

    decoder->setSampleSize(sampleSize);
    decoder->setDitherImage(doDither);
    //decoder->setPreferSize(preferSize); // the function "setPreferSize" does not exist any more on JB

    if (!decoder->decode(&memStream, bitmap, prefConfig, decodeMode)) {
        XLOGE("decodeDcfSource: SkImageDecoder-decode() failed, return NULL");
        free(decryptedBuf);
        return NULL;
    }

    free(decryptedBuf);

    // update options (if any)
    if (NULL != options) {
        env->SetIntField(options, options_widthFieldID, bitmap->width());
        env->SetIntField(options, options_heightFieldID, bitmap->height());
        // TODO: set the mimeType field with the data from the codec.
        // but how to reuse a set of strings, rather than allocating new one
        // each time?
        env->SetObjectField(options, options_mimeFieldID,
                            getMimeTypeString(env, decoder->getFormat()));
    }

    // if we're in justBounds mode, return now (skip the java bitmap)
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        XLOGD("decodeDcfSource: justBounds mode, return NULL");
        return NULL;
    }

    if (bitmap != NULL) {
        XLOGD("decodeDcfSource: bitmap valid, wrap it into a Java Bitmap");
        return GraphicsJNI::createBitmap(env, bitmap, GraphicsJNI::kBitmapCreateFlag_Premultiplied, NULL);
    } else {
        XLOGD("decodeDcfSource: bitmap invalid");
        return NULL;
    }
}

static jobject nativeDecodeFile(JNIEnv* env, jobject clazz,
                                jstring filePath,
                                jobject options,
                                jboolean consume) {
    XLOGD("nativeDecodeFile ---->");
    if (NULL == env || NULL == filePath) {
        XLOGE("nativeDecodeFile: JNIEnv or filePath is NULL");
        return NULL;
    }

    const char* pathname = env->GetStringUTFChars(filePath, NULL);
    XLOGD("nativeDecodeFile: decode file %s", pathname);
    if (NULL == pathname) {
        XLOGE("nativeDecodeFile: failed to get file name");
        return NULL;
    }
    //!!!!!! whether pathname needs to be freed when returned ?!!!!!!!!

    int fd = open(pathname, O_RDONLY);
    if (0 > fd) {
        XLOGE("nativeDecodeFile: failed to open file");
        return NULL;
    }

    DrmManagerClient *drmManagerClient = new DrmManagerClient();
    if (NULL == drmManagerClient) {
        XLOGE("nativeDecodeFile: failed to create DrmManagerClient");
        close(fd);
        return NULL;
    }

    sp<DecryptHandle> decryptHandle = drmManagerClient->openDecryptSession(fd, 0, 0, NULL);
    if (NULL == decryptHandle.get()) {
        XLOGE("nativeDecodeFile: failed to open decryptSession");
        delete drmManagerClient;
        close(fd);
        return NULL;
    }

    jobject bmp = decodeDcfSource(env, options, drmManagerClient, decryptHandle);
    if (bmp != NULL && consume == JNI_TRUE) {
        XLOGD("nativeDecodeFile: consume rights");
        drmManagerClient->consumeRights(decryptHandle, Action::DISPLAY, false);
    }

    drmManagerClient->closeDecryptSession(decryptHandle);
    delete drmManagerClient;
    close(fd);

    return bmp;
}

static jobject nativeForceDecodeFile(JNIEnv* env, jobject clazz,
                                     jstring filePath,
                                     jobject options,
                                     jboolean consume) {
    XLOGD("nativeForceDecodeFile ---->");
    if (NULL == env || NULL == filePath) {
        XLOGE("nativeForceDecodeFile: JNIEnv or filePath is NULL");
        return NULL;
    }

    const char* pathname = env->GetStringUTFChars(filePath, NULL);
    XLOGD("nativeForceDecodeFile: decode file %s", pathname);
    if (NULL == pathname) {
        XLOGE("nativeForceDecodeFile: failed to get file name");
        return NULL;
    }
    //!!!!!! whether pathname needs to be freed when returned ?!!!!!!!!

    int fd = open(pathname, O_RDONLY);
    if (0 > fd) {
        XLOGE("nativeForceDecodeFile: failed to open file");
        return NULL;
    }

    DrmManagerClient *drmManagerClient = new DrmManagerClient();
    if (NULL == drmManagerClient) {
        XLOGE("nativeForceDecodeFile: failed to create DrmManagerClient");
        close(fd);
        return NULL;
    }

    sp<DecryptHandle> decryptHandle =
            drmManagerClient->openDecryptSession(fd, 0, 0, NULL);
    if (NULL == decryptHandle.get()) {
        XLOGE("nativeForceDecodeFile: failed to open decryptSession");
        delete drmManagerClient;
        close(fd);
        return NULL;
    }

    jobject bmp = decodeDcfSource(env, options, drmManagerClient, decryptHandle);
    if (bmp != NULL && consume == JNI_TRUE) {
        XLOGD("nativeForceDecodeFile: consume rights");
        drmManagerClient->consumeRights(decryptHandle, Action::DISPLAY, false);
    }

    drmManagerClient->closeDecryptSession(decryptHandle);
    delete drmManagerClient;
    close(fd);

    return bmp;
}

static jbyteArray nativeDecryptFile(JNIEnv* env, jobject clazz,
                                    jstring filePath,
                                    jboolean consume) {
    XLOGD("nativeDecryptFile ---->");
    if (NULL == env) {
        XLOGE("nativeDecryptFile: invalid JNI env.");
        return NULL;
    }
    if (NULL == filePath) {
        XLOGE("nativeDecryptFile: invalid filePath.");
        return NULL;
    }

    //get file name
    //!!!!!! whether pathname needs to be freed when returned ?!!!!!!!!
    const char* pathname = env->GetStringUTFChars(filePath, NULL);
    if (NULL == pathname) {
        XLOGE("nativeDecryptFile: failed to get file path name.");
        return NULL;
    }
    XLOGD("nativeDecryptFile: attempt to decrypt file [%s]", pathname);

    int fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        XLOGE("nativeDecryptFile: failed to open file to read.");
        return NULL;
    }

    // open decryptSession...
    DrmManagerClient *drmManagerClient = new DrmManagerClient();
    if (NULL == drmManagerClient) {
        XLOGE("nativeDecryptFile: failed to create DrmManagerClient.");
        close(fd);
        return NULL;
    }
    sp<DecryptHandle> decryptHandle =
            drmManagerClient->openDecryptSession(fd, 0, 0, NULL);
    if (decryptHandle.get() == NULL) {
        XLOGE("nativeDecryptFile: failed to open decrypt session.");
        delete drmManagerClient;
        close(fd);
        return NULL;
    }

    int length = 0;
    unsigned char* decryptedBuf = decryptDcfSource(env, drmManagerClient, decryptHandle, &length);
    if (NULL == decryptedBuf) {
        XLOGE("nativeDecryptFile: failed to decrypt DCF.");
        drmManagerClient->closeDecryptSession(decryptHandle);
        delete drmManagerClient;
        close(fd);
        return NULL;
    }
    XLOGD("nativeDecryptFile: result length [%d]", length);

    if (consume == JNI_TRUE) {
        XLOGD("nativeDecryptFile: consume rights.");
        drmManagerClient->consumeRights(decryptHandle, Action::DISPLAY, false);
    }

    drmManagerClient->closeDecryptSession(decryptHandle);
    delete drmManagerClient;
    close(fd);

    // the result
    jbyteArray result = env->NewByteArray(length);
    env->SetByteArrayRegion(result, 0, length, (const jbyte*)decryptedBuf);
    free(decryptedBuf);

    return result;
}

static jbyteArray nativeForceDecryptFile(JNIEnv* env, jobject clazz,
                                         jstring filePath,
                                         jboolean consume) {
    XLOGD("nativeForceDecryptFile ---->");
    if (NULL == env) {
        XLOGE("nativeForceDecryptFile: invalid JNI env.");
        return NULL;
    }
    if (NULL == filePath) {
        XLOGE("nativeForceDecryptFile: invalid filePath.");
        return NULL;
    }

    //get file name
    //!!!!!! whether pathname needs to be freed when returned ?!!!!!!!!
    const char* pathname = env->GetStringUTFChars(filePath, NULL);
    if (NULL == pathname) {
        XLOGE("nativeForceDecryptFile: failed to get file path name.");
        return NULL;
    }
    XLOGD("nativeForceDecryptFile: attempt to decrypt file [%s]", pathname);

    int fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        XLOGE("nativeForceDecryptFile: failed to open file to read.");
        return NULL;
    }

    // open decryptSession...
    DrmManagerClient *drmManagerClient = new DrmManagerClient();
    if (NULL == drmManagerClient) {
        XLOGE("nativeForceDecryptFile: failed to create DrmManagerClient.");
        close(fd);
        return NULL;
    }
    sp<DecryptHandle> decryptHandle =
            drmManagerClient->openDecryptSession(fd, 0, 0, NULL);
    if (decryptHandle.get() == NULL) {
        XLOGE("nativeForceDecryptFile: failed to open decrypt session.");
        delete drmManagerClient;
        close(fd);
        return NULL;
    }

    int length = 0;
    unsigned char* decryptedBuf = decryptDcfSource(env, drmManagerClient, decryptHandle, &length);
    if (NULL == decryptedBuf) {
        XLOGE("nativeForceDecryptFile: failed to decrypt DCF.");
        drmManagerClient->closeDecryptSession(decryptHandle);
        delete drmManagerClient;
        close(fd);
        return NULL;
    }
    XLOGD("nativeForceDecryptFile: result length [%d]", length);

    if (consume == JNI_TRUE) {
        XLOGD("nativeForceDecryptFile: consume rights.");
        drmManagerClient->consumeRights(decryptHandle, Action::DISPLAY, false);
    }

    drmManagerClient->closeDecryptSession(decryptHandle);
    delete drmManagerClient;
    close(fd);

    // the result
    jbyteArray result = env->NewByteArray(length);
    env->SetByteArrayRegion(result, 0, length, (const jbyte*)decryptedBuf);
    free(decryptedBuf);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
//JNI register
///////////////////////////////////////////////////////////////////////////////

static JNINativeMethod methods[] = {
    { "nativeDecodeFile",
        "(Ljava/lang/String;Landroid/graphics/BitmapFactory$Options;Z)Landroid/graphics/Bitmap;",
        (void*)nativeDecodeFile },

    { "nativeForceDecodeFile",
        "(Ljava/lang/String;Landroid/graphics/BitmapFactory$Options;Z)Landroid/graphics/Bitmap;",
        (void*)nativeForceDecodeFile },

    { "nativeDecryptFile",
        "(Ljava/lang/String;Z)[B",
        (void*)nativeDecryptFile },

    { "nativeForceDecryptFile",
        "(Ljava/lang/String;Z)[B",
        (void*)nativeForceDecryptFile }
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
        XLOGE("registerNativeMethods: Native registration unable to find class '%s'",
                className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        XLOGE("registerNativeMethods: RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
int registerDcfNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, classPathName, methods,
                               sizeof(methods) / sizeof(methods[0]))) {
        return JNI_FALSE;
    }

    options_class = make_globalref(env, "android/graphics/BitmapFactory$Options");
    options_justBoundsFieldID = getFieldIDCheck(env, options_class, "inJustDecodeBounds", "Z");
    options_sampleSizeFieldID = getFieldIDCheck(env, options_class, "inSampleSize", "I");
    options_preferSizeFieldID = getFieldIDCheck(env, options_class, "inPreferSize", "I");
    options_configFieldID = getFieldIDCheck(env, options_class, "inPreferredConfig", "Landroid/graphics/Bitmap$Config;");
    options_ditherFieldID = getFieldIDCheck(env, options_class, "inDither", "Z");
    options_purgeableFieldID = getFieldIDCheck(env, options_class, "inPurgeable", "Z");
    options_widthFieldID = getFieldIDCheck(env, options_class, "outWidth", "I");
    options_heightFieldID = getFieldIDCheck(env, options_class, "outHeight", "I");
    options_mimeFieldID = getFieldIDCheck(env, options_class, "outMimeType", "Ljava/lang/String;");
    options_mCancelID = getFieldIDCheck(env, options_class, "mCancel", "Z");

    return JNI_TRUE;
}

// ----------------------------------------------------------------------------

int register_mediatek_DcfDecoder(JNIEnv* env)
{
    return registerDcfNatives(env);
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        XLOGE("JNI_OnLoad: ERROR: GetEnv failed\n");
        return result;
    }
    assert(env != NULL);

    if (register_mediatek_DcfDecoder(env) < 0) {
        XLOGE("JNI_OnLoad: ERROR: register_mediatek_DcfDecoder failed\n");
        return result;
    }
    result = JNI_VERSION_1_4;
    return result;
}

