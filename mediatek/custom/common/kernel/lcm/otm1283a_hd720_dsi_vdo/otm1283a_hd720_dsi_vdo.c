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
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#if defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif
#endif

#define LCM_ID_OTM1283 (0x1283)

#define LCM_DBG(fmt, arg...) \
	LCM_PRINT ("[LCM-OTM1383A-DSI-VDO] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xdd   // END OF REGISTERS MARKER


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
#if 1

{0x00,1,{0x00}},
{0xFF,3,{0x12,0x83,0x01}},	
{0x00,1,{0x80}},	
{0xFF,2,{0x12,0x83}},

{0x00,1,{0x80}},
{0xc0,9,{0x00,0x64,0x00,0x0f,0x11,0x00,0x64,0x0f,0x11}},

{0x00,1,{0x90}},
{0xc0,6,{0x00,0x55,0x00,0x01,0x00,0x04}},


{0x00,1,{0xb3}},
{0xC0,2,{0x00,0x55}},

{0x00,1,{0x81}},
{0xc1,1,{0x55}},

{0x00,1,{0x81}},
{0xc4,1,{0x82}},

{0x00,1,{0x82}},
{0xc4,1,{0x02}}, //02 //source OP down current.

{0x00,1,{0x90}},
{0xc4,1,{0x49}},

{0x00,1,{0xc6}},		
{0xB0,1,{0x03}},


{0x00,1,{0x90}},
{0xf5,4,{0x02,0x11,0x02,0x11}},

{0x00,1,{0x90}},
{0xc5,1,{0x50}},

{0x00,1,{0x94}},
{0xc5,1,{0x77}},//66-77

{0x00,1,{0xb2}},
{0xf5,2,{0x00,0x00}},

{0x00,1,{0xb4}},
{0xf5,2,{0x00,0x00}},

{0x00,1,{0xb6}},
{0xf5,2,{0x00,0x00}},

{0x00,1,{0xb8}},
{0xf5,2,{0x00,0x00}},

{0x00,1,{0x94}},			
{0xF5,1,{0x02}},

{0x00,1,{0xBA}},			
{0xF5,1,{0x03}},

{0x00,1,{0xb2}},
{0xc5,1,{0x40}},

{0x00,1,{0xb4}},
{0xc5,1,{0xc0}},		

{0x00,1,{0xa0}},
{0xc4,14,{0x05,0x10,0x06,0x02,0x05,0x15,0x10,0x05,0x10,0x07,0x02,0x05,0x15,0x10}},

{0x00,1,{0xb0}},
{0xc4,2,{0x00,0x00}},

{0x00,1,{0x91}},
{0xc5,2,{0x19,0x50}},

{0x00,1,{0x00}},
{0xd8,2,{0xbc,0xbc}},

{0x00,1,{0xb0}},
{0xc5,2,{0x04,0x08}},//b8-08

{0x00,1,{0xbb}},
{0xc5,1,{0x80}},

{0x00,1,{0x00}},
{0xd0,1,{0x40}},

{0x00,1,{0x00}},
{0xd1,2,{0x00,0x00}},

//{0x00,1,{0x00}},
//{0xE1,16,{0x08,0x0f,0x15,0x0E,0x07,0x10,0x0C,0x0C,0x02,0x06,0x0c,0x07,0x0f,0x14,0x11,0x00}},

//{0x00,1,{0x00}},
//{0xE2,16,{0x08,0x10,0x15,0x0E,0x07,0x10,0x0C,0x0C,0x02,0x06,0x0c,0x07,0x0f,0x14,0x11,0x00}},	

//{0x00,1,{0x00}},
//{0xD9,1,{0x68}},   // zx++ BOE MTP vcom

{0x00,1,{0x80}},
{0xcb,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x90}},
{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xa0}},
{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xb0}},
{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xc0}},
{0xcb,15,{0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xd0}},
{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00}},

{0x00,1,{0xe0}},
{0xcb,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05}},

{0x00,1,{0xf0}},
{0xcb,11,{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},

{0x00,1,{0x80}},
{0xcc,15,{0x0a,0x0c,0x0e,0x10,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x90}},
{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x2e,0x2d,0x09,0x0b,0x0d,0x0f,0x01,0x03,0x00,0x00}},

