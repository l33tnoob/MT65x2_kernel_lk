#ifndef ANDROID_AUDIO_MATV_RESOURCE_MANAGER_H
#define ANDROID_AUDIO_MATV_RESOURCE_MANAGER_H

#include "AudioResourceManager.h"

namespace android
{

/*==============================================================================
 *                     Classes
 *============================================================================*/

class AudioMATVResourceManager : public AudioResourceManager
{
    public:
        virtual ~AudioMATVResourceManager();

        virtual status_t SetMatvSourceModuleEnable(const bool enable);

        virtual status_t SetMatvDirectConnection(const bool enable);

        virtual uint32_t GetMatvUplinkSamplingRate() const;
        virtual uint32_t GetMatvDownlinkSamplingRate() const;


    protected:
        AudioMATVResourceManager();

        friend class AudioMATVController; // no one can use AudioMATVResourceManager but only AudioMATVController
};

} // end namespace android

#endif // end of ANDROID_AUDIO_MATV_RESOURCE_MANAGER_H
