#include "AudioMTKHardware.h"
#include "AudioVolumeFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioDigitalControlFactory.h"

#include "SpeechDriverInterface.h"
#include "SpeechDriverFactory.h"
#include "SpeechEnhancementController.h"
#include "SpeechPhoneCallController.h"
#include "SpeechVMRecorder.h"

#include "LoopbackManager.h"

#include "AudioFMController.h"
#include "AudioMATVController.h"
#include "WCNChipController.h"


#include <media/AudioSystem.h>

#include <binder/IServiceManager.h>
#include <media/IAudioPolicyService.h>
#include <AudioMTKPolicyManager.h>
#include <DfoDefines.h>

#ifdef MTK_AUDIO_HD_REC_SUPPORT
#include "AudioCustParam.h"
#endif
#include "AudioVUnlockDL.h"

//#if defined(MTK_VIBSPK_SUPPORT)
#include "AudioVIBSPKControl.h"
//#endif


#define LOG_TAG "AudioMTKHardware"
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


#define DL1_BUFFER_SIZE (0x4000)
#define DL2_BUFFER_SIZE (0x4000)
#define AWB_BUFFER_SIZE (0x4000)
#define VUL_BUFFER_SIZE (0x4000)

// for 16k samplerate  below , 8K buffer should be enough
#define DAI_BUFFER_SIZE (0x2000)
#define MOD_DAI_BUFFER_SIZE (0x2000)

#define AUXADC_HP_L_CHANNEL 15
#define AUXADC_HP_R_CHANNEL 14

namespace android
{

/*==============================================================================
 *                     setParameters() keys
 *============================================================================*/
//enable using MTK VoIP in normal mode, otherwise will only using in in-communication mode or input source is in-communication
static String8 keyEnableNormalModeVoIP = String8("EnableNormalModeVoIP");


/*==============================================================================
 *                     Emulator
 *============================================================================*/
enum
{
    Normal_Coef_Index,
    Headset_Coef_Index,
    Handfree_Coef_Index,
    VOIPBT_Coef_Index,
    VOIPNormal_Coef_Index,
    VOIPHandfree_Coef_Index,
    AUX1_Coef_Index,
    AuX2_Coef_Index
};


/*==============================================================================
 *                     Property keys
 *============================================================================*/


/*==============================================================================
 *                     Function Implementation
 *============================================================================*/

#include "AudioMTKHardwareCommonCommand.cpp"

bool AudioMTKHardware::IsOutPutStreamActive()
{
    return mAudioMTKStreamManager->IsOutPutStreamActive();
}

bool AudioMTKHardware::IsInPutStreamActive()
{
    return mAudioMTKStreamManager->IsInPutStreamActive();
}

status_t AudioMTKHardware::dumpState(int fd, const Vector<String16> &args)
{
    return NO_ERROR;
}

status_t AudioMTKHardware::dump(int fd, const Vector<String16> &args)
{
    return NO_ERROR;
}

status_t AudioMTKHardware::HardwareInit(bool bEnableSpeech)
{
    mFd = 0;
    mHardwareInit = false;
    mMode = AUDIO_MODE_NORMAL;

    ALOGD("HardwareInit +");
    mFd = ::open(kAudioDeviceName, O_RDWR);
    if (mFd == 0)
    {
        ALOGE("AudioMTKHardware contrcutor open mfd fail");
    }
    //if mediaerver died , aud driver should do recoevery.
    ::ioctl(mFd, AUD_RESTART, 0);

    mAudioResourceManager = AudioResourceManager::getInstance();
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

    mAudioFtmInstance = AudioFtm::getInstance();
    mAudioMTKStreamManager = AudioMTKStreamManager::getInstance();

    // create volume instance
    mAudioVolumeInstance = AudioVolumeFactory::CreateAudioVolumeController();
    mAudioVolumeInstance->initCheck();
    // create digital control instnace
    mAudioDigitalInstance  = AudioDigitalControlFactory::CreateAudioDigitalControl();
    mAudioDigitalInstance->InitCheck();
    // create digital control instnace
    mAudioAnalogInstance  = AudioAnalogControlFactory::CreateAudioAnalogControl();
    mAudioAnalogInstance->InitCheck();
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, 44100);

    //allocate buffer when system is boot up
    mAudioDigitalInstance->SetMemBufferSize(AudioDigitalType::MEM_DL1, DL1_BUFFER_SIZE);
    mAudioDigitalInstance->SetMemBufferSize(AudioDigitalType::MEM_DL2, DL2_BUFFER_SIZE);
    mAudioDigitalInstance->SetMemBufferSize(AudioDigitalType::MEM_AWB, AWB_BUFFER_SIZE);
    mAudioDigitalInstance->SetMemBufferSize(AudioDigitalType::MEM_VUL, VUL_BUFFER_SIZE);
#if 0
    mAudioDigitalInstance->SetMemBufferSize(AudioDigitalType::MEM_DAI, DAI_BUFFER_SIZE);
#endif
    mAudioDigitalInstance->SetMemBufferSize(AudioDigitalType::MEM_MOD_DAI, MOD_DAI_BUFFER_SIZE);

