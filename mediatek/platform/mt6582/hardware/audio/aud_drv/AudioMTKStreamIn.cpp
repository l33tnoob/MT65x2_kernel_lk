#include "AudioUtility.h"
#include "AudioMTKStreamIn.h"
#include "AudioResourceManager.h"
#include "AudioMTKStreamInManager.h"
#include "AudioMTKStreamInManagerInterface.h"
#include <utils/Log.h>
#include <math.h>

#include "SpeechDriverFactory.h"
#include "SpeechVMRecorder.h"

#include "AudioCustParam.h"

#ifdef MTK_AUDIO_HD_REC_SUPPORT
#include "CFG_Audio_Default.h"
#endif
#include "AudioVUnlockDL.h"
#define MAX_FILE_LENGTH (60000000)

#include "AudioBTCVSDControl.h"

#include "AudioFMController.h"
#include "AudioMATVController.h"


extern "C" {
#include "bli_exp.h"
}

#define LOG_TAG "AudioMTKStreamIn"
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

#define VOICE_RECOGNITION_RECORD_SAMPLE_RATE (16000)
#define HD_RECORD_SAMPLE_RATE (48000)
#define DAIBT_SAMPLE_RATE (8000)
#define MAX_DUMP_NUM (6)
#define VOIP_PLATFORM_OFFSET_TIME (-2)

#ifdef MTK_AUDIO_HD_REC_SUPPORT
static const unsigned long HDRecordEnhanceParasCommon[] =
{
    0,
    0,
    0,
    10752,
    32769,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const AUDIO_HD_RECORD_SCENE_TABLE_STRUCT DefaultRecordSceneTable[] = {Hd_Recrod_Scene_Table_default};
static const AUDIO_HD_RECORD_PARAM_STRUCT DefaultRecordParam[] = {Hd_Recrod_Par_default};
int AudioMTKStreamIn::DumpFileNum = 0;
#endif

bool AudioMTKStreamIn::CheckFormat(int *pFormat)
{
    if (*pFormat != AUDIO_FORMAT_PCM_16_BIT)
    {
        return false;
    }
    return true;
}

bool AudioMTKStreamIn::CheckSampleRate(uint32_t device, uint32_t *pRate)
{
    if (mIsHDRecTunningEnable || mIsAPDMNRTuningEnable)  //tool tunning case
    {
        ALOGD("CheckSampleRate HDRecTunningEnable \n");
        if (*pRate == 48000)
        {
            mHDRecTunning16K = false;
            return true;
        }
        else if (*pRate == 16000)
        {
            mHDRecTunning16K = true;    //VR or DMNR tuning(Depends on input source)
            *pRate = 48000;
            return true;
        }

        return false;
    }
    if (device == AUDIO_DEVICE_IN_FM)
    {
        if (*pRate != AudioFMController::GetInstance()->GetFmUplinkSamplingRate())
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else if (device == AUDIO_DEVICE_IN_MATV)
    {
        if (*pRate != AudioMATVController::GetInstance()->GetMatvUplinkSamplingRate())
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else if (*pRate != mStream_Default_SampleRate)
    {
        return false;
    }
    return true;
}

bool AudioMTKStreamIn::CheckChannel(uint32_t device, uint32_t *pChannels)
{
    if (*pChannels !=  AUDIO_CHANNEL_IN_STEREO)
    {
        return false;
    }
    return true;
}

status_t AudioMTKStreamIn::dump(int fd, const Vector<String16> &args)
{
    return NO_ERROR;
}

AudioMTKStreamIn::AudioMTKStreamIn()
{
    ALOGD("AudioMTKStreamIn contructor \n");
    memset((void *)&mAttribute, 0 , sizeof(AudioStreamAttribute));
    mStreamInManager = AudioMTKStreamInManager::getInstance();

    if (mStreamInManager == NULL)
    {
        ALOGW("AudioMTKStreamIn get mStreamInManager fail!! ");
    }
    mAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();
    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    mStreamInClient = NULL;
    SetStreamInPreprocessStatus(true);
    mStarting = false;
    mLatency = 0;
    mSuspend = false;
    mPAdcPCMDumpFile = NULL;
    mPAdcPCMInDumpFile = NULL;
    mReadCount = 0;
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    mpSPELayer = NULL;
    mHDRecordModeIndex = -1;
    mHDRecordSceneIndex = -1;
    mStereoMode = false;
    mHDRecordParamUpdate = false;
#endif

    mAPPS = NULL;
    mEcho_Reference = NULL;

#ifdef MTK_DUAL_MIC_SUPPORT
    mLRChannelSwitch = false;
    mSpecificMicChoose = 0;
#endif

    mBliHandler1 = NULL;
    mBliHandler1Buffer = NULL;
    mBliHandler2 = NULL;
    mBliHandler2Buffer = NULL;

    mBliSrc = NULL;
    mBliSrc = new BliSrc();

    mSwapBufferTwo = NULL;
    mSwapBufferTwo = new uint8_t[bufferSize()];
    if (mSwapBufferTwo == NULL)
    {
        ALOGE("mSwapBufferTwo for BliSRC allocate fail1!!! \n");
    }

    mIsHDRecTunningEnable = mAudioSpeechEnhanceInfoInstance->IsHDRecTunningEnable();
#ifndef DMNR_TUNNING_AT_MODEMSIDE
    mIsAPDMNRTuningEnable = mAudioSpeechEnhanceInfoInstance->IsAPDMNRTuningEnable();
#else
    mIsAPDMNRTuningEnable = false;
#endif
    memset(m_strTunningFileName, 0, 128);
    mAudioSpeechEnhanceInfoInstance->GetHDRecVMFileName(m_strTunningFileName);
    mHDRecTunning16K = false;
    mVoIPRunning = false;
    mForceMagiASREnable = false;
    mForceAECRecEnable = false;
    mUsingMagiASR = false;
    mUsingAECRec = false;
    if (mAudioSpeechEnhanceInfoInstance->GetForceMagiASRState() > 0)
    {
        mForceMagiASREnable = true;
        ALOGD("AudioMTKStreamIn mForceMagiASREnable is enabled \n");
    }
    if (mAudioSpeechEnhanceInfoInstance->GetForceAECRecState())
    {
        mForceAECRecEnable = true;
        ALOGD("AudioMTKStreamIn mForceAECRecEnable is enabled \n");
    }
}

uint32_t AudioMTKStreamIn::StreamInPreprocess(void *buffer , uint32_t bytes)
{
    uint32_t ProcessedBytes = bytes, ReadDataBytes = bytes;
    char *pRead = (char *)buffer;
    //    ALOGD("StreamInPreprocess buffer = %p bytes = %d, pRead=%p", buffer, bytes,pRead);
    // do streamin preprocess
    if (mEnablePreprocess)
    {
        ProcessedBytes = NativeRecordPreprocess((void *)pRead, ProcessedBytes);
        //ALOGD("StreamInPreprocess bytes=%d,ProcessedBytes=%d",bytes,ProcessedBytes);
    }
    return ProcessedBytes;
}

status_t AudioMTKStreamIn::SetStreamInPreprocessStatus(bool Enable)
{
    ALOGD("SetStreamInPreprocessStatus enable = %d, %p", Enable, mStreamInClient);
    mEnablePreprocess = Enable;

    if (mStreamInClient != NULL)
    {
        mStreamInClient->mEnableBesRecord = mEnablePreprocess;
    }
    return NO_ERROR;
}

uint32 AudioMTKStreamIn::GetBufferSizeBydevice(uint32_t devices)
{
    switch (devices)
    {
        case AUDIO_DEVICE_IN_AMBIENT :
        case AUDIO_DEVICE_IN_BUILTIN_MIC :
        case AUDIO_DEVICE_IN_WIRED_HEADSET :
        case AUDIO_DEVICE_IN_BACK_MIC :
        case AUDIO_DEVICE_IN_COMMUNICATION :
        case AUDIO_DEVICE_IN_VOICE_CALL:
            return Default_Mic_Buffer;
            break;
        case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET :
            return Default_BT_Buffer;
            break;
        case AUDIO_DEVICE_IN_FM :
        case AUDIO_DEVICE_IN_MATV:
            return Default_Mic_Buffer;
            break;
        default:
            ALOGW("%s(), devices(0x%x) use default Default_Mic_Buffer(0x%x)", __FUNCTION__, devices, Default_Mic_Buffer);
            return Default_Mic_Buffer;
    }
    return Default_Mic_Buffer;
}
void AudioMTKStreamIn::Set(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    ALOGD("AudioMTKStreamIn set devices = 0x%x format = 0x%x channele = 0x%x samplerate = %d, mIsHDRecTunningEnable=%x,mIsAPDMNRTuningEnable=%x",
          devices, *format, *channels, *sampleRate, mIsHDRecTunningEnable, mIsAPDMNRTuningEnable);

    // check if can contruct successfully
    if (CheckFormat(format) && CheckSampleRate(devices, sampleRate) && CheckChannel(devices, channels))
    {
        // stream attribute
        mAttribute.mdevices = devices;
        mAttribute.mSampleRate = *sampleRate;
        mAttribute.mChannels = *channels;
        mAttribute.mAcoustic = acoustics;
        mAttribute.mFormat = *format;
        mAttribute.mInterruptSample = 0;
        mAttribute.mBufferSize = GetBufferSizeBydevice(devices);
        ALOGD("set mAttribute.mBufferSize  = %d", mAttribute.mBufferSize);
        mAttribute.mPredevices = devices;
        if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))       //for digital MIC sample rate constrain
        {
            mAttribute.mIsDigitalMIC = true;
        }
        else
        {
            mAttribute.mIsDigitalMIC = false;
        }

        *status = NO_ERROR;
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        mpSPELayer = new SPELayer();
        if (!mpSPELayer)
        {
            ALOGE("new SPELayer() FAIL");
        }
        else
        {
            mAudioSpeechEnhanceInfoInstance->SetSPEPointer(this, mpSPELayer);
            LoadHDRecordParams();
#ifndef DMNR_TUNNING_AT_MODEMSIDE
            mpSPELayer->SetVMDumpEnable(mIsHDRecTunningEnable || mIsAPDMNRTuningEnable);
            mpSPELayer->SetVMDumpFileName(m_strTunningFileName);
#endif
            StreamAttribute DLSA;
            DLSA.mChannels = mAudioSpeechEnhanceInfoInstance->GetOutputChannelInfo();
            DLSA.mSampleRate = mAudioSpeechEnhanceInfoInstance->GetOutputSampleRateInfo();
            DLSA.mBufferSize = mAudioSpeechEnhanceInfoInstance->GetOutputBufferSize();
            mpSPELayer->SetStreamAttribute(0, DLSA);
        }
#endif
        mAPPS = new AudioPreProcess();
        mEcho_Reference = NULL;
        if (!mAPPS)
        {
            ALOGD("mAPPS() FAIL");
        }

    }
    else
    {
        // modify default paramters and let Audiorecord open again for reampler.
        if (devices == AUDIO_DEVICE_IN_FM)
        {
            *sampleRate = AudioFMController::GetInstance()->GetFmUplinkSamplingRate();
        }
        else if (devices == AUDIO_DEVICE_IN_MATV)
        {
            *sampleRate = AudioMATVController::GetInstance()->GetMatvUplinkSamplingRate();
        }
        else
        {
            *sampleRate = mStream_Default_SampleRate;
        }
        *format  = mStream_Default_Format;

        *channels =  mStream_Default_Channels;
        *status = BAD_VALUE;
    }
}


AudioMTKStreamIn::~AudioMTKStreamIn()
{
    ALOGD("AudioMTKStreamIn destructor");

    standby();

#ifdef MTK_AUDIO_HD_REC_SUPPORT
    if (mpSPELayer != NULL)
    {
        mAudioSpeechEnhanceInfoInstance->ClearSPEPointer(this);
        delete mpSPELayer;
        mpSPELayer = NULL;
    }
#endif

    if (mAPPS)
    {
        delete mAPPS;
        mAPPS = NULL;
        ALOGD("delete mAPPS() ");
        mEcho_Reference = NULL;
    }

    if (mBliSrc)
    {
        mBliSrc->close();
        delete mBliSrc;
        mBliSrc = NULL;
    }
    if (mSwapBufferTwo)
    {
        delete []mSwapBufferTwo;
        mSwapBufferTwo = NULL;
    }
}

uint32_t AudioMTKStreamIn::sampleRate() const
{
    return mAttribute.mSampleRate;
}

uint32_t AudioMTKStreamIn::bufferSize() const
{
    return mAttribute.mBufferSize;
}

uint32_t AudioMTKStreamIn::channels() const
{
    return mAttribute.mChannels;
}

int AudioMTKStreamIn::format() const
{
    return mAttribute.mFormat;
}

status_t AudioMTKStreamIn::setGain(float gain)
{
    return NO_ERROR;
}

bool AudioMTKStreamIn::SetIdentity(uint32_t id)
{
    mIdentity = id;
    return true;
}

uint32_t AudioMTKStreamIn::GetIdentity()
{
    return mIdentity;
}

status_t AudioMTKStreamIn::SetClientSourceandMemType(AudioMTKStreamInClient *mStreamInClient)
{
    ALOGD("+SetClientSourceandMemType mStreamInClient = %p", mStreamInClient);
    if (mStreamInClient == NULL)
    {
        ALOGW("SetClientSourceandMemType with null pointer");
        return INVALID_OPERATION;
    }

    switch (mStreamInClient->mAttributeClient->mSource)
    {
        case AUDIO_SOURCE_DEFAULT:
        case AUDIO_SOURCE_MIC:
        case AUDIO_SOURCE_VOICE_UPLINK :
        case AUDIO_SOURCE_VOICE_DOWNLINK :
        case AUDIO_SOURCE_VOICE_CALL :
        case AUDIO_SOURCE_CAMCORDER :
            mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
            break;
        case AUDIO_SOURCE_VOICE_RECOGNITION :
            mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
            break;
        case AUDIO_SOURCE_VOICE_COMMUNICATION :
            mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
            break;
        case AUDIO_SOURCE_VOICE_UNLOCK:
            mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
            break;
        case AUDIO_SOURCE_CUSTOMIZATION1:
        case AUDIO_SOURCE_CUSTOMIZATION2:
        case AUDIO_SOURCE_CUSTOMIZATION3:
            mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
            break;
        case AUDIO_SOURCE_MATV:
            mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_2;
            break;
        case AUDIO_SOURCE_FM:
            mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_2;
            break;
    }

    switch (mStreamInClient->mAttributeClient->mdevices)
    {
        case AUDIO_DEVICE_IN_AMBIENT :
        case AUDIO_DEVICE_IN_BUILTIN_MIC :
        case AUDIO_DEVICE_IN_WIRED_HEADSET :
        case AUDIO_DEVICE_IN_BACK_MIC :
        case AUDIO_DEVICE_IN_COMMUNICATION :
        case AUDIO_DEVICE_IN_VOICE_CALL:
            mStreamInClient->mMemDataType = AudioDigitalType::MEM_VUL;
            break;
            // for BT scenario , it's a special case nned to modify source to DAI_BT
        case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET :
            mStreamInClient->mMemDataType = AudioDigitalType::MEM_DAI;
            mStreamInClient->mSourceType = AudioDigitalType::DAI_BT;
            break;
        case AUDIO_DEVICE_IN_FM :
        case AUDIO_DEVICE_IN_MATV:
            mStreamInClient->mMemDataType = AudioDigitalType::MEM_AWB;
            break;
        default:
            ALOGW("no proper device match !!!");
            break;
    }
    ALOGD("-%s(), mStreamInClient = %p, mStreamInClient->mSourceType = %d, mStreamInClient->mMemDataType = %d\n", __FUNCTION__, mStreamInClient, mStreamInClient->mSourceType,
          mStreamInClient->mMemDataType);
    return NO_ERROR;
}

void AudioMTKStreamIn::StreamInSRC_Init(void)
{
#if 0
    if (mStreamInClient->mMemDataType == AudioDigitalType::MEM_DAI) // cases to do UL SRC
    {
        if (mBliSrc)
        {
            if (mBliSrc->initStatus() != OK)
            {
                // 6628 only support 8k BTSCO
                const uint8_t num_channel = (mAttribute.mChannels == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
                ALOGD("StreamInSRC_Init BLI_SRC, %d to mAttribute.mSampleRate=%d, num_channel=%d", DAIBT_SAMPLE_RATE, mAttribute.mSampleRate, num_channel);
                mBliSrc->init(DAIBT_SAMPLE_RATE, 1, mAttribute.mSampleRate, num_channel);
            }
        }
        else
        {
            ALOGW("StreamInSRC_Init() mBliSrc=NULL!!!");
        }
    }
#endif
}

uint32 AudioMTKStreamIn::GetSrcbufvalidSize(RingBuf *SrcInputBuf)
{

    //ALOGD("GetSrcbufvalidSize SrcInputBuf->pWrite=%x,SrcInputBuf->pRead=%x, SrcInputBuf->bufLen=%d ",SrcInputBuf->pWrite,SrcInputBuf->pRead,SrcInputBuf->bufLen);
    if (SrcInputBuf != NULL)
    {
        if (SrcInputBuf->pWrite >= SrcInputBuf->pRead)
        {
            return SrcInputBuf->pWrite - SrcInputBuf->pRead;
        }
        else
        {
            return SrcInputBuf->pRead + SrcInputBuf->bufLen - SrcInputBuf->pWrite;
        }
    }
    ALOGW("SrcInputBuf == NULL");
    return 0;
}

uint32 AudioMTKStreamIn::GetSrcbufFreeSize(RingBuf *SrcInputBuf)
{
    if (SrcInputBuf != NULL)
    {
        return SrcInputBuf->bufLen - GetSrcbufvalidSize(SrcInputBuf);
    }
    ALOGW("SrcInputBuf == NULL");
    return 0;
}

// here copy SRCbuf to input buffer , and return how many buffer copied
uint32 AudioMTKStreamIn::CopySrcBuf(char *buffer, uint32 *bytes, RingBuf *SrcInputBuf, uint32 *length)
{
    uint32 consume = 0;
    uint32 outputbyes = *bytes;
    //ALOGD("+CopySrcBuf consume = %d bytes = %d length = %d",consume,*bytes,*length);
    consume = mBliSrc->process((short *)SrcInputBuf->pRead, length, (short *)buffer, bytes);
    //ALOGD("-CopySrcBuf consume = %d bytes = %d length = %d",consume,*bytes,*length);
    SrcInputBuf->pRead += consume;
    if (SrcInputBuf->pRead >= (SrcInputBuf->pBufBase + SrcInputBuf->bufLen))
    {
        //ALOGD("SrcInputBuf->pRead = %d, SrcInputBuf->pBufBase=%d,SrcInputBuf->bufLen = %d",SrcInputBuf->pRead,SrcInputBuf->pBufBase, SrcInputBuf->bufLen);
        SrcInputBuf->pRead -= SrcInputBuf->bufLen;
    }
    *bytes = outputbyes - *bytes;
    return consume;
}

status_t AudioMTKStreamIn::RequesetRecordclock()
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    return NO_ERROR;
}
status_t AudioMTKStreamIn::ReleaseRecordclock()
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
    return NO_ERROR;
}

