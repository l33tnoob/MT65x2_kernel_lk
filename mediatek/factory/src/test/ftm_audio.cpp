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
#include "DIF_FFT.h"
#include "Audio_FFT_Types.h"
#include "Audio_FFT.h"
#endif

#include <cutils/xlog.h>
#undef LOG_TAG
#define LOG_TAG "FTM_AUDIO"
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

bool bMicbias_exit = false;

enum
{
    ITEM_MIC1,
    ITEM_MIC2,
#ifdef FEATURE_FTM_ACSLB
    ITEM_DUALMIC_DMNR,
    ITEM_DUALMIC_NO_DMNR,
#endif
    ITEM_RINGTONE,
    ITEM_MICBIAS_ON,
    ITEM_MICBIAS_OFF,
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t audio_items_loopback[] =
{
#ifdef MTK_DUAL_MIC_SUPPORT
    {ITEM_MIC1, uistr_info_audio_loopback_dualmic_mi1, 0,0},
    {ITEM_MIC2, uistr_info_audio_loopback_dualmic_mi2, 0,0},
#endif
    {ITEM_PASS, uistr_pass, 0,0},
    {ITEM_FAIL, uistr_fail, 0,0},
    { -1, NULL, 0},
};
#ifdef FEATURE_FTM_ACSLB
static item_t audio_items_acoustic_loopback[] =
{
#ifdef MTK_DUAL_MIC_SUPPORT
    {ITEM_DUALMIC_DMNR, uistr_info_audio_acoustic_loopback_DMNR, 0,0},
#endif
    {ITEM_DUALMIC_NO_DMNR, uistr_info_audio_acoustic_loopback_without_DMNR, 0,0},
    {ITEM_PASS,  uistr_pass, 0,0},
    {ITEM_FAIL,  uistr_fail, 0,0},
    { -1, NULL, 0,0},
};
#endif

static item_t audio_items_micbias[] = {
    {ITEM_MICBIAS_ON, uistr_info_audio_micbias_on, 0,0},
    {ITEM_MICBIAS_OFF, uistr_info_audio_micbias_off, 0,0},
    {ITEM_PASS,      uistr_pass, 0,0},
    {ITEM_FAIL,      uistr_fail, 0,0},
    {-1, NULL, 0},
};

static item_t audio_items[] =
{
    {ITEM_PASS, uistr_pass, 0,0},
    {ITEM_FAIL, uistr_fail, 0,0},
    { -1, NULL, 0,0},
};

static item_t audio_items_auto[] =
{
    { -1, NULL, 0,0},
};

static item_t receiver_items[] =
{
    {ITEM_RINGTONE, uistr_info_audio_ringtone, 0,0},
#ifdef MTK_DUAL_MIC_SUPPORT
    {ITEM_MIC1, uistr_info_audio_loopback_dualmic_mi1, 0,0},
    {ITEM_MIC2, uistr_info_audio_loopback_dualmic_mi2, 0,0},
#else
    {ITEM_MIC1, uistr_info_audio_loopback_dualmic_mic, 0,0},
#endif
    {ITEM_PASS, uistr_pass, 0,0},
    {ITEM_FAIL, uistr_fail, 0,0},
    { -1, NULL, 0,0},
};
// use for wave playback
static char FileListNamesdcard[] = "/sdcard/factory.ini";
static char FileListName[]           = "/system/etc/factory.ini";
static char FileStartString_WavePlayback[]      = "//AudioWavePlayFile";
static char FileStartString_LoudspkPlayback[]      = "//AudioRingtonePlayFile";
static char FileStartString_ReceiverPlayback[]      = "//AudioReceiverPlayFile";

static WaveHdr WaveHeader;


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

#ifdef GET_HEADSET_STATE
static int init_Headset(void)
{
#define ACCDET_IOC_MAGIC 'A'
#define ACCDET_INIT      _IO(ACCDET_IOC_MAGIC,0)
#define ACCDET_PATH      "/dev/accdet"

    int fd = open(ACCDET_PATH, O_RDONLY);
    if (fd < 0)
    {
        ALOGD(TAG "open %s failed, fd = %d", ACCDET_PATH, fd);
        return -1;
    }
    if (ioctl(fd, ACCDET_INIT, 0) < 0)
    {
        ALOGE(TAG "ioctl ACCDET_INIT failed\n");
        goto out;
    }
out:
    if (fd)
    {
        close(fd);
    }
    return 0;
}

static int get_headset_info(void)
{
    int ret = 0;
    int fd = -1;
    char rbuf[BUF_LEN] = {'\0'};
    char wbuf[BUF_LEN] = {'1'};
    char wbuf1[BUF_LEN] = {'2'};
    fd = open(HEADSET_STATE_PATH, O_RDONLY, 0);
    if (fd < 0)
    {
        ALOGD(TAG "Can't open %s\n", HEADSET_STATE_PATH);
        ret = -1;
        goto out;
    }
    if (read(fd, rbuf, BUF_LEN) == -1)
    {
        ALOGD(TAG "Can't read %s\n", HEADSET_STATE_PATH);
        ret = -2;
        goto out;
    }
    if (!strncmp(wbuf, rbuf, BUF_LEN))
    {
        ret = 1;
        goto out;
    }
    else if (!strncmp(wbuf1, rbuf, BUF_LEN))
    {
        ret = 2;
    }
    else
    {
        ret = 0;
    }
out:
    close(fd);
    return ret;
}
#endif

static int read_preferred_ringtone_time(void)
{
    int time = 0;
    char *pTime = NULL;
    char uName[64];

    memset(uName, 0, sizeof(uName));
    sprintf(uName, "Audio.Ringtone");
    pTime = ftm_get_prop(uName);
    if (pTime != NULL)
    {
        time = (int)atoi(pTime);
        ALOGD("preferred_ringtone_time: %d sec\n", time);
    }
    else
    {
        ALOGD("preferred_ringtone_time can't get\n");
    }
    return time;
}

static int read_preferred_ringtone_freq(void)
{
    int freq = 0;
    char *pFreq = NULL;
    char uName[64];

    memset(uName, 0, sizeof(uName));
    sprintf(uName, "Freq.Ringtone");
    pFreq = ftm_get_prop(uName);
    if (pFreq != NULL)
    {
        freq = (int)atoi(pFreq);
        ALOGD("preferred_ringtone_freq: %d hz\n", freq);
    }
    else
    {
        ALOGD("preferred_ringtone_freq can't get\n");
    }
    return freq;
}

static void read_preferred_magnitude(int device, int *pUpper, int *pLower)
{
    char *pMagLower = NULL, *pMagUpper = NULL;
    char uMagLower[64], uMagUpper[64];

    *pUpper = 0;
    *pLower = 0;
    memset(uMagLower, 0, sizeof(uMagLower));
    memset(uMagUpper, 0, sizeof(uMagUpper));
    if (device == Output_LPK) //speaker
    {
        sprintf(uMagLower, "Lower.Magnitude.Speaker");
        sprintf(uMagUpper, "Upper.Magnitude.Speaker");
    }
    else if (device == Output_HS) //receiver
    {
        sprintf(uMagLower, "Lower.Magnitude.Receiver");
        sprintf(uMagUpper, "Upper.Magnitude.Receiver");
    }

    pMagLower = ftm_get_prop(uMagLower);
    pMagUpper = ftm_get_prop(uMagUpper);
    if (pMagLower != NULL && pMagUpper != NULL)
    {
        *pLower = (int)atoi(pMagLower);
        *pUpper = (int)atoi(pMagUpper);
        ALOGD("Lower.Magnitude:%d,Upper.Magnitude:%d\n", *pLower, *pUpper);
    }
    else
    {
        ALOGD("Lower/Upper.Magnitude can not get\n");
    }
}
static bool read_preferred_recorddump(void)
{
    char *pDump = NULL;
    char uName[64];

    memset(uName, 0, sizeof(uName));
    sprintf(uName, "Audio.Record.Dump");
    pDump = ftm_get_prop(uName);
    ALOGD("pDump:%s", pDump);

    if (pDump != NULL)
    {
        if (!strcmp(pDump, "1"))
        {
            ALOGD("Dump record data");
            return true;
        }
        else
        {
            ALOGD("No need to dump record data");
            return false;
        }
    }
    else
    {
        ALOGD("Dump record prop can't get");
        return false;
    }
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

static int Audio_headset_hook_info(struct mAudio *hds, char *info)
{
    int ret = 0;
    if (HeadsetFd)
    {
        ALOGV(TAG "GET_BUTTON_STATUS ");
        ret = ::ioctl(HeadsetFd, GET_BUTTON_STATUS, 0);
        ALOGV("Audio_headset_hook_info ret = %d", ret);
    }
    return ret;
}

static void Audio_headset_update_info(struct mAudio *hds, char *info)
{
    int fd = -1;
    int hookstatus = 0;
    char *ptr ;
    int OriginState = hds->avail;

    hds->Headset_mic = 0;
    fd = open(HEADSET_STATE_PATH, O_RDONLY, 0);
    if (fd == -1)
    {
        ALOGD(TAG "Can't open %s\n", HEADSET_STATE_PATH);
        hds->avail = false;
        goto EXIT;
    }
    if (read(fd, rbuf, BUF_LEN) == -1)
    {
        ALOGD(TAG "Can't read %s\n", HEADSET_STATE_PATH);
        hds->avail = false;
        goto EXIT;
    }

    if (!strncmp(wbuf, rbuf, BUF_LEN))
    {
        ALOGD(TAG "state == 1");
        hds->avail = true;
        hds->Headset_mic = 1;
        goto EXIT;
    }

    if (!strncmp(wbuf1, rbuf, BUF_LEN))
    {
        ALOGD(TAG "state == 2");
        hds->avail = true;
    }
    else
    {
        ALOGV(TAG "state == %s", rbuf);
        hds->avail = false;
    }

EXIT:
    if (OriginState != hds->avail)
    {
        ALOGD(TAG "OriginState(%d) != hds->avail(%d)", OriginState, hds->avail);
        hds->Headset_change = true;
    }
    else
    {
        hds->Headset_change = false;
    }

    int len1, len2, len3, len4;

    /* preare text view info */
    ptr = info;
    len1 = sprintf(ptr, uistr_info_audio_loopback_note);
    ptr += len1;
    len3 = sprintf(ptr, uistr_info_audio_headset_avail, hds->avail ? uistr_info_audio_yes : uistr_info_audio_no);
    ptr += len3;
    len4 = sprintf(ptr, uistr_info_audio_headset_mic_avail, hds->Headset_mic ? uistr_info_audio_yes : uistr_info_audio_no);
    ptr += len4;
    len2 = 0;
    print_len1 = len1 + len2 + len3 + len4;

#ifdef HEADSET_BUTTON_DETECTION
    hookstatus = Audio_headset_hook_info(hds, hds->info); // get hook information
    if (hds->avail)
    {
        print_len2 = sprintf(ptr, uistr_info_audio_headset_Button, hookstatus ? uistr_info_audio_press : uistr_info_audio_release);
        ptr += print_len2;
    }
#endif

    if (b_incomplete_flag == true)
    {
        if ((b_mic1_loopback == false) || (b_mic2_loopback == false))
        {
            ptr += sprintf(ptr, "[%s] \n\n", uistr_info_audio_loopback_complete);
        }
    }

    close(fd);
    return;
}

static void *Audio_Headset_detect_thread(void *mPtr)
{
    struct mAudio *hds = (struct mAudio *)mPtr;
    struct itemview *iv = hds->iv;

    ALOGD(TAG "%s: Start\n", __FUNCTION__);

    // open headset device
    HeadsetFd = open(HEADSET_PATH, O_RDONLY);
    if (HeadsetFd < 0)
    {
        ALOGE("open %s error fd = %d", HEADSET_PATH, HeadsetFd);
        return 0;
    }

    // set headset state to 1 , enable hook
    ::ioctl(HeadsetFd, SET_CALL_STATE, 1);

    // 1st turn on
    Audio_headset_update_info(hds, hds->info);  // get headset information
    if (hds->avail)
    {
        HeadsetMic_EarphoneLR_Loopback(MIC1_ON, hds->Headset_mic);
        hds->Headset_change = false;
    }
    else
    {
        PhoneMic_Receiver_Loopback(MIC1_ON);
    }

    while (1)
    {
        char *ptr;
        usleep(100000);
        if (hds->exit_thd)
        {
            break;
        }

        Audio_headset_update_info(hds, hds->info);  // get headset information

        if (hds->Headset_change)    // Headset device change
        {
            if (hds->avail == true)    // use headset
            {
                ALOGD(TAG  " --------Audio_Headset_detect_thread  : Headset plug-in (%d)\n", g_loopback_item);
                if (g_loopback_item == 1)
                {
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC1_OFF);  // disable Receiver MIC1 loopback
                    usleep(3000);
                    HeadsetMic_EarphoneLR_Loopback(MIC1_ON, hds->Headset_mic);  // enable Earphone MIC1 loopback
                }
                else if (g_loopback_item == 2)
                {
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC2_OFF);  // disable MIC2 loopback
                    usleep(3000);
                    HeadsetMic_EarphoneLR_Loopback(MIC2_ON, hds->Headset_mic);  // enable Earphone MIC2 loopback
                }
                g_mic_change = 0;
                hds->Headset_change = false;
            }
            else    // use handset
            {
                ALOGD(TAG " --------Audio_Headset_detect_thread  : Headset plug-out (%d)\n", g_loopback_item);
                if (g_loopback_item == 1)
                {
                    usleep(3000);
                    HeadsetMic_EarphoneLR_Loopback(MIC1_OFF, hds->Headset_mic);  // disable Earphone MIC1 loopback
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC1_ON);  // enable Receiver MIC1 loopback
                }
                else if (g_loopback_item == 2)
                {
                    usleep(3000);
                    HeadsetMic_EarphoneLR_Loopback(MIC2_OFF, hds->Headset_mic);  // disable Earphone MIC2 loopback
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC2_ON);  // enable Receiver MIC2 loopback
                }
                g_mic_change = 0;
                hds->Headset_change = false;
            }
        }
        else if (g_mic_change == 1)    // mic1 <-> mic2
        {
            if (hds->avail == false)    // without Earphone plug in/out
            {
                ALOGD(TAG " --------Without Earphone, g_mic_change = 1 \n");
                g_mic_change = 0;
                if (g_loopback_item == 1)
                {
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC2_OFF);  // disable Earphone MIC2 loopback
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC1_ON);  // enable Receiver MIC1 loopback
                }
                else if (g_loopback_item == 2)
                {
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC1_OFF);  // disable Earphone MIC1 loopback
                    usleep(3000);
                    PhoneMic_Receiver_Loopback(MIC2_ON);  // enable Receiver MIC2 loopback
                }
            }
            else    // with Earphone plug in/out
            {
                ALOGD(TAG " --------With Earphone, g_mic_change = 1 \n");
                g_mic_change = 0;
                if (g_loopback_item == 1)
                {
                    usleep(3000);
                    HeadsetMic_EarphoneLR_Loopback(MIC2_OFF, hds->Headset_mic);  // disable Earphone MIC2 loopback
                    usleep(3000);
                    HeadsetMic_EarphoneLR_Loopback(MIC1_ON, hds->Headset_mic);  // enable Receiver MIC1 loopback
                }
                else if (g_loopback_item == 2)
                {
                    usleep(3000);
                    HeadsetMic_EarphoneLR_Loopback(MIC1_OFF, hds->Headset_mic);  // disable Earphone MIC1 loopback
                    usleep(3000);
                    HeadsetMic_EarphoneLR_Loopback(MIC2_ON, hds->Headset_mic);  // enable Receiver MIC2 loopback
                }
            }
        }

        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
    }

    ALOGD(TAG "%s: Audio_Headset_detect_thread Exit (%d) \n", __FUNCTION__, g_loopback_item);

    // set headset state to 0 , lose headset device.
    ::ioctl(HeadsetFd, SET_CALL_STATE, 0);

    close(HeadsetFd);
    HeadsetFd = 0;

    pthread_exit(NULL); // thread exit
    return NULL;
}


