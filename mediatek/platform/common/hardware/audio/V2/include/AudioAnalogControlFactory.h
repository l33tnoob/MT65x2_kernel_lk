#ifndef AUDIO_ANALOG_FACTORY_FACTORY_H
#define AUDIO_ANALOG_FACTORY_FACTORY_H

#include "AudioAnalogControl.h"
#include "AudioAnalogControlInterface.h"

class AudioAnalogControlFactory
{
    public:
        /**
        * a function fo CreateAudioAnalogControl
        * @return pointer of AudioAnalogControlInterface , can use AudioAnalogControlInterface to operate
        */
        static AudioAnalogControlInterface *CreateAudioAnalogControl();
        /**
        * a function fo DestroyAudioAnalogControl
        */
        static void DestroyAudioAnalogControl(AudioAnalogControlInterface *mInstance);
    private:
};

#endif
