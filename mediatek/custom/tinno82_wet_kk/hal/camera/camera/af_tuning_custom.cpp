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

#include "camera_custom_types.h"
#include "af_param.h"
#include "af_tuning_custom.h"

/*******************************************************************************
*
********************************************************************************/
AF_PARAM_T const&
getAFParam()
{
    static AF_PARAM_T g_AFparam = 
    {
        1,   //i4AFS_STEP_MIN_ENABLE
        4,   // i4AFS_STEP_MIN_NORMAL
        4,   // i4AFS_STEP_MIN_MACRO
        2,   // i4AFS_MODE  0 : singleAF, 1:smoothAF
        2,   // i4AFC_MODE  0 : singleAF, 1:smoothAF
        2,   // i4VAFC_MODE  0 : singleAF, 1:smoothAF     2:smoothAF + 3 points curve fitting.   
        1,   // i4ReadOTP 0 : disable, 1:enable 
        5,   // i4FD_DETECT_CNT
        5,   // i4FD_NONE_CNT
        
        50,  // i4FV_SHOCK_THRES
        30000,  // i4FV_SHOCK_OFFSET        
        5,   // i4FV_VALID_CNT 2
        20, // i4FV_SHOCK_FRM_CNT 100
        5    // i4FV_SHOCK_CNT 2
        
    };

    return g_AFparam;
}

AF_CONFIG_T const&
getAFConfig()
{
    static AF_CONFIG_T g_AFconfig;

    g_AFconfig.i4SGG_GAIN = 16;
    g_AFconfig.i4SGG_GMR1 = 31;
    g_AFconfig.i4SGG_GMR2 = 63;
    g_AFconfig.i4SGG_GMR3 = 127;    
    g_AFconfig.AF_DECI_1  = 0;
    g_AFconfig.AF_ZIGZAG  = 0;
    g_AFconfig.AF_ODD     = 0;
    g_AFconfig.AF_FILT1[0] = 15;
    g_AFconfig.AF_FILT1[1] = 36;
    g_AFconfig.AF_FILT1[2] = 43;
    g_AFconfig.AF_FILT1[3] = 36;
    g_AFconfig.AF_FILT1[4] = 22;
    g_AFconfig.AF_FILT1[5] = 10;
    g_AFconfig.AF_FILT1[6] = 3;
    g_AFconfig.AF_FILT1[7] = 0;
    g_AFconfig.AF_FILT1[8] = 0;
    g_AFconfig.AF_FILT1[9] = 0;    
    g_AFconfig.AF_FILT1[10] = 0;
    g_AFconfig.AF_FILT1[11] = 0;

    g_AFconfig.AF_FILT2[0] =0x100;
    g_AFconfig.AF_FILT2[1] =0x100;
    g_AFconfig.AF_FILT2[2] =0x700;
    g_AFconfig.AF_FILT2[3] =0x700;




    g_AFconfig.AF_TH[0] = 5;
    g_AFconfig.AF_TH[2] = 5;

    g_AFconfig.AF_THEX = 5;

    return g_AFconfig;
}