#ifdef FEATURE_FTM_ACSLB
static void *PhoneMic_Receiver_Headset_Acoustic_Loopback(void *mPtr)
{
    struct mAudio *hds = (struct mAudio *)mPtr;
    struct itemview *iv = hds->iv;
    int Acoustic_Status = 0;
    int Acoustic_Type = DUAL_MIC_WITHOUT_DMNR_ACS_ON;

    ALOGD(TAG "%s: Start\n", __FUNCTION__);

    // open headset device
    HeadsetFd = open(HEADSET_PATH, O_RDONLY);
    if (HeadsetFd < 0)
    {
        ALOGE("open %s error fd = %d", HEADSET_PATH, HeadsetFd);
        return 0;
    }

    //set headset state to 1 , enable hook
    ::ioctl(HeadsetFd, SET_CALL_STATE, 1);
#ifdef MTK_DUAL_MIC_SUPPORT
    Acoustic_Type = DUAL_MIC_WITH_DMNR_ACS_ON;
#endif
    Audio_headset_update_info(hds, hds->info);  // get headset information
    PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Type, &Acoustic_Status, hds->avail);

    while (1)
    {
        char *ptr;
        usleep(100000);
        if (hds->exit_thd)
        {
            break;
        }

        pthread_mutex_lock(&hds->mHeadsetMutex);
        Audio_headset_update_info(hds, hds->info);  // get headset information

        if (hds->Headset_change)
        {
            PhoneMic_Receiver_Acoustic_Loopback(ACOUSTIC_STATUS, &Acoustic_Status, hds->avail);
            if (!(Acoustic_Status & 0x1))
            {
                hds->Headset_change = false;
                pthread_mutex_unlock(&hds->mHeadsetMutex);
                continue;
            }
        }

        if (hds->avail && hds->Headset_change)
        {
            ALOGD(TAG  " --------Audio_Headset_detect_thread  : Headset plug-in (%d)\n", hds->avail);
            PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Status - 1, &Acoustic_Status, hds->avail);
            usleep(50000);
            PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Status, &Acoustic_Status, hds->avail);
            hds->Headset_change = false;
        }
        else if (hds->Headset_change)
        {
            ALOGD(TAG " --------Audio_Headset_detect_thread  : Headset plug-out (%d)\n", hds->avail);
            PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Status - 1, &Acoustic_Status, hds->avail);
            usleep(50000);
            PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Status, &Acoustic_Status, hds->avail);
            hds->Headset_change = false;
        }

        pthread_mutex_unlock(&hds->mHeadsetMutex);
        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
    }

    ALOGD(TAG "%s: Audio_Headset_detect_thread Exit (%d) \n", __FUNCTION__, hds->avail);

    // set headset state to 0 , lose headset device.
    ::ioctl(HeadsetFd, SET_CALL_STATE, 0);

    close(HeadsetFd);
    HeadsetFd = -1;

    pthread_exit(NULL); // thread exit
    return NULL;
}
#endif

