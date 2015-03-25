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

// for INT64_MAX 
#undef __STRICT_ANSI__
#define __STDINT_LIMITS
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "MtkRTSPController.h"

#include "MyHandler.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/DataSource.h>

namespace android {

MtkRTSPController::MtkRTSPController(const sp<ALooper> &looper)
    : mState(DISCONNECTED),
      mMetaData(new MetaData),
      mEnableSendPause(true), //haizhen
      m_playStatus(INIT),   //haizhen
      mLooper(looper),
      mUIDValid(false),
      mSeekDoneCb(NULL),
      mSeekDoneCookie(NULL),
      mPauseDoneCb(NULL),
      mPauseDoneCookie(NULL),
      mPlayDoneCb(NULL),
      mPlayDoneCookie(NULL),
      mLastSeekCompletedTimeUs(-1) {
    mReflector = new AHandlerReflector<MtkRTSPController>(this);
    looper->registerHandler(mReflector);
	ALOGI("====================================="); 
    ALOGI("[RTSP Playback capability info]："); 
    ALOGI("====================================="); 
}

MtkRTSPController::~MtkRTSPController() {
    CHECK_EQ((int)mState, (int)DISCONNECTED);
    mLooper->unregisterHandler(mReflector->id());
}

void MtkRTSPController::setUID(uid_t uid) {
    mUIDValid = true;
    mUID = uid;
}

status_t MtkRTSPController::connect(const char *url) {
    return connect(url, NULL);
}

status_t MtkRTSPController::connect(const char *url, const KeyedVector<String8, String8> *headers) {
    return connect(url, headers, NULL);
}

status_t MtkRTSPController::connect(const char *url,
        const KeyedVector<String8, String8> *headers,
        sp<ASessionDescription> desc) {
    Mutex::Autolock autoLock(mLock);

    if (mState != DISCONNECTED) {
        return ERROR_ALREADY_CONNECTED;
    }

    sp<AMessage> notify = new AMessage(kWhatNotify, mReflector->id());
    notify->setInt32("result", UNKNOWN_ERROR);

    mHandler = new MyHandler(url, notify, mUIDValid, mUID);
    CHECK(mHandler != NULL);
    mHandler->setLegacyMode(true);

    if (headers != NULL) {
        mHandler->parseHeaders(headers);
    }

    if (desc != NULL) {
        status_t err = mHandler->setSessionDesc(desc);
        if (err != OK)
            return err;
    } 
    mLooper->registerHandler(mHandler);

    mState = CONNECTING;

    mHandler->connect();

    while (mState == CONNECTING) {
        mCondition.wait(mLock);
    }

    if (mState != CONNECTED) {
        mHandler.clear();
    }

    if (mConnectionResult == OK) {
        mMetaData->setInt32(kKeyServerTimeout, mHandler->getServerTimeout());

        AString val;
        sp<ASessionDescription> desc = mHandler->getSessionDesc();
        if (desc->findAttribute(0, "s=", &val)) {
            ALOGI("rtsp s=%s ", val.c_str());
            mMetaData->setCString(kKeyTitle, val.c_str());
        }

        if (desc->findAttribute(0, "i=", &val)) {
            ALOGI("rtsp i=%s ", val.c_str());
            mMetaData->setCString(kKeyAuthor, val.c_str());
        }
    }
    return mConnectionResult;
}

void MtkRTSPController::disconnect() {
    Mutex::Autolock autoLock(mLock);

    if (mState == CONNECTING) {
        mState = DISCONNECTED;
        mConnectionResult = ERROR_IO;
        mCondition.broadcast();

        mHandler.clear();
        return;
    } else if (mState != CONNECTED) {
        return;
    }

    mHandler->disconnect();

    while (mState == CONNECTED) {
        mCondition.wait(mLock);
    }

    mHandler.clear();
}

status_t MtkRTSPController::sendPause() {
    ALOGI("[rtsp]MtkRTSPController::sendPause!!! enable=%d m_playStatus=%d", mEnableSendPause, m_playStatus);
    Mutex::Autolock autoLock(mLock);
    if(mEnableSendPause && (m_playStatus != STOPPED) && (m_playStatus != PAUSED)) {
        prepareSyncCall();
        mHandler->pause();
        int32_t pauseDoneRes;
        pauseDoneRes = finishSyncCall();
        if(pauseDoneRes == OK){
            m_playStatus = PAUSED;
            ALOGI("[rtsp]Send Pause Ok");
        } else if(pauseDoneRes == ALREADY_EXISTS){
            ALOGE("[rtsp]Send Pause too frequently\n");

        } else if(pauseDoneRes == INVALID_OPERATION){
            ALOGE("[rtsp]Pause is not valid!!!\n");
        } else {
            ALOGE("[rtsp]Server return fail for Pause, will abort!!!\n");
        }
        return pauseDoneRes;
    }
    return OK;   
}

// by mtk80902 - async pause
status_t MtkRTSPController::sendPause(void (*pauseDoneCb)(void *, status_t), void *cookie) {
    CHECK(pauseDoneCb != NULL);
    ALOGI("[rtsp]MtkRTSPController::send async Pause!!! m_playStatus=%d", m_playStatus);
    Mutex::Autolock autoLock(mLock);

    if (m_playStatus == STOPPED || m_playStatus == PAUSED || mPlayDoneCb)
        (*pauseDoneCb)(cookie, INVALID_OPERATION);
    else if (mPauseDoneCb)
        (*pauseDoneCb)(cookie, ALREADY_EXISTS);
    else {
        mPauseDoneCb = pauseDoneCb;
        mPauseDoneCookie = cookie;
        mHandler->pause();
    }
    return OK;   
}

status_t MtkRTSPController::sendPlay() {
    ALOGI("[rtsp]MtkRTSPController::sendPlay!!! m_playStatus=%d", m_playStatus);
    Mutex::Autolock autoLock(mLock);
    if(m_playStatus != PLAYING) {   //haizhen
        status_t playRes;
        prepareSyncCall();
        if(m_playStatus == PAUSED){
            //mHandler->play(true);
            mHandler->resume();
        } else if (m_playStatus == STOPPED){
            ALOGE("[rtsp]SendPlay after stopped!!!");
            //mHandler->play();
            mHandler->resume();
        } else {
            //mHandler->play();
            mHandler->resume();
        }
        playRes = finishSyncCall();
        if(playRes == OK){
            m_playStatus = PLAYING;
        }
        return playRes;
    }
    return OK;
}

// by mtk80902 - async play
status_t MtkRTSPController::sendPlay(void (*playDoneCb)(void *, status_t), void *cookie) {
    CHECK(playDoneCb != NULL);
    ALOGI("[rtsp]MtkRTSPController::send async Play!!! m_playStatus=%d", m_playStatus);
    Mutex::Autolock autoLock(mLock);

    if (mPauseDoneCb || m_playStatus == PLAYING)
        (*playDoneCb)(cookie, INVALID_OPERATION);
    else if (mPlayDoneCb)
        (*playDoneCb)(cookie, ALREADY_EXISTS);
    else {
        mPlayDoneCb = playDoneCb;
        mPlayDoneCookie = cookie;
        if(m_playStatus == PAUSED){
            //mHandler->play(true);
            mHandler->resume();
        } else if (m_playStatus == STOPPED){
            ALOGE("[rtsp]SendPlay after stopped!!!");
            //mHandler->play();
            mHandler->resume();
        } else {
            //mHandler->play();
            mHandler->resume();
        }
    }
    return OK;
}

status_t MtkRTSPController::preSeek(
        int64_t timeUs,
        void (*seekDoneCb)(void *), void *cookie) {
    Mutex::Autolock autoLock(mLock);
    bool tooEarly =
        mLastSeekCompletedTimeUs >= 0
        && ALooper::GetNowUs() < mLastSeekCompletedTimeUs + 500000ll;
#ifdef MTK_BSP_PACKAGE
    //cancel  ignore seek --do every seek for bsp package
    // because ignore seek and notify seek complete will cause progress return back
    tooEarly = false;
#endif
    bool needCallback = mState != CONNECTED || tooEarly;
    status_t err = ALREADY_EXISTS;
    if (!needCallback) {
        prepareSyncCall();
        mHandler->preSeek(timeUs);
        err = finishSyncCall();
        ALOGI("ARTSPController::preSeek end err=%d",err);  
    }
    if (needCallback || err == INVALID_OPERATION) {
        ALOGW("not do seek really, needCallback=%d,err=%d",needCallback,err);
        (*seekDoneCb)(cookie);
    }
    return err;
}

void MtkRTSPController::prepareSyncCall() {
    mSyncCallResult = OK;
    mSyncCallDone = false;
}

status_t MtkRTSPController::finishSyncCall() {
    while(mSyncCallDone == false) {
        mCondition.wait(mLock);
    }
    return mSyncCallResult;
}

void MtkRTSPController::completeSyncCall(const sp<AMessage>& msg) {
    Mutex::Autolock autoLock(mLock);
    if (!msg->findInt32("result", &mSyncCallResult)) {
        ALOGW("no result found in completeSyncCall");
        mSyncCallResult = OK;
    }
    mSyncCallDone = true;
    mCondition.signal();
}

void MtkRTSPController::seekAsync(
        int64_t timeUs,
        void (*seekDoneCb)(void *), void *cookie) {
    Mutex::Autolock autoLock(mLock);

    CHECK(seekDoneCb != NULL);
    CHECK(mSeekDoneCb == NULL);

    // Ignore seek requests that are too soon after the previous one has
    // completed, we don't want to swamp the server.

    mSeekDoneCb = seekDoneCb;
    mSeekDoneCookie = cookie;

    mHandler->seek(timeUs);
	m_playStatus = PLAYING;
}

size_t MtkRTSPController::countTracks() {
    if (mHandler == NULL) {
        return 0;
    }

    return mHandler->countTracks();
}

sp<MediaSource> MtkRTSPController::getTrack(size_t index) {
    CHECK(mHandler != NULL);

    return mHandler->getPacketSource(index);
}

sp<MetaData> MtkRTSPController::getTrackMetaData(
        size_t index, uint32_t flags) {
    CHECK(mHandler != NULL);

    return mHandler->getPacketSource(index)->getFormat();
}

void MtkRTSPController::onMessageReceived(const sp<AMessage> &msg) {
    if (msg->what() == kWhatNotify) {
        int32_t what;
        CHECK(msg->findInt32("what", &what));
        switch (what) {
            case MyHandler::kWhatConnected:
            {
                Mutex::Autolock autoLock(mLock);

                CHECK(msg->findInt32("result", &mConnectionResult));
                int v;
                if (msg->findInt32("unsupport-video", &v) && v) {
                    mMetaData->setInt32(kKeyHasUnsupportVideo, true);
                }
                mState = (mConnectionResult == OK) ? CONNECTED : DISCONNECTED;

                mCondition.signal();
                break;
            }

            case MyHandler::kWhatDisconnected:
            {
                Mutex::Autolock autoLock(mLock);
                msg->findInt32("result", &mConnectionResult);
                mState = DISCONNECTED;
                mCondition.signal();
                break;
            }

        //  case MyHandler::kWhatPlayDone:
        //  case MyHandler::kWhatPauseDone:
            case MyHandler::kWhatPreSeekDone:
            {
                completeSyncCall(msg);
                break;
            }
            case MyHandler::kWhatPlayDone:
            {
                if (mPlayDoneCb)	// by mtk80902 - async play
                {
                    Mutex::Autolock autoLock(mLock);
                    status_t playDoneRes = OK;
                    if (!msg->findInt32("result", &playDoneRes)) 
                    {
                        ALOGW("no result found in async play");
                        break;
                    }

                    (*mPlayDoneCb)(mPlayDoneCookie, playDoneRes);
                    mPlayDoneCb = NULL;
                    m_playStatus = PLAYING;
                }
                else
                    completeSyncCall(msg);
                break;
            }
            case MyHandler::kWhatPauseDone:
            {
                if (mPauseDoneCb)	// by mtk80902 - async pause
                {
                    Mutex::Autolock autoLock(mLock);
                    status_t pauseDoneRes = OK;
                    if (!msg->findInt32("result", &pauseDoneRes)) 
                    {
                        ALOGW("no result found in async pause");
                        break;
                    }

                    (*mPauseDoneCb)(mPauseDoneCookie, pauseDoneRes);
                    mPauseDoneCb = NULL;

                    if(pauseDoneRes == OK){
                        m_playStatus = PAUSED;
                        ALOGI("[rtsp]Send Pause Ok");
                    } else if(pauseDoneRes == ALREADY_EXISTS){
                        ALOGE("[rtsp]Send Pause too frequently\n");
                    } else if(pauseDoneRes == INVALID_OPERATION){
                        ALOGE("[rtsp]Pause is not valid!!!\n");
                    } else {
                        ALOGE("[rtsp]Server return fail for Pause, will abort!!!\n");
                    }
                }
                else
                    completeSyncCall(msg);
                break;
            }

            case MyHandler::kWhatSeekDone:
            {
                ALOGI("seek done");

                mLastSeekCompletedTimeUs = ALooper::GetNowUs();

                void (*seekDoneCb)(void *) = mSeekDoneCb;
                mSeekDoneCb = NULL;

                (*seekDoneCb)(mSeekDoneCookie);
                break;
            }

            default:
                ALOGI("ignore message %d", what);
            break;
        }
    } else {
        ALOGW("unknown message %d", msg->what());
    }
}

int64_t MtkRTSPController::getQueueDurationUs(bool *eos) {
    *eos = true;

    int64_t minQueuedDurationUs = INT64_MAX;
    for (size_t i = 0; i < mHandler->countTracks(); ++i) {
        sp<APacketSource> source = mHandler->getPacketSource(i);

        bool newEOS;
        int64_t queuedDurationUs = source->getQueueDurationUs(&newEOS);

        if (!newEOS) {
            *eos = false;
        }

        // don't let the EOS stream block buffering
        if (!newEOS && queuedDurationUs < minQueuedDurationUs) {
            minQueuedDurationUs = queuedDurationUs;
        }
    }

    return minQueuedDurationUs;
}

sp<MetaData> MtkRTSPController::getMetaData() {
    return mMetaData;
}

uint32_t MtkRTSPController::flags() const {
    int64_t durationUs;
    if (mHandler->getSessionDesc()->getDurationUs(&durationUs))
        return CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_SEEK | CAN_PAUSE;
    else
        return 0;
}

void MtkRTSPController::stopRequests() {
    Mutex::Autolock autoLock(mLock);
    if (mHandler == NULL)
        return;

    if (mState == CONNECTING) {
        mState = DISCONNECTED;
        mConnectionResult = FAILED_TRANSACTION;
        mHandler->exit();
    } else {
        mSyncCallDone = true;
        mSyncCallResult = FAILED_TRANSACTION;
        // mtk80902: notify async play/pause
        if (mPauseDoneCb)
        {
            (*mPauseDoneCb)(mPauseDoneCookie, mSyncCallResult);
            mPauseDoneCb = NULL;
        }
        if (mPlayDoneCb)
        {
            (*mPlayDoneCb)(mPlayDoneCookie, mSyncCallResult);
            mPlayDoneCb = NULL;
        }
    }
    mCondition.signal();
}

void MtkRTSPController::stop() {
    if (mHandler == NULL)
        return;
    for (size_t i = 0; i < mHandler->countTracks(); ++i) {
        sp<APacketSource> source = mHandler->getPacketSource(i);
        source->flushQueue();
        source->signalEOS(ERROR_END_OF_STREAM);
    }
    mHandler->stopTCPTrying();
	m_playStatus = STOPPED;
}

void MtkRTSPController::externalStop()	// mtk80902: ALPS00383197
{
    Mutex::Autolock autoLock(mLock);
    stop();
}

MtkSDPExtractor::MtkSDPExtractor(const sp<DataSource> &source)
    :mMetaData(new MetaData), mSessionDesc(new ASessionDescription)
{
    off64_t fileSize;
    if (source->getSize(&fileSize) != OK) {
        fileSize = 4096 * 2;
        ALOGW("no lenth of SDP, try max of %lld", fileSize);
    }

    void* data = malloc(fileSize);
    if (data != NULL) {
        ssize_t n = source->readAt(0, data, fileSize);
        if (n > 0) {
            if (n != fileSize) {
                ALOGW("data read may be incomplete %d vs %lld", (int)n, fileSize);
            }
            mSessionDesc->setTo(data, n);
        }
        free(data);
    } else {
        ALOGW("out of memory in MtkSDPExtractor");
    }

    mMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_APPLICATION_SDP);
    mMetaData->setPointer(kKeySDP, mSessionDesc.get());
}

size_t MtkSDPExtractor::countTracks() {
    return 0;
}

sp<MediaSource> MtkSDPExtractor::getTrack(size_t index) {
    return NULL;
}

sp<MetaData> MtkSDPExtractor::getTrackMetaData(size_t index, uint32_t flags) {
    return NULL;
}

sp<MetaData> MtkSDPExtractor::getMetaData() {
    return mMetaData;
}

bool SniffSDP(
        const sp<DataSource> &source, String8 *mimeType, float *confidence,
        sp<AMessage> *meta) {
    const int testLen = 7;
    uint8_t line[testLen];
    ssize_t n = source->readAt(0, line, testLen);
    if (n < testLen)
        return false;

    const char* nline = "v=0\no=";
    const char* rnline = "v=0\r\no=";

    if (!memcmp(line, nline, sizeof(nline) - 1) ||
            !memcmp(line, rnline, sizeof(rnline) - 1)) {
        *mimeType = MEDIA_MIMETYPE_APPLICATION_SDP;
        *confidence = 0.5;
        return true;
    }

    return false;
}

}  // namespace android
