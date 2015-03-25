
#define LOG_TAG "AudioTrackCenter"
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <cutils/xlog.h>
#include <media/AudioTrackCenter.h>
#include <media/AudioSystem.h>
#include <system/audio.h>

#define AUDIOFLINGER_BUFFERCOUNT 6

namespace android {

AudioTrackCenter::AudioTrackCenter() 
    : mAfFrameCount(0),
      mAfSampleRate(0)
#ifdef AUDIO_TRACK_CENTER_DEBUG	      
      ,mSysTimeUs(0),
      mRealTimeUs(0),
      mDeltaUs(0.0)
#endif 
{

}

AudioTrackCenter::~AudioTrackCenter() {
   
}

status_t AudioTrackCenter::init() {
	audio_stream_type_t streamType;
	if (!mAfFrameCount || !mAfSampleRate) {
	    if (AudioSystem::getOutputFrameCount(&mAfFrameCount, streamType) != NO_ERROR) {
	        SXLOGE("AudioSystem::getOutputFrameCount Fail!!!");
			return NO_INIT;
	    }
	    if (AudioSystem::getOutputSamplingRate(&mAfSampleRate, streamType) != NO_ERROR) {
	        SXLOGE("AudioSystem::getOutputSamplingRate Fail!!!");
			return NO_INIT;
	    }
		SXLOGD("init, mAfFrameCount = %d, mAfSampleRate = %d",mAfFrameCount, mAfSampleRate);
	}

	return OK;
}

status_t AudioTrackCenter::addTrack(int32_t trackId, uint32_t frameCount, uint32_t sampleRate, void* trackPtr) {
    Mutex::Autolock autoLock(mLock);

	SXLOGD("%s, trackId:%d, frameCount:%d, sampleRate:%d, trackPtr:%p",__FUNCTION__,trackId, frameCount, sampleRate, trackPtr);

	if (init() != OK) return NO_INIT;
        
    ssize_t index = mTrackList.indexOfKey(trackId);

    if (index >= 0) {
    	SXLOGW("trackId: %d has existed!!!", trackId);
        //return INVALID_OPERATION;
    }
    
    List<TrackMaps>::iterator it = mTrackMaps.begin();
    bool newTrack = true;
    int64_t framePlayed = 0;
    while(it != mTrackMaps.end()) {
        if ((*it).trackPtr == trackPtr ) {
            ssize_t index = mTrackList.indexOfKey((*it).trackId);
            if (index >= 0) {
                SXLOGD("%s, update track info from trackId:%d to trackId:%d", __FUNCTION__, (*it).trackId, trackId);
                struct TrackInfo &info = mTrackList.editValueFor((*it).trackId);
                framePlayed = info.framePlayed;
                mTrackList.removeItemsAt(index);
			    
                TrackMaps *maps = &*it;
			    maps->trackId = trackId;
                newTrack = false;
            }
            break;
        }
        ++it;
    }
    
    struct TrackInfo info;
    info.server = 0;
    info.frameCount = frameCount;
    info.framePlayed = framePlayed;
    info.afFrameCount = mAfSampleRate ? (sampleRate*mAfFrameCount)/mAfSampleRate : frameCount/AUDIOFLINGER_BUFFERCOUNT;
    info.sampleRate = sampleRate;
    info.middleServer = 0;
    info.active = true;
    info.ts = ALooper::GetNowUs();
    mTrackList.add(trackId, info);

    if (newTrack) {
        struct TrackMaps maps;
        maps.trackId  = trackId;
        maps.trackPtr = trackPtr;
        maps.sinkPtr  = NULL;
        mTrackMaps.push_back(maps);
    }
        
    return OK;
}

status_t AudioTrackCenter::removeTrack(void* trackPtr) {
    Mutex::Autolock autoLock(mLock);

	SXLOGD("%s, trackPtr:%p",__FUNCTION__, trackPtr);
    
    List<TrackMaps>::iterator it = mTrackMaps.begin();
    while(it != mTrackMaps.end()) {
        if ((*it).trackPtr == trackPtr ) {
            ssize_t index = mTrackList.indexOfKey((*it).trackId);
            if (index < 0) {
                return UNKNOWN_ERROR;
            } 
            mTrackList.removeItemsAt(index);
			mTrackMaps.erase(it);
            break;
        }
        ++it;
    }
        
    return OK;
   
}

status_t AudioTrackCenter::updateTrackMaps(void* trackPtr, void* sinkPtr) {
	Mutex::Autolock autoLock(mLock);

    SXLOGD("%s, trackPtr:%p, sinkPtr:%p",__FUNCTION__, trackPtr, sinkPtr);	

	List<TrackMaps>::iterator it = mTrackMaps.begin();
    while (it != mTrackMaps.end()) {
		if (it->trackPtr == trackPtr) {
			TrackMaps *maps = &*it;
			maps->sinkPtr = sinkPtr;
			return OK;
		}
        ++it;
    }

	return UNKNOWN_ERROR;
}

status_t AudioTrackCenter::updateServer(int32_t trackId, uint32_t server, bool restore) {
    Mutex::Autolock autoLock(mLock);

	SXLOGV("%s, trackId:%d, server:%d",__FUNCTION__, trackId, server);
        
    ssize_t index = mTrackList.indexOfKey(trackId);
	if (index < 0) {
    	return UNKNOWN_ERROR;
	}

    struct TrackInfo &info = mTrackList.editValueFor(trackId);

    if (!info.active) {
        SXLOGV("%s, trackId:%d, active = %d",__FUNCTION__, trackId, info.active);
        return OK;
    }    
	
    uint32_t s;
    s = (server > info.server) ? (server - info.server) : 0;
	
    if (s && info.middleServer && s < info.afFrameCount) {
		info.middleServer  = server;
        return OK;
    }
	
    if (!restore && info.server) {
	    info.framePlayed = info.framePlayed + s;
        info.ts = ALooper::GetNowUs();
    }

    info.server = server;
	
	SXLOGV("trackId:%d, info.server:%d, info.framePlayed:%lld, info.ts:%lld",trackId, info.server, info.framePlayed, info.ts);
        
    info.middleServer  = server;
    
    return OK;   
}

int32_t AudioTrackCenter::getTrackId(void* sinkPtr) {
	Mutex::Autolock autoLock(mLock);

    SXLOGV("%s, sinkPtr:%p",__FUNCTION__, sinkPtr);	

	List<TrackMaps>::iterator it = mTrackMaps.begin();
    while (it != mTrackMaps.end()) {
		if (it->sinkPtr == sinkPtr) {
            SXLOGV("%s, return trackId:%d",__FUNCTION__, it->trackId);
			return it->trackId;
		}
        ++it;
    }
    SXLOGV("%s, no valid trackId!!",__FUNCTION__);
	return 0;
}

status_t AudioTrackCenter::getRealTimePosition(int32_t trackId, int64_t *position) {
    Mutex::Autolock autoLock(mLock);

	SXLOGV("%s, trackId:%d",__FUNCTION__, trackId);
        
    ssize_t index = mTrackList.indexOfKey(trackId);
	if (index < 0) {
    	return UNKNOWN_ERROR;
	}
	
	const struct TrackInfo info = mTrackList.valueFor(trackId);
	int64_t delayUs = ALooper::GetNowUs() - info.ts;

	*position = info.framePlayed;
	
	if (!info.framePlayed) {
	    SXLOGV("trackId = %d, server = %d, framePlayed = %lld", trackId, info.server, info.framePlayed);
	    return OK;
	}

	if (info.server) {
		uint32_t deltaFrames = (uint32_t)((delayUs*info.sampleRate)/1000000);
		if (deltaFrames > info.frameCount) {
			deltaFrames = info.frameCount;
		}
        
        if (!info.active) {
            SXLOGV("%s, trackId = %d, track is not active , set deltaFrames to 0",__FUNCTION__, trackId);
            deltaFrames = 0;
        }
		*position += deltaFrames;
	}
	
#ifdef AUDIO_TRACK_CENTER_DEBUG	 
	SXLOGD("trackId = %d, realTimeUs and sysTimeUs distance: %8.3f", trackId, countDeltaUs(((int64_t)(*position)*1000000)/info.sampleRate));
#endif

	SXLOGV("trackId = %d, server = %d, framePlayed = %lld, delayUs = %lld, *position:%lld", trackId, info.server, info.framePlayed, delayUs, *position);
	    
    return OK;
}

status_t AudioTrackCenter::setTrackActive(int32_t trackId, bool active) {
    Mutex::Autolock autoLock(mLock);

    SXLOGV("%s, trackId:%d, active:%d",__FUNCTION__, trackId, active);
    
    ssize_t index = mTrackList.indexOfKey(trackId);
	if (index < 0) {
    	return UNKNOWN_ERROR;
	}

    struct TrackInfo &info = mTrackList.editValueFor(trackId);

    info.active = active;

    return OK;

}

status_t AudioTrackCenter::reset(int32_t trackId) {
	Mutex::Autolock autoLock(mLock);

	SXLOGV("%s, trackId:%d",__FUNCTION__, trackId);

	ssize_t index = mTrackList.indexOfKey(trackId);
	if (index < 0) {
    	return UNKNOWN_ERROR;
	}

    struct TrackInfo &info = mTrackList.editValueFor(trackId);

    info.server = 0;
	info.framePlayed = 0;
    info.middleServer = 0;
	info.ts = ALooper::GetNowUs();

#ifdef AUDIO_TRACK_CENTER_DEBUG	 
    mSysTimeUs = 0;
    mRealTimeUs = 0;
    mDeltaUs = 0;
#endif

	return OK;
}

#ifdef AUDIO_TRACK_CENTER_DEBUG	 
float AudioTrackCenter::countDeltaUs(int64_t realTimeUs) {
    int64_t deltaRealUs = 0;
	int64_t delatSysUs = 0 ;

	if (!realTimeUs) return 0;
	
	if (mSysTimeUs) {
		deltaRealUs = realTimeUs - mRealTimeUs;
		delatSysUs = ALooper::GetNowUs() -	mSysTimeUs;
	}	
	mSysTimeUs = ALooper::GetNowUs();
	mRealTimeUs = realTimeUs;

    mDeltaUs = (float)(deltaRealUs - delatSysUs)/1000.00;
	
	return mDeltaUs;

}
#endif

}
