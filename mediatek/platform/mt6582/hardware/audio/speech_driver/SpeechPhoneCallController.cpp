#include "SpeechPhoneCallController.h"
#include "AudioType.h"

#include <media/AudioSystem.h>
#include <cutils/properties.h>

#include <utils/threads.h>

#include "SpeechDriverInterface.h"
#include "SpeechEnhancementController.h"
#include "SpeechPcm2way.h"
#include "SpeechBGSPlayer.h"
#include "SpeechVMRecorder.h"
#include <DfoDefines.h>

#include "WCNChipController.h"

#define LOG_TAG "SpeechPhoneCallController"


// TODO(Andrew Hsu): MUST connect (DAIBT) <=> (MODEM_PCM_1 / MODEM_PCM_2) when next ECO!!
#define DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM

namespace android
{

/*==============================================================================
 *                     Property keys
 *============================================================================*/
const char PROPERTY_KEY_VT_NEED_ON[PROPERTY_KEY_MAX]  = "af.recovery.vt_need_on";
const char PROPERTY_KEY_MIC_MUTE_ON[PROPERTY_KEY_MAX] = "af.recovery.mic_mute_on";


SpeechPhoneCallController *SpeechPhoneCallController::mSpeechPhoneCallController = NULL;
SpeechPhoneCallController *SpeechPhoneCallController::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechPhoneCallController == NULL)
    {
        mSpeechPhoneCallController = new SpeechPhoneCallController();
    }
    ASSERT(mSpeechPhoneCallController != NULL);
    return mSpeechPhoneCallController;
}

SpeechPhoneCallController::SpeechPhoneCallController()
{
    // VT flag
    mVtNeedOn = false;

    // Need Mute Mic
    char mic_mute_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_MIC_MUTE_ON, mic_mute_on, "0"); //"0": default off
    mMicMute = (mic_mute_on[0] == '0') ? false : true;

    // get resource manager instance
    mAudioResourceManager = AudioResourceManager::getInstance();

    // get analog control instance
    // mAudioAnalogInstance  = AudioAnalogControlFactory::CreateAudioAnalogControl();

    // get digital control instnace
    mAudioDigitalInstance = AudioDigitalControlFactory::CreateAudioDigitalControl();

    // get volume instance
    mAudioVolumeInstance = AudioVolumeFactory::CreateAudioVolumeController();

    // get speech driver factory
    mSpeechDriverFactory = SpeechDriverFactory::GetInstance();

    // tty
    mRoutingForTty = AUDIO_DEVICE_OUT_EARPIECE;
    mTty_Ctm = AUD_TTY_OFF;

    // BT mode, 0:NB, 1:WB
    mBTMode = 0;
}

SpeechPhoneCallController::~SpeechPhoneCallController()
{

}

bool SpeechPhoneCallController::IsModeIncall(const audio_mode_t audio_mode) const
{
    return (audio_mode == AUDIO_MODE_IN_CALL || audio_mode == AUDIO_MODE_IN_CALL_2  || audio_mode == AUDIO_MODE_IN_CALL_EXTERNAL);
}


bool SpeechPhoneCallController::CheckTtyNeedOn() const
{
#ifdef MTK_TTY_SUPPORT
    return (mVtNeedOn == false && mTty_Ctm != AUD_TTY_OFF && mTty_Ctm != AUD_TTY_ERR);
#else
    return false;
#endif
}

bool SpeechPhoneCallController::CheckSideToneFilterNeedOn(const audio_devices_t output_device) const
{
    // TTY do not use STMF. Open only for earphone & receiver when side tone != 0.
    return ((CheckTtyNeedOn() == false) &&
            (mAudioVolumeInstance->GetSideToneGain(output_device) != 0) &&
            (output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
             output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET ||
             output_device == AUDIO_DEVICE_OUT_EARPIECE));
}

status_t SpeechPhoneCallController::OpenModemSpeechControlFlow(const audio_mode_t audio_mode)
{
    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), audio_mode = %d", __FUNCTION__, audio_mode);

    if (IsModeIncall(audio_mode) == false)
    {
        ALOGE("-%s() new_mode(%d) != MODE_IN_CALL / MODE_IN_CALL_2", __FUNCTION__, audio_mode);
        return INVALID_OPERATION;
    }

    // get speech driver instance
    mSpeechDriverFactory->SetActiveModemIndexByAudioMode(audio_mode);
    const modem_index_t    modem_index   = mSpeechDriverFactory->GetActiveModemIndex();
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();


    // check BT device
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)mAudioResourceManager->getDlOutputDevice());
#if 1
    int sample_rate;
    if (bt_device_on == true)
    {
        if (mBTMode == 0) //NB BTSCO
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
        sample_rate = 16000;
    }
    ALOGD("+%s(), bt_device_on = %d, sample_rate = %d", __FUNCTION__, bt_device_on, sample_rate);
