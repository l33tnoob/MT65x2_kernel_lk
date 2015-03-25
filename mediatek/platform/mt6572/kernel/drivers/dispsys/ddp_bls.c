#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/xlog.h>
#include <linux/mutex.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_smi.h>

#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_debug.h"
#include "ddp_bls.h"
#include "disp_drv.h"
#include "ddp_hal.h"


#define DDP_GAMMA_SUPPORT

#define POLLING_TIME_OUT 1000

#define PWM_LOW_LIMIT 1  //PWM output lower bound = 8

#if !defined(MTK_AAL_SUPPORT)
    #ifdef USE_DISP_BLS_MUTEX
        static int gBLSMutexID = 3;
    #endif
static int gBLSPowerOn = 0;
#endif
static int gMaxLevel = 1023;

unsigned int init_width=0;
unsigned int init_height=0;

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
    DDP_DRV_DBG("get_gamma_index!\n");
    return &g_gamma_index;
}

DISPLAY_PWM_T * get_pwm_lut(void)
{
    DDP_DRV_DBG("get_pwm_lut!\n");
    return &g_pwm_lut;
}

extern unsigned char aal_debug_flag;
void disp_onConfig_bls(DISP_AAL_PARAM *param)
{
    unsigned long prevSetting = DISP_REG_GET(DISP_REG_BLS_BLS_SETTING);
    unsigned long regVal = 0;
    
    DISP_DBG("disp_onConfig_bls!\n");

    DISP_DBG("pwm duty = %lu\n", param->pwmDuty);
    if (param->pwmDuty == 0)
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, 0);
    else if (param->pwmDuty > gMaxLevel)
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, gMaxLevel);
    else
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, param->pwmDuty);

    DISP_DBG("bls setting = %lu\n", param->setting);
#if defined(DDP_GAMMA_SUPPORT)
    if (param->setting & ENUM_FUNC_GAMMA)
        regVal |= 0x7;
    else
#endif
        regVal &= ~0x7;

    if (param->setting & ENUM_FUNC_BLS)
        regVal |= 0x10100;
    else
        regVal &= ~0x10100;
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING, regVal | 0x100000);

    if (param->setting & ENUM_FUNC_BLS)
    {
        DISP_DBG("distion threshold = %lu\n", param->maxClrDistThd);
        DISP_DBG("predistion threshold = %lu\n", param->preDistThd);
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

    if (aal_debug_flag == 0)
    {
        DISP_REG_SET(DISP_REG_BLS_EN, 0x00010001);
    }
    else
    {
        DISP_REG_SET(DISP_REG_BLS_EN, 0x00010000);
    }

}


static unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;

    // PWM duty input =  PWM_DUTY_IN / 1024
    mapped_level = level * 1023 / 255;

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
        DISP_ERR("get mutex timeout! \n");
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
        DISP_ERR("release mutex timeout! \n");
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
    
        DISP_MSG("disp_bls_update_gamma_lut!\n");
    
        if (DISP_REG_GET(DISP_REG_BLS_EN) & 0x1)
        {
            DISP_ERR("try to update gamma lut while BLS is active\n");
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
            DISP_DBG("[%d] GAMMA LUT = 0x%x, (%lu, %lu, %lu)\n", i, DISP_REG_GET(DISP_REG_BLS_GAMMA_LUT(i)), 
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

    DISP_MSG("disp_bls_update_pwm_lut!\n");

    regValue = DISP_REG_GET(DISP_REG_BLS_EN);
    if (regValue & 0x1) {
        DISP_ERR("update PWM LUT while BLS func enabled!\n");
        disp_dump_reg(DISP_MODULE_BLS);
    }
    //DISP_REG_SET(DISP_REG_BLS_EN, (regValue & 0x00010000));

    for (i = 0; i < PWM_LUT_ENTRY; i++)
    {
        DISP_REG_SET(DISP_REG_BLS_LUMINANCE(i), g_pwm_lut.entry[i]);
        DISP_DBG("[%d] PWM LUT = 0x%x (%lu)\n", i, DISP_REG_GET(DISP_REG_BLS_LUMINANCE(i)), g_pwm_lut.entry[i]);

    }
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE_255, g_pwm_lut.entry[PWM_LUT_ENTRY-1]);
    //DISP_REG_SET(DISP_REG_BLS_EN, regValue);

}

