#include "AudioSpeechEnhanceInfo.h"
#include <utils/Log.h>
#include <utils/String16.h>
#include "AudioUtility.h"
#include "AudioMTKStreamIn.h"
#include "SpeechEnhancementController.h"
#include <cutils/properties.h>
#include "AudioCustParam.h"

#define LOG_TAG "AudioSpeechEnhanceInfo"


static const char PROPERTY_KEY_VOIP_SPH_ENH_MASKS[PROPERTY_KEY_MAX] = "persist.af.voip.sph_enh_mask";

#define ECHOREF_SAMPLE_RATE (16000)

namespace android
{

AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::UniqueAudioSpeechEnhanceInfoInstance = NULL;

AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::getInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (UniqueAudioSpeechEnhanceInfoInstance == NULL)
    {
        ALOGD("+AudioSpeechEnhanceInfo");
        UniqueAudioSpeechEnhanceInfoInstance = new AudioSpeechEnhanceInfo();
        ALOGD("-AudioSpeechEnhanceInfo");
    }
    ALOGD("AudioSpeechEnhanceInfo getInstance()");
    ASSERT(UniqueAudioSpeechEnhanceInfoInstance != NULL);
    return UniqueAudioSpeechEnhanceInfoInstance;
}

void AudioSpeechEnhanceInfo::freeInstance()
{
    return;
}

AudioSpeechEnhanceInfo::AudioSpeechEnhanceInfo()
{
    ALOGD("AudioSpeechEnhanceInfo constructor");
    mHdRecScene = -1;
    mIsLRSwitch = false;
    mUseSpecificMic = 0;
    mHDRecTunningEnable = false;
    mForceMagiASR = false;
    mForceAECRec = false;
#ifndef DMNR_TUNNING_AT_MODEMSIDE
    mAPDMNRTuningEnable = false;
    mAPTuningMode = TUNING_MODE_NONE;
#endif

    mEnableNormalModeVoIP = false;
    mStreamOut = NULL;

    // default value (all enhancement on)
    char property_default_value[PROPERTY_VALUE_MAX];
    sprintf(property_default_value, "0x%x", VOIP_SPH_ENH_DYNAMIC_MASK_ALL);

    // get voip sph_enh_mask_struct from property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VOIP_SPH_ENH_MASKS, property_value, property_default_value);

    // parse mask info from property_value
    sscanf(property_value, "0x%x",
           &mVoIPSpeechEnhancementMask);

#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    mEchoRefThreadCreated = false;
    mEchoRefClient = NULL;
    mStreamInManager = NULL;
    memset((void *)&mEchoRefAttribute, 0 , sizeof(AudioStreamAttribute));
    mAudioResourceManager = NULL;
    mDLDevice = 0;

    //for android AEC effect
    mEcho_reference = NULL;
    mBliHandlerAndroidAEC = NULL;
    mBliBufferAndroidAEC = NULL;
    mBliOutBufferSizeAndroidAEC = NULL;
#endif

    ALOGD("mVoIPSpeechEnhancementMask = 0x%x", mVoIPSpeechEnhancementMask);

}

AudioSpeechEnhanceInfo::~AudioSpeechEnhanceInfo()
{
    ALOGD("AudioSpeechEnhanceInfo destructor");
    mHdRecScene = -1;
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    if (mEcho_reference)
    {
        mEcho_reference = NULL;
    }
    if (mBliHandlerAndroidAEC)
    {
        mBliHandlerAndroidAEC->Close();
        delete mBliHandlerAndroidAEC;
        mBliHandlerAndroidAEC = NULL;
        ALOGD("~AudioSpeechEnhanceInfo delete mBliHandlerAndroidAEC");
    }
    if (mBliBufferAndroidAEC)
    {
        delete[] mBliBufferAndroidAEC;
        mBliBufferAndroidAEC = NULL;
        ALOGD("~AudioSpeechEnhanceInfo delete mBliBufferAndroidAEC");
    }
#endif
}


void AudioSpeechEnhanceInfo::SetRecordLRChannelSwitch(bool bIsLRSwitch)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetRecordLRChannelSwitch=%x", bIsLRSwitch);
    mIsLRSwitch = bIsLRSwitch;
}

