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

#include "SkStream.h"
#include "SkBitmap.h"
#include "SkImageDecoder.h"
#include "GraphicsJNI.h"

#include "utils/Log.h"
#include <cutils/xlog.h>

#include "./inc/MpoDecoder.h"

#undef LOG_TAG
#define LOG_TAG "MPO_Decoder"

using namespace std;

jclass options_class;
jfieldID options_justBoundsFieldID;//
jfieldID options_sampleSizeFieldID;//
jfieldID options_preferSizeFieldID;//
jfieldID options_postprocFieldID;
jfieldID options_postprocflagFieldID;
jfieldID options_configFieldID;//
jfieldID options_ditherFieldID;//
jfieldID options_purgeableFieldID;//
//jfieldID options_shareableFieldID;
//jfieldID options_nativeAllocFieldID;
jfieldID options_widthFieldID;//
jfieldID options_heightFieldID;//
jfieldID options_mimeFieldID;//
jfieldID options_mCancelID;//

bool isJniInited = false;

MpoDecoder::MpoDecoder(const char* pathname)
:mPathname(NULL), mMP_Images(NULL), mImageCount(0),
 mWidth(0), mHeight(0)
{
    XLOGI("Decode MPO file: %s", pathname);
    mMpoFileStream = new MpoFileInputStream(pathname);
    decodeStream();
}

MpoDecoder::MpoDecoder(MpoInputStream* mpoInputStream)
:mPathname(NULL), mMP_Images(NULL), mImageCount(0),
 mWidth(0), mHeight(0)
{
    XLOGI("Decode MpoInputStream: %d", mpoInputStream);
    mMpoFileStream = mpoInputStream;
    decodeStream();
}

void MpoDecoder::decodeStream() {
    //create 1st MP individual image record
    firstMPImage = new First_MP_Image();
    firstMPImage->setImageIndex(0);
    //read first MP Image App1 & App2 iofo
    if (false == firstMPImage->readStream(mMpoFileStream)) {
        XLOGE("read mpo stream failed!");
        return;
    }
    //get total image count
    mImageCount = firstMPImage->getImageCount();
    //allocate all images records
    mMP_Images = new MP_Image*[mImageCount];
    mMP_Images[0] = firstMPImage;
    mMP_Images[0]->setOffsetInFile( firstMPImage->getOffsetInFile(0) );
    mMP_Images[0]->setImageSize( firstMPImage->getImageSize(0) );
    for(int i=1; i<mImageCount; i++) {
        mMP_Images[i] = new MP_Image();
        mMP_Images[i]->setOffsetInFile( firstMPImage->getOffsetInFile(i) );
        mMP_Images[i]->setImageSize( firstMPImage->getImageSize(i) );
    }
//the following codes is to output all MPO attribute info
//    for (int i = 1; i < mImageCount; i++) {
//        mMpoFileStream->rewind();
//        mMpoFileStream->skip(mMP_Images[i]->getOffsetInFile());
//        mMP_Images[i]->setImageIndex(i);
//        mMP_Images[i]->readStream(mMpoFileStream);
//    }
//end
    //get image dimension
    decodeBounds();
    //get internal MTK mpo type
    mMtkMpoType = firstMPImage->getMtkMpoType();
    XLOGV("MpoDecoder:got MTK MPO type 0x%x",mMtkMpoType);
}

int MpoDecoder::suggestMtkMpoType() {
    if (MTK_TYPE_MAV == mMtkMpoType ||
        MTK_TYPE_3DPan == mMtkMpoType ||
        MTK_TYPE_Stereo == mMtkMpoType) {
        return mMtkMpoType;
    }

    if (MTK_TYPE_NONE == mMtkMpoType) {
        //got none MTK MPO type, this file is recorded by
        //other model, or by previous MTK model, we guess
        //what really type it is
        if (mImageCount > 4) {
            return MTK_TYPE_MAV;
        }
        if ((float)mWidth / (float)mHeight > 2.0f) {
            return MTK_TYPE_3DPan;
        } else {
            return MTK_TYPE_Stereo;
        }
    }

    XLOGI("suggestMtkMpoType:no suggest can be made!");
    return MTK_TYPE_NONE;
}

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

