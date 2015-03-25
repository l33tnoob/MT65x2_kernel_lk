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

#ifdef CUSTOM_KERNEL_ALSPS
#include <linux/sensors_io.h>

extern sp_ata_data return_data;


/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[LPS] "
#define mod_to_lps_data(p) (struct lps_data*)((char*)(p) + sizeof(struct ftm_module))
#define FLPLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FLPLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t alsps_items[] = {
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
struct lps_priv
{
    /*specific data field*/
    char    *dev;
    int     fd;
    unsigned int als_raw;
    unsigned int ps_raw;
    unsigned int ps_threshold_value;//yucong add for cust support
    unsigned int ps_threshold_high;//yucong add for cust support
    unsigned int ps_threshold_low;//yucong add for cust support
    unsigned int als_threshold_value;//yucong add for cust support
    unsigned int als_threshold_high;//yucong add for cust support
    unsigned int als_threshold_low;//yucong add for cust support
};
/*---------------------------------------------------------------------------*/
struct lps_data
{
    struct lps_priv lps;

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

static bool thread_exit = false;


/******************************************************************************
 * Functions 
 *****************************************************************************/
static int alsps_init_priv(struct lps_priv *lps)
{
    memset(lps, 0x00, sizeof(*lps));
    lps->fd = -1;
    lps->dev = "/dev/als_ps";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int alsps_open(struct lps_priv *lps)
{
    int err = 0, max_retry = 3, retry_period = 100, retry = 0;
    unsigned int flags = 1;
    if (lps->fd == -1) {
        lps->fd = open("/dev/als_ps", O_RDONLY);
        if (lps->fd < 0) {
            FLPLOGE("Couldn't open '%s' (%s)", lps->dev, strerror(errno));
            return -1;
        }
        retry = 0;
        while ((err = ioctl(lps->fd, ALSPS_SET_PS_MODE, &flags)) && (retry ++ < max_retry))
            usleep(retry_period*1000);
        if (err) {
            FLPLOGE("enable ps fail: %s", strerror(errno));
            return -1;            
        } 
        retry = 0;
        while ((err = ioctl(lps->fd, ALSPS_SET_ALS_MODE, &flags)) && (retry ++ < max_retry)) 
            usleep(retry_period*1000);
        if (err) {
            FLPLOGE("enable als fail: %s", strerror(errno));
            return -1;            
        }
    }
    FLPLOGD("%s() %d\n", __func__, lps->fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int alsps_close(struct lps_priv *lps)
{
    unsigned int flags = 0;
    int err = 0;
    if (lps->fd != -1) {
        if ((err = ioctl(lps->fd, ALSPS_SET_PS_MODE, &flags))) {
            FLPLOGE("disable ps fail: %s", strerror(errno));
            return -1;            
        } else if ((err = ioctl(lps->fd, ALSPS_SET_ALS_MODE, &flags))) {
            FLPLOGE("disable als fail: %s", strerror(errno));
            return -1;            
        }
        close(lps->fd);
    }
    memset(lps, 0x00, sizeof(*lps));
    lps->fd = -1;
    lps->dev = "/dev/als_ps";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int alsps_update_info(struct lps_priv *lps)
{
    int err = -EINVAL;
    int als_dat, ps_dat, ps_threshold_dat, ps_high, ps_low;//yucong add for factory mode test support
    if (lps->fd == -1) {
        FLPLOGE("invalid fd\n");
        return -EINVAL;
    }else if ((err = ioctl(lps->fd, ALSPS_GET_PS_RAW_DATA, &ps_dat))) {
        FLPLOGE("read ps  raw: %d(%s)\n", errno, strerror(errno));
        return err;
    }else if ((err = ioctl(lps->fd, ALSPS_GET_ALS_RAW_DATA, &als_dat))) {
        FLPLOGE("read als raw: %d(%s)\n", errno, strerror(errno));
        return err;
    }else if ((err = ioctl(lps->fd, ALSPS_GET_PS_TEST_RESULT, &ps_threshold_dat))) {//yucong add for factory mode test support
        FLPLOGE("get thresheld infr: %d(%s)\n", errno, strerror(errno));
	ps_threshold_dat = 2;
    } 
    if ((err = ioctl(lps->fd, ALSPS_GET_PS_THRESHOLD_HIGH, &ps_high))) {//yucong add for factory mode test support
        FLPLOGE("get thresheld high infr: %d(%s)\n", errno, strerror(errno));
	ps_high = 0;
    }
    if ((err = ioctl(lps->fd, ALSPS_GET_PS_THRESHOLD_LOW, &ps_low))) {//yucong add for factory mode test support
        FLPLOGE("get thresheld low infr: %d(%s)\n", errno, strerror(errno));
	ps_low = 0;
    }

    lps->als_raw = als_dat;
    lps->ps_raw = ps_dat;
    lps->ps_threshold_value = ps_threshold_dat;//yucong add for factory mode test support
    lps->ps_threshold_high = ps_high;//yucong add for factory mode test support
    lps->ps_threshold_low = ps_low;//yucong add for factory mode test support

	//add sensor data to struct sp_ata_data for PC side
	return_data.alsps.als = lps->als_raw;
	return_data.alsps.ps = lps->ps_raw;
	
    return 0;
}
/*---------------------------------------------------------------------------*/
static void *alsps_update_iv_thread(void *priv)
{
    struct lps_data *dat = (struct lps_data *)priv; 
    struct lps_priv *lps = &dat->lps;
    struct itemview *iv = dat->iv;    
    int err = 0, len = 0;
    char *status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = alsps_open(lps))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, uistr_info_sensor_init_fail);
        iv->redraw(iv);
        FLPLOGE("alsps() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }
        
    while (1) {
        
        if (dat->exit_thd)
            break;
            
        if ((err = alsps_update_info(lps)))
            continue;     

        len = 0;
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "ALS: %4Xh (0:dark; +:bright) \n", lps->als_raw);      
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "PS : %4Xh (0:far ; +:close) \n", lps->ps_raw);
	if(lps->ps_threshold_high == 0){
	len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s: %s\n", uistr_info_sensor_alsps_thres_high, uistr_info_sensor_alsps_check_command);//yucong add for factory mode test support
	}else{
	len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s: %4Xh\n", uistr_info_sensor_alsps_thres_high, lps->ps_threshold_high);	
	}
	if(lps->ps_threshold_low == 0){
	len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s: %s\n", uistr_info_sensor_alsps_thres_low, uistr_info_sensor_alsps_check_command);//yucong add for factory mode test support
	}else{
	len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s: %4Xh\n", uistr_info_sensor_alsps_thres_low, lps->ps_threshold_low);	
	}
	if(lps->ps_threshold_value == 2){
	len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s: %s\n", uistr_info_sensor_alsps_result, uistr_info_sensor_alsps_check_command);//yucong add for factory mode test support
	}else{
	len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s: %s\n", uistr_info_sensor_alsps_result, (lps->ps_threshold_value == 1)?uistr_info_sensor_pass:uistr_info_sensor_fail	);//yucong add for factory mode test support
	}
        iv->set_text(iv, &dat->text);
        iv->redraw(iv);
		int status = get_is_ata();
        if(status == 1)
        { 
            thread_exit = true;  
            break;
        }
    }
    alsps_close(lps);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);    
    pthread_exit(NULL);    
    return NULL;
}
/*---------------------------------------------------------------------------*/
int alsps_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    struct lps_data *dat = (struct lps_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    init_text(&dat->left_btn, uistr_info_sensor_fail, COLOR_YELLOW);
    init_text(&dat->center_btn, uistr_info_sensor_pass, COLOR_YELLOW);
    init_text(&dat->right_btn, uistr_info_sensor_back, COLOR_YELLOW);
       
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
    iv->set_items(iv, alsps_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, alsps_update_iv_thread, priv);
    do {
        if(get_is_ata() != 1){
        chosen = iv->run(iv, &thread_exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                dat->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                dat->mod->test_result = FTM_TEST_FAIL;
            }           
            thread_exit = true;          
            break;
        }
        }
        iv->redraw(iv);
        if (thread_exit) {
            dat->exit_thd = true;
            break;
        }        
    } while (1);
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int alsps_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct lps_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_ALSPS, sizeof(struct lps_data));
    dat  = mod_to_lps_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    alsps_init_priv(&dat->lps);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, alsps_entry, (void*)dat);

    return ret;
}
#endif 

