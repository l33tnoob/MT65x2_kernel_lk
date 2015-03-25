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
 *   meta_fm.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   FM meta data struct define.
 *
 * Author:
 * -------
 *  LiChunhui (MTK80143)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 10 31 2012 run.liu
 * [ALPS00381117] [FM][Meta mode][6589]enable FM meta mode on 6589+6628 project
 * .
 *
 * 12 03 2010 hongcheng.xia
 * [ALPS00136616] [Need Patch] [Volunteer Patch]FM Meta bugs Fix
 * .
 *
 * 11 18 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 08 28 2010 chunhui.li
 * [ALPS00123709] [Bluetooth] meta mode check in
 * for FM meta enable

 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef __META_FM_H_
#define __META_FM_H_

#include <linux/fm.h>

#define FT_CNF_OK     0
#define FT_CNF_FAIL   1

#ifdef __cplusplus
extern "C" {
#endif

/* The TestCase Enum define of FM_module */
typedef enum {
	 FM_OP_READ_CHIP_ID = 0       //V 0
	,FM_OP_POWER_ON               //V 1
	,FM_OP_POWER_OFF              //V 2
	,FM_OP_SET_FREQ               //V 3
	,FM_OP_SET_MONO_STEREO_BLEND  //V 4
	,FM_OP_GET_SIGNAL_VAL         //V 5
	,FM_OP_GET_IF_CNT 			// 6
	,FM_OP_SEARCH_NEXT_STAT       //V 7
	,FM_OP_SEARCH_PREV_STAT       //V 8
	,FM_OP_READ_ANY_BYTE          //V 9
	,FM_OP_WRITE_ANY_BYTE         //V 10
	,FM_OP_SOFT_MUTE_ONOFF        //V 11
	,FM_OP_SELECT_SOFT_MUTE_STAGE //V 12
	,FM_OP_SELECT_STEREO_BLEND   // 13
	,FM_OP_SET_RSSI_THRESHOLD     //V 14
	,FM_OP_SET_IF_CNT_DELTA 	// 15
	,FM_OP_GET_H_L_SIDE           //V 16
	,FM_OP_GET_STEREO_MONO		// 17
	,FM_OP_SET_VOLUME             //V 18
	,FM_OP_FM_AUTOSCAN            //V 19
	,FM_OP_SET_RDS				// 20
	,FM_OP_GET_RXFILTER_BW 		// 21 
	,FM_OP_GET_PAMD_LEVEL		// 22
	,FM_OP_GET_MR				// 23
	,FM_OP_SET_DECODE_MODE		// 24
	,FM_OP_SET_HCC                //V 25
	,FM_OP_SET_PAMD_THRESHOLD     //V 26
	,FM_OP_SET_SOFTMUTE           //V 27
	,FM_OP_SET_DEEMPHASIS_LEVEL		// 28
	,FM_OP_SET_H_L_SIDE				// 29
	,FM_OP_SET_DEMOD_BW				// 30
	,FM_OP_SET_DYNAMIC_LIMITER    //V 31
	,FM_OP_SET_SOFTMUTE_RATE      //V 32
	,FM_OP_GET_PI					// 33
	,FM_OP_GET_PTY					// 34
	,FM_OP_GET_TP					// 35
	,FM_OP_GET_PS					// 36
	,FM_OP_GET_AF					// 37
	,FM_OP_GET_TA					// 38
	,FM_OP_GET_MS					// 39
	,FM_OP_GET_RT					// 40
	,FM_OP_GET_GOOD_BLOCK_COUNTER	// 41
	,FM_OP_GET_BAD_BLOCK_COUNTER	// 42
	,FM_OP_RESET_BLOCK_COUNTER		// 43
	,FM_OP_GET_GROUP_COUNTER		// 44
	,FM_OP_RESET_GROUP_COUNTER		// 45
	//,FM_OP_HWSEEK					// 46
	,FM_OP_SOFT_MUTE_SEEK			//46
	,FM_OP_HWSEARCH_STOP			// 47
	,FM_OP_SET_STEREO_BLEND			// 48
    ,FM_OP_GET_RDS_LOG				// 49
    ,FM_OP_GET_RDS_BLER				// 50
	,FM_OP_POWER_ON_TX            //V 51
	,FM_OP_SET_FREQ_TX            //V 52
	,FM_OP_SET_RDS_TX             //V 53
	,FM_OP_SET_AUDIO_PATH_TX    //54
	,FM_OP_SET_AUDIO_FREQ_TX    //55
    ,FM_OP_SET_ANTENNA             //56
    ,FM_OP_GET_CAPARRY             //57
    ,FM_OP_GET_STEP_MODE           //58
    ,FM_OP_AUDIO_BIST_TEST        //59
	,FM_OP_END						// 
} FM_OP;

typedef enum
{
    FM_CHIP_ID_MT6189AN = 0,
    FM_CHIP_ID_MT6189BN_CN = 1,
    FM_CHIP_ID_MT6188A = 3,
    FM_CHIP_ID_MT6188C = 4,
    FM_CHIP_ID_MT6188D = 5,
    FM_CHIP_ID_MT6616 = 6,
    FM_CHIP_ID_AR1000 = 7,
    FM_CHIP_ID_MT6620 = 8,
    FM_CHIP_ID_MT6626 = 9,
	FM_CHIP_ID_MT6628 = 10,
	FM_CHIP_ID_MT6627 = 11
}FM_CHIP_ID_E;

typedef enum fmtx_tone_freq {
    FMTX_1K_TONE = 1, 
    FMTX_2K_TONE = 2,
    FMTX_3K_TONE = 3,
    FMTX_4K_TONE = 4,
    FMTX_5K_TONE = 5,
    FMTX_6K_TONE = 6,
    FMTX_7K_TONE = 7,
    FMTX_8K_TONE = 8,
    FMTX_9K_TONE = 9,
    FMTX_10K_TONE = 10,
    FMTX_11K_TONE = 11,
    FMTX_12K_TONE = 12,
    FMTX_13K_TONE = 13,
    FMTX_14K_TONE = 14,
    FMTX_15K_TONE = 15,
    FMTX_MAX_TONE
}FM_TX_TONE_T;

typedef enum{
    FM_TX_AUDIO_ANALOG = 0,
    FM_TX_AUDIO_I2S = 1,
    FM_RX_AUDIO_ANALOG = 2,
    FM_RX_AUDIO_I2S = 3,
    FM_AUDIO_MAX
}FM_TX_AUDIO_PATH_T;

typedef enum{
    FM_OFF = 0,
    FM_ON_RX,
    FM_ON_TX,
    FM_MAX
}FM_STATE_T;

typedef enum{
    FM_RDS_OFF = 0,
    FM_RDS_RX_ON,
    FM_RDS_TX_ON,
    FM_RDS_MAX
}FM_RDS_T;

    enum 
    {
        FM_ANA_LONG = 0,
        FM_ANA_SHORT = 1,
        FM_ANA_MAX
    };

typedef struct{
    FM_STATE_T state;
    FM_RDS_T rds_t;
    FM_TX_TONE_T tx_tone;
    FM_TX_AUDIO_PATH_T audio_path;
}fm_status;

typedef struct {
    unsigned char pty;         // 0~31 integer
    unsigned char rds_rbds;    // 0:RDS, 1:RBDS
    unsigned char dyn_pty;     // 0:static, 1:dynamic
    unsigned short pi_code;    // 2-byte hex
    unsigned char ps_buf[8];     // hex buf of PS
    unsigned char ps_len;      // length of PS, must be 0 / 8"
    unsigned char af;          // 0~204, 0:not used, 1~204:(87.5+0.1*af)MHz
    unsigned char ah;          // Artificial head, 0:no, 1:yes
    unsigned char stereo;      // 0:mono, 1:stereo
    unsigned char compress;    // Audio compress, 0:no, 1:yes
    unsigned char tp;          // traffic program, 0:no, 1:yes
    unsigned char ta;          // traffic announcement, 0:no, 1:yes
    unsigned char speech;      // 0:music, 1:speech
}FM_RDS_TX_REQ_T;

typedef struct {
	unsigned short m_u2StopFreq;
	unsigned char m_ucSignalLvl; 
} ValidStopResStr;

typedef struct {
	unsigned int m_u4ItemValue;
} RSSIThresholdStr;

typedef struct {
	unsigned short  con_hdl;
	unsigned short  len;      	
	unsigned char   buffer[1024]; 
} FM_BUFFER;

typedef struct {
	unsigned char m_ucAddr; 
} FM_READ_BYTE_ADDR_REQ_T;

typedef struct{
 	short m_i2CurFreq;	
}FM_FREQ_REQ_T;

typedef struct{
	unsigned short m_u2MonoOrStereo;
	unsigned short m_u2SblendOnOrOff;
	unsigned int m_u4ItemValue;	
}FM_MONO_STEREO_BLEND_REQ_T;

typedef struct{
	short m_i2StartFreq;
   	short m_i2StopFreq;
}FM_FREQ_RANGE_REQ_T;

typedef struct{
	unsigned int m_u4RssiThreshold;
}FM_RSSI_THRESHOLD_REQ_T;

typedef struct{
	unsigned int m_u4IfCntDelta;	
}FM_IF_CNT_DELTA_REQ_T;

typedef struct {
	unsigned char m_ucAddr;
	unsigned short m_u2WriteByte; 
} FM_WRITE_BYTE_REQ_T;

typedef struct{
	unsigned char  m_bOnOff;
}FM_SOFT_MUTE_ONOFF_REQ_T;

typedef struct{
	unsigned char m_ucStage;
}FM_STAGE_REQ_T;

typedef struct{
    unsigned char m_ucVolume;
    char m_cDigitalGainIndex;
}FM_Volume_Setting_REQ_T;

#ifdef FM_50KHZ_SUPPORT
    typedef struct {
        unsigned short m_u2Bitmap[26];
    } FM_AutoScan_CNF_T;
#else
typedef struct{
    unsigned short m_u2Bitmap[16];
}FM_AutoScan_CNF_T;
#endif

typedef struct{
    unsigned char m_ucRDSOn;
}FM_SetRDS_REQ_T;

typedef struct{
    unsigned int m_u4DecodeMode;
}FM_Decode_Mode_REQ_T;

typedef struct{
    unsigned int m_u4HCC;
}FM_HCC_REQ_T;

typedef struct{
    unsigned int m_u4PAMDThreshold;
}FM_PAMD_Threshold_REQ_T;

typedef struct{
    unsigned int m_u4SoftmuteEnable;
}FM_Softmute_Enable_REQ_T;

typedef struct {
    unsigned int m_u4DeemphasisLevel;
}FM_Deemphasis_Level_REQ_T;

typedef struct{
	unsigned int m_u4HLSide;
}FM_HL_Side_REQ_T;

typedef struct{
    unsigned int m_u4DemodBandwidth;
}FM_Demod_Bandwidth_REQ_T;

typedef struct{
    unsigned int m_u4DynamicLimiter;
}FM_DynamicLimiter_REQ_T;

typedef struct{
    unsigned int m_u4SoftmuteRate;
}FM_Softmute_Rate_REQ_T;

typedef enum{
    RDS_CMD_NONE = 0,
    RDS_CMD_PI_CODE,
    RDS_CMD_PTY_CODE,
    RDS_CMD_PROGRAMNAME,
    RDS_CMD_LOCDATETIME,
    RDS_CMD_UTCDATETIME,
    RDS_CMD_LAST_RADIOTEXT,
    RDS_CMD_AF,
    RDS_CMD_AF_LIST,  
    RDS_CMD_AFON,
    RDS_CMD_TAON,
    RDS_CMD_TAON_OFF = 0x0fffffff
}RdsCmd;

#if 0
typedef enum{
    RDS_FLAG_IS_TP				= 0x000001,
    RDS_FLAG_IS_TA				= 0x000002,
    RDS_FLAG_IS_MUSIC			= 0x000004,
    RDS_FLAG_IS_STEREO			= 0x000008,
    RDS_FLAG_IS_ARTIFICIAL_HEAD	= 0x000010,
    RDS_FLAG_IS_COMPRESSED		= 0x000020,
    RDS_FLAG_IS_DYNAMIC_PTY		= 0x000040,
    RDS_FLAG_TEXT_AB			= 0x000080
}RdsFlag;

typedef enum{
   RDS_EVENT_FLAGS          = 0x0001,
   RDS_EVENT_PI_CODE        = 0x0002,
   RDS_EVENT_PTY_CODE       = 0x0004,
   RDS_EVENT_PROGRAMNAME    = 0x0008,
   RDS_EVENT_UTCDATETIME    = 0x0010,
   RDS_EVENT_LOCDATETIME    = 0x0020,
   RDS_EVENT_LAST_RADIOTEXT = 0x0040,
   RDS_EVENT_AF             = 0x0080,
   RDS_EVENT_AF_LIST        = 0x0100,
   RDS_EVENT_AFON_LIST      = 0x0200,
   RDS_EVENT_TAON           = 0x0400,
   RDS_EVENT_TAON_OFF       = 0x0800
} RdsEvent;
#endif

typedef struct{
    RdsCmd m_eCmd;
}FM_RDS_Info_REQ_T;

typedef struct{
    unsigned char m_buffer[64];
}FM_RDS_Info_CNF_T;

typedef struct {
    RdsFlag m_eFlag;
    unsigned char m_buffer[64];
}FM_RDS_Status_CNF_T;

typedef struct{
    unsigned short m_u2GroupCounter[32];
}FM_RDS_Group_Counter_CNF_T;

typedef struct{
    short m_i2StartFreq;
    unsigned char m_ucDirection;
}FM_HWSeek_REQ_T;

typedef struct{
    unsigned short m_u2StereoBlendControl;
}FM_SetStereoBlend_REQ_T;

typedef struct{
    int m_audioPath;
}FM_SetTxAudioPath_REQ_T;

typedef struct{
    int m_audioFreq;
}FM_SetTxAudioFreq_REQ_T;

    typedef struct{
        int ana;
    }FM_SetAntenna_REQ_T;

typedef union {
	FM_READ_BYTE_ADDR_REQ_T m_rReadAddr;
	FM_FREQ_REQ_T m_rCurFreq;
    FM_RDS_TX_REQ_T m_rRdsTx;
	FM_MONO_STEREO_BLEND_REQ_T m_rMonoStereoSettings;
	FM_FREQ_RANGE_REQ_T m_rFreqRange;
    FM_RSSI_THRESHOLD_REQ_T m_rRssiThreshold;
    FM_IF_CNT_DELTA_REQ_T m_rIfCntDelta;
	FM_WRITE_BYTE_REQ_T m_rWriteByte;
	FM_SOFT_MUTE_ONOFF_REQ_T m_rSoftMuteOnOff;
	FM_STAGE_REQ_T m_rStage;
	FM_Volume_Setting_REQ_T       m_rVolumeSetting;
	FM_SetRDS_REQ_T               m_rSetRDS;
	FM_Decode_Mode_REQ_T          m_rDecodeMode;
	FM_HCC_REQ_T                  m_rHCC;
	FM_PAMD_Threshold_REQ_T       m_rPAMDThreshold;
	FM_Softmute_Enable_REQ_T      m_rSoftmuteEnable;
	FM_Deemphasis_Level_REQ_T     m_rDeemphasisLevel;
	FM_HL_Side_REQ_T              m_rHLSide;
	FM_Demod_Bandwidth_REQ_T      m_rDemodBandwidth;
	FM_DynamicLimiter_REQ_T       m_rDynamicLimiter;
	FM_Softmute_Rate_REQ_T        m_rSoftmuteRate;
    FM_RDS_Info_REQ_T             m_rRDSInfo;
	FM_HWSeek_REQ_T               m_rHWSeek;
    FM_SetStereoBlend_REQ_T       m_rStereoBlendControl;
    FM_SetTxAudioFreq_REQ_T       m_rAudioFreqCtrl;
    FM_SetTxAudioPath_REQ_T       m_rAudioPathCtrl;
        FM_SetAntenna_REQ_T           m_rAntenna;
} META_FM_CMD_U;

typedef struct {
    FT_H	       header;  //module do not need care it
	FM_OP		   op;
	META_FM_CMD_U  cmd;
} FM_REQ;

typedef struct{
	unsigned char  m_ucChipId;
}FM_CHIP_ID_CNF_T;

typedef struct{
	int  m_ucSignalLevel;
}FM_RSSI_CNF_T;

typedef struct{
	unsigned short m_u2IfCnt;	
}FM_IF_CNT_CNF_T;

typedef struct{
	unsigned char  m_ucExit;      // 0: don't exist, 1: exist
	short int m_i2ValidFreq;  // -1: settings error, 0: invalid freq, others: 875-1080 valid
}FM_VAILD_FREQ_CNF_T;

typedef struct{
	unsigned short m_u2ReadByte;
}FM_READ_BYTE_CNF_T;

typedef struct{
	unsigned char m_ucHighOrLow;
}FM_HL_Side_CNF_T;

typedef struct{
	unsigned char  m_ucStereoOrMono;
}FM_Stereo_Mono_CNF_T;

typedef struct{
    unsigned char m_ucRXFilterBW;
} FM_RX_FilterBW_CNF_T;

typedef struct{
    unsigned char m_ucPAMDLevel;
}FM_PAMD_Level_CNF_T;

typedef struct{
    unsigned char m_ucMR;
}FM_MR_CNF_T;

typedef struct{
        int m_uCapArray;
    }FM_CapArray_CNF_T;

    typedef struct{
    unsigned short m_u2GoodBlock;
}FM_RDS_Good_Block_Counter_CNF_T;

typedef struct{
    unsigned short m_u2BadBlock;
}FM_RDS_Bad_Block_Counter_CNF_T;

typedef union
{
    FM_RDS_Status_CNF_T m_rRDSStatus;
    FM_RDS_Info_CNF_T   m_rRDSInfo;
}FM_RDS_U;

typedef struct
{
    RdsEvent            eventtype;
    FM_RDS_U            m_rRDS;
}FM_RDS_CNF_T;


typedef struct{
    short m_i2EndFreq;
}FM_HWSeek_CNF_T;

    typedef struct {
        int  m_i4Step;
    } FM_STEP_MODE_CNF_T;


typedef union {
	FM_CHIP_ID_CNF_T      m_rChipId;
	FM_RSSI_CNF_T         m_rSignalValue;
	FM_IF_CNT_CNF_T       m_rIfCnt;
	FM_VAILD_FREQ_CNF_T   m_rValidFreq;
	FM_READ_BYTE_CNF_T    m_rReadByte;
	FM_HL_Side_CNF_T      m_rHLSide;
	FM_Stereo_Mono_CNF_T  m_rStereoMono;
	FM_RX_FilterBW_CNF_T  m_rRXFilterBW;
	FM_PAMD_Level_CNF_T   m_rPAMDLevel;
	FM_MR_CNF_T           m_rMR;
	FM_RDS_Good_Block_Counter_CNF_T m_rRDSGoodBlockCounter;
	FM_RDS_Bad_Block_Counter_CNF_T  m_rRDSBadBlockCounter;
	FM_HWSeek_CNF_T                 m_rHWSeek;
    FM_CapArray_CNF_T        m_rCapArray;
    FM_STEP_MODE_CNF_T              m_rStep;
} META_FM_CNF_U;

typedef struct {
    FT_H		    header;  //module do not need care it
	FM_OP			op;
	META_FM_CNF_U   fm_result;   //fm->FT
	int drv_status;
	unsigned int	status;
} FM_CNF;

bool META_FM_init();
void META_FM_deinit();
void META_FM_OP(FM_REQ *req, char *peer_buff, unsigned short peer_len) ;

#ifdef __cplusplus
};
#endif

#endif
