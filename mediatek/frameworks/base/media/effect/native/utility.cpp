/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

#include <jni.h>
#include <JNIHelp.h>
#include "android_runtime/AndroidRuntime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GraphicsJNI.h"
#include <cutils/xlog.h>
#include "utility.h"

jint getIntField(JNIEnv* env, jclass convergenceClass, jobject convergence,
                        const char* fieldName)
{
    jfieldID fid = env->GetFieldID(convergenceClass, fieldName, "I");
    return env->GetIntField(convergence, fid);
}

void setIntField(JNIEnv* env, jclass convergenceClass, jobject convergence,
                        const char* fieldName, int value)
{
    jfieldID fid = env->GetFieldID(convergenceClass, fieldName, "I");
    env->SetIntField(convergence, fid, value);
}

void setArrayField(JNIEnv* env, jclass convergenceClass, jobject convergence,
                          const char* fieldName, int* inputArr)
{
    jfieldID fid = env->GetFieldID(convergenceClass, fieldName, "[I");
    jintArray array = (jintArray)env->GetObjectField(convergence, fid);

    int length = env->GetArrayLength(array);
    env->SetIntArrayRegion(array, 0, length, inputArr);
}

jobject createBitmap(JNIEnv* env, jintArray pixels, int width, int height)
{
    // create config object
    jclass configClass = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID fid = env->GetStaticFieldID(configClass, "ARGB_8888",
                                         "Landroid/graphics/Bitmap$Config;");
    jobject configObject = env->GetStaticObjectField(configClass, fid);

    // create bitmap object
    jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
    jmethodID mid = env->GetStaticMethodID(bitmapClass, "createBitmap",
                                           "([IIILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

    return env->CallStaticObjectMethod(bitmapClass, mid, pixels, width, height, configObject);
}

jobject createBitmapByRGBA(JNIEnv* env, jintArray jColors, int width, int height)
{
    SkBitmap bitmap;

    bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);

    jbyteArray buff = GraphicsJNI::allocateJavaPixelRef(env, &bitmap, NULL);
    if (NULL == buff) {
        return NULL;
    }

    if (jColors != NULL) {
        GraphicsJNI::SetPixels(env, jColors, 0, width, 0, 0, width, height, bitmap);
    }

    return GraphicsJNI::createBitmap(env, new SkBitmap(bitmap), buff, false, NULL, NULL);
}

void* getBitmapPixels(SkBitmap *nativeBitmap, int * width, int * height)
{
    const SkBitmap& bitmap(*nativeBitmap);
    SkAutoLockPixels autoLockPixelsL(bitmap);

    if (bitmap.isNull())
    {
        XLOGE("Stereo3D getBitmapPixels() Bitmap is null");
        return NULL;
    }

    if (NULL != width)
    {
        *width = bitmap.width();
    }

    if (NULL != height)
    {
        *height = bitmap.height();
    }

    return bitmap.getPixels();
}

void getBitmapSize(SkBitmap *nativeBitmap, int * width, int * height)
{
    const SkBitmap& bitmap(*nativeBitmap);
    SkAutoLockPixels autoLockPixelsL(bitmap);

    if (bitmap.isNull())
    {
        XLOGE("Stereo3D getBitmapSize() Bitmap is null");
        return;
    }
    *width = bitmap.width();
    *height = bitmap.height();
}

void SW_RGBA8888toYV12(char *data_rgb, char *pDst, int width, int height)
{

    unsigned char *data_yuv = (unsigned char *)pDst;

    int x, y, row_stride = width * 3;
    unsigned char *rgb, *Y, *U, *V;

    unsigned char u00, u01, u10, u11;
    unsigned char v00, v01, v10, v11;

    int RtoYCoeff = (int) ( 66 * 256);
    int GtoYCoeff = (int) (129 * 256);
    int BtoYCoeff = (int) ( 25 * 256);

    int RtoUCoeff = (int) (-38 * 256);
    int GtoUCoeff = (int) (-74 * 256);
    int BtoUCoeff = (int) (112 * 256);

    int RtoVCoeff = (int) (112 * 256);
    int GtoVCoeff = (int) (-94 * 256);
    int BtoVCoeff = (int) (-18 * 256);

    // Y plane
    rgb = (unsigned char *)data_rgb;
    Y   = (unsigned char *)data_yuv;

    for (y = height; y-- > 0; )
    {
        for (x = width; x-- > 0; )
        {
            // No need to saturate between 16 and 235
            *Y = 16 + ((32768 +
                        RtoYCoeff * *(rgb) +
                        GtoYCoeff * *(rgb + 1) +
                        BtoYCoeff * *(rgb + 2)) >> 16);
            Y++;
            rgb += 4;
        }
    }
}


