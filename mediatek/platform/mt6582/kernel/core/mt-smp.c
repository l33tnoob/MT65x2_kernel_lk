#include <linux/init.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <asm/localtimer.h>
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
#include <asm/hardware/gic.h>
#include <mach/mtk_boot_share_page.h>
#endif
#include <asm/fiq_glue.h>
#include <mach/mt_reg_base.h>
#include <mach/smp.h>
#include <mach/sync_write.h>
#include <mach/hotplug.h>
#ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
#include <mach/mt_spm_mtcmos.h>
#endif
#include <mach/mt_spm_idle.h>
#include <mach/wd_api.h>

#if defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
#include <mach/mt_secure_api.h>
#endif

#ifdef CONFIG_MTK_SCHED_TRACERS
#include <trace/events/mtk_events.h>
#include "kernel/trace/trace.h"
#endif

#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
#define SLAVE1_MAGIC_REG (0xF0002000 + 0x38)
#define SLAVE2_MAGIC_REG (0xF0002000 + 0x3C)
#define SLAVE3_MAGIC_REG (0xF0002000 + 0x40)
#else
#define SLAVE1_MAGIC_REG (SRAMROM_BASE+0x38)
#define SLAVE2_MAGIC_REG (SRAMROM_BASE+0x3C)
#define SLAVE3_MAGIC_REG (SRAMROM_BASE+0x40)
#endif

#define SLAVE1_MAGIC_NUM 0x534C4131
#define SLAVE2_MAGIC_NUM 0x4C415332
#define SLAVE3_MAGIC_NUM 0x41534C33
#define SLAVE_JUMP_REG  (SRAMROM_BASE+0x34)

#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
// Secure world location, using SRAM now.
#define NS_SLAVE_JUMP_REG  (BOOT_SHARE_BASE+1020)
#define NS_SLAVE_MAGIC_REG (BOOT_SHARE_BASE+1016)
#define NS_SLAVE_BOOT_ADDR (BOOT_SHARE_BASE+1012)
#endif

extern void mt_secondary_startup(void);
extern void irq_raise_softirq(const struct cpumask *mask, unsigned int irq);
extern void mt_gic_secondary_init(void);


extern unsigned int irq_total_secondary_cpus;
static unsigned int is_secondary_cpu_first_boot = 0;
static DEFINE_SPINLOCK(boot_lock);

static const unsigned int secure_magic_reg[] = {SLAVE1_MAGIC_REG, SLAVE2_MAGIC_REG, SLAVE3_MAGIC_REG};
static const unsigned int secure_magic_num[] = {SLAVE1_MAGIC_NUM, SLAVE2_MAGIC_NUM, SLAVE3_MAGIC_NUM};
typedef int (*spm_mtcmos_ctrl_func)(int state, int chkWfiBeforePdn);
static const spm_mtcmos_ctrl_func secure_ctrl_func[] = {spm_mtcmos_ctrl_cpu1, spm_mtcmos_ctrl_cpu2, spm_mtcmos_ctrl_cpu3};

/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen".
 */
volatile int pen_release = -1;

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void __cpuinit write_pen_release(int val)
{
    pen_release = val;
    smp_wmb();
    __cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
    outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}


int L2CTLR_get_core_count(void){
    unsigned int cores = 0;
    extern u32 get_devinfo_with_index(u32 index);
    u32 idx = 3;
    u32 value = 0;
    value = get_devinfo_with_index(idx);

    value = (value >> 24) & 0xF;
    if (value == 0x0)
        cores = 4;
    else
        cores = 2;
    
    printk("[CORE] num:%d\n",cores);
    return cores;
}

void __cpuinit platform_secondary_init(unsigned int cpu)
{
    struct wd_api *wd_api = NULL;

    printk(KERN_INFO "Slave cpu init\n");
    HOTPLUG_INFO("platform_secondary_init, cpu: %d\n", cpu);

    mt_gic_secondary_init();

    /*
     * let the primary processor know we're out of the
     * pen, then head off into the C entry point
     */
    write_pen_release(-1);

    get_wd_api(&wd_api);
    if (wd_api)
        wd_api->wd_cpu_hot_plug_on_notify(cpu);

    fiq_glue_resume();

#ifdef SPM_MCDI_FUNC
    spm_hot_plug_in_before(cpu);
#endif

#ifdef CONFIG_MTK_SCHED_TRACERS
    trace_cpu_hotplug(cpu, 1, per_cpu(last_event_ts, cpu));
    per_cpu(last_event_ts, cpu) = ns2usecs(ftrace_now(cpu));
#endif

    /*
     * Synchronise with the boot thread.
     */
    spin_lock(&boot_lock);
    spin_unlock(&boot_lock);
}

static void __cpuinit mt_wakeup_cpu(int cpu)
{
    mt65xx_reg_sync_writel(secure_magic_num[cpu-1], secure_magic_reg[cpu-1]);
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
    *((unsigned int*)NS_SLAVE_MAGIC_REG) = secure_magic_num[cpu-1];
#endif
    HOTPLUG_INFO("SLAVE%d_MAGIC_NUM:%x\n", cpu, secure_magic_num[cpu-1]);
#ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
    if (is_secondary_cpu_first_boot)
    {
    	printk("mt_wakeup_cpu: first boot!(%d)\n", cpu);
        --is_secondary_cpu_first_boot;
    }
    else
    {
    	printk("mt_wakeup_cpu: not first boot!(%d)\n", cpu);
        mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);        
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
        *((unsigned int*)NS_SLAVE_BOOT_ADDR) = virt_to_phys(mt_secondary_startup);
#endif
        (*secure_ctrl_func[cpu-1])(STA_POWER_ON, 1);
    }
