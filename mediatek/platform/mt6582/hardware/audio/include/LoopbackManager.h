#ifndef ANDROID_LOOPBACK_MANAGER_H
#define ANDROID_LOOPBACK_MANAGER_H

#include <utils/threads.h>

#include "AudioType.h"
#include "SpeechType.h"

namespace android
{

// for loopback
enum loopback_t
{
    NO_LOOPBACK                                 = 0,

    // AFE Loopback
    AP_MAIN_MIC_AFE_LOOPBACK                    = 1,
    AP_HEADSET_MIC_AFE_LOOPBACK                 = 2,
    AP_REF_MIC_AFE_LOOPBACK                     = 3,

    // Acoustic Loopback
    MD_MAIN_MIC_ACOUSTIC_LOOPBACK               = 21,
    MD_HEADSET_MIC_ACOUSTIC_LOOPBACK            = 22,
    MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR  = 23,
    MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR     = 24,
    MD_REF_MIC_ACOUSTIC_LOOPBACK                = 25,

    // BT Loopback with codec
    AP_BT_LOOPBACK                              = 30,
    MD_BT_LOOPBACK                              = 31,

    // BT Loopback without codec
    AP_BT_LOOPBACK_NO_CODEC                     = 32,
    MD_BT_LOOPBACK_NO_CODEC                     = 33,
};

enum loopback_output_device_t
{
    LOOPBACK_OUTPUT_RECEIVER = 1,
    LOOPBACK_OUTPUT_EARPHONE = 2,
    LOOPBACK_OUTPUT_SPEAKER  = 3,
};

class LoopbackManager
{
    public:
        virtual ~LoopbackManager();

        static LoopbackManager *GetInstance();

        loopback_t GetLoopbackType();

        status_t SetLoopbackOn(loopback_t loopback_type, loopback_output_device_t loopback_output_device);
#if 0        
        status_t SetLoopbackOn(loopback_t loopback_type, loopback_output_device_t loopback_output_device, uint32 ul_samplerate, uint32 dl_samplerate);
#endif
        status_t SetLoopbackOff();

    protected:
        LoopbackManager();

        status_t CheckLoopbackTypeIsValid(loopback_t loopback_type);

        audio_devices_t GetInputDeviceByLoopbackType(loopback_t loopback_type);
        audio_devices_t GetOutputDeviceByLoopbackType(loopback_t loopback_type, loopback_output_device_t loopback_output_device);

        Mutex mLock;

        loopback_t mLoopbackType;

        audio_devices_t mInputDeviceCopy;
        audio_devices_t mOutputDeviceCopy;

        sph_enh_mask_struct_t mMaskCopy;

        modem_index_t mWorkingModemIndex;

        bool mBtHeadsetNrecOnCopy;

    private:
        static LoopbackManager *mLoopbackManager; // singleton
};

} // end namespace android

#endif // end of ANDROID_LOOPBACK_MANAGER_H
