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
#endif

#include "kal_release.h"
#include "matvctrl.h"
#include "ftm_matv_common.h"
#include "cust_matv.h"
#include "ftm_audio_Common.h"
#if !defined(ANALOG_AUDIO)
#include "AudioI2S.h"
#include "ftm_matv_audio.h"
#endif
/********************
 * Data Type and Definition
 *********************/

#define mod_to_mMTAV(p)     (struct mMATV*)((char*)(p) + sizeof(struct ftm_module))
#define TAG         "[MATV] "
#define MAX_CH    128
#define TEXT_SIZE  1024
#define COUNTRY_NAME_SZIE  64
#define PREVIEW_ENABLE
//#define REFRESH_BUTTON
#define COUNTRY_SELECT
#define USE_CHANNEL_LIST_UI

#ifdef MATV_TOATL_CH 
#if (MATV_TOATL_CH!=0)
#define USE_MATV_CUST_SETTINGS
#endif
#endif

#define MATV_INIT_VALUE (-1)
#define MATV_INIT_FAIL   (0)
#define MATV_INIT_OK     (1)


enum {
    ITEM_DISP_SEARCHED_CH,
#ifdef PREVIEW_ENABLE        
    ITEM_PREVIEW_CH,    
#endif    
    ITEM_SWITCH_CH,
    ITEM_REFRESH_CH,
    //Country
    ITEM_AFGHANISTAN,
    ITEM_ARGENTINA,
    ITEM_AUSTRALIA,
    ITEM_BRAZIL,
    ITEM_BURMA,
    ITEM_CAMBODIA,
    ITEM_CANADA,
    ITEM_CHILE,
    ITEM_CHINA,
    ITEM_CHINA_HONGKONG,
    ITEM_CHINA_SHENZHEN,
    ITEM_EUROPE_EASTERN,
    ITEM_EUROPE_WESTERN,
    ITEM_FRANCE,
    ITEM_FRENCH_COLONIE,
    ITEM_INDIA,
    ITEM_INDONESIA,
    ITEM_IRAN,
    ITEM_ITALY,
    ITEM_JAPAN,
    ITEM_KOREA,
    ITEM_LAOS,
    ITEM_MALAYSIA,
    ITEM_MEXICO,
    ITEM_NEWZEALAND,
    ITEM_PAKISTAN,
    ITEM_PARAGUAY,
    ITEM_PHILIPPINES,
    ITEM_PORTUGAL,
    ITEM_RUSSIA,
    ITEM_SINGAPORE,
    ITEM_SOUTHAFRICA,
    ITEM_SPAIN,
    ITEM_TAIWAN,
    ITEM_THAILAND,
    ITEM_TURKEY,
    ITEM_UNITED_ARAB_EMIRATES,
    ITEM_UNITED_KINGDOM,
    ITEM_USA,
    ITEM_URUGUAY,
    ITEM_VENEZUELA,
    ITEM_VIETNAM,
    ITEM_IRELAND,
    ITEM_MOROCCO,
    ITEM_BANGLADESH,    
    //~Country
    ITEM_EXIT,
    ITEM_PASS,
    ITEM_FAIL
};

static item_t matv_normal_items[] = {
    {ITEM_PASS, uistr_pass, 0},
    {ITEM_FAIL, uistr_fail, 0},        
    {-1, NULL, 0},
};

#ifdef USE_CHANNEL_LIST_UI
static item_t matv_autoscan_items[] = {
    {ITEM_PREVIEW_CH, uistr_info_atv_channellist, 0},
    {ITEM_PASS, uistr_pass, 0},
    {ITEM_FAIL, uistr_fail, 0},        
    {-1, NULL, 0},
};
#else
static item_t matv_autoscan_items[] = {
    //{ITEM_DISP_SEARCHED_CH, "List Channels"},
#ifdef PREVIEW_ENABLE    
    {ITEM_PREVIEW_CH, uistr_info_atv_previewCH},
#endif        
    {ITEM_SWITCH_CH, uistr_info_atv_switchCH},
#ifdef REFRESH_BUTTON        
    {ITEM_REFRESH_CH, uistr_info_atv_refreshCH},
#endif        
    {ITEM_PASS, uistr_pass},
    {ITEM_FAIL, uistr_fail},        
    {-1, NULL},
};
#endif

#ifdef USE_CHANNEL_LIST_UI
#define MAX_CH_TABLE_SIZE 128
static item_t matv_chpreview_items[MAX_CH_TABLE_SIZE+1];
#endif

