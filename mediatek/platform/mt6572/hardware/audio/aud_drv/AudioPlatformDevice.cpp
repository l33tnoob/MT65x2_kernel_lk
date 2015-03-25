#include "AudioPlatformDevice.h"
#include "AudioAnalogType.h"
#include "audio_custom_exp.h"

#define LOG_TAG "AudioPlatformDevice"
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

status_t AudioPlatformDevice::InitCheck()
{
    ALOGD("InitCheck");
    return NO_ERROR;
}

AudioPlatformDevice::AudioPlatformDevice()
{
    ALOGD("AudioPlatformDevice constructor");
    mAudioAnalogReg = NULL;
    mAudioAnalogReg = AudioAnalogReg::getInstance();
    if (!mAudioAnalogReg)
    {
        ALOGW("mAudioAnalogReg = %p", mAudioAnalogReg);
    }
    // init analog part.
    for (int i = 0; i < AudioAnalogType::DEVICE_MAX; i++)
    {
        memset((void *)&mBlockAttribute[i], 0, sizeof(AnalogBlockAttribute));
    }
    for (int i = 0; i < AudioAnalogType::VOLUME_TYPE_MAX; i++)
    {
        memset((void *)&mVolumeAttribute[i], 0, sizeof(AnalogVolumeAttribute));
    }

    mHpRightDcCalibration = mHpLeftDcCalibration = 0;
}

/**
* a basic function for SetAnalogGain for different Volume Type
* @param VoleumType value want to set to analog volume
* @param volume function of analog gain , value between 0 ~ 255
* @return status_t
*/
status_t AudioPlatformDevice::SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume)
{
    ALOGD("SetAnalogGain VOLUME_TYPE = %d volume = %d ", VoleumType, volume);
    return NO_ERROR;
}

/**
* a basic function fo SetAnalogMute, if provide mute function of hardware.
* @param VoleumType value want to set to analog volume
* @param mute of volume type
* @return status_t
*/
status_t AudioPlatformDevice::SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute)
{
    ALOGD("AudioPlatformDevice SetAnalogMute VOLUME_TYPE = %d mute = %d ", VoleumType, mute);
    return NO_ERROR;
}


status_t AudioPlatformDevice::SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency)
{
    ALOGD("AudioPlatformDevice SetFrequency");
    mBlockSampleRate[DeviceType] = frequency;
    return NO_ERROR;
}

uint32 AudioPlatformDevice::GetDLFrequency(unsigned int frequency)
{
    ALOGD("AudioPlatformDevice GetDLFrequency = %d", frequency);
    uint32 Reg_value = 0;
    switch (frequency)
    {
        case 8000:
            Reg_value = 0;
            break;
        case 11025:
            Reg_value = 1;
            break;
        case 12000:
            Reg_value = 2;
            break;
        case 16000:
            Reg_value = 4;
            break;
        case 22050:
            Reg_value = 5;
            break;
        case 24000:
            Reg_value = 6;
            break;
        case 32000:
            Reg_value = 8;
            break;
        case 44100:
            Reg_value = 9;
            break;
        case 48000:
            Reg_value = 10;
        default:
            ALOGW("GetDLFrequency with frequency = %d", frequency);
    }
    return Reg_value;
}


uint32 AudioPlatformDevice::GetULFrequency(unsigned int frequency)
{
    ALOGD("AudioPlatformDevice GetULFrequencyGroup = %d", frequency);
    uint32 Reg_value = 0;
    switch (frequency)
    {
        case 8000:
        case 16000:
        case 32000:
            Reg_value = 0x0;
            break;
        case 48000:
            Reg_value = 0x1;
        default:
            ALOGW("GetULFrequency with frequency = %d", frequency);
    }
    ALOGD("AudioPlatformDevice GetULFrequencyGroup Reg_value = %d", Reg_value);
    return Reg_value;
}

uint32 AudioPlatformDevice::GetDLNewIFFrequency(unsigned int frequency)
{
    ALOGD("AudioPlatformDevice ApplyDLNewIFFrequency ApplyDLNewIFFrequency = %d", frequency);
    uint32 Reg_value = 0;
    switch (frequency)
    {
        case 8000:
            Reg_value = 0;
            break;
        case 11025:
            Reg_value = 1;
            break;
        case 12000:
            Reg_value = 2;
            break;
        case 16000:
            Reg_value = 3;
            break;
        case 22050:
            Reg_value = 4;
            break;
        case 24000:
            Reg_value = 5;
            break;
        case 32000:
            Reg_value = 6;
            break;
        case 44100:
            Reg_value = 7;
            break;
        case 48000:
            Reg_value = 8;
        default:
            ALOGW("ApplyDLNewIFFrequency with frequency = %d", frequency);
    }
    return Reg_value;
}

