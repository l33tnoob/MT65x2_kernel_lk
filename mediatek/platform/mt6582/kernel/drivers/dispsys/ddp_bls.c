#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/xlog.h>
#include <linux/mutex.h>
#include <mach/mt_clkmgr.h>

#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_debug.h"
#include "ddp_bls.h"
#include "disp_drv.h"
#include "ddp_hal.h"

#include <cust_leds.h>
#include <cust_leds_def.h>

#define DDP_GAMMA_SUPPORT

#define POLLING_TIME_OUT 1000

#define PWM_DEFAULT_DIV_VALUE 0x0

unsigned int bls_dbg_log = 0;
#define BLS_DBG(string, args...) if(bls_dbg_log) printk("[BLS]"string,##args)  // default off, use "adb shell "echo dbg_log:1 > sys/kernel/debug/dispsys" to enable
#define BLS_MSG(string, args...) printk("[BLS]"string,##args)  // default on, important msg, not err
#define BLS_ERR(string, args...) printk("[BLS]error:"string,##args)  //default on, err msg

#if !defined(MTK_AAL_SUPPORT)
#ifdef USE_DISP_BLS_MUTEX
static int gBLSMutexID = 3;
#endif
static int gBLSPowerOn = 0;
#endif
static int gMaxLevel = 1023;
static int gPWMDiv = PWM_DEFAULT_DIV_VALUE;

static DEFINE_MUTEX(backlight_mutex);

static DISPLAY_PWM_T g_pwm_lut;
static DISPLAY_GAMMA_T g_gamma_lut;
static DISPLAY_GAMMA_T g_gamma_index = 
{
entry:
{
    {
        0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,  68,  72,  76,  80,  84,  88,  92,  96,
        100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196,
        200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296,
        300, 304, 308, 312, 316, 320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380, 384, 388, 392, 396,
        400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496,
        500, 504, 508, 512, 516, 520, 524, 528, 532, 536, 540, 544, 548, 552, 556, 560, 564, 568, 572, 576, 580, 584, 588, 592, 596,
        600, 604, 608, 612, 616, 620, 624, 628, 632, 636, 640, 644, 648, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688, 692, 696,
        700, 704, 708, 712, 716, 720, 724, 728, 732, 736, 740, 744, 748, 752, 756, 760, 764, 768, 772, 776, 780, 784, 788, 792, 796,
        800, 804, 808, 812, 816, 820, 824, 828, 832, 836, 840, 844, 848, 852, 856, 860, 864, 868, 872, 876, 880, 884, 888, 892, 896,
        900, 904, 908, 912, 916, 920, 924, 928, 932, 936, 940, 944, 948, 952, 956, 960, 964, 968, 972, 976, 980, 984, 988, 992, 996,
        1000, 1004, 1008, 1012, 1016, 1020, 1023
    },
    {
        0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,  68,  72,  76,  80,  84,  88,  92,  96,
        100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196,
        200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296,
        300, 304, 308, 312, 316, 320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380, 384, 388, 392, 396,
        400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496,
        500, 504, 508, 512, 516, 520, 524, 528, 532, 536, 540, 544, 548, 552, 556, 560, 564, 568, 572, 576, 580, 584, 588, 592, 596,
        600, 604, 608, 612, 616, 620, 624, 628, 632, 636, 640, 644, 648, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688, 692, 696,
        700, 704, 708, 712, 716, 720, 724, 728, 732, 736, 740, 744, 748, 752, 756, 760, 764, 768, 772, 776, 780, 784, 788, 792, 796,
        800, 804, 808, 812, 816, 820, 824, 828, 832, 836, 840, 844, 848, 852, 856, 860, 864, 868, 872, 876, 880, 884, 888, 892, 896,
        900, 904, 908, 912, 916, 920, 924, 928, 932, 936, 940, 944, 948, 952, 956, 960, 964, 968, 972, 976, 980, 984, 988, 992, 996,
        1000, 1004, 1008, 1012, 1016, 1020, 1023
    },
    {
        0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,  68,  72,  76,  80,  84,  88,  92,  96,
        100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196,
        200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296,
        300, 304, 308, 312, 316, 320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380, 384, 388, 392, 396,
        400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496,
        500, 504, 508, 512, 516, 520, 524, 528, 532, 536, 540, 544, 548, 552, 556, 560, 564, 568, 572, 576, 580, 584, 588, 592, 596,
        600, 604, 608, 612, 616, 620, 624, 628, 632, 636, 640, 644, 648, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688, 692, 696,
        700, 704, 708, 712, 716, 720, 724, 728, 732, 736, 740, 744, 748, 752, 756, 760, 764, 768, 772, 776, 780, 784, 788, 792, 796,
        800, 804, 808, 812, 816, 820, 824, 828, 832, 836, 840, 844, 848, 852, 856, 860, 864, 868, 872, 876, 880, 884, 888, 892, 896,
        900, 904, 908, 912, 916, 920, 924, 928, 932, 936, 940, 944, 948, 952, 956, 960, 964, 968, 972, 976, 980, 984, 988, 992, 996,
        1000, 1004, 1008, 1012, 1016, 1020, 1023
    }
}
};

