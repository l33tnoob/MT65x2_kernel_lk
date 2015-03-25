/*
 * drivers/leds/leds-mt65xx.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * mt65xx leds driver
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/leds-mt65xx.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
//#include <cust_leds.h>
//#include <cust_leds_def.h>
#include <mach/mt_pwm.h>
#include <mach/mt_boot.h>
//#include <mach/mt_gpio.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include "leds_sw.h"
//#include <linux/leds_sw.h>
//#include <mach/mt_pmic_feature_api.h>
//#include <mach/mt_boot.h>


static DEFINE_MUTEX(leds_mutex);
static DEFINE_MUTEX(leds_pmic_mutex);
//#define ISINK_CHOP_CLK
/****************************************************************************
 * variables
 ***************************************************************************/
//struct cust_mt65xx_led* bl_setting_hal = NULL;
 static unsigned int bl_brightness_hal = 102;
static unsigned int bl_duty_hal = 21;
static unsigned int bl_div_hal = CLK_DIV1;
static unsigned int bl_frequency_hal = 32000;
//for button led don't do ISINK disable first time
static int button_flag_isink0 = 0;
static int button_flag_isink1 = 0;
static int button_flag_isink2 = 0;
static int button_flag_isink3 = 0;
/*************************************************************/
struct wake_lock leds_suspend_lock;

/****************************************************************************
 * DEBUG MACROS
 ***************************************************************************/
static int debug_enable_led_hal = 1;
#define LEDS_DEBUG(format, args...) do{ \
	if(debug_enable_led_hal) \
	{\
		printk(KERN_WARNING format,##args);\
	}\
}while(0)

#ifdef MTK_HDMI_SUPPORT
extern BOOL hdmi_is_active;
#endif
/****************************************************************************
 * custom APIs
***************************************************************************/
extern unsigned int brightness_mapping(unsigned int level);

/*****************PWM *************************************************/
static int time_array_hal[PWM_DIV_NUM]={256,512,1024,2048,4096,8192,16384,32768};
static unsigned int div_array_hal[PWM_DIV_NUM] = {1,2,4,8,16,32,64,128}; 
static unsigned int backlight_PWM_div_hal = CLK_DIV1;// this para come from cust_leds.

/******************************************************************************
   for DISP backlight High resolution 
******************************************************************************/
#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH
#define MT_LED_INTERNAL_LEVEL_BIT_CNT 10
#endif

// Use Old Mode of PWM to suppoort 256 backlight level
#define BACKLIGHT_LEVEL_PWM_256_SUPPORT 256
extern unsigned int Cust_GetBacklightLevelSupport_byPWM(void);

/****************************************************************************
 * func:return global variables
 ***************************************************************************/

void mt_leds_wake_lock_init(void)
{
	wake_lock_init(&leds_suspend_lock, WAKE_LOCK_SUSPEND, "leds wakelock");
}

unsigned int mt_get_bl_brightness(void)
{
	return bl_brightness_hal;
}

unsigned int mt_get_bl_duty(void)
{
	return bl_duty_hal;
}
unsigned int mt_get_bl_div(void)
{
	return bl_div_hal;
}
unsigned int mt_get_bl_frequency(void)
{
	return bl_frequency_hal;
}

unsigned int *mt_get_div_array(void)
{
	return &div_array_hal[0];
}

void mt_set_bl_duty(unsigned int level)
{
	bl_duty_hal = level;
}

void mt_set_bl_div(unsigned int div)
{
	bl_div_hal = div;
}

void mt_set_bl_frequency(unsigned int freq)
{
	 bl_frequency_hal = freq;
}

struct cust_mt65xx_led * mt_get_cust_led_list(void)
{
	return get_cust_led_list();
}


/****************************************************************************
 * internal functions
 ***************************************************************************/
//#if 0
static int brightness_mapto64(int level)
{
        if (level < 30)
                return (level >> 1) + 7;
        else if (level <= 120)
                return (level >> 2) + 14;
        else if (level <= 160)
                return level / 5 + 20;
        else
                return (level >> 3) + 33;
}



static int find_time_index(int time)
{	
	int index = 0;	
	while(index < 8)	
	{		
		if(time<time_array_hal[index])			
			return index;		
		else
			index++;
	}	
	return PWM_DIV_NUM-1;
}

int mt_led_set_pwm(int pwm_num, struct nled_setting* led)
{
	//struct pwm_easy_config pwm_setting;
	struct pwm_spec_config pwm_setting;
	int time_index = 0;
	pwm_setting.pwm_no = pwm_num;
    pwm_setting.mode = PWM_MODE_OLD;
    
        LEDS_DEBUG("[LED]led_set_pwm: mode=%d,pwm_no=%d\n", led->nled_mode, pwm_num);  
	//if((pwm_num != PWM3 && pwm_num != PWM4 && pwm_num != PWM5))//AP PWM all support OLD mode in MT6589
		pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
	//else
		//pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
    
	switch (led->nled_mode)
	{
		case NLED_OFF :
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 0;
			pwm_setting.clk_div = CLK_DIV1;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100;
			break;
            
		case NLED_ON :
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 30;
			pwm_setting.clk_div = CLK_DIV1;			
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100;
			break;
            
		case NLED_BLINK :
			LEDS_DEBUG("[LED]LED blink on time = %d offtime = %d\n",led->blink_on_time,led->blink_off_time);
			time_index = find_time_index(led->blink_on_time + led->blink_off_time);
			LEDS_DEBUG("[LED]LED div is %d\n",time_index);
			pwm_setting.clk_div = time_index;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = (led->blink_on_time + led->blink_off_time) * MIN_FRE_OLD_PWM / div_array_hal[time_index];
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = (led->blink_on_time*100) / (led->blink_on_time + led->blink_off_time);
	}
	
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
	pwm_set_spec_config(&pwm_setting);

	return 0;
	
}


