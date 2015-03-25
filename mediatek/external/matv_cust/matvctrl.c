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
 *   @file matvctrl.c
 *		
 *   @author lh.hsiao 
 */
 
#include "kal_release.h"
//#include "kal_trace.h"
#include "matvctrl.h"

#define LOG_TAG "Matvctrl"
#include <utils/Log.h>

extern int cli_print(const char *fmt,...);

extern kal_bool matvdrv_init(void);
extern kal_bool matvdrv_ps_init(kal_bool on);
extern kal_bool matvdrv_suspend(kal_bool on);
extern kal_bool matvdrv_shutdown(void);
extern void matvdrv_register_callback(
	void* cb_param,
	matv_autoscan_progress_cb auto_cb,
	matv_fullscan_progress_cb full_cb,
	matv_scanfinish_cb finish_cb,
	matv_audioformat_cb audfmt_cb);
	
extern void matvdrv_set_country(kal_uint8 country);
extern void matvdrv_chscan(kal_uint8 mode);
extern void matvdrv_chscan_stop(void);
extern void matvdrv_chscan_query(matv_chscan_state *scan_state);

extern kal_bool matvdrv_get_chtable(kal_uint8 ch, matv_ch_entry *entry);
extern kal_bool matvdrv_set_chtable(kal_uint8 ch, matv_ch_entry *entry);
extern kal_bool matvdrv_clear_chtable(void);

extern void matvdrv_change_channel(kal_uint8 ch);

extern void matvdrv_audio_play(void);
extern void matvdrv_audio_stop(void);

extern kal_uint32 matvdrv_audio_get_format(void);
extern void matvdrv_audio_set_format(kal_uint32 val);
extern kal_uint8 matvdrv_audio_get_sound_system(void);
extern kal_uint8 matvdrv_audio_get_fm_detect_status(kal_uint32 *qulity);

extern kal_bool matvdrv_adjust(kal_uint8 item,kal_int32 val);

extern kal_int32 matvdrv_get_chipdep(kal_uint8 item);
extern kal_bool matvdrv_set_chipdep(kal_uint8 item,kal_int32 val);
extern kal_bool matvdrv_cli_input(kal_uint8 val);

extern void vApiAudioPlayTone(UINT8 tone_type, int output_percentage);
extern void vApiAudioInit (UINT8 ramboot,UINT8 output_mode);
extern kal_bool mATV_task_init(void);
extern kal_bool mATV_ps_task_init(void);
extern kal_bool mATV_task_close(void);
extern kal_bool matvdrv_ata_lockstatus(kal_uint32 freq, kal_uint8 counrty);
extern kal_bool matvdrv_ata_avpatternout(void);
extern kal_bool matvdrv_ata_avpatternclose(void);
#ifdef __ATV_SUPPORT__

extern kal_semid 	matv_sem_id;
extern int i2c_fd;

kal_int32 g_atvMode = 0;
void matv_AudioPlayTone(kal_uint8 tone_type, int output_percentage)
{
    vApiAudioInit(0, 0);
	vApiAudioPlayTone(tone_type,output_percentage);
}

extern void matvdrv_SetTpMode(kal_int32 tp_mode);

void matv_SetTpMode(kal_int32 tp_mode)
{
    matvdrv_SetTpMode(tp_mode);
}


//extern kal_eventgrpid matv_event_id;

// Country/ region setting
// Full Scan (scan all frequency range and list the scanned channel list)
// Stop Full Scan
// Select one channel on the channel list
// Direct set frequency (Set specific channel frequency)
//	Start to retrieve signal strength & quality for selected channel

/////////////////////////////////////////////////////////////////////////////
//power on/off/suspend
/////////////////////////////////////////////////////////////////////////////

kal_bool matv_init(void)
{
	kal_bool return_value;
	
	mATV_task_init();
	kal_prompt_trace(MOD_MATV, "[ATV] matv_init In");
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);
	
	return_value=matvdrv_init();
			
	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_init Out");
	
	return return_value;
}

kal_bool matv_ps_init(kal_bool on)
{
	kal_bool return_value;
	
	ALOGD("matv_ps_init i2c_fd:%d", i2c_fd);
    //added by HP Cheng, if i2c_fd > 0, it means matv has been initialized, so do not do ps init, or the driver will be closed
       // 20100906, HP, dont need this anymore. This case will not happen because we modify sensor_hal.cpp
       // to prevent calling ps_init when mATV case
       /*
	if ( i2c_fd > 0 ) {
		ALOGD("Already has fd, means init is done, do not do ps init again");
		return KAL_FALSE;
	}
	*/
	
	mATV_ps_task_init();
	kal_prompt_trace(MOD_MATV, "[ATV] matv_ps_init In");
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);
	
	return_value=matvdrv_ps_init(on);
			
	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_ps_init Out");
	
	return return_value;
}

