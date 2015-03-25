#include <math.h>
#include "audio_custom_exp.h"
#include <media/AudioSystem.h>
#include "SpeechDriverFactory.h"
#include "AudioMTKVolumeController.h"
#include "SpeechEnhancementController.h"

//#if defined(MTK_VIBSPK_SUPPORT)
#include "AudioVIBSPKControl.h"
//#endif

#define LOG_TAG "AudioMTKVolumeController"
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

AudioMTKVolumeController *AudioMTKVolumeController::UniqueVolumeInstance = NULL;
static const char PROPERTY_KEY_SPH_DRC_VER[PROPERTY_KEY_MAX] = "persist.af.sph_drc_ver";

// here can change to match audiosystem
#if 1
// total 64 dB
static const float dBPerStep = 0.25f;
static const float VOLUME_MAPPING_STEP = 256.0f;
#else
static const float dBPerStep = 0.5f;
static const float VOLUME_MAPPING_STEP = 100.0f;
#endif

// shouldn't need to touch these
static const float dBConvert = -dBPerStep * 2.302585093f / 20.0f;
static const float dBConvertInverse = 1.0f / dBConvert;

/* 9~-5dB */
#define  AUDIO_BUFFER_HW_GAIN_STEP (14)
#define  AUDIO_AMP_HW_GAIN_STEP (12)
#define  AUDIO_IV_BUFFER_GAIN_STEP (7)
#define  AUDIO_PREAMP_HW_GAIN_STEP (13)
#define  UL_PGA_GAIN_OFFSET (2)

//#define MATV_AUDIO_LINEIN_SET_CHIP_VOLUME //MATV not set MATV Chip volume in 6589
#ifdef MATV_AUDIO_LINEIN_SET_CHIP_VOLUME
#define  MATV_ITEM_SET_VOLUME (0)
#endif

#ifdef EVDO_DT_SUPPORT
#define  VOICE_EVDO_HP_HS_EP 8
#define  VOICE_EVDO_SPK 14
#endif
static const int32 HW_Audio_Value[] = { 8 ,  7,   6,   5,   4,   3,   2,   1,  0, -1,  -2,  -3,  -4};
static const int32 HW_EXP_Value[] = { 15, 14, 13, 12, 11, 10, 9, 8 , 7 , 6};
static const int32 HW_Preamp_Value[] = { 8, -2, -8, -14, -20, -26, -32, -38 , -44};

static const uint16_t SideToneTable[] =
{
    32767, 29204, 26027, 23196, 20674, 18426, 16422, 14636, 13044, 11625,  /*1dB per step*/
    10361, 9234,  8230,  7335,  6537,  5826,  5193,  4628,  4125,  3676,
    3276,  2919,  2602,  2319,  2066,  1841,  1641,  1463,  1304,  1162,
    1035,  923,   822,   733,   653,   582,   519,   462,   412,   367,
    327,   291,   260,   231,   206,   183,   163,   145
};

static const uint16_t SwAgc_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] =
{
    19, 18, 17, 16, 15, 14, 13, 12, 11,
    16, 15,14, 13, 12, 11,
    16, 15,14, 13, 12, 11,
    16, 15,14, 13, 12, 11,
    16, 15,14, 13, 12, 11,
    16, 15,14, 13, 12, 11,
    10,9, 8, 7, 6, 5, 4
};

static const uint16_t PGA_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    6, 6, 6, 6, 6, 6,
    12, 12, 12, 12, 12, 12,
    18, 18, 18, 18, 18, 18,
    24, 24, 24, 24, 24, 24,
    30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30
};

static const uint16_t Dmic_SwAgc_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] =
{
    30, 29, 28, 27, 26, 25, 24, 23, 22,
    21, 20, 19, 18, 17, 16,
    15, 14, 13, 12, 11, 10,
    9, 8, 7, 6, 5, 4,
    3, 2, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0
};

static const uint16_t Dmic_PGA_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    6, 6, 6, 6, 6, 6,
    12, 12, 12, 12, 12, 12,
    18, 18, 18, 18, 18, 18,
    24, 24, 24, 24, 24, 24,
    30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30
};


//DRC1.0 mapping
static const uint16_t DLPGA_Gain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] =
{
    0, 0, 2, 2, 4, 4, 6, 6, 8, 8,
    10, 10, 12, 12, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14
};

static const uint16_t DlEnh1_Gain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint16_t DlDigital_Gain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] =
{
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26
};




//DRC2.0 mapping
static const uint16_t DLPGA_Gain_Map_Ver2[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] =
{
    0, 0, 2, 2, 4, 4, 6, 6, 8, 8,
    8, 10, 10, 10, 12, 12, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14
};

static const uint16_t DlEnh1_Gain_Map_Ver2[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 4, 4, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7 , 7
};

static const uint16_t DlDigital_Gain_Map_Ver2[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] =
{
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 1, 0, 1, 0, 1, 2, 3,
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
};


/* input:  DL_PGA_Gain          (dB/step)         */
/*         MMI_Sidetone_Volume  (dB/8 steps)        */
/*         SW_AGC_Ul_Gain       (dB/step)         */
/* output: DSP_ST_GAIN          (dB/step)          */

uint16_t AudioMTKVolumeController::UpdateSidetone(int DL_PGA_Gain, int  Sidetone_Volume, uint8_t SW_AGC_Ul_Gain)
{
    uint16_t sidetone_vol = 0;
    int vol = 0;
    uint16_t DSP_ST_GAIN = 0;

    ALOGD("UpdateSidetone DL_PGA_Gain = %d MMI_Sidetone_Volume = %d SW_AGC_Ul_Gain = %d",
          DL_PGA_Gain, Sidetone_Volume, SW_AGC_Ul_Gain);

    vol = Sidetone_Volume + SW_AGC_Ul_Gain; //1dB/step
    vol = DL_PGA_Gain - vol + 67 - UL_PGA_GAIN_OFFSET;
    ALOGD("vol = %d", vol);
    if (vol < 0)
    {
        vol = 0;
    }
    if (vol > 47)
    {
        vol = 47;
    }
    DSP_ST_GAIN = SideToneTable[vol]; //output 1dB/step
    if(Sidetone_Volume == 0)
    {
        DSP_ST_GAIN = 0 ;
    }
    ALOGD("DSP_ST_GAIN = %d",DSP_ST_GAIN);
    return DSP_ST_GAIN;
}

float AudioMTKVolumeController::linearToLog(int volume)
{
    //ALOGD("linearToLog(%d)=%f", volume, v);
    return volume ? exp(float(VOLUME_MAPPING_STEP - volume) * dBConvert) : 0;
}

int AudioMTKVolumeController::logToLinear(float volume)
{
    //ALOGD("logTolinear(%d)=%f", v, volume);
    return volume ? VOLUME_MAPPING_STEP - int(dBConvertInverse * log(volume) + 0.5) : 0;
}

AudioMTKVolumeController *AudioMTKVolumeController::getInstance()
{
    if (UniqueVolumeInstance == 0)
    {
        ALOGD("+UniqueVolumeInstance\n");
        UniqueVolumeInstance = new AudioMTKVolumeController();
        ALOGD("-UniqueVolumeInstance\n");
    }
    return UniqueVolumeInstance;
}

