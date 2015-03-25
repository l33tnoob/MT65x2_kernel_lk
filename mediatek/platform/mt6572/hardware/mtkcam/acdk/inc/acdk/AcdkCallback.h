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

//! \file  AcdkCallback.h

#ifndef _ACDKCALLBACK_H_
#define _ACDKCALLBACK_H_


//! Prototype of ACDK Callback Function
typedef void (*pfAcdkCallback)(void *);

/**  
*@enum ACDK_CB_ENUM
*@brief Type of ACDK Callback
*/
enum ACDK_CB_ENUM
{
    ACDK_CB_ERR = 0,    //! error callback
    ACDK_CB_PREVIEW,    //! preview callback
    ACDK_CB_RAW,        //! RAW capture callback
    ACDK_CB_JPEG,       //! JPEG capture callback
    ACDK_CB_QV,         //! QV callback
    ACDK_CB_AF          //! AF callback
};

/**  
*@struct acdkCallbackParam_s
*@brief  ACDK callback parameter used by AcdkMain
*/
typedef struct acdkCallbackParam_s {
    MUINT32 type;
    MUINT32 addr1;
    MUINT32 addr2;
    MUINT32 imgSize;
    MUINT32 thubSize;
} acdkCallbackParam_t;


/**  
*@struct acdkCBInfo
*@brief  ACDK callback parameter used by acdkObserver
*/
struct acdkCBInfo
{
    void    *mCookie;
    MUINT32 mType;
    MUINT32 mAddr1;
    MUINT32 mAddr2;
    MUINT32 mDataSize1;
    MUINT32 mDataSize2;
    
    acdkCBInfo(
        void    *cookie, 
        MUINT32 aType, 
        MUINT32 aAddr1 = 0, 
        MUINT32 aAddr2 = 0,
        MUINT32 aDataSize1 = 0,
        MUINT32 aDataSize2 = 0)
        : mCookie(cookie)
        , mType(aType)
        , mAddr1(aAddr1)
        , mAddr2(aAddr2)
        , mDataSize1(aDataSize1)
        , mDataSize2(aDataSize2)
    {}
};


/**  
*@struct acdkObserver
*@brief ACDK will use acdkObserver to set and call callback function
*/
struct acdkObserver
{
    typedef pfAcdkCallback  CallbackFunc_t;
    
    CallbackFunc_t  mpfCallback;
    void            *mCookie;
   
    acdkObserver(
        CallbackFunc_t  pfCallback = 0, 
        void            *cookie = 0
    )
        : mpfCallback(pfCallback)
        , mCookie(cookie)
    {}
    
    inline  bool operator!() const
    {
        return  (0 == mpfCallback);
    }
    
    inline  void notify(MUINT32 u4Type, MUINT32 u4Addr1 = 0, MUINT32 u4Addr2 = 0, MUINT32 u4DataSize1 = 0,MUINT32 u4DataSize2 = 0)
    {
        if( mpfCallback )
        {
            acdkCBInfo info(mCookie, u4Type, u4Addr1, u4Addr2, u4DataSize1, u4DataSize2);
            mpfCallback(&info);
        }
    }
};

#endif  //  _ACDKCALLBACK_H_