void disp_bls_init(unsigned int srcWidth, unsigned int srcHeight)
{       
    DDP_DRV_DBG("disp_bls_init : srcWidth = %d, srcHeight = %d\n", srcWidth, srcHeight);

    // TODO: fix register setting
    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE, (srcHeight << 16) | srcWidth);
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
    DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x0);
    DISP_REG_SET(DISP_REG_BLS_INTEN, 0xF);

    disp_bls_update_gamma_lut();
    //disp_bls_update_pwm_lut();  // not used in 6572

#if 0 // TODO: fix Dither setting
    // Dithering
    DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000001);
    DISP_REG_SET(DISP_REG_BLS_DITHER(6), 0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(13), 0x00000222);
    DISP_REG_SET(DISP_REG_BLS_DITHER(14), 0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(15), 0x22220001);
    DISP_REG_SET(DISP_REG_BLS_DITHER(16), 0x22222222);
    DISP_REG_SET(DISP_REG_BLS_DITHER(17), 0x00000000);
#endif

    disp_bls_config_full(srcWidth, srcHeight);

#if 0
    disp_dump_reg(DISP_MODULE_BLS);
#endif

}

int disp_bls_config(void)
{
#if !defined(MTK_AAL_SUPPORT)
    
        if (!clock_is_on(MT_CG_MDP_BLS_26M_SW_CG))
        {
            // remove CG control to DDP path
            ASSERT(0);
        }
        if (!gBLSPowerOn)
        {
            DISP_MSG("disp_bls_config: enable clock\n");
//            enable_clock(MT_CG_DISP0_SMI_LARB0, "DDP");
            gBLSPowerOn = 1;
        }
    
#ifdef USE_DISP_BLS_MUTEX 
        DISP_MSG("disp_bls_config : gBLSMutexID = %d\n", gBLSMutexID);
    
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 1);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 0);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gBLSMutexID), 0x200);    // BLS
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gBLSMutexID), 0);        // single mode
    
        if (disp_bls_get_mutex() == 0)
        {
#else
        DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x3);
#endif
    
            DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
            DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x0);
    
#ifdef USE_DISP_BLS_MUTEX 
    
            if (disp_bls_release_mutex() == 0)
                return 0;
        }
        return -1;
#else
        DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x0);
#endif
    
#endif
        DISP_MSG("disp_bls_config:-\n");
        return 0;

    return 0;
}

void disp_bls_config_full(unsigned int width, unsigned int height)
{
    unsigned int regVal;
    unsigned int dither_bpp = DISP_GetOutputBPPforDithering(); 

    DDP_DRV_DBG("disp_bls_config_full, width=%d, height=%d, reg=0x%x \n", 
        width, height, ((height<<16) + width));

    DISP_REG_SET(DISP_REG_BLS_DEBUG             ,0x00000003);
//    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY          ,0x000003ff);

#if defined(DDP_GAMMA_SUPPORT)
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING       ,0x00100007);  // remove gain setting here
#else
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING       ,0x00100000);  // remove gain setting here
#endif

//    DISP_REG_SET(DISP_REG_BLS_FANA_SETTING      ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE          ,((height<<16) + width));
//    DISP_REG_SET(DISP_REG_BLS_GAIN_SETTING      ,0x00010001);
    DISP_REG_SET(DISP_REG_BLS_GAMMA_SETTING     ,0x00000001);
    DISP_REG_SET(DISP_REG_BLS_GAMMA_BOUNDARY    ,0x3fffffff);

#if 0
    DISP_REG_SET(DISP_REG_BLS_MAXCLR_THD        ,0x00002328);
    DISP_REG_SET(DISP_REG_BLS_DISTPT_THD        ,0x00006f54);
    DISP_REG_SET(DISP_REG_BLS_MAXCLR_LIMIT      ,0x00fa00b0);
    DISP_REG_SET(DISP_REG_BLS_DISTPT_LIMIT      ,0x00800020);
    DISP_REG_SET(DISP_REG_BLS_AVE_SETTING       ,0x00000010);
    DISP_REG_SET(DISP_REG_BLS_AVE_LIMIT         ,0x00f00060);
    DISP_REG_SET(DISP_REG_BLS_DISTPT_SETTING    ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_SC_DIFF_THD       ,0x00001770);
    DISP_REG_SET(DISP_REG_BLS_SC_BIN_THD        ,0x00000008);
    DISP_REG_SET(DISP_REG_BLS_MAXCLR_GRADUAL    ,0x00020001);
    DISP_REG_SET(DISP_REG_BLS_DISTPT_GRADUAL    ,0x00040001);
    DISP_REG_SET(DISP_REG_BLS_FAST_IIR_XCOEFF   ,0x01a201a2);
    DISP_REG_SET(DISP_REG_BLS_FAST_IIR_YCOEFF   ,0x00003cbc);
    DISP_REG_SET(DISP_REG_BLS_SLOW_IIR_XCOEFF   ,0x01500150);
    DISP_REG_SET(DISP_REG_BLS_SLOW_IIR_YCOEFF   ,0x00003d60);
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY          ,0x000003ff);
    DISP_REG_SET(DISP_REG_BLS_PWM_GRADUAL       ,0x00000000);  // default 0x80001
    DISP_REG_SET(DISP_REG_BLS_PWM_CON           ,0x00000000);  // default 0x00001