DISPLAY_GAMMA_T * get_gamma_index(void)
{
    BLS_DBG("get_gamma_index!\n");
    return &g_gamma_index;
}

DISPLAY_PWM_T * get_pwm_lut(void)
{
    BLS_DBG("get_pwm_lut!\n");
    return &g_pwm_lut;
}

static unsigned int brightness_mapping(unsigned int level);
void disp_onConfig_bls(DISP_AAL_PARAM *param)
{
    unsigned long prevSetting = DISP_REG_GET(DISP_REG_BLS_BLS_SETTING);
    unsigned int regVal;
    unsigned int level = brightness_mapping(param->pwmDuty);
    
    BLS_DBG("disp_onConfig_bls!\n");

    BLS_DBG("pwm duty = %lu => %u \n", param->pwmDuty, level);
    regVal = DISP_REG_GET(DISP_REG_BLS_EN);
    if (level == 0)
    {
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, 0);
        if (regVal & 0x10000)
            DISP_REG_SET(DISP_REG_BLS_EN, regVal & 0xFFFEFFFF); 
    }
    else
    {
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, level);
        if (!(regVal & 0x10000))
            DISP_REG_SET(DISP_REG_BLS_EN, regVal | 0x10000);
    }


    BLS_DBG("bls setting = %lu\n", param->setting);

    if (param->setting & ENUM_FUNC_BLS)
    {
        BLS_DBG("distion threshold = %lu\n", param->maxClrDistThd);
        BLS_DBG("predistion threshold = %lu\n", param->preDistThd);
        // TODO: BLS porting
    }

    if (prevSetting & 0x10100) 
    {
        // TODO: BLS porting
    }
    else if (param->setting & ENUM_FUNC_BLS)
    {
        disp_set_aal_alarm(1);
    }

}


static unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;

    mapped_level = level;

    if (mapped_level > gMaxLevel)
        mapped_level = gMaxLevel;

	return mapped_level;
}

#if !defined(MTK_AAL_SUPPORT)
#ifdef USE_DISP_BLS_MUTEX
static int disp_poll_for_reg(unsigned int addr, unsigned int value, unsigned int mask, unsigned int timeout)
{
    unsigned int cnt = 0;
    
    while ((DISP_REG_GET(addr) & mask) != value)
    {
        msleep(1);
        cnt++;
        if (cnt > timeout)
        {
            return -1;
        }
    }

    return 0;
}


static int disp_bls_get_mutex(void)
{
    if (gBLSMutexID < 0)
        return -1;

    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 1);
    if(disp_poll_for_reg(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0x2, 0x2, POLLING_TIME_OUT))
    {
        BLS_ERR("get mutex timeout! \n");
        disp_dump_reg(DISP_MODULE_CONFIG);
        return -1;
    }
    return 0;
}

static int disp_bls_release_mutex(void)
{ 
    if (gBLSMutexID < 0)
        return -1;
    
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0);
    if(disp_poll_for_reg(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0, 0x2, POLLING_TIME_OUT))
    {
        BLS_ERR("release mutex timeout! \n");
        disp_dump_reg(DISP_MODULE_CONFIG);
        return -1;
    }
    return 0;
}
#endif
#endif