#ifndef FEATURE_FTM_HEADSET
static void *Audio_Mic_change_thread(void *mPtr)
{
    struct mAudio *hds = (struct mAudio *)mPtr;
    struct itemview *iv = hds->iv;

    ALOGD(TAG "%s: Start\n", __FUNCTION__);

    // default use mic1
    PhoneMic_Receiver_Loopback(MIC1_ON);

    while (1)
    {
        char *ptr;
        usleep(100000);
        if (hds->exit_thd)
        {
            break;
        }
        // without Earphone plug in/out
        if (g_mic_change == 1)
        {
            ALOGD(TAG " --------Without Earphone, g_mic_change = 1 \n");
            g_mic_change = 0;
            if (g_loopback_item == 1)
            {
                usleep(3000);
                PhoneMic_Receiver_Loopback(MIC2_OFF);  // disable Earphone MIC2 loopback
                usleep(3000);
                PhoneMic_Receiver_Loopback(MIC1_ON);  // enable Receiver MIC1 loopback
            }
            else if (g_loopback_item == 2)
            {
                usleep(3000);
                PhoneMic_Receiver_Loopback(MIC1_OFF);  // disable Earphone MIC1 loopback
                usleep(3000);
                PhoneMic_Receiver_Loopback(MIC2_ON);  // enable Receiver MIC2 loopback
            }
        }
        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
    }

    ALOGD(TAG "%s: Audio_Headset_detect_thread Exit (%d) \n", __FUNCTION__, g_loopback_item);

    pthread_exit(NULL); // thread exit
    return NULL;
}
#endif
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

static void *Audio_LoudSpk_Playabck_thread(void *mPtr)
{
    struct mAudio *hds  = (struct mAudio *)mPtr;
    struct itemview *iv = hds->iv;
    //int    play_time    = 0;
    ALOGD(TAG "%s: Start\n", __FUNCTION__);
    //play_time = read_preferred_ringtone_time();
    LouderSPKTest(1, 1);
    while (1)
    {
        usleep(100000);
        if (hds->exit_thd)
        {
            break;
        }
        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
    }

    LouderSPKTest(0, 0);
    ALOGD(TAG "%s: Audio_LoudSpk_Playabck_thread Exit \n", __FUNCTION__);
    pthread_exit(NULL); // thread exit
    return NULL;
}

static void *Audio_Record_thread(void *mPtr)
{
    struct mAudio *hds  = (struct mAudio *)mPtr;
    ALOGD(TAG "%s: Start", __FUNCTION__);
    usleep(100000);
    bool dumpFlag = read_preferred_recorddump();
    //    dumpFlag=true;//for test
    int magLower = 0, magUpper = 0;
    read_preferred_magnitude(hds->i4OutputType, &magUpper, &magLower);
    int freqOfRingtone = read_preferred_ringtone_freq();
    int lowFreq = freqOfRingtone * (1 - 0.05);
    int highFreq = freqOfRingtone * (1 + 0.05);
    short pbuffer[8192] = {0};
    short pbufferL[4096] = {0};
    short pbufferR[4096] = {0};
    unsigned int freqDataL[3] = {0}, magDataL[3] = {0};
    unsigned int freqDataR[3] = {0}, magDataR[3] = {0};
    int checkCnt = 0;

    if (hds->i4OutputType == Output_LPK) //speaker
    {
        return_data.speaker.freqL = 0;
        return_data.speaker.freqR = 0;
        return_data.speaker.amplL = 0;
        return_data.speaker.amplR = 0;
    }
    else if (hds->i4OutputType == Output_HS)
    {
        return_data.receiver.freqL = 0;
        return_data.receiver.freqR = 0;
        return_data.receiver.amplL = 0;
        return_data.receiver.amplR = 0;
    }

    recordInit(hds->recordDevice);
    while (1)
    {
        memset(pbuffer, 0, sizeof(pbuffer));
        memset(pbufferL, 0, sizeof(pbufferL));
        memset(pbufferR, 0, sizeof(pbufferR));

        int readSize  = readRecordData(pbuffer, 8192 * 2);
        for (int i = 0 ; i < 4096 ; i++)
        {
            pbufferL[i] = pbuffer[2 * i];
            pbufferR[i] = pbuffer[2 * i + 1];
        }

        if (dumpFlag)
        {
            char filenameL[] = "/data/record_dataL.pcm";
            char filenameR[] = "/data/record_dataR.pcm";
            FILE *fpL = fopen(filenameL, "wb+");
            FILE *fpR = fopen(filenameR, "wb+");

            if (fpL != NULL)
            {
                fwrite(pbufferL, readSize / 2, 1, fpL);
                fclose(fpL);
            }

            if (fpR != NULL)
            {
                fwrite(pbufferR, readSize / 2, 1, fpR);
                fclose(fpR);
            }
        }
        memset(freqDataL, 0, sizeof(freqDataL));
        memset(freqDataR, 0, sizeof(freqDataR));
        memset(magDataL, 0, sizeof(magDataL));
        memset(magDataR, 0, sizeof(magDataR));
        ApplyFFT256(48000, pbufferL, 0, freqDataL, magDataL);
        ApplyFFT256(48000, pbufferR, 0, freqDataR, magDataR);
        for (int i = 0; i < 3 ; i ++)
        {
            SXLOGV("%d.freqDataL[%d]:%d,magDataL[%d]:%d", i,i, freqDataL[i], i, magDataL[i]);
            SXLOGV("%d.freqDataR[%d]:%d,magDataR[%d]:%d", i,i, freqDataR[i], i, magDataR[i]);
        }

		if (hds->i4OutputType == Output_HS)
		{
			if (((freqDataL[0] <= (unsigned int)highFreq && freqDataL[0] >= (unsigned int)lowFreq) && (magDataL[0] <= (unsigned int)magUpper && magDataL[0] >= (unsigned int)magLower)) || ((freqDataR[0] <= (unsigned int)highFreq && freqDataR[0] >= (unsigned int)lowFreq) && (magDataR[0] <= (unsigned int)magUpper && magDataR[0] >= (unsigned int)magLower)))
			{
				checkCnt ++;
				if (checkCnt >= 5)
				{
					sprintf(hds->info + strlen(hds->info), "Check freq pass.\n");
					ALOGD(" @ info : %s", hds->info);
					break;
				}
			}
			else
			{
				checkCnt = 0;
			}
		}
		else
		{
			if (((freqDataL[0] <= (unsigned int)highFreq && freqDataL[0] >= (unsigned int)lowFreq) && (magDataL[0] <= (unsigned int)magUpper && magDataL[0] >= (unsigned int)magLower)) && ((freqDataR[0] <= (unsigned int)highFreq && freqDataR[0] >= (unsigned int)lowFreq) && (magDataR[0] <= (unsigned int)magUpper && magDataR[0] >= (unsigned int)magLower)))
			{
				checkCnt ++;
				if (checkCnt >= 5)
				{
					sprintf(hds->info + strlen(hds->info), "Check freq pass.\n");
					ALOGD(" @ info : %s", hds->info);
					break;
				}
			}
			else
			{
				checkCnt = 0;
			}
		}

        if (hds->exit_thd)
        {
            break;
        }
    }

    if (hds->i4OutputType == Output_LPK) //speaker
    {
        return_data.speaker.freqL = freqDataL[0];
        return_data.speaker.freqR = freqDataR[0];
        return_data.speaker.amplL = magDataL[0];
        return_data.speaker.amplR = magDataR[0];
		ALOGD(TAG "ATA Return Data(Speaker): FreqL = %d, FreqR = %d, AmpL = %d, AmpR = %d", return_data.speaker.freqL, return_data.speaker.freqR, return_data.speaker.amplL, return_data.speaker.amplR);    }
    else if (hds->i4OutputType == Output_HS)
    {
        return_data.receiver.freqL = freqDataL[0];
        return_data.receiver.freqR = freqDataR[0];
        return_data.receiver.amplL = magDataL[0];
        return_data.receiver.amplR = magDataR[0];
		ALOGD(TAG "ATA Return Data(Receiver): FreqL = %d, FreqR = %d, AmpL = %d, AmpR = %d", return_data.receiver.freqL, return_data.receiver.freqR, return_data.receiver.amplL, return_data.receiver.amplR);
    }
    ALOGD(TAG "%s: Stop", __FUNCTION__);
    pthread_exit(NULL); // thread exit
    return NULL;
}

static void ParseWaveHeader(FILE *PReadFile)
{
    if (PReadFile != NULL)
    {
        fread((void *)&WaveHeader, WAVE_HEADER_SIZE, 1, PReadFile);
    }
}

