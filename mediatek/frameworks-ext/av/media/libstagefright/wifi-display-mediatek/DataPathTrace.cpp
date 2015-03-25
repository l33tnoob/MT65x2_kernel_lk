#undef LOG_TAG
#define LOG_TAG "WfdDebugInfo"
#include <utils/Log.h>
#include "DataPathTrace.h"
#include <utils/Mutex.h>
#include <media/stagefright/foundation/ADebug.h>
#include <cutils/properties.h>

namespace android {

sp<WfdDebugInfo> gDefaultWfdDebugInfo = NULL;

Mutex gDefaultWfdDebugInfoLock;


int64_t getTickCountMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec*1000LL + tv.tv_usec/1000);
}



sp<WfdDebugInfo> defaultWfdDebugInfo()
{
    if (gDefaultWfdDebugInfo != NULL){
		
		return gDefaultWfdDebugInfo;

    }
    
    {
        AutoMutex _l(gDefaultWfdDebugInfoLock);
        if (gDefaultWfdDebugInfo == NULL) {
            gDefaultWfdDebugInfo = new WfdDebugInfo;
        }
    }
    
    return gDefaultWfdDebugInfo;
}

void deleteWfdDebugInfo()
{
	AutoMutex _l(gDefaultWfdDebugInfoLock);
	ALOGI("deleteWfdDebugInfo %p",gDefaultWfdDebugInfo.get());
	if (gDefaultWfdDebugInfo != NULL)   {
		gDefaultWfdDebugInfo.clear();
		gDefaultWfdDebugInfo = NULL;
	}
}

WfdDebugInfo::WfdDebugInfo(){
	mLogEnable = false;
	mLogEnablePrint=true;
	mIsEnable = true;
	mLastPrintTime[0] = 0;
	mLastPrintTime[1] = 0;
	
     memset(&mLatency[0],0,sizeof(LatencyProfile));
     memset(&mLatency[1],0,sizeof(LatencyProfile));
	
    char value[PROPERTY_VALUE_MAX];
    if (property_get("wfd.debug.log.info", value, NULL) 
	    && (!strcmp(value, "1") || !strcasecmp(value, "true"))) {	    
           ALOGI("open debug log info");
	    mLogEnable = true;
    }  
    if (property_get("wfd.debug.log.print", value, NULL) ) {
	  mLogEnablePrint = atoi(value);
           ALOGI("print debug log.print =%d",mLogEnablePrint);
    }  

    if (property_get("wfd.debug.enable", value, NULL) ) {
	  mIsEnable = atoi(value);
           ALOGI("print debug.enable =%d",mIsEnable);
    }  

   mLastPrintTime[0] =getTickCountMs();
   mLastPrintTime[1] =mLastPrintTime[0];
    ALOGI("WfdDebugInfo construct");

}
	
