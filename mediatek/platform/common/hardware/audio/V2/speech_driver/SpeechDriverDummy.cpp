#include "SpeechDriverDummy.h"

#include <utils/threads.h>

#define LOG_TAG "SpeechDriverDummy"

namespace android
{

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechDriverDummy::SpeechDriverDummy(modem_index_t modem_index)
{
    ALOGE("%s(), modem_index = %d", __FUNCTION__, modem_index);
}

SpeechDriverDummy::~SpeechDriverDummy()
{
    ALOGE("%s()", __FUNCTION__);
}

/*==============================================================================
 *                     Speech Control
 *============================================================================*/

status_t SpeechDriverDummy::SetSpeechMode(const audio_devices_t input_device, const audio_devices_t output_device)
{
    ALOGE("%s(), input_device = 0x%x, output_device = 0x%x", __FUNCTION__, input_device, output_device);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SpeechOn()
{
    ALOGE("%s()", __FUNCTION__);
    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(SPEECH_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SpeechOff()
{
    ALOGE("%s()", __FUNCTION__);
    ResetApSideModemStatus(SPEECH_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::VideoTelephonyOn()
{
    ALOGE("%s()", __FUNCTION__);
    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(VT_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::VideoTelephonyOff()
{
    ALOGE("%s()", __FUNCTION__);
    ResetApSideModemStatus(VT_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Recording Control
 *============================================================================*/

status_t SpeechDriverDummy::RecordOn()
{
    ALOGE("%s()", __FUNCTION__);
    SetApSideModemStatus(RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::RecordOff()
{
    ALOGE("%s()", __FUNCTION__);
    ResetApSideModemStatus(RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::VoiceMemoRecordOn()
{
    ALOGE("%s()", __FUNCTION__);
    SetApSideModemStatus(VM_RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::VoiceMemoRecordOff()
{
    ALOGE("%s()", __FUNCTION__);
    ResetApSideModemStatus(VM_RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

uint16_t SpeechDriverDummy::GetRecordSampleRate() const
{
    ALOGE("%s(), num_sample_rate = 8000", __FUNCTION__);
    return 8000;
}

uint16_t SpeechDriverDummy::GetRecordChannelNumber() const
{
    ALOGE("%s(), num_channel = 1", __FUNCTION__);
    return 1;
}


/*==============================================================================
 *                     Background Sound
 *============================================================================*/

status_t SpeechDriverDummy::BGSoundOn()
{
    ALOGE("%s()", __FUNCTION__);
    SetApSideModemStatus(BGS_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::BGSoundConfig(uint8_t ul_gain, uint8_t dl_gain)
{
    ALOGE("%s(), ul_gain = 0x%x, dl_gain = 0x%x", __FUNCTION__, ul_gain, dl_gain);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::BGSoundOff()
{
    ALOGE("%s()", __FUNCTION__);
    ResetApSideModemStatus(BGS_STATUS_MASK);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     PCM 2 Way
 *============================================================================*/
status_t SpeechDriverDummy::PCM2WayPlayOn()
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}


status_t SpeechDriverDummy::PCM2WayPlayOff()
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}


status_t SpeechDriverDummy::PCM2WayRecordOn()
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}


status_t SpeechDriverDummy::PCM2WayRecordOff()
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}


status_t SpeechDriverDummy::PCM2WayOn(const bool wideband_on)
{
    ALOGE("%s()", __FUNCTION__);
    SetApSideModemStatus(P2W_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::PCM2WayOff()
{
    ALOGE("%s()", __FUNCTION__);
    ResetApSideModemStatus(P2W_STATUS_MASK);
    return INVALID_OPERATION;
}

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
status_t SpeechDriverDummy::DualMicPCM2WayOn(const bool wideband_on, const bool record_only)
{
    ALOGE("%s(), wideband_on = %d, record_only = %d", __FUNCTION__, wideband_on, record_only);
    SetApSideModemStatus(P2W_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::DualMicPCM2WayOff()
{
    ALOGE("%s()", __FUNCTION__);
    ResetApSideModemStatus(P2W_STATUS_MASK);
    return INVALID_OPERATION;
}
#endif

/*==============================================================================
 *                     TTY-CTM Control
 *============================================================================*/
status_t SpeechDriverDummy::TtyCtmOn(ctm_interface_t ctm_interface)
{
    ALOGE("%s(), ctm_interface = %d, force set to BAUDOT_MODE = %d", __FUNCTION__, ctm_interface, BAUDOT_MODE);
    SetApSideModemStatus(TTY_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::TtyCtmOff()
{
    ALOGE("%s()", __FUNCTION__);
    ResetApSideModemStatus(TTY_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::TtyCtmDebugOn(bool tty_debug_flag)
{
    ALOGE("%s(), tty_debug_flag = %d", __FUNCTION__, tty_debug_flag);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Acoustic Loopback
 *============================================================================*/

status_t SpeechDriverDummy::SetAcousticLoopback(bool loopback_on)
{
    ALOGE("%s(), loopback_on = %d", __FUNCTION__, loopback_on);

    if (loopback_on == true)
    {
        CheckApSideModemStatusAllOffOrDie();
        SetApSideModemStatus(LOOPBACK_STATUS_MASK);
    }
    else
    {
        ResetApSideModemStatus(LOOPBACK_STATUS_MASK);
        CheckApSideModemStatusAllOffOrDie();
    }

    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetAcousticLoopbackBtCodec(bool enable_codec)
{
    ALOGE("%s(), enable_codec = %d", __FUNCTION__, enable_codec);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetAcousticLoopbackDelayFrames(int32_t delay_frames)
{
    ALOGE("%s(), delay_frames = %d", __FUNCTION__, delay_frames);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Volume Control
 *============================================================================*/

status_t SpeechDriverDummy::SetDownlinkGain(int16_t gain)
{
    ALOGE("%s(), gain = 0x%x, old mDownlinkGain = 0x%x", __FUNCTION__, gain, mDownlinkGain);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetEnh1DownlinkGain(int16_t gain)
{
    ALOGE("%s(), gain = 0x%x, old SetEnh1DownlinkGain = 0x%x", __FUNCTION__, gain, mDownlinkenh1Gain);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetUplinkGain(int16_t gain)
{
    ALOGE("%s(), gain = 0x%x, old mUplinkGain = 0x%x", __FUNCTION__, gain, mUplinkGain);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetDownlinkMute(bool mute_on)
{
    ALOGE("%s(), mute_on = %d, old mDownlinkMuteOn = %d", __FUNCTION__, mute_on, mDownlinkMuteOn);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetUplinkMute(bool mute_on)
{
    ALOGE("%s(), mute_on = %d, old mUplinkMuteOn = %d", __FUNCTION__, mute_on, mUplinkMuteOn);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetSidetoneGain(int16_t gain)
{
    ALOGE("%s(), gain = 0x%x, old mSideToneGain = 0x%x", __FUNCTION__, gain, mSideToneGain);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Device related Config
 *============================================================================*/

status_t SpeechDriverDummy::SetModemSideSamplingRate(uint16_t sample_rate)
{
    ALOGE("%s(), sample_rate = %d", __FUNCTION__, sample_rate);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Speech Enhancement Control
 *============================================================================*/
status_t SpeechDriverDummy::SetSpeechEnhancement(bool enhance_on)
{
    ALOGE("%s(), enhance_on = %d", __FUNCTION__, enhance_on);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetSpeechEnhancementMask(const sph_enh_mask_struct_t &mask)
{
    ALOGE("%s(), main_func = 0x%x, dynamic_func = 0x%x", __FUNCTION__, mask.main_func, mask.dynamic_func);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetBtHeadsetNrecOn(const bool bt_headset_nrec_on)
{
    ALOGE("%s(), bt_headset_nrec_on = %d", __FUNCTION__, bt_headset_nrec_on);
    return INVALID_OPERATION;
}
/*==============================================================================
 *                     Speech Enhancement Parameters
 *============================================================================*/

status_t SpeechDriverDummy::SetNBSpeechParameters(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}

#if defined(MTK_DUAL_MIC_SUPPORT)
status_t SpeechDriverDummy::SetDualMicSpeechParameters(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic)
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
status_t SpeechDriverDummy::SetWBSpeechParameters(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB)
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}
#endif

//#if defined(MTK_VIBSPK_SUPPORT)
status_t SpeechDriverDummy::GetVibSpkParam(void *eVibSpkParam)
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetVibSpkParam(void *eVibSpkParam)
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}
//#endif //defined(MTK_VIBSPK_SUPPORT)

status_t SpeechDriverDummy::GetNxpSmartpaParam(void *eParamNxpSmartpa)
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetNxpSmartpaParam(void *eParamNxpSmartpa)
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Recover State
 *============================================================================*/

void SpeechDriverDummy::RecoverModemSideStatusToInitState()
{
    ALOGE("%s()", __FUNCTION__);
}

/*==============================================================================
 *                     Check Modem Status
 *============================================================================*/
bool SpeechDriverDummy::CheckModemIsReady()
{
    ALOGE("%s()", __FUNCTION__);
    return false;
};

/*==============================================================================
 *                     Debug Info
 *============================================================================*/
status_t SpeechDriverDummy::ModemDumpSpeechParam()
{
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}

} // end of namespace android

