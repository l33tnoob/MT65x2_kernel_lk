
#ifndef VIDEO_QUALITY_CONTROLLER_H_
#define VIDEO_QUALITY_CONTROLLER_H_

#include <utils/Errors.h>
#include <media/stagefright/MetaData.h>


namespace android {
	
#define FPS_MEM_THRESHOLD_DEFAULT				40*1024*1024
#define FPS_MIN_MEM_THRESHOLD_DEFAULT			20*1024*1024
#define BITRATE_MEM_THRESHOLD_DEFAULT			10*1024*1024	
#define BITRATE_MIN_MEM_THRESHOLD_DEFAULT		5*1024*1024
	
class MPEG4Writer;

/******************************************************************************
*
*******************************************************************************/
class VideoQualityController 
{
public:
	VideoQualityController(MPEG4Writer *owner, const sp<MediaSource> &source);
	virtual ~VideoQualityController();
	
private:
	VideoQualityController(const VideoQualityController &);
	VideoQualityController &operator = (const VideoQualityController &);


/******************************************************************************
*  Operations in class VideoQualityController
*******************************************************************************/
public:
	void init(MetaData *param); 
	status_t configParams(MetaData *param);
	void propertyGetParams(MetaData *param);
	void adjustQualityIfNeed(/*Track* track,*/const int64_t memory_size = 0);
	void adjustForMemory(const int64_t memory_size);
	void adjustForTemperature();
	

private:
	MPEG4Writer *mOwner;
	sp<MediaSource> mSource;
	int32_t mVideoInitFPS;
	int32_t mVideoEncoder;
	int32_t mVideoWidth;
	int32_t mVideoHeight;
	int32_t mVideoBitRate;
	int64_t mVideoDynamicFPSLowThreshold;  // sam remove
	int64_t mVideoDynamicFPSHighThreshold;
	int64_t mVideoDynamicBitrateLowThreshold;  // sam remove
	int64_t mVideoDynamicBitrateHighThreshold;

	int64_t mLowMemoryProtectThreshold;
	
	int32_t mEnableForMemory;
	int32_t mEnableForTemperature;
	int32_t mVideoBitrateLowPercentage;
	int32_t mVideoDynamicAutoFPSDropRate;
	
	bool 	mBitrateAdjusted;
	bool 	mFPSAdjustedForMem;
	bool 	mFPSAdjustedForTemp;
};

};

#endif // VIDEO_QUALITY_CONTROLLER_H_