#else
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 16000; // TODO: MT6628 BT only use NB
#endif

#ifdef EXT_MODEM_BT_CVSD
    if (!bt_device_on || modem_index != MODEM_EXTERNAL)
    {
#endif
        // enable clock
        SetAfeAnalogClock(true);

        // set sampling rate
        mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, sample_rate);
        mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC,  sample_rate);
#ifdef EXT_MODEM_BT_CVSD
    }
#endif

    // set device
    if (CheckTtyNeedOn() == true)
    {
        SetTtyInOutDevice(GetRoutingForTty(), mTty_Ctm, audio_mode);
    }
    else
    {
        // Note: set output device in phone call will also assign input device
        mAudioResourceManager->setDlOutputDevice(mAudioResourceManager->getDlOutputDevice());
    }

    // get device
    const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
    const audio_devices_t input_device  = (audio_devices_t)mAudioResourceManager->getUlInputDevice();
    ALOGD("%s(), output_device = 0x%x, input_device = 0x%x", __FUNCTION__, output_device, input_device);

    // Open ADC/DAC I2S, or DAIBT
    OpenModemSpeechDigitalPart(modem_index, output_device);

    // AFE_ON
    mAudioDigitalInstance->SetAfeEnable(true);



    // Clean Side Tone Filter gain
    pSpeechDriver->SetSidetoneGain(0);

    // Set PMIC digital/analog part - uplink has pop, open first
    mAudioResourceManager->StartInputDevice();


    // Set MODEM_PCM - open modem pcm here s.t. modem/DSP can learn the uplink background noise, but not zero
    SetModemPcmAttribute(modem_index, sample_rate);
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, true);


    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech mode
    pSpeechDriver->SetSpeechMode(input_device, output_device);

    // Speech/VT on
    if (mVtNeedOn == true)
    {
        pSpeechDriver->VideoTelephonyOn();

        // trun on P2W for Video Telephony
        bool wideband_on = false; // VT default use Narrow Band (8k), modem side will SRC to 16K
        pSpeechDriver->PCM2WayOn(wideband_on);
    }
    else
    {
        pSpeechDriver->SpeechOn();

        // turn on TTY
        if (CheckTtyNeedOn() == true)
        {
            pSpeechDriver->TtyCtmOn(BAUDOT_MODE);
        }
    }


    // Set PMIC digital/analog part - DL need trim code.
    mAudioResourceManager->StartOutputDevice(); // also set volume here

    // start Side Tone Filter
    if (CheckSideToneFilterNeedOn(output_device) == true)
    {
        mAudioDigitalInstance->EnableSideToneFilter(true);
    }


    // check VM need open
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::GetInstance();
    if (pSpeechVMRecorder->GetVMRecordCapability() == true)
    {
        ALOGD("%s(), Open VM/EPL record", __FUNCTION__);
        pSpeechVMRecorder->Open();
    }

    ALOGD("-%s(), audio_mode = %d", __FUNCTION__, audio_mode);
    return NO_ERROR;
}


status_t SpeechPhoneCallController::CloseModemSpeechControlFlow(const audio_mode_t audio_mode)
{
    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), audio_mode = %d", __FUNCTION__, audio_mode);

    const modem_index_t modem_index = mSpeechDriverFactory->GetActiveModemIndex();
    ASSERT((modem_index == MODEM_1 && audio_mode == AUDIO_MODE_IN_CALL) ||
           (modem_index == MODEM_2 && audio_mode == AUDIO_MODE_IN_CALL_2) ||
           (modem_index == MODEM_EXTERNAL && audio_mode == AUDIO_MODE_IN_CALL_EXTERNAL));

    // check VM need close
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::GetInstance();
    if (pSpeechVMRecorder->GetVMRecordStatus() == true)
    {
        ALOGD("%s(), Close VM/EPL record", __FUNCTION__);
        pSpeechVMRecorder->Close();
    }

    // Stop PMIC digital/analog part - downlink
    mAudioResourceManager->StopOutputDevice();

    // Stop Side Tone Filter
    mAudioDigitalInstance->EnableSideToneFilter(false);

    // Stop MODEM_PCM
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, false);

    // Stop PMIC digital/analog part - uplink
    mAudioResourceManager->StopInputDevice();

    // Stop AP side digital part
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)mAudioResourceManager->getDlOutputDevice());
    CloseModemSpeechDigitalPart(modem_index, (audio_devices_t)mAudioResourceManager->getDlOutputDevice());



    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    // check BGS need close
    if (pSpeechDriver->GetApSideModemStatus(BGS_STATUS_MASK) == true)
    {
        pSpeechDriver->BGSoundOff();
    }

    // Speech/VT off
    if (pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK) == true)
    {
        pSpeechDriver->PCM2WayOff();
        pSpeechDriver->VideoTelephonyOff();
    }
    else if (pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true)
    {
        if (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == true)
        {
            pSpeechDriver->TtyCtmOff();
        }
        pSpeechDriver->SpeechOff();
    }
    else
    {
        ALOGE("%s(), audio_mode = %d, Speech & VT are already closed!!", __FUNCTION__, audio_mode);
        ASSERT(pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK)     == true ||
               pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true);
    }

    // AFE_ON = false
    mAudioDigitalInstance->SetAfeEnable(false);

