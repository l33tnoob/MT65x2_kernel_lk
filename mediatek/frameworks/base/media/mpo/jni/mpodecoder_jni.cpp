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

#define LOG_TAG "MPO_DECODER_JNI"

#include <jni.h>
#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>

#include "SkBitmap.h"
#include "GraphicsJNI.h"
#include "SkPixelRef.h"

#include "MpoDecoder.h"
#include "MpoStream.h"
#include "utils/Log.h"

#include "GraphicsJNI.h"
#include <cutils/xlog.h>

static const char *classPathName = "com/mediatek/mpo/MpoDecoder";
static jclass       mpoDecoder_class;
static jmethodID    mpoDecoder_constructorMethodID;
static jfieldID     mpoDecoder_nativeInstanceID;

class MpoDecoder;

jobject create_jmpodecoder(JNIEnv* env, MpoDecoder* mpodecoder) {
    if (NULL == mpodecoder) {
        XLOGE("NULL Mpo Decoder to wrap as Java object, return null");
        return NULL;
    }
    if (0 == mpodecoder->getImageCount()) {
        //as mpodecoder must not be NULL, delete C++ object
        delete mpodecoder;
        XLOGE("No images are recognized in mpo file, return null MpoDecoder");
        return NULL;
    }
    jobject obj = env->AllocObject(mpoDecoder_class);
    if (obj) {
        env->CallVoidMethod(obj, mpoDecoder_constructorMethodID, (jint)mpodecoder);
    }
    return obj;
}

static MpoDecoder* J2MpoDecoder(JNIEnv* env, jobject mpodecoder) {
    if (NULL == mpoDecoder_nativeInstanceID)
        return NULL;
    if (NULL == env || NULL == mpodecoder)
        return NULL;
    if (! env->IsInstanceOf(mpodecoder, mpoDecoder_class)) 
        return NULL;
    MpoDecoder* mpoDecoder = (MpoDecoder*)env->GetIntField(mpodecoder,
                                                  mpoDecoder_nativeInstanceID);

    return mpoDecoder;
}

///////////////////////////////////////////////////////////////////////////////

static jobject mpodecoder_frameBitmap(JNIEnv* env, jobject mpodecoder,
                                       int frameIndex, jobject options) {
    MpoDecoder* md = J2MpoDecoder(env, mpodecoder);
    if (NULL == md) {
        XLOGE("Null native MpoDecoder!");
        return NULL;
    }
    int totalFrame = md->getImageCount();
    if (frameIndex < 0 || frameIndex >= totalFrame) {
        XLOGE("Invalid frame Index: %d",frameIndex);
        return NULL;
    }
    SkBitmap *createdBitmap = md->getImageBitmap(env, frameIndex, options);
    if (createdBitmap != NULL)
        return GraphicsJNI::createBitmap(env, createdBitmap, false, NULL);
    else 
        return NULL;
}

static int mpodecoder_width(JNIEnv* env, jobject mpodecoder) {
    int width=0;
    MpoDecoder* md = J2MpoDecoder(env, mpodecoder);
    if (NULL != md)
        width = md->getWidth();
    return width;
}

static int mpodecoder_height(JNIEnv* env, jobject mpodecoder) {
    int height=0;
    MpoDecoder* md = J2MpoDecoder(env, mpodecoder);
    if (NULL != md)
        height = md->getHeight();
    return height;
}

static int mpodecoder_frameCount(JNIEnv* env, jobject mpodecoder) {
    int frameCount=0;
    MpoDecoder* md = J2MpoDecoder(env, mpodecoder);
    if (NULL != md)
        frameCount = md->getImageCount();
    return frameCount;
}

static int mpodecoder_getMtkMpoType(JNIEnv* env, jobject mpodecoder) {
    int mtkMpoType = MTK_TYPE_NONE;
    MpoDecoder* md = J2MpoDecoder(env, mpodecoder);
    if (NULL != md)
        mtkMpoType = md->getMtkMpoType();
    return mtkMpoType;
}

static int mpodecoder_suggestMtkMpoType(JNIEnv* env, jobject mpodecoder) {
    int mtkMpoType = MTK_TYPE_NONE;
    MpoDecoder* md = J2MpoDecoder(env, mpodecoder);
    if (NULL != md)
        mtkMpoType = md->suggestMtkMpoType();
    return mtkMpoType;
}