bool AudioSpeechEnhanceInfo::GetRecordLRChannelSwitch(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetRecordLRChannelSwitch=%x", mIsLRSwitch);
    return mIsLRSwitch;
}

void AudioSpeechEnhanceInfo::SetUseSpecificMIC(int32 UseSpecificMic)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetUseSpecificMIC=%x", UseSpecificMic);
    mUseSpecificMic = UseSpecificMic;
}

int AudioSpeechEnhanceInfo::GetUseSpecificMIC(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetUseSpecificMIC=%x", mUseSpecificMic);
    return mUseSpecificMic;
}

bool AudioSpeechEnhanceInfo::SetForceMagiASR(bool enable)
{
    ALOGD("%s, %d", __FUNCTION__, enable);
    mForceMagiASR = enable;
    return true;
}

status_t AudioSpeechEnhanceInfo::GetForceMagiASRState()
{
    status_t ret = 0;
    uint32_t feature_support = QueryFeatureSupportInfo();

    ALOGD("%s(), feature_support=%x, %x, mForceMagiASR=%d", __FUNCTION__, feature_support, (feature_support & SUPPORT_ASR), mForceMagiASR);

    if (feature_support & SUPPORT_ASR)
    {
        if (mForceMagiASR)
        {
            ret = 1;
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = 0;
    }

    return ret;
}

bool AudioSpeechEnhanceInfo::SetForceAECRec(bool enable)
{
    ALOGD("%s, %d", __FUNCTION__, enable);
    mForceAECRec = enable;
    return true;
}

bool AudioSpeechEnhanceInfo::GetForceAECRecState()
{
    status_t ret = false;

    ALOGD("%s(), mForceAECRec=%d", __FUNCTION__, mForceAECRec);

    if (mForceAECRec)
    {
        ret = true;
    }
    return ret;
}


//----------------for HD Record Preprocess-----------------------------
void AudioSpeechEnhanceInfo::SetHDRecScene(int32 HDRecScene)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo SetHDRecScene=%d", HDRecScene);
    mHdRecScene = HDRecScene;
}

int32 AudioSpeechEnhanceInfo::GetHDRecScene()
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo GetHDRecScene=%d", mHdRecScene);
    return mHdRecScene;
}

void AudioSpeechEnhanceInfo::ResetHDRecScene()
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo ResetHDRecScene");
    mHdRecScene = -1;
}

//----------------for HDRec tunning --------------------------------
void AudioSpeechEnhanceInfo::SetHDRecTunningEnable(bool bEnable)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetHDRecTunningEnable=%d", bEnable);
    mHDRecTunningEnable = bEnable;
}

bool AudioSpeechEnhanceInfo::IsHDRecTunningEnable(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("IsHDRecTunningEnable=%d", mHDRecTunningEnable);
    return mHDRecTunningEnable;
}

status_t AudioSpeechEnhanceInfo::SetHDRecVMFileName(const char *fileName)
{
    Mutex::Autolock lock(mHDRInfoLock);
    if (fileName != NULL && strlen(fileName) < 128 - 1)
    {
        ALOGD("SetHDRecVMFileName file name:%s", fileName);
        memset(mVMFileName, 0, 128);
        strcpy(mVMFileName, fileName);
    }
    else
    {
        ALOGD("input file name NULL or too long!");
        return BAD_VALUE;
    }
    return NO_ERROR;
}
void AudioSpeechEnhanceInfo::GetHDRecVMFileName(char *VMFileName)
{
    Mutex::Autolock lock(mHDRInfoLock);
    memset(VMFileName, 0, 128);
    strcpy(VMFileName, mVMFileName);
    ALOGD("GetHDRecVMFileName mVMFileName=%s, VMFileName=%s", mVMFileName, VMFileName);
}

#ifndef DMNR_TUNNING_AT_MODEMSIDE
//----------------for AP DMNR tunning --------------------------------
void AudioSpeechEnhanceInfo::SetAPDMNRTuningEnable(bool bEnable)
{
#ifdef MTK_DUAL_MIC_SUPPORT
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetAPDMNRTuningEnable=%d", bEnable);
    mAPDMNRTuningEnable = bEnable;
#else
    ALOGD("SetAPDMNRTuningEnable not Dual MIC, not set");
#endif
}

