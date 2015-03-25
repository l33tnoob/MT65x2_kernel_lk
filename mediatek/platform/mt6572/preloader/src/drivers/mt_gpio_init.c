/******************************************************************************
 * gpio_init.c - MT6516 Linux GPIO Device Driver
 *
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 *
 * DESCRIPTION:
 *     default GPIO init
 *
 ******************************************************************************/

#include <gpio.h>

#if !(CFG_FPGA_PLATFORM)
#include <cust_power.h>
#include <cust_gpio_boot.h>
#endif

#include <platform.h>
#include <mt_pmic_wrap_init.h>
//#include <debug.h>
#define GPIO_INIT_DEBUG 1
/*----------------------------------------------------------------------------*/
#define GPIOTAG "[GPIO] "
#define GPIODBG(fmt, arg...)    printf(GPIOTAG "%s: " fmt, __FUNCTION__ ,##arg)
#define GPIOERR(fmt, arg...)    printf(GPIOTAG "%s: " fmt, __FUNCTION__ ,##arg)
#define GPIOVER(fmt, arg...)    printf(GPIOTAG "%s: " fmt, __FUNCTION__ ,##arg)

#define GPIO_WR32(addr, data)   DRV_WriteReg32(addr,data)
#define GPIO_RD32(addr)         DRV_Reg32(addr)

#define GPIOEXT_WR(addr, data)   pwrap_write(addr, data)
#define GPIOEXT_RD(addr)         ({ \
		u32 ext_data; \
		int ret; \
		ret = pwrap_read(addr,&ext_data); \
		(ret != 0)?-1:ext_data;})
#define GPIOEXT_BASE        (0xC000) 			//PMIC GPIO base.

#define ADDR_BIT 0
#define VAL_BIT  1
#define MASK_BIT 2
/*----------------------------------------------------------------------------*/
#if (CFG_FPGA_PLATFORM)
void mt_gpio_set_default(void)
{
	return;
}
#else

#include <gpio_init.h>
#if defined(GPIO_INIT_DEBUG)
static UINT32 save[];
static UINT32 save_ext[];
#endif

