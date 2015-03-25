#ifndef ANDROID_AUDIO_MATV_CONTROLLER_H
#define ANDROID_AUDIO_MATV_CONTROLLER_H

#include <utils/threads.h>

#include "AudioType.h"

namespace android
{

class AudioMATVResourceManager;
class AudioMTKStreamManagerInterface;

class AudioMATVController
{
    public:
        virtual ~AudioMATVController();

        static AudioMATVController *GetInstance();

        virtual bool     GetMatvEnable();
        virtual status_t SetMatvEnable(const bool enable);

        virtual uint32_t GetMatvUplinkSamplingRate() const;
        virtual uint32_t GetMatvDownlinkSamplingRate() const;


    protected:
        AudioMATVController();

        Mutex mLock;

        AudioMATVResourceManager       *mAudioMatvResourceManager;
        AudioMTKStreamManagerInterface *mAudioMTKStreamManager;

        bool mMatvEnable;
        bool mIsMatvDirectConnectionMode;

    private:
        static AudioMATVController *mAudioMATVController; // singleton

};

} // end namespace android

#endif // end of ANDROID_AUDIO_MATV_CONTROLLER_H