#ifdef COUNTRY_SELECT
static item_t matv_select_country_items[] = {
    {ITEM_AFGHANISTAN,Country_AFGHANISTAN, 0},
    {ITEM_ARGENTINA,Country_ARGENTINA, 0},
    {ITEM_AUSTRALIA,Country_AUSTRALIA, 0},
    {ITEM_BRAZIL,Country_BRAZIL, 0},
    {ITEM_BURMA,Country_BURMA, 0},
    {ITEM_CAMBODIA,Country_CAMBODIA, 0},
    {ITEM_CANADA,Country_CANADA, 0},
    {ITEM_CHILE,Country_CHILE, 0},
    {ITEM_CHINA,Country_CHINA, 0},
    {ITEM_CHINA_HONGKONG,Country_CHINA_HONGKONG, 0},
    {ITEM_CHINA_SHENZHEN,Country_CHINA_SHENZHEN, 0},
    {ITEM_EUROPE_EASTERN,Country_EUROPE_EASTERN, 0},
    {ITEM_EUROPE_WESTERN,Country_EUROPE_WESTERN, 0},
    {ITEM_FRANCE,Country_FRANCE, 0},
    {ITEM_FRENCH_COLONIE,Country_FRENCH_COLONIE, 0},
    {ITEM_INDIA,Country_INDIA, 0},
    {ITEM_INDONESIA,Country_INDONESIA, 0},
    {ITEM_IRAN,Country_IRAN, 0},
    {ITEM_ITALY,Country_ITALY, 0},
    {ITEM_JAPAN,Country_JAPAN, 0},
    {ITEM_KOREA,Country_KOREA, 0},
    {ITEM_LAOS,Country_LAOS, 0},
    {ITEM_MALAYSIA,Country_MALAYSIA, 0},
    {ITEM_MEXICO,Country_MEXICO, 0},
    {ITEM_NEWZEALAND,Country_NEWZEALAND, 0},
    {ITEM_PAKISTAN,Country_PAKISTAN, 0},
    {ITEM_PARAGUAY,Country_PARAGUAY, 0},
    {ITEM_PHILIPPINES,Country_PHILIPPINES, 0},
    {ITEM_PORTUGAL,Country_PORTUGAL, 0},
    {ITEM_RUSSIA,Country_RUSSIA, 0},
    {ITEM_SINGAPORE,Country_SINGAPORE, 0},
    {ITEM_SOUTHAFRICA,Country_SOUTHAFRICA, 0},
    {ITEM_SPAIN,Country_SPAIN, 0},
    {ITEM_TAIWAN,Country_TAIWAN, 0},
    {ITEM_THAILAND,Country_THAILAND, 0},
    {ITEM_TURKEY,Country_TURKEY, 0},
    {ITEM_UNITED_ARAB_EMIRATES,Country_UNITED_ARAB_EMIRATES, 0},
    {ITEM_UNITED_KINGDOM,Country_UNITED_KINGDOM, 0},
    {ITEM_USA,Country_USA, 0},
    {ITEM_URUGUAY,Country_URUGUAY, 0},
    {ITEM_VENEZUELA,Country_VENEZUELA, 0},
    {ITEM_VIETNAM,Country_VIETNAM, 0},
    {ITEM_IRELAND,Country_IRELAND, 0},
    {ITEM_MOROCCO,Country_MOROCCO, 0},
    {ITEM_BANGLADESH,Country_BANGLADESH, 0},          
    {ITEM_EXIT,Country_EXIT, 0},
    {-1, NULL, 0},
};
#endif