AudioMTKVolumeController::AudioMTKVolumeController()
{
    ALOGD("AudioMTKVolumeController contructor\n");
    mAudioAnalogControl = NULL;
    mAudioDigitalControl = NULL;
    mAudioAnalogControl = AudioAnalogControlFactory::CreateAudioAnalogControl();
    if (!mAudioAnalogControl)
    {
        ALOGD("AudioMTKVolumeController  CreateAudioAnalogControl fail");
    }
    mAudioDigitalControl = AudioDigitalControlFactory::CreateAudioDigitalControl();
    if (!mAudioAnalogControl)
    {
        ALOGD("AudioMTKVolumeController  CreateAudioDigitalControl fail");
    }

    mVoiceVolume = 1.0f;
    mMasterVolume = 1.0f;
    for (size_t i = 0; i < AUDIO_STREAM_MAX; ++i)
    {
        mStreamVolume[i] = 1.0f;
    }

    mMatvVolume = 0xFF;
    mInitDone = false;
    mSwAgcGain = 12;
    mULTotalGain = 184;

    initCheck();
}

status_t AudioMTKVolumeController::initCheck()
{
    ALOGD("AudioMTKVolumeController initCheck\n");
    if (!mInitDone)
    {
        initVolumeController();
        mInitDone = true;
    }
    return NO_ERROR;
}

bool AudioMTKVolumeController::SetVolumeRange(uint32 mode, int32 MaxVolume, int32 MinVolume, int32 VolumeRange)
{
    if (mode >= Num_of_Audio_gain)
    {
        ALOGD("SetVolumeRange mode >= Num_of_Audio_gain");
        return false;
    }
    mVolumeMax[mode] = MaxVolume;
    mVolumeMin[mode] = MinVolume;
    mVolumeRange[mode] = VolumeRange;
//#if defined(MTK_VIBSPK_SUPPORT)
    if(mode == Audio_Speaker)
        AudioVIBSPKControl::getInstance()->setVibSpkGain(MaxVolume, MinVolume, VolumeRange);
//#endif
    ALOGD("SetVolumeRange mode=%d, MaxVolume=%d, MinVolume=%d VolumeRange = %d", mode, MaxVolume, MinVolume, VolumeRange);
    return true;
}

void AudioMTKVolumeController::ApplyMdDlGain(int32 degradeDb)
{
    // set degarde db to mode side, DL part, here use degrade dbg
#ifdef EVDO_DT_SUPPORT  //EVDO use volume level
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
    ALOGD("ApplyMdDlGain degradeDb = %d to modem=%d", degradeDb, modem_index);
    if (modem_index == MODEM_EXTERNAL)
	{
        return; //skip evdo modem
    }
#endif
    ALOGD("ApplyMdDlGain degradeDb = %d", degradeDb);
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetDownlinkGain((-1 * degradeDb) << 2); // degrade db * 4
}

void AudioMTKVolumeController::ApplyMdDlEhn1Gain(int32 Gain)
{
    // set degarde db to mode side, DL part, here use degrade dbg
#ifdef EVDO_DT_SUPPORT  //EVDO use volume level
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
    ALOGD("ApplyMdDlGain Gain = %d to modem=%d", Gain, modem_index);
    if (modem_index == MODEM_EXTERNAL)
	{
        return; //skip evdo modem
    }
#endif
    ALOGD("ApplyMdDlEhn1Gain degradeDb = %d", Gain);
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetEnh1DownlinkGain(-1 * (Gain) << 2); // degrade db * 4
}


void AudioMTKVolumeController::ApplyMdUlGain(int32 IncreaseDb)
{
    // set degarde db to mode side, UL part, here use positive gain becasue SW_agc always positive
    ALOGD("ApplyMdUlGain degradeDb = %d", IncreaseDb);
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetUplinkGain(IncreaseDb << 2); // degrade db * 4
}