//***********************MT6589 led breath funcion*******************************//
#if 0
static int led_breath_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led)
{
	//int time_index = 0;
	//int duty = 0;
	LEDS_DEBUG("[LED]led_blink_pmic: pmic_type=%d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK1) || led->nled_mode != NLED_BLINK) {
		return -1;
	}
				
	switch(pmic_type){
		case MT65XX_LED_PMIC_NLED_ISINK0://keypad center 4mA
			upmu_set_isinks_ch0_mode(PMIC_PWM_0);// 6320 spec 
			upmu_set_isinks_ch0_step(0x0);//4mA
			upmu_set_isinks_breath0_trf_sel(0x04);
			upmu_set_isinks_breath0_ton_sel(0x02);
			upmu_set_isinks_breath0_toff_sel(0x03);
			upmu_set_isink_dim1_duty(15);
			upmu_set_isink_dim1_fsel(11);//6320 0.25KHz
			upmu_set_rg_spk_div_pdn(0x01);
			upmu_set_rg_spk_pwm_div_pdn(0x01);
			upmu_set_isinks_ch0_en(0x01);
			break;
		case MT65XX_LED_PMIC_NLED_ISINK1://keypad LR  16mA
			upmu_set_isinks_ch1_mode(PMIC_PWM_1);// 6320 spec 
			upmu_set_isinks_ch1_step(0x3);//16mA
			upmu_set_isinks_breath0_trf_sel(0x04);
			upmu_set_isinks_breath0_ton_sel(0x02);
			upmu_set_isinks_breath0_toff_sel(0x03);
			upmu_set_isink_dim1_duty(15);
			upmu_set_isink_dim1_fsel(11);//6320 0.25KHz
			upmu_set_rg_spk_div_pdn(0x01);
			upmu_set_rg_spk_pwm_div_pdn(0x01);
			upmu_set_isinks_ch1_en(0x01);
			break;	
		default:
		break;
	}
	return 0;

}

#endif


#define PMIC_PERIOD_NUM 9
// 100 * period, ex: 0.01 Hz -> 0.01 * 100 = 1
int pmic_period_array[] = {250,500,1000,1250,1666,2000,2500,10000};
//int pmic_freqsel_array[] = {99999, 9999, 4999, 1999, 999, 499, 199, 4, 0};
int pmic_freqsel_array[] = {0, 4, 199, 499, 999, 1999, 1999, 1999};

static int find_time_index_pmic(int time_ms) {
	int i;
	for(i=0;i<PMIC_PERIOD_NUM;i++) {
		if(time_ms<=pmic_period_array[i]) {
			return i;
		} else {
			continue;
		}
	}
	return PMIC_PERIOD_NUM-1;
}

int mt_led_blink_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led) {
	int time_index = 0;
	int duty = 0;
	LEDS_DEBUG("[LED]led_blink_pmic: pmic_type=%d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK1 && 
		pmic_type!= MT65XX_LED_PMIC_NLED_ISINK2 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK3) || led->nled_mode != NLED_BLINK) {
		return -1;
	}
				
	LEDS_DEBUG("[LED]LED blink on time = %d offtime = %d\n",led->blink_on_time,led->blink_off_time);
	time_index = find_time_index_pmic(led->blink_on_time + led->blink_off_time);
	LEDS_DEBUG("[LED]LED index is %d  freqsel=%d\n", time_index, pmic_freqsel_array[time_index]);
	duty=32*led->blink_on_time/(led->blink_on_time + led->blink_off_time);
	//upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down (Indicator no need)
    upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down  
	switch(pmic_type){
		case MT65XX_LED_PMIC_NLED_ISINK0:
			upmu_set_rg_isink0_ck_pdn(0);
			upmu_set_rg_isink0_ck_sel(0);
			upmu_set_isink_ch0_mode(PMIC_PWM_0);
			upmu_set_isink_ch0_step(0x3);//16mA
			upmu_set_isink_dim0_duty(duty);
			upmu_set_isink_dim0_fsel(pmic_freqsel_array[time_index]);
			upmu_set_isink_breath0_trf_sel(0x0);
			upmu_set_isink_breath0_ton_sel(0x02);
			upmu_set_isink_breath0_toff_sel(0x05);			
			upmu_set_isink_ch0_en(0x01);
			break;
		case MT65XX_LED_PMIC_NLED_ISINK1:
			upmu_set_rg_isink1_ck_pdn(0);
			upmu_set_rg_isink1_ck_sel(0);
			upmu_set_isink_ch1_mode(PMIC_PWM_0);
			upmu_set_isink_ch1_step(0x3);//16mA
			upmu_set_isink_dim1_duty(duty);
			upmu_set_isink_dim1_fsel(pmic_freqsel_array[time_index]);
			upmu_set_isink_breath1_trf_sel(0x0);
			upmu_set_isink_breath1_ton_sel(0x02);
			upmu_set_isink_breath1_toff_sel(0x05);			
			upmu_set_isink_ch1_en(0x01);
			break;	
		case MT65XX_LED_PMIC_NLED_ISINK2:
			upmu_set_rg_isink2_ck_pdn(0);
			upmu_set_rg_isink2_ck_sel(0);
			upmu_set_isink_ch2_mode(PMIC_PWM_0);
			upmu_set_isink_ch2_step(0x3);//16mA
			upmu_set_isink_dim2_duty(duty);
			upmu_set_isink_dim2_fsel(pmic_freqsel_array[time_index]);
			upmu_set_isink_breath2_trf_sel(0x0);
			upmu_set_isink_breath2_ton_sel(0x02);
			upmu_set_isink_breath2_toff_sel(0x05);
			upmu_set_isink_ch2_en(0x01);
			break;	
		case MT65XX_LED_PMIC_NLED_ISINK3:
			upmu_set_rg_isink3_ck_pdn(0);
			upmu_set_rg_isink3_ck_sel(0);
			upmu_set_isink_ch3_mode(PMIC_PWM_0);
			upmu_set_isink_ch3_step(0x3);//16mA
			upmu_set_isink_dim3_duty(duty);
			upmu_set_isink_dim3_fsel(pmic_freqsel_array[time_index]);
			upmu_set_isink_breath3_trf_sel(0x0);
			upmu_set_isink_breath3_ton_sel(0x02);
			upmu_set_isink_breath3_toff_sel(0x05);
			upmu_set_isink_ch3_en(0x01);
			break;	
		default:
		break;
	}
	return 0;
}


