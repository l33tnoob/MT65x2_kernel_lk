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
 * @file awb_algo_if.h
 * @brief Interface to AWB algorithm library
 */

#ifndef _AWB_ALGO_IF_H_
#define _AWB_ALGO_IF_H_

namespace NS3A
{
/**  
 * @brief Interface to AWB algorithm library
 */
class IAwbAlgo {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    IAwbAlgo() {}
    virtual ~IAwbAlgo() {}

private: // disable copy constructor and copy assignment operator
    IAwbAlgo(const IAwbAlgo&);
    IAwbAlgo& operator=(const IAwbAlgo&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
	static  IAwbAlgo* createInstance();
	virtual MVOID   destroyInstance() = 0;
	virtual MRESULT initAWB(AWB_INIT_INPUT_T &a_rAWBInitInput,
	                        AWB_OUTPUT_T &a_rAWBOutput,
	                        AWB_STAT_CONFIG_T (&a_rAWBStatConfig)[AWB_STROBE_MODE_NUM][AWB_SENSOR_MODE_NUM][LIB3A_AWB_MODE_NUM]) = 0;
	virtual MRESULT setAWBStatConfig(const AWB_STAT_CONFIG_T &a_rAWBStatConfig) = 0;
	virtual MRESULT setAWBMode(LIB3A_AWB_MODE_T a_eAWBMode) = 0;
	virtual MRESULT handleAWB(AWB_INPUT_T &a_rAWBInput, AWB_OUTPUT_T &a_rAWBOutput) = 0;
	virtual MINT32 getSceneLV() = 0;
	virtual MINT32 getCCT() = 0;
	virtual MRESULT getASDInfo(AWB_ASD_INFO_T &a_rAWBASDInfo) = 0;
    virtual MRESULT getLightProb(AWB_LIGHT_PROBABILITY_T &a_rAWBLightProb) = 0;
	virtual MRESULT getDebugInfo(AWB_DEBUG_INFO_T &a_rAWBDebugInfo, AWB_DEBUG_DATA_T &a_rAWBDebugData) = 0;
	virtual MRESULT updateAWBParam(AWB_INIT_INPUT_T &a_rAWBInitInput, AWB_STAT_CONFIG_T (&a_rAWBStatConfig)[AWB_STROBE_MODE_NUM][AWB_SENSOR_MODE_NUM][LIB3A_AWB_MODE_NUM]) = 0;
};

}; // namespace NS3A

#endif

