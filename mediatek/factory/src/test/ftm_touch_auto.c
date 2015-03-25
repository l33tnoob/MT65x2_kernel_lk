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

#define TAG "[Touch] "

#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <linux/input.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

enum {
    ITEM_TOUCH_CALIBRATION,
    ITEM_TOUCH_LINEARITY,
    ITEM_TOUCH_ACCURACY,
    ITEM_TOUCH_SENSITIVITY,
    ITEM_TOUCH_DEADZONE,
    ITEM_TOUCH_ZOOM,
    ITEM_TOUCH_FREEMODE,
    ITEM_TOUCH_PASS,
    ITEM_TOUCH_FAIL,
    ITEM_TOUCH_CHOOSER,
};

item_t rtype_touch_auto_items[] = {
    item(ITEM_TOUCH_CALIBRATION,   uistr_info_touch_calibration),
    item(ITEM_TOUCH_LINEARITY,   uistr_info_touch_rtp_linearity),
    item(ITEM_TOUCH_ACCURACY,    uistr_info_touch_rtp_accuracy),
    item(ITEM_TOUCH_SENSITIVITY, uistr_info_touch_sensitivity),
    item(ITEM_TOUCH_DEADZONE,    uistr_info_touch_deadzone),
    item(ITEM_TOUCH_FREEMODE,    uistr_info_touch_freemode),
    item(ITEM_TOUCH_PASS,        uistr_pass),
    item(ITEM_TOUCH_FAIL,        uistr_fail),
    item(-1            ,  NULL),
};

item_t ctype_touch_auto_items[] = {
    //item(ITEM_TOUCH_LINEARITY,   uistr_info_touch_ctp_linearity),
    //item(ITEM_TOUCH_ACCURACY,    uistr_info_touch_ctp_accuracy),
    //item(ITEM_TOUCH_SENSITIVITY, uistr_info_touch_sensitivity),
    //item(ITEM_TOUCH_DEADZONE,    uistr_info_touch_deadzone),
    //item(ITEM_TOUCH_ZOOM,        uistr_info_touch_zoom),
    //item(ITEM_TOUCH_FREEMODE,    uistr_info_touch_freemode),
    //item(ITEM_TOUCH_PASS,        uistr_pass),
    //item(ITEM_TOUCH_FAIL,        uistr_fail),
    item(-1            ,  NULL),
};

struct touch {
    struct ftm_module *mod;
    pthread_t update_thd;
    
    bool exit_thd;
    bool enter_scene;

    int width;
    int height;
    //int event_fd;
    int scene;
    int moveto;
    struct paintview empty;
    /*struct paintview linearity;
    struct paintview linearity2;
    struct paintview accuracy;
    struct paintview sensitivity;
    struct paintview deadzone;*/
    struct paintview freemode;
    struct paintview zoom;
    struct paintview *cpv;
    struct itemview menu;
    text_t title;
};

#define mod_to_touch(p)     (struct touch*)((char*)(p) + sizeof(struct ftm_module))

static int paintview_key_handler(int key, void *priv) {
    int handled = 0;
    struct paintview *pv = (struct paintview *)priv;

    switch (key) {
    case UI_KEY_BACK:
        pv->exit(pv);
        break;
    default:
        handled = -1;
        break;
    }
    return handled;
}

