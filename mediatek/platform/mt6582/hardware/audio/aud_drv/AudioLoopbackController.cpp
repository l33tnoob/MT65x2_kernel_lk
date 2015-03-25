#include "AudioLoopbackController.h"
#include "AudioBTCVSDControl.h"
#include "AudioUtility.h"
#include "AudioMTKStreamManager.h"

#include "WCNChipController.h"

#define LOG_TAG "AudioLoopbackController"
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

// for use max gain for audio loopback
#define AUDIO_LOOPBACK_USE_MAX_GAIN

namespace android
{

//#if defined(BTCVSD_LOOPBACK_WITH_CODEC)
#if 1 //0902
static android_audio_legacy::AudioStreamOut *streamOutput;
#endif

static const float kMaxMasterVolume = 1.0;

// too big gain might cause "Bee~" tone due to the output sound is collected by input
static const int kPreAmpGainMapValue[] = {30, 24, 18, 12, 6, 0}; // Map to AUDPREAMPGAIN: (000) 2dB, (001) 8dB, (010) 14dB, ..., (101) 32dB

enum preamp_gain_index_t
{
    PREAMP_GAIN_2_DB  = 0,
    PREAMP_GAIN_8_DB  = 1,
    PREAMP_GAIN_14_DB = 2,
    PREAMP_GAIN_20_DB = 3,
    PREAMP_GAIN_26_DB = 4,
    PREAMP_GAIN_32_DB = 5,
};



AudioLoopbackController *AudioLoopbackController::mAudioLoopbackController = NULL;
AudioLoopbackController *AudioLoopbackController::GetInstance()
{
    if (mAudioLoopbackController == NULL)
    {
        mAudioLoopbackController = new AudioLoopbackController();
    }
    ASSERT(mAudioLoopbackController != NULL);
    return mAudioLoopbackController;
}

AudioLoopbackController::AudioLoopbackController()
{
    // create volume instance
    mAudioVolumeInstance = AudioVolumeFactory::CreateAudioVolumeController();

    // create digital control instnace
    mAudioDigitalInstance  = AudioDigitalControlFactory::CreateAudioDigitalControl();

    // create digital control instnace
    mAudioAnalogInstance  = AudioAnalogControlFactory::CreateAudioAnalogControl();

    mAudioResourceManager = AudioResourceManager::getInstance();

    mMasterVolumeCopy  = 1.0;
    mMicAmpLchGainCopy = 0;
    mMicAmpRchGainCopy = 0;
    mBtLoopbackWithCodec = 0;
    mBtLoopbackWithoutCodec = 0;
    mUseBtCodec = 1;
}

AudioLoopbackController::~AudioLoopbackController()
{

}

status_t AudioLoopbackController::OpenAudioLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device, uint32 ul_samplerate, uint32 dl_samplerate)
{
    ALOGD("+%s(), input_device = 0x%x, output_device = 0x%x ul_sr %d, dl_sr %d", __FUNCTION__, input_device, output_device, ul_samplerate, dl_samplerate);
    // check BT device
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);

    // enable clock
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

    // set sampling rate
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, dl_samplerate);
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC, ul_samplerate);

    // set device
    mAudioResourceManager->setDlOutputDevice(output_device);
    mAudioResourceManager->setUlInputDevice(input_device);

#if 0 //0902
#if !defined(BTCVSD_LOOPBACK_WITH_CODEC)
    mUseBtCodec = 0; // always without codec