    //allocate buffer when system is boot up
    mAudioDigitalInstance->AllocateMemBufferSize(AudioDigitalType::MEM_DL1);
    mAudioDigitalInstance->AllocateMemBufferSize(AudioDigitalType::MEM_DL2);
    mAudioDigitalInstance->AllocateMemBufferSize(AudioDigitalType::MEM_AWB);
    mAudioDigitalInstance->AllocateMemBufferSize(AudioDigitalType::MEM_VUL);
#if 0
    mAudioDigitalInstance->AllocateMemBufferSize(AudioDigitalType::MEM_DAI);
#endif
    mAudioDigitalInstance->AllocateMemBufferSize(AudioDigitalType::MEM_MOD_DAI);
    if (bEnableSpeech == false)
    {
        // no need to enable speech drivers
        mSpeechDriverFactory = NULL;
    }
    else
    {
        // first time to get speech driver factory, which will create speech dirvers
        mSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    }

    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    memset((void *)&mAudio_Control_State, 0, sizeof(SPH_Control));

    //if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
    {
        mAudioBTCVSDControl = NULL;
        mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
        if (!mAudioBTCVSDControl)
        {
            ALOGE("HardwareInit AudioBTCVSDControl::getInstance() fail");
        }
        mAudioBTCVSDControl->BT_SCO_CVSD_Init();
    }

    mHardwareInit = true;

    //DC Calibration
    GetDefaultDcCalibrationParam(&mDcCaliParam);
    DcCalibrationProcess(&mDcCaliParam);
    SetDcCalibration(&mDcCaliParam);

    // parameters calibrations instance
    mAudioTuningInstance = AudioParamTuning::getInstance();

    // default not mute mic
    mMicMute = false;

    int ret = 0;
    ret |= pthread_mutex_init(&setParametersMutex, NULL);
    if (ret != 0) { ALOGE("Failed to initialize pthread setParametersMutex"); }
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
    ALOGD("HardwareInit -");
    return NO_ERROR;
}

AudioMTKHardware::AudioMTKHardware()
{
    ALOGD("AudioMTKHardware create +\n");
    HardwareInit(true);
    ALOGD("AudioMTKHardware create -\n");
}

AudioMTKHardware::AudioMTKHardware(bool SpeechControlEnable)
{
    HardwareInit(SpeechControlEnable);
}

AudioMTKHardware::~AudioMTKHardware()
{
    if (mSpeechDriverFactory != NULL)
    {
        delete mSpeechDriverFactory;
        mSpeechDriverFactory = NULL;
    }

    if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
    {
        if (mAudioBTCVSDControl)
        {
            mAudioBTCVSDControl->BT_SCO_CVSD_DeInit();
            AudioBTCVSDControl::freeInstance();;
            mAudioBTCVSDControl = NULL;
        }
    }
}

status_t AudioMTKHardware::setParameters(const String8 &keyValuePairs)
{
    bool bPlatformCMD = true;
    status_t status = NO_ERROR;
    int value = 0;
    //String8 value_str;
    float value_float = 0.0;
    pthread_mutex_lock(&setParametersMutex);
    AudioParameter param = AudioParameter(keyValuePairs);
    ALOGD("+setParameters(): %s ", keyValuePairs.string());
    do
    {
        //#if defined(MTK_VIBSPK_SUPPORT)
        // Vibration Speaker Enable/Disable
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
        {
            if (param.getInt(keySET_VIBSPK_ENABLE, value) == NO_ERROR)
            {
                param.remove(keySET_VIBSPK_ENABLE);
                AudioVIBSPKControl::getInstance()->setVibSpkEnable((bool)value);
                ALOGD("setParameters VibSPK!!, %x", value);
                break;
            }
            if (param.getInt(keySET_VIBSPK_RAMPDOWN, value) == NO_ERROR)
            {
                param.remove(keySET_VIBSPK_RAMPDOWN);
                mAudioFtmInstance->SetVibSpkRampControl(value);
                ALOGD("setParameters VibSPK_Rampdown!!, %x", value);
                break;
            }
        }
        //#endif

        //Add PlatformDependency , overwite common command
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        if (param.getInt(keyEnableNormalModeVoIP, value) == NO_ERROR)
        {
            ALOGD("EnableNormalModeVoIP=%d", value);
            param.remove(keyEnableNormalModeVoIP);
            bool bEnableNormalModeVoIP = value;
            mAudioSpeechEnhanceInfoInstance->SetEnableNormalModeVoIP(bEnableNormalModeVoIP);
            break;
        }
#endif


#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
        if (param.getInt(keyDualMicRecPly, value) == NO_ERROR)
        {
            unsigned short cmdType = value & 0x000F;
            bool bWB = (value >> 4) & 0x000F;
            status_t ret = NO_ERROR;
            switch (cmdType)
            {
#ifdef DMNR_TUNNING_AT_MODEMSIDE
                case DUAL_MIC_REC_PLAY_STOP:
                    ret = mAudioTuningInstance->enableDMNRModem2Way(false, bWB, P2W_RECEIVER_OUT, P2W_NORMAL);
                    break;
                case DUAL_MIC_REC:
                    ret = mAudioTuningInstance->enableDMNRModem2Way(true, bWB, P2W_RECEIVER_OUT, P2W_RECONLY);
                    break;
                case DUAL_MIC_REC_PLAY:
                    ret = mAudioTuningInstance->enableDMNRModem2Way(true, bWB, P2W_RECEIVER_OUT, P2W_NORMAL);
                    break;
                case DUAL_MIC_REC_PLAY_HS:
                    ret = mAudioTuningInstance->enableDMNRModem2Way(true, bWB, P2W_HEADSET_OUT, P2W_NORMAL);
                    break;
#else//dmnr tunning at ap side
                case DUAL_MIC_REC_PLAY_STOP:
                    ret = mAudioTuningInstance->enableDMNRAtApSide(false, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_MODE);
                    mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(false);
                    break;
                case DUAL_MIC_REC:
                    mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                    ret = mAudioTuningInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECONLY_MODE);
                    break;
                case DUAL_MIC_REC_PLAY:
                    mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                    ret = mAudioTuningInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_MODE);
                    break;
                case DUAL_MIC_REC_PLAY_HS:
                    mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                    ret = mAudioTuningInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_HEADSET, RECPLAY_MODE);
                    break;
                case DUAL_MIC_REC_HF:
                    mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                    ret = mAudioTuningInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECONLY_HF_MODE);
                    break;
                case DUAL_MIC_REC_PLAY_HF:
                    mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                    ret = mAudioTuningInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_HF_MODE);
                    break;
                case DUAL_MIC_REC_PLAY_HF_HS:
                    mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                    ret = mAudioTuningInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_HEADSET, RECPLAY_HF_MODE);
                    break;
