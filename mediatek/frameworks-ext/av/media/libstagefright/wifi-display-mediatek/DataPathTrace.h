#ifndef _WFD_DEBUG_H_
#define  _WFD_DEBUG_H_
#include <media/stagefright/foundation/ABase.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <media/stagefright/foundation/AMessage.h>
namespace android {

#define DEBUG_INFO_LIST_MAX_SIZE   200
/*

key <--> char map{		
	"RpIn"  ==>repeaterSource read buffer from surface mediaSource done
	"RpDisPlay" ==>this buffer has beed repeated display
	"RpOt"  ==>repeaterSource read function, read buffer done
	"DeMs"  ==>  RpOt - RgtB  =>buffer delay in repeaterSource
	"MpIn" ==>  MediaPuller read function, read buffer done
	"CvIn" ==>  converter got the buffer
	"EnIn"==>  BUffer go Into ACodec
	"MsIn"==>  BUffer out from ACodec
	"HdCp"==>  hdcp encrypt time  
	"NeTc" ==> NetTimsUs (only this is ms, other will be all in MS)
	"StOt ==>sender send rtp packet done
	 "Latency" ==> source latency
	 "Delay"

	};

key==>timestamp
value==>timeInfo
value in AMessage is index by its valueName
*/

struct WfdDebugInfo: public RefBase {

      
	struct LatencyProfile{
		int64_t totalCnt;
		int64_t totalValue;
		int64_t avgValue;
		int64_t maxValue;
		int64_t maxVideoMappedKey;
		int32_t sentFps;
		bool validNow;
	};
	
	KeyedVector<int64_t, sp<AMessage> > mDebugInfoList[2];

      WfdDebugInfo();

   
    status_t addTimeInfoByKey(uint32_t  isVideo,int64_t key , const char* valueName, int64_t value)  ;
    int64_t getTimeInfoByKey(uint32_t isVideo,int64_t key,const char*  valueName)  ;
    void  printDebugInfoByKey(uint32_t isVideo,int64_t key)  ;
 
  
   // if reture status_t value is not OK, get value is not valid (at the beginning phase)   //
    status_t   getStatistics(uint32_t isVideo,int64_t *avgLatencyValue,int64_t *maxLatencyValue,
     							int32_t *fps, int64_t *maxValueMappedKeyTime ) ;
	
protected:
    virtual ~WfdDebugInfo();
private:
      mutable     Mutex     mLock;
	bool   mLogEnable; 
	bool   mLogEnablePrint; 
	bool   mIsEnable;
	int64_t mLastPrintTime[2];
	LatencyProfile  mLatency[2];
	
	status_t  updataTimeInfo(sp<AMessage> msg, const char* valueName, int64_t value) ;
	int64_t    getTimeInfo(sp<AMessage> msg, const char* valueName) ;

};

sp<WfdDebugInfo> defaultWfdDebugInfo();
void deleteWfdDebugInfo();


}

#endif

