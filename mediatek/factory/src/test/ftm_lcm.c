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
#include <linux/mtkfb.h>
#include "cust.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_LCM

bool lcm_test_exit = false;

enum {
	ITEM_PASS,
	ITEM_FAIL
};

static item_t items_pass[] = {
	item(ITEM_PASS,   uistr_pass),
	item(-1, NULL),
};
static item_t items_fail[] = {
	item(ITEM_FAIL,   uistr_fail),
	item(-1, NULL),
};

static unsigned int update_lcm()
{
		int fd;
		unsigned int ret;
    fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd < 0) {
        perror("cannot open fb0");
        return 0;
    }

	LOGD("%s: Start\n", __FUNCTION__);

		if (ioctl(fd, MTKFB_FACTORY_AUTO_TEST, &ret) < 0) {
        perror("failed to get call FB0 ioctl:MTKFB_FACTORY_AUTO_TEST");
        close(fd);
        return 0;
    }
//		sleep(1);
	LOGD("%s: Exit\n", __FUNCTION__);
	close(fd);
	return ret;
}

int lcm_entry(struct ftm_param *param, void *priv)
{
	int chosen;
	bool exit = false;
	unsigned int ret = 0;
	struct itemview *iv;
	text_t    title;
	struct ftm_module *mod = (struct ftm_module *)priv;

	LOGD("%s\n", __FUNCTION__);

	lcm_test_exit = false;
	ret = update_lcm();
	iv = ui_new_itemview();
	if (!iv) {
		LOGD("No memory");
		return -1;
	}

  init_text(&title, param->name, COLOR_YELLOW);

	iv->set_title(iv, &title);
	//iv->set_items(iv, items, 0);

	//pthread_create(&lcm_thread, NULL, update_lcm_thread, priv);

	if(ret == 1){
  	//iv->set_text(iv,"Pass");
  	iv->set_items(iv, items_pass, 0);
  	mod->test_result = FTM_TEST_PASS;
  }
  if(ret == 0){
  	//iv->set_text(iv,"Fail");
  	iv->set_items(iv, items_fail, 0);
  	mod->test_result = FTM_TEST_FAIL;
  }
  	sleep(1);
  		iv->start_menu(iv, 0);
			iv->redraw(iv);
			sleep(1);
#if 0
	do {
		chosen = iv->run(iv, &exit);
		switch (chosen) {
		case ITEM_PASS:
		case ITEM_FAIL:
			if (chosen == ITEM_PASS) {
				mod->test_result = FTM_TEST_PASS;
			} else if (chosen == ITEM_FAIL) {
				mod->test_result = FTM_TEST_FAIL;
			}
			exit = true;
			break;
		}

		if (exit) {
			lcm_test_exit = true;
			break;
		}
		else{
			iv->start_menu(iv, 0);
			iv->redraw(iv);
			if(lcm_test_exit)
				break;
		}
	} while (1);
#endif
	return 0;
}

int lcm_init(void)
{
	int ret = 0;
	struct ftm_module *mod;

	LOGD("%s\n", __FUNCTION__);

	mod = ftm_alloc(ITEM_LCM, sizeof(struct ftm_module));
	if (!mod)
		return -ENOMEM;

	ret = ftm_register(mod, lcm_entry, (void*)mod);

	return ret;
}

#endif // FEATURE_FTM_VIBRATOR
