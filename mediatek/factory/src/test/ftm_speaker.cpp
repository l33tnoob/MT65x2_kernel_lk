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
#include <sys/time.h>


#include "ftm_audio_Common.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FEATURE_FTM_SPK_OC

#define mod_to_mSpk(p)     (struct mSpkTest*)((char*)(p) + sizeof(struct ftm_module))
#define TAG   "[Spk] "
#define TEXT_LENGTH (1024)

enum {
    ITEM_RETEST,
    ITEM_PASS,
    ITEM_FAIL,

};

static item_t spk_items[] = {
    {ITEM_RETEST,    uistr_retest,    0,    0},
    {ITEM_PASS,      uistr_pass, 0, 0},
    {ITEM_FAIL,      uistr_fail, 0, 0},
    {-1, NULL, 0,   0},
};

struct mSpkTest {
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
    pthread_t hHeadsetThread;
    int avail;
    bool exit_thd;
    char  info[TEXT_LENGTH];

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
};

int mGet_Spk_OC_Result()
{
    int i = 0, value = 0;
	for (i = 0;i<10;i++)
	{
	    value |= Audio_READ_SPK_OC_STA();
	}
	return value;
}

//Louder Speaker Over Current Test
//Ringone test
int mSpk_OC_test_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen, i4OCFlag;
    bool exit = false;
    struct mSpkTest*mc = (struct mSpkTest *)priv;
    struct textview *tv;
    struct itemview *iv;

    LOGD(TAG "--------------mSpkOCTest_entry----------------\n" );
    LOGD(TAG "%s\n", __FUNCTION__);
    init_text(&mc->title, param->name, COLOR_YELLOW);
	init_text(&mc->text, &mc->info[0], COLOR_YELLOW);
    init_text(&mc->left_btn, uistr_key_fail, COLOR_YELLOW);
    init_text(&mc->center_btn, uistr_key_pass, COLOR_YELLOW);
    init_text(&mc->right_btn, uistr_key_back, COLOR_YELLOW);



    if (!mc->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        mc->iv = iv;
    }
    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, spk_items, 0);
    iv->set_text(iv, &mc->text);
OC_TEST_BEGIN:
    // init Audio
    Common_Audio_init();
    LouderSPKOCTest(1,1);
	usleep(500*1000);//settling time
	i4OCFlag = mGet_Spk_OC_Result();
    LouderSPKOCTest(0,0);
    Common_Audio_deinit();

    if (i4OCFlag>0)
	    sprintf(mc->info, uistr_info_speaker_oc_fail,i4OCFlag);
	else
	    sprintf(mc->info, uistr_info_speaker_oc_pass,i4OCFlag);

	exit = false;
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                mc->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                mc->mod->test_result = FTM_TEST_FAIL;
            }
            exit = true;
            break;
			case ITEM_RETEST:
			{
				sprintf(mc->info, uistr_info_speaker_oc_retest);
				iv->redraw(iv);
				goto OC_TEST_BEGIN;
			}
			default:
				break;
        }
        if (exit) {
            break;
        }
    } while (1);


    return 0;
}


int spk_init(void)
{
    int ret = 0;
    struct ftm_module *modLoudspk;
    struct mSpkTest *maudioSpkOCTest;

    LOGD(TAG "%s\n", __FUNCTION__);
    LOGD(TAG "-------Spk_OC_Test_init---------------\n" );

    modLoudspk = ftm_alloc(ITEM_SPK_OC, sizeof(struct mSpkTest));
    maudioSpkOCTest = mod_to_mSpk(modLoudspk);
    maudioSpkOCTest->mod = modLoudspk;
    if (!modLoudspk)
        return -ENOMEM;
    ret = ftm_register(modLoudspk, mSpk_OC_test_entry, (void*)maudioSpkOCTest);

    return ret;
}
#endif

#ifdef __cplusplus
};
#endif