enum {
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


struct mMATV {
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
    struct itemview *iv_country;
    struct itemview *iv_ch_preview;
    //self data 
    ///unsigned char country;
    matv_ch_entry ch_ent[MAX_CH];
    int           ch_list[MAX_CH];
    int           ch_count;
    int           current_channel;
    char          country_name[COUNTRY_NAME_SZIE];
    char          status[TEXT_SIZE];
    int           info[MAX_INFO_NUM];
    pthread_t     update_thd;
    ///pthread_t     refresh_thd;
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

typedef struct mMATV_STATUS{
    int i4INIT;
    int i4SCAN;
}MATV_STATUS;

/********************
 * Global Vairable
 *********************/
int gi4TVPreview_Start = 0;
int gi4TVChangeChannel = 0;
int gi4TVUpdate_End = 0;
static MATV_STATUS grMATV_STATUS;
static  sem_t   semaphore; 

int Audio_enable = false;
static void *mI2Sdriver = NULL;
int mI2Sid = 0;
char* mAudioBuffer = NULL;
static int gi4flag_iv_thread = 0;
/********************
 * External Function
 *********************/    
extern int matv_preview_init();
extern int matv_preview_deinit();
extern int matv_preview_start();
extern int matv_preview_stop();
extern int matv_preview_reset_layer_buffer();

/********************
 * Function Declaration
 *********************/
static void matv_audio_path(char bEnable)
{
    if (bEnable == true)
    {
#if defined(ANALOG_AUDIO)                  
        Common_Audio_init();
        ATV_AudAnalogPath(true);
#else        
        ATV_AudPlay_On();
        if(!I2SSet(mI2Sdriver, 0 ) ){
           LOGD("I2S driver set MATV fail\n");
        }
        if(!I2SStart(mI2Sdriver,mI2Sid)){
             LOGD("I2S start fialed");
        }                                        
        usleep(500);
#endif

    }
    else
    {
#if defined(ANALOG_AUDIO)
        ATV_AudAnalogPath(false);
        Common_Audio_deinit();
#else
        usleep(500);                    
        ATV_AudPlay_Off();
        I2SStop(mI2Sdriver,mI2Sid);
#endif

    }
}
static void matv_reset_status()
{
    grMATV_STATUS.i4INIT = -1;
    grMATV_STATUS.i4SCAN = -1;
}

static void matv_set_init_status(int s4value)
{
    grMATV_STATUS.i4INIT = s4value;
}

#if 0
static void matv_set_scan_status(int s4value)
{
    grMATV_STATUS.i4SCAN = s4value;
}

static int matv_get_scan_status()
{
    return grMATV_STATUS.i4SCAN;
}
#endif

static int matv_get_init_status()
{
    return grMATV_STATUS.i4INIT;
}


static int matv_pv_result_key_handler(int key, void *priv) 
{
    int handled = 0, exit = 0;
    struct mMATV *matv = (struct mMATV *)priv;
    struct textview *tv = &matv->tv;
    struct ftm_module *fm = matv->mod;
    
    switch (key) {
    case UI_KEY_BACK:
        LOGD(TAG"Back Button Click\n"); 
        exit = 1;
        break;
    case UI_KEY_LEFT:
        LOGD(TAG"Key_Left Click\n");         
        //fm->test_result = FTM_TEST_FAIL;
        //cam_preview_result = FTM_TEST_FAIL; 
        exit = 1;
        break;
    case UI_KEY_CONFIRM:
        LOGD(TAG"Key_Confirm Click\n");                 
        //fm->test_result = FTM_TEST_PASS;
        //cam_preview_result = FTM_TEST_PASS; 
        exit = 1;
        break;
    default: 
        handled = -1;
        break;
    }
    if (exit) {
        LOGD(TAG "%s: Exit thead\n", __FUNCTION__);
        matv->exit_thd = true;
        tv->exit(tv);        
    }
    return handled;
}



static void *matv_update_preview(void *priv)
{
    struct mMATV *matv = (struct mMATV *)priv;
    struct textview *tv = &matv->tv;
    struct statfs stat;
    int count = 1, chkcnt = 5;
    int key; 

    tv = &matv ->tv;
    ui_init_textview(tv, matv_pv_result_key_handler, (void*)matv );
    tv->set_title(tv, &matv->title);
    //tv->set_text(tv, &matv->text);
    tv->set_btn(tv, &matv->left_btn, &matv->center_btn, &matv->right_btn);
    tv->redraw(tv); 
    tv->redraw(tv); 
    
    LOGD(TAG "%s: Start\n", __FUNCTION__);
    matv_preview_start();     
    LOGD(TAG "PREVIEW_Start \n");
    Audio_enable = true;
    //tv->run(tv);
    matv->exit_thd = false;    
    while (1) {
        key = ui_wait_key();
        usleep(200000);
        chkcnt--;
        tv->m_khandler(key, tv->m_priv);

        if (matv ->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

       chkcnt = 5;
    }
    Audio_enable = false;
    matv_preview_stop();
        
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    //pthread_exit(NULL);    
    return NULL;
}

#ifdef USE_CHANNEL_LIST_UI
static void *matv_update_channel_list(void *priv, int rCount)
{
    struct mMATV *matv = (struct mMATV *)priv;
    struct itemview *iv;
    struct statfs stat;
    int    chosen;
    int    key; 
    bool   exit = false;
    LOGD("Total Chanel is %d\n",rCount);
    LOGD("matv->iv_ch_preview = %x\n",(unsigned int)matv->iv_ch_preview);
    iv  = matv->iv_ch_preview;   
    iv->set_title(iv, &matv->title);
    iv->set_items(iv, matv_chpreview_items, 0);
    iv->set_text(iv, &matv->text);

    

    while (1) {
        chosen = iv->run(iv, &exit);
        LOGD("Select %d\n",chosen);
        if(chosen >= rCount)
            exit = true;
        else{
            LOGD("Change to channel#%d\n",matv->ch_list[chosen]);
            matv_ts_change_channel(matv->ch_list[chosen]);
        }
        if (exit) {
            break;
        }
        //matv_preview_init();

        //--> mATV audio path
        //-------------------------------------------
        matv_audio_path(true);                                                      
        //-------------------------------------------
        //mATV audio path <--                

        matv_update_preview(priv);               
        //--> mATV audio path
        //-------------------------------------------
        matv_audio_path(false);
        //-------------------------------------------
        //mATV audio path <--                                                      

        matv_preview_reset_layer_buffer();
        //matv_preview_deinit();

        iv->redraw(iv);
        
    }
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    return NULL;
}

#endif


static void matv_normal_update_info(struct mMATV *mc, char *info, int status)
{
    /* prepare info */
    char *ptr;
    ptr  = info;
    switch(status)
    {
        case 0:
            sprintf(ptr, uistr_info_atv_init_fail);
            break;
        case 1:
            sprintf(ptr, uistr_info_atv_init_ok);
            break;
        default:
            sprintf(ptr, uistr_info_atv_initizling);
            break;            
    }
    return;
}

static void *matv_normal_update_iv_thread(void *priv)
{
    struct mMATV    *mc = (struct mMATV *)priv;
    struct itemview *iv = mc->iv;
    struct statfs   stat;
    matv_ch_entry   ch_ent;
    int             ch_candidate = 0;
    int             i4_status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);    
    i4_status = matv_ts_init();
    matv_set_init_status(i4_status);
    matv_normal_update_info(mc, mc->status, i4_status);     
    iv->redraw(iv);
    matv_ts_shutdown();
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
  
	return NULL;
}

//Normal hw test (Only see hw initialization)
int mMATV_normal_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    int i4_status;
    bool exit = false;
    struct mMATV*mc = (struct mMATV *)priv;
    struct textview *tv;
    struct itemview *iv;

