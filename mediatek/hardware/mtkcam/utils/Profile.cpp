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

#define LOG_TAG "MtkCam/Utils/Profile"
//
#include "MyUtils.h"
using namespace android;


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Utils {


/******************************************************************************
 *
 ******************************************************************************/
DurationTool::
DurationTool(char const*const szSubjectName)
    : ms8SubjectName(String8(szSubjectName))
    , mi4Count(0)
    , mnsStart(::systemTime())
    , mnsEnd(::systemTime())
{
}


/******************************************************************************
 *
 ******************************************************************************/
DurationTool::
DurationTool(char const*const szSubjectName, nsecs_t nsInitTimestamp)
    : ms8SubjectName(String8(szSubjectName))
    , mi4Count(0)
    , mnsStart(nsInitTimestamp)
    , mnsEnd(nsInitTimestamp)
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
DurationTool::
reset()
{
    reset(::systemTime());
}


/******************************************************************************
 *
 ******************************************************************************/
void
DurationTool::
reset(nsecs_t nsInitTimestamp)
{
    mi4Count = 0;
    mnsStart = nsInitTimestamp;
    mnsEnd = nsInitTimestamp;
}


/******************************************************************************
 *
 ******************************************************************************/
void
DurationTool::
update()
{
    update(::systemTime());
}


/******************************************************************************
 *
 ******************************************************************************/
void
DurationTool::
update(nsecs_t nsTimestamp)
{
    mi4Count++;
    mnsEnd = nsTimestamp;
}


/******************************************************************************
 *
 ******************************************************************************/
void
DurationTool::
showFps() const
{
    nsecs_t const   nsDuration = mnsEnd - mnsStart;
    float const     fFps = ((float)mi4Count/nsDuration)*1000000000LL;
    CAM_LOGD_IF(
        1, 
        "[%s] fps: %d / %lld ns = %3f", 
        ms8SubjectName.string(), mi4Count, nsDuration, fFps
    );
}


/******************************************************************************
 *
 ******************************************************************************/
CamProfile::
CamProfile(char const*const pszFuncName, char const*const pszClassName)
    : mpszClassName(pszClassName)
    , mpszFuncName(pszFuncName)
    , mIdx(0)
    , mi4StartUs(::ns2us(::systemTime()))
    , mi4LastUs(::ns2us(::systemTime()))
    , mfgIsProfile(false)
{
    mfgIsProfile = true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CamProfile::
print(char const*const fmt /*= ""*/, ...) const
{
    if  ( ! mfgIsProfile )
    {
        return  false;
    }

    int32_t const i4EndUs = ::ns2us(::systemTime());
    if  (0==mIdx)
    {
        va_list args;
        va_start(args, fmt);
        String8 result(String8::formatV(fmt, args));
        va_end(args);
        //
        CAM_LOGI(
            "{CamProfile}[%s::%s] %s: "
            "(%d-th) ===> [start-->now: %d ms]", 
            mpszClassName, mpszFuncName, result.string(), mIdx++, 
            (i4EndUs-mi4StartUs)/1000
        );
    }
    else
    {
        va_list args;
        va_start(args, fmt);
        String8 result(String8::formatV(fmt, args));
        va_end(args);
        //
        CAM_LOGI(
            "{CamProfile}[%s::%s] %s: "
            "(%d-th) ===> [start-->now: %d ms] [last-->now: %d ms]", 
            mpszClassName, mpszFuncName, result.string(), mIdx++, 
            (i4EndUs-mi4StartUs)/1000, (i4EndUs-mi4LastUs)/1000
        );
    }
    mi4LastUs = i4EndUs;
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CamProfile::
print_overtime(
    int32_t const msTimeInterval, 
    char const*const fmt /*= ""*/, 
    ...
) const
{
    if  ( ! mfgIsProfile )
    {
        return  false;
    }
    //
    bool ret = false;
    //
    int32_t const i4EndUs = ::ns2us(::systemTime());
    if  (0==mIdx)
    {
        int32_t const msElapsed = (i4EndUs-mi4StartUs)/1000;
        ret = ( msTimeInterval < msElapsed );
        if  ( ret )
        {
            va_list args;
            va_start(args, fmt);
            String8 result(String8::formatV(fmt, args));
            va_end(args);
            //
            CAM_LOGI(
                "{CamProfile}[%s::%s] %s: "
                "(%d-th) ===> (overtime > %d ms) [start-->now: %d ms]", 
                mpszClassName, mpszFuncName, result.string(), mIdx, 
                msTimeInterval, msElapsed
            );
            //
            goto lbExit;
        }
    }
    else
    {
        int32_t const msElapsed = (i4EndUs-mi4LastUs)/1000;
        ret = ( msTimeInterval < msElapsed );
        if  ( ret )
        {
            va_list args;
            va_start(args, fmt);
            String8 result(String8::formatV(fmt, args));
            va_end(args);
            //
            CAM_LOGI(
                "{CamProfile}[%s::%s] %s: "
                "(%d-th) ===> ( overtime > %d ms) [start-->now: %d ms] [last-->now: %d ms]", 
                mpszClassName, mpszFuncName, result.string(), mIdx, 
                msTimeInterval, (i4EndUs-mi4StartUs)/1000, msElapsed
            );
            //
            goto lbExit;
        }
    }
    //
lbExit:
    mIdx++;
    mi4LastUs = i4EndUs;
    return  ret;
}


};  // namespace Utils
};  // namespace NSCam

