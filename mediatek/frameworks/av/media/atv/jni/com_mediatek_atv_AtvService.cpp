/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "ATV/ATVJNI"
#include <cutils/xlog.h>
//#include <string>


#include <gui/Surface.h>
#include <gui/IGraphicBufferProducer.h>

#include <utils/Vector.h>
#include <camera/ICameraService.h>
#include <camera/Camera.h>
#include <binder/IMemory.h>
#include <media/mediarecorder.h>

#include <media/ATVCtrl.h>

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"


using namespace android;

struct fields_t {
    jfieldID    recorder;
    jfieldID    surface;	
    jfieldID    context;
    jmethodID   post_event; 
    
};

static fields_t fields;
static Mutex sLock;

#define MTS_MONO		0x00000001
#define MTS_STEREO		0x00000002
#define MTS_SAP 		0x00000004

/* Japan & Korea MPX */
#define MPX_MONO		0x00000008
#define MPX_STEREO		0x00000010
#define MPX_SUB 		0x00000020
#define MPX_MAIN_SUB	0x00000040
#define MPX_MAIN		0x00000080

#define FM_MONO 		0x00000100
#define A2_STEREO		0x00000200
#define A2_DUAL1		0x00000400
#define A2_DUAL2		0x00000800
#define NICAM_MONO		0x00001000
#define NICAM_STEREO	0x00002000
#define NICAM_DUAL1 	0x00004000
#define NICAM_DUAL2 	0x00008000

#define FMRDO_MONO		0x00010000
#define FMRDO_STEREO	0x00020000

#define FORMAT_DETECTING 0x00000000


#define AUDIO_MASK_MTS      (MTS_MONO | MTS_STEREO | MTS_SAP)
#define AUDIO_MASK_MPX      (MPX_MONO | MPX_STEREO | MPX_SUB | MPX_MAIN_SUB |MPX_MAIN)
#define AUDIO_MASK_FM       (FM_MONO)
#define AUDIO_MASK_A2       (A2_STEREO | A2_DUAL1 | A2_DUAL2)
#define AUDIO_MASK_NICAM    (NICAM_MONO | NICAM_STEREO | NICAM_DUAL1 | NICAM_DUAL2)
#define AUDIO_MASK_FMRDO    (FMRDO_MONO | FMRDO_STEREO)

typedef struct 
{
	kal_uint8		mode;//scan mode (auto/full/quick)
	kal_bool		is_scanning;//drv is under scanning state
	kal_uint8		ch_latest_updated;//the real channel number of current entry being updated,comparing to "ch" which is just a counter.
	matv_ch_entry		updated_entry;
} matv_chscan_state;


typedef enum 
{
	MATV_AUTOSCAN,
	MATV_FULLSCAN,
	MATV_QUICKSCAN
} atv_chscan_mode;

typedef enum 
{
	ATV_SCAN_PROGRESS = 0xf0002001,
	ATV_SCAN_FINISH,
	ATV_AUDIO_FORMAT_UPDATE,	
	ATV_FILL_PARAMETER,
	ATV_CHIP_SHUTDOWN	
} atv_scan_event;

enum
{
	// item < 100, common info provide by chip
	SIG_RSSI=0,
	SIG_SNR,
	SIG_STRENGTH
		
	/* item >= 100, variant info, not fixed
	 * example
	 * MTK_GAIN0=100,
	 * MTK_GAIN1=101 
	 */	
};

// TP Control parameter
#define TP_SINGLE     0		//Touch panel scan in single line mode
#define TP_MULTIPLE 1            //Touch panel scan in multiple line mode
#define TP_SLEEP       2            //Touch panel go to sleep
#define TP_RESUME    3            //Resume from sleep
// #define ATV_LOG_OUT 1

// Enable TP Control
#define ENABLE_TP_CTRL

// provides persistent context for calls from native code to Java
class JNIAtvContext: public CameraListener,public ATVListener
{
public:
    JNIAtvContext(JNIEnv* env, jobject weak_this, jclass clazz);
    ~JNIAtvContext() { release(); }
    virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);
    virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr,
                          camera_frame_metadata_t *metadata);
    virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);
    sp<Camera> getCamera() { Mutex::Autolock _l(mLock); return mCamera; }
    void setCamera(sp<Camera> c) { Mutex::Autolock _l(mLock); mCamera = c; }    
    void release();
    void releaseCamera();	
    void autoscan_progress_chstate_cb( kal_uint8 precent,kal_uint8 ch,kal_uint8 chnum, void *ptr, int len);
    void scanfinish_cb( kal_uint8 chnum);
	void audioformat_cb( kal_uint32 format);	
	void shutdown_cb( kal_uint32 source);
    void fullscan_progress_cb(kal_uint8 precent,kal_uint32 freq,kal_uint32 freq_start,kal_uint32 freq_end);