    LOGD(TAG "--------------mMATV_normal_entry----------------\n" );    
    init_text(&mc->title, "MATV Normal", COLOR_YELLOW);
    init_text(&mc->text, (const char*)&mc->status, COLOR_YELLOW);
    
    if (!mc->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        mc->iv = iv;
    }
    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, matv_normal_items, 0);
    iv->set_text(iv, &mc->text);

    mc->current_channel = 0;
    mc->ch_count = 0;
    matv_reset_status();
    matv_normal_update_info(mc, mc->status, -1);
    pthread_create(&mc->update_thd, NULL, matv_normal_update_iv_thread, priv);

    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {

        case ITEM_PASS:
        case ITEM_FAIL:
            if (matv_get_init_status()!=MATV_INIT_VALUE)// only after the complete of initialization, we can update result
            {       
                if (chosen == ITEM_PASS) {
                    mc->mod->test_result = FTM_TEST_PASS;
                } else if (chosen == ITEM_FAIL) {
                    mc->mod->test_result = FTM_TEST_FAIL;
                }
                exit = true;
            }
            break;
        default:
            break;
        }
        if (exit) {
            break;
        }
    } while (1);
    LOGD(TAG "%s Exit\n", __FUNCTION__);
    pthread_join(mc->update_thd, NULL);
   
    return (0);
}

static void matv_update_info(struct mMATV *mc, char *info)
{
    /* prepare info */
    char *ptr, *ptr1;
    char buf[TEXT_SIZE];
    int ch;
    int i;
    ptr = buf;
    ptr1  = info;    
    memset(buf, 0 , sizeof(buf));
#if !defined( USE_MATV_CUST_SETTINGS)
    if (/*(mc->country >= TV_AFGHANISTAN)&&*/(mc->country <= TV_MOROCCO))
    {        
        ptr +=sprintf(ptr, "TV Country = %s\n", mc->country_name); 
    }
#endif
    if (!tvscan_finish)
    {
        if (matv_get_init_status() == MATV_INIT_OK)
            ptr += sprintf(ptr, "TV Scanning...%d%%\n",tvscan_progress);        
        else if(matv_get_init_status() == MATV_INIT_FAIL)
            ptr += sprintf(ptr, "MATV init is failed.\n");        
    }
    else
    {

        //Get indicator information
        for (i=0 ; i<MAX_INFO_NUM ; i++)
            mc->info[i] = matv_ts_get_info(i);
#ifdef USE_CHANNEL_LIST_UI
        ptr += sprintf(ptr,uistr_info_atv_init_ok);
#else
        //Display info
        ch = mc->current_channel;
        ptr += sprintf(ptr,"Available CH = %d",mc->ch_list[i]);

        for (i=0;i<mc->ch_count;i++)
        {            
            ptr += sprintf(ptr,"%d,",mc->ch_list[i]);
        }
        ptr += sprintf(ptr,"\n=========\n");
        ptr += sprintf(ptr,"Current Ch[%d] = (%d/%d/%d)",mc->ch_list[ch], mc->ch_ent[ch].freq, mc->ch_ent[ch].sndsys, mc->ch_ent[ch].colsys);
        ptr += sprintf(ptr,"\n=========\n");
        ptr += sprintf(ptr,"PLT_STR = %d\n",mc->info[0]);
        ptr += sprintf(ptr,"CFO = %d\n",mc->info[1]);
        ptr += sprintf(ptr,"DRO_CVBS_SNR = %d\n",mc->info[2]);
        ptr += sprintf(ptr,"RF_Gain_Idx = %d\n",mc->info[3]);
        ptr += sprintf(ptr,"BB_Gain_Idx = %d\n",mc->info[4]);
        ptr += sprintf(ptr,"AGC_Status = %d\n",mc->info[5]);
        ptr += sprintf(ptr,"TVD_LOCK = %d\n",mc->info[6]);
        ptr += sprintf(ptr,"TVD_NrLvl = %d\n",mc->info[7]);
        ptr += sprintf(ptr,"TVD_BurstLock = %d\n",mc->info[8]);      
#endif
    }
    memcpy(ptr1,buf,TEXT_SIZE);
    return;
}

/*
 * Update auto-scan information 
 */