int touch_auto_entry(struct ftm_param *param, void *priv) {
    //bool exit = false;
    struct touch *tpd = (struct touch *)priv;
    //line_t *sensitivity_lines = NULL;
    FILE *fp;
    char type = 0;

    line_t freemode_lines[] = {
        {0,20,tpd->width,20,2,COLOR_WHITE},
    };
	
    //setup_circles(tpd);
    
    LOGD(TAG "%s\n", __FUNCTION__);

    tpd->moveto = ITEM_TOUCH_CHOOSER;
    tpd->scene = ITEM_TOUCH_CHOOSER;
    tpd->enter_scene = false;
    tpd->exit_thd = false;  

    ui_init_paintview(&(tpd->empty), paintview_key_handler, &(tpd->empty));

    /*ui_init_paintview(&(tpd->linearity), paintview_key_handler, &(tpd->linearity));
    tpd->linearity.set_line(&(tpd->linearity), linearity_lines, 2);
    ui_init_paintview(&(tpd->linearity2), paintview_key_handler, &(tpd->linearity2));
    tpd->linearity2.set_line(&(tpd->linearity2), linearity_lines2, 2);
    ui_init_paintview(&(tpd->accuracy), paintview_key_handler, &(tpd->accuracy));
    tpd->accuracy.set_circle(&(tpd->accuracy), accuracy_circles, 13);
    ui_init_paintview(&(tpd->deadzone), paintview_key_handler, &(tpd->deadzone));
    tpd->deadzone.set_line(&(tpd->deadzone), deadzone_lines, 2);
    ui_init_paintview(&(tpd->sensitivity), paintview_key_handler, &(tpd->sensitivity));
    sensitivity_lines = scene_sensitivity(tpd);
    tpd->sensitivity.set_line(&(tpd->sensitivity), &(sensitivity_lines[1]), sensitivity_lines[0].sx);*/
    ui_init_paintview(&(tpd->zoom), paintview_key_handler, &(tpd->zoom));

    ui_init_paintview(&(tpd->freemode), paintview_key_handler, &(tpd->freemode));
    tpd->freemode.set_line(&(tpd->freemode), freemode_lines, 1);

    ui_init_itemview(&(tpd->menu));
    init_text(&(tpd->title), param->name,  COLOR_YELLOW);
    tpd->menu.set_title(&(tpd->menu), &(tpd->title));
    
    if ((fp = fopen("/sys/module/tpd_setting/parameters/tpd_type_cap", "r")) == NULL) {
        printf("can not open tpd_setting sysfs.\n");
    }
  
    if (fp != NULL) {
        type = fgetc(fp);
        fclose(fp);
    }
  
    if (type == '0') {
        tpd->menu.set_items(&(tpd->menu), rtype_touch_auto_items, 0);
    } else {
        tpd->menu.set_items(&(tpd->menu), ctype_touch_auto_items, 0);
    }

#if 1	
    //tpd->menu.start_menu(&(tpd->menu),0);
    tpd->menu.redraw(&(tpd->menu));
    if ((fp = fopen("/sys/module/tpd_setting/parameters/tpd_load_status", "r")) == NULL) {
        printf("can not open tpd_setting sysfs.\n");
    }
    if (fp != NULL) {
        type = fgetc(fp);
        fclose(fp);
    }
  
    if (type == '1') {
		tpd->mod->test_result = FTM_TEST_PASS;
    } else {
		tpd->mod->test_result = FTM_TEST_FAIL;
    }
#endif 

#if 0    
    pthread_create(&(tpd->update_thd), NULL, touch_update_thread, priv);
    do {
        switch(tpd->scene) {
        case ITEM_TOUCH_CHOOSER:
            tpd->menu.redraw(&(tpd->menu)); 
            tpd->scene = tpd->menu.run(&(tpd->menu), &(exit));
            tpd->menu.redraw(&(tpd->menu)); 
            pthread_mutex_lock(&enter_mutex);
            tpd->enter_scene = true;
            pthread_mutex_unlock(&enter_mutex);
            scene_init(tpd);
            switch (tpd->scene) {
            case ITEM_TOUCH_PASS:
            case ITEM_TOUCH_FAIL:
                if (tpd->scene == ITEM_TOUCH_PASS) {
                    tpd->mod->test_result = FTM_TEST_PASS;
                } else if (tpd->scene == ITEM_TOUCH_FAIL) {
                    tpd->mod->test_result = FTM_TEST_FAIL;
                }
                tpd->exit_thd = true;
                break;
            default: 
                setup_cpv(tpd);
                break;
            }
            break;
        default:
            if(tpd->cpv) tpd->cpv->run(tpd->cpv);
            tpd->scene = (tpd->moveto!=ITEM_TOUCH_CHOOSER?tpd->moveto:ITEM_TOUCH_CHOOSER);
            pthread_mutex_lock(&enter_mutex);
            tpd->enter_scene = true;
            pthread_mutex_unlock(&enter_mutex);

            tpd->moveto = ITEM_TOUCH_CHOOSER;
            setup_cpv(tpd);
            break;
        }
        if(exit) tpd->exit_thd = true;
    } while (!tpd->exit_thd);
    pthread_join(tpd->update_thd, NULL);
    free(sensitivity_lines);
#endif
    return 0;
}

int touch_auto_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct touch *tpd;

    LOGD(TAG "%s\n", __FUNCTION__);
    //calibration_init();
    mod = ftm_alloc(ITEM_TOUCH_AUTO, sizeof(struct touch));
    tpd  = mod_to_touch(mod);
    memset(tpd, 0x00, sizeof(*tpd));
    tpd->mod = mod; 
    //tpd->event_fd = -1;
    tpd->width = ui_fb_width();
    tpd->height = ui_fb_height();
    if (!mod) return -ENOMEM;
    ret = ftm_register(mod, touch_auto_entry, (void*)tpd);
    return 0;
}