void mt_gpio_set_default_chip(void)
{
    u32 idx;
    u32 val;
    u32 mask;

    for (idx = 0; idx < sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1)); idx++){
	mask = gpio_init_value[idx][MASK_BIT];
	val = GPIO_RD32(gpio_init_value[idx][ADDR_BIT]);
	val &= ~(mask);
	val |= ((gpio_init_value[idx][VAL_BIT])&mask);
        GPIO_WR32(gpio_init_value[idx][ADDR_BIT],val);
    }


    //GPIOVER("mt_gpio_set_default() done\n");
}
void mt_gpio_set_default_ext(void)
{
#if 0
	GPIOEXT_REGS *pReg = (GPIOEXT_REGS*)(GPIOEXT_BASE);
    int idx;
    u32 val;

    for (idx = 0; idx < sizeof(pReg->dir)/sizeof(pReg->dir[0]); idx++){
		val = gpioext_init_dir_data[idx];
        GPIOEXT_WR(&pReg->dir[idx],val);
    }

    for (idx = 0; idx < sizeof(pReg->pullen)/sizeof(pReg->pullen[0]); idx++){
		val = gpioext_init_pullen_data[idx];
        GPIOEXT_WR(&pReg->pullen[idx],val);
    }

    for (idx = 0; idx < sizeof(pReg->pullsel)/sizeof(pReg->pullsel[0]); idx++){
		val = gpioext_init_pullsel_data[idx];
        GPIOEXT_WR(&pReg->pullsel[idx],val);
    }

    for (idx = 0; idx < sizeof(pReg->dinv)/sizeof(pReg->dinv[0]); idx++) {
		val = gpioext_init_dinv_data[idx];
        GPIOEXT_WR(&pReg->dinv[idx],val);
    }

    for (idx = 0; idx < sizeof(pReg->dout)/sizeof(pReg->dout[0]); idx++) {
		val = gpioext_init_dout_data[idx];
        GPIOEXT_WR(&pReg->dout[idx],val);
    }

    for (idx = 0; idx < sizeof(pReg->mode)/sizeof(pReg->mode[0]); idx++) {
		val = gpioext_init_mode_data[idx];
        GPIOEXT_WR(&pReg->mode[idx],val);
    }

    GPIOVER("mt_gpio_set_default_ext() done\n");
#endif
}
void mt_gpio_set_dsel(u8 nml2_1v8,u8 bsi_1v8,u8 bpi_1v8)
{
#if 0
	u32 mask;
	u32 val;
	u32 bit;
#define TDSEL_1V8 0x0
#define TDSEL_3V3 0x5
#define RDSEL_1V8 0x0
#define RDSEL_3V3 0xC
#define GPIO_NML2_OFFSET 0x730
#define GPIO_NML2_PWMR_OFFSET 0x740
#define GPIO_BSI_OFFSET 0x750
#define GPIO_BPI_OFFSET 0x740
	//set NML2 UART
	val = GPIO_RD32(GPIO_BASE + GPIO_NML2_OFFSET);
	mask = (1L << 4) - 1;
	bit = 8;
	val &= ~(mask << (bit));
	val |= (nml2_1v8?TDSEL_1V8:TDSEL_3V3)<<bit;
	GPIO_WR32(GPIO_BASE + GPIO_NML2_OFFSET, val);

	val = GPIO_RD32(GPIO_BASE + GPIO_NML2_OFFSET);
	mask = (1L << 6) - 1;
	bit = 16;
	val &= ~(mask << (bit));
	val |= (nml2_1v8?RDSEL_1V8:RDSEL_3V3)<<bit;
	GPIO_WR32(GPIO_BASE + GPIO_NML2_OFFSET, val);

	//set NML2 PWM
	val = GPIO_RD32(GPIO_BASE + GPIO_NML2_OFFSET);
	mask = (1L << 4) - 1;
	bit = 24;
	val &= ~(mask << (bit));
	val |= (nml2_1v8?TDSEL_1V8:TDSEL_3V3)<<bit;
	GPIO_WR32(GPIO_BASE + GPIO_NML2_OFFSET, val);

	val = GPIO_RD32(GPIO_BASE + GPIO_NML2_PWMR_OFFSET);
	mask = (1L << 6) - 1;
	bit = 0;
	val &= ~(mask << (bit));
	val |= (nml2_1v8?RDSEL_1V8:RDSEL_3V3)<<bit;
	GPIO_WR32(GPIO_BASE + GPIO_NML2_PWMR_OFFSET, val);

	//set BPI
	val = GPIO_RD32(GPIO_BASE + GPIO_BPI_OFFSET);
	mask = (1L << 4) - 1;
	bit = 8;
	val &= ~(mask << (bit));
	val |= (bpi_1v8?TDSEL_1V8:TDSEL_3V3)<<bit;
	GPIO_WR32(GPIO_BASE + GPIO_BPI_OFFSET, val);

	val = GPIO_RD32(GPIO_BASE + GPIO_BPI_OFFSET );
	mask = (1L << 6) - 1;
	bit = 16;
	val &= ~(mask << (bit));
	val |= (bpi_1v8?RDSEL_1V8:RDSEL_3V3)<<bit;
	GPIO_WR32(GPIO_BASE + GPIO_BPI_OFFSET, val);

	//set BSI
	val = GPIO_RD32(GPIO_BASE + GPIO_BSI_OFFSET);
	mask = (1L << 4) - 1;
	bit = 0;
	val &= ~(mask << (bit));
	val |= (bsi_1v8?TDSEL_1V8:TDSEL_3V3)<<bit;
	GPIO_WR32(GPIO_BASE + GPIO_BSI_OFFSET, val);

	val = GPIO_RD32(GPIO_BASE + GPIO_BSI_OFFSET );
	mask = (1L << 6) - 1;
	bit = 8;
	val &= ~(mask << (bit));
	val |= (bsi_1v8?RDSEL_1V8:RDSEL_3V3)<<bit;
	GPIO_WR32(GPIO_BASE + GPIO_BSI_OFFSET, val);
	GPIOVER("NML2 0x%x,BPI 0x%x, BSI 0x%x\n",GPIO_RD32(GPIO_BASE + GPIO_NML2_OFFSET),GPIO_RD32(GPIO_BASE + GPIO_BPI_OFFSET),GPIO_RD32(GPIO_BASE + GPIO_BSI_OFFSET));
#endif
}

