#include "AudioAnalogControlFactory.h"
#include "AudioAnalogControl.h"
#include "AudioAnalogControlExt.h"
#include "AudioAnalogControlInterface.h"

AudioAnalogControlInterface *AudioAnalogControlFactory::CreateAudioAnalogControl()
{
    // here can create different base on differemt platform or policy
    AudioAnalogControlInterface *mInstance = NULL;

    // if you want to use EXT analog control
#ifdef USING_EXT_ANALOGCONTROL
    mInstance = android::AudioAnalogControlExt::getInstance();
#else
    // if you want to use oringal analog control
    mInstance = android::AudioAnalogControl::getInstance();
#endif
    return mInstance;
}

void AudioAnalogControlFactory::DestroyAudioAnalogControl(AudioAnalogControlInterface *mInstance)
{
    if (mInstance)
    {
        delete mInstance;
    }
    //here to destroy
}