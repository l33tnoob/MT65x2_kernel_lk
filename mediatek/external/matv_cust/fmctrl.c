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
#include "fmctrl.h"

#define LOG_TAG "Matvctrl"
#include <utils/Log.h>


extern kal_bool mATV_task_init(void);
extern kal_bool mATV_ps_task_init(void);
extern kal_bool mATV_task_close(void);

extern kal_bool fmdrv_init(void);
extern kal_uint32 fmdrv_getrssi(void);
extern kal_bool fmdrv_tune(struct fm_tune_parm *parm);
extern kal_bool fmdrv_seek(struct fm_seek_parm *parm);
extern kal_bool fmdrv_powerdown(kal_uint32 val);
extern UINT32 fmdrv_getchipid(void);
extern kal_bool fmdrv_mute(kal_uint32 val);
extern kal_bool fmdrv_scan(struct fm_scan_parm *parm);


#ifdef __ATV_SUPPORT__

extern kal_semid 	matv_sem_id;

// Country/ region setting
// Full Scan (scan all frequency range and list the scanned channel list)
// Stop Full Scan
// Select one channel on the channel list
// Direct set frequency (Set specific channel frequency)
//	Start to retrieve signal strength & quality for selected channel

/////////////////////////////////////////////////////////////////////////////
//power on/off/suspend
/////////////////////////////////////////////////////////////////////////////
kal_bool fm_powerup(struct fm_tune_parm *parm){
	kal_bool return_value;
	
	mATV_task_init();
	kal_prompt_trace(MOD_MATV, "[FM] fm_powerup In");
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);
	
	return_value=fmdrv_init();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);
	
	if(return_value)
	{
		return_value = fm_tune(parm);
	}
			

	kal_prompt_trace(MOD_MATV, "[FM] fm_powerup Out");
	
	return return_value;
}
kal_bool fm_powerdown(kal_uint32 val){
	kal_bool return_value;

	kal_prompt_trace(MOD_MATV, "[FM] fm_powerdown In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=fmdrv_powerdown(val);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[FM] fm_powerdown Out");

	return return_value;
}
int fm_getrssi(void){
	kal_uint32 return_value;

	kal_prompt_trace(MOD_MATV, "[FM] fm_getrssi In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);	

	return_value=fmdrv_getrssi();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);
	
	kal_prompt_trace(MOD_MATV, "[FM] fm_getrssi Out");

	return return_value;
}
kal_bool fm_tune(struct fm_tune_parm *parm){
	kal_bool return_value;
	
	kal_prompt_trace(MOD_MATV, "[FM] fm_tune In");
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);

	return_value=fmdrv_tune(parm);
			
	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[FM] fm_tune Out");
	
	return return_value;
}
kal_bool fm_seek(struct fm_seek_parm *parm){
	kal_bool return_value;
	
	kal_prompt_trace(MOD_MATV, "[FM] fm_seek In");
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);
	
	return_value=fmdrv_seek(parm);
			
	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[FM] fm_seek Out");
	
	return return_value;
}
kal_bool fm_scan(struct fm_scan_parm *parm){
	kal_bool return_value;
	
	kal_prompt_trace(MOD_MATV, "[FM] fm_scan In");
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);
	
	return_value=fmdrv_scan(parm);
			
	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[FM] fm_scan Out");
	
	return return_value;
}
kal_bool fm_mute(kal_uint32 val){ 
	kal_uint32 return_value;

	kal_prompt_trace(MOD_MATV, "[FM] fm_mute In");
	
	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);	
	
	return_value = fmdrv_mute(val);

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);

	kal_prompt_trace(MOD_MATV, "[FM] fm_mute Out");

	return return_value; 
}
int fm_getchipid(void){
	kal_uint32 return_value;

	kal_prompt_trace(MOD_MATV, "[FM] fm_getchipid In");

	if (NULL != matv_sem_id)
			kal_take_sem(matv_sem_id, KAL_INFINITE_WAIT);	

	return_value=fmdrv_getchipid();

	if (NULL != matv_sem_id)
			kal_give_sem(matv_sem_id);
	
	kal_prompt_trace(MOD_MATV, "[FM] fm_getchipid Out");

	return return_value; 
}
#else
kal_bool fm_powerup(struct fm_tune_parm *parm){
	
	kal_prompt_trace(MOD_MATV, "[FM] fm_powerup Dummy");	
	return 0;
}
kal_bool fm_powerdown(kal_uint32 val){
	kal_prompt_trace(MOD_MATV, "[FM] fm_powerup Dummy");	
	return 0;
}
int fm_getrssi(void){
	kal_prompt_trace(MOD_MATV, "[FM] fm_getrssi Dummy");	
	return 0;
}
kal_bool fm_tune(struct fm_tune_parm *parm){
	kal_prompt_trace(MOD_MATV, "[FM] fm_tune Dummy");	
	return 0;
}
kal_bool fm_seek(struct fm_seek_parm *parm){
	kal_prompt_trace(MOD_MATV, "[FM] fm_seek Dummy");	
	return 0;
}
kal_bool fm_scan(struct fm_scan_parm *parm){
	kal_prompt_trace(MOD_MATV, "[FM] fm_scan Dummy");	
	return 0;
}
kal_bool fm_mute(kal_uint32 val){ 
	kal_prompt_trace(MOD_MATV, "[FM] fm_mute Dummy");	
	return 0;
}
int fm_getchipid(void){
	kal_prompt_trace(MOD_MATV, "[FM] fm_getchipid Dummy");	
	return 0;
}


#endif /* __MTK_TARGET__ && __ATV_SUPPORT__ */
