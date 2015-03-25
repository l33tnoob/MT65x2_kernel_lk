/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>

/* IOCTO */
#define ADC_CHANNEL_READ 		_IOW('k', 4, int)
#define BAT_STATUS_READ 		_IOW('k', 5, int)
#define Set_Charger_Current _IOW('k', 6, int)

/* ADC CHannel */

#define ADC_BAT_SEN 1

#define ADC_BAT_FG_CURRENT 66 // magic number

#define TAG     "ATCID"

/* Charging Current Setting */

typedef enum
{
	Cust_CC_70MA = 0,
	Cust_CC_200MA,
	Cust_CC_400MA,
	Cust_CC_450MA,
	Cust_CC_550MA,
	Cust_CC_650MA,
	Cust_CC_700MA,
	Cust_CC_800MA,
	Cust_CC_900MA,
	Cust_CC_1000MA,
	Cust_CC_1100MA,
	Cust_CC_1200MA,
	Cust_CC_1300MA,
	Cust_CC_1400MA,
	Cust_CC_1500MA,
	Cust_CC_1600MA,
    Cust_CC_0MA,   //add for close icharging
}cust_charging_current_enum;

#define USB_CHARGER_CURRENT		Cust_CC_450MA      
#define AC_CHARGER_CURRENT		Cust_CC_650MA
#define CUT_CHARGER_CURRENT	    Cust_CC_0MA

int ADC_COUNT = 5;

/********************************************
*											*
*	Battery AT COMMAND Related Functions	*
*											*
********************************************/
/*Input : current level*/ 
/*Output : none*/
/*please use set_Charger_Current(CUT_CHARGER_CURRENT) to cut the charging current */
int AT_set_Charger_Current(int charging_current_level)
{
	int fd = -1;
	int ret = 0;
	int current_level_in_data[1] = {0};
	current_level_in_data[0] = charging_current_level;
		

	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		ALOGD(TAG "get_ADC_channel - Can't open /dev/MT_pmic_adc_cali\n");
	}

  ret = ioctl(fd, Set_Charger_Current, current_level_in_data);
	close(fd);
	
	//ALOGD(TAG "Set_Charger_Current : %d\n", current_level_in_data[0]);

	return 0;
}

/*Input : ChannelNUm, Counts*/ 
/*Output : Sum, Result (0:success, 1:failed)*/
int get_ADC_channel(int adc_channel, int adc_count)
{
	int fd = -1;
	int ret = 0;
	int adc_in_data[2] = {1,1}; 
	
	
	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		ALOGD(TAG "get_ADC_channel - Can't open /dev/MT_pmic_adc_cali\n");
	}
	
	adc_in_data[0] = adc_channel;
	adc_in_data[1] = adc_count;
	ret = ioctl(fd, ADC_CHANNEL_READ, adc_in_data);
	close(fd);
	if (adc_in_data[1]==0) { 
		ALOGD(TAG "read channel[%d] %d times : %d\n", adc_channel, adc_count, adc_in_data[0]);		
		return adc_in_data[0];
	}
	
	return -1;
}

int AT_get_bat_voltage(void)
{	
    int temp=0;
    int battery_voltage = 0;
    
	temp = get_ADC_channel(ADC_BAT_SEN, ADC_COUNT); 
	
	if (temp != -1) {		
		battery_voltage = (temp/ADC_COUNT);	
	} 
	else {
	   battery_voltage = -1;
	}
    return battery_voltage;
}

int AT_get_charging_state_flag(void)
{	
    int temp=0;
    int battery_is_charging = 0;
    
	temp = get_ADC_channel(ADC_BAT_FG_CURRENT, ADC_COUNT); 
	
	if (temp < 0) {				
		battery_is_charging = 1;   //battery is charging
	} else {
		battery_is_charging = 0;   //battery is not charging
	}	
	return battery_is_charging;
}	


