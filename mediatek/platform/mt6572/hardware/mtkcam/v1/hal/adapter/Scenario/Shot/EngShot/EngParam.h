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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ENGPARAM_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ENGPARAM_H_


namespace android {
namespace NSShot {
/******************************************************************************
 *
 ******************************************************************************/

struct EngParam: public ShotParam
{
    MUINT32                         u4SensorWidth;
    MUINT32                         u4SensorHeight;
    MUINT32                         u4Bitdepth;
    MUINT32                         u4RawPixelID;
    EImageFormat                    eImgFmt;
    int32_t                         mi4EngRawSaveEn;
    int32_t                         mi4EngSensorMode;
    int32_t                         mi4EngIspMode;    

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    EngParam(
                                        MUINT32 const                   SensorWidth     = 0,
                                        MUINT32 const                   SensorHeight    = 0,
                                        MUINT32 const                   Bitdepth        = 0,
                                        MUINT32 const                   RawPixelID      = 0,
                                        EImageFormat const              ImgFmt          = eImgFmt_UNKNOWN,
                                        int32_t const                   i4EngRawSaveEn  = 0, 
                                        int32_t const                   i4EngSensorMode = 0,
                                        int32_t const                   i4EngIspMode    = 0
                                    )
                                        : u4SensorWidth(SensorWidth)
                                        , u4SensorHeight(SensorHeight)
                                        , u4Bitdepth(Bitdepth)
                                        , u4RawPixelID(RawPixelID)
                                        , eImgFmt(ImgFmt)
                                        , mi4EngRawSaveEn(i4EngRawSaveEn)
                                        , mi4EngSensorMode(i4EngSensorMode)
                                        , mi4EngIspMode(i4EngIspMode)
                                    {
                                    }

                                    enum EngSensorMode { // Duplicate from sensor_hal.h (mediatek\platform\mt6589\hardware\camera\inc\drv\)
                                        ENG_SENSOR_MODE_PREVIEW,            // ACDK_SCENARIO_ID_CAMERA_PREVIEW=0,
                                        ENG_SENSOR_MODE_CAPTURE,            // ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,
                                        ENG_SENSOR_MODE_VIDEO_PREVIEW,      // ACDK_SCENARIO_ID_VIDEO_PREVIEW,
                                        ENG_SENSOR_MODE_ENUM,
                                    };

                                    enum EngIspMode { // for Engineer Mode App (written in Java)
                                        ENG_ISP_MODE_PROCESSED_RAW = '0',
                                        ENG_ISP_MODE_PURE_RAW = '1',
                                        ENG_ISP_MODE_ENUM,
                                    };

                                    enum EngRawType { // for Cam IO
                                        ENG_RAW_TYPE_PURE_RAW = 0, // 0: pure raw
                                        ENG_RAW_TYPE_PROCESSED_RAW = 1, // 1: pre-process raw
                                        ENG_RAW_TYPE_ENUM,
                                    };
};

} // namespace android
} // namespace NSShot
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ENGPARAM_H_