int mt_backlight_set_pwm(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	struct pwm_spec_config pwm_setting;
	unsigned int BacklightLevelSupport = Cust_GetBacklightLevelSupport_byPWM();
	pwm_setting.pwm_no = pwm_num;
	
	if (BacklightLevelSupport == BACKLIGHT_LEVEL_PWM_256_SUPPORT)
		pwm_setting.mode = PWM_MODE_OLD;
	else
	pwm_setting.mode = PWM_MODE_FIFO; //new mode fifo and periodical mode

	pwm_setting.pmic_pad = config_data->pmic_pad;
		
	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div_hal = config_data->div;
	}
	else
		pwm_setting.clk_div = div;
	
	if(BacklightLevelSupport== BACKLIGHT_LEVEL_PWM_256_SUPPORT)
	{
		if(config_data->clock_source)
		{
			pwm_setting.clk_src = PWM_CLK_OLD_MODE_BLOCK;
		}
		else
		{
			pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;	 // actually. it's block/1625 = 26M/1625 = 16KHz @ MT6571
		}

		pwm_setting.PWM_MODE_OLD_REGS.IDLE_VALUE = 0;
		pwm_setting.PWM_MODE_OLD_REGS.GUARD_VALUE = 0;
		pwm_setting.PWM_MODE_OLD_REGS.GDURATION = 0;
		pwm_setting.PWM_MODE_OLD_REGS.WAVE_NUM = 0;
		pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 255; // 256 level
		pwm_setting.PWM_MODE_OLD_REGS.THRESH = level;

		LEDS_DEBUG("[LEDS][%d]backlight_set_pwm:duty is %d/%d\n", BacklightLevelSupport, level, pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH);
		LEDS_DEBUG("[LEDS][%d]backlight_set_pwm:clk_src/div is %d%d\n", BacklightLevelSupport, pwm_setting.clk_src, pwm_setting.clk_div);
		if(level >0 && level < 256)
		{
			pwm_set_spec_config(&pwm_setting);
			LEDS_DEBUG("[LEDS][%d]backlight_set_pwm: old mode: thres/data_width is %d/%d\n", BacklightLevelSupport, pwm_setting.PWM_MODE_OLD_REGS.THRESH, pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH);
		}
		else
		{
			LEDS_DEBUG("[LEDS][%d]Error level in backlight\n", BacklightLevelSupport);
			mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
		}
		return 0;

	}
	else
	{
		if(config_data->clock_source)
		{
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
		}
		else
		{
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
		}
	
		if(config_data->High_duration && config_data->low_duration)
		{
			pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
			pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = pwm_setting.PWM_MODE_FIFO_REGS.HDURATION;
		}
		else
		{
			pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
			pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
		}
		pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
		pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
		pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 31;
		pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = (pwm_setting.PWM_MODE_FIFO_REGS.HDURATION+1)*32 - 1;
		pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
		
		LEDS_DEBUG("[LED]backlight_set_pwm:duty is %d\n", level);
		LEDS_DEBUG("[LED]backlight_set_pwm:clk_src/div/high/low is %d%d%d%d\n", pwm_setting.clk_src,pwm_setting.clk_div,pwm_setting.PWM_MODE_FIFO_REGS.HDURATION,pwm_setting.PWM_MODE_FIFO_REGS.LDURATION);
		if(level>0 && level <= 32)
		{
			pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
			pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =  (1 << level) - 1 ;
			//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0 ;
			pwm_set_spec_config(&pwm_setting);
		}else if(level>32 && level <=64)
		{
			pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 1;
			level -= 32;
			pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1 ;
			//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =  0xFFFFFFFF ;
			//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = (1 << level) - 1;
			pwm_set_spec_config(&pwm_setting);
		}else
		{
			LEDS_DEBUG("[LED]Error level in backlight\n");
			//mt_set_pwm_disable(pwm_setting.pwm_no);
			//mt_pwm_power_off(pwm_setting.pwm_no);
			mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
		}
		//printk("[LED]PWM con register is %x \n", INREG32(PWM_BASE + 0x0150));
		return 0;
    }
}

void mt_led_pwm_disable(int pwm_num)
{
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
	mt_pwm_disable(pwm_num, cust_led_list->config_data.pmic_pad);
}

