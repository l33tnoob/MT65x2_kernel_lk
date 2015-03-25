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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/**
 * @file ae_algo_if.h
 * @brief Interface to AE algorithm library
 */

#ifndef _AE_ALGO_IF_H_
#define _AE_ALGO_IF_H_

namespace NS3A
{
/**
 * @brief Interface to AE algorithm library
 */
class IAeAlgo {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    IAeAlgo() {}
    virtual ~IAeAlgo() {}

private: // disable copy constructor and copy assignment operator
    IAeAlgo(const IAeAlgo&);
    IAeAlgo& operator=(const IAeAlgo&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief create instance
     */
    static  IAeAlgo* createInstance();
    /**
     * @brief destroy instance
     */
    virtual MVOID   destroyInstance() = 0;
    /**
     * @brief AE init function
     * @param [in] a_pAEInitParam AE init input parameters; please refer to ae_param.h
     * @param [out] a_pAEOutput AE algorithm output; please refer to ae_param.h
     * @param [out] a_pAEConfig AE statistics config parameter; please refer to ae_param.h
     */
    virtual MRESULT initAE(const AE_INITIAL_INPUT_T *a_pAEInitParam, strAEOutput *a_pAEOutput, AE_STAT_PARAM_T *a_pAEConfig) = 0;
    /**
     * @brief update AE parameter information
     * @param [in] a_pAEInitParam AE input parameters; please refer to ae_param.h
     */
    virtual MRESULT updateAEParam(const AE_INITIAL_INPUT_T *a_pAEUpdateParam) = 0;
    /**
     * @brief set AE metering mode for AE algorithm
     * @param [in] i4NewAEMeteringMode metering mode value; please refer to ae_feature.h
     */
    virtual MRESULT setAEMeteringMode(LIB3A_AE_METERING_MODE_T i4NewAEMeteringMode) = 0;
    /**
     * @brief set AE mode for AE algorithm
     * @param [in] a_eAEMode AE mode; please refer to camera_custom_AEPlinetable.h
     */
    virtual MRESULT setAEMode(LIB3A_AE_MODE_T  a_eAEMode) = 0;
    /**
     * @brief output the AE capture parameter information by the difference EV compensation
     * @param [out] aeoutput AE capture information ouput; please refer to ae_param.h
     * @param [in] iDiffEV difference EV compensation value
     */
    virtual MRESULT switchCapureDiffEVState(strAEOutput *aeoutput, MINT8 iDiffEV) = 0;
    /**
     * @brief AE algorithm main function
     * @param [in] a_Input AE frame-based input parameters; please refer to ae_param.h
     * @param [out] a_Output AE algorithm output; please refer to ae_param.h
     */
    virtual MRESULT handleAE(strAEInput*  a_Input,strAEOutput* a_Output) = 0;
    /**
     * @brief set AE ISO speed
     * @param [in] a_eISO ISO speed value; please refer to camera_custom_AEPlinetable.h
     */
    virtual MRESULT setIsoSpeed(LIB3A_AE_ISO_SPEED_T  a_eISO) = 0;
    /**
     * @brief set Anti-banding mode to let AE choose difference AE Pline table
     * @param [in] a_eAEFlickerMode flicker mode; please refer to camera_custom_AEPlinetable.h
     */
    virtual MRESULT setAEFlickerMode(LIB3A_AE_FLICKER_MODE_T a_eAEFlickerMode) = 0;
    /**
     * @brief set flicker detection result mode to let AE choose difference AE Pline table
     * @param [in] a_eAEFlickerAutoMode flicker detection result; please refer to camera_custom_AEPlinetable.h
     */
    virtual MRESULT setAEFlickerAutoMode(LIB3A_AE_FLICKER_AUTO_MODE_T a_eAEFlickerAutoMode) = 0;
    /**
     * @brief output the AE statistic window config for AE algorithm
     * @param [in] a_eZoomWindow update AE algorithm calculate window information; please refer to Ae_param.h
     * @param [out] a_pAEHistConfig AE statistics config parameter; please refer to ae_param.h
     */
    virtual MRESULT modifyHistogramWinConfig(EZOOM_WINDOW_T a_eZoomWindow, AE_STAT_PARAM_T *a_pAEHistConfig) = 0;
    /**
     * @brief set AE meter area window and weight information
     * @param [in] sNewAEMeteringArea meter area; please refer to ae_param.h
     */
    virtual MRESULT setAEMeteringArea(AEMeteringArea_T *sNewAEMeteringArea) = 0;
    /**
     * @brief set AE face detection area and weight information
     * @param [in] sNewAEFDArea face detection information; please refer to ae_param.h
     */
    virtual MRESULT setAEFDArea(AEMeterArea_T* sNewAEFDArea) = 0;
    /**
     * @brief set AE EV compensation value
     * @param [in] a_eEVComp EV compensation value; please refer to ae_feature.h
     */
    virtual MRESULT setEVCompensate(LIB3A_AE_EVCOMP_T a_eEVComp) = 0;

    virtual MRESULT setFinerEVComp(MINT32 evComp) = 0;
    virtual MRESULT setFinerEVEnable(MBOOL enable) = 0;

    /**
     * @brief set AE min / max frame rate value
     * @param [in] a_eAEMinFrameRate minimun frame rate value
     * @param [in] a_eAEMaxFrameRate maximun frame rate value
     */
    virtual MRESULT setAEMinMaxFrameRate(MINT32 a_eAEMinFrameRate, MINT32 a_eAEMaxFrameRate) = 0;
    /**
     * @brief set limiter control for AE algorithm
     * @param [in] bAElimitorEnable enable or disable AE limiter
     */
    virtual MVOID setAElimitorEnable(MBOOL bAElimitorEnable) = 0;
    /**
     * @brief set camera mode for AE algorithm
     * @param [in] a_eAECamMode AE camera mode; please refer to ae_feature.h
     */
    virtual MRESULT setAECamMode(LIB3A_AECAM_MODE_T a_eAECamMode) = 0;
    /**
     * @brief get the information for AE algorithm debug
     * @param [out] a_rAEDebugInfo debug information; please refer to Dbg_ae_param.h
     */
    virtual MRESULT getDebugInfo(AE_DEBUG_INFO_T &a_rAEDebugInfo) = 0;
    /**
     * @brief set video dynamic frame rate enable or disable
     * @param [in] bVdoEnable enable or disable video dynamic frame rate
     */
    virtual MVOID setAEVideoDynamicEnable(MBOOL bVdoEnable) = 0;
    /**
     * @brief set ISO speed is real or not
     * @param [in] bAERealISO using real iso to calculate the sensor gain and isp gain or not
     */
    virtual MRESULT setAERealISOSpeed(MBOOL bAERealISO) = 0;
    /**
     * @brief lock or unlock AE
     * @param [in] bLockAE lock AE (MTRUE) or unlock AE (MFALSE)
     */
    virtual MVOID lockAE(MBOOL bLockAE) = 0;
    /**
     * @brief set video is the record state or not
     * @param [in] bVdoRecord video recording (MTRUE) or video not recording (MFALSE)
     */
    virtual MVOID setAEVideoRecord(MBOOL bVdoRecord) = 0;
    /**
     * @brief get AE preview and capturePline table information
     * @param [out] a_PrvAEPlineTable current preview AE Pline table pointer; please refer to camera_custom_AEPlinetable.h
     * @param [out] a_CapAEPlineTable current capture AE Pline table pointer; please refer to camera_custom_AEPlinetable.h
     */
    virtual MRESULT getPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable) = 0;
    /**
     * @brief get AE senstivity delta value information
     * @param [in] u4NextSenstivity the brightness different ratio
     */
    virtual MRESULT getSenstivityDeltaIndex(MUINT32 u4NextSenstivity) = 0;
    /**
     * @brief get AE meter area luminance value
     * @param [in] sAEMeteringArea meter area information; please refer to Ae_param.h
     * @param [out] iYvalue luminance value
    */
    virtual MRESULT getAEMeteringAreaValue(AEMeterArea_T sAEMeteringArea, MUINT8 *iYvalue) = 0;
    /**
     * @brief get AE histogram value
     * @param [out] pAEHistogram AE histgoram value pointer
    */
    virtual MRESULT getAEHistogram(MUINT32 *pAEHistogram) = 0;
    /**
     * @brief modify the sensor shutter and sensor gain to meet the sensor driver request
     * @param [in] rInputData input sensor shutter, sensor gain, isp gain and ISO speed; please refer to Ae_param.h
     * @param [out] rOutputData output sensor shutter, sensor gain, isp gain and ISO speed; please refer to Ae_param.h
    */
    virtual MRESULT switchSensorExposureGain(AE_EXP_GAIN_MODIFY_T &rInputData, AE_EXP_GAIN_MODIFY_T &rOutputData) = 0;
    /**
     * @brief get AE information for ISP tuning used
     * @param [out] rAEISPInfo output AE information for ISP tuning; please refer to Ae_param.h
    */
    virtual MRESULT getAEInfoForISP(AE_INFO_T &rAEISPInfo) = 0;
    /**
     * @brief set strobe on or off infomation
     * @param [in] bIsStrobeOn strobe on (MTRUE) or strobe off (MFALSE)
     */
    virtual MRESULT setStrobeMode(MBOOL bIsStrobeOn) = 0;
    /**
     * @brief set phone rotate AE weighting or not
     * @param [in] bIsRotateWeighting rotate AE weighting degree (MTRUE) or don't rotate (MFALSE).
     */
    virtual MRESULT setAERotateWeighting(MBOOL bIsRotateWeighting) = 0;
    /**
     * @brief set the AE statistic buffer pinter
     * @param [in] a_pAEBuffer AE statistic buffer pointer
     */
    virtual MVOID setAESatisticBufferAddr(void* a_pAEBuffer) = 0;
    /**
     * @brief get the AE blocks y values
     * @param [in] size at most 25
     * @param [out] pYvalues luminance values
     */
    virtual MVOID getAEBlockYvalues(MUINT8 *pYvalues, MUINT8 size) = 0;	
    /**
     * @brief get LCE index range information
     * @param [out] u4StartIdx LCE AE start index in the AE Pline table
     * @param [out] u4EndIdx LCE AE end index in the AE Pline table
    */
    virtual MRESULT getAELCEIndexInfo(MUINT32 *u4StartIdx, MUINT32 *u4EndIdx) = 0;
    /**
     * @brief capture dynamic flare calculate
     * @param [in] a_pAEBuffer AE statistic buffer pointer
     * @param [in] bWithStrobe On or OFF strobe
     */
    virtual MUINT32	CalculateCaptureFlare( void* a_pAEBuffer,MBOOL bWithStrobe  )=0;
    /**
     * @brief preview dynamic flare calculate
     * @param [in] a_pAEBuffer AE statistic buffer pointer
     */
    virtual MVOID DoPreFlare(void* a_pAEBuffer)=0;
    /**
     * @brief set preview flare value
     * @param [in] nPreFlare preview flare value in 12 bit domain
     */
    virtual void  SetPreviewFlareValue(MINT32 nPreFlare)=0;
    /**
     * @brief set capture flare value
     * @param [in] nPreFlare preview flare value in 12 bit domain
     */
    virtual void  SetCaptureFlareValue(MINT32 nCapFlare)=0;
    /**
     * @brief get brightness value in the preview mode
     * @return the brightness value
     */
    virtual MINT32 getBrightnessAverageValue(void) = 0;
    /**
     * @brief get capture luminance value
     * @param [out] i4CapLV capture luminance value
     */
    virtual MRESULT CalculateCaptureLV(MINT32 *i4CapLV) = 0;
    /**
     * @brief get flare offset value
     * @param [out] a_FlareOffsetCali flare offset calculate
     */    
    virtual MUINT32  CalculateFlareOffset(MUINT32 a_FlareOffsetCali) = 0;
    /**  
     * @brief set AE target value by calibration
     * @param [in] u4AETargetValue AE target valie value in 8 bit domain
     */
    virtual MRESULT setAETargetValue(MUINT32 u4AETargetValue) = 0;
    /**  
     * @brief update the AE next inde for preview
     * @param [in] i4AEIndex preview next index value
     */
    virtual MRESULT updateAEIndex(MINT32 i4AEIndex) = 0;
    /**  
     * @brief update AE object tracking statue
     * @param [in] bAEOTenable enable/disable object tracking
     */    
    virtual MRESULT setAEObjectTracking(MBOOL bAEOTenable) = 0;
};

}; // namespace NS3A

#endif

