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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/rtc.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>

#include <common.h>
#include <miniui.h>
#include <ftm.h>

#ifdef FEATURE_FTM_AUDIO
#include "ftm_audio_Common.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FEATURE_FTM_RTC

#define TAG	"[RTC] "

#define RTC_DEV_PATH	"/dev/rtc0"

enum {
	ITEM_PASS,
	ITEM_FAIL
};

/* for removing compiler warnings */
//static char str_pass[] = "Test Pass";
//static char str_fail[] = "Test Fail";
#define auto_mode 1

static item_t rtc_item[] = {
#ifdef auto_mode
	//auto mode
	{ -1, NULL, 0 },
#else	
	{ ITEM_PASS,   uistr_pass,   0 },
	{ ITEM_FAIL,   uistr_fail,   0 },
	{ -1, NULL, 0 },
#endif	
};

struct rtc {
	struct ftm_module *mod;
	int fd;

	pthread_t tm_pt;
	pthread_t alm_pt;
	bool tm_exit;
	bool alm_exit;

	text_t title;
	text_t text;
	char time[32];
	struct itemview *iv;
};

#define mod_to_rtc(p)	(struct rtc *)((char *)(p) + sizeof(struct ftm_module))

#define rtc_init_tm(tm, y, m, d, h, mi, s)	\
do {						\
	(tm)->tm_year = (y) - 1900;		\
	(tm)->tm_mon = (m) - 1;			\
	(tm)->tm_mday = (d);			\
	(tm)->tm_hour = (h);			\
	(tm)->tm_min = (mi);			\
	(tm)->tm_sec = (s);			\
} while (0)

#ifdef auto_mode
static volatile int Alarm_flag=0;
//static volatile int Time_flag=0;
#endif

static void *rtc_alm_pthread(void *priv)
{
	int r;
	unsigned long data;
	struct pollfd ev_fd;
	struct rtc *rtc = (struct rtc *)priv;
	struct rtc_time tm;

	ev_fd.fd = rtc->fd;
	ev_fd.events = POLLIN;
	poll(&ev_fd, 1, 4600);	/* 4600 ms */

	if (rtc->alm_exit)
		goto ret;

	r = read(rtc->fd, &data, sizeof(unsigned long));
	if (r < 0) {
		LOGD(TAG "read rtc data failed\n");
		goto ret;
	}

	if (data & RTC_AF) {
#ifdef auto_mode
		
		r = ioctl(rtc->fd, RTC_RD_TIME, &tm);
		if (r < 0) {
			LOGD(TAG "read rtc time failed\n");
			goto ret;
		}
		if (tm.tm_sec == 0 && tm.tm_min == 0 && tm.tm_hour == 0 &&
		    tm.tm_mday == 1 && tm.tm_mon == 0 && tm.tm_year == 109) {
			Alarm_flag=1;
		}
		else
			Alarm_flag=2;
		
#else		
		usleep(200000);		/* 200 ms */
#ifdef FEATURE_FTM_AUDIO
		LouderSPKTest(1, 1);
#endif
		/* alarm 3 secs */
		for (r = 0; r < 30; r++) {
			if (rtc->alm_exit)
				break;
			usleep(100000);		/* 100 ms */
		}
#ifdef FEATURE_FTM_AUDIO
		LouderSPKTest(0, 0);
#endif

#endif
	}

ret:
	pthread_exit(NULL);
	return NULL;
}

static void *rtc_tm_pthread(void *priv)
{
	int r;
	struct rtc_time tm;
	struct rtc *rtc = (struct rtc *)priv;
	struct itemview *iv = rtc->iv;

	rtc_init_tm(&tm, 2008, 12, 31, 23, 59, 57);
	r = ioctl(rtc->fd, RTC_SET_TIME, &tm);
	if (r < 0) {
		LOGD(TAG "set rtc time failed\n");
		goto ret;
	}

	while (1) {
		if (rtc->tm_exit)
			break;

		r = ioctl(rtc->fd, RTC_RD_TIME, &tm);
		if (r < 0) {
			LOGD(TAG "read rtc time failed\n");
			goto ret;
		}

		sprintf(rtc->time, "%d/%02d/%02d %02d:%02d:%02d\n",
		        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		        tm.tm_hour, tm.tm_min, tm.tm_sec);
		iv->set_text(iv, &rtc->text);
		iv->redraw(iv);

		
#ifdef auto_mode
		/* exit at 2009/01/01 00:00:00 */
		if (tm.tm_sec == 0 && tm.tm_min == 0 && tm.tm_hour == 0 &&
		    tm.tm_mday == 1 && tm.tm_mon == 0 && tm.tm_year == 109) {
			break;
		}
#else
		/* exit at 2009/01/01 00:00:03 */
		if (tm.tm_sec == 3 && tm.tm_min == 0 && tm.tm_hour == 0 &&
		    tm.tm_mday == 1 && tm.tm_mon == 0 && tm.tm_year == 109) {
			break;
		}
#endif		
		usleep(100000);		/* 100 ms */
	}

ret:
	pthread_exit(NULL);
	return NULL;
}