#endif
}

int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
    unsigned long timeout;

    printk(KERN_CRIT "Boot slave CPU\n");

    atomic_inc(&hotplug_cpu_count);

    /*
     * Set synchronisation state between this boot processor
     * and the secondary one
     */
    spin_lock(&boot_lock);

    HOTPLUG_INFO("boot_secondary, cpu: %d\n", cpu);
    /*
     * The secondary processor is waiting to be released from
     * the holding pen - release it, then wait for it to flag
     * that it has been released by resetting pen_release.
     *
     * Note that "pen_release" is the hardware CPU ID, whereas
     * "cpu" is Linux's internal ID.
     */
    /*
     * This is really belt and braces; we hold unintended secondary
     * CPUs in the holding pen until we're ready for them.  However,
     * since we haven't sent them a soft interrupt, they shouldn't
     * be there.
     */
    write_pen_release(cpu);

    switch(cpu)
    {
        case 1:
        case 2:
        case 3:
#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
            mt_wakeup_cpu(cpu);
#else //#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
            printk("mt_wakeup_cpu: not first boot!(%d)\n", cpu);
			//mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);
			// fixme, to replace SMC with parameter of ns_slave_boot_addr
            mt_secure_call(MC_FC_SET_RESET_VECTOR, virt_to_phys(mt_secondary_startup), cpu, 0);
			(*secure_ctrl_func[cpu-1])(STA_POWER_ON, 1);
#endif //#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
            break;
        default:
            break;

    }

#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
    smp_cross_call(cpumask_of(cpu));
#endif //#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)

    /*
     * Now the secondary core is starting up let it run its
     * calibrations, then wait for it to finish
     */
    spin_unlock(&boot_lock);

    timeout = jiffies + (1 * HZ);
    while (time_before(jiffies, timeout)) {
        smp_rmb();
        if (pen_release == -1)
            break;

        udelay(10);
    }

    if (pen_release == -1)
    {
        return 0;
    }
    else
    {
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
        //to fix monitor register
        printk(KERN_EMERG "failed to boot.\n");
        mt65xx_reg_sync_writel( 0x1, 0xF020021c);
        printk(KERN_EMERG "CPU2, debug event: 0x%08x, debug monitor: 0x%08x\n", *(volatile u32 *)(0xF020021c), *(volatile u32 *)(0xF0200014));
        mt65xx_reg_sync_writel( 0x3, 0xF020021c);
        printk(KERN_EMERG "CPU3, debug event: 0x%08x, debug monitor: 0x%08x\n", *(volatile u32 *)(0xF020021c), *(volatile u32 *)(0xF0200014));
        on_each_cpu((smp_call_func_t)dump_stack, NULL, 0);
#else
        mt65xx_reg_sync_writel(cpu + 8, 0xf0200080);
        printk(KERN_EMERG "CPU%u, debug event: 0x%08x, debug monitor: 0x%08x\n", cpu, *(volatile u32 *)(0xf0200080), *(volatile u32 *)(0xf0200084));
        on_each_cpu((smp_call_func_t)dump_stack, NULL, 0);
#endif
        atomic_dec(&hotplug_cpu_count);
        return -ENOSYS;
    }
}

void __init smp_init_cpus(void)
{
    unsigned int i, ncores;

    ncores = L2CTLR_get_core_count();
    if (ncores > NR_CPUS) {
        printk(KERN_WARNING
               "L2CTLR core count (%d) > NR_CPUS (%d)\n", ncores, NR_CPUS);
        printk(KERN_WARNING
               "set nr_cores to NR_CPUS (%d)\n", NR_CPUS);
        ncores = NR_CPUS;
    }

    if (ncores == 2)
    {
        spm_mtcmos_ctrl_cpu3(STA_POWER_DOWN, 0);
        spm_mtcmos_ctrl_cpu2(STA_POWER_DOWN, 0);
    }

    for (i = 0; i < ncores; i++)
        set_cpu_possible(i, true);

#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
    irq_total_secondary_cpus = num_possible_cpus() - 1;
    is_secondary_cpu_first_boot = num_possible_cpus() - 1;
#endif //#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)

    set_smp_cross_call(irq_raise_softirq);
    
}

#include "mach/mt_spm.h"
#define BOOTROM_PWR_CTRL        (INFRACFG_AO_BASE + 0x804)
#define reg_read(addr)          (*(volatile u32 *)(addr))
#define reg_write(addr, val)    mt65xx_reg_sync_writel(val, addr)

void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
    int i;

    for (i = 0; i < max_cpus; i++)
        set_cpu_present(i, true);

#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
    /* write the address of slave startup into the system-wide flags register */
    mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), SLAVE_JUMP_REG);
#else //#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
  
    //for (i = 1; i < max_cpus; i++)
		//(*secure_ctrl_func[i-1])(STA_POWER_DOWN, 1);
  
    //printk(KERN_CRIT "platform_smp_prepare_cpus !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); 
    
	//reg_write(BOOTROM_PWR_CTRL, reg_read(BOOTROM_PWR_CTRL) | 0x80000000);
    //BOOTROM_PWR_CTRL not accessible from NW...
    //mt_secure_call(MC_FC_BOOTROM_PWR_CTRL, 0, 0, 0);
#endif //#if !defined (CONFIG_TRUSTONIC_TEE_SUPPORT)
}
