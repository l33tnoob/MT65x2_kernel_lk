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
#define LCM_PRINT printk
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (800)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x00   // END OF REGISTERS MARKER


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)      

//#define LCM_DSI_CMD_MODE

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/
#if 1
    		    {0xB9,3,{0xFF,0x83,0x69}}, 
		    {0xBA,15,{0x31,0x00,0x16,0xCA,0xB0,0x0A,0x00,0x10,0x28,0x02,0x21,0x21,0x9A,0x1A,0x8F}}, 
// Set GIP, for ?＜SDC Quad ASG, Forward, VFP, VBP
// Support GS_Panel=1
		    {0x3A,1,{0x70}},

		   {0xD5,59,{0x00,0x00,0x01,0x00,0x03,0x00,0x00,0x18,0x01,0x00,
		            0x00,0x00,0x01,0x60,0x37,0x00,0x00,0x03,0x07,0x08,
		            0x47,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
		            0x00,0x00,0x03,0x00,0x00,0x14,0x00,0x00,0x91,0x13,
		            0x35,0x57,0x75,0x18,0x00,0x00,0x00,0x86,0x64,0x42,
		            0x20,0x00,0x49,0x00,0x00,0x00,0x90,0x02,0x24}},
		            
		            
		   {0xFD,32,{0x24,0x46,0x64,0x08,0x00,0x00,0x00,0x87,0x75,0x53,
		   	    	    0x31,0x11,0x59,0x00,0x00,0x00,0x00,0x01,0x00,0x00,
		   	           0x00,0x0F,0x00,0x0F,0xFF,0xFF,0x0F,0x00,0x0F,0xFF,
		   	    	    0xFF,0x00,0x85,0x5A}},

		{0xB1,10 ,{0x0B, 0x83 ,0x77 ,0x00, 0x0F, 0x0F, 0x17 ,0x17, 0x0C, 0x0A}},
//VGH=13V, VGL=-12V, VSP=5.26V, VSN=-5.2V    VSPR=4.45V, VSNR=-4.45V

		{0xB3,4,{0x83,0x00,0x31,0x03}},

		{0xB4,1,{0x00}},

	       {0xB6,2,{0xA4,0xA4}},

//CMD_CB_2DATA  DBH CB, 6D ; ??
		//{0xCB,1,{0x6D}},

		{0xCC,1,{0x0E}},

	{0xC1,1,{0x00}},

	{0xC6,4,{0x41 ,0xFF ,0x7A, 0xFF}},

	{0xEA,1,{0x72}},
//CABC_PWM2_OE=?．0?．: CABC_PWM2_OUT output disable
//VCOM_PWR=?．1?．: VCOM power source comes from VSN

	{0xE3, 4,{0x07, 0x0F, 0x07, 0x0F}},

	{0xC0, 6,{0x73, 0x50, 0x00, 0x34, 0xC4, 0x09}},
//Set Gamma
	{0xE0,35,{0x01 ,0x0E, 0x12, 0x29, 0x2D, 0x30, 0x1E, 0x3B, 0x08, 0x0D,
		0x0F, 0x13, 0x15, 0x13 ,0x14, 0x10, 0x16, 0x01 ,0x0E, 0x11,
		 0x2A, 0x2D, 0x30, 0x1D, 0x3B, 0x08, 0x0D, 0x0F, 0x13, 0x15,
		  0x13 ,0x14, 0x0F, 0x16, 0x01}},
#endif	

	#if 0
	{0xB9,3,{0xff,0x83,0x69}},
	{0xBA, 15,{0x31,0x00,0x16, 0xCA, 0xB0, 0x0A, 0x00, 0x10, 0x28, 0x02, 0x21, 0x21, 0x9A, 0x1A, 0x8F}},
	{0x3A, 1,{0x70}},
	{0xD5,92,{0x00,0x00,0x02,0x00,0x04,0x00,0x00,0x18,0x01,0x00,
	                0x00,0x00,0x01,0x48,0x37,0x00,0x00,0x03,0x49,0x4A,
	                0x47,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00, 
	                0x00,0x00,0x03,0x00,0x00,0x10,0x00,0x00,0x91,0x13,
	                0x35,0x57,0x75,0x18,0x00,0x00,0x00,0x86,0x64,0x42,
	                0x20,0x00,0x49,0x00,0x00,0x00,0x90,0x02,0x24,0x46,
	                0x64,0x08,0x00,0x00,0x00,0x87,0x75,0x53,0x31,0x11,
	                0x59,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x0F,
	                0x00,0x0F,0xFF,0xFF,0x0F,0x00,0x0F,0xFF,0xFF,0x00,
	                0x05,0x5A}},
	

	{0xB1,10,{0x0B,0x83,0x77,0x00,0x0F,0x0F,0x17,0x17,0x0C,0x1A}},

	
	//{0xB2,15,{0x00,0x20,0x03,0x03,0x70,0x00,0xFF,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x01}},
	{0xB3,4,{0x83,0x00,0x31,0x03}},
	{0xB4,1,{0x00}},
	{0xB6,2,{0xA4,0xA4}},


	{0xCC,1,{0x06}},
	{0xC6,3,{0x41,0xFF, 0x7D}},

	{0xEA,1,{0x72}},       
	{0xE3,4,{0x07,0x0F, 0x07, 0x0F}},

	{0xC0,6,{0x73,0x50, 0x00, 0x34,0xC4, 0x09}},
       {0xE0,35,{0x00,0x06,0x0C,0x31,0x38,0x3F,0x1B,0x3B,0x07,0x0D,
       			0x0F,0x12,0x15,0x13,0x14,0x0F,0x16,0x00,0x07,0x0D,
       			0x31,0x38,0x3F,0x1B,0x3B,0x07,0x0C,0x0F,0x12,0x15,
       			0x13,0x14,0x0F,0x16,0x01}},
	

	//{0x2C,0,{}},
	#endif
	{0x11,0,{}},
	{REGFLAG_DELAY, 120, {}},
	{0x29,0,{}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 100, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	//{0x28, 1, {0x00}},
    //{REGFLAG_DELAY, 10, {}},
    // Sleep Mode On
	{0x10, 1, {0x00}},
    {REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xB9,	3,	{0xFF, 0x83, 0x69}},
	{REGFLAG_DELAY, 10, {}},

        // Sleep Mode On
        // {0xC3, 1, {0xFF}},

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
				//MDELAY(10);
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

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=480*3;

		params->dsi.vertical_sync_active				= 4;
		params->dsi.vertical_backporch					= 11;
		params->dsi.vertical_frontporch					= 11;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 11;
		params->dsi.horizontal_backporch				= 31;
		params->dsi.horizontal_frontporch				= 31;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		//params->dsi.pll_div1=26;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		//params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		params->dsi.PLL_CLOCK=186;
}


