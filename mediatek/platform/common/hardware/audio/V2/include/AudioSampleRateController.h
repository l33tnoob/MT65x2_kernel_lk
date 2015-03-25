#ifndef ANDROID_AUDIO_SAMPLE_RATE_CONTROLLER_H
#define ANDROID_AUDIO_SAMPLE_RATE_CONTROLLER_H

#include <utils/threads.h>
#include <utils/Vector.h>
#include <sys/time.h>

#include "AudioType.h"
#include "AudioMTKStreamOut.h"

#define MAX_RECORD 100

namespace android
{
class AudioMTKStreamOut;
class AudioSampleRateController
{
    public:
        virtual ~AudioSampleRateController();


        enum SampleRateScenario_T{
            FM,
            AUDIOTRACK,
            OTHER
        };
        
        enum Setting_Result_T{
            FAILD_SRC_RUNING,
            FAILD_NOT_ALLOW,
            SUCCESS = 0,
            SUCCESS_NO_CHANGE,
            SUCCESS_DID_CHANGE
        };
        
        struct Setting_T{
            SampleRateScenario_T Scenario;
            uint32_t SampleRate;
            Setting_Result_T Result;
            struct tm Time;
        };
        
        static AudioSampleRateController *GetInstance();
        Vector<struct Setting_T> mRecord;
        virtual uint32_t RegistreStreamOut(AudioMTKStreamOut* streamout);
        virtual uint32_t AddRecord(SampleRateScenario_T scenario, uint32_t samplerate, Setting_Result_T result);
        virtual uint32_t ApplySampleRate(SampleRateScenario_T scenario, uint32_t samplerate);
        
        virtual bool  Lock(void);
        virtual bool  Unlock(void);
        
    protected:
        AudioSampleRateController();

        Mutex mLock;

    private:
        static AudioSampleRateController *mAudioSampleRateController; // singleton
        AudioMTKStreamOut *mStreamOut;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_SAMPLE_RATE_CONTROLLER_H
