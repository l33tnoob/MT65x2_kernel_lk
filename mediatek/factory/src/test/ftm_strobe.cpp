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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define TAG                  "[CAMERA] "
#define LOG_TAG "flash_mgr.cpp"

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

extern "C" {
#include "common.h"
#include "miniui.h"
#include "ftm.h"
}

#include "flash_drv.h"

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#define DEBUG_FTM_CAMERA
#ifdef DEBUG_FTM_CAMERA
#define FTM_CAMERA_DBG(fmt, arg...) LOGD(fmt, ##arg)
#define FTM_CAMERA_ERR(fmt, arg...)  LOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define FTM_CAMERA_DBG(a,...)
#define FTM_CAMERA_ERR(a,...)
#endif
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
enum
{
	ITEM_NULL,
    ITEM_PASS,
    ITEM_FAIL,
    ITEM_STROBE_TEST,
};
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
struct camera
{
    char  info[1024];
    int isExit;
    int cmd;
	int isTestDone;
    text_t    title;
    text_t    text;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
};
#define mod_to_camera(p)     (struct camera*)((char*)(p) + sizeof(struct ftm_module))
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
static item_t menu_items[5];

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
static void *strobe_test_thread(void *priv)
{
	FTM_CAMERA_DBG("strobe_test_thread %d\n",__LINE__);
	struct camera *stb = (struct camera *)priv;
	while(!stb->isExit)
	{
		//FTM_CAMERA_DBG("strobe_test_thread %d\n",__LINE__);
		if(stb->cmd==ITEM_STROBE_TEST)
		{
			//FTM_CAMERA_DBG("strobe_test_thread %d\n",__LINE__);
			FlashSimpleDrv*  pStrobe;
			pStrobe = FlashSimpleDrv::getInstance();
			pStrobe->init(1);
			stb->isTestDone = 1;
		    while(!stb->isExit && stb->cmd==ITEM_STROBE_TEST)
		    {
		    	//FTM_CAMERA_DBG("strobe_test_thread %d\n",__LINE__);
		    	pStrobe->setPreOn();
		    	usleep(30000);
		        pStrobe->setOnOff(1);
		        usleep(30000);
		        pStrobe->setOnOff(0);
		        usleep(100000);
		    }
		    pStrobe->uninit();
		}
		usleep(30000);
	}
	return NULL;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int strobe_entry(struct ftm_param *param, void *priv)
{
	FTM_CAMERA_DBG("strobe_entry %d\n",__LINE__);
    int chosen;
    bool exit = false;
    struct camera *cam = (struct camera *)priv;
    struct textview *tv = NULL ;
    struct itemview *iv = NULL ;
    static int isTestDone = 0;

    init_text(&cam ->title, param->name, COLOR_YELLOW);
    init_text(&cam ->text, &cam->info[0], COLOR_YELLOW);
    if (!cam->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            FTM_CAMERA_DBG("No memory");
            return -1;
        }
        cam->iv = iv;
    }
    iv = cam->iv;
    iv->set_title(iv, &cam->title);

    //menu_items[0].id = ITEM_STROBE_TEST;
    //menu_items[0].name = uistr_strobe_test;
    //menu_items[1].id = -1;
    //menu_items[1].name = NULL;



    menu_items[0].id = ITEM_PASS;
    menu_items[0].name = uistr_pass;
    menu_items[1].id = ITEM_FAIL;
    menu_items[1].name = uistr_fail;
    menu_items[2].id = -1;
    menu_items[2].name = NULL;

    iv->set_items(iv, menu_items, 0);
    iv->set_text(iv, &cam->text);

    pthread_t strobeTestTh;
    pthread_create(&strobeTestTh, NULL, strobe_test_thread, cam);
	cam->cmd = ITEM_NULL;
	cam->isExit = 0;
	cam->isTestDone = isTestDone;

    if(isTestDone == 0)
    {
        cam->cmd = ITEM_STROBE_TEST;
        isTestDone=1;
    }

	while(1)
	{
		/*if(cam->isTestDone==1)
		{
			menu_items[1].id = ITEM_PASS;
	    	menu_items[1].name = uistr_pass;
	    	menu_items[2].id = ITEM_FAIL;
	    	menu_items[2].name = uistr_fail;
	    	menu_items[3].id = -1;
	    	menu_items[3].name = NULL;
	    	iv->set_items(iv, menu_items, 0);
  		}*/
		chosen = iv->run(iv, &exit);
		switch (chosen)
		{
			//case ITEM_STROBE_TEST:
			//	cam->cmd = ITEM_STROBE_TEST;
			//	isTestDone=1;
			//break;
			case ITEM_PASS:
				cam->cmd = ITEM_NULL;
				if(cam->isTestDone)
				{
				    cam->mod->test_result = FTM_TEST_PASS;
				    exit = true;
				}
				else
				{
				    memset(cam->info, 0, 1024);
				    sprintf(cam->info, "Not test done !! \n");
				    iv->set_text(iv, &cam->text);
				    iv->redraw(iv);
				}
				break;
			case ITEM_FAIL:
				cam->cmd = ITEM_NULL;
				if(cam->isTestDone)
				{
				    cam->mod->test_result = FTM_TEST_FAIL;
				    exit = true;
				}
				else
				{
				    memset(cam->info, 0, 1024);
				    sprintf(cam->info, "Not test done !! \n");
				    iv->set_text(iv, &cam->text);
				    iv->redraw(iv);
				}
			break;
		}
		if(exit)
		{
			isTestDone = cam->isTestDone;
			cam->cmd = ITEM_NULL;
			cam->isExit = true;
            isTestDone = 0;
			break;
		}
		usleep(30000);
	}
FTM_CAMERA_DBG("strobe_entry %d\n",__LINE__);
    pthread_join(strobeTestTh, NULL);
FTM_CAMERA_DBG("strobe_entry %d\n",__LINE__);
    return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
extern "C" int strobe_init(void)
{
	FTM_CAMERA_DBG("strobe_init %d\n",__LINE__);
    int ret = 0;
    struct ftm_module *mod;
    struct camera *cam;

    mod = ftm_alloc(ITEM_STROBE, sizeof(struct camera));
    cam  = mod_to_camera(mod);
    memset(cam, 0x0, sizeof(struct camera));
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/
    cam->mod = mod;
    if (!mod)
        return -ENOMEM;
    ret = ftm_register(mod, strobe_entry, (void*)cam);
    return ret;
}


