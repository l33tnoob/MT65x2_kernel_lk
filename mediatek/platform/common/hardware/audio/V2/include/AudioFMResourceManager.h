#ifndef ANDROID_AUDIO_FM_RESOURCE_MANAGER_H
#define ANDROID_AUDIO_FM_RESOURCE_MANAGER_H

#include "AudioResourceManager.h"

namespace android
{

/*==============================================================================
 *                     Classes
 *============================================================================*/

class WCNChipController;

class AudioFMResourceManager : public AudioResourceManager
{
    public:
        virtual ~AudioFMResourceManager();

        virtual status_t SetFmSourceModuleEnable(const bool enable);

        virtual status_t SetFmDirectConnection(const bool enable);

        virtual status_t SetFmVolume(const float fm_volume);

        virtual uint32_t GetFmUplinkSamplingRate() const;
        virtual uint32_t GetFmDownlinkSamplingRate() const;


    protected:
        AudioFMResourceManager();

        friend class AudioFMController; // no one can use AudioFMResourceManager but only AudioFMController

        WCNChipController *mWCNChipController;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_FM_RESOURCE_MANAGER_H