void mt_backlight_set_pwm_duty(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	mt_backlight_set_pwm(pwm_num, level, div, config_data);
}

void mt_backlight_set_pwm_div(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	mt_backlight_set_pwm(pwm_num, level, div, config_data);
}

void mt_backlight_get_pwm_fsel(unsigned int bl_div, unsigned int *bl_frequency)
{

}

void mt_store_pwm_register(unsigned int addr, unsigned int value)
{

}

unsigned int mt_show_pwm_register(unsigned int addr)
{
	return 0;
}
#if 0
static void pmic_isink_power_set(int enable)
{
	int i = 0;
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
	if(enable) {
		for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
			if ((cust_led_list[i].mode == MT65XX_LED_MODE_PMIC) && (cust_led_list[i].data == MT65XX_LED_PMIC_LCD_ISINK0 || 
				cust_led_list[i].data == MT65XX_LED_PMIC_LCD_ISINK1 || cust_led_list[i].data == MT65XX_LED_PMIC_LCD_ISINK2 || 
				cust_led_list[i].data == MT65XX_LED_PMIC_LCD_ISINK3)) {
				upmu_set_rg_drv_2m_ck_pdn(0);
				break;
		    }
		}
		for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
			if ((cust_led_list[i].mode == MT65XX_LED_MODE_PMIC) && (cust_led_list[i].data == MT65XX_LED_PMIC_NLED_ISINK0 ||
				cust_led_list[i].data == MT65XX_LED_PMIC_NLED_ISINK1 || cust_led_list[i].data == MT65XX_LED_PMIC_NLED_ISINK2 || 
				cust_led_list[i].data == MT65XX_LED_PMIC_NLED_ISINK3)) {
				upmu_set_rg_drv_32k_ck_pdn(0);
				break;
		    }
		}
    }else {
    	upmu_set_rg_drv_2m_ck_pdn(1);
		upmu_set_rg_drv_32k_ck_pdn(1);
    }
}
// define for 15 level mapping(backlight controlled by PMIC ISINK channel)

