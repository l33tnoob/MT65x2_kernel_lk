#ifndef ANDROID_AUDIO_LOOPBACK_CONTROLLER_H
#define ANDROID_AUDIO_LOOPBACK_CONTROLLER_H

#include "AudioType.h"
#include "AudioVolumeFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioDigitalControlFactory.h"
#include "AudioResourceManagerInterface.h"

namespace android
{

class AudioLoopbackController
{
    public:
        virtual ~AudioLoopbackController();

        static AudioLoopbackController *GetInstance();

        status_t OpenAudioLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device);
        status_t OpenAudioLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device, uint32 ul_samplerate, uint32 dl_samplerate);
        status_t CloseAudioLoopbackControlFlow();
        status_t SetApBTCodec(bool enable_codec);
		  bool IsAPBTLoopbackWithCodec(void); //0902
//#if defined(BTCVSD_LOOPBACK_WITH_CODEC)
#if 1 //0902
        class AudioMTKLoopbackThread : public Thread
        {
            public:
                AudioMTKLoopbackThread();
                virtual ~AudioMTKLoopbackThread();
                virtual status_t    readyToRun();
                virtual void        onFirstRef();
            private:
                String8 mName;
                virtual bool threadLoop();
        };
#endif

    protected:
        AudioLoopbackController();

        status_t SetDACI2sOutAttribute(int sample_rate);
        status_t SetADCI2sInAttribute(int sample_rate);
        status_t SetDAIBTAttribute(int sample_rate);

        float mMasterVolumeCopy;
        int   mMicAmpLchGainCopy;
        int   mMicAmpRchGainCopy;

        AudioMTKVolumeInterface *mAudioVolumeInstance;
        AudioAnalogControlInterface *mAudioAnalogInstance;
        AudioDigitalControlInterface *mAudioDigitalInstance;
        AudioResourceManagerInterface *mAudioResourceManager;

    private:
        static AudioLoopbackController *mAudioLoopbackController; // singleton

        int mFd2;
        bool mBtLoopbackWithCodec;
        bool mBtLoopbackWithoutCodec;
//#if defined(BTCVSD_LOOPBACK_WITH_CODEC)
#if 1 //0902
        sp<AudioMTKLoopbackThread>  mBTCVSDLoopbackThread;
#endif
        //for BT SW BT CVSD loopback test
        bool mUseBtCodec;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_LOOPBACK_CONTROLLER_H