#ifdef EXT_MODEM_BT_CVSD
    if (!bt_device_on || modem_index != MODEM_EXTERNAL)
    {
#endif
        // recover sampling rate
        mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, 44100);
        mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC, 44100);

        // disable clock
        SetAfeAnalogClock(false);
#ifdef EXT_MODEM_BT_CVSD
    }
#endif

    // clean VT status
    if (mVtNeedOn == true)
    {
        ALOGD("%s(), Set mVtNeedOn = false");
        mVtNeedOn = false;
    }

    ALOGD("-%s(), audio_mode = %d", __FUNCTION__, audio_mode);

    return NO_ERROR;
}


status_t SpeechPhoneCallController::ChangeDeviceForModemSpeechControlFlow(const audio_mode_t audio_mode, const audio_devices_t new_device)
{
    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), audio_mode = %d, new_device = 0x%x", __FUNCTION__, audio_mode, new_device);

    const modem_index_t modem_index = mSpeechDriverFactory->GetActiveModemIndex();
    ASSERT((modem_index == MODEM_1 && audio_mode == AUDIO_MODE_IN_CALL) ||
           (modem_index == MODEM_2 && audio_mode == AUDIO_MODE_IN_CALL_2) ||
           (modem_index == MODEM_EXTERNAL && audio_mode == AUDIO_MODE_IN_CALL_EXTERNAL));

    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();


    // Mute during device change.
    pSpeechDriver->SetDownlinkMute(true);
    pSpeechDriver->SetUplinkMute(true);


    // Stop PMIC digital/analog part - downlink
    mAudioResourceManager->StopOutputDevice();

    // Stop Side Tone Filter
    mAudioDigitalInstance->EnableSideToneFilter(false);

    // Stop MODEM_PCM
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, false);

    // Stop PMIC digital/analog part - uplink
    mAudioResourceManager->StopInputDevice();

    // Stop AP side digital part
    CloseModemSpeechDigitalPart(modem_index, (audio_devices_t)mAudioResourceManager->getDlOutputDevice());




    // Set new device
    if (CheckTtyNeedOn() == true)
    {
        SetTtyInOutDevice(GetRoutingForTty(), mTty_Ctm, audio_mode);
    }
    else
    {
        mAudioResourceManager->setDlOutputDevice(new_device);
    }

    // Get new device
    const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
    const audio_devices_t input_device  = (audio_devices_t)mAudioResourceManager->getUlInputDevice();
    ALOGD("%s(), output_device = 0x%x, input_device = 0x%x", __FUNCTION__, output_device, input_device);



    // Check BT device
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);

#if 1
    int sample_rate;
    if (bt_device_on == true)
    {
        if (mBTMode == 0) //NB BTSCO
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
        sample_rate = 16000;
    }
    ALOGD("+%s(), bt_device_on = %d, sample_rate = %d", __FUNCTION__, bt_device_on, sample_rate);
#else
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 16000; // TODO: MT6628 BT only use NB
#endif

#ifdef EXT_MODEM_BT_CVSD
    if (!bt_device_on || modem_index != MODEM_EXTERNAL)
    {
#endif
        // Set sampling rate
        mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, sample_rate);
        mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC,  sample_rate);
#ifdef EXT_MODEM_BT_CVSD
    }
