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

#ifndef _MTK_CAMERA_COMMON_PARAMSMGR_INC_PARAMSMANAGER_H_
#define _MTK_CAMERA_COMMON_PARAMSMGR_INC_PARAMSMANAGER_H_
//
#if (PLATFORM_VERSION_MAJOR == 2)
#include <utils/threads.h>
#else
#include <utils/RWLock.h>
#endif
//
#include <camera/CameraParameters.h>
#include <camera/MtkCameraParameters.h>
//
//#if '1'==MTKCAM_HAVE_CAMFEATURE
    #include "inc/FeatureDef.h"
    #include "inc/IFeature.h"
//#endif


namespace android {
/*******************************************************************************
 *
 ******************************************************************************/


/*******************************************************************************
 *
 ******************************************************************************/
class ParamsManager : public IParamsManager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Instantiation.
    virtual bool                        init();
    virtual bool                        uninit();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                        Attributes.
    virtual char const*                 getName()   const   { return mName.string(); }
    virtual int32_t                     getOpenId() const   { return mi4OpenId; }

    virtual int32_t                     getHalAppMode() const;
    virtual bool                        updateHalAppMode();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                        Operations.
    virtual status_t                    setParameters(CameraParameters const& params);
    virtual String8                     getPreviewFormat() const;
    virtual void                        getPreviewSize(int *width, int *height) const;
    virtual void                        getVideoSize(int *width, int *height) const;
    virtual void                        getPictureSize(int *width, int *height) const;
    virtual uint32_t                    getZoomRatio() const;
    virtual uint32_t                    getZoomRatioByIndex(uint32_t index) const;
    virtual bool                        getRecordingHint() const;
    virtual bool                        getVideoStabilization() const;
    virtual bool                        getCShotIndicator() const;
    virtual String8                     getShotModeStr() const;
    virtual uint32_t                    getShotMode() const;

    /*
     * @brief update brightness value.
     *
     * @details update (BV) brightness value.
     *
     * @note
     *
     * @param[in] iBV: brightness value (scaled by 10).
     *
     * @return
     *
     */
    virtual void                        updateBrightnessValue(int const iBV);
    virtual void                        updatePreviewFPS(int const fps);
    virtual void                        updatePreviewFrameInterval(int const interval);
    virtual void                        updatePreviewAEIndexShutterGain(int const index, int const shutter, int const isp_gain, int const sensor_gain);
    virtual void                        updateCaptureShutterGain(int const shutter, int const isp_gain, int const sensor_gain);
    virtual void                        updateEngMsg(char const* msg);

public:     ////                        Utility.

    static  String8                     getValuesKeyName(char const aKeyName[]);
    static  String8                     getValuesKeyName(String8 const& s8KeyName);
    static  String8                     getSupportedKeyName(String8 const& s8KeyName);
                                        //
    static  bool                        splitInt(String8 const& s8Input, Vector<int>& rOutput);
    static  bool                        splitRange(String8 const& s8Input, List< Vector<int> >& rOutput);
                                        //
    virtual void                        showParameters() const;
                                        //
    virtual String8                     flatten() const;
    virtual void                        set(char const* key, char const* value);
    virtual void                        set(char const* key, int value);
    virtual String8                     getStr(char const* key) const;
    virtual int                         getInt(char const* key) const;
    virtual float                       getFloat(char const* key) const;
    virtual bool                        isEnabled(char const* key) const;
                                        //
    virtual status_t                    parseCamAreas(
                                            char const* areaStr, 
                                            List<camera_area_t>& areas, 
                                            int const maxNumArea
                                        ) const;
                                        //
    virtual status_t                    dump(int fd, Vector<String8>const& args);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                        Instantiation.
    virtual                             ~ParamsManager();
                                        ParamsManager(
                                            String8 const& rName, 
                                            int32_t const i4OpenId
                                        );

protected:  ////                        Check.
    virtual status_t                    checkParams(CameraParameters const& params) const;
    virtual status_t                    checkZoomValue(CameraParameters const& params) const;
    virtual status_t                    checkPreviewFpsRange(CameraParameters const& params) const;
    virtual status_t                    checkFlashMode(CameraParameters const& params) const;
    virtual status_t                    checkFocusMode(CameraParameters const& params) const;
    virtual status_t                    checkFocusAreas(CameraParameters const& params) const;
    virtual status_t                    checkMeteringAreas(CameraParameters const& params) const;
    virtual status_t                    checkPreviewSize(CameraParameters const& params) const;
    virtual status_t                    isParameterValid(char const* param, char const* supportedParams) const;
    virtual status_t                    isAreaValid(char const* param, int const maxNumArea) const;
    virtual bool                        checkCamArea(camera_area_t const& camArea) const;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                        Update.
    virtual bool                        updateDefaultParams();

protected:  ////                        Called by updateDefaultParams().
    virtual bool                        updateDefaultParams0();
    virtual bool                        updateDefaultParams1();
    virtual bool                        updateDefaultParams2();

protected:  ////                        Called by updateDefaultParams1().
    virtual bool                        updateDefaultParams1_ByQuery()      = 0;
    virtual bool                        updateDefaultParams1_ByDefault();
    virtual bool                        updatePreferredPreviewSize();
    virtual bool                        updateDefaultFaceCapacity()         = 0;

protected:  ////                        Called by updateDefaultParams2().
    virtual bool                        updateDefaultParams2_ByQuery()      = 0;
    virtual bool                        updateDefaultParams2_ByDefault();

protected:  ////                        Called by updateDefaultParams3().
    virtual bool                        updateDefaultPreviewFormat();

protected:  ////                        Called by setParameters().
    virtual bool                        updateSceneAndParams();
    virtual bool                        updateScene();
    virtual bool                        updateSceneDependentParams();
    virtual bool                        updatePreviewSize();
    virtual bool                        updateDefaultVideoFormat()          = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                        Update Params based on FeatureInfo
    bool                                updateParams(int32_t const i4FKeyIndex);
    bool                                updateUserTypeParams(String8 const& s8KeyName, NSCameraFeature::FeatureInfo const& rFeatureInfo);
    bool                                updateZoomParams(NSCameraFeature::FeatureInfo const& rFeatureInfo);
    bool                                updatePreviewFpsParams(NSCameraFeature::FeatureInfo const& rFeatureInfo);
    bool                                updateExposureCompensationParams(NSCameraFeature::FeatureInfo const& rFeatureInfo);
    bool                                updateVideoSnapshotParams(NSCameraFeature::FeatureInfo const& rFeatureInfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    mutable RWLock                      mRWLock;

//#if '1'==MTKCAM_HAVE_CAMFEATURE
    NSCameraFeature::FeatureKeyedMap const* mpFeatureKeyedMap;
//#endif

    mutable MtkCameraParameters         mParameters;
    String8 const                       mName;
    int32_t const                       mi4OpenId;
    //
    String8                             ms8ClientAppMode;   //  Client App Mode.
    int32_t                             mi4HalAppMode;      //  Hal App Mode.
    //
    Vector<uint32_t>                    mvZoomRatios;

};


/*******************************************************************************
 *
 ******************************************************************************/
}; // namespace android
#endif  //_MTK_CAMERA_COMMON_PARAMSMGR_INC_PARAMSMANAGER_H_

