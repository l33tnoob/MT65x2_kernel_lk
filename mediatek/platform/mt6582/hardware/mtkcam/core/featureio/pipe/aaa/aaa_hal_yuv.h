/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
/**
 * @file aaa_hal_yuv.h
 * @brief 3A hardware abstraction layer for yuv sensor.
 */
#ifndef _AAA_HAL_YUV_H_
#define _AAA_HAL_YUV_H_

//------------Thread-------------
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
//-------------------------------
#include <mtkcam/drv/isp_drv.h>

#include <mtkcam/hal/aaa_hal_base.h>
#include <utils/threads.h>
#include <mtkcam/hal/sensor_hal.h>
#include "strobe_drv.h"
#include "kd_imgsensor_define.h"

using namespace android;

/**
 * @brief AREA_T structure
 */
typedef struct
{
    MINT32 i4Left;
    MINT32 i4Right;
    MINT32 i4Top;
    MINT32 i4Bottom;
    MINT32 i4Info;
} AREA_T;

#define MAX_FOCUS_AREAS    (9)
#define MAX_METERING_AREAS (9)


namespace NS3A
{


/*******************************************************************************
*
********************************************************************************/
/**  
 * @brief class of 3A hardware abstraction layer for yuv sensor
 */
class Hal3AYuv : public Hal3ABase
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    Hal3AYuv();
    virtual ~Hal3AYuv();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**  
     * @brief Create Hal3AYuv instance.
     */
    static Hal3AYuv* createInstance(MINT32 const i4SensorDevId);

    /**  
     * @brief Get Hal3AYuv instance.
     */
    static Hal3AYuv* getInstance();

    /**  
     * @brief Destroy Hal3AYuv instance.
     */
    virtual MVOID destroyInstance();