{0x00,1,{0xa0}},
{0xcc,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2e,0x2d}},

{0x00,1,{0xb0}},
{0xcc,15,{0x0F,0x0D,0x0B,0x09,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xc0}},
{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x2d,0x2e,0x10,0x0E,0x0C,0x0A,0x04,0x02,0x00,0x00}},

{0x00,1,{0xd0}},
{0xcc,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2d,0x2e}},

{0x00,1,{0x80}},
{0xce,12,{0x8d,0x03,0x00,0x8c,0x03,0x00,0x8b,0x03,0x00,0x8a,0x03,0x00}},

{0x00,1,{0xa0}},
{0xce,14,{0x38,0x0b,0x04,0xfc,0x00,0x00,0x00,0x38,0x0a,0x04,0xfd,0x00,0x00,0x00}},

{0x00,1,{0xb0}},
{0xce,14,{0x38,0x09,0x04,0xfe,0x00,0x00,0x00,0x38,0x08,0x04,0xff,0x00,0x00,0x00}},

{0x00,1,{0xc0}},
{0xce,14,{0x38,0x07,0x05,0x00,0x00,0x00,0x00,0x38,0x06,0x05,0x01,0x00,0x00,0x00}},

{0x00,1,{0xd0}},
{0xce,14,{0x38,0x05,0x05,0x02,0x00,0x00,0x00,0x38,0x04,0x05,0x03,0x00,0x00,0x00}},

{0x00,1,{0x80}},
{0xcf,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x90}},
{0xcf,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xa0}},
{0xcf,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xb0}},
{0xcf,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xc0}},
{0xcf,11,{0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x02,0x00,0x00,0x08}},

{0x00,1,{0xb5}},
{0xc5,6,{0x33,0xf1,0xff,0x33,0xf1,0xff}},



//add20131030

{0x00,1,{0xB4}},
{0xC5,1,{0xC0}},

{0x00,1,{0x87}},
{0xC4,1,{0x18}},

{0x00,1,{0xA4}},
{0xC0,1,{0x00}},

//{0x00,1,{0xA0}},
//{0xC1,1,{0x02}},

{0x00,1,{0xA0}},
{0xC1,1,{0x00}},
//{REGFLAG_DELAY,100,{}},

{0x00,1,{0xA2}},
{0xC1,1,{0x08}},//08

{0x00,1,{0xA4}},
{0xC1,1,{0xF0}},//f0

///////////////

{0x00,1,{0xB2}},
{0xC4,1,{0x81}},

{0x00,1,{0x93}},
{0xC1,1,{0x90}},


{0x00,1,{0xB5}},
{0xC4,1,{0x80}},

{0x00,1,{0x80}},
{0xC4,1,{0x30}},

{0x00,1,{0x8A}},
{0xC4,1,{0x40}},

{0x00,1,{0x8B}},
{0xC4,1,{0x00}},


{0x00, 1 ,{0xC2}}, 
{0xF5, 1 ,{0xC0}}, 

//{0x00, 1 , {0x80}}, 
//{0xC4, 1 ,{0x01}}, 

{0x00, 1 ,{0x88}}, 
{0xC4, 1 ,{0x80}},

{0x00,1,{0x91}},
//{0xf4,3,{0x70,0x00,0x00}},
//{0xf4,3,{0x12,0x20,0x22 }},
{0xf4,3,{0x01,0x10,0x11}},

//{0x00, 1 , {0xAA}}, 
//{0xF5, 2 ,{0x00,0x00}}, 

{0x00, 1 , {0x00}}, 
//{0xE1, 16 ,{0x0A,0x1D,0x23,0x0D,0x04,0x0F,0x0A,0x09,0x03,0x06,0x0A,0x05,0x0E,0x11,0x0D,0x01}}, 
//{0xE1, 16 ,{0x08,0x0D,0x11,0x0C,0x05,0x0B,0x0A,0x09,0x05,0x08,0x12,0x09,0x0F,0x11,0x0B,0x01}},
{0xE1, 16 ,{0x08,0x0C,0x10,0x0B,0x04,0x0A,0x0A,0x0A,0x03,0x07,0x12,0x07,0x0F,0x10,0x0A,0x00}},

