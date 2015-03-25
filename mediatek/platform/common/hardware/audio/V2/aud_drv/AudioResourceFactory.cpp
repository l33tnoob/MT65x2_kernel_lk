
#include "AudioResourceFactory.h"
#include "AudioResourceManager.h"

AudioResourceManagerInterface *AudioResourceManagerFactory::CreateAudioResource()
{

    AudioResourceManagerInterface *mInstance = NULL;
    mInstance = android::AudioResourceManager::getInstance();
    return mInstance;

}

void AudioResourceManagerFactory::DestroyAudioResource(AudioResourceManagerInterface *mInstance)
{
    // do nothing
}



