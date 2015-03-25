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

#define MPU3000_FS_MAX_LSB			131
#ifdef CUSTOM_KERNEL_GYROSCOPE
#include <linux/sensors_io.h>
#include <linux/hwmsensor.h>
#include "libhwm.h"

/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[Gyroscope]"
#define mod_to_gyro_data(p) (struct gyro_data*)((char*)(p) + sizeof(struct ftm_module))
#define FLPLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FLPLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)

#define MPU3000_AXIS_X          0
#define MPU3000_AXIS_Y          1
#define MPU3000_AXIS_Z          2
#define MPU3000_AXES_NUM        3
#define MPU3000_FIFOSIZE				512

/******************************************************************************
 * grobal variable
 *****************************************************************************/
int bias_thresh = 5242; // 40 dps * 131.072 LSB/dps
float RMS_thresh = 687.19f; // (.2 dps * 131.072) ^ 2

extern sp_ata_data return_data;

/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t gyro_items[] = {
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
struct gyro_priv
{
    /*specific data field*/
    char    *dev;
    int     fd;
    float gyro_x;
    float gyro_y;
    float gyro_z;
	HwmData dat;
    int resultcode;
	int smtflag;
	int smtRes;
};
/*---------------------------------------------------------------------------*/
struct gyro_data
{
    struct gyro_priv gyro;

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


static bool thread_exit = false;


/******************************************************************************
 * Functions 
 *****************************************************************************/
static int gyro_init_priv(struct gyro_priv *gyro)
{
    memset(gyro, 0x00, sizeof(*gyro));
    gyro->fd = -1;
    gyro->dev = "/dev/gyroscope";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int gyro_open(struct gyro_priv *gyro)
{
    int err, max_retry = 3, retry_period = 100, retry;
    unsigned int flags = 1;
    if (gyro->fd == -1) {
        gyro->fd = open("/dev/gyroscope", O_RDONLY);
        if (gyro->fd < 0) {
            FLPLOGE("Couldn't open '%s' (%s)", gyro->dev, strerror(errno));
            return -1;
        }
        retry = 0;
        while ((err = ioctl(gyro->fd, GYROSCOPE_IOCTL_INIT, &flags)) && (retry ++ < max_retry))
            usleep(retry_period*1000);
        if (err) {
            FLPLOGE("enable gyro fail: %s", strerror(errno));
            return -1;            
        }          
    }
    FLPLOGD("%s() %d\n", __func__, gyro->fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int gyro_close(struct gyro_priv *gyro)
{
    unsigned int flags = 0;
    int err;
    if (gyro->fd != -1) {
        close(gyro->fd);
    }
    memset(gyro, 0x00, sizeof(*gyro));
    gyro->fd = -1;
    gyro->dev = "/dev/gyroscope";
    return 0;
}

/*---------------------------------------------------------------------------*/
static int gyro_update_info(struct gyro_priv *gyro)
{
    //short data[800];
    //int smtRes=-1;
    int i;
    int retval =0;
    int err = -EINVAL;
	char buf[64];
	int x,y,z=0;
	int smtflag=0;
		
    if (gyro->fd == -1) {
        FLPLOGE("invalid fd\n");
        return err;
    }
 
	
	if(1!= gyro->smtflag)
	{
	   err = ioctl(gyro->fd, GYROSCOPE_IOCTL_SMT_DATA, &(gyro->smtRes));
	   if(err)
	   {
        FLPLOGE("read gyro data failed: %d(%s)\n", errno, strerror(errno));
        return err;
       }
       gyro->smtflag =1;
	}
    
    err = ioctl(gyro->fd, GYROSCOPE_IOCTL_READ_SENSORDATA, buf);
	if(err)
	{
        FLPLOGE("read gyro data failed: %d(%s)\n", errno, strerror(errno));
        return err;
    }
	sscanf(buf, "%x %x %x", &x, &y, &z);
	FLPLOGE("read data: x=%d, y=%d, z=%d, \n", x,  y,  z);
	
	
	gyro->gyro_x = ( (float)x / MPU3000_FS_MAX_LSB);
	gyro->gyro_y = ( (float)y / MPU3000_FS_MAX_LSB);
	gyro->gyro_z = ( (float)z / MPU3000_FS_MAX_LSB);
	gyro->resultcode = gyro->smtRes;
	FLPLOGE("read gyro data OK: x=%f, y=%f, z=%f, resultcode=0x%4x\n", 
		gyro->gyro_x, gyro->gyro_y, gyro->gyro_z, gyro->resultcode);

	//add sensor data to struct sp_ata_data for PC side
	return_data.gyroscope.gyroscope_x = gyro->gyro_x;
	return_data.gyroscope.gyroscope_x = gyro->gyro_x;
	return_data.gyroscope.gyroscope_x = gyro->gyro_x;
	return_data.gyroscope.accuracy= 3;
	
    return 0;
}
/*---------------------------------------------------------------------------*/
static void *gyro_update_iv_thread(void *priv)
{
    struct gyro_data *dat = (struct gyro_data *)priv; 
    struct gyro_priv *gyro = &dat->gyro;
    struct itemview *iv = dat->iv;    
    int err = 0;
    char *status;

    FLPLOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = gyro_open(gyro))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, uistr_info_sensor_init_fail);
        iv->redraw(iv);
        FLPLOGE("gyro() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }
        
    while (1) {
        
        if (dat->exit_thd)
            break;
            
        if ((err = gyro_update_info(gyro))){
	   FLPLOGE("gyro_update_info() = (%s), %d\n", strerror(errno), err);
            break;
        }
        FLPLOGD("MPU3000 gyro_update_info OK\n"); 
	
        snprintf(dat->info, sizeof(dat->info), "X: %+7.4f \nY: %+7.4f \nZ: %+7.4f \n", 
			gyro->gyro_x, gyro->gyro_y, gyro->gyro_z);
        
		iv->set_text(iv, &dat->text);
        iv->redraw(iv);
		int status = get_is_ata();
        if(status == 1)
        { 
            thread_exit = true;  
            break;
        }

    }
    gyro_close(gyro);
    FLPLOGD(TAG "%s: Exit\n", __FUNCTION__);    
    pthread_exit(NULL);    
    return NULL;
}
/*---------------------------------------------------------------------------*/
int gyro_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    struct gyro_data *dat = (struct gyro_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err;

    FLPLOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    init_text(&dat->left_btn, uistr_info_sensor_fail, COLOR_YELLOW);
    init_text(&dat->center_btn, uistr_info_sensor_pass, COLOR_YELLOW);
    init_text(&dat->right_btn, uistr_info_sensor_back, COLOR_YELLOW);
       
    snprintf(dat->info, sizeof(dat->info),  uistr_info_sensor_initializing);
    dat->exit_thd = false;  


    if (!dat->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            FLPLOGD(TAG "No memory");
            return -1;
        }
        dat->iv = iv;
    }
    iv = dat->iv;
    iv->set_title(iv, &dat->title);
    iv->set_items(iv, gyro_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, gyro_update_iv_thread, priv);
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
int gyro_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct gyro_data *dat;

    FLPLOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_GYROSCOPE, sizeof(struct gyro_data));
    dat  = mod_to_gyro_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    gyro_init_priv(&dat->gyro);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, gyro_entry, (void*)dat);

    return ret;
}
#endif 

