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
#include <linux/sensors_io.h>

extern sp_ata_data return_data;

/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[ACC] "
#define mod_to_acc_data(p) (struct acc_data*)((char*)(p) + sizeof(struct ftm_module))
#define FTGLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FTGLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
#define	GSENSOR_NAME "/dev/gsensor"
#define GSENSOR_ATTR_SELFTEST "/sys/bus/platform/drivers/gsensor/selftest"
#define C_MAX_HWMSEN_EVENT_NUM 4
/*---------------------------------------------------------------------------*/
#define GRAVITY_EARTH           (9.80665f)
/*---------------------------------------------------------------------------*/
#define TOLERANCE_0G            (1.00)
#define TOLERANCE_1G            (9.5)
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t gsensor_items[] = {
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
typedef enum {
    TILT_UNKNOWN = 0,
    TILT_X_POS  = 1,
    TILT_X_NEG  = 2,
    TILT_Y_POS  = 3,
    TILT_Y_NEG  = 4,
    TILT_Z_POS  = 5,
    TILT_Z_NEG  = 6, 

    TILT_MAX,
}TILT_POS;
/*---------------------------------------------------------------------------*/
char *gsensor_pos[] = {
    uistr_info_g_sensor_unknow,
    "X+",
    "X-",
    "Y+",
    "Y-",
    "Z+",
    "Z-",
};
/*---------------------------------------------------------------------------*/
struct acc_evt {
    float x;
    float y;
    float z;
    //int64_t time;
};
/*---------------------------------------------------------------------------*/
#define C_MAX_MEASURE_NUM (20)
/*---------------------------------------------------------------------------*/
struct acc_priv
{
    /*specific data field*/
    char *ctl;
    char *dat;
    int fd;
    struct acc_evt evt;
    int  tilt;
    unsigned int pos_chk;

    /*self test*/
	int support_selftest;
    int selftest;   /*0: testing; 1: test ok; -1: test fail*/
    int avg_prv[C_MAX_HWMSEN_EVENT_NUM];
    int avg_nxt[C_MAX_HWMSEN_EVENT_NUM];

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
struct acc_data
{
    struct acc_priv acc;

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

/*---------------------------------------------------------------------------*/
/*
int gsensor_enable_selftest(int enable)
{ 
     LOGD(TAG " fwq 1\n");
	int err, fd = open(GSENSOR_ATTR_SELFTEST, O_RDWR);
	char buf[] = {enable + '0'};

	if(fd == -1)
	{
		FTGLOGD("open gsensor err = %s\n", strerror(errno));
		return -errno;
	}
	 LOGD(TAG " fwq 2\n");
	do{
		err = write(fd, buf, sizeof(buf) );
		LOGD(TAG " fwq 3\n");
	}while(err < 0 && errno == EINTR);
    
	if(err != sizeof(buf))
	{ 
		FTGLOGD("write fails = %s\n", strerror(errno));
		err = -errno;
	}
	else
	{  
		err = 0;    
	}
	
	if(close(fd) == -1)
	{
		FTGLOGD("close fails = %s\n", strerror(errno));
		err = (err) ? (err) : (-errno);
	}
	return err;
    
} 
*/
/*----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int gsensor_selftest(struct acc_priv *acc, int period, int count)
{
   
    int res = 0;
	int selftest=0;
	int fd=0;
    char buf[] = {"9"};
    char selftestRes[8];
	memset(selftestRes,0,sizeof(selftestRes));
	fd = open(GSENSOR_ATTR_SELFTEST, O_RDWR);
	if(fd == -1)
	{
		FTGLOGD("open selftest attr err = %s\n", strerror(errno));
		return -errno;
	}

    //enalbe selftest
	res = write(fd, buf, sizeof(buf));
	if(res <= 0)
	{
	   LOGD(" write attr failed\n");
	}
    sleep(1);

	lseek(fd, 0, SEEK_SET);
	res = read(fd,selftestRes,8);
	
	LOGD("fwq factory read selftestbuf = %s\n",selftestRes);
	
	
	if(res < 0)
	{
	  LOGD(" read attr failed\n");
	  LOGD("errno %d , %s",errno,strerror(errno));
	  perror("Read");
	}

	if('y' == selftestRes[0])
	{
	  LOGD("SELFTEST : PASS\n");
	  acc->selftest = 1;
	  
	}
    else
    {
      LOGD("SELFTEST : FAIL\n");
	  acc->selftest = -1;
    }
    res=0;
	if(close(fd) == -1)
	{
		FTGLOGD("close selftest attr fails = %s\n", strerror(errno));
		res = (res) ? (res) : (-errno);
	}
    sleep(1);   /*make sure g-sensor is back to normal*/
    return res;    
}

/*---------------------------------------------------------------------------*/
static int gsensor_statistic(struct acc_priv* acc)
{
    int idx;
    float diff;

    if (acc->statistics)
        return 0;

    /*record data*/
	if(acc->measure_idx < C_MAX_MEASURE_NUM)
	{
	   //FTGLOGD("init  acc->measure_idx =%d \n",acc->measure_idx );
       acc->raw[acc->measure_idx][0] = acc->evt.x;
       acc->raw[acc->measure_idx][1] = acc->evt.y;
       acc->raw[acc->measure_idx][2] = acc->evt.z;

       acc->avg[0] += acc->evt.x;
       acc->avg[1] += acc->evt.y;
       acc->avg[2] += acc->evt.z;
	}

    if (acc->measure_idx < C_MAX_MEASURE_NUM) {
        acc->measure_idx++;
        return 0;
    }

        
    for (idx = 0; idx < C_MAX_HWMSEN_EVENT_NUM; idx++) {
        acc->min[idx] = +100*GRAVITY_EARTH;
        acc->max[idx] = -100*GRAVITY_EARTH;
    }        
    acc->avg[0] /= C_MAX_MEASURE_NUM;
    acc->avg[1] /= C_MAX_MEASURE_NUM;
    acc->avg[2] /= C_MAX_MEASURE_NUM;
    
    for (idx = 0; idx < C_MAX_MEASURE_NUM; idx++) {
        diff = acc->raw[idx][0] - acc->avg[0];
        acc->std[0] += diff * diff;
        diff = acc->raw[idx][1] - acc->avg[1];
        acc->std[1] += diff * diff;
        diff = acc->raw[idx][2] - acc->avg[2];
        acc->std[2] += diff * diff;    
        if (acc->max[0] < acc->raw[idx][0])
            acc->max[0] = acc->raw[idx][0];
        if (acc->max[1] < acc->raw[idx][1])
            acc->max[1] = acc->raw[idx][1];
        if (acc->max[2] < acc->raw[idx][2])
            acc->max[2] = acc->raw[idx][2];
        
        if (acc->min[0] > acc->raw[idx][0])
            acc->min[0] = acc->raw[idx][0];
        if (acc->min[1] > acc->raw[idx][1])
            acc->min[1] = acc->raw[idx][1];
        if (acc->min[2] > acc->raw[idx][2])
            acc->min[2] = acc->raw[idx][2];

        FTGLOGD("[%2d][%+6.3f %+6.3f %+6.3f]\n", idx, acc->raw[idx][0], acc->raw[idx][1], acc->raw[idx][2]);
    }
    acc->std[0] = sqrt(acc->std[0]/C_MAX_MEASURE_NUM);
    acc->std[1] = sqrt(acc->std[1]/C_MAX_MEASURE_NUM);
    acc->std[2] = sqrt(acc->std[2]/C_MAX_MEASURE_NUM);
    acc->statistics = 1;
    return 0;
}

/******************************************************************************
 * common interface
 *****************************************************************************/
static int gsensor_open(struct acc_priv *acc)
{
	int fd = -1;
    if(acc->fd < 0)
	{
		acc->fd = open(GSENSOR_NAME, O_RDONLY);
	}

	fd = open(GSENSOR_ATTR_SELFTEST, O_RDWR);
	if(fd < 0)
	{
		acc->support_selftest = 0;
		FTGLOGE("gsensor not support self test!\n");
	}
	else
	{
		acc->support_selftest = 1;
		close(fd);
	}
	
	if(acc->fd < 0)
	{
		FTGLOGE("Couldn't open gsensor device!\n");
		return -EINVAL;
	}

	return 0;	
}
/*---------------------------------------------------------------------------*/
static int gsensor_close(struct acc_priv *acc)
{
    int i=0;
    if(acc->fd != 0)
    {
		close(acc->fd);
        acc->fd = -1;
	}
	acc->pos_chk = 0x00;
	acc->statistics=0;
	acc->support_selftest = 0;
	acc->selftest = 0;
	acc->measure_idx =0;
	for(i=0; i<C_MAX_HWMSEN_EVENT_NUM; i++)
	{
	  acc->max[i] = 0;
	  acc->min[i] = 0;
	  acc->avg[i] = 0;
	  acc->std[i] = 0;
	  
	}
	
    return 0;
}
/*---------------------------------------------------------------------------*/
static int gsensor_check_tilt(struct acc_priv *acc)
{
    if ((acc->evt.x  >  TOLERANCE_1G) && 
        (acc->evt.y  > -TOLERANCE_0G  &&  acc->evt.y  <  TOLERANCE_0G) &&
        (acc->evt.z  > -TOLERANCE_0G  &&  acc->evt.z  <  TOLERANCE_0G)) {
        acc->tilt = TILT_X_POS;
        return 0;
    }
    if ((acc->evt.x  < -TOLERANCE_1G) && 
        (acc->evt.y  > -TOLERANCE_0G  &&  acc->evt.y  <  TOLERANCE_0G) &&
        (acc->evt.z  > -TOLERANCE_0G  &&  acc->evt.z  <  TOLERANCE_0G)) {
        acc->tilt = TILT_X_NEG;
        return 0;
    }
    if ((acc->evt.y  >  TOLERANCE_1G) && 
        (acc->evt.x  > -TOLERANCE_0G  &&  acc->evt.x  <  TOLERANCE_0G) &&
        (acc->evt.z  > -TOLERANCE_0G  &&  acc->evt.z  <  TOLERANCE_0G)) {
        acc->tilt = TILT_Y_POS;
        return 0;
    }
    if ((acc->evt.y  < -TOLERANCE_1G) && 
        (acc->evt.x  > -TOLERANCE_0G  &&  acc->evt.x  <  TOLERANCE_0G) &&
        (acc->evt.z  > -TOLERANCE_0G  &&  acc->evt.z  <  TOLERANCE_0G)) {
        acc->tilt = TILT_Y_NEG;
        return 0;
    }
    if ((acc->evt.z  >  TOLERANCE_1G) && 
        (acc->evt.x  > -TOLERANCE_0G  &&  acc->evt.x  <  TOLERANCE_0G) &&
        (acc->evt.y  > -TOLERANCE_0G  &&  acc->evt.y  <  TOLERANCE_0G)) {
        acc->tilt = TILT_Z_POS;
        return 0;
    }
    if ((acc->evt.z  < -TOLERANCE_1G) && 
        (acc->evt.x  > -TOLERANCE_0G  &&  acc->evt.x  <  TOLERANCE_0G) &&
        (acc->evt.y  > -TOLERANCE_0G  &&  acc->evt.y  <  TOLERANCE_0G)) {
        acc->tilt = TILT_Z_NEG;
        return 0;
    }
    //snprintf(acc->tiltbuf, sizeof(acc->tiltbuf), "Tilt: unknown\n");
    acc->tilt = TILT_UNKNOWN; 
    return 0;
}
/******************************************************************************
 * Functions 
 *****************************************************************************/
static int gsensor_init_priv(struct acc_priv *acc)
{
    memset(acc, 0x00, sizeof(*acc));
    acc->fd = -1;
	acc->support_selftest = 0;
    acc->pos_chk = 0x00;  
    return 0;
}
/*---------------------------------------------------------------------------*/
static int gsensor_read(struct acc_priv *acc)
{
	static char buf[128];    
	int x, y, z, err;

	if(acc->fd == -1)
	{
		FTGLOGE("invalid file descriptor\n");
		err = -EINVAL;
	}
	else if((acc->support_selftest == 1) && (!acc->selftest) && (err = gsensor_selftest(acc, 10, 20)))
	{    
		FTGLOGE("selftest fail: %s(%d)\n", strerror(errno), errno);
	}
	else
	{
		err = ioctl(acc->fd, GSENSOR_IOCTL_READ_SENSORDATA, buf);
		if(err)
		{
			FTGLOGE("read data fail: %s(%d)\n", strerror(errno), errno);
		}
		else if(3 != sscanf(buf, "%x %x %x", &x, &y, &z))
		{
			FTGLOGE("read format fail: %s(%d)\n", strerror(errno), errno);
		}
		else
		{
			acc->evt.x = (float)(x)/1000;
			acc->evt.y = (float)(y)/1000;
			acc->evt.z = (float)(z)/1000;
			err = 0;
			gsensor_statistic(acc);

			//add sensor data to struct sp_ata_data for PC side
			return_data.gsensor.g_sensor_x = acc->evt.x;
			return_data.gsensor.g_sensor_y = acc->evt.y;
			return_data.gsensor.g_sensor_z = acc->evt.z;
			return_data.gsensor.accuracy = 3;
			
		}
	}
	return err;    
}

/*---------------------------------------------------------------------------*/
static void *gsensor_update_iv_thread(void *priv)
{
    struct acc_data *dat = (struct acc_data *)priv; 
    struct acc_priv *acc = &dat->acc;
    struct itemview *iv = dat->iv;    
    int err = 0, len = 0;
    char *status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = gsensor_open(acc))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, "INIT FAILED\n");
        iv->redraw(iv);
        FTGLOGE("gsensor_open() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }
        
    while (1) {
        
        if (dat->exit_thd){
            FTGLOGE("dat -> exit_thd\n");
            break;
        }    
        if ((err = gsensor_read(acc))) {
            FTGLOGE("gsensor_update_info() = (%s), %d\n", strerror(errno), err);
            break;
        } else if ((err = gsensor_check_tilt(acc))) {
            FTGLOGE("gsensor_check_tilt() = (%s), %d\n", strerror(errno), err);
            break;
        } else if (acc->tilt >= TILT_MAX) {    
            FTGLOGE("invalid tilt = %d\n", acc->tilt);
            break;        
        } else if (TILT_UNKNOWN != acc->tilt) {
            acc->pos_chk |= (1 << acc->tilt);
        }

		if(acc->support_selftest == 0)
		{
			status = uistr_info_g_sensor_notsupport;
		}
		else if(acc->selftest == 1)
		{
			status = uistr_info_sensor_pass;
		}
		else if(acc->selftest == 0)
		{
			status = uistr_info_g_sensor_testing;
		}
		else
		{
			status = uistr_info_sensor_fail;
		}
        len = 0;
        len = snprintf(dat->info+len, sizeof(dat->info)-len, "%+6.3f %+6.3f %+6.3f\n%s (%s)\n%s: %s %s: %s\n%s: %s %s: %s\n%s: %s %s: %s\n", 
                 acc->evt.x, acc->evt.y, acc->evt.z, 
                 (acc->tilt != TILT_UNKNOWN) ? (uistr_info_sensor_pass) : ("NG"), gsensor_pos[acc->tilt],
                 gsensor_pos[TILT_X_POS], (acc->pos_chk & (1 << TILT_X_POS)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing),
                 gsensor_pos[TILT_X_NEG], (acc->pos_chk & (1 << TILT_X_NEG)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing),
                 gsensor_pos[TILT_Y_POS], (acc->pos_chk & (1 << TILT_Y_POS)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing),
                 gsensor_pos[TILT_Y_NEG], (acc->pos_chk & (1 << TILT_Y_NEG)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing),
                 gsensor_pos[TILT_Z_POS], (acc->pos_chk & (1 << TILT_Z_POS)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing),
                 gsensor_pos[TILT_Z_NEG], (acc->pos_chk & (1 << TILT_Z_NEG)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing));

		if(len < 0)
		{
		   LOGE(TAG "%s: snprintf error \n", __FUNCTION__); 
		   len = 0;
		}

        if((acc->support_selftest == 1) && (acc->selftest != 0))
		{
            len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s %s\n", uistr_info_g_sensor_selftest, status);   
        }
		if(len < 0)
		{
		   LOGE(TAG "%s: snprintf error \n", __FUNCTION__); 
		   len = 0;
		}

        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s %s\n%s: %+6.3f %+6.3f %+6.3f\n%s: %+6.3f %+6.3f %+6.3f\n%s-%s: %+6.3f %+6.3f %+6.3f\n%s: %+6.3f %+6.3f %+6.3f\n%s: %+6.3f %+6.3f %+6.3f\n",
                 uistr_info_g_sensor_statistic,
                 (!acc->statistics) ? (uistr_info_g_sensor_doing) : (uistr_info_g_sensor_done),
                 uistr_info_g_sensor_max,
                 acc->max[0], acc->max[1], acc->max[2],
                 uistr_info_g_sensor_min,
                 acc->min[0], acc->min[1], acc->min[2],
                 uistr_info_g_sensor_max, uistr_info_g_sensor_min,
                 acc->max[0]-acc->min[0], acc->max[1]-acc->min[1], acc->max[2]-acc->min[2],
                 uistr_info_g_sensor_avg,
                 acc->avg[0], acc->avg[1], acc->avg[2],
                 uistr_info_g_sensor_std,
                 acc->std[0], acc->std[1], acc->std[2]);
		if(len < 0)
		{
		   LOGE(TAG "%s: snprintf error \n", __FUNCTION__); 
		   len = 0;
		}
        
        //len += snprintf(dat->info+len, sizeof(dat->info)-len, uistr_info_g_sensor_range);   
        iv->set_text(iv, &dat->text);
        iv->redraw(iv);
        int status = get_is_ata();
        if(status == 1)
        { 
            thread_exit = true;  
            break;
        }
    }
    gsensor_close(acc);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);    
    pthread_exit(NULL);
    
    return NULL;
}
/*---------------------------------------------------------------------------*/
int gsensor_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    struct acc_data *dat = (struct acc_data *)priv;
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
    iv->set_items(iv, gsensor_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, gsensor_update_iv_thread, priv);
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
int gsensor_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct acc_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_GSENSOR, sizeof(struct acc_data));
    dat  = mod_to_acc_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    gsensor_init_priv(&dat->acc);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, gsensor_entry, (void*)dat);

    return ret;
}
#endif 

