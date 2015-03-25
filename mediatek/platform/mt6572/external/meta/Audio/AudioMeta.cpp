/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   AudioMeta.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Audio HW Testing Tool for META
 *
 * Author:
 * -------
 *   Stan Huang
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 01 11 2013 changqing.yan
 * [ALPS00435826] [Factory Mode] AutoTest Fail in Loopback(Spk-Mic)
 * .
 *
 * 12 19 2012 changqing.yan
 * [ALPS00409438] [Need Patch] [Volunteer Patch]JB2 build error
 * .
 *
 *
 * 28 10 2012 ship.hsu
 * modify for 6589
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/


/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include <hardware_legacy/AudioHardwareInterface.h>

#include <AudioFtmBase.h>
#include "AudioMeta.h"

#ifdef __cplusplus
extern "C" {
#include "DIF_FFT.h"
#include "Audio_FFT_Types.h"
#include "Audio_FFT.h"
#endif

#include "meta.h"
//#include "FT_Cmd_Para.h"
#include "FT_Public.h"

using namespace android;

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/
#define TEMP_FOR_DUALMIC
#define  AUD_DL1_USE_SLAVE

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "AudioMETA"
#define LOGV META_LOG
#define META_PREFIX "AudioMETA,"

#ifndef META_LOG
#define META_LOG ALOGD
#endif

//#define META_LOG LOGD
/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

//keep same as 6577
#define AUDIO_APPLY_MAX_GAIN (0xffff)
#define AUDIO_APPLY_BIG_GAIN (0xcccc)

#define PEER_BUF_SIZE 2*1024
#define ADVANCED_META_MODE 5

/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/
enum audio_devices
{
    // output devices
    OUT_EARPIECE = 0,
    OUT_SPEAKER = 1,
    OUT_WIRED_HEADSET = 2,
    DEVICE_OUT_WIRED_HEADPHONE = 3,
    DEVICE_OUT_BLUETOOTH_SCO = 4
};


/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/


/*****************************************************************************
*                   G L O B A L      V A R I A B L E
******************************************************************************
*/

/*
Mutex mLock;
Mutex mLockStop;
Condition mWaitWorkCV;
Condition mWaitStopCV;

static int META_SetEMParameter( void *audio_par );
static int META_GetEMParameter( void *audio_par );
static void Audio_Set_Speaker_Vol(int level);
static void Audio_Set_Speaker_On(int Channel);
static void Audio_Set_Speaker_Off(int Channel);
*/

static android::AudioFtmBase *mAudioFtm;
static bool bMetaAudioInited = false;
/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/
//#define GENERIC_AUDIO   //for test

#ifndef GENERIC_AUDIO

static android_audio_legacy::AudioHardwareInterface *gAudioHardware = NULL;
static android_audio_legacy::AudioStreamIn *gAudioStreamIn = NULL;

static void *AudioRecordControlLoop(void *arg)
{
    return NULL;
}
static void Audio_Set_Speaker_Vol(int level)
{
    LOGD("Audio_Set_Speaker_Vol with level = %d", level);
}

static void Audio_Set_Speaker_On(int Channel)
{
    META_LOG("Audio_Set_Speaker_On Channel = %d\n", Channel);
}

static void Audio_Set_Speaker_Off(int Channel)
{
    META_LOG("Audio_Set_Speaker_Off Channel = %d\n", Channel);

}

bool META_Audio_init(void)
{
    META_LOG("+META_Audio_init");
    if (gAudioHardware == NULL)
    {
        gAudioHardware = android_audio_legacy::createAudioHardware();
    }
    mAudioFtm = android::AudioFtmBase::createAudioFtmInstance();
    //mAudioFtm->RequestClock();
    bMetaAudioInited = true;
    META_LOG("-META_Audio_init");
    return true;
}


bool META_Audio_deinit()
{
    META_LOG("META_Audio_deinit bMetaAudioInited = %d", bMetaAudioInited);
    //mAudioFtm->ReleaseClock();

    return true;
}

bool RecieverLoopbackTest(char echoflag)
{
    META_LOG("RecieverLoopbackTest echoflag=%d", echoflag);
    if (echoflag)
    {
        mAudioFtm->PhoneMic_Receiver_Loopback(MIC1_ON);
    }
    else
    {
        mAudioFtm->PhoneMic_Receiver_Loopback(MIC1_OFF);
    }
    return true;
}

bool RecieverLoopbackTest_Mic2(char echoflag)
{
    LOGD("RecieverLoopbackTest_Mic2 echoflag=%d", echoflag);
    if (echoflag)
    {
        mAudioFtm->PhoneMic_Receiver_Loopback(MIC2_ON);
    }
    else
    {
        mAudioFtm->PhoneMic_Receiver_Loopback(MIC2_OFF);
    }
    return true;
}


bool EarphoneLoopbackTest(char echoflag)
{
    META_LOG("EarphoneLoopbackTest echoflag = %d", echoflag);
    return mAudioFtm->PhoneMic_EarphoneLR_Loopback(echoflag);
}


static int HeadsetMic_EarphoneLR_Loopback(char bEnable)
{
    META_LOG("HeadsetMic_EarphoneLR_Loopback bEnable = %d", bEnable);
    return mAudioFtm->HeadsetMic_EarphoneLR_Loopback(bEnable, true);
}


bool RecieverTest(char receiver_test)
{
    META_LOG("RecieverTest receiver_test=%d", receiver_test);
    mAudioFtm->RecieverTest(receiver_test);
    return true;
}

bool LouderSPKTest(char left_channel, char right_channel)
{
    META_LOG("LouderSPKTest left_channel=%d, right_channel=%d", left_channel, right_channel);
    //mAudioFtm->Afe_Enable_SineWave(0);  //need to turnoff before use
    //mAudioFtm->LouderSPKTest(0, 0);
    mAudioFtm->LouderSPKTest(left_channel, right_channel);
    return true;
}


bool EarphoneTest(char bEnable)
{
    META_LOG("EarphoneTest bEnable=%d", bEnable);
    mAudioFtm->EarphoneTest(bEnable);
    return true;
}

bool FMLoopbackTest(char bEnable) //ship to do
{
    META_LOG("FMLoopbackTest bEnable = %d", bEnable);
    mAudioFtm->FMLoopbackTest(bEnable);
    /*
    if(bEnable){
        Audio_Set_Speaker_On(Channel_Stereo);
    }else{
        Audio_Set_Speaker_Off(Channel_Stereo);
    }
    */
    return true;
}

int Audio_I2S_Play(int enable_flag)     //ship to do
{
    LOGD("[META] Audio_I2S_Play");
    mAudioFtm->Audio_FM_I2S_Play(enable_flag);
    return true;
}

int Audio_FMTX_Play(bool Enable, unsigned int Freq)
{

    return true;
    //return mAudFtm->WavGen_SW_SineWave(Enable, Freq, 0); // 0: FM-Tx, 1: HDMI
}

bool EarphoneMicbiasEnable(bool bMicEnable)
{
    META_LOG("EarphoneMicbiasEnable bEnable = %d", bMicEnable);

    return true;
}

static int META_SetEMParameter(void *audio_par)
{
    int WriteCount = 0;
    //android::SetCustParamToNV( (AUDIO_CUSTOM_PARAM_STRUCT *)audio_par);
    return WriteCount;
}

static int META_GetEMParameter(void *audio_par)
{
    int ReadConut = 0;
    //android::GetCustParamFromNV( (AUDIO_CUSTOM_PARAM_STRUCT *)audio_par);

    return ReadConut;
}

static int META_SetACFParameter(void *audio_par)
{
    int WriteCount = 0;
    //android::SetAudioCompFltCustParamToNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);
    return WriteCount;
}

static int META_GetACFParameter(void *audio_par)
{
    int ReadConut = 0;
    //android::GetAudioCompFltCustParamFromNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);
    return ReadConut;
}

static int META_SetACFPreviewParameter(void *audio_par)
{
    int WriteCount = 0;
    //set to working buffer

    //android::AudioSystem::SetACFPreviewParameter( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    return WriteCount;
}

static int META_SetHCFParameter(void *audio_par)
{
    int WriteCount = 0;
    //android::SetHeadphoneCompFltCustParamToNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);
    return WriteCount;
}

static int META_GetHCFParameter(void *audio_par)
{
    int ReadConut = 0;
    //android::GetHeadphoneCompFltCustParamFromNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);

    return ReadConut;
}

static int META_SetHCFPreviewParameter(void *audio_par)
{
    int WriteCount = 0;
    //set to working buffer

    //android::AudioSystem::SetHCFPreviewParameter( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    return WriteCount;
}

static void META_Load_Volume(int var)
{
    //android::AudioSystem::SetAudioCommand(0x50,0x0);
    return;
}


//<--- add for dual mic support on advanced meta mode
static META_BOOL SetPlaybackFile(const char *fileName)
{
    FILE *fp;
    LOGD("SetPlaybackFile() file name %s", fileName);

    return true;
}

static META_BOOL DownloadDataToFile(const char *fileName, char *data, unsigned short size)
{
    FILE *fp;
    LOGV("DownloadDataToFile() file name %s, data 0x%x, size %d", fileName, data, size);

    return true;
}

static META_BOOL DualMICRecorder(FT_L4AUD_REQ *req, FT_L4AUD_CNF *audio_par)
{
    int ret = 0;

    return true;

}


static META_BOOL StopDualMICRecorder()
{
    LOGV("StopDualMICRecorder():Stop dual mic recording ");
    return true;
}

static META_BOOL UplinkDataToPC(ft_l4aud_ul_data_package_req &uplike_par, void *audio_par, unsigned char *pBuff)
{
    FILE *fp;
    int uplinkdatasize = 0;
    static long mCurrFilePosition = 0;
    static char mLastFileName[256] = "hello";

    return true;
}

static META_BOOL setParameters(ft_l4aud_dualmic_set_params_req &set_par)
{
    char mParams[128];
    /*
    if (0==strlen(set_par.param))
    {
        LOGE("parameters name is null");
        return false;
    }

    sprintf(mParams, "%s=%d", set_par.param, set_par.value);
    AudioSystem::setParameters(0, String8(mParams));
    */
    LOGD("META_BOOL setParameters");
    return true;
}

static META_BOOL getParameters(ft_l4aud_dualmic_get_params_req &get_par, void *audio_par)
{
    //LOGD("getParameters: param name %s, param value %d", get_par->param_name, get_par->value);
    LOGD("META_BOOL getParameters");
    return true;
}
//---> add for dual mic support on advanced meta mode

void META_Audio_OP(FT_L4AUD_REQ *req, char *peer_buff, unsigned short peer_len)
{
    META_BOOL ret = true;
    unsigned char pBuff[PEER_BUF_SIZE];
    unsigned short mReadSize = 0;
    FT_L4AUD_CNF audio_cnf;
    memset(&audio_cnf, 0, sizeof(FT_L4AUD_CNF));
    audio_cnf.header.id = FT_L4AUD_CNF_ID;
    audio_cnf.header.token = req->header.token;
    audio_cnf.op = req->op;
    audio_cnf.status = META_SUCCESS;

    META_LOG("+META_Audio_OP");
#if 0
    if (bMetaAudioInited == FALSE)
    {
        META_LOG("META_Audio_OP not initialed \r");
        audio_cnf.status = META_FAILED;
        WriteDataToPC(&audio_cnf, sizeof(FT_L4AUD_CNF), NULL, 0);
    }
#endif
    META_LOG("META_Audio_OP req->op=%d \r", req->op);

    switch (req->op)
    {
        case FT_L4AUD_OP_SET_PARAM_SETTINGS_0809:
        {
            META_LOG("META_Audio_OP, Audio Set Param Req \r");

            break;
        }

        case FT_L4AUD_OP_GET_PARAM_SETTINGS_0809:
        {
            META_LOG("META_Audio_OP, Audio Get Param Req\r\n");
            break;
        }
        case FT_L4AUD_OP_SET_ACF_COEFFS:
        {
            META_LOG("META_Audio_OP, Audio Set ACF Param Req \r");
            break;
        }

        case FT_L4AUD_OP_GET_ACF_COEFFS:
        {
            META_LOG("META_Audio_OP, Audio Get ACF Param Req\r\n");
            break;
        }
        case FT_L4AUD_OP_SET_PREVIEW_ACF_COEFFS:
        {
            META_LOG("META_Audio_OP, Audio Set ACF Preview Param Req\r\n");

            break;
        }
        case FT_L4AUD_OP_SET_HCF_COEFFS:
        {
            META_LOG("META_Audio_OP, Audio Set HCF Param Req \r");

            break;
        }

        case FT_L4AUD_OP_GET_HCF_COEFFS:
        {
            META_LOG("META_Audio_OP, Audio Get HCF Param Req\r\n");

            break;
        }
        case FT_L4AUD_OP_SET_PREVIEW_HCF_COEFFS:
        {
            META_LOG("META_Audio_OP, Audio Set HCF Preview Param Req\r\n");

            break;
        }

        case FT_L4AUD_OP_SET_ECHO:   //ship to do
        {
            META_LOG("META_Audio_OP, Loopback test \r\n");
            ft_l4aud_set_echo *par;
            par = (ft_l4aud_set_echo *)&req->req;
            ret = RecieverLoopbackTest(par->echoflag);
            break;
        }
        case FT_L4AUD_OP_MIC2_LOOPBACK:   //ship to do
        {
            LOGD("META_Audio_OP, MIC2 Loopback test \r\n");
            ft_l4aud_set_echo *par;
            par = (ft_l4aud_set_echo *)&req->req;
            LOGD("ft_l4aud_set_echo->req->echoflag: %d \r\n", par->echoflag);
            ret = RecieverLoopbackTest_Mic2(par->echoflag);
            break;
        }
        case FT_L4AUD_OP_RECEIVER_TEST:    //ship to do
        {
            META_LOG("META_Audio_OP, Receiver test \r\n");
            ft_l4aud_receiver_test *par;
            par = (ft_l4aud_receiver_test *)&req->req;
            ret = RecieverTest((char)par->receiver_test);
            break;
        }
        case FT_L4AUD_OP_LOUDSPK_TEST:    //ship to do
        {
            META_LOG("META_Audio_OP, LoudSpk test \r\n");
            ft_l4aud_loudspk *par;
            par = (ft_l4aud_loudspk *)&req->req;
            ret = LouderSPKTest(par->left_channel, par->right_channel);
            break;
        }
        case FT_L4AUD_OP_EARPHONE_TEST:    //ship to do
        {
            META_LOG("META_Audio_OP, Earphone test \r\n");
            ret = EarphoneTest(req->req.eaphone_test.bEnable);
            break;
        }

        case FT_L4AUD_OP_HEADSET_LOOPBACK_TEST:    //ship to do
        {
            META_LOG("META_Audio_OP, Headset loopback test \r\n");
            ret = HeadsetMic_EarphoneLR_Loopback(req->req.headset_loopback_test.bEnable);
            break;
        }

        case FT_L4AUD_OP_FM_LOOPBACK_TEST:   //ship to do
        {
            META_LOG("META_Audio_OP, FM loopback test \r\n");
            ret = FMLoopbackTest(req->req.fm_loopback_test.bEnable);
            break;
        }

        case FT_L4AUD_OP_SET_PLAYBACK_FILE:
        {
            META_LOG("META_Audio_OP, set playback file \r\n");
            break;
        }

        case FT_L4AUD_OP_DL_DATA_PACKAGE:
        {
            META_LOG("META_Audio_OP, down link data pakage \r\n");
            break;
        }

        case FT_L4AUD_OP_DUALMIC_RECORD:
        {
            META_LOG("META_Audio_OP, dual mic recording \r\n");
            break;
        }

        case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD:
        {
            META_LOG("META_Audio_OP, playback and dual mic recording \r\n");

            break;
        }

        case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS:
        {
            META_LOG("META_Audio_OP, headset playback and dual mic recording \r\n");

            break;
        }

        case FT_L4AUD_OP_STOP_DUALMIC_RECORD:
        {
            META_LOG("META_Audio_OP, stop dual mic recording \r\n");

            break;
        }

        case FT_L4AUD_OP_UL_DATA_PACKAGE:
        {

            break;
        }

        case FT_L4AUD_OP_DUALMIC_SET_PARAMS:
        {
            break;
        }

        case FT_L4AUD_OP_DUALMIC_GET_PARAMS:
        {
            break;
        }

        case FT_L4AUD_OP_LOAD_VOLUME:
        {
            break;
        }
        case FT_L4AUD_OP_GET_GAINTABLE_SUPPORT:
        {
            break;
        }
        case FT_L4AUD_OP_GET_GAINTABLE_NUM:
        {
            ret = true;
            break;
        }
        case FT_L4AUD_OP_GET_GAINTABLE_LEVEL:
        {
            META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_GAINTABLE_LEVEL \r\n");
            ret = true;
            break;
        }
        case FT_L4AUD_OP_GET_CTRPOINT_NUM:
        {
            META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_CTRPOINT_NUM \r\n");
            ret = true;
            break;
        }
        case FT_L4AUD_OP_GET_CTRPOINT_BITS:
        {
            META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_CTRPOINT_BITS \r\n");
            ret = true;
            break;
        }
        case FT_L4AUD_OP_GET_CTRPOINT_TABLE:
        {
            META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_CTRPOINT_TABLE \r\n");

            break;
        }
        default:
            audio_cnf.status = META_FAILED;
            break;
    }

    if (!ret)
    {
        audio_cnf.status = META_FAILED;
    }

    META_LOG("-META_Audio_OP, audio_cnf.status = %d \r", audio_cnf.status);
    WriteDataToPC(&audio_cnf, sizeof(FT_L4AUD_CNF), pBuff, mReadSize);

}
//short pbuffer[512],bytes:512*2
int readRecordData(void *pbuffer, int bytes)
{
    int nBytes = 0;

    if (gAudioStreamIn == NULL)
    {
        android::AudioParameter paramVoiceMode = android::AudioParameter();
        paramVoiceMode.addInt(android::String8("HDREC_SET_VOICE_MODE"), 0);
        gAudioHardware->setParameters(paramVoiceMode.toString());

        uint32_t device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        int format = AUDIO_FORMAT_PCM_16_BIT;
        uint32_t channel = AUDIO_CHANNEL_IN_STEREO;
        uint32_t sampleRate = 48000;
        status_t status = 0;
        gAudioStreamIn = gAudioHardware->openInputStream(device, &format, &channel, &sampleRate, &status, (android_audio_legacy::AudioSystem::audio_in_acoustics)0);
        android::AudioParameter param = android::AudioParameter();
        param.addInt(android::String8(android::AudioParameter::keyRouting), device);
        param.addInt(android::String8(android::AudioParameter::keyInputSource), android_audio_legacy::AUDIO_SOURCE_MIC);
        gAudioStreamIn->setParameters(param.toString());
    }

    nBytes = gAudioStreamIn->read(pbuffer, bytes);
    return nBytes;
}
//short pbuffer[512],bytes:512*2
bool freqCheck(short pbuffer[], int bytes)
{
    short pbufferL[256] = {0};
    short pbufferR[256] = {0};
    int lowFreq = 1000 * (1 - 0.1);
    int highFreq = 1000 * (1 + 0.1);
    for (int i = 0 ; i < 256 ; i++)
    {
        pbufferL[i] = pbuffer[2 * i];
        pbufferR[i] = pbuffer[2 * i + 1];
    }
#if 0
    char filenameL[] = "/data/record_dataL.pcm";
    char filenameR[] = "/data/record_dataR.pcm";
    FILE *fpL = fopen(filenameL, "ab+");
    FILE *fpR = fopen(filenameR, "ab+");

    if (fpL != NULL)
    {
        fwrite(pbufferL, bytes / 2, 1, fpL);
        fclose(fpL);
    }

    if (fpR != NULL)
    {
        fwrite(pbufferR, bytes / 2, 1, fpR);
        fclose(fpR);
    }
#endif
    unsigned int freqDataL[3] = {0}, magDataL[3] = {0};
    unsigned int freqDataR[3] = {0}, magDataR[3] = {0};
    ApplyFFT256(48000, pbufferL, 0, freqDataL, magDataL);
    ApplyFFT256(48000, pbufferR, 0, freqDataR, magDataR);
    if ((freqDataL[0] <= highFreq && freqDataL[0] >= lowFreq) && (freqDataR[0] <= highFreq && freqDataR[0] >= lowFreq))
    {
        return true;
    }
    return false;

}

#else   //GENERIC_AUDIO is defined, dummy function

static void *AudioRecordControlLoop(void *arg) { return NULL; }
static void Audio_Set_Speaker_Vol(int level) {}
static void Audio_Set_Speaker_On(int Channel) {}
static void Audio_Set_Speaker_Off(int Channel) {}
bool META_Audio_init(void) { return true; }
bool META_Audio_deinit() { return true; }
bool RecieverLoopbackTest(char echoflag) { return true; }
bool RecieverLoopbackTest_Mic2(char echoflag) { return true; }
bool RecieverTest(char receiver_test) { return true; }
bool LouderSPKTest(char left_channel, char right_channel) { return true; }
bool EarphoneLoopbackTest(char echoflag) { return true; }
bool EarphoneTest(char bEnable) { return true; }
bool FMLoopbackTest(char bEnable) { return true; }
int Audio_I2S_Play(int enable_flag) { return true; }
int Audio_FMTX_Play(bool Enable, unsigned int Freq) { return true; }
bool EarphoneMicbiasEnable(bool bMicEnable) { return true; }
static int META_SetEMParameter(void *audio_par) { return 0; }
static int META_GetEMParameter(void *audio_par) { return 0; }
static int META_SetACFParameter(void *audio_par) { return 0; }
static int META_GetACFParameter(void *audio_par) { return 0; }
static int META_SetACFPreviewParameter(void *audio_par) { return 0; }
static int META_SetHCFParameter(void *audio_par) { return 0; }
static int META_GetHCFParameter(void *audio_par) { return 0; }
static int META_SetHCFPreviewParameter(void *audio_par) { return 0; }
static void META_Load_Volume(int var) { return; }
static META_BOOL SetPlaybackFile(const char *fileName) { return true; }
static META_BOOL DownloadDataToFile(const char *fileName, char *data, unsigned short size) { return true; }
static META_BOOL DualMICRecorder(FT_L4AUD_REQ *req, FT_L4AUD_CNF *audio_par) { return true; }
static META_BOOL StopDualMICRecorder() { return true; }
static META_BOOL UplinkDataToPC(ft_l4aud_ul_data_package_req &uplike_par, void *audio_par, unsigned char *pBuff) { return true; }
static META_BOOL setParameters(ft_l4aud_dualmic_set_params_req &set_par) { return true; }
static META_BOOL getParameters(ft_l4aud_dualmic_get_params_req &get_par, void *audio_par) { return true; }
void META_Audio_OP(FT_L4AUD_REQ *req, char *peer_buff, unsigned short peer_len) {}
static int HeadsetMic_EarphoneLR_Loopback(char bEnable) { return true; }
int readRecordData(void *pbuffer, int bytes) {return 0;}
bool freqCheck(short pbuffer[], int bytes) {return true;}

#endif  //end ifndefined GENERIC_AUDIO
#ifdef __cplusplus
};
#endif