void disp_bls_update_gamma_lut(void)
{
#if defined(DDP_GAMMA_SUPPORT)    
    int index, i;
    unsigned long CurVal, Count;

    BLS_MSG("disp_bls_update_gamma_lut!\n");

    if (DISP_REG_GET(DISP_REG_BLS_EN) & 0x1)
    {
        BLS_ERR("try to update gamma lut while BLS is active\n");
        return;
    }

    // init gamma table
    for(index = 0; index < 3; index++)
    {    
        for(Count = 0; Count < 257 ; Count++)
        {  
             g_gamma_lut.entry[index][Count] = g_gamma_index.entry[index][Count];
        }
    }

    DISP_REG_SET(DISP_REG_BLS_LUT_UPDATE, 0x1);
        
    for (i = 0; i < 256 ; i++)
    {
        CurVal = (((g_gamma_lut.entry[0][i]&0x3FF)<<20) | ((g_gamma_lut.entry[1][i]&0x3FF)<<10) | (g_gamma_lut.entry[2][i]&0x3FF));
        DISP_REG_SET(DISP_REG_BLS_GAMMA_LUT(i), CurVal);
        BLS_DBG("[%d] GAMMA LUT = 0x%x, (%lu, %lu, %lu)\n", i, DISP_REG_GET(DISP_REG_BLS_GAMMA_LUT(i)), 
            g_gamma_lut.entry[0][i], g_gamma_lut.entry[1][i], g_gamma_lut.entry[2][i]);
    }
    
    /* Set Gamma Last point*/    
    DISP_REG_SET(DISP_REG_BLS_GAMMA_SETTING, 0x00000001);
    
    // set gamma last index
    CurVal = (((g_gamma_lut.entry[0][256]&0x3FF)<<20) | ((g_gamma_lut.entry[1][256]&0x3FF)<<10) | (g_gamma_lut.entry[2][256]&0x3FF));
    DISP_REG_SET(DISP_REG_BLS_GAMMA_BOUNDARY, CurVal);
        
    DISP_REG_SET(DISP_REG_BLS_LUT_UPDATE, 0);
#endif    
}

void disp_bls_update_pwm_lut(void)
{
    int i;
    unsigned int regValue;

    BLS_MSG("disp_bls_update_pwm_lut!\n");

    regValue = DISP_REG_GET(DISP_REG_BLS_EN);
    if (regValue & 0x1) {
        BLS_ERR("update PWM LUT while BLS func enabled!\n");
        disp_dump_reg(DISP_MODULE_BLS);
    }
    //DISP_REG_SET(DISP_REG_BLS_EN, (regValue & 0x00010000));

    for (i = 0; i < PWM_LUT_ENTRY; i++)
    {
        DISP_REG_SET(DISP_REG_BLS_LUMINANCE(i), g_pwm_lut.entry[i]);
        BLS_DBG("[%d] PWM LUT = 0x%x (%lu)\n", i, DISP_REG_GET(DISP_REG_BLS_LUMINANCE(i)), g_pwm_lut.entry[i]);

    }
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE_255, g_pwm_lut.entry[PWM_LUT_ENTRY-1]);
    //DISP_REG_SET(DISP_REG_BLS_EN, regValue);
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
                clkmux_sel(MT_MUX_PWM, config_data->clock_source, "DISP_PWM");
                BLS_DBG("disp_bls_init : CLK_CFG_1 0x%x => 0x%x\n", regVal, DISP_REG_GET(CLK_CFG_1));
            }
            gPWMDiv = (config_data->div == 0) ? PWM_DEFAULT_DIV_VALUE : config_data->div;
            gPWMDiv &= 0x3FF;
            BLS_MSG("disp_bls_init : PWM config data (%d,%d)\n", config_data->clock_source, config_data->div);
        }
    }
        
    BLS_DBG("disp_bls_init : srcWidth = %d, srcHeight = %d\n", srcWidth, srcHeight);
    BLS_MSG("disp_bls_init : BLS_EN=0x%x, PWM_DUTY=%d, PWM_DUTY_RD=%d, CG=0x%x, %d, %d\n", 
        DISP_REG_GET(DISP_REG_BLS_EN),
        DISP_REG_GET(DISP_REG_BLS_PWM_DUTY),
        DISP_REG_GET(DISP_REG_BLS_PWM_DUTY_RD),
        DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0),
        clock_is_on(MT_CG_DISP0_MDP_BLS_26M),
        clock_is_on(MT_CG_DISP0_DISP_BLS));
      
    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE, (srcHeight << 16) | srcWidth);
    DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x0 | (gPWMDiv << 16));
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING, 0x0);
    DISP_REG_SET(DISP_REG_BLS_INTEN, 0xF);
    if (!(DISP_REG_GET(DISP_REG_BLS_EN) & 0x10000))
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, 0);

    disp_bls_update_gamma_lut();
    //disp_bls_update_pwm_lut();

    disp_bls_config_full(srcWidth, srcHeight);

    if (dbg_log) 
        disp_dump_reg(DISP_MODULE_BLS);
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
                clkmux_sel(MT_MUX_PWM, config_data->clock_source, "DISP_PWM");
                BLS_DBG("disp_bls_init : CLK_CFG_1 0x%x => 0x%x\n", regVal, DISP_REG_GET(CLK_CFG_1));
            }
            gPWMDiv = (config_data->div == 0) ? PWM_DEFAULT_DIV_VALUE : config_data->div;
            gPWMDiv &= 0x3FF;
            BLS_MSG("disp_bls_config : PWM config data (%d,%d)\n", config_data->clock_source, config_data->div);
        }
    }
    
    if (!clock_is_on(MT_CG_DISP0_MDP_BLS_26M) || !gBLSPowerOn)
    {
        BLS_MSG("disp_bls_config: enable clock\n");
        enable_clock(MT_CG_DISP0_SMI_LARB0, "DDP");
        enable_clock(MT_CG_DISP0_MDP_BLS_26M         , "DDP");
        gBLSPowerOn = 1;
    }

    BLS_MSG("disp_bls_config : BLS_EN=0x%x, PWM_DUTY=%d, PWM_DUTY_RD=%d, CG=0x%x, %d, %d\n", 
        DISP_REG_GET(DISP_REG_BLS_EN),
        DISP_REG_GET(DISP_REG_BLS_PWM_DUTY),
        DISP_REG_GET(DISP_REG_BLS_PWM_DUTY_RD),
        DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0),
        clock_is_on(MT_CG_DISP0_MDP_BLS_26M),
        clock_is_on(MT_CG_DISP0_DISP_BLS));