void initJni(JNIEnv* env) {
    options_class = make_globalref(env, "android/graphics/BitmapFactory$Options");
    options_justBoundsFieldID = getFieldIDCheck(env, options_class, "inJustDecodeBounds", "Z");
    options_sampleSizeFieldID = getFieldIDCheck(env, options_class, "inSampleSize", "I");
//    options_preferSizeFieldID = getFieldIDCheck(env, options_class, "inPreferSize", "I");
//    options_postprocFieldID = getFieldIDCheck(env, options_class, "inPostProc", "Z");
//    options_postprocflagFieldID = getFieldIDCheck(env, options_class, "inPostProcFlag", "I");
    options_configFieldID = getFieldIDCheck(env, options_class, "inPreferredConfig",
            "Landroid/graphics/Bitmap$Config;");
    options_ditherFieldID = getFieldIDCheck(env, options_class, "inDither", "Z");
    options_purgeableFieldID = getFieldIDCheck(env, options_class, "inPurgeable", "Z");
//    options_shareableFieldID = getFieldIDCheck(env, options_class, "inInputShareable", "Z");
//    options_nativeAllocFieldID = getFieldIDCheck(env, options_class, "inNativeAlloc", "Z");
    options_widthFieldID = getFieldIDCheck(env, options_class, "outWidth", "I");
    options_heightFieldID = getFieldIDCheck(env, options_class, "outHeight", "I");
    options_mimeFieldID = getFieldIDCheck(env, options_class, "outMimeType", "Ljava/lang/String;");
    options_mCancelID = getFieldIDCheck(env, options_class, "mCancel", "Z");
}

SkBitmap* MpoDecoder::getImageBitmap(JNIEnv* env, 
                                     int imageIndex,jobject options)
{
    if (imageIndex < 0 || imageIndex >= mImageCount) {
        XLOGE("getImageBitmap(imageIndex=%d):imageIndex is invalid",imageIndex);
        return NULL;
    }

    //init jni objects
    if (!isJniInited) {
        XLOGI("getImageBitmap:init JNIs");
        initJni(env);
        isJniInited = true;
    }

    long bufferSize = mMP_Images[imageIndex]->getImageSize() + 2 + 2; //SOI + JPEG + EOI
    mMpoFileStream->rewind();
    if (false == mMpoFileStream->skip(mMP_Images[imageIndex]->getOffsetInFile()) ) {
        XLOGE("failed to jump to image data");
        return NULL;
    }

    char* jpegBuffer = (char*)malloc((unsigned int)bufferSize);
    if (NULL == jpegBuffer) {
        XLOGE("can not allocate memory to hold JPEG data");
        return NULL;
    }

    if (bufferSize != mMpoFileStream->read(jpegBuffer, bufferSize)) {
        if (!mMpoFileStream->end()) {
            free(jpegBuffer);
            XLOGE("read jpeg data failed");
            return NULL;
        }
    }
    SkMemoryStream memStream(jpegBuffer, bufferSize);

    //decode buffer
    SkBitmap* bitmap = decodeBuffer(env, options, &memStream);

    //free memory buffer
    free(jpegBuffer);
    return bitmap;
}