#endif

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
        DISP_MSG("error diter bpp = %d\n", dither_bpp);        
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000000);
    }


    DISP_REG_SET(DISP_REG_BLS_INTEN             ,0x0000000f); // no scene change

    regVal = DISP_REG_GET(DISP_REG_BLS_EN);
    DISP_REG_SET(DISP_REG_BLS_EN                ,regVal | 0x1);

    DISP_REG_SET(DISP_REG_BLS_DEBUG             ,0x00000000);


}


int disp_bls_set_max_backlight(unsigned int level)
{
    mutex_lock(&backlight_mutex);
    DISP_MSG("disp_bls_set_max_backlight: level = %d, current level = %d\n", level * 1023 / 255, gMaxLevel);
    //PWM duty input =  PWM_DUTY_IN / 1024
    gMaxLevel = level * 1023 / 255;
    mutex_unlock(&backlight_mutex);
    return 0;
}


#if !defined(MTK_AAL_SUPPORT)
int disp_bls_set_backlight(unsigned int level)
{
    unsigned int mapped_level;
    DISP_MSG("disp_bls_set_backlight: %d, gBLSPowerOn = %d\n", level, gBLSPowerOn);

    mutex_lock(&backlight_mutex);

    if (level && !clock_is_on(MT_CG_MDP_BLS_26M_SW_CG)) 
    {   
        // remove CG control to DDP path
        ASSERT(0);

        if (!gBLSPowerOn)
        {
            // config BLS parameter
            disp_bls_config();
        }
    }

#ifdef USE_DISP_BLS_MUTEX 
    disp_bls_get_mutex();
#else
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x3);
#endif

    mapped_level = brightness_mapping(level);
    DISP_MSG("after mapping, mapped_level: %d\n", mapped_level);
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, mapped_level);
    if (mapped_level)	// enable PWM generator
        DISP_REG_SET(DISP_REG_BLS_EN, DISP_REG_GET(DISP_REG_BLS_EN) | 0x10000);
    else		// disable PWM generator
        DISP_REG_SET(DISP_REG_BLS_EN, DISP_REG_GET(DISP_REG_BLS_EN) & 0xFFFEFFFF);
    DISP_MSG("after SET, PWM_DUTY: %d\n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));

#ifdef USE_DISP_BLS_MUTEX 
    disp_bls_release_mutex();
#else
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x0);
#endif

    if (!level && gBLSPowerOn) 
    {
        DISP_MSG("disp_bls_set_backlight: disable clock\n");
//        disable_clock(MT_CG_DISP0_SMI_LARB0   , "DDP");
        gBLSPowerOn = 0;
    }
    mutex_unlock(&backlight_mutex);

    return 0;    

}
#else
int disp_bls_set_backlight(unsigned int level)
{
    DISP_AAL_PARAM *param;
    DISP_MSG("disp_bls_set_backlight: %d\n", level);

    mutex_lock(&backlight_mutex);
    disp_aal_lock();
    param = get_aal_config();
    param->pwmDuty = brightness_mapping(level);
    disp_aal_unlock();
    mutex_unlock(&backlight_mutex);
    return 0;
}
#endif


int disp_bls_reset(void)
{
    unsigned int regValue;

    regValue = DISP_REG_GET(DISP_REG_BLS_RST);
    DISP_REG_SET(DISP_REG_BLS_RST, regValue | 0x1);
    DISP_REG_SET(DISP_REG_BLS_RST, regValue & (~0x1));

    return 0;
}


int disp_bls_enable_irq(unsigned int value)
{
    DISP_REG_SET(DISP_REG_BLS_INTSTA, 0x00);
    DISP_REG_SET(DISP_REG_BLS_INTEN, value);

    return 0;
}


