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

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MINIUI_H_
#define _MINIUI_H_

#include "common.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MAX_COLS        64
#define MAX_ROWS        128

/* key definition */
#define UI_KEY_DOWN     CUST_KEY_DOWN
#define UI_KEY_VOLDOWN  CUST_KEY_VOLDOWN
#define UI_KEY_UP       CUST_KEY_UP
#define UI_KEY_VOLUP    CUST_KEY_VOLUP
#define UI_KEY_CENTER   CUST_KEY_CENTER
#define UI_KEY_CONFIRM  CUST_KEY_CONFIRM
#define UI_KEY_BACK     CUST_KEY_BACK
#define UI_KEY_LEFT     CUST_KEY_LEFT
#define UI_KEY_RIGHT    CUST_KEY_RIGHT

/* color definition */
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_BLACK     0x000000FF
#define COLOR_RED       0xFF0000FF
#define COLOR_GREEN     0x00FF00FF
#define COLOR_BLUE      0x0000FFFF
#define COLOR_YELLOW    0xFFFF00FF
#define COLOR_PINK      0xFF00FFFF

#define color_r(c)    (((c) >> 24) & 0xFF)
#define color_g(c)    (((c) >> 16) & 0xFF)
#define color_b(c)    (((c) >>  8) & 0xFF)
#define color_a(c)    (((c) >>  0) & 0xFF)

#define set_color(c,r,g,b,a)  \
    do { (c) = ((r) << 24 | (g) << 16 | (b) << 8 | (a)); } while(0)

#define get_color(c,r,g,b,a)  \
    do { (r) = color_r(c); \
         (g) = color_g(c); \
         (b) = color_b(c); \
         (a) = color_a(c); \
    } while(0)

#define init_text(t,s,c) \
    do { (t)->string = (s); \
         (t)->color = (c); \
    } while(0)

#define init_ctext(ct,s,c,x,y) \
    do { (ct)->string = (s); \
         (ct)->color = (c); \
         (ct)->x = (x); \
         (ct)->y = (y); \
    } while(0)

#define init_line(t,sx,sy,dx,dy,w,c) \
    do { (t)->sx = (sx); \
         (t)->sy = (sy); \
         (t)->dx = (dx); \
         (t)->dy = (dy); \
         (t)->width = (w); \
         (t)->color = (c); \
    } while(0)    

#define item(i, n) { .id = (i), .name = (n) , .background =0 ,.mode =0}


/* type definition */
typedef int (*khandler)(int key, void *priv);
typedef unsigned int color_t;

typedef struct {
    const char *string;
    color_t     color;
} text_t;

typedef struct {
    const char *string;
    color_t     color;
    int         x;
    int         y;
} ctext_t;

typedef struct {
    void    *surface;
    int      x;
    int      y;
} image_t;

typedef struct {
    int         id;
    const char *name;
    color_t     background;
	int mode;
} item_t;

typedef struct {
    int      x;
    int      y;
    int      radius;
    color_t  color;
} point_t;

typedef struct {
    int      sx;
    int      sy;
    int      dx;
    int      dy;
    int      width;
    color_t  color;
} line_t;

typedef struct {
    int      x;
    int      y;    
    int      radius;
    color_t  color;
} circle_t;

struct itemview {
    char menu[MAX_ROWS][MAX_COLS*2];
    char text[MAX_ROWS][MAX_COLS*2];
    int  text_col;
    int  text_row;
    int  text_top;
    int  menu_top;
    int  menu_from;
    int  menu_items;
    int  menu_sel;

    text_t *m_title;
    item_t *m_items;
    text_t *m_text;

    int m_selected;

    int (*set_title)(struct itemview *iv, text_t *title);
    int (*set_text)(struct itemview *iv, text_t *text);
    int (*set_items)(struct itemview *iv, item_t *items, int selected);
    int (*run)(struct itemview *iv, bool *exit);
    int (*redraw)(struct itemview *iv);
    int (*exit)(struct itemview *iv);
    void (*start_menu)(struct itemview *iv, int item_sel);
};

