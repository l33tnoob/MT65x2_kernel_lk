#ifndef AUDIO_TRACK_CENTER_H_
#define AUDIO_TRACK_CENTER_H_

#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/List.h>
#include <media/stagefright/foundation/ABase.h>

//#define AUDIO_TRACK_CENTER_DEBUG

namespace android {

class AudioTrackCenter : public RefBase {
public:	
	AudioTrackCenter();
	
	virtual ~AudioTrackCenter();

	status_t addTrack(int32_t trackId, uint32_t frameCount, uint32_t sampleRate, void* trackPtr);

	status_t removeTrack(void* trackPtr);

	status_t updateTrackMaps(void* trackPtr, void* sinkPtr);

	status_t updateServer(int32_t trackId, uint32_t server, bool restore=false);

	int32_t getTrackId(void* sinkPtr);
	
	status_t getRealTimePosition(int32_t trackId, int64_t *position);

    status_t setTrackActive(int32_t trackId, bool active);

	status_t reset(int32_t trackId);

private:
	status_t init();
#ifdef AUDIO_TRACK_CENTER_DEBUG	
	float countDeltaUs(int64_t realTimeUs);
#endif
	
    struct TrackInfo {
        uint32_t server;
        uint32_t frameCount;
		int64_t framePlayed;
		uint32_t afFrameCount;
        uint32_t sampleRate;
        uint32_t middleServer;
        bool active;
        int64_t  ts;
    };

	struct TrackMaps {
		int32_t trackId;
		void*   trackPtr;
		void*   sinkPtr;
	};
	
	Mutex mLock;
	KeyedVector<int32_t, TrackInfo > mTrackList;
	List<TrackMaps> mTrackMaps;
	
	size_t mAfFrameCount;
	uint32_t mAfSampleRate;

#ifdef AUDIO_TRACK_CENTER_DEBUG
	int64_t mSysTimeUs;
	int64_t mRealTimeUs;
	float   mDeltaUs;
#endif	
	
    DISALLOW_EVIL_CONSTRUCTORS(AudioTrackCenter);

};

}

#endif  //AUDIO_TRACK_CENTER_H_