bool AudioMTKStreamIn::IsNeedDMICSRC(void)
{
    if ((mStreamInClient->mMemDataType == AudioDigitalType::MEM_VUL) && (mAttribute.mIsDigitalMIC == true) && (mStreamInClient->mSourceType == AudioDigitalType::I2S_IN_ADC)) // for digital MIC
    {
        return true;
    }
    return false;
}

bool AudioMTKStreamIn::StreamIn_NeedToSRC(void)
{
#if 1
    return false;
#else
    if (mStreamInClient->mMemDataType == AudioDigitalType::MEM_DAI && mAttribute.mSampleRate != DAIBT_SAMPLE_RATE)  // cases to Do UL SRC
    {
        return true;
    }
    else
    {
        return false;
    }
#endif
}

void AudioMTKStreamIn::StreamInSRC_Process(void *buffer, uint32_t bytes)
{
    uint32_t CopyBufferwriteIdx = 0;
    RingBuf *SrcInputBuf = &mStreamInClient->mRingBuf;
    uint32_t readbytes = bytes;

    if (mBliSrc->initStatus() == OK)
    {
        //ALOGD("read(), StreamInSRC_Process");
        do
        {
            //here need to do SRC and copy to buffer,check if there is any buf in src , exhaust it.
            if (GetSrcbufvalidSize(SrcInputBuf))
            {
                //ALOGD("read(), GetSrcbufvalidSize OK");
                uint32 bufLen = 0;
                uint32 ConsumeReadbytes = 0;
                if (SrcInputBuf->pWrite >= SrcInputBuf->pRead)
                {
                    bufLen = GetSrcbufvalidSize(SrcInputBuf);
                    ALOGV("SrcInputBuf->pWrite = %d SrcInputBuf->pRead = %d SrcInputBuf->bufLen = %d readbytes = %d CopyBufferwriteIdx = %d",
                          SrcInputBuf->pWrite, SrcInputBuf->pRead, SrcInputBuf->bufLen, readbytes, CopyBufferwriteIdx);
                    ConsumeReadbytes = readbytes;
                    CopySrcBuf((char *)buffer + CopyBufferwriteIdx, &readbytes, SrcInputBuf, &bufLen);
                    CopyBufferwriteIdx += ConsumeReadbytes - readbytes;
                    if (readbytes == 0)
                    {
                        break;
                    }
                }
                else
                {
                    bufLen = SrcInputBuf->pBufBase + SrcInputBuf->bufLen - SrcInputBuf->pRead;
                    ALOGV("SrcInputBuf->pWrite = %d SrcInputBuf->pRead = %d SrcInputBuf->bufLen = %d readbytes = %d CopyBufferwriteIdx = %d",
                          SrcInputBuf->pWrite, SrcInputBuf->pRead, SrcInputBuf->bufLen, readbytes, CopyBufferwriteIdx);
                    ConsumeReadbytes = readbytes;
                    CopySrcBuf((char *)buffer + CopyBufferwriteIdx, &readbytes, SrcInputBuf, &bufLen);
                    CopyBufferwriteIdx += ConsumeReadbytes - readbytes;
                    if (readbytes == 0)
                    {
                        break;
                    }
                    else
                    {
                        bufLen = SrcInputBuf->pWrite - SrcInputBuf->pRead;
                        ALOGV("SrcInputBuf->pWrite = %d SrcInputBuf->pRead = %d SrcInputBuf->bufLen = %d readbytes = %d CopyBufferwriteIdx = %d",
                              SrcInputBuf->pWrite, SrcInputBuf->pRead, SrcInputBuf->bufLen, readbytes, CopyBufferwriteIdx);
                        ConsumeReadbytes = readbytes;
                        CopySrcBuf((char *)buffer + CopyBufferwriteIdx, &readbytes, SrcInputBuf, &bufLen);
                        CopyBufferwriteIdx += ConsumeReadbytes - readbytes;
                        if (readbytes == 0)
                        {
                            break;
                        }
                    }
                }
            }
            usleep(10 * 1000); //let MTKRecordThread to copy data
        }
        while (readbytes);
        //ALOGD("read(), StreamInSRC_Process, finish while loop~");
    }
}

