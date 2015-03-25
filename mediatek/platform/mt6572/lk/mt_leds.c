/******************************************************************************
 * mt65xx_leds.c
 * 
 * Copyright 2010 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *
 ******************************************************************************/

//#include <common.h>
//#include <platform/mt.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_typedefs.h>


// FIXME: should include power related APIs

#include <platform/mt_pwm.h>
#include <platform/mt_gpio.h>
#include <platform/mt_leds.h>
//#include <asm/io.h>

#include <platform/mt_pmic.h> 


//extern void mt_power_off (U32 pwm_no);
//extern S32 mt_set_pwm_disable ( U32 pwm_no );
extern void mt_pwm_disable(U32 pwm_no, BOOL pmic_pad);
extern int strcmp(const char *cs, const char *ct);

/****************************************************************************
 * DEBUG MACROS
 ***************************************************************************/
int debug_enable = 0;
#define LEDS_DEBUG(format, args...) do{ \
		if(debug_enable) \
		{\
			printf(format,##args);\
		}\
	}while(0)
#define LEDS_INFO LEDS_DEBUG 	
/****************************************************************************
 * structures
 ***************************************************************************/
static int g_lastlevel[MT65XX_LED_TYPE_TOTAL] = {-1, -1, -1, -1, -1, -1, -1};
int backlight_PWM_div = CLK_DIV1;
/****************************************************************************
 * function prototypes
 ***************************************************************************/

/* import functions */
// FIXME: should extern from pmu driver
//void pmic_backlight_on(void) {}
//void pmic_backlight_off(void) {}
//void pmic_config_interface(kal_uint16 RegNum, kal_uint8 val, kal_uint16 MASK, kal_uint16 SHIFT) {}

/* internal functions */
static int brightness_set_pwm(int pwm_num, enum led_brightness level,struct PWM_config *config_data);
static int led_set_pwm(int pwm_num, enum led_brightness level);
static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, enum led_brightness level);
//static int brightness_set_gpio(int gpio_num, enum led_brightness level);
static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level);

/****************************************************************************
 * global variables
 ***************************************************************************/
static unsigned int limit = 255;

/****************************************************************************
 * internal functions
 ***************************************************************************/

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

unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;
    
    mapped_level = level;
       
	return mapped_level;
}

static int brightness_set_pwm(int pwm_num, enum led_brightness level,struct PWM_config *config_data)
{
	struct pwm_spec_config pwm_setting;

	pwm_setting.pwm_no = pwm_num;
	pwm_setting.mode = PWM_MODE_FIFO; // New mode fifo and periodical mode

	pwm_setting.pmic_pad = config_data->pmic_pad;

	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div = config_data->div;
	}
	else
	{
		pwm_setting.clk_div = CLK_DIV1;
	}

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
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION = pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION;
	}
	else
	{
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION = 4;
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION = 4;
	}

	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 31;
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GDURATION = (pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION + 1) * 32 - 1;
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;

	printf("[LEDS]LK: backlight_set_pwm:duty is %d\n", level);
	printf("[LEDS]LK: backlight_set_pwm:clk_src/div/high/low is %d%d%d%d\n", pwm_setting.clk_src, pwm_setting.clk_div, pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION, pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION);

	if(level > 0 && level <= 32)
	{
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1;
		pwm_set_spec_config(&pwm_setting);
	}else if(level > 32 && level <= 64)
	{
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GUARD_VALUE = 1;
		level -= 32;
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1 ;
		pwm_set_spec_config(&pwm_setting);
	}else
	{
		printf("[LEDS]Error level in backlight\n");
		mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
	}

	return 0;
}

static int led_set_pwm(int pwm_num, enum led_brightness level)
{
	struct pwm_spec_config pwm_setting;
	pwm_setting.pwm_no = pwm_num;
	pwm_setting.clk_div = CLK_DIV1; 		

	pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.DATA_WIDTH = 10;
    
	// Only old PWM mode with 32kHz clock source can work in the system sleep mode
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
    
	if(level)
	{
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.THRESH = 30;
	}else
	{
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.THRESH = 0;
	}
	printf("[LEDS]LK: brightness_set_pwm: level=%d, clk=%d \n\r", level, pwm_setting.clk_src);

	pwm_set_spec_config(&pwm_setting);

	return 0;
}

