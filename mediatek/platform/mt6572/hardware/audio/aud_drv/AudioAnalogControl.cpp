#include "AudioType.h"
#include "AudioAnalogControl.h"
#include "audio_custom_exp.h"
#include <utils/Log.h>

#define LOG_TAG "AudioAnalogControl"
#ifndef ANDROID_DEFAULT_CODE
#include <cutils/xlog.h>
#ifdef ALOGE
#undef ALOGE
#endif
#ifdef ALOGW
#undef ALOGW
#endif ALOGI
#undef ALOGI
#ifdef ALOGD
#undef ALOGD
#endif
#ifdef ALOGV
#undef ALOGV
#endif
#define ALOGE XLOGE
#define ALOGW XLOGW
#define ALOGI XLOGI
#define ALOGD XLOGD
#define ALOGV XLOGV
#else
#include <utils/Log.h>
#endif

namespace android
{

AudioAnalogControl *AudioAnalogControl::UniqueAnalogInstance = 0;

AudioAnalogControl *AudioAnalogControl::getInstance()
{
    if (UniqueAnalogInstance == 0)
    {
        ALOGD("+UniqueAnalogInstance\n");
        UniqueAnalogInstance = new AudioAnalogControl();
        ALOGD("-UniqueAnalogInstance\n");
    }
    return UniqueAnalogInstance;
}

AudioAnalogControl::~AudioAnalogControl()
{

}

AudioAnalogControl::AudioAnalogControl()
{
    ALOGD("AudioAnalogControl contructor \n");

    // init analog part.
    for (int i = 0; i < AudioAnalogType::DEVICE_MAX; i++)
    {
        memset((void *)&mBlockAttribute[i], 0, sizeof(AnalogBlockAttribute));
    }
    for (int i = 0; i < AudioAnalogType::VOLUME_TYPE_MAX; i++)
    {
        memset((void *)&mVolumeAttribute[i], 0, sizeof(AnalogVolumeAttribute));
    }
    // mAudioDevice is use to open external component, like Headset , speaker etc
    mAudioPlatformDevice = new AudioPlatformDevice();
    mAudioMachineDevice = new AudioMachineDevice();
    // start trim function
    AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL, AudioAnalogType::MUX_AUDIO);
    AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R, AudioAnalogType::MUX_AUDIO);
    AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L, AudioAnalogType::MUX_AUDIO);
    //mAudioMachineDevice->StartHeadphoneTrimFunction();
    //Disable Spk Trim when bring up
    /*
    mAudioPlatformDevice->AnalogOpen ( AudioAnalogType::DEVICE_OUT_SPEAKERL);
    mAudioMachineDevice->AnalogOpen ( AudioAnalogType::DEVICE_OUT_SPEAKERL);
    mAudioMachineDevice->StartSpkTrimFunction();
    mAudioMachineDevice->AnalogClose ( AudioAnalogType::DEVICE_OUT_SPEAKERL);
    mAudioPlatformDevice->AnalogClose ( AudioAnalogType::DEVICE_OUT_SPEAKERL);
    */
    mPinmuxInverse = false;
    mMode = AUDIO_MODE_NORMAL;
}

status_t AudioAnalogControl::InitCheck()
{
    ALOGD("InitCheck \n");
    return NO_ERROR;
}
void AudioAnalogControl::CheckDevicePolicy(uint32 *Par1, AudioAnalogType::AUDIOANALOG_TYPE AnalogType)
{
    ALOGD("+CheckDevicePolicy Par1 = %d AnalogType = %d", *Par1 , AnalogType);
    switch (AnalogType)
    {
        case AudioAnalogType::AUDIOANALOG_DEVICE:
            if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
            {
                // in 2in1 speaker mode, open earpiece means open 2in1 speaker mode
                if (*Par1 == AudioAnalogType::DEVICE_OUT_EARPIECER)
                {
                    *Par1 = AudioAnalogType::DEVICE_2IN1_SPK;
                }
                else if (*Par1 == AudioAnalogType::DEVICE_OUT_EARPIECEL)
                {
                    *Par1 = AudioAnalogType::DEVICE_2IN1_SPK;
                }
            }
            break;
        case AudioAnalogType::AUDIOANALOG_MUX:
            // handle for ext amp select
#ifdef USING_EXTAMP_HP
            if ((*Par1 == AudioAnalogType::DEVICE_OUT_SPEAKERR) || (*Par1 == AudioAnalogType::DEVICE_OUT_SPEAKERL) ||
                (*Par1 == AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R) || (*Par1 == AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L))
            {
                *Par1 = AudioAnalogType::DEVICE_OUT_HEADSETR;
            }
#endif
            break;
        default:
            ALOGW("CheckDevicePolicy with Par1 = %d", *Par1);
            break;
    }
    ALOGD("-CheckDevicePolicy Par1 = %d AnalogType = %d", *Par1 , AnalogType);
}

