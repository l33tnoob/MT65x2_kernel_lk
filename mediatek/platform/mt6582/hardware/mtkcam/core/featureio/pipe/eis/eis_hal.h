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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

//! \file  eis_hal.h

#ifndef _EIS_HAL_H_
#define _EIS_HAL_H_

#include <mtkcam/featureio/eis_hal_base.h>

using namespace android;

/**
  *@class EisHal
  *@brief Implementation of EisHalBase class
*/
class EisHal : public EisHalBase
{
public:

    /**
         *@brief Create EisHal object
         *@return
         *-EisHal object
       */
    static EisHalBase* getInstance();

    /**
         *@brief Destroy EisHal object
         *@param[in] userName : user name,i.e. who destroy EIS object
       */
    virtual MVOID destroyInstance(char const *userName);

    /**
         *@brief Config EIS
         *@details Call before pass1
         *@param[in] a_ehwMode : hardware scenario
         *@param[in] a_sEisConfig : size of image sent to EIS (TG out size or size after CDRZ)
         *
       */
    virtual MVOID  configEIS(NSHwScenario::EhwMode a_ehwMode, eisHal_config_t a_sEisConfig);

    /**
         *@brief Execute EIS
         *@return
         *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 doEIS();

    /**
         *@brief Get EIS algorithm result (CMV)
         *@param[in,out] a_CMV_X_Int : EIS algo result of X-direction integer part
         *@param[in,out] a_CMV_X_Flt  : EIS algo result of X-direction float part
         *@param[in,out] a_CMV_Y_Int : EIS algo result of Y-direction integer part
         *@param[in,out] a_CMV_Y_Flt  : EIS algo result of Y-direction float part
         *@param[in,out] a_TarWidth    : EIS width crop size
         *@param[in,out] a_TarHeight   : EIS height crop size
       */
    virtual MVOID getEISResult(MUINT32 &a_CMV_X_Int,
                                   MUINT32 &a_CMV_X_Flt,
                                   MUINT32 &a_CMV_Y_Int,
                                   MUINT32 &a_CMV_Y_Flt,
                                   MUINT32 &a_TarWidth,
                                   MUINT32 &a_TarHeight);

    /**
         *@brief Get EIS GMV
         *@details The value is 256x
         *@param[in,out] a_GMV_X : x-direction global motion vector between two frames
         *@param[in,out] a_GMV_Y  : y-direction global motion vector between two frames
       */
    virtual MVOID getEISGmv(MUINT32 &a_GMV_X, MUINT32 &a_GMV_Y);

    /**
         *@brief Return EIS HW statistic result
         *@param[in,out] a_pEIS_Stat : EIS_STATISTIC_STRUCT
       */
    virtual MVOID getEISStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat);

private:

    /**
         *@brief EisHal constructor
       */
    EisHal();

    /**
         *@brief EisHal destructor
       */
    virtual ~EisHal() {};

    /**
         *@brief Initialization function
       */
    virtual MINT32 init();

    /**
         *@brief Unitialization function
       */
    virtual MINT32 uninit();

    /**
         *@brief Get EIS customize info
         *@param[in,out] a_pDataOut : EIS_TUNING_PARA_STRUCT
       */
    virtual MVOID  getEISCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut);

    /**
         *@brief Dump EIS HW statistic info
         *@param[in,out] a_EISStat : EIS_STATISTIC_T
       */
    virtual MVOID  dumpStatistic(EIS_STATISTIC_STRUCT a_EISStat);

    /**
         *@brief Create IMem buffer
         *
         *@param[in,out] memSize : memory size, will align to L1 cache
         *@param[in,out] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *
         *@return
         *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 createMemBuf(MUINT32 &memSize, MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo);

    /**
         *@brief Destroy IMem buffer
         *
         *@param[in,out] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *
         *@return
         *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 destroyMemBuf(MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo);

    /***************************************************************************************/

    volatile MINT32 mUsers;
    mutable Mutex mLock;

    // EIS object
    EisDrvBase *m_pEisDrv;
    MTKEis *m_pEisAlg;

    // member variable
    MUINT32 mInput_W;
    MUINT32 mInput_H;
    MUINT32 mTarget_W;
    MUINT32 mTarget_H;
    MINT32 mCMV_X;
    MINT32 mCMV_Y;
    MUINT32 mFirstFlag;

    MINT32 mGMV_X;
    MINT32 mGMV_Y;

    // IMEM
    IMemDrv *m_pIMemDrv;
    IMEM_BUF_INFO mEisAlgoIMemBuf;

    // EIS config protection usage
    MBOOL mConfigPass;
};

#endif