static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, enum led_brightness level)
{
#define PMIC_BACKLIGHT_LEVEL    80

	int tmp_level = level;
	//static bool backlight_init_flag = false;
	static bool first_time = true;
        static unsigned char duty_mapping[PMIC_BACKLIGHT_LEVEL] = {
                0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	
                12,     13,     14,     15,     16,     17,     18,     19,     20,     21,     22,     23,     
                24,     25,     26,     27,     28,     29,     30,     31,     16,     17,     18,     19,     
                20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     
                21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     24,     
                25,     26,     27,     28,     29,     30,     31,     25,     26,     27,     28,     29,     
                30,     31,     26,     27,     28,     29,     30,     31,

    };
        static unsigned char current_mapping[PMIC_BACKLIGHT_LEVEL] = {
                0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
                0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      
                0,      0,      0,      0,      0,      0,      0,      0,      1,      1,      1,      1,      
                1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      
                2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      3,      
                3,      3,      3,      3,      3,      3,      3,      4,      4,      4,      4,      4,      
                4,      4,      5,      5,      5,      5,      5,      5,
    };

	printf("[LEDS]LK: PMIC Type: %d, Level: %d\n", pmic_type, level);

	if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
	{
		//if(backlight_init_flag == false)
		{
            upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down
            upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down (backlight no need?)    

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
                        //backlight_init_flag = true;
		}
		
		if (level) 
		{
			level = brightness_mapping(tmp_level);
			if(level == ERROR_BL_LEVEL)
				level = limit;
#if 0            
            if(((level << 5) / limit) < 1)
            {
                level = 0;
            }
            else
            {
                level = ((level << 5) / limit) - 1;
            }
#endif      

            if(level == limit)
            {
				level = PMIC_BACKLIGHT_LEVEL;
            }
            else
            {
				level = ((level * PMIC_BACKLIGHT_LEVEL) / 255) + 1;
            }
            printf("[LEDS]LK: Level Mapping = %d \n", level);
            printf("[LEDS]LK: ISINK DIM Duty = %d \n", duty_mapping[level-1]);
            printf("[LEDS]LK: ISINK Current = %d \n", current_mapping[level-1]);
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
		}
		else 
		{
            upmu_set_isink_ch0_en(0x0); // Turn off ISINK Channel 0
            upmu_set_isink_ch1_en(0x0); // Turn off ISINK Channel 1
            upmu_set_isink_ch2_en(0x0); // Turn off ISINK Channel 2
            upmu_set_isink_ch3_en(0x0); // Turn off ISINK Channel 3
		}
        
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK0)
	{
		if(first_time == true)
		{
            upmu_set_isink_ch0_en(0x0); // Turn off ISINK Channel 0
			first_time = false;
		}

            upmu_set_rg_isink0_ck_pdn(0x0); // Disable power down    
            upmu_set_rg_isink0_ck_sel(0x0); // Freq = 32KHz for Indicator            
            upmu_set_isink_dim0_duty(15); // 16 / 32, no use for register mode
			upmu_set_isink_ch0_mode(ISINK_REGISTER_MODE);
            upmu_set_isink_dim0_fsel(0x0); // 1KHz, no use for register mode
            upmu_set_isink_ch0_step(0x0); // 4mA
            upmu_set_isink_sfstr0_tc(0x0); // 0.5us
            upmu_set_isink_sfstr0_en(0x0); // Disable soft start
			upmu_set_rg_isink0_double_en(0x0); // Disable double current
			upmu_set_isink_phase0_dly_en(0x0); // Disable phase delay
            upmu_set_isink_chop0_en(0x0); // Disable CHOP clk
		
		if (level) 
		{
            upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down (indicator no need?)     
            upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down            
            upmu_set_isink_ch0_en(0x1); // Turn on ISINK Channel 0
			
		}
		else 
		{
            upmu_set_isink_ch0_en(0x0); // Turn off ISINK Channel 0
		}
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK1)
	{
		if(first_time == true)
		{
            upmu_set_isink_ch1_en(0x0); // Turn off ISINK Channel 1
			first_time = false;
		}

            upmu_set_rg_isink1_ck_pdn(0x0); // Disable power down    
            upmu_set_rg_isink1_ck_sel(0x0); // Freq = 32KHz for Indicator            
            upmu_set_isink_dim1_duty(15); // 16 / 32, no use for register mode
			upmu_set_isink_ch1_mode(ISINK_REGISTER_MODE);
            upmu_set_isink_dim1_fsel(0x0); // 1KHz, no use for register mode
            upmu_set_isink_ch1_step(0x0); // 4mA
            upmu_set_isink_sfstr1_tc(0x0); // 0.5us
            upmu_set_isink_sfstr1_en(0x0); // Disable soft start
			upmu_set_rg_isink1_double_en(0x0); // Disable double current
			upmu_set_isink_phase1_dly_en(0x0); // Disable phase delay
            upmu_set_isink_chop1_en(0x0); // Disable CHOP clk

		
		if (level) 
		{
            upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down (indicator no need?)     
            upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down            
            upmu_set_isink_ch1_en(0x1); // Turn on ISINK Channel 1
			
		}
		else 
		{
            upmu_set_isink_ch1_en(0x0); // Turn off ISINK Channel 1
		}
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK2)
	{
		if(first_time == true)
		{
            upmu_set_isink_ch2_en(0x0); // Turn off ISINK Channel 2
			first_time = false;
		}

            upmu_set_rg_isink2_ck_pdn(0x0); // Disable power down    
            upmu_set_rg_isink2_ck_sel(0x0); // Freq = 32KHz for Indicator            
            upmu_set_isink_dim2_duty(15); // 16 / 32, no use for register mode
			upmu_set_isink_ch2_mode(ISINK_REGISTER_MODE);
            upmu_set_isink_dim2_fsel(0x0); // 1KHz, no use for register mode
            upmu_set_isink_ch2_step(0x0); // 4mA
            upmu_set_isink_sfstr2_tc(0x0); // 0.5us
            upmu_set_isink_sfstr2_en(0x0); // Disable soft start
			upmu_set_rg_isink2_double_en(0x0); // Disable double current
			upmu_set_isink_phase2_dly_en(0x0); // Disable phase delay
            upmu_set_isink_chop2_en(0x0); // Disable CHOP clk

		
		if (level) 
		{
            upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down (indicator no need?)     
            upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down            
            upmu_set_isink_ch2_en(0x1); // Turn on ISINK Channel 2
			
		}
		else 
		{
            upmu_set_isink_ch2_en(0x0); // Turn off ISINK Channel 2
		}
		return 0;
	}
    else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK3)
	{
		if(first_time == true)
		{
            upmu_set_isink_ch3_en(0x0); // Turn off ISINK Channel 3
			first_time = false;
		}

            upmu_set_rg_isink3_ck_pdn(0x0); // Disable power down    
            upmu_set_rg_isink3_ck_sel(0x0); // Freq = 32KHz for Indicator            
            upmu_set_isink_dim3_duty(15); // 16 / 32, no use for register mode
			upmu_set_isink_ch3_mode(ISINK_REGISTER_MODE);
            upmu_set_isink_dim3_fsel(0x0); // 1KHz, no use for register mode
            upmu_set_isink_ch3_step(0x0); // 4mA
            upmu_set_isink_sfstr3_tc(0x0); // 0.5us
            upmu_set_isink_sfstr3_en(0x0); // Disable soft start
			upmu_set_rg_isink3_double_en(0x0); // Disable double current
			upmu_set_isink_phase3_dly_en(0x0); // Disable phase delay
            upmu_set_isink_chop3_en(0x0); // Disable CHOP clk
		
		if (level) 
		{
            upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down (indicator no need?)     
            upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down            
            upmu_set_isink_ch3_en(0x1); // Turn on ISINK Channel 3
			
		}
		else 
		{
            upmu_set_isink_ch3_en(0x0); // Turn off ISINK Channel 3
		}
		return 0;
	}
	return -1;
}

