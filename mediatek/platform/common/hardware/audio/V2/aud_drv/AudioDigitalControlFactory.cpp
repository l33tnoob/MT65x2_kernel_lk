#include "AudioDigitalControlFactory.h"
#include "AudioDigitalControl.h"
#include "AudioDigitalControlInterface.h"
namespace android
{

AudioDigitalControlInterface *AudioDigitalControlFactory::CreateAudioDigitalControl()
{
    // here can create different base on differemt platform or policy
    AudioDigitalControlInterface *mInstance = NULL;
    mInstance = android::AudioDigitalControl::getInstance();
    return mInstance;
}

void DestroyAudioDigitalControl(AudioDigitalControlInterface *mInstance)
{
    if (mInstance)
    {
        delete mInstance;
    }

    //here to destroy
}

}