static void brightness_set_pmic_isink_duty(unsigned int level)
{
	if(level<11) {
		upmu_set_isink_dim0_duty(level);
	}else {
		switch(level) {
			case 11:
				upmu_set_isink_dim0_duty(12);
				break;
			case 12:
				upmu_set_isink_dim0_duty(16);
				break;
			case 13:
				upmu_set_isink_dim0_duty(20);
				break;
			case 14:
				upmu_set_isink_dim0_duty(25);
				break;
			case 15:
				upmu_set_isink_dim0_duty(31);
				break;
			default:
				LEDS_DEBUG("mapping BLK level is error for ISINK!\n");
			break;
		}
	}
}
#endif
int mt_brightness_set_pmic(enum mt65xx_led_pmic pmic_type, u32 level, u32 div)
{
		int tmp_level = level;
		//static bool backlight_init_flag[4] = {false, false, false, false};
		static bool backlight_init_flag = false;
		//static bool led_init_flag[4] = {false, false, false, false};
		static bool first_time = true;
		static unsigned char duty_mapping[108] = {
       		 0,	0,	0,	0,	0,	6,	1,	2,	1,	10,	1,	12,
        	 6,	14,	2,	3,	16,	2,	18,	3,	6,	10,	22, 3,
        	12,	8,	6,	28,	4,	30,	7,	10,	16,	6,	5,	18,
        	12,	7,	6,	10,	8,	22,	7,	9,	16,	12,	8,	10,
        	13,	18,	28,	9,	30,	20,	15,	12,	10,	16,	22,	13,
        	11,	14,	18,	12,	19,	15,	26,	13,	16,	28,	21,	14,
        	22,	30,	18,	15,	19,	16,	25,	20,	17,	21,	27,	18,
        	22,	28,	19,	30,	24,	20,	31,	25,	21,	26,	22,	27,
        	23,	28,	24,	30,	25,	31,	26,	27,	28,	29,	30,	31,
    	};
    	static unsigned char current_mapping[108] = {
        	1,	2,	3,	4,	5,	0,	3,	2,	4,	0,	5,	0,
        	1,	0,	4,	3,	0,	5,	0,	4,	2,	1,	0,	5,
        	1,	2,	3,	0,	5,	0,	3,	2,	1,	4,	5,	1,
        	2,	4,	5,	3,	4,	1,	5,	4,	2,	3,	5,	4,
        	3,	2,	1,	5,	1,	2,	3,	4,	5,	3,	2,	4,
        	5,	4,	3,	5,	3,	4,	2,	5,	4,	2,	3,	5,
        	3,	2,	4,	5,	4,	5,	3,	4,	5,	4,	3,	5,
        	4,	3,	5,	3,	4,	5,	3,	4,	5,	4,	5,	4,
        	5,	4,	5,	4,	5,	4,	5,	5,	5,	5,	5,	5,
    	};

	LEDS_DEBUG("[LED]PMIC#%d:%d\n", pmic_type, level);
	mutex_lock(&leds_pmic_mutex);
	if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
	{
		if(backlight_init_flag == false)
		{
            upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down 

            // For backlight: Current: 24mA, PWM frequency: 20K, Duty: 20~100, Soft start: off, Phase shift: on
            // ISINK0
            upmu_set_rg_isink0_ck_pdn(0x0); // Disable power down    
            upmu_set_rg_isink0_ck_sel(0x1); // Freq = 1Mhz for Backlight
			upmu_set_isink_ch0_mode(ISINK_PWM_MODE);
            upmu_set_isink_ch0_step(0x5); // 24mA
            upmu_set_isink_sfstr0_en(0x0); // Disable soft start
			upmu_set_rg_isink0_double_en(0x1); // Enable double current
			upmu_set_isink_phase_dly_tc(0x0); // TC = 0.5us
			upmu_set_isink_phase0_dly_en(0x1); // Enable phase delay
            upmu_set_isink_chop0_en(0x1); // Enable CHOP clk
            // ISINK1
            upmu_set_rg_isink1_ck_pdn(0x0); // Disable power down   
            upmu_set_rg_isink1_ck_sel(0x1); // Freq = 1Mhz for Backlight
			upmu_set_isink_ch1_mode(ISINK_PWM_MODE);
            upmu_set_isink_ch1_step(0x5); // 24mA
            upmu_set_isink_sfstr1_en(0x0); // Disable soft start
			upmu_set_rg_isink1_double_en(0x1); // Enable double current
			upmu_set_isink_phase1_dly_en(0x1); // Enable phase delay
            upmu_set_isink_chop1_en(0x1); // Enable CHOP clk         
            // ISINK2
            upmu_set_rg_isink2_ck_pdn(0x0); // Disable power down   
            upmu_set_rg_isink2_ck_sel(0x1); // Freq = 1Mhz for Backlight
			upmu_set_isink_ch2_mode(ISINK_PWM_MODE);
            upmu_set_isink_ch2_step(0x5); // 24mA
            upmu_set_isink_sfstr2_en(0x0); // Disable soft start
			upmu_set_rg_isink2_double_en(0x1); // Enable double current
			upmu_set_isink_phase2_dly_en(0x1); // Enable phase delay
            upmu_set_isink_chop2_en(0x1); // Enable CHOP clk   
            // ISINK3
            upmu_set_rg_isink3_ck_pdn(0x0); // Disable power down   
            upmu_set_rg_isink3_ck_sel(0x1); // Freq = 1Mhz for Backlight
			upmu_set_isink_ch3_mode(ISINK_PWM_MODE);
            upmu_set_isink_ch3_step(0x5); // 24mA
            upmu_set_isink_sfstr3_en(0x0); // Disable soft start
			upmu_set_rg_isink3_double_en(0x1); // Enable double current
			upmu_set_isink_phase3_dly_en(0x1); // Enable phase delay
            upmu_set_isink_chop3_en(0x1); // Enable CHOP clk                
			backlight_init_flag = true;
		}
		
		if (level) 
		{
			level = brightness_mapping(tmp_level);
			if(level == ERROR_BL_LEVEL)
				level = 255;
		    if(level == 255)
            {
                level = 108;
            }
            else
            {
                level = ((level * 108) / 255) + 1;
            }
            LEDS_DEBUG("[LED]Level Mapping = %d \n", level);
            LEDS_DEBUG("[LED]ISINK DIM Duty = %d \n", duty_mapping[level-1]);
            LEDS_DEBUG("[LED]ISINK Current = %d \n", current_mapping[level-1]);
            upmu_set_isink_dim0_duty(duty_mapping[level-1]);
            upmu_set_isink_dim1_duty(duty_mapping[level-1]);
            upmu_set_isink_dim2_duty(duty_mapping[level-1]);
            upmu_set_isink_dim3_duty(duty_mapping[level-1]);
            upmu_set_isink_ch0_step(current_mapping[level-1]);
            upmu_set_isink_ch1_step(current_mapping[level-1]);
            upmu_set_isink_ch2_step(current_mapping[level-1]);
            upmu_set_isink_ch3_step(current_mapping[level-1]);
            upmu_set_isink_dim0_fsel(0x2); // 20Khz
            upmu_set_isink_dim1_fsel(0x2); // 20Khz
            upmu_set_isink_dim2_fsel(0x2); // 20Khz
            upmu_set_isink_dim3_fsel(0x2); // 20Khz            
            upmu_set_isink_ch0_en(0x1); // Turn on ISINK Channel 0
            upmu_set_isink_ch1_en(0x1); // Turn on ISINK Channel 1
            upmu_set_isink_ch2_en(0x1); // Turn on ISINK Channel 2
            upmu_set_isink_ch3_en(0x1); // Turn on ISINK Channel 3
			bl_duty_hal = level;
		}
		else 
		{
            upmu_set_isink_ch0_en(0x0); // Turn off ISINK Channel 0
            upmu_set_isink_ch1_en(0x0); // Turn off ISINK Channel 1
            upmu_set_isink_ch2_en(0x0); // Turn off ISINK Channel 2
            upmu_set_isink_ch3_en(0x0); // Turn off ISINK Channel 3
			bl_duty_hal = level;
		}
        mutex_unlock(&leds_pmic_mutex);
		return 0;
	}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK0)
		{
			if((button_flag_isink0==0) && (first_time == true)) {//button flag ==0, means this ISINK is not for button backlight
				if(button_flag_isink1==0)
					upmu_set_isink_ch1_en(0); 
				if(button_flag_isink2==0)
					upmu_set_isink_ch2_en(0);
				if(button_flag_isink3==0)
					upmu_set_isink_ch3_en(0);
				first_time = false;
			}

				upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down  
				upmu_set_rg_isink0_ck_pdn(0);
				upmu_set_rg_isink0_ck_sel(0);
				upmu_set_isink_ch0_mode(PMIC_PWM_0);
				upmu_set_isink_ch0_step(0x3);//16mA
				//hwPWMsetting(PMIC_PWM_1, 15, 8);
				upmu_set_isink_dim0_duty(15);
				upmu_set_isink_dim0_fsel(0);//6323 1KHz
				
			
			if (level) 
			{

				upmu_set_isink_ch0_en(0x01);
				
			}
			else 
			{
				upmu_set_isink_ch0_en(0x0);
			}
			mutex_unlock(&leds_pmic_mutex);
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK1)
		{
			if((button_flag_isink1==0) && (first_time == true)) {//button flag ==0, means this ISINK is not for button backlight
				if(button_flag_isink0==0)
					upmu_set_isink_ch0_en(0); 
				if(button_flag_isink2==0)
					upmu_set_isink_ch2_en(0);
				if(button_flag_isink3==0)
					upmu_set_isink_ch3_en(0);
				first_time = false;
			}

				upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down  
				upmu_set_rg_isink1_ck_pdn(0);
				upmu_set_rg_isink1_ck_sel(0);
				upmu_set_isink_ch1_mode(PMIC_PWM_0);
				upmu_set_isink_ch1_step(0x3);//16mA
				//hwPWMsetting(PMIC_PWM_2, 15, 8);
				upmu_set_isink_dim1_duty(15);
				upmu_set_isink_dim1_fsel(0);//6323 1KHz

			if (level) 
			{
			   #ifdef NO_NEED_USB_LED
			    LEDS_DEBUG("[LED]NO_NEED_USB_LED +++++++++++++++\n");
				if(	FACTORY_BOOT == get_boot_mode() || META_BOOT == get_boot_mode()) {
					upmu_set_isink_ch1_en(0x1);
				}else {
					upmu_set_isink_ch1_en(0x0);
				}
			   #else
				upmu_set_isink_ch1_en(0x01);
			   #endif
			}
			else 
			{
				upmu_set_isink_ch1_en(0x0);
			}
			mutex_unlock(&leds_pmic_mutex);
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK2)
		{
			if((button_flag_isink2==0) && (first_time == true)) {//button flag ==0, means this ISINK is not for button backlight
				if(button_flag_isink0==0)
					upmu_set_isink_ch0_en(0); 
				if(button_flag_isink1==0)
					upmu_set_isink_ch1_en(0);
				if(button_flag_isink3==0)
					upmu_set_isink_ch3_en(0);
				first_time = false;
			}
				upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down  
				upmu_set_rg_isink2_ck_pdn(0);
				upmu_set_rg_isink2_ck_sel(0);
				upmu_set_isink_ch2_mode(PMIC_PWM_0);
				upmu_set_isink_ch2_step(0x3);//16mA
				//hwPWMsetting(PMIC_PWM_2, 15, 8);
				upmu_set_isink_dim2_duty(15);
				upmu_set_isink_dim2_fsel(0);//6323 1KHz


			if (level) 
			{
				upmu_set_isink_ch2_en(0x01);
			}
			else 
			{
				upmu_set_isink_ch2_en(0x0);
			}
			mutex_unlock(&leds_pmic_mutex);
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK3)
		{
			if((button_flag_isink3==0) && (first_time == true)) {//button flag ==0, means this ISINK is not for button backlight
				if(button_flag_isink0==0)
					upmu_set_isink_ch0_en(0); 
				if(button_flag_isink1==0)
					upmu_set_isink_ch1_en(0);
				if(button_flag_isink2==0)
					upmu_set_isink_ch2_en(0);
				first_time = false;
			}

				upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down  
				upmu_set_rg_isink3_ck_pdn(0);
				upmu_set_rg_isink3_ck_sel(0);
				upmu_set_isink_ch3_mode(PMIC_PWM_0);
				upmu_set_isink_ch3_step(0x3);//16mA
				//hwPWMsetting(PMIC_PWM_1, 15, 8);
				upmu_set_isink_dim3_duty(15);
				upmu_set_isink_dim3_fsel(0);//6323 1KHz

			
			if (level) 
			{
				upmu_set_isink_ch3_en(0x01);
				
			}
			else 
			{
				upmu_set_isink_ch3_en(0x0);
			}
			mutex_unlock(&leds_pmic_mutex);
			return 0;
		}
	  #if 0
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK01)
		{
	
			//hwBacklightISINKTuning(0x0, 0x3, 0x0, 0);  //register mode, ch0_step=4ma ,disable CABC
			//hwBacklightISINKTurnOn(0x0);  //turn on ISINK0
	
			//if(led_init_flag[1] == false)
			{

				upmu_set_isinks_ch0_mode(PMIC_PWM_0);
				upmu_set_isinks_ch0_step(0x0);//4mA
				upmu_set_isink_dim0_duty(1);
				upmu_set_isink_dim0_fsel(1);//6320 1.5KHz
				//hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK5, PMIC_PWM_2, 0x0, 0);
				upmu_set_isinks_ch1_mode(PMIC_PWM_0);
				upmu_set_isinks_ch1_step(0x3);//4mA
				//hwPWMsetting(PMIC_PWM_2, 15, 8);
				upmu_set_isink_dim1_duty(15);
				upmu_set_isink_dim1_fsel(11);//6320 0.25KHz
				led_init_flag[0] = true;
				led_init_flag[1] = true;
			}
			if (level) 
			{
				//upmu_top2_bst_drv_ck_pdn(0x0);
				
				//upmu_set_rg_spk_div_pdn(0x01);
				//upmu_set_rg_spk_pwm_div_pdn(0x01);
				upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
				#ifdef ISINK_CHOP_CLK
				//upmu_set_isinks0_chop_en(0x1);
				//upmu_set_isinks1_chop_en(0x1);
				#endif
				upmu_set_isinks_ch0_en(0x01);
				upmu_set_isinks_ch1_en(0x01);
			}
			else 
			{
				upmu_set_isinks_ch0_en(0x00);
				upmu_set_isinks_ch1_en(0x00);
				//upmu_set_rg_bst_drv_1m_ck_pdn(0x1);
				//upmu_top2_bst_drv_ck_pdn(0x1);
			}
			return 0;
		}
	   #endif
		mutex_unlock(&leds_pmic_mutex);	
		return -1;

}