static void *matv_update_iv_thread(void *priv)
{
    struct mMATV    *mc = (struct mMATV *)priv;
    struct itemview *iv = mc->iv;
    struct statfs   stat;
    ///matv_ch_entry   ch_ent;
    int             ch_candidate = 0;
    int             i;
    int             status;
#ifdef USE_CHANNEL_LIST_UI
    item_t          *preview_menu;
#endif
    LOGD(TAG "%s: Start\n", __FUNCTION__);
    gi4flag_iv_thread = 0;
#if 0 
    matv_preview_init();
#endif 
    status = matv_ts_init();
    matv_set_init_status(status);

    if (matv_get_init_status()==MATV_INIT_OK)
    {
#ifdef USE_MATV_CUST_SETTINGS
        for(i = 0; i<MATV_TOATL_CH ; i++)
        {
            matv_ts_set_chttable(i+1,&MATV_CH_TABLE[i]);
        }
        tvscan_finish=1; 
#else
        matv_ts_scan(mc->country);
        //Wait scan complete
        while (1) {
            usleep(500000);

            if (tvscan_finish)
                break;

            matv_update_info(mc, mc->status);
            iv->redraw(iv);
        }
#endif
        //Get cht table
        i=1;
        mc->ch_count = 0;
        while(matv_ts_get_chttable(i++,&mc->ch_ent[mc->ch_count]))    
        {   
            
            if(mc->ch_ent[mc->ch_count].flag&CH_VALID)        
            {
                LOGD(TAG "channel:%d, freq %d\n",i,mc->ch_ent[mc->ch_count].freq);
                mc->ch_list[mc->ch_count] = i-1;
                mc->ch_count++;  
                if(mc->ch_count >= MAX_CH)
                {
                    LOGD(TAG "ch_count Exceed MAX_CH");
                    break;
                }
            }   
        }
        
        //Change to the last channel
        LOGD(TAG "CHANGE to the latest channel!\n");
        {
            mc->current_channel = mc->ch_count-1; 
            if((mc->current_channel >=0)
                && (mc->current_channel < MAX_CH))
                matv_ts_change_channel(mc->ch_list[mc->current_channel]);
        }
        #ifdef ANALOG_AUDIO    
        matv_set_chipdep(190,0);//Turn on analog audio
        #endif

        matv_update_info(mc, mc->status);  
        iv->redraw(iv);

        while (1) {
            usleep(500000);//update every 0.5 sec

            if (gi4TVUpdate_End/*End Signal*/)
                break;
            if (tvscan_finish)
            {
                 if (gi4TVChangeChannel == 1)
                 {
                    gi4TVChangeChannel= 0;
                    mc->current_channel ++;
                    if (mc->current_channel >= mc->ch_count)
                    {
                        mc->current_channel = 0;    
                    }
                    matv_ts_change_channel(mc->ch_list[mc->current_channel]);
                }
                if (0==gi4TVPreview_Start)
                {
                    //LOGD("Update View while no tv preview\n");
                    matv_update_info(mc, mc->status);
                    iv->redraw(iv);
                }            
                else
                {

#ifdef USE_CHANNEL_LIST_UI
                    preview_menu = &matv_chpreview_items[0];
                    //LOGD("CH count = %d\n",mc->ch_count);
                    if (mc->ch_count<MAX_CH_TABLE_SIZE)
                    {
                        for (i=0;i<mc->ch_count;i++)
                        {
                            preview_menu[i].id   = i;
                            preview_menu[i].name = (const char*)malloc(10);
                            sprintf((char*)preview_menu[i].name,"%s_%d",uistr_info_atv_CH, mc->ch_list[i]);
                        }    
                        preview_menu[i].id    = i;
                        preview_menu[i].name  = (const char*)malloc(10);
                        sprintf((char*)preview_menu[i].name,uistr_key_back);
                        preview_menu[i+1].id  = -1;
                        preview_menu[i+1].name= NULL;
                    }
                    else{
                        LOGE("[Error]channel count %d exceeds %d\n",mc->ch_count,MAX_CH_TABLE_SIZE);
                    }
                    matv_update_channel_list(priv,mc->ch_count); 
                    preview_menu = &matv_chpreview_items[0];
                    for (i=0;i<MAX_CH_TABLE_SIZE;i++)
                    {
                        char *ptr = (char*) preview_menu[i].name;
                        if(ptr){
                            free(ptr);
                            ptr = NULL;
                        }
                        preview_menu[i].name = NULL;
                        preview_menu[i].id   = -1;
                    }
                    gi4TVPreview_Start = 0;
#else                    
                    
                    
                    //--> mATV audio path
                    //-------------------------------------------
                    matv_audio_path(true);                  
                    //-------------------------------------------
                    //mATV audio path <--                
                                                                   
                    matv_update_preview(priv);               

                    //--> mATV audio path
                    //-------------------------------------------
                    matv_audio_path(false);
                    //-------------------------------------------
                    //mATV audio path <--                                                      


                    gi4TVPreview_Start = 0;
#endif
                }                       
            }
        }
    }
    else
    {
        LOGD(TAG "MATV Init Fail!\n");
        matv_update_info(mc, mc->status);  
        iv->redraw(iv);        
    }
matv_update_iv_thread_exit:   
    
    matv_preview_reset_layer_buffer();
#if 0 
    matv_preview_deinit();
#endif 
    gi4flag_iv_thread = 1;
    pthread_exit(NULL);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
	return NULL;
}