#endif

    // Open ADC/DAC I2S, or DAIBT
    OpenModemSpeechDigitalPart(modem_index, output_device);

    // Clean Side Tone Filter gain
    pSpeechDriver->SetSidetoneGain(0);

    // Set PMIC digital/analog part - uplink has pop, open first
    mAudioResourceManager->StartInputDevice();

    // Set PMIC digital/analog part - DL need trim code.
    mAudioResourceManager->StartOutputDevice(); // also set volume here

    // start Side Tone Filter
    if (CheckSideToneFilterNeedOn(output_device) == true)
    {
        mAudioDigitalInstance->EnableSideToneFilter(true);
    }



    // Set MODEM_PCM - open modem pcm here s.t. modem/DSP can learn the uplink background noise, but not zero
    SetModemPcmAttribute(modem_index, sample_rate);
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, true);



    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech mode
    pSpeechDriver->SetSpeechMode(input_device, output_device);

    // Need recover mute state
    pSpeechDriver->SetUplinkMute(mMicMute);
    pSpeechDriver->SetDownlinkMute(false);

    ALOGD("-%s(), audio_mode = %d", __FUNCTION__, audio_mode);
    return NO_ERROR;
}


status_t SpeechPhoneCallController::SetAfeAnalogClock(const bool clock_on)
{
    if (clock_on == true)
    {
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);
    }
    else
    {
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    }
    return NO_ERROR;
}

status_t SpeechPhoneCallController::OpenModemSpeechDigitalPart(const modem_index_t modem_index, const audio_devices_t output_device)
{
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);
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
        sample_rate = 16000;
    }

    if (bt_device_on) // DAIBT
    {
        if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
        {
            if (modem_index != MODEM_EXTERNAL)
            {

                SetModemSpeechInterConnection(AudioDigitalType::DAI_BT, modem_index, AudioDigitalType::Connection);

                SetModemSpeechDAIBTAttribute(sample_rate);
                mAudioDigitalInstance->SetDAIBTEnable(true);

#ifdef DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM // DAIBT <-> HW_GAIN2 <-> MODEM_PCM
                AudioMEMIFAttribute::SAMPLINGRATE mem_sample_rate =
                    (sample_rate == 8000) ? AudioMEMIFAttribute::AFE_8000HZ : AudioMEMIFAttribute::AFE_16000HZ;

                // SET HW_GAIN2
                mAudioDigitalInstance->SetHwDigitalGainMode(AudioDigitalType::HW_DIGITAL_GAIN2, mem_sample_rate, 0xC8);
                mAudioDigitalInstance->SetHwDigitalGain(0x80000, AudioDigitalType::HW_DIGITAL_GAIN2);
                mAudioDigitalInstance->SetHwDigitalGainEnable(AudioDigitalType::HW_DIGITAL_GAIN2, true);

                // USE HW_GAIN2 MUST TURN ON "MASTER/SLAVE" I2S IN/OUT
                AudioDigtalI2S master_2nd_i2s_out_attribute;
                memset((void *)&master_2nd_i2s_out_attribute, 0, sizeof(master_2nd_i2s_out_attribute));
                master_2nd_i2s_out_attribute.mI2S_SAMPLERATE = sample_rate;
                master_2nd_i2s_out_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
                master_2nd_i2s_out_attribute.mI2S_FMT  = AudioDigtalI2S::I2S;
                master_2nd_i2s_out_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
                mAudioDigitalInstance->Set2ndI2SOut(&master_2nd_i2s_out_attribute);
#if 1
                mAudioDigitalInstance->Set2ndI2SOutEnable(true);
#else
                mAudioDigitalInstance->Set2ndI2SEnable(true);
#endif
#endif
            }
            else
            {
                mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
                if (!mAudioBTCVSDControl)
                {
                    ALOGE("OpenModemSpeechDigitalPart() AudioBTCVSDControl::getInstance() fail");
                }
                mAudioBTCVSDControl->AudioExtMDCVSDCreateThread();
            }
        }
        else
        {
            SetModemSpeechInterConnection(AudioDigitalType::DAI_BT, modem_index, AudioDigitalType::Connection);

            SetModemSpeechDAIBTAttribute(sample_rate);
            mAudioDigitalInstance->SetDAIBTEnable(true);

#ifdef DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM // DAIBT <-> HW_GAIN2 <-> MODEM_PCM
            AudioMEMIFAttribute::SAMPLINGRATE mem_sample_rate =
                (sample_rate == 8000) ? AudioMEMIFAttribute::AFE_8000HZ : AudioMEMIFAttribute::AFE_16000HZ;

            // SET HW_GAIN2
            mAudioDigitalInstance->SetHwDigitalGainMode(AudioDigitalType::HW_DIGITAL_GAIN2, mem_sample_rate, 0xC8);
            mAudioDigitalInstance->SetHwDigitalGain(0x80000, AudioDigitalType::HW_DIGITAL_GAIN2);
            mAudioDigitalInstance->SetHwDigitalGainEnable(AudioDigitalType::HW_DIGITAL_GAIN2, true);

            // USE HW_GAIN2 MUST TURN ON "MASTER/SLAVE" I2S IN/OUT
            AudioDigtalI2S master_2nd_i2s_out_attribute;
            memset((void *)&master_2nd_i2s_out_attribute, 0, sizeof(master_2nd_i2s_out_attribute));
            master_2nd_i2s_out_attribute.mI2S_SAMPLERATE = sample_rate;
            master_2nd_i2s_out_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
            master_2nd_i2s_out_attribute.mI2S_FMT  = AudioDigtalI2S::I2S;
            master_2nd_i2s_out_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
            mAudioDigitalInstance->Set2ndI2SOut(&master_2nd_i2s_out_attribute);
#if 1
            mAudioDigitalInstance->Set2ndI2SOutEnable(true);
#else
            mAudioDigitalInstance->Set2ndI2SEnable(true);
#endif
#endif
        }
    }
    else // ADC/DAC I2S
    {
        SetModemSpeechInterConnection(AudioDigitalType::I2S_IN_ADC, modem_index, AudioDigitalType::Connection);
        SetModemSpeechInterConnection(AudioDigitalType::I2S_OUT_DAC, modem_index, AudioDigitalType::Connection);

        SetModemSpeechADCI2sInAttribute(sample_rate);
        SetModemSpeechDACI2sOutAttribute(sample_rate);

        mAudioDigitalInstance->SetI2SAdcEnable(true);
        mAudioDigitalInstance->SetI2SDacEnable(true);
    }

    return NO_ERROR;

}