    /**  
     * @brief Interface of sending command for preview/capture/recording state control by middleware.
     */
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINT32 const i4Arg = 0);

    /**  
     * @brief Get error code.
     */
    virtual inline MINT32 getErrorCode() const
    {
        return m_errorCode;
    }

    /**  
     * @brief Get current 3A parameters.
     * @param [out] rParam
     */
    virtual inline MBOOL getParams(Param_T &rParam) const
    {
        rParam = m_rParam;
        return MTRUE;
    }

    /**  
     * @brief Set 3A parameters.
     * @param [in] rNewParam
     */
    virtual MBOOL setParams(Param_T const &rNewParam);

    /**  
     * @brief Get 3A supported feature parameters.
     * @details Capability of current yuv sensor
     * @param [out] rFeatureParam
     */
    virtual MBOOL getSupportedParams(FeatureParam_T &rFeatureParam);

    /**  
     * @brief Get flag to check if it is ready to capture.
     *
     * @return
     * - MTRUE indicates it is ready to capture.
     * - MFALSE indicates it is not ready to capture.
     */
    virtual inline MBOOL isReadyToCapture() const
    {
        return m_bReadyToCapture;
    }

    /**  
     * @brief Trigger yuv sensor to do single AF.
     * @note Set AF mode to AF_MODE_AFS before doing single AF.
     * @return
     * - MTRUE indicates command is sent successfully.
     */
    virtual MBOOL autoFocus();

    /**  
     * @brief Cancel auto focus, and AF status will become idle.
     * @return
     * - MTRUE indicates command is sent successfully.
     */
    virtual MBOOL cancelAutoFocus();
    
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);

    /**  
     * @brief Set 3A EXIF info for creating exif header.
     * @param [in,out] pIBaseCamExif
     * @return
     * - MTRUE indicates finish.
     */
    virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const;

    /**  
     * @brief Set debug info.
     */
    virtual MBOOL setDebugInfo(IBaseCamExif *pIBaseCamExif) const;

    /**  
     * @brief Get delay frame information.
     * @details Query the delay frame count for the purpose of controlling some functions taking effect after delayed frames.
     * @param [in] eQueryType
     * @return
     * - delay frame count of specified eQueryType.
     */
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const;

    /**  
     * @brief Set ISP profile.
     */
    virtual MBOOL setIspProfile(EIspProfile_T const eIspProfile);

    /**  
     * @brief Enable thread for AF.
     * @param [in] a_bEnable 1 for creating thread, 0 for destroying thread
     */
    virtual MRESULT EnableAFThread(MINT32 a_bEnable);

    /**  
     * @brief Interface for FD client setting face detecion result information.
     * @param [in] a_sFaces Face detection result for updating AF/AE window; set NULL for indicating FD off.
     */
    virtual MBOOL setFDInfo(MVOID* a_sFaces);

    /**  
     * @brief Set callback pointer to Hal3AYuv.
     * @details When AF is doing or done, sensor reports AF status. This pointer receives the reporting way.
     * @param [in] cb Callback function pointer.
     */
    virtual MBOOL setCallbacks(I3ACallBack* cb);

    /**  
     * @brief Get capture parameters.
     */
    virtual MINT32 getCaptureParams(MINT8 index, MINT32 i4EVidx, CaptureParam_T &a_rCaptureInfo);

    /**  
     * @brief Update capture parameters.
     */
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo);

    /**  
     * @brief Finish jobs at the end of continuous shot.
     */
    virtual MVOID endContinuousShotJobs();

    /**  
     * @brief Enable AE limiter control.
     */
    virtual MINT32 enableAELimiterControl(MBOOL  bIsAELimiter);

    /**  
     * @brief Check if it is necessary to firiing flash.
     * @return
     * - MTRUE indicates flash on.
     * - MFALSE indicates flash off.
     */
    virtual MINT32 isNeedFiringFlash();

    virtual MINT32 getFlashFrameNumBeforeCapFrame();
    virtual MVOID onFireCapFlashIfNeeded();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**  
     * @brief Initialize Hal3AYuv.
     * @param [in] i4SensorDev 
     */
    MRESULT init(MINT32 i4SensorDev);

    /**  
     * @brief Uninitialize Hal3AYuv.
     */
    MRESULT uninit();

    /**  
     * @brief Set error code.
     */
    inline MVOID setErrorCode(MRESULT errorCode)
    {
        m_errorCode = errorCode;
    }

    /**  
     * @brief Reset ready-to-capture flag to 0.
     */
    inline MVOID resetReadyToCapture()
    {
        m_bReadyToCapture = MFALSE;
    }

    /**  
     * @brief Set ready-to-capture flag to 1.
     */
    inline MVOID notifyReadyToCapture()
    {
        m_bReadyToCapture = MTRUE;
    }

    /**  
     * @brief Get current sensor device ID.
     */
    inline MINT32 getSensorDev()
    {
        return m_i4SensorDev;
    }