{0x00, 1 , {0x00}}, 
//{0xE2, 16 ,{0x0A,0x1D,0x22,0x0E,0x04,0x0E,0x0B,0x0A,0x02,0x06,0x09,0x05,0x0E,0x11,0x0D,0x01}}, 
//{0xE2, 16 ,{0x08,0x0D,0x11,0x0C,0x05,0x0B,0x0A,0x08,0x05,0x08,0x11,0x08,0x0F,0x11,0x0B,0x01}},
{0xE2, 16 ,{0x08,0x0c,0x10,0x0b,0x04,0x0a,0x0a,0x0a,0x03,0x07,0x12,0x07,0x0f,0x10,0x0A,0x00}},
                   
{0x00,1,{0x00}},
{0x35,1,{0x00}},

{0x00,1,{0x00}},
{0x13,1,{0x00}},
{REGFLAG_DELAY,50,{}},

{0x00,1,{0x00}},
{0xFF,3,{0xFF,0xFF,0xFF}},


{0x11,	1,{0x00}},
{REGFLAG_DELAY,120,{}},

{0x29,	1,{0x00}},
{REGFLAG_DELAY,50,{}},


#else

/*
{0xFF,  3 ,{0x12,0x83,0x01}}, 

{0x00, 1 , {0x80}}, 
{0xFF,  2 ,{0x12,0x83}}, 

{0x00, 1 , {0xA0}}, 
{0xC1,  1 ,{0x02}}, 

{0x00, 1 , {0x80}}, 
{0xC0,  9 ,{0x00,0x64,0x00,0x0F,0x11,0x00,0x64,0x0F,0x11}}, 

{0x00, 1 , {0x90}}, 
{0xC0,  6 ,{0x00,0x5C,0x00,0x01,0x00,0x04}}, 

{0x00, 1 , {0xA4}}, 
{0xC0,  1 ,{0x1C}}, 

{0x00, 1 , {0xB3}}, 
{0xC0, 2 ,{0x00,0x50}}, 

{0x00, 1 , {0x81}}, 
{0xC1, 1 ,{0x66}}, //55 

{0x00, 1 , {0x81}}, 
{0xC4,  1 ,{0x82}}, 

{0x00, 1 , {0x90}}, 
{0xC4,  1 ,{0x49}}, 

{0x00, 1 , {0xA0}}, 
{0xC4, 14 ,{0x05,0x10,0x06,0x02,0x05,0x15,0x10,0x05,0x10,0x07,0x02,0x05,0x15,0x10}}, 

{0x00, 1 , {0xB0}}, 
{0xC4,  2 ,{0x00,0x00}}, 

{0x00, 1 , {0x91}}, 
{0xC5,  2 ,{0x19,0x50}}, 

{0x00, 1 , {0x00}}, 
{0xD8,  2 ,{0xBC,0xBC}}, 



//{0x00, 1 , {0x00}}, 
//{0x65,  1 ,{0xD9}},  \u9519\u8bef\u683c\u5f0f 


{0x00, 1 , {0x00}}, 
{0xD9,  1 ,{0x65}}, 


{0x00, 1 , {0xB0}}, 
{0xC5,  2 ,{0x04,0x08}}, //b8 

{0x00, 1 , {0xBB}}, 
{0xC5,  1 ,{0x80}}, 

{0x00, 1 , {0x00}}, 
{0xD0,  1 ,{0x40}}, 

{0x00, 1 , {0x00}}, 
{0xD1,  2 ,{0x00,0x00}}, 

{0x00, 1 , {0x80}}, 
{0xCB, 11 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0x90}}, 
{0xCB, 15 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xA0}}, 
{0xCB, 15 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xB0}}, 
{0xCB, 15 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xC0}}, 
{0xCB, 15 ,{0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xD0}}, 
{0xCB, 15 ,{0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00}}, 

{0x00, 1 , {0xE0}}, 
{0xCB, 14 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05}}, 

{0x00, 1 , {0xF0}}, 
{0xCB, 11 ,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}}, 

{0x00, 1 , {0x80}}, 
{0xCC, 15 ,{0x0A,0x0C,0x0E,0x10,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0x90}}, 
{0xCC, 15 ,{0x00,0x00,0x00,0x00,0x00,0x2E,0x2D,0x09,0x0B,0x0D,0x0F,0x01,0x03,0x00,0x00}}, 

{0x00, 1 , {0xA0}}, 
{0xCC, 14 ,{0x00,0x00,0x00,0x10,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x2E,0x2D}}, 

{0x00, 1 , {0xB0}}, 
{0xCC, 15 ,{0x0F,0x0D,0x0B,0x09,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xC0}}, 
{0xCC, 15 ,{0x00,0x00,0x00,0x00,0x00,0x2D,0x2E,0x10,0x0E,0x0C,0x0A,0x04,0x02,0x00,0x00}}, 

{0x00, 1 , {0xD0}}, 
{0xCC, 14 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2D,0x2E}}, 

{0x00, 1 , {0x80}}, 
{0xCE, 12 ,{0x8F,0x03,0x18,0x8E,0x03,0x18,0x8D,0x03,0x18,0x8C,0x03,0x18}}, 

{0x00, 1 , {0x90}}, 
{0xCE, 14 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xA0}}, 
{0xCE, 14 ,{0x38,0x0B,0x05,0x00,0x00,0x00,0x00,0x38,0x0A,0x05,0x01,0x00,0x00,0x00}}, 

{0x00, 1 , {0xB0}}, 
{0xCE, 14 ,{0x38,0x09,0x05,0x02,0x00,0x00,0x00,0x38,0x08,0x05,0x03,0x00,0x00,0x00}}, 

{0x00, 1 , {0xC0}}, 
{0xCE, 14 ,{0x38,0x07,0x05,0x04,0x00,0x00,0x00,0x38,0x06,0x05,0x05,0x00,0x00,0x00}}, 

{0x00, 1 , {0xD0}}, 
{0xCE, 14 ,{0x38,0x05,0x05,0x06,0x00,0x00,0x00,0x38,0x04,0x05,0x07,0x00,0x00,0x00}}, 

{0x00, 1 , {0x80}}, 
{0xCF, 14 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0x90}}, 
{0xCF, 14 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xA0}}, 
{0xCF, 14 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xB0}}, 
{0xCF, 14 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 

{0x00, 1 , {0xC0}}, 
{0xCF, 11 ,{0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x81,0x00,0x03,0x08}}, 

{0x00, 1 , {0xB5}}, 
{0xC5,  6 ,{0x33,0xF1,0xFF,0x33,0xF1,0xFF}}, 

{0x00, 1 , {0x90}}, 
{0xF5,  4 ,{0x02,0x11,0x02,0x11}}, 

{0x00, 1 , {0x90}}, 
{0xC5,  1 ,{0x50}}, 

{0x00, 1 , {0x94}}, 
{0xC5,  1 ,{0x77}},//66 

{0x00, 1 , {0xB2}}, 
{0xF5,  2 ,{0x00,0x00}}, 

{0x00, 1 , {0xB4}}, 
{0xF5,  2 ,{0x00,0x00}}, 

{0x00, 1 , {0xB6}}, 
{0xF5,  2 ,{0x00,0x00}}, 

{0x00, 1 , {0xB8}}, 
{0xF5,  2 ,{0x00,0x00}}, 


// add  20131030 
{0x00, 1 , {0xB4}}, 
{0xC5,  1 ,{0xC0}}, 

{0x00, 1 , {0x87}}, 
{0xC4,  1 ,{0x18}}, 

{0x00, 1 , {0xA4}}, 
{0xC0,  1 ,{0x00}}, 

{0x00, 1 , {0xA4}}, 
{0xC1,  1 ,{0xF0}}, 

{0x00, 1 , {0xB2}}, 
{0xC4,  1 ,{0x81}}, 

{0x00, 1 , {0x93}}, 
{0xC1,  1 ,{0x90}}, 

{0x00, 1 , {0xB2}}, 
{0xC4,  1 ,{0x81}}, 

{0x00, 1 , {0xB5}}, 
{0xC4,  1 ,{0x80}}, 

{0x00, 1 , {0x80}}, 
{0xC4,  1 ,{0x30}}, 

{0x00, 1 , {0x8B}}, 
{0xC4,  1 ,{0x40}}, 
// 

{0x00, 1 , {0x00}}, 
{0xE1, 16 ,{0x0A,0x1D,0x23,0x0D,0x04,0x0F,0x0A,0x09,0x03,0x06,0x0A,0x05,0x0E,0x11,0x0D,0x01}}, 

{0x00, 1 , {0x00}}, 
{0xE2, 16 ,{0x0A,0x1D,0x22,0x0E,0x04,0x0E,0x0B,0x0A,0x02,0x06,0x09,0x05,0x0E,0x11,0x0D,0x01}}, 

//{0x00, 1 , {0x00}}, 
//{0xFF,  3 ,{0x00,0x00,0x00}}, \u9519\u8bef\u683c\u5f0f

{0x00, 1 , {0x00}}, 
{0xFF,  3 ,{0xff,0xff,0xff}},

*/
#endif
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
// Display off sequence
	{0x22, 0, {0x00}},
   {REGFLAG_DELAY, 50,{}},

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
    {REGFLAG_DELAY, 5, {}},

    // Display ON
    {0x29, 0, {0x00}},
    //	{REGFLAG_DELAY, 20, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	//All pixel off
	{0x22, 1, {0x00}},
	{REGFLAG_DELAY,50,{}},
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY,20,{}},
    // Sleep Mode On
	{0x10, 1, {0x00}},
    {REGFLAG_DELAY,120,{}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xB9,	3,	{0xFF, 0x83, 0x69}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
//	{0xC3, 1, {0xFF}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#if 0
static void lcm_init_registers(void)
{
    unsigned int data_array[16];
    data_array[0] = 0x00042902;
    data_array[1] = 0x018312FF;
    dsi_set_cmdq(&data_array, 2, 1);//EXTC = 1

    data_array[0] = 0x80002300;
    dsi_set_cmdq(&data_array, 1, 1);	//Orise mode enable

    data_array[0] = 0x00032902;
    data_array[1] = 0x008312FF;
    dsi_set_cmdq(&data_array, 2, 1);

    #if 0
    data_array[0] = 0xB2002300;
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x81C42300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x93002300;
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x90C12300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0xB5002300;
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x80C42300;
    dsi_set_cmdq(&data_array, 1, 1);
    #else
    /*Gen_write_1A_2P(0xFF,0x12,0x83); 
Gen_write _1A_1P(0x00,0xA0); 
Gen_write _1A_1P(0xC1,0x02);*/  
    data_array[0] = 0xA0002300;
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x02C12300;
    dsi_set_cmdq(&data_array, 1, 1); // Disable time-out function
    #endif
	
    
    /*===================panel setting====================*/
    data_array[0] = 0x80002300;
    dsi_set_cmdq(&data_array, 1, 1);	//TCON Setting

    data_array[0] = 0x000A2902;
    data_array[1] = 0x006400C0;
    data_array[2] = 0x6400110F;
    data_array[3] = 0x0000110F;
    dsi_set_cmdq(&data_array, 4, 1);

    data_array[0] = 0x90002300;
    dsi_set_cmdq(&data_array, 1, 1);	//Panel Timing Setting

    data_array[0] = 0x00072902;
    data_array[1] = 0x005C00C0;
    data_array[2] = 0x00040001;
    dsi_set_cmdq(&data_array, 3, 1);

    data_array[0] = 0xA4002300;
    dsi_set_cmdq(&data_array, 1, 1);	//Source pre.

    data_array[0] = 0x1CC02300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0xB3002300;
    dsi_set_cmdq(&data_array, 1, 1);	//Interval Scan Frame: 0 frame, column inversion

    data_array[0] = 0x00032902;
    data_array[1] = 0x005000C0;
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0x81002300;
    dsi_set_cmdq(&data_array, 1, 1);	//frame rate: 60Hz

    data_array[0] = 0x66C12300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x81002300;				//Source bias 0.75uA
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x82C42300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x90002300;				//clock delay for data latch
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x49C42300;
    dsi_set_cmdq(&data_array, 1, 1);

    /*================Power setting===============*/
    data_array[0] = 0xA0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000F2902;	//DCDC setting
    data_array[1] = 0x061005C4;
    data_array[2] = 0x10150502;
    data_array[3] = 0x02071005;
    data_array[4] = 0x00101505;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xB0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//Clamp coltage setting
    data_array[1] = 0x000000C4;
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0x91002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//VGH=12v, VGL=-12v, pump ratio: VGH=6x, VGL=-5x
    data_array[1] = 0x005019c5;
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0x00002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//GVDD=4.87v, NGVDD = -4.87V
    data_array[1] = 0x00bcbcd8;
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0x00002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0xd9652300;		//VCOMDC=-1.1
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0xB0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//VDD_18v=1.6v, LVDSVDD=1.55v
    data_array[1] = 0x000804c5;// c5b1-b8-28
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0xBB002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x80c52300;	//LVD voltage level setting
    dsi_set_cmdq(&data_array, 1, 1);

    /*=========================Control setting======================*/
    data_array[0] = 0x00002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x40d02300;		//ID1
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//ID2, ID3
    data_array[1] = 0x000000d1;
    dsi_set_cmdq(&data_array, 2, 1);

    /*=========================Panel Timming State Control================*/
    data_array[0] = 0x80002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000C2902;	//Panel timing state control
    data_array[1] = 0x000000cb;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    dsi_set_cmdq(&data_array, 4, 1);

    data_array[0] = 0x90002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel timing state control
    data_array[1] = 0x000000cb;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xa0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel timing state control
    data_array[1] = 0x000000cb;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xb0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel timing state control
    data_array[1] = 0x000000cb;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xc0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel timing state control
    data_array[1] = 0x050505cb;
    data_array[2] = 0x00050505;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xd0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel timing state control
    data_array[1] = 0x000000cb;
    data_array[2] = 0x05050000;
    data_array[3] = 0x05050505;
    data_array[4] = 0x00000505;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xe0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000F2902;	//Panel timing state control
    data_array[1] = 0x000000cb;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00050500;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xf0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000c2902;	//Panel timing state control
    data_array[1] = 0xffffffcb;
    data_array[2] = 0xffffffff;
    data_array[3] = 0xffffffff;
    dsi_set_cmdq(&data_array, 4, 1);

    /*===============Panel pad mapping control===============*/
    data_array[0] = 0x80002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel pad mapping control
    data_array[1] = 0x0e0c0acc;
    data_array[2] = 0x00040210;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0x90002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel pad mapping control
    data_array[1] = 0x000000cc;
    data_array[2] = 0x2d2e0000;
    data_array[3] = 0x0f0d0b09;
    data_array[4] = 0x00000301;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xa0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel pad mapping control
    data_array[1] = 0x000000cc;
    data_array[2] = 0x00040210;
    data_array[3] = 0x00000000;
    data_array[4] = 0x002d2e00;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xb0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel pad mapping control
    data_array[1] = 0x0b0d0fcc;
    data_array[2] = 0x00010309;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xc0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00102902;	//Panel pad mapping control
    data_array[1] = 0x000000cc;
    data_array[2] = 0x2e2d0000;
    data_array[3] = 0x0a0c0e10;
    data_array[4] = 0x00000204;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xd0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel pad mapping control
    data_array[1] = 0x000000cc;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x002e2d00;
    dsi_set_cmdq(&data_array, 5, 1);

    /*===============Panel Timing Setting====================*/
    data_array[0] = 0x80002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000d2902;	//Panel VST Setting
    data_array[1] = 0x18038fce;
    data_array[2] = 0x8d18038e;
    data_array[3] = 0x038c1803;
    data_array[4] = 0x00000018;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0x90002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel vend setting
    data_array[1] = 0x000000ce;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xa0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel clka1/2 setting
    data_array[1] = 0x050b38ce;
    data_array[2] = 0x00000000;
    data_array[3] = 0x01050a38;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xb0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel clka3/4 setting
    data_array[1] = 0x050938ce;
    data_array[2] = 0x00000002;
    data_array[3] = 0x03050838;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xc0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel clkb1/2 setting
    data_array[1] = 0x050738ce;
    data_array[2] = 0x00000004;
    data_array[3] = 0x05050638;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xd0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel clkb3/4 setting
    data_array[1] = 0x050538ce;
    data_array[2] = 0x00000006;
    data_array[3] = 0x07050438;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0x80002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel clkc1/2 setting
    data_array[1] = 0x000000cf;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0x90002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel clkc3/4 setting
    data_array[1] = 0x000000cf;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xa0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel clkd1/2 setting
    data_array[1] = 0x000000cf;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xb0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000f2902;	//Panel clkd3/4 setting
    data_array[1] = 0x000000cf;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;
    dsi_set_cmdq(&data_array, 5, 1);

    data_array[0] = 0xc0002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x000c2902;	//gate pre. ena
    data_array[1] = 0x200101cf;
    data_array[2] = 0x01000020;
    data_array[3] = 0x08030081;
    dsi_set_cmdq(&data_array, 4, 1);

    data_array[0] = 0xb5002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00072902;	//normal output with VGH/VGL
    data_array[1] = 0xfff133c5;
    data_array[2] = 0x00fff133;
    dsi_set_cmdq(&data_array, 3, 1);

    data_array[0] = 0x90002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00052902;	//mode-3
    data_array[1] = 0x021102f5;
    data_array[2] = 0x00000011;
    dsi_set_cmdq(&data_array, 3, 1);

    data_array[0] = 0x90002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x50c52300;	//2xVPNL, 1.5*=00, 2*=50, 3*=A0
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x94002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x77c52300;	//Frequency 66-77
    dsi_set_cmdq(&data_array, 1, 1);

    /*===============VGL01/02 disable================*/
    data_array[0] = 0xb2002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//VGL01
    data_array[1] = 0x000000f5;
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0xb4002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//VGL01_S
    data_array[1] = 0x000000f5;
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0xb6002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//VGL02
    data_array[1] = 0x000000f5;
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0xb8002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00032902;	//VGL02_S
    data_array[1] = 0x000000f5;
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0xb4002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0xC0C52300;
    dsi_set_cmdq(&data_array, 1, 1);


	data_array[0] = 0x87002300;  // zhangxian reg:C487=0x18
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x18C42300;
    dsi_set_cmdq(&data_array, 1, 1);

	data_array[0] = 0xA4002300;  // // reg:C0A4=0x00
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x00C02300;
    dsi_set_cmdq(&data_array, 1, 1);

	data_array[0] = 0xA4002300;  // // reg:C1a4=0xf0
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0xf0C12300;
    dsi_set_cmdq(&data_array, 1, 1);


	data_array[0] = 0xb2002300;  // // reg:C4b2=0x81
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x81C42300;
    dsi_set_cmdq(&data_array, 1, 1);


	data_array[0] = 0x93002300;  // // reg:C193=0x90
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x90C12300;
    dsi_set_cmdq(&data_array, 1, 1);

	data_array[0] = 0xb2002300;  // // reg:C4b2=0x81
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x81C42300;
    dsi_set_cmdq(&data_array, 1, 1);


	data_array[0] = 0xb5002300;  // // reg:C4b5=0x80
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x80C42300;
    dsi_set_cmdq(&data_array, 1, 1);


	data_array[0] = 0x80002300;  // // reg:C480=0x30
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x30C42300;
    dsi_set_cmdq(&data_array, 1, 1);



	data_array[0] = 0x8b002300;  // // reg:C48b=0x40
    dsi_set_cmdq(&data_array, 1, 1);
    data_array[0] = 0x40C42300;
    dsi_set_cmdq(&data_array, 1, 1);

    /*===================Gamma====================*/
    data_array[0] = 0x00002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00112902;
    data_array[1] = 0x231d0ae1;
    data_array[2] = 0x0a0f040d;
    data_array[3] = 0x0a060309;
    data_array[4] = 0x0d110e05;
    data_array[5] = 0x00000001;
    dsi_set_cmdq(&data_array, 6 ,1);

    data_array[0] = 0x00002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00112902;
    data_array[1] = 0x221d0ae2;
    data_array[2] = 0x0b0e040e;
    data_array[3] = 0x0906020a;
    data_array[4] = 0x0d110e05;
    data_array[5] = 0x00000001;
    dsi_set_cmdq(&data_array, 6 ,1);

    data_array[0] = 0x00002300;
    dsi_set_cmdq(&data_array, 1, 1);

    data_array[0] = 0x00042902;
    data_array[1] = 0x000000ff;
    dsi_set_cmdq(&data_array, 2 ,1);
    }
#endif

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


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

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
		params->dsi.vertical_backporch					= 40;//14;	//14;
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
		params->dsi.PLL_CLOCK = 259; //264 //210; //211;//208;//230;//270;//245;//250//260;//265; //273;		

}

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0, id1 = 0,id2 = 0;
	unsigned int check_esd = 0;
	unsigned int array[16]; 
	unsigned char buffer[4];
       int volt = 0;
	unsigned int data[4];
	
	array[0] = 0x00043700;// read id return 4 bytes,version and id
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);	
	read_reg_v2(0xA1, buffer, 4);
       id1 = buffer[2];
       id2 = buffer[3];
       id = (id1<<8 | id2);
       LCM_DBG("lcm_compare_id read id=0x%x, id1=0x%x, id2=0x%x",id, id1,id2);
    
	array[0] = 0x00033700;// read esd return 3 bytes
	dsi_set_cmdq(array, 1, 1);	
	MDELAY(10);
	read_reg_v2(0x0A, buffer, 3);    
       check_esd = buffer[0];
       LCM_DBG("lcm_compare_id read check_esd=0x%x",check_esd);
 
       if(LCM_ID_OTM1283 == id)
       {
           IMM_GetOneChannelValue(0, data, &volt);
      	    #ifndef BUILD_LK
      	        printk(" lcm_compare_id tcl_oncell lishengli    volt = %d ", volt);
      	    #else
      	        printf(" lcm_compare_id tcl_onclee lishengli   volt = %d ", volt);
      	    #endif
      
      	    if(volt > 100)
      	 	 return 1;
      	    else
      		return 0;
	}
	else
	    return 0;

	//return (LCM_ID_OTM1283 == id)?1:0;
}                                     

