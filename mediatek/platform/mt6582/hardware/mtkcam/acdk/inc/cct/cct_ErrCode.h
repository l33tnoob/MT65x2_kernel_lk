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


//! \file  cct_ErrCode.h

#ifndef _CCT_ERRCODE_H_
#define _CCT_ERRCODE_H_

#include "mtkcam/acdk/AcdkTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//! Helper macros to define error code
#define ERRCODE(modid, errid)  ((MINT32)((MUINT32)((modid & 0xff) << 20) | (MUINT32)(errid & 0xff)))

//! Helper macros to define ok code
#define OKCODE(modid, okid)    ((MINT32)((MUINT32)((modid & 0xff) << 20) | (MUINT32)(okid & 0xff)))

//! Helper macros to indicate succeed
#define SUCCEEDED(Status)   ((MINT32)(Status) >= 0)

//! Helper macros to indicate fail
#define FAILED(Status)      ((MINT32)(Status) < 0)


/*********************************************************************************
*
*********************************************************************************/

/**
*@brief Use enum to check duplicated error ID
*/
enum
{
    MODULE_CCT_CALIBRATION  = 0x91,    //! module calibration
    MOUDLE_CCT_IMGTOOL      = 0x92,    //! module imagetool
    MODULE_CCT_CCAP         = 0x93,    //! module ccap
};

/*********************************************************************************
*
*********************************************************************************/

//! Helper macros to define ImageTool ok code
#define CCT_IMGTOOL_OKCODE(errid)          OKCODE(MOUDLE_CCT_IMGTOOL, errid)

//! Helper macros to define ImageTool error code
#define CCT_IMGTOOL_ERRCODE(errid)         ERRCODE(MOUDLE_CCT_IMGTOOL, errid)

/**
*@brief Return value of Image Tool
*/
enum
{
    S_CCT_IMGTOOL_OK             = CCT_IMGTOOL_OKCODE(0),
    E_CCT_IMGTOOL_BAD_ARG        = CCT_IMGTOOL_ERRCODE(0x0001),   //! bad arguments
    E_CCT_IMGTOOL_API_FAIL       = CCT_IMGTOOL_ERRCODE(0x0002),   //! API Fail
    E_CCT_IMGTOOL_NULL_OBJ       = CCT_IMGTOOL_ERRCODE(0x0003),   //! Null Obj
    E_CCT_IMGTOOL_TIMEOUT        = CCT_IMGTOOL_ERRCODE(0x0004),   //! Time out
    E_CCT_IMGTOOL_FILE_OPEN_FAIL = CCT_IMGTOOL_ERRCODE(0x0005),   //! Open File Fail
    E_CCT_IMGTOOL_MEMORY_MAX     = CCT_IMGTOOL_ERRCODE(0x0080)    //! Max error code
};

/*********************************************************************************
*
*********************************************************************************/

//! Helper macros to define calibration ok code
#define CCT_CALIBRATION_OKCODE(errid)          OKCODE(MODULE_CCT_CALIBRATION, errid)

//! Helper macros to define calibration error code
#define CCT_CALIBRATION_ERRCODE(errid)         ERRCODE(MODULE_CCT_CALIBRATION, errid)

/**
*@brief Return value of Calibration Tool
*/
enum
{
    S_CCT_CALIBRATION_OK               = CCT_CALIBRATION_OKCODE(0),
    E_CCT_CALIBRATION_BAD_ARG          = CCT_CALIBRATION_ERRCODE(0x0001),    //! bad arguments
    E_CCT_CALIBRATION_API_FAIL         = CCT_CALIBRATION_ERRCODE(0x0002),    //! API Fail
    E_CCT_CALIBRATION_NULL_OBJ         = CCT_CALIBRATION_ERRCODE(0x0003),    //! Null Obj
    E_CCT_CALIBRATION_TIMEOUT          = CCT_CALIBRATION_ERRCODE(0x0004),    //! Time out
    E_CCT_CALIBRATION_GET_FAIL         = CCT_CALIBRATION_ERRCODE(0x0005),    //! Get calibration result fail
    E_CCT_CALIBRATION_FILE_OPEN_FAIL   = CCT_CALIBRATION_ERRCODE(0x0006),    //! Open File Fail
    E_CCT_CALIBRATION_DISABLE          = CCT_CALIBRATION_ERRCODE(0x0007),    //! Disable
    E_CCT_CALIBRATION_COEF_OVERFLOW	   = CCT_CALIBRATION_ERRCODE(0x0008),    //! Shading Coef Overflow
    E_CCT_CALIBRATION_MEMORY_MAX       = CCT_CALIBRATION_ERRCODE(0x0080)     //! Max error code
};

/*********************************************************************************
*
*********************************************************************************/

//! Helper macros to define CCAP ok code
#define CCT_CCAP_OKCODE(errid)   OKCODE(MODULE_CCT_CCAP, errid)

//! Helper macros to define CCAP error code
#define CCT_CCAP_ERRCODE(errid)  ERRCODE(MODULE_CCT_CCAP, errid)

/**
*@brief Return value of CCAP
*/
enum
{
    S_CCT_CCAP_OK       = CCT_CCAP_OKCODE(0),
    E_CCT_CCAP_API_FAIL = CCT_CCAP_ERRCODE(0x0001),
};

#ifdef __cplusplus
}
#endif

#endif //end _CCT_ERRCODE_H_

