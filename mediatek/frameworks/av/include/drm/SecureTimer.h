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

#ifndef SECURETIMER_H_
#define SECURETIMER_H_

#include <drm/drm_framework_common.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <time.h>

namespace android
{

class SecureTimer
{
public:
    static const int RESULT_OK = DRM_NO_ERROR;
    static const int RESULT_ERR = DRM_ERROR_UNKNOWN;

    static const int CLOCK_VALID = 0;
    static const int CLOCK_INVALID = -1;
    static const int CLOCK_NEED_SYNC = -2;

public:  // singleton pattern
    static SecureTimer& instance(void);

private:  // avoid explicit construct
    SecureTimer();
    SecureTimer(const SecureTimer& copy);
    SecureTimer& operator=(SecureTimer& other);

public:
    ~SecureTimer();

public:
    bool isValid();
    int updateTimeBase();
    int updateOffset();
    int updateDRMTime(time_t offset);
    int getDRMTime(time_t& t); // time_t: long in linux
    time_t getOffset();
    int load();
    int save();
    void reset();

private:
    time_t deviceTime();
    time_t deviceTicks();

private:
    static SecureTimer* m_pTimer; // for singleton pattern
    bool m_bIsValid;    // ture if the secure clock is in valid state
    time_t m_nOffset;   // in seconds, (t_device + offset == t_real)
    time_t m_nBaseTicks;// in seconds, the interval since last boot of system
    time_t m_nBaseTime; // in seconds, the device time value
    time_t m_nLastSync; // in seconds, the time after the offset is synchronized by SNTP
    time_t m_nLastSave; // in seconds, the time every time the secure clock state is saved

private:
    static Mutex mLock;
};

} // namespace android

#endif /* SECURETIMER_H_ */

