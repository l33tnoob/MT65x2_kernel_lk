#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_spm_idle.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_power_gs.h>

#include <mach/mt_pmic_wrap.h>
#include <mach/pmic_mt6323_sw.h>
#include <mach/upmu_common.h>
#include <mach/upmu_hw.h>

#define SUPPORT_MT6333 0

#define gs_read(addr) (*(volatile u32 *)(addr))

struct proc_dir_entry *mt_power_gs_dir = NULL;

static U16 gs6323_pmic_read(U16 addr)
{
	U32 rdata=0;
	pwrap_read((U32)addr, &rdata);
	return (U16)rdata;
}

#if SUPPORT_MT6333
extern kal_uint32 mt6333_get_reg_value(kal_uint32 reg);
static U16 gs6333_pmic_read(U16 addr)
{
	U32 rdata=0;
	rdata = mt6333_get_reg_value((U32)addr);
	return (U16)rdata;
}
#endif

static void mt_power_gs_compare_pll(void)
{
    if (pll_is_on(MSDCPLL)) {
        printk("MSDCPLL: %s\n", pll_is_on(MSDCPLL) ?  "on" : "off");
    }

    if (subsys_is_on(SYS_MD1)) {
        printk("SYS_MD1: %s\n", subsys_is_on(SYS_MD1) ? "on" : "off");
    }

    if (subsys_is_on(SYS_CONN)) {
        printk("SYS_CONN: %s\n", subsys_is_on(SYS_CONN) ? "on" : "off");
    }

    if (subsys_is_on(SYS_DIS)) {
        printk("SYS_DIS: %s\n", subsys_is_on(SYS_DIS) ? "on" : "off");
    }

    if (subsys_is_on(SYS_MFG)) {
        printk("SYS_MFG: %s\n", subsys_is_on(SYS_MFG) ? "on" : "off");
    }

    if (subsys_is_on(SYS_ISP)) {
        printk("SYS_ISP: %s\n", subsys_is_on(SYS_ISP) ? "on" : "off");
    }

    if (subsys_is_on(SYS_VDE)) {
        printk("SYS_VDE: %s\n", subsys_is_on(SYS_VDE) ? "on" : "off");
    }
}

void mt_power_gs_diff_output(unsigned int val1, unsigned int val2)
{
    int i = 0;
    unsigned int diff = val1 ^ val2;

    while (diff != 0)
    {
        if ((diff % 2) != 0) printk("%d ", i);
        diff /= 2;
        i++;
    }
    printk("\n");
}

void mt_power_gs_compare(char *scenario, \
                         unsigned int *mt6582_power_gs, unsigned int mt6582_power_gs_len, \
                         unsigned int *mt6323_power_gs, unsigned int mt6323_power_gs_len, \
                         unsigned int *mt6333_power_gs, unsigned int mt6333_power_gs_len)
{
    unsigned int i, val1, val2;

    // MT6582
    for (i = 0 ; i < mt6582_power_gs_len ; i += 3)
    {
        val1 = gs_read(mt6582_power_gs[i]) & mt6582_power_gs[i+1];
        val2 = mt6582_power_gs[i+2] & mt6582_power_gs[i+1];
        if (val1 != val2)
        {
            printk("%s - MT6582 - 0x%x - 0x%x - 0x%x - 0x%x - ", \
                    scenario, mt6582_power_gs[i], gs_read(mt6582_power_gs[i]), mt6582_power_gs[i+1], mt6582_power_gs[i+2]);
            mt_power_gs_diff_output(val1, val2);
        }
    }

    // MT6323
    for (i = 0 ; i < mt6323_power_gs_len ; i += 3)
    {
        val1 = gs6323_pmic_read(mt6323_power_gs[i]) & mt6323_power_gs[i+1];
        val2 = mt6323_power_gs[i+2] & mt6323_power_gs[i+1];
        if (val1 != val2)
        {
            printk("%s - MT6323 - 0x%x - 0x%x - 0x%x - 0x%x - ", \
                    scenario, mt6323_power_gs[i], gs6323_pmic_read(mt6323_power_gs[i]), mt6323_power_gs[i+1], mt6323_power_gs[i+2]);
            mt_power_gs_diff_output(val1, val2);
        }
    }

    #if SUPPORT_MT6333
    // MT6333
    for (i = 0 ; i < mt6333_power_gs_len ; i += 3)
    {
        val1 = gs6333_pmic_read(mt6333_power_gs[i]) & mt6333_power_gs[i+1];
        val2 = mt6333_power_gs[i+2] & mt6333_power_gs[i+1];
        if (val1 != val2)
        {
            printk("%s - MT6333 - 0x%x - 0x%x - 0x%x - 0x%x - ", \
                    scenario, mt6333_power_gs[i], gs6333_pmic_read(mt6333_power_gs[i]), mt6333_power_gs[i+1], mt6333_power_gs[i+2]);
            mt_power_gs_diff_output(val1, val2);
        }
    }
    #endif

    mt_power_gs_compare_pll();
}
EXPORT_SYMBOL(mt_power_gs_compare);

static void __exit mt_power_gs_exit(void)
{
    //return 0;
}

static int __init mt_power_gs_init(void)
{
    mt_power_gs_dir = proc_mkdir("mt_power_gs", NULL);
    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }

    return 0;
}

module_init(mt_power_gs_init);
module_exit(mt_power_gs_exit);

MODULE_DESCRIPTION("MT6582 Low Power Golden Setting");