SkBitmap* MpoDecoder::decodeBuffer(JNIEnv* env, jobject options, 
                                   SkStream* stream) {
    int sampleSize = 1;
    int preferSize = 0;
    int postproc = 0;
    int postprocflag = 0;
    SkImageDecoder::Mode mode = SkImageDecoder::kDecodePixels_Mode;
    SkBitmap::Config prefConfig = SkBitmap::kNo_Config;
    bool doDither = true;
    bool isPurgeable = options != NULL &&
            env->GetBooleanField(options, options_purgeableFieldID);

    if (NULL != options) {
        sampleSize = env->GetIntField(options, options_sampleSizeFieldID);
        //preferSize = env->GetIntField(options, options_preferSizeFieldID);
        //postproc = env->GetBooleanField(options, options_postprocFieldID);
        //postprocflag = env->GetIntField(options, options_postprocflagFieldID);
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

    SkImageDecoder* decoder = SkImageDecoder::Factory(stream);
    if (NULL == decoder) {
        XLOGE("SkImageDecoder-Factory() returned false");
        return NULL;
    }
    
    decoder->setSampleSize(sampleSize);
    decoder->setDitherImage(doDither);
    //decoder->setPreferSize(preferSize);
    //decoder->setPostProcFlag((postproc | (postprocflag << 4)));

    // To fix the race condition in case "requestCancelDecode"
    // happens earlier than AutoDecoderCancel object is added
    // to the gAutoDecoderCancelMutex linked list.
    if (NULL != options && env->GetBooleanField(options, options_mCancelID)) {
        XLOGE("Decoding is cancelled by requestCancelDecode");
        return NULL;
    }

    SkImageDecoder::Mode decodeMode = mode;
    if (isPurgeable) {
        decodeMode = SkImageDecoder::kDecodeBounds_Mode;
    }

    SkBitmap* bitmap = new SkBitmap;
    
    if (!decoder->decode(stream, bitmap, prefConfig, decodeMode)) {
        XLOGE("SkImageDecoder-decode() returned false");
        return NULL;
    }

    // update options (if any)
    if (NULL != options) {
        env->SetIntField(options, options_widthFieldID, bitmap->width());
        env->SetIntField(options, options_heightFieldID, bitmap->height());
        // TODO: set the mimeType field with the data from the codec.
        // but how to reuse a set of strings, rather than allocating new one
        // each time?
        env->SetObjectField(options, options_mimeFieldID,env->NewStringUTF("image/mpo"));
    }

    // if we're in justBounds mode, return now (skip the java bitmap)
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        delete bitmap;
        return NULL;
    } else {
        return bitmap;
    }
}

bool MpoDecoder::decodeBounds()
{
    long bufferSize = mMP_Images[0]->getImageSize() + 2 + 2; //SOI + JPEG + EOI
    mMpoFileStream->rewind();
    if (false == mMpoFileStream->skip(mMP_Images[0]->getOffsetInFile()) ) {
        XLOGE("failed to jump to image data");
        return false;
    }

    char* jpegBuffer = (char*)malloc((unsigned int)bufferSize);
    if (NULL == jpegBuffer) {
        XLOGE("can not allocate memory to hold JPEG data");
        return false;
    }

    if (bufferSize != mMpoFileStream->read(jpegBuffer, bufferSize)) {
        free(jpegBuffer);
        XLOGE("read jpeg data failed");
        return false;
    }
    SkMemoryStream memStream(jpegBuffer, bufferSize);

    int sampleSize = 1;
    int preferSize = 0;
    bool doDither = true;

    SkBitmap* bitmap = new SkBitmap;
    SkBitmap::Config prefConfig = SkBitmap::kNo_Config;
    SkImageDecoder::Mode decodeMode = SkImageDecoder::kDecodeBounds_Mode;
    SkImageDecoder* decoder = SkImageDecoder::Factory(&memStream);
    if (NULL == decoder) {
        XLOGE("SkImageDecoder-Factory() returned false");
        free(jpegBuffer);
        return false;
    }
    
    decoder->setSampleSize(sampleSize);
    decoder->setDitherImage(doDither);
    //decoder->setPreferSize(preferSize);

    if (!decoder->decode(&memStream, bitmap, prefConfig, decodeMode)) {
        XLOGE("SkImageDecoder-decode() returned false");
        free(jpegBuffer);
        return false;
    }
    //free memory buffer
    free(jpegBuffer);
    mWidth = bitmap->width();
    mHeight = bitmap->height();
    //delete created SkBitmap
    delete bitmap;
    return true;
}

MpoDecoder::~MpoDecoder()
{
    if (NULL != mPathname) {
        delete [] mPathname;
        mPathname = NULL;
    }

    if (NULL != mMP_Images) {
        for (int i=0; i<mImageCount; i++) {
            if (NULL != mMP_Images[i])
                delete mMP_Images[i];
        }
        delete mMP_Images;
    }
    
    if (NULL != mMpoFileStream) {
        delete mMpoFileStream;
        mMpoFileStream = NULL;
    }

}

