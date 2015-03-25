
#define LOG_TAG "VideoQualityController"
#include <utils/Log.h>
#include <cutils/xlog.h>
#include <media/MediaProfiles.h> 
#include <cutils/properties.h>
#include <media/stagefright/OMXCodec.h>
#include <media/stagefright/CameraSource.h>
#include "custom_vr_video_drop_rate.h"
#include <media/stagefright/MPEG4Writer.h>
#include <stdio.h>

#include <VideoQualityController.h>

#define ALOGV(fmt, arg...)       XLOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGD(fmt, arg...)       XLOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGI(fmt, arg...)       XLOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGW(fmt, arg...)       XLOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGE(fmt, arg...)       XLOGE("[%s] "fmt, __FUNCTION__, ##arg)


namespace android {

/******************************************************************************
*
*******************************************************************************/
VideoQualityController::VideoQualityController(MPEG4Writer *owner, const sp<MediaSource> &source) 
    :mOwner(owner)
    ,mSource(source)
    ,mVideoInitFPS(-1)
	,mVideoEncoder(-1)
	,mVideoWidth(-1)
	,mVideoHeight(-1)
	,mVideoBitRate(-1)
	,mVideoDynamicFPSLowThreshold(FPS_MIN_MEM_THRESHOLD_DEFAULT)
	,mVideoDynamicFPSHighThreshold(FPS_MEM_THRESHOLD_DEFAULT)
	,mVideoDynamicBitrateLowThreshold(BITRATE_MIN_MEM_THRESHOLD_DEFAULT)
	,mVideoDynamicBitrateHighThreshold(BITRATE_MEM_THRESHOLD_DEFAULT)
	,mBitrateAdjusted(false)
	,mFPSAdjustedForMem(false)
	,mFPSAdjustedForTemp(false)
	,mEnableForMemory(false)
	,mEnableForTemperature(false)
	,mVideoBitrateLowPercentage(-1)
	,mVideoDynamicAutoFPSDropRate(-1) {

	

}

/******************************************************************************
*
*******************************************************************************/
VideoQualityController::~VideoQualityController() {
	ALOGD("+");


	ALOGD("-");
}

void VideoQualityController::init(MetaData *param) {
	ALOGD("+");
	if(param && param->findInt32(kKeyVideoEncoder, &mVideoEncoder)){//has video track
		ALOGI("start,has video track,mVideoEncoder=%d,start config params",mVideoEncoder);
		configParams(param);
		propertyGetParams(param);
	}
	else {
		ALOGE("not has video track");
	}
	
	ALOGD("-");
}

status_t VideoQualityController::configParams(MetaData *param){
	
	ALOGD("+");
	param->findInt32(kKeyFrameRate, &mVideoInitFPS);
	param->findInt32(kKeyWidth,&mVideoWidth);
	param->findInt32(kKeyHeight,&mVideoHeight);
	param->findInt32(kKeyVideoBitRate,&mVideoBitRate);
	param->findInt32(kKeyVQForMem,&mEnableForMemory);

	if(mVideoInitFPS>30 || mVideoWidth>1920 || mVideoHeight>1088) {
		param->findInt32(kKeyVQForTemp,&mEnableForTemperature);
		ALOGD("get kKeyVQForTemp=%d", mEnableForTemperature);
	}

	ALOGD("video init FPS=%d,int bitrate=%d,init width=%d,height=%d, mEnableForMemory=%d, mEnableForTemperature=%d",\
	 mVideoInitFPS, mVideoBitRate, mVideoWidth, mVideoHeight, mEnableForMemory, mEnableForTemperature);

	MediaProfiles * mEncoderProfiles = NULL; 
	mEncoderProfiles = MediaProfiles::getInstance();
	if(!mEncoderProfiles) {
		ALOGE("can not get MediaProfile instance");
		mVideoBitrateLowPercentage = -1;
		mVideoDynamicAutoFPSDropRate = -1;
		return INVALID_OPERATION;
	}
	
	int iEntyNum = 0; 
	iEntyNum= sizeof(sVideoQualityAdjustParamTable)/sizeof(sVideoQualityAdjustParamTable[0]);
	if(iEntyNum <=0) {
		ALOGE("no sVideoQualityAdjustParamTable set");
		mVideoBitrateLowPercentage = -1;
		mVideoDynamicAutoFPSDropRate = -1;
		return INVALID_OPERATION;
	}
	//int iCameraId = BACK_CAMERA
	for(int i = 0; i < iEntyNum ;i++){
		camcorder_quality quality = sVideoQualityAdjustParamTable[i].mQuality;
		int32_t iCameraId = sVideoQualityAdjustParamTable[i].mCameraId;
		
		int videoCodec  = mEncoderProfiles->getCamcorderProfileParamByName("vid.codec", iCameraId, quality);
		int videoFrameRate   = mEncoderProfiles->getCamcorderProfileParamByName("vid.fps", iCameraId, quality);
    	int videoFrameWidth  = mEncoderProfiles->getCamcorderProfileParamByName("vid.width", iCameraId, quality);
    	int videoFrameHeight = mEncoderProfiles->getCamcorderProfileParamByName("vid.height", iCameraId, quality);
		int videoBitRate     = mEncoderProfiles->getCamcorderProfileParamByName("vid.bps", iCameraId, quality);
		ALOGI("i=%d,quality=%d,iCameraId=%d",\ 
			i,quality,iCameraId);
		ALOGI("videoCodec=%d,videoFrameRate=%d,videoFrameWidth=%d,videoFrameHeight=%d,videoBitRate=%d",\ 
			videoCodec,videoFrameRate,videoFrameWidth,videoFrameHeight,videoBitRate);
		
		if(	videoCodec == mVideoEncoder &&
			videoFrameRate == mVideoInitFPS &&
			videoFrameWidth == mVideoWidth &&
			videoFrameHeight == mVideoHeight &&
			videoBitRate == mVideoBitRate){

			mVideoBitrateLowPercentage = sVideoQualityAdjustParamTable[i].mBitRateDropPercentage;	
			mVideoDynamicAutoFPSDropRate = sVideoQualityAdjustParamTable[i].mFrameRateDropPercentage;
			ALOGI(" match a quality,quality=%d,cameraId=%d,mVideoBitrateLowPercentage=%d,mVideoDynamicAutoFPSDropRate=%d",\
				 quality,iCameraId,mVideoBitrateLowPercentage,mVideoDynamicAutoFPSDropRate);

			if(mVideoBitrateLowPercentage > 0 && mVideoBitrateLowPercentage <= 100 &&
				mVideoDynamicAutoFPSDropRate > 0 && mVideoDynamicAutoFPSDropRate <=100)
					return OK;
			else{
				mVideoBitrateLowPercentage = -1;
				mVideoDynamicAutoFPSDropRate = -1;
				ALOGE("mVideoBitrateLowPercentage,mVideoDynamicAutoFPSDropRate invalid value!!!");
				return INVALID_OPERATION;
			}
				
		}
   		
	}
	
	ALOGW("sVideoQualityAdjustParamTable no related Quality Param");
	mVideoBitrateLowPercentage = -1;
	mVideoDynamicAutoFPSDropRate = -1;
	
	ALOGD("-");
	return INVALID_OPERATION;
	
}

void VideoQualityController::propertyGetParams(MetaData *metadata)
{
	ALOGD("+");

	char param[PROPERTY_VALUE_MAX];
	int32_t value;

	property_get("vr.fps.adjust.for.temp", param, "-1");  // this only for temperture  protect
    value = atol(param);
	if(value >= 1)
	{
		ALOGI("enable fps adjust for temperature by property_get");
		mEnableForTemperature = true;
	}

	property_get("vr.quality.adjust.disable", param, "-1");  // this only for memory protect
    value = atol(param);
	if(value >= 1)
	{
		mEnableForMemory = false;
	}
	
	if(!mEnableForMemory) {
		ALOGI("not enable QualityAdjust");
		return;
	}	
  
    property_get("vr.auto.fps.drop.rate", param, "-1");
    value = atol(param);
	if(value > 0)
	{
		if (value > 100) {
			ALOGW("auto.fps.drop.rate set fail %d, keep default value", value);
		} else {
			mVideoDynamicAutoFPSDropRate = value;
		}
	}

	
    property_get("vr.bitrate.low.percentage", param, "-1");
    value = atol(param);
	if(value > 0)
	{
		if (value > 100) {
			ALOGW("bitrate.low.percentage set fail %d, keep default value", value);
		} else {
			mVideoBitrateLowPercentage = value;
		}
	}

    property_get("vr.bitrate.low.threshold", param, "-1");
    value = atol(param);
	if(value > 0)
	{
		if (value >= mLowMemoryProtectThreshold) {
			ALOGW("bitrate.low.threshold too large %d, disable bitrate adjustment", value);
		}
		mVideoDynamicBitrateLowThreshold = value;
	}
	
    property_get("vr.bitrate.high.threshold", param, "-1");
    value = atol(param);
	if(value > 0)
	{
		if (value >= (int32_t)mLowMemoryProtectThreshold) {
			ALOGW("bitrate.high.threshold too large %d, disable bitrate adjustment", value);
		} else if (value <= mVideoDynamicBitrateLowThreshold) {
			ALOGW("bitrate.high.threshold too small %d, set it to %d", value, mVideoDynamicBitrateLowThreshold + 5*1024*1024);
			value = (int32_t)mVideoDynamicBitrateLowThreshold + 5*1024*1024;
		}
		mVideoDynamicBitrateHighThreshold = value;
	}
	
    property_get("vr.fps.low.threshold", param, "-1");
    value = atol(param);
	if(value > 0)
	{
		if (value >= mLowMemoryProtectThreshold) {
			ALOGW("fps.low.threshold too large %d, disable fps adjustment", value);
		}
		mVideoDynamicFPSLowThreshold = value;
	}
	
    property_get("vr.fps.high.threshold", param, "-1");
    value = atol(param);
	if(value > 0)
	{
		if (value >= (int32_t)mLowMemoryProtectThreshold) {
			ALOGW("fps.high.threshold too large %d, disable bitrate adjustment", value);
		} else if (value <= mVideoDynamicFPSLowThreshold) {
			ALOGW("fps.high.threshold too small %d, set it to %d", value, mVideoDynamicFPSLowThreshold + 5*1024*1024);
			value = (int32_t)mVideoDynamicFPSLowThreshold + 5*1024*1024;
		}		
		mVideoDynamicFPSHighThreshold = value;
	}
	
	ALOGD("@@[RECORD_PROPERTY]quality.adjust.enable[temp] = %d", mEnableForTemperature);
	ALOGD("@@[RECORD_PROPERTY]quality.adjust.enable[mem] = %d", mEnableForMemory);
	ALOGD("@@[RECORD_PROPERTY]auto.fps.drop.rate = %d", mVideoDynamicAutoFPSDropRate);
	ALOGD("@@[RECORD_PROPERTY]bitrate.low.percentage = %d", mVideoBitrateLowPercentage);
	ALOGD("@@[RECORD_PROPERTY]bitrate.low.threshold = %lld", mVideoDynamicBitrateLowThreshold);
	ALOGD("@@[RECORD_PROPERTY]bitrate.high.threshold = %lld", mVideoDynamicBitrateHighThreshold);
	ALOGD("@@[RECORD_PROPERTY]fps.low.threshold = %lld", mVideoDynamicFPSLowThreshold);
	ALOGD("@@[RECORD_PROPERTY]fps.high.threshold = %lld", mVideoDynamicFPSHighThreshold);
	
	ALOGD("-");
	return;
}

void VideoQualityController::adjustQualityIfNeed(const int64_t memory_size) {
	ALOGD("+");

	if(mEnableForMemory) {
		adjustForMemory(memory_size);
	}

	if(mEnableForTemperature) {
		adjustForTemperature();
	}
	ALOGD("-");

	return;
}

void VideoQualityController::adjustForMemory(const int64_t memory_size) {
	
	ALOGD("memory_size(%lldKB), mBitrateAdjusted(%d), mFPSAdjustedForMem(%d) +", memory_size/1024, mBitrateAdjusted, mFPSAdjustedForMem);
	sp<OMXCodec> omx_enc =reinterpret_cast<OMXCodec *>(mSource.get()); 
	//non-auto mode
	//Adjust bitrate
	if (!mBitrateAdjusted && (memory_size > mVideoDynamicBitrateHighThreshold)) {	
		int64_t target_bitrate = ((int64_t)mVideoBitRate) * mVideoBitrateLowPercentage / 100;
		
		status_t err = omx_enc->vEncSetBitRate((uint32_t)target_bitrate);
		if (err == OK) {
			mOwner->notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_BITRATE_ADJUSTED, 0);
		}
		ALOGD("set bitrate to %lld return %d", target_bitrate, err);
		mBitrateAdjusted = true;
	}
	
	//Adjust fps
	if (!mFPSAdjustedForMem && (memory_size > mVideoDynamicFPSHighThreshold)) {
		int32_t target_fps = (int32_t)((float)(mVideoInitFPS * mVideoDynamicAutoFPSDropRate)/100 + 0.5);
		
		if(mFPSAdjustedForTemp) {
			ALOGI("-  Fps has been adjusted for temperature, ignore for memory");
			return;
		}
		
		status_t err = reinterpret_cast<CameraSource *>((omx_enc->getSource()).get())
			->setFrameRate(target_fps);
		if (err == OK) {
			mOwner->notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_FPS_ADJUSTED, 0);
		}
		ALOGD("set fps(%d) return %d", target_fps, err);
		mFPSAdjustedForMem = true;
	}
	
	ALOGD("-");
	return;
}
void VideoQualityController::adjustForTemperature() {
	ALOGD("mFPSAdjustedForTemp(%d) +", mFPSAdjustedForTemp);
	// not implement, directly return	
	ALOGD("-");
	return;
}

}