bool AudioMTKVolumeController::IsHeadsetMicInput(uint32 device)
{
    //check mic with headset or normal mic.
    if (device == Idle_Headset_Record || device == Voice_Rec_Mic_Headset || device == Idle_Video_Record_Headset ||
        device == Headset_Mic  || device == VOIP_Headset_Mic)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//this function map 255 ==> Audiocustomvolume
static float MampVoiceBufferVolume(unsigned char Volume)
{
    if (Volume > VOICE_VOLUME_MAX)
    {
        Volume = VOICE_VOLUME_MAX;
    }
    float DegradedB = VOICE_VOLUME_MAX - Volume;
    DegradedB = DegradedB / AUDIO_TWODB_STEP; // how many 2dB degrade(MT6323 HP and HS are 2 db step)

    ALOGD("Volume = %d MampVoiceVolume DegradedB = %f ", Volume, DegradedB);
    return DegradedB * 2;
}

//this function map 255 ==> Audiocustomvolume
static float MampAudioBufferVolume(unsigned char Volume)
{
    if (Volume > AUDIO_VOLUME_MAX)
    {
        Volume = AUDIO_VOLUME_MAX;
    }
    float DegradedB = AUDIO_VOLUME_MAX - Volume;
    DegradedB = DegradedB / AUDIO_TWODB_STEP; // how many 2dB degrade(MT6323 HP and HS are 2 db step)

    ALOGD("Volume = %d MampAudioVolume DegradedB = %f ", Volume, DegradedB);
    return DegradedB * 2;
}

//this function map 255 ==> Audiocustomvolume
static float MampSPKAMPVolume(unsigned char Volume)
{
    if (Volume > AMP_VOLUME_MAX)
    {
        Volume = AMP_VOLUME_MAX;
    }
    float DegradedB = AMP_VOLUME_MAX - Volume;
    DegradedB = DegradedB / AMP_ONEDB_STEP; // how many dB degrade

    ALOGD("Volume = %d MampSpkAmpVolume DegradedB = %f ", Volume, DegradedB);
    // for volume peroformance ,start with 15
    return (DegradedB);
}

//this function map 255 ==> Audiocustomvolume
static float MampLevelShiftBufferGain(unsigned char Volume)
{
    float DegradedB = 0;
    if (Volume > LEVEL_SHIFT_BUFFER_GAIN_MAX)
    {
        Volume = LEVEL_SHIFT_BUFFER_GAIN_MAX;
    }
    if (Volume != 0)
    {
        DegradedB = LEVEL_SHIFT_BUFFER_GAIN_MAX - Volume;
        DegradedB = DegradedB / LEVEL_SHIFT_THREEDB_STEP; // how many dB DegradedB
    }

    if (DegradedB > LEVEL_SHIFT_BUFFER_MAXDB)
    {
        DegradedB = LEVEL_SHIFT_BUFFER_MAXDB;
    }
    ALOGD("Volume = %d MampLevelShiftBufferGain DegradedB = %f ", Volume, DegradedB);
    return DegradedB * 3;
}

//this function map 255 ==> Audiocustomvolume
static float MampUplinkGain(unsigned char Volume)
{
    if (Volume > UPLINK_GAIN_MAX)
    {
        Volume = UPLINK_GAIN_MAX;
    }
    float DegradedB = UPLINK_GAIN_MAX - Volume;
    DegradedB = DegradedB / UPLINK_ONEDB_STEP; // how many dB degrade

    ALOGD("Volume = %d UPLINK_GAIN_MAX DegradedB = %f ", Volume, DegradedB);
    return DegradedB;
}

//this function map 255 ==> Audiocustomvolume
static float MampLineInPLayGain(unsigned char Volume)
{
    if (Volume > LINEIN_GAIN_MAX)
    {
        Volume = LINEIN_GAIN_MAX;
    }
    float DegradedB = UPLINK_GAIN_MAX - Volume;
    DegradedB = DegradedB / LINEIN_GAIN_2DB_STEP; // how many dB degrade

    ALOGD("Volume = %d MampLineInPLayGain DegradedB = %f ", Volume, DegradedB);
    return DegradedB * 2;
}

uint16_t AudioMTKVolumeController::MappingToDigitalGain(unsigned char Gain)
{
    uint16_t DegradedBGain = (uint16_t)MampUplinkGain(Gain);

    // bounded systen total gain
    if (DegradedBGain > AUDIO_SYSTEM_UL_GAIN_MAX)
    {
        DegradedBGain = AUDIO_SYSTEM_UL_GAIN_MAX;
    }

    return SwAgc_Gain_Map[DegradedBGain];
}

uint16_t AudioMTKVolumeController::MappingToPGAGain(unsigned char Gain)
{
    uint16_t DegradedBGain = (uint16_t)MampUplinkGain(Gain);

    // bounded systen total gain
    if (DegradedBGain > AUDIO_SYSTEM_UL_GAIN_MAX)
    {
        DegradedBGain = AUDIO_SYSTEM_UL_GAIN_MAX;
    }
    return PGA_Gain_Map[DegradedBGain];
}

status_t AudioMTKVolumeController::initVolumeController()
{
    ALOGD("AudioMTKVolumeController initVolumeController\n");
    GetDefaultVolumeParameters(&mVolumeParam);
    GetNBSpeechParamFromNVRam(&mSphParamNB);
    GetWBSpeechParamFromNVRam(&mSphParamWB);
    for (int i = 0 ; i < NORMAL_VOLUME_TYPE_MAX ; i++)
    {
        ALOGD("normalaudiovolume %d = %d", i, mVolumeParam.normalaudiovolume[i]);
    }
    for (int i = 0 ; i < HEADSET_VOLUME_TYPE_MAX ; i++)
    {
        ALOGD("headsetaudiovolume %d = %d", i, mVolumeParam.headsetaudiovolume[i]);
    }
    for (int i = 0 ; i < SPEAKER_VOLUME_TYPE_MAX ; i++)
    {
        ALOGD("speakeraudiovolume %d = %d", i, mVolumeParam.speakeraudiovolume[i]);
    }
    for (int i = 0 ; i < HEADSET_SPEAKER_VOLUME_TYPE_MAX ; i++)
    {
        ALOGD("headsetspeakeraudiovolume %d = %d", i, mVolumeParam.headsetspeakeraudiovolume[i]);
    }
    for (int i = 0 ; i < VER1_NUM_OF_VOL_TYPE ; i++)
    {
        ALOGD("audiovolume_level %d = %d", i, mVolumeParam.audiovolume_level[i]);
    }

    // for normal platyback , let audio drvier can achevie maximun volume , and let computecustomvolume to
    // set mastervolume

    float MaxdB = 0.0, MindB = 0.0, micgain = 0.0;
    int degradegain = 0;
    degradegain = (unsigned char)MampVoiceBufferVolume(mVolumeParam.normalaudiovolume[NORMAL_AUDIO_BUFFER]);
    SetVolumeRange(Audio_Earpiece, DEVICE_VOICE_MAX_VOLUME  , DEVICE_VOICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.headsetaudiovolume[HEADSET_AUDIO_BUFFER]);
    SetVolumeRange(Audio_Headset, DEVICE_MAX_VOLUME  , DEVICE_MIN_VOLUME, degradegain);
    SetVolumeRange(Audio_Headphone, DEVICE_MAX_VOLUME  , DEVICE_MIN_VOLUME, degradegain);
#ifdef USING_EXTAMP_HP
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.speakeraudiovolume[SPEAKER_AMP]);
#else
    degradegain = (unsigned char)MampSPKAMPVolume(mVolumeParam.speakeraudiovolume[SPEAKER_AMP]);
#endif
    SetVolumeRange(Audio_Speaker, DEVICE_AMP_MAX_VOLUME  , DEVICE_AMP_MIN_VOLUME, degradegain);

    degradegain = (unsigned char)MampVoiceBufferVolume(mVolumeParam.normalaudiovolume[NORMAL_AUDIO_BUFFER]);
    SetVolumeRange(Audio_DualMode_Earpiece, DEVICE_VOICE_MAX_VOLUME  , DEVICE_VOICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.headsetspeakeraudiovolume[HEADSET_SPEAKER_AUDIO_BUFFER]);
    SetVolumeRange(Audio_DualMode_Headset, DEVICE_MAX_VOLUME , DEVICE_MIN_VOLUME, degradegain);
    SetVolumeRange(Audio_DualMode_Headphone, DEVICE_MAX_VOLUME , DEVICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampSPKAMPVolume(mVolumeParam.headsetspeakeraudiovolume[HEADSET_SPEAKER_AMP]);
    SetVolumeRange(Audio_DualMode_speaker, DEVICE_AMP_MAX_VOLUME  , DEVICE_AMP_MIN_VOLUME, degradegain);

    degradegain = (unsigned char)MampVoiceBufferVolume(mVolumeParam.normalaudiovolume[NORMAL_AUDIO_BUFFER]);
    SetVolumeRange(Ringtone_Earpiece, DEVICE_VOICE_MAX_VOLUME  , DEVICE_VOICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.headsetaudiovolume[HEADSET_AUDIO_BUFFER]);
    SetVolumeRange(Ringtone_Headset, DEVICE_MAX_VOLUME  , DEVICE_MIN_VOLUME, degradegain);
    SetVolumeRange(Ringtone_Headphone, DEVICE_MAX_VOLUME  , DEVICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampSPKAMPVolume(mVolumeParam.speakeraudiovolume[HEADSET_SPEAKER_AMP]);
    SetVolumeRange(Ringtone_Speaker, DEVICE_AMP_MAX_VOLUME  , DEVICE_AMP_MIN_VOLUME, degradegain);

    degradegain = (unsigned char)MampVoiceBufferVolume(mVolumeParam.normalaudiovolume[NORMAL_AUDIO_BUFFER]);
    SetVolumeRange(Sipcall_Earpiece, DEVICE_VOICE_MAX_VOLUME  , DEVICE_VOICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.headsetaudiovolume[HEADSET_AUDIO_BUFFER]);
    SetVolumeRange(Sipcall_Headset, DEVICE_MAX_VOLUME  , DEVICE_MIN_VOLUME, degradegain);
    SetVolumeRange(Sipcall_Headphone, DEVICE_MAX_VOLUME  , DEVICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampSPKAMPVolume(mVolumeParam.speakeraudiovolume[HEADSET_SPEAKER_AMP]);
    SetVolumeRange(Sipcall_Speaker, DEVICE_AMP_MAX_VOLUME , DEVICE_AMP_MIN_VOLUME, degradegain);

    //-----MIC VOLUME SETTING
    ALOGD(" not define MTK_AUDIO_GAIN_TABLE_SUPPORT");
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][4]);
    SetULTotalGain(Idle_Normal_Record, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][4]);
    SetMicGain(Idle_Normal_Record,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][4]);
    SetULTotalGain(Idle_Headset_Record, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][4]);
    SetMicGain(Idle_Headset_Record, degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][3]);
    SetULTotalGain(Normal_Mic, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][3]);
    SetMicGain(Normal_Mic,         degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][3]);
    SetULTotalGain(Headset_Mic, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][3]);
    SetMicGain(Headset_Mic,       degradegain);


    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][3]);
    SetULTotalGain(Handfree_Mic, mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][3]);
    SetMicGain(Handfree_Mic,     degradegain);


    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][0]);
    SetULTotalGain(TTY_CTM_Mic, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][0]);
    SetMicGain(TTY_CTM_Mic ,     degradegain);

    // voice reconition usage
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][5]);
    SetULTotalGain(Voice_Rec_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][5]);
    SetMicGain(Voice_Rec_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][5]);
    SetULTotalGain(Voice_Rec_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][5]);
    SetMicGain(Voice_Rec_Mic_Headset,  degradegain);

    // add by chiepeng for VOIP mode
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][6]);
    SetULTotalGain(VOIP_Normal_Mic, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][6]);
    SetMicGain(VOIP_Normal_Mic,     degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][6]);
    SetULTotalGain(VOIP_Headset_Mic, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][6]);
    SetMicGain(VOIP_Headset_Mic,   degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][6]);
    SetULTotalGain(VOIP_Handfree_Mic, mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][6]);
    SetMicGain(VOIP_Handfree_Mic, degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][2]);
    SetULTotalGain(Idle_Video_Record_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][2]);
    SetMicGain(Idle_Video_Record_Handset,   degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][2]);
    SetULTotalGain(Idle_Video_Record_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][2]);
    SetMicGain(Idle_Video_Record_Headset,  degradegain);

    // voice unlock usage (input source AUDIO_SOURCE_VOICE_UNLOCK)
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][7]);
    SetULTotalGain(Voice_UnLock_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][7]);
    SetMicGain(Voice_UnLock_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][7]);
    SetULTotalGain(Voice_UnLock_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][7]);
    SetMicGain(Voice_UnLock_Mic_Headset,  degradegain);

    //MIC gain for customization input source usage
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][8]);
    SetULTotalGain(Customization1_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][8]);
    SetMicGain(Customization1_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][8]);
    SetULTotalGain(Customization1_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][8]);
    SetMicGain(Customization1_Mic_Headset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][9]);
    SetULTotalGain(Customization2_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][9]);
    SetMicGain(Customization2_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][9]);
    SetULTotalGain(Customization2_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][9]);
    SetMicGain(Customization2_Mic_Headset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][10]);
    SetULTotalGain(Customization3_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][10]);
    SetMicGain(Customization3_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][10]);
    SetULTotalGain(Customization3_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][10]);
    SetMicGain(Customization3_Mic_Headset,  degradegain);


    // level shift buffer, level shift buffer gain is used degrade gain.
    degradegain = (unsigned char)MampLevelShiftBufferGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][1]);
    SetLevelShiftBufferGain(Level_Shift_Buffer_Gain,  degradegain);

    for (int i = 0; i < Num_Mic_Gain ; i++)
    {
        ALOGD("micgain %d = %d", i, mMicGain[i]);
    }

    // here save sidewtone gain to msidetone
    SetSideTone(EarPiece_SideTone_Gain, mVolumeParam.audiovolume_sid[VOLUME_NORMAL_MODE][3]);
    SetSideTone(Headset_SideTone_Gain, mVolumeParam.audiovolume_sid[VOLUME_HEADSET_MODE][3]);
    SetSideTone(LoudSpk_SideTone_Gain, mVolumeParam.audiovolume_sid[VOLUME_SPEAKER_MODE][3]);
    mSpeechDrcType  =  MagiLoudness_TE_mode;

    return NO_ERROR;
}

