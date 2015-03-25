#include "AudioFMResourceManager.h"

#include "AudioMTKStreamManager.h"
#include "AudioMTKVolumeController.h"

#include "WCNChipController.h"

#define LOG_TAG "AudioFMResourceManager"

namespace android
{

/*==============================================================================
 *                     Const Value
 *============================================================================*/

static const uint16_t kFmUplinkSamplingRateHz   = 44100;
static const uint16_t kFmDownlinkSamplingRateHz = 44100;

static const uint32_t kHWGainMap[] =
{
    0x00000, //   0, -64.0 dB (mute)
    0x0015E, //   1, -63.5 dB
    0x00173, //   2, -63.0 dB
    0x00189, //   3, -62.5 dB
    0x001A0, //   4, -62.0 dB
    0x001B9, //   5, -61.5 dB
    0x001D3, //   6, -61.0 dB
    0x001EE, //   7, -60.5 dB
    0x0020C, //   8, -60.0 dB
    0x0022B, //   9, -59.5 dB
    0x0024C, //  10, -59.0 dB
    0x0026F, //  11, -58.5 dB
    0x00294, //  12, -58.0 dB
    0x002BB, //  13, -57.5 dB
    0x002E4, //  14, -57.0 dB
    0x00310, //  15, -56.5 dB
    0x0033E, //  16, -56.0 dB
    0x00370, //  17, -55.5 dB
    0x003A4, //  18, -55.0 dB
    0x003DB, //  19, -54.5 dB
    0x00416, //  20, -54.0 dB
    0x00454, //  21, -53.5 dB
    0x00495, //  22, -53.0 dB
    0x004DB, //  23, -52.5 dB
    0x00524, //  24, -52.0 dB
    0x00572, //  25, -51.5 dB
    0x005C5, //  26, -51.0 dB
    0x0061D, //  27, -50.5 dB
    0x00679, //  28, -50.0 dB
    0x006DC, //  29, -49.5 dB
    0x00744, //  30, -49.0 dB
    0x007B2, //  31, -48.5 dB
    0x00827, //  32, -48.0 dB
    0x008A2, //  33, -47.5 dB
    0x00925, //  34, -47.0 dB
    0x009B0, //  35, -46.5 dB
    0x00A43, //  36, -46.0 dB
    0x00ADF, //  37, -45.5 dB
    0x00B84, //  38, -45.0 dB
    0x00C32, //  39, -44.5 dB
    0x00CEC, //  40, -44.0 dB
    0x00DB0, //  41, -43.5 dB
    0x00E7F, //  42, -43.0 dB
    0x00F5B, //  43, -42.5 dB
    0x01044, //  44, -42.0 dB
    0x0113B, //  45, -41.5 dB
    0x01240, //  46, -41.0 dB
    0x01355, //  47, -40.5 dB
    0x0147A, //  48, -40.0 dB
    0x015B1, //  49, -39.5 dB
    0x016FA, //  50, -39.0 dB
    0x01857, //  51, -38.5 dB
    0x019C8, //  52, -38.0 dB
    0x01B4F, //  53, -37.5 dB
    0x01CED, //  54, -37.0 dB
    0x01EA4, //  55, -36.5 dB
    0x02075, //  56, -36.0 dB
    0x02261, //  57, -35.5 dB
    0x0246B, //  58, -35.0 dB
    0x02693, //  59, -34.5 dB
    0x028DC, //  60, -34.0 dB
    0x02B48, //  61, -33.5 dB
    0x02DD9, //  62, -33.0 dB
    0x03090, //  63, -32.5 dB
    0x03371, //  64, -32.0 dB
    0x0367D, //  65, -31.5 dB
    0x039B8, //  66, -31.0 dB
    0x03D24, //  67, -30.5 dB
    0x040C3, //  68, -30.0 dB
    0x04499, //  69, -29.5 dB
    0x048AA, //  70, -29.0 dB
    0x04CF8, //  71, -28.5 dB
    0x05188, //  72, -28.0 dB
    0x0565D, //  73, -27.5 dB
    0x05B7B, //  74, -27.0 dB
    0x060E6, //  75, -26.5 dB
    0x066A4, //  76, -26.0 dB
    0x06CB9, //  77, -25.5 dB
    0x0732A, //  78, -25.0 dB
    0x079FD, //  79, -24.5 dB
    0x08138, //  80, -24.0 dB
    0x088E0, //  81, -23.5 dB
    0x090FC, //  82, -23.0 dB
    0x09994, //  83, -22.5 dB
    0x0A2AD, //  84, -22.0 dB
    0x0AC51, //  85, -21.5 dB
    0x0B687, //  86, -21.0 dB
    0x0C157, //  87, -20.5 dB
    0x0CCCC, //  88, -20.0 dB
    0x0D8EF, //  89, -19.5 dB
    0x0E5CA, //  90, -19.0 dB
    0x0F367, //  91, -18.5 dB
    0x101D3, //  92, -18.0 dB
    0x1111A, //  93, -17.5 dB
    0x12149, //  94, -17.0 dB
    0x1326D, //  95, -16.5 dB
    0x14496, //  96, -16.0 dB
    0x157D1, //  97, -15.5 dB
    0x16C31, //  98, -15.0 dB
    0x181C5, //  99, -14.5 dB
    0x198A1, // 100, -14.0 dB
    0x1B0D7, // 101, -13.5 dB
    0x1CA7D, // 102, -13.0 dB
    0x1E5A8, // 103, -12.5 dB
    0x2026F, // 104, -12.0 dB
    0x220EA, // 105, -11.5 dB
    0x24134, // 106, -11.0 dB
    0x26368, // 107, -10.5 dB
    0x287A2, // 108, -10.0 dB
    0x2AE02, // 109,  -9.5 dB
    0x2D6A8, // 110,  -9.0 dB
    0x301B7, // 111,  -8.5 dB
    0x32F52, // 112,  -8.0 dB
    0x35FA2, // 113,  -7.5 dB
    0x392CE, // 114,  -7.0 dB
    0x3C903, // 115,  -6.5 dB
    0x4026E, // 116,  -6.0 dB
    0x43F40, // 117,  -5.5 dB
    0x47FAC, // 118,  -5.0 dB
    0x4C3EA, // 119,  -4.5 dB
    0x50C33, // 120,  -4.0 dB
    0x558C4, // 121,  -3.5 dB
    0x5A9DF, // 122,  -3.0 dB
    0x5FFC8, // 123,  -2.5 dB
    0x65AC8, // 124,  -2.0 dB
    0x6BB2D, // 125,  -1.5 dB
    0x72148, // 126,  -1.0 dB
    0x78D6F, // 127,  -0.5 dB
    0x80000, // 128,   0.0 dB
};


/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioFMResourceManager::AudioFMResourceManager()
{
    ALOGD("%s()", __FUNCTION__);

    mWCNChipController = WCNChipController::GetInstance();
}

AudioFMResourceManager::~AudioFMResourceManager()
{
    ALOGD("%s()", __FUNCTION__);
}


/*==============================================================================
 *                     Audio HW Control
 *============================================================================*/

uint32_t AudioFMResourceManager::GetFmUplinkSamplingRate() const
{
    ALOGD("%s(), sampling rate = %d", __FUNCTION__, kFmUplinkSamplingRateHz);
    return kFmUplinkSamplingRateHz;
}

uint32_t AudioFMResourceManager::GetFmDownlinkSamplingRate() const
{
    ALOGD("%s(), sampling rate = %d", __FUNCTION__, kFmDownlinkSamplingRateHz);
    return kFmDownlinkSamplingRateHz;
}

status_t AudioFMResourceManager::SetFmVolume(const float fm_volume)
{
    ALOGD("+%s(), fm_volume = %f", __FUNCTION__, fm_volume);

    // Calculate HW Gain Value
    uint32_t volume_index = AudioMTKVolumeController::logToLinear(fm_volume); // 0 ~ 256
    uint32_t hw_gain = kHWGainMap[volume_index >> 1]; // 0 ~ 0x80000

    // Set HW Gain
    mAudioDigitalInstance->SetHwDigitalGain(hw_gain, AudioDigitalType::HW_DIGITAL_GAIN2);

    ALOGD("+%s(), fm_volume = %f", __FUNCTION__, fm_volume);
    return NO_ERROR;
}


status_t AudioFMResourceManager::SetFmSourceModuleEnable(const bool enable)
{
    ALOGD("+%s(), enable = %d", __FUNCTION__, enable);

    // Not support Merge Interface
    ASSERT(mWCNChipController->IsFMMergeInterfaceSupported() == false);

    if (enable == true) // Enable 2nd I2S IN
    {
        // Config I2S In PAD Sel
        const AudioDigtalI2S::I2S_IN_PAD_SEL audio_i2s_pad_sel = (mWCNChipController->IsFmChipPadSelConnSys() ==  true)
                                                                 ? AudioDigtalI2S::I2S_IN_FROM_CONNSYS
                                                                 : AudioDigtalI2S::I2S_IN_FROM_IO_MUX;

        // Config I2S Clock Source
        const AudioDigtalI2S::I2S_SRC audio_i2s_clock_source = (mWCNChipController->IsFmChipUseSlaveMode() == true)
                                                               ? AudioDigtalI2S::MASTER_MODE // fm slave,  audio master
                                                               : AudioDigtalI2S::SLAVE_MODE; // fm master, audio slave

        // Config HW ASRC
        bool b_need_src;
        const uint32_t fm_chip_i2s_sample_rate = mWCNChipController->GetFmChipSamplingRate();
        const uint32_t audio_i2s_sample_rate = GetFmUplinkSamplingRate();
        if (fm_chip_i2s_sample_rate != audio_i2s_sample_rate)
        {
            ALOGD("%s(), need SRC: %d => %d", __FUNCTION__, fm_chip_i2s_sample_rate, audio_i2s_sample_rate);
            b_need_src = true;
        }
        else
        {
            ALOGD("%s(), don't need to SRC", __FUNCTION__);
            b_need_src = false;
        }

        // Config 2nd I2S IN
        AudioDigtalI2S m2ndI2SInAttribute;
        memset((void *)&m2ndI2SInAttribute, 0, sizeof(m2ndI2SInAttribute));

        m2ndI2SInAttribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
        m2ndI2SInAttribute.mI2S_IN_PAD_SEL = audio_i2s_pad_sel; // 72,82 only
        m2ndI2SInAttribute.mI2S_SLAVE = audio_i2s_clock_source;
        m2ndI2SInAttribute.mI2S_SAMPLERATE = fm_chip_i2s_sample_rate;
        m2ndI2SInAttribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
        m2ndI2SInAttribute.mI2S_FMT = AudioDigtalI2S::I2S;
        m2ndI2SInAttribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
        mAudioDigitalInstance->Set2ndI2SIn(&m2ndI2SInAttribute); // 72,82 use Set2ndI2SIn

        // Enable ASRC
        if (b_need_src == true)
        {
            ASSERT(audio_i2s_clock_source == AudioDigtalI2S::SLAVE_MODE); // ASRC only work on slave mode

            mAudioDigitalInstance->SetI2SASRCConfig(true, audio_i2s_sample_rate);  // Covert from 32000 Hz to 44100 Hz
            mAudioDigitalInstance->SetI2SASRCEnable(true);
        }

        // Enable 2nd I2S IN
        mAudioDigitalInstance->Set2ndI2SInEnable(true);
    }
    else // Disable 2nd I2S IN
    {
        // Disable ASRC first and than 2nd I2S IN later
        mAudioDigitalInstance->SetI2SASRCEnable(false);
        mAudioDigitalInstance->SetI2SASRCConfig(false, 0); // Setting to bypass ASRC
        mAudioDigitalInstance->Set2ndI2SInEnable(false);
    }

    ALOGD("-%s(), enable = %d", __FUNCTION__, enable);
    return NO_ERROR;
}


status_t AudioFMResourceManager::SetFmDirectConnection(const bool enable)
{
    ALOGD("+%s(), enable = %d", __FUNCTION__, enable);

    if (enable == true)
    {
        // Set InterConnection
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I00, AudioDigitalType::O15);
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I01, AudioDigitalType::O16);

        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I12, AudioDigitalType::O03);
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I13, AudioDigitalType::O04);

        // Set HW_GAIN2
        const AudioMEMIFAttribute::SAMPLINGRATE mem_sample_rate = mAudioDigitalInstance->SampleRateTransform(GetFmUplinkSamplingRate());
        mAudioDigitalInstance->SetHwDigitalGainMode(AudioDigitalType::HW_DIGITAL_GAIN2, mem_sample_rate, 0x40);
        SetFmVolume(0); // init mute
        mAudioDigitalInstance->SetHwDigitalGainEnable(AudioDigitalType::HW_DIGITAL_GAIN2, true);

        // Set DAC I2S Out
        if (AudioMTKStreamManager::getInstance()->IsOutPutStreamActive() == false)
        {
            mAudioDigitalInstance->SetI2SDacOutAttribute(GetFmDownlinkSamplingRate());
            mAudioDigitalInstance->SetI2SDacEnable(true);
        }
    }
    else
    {
        // Disable InterConnection
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I00, AudioDigitalType::O15);
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I01, AudioDigitalType::O16);

        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I12, AudioDigitalType::O03);
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I13, AudioDigitalType::O04);

        // Disable HW_GAIN2
        mAudioDigitalInstance->SetHwDigitalGainEnable(AudioDigitalType::HW_DIGITAL_GAIN2, false);

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

