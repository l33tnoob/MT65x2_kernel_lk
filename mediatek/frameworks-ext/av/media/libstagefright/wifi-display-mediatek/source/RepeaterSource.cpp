//#define LOG_NDEBUG 0
#define LOG_TAG "RepeaterSource"
#include <utils/Log.h>
#ifdef MTB_SUPPORT
#define ATRACE_TAG ATRACE_TAG_WFD
#include <utils/Trace.h>
#endif
#include "RepeaterSource.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MetaData.h>

#ifndef ANDROID_DEFAULT_CODE

#include <cutils/properties.h>
#include "DataPathTrace.h"
#endif

namespace android {

RepeaterSource::RepeaterSource(const sp<MediaSource> &source, double rateHz)
    : mStarted(false),
      mSource(source),
      mRateHz(rateHz),
      mBuffer(NULL),
      mResult(OK),
      mLastBufferUpdateUs(-1ll),
      mStartTimeUs(-1ll),
      mFrameCount(0) {
}

RepeaterSource::~RepeaterSource() {
    CHECK(!mStarted);
}

double RepeaterSource::getFrameRate() const {
    return mRateHz;
}

void RepeaterSource::setFrameRate(double rateHz) {
    Mutex::Autolock autoLock(mLock);

    if (rateHz == mRateHz) {
        return;
    }

    if (mStartTimeUs >= 0ll) {
        int64_t nextTimeUs = mStartTimeUs + (mFrameCount * 1000000ll) / mRateHz;
        mStartTimeUs = nextTimeUs;
        mFrameCount = 0;
    }
    mRateHz = rateHz;
}

status_t RepeaterSource::start(MetaData *params) {
    CHECK(!mStarted);
#ifdef MTB_SUPPORT	
    ATRACE_CALL();
#endif
    ALOGI("RepeaterSource start");

    status_t err = mSource->start(params);

    if (err != OK) {
    		ALOGI("surfaceMediaSource start err");
        return err;
    }

    mBuffer = NULL;
    mResult = OK;
    mStartTimeUs = -1ll;
    mFrameCount = 0;

    mLooper = new ALooper;
    mLooper->setName("repeater_looper");
    mLooper->start();

    mReflector = new AHandlerReflector<RepeaterSource>(this);
    mLooper->registerHandler(mReflector);

    postRead();

    mStarted = true;
		ALOGI("RepeaterSource start done");
    return OK;
}

status_t RepeaterSource::stop() {
    CHECK(mStarted);

    ALOGI("stopping");

    if (mLooper != NULL) {
        mLooper->stop();
        mLooper.clear();

        mReflector.clear();
    }

    if (mBuffer != NULL) {
        ALOGV("releasing mbuf %p", mBuffer);
        mBuffer->release();
        mBuffer = NULL;
    }

    status_t err = mSource->stop();

    ALOGI("stopped");

    mStarted = false;

    return err;
}

sp<MetaData> RepeaterSource::getFormat() {
    return mSource->getFormat();
}

status_t RepeaterSource::read(
        MediaBuffer **buffer, const ReadOptions *options) {
    int64_t seekTimeUs;
    ReadOptions::SeekMode seekMode;
    CHECK(options == NULL || !options->getSeekTo(&seekTimeUs, &seekMode));

 //   ALOGI("[video buffer]read+ mBuffer=0x%08x",(mBuffer==NULL)?0:mBuffer);
    for (;;) {
        int64_t bufferTimeUs = -1ll;

        if (mStartTimeUs < 0ll) {
            Mutex::Autolock autoLock(mLock);
            while ((mLastBufferUpdateUs < 0ll || mBuffer == NULL)
                    && mResult == OK) {
                mCondition.wait(mLock);
            }

            
            mStartTimeUs = ALooper::GetNowUs();
            bufferTimeUs = mStartTimeUs;
	     ALOGI("now resuming.mStartTimeUs=%lld ms",mStartTimeUs/1000);
        } else {
            bufferTimeUs = mStartTimeUs + (mFrameCount * 1000000ll) / mRateHz;

            int64_t nowUs = ALooper::GetNowUs();
            int64_t delayUs = bufferTimeUs - nowUs;

            if (delayUs > 0ll) {
                usleep(delayUs);
            }
        }

        bool stale = false;

        {
	   
            Mutex::Autolock autoLock(mLock);
            if (mResult != OK) {
                CHECK(mBuffer == NULL);
		  ALOGI("read return error %d",mResult);
                return mResult;
            }

#if SUSPEND_VIDEO_IF_IDLE
            int64_t nowUs = ALooper::GetNowUs();
            if (nowUs - mLastBufferUpdateUs > 1000000ll) {
                mLastBufferUpdateUs = -1ll;
                stale = true;
		  ALOGI("[video buffer] has not  been updated than >1S");
            } else
#endif
	     {
                mBuffer->add_ref();
                *buffer = mBuffer;
                (*buffer)->meta_data()->setInt64(kKeyTime, bufferTimeUs);
#ifdef MTB_SUPPORT    
                ATRACE_ONESHOT(ATRACE_ONESHOT_VDATA, "Repeater, TS: %lld ms", bufferTimeUs/1000);
#endif

#ifndef ANDROID_DEFAULT_CODE	
		
		int32_t usedTimes=0;
		if(mBuffer->meta_data()->findInt32('used', &usedTimes)){

			mBuffer->meta_data()->setInt32('used', usedTimes+1);
		}else{
	      		 mBuffer->meta_data()->setInt32('used', 1);
		}

		int64_t gotTime,delayTime;
		if( mBuffer->meta_data()->findInt64('RpIn', &gotTime) ){
			int64_t nowUs = ALooper::GetNowUs();
			if(usedTimes > 0) {
				delayTime = 0; 
				gotTime = nowUs/1000;
				ALOGV("[WFDP]this buffer has beed used for %d times",usedTimes);
			}else{
			      delayTime = (nowUs - gotTime*1000)/1000;
			}

			sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
			debugInfo->addTimeInfoByKey(1, bufferTimeUs, "RpIn", gotTime);
			debugInfo->addTimeInfoByKey(1, bufferTimeUs, "RpDisPlay", usedTimes);
			debugInfo->addTimeInfoByKey(1, bufferTimeUs, "DeMs", delayTime);
			debugInfo->addTimeInfoByKey(1, bufferTimeUs, "RpOt", nowUs/1000);
		}              
		   
#endif
		  ALOGV("[WFDP][video]read one video buffer  framecount = %d, bufferTimeUs = %lld ms", mFrameCount, bufferTimeUs / 1000);
                ++mFrameCount;
		  //workaround for encoder init slow
		  if(mFrameCount == 1)
		  {
		      mFrameCount = 6;
                    ALOGI("read deley 5frames times");
		  }
	         // ALOGI("[video buffer] mBuffer=%p, add ref ,refcount =%d",mBuffer,mBuffer->refcount());

		 
            }
        }

        if (!stale) {
            break;
        }

        mStartTimeUs = -1ll;
        mFrameCount = 0;
        ALOGI("now dormant");
    }

    return OK;
}

void RepeaterSource::postRead() {
    (new AMessage(kWhatRead, mReflector->id()))->post();
}

void RepeaterSource::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatRead:
        {
            MediaBuffer *buffer;
#ifdef MTB_SUPPORT			
            ATRACE_BEGIN("Repeater, KWhatRead");
#endif
#ifndef ANDROID_DEFAULT_CODE
	     int64_t startUs = ALooper::GetNowUs();
#endif
            status_t err = mSource->read(&buffer);
            

            Mutex::Autolock autoLock(mLock);//update mBuffer lock
            if (mBuffer != NULL) {
	           int32_t used=0;
#ifndef ANDROID_DEFAULT_CODE	
		  if(!mBuffer->meta_data()->findInt32('used', &used) ){
#ifdef MTB_SUPPORT
                ATRACE_ONESHOT(ATRACE_ONESHOT_SPECIAL, "RptSrc_DropFrm"); 
#endif
			 ALOGW("[video buffer] mBuffer=%p is not used before release,used=%d",mBuffer,used);
		    }
#endif			
				
		    mBuffer->release();
		

                mBuffer = NULL;
            }
            mBuffer = buffer;
            mResult = err;
            mLastBufferUpdateUs = ALooper::GetNowUs();

#ifndef ANDROID_DEFAULT_CODE	
 	     mBuffer->meta_data()->setInt64('RpIn', (mLastBufferUpdateUs / 1000));
	    ALOGI("[WFDP][video]read MediaBuffer %p,readtime=%lld ms",mBuffer, (mLastBufferUpdateUs-startUs)/1000);
#endif	

            mCondition.broadcast();

            if (err == OK) {
                postRead();
            }
#ifdef MTB_SUPPORT			
            ATRACE_END();
#endif
            break;
        }

        default:
            TRESPASS();
    }
}

void RepeaterSource::wakeUp() {
    ALOGV("wakeUp");
    Mutex::Autolock autoLock(mLock);
    if (mLastBufferUpdateUs < 0ll && mBuffer != NULL) {
        mLastBufferUpdateUs = ALooper::GetNowUs();
        mCondition.broadcast();
    }
}

}  // namespace android