#endif
#endif

    // Open ADC/DAC I2S, or DAIBT
    ALOGD("+%s(), bt_device_on = %d, mUseBtCodec = %d, mBtLoopbackWithoutCodec: %d, mBtLoopbackWithCodec: %d",
          __FUNCTION__, bt_device_on, mUseBtCodec, mBtLoopbackWithoutCodec, mBtLoopbackWithCodec);
    if (bt_device_on == true)
    {
        // DAIBT
        if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
        {
        if (!mUseBtCodec)
        {
            mBtLoopbackWithoutCodec = 1;
            mFd2 = 0;
            mFd2 = ::open(kBTDeviceName, O_RDWR);
            ALOGD("+%s(), CVSD AP loopback without codec, mFd2: %d, AP errno: %d", __FUNCTION__, mFd2, errno);
            ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 0); //allocate TX working buffers in kernel
            ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 2); //allocate TX working buffers in kernel
            ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_TXSTATE_DIRECT_LOOPBACK); //set state to kernel
        }
        else
        {
//#if defined(BTCVSD_LOOPBACK_WITH_CODEC)
#if 1 //0902
            int format = AUDIO_FORMAT_PCM_16_BIT;
            uint32_t channels = AUDIO_CHANNEL_OUT_MONO;
            uint32_t sampleRate = 8000;
            status_t status;
            mBtLoopbackWithCodec = 1;
            streamOutput = AudioMTKStreamManager::getInstance()->openOutputStream(output_device, &format, &channels, &sampleRate, &status);
            ALOGD("+%s(), CVSD AP loopback with codec, streamOutput: %d", __FUNCTION__, streamOutput);
            mBTCVSDLoopbackThread = new AudioMTKLoopbackThread();
            if (mBTCVSDLoopbackThread.get())
            {
                mBTCVSDLoopbackThread->run();
            }
#endif
        }
        }
        else
        {
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I02, AudioDigitalType::O02); // DAIBT_IN -> DAIBT_OUT

        SetDAIBTAttribute(dl_samplerate);

        mAudioDigitalInstance->SetDAIBTEnable(true);
        }
    }
    else // ADC/DAC I2S
    {
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I03, AudioDigitalType::O03); // ADC_I2S_IN_L -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I03, AudioDigitalType::O04); // ADC_I2S_IN_L -> DAC_I2S_OUT_R

        SetADCI2sInAttribute(ul_samplerate);
        SetDACI2sOutAttribute(dl_samplerate);

        mAudioDigitalInstance->SetI2SAdcEnable(true);
        mAudioDigitalInstance->SetI2SDacEnable(true);
    }

    // AFE_ON
    mAudioDigitalInstance->SetAfeEnable(true);

    // Set PMIC digital/analog part - uplink has pop, open first
    mAudioResourceManager->StartInputDevice();
    usleep(100 * 1000); // HW pulse

    // Set PMIC digital/analog part - DL need trim code. Otherwise, just increase the delay between UL & DL...
    usleep(200 * 1000); // HW pulse
    mAudioResourceManager->StartOutputDevice();

#ifdef AUDIO_LOOPBACK_USE_MAX_GAIN
    // adjust downlink volume for current mode and routes
    mMasterVolumeCopy = mAudioVolumeInstance->getMasterVolume();
    mAudioVolumeInstance->setMasterVolume(kMaxMasterVolume, AUDIO_MODE_NORMAL, output_device);

    // adjust uplink volume for current mode and routes
    mMicAmpLchGainCopy = mAudioAnalogInstance->GetAnalogGain(AudioAnalogType::VOLUME_MICAMPL);
    mMicAmpRchGainCopy = mAudioAnalogInstance->GetAnalogGain(AudioAnalogType::VOLUME_MICAMPR);
    if (output_device == AUDIO_DEVICE_OUT_SPEAKER)
    {
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, kPreAmpGainMapValue[PREAMP_GAIN_2_DB]);
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, kPreAmpGainMapValue[PREAMP_GAIN_2_DB]);
    }
    else
    {
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, kPreAmpGainMapValue[PREAMP_GAIN_20_DB]);
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, kPreAmpGainMapValue[PREAMP_GAIN_20_DB]);
    }
#endif

    ALOGD("-%s(), input_device = 0x%x, output_device = 0x%x ul_sr %d, dl_sr %d", __FUNCTION__, input_device, output_device, ul_samplerate, dl_samplerate);
    return NO_ERROR;
}

status_t AudioLoopbackController::OpenAudioLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device)
{
    ALOGD("+%s(), input_device = 0x%x, output_device = 0x%x", __FUNCTION__, input_device, output_device);

    // check BT device
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);
    // set sample rate
    int  sample_rate;

    if (bt_device_on == true)
    {
        if (WCNChipController::GetInstance()->BTChipSamplingRate() == 0)
        {
            sample_rate = 8000;

        }
        else
        {
            sample_rate = 16000;
        }
    }
    else
    {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))
        {
            sample_rate  = 32000;
        }
        else
        {
            sample_rate  = 48000;
        }
    }


    return OpenAudioLoopbackControlFlow(input_device, output_device, sample_rate, sample_rate);
}

