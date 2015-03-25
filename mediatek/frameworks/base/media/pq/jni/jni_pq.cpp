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

#include <jni_pq.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

int drvID = -1;
int ret = 0;
DISP_PQ_PARAM pqparam_original;
DISP_PQ_PARAM pqparam_camera;

JNIEXPORT void JNICALL
Java_com_mediatek_pq_PictureQuality_setCameraPreviewMode(JNIEnv* env, jobject thiz)
{   
    int drvID = -1;

    drvID = open("/dev/mtk_disp", O_RDONLY, 0);

    ioctl(drvID, DISP_IOCTL_GET_PQ_CAM_PARAM, &pqparam_camera);

    ioctl(drvID, DISP_IOCTL_SET_PQPARAM, &pqparam_camera);
    
    close(drvID);
  
    return;
}

JNIEXPORT void JNICALL
Java_com_mediatek_pq_PictureQuality_setGalleryNormalMode(JNIEnv* env, jobject thiz)
{

    int drvID = -1;
    
    drvID = open("/dev/mtk_disp", O_RDONLY, 0);
    
    ioctl(drvID, DISP_IOCTL_GET_PQ_GAL_PARAM, &pqparam_original);

    ioctl(drvID, DISP_IOCTL_SET_PQPARAM, &pqparam_original);
    
    close(drvID);

    return;
}