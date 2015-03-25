/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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
/**
 *   @file matvctrl.h
 *		
 *   @author lh.hsiao 
 */
/////////////////////////////////////////////////////////////////////////////
//power on/off/suspend
/////////////////////////////////////////////////////////////////////////////
#ifndef __MATVCTRL_H__
#define __MATVCTRL_H__

#include "kal_release.h"
#ifdef __cplusplus
extern "C"
{
#endif
kal_bool matv_init(void);
kal_bool matv_ps_init(kal_bool on);
kal_bool matv_suspend(kal_bool on);
kal_bool matv_shutdown(void);

//by HP cheng
void matv_AudioPlayTone(kal_uint8 tone_type, int output_percentage);

//by Charlie Lu
void matv_SetTpMode(int tp_mode);

/////////////////////////////////////////////////////////////////////////////
//ch scan/control
/////////////////////////////////////////////////////////////////////////////
typedef enum 
{
	MATV_AUTOSCAN,
	MATV_FULLSCAN,
	MATV_QUICKSCAN
} matv_chscan_mode;

void matv_chscan(kal_uint8 mode);
void matv_chscan_stop(void);
/////////////////////////////////////////////////////////////////////////////
//callback
/////////////////////////////////////////////////////////////////////////////
typedef  void (*matv_autoscan_progress_cb)(void* cb_param, kal_uint8 percent,kal_uint8 ch,kal_uint8 chnum);
typedef  void (*matv_fullscan_progress_cb)(void* cb_param, kal_uint8 percent,kal_uint32 freq,kal_uint32 freq_start,kal_uint32 freq_end);
typedef  void (*matv_scanfinish_cb)(void* cb_param, kal_uint8 chnum);
typedef  void (*matv_audioformat_cb)(void* cb_param, kal_uint32 format);
void matv_register_callback(void* cb_param,
	matv_autoscan_progress_cb auto_cb,
	matv_fullscan_progress_cb full_cb,
	matv_scanfinish_cb finish_cb,
	matv_audioformat_cb audfmt_cb);

/////////////////////////////////////////////////////////////////////////////
//ch scan/control
/////////////////////////////////////////////////////////////////////////////

/* Color system */
enum
{
	SV_CS_PAL_N,
	SV_CS_PAL,
	SV_CS_PAL_M,
	SV_CS_NTSC358,
	SV_CS_SECAM,
	SV_CS_PAL_60,
	SV_CS_UNSTABLE,		
	SV_CS_NTSC443,
	SV_CS_AUTO,
	SV_CS_NONE,
	SV_CS_MAX
};

#ifndef __TV_AUD_SYS_T__
#define __TV_AUD_SYS_T__

typedef enum 
{
	SV_NONE_DETECTED,	//0x0
	SV_MTS,			//0x1
	SV_FM_FM,		//0x2
	SV_NTSC_M,		//0x3
	SV_A2_BG,		//0x4
	SV_A2_DK1,		//0x5
	SV_A2_DK2,		//0x6
	SV_A2_DK3,		//0x7
	SV_PAL_I,		//0x8
	SV_PAL_BG,		//0x09
	SV_PAL_DK,		//0x0a
	SV_SECAM_L,         //0x0b
	SV_SECAM_L_PLUM,    //0x0c
	SV_SECAM_BG,	    //0x0d
	SV_SECAM_DK,        //0x0e
        SV_FMRDO,           //0x0f
	SV_SLT,             //0x10
	SV_PAL_I_FMMONO,    //0x11
	SV_PAL_BG_FMMONO,   //0x12
	SV_PAL_DK_FMMONO,   //0x13
	SV_KOREA,           //0x14
	SV_NUM	
} TV_AUD_SYS_T;
#endif


enum
{
	CH_VALID=1
};

typedef struct 
{
	kal_uint32	freq; //khz
	kal_uint8	sndsys;	/* reference sv_const.h, TV_AUD_SYS_T ...*/
	kal_uint8	colsys;	/* reference sv_const.h, SV_CS_PAL_N, SV_CS_PAL,SV_CS_NTSC358...*/
	kal_uint8	flag;
} matv_ch_entry;

kal_bool matv_get_chtable(kal_uint8 ch, matv_ch_entry *entry);
kal_bool matv_set_chtable(kal_uint8 ch, matv_ch_entry *entry);
kal_bool matv_clear_chtable(void);

void matv_change_channel(kal_uint8 ch);

enum {
TV_AFGHANISTAN,
TV_ARGENTINA,
TV_AUSTRALIA,
TV_BRAZIL,
TV_BURMA,
TV_CAMBODIA,
TV_CANADA,
TV_CHILE,
TV_CHINA,
TV_CHINA_HONGKONG,
TV_CHINA_SHENZHEN,
TV_EUROPE_EASTERN,
TV_EUROPE_WESTERN,
TV_FRANCE,
TV_FRENCH_COLONIE,
TV_INDIA,
TV_INDONESIA,
TV_IRAN,
TV_ITALY,
TV_JAPAN,
TV_KOREA,
TV_LAOS,
TV_MALAYSIA,
TV_MEXICO,
TV_NEWZEALAND,
TV_PAKISTAN,
TV_PARAGUAY,
TV_PHILIPPINES,
TV_PORTUGAL,
TV_RUSSIA,
TV_SINGAPORE,
TV_SOUTHAFRICA,
TV_SPAIN,
TV_TAIWAN,
TV_THAILAND,
TV_TURKEY,
TV_UNITED_ARAB_EMIRATES,
TV_UNITED_KINGDOM,
TV_USA,
TV_URUGUAY,
TV_VENEZUELA,
TV_VIETNAM,
TV_IRELAND,
TV_MOROCCO,
TV_COUNTRY_MAX,
//alias
TV_BANGLADESH=TV_PAKISTAN
};
void matv_set_country(kal_uint8 country);


//query function used in chscan callback

typedef struct 
{
	kal_uint8		mode;
	kal_bool		is_scanning;
	kal_uint8		ch_latest_updated;
	matv_ch_entry		updated_entry;
} matv_chscan_state;

void matv_chscan_query(matv_chscan_state *scan_state);
/////////////////////////////////////////////////////////////////////////////
//AUDIO
/////////////////////////////////////////////////////////////////////////////
	/* MTS auto update use */
	#define MTS_MONO        0x00000001
	#define MTS_STEREO      0x00000002
	#define MTS_SAP         0x00000004

	/* Japan & Korea MPX */
	#define MPX_MONO        0x00000008
	#define MPX_STEREO      0x00000010
	#define MPX_SUB         0x00000020
	#define MPX_MAIN_SUB    0x00000040
	#define MPX_MAIN        0x00000080

	#define FM_MONO         0x00000100
	#define A2_STEREO       0x00000200
	#define A2_DUAL1        0x00000400
	#define A2_DUAL2        0x00000800
	#define NICAM_MONO      0x00001000
	#define NICAM_STEREO    0x00002000
	#define NICAM_DUAL1     0x00004000
	#define NICAM_DUAL2     0x00008000

	#define FMRDO_MONO      0x00010000
	#define FMRDO_STEREO    0x00020000
	
	#define FORMAT_DETECTING 0x00000000

	
void matv_audio_play(void);
void matv_audio_stop(void);

kal_uint32 matv_audio_get_format(void);
void matv_audio_set_format(kal_uint32 val);

//META tool API
kal_uint8 matv_audio_get_sound_system(void);
kal_uint8 matv_audio_get_fm_detect_status(kal_uint32 *qulity);
kal_int32 matv_get_mode();
kal_int32 matv_get_chipdep(kal_uint8 item);
kal_bool matv_set_chipdep(kal_uint8 item,kal_int32 val);
kal_bool matv_cli_input(kal_uint8 val);
kal_bool matv_ata_lockstatus(kal_uint32 freq, kal_uint8 counrty);
kal_bool matv_ata_avpatternout(void);
kal_bool matv_ata_avpatternclose();


//kal_uint8 matv_readbyte(kal_uint32 addr);
//void matv_writebyte(kal_uint32 addr,kal_uint8 data);

/////////////////////////////////////////////////////////////////////////////
//Common adjust for Video/Audio
/////////////////////////////////////////////////////////////////////////////
enum
{
	MATV_AUD_VOLUME,
	MATV_VDO_BRIGHTNESS,
	MATV_VDO_CONTRAST,
	MATV_VDO_SATURATION
};
kal_bool matv_adjust(kal_uint8 item,kal_int32 val);


/////////////////////////////////////////////////////////////////////////////
//Chip/Vendor dependent
/////////////////////////////////////////////////////////////////////////////

enum
{
	// item < 100, common info provide by chip
	SIG_RSSI=0,
	SIG_SNR,
	SIG_STRENGTH
		
	/* item >= 100, variant info, not fixed
	 * example
	 * MTK_GAIN0=100,
	 * MTK_GAIN1=101 
	 */	
};
#ifdef __cplusplus
}
#endif
#endif