// cal and set and set analog gain
void AudioMTKVolumeController::ApplyAudioGain(int Gain, uint32 mode, uint32 device)
{
    ALOGD("ApplyAudioGain  Gain = %d mode= %d device = %d", Gain, mode, device);

    int32 HW_Gain = 0;
    int Setting_value = 0;
    if (device >=  Num_of_Audio_gain)
    {
        ALOGW(" Calgain out of boundary mode = %d device = %0x%x", mode, device);
        return;
    }
    int DegradedBGain = mVolumeRange[device];
    DegradedBGain = DegradedBGain + (DEVICE_VOLUME_RANGE - DegradedBGain) * ((VOLUME_MAPPING_STEP - Gain) / VOLUME_MAPPING_STEP);
    ALOGD("ApplyAudioGain  DegradedBGain = %d mVolumeRange[mode] = %d ", DegradedBGain, mVolumeRange[device]);
    if (device  ==  Audio_Earpiece || device == Audio_DualMode_Earpiece || device == Sipcall_Earpiece)
    {
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, DegradedBGain);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, DegradedBGain);
    }
    else if ((device  == Audio_Headset) || (device == Audio_Headphone) || (device == Sipcall_Headset) || (device == Sipcall_Headphone))
    {
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, DegradedBGain);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, DegradedBGain);
    }
    else if ((device  == Audio_DualMode_Headset) || (device == Audio_DualMode_Headphone))
    {
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, DegradedBGain);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, DegradedBGain);
    }
}


// cal and set and set analog gain
void AudioMTKVolumeController::ApplyAmpGain(int Gain, uint32 mode, uint32 device)
{
    ALOGD("ApplyAmpGain  Gain = %d mode= %d device = %d", Gain, mode, device);
    if (device > Num_of_Audio_gain)
    {
        ALOGW(" Calgain out of boundary mode = %d device = %0x%x", mode, device);
    }
    int DegradedBGain = mVolumeRange[device];
    DegradedBGain = DegradedBGain + (DEVICE_VOLUME_RANGE - DegradedBGain) * ((VOLUME_MAPPING_STEP - Gain) / VOLUME_MAPPING_STEP);
    ALOGD("DegradedBGain   = %d", DegradedBGain);
    mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, DegradedBGain);
    mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, DegradedBGain);
}

// cal and set and set analog gain
void AudioMTKVolumeController::ApplyExtAmpHeadPhoneGain(int Gain, uint32 mode, uint32 device)
{
    ALOGD("ApplyExtAmpHeadPhoneGain  Gain = %d mode= %d device = %d", Gain, mode, device);
    if (device > Num_of_Audio_gain)
    {
        ALOGW(" Calgain out of boundary mode = %d device = %0x%x", mode, device);
    }
    int DegradedBGain = mVolumeRange[device];
    DegradedBGain = DegradedBGain + (DEVICE_VOLUME_RANGE - DegradedBGain) * ((VOLUME_MAPPING_STEP - Gain) / VOLUME_MAPPING_STEP);
    ALOGD("DegradedBGain   = %d", DegradedBGain);
    mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, DegradedBGain);
    mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, DegradedBGain);
}