struct textview {
    bool  m_redraw;
    bool  m_exit;
    void *m_priv;
    char **m_pstr;
    khandler m_khandler;

    text_t *m_title;
    text_t *m_text;
    text_t *m_left_btn;
    text_t *m_right_btn;
    text_t *m_center_btn;
    ctext_t *m_ctexts;
    int  m_nr_ctexts;
    int  m_nr_lines;
    int  m_start;
    int  m_end;

    int (*set_title)(struct textview *tv, text_t *title);
    int (*set_text)(struct textview *tv, text_t *text);
    int (*set_ctext)(struct textview *tv, ctext_t *ctexts, int nr_ctext);
    int (*set_btn)(struct textview *tv, text_t *left, text_t *center, text_t *right);
    int (*run)(struct textview *tv);
    int (*redraw)(struct textview *tv);
    int (*exit)(struct textview *tv);
};

struct imageview {
    bool  m_redraw;
    bool  m_exit;
    void *m_priv;
    khandler m_khandler;

    image_t m_image;

    text_t *m_title;
    text_t *m_left_btn;
    text_t *m_right_btn;
    text_t *m_center_btn;

    int (*set_title)(struct imageview *imv, text_t *title);
    int (*set_image)(struct imageview *imv, char *filename, int x, int y);
    int (*set_btn)(struct imageview *imv, text_t *left, text_t *center, text_t *right);
    int (*redraw)(struct imageview *imv);
    int (*run)(struct imageview *imv);
    int (*exit)(struct imageview *imv);
};

struct paintview {
    bool        m_redraw;
    bool        m_exit;
    void        *m_priv;
    text_t      *m_title;
   
    khandler    m_khandler;
    
    text_t      *m_left_btn;
    text_t      *m_right_btn;
    text_t      *m_center_btn;

    point_t     *m_points;
    line_t      *m_lines;
    circle_t    *m_circles;

    int         m_nr_points;    
    int         m_nr_lines;
    int         m_nr_circles;
    image_t *m_img; 

    int (*set_title)(struct paintview *pv, text_t *title);
    int (*set_btn)(struct paintview *pv, text_t *left, text_t *center, text_t *right);
    int (*set_point)(struct paintview *pv, point_t *my_point, int nr_points);
    int (*set_line)(struct paintview *pv, line_t *my_line, int nr_lines);
    int (*set_circle)(struct paintview *pv, circle_t *my_circle, int nr_circles);    
    int (*set_img) (struct paintview *pv, image_t *my_img);
    int (*run)(struct paintview *pv);
    int (*redraw)(struct paintview *pv);
    int (*drawpoint)(struct paintview *pv);
    int (*drawline)(struct paintview *pv);
    int (*drawcircle)(struct paintview *pv);        
    int (*drawimg)(struct paintview *pv);
    int (*flip)(void);
    int (*exit)(struct paintview *pv);
};

// Initialize the graphics system.
extern void ui_init();

extern int ui_key_pressed(int key);
extern int ui_wait_key(void);
extern int ui_wait_phisical_key(void);

extern int ui_init_textview(struct textview *tv, khandler handler, void *priv);
extern int ui_init_itemview(struct itemview *iv);
extern int ui_init_imageview(struct imageview *iv, khandler handler, void *priv);
extern int ui_init_paintview(struct paintview *pv, khandler handler, void *priv);
extern struct itemview *ui_new_itemview(void);
extern void ui_free_itemview(struct itemview **iv);

extern void ui_line(int x1, int y1, int x2, int y2, int width);
extern void ui_color(int r, int g, int b, int a);
extern void ui_text(int x, int y, char *str);
extern void ui_fill(int x, int y, int w, int h);
extern void ui_circle(int x, int y, int r);
extern void ui_flip();
extern int ui_fb_width();
extern int ui_fb_height();
extern int get_avail_textline();

extern void ui_print(const char *fmt, ...);
extern void ui_printf(const char *fmt, ...);


#ifdef __cplusplus
}
#endif


#endif