#ifdef ENABLE_TP_CTRL
	static int setTparamThread(void*);
#endif

private:
    void copyAndPost(JNIEnv* env, const sp<IMemory>& dataPtr, int msgType);

    jobject     mAtvJObjectWeak;     // weak reference to java object
    jclass      mAtvJClass;          // strong reference to java class
    sp<Camera>  mCamera;                // strong reference to native object  
    Mutex       mLock;

	bool mAudioCallbackEnabled; 			 // Whether to use application managed buffers.
	
};

//sp<JNIAtvContext> context;//we don't need to store the context pointer in multiple java object because only on instance of mATV should exist at any given time.

//jobject JNIAtvContext::mAtvJObjectWeak;     // weak reference to java object
//jclass JNIAtvContext::mAtvJClass;          // strong reference to java class


sp<Camera> get_native_camera(JNIEnv *env, jobject thiz, JNIAtvContext** pContext)
{
	#ifdef ATV_LOG_OUT
    XLOGD("get_native_camera in");
	#endif
    sp<Camera> camera;
    Mutex::Autolock _l(sLock);
    JNIAtvContext* context = reinterpret_cast<JNIAtvContext*>(env->GetIntField(thiz, fields.context));

	if (pContext != NULL) { 
		//just get the context
		*pContext = context;
		return NULL;
	}
	
    if (context != NULL) {
		#ifdef ATV_LOG_OUT
    	XLOGD("context->getCamera in");
		#endif
        camera = context->getCamera();
		#ifdef ATV_LOG_OUT
    	XLOGD("context->getCamera out");
		#endif
    }
    XLOGV("get_native_camera: context=%p, camera=%p", context, camera.get());
    if (camera == 0) {
		XLOGE("get_native_camera: Method called after release()", context, camera.get());
    }  
	#ifdef ATV_LOG_OUT
    XLOGD("get_native_camera out");
	#endif
    return camera;
}


JNIAtvContext::JNIAtvContext(JNIEnv* env, jobject weak_this, jclass clazz)
{
    mAtvJObjectWeak = env->NewGlobalRef(weak_this);
    mAtvJClass = (jclass)env->NewGlobalRef(clazz);
	mAudioCallbackEnabled = false;
}

void JNIAtvContext::release()
{
    XLOGD("release");
    Mutex::Autolock _l(mLock);
    JNIEnv *env = AndroidRuntime::getJNIEnv();

    if (mAtvJObjectWeak != NULL) {
        env->DeleteGlobalRef(mAtvJObjectWeak);
        mAtvJObjectWeak = NULL;
    }
    if (mAtvJClass != NULL) {
        env->DeleteGlobalRef(mAtvJClass);
        mAtvJClass = NULL;
    }
}

void JNIAtvContext::releaseCamera()
{
    XLOGD("releaseVideo");
    mCamera.clear();
}


void JNIAtvContext::notify(int32_t msgType, int32_t ext1, int32_t ext2)
{
    XLOGD("notify");

    // VM pointer will be NULL if object is released
    Mutex::Autolock _l(mLock);
    if (mAtvJObjectWeak == NULL) {
        XLOGW("callback on dead atv object");
        return;
    }
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->CallStaticVoidMethod(mAtvJClass, fields.post_event,
            mAtvJObjectWeak, msgType, ext1, (jlong)ext2, NULL);
}

void JNIAtvContext::copyAndPost(JNIEnv* env, const sp<IMemory>& dataPtr, int msgType)
{
    jbyteArray obj = NULL;

    // allocate Java byte array and copy data
    if (dataPtr != NULL) {
        ssize_t offset;
        size_t size;
        sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
        XLOGD("postData: off=%d, size=%d", (int)offset, size);
        uint8_t *heapBase = (uint8_t*)heap->base();

        if (heapBase != NULL) {
            const jbyte* data = reinterpret_cast<const jbyte*>(heapBase + offset);
            obj = env->NewByteArray(size);
            XLOGD("Allocating callback buffer data[10] = %d,obj = %d",data[10],(int)obj);

            if (obj == NULL) {
                XLOGE("Couldn't allocate byte array for JPEG data");
                env->ExceptionClear();
            } else {
                env->SetByteArrayRegion(obj, 0, size, data);
            }
        } else {
            XLOGE("image heap is NULL");
        }
    }

    // post image data to Java
    env->CallStaticVoidMethod(mAtvJClass, fields.post_event,
            mAtvJObjectWeak, msgType, 0, (jlong)0, obj);
    if (obj) {
        env->DeleteLocalRef(obj);
    }
}

