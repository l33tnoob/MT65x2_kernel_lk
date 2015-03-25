#include <utils/String16.h>
#include "AudioMTKStreamManagerBase.h"
#include "AudioUtility.h"
#include "AudioMTKStreamOut.h"
#include "AudioMTKStreamIn.h"
#include "AudioMTKStreamInManager.h"
#include "AudioCompFltCustParam.h"

#define LOG_TAG "AudioMTKStreamManagerBase"
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


status_t  AudioMTKStreamManagerBase::initCheck()
{
    ALOGD("AudioMTKStreamManagerBase initCheck \n");
    return NO_ERROR;
}

bool  AudioMTKStreamManagerBase::StreamExist()
{
    ALOGD("AudioMTKStreamManagerBase StreamExist \n");

    return false;
}

bool  AudioMTKStreamManagerBase::InputStreamExist()
{
    ALOGD("AudioMTKStreamManagerBase InputStreamExist \n");
    return false;
}

bool  AudioMTKStreamManagerBase::OutputStreamExist()
{
    ALOGD("AudioMTKStreamManagerBase OutputStreamExist \n");
    return false;
}

status_t  AudioMTKStreamManagerBase::closeInputStream(android_audio_legacy::AudioStreamIn *in)
{
    ALOGD("AudioMTKStreamManagerBase closeInputStream \n");
    return NO_ERROR;
}

android_audio_legacy::AudioStreamIn *AudioMTKStreamManagerBase::openInputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    ALOGD("AudioMTKStreamManagerBase openInputStream, devices = 0x%x format=0x%x ,channels=0x%x, rate=%d acoustics = 0x%x", devices, *format, *channels, *sampleRate, acoustics);
    return NULL;
}

android_audio_legacy::AudioStreamOut *AudioMTKStreamManagerBase::openOutputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    uint32_t output_flag)
{
    ALOGD("AudioMTKStreamManagerBase openOutputStream \n");
    return NULL;
}

status_t AudioMTKStreamManagerBase::closeOutputStream(android_audio_legacy::AudioStreamOut *out)
{
    ALOGD("AudioMTKStreamManagerBase closeOutputStream ");
    return NO_ERROR;
}

bool AudioMTKStreamManagerBase::IsOutPutStreamActive()
{
    ALOGD("AudioMTKStreamManagerBase IsOutPutStreamActive ");
    return false;
}

bool AudioMTKStreamManagerBase::IsInPutStreamActive()
{
    ALOGD("AudioMTKStreamManagerBase IsInPutStreamActive ");
    return false;
}

// set musicplus to streamout
void AudioMTKStreamManagerBase::SetMusicPlusStatus(bool bEnable)
{
    ALOGD("AudioMTKStreamManagerBase SetMusicPlusStatus ");

}

bool AudioMTKStreamManagerBase::GetMusicPlusStatus()
{
    ALOGD("AudioMTKStreamManagerBase GetMusicPlusStatus ");

    return false;
}

size_t AudioMTKStreamManagerBase::getInputBufferSize(int32_t sampleRate, int format, int channelCount)
{
    ALOGD("AudioMTKStreamManagerBase getInputBufferSize ");

    return 320;
}

status_t AudioMTKStreamManagerBase::UpdateACFHCF(int value)
{
    ALOGD("AudioMTKStreamManagerBase UpdateACFHCF ");

    return NO_ERROR;
}

// ACF Preview parameter
status_t AudioMTKStreamManagerBase::SetACFPreviewParameter(void *ptr , int len)
{
    ALOGD("AudioMTKStreamManagerBase SetACFPreviewParameter ");

    return NO_ERROR;
}

status_t AudioMTKStreamManagerBase::SetHCFPreviewParameter(void *ptr , int len)
{
    ALOGD("AudioMTKHardware SetHCFPreviewParameter\n");
    return NO_ERROR;
}

//suspend input and output standby
status_t AudioMTKStreamManagerBase::SetOutputStreamSuspend(bool bEnable)
{
    ALOGD("SetOutputStreamSuspend");
    return NO_ERROR;
}

status_t AudioMTKStreamManagerBase::SetInputStreamSuspend(bool bEnable)
{
    ALOGD("SetOutputStreamSuspend");
    return NO_ERROR;
}

status_t AudioMTKStreamManagerBase::ForceAllStandby()
{
    // force all stream to standby
    ALOGD("AudioMTKStreamManagerBasev ForceAllStandby");
    return NO_ERROR;
}

// FSync flag
bool AudioMTKStreamManagerBase::GetFSyncFlag(int streamType)
{
    ALOGD("AudioMTKStreamManagerBasev GetFSyncFlag");

    return false;
}

void AudioMTKStreamManagerBase::ClearFSync(int streamType)
{
    ALOGD("AudioMTKStreamManagerBasev ClearFSync");
}

status_t AudioMTKStreamManagerBase::setParameters(const String8 &keyValuePairs, int IOport)
{
    ALOGD("AudioMTKStreamManagerBasev setParameters");
    return NO_ERROR;
}

String8  AudioMTKStreamManagerBase::getParameters(const String8 &keys, int IOport)
{
    ALOGD("AudioMTKStreamManagerBasev getParameters");
    String8 temp;
    return temp;
}

void AudioMTKStreamManagerBase::SetInputMute(bool bEnable)
{
    ALOGD("AudioMTKStreamManagerBase  SetInputMute");
}

void AudioMTKStreamManagerBase::SetBesLoudnessStatus(bool bEnable)
{
    ALOGD("AudioMTKStreamManagerBase SetBesLoudnessStatus ");

}

bool AudioMTKStreamManagerBase::GetBesLoudnessStatus()
{
    ALOGD("AudioMTKStreamManagerBase GetBesLoudnessStatus ");

    return false;
}

void AudioMTKStreamManagerBase::SetBesLoudnessControlCallback(const BESLOUDNESS_CONTROL_CALLBACK_STRUCT *callback_data)
{
    ALOGD("AudioMTKStreamManagerBase SetBesLoudnessControlCallback ");
}

}




