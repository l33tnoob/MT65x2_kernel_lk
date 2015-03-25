
#define LOG_TAG "MPEG4FileCacheWriter"

#include <stdio.h>
#include <utils/Log.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
#include <media/stagefright/MPEG4Writer.h>
#include <media/mediarecorder.h>
#include <MPEG4FileCacheWriter.h>

#define ALOGV(fmt, arg...)       XLOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGD(fmt, arg...)       XLOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGI(fmt, arg...)       XLOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGW(fmt, arg...)       XLOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGE(fmt, arg...)       XLOGE("[%s] "fmt, __FUNCTION__, ##arg)

namespace android {
MPEG4FileCacheWriter::MPEG4FileCacheWriter(int fd,size_t cachesize)
{
	if (fd >= 0 && cachesize)
	{
		mpCache = malloc(cachesize);
		mCacheSize = cachesize;
		mDirtySize = 0;
		mFd = fd;
		mFileOpen = true;
		//mWriteDirty = false;
	}
	else
	{
		mpCache = NULL;
		mCacheSize = 0;
		mDirtySize = 0;
		mFd = -1;
		mFileOpen = false;
	}
#ifdef PERFORMANCE_PROFILE
	mTotaltime = 0;
	mMaxtime = 0;
	mTimesofwrite = 0;
	//moniter sdcard write speed
#ifdef TEST_BAD_PERFORMANCE
	mTestDelayFreq = -1;
	mTestDelayTimeUs = 0;
	
	char param[PROPERTY_VALUE_MAX];
	int64_t value;
	property_get("vr.test.write.delay.freq", param, "-1");
	value = atol(param);
	if(value >= 0)
	{
		mTestDelayFreq = value;
		ALOGD("[@RECORD_TEST]write.delay.freq = %lld", mTestDelayFreq);
	}

	
	property_get("vr.test.write.delay.time.us", param, "-1");
	value = atol(param);
	if(value >= 0)
	{
		mTestDelayTimeUs = value;
		ALOGD("[@RECORD_TEST]write.delay.time.us = %lld", mTestDelayTimeUs);
	}
#endif
#endif


}

MPEG4FileCacheWriter::~MPEG4FileCacheWriter()
{
	close();
	if (mpCache)
		free(mpCache);
	mpCache = NULL;
	mFileOpen = false;
#ifdef PERFORMANCE_PROFILE
	if(mTimesofwrite > 0 && mTotaltime > 0)
	ALOGD("write %lld times using %lld us, average is %lld us, max is %lld us", mTimesofwrite, mTotaltime, mTotaltime/mTimesofwrite, mMaxtime);
#endif
}

bool MPEG4FileCacheWriter::isFileOpen()
{
	return mFileOpen;
}

size_t MPEG4FileCacheWriter::write(const void * data,size_t size,size_t num)
{
	size_t ret = size*num;

	const uint8_t* ptr = (const uint8_t*)data;

	if (!mFileOpen)
	{
		ALOGE("File is not open when write");
		return -1;
	}
#ifdef SD_FULL_PROTECT
	if (mOwner->isSDFull()) {
		return ret;
	}
#endif
	if ((ret + mDirtySize) >= mCacheSize)
	{
//
		memcpy((uint8_t*)mpCache+mDirtySize, ptr, mCacheSize-mDirtySize);
#ifdef PERFORMANCE_PROFILE
		int64_t starttime = systemTime()/1000;
		//ALOGD("fwrite+");
#ifdef TEST_BAD_PERFORMANCE
		if ((mTestDelayFreq > 0) && (mTimesofwrite % mTestDelayFreq == 0)){
			usleep(mTestDelayTimeUs);
			//ALOGD("usleep %lld", mTestDelayTimeUs);
		}
#endif
#endif


		size_t real_write = ::write(mFd, mpCache, mCacheSize);
		//if (::write(mFd, mpCache, mCacheSize) < mCacheSize)
		if (real_write != mCacheSize)
		{
			ALOGE("file system write return error!!!Notify APP to stop record, write %d bytes, but return %d", mCacheSize, real_write);
			mOwner->notify(MEDIA_RECORDER_EVENT_ERROR, MEDIA_RECORDER_ERROR_UNKNOWN, 0);
#ifdef SD_FULL_PROTECT
			mOwner->setSDFull();
			mDirtySize = 0;
			return size*num;
#endif
		}
#ifdef PERFORMANCE_PROFILE
		//ALOGD("fwrite-");
		int64_t endtime = systemTime()/1000;
		int64_t durtime = endtime - starttime;
		mTotaltime += durtime;
		if(durtime > mMaxtime)
			mMaxtime = durtime;
		mTimesofwrite++;
		ALOGV("[PERFORMANCE]write 128k,consume time us = %lld, line %d",durtime,__LINE__);
#endif
		ret -= (mCacheSize - mDirtySize);
		ptr += (mCacheSize-mDirtySize);
		mDirtySize = 0;
//
		//if (flush() != OK)
		//	return -1;
	//}

		while (ret >= mCacheSize)
		{
#ifdef PERFORMANCE_PROFILE
			int64_t starttime = systemTime()/1000;
			//ALOGD("fwrite+");
#ifdef TEST_BAD_PERFORMANCE
			if ((mTestDelayFreq > 0) && (mTimesofwrite % mTestDelayFreq == 0)){
				usleep(mTestDelayTimeUs);
				//ALOGD("usleep %lld", mTestDelayTimeUs);
			}
#endif
#endif


			size_t real_write = ::write(mFd, ptr, mCacheSize);
			if (real_write != mCacheSize)
			//if (::write(mFd, ptr, mCacheSize) < mCacheSize)
			{
				ALOGE("file system write return error!!!Notify APP to stop record, write %d bytes, but return %d", mCacheSize, real_write);
				mOwner->notify(MEDIA_RECORDER_EVENT_ERROR, MEDIA_RECORDER_ERROR_UNKNOWN, 0);
#ifdef SD_FULL_PROTECT
				mOwner->setSDFull();
				mDirtySize = 0;
				return size*num;
#endif
			}
#ifdef PERFORMANCE_PROFILE
			//ALOGD("fwrite-");
			int64_t endtime = systemTime()/1000;
			int64_t durtime = endtime - starttime;
			mTotaltime += durtime;
			if(durtime > mMaxtime)
				mMaxtime = durtime;
			mTimesofwrite++;
			ALOGV("[PERFORMANCE]write 128k,consume time us = %lld, line %d",durtime,__LINE__);
#endif
			ret -= mCacheSize;
			ptr += mCacheSize;
		}
	}

		if(mpCache)
		{
			uint8_t *p = (uint8_t *)mpCache;
			p += mDirtySize;
			memcpy(p, ptr, ret);
			mDirtySize += ret;
		}
	return num*size;
}

int MPEG4FileCacheWriter::seek(off64_t offset,int refpos)
{
	if (!mFileOpen)
	{
		ALOGE("File is not open when seek");
		return -1;
	}
	flush();
	return lseek64(mFd, offset, refpos);
}

status_t MPEG4FileCacheWriter::flush()
{
	if (!mFileOpen)
	{
		ALOGE("File is not open when flush cache");
	}
#ifdef SD_FULL_PROTECT
	if (mOwner->isSDFull()) {
		return OK;
	}
#endif
	if (mDirtySize)
	{
		if (mpCache)
		{
			size_t ret;
			ret = ::write(mFd, mpCache, mDirtySize);
			if (ret != mDirtySize)
			{
				ALOGE("file system write return error!!!Notify APP to stop record, write %d bytes ,but return %d", mDirtySize, ret);
				mOwner->notify(MEDIA_RECORDER_EVENT_ERROR, MEDIA_RECORDER_ERROR_UNKNOWN, 0);
#ifdef SD_FULL_PROTECT
				mOwner->setSDFull();
				mDirtySize = 0;
				return -1;
#endif
			}
			
/*			if (!mWriteDirty)
			{
				mWriteDirty = true;
			}*/
		}
		mDirtySize = 0;
	}
	return OK;
}

int MPEG4FileCacheWriter::close()
{
	int ret = 0;
	if (mFileOpen)
	{
		flush();
		//mFileOpen = false;
		//if (mWriteDirty)
		//{
			//ALOGD("fflush");
			//fflush(mFile);
			//ALOGD("fsync");
			//fsync(fileno(mFile));
			ALOGD("Make sure write file to sd card complete before close");
		//}//make sure to write file to sd card complete
#ifdef SD_FULL_PROTECT
		if (mOwner->isSDFull()) {
			ALOGW("SD Card unexpected full when write meta data");
			mOwner->writeMetaData();
		}
		mOwner->finishHandleSDFull();
#endif		
		ret = ::close(mFd);
		mFd = -1;
		mFileOpen = false;
	}
//for test
#ifdef PERFORMANCE_PROFILE
	int64_t total_time,max_time,times_of_write;
	getPerformanceInfo(&total_time,&max_time,&times_of_write);
	ALOGI("MPEG4FileCacheWriter::close,total_time= %lld,max_time= %lld,times_of_write= %lld",\ 
		total_time,max_time,times_of_write);
	if((total_time > 0) && (max_time > 0) && (times_of_write > 0))
	ALOGI("MPEG4FileCacheWriter::close,write speed = %lld KByte/second",times_of_write * 128 * 1000000/total_time); //kbyte/second
#endif
	return ret;
}

bool MPEG4FileCacheWriter::getFile()
{
	return (mFd >= 0);
}

void MPEG4FileCacheWriter::setOwner(MPEG4Writer *owner)
{
	mOwner = owner;
}

#ifdef PERFORMANCE_PROFILE
void MPEG4FileCacheWriter::getPerformanceInfo(int64_t* total_time, 
	int64_t* max_time, int64_t* times_of_write) {
	if (total_time != NULL)
		*total_time = mTotaltime;
	if (max_time != NULL)
		*max_time = mMaxtime;
	if (times_of_write != NULL)
		*times_of_write = mTimesofwrite;
}
#endif

}