static void lcm_init(void)
{
    unsigned int data_array[16];
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
//    lcm_init_registers();
    //data_array[0] = 0x00352500;
    //dsi_set_cmdq(&data_array, 1, 1);
   // push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    
}

static void lcm_suspend(void)
{
	LCM_DBG("lcm_suspend");


	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
    SET_RESET_PIN(1);
	SET_RESET_PIN(0);	
	MDELAY(10);	
	SET_RESET_PIN(1);
    MDELAY(120);	
}


static void lcm_resume(void)
{
	LCM_DBG("lcm_resume");
	lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


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
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}


static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

extern void Tinno_set_HS_read();
extern void Tinno_restore_HS_read();

static unsigned int lcm_esd_check(void)
{
	unsigned int ret=FALSE;
#ifndef BUILD_LK	
	char  buffer[6];
	int   array[4];
	char esd1,esd2,esd3,esd4;
    if(lcm_esd_test)
    {
       lcm_esd_test = FALSE;
       return TRUE;
    }
	//Tinno_set_HS_read();
	array[0] = 0x00013708;
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0x0A, buffer, 1);
	esd1=buffer[0];

	array[0] = 0x00013708;
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0x0E, buffer, 1);
	esd2=buffer[0];

	//Tinno_restore_HS_read();
	
