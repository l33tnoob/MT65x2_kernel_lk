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
    ITEM_TOUCH_MAIN,
    ITEM_TOUCH_CALIBRATION,
    ITEM_TOUCH_LINEARITY,
    ITEM_TOUCH_LINEARITY2,
    ITEM_TOUCH_ACCURACY,
    ITEM_TOUCH_SENSITIVITY,
    ITEM_TOUCH_DEADZONE,
    ITEM_TOUCH_ZOOM,
    ITEM_TOUCH_FREEMODE,
    ITEM_TOUCH_PASS,
    ITEM_TOUCH_FAIL,
    ITEM_TOUCH_CHOOSER,
};

item_t rtype_touch_items[] = {
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

item_t ctype_touch_items[] = {
    item(ITEM_TOUCH_MAIN,   uistr_info_touch_ctp_main),
    //item(ITEM_TOUCH_LINEARITY,   "CTP Linearity"),
    //item(ITEM_TOUCH_ACCURACY,    "CTP Accuracy"),
    //item(ITEM_TOUCH_SENSITIVITY, "Sensitivity"),
    //item(ITEM_TOUCH_DEADZONE,    "Deadzone"),
    //item(ITEM_TOUCH_ZOOM,        "PinchToZoom"),
    item(ITEM_TOUCH_FREEMODE,    uistr_info_touch_freemode),
    item(ITEM_TOUCH_PASS,        uistr_pass),
    item(ITEM_TOUCH_FAIL,        uistr_fail),
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
#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)

static pthread_mutex_t enter_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t event_mutex = PTHREAD_MUTEX_INITIALIZER;

#define ABS_MT_POSITION_X       0x35
#define ABS_MT_POSITION_Y       0x36
#define ABS_MT_TOUCH_MAJOR      0x30
#define ABS_MT_WIDTH_MAJOR      0x32

#define SYN_MT_REPORT           2
#define SYN_REPORT              0

#define PASS    (0)
#define FAIL    (-1)
//update display framerate=report rate/TOUCH_UPDATE_FR_DIV
#ifndef TOUCH_UPDATE_FR_DIV
#define TOUCH_UPDATE_FR_DIV 2
#endif
static int accuracy_circles[13][5];
static int mt_radius, mt_max_radius, mt_min_radius;
static int dz_minx = TOUCH_DZ_X, dz_maxx = 0, dz_miny = TOUCH_DZ_Y, dz_maxy= 0;

//static struct pollfd ev_fds[1];
static struct input_event ev_queue[2048];
static int ev_tail_num = 0;
static int ev_head_num = 0;
static int cross_num = 0;
static bool retry_flag=false;
struct point {
    int x;
    int y;    
};
struct point tpd_point[4];
struct point lcd_point[4];   

long calmat[2][3] = {
    {4096, 0, 0},
    {0, 4096, 0}
};

struct detect_date {
	int count_up[50];
	int count_left[50];
	int count_down[50];
	int count_right[50];
	int count_mid_upright[50];
	int count_mid_transverse[50];
	int count_oblique1[50];
	int count_oblique2[50];
};

struct detect_date detect_date;

static void clear_detect_date(void)
{
	int i = 0;

	while(i < 50)
	{
		detect_date.count_up[i] = 0;
		detect_date.count_left[i] = 0;
		detect_date.count_down[i] = 0;
		detect_date.count_right[i] = 0;
		detect_date.count_mid_upright[i] = 0;
		detect_date.count_mid_transverse[i] = 0;
		detect_date.count_oblique1[i] = 0;
		detect_date.count_oblique2[i] = 0;
		i++;
	}
}
int bias[2] = {0,0};
int matrix[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int g_pass_case1 =0;
int g_pass_case2 =0;
int g_pass_case3 =0;
int g_pass_case4 =0;
int g_pass_case5 =0;

int tangle_px1 =0;
int tangle_py1 =0;
int tangle_px2 =0;
int tangle_py2 =0;
int tangle_px3 =0;
int tangle_py3 =0;
int tangle_px4 =0;
int tangle_py4 =0;

int tangle_l1=0;
int tangle_l2=0;
int tangle_l3=0;
int tangle_l4=0;

void clear_touch_temp_data(void)
{
    tangle_px1 =0;
    tangle_py1 =0;
    tangle_px2 =0;
    tangle_py2 =0;
    tangle_px3 =0;
    tangle_py3 =0;
    tangle_px4 =0;
    tangle_py4 =0;

    tangle_l1=0;
    tangle_l2=0;
    tangle_l3=0;
    tangle_l4=0;
}

void clear_touch_result(void)
{
    g_pass_case1 =0;
    g_pass_case2 =0;
    g_pass_case3 =0;
    g_pass_case4 =0;
    g_pass_case5 =0;
}


/*
void setline(line_t *line,int sx, int sy, int dx, int dy, int width, int color) {
  line->sx = sx;
  line->sy = sy;
  line->dx = dx;
  line->dy = dy;
  line->width = width;
  line->color = color;
}*/

/*line_t *scene_sensitivity(struct touch *tpd) {
    int d=TOUCH_D1*2, i=0,j=0,count=0;
    int x,y,px,py,dx,dy;
    line_t *lines = NULL;
    for(j=0;j<2;j++) {
        dx=tpd->width-(2*d), dy=tpd->height-(2*d);
        x=d;y=d;px=d;py=d;i=0;
        if(j==0) count++; 
        else {
          lines = (line_t *)malloc(sizeof(line_t)*(count+1));
          lines[0].sx = count;
          count=1;
          setline(&(lines[count++]),d,d,d,tpd->height-d,2,COLOR_RED);
        }
        while(1) {
            if(dx<0 || dy<0) break;
            switch(i%4) {
            case 0: x+=dx; dx-=d; break;
            case 1: y+=dy; dy-=d; break;
            case 2: x-=dx; dx-=d; break;
            case 3: y-=dy; dy-=d; break;
            }
            if(j==0) count++;
            else setline(&(lines[count++]),x,y,px,py,2,COLOR_RED);
            px = x, py=y;
            i = ((i+1)%4);
        }
    } return lines;
}*/


//add by factory mode owner:  this is the same as miniui.c, so please use that function. 
//if u want to use customized function, please add prefix "touch_" to avoid naming conflict.
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

void paint_sensitivity(struct touch *tpd) {
    int d=TOUCH_S1, i=0,j=0,count=0;
    int x,y,px,py,dx,dy;
    //line_t *lines = NULL;
    ui_color(255,0,0,255);
    LOGI(TAG "tpd_fwq tpd->width=%d \n" , tpd->width);
	LOGI(TAG "tpd_fwq tpd->height=%d \n" , tpd->height);
	LOGI(TAG "tpd_fwq d=%d \n " , d);
	 
	 
    for(j=0;j<2;j++) 
	{
        dx=tpd->width-(2*d), dy=tpd->height-(2*d);
        x=d;y=d;px=d;py=d;i=0;
		LOGI(TAG "tpd_fwq dx=%d,dy=%d,x=%d,y=%d,px=%d,py=%d \n " ,dx,dy,x,y,px,py);
        if(j==0)
		{
		   count++;

		   LOGI(TAG "tpd_fwq 1... j=%d count=%d \n " ,j,count);
        }
        else 
		{
          //lines = (line_t *)malloc(sizeof(line_t)*(count+1));
          //lines[0].sx = count;
          count=1;
          //setline(&(lines[count++]),d,d,d,tpd->height-d,2,COLOR_RED);
          LOGI(TAG "tpd_fwq draw line(%d,%d,%d,%d) \n " ,d,d,d,tpd->height);
          ui_line(d,d,d,tpd->height,2);
        }
        while(1) 
		{
            if(dx<0 || dy<0 ) 
			{
			  LOGI(TAG "tpd_fwq  break dx=%d,dy=%d\n " ,dx,dy);
			  LOGI(TAG "tpd_fwq  break x=%d,y=%d,px=%d,py=%d\n " ,x,y,px,py);
			  if(tpd->width==540 && tpd->height==960)
			  {
			     ui_line(300,720,300,240,2);
				 LOGI(TAG "if qhd draw the last line\n ");
			  }
			  	
			  break;
			   
			}
			
            switch(i%4) 
			{
            case 0: x+=dx; dx-=d; break;
            case 1: y+=dy; dy-=d; break;
            case 2: x-=dx; dx-=d; break;
            case 3: y-=dy; dy-=d; break;
            }
			LOGI(TAG "tpd_fwq dx=%d ,dy=%d \n " ,dx,dy);

            if(j==0) 
			{
				count++;
				LOGI(TAG "tpd_fwq 2... j=%d count=%d \n " ,j,count);
			}
            else 
			{
			  ui_line(x,y,px,py,2);
			  LOGI(TAG "tpd_fwq draw line2 (%d,%d,%d,%d) \n " ,x,y,px,py);
            }
            //else setline(&(lines[count++]),x,y,px,py,2,COLOR_RED);
            px = x, py=y;
            i = ((i+1)%4);
			LOGI(TAG "tpd_fwq i=%d ,px=%d,py=%d \n " ,i,px,py);
        }
    } //return lines;
}

static void paint_accuracy(struct touch *tpd) {
    int i;
    for(i=0;i<13;i++) {
        if(accuracy_circles[i][4]==0) ui_color(255,0,0,255);
        else ui_color(0,128,0,255);
        ui_circle(
            accuracy_circles[i][0], accuracy_circles[i][1], accuracy_circles[i][2]
        );
    }
}

static void paint_deadzone(struct touch *tpd) {
    if(dz_miny<TOUCH_DZ_Y) ui_color(0,255,0,255); else ui_color(255,0,0,255);
    ui_line(tpd->width/2,TOUCH_DZ_Y,tpd->width/2,TOUCH_D1+TOUCH_DZ_Y,5);
    if(dz_maxy>tpd->height-TOUCH_DZ_Y) ui_color(0,255,0,255); else ui_color(255,0,0,255);
    ui_line(tpd->width/2,tpd->height-TOUCH_DZ_Y,tpd->width/2,tpd->height-TOUCH_D1-TOUCH_DZ_Y,5);
    if(dz_minx<TOUCH_DZ_X) ui_color(0,255,0,255); else ui_color(255,0,0,255);
    ui_line(TOUCH_DZ_X,tpd->height/2,TOUCH_D1+TOUCH_DZ_X,tpd->height/2,5);
    if(dz_maxx>tpd->width-TOUCH_DZ_X) ui_color(0,255,0,255); else ui_color(255,0,0,255);
    ui_line(tpd->width-TOUCH_DZ_X,tpd->height/2,tpd->width-TOUCH_D1-TOUCH_DZ_X,tpd->height/2,5);
}

static void paint_zoom(struct touch *tpd) {
    ui_color(0, 0, 0, 50);
    ui_fill(0, 0, tpd->width, tpd->height);
    ui_color(255, 255, 255, 255);
    ui_circle(tpd->width/2, tpd->height/2, mt_radius);
    if(mt_max_radius==tpd->width/2) ui_color(0,255,0,255);
    else ui_color(255,0,0,255);
    ui_circle(tpd->width/2, tpd->height/2, tpd->width/2);
    if(mt_min_radius==tpd->width/10) ui_color(0,255,0,255);
    else ui_color(255,0,0,255);
    ui_circle(tpd->width/2, tpd->height/2, tpd->width/10);
}

static void detect_points(struct touch *tpd, int x, int y)
{
	int update_px, update_py;
	int update_x, update_y;

	if (TOUCH_L1 > y && g_pass_case1 == 0)		// up side
	{
		update_px = (x/TOUCH_L1) * TOUCH_L1;
		update_py = 0;
		update_x = (x/TOUCH_L1 + 1) * TOUCH_L1;
		update_y = 0;

		ui_color(0, 128, 0, 255);
		ui_line(update_px, update_py, update_px, update_py + TOUCH_L1, 4);
		ui_line(update_px, update_py, update_x, update_y, 4);
		ui_line(update_px, update_py + TOUCH_L1, update_x, update_y + TOUCH_L1, 4);
		ui_line(update_x, update_y, update_x, update_y + TOUCH_L1, 4);
		
		if(x > tpd->width - TOUCH_L1)
			ui_line(tpd->width, 0, tpd->width, TOUCH_L1, 4);
		ui_color(255, 255, 255, 255);
		detect_date.count_up[x/TOUCH_L1] = 1;
	}
	if (TOUCH_L1 > (tpd->width - x) && g_pass_case1 == 0)		// right side
	{
		update_px = tpd->width;
		update_py = (y/TOUCH_L1) * TOUCH_L1;
		update_x = tpd->width;
		update_y = (y/TOUCH_L1 + 1) * TOUCH_L1;

		ui_color(0, 128, 0, 255);
		if(y > TOUCH_L1)
		{
			ui_line(update_px - TOUCH_L1, update_py, update_px , update_py, 4);
			ui_line(update_px, update_py, update_x, update_y, 4);
			ui_line(update_px - TOUCH_L1, update_py, update_x - TOUCH_L1, update_y, 4);
			ui_line(update_x - TOUCH_L1, update_y, update_x, update_y, 4);
		}
		if( y > tpd->height - TOUCH_L1)
			ui_line( tpd->width - TOUCH_L1, tpd->height, tpd->width, tpd->height, 4);

		ui_color(255, 255, 255, 255);
		detect_date.count_right[y/TOUCH_L1] = 1;
	}
	if ((TOUCH_L1 > (tpd->height - y)) && g_pass_case1 == 0)		//down side
	{
		update_px = (x/TOUCH_L1) * TOUCH_L1;
		update_py = tpd->height;
		update_x = (x/TOUCH_L1 + 1) * TOUCH_L1;
		update_y = tpd->height;

		ui_color(0, 128, 0, 255);
		if(x < tpd->width - TOUCH_L1)
		{
			ui_line(update_px, update_py, update_x, update_y, 4);
			ui_line(update_px, update_py - TOUCH_L1, update_x, update_y - TOUCH_L1, 4);
			ui_line(update_px, update_py, update_px, update_py - TOUCH_L1, 4);
			ui_line(update_x, update_y - TOUCH_L1, update_x, update_y, 4);
		}
		if(x < TOUCH_L1)
			ui_line(0, tpd->height - TOUCH_L1, 0, tpd->height, 4);

		ui_color(255, 255, 255, 255);
		detect_date.count_down[x/TOUCH_L1] = 1;
	}
	if (TOUCH_L1 > x && g_pass_case1 == 0)		//left side
	{
		update_px = 0;
		update_py = (y/TOUCH_L1) * TOUCH_L1;
		update_x = 0;
		update_y = (y/TOUCH_L1 + 1) * TOUCH_L1;

		ui_color(0, 128, 0, 255);
		if( y > TOUCH_L1)
		{
			ui_line(update_px, update_py, update_px + TOUCH_L1, update_py, 4);
			ui_line(update_px, update_py, update_x, update_y, 4);
			ui_line(update_px + TOUCH_L1, update_py, update_x + TOUCH_L1, update_y, 4);
			ui_line(update_x, update_y, update_x + TOUCH_L1, update_y, 4);
		}
		if( y < TOUCH_L1)
			ui_line(0, 0, TOUCH_L1, 0, 4);

		ui_color(255, 255, 255, 255);
		detect_date.count_left[y/TOUCH_L1] = 1;
	}
	if ( g_pass_case1 == 1 && g_pass_case2 == 0 && g_pass_case3 == 0)
	{
		if((x * tpd->height) - y * tpd->width > -TOUCH_L1 * tpd->width && \
				(x * tpd->height) - y * tpd->width < TOUCH_L1 * tpd->height)
		{
			if( y / TOUCH_L1 == 0)
			{
				ui_color(0, 128, 0, 255);
				ui_line(TOUCH_L1, 0, TOUCH_L1 + ((tpd->width - TOUCH_L1) * TOUCH_L1 / (tpd->height - TOUCH_L1)), TOUCH_L1, 4);
				ui_line(0, TOUCH_L1, TOUCH_L1 + ((tpd->width - TOUCH_L1) * TOUCH_L1 / (tpd->height - TOUCH_L1)), TOUCH_L1, 4);
				ui_color(255, 255, 255, 255);
			}else
			{
				update_py = (y / TOUCH_L1) *  TOUCH_L1;
//				update_px = (y / TOUCH_L1 - 1) * ((TOUCH_L1 * tpd->width / tpd->height) + 0.2);
				update_px = (tpd->width - (TOUCH_L1 * tpd->width / tpd->height)) * ((y / TOUCH_L1) * TOUCH_L1) / (tpd->height - TOUCH_L1) - TOUCH_L1 * tpd->width / tpd->height;
				update_x = update_px + (TOUCH_L1 * tpd->width / tpd->height);
				update_y = update_py + TOUCH_L1;
				ui_color(0, 128, 0, 255);
				ui_line(update_px, update_py, update_x, update_y, 4);
				ui_line(update_px, update_py, update_x + TOUCH_L1 ,update_py, 4);
				ui_line(update_px + ((tpd->width - TOUCH_L1) * TOUCH_L1 / (tpd->height - TOUCH_L1)) + \
						TOUCH_L1, update_py, update_x + ((tpd->width - TOUCH_L1) * TOUCH_L1 / \
							(tpd->height - TOUCH_L1)) + TOUCH_L1, update_y, 4);
				ui_line( update_x, update_y, update_x + ((tpd->width - TOUCH_L1) * TOUCH_L1 / \
							(tpd->height - TOUCH_L1)) + TOUCH_L1, update_y, 4);

			}
			ui_color(255, 255, 255, 255);
			detect_date.count_oblique1[y/TOUCH_L1] = 1;
		}
		if(((tpd->width * tpd->height  - (y * tpd->width)) - x * tpd->height) <= TOUCH_L1 * tpd->height && \
			 ((tpd->width * tpd->height  - (y * tpd->width)) - x * tpd->height) >= -TOUCH_L1 * tpd->width)
		{
			update_py = (y / TOUCH_L1) * TOUCH_L1;
			update_px = tpd->width - TOUCH_L1 - (y / TOUCH_L1) * TOUCH_L1 * tpd->width / tpd->height;
			update_x = update_px - TOUCH_L1 * tpd->width / tpd->height;
			update_y = update_py + TOUCH_L1;


			ui_color(0, 128, 0, 255);
			ui_line(update_px, update_py, update_x, update_y, 4);
			ui_line(update_x, update_y, update_x + TOUCH_L1 + ((tpd->width - TOUCH_L1) * \
						TOUCH_L1 / (tpd->height - TOUCH_L1)), update_y, 4);
			ui_line(update_px, update_py, update_px + TOUCH_L1 + ((tpd->width - TOUCH_L1) * \
						TOUCH_L1 / (tpd->height - TOUCH_L1)), update_py, 4);
			if((y / TOUCH_L1) != 0)
			{
				ui_line(update_px + TOUCH_L1 + ((tpd->width - TOUCH_L1) * TOUCH_L1 / \
							(tpd->height - TOUCH_L1)), update_py, update_x + TOUCH_L1 + \
						((tpd->width - TOUCH_L1) * TOUCH_L1 / (tpd->height - TOUCH_L1)), \
						update_y, 4);

			}
			ui_color(255, 255, 255, 255);
			detect_date.count_oblique2[y/TOUCH_L1] = 1;
		}
	}

}

/*
 *place 1:down 2:left 3:up 4:right   
 */
static void paint_short_line_test(struct touch *tpd, int px ,int py ,int x,int y, int width, int place)
{
	int update_px, update_py, update_x, update_y;

	update_px = px;
	update_py = py;
	update_x = px;
	update_y = py;

	while ((update_px + TOUCH_L1) <= x && (place == 1 || place == 3)){
		if(place == 1 || place == 3)
		{
			update_px = update_px + TOUCH_L1;
			update_py = update_py;
			if(place == 1)
			{      update_x = update_px;
				update_y =  py + TOUCH_L1;
			}   
			if(place == 3)
			{      update_x = update_px;
				update_y = py - TOUCH_L1;
			}   
		}
		ui_line(update_px, update_py, update_x, update_y, width);
	}

	while ((update_py + TOUCH_L1) <= y && (place == 2 || place == 4)){
		if(place == 2 || place == 4)
		{
			update_px = update_px;
			update_py = update_py + TOUCH_L1;
			if(place == 2)
			{      update_y = update_py;
				update_px = px - TOUCH_L1;
			}   
			if(place == 4)
			{      update_x = px + TOUCH_L1;
				update_y = update_py;
			}   
		}
		ui_line(update_px, update_py, update_x, update_y, width);

	}

	while (update_py < y && (place == 5 || place == 6))
	{

		if( place == 5)
		{
			update_py = update_py;
			update_px = (x * update_py - x * TOUCH_L1) / (y - TOUCH_L1);
			update_x = update_px + (x * TOUCH_L1) / (y - TOUCH_L1) + TOUCH_L1;
			update_y = update_py;
			ui_line(update_px, update_py, update_x, update_y, width);
			update_py = update_py + TOUCH_L1;
		}
		if( place == 6 )
		{
			update_py = update_py + TOUCH_L1;
			update_px = px - (update_py * px / y);
			update_x = update_px + (px * TOUCH_L1 /y + TOUCH_L1);
			update_y = update_py;
			ui_line(update_px, update_py, update_x, update_y, width);

		}	

	}

}
static void paint_ctpTest(struct touch *tpd) 
{
     LOGD(TAG "paint_ctpTest +++++++++++++\n");
    ui_color(255,0,0,255);
    {
		
		if(0==g_pass_case1 && 0==g_pass_case2 && 0==g_pass_case3 && 0==g_pass_case4 && 0==g_pass_case5 )
		{
		     // draw the tangle
		      LOGD(TAG " draw the tangle\n");
			clear_detect_date();	//clear detect date buffer.
		      ui_color(255,0,0,255);
		      ui_line(0,0,tpd->width,0, 2);
                   ui_line(tpd->width,0,tpd->width,tpd->height, 2);
		      ui_line(tpd->width,tpd->height,0,tpd->height, 2);
		      ui_line(0,tpd->height,0,0, 2);
		
			paint_short_line_test(tpd, 0, 0, tpd->width, 0, 2, 1);		
			paint_short_line_test(tpd, tpd->width, 0, tpd->width, tpd->height, 2, 2);		
			paint_short_line_test(tpd, 0, tpd->height, tpd->width, tpd->height, 2, 3);		
			paint_short_line_test(tpd, 0, 0, 0, tpd->height, 2, 4);		

		      ui_line(TOUCH_L1,TOUCH_L1,tpd->width-TOUCH_L1,TOUCH_L1, 2);
                   ui_line(tpd->width-TOUCH_L1,TOUCH_L1,tpd->width-TOUCH_L1,tpd->height-TOUCH_L1, 2);
		      ui_line(tpd->width-TOUCH_L1,tpd->height-TOUCH_L1,TOUCH_L1,tpd->height-TOUCH_L1, 2);
		      ui_line(TOUCH_L1,tpd->height-TOUCH_L1,TOUCH_L1,TOUCH_L1, 2);
		}
		if(1==g_pass_case1 && 0==g_pass_case2 && 0==g_pass_case3 && 0==g_pass_case4 && 0==g_pass_case5 )
		{
		  // draw corner
		  LOGD(TAG " draw draw cornern");
			clear_detect_date();	//clear detect date buffer.
		  ui_color(255,0,0,255);
			ui_line(TOUCH_L1, 0, tpd->width, tpd->height - (TOUCH_L1 * tpd->height / tpd->width), 2);
			ui_line(0, TOUCH_L1, tpd->width - (TOUCH_L1 * tpd->width / tpd->height), tpd->height, 2);

			paint_short_line_test(tpd, 0, TOUCH_L1, tpd->width - (TOUCH_L1 * tpd->width / tpd->height) , tpd->height, 3, 5);
		}

		if(1==g_pass_case1 && 0==g_pass_case2 && 0==g_pass_case3 && 0==g_pass_case4 && 0==g_pass_case5 )
		{
		      LOGD(TAG " 1..................................");
			clear_detect_date();	//clear detect date buffer.
			ui_color(255, 255, 255, 255);
		       ui_color(255,0,0,255);
			ui_line(tpd->width-TOUCH_L1,0,0 , tpd->height - (TOUCH_L1 * tpd->height / tpd->width), 2);
			ui_line(tpd->width,TOUCH_L1, TOUCH_L1 * tpd->width / tpd->height , tpd->height, 2);

			paint_short_line_test(tpd, tpd->width - TOUCH_L1, 0, 0, tpd->height - (TOUCH_L1 * tpd->height / tpd->width), 2, 6);
		}

		if(1==g_pass_case1 && 1==g_pass_case2 && 1==g_pass_case3 && 0==g_pass_case4 && 0==g_pass_case5 )
		{
		  LOGD(TAG " 2..................................");
		  ui_color(255,0,0,255);
		  ui_line(0,tpd->height/2-(TOUCH_L1/2),tpd->width, tpd->height/2-(TOUCH_L1/2), 2);
               ui_line(0,tpd->height/2+(TOUCH_L1/2),tpd->width, tpd->height/2+(TOUCH_L1/2), 2);

	      paint_short_line_test(tpd, 0, tpd->height/2-(TOUCH_L1/2), tpd->width, tpd->height/2-(TOUCH_L1/2), 2, 1);
		
		  
		}
		if(1==g_pass_case1 && 1==g_pass_case2 && 1==g_pass_case3 && 1==g_pass_case4 && 0== g_pass_case5 )
		{
		  LOGD(TAG " 3..................................");
		  ui_color(255,0,0,255);
		 ui_line(tpd->width/2-(TOUCH_L1/2),0,tpd->width/2-(TOUCH_L1/2),tpd->height, 2);
               ui_line(tpd->width/2+(TOUCH_L1/2),0,tpd->width/2+(TOUCH_L1/2),tpd->height, 2);
		  
		  paint_short_line_test(tpd, tpd->width/2-(TOUCH_L1/2), 0, tpd->width/2-(TOUCH_L1/2), tpd->height, 2, 4);		
		} 
    }
    LOGD(TAG "paint_ctpTest ----------\n");
}

static void paint_linearity(struct touch *tpd, int d) {
    ui_color(255,0,0,255);
    if(d==0) {
        ui_line(TOUCH_L1,0,tpd->width,tpd->height-TOUCH_L1, 2);
        ui_line(0,TOUCH_L1,tpd->width-TOUCH_L1,tpd->height, 2);
    } else {
    ui_line(tpd->width-TOUCH_L1,0,0, tpd->height-TOUCH_L1, 2);
    ui_line(tpd->width,TOUCH_L1,TOUCH_L1, tpd->height, 2);
    }
}

static void paint_cross(struct touch *tpd, int d) {
	ui_color(255,0,0,255);
	switch(d) {
	case 0:
	        ui_line(tpd->width/10-tpd->width/20,tpd->height/10,tpd->width/10+tpd->width/20,tpd->height/10, 2);
	        ui_line( tpd->width/10,tpd->height/10-tpd->height/20, tpd->width/10,tpd->height/10+tpd->height/20, 2);
		lcd_point[0].x = tpd->width/10;//80;
		lcd_point[0].y =tpd->height/10;// 80;
		break;
	case 1:
	        ui_line(tpd->width/10-tpd->width/20,tpd->height-tpd->height/10,tpd->width/10+tpd->width/20,tpd->height-tpd->height/10, 2);
	        ui_line(tpd->width/10 ,tpd->height-tpd->height/10-tpd->height/20,tpd->width/10 ,tpd->height-tpd->height/10+tpd->height/20, 2);
		lcd_point[1].x =tpd->width/10 ;//80;
		lcd_point[1].y =tpd->height-tpd->height/10;//80;			
		break;
	case 2:
	        ui_line(tpd->width- tpd->width/10-tpd->width/20,tpd->height/10,tpd->width- tpd->width/10+tpd->width/20,tpd->height/10, 2);
	        ui_line( tpd->width- tpd->width/10,tpd->height/10-tpd->height/20, tpd->width- tpd->width/10,tpd->height/10+tpd->height/20, 2);
		lcd_point[2].x = tpd->width- tpd->width/10;//80;
		lcd_point[2].y =tpd->height/10;//80;
		break;
	case 3:
	        ui_line( tpd->width- tpd->width/10-tpd->width/20,tpd->height-tpd->height/10,tpd->width- tpd->width/10+tpd->width/20,tpd->height-tpd->height/10, 2);
	        ui_line(tpd->width- tpd->width/10,tpd->height-tpd->height/10-tpd->height/20,tpd->width- tpd->width/10,tpd->height-tpd->height/10+tpd->height/20, 2);
		lcd_point[3].x = tpd->width- tpd->width/10;
		lcd_point[3].y = tpd->height-tpd->height/10;		
		break;		
	}
}
static int hit_sensitivity(struct touch *tpd, int x, int y, int index) {
    int bl=0, br=0, bt=0, bb=0; //boundary
    int wl=0, wr=0, wt=0, wb=0; //wrap area
    int R=index/4; //round
    int side=(index%4); //side
    if(index==-1) {
      if(x>TOUCH_S1 || y<tpd->height-TOUCH_S1) return 0; 
      return 1;
    }
    switch(side) {
    case 0: bl=TOUCH_S1*R; br=bl+TOUCH_S1; bt=R*TOUCH_S1; bb=tpd->height-bt+TOUCH_S1; break;
    case 1: bt=TOUCH_S1*R; bb=bt+TOUCH_S1; bl=R*TOUCH_S1; br=tpd->width-bl; break;
    case 2: br=tpd->width-TOUCH_S1*R; bl=br-TOUCH_S1; bt=R*TOUCH_S1; bb=tpd->height-bt; break;
    case 3: bb=tpd->height-TOUCH_S1*R; bt=bb-TOUCH_S1; bl=(R+1)*TOUCH_S1; br=tpd->width-bl+TOUCH_S1; break;
    }
    switch(side) {
    case 0: wl=TOUCH_S1*R; wr=wl+TOUCH_S1; wt=R*TOUCH_S1; wb=wt+TOUCH_S1; break;
    case 1: wt=TOUCH_S1*R; wb=wt+TOUCH_S1; wr=tpd->width-R*TOUCH_S1; wl=wr-TOUCH_S1; break;
    case 2: wr=tpd->width-TOUCH_S1*R; wl=wr-TOUCH_S1; wb=tpd->height-R*TOUCH_S1; wt=wb-TOUCH_S1; break;
    case 3: wb=tpd->height-TOUCH_S1*R; wt=wb-TOUCH_S1; wl=(R+1)*TOUCH_S1; wr=wl+TOUCH_S1; break;
    }

    if(tpd->width<tpd->height && index>=2*(tpd->width/TOUCH_S1)) 
	{
	   LOGI(TAG "tpd_fwq hit 1........... \n " );
	   LOGI(TAG "tpd->width=%d \n ",tpd->width );
	   LOGI(TAG "tpd->height=%d \n ",tpd->height );
	   LOGI(TAG "index=%d \n ",index );
	   
	   return 3; //pass
	}
    if(tpd->width>=tpd->height && index>=2*(tpd->height/TOUCH_S1)) 
	{
	  LOGI(TAG "tpd_fwq hit 2........... \n " );
	  LOGI(TAG "tpd->width=%d \n ",tpd->width );
	   LOGI(TAG "tpd->height=%d \n ",tpd->height );
	   LOGI(TAG "index=%d \n ",index );
	  return 3; //pass
	}
    if(bl==wl && br==wr && bt==wt && bb==wb)
	{ 

	 LOGI(TAG "tpd_fwq hit 3........... \n " );
	 LOGI(TAG "bl=%d, br=%d,bt=%d,bb=%d \n ",bl,br,bt,bb );
	 LOGI(TAG "wl=%d, wr=%d,wt=%d,wb=%d \n ",wl,wr,wt,wb );
 	 
	 return 3;
	}//pass
    if(x<bl || x>br || y<bt || y>bb) {  return 2;} //failed
    if(x>=wl && x<=wr && y>=wt && y<=wb) {  return 1;}//wrap
    return 0; //nothing
}

static int hit_accuracy(struct touch *tpd, int x, int y) {
    int i,dx,dy,r;
    for(i=0;i<13;i++) {
        dx = x-accuracy_circles[i][0];
        dy = y-accuracy_circles[i][1];
        r  = accuracy_circles[i][2];
        if(dx*dx+dy*dy<=r*r) return i;
    } return -1;
}

static void paint_redraw(struct touch *tpd) {
    switch(tpd->scene) {
    case ITEM_TOUCH_CALIBRATION:
		if(cross_num==0) { 
			ui_color(255,0,0,255);
			ui_text(tpd->width/10,tpd->height/2,uistr_info_touch_start);
		} else if(cross_num==1) {
			paint_cross(tpd,0); ui_text(tpd->width/10,tpd->height/2,uistr_info_touch_red_cross);
		} else if(cross_num==2) {
			paint_cross(tpd,1); ui_text(tpd->width/10,tpd->height/2,uistr_info_touch_red_cross);
		} else if(cross_num==3) {
			paint_cross(tpd,2); ui_text(tpd->width/10,tpd->height/2,uistr_info_touch_red_cross);
		} else if(cross_num==4) {
			paint_cross(tpd,3); ui_text(tpd->width/10,tpd->height/2,uistr_info_touch_red_cross);
		}
		break;
	case ITEM_TOUCH_MAIN:        paint_ctpTest(tpd); break;
    case ITEM_TOUCH_LINEARITY:   paint_linearity(tpd,0); break;
    case ITEM_TOUCH_LINEARITY2:  paint_linearity(tpd,1); break;
    case ITEM_TOUCH_ACCURACY:    paint_accuracy(tpd); break;
    case ITEM_TOUCH_SENSITIVITY: paint_sensitivity(tpd); break;
    case ITEM_TOUCH_DEADZONE:    paint_deadzone(tpd); break;
    case ITEM_TOUCH_ZOOM:        paint_zoom(tpd); break;
    default: break;
    } ui_flip();
}

static void scene_init(struct touch *tpd) {
    int i;
    switch(tpd->scene) {
    case ITEM_TOUCH_ACCURACY: for(i=0;i<13;i++) accuracy_circles[i][4]=0; break;
    case ITEM_TOUCH_DEADZONE:
        dz_minx=tpd->width, dz_maxx=0; 
        dz_miny=tpd->height, dz_maxy=0; 
        break;
    case ITEM_TOUCH_ZOOM:
        mt_radius = tpd->height/4, mt_max_radius=0, mt_min_radius=tpd->width/2;
        break;
    }
}

static void clear(struct touch *tpd) {
    ui_color(0,0,0,255);
    ui_fill(0,0,tpd->width, tpd->height);
    usleep(50000);
    paint_redraw(tpd); 
    ui_flip();
}

static void part(struct touch *tpd, int mode) {
    ui_color(255, 255, 255, 255);
    ui_text(10,CHAR_HEIGHT,uistr_info_touch_pass_continue);
    ui_flip();
    tpd->scene = ITEM_TOUCH_CHOOSER;
    tpd->moveto = mode;
	if(tpd->cpv)
	{
		tpd->cpv->exit(tpd->cpv);
		g_pass_case1 = 0;
		g_pass_case2 = 0;
		g_pass_case3 = 0;
		g_pass_case4 = 0;
		g_pass_case5 = 0;
	}
}

#ifdef MTK_LCM_PHYSICAL_ROTATION
void rotate_releative_to_lcm(struct input_event *ev)
{
	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3))
	{
		// X reverse; Y reverse
		if (ev->type == EV_ABS && (ev->code == ABS_X || ev->code == ABS_MT_POSITION_X))
			ev_queue[ev_head_num].value = ui_fb_width() - ev->value - 1;
		else if (ev->type == EV_ABS && (ev->code == ABS_Y || ev->code == ABS_MT_POSITION_Y))
			ev_queue[ev_head_num].value = ui_fb_height() - ev->value - 1;
		else 
     		ev_queue[ev_head_num].value = ev->value; 
	}
	else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3))
	{
		//X,Y change; X reverse
		if (ev->type == EV_ABS && (ev->code == ABS_X || ev->code == ABS_MT_POSITION_X))
		{
			if (ev->code == ABS_X)
				ev_queue[ev_head_num].code = ABS_Y;
			else if (ev->code == ABS_MT_POSITION_X)
				ev_queue[ev_head_num].code = ABS_MT_POSITION_Y;

			ev_queue[ev_head_num].value = ev->value;
		}
		else if (ev->type == EV_ABS && (ev->code == ABS_Y || ev->code == ABS_MT_POSITION_Y))
		{
			if (ev->code == ABS_Y)
				ev_queue[ev_head_num].code = ABS_X;
			else if (ev->code == ABS_MT_POSITION_Y)
				ev_queue[ev_head_num].code = ABS_MT_POSITION_X;

			ev_queue[ev_head_num].value = ui_fb_width() - ev->value - 1;
		}
		else 
     		ev_queue[ev_head_num].value = ev->value;
	}
	else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2))
	{
		//X, Y change;  Y reverse
		if (ev->type == EV_ABS && (ev->code == ABS_X || ev->code == ABS_MT_POSITION_X))
		{
			if (ev->code == ABS_X)
				ev_queue[ev_head_num].code = ABS_Y;
			else if (ev->code == ABS_MT_POSITION_X)
				ev_queue[ev_head_num].code = ABS_MT_POSITION_Y;

			ev_queue[ev_head_num].value = ui_fb_height() - ev->value - 1;
		}
		else if (ev->type == EV_ABS && (ev->code == ABS_Y || ev->code == ABS_MT_POSITION_Y))
		{
			if (ev->code == ABS_Y)
				ev_queue[ev_head_num].code = ABS_X;
			else if (ev->code == ABS_MT_POSITION_Y)
				ev_queue[ev_head_num].code = ABS_MT_POSITION_X;

			ev_queue[ev_head_num].value = ev->value;
		}
		else 
     		ev_queue[ev_head_num].value = ev->value;
	}
	else
    	ev_queue[ev_head_num].value = ev->value; 		
}
#endif
void add_event(struct input_event *ev) {
     ev_queue[ev_head_num].type = ev->type; 
     ev_queue[ev_head_num].code = ev->code; 
#if 0 //defined(MTK_LCM_PHYSICAL_ROTATION)
	 rotate_releative_to_lcm(ev);
#else
     ev_queue[ev_head_num].value = ev->value; 
#endif
     pthread_mutex_lock(&event_mutex);
     ev_head_num++;
     if (ev_head_num == 2048) ev_head_num = 0;
     pthread_mutex_unlock(&event_mutex);
}