status_t SpeechPhoneCallController::CloseModemSpeechDigitalPart(const modem_index_t modem_index, const audio_devices_t output_device)
{
    // stop Input/Output module
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);

    if (bt_device_on)
    {
        if (WCNChipController::GetInstance()->BTUseCVSDRemoval() != true)
        {
#ifdef DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM // DAIBT <-> HW_GAIN2 <-> MODEM_PCM
#if 1
            mAudioDigitalInstance->Set2ndI2SOutEnable(false);
#else
            mAudioDigitalInstance->Set2ndI2SEnable(false);
#endif
            mAudioDigitalInstance->SetHwDigitalGainEnable(AudioDigitalType::HW_DIGITAL_GAIN2, false);
#endif
            mAudioDigitalInstance->SetDAIBTEnable(false);
            SetModemSpeechInterConnection(AudioDigitalType::DAI_BT, modem_index, AudioDigitalType::DisConnect);
        }
        else
        {
            if (modem_index == MODEM_EXTERNAL)
            {
                mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
                if (!mAudioBTCVSDControl)
                {
                    ALOGE("AudioBTCVSDControl::getInstance() fail");
                }
                mAudioBTCVSDControl->AudioExtMDCVSDDeleteThread();
            }
            else
            {
#ifdef DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM // DAIBT <-> HW_GAIN2 <-> MODEM_PCM
#if 1
                mAudioDigitalInstance->Set2ndI2SOutEnable(false);
#else
                mAudioDigitalInstance->Set2ndI2SEnable(false);
#endif
                mAudioDigitalInstance->SetHwDigitalGainEnable(AudioDigitalType::HW_DIGITAL_GAIN2, false);
#endif
                mAudioDigitalInstance->SetDAIBTEnable(false);
                SetModemSpeechInterConnection(AudioDigitalType::DAI_BT, modem_index, AudioDigitalType::DisConnect);
            }
        }
    }
    else
    {
        mAudioDigitalInstance->SetI2SDacEnable(false);
        mAudioDigitalInstance->SetI2SAdcEnable(false);
        SetModemSpeechInterConnection(AudioDigitalType::I2S_OUT_DAC, modem_index, AudioDigitalType::DisConnect);
        SetModemSpeechInterConnection(AudioDigitalType::I2S_IN_ADC, modem_index, AudioDigitalType::DisConnect);
    }

    return NO_ERROR;

}