void JNIAtvContext::postData(int32_t msgType, const sp<IMemory>& dataPtr,
                          camera_frame_metadata_t *metadata)
{
    // VM pointer will be NULL if object is released
    Mutex::Autolock _l(mLock);
    if (mAtvJObjectWeak == NULL) {
        XLOGW("callback on dead atv object");
        return;
    }
    
    JNIEnv *env = AndroidRuntime::getJNIEnv();

    XLOGD("dataCallback(%d, %p)", msgType, dataPtr.get());
    copyAndPost(env, dataPtr, msgType);
}

void JNIAtvContext:: postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr){

}

void JNIAtvContext::audioformat_cb( kal_uint32 format)
{
    XLOGD("audioformat_cb");
     
	 // VM pointer will be NULL if object is released
	 Mutex::Autolock _l(mLock);
	 if (mAtvJObjectWeak == NULL) {
		 XLOGW("callback on dead atv object");
	} else {	 
	    JNIEnv *env = AndroidRuntime::getJNIEnv();    
	    env->CallStaticVoidMethod(mAtvJClass, fields.post_event,
	            	mAtvJObjectWeak, ATV_AUDIO_FORMAT_UPDATE, format,(jlong)0,NULL);     
	}

}


/*called during auto scanning to notify the progress.
  * This callback should be implemented as static because JNIAtvContext object may be freed before it is called in the thread created for serving the request
  * from ATVCtrl.We don't need to do so on camera callbacks because we will set a listener at the beginning which prevent JNIAtvContext from being freed.
  */ 

void JNIAtvContext::autoscan_progress_chstate_cb( kal_uint8 precent,kal_uint8 ch,kal_uint8 chnum,void *ptr, int len)
{
    XLOGD("autoscan_progress_chstate_cb");

	 // VM pointer will be NULL if object is released
	Mutex::Autolock _l(mLock);
	if (mAtvJObjectWeak == NULL) {
		 XLOGW("callback on dead atv object");
	} else 	{
	    JNIEnv *env = AndroidRuntime::getJNIEnv();    
		matv_chscan_state* state = (matv_chscan_state*)ptr; 
	    matv_ch_entry* entry = &state->updated_entry;    
	    jlong packedEntry = 0;
		XLOGD("ch_latest_updated:%d,freq:%d,sndsys:%d,colsys:%d,flag:%d,chnum:%d",state->ch_latest_updated,entry->freq,entry->sndsys,entry->colsys,entry->flag,chnum);
		if (entry->flag) {
	        	packedEntry = (jlong)entry->freq << 32 | (jlong)entry->sndsys << 16 | (jlong)entry->colsys << 8 | (jlong)entry->flag;
			
		}
		
	    env->CallStaticVoidMethod(mAtvJClass, fields.post_event,
	            mAtvJObjectWeak, ATV_SCAN_PROGRESS, chnum << 8 |state->ch_latest_updated,packedEntry,NULL);   
	}

}

void JNIAtvContext::fullscan_progress_cb(kal_uint8 precent,kal_uint32 freq,kal_uint32 freq_start,kal_uint32 freq_end) {
    XLOGE("fullscan_progress_cb is not implemented yet and should not be called");
}


// Added by HP Cheng to create a thread
// This thread is to call ATVC_matv_set_tparam in our callback to prevent deadlock in ATVCtrl,
// Because ATVC_matv_chscan_stop has the lock in ATVCtrl and calls the call back scanfinish_cb
// If we call ATVC_matv_set_tparam now, it also requires the lock in ATVCtrl, deadlock happens
#ifdef ENABLE_TP_CTRL
int JNIAtvContext::setTparamThread(void* p) {
    ATVCtrl::ATVC_matv_set_tparam(TP_MULTIPLE);
    return 0;
}
#endif

/*called on scanning finish.
  * This callback should be implemented as static because JNIAtvContext object may be freed before it is called in the thread created for serving the request
  * from ATVCtrl.We don't need to do so on camera callbacks because we will set a listener at the beginning which prevent JNIAtvContext from being freed.
  */ 

void JNIAtvContext::scanfinish_cb( kal_uint8 chnum)
{
    XLOGD("scanfinish_cb");

    // HP Cheng: Here means the scan is done, make TP multiple scan
#ifdef ENABLE_TP_CTRL
    //ATVCtrl::ATVC_matv_set_tparam(TP_MULTIPLE);
    createThreadEtc(setTparamThread, 0, "setTparamThread", ANDROID_PRIORITY_AUDIO);
#endif

	Mutex::Autolock _l(mLock);

	if (mAtvJObjectWeak == NULL) {
		 XLOGW("callback on dead atv object");
	} else 	{
	    JNIEnv *env = AndroidRuntime::getJNIEnv();
	    env->CallStaticVoidMethod(mAtvJClass, fields.post_event,
	            mAtvJObjectWeak, ATV_SCAN_FINISH, chnum,(jlong)0,NULL);         
    }

    
}