bool AudioSpeechEnhanceInfo::IsAPDMNRTuningEnable(void)
{
#ifdef MTK_DUAL_MIC_SUPPORT
    Mutex::Autolock lock(mHDRInfoLock);
    //ALOGD("IsAPDMNRTuningEnable=%d",mAPDMNRTuningEnable);
    return mAPDMNRTuningEnable;
#else
    return false;
#endif
}

bool AudioSpeechEnhanceInfo::SetAPTuningMode(const TOOL_TUNING_MODE mode)
{
    bool bRet = false;
    ALOGD("SetAPTuningMode mAPDMNRTuningEnable=%d, mode=%d", mAPDMNRTuningEnable, mode);
    if (mAPDMNRTuningEnable)
    {
        mAPTuningMode = mode;
        bRet = true;
    }
    return bRet;
}

int AudioSpeechEnhanceInfo::GetAPTuningMode()
{
    ALOGD("GetAPTuningMode, mAPTuningMode=%d", mAPTuningMode);
    return mAPTuningMode;
}

#endif

//----------------Get MMI info for AP Speech Enhancement --------------------------------
bool AudioSpeechEnhanceInfo::GetDynamicSpeechEnhancementMaskOnOff(const voip_sph_enh_dynamic_mask_t dynamic_mask_type)
{
    bool bret = false;
    voip_sph_enh_mask_struct_t mask;

    mask = GetDynamicVoIPSpeechEnhancementMask();

    if ((mask.dynamic_func & dynamic_mask_type) > 0)
    {
        bret = true;
    }

    ALOGD("%s(), %x, %x, bret=%d", __FUNCTION__, mask.dynamic_func, dynamic_mask_type, bret);
    return bret;
}

void AudioSpeechEnhanceInfo::UpdateDynamicSpeechEnhancementMask(const voip_sph_enh_mask_struct_t &mask)
{
    uint32_t feature_support = QueryFeatureSupportInfo();

    ALOGD("%s(), mask = %x, feature_support=%x, %x", __FUNCTION__, mask, feature_support, (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE)));

    if (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE))
    {

        char property_value[PROPERTY_VALUE_MAX];
        sprintf(property_value, "0x%x", mask);
        property_set(PROPERTY_KEY_VOIP_SPH_ENH_MASKS, property_value);

        mVoIPSpeechEnhancementMask = mask;

        if (mSPELayerVector.size())
        {
            for (size_t i = 0; i < mSPELayerVector.size() ; i++)
            {
                AudioMTKStreamIn *pTempMTKStreamIn = (AudioMTKStreamIn *)mSPELayerVector.keyAt(i);
                pTempMTKStreamIn->UpdateDynamicFunction();
            }
        }
    }
    else
    {
        ALOGD("%s(), not support", __FUNCTION__);
    }

}

status_t AudioSpeechEnhanceInfo::SetDynamicVoIPSpeechEnhancementMask(const voip_sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on)
{
    //Mutex::Autolock lock(mHDRInfoLock);
    uint32_t feature_support = QueryFeatureSupportInfo();

    ALOGD("%s(), feature_support=%x, %x", __FUNCTION__, feature_support, (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE)));

    if (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE))
    {
        voip_sph_enh_mask_struct_t mask = GetDynamicVoIPSpeechEnhancementMask();

        ALOGW("%s(), dynamic_mask_type(%x), %x",
              __FUNCTION__, dynamic_mask_type, mask.dynamic_func);
        const bool current_flag_on = ((mask.dynamic_func & dynamic_mask_type) > 0);
        if (new_flag_on == current_flag_on)
        {
            ALOGW("%s(), dynamic_mask_type(%x), new_flag_on(%d) == current_flag_on(%d), return",
                  __FUNCTION__, dynamic_mask_type, new_flag_on, current_flag_on);
            return NO_ERROR;
        }

        if (new_flag_on == false)
        {
            mask.dynamic_func &= (~dynamic_mask_type);
        }
        else
        {
            mask.dynamic_func |= dynamic_mask_type;
        }

        UpdateDynamicSpeechEnhancementMask(mask);
    }
    else
    {
        ALOGW("%s(), not support", __FUNCTION__);
    }

    return NO_ERROR;
}


