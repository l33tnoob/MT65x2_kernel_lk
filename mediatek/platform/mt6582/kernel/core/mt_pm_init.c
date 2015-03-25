#include <linux/pm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/xlog.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "mach/irqs.h"
#include "mach/sync_write.h"
#include "mach/mt_reg_base.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_spm.h"
#include "mach/mt_sleep.h"
#include "mach/mt_dcm.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_cpufreq.h"
#include "mach/mt_gpufreq.h"
#include "mach/mt_dormant.h"

#define pminit_write(addr, val)        mt65xx_reg_sync_writel((val), ((void *)addr))

//fix for bring up
extern int mt_clkmgr_bringup_init(void);
extern void mt_idle_init(void);
extern void mt_power_off(void);
/*********************************************************************
 * FUNCTION DEFINATIONS
 ********************************************************************/

unsigned int mt_get_emi_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_8, clk_misc_cfg_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    pminit_write(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    pminit_write(CLK_MISC_CFG_1, 0xFFFFFF00); // select divider

    clk_cfg_8 = DRV_Reg32(CLK_CFG_8);
    pminit_write(CLK_CFG_8, (14 << 8)); // select abist_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    pminit_write(CLK26CALI_0, temp | 0x1); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x1)
    {
        printk("wait for emi frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        mdelay(10);
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;

    output = (temp * 26000) / 1024; // Khz

    pminit_write(CLK_CFG_8, clk_cfg_8);
    pminit_write(CLK_MISC_CFG_1, clk_misc_cfg_1);
    pminit_write(CLK26CALI_0, clk26cali_0);

    //printk("CLK26CALI = 0x%x, mem frequency = %d Khz\n", temp, output);

    return output;
}
EXPORT_SYMBOL(mt_get_emi_freq);

unsigned int mt_get_bus_freq(void)
{
#if 0
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_9, clk_misc_cfg_1, clk26cali_2;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    pminit_write(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    pminit_write(CLK_MISC_CFG_1, 0x00FFFFFF); // select divider

    clk_cfg_9 = DRV_Reg32(CLK_CFG_9);
    pminit_write(CLK_CFG_9, (1 << 16)); // select ckgen_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    pminit_write(CLK26CALI_0, temp | 0x10); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x10)
    {
        //printk("wait for bus frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        mdelay(10);
    }

    temp = DRV_Reg32(CLK26CALI_2) & 0xFFFF;

    output = (temp * 26000) / 1024; // Khz

    pminit_write(CLK_CFG_9, clk_cfg_9);
    pminit_write(CLK_MISC_CFG_1, clk_misc_cfg_1);
    pminit_write(CLK26CALI_0, clk26cali_0);

    //printk("CLK26CALI = 0x%x, bus frequency = %d Khz\n", temp, output);

    return output;
#else
    unsigned int mainpll_con0, mainpll_con1, main_diff;
    unsigned int clk_cfg_0, bus_clk;
    unsigned int output_freq = 0;
    
    clk_cfg_0 = DRV_Reg32(CLK_CFG_0);
    
    mainpll_con0 = DRV_Reg32(MAINPLL_CON0);
    mainpll_con1 = DRV_Reg32(MAINPLL_CON1);
    
    //main_diff = ((mainpll_con1 >> 12) - 0x8009A) / 2;
    main_diff = (((mainpll_con1 & 0x1FFFFF) >> 12) - 0x9A) / 2;
    
    if ((mainpll_con0 & 0xFF) == 0x01)
    {
        output_freq = 1001 + (main_diff * 13); // Mhz
    }
    
    if ((clk_cfg_0 & 0x7) == 1) // SYSPLL1_D2 = MAINPLL / 2 / 2
    {
        bus_clk = ((output_freq * 1000) / 2) / 2;
    }
    else if ((clk_cfg_0 & 0x7) == 2) // SYSPLL_D5 = MAINPLL / 5
    {
        bus_clk = (output_freq * 1000) / 5;
    }
    else if ((clk_cfg_0 & 0x7) == 3) // SYSPLL1_D4 = MAINPLL / 2 / 4
    {
        bus_clk = ((output_freq * 1000) / 2) / 4;
    }
    else if ((clk_cfg_0 & 0x7) == 4) // UNIVPLL_D5 = UNIVPLL / 5
    {
        bus_clk = (1248 * 1000) / 5;
    }
    else if ((clk_cfg_0 & 0x7) == 5) // UNIVPLL2_D2 = UNIVPLL / 3 / 2
    {
        bus_clk = ((1248 * 1000) / 3) / 2;
    }
    else if ((clk_cfg_0 & 0x7) == 6) // DMPLL_CK = DMPLL /2
    {
        bus_clk = (533 * 1000) / 2;
    }
    else if ((clk_cfg_0 & 0x7) == 7) // DMPLL_D2 = DMPLL / 2 /2
    {
        bus_clk = ((533 * 1000) / 2) / 2 ;
    }
    else // CLKSQ
    {
        bus_clk = 26 * 1000;
    }
    
    //printk("bus frequency = %d Khz\n", bus_clk);

    return bus_clk; // Khz
#endif
}
EXPORT_SYMBOL(mt_get_bus_freq);

unsigned int mt_get_cpu_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_8, clk_misc_cfg_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    pminit_write(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    pminit_write(CLK_MISC_CFG_1, 0xFFFF0300); // select divider

    clk_cfg_8 = DRV_Reg32(CLK_CFG_8);
    pminit_write(CLK_CFG_8, (39 << 8)); // select abist_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    pminit_write(CLK26CALI_0, temp | 0x1); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x1)
    {
        printk("wait for cpu frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        mdelay(10);
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;

    output = ((temp * 26000) / 1024) * 4; // Khz

    pminit_write(CLK_CFG_8, clk_cfg_8);
    pminit_write(CLK_MISC_CFG_1, clk_misc_cfg_1);
    pminit_write(CLK26CALI_0, clk26cali_0);

    //printk("CLK26CALI = 0x%x, cpu frequency = %d Khz\n", temp, output);

    return output;
}
EXPORT_SYMBOL(mt_get_cpu_freq);

static int cpu_speed_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "%d\n", mt_get_cpu_freq());

    len = p - buf;
    return len;
}

static int emi_speed_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "%d\n", mt_get_emi_freq());

    len = p - buf;
    return len;
}

static int bus_speed_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "%d\n", mt_get_bus_freq());

    len = p - buf;
    return len;
}

