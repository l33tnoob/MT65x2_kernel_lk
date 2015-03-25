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
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/poll.h>

#include <linux/input.h>
#include <common.h>
#include <cust_keys.h>
#include <miniui.h>
#include <ftm.h>

#ifdef FEATURE_FTM_OFN
/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG	"[OFN] "
#define OFNLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define OFNLOGE(fmt, arg ...) LOGE(TAG fmt, ##arg)
/*---------------------------------------------------------------------------*/
enum {
    ITEM_OFN_FREE,
	ITEM_OFN_PASS,
	ITEM_OFN_FAIL,
	ITEM_OFN_RETURN
};
/*---------------------------------------------------------------------------*/
static item_t ofn_items[] = {
	item(ITEM_OFN_FREE,     uistr_info_ofn_free_mode_item),
	item(ITEM_OFN_PASS,     uistr_info_ofn_pass_item),
	item(ITEM_OFN_FAIL,      uistr_info_ofn_fail_item),
	item(ITEM_OFN_RETURN,   uistr_info_ofn_return_item),
	item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
struct ofn_priv {
    int event_fd;
    int width;
    int height;
    int max_x;  /*the maximum x position*/  
    int min_x;  /*the minimum x position*/  
    int max_y;  /*the maximum y position*/
    int min_y;  /*the minimum y position*/
    int cx;     /*current x position*/
    int cy;     /*current y position*/
    int nx;     /*next x position*/
    int ny;     /*next y position*/
    int text_x; /*the x position of text*/
    int text_y; /*the y position of text*/
    char info[256];
};
/*---------------------------------------------------------------------------*/
struct ofn_data {

    struct ofn_priv ofn;

    /*common for each factory mode*/
    bool  exit_thd;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
    
    pthread_t update_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
    struct paintview pv;            
};
/*---------------------------------------------------------------------------*/
#define mod_to_ofn_data(p)	(struct ofn_data *)((char *)(p) + sizeof(struct ftm_module))
/*---------------------------------------------------------------------------*/
int ofn_init_priv(struct ofn_priv *obj)
{
    obj->event_fd = -1;
    obj->width = ui_fb_width();
    obj->height = ui_fb_height();
    obj->max_x = obj->width;
    obj->min_x = 0;
    obj->max_y = obj->height - 36;
    obj->min_y = 36;
    obj->text_x = obj->min_x;
    obj->text_y = obj->min_y;
    obj->cx = obj->width/2;
    obj->cy = obj->height/2;
    obj->nx = obj->cx;
    obj->ny = obj->cy;
    obj->info[0] = 0;
    return 0;
}
/*---------------------------------------------------------------------------*/
int ofn_open_fd(struct ofn_priv *obj)
{
    DIR *dir;
    struct dirent *de;
    int fd;

    dir = opendir("/dev/input");
    if(dir != 0) {
        while((de = readdir(dir))) {
            if(strncmp(de->d_name,"event",5)) 
                continue;
            fd = openat(dirfd(dir), de->d_name, O_RDONLY);
            if(fd < 0) {
                continue;
            } else {
                char name[80];
                if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
                    name[0] = '\0';
                OFNLOGD("%s: %s is found\n", __func__, name);
                if (!strcmp(name, "mtofn")) {                    
                    obj->event_fd = fd;
                    fcntl(obj->event_fd, F_SETFL, O_NONBLOCK);                    
                    break;
                }
                close(fd);
            }
        }
    }
    if (obj->event_fd != -1)
        return 0;
    return -1;    
}
/*---------------------------------------------------------------------------*/
int ofn_close_fd(struct ofn_priv *obj)
{
    if (obj->event_fd != -1)
        close(obj->event_fd);
    obj->event_fd = -1;
    return 0;
}
/*---------------------------------------------------------------------------*/
static int ofn_pv_key_handler(int key, void *priv) 
{
    int handled = 0;
    int exit = 0;
    struct ofn_data *dat = (struct ofn_data *)priv;
    struct ofn_priv *obj = &dat->ofn;
    struct paintview *pv = &dat->pv;

    switch (key) {
    case UI_KEY_BACK:
        OFNLOGD("Back Button Click\n"); 
        exit = 1;
        break;
    case UI_KEY_LEFT:
        OFNLOGD("Key_Left Click\n");         
        dat->mod->test_result = FTM_TEST_FAIL;
        exit = 1;
        break;
    case UI_KEY_CONFIRM:
        OFNLOGD("Key_Confirm Click\n");                 
        dat->mod->test_result = FTM_TEST_PASS;
        exit = 1;
        break;
    default:
        handled = -1;
        break;
    }
    if (exit) {
        OFNLOGD( "%s: Exit thead\n", __FUNCTION__);
        dat->exit_thd = true;
        pv->exit(pv);        
    }
    return handled;
}
/*---------------------------------------------------------------------------*/
static void ofn_draw_string(void *priv, char* str, int x, int y, int dx, int dy)
{
    struct ofn_data *dat = (struct ofn_data *)priv;
    struct ofn_priv *obj = &dat->ofn;
    struct paintview *pv = &dat->pv;
    int sx = x + dx;
    int sy = y + dy;

    if (sx > obj->width)
        sx = obj->width;
    if (sy > obj->height)
        sy = obj->height;

    ui_color(32, 32, 32, 255);
    ui_fill(x, y, sx, sy);
    ui_color(255, 255, 255, 255);
    ui_text(x, y+(dy*3)/4, str);
}
/*---------------------------------------------------------------------------*/
static void *ofn_update_thread(void *priv) 
{
    struct ofn_data *dat = (struct ofn_data *)priv;
    struct ofn_priv *obj = &dat->ofn;
    struct paintview *pv = &dat->pv;
    struct input_event data;    
    int err, dx = 0, dy = 0;
    char ptloc[80];    


    OFNLOGD( "enter thread\n");
    if ((err = ofn_init_priv(obj))) {
        snprintf(obj->info, sizeof(obj->info), "init_priv err: %d\n", err);
        OFNLOGE("init_priv err: %d\n", err);
        //jbd_draw_string(dat, ptloc);
        //ui_flip();
        goto exit;
    } else if ((err = ofn_open_fd(obj))) {
        snprintf(obj->info, sizeof(obj->info), "dev not found: %d\n", err);
        OFNLOGE("dev not found: %d\n", err);
        //jbd_draw_string(dat, ptloc);
        //ui_flip();
        goto exit;
    }
    OFNLOGD("event_fd = %d\n", obj->event_fd);
    while (1) {

        if (dat->exit_thd) 
            break;
        if (obj->event_fd == -1)
            continue;
        if (read(obj->event_fd, &data, sizeof(struct input_event)) <= 0) 
            continue;
        //OFNLOGD("event: (%d %d %d)\n", data.type, data.code, data.value);
        if (data.type == EV_REL && data.code == REL_X)
            dx = data.value;
        if (data.type == EV_REL && data.code == REL_Y)
            dy = data.value;
        if (data.type != EV_SYN)
            continue;
        OFNLOGD("event: (%+3d, %+3d)\n", dx, dy);
        ui_color(255,255,255,255);
        obj->nx += dx*3;
        obj->ny += dy*3;
        if (obj->nx >= obj->max_x)  obj->nx = obj->max_x;
        if (obj->nx <  obj->min_x)  obj->nx = obj->min_x;
        if (obj->ny >= obj->max_y)  obj->ny = obj->max_y;            
        if (obj->ny <  obj->min_y)  obj->ny = obj->min_y;            
        ui_line(obj->cx, obj->cy, obj->nx, obj->ny, 2);        
        obj->cx = obj->nx;
        obj->cy = obj->ny;
        sprintf(ptloc,"x:%4d y:%4d",obj->cx, obj->cy);
        ofn_draw_string(dat, ptloc, obj->width/2, 0, obj->width/2, 36);
        dx = dy = 0;
        ui_flip();
    }
    ofn_close_fd(obj);    
exit:    
    OFNLOGD( "exit thread\n");
    pthread_exit(NULL);
    return NULL;
}
/*---------------------------------------------------------------------------*/
static int ofn_entry(struct ftm_param *param, void *priv)
{
	int i, err, num = 0, key, handled;
    int chosen;    
	bool exit;
    struct ofn_data *dat = (struct ofn_data *)priv;
    struct ofn_priv *obj = &dat->ofn;
	struct textview *tv = &dat->tv;
	struct paintview *pv = &dat->pv;
   
    OFNLOGD( "%s\n", __FUNCTION__);
    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->left_btn, uistr_info_ofn_fail, COLOR_YELLOW);
    init_text(&dat->center_btn, uistr_info_ofn_pass, COLOR_YELLOW);
    init_text(&dat->right_btn, uistr_info_ofn_back, COLOR_YELLOW);       
    dat->exit_thd = false;  

    ui_init_paintview(pv, ofn_pv_key_handler, (void*)dat);
    pv->set_title(pv, &dat->title);
    pv->set_btn(pv, &dat->left_btn, &dat->center_btn, &dat->right_btn);
    pthread_create(&(dat->update_thd), NULL, ofn_update_thread, priv);
    do {
#if 1
        int key, handled;
        extern void ui_clear_key_queue();
        ui_clear_key_queue();
        pv->m_redraw = 1;
        do {        
            if (pv->m_redraw)
                pv->redraw(pv);

            /* Try to paint something */      
            pv->drawpoint(pv);
            pv->drawline(pv);
            pv->drawcircle(pv);        
            pv->drawimg(pv);
            if (obj->info[0])
                ofn_draw_string(dat, obj->info, obj->text_x, obj->text_y, obj->width, 36);    
            pv->flip();        

            key = ui_wait_key();

            LOGD(TAG "KEY: %d\n", key);
            
            if (pv->m_khandler)
                handled = pv->m_khandler(key, pv->m_priv);
            if (pv->m_exit)
                break;
        } while (1);
        pv->m_exit = 0;
        
#else        
        pv->run(pv);
#endif 
        if (dat->exit_thd == true)
            break;
    } while (1);    
    pthread_join(dat->update_thd, NULL);
	return 0;
}
/*---------------------------------------------------------------------------*/
int ofn_init(void)
{
	int r;
	struct ftm_module *mod;
	struct ofn_data *dat;

	mod = ftm_alloc(ITEM_OFN, sizeof(struct ofn_data));
	if (!mod)
		return -ENOMEM;

    dat = mod_to_ofn_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    ofn_init_priv(&dat->ofn);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;    

	r = ftm_register(mod, ofn_entry, (void*)dat);
	if (r) {
		OFNLOGD( "register jogball failed (%d)\n", r);
		return r;
	}

	return 0;
}

#endif