//in order to enter camera application successfully,driver will force shutdown tv chip.
void JNIAtvContext::shutdown_cb( kal_uint32 source)
{
	XLOGD("shutdown_cb");
	
	Mutex::Autolock _l(mLock);
	
	if (mAtvJObjectWeak == NULL) {
		XLOGW("callback on dead atv object");
	} else 	{
	    JNIEnv *env = AndroidRuntime::getJNIEnv();

		env->CallStaticVoidMethod(mAtvJClass, fields.post_event,
			mAtvJObjectWeak, ATV_CHIP_SHUTDOWN, source,(jlong)0,NULL); 	
	}
}


	/*Initializes TV chip*/
static void atv_setup(JNIEnv *env, jobject thiz,jobject weak_this)
{
    
    XLOGD("setup");

    jclass clazz = env->GetObjectClass(thiz);	
    if (clazz == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Can't find com/mediatek/atv/AtvService");
        return;
    }

    // We use a weak reference so the atv object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
    sp<JNIAtvContext> context = new JNIAtvContext(env, weak_this, clazz);
    context->incStrong(thiz);

    // save context in opaque field
    env->SetIntField(thiz, fields.context, (int)context.get());	

}

	/*Initializes TV chip*/
static void atv_init(JNIEnv *env, jobject thiz)
{
    
    XLOGD("atv_init");	
	int ret = ATVCtrl::ATVC_matv_init();
    if (ret != true) {		
		XLOGE("ATV init failed with error code %d",ret);	        
		jniThrowException(env, "java/io/IOException", "ATV init failed");
	} else {	
		JNIAtvContext* context;
		get_native_camera(env, thiz, &context);
		if (context == NULL) return;

		ATVCtrl::ATVC_matv_setListener(context);	
	}
}	

static void atv_shutdown(JNIEnv *env, jobject thiz,jboolean shutdownHardware)
{
    // TODO: Change to XLOGD
    XLOGD("shutdown");

	JNIAtvContext* context;
    {
        Mutex::Autolock _l(sLock);
        context = reinterpret_cast<JNIAtvContext*>(env->GetIntField(thiz, fields.context));

        // Make sure we do not attempt to callback on a deleted Java object.
        env->SetIntField(thiz, fields.context, 0);
    }

	if (context != NULL) {
		ATVCtrl::ATVC_matv_setListener(NULL);	
		
		if (shutdownHardware) {
			ATVCtrl::ATVC_matv_shutdown();
		}
			

		context->release();
		context->decStrong(thiz);
	}
}

static void atv_startVideo(JNIEnv *env, jobject thiz) {

	XLOGD("startVideo");
	
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

	/*
	//set sensor to matv., 1:main, 2:sub, 4:atv 
    if (camera->sendCommand(CAMERA_CMD_SET_SENSOR_DEV, 0x04, 0) != NO_ERROR) {
        jniThrowException(env, "java/lang/RuntimeException", "setSensorDev failed");
    }*/
	
	#ifdef ATV_LOG_OUT
        XLOGD("camera->startPreview in");
	#endif	
	if (camera->startPreview() != NO_ERROR) {
		jniThrowException(env, "java/lang/RuntimeException", "startVideo failed");
		return;
	}
	#ifdef ATV_LOG_OUT
        XLOGD("camera->startPreview out");
	#endif	
	
		// HP Cheng: Here means the open is success, make TP single scan
#ifdef ENABLE_TP_CTRL
	#ifdef ATV_LOG_OUT
        XLOGD("ATVCtrl::ATVC_matv_set_tparam in");
	#endif	
		ATVCtrl::ATVC_matv_set_tparam(TP_SINGLE);
	#ifdef ATV_LOG_OUT
        XLOGD("ATVCtrl::ATVC_matv_set_tparam out");
	#endif	
#endif
}