int mt_brightness_set_pmic_duty_store(u32 level, u32 div)
{
	return -1;
}
int mt_mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	struct nled_setting led_tmp_setting = {0,0,0};
	int tmp_level = level;
	static bool button_flag = false;
	unsigned int BacklightLevelSupport = Cust_GetBacklightLevelSupport_byPWM();
	//Mark out since the level is already cliped before sending in
	/*
		if (level > LED_FULL)
			level = LED_FULL;
		else if (level < 0)
			level = 0;
	*/	
    LEDS_DEBUG("mt65xx_leds_set_cust: set brightness, name:%s, mode:%d, level:%d\n", 
		cust->name, cust->mode, level);
#ifdef MTK_HDMI_SUPPORT
    if(hdmi_is_active) {
        printk("mt_mt65xx_led_set_cust hdmi active, return!\n");
        return 0;
    }
#endif		
	switch (cust->mode) {
		
		case MT65XX_LED_MODE_PWM:
			if(strcmp(cust->name,"lcd-backlight") == 0)
			{
				bl_brightness_hal = level;
				if(level == 0)
				{
					//mt_set_pwm_disable(cust->data);
					//mt_pwm_power_off (cust->data);
					mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
					
				}else
				{
					if (BacklightLevelSupport == BACKLIGHT_LEVEL_PWM_256_SUPPORT)
						level = brightness_mapping(tmp_level);
					else
						level = brightness_mapto64(tmp_level);	
					mt_backlight_set_pwm(cust->data, level, bl_div_hal,&cust->config_data);
				}
                bl_duty_hal = level;	
				
			}else
			{
				if(level == 0)
				{
					led_tmp_setting.nled_mode = NLED_OFF;
					mt_led_set_pwm(cust->data,&led_tmp_setting);
					mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
				}else
				{
					led_tmp_setting.nled_mode = NLED_ON;
					mt_led_set_pwm(cust->data,&led_tmp_setting);
				}
				
			}
			return 1;
          
		case MT65XX_LED_MODE_GPIO:
			LEDS_DEBUG("brightness_set_cust:go GPIO mode!!!!!\n");
			return ((cust_set_brightness)(cust->data))(level);
              
		case MT65XX_LED_MODE_PMIC:
			//for button baclight used SINK channel, when set button ISINK, don't do disable other ISINK channel
			if((strcmp(cust->name,"button-backlight") == 0)) {
				if(button_flag==false) {
					switch (cust->data) {
						case MT65XX_LED_PMIC_NLED_ISINK0:
							button_flag_isink0 = 1;
							break;
						case MT65XX_LED_PMIC_NLED_ISINK1:
							button_flag_isink1 = 1;
							break;
						case MT65XX_LED_PMIC_NLED_ISINK2:
							button_flag_isink2 = 1;
							break;
						case MT65XX_LED_PMIC_NLED_ISINK3:
							button_flag_isink3 = 1;
							break;
						default:
							break;
					}
					button_flag=true;
				}
			}					
			return mt_brightness_set_pmic(cust->data, level, bl_div_hal);
            
		case MT65XX_LED_MODE_CUST_LCM:
            if(strcmp(cust->name,"lcd-backlight") == 0)
			{
			    bl_brightness_hal = level;
            }
			LEDS_DEBUG("brightness_set_cust:backlight control by LCM\n");
			return ((cust_brightness_set)(cust->data))(level, bl_div_hal);

		
		case MT65XX_LED_MODE_CUST_BLS_PWM:
			if(strcmp(cust->name,"lcd-backlight") == 0)
			{
				bl_brightness_hal = level;
			}
			//LEDS_DEBUG("brightness_set_cust:backlight control by BLS_PWM!!\n");
			//#if !defined (MTK_AAL_SUPPORT)
			return ((cust_set_brightness)(cust->data))(level);
			//LEDS_DEBUG("brightness_set_cust:backlight control by BLS_PWM done!!\n");
			//#endif
            
		case MT65XX_LED_MODE_NONE:
		default:
			break;
	}
	return -1;
}

