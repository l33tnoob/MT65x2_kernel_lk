#include <printf.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>

#define POLLING_TIME_OUT 10000

static int gBLSMutexID = 3;

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
    printf("[DDP] disp_bls_init : srcWidth = %d, srcHeight = %d\n", srcWidth, srcHeight);

    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE, (srcHeight << 16) | srcWidth);
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
    DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x0);
    DISP_REG_SET(DISP_REG_BLS_EN, 0x00010000);

}

int disp_bls_config()
{
#if !defined(MTK_AAL_SUPPORT) 
#ifdef USE_DISP_BLS_MUTEX
    printf("[DDP] disp_bls_config : gBLSMutexID = %d\n", gBLSMutexID);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 1);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 0);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gBLSMutexID), 0x200);    // BLS
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gBLSMutexID), 0);        // single mode

    if (disp_bls_get_mutex() == 0)
    {
#else
        printf("[DDP] disp_bls_config\n");
        DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x3);
#endif

        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
        DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x0);
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
    printf("[DDP] disp_bls_set_backlight: %d\n", level);
    
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