status_t WfdDebugInfo::addTimeInfoByKey(uint32_t  isVideo,int64_t key , const char* valueName, int64_t value)  {\
   
    //Mutex::Autolock lock(mLock);//in case multi-thread reenter
    if(!mIsEnable) return -1;
    if (mLock.tryLock() != NO_ERROR) return -1;
	  
	int32_t index =-1;
	index = mDebugInfoList[isVideo].indexOfKey(key) ;
	if(mLogEnable){
		ALOGI("addTimeInfoByKey:[%s] [key=%lld us]name=%s has %d infos,index=%d",
			isVideo?"video":"audio",key,valueName,mDebugInfoList[isVideo].size(),index);

	}

	if( index >= 0){//exist 
		
		sp<AMessage>   msg= mDebugInfoList[isVideo].editValueAt(index);		
		msg->setInt64(valueName, value);

	}else{	
		
		if(mDebugInfoList[isVideo].size() >= DEBUG_INFO_LIST_MAX_SIZE){//max store 50 infos, in case memory usage exceed 
			

			sp<AMessage>  msg = mDebugInfoList[isVideo].editValueAt(0) ;
			int32_t PrintedDone=0;
			if(msg->findInt32("PrintedDone", &PrintedDone) && PrintedDone == 0){	
				int64_t key = mDebugInfoList[isVideo].keyAt(0);
				ALOGW("addTimeInfoByKey:%s   timestamp =%lld  has not beed print before remove",
					isVideo?"video":"audio",key);
			}
			mDebugInfoList[isVideo].removeItemsAt(0);	
		}
		 /* 
		for(uint32_t i =0; i< mDebugInfoList[isVideo].size() ; i++){//remove used info
			sp<AMessage>  msg = mDebugInfoList[isVideo].editValueAt(i) ;
			int32_t PrintedDone=0;
			if(msg->findInt32("PrintedDone", &PrintedDone) && PrintedDone == 1){	
				mDebugInfoList[isVideo].removeItemsAt(i);
			}
		}
		*/	
		sp<AMessage> msg = new AMessage;
		msg->setInt64(valueName, value);
		msg->setInt32("usedForCalc", 0) ;
		msg->setInt32("PrintedDone", 0) ;
		mDebugInfoList[isVideo].add(key,msg);

		if(mLogEnable){
			index = mDebugInfoList[isVideo].indexOfKey(key) ;
			ALOGI("addTimeInfoByKey new+++:[%s] [key=%lld us]name=%s has %d infos,index=%d",
				isVideo?"video":"audio",key,valueName,mDebugInfoList[isVideo].size(),index);
		}
	}

	
	 if (!strncmp(valueName, "Latency", 7)) {
	 	mLatency[isVideo].totalCnt++;
		mLatency[isVideo].totalValue += value;
		mLatency[isVideo].avgValue =mLatency[isVideo].totalValue / mLatency[isVideo].totalCnt;
		if(value > mLatency[isVideo].maxValue ) {
			mLatency[isVideo].maxValue = value;
			mLatency[isVideo].maxVideoMappedKey = key;
		}
		mLatency[isVideo].validNow = true;
	 }

	  if (!strncmp(valueName, "SentFps", 7)) {
	 	mLatency[isVideo].sentFps = value;
	 }
	  
	 ALOGV("addTimeInfoByKey :[%s] [key=%lld us] %s,max=%lld  avg %lld ",
				isVideo?"video":"audio",key,valueName,mLatency[isVideo].maxValue ,mLatency[isVideo].avgValue);

	mLock.unlock();
	 
	return OK;	

}
int64_t WfdDebugInfo::getTimeInfoByKey(uint32_t isVideo,int64_t key,const char* valueName)  {
	 if(!mIsEnable) return -1;
	if (mLock.tryLock() != NO_ERROR) return -1;
	
	int32_t index =0;
	int64_t value;
	index = mDebugInfoList[isVideo].indexOfKey(key) ;
	if(mLogEnable){
		ALOGI("getTimeInfoByKey: [%s]find key=%lld, valueName=%s,index=%d  ",isVideo?"video":"audio",key,valueName,index);
	}
	if( index >= 0){//exist 
		sp<AMessage> msg  = mDebugInfoList[isVideo].editValueAt(index);
		if(!msg->findInt64(valueName, &value)){
			ALOGV("getTimeInfoByKey: %s %lld not find the %s£¬size=%d ",isVideo?"video":"audio",key ,valueName, mDebugInfoList[isVideo].size());
			mLock.unlock();
			return -1 ;
		}
		if (!strncmp(valueName, "StOt", 4)) {
			msg->setInt32("usedForCalc", 1) ;
		}
		mLock.unlock();
		return value;

	}
	mLock.unlock();
	return -1;
}
void WfdDebugInfo::printDebugInfoByKey(uint32_t isVideo,int64_t key)  {
	 if(!mIsEnable) return;
	if (mLock.tryLock() != NO_ERROR) return;
	
	int32_t index =0;
	index = mDebugInfoList[isVideo].indexOfKey(key) ;
	if( index >= 0){//exist 
		sp<AMessage> msg  = mDebugInfoList[isVideo].editValueAt(index);	 
		
		if(mLogEnablePrint){
			int64_t nowMs = getTickCountMs();
			if(nowMs - mLastPrintTime[isVideo]  > 4000){
			        ALOGD("[%s]total info items now =%d,key=%lld us,index=%d, info is '%s'", 
						isVideo?"video":"audio",				
						mDebugInfoList[isVideo].size(),
						key,index,
						msg->debugString(0).c_str());
					
				mLastPrintTime[isVideo] = nowMs;
			}
			
		}
		
		msg->setInt32("PrintedDone", 1) ;
		mDebugInfoList[isVideo].removeItemsAt(index);
			
	}
	mLock.unlock();
	  
}

status_t   WfdDebugInfo::getStatistics(uint32_t isVideo,int64_t *avgLatencyValue,int64_t *maxLatencyValue,
     							int32_t *fps, int64_t *maxValueMappedKeyTime )   {
		if(!mIsEnable) return -1;
		if (mLock.tryLock() != NO_ERROR) return -1;
		
		if(mLatency[isVideo].validNow ){
				*avgLatencyValue = mLatency[isVideo].avgValue;
				*maxLatencyValue = mLatency[isVideo].maxValue;
				*fps = mLatency[isVideo].sentFps;
				*maxValueMappedKeyTime = mLatency[isVideo].maxVideoMappedKey;
				mLock.unlock();
				return OK;	
		}
		mLock.unlock();
		return -EINVAL;
 }

WfdDebugInfo::~WfdDebugInfo(){

	for(int32_t index =0; index <2 ;index++){
		for(uint32_t i =0; i< mDebugInfoList[index].size() ; i++){
			if(mDebugInfoList[index].editValueAt(i) != NULL){
				mDebugInfoList[index].removeItemsAt(i);
			}
		}
		mDebugInfoList[index].clear();
	}
	 ALOGI("WfdDebugInfo de-construct");
}

}