#if 0
static void *matv_audio_thread(void *mPtr)
{
    struct mMATV    *mc = (struct mMATV *)mPtr;
    struct itemview *iv = mc->iv;
    struct statfs   stat;
    int bufSize = 0;
    int numread = 0;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
#if !defined(ANALOG_AUDIO)    
    bufSize = I2SGetReadBufferSize(mI2Sdriver);
    LOGD("got buffer size = %d", bufSize);
    mAudioBuffer = new char[bufSize * 3];

    while (1) 
    {
        if(gi4TVUpdate_End)
           break;
        
        if(Audio_enable == true)
        {
           numread = I2SRead(mI2Sdriver, mI2Sid, mAudioBuffer, bufSize);
           ATV_AudioWrite(mAudioBuffer, numread);           
        }
        else
        {
           usleep(500 * 1000);
        }
    }

    LOGD(TAG "%s: matv_audio_thread Exit\n", __FUNCTION__);

    if (mAudioBuffer) {
        delete [] mAudioBuffer;
        mAudioBuffer = NULL;
    }    
#endif    
    return NULL;
}
#endif



//Auto-scan test
int mMATV_scan_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    int i;
    bool exit = false;
    struct mMATV*mc = (struct mMATV *)priv;
    struct textview *tv;
    struct itemview *iv; 
    item_t *preview_menu;

    LOGD(TAG "--------------mMATV_entry----------------\n" );
    LOGD(TAG "%s\n", __FUNCTION__);
    matv_reset_status();
    if (!mc->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        mc->iv = iv;
    }
    
    if (!mc->iv_country) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        mc->iv_country = iv;
    }
#ifdef USE_CHANNEL_LIST_UI
    if (!mc->iv_ch_preview) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        mc->iv_ch_preview = iv;
    }
#endif    
    memset(mc->status,0,sizeof(mc->status));
    memset(mc->country_name,0,sizeof(mc->country_name));
    gi4TVUpdate_End = 0;
    gi4TVPreview_Start = 0;
    gi4TVChangeChannel = 0;

    matv_preview_init();

    
#ifdef COUNTRY_SELECT
#ifdef USE_MATV_CUST_SETTINGS
    //mc->country = MATV_COUNTRY;
    //sprintf(mc->country_name,"%s",matv_select_country_items[MATV_COUNTRY].name);
