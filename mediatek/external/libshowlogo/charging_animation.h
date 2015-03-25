/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _CHARGING_ANIMATION_H
#define _CHARGING_ANIMATION_H

#include "show_logo_common.h"
#include "show_animation_common.h"


#ifdef __cplusplus
extern "C" {
#endif

// logo.bin image max size
#define LOGO_BUFFER_SIZE    0x300000

#define FB_NODE_PATH   "/dev/graphics/fb0"



/********** draw_anim_mode:  charging animation drawing method  ************/
/*                                                                       */ 
/* mode 0: use framebuffer, in KPOC mode                                 */
/* mode 1: use surface flinger , in IPO mode                             */
/*                                                                       */ 
/***                                                                   ***/
#define DRAW_ANIM_MODE_FB       1
#define DRAW_ANIM_MODE_SURFACE  0



/* public function*/

// common api
void anim_init();
void anim_deinit();

// set running mode function
void set_anim_version(int version);
void set_draw_mode(int draw_mode);

// show logos function
void show_boot_logo(void);
void show_kernel_logo(void);
void show_low_battery(void);
void show_charger_ov_logo(void);
void show_black_logo(void);
 
 // show aniamtion function
void show_battery_capacity(unsigned int capacity);



/* internal function */

// charging logo init and deinit
void anim_logo_init(void);
void anim_logo_deinit(void);

// framebuffer function
int  anim_fb_init(void);
void anim_fb_deinit(void);
void anim_fb_disp_update(void);
void anim_fb_addr_switch(void);

// surface flinger function
void anim_surface_init(void);
void anim_surface_deinit(void);

// show logo function
void anim_show_logo(int index);


#ifdef __cplusplus
}
#endif

#endif