//----------------for Android Native Preprocess-----------------------------
void AudioSpeechEnhanceInfo::SetStreamOutPointer(void *pStreamOut)
{
    if (pStreamOut == NULL)
    {
        ALOGW(" SetStreamOutPointer pStreamOut = NULL");
    }
    else
    {
        mStreamOut = (AudioMTKStreamOut *)pStreamOut;
        ALOGW("SetStreamOutPointer mStreamOut=%p", mStreamOut);
    }
}

int AudioSpeechEnhanceInfo::GetOutputSampleRateInfo(void)
{
    int samplerate = 48000;
    if (mStreamOut != NULL)
    {
        samplerate = mStreamOut->GetSampleRate();
    }
    ALOGD("AudioSpeechEnhanceInfo GetOutputSampleRateInfo=%d", samplerate);
    return samplerate;
}

int AudioSpeechEnhanceInfo::GetOutputChannelInfo(void)
{
    int chn = 1;
    if (mStreamOut != NULL)
    {
        chn = mStreamOut->GetChannel();
    }
    ALOGD("AudioSpeechEnhanceInfo GetOutputChannelInfo=%d", chn);
    return chn;
}

bool AudioSpeechEnhanceInfo::IsOutputRunning(void)
{
    if (mStreamOut == NULL)
    {
        return false;
    }
    return mStreamOut->GetStreamRunning();
}

void AudioSpeechEnhanceInfo::add_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("AudioSpeechEnhanceInfo add_echo_reference=%p", reference);
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock _l(mEffectLock);
    ALOGD("add_echo_reference %p", reference);
    mEcho_reference = reference;
    if (mBliHandlerAndroidAEC == NULL)
    {
        if (mStreamInManager != NULL)
        {
            mBliOutBufferSizeAndroidAEC = mStreamInManager->MemAWBBufferSize;
        }
        else
        {
            mBliOutBufferSizeAndroidAEC = 0x2000;
        }
        //data from record echo ref thread is 16k mono, but android AEC needs stereo data
        mBliHandlerAndroidAEC = new MtkAudioSrc(ECHOREF_SAMPLE_RATE , 1, ECHOREF_SAMPLE_RATE, 2, SRC_IN_Q1P15_OUT_Q1P15);

        mBliHandlerAndroidAEC->Open();

        mBliBufferAndroidAEC = new char[mBliOutBufferSizeAndroidAEC]; // tmp buffer for blisrc out
        ASSERT(mBliBufferAndroidAEC != NULL);
        ALOGD("add_echo_reference create mBliHandlerAndroidAEC");
    }
    else
    {
        ALOGD("add_echo_reference mBliHandlerAndroidAEC already exist!!");
    }
#else
    if (mStreamOut != NULL)
    {
        mStreamOut->add_echo_reference(reference);
    }
#endif
}
void AudioSpeechEnhanceInfo::remove_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("AudioSpeechEnhanceInfo remove_echo_reference=%p", reference);
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock _l(mEffectLock);
    ALOGD("remove_echo_reference %p", reference);
    if (mEcho_reference == reference)
    {
        /* stop writing to echo reference */
        reference->write(reference, NULL);
        mEcho_reference = NULL;
    }
    else
    {
        ALOGW("remove wrong echo reference %p, mEcho_reference %p", reference, mEcho_reference);
    }

    if (mEcho_reference == NULL)
    {
        if (mBliHandlerAndroidAEC)
        {
            mBliHandlerAndroidAEC->Close();
            delete mBliHandlerAndroidAEC;
            mBliHandlerAndroidAEC = NULL;
            ALOGD("remove_echo_reference delete mBliHandlerAndroidAEC");
        }
        if (mBliBufferAndroidAEC)
        {
            delete[] mBliBufferAndroidAEC;
            mBliBufferAndroidAEC = NULL;
            ALOGD("remove_echo_reference delete mBliBufferAndroidAEC");
        }
    }
    ALOGD("remove_echo_reference ---");
#else
    if (mStreamOut != NULL)
    {
        mStreamOut->remove_echo_reference(reference);
    }
#endif
}

