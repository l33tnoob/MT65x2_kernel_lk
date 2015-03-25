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

#include "cust.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef __cplusplus
extern "C" {
#include "DIF_FFT.h"
#include "Audio_FFT_Types.h"
#include "Audio_FFT.h"
#endif
#include "ftm_audio_Common.h"

#define VIBRATOR_AUTOTEST

//add by chipeng
//#define VIBRATOR_SPEAKER
//#if defined(MTK_VIBSPK_SUPPORT)
#include "ftm_audio_Common.h"
#include "SineWave_156Hz.h"
//#endif
#include <cutils/properties.h>



#define TAG   "[Vibrator] "
static uint32_t vibrator_time = 0;
extern int status;

#ifdef FEATURE_FTM_VIBRATOR

#define VIBRATOR_ENABLE "/sys/class/timed_output/vibrator/enable"

bool vibrator_test_exit = false;
static pthread_t vibrator_thread;

enum {
	ITEM_PASS,
	ITEM_FAIL
};

static item_t items[] = {
	item(ITEM_PASS,   uistr_pass),
	item(ITEM_FAIL,   uistr_fail),
	item(-1, NULL),
};

#ifdef VIBRATOR_AUTOTEST
static item_t items_auto[] = {
	{-1, NULL, 0},
};

struct mVibrator {
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
    pthread_t hHeadsetThread;
	pthread_t hRecordThread;
	pthread_mutex_t mHeadsetMutex;
    int avail;
    int Headset_change;
    int Headset_mic;
    bool exit_thd;
    char  info[1024]; //TEXT_LENGTH
    char  file_name[100]; //MAX_FILE_NAME_SIZE
    int   i4OutputType;
    int   i4Playtime;
	int  recordDevice;
    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
};

#define mod_to_mVibrator(p)     (struct mVibrator*)((char*)(p) + sizeof(struct ftm_module))

extern sp_ata_data return_data; // Feed data for ATA Tool
#endif

const char PROPERTY_KEY_VIBSPK_ON_FTM[PROPERTY_KEY_MAX] = "persist.af.feature.vibspk";

static bool IsFTMSupportVibSpk(void)
{
    bool bSupportFlg = false;
    char stForFeatureUsage[PROPERTY_VALUE_MAX];

#if defined(MTK_VIBSPK_SUPPORT)
    property_get(PROPERTY_KEY_VIBSPK_ON_FTM, stForFeatureUsage, "1"); //"1": default on
#else
    property_get(PROPERTY_KEY_VIBSPK_ON_FTM, stForFeatureUsage, "0"); //"0": default off
#endif
    bSupportFlg = (stForFeatureUsage[0] == '0') ? false : true;

    return bSupportFlg;
}


static void *update_vibrator_thread_vibspk(void *mPtr)
{
    Audio_VSCurrent_Enable(true);
    Audio_VSCurrent_GetFrequency();
    
    while (1) {
        char *ptr;
        if (vibrator_test_exit){
            break;
    }
        Audio_VSCurrent_WriteRoutine();
    }
    
    Audio_VSCurrent_Enable(false);
    LOGD("VibSpkFactory Thread Exit \n");
    pthread_exit(NULL); // thread exit
    return NULL;
}

static int
write_int(char const* path, int value)
{
	int fd;

	if (path == NULL)
		return -1;

	fd = open(path, O_RDWR);
	if (fd >= 0) {
		char buffer[20];
		int bytes = sprintf(buffer, "%d\n", value);
		int amt = write(fd, buffer, bytes);
		close(fd);
		return amt == -1 ? -errno : 0;
	}

	LOGE("write_int failed to open %s\n", path);
	return -errno;
}

static void *update_vibrator_thread_default(void *priv)
{
	LOGD("%s: Start\n", __FUNCTION__);

	if(vibrator_time == 0)
	{
	do {
        write_int(VIBRATOR_ENABLE, 8000); // 1 seconds
		if (vibrator_test_exit)
			break;
		sleep(1);
		} while (1);	
		write_int(VIBRATOR_ENABLE, 0);
	}
	else
	{
		LOGD("%s: write vibrator_enable=%d\n", __FUNCTION__, vibrator_time);
		write_int(VIBRATOR_ENABLE, vibrator_time);
		sleep(1);
	write_int(VIBRATOR_ENABLE, 0);
		LOGD("%s: write vibrator_enable=0\n", __FUNCTION__);
	}

	pthread_exit(NULL);

	LOGD("%s: Exit\n", __FUNCTION__);

	return NULL;
}

static void *update_vibrator_thread(void *priv)
{
    if (IsFTMSupportVibSpk())
        return update_vibrator_thread_vibspk(priv);
    else
        return update_vibrator_thread_default(priv);
}

#ifdef VIBRATOR_AUTOTEST
static void * Audio_Record_thread(void *mPtr)
{
    struct mVibrator *hds  = (struct mVibrator *)mPtr;
    ALOGD(TAG "%s: Start", __FUNCTION__);
    usleep(100000);
    bool dumpFlag = 0; // read_preferred_recorddump() = 0
//    dumpFlag=true;//for test
    int magLower = 0,magUpper = 0;
    //read_preferred_magnitude(hds->i4OutputType,&magUpper,&magLower);
    //int freqOfRingtone = read_preferred_ringtone_freq();
    magLower = 1000;
    magUpper = 1000000000;
	int freqOfRingtone = 200;
	
    int lowFreq = freqOfRingtone * (1-0.1);
    int highFreq = freqOfRingtone * (1+0.1);
    short pbuffer[8192]={0};	
    short pbufferL[4096]={0};	
	  short pbufferR[4096]={0};	
    unsigned int freqDataL[3]={0},magDataL[3]={0};
	  unsigned int freqDataR[3]={0},magDataR[3]={0};
    int checkCnt = 0;

    return_data.vibrator.freq = 0;
    return_data.vibrator.ampl = 0;
	
    recordInit(hds->recordDevice);
    while (1) {
       memset(pbuffer,0,sizeof(pbuffer));
       memset(pbufferL,0,sizeof(pbufferL));
       memset(pbufferR,0,sizeof(pbufferR));
       
       int readSize  = readRecordData(pbuffer,8192*2);
       int i;
	   for(i = 0 ; i < 4096 ; i++)
       {
           pbufferL[i] = pbuffer[2 * i];
           pbufferR[i] = pbuffer[2 * i + 1];
       }

	    if(dumpFlag)
        {
            char filenameL[]="/data/record_dataL.pcm";
            char filenameR[]="/data/record_dataR.pcm";
		    FILE * fpL= fopen(filenameL, "wb+");
            FILE * fpR= fopen(filenameR, "wb+");
 		    
            if(fpL!=NULL)
	        {
	           fwrite(pbufferL,readSize/2,1,fpL);
		       fclose(fpL);
            }
	   
            if(fpR!=NULL)
	        {
	           fwrite(pbufferR,readSize/2,1,fpR);
		       fclose(fpR);
            }
        } 
        memset(freqDataL,0,sizeof(freqDataL));
        memset(freqDataR,0,sizeof(freqDataR));
        memset(magDataL,0,sizeof(magDataL));
        memset(magDataR,0,sizeof(magDataR));
        ApplyFFT256(48000,pbufferL,0,freqDataL,magDataL);
        ApplyFFT256(48000,pbufferR,0,freqDataR,magDataR);

        /*int j;
		for(j = 0;j < 3 ;j ++)
        {
            ALOGD("freqDataL[%d]:%d,magDataL[%d]:%d",j,freqDataL[j],j,magDataL[j]);
            ALOGD("freqDataR[%d]:%d,magDataR[%d]:%d",j,freqDataR[j],j,magDataR[j]);
        }*/
	       
	    if (((freqDataL[0] <= highFreq && freqDataL[0] >= lowFreq) && (magDataL[0] <= magUpper && magDataL[0] >= magLower))&&((freqDataR[0] <= highFreq && freqDataR[0] >= lowFreq) && (magDataR[0] <= magUpper && magDataR[0] >= magLower)))
		{
		    checkCnt ++;	
            if(checkCnt >= 5)
		    {
		        sprintf(hds->info + strlen(hds->info),"Check freq pass.\n");
	            ALOGD(" @ info : %s",hds->info);	
		        break;
		    }
	     }
	     else
		    checkCnt = 0;
	    
       if (hds->exit_thd){
	       break;
	     }
      }

      return_data.vibrator.freq = freqDataL[0];
      return_data.vibrator.ampl = magDataL[0];
	  
      ALOGD(TAG "VIBRATOR FFT: FreqL = %d, FreqR = %d, AmpL = %d, AmpR = %d", freqDataL[0], freqDataR[0], magDataL[0], magDataR[0]);

      ALOGD(TAG "%s: Stop", __FUNCTION__);
      pthread_exit(NULL); // thread exit
      return NULL;
}

int vibrator_autotest_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct mVibrator *mc = (struct mVibrator *)priv;
    struct textview *tv;
    struct itemview *iv;

    ALOGD(TAG "--------mAudio_receiver_entry-----------------------\n" );
    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, "", COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);
    
    // init Audio
    Common_Audio_init();
    mc->exit_thd = false;
    vibrator_test_exit = false;

    // ui start
    if (!mc->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            ALOGD(TAG "No memory");
            return -1;
        }
        mc->iv = iv;
    }
    
    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, items_auto, 0);
    iv->set_text(iv, &mc->text);
	iv->start_menu(iv,0);
    iv->redraw(iv);

    memset(mc->info, 0, sizeof(mc->info) / sizeof(*(mc->info)));
    mc->i4Playtime = 5*1000;//ms         //read_preferred_ringtone_time() = 5
    mc->recordDevice = WIRED_HEADSET;
    pthread_create(&mc->hHeadsetThread, NULL, update_vibrator_thread, priv);
    pthread_create(&mc->hRecordThread, NULL, Audio_Record_thread, priv);
    
    int    play_time = mc->i4Playtime;
    mc->mod->test_result = FTM_TEST_FAIL;

    int i;
    for(i = 0; i < 100 ; i ++)
    {
      //ALOGD("check mc info:%d",i);
      if (strstr(mc->info, "Check freq pass")) 
      {      
          mc->mod->test_result = FTM_TEST_PASS;
          ALOGD("Check freq pass");
          break;
      }
      usleep(play_time * 10);
    }

    if(mc->mod->test_result == FTM_TEST_FAIL)
       ALOGD("Check freq fail");

    if(mc->mod->test_result == FTM_TEST_PASS)
        usleep(2000000);

    mc->exit_thd = true;
    vibrator_test_exit = true;
	
    pthread_join(mc->hRecordThread, NULL);
    pthread_join(mc->hHeadsetThread, NULL);
    Common_Audio_deinit();
    
    LOGD(TAG "%s: End\n", __FUNCTION__);
    return 0;
}
#endif