void del_event() {
     pthread_mutex_lock(&event_mutex);
     ev_tail_num++; 
     if (ev_tail_num == 2048) ev_tail_num = 0;
     pthread_mutex_unlock(&event_mutex);
}

void mmul(float matrix1[][3], float matrix2[][3], float result[][3]) 
{
    int i, j, k;
    
    for (i = 0; i < 3; i++) 
    {
        for (j = 0; j < 3; j++) 
        {
            result[i][j] = 0;
            for (k = 0; k < 3; k++) 
                result[i][j] += (matrix1[i][k] * matrix2[k][j]);
        }
    }
}

void identity(float src[][3]) 
{
    int i,j;
    for (i = 0; i < 3; i++) 
        for(j = 0; j < 3; j++) 
            src[i][j]=0;
            
    for (i = 0; i < 3; i++) 
        src[i][i] = 1;
}
void duplicate(float src[][3], float des[][3]) 
{
    int i, j;
    for (i = 0; i < 3; i++) 
        for (j = 0; j < 3; j++)
            des[i][j] = src[i][j];
}

void vadd(float vec1[3], float vec2[3]) 
{
    int i;
    for (i = 0; i < 3; i++) vec1[i] = vec1[i] + vec2[i];
}

void vmul(float vec[3], float coef) 
{
    int i;
    for (i = 0; i < 3; i++) vec[i] *= coef;
}

