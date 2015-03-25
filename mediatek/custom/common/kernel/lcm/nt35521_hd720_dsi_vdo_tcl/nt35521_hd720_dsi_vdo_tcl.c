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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_NT35521 (0x80)
#define LCM_ID_NT35521_VERSION (0x5521)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};
static unsigned char tcl_otp_flag = 0;		//add by lishengli 20140214

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))			//add by lishengli 20140128

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   

#define   LCM_DSI_CMD_MODE							0
#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xdd   // END OF REGISTERS MARKER
#define REGFLAG_OTP						0xBE			//add by lishengli 20140214
#if 0								//add by lishengli 20140128
static void NT35521_set_reset_pin(int high){
	mt_set_gpio_mode(GPIO_DISP_LRSTB_PIN, GPIO_MODE_GPIO);
	if(1 == high)
		mt_set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
	else
		mt_set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
}

#define SET_RESET_PIN(v)    (NT35521_set_reset_pin((v)))
#endif

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {

{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
{0xFF,4,{0xAA,0x55,0xA5,0x80}},
{0x6F,2,{0x11,0x00}},
{0xF7,2,{0x20,0x00}},
{0x6F,1,{0x01}},
{0xB1,1,{0x21}},
{0xBD,5,{0x01,0xA0,0x10,0x08,0x01}},
{0xB8,4,{0x01,0x02,0x0C,0x02}},
{0xBB,2,{0x11,0x11}},
{0xBC,2,{0x00,0x00}},
{0xB6,1,{0x02}},
        
{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
{0xB0,2,{0x09,0x09}},
{0xB1,2,{0x09,0x09}},
{0xBC,2,{0x8C,0x00}},
{0xBD,2,{0x8C,0x00}},
{0xCA,1,{0x00}},
{0xC0,1,{0x04}},
//{0xBE,1,{0x8B}},//89				//add by lishengli 20140214
{0xB3,2,{0x35,0x35}},
{0xB4,2,{0x25,0x25}},
        
{0xB9,2,{0x43,0x43}},//43
{0xBA,2,{0x23,0x23}},//24
        
{0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
{0xEE,1,{0x01}},
{0xB0,16,{0x00,0x00,0x00,0x12,0x00,0x37,0x00,0x5B,0x00,0x75,0x00,0x9A,0x00,0xB8,0x00,0xE6}},
{0xB1,16,{0x01,0x0B,0x01,0x43,0x01,0x72,0x01,0xBD,0x01,0xF7,0x01,0xFA,0x02,0x32,0x02,0x74}},
{0xB2,16,{0x02,0x9F,0x02,0xDB,0x03,0x05,0x03,0x3C,0x03,0x60,0x03,0x8E,0x03,0xAA,0x03,0xDC}},
{0xB3,4,{0x03,0xFC,0x03,0xFF}},
{0x6F,1,{0x02}},
{0xF7,1,{0x47}},
{0x6F,1,{0x0A}},
{0xF7,1,{0x02}},
{0x6F,1,{0x17}},
{0xF4,1,{0x60}},
        
{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xB0,2,{0x20,0x00}},
{0xB1,2,{0x20,0x00}},
{0xB2,5,{0x15,0x00,0x60,0x00,0x00}},
{0xB3,5,{0x15,0x00,0x60,0x00,0x00}},
{0xB4,5,{0x05,0x00,0x60,0x00,0x00}},
{0xB5,5,{0x05,0x00,0x60,0x00,0x00}},
{0xBA,5,{0x44,0x10,0x60,0x01,0x90}},
{0xBB,5,{0x44,0x10,0x60,0x01,0x90}},
{0xBC,5,{0x44,0x10,0x60,0x01,0x90}},
{0xBD,5,{0x44,0x10,0x60,0x01,0x90}},
{0xC0,4,{0x00,0x34,0x00,0x00}},
{0xC1,4,{0x00,0x00,0x34,0x00}},
{0xC2,4,{0x00,0x00,0x34,0x00}},
{0xC3,4,{0x00,0x00,0x34,0x00}},
{0xC4,1,{0x60}},
{0xC5,1,{0xC0}},
{0xC6,1,{0x00}},
{0xC7,1,{0x00}},
        
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xB0,2,{0x17,0x06}},
{0xB1,2,{0x17,0x06}},
{0xB2,2,{0x17,0x06}},
{0xB3,2,{0x17,0x06}},
{0xB4,2,{0x17,0x06}},
{0xB5,2,{0x17,0x06}},
{0xB6,2,{0x14,0x03}},
{0xB7,2,{0x00,0x00}},
{0xB8,1,{0x0C}},
{0xB9,2,{0x00,0x03}},
{0xBA,2,{0x00,0x01}},
{0xBB,2,{0x0A,0x03}},
{0xBC,2,{0x02,0x03}},
{0xBD,5,{0x03,0x03,0x01,0x03,0x03}},
{0xC0,1,{0x07}},
{0xC1,1,{0x06}},
{0xC2,1,{0xA6}},
{0xC3,1,{0x05}},
{0xC4,1,{0xA6}},
{0xC5,1,{0xA6}},
{0xC6,1,{0xA6}},
{0xC7,1,{0xA6}},
{0xC8,2,{0x05,0x20}},
{0xC9,2,{0x04,0x20}},
{0xCA,2,{0x01,0x25}},
{0xCB,2,{0x01,0x60}},
{0xCC,3,{0x00,0x00,0x01}},
{0xCD,3,{0x00,0x00,0x01}},
{0xCE,3,{0x00,0x00,0x02}},
{0xCF,3,{0x00,0x00,0x02}},
{0xD0,7,{0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD1,5,{0x00,0x35,0x01,0x07,0x10}},
{0xD2,5,{0x10,0x35,0x02,0x03,0x10}},
{0xD3,5,{0x20,0x00,0x43,0x07,0x10}},
{0xD4,5,{0x30,0x00,0x43,0x07,0x10}},
{0xD5,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD6,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD7,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
{0xE5,1,{0x06}},
{0xE6,1,{0x06}},
{0xE7,1,{0x06}},
{0xE8,1,{0x06}},
{0xE9,1,{0x06}},
{0xEA,1,{0x06}},
{0xEB,1,{0x00}},
{0xEC,1,{0x00}},
{0xED,1,{0x30}},
{0xF0,5,{0x55,0xAA,0x52,0x08,0x06}},
{0xB5,2,{0x10,0x13}},
{0xB6,2,{0x12,0x11}},
{0xB7,2,{0x00,0x01}},
{0xB8,2,{0x08,0x31}},
{0xB9,2,{0x31,0x31}},
{0xBA,2,{0x31,0x31}},
{0xBB,2,{0x31,0x08}},
{0xBC,2,{0x03,0x02}},
{0xBD,2,{0x17,0x18}},
{0xBE,2,{0x19,0x16}},
{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
{0xD9,5,{0x00,0x00,0x00,0x00,0x00}},
{0xE5,2,{0x00,0x00}},
{0xE7,1,{0x00}},
{0x6F,1,{0x01}},
{0xF9,1,{0x46}},
{0x6F,1,{0x11}},
{0xF3,1,{0x01}},
{0x35,1,{0x00}},

{0x11,	1,{0x00}},
{REGFLAG_DELAY,120,{}},

{0x29,	1,{0x00}},
{REGFLAG_DELAY,50,{}},

};

//add by lishengli 20140214
static struct LCM_setting_table lcm_initialization_withoutotp_setting[] = {

{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
{0xFF,4,{0xAA,0x55,0xA5,0x80}},
{0x6F,2,{0x11,0x00}},
{0xF7,2,{0x20,0x00}},
{0x6F,1,{0x01}},
{0xB1,1,{0x21}},
{0xBD,5,{0x01,0xA0,0x10,0x08,0x01}},
{0xB8,4,{0x01,0x02,0x0C,0x02}},
{0xBB,2,{0x11,0x11}},
{0xBC,2,{0x00,0x00}},
{0xB6,1,{0x02}},
        
{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
{0xB0,2,{0x09,0x09}},
{0xB1,2,{0x09,0x09}},
{0xBC,2,{0x8C,0x00}},
{0xBD,2,{0x8C,0x00}},
{0xCA,1,{0x00}},
{0xC0,1,{0x04}},
{0xBE,1,{0x8B}},//89			
{0xB3,2,{0x35,0x35}},
{0xB4,2,{0x25,0x25}},
        
{0xB9,2,{0x43,0x43}},//43
{0xBA,2,{0x23,0x23}},//24
        
{0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
{0xEE,1,{0x01}},
{0xB0,16,{0x00,0x00,0x00,0x12,0x00,0x37,0x00,0x5B,0x00,0x75,0x00,0x9A,0x00,0xB8,0x00,0xE6}},
{0xB1,16,{0x01,0x0B,0x01,0x43,0x01,0x72,0x01,0xBD,0x01,0xF7,0x01,0xFA,0x02,0x32,0x02,0x74}},
{0xB2,16,{0x02,0x9F,0x02,0xDB,0x03,0x05,0x03,0x3C,0x03,0x60,0x03,0x8E,0x03,0xAA,0x03,0xDC}},
{0xB3,4,{0x03,0xFC,0x03,0xFF}},
{0x6F,1,{0x02}},
{0xF7,1,{0x47}},
{0x6F,1,{0x0A}},
{0xF7,1,{0x02}},
{0x6F,1,{0x17}},
{0xF4,1,{0x60}},
        
{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xB0,2,{0x20,0x00}},
{0xB1,2,{0x20,0x00}},
{0xB2,5,{0x15,0x00,0x60,0x00,0x00}},
{0xB3,5,{0x15,0x00,0x60,0x00,0x00}},
{0xB4,5,{0x05,0x00,0x60,0x00,0x00}},
{0xB5,5,{0x05,0x00,0x60,0x00,0x00}},
{0xBA,5,{0x44,0x10,0x60,0x01,0x90}},
{0xBB,5,{0x44,0x10,0x60,0x01,0x90}},
{0xBC,5,{0x44,0x10,0x60,0x01,0x90}},
{0xBD,5,{0x44,0x10,0x60,0x01,0x90}},
{0xC0,4,{0x00,0x34,0x00,0x00}},
{0xC1,4,{0x00,0x00,0x34,0x00}},
{0xC2,4,{0x00,0x00,0x34,0x00}},
{0xC3,4,{0x00,0x00,0x34,0x00}},
{0xC4,1,{0x60}},
{0xC5,1,{0xC0}},
{0xC6,1,{0x00}},
{0xC7,1,{0x00}},
        
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xB0,2,{0x17,0x06}},
{0xB1,2,{0x17,0x06}},
{0xB2,2,{0x17,0x06}},
{0xB3,2,{0x17,0x06}},
{0xB4,2,{0x17,0x06}},
{0xB5,2,{0x17,0x06}},
{0xB6,2,{0x14,0x03}},
{0xB7,2,{0x00,0x00}},
{0xB8,1,{0x0C}},
{0xB9,2,{0x00,0x03}},
{0xBA,2,{0x00,0x01}},
{0xBB,2,{0x0A,0x03}},
{0xBC,2,{0x02,0x03}},
{0xBD,5,{0x03,0x03,0x01,0x03,0x03}},
{0xC0,1,{0x07}},
{0xC1,1,{0x06}},
{0xC2,1,{0xA6}},
{0xC3,1,{0x05}},
{0xC4,1,{0xA6}},
{0xC5,1,{0xA6}},
{0xC6,1,{0xA6}},
{0xC7,1,{0xA6}},
{0xC8,2,{0x05,0x20}},
{0xC9,2,{0x04,0x20}},
{0xCA,2,{0x01,0x25}},
{0xCB,2,{0x01,0x60}},
{0xCC,3,{0x00,0x00,0x01}},
{0xCD,3,{0x00,0x00,0x01}},
{0xCE,3,{0x00,0x00,0x02}},
{0xCF,3,{0x00,0x00,0x02}},
{0xD0,7,{0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD1,5,{0x00,0x35,0x01,0x07,0x10}},
{0xD2,5,{0x10,0x35,0x02,0x03,0x10}},
{0xD3,5,{0x20,0x00,0x43,0x07,0x10}},
{0xD4,5,{0x30,0x00,0x43,0x07,0x10}},
{0xD5,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD6,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD7,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
{0xE5,1,{0x06}},
{0xE6,1,{0x06}},
{0xE7,1,{0x06}},
{0xE8,1,{0x06}},
{0xE9,1,{0x06}},
{0xEA,1,{0x06}},
{0xEB,1,{0x00}},
{0xEC,1,{0x00}},
{0xED,1,{0x30}},
{0xF0,5,{0x55,0xAA,0x52,0x08,0x06}},
{0xB5,2,{0x10,0x13}},
{0xB6,2,{0x12,0x11}},
{0xB7,2,{0x00,0x01}},
{0xB8,2,{0x08,0x31}},
{0xB9,2,{0x31,0x31}},
{0xBA,2,{0x31,0x31}},
{0xBB,2,{0x31,0x08}},
{0xBC,2,{0x03,0x02}},
{0xBD,2,{0x17,0x18}},
{0xBE,2,{0x19,0x16}},
{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
{0xD9,5,{0x00,0x00,0x00,0x00,0x00}},
{0xE5,2,{0x00,0x00}},
{0xE7,1,{0x00}},
{0x6F,1,{0x01}},
{0xF9,1,{0x46}},
{0x6F,1,{0x11}},
{0xF3,1,{0x01}},
{0x35,1,{0x00}},
{0x11,	1,{0x00}},
{REGFLAG_DELAY,120,{}},

{0x29,	1,{0x00}},
{REGFLAG_DELAY,50,{}},

};

static struct LCM_setting_table lcm_sleep_in_setting[] = {

	// Display off sequence
	{0x28, 0, {0x00}},
   {REGFLAG_DELAY, 50,{}},
    // Sleep Mode On
	{0x10, 0, {0x00}},
   {REGFLAG_DELAY, 50, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 10, {}},

    // Display ON
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 10, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void init_lcm_registers(void)
{
	unsigned int data_array[16];
	
		
		data_array[0] = 0x00062902; 						 
		data_array[1] = 0x52AA55F0; 
		data_array[2] = 0x00000008;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);
		
		data_array[0] = 0x00052902; 						 
		data_array[1] = 0xA555AAFF; 
		data_array[2] = 0x00000080;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);
	
		
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000176F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);
		
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000070F4;  
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		 
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000066F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);
		 
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000020F7;  
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000000F7;  
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00062902; 						 
		data_array[1] = 0x050000FC; 
		data_array[2] = 0x00000B08;  
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);
		
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00001F6F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);	
		
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000090FA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000216F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);
	
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000018FA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000026F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000003F8; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00000A6F; 
		dsi_set_cmdq(data_array, 2, 1); 

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000032F8; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00000B6F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000020F8; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00000F6F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000013FA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000026F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000EFF7; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00062902; 						 
		data_array[1] = 0x20C201BD;
		data_array[2] = 0x00000110;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00022902;  //add 						 
		data_array[1] = 0x0000016F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000026F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000008B8; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00032902; 						 
		data_array[1] = 0x004474BB; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00062902; 						 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000108;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00032902; 						 
		data_array[1] = 0x003434B9; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00032902; 						 
		data_array[1] = 0x000078BC; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00032902; 						 
		data_array[1] = 0x000078BD; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000004B5; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000000CA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00000AB0; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00000AB1; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00000C6F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000A7F4; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00000E6F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000043F4; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000066CE; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x00000CC0; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00062902; 						 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000608;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902; 						 
		data_array[1] = 0x000000D8;
		data_array[2] = 0x00002000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902; 						 
		data_array[1] = 0x000008D9;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000000E7; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x0000116F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000001F3; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000008;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00022902; 						 
		data_array[1] = 0x000005BC; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000108;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00022902;	 
		data_array[1] = 0x000000CA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00062902; 						 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000308;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x7D0004B2;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x7D0004B3;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x0A0003B4;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x0A0003B5;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x7D0004B6;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x7D0004B7;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x190002B8;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x190002B9;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x0F0044BA;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x0F0044BB;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x4F0088BC;
		data_array[2] = 0x00005F01;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x4F0088BD;
		data_array[2] = 0x00005F01;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000508;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00052902;
		data_array[1] = 0x790001D1;
		data_array[2] = 0x00000005;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00052902;
		data_array[1] = 0x790001D2;
		data_array[2] = 0x00000003;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00052902;
		data_array[1] = 0x790001D3;
		data_array[2] = 0x00000001;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00052902;
		data_array[1] = 0x780001D4;
		data_array[2] = 0x00000009;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00032902;
		data_array[1] = 0x003003C8;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00032902;
		data_array[1] = 0x003103C9;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00032902;
		data_array[1] = 0x003007CA;
		dsi_set_cmdq(data_array, 2, 1);

		data_array[0] = 0x00032902;
		data_array[1] = 0x003101CB;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000608;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x000000D8;
		data_array[2] = 0x00002000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x000008D9;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00022902;
		data_array[1] = 0x000000E7;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00062902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00032902;  //add for test flip
		data_array[1] = 0x0000C036;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00110500; // Sleep Out
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(120);
		
		data_array[0] = 0x00290500; // Display On
		dsi_set_cmdq(data_array, 1, 1);

#if 0 // for test
		data_array[0] = 0x00280500; // Sleep Out
		dsi_set_cmdq(data_array, 1, 1);
		
		data_array[0] = 0x00100500; // Display On
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(120);

		data_array[0] = 0x00062902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000008;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00052902;
		data_array[1] = 0x027887EE;
		data_array[2] = 0x00000040;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00032902;
		data_array[1] = 0x00FF07EF;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
#endif


}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
    for(i = 0; i < count; i++) {	
        unsigned cmd;
        cmd = table[i].cmd;
        switch (cmd) {
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
            case REGFLAG_END_OF_TABLE :
                break;
   
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{

        memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

/*
 * Active Area (WXH) = 62.1×110.4 mm
*/
		params->physical_width  = 62.1;
		params->physical_height = 110.4;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   =BURST_VDO_MODE;// SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;	
#if 1
		
		params->dsi.vertical_sync_active				= 4;//2;	//2;
		params->dsi.vertical_backporch					= 40;//40;	//14;
		params->dsi.vertical_frontporch					= 40;//16;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 10;//4;	//2;
		params->dsi.horizontal_backporch				= 160;//42;	//60;	//42;
		params->dsi.horizontal_frontporch				= 160;//44;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#else

		
		params->dsi.vertical_sync_active				= 3;//2;	//2;
		params->dsi.vertical_backporch					= 20;//14;	//14;
		params->dsi.vertical_frontporch					= 20;//16;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 4;//4;	//2;
		params->dsi.horizontal_backporch				= 80;//42;	//60;	//42;
		params->dsi.horizontal_frontporch				= 80;//44;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#endif
		// Bit rate calculation
		//1 Every lane speed
		//params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		//params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		//params->dsi.fbk_div =19;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
		
		//params->dsi.PLL_CLOCK = 265;		
		params->dsi.PLL_CLOCK = 259; 
        
}

static void lcm_init(void)
{
	SET_RESET_PIN(1);
    MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(10);
	
	SET_RESET_PIN(1);
	MDELAY(120);      


	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
/*
	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	
	MDELAY(10);

	//SET_RESET_PIN(0);

	//MDELAY(120);
*/
     	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
    SET_RESET_PIN(1);
	SET_RESET_PIN(0);	
	MDELAY(10);	
	SET_RESET_PIN(1);
    MDELAY(120);	
	
}


static void lcm_resume(void)
{
	lcm_init();
}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif
#if 1
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
    unsigned int id0=0;
    unsigned int id1=0;
	unsigned char buffer[2];
	unsigned int array[16];  
	unsigned char otp_buffer[5];
	struct LCM_setting_table page1 = {0xF0,5,{0x55,0xAA,0x52,0x08,0x01}};

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20); 

    //begin add by lishengli 20140214
    dsi_set_cmdq_V2(page1.cmd, page1.count, page1.para_list, 1);
    MDELAY(1);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

    //read_reg_v2(0x04, buffer, 2);
	//id = buffer[1]; //we only need ID
	
	read_reg_v2(0xC5, buffer, 2);
	//id = buffer[1]; //we only need ID
    id0 = buffer[0];//0X55
    id1 = buffer[1];//0X21

	
    #ifdef BUILD_LK
		printf("%s, LK nt35521 debug: nt35521 id = 0x%08x id0=0x%08x id1==0x%08x\n", __func__, id,id0,id1);
    #else
		printk("%s, kernel nt35521 horse debug: nt35521 id = 0x%08x id0=0x%08x id1==0x%08x\n", __func__, id,id0,id1);
    #endif

    if((id0 == 0x55)&&(id1 == 0x21))
    {
     //begin add by lishengli 20140214
     dsi_set_cmdq_V2(page1.cmd, page1.count, page1.para_list, 1);
     MDELAY(1);

	 array[0] = 0x00043700;// read id return two byte,version and id
	 dsi_set_cmdq(array, 1, 1);
	
	 read_reg_v2(0xEF, otp_buffer, 4);

	 #ifdef BUILD_LK
		printf("%s, LK nt35521 OTP: otp_buffer[0] = 0x%x, otp_buffer[1] = 0x%x, otp_buffer[2] = 0x%x, otp_buffer[3] = 0x%x\n", __func__, 
		         otp_buffer[0], otp_buffer[1], otp_buffer[2], otp_buffer[3]);
        #else
		printk("%s, kernel nt35521 OTP:otp_buffer[0] = 0x%x, otp_buffer[1] = 0x%x, otp_buffer[2] = 0x%x, otp_buffer[3] = 0x%x\n", __func__, 
		         otp_buffer[0], otp_buffer[1], otp_buffer[2], otp_buffer[3]);
        #endif

	 tcl_otp_flag = ((otp_buffer[1] & 0xF0) == 0) ? 0 : 1;
        //end add by lishengli 20140214		
		
        return 1;
    }
    else
        return 0;


}
#endif

LCM_DRIVER nt35521_hd720_dsi_vdo_tcl_lcm_drv = 
{
    .name			= "nt35521_hd720_dsi_vdo_tcl",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
