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
/*******************************************************************************
 *
 * Filename:
 * ---------
 * meta_audio_para.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Audio meta test
 *
 * Author:
 * -------
 * mtk80306
 *
 *------------------------------------------------------------------------------
 *******************************************************************************/

#ifndef __META_AUDIO_PARA_H_
#define __META_AUDIO_PARA_H_

#include "FT_Public.h"


#ifdef __cplusplus
extern "C" {

#endif

#define FT_L4AUD_MAX_MELODY_FILE_NAME   MAX_MELODY_FILE_NAME
#define FT_L4AUD_MAX_MEDIA_FILE_NAME    MAX_MEDIA_FILE_NAME
#define FT_L4AUD_ERR_FILEPATH_TOO_LONG  0xFF
#define FT_L4AUD_ERR_FILEPATH_ERROR     0xFE
#define FT_L4AUD_ERR_PEER_BUF_ERROR     0xFD
#define FT_L4AUD_ERR_STILL_PLAYING      0xFC
#define FT_L4AUD_ERR_OP_NOT_SUPPORT     0xFB

/* operation */
typedef enum {
    FT_L4AUD_OP_AUDIO_QUERY_ID = 0
                                 , FT_L4AUD_OP_AUDIO_PLAY
                                 , FT_L4AUD_OP_AUDIO_PLAY_BY_NAME
                                 , FT_L4AUD_OP_AUDIO_DEMO_IMY
                                 , FT_L4AUD_OP_AUDIO_STOP
                                 , FT_L4AUD_OP_AUDIO_PLAY_OVER_IND
                                 , FT_L4AUD_OP_MEDIA_PLAY
                                 , FT_L4AUD_OP_MEDIA_STOP
                                 , FT_L4AUD_OP_MEDIA_PLAY_OVER_IND
                                 , FT_L4AUD_OP_SET_VOLUME
                                 , FT_L4AUD_OP_SET_ECHO
                                 , FT_L4AUD_OP_SET_MODE
                                 , FT_L4AUD_OP_SET_GAIN
                                 , FT_L4AUD_OP_TONE_LOOP_BACK_REC
                                 , FT_L4AUD_OP_SET_LOUDSPK_FIR_COEFFS
                                 , FT_L4AUD_OP_SET_SPEECH_COMMON
                                 , FT_L4AUD_OP_SET_LOUDSPK_MODE
                                 , FT_L4AUD_OP_SET_PLAYBACK_MAX_SWING
                                 , FT_L4AUD_OP_SET_MELODY_FIR
                                 , FT_L4AUD_OP_SET_SPEECH_COMMON_AND_MODE
                                 , FT_L4AUD_OP_PLAY_FREQ_VOL_TONE
                                 , FT_L4AUD_OP_STOP_FREQ_VOL_TONE
                                 , FT_L4AUD_OP_TONE_LOOP_BACK_REC_2K
                                 , FT_L4AUD_OP_TONE_LOOP_BACK_REC_2K_NORMAL
                                 , FT_L4AUD_OP_GET_PROFILE_SETTINGS_BY_MODE
                                 , FT_L4AUD_OP_SET_PROFILE_SETTINGS_BY_MODE
                                 , FT_L4AUD_OP_GET_PARAM_SETTINGS_0809
                                 , FT_L4AUD_OP_SET_PARAM_SETTINGS_0809
                                 , FT_L4AUD_OP_RECEIVER_TEST
                                 , FT_L4AUD_OP_LOUDSPK_TEST
                                 , FT_L4AUD_OP_EARPHONE_TEST
                                 , FT_L4AUD_OP_HEADSET_LOOPBACK_TEST
                                 , FT_L4AUD_OP_FM_LOOPBACK_TEST
                                 , FT_L4AUD_OP_SET_ACF_COEFFS
                                 , FT_L4AUD_OP_GET_ACF_COEFFS
                                 , FT_L4AUD_OP_SET_PREVIEW_ACF_COEFFS
                                 , FT_L4AUD_OP_MIC2_LOOPBACK
                                 , FT_L4AUD_OP_SET_PLAYBACK_FILE
                                 , FT_L4AUD_OP_DL_DATA_PACKAGE
                                 , FT_L4AUD_OP_DUALMIC_RECORD
                                 , FT_L4AUD_OP_PLAYBACK_DUALMICRECORD
                                 , FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS
                                 , FT_L4AUD_OP_STOP_DUALMIC_RECORD
                                 , FT_L4AUD_OP_UL_DATA_PACKAGE
                                 , FT_L4AUD_OP_DUALMIC_SET_PARAMS
                                 , FT_L4AUD_OP_DUALMIC_GET_PARAMS
                                 , FT_L4AUD_OP_LOAD_VOLUME
                                 , FT_L4AUD_OP_SET_HCF_COEFFS
                                 , FT_L4AUD_OP_GET_HCF_COEFFS
                                 , FT_L4AUD_OP_SET_PREVIEW_HCF_COEFFS
                                 , FT_L4AUD_OP_GET_GAINTABLE_SUPPORT
                                 , FT_L4AUD_OP_GET_GAINTABLE_NUM
                                 , FT_L4AUD_OP_GET_GAINTABLE_LEVEL
                                 , FT_L4AUD_OP_GET_CTRPOINT_NUM
                                 , FT_L4AUD_OP_GET_CTRPOINT_BITS
                                 , FT_L4AUD_OP_GET_CTRPOINT_TABLE
                                 , FT_L4AUD_OP_END
}
FT_L4AUD_OP;

typedef enum
{
    FT_L4AUD_AUDIO_PLAY_CRESCENDO = 0,
    FT_L4AUD_AUDIO_PLAY_INFINITE = 1,
    FT_L4AUD_AUDIO_PLAY_ONCE = 2,
    FT_L4AUD_AUDIO_PLAY_DESCENDO = 3
} FT_L4AUD_AUDIO_PLAY_STYLE;

typedef enum
{
    RECORD_ERROR = 0,
    RECORD_START,
    RECORD_END,
    NUM_RECORD_STATES
} record_states;


/* request */
typedef struct
{
    unsigned short                  audio_id;       // default system embeded audio id
    FT_L4AUD_AUDIO_PLAY_STYLE   play_style;     // play style
} ft_l4aud_play_req;

typedef struct
{
    char                    volume;         // play volume, 0 ~ 255
} ft_l4aud_set_volume_req;

typedef struct
{
    char                    echoflag;           // echoflag true mean EchoOK
} ft_l4aud_set_echo;

typedef struct
{
    char                    receiver_test;      // receiver_test true mean enable
} ft_l4aud_receiver_test;
typedef struct
{
    char  bEnable;          // true means to enable
} ft_l4aud_earphone_test;

typedef struct
{
    int  bEnable;    // retrun if support gain table
} ft_l4aud_gaintable_cnt;

typedef struct
{
    int  gaintablenum;  // return number ogf gain type
} ft_l4aud_gaintablenum_cnf;

typedef struct
{
    int gaintabletype;    // query of which gain type
} ft_l4aud_gaintablelevel_req;

typedef struct
{
    int gaintabletype;     // return of gain type4
    int gainttablelevel;    // return level of query gain type
} ft_l4aud_gaintablelevel_cnf;

typedef struct
{
    int  i4ctrpointnum;        // this get control point number
} ft_l4aud_ctrpointnum_cnt;

typedef struct
{
    int  i4ctrpoint;        //  this range should depend on amp control pointer number
} ft_l4aud_ctrpointbits_req;

typedef struct
{
    int  i4ctrpoint;        //  this range should depend on amp control pointer number
    int  i4ctrpointbits;   // return bits of this control bpoint
} ft_l4aud_ctrpointbits_cnt;

typedef struct
{
    int  i4ctrpoint;        //  query control point of table
} ft_l4aud_ctrpointtable_req;

typedef struct
{
    int  i4ctrpoint;        //  query control point of table
    char buffer[255];          //   buffer to fill contorl point table
    int i4bufferlength;  //   length of buffer  filled.
} ft_l4aud_ctrpointtable_cnt;

typedef struct
{
    char  bEnable;          // true means to enable
} ft_l4aud_headset_loopback_test;

typedef struct
{
    char  bEnable;          // true means to enable
} ft_l4aud_FM_loopback_test;

typedef struct
{
    char                    left_channel;       // true mean enable
    char                    right_channel;      // true mean enable
} ft_l4aud_loudspk;

typedef struct
{
    char                    modeflag;           // modeflag: normal, headset, loudspeaker
} ft_l4aud_set_mode;

typedef struct
{
    char                    type;
    char                    gain;
} ft_l4aud_set_gain;

typedef struct
{
    unsigned short      fre;
    char        spkgain;
    char        micgain;
    unsigned short      ulgain;
    unsigned short      dlgain;
    unsigned short      amp;
} ft_tone_loopbackrec_req;

typedef struct
{
    char        pivot;
    unsigned int        buffer[500];
} ft_tone_loopbackrec_cnf;

typedef struct
{
    unsigned int        buffer[500];
} ft_tone_loopbackrec_cnf_2k;

typedef struct
{
    short       in_fir_coeffs[45];
    short       out_fir_coeffs[45];
} ft_l4aud_set_loudspk_fir_coeffs_req;

typedef struct
{
    unsigned short speech_common_para[12];
} ft_l4aud_set_speech_common_req;

typedef struct
{
    unsigned short speech_loudspk_mode_para[16];  // change from 8 to 16, because at most: 16
} ft_l4aud_set_loudSpk_mode_req;

typedef struct
{
    unsigned short Media_Playback_Maximum_Swing;
} ft_l4aud_set_playback_maximum_swing_req;

typedef struct
{
    short Melody_FIR_Output_Coeff_32k_Tbl1[25];
} ft_l4aud_set_melody_fir_output_coeffs_req;

typedef struct
{
    unsigned short speech_common_para[12];
    unsigned short speech_loudspk_mode_para[16]; // change from 8 to 16, because at most: 16
} ft_l4aud_set_speech_common_and_mode_req;

/*Parameters of Audio Compensation Filter*/
typedef struct
{
    /* Compensation Filter HSF coeffs       */
    /* BesLoudness also uses this coeffs    */
    unsigned int bes_loudness_hsf_coeff[9][4];

    /* Compensation Filter BPF coeffs       */
    unsigned int bes_loudness_bpf_coeff[4][6][3];
    unsigned int bes_loudness_DRC_Forget_Table[9][2];
    unsigned int bes_loudness_WS_Gain_Max;
    unsigned int bes_loudness_WS_Gain_Min;
    unsigned int bes_loudness_Filter_First;
    char bes_loudness_Gain_Map_In[5];
    char bes_loudness_Gain_Map_Out[5];
} ft_l4aud_set_acf_param_req;

typedef struct
{
    char  m_ucVolume;
    unsigned short m_u2Freq;
} ft_l4aud_set_freq_vol_tone;

typedef struct
{
    char   m_ucMode;
} ft_l4aud_get_profile_by_mode;

typedef struct
{
    char mode;
    char melody[7];
    char sound[7];
    char keytone[7];
    char speech[7];
    char mic[7];
    char sidetone;
    char max_melody_volume_gain;
    char melody_volume_gain_step;
    char tv_out_volume_gain[7];

} ft_l4aud_set_profile_by_mode; // should be the same as audio_profile_struct

typedef struct
{
    /* volume setting */
    char volume[3][7];
    /* speech enhancement */
    unsigned short speech_common_para[12];
    unsigned short speech_mode_para[8][16];
    unsigned short speech_volume_para[4];//in the feature, should extend to [MAX_VOL_CATE][MAX_VOL_TYPE][4]
    /* debug info */
    unsigned short debug_info[16];
    /* speech input FIR */
    short          sph_in_fir[6][45];
    /* speech output FIR */
    short          sph_out_fir[6][45];
    /* digital gain of DL speech */
    unsigned short Digi_DL_Speech;
    /* digital gain of uplink speech */
    unsigned short Digi_Microphone;
    /* FM record volume*/
    unsigned short FM_Record_Volume;
    /* user mode : normal mode, earphone mode, loud speaker mode */
    unsigned short Bluetooth_Sync_Type;
    unsigned short Bluetooth_Sync_Length;
    unsigned short bt_pcm_in_vol;
    unsigned short bt_pcm_out_vol;
    unsigned short user_mode;
    /* auto VM record setting */
    unsigned short bSupportVM;
    unsigned short bAutoVM;
    unsigned short uMicbiasVolt;
} ft_l4aud_set_param_0809;   // must the same as audio_param_struct in \src\common\cfgfileinc\CFG_AUDIO_File.h

/*@set audio playback file */
typedef struct
{
    char filename[256];
} ft_l4aud_dl_playback_file_req;

/*@download file data package */
typedef struct
{
    char filename[256];
    unsigned char data[2048];
    int size;
} ft_l4aud_dl_data_package_req;

/*@set audio dual mic record */
typedef struct
{
    char filename[256];
    int duration;     /* record duration */
} ft_l4aud_dualmic_record_req;

/*@playback background sound and dual mic record */
typedef struct
{
    char playbackFilename[256];
    char recordingFilename[256];
    int recordDuration;
} ft_l4aud_playback_dualmicrecord_req;

/*@headset playback background sound and dual mic record */
typedef struct
{
    char playbackFilename[256];
    char recordingFilename[256];
    int recordDuration;
} ft_l4aud_playback_dualmicrecord_hs_req;

/*@get record data by package */
typedef struct
{
    char filename[256];
    int size;
    int flag; // add by donglei for bit0 the first hit; bit1 end hit;
} ft_l4aud_ul_data_package_req;

/*set parameters: FT_L4AUD_OP_DUALMIC_SET_PARAMS*/
typedef struct
{
    char param[128];
    int value;
} ft_l4aud_dualmic_set_params_req;

/*get parameters: FT_L4AUD_OP_DUALMIC_GET_PARAMS*/
typedef struct
{
    char param[128];
} ft_l4aud_dualmic_get_params_req;

typedef union
{
    ft_l4aud_play_req           play;
    ft_l4aud_set_volume_req     set_volume;
    ft_l4aud_set_echo           set_echo;
    ft_l4aud_set_mode           set_mode;
    ft_l4aud_set_gain           set_gain;
    ft_l4aud_receiver_test  receiver_test;
    ft_l4aud_loudspk        loudspk_test;
    ft_tone_loopbackrec_req     tone_loopbackrec;
    ft_l4aud_set_loudspk_fir_coeffs_req  set_loudspk_fir_coeffs;
    ft_l4aud_set_speech_common_req   set_speech_common;
    ft_l4aud_set_loudSpk_mode_req   set_loudSpk_mode;
    ft_l4aud_set_playback_maximum_swing_req    set_playback_maximum_swing;
    ft_l4aud_set_melody_fir_output_coeffs_req  set_melody_fir_output_coeffs;
    ft_l4aud_set_speech_common_and_mode_req    set_speech_common_and_mode;
    ft_l4aud_set_freq_vol_tone                 set_freq_vol_tone;
    ft_l4aud_get_profile_by_mode               get_profile_by_mode;
    ft_l4aud_set_profile_by_mode               set_profile_by_mode;
    ft_l4aud_set_param_0809                    set_param_0809;
    ft_l4aud_earphone_test                     eaphone_test;
    ft_l4aud_headset_loopback_test             headset_loopback_test;
    ft_l4aud_FM_loopback_test                  fm_loopback_test;
    ft_l4aud_set_acf_param_req                 set_acf_param;
    ft_l4aud_dl_playback_file_req              dl_playback_file;
    ft_l4aud_dl_data_package_req               dl_data_package;
    ft_l4aud_dualmic_record_req                dualmic_record;
    ft_l4aud_playback_dualmicrecord_req        playback_dualmic_record;
    ft_l4aud_playback_dualmicrecord_hs_req     playback_dualmic_record_hs;
    ft_l4aud_ul_data_package_req               ul_data_package;
    ft_l4aud_dualmic_set_params_req            dualmic_set_params;
    ft_l4aud_dualmic_get_params_req            dualmic_get_params;
    ft_l4aud_set_acf_param_req                 set_hcf_param;
    ft_l4aud_gaintablelevel_req                   get_gaintable_level_par;
    ft_l4aud_ctrpointbits_req                      get_ctrpoint_bit_par;
    ft_l4aud_ctrpointtable_req                    get_ctrpoint_table_par;

    unsigned int                               dummy;           // extend alignment to 4 bytes
    unsigned int                               dummy2;          // extend alignment to 4 bytes
} ft_l4aud_request;

typedef struct
{
    FT_H                        header;
    FT_L4AUD_OP                 op;
    ft_l4aud_request            req;
} FT_L4AUD_REQ;

/* confirm */
typedef struct
{
    unsigned short                  min_ringtone_id;
    unsigned short                  max_ringtone_id;
    unsigned short                  min_midi_id;
    unsigned short                  max_midi_id;
    unsigned short                  min_sound_id;
    unsigned short                  max_sound_id;
} ft_l4aud_query_id_cnf;

typedef struct
{
    char mode;
    char melody[7];
    char sound[7];
    char keytone[7];
    char speech[7];
    char mic[7];
    char sidetone;
    char max_melody_volume_gain;
    char melody_volume_gain_step;
    char tv_out_volume_gain[7];

} ft_l4aud_get_profile_by_mode_cnf; // should be the same as audio_profile_struct in interface\media\med_struct.h

typedef struct
{
    unsigned short m_u2FailReason;
} ft_l4aud_set_profile_by_mode_cnf;

typedef struct
{
    /* volume setting */
    char volume[3][7];
    /* speech enhancement */
    unsigned short speech_common_para[12];
    unsigned short speech_mode_para[8][16];
    unsigned short speech_volume_para[4];//in the feature, should extend to [MAX_VOL_CATE][MAX_VOL_TYPE][4]
    /* debug info */
    unsigned short debug_info[16];
    /* speech input FIR */
    short          sph_in_fir[6][45];
    /* speech output FIR */
    short          sph_out_fir[6][45];
    /* digital gain of DL speech */
    unsigned short Digi_DL_Speech;
    /* digital gain of uplink speech */
    unsigned short Digi_Microphone;
    /* FM record volume*/
    unsigned short FM_Record_Volume;
    /* user mode : normal mode, earphone mode, loud speaker mode */
    unsigned short Bluetooth_Sync_Type;
    unsigned short Bluetooth_Sync_Length;
    unsigned short bt_pcm_in_vol;
    unsigned short bt_pcm_out_vol;
    unsigned short user_mode;
    /* auto VM record setting */
    unsigned short bSupportVM;
    unsigned short bAutoVM;
    // set to 1900, 2000, 2100, 2200¡Adefault 1900
    // control AFE_VAC_CON0  VMIC_VREF
    unsigned short uMicbiasVolt;
} ft_l4aud_get_param_cnf_0809; // must the same as speech_param_struct in \src\common\cfgfileinc\CFG_AUDIO_File.h

typedef struct
{
    unsigned short  m_u2FailReason;
} ft_l4aud_set_param_cnf;

typedef struct
{
    /* Compensation Filter HSF coeffs       */
    /* BesLoudness also uses this coeffs    */
    unsigned int bes_loudness_hsf_coeff[9][4];

    /* Compensation Filter BPF coeffs       */
    unsigned int bes_loudness_bpf_coeff[4][6][3];
    unsigned int bes_loudness_DRC_Forget_Table[9][2];
    unsigned int bes_loudness_WS_Gain_Max;
    unsigned int bes_loudness_WS_Gain_Min;
    unsigned int bes_loudness_Filter_First;
    char bes_loudness_Gain_Map_In[5];
    char bes_loudness_Gain_Map_Out[5];

} ft_l4aud_get_acf_param_cnf; // must the same as audio_param_struct in \src\common\cfgfileinc\CFG_AUDIO_File.h

typedef struct
{
    unsigned short  m_u2FailReason;
} ft_l4aud_set_acf_param_cnf;

/*@dual mic record confirm message */
typedef struct
{
    record_states state;
} ft_l4aud_dualmic_record_cnf;

/*@dual mic update package confirm message */
typedef struct
{
    //    unsigned char data[2048];
    int size;
    int flag;  // 1 indicate file end
} ft_l4aud_ul_data_package_cnf;

/*@dual mic get parameters confirm message */
typedef struct
{
    char param_name[128];
    int value;
} ft_l4aud_dualmic_get_param_cnf;

typedef union
{
    ft_l4aud_query_id_cnf                 query_id;
    ft_tone_loopbackrec_cnf               tone_loopbackrec_cnf;
    ft_tone_loopbackrec_cnf_2k            tone_loopbackrec_cnf_2k;
    ft_l4aud_get_profile_by_mode_cnf      get_profile_cnf;
    ft_l4aud_set_profile_by_mode_cnf      set_profile_cnf;
    ft_l4aud_get_param_cnf_0809           get_param_cnf_0809;
    ft_l4aud_set_param_cnf                set_param_cnf;
    ft_l4aud_get_acf_param_cnf            get_acf_param_cnf;
    ft_l4aud_set_acf_param_cnf            set_acf_param_cnf;
    ft_l4aud_dualmic_record_cnf           dualmic_record_cnf;
    ft_l4aud_ul_data_package_cnf          ul_data_package_cnf; //add by Donglei
    ft_l4aud_dualmic_get_param_cnf        dualmic_get_params_cnf; //add by Donglei
    ft_l4aud_get_acf_param_cnf            get_hcf_param_cnf;
    ft_l4aud_set_acf_param_cnf            set_hcf_param_cnf;
    ft_l4aud_gaintable_cnt                    get_gain_table_cnf;
    ft_l4aud_gaintablenum_cnf             get_gaintablenum_cnf;
    ft_l4aud_gaintablelevel_cnf             get_gaintablelevel_cnf;
    ft_l4aud_ctrpointnum_cnt               get_ctrpoint_num_cnf;
    ft_l4aud_ctrpointbits_cnt                get_ctrpoint_bits_cnf;
    ft_l4aud_ctrpointtable_cnt              get_ctrpoint_table_cnf;
    unsigned int                          dummy;            // extend alignment to 4 bytes
} ft_l4aud_confirm;

typedef struct
{
    FT_H                                  header;
    FT_L4AUD_OP                  op;
    char                                   status;
    ft_l4aud_confirm            cnf;
} FT_L4AUD_CNF;


///------------------the following is common API---------------------------------
/********************************************************************************
//FUNCTION:
//      META_Audio_init
//DESCRIPTION:
//      this function is initialize the meta audio system resource. when meta or
//      factory mode test, it must be called first.
//
//PARAMETERS:
//      None
//
//RETURN VALUE:
//      TRUE meams success
//
//DEPENDENCY:
//      None
//
//GLOBALS AFFECTED
//      None
********************************************************************************/
bool META_Audio_init(void);

/********************************************************************************
//FUNCTION:
//      META_Audio_init
//DESCRIPTION:
//      this function is deinitialize the meta audio system resource. when meta or
//      factory mode test, it must be called first.
//
//PARAMETERS:
//      None
//
//RETURN VALUE:
//      TRUE meams success
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
********************************************************************************/
bool META_Audio_deinit(void);


///------------------the following is META test API-----------------------------
/********************************************************************************
//FUNCTION:
//      META_Audio_init
//DESCRIPTION:
//      this function is called to used to do meta related test case.
//
//PARAMETERS:
//      refers to the definition of FT_L4AUD_REQ
//
//RETURN VALUE:
//      refers to the difinition of FT_L4AUD_CNF
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
********************************************************************************/
void META_Audio_OP(FT_L4AUD_REQ *req, char *peer_buff, unsigned short peer_len);


///------------------the following is Factory mode test API----------------------
//
//FUNCTION:
//      RecieverLoopbackTest
//DESCRIPTION:
//      this function is called to test reciever loop back.
//
//PARAMETERS:
//      echoflag:   [IN]    (char)true mean enable, otherwise 0 is disable
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------

bool RecieverLoopbackTest(char echoflag);

//-------------------------------------------------------------------
//FUNCTION:
//      RecieverTest
//DESCRIPTION:
//      this function is called to test reciever channel using inner sine wave.
//
//PARAMETERS:
//      receiver_test:  [IN]    (char)true mean enable, otherwise 0 is disable
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
bool RecieverTest(char receiver_test);

//-------------------------------------------------------------------
//FUNCTION:
//      LouderSPKTest
//DESCRIPTION:
//      this function is called to test loud speaker channel using inner sine wave.
//
//PARAMETERS:
//      left_channel/right_channel:     [IN]    (char)true mean enable, otherwise 0 is turnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//------------------------------------------------------------------

bool LouderSPKTest(char left_channel, char right_channel);

//-------------------------------------------------------------------
//FUNCTION:
//      EarphoneLoopbackTest
//DESCRIPTION:
//      this function is called to test earphone loobback .
//
//PARAMETERS:
//      bEnable:    [IN]    (char)true mean enable, otherwise 0 is turnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------

bool EarphoneLoopbackTest(char bEnable);

//-------------------------------------------------------------------
//FUNCTION:
//      EarphoneTest
//DESCRIPTION:
//      this function is called to test earphone speaker channel using inner sine wave.
//
//PARAMETERS:
//      bEnable:    [IN]    (char)true mean enable, otherwise 0 is turnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------

bool EarphoneTest(char bEnable);

//-------------------------------------------------------------------
//FUNCTION:
//      FMLoopbackTest
//DESCRIPTION:
//      this function is called to enable or disable FM channel.
//
//PARAMETERS:
//      bEnable:    [IN]    (char)true mean enable, otherwise 0 is turnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------

/// bEnable true mean enable, otherwise 0 is disable
bool FMLoopbackTest(char bEnable);

//-------------------------------------------------------------------
//FUNCTION:
//      EarphoneMicbiasEnable
//DESCRIPTION:
//      this function is called to Enable Mic bias enable when earphone is insert.
//
//PARAMETERS:
//      bMicEnable:     [IN]    (char)true mean enable, otherwise 0 is turnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------

/// bMicEnable true mean enable, otherwise 0 is disable

bool EarphoneMicbiasEnable(bool bMicEnable);

//-------------------------------------------------------------------
//FUNCTION:
//      Audio_I2S_Play
//DESCRIPTION:
//      this function is called to Play FM via I2S.
//
//PARAMETERS:
//      enable_flag:    [IN]    1: enable playback.  0: disable playback
//
//RETURN VALUE:
//
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//
//-------------------------------------------------------------------
int Audio_I2S_Play(int enable_flag);
int Audio_FMTX_Play(bool Enable, unsigned int Freq);
int readRecordData(void *pbuffer, int bytes);
bool freqCheck(short pbuffer[], int bytes);
#ifdef __cplusplus
};
#endif

#endif