static int getFileNAmeSize(char buffer[])
{
    int length = 0;
    while (buffer[length] != '\0' &&  buffer[length] != '\n' &&  buffer[length] != '\r' && buffer[length] != ';')
    {
        //printf("buffer[%d] = %x \n",length,buffer[length]);
        length++;
    }
    return length;
}

static void Audio_Wave_clear_WavePlayInstance(WavePlayData *pWaveInstance)
{
    if (pWaveInstance->FileName != NULL)
    {
        delete[] pWaveInstance->FileName;
        pWaveInstance->FileName = NULL;
    }
    printf("delete[] WavePlayInstance.FileName; \n");
    if (pWaveInstance->pFile != NULL)
    {
        fclose(pWaveInstance->pFile);
        pWaveInstance->pFile = NULL;
    }
    printf(" fclose(WavePlayInstance.pFile; \n");
    memset((void *)pWaveInstance, 0 , sizeof(WavePlayData) - sizeof(pthread_t));
    printf("memset((void*)&WavePlayInstance, 0 ,sizeof(WavePlayData) - sizeof(pthread_t)) \n");
}

static void *Audio_Wave_Playabck_thread(void *mPtr)
{
    struct mAudio *hds  = (struct mAudio *)mPtr;
    struct itemview *iv = hds->iv;
    int wave_play_time  = hds->i4Playtime;

    ALOGD(TAG "%s: Start\n", __FUNCTION__);

    // for filelist and now read file
    FILE *pFileList = NULL;
    FILE *PReadFile = NULL;
    // buffer fir read filelist and file
    char FileNamebuffer[MAX_FILE_NAME_SIZE];
    int readlength = 0;
    bool FileListEnd = false;
    bool openfielerror = false;
    WavePlayData WavePlayInstance;
    memset((void *)&WavePlayInstance, 0 , sizeof(WavePlayData));

    //open input file  list
    pFileList = fopen(FileListNamesdcard, "rb");
    if (pFileList == NULL)
    {
        printf("error reopen file %s\n", FileListNamesdcard);
        pFileList = fopen(FileListName, "rb");
        if (pFileList == NULL)
        {
            FileListEnd = true;
            openfielerror = true;
            printf("error opening file %s\n", FileListName);
        }
    }

    while (pFileList && !feof(pFileList))
    {
        char *CompareNamebuffer = NULL;
        int filelength = 0;
        int CompareResult = -1;
        memset((void *)FileNamebuffer, 0, MAX_FILE_NAME_SIZE);
        fgets(FileNamebuffer, 100, pFileList);

        // crop file name to waveplay data structure
        filelength = getFileNAmeSize(FileNamebuffer);
        printf("getFileNAmeSize = %d\n", filelength);
        if (filelength > 0)
        {
            CompareNamebuffer = new char[filelength + 1];
            memset((void *)CompareNamebuffer, '\0', filelength + 1);
            memcpy((void *)CompareNamebuffer, (void *)FileNamebuffer, filelength);
            printf("get file list filename %s\n", FileNamebuffer);
            printf("get file list CompareNamebuffer %s\n", CompareNamebuffer);
            CompareResult = strcmp(CompareNamebuffer, hds->file_name);
            printf("CompareResult = %d \n", CompareResult);
        }

        if (CompareNamebuffer)
        {
            delete[] CompareNamebuffer;
            CompareNamebuffer = NULL;
        }

        if (CompareResult == 0)
        {
            printf("CompareResult ==0 \n");
            break;
        }
    }

    WavePlayInstance.i4Output = hds->i4OutputType;

    while (1)
    {
        if (openfielerror == true)
        {
            /* preare text view info */
            char *ptr;
            ptr = hds->info;
            ptr += sprintf(ptr, "error open ini file\n");
        }
        // read file list is not null
        while (((pFileList && !feof(pFileList) && FileListEnd == false) || (WavePlayInstance.ThreadStart == true)) && hds->exit_thd == false)
        {
            if (wave_play_time < 0)
            {
                WavePlayInstance.ThreadExit = true;
                goto WAVE_SLEEP;
            }
            if (WavePlayInstance.ThreadStart == false)
            {
                // clear all wave data.
                if (WavePlayInstance.ThreadExit == true && WavePlayInstance.ThreadStart == false)
                {
                    printf("WavePlayInstance.ThreadExit = true clean all data\n");
                    Audio_Wave_clear_WavePlayInstance(&WavePlayInstance);
                    WavePlayInstance.WavePlayThread = (pthread_t) NULL;
                }

                // get Filelist FileNamebuffer
                int filelength = 0;
                memset((void *)FileNamebuffer, 0, MAX_FILE_NAME_SIZE);
                fgets(FileNamebuffer, 100, pFileList);
                printf("get file list filename %s\n", FileNamebuffer);

                // crop file name to waveplay data structure
                filelength = getFileNAmeSize(FileNamebuffer);
                printf("getFileNAmeSize = %d\n", filelength);
                if (filelength > 0)
                {
                    WavePlayInstance.FileName = new char[filelength+7+1]; 
                    memset((void *)WavePlayInstance.FileName, '\0', filelength + 7 + 1);
                    memcpy((void *)WavePlayInstance.FileName, "/system", 7);  // add prefix "/system"
                    memcpy((void *)WavePlayInstance.FileName + 7, (void *)FileNamebuffer, filelength);                   
                }
                printf("get filename %s\n", WavePlayInstance.FileName);

                // create audio playback rounte
                if (WavePlayInstance.WavePlayThread == (pthread_t) NULL)
                {
                    WavePlayInstance.ThreadStart = true ;
                    WavePlayInstance.ThreadExit = false ;
                    printf("Audio_Wave_playback\n");
                    Audio_Wave_playback((void *)&WavePlayInstance);
                    printf("Audio_Wave_playback thread create\n");
                }
            }

WAVE_SLEEP:
            usleep(WAVE_PLAY_SLEEP_TIME * 1000); // sleep 100 ms
            wave_play_time -= WAVE_PLAY_SLEEP_TIME ;
        }

        if (hds->exit_thd)
        {
            WavePlayInstance.ThreadExit = true;
            int RetryCount = 10;
            printf("hds->exit_thd WavePlayInstance.ThreadExit = true\n");
            while (WavePlayInstance.ThreadStart != false && RetryCount > 0)
            {
                printf("WavePlayInstance.ThreadStart = false; \n");
                RetryCount--;
                usleep(50 * 1000);
            }
            // clear all wave data.
            if (WavePlayInstance.ThreadExit == true)
            {
                printf("WavePlayInstance.ThreadExit = true clean all data\n");
                Audio_Wave_clear_WavePlayInstance(&WavePlayInstance);
            }
            break;
        }
        usleep(WAVE_PLAY_SLEEP_TIME * 1000); // sleep 100 ms
        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
    }

    ALOGD(TAG "%s: Audio_Wave_Playabck_thread Exit \n", __FUNCTION__);
    pthread_exit(NULL); // thread exit
    return NULL;
}

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

//Loopback phonemic and speaker test
int mAudio_loopback_phonemicspk_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct mAudio *mc = (struct mAudio *)priv;
    struct textview *tv;
    struct itemview *iv;

    ALOGD(TAG "--------------mAudio_entry----------------\n");
    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, "", COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

    // init Audio
    Common_Audio_init();
    mc->exit_thd = false;

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
    iv->set_items(iv, audio_items_auto, 0);
    iv->set_text(iv, &mc->text);
    iv->start_menu(iv, 0);
    iv->redraw(iv);
#if 0
#if defined(WAVE_PLAYBACK)
    memset(mc->file_name, 0, MAX_FILE_NAME_SIZE);
    strcpy(mc->file_name, FileStartString_LoudspkPlayback);
    mc->i4OutputType = Output_LPK;
    mc->i4Playtime = read_preferred_ringtone_time() * 1000;
    pthread_create(&mc->hHeadsetThread, NULL, Audio_Wave_Playabck_thread, priv);
#else
    pthread_create(&mc->hHeadsetThread, NULL, Audio_LoudSpk_Playabck_thread, priv);
#endif
    do
    {
        chosen = iv->run(iv, &exit);
        switch (chosen)
        {
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
            ALOGD("mAudio_loopback_phonemicspk_entry set exit_thd = true\n");
            mc->exit_thd = true;
            break;
        }
    }
    while (1);
#endif

    memset(mc->info, 0, sizeof(mc->info) / sizeof(*(mc->info)));
    memset(mc->file_name, 0, MAX_FILE_NAME_SIZE);
    strcpy(mc->file_name, FileStartString_LoudspkPlayback);
    mc->i4OutputType = Output_LPK;
    mc->i4Playtime = read_preferred_ringtone_time() * 1000; //ms
    //mc->recordDevice = BUILTIN_MIC;   //Yi-Lung: Record device is selected in function init.
    pthread_create(&mc->hHeadsetThread, NULL, Audio_Wave_Playabck_thread, priv);
    pthread_create(&mc->hRecordThread, NULL, Audio_Record_thread, priv);

    int    play_time = mc->i4Playtime;
    mc->mod->test_result = FTM_TEST_FAIL;

    for (int i = 0; i < 100 ; i ++)
    {
        if (strstr(mc->info, "Check freq pass"))
        {
            mc->mod->test_result = FTM_TEST_PASS;
            ALOGD("Check freq pass");
            break;
        }
        usleep(play_time * 10);
    }

    if (mc->mod->test_result == FTM_TEST_FAIL)
    {
        ALOGD("Check freq fail");
    }
    if (mc->mod->test_result == FTM_TEST_PASS)
    {
        usleep(2000000);
    }

    mc->exit_thd = true;
    pthread_join(mc->hRecordThread, NULL);
    pthread_join(mc->hHeadsetThread, NULL);
    Common_Audio_deinit();

    LOGD(TAG "%s: End\n", __FUNCTION__);

    return 0;
}