ssize_t AudioMTKStreamIn::read(void *buffer, ssize_t bytes)
{
    ssize_t ReadSize = 0;
    int ret = 0;
    uint32 RingBufferSize = 0, ReadDataBytes = bytes;
    int TryCount = 20;

    uint32 GetDataBytes = bytes;
    int RefillTryCount = 20;

#ifndef EXTMD_LOOPBACK_TEST
    if ((mReadCount % 10) == 0)
    {
        ALOGD("AudioMTKStreamIn::read buffer = %p bytes = %d  this = %p", buffer, bytes, this);
    }
    mReadCount++;
#ifdef SPEECH_PCM_VM_SUPPORT
    if (mSuspend)
#else
    if (mSuspend || SpeechVMRecorder::GetInstance()->GetVMRecordStatus() == true)
#endif
    {
        // here to sleep a buffer size latency and return.
        ALOGD("read suspend = %d", mSuspend);
        memset(buffer, 0, bytes);
        usleep(30 * 1000);
        return bytes;
    }
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    if (ret)
    {
        ALOGW("read EnableAudioLock AUDIO_HARDWARE_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }

    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    if (ret)
    {
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
        ALOGW("read EnableAudioLock AUDIO_STREAMINMANAGERCLIENT_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }

    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    if (ret)
    {
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
        ALOGW("read EnableAudioLock AUDIO_STREAMINMANAGER_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }


#ifndef SPEECH_PCM_VM_SUPPORT
    if (SpeechVMRecorder::GetInstance()->GetVMRecordStatus() == true)
    {
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);

        memset(buffer, 0, bytes);
        usleep(30 * 1000);
        return bytes;
    }
#endif

    Mutex::Autolock _l(mLock);

    if (mStarting == false)
    {
        mStarting = true;
        RequesetRecordclock();
        ALOGD("read mStarting == false , first start");

        char Buf[10];
        sprintf(Buf, "%d.pcm", DumpFileNum);

        DumpFileName = String8(streamin);
        DumpFileName.append((const char *)Buf);
        mPAdcPCMDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);
#if 0
        DumpFileName = String8(streaminOri);
        DumpFileName.append((const char *)Buf);
        mPAdcPCMInDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);
#endif
        DumpFileNum++;
        DumpFileNum %= MAX_DUMP_NUM;

        // start to open record
        switch (mAudioResourceManager->GetAudioMode())
        {
            case AUDIO_MODE_NORMAL:
            case AUDIO_MODE_RINGTONE:
            case AUDIO_MODE_IN_COMMUNICATION:
            {
                mStreamInClient = mStreamInManager->RequestClient();
                if (mStreamInClient == NULL)
                {
                    ALOGE("read mStarting = false cannot get mStreamInClient");
                }
                mStreamInClient->mAttributeClient = &mAttribute; // set attribute with request client
                SetClientSourceandMemType(mStreamInClient); // make sure mStreamInClient mem, set to digital CONTROL_IFACE_PATH

                mStreamInClient->mStreamIn = this;  //record this MTK streamin
                SetStreamInPreprocessStatus(mEnablePreprocess); //set current mEnablePreprocess status

                StreamInSRC_Init();

#ifdef MTK_DUAL_MIC_SUPPORT
                mLRChannelSwitch = mAudioSpeechEnhanceInfoInstance->GetRecordLRChannelSwitch();
                mSpecificMicChoose = mAudioSpeechEnhanceInfoInstance->GetUseSpecificMIC();
#endif

                ALOGD("mStreamInManager Do_input_start");
                mStreamInManager->Do_input_start(mStreamInClient);

                mpSPELayer->SetUPLinkDropTime(mStreamInManager->GetRecordDropTime());
                //mpSPELayer->GetUPlinkIntrStartTime();
                mpSPELayer->SetUPLinkIntrStartTime(mStreamInClient->mInputStartTime);

#ifdef MTK_AUDIO_HD_REC_SUPPORT
                CheckHDRecordVoIPSwitch();

                if ((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) || mUsingMagiASR || mUsingAECRec || mVoIPRunning || mHDRecTunning16K)
                {
                    // set blisrc 48k->16k and 16k->48k
                    uint16_t num_sample_rate = mAttribute.mSampleRate;  //48k

                    const uint8_t num_channel = (mAttribute.mChannels == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
                    uint16_t process_sample_rate = HD_RECORD_SAMPLE_RATE;
                    if ((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) || mUsingMagiASR || mUsingAECRec || mVoIPRunning || mHDRecTunning16K)
                    {
                        process_sample_rate = VOICE_RECOGNITION_RECORD_SAMPLE_RATE;
                    }

                    ALOGD("need src? ori_sample_rate=%d, process_sample_rate=%d", num_sample_rate, process_sample_rate);
                    if (process_sample_rate != num_sample_rate)
                    {
                        uint32_t srcBufLen = 0;
                        // Need SRC 48k->16k
                        BLI_GetMemSize(num_sample_rate, num_channel,
                                       process_sample_rate, num_channel,
                                       &srcBufLen);
                        mBliHandler1 =
                            BLI_Open(num_sample_rate, num_channel,
                                     process_sample_rate, num_channel,
                                     new char[srcBufLen], NULL);
                        mBliHandler1Buffer = new char[mAttribute.mBufferSize]; // tmp buffer for blisrc out, need check if buffer size or sample rate change
                        ASSERT(mBliHandler1Buffer != NULL);

                        ALOGD("srcBufLen=%d,mAttribute.mBufferSize=%d", srcBufLen, mAttribute.mBufferSize);
                        // Need SRC 16k->48k
                        srcBufLen = 0;
                        BLI_GetMemSize(process_sample_rate, num_channel,
                                       num_sample_rate, num_channel,
                                       &srcBufLen);
                        mBliHandler2 =
                            BLI_Open(process_sample_rate, num_channel,
                                     num_sample_rate, num_channel,
                                     new char[srcBufLen], NULL);
                        mBliHandler2Buffer = new char[mAttribute.mBufferSize]; // tmp buffer for blisrc out
                        ASSERT(mBliHandler2Buffer != NULL);
                        //ALOGD("mBliHandler2 srcBufLen=%d,mAttribute.mBufferSize=%d",srcBufLen,mAttribute.mBufferSize);
                        //ALOGD("mBliHandler1=%p,mBliHandler1Buffer=%p, mBliHandler2=%p,mBliHandler2Buffer=%p",mBliHandler1,mBliHandler1Buffer,mBliHandler2,mBliHandler2Buffer);
                    }
                }
#endif

                AudioVUnlockDL *VUnlockhdl = AudioVUnlockDL::getInstance();
                if (VUnlockhdl != NULL)
                {
                    VUnlockhdl->SetUplinkStartTime(mStreamInClient->mInputStartTime);
                }

                break;
            }
            case AUDIO_MODE_IN_CALL:
            case AUDIO_MODE_IN_CALL_2:
            case AUDIO_MODE_IN_CALL_EXTERNAL:
            {
                // request client for ring buffer
                mStreamInClient = mStreamInManager->RequestClient();
                mStreamInClient->mAttributeClient = &mAttribute; // set attribute with request client

                // set blisrc
                SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
                const uint16_t modem_num_sample_rate = pSpeechDriver->GetRecordSampleRate();
                const uint16_t modem_num_channel     = pSpeechDriver->GetRecordChannelNumber();

                const uint16_t target_num_sample_rate = mAttribute.mSampleRate;
                const uint16_t target_num_channel = (mAttribute.mChannels == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;

                if (modem_num_sample_rate != target_num_sample_rate ||
                    modem_num_channel     != target_num_channel) // Need SRC
                {
                    uint32_t srcBufLen = 0;
                    BLI_GetMemSize(modem_num_sample_rate, modem_num_channel,
                                   target_num_sample_rate, target_num_channel,
                                   &srcBufLen);
                    mStreamInClient->mBliHandlerBuffer =
                        BLI_Open(modem_num_sample_rate, modem_num_channel,
                                 target_num_sample_rate, target_num_channel,
                                 new char[srcBufLen], NULL);
                }

                // open modem record path
                mStreamInManager->Do_input_start(mStreamInClient);

                // No need extra AP side speech enhancement
                SetStreamInPreprocessStatus(false);
                break;
            }
            default:
                ALOGW("no mode is select!!");
                break;
        }

        ALOGD("read mStarting == false , first done");
    }

    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);

    ASSERT(mStreamInClient != NULL);

    //ALOGD("mStreamInClient->mMemDataType=%d,mAttribute.mSampleRate=%d,mBliSrc->initStatus()=%d",mStreamInClient->mMemDataType,mAttribute.mSampleRate,mBliSrc->initStatus());
    char *pWrite = (char *)buffer;
    char *pStart = (char *)buffer;
    GetDataBytes = ReadDataBytes;

    uint32_t ProcessedDataBytes;
    bool isBusy = false;
    do
    {
        if (StreamIn_NeedToSRC() == true)
        {
            StreamInSRC_Process(pStart, ReadDataBytes);
        }
        else
        {
            //ALOGD("read(), No SRC path");
            //char *pWrite = (char *)buffer;

            do
            {
                mStreamInClient->mLock.lock();
                RingBufferSize = RingBuf_getDataCount(&mStreamInClient->mRingBuf);
                //ALOGD("read RingBufferSize = %d TryCount = %d ReadDataBytes = %d mRingBuf = %p", RingBufferSize, TryCount, ReadDataBytes, &mStreamInClient->mRingBuf);
                if (RingBufferSize >= ReadDataBytes) // ring buffer is enough, copy & exit
                {
                    RingBuf_copyToLinear((char *)pWrite, &mStreamInClient->mRingBuf, ReadDataBytes);
                    mStreamInClient->mLock.unlock();
                    break;
                }
                else // ring buffer is not enough, copy all data & wait
                {
                    RingBuf_copyToLinear((char *)pWrite, &mStreamInClient->mRingBuf, RingBufferSize);
                    ReadDataBytes -= RingBufferSize;
                    pWrite += RingBufferSize;
                }
                //ALOGD("+read wait for mStreamInClient->mLock mStreamInClient = %p", mStreamInClient);
                if (mStreamInClient->mWaitWorkCV.waitRelative(mStreamInClient->mLock, milliseconds(300)) != NO_ERROR)
                {
                    ALOGW("waitRelative fail");
                    if (mStreamInClient != NULL) // busy
                    {
                        mStreamInClient->mLock.unlock();
                        isBusy = true;
                        break;
                    }
                    else // die
                    {
                        return bytes;
                    }
                }
                //ALOGD("-read wait for mStreamInClient->mLock");
                mStreamInClient->mLock.unlock();
                TryCount--;
            }
            while (ReadDataBytes && TryCount);
        }

#if 0
        if (mPAdcPCMInDumpFile)
        {
            long int position = 0;
            position = ftell(mPAdcPCMInDumpFile);
            if (position > MAX_FILE_LENGTH)
            {
                rewind(mPAdcPCMInDumpFile);
            }
            AudioDumpPCMData((void *)pStart , GetDataBytes, mPAdcPCMInDumpFile);
        }
#endif

        ProcessedDataBytes = StreamInPreprocess(pStart, GetDataBytes);

        pStart += ProcessedDataBytes;

        ReadDataBytes = GetDataBytes - ProcessedDataBytes;
        GetDataBytes = ReadDataBytes;

        //ALOGD("bytes=%d, ProcessedDataBytes=%d,GetDataBytes=%d,RefillTryCount=%d,pStart=%p,pWrite=%p",bytes,ProcessedDataBytes,GetDataBytes,RefillTryCount,pStart,pWrite);
        pWrite = pStart;

        RefillTryCount--;

        if (isBusy)
        {
            ALOGW("waitRelative fail busy break");
            break;
        }
    }
    while (GetDataBytes && RefillTryCount);

    if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))
    {
        CheckDmicNeedLRSwitch((short *)buffer, bytes);    //for DMIC MTKIF cannot swap by I2S
    }


    if (mAudioResourceManager->IsModeIncall() == false)
    {
        CheckNeedDataConvert((short *)buffer, bytes);
    }

    if (mPAdcPCMDumpFile)
    {
        long int position = 0;
        position = ftell(mPAdcPCMDumpFile);
        if (position > MAX_FILE_LENGTH)
        {
            rewind(mPAdcPCMDumpFile);
        }
        AudioDumpPCMData((void *)buffer , bytes, mPAdcPCMDumpFile);
    }        
#endif  //!EXTMD_LOOPBACK_TEST


    return bytes;
}

status_t AudioMTKStreamIn::standby()
{
    //here to stanby input stream
    ALOGD("audioMTKStreamIn::standby()");
#ifndef EXTMD_LOOPBACK_TEST
    SetSuspend(true);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    mLock.lock();
    standbyWithMode();
    AudioCloseDumpPCMFile(mPAdcPCMDumpFile);
    //    AudioCloseDumpPCMFile(mPAdcPCMInDumpFile);
    mLock.unlock();
    SetSuspend(false);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
    ALOGD("audioMTKStreamIn::standby()---");
#endif
    return NO_ERROR;
}

status_t AudioMTKStreamIn::standbyWithMode()
{
    if (mStarting == true)
    {
        mStarting = false;
        switch (mAudioResourceManager->GetAudioMode())
        {
            case AUDIO_MODE_NORMAL:
            case AUDIO_MODE_RINGTONE:
            case AUDIO_MODE_IN_COMMUNICATION:
            {
                mStreamInManager->Do_input_standby(mStreamInClient);
                mStreamInManager->ReleaseClient(mStreamInClient);
                mStreamInClient = NULL;//assign null value;
#ifdef MTK_AUDIO_HD_REC_SUPPORT
                StopHDRecord();
                if (mpSPELayer != NULL && mEnablePreprocess == true)
                {
                    mpSPELayer->Standby();
                }
#endif
                if (mBliHandler1)
                {
                    BLI_Close(mBliHandler1, NULL);
                    delete[] mBliHandler1;
                    mBliHandler1 = NULL;
                }
                if (mBliHandler1Buffer)
                {
                    delete[] mBliHandler1Buffer;
                    mBliHandler1Buffer = NULL;
                }
                if (mBliHandler2)
                {
                    BLI_Close(mBliHandler2, NULL);
                    delete[] mBliHandler2;
                    mBliHandler2 = NULL;
                }
                if (mBliHandler2Buffer)
                {
                    delete[] mBliHandler2Buffer;
                    mBliHandler2Buffer = NULL;
                }
                if (mBliSrc)
                {
                    mBliSrc->close();
                }
                break;
            }
            case AUDIO_MODE_IN_CALL:
            case AUDIO_MODE_IN_CALL_2:
            case AUDIO_MODE_IN_CALL_EXTERNAL:
            {
                mStreamInManager->Do_input_standby(mStreamInClient);
                if (mStreamInClient->mBliHandlerBuffer != NULL)
                {
                    BLI_Close(mStreamInClient->mBliHandlerBuffer, NULL);
                    delete[] mStreamInClient->mBliHandlerBuffer;
                }
                mStreamInManager->ReleaseClient(mStreamInClient);
                mStreamInClient = NULL;//assign null value;
                break;
            }
            default:
            {
                break;
            }
        }
        ReleaseRecordclock();
    }
    else
    {
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        if (mpSPELayer != NULL)
        {
            mpSPELayer->Standby();
        }
#endif
    }
    return NO_ERROR;
}

status_t  AudioMTKStreamIn::setParameters(const String8 &keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key_routing = String8(AudioParameter::keyRouting);
    String8 key_input_source = String8(AudioParameter::keyInputSource);

    status_t status = NO_ERROR;
    int device;
    int input_source;
    bool InputSourceChange = false;
    ALOGD("+setParameters() %s", keyValuePairs.string());
    mLock.lock();

    if (param.getInt(key_input_source, input_source) == NO_ERROR)
    {
        ALOGD("setParameters, input_source(%d)", input_source);
        if (mForceMagiASREnable)
        {
            ALOGD("setParameters, force input source to AUDIO_SOURCE_CUSTOMIZATION1");
            input_source = AUDIO_SOURCE_CUSTOMIZATION1;

        }
        else if (mForceAECRecEnable)
        {
            ALOGD("setParameters, force input source to AUDIO_SOURCE_CUSTOMIZATION2");
            input_source = AUDIO_SOURCE_CUSTOMIZATION2;
        }

        if (mAttribute.mSource != input_source)
        {
            mAttribute.mSource = input_source;
            InputSourceChange = true;
        }
        switch (input_source)
        {
            case AUDIO_SOURCE_DEFAULT:
                input_source = AUDIO_SOURCE_MIC; // set default as mic source.
            case AUDIO_SOURCE_MIC:
            case AUDIO_SOURCE_CAMCORDER:
            case AUDIO_SOURCE_VOICE_UPLINK:
            case AUDIO_SOURCE_VOICE_DOWNLINK:
            case AUDIO_SOURCE_VOICE_CALL:
            case AUDIO_SOURCE_VOICE_COMMUNICATION:
                SetStreamInPreprocessStatus(true);
                break;
            case AUDIO_SOURCE_VOICE_RECOGNITION:
                // when recording use voice recognition , not to do pre processing
                //SetStreamInPreprocessStatus(false);
                SetStreamInPreprocessStatus(true);
                break;
            case AUDIO_SOURCE_VOICE_UNLOCK:
                // when voice unlock , not to do pre processing
                SetStreamInPreprocessStatus(false);
                break;
            case AUDIO_SOURCE_CUSTOMIZATION1:
                SetStreamInPreprocessStatus(true);
                break;
            case AUDIO_SOURCE_CUSTOMIZATION2:
            case AUDIO_SOURCE_CUSTOMIZATION3:
                SetStreamInPreprocessStatus(true);
                break;
            case AUDIO_SOURCE_MATV:
            case AUDIO_SOURCE_FM:
                SetStreamInPreprocessStatus(false);
                if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))       //for digital MIC sample rate constrain
                {
                    mAttribute.mIsDigitalMIC = false;
                }

                break;
            default:
                SetStreamInPreprocessStatus(false);
                break;
        }

        //MagiASR case
        if (input_source == AUDIO_SOURCE_CUSTOMIZATION1)
        {
            mUsingMagiASR = true;
        }
        else
        {
            mUsingMagiASR = false;
        }

        //Rec with AEC case
        if (input_source == AUDIO_SOURCE_CUSTOMIZATION2)
        {
            mUsingAECRec = true;
        }
        else
        {
            mUsingAECRec = false;
        }

        mAudioResourceManager->setUlInputSource(input_source);
        param.remove(key_input_source);
    }

    if (param.getInt(key_routing, device) == NO_ERROR)
    {
        if ((device !=  mAttribute.mdevices) && device)
        {
            if (mStarting == true)
            {
                if (CheckMemTypeChange(mAttribute.mdevices, device))
                {
                    standbyWithMode(); // standby this stream ,wait for read to reopen hardware again
                }

                mAudioResourceManager->SelectInputDevice(device);
            }
            else
            {
                mAudioResourceManager->setUlInputDevice(device);
            }

            // update device for streamclient
            mAttribute.mPredevices = mAttribute.mdevices;
            mAttribute.mdevices = device;
            //SetClientSourceandMemType(mStreamInClient);
#ifdef MTK_AUDIO_HD_REC_SUPPORT
            //restart the preprocess to apply new MIC gain if device change
            CheckHDRecordVoIPSwitch();
#endif
        }
        else
        {
            mAudioResourceManager->setUlInputDevice(device); // Set Input device in AudioResource Manager
            ALOGD("set parameters with same input device", device);
        }
        ALOGD("setParameters, device(0x%x)", device);
        param.remove(key_routing);
    }

    if (param.size())
    {
        status = BAD_VALUE;
    }
    mLock.unlock();
    ALOGD("-setParameters()");
    return status;
}