// connect to camera service
//not thread safe
static void atv_openVideo(JNIEnv *env, jobject thiz,jboolean connect)
{
    XLOGD("open Video in");
	char* parameters = NULL;
	sp<Camera> camera = NULL;
	
	//enable matv camera interface.
	#ifdef ATV_LOG_OUT
    XLOGD("ATVCtrl::ATVC_matv_suspend(0) in");
	#endif
        ATVCtrl::ATVC_matv_suspend(0);	
	#ifdef ATV_LOG_OUT
    XLOGD("ATVCtrl::ATVC_matv_suspend(0) out");
	#endif

	if (connect) {
		#ifdef ATV_LOG_OUT
    	XLOGD("connect true");
    	XLOGD("Camera::connect in");
		#endif
        Camera::setProperty(String8("client.appmode"),String8("MtkAtv"));
	    camera = Camera::connect(0x0,String16("com.mediatek.app.mtv"),Camera::USE_CALLING_UID);// 0xFF is the id for atv sensor.
		#ifdef ATV_LOG_OUT
    	XLOGD("Camera::connect out");
		#endif

	    if (camera == NULL) {
	        jniThrowException(env, "java/lang/RuntimeException",
	                          "Fail to connect to camera service in ATV");
	        return;
	    }

	    // make sure camera hardware is alive
	    if (camera->getStatus() != NO_ERROR) {
	        jniThrowException(env, "java/lang/RuntimeException", "Camera initialization failed in ATV");
	        return;
	    }
		
		JNIAtvContext* context;
		get_native_camera(env, thiz, &context);
		if (context == NULL) return;

	    camera->setListener(context);
	    context->setCamera(camera);
	} else {
		#ifdef ATV_LOG_OUT
    	XLOGD("connect false");
		#endif
		camera = get_native_camera(env, thiz, NULL);
	    if (camera == 0) return;
        #ifdef ATV_LOG_OUT
    	XLOGD("camera->setPreviewTexture in");
		#endif
		
		if (camera->setPreviewTarget((sp<IGraphicBufferProducer>)NULL) != NO_ERROR) {
			jniThrowException(env, "java/io/IOException", "setPreviewTexture failed");
		}
        #ifdef ATV_LOG_OUT
    	XLOGD("camera->setPreviewTexture out");
		#endif
	}
	
	#ifdef ATV_LOG_OUT
    XLOGD("open Video out");
	#endif
}




/*sh: the SurfaceHolder to use for displaying the video portion of the media.*/
static void atv_setPreviewDisplay(JNIEnv *env, jobject thiz, jobject jSurface)
{
    XLOGD("setPreviewTexture in");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    sp<Surface> surface = NULL;
    if (jSurface != NULL) {
        surface = reinterpret_cast<Surface*>(env->GetIntField(jSurface, fields.surface));
    }
	#ifdef ATV_LOG_OUT
    XLOGD("camera->setPreviewTexture(buffer) in");
	#endif
    //sp<IGraphicBufferProducer> buffer = surface->getIGraphicBufferProducer();
    if (camera->setPreviewTarget(surface->getIGraphicBufferProducer()) != NO_ERROR) {
        jniThrowException(env, "java/io/IOException", "setPreviewTexture failed");
    }
	#ifdef ATV_LOG_OUT
    XLOGD("camera->setPreviewTexture(buffer) out");
    XLOGD("setPreviewTexture out");
	#endif
}

//Fix me:we suffer to the change of MediaRecorder framework by implementing setCamera in ATV JNI.
static void atv_setRecorder(JNIEnv *env, jobject thiz,jobject mediarecorder)
{
    XLOGD("atv_setRecorder");
	
    sp<Camera> c = get_native_camera(env, thiz, NULL);	
    if (c == 0) return;

	MediaRecorder* const p = (MediaRecorder*)env->GetIntField(mediarecorder, fields.recorder);
    p->setCamera(c->remote(),c->getRecordingProxy());
}
static void atv_closeVideo(JNIEnv *env, jobject thiz,jboolean disconnect)
{
    XLOGD("closeVideo");

    // HP Cheng: Change TP to Muliple once we try to close the video
#ifdef ENABLE_TP_CTRL
    ATVCtrl::ATVC_matv_set_tparam(TP_MULTIPLE);
#endif

    sp<Camera> c = get_native_camera(env, thiz, NULL);
    if (c == 0) return;

    c->stopPreview();
	if (disconnect) {
		c->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_NOOP);
		c->disconnect();

		JNIAtvContext* context;
		get_native_camera(env, thiz, &context);
		if (context == NULL) return;
		
		context->releaseCamera();
	}
}

static bool atv_previewEnabled(JNIEnv *env, jobject thiz)
{
    XLOGD("previewEnabled");
    sp<Camera> c = get_native_camera(env, thiz, NULL);
    if (c == 0) return false;

    return c->previewEnabled();
}

static void atv_capture(JNIEnv *env, jobject thiz)
{
    XLOGV("capture");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->takePicture(CAMERA_MSG_COMPRESSED_IMAGE) != NO_ERROR) {
        jniThrowException(env, "java/lang/RuntimeException", "capture failed");
        return;
    }
}

/*
static void atv_setHasPreviewCallback(JNIEnv *env, jobject thiz, jboolean installed)
{
    XLOGD("setHasPreviewCallback: installed:%d", (int)installed);
    // Important: Only install preview_callback if the Java code has called
    // setPreviewCallback() with a non-null value, otherwise we'd pay to memcpy
    // each preview frame for nothing.
    JNIAtvContext* context;
    sp<Camera> camera = get_native_camera(env, thiz, &context);
    if (camera == 0) return;

    // setCallbackMode will take care of setting the context flags and calling
    // camera->setPreviewCallbackFlags within a mutex for us.
    context->setCallbackMode(env, installed);
}*/

