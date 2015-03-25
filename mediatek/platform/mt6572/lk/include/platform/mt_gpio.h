#ifndef _MT_GPIO_H_
#define _MT_GPIO_H_

#include <debug.h>
#include <platform/mt_typedefs.h>
#include <cust_gpio_usage.h>
#include <platform/gpio_const.h>
//#if !defined(MACH_FPGA)
//#define MT_GPIO_ENABLED 1
//#endif

/*----------------------------------------------------------------------------*/
//  Error Code No.
#define RSUCCESS	0
#define ERACCESS	1
#define ERINVAL		2
#define ERWRAPPER	3
/*----------------------------------------------------------------------------*/
#ifndef s32
	#define s32 signed int
#endif
#ifndef s64
	#define s64 signed long long
#endif
         
#define MAX_GPIO_PIN    (MT_GPIO_BASE_MAX)
/******************************************************************************
* Enumeration for GPIO pin
******************************************************************************/
/* GPIO MODE CONTROL VALUE*/
typedef enum {
    GPIO_MODE_UNSUPPORTED = -1,
    GPIO_MODE_GPIO  = 0,
    GPIO_MODE_00    = 0,
    GPIO_MODE_01    = 1,
    GPIO_MODE_02    = 2,
    GPIO_MODE_03    = 3,
    GPIO_MODE_04    = 4,
    GPIO_MODE_05    = 5,
    GPIO_MODE_06    = 6,
    GPIO_MODE_07    = 7,

    GPIO_MODE_MAX,
    GPIO_MODE_DEFAULT = GPIO_MODE_01,
} GPIO_MODE;
/*----------------------------------------------------------------------------*/
/* GPIO DIRECTION */
typedef enum {
    GPIO_DIR_UNSUPPORTED = -1,
    GPIO_DIR_IN     = 0,
    GPIO_DIR_OUT    = 1,

    GPIO_DIR_MAX,
    GPIO_DIR_DEFAULT = GPIO_DIR_IN,
} GPIO_DIR;
/*----------------------------------------------------------------------------*/
/* GPIO PULL ENABLE*/
typedef enum {
    GPIO_PULL_EN_UNSUPPORTED = -1,
    GPIO_PULL_DISABLE = 0,
    GPIO_PULL_ENABLE  = 1,

    GPIO_PULL_EN_MAX,
    GPIO_PULL_EN_DEFAULT = GPIO_PULL_ENABLE,
} GPIO_PULL_EN;
/*----------------------------------------------------------------------------*/
/* GPIO PULL-UP/PULL-DOWN*/
typedef enum {
    GPIO_PULL_UNSUPPORTED = -1,
    GPIO_PULL_DOWN  = 0,
    GPIO_PULL_UP    = 1,

    GPIO_PULL_MAX,
    GPIO_PULL_DEFAULT = GPIO_PULL_DOWN
} GPIO_PULL;
/*----------------------------------------------------------------------------*/
/* GPIO INVERSION */
typedef enum {
    GPIO_DATA_INV_UNSUPPORTED = -1,
    GPIO_DATA_UNINV = 0,
    GPIO_DATA_INV   = 1,

    GPIO_DATA_INV_MAX,
    GPIO_DATA_INV_DEFAULT = GPIO_DATA_UNINV
} GPIO_INVERSION;
/*----------------------------------------------------------------------------*/
/* GPIO OUTPUT */
typedef enum {
    GPIO_OUT_UNSUPPORTED = -1,
    GPIO_OUT_ZERO = 0,
    GPIO_OUT_ONE  = 1,

    GPIO_OUT_MAX,
    GPIO_OUT_DEFAULT = GPIO_OUT_ZERO,
    GPIO_DATA_OUT_DEFAULT = GPIO_OUT_ZERO,  /*compatible with DCT*/
} GPIO_OUT;
/* GPIO INPUT */
typedef enum {
    GPIO_IN_UNSUPPORTED = -1,
    GPIO_IN_ZERO = 0,
    GPIO_IN_ONE  = 1,

    GPIO_IN_MAX,
} GPIO_IN;
/*CLOCK OUT*/
typedef enum {
    CLK_OUT_UNSUPPORTED = -1,
    CLK_OUT0,
    CLK_OUT1,
    CLK_OUT2,
    CLK_OUT3,
    CLK_OUT4,
    CLK_OUT5,
    CLK_MAX
}GPIO_CLKOUT;

