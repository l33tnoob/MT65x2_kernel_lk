#include "AudioVolumeFactory.h"

AudioMTKVolumeInterface *AudioVolumeFactory::CreateAudioVolumeController()
{
    // here can create diffeerent volumecontroller base on differemt platform or policy
    AudioMTKVolumeInterface *mInstance = NULL;
    mInstance = android::AudioMTKVolumeController::getInstance();
    return mInstance;
}

void DestroyAudioVolumeController(AudioMTKVolumeInterface *mInstance)
{
    //here to destroy
}