static void lcm_init(void)
{
	
    SET_RESET_PIN(0);
    MDELAY(25);
    SET_RESET_PIN(1);
    MDELAY(100);
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);


}


static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
}


static void lcm_resume(void)
{
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

	dsi_set_cmdq(&data_array, 7, 0);

}

static void lcm_setbacklight(unsigned int level)
{
	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_setpwm(unsigned int divider)
{
	// TBD
}


static unsigned int lcm_getpwm(unsigned int divider)
{
	// ref freq = 15MHz, B0h setting 0x80, so 80.6% * freq is pwm_clk;
	// pwm_clk / 255 / 2(lcm_setpwm() 6th params) = pwm_duration = 23706
	unsigned int pwm_clk = 23706 / (1<<divider);	
	return pwm_clk;
}


// ---------------------------------------------------------------------------
//  Get LCM ID Information
// ---------------------------------------------------------------------------
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
static unsigned int lcm_compare_id()
    {
        unsigned int id = 0;
        unsigned char buffer[2];
        unsigned int array[16];
       int volt = 0;
        unsigned int data[4];

        SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
        SET_RESET_PIN(0);
        MDELAY(10);
        SET_RESET_PIN(1);
        MDELAY(10);
    
        push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
    
        array[0] = 0x00023700;// read id return two byte,version and id
        dsi_set_cmdq(array, 1, 1);
        //id = read_reg(0xF4);
        read_reg_v2(0xF4, buffer, 2);
        id = buffer[0]; //we only need ID
        
        #ifdef BUILD_LK
           printf("%s, zenghaihui id1 = 0x%08x\n", __func__, id); 
        #else
           printk("%s, zenghaihui id1 = 0x%08x\n", __func__, id);   
        #endif

        //LINE<JIRA_ID><DATE20140319><BUG_INFO>zenghaihui
        //return (0x69 == id)?1:0;
       if(1) // 0x69 == id)
       {
               IMM_GetOneChannelValue(0, data, &volt);
        #ifndef BUILD_LK
                    printk(" lcm_compare_id tcl_oncell zenghaihui    volt = %d ", volt);
        #else
                    printf(" lcm_compare_id tcl_onclee zenghaihui   volt = %d ", volt);
        #endif
          
                if(volt > 200)
                {
                     return 1;
                }
                else
                {
                    return 0;
                }
        }
        else
        {
            return 0;
        }
        
    }

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
//extern void Tinno_set_HS_read();
//extern void Tinno_restore_HS_read();
static unsigned int lcm_esd_check(void)
{
	unsigned int ret=FALSE;
#if 0//ndef BUILD_LK	
	char  buffer[6];
	int   array[4];
	char esd1,esd2,esd3,esd4;
        if(lcm_esd_test)
        {
            lcm_esd_test = FALSE;
            return TRUE;
        } 

	Tinno_set_HS_read();
#if 1
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0x0a, buffer, 2);
	esd1=buffer[1]; //0x9c
	
	//LCM_DBG("esd1 = 0x%x, esd2 = 0x%x,esd3 = 0x%x", esd1, esd2, esd3);
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0x09, buffer, 2);
	esd2=buffer[0];
	esd3=buffer[1];
	//printk("esd1 = 0x%x, esd2 = 0x%x,esd3 = 0x%x", esd1, esd2, esd3);
	Tinno_restore_HS_read();
	
	if(esd1 == 0x9c && esd2==0x80 && esd3==0x73)
	{
		ret=FALSE;
	}
	else
	{			 
		ret=TRUE;
	}
#endif

	
#endif
	return ret;
}

static unsigned int lcm_esd_recover(void)
{
	lcm_resume();
	return TRUE;
}

LCM_DRIVER hx8369b_wvga_dsi_vdo_tinno_txd_drv = 
{
    .name			= "hx8369_wvga_dsi_vdo_tinno_txd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	//.set_backlight	= lcm_setbacklight,
	//.set_pwm        = lcm_setpwm,
	//.get_pwm        = lcm_getpwm,
	.esd_check = lcm_esd_check,
	//.esd_recover = lcm_esd_recover,	
	.compare_id     = lcm_compare_id,
#if defined(LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };

