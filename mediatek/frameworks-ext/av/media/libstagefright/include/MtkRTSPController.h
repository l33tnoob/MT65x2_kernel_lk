/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef A_RTSP_CONTROLLER_H_

#define A_RTSP_CONTROLLER_H_

#include <utils/String8.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/MediaExtractor.h>

namespace android {

struct ALooper;
struct MyHandler;
struct ASessionDescription;

struct MtkRTSPController : public MediaExtractor {
    MtkRTSPController(const sp<ALooper> &looper);

    void setUID(uid_t uid);

    status_t connect(const char *url);
    status_t connect(const char *url, const KeyedVector<String8, String8> *headers);
    status_t connect(const char *url,
            const KeyedVector<String8, String8> *headers, sp<ASessionDescription> desc);
    void disconnect();

    status_t sendPlay();
    status_t sendPause(); //haizhen
    status_t sendPlay(void (*playDoneCb)(void *, status_t), void *cookie);// mtk80902: async play
    status_t sendPause(void (*pauseDoneCb)(void *, status_t), void *cookie); // mtk80902: async pause
    // preSeek must be called before seekAsync
    status_t preSeek(int64_t timeUs, void (*seekDoneCb)(void *), void *cookie);
    void stopRequests();
    void stop();
    void externalStop();	// mtk80902: ALPS00383197
    void seekAsync(int64_t timeUs, void (*seekDoneCb)(void *), void *cookie);

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);

    virtual sp<MetaData> getTrackMetaData(
            size_t index, uint32_t flags);

    virtual sp<MetaData> getMetaData();
    int64_t getQueueDurationUs(bool *eos);

    void onMessageReceived(const sp<AMessage> &msg);

    virtual uint32_t flags() const;

protected:
    virtual ~MtkRTSPController();

private:
    enum {
        kWhatConnectDone    = 'cdon',
        kWhatDisconnectDone = 'ddon',
        kWhatSeekDone       = 'sdon',
        kWhatSyncCallDone   = 'ndon',
        kWhatNotify         = 'noti',
        kWhatDisconnect     = 'disc',
        kWhatPerformSeek    = 'seek',
    };

    enum State {
        DISCONNECTED,
        CONNECTED,
        CONNECTING,
    };

   enum PlayStatus{
	INIT = 0,
	PAUSED,
	PLAYING,
	STOPPED,		
    };

    Mutex mLock;
    Condition mCondition;
    status_t mSyncCallResult;
    bool mSyncCallDone;
    status_t finishSyncCall();
    void prepareSyncCall();
    void completeSyncCall(const sp<AMessage>& msg);

    State mState;
    sp<MetaData> mMetaData;
	bool mEnableSendPause; //haizhen
	PlayStatus m_playStatus; //haizhen
    status_t mConnectionResult;

    sp<ALooper> mLooper;
    sp<MyHandler> mHandler;
    sp<AHandlerReflector<MtkRTSPController> > mReflector;

    bool mUIDValid;
    uid_t mUID;

    void (*mSeekDoneCb)(void *);
    void *mSeekDoneCookie;
    void (*mPauseDoneCb)(void *, status_t);	// mtk80902
    void *mPauseDoneCookie;
    void (*mPlayDoneCb)(void *, status_t);	// mtk80902
    void *mPlayDoneCookie;
    int64_t mLastSeekCompletedTimeUs;

    DISALLOW_EVIL_CONSTRUCTORS(MtkRTSPController);
};

class MtkSDPExtractor : public MediaExtractor {
    public:
        MtkSDPExtractor(const sp<DataSource> &source);
        virtual size_t countTracks();
        virtual sp<MediaSource> getTrack(size_t index);
        virtual sp<MetaData> getTrackMetaData(
                size_t index, uint32_t flags = 0);
        virtual sp<MetaData> getMetaData();
    private:
        sp<MetaData> mMetaData;
        sp<ASessionDescription> mSessionDesc;
    DISALLOW_EVIL_CONSTRUCTORS(MtkSDPExtractor);
};

bool SniffSDP(
        const sp<DataSource> &source, String8 *mimeType, float *confidence,
        sp<AMessage> *meta);
}  // namespace android

#endif  // A_RTSP_CONTROLLER_H_
