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

#include "common.h"
#include "miniui.h"
#include "ftm.h"


#ifdef FEATURE_FTM_MATV

#ifdef __cplusplus
extern "C" {
#include "kal_release.h"

#include "DIF_FFT.h"
#include "Audio_FFT.h"

#endif

#include "matvctrl.h"
#include "ftm_matv_common.h"
#include "cust_matv.h"
#include "cust_ftm_matv_comm.h"

#include "ftm_audio_Common.h"
/********************
 * Data Type and Definition
 *********************/

#define mod_to_mMTAV(p)     (struct mMATV*)((char*)(p) + sizeof(struct ftm_module))
#define TAG         "[MATV] "
#define MAX_CH    128
#define TEXT_SIZE  1024
#define COUNTRY_NAME_SZIE  64


#define MATV_INIT_VALUE (-1)
#define MATV_INIT_FAIL   (0)
#define MATV_INIT_OK     (1)

static item_t matv_autoscan_items[] =
{
    { -1, NULL, 0},
};

#define MAX_CH_TABLE_SIZE 128
static item_t matv_chpreview_items[MAX_CH_TABLE_SIZE + 1];

enum
{
    PLT_STR = 0,
    CFO,
    DRO_CVBS_SNR,
    RF_Gain_Idx,
    BB_Gain_Idx,
    AGC_Status,
    TVD_LOCK,
    TVD_NrLvl,
    TVD_BurstLock,
    MAX_INFO_NUM,
};


struct mMATV
{
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
    struct itemview *iv_country;
    struct itemview *iv_ch_preview;
    //self data
    unsigned char country;
    matv_ch_entry ch_ent[MAX_CH];
    int           ch_list[MAX_CH];
    int           ch_count;
    int           current_channel;
    char          country_name[COUNTRY_NAME_SZIE];
    char          status[TEXT_SIZE];
    int           info[MAX_INFO_NUM];
    pthread_t     update_thd;
    pthread_t     refresh_thd;
    pthread_t     mAudioThread;

    bool          exit_thd;
    //~self data
    //new button
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
    //~
    text_t    title;
    text_t    text;
};

typedef struct mMATV_STATUS
{
    int i4INIT;
    int i4SCAN;
} MATV_STATUS;

/********************
 * Global Vairable
 *********************/
int Audio_enable = false;
const char * CfgFileNamesdcard = "/sdcard/factory.ini";
const char * CfgFileName        = "/system/etc/factory.ini";
bool sdcard_insert = false;


static void *mI2Sdriver = NULL;
int mI2Sid = 0;
char *mAudioBuffer = NULL;
/********************
 * External Function
 *********************/
extern int matv_preview_init();
extern int matv_preview_deinit();
extern int matv_preview_start();
extern int matv_preview_stop();
extern int matv_preview_reset_layer_buffer();
extern bool matv_capture_check();

///extern bool recordOpen();
//extern int  recordRead(void *buffer, int size);
///extern bool recordClose();

extern int readRecordData(void * pbuffer,int bytes);


/********************
 * Function Declaration
 *********************/
 int matv_read_preferred_para(const char *pstr)
{
    int value = 0;
    unsigned int i = 0;
    char *pTime = NULL;

    if(pstr== NULL)
    {
        ALOGD("matv_read_preferred_para error %s", pstr);
        return 0;
    }
    pTime = ftm_get_prop(pstr);
    if (pTime != NULL){
        value = (int)atoi(pTime);
        ALOGD("preferred_receiver_para %s- %d \n",pstr,value);
    }
    else{
        ALOGD("preferred_receiver_para can't get %s\n", pstr);
    }
    return value;
}

static void matv_audio_path(char bEnable)
{
    if (bEnable == true)
    {
        Common_Audio_init();
#ifdef ANALOG_AUDIO
        ATV_AudAnalogPath(true);
#else
        Audio_MATV_I2S_Play(true);
#endif
    }
    else
    {
#ifdef ANALOG_AUDIO    
        ATV_AudAnalogPath(false);
#else
        Audio_MATV_I2S_Play(false);
#endif
        Common_Audio_deinit();
    }
}

static bool matv_auto_check_iv(void *priv)
{
    struct mMATV    *mc = (struct mMATV *)priv;
    struct itemview *iv = mc->iv;
    struct statfs   stat;
    matv_ch_entry   ch_ent;
    int             ch_candidate = 0;
    int             status;
    bool return_value = false;
    bool audio_check = true;
    bool preview_check = true;
    int  checkpass_cnt = 0;      
    int  chkcnt = 0;    
    int i = 0;
    
    int mAutoLockFreqTest = matv_read_preferred_para("mAutoLockFreqTest");;    
    int mAutoLockFreq = matv_read_preferred_para("mAutoLockFreq");
    int mAutoLockCountry = matv_read_preferred_para("mAutoLockCountry");
    
    LOGD(TAG "%s: Start.\n", __FUNCTION__);

    if((mAutoLockFreqTest > 0)
        && (mAutoLockFreq !=0))
    {
        if (matv_ata_lockstatus(mAutoLockFreq, mAutoLockCountry) != 0)
        {
            LOGD(TAG "%s: lock freq %d, country %d ok!!!!", __FUNCTION__, 
                    mAutoLockFreq, mAutoLockCountry);
            ///return_value = true;
        }
        else
        {
            LOGD(TAG "%s: lock  freq %d, country %d fail!!!!", __FUNCTION__, 
                    mAutoLockFreq, mAutoLockCountry);
            return return_value;
        }
    }
    
    matv_ata_avpatternout();

#ifdef ANALOG_AUDIO
    matv_set_chipdep(190, 3); //Turn on analog audio
#endif

    iv->redraw(iv);

    ///system("rm /data/record_atv_dataL.pcm");
    ///system("rm /data/record_atv_dataR.pcm");

    //--> mATV audio path
    matv_audio_path(true);
    Audio_enable = true;

    LOGD(TAG "%s: Start video\n", __FUNCTION__);

#ifdef ANALOG_AUDIO    
    recordInit(MATV_ANALOG);
#else
    recordInit(MATV_I2S);
#endif

    usleep(300 * 1000);
    ///recordOpen();
    short pbuffer[8192]={0}; 
    
    for(int recint = 0; recint <100; recint++)
        readRecordData(pbuffer,8192*2);
    
    while (chkcnt < 3)
    {
        preview_check = true;
        audio_check = true;

        matv_preview_start();

        usleep(300 * 1000);

        LOGD(TAG "matv_capture_start %d\n", chkcnt);

        short pbufferL[4096]={0};	
        short pbufferR[4096]={0};	
        unsigned int freqDataL[3]={0},magDataL[3]={0};
	    unsigned int freqDataR[3]={0},magDataR[3]={0};
        memset(pbuffer, 0, 8192*2);
        int readSize  = readRecordData(pbuffer,8192*2);
        
        LOGD(TAG "matv_capture_start_audio %d\n", chkcnt);
        
        for(int i = 0 ; i < 4096 ; i++)
        {
            pbufferL[i] = pbuffer[2 * i];
            pbufferR[i] = pbuffer[2 * i + 1];
        }

#if 0   
        LOGD(TAG "matv_capture_start3 %d\n", chkcnt);
        
        char filenameL[]="/data/record_atv_dataL.pcm";
        char filenameR[]="/data/record_atv_dataR.pcm";
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
#endif        

        memset(freqDataL,0,sizeof(freqDataL));
        memset(freqDataR,0,sizeof(freqDataR));
        memset(magDataL,0,sizeof(magDataL));
        memset(magDataR,0,sizeof(magDataR)); 
#ifdef ANALOG_AUDIO          
        ApplyFFT256(48000,pbufferL,0,freqDataL,magDataL);
        ApplyFFT256(48000,pbufferR,0,freqDataR,magDataR);
#else
        ApplyFFT256(32000,pbufferL,0,freqDataL,magDataL);
        ApplyFFT256(32000,pbufferR,0,freqDataR,magDataR);
#endif
        LOGD(TAG "matv_capture_audio fre L %d H %d\n", freqDataL[0], freqDataR[0] );
        if (((freqDataL[0] > 1100) || (freqDataL[0] < 900))
            || ((freqDataR[0] > 1100) || (freqDataR[0] < 900)))
        {
            audio_check = false;
            break;
        }

        preview_check = matv_capture_check();

        matv_preview_stop();
        chkcnt++;

        int mDisaleImageTest = matv_read_preferred_para("mDisaleImageTest");
        if(mDisaleImageTest > 0)
        {
            preview_check = true;
            LOGD(TAG "matv mDisaleImageTest, skip it ");
        }
        
        if ((preview_check == true)
                && (audio_check == true))
        {
            checkpass_cnt++;
        }
        LOGD(TAG "matv_checking chkcnt %d, prev_ret %d aud_ret %d ", chkcnt, preview_check, audio_check);
        if (checkpass_cnt >= 2)
        {
            return_value = true;
            break;
        }
    }

    Audio_enable = false;
    ///recordClose();

    LOGD(TAG "%s: stop video", __FUNCTION__);

    matv_ata_avpatternclose();
    matv_audio_path(false);

matv_update_iv_thread_exit:

    LOGD(TAG "%s: matv_test_result, checkpass_cnt %d return value %d", __FUNCTION__, checkpass_cnt, return_value);
    return return_value;
}


//Auto-scan test
int mMATV_scan_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    int i;
    bool exit = false;
    FILE *pFileList = NULL;
    struct mMATV *mc = (struct mMATV *)priv;
    struct textview *tv;
    struct itemview *iv;
    mc->mod->test_result = FTM_TEST_FAIL;

    LOGD(TAG "--------------mMATV_entry----------------\n");
    LOGD(TAG "%s\n", __FUNCTION__);

    if (!mc->iv)
    {
        iv = ui_new_itemview();

        if (!iv)
        {
            LOGD(TAG "No memory");
            return -1;
        }

        mc->iv = iv;
    }

    if (!mc->iv_country)
    {
        iv = ui_new_itemview();

        if (!iv)
        {
            LOGD(TAG "No memory");
            return -1;
        }

        mc->iv_country = iv;
    }

    if (!mc->iv_ch_preview)
    {
        iv = ui_new_itemview();

        if (!iv)
        {
            LOGD(TAG "No memory");
            return -1;
        }

        mc->iv_ch_preview = iv;
    }

    memset(mc->status, 0, sizeof(mc->status));
    memset(mc->country_name, 0, sizeof(mc->country_name));

    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, (const char *)&mc->status, COLOR_YELLOW);

    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, matv_autoscan_items, 0);
    iv->set_text(iv, &mc->text);
    mc->current_channel = 0;
    mc->ch_count = 0;
    iv->start_menu(iv, 0);
    iv->redraw(iv);

    if (matv_ts_init() != MATV_INIT_OK)
    {
        LOGE("matv_ts_init fail!!!!!");
        return -1;
    }

    matv_preview_init();

    Audio_enable = false;

    pFileList = fopen(CfgFileNamesdcard,"rb");
    if(pFileList != NULL)
    {
        sdcard_insert = true;
        fclose(pFileList);
        
        pFileList = NULL;
    }
    
    LOGD("matv_ts_init sdcard status %d!!!!!", sdcard_insert);