status_t SpeechPhoneCallController::SetModemSpeechInterConnection(AudioDigitalType::Digital_Block block, modem_index_t modem_index, AudioDigitalType::InterConnectionState Connection)
{
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_lch = (modem_index == MODEM_1) ? AudioDigitalType::O17 : AudioDigitalType::O07;
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_rch = (modem_index == MODEM_1) ? AudioDigitalType::O18 : AudioDigitalType::O08;
    AudioDigitalType::InterConnectionInput  modem_pcm_rx     = (modem_index == MODEM_1) ? AudioDigitalType::I14 : AudioDigitalType::I09;

#ifdef DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM
    AudioDigitalType::InterConnectionOutput hw_gain_2_in_lch  = AudioDigitalType::O15;
    AudioDigitalType::InterConnectionOutput hw_gain_2_in_rch  = AudioDigitalType::O16;
    AudioDigitalType::InterConnectionInput  hw_gain_2_out_lch = AudioDigitalType::I12;
    AudioDigitalType::InterConnectionInput  hw_gain_2_out_rch = AudioDigitalType::I13;
#endif

    switch (block)
    {
        case AudioDigitalType::DAI_BT:
#ifndef DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM
            mAudioDigitalInstance->SetinputConnection(Connection, AudioDigitalType::I02, modem_pcm_tx_lch);      // DAIBT_IN       -> MODEM_PCM_TX_L
#else
            mAudioDigitalInstance->SetinputConnection(Connection, AudioDigitalType::I02, hw_gain_2_in_lch);      // DAIBT_IN       -> HW_GAIN2_IN_L
            mAudioDigitalInstance->SetinputConnection(Connection, AudioDigitalType::I02, hw_gain_2_in_rch);      // DAIBT_IN       -> HW_GAIN2_IN_R
            mAudioDigitalInstance->SetinputConnection(Connection, hw_gain_2_out_lch    , modem_pcm_tx_lch);      // HW_GAIN2_OUT_L -> MODEM_PCM_TX_L
            mAudioDigitalInstance->SetinputConnection(Connection, hw_gain_2_out_rch    , modem_pcm_tx_rch);      // HW_GAIN2_OUT_R -> MODEM_PCM_TX_R
#endif
            mAudioDigitalInstance->SetinputConnection(Connection, modem_pcm_rx         , AudioDigitalType::O02); // MODEM_PCM_RX   -> DAIBT_OUT
            break;
        case AudioDigitalType::I2S_IN_ADC:
            mAudioDigitalInstance->SetinputConnection(Connection, AudioDigitalType::I03, modem_pcm_tx_lch);      // ADC_I2S_IN_L   -> MODEM_PCM_TX_L
            mAudioDigitalInstance->SetinputConnection(Connection, AudioDigitalType::I04, modem_pcm_tx_rch);      // ADC_I2S_IN_R   -> MODEM_PCM_TX_R
            break;
        case AudioDigitalType::I2S_OUT_DAC:
            mAudioDigitalInstance->SetinputConnection(Connection, modem_pcm_rx         , AudioDigitalType::O03); // MODEM_PCM_RX   -> DAC_I2S_OUT_L
            mAudioDigitalInstance->SetinputConnection(Connection, modem_pcm_rx         , AudioDigitalType::O04); // MODEM_PCM_RX   -> DAC_I2S_OUT_R
            break;
        default:
            ALOGW("SetModemSpeechInterConnection block = %d", block);
    }
    return NO_ERROR;
}


status_t SpeechPhoneCallController::SetModemSpeechDACI2sOutAttribute(int sample_rate)
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


status_t SpeechPhoneCallController::SetModemSpeechADCI2sInAttribute(int sample_rate)
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


status_t SpeechPhoneCallController::SetModemSpeechDAIBTAttribute(int sample_rate)
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
    daibt_attribute.mBT_LEN  = WCNChipController::GetInstance()->BTChipSyncLength();;
    daibt_attribute.mDATA_RDY = true;
    daibt_attribute.mBT_SYNC = WCNChipController::GetInstance()->BTChipSyncFormat();
    daibt_attribute.mBT_ON = true;
    daibt_attribute.mDAIBT_ON = false;
    mAudioDigitalInstance->SetDAIBBT(&daibt_attribute);
    return NO_ERROR;
}

