/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaPuller"
#include <utils/Log.h>
#ifdef MTB_SUPPORT
#define ATRACE_TAG ATRACE_TAG_WFD
#include <utils/Trace.h>
#endif

#include "MediaPuller.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>

#ifndef ANDROID_DEFAULT_CODE

#include "DataPathTrace.h"
#endif

namespace android {

MediaPuller::MediaPuller(
        const sp<MediaSource> &source, const sp<AMessage> &notify)
    : mSource(source),
      mNotify(notify),
      mPullGeneration(0),
      mIsAudio(false),
      mPaused(false) {
    sp<MetaData> meta = source->getFormat();
    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));

    mIsAudio = !strncasecmp(mime, "audio/", 6);

#ifndef ANDROID_DEFAULT_CODE	
      mFirstDeltaMs = -1;
#endif
	
}

MediaPuller::~MediaPuller() {
}

status_t MediaPuller::postSynchronouslyAndReturnError(
        const sp<AMessage> &msg) {
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if (err != OK) {
        return err;
    }

    if (!response->findInt32("err", &err)) {
        err = OK;
    }

    return err;
}

status_t MediaPuller::start() {
    return postSynchronouslyAndReturnError(new AMessage(kWhatStart, id()));
}

void MediaPuller::stopAsync(const sp<AMessage> &notify) {
    sp<AMessage> msg = new AMessage(kWhatStop, id());
    msg->setMessage("notify", notify);
    msg->post();
}

void MediaPuller::pause() {
    (new AMessage(kWhatPause, id()))->post();
}

void MediaPuller::resume() {
    (new AMessage(kWhatResume, id()))->post();
}

void MediaPuller::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatStart:
        {
            status_t err;
            ALOGI("start mIsAudio=%d",mIsAudio);
            if (mIsAudio) {
                // This atrocity causes AudioSource to deliver absolute
                // systemTime() based timestamps (off by 1 us).
#ifdef MTB_SUPPORT                
                ATRACE_BEGIN("AudioPuller, kWhatStart");
#endif
                sp<MetaData> params = new MetaData;
                params->setInt64(kKeyTime, 1ll);
                err = mSource->start(params.get());
            } else {
#ifdef MTB_SUPPORT            
                ATRACE_BEGIN("VideoPuller, kWhatStart");
#endif
                err = mSource->start();
                if (err != OK) {
                    ALOGE("source failed to start w/ err %d", err);
                }
            }

            if (err == OK) {
							 ALOGI("start done, start to schedulePull data");
                schedulePull();
            }

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);

            uint32_t replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            response->postReply(replyID);
#ifdef MTB_SUPPORT			
            ATRACE_END();
#endif
            break;
        }

        case kWhatStop:
        {
            sp<MetaData> meta = mSource->getFormat();
            const char *tmp;
            CHECK(meta->findCString(kKeyMIMEType, &tmp));
            AString mime = tmp;

            ALOGI("MediaPuller(%s) stopping.", mime.c_str());
            mSource->stop();
            ALOGI("MediaPuller(%s) stopped.", mime.c_str());
            ++mPullGeneration;

            sp<AMessage> notify;
            CHECK(msg->findMessage("notify", &notify));
            notify->post();
            break;
        }

        case kWhatPull:
        {
            int32_t generation;
#ifdef MTB_SUPPORT			
            if (mIsAudio) {
                ATRACE_BEGIN("AudioPuller, kWhatPull");
            } else {
                ATRACE_BEGIN("VideoPuller, kWhatPull");
            }
#endif			
            CHECK(msg->findInt32("generation", &generation));

            if (generation != mPullGeneration) {
                break;
            }

            MediaBuffer *mbuf;
	 
            status_t err = mSource->read(&mbuf);

            if (mPaused) {
                if (err == OK) {
                    mbuf->release();
                    mbuf = NULL;
                }

                schedulePull();
                break;
            }

            if (err != OK) {
                if (err == ERROR_END_OF_STREAM) {
                    ALOGI("stream ended.");
                } else {
                    ALOGE("error %d reading stream.", err);
                }
                ALOGI("err=%d.post kWhatEOS",err);
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what", kWhatEOS);
                notify->post();
            } else {
            
                int64_t timeUs;
                CHECK(mbuf->meta_data()->findInt64(kKeyTime, &timeUs));
#ifdef MTB_SUPPORT
                if (mIsAudio) {
                    ATRACE_ONESHOT(ATRACE_ONESHOT_ADATA, "AudioPuller, TS: %lld ms", timeUs/1000);
                }
                else {
                    ATRACE_ONESHOT(ATRACE_ONESHOT_VDATA, "VideoPuller, TS: %lld ms", timeUs/1000);
                }
#endif				
                sp<ABuffer> accessUnit = new ABuffer(mbuf->range_length());

                memcpy(accessUnit->data(),
                       (const uint8_t *)mbuf->data() + mbuf->range_offset(),
                       mbuf->range_length());

                accessUnit->meta()->setInt64("timeUs", timeUs);

#ifndef ANDROID_DEFAULT_CODE	
		sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
	   	 int64_t MpMs = ALooper::GetNowUs();
		 debugInfo->addTimeInfoByKey(!mIsAudio , timeUs, "MpIn", MpMs/1000);	
		 
		 int64_t NowMpDelta =0;
		
		 NowMpDelta = (MpMs - timeUs)/1000;	
		 
		 if(mFirstDeltaMs == -1){
			mFirstDeltaMs = NowMpDelta;
			ALOGE("[check Input 1th][%s] ,timestamp=%lld ms,[ts and now delta change]=%lld ms",
			 		mIsAudio?"audio":"video",timeUs/1000,NowMpDelta);
		 }	
		 NowMpDelta = NowMpDelta - mFirstDeltaMs;
		
		 if(NowMpDelta > 500ll || NowMpDelta < -500ll ){
			 ALOGE("[check Input][%s] ,timestamp=%lld ms,[ts and now delta change]=%lld ms",
			 		mIsAudio?"audio":"video",timeUs/1000,NowMpDelta);
		 }
		 
#endif

		 
                if (mIsAudio) {
                    mbuf->release();
                    mbuf = NULL;
		      ALOGI("[WFDP][%s] ,timestamp=%lld ms",mIsAudio?"audio":"video",timeUs/1000);
                } else {
                    // video encoder will release MediaBuffer when done
                    // with underlying data.
                    accessUnit->meta()->setPointer("mediaBuffer", mbuf);
		      ALOGI("[WFDP][%s] ,mediaBuffer=%p,timestamp=%lld ms",mIsAudio?"audio":"video",mbuf,timeUs/1000);
                }

                sp<AMessage> notify = mNotify->dup();

                notify->setInt32("what", kWhatAccessUnit);
                notify->setBuffer("accessUnit", accessUnit);
                notify->post();

                if (mbuf != NULL) {
                    ALOGV("posted mbuf %p", mbuf);
                }

                schedulePull();
#ifdef MTB_SUPPORT				
                ATRACE_END();
#endif
            }
            break;
        }

        case kWhatPause:
        {
            mPaused = true;
            break;
        }

        case kWhatResume:
        {
            mPaused = false;
            break;
        }
        default:
            TRESPASS();
    }
}

void MediaPuller::schedulePull() {
    sp<AMessage> msg = new AMessage(kWhatPull, id());
    msg->setInt32("generation", mPullGeneration);
    msg->post();
}

}  // namespace android

