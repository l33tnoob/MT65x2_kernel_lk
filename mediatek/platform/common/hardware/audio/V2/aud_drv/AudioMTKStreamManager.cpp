#include <utils/String16.h>
#include "AudioMTKStreamManager.h"
#include "AudioUtility.h"
#include "AudioMTKStreamOut.h"
#include "AudioMTKStreamIn.h"
#include "AudioMTKStreamInManager.h"
#include "AudioCompFltCustParam.h"

#define LOG_TAG "AudioMTKStreamManager"
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

#ifdef USE_SPECIFIC_STREAM_MANAGER
extern "C" AudioMTKStreamManager *createSpecificStreamManager();
#endif

AudioMTKStreamManager *AudioMTKStreamManager::UniqueStreamManagerInstance = NULL;

AudioMTKStreamManager *AudioMTKStreamManager::getInstance()
{
    static Mutex gLock;
    AutoMutex lock(gLock);
    if (UniqueStreamManagerInstance == 0)
    {
        ALOGD("+UniqueAnalogInstance\n");
#ifndef USE_SPECIFIC_STREAM_MANAGER
        UniqueStreamManagerInstance = new AudioMTKStreamManager();
#else
        UniqueStreamManagerInstance = createSpecificStreamManager();
#endif
        ALOGD("-UniqueAnalogInstance\n");
    }
    return UniqueStreamManagerInstance;
}

AudioMTKStreamManager::AudioMTKStreamManager()
{
    ALOGD("AudioMTKStreamManager contructor \n");
    mStreamInNumber = 1;
    mStreamOutNumber = 1;
#ifdef MTK_BESLOUDNESS_SUPPORT
    unsigned int result = 0 ;
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT audioParam;
    if (GetBesLoudnessControlOptionParamFromNV(&audioParam))
    {
        result = audioParam.u32EnableFlg;
    }
    mBesLoudnessStatus = (result ? true : false);
    ALOGD("AudioMTKStreamManager mBesLoudnessStatus [%d] (From NvRam) \n",mBesLoudnessStatus);
#else
    mBesLoudnessStatus = true;
    ALOGD("AudioMTKStreamManager mBesLoudnessStatus [%d] (Always) \n",mBesLoudnessStatus);
#endif
    mBesLoudnessControlCallback = NULL;
}

status_t  AudioMTKStreamManager::initCheck()
{
    ALOGD("AudioMTKStreamManager initCheck \n");
    return NO_ERROR;
}

bool  AudioMTKStreamManager::StreamExist()
{
    return InputStreamExist() || OutputStreamExist();
}

bool  AudioMTKStreamManager::InputStreamExist()
{
    if (mStreamInVector.size())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool  AudioMTKStreamManager::OutputStreamExist()
{
    if (mStreamOutVector.size())
    {
        return true;
    }
    else
    {
        return false;
    }
}

status_t  AudioMTKStreamManager::closeInputStream(android_audio_legacy::AudioStreamIn *in)
{
    AudioMTKStreamIn  *StreamIn = (AudioMTKStreamIn *)in;
    uint32_t  Identity = StreamIn->GetIdentity();
    ALOGD("closeInputStream in = %p Identity = %d", in, Identity);
    ssize_t index = mStreamInVector.indexOfKey(Identity);
    if (in)
    {
        delete mStreamInVector.valueAt(index);
        mStreamInVector.removeItem(Identity);
        ALOGD("index = %d mStreamInVector.size() = %d", index, mStreamInVector.size());
    }
    return NO_ERROR;
}

android_audio_legacy::AudioStreamIn *AudioMTKStreamManager::openInputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    ALOGD("openInputStream, devices = 0x%x format=0x%x ,channels=0x%x, rate=%d acoustics = 0x%x", devices, *format, *channels, *sampleRate, acoustics);

    if (SpeechVMRecorder::GetInstance()->GetVMRecordStatus() == true)
    {
        ALOGW("%s(), The following record data will be muted until VM/EPL is closed.", __FUNCTION__);
    }

    AudioMTKStreamIn *StreamIn = new AudioMTKStreamIn();
    StreamIn->Set(devices, format, channels, sampleRate, status, acoustics);
    if (*status == NO_ERROR)
    {
        mStreamInNumber++;
        StreamIn->SetIdentity(mStreamInNumber);
        mStreamInVector.add(mStreamInNumber, StreamIn);
        ALOGD("openInputStream NO_ERROR mStreamInNumber = %d mStreamInVector.size() = %d" , mStreamInNumber, mStreamInVector.size());
        return StreamIn;
    }
    else
    {
        ALOGD("openInputStream delete streamin");
        delete StreamIn;
        return NULL;
    }
}