// cal and set and set analog gain
void AudioMTKVolumeController::ApplyDualmodeGain(int Gain, uint32 mode, uint32 devices)
{
    ALOGD("ApplyDualmodeGain gain = %d mode = %d devices = %d", Gain, mode, devices);
    if (devices  == Audio_DualMode_speaker)
    {
        ApplyAmpGain(Gain,  mode, Audio_DualMode_speaker);
    }
    if (devices == Audio_DualMode_Earpiece)
    {
        ApplyAudioGain(Gain,  mode, Audio_DualMode_Earpiece);
    }
    if (devices == Audio_DualMode_Headset)
    {
        ApplyAudioGain(Gain,  mode, Audio_DualMode_Headset);
    }
    if (devices == Audio_DualMode_Headphone)
    {
        ApplyAudioGain(Gain,  mode, Audio_DualMode_Headphone);
    }
}

status_t AudioMTKVolumeController::setMasterVolume(float v, audio_mode_t mode, uint32_t devices)
{
    ALOGD("AudioMTKVolumeController setMasterVolume v = %f mode = %d devices = 0x%x", v, mode, devices);
    int MapVolume = AudioMTKVolumeController::logToLinear(v);
    mMasterVolume = v;
    switch (mode)
    {
        case AUDIO_MODE_NORMAL :   // normal mode
        {
            if (android_audio_legacy::AudioSystem::popCount(devices) == 1)
            {
                switch (devices)
                {
                    case (AUDIO_DEVICE_OUT_EARPIECE):
                    {
                        ApplyAudioGain(MapVolume,  mode, Audio_Earpiece);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_WIRED_HEADSET):
                    {
                        ApplyAudioGain(MapVolume,  mode, Audio_Headset);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_WIRED_HEADPHONE):
                    {
                        ApplyAudioGain(MapVolume,  mode, Audio_Headphone);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_SPEAKER) :
                    {
#ifdef USING_EXTAMP_HP
                        ApplyExtAmpHeadPhoneGain(MapVolume,  mode, Audio_Speaker);
#else
                        ApplyAmpGain(MapVolume,  mode, Audio_Speaker);
#endif
                        break;
                    }
                    default:
                    {
                        ALOGD("setMasterVolume with device = 0x%x", devices);
                        break;
                    }
                }
            }
            // pop device is more than one , should use dual mode.
            else
            {
                ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_Headphone);
                ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_speaker);
            }
            break;
        }
        case AUDIO_MODE_RINGTONE :
        {
            if (android_audio_legacy::AudioSystem::popCount(devices) == 1)
            {
                switch (devices)
                {
                    case (AUDIO_DEVICE_OUT_EARPIECE):
                    {
                        ApplyAudioGain(MapVolume,  mode, Ringtone_Earpiece);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_WIRED_HEADSET):
                    {
                        ApplyAudioGain(MapVolume,  mode, Ringtone_Headset);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_WIRED_HEADPHONE):
                    {
                        ApplyAudioGain(MapVolume,  mode, Ringtone_Headphone);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_SPEAKER) :
                    {
#ifdef USING_EXTAMP_HP
                        ApplyExtAmpHeadPhoneGain(MapVolume,  mode, Audio_Speaker);
#else
                        ApplyAmpGain(MapVolume,  mode, Audio_Speaker);
#endif
                        break;
                    }
                    default:
                    {
                        ALOGD("setMasterVolume with device = 0x%x", devices);
                        break;
                    }
                }
            }
            // pop device is more than one , should use dual mode.
            else
            {
                ALOGD("AudioMTKVolumeController setMasterVolume with dual mode");
                ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_Headphone);
                ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_speaker);
            }
            break;
        }
        case AUDIO_MODE_IN_CALL :
        case AUDIO_MODE_IN_CALL_2 :
        case AUDIO_MODE_IN_CALL_EXTERNAL:
        {
            ALOGW("set mastervolume with in call ~~~~");
            default:
                break;
            }
        case AUDIO_MODE_IN_COMMUNICATION :
        {
            if (android_audio_legacy::AudioSystem::popCount(devices) == 1)
            {
                switch (devices)
                {
                    case (AUDIO_DEVICE_OUT_EARPIECE):
                    {
                        ApplyAudioGain(MapVolume, mode, Sipcall_Earpiece);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_WIRED_HEADSET):
                    {
                        ApplyAudioGain(MapVolume, mode, Sipcall_Headset);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_WIRED_HEADPHONE):
                    {
                        ApplyAudioGain(MapVolume, mode, Sipcall_Headphone);
                        break;
                    }
                    case (AUDIO_DEVICE_OUT_SPEAKER) :
                    {
#ifdef USING_EXTAMP_HP
                        ApplyExtAmpHeadPhoneGain(MapVolume,  mode, Sipcall_Speaker);
#else
                        ApplyAmpGain(MapVolume,  mode, Sipcall_Speaker);
#endif
                        break;
                    }
                    default:
                    {
                        ALOGD("setMasterVolume with device = 0x%x", devices);
                        break;
                    }
                }
            }
            // pop device is more than one , should use dual mode.
            else
            {
                ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_Headphone);
                ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_speaker);
            }
            break;
        }
    }

    return NO_ERROR;
}

float AudioMTKVolumeController::getMasterVolume()
{
    ALOGD("AudioMTKVolumeController getMasterVolume");
    return mMasterVolume;
}

bool AudioMTKVolumeController::ModeSetVoiceVolume(int mode)
{
    return (mode == AUDIO_MODE_IN_CALL ||
            mode == AUDIO_MODE_IN_CALL_2 ||
            mode == AUDIO_MODE_IN_CALL_EXTERNAL);
}

uint32_t AudioMTKVolumeController::GetDRCVersion(uint32 device)
{
    int DrcSpeechModeBits = 0;

    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SPH_DRC_VER, property_value, "0");
    int Sph_Drc_Version = atoi(property_value);
    if (Sph_Drc_Version)
    {
        ALOGD("change mSpeechDrcType to Sph_Drc_Version = %d", Sph_Drc_Version);
    }

    if (device & AUDIO_DEVICE_OUT_EARPIECE)
    {
        DrcSpeechModeBits = 1;
    }
    else if (device & AUDIO_DEVICE_OUT_WIRED_HEADSET ||  device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
    {
        DrcSpeechModeBits = 1 << 1;
    }
    else if (device & AUDIO_DEVICE_OUT_SPEAKER)
    {
        DrcSpeechModeBits = 1 << 2;
    }
    ALOGD("GetDRCVersion DrcSpeechModeBits = %d device= 0x%x mSpeechDrcType = 0x%x Sph_Drc_Version = 0x%x", DrcSpeechModeBits, device, mSpeechDrcType, Sph_Drc_Version);
    if ((mSpeechDrcType & DrcSpeechModeBits) || (Sph_Drc_Version & DrcSpeechModeBits))
    {
        ALOGD("DRC_VERSION_2");
        return DRC_VERSION_2;
    }
    else
    {
        ALOGD("DRC_VERSION_1");
        return DRC_VERSION_1;
    }
}


