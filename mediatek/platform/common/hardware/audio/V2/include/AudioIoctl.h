/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   AudioIoctl.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *    Audio Driver I/O Control
 *
 * Author:
 * -------
 *   Stan Huang (mtk01728)
 *
 *
 *------------------------------------------------------------------------------
 * $Revision: #5 $
 * $Modtime:$
 * $Log:$
 *
 * 04 13 2012 donglei.ji
 * [ALPS00267675] [Need Patch] [Volunteer Patch] modify FM and MATV sample rate setting
 * get FM and MATV I2S sample rate from driver.
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 * 01 05 2012 weiguo.li
 * [ALPS00108538] [Need Patch] [Volunteer Patch]patch from GB2 to alpsDev4.0
 * .
 *
 * 11 09 2011 weiguo.li
 * [ALPS00090825] [Need Patch] [Volunteer Patch]add dcl code to gb2 and alps
 * .
 *
 * 09 28 2011 weiguo.li
 * [ALPS00076254] [Need Patch] [Volunteer Patch]LGE audio driver using Voicebuffer for incall
 * .
 *
 *
 *******************************************************************************/
#ifndef __ANDES_IOCTL_H
#define __ANDES_IOCTL_H


/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include <linux/ioctl.h>

#include "AudioPolicyParameters.h"

/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/
typedef struct
{
    unsigned int  offset;
    unsigned int  value;
    unsigned int  mask;
} Register_Control;

typedef struct
{
    int bSpeechFlag;
    int bBgsFlag;
    int bRecordFlag;
    int bTtyFlag;
    int bVT;
    int bAudioPlay;
} SPH_Control;

struct _Info_Data      //HP
{
    unsigned int info;
    unsigned int param1;
    unsigned int param2;
};

typedef struct
{
    int SampleRate;
    int ClkApllSel; // 0-5
} Hdmi_Clock_Control;

enum SPEAKER_CHANNEL
{
    Channel_None = 0 ,
    Channel_Right,
    Channel_Left,
    Channel_Stereo
};

enum SOUND_PATH
{
    DEFAULT_PATH = 0,
    IN1_PATH,
    IN2_PATH,
    IN3_PATH,
    IN1_IN2_MIX,
};

enum MIC_ANALOG_SWICTH
{
    MIC_ANA_DEFAULT_PATH = 0,
    MIC_ANA_SWITCH1_HIGH
};

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
// routing
static const char *keyAddOutputDevice    = "addoutputdevice";
static const char *keyRemoveOutputDevice = "Removeoutputdevice";
static const char *keyAddIntputDevice    = "addinputdevice";
static const char *keyRemoveIntputDevice = "Removeinputdevice";
// mode
static const char *keyPhoneStateRouting  = "phonestate_routing";
// forceuse
static const char *keyAddtForceuseNormal = "AddForceuseNormal";
static const char *keyAddtForceusePhone = "AddForceusePhone";

// detect headset
static const char *keyDetectHeadset = "DetectHeadset";

// samplerate
static const char *keySetSampleRate = "AudioSetSampleRate";
// FmEnable
static const char *keySetFmEnable = "AudioSetFmEnable";
static const char *keySetFmDigitalEnable = "AudioSetFmDigitalEnable";

static const char *keySetMatvDigitalEnable = "AudioSetMatvDigitalEnable";
//
static const char *keyRouting = "routing";

// keySetForceToSpeaker
static const char *keySetForceToSpeaker = "AudioSetForceToSpeaker";
// keySetFmVolume
static const char *keySetFmVolume = "SetFmVolume";
static const char *keySetMatvVolume = "SetMatvVolume";

// keySetFmTxEnable
static const char *keySetFmTxEnable = "SetFmTxEnable";
static const char *strFMRXForceDisableFMTX = "FMRXForceDisableFMTX";
static const char *keySetLineInEnable = "AtvAudioLineInEnable";
static const char *keySetTtyMode = "tty_mode";
// set VT Speech Call String
static const char *keySetVTSpeechCall = "SetVTSpeechCall";
static const char *keyPhoneMode = "PhoneMode";
// Force turon spealer
static const char *keyForceSpeakerOn = "ForceSpeakerOn";

/*****************************************************************************
*           I / O     C O N T R O L      M E S S A G E      D E F I N E
******************************************************************************
*/
//Audio driver I/O control message
#define AUD_DRV_IOC_MAGIC 'C'


#define SET_AUDSYS_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x00, Register_Control*)
#define GET_AUDSYS_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x01, Register_Control*)
#define SET_ANAAFE_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x02, Register_Control*)
#define GET_ANAAFE_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x03, Register_Control*)
#define AUDDRV_GET_DL1_REMAINDATA_TIME  _IOWR(AUD_DRV_IOC_MAGIC, 0x0A, int)
#define AUDDRV_GET_UL_REMAINDATA_TIME   _IOWR(AUD_DRV_IOC_MAGIC, 0x0B, int)


