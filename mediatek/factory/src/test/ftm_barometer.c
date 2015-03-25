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
#include <dirent.h>
#include <linux/input.h>
#include <math.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef CUSTOM_KERNEL_BAROMETER
#include <linux/sensors_io.h>
/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[BARO] "
#define mod_to_baro_data(p) (struct baro_data*)((char*)(p) + sizeof(struct ftm_module))
#define FBLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FBLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t barometer_items[] = {
    item(ITEM_PASS,   "Test Pass"),
    item(ITEM_FAIL,   "Test Fail"),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
struct baro_priv
{
    /*specific data field*/
    char    *dev;
    int     fd;
    float temp_raw;
    float press_raw;
};
/*---------------------------------------------------------------------------*/
struct baro_data
{
    struct baro_priv baro;

    /*common for each factory mode*/
    char  info[1024];
    //bool  avail;
    bool  exit_thd;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
    
    pthread_t update_thd;
    struct ftm_module *mod;
    //struct textview tv;
    struct itemview *iv;
};
/******************************************************************************
 * Functions 
 *****************************************************************************/
static int barometer_init_priv(struct baro_priv *baro)
{
    memset(baro, 0x00, sizeof(*baro));
    baro->fd = -1;
    baro->dev = "/dev/barometer";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int barometer_open(struct baro_priv *baro)
{
    int err, max_retry = 3, retry_period = 100, retry;
    unsigned int flags = 1;
    err = 0;
    if (baro->fd == -1) 
	{
          baro->fd = open("/dev/barometer", O_RDONLY);
          if (baro->fd < 0) 
		  {
            FBLOGE("Couldn't open '%s' (%s)", baro->dev, strerror(errno));
            return -1;
          }
		  //
		  if ((err = ioctl(baro->fd, BAROMETER_IOCTL_INIT, NULL))) 
		  {
             FBLOGE("read press : %d(%s)\n", errno, strerror(errno));
             return err;
         }
                
        
    }
    FBLOGD("%s() %d\n", __func__, baro->fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int barometer_close(struct baro_priv *baro)
{
    unsigned int flags = 0;
    int err;
    if (baro->fd != -1) 
	{
        close(baro->fd);
    }
    memset(baro, 0x00, sizeof(*baro));
    baro->fd = -1;
    baro->dev = "/dev/barometer";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int barometer_update_info(struct baro_priv *baro)
{
    int err = -EINVAL;
    int temp_dat, press_dat;
    if (baro->fd == -1) {
        FBLOGE("invalid fd\n");
        return -EINVAL;
    } else if ((err = ioctl(baro->fd, BAROMETER_GET_PRESS_DATA, &press_dat))) {
        FBLOGE("read press : %d(%s)\n", errno, strerror(errno));
        return err;
    } else if ((err = ioctl(baro->fd, BAROMETER_GET_TEMP_DATA, &temp_dat))) {
        FBLOGE("read temp: %d(%s)\n", errno, strerror(errno));
        return err;
    }
	
    baro->temp_raw = temp_dat;
    baro->press_raw = press_dat;
    return 0;
}
/*---------------------------------------------------------------------------*/
static void *barometer_update_iv_thread(void *priv)
{
    struct baro_data *dat = (struct baro_data *)priv; 
    struct baro_priv *baro = &dat->baro;
    struct itemview *iv = dat->iv;    
    int err = 0, len = 0;
    char *status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = barometer_open(baro))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, uistr_info_sensor_init_fail);
        iv->redraw(iv);
        FBLOGE("barometer() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }
        
    while (1) {
        
        if (dat->exit_thd)
            break;
            
        if ((err = barometer_update_info(baro)))
            continue;     

        len = 0;
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s: %f\n", uistr_info_sensor_temperature_value, baro->temp_raw/10);      
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s: %f\n", uistr_info_sensor_pressure_value, baro->press_raw/100);
        iv->set_text(iv, &dat->text);
        iv->redraw(iv);
    }
    barometer_close(baro);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);    
    pthread_exit(NULL);    
    return NULL;
}
/*---------------------------------------------------------------------------*/
int barometer_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct baro_data *dat = (struct baro_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    init_text(&dat->left_btn, "Fail", COLOR_YELLOW);
    init_text(&dat->center_btn, "Pass", COLOR_YELLOW);
    init_text(&dat->right_btn, "Back", COLOR_YELLOW);
       
    snprintf(dat->info, sizeof(dat->info), uistr_info_sensor_initializing);
    dat->exit_thd = false;  


    if (!dat->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        dat->iv = iv;
    }
    iv = dat->iv;
    iv->set_title(iv, &dat->title);
    iv->set_items(iv, barometer_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, barometer_update_iv_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                dat->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                dat->mod->test_result = FTM_TEST_FAIL;
            }           
            exit = true;            
            break;
        }
        
        if (exit) {
            dat->exit_thd = true;
            break;
        }        
    } while (1);
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int barometer_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct baro_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_BAROMETER, sizeof(struct baro_data));
    dat  = mod_to_baro_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    barometer_init_priv(&dat->baro);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, barometer_entry, (void*)dat);

    return ret;
}
#endif 

