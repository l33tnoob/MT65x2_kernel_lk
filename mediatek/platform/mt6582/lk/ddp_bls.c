#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>
#include <debug.h>
#include <string.h>
#include <cust_leds.h>

#ifndef CLK_CFG_1
#define CLK_CFG_1 0x10000050
#endif

#define POLLING_TIME_OUT 10000

#define PWM_DEFAULT_DIV_VALUE 0x0

#if !defined(MTK_AAL_SUPPORT) 
static int gBLSMutexID = 3;
#endif

static int gPWMDiv = PWM_DEFAULT_DIV_VALUE;

static unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;

    // PWM duty input =  PWM_DUTY_IN / 1024
    mapped_level = level * 1023 / 255;

    if (mapped_level > 0x3FF)
        mapped_level = 0x3FF;

	return mapped_level;
}


int disp_poll_for_reg(unsigned int addr, unsigned int value, unsigned int mask, unsigned int timeout)
{
    unsigned int cnt = 0;
    
    while ((DISP_REG_GET(addr) & mask) != value)
    {
        cnt++;
        if (cnt > timeout)
        {
            return -1;
        }
    }

    return 0;
}

static int disp_bls_get_mutex()
{
#if !defined(MTK_AAL_SUPPORT)    
    if (gBLSMutexID < 0)
        return -1;

    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 1);
    if(disp_poll_for_reg(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0x2, 0x2, POLLING_TIME_OUT))
    {
        printf("[DDP] error! disp_bls_get_mutex(), get mutex timeout! \n");
        disp_dump_reg(DISP_MODULE_CONFIG);        
        return -1;
    }
#endif    
    return 0;
}

static int disp_bls_release_mutex()
{
#if !defined(MTK_AAL_SUPPORT)    
    if (gBLSMutexID < 0)
        return -1;
    
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0);
    if(disp_poll_for_reg(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0, 0x2, POLLING_TIME_OUT))
    {
        printf("[DDP] error! disp_bls_release_mutex(), release mutex timeout! \n");
        disp_dump_reg(DISP_MODULE_CONFIG);
        return -1;
    }
#endif    
    return 0;
}

void disp_bls_init(unsigned int srcWidth, unsigned int srcHeight)
{
    struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
    struct cust_mt65xx_led *cust = NULL;
    struct PWM_config *config_data = NULL;

    if(cust_led_list)
    {
        cust = &cust_led_list[MT65XX_LED_TYPE_LCD];
        if((strcmp(cust->name,"lcd-backlight") == 0) && (cust->mode == MT65XX_LED_MODE_CUST_BLS_PWM))
        {
            config_data = &cust->config_data;
            if (config_data->clock_source >= 0 && config_data->clock_source <= 3)
            {
            	unsigned int regVal = DISP_REG_GET(CLK_CFG_1);
                DISP_REG_SET(CLK_CFG_1, (regVal & ~0x3) | config_data->clock_source);
                printf("disp_bls_init : CLK_CFG_1 0x%x => 0x%x\n", regVal, DISP_REG_GET(CLK_CFG_1));
            }
            gPWMDiv = (config_data->div == 0) ? PWM_DEFAULT_DIV_VALUE : config_data->div;
            gPWMDiv &= 0x3FF;
            printf("disp_bls_init : PWM config data (%d,%d)\n", config_data->clock_source, config_data->div);
        }
    }
    
    printf("[DDP] disp_bls_init : srcWidth = %d, srcHeight = %d\n", srcWidth, srcHeight);
    printf("[DDP] disp_bls_init : CG = 0x%x, BLS_EN = 0x%x, PWM_DUTY = %d\n", 
        DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0), 
        DISP_REG_GET(DISP_REG_BLS_EN),
        DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
    
    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE, (srcHeight << 16) | srcWidth);
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, 0);
    DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x0 | (gPWMDiv << 16));
    DISP_REG_SET(DISP_REG_BLS_EN, 0x00010000);
}

int disp_bls_config(void)
{
#if !defined(MTK_AAL_SUPPORT) 
    struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
    struct cust_mt65xx_led *cust = NULL;
    struct PWM_config *config_data = NULL;

    if(cust_led_list)
    {
        cust = &cust_led_list[MT65XX_LED_TYPE_LCD];
        if((strcmp(cust->name,"lcd-backlight") == 0) && (cust->mode == MT65XX_LED_MODE_CUST_BLS_PWM))
        {
            config_data = &cust->config_data;
            if (config_data->clock_source >= 0 && config_data->clock_source <= 3)
            {
		unsigned int regVal = DISP_REG_GET(CLK_CFG_1);
                DISP_REG_SET(CLK_CFG_1, (regVal & ~0x3) | config_data->clock_source);
                printf("disp_bls_config : CLK_CFG_1 0x%x => 0x%x\n", regVal, DISP_REG_GET(CLK_CFG_1));
            }
            gPWMDiv = (config_data->div == 0) ? PWM_DEFAULT_DIV_VALUE : config_data->div;
            gPWMDiv &= 0x3FF;
            printf("disp_bls_config : PWM config data (%d,%d)\n", config_data->clock_source, config_data->div);
        }
    }
    
    printf("[DDP] disp_bls_config : CG = 0x%x, BLS_EN = 0x%x, PWM_DUTY = %d\n", 
        DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0), 
        DISP_REG_GET(DISP_REG_BLS_EN),
        DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
#ifdef USE_DISP_BLS_MUTEX
    printf("[DDP] disp_bls_config : gBLSMutexID = %d\n", gBLSMutexID);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 1);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 0);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gBLSMutexID), 0x200);    // BLS
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gBLSMutexID), 0);        // single mode
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gBLSMutexID), 1);

    if (disp_bls_get_mutex() == 0)
    {
#else
        DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x3);
#endif

        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, 0);
        DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x0 | (gPWMDiv << 16));
        DISP_REG_SET(DISP_REG_BLS_EN, 0x00010000);

#ifdef USE_DISP_BLS_MUTEX
        if (disp_bls_release_mutex() == 0)
            return 0;
    }
    return -1;
#else
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x0);
#endif

#endif
    return 0;
}

int disp_bls_set_backlight(unsigned int level)
{
    printf("[DDP] disp_bls_set_backlight: %d, CG = 0x%x, BLS_EN = 0x%x, PWM_DUTY = %d\n", 
        level,
        DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0), 
        DISP_REG_GET(DISP_REG_BLS_EN),
        DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
    
    if (level && (!(DISP_REG_GET(DISP_REG_BLS_EN) & 0x10000)))
    {
        disp_bls_config();
    }

#ifdef USE_DISP_BLS_MUTEX
    disp_bls_get_mutex();
#else
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x3);
#endif

    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, brightness_mapping(level));
    printf("[DDP] PWM_DUTY: %x\n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));

#ifdef USE_DISP_BLS_MUTEX
    disp_bls_release_mutex();
#else
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x0);
#endif

    return 0;    
}