status_t SpeechPhoneCallController::SetModemPcmAttribute(const modem_index_t modem_index, int sample_rate)
{
    AudioDigitalPCM modem_pcm_attribute;
    memset((void *)&modem_pcm_attribute, 0, sizeof(modem_pcm_attribute));

    // modem 2 only
    if (modem_index == MODEM_2)
    {
        // TODO: only config internal modem here.. Add external modem setting by project config!!
        modem_pcm_attribute.mExtModemSel          = AudioDigitalPCM::MODEM_2_USE_INTERNAL_MODEM;
        modem_pcm_attribute.mSlaveModeSel         = AudioDigitalPCM::SALVE_MODE;
        modem_pcm_attribute.mAsyncFifoSel         = AudioDigitalPCM::SLAVE_USE_ASRC;

        modem_pcm_attribute.mExtendBckSyncTypeSel = AudioDigitalPCM::BCK_CYCLE_SYNC; // short sync
        modem_pcm_attribute.mExtendBckSyncLength  = 0;
        modem_pcm_attribute.mPcmBckInInv  = AudioDigitalPCM::PCM_NO_INVERSE;
    }
#ifdef EVDO_DT_SUPPORT  //overwrite some config when modem is EVDO
    if (modem_index == MODEM_EXTERNAL)
    {
        modem_pcm_attribute.mExtModemSel          = AudioDigitalPCM::MODEM_2_USE_EXTERNAL_MODEM;
        modem_pcm_attribute.mSlaveModeSel         = AudioDigitalPCM::MASTER_MODE;    // pcm is master mode when external modem is EVDO
        modem_pcm_attribute.mAsyncFifoSel         = AudioDigitalPCM::SLAVE_USE_ASRC;
        modem_pcm_attribute.mExtendBckSyncTypeSel = AudioDigitalPCM::BCK_CYCLE_SYNC; // short sync
        modem_pcm_attribute.mExtendBckSyncLength  = 0;
        modem_pcm_attribute.mPcmBckInInv  = AudioDigitalPCM::PCM_NO_INVERSE;
    }
#endif
    if (MTK_ENABLE_MD5 == true)
    {
        // modem external (MT6290): Slave mode, bclock input inverse
        if (modem_index == MODEM_EXTERNAL)
        {
            // TODO: only config internal modem here.. Add external modem setting by project config!!
            modem_pcm_attribute.mExtModemSel          = AudioDigitalPCM::MODEM_2_USE_EXTERNAL_MODEM;
            modem_pcm_attribute.mSlaveModeSel         = AudioDigitalPCM::SALVE_MODE;
            modem_pcm_attribute.mAsyncFifoSel         = AudioDigitalPCM::SLAVE_USE_ASRC;

            modem_pcm_attribute.mExtendBckSyncTypeSel = AudioDigitalPCM::BCK_CYCLE_SYNC; // short sync
            modem_pcm_attribute.mExtendBckSyncLength  = 0;

            modem_pcm_attribute.mPcmBckInInv  = AudioDigitalPCM::PCM_INVERSE;
        }
    }
    modem_pcm_attribute.mPcmSyncInInv  = AudioDigitalPCM::PCM_NO_INVERSE;
    modem_pcm_attribute.mPcmBckOutInv  = AudioDigitalPCM::PCM_NO_INVERSE;
    modem_pcm_attribute.mPcmSyncOutInv  = AudioDigitalPCM::PCM_NO_INVERSE;

    // here modem_1 & modem_2 use the same config, but register field offset are not the same
    modem_pcm_attribute.mVbt16kModeSel      = AudioDigitalPCM::VBT_16K_MODE_DISABLE;

    modem_pcm_attribute.mSingelMicSel       = AudioDigitalPCM::DUAL_MIC_ON_TX;
    modem_pcm_attribute.mTxLchRepeatSel     = AudioDigitalPCM::TX_LCH_NO_REPEAT;

    modem_pcm_attribute.mPcmWordLength      = AudioDigitalPCM::PCM_16BIT;
    modem_pcm_attribute.mPcmModeWidebandSel = (sample_rate == 8000) ? AudioDigitalPCM::PCM_MODE_8K : AudioDigitalPCM::PCM_MODE_16K;
    modem_pcm_attribute.mPcmFormat          = AudioDigitalPCM::PCM_MODE_B;
    modem_pcm_attribute.mModemPcmOn         = false;

    return mAudioDigitalInstance->SetModemPcmConfig(modem_index, &modem_pcm_attribute);
}