#ifdef RECEIVER_HEADSET_AUTOTEST
//Receiver test
int mAudio_receiver_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct mAudio *mc = (struct mAudio *)priv;
    struct textview *tv;
    struct itemview *iv;

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
    iv->set_items(iv, audio_items_auto, 0);
    iv->set_text(iv, &mc->text);
    iv->start_menu(iv, 0);
    iv->redraw(iv);

    memset(mc->info, 0, sizeof(mc->info) / sizeof(*(mc->info)));
    memset(mc->file_name, 0, MAX_FILE_NAME_SIZE);
    strcpy(mc->file_name, FileStartString_LoudspkPlayback);
    mc->i4OutputType = Output_HS;
    mc->i4Playtime = read_preferred_ringtone_time() * 1000; //ms
    //mc->recordDevice = BUILTIN_MIC;   //Yi-Lung: Record device is selected in function init.
    pthread_create(&mc->hHeadsetThread, NULL, Audio_Wave_Playabck_thread, priv);
    pthread_create(&mc->hRecordThread, NULL, Audio_Record_thread, priv);

    int    play_time = mc->i4Playtime;
    mc->mod->test_result = FTM_TEST_FAIL;

    for (int i = 0; i < 200 ; i ++)
    {
        //ALOGD("check mc info:%d",i);
        if (strstr(mc->info, "Check freq pass"))
        {
            mc->mod->test_result = FTM_TEST_PASS;
            ALOGD("Check freq pass");
            break;
        }
        usleep(play_time * 5);
    }

    if (mc->mod->test_result == FTM_TEST_FAIL)
    {
        ALOGD("Check freq fail");
    }

    if (mc->mod->test_result == FTM_TEST_PASS)
    {
        usleep(2000000);
    }

    mc->exit_thd = true;
    pthread_join(mc->hRecordThread, NULL);
    pthread_join(mc->hHeadsetThread, NULL);
    Common_Audio_deinit();

    LOGD(TAG "%s: End\n", __FUNCTION__);
    return 0;
}
#else
void mAudio_receiver_playtone(struct mAudio *mc, void *priv)
{
#if defined(WAVE_PLAYBACK)
    memset(mc->file_name, 0, MAX_FILE_NAME_SIZE);
    strcpy(mc->file_name, FileStartString_ReceiverPlayback);
    mc->i4OutputType = Output_HS;
    mc->i4Playtime = read_preferred_receiver_time() * 1000;
    pthread_create(&mc->hHeadsetThread, NULL, Audio_Wave_Playabck_thread, priv);
#else
    pthread_create(&mc->hHeadsetThread, NULL, Audio_Receiver_Playabck_thread, priv);
#endif
}

void mAudio_receiver_stoptone(struct mAudio *mc)
{
    pthread_join(mc->hHeadsetThread, NULL);
}

//Receiver test
int mAudio_receiver_entry(struct ftm_param *param, void *priv)
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
#endif
//Loopback test
int mAudio_reveiverloopback_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct mAudio *mc = (struct mAudio *)priv;
    mc->exit_thd = false;
    mc->Headset_change = false;
    mc->avail = false;

    struct textview *tv;
    struct itemview *iv;

    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, &mc->info[0], COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

    // init Audio
    Common_Audio_init();
    g_loopback_item = 1;  // default: use MIC1 loopback
    g_prev_mic_state = 1;
    g_mic_change = 0;

    b_mic1_loopback = true;
#ifdef MTK_DUAL_MIC_SUPPORT
    b_mic2_loopback = false;
#else
    b_mic2_loopback = true; //No need to do mic2 test.
#endif
    print_len1 = 0;
    print_len2 = 0;
    b_incomplete_flag = false;

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
    iv->set_items(iv, audio_items_loopback, 0);
    iv->set_text(iv, &mc->text);

#ifdef FEATURE_FTM_HEADSET
    pthread_create(&mc->hHeadsetThread, NULL, Audio_Headset_detect_thread, priv);
#else
    //Add by Charlie, temp solution
    pthread_create(&mc->hHeadsetThread, NULL, Audio_Mic_change_thread, priv);
    //~Add by Charlie, temp solution
#endif

    do
    {
        chosen = iv->run(iv, &exit);
        switch (chosen)
        {
            case ITEM_MIC1:
                ALOGD("Select Mic1 loopback");
                b_mic1_loopback = true;
                g_loopback_item = 1;  // use MIC1 loopback
                if (g_prev_mic_state != g_loopback_item)
                {
                    g_mic_change = 1;
                }
                g_prev_mic_state = 1;
                break;
            case ITEM_MIC2:
                ALOGD("Select Mic2 loopback");
                b_mic2_loopback = true;
                g_loopback_item = 2;  // use MIC2 loopback
                if (g_prev_mic_state != g_loopback_item)
                {
                    g_mic_change = 1;
                }
                g_prev_mic_state = 2;
                break;
            case ITEM_PASS:
            case ITEM_FAIL:
                if (chosen == ITEM_PASS)
                {
                    if ((b_mic1_loopback == true) && (b_mic2_loopback == true))
                    {
                        b_incomplete_flag = false;
                        mc->mod->test_result = FTM_TEST_PASS;
                    }
                    else
                    {
                        char *ptr = mc->info + print_len1 + print_len2;
                        sprintf(ptr, "[%s] \n\n", " Test Loopback Case In-Complete ");
                        b_incomplete_flag = true;
                        iv->set_text(iv, &mc->text);
                        iv->redraw(iv);

                        break;
                    }
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
            ALOGD("mAudio_reveiverloopback_entry set exit_thd = true");
            mc->exit_thd = true;
            break;
        }
    }
    while (1);

#ifdef FEATURE_FTM_HEADSET
    pthread_join(mc->hHeadsetThread, NULL);

    if (mc->avail == true)
    {
        if (g_loopback_item == 1)
        {
            HeadsetMic_EarphoneLR_Loopback(MIC1_OFF, mc->Headset_mic);  // disable Earphone MIC1 loopback
        }
        else if (g_loopback_item == 2)
        {
            HeadsetMic_EarphoneLR_Loopback(MIC2_OFF, mc->Headset_mic);  // disable Earphone MIC2 loopback
        }
    }
    else
    {
        if (g_loopback_item == 1)
        {
            PhoneMic_Receiver_Loopback(MIC1_OFF);    // disable Receiver MIC1 loopback
        }
        else if (g_loopback_item == 2)
        {
            PhoneMic_Receiver_Loopback(MIC2_OFF);    // disable Receiver MIC2 loopback
        }
    }
#else
    //Add by Charlie, temp solution
    pthread_join(mc->hHeadsetThread, NULL);
    //~Add by Charlie, temp solution
    if (g_loopback_item == 1)
    {
        PhoneMic_Receiver_Loopback(MIC1_OFF);    // disable MIC1 loopback
    }
    else if (g_loopback_item == 2)
    {
        PhoneMic_Receiver_Loopback(MIC2_OFF);    // disable MIC2 loopback
    }
#endif

    g_loopback_item = 0;
    g_prev_mic_state = 0;
    g_mic_change = 0;
    Common_Audio_deinit();

    return 0;
}

#ifdef FEATURE_FTM_ACSLB
int mAudio_Acoustic_Loopback_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    int Acoustic_Type = DUAL_MIC_WITHOUT_DMNR_ACS_ON; //0:Acoustic loopback off; 1:Singlemic acoustic loopback; 2:Dualmic acoustic loopback
    int Acoustic_Status = 0;
    bool exit = false;
    bool bDualMicTestComplete = false;
    struct mAudio *mc = (struct mAudio *)priv;
    mc->exit_thd = false;
    mc->Headset_change = false;
    mc->avail = false;

    struct textview *tv;
    struct itemview *iv;

    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, &mc->info[0], COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

    // init Audio
    Common_Audio_init();

    // ui start
    if (!mc->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            ALOGD(TAG "No memory");
            Common_Audio_deinit();
            return -1;
        }
        mc->iv = iv;
    }
    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, audio_items_acoustic_loopback, 0);
    iv->set_text(iv, &mc->text);

#ifdef MTK_DUAL_MIC_SUPPORT
    Acoustic_Type = DUAL_MIC_WITH_DMNR_ACS_ON;
#endif

#ifdef FEATURE_FTM_HEADSET
    // inint thread mutex
    pthread_mutex_init(&mc->mHeadsetMutex, NULL);
    pthread_create(&mc->hHeadsetThread, NULL, PhoneMic_Receiver_Headset_Acoustic_Loopback, priv);
#else
    PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Type, &Acoustic_Status, mc->avail);