status_t AudioMTKVolumeController::setVoiceVolume(float v, audio_mode_t mode, uint32_t device)
{
    ALOGD("AudioMTKVolumeController setVoiceVolume v = %f mode = %d routes = %d", v , mode, device);
    mVoiceVolume = v;
    int MapVolume = 0;
    int DRCversion = GetDRCVersion(device);
    int degradeDb = 0;
    int DigitalgradeDb = 0;  // digital gain
    int Enh1degradeDb = 0;  // digital enh1 gain
    int VoiceAnalogRange = 0;

    if (ModeSetVoiceVolume(mode) == false)
    {
        return INVALID_OPERATION;
    }

    // set dec version to modem side.
    SpeechEnhancementController::GetInstance()->SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN, (bool)DRCversion);

    MapVolume = AudioMTKVolumeController::logToLinear(v);
    degradeDb = (DEVICE_VOLUME_STEP - MapVolume) / VOICE_ONEDB_STEP;
    ALOGD("degradeDb = %d MapVolume = %d ", degradeDb, MapVolume);
    if (DRCversion == DRC_VERSION_1)
    {
        VoiceAnalogRange = DLPGA_Gain_Map_Ver1[degradeDb];
        DigitalgradeDb = DlDigital_Gain_Map_Ver1[degradeDb];
        Enh1degradeDb = DlEnh1_Gain_Map_Ver1[degradeDb];
    }
    else
    {
        VoiceAnalogRange = DLPGA_Gain_Map_Ver2[degradeDb];
        DigitalgradeDb = DlDigital_Gain_Map_Ver2[degradeDb];
        Enh1degradeDb = DlEnh1_Gain_Map_Ver2[degradeDb];
    }
#ifdef EVDO_DT_SUPPORT  //EVDO use volume level
	modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
	if (modem_index == MODEM_EXTERNAL)
	{
		ALOGD("SpeechDriver ModemType=EVDO");		 
		if (device & (AUDIO_DEVICE_OUT_EARPIECE | AUDIO_DEVICE_OUT_WIRED_HEADSET | AUDIO_DEVICE_OUT_WIRED_HEADPHONE) )
		{
			VoiceAnalogRange = VOICE_EVDO_HP_HS_EP;	 //setting volume to 1db
		}
		else if (device & AUDIO_DEVICE_OUT_SPEAKER)
		{
			VoiceAnalogRange = VOICE_EVDO_SPK;	 //setting volume to -5db
		}
		else
		{
			VoiceAnalogRange = VOICE_EVDO_HP_HS_EP;	 //setting volume to maximum
		}		 
		ALOGD("VoiceAnalogRange = %d MapVolume = %d (after adjust for evdo)", VoiceAnalogRange, MapVolume);
		Enh1degradeDb = 0;
		DigitalgradeDb = 0;	
	}
#endif
    ALOGD("DigitalgradeDb = %d Enh1degradeDb = %d VoiceAnalogRange = %d ", DigitalgradeDb, Enh1degradeDb, VoiceAnalogRange);

    if (device & AUDIO_DEVICE_OUT_EARPIECE)
    {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
        {
            mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, VoiceAnalogRange);
            mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, VoiceAnalogRange);
            ApplyMdDlGain(DigitalgradeDb);
            ApplyMdDlEhn1Gain(Enh1degradeDb);
        }
        else
        {
            mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, VoiceAnalogRange);
            mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, VoiceAnalogRange);
            ApplyMdDlGain(DigitalgradeDb);
            ApplyMdDlEhn1Gain(Enh1degradeDb);
        }
        ApplyMicGain(Normal_Mic, mode); // set incall mic gain
    }
    if (device & AUDIO_DEVICE_OUT_WIRED_HEADSET ||  device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
    {
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, VoiceAnalogRange);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, VoiceAnalogRange);
        ApplyMdDlGain(DigitalgradeDb);
        ApplyMdDlEhn1Gain(Enh1degradeDb);
        ApplyMicGain(Headset_Mic, mode); // set incall mic gain
    }
    if (device & AUDIO_DEVICE_OUT_SPEAKER)
    {
#ifdef USING_EXTAMP_HP
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, VoiceAnalogRange);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, VoiceAnalogRange);
        ApplyMdDlGain(DigitalgradeDb);
        ApplyMdDlEhn1Gain(Enh1degradeDb);
#else

        degradeDb += 3; //At the same EM, voice buffer gain for speaker should be lower 3 dB than for HS/HP
#ifdef EVDO_DT_SUPPORT  //EVDO use volume level
		if (modem_index != MODEM_EXTERNAL)
		{	
#endif
	        if (degradeDb > (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP))
	        {
	            degradeDb  = (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP);
	        }
	        if (DRCversion == DRC_VERSION_1)
	        {
	            VoiceAnalogRange = DLPGA_Gain_Map_Ver1[degradeDb];
	            DigitalgradeDb =  DlDigital_Gain_Map_Ver1[degradeDb];
	            Enh1degradeDb = DlEnh1_Gain_Map_Ver1[degradeDb];
	        }
	        else
	        {
		        VoiceAnalogRange = DLPGA_Gain_Map_Ver2[degradeDb];
		        DigitalgradeDb = DlDigital_Gain_Map_Ver2[degradeDb];
		        Enh1degradeDb = DlEnh1_Gain_Map_Ver2[degradeDb];
	        }
#ifdef EVDO_DT_SUPPORT
		}
#endif
        ALOGD("degradeDb = %d MapVolume = %d ", degradeDb, MapVolume);
        ALOGD("DigitalgradeDb = %d Enh1degradeDb = %d VoiceAnalogRange = %d ", DigitalgradeDb, Enh1degradeDb, VoiceAnalogRange);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, 3);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, 3);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, VoiceAnalogRange);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, VoiceAnalogRange);
        ApplyMdDlGain(DigitalgradeDb);
        ApplyMdDlEhn1Gain(Enh1degradeDb);

#endif
        ApplyMicGain(Handfree_Mic, mode); // set incall mic gain
    }

    if ((device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO) || (device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET) || (device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET))
    {
        //when use BT_SCO , apply digital to 0db.
        ApplyMdDlGain(0);
        ApplyMdDlEhn1Gain(0);
        ApplyMdUlGain(0);
    }
    ApplySideTone(GetSideToneGainType(device));
    return NO_ERROR;
}

float AudioMTKVolumeController::getVoiceVolume(void)
{
    ALOGD("AudioMTKVolumeController getVoiceVolume");
    return mVoiceVolume;
}

status_t AudioMTKVolumeController::setStreamVolume(int stream, float v)
{
    ALOGD("AudioMTKVolumeController setStreamVolume stream = %d", stream);
    return NO_ERROR;
}

status_t AudioMTKVolumeController::setStreamMute(int stream, bool mute)
{
    ALOGD("AudioMTKVolumeController setStreamMute stream = %d mute = %d", stream, mute);
    return NO_ERROR;
}

float AudioMTKVolumeController::getStreamVolume(int stream)
{
    ALOGD("AudioMTKVolumeController getStreamVolume stream = %d", stream);
    return mStreamVolume[stream];
}


