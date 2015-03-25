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
 * @file ae_mgr.h
 * @brief AE manager
 */

#ifndef _AE_MGR_H_
#define _AE_MGR_H_

#include <ae_feature.h>

/**  
 * @brief AE state motion define
 */
typedef enum
{
    AE_INIT_STATE = -1,                       /*!< AE init state */
    AE_AUTO_FRAMERATE_STATE = 0,  /*!< AE dynamic frame rate state */
    AE_MANUAL_FRAMERATE_STATE,    /*!< AE fix frame rate state */
    AE_AF_STATE,                                  /*!< AE auto focus state */
    AE_PRE_CAPTURE_STATE,                /*!< AE pre-capture state */
    AE_CAPTURE_STATE,                        /*!< AE start capture state */
    AE_POST_CAPTURE_STATE,             /*!< AE post capture state */
    AE_REINIT_STATE,                           /*!< AE re-init state */
    AE_AF_RESTORE_STATE                   /*!< AE AF restore state */
} AE_STATE_T;


/**  
 * @brief AE exposure mode define
 */
typedef enum {
    eAE_EXPO_TIME = 0,     /*!< AE exposure by time */
    eAE_EXPO_LINE             /*!< AE exposure by line */
}eAE_EXPO_BASE;

#define DYNAMIC_FRAME_COUNT (10)

namespace NS3A
{

class IAeAlgo;

/*******************************************************************************
*
*******************************************************************************/
/**  
 * @brief AE manager 
 */

class AeMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AeMgr(AeMgr const&);
    //  Copy-assignment operator is disallowed.
    AeMgr& operator=(AeMgr const&);

public:  ////
    AeMgr();
    ~AeMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**  
     * @brief AE get instance
     */
    static AeMgr& getInstance();
    /**  
     * @brief camera preview init
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam camera parameter; please refer to aaa_hal_base.h
     */       
    MRESULT cameraPreviewInit(MINT32 i4SensorDev, Param_T &rParam);
    /**  
     * @brief camera video init
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam camera parameter; please refer to aaa_hal_base.h
     */       
    MRESULT camcorderPreviewInit(MINT32 i4SensorDev, Param_T &rParam);
    /**  
     * @brief camera preview re-initial
     */       
    MRESULT cameraPreviewReinit();
    /**  
     * @brief uninit
     */    
    MRESULT uninit();
    /**  
     * @brief check if AE lock is supported or not
     * @return always return TRUE
     */
    inline MBOOL isAELockSupported()
    {
        return MTRUE;
    }
    /**  
     * @brief backup AE meter information before capture and resotre before return to preview
     * @param [in] en "1" is backup and "0" is restore
     */
    void setAeMeterAreaEn(int en);

