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

//fix for bring up
extern void mt_idle_init(void);

/*********************************************************************
 * FUNCTION DEFINATIONS
 ********************************************************************/

static int cpu_speed_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "%d\n", mt_get_cpu_freq());

    len = p - buf;
    return len;
}

static int __init mt_power_management_init(void)
{
    struct proc_dir_entry *entry = NULL;
    struct proc_dir_entry *pm_init_dir = NULL;

    xlog_printk(ANDROID_LOG_INFO, "Power/PM_INIT", "Bus Frequency = %d KHz\n", mt_get_bus_freq());

    // clock manager init, including clock gating init
    mt_clkmgr_init();

    // SPM driver init
    spm_module_init();

    // Sleep driver init (for suspend)
    slp_module_init();

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
    }

    return 0;
}

arch_initcall(mt_power_management_init);

MODULE_DESCRIPTION("MTK Power Management Init Driver");
MODULE_LICENSE("GPL");
