#ifndef ANDROID_AUDIO_MTK_HARDWARE_COMMONCOMMAND_CPP
#define ANDROID_AUDIO_MTK_HARDWARE_COMMONCOMMAND_CPP

/*==============================================================================
 *                     setParameters() keys for common
 *============================================================================*/

// Phone Call Related
static String8 keySetVTSpeechCall     = String8("SetVTSpeechCall");

static String8 keyBtHeadsetNrec       = String8("bt_headset_nrec");

// FM Related
static String8 keyAnalogFmEnable      = String8("AudioSetFmEnable");
static String8 keyDigitalFmEnable     = String8("AudioSetFmDigitalEnable");
static String8 keyGetFmEnable         = String8("GetFmEnable");

static String8 keySetFmVolume         = String8("SetFmVolume");

static String8 keySetFmForceToSpk     = String8("AudioSetForceToSpeaker");

static String8 keyGetIsWiredHeadsetOn = String8("AudioFmIsWiredHeadsetOn");

//mATV Related
static String8 keyMatvAnalogEnable    = String8("AtvAudioLineInEnable");;
static String8 keyMatvDigitalEnable   = String8("AudioSetMatvDigitalEnable");
static String8 keySetMatvVolume       = String8("SetMatvVolume");
static String8 keySetMatvMute         = String8("SetMatvMute");

//record left/right channel switch
//only support on dual MIC for switch LR input channel for video record when the device rotate
static String8 keyLR_ChannelSwitch = String8("LRChannelSwitch");
//force use Min MIC or Ref MIC data
//only support on dual MIC for only get main Mic or Ref Mic data
static String8 keyForceUseSpecificMicData = String8("ForceUseSpecificMic");

#ifdef MTK_AUDIO_HD_REC_SUPPORT
//static String8 key_HD_REC_MODE = String8("HDRecordMode");
static String8 keyHDREC_SET_VOICE_MODE = String8("HDREC_SET_VOICE_MODE");
static String8 keyHDREC_SET_VIDEO_MODE = String8("HDREC_SET_VIDEO_MODE");
#endif

//HDMI command
static String8 key_GET_HDMI_AUDIO_STATUS = String8("GetHDMIAudioStatus");
static String8 key_SET_HDMI_AUDIO_ENABLE = String8("SetHDMIAudioEnable");


// Audio Tool related
//<---for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration) and HQA
static String8 keySpeechParams_Update = String8("UpdateSpeechParameter");
static String8 keySpeechVolume_Update = String8("UpdateSphVolumeParameter");
static String8 keyACFHCF_Update = String8("UpdateACFHCFParameters");
#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
static String8 keyDualMicParams_Update = String8("UpdateDualMicParameters");
static String8 keyDualMicRecPly = String8("DUAL_MIC_REC_PLAY");
static String8 keyDUALMIC_IN_FILE_NAME = String8("DUAL_MIC_IN_FILE_NAME");
static String8 keyDUALMIC_OUT_FILE_NAME = String8("DUAL_MIC_OUT_FILE_NAME");
static String8 keyDUALMIC_GET_GAIN = String8("DUAL_MIC_GET_GAIN");
static String8 keyDUALMIC_SET_UL_GAIN = String8("DUAL_MIC_SET_UL_GAIN");
static String8 keyDUALMIC_SET_DL_GAIN = String8("DUAL_MIC_SET_DL_GAIN");
static String8 keyDUALMIC_SET_HSDL_GAIN = String8("DUAL_MIC_SET_HSDL_GAIN");
static String8 keyDUALMIC_SET_UL_GAIN_HF = String8("DUAL_MIC_SET_UL_GAIN_HF");
#endif
static String8 keyMusicPlusSet      = String8("SetMusicPlusStatus");
static String8 keyMusicPlusGet      = String8("GetMusicPlusStatus");
static String8 keyHiFiDACSet      = String8("SetHiFiDACStatus");
static String8 keyHiFiDACGet      = String8("GetHiFiDACStatus");
static String8 keyHDRecTunningEnable    = String8("HDRecTunningEnable");
static String8 keyHDRecVMFileName   = String8("HDRecVMFileName");

static String8 keyBesLoudnessSet      = String8("SetBesLoudnessStatus");
static String8 keyBesLoudnessGet      = String8("GetBesLoudnessStatus");

//--->




// Dual Mic Noise Reduction, DMNR for Receiver
static String8 keyEnable_Dual_Mic_Setting = String8("Enable_Dual_Mic_Setting");
static String8 keyGet_Dual_Mic_Setting    = String8("Get_Dual_Mic_Setting");

// Dual Mic Noise Reduction, DMNR for Loud Speaker
static String8 keySET_LSPK_DMNR_ENABLE = String8("SET_LSPK_DMNR_ENABLE");
static String8 keyGET_LSPK_DMNR_ENABLE = String8("GET_LSPK_DMNR_ENABLE");

// Voice Clarity Engine, VCE
static String8 keySET_VCE_ENABLE = String8("SET_VCE_ENABLE");
static String8 keyGET_VCE_ENABLE = String8("GET_VCE_ENABLE");
static String8 keyGET_VCE_STATUS = String8("GET_VCE_STATUS"); // old name, rename to GET_VCE_ENABLE, but still reserve it

// Magic Conference Call
static String8 keySET_MAGIC_CON_CALL_ENABLE = String8("SET_MAGIC_CON_CALL_ENABLE");
static String8 keyGET_MAGIC_CON_CALL_ENABLE = String8("GET_MAGIC_CON_CALL_ENABLE");

//VoIP
//VoIP Dual Mic Noise Reduction, DMNR for Receiver
static String8 keySET_VOIP_RECEIVER_DMNR_ENABLE = String8("SET_VOIP_RECEIVER_DMNR_ENABLE");
static String8 keyGET_VOIP_RECEIVER_DMNR_ENABLE    = String8("GET_VOIP_RECEIVER_DMNR_ENABLE");

//VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
static String8 keySET_VOIP_LSPK_DMNR_ENABLE = String8("SET_VOIP_LSPK_DMNR_ENABLE");
static String8 keyGET_VOIP_LSPK_DMNR_ENABLE = String8("GET_VOIP_LSPK_DMNR_ENABLE");

static String8 keyGET_AUDIO_VOLUME_VER = String8("GET_AUDIO_VOLUME_VERSION");

// Loopbacks
static String8 keySET_LOOPBACK_USE_LOUD_SPEAKER = String8("SET_LOOPBACK_USE_LOUD_SPEAKER");
static String8 keySET_LOOPBACK_TYPE = String8("SET_LOOPBACK_TYPE");
static String8 keySET_LOOPBACK_MODEM_DELAY_FRAMES = String8("SET_LOOPBACK_MODEM_DELAY_FRAMES");


// TTY
static String8 keySetTtyMode     = String8("tty_mode");
#ifdef EVDO_DT_SUPPORT
//EVDO
static String8 keySET_WARNING_TONE = String8("SetWarningTone");
static String8 keySTOP_WARNING_TONE = String8("StopWarningTone");
static String8 keySET_VOICE_VOLUME_INDEX = String8("SetVoiceVolumeIndex");
#endif
//#if defined(MTK_VIBSPK_SUPPORT)
static String8 keySET_VIBSPK_ENABLE = String8("SET_VIBSPK_ENABLE");
static String8 keySET_VIBSPK_RAMPDOWN = String8("SET_VIBSPK_RAMPDOWN");
//#endif

static String8 keySCREEN_STATE = String8("screen_state");
static String8 keySET_KERNEL_DEBUG_MODE = String8("SET_KERNEL_DEBUG_MODE");

// BT WB
static String8 keySetBTMode     = String8("SET_DAIBT_MODE");

// for stereo output
static String8 keyEnableStereoOutput = String8("EnableStereoOutput");


