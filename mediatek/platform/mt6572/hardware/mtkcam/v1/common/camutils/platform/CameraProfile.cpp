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

#define LOG_TAG "CameraProfile"

#include <common/camutils/CameraProfile.h>

static bool gbInit = false; 

// Event, parent , name 
static CPT_Event_Info gCPTEventInfo[] =
{
    {Event_Camera_Platform_Root, EVENT_CAMERA_ROOT, "CameraPlatform"}

        // Define the event used in Hal
        ,{Event_Hal, Event_Camera_Platform_Root, "Hal"}

            // Define the event used in Hal::Device
            ,{Event_Hal_Device, Event_Hal, "Device"}
                // Define the event used in Hal::Device::DefaultCamDevice
                ,{Event_Hal_DefaultCamDevice, Event_Hal_Device, "DefaultCamDevice"}
                ,{Event_Hal_DefaultCamDevice_init, Event_Hal_DefaultCamDevice, "init"}
                ,{Event_Hal_DefaultCamDevice_uninit, Event_Hal_DefaultCamDevice, "uninit"}

            // Define the event used in Hal::Adapter
            ,{Event_Hal_Adapter, Event_Hal, "Adapter"}
                // Define the event used in Hal::Adapter::Scenario
                ,{Event_Hal_Adapter_Scenario, Event_Hal_Adapter, "Scenario"}
                    // Define the event used in Hal::Adapter::Scenario::Shot
                    ,{Event_Hal_Adapter_Scenario_Shot, Event_Hal_Adapter_Scenario, "Shot"}

                        // --Define the event used in continuous shot
                        ,{Event_CShot, Event_Hal_Adapter_Scenario_Shot, "ContinuousShot"}
                            ,{Event_CShot_capture, Event_CShot, "capture"}
                            ,{Event_CShot_sendCmd, Event_CShot, "sendCommand"}
                            ,{Event_CShot_cancel, Event_CShot, "cancelCapture"}
                            ,{Event_CShot_handleNotifyCb, Event_CShot, "handleNotifyCb"}
                            ,{Event_CShot_handlePVData, Event_CShot, "handlePostViewData"}
                            ,{Event_CShot_handleJpegData, Event_CShot, "handleJpegData"}
                        // -- Define the event used in normal shot
                        ,{Event_Shot,  Event_Hal_Adapter_Scenario_Shot, "NormalShot"}
                            ,{Event_Shot_capture, Event_Shot, "capture"}
                            ,{Event_Shot_sendCmd, Event_Shot, "sendCommand"}
                            ,{Event_Shot_cancel, Event_Shot, "cancelCapture"}
                            ,{Event_Shot_handleNotifyCb, Event_Shot, "handleNotifyCb"}
                            ,{Event_Shot_handlePVData, Event_Shot, "handlePostViewData"}
                            ,{Event_Shot_handleJpegData, Event_Shot, "handleJpegData"}
                        // --Define the event info used in FaceBeautyShot
                        ,{Event_FcaeBeautyShot, Event_Hal_Adapter_Scenario_Shot, "FaceBeautyShot"}
            	            ,{Event_FBShot_createFullFrame, Event_FcaeBeautyShot, "createFullFrame"}
                            ,{Event_FBShot_STEP1, Event_FcaeBeautyShot, "STEP1"}
                            ,{Event_FBShot_STEP1Algo, Event_FcaeBeautyShot, "STEP1Algo"}
            	            ,{Event_FBShot_STEP2, Event_FcaeBeautyShot, "STEP2"}
            	            ,{Event_FBShot_STEP3, Event_FcaeBeautyShot, "STEP3"}
            	            ,{Event_FBShot_STEP4, Event_FcaeBeautyShot, "STEP4"}
            	            ,{Event_FBShot_STEP4Algo, Event_FcaeBeautyShot, "STEP4Algo"}
                            ,{Event_FBShot_STEP5, Event_FcaeBeautyShot, "STEP5"}
                            ,{Event_FBShot_STEP5Algo, Event_FcaeBeautyShot, "STEP5Algo"}
                            ,{Event_FBShot_STEP6, Event_FcaeBeautyShot, "STEP6"}
                            ,{Event_FBShot_createFBJpegImg, Event_FcaeBeautyShot, "createFBJpegImg"}
            	            ,{Event_FBShot_Utility, Event_FcaeBeautyShot, "Utility"}
            	            ,{Event_FBShot_requestBufs, Event_FBShot_Utility, "requestBufs"}
            	            ,{Event_FBShot_InitialAlgorithm, Event_FBShot_Utility, "InitialAlgorithm"}
                            ,{Event_FBShot_JpegEncodeImg, Event_FBShot_Utility, "JpegEncodeImg"}
                            ,{Event_FBShot_ResizeImg, Event_FBShot_Utility, "ResizeImg"}
                        // -- Define the event used in zsd shot
                        ,{Event_ZsdShot,  Event_Hal_Adapter_Scenario_Shot, "ZsdShot"}
                            ,{Event_ZsdShot_capture, Event_ZsdShot, "capture"}
                            ,{Event_ZsdShot_handleJpegData, Event_ZsdShot, "handleJpegData"}

                // Define the event used in Hal::Adapter::Preview
                ,{Event_Hal_Adapter_MtkPhotoPreview, Event_Hal_Adapter, "MtkPhotoPreview"}
                    ,{Event_Hal_Adapter_MtkPhotoPreview_start, Event_Hal_Adapter_MtkPhotoPreview, "start"}
                    ,{Event_Hal_Adapter_MtkPhotoPreview_start_init, Event_Hal_Adapter_MtkPhotoPreview, "start_init"}
                    ,{Event_Hal_Adapter_MtkPhotoPreview_start_stable, Event_Hal_Adapter_MtkPhotoPreview, "start_stable"}
                    ,{Event_Hal_Adapter_MtkPhotoPreview_proc, Event_Hal_Adapter_MtkPhotoPreview, "proc"}
                    ,{Event_Hal_Adapter_MtkPhotoPreview_precap, Event_Hal_Adapter_MtkPhotoPreview, "precap"}
                    ,{Event_Hal_Adapter_MtkPhotoPreview_stop, Event_Hal_Adapter_MtkPhotoPreview, "stop"}

                // Define the event used in Hal::Adapter::Default
                ,{Event_Hal_Adapter_MtkDefaultPreview, Event_Hal_Adapter, "MtkDefaultPreview"}
                    ,{Event_Hal_Adapter_MtkDefaultPreview_start, Event_Hal_Adapter_MtkDefaultPreview, "start"}
                    ,{Event_Hal_Adapter_MtkDefaultPreview_start_init, Event_Hal_Adapter_MtkDefaultPreview, "start_init"}
                    ,{Event_Hal_Adapter_MtkDefaultPreview_start_stable, Event_Hal_Adapter_MtkDefaultPreview, "start_stable"}
                    ,{Event_Hal_Adapter_MtkDefaultPreview_proc, Event_Hal_Adapter_MtkDefaultPreview, "proc"}
                    ,{Event_Hal_Adapter_MtkDefaultPreview_precap, Event_Hal_Adapter_MtkDefaultPreview, "precap"}
                    ,{Event_Hal_Adapter_MtkDefaultPreview_stop, Event_Hal_Adapter_MtkDefaultPreview, "stop"}
                    ,{Event_Hal_Adapter_MtkDefaultPreview_vss, Event_Hal_Adapter_MtkDefaultPreview, "vss"}

                // Define the event used in Hal::Adapter::ZSD
                ,{Event_Hal_Adapter_MtkZsdPreview, Event_Hal_Adapter, "MtkZsdPreview"}
                    ,{Event_Hal_Adapter_MtkZsdPreview_start, Event_Hal_Adapter_MtkZsdPreview, "start"}
                    ,{Event_Hal_Adapter_MtkZsdPreview_start_init, Event_Hal_Adapter_MtkZsdPreview, "start_init"}
                    ,{Event_Hal_Adapter_MtkZsdPreview_start_stable, Event_Hal_Adapter_MtkZsdPreview, "start_stable"}
                    ,{Event_Hal_Adapter_MtkZsdPreview_proc, Event_Hal_Adapter_MtkZsdPreview, "proc"}
                    ,{Event_Hal_Adapter_MtkZsdPreview_precap, Event_Hal_Adapter_MtkZsdPreview, "precap"}
                    ,{Event_Hal_Adapter_MtkZsdPreview_stop, Event_Hal_Adapter_MtkZsdPreview, "stop"}
                    ,{Event_Hal_Adapter_MtkZsdCapture, Event_Hal_Adapter, "MtkZsdCapture"}

            // Define the event used in Hal::Client
            ,{Event_Hal_Client, Event_Hal, "Client"}
                // Define the event used in Hal::Client::CamClient
                ,{Event_Hal_Client_CamClient, Event_Hal_Client, "CamClient"}
                    // Define the event used in Hal::Adapter::Scenario::Shot
                    ,{Event_Hal_Client_CamClient_FD, Event_Hal_Client_CamClient, "FD"}


        // Define the event used in Core
        ,{Event_Core, Event_Camera_Platform_Root, "Core"}
            // Define the event used in Core::CamShot
            ,{Event_Core_CamShot, Event_Core, "CamShot"}
                // --Define the event used in multi shot
                ,{Event_MShot, Event_Core_CamShot, "MultiShot"}
                    ,{Event_MShot_init, Event_MShot, "init"}
                    ,{Event_MShot_uninit, Event_MShot, "uninit"}
                    ,{Event_MShot_start, Event_MShot, "start"}
                    ,{Event_MShot_onCreateImage, Event_MShot, "onCreateImage"}
                    ,{Event_MShot_stop, Event_MShot, "stop"}
                    ,{Event_MShot_onCreateYuvImage, Event_MShot, "onCreateYuvImage"}
                    ,{Event_MShot_onCreateThumbImage, Event_MShot, "onCreateThumbImage"}
                    ,{Event_MShot_onCreateJpegImage, Event_MShot, "onCreateJpegImage"}
                    ,{Event_MShot_createSensorRawImg, Event_MShot, "createSensorRawImg"}
                    ,{Event_MShot_createYuvRawImg, Event_MShot, "createYuvRawImg"}
                    ,{Event_MShot_createJpegImg, Event_MShot, "createJpegImg"}
                    ,{Event_MShot_createJpegImgSW, Event_MShot, "createJpegImgSW"}
                    ,{Event_MShot_convertImage, Event_MShot, "convertImage"}
                    ,{Event_MShot_YV12ToJpeg, Event_MShot, "YV12ToJpeg"}

                // --Define the event used in single shot
                ,{Event_SShot, Event_Core_CamShot, "SingleShot"}
                    ,{Event_SShot_init, Event_SShot, "init"}
                    ,{Event_SShot_uninit, Event_SShot, "uninit"}
                    ,{Event_SShot_startOneSensor, Event_SShot, "startOneSensor"}
                    ,{Event_SShot_startOneMem, Event_SShot, "startOneMem"}
                    ,{Event_SShot_createSensorRawImg, Event_SShot, "createSensorRawImg"}
                    ,{Event_SShot_createYuvRawImg, Event_SShot, "createYuvRawImg"}
                    ,{Event_SShot_createJpegImg, Event_SShot, "createJpegImg"}

            // Define the event used in Core::drv
            ,{Event_Core_Drv, Event_Core, "Drv"}
                // --Define the event used in sensor driver
                ,{Event_Sensor, Event_Core_Drv, "Sensor"}
                    ,{Event_Sensor_search, Event_Sensor, "searchSensor"}
                    ,{Event_Sensor_open, Event_Sensor, "open"}
                    ,{Event_Sensor_close, Event_Sensor, "close"}
                    ,{Event_Sensor_setScenario, Event_Sensor, "setScenario"}

            // Define the event used in Core::FeatureIO
            ,{Event_Core_FeatureIO, Event_Core, "FeatureIO"}
                // --Define the event used in 3A pipe
                ,{Event_Pipe_3A, Event_Core_FeatureIO, "Pipe3A"}
                    ,{Event_Pipe_3A_AE, Event_Pipe_3A, "AE"}
                    ,{Event_Pipe_3A_Single_AF, Event_Pipe_3A, "SingleAF"}
                    ,{Event_Pipe_3A_Continue_AF, Event_Pipe_3A, "ContinueAF"}
                    ,{Event_Pipe_3A_AWB, Event_Pipe_3A, "AWB"}
                    ,{Event_Pipe_3A_Strobe, Event_Pipe_3A, "Strobe"}
                    ,{Event_Pipe_3A_ISP, Event_Pipe_3A, "ISP"}
                    ,{Event_Pipe_3A_ISP_DRVMGR_INIT, Event_Pipe_3A, "ISPDrvMgrInit"}
                    ,{Event_Pipe_3A_ISP_TDRIMGR_INIT, Event_Pipe_3A, "ISPTdriMgrInit"}
                    ,{Event_Pipe_3A_ISP_LSCMGR_INIT, Event_Pipe_3A, "ISPLscMgrInit"}
                    ,{Event_Pipe_3A_ISP_VALIDATE_FRAMELESS, Event_Pipe_3A, "ISPValidateFrameless"}
                    ,{Event_Pipe_3A_ISP_VALIDATE_PERFRAME, Event_Pipe_3A, "ISPValidatePerframe"}
                    ,{Event_Pipe_3A_ISP_VALIDATE_PERFRAME_DYNAMIC_TUNING, Event_Pipe_3A, "ISPValidatePerframeDynamicTuning"}
                    ,{Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, Event_Pipe_3A, "ISPValidatePerframePrepare"}
                    ,{Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY, Event_Pipe_3A, "ISPValidatePerframeApply"}

};

bool initCameraProfile()
{
    bool ret = false; 
    if(!gbInit)
    {
        ret = CPTRegisterEvents(gCPTEventInfo, sizeof(gCPTEventInfo) / sizeof(gCPTEventInfo[0]));     
        gbInit = ret;
    }
    return ret;
}