static void atv_setParameters(JNIEnv *env, jobject thiz, jstring params)
{
    XLOGD("setParameters");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    const jchar* str = env->GetStringCritical(params, 0);
    String8 params8;
    if (params) {
        params8 = String8(str, env->GetStringLength(params));
        env->ReleaseStringCritical(params, str);
    }
    if (camera->setParameters(params8) != NO_ERROR) {
        jniThrowException(env, "java/lang/RuntimeException", "setParameters failed");
        return;
    }
}

static jstring atv_getParameters(JNIEnv *env, jobject thiz)
{
    XLOGD("getParameters");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return 0;

    return env->NewStringUTF(camera->getParameters().string());
}

static void atv_reconnect(JNIEnv *env, jobject thiz)
{
    XLOGD("reconnect");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->reconnect() != NO_ERROR) {
        jniThrowException(env, "java/io/IOException", "reconnect failed");
        return;
    }
}

static void atv_lock(JNIEnv *env, jobject thiz)
{
    XLOGD("lock");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->lock() != NO_ERROR) {
        jniThrowException(env, "java/lang/RuntimeException", "lock failed");
    }
}

static void atv_unlock(JNIEnv *env, jobject thiz)
{
    XLOGD("unlock");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->unlock() != NO_ERROR) {
        jniThrowException(env, "java/lang/RuntimeException", "unlock failed");
    }
}  
		
	/*Scan channel*/
static void atv_channelScan(JNIEnv *env, jobject thiz,jint mode,jint area_code)
{
    XLOGD("scan channel");
	
    if (mode!=MATV_AUTOSCAN){
        XLOGD("Scan mode=%d", mode);        
        jniThrowException(env, "java/lang/RuntimeException", "Scan mode not support yet"); 
        return;
    }


    //disable matv camera interface.
    ATVCtrl::ATVC_matv_suspend(1);

    ATVCtrl::ATVC_matv_set_country(area_code);
    ATVCtrl::ATVC_matv_chscan(mode);
    
    // HP Cheng: Here means the scan starting is success, make TP single scan
#ifdef ENABLE_TP_CTRL
    ATVCtrl::ATVC_matv_set_tparam(TP_SINGLE);
#endif
}
	
	/*Stop channel scan.*/
static void atv_stopChannelScan (JNIEnv *env, jobject thiz)
{
    
    XLOGD("stop scan");
    ATVCtrl::ATVC_matv_chscan_stop();    

// HP Cheng: Here means the scan stoping is success, make TP multiple scan
#ifdef ENABLE_TP_CTRL
    ATVCtrl::ATVC_matv_set_tparam(TP_MULTIPLE);
#endif
}   
	
	/*Get channel entry*/
static jlong atv_getChannelTable(JNIEnv *env, jobject thiz,jint ch)
{
    XLOGD("get channel table");

    matv_ch_entry entry;    
    jlong packedEntry;

    if (ATVCtrl::ATVC_matv_get_chtable(ch,&entry)){
        packedEntry = (jlong)entry.freq << 32 | (jlong)entry.sndsys << 16 | (jlong)entry.colsys << 8 | (jlong)entry.flag;
    }
    else {
        jniThrowException(env, "java/lang/RuntimeException", "Get Channel Table Fail"); 
        return 0;
    }

    return packedEntry;
}   
   

 /*Set channel entry.
    *Fixme: should implement setchanneltable more effiently because it may need to set a very large array.        
    */
static void atv_setChannelTable(JNIEnv *env, jobject thiz,jint ch,jlong packedEntry)
{
     

     matv_ch_entry entry;    

     entry.freq = (kal_uint32)(packedEntry >> 32)&0xffffffff;
     entry.sndsys = (kal_uint8)(packedEntry >> 16)&0xff;
     entry.colsys = (kal_uint8)(packedEntry >> 8)&0xff;
     entry.flag = (kal_uint8)packedEntry&0xff;
	 
	 XLOGD("set channel table packedEntry = %lld,freq = %d sndsys = %d colsys = %d flag = %d",packedEntry,entry.freq,entry.sndsys,entry.colsys,entry.flag);
	 
     if (!ATVCtrl::ATVC_matv_set_chtable(ch,&entry)){
         jniThrowException(env, "java/lang/RuntimeException", "Set Channel Table Fail"); 
     }
}   
	
	/*Clear channel table in driver.*/
static void atv_clearChannelTable(JNIEnv *env, jobject thiz)
{
    
    XLOGD("clear channel table");
    ATVCtrl::ATVC_matv_clear_chtable();
}   
	
	/*Change channel.*/
static void atv_changeChannel(JNIEnv *env, jobject thiz,jint ch)
{
    XLOGD("change channel in");
    ATVCtrl::ATVC_matv_change_channel(ch);
	#ifdef ATV_LOG_OUT
    XLOGD("change channel out");
	#endif
}   
	
	/*set location area code for scanning.*/
