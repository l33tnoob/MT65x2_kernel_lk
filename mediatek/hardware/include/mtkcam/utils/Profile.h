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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_PROFILE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_PROFILE_H_
/******************************************************************************
 *
 ******************************************************************************/
#include <utils/String8.h>
#include <utils/Timers.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Utils {


/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *  get the time in micro-seconds
 ******************************************************************************/
inline
int64_t
getTimeInUs()
{
    return  ::ns2us(::systemTime());
}


/******************************************************************************
 *  get the time in milli-seconds
 ******************************************************************************/
inline
int64_t
getTimeInMs()
{
    return  ::ns2ms(::systemTime());
}


/******************************************************************************
 *
 ******************************************************************************/
class DurationTool
{
public:     ////        Interfaces.
                        DurationTool(char const*const szSubjectName);
                        DurationTool(char const*const szSubjectName, nsecs_t nsInitTimestamp);
                        //
    void                reset();
    void                reset(nsecs_t nsInitTimestamp);
                        //
    void                update();
    void                update(nsecs_t nsTimestamp);
                        //
    void                showFps() const;
                        //
    int32_t             getCount() const        { return mi4Count; }
    nsecs_t             getDuration() const     { return mnsEnd - mnsStart; }
                        //
protected:  ////        Data Members.
    android::String8    ms8SubjectName;
    //
    int32_t             mi4Count;
    nsecs_t             mnsStart;
    nsecs_t             mnsEnd;
};


/******************************************************************************
 *
 ******************************************************************************/
class CamProfile
{
public:     ////        Interfaces.
                        CamProfile(
                            char const*const pszFuncName, 
                            char const*const pszClassName = ""
                        );
                        //
                        inline
                        void
                        enable(bool fgEnable)
                        {
                            mfgIsProfile = fgEnable;
                        }
                        //
                        bool
                        print(
                            char const*const fmt = "", 
                            ...
                        ) const;
                        //
                        bool
                        print_overtime(
                            int32_t const msTimeInterval, 
                            char const*const fmt = "", 
                            ...
                        ) const;

protected:  ////        Data Members.
    char const*const    mpszClassName;
    char const*const    mpszFuncName;
    mutable int32_t     mIdx;
    int32_t const       mi4StartUs;
    mutable int32_t     mi4LastUs;
    bool                mfgIsProfile;
};


/******************************************************************************
*
*******************************************************************************/
};  // namespace Utils
};  // namespace NSCam
#endif  //  _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_PROFILE_H_

