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

#ifndef _MTK_CAMERA_COMMON_PARAMSMGR_FEATURE_INC_FEATURE_H_
#define _MTK_CAMERA_COMMON_PARAMSMGR_FEATURE_INC_FEATURE_H_
//
#include "../../inc/FeatureDef.h"
#include "../../inc/IFeature.h"


using namespace android;
using namespace NSCameraFeature;
namespace NSCameraFeature
{
/*******************************************************************************
 *
 ******************************************************************************/
#define DLSYM_MODULE_NAME_COMMON_SENSOR_RAW "SENSOR_DRVNAME_COMMON_RAW"
#define DLSYM_MODULE_NAME_COMMON_SENSOR_YUV "SENSOR_DRVNAME_COMMON_YUV"
#define DLSYM_MODULE_NAME_COMMON_SENSOR_ATV "COMMON_ATVSENSOR"
//
#define DLSYM_MODULE_NAME_FLASHLIGHT        "CUSTOM_FLASHLIGHT"
#define DLSYM_MODULE_NAME_AFLAMP            "CUSTOM_AFLAMP"
#define DLSYM_MODULE_NAME_CAMERASHOT        "CUSTOM_CAMERASHOT"


/*******************************************************************************
 *
 ******************************************************************************/
class Feature : public IFeature
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    int32_t const                   mi4OpenId;
    String8                         ms8SensorName;
    uint32_t                        mu4SensorType;
    FeatureKeyedMap                 mFeatureMap;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Instantiation.
                                    Feature(int32_t const i4OpenId);
    bool                            init();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                    Custom Features.
    bool                            initFeatures_Custom();
    bool                            initFeatures_Custom_dlsym();
    bool                            initFeatures_Custom_v2();   //  version 2 (new common)
    bool                            initFeatures_Custom_v1();   //  version 1 (old yuv; discard)

protected:  ////                    Run-Time Features.
    bool                            initFeatures_RunTime();
    bool                            initFeatures_Flashlight();
    bool                            initFeatures_AFLamp();      //  Auto-Focus Lamp.
    bool                            initFeatures_CameraShot();

protected:  ////                    Correction.
    bool                            initFeatures_NoWarningCorrection();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Utils.
    bool                            setupSensorInfo_FromExModule();
    bool                            queryCustomFeature(FeatureKeyedMap& rFMap, String8 const& rs8ModuleName);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Interfaces.
    virtual void                    destroyInstance();
    virtual FeatureKeyedMap const*  getFeatureKeyedMap() const { return &mFeatureMap; }

};


/*******************************************************************************
 *
 ******************************************************************************/
};  //  namespace NSCameraFeature
#endif // _MTK_CAMERA_COMMON_PARAMSMGR_FEATURE_INC_FEATURE_H_