uint32 AudioPlatformDevice::GetULNewIFFrequency(unsigned int frequency)
{
    ALOGD("AudioPlatformDevice GetULNewIFFrequency ApplyULNewIFFrequency = %d", frequency);
    uint32 Reg_value = 0;
    switch (frequency)
    {
        case 8000:
        case 16000:
        case 32000:
            Reg_value = 1;
            break;
        case 48000:
            Reg_value = 3;
        default:
            ALOGW("GetULNewIFFrequency with frequency = %d", frequency);
    }
    ALOGD("AudioPlatformDevice GetULNewIFFrequency Reg_value = %d", Reg_value);
    return Reg_value;
}

status_t AudioPlatformDevice::TopCtlChangeTrigger(void)
{
    uint32_t top_ctrl_status_now = mAudioAnalogReg->GetAnalogReg(ABB_AFE_CON11);
    mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON11, ((top_ctrl_status_now & 0x0001) ? 0 : 1) << 8, 0x0100);
    return NO_ERROR;
}

status_t AudioPlatformDevice::DCChangeTrigger(void)
{
    uint32_t dc_status_now = mAudioAnalogReg->GetAnalogReg(0x4016);
    mAudioAnalogReg->SetAnalogReg(0x4016, ((dc_status_now & 0x0002) ? 0 : 1) << 9, 0x0200);
    return NO_ERROR;
}


bool AudioPlatformDevice::GetDownLinkStatus(void)
{
    for (int i = 0; i <= AudioAnalogType::DEVICE_2IN1_SPK; i++)
    {
        if (mBlockAttribute[i].mEnable == true)
        {
            ALOGD("GetDownLinkStatus True : i = %d DeviceType = %s", i, kAudioAnalogDeviceTypeName[i]);
            return true;
        }
    }
    return false;
}

bool AudioPlatformDevice::GetULinkStatus(void)
{
    for (int i = AudioAnalogType::DEVICE_2IN1_SPK; i <= AudioAnalogType::DEVICE_IN_DIGITAL_MIC; i++)
    {
        if (mBlockAttribute[i].mEnable == true)
        {
            ALOGD("GetULinkStatus True : i = %d DeviceType = %s", i, kAudioAnalogDeviceTypeName[i]);
            return true;
        }
    }
    return false;
}

/**
* a basic function fo AnalogOpen, open analog power
* @param DeviceType analog part power
* @return status_t
*/
status_t AudioPlatformDevice::AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioPlatformDevice AnalogOpen DeviceType = %s ", kAudioAnalogDeviceTypeName[DeviceType]);
    uint32 ulFreq, dlFreq;
    mLock.lock();
    if (mBlockAttribute[DeviceType].mEnable == true)
    {
        ALOGW("AudioPlatformDevice AnalogOpen bypass with DeviceType = %d", DeviceType);
        mLock.unlock();
        return NO_ERROR;;
    }
    mBlockAttribute[DeviceType].mEnable = true;
    // here to open pmic digital part
    ulFreq = GetULFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]);
    dlFreq = GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]);

    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
#if 0
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x0100, 0x0100);      // AUD 26M clock power down release
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON1 , GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0x000f);
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0 , GetDLNewIFFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]) << 12, 0xf000);
            TopCtlChangeTrigger();
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON5 , 0x28, 0x003f); //Use Default SDM Gain 0x28/0x3f = 0.63
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_TOP_CON0 , 0, 0xffff); //Use Normal Path
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON2 , 0, 0xffff); //Use Normal Path
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0 , 1, 0x0001); //Enable DL path
            TopCtlChangeTrigger();
#else
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x0100, 0x0100);      // AUD 26M clock power down release
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0, GetDLNewIFFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]) << 12 | 0x330, 0xffff); // config up8x_rxif
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON1, dlFreq, 0x000f); // DL sampling rate
            TopCtlChangeTrigger();
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0, 0x0001, 0x0001);                       // turn on DL
#endif

            break;

        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
#if 0
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x0100, 0x0100);      // AUD 26M clock power down release
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON1 , GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0x000f);
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0 , GetDLNewIFFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]) << 12, 0xf000);
            TopCtlChangeTrigger();
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON5 , 0x28, 0x003f); //Use Default SDM Gain 0x28/0x3f = 0.63
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_TOP_CON0 , 0, 0xffff); //Use Normal Path
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON2 , 0, 0xffff); //Use Normal Path
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0 , 1, 0x0001); //Enable DL path
            TopCtlChangeTrigger();
