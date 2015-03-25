#ifndef AUDIO_VOLUME_FACTORY_H
#define AUDIO_VOLUME_FACTORY_H

#include "AudioMTKVolumeController.h"
#include "AudioMTKVolumeInterface.h"

class AudioVolumeFactory
{
    public:
        // here to implement create and
        static AudioMTKVolumeInterface *CreateAudioVolumeController();
        static void DestroyAudioVolumeController();
    private:
};

#endif
