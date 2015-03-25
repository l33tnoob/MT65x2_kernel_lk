/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/******************************************************
*created by mingliangzhong(mtk80309)@2010-08-30
*modify history:
*	1, add three MetaData corresponding to videodecode
*                 ,haizhenwang(mtk80691)@2010-10-20
*       2, add  two MetaData corresponding to videoencode
                  ,haizhenwang(mtk80691)@2010-10-29
*
******************************************************/

#ifndef _MEDIA_TYPE_DEF_H_
#define _MEDIA_TYPE_DEF_H_
#include "compile_ops.hpp"
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>


#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <binder/MemoryDealer.h>
#include <binder/MemoryBase.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/Log.h>
#include <utils/Singleton.h>
#include <utils/String8.h>
#include <utils/Errors.h>


//#include <gui/ISurface.h>
#include <camera/ICamera.h>
#include <camera/Camera.h>



using namespace android;	
using android::INVALID_OPERATION;
using android::NO_INIT;
using android::OK;
using android::NO_MEMORY;
using android::INVALID_OPERATION;
using android::BAD_VALUE;
using android::UNKNOWN_ERROR;
using android::ALREADY_EXISTS;
//using android::ISurface;
using android::SortedVector;
using android::sp;
using android::wp;
using android::status_t;
using android::String8;
using android::KeyedVector;
using android::MemoryBase;

namespace videotelephone
{
	
//VT MIMETYPE	
extern const char *MEDIA_MIMETYPE_VIDEO_ALL;	
extern const char *MEDIA_MIMETYPE_AUDIO_ALL ;
extern const char *MEDIA_MIMETYPE_IMAGE_JPEG;
extern const char *MEDIA_MIMETYPE_VIDEO_MPEG4;
extern const char *MEDIA_MIMETYPE_VIDEO_H263;
extern const char *MEDIA_MIMETYPE_VIDEO_YUV420;
extern const char *MEDIA_MIMETYPE_VIDEO_AVC;

extern const char *MEDIA_MIMETYPE_AUDIO_AMR_NB;
extern const char *MEDIA_MIMETYPE_AUDIO_AMR_WB;
extern const char *MEDIA_MIMETYPE_AUDIO_RAW;

//error return values define
//#define VTMAL_OK 0
//#define VTMAL_FAIL -1
//#define VTMAL_E_POINTER -2

// Audio frame number for each transfer
#define FRAME_NUMBER_TRANSFORM 1
#define MEDIA_FRAME_SIZE_AUDIO 32
enum MediaType_Category{MTC_INPUT,
	                              MTC_OUTPUT,MTC_UNKNOWN};


// The following keys map to int32_t data unless indicated otherwise.
enum {
    kKeyMIMEType          = 'mime',  // cstring
    kKeyWidth             = 'widt',
    kKeyHeight            = 'heig',
    kKeyChannelCount      = '#chn',
    kKeySampleRate        = 'srte',
    kKeyBitRate           = 'brte',  // int32_t (bps)
    kKeyESDS              = 'esds',  // raw data
    kKeyAVCC              = 'avcc',  // raw data
    kKeyWantsNALFragments = 'NALf',
    kKeyIsSyncFrame       = 'sync',  // int32_t (bool)
    kKeyIsKeyFrame     = 'keyf',  // int32_t (bool)
    kKeyTime              = 'time',  // int64_t (usecs)
    kKeyDuration          = 'dura',  // int64_t (usecs)
    kKeyColorFormat       = 'colf',
    kKeyBufferID          = 'bfID',
    kKeyMaxInputSize      = 'inpS',   
    kKeyEndOfStream		= 'eosk',	//in32
    kKeyMediaSampleSize = 'kmss',  //in32 buffer size bytes
    kKeyMediaBufferCount = 'mbCt',////in32 buffer count


//add by mtk80691
	kKeyIsVOS            ='VOSb',  //uint32_t(bool)
    kKeyGetVOSlen           ='VOSl', //uint32_t
	//kKeyFramelen         ='fram', //uint32_t
	//kKeyForceIFrame      ='ForI',

	kKeyPacketNumber	 ='PktN', //uint32_t
	kKeyVpInfo           ='VpIf',// pointer

	kKeyCurSensor		='curS',  // int32
	kKeyCurSensorMode   ='cuSM',  //int32
	kKeyCurSensorFrameRate = 'CSFR',//int32
    
