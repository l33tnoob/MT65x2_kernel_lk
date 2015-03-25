/******************************************************************************
 * mt6575_vibrator.c - MT6575 Android Linux Vibrator Device Driver
 * 
 * Copyright 2009-2010 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *     This file provid the other drivers vibrator relative functions
 *
 ******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <mach/mt_typedefs.h>
#include <cust_vibrator.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>

extern S32 pwrap_read( U32  adr, U32 *rdata );
extern S32 pwrap_write( U32  adr, U32  wdata );
#if 0
// pmic wrap read and write func
static unsigned int vibr_pmic_pwrap_read(U32 addr)
{

	U32 val =0;
	pwrap_read(addr, &val);
	return val;
	
}

static void vibr_pmic_pwrap_write(unsigned int addr, unsigned int wdata)

{
	//unsigned int val =0;
    pwrap_write(addr, wdata);
}
#endif
extern void dct_pmic_VIBR_enable(kal_bool dctEnable);

void vibr_Enable_HW(void)
{
	
	dct_pmic_VIBR_enable(1);

}

void vibr_Disable_HW(void)
{
	dct_pmic_VIBR_enable(0);
}

void vibr_power_set(void)
{
#ifdef CUST_VIBR_VOL
	struct vibrator_hw* hw = get_cust_vibrator_hw();	
	printk("[vibrator]vibr_init: vibrator set voltage = %d\n", hw->vib_vol);
	upmu_set_rg_vibr_vosel(hw->vib_vol);
#endif
}

struct vibrator_hw* mt_get_cust_vibrator_hw(void)
{
	return get_cust_vibrator_hw();
}
