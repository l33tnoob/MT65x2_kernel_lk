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



#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <linux/ioctl.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"


#ifdef FEATURE_FTM_HEADSET
#include "ftm_audio_Common.h"
//#include <AudioYusuHeadsetMessage.h>

#ifdef __cplusplus
extern "C" {
#include "DIF_FFT.h"
#include "Audio_FFT_Types.h"
#include "Audio_FFT.h"
#endif


#define TAG                 "[HEADSET] "
#define HEADSET_STATE_PATH "/sys/class/switch/h2w/state"

#define BUF_LEN 1
char rbuf_headset_debug[BUF_LEN] = {'\0'};
char wbuf_headset_debug[BUF_LEN] = {'1'};
char wbuf_2_headset_debug[BUF_LEN] = {'2'};

enum {
		ITEM_RINGTONE,
		ITEM_HEADSET_MIC,
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t headset_items[] = {
    {ITEM_RINGTONE, uistr_info_audio_ringtone, COLOR_BLACK},
    {ITEM_HEADSET_MIC, uistr_info_audio_loopback_headset_mic, COLOR_BLACK},
    {ITEM_PASS, uistr_pass, COLOR_BLACK},
    {ITEM_FAIL, uistr_fail, COLOR_BLACK},
    {-1, NULL, 0},
};
struct headset {
    char  info[1024];
	  bool  avail;
	  bool  Headset_mic;
    bool  exit_thd;
    pthread_t hRecordThread;
    pthread_t headset_update_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
    int  recordDevice;
    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
};

#define mod_to_headset(p)     (struct headset*)((char*)(p) + sizeof(struct ftm_module))

#define ACCDET_IOC_MAGIC 'A'
#define ACCDET_INIT       _IO(ACCDET_IOC_MAGIC,0)  // call wehn first time
#define SET_CALL_STATE    _IO(ACCDET_IOC_MAGIC,1)  // when state is changing , tell headset driver.
#define GET_BUTTON_STATUS _IO(ACCDET_IOC_MAGIC,2)  // ioctl to get hook button state.
static const char *HEADSET_PATH= "/dev/accdet";

// use for ioctl for headset driver
static int HeadsetFd =0;
extern sp_ata_data return_data;

static void headset_update_info(struct headset *hds, char *info)
{

    char *ptr;
    int rc;
	int fd = -1;
	int hb_status = 0;
	int ret = 0;
	hds->Headset_mic = false;
  
	fd = open(HEADSET_STATE_PATH, O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "Can't open %s\n", HEADSET_STATE_PATH);
		hds->avail = false;
		goto EXIT;
	}
	if (read(fd, rbuf_headset_debug, BUF_LEN) == -1) {
		LOGD(TAG "Can't read %s\n", HEADSET_STATE_PATH);
		hds->avail = false;
		goto EXIT;
	}
	if (!strncmp(wbuf_headset_debug, rbuf_headset_debug, BUF_LEN)) { /*the same*/
		hds->avail = true;
		hds->Headset_mic = true;
    } else {
    	hds->avail = false;
    }
	if (!strncmp(wbuf_2_headset_debug, rbuf_headset_debug, BUF_LEN)) { /*the same*/
		hds->avail = true;
		hds->Headset_mic = false;
		}

EXIT:
	close(fd);

	//if (hds->avail) {
	if (!strncmp(wbuf_headset_debug, rbuf_headset_debug, BUF_LEN)) {
#ifdef HEADSET_BUTTON_DETECTION
	// open headset device
	HeadsetFd = open(HEADSET_PATH, O_RDONLY);
	if(HeadsetFd <0){
		LOGD(TAG "FTM:HEADSET open %s error fd = %d",HEADSET_PATH,HeadsetFd);
		goto EXIT_HEADSET;
	}

	// enable button detection
	LOGD(TAG "enable button detection \n");
	ret = ::ioctl(HeadsetFd,SET_CALL_STATE,1);

	// read button status
	LOGD(TAG "read button status \n");
	ret = ::ioctl(HeadsetFd,GET_BUTTON_STATUS,0);
    if(ret == 0){
    	hb_status = 0;
    } else {
    	hb_status = 1;
    }

	// disable button detection
	//LOGD(TAG "disable button detection \n");
	//ret = ::ioctl(HeadsetFd,HEADSET_SET_STATE,0);

EXIT_HEADSET:
	close(HeadsetFd);
#endif
	}

    /* preare text view info */
    ptr  = info;
    if(!hds->avail)
       ptr += sprintf(ptr, "%s",uistr_info_audio_headset_note);
    ptr += sprintf(ptr, "%s : %s\n\n",uistr_info_avail ,hds->avail ? "Yes" : "No");
#if 0
	if (hds->avail)
		ptr += sprintf(ptr, "Please listen the sound from the headset\n\n");
	else
		ptr += sprintf(ptr, "Please insert the headset to test this item\n\n");
#endif

#ifdef HEADSET_BUTTON_DETECTION
	if (hds->avail)
		ptr += sprintf(ptr, "%s: %s\n\n",uistr_info_button, hb_status ? uistr_info_press: uistr_info_release);
#endif
	if (hds->avail)
	{
		ptr += sprintf(ptr,uistr_info_audio_headset_mic_avail, hds->Headset_mic? uistr_info_audio_yes : uistr_info_audio_no);
	  if (!hds->Headset_mic)
		  ptr += sprintf(ptr, "%s:N/A\n",uistr_info_audio_loopback_headset_mic);
	}
           
    return;
}


static void *headset_update_iv_thread(void *priv)
{
    struct headset *hds = (struct headset *)priv;
    struct itemview *iv = hds->iv;
    int chkcnt = 5;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    while (1) {
        //usleep(200000);
        usleep(20000);
        chkcnt--;

        if (hds->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

		headset_update_info(hds, hds->info);
   	
        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
        chkcnt = 5;
    }

    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);

	return NULL;
}

int headset_debug_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct headset *hds = (struct headset *)priv;
    struct textview *tv;
    struct itemview *iv;
    int privChosen = -1;
    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&hds->title, param->name, COLOR_YELLOW);
    init_text(&hds->text, &hds->info[0], COLOR_YELLOW);
    init_text(&hds->left_btn, "Fail", COLOR_YELLOW);
    init_text(&hds->center_btn, "Pass", COLOR_YELLOW);
    init_text(&hds->right_btn, "Back", COLOR_YELLOW);

    headset_update_info(hds, hds->info);

    hds->exit_thd = false;

	if (!hds->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        hds->iv = iv;
    }

    iv = hds->iv;
    iv->set_title(iv, &hds->title);
    iv->set_items(iv, headset_items, 0);
    iv->set_text(iv, &hds->text);

    Common_Audio_init();

    pthread_create(&hds->headset_update_thd, NULL, headset_update_iv_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_RINGTONE:
        	   if(!hds->avail || privChosen == ITEM_RINGTONE)
        	   	 break;
        	   if(privChosen == ITEM_HEADSET_MIC && hds->Headset_mic)
        	   	 HeadsetMic_EarphoneLR_Loopback(0,1);
        	   usleep(20000);
        	   EarphoneTest(1);
        	   privChosen = ITEM_RINGTONE;
        	   break;
        case ITEM_HEADSET_MIC:
        	   if(!hds->avail || privChosen == ITEM_HEADSET_MIC || !hds->Headset_mic)
        	   	 break;
        	   if(privChosen == ITEM_RINGTONE)
        	   	 EarphoneTest(0);
        	   usleep(20000);
        	   HeadsetMic_EarphoneLR_Loopback(1,1);
        	   privChosen = ITEM_HEADSET_MIC;	        	   
        	   break;
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                hds->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                hds->mod->test_result = FTM_TEST_FAIL;
            }
            exit = true;
            break;
        }

        if (exit) {
            hds->exit_thd = true;
            break;
        }
    } while (1);
    
    if(privChosen == ITEM_RINGTONE)
    	EarphoneTest(0);
    if(privChosen == ITEM_HEADSET_MIC)
    	HeadsetMic_EarphoneLR_Loopback(0,1);
    	
    pthread_join(hds->headset_update_thd, NULL);
    
    Common_Audio_deinit();
    
    return 0;
}

int headset_debug_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct headset *hds;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_HEADSET_DEBUG, sizeof(struct headset));
    hds = mod_to_headset(mod);

    hds->mod    = mod;
	  hds->avail	= false;

    if (!mod)
        return -ENOMEM;

    ret = ftm_db_register(mod, headset_debug_entry, (void*)hds);

    return ret;
}


#ifdef __cplusplus
}
#endif

#endif
