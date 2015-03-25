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
// AcdkLog.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkLog.h 
//! \brief

#ifndef _ACDKLOG_H_
#define _ACDKLOG_H_


//#define DEBUG_ACDK
#define MCAM_ASSERT_ON  1


#ifdef USING_MTK_LDVT
#include "uvvf.h"
#elif defined DEBUG_ACDK
#include <cutils/log.h>
#else
#include <cutils/xlog.h>
#endif


#ifdef USING_MTK_LDVT

#undef __func__
#define __func__ __FUNCTION__

#define ACDK_LOGD(fmt,arg...)  VV_MSG("[%s] " fmt, __func__, ##arg)
#define ACDK_INFO(fmt,arg...)  VV_MSG("[%s]INF(%5d):" fmt, __func__, __LINE__, ##arg)
#define ACDK_WARN(fmt,arg...)  VV_MSG("[%s]WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define ACDK_LOGE(fmt,arg...)  VV_ERRMSG("{#%d:%s}Err : " fmt,__LINE__,__FILE__, ##arg)


#elif defined DEBUG_ACDK

#define ACDK_LOGD(fmt, arg...) LOGD(fmt, ##arg);
#define ACDK_LOGE(fmt, arg...) LOGE("Err: %5d:, "fmt, __LINE__, ##arg);

#else


#undef __func__
#define __func__ __FUNCTION__

//#define ACDK_LOGD(fmt,...)      XLOGD("[%s]"fmt, __FUNCTION__, ##__VA_ARGS__)
//#define ACDK_INFO(fmt,...)      XLOGI("[%s]"fmt, __FUNCTION__, ##__VA_ARGS__)
//#define ACDK_WARN(fmt,...)      XLOGW("[%s]"fmt, __FUNCTION__, ##__VA_ARGS__)
//#define ACDK_LOGE(fmt,...)      XLOGE("[%s]Err"fmt, __FUNCTION__, ##__VA_ARGS__)

//#define ACDK_LOGD(fmt,arg...)      XLOGD(fmt, ##arg)
//#define ACDK_INFO(fmt,arg...)      XLOGI(fmt, ##arg)
//#define ACDK_WARN(fmt,arg...)      XLOGW(fmt, ##arg)
//#define ACDK_LOGE(fmt,arg...)      XLOGE("{#%d:%s}Err : "fmt,__LINE__,__FILE__, ##arg)

#define ACDK_LOGD(fmt,arg...)      XLOGD("[%s] " fmt, __func__, ##arg)
#define ACDK_INFO(fmt,arg...)      XLOGI("[%s]INF(%5d):" fmt, __func__, __LINE__, ##arg)
#define ACDK_WARN(fmt,arg...)      XLOGW("[%s]WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define ACDK_LOGE(fmt,arg...)      XLOGE("{#%d:%s}Err : " fmt,__LINE__,__FILE__, ##arg)


#endif

#define ACDK_LOGD_DYN(flag, ...)      do { if ( (flag) ) { ACDK_LOGD(__VA_ARGS__); } }while(0)
#define ACDK_INFO_DYN(flag, ...)      do { if ( (flag) ) { ACDK_INFO(__VA_ARGS__); } }while(0)
#define ACDK_WARN_DYN(flag, ...)      do { if ( (flag) ) { ACDK_WARN(__VA_ARGS__); } }while(0)
#define ACDK_LOGE_DYN(flag, ...)      do { if ( (flag) ) { ACDK_LOGE(__VA_ARGS__); } }while(0)



#if MCAM_ASSERT_ON

#if USING_MTK_LDVT
#define ACDK_ASSERT(x, str); \
        if (x) {} \
        else { \
            printf("[Assert %s, %d]: %s", __FILE__, __LINE__, str); \
            VV_ERRMSG("[Assert %s, %d]: %s", __FILE__, __LINE__, str); \
            while(1) { \
                usleep(500 * 1000); \
            } \
        }
#elif defined DEBUG_ACDK
#define ACDK_ASSERT(x, str); \
        if (x) {} \
        else { \
            printf("[Assert %s, %d]: %s", __FILE__, __LINE__, str); \
            LOGE("[Assert %s, %d]: %s", __FILE__, __LINE__, str); \
            while(1) { \
                usleep(500 * 1000); \
            } \
        }
#else
#define ACDK_ASSERT(x, str); \
        if (x) {} \
        else { \
            printf("[Assert %s, %d]: %s", __FILE__, __LINE__, str); \
            XLOGE("[Assert %s, %d]: %s", __FILE__, __LINE__, str); \
            while(1) { \
                usleep(500 * 1000); \
            } \
        }
#endif

#else
#define ACDK_ASSERT(x, str)
#endif


#endif //end _ACDKLOG_H_

