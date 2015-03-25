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

#ifdef CUSTOM_KERNEL_MAGNETOMETER
#include <linux/sensors_io.h>


extern sp_ata_data return_data;

/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[MAG] "
#define mod_to_mag_data(p) (struct mag_data*)((char*)(p) + sizeof(struct ftm_module))
#define FTMLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FTMLOGE(fmt, arg ...) LOGE(TAG fmt, ##arg)
#define MSENSOR_NAME	"/dev/msensor"
#define C_MAX_HWMSEN_EVENT_NUM 4
/*---------------------------------------------------------------------------*/
#define MAG_VAL_MAX (2047)
#define MAG_VAL_MIN (-2048)
#define MAG_MIN_DIFF (64)   /*the minimum difference for passing the test*/
#define MAG_TEST_OK ((char*)"OK")
#define MAG_TESTING ((char*)"Testing")
#define MSENSOR_ATTR_SHIPMENT "/sys/bus/platform/drivers/msensor/shipmenttest"
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t msensor_items[] = {
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
struct mag_evt {
    int x;
    int y;
    int z;
	int yaw;
	int pitch;
	int roll;
	unsigned int status;
	unsigned int div;
    //int64_t time;
};

/*---------------------------------------------------------------------------*/
#define C_MAX_MEASURE_NUM (40)
/*---------------------------------------------------------------------------*/
struct mag_priv
{
    /*specific data field*/
    char *ctl;
    char *dat;
    int  fd;
    struct mag_evt evt;
    int     x_max;
    int     x_min;
    char   *x_sta;
    int     y_max;
    int     y_min;
    char   *y_sta;
    int     z_max;
    int     z_min;
    char   *z_sta;
    /*statistics information*/
    int statistics;
    int measure_idx; 
    int max[C_MAX_HWMSEN_EVENT_NUM];
    int min[C_MAX_HWMSEN_EVENT_NUM];
    int raw[C_MAX_MEASURE_NUM][C_MAX_HWMSEN_EVENT_NUM];
    float std[C_MAX_HWMSEN_EVENT_NUM];
    int avg[C_MAX_HWMSEN_EVENT_NUM];        
	int support_shipment_test;
	int shipment_test;
};
/*---------------------------------------------------------------------------*/
struct mag_data
{
    struct mag_priv mag;   
    
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
static void msensor_init_priv(struct mag_priv *mag);

/******************************************************************************
 * common interface
 *****************************************************************************/
static int msensor_open(struct mag_priv *mag)
{
    int fd = -1;
	int err =0;
	unsigned int enable = 1;
	if(mag->fd < 0)
	{
		mag->fd = open(MSENSOR_NAME, O_RDONLY);
	}
	
	if(mag->fd < 0)
	{
		FTMLOGE("Couldn't open msensor device!\n");
		return -EINVAL;
	}

	fd = open(MSENSOR_ATTR_SHIPMENT, O_RDWR);
	if(fd < 0)
	{
		mag->support_shipment_test= 0;
		FTMLOGE("msensor not support shipment test!\n");
	}
	else
	{
		mag->support_shipment_test= 1;
		close(fd);
	}

	 //enalbe msensor lib
	err = ioctl(mag->fd, MSENSOR_IOCTL_SENSOR_ENABLE, &enable);
    if(err)
	{
		FTMLOGE("enalbe msensor lib fail: %s(%d)\n", strerror(errno), errno);
	}

	return 0;	
}

/*---------------------------------------------------------------------------*/
static int msensor_close(struct mag_priv *mag)
{
    int err =0;
	unsigned int enable = 0;
    if(mag->fd != 0)
    {
		close(mag->fd);
        mag->fd = -1;
	}

	msensor_init_priv(mag);
	
	err = ioctl(mag->fd, MSENSOR_IOCTL_SENSOR_ENABLE, &enable);
    if(err)
	{
		FTMLOGE("enalbe msensor lib fail: %s(%d)\n", strerror(errno), errno);
	}
    return 0;
}

int msensor_shipment_test(struct mag_priv *mag)
{
   
    int res = 0;
	int selftest=0;
	int fd=0;
    char buf[] = {"9"};
    char selftestRes[8];
	memset(selftestRes,0,sizeof(selftestRes));
	fd = open(MSENSOR_ATTR_SHIPMENT, O_RDWR);
	if(fd == -1)
	{
		FTMLOGD("open shipmenttest attr err = %s\n", strerror(errno));
		mag->support_shipment_test = 0;
		return -errno;
	}
	mag->support_shipment_test = 1;

    //enalbe selftest
    /*
	res = write(fd, buf, sizeof(buf));
	if(res <= 0)
	{
	   LOGD(" write attr failed\n");
	}
    sleep(1);

	lseek(fd, 0, SEEK_SET);
	*/
	res = read(fd,selftestRes,8);
	
	LOGD("factory read shipmenttestbuf = %s\n",selftestRes);
	
	
	if(res < 0)
	{
	  LOGD(" read attr failed\n");
	  LOGD("errno %d , %s",errno,strerror(errno));
	  perror("Read");
	}

	if('y' == selftestRes[0])
	{
	  LOGD("shipment test : PASS\n");
	  mag->shipment_test= 1;
	  
	}
    else
    {
      LOGD("shipment test : FAIL\n");
	  mag->shipment_test= -1;
    }
    res=0;
	if(close(fd) == -1)
	{
		FTMLOGD("close shipment test attr fails = %s\n", strerror(errno));
		res = (res) ? (res) : (-errno);
	}
    sleep(1);   /*make sure g-sensor is back to normal*/
    return res;    
}

static int msensor_read(struct mag_priv *mag)
{
    static char buf[512];
    int x, y, z,err;
	unsigned int div;
	unsigned int status;
	unsigned int enable = 0;
	
	int res=0;
	if(1== mag->support_shipment_test && !(mag->shipment_test))
	{
	  FTMLOGD("shipment test...\n");
	  enable = 0;
	  ioctl(mag->fd, MSENSOR_IOCTL_SENSOR_ENABLE, &enable);
	  sleep(1);
	  msensor_shipment_test(mag);
	  sleep(1);
	  enable =1;
	  err = ioctl(mag->fd, MSENSOR_IOCTL_SENSOR_ENABLE, &enable);
	}

	if(mag->fd == -1)
	{
		FTMLOGE("invalid file descriptor\n");
		err = -EINVAL;
	}
	else
	{  
	    //get orientation data
		err = ioctl(mag->fd, MSENSOR_IOCTL_READ_FACTORY_SENSORDATA, buf);
		if(err)
		{
			FTMLOGE("read data fail: %s(%d)\n", strerror(errno), errno);
		}
		res= sscanf(buf, "%x %x %x %x %x", &x, &y, &z,&status,&div);
		if(5!= res)
		{
			FTMLOGE("read format fail: %s(%d)\n", strerror(errno), errno);
		}
		else
		{
			mag->evt.status = status;
			mag->evt.div = div;
			mag->evt.yaw = (float)x/(float)div;
			mag->evt.pitch = (float)y/(float)div;
			mag->evt.roll = (float)z/(float)div;
			err =0;
			FTMLOGE(" (  %d %d %d %d %d)\n",mag->evt.x,mag->evt.y,mag->evt.z 
				,mag->evt.status,mag->evt.div);

			//add sensor data to struct sp_ata_data for PC side
			return_data.msensor.accuracy = mag->evt.status;
		}
		//get msensor raw
		err = ioctl(mag->fd, MSENSOR_IOCTL_READ_SENSORDATA, buf);
		if(err)
		{
			FTMLOGE("read data fail: %s(%d)\n", strerror(errno), errno);
		}
		res= sscanf(buf, "%x %x %x", &x, &y, &z);
		if(3!= res)
		{
			FTMLOGE("read format fail: %s(%d)\n", strerror(errno), errno);
		}
		else
		{
			mag->evt.x = x;
			mag->evt.y = y;
			mag->evt.z = z;
			err =0;

			//add sensor data to struct sp_ata_data for PC side
			return_data.msensor.m_sensor_x = mag->evt.x;
			return_data.msensor.m_sensor_y = mag->evt.y;
			return_data.msensor.m_sensor_z = mag->evt.z;	
		}
		
		
	}
	return err;    
}

/*---------------------------------------------------------------------------*/
static void msensor_init_priv(struct mag_priv *mag)
{
    memset(mag, 0x00, sizeof(*mag));
    mag->fd = -1;    
    mag->x_max  = MAG_VAL_MIN;
    mag->x_min  = MAG_VAL_MAX;
    mag->x_sta  = NULL;
    mag->y_max  = MAG_VAL_MIN;
    mag->y_min  = MAG_VAL_MAX;
    mag->y_sta  = NULL;
    mag->z_max  = MAG_VAL_MIN;
    mag->z_min  = MAG_VAL_MAX;
    mag->z_sta  = NULL;   
	mag->support_shipment_test =0;
	mag->shipment_test = 0;
}
 
/*---------------------------------------------------------------------------*/
static void *msensor_update_iv_thread(void *priv)
{
    struct mag_data *dat = (struct mag_data *)priv;
    struct mag_priv *mag = &dat->mag;
    struct itemview *iv = dat->iv;    
    int err = 0, len;
	char* status=0;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
	status = malloc(sizeof(char)*128);
	
    if ((err = msensor_open(mag))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, "INIT FAILED\n");  
    	iv->redraw(iv); /*force to print the log*/
        FTMLOGD("msensor_open() err = %d (%s)\n", err, dat->info);
        pthread_exit(NULL);
        free(status);
        return NULL;
    }
        
    while(1)
	{
        
        if (dat->exit_thd)
            break;
            
        if ((err = msensor_read(mag))) {
            FTMLOGE("msensor_update_info() = (%s), %d\n", strerror(errno), err);
            break;
        }
       
		if(mag->support_shipment_test == 0)
		{
			status = uistr_info_m_sensor_notsupport;
		}
		else if(mag->shipment_test== 1)
		{
			status = uistr_info_m_sensor_ok;
		}
		else if(mag->shipment_test == 0)
		{
			status = uistr_info_m_sensor_testing;
		}
		else
		{
			status = uistr_info_m_sensor_fail;
		}
		
        len = 0;    
		len += snprintf(dat->info+len, sizeof(dat->info)+len,  "%s: %s \n",uistr_info_m_sensor_self, status);
		len += snprintf(dat->info+len, sizeof(dat->info)+len, "%s: %d\n",uistr_info_m_sensor_status, mag->evt.status);
		//len += snprintf(dat->info+len, sizeof(dat->info)+len, "Yaw: %d\nPietch: %d\nRoll: %d\n",
		//	mag->evt.yaw,mag->evt.pitch,mag->evt.roll);
		len += snprintf(dat->info+len, sizeof(dat->info)+len, "%s: \n", uistr_info_m_sensor_data);
		len += snprintf(dat->info+len, sizeof(dat->info)+len, "X: %d\nY: %d\nZ: %d\n",
			mag->evt.x,mag->evt.y,mag->evt.z);

        iv->set_text(iv, &dat->text);
        iv->redraw(iv);

		int status = get_is_ata();
        if(status == 1)
        { 
            thread_exit = true;  
            break;
        }
    }
    

    //free operation remove
    msensor_close(mag);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);    
    pthread_exit(NULL);
    free(status);
    return NULL;
}
/*---------------------------------------------------------------------------*/
int msensor_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    struct mag_data *dat = (struct mag_data *)priv;
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
       
    snprintf(dat->info, sizeof(dat->info), "Initializing...\n");
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
    iv->set_items(iv, msensor_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, msensor_update_iv_thread, priv);
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
int msensor_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct mag_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_MSENSOR, sizeof(struct mag_data));
    dat = mod_to_mag_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    msensor_init_priv(&dat->mag);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, msensor_entry, (void*)dat);

    return ret;
}
#endif 