android_audio_legacy::AudioStreamOut *AudioMTKStreamManager::openOutputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    uint32_t output_flag)
{
    unsigned int Address = 0;

    // Check the Attribute
    if (*format == 0)
    {
        *format = AUDIO_FORMAT_PCM_16_BIT;
    }
    if (*channels == 0)
    {
        *channels = AUDIO_CHANNEL_OUT_STEREO;
    }
    if (*sampleRate == 0)
    {
        *sampleRate = 44100;
    }

    ALOGD("openOutputStream, devices = 0x%x format=0x%x ,channels=0x%x, rate=%d", devices, *format, *channels, *sampleRate);

    AudioMTKStreamOut *out = new AudioMTKStreamOut(devices, format,  channels,  sampleRate, status);
    Address = (unsigned int)out;  // use pointer address
    mStreamOutVector.add(Address, out);
    return out;
}

status_t AudioMTKStreamManager::closeOutputStream(android_audio_legacy::AudioStreamOut *out)
{
    return NO_ERROR;
}

bool AudioMTKStreamManager::IsOutPutStreamActive()
{
    for (int i = 0; i < mStreamOutVector.size() ; i++)
    {
        AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
        if (pTempOut->GetStreamRunning())
        {
            return true;
        }
    }
    return false;
}

bool AudioMTKStreamManager::IsInPutStreamActive()
{
    for (int i = 0; i < mStreamInVector.size() ; i++)
    {
        AudioMTKStreamIn  *pTempIn = (AudioMTKStreamIn *)mStreamInVector.valueAt(i);
        if (pTempIn != NULL) // fpr input ,exist means active
        {
            return true;
        }
    }
    return false;
}

// set musicplus to streamout
void AudioMTKStreamManager::SetMusicPlusStatus(bool bEnable)
{
    if (mStreamOutVector.size())
    {
        for (size_t i = 0; i < mStreamOutVector.size() ; i++)
        {
            AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
            pTempOut->SetMusicPlusStatus(bEnable ? true : false);
        }
    }
}

bool AudioMTKStreamManager::GetMusicPlusStatus()
{
    if (mStreamOutVector.size())
    {
        for (size_t i = 0; i < mStreamOutVector.size() ; i++)
        {
            AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
            bool musicplus_status = pTempOut->GetMusicPlusStatus();
            if (musicplus_status)
            {
                return true;
            }
        }
    }
    return false;
}

void AudioMTKStreamManager::SetBesLoudnessStatus(bool bEnable)
{
    #ifdef MTK_BESLOUDNESS_SUPPORT
    ALOGD("mBesLoudnessStatus() flag %d", bEnable);
    mBesLoudnessStatus = bEnable;
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT audioParam;
    audioParam.u32EnableFlg = bEnable ? 1 : 0;
    SetBesLoudnessControlOptionParamToNV(&audioParam);
    if (mBesLoudnessControlCallback != NULL)
    {
        mBesLoudnessControlCallback((void *)mBesLoudnessStatus);
    }
    #else
    ALOGD("Unsupport set mBesLoudnessStatus()");
    #endif
}

bool AudioMTKStreamManager::GetBesLoudnessStatus()
{
    return mBesLoudnessStatus;
}