//analog gain setting
status_t AudioAnalogControl::SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume)
{
    ALOGD("SetAnalogGain VoleumType = %d volume = %d \n", VoleumType, volume);
    mAudioPlatformDevice->SetAnalogGain(VoleumType, volume);
    mAudioMachineDevice->SetAnalogGain(VoleumType, volume);
    return NO_ERROR;
}

int AudioAnalogControl::GetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType)
{
    ALOGD("GetAnalogGain VoleumType = %d", VoleumType);
    return mAudioMachineDevice->GetAnalogGain(VoleumType);
}

status_t AudioAnalogControl::SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute)
{
    ALOGD("SetAnalogMute VoleumType = %d mute = %d \n", VoleumType, mute);
    mAudioMachineDevice->SetAnalogMute(VoleumType, mute);
    return NO_ERROR;
}

// analog open power , need to open by mux setting
status_t AudioAnalogControl::AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    ALOGD("AnalogOpen DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    CheckDevicePolicy((uint32 *)&DeviceType, AudioAnalogType::AUDIOANALOG_DEVICE);
    mBlockAttribute[DeviceType].mEnable = true;
    mAudioPlatformDevice->AnalogOpen(DeviceType);
    mAudioMachineDevice->AnalogOpen(DeviceType);
    return NO_ERROR;
}

status_t AudioAnalogControl::AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    ALOGD("AnalogOpenForAddSPK DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    CheckDevicePolicy((uint32 *)&DeviceType, AudioAnalogType::AUDIOANALOG_DEVICE);
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETR].mEnable = mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETL].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R].mEnable = true;
    mAudioPlatformDevice->AnalogOpenForAddSPK(DeviceType);
    mAudioMachineDevice->AnalogOpenForAddSPK(DeviceType);
    return NO_ERROR;
}
status_t AudioAnalogControl::SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency)
{
    ALOGD("AudioAnalogControl SetFrequency DeviceType = %d, frequency = %d", DeviceType, frequency);
    mBlockAttribute[DeviceType].mFrequency = frequency;
    mAudioPlatformDevice->SetFrequency(DeviceType, frequency);
    mAudioMachineDevice->SetFrequency(DeviceType, frequency);
    return NO_ERROR;
}

status_t AudioAnalogControl::AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    ALOGD("AnalogClose DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    CheckDevicePolicy((uint32 *)&DeviceType, AudioAnalogType::AUDIOANALOG_DEVICE);
    mBlockAttribute[DeviceType].mEnable = false;
    mAudioMachineDevice->AnalogClose(DeviceType);
    mAudioPlatformDevice->AnalogClose(DeviceType);
    return NO_ERROR;
}
status_t AudioAnalogControl::AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    ALOGD("AnalogCloseForSubSPK DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    CheckDevicePolicy((uint32 *)&DeviceType, AudioAnalogType::AUDIOANALOG_DEVICE);
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R].mEnable = mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETR].mEnable = true;
    mAudioPlatformDevice->AnalogCloseForSubSPK(DeviceType);
    mAudioMachineDevice->AnalogCloseForSubSPK(DeviceType);
    return NO_ERROR;
}

status_t AudioAnalogControl::SetAnalogPinmuxInverse(bool bEnable)
{
    ALOGD("SetAnalogPinmuxInverse bEnable = %d", bEnable);
    mPinmuxInverse = bEnable;
    return NO_ERROR;
}

bool AudioAnalogControl::GetAnalogPinmuxInverse(void)
{
    return mPinmuxInverse;
}

status_t AudioAnalogControl::CheckPinmuxInverse(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE &MuxType)
{
#ifndef MTK_DUAL_MIC_SUPPORT
    // no dual mic , only use the mic1 as input
#else
    if (mPinmuxInverse == true)
    {
        ALOGD("CheckPinmuxInverse inverse DeviceType = %s MuxType= %s", kAudioAnalogDeviceTypeName[DeviceType], kAudioAnalogMuxTypeName[MuxType]);
        if (DeviceType == AudioAnalogType::DEVICE_IN_PREAMP_L && MuxType == AudioAnalogType::MUX_IN_MIC1)
        {
            MuxType = AudioAnalogType::MUX_IN_MIC3;
        }
        if (DeviceType == AudioAnalogType::DEVICE_IN_PREAMP_R && MuxType == AudioAnalogType::MUX_IN_MIC3)
        {
            MuxType = AudioAnalogType::MUX_IN_MIC1;
        }
    }
#endif
    return NO_ERROR;
}