void AudioSpeechEnhanceInfo::SetOutputStreamRunning(bool bRun)
{
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock lock(mInputInfoLock);
#else
    Mutex::Autolock lock(mHDRInfoLock);
#endif
    ALOGD("SetOutputStreamRunning %d, SPELayer %d", bRun, mSPELayerVector.size());

    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            SPELayer *pTempSPELayer = (SPELayer *)mSPELayerVector.valueAt(i);
            pTempSPELayer->SetOutputStreamRunning(bRun, true);
        }
    }
}
void AudioSpeechEnhanceInfo::SetSPEPointer(AudioMTKStreamIn *pMTKStreamIn, SPELayer *pSPE)
{
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock lock(mInputInfoLock);
#else
    Mutex::Autolock lock(mHDRInfoLock);
#endif

    ALOGD("AudioSpeechEnhanceInfo SetSPEPointer %p, %p", pMTKStreamIn, pSPE);
    //mStreamOut->SetSPEPointer(pSPE);
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            if (pMTKStreamIn == mSPELayerVector.keyAt(i))
            {
                ALOGD("SetSPEPointer already add this before, not add it again");
                return;
            }
        }
    }
    if (mStreamOut != NULL)
    {
        pSPE->SetDownLinkLatencyTime(mStreamOut->latency());
    }
    mSPELayerVector.add(pMTKStreamIn, pSPE);
    ALOGD("SetSPEPointer size %d", mSPELayerVector.size());
}

void AudioSpeechEnhanceInfo::ClearSPEPointer(AudioMTKStreamIn *pMTKStreamIn)
{
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock lock(mInputInfoLock);
#else
    Mutex::Autolock lock(mHDRInfoLock);
#endif

    ALOGD("ClearSPEPointer %p, size=%d", pMTKStreamIn, mSPELayerVector.size());
    //mStreamOut->ClearSPEPointer();
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            if (pMTKStreamIn == mSPELayerVector.keyAt(i))
            {
                ALOGD("find and remove it ++");
                mSPELayerVector.removeItem(pMTKStreamIn);
                ALOGD("find and remove it --");
            }
        }
    }
}

bool AudioSpeechEnhanceInfo::IsInputStreamAlive(void)
{
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock lock(mInputInfoLock);
#else
    Mutex::Autolock lock(mHDRInfoLock);
#endif

    if (mSPELayerVector.size())
    {
        return true;
    }
    return false;
}

//no argument, check if there is VoIP running input stream
//MTKStreamIn argument, check if the dedicated MTKStreamIn is VoIP running stream
bool AudioSpeechEnhanceInfo::IsVoIPActive(AudioMTKStreamIn *pMTKStreamIn)
{
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock lock(mInputInfoLock);
#else
    Mutex::Autolock lock(mHDRInfoLock);
#endif

    if (mSPELayerVector.size())
    {
        if (pMTKStreamIn == NULL)
        {
            //ALOGD("IsVoIPActive!");
            for (size_t i = 0; i < mSPELayerVector.size() ; i++)
            {
                AudioMTKStreamIn *pTempMTKStreamIn = (AudioMTKStreamIn *)mSPELayerVector.keyAt(i);
                if (pTempMTKStreamIn->GetVoIPRunningState())
                {
                    return true;
                }
            }
            return false;
        }
        else
        {
            for (size_t i = 0; i < mSPELayerVector.size() ; i++)
            {
                if (pMTKStreamIn == mSPELayerVector.keyAt(i))
                {
                    if (pMTKStreamIn->GetVoIPRunningState())
                    {
                        return true;
                    }
                }
            }
            return false;
        }
    }
    return false;
}

void AudioSpeechEnhanceInfo::GetDownlinkIntrStartTime(void)
{
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock lock(mInputInfoLock);
#else
    Mutex::Autolock lock(mHDRInfoLock);
#endif

    ALOGD("GetDownlinkIntrStartTime %d", mSPELayerVector.size());
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            SPELayer *pTempSPELayer = (SPELayer *)mSPELayerVector.valueAt(i);
            pTempSPELayer->GetDownlinkIntrStartTime();
        }
    }
}