void AudioMTKVolumeController::GetMatvService()
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder;
    do
    {
        binder = sm->getService(String16("media.ATVCtrlService"));
        if (binder != 0)
        {
            break;
        }
        ALOGW("ATVCtrlService not published, waiting...");
        usleep(1000 * 1000); // 1 s
    }
    while (true);
    spATVCtrlService = interface_cast<IATVCtrlService>(binder);
}


bool AudioMTKVolumeController::SetMatvMute(bool b_mute)
{
    ALOGD("SetMatvMute(%d), mMatvVolume(%d)", b_mute, mMatvVolume);
#ifdef MATV_AUDIO_LINEIN_PATH
#ifdef MATV_AUDIO_LINEIN_SET_CHIP_VOLUME
    if (spATVCtrlService == NULL)
    {
        ALOGW("SetMatvMute, can't get spATVCtrlService");
        GetMatvService();
        if (spATVCtrlService == NULL)
        {
            ALOGE("SetMatvMute cannot get matv service");
            return false;
        }
    }

    if (b_mute == true)
    {
        ALOGD("SetMatvMute ATVCS_matv_adjust MATV_ITEM_SET_VOLUME(%d), volume(%d)", MATV_ITEM_SET_VOLUME, 0);
        mMatvMute = true;
        spATVCtrlService->ATVCS_matv_adjust(MATV_ITEM_SET_VOLUME, 0);
    }
    else
    {
        mMatvMute = false;
        ALOGD("SetMatvMute ATVCS_matv_adjust MATV_ITEM_SET_VOLUME(%d), volume(%d)", MATV_ITEM_SET_VOLUME, mMatvVolume);
        spATVCtrlService->ATVCS_matv_adjust(MATV_ITEM_SET_VOLUME, mMatvVolume);
    }
#endif
#endif

    return true;
}

bool AudioMTKVolumeController::setMatvVolume(int volume)
{
    ALOGD("setMatvVolume volume=%d", volume);
#ifdef MATV_AUDIO_LINEIN_PATH
#ifdef MATV_AUDIO_LINEIN_SET_CHIP_VOLUME
    if (spATVCtrlService == NULL)
    {
        ALOGW("setMatvVolume but spATVCtrlService == NULL");
        GetMatvService();
        if (spATVCtrlService == NULL)
        {
            ALOGE("setMatvVolume cannot get matv service");
            return false;
        }
    }

    if (mMatvMute == false)
    {
        ALOGD("setMatvVolume ATVCS_matv_adjust MATV_ITEM_SET_VOLUME(%d), volume(%d)", MATV_ITEM_SET_VOLUME, volume);
        spATVCtrlService->ATVCS_matv_adjust(MATV_ITEM_SET_VOLUME, volume);
    }
#endif
#endif

    mMatvVolume = volume;
    return true;
}

int AudioMTKVolumeController::GetMatvVolume(void)
{
    return mMatvVolume;
}

// should depend on different usage , FM ,MATV and output device to setline in gain
status_t AudioMTKVolumeController::SetLineInPlaybackGain(int type)
{
    ALOGD("SetLineInPlaybackGain type stream = %d", type);
    int DegradedBGain =  mMicGain[type];
    if (type == Analog_PLay_Gain)
    {
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_LINEINL, DegradedBGain);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_LINEINR, DegradedBGain);
    }
    return NO_ERROR;
}

status_t AudioMTKVolumeController::SetLineInRecordingGain(int type)
{
    ALOGD("SetLineInRecordingGain type stream = %d", type);
    ApplyLevelShiftBufferGain(Level_Shift_Buffer_Gain);
    return NO_ERROR;
}

status_t AudioMTKVolumeController::SetSideTone(uint32_t Mode, uint32_t Gain)
{
    ALOGD("SetSideTone type Mode = %d devices = %d", Mode, Gain);
    if (Mode >=  Num_Side_Tone_Gain)
    {
        ALOGD("Mode >Num_Side_Tone_Gain");
        return INVALID_OPERATION;
    }
    mSideTone[Mode] = Gain;
    return NO_ERROR;
}

uint32_t AudioMTKVolumeController::GetSideToneGain(uint32_t device)
{
    ALOGD("GetSideToneGain type device = %d ", device);
    uint32 Gain_type = GetSideToneGainType(device);
    if (Gain_type >=  Num_Side_Tone_Gain)
    {
        ALOGD("Mode >Num_Side_Tone_Gain");
        return INVALID_OPERATION;
    }
    ALOGD("GetSideToneGain = %d", mSideTone[Gain_type]);
    return mSideTone[Gain_type];
}

uint32_t AudioMTKVolumeController::GetSideToneGainType(uint32 devices)
{
    if (devices & AUDIO_DEVICE_OUT_EARPIECE)
    {
        return EarPiece_SideTone_Gain;
    }
    else if (devices & AUDIO_DEVICE_OUT_SPEAKER)
    {
        return LoudSpk_SideTone_Gain;
    }
    else if ((devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) || (devices & AUDIO_DEVICE_OUT_WIRED_HEADSET))
    {
        return Headset_SideTone_Gain;
    }
    else
    {
        ALOGW("GetSideToneGainType with devices = 0x%x", devices);
        return LoudSpk_SideTone_Gain;
    }
}

status_t AudioMTKVolumeController::ApplySideTone(uint32_t Mode)
{
    // here apply side tone gain, need base on UL and DL analog gainQuant
    uint16_t DspSideToneGain = 0;
    int SidetoneDb = 0;
    ALOGD("ApplySideTone mode = %d", Mode);
    if (Mode == EarPiece_SideTone_Gain)
    {
        SidetoneDb = mVolumeParam.audiovolume_sid[VOLUME_NORMAL_MODE][3] >> 3;
        DspSideToneGain = UpdateSidetone(mAudioAnalogControl->GetAnalogGain(AudioAnalogType::VOLUME_HSOUTR), SidetoneDb, mSwAgcGain);
    }
    else if (Mode == Headset_SideTone_Gain)
    {
        SidetoneDb = mVolumeParam.audiovolume_sid[VOLUME_HEADSET_MODE][3] >> 3;
        DspSideToneGain = UpdateSidetone(mAudioAnalogControl->GetAnalogGain(AudioAnalogType::VOLUME_HPOUTL), SidetoneDb, mSwAgcGain);
    }
    else if (Mode == LoudSpk_SideTone_Gain)
    {
        SidetoneDb = mVolumeParam.audiovolume_sid[VOLUME_SPEAKER_MODE][3] >> 3;
        //DspSideToneGain = UpdateSidetone(mAudioAnalogControl->GetAnalogGain(AudioAnalogType::VOLUME_HSOUTR), SidetoneDb, mSwAgcGain);
        // mute sidetone gain when speaker mode.
        DspSideToneGain =0;
    }
    ALOGD("ApplySideTone mode = %d DspSideToneGain = %d", Mode, DspSideToneGain);
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetSidetoneGain(DspSideToneGain);

    return NO_ERROR;
}

