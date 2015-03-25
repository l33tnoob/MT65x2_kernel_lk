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
#include <linux/ioctl.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"


#ifdef FEATURE_FTM_OTG

#ifdef __cplusplus
extern "C" {
#endif


#define TAG                 "[OTG] "
#define OTG_STATE_PATH "/sys/class/switch/otg_state/state"

#define BUF_LEN 1
char r_buf[BUF_LEN] = {'\0'};
char w_buf[BUF_LEN] = {'1'};


enum {
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t otg_items[] = {
    {ITEM_PASS, uistr_pass},
    {ITEM_FAIL, uistr_fail},
    {-1, NULL},
};

struct otg {
    char  info[1024];
	bool  avail;
    bool  exit_thd;

    pthread_t otg_update_thd;
    struct ftm_module *mod;
    struct itemview *iv;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
};

#define mod_to_otg(p)     (struct otg*)((char*)(p) + sizeof(struct ftm_module))


// use for ioctl for otg driver
static int OtgFd =0;

static void otg_update_info(struct otg *hds, char *info)
{
  char *ptr;
  int rc;
	int fd = -1;
	int hb_status = 0;
	int ret = 0;

	fd = open(OTG_STATE_PATH, O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "Can't open %s\n", OTG_STATE_PATH);
		hds->avail = false;
		goto EXIT;
	}
	if (read(fd, r_buf, BUF_LEN) == -1) {
		LOGD(TAG "Can't read %s\n",OTG_STATE_PATH);
		hds->avail = false;
		goto EXIT;
	}
	if (!strncmp(w_buf, r_buf, BUF_LEN)) { /*the same*/
		hds->avail = true;
    } else {
    	hds->avail = false;
    }
    		LOGD("OTG state is %s\n",r_buf);
EXIT:
	close(fd);

    /* preare text view info */
    ptr  = info;
    ptr += sprintf(ptr, uistr_info_otg_status " : %s\n", hds->avail ? uistr_info_otg_status_host : uistr_info_otg_status_device);
    return;
}


static void *otg_update_iv_thread(void *priv)
{
    struct otg *hds = (struct otg *)priv;
    struct itemview *iv = hds->iv;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    while (1) {
        usleep(200000);

        if (hds->exit_thd)
            break;

			otg_update_info(hds, hds->info);
      iv->set_text(iv, &hds->text);
      iv->redraw(iv);
      }

    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);

	return NULL;
}

int otg_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct otg *hds = (struct otg *)priv;
    struct itemview *iv;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&hds->title, param->name, COLOR_YELLOW);
    init_text(&hds->text, &hds->info[0], COLOR_YELLOW);
    init_text(&hds->left_btn, "Fail", COLOR_YELLOW);
    init_text(&hds->center_btn, "Pass", COLOR_YELLOW);
    init_text(&hds->right_btn, "Back", COLOR_YELLOW);

    otg_update_info(hds, hds->info);

    hds->exit_thd = false;

	if (!hds->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        hds->iv = iv;
    }

    iv = hds->iv;
    iv->set_title(iv, &hds->title);
    iv->set_items(iv, otg_items, 0);
    iv->set_text(iv, &hds->text);

    pthread_create(&hds->otg_update_thd, NULL, otg_update_iv_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                hds->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                hds->mod->test_result = FTM_TEST_FAIL;
            }
            exit = true;
            break;
        }

        if (exit) {
            hds->exit_thd = true;
            break;
        }
    } while (1);
    pthread_join(hds->otg_update_thd, NULL);

    return 0;
}

int otg_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct otg *hds;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_OTG, sizeof(struct otg));
    hds = mod_to_otg(mod);

    hds->mod    = mod;
		hds->avail	= false;

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, otg_entry, (void*)hds);

    return ret;
}


#ifdef __cplusplus
}
#endif

#endif