#endif
                default:
                    ret = BAD_VALUE;
                    break;
            }
            if (ret == NO_ERROR)
            {
                param.remove(keyDualMicRecPly);
            }
            break;
        }
#endif
        // --->for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration)
        // Used for debug and Speech DVT
        if (param.getInt(String8("HAHA"), value) == NO_ERROR)
        {
            param.remove(String8("HAHA"));
            SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
            switch (value)
            {
                case 0:
                {
                    pSpeechDriver->PCM2WayOff();

                    Play2Way::GetInstance()->Stop();
                    Record2Way::GetInstance()->Stop();

                    setMode(android_audio_legacy::AudioSystem::MODE_NORMAL);
                    break;
                }
                case 1:
                {
                    setMode(android_audio_legacy::AudioSystem::MODE_IN_CALL);

                    Play2Way::GetInstance()->Start();
                    Record2Way::GetInstance()->Start();

                    pSpeechDriver->PCM2WayOn(false);

                    break;
                }
            }
            break;
        }

#if defined(MTK_DUAL_MIC_SUPPORT)
        // Dual Mic Noise Reduction, DMNR for Receiver
        if (param.getInt(keyEnable_Dual_Mic_Setting, value) == NO_ERROR)
        {
            param.remove(keyEnable_Dual_Mic_Setting);
            SpeechEnhancementController::GetInstance()->SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_DMNR, (bool)value);
            break;
        }

        // Dual Mic Noise Reduction, DMNR for Loud Speaker
        if (param.getInt(keySET_LSPK_DMNR_ENABLE, value) == NO_ERROR)
        {
            param.remove(keySET_LSPK_DMNR_ENABLE);
            SpeechEnhancementController::GetInstance()->SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR, (bool)value);
            if (SpeechEnhancementController::GetInstance()->GetMagicConferenceCallOn() == true &&
                SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) == true)
            {
                ALOGE("Cannot open MagicConCall & LoudSpeaker DMNR at the same time!!");
            }

            break;
        }

#endif

        bPlatformCMD = false;
    }
    while (0);

    if (bPlatformCMD)
    {
        if (param.size())
        {
            ALOGE("%s() still have param.size() = %d, remain param = \"%s\"", __FUNCTION__, param.size(), param.toString().string());
            status = BAD_VALUE;
        }
        ALOGD("-setParameters(): %s ", keyValuePairs.string());
    }
    else
    {
        status = setCommonParameters(keyValuePairs);
    }

    pthread_mutex_unlock(&setParametersMutex);
    return status;
}

String8 AudioMTKHardware::getParameters(const String8 &keys)
{
    ALOGD("+%s(), keys = %s", __FUNCTION__, keys.string());
    bool bPlatformCMD = true;
    String8 value;
    AudioParameter param = AudioParameter(keys);
    AudioParameter returnParam = AudioParameter();

    do
    {
        //Add PlatformDependency

        bPlatformCMD = false;

    }
    while (0);

    if (bPlatformCMD)
    {
        String8 keyValuePairs = returnParam.toString();
        ALOGD("-%s(), keyValuePairs = %s", __FUNCTION__, keyValuePairs.string());
        return keyValuePairs;
    }
    else
    {
        return getCommonParameters(param, returnParam);
    }
}

size_t AudioMTKHardware::getInputBufferSize(uint32_t sampleRate, int format, int channelCount)
{
    return mAudioMTKStreamManager->getInputBufferSize(sampleRate, format, channelCount);
}