private:
    /**  
     * @brief Thread function for handling AF.
     */
    static MVOID* AFThreadFunc(void *arg);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    volatile int  m_Users;
    mutable Mutex m_Lock;
    mutable Mutex m_LockAF;
    MRESULT       m_errorCode;
    Param_T       m_rParam;
    MBOOL         m_bForceUpdatParam;
    MBOOL         m_bReadyToCapture;
    MINT32        m_i4SensorDev;
    I3ACallBack*  m_pAFYuvCallBack;
    SensorHal*    m_pSensorHal;

    //for ASD
    MINT32        m_i4AELv_x10;
    MBOOL         m_bAEStable;
    SENSOR_AE_AWB_REF_STRUCT m_AsdRef;
    
    //AF related
    MINT32        m_bIsdummylens;
    MINT32        m_AFzone[6];	
    MINT32        m_AEzone[6];
    AREA_T        m_sAFAREA[MAX_FOCUS_AREAS];      //
    AREA_T        m_sAEAREA[MAX_METERING_AREAS];   //
    MINT32        m_imageXS;
    MINT32        m_imageYS;
    MINT32        m_max_metering_areas;
    MINT32        m_max_af_areas;
    MINT32        m_i4PreAfStatus;
    MINT32        m_i4PreAfMode;
    MINT32        m_i4AFSwitchCtrl;
    MINT32        m_i4AutoFocus;
    MINT32        m_i4AutoFocusTimeout;
    MINT32        m_i4FDFrmCnt;
    MINT32        m_i4FDApplyCnt;
    MINT32        m_i4WinState;
    MINT32        m_i4State;
    MBOOL         m_fgAfTrig;

    MBOOL         bAELockSupp;
    MBOOL         bAWBLockSupp;
    MBOOL         m_bAeLimiter;
    
    //Strobe related
    StrobeDrv     *m_pStrobeDrvObj;
    double        m_strobeTrigerBV;	
    double        BV_THRESHOLD;	
    double        m_strobecurrent_BV;
    double        m_strobePreflashBV;
    MINT32        m_aeFlashlightType;
    SENSOR_FLASHLIGHT_AE_INFO_STRUCT m_AEFlashlightInfo;
    MINT32        pre_shutter;
    MINT32        pre_gain;
    MUINT32       m_preflashFrmCnt;
    MUINT32       m_strobeWidth;
    MBOOL         m_bFlashActive;

    MBOOL         m_bExifFlashOn;
    MBOOL         m_isFlashOnCapture;
    MBOOL         m_TrigFlashFire;

    // AF thread
    IspDrv*       m_pIspDrv;
    MINT32        m_bAFThreadLoop;
    pthread_t     m_AFThread;
    sem_t         m_semAFThreadStart;

