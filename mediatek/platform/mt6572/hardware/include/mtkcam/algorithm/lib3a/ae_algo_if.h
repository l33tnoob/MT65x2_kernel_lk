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
	static  IAeAlgo* createInstance();
	virtual MVOID   destroyInstance() = 0;
	virtual MRESULT initAE(const AE_INITIAL_INPUT_T *a_pAEInitParam, strAEOutput *a_pAEOutput, AE_STAT_PARAM_T *a_pAEConfig) = 0;
	virtual MRESULT updateAEParam(const AE_INITIAL_INPUT_T *a_pAEUpdateParam) = 0;
	virtual MRESULT setAEMeteringMode(LIB3A_AE_METERING_MODE_T i4NewAEMeteringMode) = 0;
	virtual MRESULT setAEMode(LIB3A_AE_MODE_T  a_eAEMode) = 0;
	virtual MRESULT switchCapureDiffEVState(strAEOutput *aeoutput, MINT8 iDiffEV) = 0;
       virtual MRESULT handleAE(strAEInput*  a_Input,strAEOutput* a_Output) = 0;
       virtual MRESULT setIsoSpeed(LIB3A_AE_ISO_SPEED_T  a_eISO) = 0;
       virtual MRESULT setAEFlickerMode(LIB3A_AE_FLICKER_MODE_T a_eAEFlickerMode) = 0;
       virtual MRESULT setAEFlickerAutoMode(LIB3A_AE_FLICKER_AUTO_MODE_T a_eAEFlickerAutoMode) = 0;
       virtual MRESULT modifyHistogramWinConfig(EZOOM_WINDOW_T a_eZoomWindow, AE_STAT_PARAM_T *a_pAEHistConfig) = 0;
       virtual MRESULT setAEMeteringArea(AEMeteringArea_T *sNewAEMeteringArea) = 0;
       virtual MRESULT setAEFDArea(AEMeterArea_T* sNewAEFDArea) = 0;
       virtual MRESULT setEVCompensate(LIB3A_AE_EVCOMP_T a_eEVComp) = 0;
       virtual MRESULT setAEMinMaxFrameRate(MINT32 a_eAEMinFrameRate, MINT32 a_eAEMaxFrameRate) = 0;
       virtual MVOID setAElimitorEnable(MBOOL bAElimitorEnable) = 0;
       virtual MRESULT setAECamMode(LIB3A_AECAM_MODE_T a_eAECamMode) = 0;
       virtual MRESULT getDebugInfo(AE_DEBUG_INFO_T &a_rAEDebugInfo) = 0;
       virtual MVOID setAEVideoDynamicEnable(MBOOL bVdoEnable) = 0;
       virtual MRESULT setAERealISOSpeed(MBOOL bAERealISO) = 0;
       virtual MVOID lockAE(MBOOL bLockAE) = 0;
       virtual MVOID setAEVideoRecord(MBOOL bVdoRecord) = 0;
       virtual MRESULT getPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable) = 0;
       virtual MRESULT getSenstivityDeltaIndex(MUINT32 u4NextSenstivity) = 0;
       virtual MRESULT getAEMeteringAreaValue(AEMeterArea_T sAEMeteringArea, MUINT8 *iYvalue) = 0;
       virtual MRESULT getAEHistogram(MUINT32 *pAEHistogram) = 0;
       virtual MRESULT switchSensorExposureGain(AE_EXP_GAIN_MODIFY_T &rInputData, AE_EXP_GAIN_MODIFY_T &rOutputData) = 0;
       virtual MRESULT getAEInfoForISP(AE_INFO_T &rAEISPInfo) = 0;
       virtual MRESULT setStrobeMode(MBOOL bIsStrobeOn) = 0;
       virtual MRESULT setAERotateWeighting(MBOOL bIsRotateWeighting) = 0;
       virtual MVOID setAESatisticBufferAddr(void* a_pAEBuffer) = 0;
       virtual MRESULT getAELCEIndexInfo(MUINT32 *u4StartIdx, MUINT32 *u4EndIdx) = 0;
};

}; // namespace NS3A

#endif