//some analog part may has mux for different output
status_t AudioAnalogControl::AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType)
{
    ALOGD("AnalogSetMux DeviceType = %s MUX_TYPE = %s", kAudioAnalogDeviceTypeName[DeviceType], kAudioAnalogMuxTypeName[MuxType]);
    CheckDevicePolicy((uint32 *)&DeviceType, AudioAnalogType::AUDIOANALOG_MUX);
    CheckPinmuxInverse(DeviceType, MuxType); // check if inverse pinmux
    mAudioMachineDevice->AnalogSetMux(DeviceType, MuxType);
    mAudioPlatformDevice->AnalogSetMux(DeviceType, MuxType);
    return NO_ERROR;
}

//some analog part may has mux for different output
AudioAnalogType::MUX_TYPE AudioAnalogControl::AnalogGetMux(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    AudioAnalogType::MUX_TYPE MuxType;
    CheckDevicePolicy((uint32 *)&DeviceType, AudioAnalogType::AUDIOANALOG_MUX);
    MuxType = mAudioMachineDevice->AnalogGetMux(DeviceType);
    ALOGD("AnalogGetMux DeviceType = %s MUX_TYPE = %s", kAudioAnalogDeviceTypeName[DeviceType], kAudioAnalogMuxTypeName[MuxType]);
    return MuxType;
}

bool AudioAnalogControl::GetAnalogState(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("+GetAnalogState DeviceType [%s]=%d\n", kAudioAnalogDeviceTypeName[DeviceType], mBlockAttribute[DeviceType].mEnable);
    return mBlockAttribute[DeviceType].mEnable;
}

bool AudioAnalogControl::AnalogDLlinkEnable(void)
{
    for (int i = AudioAnalogType::DEVICE_OUT_EARPIECER; i <= AudioAnalogType::DEVICE_OUT_LINEOUTL; i++)
    {
        if (mBlockAttribute[i].mEnable == true)
        {
            ALOGD("AnalogUplinkEnable i = %d", i);
            return true;
        }
    }
    ALOGD("AnalogDLlinkEnable return false");
    return false;
}

bool AudioAnalogControl::AnalogUplinkEnable(void)
{
    for (int i = AudioAnalogType::DEVICE_IN_ADC1; i <= AudioAnalogType::DEVICE_IN_DIGITAL_MIC; i++)
    {
        if (mBlockAttribute[i].mEnable == true)
        {
            ALOGD("AnalogUplinkEnable i = %d", i);
            return true;
        }
    }
    ALOGD("AnalogUplinkEnable return false");
    return false;
}


status_t AudioAnalogControl::setmode(audio_mode_t mode)
{
    mMode = mode;
    return NO_ERROR;
}

// set parameters and get parameters
status_t AudioAnalogControl::setParameters(int command1 , int command2 , unsigned int data)
{
    return NO_ERROR;
}

status_t AudioAnalogControl::setParameters(int command1 , void *data)
{
    mAudioPlatformDevice->setParameters(command1,data);
    mAudioMachineDevice->setParameters(command1,data);
    return NO_ERROR;
}

int AudioAnalogControl::getParameters(int command1 , int command2 , void *data)
{
    return NO_ERROR;
}

// Fade out / fade in
status_t AudioAnalogControl::FadeOutDownlink(uint16_t sample_rate)
{
    return mAudioPlatformDevice->FadeOutDownlink(sample_rate);
}

status_t AudioAnalogControl::FadeInDownlink(uint16_t sample_rate)
{
    return mAudioPlatformDevice->FadeInDownlink(sample_rate);
}

status_t AudioAnalogControl::SetDcCalibration(AudioAnalogType::DEVICE_TYPE DeviceType, int dc_cali_value)
{
    ALOGD("AnalogOpen DeviceType = %s, dc_calibration 0x%x", kAudioAnalogDeviceTypeName[DeviceType], dc_cali_value);
    mAudioPlatformDevice->SetDcCalibration(DeviceType, dc_cali_value);
    return NO_ERROR;
}

bool AudioAnalogControl::GetAnalogSpkOCState(void)
{
    return mAudioMachineDevice->GetAnalogSpkOCState();
}


}