static int __init mt_power_management_init(void)
{
    struct proc_dir_entry *entry = NULL;
    struct proc_dir_entry *pm_init_dir = NULL;

		pm_power_off = mt_power_off;
		
    #if !defined (CONFIG_MT6582_FPGA_CA7)
    xlog_printk(ANDROID_LOG_INFO, "Power/PM_INIT", "Bus Frequency = %d KHz\n", mt_get_bus_freq());

    //cpu dormant driver init
    cpu_dormant_init();

    // SPM driver init
    spm_module_init();

    // Sleep driver init (for suspend)
    slp_module_init();

    //fix for bring up
    //mt_clk_mgr_init(); // clock manager init, including clock gating init
    mt_clkmgr_bringup_init();
    //mt_clkmgr_init();

    //mt_pm_log_init(); // power management log init

    mt_dcm_init(); // dynamic clock management init
    mt_idle_init();

    pm_init_dir = proc_mkdir("pm_init", NULL);
    if (!pm_init_dir)
    {
        pr_err("[%s]: mkdir /proc/pm_init failed\n", __FUNCTION__);
    }
    else
    {
        entry = create_proc_entry("cpu_speed_dump", S_IRUGO, pm_init_dir);
        if (entry)
        {
            entry->read_proc = cpu_speed_dump_read;
        }
        entry = create_proc_entry("emi_speed_dump", S_IRUGO, pm_init_dir);
        if (entry)
        {
            entry->read_proc = emi_speed_dump_read;
        }
        entry = create_proc_entry("bus_speed_dump", S_IRUGO, pm_init_dir);
        if (entry)
        {
            entry->read_proc = bus_speed_dump_read;
        }
    }
    #endif
    
    return 0;
}

arch_initcall(mt_power_management_init);

MODULE_DESCRIPTION("MTK Power Management Init Driver");
MODULE_LICENSE("GPL");