bool  AudioMTKStreamIn::CheckMemTypeChange(uint32_t oldDevice, uint32_t newDevice)
{
    uint32_t OldMemType = 0, NewMemType = 0;
    OldMemType = GetMemTypeByDevice(oldDevice);
    NewMemType = GetMemTypeByDevice(newDevice);
    ALOGD("oldDevice = 0x%x newDevice = 0x%x ", oldDevice, newDevice);
    return (OldMemType != NewMemType);
}

uint32_t  AudioMTKStreamIn::GetMemTypeByDevice(uint32_t Device)
{
    switch (Device)
    {
        case AUDIO_DEVICE_IN_AMBIENT :
        case AUDIO_DEVICE_IN_BUILTIN_MIC :
        case AUDIO_DEVICE_IN_WIRED_HEADSET :
        case AUDIO_DEVICE_IN_BACK_MIC :
        case AUDIO_DEVICE_IN_COMMUNICATION :
        case AUDIO_DEVICE_IN_VOICE_CALL:
            return AudioDigitalType::MEM_VUL;
            // for BT scenario , it's a special case need to modify source to DAI_BT
        case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET :
            return AudioDigitalType::MEM_DAI;
        case AUDIO_DEVICE_IN_FM :
        case AUDIO_DEVICE_IN_MATV:
            return AudioDigitalType::MEM_AWB;
        default:
            ALOGW("no proper device match !!!");
            return AudioDigitalType::MEM_VUL;
    }
}

String8 AudioMTKStreamIn::getParameters(const String8 &keys)
{
    ALOGD("AudioMTKHardware getParameters\n");
    AudioParameter param = AudioParameter(keys);
    return param.toString();
}

unsigned int AudioMTKStreamIn::getInputFramesLost() const
{
    //here need to implement frame lost , now not support
    return 0;
}

size_t  AudioMTKStreamIn::GetfixBufferSize(uint32_t sampleRate, int format, int channelCount)
{
    //for 8192 bytes ==> 48K , stereo , channel;
    size_t FixBufferSize = 0;
    FixBufferSize = Default_Mic_Buffer ;
    ALOGD("GetfixBufferSize FixBufferSize = %d", FixBufferSize);
    return FixBufferSize;
}

bool AudioMTKStreamIn::MutexLock(void)
{
    mLock.lock();
    return true;
}
bool AudioMTKStreamIn::MutexUnlock(void)
{
    mLock.unlock();
    return true;
}