status_t SpeechPhoneCallController::SetTtyCtmMode(const tty_mode_t tty_mode, const audio_mode_t audio_mode)
{
    ALOGD("+%s(), mTty_Ctm = %d, new tty mode = %d, audio_mode = %d", __FUNCTION__, mTty_Ctm, tty_mode, audio_mode);

    if (mTty_Ctm != tty_mode)
    {
        mTty_Ctm = tty_mode;

        SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
        if (pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK) == false &&
            pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true)
        {
            mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
            mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);

            pSpeechDriver->SetUplinkMute(true);
            if (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == true)
            {
                pSpeechDriver->TtyCtmOff();
            }

            mAudioResourceManager->StopOutputDevice();
            mAudioResourceManager->StopInputDevice();
            SetTtyInOutDevice(GetRoutingForTty(), mTty_Ctm, audio_mode);
            mAudioResourceManager->StartOutputDevice();
            mAudioResourceManager->StartInputDevice();

            const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
            const audio_devices_t input_device  = (audio_devices_t)mAudioResourceManager->getUlInputDevice();
            pSpeechDriver->SetSpeechMode(input_device, output_device);

            if ((mTty_Ctm != AUD_TTY_OFF) && (mTty_Ctm != AUD_TTY_ERR) &&
                (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == false))
            {
                pSpeechDriver->TtyCtmOn(BAUDOT_MODE);
            }
            pSpeechDriver->SetUplinkMute(mMicMute);

            mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
            mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
        }
    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void SpeechPhoneCallController::SetTtyInOutDevice(audio_devices_t routing_device, tty_mode_t tty_mode, audio_mode_t audio_mode)
{
    ALOGD("+%s(), routing_device = 0x%x, tty_mode = %d", __FUNCTION__, routing_device, tty_mode);

    if (tty_mode == AUD_TTY_OFF)
    {
        mAudioResourceManager->setDlOutputDevice(routing_device);
    }
    else
    {
        if (routing_device == 0)
        {
            mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_DEFAULT);    // avoid routing=0 cause reamain headset path
        }
        else if (routing_device & AUDIO_DEVICE_OUT_SPEAKER)
        {
            if (tty_mode == AUD_TTY_VCO)
            {
                ALOGD("%s(), speaker, TTY_VCO", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
#else
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
#endif
                mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
                mAudioVolumeInstance->ApplyMicGain(Handfree_Mic, audio_mode);
            }
            else if (tty_mode == AUD_TTY_HCO)
            {
                ALOGD("%s(), speaker, TTY_HCO", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
#else
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_SPEAKER);
#endif
                mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                mAudioVolumeInstance->ApplyMicGain(TTY_CTM_Mic, audio_mode);
            }
            else if (tty_mode == AUD_TTY_FULL)
            {
                ALOGD("%s(), speaker, TTY_FULL", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
#else
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
#endif
                mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                mAudioVolumeInstance->ApplyMicGain(TTY_CTM_Mic, audio_mode);
            }
        }
        else if ((routing_device == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                 (routing_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
        {
            if (tty_mode == AUD_TTY_VCO)
            {
                ALOGD("%s(), headset, TTY_VCO", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
#else
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
#endif
                mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
                mAudioVolumeInstance->ApplyMicGain(Normal_Mic, audio_mode);
            }
            else if (tty_mode == AUD_TTY_HCO)
            {
                ALOGD("%s(), headset, TTY_HCO", __FUNCTION__);
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
                mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                mAudioVolumeInstance->ApplyMicGain(TTY_CTM_Mic, audio_mode);
            }
            else if (tty_mode == AUD_TTY_FULL)
            {
                ALOGD("%s(), headset, TTY_FULL", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
#else
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
#endif
                mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                mAudioVolumeInstance->ApplyMicGain(TTY_CTM_Mic, audio_mode);
            }
        }
        else if (routing_device == AUDIO_DEVICE_OUT_EARPIECE)
        {
            // tty device is removed. TtyCtm already off in CloseMD.
            ALOGD("%s(), receiver", __FUNCTION__);
        }
        else
        {
            mAudioResourceManager->setDlOutputDevice(routing_device);
            ALOGD("%s(), routing = 0x%x", __FUNCTION__, routing_device);
        }
    }

    ALOGD("-%s()", __FUNCTION__);
}

void SpeechPhoneCallController::SetVtNeedOn(const bool vt_on)
{
    Mutex::Autolock _l(mLock);

    ALOGD("%s(), new vt_on = %d, old mVtNeedOn = %d", __FUNCTION__, vt_on, mVtNeedOn);

    property_set(PROPERTY_KEY_VT_NEED_ON, (vt_on == false) ? "0" : "1");
    mVtNeedOn = vt_on;
}

void SpeechPhoneCallController::SetMicMute(const bool mute_on)
{
    Mutex::Autolock _l(mLock);

    ALOGD("%s(), new mute_on = %d, old mMicMute = %d", __FUNCTION__, mute_on, mMicMute);

    mSpeechDriverFactory->GetSpeechDriver()->SetUplinkMute(mute_on);

    property_set(PROPERTY_KEY_MIC_MUTE_ON, (mute_on == false) ? "0" : "1");
    mMicMute = mute_on;
}

void SpeechPhoneCallController::SetBTMode(const int mode)
{
    Mutex::Autolock _l(mLock);

    ALOGD("%s(), mode %d", __FUNCTION__, mode);

    mBTMode = mode;
}

} // end of namespace android
