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
// AcdkCCTCtrl.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCCTCtrl.h
//! \brief

#ifndef _ACDKCCTCTRL_H_
#define _ACDKCCTCTRL_H_

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkBase.h"

namespace NSACDK {

typedef enum {
    ACDKCCTCTRL_NO_ERROR         = 0,            ///< The function work successfully
    ACDKCCTCTRL_UNKNOWN_ERROR    = 0x80000000,   ///< Unknown error    
    ACDKCCTCTRL_INVALID_DRIVER   = 0x80000001,
    ACDKCCTCTRL_API_FAIL         = 0x80000002,
    ACDKCCTCTRL_INVALID_PARA     = 0x80000004,
    ACDKCCTCTRL_NULL_OBJ         = 0x80000005,  
} ACDKCCTCTRL_ERROR_ENUM;

/*******************************************************************************
* ! \class AcdkCCTCtrl 
*******************************************************************************/
class AcdkCCTCtrl: public AcdkBase
{
public:
   /*******************************************************************************
    * 
    *******************************************************************************/	
    static AcdkCCTCtrl* createInstance();

    virtual void destroyInstance() = 0;    

    virtual MINT32 init () = 0; 

    virtual MINT32 uninit() = 0; 
 
    //\@param prvCb is for App to get preview buffer 
    virtual MINT32 startPreview(Func_CB prvCb) = 0;    

    virtual MINT32 stopPreview() = 0;

    // \@param mode is for preview/capture mode 
    // \@param imgType 0:raw, 1:jpeg 
    // \@param capCb is for App to get capture buffer 
    virtual MINT32 takePicture( 
                MUINT32 const mode, 
                MUINT32 const imgType, 
                Func_CB const capCb = 0, 
                MUINT32 const width = 0, 
                MUINT32 const height = 0,  
                MINT32 const isSaveImg = 0
    ) = 0; 

    virtual MINT32 getFrameCnt(MUINT32 &frameCnt) = 0; 

    virtual MINT32 setSrcDev(MINT32 srcDev) = 0; 

    virtual MINT32 quickViewImg(MUINT32 qvFormat) = 0;
 
    virtual MINT32 sendcommand(
                MUINT32 const a_u4Ioctl,
                MUINT8 *puParaIn,
                MUINT32 const u4ParaInLen,
                MUINT8 *puParaOut,
                MUINT32 const u4ParaOutLen,
                MUINT32 *pu4RealParaOutLen
    ) = 0; 

protected:
    
protected:
    AcdkCCTCtrl () {};
    virtual ~AcdkCCTCtrl() {};    
};
}
#endif //end AcdkCCTCtrl.h 
