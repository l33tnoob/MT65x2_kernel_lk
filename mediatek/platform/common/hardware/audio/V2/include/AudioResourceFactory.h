#ifndef AUDIO_RESOURCE_FACTORY_H
#define AUDIO_RESOURCE_FACTORY_H

#include "AudioResourceManagerInterface.h"
#include "AudioResourceManager.h"

class AudioResourceManagerFactory
{

    public:
        // here to implement create
        static AudioResourceManagerInterface *CreateAudioResource();
        static void DestroyAudioResource(AudioResourceManagerInterface *mInstance);

    private:

};

#endif

