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

#include <stdio.h>
#include <utils/Log.h>
#include "ipodmain.h"

#define LED_RED_PATH "/sys/class/leds/red/brightness"
#define LED_GREEN_PATH "/sys/class/leds/green/brightness"
#define LED_ONOFF_PROP  "sys.ipo.ledon"


/*	
int (*ipod_chglights[])(void) = {
	lights_init,
	lights_chgfull,
	lights_chgon,
	lights_chgexit,
};
*/

static int led_isOn = CONTROL_UNKNOWN;
static int led_green_isOn = CONTROL_UNKNOWN;
static int led_red_isOn = CONTROL_UNKNOWN;
static int nChargingFull = CONTROL_UNKNOWN;
static int nChargingRemoved = CONTROL_UNKNOWN;

void turn_led_on() 
{
	if (led_isOn == CONTROL_ON)
		return;
	led_isOn = CONTROL_ON;

	property_set(LED_ONOFF_PROP, "1");
}

void turn_led_off() 
{
	if (led_isOn == CONTROL_OFF)
		return;
	led_isOn = CONTROL_OFF;

	property_set(LED_ONOFF_PROP, "0");
}

void start_red_led(int skew)
{
	if (led_red_isOn == CONTROL_ON)
		return;
	led_red_isOn = CONTROL_ON;

	if (skew > 255) skew = 255;
	else if (skew < 0) skew = 0;
	
	set_int_value(LED_RED_PATH, skew);
}

void stop_red_led()
{
	if (led_red_isOn == CONTROL_OFF)
		return;
	led_red_isOn = CONTROL_OFF;
	set_int_value(LED_RED_PATH, 0);
}

void start_green_led(int skew)
{
	if (led_green_isOn == CONTROL_ON)
		return;
	led_green_isOn = CONTROL_ON;

	if (skew > 255) skew = 255;
	else if (skew < 0) skew = 0;
	
	set_int_value(LED_GREEN_PATH, skew);
}

void stop_green_led()
{
	if (led_green_isOn == CONTROL_OFF)
		return;
	led_green_isOn = CONTROL_OFF;

	set_int_value(LED_GREEN_PATH, 0);
}

int lights_chgfull()
{
	/* do not block this function like sleep */
	SXLOGI("lights_chgfull");

	pthread_mutex_lock(&mutex);
	nChargingFull = CONTROL_ON;
	nChargingRemoved = CONTROL_OFF;

	stop_red_led();
	start_green_led(255);
	turn_led_on();
	pthread_mutex_unlock(&mutex);
	
	return 0;
}

int lights_chgon()
{
	SXLOGI("lights_chgon");

#if 1
	pthread_mutex_lock(&mutex);
	nChargingFull = CONTROL_OFF;
	nChargingRemoved = CONTROL_OFF;

	stop_green_led();
	start_red_led(255);
	turn_led_on();
	pthread_mutex_unlock(&mutex);
#else
	// an example of light on 2sec, off 3sec.
	pthread_mutex_lock(&mutex);
	if (nChargingFull != CONTROL_ON && nChargingRemoved != CONTROL_ON)
	{
		nChargingFull = CONTROL_OFF;
		nChargingRemoved = CONTROL_OFF;
		stop_green_led();
		start_red_led(255);
		turn_led_on();
	}
	else
	{
		nChargingFull = CONTROL_OFF;
		nChargingRemoved = CONTROL_OFF;
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	pthread_mutex_unlock(&mutex);
		
	usleep(2000*1000); //sleep 2 sec

	pthread_mutex_lock(&mutex);
	if (nChargingFull != CONTROL_ON && nChargingRemoved != CONTROL_ON)
	{
		stop_red_led();
		turn_led_off();
	}

	// reset the state to let next time chgon can work normally.
	nChargingFull = CONTROL_OFF;
	nChargingRemoved = CONTROL_OFF;
	pthread_mutex_unlock(&mutex);
#endif 
	return 0;
}

int lights_chgexit()
{
	/* do not block this function like sleep */
	SXLOGI("lights_chgexit");

	pthread_mutex_lock(&mutex);
	nChargingFull = CONTROL_OFF;
	nChargingRemoved = CONTROL_ON;

	stop_green_led();
	stop_red_led();
	turn_led_off();
	pthread_mutex_unlock(&mutex);

	return 0;
}