private:
    /**  
     * @brief Set auto exposure lock.
     * @param [in] bLock 1 for lock, 0 for unlock
     */
    MBOOL setAeLock(MBOOL bLock);

    /**  
     * @brief Set auto white balance lock.
     * @param [in] bLock 1 for lock, 0 for unlock
     */
    MBOOL setAwbLock(MBOOL bLock);

    /**  
     * @brief Map EV value to enum type AE_EVCOMP_T.
     * @param [in] mval, mstep
     * @return
     * - AE_EVCOMP_T refer to kd_camera_feature_enum.h in detail.
     */
    MINT32 mapAEToEnum(MINT32 mval,MFLOAT mstep);

    /**  
     * @brief Map ISO value to enum type AE_ISO_T.
     * @param [in] u4NewAEISOSpeed
     * @return
     * - AE_ISO_T refer to kd_camera_feature_enum.h in detail.
     */
    MINT32 mapISOToEnum(MUINT32 u4NewAEISOSpeed);

    /**  
     * @brief Map enum type AE_ISO_T to ISO value.
     * @param [in] u4NewAEIsoEnum.
     * @return
     * - ISO value
     */
    MINT32 mapEnumToISO(MUINT32 u4NewAEIsoEnum) const;
    
    //AF related

    /**  
     * @brief Do AF update for reading AF status and callback for drawing AF window in AFThreadFunc.
     */
    MINT32 doAFUpdate(void);

    /**  
     * @brief Set AF mode.
     * @param [in] AFMode refer to AF_MODE_T in kd_camera_feature_enum.h.
     */
    MINT32 setAFMode(MINT32 AFMode);

    /**  
     * @brief Get AF status.
     * @return
     * - SENSOR_AF_FOCUSED indicates focused and lens stops moving.
     * - SENSOR_AF_FOCUSING indicates lens is moving for focusing.
     * - SENSOR_AF_IDLE indicates lens idle and no action.
     * - SENSOR_AF_ERROR indicates error happened.
     */
    MINT32 isFocused();

    /**  
     * @brief Set focus area.
     * @param [in] a_i4Cnt number of area; a_psFocusArea array of areas.
     */
    MVOID  setFocusAreas(MINT32 a_i4Cnt, AREA_T *a_psFocusArea);

    /**  
     * @brief Get focus area.
     * @param [out] a_i4Cnt number of area; a_psFocusArea array of areas.
     */
    MVOID  getFocusAreas(MINT32 &a_i4Cnt, AREA_T **a_psFocusArea);

    /**  
     * @brief Utility for clamp x at range from min to max.
     * @param [in] x, min, max
     * @return
     * - clamped value
     */
    MINT32 clamp(MINT32 x, MINT32 min, MINT32 max);

    /**  
     * @brief Get AE metering area.
     * @param [out] a_i4Cnt number of area; a_psAEArea array of areas.
     */
    MVOID  getMeteringAreas(MINT32 &a_i4Cnt, AREA_T **a_psAEArea);

    /**  
     * @brief Set AE metering area.
     * @param [in] a_i4Cnt number of area; a_psAEArea array of areas.
     */
    MVOID  setMeteringAreas(MINT32 a_i4Cnt, AREA_T const *a_psAEArea);

    /**  
     * @brief Map area to zone.
     * @param [in] p_area pointer to area.
     * @param [in] areaW, areaH dimension of area coordinate.
     * @param [in] zoneW, zoneH dimension of zone coordinate.
     * @param [out] p_zone pointer to zone.
     */
    MVOID  mapAeraToZone(AREA_T *p_area, MINT32 areaW,
                         MINT32 areaH, MINT32* p_zone, 
                         MINT32 zoneW, MINT32 zoneH);

    /**  
     * @brief Reset AF and AE window to center.
     */
    MBOOL  resetAFAEWindow();

    /**  
     * @brief Set flashlight mode.
     * @param [in] mode refer to AE_STROBE_T in kd_camera_feature_enum.h.
     */
    MINT32 setFlashMode(MINT32 mode);

    /**  
     * @brief Check if flashlight can be on.
     * @return
     * - MTRUE indicates ON.
     * - MFALSE indicates OFF.
     */
    MINT32 isAEFlashOn();

    /**  
     * @brief Set lamp on or off in AF process
     * @param [in] bOnOff
     * @return
     * - S_3A_OK indicates OK.
     * - E_3A_NULL_OBJECT indicates no strobe.
     */
    MINT32 setAFLampOnOff(MBOOL bOnOff);

    /**  
     * @brief Log2 operation
     * @param [in] x
     * @return
     * - Log2(x)
     */
    double AEFlashlightLog2(double x);

    /**  
     * @brief Calculate brightness value.
     * @return
     * - BV
     */
    double calcBV();

    /**  
     * @brief Query exposure info from sensor.
     */
    MINT32 queryAEFlashlightInfoFromSensor();

    /**  
     * @brief Update capture parameters for flashlight on.
     */
    MINT32 updateAeFlashCaptureParams();

    /**  
     * @brief Set exposure time and sensor gain.
     * @param [in] a_u4ExpTime exposure time in microsecond
     * @param [in] a_u4SensorGain sensor gain in GAIN_BASE
     */
    MINT32 setEShutterParam(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain);

    /**  
     * @brief Get HDR capture info.
     * @param [out] a_strHDROutputInfo
     */
    MINT32 getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo);

    /**  
     * @brief Convert flashlight exposure parameters
     */
    void convertFlashExpPara(MINT32 flashEngRatio_x10, MINT32 minAfeGain_x1024, 
								 MINT32 bv0_x1024, MINT32 bv1_x1024,
								 MINT32  exp1, MINT32  afeGain1_x1024, MINT32  ispGain1_x1024, 
								 MINT32& exp2, MINT32& afeGain2_x1024, MINT32& ispGain2_x1024) const;

    MINT32 getASDInfo(ASDInfo_T &a_ASDInfo);

    void calcASDEv(const SENSOR_AE_AWB_CUR_STRUCT& cur);        
   
    MINT32 ASDLog2Func(MUINT32 numerator, MUINT32 denominator) const;

    MBOOL isInVideo();

};

}; // namespace NS3A

#endif