status_t AudioLoopbackController::CloseAudioLoopbackControlFlow()
{
    ALOGD("+%s()", __FUNCTION__);

    // Stop PMIC digital/analog part
    mAudioResourceManager->StopOutputDevice();
    mAudioResourceManager->StopInputDevice();
    // Stop AP side digital part
    const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);

    ALOGD("%s(), bt_device_on = %d, mBtLoopbackWithoutCodec: %d, mBtLoopbackWithCodec: %d",
          __FUNCTION__, bt_device_on, mBtLoopbackWithoutCodec, mBtLoopbackWithCodec);

    if (bt_device_on)
    {
        if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
        {
            if (mBtLoopbackWithoutCodec)
            {
                ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 1); //allocate TX working buffers in kernel
                ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 3); //allocate TX working buffers in kernel
                ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_TXSTATE_IDLE); //set state to kernel
                mBtLoopbackWithoutCodec = 0;
            }
            else if (mBtLoopbackWithCodec)
            {
//#if defined(BTCVSD_LOOPBACK_WITH_CODEC)
#if 1 //0902
                streamOutput->standby();
                if (mBTCVSDLoopbackThread.get())
                {
                    int ret = 0;
                    //ret = mBTCVSDLoopbackThread->requestExitAndWait();
                    //if (ret == WOULD_BLOCK)
                    {
                        mBTCVSDLoopbackThread->requestExit();
                    }
                    mBTCVSDLoopbackThread.clear();
                }
                AudioMTKStreamManager::getInstance()->closeOutputStream(streamOutput);
                mBtLoopbackWithCodec = 0;
#endif
            }
        }
        else
        {
            mAudioDigitalInstance->SetDAIBTEnable(false);
            mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I02, AudioDigitalType::O02); // DAIBT_IN -> DAIBT_OUT
        }
    }
    else
    {
        mAudioDigitalInstance->SetI2SDacEnable(false);
        mAudioDigitalInstance->SetI2SAdcEnable(false);

        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I03, AudioDigitalType::O03); // ADC_I2S_IN_L -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I03, AudioDigitalType::O04); // ADC_I2S_IN_R -> DAC_I2S_OUT_R
    }

    // AFE_ON = false
    mAudioDigitalInstance->SetAfeEnable(false);

    // recover sampling rate
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, 44100);
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC, 44100);

#ifdef AUDIO_LOOPBACK_USE_MAX_GAIN
    // recover volume for current mode and routes
    mAudioVolumeInstance->setMasterVolume(mMasterVolumeCopy, AUDIO_MODE_NORMAL, output_device);
    mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, mMicAmpLchGainCopy);
    mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, mMicAmpRchGainCopy);
#endif

    // disable clock
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);

    ALOGD("-%s()", __FUNCTION__);

    return NO_ERROR;
}

status_t AudioLoopbackController::SetApBTCodec(bool enable_codec)
{
    ALOGD("+%s(), enable_codec = %d", __FUNCTION__, enable_codec);
    mUseBtCodec = enable_codec;
    return NO_ERROR;
}

bool AudioLoopbackController::IsAPBTLoopbackWithCodec(void)
{
    ALOGD("+%s(), mBtLoopbackWithCodec = %d", __FUNCTION__, mBtLoopbackWithCodec);
    return mBtLoopbackWithCodec;
}

status_t AudioLoopbackController::SetDACI2sOutAttribute(int sample_rate)
{
    AudioDigtalI2S dac_i2s_out_attribute;
    memset((void *)&dac_i2s_out_attribute, 0, sizeof(dac_i2s_out_attribute));

    dac_i2s_out_attribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    dac_i2s_out_attribute.mI2S_SAMPLERATE = sample_rate;
    dac_i2s_out_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    dac_i2s_out_attribute.mI2S_FMT = AudioDigtalI2S::I2S;
    dac_i2s_out_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    mAudioDigitalInstance->SetI2SDacOut(&dac_i2s_out_attribute);
    return NO_ERROR;
}