#endif

    bDualMicTestComplete = true;

    do
    {
        chosen = iv->run(iv, &exit);
        Audio_headset_update_info(mc, mc->info);  // get headset information
        ALOGD("mAudio_Acoustic_Loopback_entry headset available %d", mc->avail);
        switch (chosen)
        {
            case ITEM_DUALMIC_DMNR:
#ifdef FEATURE_FTM_HEADSET
                pthread_mutex_lock(&mc->mHeadsetMutex);
#endif
                PhoneMic_Receiver_Acoustic_Loopback(ACOUSTIC_STATUS, &Acoustic_Status, mc->avail);
                if (Acoustic_Status & 0x1)
                {
                    PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Status - 1, &Acoustic_Status, mc->avail);
                }
                PhoneMic_Receiver_Acoustic_Loopback(DUAL_MIC_WITH_DMNR_ACS_ON, &Acoustic_Status, mc->avail);
                bDualMicTestComplete = true;
#ifdef FEATURE_FTM_HEADSET
                pthread_mutex_unlock(&mc->mHeadsetMutex);
#endif
                break;
            case ITEM_DUALMIC_NO_DMNR:
#ifdef FEATURE_FTM_HEADSET
                pthread_mutex_lock(&mc->mHeadsetMutex);
#endif
                PhoneMic_Receiver_Acoustic_Loopback(ACOUSTIC_STATUS, &Acoustic_Status, mc->avail);
                if (Acoustic_Status & 0x1)
                {
                    PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Status - 1, &Acoustic_Status, mc->avail);
                }
                PhoneMic_Receiver_Acoustic_Loopback(DUAL_MIC_WITHOUT_DMNR_ACS_ON, &Acoustic_Status, mc->avail);
#ifdef FEATURE_FTM_HEADSET
                pthread_mutex_unlock(&mc->mHeadsetMutex);
#endif
                break;
            case ITEM_PASS:
#ifdef MTK_DUAL_MIC_SUPPORT
                if (bDualMicTestComplete == true)
                {
                    mc->mod->test_result = FTM_TEST_PASS;
                }
                else
                {
                    char *ptr = mc->info;
                    sprintf(ptr, "[%s] \n\n", " Test DMNR Case In-Complete ");
                    iv->set_text(iv, &mc->text);
                    iv->redraw(iv);
                    break;
                }
#else
                mc->mod->test_result = FTM_TEST_PASS;
#endif
                exit = true;
                break;
            case ITEM_FAIL:
                mc->mod->test_result = FTM_TEST_FAIL;
                exit = true;
                break;
            default:
                break;
        }

        if (exit)
        {
            ALOGD("mAudio_reveiverloopback_entry set exit_thd = true");
            mc->exit_thd = true;
            break;
        }
    }
    while (1);

#ifdef FEATURE_FTM_HEADSET
    pthread_join(mc->hHeadsetThread, NULL);
    pthread_mutex_lock(&mc->mHeadsetMutex);
#endif
    PhoneMic_Receiver_Acoustic_Loopback(ACOUSTIC_STATUS, &Acoustic_Status, mc->avail);
    if (Acoustic_Status & 0x1)
    {
        ALOGD("mAudio_Acoustic_Loopback_entry turn off loopback Acoustic_Status = %d", Acoustic_Status);
        PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Status - 1, &Acoustic_Status, mc->avail);
    }
#ifdef FEATURE_FTM_HEADSET
    pthread_mutex_unlock(&mc->mHeadsetMutex);
    pthread_mutex_destroy(&mc->mHeadsetMutex);
#endif
    usleep(50000);
    Common_Audio_deinit();
    mc->exit_thd = true;
    return 0;
}
#endif

int mAudio_waveplayback_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct mAudio *mc = (struct mAudio *)priv;
    struct textview *tv;
    struct itemview *iv;

    ALOGD(TAG "----------mAudio_waveplayback_entry----------------\n");
    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, &mc->info[0], COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

    // init Audio
    Common_Audio_init();
    mc->exit_thd = false;

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
    iv->set_items(iv, audio_items, 0);
    iv->set_text(iv, &mc->text);

    memset(mc->file_name, 0, MAX_FILE_NAME_SIZE);
    strcpy(mc->file_name, FileStartString_WavePlayback);
    mc->i4OutputType = Output_LPK;
    mc->i4Playtime = WAVE_PLAY_MAX_TIME;

    pthread_create(&mc->hHeadsetThread, NULL, Audio_Wave_Playabck_thread, priv);

    do
    {
        chosen = iv->run(iv, &exit);
        switch (chosen)
        {
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
            ALOGD("mAudio_waveplayback_entry set exit_thd = true\n");
            mc->exit_thd = true;
            break;
        }
    }
    while (1);

    pthread_join(mc->hHeadsetThread, NULL);
    Common_Audio_deinit();
    return 0;

}

static void Audio_headset_info(struct mAudio *hds, char *info)
{
    int fd = -1;
    int hookstatus = 0;
    char *ptr ;
    int OriginState = hds->avail;

    fd = open(HEADSET_STATE_PATH, O_RDONLY, 0);
    if (fd == -1)
    {
        ALOGD(TAG "1Can't open %s\n", HEADSET_STATE_PATH);
        hds->avail = false;
        goto EXIT;
    }
    if (read(fd, rbuf, BUF_LEN) == -1)
    {
        ALOGD(TAG "1Can't read %s\n", HEADSET_STATE_PATH);
        hds->avail = false;
        goto EXIT;
    }
    if (!strncmp(wbuf, rbuf, BUF_LEN))
    {
        hds->avail = true;
        goto EXIT;
    }

    if (!strncmp(wbuf1, rbuf, BUF_LEN))
    {
        hds->avail = true;
    }
    else
    {
        hds->avail = false;
    }
EXIT:
    if (OriginState  !=  hds->avail)
    {
        hds->Headset_change = true;
    }

    /* preare text view info */
    ptr = info;
    ptr += sprintf(ptr, "Please Insert The Headset for This Test  \n\n");

    close(fd);
    return;
}

static void *Audio_PMic_Headset_Loopback(void *mPtr)
{
    struct mAudio *hds = (struct mAudio *)mPtr;
    struct itemview *iv = hds->iv;

    ALOGD(TAG "%s: Start\n", __FUNCTION__);

    // open headset device
    HeadsetFd = open(HEADSET_PATH, O_RDONLY);
    if (HeadsetFd < 0)
    {
        ALOGE("1open %s error fd = %d", HEADSET_PATH, HeadsetFd);
        return 0;
    }

    //set headset state to 1 , enable hook
    ::ioctl(HeadsetFd, SET_CALL_STATE, 1);

    PhoneMic_EarphoneLR_Loopback(MIC1_ON);

    while (1)
    {
        char *ptr;
        usleep(100000);
        if (hds->exit_thd)
        {
            break;
        }

        Audio_headset_info(hds, hds->info);  // get headset information

        if (hds->avail && hds->Headset_change)
        {
            ALOGD(TAG  "Audio_PMic_Headset_Loopback:Headset plug-in (%d)\n", g_loopback_item);
            if (g_loopback_item == 1)
            {
                usleep(3000);
                PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
                usleep(3000);
                PhoneMic_EarphoneLR_Loopback(MIC1_ON);
            }
            else if (g_loopback_item == 2)
            {
                usleep(3000);
                PhoneMic_EarphoneLR_Loopback(MIC2_OFF);
                usleep(3000);
                PhoneMic_EarphoneLR_Loopback(MIC2_ON);
            }
            g_mic_change = 0;
            hds->Headset_change = false;
        }
        else if (hds->Headset_change)
        {
            ALOGD(TAG "Audio_PMic_Headset_Loopback:Headset plug-out (%d)\n", g_loopback_item);
            if (g_loopback_item == 1)
            {
                usleep(3000);
                PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
                usleep(3000);
                PhoneMic_EarphoneLR_Loopback(MIC1_ON);
            }
            else if (g_loopback_item == 2)
            {
                usleep(3000);
                PhoneMic_EarphoneLR_Loopback(MIC2_OFF);
                usleep(3000);
                PhoneMic_EarphoneLR_Loopback(MIC2_ON);
            }
            g_mic_change = 0;
            hds->Headset_change = false;
        }
        else
        {
            if (g_mic_change == 1)
            {
                g_mic_change = 0;
                if (g_loopback_item == 1)
                {
                    usleep(3000);
                    PhoneMic_EarphoneLR_Loopback(MIC2_OFF);
                    usleep(3000);
                    PhoneMic_EarphoneLR_Loopback(MIC1_ON);
                }
                else if (g_loopback_item == 2)
                {
                    usleep(3000);
                    PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
                    usleep(3000);
                    PhoneMic_EarphoneLR_Loopback(MIC2_ON);
                }
            }
        }

        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
    }

    ALOGD(TAG "%s: Audio_PMic_Headset_Loopback Exit (%d) \n", __FUNCTION__, g_loopback_item);

    PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
    usleep(3000);
    PhoneMic_EarphoneLR_Loopback(MIC2_OFF);

    // set headset state to 0 , lose headset device.
    ::ioctl(HeadsetFd, SET_CALL_STATE, 0);

    close(HeadsetFd);
    HeadsetFd = 0;

    pthread_exit(NULL); // thread exit
    return NULL;
}