#else  //defined(USE_MATV_CUST_SETTINGS) 
    init_text(&mc->title, "Select Country", COLOR_YELLOW);
    init_text(&mc->text, (const char*)&mc->status, COLOR_YELLOW);

    iv = mc->iv_country;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, matv_select_country_items, 0);
    iv->set_text(iv, &mc->text);
    exit = false;
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
            case ITEM_AFGHANISTAN:
                mc->country  = matv_ts_get_country_id("TV_AFGHANISTAN");
                exit = true;
                break;
            case ITEM_ARGENTINA:
                mc->country  = matv_ts_get_country_id("TV_ARGENTINA");
                exit = true;
                break;
            case ITEM_AUSTRALIA:
                mc->country  = matv_ts_get_country_id("TV_AUSTRALIA");             
                exit = true;
                break;
            case ITEM_BRAZIL:
                mc->country  = matv_ts_get_country_id("TV_BRAZIL");
                exit = true;
                break;
            case ITEM_BURMA:
                mc->country  = matv_ts_get_country_id("TV_BURMA");
                exit = true;                
                break;
            case ITEM_CAMBODIA:
                mc->country  = matv_ts_get_country_id("TV_CAMBODIA");
                exit = true;                
                break;
            case ITEM_CANADA:
                mc->country  = matv_ts_get_country_id("TV_CANADA");
                exit = true;                
                break;
            case ITEM_CHILE:
                mc->country  = matv_ts_get_country_id("TV_CHILE");
                exit = true;                
                break;
            case ITEM_CHINA:
                mc->country  = matv_ts_get_country_id("TV_CHINA");
                exit = true;                
                break;
            case ITEM_CHINA_HONGKONG:
                mc->country  = matv_ts_get_country_id("TV_CHINA_HONGKONG");
                exit = true;                
                break;
            case ITEM_CHINA_SHENZHEN:
                mc->country  = matv_ts_get_country_id("TV_CHINA_SHENZHEN");
                exit = true;                
                break;
            case ITEM_EUROPE_EASTERN:
                mc->country  = matv_ts_get_country_id("TV_EUROPE_EASTERN");
                exit = true;                
                break;
            case ITEM_EUROPE_WESTERN:
                mc->country  = matv_ts_get_country_id("TV_EUROPE_WESTERN");
                exit = true;                
                break;
            case ITEM_FRANCE:
                mc->country  = matv_ts_get_country_id("TV_FRANCE");
                exit = true;
                break;
            case ITEM_FRENCH_COLONIE:
                mc->country  = matv_ts_get_country_id("TV_FRENCH_COLONIE");
                exit = true;
                break;
            case ITEM_INDIA:
                mc->country  = matv_ts_get_country_id("TV_INDIA");
                exit = true;                
                break;
            case ITEM_INDONESIA:
                mc->country  = matv_ts_get_country_id("TV_INDONESIA");
                exit = true;                
                break;
            case ITEM_IRAN:
                mc->country  = matv_ts_get_country_id("TV_IRAN");
                exit = true;                
                break;
            case ITEM_ITALY:
                mc->country  = matv_ts_get_country_id("TV_ITALY");
                exit = true;                
                break;
            case ITEM_JAPAN:
                mc->country  = matv_ts_get_country_id("TV_JAPAN");
                exit = true;                
                break;
            case ITEM_KOREA:
                mc->country  = matv_ts_get_country_id("TV_KOREA");
                exit = true;                
                break;
            case ITEM_LAOS:
                mc->country  = matv_ts_get_country_id("TV_LAOS");
                exit = true;                
                break;
            case ITEM_MALAYSIA:
                mc->country  = matv_ts_get_country_id("TV_MALAYSIA");
                exit = true;                
                break;
            case ITEM_MEXICO:
                mc->country  = matv_ts_get_country_id("TV_MEXICO");
                exit = true;                
                break;
            case ITEM_NEWZEALAND:
                mc->country  = matv_ts_get_country_id("TV_NEWZEALAND");
                exit = true;                
                break;
            case ITEM_PAKISTAN:
                mc->country  = matv_ts_get_country_id("TV_PAKISTAN");
                exit = true;                
                break;
            case ITEM_PARAGUAY:
                mc->country  = matv_ts_get_country_id("TV_PARAGUAY");
                exit = true;                
                break;
            case ITEM_PHILIPPINES:
                mc->country  = matv_ts_get_country_id("TV_PHILIPPINES");
                exit = true;                
                break;
            case ITEM_PORTUGAL:
                mc->country  = matv_ts_get_country_id("TV_PORTUGAL");
                exit = true;                
                break;
            case ITEM_RUSSIA:
                mc->country  = matv_ts_get_country_id("TV_RUSSIA");
                exit = true;                
                break;
            case ITEM_SINGAPORE:
                mc->country  = matv_ts_get_country_id("TV_SINGAPORE");
                exit = true;      
                break;
            case ITEM_SOUTHAFRICA:
                mc->country  = matv_ts_get_country_id("TV_SOUTHAFRICA");
                exit = true;                
                break;
            case ITEM_SPAIN:
                mc->country  = matv_ts_get_country_id("TV_SPAIN");
                exit = true;                
                break;
            case ITEM_TAIWAN:
                mc->country  = matv_ts_get_country_id("TV_TAIWAN");
                exit = true;                
                break;
            case ITEM_THAILAND:
                mc->country  = matv_ts_get_country_id("TV_THAILAND");
                exit = true;                
                break;
            case ITEM_TURKEY:
                mc->country  = matv_ts_get_country_id("TV_TURKEY");
                exit = true;                
                break;
            case ITEM_UNITED_ARAB_EMIRATES:
                mc->country  = matv_ts_get_country_id("TV_UNITED_ARAB_EMIRATES");
                exit = true;
                break;
            case ITEM_UNITED_KINGDOM:
                mc->country  = matv_ts_get_country_id("TV_UNITED_KINGDOM");
                exit = true;                
                break;
            case ITEM_USA:
                mc->country  = matv_ts_get_country_id("TV_USA");
                exit = true;                
                break;
            case ITEM_URUGUAY:
                mc->country  = matv_ts_get_country_id("TV_URUGUAY");
                exit = true;                
                break;
            case ITEM_VENEZUELA:
                mc->country  = matv_ts_get_country_id("TV_VENEZUELA");
                exit = true;                
                break;
            case ITEM_VIETNAM:
                mc->country  = matv_ts_get_country_id("TV_VIETNAM");
                exit = true;                
                break;
            case ITEM_IRELAND:
                mc->country  = matv_ts_get_country_id("TV_IRELAND");
                exit = true;                
                break;
            case ITEM_MOROCCO:
                mc->country  = matv_ts_get_country_id("TV_MOROCCO");
                exit = true;                
                break;
            case ITEM_BANGLADESH:
                mc->country  = matv_ts_get_country_id("TV_BANGLADESH");
                exit = true;
                break;
            case ITEM_EXIT:
                goto EXIT;
                break;
            default:
                goto EXIT;
                break;
        }
        if (exit) {
            sprintf(mc->country_name,"%s",matv_select_country_items[chosen-ITEM_AFGHANISTAN].name);
            break;
        }
    } while (1);
    
    exit = false;
#endif
#else  //!defined(COUNTRY_SELECT)
       //[ToDo]: Add read default channel info from sdcard. (Use ftm_get_prop)
        ptr = ftm_get_prop("MATV.COUNTRY");
        if(ptr!=NULL)
        {       
            mc->country = matv_ts_get_country_id(ptr);
        }else{

            mc->country = matv_ts_get_country_id("TV_TAIWAN");
        }
#endif 


    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, (const char*)&mc->status, COLOR_YELLOW);

    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, matv_autoscan_items, 0);
    iv->set_text(iv, &mc->text);
    
    mc->current_channel = 0;
    mc->ch_count = 0;
    
   
    matv_update_info(mc, mc->status);
    iv->redraw(iv);
    
    pthread_create(&mc->update_thd, NULL, matv_update_iv_thread, priv);

    Audio_enable = false;
#if !defined(ANALOG_AUDIO)   
    Common_Audio_init();
    mI2Sdriver = I2SGetInstance();
    if(mI2Sdriver == NULL){
       LOGD("I2S driver doesn't exists\n");
       goto EXIT_ATV;
    }                    
    mI2Sid = I2SOpen(mI2Sdriver);
    if(mI2Sid == 0){
       LOGD("I2S driver get ID fail\n");
       goto EXIT_ATV;
    }
    ////pthread_create(&mc->mAudioThread, NULL, matv_audio_thread, priv);