status_t AudioMTKStreamIn::addAudioEffect(effect_handle_t effect)
{
    int status;
    effect_descriptor_t desc;
    const uint8_t num_channel = (channels() == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    MutexLock();
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
    ALOGD("addAudioEffect+++ effect %p", effect);
    if (mVoIPRunning) //not use android native preprocess if using BesVoIP
    {
        status = -ENOSYS;
        ALOGD("addAudioEffect bypass due to enable MTK VoIP");
        goto exit;
    }

    if (mAPPS->num_preprocessors >= MAX_PREPROCESSORS)
    {
        status = -ENOSYS;
        goto exit;
    }

    status = (*effect)->get_descriptor(effect, &desc);
    if (status != 0)
    {
        goto exit;
    }

    mAPPS->preprocessors[mAPPS->num_preprocessors].effect_itfe = effect;
    /* add the supported channel of the effect in the channel_configs */
    //in_read_audio_effect_channel_configs(&preprocessors[num_preprocessors]);

    mAPPS->num_preprocessors++;

    /* check compatibility between main channel supported and possible auxiliary channels */
    //in_update_aux_channels(effect);

    ALOGD("addAudioEffect, effect type: %08x, effect name:%s", desc.type.timeLow, desc.name);
    //echo reference
    if (memcmp(&desc.type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0)
    {
        mAPPS->need_echo_reference = true;
        ALOGD("find AEC: %x", mStarting);
        if (mStarting)
        {
            if (mEcho_Reference != NULL)
            {
                MutexUnlock();
                standby();
                mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, 3000);
                MutexLock();
                /* stop reading from echo reference */
                mAPPS->stop_echo_reference(mEcho_Reference);
                ALOGD("stop_echo_reference done");
                mEcho_Reference = NULL;
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
            }
            else
            {
                ALOGD("open AEC even record is already start");
                mEcho_Reference = mAPPS->start_echo_reference(
                                      AUDIO_FORMAT_PCM_16_BIT,
                                      num_channel,
                                      sampleRate());
            }
        }
        mAPPS->in_configure_reverse(num_channel, sampleRate());
    }

exit:
    MutexUnlock();
    ALOGD("addAudioEffect--- error %d", status);
    return status;
}

status_t AudioMTKStreamIn::removeAudioEffect(effect_handle_t effect)
{
    int i;
    int status = -EINVAL;
    effect_descriptor_t desc;
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    MutexLock();
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
    ALOGD("removeAudioEffect+++ effect %p, num_preprocessors=%d", effect, mAPPS->num_preprocessors);
    if (mAPPS->num_preprocessors <= 0)
    {
        status = -ENOSYS;
        goto exit;
    }

    for (i = 0; i < mAPPS->num_preprocessors; i++)
    {
        if (status == 0)   /* status == 0 means an effect was removed from a previous slot */
        {
            mAPPS->preprocessors[i - 1].effect_itfe = mAPPS->preprocessors[i].effect_itfe;
            mAPPS->preprocessors[i - 1].channel_configs = mAPPS->preprocessors[i].channel_configs;
            mAPPS->preprocessors[i - 1].num_channel_configs = mAPPS->preprocessors[i].num_channel_configs;
            ALOGD("in_remove_audio_effect moving fx from %d to %d", i, i - 1);
            continue;
        }
        if (mAPPS->preprocessors[i].effect_itfe == effect)
        {
            ALOGD("in_remove_audio_effect found fx at index %d", i);
            //            free(preprocessors[i].channel_configs);
            status = 0;
        }
    }

    if (status != 0)
    {
        goto exit;
    }

    mAPPS->num_preprocessors--;
    /* if we remove one effect, at least the last preproc should be reset */
    mAPPS->preprocessors[mAPPS->num_preprocessors].num_channel_configs = 0;
    mAPPS->preprocessors[mAPPS->num_preprocessors].effect_itfe = NULL;
    mAPPS->preprocessors[mAPPS->num_preprocessors].channel_configs = NULL;


    /* check compatibility between main channel supported and possible auxiliary channels */
    //in_update_aux_channels(in, NULL);

    status = (*effect)->get_descriptor(effect, &desc);
    if (status != 0)
    {
        goto exit;
    }

    ALOGD("in_remove_audio_effect(), effect type: %08x, effect name:%s", desc.type.timeLow, desc.name);

    //echo reference
    if (memcmp(&desc.type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0)
    {
        mAPPS->need_echo_reference = false;
        if (mStarting)
        {
            if (mEcho_Reference != NULL)
            {
                MutexUnlock();
                standby();
                mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
                MutexLock();
                /* stop reading from echo reference */
                mAPPS->stop_echo_reference(mEcho_Reference);
                ALOGD("stop_echo_reference done");
                mEcho_Reference = NULL;
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
            }
        }
    }

exit:

    MutexUnlock();
    ALOGD("removeAudioEffect--- error %d", status);
    return status;
}

status_t AudioMTKStreamIn::SetSuspend(bool suspend)
{
    if (suspend)
    {
        mSuspend++;
    }
    else
    {
        mSuspend--;
        if (mSuspend < 0)
        {
            ALOGW("mSuspend = %d", mSuspend);
            mSuspend = 0;
        }
    }
    return NO_ERROR;
}

AudioMTKStreamIn::BliSrc::BliSrc()
    : mHandle(NULL), mBuffer(NULL), mInitCheck(NO_INIT)
{
}

AudioMTKStreamIn::BliSrc::~BliSrc()
{
    close();
}

status_t AudioMTKStreamIn::BliSrc::initStatus()
{
    return mInitCheck;
}

status_t  AudioMTKStreamIn::BliSrc::init(uint32 inSamplerate, uint32 inChannel, uint32 OutSamplerate, uint32 OutChannel)
{
    if (mHandle == NULL)
    {
        uint32_t workBufSize;
        BLI_GetMemSize(inSamplerate, inChannel, OutSamplerate, OutChannel, &workBufSize);
        ALOGD("BliSrc::init InputSampleRate=%u, inChannel=%u, OutputSampleRate=%u, OutChannel=%u, mWorkBufSize = %u",
              inSamplerate, inChannel, OutSamplerate, OutChannel, workBufSize);
        mBuffer = new uint8_t[workBufSize];
        if (!mBuffer)
        {
            ALOGE("BliSrc::init Fail to create work buffer");
            return NO_MEMORY;
        }
        memset((void *)mBuffer, 0, workBufSize);
        mHandle = BLI_Open(inSamplerate, inChannel, OutSamplerate, OutChannel, (char *)mBuffer, NULL);
        if (!mHandle)
        {
            ALOGE("BliSrc::init Fail to get blisrc handle");
            if (mBuffer)
            {
                delete []mBuffer;
                mBuffer = NULL;
            }
            return NO_INIT;
        }
        mInitCheck = OK;
    }
    return NO_ERROR;

}

size_t AudioMTKStreamIn::BliSrc::process(const void *inbuffer, size_t *inBytes, void *outbuffer, size_t *outBytes)
{
    if (mHandle)
    {
        size_t consume = BLI_Convert(mHandle, (short *)inbuffer, inBytes, (short *)outbuffer, outBytes);
        ALOGD_IF(consume != *inBytes, "inputLength=%d,consume=%d,outputLength=%d", *inBytes, consume, *outBytes);
        return consume;
    }
    ALOGW("BliSrc::process src not initialized");
    return 0;
}

status_t  AudioMTKStreamIn::BliSrc::close(void)
{
    if (mHandle)
    {
        BLI_Close(mHandle, NULL);
        mHandle = NULL;
    }
    if (mBuffer)
    {
        delete []mBuffer;
        mBuffer = NULL;
    }
    mInitCheck = NO_INIT;
    return NO_ERROR;
}



void AudioMTKStreamIn::CheckNeedDataConvert(short *buffer, ssize_t bytes)
{
    //specific MIC choose or LR channel Switch
    short left , right;
    int copysize = bytes >> 2;  //stereo, 16bits
    int copysizebackup = copysize;
    short *pBufferAddr = buffer;

#ifdef MTK_DUAL_MIC_SUPPORT
    char value[PROPERTY_VALUE_MAX];
    property_get("streamin.micchoose", value, "0");
    int bflag = atoi(value);

    char value1[PROPERTY_VALUE_MAX];
    property_get("streamin.LRSwitch", value1, "0");
    int bflag1 = atoi(value1);

    //    ALOGD("mbLRChannelSwitch=%d, miSpecificMicChoose=%d,bflag %d, bflag1 %d",mLRChannelSwitch,mSpecificMicChoose,bflag,bflag1);

    if (mSpecificMicChoose == 1 || bflag == 1) //use main MIC
    {
        while (copysize)
        {
            left = *(buffer);
            *(buffer) = left;
            *(buffer + 1) = left;
            buffer += 2;
            copysize--;
        }
        buffer = pBufferAddr;
        copysize = copysizebackup;
    }
    else if (mSpecificMicChoose == 2 || bflag == 2) //use ref MIC
    {
        while (copysize)
        {
            right = *(buffer + 1);
            *(buffer) = right;
            *(buffer + 1) = right;
            buffer += 2;
            copysize--;
        }
        buffer = pBufferAddr;
        copysize = copysizebackup;
    }
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    if ((bflag1 || mLRChannelSwitch) && (mSpecificMicChoose == 0 && bflag != 1 && bflag != 2) && mStereoMode) //channel switch
#else
    if ((bflag1 || mLRChannelSwitch) && (mSpecificMicChoose == 0 && bflag != 1 && bflag != 2)) //channel switch
#endif
    {
        while (copysize)
        {
            left = *(buffer);
            right = *(buffer + 1);
            *(buffer) = right;
            *(buffer + 1) = left;
            buffer += 2;
            copysize--;
        }
        buffer = pBufferAddr;
        copysize = copysizebackup;
    }
#endif

    //Stereo/mono convert
    const uint8_t num_channel = (mAttribute.mChannels == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
    //    ALOGD("num_channel=%d, mStereoMode=%d",num_channel,mStereoMode);
#ifndef AUDIO_HQA_SUPPORT
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    //need return stereo data
    if (num_channel == 2)
    {
        if (!mStereoMode) //speech enhancement output data is mono, need to convert to stereo
        {
            short left;
            int copysize = bytes >> 2;

            while (copysize)    //only left channel data is processed
            {
                left = *(buffer);
                *(buffer) = left;
                *(buffer + 1) = left;
                buffer += 2;
                copysize--;
            }
            buffer = pBufferAddr;
        }
    }
#endif
#endif

}


void AudioMTKStreamIn::CheckDmicNeedLRSwitch(short *buffer, ssize_t bytes)
{
    short left , right;
    int copysize = bytes >> 2;  //stereo, 16bits

    if (mAttribute.mIsDigitalMIC && mAudioResourceManager->GetMicInvserse())  //channel switch
    {
        ALOGD("CheckDmicNeedLRSwitch, mIsDigitalMIC=%d,GetMicInvserse() =%d", mAttribute.mIsDigitalMIC, mAudioResourceManager->GetMicInvserse());
        while (copysize)
        {
            left = *(buffer);
            right = *(buffer + 1);
            *(buffer) = right;
            *(buffer + 1) = left;
            buffer += 2;
            copysize--;
        }
    }
}


void AudioMTKStreamIn::StereoToMono(short *buffer , int length)
{
    short left , right;
    int sum;
    int copysize = length >> 2;

    while (copysize)
    {
        left = *(buffer);
        right = *(buffer + 1);
        sum = (left + right) >> 1;
        *(buffer) = (short)sum;
        *(buffer + 1) = (short)sum;
        buffer += 2;
        copysize--;
    }
}

ssize_t AudioMTKStreamIn::NativeRecordPreprocess(void *buffer, ssize_t bytes)
{

    if (mAPPS == NULL)
    {
        return bytes;
    }

    if (mVoIPRunning) //not use android native preprocess if using BesVoIP
    {
        return bytes;
    }

    if (mAPPS->num_preprocessors == 0)
    {
        return bytes;
    }
    else
    {
        ssize_t frames_wr = 0;
        audio_buffer_t in_buf;
        audio_buffer_t out_buf;
        const uint8_t num_channel = (channels() == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
        ssize_t frames = bytes / sizeof(int16_t) / num_channel;
        ssize_t needframes = frames + mAPPS->proc_buf_frames;
        int i;

        //ALOGD("process_frames(): %d bytes, %d frames, proc_buf_frames=%d, mAPPS->num_preprocessors=%d,num_channel=%d", bytes,frames,mAPPS->proc_buf_frames,mAPPS->num_preprocessors,num_channel);
        mAPPS->proc_buf_out = (int16_t *)buffer;

        if ((mAPPS->proc_buf_size < (size_t)needframes) || (mAPPS->proc_buf_in == NULL))
        {
            mAPPS->proc_buf_size = (size_t)needframes;
            mAPPS->proc_buf_in = (int16_t *)realloc(mAPPS->proc_buf_in, mAPPS->proc_buf_size * num_channel * sizeof(int16_t));
            //mpPreProcessIn->proc_buf_out = (int16_t *)realloc(mpPreProcessIn->proc_buf_out, mpPreProcessIn->proc_buf_size*mChNum*sizeof(int16_t));

            if (mAPPS->proc_buf_in == NULL)
            {
                ALOGW("proc_buf_in realloc fail");
                return bytes;
            }
            ALOGD("process_frames(): proc_buf_in %p extended to %d bytes", mAPPS->proc_buf_in, mAPPS->proc_buf_size * sizeof(int16_t)*num_channel);
        }

        memcpy(mAPPS->proc_buf_in + mAPPS->proc_buf_frames * num_channel, buffer, bytes);

        mAPPS->proc_buf_frames += frames;

        while (frames_wr < frames)
        {
            //AEC
            if (mEcho_Reference != NULL)
            {
                mAPPS->push_echo_reference(mAPPS->proc_buf_frames);
            }
            else
            {
                //prevent start_echo_reference fail previously due to output not enable
                if (mAPPS->need_echo_reference)
                {
                    ALOGD("try start_echo_reference");
                    mEcho_Reference = mAPPS->start_echo_reference(
                                          AUDIO_FORMAT_PCM_16_BIT,
                                          num_channel,
                                          sampleRate());
                }
            }

            /* in_buf.frameCount and out_buf.frameCount indicate respectively
                  * the maximum number of frames to be consumed and produced by process() */
            in_buf.frameCount = mAPPS->proc_buf_frames;
            in_buf.s16 = mAPPS->proc_buf_in;
            out_buf.frameCount = frames - frames_wr;
            out_buf.s16 = (int16_t *)mAPPS->proc_buf_out + frames_wr * num_channel;

            /* FIXME: this works because of current pre processing library implementation that
                 * does the actual process only when the last enabled effect process is called.
                 * The generic solution is to have an output buffer for each effect and pass it as
                 * input to the next.
                 */

            for (i = 0; i < mAPPS->num_preprocessors; i++)
            {
                (*mAPPS->preprocessors[i].effect_itfe)->process(mAPPS->preprocessors[i].effect_itfe,
                                                                &in_buf,
                                                                &out_buf);
            }

            /* process() has updated the number of frames consumed and produced in
                  * in_buf.frameCount and out_buf.frameCount respectively
                 * move remaining frames to the beginning of in->proc_buf_in */
            mAPPS->proc_buf_frames -= in_buf.frameCount;

            if (mAPPS->proc_buf_frames)
            {
                memcpy(mAPPS->proc_buf_in,
                       mAPPS->proc_buf_in + in_buf.frameCount * num_channel,
                       mAPPS->proc_buf_frames * num_channel * sizeof(int16_t));
            }

            /* if not enough frames were passed to process(), read more and retry. */
            if (out_buf.frameCount == 0)
            {
                ALOGV("No frames produced by preproc");
                break;
            }

            if ((frames_wr + (ssize_t)out_buf.frameCount) <= frames)
            {
                frames_wr += out_buf.frameCount;
                ALOGV("out_buf.frameCount=%d,frames_wr=%d", out_buf.frameCount, frames_wr);
            }
            else
            {
                /* The effect does not comply to the API. In theory, we should never end up here! */
                ALOGE("preprocessing produced too many frames: %d + %d  > %d !",
                      (unsigned int)frames_wr, out_buf.frameCount, (unsigned int)frames);
                frames_wr = frames;
            }
        }
        //        ALOGD("frames_wr=%d, bytes=%d",frames_wr,frames_wr*num_channel*sizeof(int16_t));
        return frames_wr * num_channel * sizeof(int16_t);
    }

}

#ifdef MTK_AUDIO_HD_REC_SUPPORT
#define NORMAL_RECORDING_DEFAULT_MODE    (1)
#define VOICE_REC_RECORDING_DEFAULT_MODE (0)
#define VOICE_UnLock_RECORDING_DEFAULT_MODE (6)


void AudioMTKStreamIn::LoadHDRecordParams(void)
{
    uint8_t modeIndex;
    uint8_t total_num_scenes = MAX_HD_REC_SCENES;
    ALOGD("LoadHdRecordParams");
    // get scene table
    if (GetHdRecordSceneTableFromNV(&mhdRecordSceneTable) == 0)
    {
        ALOGD("GetHdRecordSceneTableFromNV fail, use default value");
        memcpy(&mhdRecordSceneTable, &DefaultRecordSceneTable, sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
    }

    // get hd rec param
    if (GetHdRecordParamFromNV(&mhdRecordParam) == 0)
    {
        ALOGD("GetHdRecordParamFromNV fail, use default value");
        memcpy(&mhdRecordParam, &DefaultRecordParam, sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
    }

    //get VoIP param
    if (GetAudioVoIPParamFromNV(&mVOIPParam) == 0)
    {
        ALOGD("GetAudioVoIPParamFromNV fail, use default value");
        memcpy(&mVOIPParam, &Audio_VOIP_Par_default, sizeof(AUDIO_VOIP_PARAM_STRUCT));
    }

#if defined(MTK_DUAL_MIC_SUPPORT)
    //get DMNR param
    if (GetDualMicSpeechParamFromNVRam(&mDMNRParam) == 0)
    {
        ALOGD("GetDualMicSpeechParamFromNVRam fail, use default value");
        memcpy(&mDMNRParam, &dual_mic_custom_default, sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
    }
#endif

#if 1 // Debug print
    for (int i = 0; i < total_num_scenes; i++)
        for (int j = 0; j < NUM_HD_REC_DEVICE_SOURCE; j++)
        {
            ALOGD("vGetHdRecordModeInfo, scene_table[%d][%d] = %d", i, j, mhdRecordSceneTable.scene_table[i][j]);
        }
#endif
}

int AudioMTKStreamIn::CheckHDRecordMode(void)
{
    if (mAudioResourceManager->IsModeIncall() == true)
    {
        ALOGD("CheckHDRecordMode, IsModeIncall = true, return ");
        return -1;
    }

    // HD Record
    uint8_t modeIndex = 0;
    int32 u4SceneIdx = 0;

    if (!GetHdRecordModeInfo(&modeIndex)) //no scene/mode get
    {
        ALOGD("CheckHDRecordMode mAttribute.mdevices=%x, mAttribute.mPredevices=%x, mHDRecordSceneIndex = %d ", mAttribute.mdevices, mAttribute.mPredevices, mHDRecordSceneIndex);
        //can not get match HD record mode, use the default one
        // check if 3rd party camcorder
        if (mAttribute.mSource != AUDIO_SOURCE_CAMCORDER) //not camcorder
        {
            if (mAttribute.mdevices != mAttribute.mPredevices)  //device changed, use previous scene (since scene not changed)
            {
                if (mHDRecordSceneIndex == -1)
                {
                    mHDRecordSceneIndex = NORMAL_RECORDING_DEFAULT_MODE;
                }
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
            else
            {
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else    //default use internal one
                {
                    modeIndex = mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
        }
        else  //camcoder
        {
            u4SceneIdx = mhdRecordSceneTable.num_voice_rec_scenes + NORMAL_RECORDING_DEFAULT_MODE;//1:cts verifier offset
            if (mAttribute.mdevices != mAttribute.mPredevices)  //device changed, use previous scene
            {
                if (mHDRecordSceneIndex == -1)
                {
                    mHDRecordSceneIndex = u4SceneIdx;
                }
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
            else
            {
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else    //default use internal one
                {
                    modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
        }

#if defined(MTK_DUAL_MIC_SUPPORT)
        //also need to configure the channel when use default mode
        /* only stereo flag is true, the stereo record is enabled */
        if (mAttribute.mdevices ==  AUDIO_DEVICE_IN_BUILTIN_MIC) //handset
        {
            if (mhdRecordParam.hd_rec_map_to_stereo_flag[modeIndex] != 0)
            {
                mStereoMode = true;
            }
        }
#endif
    }
    ALOGD("SetHdrecordingMode,modeIndex=%d,mdevices=%d", modeIndex, mAttribute.mdevices);
    return modeIndex;
}

bool AudioMTKStreamIn::GetHdRecordModeInfo(uint8_t *modeIndex)
{
    bool  ret = false;
    int32 u4SceneIdx = mAudioSpeechEnhanceInfoInstance->GetHDRecScene();
    mAudioSpeechEnhanceInfoInstance->ResetHDRecScene();
    mStereoMode = false;

    ALOGD("+GetHdRecordModeInfo: u4SceneIdx = %d", u4SceneIdx);
    //special input source case
    if ((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) || mHDRecTunning16K)
    {
        ALOGD("voice recognition case");
        u4SceneIdx = VOICE_REC_RECORDING_DEFAULT_MODE;
    }
    else if (mAttribute.mSource == AUDIO_SOURCE_VOICE_UNLOCK)
    {
        ALOGD("voice unlock case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE;
    }
    else if (mAttribute.mSource == AUDIO_SOURCE_CUSTOMIZATION1)
    {
        ALOGD("CUSTOMIZATION1 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 1;
    }
    else if (mAttribute.mSource == AUDIO_SOURCE_CUSTOMIZATION2)
    {
        ALOGD("CUSTOMIZATION2 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 2;
    }
    else if (mAttribute.mSource == AUDIO_SOURCE_CUSTOMIZATION3)
    {
        ALOGD("CUSTOMIZATION3 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 3;
    }

    //propitary API case
    if (mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)   //for BT record case, use specific params
    {
        if (mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_BT_EARPHONE] != 0xFF)
        {
            *modeIndex = mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_BT_EARPHONE];
            ALOGD("(RecOpen)vGetHdRecordModeInfo: BT,  modeIndex = %d", *modeIndex);
        }
        else
        {
            *modeIndex = mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HANDSET];
        }
        mHDRecordSceneIndex = NORMAL_RECORDING_DEFAULT_MODE;
        ret = true;
    }
    else if ((u4SceneIdx >= 0) && (u4SceneIdx < MAX_HD_REC_SCENES))
    {
        // get mode index
        if (mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET] != 0xFF
            && mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)
        {
            *modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
            ALOGD("(RecOpen)vGetHdRecordModeInfo: HEADSET,  modeIndex = %d", *modeIndex);
        }
        // Handset Mic
        else if (mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET] != 0xFF)
        {
            *modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
            ALOGD("(RecOpen)vGetHdRecordModeInfo: HANDSET,  modeIndex = %d", *modeIndex);
#if defined(MTK_DUAL_MIC_SUPPORT)
            /* only stereo flag is true, the stereo record is enabled */
            if (mhdRecordParam.hd_rec_map_to_stereo_flag[*modeIndex] != 0)
            {
                mStereoMode = true;
            }
#endif
        }
        else
        {
            ALOGD("(RecOpen)vGetHdRecordModeInfo: Handset mode index shoule not be -1");
        }

#if 1 // Debug print
        ALOGD("(RecOpen)vGetHdRecordModeInfo: map_fir_ch1=%d, map_fir_ch2=%d, device_mode=%d",
              mhdRecordParam.hd_rec_map_to_fir_for_ch1[*modeIndex],
              mhdRecordParam.hd_rec_map_to_fir_for_ch2[*modeIndex],
              mhdRecordParam.hd_rec_map_to_dev_mode[*modeIndex]);
#endif
        mHDRecordSceneIndex = u4SceneIdx;
        ret = true;
    }
    else
    {
        *modeIndex = 0;
        ret = false;
    }

    ALOGD("(RecOpen)-vGetHdRecordModeInfo: ENUM_HD_Record_Mode = %d, mStereoMode = %d", *modeIndex, mStereoMode);
    return ret;
}

void AudioMTKStreamIn::ConfigHDRecordParams(SPE_MODE mode)
{
    bool ret = false;
    unsigned long HDRecordEnhanceParas[EnhanceParasNum] = {0};
    short HDRecordCompenFilter[CompenFilterNum] = {0};
    short DMNRParam[DMNRCalDataNum] = {0};

    int RoutePath = GetRoutePath();

    ALOGD("ConfigHDRecordParams mode=%d,mStereoMode=%d, input source= %d, mdevices=%x,RoutePath=%d,mHDRecordModeIndex=%d", mode, mStereoMode, mAttribute.mSource, mAttribute.mdevices, RoutePath,
          mHDRecordModeIndex);
    //speech parameters+++
    for (int i = 0; i < EnhanceParasNum; i++) //EnhanceParasNum = 16+12(common parameters)
    {
        if (!mIsAPDMNRTuningEnable)
        {
            if (mode == SPE_MODE_VOIP) //VoIP case
            {
                if (i < SPEECH_PARA_NUM)
                {
                    //specific parameters
                    if (RoutePath == ROUTE_BT)
                    {
                        HDRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_BT][i];
                    }
                    else if (RoutePath == ROUTE_HEADSET)
                    {
                        HDRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_HEADSET][i];
                    }
                    else if (RoutePath == ROUTE_SPEAKER)
                    {
                        HDRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_SPEAKER][i];
                    }
                    else    //normal receiver case
                    {
                        HDRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_NORMAL][i];
                    }
                }
                else
                {
                    //common parameters
                    HDRecordEnhanceParas[i] = mVOIPParam.speech_common_para[i - SPEECH_PARA_NUM];
                }
            }
            else    //default use record params
            {
                if (i < SPEECH_PARA_NUM)
                {
                    HDRecordEnhanceParas[i] = mhdRecordParam.hd_rec_speech_mode_para[mHDRecordModeIndex][i];
                }
                else
                {
                    //common parameters also use VoIP's
                    HDRecordEnhanceParas[i] = mVOIPParam.speech_common_para[i - SPEECH_PARA_NUM];
                }
            }
        }
        else    //audio tool tuning case
        {
            if (i < SPEECH_PARA_NUM)
            {
                //specific parameters
                HDRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[0][i];     //use loud speaker mode speech params
            }
            else
            {
                //common parameters
                HDRecordEnhanceParas[i] = mVOIPParam.speech_common_para[i - SPEECH_PARA_NUM];
            }
        }
        //ALOGD("HDRecordEnhanceParas[%d]=%d", i, HDRecordEnhanceParas[i]);
    }

    mpSPELayer->SetEnhPara(mode, HDRecordEnhanceParas);
    //speech parameters---

    //FIR parameters+++
    for (int i = 0; i < WB_FIR_NUM; i++)
    {
        if (!mIsAPDMNRTuningEnable)
        {
            if (mode == SPE_MODE_VOIP) //VoIP case
            {
                if (RoutePath == ROUTE_BT)
                {
                    HDRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_BT][i];   //UL1 params
                    HDRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_BT][i];  //UL2 params
                    HDRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_BT][i]; //DL params
                }
                else if (RoutePath == ROUTE_HEADSET)
                {
                    HDRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_HEADSET][i];   //UL1 params
                    HDRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_HEADSET][i];  //UL2 params
                    HDRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_HEADSET][i]; //DL params
                }
                else if (RoutePath == ROUTE_SPEAKER)
                {
                    HDRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_SPEAKER][i];   //UL1 params
                    HDRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_SPEAKER][i];  //UL2 params
                    HDRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_SPEAKER][i]; //DL params
                }
                else    //normal receiver case
                {
                    HDRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];   //UL1 params
                    HDRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];  //UL2 params
                    HDRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_NORMAL][i]; //DL params
                }
                //ALOGD("HDRecordCompenFilter[%d]=%d, DL %d", i, HDRecordCompenFilter[i],HDRecordCompenFilter[i+WB_FIR_NUM*2]);

            }
            else    //default use recording params
            {
                HDRecordCompenFilter[i] = mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch1[mHDRecordModeIndex]][i];
                //ALOGD("HDRecordCompenFilter[%d]=%d", i, HDRecordCompenFilter[i]);
                if (mStereoMode) //stereo, UL2 use different FIR filter
                {
                    HDRecordCompenFilter[i + WB_FIR_NUM] = mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch2[mHDRecordModeIndex]][i];
                }
                else    //mono, UL2 use the same FIR filter
                {
                    HDRecordCompenFilter[i + WB_FIR_NUM] = mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch1[mHDRecordModeIndex]][i];
                }
            }
        }
        else
        {
            HDRecordCompenFilter[i] = mVOIPParam.in_fir[0][i];
            //ALOGD("HDRecordCompenFilter[%d]=%d", i, HDRecordCompenFilter[i]);
            HDRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[0][i];
        }
    }

    mpSPELayer->SetCompFilter(mode, HDRecordCompenFilter);
    //FIR parameters---

    //DMNR parameters+++
