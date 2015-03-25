#ifndef AUDIO_DIGITAL_CONTROL_FACTORY_H
#define AUDIO_DIGITAL_CONTROL_FACTORY_H

#include "AudioDigitalControlInterface.h"

namespace android
{

class AudioDigitalControlFactory
{
    public:
        // here to implement create and
        static AudioDigitalControlInterface *CreateAudioDigitalControl();
        static void DestroyAudioDigitalControl();
    private:
};

}
#endif
