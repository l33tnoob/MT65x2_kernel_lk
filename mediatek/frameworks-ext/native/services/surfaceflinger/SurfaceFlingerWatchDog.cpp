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

/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/xlog.h>
#include <cutils/properties.h>

#include "SurfaceFlingerWatchDog.h"

namespace android {

ANDROID_SINGLETON_STATIC_INSTANCE(SFWatchDog);

#define SW_WATCHDOG_TIMER       1000
#define SW_WATCHDOG_THRESHOLD   500
#define SW_WATCHDOG_RTTCOUNT    10
#define RTT_FOLDER_PATH         "/data/anr/SF_RTT/"
#define RTT_FILE_NAME           "rtt_dump"
#define RTT_DUMP                (RTT_FOLDER_PATH RTT_FILE_NAME)

Mutex SFWatchDog::sLock(Mutex::PRIVATE);
sp<SFWatchDog> SFWatchDog::sInstance;

SFWatchDog::SFWatchDog() : Thread(false) {
    XLOGI("[%s]", __func__);
}

SFWatchDog::~SFWatchDog() {
    XLOGI("[%s]", __func__);

    mNodeList.clear();
}

void SFWatchDog::onFirstRef() {
    XLOGI("[%s]", __func__);
    run("SFWatchDog", PRIORITY_BACKGROUND);
}

status_t SFWatchDog::readyToRun() {
    XLOGI("[%s]", __func__);

    mThreshold = SW_WATCHDOG_THRESHOLD;
    mTimer = SW_WATCHDOG_TIMER;
    mShowLog = false;

    char cmds[256];

    // create rtt folder
    sprintf(cmds, "mkdir %s", RTT_FOLDER_PATH);
    system(cmds);

    return NO_ERROR;
}

sp<SFWatchDog> SFWatchDog::getInstance() {
    Mutex::Autolock _l(sLock);
    static sp<SFWatchDog> instance = sInstance;
    if (instance.get() == 0) {
        instance = new SFWatchDog();
        sInstance = instance;
    }
    return instance;
}

bool SFWatchDog::threadLoop() {
    XLOGV("[%s]", __func__);

    {
        Mutex::Autolock _l(mScreenLock);
    }

    nsecs_t ct = 0;
    nsecs_t stopTime = 1;
    if (isSFThreadHang(ct)) {
        char cmds[256];

        XLOGW("[SF-WD] ============================================");
        XLOGW("[SF-WD] detect SF maybe hang, state: ");

        for (uint32_t i = 0; i < mNodeList.size(); i++)
        {
            if (mNodeList[i]->mStartTransactionTime == 0) {
                XLOGI("    [%s] wait event", mNodeList[i]->mName.string());
            } else {
                stopTime = ns2ms(ct - mNodeList[i]->mStartTransactionTime);

                XLOGW("    [%s] stop = %lld ms", mNodeList[i]->mName.string(), stopTime);
            }
        }

        static uint32_t rtt_ct = SW_WATCHDOG_RTTCOUNT;

        if (rtt_ct > 0) {
            rtt_ct --;
        } else {
            XLOGD("[SF-WD] swap rtt dump file");

            // swap rtt dump file
            sprintf(cmds, "mv %s.txt %s_1.txt", RTT_DUMP, RTT_DUMP);
            system(cmds);

            rtt_ct = SW_WATCHDOG_RTTCOUNT;
        }

        // append SurfaceFlinger rtt information to rtt file
        sprintf(cmds, "rtt -f bt -p %d >> %s.txt", getpid(), RTT_DUMP);
        system(cmds);

        XLOGD("[SF-WD] dump rtt file: %s.txt", RTT_DUMP);

        XLOGW("[SF-WD] ============================================");
    }

    getProperty();
    usleep(mTimer * 1000);

    char value[PROPERTY_VALUE_MAX];
    sprintf(value, "%lld", stopTime);
    uint32_t ret = property_set("service.sf.status", value);

    return true;
}

uint32_t SFWatchDog::registerNodeName(const char* name) {
    uint32_t index = 0;

    for (uint32_t i = 0; i < mNodeList.size(); i++) {
        if (name == mNodeList[i]->mName) {
            XLOGI("register an already registered name: %s (%s)", name, mNodeList[i]->mName.string());
            return i;
        }
    }

    index = mNodeList.size();
    sp<NodeElement> node = new NodeElement();
    node->mName = name;
    node->mStartTransactionTime = 0;
    mNodeList.add(node);

    XLOGI("[%s] name=%s, index=%d", __func__, name, index);

    return index;
}

bool SFWatchDog::isSFThreadHang(nsecs_t& ct) {
    Mutex::Autolock _l(mLock);

    const nsecs_t now = systemTime();
    if (mShowLog) {
        XLOGI("[SF-WD], last transaction: %lld, now: %lld", mNodeList[0]->mStartTransactionTime, now);
    }

    for (uint32_t i = 0; i < mNodeList.size(); i++) {
        if (mNodeList[i]->mStartTransactionTime != 0 &&
            mThreshold * 1000 * 1000 < now - mNodeList[i]->mStartTransactionTime) {
            ct = now;
            return true;
        }
    }

    return false;
}

void SFWatchDog::markStartTransactionTime(uint32_t index) {
    Mutex::Autolock _l(mLock);

    if (index >= mNodeList.size()) {
        XLOGE("[unmarkStartTransactionTime] index=%d > Node list size=%d", index, mNodeList.size());
        return;
    }

    mNodeList[index]->mStartTransactionTime = systemTime();

    XLOGV("[%s] name=%s, index=%d, time = %lld", __func__, mNodeList[index]->mName.string(), index, mNodeList[index]->mStartTransactionTime);
}

void SFWatchDog::unmarkStartTransactionTime(uint32_t index) {
    Mutex::Autolock _l(mLock);

    if (index >= mNodeList.size()) {
        XLOGE("[unmarkStartTransactionTime] index=%d > Node list size=%d", index, mNodeList.size());
        return;
    }

    mNodeList[index]->mStartTransactionTime = 0;

    XLOGV("[%s] name=%s, index=%d, time = %lld", __func__, mNodeList[index]->mName.string(), index, mNodeList[index]->mStartTransactionTime);
}

void SFWatchDog::screenReleased() {
    mScreenLock.lock();

    if (mShowLog)
        XLOGD("[SF-WD] screen give-up");
}

void SFWatchDog::screenAcquired() {
    mScreenLock.unlock();

    if (mShowLog)
        XLOGD("[SF-WD] about to return");
}

void SFWatchDog::getProperty() {
    char value[PROPERTY_VALUE_MAX];

    property_get("debug.sf.wdthreshold", value, "0");
    int threshold = atoi(value);
    if (threshold != 0 && threshold != (int)mThreshold) {
        XLOGD("SF watch dog change threshold from %d --> %d", mThreshold, threshold);
        mThreshold = threshold;
    }

    property_get("debug.sf.wdtimer", value, "0");
    int timer = atoi(value);
    if (timer != 0 && timer != (int)mTimer) {
        XLOGD("SF watch dog change timer from %d --> %d", mTimer, timer);
        mTimer = timer;
    }

    property_get("debug.sf.wdlog", value, "0");
    mShowLog = atoi(value);
}

}; // namespace android