// Allocate mean allocate buffer and set stream into ready state.
#define ALLOCATE_MEMIF_DL1           _IOWR(AUD_DRV_IOC_MAGIC, 0x10,unsigned int)
#define FREE_MEMIF_DL1                    _IOWR(AUD_DRV_IOC_MAGIC, 0x11,unsigned int)
#define ALLOCATE_MEMIF_DL2           _IOWR(AUD_DRV_IOC_MAGIC, 0x12,unsigned int)
#define FREE_MEMIF_DL2                    _IOWR(AUD_DRV_IOC_MAGIC, 0x13,unsigned int)
#define ALLOCATE_MEMIF_AWB          _IOWR(AUD_DRV_IOC_MAGIC, 0x14,unsigned int)
#define FREE_MEMIF_AWB                  _IOWR(AUD_DRV_IOC_MAGIC, 0x15,unsigned int)
#define ALLOCATE_MEMIF_ADC           _IOWR(AUD_DRV_IOC_MAGIC, 0x16,unsigned int)
#define FREE_MEMIF_ADC                   _IOWR(AUD_DRV_IOC_MAGIC, 0x17,unsigned int)
#define ALLOCATE_MEMIF_DAI           _IOWR(AUD_DRV_IOC_MAGIC, 0x18,unsigned int)
#define FREE_MEMIF_DAI                    _IOWR(AUD_DRV_IOC_MAGIC, 0x19,unsigned int)
#define ALLOCATE_MEMIF_MODDAI    _IOWR(AUD_DRV_IOC_MAGIC, 0x1a,unsigned int)
#define FREE_MEMIF_MODDAI             _IOWR(AUD_DRV_IOC_MAGIC, 0x1b,unsigned int)

// when mediaserver died ,restart for auddriver
#define AUD_RESTART                         _IOWR(AUD_DRV_IOC_MAGIC, 0x1F,unsigned int)

#define START_MEMIF_TYPE               _IOWR(AUD_DRV_IOC_MAGIC, 0x20,unsigned int)
#define STANDBY_MEMIF_TYPE              _IOWR(AUD_DRV_IOC_MAGIC, 0x21,unsigned int)

#define START_HDMI_MEMIF_TYPE             _IOWR(AUD_DRV_IOC_MAGIC, 0x22, unsigned int)
#define STANDBY_HDMI_MEMIF_TYPE           _IOWR(AUD_DRV_IOC_MAGIC, 0x23, unsigned int)
#define ALLOCATE_MEMIF_HDMI_STEREO_PCM    _IOWR(AUD_DRV_IOC_MAGIC, 0x24, unsigned int)
#define FREE_MEMIF_HDMI_STEREO_PCM        _IOWR(AUD_DRV_IOC_MAGIC, 0x25, unsigned int)
#define ALLOCATE_MEMIF_HDMI_MULTI_CH_PCM  _IOWR(AUD_DRV_IOC_MAGIC, 0x26, unsigned int)
#define FREE_MEMIF_HDMI_MULTI_CH_PCM      _IOWR(AUD_DRV_IOC_MAGIC, 0x27, unsigned int)
#define SET_HDMI_CLOCK_SOURCE          _IOWR(AUD_DRV_IOC_MAGIC, 0x28, Hdmi_Clock_Control *)

#define GET_EAMP_PARAMETER     _IOWR(AUD_DRV_IOC_MAGIC, 0x3e, AMP_Control *)
#define SET_EAMP_PARAMETER     _IOWR(AUD_DRV_IOC_MAGIC, 0x3f, AMP_Control *)

#define SET_2IN1_SPEAKER        _IOW(AUD_DRV_IOC_MAGIC, 0x41, int)
#define SET_AUDIO_STATE         _IOWR(AUD_DRV_IOC_MAGIC, 0x42, SPH_Control*)
#define GET_AUDIO_STATE         _IOWR(AUD_DRV_IOC_MAGIC, 0x43, SPH_Control*)
#define GET_PMIC_VERSION        _IOWR(AUD_DRV_IOC_MAGIC, 0x44, int)

