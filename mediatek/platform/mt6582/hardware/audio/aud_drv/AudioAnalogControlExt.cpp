#include "AudioAnalogControlExt.h"
#include "AudioAnalogControl.h"
#include "AudioType.h"
#include "audio_custom_exp.h"
#include <utils/Log.h>

#define LOG_TAG "AudioAnalogControlExt"

namespace android
{

AudioAnalogControlExt *AudioAnalogControlExt::UniqueAnalogExtInstance = 0;
AudioAnalogControlInterface *AudioAnalogControlExt::UniqueAnalogInstance = 0;

AudioAnalogControlExt *AudioAnalogControlExt::getInstance()
{
    if (UniqueAnalogExtInstance == 0)
    {
        ALOGD("+AudioAnalogControlExt");
        UniqueAnalogInstance = AudioAnalogControl::getInstance();
        UniqueAnalogExtInstance = new AudioAnalogControlExt();
        ALOGD("-AudioAnalogControlExt");
    }
    return UniqueAnalogExtInstance;
}

AudioAnalogControlExt::~AudioAnalogControlExt()
{

}

AudioAnalogControlExt::AudioAnalogControlExt()
{
    ALOGD("AudioAnalogControlExt contructor \n");

    // init analog part.
    for (int i = 0; i < AudioAnalogType::DEVICE_MAX; i++)
    {
        memset((void *)&mBlockAttribute[i], 0, sizeof(AnalogBlockAttribute));
    }
    for (int i = 0; i < AudioAnalogType::VOLUME_TYPE_MAX; i++)
    {
        memset((void *)&mVolumeAttribute[i], 0, sizeof(AnalogVolumeAttribute));
    }
}

status_t AudioAnalogControlExt::InitCheck()
{
    ALOGD("InitCheck \n");
    return UniqueAnalogInstance->InitCheck();
}

//analog gain setting
status_t AudioAnalogControlExt::SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume)
{
    ALOGD("SetAnalogGain VoleumType = %d volume = %d \n", VoleumType, volume);
    return UniqueAnalogInstance->SetAnalogGain(VoleumType, volume);
}

int AudioAnalogControlExt::GetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType)
{
    ALOGD("GetAnalogGain VoleumType = %d", VoleumType);
    return UniqueAnalogInstance->GetAnalogGain(VoleumType);
}

status_t AudioAnalogControlExt::SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute)
{
    ALOGD("SetAnalogMute VoleumType = %d mute = %d \n", VoleumType, mute);
    return UniqueAnalogInstance->SetAnalogMute(VoleumType, mute);
}

// analog open power , need to open by mux setting
status_t AudioAnalogControlExt::AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    ALOGD("AudioAnalogControlExt::AnalogOpen");
    return UniqueAnalogInstance->AnalogOpen(DeviceType, Type_setting);
}

status_t AudioAnalogControlExt::SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency)
{
    ALOGD("AudioAnalogControlExt SetFrequency DeviceType = %dfrequency = %d", DeviceType, frequency);

    return UniqueAnalogInstance->SetFrequency(DeviceType, frequency);
}

status_t AudioAnalogControlExt::AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    ALOGD("AudioAnalogControlExt::AnalogClose");
    return UniqueAnalogInstance->AnalogClose(DeviceType, Type_setting);
}

status_t AudioAnalogControlExt::SetAnalogPinmuxInverse(bool bEnable)
{
    ALOGD("SetAnalogPinmuxInverse bEnable = %d", bEnable);
    return UniqueAnalogInstance->SetAnalogPinmuxInverse(bEnable);
}

bool AudioAnalogControlExt::GetAnalogPinmuxInverse(void)
{
    return UniqueAnalogInstance->GetAnalogPinmuxInverse();
}

//some analog part may has mux for different output
AudioAnalogType::MUX_TYPE AudioAnalogControlExt::AnalogGetMux(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioAnalogControlExt AnalogSetMux");
    return UniqueAnalogInstance->AnalogGetMux(DeviceType);
}

//some analog part may has mux for different output
status_t AudioAnalogControlExt::AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType)
{
    ALOGD("AudioAnalogControlExt AnalogSetMux");
    return UniqueAnalogInstance->AnalogSetMux(DeviceType, MuxType);
}

bool AudioAnalogControlExt::GetAnalogState(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    return UniqueAnalogInstance->GetAnalogState(DeviceType);
}

bool AudioAnalogControlExt::AnalogDLlinkEnable(void)
{
    return UniqueAnalogInstance->AnalogDLlinkEnable();
}

bool AudioAnalogControlExt::AnalogUplinkEnable(void)
{
    return UniqueAnalogInstance->AnalogUplinkEnable();
}

// set parameters and get parameters
status_t AudioAnalogControlExt::setParameters(int command1 , int command2 , unsigned int data)
{
    return UniqueAnalogInstance->setParameters(command1, command2, data);
}

status_t AudioAnalogControlExt::setParameters(int command1 , void *data)
{
    return UniqueAnalogInstance->setParameters(command1, data);
}

int AudioAnalogControlExt::getParameters(int command1 , int command2 , void *data)
{
    return UniqueAnalogInstance->getParameters(command1, command2, data);
}

status_t AudioAnalogControlExt::setmode(audio_mode_t mode)
{
    return UniqueAnalogInstance->setmode(mode);
}

// Fade out / fade in
status_t AudioAnalogControlExt::FadeOutDownlink(uint16_t sample_rate)
{
    return UniqueAnalogInstance->FadeOutDownlink(sample_rate);
}

status_t AudioAnalogControlExt::FadeInDownlink(uint16_t sample_rate)
{
    return UniqueAnalogInstance->FadeInDownlink(sample_rate);
}

status_t AudioAnalogControlExt::SetDcCalibration(AudioAnalogType::DEVICE_TYPE DeviceType, int dc_cali_value)
{
    return UniqueAnalogInstance->SetDcCalibration(DeviceType, dc_cali_value);
}

bool AudioAnalogControlExt::GetAnalogSpkOCState(void)
{
    return UniqueAnalogInstance->GetAnalogSpkOCState();
}

status_t AudioAnalogControlExt::AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    return NO_ERROR;
}
status_t AudioAnalogControlExt::AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    return NO_ERROR;
}

}