void AudioMTKStreamManager::SetBesLoudnessControlCallback(const BESLOUDNESS_CONTROL_CALLBACK_STRUCT *callback_data)
{
    if (callback_data == NULL)
    {
        mBesLoudnessControlCallback = NULL;
    }
    else
    {
        mBesLoudnessControlCallback = callback_data->callback;
        ASSERT(mBesLoudnessControlCallback != NULL);
        mBesLoudnessControlCallback((void *)mBesLoudnessStatus);
    }
}

void AudioMTKStreamManager::SetHiFiDACStatus(bool bEnable)
{
#ifdef HIFIDAC_SWITCH //HP switch use AudEnh setting
    ALOGD("SetHiFiDACStatus mHiFiDACStatus %d", bEnable);
    mHiFiDACStatus = bEnable;
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT audioParam;
    audioParam.u32EnableFlg = bEnable ? 1 : 0;
    SetHiFiDACControlOptionParamToNV(&audioParam);
#else
    ALOGD("Unsupport SetHiFiDACStatus()");
#endif
}

bool AudioMTKStreamManager::GetHiFiDACStatus()
{
#ifdef HIFIDAC_SWITCH //HP switch use AudEnh setting
    ALOGD("GetHiFiDACStatus(+) ");

    AUDIO_AUDENH_CONTROL_OPTION_STRUCT audioParam;
    unsigned int result = 0 ;
    if (GetHiFiDACControlOptionParamFromNV(&audioParam))
    {
        result = audioParam.u32EnableFlg;
    }
    mHiFiDACStatus = (result ? true : false);
    ALOGD("GetHiFiDACStatus mHiFiDACStatus %d", mHiFiDACStatus);
    return mHiFiDACStatus;
#else
    ALOGD("Unsupport GetHiFiDACStatus()");
    return false;
#endif
}

size_t AudioMTKStreamManager::getInputBufferSize(int32_t sampleRate, int format, int channelCount)
{
    ALOGD("AudioMTKHardware getInputBufferSize, sampleRate=%d, format=%d, channelCount=%d\n", sampleRate, format, channelCount);
#if 1
    size_t bufferSize = 0;
    bufferSize = (sampleRate * channelCount * 20 * sizeof(int16_t)) / 1000;
    ALOGD("getInputBufferSize bufferSize=%d\n", bufferSize);
    return bufferSize;
#else
    if (mStreamInVector.size())
    {
        // get input stream szie
        android_audio_legacy::AudioStreamIn *Input =   mStreamInVector.valueAt(0);
        return Input->bufferSize();
    }
    else
    {
        return AudioMTKStreamIn::GetfixBufferSize(sampleRate, format, channelCount);
    }
#endif
}

status_t AudioMTKStreamManager::UpdateACFHCF(int value)
{
    AUDIO_ACF_CUSTOM_PARAM_STRUCT sACFHCFParam;
    for (int i = 0; i < mStreamOutVector.size() ; i++)
    {
        AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
        if (value == 0)
        {
            ALOGD("setParameters Update ACF Parames");
            GetAudioCompFltCustParamFromNV(AUDIO_COMP_FLT_AUDIO, &sACFHCFParam);
            pTempOut->StreamOutCompFltPreviewParameter(AUDIO_COMP_FLT_AUDIO, (void *)&sACFHCFParam, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

            if ((pTempOut->GetStreamRunning()) && (pTempOut->GetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO)))
            {
                pTempOut->SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO, false);
                pTempOut->SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO, true);
            }
        }
        else if (value == 1)
        {
            ALOGD("setParameters Update HCF Parames");
            GetAudioCompFltCustParamFromNV(AUDIO_COMP_FLT_HEADPHONE, &sACFHCFParam);
            pTempOut->StreamOutCompFltPreviewParameter(AUDIO_COMP_FLT_HEADPHONE, (void *)&sACFHCFParam, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

            if ((pTempOut->GetStreamRunning()) && (pTempOut->GetStreamOutCompFltStatus(AUDIO_COMP_FLT_HEADPHONE)))
            {
                pTempOut->SetStreamOutCompFltStatus(AUDIO_COMP_FLT_HEADPHONE, false);
                pTempOut->SetStreamOutCompFltStatus(AUDIO_COMP_FLT_HEADPHONE, true);
            }
        }
        else if (value == 2)
        {
            ALOGD("setParameters Update ACFSub Parames");
            GetAudioCompFltCustParamFromNV(AUDIO_COMP_FLT_AUDIO_SUB, &sACFHCFParam);
            pTempOut->StreamOutCompFltPreviewParameter(AUDIO_COMP_FLT_AUDIO_SUB, (void *)&sACFHCFParam, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

            if ((pTempOut->GetStreamRunning()) && (pTempOut->GetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO)))
            {
                pTempOut->SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO, false);
                pTempOut->SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO, true);
            }
        }
    }
    return NO_ERROR;
}