void AudioSpeechEnhanceInfo::WriteReferenceBuffer(struct InBufferInfo *Binfo)
{
    //ALOGD("WriteReferenceBuffer +++");
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    //push the output data to echo reference
    mEffectLock.lock();
    if (mEcho_reference != NULL)
    {
        ALOGV("WriteReferenceBuffer echo_reference %p", mEcho_reference);
        uint32 consume = 0;
        size_t inputLength = Binfo->BufLen;
        size_t outputLength = mBliOutBufferSizeAndroidAEC;

        //ALOGD("WriteReferenceBuffer inputLength=%d, outputLength=%d",inputLength,outputLength);
        if (mBliHandlerAndroidAEC)   //do blisrc from 16K mono to 16K  stereo for android echo reference using
        {
            consume = inputLength;
            mBliHandlerAndroidAEC->Process((int16_t *)Binfo->pBufBase, &inputLength, (int16_t *)mBliBufferAndroidAEC, &outputLength);
            consume -= inputLength;
        }
        else
        {
            ALOGW("WriteReferenceBuffer no mBliHandlerAndroidAEC??");
            outputLength = inputLength;
            mBliBufferAndroidAEC = (char *)Binfo->pBufBase;
        }

        int fixchannel = 2; //due to android limitation in Echo_reference.c
        struct echo_reference_buffer b;
        b.raw = (void *)mBliBufferAndroidAEC;
        b.frame_count = outputLength / sizeof(int16_t) / fixchannel;
        //ALOGD(" b.frame_count=%d, outputLength=%d",b.frame_count,outputLength);

        get_playback_delay(b.frame_count, &b, Binfo->bHasRemainInfo, Binfo->time_stamp_predict);
        
        mEcho_reference->write(mEcho_reference, &b);
    }
    mEffectLock.unlock();
#endif

#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock lock(mInputInfoLock);
#else
    Mutex::Autolock lock(mHDRInfoLock);
#endif

    ALOGD("WriteReferenceBuffer %d", mSPELayerVector.size());
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            SPELayer *pTempSPELayer = (SPELayer *)mSPELayerVector.valueAt(i);
            pTempSPELayer->WriteReferenceBuffer(Binfo);
        }
    }
    ALOGD("WriteReferenceBuffer ---");
}

void AudioSpeechEnhanceInfo::NeedUpdateVoIPParams(void)
{
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    Mutex::Autolock lock(mInputInfoLock);
#else
    Mutex::Autolock lock(mHDRInfoLock);
#endif

    ALOGD("NeedUpdateVoIPParams %d", mSPELayerVector.size());
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            AudioMTKStreamIn *pTempMTKStreamIn = (AudioMTKStreamIn *)mSPELayerVector.keyAt(i);
            pTempMTKStreamIn->NeedUpdateVoIPParams();
        }
    }
}

int AudioSpeechEnhanceInfo::GetOutputBufferSize(void)
{
    ALOGD("%s()", __FUNCTION__);
    int BufferSize = 4096;
    int format = AUDIO_FORMAT_PCM_16_BIT;

    if (mStreamOut != NULL)
    {
        format = mStreamOut->format();
        BufferSize = mStreamOut->bufferSize();
        if (format == AUDIO_FORMAT_PCM_32_BIT)
        {
            BufferSize >>= 1;
        }
    }
    ALOGD("%s(),BufferSize=%d,format=%d", __FUNCTION__, BufferSize, format);
    return BufferSize;
}

void AudioSpeechEnhanceInfo::SetEnableNormalModeVoIP(bool bEnable)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetEnableNormalModeVoIP=%d", bEnable);
    mEnableNormalModeVoIP = bEnable;
}

bool AudioSpeechEnhanceInfo::GetEnableNormalModeVoIP(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetEnableNormalModeVoIP=%x", mEnableNormalModeVoIP);
    return mEnableNormalModeVoIP;
}

