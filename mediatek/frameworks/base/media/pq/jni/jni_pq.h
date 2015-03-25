/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef COM_ANDROID_JNI_PQ_H
#define COM_ANDROID_JNI_PQ_H

#include <jni.h>
#include <linux/ioctl.h>
#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG(msg...) __android_log_print(ANDROID_LOG_VERBOSE, "PQ", msg)

#define PQ_HUE_ADJ_PHASE_CNT 4
#define PQ_SAT_ADJ_PHASE_CNT 4

typedef struct {
    unsigned long u4SHPGain;// 0 : min , 9 : max.
    unsigned long u4SatGain;// 0 : min , 9 : max.
    unsigned long u4HueAdj[PQ_HUE_ADJ_PHASE_CNT];
    unsigned long u4SatAdj[PQ_SAT_ADJ_PHASE_CNT];
} DISP_PQ_PARAM;

#define DISP_IOCTL_MAGIC        'x'
#define DISP_IOCTL_SET_PQPARAM          _IOW    (DISP_IOCTL_MAGIC, 18 , DISP_PQ_PARAM)
#define DISP_IOCTL_GET_PQ_CAM_PARAM     _IOR    (DISP_IOCTL_MAGIC, 58 , DISP_PQ_PARAM)
#define DISP_IOCTL_GET_PQ_GAL_PARAM     _IOR    (DISP_IOCTL_MAGIC, 60 , DISP_PQ_PARAM)

JNIEXPORT void JNICALL
Java_com_mediatek_pq_PictureQuality_setCameraPreviewMode(JNIEnv* env, jobject thiz);

JNIEXPORT void JNICALL
Java_com_mediatek_pq_PictureQuality_setGalleryNormalMode(JNIEnv* env, jobject thiz);

#ifdef __cplusplus
}
#endif

#endif  /* COM_ANDROID_JNI_PQ_H */