#ifdef USE_DISP_BLS_MUTEX 
    BLS_MSG("disp_bls_config : gBLSMutexID = %d\n", gBLSMutexID);

    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 1);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 0);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gBLSMutexID), 0x200);    // BLS
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gBLSMutexID), 0);        // single mode
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gBLSMutexID), 1);

    if (disp_bls_get_mutex() == 0)
    {
    
#else
    BLS_MSG("disp_bls_config\n");
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x3);
#endif

        if (!(DISP_REG_GET(DISP_REG_BLS_EN) & 0x10000))
            DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, 0);
        DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x0 | (gPWMDiv << 16));
        //DISP_REG_SET(DISP_REG_BLS_EN, 0x00010001); //Enable BLS_EN

#ifdef USE_DISP_BLS_MUTEX 

        if (disp_bls_release_mutex() == 0)
            return 0;
    }
    return -1;
#else
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x0);
#endif

#endif
    BLS_MSG("disp_bls_config:-\n");
    return 0;
}

void disp_bls_config_full(unsigned int width, unsigned int height)
{
    unsigned int regVal;
    unsigned int dither_bpp = DISP_GetOutputBPPforDithering(); 

    BLS_DBG("disp_bls_config_full, width=%d, height=%d, reg=0x%x \n", 
        width, height, ((height<<16) + width));

    //DISP_REG_SET(DISP_REG_BLS_DEBUG             ,0x00000003);

#if defined(DDP_GAMMA_SUPPORT)
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING       ,0x00100007);
#else
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING       ,0x00100000);
#endif
    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE          ,((height<<16) + width));
    DISP_REG_SET(DISP_REG_BLS_GAMMA_SETTING     ,0x00000001);
    DISP_REG_SET(DISP_REG_BLS_GAMMA_BOUNDARY    ,0x3fffffff);

/* BLS Luminance LUT */
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(0)      ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(1)      ,0x00000004);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(2)      ,0x00000010);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(3)      ,0x00000024);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(4)      ,0x00000040);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(5)      ,0x00000064);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(6)      ,0x00000090);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(7)      ,0x000000C4);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(8)      ,0x00000100);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(9)      ,0x00000144);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(10)     ,0x00000190);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(11)     ,0x000001E4);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(12)     ,0x00000240);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(13)     ,0x00000244);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(14)     ,0x00000310);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(15)     ,0x00000384);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(16)     ,0x00000400);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(17)     ,0x00000484);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(18)     ,0x00000510);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(19)     ,0x000005A4);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(20)     ,0x00000640);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(21)     ,0x000006E4);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(22)     ,0x00000790);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(23)     ,0x00000843);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(24)     ,0x000008FF);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(25)     ,0x000009C3);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(26)     ,0x00000A8F);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(27)     ,0x00000B63);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(28)     ,0x00000C3F);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(29)     ,0x00000D23);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(30)     ,0x00000E0F);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(31)     ,0x00000F03);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(32)     ,0x00000FFF);
/* BLS Luminance 255 */
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE_255     ,0x00000FDF);
    
/* Dither */
    DISP_REG_SET(DISP_REG_BLS_DITHER(5)         ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(6)         ,0x00003004);
    DISP_REG_SET(DISP_REG_BLS_DITHER(7)         ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(8)         ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(9)         ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(10)        ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(11)        ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(12)        ,0x00000011);
    DISP_REG_SET(DISP_REG_BLS_DITHER(13)        ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(14)        ,0x00000000);