status_t AudioMTKHardware::SetAudioCommonCommand(int par1, int par2)
{
    ALOGD("%s(), par1 = 0x%x, par2 = %d", __FUNCTION__, par1, par2);
    char value[PROPERTY_VALUE_MAX];
    switch (par1)
    {
        case SETOUTPUTFIRINDEX:
        {
            UpdateOutputFIR(Normal_Coef_Index, par2);
            break;
        }
        case START_FMTX_SINEWAVE:
        {
            return NO_ERROR;
        }
        case STOP_FMTX_SINEWAVE:
        {
            return NO_ERROR;
        }
        case SETNORMALOUTPUTFIRINDEX:
        {
            UpdateOutputFIR(Normal_Coef_Index, par2);
            break;
        }
        case SETHEADSETOUTPUTFIRINDEX:
        {
            UpdateOutputFIR(Headset_Coef_Index, par2);
            break;
        }
        case SETSPEAKEROUTPUTFIRINDEX:
        {
            UpdateOutputFIR(Handfree_Coef_Index, par2);
            break;
        }
        case SET_LOAD_VOLUME_SETTING:
        {
            mAudioVolumeInstance->initVolumeController();
            setMasterVolume(mAudioVolumeInstance->getMasterVolume());
            const sp<IAudioPolicyService> &aps = AudioSystem::get_audio_policy_service();
            aps->SetPolicyManagerParameters(POLICY_LOAD_VOLUME, 0, 0, 0);
            break;
        }
        case SET_SPEECH_VM_ENABLE:
        {
            ALOGD("+SET_SPEECH_VM_ENABLE(%d)", par2);
            AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
            GetNBSpeechParamFromNVRam(&eSphParamNB);
            if (par2 == 0) // normal VM
            {
                eSphParamNB.debug_info[0] = 0;
            }
            else // EPL
            {
                eSphParamNB.debug_info[0] = 3;
                if (eSphParamNB.speech_common_para[0] == 0) // if not assign EPL debug type yet, set a default one
                {
                    eSphParamNB.speech_common_para[0] = 6;
                }
            }
            SetNBSpeechParamToNVRam(&eSphParamNB);
            SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(&eSphParamNB);
            ALOGD("-SET_SPEECH_VM_ENABLE(%d)", par2);
            break;
        }
        case SET_DUMP_SPEECH_DEBUG_INFO:
        {
            ALOGD(" SET_DUMP_SPEECH_DEBUG_INFO(%d)", par2);
            mSpeechDriverFactory->GetSpeechDriver()->ModemDumpSpeechParam();
            break;
        }
        case SET_DUMP_AUDIO_DEBUG_INFO:
        {
            ALOGD(" SET_DUMP_AUDIO_DEBUG_INFO(%d)", par2);
            ::ioctl(mFd, AUDDRV_LOG_PRINT, 0);
            mAudioDigitalInstance->EnableSideToneHw(AudioDigitalType::O03 , false, true);
            sleep(3);
            mAudioDigitalInstance->EnableSideToneHw(AudioDigitalType::O03 , false, false);
            break;
        }
        case SET_DUMP_AUDIO_AEE_CHECK:
        {
            ALOGD(" SET_DUMP_AUDIO_AEE_CHECK(%d)", par2);
            if (par2 == 0)
            {
                property_set("streamout.aee.dump", "0");
            }
            else
            {
                property_set("streamout.aee.dump", "1");
            }
            break;
        }
        case SET_DUMP_AUDIO_STREAM_OUT:
        {
            ALOGD(" SET_DUMP_AUDIO_STREAM_OUT(%d)", par2);
            if (par2 == 0)
            {
                property_set("streamout.pcm.dump", "0");
                ::ioctl(mFd, AUDDRV_AEE_IOCTL, 0);
            }
            else
            {
                property_set("streamout.pcm.dump", "1");
                ::ioctl(mFd, AUDDRV_AEE_IOCTL, 1);
            }
            break;
        }
        case SET_DUMP_AUDIO_MIXER_BUF:
        {
            ALOGD(" SET_DUMP_AUDIO_MIXER_BUF(%d)", par2);
            if (par2 == 0)
            {
                property_set("af.mixer.pcm", "0");
            }
            else
            {
                property_set("af.mixer.pcm", "1");
            }
            break;
        }
        case SET_DUMP_AUDIO_TRACK_BUF:
        {
            ALOGD(" SET_DUMP_AUDIO_TRACK_BUF(%d)", par2);
            if (par2 == 0)
            {
                property_set("af.track.pcm", "0");
            }
            else
            {
                property_set("af.track.pcm", "1");
            }
            break;
        }
        case SET_DUMP_A2DP_STREAM_OUT:
        {
            ALOGD(" SET_DUMP_A2DP_STREAM_OUT(%d)", par2);
            if (par2 == 0)
            {
                property_set("a2dp.streamout.pcm", "0");
            }
            else
            {
                property_set("a2dp.streamout.pcm", "1");
            }
            break;
        }
        case SET_DUMP_AUDIO_STREAM_IN:
        {
            ALOGD(" SET_DUMP_AUDIO_STREAM_IN(%d)", par2);
            if (par2 == 0)
            {
                property_set("streamin.pcm.dump", "0");
            }
            else
            {
                property_set("streamin.pcm.dump", "1");
            }
            break;
        }
        case SET_DUMP_IDLE_VM_RECORD:
        {
            ALOGD(" SET_DUMP_IDLE_VM_RECORD(%d)", par2);
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
            if (par2 == 0)
            {
                property_set("streamin.vm.dump", "0");
            }
            else
            {
                property_set("streamin.vm.dump", "1");
            }
#endif
            break;
        }
        case AUDIO_USER_TEST:
        {
            if (par2 == 0)
            {
                mAudioFtmInstance->EarphoneTest(true);
            }
            else if (par2 == 1)
            {
                mAudioFtmInstance->EarphoneTest(false);
            }
            else if (par2 == 2)
            {
                mAudioFtmInstance->FTMPMICLoopbackTest(true);
            }
            else if (par2 == 3)
            {
                mAudioFtmInstance->FTMPMICLoopbackTest(false);
            }
            else if (par2 == 4)
            {
                mAudioFtmInstance->LouderSPKTest(true, true);
            }
            else if (par2 == 5)
            {
                mAudioFtmInstance->LouderSPKTest(false, false);
            }
            else if (par2 == 6)
            {
                mAudioFtmInstance->RecieverTest(true);
            }
            else if (par2 == 7)
            {
                mAudioFtmInstance->RecieverTest(false);
            }
            else if (par2 == 8)
            {
                mAudioFtmInstance->FTMPMICEarpieceLoopbackTest(true);
            }
            else if (par2 == 9)
            {
                mAudioFtmInstance->FTMPMICEarpieceLoopbackTest(false);
            }
            else if (par2 == 0x10)
            {
                mAudioFtmInstance->FTMPMICDualModeLoopbackTest(true);
            }
            else if (par2 == 0x11)
            {
                mAudioFtmInstance->FTMPMICDualModeLoopbackTest(false);
            }
            else if (par2 == 0x12)
            {
                //mAudioFtmInstance->PhoneMic_EarphoneLR_Loopback(true);
                LoopbackManager::GetInstance()->SetLoopbackOn(AP_MAIN_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
            }
            else if (par2 == 0x13)
            {
                //mAudioFtmInstance->PhoneMic_EarphoneLR_Loopback(false);
                LoopbackManager::GetInstance()->SetLoopbackOff();
            }
            else if (par2 == 0x14)
            {
                mAudioFtmInstance->Pmic_I2s_out(true);
            }
            else if (par2 == 0x15)
            {
                mAudioFtmInstance->Pmic_I2s_out(false);
            }
            else if (par2 == 0x16)
            {
                mAudioFtmInstance->FMLoopbackTest(true);
            }
            else if (par2 == 0x17)
            {
                mAudioFtmInstance->FMLoopbackTest(false);
            }
            else if (par2 == 0x18)
            {
                //mAudioFtmInstance->PhoneMic_Receiver_Loopback(true);
                LoopbackManager::GetInstance()->SetLoopbackOn(AP_MAIN_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_RECEIVER);
            }
            else if (par2 == 0x19)
            {
                //mAudioFtmInstance->PhoneMic_Receiver_Loopback(false);
                LoopbackManager::GetInstance()->SetLoopbackOff();
            }
            else if (par2 == 0x20) // same as 0x12 ??
            {
                //mAudioFtmInstance->PhoneMic_EarphoneLR_Loopback(true);
                LoopbackManager::GetInstance()->SetLoopbackOn(AP_MAIN_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
            }
            else if (par2 == 0x21) // same as 0x13 ??
            {
                //mAudioFtmInstance->PhoneMic_EarphoneLR_Loopback(false);
                LoopbackManager::GetInstance()->SetLoopbackOff();
            }
            else if (par2 == 0x22)
            {
                //mAudioFtmInstance->PhoneMic_SpkLR_Loopback(true);
                LoopbackManager::GetInstance()->SetLoopbackOn(AP_MAIN_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_SPEAKER);
            }
            else if (par2 == 0x23)
            {
                //mAudioFtmInstance->PhoneMic_SpkLR_Loopback(false);
                LoopbackManager::GetInstance()->SetLoopbackOff();
            }
            else if (par2 == 0x24)
            {
                //mAudioFtmInstance->HeadsetMic_EarphoneLR_Loopback(true, true);
                LoopbackManager::GetInstance()->SetLoopbackOn(AP_HEADSET_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
            }
            else if (par2 == 0x25)
            {
                //mAudioFtmInstance->HeadsetMic_EarphoneLR_Loopback(false, false);
                LoopbackManager::GetInstance()->SetLoopbackOff();
            }
            else if (par2 == 0x26)
            {
                //mAudioFtmInstance->HeadsetMic_SpkLR_Loopback(true);
                LoopbackManager::GetInstance()->SetLoopbackOn(AP_HEADSET_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_SPEAKER);
            }
            else if (par2 == 0x27)
            {
                //mAudioFtmInstance->HeadsetMic_SpkLR_Loopback(false);
                LoopbackManager::GetInstance()->SetLoopbackOff();
            }
            else if (par2 == 0x28)
            {
                //mAudioFtmInstance->HeadsetMic_Receiver_Loopback(true, true);
                LoopbackManager::GetInstance()->SetLoopbackOn(AP_HEADSET_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_RECEIVER);
            }
            else if (par2 == 0x29)
            {
                //mAudioFtmInstance->HeadsetMic_Receiver_Loopback(false, false);
                LoopbackManager::GetInstance()->SetLoopbackOff();
            }
            else if (par2 == 0x30)
            {
                mAudioResourceManager->StartInputDevice();
                mAudioResourceManager->StartOutputDevice();
                ALOGD("2 0x30 sleep");
                sleep(3);
            }
            else if (par2 == 0x31)
            {
                mAudioResourceManager->StopOutputDevice();
                mAudioResourceManager->StopInputDevice();
            }
            break;
        }
        case SET_DUMP_AP_SPEECH_EPL:
        {
            ALOGD(" SET_DUMP_AP_SPEECH_EPL(%d)", par2);
            if (par2 == 0)
            {
                property_set("streamin.epl.dump", "0");
            }
            else
            {
                property_set("streamin.epl.dump", "1");
            }
            break;
        }
        case SET_MagiASR_TEST_ENABLE:
        {
            ALOGD(" SET_MagiASR_TEST_ENABLE(%d)", par2);
            if (par2 == 0)
            {
                //disable MagiASR verify mode
                mAudioSpeechEnhanceInfoInstance->SetForceMagiASR(false);
            }
            else
            {
                //enable MagiASR verify mode
                mAudioSpeechEnhanceInfoInstance->SetForceMagiASR(true);
            }
            break;
        }
        case SET_AECREC_TEST_ENABLE:
        {
            ALOGD(" SET_AECREC_TEST_ENABLE(%d)", par2);
            if (par2 == 0)
            {
                //disable AECRec verify mode
                mAudioSpeechEnhanceInfoInstance->SetForceAECRec(false);
            }
            else
            {
                //enable AECRec verify mode
                mAudioSpeechEnhanceInfoInstance->SetForceAECRec(true);
            }
            break;
        }
        default:
            break;
    }
    return NO_ERROR;
}

status_t AudioMTKHardware::GetAudioCommonCommand(int parameters1)
{
    ALOGD("GetAudioCommonCommand parameters1 = %d ", parameters1);
    int result = 0 ;
    char value[PROPERTY_VALUE_MAX];
    switch (parameters1)
    {
        case GETOUTPUTFIRINDEX:
        {
            AUDIO_PARAM_MED_STRUCT pMedPara;
            GetMedParamFromNV(&pMedPara);
            result = pMedPara.select_FIR_output_index[Normal_Coef_Index];
            break;
        }
        case GETAUDIOCUSTOMDATASIZE:
        {
            int AudioCustomDataSize = sizeof(AUDIO_VOLUME_CUSTOM_STRUCT);
            ALOGD("GETAUDIOCUSTOMDATASIZE  AudioCustomDataSize = %d", AudioCustomDataSize);
            return AudioCustomDataSize;
        }
        case GETNORMALOUTPUTFIRINDEX:
        {
            AUDIO_PARAM_MED_STRUCT pMedPara;
            GetMedParamFromNV(&pMedPara);
            result = pMedPara.select_FIR_output_index[Normal_Coef_Index];
            break;
        }
        case GETHEADSETOUTPUTFIRINDEX:
        {
            AUDIO_PARAM_MED_STRUCT pMedPara;
            GetMedParamFromNV(&pMedPara);
            result = pMedPara.select_FIR_output_index[Headset_Coef_Index];
            break;
        }
        case GETSPEAKEROUTPUTFIRINDEX:
        {
            AUDIO_PARAM_MED_STRUCT pMedPara;
            GetMedParamFromNV(&pMedPara);
            result = pMedPara.select_FIR_output_index[Handfree_Coef_Index];
            break;
        }
        case GET_DUMP_AUDIO_AEE_CHECK:
        {
            property_get("streamout.aee.dump", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_STREAM_OUT=%d", result);
            break;
        }
        case GET_DUMP_AUDIO_STREAM_OUT:
        {
            property_get("streamout.pcm.dump", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_STREAM_OUT=%d", result);
            break;
        }
        case GET_DUMP_AUDIO_MIXER_BUF:
        {
            property_get("af.mixer.pcm", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_MIXER_BUF=%d", result);
            break;
        }
        case GET_DUMP_AUDIO_TRACK_BUF:
        {
            property_get("af.track.pcm", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_TRACK_BUF=%d", result);
            break;
        }
        case GET_DUMP_A2DP_STREAM_OUT:
        {
            property_get("a2dp.streamout.pcm", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_A2DP_STREAM_OUT=%d", result);
            break;
        }
        case GET_DUMP_AUDIO_STREAM_IN:
        {
            property_get("streamin.pcm.dump", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_STREAM_IN=%d", result);
            break;
        }
        case GET_DUMP_IDLE_VM_RECORD:
        {
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
            property_get("streamin.vm.dump", value, "0");
            result = atoi(value);
#else
            result = 0;
#endif
            ALOGD(" GET_DUMP_IDLE_VM_RECORD=%d", result);
            break;
        }
        case GET_DUMP_AP_SPEECH_EPL:
        {
            property_get("SPE_EPL", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AP_SPEECH_EPL=%d", result);
            break;
        }
        case GET_MagiASR_TEST_ENABLE:
        {
            //get the MagiASR verify mode status
            result = mAudioSpeechEnhanceInfoInstance->GetForceMagiASRState();
            ALOGD(" GET_MagiASR_TEST_ENABLE=%d", result);
            break;
        }
        case GET_AECREC_TEST_ENABLE:
        {
            //get the AECRec verify mode status
            result = 0;
            if (mAudioSpeechEnhanceInfoInstance->GetForceAECRecState())
            {
                result = 1;
            }
            ALOGD(" GET_AECREC_TEST_ENABLE=%d", result);
            break;
        }

#ifdef MTK_ACF_AUTO_GEN_SUPPORT
        case AUDIO_ACF_FO:
        {
            result = mAudioTuningInstance->getFOValue();
            ALOGD("getFOValue result is %d", result);
            break;
        }
#endif

        default:
        {
            ALOGD(" GetAudioCommonCommand: Unknown command\n");
            break;
        }
    }
    // call fucntion want to get status adn return it.
    return result;
}

status_t AudioMTKHardware::SetAudioCommonData(int par1, size_t len, void *ptr)
{
    ALOGD("%s(), par1 = 0x%x, len = %d", __FUNCTION__, par1, len);
    switch (par1)
    {
        case HOOK_BESLOUDNESS_CONTROL_CALLBACK:
        {
            mAudioMTKStreamManager->SetBesLoudnessControlCallback((BESLOUDNESS_CONTROL_CALLBACK_STRUCT *)ptr);
            break;
        }
        case UNHOOK_BESLOUDNESS_CONTROL_CALLBACK:
        {
            mAudioMTKStreamManager->SetBesLoudnessControlCallback(NULL);
            break;
        }
        case SETMEDDATA:
        {
            ASSERT(len == sizeof(AUDIO_PARAM_MED_STRUCT));
            SetMedParamToNV((AUDIO_PARAM_MED_STRUCT *)ptr);
            break;
        }
        case SETAUDIOCUSTOMDATA:
        {
            ASSERT(len == sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
            SetAudioCustomParamToNV((AUDIO_VOLUME_CUSTOM_STRUCT *)ptr);
            mAudioVolumeInstance->initVolumeController();
            setMasterVolume(mAudioVolumeInstance->getMasterVolume());
            break;
        }
#if defined(MTK_DUAL_MIC_SUPPORT)
        case SET_DUAL_MIC_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
            SetDualMicSpeechParamToNVRam((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
            mAudioVolumeInstance->initVolumeController();
            SpeechEnhancementController::GetInstance()->SetDualMicSpeechParametersToAllModem((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
            break;
        }
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
        case SET_WB_SPEECH_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
            SetWBSpeechParamToNVRam((AUDIO_CUSTOM_WB_PARAM_STRUCT *)ptr);
            SpeechEnhancementController::GetInstance()->SetWBSpeechParametersToAllModem((AUDIO_CUSTOM_WB_PARAM_STRUCT *)ptr);
            mAudioVolumeInstance->initVolumeController(); // for DRC2.0 need volume to get speech mode
            break;
        }
#endif
        case SET_AUDIO_VER1_DATA:
        {
            ASSERT(len == sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT));
            SetVolumeVer1ParamToNV((AUDIO_VER1_CUSTOM_VOLUME_STRUCT *)ptr);
            mAudioVolumeInstance->initVolumeController();
            setMasterVolume(mAudioVolumeInstance->getMasterVolume());
            const sp<IAudioPolicyService> &aps = AudioSystem::get_audio_policy_service();
            aps->SetPolicyManagerParameters(POLICY_LOAD_VOLUME, 0, 0, 0);
            break;
        }

        // for Audio Taste Tuning
        case AUD_TASTE_TUNING:
        {
#if 1
            status_t ret = NO_ERROR;
            AudioTasteTuningStruct audioTasteTuningParam;
            memcpy((void *)&audioTasteTuningParam, ptr, sizeof(AudioTasteTuningStruct));

            switch (audioTasteTuningParam.cmd_type)
            {
                case AUD_TASTE_STOP:
                {

                    mAudioTuningInstance->enableModemPlaybackVIASPHPROC(false);
                    audioTasteTuningParam.wb_mode = mAudioTuningInstance->m_bWBMode;
                    mAudioTuningInstance->updataOutputFIRCoffes(&audioTasteTuningParam);

                    break;
                }
                case AUD_TASTE_START:
                {

                    mAudioTuningInstance->setMode(audioTasteTuningParam.phone_mode);
                    ret = mAudioTuningInstance->setPlaybackFileName(audioTasteTuningParam.input_file);
                    if (ret != NO_ERROR)
                    {
                        return ret;
                    }
                    ret = mAudioTuningInstance->setDLPGA((uint32) audioTasteTuningParam.dlPGA);
                    if (ret != NO_ERROR)
                    {
                        return ret;
                    }
                    mAudioTuningInstance->updataOutputFIRCoffes(&audioTasteTuningParam);
                    ret = mAudioTuningInstance->enableModemPlaybackVIASPHPROC(true, audioTasteTuningParam.wb_mode);
                    if (ret != NO_ERROR)
                    {
                        return ret;
                    }

                    break;
                }
                case AUD_TASTE_DLDG_SETTING:
                case AUD_TASTE_INDEX_SETTING:
                {
                    mAudioTuningInstance->updataOutputFIRCoffes(&audioTasteTuningParam);
                    break;
                }
                case AUD_TASTE_DLPGA_SETTING:
                {
                    mAudioTuningInstance->setMode(audioTasteTuningParam.phone_mode);
                    ret = mAudioTuningInstance->setDLPGA((uint32) audioTasteTuningParam.dlPGA);
                    if (ret != NO_ERROR)
                    {
                        return ret;
                    }

                    break;
                }
                default:
                    break;
            }
#endif
            break;
        }
        default:
            break;
    }
    return NO_ERROR;
}

status_t AudioMTKHardware::GetAudioCommonData(int par1, size_t len, void *ptr)
{
    ALOGD("%s par1=%d, len=%d", __FUNCTION__, par1, len);
    switch (par1)
    {
        case GETMEDDATA:
        {
            ASSERT(len == sizeof(AUDIO_PARAM_MED_STRUCT));
            GetMedParamFromNV((AUDIO_PARAM_MED_STRUCT *)ptr);
            break;
        }
        case GETAUDIOCUSTOMDATA:
        {
            ASSERT(len == sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
            GetAudioCustomParamFromNV((AUDIO_VOLUME_CUSTOM_STRUCT *)ptr);
            break;
        }
#if defined(MTK_DUAL_MIC_SUPPORT)
        case GET_DUAL_MIC_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
            GetDualMicSpeechParamFromNVRam((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
            break;
        }
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
        case GET_WB_SPEECH_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
            GetWBSpeechParamFromNVRam((AUDIO_CUSTOM_WB_PARAM_STRUCT *) ptr);
            break;
        }
#endif
#if defined(MTK_AUDIO_GAIN_TABLE)
        case GET_GAIN_TABLE_CTRPOINT_NUM:
        {
            int *p = (int *)ptr ;
            if (mAuioDevice != NULL)
            {
                *p = mAuioDevice->getParameters(AUD_AMP_GET_CTRP_NUM, 0, NULL);
            }
            break;
        }
        case GET_GAIN_TABLE_CTRPOINT_BITS:
        {
            int *point = (int *)ptr ;
            int *value = point + 1;
            if (mAuioDevice != NULL)
            {
                *value = mAuioDevice->getParameters(AUD_AMP_GET_CTRP_BITS, *point, NULL);
            }
            LOG_HARDWARE("GetAudioData GET_GAIN_TABLE_CTRPOINT_BITS point %d, value %d", *point, *value);
            break;
        }
        case GET_GAIN_TABLE_CTRPOINT_TABLE:
        {
            char *point = (char *)ptr ;
            int value = *point;
            if (mAuioDevice != NULL)
            {
                mAuioDevice->getParameters(AUD_AMP_GET_CTRP_TABLE, value, ptr);
            }
            break;
        }
#endif
        case GET_AUDIO_VER1_DATA:
        {
            GetVolumeVer1ParamFromNV((AUDIO_VER1_CUSTOM_VOLUME_STRUCT *) ptr);
            break;
        }
        case GET_VOICE_CUST_PARAM:
        {
            GetVoiceRecogCustParamFromNV((VOICE_RECOGNITION_PARAM_STRUCT *)ptr);
            break;
        }
        case GET_VOICE_FIR_COEF:
        {
            AUDIO_HD_RECORD_PARAM_STRUCT custHDRECParam;
            GetHdRecordParamFromNV(&custHDRECParam);
            ASSERT(len == sizeof(custHDRECParam.hd_rec_fir));
            memcpy(ptr, (void *)custHDRECParam.hd_rec_fir, len);
            break;
        }
        case GET_VOICE_GAIN:
        {
            AUDIO_VER1_CUSTOM_VOLUME_STRUCT custGainParam;
            GetVolumeVer1ParamFromNV(&custGainParam);
            uint16_t *pGain = (uint16_t *)ptr;
            *pGain = mAudioVolumeInstance->MappingToDigitalGain(custGainParam.audiovolume_mic[VOLUME_NORMAL_MODE][7]);
            *(pGain + 1) = mAudioVolumeInstance->MappingToDigitalGain(custGainParam.audiovolume_mic[VOLUME_HEADSET_MODE][7]);
            break;
        }
        default:
            break;
    }
    return NO_ERROR;
}
status_t AudioMTKHardware::setCommonParameters(const String8 &keyValuePairs)
{
    status_t status = NO_ERROR;
    int value = 0;
    String8 value_str;
    float value_float = 0.0;
    AudioParameter param = AudioParameter(keyValuePairs);
    ALOGD("+setCommonParameters(): %s ", keyValuePairs.string());

    do
    {
        // VT call (true) / Voice call (false)
        if (param.getInt(keySetVTSpeechCall, value) == NO_ERROR)
        {
            param.remove(keySetVTSpeechCall);
            SpeechPhoneCallController::GetInstance()->SetVtNeedOn((bool)value);
            break;
        }

#ifdef BTNREC_DECIDED_BY_DEVICE
        // BT NREC on/off
        if (param.get(keyBtHeadsetNrec, value_str) == NO_ERROR)
        {
            param.remove(keyBtHeadsetNrec);
            if (value_str == "on")
            {
                SpeechEnhancementController::GetInstance()->SetBtHeadsetNrecOnToAllModem(true);
            }
            else if (value_str == "off")
            {
                SpeechEnhancementController::GetInstance()->SetBtHeadsetNrecOnToAllModem(false);
            }
            break;
        }
#endif

        //Analog FM enable
        if (param.getInt(keyAnalogFmEnable, value) == NO_ERROR)
        {
            param.remove(keyAnalogFmEnable);
            WARNING("Not Support FM Analog Line In Path Anymore. Please Use Merge_I2S / 2nd_I2S_In");
            break;
        }
        //Digital FM enable
        if (param.getInt(keyDigitalFmEnable, value) == NO_ERROR)
        {
            param.remove(keyDigitalFmEnable);
            AudioFMController::GetInstance()->SetFmEnable((bool)value);
            break;
        }
        //Set FM volume
        if (param.getFloat(keySetFmVolume, value_float) == NO_ERROR)
        {
            param.remove(keySetFmVolume);
            AudioFMController::GetInstance()->SetFmVolume(value_float);
            break;
        }
        //Force FM to loudspeaker
        if (param.getInt(keySetFmForceToSpk, value) == NO_ERROR)
        {
            param.remove(keySetFmForceToSpk);
            WARNING("Do nothing for this command: AudioSetForceToSpeaker");
            break;
        }
        //Analog mATV Enable
        if (param.getInt(keyMatvAnalogEnable, value) == NO_ERROR)
        {
            param.remove(keyMatvAnalogEnable);
            WARNING("Do nothing for this command: AtvAudioLineInEnable");
            break;
        }
        //Digital mATV Enable
        if (param.getInt(keyMatvDigitalEnable, value) == NO_ERROR)
        {
            param.remove(keyMatvDigitalEnable);
#if defined(MATV_AUDIO_SUPPORT)
            AudioMATVController::GetInstance()->SetMatvEnable((bool)value);
#else
            WARNING("Do nothing for this command: AudioSetMatvDigitalEnable");
#endif
            break;
        }
        //Set mATV Volume
        if (param.getInt(keySetMatvVolume, value) == NO_ERROR)
        {
            param.remove(keySetMatvVolume);
            WARNING("Do nothing for this command: AtvAudioLineInEnable");
            break;
        }
        //mute mATV
        if (param.getInt(keySetMatvMute, value) == NO_ERROR)
        {
            param.remove(keySetMatvMute);
            WARNING("Do nothing for this command: AtvAudioLineInEnable");
            break;
        }

        //MusicPlus enable
        if (param.getInt(keyMusicPlusSet, value) == NO_ERROR)
        {
#ifndef HIFI_SWITCH_BY_AUDENH  //HP switch use AudEnh setting
            mAudioMTKStreamManager->SetMusicPlusStatus(value ? true : false);
#else
            bool prev_HiFiDACStatus, cur_HiFiDACStatus;
            prev_HiFiDACStatus = mAudioMTKStreamManager->GetHiFiDACStatus();
            cur_HiFiDACStatus = (value ? true : false);
            ALOGD("prev_HiFiDACStatus=%d, cur_HiFiDACStatus=%d", prev_HiFiDACStatus, cur_HiFiDACStatus);

            // update HiFiDACStatus
            mAudioMTKStreamManager->SetHiFiDACStatus(value ? true : false);

            if ((prev_HiFiDACStatus == true && cur_HiFiDACStatus == false) ||
                (prev_HiFiDACStatus == false && cur_HiFiDACStatus == true))
            {
                if (mAudioMTKStreamManager->IsOutPutStreamActive() == true ||
                    AudioFMController::GetInstance()->GetFmEnable() == true ||
                    AudioMATVController::GetInstance()->GetMatvEnable() == true)
                {
                    //Close original DAC path
                    mAudioResourceManager->StopOutputDevice();

                    //usleep(2000000); //experiment: let circuit to release voltage

                    //Open new DAC path
                    mAudioResourceManager->StartOutputDevice();
                }
            }
#endif
            param.remove(keyMusicPlusSet);
            break;
        }
        if (param.getInt(keyBesLoudnessSet, value) == NO_ERROR)
        {
            mAudioMTKStreamManager->SetBesLoudnessStatus(value ? true : false);
            param.remove(keyBesLoudnessSet);
            break;
        }

        //HiFiDAC enable
        if (param.getInt(keyHiFiDACSet, value) == NO_ERROR)
        {
            bool prev_HiFiDACStatus, cur_HiFiDACStatus;
            prev_HiFiDACStatus = mAudioMTKStreamManager->GetHiFiDACStatus();
            cur_HiFiDACStatus = (value ? true : false);
            ALOGD("prev_HiFiDACStatus=%d, cur_HiFiDACStatus=%d", prev_HiFiDACStatus, cur_HiFiDACStatus);

            if ((prev_HiFiDACStatus == true && cur_HiFiDACStatus == false) ||
                (prev_HiFiDACStatus == false && cur_HiFiDACStatus == true))
            {
                if (mAudioMTKStreamManager->IsOutPutStreamActive() == true ||
                    AudioFMController::GetInstance()->GetFmEnable() == true ||
                    AudioMATVController::GetInstance()->GetMatvEnable() == true)
                {
                    //Close original DAC path
                    mAudioResourceManager->StopOutputDevice();

                    // update HiFiDACStatus
                    mAudioMTKStreamManager->SetHiFiDACStatus(value ? true : false);

                    //Open new DAC path
                    mAudioResourceManager->StartOutputDevice();
                }
            }

            param.remove(keyHiFiDACSet);
            break;
        }

        if (param.getInt(keyLR_ChannelSwitch, value) == NO_ERROR)
        {
#ifdef MTK_DUAL_MIC_SUPPORT
            ALOGD("keyLR_ChannelSwitch=%d", value);
            bool bIsLRSwitch = value;
            mAudioSpeechEnhanceInfoInstance->SetRecordLRChannelSwitch(bIsLRSwitch);
#else
            ALOGD("only support in dual MIC");
#endif
            param.remove(keyLR_ChannelSwitch);
            //goto EXIT_SETPARAMETERS;
            //Because parameters will send two strings, we need to parse another.(HD Record info and Channel Switch info)
        }

        if (param.getInt(keyForceUseSpecificMicData, value) == NO_ERROR)
        {
#ifdef MTK_DUAL_MIC_SUPPORT
            ALOGD("keyForceUseSpecificMicData=%d", value);
            int32 UseSpecificMic = value;
            mAudioSpeechEnhanceInfoInstance->SetUseSpecificMIC(UseSpecificMic);
#else
            ALOGD("only support in dual MIC");
#endif
            param.remove(keyForceUseSpecificMicData);
            break;
        }

#ifdef MTK_AUDIO_HD_REC_SUPPORT
        if (param.getInt(keyHDREC_SET_VOICE_MODE, value) == NO_ERROR)
        {
            ALOGD("HDREC_SET_VOICE_MODE=%d", value); // Normal, Indoor, Outdoor,
            param.remove(keyHDREC_SET_VOICE_MODE);
            //Get and Check Voice/Video Mode Offset
            AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
            GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
            if (value < hdRecordSceneTable.num_voice_rec_scenes)
            {
                int32 HDRecScene = value + 1;//1:cts verifier offset
                mAudioSpeechEnhanceInfoInstance->SetHDRecScene(HDRecScene);
            }
            else
            {
                ALOGE("HDREC_SET_VOICE_MODE=%d exceed max value(%d)\n", value, hdRecordSceneTable.num_voice_rec_scenes);
            }
            break;
        }

        if (param.getInt(keyHDREC_SET_VIDEO_MODE, value) == NO_ERROR)
        {
            ALOGD("HDREC_SET_VIDEO_MODE=%d", value); // Normal, Indoor, Outdoor,
            param.remove(keyHDREC_SET_VIDEO_MODE);
            //Get and Check Voice/Video Mode Offset
            AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
            GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
            if (value < hdRecordSceneTable.num_video_rec_scenes)
            {
                uint32 offset = hdRecordSceneTable.num_voice_rec_scenes + 1;//1:cts verifier offset
                int32 HDRecScene = value + offset;
                mAudioSpeechEnhanceInfoInstance->SetHDRecScene(HDRecScene);
            }
            else
            {
                ALOGE("HDREC_SET_VIDEO_MODE=%d exceed max value(%d)\n", value, hdRecordSceneTable.num_video_rec_scenes);
            }
            break;
        }
#endif
        //<---for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration)
        // calibrate speech parameters
        if (param.getInt(keySpeechParams_Update, value) == NO_ERROR)
        {
            ALOGD("setParameters Update Speech Parames");
            if (value == 0)
            {
                AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
                GetNBSpeechParamFromNVRam(&eSphParamNB);
                SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(&eSphParamNB);
            }
#if defined(MTK_WB_SPEECH_SUPPORT)
            else if (value == 1)
            {
                AUDIO_CUSTOM_WB_PARAM_STRUCT eSphParamWB;
                GetWBSpeechParamFromNVRam(&eSphParamWB);
                SpeechEnhancementController::GetInstance()->SetWBSpeechParametersToAllModem(&eSphParamWB);
            }
#endif

            if (ModeInCall(mMode) == true) // get output device for in_call, and set speech mode
            {
                const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
                const audio_devices_t input_device  = (audio_devices_t)mAudioResourceManager->getUlInputDevice();
                mSpeechDriverFactory->GetSpeechDriver()->SetSpeechMode(input_device, output_device);
            }
            param.remove(keySpeechParams_Update);
            break;
        }
#if defined(MTK_DUAL_MIC_SUPPORT)
        if (param.getInt(keyDualMicParams_Update, value) == NO_ERROR)
        {
            param.remove(keyDualMicParams_Update);
            AUDIO_CUSTOM_EXTRA_PARAM_STRUCT eSphParamDualMic;
            GetDualMicSpeechParamFromNVRam(&eSphParamDualMic);
            SpeechEnhancementController::GetInstance()->SetDualMicSpeechParametersToAllModem(&eSphParamDualMic);

            if (ModeInCall(mMode) == true) // get output device for in_call, and set speech mode
            {
                const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
                const audio_devices_t input_device  = (audio_devices_t)mAudioResourceManager->getUlInputDevice();
                mSpeechDriverFactory->GetSpeechDriver()->SetSpeechMode(input_device, output_device);
            }
            break;
        }
#endif
        // calibrate speech volume
        if (param.getInt(keySpeechVolume_Update, value) == NO_ERROR)
        {
            ALOGD("setParameters Update Speech volume");
            mAudioVolumeInstance->initVolumeController();
            if (ModeInCall(mMode) == true)
            {
                int32_t outputDevice = mAudioResourceManager->getDlOutputDevice();
                SpeechPhoneCallController *pSpeechPhoneCallController = SpeechPhoneCallController::GetInstance();
#ifndef MTK_AUDIO_GAIN_TABLE
                mAudioVolumeInstance->setVoiceVolume(mAudioVolumeInstance->getVoiceVolume(), mMode, (uint32)outputDevice);
#endif
                switch (outputDevice)
                {
                    case AUDIO_DEVICE_OUT_WIRED_HEADSET :
                    {
#ifdef  MTK_TTY_SUPPORT
                        if (pSpeechPhoneCallController->GetTtyCtmMode() == AUD_TTY_VCO)
                        {
                            mAudioVolumeInstance->ApplyMicGain(Normal_Mic, mMode);
                        }
                        else if (pSpeechPhoneCallController->GetTtyCtmMode() == AUD_TTY_HCO || pSpeechPhoneCallController->GetTtyCtmMode() == AUD_TTY_FULL)
                        {
                            mAudioVolumeInstance->ApplyMicGain(TTY_CTM_Mic, mMode);
                        }
                        else
                        {
                            mAudioVolumeInstance->ApplyMicGain(Headset_Mic, mMode);
                        }
#else
                        mAudioVolumeInstance->ApplyMicGain(Headset_Mic, mMode);
#endif
                        break;
                    }
                    case AUDIO_DEVICE_OUT_WIRED_HEADPHONE :
                    {
#ifdef  MTK_TTY_SUPPORT
                        if (pSpeechPhoneCallController->GetTtyCtmMode() == AUD_TTY_VCO)
                        {
                            mAudioVolumeInstance->ApplyMicGain(Normal_Mic, mMode);
                        }
                        else if (pSpeechPhoneCallController->GetTtyCtmMode() == AUD_TTY_HCO || pSpeechPhoneCallController->GetTtyCtmMode() == AUD_TTY_FULL)
                        {
                            mAudioVolumeInstance->ApplyMicGain(TTY_CTM_Mic, mMode);
                        }
                        else
                        {
                            mAudioVolumeInstance->ApplyMicGain(Handfree_Mic, mMode);
                        }
#else
                        mAudioVolumeInstance->ApplyMicGain(Handfree_Mic, mMode);
#endif
                        break;
                    }
                    case AUDIO_DEVICE_OUT_SPEAKER:
                    {
                        mAudioVolumeInstance->ApplyMicGain(Handfree_Mic, mMode);
                        break;
                    }
                    case AUDIO_DEVICE_OUT_EARPIECE:
                    {
                        mAudioVolumeInstance->ApplyMicGain(Normal_Mic, mMode);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else
            {
                setMasterVolume(mAudioVolumeInstance->getMasterVolume());
            }
            param.remove(keySpeechVolume_Update);
            break;
        }
        // ACF/HCF parameters calibration
        if (param.getInt(keyACFHCF_Update, value) == NO_ERROR)
        {
            mAudioMTKStreamManager->UpdateACFHCF(value);
            param.remove(keyACFHCF_Update);
            break;
        }
        // HD recording and DMNR calibration
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

        if (param.get(keyDUALMIC_IN_FILE_NAME, value_str) == NO_ERROR)
        {
            if (mAudioTuningInstance->setPlaybackFileName(value_str.string()) == NO_ERROR)
            {
                param.remove(keyDUALMIC_IN_FILE_NAME);
            }
            break;
        }

        if (param.get(keyDUALMIC_OUT_FILE_NAME, value_str) == NO_ERROR)
        {
            if (mAudioTuningInstance->setRecordFileName(value_str.string()) == NO_ERROR)
            {
#ifndef DMNR_TUNNING_AT_MODEMSIDE
                if (mAudioSpeechEnhanceInfoInstance->SetHDRecVMFileName(value_str.string()) == NO_ERROR)
#endif
                    param.remove(keyDUALMIC_OUT_FILE_NAME);
            }
            break;
        }

        if (param.getInt(keyDUALMIC_SET_UL_GAIN, value) == NO_ERROR)
        {
            if (mAudioTuningInstance->setDMNRGain(AUD_MIC_GAIN, value) == NO_ERROR)
            {
                param.remove(keyDUALMIC_SET_UL_GAIN);
            }
            break;
        }

        if (param.getInt(keyDUALMIC_SET_DL_GAIN, value) == NO_ERROR)
        {
            if (mAudioTuningInstance->setDMNRGain(AUD_RECEIVER_GAIN, value) == NO_ERROR)
            {
                param.remove(keyDUALMIC_SET_DL_GAIN);
            }
            break;
        }

        if (param.getInt(keyDUALMIC_SET_HSDL_GAIN, value) == NO_ERROR)
        {
            if (mAudioTuningInstance->setDMNRGain(AUD_HS_GAIN, value) == NO_ERROR)
            {
                param.remove(keyDUALMIC_SET_HSDL_GAIN);
            }
            break;
        }

        if (param.getInt(keyDUALMIC_SET_UL_GAIN_HF, value) == NO_ERROR)
        {
            if (mAudioTuningInstance->setDMNRGain(AUD_MIC_GAIN_HF, value) == NO_ERROR)
            {
                param.remove(keyDUALMIC_SET_UL_GAIN_HF);
            }
            break;
        }
#endif

        if (param.getInt(keyHDRecTunningEnable, value) == NO_ERROR)
        {
            ALOGD("keyHDRecTunningEnable=%d", value);
            bool bEnable = value;
            mAudioSpeechEnhanceInfoInstance->SetHDRecTunningEnable(bEnable);
            param.remove(keyHDRecTunningEnable);
            break;
        }

        if (param.get(keyHDRecVMFileName, value_str) == NO_ERROR)
        {
            ALOGD("keyHDRecVMFileName=%s", value_str.string());
            if (mAudioSpeechEnhanceInfoInstance->SetHDRecVMFileName(value_str.string()) == NO_ERROR)
            {
                param.remove(keyHDRecVMFileName);
            }
            break;
        }

        // --->for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration)

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

        // VoIP Dual Mic Noise Reduction, DMNR for Receiver
        if (param.getInt(keySET_VOIP_RECEIVER_DMNR_ENABLE, value) == NO_ERROR)
        {
            param.remove(keySET_VOIP_RECEIVER_DMNR_ENABLE);
            mAudioSpeechEnhanceInfoInstance->SetDynamicVoIPSpeechEnhancementMask(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR, (bool)value);
            break;
        }

        // VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
        if (param.getInt(keySET_VOIP_LSPK_DMNR_ENABLE, value) == NO_ERROR)
        {
            param.remove(keySET_VOIP_LSPK_DMNR_ENABLE);
            mAudioSpeechEnhanceInfoInstance->SetDynamicVoIPSpeechEnhancementMask(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR, (bool)value);
            break;
        }

#endif

        // Voice Clarity Engine, VCE
        if (param.getInt(keySET_VCE_ENABLE, value) == NO_ERROR)
        {
            param.remove(keySET_VCE_ENABLE);
            SpeechEnhancementController::GetInstance()->SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_VCE, (bool)value);
            break;
        }

        // Magic Conference Call
        if (param.getInt(keySET_MAGIC_CON_CALL_ENABLE, value) == NO_ERROR)
        {
            param.remove(keySET_MAGIC_CON_CALL_ENABLE);

            const bool magic_conference_call_on = (bool)value;

            // enable/disable flag
            SpeechEnhancementController::GetInstance()->SetMagicConferenceCallOn(magic_conference_call_on);

            // apply speech mode
            if (ModeInCall(mMode) == true)
            {
                const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
                const audio_devices_t input_device  = (audio_devices_t)mAudioResourceManager->getUlInputDevice();
                if (output_device == AUDIO_DEVICE_OUT_SPEAKER)
                {
                    mSpeechDriverFactory->GetSpeechDriver()->SetSpeechMode(input_device, output_device);
                }
            }

            if (SpeechEnhancementController::GetInstance()->GetMagicConferenceCallOn() == true &&
                SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) == true)
            {
                ALOGE("Cannot open MagicConCall & LoudSpeaker DMNR at the same time!!");
            }

            break;
        }


        // Loopback use speaker or not
        static bool bForceUseLoudSpeakerInsteadOfReceiver = false;
        if (param.getInt(keySET_LOOPBACK_USE_LOUD_SPEAKER, value) == NO_ERROR)
        {
            param.remove(keySET_LOOPBACK_USE_LOUD_SPEAKER);
            bForceUseLoudSpeakerInsteadOfReceiver = value & 0x1;
            break;
        }

        // Assign delay frame for modem loopback // 1 frame = 20ms
        if (param.getInt(keySET_LOOPBACK_MODEM_DELAY_FRAMES, value) == NO_ERROR)
        {
            param.remove(keySET_LOOPBACK_MODEM_DELAY_FRAMES);
            SpeechDriverInterface *pSpeechDriver = NULL;
            for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
            {
                pSpeechDriver = mSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
                if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
                {
                    pSpeechDriver->SetAcousticLoopbackDelayFrames((int32_t)value);
                }
            }

            break;
        }

        // Loopback
        if (param.get(keySET_LOOPBACK_TYPE, value_str) == NO_ERROR)
        {
            param.remove(keySET_LOOPBACK_TYPE);

            // parse format like "SET_LOOPBACK_TYPE=1" / "SET_LOOPBACK_TYPE=1+0"
            int type_value = NO_LOOPBACK;
            int device_value = -1;
            sscanf(value_str.string(), "%d,%d", &type_value, &device_value);
            ALOGV("type_value = %d, device_value = %d", type_value, device_value);

            const loopback_t loopback_type = (loopback_t)type_value;
            loopback_output_device_t loopback_output_device;

            if (loopback_type == NO_LOOPBACK) // close loopback
            {
                LoopbackManager::GetInstance()->SetLoopbackOff();
            }
            else // open loopback
            {
                if (device_value == LOOPBACK_OUTPUT_RECEIVER ||
                    device_value == LOOPBACK_OUTPUT_EARPHONE ||
                    device_value == LOOPBACK_OUTPUT_SPEAKER) // assign output device
                {
                    loopback_output_device = (loopback_output_device_t)device_value;
                }
                else // not assign output device
                {
                    if (AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_WIRED_HEADSET,   "") == android_audio_legacy::AudioSystem::DEVICE_STATE_AVAILABLE ||
                        AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_WIRED_HEADPHONE, "") == android_audio_legacy::AudioSystem::DEVICE_STATE_AVAILABLE)
                    {
                        loopback_output_device = LOOPBACK_OUTPUT_EARPHONE;
                    }
                    else if (bForceUseLoudSpeakerInsteadOfReceiver == true)
                    {
                        loopback_output_device = LOOPBACK_OUTPUT_SPEAKER;
                    }
                    else
                    {
                        loopback_output_device = LOOPBACK_OUTPUT_RECEIVER;
                    }
                }
                LoopbackManager::GetInstance()->SetLoopbackOn(loopback_type, loopback_output_device);
            }
            break;
        }


#ifdef  MTK_TTY_SUPPORT
        // Set TTY mode
        if (param.get(keySetTtyMode, value_str) == NO_ERROR)
        {
            param.remove(keySetTtyMode);
            tty_mode_t tty_mode;

            if (value_str == "tty_full")
            {
                tty_mode = AUD_TTY_FULL;
            }
            else if (value_str == "tty_vco")
            {
                tty_mode = AUD_TTY_VCO;
            }
            else if (value_str == "tty_hco")
            {
                tty_mode = AUD_TTY_HCO;
            }
            else if (value_str == "tty_off")
            {
                tty_mode = AUD_TTY_OFF;
            }
            else
            {
                ALOGD("setParameters tty_mode error !!");
                tty_mode = AUD_TTY_ERR;
            }

            SpeechPhoneCallController::GetInstance()->SetTtyCtmMode(tty_mode, mMode);
            break;
        }
#endif
#if defined(EVDO_DT_SUPPORT)
        //Analog FM enable
        if (param.getInt(keySET_WARNING_TONE, value) == NO_ERROR)
        {
            param.remove(keySET_WARNING_TONE);
            ALOGD("keySET_WARNING_TONE=%d, GetActiveModemIndex=%d \n", value, mSpeechDriverFactory->GetActiveModemIndex());

            mSpeechDriverFactory->GetSpeechDriver()->SetWarningTone(value);
            break;
        }
        if (param.getInt(keySTOP_WARNING_TONE, value) == NO_ERROR)
        {
            param.remove(keySTOP_WARNING_TONE);
            mSpeechDriverFactory->GetSpeechDriver()->StopWarningTone();
            break;
        }
        if (param.getInt(keySET_VOICE_VOLUME_INDEX, value) == NO_ERROR)
        {
            param.remove(keySET_VOICE_VOLUME_INDEX);
            ALOGD("keySET_VOICE_VOLUME_INDEX=%d, GetActiveModemIndex=%d \n", value, mSpeechDriverFactory->GetActiveModemIndex());

            modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
            if (modem_index == MODEM_EXTERNAL)
            {
                ALOGD("SpeechDriver ModemType=EVDO");
                mSpeechDriverFactory->GetSpeechDriverByIndex(modem_index)->SetDownlinkGain(value);
            }
            break;
        }
#endif
        if (param.getInt(keySetBTMode, value) == NO_ERROR)
        {
            param.remove(keySetBTMode);

            if ((0 != value) && (1 != value))
            {
                ALOGD("setParameters BTMode error, no support mode %d !!", value);
                value = 0;
            }

            mAudioBTCVSDControl->BT_SCO_SetMode(value);
#if 1 //mark temp, should unmark for phone call WB BTSCO
            SpeechPhoneCallController::GetInstance()->SetBTMode(value);
#endif
            break;
        }

        if (param.getInt(keyEnableStereoOutput, value) == NO_ERROR)
        {
            ALOGD("keyEnableStereoOutput=%d", value);
            mAudioMTKStreamManager->setParametersToStreamOut(keyValuePairs);
            param.remove(keyEnableStereoOutput);
            break;
        }

    }
    while (0);

    if (param.size())
    {
        ALOGE("%s() still have param.size() = %d, remain param = \"%s\"", __FUNCTION__, param.size(), param.toString().string());
        status = BAD_VALUE;
    }
    ALOGD("-setCommonParameters(): %s ", keyValuePairs.string());

    return status;
}


String8 AudioMTKHardware::getCommonParameters(AudioParameter &param, AudioParameter &returnParam)
{
    String8 value;
    int cmdType = 0;

    do
    {
        if (param.get(keyGetIsWiredHeadsetOn, value) == NO_ERROR)
        {
            bool isWiredHeadsetOn = mAudioResourceManager->IsWiredHeadsetOn();
            value = (isWiredHeadsetOn) ? "true" : "false";
            param.remove(keyGetIsWiredHeadsetOn);
            returnParam.add(keyGetIsWiredHeadsetOn, value);
            break;
        }

        if (param.get(keyGetFmEnable, value) == NO_ERROR)
        {
            AudioFMController *pAudioFMController = AudioFMController::GetInstance();
            const bool rx_status       = pAudioFMController->GetFmEnable();
            const bool fm_power_status = pAudioFMController->GetFmChipPowerInfo();
            value = (rx_status && fm_power_status) ? "true" : "false";
            param.remove(keyGetFmEnable);
            returnParam.add(keyGetFmEnable, value);
            break;
        }

#if defined(MTK_DUAL_MIC_SUPPORT)
        if (param.getInt(keyDUALMIC_GET_GAIN, cmdType) == NO_ERROR)
        {
            unsigned short gain = 0;
            char buf[32];

            if (mAudioTuningInstance->getDMNRGain((unsigned short)cmdType, &gain) == NO_ERROR)
            {
                sprintf(buf, "%d", gain);
                returnParam.add(keyDUALMIC_GET_GAIN, String8(buf));
                param.remove(keyDUALMIC_GET_GAIN);
            }
            break;
        }
#endif

        if (param.get(keyMusicPlusGet, value) == NO_ERROR)
        {
#ifndef HIFI_SWITCH_BY_AUDENH  //HP switch use AudEnh setting
            bool musicplus_status = mAudioMTKStreamManager->GetMusicPlusStatus();
#else
            bool musicplus_status = mAudioMTKStreamManager->GetHiFiDACStatus();
#endif
            value = (musicplus_status) ? "1" : "0";
            param.remove(keyMusicPlusGet);
            returnParam.add(keyMusicPlusGet, value);
            break;
        }
        if (param.get(keyBesLoudnessGet, value) == NO_ERROR)
        {

            bool besloudness_status = mAudioMTKStreamManager->GetBesLoudnessStatus();
            value = (besloudness_status) ? "1" : "0";
            param.remove(keyBesLoudnessGet);
            returnParam.add(keyBesLoudnessGet, value);
            break;
        }

        if (param.get(keyHiFiDACGet, value) == NO_ERROR)
        {
            bool hifidac_status = mAudioMTKStreamManager->GetHiFiDACStatus();
            value = (hifidac_status) ? "1" : "0";
            param.remove(keyHiFiDACGet);
            returnParam.add(keyHiFiDACGet, value);
            break;
        }


        // Dual Mic Noise Reduction, DMNR for Receiver
        if (param.get(keyGet_Dual_Mic_Setting, value) == NO_ERROR) // new name
        {
            param.remove(keyGet_Dual_Mic_Setting);
            value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_DMNR) > 0) ? "1" : "0";
            returnParam.add(keyGet_Dual_Mic_Setting, value);
            break;
        }

        // Dual Mic Noise Reduction, DMNR for Loud Speaker
        if (param.get(keyGET_LSPK_DMNR_ENABLE, value) == NO_ERROR) // new name
        {
            param.remove(keyGET_LSPK_DMNR_ENABLE);
            value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) > 0) ? "1" : "0";
            returnParam.add(keyGET_LSPK_DMNR_ENABLE, value);
            break;
        }


        // Voice Clarity Engine, VCE
        if (param.get(keyGET_VCE_ENABLE, value) == NO_ERROR) // new name
        {
            param.remove(keyGET_VCE_ENABLE);
            value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_VCE) > 0) ? "1" : "0";
            returnParam.add(keyGET_VCE_ENABLE, value);
            break;
        }
        if (param.get(keyGET_VCE_STATUS, value) == NO_ERROR) // old name
        {
            param.remove(keyGET_VCE_STATUS);
            value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_VCE) > 0) ? "1" : "0";
            returnParam.add(keyGET_VCE_STATUS, value);
            break;
        }

        // Magic Conference Call
        if (param.get(keyGET_MAGIC_CON_CALL_ENABLE, value) == NO_ERROR) // new name
        {
            param.remove(keyGET_MAGIC_CON_CALL_ENABLE);
            value = (SpeechEnhancementController::GetInstance()->GetMagicConferenceCallOn() == true) ? "1" : "0";
            returnParam.add(keyGET_MAGIC_CON_CALL_ENABLE, value);
            break;
        }

        // VoIP Dual Mic Noise Reduction, DMNR for Receiver
        if (param.get(keyGET_VOIP_RECEIVER_DMNR_ENABLE, value) == NO_ERROR)
        {
            param.remove(keyGET_VOIP_RECEIVER_DMNR_ENABLE);
            value = (mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR) > 0) ? "1" : "0";
            returnParam.add(keyGET_VOIP_RECEIVER_DMNR_ENABLE, value);
            break;
        }

        // VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
        if (param.get(keyGET_VOIP_LSPK_DMNR_ENABLE, value) == NO_ERROR)
        {
            param.remove(keyGET_VOIP_LSPK_DMNR_ENABLE);
            value = (mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) > 0) ? "1" : "0";
            returnParam.add(keyGET_VOIP_LSPK_DMNR_ENABLE, value);
            break;
        }

        // Audio Volume version
        if (param.get(keyGET_AUDIO_VOLUME_VER, value) == NO_ERROR)
        {
            param.remove(keyGET_AUDIO_VOLUME_VER);
            value = "1";
            returnParam.add(keyGET_AUDIO_VOLUME_VER, value);
            break;
        }

        // check if the LR channel switched
        if (param.get(keyLR_ChannelSwitch, value) == NO_ERROR)
        {
#ifdef MTK_DUAL_MIC_SUPPORT
            char buf[32];
            bool bIsLRSwitch = mAudioSpeechEnhanceInfoInstance->GetRecordLRChannelSwitch();
            sprintf(buf, "%d", bIsLRSwitch);
            returnParam.add(keyLR_ChannelSwitch, String8(buf));
            ALOGD("LRChannelSwitch=%d", bIsLRSwitch);
#else
            ALOGD("only support in dual MIC");
#endif
            param.remove(keyLR_ChannelSwitch);
            break;
        }
    }
    while (0);

    String8 keyValuePairs = returnParam.toString();
    ALOGD("-%s(), keyValuePairs = %s", __FUNCTION__, keyValuePairs.string());
    return keyValuePairs;
}


#endif