#if 0
static int brightness_set_gpio(int gpio_num, enum led_brightness level)
{
//	LEDS_INFO("LED GPIO#%d:%d\n", gpio_num, level);
	mt_set_gpio_mode(gpio_num, GPIO_MODE_00);// GPIO MODE
	mt_set_gpio_dir(gpio_num, GPIO_DIR_OUT);

	if (level)
		mt_set_gpio_out(gpio_num, GPIO_OUT_ONE);
	else
		mt_set_gpio_out(gpio_num, GPIO_OUT_ZERO);

	return 0;
}
#endif

static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;

//	printf("[LEDS]LK: mt65xx_led_set_cust: mode=%d, level=%d \n\r", cust->mode, level);

	switch (cust->mode) {
		
		case MT65XX_LED_MODE_PWM:
			if(strcmp(cust->name, "lcd-backlight") == 0)
			{

				if(level == 0)
				{
					//printf("[LEDS]LK: mt65xx_leds_set_cust: enter mt_pwm_disable()\n");

					mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
					return 1;
				}
				else
				{
					level = brightness_mapto64(level);

					//printf("[LEDS]LK: mt65xx_led_set_cust: mode=%d, level=%d \n\r", cust->mode, level);

			    return brightness_set_pwm(cust->data, level, &cust->config_data);
			}
			}
			else
			{
				return led_set_pwm(cust->data, level);
			}
		
		case MT65XX_LED_MODE_GPIO:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_PMIC:
			return brightness_set_pmic(cust->data, level);
		case MT65XX_LED_MODE_CUST_LCM:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_CUST_BLS_PWM:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_NONE:
		default:
			break;
	}
	return -1;
}