#if 0	
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x0D, buffer, 2);
	esd2=buffer[0];
	
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);	
	read_reg_v2(0x0E, buffer, 2);
	esd3=buffer[0];

	//array[0] = 0x00023700;
	//dsi_set_cmdq(array, 1, 1);	
	//read_reg_v2(0x0E, buffer, 2);
	//esd4=buffer[0];
    //    LCM_DBG("lcm_esd_check: esd1234 = %x,%x,%x,%x",esd1,esd2,esd3);
	//if(esd1==0x9C && esd2==0 && esd3==0 && esd4==0x80)
	if(esd1==0x9C && esd2==0 && esd3==0x80)
#endif
	//LCM_DBG("lcm_esd_check: esd1 = %x,esd2 = %x",esd1,esd2);
	if(esd1==0x9C&&(esd2&0xF0)==0x80)
	{
		ret=FALSE;
	}
	else
	{			 
		ret=TRUE;
	}
#endif
	return ret;
}

static unsigned int lcm_esd_recover(void)
{
    lcm_init();
    return TRUE;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm1283a_hd720_dsi_vdo_lcm_drv = 
{
    .name			= "otm1283a_hd720_dsi_vdo_lcm_boe",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
//.set_backlight	= lcm_setbacklight,
//	.set_pwm        = lcm_setpwm,
//	.get_pwm        = lcm_getpwm,
#endif
};
