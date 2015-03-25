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

#ifdef CUSTOM_KERNEL_ACCELEROMETER
#include <linux/hwmsensor.h>
#include "libhwm.h"
/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[GSC] "
#define mod_to_gsc_data(p) (struct gsc_data*)((char*)(p) + sizeof(struct ftm_module))
#define GSCLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define GSCLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_CLEAR,
    ITEM_DO20,
    ITEM_DO40,
    ITEM_EXIT,
};
/*---------------------------------------------------------------------------*/
static item_t gs_cali_items[] = {
    item(ITEM_CLEAR,  uistr_info_sensor_cali_clear),
    item(ITEM_DO20,   uistr_info_sensor_cali_do_20),
    item(ITEM_DO40,   uistr_info_sensor_cali_do_40),
    item(ITEM_EXIT,   uistr_info_sensor_back),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
enum{
    GS_OP_NONE,
    GS_OP_CLEAR,
    GS_OP_CALI_PRE,
    GS_OP_CALI,
}; 
/*---------------------------------------------------------------------------*/
#define C_MAX_MEASURE_NUM (20)
/*---------------------------------------------------------------------------*/
struct gsc_priv
{
    pthread_mutex_t evtmutex;
    /*specific data field*/
   int fd;

    int  pending_op;
    int  cali_delay;
    int  cali_num;
    int  cali_tolerance;
    bool bUpToDate; 
    HwmData cali_drv;
    HwmData cali_nvram;
    HwmData dat;

    char status[1024];

    /*calculate statical information*/
    int statistics;  /*0: calculating; 1: done*/
    int measure_idx; 
    float raw[C_MAX_MEASURE_NUM][C_MAX_HWMSEN_EVENT_NUM];
    float std[C_MAX_HWMSEN_EVENT_NUM];
    float avg[C_MAX_HWMSEN_EVENT_NUM];
    float max[C_MAX_HWMSEN_EVENT_NUM];
    float min[C_MAX_HWMSEN_EVENT_NUM];
};
/*---------------------------------------------------------------------------*/
struct gsc_data
{
    struct gsc_priv gsc;

    /*common for each factory mode*/
    char  info[1024];
    bool  avail;
    bool  exit_thd;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
    
    pthread_t update_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
};
/******************************************************************************
 * Functions 
 *****************************************************************************/
static int gs_cali_init_priv(struct gsc_priv *gsc)
{
    memset(gsc, 0x00, sizeof(*gsc));
    return 0;
}
/*---------------------------------------------------------------------------*/
static int gs_cali_update_info(struct gsc_priv *gsc)
{
	int err = 0;
	if(!gsc->bUpToDate)
	{
		err = gsensor_read_nvram(&gsc->cali_nvram);
		if(err)
		{
			GSCLOGE("read nvram: %d(%s)\n", errno, strerror(errno));
		}
		else if((err = gsensor_get_cali(gsc->fd, &gsc->cali_drv)) != 0)
		{
			GSCLOGE("get calibration: %d(%s)\n", errno, strerror(errno));
		}
		else
		{
			gsc->bUpToDate = true;
		}        
	}

	err = gsensor_read(gsc->fd, &gsc->dat);
	if(err)
	{
		GSCLOGE("read: %d(%s)\n", errno, strerror(errno));
	}
	return err;
}

/*---------------------------------------------------------------------------*/
static int gs_cali_close(struct gsc_priv *gsc)
{
	int err;
	if(!gsc || (gsc->fd < 0))
	{
		return -EINVAL;
	}
	
	err = gsensor_close(gsc->fd);
	memset(gsc, 0x00, sizeof(*gsc));
	return err;
}
/*---------------------------------------------------------------------------*/
static void *gs_cali_update_iv_thread(void *priv)
{
	struct gsc_data *dat = (struct gsc_data *)priv; 
	struct gsc_priv *gsc = &dat->gsc;
	struct itemview *iv = dat->iv;
	int err = 0, len = 0;
	char *status;
	HwmData cali;
	static int op = -1;

	LOGD(TAG "%s: Start\n", __FUNCTION__);
	err = gsensor_open(&(gsc->fd));
	if(err)
	{
		memset(dat->info, 0x00, sizeof(dat->info));
		sprintf(dat->info, uistr_info_sensor_init_fail);
		iv->redraw(iv);
		GSCLOGE("gs_cali_open() err = %d(%s)\n", err, dat->info);
		pthread_exit(NULL);
		return NULL;
	}

	while(1)
	{

		if(dat->exit_thd)
		{
			break;
		}

		pthread_mutex_lock(&dat->gsc.evtmutex);
		if(op != dat->gsc.pending_op)
		{
			op = dat->gsc.pending_op;
			GSCLOGD("op: %d\n", dat->gsc.pending_op);
		}
		pthread_mutex_unlock(&dat->gsc.evtmutex);
		err = 0;

		if(op == GS_OP_CLEAR)
		{
			memset(&dat->gsc.cali_nvram, 0x00, sizeof(dat->gsc.cali_nvram));
			memset(&dat->gsc.cali_drv, 0x00, sizeof(dat->gsc.cali_drv));
			err = gsensor_rst_cali(gsc->fd);
			if(err)
			{
				GSCLOGE("rst calibration: %d\n", err);                
			}
			else if((err = gsensor_write_nvram(&dat->gsc.cali_nvram)) != 0)
			{
				GSCLOGE("write nvram: %d\n", err);                
			}

			if(err)
			{
				snprintf(dat->gsc.status, sizeof(dat->gsc.status), uistr_info_sensor_cali_fail);
				//dat->mod->test_result = FTM_TEST_FAIL;
			}
			else
			{
				snprintf(dat->gsc.status, sizeof(dat->gsc.status), uistr_info_sensor_cali_ok);
				//dat->mod->test_result = FTM_TEST_PASS;
			}

			gsc->bUpToDate = false;    
			pthread_mutex_lock(&dat->gsc.evtmutex);
			dat->gsc.pending_op = GS_OP_NONE;
			pthread_mutex_unlock(&dat->gsc.evtmutex);
		}
		else if(op == GS_OP_CALI_PRE)
		{
			err = 0;
			/*by-pass*/
			snprintf(dat->gsc.status, sizeof(dat->gsc.status), uistr_info_sensor_cali_ongoing);            
			pthread_mutex_lock(&dat->gsc.evtmutex);
			dat->gsc.pending_op = GS_OP_CALI;
			pthread_mutex_unlock(&dat->gsc.evtmutex);
		}
		else if(op == GS_OP_CALI)
		{
			if(!dat->gsc.cali_delay || !dat->gsc.cali_num || !dat->gsc.cali_tolerance)
			{
				GSCLOGE("ignore calibration: %d %d %d\n", dat->gsc.cali_delay, dat->gsc.cali_num, dat->gsc.cali_tolerance);                
			}
			else if((err = gsensor_calibration(gsc->fd, dat->gsc.cali_delay, dat->gsc.cali_num, 
			                          dat->gsc.cali_tolerance, 0, &cali)) != 0)
			{
				GSCLOGE("calibrate acc: %d\n", err);                
			}
			else if((err = gsensor_set_cali(gsc->fd, &cali)) != 0)
			{    
				GSCLOGE("set calibration fail: (%s) %d\n", strerror(errno), err);
			}
			else if((err = gsensor_get_cali(gsc->fd, &cali)) != 0)
			{    
				GSCLOGE("get calibration fail: (%s) %d\n", strerror(errno), err);
			}
			else if((err = gsensor_write_nvram(&cali)) != 0)
			{
				GSCLOGE("write nvram fail: (%s) %d\n", strerror(errno), err);
			}
			else
			{
				dat->gsc.cali_delay = dat->gsc.cali_num = dat->gsc.cali_tolerance = 0;
				dat->gsc.bUpToDate = false;
			}
			
			if(err)
			{
				len = snprintf(dat->gsc.status, sizeof(dat->gsc.status), uistr_info_sensor_cali_fail);  
				dat->mod->test_result = FTM_TEST_FAIL;
			}
			else
			{
				len = snprintf(dat->gsc.status, sizeof(dat->gsc.status), uistr_info_sensor_cali_ok);
				dat->mod->test_result = FTM_TEST_PASS;
			}

			pthread_mutex_lock(&dat->gsc.evtmutex);
			dat->gsc.pending_op = GS_OP_NONE;
			pthread_mutex_unlock(&dat->gsc.evtmutex);
		}

		err = gs_cali_update_info(gsc);
		if(err)
		{
			GSCLOGE("gs_cali_update_info() = (%s), %d\n", strerror(errno), err);
			break;
		} 


		len = 0;
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "R: %+7.4f %+7.4f %+7.4f\n", gsc->dat.x, gsc->dat.y, gsc->dat.z);
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "D: %+7.4f %+7.4f %+7.4f\n", gsc->cali_drv.x, gsc->cali_drv.y, gsc->cali_drv.z);
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "N: %+7.4f %+7.4f %+7.4f\n", gsc->cali_nvram.x, gsc->cali_nvram.y, gsc->cali_nvram.z);
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s\n", gsc->status);
		        
		iv->set_text(iv, &dat->text);
		iv->redraw(iv);
	}
	
	gs_cali_close(gsc);
	LOGD(TAG "%s: Exit\n", __FUNCTION__);    
	pthread_exit(NULL);

	return NULL;
}
/*---------------------------------------------------------------------------*/
int gs_cali_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct gsc_data *dat = (struct gsc_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err, op;

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
    iv->set_items(iv, gs_cali_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, gs_cali_update_iv_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        pthread_mutex_lock(&dat->gsc.evtmutex);
        op = dat->gsc.pending_op;
        pthread_mutex_unlock(&dat->gsc.evtmutex);        
        if ((chosen != ITEM_EXIT) && (op != GS_OP_NONE))   /*some OP is pending*/
            continue;
        switch (chosen) {
        case ITEM_CLEAR:
            pthread_mutex_lock(&dat->gsc.evtmutex);
            dat->gsc.pending_op = GS_OP_CLEAR;
            GSCLOGD("chosen clear: %d\n", dat->gsc.pending_op);
            pthread_mutex_unlock(&dat->gsc.evtmutex);
            break;
        case ITEM_DO20:
            pthread_mutex_lock(&dat->gsc.evtmutex);            
            dat->gsc.pending_op = GS_OP_CALI_PRE;
            dat->gsc.cali_delay = 50;
            dat->gsc.cali_num   = 20;
            dat->gsc.cali_tolerance = 20;            
            GSCLOGD("chosen DO20\n");
            pthread_mutex_unlock(&dat->gsc.evtmutex);
            break;
        case ITEM_DO40:
            pthread_mutex_lock(&dat->gsc.evtmutex);            
            dat->gsc.pending_op = GS_OP_CALI_PRE;
            dat->gsc.cali_delay = 50;
            dat->gsc.cali_num   = 20;
            dat->gsc.cali_tolerance = 40;            
            GSCLOGD("chosen DO40\n");
            pthread_mutex_unlock(&dat->gsc.evtmutex);
            break;
        case ITEM_EXIT:
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
int gs_cali_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct gsc_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_GS_CALI, sizeof(struct gsc_data));
    dat  = mod_to_gsc_data(mod);

    memset(dat, 0x00, sizeof(*dat));
	dat->gsc.fd = -1;
    gs_cali_init_priv(&dat->gsc);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, gs_cali_entry, (void*)dat);
//    if (!ret)
//        mod->visible = false;

    return ret;
}
#endif 