void mt_mt65xx_led_work(struct work_struct *work)
{
	struct mt65xx_led_data *led_data =
		container_of(work, struct mt65xx_led_data, work);

	LEDS_DEBUG("[LED]%s:%d\n", led_data->cust.name, led_data->level);
	mutex_lock(&leds_mutex);
	mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);
	mutex_unlock(&leds_mutex);;
}

void mt_mt65xx_led_set(struct led_classdev *led_cdev, enum led_brightness level)
{
	struct mt65xx_led_data *led_data =
		container_of(led_cdev, struct mt65xx_led_data, cdev);
	//unsigned long flags;
	//spin_lock_irqsave(&leds_lock, flags);
	
#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH
	
		if(level >> LED_RESERVEBIT_SHIFT)
		{
			if(LED_RESERVEBIT_PATTERN != (level >> LED_RESERVEBIT_SHIFT))
			{
				//sanity check for hidden code
				printk("incorrect input : %d,%d\n" , level , (level >> LED_RESERVEBIT_SHIFT));
				return;
			}
	
			if(MT65XX_LED_MODE_CUST_BLS_PWM != led_data->cust.mode)
			{
				//only BLS PWM support expand bit
				printk("Not BLS PWM %d \n" , led_data->cust.mode);
				return;
			}
	
			level &= ((1 << LED_RESERVEBIT_SHIFT) - 1);
	
			if((level +1) > (1 << MT_LED_INTERNAL_LEVEL_BIT_CNT))
			{
				//clip to max value
				level = (1 << MT_LED_INTERNAL_LEVEL_BIT_CNT) - 1;
			}
	
			led_cdev->brightness = (level >> (MT_LED_INTERNAL_LEVEL_BIT_CNT - 8));//brightness is 8 bit level
			if(led_cdev->brightness > led_cdev->max_brightness)
			{
				led_cdev->brightness = led_cdev->max_brightness;
			}
	
			if(led_data->level != level)
			{
				led_data->level = level;
				mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);
			}
		}
		else
		{
			if(led_data->level != level)
			{
				led_data->level = level;
				if(strcmp(led_data->cust.name,"lcd-backlight") != 0)
				{
					LEDS_DEBUG("[LED]Set NLED directly %d at time %lu\n",led_data->level,jiffies);
					schedule_work(&led_data->work);				
				}
			    else
				{
					LEDS_DEBUG("[LED]Set Backlight directly %d at time %lu\n",led_data->level,jiffies);
					if(MT65XX_LED_MODE_CUST_BLS_PWM == led_data->cust.mode)
					{
						mt_mt65xx_led_set_cust(&led_data->cust, ((((1 << MT_LED_INTERNAL_LEVEL_BIT_CNT) - 1)*level + 127)/255));
						//mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);	
					}
					else
					{
						mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);	
					}	
				}
			}
		}						
