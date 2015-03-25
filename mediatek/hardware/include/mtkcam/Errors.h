/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_ERRORS_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_ERRORS_H_
/******************************************************************************
 *
 ******************************************************************************/
#include <utils/Errors.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 * Error codes. 
 * All error codes are negative values.
 ******************************************************************************/
typedef int                     MERROR;


/******************************************************************************
 * Error codes. 
 * All error codes are negative values.
 ******************************************************************************/
enum
{
#define OK                      android::OK                     // 0: Everything's swell.
#define UNKNOWN_ERROR           android::UNKNOWN_ERROR          // 0x80000000,

#define NO_MEMORY               android::NO_MEMORY
#define INVALID_OPERATION       android::INVALID_OPERATION
#define BAD_VALUE               android::BAD_VALUE
#define BAD_TYPE                android::BAD_TYPE
#define NAME_NOT_FOUND          android::NAME_NOT_FOUND
#define PERMISSION_DENIED       android::PERMISSION_DENIED
#define NO_INIT                 android::NO_INIT
#define ALREADY_EXISTS          android::ALREADY_EXISTS
#define DEAD_OBJECT             android::DEAD_OBJECT
#define FAILED_TRANSACTION      android::FAILED_TRANSACTION
#define JPARKS_BROKE_IT         android::JPARKS_BROKE_IT        //-EPIPE

#define BAD_INDEX               android::BAD_INDEX
#define NOT_ENOUGH_DATA         android::NOT_ENOUGH_DATA
#define WOULD_BLOCK             android::WOULD_BLOCK
#define TIMED_OUT               android::TIMED_OUT
#define UNKNOWN_TRANSACTION     android::UNKNOWN_TRANSACTION
#define FDS_NOT_ALLOWED         android::FDS_NOT_ALLOWED

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_ERRORS_H_