static int rtc_entry(struct ftm_param *param, void *priv)
{
	int r, num;
	bool exit=false;
	struct rtc_wkalrm alm;
	struct rtc *rtc = (struct rtc *)priv;
	struct itemview *iv = rtc->iv;

	rtc->fd = open(RTC_DEV_PATH, O_RDWR);
	if (rtc->fd < 0) {
		LOGD(TAG "open %s failed\n", RTC_DEV_PATH);
		return rtc->fd;
	}

#ifdef FEATURE_FTM_AUDIO
	if (!Common_Audio_init()) {
		LOGD(TAG "init audio failed\n");
		close(rtc->fd);
		return -1;
	}
#endif

	rtc_init_tm(&alm.time, 2008, 12, 29, 0, 0, 0);
	r = ioctl(rtc->fd, RTC_SET_TIME, &alm.time);
	if (r < 0) {
		LOGD(TAG "set rtc time failed\n");
		goto ret;
	}

	alm.enabled = 1;
	rtc_init_tm(&alm.time, 2009, 1, 1, 0, 0, 0);
	r = ioctl(rtc->fd, RTC_WKALM_SET, &alm);
	if (r < 0) {
		LOGD(TAG "enable rtc alarm failed\n");
		goto ret;
	}

	init_text(&rtc->title, param->name, COLOR_YELLOW);
	init_text(&rtc->text, rtc->time, COLOR_YELLOW);
	iv->set_title(iv, &rtc->title);
	iv->set_items(iv, rtc_item, ITEM_PASS);

	rtc->tm_exit = false;
	rtc->alm_exit = false;
	pthread_create(&rtc->tm_pt, NULL, rtc_tm_pthread, priv);
	pthread_create(&rtc->alm_pt, NULL, rtc_alm_pthread, priv);

	iv->start_menu(iv,0);

	while (1) {
#ifdef auto_mode
		if(Alarm_flag==1)
		{
			Alarm_flag=0;
			num = ITEM_PASS;
			if (num == ITEM_PASS) {
				rtc->mod->test_result = FTM_TEST_PASS;
				exit = true;
			}
		}
		else if(Alarm_flag==2)
		{
			Alarm_flag=0;
			num = ITEM_FAIL;
			if (num == ITEM_FAIL) {
				rtc->mod->test_result = FTM_TEST_FAIL;
				exit = true;
			}
		}
		if (exit) {
				rtc->tm_exit = true;
				rtc->alm_exit = true;
				break;
		}
#else		
		num = iv->run(iv, &exit);
		if (num == ITEM_PASS) {
			rtc->mod->test_result = FTM_TEST_PASS;
			exit = true;
		} else if (num == ITEM_FAIL) {
			rtc->mod->test_result = FTM_TEST_FAIL;
			exit = true;
		}

		if (exit) {
			rtc->tm_exit = true;
			rtc->alm_exit = true;
			break;
		}
#endif		
	}

	pthread_join(rtc->tm_pt, NULL);
	pthread_join(rtc->alm_pt, NULL);

	alm.enabled = 0;
	rtc_init_tm(&alm.time, 2009, 1, 1, 0, 0, 0);
	r = ioctl(rtc->fd, RTC_WKALM_SET, &alm);
	if (r < 0)
		LOGD(TAG "disable rtc alarm failed\n");

ret:
#ifdef FEATURE_FTM_AUDIO
	Common_Audio_deinit();
#endif
	close(rtc->fd);
	return r;
}

int rtc_init(void)
{
	int r;
	struct ftm_module *mod;
	struct rtc *rtc;

	mod = ftm_alloc(ITEM_RTC, sizeof(struct rtc));
	if (!mod)
		return -ENOMEM;

	rtc = mod_to_rtc(mod);
	rtc->mod = mod;
	rtc->iv = ui_new_itemview();
	if (!rtc->iv)
		return -ENOMEM;

	r = ftm_register(mod, rtc_entry, (void*)rtc);
	if (r) {
		LOGD(TAG "register RTC failed (%d)\n", r);
		return r;
	}

	return 0;
}

#endif

#ifdef __cplusplus
}
#endif
