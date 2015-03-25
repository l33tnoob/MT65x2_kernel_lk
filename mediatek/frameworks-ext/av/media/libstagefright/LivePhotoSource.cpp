
#define LOG_TAG "LivePhotoSource"
#include <utils/Log.h>
#include <cutils/xlog.h>

#include <linux/rtpm_prio.h>
#include <sys/prctl.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/OMXCodec.h>

#include <LivePhotoSource.h>

#define ALOGV(fmt, arg...)       XLOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGD(fmt, arg...)       XLOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGI(fmt, arg...)       XLOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGW(fmt, arg...)       XLOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGE(fmt, arg...)       XLOGE("[%s] "fmt, __FUNCTION__, ##arg)

#define DEFAULT_KEEP_TIMEUS 	3000000 // 3S

namespace android {

/******************************************************************************
*
*******************************************************************************/
LivePhotoSource::LivePhotoSource(const sp<MediaSource> &source)
	:mSource(source)
	,mKeepTimeUs(DEFAULT_KEEP_TIMEUS)
	,mCodecConfigBuffer(NULL)
	,mMediaBufferPool()		// Sample data
	,mSourceStarted(false)
	,mLivePhotoStarted(false)
	,mLock()
	,mFrameAvailableCond() 
	,mWriterReadEndCond()
	,mThreadExitCond()
	,mIsThreadExit(false)
	{

}

/******************************************************************************
*
*******************************************************************************/
LivePhotoSource::~LivePhotoSource() {
	ALOGD("+");

	stop();

	if(mSource != NULL)
		mSource.clear();

	if(mCodecConfigBuffer != NULL) {
		mCodecConfigBuffer->release();
		mCodecConfigBuffer = NULL;
	}
	
	while (!mMediaBufferPool.empty()) {
		List<MediaBuffer*>::iterator it = mMediaBufferPool.begin();
		(*it)->release();
		(*it) = NULL;
		mMediaBufferPool.erase(it);
	}

	mMediaBufferPool.clear();

	ALOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
status_t LivePhotoSource::start(MetaData *params) {
	ALOGD("+");
	Mutex::Autolock _lock(mLock);
	
	if(mSource == NULL)	{
		ALOGE("Failed: mSource is NULL");
		return UNKNOWN_ERROR;
	}
	status_t err = mSource->start(params);
	if (err != OK) {
		ALOGE("Failed: source start err(%d)", err);
		return err;
	}

	mSourceStarted = true;
	run();
	
	ALOGD("-");
	return err;
}
/******************************************************************************
*
*******************************************************************************/
status_t LivePhotoSource::stop() 
{
	ALOGD("+");
	bool bStopSource = false;
	{
		Mutex::Autolock _lock(mLock);

		if(mSourceStarted && !mLivePhotoStarted) {
			bStopSource = true;
			mSourceStarted = false;
			ALOGD("signal to stop writer read");
			mFrameAvailableCond.signal();
			//mWriterReadEndCond.wait(mLock);
		}
		else if(mLivePhotoStarted) {
			ALOGD("wait writer read end");
			mWriterReadEndCond.wait(mLock);
		}
	}
	
	status_t err = OK;
	if(bStopSource) {
		{
			Mutex::Autolock _lock(mLock);
			if(mIsThreadExit) {
				ALOGD("thread exited, no need wait");
			}
			else {
				ALOGD("wait thread exit");
				mThreadExitCond.wait(mLock);
			}
		}
		requestExit();
		requestExitAndWait();

		if(mSource != NULL)	{
			ALOGD("mSource stop()");
			err = mSource->stop();
		}
	}

	ALOGD("-");
	return err; 
}

/******************************************************************************
*
*******************************************************************************/
status_t LivePhotoSource::read(MediaBuffer **buffer, const ReadOptions *options) {
	ALOGD("+");

    *buffer = NULL;
	if(options != NULL) {
		ALOGE("Failed: LivePhotoSource dose not support read options");
		return ERROR_UNSUPPORTED;
	}
	
	{
		Mutex::Autolock _lock(mLock);

		if( (mSourceStarted && !mLivePhotoStarted) || (mSourceStarted && mLivePhotoStarted && (mMediaBufferPool.empty() && mCodecConfigBuffer==NULL))) {
	        status_t status = mFrameAvailableCond.wait(mLock);
	        if  ( NO_ERROR != status ) {
	            ALOGE("wait status(%d) err", status);
				return UNKNOWN_ERROR;
	        }
		}

		if(mLivePhotoStarted && (mMediaBufferPool.empty() && mCodecConfigBuffer==NULL))
			mLivePhotoStarted = false;

		if( !mSourceStarted && !mLivePhotoStarted ) {
			mWriterReadEndCond.signal();
			ALOGD("- Live photo stoped, return ERROR_END_OF_STREAM");
			return ERROR_END_OF_STREAM;
		}
			
		if( mCodecConfigBuffer != NULL)	{
			ALOGD("codec config buffer");
			*buffer = mCodecConfigBuffer;
			mCodecConfigBuffer = NULL;
		}
		else if(!(mMediaBufferPool.empty())) {
			List<MediaBuffer *>::iterator it = mMediaBufferPool.begin();
			*buffer = (*it);
			mMediaBufferPool.erase(it);
		}
		
		ALOGD("-");
		return OK;
	}
}

/******************************************************************************
*
*******************************************************************************/
sp<MetaData> LivePhotoSource::getFormat() {
	if(mSource == NULL)	{
		ALOGE("Failed: mSource is NULL");
		return NULL;
	}
	return mSource->getFormat();
}
status_t LivePhotoSource::startLivePhoto() {
	ALOGD("+");
	bool bStopSource = false;
	
	{
		Mutex::Autolock _lock(mLock);

		if(mSourceStarted && !mLivePhotoStarted) {
			bStopSource = true;
			mLivePhotoStarted = true;
			//mSourceStarted = false;  // move before source stop()??
			ALOGD("wait read source end");
		}
		else if(mLivePhotoStarted) {
			ALOGD("live photo has been started");
			return OK;
		}
		else
			return UNKNOWN_ERROR; 
	}
	
	status_t err = OK;
	if(bStopSource) {
		{
			Mutex::Autolock _lock(mLock);
			ALOGD("wait thread exit");
			mThreadExitCond.wait(mLock);
		}
		requestExit();
		requestExitAndWait();

		if(mSource != NULL)	{
			ALOGD("mSource stop()");
			err = mSource->stop();
		}
	}

	ALOGD("-");
	return err; 
}

/******************************************************************************
*
*******************************************************************************/
void LivePhotoSource::setLPKeepTimeUs(int64_t timeUs) {
	ALOGD("%lldus +", timeUs);

	Mutex::Autolock _lock(mLock);
	
	if(!mLivePhotoStarted) {
		ALOGD("real set keep time: %lldus", timeUs);
		mKeepTimeUs = timeUs;
		updateBufferPool();  // how to update the lastext time -1??
	}
	
	ALOGD("-");
}


/******************************************************************************
*
*******************************************************************************/
void LivePhotoSource::updateBufferPool() 
{  // must be protected by mLock
	ALOGD("+");

	if(!mMediaBufferPool.empty()) 
	{  // only check mStarted

			List<MediaBuffer *>::iterator newBegin = mMediaBufferPool.begin();
			int64_t timestampUs;
			int64_t latestTimestampUs;
			
			List<MediaBuffer *>::iterator latest = mMediaBufferPool.end();   // this will be null, there alse need a wait
			latest--;
			CHECK((*latest)->meta_data()->findInt64(kKeyTime, &latestTimestampUs));

			for (List<MediaBuffer *>::iterator it = mMediaBufferPool.begin();
				 it != mMediaBufferPool.end(); ++it) 
			{
				CHECK((*it)->meta_data()->findInt64(kKeyTime, &timestampUs));

				
				ALOGI("check timestamp is %lldus, latestTimestampUs=%lld", timestampUs, latestTimestampUs);
				if(latestTimestampUs - timestampUs < mKeepTimeUs)	
					break;
				
				int32_t isSync = false;
				(*it)->meta_data()->findInt32(kKeyIsSyncFrame, &isSync);
				if(isSync) 
				{
					newBegin = it;
				}
		 	}

			for (List<MediaBuffer *>::iterator it = mMediaBufferPool.begin();
				 it != newBegin; ) 
			{
				 (*it)->release();
				 (*it) = NULL;
				 it = mMediaBufferPool.erase(it);
			}
	}
	
	ALOGD(" -");
}

/******************************************************************************
*
*******************************************************************************/
// Good place to do one-time initializations
status_t LivePhotoSource::readyToRun() {
	ALOGD("+");
	::prctl(PR_SET_NAME,"LivePhotoThread", 0, 0, 0);
	//	thread policy & priority
	//	Notes:
	//		Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
	//		may inherit the non-SCHED_OTHER policy & priority of the thread creator.
	//		And thus, we must set the expected policy & priority after a thread creation.
	int const policy	= SCHED_RR;
	int const priority	= RTPM_PRIO_VIDEO_BS_BUFFER;
	//
	struct sched_param sched_p;
	::sched_getparam(0, &sched_p);
	//
	//	set
	sched_p.sched_priority = priority;	//	Note: "priority" is real-time priority.
	::sched_setscheduler(0, policy, &sched_p);
	//
	//	get
	::sched_getparam(0, &sched_p);
	//
	ALOGD("policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d) -",
		 policy, ::sched_getscheduler(0), priority, sched_p.sched_priority);
	return NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
bool LivePhotoSource:: threadLoop() {
	ALOGD("+");
	status_t err = OK;
    MediaBuffer *buffer = NULL;
	int32_t isSync = false;

	while(mSourceStarted && !exitPending() && ((err = mSource->read(&buffer)) == OK)) {
        MediaBuffer* copy = new MediaBuffer(buffer->range_length(), buffer->meta_data());
        memcpy( copy->data(), (uint8_t *)buffer->data() + buffer->range_offset(), buffer->range_length() );
        copy->set_range(0, buffer->range_length());

		int64_t latestTimestampUs;
		CHECK(copy->meta_data()->findInt64(kKeyTime, &latestTimestampUs));
		ALOGI("cur timestamp is %lldus", latestTimestampUs);
		{
			Mutex::Autolock _lock(mLock);
			
			int32_t isCodecConfig;
			if(copy->meta_data()->findInt32(kKeyIsCodecConfig, &isCodecConfig) && isCodecConfig ) {
				if(mCodecConfigBuffer != NULL) {
					mCodecConfigBuffer->release();
					mCodecConfigBuffer = NULL;
				}
				
				ALOGD("keep codec config buffer");
				mCodecConfigBuffer = copy;
			}
			else {
		        mMediaBufferPool.push_back(copy);

				if(mLivePhotoStarted) {
					mFrameAvailableCond.signal();
					copy->meta_data()->findInt32(kKeyIsSyncFrame, &isSync);
					
					if (!isSync) {
						if (reinterpret_cast<OMXCodec *>(mSource.get())->
							vEncSetForceIframe(true) != OK)
							ALOGW("Send force I cmd fail");
					}
					else {
						mSourceStarted = false;
						buffer->release();
						buffer = NULL;
						break; // 
					}
				}
				else {
					updateBufferPool();
				}
			}
		}

		buffer->release();
		buffer = NULL;
	}

	{
		Mutex::Autolock _lock(mLock);
		if(err != OK) {
			ALOGE("read source err(%d) . this is a bad livephoto", err);
		}
		
		if(mSourceStarted && mLivePhotoStarted) {
			mLivePhotoStarted = false;
			mSourceStarted = false;
			ALOGE("there is an error with exiting while when livephoto started");
			mFrameAvailableCond.signal();
		}
		ALOGD("Thread exit signal");
		mThreadExitCond.signal();
		mIsThreadExit = true;
	}
	
	ALOGD("-");
	return false;
}

}

