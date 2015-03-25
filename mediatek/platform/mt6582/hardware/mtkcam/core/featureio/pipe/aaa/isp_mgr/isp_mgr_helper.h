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
#ifndef _ISP_MGR_HELPER_H_
#define _ISP_MGR_HELPER_H_


namespace NSIspTuning
{


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <class T>
struct ISPType2Type;

#define BIND_ISP_TYPES(FROM_T, TO_T)\
    template <> struct ISPType2Type<FROM_T> { typedef TO_T type; }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BIND_ISP_TYPES(ISP_NVRAM_CTL_EN_T, ISP_MGR_CTL_EN_T);
BIND_ISP_TYPES(ISP_NVRAM_OBC_T, ISP_MGR_OBC_T);
BIND_ISP_TYPES(ISP_NVRAM_BPC_T, ISP_MGR_BNR_T);
BIND_ISP_TYPES(ISP_NVRAM_NR1_T, ISP_MGR_BNR_T);
BIND_ISP_TYPES(ISP_NVRAM_LSC_CFG_T, ISP_MGR_LSC_T);
BIND_ISP_TYPES(ISP_NVRAM_SL2_T, ISP_MGR_SL2_T);
BIND_ISP_TYPES(ISP_NVRAM_PGN_T, ISP_MGR_PGN_T);
BIND_ISP_TYPES(ISP_NVRAM_CFA_T, ISP_MGR_CFA_T);
BIND_ISP_TYPES(ISP_NVRAM_CCM_T, ISP_MGR_CCM_T);
BIND_ISP_TYPES(ISP_NVRAM_GGM_T, ISP_MGR_GGM_T);
BIND_ISP_TYPES(ISP_NVRAM_G2C_T, ISP_MGR_G2C_T);
BIND_ISP_TYPES(ISP_NVRAM_ANR_T, ISP_MGR_NBC_T);
BIND_ISP_TYPES(ISP_NVRAM_CCR_T, ISP_MGR_NBC_T);
BIND_ISP_TYPES(ISP_NVRAM_PCA_T, ISP_MGR_PCA_T);
BIND_ISP_TYPES(ISP_NVRAM_EE_T, ISP_MGR_SEEE_T);
BIND_ISP_TYPES(ISP_NVRAM_SE_T, ISP_MGR_SEEE_T);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <class ISP_XXX_T>
MVOID
getIspHWBuf(ESensorDev_T const eSensorDev, ISP_XXX_T& rParam)
{
    typedef typename ISPType2Type<ISP_XXX_T>::type ISP_MGR_T;
    ISP_MGR_T::getInstance(eSensorDev).get(rParam);
}


template <class ISP_XXX_T>
MVOID
putIspHWBuf(ESensorDev_T const eSensorDev, ISP_XXX_T const& rParam)
{
    typedef typename ISPType2Type<ISP_XXX_T>::type ISP_MGR_T;
    ISP_MGR_T::getInstance(eSensorDev).put(rParam);
}

template <class ISP_XXX_T>
MVOID
getIspReg(ESensorDev_T const eSensorDev, ISP_XXX_T& rParam)
{
    typedef typename ISPType2Type<ISP_XXX_T>::type ISP_MGR_T;
    ISP_MGR_T::getInstance(eSensorDev).reset();
    ISP_MGR_T::getInstance(eSensorDev).get(rParam);
}

#define prepareIspHWBuf putIspHWBuf

#if 0

//  Specialization: EFFECT EDGE -> REG: DM + EDGE + EE
template <>
MVOID
getIspHWBuf<ISP_EFFECT_EDGE_T>(ISP_EFFECT_EDGE_T& rParam);


//  Specialization: EFFECT EDGE -> REG: DM + EDGE + EE
template <>
MVOID
putIspHWBuf<ISP_EFFECT_EDGE_T>(ISP_EFFECT_EDGE_T const& rParam);


/*******************************************************************************
*
*******************************************************************************/
inline
MVOID
prepareIspHWBuf_enableShading(MBOOL const fgEnable)
{
    ISP_MGR_SHADING_T::getInstance().setEnableShading(fgEnable);
}


inline
MVOID
prepareIspHWBuf_enableNR1_DP(MBOOL const fgEnable)
{
    ISP_MGR_NR1_T::getInstance().setEnableDP(fgEnable);
}


inline
MVOID
prepareIspHWBuf_enableNR1_CT(MBOOL const fgEnable)
{
    ISP_MGR_NR1_T::getInstance().setEnableCT(fgEnable);
}


inline
MVOID
prepareIspHWBuf_enableNR1_NR(MBOOL const fgEnable)
{
    ISP_MGR_NR1_T::getInstance().setEnableNR(fgEnable);
}


inline
MVOID
prepareIspHWBuf_enableGamma(MBOOL const fgEnable)
{
    ISP_MGR_EE_T::getInstance().setEnableGamma(fgEnable);
}


/*******************************************************************************
*
*******************************************************************************/
#endif

};  //  NSIspTuning
#endif // _ISP_MGR_HELPER_H_