status_t AudioMTKVolumeController::SetMicGain(uint32_t Mode, uint32_t Gain)
{
    if (Mode >= Num_Mic_Gain)
    {
        ALOGD("SetMicGain error");
        return false;
    }
    ALOGD("SetMicGain MicMode=%d, Gain=%d", Mode, Gain);
    mMicGain[Mode] = Gain;
    return NO_ERROR;
}

status_t AudioMTKVolumeController::SetULTotalGain(uint32_t Mode, unsigned char Volume)
{
    if (Volume > UPLINK_GAIN_MAX)
    {
        Volume = UPLINK_GAIN_MAX;
    }

    ALOGD("SetULTotalGain MicMode=%d, Volume=%d", Mode, Volume);
    mULTotalGainTable[Mode] = Volume;
    return NO_ERROR;
}

bool AudioMTKVolumeController::CheckMicUsageWithMode(uint32_t MicType, int mode)
{
    if ((MicType == Normal_Mic ||
         MicType == Headset_Mic ||
         MicType == Handfree_Mic) &&
        (mode != AUDIO_MODE_IN_CALL &&
         mode != AUDIO_MODE_IN_CALL_2 &&
         mode != AUDIO_MODE_IN_CALL_EXTERNAL))
    {
        return true;
    }
    else
    {
        return false;
    }
}

status_t AudioMTKVolumeController::ApplyMicGain(uint32_t MicType, int mode)
{
    if (MicType >= Num_Mic_Gain)
    {
        ALOGD("SetMicGain error");
        return false;
    }
    mSwAgcGain = SW_AGC_GAIN_MAX;
    mULTotalGain = mULTotalGainTable[MicType];
    // here base on mic  and use degrade gain to set hardware register
    int DegradedBGain = mMicGain[MicType];

    // bounded systen total gain
    if (DegradedBGain > AUDIO_SYSTEM_UL_GAIN_MAX)
    {
        DegradedBGain = AUDIO_SYSTEM_UL_GAIN_MAX;
    }

    if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))
    {
        mSwAgcGain  =  Dmic_SwAgc_Gain_Map[DegradedBGain];
        DegradedBGain  =  Dmic_PGA_Gain_Map[DegradedBGain];
    }
    else
    {
        mSwAgcGain  =  SwAgc_Gain_Map[DegradedBGain];
        DegradedBGain  =  PGA_Gain_Map[DegradedBGain];
    }
#ifdef EVDO_DT_SUPPORT
	if (mode == AUDIO_MODE_IN_CALL_EXTERNAL)
	{
		ALOGD("ApplyMicGain EVDO_DT_SUPPORT DegradedBGain(%d)-=mSwAgcGain(%d)", DegradedBGain, mSwAgcGain);
		DegradedBGain-=mSwAgcGain;
		DegradedBGain+=6;
	}
#endif

    ALOGD("ApplyMicGain MicType = %d DegradedBGain = %d SwAgcGain = %d, mULTotalGain = %d",
          MicType, DegradedBGain, mSwAgcGain, mULTotalGain);
    mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, DegradedBGain);
    mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, DegradedBGain);

    // fix me: here need t send reminder DB to HD record or modem side
    if (mode == AUDIO_MODE_IN_CALL ||
        mode == AUDIO_MODE_IN_CALL_2 ||
        mode == AUDIO_MODE_IN_CALL_EXTERNAL)
    {
        ApplyMdUlGain(mSwAgcGain);
    }
    else
    {
        // fix me ,here to apply HD record AGC gain
        //HD record will get mSwAgcGain actively when needed
        ALOGD("ApplyMicGain mSwAgcGain = %d, mULTotalGain=%d", mSwAgcGain, mULTotalGain);
    }

    return NO_ERROR;
}

bool AudioMTKVolumeController::SetLevelShiftBufferGain(uint32 MicMode, uint32 Gain)
{
    if (MicMode != Level_Shift_Buffer_Gain)
    {
        ALOGD("SetLevelShiftBufferGain error");
        return false;
    }
    ALOGD("SetLevelShiftBufferGain MicMode=%d, Gain=%x", MicMode, Gain);
    mMicGain[Level_Shift_Buffer_Gain] = Gain;
    return true;
}

bool AudioMTKVolumeController::ApplyLevelShiftBufferGain(uint32 MicMode)
{
    uint32 DegradedBGain = 0;
    if (MicMode != Level_Shift_Buffer_Gain)
    {
        ALOGD("SetLevelShiftBufferGain error");
        return false;
    }
    ALOGD("ApplyLevelShiftBufferGain MicMode=%d, mMicGain[Level_Shift_Buffer_Gain]=%x", MicMode, mMicGain[Level_Shift_Buffer_Gain]);
    DegradedBGain = mMicGain[Level_Shift_Buffer_Gain];
    mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_LEVELSHIFTL, DegradedBGain);
    mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_LEVELSHIFTR, DegradedBGain);
    return true;
}

uint32_t AudioMTKVolumeController::MapDigitalHwGain(uint32_t Gain)
{
    uint32_t DegradeDB = 0;
    if (Gain > HW_DIGITAL_GAIN_MAX)
    {
        Gain = HW_DIGITAL_GAIN_MAX;
    }
    DegradeDB = (HW_DIGITAL_GAIN_MAX - Gain) / HW_DIGITAL_GAIN_STEP;
    return DegradeDB;
}

status_t AudioMTKVolumeController::SetDigitalHwGain(uint32_t Mode, uint32_t Gain , uint32_t routes)
{
    uint32_t DegradeDB = MapDigitalHwGain(Gain);
    return NO_ERROR;
}

void AudioMTKVolumeController::GetDefaultVolumeParameters(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *volume_param)
{
    GetVolumeVer1ParamFromNV(volume_param);

    // fix me , here just get default , need to get from nvram
    for (int i = 0 ; i < NORMAL_VOLUME_TYPE_MAX ; i++)
    {
        ALOGD("ad_audio_custom_default normalaudiovolume %d = %d", i, volume_param->normalaudiovolume[i]);
    }
    for (int i = 0 ; i < HEADSET_VOLUME_TYPE_MAX ; i++)
    {
        ALOGD("ad_audio_custom_default headsetaudiovolume %d = %d", i, volume_param->headsetaudiovolume[i]);
    }
    for (int i = 0 ; i < SPEAKER_VOLUME_TYPE_MAX ; i++)
    {
        ALOGD("ad_audio_custom_default speakeraudiovolume %d = %d", i, volume_param->speakeraudiovolume[i]);
    }
    for (int i = 0 ; i < HEADSET_SPEAKER_VOLUME_TYPE_MAX ; i++)
    {
        ALOGD("ad_audio_custom_default headsetspeakeraudiovolume %d = %d", i, volume_param->headsetspeakeraudiovolume[i]);
    }
    //memcpy((void *)volume_param, (void *)&ad_audio_custom_default, sizeof(ADAUDIO_CUSTOM_VOLUME_PARAM_STRUCT));
}

status_t AudioMTKVolumeController::SetMicGainTuning(uint32_t Mode, uint32_t Gain)
{
    ALOGD("SetMicGainTuning Mode = %d, Gain = %d", Mode, Gain);
    int degradegain;
    degradegain = (unsigned char)MampUplinkGain(Gain);
    SetMicGain(Mode,  degradegain);
    return NO_ERROR;
}
}
