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

#ifndef _MTK_CAMERA_INC_COMMON_IPARAMSMANAGER_H_
#define _MTK_CAMERA_INC_COMMON_IPARAMSMANAGER_H_
//
#include <camera/CameraParameters.h>
#include <camera/MtkCameraParameters.h>
//
#include <utils/RefBase.h>
#include <utils/List.h>


namespace android {
/*******************************************************************************
 *
 ******************************************************************************/


/*******************************************************************************
 *
 ******************************************************************************/
typedef struct camera_area
{
    /**     
    * Bounds of the area [left, top, right, bottom]. (-1000, -1000) represents
    * the top-left of the camera field of view, and (1000, 1000) represents the
    * bottom-right of the field of view. The width and height cannot be 0 or
    * negative.      
    */
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
    /**
    * The weight must range from 1 to 1000, and represents a weight for
    * every pixel in the area. This means that a large metering area with
    * the same weight as a smaller area will have more effect in the
    * metering result. Metering areas can partially overlap and the driver
    * will add the weights in the overlap region
    */
    int32_t weight;
} camera_area_t;


/*******************************************************************************
 *
 ******************************************************************************/
class IParamsManager : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Map Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    IMap Enum
                                    enum
                                    {
                                        eMapAppMode,            //  App Mode.
                                        eMapLevel,              //  Level: low/middle/high
                                        eMapScene, 
                                        eMapEffect, 
                                        eMapWhiteBalance, 
                                        eMapFocusMode, 
                                        eMapFocusLamp, 
                                        eMapExpMode, 
                                        eMapIso, 
                                        eMapAntiBanding, 
                                        eMapFlashMode, 
                                        eMapShotMode, 
                                        eMapMeterMode,
                                    };

public:     ////                    IMap Interface
                                    struct IMap
                                    {
                                        typedef uint32_t    VAL_T;
                                        typedef String8     STR_T;
                                        //
                                        virtual             ~IMap() {}
                                        virtual VAL_T       valueFor(STR_T const& str) const    = 0;
                                        virtual STR_T       stringFor(VAL_T const& val) const   = 0;
                                    };

public:     ////
    /*
     *  IParamsManager::getMapInst(IParamsManager::int2type<IParamsManager::eMapXXX>()) returns a pointer to IMap const (i.e. IMap const*)
     */
    template<int value>             struct int2type { enum {v=value}; };
    template <int eMapXXX>
    static IMap const*              getMapInst(int2type<eMapXXX>);
#define PARAMSMANAGER_MAP_INST(eMapXXX) IParamsManager::getMapInst(IParamsManager::int2type<IParamsManager::eMapXXX>())

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static IParamsManager*          createInstance(
                                        String8 const& rName, 
                                        int32_t const i4OpenId
                                    );
    virtual                         ~IParamsManager() {}
    virtual bool                    init()                                          = 0;
    virtual bool                    uninit()                                        = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Attributes.
    virtual char const*             getName()   const                               = 0;
    virtual int32_t                 getOpenId() const                               = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Operations.
    virtual status_t                setParameters(CameraParameters const& params)   = 0;
    virtual String8                 getPreviewFormat() const                        = 0;
    virtual void                    getPreviewSize(int *width, int *height) const   = 0;
    virtual void                    getVideoSize(int *width, int *height) const     = 0;
    virtual void                    getPictureSize(int *width, int *height) const   = 0;
    virtual uint32_t                getZoomRatio() const                            = 0;
    virtual uint32_t                getZoomRatioByIndex(uint32_t index) const       = 0;
    virtual bool                    getRecordingHint() const                        = 0;
    virtual bool                    getVideoStabilization() const                   = 0;
    virtual String8                 getShotModeStr() const                          = 0;
    virtual uint32_t                getShotMode() const                             = 0;

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
    virtual void                    updateBrightnessValue(int const iBV)            = 0;
    virtual void                    updatePreviewFPS(int const fps)            = 0;
    virtual void                    updatePreviewFrameInterval(int const interval)            = 0;
    virtual void                    updatePreviewAEIndexShutterGain(int const index, int const shutter, int const isp_gain, int const sensor_gain) = 0;
    virtual void                    updateCaptureShutterGain(int const shutter, int const isp_gain, int const sensor_gain) = 0;
    virtual void                    updateEngMsg(char const* msg) = 0;

    virtual int32_t                 getHalAppMode() const                           = 0;
    virtual bool                    updateHalAppMode()                              = 0;
    virtual bool                    updateBestFocusStep() const                     = 0;

public:     ////                    Utility.
    static void                     showParameters(String8 const& rs8Param);
    virtual void                    showParameters() const                          = 0;
                                    //
    virtual String8                 flatten() const                                 = 0;
    virtual void                    set(char const* key, char const* value)         = 0;
    virtual void                    set(char const* key, int value)                 = 0;
    virtual String8                 getStr(char const* key) const                   = 0;
    virtual int                     getInt(char const* key) const                   = 0;
    virtual float                   getFloat(char const* key) const                 = 0;
    virtual bool                    isEnabled(char const* key) const                = 0;
                                    //
    virtual status_t                parseCamAreas(
                                        char const* areaStr, 
                                        List<camera_area_t>& areas, 
                                        int const maxNumArea
                                    ) const                                         = 0;
                                    //
    virtual status_t                dump(int fd, Vector<String8>const& args)        = 0;

};


/*******************************************************************************
 *
 ******************************************************************************/
}; // namespace android
#endif  //_MTK_CAMERA_INC_COMMON_IPARAMSMANAGER_H_