    /**  
     * @brief send the exposure time to sensor driver
     * @param [in] exp exposure time value (micro-second)
     */
    void setExp(int exp);
    /**  
     * @brief send the sensor gain to sensor driver
     * @param [in] afe sensor gain value (1x = 1024)
     */
    void setAfe(int afe);
    /**  
     * @brief send the isp gain to ISP register
     * @param [in] isp gain value (1x = 1024)
     */
    void setIsp(int isp);
    /**  
     * @brief Restore the sensor frame rate
     * @param [in] frm delay frame rate
     */
    void setRestore(int frm);
    /**  
     * @brief set AE meter area window and weight information
     * @param [in] sNewAEMeteringArea meter area; please refer to aaa_hal_base.h
     */
    MRESULT setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea);
    /**  
     * @brief set AE EV compensation index and step
     * @param [in] i4NewEVIndex EV index value;
     * @param [in] fStep Step value; The EV compensation value is equal i4NewEVIndex*fStep
     */
    MRESULT setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep);
    /**  
     * @brief set AE metering mode for AE control
     * @param [in] u4NewAEMeteringMode metering mode value; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setAEMeteringMode(MUINT32 u4NewAEMeteringMode);
    /**  
     * @brief get AE metering mode
     * @return the AE metering mode; please refer to Kd_camera_feature_enum.h
     */
    MINT32 getAEMeterMode() const;
    /**  
     * @brief set AE ISO speed
     * @param [in] i4NewAEISOSpeed ISO value; "0" means "Auto"
     */    
    MRESULT setAEISOSpeed(MUINT32 i4NewAEISOSpeed);
    /**  
     * @brief get AE ISO speed
     * @return the AE ISO value
     */    
    MINT32 getAEISOSpeedMode() const;
    /**  
     * @brief set AE min / max frame rate value
     * @param [in] i4NewAEMinFps minimun frame rate value
     * @param [in] i4NewAEMaxFps maximun frame rate value
     */
    MRESULT setAEMinMaxFrameRate(MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps);
    /**  
     * @brief set Anti-banding mode
     * @param [in] u4NewAEFLKMode flicker mode; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setAEFlickerMode(MUINT32 u4NewAEFLKMode);
    /**  
     * @brief set flicker detection result mode
     * @param [in] u4NewAEAutoFLKMode flicker detection result; please refer to Ae_feature.h
     */
    MRESULT setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode);
    /**  
     * @brief set camera mode
     * @param [in] u4NewAECamMode camera mode; please refer to CamDefs.h
     */
    MRESULT setAECamMode(MUINT32 u4NewAECamMode);
    /**  
     * @brief set capture mode
     * @param [in] u4NewAEShotMode capture mode; please refer to CamDefs.h
     */
    MRESULT setAEShotMode(MUINT32 u4NewAEShotMode);
    /**  
     * @brief set limiter control
     * @param [in] bAELimter enable or disable AE limiter
     */
    MRESULT setAELimiterMode(MBOOL bAELimter);
    /**  
     * @brief set AE mode
     * @param [in] u4NewAEMode AE mode; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setAEMode(MUINT32 u4NewAEMode);
    /**  
     * @brief get AE mode
     * @return the AE mode value; please refer to Kd_camera_feature_enum.h
     */
    MINT32 getAEMode() const;
    /**  
     * @brief lock or unlock AE
     * @param [in] bAELock lock AE (MTRUE) or unlock AE (MFALSE)
     */
    MRESULT setAELock(MBOOL bAELock);
    /**  
     * @brief set Zoom window information
     * @param [in] u4XOffset horizontal offset value
     * @param [in] u4YOffset vertical offset value
     * @param [in] u4Width horizontal width value
     * @param [in] u4Height vertical height value
     */
    MRESULT setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    /**  
     * @brief enable AE
     */
    MRESULT enableAE();
    /**  
     * @brief disable AE
     */
    MRESULT disableAE();
    /**  
     * @brief capture dynamic flare calculate
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] bIsStrobe On or OFF strobe
     */
    MRESULT doCapFlare(MVOID *pAEStatBuf, MBOOL bIsStrobe);
    /**  
     * @brief Auto focus AE calculate
     * @param [in] pAEStatBuf AE statistic buffer pointer
     */
    MRESULT doAFAE(MINT32 i4FrameCount, MVOID *pAEStatBuf);
    /**  
     * @brief AE calculate the capture parameters
     * @param [in] bIsStrobeFired On or OFF strobe
     * @param [in] pAEStatBuf AE statistic buffer pointer
     */
    MRESULT doPreCapAE(MBOOL bIsStrobeFired, MINT32 i4FrameCount, MVOID *pAEStatBuf);
    /**  
     * @brief send the capture paramters to sensor and isp
     */
    MRESULT doCapAE();
    /**  
     * @brief Backup the AE information
     */
    MRESULT doBackAEInfo();
    /**  
     * @brief Restore the AE information
     */
    MRESULT doRestoreAEInfo();
    /**  
     * @brief Calculate the preview AE information
     * @param [in] pAEStatBuf AE statistic buffer pointer
     */
    MRESULT doPvAE(MINT32 i4FrameCount, MVOID *pAEStatBuf, MBOOL bVideoMode);
    /**  
     * @brief get the AE debug parser information
     * @param [out] rAEDebugInfo debug information; please refer to Dbg_ae_param.h
     */
    MRESULT getDebugInfo(AE_DEBUG_INFO_T &rAEDebugInfo);
    /**  
     * @brief get AE luminance value
     * @param [in] isStrobeOn the LV value include the strobe or not
     * @return the AE luminance value
     */    
    MINT32 getLVvalue(MBOOL isStrobeOn);
    /**  
     * @brief get AE brightness value
     * @return the AE brightness value
     */    
    MINT32 getBVvalue();
    /**  
     * @brief get AE capture luminance value
     * @return the AE capture luminance value
     */    
    MINT32 getCaptureLVvalue();
    /**  
     * @brief get AE maximun meter area number
     * @return the AE support max area value
     */    
    MUINT32 getAEMaxMeterAreaNum();
    /**  
     * @brief get AE EV compensation index
     * @return the AE EV index value
     */    
    MINT32 getEVCompensateIndex();
    /**  
     * @brief get AE preview, capture and strobe Pline table information
     * @param [out] a_PrvAEPlineTable current preview AE Pline table pointer; please refer to camera_custom_AEPlinetable.h
     * @param [out] a_CapAEPlineTable current capture AE Pline table pointer; please refer to camera_custom_AEPlinetable.h
     * @param [out] a_StrobeAEPlineTable current strobe AE Pline table pointer; please refer to camera_custom_AEPlinetable.h
     */    
    MRESULT getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAFPlineInfo &a_StrobeAEPlineTable);
    /**  
     * @brief get Sensor device information
     * @param [out] a_rDeviceInfo sensor information structure; please refer to Camera_custom_nvram.h
     */    
    MRESULT getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo);
    /**  
     * @brief AE need lock before AF or not
     * @return AE need lock before AF (MTRUE) or AE lock after AF lock (MFALSE)
     */    
    MBOOL IsDoAEInPreAF();
    /**  
     * @brief AE converge stable or not
     * @return AE stable (MTRUE) or AE converge continue (MFALSE)
     */    
    MBOOL IsAEStable();
    /**  
     * @brief the strobe trigger threshold is bigger than brightness value or not
     * @return Strobe trigger threshold is bigger than brightness value (MTRUE) or strobe trigger threshold is smaller than brightness value (MFALSE)
     */    
    MBOOL IsStrobeBVTrigger();
    /**  
     * @brief get preview AE parameters information
     * @param [out] a_rPreviewInfo AE information structure; please refer to Ae_param.h
    */        
    MRESULT getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo);
    /**  
     * @brief update preview AE parameters
     * @param [in] a_rPreviewInfo preview AE parameters information
     * @param [in] i4AEidxNext new AE index value
    */ 
    MRESULT updatePreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext);
    /**  
     * @brief get capture AE parameters information
     * @param [in] index capture index information, the value is 0~2
     * @param [in] i4EVidx increase or decrease capture AE parameters information
     * @param [out] a_rCaptureInfo AE information structure; please refer to Ae_param.h
    */        
    MRESULT getCaptureParams(MINT8 index, MINT32 i4EVidx, AE_MODE_CFG_T &a_rCaptureInfo);
    /**  
     * @brief update capture AE parameters
     * @param [in] a_rCaptureInfo capture AE parameters information
    */        
    MRESULT updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo);
    /**  
     * @brief get AE meter area luminance value
     * @param [in] rWinSize meter area information; please refer to Ae_param.h
     * @param [out] iYvalue luminance value
    */        
    MRESULT getAEMeteringYvalue(AEMeterArea_T rWinSize, MUINT8 *iYvalue);
    /**  
     * @brief get AE 25 blocks luminance value
     * @param [in] size at most 25
     * @param [out] pYvalues luminance values
    */
    MRESULT getAEBlockYvalues(MUINT8 *pYvalues, MUINT8 size);	
    /**  
     * @brief get High dynamic range capture information
     * @param [out] strHDROutputInfo capture information; please refer to aaa_hal_base.h
    */        
    MRESULT getHDRCapInfo(Hal3A_HDROutputParam_T & strHDROutputInfo);
    /**  
     * @brief get real time AE parameters information
     * @param [out] a_strFrameInfo previiew AE information; please refer to aaa_hal_base.h
    */        
    MRESULT getRTParams(FrameOutputParam_T &a_strFrameInfo);
    /**  
     * @brief set AE face detection area and weight information
     * @param [in] a_sFaces face detection information; please refer to Faces.h
     */
    MRESULT setFDInfo(MVOID* a_sFaces);
    /**  
     * @brief set strobe on or off infomation
     * @param [in] bIsStrobeOn strobe on (MTRUE) or strobe off (MFALSE)
     */
    MRESULT setStrobeMode(MBOOL bIsStrobeOn);
    /**  
     * @brief set phone rotate degree
     * @param [in] i4RotateDegree rotate degree. The value is 0, 90, 180 or 270 only.
     */
    MRESULT setAERotateDegree(MINT32 i4RotateDegree);
    /**  
     * @brief get AE algorithm condition result
     * @param [out] i4AECondition AE condition value. please refer to Ae_param.h
     */
    MBOOL getAECondition(MUINT32 i4AECondition);
    /**  
     * @brief get LCE AE information 
     * @param [out] a_rLCEInfo LCE AE information; please refer to aaa_hal_base.h
    */       
    MRESULT getLCEPlineInfo(LCEInfo_T &a_rLCEInfo);
    /**  
     * @brief get Face AE information 
     * @return the luminance value change
    */       
    MINT16 getAEFaceDiffIndex();
    /**  
     * @brief update the sensor delay information
     * @param [in] i4SutterDelay sensor shutter delay information
     * @param [in] i4SensorGainDelay sensor gain delay information
     * @param [in] i4IspGainDelay isp gain delay information
    */        
    MRESULT updateSensorDelayInfo(MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay);
    /**  
     * @brief get brightness value by frame 
     * @param [out] bFrameUpdate frame update (MTRUE) or no update (MFALSE)
     * @param [out] iYvalue luminance value
    */       
    MRESULT getBrightnessValue(MBOOL * bFrameUpdate, MINT32* i4Yvalue);
    /**  
     * @brief set the object tracking information to AE 
     * @param [in] a_sOT object tracking structure pointer
    */       
    MRESULT setOTInfo(MVOID* a_sOT);
    /**  
     * @brief get NVRAM data
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */    
    MRESULT getNvramData(MINT32 i4SensorDev);

    // CCT feature APIs.
    MINT32 CCTOPAEEnable();
    MINT32 CCTOPAEDisable();
    MINT32 CCTOPAEGetEnableInfo(MINT32 *a_pEnableAE, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetAEMode(MINT32 a_AEMode);
    MINT32 CCTOPAEGetAEMode(MINT32 *a_pAEMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetMeteringMode(MINT32 a_AEMeteringMode);
    MINT32 CCTOPAEApplyExpParam(MVOID *a_pAEExpParam);
    MINT32 CCTOPAESetFlickerMode(MINT32 a_AEFlickerMode);
    MINT32 CCTOPAEGetExpParam(MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEGetFlickerMode(MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEGetMeteringMode(MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEApplyNVRAMParam(MVOID *a_pAENVRAM);
    MINT32 CCTOPAEGetNVRAMParam(MVOID *a_pAENVRAM, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESaveNVRAMParam();
    MINT32 CCTOPAEGetCurrentEV(MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen);
    MINT32 CCTOPAELockExpSetting();
    MINT32 CCTOPAEUnLockExpSetting();
    MINT32 CCTOPAEGetIspOB(MUINT32 *a_pIspOB, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetIspOB(MUINT32 a_IspOB);
    MINT32 CCTOPAEGetIspRAWGain(MUINT32 *a_pIspRawGain, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetIspRAWGain(MUINT32 a_IspRAWGain);
    MINT32 CCTOPAESetSensorExpTime(MUINT32 a_ExpTime);
    MINT32 CCTOPAESetSensorExpLine(MUINT32 a_ExpLine) const;
    MINT32 CCTOPAESetSensorGain(MUINT32 a_SensorGain) const;
    MINT32 CCTOPAESetCaptureMode(MUINT32 a_CaptureMode);
    MINT32 CCTOSetCaptureParams(MVOID *a_pAEExpParam);
    MINT32 CCTOGetCaptureParams(MVOID *a_pAEExpParam);
    MINT32 CCTOPAEGetFlareOffset(MUINT32 a_FlareThres, MUINT32 *a_pAEFlareOffset, MUINT32 *a_pOutLen);
    MINT32 CCTOPSetAETargetValue(MUINT32 u4AETargetValue);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**  
     * @brief get sensor resolution
     */
    MRESULT getSensorResolution();
    /**  
     * @brief AE init
     * @param [in] rParam camera parameter; please refer to aaa_hal_base.h
     */  
    MRESULT AEInit(Param_T &rParam);
    /**  
     * @brief Copy the AE information for sensor and isp control
     * @param [in] sAEOutputInfo AE parameter information; please refer to Ae_param.h
     * @param [out] sAEInfo keep the AE parameter information; please refer to Ae_param.h
     */  
    MRESULT copyAEInfo2mgr(AE_MODE_CFG_T *sAEOutputInfo, strAEOutput *sAEInfo);
    /**  
     * @brief prepare the AE parameter information for capture
     */    
    MRESULT prepareCapParams();
    /**  
     * @brief prepare the AE capture parameters for high dynamic range
     */    
    MRESULT updateCapParamsByHDR();
    /**  
     * @brief Send the AE parameter information to sensor and isp
     * @param [in] eNewAEState next AE state
     */  
    MRESULT UpdateSensorISPParams(AE_STATE_T eNewAEState);
    /**  
     * @brief Send the flare parameter information to isp
     */      
    MRESULT UpdateFlare2ISP();
    /**  
     * @brief camera AE preview init
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam camera parameter; please refer to aaa_hal_base.h
     */       
    MRESULT PreviewAEInit(MINT32 i4SensorDev, Param_T &rParam);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IAeAlgo* m_pIAeAlgo;
    AE_CCT_CFG_T m_AeMgrCCTConfig;
    EZOOM_WINDOW_T m_eZoomWinInfo;
    AEMeteringArea_T m_eAEMeterArea;
    AEMeterArea_T m_eAEFDArea;
    MINT32 m_i4SensorDev;
    MINT32 m_BVvalue;
    MINT32 m_BVvalueWOStrobe;
    MINT32 m_i4EVvalue;
    MINT32 m_i4WaitVDNum;
    MINT32 m_i4RotateDegree;
    MINT32 m_i4TimeOutCnt;
    MINT32 m_i4ShutterDelayFrames;
    MINT32 m_i4SensorGainDelayFrames;
    MINT32 m_i4SensorGainDelayFramesWOShutter;
    MINT32 m_i4IspGainDelayFrames;
    MINT32 m_i4AEidxCurrent;  // current AE idx
    MINT32 m_i4AEidxNext;   // next AE idx
    MINT16 m_i2AEFaceDiffIndex;
    MUINT32 m_u4PreExposureTime;
    MUINT32 m_u4PreSensorGain;
    MUINT32 m_u4PreIspGain;
    MUINT32 m_u4SmoothIspGain;
    MUINT32 m_u4AECondition;
    MUINT32 m_u4DynamicFrameCnt;

    MBOOL m_bOneShotAEBeforeLock;
    MBOOL m_bAEModeChanged;
    MBOOL m_bAELock;
    MBOOL m_bEnableAE;
    MBOOL m_bVideoDynamic;
    MBOOL m_bRealISOSpeed;
    MBOOL m_bAElimitor;
    MBOOL m_bAEStable;
    MBOOL m_bAEReadyCapture;
    MBOOL m_bLockExposureSetting;
    MBOOL m_bStrobeOn;
    MBOOL m_bAEMgrDebugEnable;
    MBOOL m_bRestoreAE;
    MBOOL m_bOtherIPRestoreAE;
    LIB3A_AE_MODE_T m_eAEMode;     // change AE Pline
    MFLOAT  m_fEVCompStep;
    MINT32  m_i4EVIndex;
    LIB3A_AE_METERING_MODE_T    m_eAEMeterMode;
    LIB3A_AE_ISO_SPEED_T    m_eAEISOSpeed;   // change AE Pline
    LIB3A_AE_FLICKER_MODE_T    m_eAEFlickerMode;    // change AE Pline
    MINT32    m_i4AEMaxFps;
    MINT32    m_i4AEMinFps;
    LIB3A_AE_FLICKER_AUTO_MODE_T    m_eAEAutoFlickerMode;   // change AE Pline
    EAppMode m_eCamMode;
    LIB3A_AECAM_MODE_T m_eAECamMode;
    EShotMode m_eShotMode;
    strAETable m_CurrentPreviewTable;
    strAETable m_CurrentCaptureTable;
    LIB3A_AE_EVCOMP_T m_eAEEVcomp;
    AE_MODE_CFG_T mPreviewMode;
    AE_MODE_CFG_T mCaptureMode;
    Hal3A_HDROutputParam_T m_strHDROutputInfo;
    AE_STATE_T m_AEState;
    MBOOL m_bIsAutoFlare;
    MBOOL m_bFrameUpdate;
    MINT32 m_i4ObjectTrackNum;
    MINT32 const* m_pIsAEActive;    
    MINT32 const m_i4AECycleNum;
        
    CameraMeteringArea_T m_backupMeterArea;
    int m_isAeMeterAreaEn;
};

};  //  namespace NS3A
#endif // _AE_MGR_H_

