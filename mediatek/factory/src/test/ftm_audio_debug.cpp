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
#include <sys/time.h>


#include "ftm_audio_Common.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FEATURE_FTM_AUDIO

#include <AudioMTKHeadsetMessager.h>

#define mod_to_mAudio(p)     (struct mAudio*)((char*)(p) + sizeof(struct ftm_module))
#define TAG   "[Audio] "
#define HEADSET_STATE_PATH "/sys/class/switch/h2w/state"
#define ACCDET_STATE_PATH "/sys/class/switch/h2w/state"

#define GET_HEADSET_STATE

#define MAX_FILE_NAME_SIZE (100)
#define TEXT_LENGTH (1024)

#define WAVE_PLAY_MAX_TIME  (5000)   //in ms.
#define WAVE_PLAY_SLEEP_TIME (100)

//#define WAVE_PLAYBACK //use Audio_Wave_Playabck_thread for Ringtone/Receiver test

#define BUF_LEN 1
static char rbuf[BUF_LEN] = {'\0'};
static char wbuf[BUF_LEN] = {'1'};
static char wbuf1[BUF_LEN] = {'2'};

// Global variable
static int HeadsetFd = 0;
static int g_loopback_item  = 0;
static int g_mic_change     = 0;
static int g_prev_mic_state = 0;

static int b_mic1_loopback = false;
static int b_mic2_loopback = false;
static int print_len1 = 0;
static int print_len2 = 0;
static int b_incomplete_flag = false;
extern sp_ata_data return_data;
enum
{
    ITEM_MIC1,
    ITEM_MIC2,
    ITEM_RINGTONE,
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t audio_items_loopback[] =
{
#ifdef MTK_DUAL_MIC_SUPPORT
    {ITEM_MIC1, uistr_info_audio_loopback_dualmic_mi1, 0, 0},
    {ITEM_MIC2, uistr_info_audio_loopback_dualmic_mi2, 0, 0},
#endif
    {ITEM_PASS, uistr_pass, 0, 0},
    {ITEM_FAIL, uistr_fail, 0, 0},
    { -1, NULL, 0, 0},
};

static item_t audio_items[] =
{
    {ITEM_PASS, uistr_pass, 0, 0},
    {ITEM_FAIL, uistr_fail, 0, 0},
    { -1, NULL, 0, 0},
};

static item_t receiver_items[] =
{
    {ITEM_RINGTONE, uistr_info_audio_ringtone, 0, 0},
#ifdef MTK_DUAL_MIC_SUPPORT
    {ITEM_MIC1, uistr_info_audio_loopback_dualmic_mi1, 0, 0},
    {ITEM_MIC2, uistr_info_audio_loopback_dualmic_mi2, 0, 0},
#else
    {ITEM_MIC1, uistr_info_audio_loopback_dualmic_mic, 0, 0},
#endif
    {ITEM_PASS, uistr_pass, 0, 0},
    {ITEM_FAIL, uistr_fail, 0, 0},
    { -1, NULL, 0, 0},
};

struct mAudio
{
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
    char  info[TEXT_LENGTH];
    char  file_name[MAX_FILE_NAME_SIZE];
    int   i4OutputType;
    int   i4Playtime;
    int  recordDevice;
    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
};

static int audio_key_handler(int key, void *priv)
{
    int handled = 0, exit = 0;
    struct mAudio *mc = (struct mAudio *)priv;
    struct textview *tv = &mc->tv;
    struct ftm_module *fm = mc->mod;

    switch (key)
    {
        case UI_KEY_RIGHT:
            exit = 1;
            break;
        case UI_KEY_LEFT:
            fm->test_result = FTM_TEST_FAIL;
            exit = 1;
            break;
        case UI_KEY_CENTER:
            fm->test_result = FTM_TEST_PASS;
            exit = 1;
            break;
        default:
            handled = -1;
            break;
    }
    if (exit)
    {
        ALOGD(TAG "%s: Exit thead\n", __FUNCTION__);
        tv->exit(tv);
    }
    return handled;
}

static int read_preferred_receiver_time(void)
{
    int time = 0;
    unsigned int i = 0;
    char *pTime = NULL;
    char uName[64];

    memset(uName, 0, sizeof(uName));
    sprintf(uName, "Audio.Receiver");
    pTime = ftm_get_prop(uName);
    if (pTime != NULL)
    {
        time = (int)atoi(pTime);
        ALOGD("preferred_receiver_time: %d sec\n", time);
    }
    else
    {
        ALOGD("preferred_receiver_time can't get\n");
    }
    return time;
}

static void *Audio_Receiver_Playabck_thread(void *mPtr)
{
    struct mAudio *hds  = (struct mAudio *)mPtr;
    struct itemview *iv = hds->iv;
    int    play_time    = 0;
    ALOGD(TAG "%s: Start\n", __FUNCTION__);
    play_time = read_preferred_receiver_time();
    RecieverTest(1);
    if (play_time > 0)
    {
        usleep(play_time * 1000 * 1000);
        RecieverTest(0);
    }
    while (1)
    {
        char *ptr;
        usleep(100000);
        if (hds->exit_thd)
        {
            break;
        }
        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
    }
    if (play_time <= 0)
    {
        RecieverTest(0);
    }
    ALOGD(TAG "%s: Audio_Headset_detect_thread Exit \n", __FUNCTION__);
    pthread_exit(NULL); // thread exit
    return NULL;
}
void mAudio_receiver_playtone(struct mAudio *mc, void *priv)
{
    pthread_create(&mc->hHeadsetThread, NULL, Audio_Receiver_Playabck_thread, priv);
}

void mAudio_receiver_stoptone(struct mAudio *mc)
{
    pthread_join(mc->hHeadsetThread, NULL);
}

//Receiver test
int mAudio_receiver_debug_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct mAudio *mc = (struct mAudio *)priv;
    struct textview *tv;
    struct itemview *iv;
    int privChosen = -1;

