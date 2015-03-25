


#define LOG_TAG  "AudioMTKFilter"

#include <media/AudioSystem.h>
#include <cutils/compiler.h>

#include "audio_custom_exp.h"
#include "AudioCustParam.h"
#include "CFG_AUDIO_File.h"
#include"AudioMTKFilter.h"
#include <cutils/xlog.h>

#include "AudioType.h"
#include "AudioUtility.h"

namespace
{

//Configure ACF Work Mode
#if defined(ENABLE_AUDIO_COMPENSATION_FILTER) && defined(ENABLE_AUDIO_DRC_SPEAKER)
//5
#define AUDIO_COMPENSATION_FLT_MODE AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC

#elif defined(ENABLE_AUDIO_COMPENSATION_FILTER)
// 4
#define AUDIO_COMPENSATION_FLT_MODE AUDIO_CMP_FLT_LOUDNESS_COMP

#elif defined(ENABLE_AUDIO_DRC_SPEAKER)
// 3
#define AUDIO_COMPENSATION_FLT_MODE AUDIO_CMP_FLT_LOUDNESS_LITE

#endif

}


namespace android
{

AudioMTKFilter::AudioMTKFilter(
    AudioCompFltType_t type,
    AudioComFltMode_t mode,
    uint32_t sampleRate,
    uint32_t channel,
    uint32_t format,
    size_t bufferSize)
    : mType(type),
      mMode(mode),
      mSampleTate(sampleRate),
      mChannel(channel),
      mFormat(format),
      mBufferSize(bufferSize),
      mFilter(NULL),
      mStart(false),
      mActive(false)
{
    init();
}

AudioMTKFilter::~AudioMTKFilter()
{
    if (mFilter)
    {
        //mFilter->Stop();
        mFilter->Close();
        //mFilter->Deinit();
    }
}

status_t AudioMTKFilter::init()
{
    Mutex::Autolock _l(&mLock);
    if (mType < AUDIO_COMP_FLT_NUM)
    {
        mFilter = new MtkAudioLoud(mType);

        if (NULL != mFilter)
        {
            //mFilter->Init();
            //mFilter->LoadACFParameter();
            mFilter->SetParameter(BLOUD_PAR_SET_USE_DEFAULT_PARAM, (void *)NULL);
#if defined(ENABLE_STEREO_SPEAKER)&&defined(MTK_STEREO_SPK_ACF_TUNING_SUPPORT)
            if (AUDIO_COMP_FLT_AUDIO == mType)
            {
                mFilter->SetParameter(BLOUD_PAR_SET_SEP_LR_FILTER, (void *)true);
                mFilter->SetParameter(BLOUD_PAR_SET_USE_DEFAULT_PARAM_SUB, (void *)NULL);
            }
#endif
            int format = (mFormat == AUDIO_FORMAT_PCM_32_BIT ? BLOUD_IN_Q1P31_OUT_Q1P31 : BLOUD_IN_Q1P15_OUT_Q1P15);
            mFilter->SetParameter(BLOUD_PAR_SET_PCM_FORMAT, (void *)format);
            return NO_ERROR;
        }
    }
    return NO_INIT;
}

void AudioMTKFilter::start()
{
    Mutex::Autolock _l(mLock);
    if (mFilter && !mActive)
    {
        SXLOGD("AudioMTKFilter::start() type %d mode %d", mType, mMode);

        mFilter->SetWorkMode(mChannel, mSampleTate, mMode);
        mFilter->Open();
        mStart  = true;
        mActive = true;
    }
    return;
}

void AudioMTKFilter::stop()
{
    Mutex::Autolock _l(mLock);
    if (mFilter && mActive)
    {
        SXLOGD("AudioMTKFilter::stop() type %d mode %d", mType, mMode);
        //mFilter->Stop();
        //        mFilter->Close();
        mFilter->ResetBuffer();
        mStart  = false;
        mActive = false;
    }
    return;
}

void AudioMTKFilter::pause()
{
    Mutex::Autolock _l(mLock);
    if (mFilter && mActive)
    {
        SXLOGD("AudioMTKFilter::pause() type %d mode %d", mType, mMode);
        if (mFilter->Change2ByPass() == ACE_SUCCESS)
        {
        mActive = false;
    }
}
}

void AudioMTKFilter::resume()
{
    Mutex::Autolock _l(mLock);
    if (mFilter && !mActive)
    {
        SXLOGD("AudioMTKFilter::resume() type %d mode %d", mType, mMode);
        if (mFilter->Change2Normal() == ACE_SUCCESS)
        {
        mActive = true;
    }
}
}

bool AudioMTKFilter::isStart()
{
    Mutex::Autolock _l(mLock);
    return mStart;
}

bool AudioMTKFilter::isActive()
{
    Mutex::Autolock _l(mLock);
    return mActive;
}

void AudioMTKFilter::setParameter(void *param)
{
    Mutex::Autolock _l(mLock);
    if (mFilter)
    {
        SXLOGD("AudioMTKFilter::setParameter type %d mode %d mActive %d", mType, mMode,mActive);
        mFilter->ResetBuffer();
        mFilter->Close();                
        mFilter->SetParameter(BLOUD_PAR_SET_PREVIEW_PARAM, (void *)param);
        mFilter->SetParameter(BLOUD_PAR_SET_CHANNEL_NUMBER, (void *)mChannel);
        mFilter->SetParameter(BLOUD_PAR_SET_SAMPLE_RATE, (void *)mSampleTate);
        mFilter->SetParameter(BLOUD_PAR_SET_WORK_MODE, (void *)mMode);
        mFilter->Open();
        if(!mActive && mStart)
            mFilter->Change2ByPass();
        
    }
}

void AudioMTKFilter::setParameter2Sub(void *param)
{
#if defined(ENABLE_STEREO_SPEAKER)&&defined(MTK_STEREO_SPK_ACF_TUNING_SUPPORT)

    Mutex::Autolock _l(mLock);
    if (mFilter)
    {
        SXLOGV("AudioMTKFilter::setParameter2Sub type %d mode %d mActive %d", mType, mMode,mActive);
        mFilter->ResetBuffer();
        mFilter->Close();                
        mFilter->SetParameter(BLOUD_PAR_SET_PREVIEW_PARAM_SUB, (void *)param);
        mFilter->SetParameter(BLOUD_PAR_SET_CHANNEL_NUMBER, (void *)mChannel);
        mFilter->SetParameter(BLOUD_PAR_SET_SAMPLE_RATE, (void *)mSampleTate);
        mFilter->SetParameter(BLOUD_PAR_SET_WORK_MODE, (void *)mMode);
        mFilter->Open();
        if(!mActive && mStart)
            mFilter->Change2ByPass();         
    }
#else
    SXLOGD("UnSupport Stereo Speaker.");
#endif
}


uint32_t AudioMTKFilter::process(void *inBuffer, uint32_t bytes, void *outBuffer, uint32_t outBytes)
{
    // if return 0, means CompFilter can't do anything. Caller should use input buffer to write to Hw.
    // do post process
    Mutex::Autolock _l(mLock);
    if (mFilter && mStart)
    {
        //SXLOGD("AudioMTKFilter::process type %d mode %d", mType, mMode);
        uint32_t inBytes =  bytes;
        uint32_t outBytes2 =  outBytes;

        mFilter->Process((short *)inBuffer, &inBytes, (short *)outBuffer, &outBytes2);
        //SXLOGD("AudioMTKFilter::process type %d mode %d", mType, mMode);
        return outBytes2;
    }
    return 0;
}


//filter manager
#undef  LOG_TAG
#define LOG_TAG  "AudioMTKFilterManager"

AudioMTKFilterManager::AudioMTKFilterManager(
    uint32_t sampleRate,
    uint32_t channel,
    uint32_t format,
    size_t bufferSize)
    : mSamplerate(sampleRate),
      mChannel(channel),
      mFormat(format),
      mBufferSize(bufferSize),
      mFixedParam(false),
      mSpeakerFilter(NULL),
      mHeadphoneFilter(NULL),
      mEnhanceFilter(NULL),
//#if defined(MTK_VIBSPK_SUPPORT)
      mVIBSPKFilter(NULL),
//#endif
      mBuffer(NULL),
      mDevices(0)
{
    init();
}

AudioMTKFilterManager::~AudioMTKFilterManager()
{
    if (mSpeakerFilter)
    {
        mSpeakerFilter->stop();
        delete mSpeakerFilter;
        mSpeakerFilter = NULL;
    }
    if (mHeadphoneFilter)
    {
        mHeadphoneFilter->stop();
        delete mHeadphoneFilter;
        mHeadphoneFilter = NULL;
    }
    if (mEnhanceFilter)
    {
        mEnhanceFilter->stop();
        delete mEnhanceFilter;
        mEnhanceFilter = NULL;
    }
    if (mBuffer)
    {
        delete[] mBuffer;
        mBuffer = NULL;
    }

}

bool AudioMTKFilterManager::init()
{

#if defined(ENABLE_AUDIO_COMPENSATION_FILTER)||defined(ENABLE_AUDIO_DRC_SPEAKER)
    mSpeakerFilter = new AudioMTKFilter(AUDIO_COMP_FLT_AUDIO, AUDIO_COMPENSATION_FLT_MODE,
                                        mSamplerate, mChannel, mFormat, mBufferSize);
    ASSERT(mSpeakerFilter != NULL);
#endif

#if defined(ENABLE_HEADPHONE_COMPENSATION_FILTER)
    mHeadphoneFilter = new AudioMTKFilter(AUDIO_COMP_FLT_HEADPHONE, AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE,
                                          mSamplerate, mChannel, mFormat, mBufferSize);
    ASSERT(mHeadphoneFilter != NULL);
#endif

#if defined(MTK_AUDENH_SUPPORT) //For reduce resource
    mEnhanceFilter = new AudioMTKFilter(AUDIO_COMP_FLT_AUDENH, AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH,
                                        mSamplerate, mChannel, mFormat, mBufferSize);

    ASSERT(mEnhanceFilter != NULL);

    mBuffer = new uint8_t[mBufferSize];
    unsigned int result = 0 ;
#if 0
    char value[PROPERTY_VALUE_MAX];
    int result = 0 ;
    property_get("persist.af.audenh.ctrl", value, "1");
    result = atoi(value);
#else

#ifndef HIFI_SWITCH_BY_AUDENH //HP switch use AudEnh setting, remove original AudEnh control
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT audioParam;
    if (GetAudEnhControlOptionParamFromNV(&audioParam))
    {
        result = audioParam.u32EnableFlg;
    }
#endif
#endif
    mFixedParam = (result ? true : false);
#endif

//#if defined(MTK_VIBSPK_SUPPORT)
    mVIBSPKFilter = new AudioMTKFilter(AUDIO_COMP_FLT_VIBSPK, AUDIO_CMP_FLT_LOUDNESS_COMP,
                                        mSamplerate, mChannel, mFormat, mBufferSize);

    ASSERT(mVIBSPKFilter != NULL);
    if(NULL == mBuffer)
    {
        mBuffer = new uint8_t[mBufferSize];
    }

    ASSERT(mBuffer != NULL);

//#endif


    SXLOGD("init() fixedParam %d", mFixedParam);

    return NO_ERROR;
}

void AudioMTKFilterManager::start()
{
    uint32_t device = mDevices;
    SXLOGV("start() device 0x%x", device);

    if (device & AUDIO_DEVICE_OUT_SPEAKER)
    {
        //stop hcf & enhangce
        if (mHeadphoneFilter) { mHeadphoneFilter->stop(); }
        if (mEnhanceFilter) { mEnhanceFilter->stop(); }
        // start acf
        if (mSpeakerFilter) { mSpeakerFilter->start(); }
//#if defined(MTK_VIBSPK_SUPPORT)
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
        if (mVIBSPKFilter) { mVIBSPKFilter->start(); }
//#endif

    }
    else if ((device & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
    {
//#if defined(MTK_VIBSPK_SUPPORT)
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
        if (mVIBSPKFilter) { mVIBSPKFilter->stop(); }
//#endif
        // stop acf
        if (mSpeakerFilter) { mSpeakerFilter->stop(); }
        // start hcf
        if (mHeadphoneFilter) { mHeadphoneFilter->start(); }

        if (mEnhanceFilter)
        {
            if (false == mFixedParam)
            {
                if (mEnhanceFilter->isStart()) { mEnhanceFilter->pause(); }
            }
            else
            {
                if (!mEnhanceFilter->isStart())
                {
                    mEnhanceFilter->start();
                }
                else
                {
                    mEnhanceFilter->resume();
                }
            }
        }
    }
//#if defined(MTK_VIBSPK_SUPPORT)
    else if(device & AUDIO_DEVICE_OUT_EARPIECE)
    {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER) && IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
        {
        if (mHeadphoneFilter) { mHeadphoneFilter->stop(); }
        if (mEnhanceFilter) { mEnhanceFilter->stop(); }
        if (mSpeakerFilter) { mSpeakerFilter->stop(); }
        if (mVIBSPKFilter) { mVIBSPKFilter->start(); }
    }   
    }   
//#endif

}

void AudioMTKFilterManager::stop()
{
    SXLOGV("stop()");
//#if defined(MTK_VIBSPK_SUPPORT)
    if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
    if (mVIBSPKFilter) { mVIBSPKFilter->stop(); }
//#endif
    if (mSpeakerFilter) { mSpeakerFilter->stop(); }
    if (mHeadphoneFilter) { mHeadphoneFilter->stop(); }
    if (mEnhanceFilter) { mEnhanceFilter->stop(); }
}

bool  AudioMTKFilterManager::isFilterStart(uint32_t type)
{
    if (type == AUDIO_COMP_FLT_AUDIO && mSpeakerFilter)
    {
        return mSpeakerFilter->isStart();
    }
    else if (type == AUDIO_COMP_FLT_HEADPHONE && mHeadphoneFilter)
    {
        return mHeadphoneFilter->isStart();
    }
    else if (type == AUDIO_COMP_FLT_AUDENH && mEnhanceFilter)
    {
        return mEnhanceFilter->isStart();
    }
//#if defined(MTK_VIBSPK_SUPPORT)
    else if (type == AUDIO_COMP_FLT_VIBSPK && mVIBSPKFilter && (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER)))
    {
        return mVIBSPKFilter->isStart();
    }
//#endif

    return false;
}

void AudioMTKFilterManager::setParamFixed(bool flag)
{
    SXLOGV("setParamFixed() flag %d", flag);

#if defined(MTK_AUDENH_SUPPORT)
    mFixedParam = flag;
#if 0
    if (flag)
    {
        property_set("persist.af.audenh.ctrl", "1");
    }
    else
    {
        property_set("persist.af.audenh.ctrl", "0");
    }
#else

#ifndef HIFI_SWITCH_BY_AUDENH    //HP switch use AudEnh setting, remove original AudEnh control
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT audioParam;
    audioParam.u32EnableFlg = flag ? 1 : 0;
    SetAudEnhControlOptionParamToNV(&audioParam);
#endif
#endif
#else
    SXLOGW("Unsupport AudEnh Feature");
#endif
}

bool AudioMTKFilterManager::isParamFixed()
{
    return mFixedParam;
}

void AudioMTKFilterManager::setDevice(uint32_t devices)
{
    mDevices = devices;
    return;
}

uint32_t  AudioMTKFilterManager::process(void *inBuffer, uint32_t bytes, void *outBuffer, uint32_t outSize)
{
    SXLOGV("+process() insize %u", bytes);
    uint32_t outputSize = 0;
    uint32_t device = mDevices;
    if (device & AUDIO_DEVICE_OUT_SPEAKER)
    {
        if (mSpeakerFilter)
        {
            if (mSpeakerFilter->isStart())
            {
                outputSize = mSpeakerFilter->process(inBuffer, bytes, outBuffer, outSize);
            }
        }
//#if defined(MTK_VIBSPK_SUPPORT)
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
        {
        if (mVIBSPKFilter)
        {
            if (mVIBSPKFilter->isStart())
            {
                void *out = mBuffer;

                if (CC_UNLIKELY(outputSize == 0))
                {
                        outputSize = mVIBSPKFilter->process(inBuffer, bytes, outBuffer, outSize);
                }
                else
                {
                    void *in = outBuffer;
                    void *out = mBuffer;
                        outputSize = mVIBSPKFilter->process(in, bytes, out, outSize);
                    if (outputSize > 0) { memcpy(outBuffer, out, outputSize); }
                }           
            }
        }
        }
//#endif 
    }
    else if ((device & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
    {
        if (mEnhanceFilter)
        {
            if (mEnhanceFilter->isStart())
            {
                outputSize = mEnhanceFilter->process(inBuffer, bytes, outBuffer, outSize);
            }
        }
        if (mHeadphoneFilter)
        {
            if (mHeadphoneFilter->isStart())
            {
                if (CC_UNLIKELY(outputSize == 0))
                {
                    outputSize = mHeadphoneFilter->process(inBuffer, bytes, outBuffer, outSize);
                }
                else
                {
                    void *in = outBuffer;
                    void *out = mBuffer;
                    outputSize = mHeadphoneFilter->process(in, outputSize, out, outSize);
                    if (outputSize > 0) { memcpy(outBuffer, out, outputSize); }
                }
            }
        }
    }
//#if defined(MTK_VIBSPK_SUPPORT)
    else if(device & AUDIO_DEVICE_OUT_EARPIECE)
    {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER) && IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
        {
        if (mVIBSPKFilter)
        {            
            if (mVIBSPKFilter->isStart())
            {
                    outputSize = mVIBSPKFilter->process(inBuffer, bytes, outBuffer, outSize);
            }
        }
    }
    }
//#endif

    SXLOGV("-process() outsize %u", outputSize);
    return outputSize;

}

void AudioMTKFilterManager::setParameter(uint32_t type, void *param)
{
    SXLOGV("setParameter() type %u", type);

    if (type == AUDIO_COMP_FLT_AUDIO && mSpeakerFilter)
    {
        return mSpeakerFilter->setParameter(param);
    }
    else if (type == AUDIO_COMP_FLT_HEADPHONE && mHeadphoneFilter)
    {
        return mHeadphoneFilter->setParameter(param);
    }
    else if (type == AUDIO_COMP_FLT_AUDENH && mEnhanceFilter)
    {
        return mEnhanceFilter->setParameter(param);
    }
    else if (type == AUDIO_COMP_FLT_AUDIO_SUB && mSpeakerFilter)
    {
        return mSpeakerFilter->setParameter2Sub(param);
    }
}

}

