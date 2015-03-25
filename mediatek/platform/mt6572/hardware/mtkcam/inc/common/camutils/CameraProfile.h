/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MHAL_INC_CAMERA_PROFILE_H_
#define _MHAL_INC_CAMERA_PROFILE_H_

#include <mtkcam/mmp/mmp.h>
using namespace CPTool;

/**
  * @brief Camera profile event of the camera platform 
  */
typedef enum
{
    // Define the root event of camera
    Event_Camera_Platform_Root = EVENT_CAMERA_PLATFORM,

        // Define the event used in Hal
        Event_Hal,

            // Define the event used in Hal::Device
            Event_Hal_Device,
                // Define the event used in Hal::Device::DefaultCamDevice
                Event_Hal_DefaultCamDevice,
                    Event_Hal_DefaultCamDevice_init,
                    Event_Hal_DefaultCamDevice_uninit,

            // Define the event used in Hal::Adapter
            Event_Hal_Adapter,
                // Define the event used in Hal::Adapter::Scenario
                Event_Hal_Adapter_Scenario,
                    // Define the event used in Hal::Adapter::Scenario::Shot
                    Event_Hal_Adapter_Scenario_Shot,
                        // --Define the event used in continuous shot
                        Event_CShot,
                            Event_CShot_capture,
                            Event_CShot_sendCmd,
                            Event_CShot_cancel,
                            Event_CShot_handleNotifyCb,
                            Event_CShot_handlePVData,
                            Event_CShot_handleJpegData,
                        // -- Define the event used in normal shot
                        Event_Shot,
                            Event_Shot_capture,
                            Event_Shot_sendCmd,
                            Event_Shot_cancel,
                            Event_Shot_handleNotifyCb,
                            Event_Shot_handlePVData,
                            Event_Shot_handleJpegData,
                        // --Define the event used in shot
                        Event_FcaeBeautyShot,
                            Event_FBShot_createFullFrame,
                            Event_FBShot_STEP1,
                            Event_FBShot_STEP1Algo,
                            Event_FBShot_STEP2,
                            Event_FBShot_STEP3,
                            Event_FBShot_STEP4,
                            Event_FBShot_STEP4Algo,
                            Event_FBShot_STEP5,
                            Event_FBShot_STEP5Algo,
                            Event_FBShot_STEP6,
                            Event_FBShot_createFBJpegImg,
                            Event_FBShot_Utility,
                            Event_FBShot_requestBufs,
                            Event_FBShot_InitialAlgorithm,
                            Event_FBShot_JpegEncodeImg,
                            Event_FBShot_ResizeImg,
                        // --Define the event used in HdrShot
                        Event_HdrShot,
                            Event_HdrShot_EVCapture,
                            Event_HdrShot_SingleCapture,
                            Event_HdrShot_ImageRegistration,
                            Event_HdrShot_SE,
                            Event_HdrShot_MAV,
                            Event_HdrShot_WeightingMapGeneration,
                            Event_HdrShot_DownSize,
                            Event_HdrShot_UpSize,
                            Event_HdrShot_Blending,
                            Event_HdrShot_Fusion,
                            Event_HdrShot_SaveNormal,
                            Event_HdrShot_SaveHdr,
                        // -- Define the event used in zsd shot
                        Event_ZsdShot,
                            Event_ZsdShot_capture,
                            Event_ZsdShot_handleJpegData,

                // Define the event used in Hal::Adapter::Preview
                Event_Hal_Adapter_MtkPhotoPreview,
                    Event_Hal_Adapter_MtkPhotoPreview_start,
                    Event_Hal_Adapter_MtkPhotoPreview_start_init,
                    Event_Hal_Adapter_MtkPhotoPreview_start_stable,
                    Event_Hal_Adapter_MtkPhotoPreview_proc,
                    Event_Hal_Adapter_MtkPhotoPreview_precap,
                    Event_Hal_Adapter_MtkPhotoPreview_stop,

                // Define the event used in Hal::Adapter::Default
                Event_Hal_Adapter_MtkDefaultPreview,
                    Event_Hal_Adapter_MtkDefaultPreview_start,
                    Event_Hal_Adapter_MtkDefaultPreview_start_init,
                    Event_Hal_Adapter_MtkDefaultPreview_start_stable,
                    Event_Hal_Adapter_MtkDefaultPreview_proc,
                    Event_Hal_Adapter_MtkDefaultPreview_precap,
                    Event_Hal_Adapter_MtkDefaultPreview_stop,
                    Event_Hal_Adapter_MtkDefaultPreview_vss,

                // Define the event used in Hal::Adapter::ZSD
                Event_Hal_Adapter_MtkZsdPreview,
                    Event_Hal_Adapter_MtkZsdPreview_start,
                    Event_Hal_Adapter_MtkZsdPreview_start_init,
                    Event_Hal_Adapter_MtkZsdPreview_start_stable,
                    Event_Hal_Adapter_MtkZsdPreview_proc,
                    Event_Hal_Adapter_MtkZsdPreview_precap,
                    Event_Hal_Adapter_MtkZsdPreview_stop,
                    Event_Hal_Adapter_MtkZsdCapture,

            // Define the event used in Hal::Client
            Event_Hal_Client,
                // Define the event used in Hal::Client::CamClient
                Event_Hal_Client_CamClient,
                    // Define the event used in Hal::Client::CamClient::FD
                    Event_Hal_Client_CamClient_FD,

        // Define the event used in Core
        Event_Core,

            // Define the event used in Core::CamShot
            Event_Core_CamShot,
                // --Define the event used in multi shot
                Event_MShot,
                    Event_MShot_init,
                    Event_MShot_uninit,
                    Event_MShot_start,
                    Event_MShot_onCreateImage,
                    Event_MShot_stop,
                    Event_MShot_onCreateYuvImage,
                    Event_MShot_onCreateThumbImage,
                    Event_MShot_onCreateJpegImage,
                    Event_MShot_createSensorRawImg,
                    Event_MShot_createYuvRawImg,
                    Event_MShot_createJpegImg,
                    Event_MShot_createJpegImgSW,
                    Event_MShot_convertImage,
                    Event_MShot_YV12ToJpeg,
                // --Define the event used in single shot
                Event_SShot,
                    Event_SShot_init,
                    Event_SShot_uninit,
                    Event_SShot_startOneSensor,
                    Event_SShot_startOneMem,
                    Event_SShot_createSensorRawImg,
                    Event_SShot_createYuvRawImg,
                    Event_SShot_createJpegImg,

            // Define the event used in Core::drv
            Event_Core_Drv,

                // --Define the event used in sensor driver
                Event_Sensor,
                    Event_Sensor_search,
                    Event_Sensor_open,
                    Event_Sensor_close,
                    Event_Sensor_setScenario,

            // Define the event used in Core::FeatureIO
            Event_Core_FeatureIO,

                // --Define the event used in aaa pipe layer
                Event_Pipe_3A,
                    Event_Pipe_3A_AE,
                    Event_Pipe_3A_Single_AF,
                    Event_Pipe_3A_Continue_AF,
                    Event_Pipe_3A_AWB,
                    Event_Pipe_3A_Strobe,
                    Event_Pipe_3A_ISP,
                    Event_Pipe_3A_ISP_DRVMGR_INIT,
                    Event_Pipe_3A_ISP_TDRIMGR_INIT,
                    Event_Pipe_3A_ISP_LSCMGR_INIT,
                    Event_Pipe_3A_ISP_VALIDATE_FRAMELESS,
                    Event_Pipe_3A_ISP_VALIDATE_PERFRAME,
                    Event_Pipe_3A_ISP_VALIDATE_PERFRAME_DYNAMIC_TUNING,
                    Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE,
                    Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY,                        
    Event_Max_Num
}CPT_Event;

bool initCameraProfile();

////////////////////////////////////////////////////////////////////////////////
#endif  //  _MHAL_INC_CAMERA_PROFILE_H_
