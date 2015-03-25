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

#include "cust.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_LED

#define RED_LED_FILE		"/sys/class/leds/red/brightness"
#define GREEN_LED_FILE		"/sys/class/leds/green/brightness"
#define BLUE_LED_FILE		"/sys/class/leds/blue/brightness"
#define BUTTON_LED_FILE	    "/sys/class/leds/button-backlight/brightness"
#define KEYPAD_LED_FILE	    "/sys/class/leds/keypad-backlight/brightness"

bool led_test_exit = false;
static bool led_thread_exit = false;
static bool led_thread_start = false;


bool keypadled_test_exit = false;
static bool keypadled_thread_exit = false;
static bool keypadled_thread_start = false;

static pthread_t led_thread;
static pthread_t keypadled_thread;


#if defined (CUST_LED_HAVE_BUTTON_BACKLIGHT)
#define CUST_HAVE_KEYPAD_LED
#elif defined (CUST_LED_HAVE_KEYPAD_BACKLIGHT)
#define CUST_HAVE_KEYPAD_LED
#endif

static char *led_seq[] = {
#ifdef CUST_LED_HAVE_RED
	RED_LED_FILE,
#endif
#ifdef CUST_LED_HAVE_GREEN
	GREEN_LED_FILE,
#endif
#ifdef CUST_LED_HAVE_BLUE
	BLUE_LED_FILE,
#endif
	NULL
};

static char *keypadled_seq[] = {
#ifdef CUST_LED_HAVE_BUTTON_BACKLIGHT
	BUTTON_LED_FILE,
#endif
#ifdef CUST_LED_HAVE_KEYPAD_BACKLIGHT
	KEYPAD_LED_FILE,
#endif
	NULL
};


enum {
	ITEM_NLED_TEST,
	ITEM_KEYPAD_LED_TEST,
	ITEM_PASS,
	ITEM_FAIL,
};

static item_t items[] = {

	#ifndef CUST_HAVE_NLEDS
	item(ITEM_NLED_TEST, uistr_info_nled_test),
	#endif
		
	#ifdef CUST_HAVE_KEYPAD_LED
	item(ITEM_KEYPAD_LED_TEST,  uistr_info_keypad_led_test),
	#endif
	
	item(ITEM_PASS,   uistr_pass),
	item(ITEM_FAIL,   uistr_fail),
	item(-1, NULL),
};

extern int status;
static int
write_int(char const* path, int value)
{
	int fd;

	if (path == NULL)
		return -1;

	fd = open(path, O_RDWR);
	if (fd >= 0) {
		char buffer[20];
		int bytes = sprintf(buffer, "%d\n", value);
		int amt = write(fd, buffer, bytes);
		close(fd);
		if(amt == -1)
		{
			LOGE("write_int failed to write %s\n", path);
			return -errno;
		}
		else
		{
			//LOGD("write_int write %s:%d OK!\n", path, value);
			return 0;
		}
		//return amt == -1 ? -errno : 0;
	}

	LOGE("write_int failed to open %s\n", path);
	return -errno;
}

static void *update_led_thread(void *priv)
{
	int index = 0;
if(status == 0){

	// no LED for test
	if (led_seq[0] == NULL) {
		pthread_exit(NULL);
		return NULL;
	}

	LOGD("%s: Start\n", __FUNCTION__);
	led_thread_start = true;
	led_thread_exit = false;

	do {
		write_int(led_seq[index], 255);
		if (led_test_exit)
			break;
		sleep(1);
		if (led_test_exit)
			break;
		write_int(led_seq[index], 0);
		sleep(1);
	
		if (led_seq[++index] == NULL)
			index = 0;
		
	} while (1);

	
	// switch all leds off
	while(led_seq[index] != NULL)
		write_int(led_seq[index++], 0);
	
	led_thread_exit = true;
	led_thread_start = false;
	LOGD("%s: Exit\n", __FUNCTION__);
	pthread_exit(NULL);
}
else if(status == 1) {
	// no LED for test
	if (led_seq[0] == NULL) {
		pthread_exit(NULL);
		return NULL;
	}

	LOGD("%s: Start\n", __FUNCTION__);
	led_thread_start = true;
	led_thread_exit = false;

	do {
		write_int(led_seq[index], 255);
		if (led_test_exit)
			break;
		sleep(1);
	} while (1);

	
	// switch all leds off
	while(led_seq[index] != NULL)
		write_int(led_seq[index++], 0);
	
	led_thread_exit = true;
	led_thread_start = false;
	LOGD("%s: Exit\n", __FUNCTION__);
	pthread_exit(NULL);
}
	return NULL;
}

