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
#include <linux/signal.h>
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
extern void Tinno_set_HS_read();
extern void Tinno_restore_HS_read();
#endif

#define LCM_ID_OTM1283 (0x1283)

#define LCM_DBG(fmt, arg...) \
	LCM_PRINT ("[LCM-OTM1283A-DSI-VDO] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

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
static unsigned int dot_to_column_inversion_flag = FALSE;
static unsigned int sys_suspend_num = 0;
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

    {0x00,1,{0x00}}, 
    {0xFF,3,{0x12,0x83,0x01}}, 
    
    {0x00,1,{0x80}}, 
    {0xFF,2,{0x12,0x83}}, 
    
    //Start added by mickal.ma   
    {0x00, 1, {0xA0}}, 
    {0xC1, 1, {0x02}},	// Disable time-out function
    //End added by mickal.ma 
       
    {0x00,1,{0x80}}, 
    {0xC0,9,{0x00,0x64,0x00,0x0F,0x11,0x00,0x64,0x0F,0x11}}, 
    
    {0x00,1,{0x90}}, 
    {0xC0,6,{0x00,0x55,0x00,0x01,0x00,0x04}}, 
    
    {0x00,1,{0xA4}}, 
    {0xC0,1,{0x00}}, 
    
    {0x00,1,{0x87}}, 
    {0xC4,1,{0x18}}, 
#if 0  
  
#ifndef BUILD_LK    //kernel
    {0x00,1,{0xB3}}, 
    {0xC0,2,{0x00,0x55}}, 
#else
    {0x00,1,{0xB3}}, 
    {0xC0,2,{0x00,0x10}},	//lk,1+2dot
#endif

#else
    {0x00,1,{0xB3}}, 
    {0xC0,2,{0x00,0x10}},	//1+2dot
#endif
    {0x00,1,{0x81}}, 
    {0xC1,1,{0x66}}, 
    
    {0x00,1,{0x81}}, 
    {0xC4,1,{0x84}}, 
    
    {0x00,1,{0x82}}, 
    {0xC4,1,{0x02}}, 
    
    {0x00,1,{0x90}}, 
    {0xC4,1,{0x49}}, 
    
    {0x00,1,{0xB9}}, 
    {0xB0,1,{0x51}}, 
    
    {0x00,1,{0xC6}}, 
    {0xB0,1,{0x03}}, 
    
    {0x00,1,{0x90}}, 
    {0xF5,4,{0x02,0x11,0x02,0x11}}, 
    
    {0x00,1,{0x90}}, 
    {0xC5,1,{0x50}}, 
    
    {0x00,1,{0x94}}, 
    {0xC5,1,{0x66}}, 
    
    {0x00,1,{0xB2}}, 
    {0xF5,2,{0x00,0x00}}, 
    
    {0x00,1,{0xB4}}, 
    {0xF5,2,{0x00,0x00}}, 
    
    {0x00,1,{0xB6}}, 
    {0xF5,2,{0x00,0x00}}, 
    
    {0x00,1,{0xB8}}, 
    {0xF5,2,{0x00,0x00}}, 
    
    {0x00,1,{0x94}}, 
    {0xF5,1,{0x02}}, 
    
    {0x00,1,{0xBA}}, 
    {0xF5,1,{0x03}}, 
    
    {0x00,1,{0xB4}}, 
    {0xC5,1,{0xC0}}, 
    
    {0x00,1,{0xA0}}, 
    {0xC4,14,{0x05,0x10,0x04,0x02,0x05,0x15,0x11,0x05,0x10,0x07,0x02,0x05,0x15,0x11}}, 
    
    {0x00,1,{0xB0}}, 
    {0xC4,2,{0x66,0x66}}, 
    
    {0x00,1,{0x91}}, 
    {0xC5,2,{0x37,0x52}}, 
    
    {0x00,1,{0xB0}}, 
    {0xC5,2,{0x04,0x58}}, 
    
    {0x00,1,{0xB5}}, 
    {0xC5,6,{0x03,0xE8,0x40,0x03,0xE8,0x40}}, 
    
    {0x00,1,{0xBB}}, 
    {0xC5,1,{0x80}}, 
    
    {0x00,1,{0x00}}, 
    {0xD0,1,{0x40}}, 
    
    {0x00,1,{0x00}}, 
    {0xD1,2,{0x00,0x00}}, 
    
    {0x00,1,{0x80}}, 
    {0xCB,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0x90}}, 
    {0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0xFF,0x00}}, 
    
    {0x00,1,{0xA0}}, 
    {0xCB,15,{0xFF,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xB0}}, 
    {0xCB,15,{0x00,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xC0}}, 
    {0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x05,0x05}}, 
    
    {0x00,1,{0xD0}}, 
    {0xCB,15,{0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xE0}}, 
    {0xCB,14,{0x00,0x00,0x00,0x05,0x00,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xF0}}, 
    {0xCB,11,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}}, 
    
    {0x00,1,{0x80}}, 
    {0xCC,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x10,0x0E}}, 
    
    {0x00,1,{0x90}}, 
    {0xCC,15,{0x0C,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xA0}}, 
    {0xCC,14,{0x00,0x00,0x00,0x09,0x00,0x0F,0x0D,0x0B,0x01,0x03,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xB0}}, 
    {0xCC,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x10,0x0E}}, 
    
    {0x00,1,{0xC0}}, 
    {0xCC,15,{0x0C,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xD0}}, 
    {0xCC,14,{0x00,0x00,0x00,0x09,0x00,0x0F,0x0D,0x0B,0x01,0x03,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0x80}}, 
    {0xCE,12,{0x87,0x03,0x06,0x86,0x03,0x06,0x85,0x03,0x06,0x84,0x03,0x06}}, 
    
    {0x00,1,{0x90}}, 
    {0xCE,14,{0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xA0}}, 
    {0xCE,14,{0x38,0x05,0x84,0xFE,0x00,0x06,0x00,0x38,0x04,0x84,0xFF,0x00,0x06,0x00}}, 
    
    {0x00,1,{0xB0}}, 
    {0xCE,14,{0x38,0x03,0x85,0x00,0x00,0x06,0x00,0x38,0x02,0x85,0x01,0x00,0x06,0x00}}, 
    
    {0x00,1,{0xC0}}, 
    {0xCE,14,{0x38,0x01,0x85,0x02,0x00,0x06,0x00,0x38,0x00,0x85,0x03,0x00,0x06,0x00}}, 
    
    {0x00,1,{0xD0}}, 
    {0xCE,14,{0x30,0x00,0x85,0x04,0x00,0x06,0x00,0x30,0x01,0x85,0x05,0x00,0x06,0x00}}, 
    
    {0x00,1,{0x80}}, 
    {0xCF,14,{0x70,0x00,0x00,0x10,0x00,0x00,0x00,0x70,0x00,0x00,0x10,0x00,0x00,0x00}}, 
    
    {0x00,1,{0x90}}, 
    {0xCF,14,{0x70,0x00,0x00,0x10,0x00,0x00,0x00,0x70,0x00,0x00,0x10,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xA0}}, 
    {0xCF,14,{0x70,0x00,0x00,0x10,0x00,0x00,0x00,0x70,0x00,0x00,0x10,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xB0}}, 
    {0xCF,14,{0x70,0x00,0x00,0x10,0x00,0x00,0x00,0x70,0x00,0x00,0x10,0x00,0x00,0x00}}, 
    
    {0x00,1,{0xC0}}, 
    {0xCF,11,{0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x80,0x01,0x03,0x08}}, 
    
    {0x00,1,{0x00}}, 
	{0xD8,2,{0xAC,0xAC}}, 
    
    {0x00,1,{0x00}}, 
    {0xD9,1,{0xA2}},//解残影 
    
    {0x00,1,{0x00}}, 
    //{0xE1,16,{0x04,0x09,0x0C,0x0C,0x05,0x0D,0x0A,0x08,0x05,0x08,0x0E,0x07,0x0E,0x16,0x12,0x0A}},   //old
     {0xE1,16,{0x04,0x09,0x0C,0x09,0x02,0x0a,0x07,0x05,0x05,0x08,0x0E,0x07,0x0E,0x16,0x12,0x0A}},
 
 
    
    {0x00,1,{0x00}}, 
    //{0xE2,16,{0x04,0x08,0x0D,0x0D,0x04,0x0D,0x09,0x09,0x05,0x08,0x0E,0x07,0x0E,0x16,0x11,0x0A}},   //old
       {0xE2,16,{0x04,0x08,0x0D,0x0b,0x01,0x0a,0x06,0x06,0x05,0x08,0x0E,0x07,0x0E,0x16,0x11,0x0A}}, 

    
    {0x00,1,{0xA4}}, 
    {0xC1,1,{0xF0}}, 
    
    {0x00,1,{0xB2}}, 
    {0xC4,1,{0x81}}, 
    
    {0x00,1,{0x93}}, 
    {0xC1,1,{0x90}}, 
    
  //    {0x00,1,{0xB4}}, //jovan
  //  {0xC0,1,{0x01}}, //jovan
    
    {0x00,1,{0xB2}}, 
    {0xC4,1,{0x81}}, 
    
    {0x00,1,{0xB5}}, 
    {0xC4,1,{0x80}}, 
    
    {0x00,1,{0x80}}, 
    {0xC4,1,{0x30}}, 
    
    {0x00,1,{0x8B}}, 
    {0xC4,1,{0x40}}, 
    
    {0x00,1,{0x00}}, 
    {0xFF,3,{0xFF,0xFF,0xFF}}, 


    
    {0x35,1,{0x00}},
    
    {0x26,1,{0x01}},     //jovan
    {REGFLAG_DELAY,20,{}},   //jovan
    
    {0x13, 1, {0x00}},
	{REGFLAG_DELAY,20,{}},
    
    {0x00,1,{0x83}},
    {0xb3,1,{0x01}},
    
    {0x11,1,{0x00}},
    
    {REGFLAG_DELAY,150,{}},
    {0x29,1,{0x00}},
    // Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}

};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
	//Normal mode on
	{0x13, 1, {0x00}},
	{REGFLAG_DELAY,20,{}},
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	//All pixel off
	{0x22, 1, {0x00}},
	{REGFLAG_DELAY,50,{}},
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY,20,{}},
	
	{0x00,1,{0x94}},                                //让AVDD电压从5.xV快速掉到2.8V.
	{0xf5,4,{0x00,0x00,0x00,0x00}}, 

	{0x00,1,{0xa5}},             
	{0xC4,1,{0x11}}, 

	
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
	LCM_DBG();
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		LCM_DBG();
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE;
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

		
		params->dsi.vertical_sync_active				= 4;	//2;
		params->dsi.vertical_backporch					= 40;	//14;
		params->dsi.vertical_frontporch					= 40;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 10;	//2;
		params->dsi.horizontal_backporch				= 160;	//60;	//42;
		params->dsi.horizontal_frontporch				= 160;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		//1 Every lane speed
		//params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		//params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	
		//params->dsi.fbk_div =0x12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
		
		// zhangxiaofei add for test
		params->dsi.PLL_CLOCK = 265;		

}