    ALOGD(TAG "--------mAudio_receiver_entry-----------------------\n");
    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, "", COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

    // init Audio
    Common_Audio_init();
    mc->exit_thd = false;

    // ui start
    if (!mc->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            ALOGD(TAG "No memory");
            return -1;
        }
        mc->iv = iv;
    }
    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, receiver_items, 0);
    iv->set_text(iv, &mc->text);
    do
    {
        chosen = iv->run(iv, &exit);
        switch (chosen)
        {
            case ITEM_RINGTONE:
                if (privChosen == ITEM_RINGTONE)
                {
                    break;
                }
                else if (privChosen == ITEM_MIC1)
                {
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC1_OFF);  // disable Receiver MIC1 loopback
                }
                else if (privChosen == ITEM_MIC2)
                {
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC2_OFF);  // disable Receiver MIC2 loopback
                }
                mAudio_receiver_playtone(mc, priv);
                privChosen = ITEM_RINGTONE;
                exit = false;
                break;
            case ITEM_MIC1:
                if (privChosen == ITEM_MIC1)
                {
                    break;
                }
                else if (privChosen == ITEM_MIC2)
                {
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC2_OFF);  // disable Receiver MIC2 loopback
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC1_ON);  // enable Receiver MIC1 loopback
                }
                else if (privChosen == ITEM_RINGTONE)
                {
                    mc->exit_thd = true;
                    mAudio_receiver_stoptone(mc);
                    mc->exit_thd = false;
                }
                usleep(3000);
                PhoneMic_Receiver_Loopback(MIC1_ON);  // enable Receiver MIC1 loopback
                privChosen = ITEM_MIC1;
                exit = false;
                break;
            case ITEM_MIC2:
                if (privChosen == ITEM_MIC2)
                {
                    break;
                }
                else if (privChosen == ITEM_MIC1)
                {
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC1_OFF);  // disable Receiver MIC1 loopback
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC2_ON);  // enable Receiver MIC2 loopback
                }
                else if (privChosen == ITEM_RINGTONE)
                {
                    mc->exit_thd = true;
                    mAudio_receiver_stoptone(mc);
                    mc->exit_thd = false;
                }
                usleep(3000);
                PhoneMic_Receiver_Loopback(MIC2_ON);  // enable Receiver MIC2 loopback
                privChosen = ITEM_MIC2;
                exit = false;
                break;
            case ITEM_PASS:
            case ITEM_FAIL:
                if (chosen == ITEM_PASS)
                {
                    mc->mod->test_result = FTM_TEST_PASS;
                }
                else if (chosen == ITEM_FAIL)
                {
                    mc->mod->test_result = FTM_TEST_FAIL;
                }
                exit = true;
                break;
        }
        if (exit)
        {
            ALOGD("mAudio_receiver_entry set exit_thd = true\n");
            mc->exit_thd = true;
            break;
        }
    }
    while (1);

    if (privChosen == ITEM_RINGTONE)
    {
        mAudio_receiver_stoptone(mc);
    }
    else if (privChosen == ITEM_MIC1)
    {
        usleep(3000);
        PhoneMic_Receiver_Loopback(MIC1_OFF);  // disable Receiver MIC1 loopback
    }
    else if (privChosen == ITEM_MIC2)
    {
        usleep(3000);
        PhoneMic_Receiver_Loopback(MIC2_OFF);  // disable Receiver MIC2 loopback
    }

    Common_Audio_deinit();
    return 0;
}

int audio_debug_init(void)
{
    int ret = 0;
    struct ftm_module *modLoudspk, *modReceiver,
            *modReceiverLoopback,
            *modPMic_Headset_Loopback,
            *modPMic_SPK_Loopback,
            *modHMic_SPK_Loopback,
            *modWavePlayback,
            *modAcoustic_Loopback;

    struct mAudio *maudioSpk, *maudioReveiver,
            *maudioReveiverLoopback,
            *maudioPMic_Headset_Loopback,
            *maudioPMic_SPK_Loopback,
            *maudioHMic_SPK_Loopback,
            *maudioWavePlayback,
            *maudioAcousticLoopback;

    ALOGD(TAG "%s\n", __FUNCTION__);
    ALOGD(TAG "-------Audio_init------------------\n");

    modReceiver = ftm_alloc(ITEM_RECEIVER_DEBUG, sizeof(struct mAudio));
    maudioReveiver = mod_to_mAudio(modReceiver);
    maudioReveiver->mod = modReceiver;
    if (!modReceiver)
{
        return -ENOMEM;
    }
    ret = ftm_db_register(modReceiver, mAudio_receiver_debug_entry, (void *)maudioReveiver);

    return ret;
}
#endif

#ifdef __cplusplus
};
#endif