#else
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x0100, 0x0100);      // AUD 26M clock power down release
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0, GetDLNewIFFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]) << 12 | 0x330, 0xffff); // config up8x_rxif
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON1, dlFreq, 0x000f); // DL sampling rate
            //DC compensation setting
            ALOGD("AnalogOpen mHpRightDcCalibration [0x%x] mHpLeftDcCalibration [0x%x]", mHpRightDcCalibration, mHpLeftDcCalibration);
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON3, mHpLeftDcCalibration, 0xffff); // LCH cpmpensation value
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON4, mHpRightDcCalibration, 0xffff); // RCH cpmpensation value
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON10, 0x0001, 0x0001); // enable DC cpmpensation
            DCChangeTrigger();//Trigger DC compensation
            TopCtlChangeTrigger();
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0, 0x0001, 0x0001);  // turn on DL
#endif
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
#ifdef USING_EXTAMP_HP
            mLock.unlock();
            AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock();
#else
            mLock.unlock();
            AnalogOpen(AudioAnalogType::DEVICE_OUT_EARPIECER);
            mLock.lock();
#endif

            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
            mLock.unlock();
            AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock();
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            ALOGD("AudioPlatformDevice::DEVICE_IN_ADC2:");
#if 0
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x0100, 0x0100);      // AUD 26M clock power down release
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON1 , GetULFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]) << 4, 0x0010);
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG2, 0x302F | (GetULNewIFFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]) << 10), 0xffff); // config UL up8x_rxif adc voice mode
            TopCtlChangeTrigger();
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0 , 0x02, 0x0002); //Enable UL path
            TopCtlChangeTrigger();
#else
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x0100, 0x0100);      // AUD 26M clock power down release
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG2, 0x302F | (GetULNewIFFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]) << 10), 0xffff); // config UL up8x_rxif adc voice mode
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON1, GetULFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]) << 4, 0x0010); // UL sampling rate
            TopCtlChangeTrigger();
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0, 0x0002, 0x0002);  // turn on UL
#endif

            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
#if 0
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x0100, 0x0100);      // AUD 26M clock power down release
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON1 , GetULFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]), 0x0010);
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG2, 0x302F | (GetULNewIFFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]) << 10), 0xffff); // config UL up8x_rxif adc voice mode
            TopCtlChangeTrigger();
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON9 , 0x0011, 0x0011); // enable digital mic, 3.25M clock rate
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0 , 0x02, 0x0002); //Enable UL path
            TopCtlChangeTrigger();
#else
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x0100, 0x0100);      // AUD 26M clock power down release
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG2, 0x302F | (GetULNewIFFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]) << 10), 0xffff); // config UL up8x_rxif adc voice mode
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON1, GetULFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]) << 4, 0x0010); // UL sampling rate
            TopCtlChangeTrigger();
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON9, 0x0011, 0x0011);  // enable digital mic, 3.25M clock rate
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0, 0x0002, 0x0002);  // turn on UL
#endif

            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
            {
                mLock.unlock();
                AnalogOpen(AudioAnalogType::DEVICE_OUT_EARPIECER);
                mLock.lock();
            }
            break;
    }
    mLock.unlock();
    return NO_ERROR;
}

status_t AudioPlatformDevice::AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioPlatformDevice AnalogOpenForAddSPK DeviceType = %s ", kAudioAnalogDeviceTypeName[DeviceType]);
 //   uint32 ulFreq, dlFreq;
    mLock.lock();

        mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETR].mEnable = false;
        mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETL].mEnable = false;
        mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R].mEnable = true;   
        
     mLock.unlock();
    return NO_ERROR;
}

status_t AudioPlatformDevice::AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioPlatformDevice AnalogCloseForSubSPK DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();

    mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETR].mEnable = true;
  
    mLock.unlock();
    return NO_ERROR;
}
/**
* a basic function fo AnalogClose, ckose analog power
* @param DeviceType analog part power
* @return status_t
*/
status_t AudioPlatformDevice::AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioPlatformDevice AnalogClose DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();
    mBlockAttribute[DeviceType].mEnable = false;
    // here to open pmic digital part
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0, 0x0000, 0x0001);  // turn off DL
            //            TopCtlChangeTrigger();
            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0, 0x0000, 0x0001);  // turn off DL
            TopCtlChangeTrigger();
            ALOGD("AnalogClose Reset mHpRightDcCalibration/mHpLeftDcCalibration from [0x%x] [0x%x]", mHpRightDcCalibration, mHpLeftDcCalibration);
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON3, 0, 0xffff); // LCH cancel DC
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON4, 0, 0xffff); // RCH cancel DC
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON10, 0x0000, 0x0001); // enable DC cpmpensation
            DCChangeTrigger();//Trigger DC compensation
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
#ifdef USING_EXTAMP_HP
            mLock.unlock();
            AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock();
#else
            mLock.unlock();
            AnalogClose(AudioAnalogType::DEVICE_OUT_EARPIECER);
            mLock.lock();