int vibrator_entry(struct ftm_param *param, void *priv)
{
	int chosen;
	bool exit = false;
	struct itemview *iv;
	text_t    title;
	char* vibr_time = NULL;
	struct ftm_module *mod = (struct ftm_module *)priv;

	LOGD("%s\n", __FUNCTION__);

	vibrator_test_exit = false;

	iv = ui_new_itemview();
	if (!iv) {
		LOGD("No memory");
		return -1;
	}
        init_text(&title, param->name, COLOR_YELLOW);

	iv->set_title(iv, &title);
	iv->set_items(iv, items, 0);

	vibr_time = ftm_get_prop("Vibrator_Last_Time");
	LOGD("%s: get vibrator last time=%s!\n", __FUNCTION__, vibr_time);
	if(vibr_time != NULL)
	{
		vibrator_time = (uint32_t)atoi(vibr_time);
		LOGD("%s: get vibrator last time=%d!\n", __FUNCTION__, vibrator_time);
	}
	else
	{
		LOGD("%s: get vibrator last time fail!\n", __FUNCTION__);
	}
//#if defined(MTK_VIBSPK_SUPPORT)
    if (IsFTMSupportVibSpk())
     Common_Audio_init();
//#endif

	pthread_create(&vibrator_thread, NULL, update_vibrator_thread, priv);
	
	do {
		if(status == 0){
		chosen = iv->run(iv, &exit);
		switch (chosen) {
		case ITEM_PASS:
		case ITEM_FAIL:
			if (chosen == ITEM_PASS) {
				mod->test_result = FTM_TEST_PASS;
			} else if (chosen == ITEM_FAIL) {
				mod->test_result = FTM_TEST_FAIL;
			}
			exit = true;
			break;
		}

		if (exit) {
			vibrator_test_exit = true;
			break;
		}
		}
		else{
			iv->start_menu(iv, 0);
			iv->redraw(iv);
			if(vibrator_test_exit)
				break;
		}
	} while (1);
	pthread_join(vibrator_thread, NULL);

//    #if defined(MTK_VIBSPK_SUPPORT)
    if (IsFTMSupportVibSpk())
    Common_Audio_deinit();
//    #endif

	return 0;
}

int vibrator_init(void)
{
	int ret = 0;
    struct ftm_module *mod, *mod_autotest;
    struct mVibrator *mvibrator_autotest;

	LOGD("%s\n", __FUNCTION__);

	mod = ftm_alloc(ITEM_VIBRATOR, sizeof(struct ftm_module));
	if (!mod)
		return -ENOMEM;

	ret = ftm_register(mod, vibrator_entry, (void*)mod);

#ifdef VIBRATOR_AUTOTEST
    mod_autotest = ftm_alloc(ITEM_VIBRATOR_PHONE, sizeof(struct mVibrator));
    mvibrator_autotest = mod_to_mVibrator(mod_autotest);
    mvibrator_autotest->mod = mod_autotest;
    if (!mod_autotest)
        return -ENOMEM;
    ret = ftm_register(mod_autotest, vibrator_autotest_entry, (void*)mvibrator_autotest);
#endif

	return ret;
}

#endif // FEATURE_FTM_VIBRATOR

#ifdef __cplusplus
};
#endif

