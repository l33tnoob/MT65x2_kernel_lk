#include "AudioFtmBase.h"

#include <utils/threads.h>

#include "AudioAssert.h"

#include "AudioFtm.h"

#define LOG_TAG "AudioFtmBase"

namespace android
{

AudioFtmBase *AudioFtmBase::createAudioFtmInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    return AudioFtm::getInstance();
}

AudioFtmBase::AudioFtmBase()
{
    ALOGD("%s()", __FUNCTION__);
}

AudioFtmBase::~AudioFtmBase()
{
    ALOGD("%s()", __FUNCTION__);
}


/// Codec
void AudioFtmBase::Audio_Set_Speaker_Vol(int level) { return; }
void AudioFtmBase::Audio_Set_Speaker_On(int Channel) { return; }
void AudioFtmBase::Audio_Set_Speaker_Off(int Channel) { return; }
void AudioFtmBase::Audio_Set_HeadPhone_On(int Channel) { return; }
void AudioFtmBase::Audio_Set_HeadPhone_Off(int Channel) { return; }
void AudioFtmBase::Audio_Set_Earpiece_On() { return; }
void AudioFtmBase::Audio_Set_Earpiece_Off() { return; }


/// for factory mode & Meta mode (Analog part)
void AudioFtmBase::FTM_AnaLpk_on(void) { return; }
void AudioFtmBase::FTM_AnaLpk_off(void) { return; }


/// Output device test
int AudioFtmBase::RecieverTest(char receiver_test) { return true; }
int AudioFtmBase::LouderSPKTest(char left_channel, char right_channel) { return true; }
int AudioFtmBase::EarphoneTest(char bEnable) { return true; }
int AudioFtmBase::EarphoneTestLR(char bLR) { return true; }


/// Speaker over current test
int AudioFtmBase::Audio_READ_SPK_OC_STA(void) { return true; }
int AudioFtmBase::LouderSPKOCTest(char left_channel, char right_channel) { return true; }


/// Loopback
int AudioFtmBase::PhoneMic_Receiver_Loopback(char echoflag) { return true; }
int AudioFtmBase::PhoneMic_EarphoneLR_Loopback(char echoflag) { return true; }
int AudioFtmBase::PhoneMic_SpkLR_Loopback(char echoflag) { return true; }
int AudioFtmBase::HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic) { return true; }
int AudioFtmBase::HeadsetMic_SpkLR_Loopback(char echoflag) { return true; }

int AudioFtmBase::PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output) { return true; }


/// FM / mATV
int AudioFtmBase::FMLoopbackTest(char bEnable) { return true; }

int AudioFtmBase::Audio_FM_I2S_Play(char bEnable) { return true; }
int AudioFtmBase::Audio_MATV_I2S_Play(int enable_flag) { return true; }
int AudioFtmBase::Audio_FMTX_Play(bool Enable, unsigned int Freq) { return true; }

int AudioFtmBase::ATV_AudPlay_On(void) { return true; }
int AudioFtmBase::ATV_AudPlay_Off(void) { return true; }
unsigned int AudioFtmBase::ATV_AudioWrite(void *buffer, unsigned int bytes) { return true; }


/// HDMI
int AudioFtmBase::Audio_HDMI_Play(bool Enable, unsigned int Freq) { return true; }


/// Vibration Speaker
int AudioFtmBase::SetVibSpkCalibrationParam(void *cali_param)
{
    ALOGE("%s() is not supported!!", __FUNCTION__);
    return NO_ERROR;
}

uint32_t AudioFtmBase::GetVibSpkCalibrationStatus()
{
    ALOGE("%s() is not supported!!", __FUNCTION__);
    return 0;
}

void AudioFtmBase::SetVibSpkEnable(bool enable, uint32_t freq)
{
    ALOGE("%s() is not supported!!", __FUNCTION__);
}

void AudioFtmBase::SetVibSpkRampControl(uint8_t rampcontrol)
{
    ALOGE("%s() is not supported!!", __FUNCTION__);
}

bool AudioFtmBase::ReadAuxadcData(int channel, int *value)
{
    ALOGE("%s() is not supported!!", __FUNCTION__);
    return false;
}

int AudioFtmBase::HDMI_SineGenPlayback(bool bEnable, int dSamplingRate) 
{
    ALOGE("%s() is not supported!!", __FUNCTION__);
    return false;
}




} // end of namespace android
