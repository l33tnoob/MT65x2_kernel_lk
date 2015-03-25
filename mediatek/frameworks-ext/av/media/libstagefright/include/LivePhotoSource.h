
#ifndef LIVE_PHOTO_SOURCE_H_
#define LIVE_PHOTO_SOURCE_H_

#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/threads.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>

namespace android {

/******************************************************************************
*
*******************************************************************************/
class LivePhotoSource : public virtual MediaSource, public virtual Thread
{
public:
	LivePhotoSource(const sp<MediaSource> &source);
	virtual ~LivePhotoSource();

/******************************************************************************
*  Operations in base class MediaSource
*******************************************************************************/
public:
	virtual status_t 		start(MetaData *params = NULL);
	virtual status_t 		stop();
	virtual status_t 		read(MediaBuffer **buffer, const ReadOptions *options = NULL);
	virtual sp<MetaData> 	getFormat();
	
/******************************************************************************
*  Operations in base class Thread
*******************************************************************************/
public: 	////
	virtual status_t	readyToRun();
	
private:
	virtual bool		threadLoop();

/******************************************************************************
*  Operations in class LivePhotoSource
*******************************************************************************/
public:
	status_t	startLivePhoto();
	void 		setLPKeepTimeUs(int64_t timeUs);
	void		updateBufferPool();
private:
	LivePhotoSource(const LivePhotoSource &);
	LivePhotoSource &operator = (const LivePhotoSource &);
	
private:
    sp<MediaSource> 	mSource;
	int64_t 			mKeepTimeUs;
	MediaBuffer *       mCodecConfigBuffer;
	List<MediaBuffer *> mMediaBufferPool;
	volatile bool		mSourceStarted;
	volatile bool		mLivePhotoStarted;
	Mutex 				mLock;
	Condition 			mFrameAvailableCond;
	Condition 			mWriterReadEndCond;
	Condition 			mThreadExitCond;
	volatile bool		mIsThreadExit;
};

};

#endif // LIVE_PHOTO_SOURCE_H_