typedef enum CLK_SRC
{
    CLK_SRC_UNSUPPORTED = -1,
    CLK_SRC_F32K 	= 0x2,
    CLK_SRC_F26M 	= 0x4,
    CLK_SRC_FAXI0 	= 0x6,
    CLK_SRC_FAXI1 	= 0x8,
    CLK_SRC_FAXI2,
    CLK_SRC_FAXI3,
    CLK_SRC_FAXI4,
    CLK_SRC_DPI_PIXELPLL,
    CLK_SRC_3GPLL_123M,
    CLK_SRC_FMM,

    CLK_SRC_MAX
}GPIO_CLKSRC;
    
/* GPIO POWER*/
typedef enum {
    GPIO_VIO28 = 0,
    GPIO_VIO18 = 1,
    MSDC_VIO28_MC1 = 2,
    MSDC_VIO18_MC1 = 3,
    MSDC_VMC = 4,

    GPIO_VIO_MAX,
} GPIO_POWER;
/*----------------------------------------------------------------------------*/
typedef struct {
    u32 val;
    u32 set;
    u32 rst;
    u32 _align1;
} VAL_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    VAL_REGS    dir[5];             /*0x0000 ~ 0x004F: 80  bytes*/
    u8          rsv00[176];         /*0x0050 ~ 0x00FF: 176 bytes*/
    VAL_REGS    dout[5];            /*0x0100 ~ 0x014F: 80  bytes*/
    u8          rsv01[176];         /*0x0150 ~ 0x01FF: 176 bytes*/
    VAL_REGS    din[5];             /*0x0200 ~ 0x024F: 80  bytes*/
    u8          rsv02[176];         /*0x0250 ~ 0x02FF: 176 bytes*/
    VAL_REGS    mode[20];           /*0x0300 ~ 0x043F: 320 bytes*/
} GPIO_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    unsigned int no     : 16;
    unsigned int mode   : 3;
    unsigned int pullsel: 1;
    unsigned int din    : 1;
    unsigned int dout   : 1;
    unsigned int pullen : 1;
    unsigned int dir    : 1;
    unsigned int dinv   : 1;
    unsigned int _align : 7;
} GPIO_CFG;
/******************************************************************************
* GPIO Driver interface
******************************************************************************/
/*direction*/
s32 mt_set_gpio_dir(u32 pin, u32 dir);
s32 mt_get_gpio_dir(u32 pin);

/*pull enable*/
s32 mt_set_gpio_pull_enable(u32 pin, u32 enable);
s32 mt_get_gpio_pull_enable(u32 pin);
/*pull select*/
s32 mt_set_gpio_pull_select(u32 pin, u32 select);
s32 mt_get_gpio_pull_select(u32 pin);

/*data inversion*/
s32 mt_set_gpio_inversion(u32 pin, u32 enable);
s32 mt_get_gpio_inversion(u32 pin);

/*input/output*/
s32 mt_set_gpio_out(u32 pin, u32 output);
s32 mt_get_gpio_out(u32 pin);
s32 mt_get_gpio_in(u32 pin);

/*mode control*/
s32 mt_set_gpio_mode(u32 pin, u32 mode);
s32 mt_get_gpio_mode(u32 pin);

/*clock output setting*/
s32 mt_set_clock_output(u32 num, u32 src, u32 div);
s32 mt_get_clock_output(u32 num, u32 *src, u32 *div);

/*misc functions for protect GPIO*/
//void mt_gpio_unlock_init(int all);
void mt_gpio_set_default(void);
void mt_gpio_dump(void);
void mt_gpio_load(GPIO_REGS *regs);
void mt_gpio_checkpoint_save(void);
void mt_gpio_checkpoint_compare(void);
#endif //_MT_GPIO_H_
