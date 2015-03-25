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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkIF.cpp

#define LOG_TAG "ACDK_IF"


extern "C" {
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/mtkfb.h>
}

#include "AcdkLog.h"
#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkErrCode.h"
#include "mtkcam/acdk/AcdkCommon.h"
#include "AcdkBase.h"
#include "mtkcam/acdk/AcdkIF.h"


static MBOOL g_bAcdkOpend = MFALSE;

using namespace NSACDK;

/*static*/ AcdkBase *g_pAcdkBaseObj = NULL;

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////
//
//   MDK_Open () -
//!  brief ACDK I/F MDK_Open()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_Open()
{
    ACDK_LOGD("+");

    if(g_bAcdkOpend == MTRUE)
    {
        ACDK_LOGE("ACDK device already opened");
        return MFALSE;
    }

    g_pAcdkBaseObj = AcdkBase::createInstance();
    g_bAcdkOpend = MTRUE;

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   MDK_Close () -
//!  brief ACDK I/F MDK_Close()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_Close()
{
    ACDK_LOGD("+");

    if(g_bAcdkOpend == MFALSE)
    {
        ACDK_LOGE("Acdk device is not opened");
        return MFALSE;
    }

    g_pAcdkBaseObj->destroyInstance();
    g_bAcdkOpend = MFALSE;

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   MDK_Init () -
//!  brief ACDK I/F MDK_Init()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_Init()
{
    ACDK_LOGD("+");

    if(g_bAcdkOpend == MFALSE)
    {
        ACDK_LOGE("ACDK device is not opened");
        return MFALSE;
    }

    MINT32 mrRet;

    mrRet = g_pAcdkBaseObj->init();
    if(mrRet != 0)
    {
        ACDK_LOGE("Fail to init AcdkCCTCtrl");
        return MFALSE;
    }

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   MDK_DeInit () -
//!  brief ACDK I/F MDK_DeInit()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_DeInit()
{
    ACDK_LOGD("+");

    MINT32 err;

    if (g_bAcdkOpend == MFALSE)
    {
        ACDK_LOGE("ACDK device is not opened");
        return MFALSE;
    }

    err = g_pAcdkBaseObj->uninit();
    if (err != 0)
    {
        ACDK_LOGE("Fail to disable ACDK CamCtrl  err(0x%x)", err);
        return MFALSE;
    }

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   MDK_IOControl () -
//!  brief ACDK I/F MDK_IOControl()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_IOControl(MUINT32 a_u4Ioctl, ACDK_FEATURE_INFO_STRUCT *a_prAcdkFeatureInfo)
{
    ACDK_LOGD("+");

    if (g_bAcdkOpend == MFALSE)
    {
        ACDK_LOGE("ACDK device is not opened");
        return MFALSE;
    }

    MBOOL bRet = MTRUE;

    ACDK_LOGD("Reveive IOControl Code:0x%x", a_u4Ioctl);

    if(a_u4Ioctl > ACDK_COMMAND_START && a_u4Ioctl < ACDK_COMMAND_END)
    {
        ACDK_LOGD("ACDK_COMMAND");
        bRet = g_pAcdkBaseObj->sendcommand (a_u4Ioctl,
                                            a_prAcdkFeatureInfo->puParaIn,
                                            a_prAcdkFeatureInfo->u4ParaInLen,
                                            a_prAcdkFeatureInfo->puParaOut,
                                            a_prAcdkFeatureInfo->u4ParaOutLen,
                                            a_prAcdkFeatureInfo->pu4RealParaOutLen);
    }
	else
	{
		ACDK_LOGD("[%s] Can't interpret ADCK cmd\n", __FUNCTION__);
		return MFALSE;
	}


    ACDK_LOGD("-");
    return MTRUE;
}

#ifdef __cplusplus
} // extern "C"
#endif