#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
void AudioSpeechEnhanceInfo::StartEchoReferenceThread(uint32_t DLdevice)
{
    if (mStreamInManager == NULL)
    {
        ALOGD("StartEchoReferenceThread AudioMTKStreamInManager::getInstance() ");
        mStreamInManager = AudioMTKStreamInManager::getInstance();
        if (mStreamInManager == NULL)
        {
            ALOGW("StartEchoReferenceThread get mStreamInManager fail!! ");
        }
    }

    if (mAudioResourceManager == NULL)
    {
        ALOGD("StartEchoReferenceThread AudioResourceManagerFactory::CreateAudioResource() ");
        mAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();
        if (mAudioResourceManager == NULL)
        {
            ALOGW("StartEchoReferenceThread get mAudioResourceManager fail!! ");
        }
    }

    if (mDLDevice != DLdevice)
    {
        ALOGD("StartEchoReferenceThread mDLDevice %x, DLdevice %x", mDLDevice, DLdevice);
        if (((DLdevice == AUDIO_DEVICE_OUT_SPEAKER) && (mDLDevice != AUDIO_DEVICE_OUT_SPEAKER)) ||
            ((mDLDevice == AUDIO_DEVICE_OUT_SPEAKER) && (DLdevice != AUDIO_DEVICE_OUT_SPEAKER)))
        {
            //need to restart echo reference thread due to path change
            if (mEchoRefThreadCreated)
            {
                StopEchoReferenceThread();
            }
        }
        mDLDevice = DLdevice;
    }

#if 1
    mInputInfoLock.lock();
    if (mEchoRefThreadCreated && (mSPELayerVector.size() <= 0))
    {
        mInputInfoLock.unlock();
        ALOGE("StartEchoReferenceThread stop echo thread due to no input");
        StopEchoReferenceThread();
    }
    else
    {
        mInputInfoLock.unlock();
    }
#endif

    mInputInfoLock.lock();
    if ((!mEchoRefThreadCreated) && (mSPELayerVector.size() > 0))
    {
        ALOGD("StartEchoReferenceThread + %d", mSPELayerVector.size());
        mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, 3000);
        //mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_ECHOREFERENCECLIENT_LOCK, 3000);
        mEchoRefClient = mStreamInManager->RequestClient();
        if (mEchoRefClient == NULL)
        {
            ALOGE("StartEchoReferenceThread mEchoRefThreadCreated = false cannot get mEchoRefClient");
        }

        //setup client info
        mEchoRefClient->mMemDataType = AudioDigitalType::MEM_AWB;
        if (mDLDevice == AUDIO_DEVICE_OUT_SPEAKER)
        {
#ifdef NXP_SMARTPA_SUPPORT
            mEchoRefClient->mSourceType = AudioDigitalType::I2S_IN_2;   //speaker path from NXP
            mEchoRefAttribute.mSampleRate = 44100;//echo reference data from I2S_IN_2 NXP is 44100
#else
            mEchoRefClient->mSourceType = AudioDigitalType::MEM_DL1; //for test
            mEchoRefAttribute.mSampleRate = GetOutputSampleRateInfo(); //echo reference data from DL1
#endif
        }
        else
        {
            mEchoRefClient->mSourceType = AudioDigitalType::MEM_DL1;      //receiver and headset path, need to restart when the path changed!!!
            mEchoRefAttribute.mSampleRate = GetOutputSampleRateInfo();  //echo reference data from DL1
        }
        mEchoRefAttribute.mChannels = AUDIO_CHANNEL_IN_STEREO;
        mEchoRefAttribute.mIsDigitalMIC = false;
        mEchoRefAttribute.mEchoRefUse = true;
        mEchoRefAttribute.mdevices = mDLDevice;
        mEchoRefClient->mAttributeClient = &mEchoRefAttribute; // set attribute with request client

        ALOGD("StartEchoReferenceThread Do_input_start");
        RequesetRecordclock();
        mStreamInManager->Do_input_start(mEchoRefClient);
        SetEchoRefStartTime(mEchoRefClient->mInputStartTime);
        mEchoRefThreadCreated = true;
        //mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_ECHOREFERENCECLIENT_LOCK);
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
    }

    mInputInfoLock.unlock();
}