#define AUD_SET_LINE_IN_CLOCK   _IOWR(AUD_DRV_IOC_MAGIC, 0x50, int)
#define AUD_SET_CLOCK           _IOWR(AUD_DRV_IOC_MAGIC, 0x51, int)
#define AUD_SET_26MCLOCK        _IOWR(AUD_DRV_IOC_MAGIC, 0x52, int)
#define AUD_SET_ADC_CLOCK       _IOWR(AUD_DRV_IOC_MAGIC, 0x53, int)
#define AUD_SET_I2S_CLOCK       _IOWR(AUD_DRV_IOC_MAGIC, 0x54, int)
#define AUD_SET_ANA_CLOCK       _IOWR(AUD_DRV_IOC_MAGIC, 0x55, int)
#define AUD_GET_ANA_CLOCK_CNT   _IOWR(AUD_DRV_IOC_MAGIC, 0x56, int)

#define AUDDRV_MOD_PCM_GPIO   _IOWR(AUD_DRV_IOC_MAGIC, 0x5E, int)
#define AUDDRV_SET_BT_FM_GPIO   _IOWR(AUD_DRV_IOC_MAGIC, 0x5f, int)
#define AUDDRV_RESET_BT_FM_GPIO   _IOWR(AUD_DRV_IOC_MAGIC, 0x60, int)
#define AUDDRV_SET_BT_PCM_GPIO    _IOWR(AUD_DRV_IOC_MAGIC, 0x61, int)
#define AUDDRV_SET_FM_I2S_GPIO    _IOWR(AUD_DRV_IOC_MAGIC, 0x62, int)
#define AUDDRV_CHIP_VER           _IOWR(AUD_DRV_IOC_MAGIC, 0x63, int)
#define AUDDRV_SET_RECEIVER_GPIO  _IOWR(AUD_DRV_IOC_MAGIC, 0x64, int)

#define AUDDRV_ENABLE_ATV_I2S_GPIO   _IOWR(AUD_DRV_IOC_MAGIC, 0x65, int)
#define AUDDRV_DISABLE_ATV_I2S_GPIO  _IOWR(AUD_DRV_IOC_MAGIC, 0x66, int)
#define AUDDRV_RESET_FMCHIP_MERGEIF  _IOWR(AUD_DRV_IOC_MAGIC, 0x67, int)

#define AUD_SET_HDMI_CLOCK           _IOWR(AUD_DRV_IOC_MAGIC, 0x68, int)
#define AUD_SET_HDMI_GPIO            _IOWR(AUD_DRV_IOC_MAGIC, 0x69, int)
#define AUD_SET_HDMI_SR              _IOWR(AUD_DRV_IOC_MAGIC, 0x70, int)
#define AUD_SET_HDMI_MUTE            _IOWR(AUD_DRV_IOC_MAGIC, 0x72, int)
#define AUD_SET_APLL_TUNER_CLOCK     _IOWR(AUD_DRV_IOC_MAGIC, 0x73, int)

#define YUSU_INFO_FROM_USER       _IOWR(AUD_DRV_IOC_MAGIC, 0x71, struct _Info_Data*)      //by HP

#define AUDDRV_START_DAI_OUTPUT   _IOWR(AUD_DRV_IOC_MAGIC, 0x81, int)
#define AUDDRV_STOP_DAI_OUTPUT    _IOWR(AUD_DRV_IOC_MAGIC, 0x82, int)
#define AUDDRV_HQA_AMP_MODESEL    _IOWR(AUD_DRV_IOC_MAGIC, 0x90, int)
#define AUDDRV_HQA_AMP_AMPEN      _IOWR(AUD_DRV_IOC_MAGIC, 0x91, int)
#define AUDDRV_HQA_AMP_AMPVOL     _IOWR(AUD_DRV_IOC_MAGIC, 0x92, int)
#define AUDDRV_HQA_AMP_RECEIVER   _IOWR(AUD_DRV_IOC_MAGIC, 0x93, int)
#define AUDDRV_HQA_AMP_RECGAIN    _IOWR(AUD_DRV_IOC_MAGIC, 0x94, int)
#define AUDDRV_AMP_OC_CFG         _IOWR(AUD_DRV_IOC_MAGIC, 0x95, int)
#define AUDDRV_AMP_OC_READ        _IOWR(AUD_DRV_IOC_MAGIC, 0x96, int)
#define AUDDRV_MD_RST_RECOVERY    _IOWR(AUD_DRV_IOC_MAGIC, 0x97, int)

#define AUDDRV_LOWLATENCY_MODE   _IOW(AUD_DRV_IOC_MAGIC, 0x98, int)

#define AUDDRV_KERNEL_DEBUG_MODE   _IOW(AUD_DRV_IOC_MAGIC, 0x99, int)

#define AUDDRV_ECHOREF_MODE   _IOW(AUD_DRV_IOC_MAGIC, 0x9a, int)