int Audio_PhoneMic_Headset_loopback_entry(struct ftm_param *param, void *priv)
{
    ALOGD("Select Audio_PhoneMic_Headset_loopback_entry");
    char *ptr;
    int chosen;
    bool exit = false;
    struct mAudio *mc = (struct mAudio *)priv;
    mc->exit_thd = false;
    mc->Headset_change = false;
    mc->avail = false;

    struct textview *tv;
    struct itemview *iv;

    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, &mc->info[0], COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

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
    iv->set_items(iv, audio_items_loopback, 0);
    iv->set_text(iv, &mc->text);

    //check headset state
#ifdef GET_HEADSET_STATE
    init_Headset();
    int headset_state = get_headset_info();
    if (headset_state > 0)
    {
        ;
    }
    else
    {
        ALOGD("no headset device\n");
        sprintf(mc->info, "%s", "Please Insert Headset before This Test!\n");
        iv->redraw(iv);
        usleep(3000 * 1000);
        return -1;
    }
#endif

    // init Audio
    Common_Audio_init();
    g_loopback_item = 1;  // default: use MIC1 loopback
    g_prev_mic_state = 1;
    g_mic_change = 0;

#ifdef FEATURE_FTM_HEADSET
    Audio_headset_info(mc, mc->info);  // get headse information
#endif

#ifdef FEATURE_FTM_HEADSET
    pthread_create(&mc->hHeadsetThread, NULL, Audio_PMic_Headset_Loopback, priv);
#endif

    do
    {
        chosen = iv->run(iv, &exit);
        switch (chosen)
        {
            case ITEM_MIC1:
                ALOGD("Select Mic1 loopback");
                g_loopback_item = 1;  // use MIC1 loopback
                if (g_prev_mic_state != g_loopback_item)
                {
                    g_mic_change = 1;
                }
                g_prev_mic_state = 1;
                break;
            case ITEM_MIC2:
                ALOGD("Select Mic2 loopback");
                g_loopback_item = 2;  // use MIC2 loopback
                if (g_prev_mic_state != g_loopback_item)
                {
                    g_mic_change = 1;
                }
                g_prev_mic_state = 2;
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
            ALOGD("Audio_PhoneMic_Headset_loopback_entry set exit_thd = true");
            mc->exit_thd = true;
            break;
        }
    }
    while (1);

#ifdef FEATURE_FTM_HEADSET
    pthread_join(mc->hHeadsetThread, NULL);

    if (mc->avail == true)
    {
        ALOGD("disable HeadsetMic_EarphoneLR_Loopback:%d ", g_loopback_item);
        if (g_loopback_item == 1)
        {
            PhoneMic_EarphoneLR_Loopback(MIC1_OFF);    // disable MIC1 loopback
        }
        else if (g_loopback_item == 2)
        {
            PhoneMic_EarphoneLR_Loopback(MIC2_OFF);    // disable MIC2 loopback
        }
    }
#endif

    g_loopback_item = 0;
    g_prev_mic_state = 0;
    g_mic_change = 0;
    Common_Audio_deinit();
    return 0;
}

int Audio_PhoneMic_Speaker_loopback_entry(struct ftm_param *param, void *priv)
{
    ALOGD("Select Audio_PhoneMic_Speaker_loopback_entry");
    char *ptr;
    int chosen;
    bool exit = false;
    struct mAudio *mc = (struct mAudio *)priv;
    mc->exit_thd = false;
    mc->Headset_change = false;
    mc->avail = false;

    struct textview *tv;
    struct itemview *iv;

    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, &mc->info[0], COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

    // init Audio
    Common_Audio_init();
    g_loopback_item = 1;  // default: use MIC1 loopback
    g_prev_mic_state = 1;

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
    iv->set_items(iv, audio_items_loopback, 0);
    iv->set_text(iv, &mc->text);

    PhoneMic_SpkLR_Loopback(MIC1_ON);

    do
    {
        chosen = iv->run(iv, &exit);
        switch (chosen)
        {
            case ITEM_MIC1:
                ALOGD("Select Mic1 loopback");
                g_loopback_item = 1;  // use MIC1 loopback
                if (g_prev_mic_state != g_loopback_item)
                {
                    PhoneMic_SpkLR_Loopback(MIC2_OFF);
                }
                else
                {
                    break;
                }
                PhoneMic_SpkLR_Loopback(MIC1_ON);
                g_prev_mic_state = 1;
                break;
            case ITEM_MIC2:
                ALOGD("Select Mic2 loopback");
                g_loopback_item = 2;  // use MIC2 loopback
                if (g_prev_mic_state != g_loopback_item)
                {
                    PhoneMic_SpkLR_Loopback(MIC1_OFF);
                }
                else
                {
                    break;
                }
                PhoneMic_SpkLR_Loopback(MIC2_ON);
                g_prev_mic_state = 2;
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
            ALOGD("Audio_PhoneMic_Speaker_loopback_entry exit_thd = true");
            mc->exit_thd = true;
            break;
        }
    }
    while (1);


    ALOGD("disable Audio_PhoneMic_Speaker_loopback_entry:%d ", g_loopback_item);
    if (g_loopback_item == 1)
    {
        PhoneMic_SpkLR_Loopback(MIC1_OFF);    // disable MIC1 loopback
    }
    else if (g_loopback_item == 2)
    {
        PhoneMic_SpkLR_Loopback(MIC2_OFF);    // disable MIC2 loopback
    }

    g_loopback_item = 0;
    g_prev_mic_state = 0;

    Common_Audio_deinit();
    return 0;
}

static void *Audio_HMic_SPK_Loopback(void *mPtr)
{
    struct mAudio *hds = (struct mAudio *)mPtr;
    struct itemview *iv = hds->iv;
    ALOGD(TAG "%s: Start\n", __FUNCTION__);

    // open headset device
    HeadsetFd = open(HEADSET_PATH, O_RDONLY);
    if (HeadsetFd < 0)
    {
        ALOGE("2open %s error fd = %d", HEADSET_PATH, HeadsetFd);
        return 0;
    }

    //set headset state to 1 , enable hook
    ::ioctl(HeadsetFd, SET_CALL_STATE, 1);
    HeadsetMic_SpkLR_Loopback(MIC1_ON);

    while (1)
    {
        char *ptr;
        usleep(100000);
        if (hds->exit_thd)
        {
            break;
        }
        Audio_headset_info(hds, hds->info);  // get headset information
        if (hds->avail && hds->Headset_change)
        {
            ALOGD(TAG  "Audio_HMic_SPK_Loopback:Headset plug-in (%d)\n", g_loopback_item);
            HeadsetMic_SpkLR_Loopback(MIC1_ON);
            hds->Headset_change = false;
        }
        else if (hds->Headset_change)
        {
            ALOGD(TAG "Audio_HMic_SPK_Loopback:Headset plug-out (%d)\n", g_loopback_item);
            HeadsetMic_SpkLR_Loopback(MIC1_ON);
            hds->Headset_change = false;
        }
        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
    }

    ALOGD(TAG "%s: Audio_HMic_SPK_Loopback Exit (%d) \n", __FUNCTION__, g_loopback_item);
    HeadsetMic_SpkLR_Loopback(MIC1_OFF);

    // set headset state to 0 , lose headset device.
    ::ioctl(HeadsetFd, SET_CALL_STATE, 0);

    close(HeadsetFd);
    HeadsetFd = 0;

    pthread_exit(NULL); // thread exit
    return NULL;
}

int Audio_HeadsetMic_Speaker_loopback_entry(struct ftm_param *param, void *priv)
{
    ALOGD("Select Audio_HeadsetMic_Speaker_loopback_entry");
    char *ptr;
    int chosen;
    bool exit = false;
    struct mAudio *mc = (struct mAudio *)priv;
    mc->exit_thd = false;
    mc->Headset_change = false;
    mc->avail = false;

    struct textview *tv;
    struct itemview *iv;

    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, &mc->info[0], COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

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
    iv->set_items(iv, audio_items, 0);
    iv->set_text(iv, &mc->text);

    //check headset state
#ifdef GET_HEADSET_STATE
    init_Headset();
    int headset_state = get_headset_info();
    if (headset_state > 0)
    {
        ;
    }
    else
    {
        ALOGD("[FM]no headset device\n");
        sprintf(mc->info, "%s", "Please Insert Headset before This Test!\n");
        iv->redraw(iv);
        usleep(3000 * 1000);
        return -1;
    }
#endif

    // init Audio
    Common_Audio_init();
    g_loopback_item = 1;  // default: use MIC1 loopback
    g_prev_mic_state = 1;
    g_mic_change = 0;

#ifdef FEATURE_FTM_HEADSET
    Audio_headset_info(mc, mc->info);  // get headse information
#endif

#ifdef FEATURE_FTM_HEADSET
    pthread_create(&mc->hHeadsetThread, NULL, Audio_HMic_SPK_Loopback, priv);
#endif

    do
    {
        chosen = iv->run(iv, &exit);
        switch (chosen)
        {
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
            ALOGD("Audio_HeadsetMic_Speaker_loopback_entry set exit_thd = true");
            mc->exit_thd = true;
            break;
        }
    }
    while (1);

#ifdef FEATURE_FTM_HEADSET
    pthread_join(mc->hHeadsetThread, NULL);
    if (mc->avail == true)
    {
        ALOGD("disable Audio_HeadsetMic_Speaker_loopback_entry:%d ", g_loopback_item);
        HeadsetMic_SpkLR_Loopback(MIC1_OFF);    // disable MIC1 loopback
    }
#endif

    g_loopback_item = 0;
    g_prev_mic_state = 0;
    g_mic_change = 0;
    Common_Audio_deinit();
    return 0;
}