kal_bool matv_suspend(kal_bool on)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_suspend In");
	
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_suspend(on);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);
	
	kal_prompt_trace(MOD_MATV, "[ATV] matv_suspend Out");
	
	return 	return_value;
}

kal_bool matv_shutdown(void)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_shutdown In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_shutdown();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_shutdown Out");

	return return_value;
}

/////////////////////////////////////////////////////////////////////////////
//callback
/////////////////////////////////////////////////////////////////////////////
void matv_register_callback(
	void* cb_param,
	matv_autoscan_progress_cb auto_cb,
	matv_fullscan_progress_cb full_cb,
	matv_scanfinish_cb finish_cb,
	matv_audioformat_cb audfmt_cb)
{
	matvdrv_register_callback(cb_param, auto_cb,full_cb,finish_cb,audfmt_cb);
}
/////////////////////////////////////////////////////////////////////////////
//ch scan/control
/////////////////////////////////////////////////////////////////////////////

// Full Scan (scan all frequency range and list the scanned channel list)
void matv_chscan(kal_uint8 mode)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_chscan In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	matvdrv_chscan(mode);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_chscan Out");

}

void matv_chscan_stop(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_chscan_stop In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	matvdrv_chscan_stop();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_chscan_stop Out");

}

void matv_chscan_query(matv_chscan_state *scan_state)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_chscan_query In");

	matvdrv_chscan_query(scan_state);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_chscan_query Out");
}


kal_bool matv_get_chtable(kal_uint8 ch, matv_ch_entry *entry)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_get_chtable In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_get_chtable(ch,entry);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);
	
	kal_prompt_trace(MOD_MATV, "[ATV] matv_get_chtable Out");

	return return_value;
}

kal_bool matv_set_chtable(kal_uint8 ch, matv_ch_entry *entry)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_set_chtable In, freq %d , sndsys %d, col: %d, flag %d", 
				entry->freq, entry->sndsys, entry->colsys, entry->flag);

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_set_chtable(ch,entry);

	g_atvMode = matvdrv_get_chipdep(187);
	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);
			
	kal_prompt_trace(MOD_MATV, "[ATV] matv_set_chtable Out");			

	return return_value;
}

kal_bool matv_clear_chtable(void)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_clear_chtable In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_clear_chtable();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_clear_chtable Out");

	return return_value;
}

void matv_change_channel(kal_uint8 ch)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_change_channel In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	matvdrv_change_channel(ch);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_change_channel Out");

}

void matv_set_country(kal_uint8 country)
{

	kal_prompt_trace(MOD_MATV, "[ATV] matv_set_country In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	if(country<TV_COUNTRY_MAX)
		matvdrv_set_country(country);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_set_country Out");
}

/////////////////////////////////////////////////////////////////////////////
//AUDIO
/////////////////////////////////////////////////////////////////////////////	
void matv_audio_play(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_play In");
	
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	matvdrv_audio_play();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);
			
	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_play Out");
}

void matv_audio_stop(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_stop In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	matvdrv_audio_stop();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_stop out");
}

kal_uint32 matv_audio_get_format(void)
{
	kal_uint32 return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_format In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);	

	return_value=matvdrv_audio_get_format();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);
	
	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_format Out");

	return return_value;
}

void matv_audio_set_format(kal_uint32 val)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_set_format In");
	
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);	
	
	//matvdrv_audio_set_format(val);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_set_format Out");
}

kal_uint8 matv_audio_get_sound_system(void)
{
	kal_uint8 return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_sound_system In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_audio_get_sound_system();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_sound_system Out");

	return return_value;
}

kal_uint8 matv_audio_get_fm_detect_status(kal_uint32 *qulity)
{
	kal_uint8 return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_fm_detect_status In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_audio_get_fm_detect_status(qulity);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_fm_detect_status Out");

	return return_value;
}


/////////////////////////////////////////////////////////////////////////////
//Common adjust for Video/Audio
/////////////////////////////////////////////////////////////////////////////

kal_bool matv_adjust(kal_uint8 item,kal_int32 val)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_adjust In, item=%d, value=%d",item, val);

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_adjust(item,val);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_adjust Out, return=%d",return_value);

	return return_value;
}

/////////////////////////////////////////////////////////////////////////////
//Chip/Vendor dependent
/////////////////////////////////////////////////////////////////////////////
kal_int32 matv_get_mode()
{
	return g_atvMode;
}