// device selection ioctl
#define SET_SPEAKER_VOL          _IOW(AUD_DRV_IOC_MAGIC, 0xa0, int)
#define SET_SPEAKER_ON            _IOW(AUD_DRV_IOC_MAGIC, 0xa1, int)
#define SET_SPEAKER_OFF          _IOW(AUD_DRV_IOC_MAGIC, 0xa2, int)
#define SET_HEADSET_STATE      _IOW(AUD_DRV_IOC_MAGIC, 0xa3, int)
#define SET_HEADPHONE_ON      _IOW(AUD_DRV_IOC_MAGIC, 0xa4, int)
#define SET_HEADPHONE_OFF     _IOW(AUD_DRV_IOC_MAGIC, 0xa5, int)
#define SET_EARPIECE_ON       _IOW(AUD_DRV_IOC_MAGIC, 0xa6, int)
#define SET_EARPIECE_OFF          _IOW(AUD_DRV_IOC_MAGIC, 0xa7, int)

#define SET_EXTCODEC_ON             _IOW(AUD_DRV_IOC_MAGIC, 0xa8, int)
#define SET_EXTCODEC_OFF            _IOW(AUD_DRV_IOC_MAGIC, 0xa9, int)
#define SET_EXTHEADPHONE_AMP_ON     _IOW(AUD_DRV_IOC_MAGIC, 0xaa, int)
#define SET_EXTHEADPHONE_AMP_OFF    _IOW(AUD_DRV_IOC_MAGIC, 0xab, int)
#define SET_EXTCODEC_GAIN           _IOW(AUD_DRV_IOC_MAGIC, 0xac, int)
#define SET_EXTCODEC_MUTE           _IOW(AUD_DRV_IOC_MAGIC, 0xad, int)
#define SET_LOWJITTER_CLK_ON        _IOW(AUD_DRV_IOC_MAGIC, 0xae, int)
#define SET_LOWJITTER_CLK_OFF       _IOW(AUD_DRV_IOC_MAGIC, 0xaf, int)

//auxadc
#define AUDDRV_GET_AUXADC_CHANNEL_VALUE   _IOW(AUD_DRV_IOC_MAGIC, 0xb0, int)

//GPIO
#define SET_GPIO_CURRENT        _IOWR(AUD_DRV_IOC_MAGIC, 0xc0,unsigned int)

#define ALLOCATE_FREE_BTCVSD_BUF _IOWR(AUD_DRV_IOC_MAGIC, 0xE0, unsigned int)
#define SET_BTCVSD_STATE         _IOWR(AUD_DRV_IOC_MAGIC, 0xE1, unsigned int)
#define GET_BTCVSD_STATE         _IOWR(AUD_DRV_IOC_MAGIC, 0xE2, unsigned int)

#define AUDDRV_GET_IPO_EVER              _IOW(AUD_DRV_IOC_MAGIC, 0xF8, int)
#define AUDDRV_CLEAR_IPO_EVER              _IOW(AUD_DRV_IOC_MAGIC, 0xF9, int)
#define AUDDRV_GET_SUSPEND_EVER              _IOW(AUD_DRV_IOC_MAGIC, 0xF8, int)
#define AUDDRV_CLEAR_SUSPEND_EVER              _IOW(AUD_DRV_IOC_MAGIC, 0xF9, int)
#define AUDDRV_AEE_IOCTL              _IOW(AUD_DRV_IOC_MAGIC, 0xFA, int)
#define AUDDRV_GPIO_IOCTL     _IOWR(AUD_DRV_IOC_MAGIC, 0xFB, int)
#define AUDDRV_DUMPFTRACE_DBG     _IOWR(AUD_DRV_IOC_MAGIC, 0xFC, int)
#define AUDDRV_LOG_PRINT          _IOWR(AUD_DRV_IOC_MAGIC, 0xFD, int)
#define AUDDRV_ASSERT_IOCTL       _IOW(AUD_DRV_IOC_MAGIC, 0xFE, int)
#define AUDDRV_BEE_IOCTL          _IOW(AUD_DRV_IOC_MAGIC, 0xFF, int)

// below defines the YUSU_INFO_FROM_USER message
#define INFO_U2K_MATV_AUDIO_START   0x1001
#define INFO_U2K_MATV_AUDIO_STOP     0x1002

#define INFO_U2K_MICANA_SWITCH         0x1003


static char const *const kAudioDeviceName = "/dev/eac";
static char const *const kBTDeviceName = "/dev/ebc";

//<--- for open MATV device
#ifndef MATV_QUERY_I2S_INFO
typedef struct matv_i2s_info
{
    int status;
    int mode;
    int rate;
} matv_i2s_info_t;

#define MATV_QUERY_I2S_INFO     _IOW(MATV_IOC_MAGIC, 0x08,  struct matv_i2s_info*)

#endif
//--->
#endif