static void atv_setLocation(JNIEnv *env, jobject thiz,jint loc)  
{    
    XLOGD("set location");
    ATVCtrl::ATVC_matv_set_country(loc);
}   
	
	/*set chipdep.*/
static void atv_setChipDep(JNIEnv *env, jobject thiz,jint item,jint val)
{
    
    XLOGD("set chipdep");
    ATVCtrl::ATVC_matv_set_chipdep(item,val);
}	

	/*configure video/audio parameters.*/
static void atv_adjustSetting(JNIEnv *env, jobject thiz,jbyte item,jint val)
{
    
    XLOGD("adjust setting");
    ATVCtrl::ATVC_matv_adjust(item,val);
}	

static jint atv_getSignalStrength(JNIEnv *env, jobject thiz)
{
    
    XLOGD("atv_getSignalStrength");
    return (jint)ATVCtrl::ATVC_matv_get_chipdep(SIG_RSSI);
}	   

static void atv_setAudioCallback(JNIEnv *env, jobject thiz,jboolean set)
{
    XLOGD("atv_setAudioFormat");
} 


static jint atv_getAudioFormat(JNIEnv *env, jobject thiz)
{
    
    XLOGD("atv_getAudioFormat");
    return (jint)ATVCtrl::ATVC_matv_audio_get_format();
}	

static kal_uint32 setPreferAudioFormat(kal_uint32 supported_audio_mode)
{
    kal_uint32 prefer_audio_mode = 0xFFFFFFFF;
	
	
#define AUDIO_MASK_MPX      (MPX_MONO | MPX_STEREO | MPX_SUB | MPX_MAIN_SUB |MPX_MAIN)
    if (supported_audio_mode & AUDIO_MASK_MTS)
    {
        /* MTS SYSTEM */
        prefer_audio_mode = MTS_MONO;
    }
    else if (supported_audio_mode & AUDIO_MASK_NICAM)
    {
        /* NICAM, DUAL1 > STEREO > MONO */
        if (supported_audio_mode & NICAM_DUAL1)
        {
            prefer_audio_mode = NICAM_DUAL1;
        }
        else if (supported_audio_mode & NICAM_STEREO)
        {
            prefer_audio_mode = NICAM_STEREO;
        }
        else if (supported_audio_mode & NICAM_MONO)
        {
            prefer_audio_mode = NICAM_MONO;
        }
    }
    else if (supported_audio_mode & AUDIO_MASK_A2)
    {
        /* A2, DUAL1 > STEREO*/
        if (supported_audio_mode & A2_DUAL1)
        {
            prefer_audio_mode = A2_DUAL1;
        }
        else if (supported_audio_mode & A2_STEREO)
        {
            prefer_audio_mode = A2_STEREO;
        }
    }
    else if (supported_audio_mode & AUDIO_MASK_MPX)
    {
        /* NICAM, DUAL1 > STEREO > MONO */
        if (supported_audio_mode & MPX_STEREO)
        {
            prefer_audio_mode = MPX_STEREO;
        }
        else if (supported_audio_mode & MPX_MAIN_SUB)
        {
            prefer_audio_mode = MPX_MAIN_SUB;
        }
        else if (supported_audio_mode & MPX_MONO)
        {
            prefer_audio_mode = MPX_MONO;
        }
        else if (supported_audio_mode & MPX_MAIN)
        {
            prefer_audio_mode = MPX_MAIN;
        }
        else if (supported_audio_mode & MPX_SUB)
        {
            prefer_audio_mode = MPX_SUB;
        }		
    }	
    else if (supported_audio_mode & AUDIO_MASK_FM)
    {
        /* FM, MONO only (NICAM & A2 first) */
        if (supported_audio_mode & FM_MONO)
        {
            prefer_audio_mode = FM_MONO;
        }
    }
    else if (supported_audio_mode & AUDIO_MASK_FMRDO)
    {
        /* FMRDO, STEREO > MONO */
        if (supported_audio_mode & FMRDO_STEREO)
        {
            prefer_audio_mode = FMRDO_STEREO;
        }
        else if (supported_audio_mode & FMRDO_MONO)
        {
            prefer_audio_mode = FMRDO_MONO;
        }
    }
	XLOGD("atv_setPreferAudioFormat:%d",prefer_audio_mode);

	ATVCtrl::ATVC_matv_audio_set_format(prefer_audio_mode);
    return prefer_audio_mode;
}

static void atv_setAudioFormat(JNIEnv *env, jobject thiz,jint val)
{
    XLOGD("atv_setAudioFormat");
	//Mutex::Autolock _l(sLock);	
	if (val<0){		
		//context->setAutoAudioFormat(true);
		setPreferAudioFormat(ATVCtrl::ATVC_matv_audio_get_format());
	}
	else {
		//context->setAutoAudioFormat(false);		
    	ATVCtrl::ATVC_matv_audio_set_format(val);
	}
}   