static unsigned int lcm_compare_id(void)
{
	unsigned int id=0, id1 = 0,id2 = 0;
	unsigned int check_esd = 0;
	unsigned int array[16]; 
	unsigned char buffer[4];
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
	return (LCM_ID_OTM1283 == id)?1:0;
}                                     

#ifndef BUILD_LK
static struct LCM_setting_table lcm_column_inversion_setting[] = {
	{0x00,1,{0x00}}, 
    {0xFF,3,{0x12,0x83,0x01}}, 
        
    {0x00,1,{0x80}}, 
    {0xFF,2,{0x12,0x83}},
    
	{0x00,1,{0xB3}}, 
    {0xC0,2,{0x00,0x55}}, 
    
    {0x00,1,{0x00}}, 
    {0xFF,3,{0xFF,0xFF,0xFF}},
        
    {REGFLAG_DELAY,120,{}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

void set_lcm_column_inversion()
{
	LCM_DBG();
	push_table(lcm_column_inversion_setting, sizeof(lcm_column_inversion_setting) / sizeof(struct LCM_setting_table), 1);
}

#endif

static void lcm_init(void)
{
	LCM_DBG();
	#ifndef BUILD_LK
	struct sigaction tact;	
	#endif
	mt_set_gpio_mode(GPIO112, GPIO_MODE_01);//Line added mickal.ma
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    
    #ifndef BUILD_LK
    if(sys_suspend_num > 3 && dot_to_column_inversion_flag == FALSE){
		set_lcm_column_inversion();
		dot_to_column_inversion_flag = TRUE;
	}
	#endif
}


static void lcm_suspend(void)
{
	LCM_DBG();
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	#if 0
	//SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    //MDELAY(10);
    //SET_RESET_PIN(1);
    //MDELAY(10);
    #endif
}


static void lcm_resume(void)
{
	LCM_DBG();	
	sys_suspend_num = sys_suspend_num + 1;
	LCM_DBG("sys_suspend_num = %d\n",sys_suspend_num);
	lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	LCM_DBG();
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
	LCM_DBG();
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
	
	Tinno_set_HS_read();
	
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x0A, buffer, 2);
	esd1=buffer[0];
	
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x0B, buffer, 2);
	esd2=buffer[0];
	
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);	
	read_reg_v2(0x0D, buffer, 2);
	esd3=buffer[0];
	
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);	
	read_reg_v2(0x0E, buffer, 2);
	esd4=buffer[0];	
	//LCM_DBG("esd1 = 0x%x, esd2 = 0x%x, esd3 = 0x%x, esd4 = 0x%x", esd1, esd2, esd3, esd4);	
	
	Tinno_restore_HS_read();
	
	//LCM_DBG("esd1 = 0x%x", esd1);
	//if(esd1==0x9C)
	if(esd1==0x9C && esd2==0 && esd3==0 && esd4==0x80)	
	{
		ret=FALSE;
	}
	else
	{	
		LCM_DBG("esd1 = 0x%x, esd2 = 0x%x, esd3 = 0x%x, esd4 = 0x%x", esd1, esd2, esd3, esd4);		 
		ret=TRUE;
	}
#endif
	return ret;
}

static unsigned int lcm_esd_recover(void)
{
	LCM_DBG();
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(120);
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);
    return TRUE;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm1283a_hd720_dsi_vdo_truly_lcm_drv = 
{
    .name			= "otm1283a_hd720_dsi_vdo_truly",
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
