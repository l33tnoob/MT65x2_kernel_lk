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

static int vibr_voltage;
#if 1
// pmic wrap read and write func
static unsigned int vibr_pmic_pwrap_read(U32 addr)
{

	U32 val =0;
	pwrap_read(addr, &val);
	return val;
	
}
#if 0
static void vibr_pmic_pwrap_write(unsigned int addr, unsigned int wdata)

{
	//unsigned int val =0;
    pwrap_write(addr, wdata);
}
#endif
#endif
extern void dct_pmic_VIBR_enable(kal_bool dctEnable);

void vibr_Enable_HW(void)
{
	
	// dct_pmic_VIBR_enable(1);
	hwPowerOn(MT6323_POWER_LDO_VIBR, vibr_voltage, "VIBR");
	printk("vibrator enable register = 0x%x\n", vibr_pmic_pwrap_read(0x0542));
	printk("[vibrator]vibr_Enable After\n");

}

void vibr_Disable_HW(void)
{
	// dct_pmic_VIBR_enable(0);
	hwPowerDown(MT6323_POWER_LDO_VIBR, "VIBR");
	printk("vibrator disable register = 0x%x\n", vibr_pmic_pwrap_read(0x0542));
	printk("[vibrator]vibr_Disable After\n");
}

void vibr_power_set(void)
{
	struct vibrator_hw* hw = get_cust_vibrator_hw();	
	vibr_voltage = hw->vosel;
	printk("[vibrator]vibr_init: vibrator set voltage = %d\n", vibr_voltage);
	//upmu_set_rg_vibr_vosel(hw->vib_vol);
}

struct vibrator_hw* mt_get_cust_vibrator_hw(void)
{
	return get_cust_vibrator_hw();
}
