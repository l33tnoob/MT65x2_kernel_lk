#include "SpeechLoopbackController.h"
#include <utils/threads.h>

#define LOG_TAG "SpeechLoopbackController"

// for use max gain for speech loopback
#define SPEECH_LOOPBACK_USE_MAX_GAIN

namespace android
{

static const float kMaxVoiceVolume = 1.0;

SpeechLoopbackController *SpeechLoopbackController::mSpeechLoopbackController = NULL;
SpeechLoopbackController *SpeechLoopbackController::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechLoopbackController == NULL)
    {
        mSpeechLoopbackController = new SpeechLoopbackController();
    }
    ASSERT(mSpeechLoopbackController != NULL);
    return mSpeechLoopbackController;
}

SpeechLoopbackController::SpeechLoopbackController()
{
    mVoiceVolumeCopy = 1.0;
    mUseBtCodec = 1;
}

SpeechLoopbackController::~SpeechLoopbackController()
{

}

status_t SpeechLoopbackController::SetModemBTCodec(bool enable_codec)
{
    ALOGD("+%s(), enable_codec = %d", __FUNCTION__, enable_codec);
    mUseBtCodec = enable_codec;
    return NO_ERROR;
}

status_t SpeechLoopbackController::OpenModemLoopbackControlFlow(const modem_index_t modem_index, const audio_devices_t input_device, const audio_devices_t output_device)
{
    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), modem_index = %d, input_device = 0x%x, output_device = 0x%x", __FUNCTION__, modem_index, input_device, output_device);

    ASSERT(mAudioResourceManager->GetAudioMode() == AUDIO_MODE_NORMAL);

    // pretend to be phone call state
    if (modem_index == MODEM_1)
    {
        mAudioResourceManager->SetAudioMode(AUDIO_MODE_IN_CALL);
    }
    else if (modem_index == MODEM_2)
    {
        mAudioResourceManager->SetAudioMode(AUDIO_MODE_IN_CALL_2);
    }
    else    //External modem
    {
        mAudioResourceManager->SetAudioMode(AUDIO_MODE_IN_CALL_EXTERNAL);
    }
    // get speech driver instance
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriverByIndex(modem_index);

    // check BT device
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 16000; // TODO: MT6628 BT only use NB

    // enable clock
    SetAfeAnalogClock(true);

    // set sampling rate
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, sample_rate);
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC, sample_rate);

    // set device
    mAudioResourceManager->setDlOutputDevice(output_device);
    mAudioResourceManager->setUlInputDevice(input_device);

    // Open ADC/DAC I2S, or DAIBT
    OpenModemSpeechDigitalPart(modem_index, output_device);

    // AFE_ON
    mAudioDigitalInstance->SetAfeEnable(true);

    // Set PMIC digital/analog part - uplink has pop, open first
    mAudioResourceManager->StartInputDevice();

    // set MODEM_PCM - open modem pcm here s.t. modem/DSP can learn the uplink background noise, but not zero
    SetModemPcmAttribute(modem_index, sample_rate);
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, true);



    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech audio_mode
    pSpeechDriver->SetSpeechMode(input_device, output_device);

    // Set Loopback on
    pSpeechDriver->SetAcousticLoopbackBtCodec(mUseBtCodec);
    pSpeechDriver->SetAcousticLoopback(true);



    // Set PMIC digital/analog part - DL need trim code.
    mAudioResourceManager->StartOutputDevice();

#ifdef SPEECH_LOOPBACK_USE_MAX_GAIN
    // adjust volume
    mVoiceVolumeCopy = mAudioVolumeInstance->getVoiceVolume();
    mAudioVolumeInstance->setVoiceVolume(kMaxVoiceVolume, mAudioResourceManager->GetAudioMode(), output_device);
#endif

    ALOGD("-%s(), modem_index = %d, input_device = 0x%x, output_device = 0x%x", __FUNCTION__, modem_index, input_device, output_device);
    return NO_ERROR;
}


status_t SpeechLoopbackController::CloseModemLoopbackControlFlow(const modem_index_t modem_index)
{
    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), modem_index = %d", __FUNCTION__, modem_index);

    // Stop PMIC digital/analog part - downlink
    mAudioResourceManager->StopOutputDevice();

    // Stop MODEM_PCM
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, false);

    // Stop PMIC digital/analog part - uplink
    mAudioResourceManager->StopInputDevice();

    // Stop AP side digital part
    const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
    CloseModemSpeechDigitalPart(modem_index, output_device);



    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriverByIndex(modem_index);
    ASSERT(pSpeechDriver->GetApSideModemStatus(LOOPBACK_STATUS_MASK) == true);

    // Set Loopback off
    pSpeechDriver->SetAcousticLoopback(false);

    // AFE_ON = false
    mAudioDigitalInstance->SetAfeEnable(false);

    // recover sampling rate
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, 44100);
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC, 44100);

#ifdef SPEECH_LOOPBACK_USE_MAX_GAIN
    // recover volume
    mAudioVolumeInstance->setVoiceVolume(mVoiceVolumeCopy, mAudioResourceManager->GetAudioMode(), output_device);
#endif

    // recover phone call state to normal mode
    mAudioResourceManager->SetAudioMode(AUDIO_MODE_NORMAL);

    // disable clock
    SetAfeAnalogClock(false);

    ALOGD("-%s(), modem_index_t = %d", __FUNCTION__, modem_index);

    return NO_ERROR;
}

} // end of namespace android
