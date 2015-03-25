#include "AudioMATVResourceManager.h"

#include "AudioMTKStreamManager.h"
#include "AudioMTKVolumeController.h"

#define LOG_TAG "AudioMATVResourceManager"

namespace android
{

/*==============================================================================
 *                     Const Value
 *============================================================================*/

static const uint16_t kMatvChipSamplingRateHz = 32000;

static const uint16_t kMatvUplinkSamplingRateHz = kMatvChipSamplingRateHz;
static const uint16_t kMatvDownlinkSamplingRateHz = 44100;

static const AudioDigtalI2S::I2S_SRC kMatvChipClockSource = AudioDigtalI2S::MASTER_MODE;

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioMATVResourceManager::AudioMATVResourceManager()
{
    ALOGD("%s()", __FUNCTION__);
}

AudioMATVResourceManager::~AudioMATVResourceManager()
{
    ALOGD("%s()", __FUNCTION__);
}


/*==============================================================================
 *                     Audio HW Control
 *============================================================================*/

uint32_t AudioMATVResourceManager::GetMatvUplinkSamplingRate() const
{
    ALOGD("%s(), sampling rate = %d", __FUNCTION__, kMatvUplinkSamplingRateHz);
    return kMatvUplinkSamplingRateHz;
}

uint32_t AudioMATVResourceManager::GetMatvDownlinkSamplingRate() const
{
    ALOGD("%s(), sampling rate = %d", __FUNCTION__, kMatvDownlinkSamplingRateHz);
    return kMatvDownlinkSamplingRateHz;
}

status_t AudioMATVResourceManager::SetMatvSourceModuleEnable(const bool enable)
{
    ALOGD("+%s(), enable = %d", __FUNCTION__, enable);

    if (enable == true)
    {
        // Config 2nd I2S IN
        AudioDigtalI2S m2ndI2SInAttribute;
        memset((void *)&m2ndI2SInAttribute, 0, sizeof(m2ndI2SInAttribute));

        m2ndI2SInAttribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
        m2ndI2SInAttribute.mI2S_IN_PAD_SEL = AudioDigtalI2S::I2S_IN_FROM_IO_MUX; // 72,82 only
        m2ndI2SInAttribute.mI2S_SLAVE = kMatvChipClockSource;
        m2ndI2SInAttribute.mI2S_SAMPLERATE = kMatvChipSamplingRateHz;
        m2ndI2SInAttribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
        m2ndI2SInAttribute.mI2S_FMT = AudioDigtalI2S::I2S;
        m2ndI2SInAttribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
        mAudioDigitalInstance->Set2ndI2SIn(&m2ndI2SInAttribute); // 72,82 use Set2ndI2SIn

        // Enable 2nd I2S IN
        mAudioDigitalInstance->Set2ndI2SInEnable(true); // 72,82 use Set2ndI2SInEnable
    }
    else
    {
        // Disable 2nd I2S IN
        mAudioDigitalInstance->Set2ndI2SInEnable(false); // 72,82 use Set2ndI2SInEnable
    }

    ALOGD("-%s(), enable = %d", __FUNCTION__, enable);
    return NO_ERROR;
}

status_t AudioMATVResourceManager::SetMatvDirectConnection(const bool enable)
{
    ALOGD("+%s(), enable = %d", __FUNCTION__, enable);

    if (enable == true)
    {
        // Set InterConnection
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I00, AudioDigitalType::O03);
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I01, AudioDigitalType::O04);

        // Set DAC I2S Out
        if (AudioMTKStreamManager::getInstance()->IsOutPutStreamActive() == false)
        {
            mAudioDigitalInstance->SetI2SDacOutAttribute(GetMatvDownlinkSamplingRate());
            mAudioDigitalInstance->SetI2SDacEnable(true);
        }
    }
    else
    {
        // Disable InterConnection
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I00, AudioDigitalType::O03);
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I01, AudioDigitalType::O04);

        // Set DAC I2S Out
        if (AudioMTKStreamManager::getInstance()->IsOutPutStreamActive() == false)
        {
            mAudioDigitalInstance->SetI2SDacEnable(false);
        }
    }

    ALOGD("-%s(), enable = %d", __FUNCTION__, enable);
    return NO_ERROR;
}

} // end of namespace android
