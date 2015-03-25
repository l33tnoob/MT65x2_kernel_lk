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

#define LOG_TAG "Stereo3DAntiFatigue"

#include <jni.h>
#include <JNIHelp.h>
#include "android_runtime/AndroidRuntime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MTKStereoKernel.h"
#include "GraphicsJNI.h"
#include <cutils/xlog.h>
#include "utility.h"
#include "CustomerTuningInfo.h"


#ifdef __cplusplus
extern "C" {
#endif

//Expected image width/height, value can be [w/1.2, w/1.1]
inline int toExpected(float in)
{
    return (int)(in / 1.2 + 0.5) / 10 * 10;
}

static void printEnvInfo(STEREO_KERNEL_SET_ENV_INFO_STRUCT *info)
{
    XLOGD("dump ENV_INFO_STRUCT");
    XLOGD("scenario: %d", info->scenario);
    XLOGD("working_buffer_size: %d", info->working_buffer_size);
    XLOGD("screen_layout: %d", info->screen_layout);
    XLOGD("source_image_width: %d", info->source_image_width);
    XLOGD("source_image_height: %d", info->source_image_height);
    XLOGD("crop_image_width: %d", info->crop_image_width);
    XLOGD("crop_image_height: %d", info->crop_image_height);
    XLOGD("rgba_image_width: %d", info->rgba_image_width);
    XLOGD("rgba_image_height: %d", info->rgba_image_height);
    XLOGD("rgba_image_stride: %d", info->rgba_image_stride);
    XLOGD("fefm_image_width: %d", info->fefm_image_width);
    XLOGD("fefm_image_height: %d", info->fefm_image_height);
    XLOGD("fefm_image_stride: %d", info->fefm_image_stride);
    XLOGD("fefm_image_format: %d", info->fefm_image_format);
    XLOGD("isUseHWFE: %d", info->isUseHWFE);
    XLOGD("hw_block_size: %d", info->hw_block_size);
    XLOGD("learning_data: 0x%x", info->learning_data);
    XLOGD("cl_correction: %d", info->cl_correction);
    XLOGD("enable_gpu: %d", info->enable_gpu);
    XLOGD("mtk3Dtag: %d", info->mtk3Dtag);
    XLOGD("pic_idx_for_warp: %d", info->pic_idx_for_warp);
    XLOGD("left_ratio_crop_start_x: %d", info->left_ratio_crop_start_x);
    XLOGD("left_ratio_crop_start_y: %d", info->left_ratio_crop_start_y);
    XLOGD("right_ratio_crop_start_x: %d", info->right_ratio_crop_start_x);
    XLOGD("right_ratio_crop_start_y: %d", info->right_ratio_crop_start_y);
    XLOGD("ptuning_para: 0x%x", info->ptuning_para);
}



static void printProcessInfo(STEREO_KERNEL_SET_PROC_INFO_STRUCT *info)
{
    XLOGD("dump PROC_INFO_STRUCT");
    XLOGD("source_image_right_addr: 0x%x", info->source_image_right_addr);
    XLOGD("fefm_image_left_addr: 0x%x", info->fefm_image_left_addr);
    XLOGD("fefm_image_right_addr: 0x%x", info->fefm_image_right_addr);
    XLOGD("rgba_image_left_addr: 0x%x", info->rgba_image_left_addr);
    XLOGD("rgba_image_right_addr: 0x%x", info->rgba_image_right_addr);
    XLOGD("warped_image_addr: 0x%x", info->warped_image_addr);
    XLOGD("hwfe_data_left: 0x%x", info->hwfe_data_left);
    XLOGD("hwfe_data_right: 0x%x", info->hwfe_data_right);
}

static void printResultInfo(STEREO_KERNEL_RESULT_STRUCT *info)
{
    int i;
    char buffer[256];
    char *ptr;
    XLOGD("dump RESULT_INFO_STRUCT");
    XLOGD("left_offset_x: %d", info->left_offset_x);
    XLOGD("left_offset_y: %d", info->left_offset_y);
    XLOGD("right_offset_x: %d", info->right_offset_x);
    XLOGD("right_offset_y: %d", info->right_offset_y);

    ptr = buffer;
    for(i=0 ; i<9 ; i++) {
        ptr += sprintf(ptr, "%d ", info->cropping_offsetX_L[i]);
    }
    XLOGD("cropping_offsetX_L: %s", buffer);

    ptr = buffer;
    for(i=0 ; i<9 ; i++) {
        ptr += sprintf(ptr, "%d ", info->cropping_offsetX_R[i]);
    }
    XLOGD("cropping_offsetX_R: %s", buffer);

    XLOGD("cropping_interval_default: %d", info->cropping_interval_default);

    ptr = buffer;
    for(i=0 ; i<9 ; i++) {
        ptr += sprintf(ptr, "%d ", info->cropping_size_width[i]);
    }
    XLOGD("cropping_size_width: %s", buffer);

    ptr = buffer;
    for(i=0 ; i<9 ; i++) {
        ptr += sprintf(ptr, "%d ", info->cropping_size_height[i]);
    }
    XLOGD("cropping_size_height: %s", buffer);

    XLOGD("cropping_offsetY_L: %d", info->cropping_offsetY_L);
    XLOGD("cropping_offsetY_R: %d", info->cropping_offsetY_R);
    XLOGD("isBounded: %d", info->isBounded);
    XLOGD("pt_info_NVRAM: 0x%x", info->pt_info_NVRAM);
}

JNIEXPORT jint JNICALL Java_com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_native_init(JNIEnv* env, jobject thiz, jint mtk3dtag)
{
    jclass cls;
    MTKStereoKernel* MyStereo = NULL;
    STEREO_KERNEL_SET_PROC_INFO_STRUCT      *S3DKProcInfo;
    STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT  *S3DKWorkBufInfo;
    STEREO_KERNEL_SET_ENV_INFO_STRUCT       *S3DKInitInfo;
    STEREO_KERNEL_TUNING_PARA_STRUCT        *S3DKTuningParaInfo;
    STEREO_KERNEL_RESULT_STRUCT             *S3DKResultInfo;

    XLOGD("Stereo3D AntiFatigue init()");

    cls = env->GetObjectClass(thiz);
    MyStereo = (MTKStereoKernel *)getIntField(env, cls, thiz, "mNativeMyStereo");
    S3DKProcInfo = (STEREO_KERNEL_SET_PROC_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKProcInfo");
    S3DKWorkBufInfo = (STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKWorkBufInfo");
    S3DKInitInfo = (STEREO_KERNEL_SET_ENV_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKInitInfo");
    S3DKTuningParaInfo = (STEREO_KERNEL_TUNING_PARA_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKTuningParaInfo");
    S3DKResultInfo = (STEREO_KERNEL_RESULT_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKResultInfo");

    XLOGD("get MyStereo: 0x%x", MyStereo);
    XLOGD("get S3DKProcInfo: 0x%x", S3DKProcInfo);
    XLOGD("get S3DKWorkBufInfo: 0x%x", S3DKWorkBufInfo);
    XLOGD("get S3DKInitInfo: 0x%x", S3DKInitInfo);
    XLOGD("get S3DKTuningParaInfo: 0x%x", S3DKTuningParaInfo);
    XLOGD("get S3DKResultInfo: 0x%x", S3DKResultInfo);

    if (NULL == MyStereo)
    {
        MyStereo = MTKStereoKernel::createInstance();
        setIntField(env, cls, thiz, "mNativeMyStereo", (int)MyStereo);
    }

    if (NULL == S3DKProcInfo)
    {
        S3DKProcInfo = (STEREO_KERNEL_SET_PROC_INFO_STRUCT *)malloc(sizeof(STEREO_KERNEL_SET_PROC_INFO_STRUCT));
        memset(S3DKProcInfo, 0, sizeof(STEREO_KERNEL_SET_PROC_INFO_STRUCT));
        setIntField(env, cls, thiz, "mNativeS3DKProcInfo", (int)S3DKProcInfo);
    }

    if (NULL == S3DKWorkBufInfo)
    {
        S3DKWorkBufInfo = (STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT *)malloc(sizeof(STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT));
        memset(S3DKWorkBufInfo, 0, sizeof(STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT));
        setIntField(env, cls, thiz, "mNativeS3DKWorkBufInfo", (int)S3DKWorkBufInfo);
    }

    if (NULL == S3DKInitInfo)
    {
        S3DKInitInfo = (STEREO_KERNEL_SET_ENV_INFO_STRUCT *)malloc(sizeof(STEREO_KERNEL_SET_ENV_INFO_STRUCT));
        memset(S3DKInitInfo, 0, sizeof(STEREO_KERNEL_SET_ENV_INFO_STRUCT));
        setIntField(env, cls, thiz, "mNativeS3DKInitInfo", (int)S3DKInitInfo);
    }

    if (NULL == S3DKTuningParaInfo)
    {
        S3DKTuningParaInfo = (STEREO_KERNEL_TUNING_PARA_STRUCT *)malloc(sizeof(STEREO_KERNEL_TUNING_PARA_STRUCT));
        memset(S3DKTuningParaInfo, 0, sizeof(STEREO_KERNEL_TUNING_PARA_STRUCT));
        setIntField(env, cls, thiz, "mNativeS3DKTuningParaInfo", (int)S3DKTuningParaInfo);
    }

    if (NULL == S3DKResultInfo)
    {
        S3DKResultInfo = (STEREO_KERNEL_RESULT_STRUCT *)malloc(sizeof(STEREO_KERNEL_RESULT_STRUCT));
        memset(S3DKResultInfo, 0, sizeof(STEREO_KERNEL_RESULT_STRUCT));
        setIntField(env, cls, thiz, "mNativeS3DKResultInfo", (int)S3DKResultInfo);
    }

    XLOGD("set MyStereo: 0x%x", MyStereo);
    XLOGD("set S3DKProcInfo: 0x%x", S3DKProcInfo);
    XLOGD("set S3DKWorkBufInfo: 0x%x", S3DKWorkBufInfo);
    XLOGD("set S3DKInitInfo: 0x%x", S3DKInitInfo);
    XLOGD("set S3DKTuningParaInfo: 0x%x", S3DKTuningParaInfo);
    XLOGD("set S3DKResultInfo: 0x%x", S3DKResultInfo);

    // step 1
    MyStereo->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_DEFAULT_TUNING, NULL, S3DKTuningParaInfo);


    // default value and can be modified by customer
    S3DKTuningParaInfo->customer_tuning_info.cc_thr             = ANTI_FATIGUE_DEFAULT_CC_THR;
    S3DKTuningParaInfo->customer_tuning_info.conv_behavior      = ANTI_FATIGUE_DEFAULT_CONV_BEHAVIOR;
    S3DKTuningParaInfo->customer_tuning_info.cc_gap_img         = ANTI_FATIGUE_DEFAULT_CC_GAP_IMG;
    S3DKTuningParaInfo->customer_tuning_info.conv_min_deg_h     = ANTI_FATIGUE_DEFAULT_CONV_MIN_DEGREE_H;
    S3DKTuningParaInfo->customer_tuning_info.conv_max_deg_h     = ANTI_FATIGUE_DEFAULT_CONV_MAX_DEGREE_H;
    S3DKTuningParaInfo->customer_tuning_info.conv_min_deg_v     = ANTI_FATIGUE_DEFAULT_CONV_MIN_DEGREE_V;
    S3DKTuningParaInfo->customer_tuning_info.conv_max_deg_v     = ANTI_FATIGUE_DEFAULT_CONV_MAX_DEGREE_V;


    S3DKInitInfo->source_image_width   = 800; // single side
    S3DKInitInfo->source_image_height  = 900; // single side
    S3DKInitInfo->crop_image_width     = toExpected((float)S3DKInitInfo->source_image_width);
    S3DKInitInfo->crop_image_height    = toExpected((float)S3DKInitInfo->source_image_height);
    S3DKInitInfo->ptuning_para         = S3DKTuningParaInfo;
    S3DKInitInfo->scenario             = STEREO_KERNEL_SCENARIO_IMAGE_PLAYBACK;
    S3DKInitInfo->screen_layout        = DISP_SCEREEN_LAYOUT_HORIZONTAL ;
    S3DKInitInfo->pic_idx_for_warp     = 1; // 0: LEFT, 1: RIGHT

    S3DKInitInfo->fefm_image_width     = 0; // S3DKInitInfo->source_image_width/4 ;
    S3DKInitInfo->fefm_image_height    = 0; // S3DKInitInfo->source_image_height/4 ;
    S3DKInitInfo->fefm_image_format    = INPUT_FORMAT_RGBA;
    S3DKInitInfo->fefm_image_stride    = 0;
    S3DKInitInfo->rgba_image_width     = 0; // S3DKInitInfo->source_image_width/16 ;
    S3DKInitInfo->rgba_image_height    = 0; // S3DKInitInfo->source_image_height/16;
    S3DKInitInfo->rgba_image_stride    = 0;

    S3DKInitInfo->isUseHWFE            = 0;  // 1: Use HW FE data,  0: Use SW FE
    S3DKInitInfo->hw_block_size        = 16; // BLOCK_SIZE;
    S3DKInitInfo->cl_correction        = 1;  // 1: Enable color corection
    S3DKInitInfo->enable_gpu           = 1;  // 1: GPU , 0: CPU
    S3DKInitInfo->mtk3Dtag             = mtk3dtag; // 1 for N3D image(picture from MTK's n3d camera), 0: other picture

    return 0;
}


JNIEXPORT jobject JNICALL Java_com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_native_process(JNIEnv* env, jobject thiz,
    jobject bitmapR, jobject bmRight16, jobject bmLeft16, jobject bmRight4, jobject bmLeft4,
    jint operation, jint screen_layout, jobject antiFatigueInfo)
{
    XLOGD("Stereo3D AntiFatigue process()");
    MRESULT result = 0;
    int src_img_width = 0;
    int src_img_heigh = 0;

    jclass cls;
    MTKStereoKernel* MyStereo = NULL;
    STEREO_KERNEL_SET_PROC_INFO_STRUCT      *S3DKProcInfo;
    STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT  *S3DKWorkBufInfo;
    STEREO_KERNEL_SET_ENV_INFO_STRUCT       *S3DKInitInfo;
    STEREO_KERNEL_RESULT_STRUCT             *S3DKResultInfo;
    MUINT8* WorkBuf;

    cls = env->GetObjectClass(thiz);
    MyStereo = (MTKStereoKernel *)getIntField(env, cls, thiz, "mNativeMyStereo");
    S3DKProcInfo = (STEREO_KERNEL_SET_PROC_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKProcInfo");
    S3DKWorkBufInfo = (STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKWorkBufInfo");
    S3DKInitInfo = (STEREO_KERNEL_SET_ENV_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKInitInfo");
    S3DKResultInfo = (STEREO_KERNEL_RESULT_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKResultInfo");

    XLOGD("get MyStereo: 0x%x", MyStereo);
    XLOGD("get S3DKProcInfo: 0x%x", S3DKProcInfo);
    XLOGD("get S3DKWorkBufInfo: 0x%x", S3DKWorkBufInfo);
    XLOGD("get S3DKInitInfo: 0x%x", S3DKInitInfo);
    XLOGD("get S3DKResultInfo: 0x%x", S3DKResultInfo);

    if ((NULL == S3DKProcInfo || NULL == S3DKWorkBufInfo) ||
        (NULL == S3DKInitInfo || NULL == S3DKResultInfo))
    {
        return NULL;
    }

    if (NULL == MyStereo)
    {
        XLOGD("Not init() first");
        return NULL;
    }

    SkBitmap* bm = GraphicsJNI::getNativeBitmap(env, bitmapR);
    if (NULL == bm)
    {
        return NULL;
    }

    // make sure memory for output image can be allocated
    getBitmapSize(bm, &src_img_width, &src_img_heigh);
    jintArray outputImage = env->NewIntArray(src_img_width * src_img_heigh);

    if (outputImage == NULL)
    {
        XLOGD("Fail to allocate output image memory");
        return NULL;
    }

    S3DKInitInfo->source_image_width  = src_img_width;
    S3DKInitInfo->source_image_height = src_img_heigh;
    S3DKInitInfo->crop_image_width    = toExpected((float)S3DKInitInfo->source_image_width);
    S3DKInitInfo->crop_image_height   = toExpected((float)S3DKInitInfo->source_image_height);
    S3DKInitInfo->scenario            = (operation == 0) ? STEREO_KERNEL_SCENARIO_IMAGE_PLAYBACK : STEREO_KERNEL_SCENARIO_IMAGE_ZOOM;
    S3DKInitInfo->screen_layout       = (screen_layout == 0) ? DISP_SCEREEN_LAYOUT_HORIZONTAL : DISP_SCEREEN_LAYOUT_VERTICAL;

    S3DKProcInfo->source_image_right_addr = (MUINT32)getBitmapPixels(bm, &src_img_width, &src_img_heigh);

    if (operation == 0)
    {
        // 1/4 size left & right
        if (NULL != bmLeft4)
        {
            bm = GraphicsJNI::getNativeBitmap(env, bmLeft4);
            if (NULL != bm)
            {
                S3DKProcInfo->fefm_image_left_addr    = (MUINT32)getBitmapPixels(bm, &src_img_width, &src_img_heigh);
                S3DKInitInfo->fefm_image_width        = src_img_width;
                S3DKInitInfo->fefm_image_height       = src_img_heigh;
            }
        }
        if (NULL != bmRight4)
        {
            bm = GraphicsJNI::getNativeBitmap(env, bmRight4);
            if (NULL != bm)
            {
                S3DKProcInfo->fefm_image_right_addr   = (MUINT32)getBitmapPixels(bm, &src_img_width, &src_img_heigh);
            }
        }

        // 1/16 size left & right
        if (NULL != bmLeft16)
        {
            bm = GraphicsJNI::getNativeBitmap(env, bmLeft16);
            if (NULL != bm)
            {
                S3DKProcInfo->rgba_image_left_addr    = (MUINT32)getBitmapPixels(bm, &src_img_width, &src_img_heigh);
                S3DKInitInfo->rgba_image_width        = src_img_width;
                S3DKInitInfo->rgba_image_height       = src_img_heigh;
            }
        }
        if (NULL != bmRight16)
        {
            bm = GraphicsJNI::getNativeBitmap(env, bmRight16);
            if (NULL != bm)
            {
                S3DKProcInfo->rgba_image_right_addr   = (MUINT32)getBitmapPixels(bm, &src_img_width, &src_img_heigh);
            }
        }
    }


    // output image
    jint* outputImagePtr = env->GetIntArrayElements(outputImage, 0);
    S3DKProcInfo->warped_image_addr = (MUINT32) outputImagePtr;


    // step 1 set initial info based on init() done
    printEnvInfo(S3DKInitInfo);
    result |= MyStereo->StereoKernelInit(S3DKInitInfo);
    XLOGD("set initial info");


    if (operation == 0)
    {
        // step 2 set work info
        WorkBuf = (MUINT8*)S3DKWorkBufInfo->ext_mem_start_addr;
        if (WorkBuf != NULL)
        {
            free(WorkBuf);
        }
        WorkBuf = (MUINT8*)malloc(sizeof(MUINT8) * S3DKInitInfo->working_buffer_size);
        S3DKWorkBufInfo->ext_mem_size = S3DKInitInfo->working_buffer_size;
        S3DKWorkBufInfo->ext_mem_start_addr = (MUINT32)WorkBuf;
        XLOGD("ext_mem_size: %d", S3DKWorkBufInfo->ext_mem_size);
        XLOGD("ext_mem_start_addr: 0x%x", S3DKWorkBufInfo->ext_mem_start_addr);
        result |= MyStereo->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_WORK_BUF_INFO, S3DKWorkBufInfo, NULL);
        XLOGD("set worked info");
    }


    // step 3 set process info
    printProcessInfo(S3DKProcInfo);
    result |= MyStereo->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_PROC_INFO, S3DKProcInfo, NULL);
    XLOGD("set process info");


    // setp 4 run
    result |= MyStereo->StereoKernelMain();
    XLOGD("run");


    // Step 5 get result
    result |= MyStereo->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_RESULT, NULL, S3DKResultInfo);

    jobject bitmap = NULL;
    if (result == 0)
    {
        // create bitmap
        bitmap = createBitmap(env, outputImage, S3DKInitInfo->source_image_width, S3DKInitInfo->source_image_height);
        printResultInfo(S3DKResultInfo);
    }

    // release array
    env->ReleaseIntArrayElements(outputImage, outputImagePtr, JNI_ABORT);
    env->DeleteLocalRef(outputImage);
    XLOGD("get result");

    // step 6 assign to Java
    if (result == 0)
    {
        jclass InfoClass = env->FindClass("com/mediatek/effect/effects/Stereo3DAntiFatigueEffect$AntiFatigueInfo");

        if (NULL != antiFatigueInfo)
        {
            setIntField(env, InfoClass, antiFatigueInfo, "mLeftOffsetX", S3DKResultInfo->left_offset_x);
            setIntField(env, InfoClass, antiFatigueInfo, "mLeftOffsetY", S3DKResultInfo->left_offset_y);
            setIntField(env, InfoClass, antiFatigueInfo, "mRightOffsetX", S3DKResultInfo->right_offset_x);
            setIntField(env, InfoClass, antiFatigueInfo, "mRightOffsetY", S3DKResultInfo->right_offset_y);
            setIntField(env, InfoClass, antiFatigueInfo, "mCroppingIntervalDefault", S3DKResultInfo->cropping_interval_default);
            setIntField(env, InfoClass, antiFatigueInfo, "mCroppingOffectY_L", S3DKResultInfo->cropping_offsetY_L);
            setIntField(env, InfoClass, antiFatigueInfo, "mCroppingOffectY_R", S3DKResultInfo->cropping_offsetY_R);

            setArrayField(env, InfoClass, antiFatigueInfo, "mCroppingOffectX_L", S3DKResultInfo->cropping_offsetX_L);
            setArrayField(env, InfoClass, antiFatigueInfo, "mCroppingOffectX_R", S3DKResultInfo->cropping_offsetX_R);
            setArrayField(env, InfoClass, antiFatigueInfo, "mCroppingSizeWidth", S3DKResultInfo->cropping_size_width);
            setArrayField(env, InfoClass, antiFatigueInfo, "mCroppingSizeHeight", S3DKResultInfo->cropping_size_height);

            XLOGD("assign to JAVA");
        }
    }

    return bitmap;
}

JNIEXPORT jint JNICALL Java_com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_native_close(JNIEnv* env, jobject thiz)
{
    jclass cls;
    MTKStereoKernel* MyStereo = NULL;
    STEREO_KERNEL_SET_PROC_INFO_STRUCT      *S3DKProcInfo;
    STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT  *S3DKWorkBufInfo;
    STEREO_KERNEL_SET_ENV_INFO_STRUCT       *S3DKInitInfo;
    STEREO_KERNEL_TUNING_PARA_STRUCT        *S3DKTuningParaInfo;
    STEREO_KERNEL_RESULT_STRUCT             *S3DKResultInfo;
    MUINT8* WorkBuf;

    XLOGD("Stereo3D AntiFatigue close()");

    cls = env->GetObjectClass(thiz);
    MyStereo = (MTKStereoKernel *)getIntField(env, cls, thiz, "mNativeMyStereo");
    S3DKProcInfo = (STEREO_KERNEL_SET_PROC_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKProcInfo");
    S3DKWorkBufInfo = (STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKWorkBufInfo");
    S3DKInitInfo = (STEREO_KERNEL_SET_ENV_INFO_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKInitInfo");
    S3DKTuningParaInfo = (STEREO_KERNEL_TUNING_PARA_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKTuningParaInfo");
    S3DKResultInfo = (STEREO_KERNEL_RESULT_STRUCT *)getIntField(env, cls, thiz, "mNativeS3DKResultInfo");

    XLOGD("get MyStereo: 0x%x", MyStereo);
    XLOGD("get S3DKProcInfo: 0x%x", S3DKProcInfo);
    XLOGD("get S3DKWorkBufInfo: 0x%x", S3DKWorkBufInfo);
    XLOGD("get S3DKInitInfo: 0x%x", S3DKInitInfo);
    XLOGD("get S3DKTuningParaInfo: 0x%x", S3DKTuningParaInfo);
    XLOGD("get S3DKResultInfo: 0x%x", S3DKResultInfo);

    if (S3DKProcInfo)
    {
        free(S3DKProcInfo);
        setIntField(env, cls, thiz, "mNativeS3DKProcInfo", 0);
    }

    if (S3DKWorkBufInfo)
    {
        WorkBuf = (MUINT8*)S3DKWorkBufInfo->ext_mem_start_addr;
        if (NULL != WorkBuf) {
            free(WorkBuf);
        }
        free(S3DKWorkBufInfo);
        setIntField(env, cls, thiz, "mNativeS3DKWorkBufInfo", 0);
    }

    if (S3DKInitInfo)
    {
        free(S3DKInitInfo);
        setIntField(env, cls, thiz, "mNativeS3DKInitInfo", 0);
    }

    if (S3DKTuningParaInfo)
    {
        free(S3DKTuningParaInfo);
        setIntField(env, cls, thiz, "mNativeS3DKTuningParaInfo", 0);
    }

    if (S3DKResultInfo)
    {
        free(S3DKResultInfo);
        setIntField(env, cls, thiz, "mNativeS3DKResultInfo", 0);
    }

    if (NULL != MyStereo)
    {
        MyStereo->StereoKernelReset();
        MyStereo->destroyInstance(); // would do "delete MyStereo"
        MyStereo = NULL;
        setIntField(env, cls, thiz, "mNativeMyStereo", 0);
    }
    return 0;
}

static JNINativeMethod com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_methods[] =
{
    {
        "native_init", "(I)I",
        (void*)Java_com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_native_init
    },
    {
        "native_process", "(Landroid/graphics/Bitmap;Landroid/graphics/Bitmap;Landroid/graphics/Bitmap;Landroid/graphics/Bitmap;Landroid/graphics/Bitmap;IILcom/mediatek/effect/effects/Stereo3DAntiFatigueEffect$AntiFatigueInfo;)Landroid/graphics/Bitmap;",
        (void*)Java_com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_native_process
    },
    {
        "native_close", "()I",
        (void*)Java_com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_native_close
    },
};

int register_com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_methods(JNIEnv* env)
{
    return android::AndroidRuntime::registerNativeMethods(env,
            "com/mediatek/effect/filterpacks/Stereo3DAntiFatigueFilter",
            com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_methods,
            NELEM(com_mediatek_effect_filterpacks_Stereo3DAntiFatigueFilter_methods));
}

#ifdef __cplusplus
}
#endif