#ifdef MTK_DUAL_MIC_SUPPORT
    //DMNR parameters
    if (mode == SPE_MODE_VOIP) //VoIP case
    {
        //receiver path & receiver DMNR is enabled
        if ((RoutePath == ROUTE_NORMAL) && mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR) &&
            ((QueryFeatureSupportInfo()& SUPPORT_VOIP_NORMAL_DMNR) > 0))
        {
            //enable corresponding DMNR flag
            for (int i = 0; i < NUM_ABFWB_PARAM; i++)
            {
                DMNRParam[i] = mDMNRParam.ABF_para_VOIP[i];
            }
            SetDMNREnable(DMNR_NORMAL, true);
        }
        else if ((RoutePath == ROUTE_SPEAKER) && mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) &&
                 ((QueryFeatureSupportInfo()& SUPPORT_VOIP_HANDSFREE_DMNR) > 0)) //speaker path
        {
            for (int i = 0; i < NUM_ABFWB_PARAM; i++)
            {
                DMNRParam[i] = mDMNRParam.ABF_para_VOIP_LoudSPK[i];
            }
            SetDMNREnable(DMNR_HANDSFREE, true);
        }
        else
        {
            memset(DMNRParam, 0, sizeof(DMNRParam));
            SetDMNREnable(DMNR_DISABLE, false);
        }
    }
    else    //default using recording one
    {
        //google default input source AUDIO_SOURCE_VOICE_RECOGNITION not using DMNR
        if (mIsAPDMNRTuningEnable || (((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) || mUsingMagiASR) && ((QueryFeatureSupportInfo()& SUPPORT_ASR) > 0)))
        {
            for (int i = 0; i < NUM_ABFWB_PARAM; i++)
            {
                DMNRParam[i] = mDMNRParam.ABF_para_VR[i];
            }
        }
        else
        {
            memset(DMNRParam, 0, sizeof(DMNRParam));
        }
    }

    mpSPELayer->SetDMNRPara(mode, DMNRParam);