/****************************************************************************
 * external functions
 ***************************************************************************/
int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level)
{
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();

	if (type >= MT65XX_LED_TYPE_TOTAL)
		return -1;

	if (level > LED_FULL)
		level = LED_FULL;
//	else if (level < 0)  //level cannot < 0
//		level = 0;

	if (g_lastlevel[type] != (int)level) {
		g_lastlevel[type] = level;
		printf("[LEDS]LK: %s level is %d \n\r", cust_led_list[type].name, level);
		return mt65xx_led_set_cust(&cust_led_list[type], level);
	}
	else {
		return -1;
	}

}

void leds_battery_full_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_battery_low_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_battery_medium_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_init(void)
{
	printf("[LEDS]LK: leds_init: mt65xx_backlight_off \n\r");
	mt65xx_backlight_off();
}

void isink0_init(void)
{
    /*
    printf("[LEDS]LK: isink_init: turn on PMIC6323 isink \n\r");
    upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down
    upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down (backlight no need?)    

    // For backlight: Current: 24mA, PWM frequency: 20K, Duty: 20~100, Soft start: off, Phase shift: on
    // ISINK0
    upmu_set_rg_isink0_ck_pdn(0x0); // Disable power down    
    upmu_set_rg_isink0_ck_sel(0x1); // Freq = 1Mhz for Backlight
	upmu_set_isink_ch0_mode(ISINK_REGISTER_MODE);
    upmu_set_isink_ch0_step(0x5); // 24mA
    upmu_set_isink_sfstr0_en(0x0); // Disable soft start
	upmu_set_rg_isink0_double_en(0x0); // Disable double current
	upmu_set_isink_phase0_dly_en(0x1); // Enable phase delay
    upmu_set_isink_chop0_en(0x0); // Disable CHOP clk          
    upmu_set_isink_ch0_en(0x1); // Turn on ISINK Channel 0
    */
}

void leds_deinit(void)
{
    printf("[LEDS]LK: leds_deinit: LEDS off \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void mt65xx_backlight_on(void)
{
	printf("[LEDS]LK: mt65xx_backlight_on \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_FULL);
}

void mt65xx_backlight_off(void)
{
	printf("[LEDS]LK: mt65xx_backlight_off \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_OFF);
}