void AudioSpeechEnhanceInfo::StopEchoReferenceThread(void)
{
    if (mStreamInManager == NULL)
    {
        ALOGD("StopEchoReferenceThread AudioMTKStreamInManager::getInstance() ");
        mStreamInManager = AudioMTKStreamInManager::getInstance();
        if (mStreamInManager == NULL)
        {
            ALOGW("StopEchoReferenceThread get mStreamInManager fail!! ");
        }
    }
    if (mAudioResourceManager == NULL)
    {
        ALOGD("StopEchoReferenceThread AudioResourceManagerFactory::CreateAudioResource() ");
        mAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();
        if (mAudioResourceManager == NULL)
        {
            ALOGW("StopEchoReferenceThread get mAudioResourceManager fail!! ");
        }
    }

    if (mEchoRefThreadCreated)
    {
        //need lock
        ALOGD("StopEchoReferenceThread +");
        mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, 3000);
        //mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_ECHOREFERENCECLIENT_LOCK, 3000);
        mStreamInManager->Do_input_standby(mEchoRefClient);
        mStreamInManager->ReleaseClient(mEchoRefClient);
        mEchoRefClient = NULL;//assign null value;
        mEchoRefThreadCreated = false;
        ReleaseRecordclock();
        //mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_ECHOREFERENCECLIENT_LOCK);
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
        NeedEchoRefResync();
        ALOGD("StopEchoReferenceThread ---");
    }
}

void AudioSpeechEnhanceInfo::CloseEchoReferenceThread(void)
{
    //ALOGD("CloseEchoReferenceThread +++");
    StopEchoReferenceThread();
    //ALOGD("CloseEchoReferenceThread ---");
}

status_t AudioSpeechEnhanceInfo::RequesetRecordclock()
{
    ALOGD("%s()+++", __FUNCTION__);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    ALOGD("%s()---", __FUNCTION__);
    return NO_ERROR;
}
status_t AudioSpeechEnhanceInfo::ReleaseRecordclock()
{
    ALOGD("%s()+++", __FUNCTION__);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
    ALOGD("%s()---", __FUNCTION__);
    return NO_ERROR;
}

void AudioSpeechEnhanceInfo::NeedEchoRefResync(void)
{
    ALOGD("NeedEchoRefResync %d", mSPELayerVector.size());
    Mutex::Autolock lock(mInputInfoLock);
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            AudioMTKStreamIn *pTempMTKStreamIn = (AudioMTKStreamIn *)mSPELayerVector.keyAt(i);
            pTempMTKStreamIn->NeedVoIPResync();
        }
    }
    ALOGD("NeedEchoRefResync ---");
}

void AudioSpeechEnhanceInfo::SetEchoRefStartTime(struct timespec EchoRefStartTime)
{
    ALOGD("SetEchoRefStartTime %d", mSPELayerVector.size());
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            SPELayer *pTempSPELayer = (SPELayer *)mSPELayerVector.valueAt(i);
            pTempSPELayer->SetEchoRefStartTime(EchoRefStartTime);
        }
    }
}

//for android AEC
int AudioSpeechEnhanceInfo::get_playback_delay(size_t frames, struct echo_reference_buffer *buffer, bool bIsAccuracyTime, struct timespec TimeInfo)
{
    struct timespec tstamp;
    size_t kernel_frames;

    if (bIsAccuracyTime)
    {
        buffer->delay_ns = 0;
        buffer->time_stamp = TimeInfo;
    }
    else
    {
        /* adjust render time stamp with delay added by current driver buffer.
         * Add the duration of current frame as we want the render time of the last
         * sample being written. */
        buffer->delay_ns = (long)(((int64_t)(frames) * 1000000000) / ECHOREF_SAMPLE_RATE);

        buffer->time_stamp = TimeInfo;
    }
    
#if 0
    //FIXME:: calculate for more precise time delay

    int rc = clock_gettime(CLOCK_MONOTONIC, &tstamp);
    if (rc != 0)
    {
        buffer->time_stamp.tv_sec  = 0;
        buffer->time_stamp.tv_nsec = 0;
        buffer->delay_ns           = 0;
        ALOGW("get_playback_delay(): pcm_get_htimestamp error,"
              "setting playbackTimestamp to 0");
        return 0;
    }

    /* adjust render time stamp with delay added by current driver buffer.
     * Add the duration of current frame as we want the render time of the last
     * sample being written. */
    buffer->delay_ns = (long)(((int64_t)(frames) * 1000000000) /
                              16000); //mDL1Attribute->mSampleRate);
    //need to check this
    buffer->delay_ns = 0;
    buffer->time_stamp = tstamp;

    //ALOGD("get_playback_delay time_stamp = [%ld].[%ld], delay_ns: [%d]",buffer->time_stamp.tv_sec , buffer->time_stamp.tv_nsec, buffer->delay_ns);
#endif
    return 0;
}

#endif
}