static void *update_keypadled_thread(void *priv)
{
	int index = 0;
	// no LED for test
	if (keypadled_seq[0] == NULL) {
	pthread_exit(NULL);
		return NULL;
	}

	LOGD("%s: Start\n", __FUNCTION__);
	keypadled_thread_start = true;
	keypadled_thread_exit = false;

	do {
		write_int(keypadled_seq[index], 255);
		if (keypadled_test_exit)
			break;
		sleep(1);
		if (keypadled_test_exit)
			break;
		write_int(keypadled_seq[index], 0);
		sleep(1);
		
		if (keypadled_seq[++index] == NULL)
			index = 0;
		
	} while (1);

	// switch all leds off
	while(keypadled_seq[index] != NULL)
		write_int(keypadled_seq[index++], 0);
	
	keypadled_thread_exit = true;
	keypadled_thread_start = false;
	LOGD("%s: Exit\n", __FUNCTION__);
	pthread_exit(NULL);

	return NULL;
}


int led_entry(struct ftm_param *param, void *priv)
{
	int chosen;
	bool exit = false;
	struct itemview *iv;
	text_t    title;
	struct ftm_module *mod = (struct ftm_module *)priv;
	static int nled_cnt = 0;
	static int keypadled_cnt = 0;

	LOGD("%s\n", __FUNCTION__);

	led_test_exit = false;
	keypadled_test_exit = false;

	iv = ui_new_itemview();
	if (!iv) {
		LOGD("No memory");
		return -1;
	}

	init_text(&title, param->name, COLOR_YELLOW);
	iv->set_title(iv, &title);
	iv->set_items(iv, items, 0);
if(status == 0){
	do {
		chosen = iv->run(iv, &exit);
		switch (chosen) {
			case ITEM_KEYPAD_LED_TEST:
				nled_cnt = 0;

				if(keypadled_cnt == 0)
				{
					if(led_thread_start)
					{
						led_test_exit = true;
						while(!led_thread_exit)
						{
							//msleep(500);
							LOGD("%s: sleep\n", __FUNCTION__);
							sleep(1);
						}
						led_test_exit = false;
					}
					
					pthread_create(&keypadled_thread, NULL, update_keypadled_thread, priv);
				}

				keypadled_cnt++;
				
				break;
				
			case ITEM_NLED_TEST:
				keypadled_cnt = 0;
				
				if(nled_cnt == 0)
				{
					if(keypadled_thread_start)
					{
						keypadled_test_exit = true;
						while(!keypadled_thread_exit)
						{
							//msleep(500);
							LOGD("%s: sleep\n", __FUNCTION__);
							sleep(1);
						}
						keypadled_test_exit = false;
					}
					
				pthread_create(&led_thread, NULL, update_led_thread, priv);
				}		
				
				nled_cnt++;
				
				break;
				
		case ITEM_PASS:
			mod->test_result = FTM_TEST_PASS;
			exit = true;
			break;
		case ITEM_FAIL:
			mod->test_result = FTM_TEST_FAIL;
			exit = true;
			break;
		default:
			//exit = true;
			break;
		}
		
		if (exit) {
			led_test_exit = true;
				keypadled_test_exit = true;
				nled_cnt = 0;
				keypadled_cnt = 0;
			break;
		}		
	} 
	while (1);
}
else if(status == 1)
{
	iv->start_menu(iv, 0);
	iv->redraw(iv);
	#ifdef CUST_HAVE_KEYPAD_LED
		nled_cnt = 0;

				if(keypadled_cnt == 0)
				{
					if(led_thread_start)
					{
						led_test_exit = true;
						while(!led_thread_exit)
						{
							//msleep(500);
							LOGD("%s: sleep\n", __FUNCTION__);
							sleep(1);
						}
						led_test_exit = false;
					}
					
					pthread_create(&keypadled_thread, NULL, update_keypadled_thread, priv);
				}

			//	keypadled_cnt++;
	#endif
	
	#ifndef CUST_HAVE_NLEDS
	
			keypadled_cnt = 0;
					
					if(nled_cnt == 0)
					{
						if(keypadled_thread_start)
						{
							keypadled_test_exit = true;
							while(!keypadled_thread_exit)
							{
								//msleep(500);
								LOGD("%s: sleep\n", __FUNCTION__);
								sleep(1);
							}
							keypadled_test_exit = false;
						}
						
					pthread_create(&led_thread, NULL, update_led_thread, priv);
					}		
					
				//	nled_cnt++;
		
	#endif
}
	
	pthread_join(led_thread, NULL);
	pthread_join(keypadled_thread, NULL);

	return 0;
}

int led_init(void)
{
	int index;
	int ret = 0;
	struct ftm_module *mod;

	LOGD("%s\n", __FUNCTION__);
	
	mod = ftm_alloc(ITEM_LED, sizeof(struct ftm_module));
	if (!mod)
		return -ENOMEM;

	// switch all leds off
	while(led_seq[index] != NULL)
		write_int(led_seq[index++], 0);

	ret = ftm_register(mod, led_entry, (void*)mod);

	return ret;
}

#endif // FEATURE_FTM_LED