void vrmul(float src[3], float coef, float des[3]) 
{
    int i;
    for(i = 0; i < 3; i++) des[i] = src[i] * coef;
}

int inverse(float src[][3], float des[][3])
{
    float dup[3][3], bak[3];
    int i, j;
    
    identity(des);
    
    duplicate(src, dup);
    
    if (dup[0][0] == 0 && dup[1][0] == 0 && dup[2][0] == 0) 
        return FAIL;
        
    if (dup[0][0] == 0) 
    {
        vadd(dup[0], dup[1]);
        vadd(des[0], des[1]);
    } 
    else 
    {
        vadd(dup[0], dup[2]);
        vadd(des[0], des[2]);
    }
    
    vmul(des[0], 1.0f / dup[0][0]);
    vmul(dup[0], 1.0f / dup[0][0]);
    
    vrmul(des[0], -dup[1][0], bak);
    vadd(des[1], bak);
    vrmul(dup[0], -dup[1][0], bak);
    vadd(dup[1], bak);
    
    vrmul(des[0], -dup[2][0], bak);
    vadd(des[2], bak);
    vrmul(dup[0], -dup[2][0], bak);
    vadd(dup[2], bak);
    
    if (dup[1][1] == 0 && dup[2][1] == 0) return FAIL;
        
    if (dup[1][1] == 0) 
    { 
        vadd(dup[1], dup[2]);
        vadd(des[1], des[2]); 
    }
    
    vmul(des[1], 1.0f / dup[1][1]);
    vmul(dup[1], 1.0f / dup[1][1]);
    
    vrmul(des[1], -dup[0][1], bak);
    vadd(des[0], bak);
    vrmul(dup[1], -dup[0][1], bak);
    vadd(dup[0], bak);
    
    vrmul(des[1], -dup[2][1], bak);
    vadd(des[2], bak);
    vrmul(dup[1], -dup[2][1], bak);
    vadd(dup[2], bak);
    
    if (dup[2][2] == 0) return FAIL;
    
    vmul(des[2], 1.0f / dup[2][2]);
    vmul(dup[2], 1.0f / dup[2][2]);
    
    vrmul(des[2], -dup[0][2], bak);
    vadd(des[0], bak);
    vrmul(dup[2], -dup[0][2], bak);
    vadd(dup[0], bak);
    
    vrmul(des[2], -dup[1][2], bak);
    vadd(des[1], bak);
    vrmul(dup[2], -dup[1][2], bak);
    vadd(dup[1], bak);
    
    return PASS;
}
void calibrate() 
{
    int i, j, tmp_x, tmp_y;
    
    float ret_matrix[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    float inv_matrix[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    
    float tpd_matrix[3][3] = {
        {tpd_point[0].x, tpd_point[0].y, 1},
        {tpd_point[1].x, tpd_point[1].y, 1},
        {tpd_point[2].x, tpd_point[2].y, 1}
    };
    
    float lcd_matrix[3][3] = {
        {lcd_point[0].x, lcd_point[0].y,0},
        {lcd_point[1].x, lcd_point[1].y,0},
        {lcd_point[2].x, lcd_point[2].y,0}
    };
    LOGI(TAG "tpd_matrix =");
    for (i = 0; i < 3; i++) 
        for (j = 0; j < 3; j++) 
            LOGI(TAG "%10.6f ", tpd_matrix[i][j]);
    LOGI(TAG "\n");	

    LOGI(TAG "lcd_matrix =");
    for (i = 0; i < 3; i++) 
        for (j = 0; j < 3; j++) 
            LOGI(TAG "%10.6f ", lcd_matrix[i][j]);
    LOGI(TAG "\n");		
    
    inverse(tpd_matrix, inv_matrix);
    mmul(inv_matrix, lcd_matrix, ret_matrix);
    for (i = 0; i < 2; i++) 
        for (j = 0; j < 3; j++) 
            calmat[i][j] = (int)(ret_matrix[j][i] * 4096);
    
    tmp_x = (int)((calmat[0][0] * tpd_point[3].x + calmat[0][1] * tpd_point[3].y + calmat[0][2]) / (long)4096);
    tmp_y = (int)((calmat[1][0] * tpd_point[3].x + calmat[1][1] * tpd_point[3].y + calmat[1][2]) / (long)4096);
    
    bias[0] = lcd_point[3].x - tmp_x;
    bias[1] = lcd_point[3].y - tmp_y;
    
    for (i = 0; i < 2; i++) 
        for (j = 0; j < 3; j++) 
            matrix[i * 3 + j] = (int)calmat[i][j];
            
    matrix[6] = bias[0];
    matrix[7] = bias[1];
    
}
int save()
{
    int i;
    int def[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int des[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    FILE *file = NULL;
    
    file = fopen("/sys/module/tpd_setting/parameters/tpd_def_calmat", "r");
    if (!file) 
    {
        printf("tpd_calibrator: cannot open /sys/module/tpd_setting/parameters/tpd_def_calmat\n");
        return FAIL;
    }
    
    for (i = 0; i < 8; i++) 
        fscanf(file, "%d,",&(def[i]));
    fclose(file);
    
    
    des[0] = (int)((matrix[0] * (long long)def[0] + matrix[1] * (long long)def[3]) / 4096);
    des[1] = (int)((matrix[0] * (long long)def[1] + matrix[1] * (long long)def[4]) / 4096);
    des[2] = (int)((matrix[0] * (long long)def[2] + matrix[1] * (long long)def[5] + (long long)matrix[2] * 4096) / 4096);
    des[3] = (int)((matrix[3] * (long long)def[0] + matrix[4] * (long long)def[3]) / 4096);
    des[4] = (int)((matrix[3] * (long long)def[1] + matrix[4] * (long long)def[4]) / 4096);
    des[5] = (int)((matrix[3] * (long long)def[2] + matrix[4] * (long long)def[5] + (long long)matrix[5] * 4096) / 4096);
    des[6] = matrix[6];
    des[7] = matrix[7];

    file = fopen("/data/misc/touchpanel/calibration", "w");
    if (!file) 
    { 
        printf("tpd_calibrator: cannot open /data/misc/touchpanel/calibration\n");
        return FAIL;
    }
    
    for (i = 0; i < 8; i++) 
        fprintf(file, "%d,", des[i]);
    fclose(file);
    
    file = fopen("/sys/module/tpd_setting/parameters/tpd_calmat", "w");
    if(!file) 
    {
        printf("tpd_calibrator: cannot open /sys/module/tpd_setting/parameters/tpd_calmat\n");
        return FAIL;
    }
    
    for (i = 0; i < 8; i++) 
        fprintf(file, "%d,", des[i]);
    fclose(file);
    
    for (i = 0; i < 8; i++) 
        printf("%d, ", des[i]);
   
    return PASS; 
}
int calibration_init()
{
    int i;
    int def[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    FILE *file = NULL;
    
    file = fopen("/sys/module/tpd_setting/parameters/tpd_def_calmat", "r");
    if (!file) 
    {
        printf("tpd_calibrator: cannot open /sys/module/tpd_setting/parameters/tpd_def_calmat\n");
        return FAIL;
    }
    
    for (i = 0; i < 8; i++) 
        fscanf(file, "%d,",&(def[i]));
    fclose(file);
    file = fopen("/data/misc/touchpanel/calibration", "w");
    if (!file) 
    { 
        printf("tpd_calibrator: cannot open /data/misc/touchpanel/calibration\n");
        return FAIL;
    }
    
    for (i = 0; i < 8; i++) 
        fprintf(file, "%d,", def[i]);
    fclose(file);
    
    file = fopen("/sys/module/tpd_setting/parameters/tpd_calmat", "w");
    if(!file) 
    {
        printf("tpd_calibrator: cannot open /sys/module/tpd_setting/parameters/tpd_calmat\n");
        return FAIL;
    }
    
    for (i = 0; i < 8; i++) 
        fprintf(file, "%d,", def[i]);
    fclose(file);
    
    for (i = 0; i < 8; i++) 
        printf("%d, ", def[i]);
   
    return PASS; 
}
static int touch_deviation(int x1, int y1, int x2, int y2) {
	int temp1=0, temp2=0;

	temp1=x1*x1-2*x1*x2+x2*x2;
	temp2=y1*y1-2*y1*y2+y2*y2;

	
	LOGI(TAG "x1=%d, y1=%d, x2=%d, y2=%d, temp1=%d, temp2=%d.\n", x1, y1, x2, y2, temp1, temp2);
	if(temp1<200 && temp2<200) {
		return 1;
	}
	return 0;
}
static void *touch_update_thread(void *priv) {
    struct touch *tpd = (struct touch *)priv;
    struct input_event data;
    int x=0,y=0,p=0,px=0,py=0,pp=0,d=0,count=0;
    int x2=0,y2=0,p2=0,pc=0, point_count=0;
    bool enter = false, release = false;
    bool cal_flag = false;
    line_t line = {0,0,0,0,2,COLOR_WHITE};
    char ptloc[80];
    int status = 0,i=0,distance=0, p_distance=0;
    int zoomrate = 0;

	bool detect_flags;
    LOGD(TAG "enter update thread\n");
    while (1) {
        
        if (tpd->exit_thd) break;
            pthread_mutex_lock(&enter_mutex);
        if (tpd->scene==ITEM_TOUCH_CHOOSER) {
            pthread_mutex_unlock(&enter_mutex);
            //if(read(tpd->event_fd,&data,sizeof(struct input_event))<=0) continue;
            if (ev_head_num == ev_tail_num) continue;
               
            data.type = ev_queue[ev_tail_num].type;
            data.code = ev_queue[ev_tail_num].code;
            data.value = ev_queue[ev_tail_num].value;
            del_event();
        } else {
            if(tpd->enter_scene==true) {
                tpd->enter_scene = false;
                enter = true; release = true;
                clear(tpd);
            }
            pthread_mutex_unlock(&enter_mutex);
         
            if (ev_head_num == ev_tail_num) continue;
               
            data.type = ev_queue[ev_tail_num].type;
            data.code = ev_queue[ev_tail_num].code;
            data.value = ev_queue[ev_tail_num].value;
            del_event();
           
            LOGD("[touch] data.value:%d\n", data.value);
            if(data.type==EV_ABS && data.code==ABS_X) { x=data.value; }
            else if(data.type==EV_ABS && data.code==ABS_MT_POSITION_X) { x=data.value; }
            else if(data.type==EV_ABS && data.code==ABS_Y) { y=data.value; }
            else if(data.type==EV_ABS && data.code==ABS_MT_POSITION_Y) { y=data.value; }
            else if(data.type==EV_ABS && data.code==ABS_PRESSURE) { p=data.value; }
            else if(data.type==EV_ABS && data.code==ABS_MT_TOUCH_MAJOR) { p=data.value; }
            else if(data.type==EV_KEY && data.code==BTN_TOUCH) { d=data.value; }
            else if(data.type==EV_SYN && data.code==SYN_MT_REPORT) {
                //LOGD("[touch] x:%d y:%d p:%d\n", x,y,p);
                if(pc==0) { x2 = x; y2 =y; p2 = p; }
                pc++;
                if(p==0) distance = 0;
            } else if(data.type==EV_SYN) {
                //LOGD("[touch] synced. point count:%d\n",pc);
                point_count++;
                switch(tpd->scene) {
		case ITEM_TOUCH_CALIBRATION:
			paint_redraw(tpd);
			switch(cross_num) {
			case 0:
				break;
			case 1:
				if(touch_deviation(x, y, lcd_point[0].x, lcd_point[0].y)) {
				retry_flag=false;
				tpd_point[0].x = x;
				tpd_point[0].y = y;
				}
				else  {
					cross_num = 1;
					retry_flag=true;
				}
				break;
			case 2:
				if(touch_deviation(x, y, lcd_point[1].x, lcd_point[1].y)) {				
				retry_flag=false;
				tpd_point[1].x = x;
				tpd_point[1].y = y;
				}
				else  {
					cross_num =2;
					retry_flag=true;
				}				
				break;
			case 3:
				if(touch_deviation(x, y, lcd_point[2].x, lcd_point[2].y)) {						
				retry_flag=false;
				tpd_point[2].x = x;
				tpd_point[2].y = y;
				}
				else  {
					cross_num = 3;
					retry_flag=true;
				}				
				break;
			case 4:
				if(touch_deviation(x, y, lcd_point[3].x, lcd_point[3].y)) {						
				retry_flag=false;
				tpd_point[3].x = x;
				tpd_point[3].y = y;
				if(cal_flag==false)
				{
					calibrate();
					save();
					LOGI(TAG "calibration finished.\n");
					cal_flag=true;
				}
				}
				else  {
					cross_num = 4;
					retry_flag=true;
				}				
				break;
			default:
			break;
			}
			
			if(d==0 && retry_flag==false) {
				cross_num++;
				clear(tpd);
			} else if(d==0 && retry_flag==true) {
				clear(tpd);
			}
			if(enter==false&&cross_num==5 && retry_flag==false) {
				tpd->menu.m_selected++;
				part(tpd,ITEM_TOUCH_CHOOSER);
				cross_num = 0;
				cal_flag=false;
			}
			break;	
			    case ITEM_TOUCH_MAIN:
					 LOGD(TAG "enter = %d\n",enter);
						detect_points(tpd, x, y);
					if(0 == g_pass_case1)
					{					   
					  if((x>0&&x<TOUCH_L1)&& (y>0&&y<TOUCH_L1))
					  {
					    	tangle_px1 = x;
						tangle_px1 = y;
						tangle_l1 =1;
						LOGD(TAG "tangle_px1=%d tangle_py1=%d\n",tangle_px1,tangle_py1);
					  }
					  if(x> tpd->width-TOUCH_L1 && (y >0&&y<TOUCH_L1))
					  {
						tangle_px2 = x;
						tangle_py2 = y;
						tangle_l2 =1;
						LOGD(TAG "tangle_px2=%d tangle_py2=%d\n",tangle_px2,tangle_py2);
					    
					  }
					  if(x> tpd->width-TOUCH_L1 && y >tpd->height-TOUCH_L1)
					  {
					    	tangle_px3 = x;
						tangle_py3 = y;
						tangle_l3 =1;
						LOGD(TAG "tangle_px3=%d tangle_py3=%d\n",tangle_px3,tangle_py3);
					    
					  }
					  if(x<TOUCH_L1 && y >tpd->height-TOUCH_L1)
					  {
						tangle_px4 = x;
						tangle_py4 = y;
						tangle_l4 =1;
					  	LOGD(TAG "tangle_px4=%d tangle_py4=%d\n",tangle_px4,tangle_py4);
					  }

					  	LOGD(TAG "draw circle length=%d\n",((tangle_px2-tangle_px1)+(tangle_py3-tangle_py2)+(tangle_px3-tangle_px4)+(tangle_py4-tangle_py1)));
						LOGD(TAG "pass circle length=%d\n",(2*(tpd->width-2*TOUCH_L1)+2*(tpd->height-2*TOUCH_L1)));
                       LOGD(TAG "tangle_l1=%d\n",tangle_l1);
						LOGD(TAG "tangle_l2=%d\n",tangle_l2);
						LOGD(TAG "tangle_l3=%d\n",tangle_l3);
						LOGD(TAG "tangle_l4=%d\n",tangle_l4);
#if 1
					detect_flags = 1;
					i = 0;
					while(i < (tpd->width/TOUCH_L1))
					{
						if(detect_date.count_up[i] != 1 || detect_date.count_down[i] != 1)
						{
							detect_flags = 0;
							break;
						}else
							i++;
					}
					i = 0;
					while(i < tpd->height/TOUCH_L1)
					{
						if(detect_date.count_left[i] != 1 || detect_date.count_right[i] != 1)
						{
							detect_flags = 0;
							break;
						}else
							i++;
					}
					if(detect_flags != 0)
					{
						tpd->menu.m_selected++; 
						part(tpd,ITEM_TOUCH_MAIN);
						g_pass_case1 =1;
						clear_touch_temp_data();  
						//part(tpd,ITEM_TOUCH_LINEARITY);
						clear_detect_date();
					}
					i = 0;	
			}
					  
#endif
			if(1 == g_pass_case1 && 0 == g_pass_case2 && 0 == g_pass_case3)
			{
				detect_flags = 1;
				i = 0;
				while( i < tpd->height/TOUCH_L1 && (g_pass_case2 == 0) && (g_pass_case3 == 0))
				{
					if(detect_date.count_oblique1[i] != 1 || detect_date.count_oblique2[i] != 1)
					{
						detect_flags = 0;
						break;
					}else
						i++;
				}
				if(detect_flags != 0)
				{
					g_pass_case2 = 1;
					g_pass_case3 = 1;
				}
				i = 0;

			}
			if(1 == g_pass_case1 && 1 == g_pass_case2 && 1 ==g_pass_case3)
			{

				tpd->menu.m_selected++;

				part(tpd, ITEM_TOUCH_CHOOSER);
				clear_detect_date();
				break;
			}
			if(1 == g_pass_case1 && 1 == g_pass_case2 && 1 ==g_pass_case3 && 1 == g_pass_case4)
			{
				if(y> 0 && y<TOUCH_L1)
					   {
						//tangle_px1 = x;
						tangle_py1 = y;
						tangle_l1 =1;
						LOGD(TAG "tangle_px1=%d tangle_py1=%d\n",tangle_px1,tangle_py1);
					   }
				if(y> tpd->height-TOUCH_L1 && y<tpd->height)
					   {
						//tangle_px2 = x;
						tangle_py2 = y;
						tangle_l2 =1;
						LOGD(TAG "tangle_px2=%d tangle_py2=%d\n",tangle_px2,tangle_py2);
					   }
#if 1
				detect_flags = 1;
				i = 0;
				while(i < tpd->height/TOUCH_L1)
				{
					if(detect_date.count_mid_upright[i] != 1)
					{
						detect_flags = 0;
						break;
					}else
						i++;
				}

				if(detect_flags != 0)
				{
                         			g_pass_case5 =1;
						clear(tpd);
						paint_ctpTest(tpd);
						  
						 
						tpd->menu.m_selected++; 
						part(tpd, ITEM_TOUCH_CHOOSER);
						clear_detect_date();
				}
				i = 0;
#endif
		    }
					
                    break;
                case ITEM_TOUCH_LINEARITY:
                    if(enter==true && (x>TOUCH_L1 || y>TOUCH_L1)) continue;
                    if(enter==false && d==0) { enter=true; clear(tpd); continue; }
                    if(enter==false && (((x*tpd->height)/tpd->width)-y<-TOUCH_L1 || 
                                        ((x*tpd->height)/tpd->width)-y>TOUCH_L1)) { 
                        enter=true; clear(tpd); continue;
                    }
                    if(enter==false && x>tpd->width-TOUCH_L1 && y>tpd->height-TOUCH_L1) { 
                        part(tpd,ITEM_TOUCH_LINEARITY2);
                    }
                    break;
                case ITEM_TOUCH_LINEARITY2:
                    if(enter==true && (x<tpd->width-TOUCH_L1 || y>TOUCH_L1)) continue;
                    if(enter==false && d==0) { enter=true; clear(tpd); continue; }
                    if(enter==false && (((x*tpd->height)/tpd->width)+y<tpd->height-TOUCH_L1 || 
                                        ((x*tpd->height)/tpd->width)+y>tpd->height+TOUCH_L1)) { 
                        enter=true; clear(tpd); continue;
                    }
                    if(enter==false && x<TOUCH_L1 && y>tpd->height-TOUCH_L1) { 
                        tpd->menu.m_selected++;
                        part(tpd,ITEM_TOUCH_CHOOSER); 
                    } 
                    break;
                case ITEM_TOUCH_ACCURACY:
                    if(enter==true) scene_init(tpd); //for(i=0;i<13;i++) accuracy_circles[i][4]=0;
                    for(i=0;i<13;i++) if(accuracy_circles[i][4]==0) break;
                    if(i==13) { 
                        for(i=0;i<13;i++) accuracy_circles[i][4]=0;
                        tpd->menu.m_selected++; 
                        part(tpd,ITEM_TOUCH_CHOOSER); 
                        continue;
                    }
                    i = hit_accuracy(tpd,x,y);
                    if(i<0) { enter=true; clear(tpd); continue; }
                    else if(accuracy_circles[i][4]==0) {
                      accuracy_circles[i][4]=1;
                      paint_redraw(tpd); 
                    }
                    break;
                case ITEM_TOUCH_SENSITIVITY:
                    if(enter==true && hit_sensitivity(tpd,x,y,-1)==0) continue;
                    if(enter==true) { status = 0; }
                    if(enter==false && d==0) { enter=true; clear(tpd); continue; }
                    switch(hit_sensitivity(tpd,x,y,status)) {
                    case 1: status++; break;
                    case 2: enter=true; clear(tpd); continue; break;
                    case 3: tpd->menu.m_selected++; part(tpd,ITEM_TOUCH_CHOOSER); break;
                    default: break;
                    }
                    break;
                case ITEM_TOUCH_DEADZONE:
                    if(enter==true) scene_init(tpd); 
                    /*{
                        dz_minx=tpd->width, dz_maxx=0;
                        dz_miny=tpd->height, dz_maxy=0;
                        clear(tpd);
                    }*/
                    if(dz_minx>x) { dz_minx=x; if(dz_minx<TOUCH_DZ_X) paint_redraw(tpd);  }
                    if(dz_maxx<x) { dz_maxx=x; if(dz_maxx>tpd->width-TOUCH_DZ_X) paint_redraw(tpd);  }
                    if(dz_miny>y) { dz_miny=y; if(dz_miny<TOUCH_DZ_Y) paint_redraw(tpd);  }
                    if(dz_maxy<y) { dz_maxy=y; if(dz_maxy>tpd->height-TOUCH_DZ_Y) paint_redraw(tpd);  }
                    if(dz_minx<TOUCH_DZ_X && dz_maxx>tpd->width-TOUCH_DZ_X &&
                       dz_miny<TOUCH_DZ_Y && dz_maxy>tpd->height-TOUCH_DZ_Y) {
                        dz_minx=tpd->width, dz_maxx=0;
                        dz_miny=tpd->height, dz_maxy=0;
                        tpd->menu.m_selected++; part(tpd, ITEM_TOUCH_CHOOSER);
                    }
                    break;
                case ITEM_TOUCH_ZOOM:
                    if(enter==true) scene_init(tpd); //mt_radius = tpd->height/4, mt_max_radius=0, mt_min_radius=tpd->width/2;
                    if(pc==2 && p) {
                        p_distance = distance;
                        distance = ((x2-x)*(x2-x)+(y2-y)*(y2-y));
                        if(p_distance==0) p_distance = distance;
                        if(distance-p_distance>0) zoomrate=10; else zoomrate=-10;
                        if(distance==p_distance) zoomrate=0;
                        mt_radius += ((int)(zoomrate));
                        if(mt_radius>tpd->width/2) mt_radius=tpd->width/2;
                        if(mt_radius<tpd->width/10) mt_radius=tpd->width/10;
                        if(mt_max_radius<mt_radius) mt_max_radius = mt_radius;
                        if(mt_min_radius>mt_radius) mt_min_radius = mt_radius;
                    }
                    paint_redraw(tpd);
                    if(mt_max_radius==tpd->width/2 && mt_min_radius==tpd->width/10) {
                        mt_max_radius=0, mt_min_radius=0;
                        usleep(20000);
                        tpd->menu.m_selected++; part(tpd, ITEM_TOUCH_CHOOSER);
                    }
                    break;
                }
                
                if(enter==true || release==true) { px=x;py=y;enter=false; release=false; }
                if(tpd->scene!=ITEM_TOUCH_ACCURACY && tpd->scene!=ITEM_TOUCH_ZOOM && tpd->scene!=ITEM_TOUCH_CHOOSER) {
                    if(tpd->scene==ITEM_TOUCH_FREEMODE) {
                        //ui_color(0, 0, 0, 10);
                        //ui_fill(0, 21, tpd->width, tpd->height-10);
                    }
                    
                    ui_color(255,255,255,255);
                    ui_line(px,py,x,y,2);
                    
                    LOGD(TAG "freemode: x = %d, y = %d ; px = %d, py = %d\n", x, y, px, py);
                    if(tpd->scene==ITEM_TOUCH_FREEMODE) {
                        sprintf(ptloc,"x:%4d y:%4d",x,y);
                        ui_color(0, 0, 0, 255);
                        ui_fill(0, 0, tpd->width, CHAR_HEIGHT);
                        ui_color(255, 255, 255, 255);
                        ui_text(10,CHAR_HEIGHT,ptloc);
                    }
                }
                if (!(point_count%TOUCH_UPDATE_FR_DIV))
                    ui_flip();
                px=x,py=y;pc=0;
                if(d==0) release=true;
                point_count=point_count%TOUCH_UPDATE_FR_DIV;
            }
        } 
    }
    LOGD(TAG "exit thread\n");
    pthread_exit(NULL);
    return NULL;
}

void setup_cpv(struct touch *tpd) {
    switch(tpd->scene) {
	
    case ITEM_TOUCH_CALIBRATION: cross_num=0; tpd->cpv = &(tpd->empty); break;
	case ITEM_TOUCH_MAIN: 
    case ITEM_TOUCH_LINEARITY: case ITEM_TOUCH_LINEARITY2:
    case ITEM_TOUCH_ACCURACY:  case ITEM_TOUCH_SENSITIVITY:
    case ITEM_TOUCH_DEADZONE:  tpd->cpv = &(tpd->empty); break;
    case ITEM_TOUCH_FREEMODE: tpd->cpv = &(tpd->freemode); break;
    case ITEM_TOUCH_ZOOM: tpd->cpv = &(tpd->zoom); break;
    default: tpd->cpv = NULL; break;
    }
}

/*void setup_cpv(struct touch *tpd) {
    switch(tpd->scene) {
    case ITEM_TOUCH_LINEARITY: tpd->cpv = &(tpd->linearity); break;
    case ITEM_TOUCH_LINEARITY2: tpd->cpv = &(tpd->linearity2); break;
    case ITEM_TOUCH_ACCURACY: tpd->cpv = &(tpd->accuracy); break;
    case ITEM_TOUCH_SENSITIVITY: tpd->cpv = &(tpd->sensitivity); break;
    case ITEM_TOUCH_DEADZONE: tpd->cpv = &(tpd->deadzone); break;
    case ITEM_TOUCH_FREEMODE: tpd->cpv = &(tpd->freemode); break;
    case ITEM_TOUCH_ZOOM: tpd->cpv = &(tpd->zoom); break;
    default: tpd->cpv = NULL; break;
    }
}*/

void setup_circles(struct touch *tpd) {
    int i,j;
    int circles[][5] = {
        {TOUCH_R1,TOUCH_R1,TOUCH_R1,COLOR_RED,0},
        {tpd->width-TOUCH_R1,TOUCH_R1,TOUCH_R1,COLOR_RED,0},
        {TOUCH_R1,tpd->height-TOUCH_R1,TOUCH_R1,COLOR_RED,0},
        {tpd->width-TOUCH_R1,tpd->height-TOUCH_R1,TOUCH_R1,COLOR_RED,0},
        {tpd->width/2,TOUCH_R1,TOUCH_R1,COLOR_RED,0},
        {TOUCH_R1,tpd->height/2,TOUCH_R1,COLOR_RED,0},
        {tpd->width/2,tpd->height-TOUCH_R1,TOUCH_R1,COLOR_RED,0},
        {tpd->width-TOUCH_R1,tpd->height/2,TOUCH_R1,COLOR_RED,0},
        {tpd->width/4,tpd->height/4,TOUCH_R2,COLOR_RED,0},
        {3*tpd->width/4,tpd->height/4,TOUCH_R2,COLOR_RED,0},
        {tpd->width/4,3*tpd->height/4,TOUCH_R2,COLOR_RED,0},
        {3*tpd->width/4,3*tpd->height/4,TOUCH_R2,COLOR_RED,0},
        {tpd->width/2, tpd->height/2, TOUCH_R2,COLOR_RED,0},
    };
    for(i=0;i<13;i++) for(j=0;j<5;j++) accuracy_circles[i][j] = circles[i][j];
}

int touch_entry(struct ftm_param *param, void *priv) {
    bool exit = false;
    struct touch *tpd = (struct touch *)priv;
    line_t *sensitivity_lines = NULL;
    FILE *fp;
    char type = 0;
    /*
    line_t linearity_lines[] = {
        {TOUCH_D1,0,tpd->width, tpd->height-TOUCH_D1, 5, COLOR_RED},
        {0,TOUCH_D1,tpd->width-TOUCH_D1, tpd->height, 5, COLOR_RED},
    };
    line_t linearity_lines2[] = {
        {tpd->width-TOUCH_D1,0,0, tpd->height-TOUCH_D1, 5, COLOR_RED},
        {tpd->width,TOUCH_D1,TOUCH_D1, tpd->height, 5, COLOR_RED},
    };*/

    /*line_t deadzone_lines[] = {
        {tpd->width/2,0,tpd->width/2,tpd->height,5,COLOR_RED},
        {0,tpd->height/2,tpd->width,tpd->height/2,5,COLOR_RED},
    };*/

    line_t freemode_lines[] = {
        {0,20,tpd->width,20,2,COLOR_WHITE},
    };
    setup_circles(tpd);

    LOGD(TAG "%s\n", __FUNCTION__);

    //if (tpd->event_fd==-1 && (tpd->event_fd = open("/dev/input/event0", O_RDONLY)) < 0) {
    //  LOGE(TAG "failed to open /dev/input/event0. cannot get touch panel event.");
    //  fprintf(stderr, "cannot open /dev/input/event0.\n");
    //  return 0;
    //}
    //memset (ev_fds, 0, sizeof(ev_fds));
    //ev_fds[0].fd = tpd->event_fd;
    //ev_fds[0].events = POLLIN;

    //fcntl(tpd->event_fd, F_SETFL, O_NONBLOCK);

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
        tpd->menu.set_items(&(tpd->menu), rtype_touch_items, 0);
    } else {
        tpd->menu.set_items(&(tpd->menu), ctype_touch_items, 0);
    }
    
    pthread_create(&(tpd->update_thd), NULL, touch_update_thread, priv);
	clear_touch_result();
    do {
    	//LOGD(TAG "%s\n", __FUNCTION__);
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
	    if(tpd->scene != ITEM_TOUCH_CHOOSER || g_pass_case1 != 0)
            {
            tpd->enter_scene = true;
            }
            pthread_mutex_unlock(&enter_mutex);

            tpd->moveto = ITEM_TOUCH_CHOOSER;
            setup_cpv(tpd);
            break;
        }
        if(exit) tpd->exit_thd = true;
    } while (!tpd->exit_thd);
    pthread_join(tpd->update_thd, NULL);
    free(sensitivity_lines);
    return 0;
}

int touch_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct touch *tpd;

    LOGD(TAG "%s\n", __FUNCTION__);
    calibration_init();
    mod = ftm_alloc(ITEM_TOUCH, sizeof(struct touch));
    tpd  = mod_to_touch(mod);
    memset(tpd, 0x00, sizeof(*tpd));
    tpd->mod = mod; 
    //tpd->event_fd = -1;
    tpd->width = ui_fb_width();
    tpd->height = ui_fb_height();
    if (!mod) return -ENOMEM;
    ret = ftm_register(mod, touch_entry, (void*)tpd);
    return 0;
}