//-------- Micbias Test Entry --------
int mAudio_Micbias_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    bMicbias_exit = false; //for ATA Tool control.
    
    short pbuffer[2]={0};
    bool isMicOn = false;
	
    struct mAudio*mc = (struct mAudio *)priv;
    struct textview *tv;
    struct itemview *iv;
    
    ALOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, &mc->info[0], COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);

    // init Audio
    Common_Audio_init();

    // ui start
    if (!mc->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            ALOGD(TAG "No memory");
            Common_Audio_deinit();
            return -1;
        }
        mc->iv = iv;
    }
    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, audio_items_micbias, 0);
    iv->set_text(iv, &mc->text);

    do {
        // Test by Manual
        if(!get_is_ata())
        {
        chosen = iv->run(iv, &exit);
        
        switch (chosen) {
        case ITEM_MICBIAS_ON: /*Open Mic*/
            ALOGD(TAG "Micbias ON.");
			if(!isMicOn)
            {
                recordInit(BUILTIN_MIC);
                memset(pbuffer,0,sizeof(pbuffer));
                readRecordData(pbuffer,2*2);
            }
			isMicOn = true;
            break;
        case ITEM_MICBIAS_OFF: /*Close Mic*/
            ALOGD(TAG "Micbias OFF.");
            if(isMicOn)
                Common_Audio_deinit();
            isMicOn = false;
			break;
        case ITEM_PASS:
            mc->mod->test_result = FTM_TEST_PASS;
            exit = true;
            break;
        case ITEM_FAIL:
            mc->mod->test_result = FTM_TEST_FAIL;
            exit = true;
            break;
        default:
            break;
        }

        if (exit) {
            bMicbias_exit = true;
            ALOGD(TAG "Micbias EXIT.");
            break;
        }
		}
        // Test by ATA Tool
        else
        {
            if(!isMicOn)
            {
                ALOGD(TAG "Micbias ON by ATA.");
                recordInit(BUILTIN_MIC);
                memset(pbuffer,0,sizeof(pbuffer));
                readRecordData(pbuffer,2*2);
				isMicOn = true;
            }
            iv->start_menu(iv, 0);
			iv->redraw(iv);
            if(bMicbias_exit)
            {
                ALOGD(TAG "Micbias EXIT by ATA.");
                break;
            }
        }
    } while (1);

    usleep(50000);
    Common_Audio_deinit();
	
    return 0;
}

//-------------------------------

int audio_init(void)
{
    int ret = 0;
    struct ftm_module *modLoudspk, *modReceiver,
            *modReceiverLoopback,
            *modPMic_Headset_Loopback,
            *modPMic_SPK_Loopback,
            *modHMic_SPK_Loopback,
            *modWavePlayback,
            *modAcoustic_Loopback,
            *modLoudspk_phone,
                      *modReceiver_phone,
                      *modMicbias;

    struct mAudio *maudioSpk, *maudioReveiver,
            *maudioReveiverLoopback,
            *maudioPMic_Headset_Loopback,
            *maudioPMic_SPK_Loopback,
            *maudioHMic_SPK_Loopback,
            *maudioWavePlayback,
            *maudioAcousticLoopback,
            *maudioSpk_phone,
                  *maudioReveiver_phone,
                  *maudioMicbias;

    ALOGD(TAG "%s\n", __FUNCTION__);
    ALOGD(TAG "-------Audio_init------------------\n");

    modLoudspk = ftm_alloc(ITEM_LOOPBACK_PHONEMICSPK, sizeof(struct mAudio));
    maudioSpk = mod_to_mAudio(modLoudspk);
    maudioSpk->mod = modLoudspk;
	maudioSpk->recordDevice = BUILTIN_MIC;
    if (!modLoudspk)
{
        return -ENOMEM;
    }
    ret = ftm_register(modLoudspk, mAudio_loopback_phonemicspk_entry, (void *)maudioSpk);

    modReceiver = ftm_alloc(ITEM_RECEIVER, sizeof(struct mAudio));
    maudioReveiver = mod_to_mAudio(modReceiver);
    maudioReveiver->mod = modReceiver;
	maudioReveiver->recordDevice = BUILTIN_MIC;
    if (!modReceiver)
    {
        return -ENOMEM;
    }
    ret = ftm_register(modReceiver, mAudio_receiver_entry, (void *)maudioReveiver);

    modReceiverLoopback = ftm_alloc(ITEM_LOOPBACK, sizeof(struct mAudio));
    maudioReveiverLoopback = mod_to_mAudio(modReceiverLoopback);
    maudioReveiverLoopback->mod = modReceiverLoopback;
    if (!modReceiverLoopback)
    {
        return -ENOMEM;
    }
    ret = ftm_register(modReceiverLoopback, mAudio_reveiverloopback_entry, (void *)maudioReveiverLoopback);

    modPMic_Headset_Loopback = ftm_alloc(ITEM_LOOPBACK1, sizeof(struct mAudio));
    maudioPMic_Headset_Loopback = mod_to_mAudio(modPMic_Headset_Loopback);
    maudioPMic_Headset_Loopback->mod = modPMic_Headset_Loopback;
    if (!modPMic_Headset_Loopback)
    {
        return -ENOMEM;
    }
    ret = ftm_register(modPMic_Headset_Loopback, Audio_PhoneMic_Headset_loopback_entry, (void *)maudioPMic_Headset_Loopback);

    modPMic_SPK_Loopback = ftm_alloc(ITEM_LOOPBACK2, sizeof(struct mAudio));
    maudioPMic_SPK_Loopback = mod_to_mAudio(modPMic_SPK_Loopback);
    maudioPMic_SPK_Loopback->mod = modPMic_SPK_Loopback;
    if (!modPMic_SPK_Loopback)
    {
        return -ENOMEM;
    }
    ret = ftm_register(modPMic_SPK_Loopback, Audio_PhoneMic_Speaker_loopback_entry, (void *)maudioPMic_SPK_Loopback);

    modHMic_SPK_Loopback = ftm_alloc(ITEM_LOOPBACK3, sizeof(struct mAudio));
    maudioHMic_SPK_Loopback = mod_to_mAudio(modHMic_SPK_Loopback);
    maudioHMic_SPK_Loopback->mod = modHMic_SPK_Loopback;
    if (!modHMic_SPK_Loopback)
    {
        return -ENOMEM;
    }
    ret = ftm_register(modHMic_SPK_Loopback, Audio_HeadsetMic_Speaker_loopback_entry, (void *)maudioHMic_SPK_Loopback);

    modWavePlayback = ftm_alloc(ITEM_WAVEPLAYBACK, sizeof(struct mAudio));
    maudioWavePlayback = mod_to_mAudio(modWavePlayback);
    maudioWavePlayback->mod = modWavePlayback;
    if (!maudioWavePlayback)
    {
        return -ENOMEM;
    }
    ret = ftm_register(modWavePlayback, mAudio_waveplayback_entry, (void *)maudioWavePlayback);
#ifdef FEATURE_FTM_ACSLB
    modAcoustic_Loopback = ftm_alloc(ITEM_ACOUSTICLOOPBACK, sizeof(struct mAudio));
    maudioAcousticLoopback = mod_to_mAudio(modAcoustic_Loopback);
    maudioAcousticLoopback->mod = modAcoustic_Loopback;
    if (!maudioAcousticLoopback)
    {
        return -ENOMEM;
    }
    ret = ftm_register(modAcoustic_Loopback, mAudio_Acoustic_Loopback_entry, (void *)maudioAcousticLoopback);
#endif
//---Phone Level Test
    modLoudspk_phone= ftm_alloc(ITEM_LOOPBACK_PHONEMICSPK_PHONE, sizeof(struct mAudio));
    maudioSpk_phone= mod_to_mAudio(modLoudspk_phone);
    maudioSpk_phone->mod = modLoudspk_phone;
	maudioSpk_phone->recordDevice = WIRED_HEADSET;
    if (!modLoudspk_phone)
        return -ENOMEM;
    ret = ftm_register(modLoudspk_phone, mAudio_loopback_phonemicspk_entry, (void*)maudioSpk_phone);

	modReceiver_phone= ftm_alloc(ITEM_RECEIVER_PHONE, sizeof(struct mAudio));
    maudioReveiver_phone= mod_to_mAudio(modReceiver_phone);
    maudioReveiver_phone->mod = modReceiver_phone;
	maudioReveiver_phone->recordDevice = WIRED_HEADSET;
    if (!modReceiver_phone)
        return -ENOMEM;
    ret = ftm_register(modReceiver_phone, mAudio_receiver_entry, (void*)maudioReveiver_phone);
//---
    //Micbias Test
    modMicbias = ftm_alloc(ITEM_MICBIAS, sizeof(struct mAudio));
    maudioMicbias = mod_to_mAudio(modMicbias);
    maudioMicbias->mod = modMicbias;
    if (!modMicbias)
        return -ENOMEM;
    ret = ftm_register(modMicbias, mAudio_Micbias_entry, (void*)maudioMicbias);

    return ret;
}
#endif

#ifdef __cplusplus
};
#endif