static void mpodecoder_close(JNIEnv* env, jobject mpodecoder) {
    if (NULL == mpoDecoder_nativeInstanceID)
        return;
    if (NULL == env || NULL == mpodecoder)
        return;
    if (! env->IsInstanceOf(mpodecoder, mpoDecoder_class)) 
        return;
    MpoDecoder* mpoDecoder = (MpoDecoder*)env->GetIntField(mpodecoder,
                                                  mpoDecoder_nativeInstanceID);
    XLOGV("MpoDecoder:mpodecoder_close()");
    delete mpoDecoder;
    env->SetIntField(mpodecoder,mpoDecoder_nativeInstanceID,0);
}


static jobject mpodecoder_decodeFile(JNIEnv* env, jobject clazz, 
                                       jstring jpathname) {
                              
    const char* pathname = env->GetStringUTFChars(jpathname, NULL);
    XLOGI("MpoDecoder:decode file %s", pathname);

    MpoDecoder *mpoDecoder = new MpoDecoder(pathname);
    return create_jmpodecoder(env, mpoDecoder);
}

static jobject mpodecoder_decodeByteArray(JNIEnv* env, jobject, 
                              jbyteArray byteArray, int offset, int length) {
    XLOGE("MpoDecoder:decode byte array:offset=%d, length=%d", offset,length);
    AutoJavaByteArray ar(env, byteArray);
    MpoInputStream* mpoInputStream = new MpoMemoryInputStream(ar.ptr() + offset,
                                                              length, true);
    XLOGE("MpoDecoder:got mpoInputStream %d", (int)mpoInputStream);
    MpoDecoder *mpoDecoder = new MpoDecoder(mpoInputStream);
    return create_jmpodecoder(env, mpoDecoder);
}


//JNI register
////////////////////////////////////////////////////////////////

static JNINativeMethod methods[] = {
    { "decodeByteArray", "([BII)Lcom/mediatek/mpo/MpoDecoder;",
                             (void*)mpodecoder_decodeByteArray },
    { "decodeFile", "(Ljava/lang/String;)Lcom/mediatek/mpo/MpoDecoder;",
                             (void*)mpodecoder_decodeFile },
    { "width",        "()I", (void*)mpodecoder_width  },
    { "height",       "()I", (void*)mpodecoder_height  },
    { "frameCount",   "()I", (void*)mpodecoder_frameCount  },
    { "getMtkMpoType",   "()I", (void*)mpodecoder_getMtkMpoType  },
    { "suggestMtkMpoType",   "()I", (void*)mpodecoder_suggestMtkMpoType  },
    { "frameBitmap", 
      "(ILandroid/graphics/BitmapFactory$Options;)Landroid/graphics/Bitmap;",
                             (void*)mpodecoder_frameBitmap  },
    { "close",   "()V",   (void*)mpodecoder_close  }
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
        XLOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        XLOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
int registerMpoNatives(JNIEnv* env)
{
  if (!registerNativeMethods(env, classPathName,
                 methods, sizeof(methods) / sizeof(methods[0]))) {
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

// ----------------------------------------------------------------------------
int register_mediatek_mpo_MpoDecoder(JNIEnv* env);
int register_mediatek_mpo_MpoDecoder(JNIEnv* env)
{
    mpoDecoder_class = env->FindClass(classPathName);
    if (NULL == mpoDecoder_class) 
        return JNI_FALSE;
    mpoDecoder_class = (jclass)env->NewGlobalRef(mpoDecoder_class);
    if (NULL == mpoDecoder_class) 
        return JNI_FALSE;
    mpoDecoder_constructorMethodID = 
               env->GetMethodID(mpoDecoder_class, "<init>", "(I)V");
    if (NULL == mpoDecoder_constructorMethodID) 
        return JNI_FALSE;
    mpoDecoder_nativeInstanceID = 
               env->GetFieldID(mpoDecoder_class, "mNativeMpoDecoder", "I");
    if (NULL == mpoDecoder_nativeInstanceID) 
        return JNI_FALSE;

    return registerMpoNatives(env);
}


/*
 * This is called by the VM when the shared library is first loaded.
 */
 
typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;
    
    XLOGI("JNI_OnLoad");
    
    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        XLOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (register_mediatek_mpo_MpoDecoder(env) != JNI_TRUE) {
        XLOGE("ERROR: registerNatives failed");
        goto bail;
    }
    result = JNI_VERSION_1_4;
    
bail:
    return result;
}