#else
    memset(DMNRParam, 0, sizeof(DMNRParam));
    SetDMNREnable(DMNR_DISABLE, false);
    mpSPELayer->SetDMNRPara(mode, DMNRParam);
#endif

    //DMNR parameters---

    //config APP table
    if (mode == SPE_MODE_VOIP) //VoIP case
    {
        mStereoMode = false;    //VoIP only mono
        mpSPELayer->SetSampleRate(mode, VOICE_RECOGNITION_RECORD_SAMPLE_RATE);
        mpSPELayer->SetAPPTable(mode, WB_VOIP);
    }
    else
    {
        //need to config as 16k sample rate for voice recognition
        if ((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) || mUsingMagiASR || mUsingAECRec || mHDRecTunning16K)
        {
            mpSPELayer->SetSampleRate(mode, VOICE_RECOGNITION_RECORD_SAMPLE_RATE);
            if (mUsingAECRec)
            {
                mpSPELayer->SetAPPTable(mode, MONO_AEC_RECORD);
            }
            else
            {
                mpSPELayer->SetAPPTable(mode, SPEECH_RECOGNITION);
            }
        }
        else    //normal record  use 48k
        {
            mpSPELayer->SetSampleRate(mode, mAttribute.mSampleRate);
            if (mStereoMode)
            {
                mpSPELayer->SetAPPTable(mode, STEREO_RECORD);    //set library do stereo process
            }
            else
            {
                mpSPELayer->SetAPPTable(mode, MONO_RECORD);    //set library do mono process
            }
        }
    }

    mpSPELayer->SetRoute((SPE_ROUTE)RoutePath);

    //set MIC digital gain to library
    long gain = mAudioResourceManager->GetSwMICDigitalGain();
    uint8_t TotalGain = mAudioResourceManager->GetULTotalGainValue();
    if (mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        gain = 0;
        TotalGain = 0;
        ALOGD("BT path set Digital MIC gain = 0");
    }
    mpSPELayer->SetMICDigitalGain(mode, gain);
    mpSPELayer->SetUpLinkTotalGain(mode, TotalGain);

    if ((mode == SPE_MODE_VOIP) || (mode == SPE_MODE_AECREC)) //VoIP case
    {
        //set VoIP platform dependent delay time offset (ms)
        mpSPELayer->SetPlatfromTimeOffset(VOIP_PLATFORM_OFFSET_TIME);
    }

}

void AudioMTKStreamIn::UpdateDynamicFunction()
{
#ifdef MTK_DUAL_MIC_SUPPORT
    int RoutePath = GetRoutePath();
    SPE_MODE mode = mpSPELayer->GetMode();
    short DMNRParam[DMNRCalDataNum] = {0};

    ALOGD("%s(), RoutePath %d, mode %d", __FUNCTION__, RoutePath, mode);

#ifdef MTK_HANDSFREE_DMNR_SUPPORT
    if (mode == SPE_MODE_VOIP)
    {
        //receiver path & receiver DMNR is enabled
        if ((RoutePath == ROUTE_NORMAL) && mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR) &&
            ((QueryFeatureSupportInfo()& SUPPORT_VOIP_NORMAL_DMNR) > 0))
        {
            ALOGD("enable normal mode DMNR");
            //enable corresponding DMNR flag
            for (int i = 0; i < NUM_ABFWB_PARAM; i++)
            {
                DMNRParam[i] = mDMNRParam.ABF_para_VOIP[i];
            }
            SetDMNREnable(DMNR_NORMAL, true);
        }
        else if ((RoutePath == ROUTE_SPEAKER) && mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) &&
                 ((QueryFeatureSupportInfo()& SUPPORT_VOIP_HANDSFREE_DMNR) > 0)) //speaker path
        {
            ALOGD("enable loudspeaker mode DMNR");
            for (int i = 0; i < NUM_ABFWB_PARAM; i++)
            {
                DMNRParam[i] = mDMNRParam.ABF_para_VOIP_LoudSPK[i];
            }
            SetDMNREnable(DMNR_HANDSFREE, true);
        }
        else
        {
            ALOGD("disable DMNR");
            memset(DMNRParam, 0, sizeof(DMNRParam));
            SetDMNREnable(DMNR_DISABLE, false);
        }
        mpSPELayer->SetDMNRPara(mode, DMNRParam);
    }

#else
    ALOGD("%s(),disable DMNR due to not support", __FUNCTION__);
    memset(DMNRParam, 0, sizeof(DMNRParam));
    SetDMNREnable(DMNR_DISABLE, false);
    mpSPELayer->SetDMNRPara(mode, DMNRParam);
#endif
#endif
}