void mt_gpio_set_default(void)
{
	mt_gpio_set_default_chip();
	//mt_gpio_set_default_ext();
	//mt_gpio_set_power();
}
/*----------------------------------------------------------------------------*/
//EXPORT_SYMBOL(mt_gpio_set_default);
/*----------------------------------------------------------------------------*/
#if 0
void mt_gpio_checkpoint_save_ext(void)
{
#if defined(GPIO_INIT_DEBUG)
    int idx;

    memset(save_ext, 0x00, sizeof(gpio_init_value_ext)/((sizeof(UINT32))*(MASK_BIT+1)));
    for (idx = 0; idx < sizeof(gpio_init_value_ext)/((sizeof(UINT32))*(MASK_BIT+1)); idx++)
        save_ext[idx] = GPIO_RD32(gpio_init_value_ext[idx][0]);

#endif
}
#endif
#if 0
void mt_gpio_checkpoint_save(void)
{
#if defined(GPIO_INIT_DEBUG)
    int idx;

    memset(save, 0x00, sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1)));
    for (idx = 0; idx < sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1)); idx++)
        save[idx] = GPIO_RD32(gpio_init_value[idx][0]);

#endif
	//mt_gpio_checkpoint_save_ext();
}
/*----------------------------------------------------------------------------*/
EXPORT_SYMBOL(mt_gpio_checkpoint_save);
/*----------------------------------------------------------------------------*/
void mt_gpio_dump_diff_ext(UINT32 pre, UINT32 cur)
{
#if defined(GPIO_INIT_DEBUG)
    int idx;

    GPIOVER("------ GPIOEXT dumping difference between %p and %p ------\n", &pre, &cur);
    for (idx = 0; idx < sizeof(pre); idx++) {
        if (pre != cur)
            GPIOVER("diff: raw[%2d]    : 0x%08X <=> 0x%08X\n", idx, pre, cur);
    }

    //GPIOVER("memcmp(%p, %p, %d) = %d\n", p, q, sizeof(*pre), memcmp(p, q, sizeof(*pre)));
    GPIOVER("------ GPIOEXT dumping difference end --------------------------------\n");
#endif
}
void mt_gpio_dump_diff(UINT32 pre, UINT32 cur)
{
#if defined(GPIO_INIT_DEBUG)
    int idx;

    GPIOVER("------ dumping difference between %p and %p ------\n", &pre, &cur);
    for (idx = 0; idx < sizeof(pre); idx++) {
        if (pre != cur)
            GPIOVER("diff: raw[%2d]    : 0x%08X <=> 0x%08X\n", idx, pre, cur);
    }

    //GPIOVER("memcmp(%p, %p, %d) = %d\n", p, q, sizeof(*pre), memcmp(p, q, sizeof(*pre)));
    GPIOVER("------ dumping difference end --------------------------------\n");
#endif
}
#endif
/*----------------------------------------------------------------------------*/
#if 0
void mt_gpio_checkpoint_compare_ext(void)
{
#if defined(GPIO_INIT_DEBUG)
    UINT32 latest[sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1))];
    int idx;

    memset(&latest[0], 0x00, sizeof(latest));
    for (idx = 0; idx < sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1)); idx++)
        latest[idx] = GPIOEXT_RD(gpio_init_value[idx][0]);

    if (memcmp(&latest, &save_ext, sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1)))) {
        GPIODBG("GPIOEXT checkpoint compare fail!!\n");
        GPIODBG("dump checkpoint....\n");
        //mt_gpio_dump(&save_ext);
        GPIODBG("\n\n");
        GPIODBG("dump current state\n");
        //mt_gpio_dump(&latest);
        GPIODBG("\n\n");
        mt_gpio_dump_diff_ext(&save_ext, &latest);
        //WARN_ON(1);
    } else {
        GPIODBG("GPIOEXT checkpoint compare success!!\n");
    }
#endif
}
void mt_gpio_checkpoint_compare(void)
{
#if defined(GPIO_INIT_DEBUG)
    UINT32 latest[sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1))];
    int idx;

    memset(&latest[0], 0x00, sizeof(latest));
    for (idx = 0; idx < sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1)); idx++)
        latest[idx] = GPIO_RD32(gpio_init_value[idx][0]);

    if (memcmp(&latest, &save, sizeof(gpio_init_value)/((sizeof(UINT32))*(MASK_BIT+1)))) {
        GPIODBG("checkpoint compare fail!!\n");
        GPIODBG("dump checkpoint....\n");
        //mt_gpio_dump(&save);
        GPIODBG("\n\n");
        GPIODBG("dump current state\n");
        //mt_gpio_dump(&latest);
        GPIODBG("\n\n");
        mt_gpio_dump_diff(&save, &latest);
        //WARN_ON(1);
    } else {
        GPIODBG("checkpoint compare success!!\n");
    }
#endif
	//mt_gpio_checkpoint_compare_ext();
}
/*----------------------------------------------------------------------------*/
EXPORT_SYMBOL(mt_gpio_checkpoint_compare);
/*----------------------------------------------------------------------------*/
#endif
#endif
