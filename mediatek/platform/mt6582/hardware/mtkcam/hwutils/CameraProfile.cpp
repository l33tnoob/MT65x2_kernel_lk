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

#define LOG_TAG "CameraProfile"

#define BUILD_PLATFORM_CPTEVENTINFO_COMMON  gCPTEventInfo_Common
#define BUILD_PLATFORM_CPTEVENTINFO_V1      gCPTEventInfo_V1
//#define BUILD_PLATFORM_CPTEVENTINFO_V3      gCPTEventInfo_V3
#include <mtkcam/hwutils/CameraProfile.h>
#include <mtkcam/mmp/Profile.h>


/******************************************************************************
 *
 ******************************************************************************/
#define ARRAY_OF(a)     (sizeof(a)/sizeof(a[0]))


/******************************************************************************
 *  Camera Profiling Tool
 ******************************************************************************/
namespace CPTool
{

static bool gbInit = false; 

bool initPlatformProfile()
{
    if  ( ! initCommonProfile() )
    {
        return false;
    }
    //
    if  ( ! gbInit )
    {
        bool ret = false;

#if defined(BUILD_PLATFORM_CPTEVENTINFO_COMMON)
        ret = CPTRegisterEvents(BUILD_PLATFORM_CPTEVENTINFO_COMMON, ARRAY_OF(BUILD_PLATFORM_CPTEVENTINFO_COMMON));
        if  ( ! ret ) return false;
#endif
        //
#if defined(BUILD_PLATFORM_CPTEVENTINFO_V1)
        ret = CPTRegisterEvents(BUILD_PLATFORM_CPTEVENTINFO_V1, ARRAY_OF(BUILD_PLATFORM_CPTEVENTINFO_V1));
        if  ( ! ret ) return false;
#endif
        //
#if defined(BUILD_PLATFORM_CPTEVENTINFO_V3)
        ret = CPTRegisterEvents(BUILD_PLATFORM_CPTEVENTINFO_V3, ARRAY_OF(BUILD_PLATFORM_CPTEVENTINFO_V3));
        if  ( ! ret ) return false;
#endif
        //
        gbInit = true;
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace CPTool