//0: disable DMNR
//1: normal mode DMNR
//2: handsfree mode DMNR
void AudioMTKStreamIn::SetDMNREnable(DMNR_TYPE type, bool enable)
{
    ALOGD("%s(), type=%d", __FUNCTION__, type);
#ifdef MTK_DUAL_MIC_SUPPORT
#ifdef MTK_HANDSFREE_DMNR_SUPPORT
    switch (type)
    {
        case DMNR_DISABLE :
            mpSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
            mpSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
            break;
        case DMNR_NORMAL :
            mpSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, enable);
            break;
        case DMNR_HANDSFREE :
            mpSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, enable);
            break;
        default:
            mpSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
            mpSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
            break;
    }
#else
    ALOGD("%s(), turn off due to not support", __FUNCTION__);
    mpSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
    mpSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
#endif
#endif
}

void AudioMTKStreamIn::StartHDRecord(SPE_MODE mode)
{
    if (mAttribute.mSource != AUDIO_SOURCE_FM && mAttribute.mSource != AUDIO_SOURCE_MATV)
    {
        mpSPELayer->Start(mode);
    }
}

void AudioMTKStreamIn::StopHDRecord(void)
{
    if (mAttribute.mSource != AUDIO_SOURCE_FM && mAttribute.mSource != AUDIO_SOURCE_MATV)
    {
        mpSPELayer->Stop();
    }
}

uint32_t AudioMTKStreamIn::BesRecordPreprocess(void *buffer , uint32_t bytes, AdditionalInfo_STRUCT AddInfo)
{
    //    ALOGD("BesRecordPreprocess bytes=%d", bytes);
    HDRecordPreprocess(buffer , bytes, AddInfo);
    return bytes;
}
uint32_t AudioMTKStreamIn::HDRecordPreprocess(void *buffer , uint32_t bytes, AdditionalInfo_STRUCT AddInfo)
{
    if (mAttribute.mSource != AUDIO_SOURCE_FM && mAttribute.mSource != AUDIO_SOURCE_MATV)
    {
        struct InBufferInfo InBufinfo;
        //if((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) ||mHDRecTunning16K || mVoIPRunning)
        if (mBliHandler1 != 0 && mBliHandler2 != 0)
        {
            //            ALOGD("HDRecordPreprocess buffer=%p, bytes=%d", buffer, bytes);
            uint32 consume = 0;
            size_t inputLength = bytes;
            size_t outputLength = bytes;
            //convert to 16k
            consume = BLI_Convert(mBliHandler1, (int16_t *)buffer, &inputLength, (int16_t *)mBliHandler1Buffer, &outputLength);
            //            ALOGD("HDRecordPreprocess outputLength=%d,consume=%d,mBliHandler1Buffer=%p", outputLength,consume,mBliHandler1Buffer);

            InBufinfo.pBufBase = (short *)mBliHandler1Buffer;
            InBufinfo.BufLen = outputLength;
            InBufinfo.time_stamp_queued = GetSystemTime(false);
            InBufinfo.bHasRemainInfo = AddInfo.bHasAdditionalInfo;
            InBufinfo.time_stamp_predict = AddInfo.timestamp_info;

            mpSPELayer->Process(&InBufinfo);

            //convert back to 48k
            //            ALOGD("HDRecordPreprocess mBliHandler1Buffer=%p,outputLength=%d", mBliHandler1Buffer,outputLength);
            consume = BLI_Convert(mBliHandler2, (int16_t *)mBliHandler1Buffer, &outputLength, (int16_t *)buffer, &bytes);
            //            ALOGD("HDRecordPreprocess 2 bytes=%d,consume=%d", bytes,consume);
            return bytes;
        }
        else
        {
            InBufinfo.pBufBase = (short *)buffer;
            InBufinfo.BufLen = bytes;
            InBufinfo.time_stamp_queued = GetSystemTime(false);
            InBufinfo.bHasRemainInfo = AddInfo.bHasAdditionalInfo;
            InBufinfo.time_stamp_predict = AddInfo.timestamp_info;

            mpSPELayer->Process(&InBufinfo);

            return bytes;
        }
    }
    return bytes;
}

bool AudioMTKStreamIn::IsHDRecordRunning(void)
{
    return mpSPELayer->IsSPERunning();
}

void AudioMTKStreamIn::CheckHDRecordVoIPSwitch(void)
{
    int modeIndex;
    bool forceupdate = ((mAttribute.mSource == AUDIO_SOURCE_VOICE_COMMUNICATION) ||
                        (mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) ||
                        (mAttribute.mPredevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET));
    modeIndex = CheckHDRecordMode();
    if (((modeIndex != mHDRecordModeIndex) && (modeIndex >= 0)) || forceupdate)
    {
        ALOGD("modeIndex=%d, forceupdate=%d", modeIndex, forceupdate);
        if (modeIndex >= 0)
        {
            mHDRecordModeIndex = modeIndex;
        }
        mHDRecordParamUpdate = true;
    }

    if (CheckVoIPNeeded())
    {
        ALOGD("CheckHDRecordVoIPSwitch enter VoIP %p", this);

        if (IsHDRecordRunning())
        {
            if ((mpSPELayer->GetMode() == SPE_MODE_REC) || IsVoIPRouteChange())
            {
                StopHDRecord();
                ConfigHDRecordParams(SPE_MODE_VOIP);
            }
        }
        else
        {
            if (IsVoIPRouteChange())
            {
                ConfigHDRecordParams(SPE_MODE_VOIP);
            }
        }
        mpSPELayer->SetOutputStreamRunning(mAudioSpeechEnhanceInfoInstance->IsOutputRunning());
        StartHDRecord(SPE_MODE_VOIP);
        mVoIPRunning = true;
    }
    else    //normal record
    {
        ALOGD("CheckHDRecordVoIPSwitch enter REC, MagiASR=%d, mUsingAECRec=%d ", mUsingMagiASR, mUsingAECRec);
        if (IsHDRecordRunning() && (mHDRecordParamUpdate || (mpSPELayer->GetMode() == SPE_MODE_VOIP)))
        {
            StopHDRecord();
        }

        if (mHDRecordParamUpdate || (mpSPELayer->GetMode() == SPE_MODE_VOIP))
        {
            if (mUsingMagiASR || mUsingAECRec)
            {
                ConfigHDRecordParams(SPE_MODE_AECREC);
            }
            else
            {
                ConfigHDRecordParams(SPE_MODE_REC);
            }
        }

        if (mUsingMagiASR || mUsingAECRec)
        {
            StartHDRecord(SPE_MODE_AECREC);
        }
        else
        {
            StartHDRecord(SPE_MODE_REC);
        }
        mVoIPRunning = false;
        mHDRecordParamUpdate = false;
    }
}

void AudioMTKStreamIn::NeedUpdateVoIPParams(void)
{
    Mutex::Autolock _l(mLock);
    ALOGD("NeedUpdateVoIPParams");
    if (mVoIPRunning && IsVoIPRouteChange())
    {
        if (IsHDRecordRunning())
        {
            StopHDRecord();
        }
        //mpSPELayer->SetAPPTable(SPE_MODE_VOIP, WB_VOIP);
        ConfigHDRecordParams(SPE_MODE_VOIP);
        StartHDRecord(SPE_MODE_VOIP);
    }
}

bool AudioMTKStreamIn::CheckVoIPNeeded(void)
{
    bool bRet = false;
    bool bNormalModeVoIP = false;
    bool bSkipVoIP = true;

    char value[PROPERTY_VALUE_MAX];
    char valuedisable[PROPERTY_VALUE_MAX];
    property_get("EnableNormalModeVoIP", value, "0");
    int bflag = atoi(value);
    property_get("DisableVoIP", valuedisable, "0");
    int bdisableflag = atoi(valuedisable);

#ifndef MTK_VOIP_ENHANCEMENT_SUPPORT
    if (bSkipVoIP)
    {
        ALOGD("CheckVoIPNeeded, no VoIP define, default off");
        return false;
    }
#endif

    if (bdisableflag)
    {
        ALOGD("CheckVoIPNeeded, disable VoIP");
        return false;
    }

    if (mIsAPDMNRTuningEnable)
    {
        bRet = false;
    }
    else if (GetVoIPRunningState()) //this streamin is in VoIP
    {
#if 0 //change back to normal record if no output is running
        if (!mAudioSpeechEnhanceInfoInstance->IsOutputRunning())
        {
            bRet = false;
        }
        else
        {
            bRet = true;
        }
#else //if it is VoIP process, keep VoIP no matter output is running or not
        bRet = true;
#endif
        ALOGD("CheckVoIPNeeded, check self %d", bRet);
    }
    else if (mAudioSpeechEnhanceInfoInstance->IsVoIPActive())
    {
        bRet = false;
        ALOGD("CheckVoIPNeeded, already has one VoIP running");
    }
    else
    {
        if (bflag || mAudioSpeechEnhanceInfoInstance->GetEnableNormalModeVoIP())
        {
            bNormalModeVoIP = true;
        }
        mpSPELayer->EnableNormalModeVoIP(bNormalModeVoIP);
        if (bNormalModeVoIP)
        {
            ALOGD("CheckVoIPNeeded mHDRecordSceneIndex=%d", mHDRecordSceneIndex);
            if ((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) || mUsingMagiASR || mUsingAECRec || (mHDRecordSceneIndex != -1))
            {
                bRet = false;
            }
            else
            {
                //if enabled normal mode VoIP, always using 16K voip process even is normal record?
                //if(mAudioSpeechEnhanceInfoInstance->IsOutputRunning())
                bRet = true;
            }
        }
        else
        {
            //Need in-communication mode or input source
            if ((mAudioResourceManager->GetAudioMode() == AUDIO_MODE_IN_COMMUNICATION) || (mAttribute.mSource == AUDIO_SOURCE_VOICE_COMMUNICATION))
            {
                bRet = true;
            }
        }

    }

    if (bRet) //if need enter voip mode
    {
        char valueInDump[PROPERTY_VALUE_MAX];
        char valueOutDump[PROPERTY_VALUE_MAX];
        property_get(streamin_propty, valueInDump, "0");
        int bflagInDump = atoi(valueInDump);
        property_get(streamout_propty, valueOutDump, "0");
        int bflagOutDump = atoi(valueOutDump);
        if (bflagInDump || bflagOutDump)
        {
            ALOGW("CheckVoIPNeeded, streamIn/Out pcm dump is enabled, might impact VoIP performance");
        }
    }
    return bRet;
}

int AudioMTKStreamIn::GetRoutePath(void)
{
    int RoutePath;

    if (mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        RoutePath = ROUTE_BT;
    }
    else if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)
    {
        RoutePath = ROUTE_HEADSET;
    }
    else if (mAudioResourceManager->getDlOutputDevice() & AUDIO_DEVICE_OUT_SPEAKER)  //speaker path
    {
        RoutePath = ROUTE_SPEAKER;
    }
    else
    {
        RoutePath = ROUTE_NORMAL;
    }

    return RoutePath;
}

bool AudioMTKStreamIn::IsVoIPRouteChange(void)
{
    bool bRet = false;
    int pre_route = (int)mpSPELayer->GetRoute();
    int new_route = GetRoutePath();

    if (pre_route != new_route)
    {
        bRet = true;
    }

    return bRet;
}
bool AudioMTKStreamIn::GetVoIPRunningState(void)
{
    return mVoIPRunning;
}

#endif  //MTK_AUDIO_HD_REC_SUPPORT

timespec AudioMTKStreamIn::GetSystemTime(bool print)
{
    struct timespec systemtime;
    int rc;
    rc = clock_gettime(CLOCK_MONOTONIC, &systemtime);
    if (rc != 0)
    {
        systemtime.tv_sec  = 0;
        systemtime.tv_nsec = 0;
        ALOGD("clock_gettime error");
    }
    if (print == true)
    {
        ALOGD("GetSystemTime, sec %ld nsec %ld", systemtime.tv_sec, systemtime.tv_nsec);
    }

    return systemtime;
}

}

