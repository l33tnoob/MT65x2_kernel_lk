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

#define LOG_TAG "MtkCameraProfile"

#include <camera/MtkCameraProfile.h>


/******************************************************************************
 * 
 ******************************************************************************/
namespace CFPT {


static bool gbInit = false; 

// Event, parent, name 
static CPT_Event_Info gCPTEventInfo[] =
{
    {Event_Camera_Framework_Root, EVENT_CAMERA_ROOT, "CameraFramework"}

    // Define the event info used in Cameraservice
    ,{Event_CameraService, Event_Camera_Framework_Root, "CameraService"}
        ,{Event_CS_playSound, Event_CameraService, "playSound"}
        ,{Event_CS_newMediaPlayer, Event_CameraService, "newMediaPlayer"}
        ,{Event_CS_connect, Event_CameraService, "CS_connect"}
            ,{Event_CS_connectFinishUnsafe, Event_CS_connect, "connectFinishUnsafe"}
            ,{Event_CS_Client_Ctor, Event_CS_connect, "Client_Ctor"}

    ,{Event_CameraClient, Event_Camera_Framework_Root, "CameraClient"}
        ,{Event_C1C_disconnect, Event_CameraClient, "disconnect"}
        ,{Event_C1C_disconnectWindow, Event_CameraClient, "disconnectWindow"}
        ,{Event_C1C_setPreviewWindow, Event_CameraClient, "setPreviewWindow"}
        ,{Event_C1C_getParameters, Event_CameraClient, "getParameters"}
        ,{Event_C1C_setParameters, Event_CameraClient, "setParameters"}
        ,{Event_C1C_sendCommand, Event_CameraClient, "sendCommand"}
        ,{Event_C1C_takePicture, Event_CameraClient, "takePicture"}
        ,{Event_C1C_startPreview, Event_CameraClient, "startPreview"}
        ,{Event_C1C_stopPreview, Event_CameraClient, "stopPreview"}
        ,{Event_C1C_startRecording, Event_CameraClient, "startRecording"}
        ,{Event_C1C_stopRecording, Event_CameraClient, "stopRecording"}
        ,{Event_C1C_releaseRecordingFrame, Event_CameraClient, "releaseRecordingFrame"}
        ,{Event_C1C_dataCallbackTimestamp, Event_CameraClient, "dataCallbackTimestamp"}

};


bool initCameraProfile()
{
    bool ret = false;
    if  ( ! gbInit )
    {
        ret = RegisterEvents(gCPTEventInfo, sizeof(gCPTEventInfo) / sizeof(gCPTEventInfo[0]));
        gbInit = ret;
    }
    return ret;
}


/******************************************************************************
 * 
 ******************************************************************************/
};  // namespace CFPT