/* output RGB888 */
    if (dither_bpp == 16) // 565
    {
        DISP_REG_SET(DISP_REG_BLS_DITHER(15), 0x50500001);
        DISP_REG_SET(DISP_REG_BLS_DITHER(16), 0x50504040);
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000001);
    }
    else if (dither_bpp == 18) // 666
    {
        DISP_REG_SET(DISP_REG_BLS_DITHER(15), 0x40400001);
        DISP_REG_SET(DISP_REG_BLS_DITHER(16), 0x40404040);
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000001);
    }
    else if (dither_bpp == 24) // 888
    {
        DISP_REG_SET(DISP_REG_BLS_DITHER(15), 0x20200001);
        DISP_REG_SET(DISP_REG_BLS_DITHER(16), 0x20202020);
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000001);
    }
    else
    {
        BLS_MSG("error diter bpp = %d\n", dither_bpp);        
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000000);
    }


    DISP_REG_SET(DISP_REG_BLS_INTEN             ,0x0000000f); // no scene change

    regVal = DISP_REG_GET(DISP_REG_BLS_EN);    
    if (regVal & 0x10000) // PWM_EN has been enabled
        DISP_REG_SET(DISP_REG_BLS_EN                ,0x00010001);
    else   // on resume
        DISP_REG_SET(DISP_REG_BLS_EN                ,0x00000001);

    //DISP_REG_SET(DISP_REG_BLS_EN                ,0x00000001);

    //DISP_REG_SET(DISP_REG_BLS_DEBUG             ,0x00000000);
}


int disp_bls_set_max_backlight_(unsigned int level)
{
    mutex_lock(&backlight_mutex);
    BLS_MSG("disp_bls_set_max_backlight: level = %d, current level = %d\n", level * 1023 / 255, gMaxLevel);
    //PWM duty input =  PWM_DUTY_IN / 1024
    gMaxLevel = level * 1023 / 255;
    mutex_unlock(&backlight_mutex);
    return 0;
}


#if !defined(MTK_AAL_SUPPORT)
int disp_bls_set_backlight(unsigned int level)
{
    unsigned int regVal; 
    unsigned int mapped_level;
    BLS_MSG("disp_bls_set_backlight: %d, gBLSPowerOn = %d\n", level, gBLSPowerOn);

    mutex_lock(&backlight_mutex);

//	    if (level && (!clock_is_on(MT_CG_DISP0_MDP_BLS_26M) || !gBLSPowerOn)) 
//	    {   
//	        disp_bls_config();
//	    }
#ifdef USE_DISP_BLS_MUTEX 
    disp_bls_get_mutex();
#else
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x3);
#endif

    mapped_level = brightness_mapping(level);
    BLS_MSG("after mapping, mapped_level: %d\n", mapped_level);
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, mapped_level);
    if(level != 0)
    {
        regVal = DISP_REG_GET(DISP_REG_BLS_EN);
        if (!(regVal & 0x10000))
            DISP_REG_SET(DISP_REG_BLS_EN, regVal | 0x10000);
    }
    else
    {
        regVal = DISP_REG_GET(DISP_REG_BLS_EN);
        if (regVal & 0x10000)
            DISP_REG_SET(DISP_REG_BLS_EN, regVal & 0xFFFEFFFF);
    }
    BLS_MSG("after SET, PWM_DUTY: %d\n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));

#ifdef USE_DISP_BLS_MUTEX 
    disp_bls_release_mutex();
#else
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x0);
#endif

//	    if (!level && (clock_is_on(MT_CG_DISP0_MDP_BLS_26M) && gBLSPowerOn)) 
//	    {
//	        BLS_MSG("disp_bls_set_backlight: disable clock\n");
//	        disable_clock(MT_CG_DISP0_MDP_BLS_26M         , "DDP");
//	        disable_clock(MT_CG_DISP0_SMI_LARB0   , "DDP");
//	        gBLSPowerOn = 0;
//	    }
    mutex_unlock(&backlight_mutex);
    return 0;    
}
#else
int disp_bls_set_backlight(unsigned int level)
{
    DISP_AAL_PARAM *param;
    BLS_MSG("disp_bls_set_backlight: %d\n", level);

    mutex_lock(&backlight_mutex);
    disp_aal_lock();
    param = get_aal_config();
    param->pwmDuty = level;
    disp_aal_unlock();
    mutex_unlock(&backlight_mutex);
    return 0;
}
#endif

