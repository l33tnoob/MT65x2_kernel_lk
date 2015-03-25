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
 * @file af_algo_if.h
 * @brief AF algorithm interface, for raw sensor.
 */
#ifndef _AF_ALGO_IF_H_
#define _AF_ALGO_IF_H_

namespace NS3A
{

/**  
 * @brief AF algorithm interface class
 */
class IAfAlgo {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    IAfAlgo() {}
    virtual ~IAfAlgo() {}

private:
    IAfAlgo(const IAfAlgo&);
    IAfAlgo& operator=(const IAfAlgo&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
	static  IAfAlgo* createInstance();
	virtual MVOID   destroyInstance() = 0;

    /**  
     * @brief Trigger focusing algorithm.
     */
	virtual MRESULT triggerAF() = 0;
    
    /**  
     * @brief Pause focusing algorithm.
     */
	virtual MRESULT pauseAF() = 0;    
    
    /**  
     * @brief Reset focusing algorithm.
     */
	virtual MRESULT resetAF() = 0;
    
    /**  
     * @brief Set AF algorithm mode.
     * @param [in] a_eAFMode Set AF mode for single/continous/Fullscan/MF; Please refer LIB3A_AF_MODE_T in af_feature.h
     */   
    virtual MRESULT setAFMode(LIB3A_AF_MODE_T a_eAFMode) = 0;
    
    /**  
     * @brief Initial AF algorithm.
     * @param [in] a_sAFInput Input AF algorithm settings from af manager; Please refer AF_INPUT_T in af_param.h
     * @param [in] a_sAFOutput Onput AF algorithm settings to af manager; Please refer AF_OUTPUT_T in af_param.h
     */  
	virtual MRESULT initAF(AF_INPUT_T a_sAFInput, AF_OUTPUT_T &a_sAFOutput) = 0;
    
    /**  
     * @brief Handle AF algorithm tasks.
     * @param [in] a_sAFInput Input AF algorithm settings from af manager; Please refer AF_INPUT_T in af_param.h
     * @param [in] a_sAFOutput Onput AF algorithm settings to af manager; Please refer AF_OUTPUT_T in af_param.h
     */ 
	virtual MRESULT handleAF(AF_INPUT_T a_sAFInput, AF_OUTPUT_T &a_sAFOutput) = 0;
    
    /**  
     * @brief Set AF parameters to AF algorithm.
     * @param [in] a_sAFParam Input AF algorithm settings from af manager; Please refer AF_PARAM_T in af_param.h
     * @param [in] a_sAFConfig Input AF algorithm settings from af manager; Please refer AF_CONFIG_T in af_param.h
     * @param [in] a_sAFNvram Input AF algorithm settings from af manager; Please refer AF_NVRAM_T in camera_custom_nvram.h.
     */ 
	virtual MRESULT setAFParam(AF_PARAM_T a_sAFParam, AF_CONFIG_T a_sAFConfig, AF_NVRAM_T a_sAFNvram) = 0;    
  
    /**  
     * @brief Send debug information to AF manager. For internal debug information.
     * @param [in] a_sAFDebugInfo debug information data pointer.;Please refer AF_DEBUG_INFO_T in dbg_af_param.h
     */  
  virtual MRESULT getDebugInfo(AF_DEBUG_INFO_T &a_sAFDebugInfo) = 0;
    
    /**  
     * @brief Set manual focus position in AF algorithm. When AF mode is MF, use it to set lens position.
     * @param [in] a_i4Pos Lens position. Usually value in 0~1023.
     */  
    virtual void    setMFPos(MINT32 a_i4Pos) = 0;

    /**  
     * @brief This function is used for AF factory calibration. It is called by ReadOTP in AF manager. It calculates and applies the factory data to AF table.
     * @param [in] a_i4InfPos Factory calibrated infinite lens position. 
     * @param [in] a_i4MacroPos Factory calibrated macro lens position. 
     */  
    virtual void    updateAFtableBoundary(MINT32 a_i4InfPos, MINT32 a_i4MacroPos) = 0;
    
    /**  
     * @brief Set face detection information. When face detection is turn ON, use it to set face window and counter to AF algorithm.
     * @param [in] a_sFDInfo Face detection information. ;Please refer AF_AREA_T in af_param.h
     */  
    virtual MRESULT setFDWin(AF_AREA_T a_sFDInfo) = 0;
    // AF v1.2
    virtual MRESULT setAcceSensorInfo(MINT32 acce[3], MUINT32 scale) = 0;		        
    virtual MRESULT setGyroSensorInfo(MINT32 gyro[3], MUINT32 scale) = 0;	
    virtual MRESULT setAEBlockInfo(MUINT8* winValues, MUINT8 winNum) = 0;
};

}; // namespace NS3A

#endif