Auto_Check:

    if(matv_auto_check_iv(priv) == true)
        mc->mod->test_result = FTM_TEST_PASS;

EXIT_ATV:

    usleep(100 * 1000);


    matv_ts_shutdown();
    matv_preview_reset_layer_buffer();
    matv_preview_deinit();

    //remove temp file
    ///system("rm /data/matv_pattern.raw");

EXIT:
    return 0;
}

int MATV_init(void)
{
    int ret = 0;

    struct ftm_module *modNormal, *modScan;
    struct mMATV *mMATVNormal, *mMATVScan;

    LOGD(TAG "%s\n", __FUNCTION__);
    LOGD(TAG "-------mATV_init new------------------\n");

    //Auto-test
    modScan = ftm_alloc(ITEM_MATV_AUTOSCAN, sizeof(struct mMATV));

    if (!modScan)
    {
        LOGD(TAG "modInit = NULL");
        return -ENOMEM;
    }

    mMATVScan = mod_to_mMTAV(modScan);
    mMATVScan->mod = modScan;

    ret = ftm_register(modScan, mMATV_scan_entry, (void *)mMATVScan);

    if (ret != 0)
    {
        LOGD(TAG "ftm_register MATV_init fail!");
    }

    return ret;
}

#ifdef __cplusplus
};
#endif

#endif