#endif    
#ifdef USE_CHANNEL_LIST_UI
    do {
        chosen = iv->run(iv, &exit);
        //LOGD(TAG "chosen code = %x\n",chosen);
        switch (chosen) {
        case ITEM_PREVIEW_CH:
            LOGD(TAG "Preview Channel");
            if (tvscan_finish)
            {
                init_text(&mc ->left_btn, " ", COLOR_YELLOW);
                init_text(&mc ->center_btn, uistr_key_back, COLOR_YELLOW);
                init_text(&mc ->right_btn, " ", COLOR_YELLOW);                 
                gi4TVPreview_Start = 1;
                while(gi4TVPreview_Start == 1)
                {
                    usleep(200000);
                }               
            }
            break;            
        case ITEM_PASS:
        case ITEM_FAIL:
            if ((tvscan_finish)||(matv_get_init_status() == MATV_INIT_FAIL))
            {
                if (chosen == ITEM_PASS) {
                    mc->mod->test_result = FTM_TEST_PASS;
                } else if (chosen == ITEM_FAIL) {
                    mc->mod->test_result = FTM_TEST_FAIL;
                }
                exit = true;
                
            }
            break;
        default:
            //LOGD(TAG "default");
            break;
        }
        if (exit) {
            gi4TVUpdate_End = 1;
            break;
        }
    } while (1);
#else
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
#ifdef PREVIEW_ENABLE             
        case ITEM_PREVIEW_CH:
            LOGD(TAG "Preview Channel");
            if (tvscan_finish)
            {
                init_text(&mc ->left_btn, uistr_fail, COLOR_YELLOW);
                init_text(&mc ->center_btn, uistr_pass, COLOR_YELLOW);
                init_text(&mc ->right_btn, uistr_key_back, COLOR_YELLOW);                 
                gi4TVPreview_Start = 1;
                while(gi4TVPreview_Start == 1)
                {
                    usleep(200000);
                }               
            }
            break;
#endif            
        case ITEM_SWITCH_CH:
            LOGD(TAG "Switch Channel!");
            if (tvscan_finish)
            {
                gi4TVChangeChannel = 1;
            }
            break;
#ifdef REFRESH_BUTTON            
        case ITEM_REFRESH_CH:
            LOGD(TAG "Refresh Channel!");
            if (tvscan_finish)
            {
                matv_update_info(mc, mc->status);  
                iv->redraw(iv);
            }
            break;           
#endif            
        case ITEM_PASS:
        case ITEM_FAIL:
            if ((tvscan_finish)||(matv_get_init_status() == MATV_INIT_FAIL))
            {
                if (chosen == ITEM_PASS) {
                    mc->mod->test_result = FTM_TEST_PASS;
                } else if (chosen == ITEM_FAIL) {
                    mc->mod->test_result = FTM_TEST_FAIL;
                }
                exit = true;
                
            }
            break;
        default:
            break;
        }
        if (exit) {
            gi4TVUpdate_End = 1;
            break;
        }
    } while (1);
#endif

EXIT_ATV:

    while(gi4flag_iv_thread == 0)
    {
        usleep(100*1000);
    }
#if !defined(ANALOG_AUDIO)
    I2SClose(mI2Sdriver,mI2Sid);
    //free I2S instance
    I2SFreeInstance(mI2Sdriver);
    mI2Sdriver = NULL;    

    Common_Audio_deinit();
#endif       
    matv_ts_shutdown();
    pthread_join(mc->update_thd, NULL);
    pthread_join(mc->mAudioThread, NULL);
    matv_preview_reset_layer_buffer();
    matv_preview_deinit();
  
EXIT:
    return 0;
}

int MATV_init(void)
{
    int ret = 0;
 
    struct ftm_module *modNormal,*modScan;
    struct mMATV *mMATVNormal,*mMATVScan;

    LOGD(TAG "%s\n", __FUNCTION__);
    LOGD(TAG "-------mATV_init------------------\n" );
    //Normal case
    modNormal = ftm_alloc(ITEM_MATV_NORMAL, sizeof(struct mMATV));
    if (!modNormal){
        LOGD(TAG "modInit = NULL" );
        return -ENOMEM;
    }
    mMATVNormal = mod_to_mMTAV(modNormal);
    mMATVNormal->mod = modNormal;

    ret = ftm_register(modNormal, mMATV_normal_entry, (void*)mMATVNormal);
    if (ret!=0)
        LOGD(TAG "ftm_register MATV_init fail!" );

    //Auto-scan case
    modScan = ftm_alloc(ITEM_MATV_AUTOSCAN, sizeof(struct mMATV));
    if (!modScan){
        LOGD(TAG "modInit = NULL" );
        return -ENOMEM;
    }
    mMATVScan = mod_to_mMTAV(modScan);
    mMATVScan->mod = modScan;

    ret = ftm_register(modScan, mMATV_scan_entry, (void*)mMATVScan);
    if (ret!=0)
        LOGD(TAG "ftm_register MATV_init fail!" );
    return ret;
}

#ifdef __cplusplus
};
#endif

#endif
