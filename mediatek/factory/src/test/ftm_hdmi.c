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
#include <fcntl.h>
#include <errno.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include <linux/hdmitx.h>

#ifdef FEATURE_FTM_HDMI

#include "ftm_audio_Common.h"


#define TAG                 "[HDMI] "


static const char *devpath = "/dev/hdmitx";

#define mod_to_hdmi(p)	(hdmi_module *)((char *)(p) + sizeof(struct ftm_module))

#define ARY_SIZE(x)     (sizeof((x)) / sizeof((x[0])))
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
static item_t hdmi_item[] = {
    item(ITEM_PASS,   uistr_pass),
	item(ITEM_FAIL,   uistr_fail),
	item(-1,          NULL),
};

typedef struct {
	struct ftm_module *module;
    char info[1024];

    /* item view */
	struct itemview *itm_view;
    text_t title;
    text_t text;

} hdmi_module;



static int hdmi_enable()
{
    int fd = -1;
    int ret = 0;
    int check_res = 0;

    fd = open(devpath, O_RDONLY);
    if (fd == -1)
    {
        LOGE(TAG "Error, Can't open /dev/hdmitx\n");
        return -1;
    }

    ret = ioctl(fd, MTK_HDMI_FACTORY_MODE_ENABLE, 1);
    if(ret < 0)
    {
        check_res = -1;
        goto check_exit;
    }
    
    //Common_Audio_init();
    //Audio_HDMI_SineTonePlayback(true, 44100);
    ioctl(fd, MTK_HDMI_AUDIO_VIDEO_ENABLE, 1);
    usleep(1000*2500);
    ioctl(fd, MTK_HDMI_VIDEO_CONFIG, 1);

    usleep(1000*1500);
    
check_exit:

    close(fd);
    return check_res;
}


static int hdmi_disable()
{
    int fd = -1;
    int ret = 0;
    fd = open(devpath, O_RDONLY);
    if (fd == -1)
    {
        LOGE(TAG "Error, Can't open /dev/hdmitx\n");
        return -1;
    }
    ///ioctl(fd, MTK_HDMI_FACTORY_MODE_ENABLE, 0);
    //Audio_HDMI_SineTonePlayback(false, 44100);
    ret = ioctl(fd, MTK_HDMI_AUDIO_VIDEO_ENABLE, 0);
    
    close(fd);
    //Common_Audio_deinit();
    return ret;
}


int hdmi_entry(struct ftm_param *param, void *priv)
{
    bool exit = false;
    hdmi_module *hdmi = (hdmi_module *)priv;
    struct itemview *iv;

    LOGD(TAG "hdmi_entry\n");

     /* show text view */
    if (!hdmi->itm_view) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory for item view");
            return -1;
        }
        hdmi->itm_view = iv;
    }

    iv = hdmi->itm_view;

    //init item view
    memset(&hdmi->info[0], 0, sizeof(hdmi->info));
    memset(&hdmi->info[0], '\n', 10);
    init_text(&hdmi->title, param->name, COLOR_YELLOW);
    init_text(&hdmi->text, &hdmi->info[0], COLOR_YELLOW);

    iv->set_title(iv, &hdmi->title);
    iv->set_items(iv, hdmi_item, 0);
    iv->set_text(iv, &hdmi->text);

    //iv->redraw(iv);

    if(hdmi_enable())
    {
        LOGD(TAG "hdmi test fail\n");
        //hdmi->text.color = COLOR_RED;
        sprintf(hdmi->info, "HDMI "uistr_fail"\n");
    }
    else
    {
        LOGD(TAG "hdmi test pass\n");
        //hdmi->text.color = COLOR_GREEN;
        sprintf(hdmi->info, "HDMI Basic "uistr_pass" \n");

        int fd = open(devpath, O_RDONLY);
        int check_res = 0;
        ioctl(fd, MTK_HDMI_FACTORY_GET_STATUS, &check_res);
        if(check_res <=0)
        {
            sprintf(hdmi->info, "please check your cable and retest.\n");
            goto skip_dpi_exit;
        }

        sprintf(hdmi->info, "HDMI Basic "uistr_pass", please check TV status \n");
        
        ioctl(fd, MTK_HDMI_FACTORY_DPI_TEST, &check_res);
        
        usleep(1000*500);
    }

skip_dpi_exit:

    while(!exit)
    {
        switch(iv->run(iv, NULL))
        {
        case ITEM_PASS:
            hdmi->module->test_result = FTM_TEST_PASS;
            exit = true;
            break;
        case ITEM_FAIL:
            hdmi->module->test_result = FTM_TEST_FAIL;
            exit = true;
            break;
        case -1:
            exit = true;
            break;
        default:
            break;
        }
    }

    hdmi_disable();

    return 0;
}

int hdmi_init(void)
{
    int ret = 0;

    struct ftm_module *mod;
	hdmi_module *hdmi;

    LOGD(TAG "hdmi_init\n");

    mod = ftm_alloc(ITEM_HDMI, sizeof(hdmi_module));
    if (!mod)
    {
        LOGD(TAG "hdmi_init failed\n");
        return -ENOMEM;
    }

	hdmi = mod_to_hdmi(mod);
    hdmi->module = mod;

    ret = ftm_register(mod, hdmi_entry, (void*)hdmi);
    if (ret)
    {
		LOGE(TAG "register HDMI failed (%d)\n", ret);
    }

    return ret;
}
#endif