#endif
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
            mLock.unlock();
            AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock();
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0, 0x0000, 0x0002);  // turn off UL
            //           TopCtlChangeTrigger();
            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON9, 0x0000, 0x0010);  // disable digital mic
            mAudioAnalogReg->SetAnalogReg(ABB_AFE_CON0, 0x0000, 0x0002);  // turn off UL
            //            TopCtlChangeTrigger();
            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
            {
                mLock.unlock();
                AnalogClose(AudioAnalogType::DEVICE_OUT_EARPIECER);
                mLock.lock();
            }
            break;
    }
    if (!GetDownLinkStatus() && !GetULinkStatus())
    {
        mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_SET, 0x0100, 0x0100);      // AUD 26M clock power down
        ALOGD("AudioPlatformDevice AnalogClose Power Down TOP_CKPDN1_SET");
    }
    else
    {
        ALOGD("AudioPlatformDevice AnalogClose No Power Down TOP_CKPDN1_SET");
    }
    mLock.unlock();
    return NO_ERROR;
}

/**
* a basic function fo select mux of device type, not all device may have mux
* if select a device with no mux support , report error.
* @param DeviceType analog part
* @param MuxType analog mux selection
* @return status_t
*/
status_t AudioPlatformDevice::AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType)
{
    ALOGD("AAudioPlatformDevice nalogSetMux DeviceType = %s MuxType = %s", kAudioAnalogDeviceTypeName[DeviceType], kAudioAnalogMuxTypeName[MuxType]);
    return NO_ERROR;
}

/**
* a  function for setParameters , provide wide usage of analog control
* @param command1
* @param command2
* @param data
* @return status_t
*/
status_t AudioPlatformDevice::setParameters(int command1 , int command2 , unsigned int data)
{
    return NO_ERROR;
}

/**
* a function for setParameters , provide wide usage of analog control
* @param command1
* @param data
* @return status_t
*/
status_t AudioPlatformDevice::setParameters(int command1 , void *data)
{
    return NO_ERROR;
}

/**
* a function fo getParameters , provide wide usage of analog control
* @param command1
* @param command2
* @param data
* @return copy_size
*/
int AudioPlatformDevice::getParameters(int command1 , int command2 , void *data)
{
    return 0;
}

//static const uint32_t kFadeSamples[4] = {4096, 2048, 1024, 512};

status_t AudioPlatformDevice::FadeOutDownlink(uint16_t sample_rate)
{
#if 0
    const uint16_t fade_unit_index = 3;

    // Set gain speed:              bit[9:10]  = 0x3 => FadeSamples/sample_rate (sec)
    uint32_t time_ms = (kFadeSamples[fade_unit_index] * 1000) / sample_rate;
    mAudioAnalogReg->SetAnalogReg(0x4004, fade_unit_index << 9, 3 << 9);

    // Set ch1 & ch2 fade type:     bit[3:4]   = 0x3 => fade out
    mAudioAnalogReg->SetAnalogReg(0x4004, 3 << 3, 3 << 3);

    // Mute function of ch1 & ch2:  bit[11:12] = 0x0 => Enable
    mAudioAnalogReg->SetAnalogReg(0x4004, 0 << 11, 3 << 11);

    // Sleep
    usleep(time_ms * 1000);
#endif
    return NO_ERROR;
}

status_t AudioPlatformDevice::FadeInDownlink(uint16_t sample_rate)
{
#if 0
    const uint16_t fade_unit_index = 3;

    // Set gain speed:              bit[9:10]  = 0x3 => FadeSamples/sample_rate (sec)
    uint32_t time_ms = (kFadeSamples[fade_unit_index] * 1000) / sample_rate;
    mAudioAnalogReg->SetAnalogReg(0x4004, fade_unit_index << 9, 3 << 9);

    // Set ch1 & ch2 fade type:      bit[3:4]  = 0x0 => fade in
    mAudioAnalogReg->SetAnalogReg(0x4004, 0 << 3, 3 << 3);

    // Sleep
    usleep(time_ms * 1000);

    // Mute function of ch1 & ch2:  bit[11:12] = 0x3 => Disable
    mAudioAnalogReg->SetAnalogReg(0x4004, 3 << 11, 3 << 11);
#endif
    return NO_ERROR;
}

status_t AudioPlatformDevice::SetDcCalibration(AudioAnalogType::DEVICE_TYPE DeviceType, int dc_cali_value)
{
    ALOGD("SetDcCalibration Type[%d] value[0x%x]", DeviceType, dc_cali_value);
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
            mHpRightDcCalibration = dc_cali_value;
            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            mHpLeftDcCalibration = dc_cali_value;
            break;
        default:
            break;
    }
    ALOGD("SetDcCalibration mHpRightDcCalibration [0x%x] mHpLeftDcCalibration [0x%x]", mHpRightDcCalibration, mHpLeftDcCalibration);
    return NO_ERROR;
}

}