#else
	// do something only when level is changed
	if (led_data->level != level) {
		led_data->level = level;
		if(strcmp(led_data->cust.name,"lcd-backlight"))
		{
				schedule_work(&led_data->work);
		}else
		{
				LEDS_DEBUG("[LED]Set Backlight directly %d at time %lu\n",led_data->level,jiffies);
				mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);	
		}
	}
	//spin_unlock_irqrestore(&leds_lock, flags);
#endif
}

int  mt_mt65xx_blink_set(struct led_classdev *led_cdev,
			     unsigned long *delay_on,
			     unsigned long *delay_off)
{
	struct mt65xx_led_data *led_data =
		container_of(led_cdev, struct mt65xx_led_data, cdev);
	static int got_wake_lock = 0;
	struct nled_setting nled_tmp_setting = {0,0,0};

	// only allow software blink when delay_on or delay_off changed
	if (*delay_on != led_data->delay_on || *delay_off != led_data->delay_off) {
		led_data->delay_on = *delay_on;
		led_data->delay_off = *delay_off;
		if (led_data->delay_on && led_data->delay_off) { // enable blink
			led_data->level = 255; // when enable blink  then to set the level  (255)
			//if(led_data->cust.mode == MT65XX_LED_MODE_PWM && 
			//(led_data->cust.data != PWM3 && led_data->cust.data != PWM4 && led_data->cust.data != PWM5))

			//AP PWM all support OLD mode in MT6589
			if(led_data->cust.mode == MT65XX_LED_MODE_PWM)
			{
				nled_tmp_setting.nled_mode = NLED_BLINK;
				nled_tmp_setting.blink_off_time = led_data->delay_off;
				nled_tmp_setting.blink_on_time = led_data->delay_on;
				mt_led_set_pwm(led_data->cust.data,&nled_tmp_setting);
				return 0;
			}
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0
				|| led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK1 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK2 
				|| led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK3))
			{
				//if(get_chip_eco_ver() == CHIP_E2) {
				if(1){
					nled_tmp_setting.nled_mode = NLED_BLINK;
					nled_tmp_setting.blink_off_time = led_data->delay_off;
					nled_tmp_setting.blink_on_time = led_data->delay_on;
					mt_led_blink_pmic(led_data->cust.data, &nled_tmp_setting);
					return 0;
				} else {
					wake_lock(&leds_suspend_lock);
				}
			}				
			else if (!got_wake_lock) {
				wake_lock(&leds_suspend_lock);
				got_wake_lock = 1;
			}
		}
		else if (!led_data->delay_on && !led_data->delay_off) { // disable blink
			//if(led_data->cust.mode == MT65XX_LED_MODE_PWM && 
			//(led_data->cust.data != PWM3 && led_data->cust.data != PWM4 && led_data->cust.data != PWM5))

			//AP PWM all support OLD mode in MT6589
			
			if(led_data->cust.mode == MT65XX_LED_MODE_PWM)
			{
				nled_tmp_setting.nled_mode = NLED_OFF;
				mt_led_set_pwm(led_data->cust.data,&nled_tmp_setting);
				return 0;
			}
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0
				|| led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK1 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK2 
				|| led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK3))
			{
				//if(get_chip_eco_ver() == CHIP_E2) {
				if(1){
					mt_brightness_set_pmic(led_data->cust.data, 0, 0);
					return 0;
				} else {
					wake_unlock(&leds_suspend_lock);
				}
			}		
			else if (got_wake_lock) {
				wake_unlock(&leds_suspend_lock);
				got_wake_lock = 0;
			}
		}
		return -1;
	}

	// delay_on and delay_off are not changed
	return 0;
}