status_t AudioLoopbackController::SetADCI2sInAttribute(int sample_rate)
{
    AudioDigtalI2S adc_i2s_in_attribute;
    memset((void *)&adc_i2s_in_attribute, 0, sizeof(adc_i2s_in_attribute));

    adc_i2s_in_attribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    adc_i2s_in_attribute.mBuffer_Update_word = 8;
    adc_i2s_in_attribute.mFpga_bit_test = 0;
    adc_i2s_in_attribute.mFpga_bit = 0;
    adc_i2s_in_attribute.mloopback = 0;
    adc_i2s_in_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    adc_i2s_in_attribute.mI2S_FMT = AudioDigtalI2S::I2S;
    adc_i2s_in_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    adc_i2s_in_attribute.mI2S_SAMPLERATE = sample_rate;
    adc_i2s_in_attribute.mI2S_EN = false;
    mAudioDigitalInstance->SetI2SAdcIn(&adc_i2s_in_attribute);
    return NO_ERROR;
}


status_t AudioLoopbackController::SetDAIBTAttribute(int sample_rate)
{
    AudioDigitalDAIBT daibt_attribute;
    memset((void *)&daibt_attribute, 0, sizeof(daibt_attribute));

    if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true)
    {
        daibt_attribute.mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_MGRIF;
    }
    else
    {
        daibt_attribute.mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_BT;
    }
    daibt_attribute.mDAI_BT_MODE = (sample_rate == 8000) ? AudioDigitalDAIBT::Mode8K : AudioDigitalDAIBT::Mode16K;
    daibt_attribute.mDAI_DEL = AudioDigitalDAIBT::HighWord; // suggest always HighWord
    daibt_attribute.mBT_LEN  = WCNChipController::GetInstance()->BTChipSyncLength();
    daibt_attribute.mDATA_RDY = true;
    daibt_attribute.mBT_SYNC = WCNChipController::GetInstance()->BTChipSyncFormat();
    daibt_attribute.mBT_ON = true;
    daibt_attribute.mDAIBT_ON = false;
    mAudioDigitalInstance->SetDAIBBT(&daibt_attribute);
    return NO_ERROR;
}
//#if defined(BTCVSD_LOOPBACK_WITH_CODEC)
#if 1 //0902

extern void CVSDLoopbackResetBuffer();
extern void CVSDLoopbackReadDataDone(uint32_t len);
extern void CVSDLoopbackGetReadBuffer(uint8_t **buffer, uint32_t *buf_len);
extern int32_t CVSDLoopbackGetDataCount();

AudioLoopbackController::AudioMTKLoopbackThread::AudioMTKLoopbackThread()
{
    ALOGD("BT_SW_CVSD AP loopback AudioMTKLoopbackThread constructor");
}

AudioLoopbackController::AudioMTKLoopbackThread::~AudioMTKLoopbackThread()
{
    ALOGD("BT_SW_CVSD AP loopback ~AudioMTKLoopbackThread");
}

void AudioLoopbackController::AudioMTKLoopbackThread::onFirstRef()
{
    ALOGD("BT_SW_CVSD AP loopback AudioMTKLoopbackThread::onFirstRef");
    run(mName, ANDROID_PRIORITY_URGENT_AUDIO);
}

status_t  AudioLoopbackController::AudioMTKLoopbackThread::readyToRun()
{
    ALOGD("BT_SW_CVSD AP loopback AudioMTKLoopbackThread::readyToRun()");
    return NO_ERROR;
}

bool AudioLoopbackController::AudioMTKLoopbackThread::threadLoop()
{
    uint8_t *pReadBuffer;
    uint32_t uReadByte, uWriteDataToBT;
    CVSDLoopbackResetBuffer();
    while (!(exitPending() == true))
    {
        ALOGD("BT_SW_CVSD AP loopback threadLoop(+)");
        uWriteDataToBT = 0;
        CVSDLoopbackGetReadBuffer(&pReadBuffer, &uReadByte);
        uReadByte &= 0xFFFFFFFE;
        if (uReadByte)
        {
            uWriteDataToBT = streamOutput->write(pReadBuffer, uReadByte);
            CVSDLoopbackReadDataDone(uWriteDataToBT);
        }
        else
        {
            usleep(5 * 1000); //5ms
        }
        ALOGD("BT_SW_CVSD AP loopback threadLoop(-), uReadByte: %d, uWriteDataToBT: %d", uReadByte, uWriteDataToBT);
    }
    ALOGD("BT_SW_CVSD AP loopback threadLoop exit");
    return false;
}

#endif

} // end of namespace android
