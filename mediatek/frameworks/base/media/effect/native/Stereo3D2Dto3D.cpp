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

#define LOG_TAG "Stereo3D2Dto3D"

#include <jni.h>
#include <JNIHelp.h>
#include <string.h>
#include "android_runtime/AndroidRuntime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cutils/xlog.h>
#include "GraphicsJNI.h"
#include "utility.h"
#include "MTKTo3d.h"


// tunning parameters
#define TUNE_BASE_LINE 1.4f
#define TUNE_GLOBAL_THR_RATIO 0.65f // range[0.5,0.8]
#define TUNE_GLOBAL_WEIGHTING 8     // range[8,16]
#define TUNE_SCENE_CHANGE_THR 80    // range[30,100]

#ifdef __cplusplus
extern "C" {
#endif

static void printInitInfo(TO3D_SET_ENV_INFO_STRUCT *info)
{
    XLOGD("dump ENV_INFO_STRUCT");
    XLOGD("large_image_width: %d", info->large_image_width);
    XLOGD("large_image_height: %d", info->large_image_height);
    XLOGD("small_image_width: %d", info->small_image_width);
    XLOGD("small_image_height: %d", info->small_image_height);
    XLOGD("large_image_format: %d", info->large_image_format);
    XLOGD("small_image_format: %d", info->small_image_format);
    XLOGD("to3d_scenario: %d", info->to3d_scenario);
    XLOGD("to3d_tuning_data: %x", &(info->to3d_tuning_data));
}


static void printProcInfo(TO3D_SET_PROC_INFO_STRUCT *info)
{
    XLOGD("dump PROC_INFO_STRUCT");
    XLOGD("output_image_width: %d", info->output_image_width);
    XLOGD("output_image_height: %d", info->output_image_height);
    XLOGD("large_image_addr: %x", info->large_image_addr);
    XLOGD("small_image_addr: %x", info->small_image_addr);
    XLOGD("output_image_addr: %x", info->output_image_addr);
    XLOGD("angle: %d", info->angle);
    XLOGD("large_image_texID: %d", info->large_image_texID);
    XLOGD("output_image_texID: %d", info->output_image_texID);
    XLOGD("output_image_fboID: %d", info->output_image_fboID);
}



JNIEXPORT jint Java_com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_init(JNIEnv* env, jobject thiz,
        jint inputWidth, jint inputHeight)
{
    jint result = 0;
    jclass cls;
    MTKTo3d *MyTo3d;
    TO3D_SET_ENV_INFO_STRUCT *MyTo3dInitInfo;
    TO3D_SET_PROC_INFO_STRUCT *MyTo3dProcInfo;

    XLOGD("Stereo3D 2Dto3D init()");

    cls = env->GetObjectClass(thiz);
    MyTo3d = (MTKTo3d *)getIntField(env, cls, thiz, "mNativeMyTo3d");
    MyTo3dInitInfo = (TO3D_SET_ENV_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeInitInfo");
    MyTo3dProcInfo = (TO3D_SET_PROC_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeProcInfo");

    XLOGD("get MyTo3d: 0x%x", MyTo3d);
    XLOGD("get MyTo3dInitInfo: 0x%x", MyTo3dInitInfo);
    XLOGD("get MyTo3dProcInfo: 0x%x", MyTo3dProcInfo);

    if (NULL == MyTo3d)
    {
        MyTo3d = MTKTo3d::createInstance();
        setIntField(env, cls, thiz, "mNativeMyTo3d", (int)MyTo3d);
    }

    if (NULL ==MyTo3dInitInfo)
    {
        MyTo3dInitInfo = (TO3D_SET_ENV_INFO_STRUCT *)malloc(sizeof(TO3D_SET_ENV_INFO_STRUCT));
        memset(MyTo3dInitInfo, 0, sizeof(TO3D_SET_ENV_INFO_STRUCT));
        setIntField(env, cls, thiz, "mNativeInitInfo", (int)MyTo3dInitInfo);
    }

    if (NULL == MyTo3dProcInfo)
    {
        MyTo3dProcInfo = (TO3D_SET_PROC_INFO_STRUCT *)malloc(sizeof(TO3D_SET_PROC_INFO_STRUCT));
        memset(MyTo3dProcInfo, 0, sizeof(TO3D_SET_PROC_INFO_STRUCT));
        setIntField(env, cls, thiz, "mNativeProcInfo", (int)MyTo3dProcInfo);
    }

    XLOGD("set MyTo3d: 0x%x", MyTo3d);
    XLOGD("set MyTo3dInitInfo: 0x%x", MyTo3dInitInfo);
    XLOGD("set MyTo3dProcInfo: 0x%x", MyTo3dProcInfo);

    MyTo3dInitInfo->large_image_format = TO3D_IMAGE_FORMAT_RGBA8888;
    MyTo3dInitInfo->small_image_format = TO3D_IMAGE_FORMAT_LUMA;
    MyTo3dInitInfo->to3d_scenario = TO3D_STILL_IMAGE_PLAYBACK;
    MyTo3dInitInfo->to3d_tuning_data.baseline = TUNE_BASE_LINE;
    MyTo3dInitInfo->to3d_tuning_data.global_thr_ratio = TUNE_GLOBAL_THR_RATIO;
    MyTo3dInitInfo->to3d_tuning_data.global_weighting = TUNE_GLOBAL_WEIGHTING;
    MyTo3dInitInfo->to3d_tuning_data.scene_change_thr = TUNE_SCENE_CHANGE_THR;
    MyTo3dInitInfo->large_image_height = inputHeight; // input height = output height
    MyTo3dInitInfo->large_image_width = inputWidth; // input width = output width
    MyTo3dInitInfo->small_image_width = 0;  // gMyTo3dInitInfo.large_image_width /10;
    MyTo3dInitInfo->small_image_height = 0; // gMyTo3dInitInfo.large_image_height/10;

    return result;
}

JNIEXPORT jint Java_com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_process(JNIEnv* env, jobject thiz,
        jint inputTextureId, jobject smallBitmap, jint outputTextureId, jint outputFboId)
{
    MRESULT result = 0;
    int small_img_width = 0;
    int small_img_heigh = 0;
    jclass cls;
    MTKTo3d *MyTo3d;
    TO3D_SET_ENV_INFO_STRUCT *MyTo3dInitInfo;
    TO3D_SET_PROC_INFO_STRUCT *MyTo3dProcInfo;
    TO3D_RESULT_STRUCT MyTo3dResult;
    
    XLOGD("Stereo3D 2Dto3D process()");

    cls = env->GetObjectClass(thiz);
    MyTo3d = (MTKTo3d *)getIntField(env, cls, thiz, "mNativeMyTo3d");
    MyTo3dInitInfo = (TO3D_SET_ENV_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeInitInfo");
    MyTo3dProcInfo = (TO3D_SET_PROC_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeProcInfo");

    XLOGD("get MyTo3d: 0x%x", MyTo3d);
    XLOGD("get MyTo3dInitInfo: 0x%x", MyTo3dInitInfo);
    XLOGD("get MyTo3dProcInfo: 0x%x", MyTo3dProcInfo);

    if (NULL == MyTo3d || (NULL == MyTo3dInitInfo || NULL == MyTo3dProcInfo))
    {
        return 1;
    }

    SkBitmap* bm = GraphicsJNI::getNativeBitmap(env, smallBitmap);
    if (NULL == bm)
    {
        return 1;
    }

    char *smallpic = (char *)getBitmapPixels(bm, &small_img_width, &small_img_heigh);
    char *luma_y_plane_array;

    luma_y_plane_array = (char *)malloc(small_img_width * small_img_heigh);

    SW_RGBA8888toYV12(smallpic, luma_y_plane_array, small_img_width, small_img_heigh);

    MyTo3dProcInfo->small_image_addr = (MUINT32)luma_y_plane_array;
    MyTo3dInitInfo->small_image_width = small_img_width;
    MyTo3dInitInfo->small_image_height = small_img_heigh;
    result |= MyTo3d->To3dInit(MyTo3dInitInfo,0);
    printInitInfo(MyTo3dInitInfo);


    MyTo3dProcInfo->output_image_height = MyTo3dInitInfo->large_image_height;
    MyTo3dProcInfo->output_image_width =  MyTo3dInitInfo->large_image_width;
    MyTo3dProcInfo->angle = 0;
    MyTo3dProcInfo->large_image_texID = inputTextureId;
    MyTo3dProcInfo->output_image_texID = outputTextureId;
    MyTo3dProcInfo->output_image_fboID = outputFboId;
    result |= MyTo3d->To3dFeatureCtrl(TO3D_FEATURE_SET_PROC_INFO, MyTo3dProcInfo, NULL);
    XLOGD("set process info");
    printProcInfo(MyTo3dProcInfo);

    result |= MyTo3d->To3dMain();
    XLOGD("run");

    free(luma_y_plane_array);

    if (0 == result)
    {
        result |= MyTo3d->To3dFeatureCtrl(TO3D_FEATURE_GET_RESULT, 0, &MyTo3dResult);
    }

    return result;
}

JNIEXPORT jint Java_com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_close(JNIEnv* env, jobject thiz)
{
    jint result = 0;
    jclass cls;
    MTKTo3d *MyTo3d;
    TO3D_SET_ENV_INFO_STRUCT *MyTo3dInitInfo;
    TO3D_SET_PROC_INFO_STRUCT *MyTo3dProcInfo;
    
    XLOGD("Stereo3D 2Dto3D close()");

    cls = env->GetObjectClass(thiz);
    MyTo3d = (MTKTo3d *)getIntField(env, cls, thiz, "mNativeMyTo3d");
    MyTo3dInitInfo = (TO3D_SET_ENV_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeInitInfo");
    MyTo3dProcInfo = (TO3D_SET_PROC_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeProcInfo");

    XLOGD("get MyTo3d: 0x%x", MyTo3d);
    XLOGD("get MyTo3dInitInfo: 0x%x", MyTo3dInitInfo);
    XLOGD("get MyTo3dProcInfo: 0x%x", MyTo3dProcInfo);

    if (NULL != MyTo3dInitInfo)
    {
        free(MyTo3dInitInfo);
        setIntField(env, cls, thiz, "mNativeInitInfo", 0);
    }

    if (NULL != MyTo3dProcInfo)
    {
        free(MyTo3dProcInfo);
        setIntField(env, cls, thiz, "mNativeProcInfo", 0);
    }

    if (NULL != MyTo3d)
    {
        MyTo3d->To3dReset();
        MyTo3d->destroyInstance();
        MyTo3d = NULL;
        setIntField(env, cls, thiz, "mNativeMyTo3d", 0);
    }
    return result;
}

static JNINativeMethod com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_methods[] =
{
    {
        "native_init", "(II)I",
        (void*)Java_com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_init
    },
    {
        "native_process",
        "(ILandroid/graphics/Bitmap;II)I",
        (void*)Java_com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_process
    },
    {
        "native_close", "()I",
        (void*)Java_com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_close
    },
};

int register_com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_methods(JNIEnv* env)
{
    return android::AndroidRuntime::registerNativeMethods(env,
            "com/mediatek/effect/filterpacks/Stereo3D2Dto3DFilter",
            com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_methods,
            NELEM(com_mediatek_effect_filterpacks_Stereo3D2Dto3DFilter_methods));
}

#ifdef __cplusplus
}
#endif

