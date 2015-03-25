/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CONVERTER_H_

#define CONVERTER_H_

#include "WifiDisplaySource.h"

#include <media/stagefright/foundation/AHandler.h>

namespace android {

struct ABuffer;
struct IGraphicBufferProducer;
struct MediaCodec;

#define ENABLE_SILENCE_DETECTION        0

#ifndef ANDROID_DEFAULT_CODE	
#define USE_OMX_BITRATE_CONTROLLER  (1)
#endif

// Utility class that receives media access units and converts them into
// media access unit of a different format.
// Right now this'll convert raw video into H.264 and raw audio into AAC.
struct Converter : public AHandler {
    enum {
        kWhatAccessUnit,
        kWhatEOS,
        kWhatError,
        kWhatShutdownCompleted,
    };

    enum FlagBits {
        FLAG_USE_SURFACE_INPUT          = 1,
        FLAG_PREPEND_CSD_IF_NECESSARY   = 2,
    };
    Converter(const sp<AMessage> &notify,
              const sp<ALooper> &codecLooper,
              const sp<AMessage> &outputFormat,
              uint32_t flags = 0);

    status_t init();

    sp<IGraphicBufferProducer> getGraphicBufferProducer();

    size_t getInputBufferCount() const;

    sp<AMessage> getOutputFormat() const;
    bool needToManuallyPrependSPSPPS() const;

    void feedAccessUnit(const sp<ABuffer> &accessUnit);
    void signalEOS();

    void requestIDRFrame();
    void dropAFrame();
	void suspendEncoding(bool suspend);
   
	void shutdownAsync();
   
	int32_t getVideoBitrate() const;
    
	void setVideoBitrate(int32_t bitrate);
    
	static int32_t GetInt32Property(const char *propName, int32_t defaultValue);
    
	enum {
       
		// MUST not conflict with private enums below.
        
		kWhatMediaPullerNotify = 'pulN',
   
	};
#ifndef ANDROID_DEFAULT_CODE	
    status_t setWfdLevel(int32_t level);
    int      getWfdParam(int paramType);
    void  forceBlackScreen(bool blackNow) ;
#endif 
#ifdef SEC_WFD_VIDEO_PATH_SUPPORT
    void releaseOutputBuffer(size_t bufferIndex);
#endif

protected:
    virtual ~Converter();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatDoMoreWork,
        kWhatRequestIDRFrame,
        kWhatSuspendEncoding,
        kWhatShutdown,
        kWhatEncoderActivity,
        kWhatDropAFrame,
        kWhatReleaseOutputBuffer,
    };

    sp<AMessage> mNotify;
    sp<ALooper> mCodecLooper;
    sp<AMessage> mOutputFormat;
    uint32_t mFlags;
    bool mIsVideo;
    bool mIsH264;
    bool mIsPCMAudio;
    bool mNeedToManuallyPrependSPSPPS;

    sp<MediaCodec> mEncoder;
    sp<AMessage> mEncoderActivityNotify;

    sp<IGraphicBufferProducer> mGraphicBufferProducer;

    Vector<sp<ABuffer> > mEncoderInputBuffers;
    Vector<sp<ABuffer> > mEncoderOutputBuffers;

    List<size_t> mAvailEncoderInputIndices;

    List<sp<ABuffer> > mInputBufferQueue;

    sp<ABuffer> mCSD0;

    bool mDoMoreWorkPending;

#if ENABLE_SILENCE_DETECTION
    int64_t mFirstSilentFrameUs;
    bool mInSilentMode;
#endif

    sp<ABuffer> mPartialAudioAU;

    int32_t mPrevVideoBitrate;

    int32_t mNumFramesToDrop;
    bool mEncodingSuspended;
#if USE_OMX_BITRATE_CONTROLLER
     void *  mBitrateCtrlerLib;
    struct BitrateCtrler{
        void *bcHandle;
        int (*InitBC)(void **);
        int (*SetOneFrameBits)(void *, int, bool);
        bool (*CheckSkip)(void *);
        int (*UpdownLevel)(void *, int);
        int (*GetStatus)(void *, int);
        int (*SetTolerantBitrate)(void *, int);
        int (*DeInitBC)(void *);
    } ;
    BitrateCtrler mBitrateCtrler;
#endif

    status_t initEncoder();
   void	releaseEncoder();

    status_t feedEncoderInputBuffers();

    void scheduleDoMoreWork();
    status_t doMoreWork();

    void notifyError(status_t err);

    // Packetizes raw PCM audio data available in mInputBufferQueue
    // into a format suitable for transport stream inclusion and
    // notifies the observer.
    status_t feedRawAudioInputBuffers();

    static bool IsSilence(const sp<ABuffer> &accessUnit);

    sp<ABuffer> prependCSD(const sp<ABuffer> &accessUnit) const;

    DISALLOW_EVIL_CONSTRUCTORS(Converter);
};

}  // namespace android

#endif  // CONVERTER_H_