//-------------------------------------------------

static JNINativeMethod atvMethods[] = {
  { "setup",
    "(Ljava/lang/Object;)V",
      (void*)atv_setup },
  { "init",
    "()V",
      (void*)atv_init },      
  { "_openVideo",
    "(Z)V",
    (void*)atv_openVideo },
  { "startVideo",
	"()V",
	(void*)atv_startVideo },	
  { "shutdown",
	"(Z)V",
    (void*)atv_shutdown},
  { "setSurface",
    "(Landroid/view/Surface;)V",
    (void *)atv_setPreviewDisplay },
  { "setRecorder",
    "(Landroid/media/MediaRecorder;)V",
    (void*)atv_setRecorder},    
  { "closeVideo",
    "(Z)V",
    (void *)atv_closeVideo },
  { "previewEnabled",
    "()Z",
    (void *)atv_previewEnabled },
  { "capture",
    "()V",
    (void *)atv_capture },
  { "native_setParameters",
    "(Ljava/lang/String;)V",
    (void *)atv_setParameters },
  { "native_getParameters",
    "()Ljava/lang/String;",
    (void *)atv_getParameters },
  { "reconnect",
    "()V",
    (void*)atv_reconnect },
  { "lock",
    "()V",
    (void*)atv_lock },
  { "unlock",
    "()V",
    (void*)atv_unlock },
  { "channelScan",
    "(II)V",
    (void *)atv_channelScan },    
  { "stopChannelScan",
    "()V",
    (void *)atv_stopChannelScan},       
  { "getChannelTable",
    "(I)J",
    (void *)atv_getChannelTable},     
  { "setChannelTable",
    "(IJ)V",
    (void *)atv_setChannelTable},      
  { "clearChannelTable",
    "()V",
    (void *)atv_clearChannelTable},  
  { "changeChannel",
    "(I)V",
    (void *)atv_changeChannel},    
  { "setLocation",
    "(I)V",
    (void *)atv_setLocation},   
  { "adjustSetting",
    "(BI)V",
    (void *)atv_adjustSetting},
  { "setChipDep",
    "(II)V",
    (void *)atv_setChipDep},    
  { "getSignalStrength",
    "()I",
    (void *)atv_getSignalStrength},
  { "setAudioCallback",
    "(Z)V",
    (void *)atv_setAudioCallback},    
  { "getAudioFormat",
    "()I",
    (void *)atv_getAudioFormat},
  { "setAudioFormat",
    "(I)V",
    (void *)atv_setAudioFormat},
    
};

struct field {
    const char *class_name;
    const char *field_name;
    const char *field_type;
    jfieldID   *jfield;
};

static int find_fields(JNIEnv *env, field *fields, int count)
{
    for (int i = 0; i < count; i++) {
        field *f = &fields[i];
        jclass clazz = env->FindClass(f->class_name);
        if (clazz == NULL) {
            XLOGE("Can't find %s", f->class_name);
            return -1;
        }

        jfieldID field = env->GetFieldID(clazz, f->field_name, f->field_type);
        if (field == NULL) {
            XLOGE("Can't find %s.%s", f->class_name, f->field_name);
            return -1;
        }

        *(f->jfield) = field;
    }

    return 0;
}

// Get all the required offsets in java class and register native functions
int register_android_hardware_Atv(JNIEnv *env)
{
    field fields_to_find[] = {
        //{ "com/mediatek/atv/AtvService", "x",   "I", &fields.jniteststring },
        { "android/view/Surface",    "mNativeObject",         "I", &fields.surface }, 
        { "com/mediatek/atv/AtvService", "mNativeContext",   "I", &fields.context },       
        { "android/media/MediaRecorder", "mNativeContext",   "I", &fields.recorder }
    };


    if (find_fields(env, fields_to_find, NELEM(fields_to_find)) < 0)
        return -1;

    jclass clazz = env->FindClass("com/mediatek/atv/AtvService");
    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIJLjava/lang/Object;)V");
    
    if (fields.post_event == NULL) {
        XLOGE("Can't find com/mediatek/atv/AtvService.postEventFromNative");
        return -1;
    }
	/*
    clazz = env->FindClass("android/media/MediaRecorder");
    if (clazz == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Can't find android/media/MediaRecorder");
        return -1;
    }

    fields.recorder = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.recorder == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Can't find MediaRecorder.mNativeContext");
        return -1;
    }*/

    // Register native functions
    return AndroidRuntime::registerNativeMethods(env, "com/mediatek/atv/AtvService",
                                              atvMethods, NELEM(atvMethods));
}


extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        XLOGE("GetEnv failed!");
        return result;
    }

    register_android_hardware_Atv(env);

    return JNI_VERSION_1_4;
}