// ACF Preview parameter
status_t AudioMTKStreamManager::SetACFPreviewParameter(void *ptr , int len)
{
    ALOGD("AudioMTKHardware SetACFPreviewParameter\n");
    if (mStreamOutVector.size())
    {
        for (size_t i = 0; i < mStreamOutVector.size() ; i++)
        {
            AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
            pTempOut->StreamOutCompFltPreviewParameter(AUDIO_COMP_FLT_AUDIO, ptr, len);
        }
    }
    return NO_ERROR;
}

status_t AudioMTKStreamManager::SetHCFPreviewParameter(void *ptr , int len)
{
    ALOGD("AudioMTKHardware SetHCFPreviewParameter\n");
    if (mStreamOutVector.size())
    {
        for (size_t i = 0; i < mStreamOutVector.size() ; i++)
        {
            AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
            pTempOut->StreamOutCompFltPreviewParameter(AUDIO_COMP_FLT_HEADPHONE, ptr, len);
        }
    }
    return NO_ERROR;
}

//suspend input and output standby
status_t AudioMTKStreamManager::SetOutputStreamSuspend(bool bEnable)
{
    ALOGD("SetOutputStreamSuspend");
    if (mStreamOutVector.size())
    {
        for (size_t i = 0; i < mStreamOutVector.size() ; i++)
        {
            AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
            pTempOut->SetSuspend(bEnable);
        }
    }
    return NO_ERROR;
}

status_t AudioMTKStreamManager::SetInputStreamSuspend(bool bEnable)
{
    ALOGD("SetOutputStreamSuspend");
    if (mStreamInVector.size())
    {
        for (size_t i = 0; i < mStreamInVector.size() ; i++)
        {
            AudioMTKStreamIn  *pTempIn = (AudioMTKStreamIn *)mStreamInVector.valueAt(i);
            pTempIn->SetSuspend(bEnable);
        }
    }
    return NO_ERROR;
}

status_t AudioMTKStreamManager::ForceAllStandby()
{
    // force all stream to standby
    ALOGD("ForceAllStandby");
    if (mStreamInVector.size())
    {
        for (size_t i = 0; i < mStreamInVector.size() ; i++)
        {
            AudioMTKStreamIn  *pTempIn = (AudioMTKStreamIn *)mStreamInVector.valueAt(i);
            pTempIn->standby();
        }
    }
    if (mStreamOutVector.size())
    {
        for (size_t i = 0; i < mStreamOutVector.size() ; i++)
        {
            AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
            pTempOut->setForceStandby(true);
            pTempOut->standby();
            pTempOut->setForceStandby(false);
        }
    }
    return NO_ERROR;
}

void AudioMTKStreamManager::SetInputMute(bool bEnable)
{
    AudioMTKStreamInManager::getInstance()->SetInputMute(bEnable);
}

status_t AudioMTKStreamManager::setParametersToStreamOut(const String8 &keyValuePairs)
{
    if (mStreamOutVector.size())
    {
        for (size_t i = 0; i < mStreamOutVector.size() ; i++)
        {
            AudioMTKStreamOut  *pTempOut = (AudioMTKStreamOut *)mStreamOutVector.valueAt(i);
            pTempOut->setParameters(keyValuePairs);
        }
    }
    return NO_ERROR;
}


}