	//add for record video and audio
	kKeyIsCodecConfig     = 'conf',  // int32_t (bool)
    kKeyCRCError = 'crce',  // int32_t (bool)
    kKeyPacketLost = 'PakL' //int32_t (bool)
};


enum {
    kTypeESDS        = 'esds',
    kTypeAVCC        = 'avcc'
};


class MetaData:public RefBase  //reference MetaData in Stagefright
{
public:
	MetaData();
	
	MetaData(const MetaData &from);
	
	virtual ~MetaData();
	
	int GetCategory() const
	{
		return m_iCategory;
	}
	
	void  SetCategory(const int Category)
	{
		m_iCategory = Category;
	}


 enum Type {
        TYPE_NONE     = 'none',
        TYPE_C_STRING = 'cstr',
        TYPE_INT32    = 'in32',
        TYPE_INT64    = 'in64',
        TYPE_FLOAT    = 'floa',
        TYPE_POINTER  = 'ptr ',
    };

    void clear();
	
    bool remove(uint32_t key);

    bool setCString(uint32_t key, const char *value);
	
    bool setInt32(uint32_t key, int32_t value);
	
    bool setInt64(uint32_t key, int64_t value);
	
    bool setFloat(uint32_t key, float value);
	
    bool setPointer(uint32_t key, void *value);

    bool findCString(uint32_t key, const char **value);
	
    bool findInt32(uint32_t key, int32_t *value);
	
    bool findInt64(uint32_t key, int64_t *value);
	
    bool findFloat(uint32_t key, float *value);
	
    bool findPointer(uint32_t key, void **value);

    bool setData(uint32_t key, uint32_t type, const void *data, size_t size);

    bool findData(uint32_t key, uint32_t *type,
                  const void **data, size_t *size) const;

private:

	 int m_iCategory;
    struct typed_data {
        typed_data();
        ~typed_data();

        typed_data(const MetaData::typed_data &);
		
        typed_data &operator=(const MetaData::typed_data &);

        void clear();
		
        void setData(uint32_t type, const void *data, size_t size);
		
        void getData(uint32_t *type, const void **data, size_t *size) const;

    private:
		
        uint32_t mType;
		
        size_t mSize;

        union {
            void *ext_data;
            float reservoir;
        } u;

        bool usesReservoir() const {
            return mSize <= sizeof(u.reservoir);
        }

        void allocateStorage(size_t size);
		
        void freeStorage();

        void *storage() {
            return usesReservoir() ? &u.reservoir : u.ext_data;
        }

        const void *storage() const {
            return usesReservoir() ? &u.reservoir : u.ext_data;
        }
    };

    KeyedVector<uint32_t, typed_data> mItems;

		
};

class IRefClock:public RefBase
{
public:
	IRefClock(){}
	virtual ~IRefClock(){}
	virtual int64_t getRealTimeUs() = 0;
};



class CSystemTime:public IRefClock
{
public:
    CSystemTime();
	virtual ~ CSystemTime(){}

    virtual int64_t getRealTimeUs();

private:
    static int64_t GetSystemTimeUs();

    int64_t mStartTimeUs;
};

class LockEx 
{
public:
	LockEx();

	~LockEx();


	// lock or unlock the mutex
	inline status_t    lock()
	{
	     if (mLockThreadId != getThreadId()) 
	    {
			mLock.lock();	 	
			mLockThreadId = getThreadId();
			mlocked = 0;
	    }
	    return mlocked++;
	}


	inline void    unlock()
	{
	    if ((mlocked >0)  && (mLockThreadId == getThreadId())) 
	    {
	         mlocked --;
		 if( mlocked <=0 )
	 	{
	 		mlocked = 0;
			mLockThreadId = 0;
			mLock.unlock();	 	  
	 	}
	    }
	}


    // lock if possible; returns 0 on success, error otherwise
    status_t    tryLock();

private:
	int mlocked;
	thread_id_t                 mLockThreadId;
	Mutex mLock;
};

    class AutolockEx {
    public:
	   inline AutolockEx(LockEx& mutex) : mLock(mutex)  { mLock.lock(); }
        inline AutolockEx(LockEx* mutex) : mLock(*mutex) { mLock.lock(); }
        inline ~AutolockEx() { mLock.unlock(); }
    private:
        LockEx& mLock;
    };

//#define VTMAL_LOGI(x, ...) LOGI("[%s] "x, mComponentName, ##__VA_ARGS__)
//#define VTMAL_LOGV(x, ...) LOGV("[%s] "x, mComponentName, ##__VA_ARGS__)
//#define VTMAL_LOGE(x, ...) LOGE("[%s] "x, mComponentName, ##__VA_ARGS__)



//using namespace videotelephone;
}



#endif //_MEDIA_TYPE_DEF_H_