status_t AudioMTKHardware::SetEMParameter(void *ptr, int len)
{
    ALOGD("%s() len [%d] sizeof [%d]", __FUNCTION__,len,sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

    if (len == sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
    {
        AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB = (AUDIO_CUSTOM_PARAM_STRUCT *)ptr;

        SetNBSpeechParamToNVRam(pSphParamNB);
        SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(pSphParamNB);

        // update VM/EPL/TTY record capability & enable if needed
        SpeechVMRecorder::GetInstance()->SetVMRecordCapability(pSphParamNB);

        SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
        if (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == true)
        {
            pSpeechDriver->TtyCtmDebugOn(SpeechVMRecorder::GetInstance()->GetVMRecordCapabilityForCTM4Way());
        }

        return NO_ERROR;
    }
    else
    {
        ALOGE("len [%d] != Sizeof(AUDIO_CUSTOM_PARAM_STRUCT) [%d] ",len,sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        return UNKNOWN_ERROR;
    }
}

status_t AudioMTKHardware::GetEMParameter(void *ptr , int len)
{
    ALOGD("%s() len [%d] sizeof [%d]", __FUNCTION__,len,sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

    if (len == sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
    {
        GetNBSpeechParamFromNVRam((AUDIO_CUSTOM_PARAM_STRUCT *)ptr);    
        return NO_ERROR;
    }
    else
    {
        ALOGE("len [%d] != Sizeof(AUDIO_CUSTOM_PARAM_STRUCT) [%d] ",len,sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        return UNKNOWN_ERROR;
    }
}


bool AudioMTKHardware::UpdateOutputFIR(int mode , int index)
{
    ALOGD("%s(),  mode = %d, index = %d", __FUNCTION__, mode, index);

    // save index to MED with different mode.
    AUDIO_PARAM_MED_STRUCT eMedPara;
    GetMedParamFromNV(&eMedPara);
    eMedPara.select_FIR_output_index[mode] = index;

    // copy med data into audio_custom param
    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    GetNBSpeechParamFromNVRam(&eSphParamNB);

    for (int i = 0; i < NB_FIR_NUM;  i++)
    {
        ALOGD("eSphParamNB.sph_out_fir[%d][%d] = %d <= eMedPara.speech_output_FIR_coeffs[%d][%d][%d] = %d",
              mode, i, eSphParamNB.sph_out_fir[mode][i],
              mode, index, i, eMedPara.speech_output_FIR_coeffs[mode][index][i]);
    }

    memcpy((void *)eSphParamNB.sph_out_fir[mode],
           (void *)eMedPara.speech_output_FIR_coeffs[mode][index],
           sizeof(eSphParamNB.sph_out_fir[mode]));

    // set to nvram
    SetNBSpeechParamToNVRam(&eSphParamNB);
    SetMedParamToNV(&eMedPara);

    // set to modem side
    SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(&eSphParamNB);

    return true;
}

status_t AudioMTKHardware::SetAudioCommand(int par1, int par2)
{
    ALOGD("%s(), par1 = 0x%x, par2 = %d", __FUNCTION__, par1, par2);
    char value[PROPERTY_VALUE_MAX];
    int dIsCommonCommand = 0;
    switch (par1)
    {
        case AUDIO_USER_TEST:
        {
            if (par2 == 0x100)
            {
                AudioFMController::GetInstance()->SetFmEnable(true);
            }
            else if (par2 == 0x101)
            {
                AudioFMController::GetInstance()->SetFmEnable(false);
            }
            else
            {
                dIsCommonCommand = 1;
            }

            break;

        }
        case SET_CUREENT_SENSOR_ENABLE:
        {
            mAudioAnalogInstance->setParameters(AudioAnalogType::SET_CURRENT_SENSING, (void *)par2);
            break;
        }
        case SET_CURRENT_SENSOR_RESET:
        {
            mAudioAnalogInstance->setParameters(AudioAnalogType::SET_CURRENT_SENSING_PEAK_DETECTOR, (void *)par2);
            break;
        }
        default:
            dIsCommonCommand = 1;
            break;
    }
    if (dIsCommonCommand)
    {
        return SetAudioCommonCommand(par1, par2);
    }
    else
    {
        return NO_ERROR;
    }
}

status_t AudioMTKHardware::GetAudioCommand(int parameters1)
{
    ALOGD("GetAudioCommand parameters1 = %d ", parameters1);
    int result = 0 ;
    int dIsCommonCommand = 0;
    switch (parameters1) //assign result
    {
        default:
            ALOGD(" GetAudioCommand: Unknown command\n");
            dIsCommonCommand = 1;
            break;
    }
    if (dIsCommonCommand)
    {
        return GetAudioCommonCommand(parameters1);
    }
    else
    {
        return result;
    }
}


status_t AudioMTKHardware::SetAudioData(int par1, size_t len, void *ptr)
{
    ALOGD("%s(), par1 = 0x%x, len = %d", __FUNCTION__, par1, len);
    int dIsCommonCommand = 0;
    switch (par1)
    {
        case HOOK_FM_DEVICE_CALLBACK:
        {
            AudioFMController::GetInstance()->SetFmDeviceCallback((AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT *)ptr);
            break;
        }
        case UNHOOK_FM_DEVICE_CALLBACK:
        {
            AudioFMController::GetInstance()->SetFmDeviceCallback(NULL);
            break;
        }
        default:
            dIsCommonCommand = 1;
            break;
    }

    if (dIsCommonCommand)
    {
        return SetAudioCommonData(par1, len, ptr);
    }
    else
    {
        return NO_ERROR;
    }
}


status_t AudioMTKHardware::GetAudioData(int par1, size_t len, void *ptr)
{
    ALOGD("%s(), par1 = 0x%x, len = %d", __FUNCTION__, par1, len);
    int dIsCommonCommand = 0;
    switch (par1)
    {
        default:
            dIsCommonCommand = 1;
            break;
    }

    if (dIsCommonCommand)
    {
        return GetAudioCommonData(par1, len, ptr);
    }
    else
    {
        return NO_ERROR;
    }
}


// ACF Preview parameter
status_t AudioMTKHardware::SetACFPreviewParameter(void *ptr , int len)
{
    ALOGD("AudioMTKHardware SetACFPreviewParameter\n");
    mAudioMTKStreamManager->SetACFPreviewParameter(ptr, len);
    return NO_ERROR;
}
status_t AudioMTKHardware::SetHCFPreviewParameter(void *ptr , int len)
{
    ALOGD("AudioMTKHardware SetHCFPreviewParameter\n");
    mAudioMTKStreamManager->SetHCFPreviewParameter(ptr, len);
    return NO_ERROR;
}

//for PCMxWay Interface API
int AudioMTKHardware::xWayPlay_Start(int sample_rate)
{
    ALOGV("AudioMTKHardware xWayPlay_Start");
    return Play2Way::GetInstance()->Start();
}

int AudioMTKHardware::xWayPlay_Stop(void)
{
    ALOGV("AudioMTKHardware xWayPlay_Stop");
    return Play2Way::GetInstance()->Stop();
}

int AudioMTKHardware::xWayPlay_Write(void *buffer, int size_bytes)
{
    ALOGV("AudioMTKHardware xWayPlay_Write");
    return Play2Way::GetInstance()->Write(buffer, size_bytes);
}

int AudioMTKHardware::xWayPlay_GetFreeBufferCount(void)
{
    ALOGV("AudioMTKHardware xWayPlay_GetFreeBufferCount");
    return Play2Way::GetInstance()->GetFreeBufferCount();
}

int AudioMTKHardware::xWayRec_Start(int sample_rate)
{
    ALOGV("AudioMTKHardware xWayRec_Start");
    return Record2Way::GetInstance()->Start();
}

int AudioMTKHardware::xWayRec_Stop(void)
{
    ALOGV("AudioMTKHardware xWayRec_Stop");
    return Record2Way::GetInstance()->Stop();
}

int AudioMTKHardware::xWayRec_Read(void *buffer, int size_bytes)
{
    ALOGV("AudioMTKHardware xWayRec_Read");
    return Record2Way::GetInstance()->Read(buffer, size_bytes);
}

//add by wendy
int AudioMTKHardware::ReadRefFromRing(void *buf, uint32_t datasz, void *DLtime)
{
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->ReadRefFromRing(buf, datasz, DLtime);
}
int AudioMTKHardware::GetVoiceUnlockULTime(void *DLtime)
{
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->GetVoiceUnlockULTime(DLtime);
}

int AudioMTKHardware::SetVoiceUnlockSRC(uint outSR, uint outChannel)
{
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->SetSRC(outSR, outChannel);
}
bool AudioMTKHardware::startVoiceUnlockDL()
{
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->startInput();
}
bool AudioMTKHardware::stopVoiceUnlockDL()
{
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->stopInput();
}
void AudioMTKHardware::freeVoiceUnlockDLInstance()
{
    AudioVUnlockDL::freeInstance();
    return ;
}
bool AudioMTKHardware::getVoiceUnlockDLInstance()
{
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    if (VInstance != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}
int AudioMTKHardware::GetVoiceUnlockDLLatency()
{
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->GetLatency();
}
status_t AudioMTKHardware::initCheck()
{
    ALOGD("AudioMTKHardware initCheck\n");
    return NO_ERROR;
}

status_t AudioMTKHardware::setVoiceVolume(float volume)
{
    ALOGD("AudioMTKHardware setVoiceVolume volume = %f\n", volume);
    if ((volume > 1) || (volume < 0))
    {
        ALOGE("AudioMTKHardware setVoiceVolume strange volume level, something wrong!!\n");
        return NO_ERROR;
    }
    mAudioVolumeInstance->setVoiceVolume(volume, mMode, mAudioResourceManager->getDlOutputDevice());
    return NO_ERROR;
}

status_t AudioMTKHardware::setMasterVolume(float volume)
{
    ALOGD("AudioMTKHardware setMasterVolume volume = %f", volume);
    if ((volume > 1) || (volume < 0))
    {
        ALOGE("AudioMTKHardware setMasterVolume strange volume level, something wrong!!\n");
        return NO_ERROR;
    }
    mAudioVolumeInstance->setMasterVolume(volume, mMode, mAudioResourceManager->getDlOutputDevice());
    return NO_ERROR;
}

status_t AudioMTKHardware::ForceAllStandby()
{
    mAudioMTKStreamManager->ForceAllStandby();
    return NO_ERROR;
}

status_t AudioMTKHardware::SetOutputSuspend(bool bEnable)
{
    ALOGD("SetOutputSuspend bEnable = %d", bEnable);
    mAudioMTKStreamManager->SetOutputStreamSuspend(bEnable);
    return NO_ERROR;
}
status_t AudioMTKHardware::SetInputSuspend(bool bEnable)
{
    ALOGD("SetInputSuspend bEnable = %d", bEnable);
    mAudioMTKStreamManager->SetInputStreamSuspend(bEnable);
    return NO_ERROR;
}


bool AudioMTKHardware::ModeInCall(audio_mode_t mode)
{
    return (mode == AUDIO_MODE_IN_CALL ||
            mode == AUDIO_MODE_IN_CALL_2 ||
            mode == AUDIO_MODE_IN_CALL_EXTERNAL);
}

bool AudioMTKHardware::ModeEnterCall(audio_mode_t Mode)
{
    return ((mMode == AUDIO_MODE_NORMAL ||
             mMode == AUDIO_MODE_RINGTONE ||
             mMode == AUDIO_MODE_IN_COMMUNICATION) &&
            (ModeInCall(Mode) == true));
}

bool AudioMTKHardware::ModeLeaveCall(audio_mode_t Mode)
{
    return ((ModeInCall(mMode)) &&
            (Mode  == AUDIO_MODE_NORMAL ||
             Mode  == AUDIO_MODE_RINGTONE ||
             Mode  == AUDIO_MODE_IN_COMMUNICATION));
}
bool AudioMTKHardware::ModeEnterSipCall(audio_mode_t Mode)
{
    return (mMode != AUDIO_MODE_IN_COMMUNICATION &&
            Mode  == AUDIO_MODE_IN_COMMUNICATION);
}

bool AudioMTKHardware::ModeLeaveSipCall(audio_mode_t Mode)
{
    return (mMode == AUDIO_MODE_IN_COMMUNICATION &&
            Mode  != AUDIO_MODE_IN_COMMUNICATION);
}


bool AudioMTKHardware::ModeCallSwitch(audio_mode_t Mode)
{
    return ModeInCall(Mode) && ModeInCall(mMode) && mMode != Mode;
}

bool AudioMTKHardware::InputStreamExist()
{
    return mAudioMTKStreamManager->InputStreamExist();
}

bool AudioMTKHardware::OutputStreamExist()
{
    return mAudioMTKStreamManager->OutputStreamExist();
}

void AudioMTKHardware::UpdateKernelState()
{
    ::ioctl(mFd, SET_AUDIO_STATE, &mAudio_Control_State);
}

bool AudioMTKHardware::StreamExist()
{
    return mAudioMTKStreamManager->StreamExist();
}

status_t AudioMTKHardware::setMode(int NewMode)
{
    audio_mode_t new_mode = (audio_mode_t)NewMode;
    ALOGD("+%s(), mode = %d", __FUNCTION__, new_mode);

    if ((new_mode < 0) || (new_mode > AUDIO_MODE_MAX))
    {
        return BAD_VALUE;
    }

    if (MTK_ENABLE_MD1 == false && MTK_ENABLE_MD2 == true) // CTS want to set MODE_IN_CALL, but only modem 2 is available
    {
        if (new_mode == AUDIO_MODE_IN_CALL)
        {
            ALOGE("There is no modem 1 in this project!! Set modem 2 instead!!");
            new_mode = AUDIO_MODE_IN_CALL_2;
        }
    }
    else if (MTK_ENABLE_MD1 == false && MTK_ENABLE_MD2 == false) // CTS want to set MODE_IN_CALL, but wifi only project has no modem, just bypass it
    {
#ifdef EVDO_DT_SUPPORT
        if (new_mode == AUDIO_MODE_IN_CALL)
        {
            ALOGE("There is no modem 1 and no modem2 in this project!! Set modem external instead!!");
            new_mode = AUDIO_MODE_IN_CALL_EXTERNAL;
        }
#endif
        if (new_mode == AUDIO_MODE_IN_CALL)
        {
            if (MTK_ENABLE_MD5 == true)
            {
                ALOGE("There is no modem 1 & modem 2 in this project!! Set AUDIO_MODE_IN_CALL_EXTERNAL LTE instead!!");
                new_mode = AUDIO_MODE_IN_CALL_EXTERNAL;
            }
        }
        if (new_mode == AUDIO_MODE_IN_CALL)
        {
            ALOGE("There is no modem 1 & modem_x in this project!! Just bypass AUDIO_MODE_IN_CALL!!");
            return NO_ERROR;
        }
    }

    if (new_mode == mMode)
    {
        ALOGE("Newmode and Oldmode is the same!!!!");
        return INVALID_OPERATION;
    }


    // check input/output need suspend
    if (ModeEnterCall(new_mode) || ModeCallSwitch(new_mode) || ModeLeaveCall(new_mode) || ModeEnterSipCall(new_mode) || ModeLeaveSipCall(new_mode))
    {
        SetOutputSuspend(true);
        SetInputSuspend(true);
    }


    // close FM when mode swiching
    AudioFMController *pAudioFMController = AudioFMController::GetInstance();
    if (pAudioFMController->GetFmEnable() == true)
    {
        pAudioFMController->SetFmEnable(false);
    }

    // close mATV when mode swiching
    AudioMATVController *pAudioMATVController = AudioMATVController::GetInstance();
    if (pAudioMATVController->GetMatvEnable() == true)
    {
        pAudioMATVController->SetMatvEnable(false);
    }

    // Lock
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_VOLUME_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);

    SpeechPhoneCallController *pSpeechPhoneCallController = SpeechPhoneCallController::GetInstance();

    // check input/output need standby
    if (ModeEnterCall(new_mode) || ModeCallSwitch(new_mode) || ModeLeaveCall(new_mode) || ModeEnterSipCall(new_mode) || ModeLeaveSipCall(new_mode))
    {
        ForceAllStandby();
    }

    if (ModeEnterCall(new_mode))
    {
        mAudio_Control_State.bSpeechFlag = true;
        UpdateKernelState();
    }

    // set to AudioReourceManager and get input and output device
    mAudioResourceManager->SetAudioMode(new_mode);

    // mMode is previous
    switch (mMode)
    {
        case AUDIO_MODE_NORMAL:
        {
            switch (new_mode)
            {
                case AUDIO_MODE_RINGTONE:         // Normal->Ringtone: MT call incoming. [but not accept yet]
                    break;
                case AUDIO_MODE_IN_CALL:          // Normal->Incall:  MD1 MO call dial out
                case AUDIO_MODE_IN_CALL_2:        // Normal->Incall2: MD2 MO call dial out
                case AUDIO_MODE_IN_CALL_EXTERNAL:     //  Normal->Incall External: MD External MO call dial out
                {
                    pSpeechPhoneCallController->OpenModemSpeechControlFlow(new_mode);
                    break;
                }
                case AUDIO_MODE_IN_COMMUNICATION: // Normal->Incommunication: SIP MO call dial out
                    break;
            }
            break;
        }
        case AUDIO_MODE_RINGTONE:
        {
            switch (new_mode)
            {
                case AUDIO_MODE_NORMAL:           // Ringtone->Normal: MT call incoming, and reject. [no other call connected]
                    break;
                case AUDIO_MODE_IN_CALL:          // Ringtone->Incall:  Accept MD1 MT call
                case AUDIO_MODE_IN_CALL_2:        // Ringtone->Incall2: Accept MD2 MT call
                case AUDIO_MODE_IN_CALL_EXTERNAL:    // Ringtone->Incall External: Accept MD External MT call
                {
                    pSpeechPhoneCallController->OpenModemSpeechControlFlow(new_mode);
                    break;
                }
                case AUDIO_MODE_IN_COMMUNICATION: // Ringtone->Incommunication: Accept SIP MT call
                    break;
            }
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2:
        case AUDIO_MODE_IN_CALL_EXTERNAL:
        {
            switch (new_mode)
            {
                case AUDIO_MODE_NORMAL:           // Incall_x->Normal: Hang up MDx call. [no other call connected]
                    pSpeechPhoneCallController->CloseModemSpeechControlFlow(mMode);
                    break;
                case AUDIO_MODE_RINGTONE:         // Incall_x->Ringtone: Accept another MT call
                    pSpeechPhoneCallController->CloseModemSpeechControlFlow(mMode);
                    break;
                case AUDIO_MODE_IN_CALL:          // Incall2 or Incall External->Incall : MD1 dail out & hold MD2 or MD External, or Hang up MD2 or MD External  and back to MD1
                case AUDIO_MODE_IN_CALL_2:        // Incall or Incall External ->Incall2: MD2 dail out & hold MD1 or MD External, or Hang up MD1 or MD External and back to MD2
                case AUDIO_MODE_IN_CALL_EXTERNAL:     // Incall or Incall2  ->Incall External: MD External dail out & hold MD1 or MD2, or Hang up MD1 or MD2 and back to MD External
                {
                    pSpeechPhoneCallController->CloseModemSpeechControlFlow(mMode);
                    pSpeechPhoneCallController->OpenModemSpeechControlFlow(new_mode);
                    break;
                }
                case AUDIO_MODE_IN_COMMUNICATION: // Incall_x->Incommunication: SIP call dail out & Hold MDx, or Hang up MDx and back to SIP call
                    pSpeechPhoneCallController->CloseModemSpeechControlFlow(mMode);
                    break;
            }
            break;
        }
        case AUDIO_MODE_IN_COMMUNICATION:
        {
            switch (new_mode)
            {
                case AUDIO_MODE_NORMAL:           // Incommunication->Normal: Hang  up SIP call. [no other call connected]
                    break;
                case AUDIO_MODE_RINGTONE:         // Incommunication->Ringtone: Accept another MT call
                    break;
                case AUDIO_MODE_IN_CALL:          // Incommunication->Incall: MD1 Dail out & Hold SIP call, or Hang up SIP call and back to MD1
                case AUDIO_MODE_IN_CALL_2:        // Incommunication->Incall2: MD2 Dail out & Hold SIP call, or Hang up SIP call and back to MD2
                case AUDIO_MODE_IN_CALL_EXTERNAL:        // Incommunication->Incall External: MD External Dail out & Hold SIP call, or Hang up SIP call and back to MD External
                {
                    pSpeechPhoneCallController->OpenModemSpeechControlFlow(new_mode);
                    break;
                }
            }
            break;
        }
        default:
            break;
    }

    if (ModeLeaveCall(new_mode))
    {
        mAudio_Control_State.bSpeechFlag = false;
        UpdateKernelState();
    }

    // check input/output need suspend_cb_table
    if (ModeEnterCall(new_mode) || ModeCallSwitch(new_mode) || ModeLeaveCall(new_mode) || ModeEnterSipCall(new_mode) || ModeLeaveSipCall(new_mode))
    {
        SetOutputSuspend(false);
        SetInputSuspend(false);
    }

    mMode = new_mode;  // save mode when all things done.
    if (ModeInCall(mMode) == false)
    {
        setMasterVolume(mAudioVolumeInstance->getMasterVolume());
    }
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_VOLUME_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);

    ALOGD("-%s(), mode = %d", __FUNCTION__, new_mode);
    return NO_ERROR;
}

status_t AudioMTKHardware::setMicMute(bool state)
{
    ALOGD("%s(), new state = %d, old mMicMute = %d", __FUNCTION__, state, mMicMute);
    if (ModeInCall(mMode) == true) // modem phone call
    {
        SpeechPhoneCallController::GetInstance()->SetMicMute(state);
    }
    else // SIP call
    {
        mAudioMTKStreamManager->SetInputMute(state);
    }
    mMicMute = state;
    return NO_ERROR;
}

status_t AudioMTKHardware::getMicMute(bool *state)
{
    ALOGD("%s(), mMicMute = %d", __FUNCTION__, mMicMute);
    *state = mMicMute;
    return NO_ERROR;
}

android_audio_legacy::AudioStreamOut *AudioMTKHardware::openOutputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status)
{
    return mAudioMTKStreamManager->openOutputStream(devices, format, channels, sampleRate, status);
}

void AudioMTKHardware::closeOutputStream(android_audio_legacy::AudioStreamOut *out)
{
    mAudioMTKStreamManager->closeOutputStream(out);
}

android_audio_legacy::AudioStreamIn *AudioMTKHardware::openInputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    ALOGD("openInputStream, devices = 0x%x format=0x%x ,channels=0x%x, rate=%d acoustics = 0x%x", devices, *format, *channels, *sampleRate, acoustics);
    return mAudioMTKStreamManager->openInputStream(devices, format,  channels,  sampleRate,  status, acoustics);
}

void AudioMTKHardware::closeInputStream(android_audio_legacy::AudioStreamIn *in)
{
    mAudioMTKStreamManager->closeInputStream(in);
}


status_t AudioMTKHardware::GetDefaultDcCalibrationParam(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *cali_param)
{
    GetDcCalibrationParamFromNV(cali_param);
    ALOGD("GetDefaultDcCalibrationParam 0x%x HPL= 0x%x, HPR = 0x%x", cali_param->cali_flag, cali_param->cali_val_hp_left, cali_param->cali_val_hp_right);
    return NO_ERROR;
}

status_t AudioMTKHardware::DcCalibrationProcess(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *cali_param)
{
    ALOGD("DC Calibration Flag %d, HPL 0x%x, HPR 0x%x\n", cali_param->cali_flag, cali_param->cali_val_hp_left, cali_param->cali_val_hp_right);

    if (cali_param->cali_flag != 1)
    {
        //Enter Calibration and save to NV
        int val_hpr_on, val_hpl_on, val_hpr_off, val_hpl_off, val_hpr_on_sum = 0, val_hpl_on_sum = 0;
        ALOGD("DC Calibration Processing\n");
        mAudioFtmInstance->ReadAuxadcData(AUXADC_HP_L_CHANNEL, &val_hpl_off); //LCH
        mAudioFtmInstance->ReadAuxadcData(AUXADC_HP_R_CHANNEL, &val_hpr_off); //RCH
        ALOGD("ReadAuxAdc buffer off L 0x%x R 0x%x\n", val_hpl_off, val_hpr_off);
        mAudioDigitalInstance->SetI2SDacOutAttribute(44100);
        mAudioDigitalInstance->SetI2SDacEnable(true);
        mAudioAnalogInstance->SetDcCalibration(AudioAnalogType::DEVICE_OUT_HEADSETR, 0);
        mAudioAnalogInstance->SetDcCalibration(AudioAnalogType::DEVICE_OUT_HEADSETL, 0);
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, 10);
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, 10); // -1dB, (9-(-1) = 10)

        for (int i = 0; i < 5 ; i ++)
        {
            mAudioFtmInstance->ReadAuxadcData(AUXADC_HP_L_CHANNEL, &val_hpl_on); //LCH
            mAudioFtmInstance->ReadAuxadcData(AUXADC_HP_R_CHANNEL, &val_hpr_on); //RCH
            ALOGD("ReadAuxAdc buffer on %d th L 0x%x R 0x%x\n", i, val_hpl_on, val_hpr_on);
            val_hpl_on_sum += val_hpl_on;
            val_hpr_on_sum += val_hpr_on;
        }
        val_hpl_on = val_hpl_on_sum / 5;
        val_hpr_on = val_hpr_on_sum / 5;
        ALOGD("ReadAuxAdc buffer on Avg L 0x%x R 0x%x\n", val_hpl_on, val_hpr_on);
        mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        mAudioDigitalInstance->SetI2SDacEnable(false);
        mAudioDigitalInstance->SetAfeEnable(false);
        cali_param->cali_flag = 1;
        cali_param->cali_val_hp_left = val_hpl_on - val_hpl_off;
        cali_param->cali_val_hp_right = val_hpr_on - val_hpr_off;
        SetDcCalibrationParamToNV(cali_param); //Save to NVRAM
    }
    else
    {
        ALOGD("DC Calibration Value from NvRam\n");
    }
    return NO_ERROR;
}

status_t AudioMTKHardware::SetDcCalibration(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *cali_param)
{
    mAudioAnalogInstance->SetDcCalibration(AudioAnalogType::DEVICE_OUT_HEADSETR, (cali_param->cali_val_hp_right * 18) / 10);
    mAudioAnalogInstance->SetDcCalibration(AudioAnalogType::DEVICE_OUT_HEADSETL, (cali_param->cali_val_hp_left * 18) / 10);
    return NO_ERROR;
}


}