kal_int32 matv_get_chipdep(kal_uint8 item)
{
	kal_int32 return_value;
	
	kal_prompt_trace(MOD_MATV, "[ATV] matv_get_chipdep In, item=%d",item);

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_get_chipdep(item);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_get_chipdep Out, return=%d",return_value);

	return return_value;
}

kal_bool matv_set_chipdep(kal_uint8 item,kal_int32 val)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_set_chipdep In, item=%d, value=%d",item, val);

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_set_chipdep(item,val);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_set_chipdep Out, return=%d", return_value);
	
	return return_value;
}

kal_bool matv_cli_input(kal_uint8 val)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_cli_input In, value=%d", val);

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_cli_input(val);
	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_cli_input Out, return=%d", return_value);
	
	return return_value;
}

kal_bool matv_ata_lockstatus(kal_uint32 freq, kal_uint8 counrty)
{
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[ATV] matv_ata_lockstatus In, freq=%d counrty=%d\n", freq,counrty);

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_ata_lockstatus(freq,counrty);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_ata_lockstatus Out, return=%d", return_value);
	
	return return_value;
}

kal_bool matv_ata_avpatternout(void)
{
	kal_bool return_value;
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_ata_avpatternout();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_cli_input Out, return=%d", return_value);
	
	return return_value;
}

kal_bool matv_ata_avpatternclose(void)
{
	kal_bool return_value;
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=matvdrv_ata_avpatternclose();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[ATV] matv_cli_input Out, return=%d", return_value);
	
	return return_value;
}


#else

void matv_AudioPlayTone(kal_uint8 tone_type, int output_percentage)
{
}

void matv_SetTpMode(kal_int32 tp_mode)
{
}

kal_bool matv_init(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_init dummy");	
	return 0;
}

kal_bool matv_ps_init(kal_bool on)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_init dummy");	
	return 0;
}

kal_bool matv_suspend(kal_bool on)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_init dummy");	
	return 0;
}

kal_bool matv_shutdown(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_init dummy");	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//callback
/////////////////////////////////////////////////////////////////////////////
void matv_register_callback(
	void* cb_param,
	matv_autoscan_progress_cb auto_cb,
	matv_fullscan_progress_cb full_cb,
	matv_scanfinish_cb finish_cb,
	matv_audioformat_cb audfmt_cb)
{
}
/////////////////////////////////////////////////////////////////////////////
//ch scan/control
/////////////////////////////////////////////////////////////////////////////

// Full Scan (scan all frequency range and list the scanned channel list)
void matv_chscan(kal_uint8 mode)
{
}

void matv_chscan_stop(void)
{
}

void matv_chscan_query(matv_chscan_state *scan_state)
{
}


kal_bool matv_get_chtable(kal_uint8 ch, matv_ch_entry *entry)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_get_chtable dummy");	
	return 0;
}

kal_bool matv_set_chtable(kal_uint8 ch, matv_ch_entry *entry)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_set_chtable dummy");	
	return 0;
}

kal_bool matv_clear_chtable(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_clear_chtable dummy");	
	return 0;
}

void matv_change_channel(kal_uint8 ch)
{
}

void matv_set_country(kal_uint8 country)
{
}

void matv_audio_play(void)
{
}

void matv_audio_stop(void)
{
}

kal_uint32 matv_audio_get_format(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_format dummy");	
	return 0;

}

void matv_audio_set_format(kal_uint32 val)
{
}

kal_uint8 matv_audio_get_sound_system(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_sound_system dummy");	
	return 0;
}

kal_uint8 matv_audio_get_fm_detect_status(kal_uint32 *qulity)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_audio_get_fm_detect_status dummy");	
	return 0;
}

kal_bool matv_adjust(kal_uint8 item,kal_int32 val)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_adjust dummy");	
	return 0;
}

kal_int32 matv_get_mode()
{
	return 0;
}

kal_int32 matv_get_chipdep(kal_uint8 item)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_get_chipdep dummy");	
	return 0;
}

kal_bool matv_set_chipdep(kal_uint8 item,kal_int32 val)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_set_chipdep dummy");	
	return 0;
}

kal_bool matv_cli_input(kal_uint8 val)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_cli_input dummy");	
	return 0;
}

kal_bool matv_ata_lockstatus(kal_uint32 freq, kal_uint8 counrty)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_ata_lockstatus dummy");	
			return 0;
}

kal_bool matv_ata_avpatternout(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matv_ata_avpatternout dummy");	
		return 0;
}

kal_bool matvdrv_ata_avpatternclose(void)
{
	kal_prompt_trace(MOD_MATV, "[ATV] matvdrv_ata_avpatternclose dummy");	
		return 0;
}
#endif /* __MTK_TARGET__ && __ATV_SUPPORT__ */
