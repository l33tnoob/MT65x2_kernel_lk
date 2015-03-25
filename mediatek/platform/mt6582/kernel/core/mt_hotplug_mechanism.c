/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*********************************
* include
**********************************/
#include <linux/kernel.h> 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/earlysuspend.h>
#include <linux/proc_fs.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <mach/hotplug.h>
#include <mach/sync_write.h>



/*********************************
* macro
**********************************/
#ifdef CONFIG_HAS_EARLYSUSPEND
#define STATE_INIT                          0
#define STATE_ENTER_EARLY_SUSPEND           1
#define STATE_ENTER_LATE_RESUME             2
#endif //#ifdef CONFIG_HAS_EARLYSUSPEND

#define FORCE_CPU_OFF_DELAYED_WORK_TIME     3 //second
#define FORCE_CPU_OFF_WAKE_LOCK_TIME        5 //second



/*********************************
* glabal variable
**********************************/
static int g_enable = 1;

#ifdef CONFIG_HAS_EARLYSUSPEND
static int g_enable_cpu_rush_boost = 0;
static int g_prev_cpu_rush_boost_enable = 0;

static struct early_suspend mt_hotplug_mechanism_early_suspend_handler =
{
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 250,
    .suspend = NULL,
    .resume  = NULL,
};
static int g_cur_state = STATE_ENTER_LATE_RESUME;
#endif //#ifdef CONFIG_HAS_EARLYSUSPEND

static int g_enable_dynamic_cpu_hotplug_at_suspend = 0;
static int g_prev_dynamic_cpu_hotplug_enable = 0;
static int mt_hotplug_mechanism_probe(struct platform_device *pdev);
static int mt_hotplug_mechanism_suspend(struct platform_device *pdev, pm_message_t state);
static int mt_hotplug_mechanism_resume(struct platform_device *pdev);
static struct platform_driver mt_hotplug_mechanism_pdrv =
{
    .remove     = NULL,
    .shutdown   = NULL,
    .probe      = mt_hotplug_mechanism_probe,
    .suspend    = mt_hotplug_mechanism_suspend,
    .resume     = mt_hotplug_mechanism_resume,
    .driver     = {
        .name = "mt_cpu_hotplug",
    },
};

static int g_test0 = 0;
static int g_test1 = 0;



/*********************************
* extern function
**********************************/
#ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
extern int hp_get_cpu_rush_boost_enable(void);
extern void hp_set_cpu_rush_boost_enable(int enable);
extern int hp_get_dynamic_cpu_hotplug_enable(void);
extern void hp_set_dynamic_cpu_hotplug_enable(int enable);
#endif



/*********************************
* early suspend callback function
**********************************/
#ifdef CONFIG_HAS_EARLYSUSPEND
static void mt_hotplug_mechanism_early_suspend(struct early_suspend *h)
{
    HOTPLUG_INFO("mt_hotplug_mechanism_early_suspend");

    if (!g_enable)
        goto early_suspend_end;
    
    if (!g_enable_cpu_rush_boost)
    {
    #ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
        g_prev_cpu_rush_boost_enable = hp_get_cpu_rush_boost_enable();
        hp_set_cpu_rush_boost_enable(0);
    #endif //#ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
    }
    
early_suspend_end:
    g_cur_state = STATE_ENTER_EARLY_SUSPEND;

    return;
}
#endif //#ifdef CONFIG_HAS_EARLYSUSPEND



/*******************************
* late resume callback function
********************************/
#ifdef CONFIG_HAS_EARLYSUSPEND
static void mt_hotplug_mechanism_late_resume(struct early_suspend *h)
{
    HOTPLUG_INFO("mt_hotplug_mechanism_late_resume");

    if (!g_enable)
        goto late_resume_end;
    
    if (!g_enable_cpu_rush_boost)
    {
    #ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
        hp_set_cpu_rush_boost_enable(g_prev_cpu_rush_boost_enable);
    #endif //#ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
    }
    
late_resume_end:
    g_cur_state = STATE_ENTER_LATE_RESUME;

    return;
}
#endif //#ifdef CONFIG_HAS_EARLYSUSPEND



/*******************************
* probe callback function
********************************/
static int mt_hotplug_mechanism_probe(struct platform_device *pdev)
{
    HOTPLUG_INFO("mt_hotplug_mechanism_probe\n");
    
    return 0;
}



/*******************************
* suspend callback function
********************************/
static int mt_hotplug_mechanism_suspend(struct platform_device *pdev, pm_message_t state)
{
    HOTPLUG_INFO("mt_hotplug_mechanism_suspend\n");
    
    if (!g_enable)
        return 0;
    
    if (!g_enable_dynamic_cpu_hotplug_at_suspend)
    {
    #ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
        g_prev_dynamic_cpu_hotplug_enable = hp_get_dynamic_cpu_hotplug_enable();
        hp_set_dynamic_cpu_hotplug_enable(0);
    #endif //#ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
    }
    
    return 0;
}



/*******************************
* resume callback function
********************************/
static int mt_hotplug_mechanism_resume(struct platform_device *pdev)
{
    HOTPLUG_INFO("mt_hotplug_mechanism_resume\n");
    
    if (!g_enable)
        return 0;
    
    if (!g_enable_dynamic_cpu_hotplug_at_suspend)
    {
    #ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
        hp_set_dynamic_cpu_hotplug_enable(g_prev_dynamic_cpu_hotplug_enable);
    #endif //#ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
    }
    
    return 0;
}



/**************************************************************
* mt hotplug mechanism control interface for procfs test0
***************************************************************/
static int mt_hotplug_mechanism_read_test0(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    char *p = buf;
    
    p += sprintf(p, "%d\n", g_test0);
    *eof = 1;
    
    HOTPLUG_INFO("mt_hotplug_mechanism_read_test0, hotplug_cpu_count: %d\n", atomic_read(&hotplug_cpu_count));
    on_each_cpu((smp_call_func_t)dump_stack, NULL, 1);
    
    mt65xx_reg_sync_writel(8, 0xf0200080);
    printk(KERN_EMERG "CPU%u, debug event: 0x%08x, debug monitor: 0x%08x\n", 0, *(volatile u32 *)(0xf0200080), *(volatile u32 *)(0xf0200084));
    mt65xx_reg_sync_writel(9, 0xf0200080);
    printk(KERN_EMERG "CPU%u, debug event: 0x%08x, debug monitor: 0x%08x\n", 1, *(volatile u32 *)(0xf0200080), *(volatile u32 *)(0xf0200084));
    mt65xx_reg_sync_writel(10, 0xf0200080);
    printk(KERN_EMERG "CPU%u, debug event: 0x%08x, debug monitor: 0x%08x\n", 2, *(volatile u32 *)(0xf0200080), *(volatile u32 *)(0xf0200084));
    mt65xx_reg_sync_writel(11, 0xf0200080);
    printk(KERN_EMERG "CPU%u, debug event: 0x%08x, debug monitor: 0x%08x\n", 3, *(volatile u32 *)(0xf0200080), *(volatile u32 *)(0xf0200084));
    
    return p - buf;
}

static int mt_hotplug_mechanism_write_test0(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int len = 0, test0 = 0;
    char desc[32];
    
    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
    if (copy_from_user(desc, buffer, len))
    {
        return 0;
    }
    desc[len] = '\0';
    
    if (sscanf(desc, "%d", &test0) == 1)
    {
        g_test0 = test0;
        return count;
    }
    else
    {
        HOTPLUG_INFO("mt_hotplug_mechanism_write_test0, bad argument\n");
    }
    
    return -EINVAL;
}



/**************************************************************
* mt hotplug mechanism control interface for procfs test1
***************************************************************/
extern int g_cpus_sum_load_current;   //set global for information purpose

extern long g_cpu_up_sum_load;
extern int g_cpu_up_count;
extern int g_cpu_up_load_index;

extern long g_cpu_down_sum_load;
extern int g_cpu_down_count;
extern int g_cpu_down_load_index;

typedef enum {
    CPU_HOTPLUG_WORK_TYPE_NONE = 0,
    CPU_HOTPLUG_WORK_TYPE_BASE,
    CPU_HOTPLUG_WORK_TYPE_LIMIT,
    CPU_HOTPLUG_WORK_TYPE_UP,
    CPU_HOTPLUG_WORK_TYPE_DOWN,
    CPU_HOTPLUG_WORK_TYPE_RUSH,
} cpu_hotplug_work_type_t;
extern cpu_hotplug_work_type_t g_trigger_hp_work;

extern int g_tlp_avg_current;       //set global for information purpose
extern int g_tlp_avg_sum;
extern int g_tlp_avg_count;
extern int g_tlp_avg_index;
extern int g_tlp_avg_average; 

static int mt_hotplug_mechanism_read_test1(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    char *p = buf;
    
    p += sprintf(p, "%d\n", g_test1);
    *eof = 1;
    
    HOTPLUG_INFO("[power/hotplug] thermal_debug_1 (%d)(%d)(%d)(%d)(%ld)(%ld)\n", g_trigger_hp_work, g_tlp_avg_average, g_tlp_avg_current,
        g_cpus_sum_load_current, g_cpu_up_sum_load, g_cpu_down_sum_load);
    HOTPLUG_INFO("[power/hotplug] thermal_debug_2 (%d)(%d)(%d)(%d)\n", g_cpu_up_count, g_cpu_up_load_index, g_cpu_down_count, g_cpu_down_load_index);
	
    return p - buf;
}

static int mt_hotplug_mechanism_write_test1(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int len = 0, test1 = 0;
    char desc[32];
    
    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
    if (copy_from_user(desc, buffer, len))
    {
        return 0;
    }
    desc[len] = '\0';
    
    if (sscanf(desc, "%d", &test1) == 1)
    {
        g_test1 = test1;
        return count;
    }
    else
    {
        HOTPLUG_INFO("mt_hotplug_mechanism_write_test1, bad argument\n");
    }
    
    return -EINVAL;
}



/*******************************
* kernel module init function
********************************/
static int __init mt_hotplug_mechanism_init(void)
{
    struct proc_dir_entry *entry = NULL;
    struct proc_dir_entry *mt_hotplug_dir = NULL;
    int r = 0;
    
    HOTPLUG_INFO("mt_hotplug_mechanism_init");
    
    mt_hotplug_dir = proc_mkdir("mt_hotplug", NULL);
    if (!mt_hotplug_dir)
    {
        HOTPLUG_INFO("mkdir /proc/mt_hotplug failed");
    }
    else
    {
        entry = create_proc_entry("test0", S_IRUGO | S_IWUSR, mt_hotplug_dir);
        if (entry)
        {
            entry->read_proc = mt_hotplug_mechanism_read_test0;
            entry->write_proc = mt_hotplug_mechanism_write_test0;
        }
        entry = create_proc_entry("test1", S_IRUGO | S_IWUSR, mt_hotplug_dir);
        if (entry)
        {
            entry->read_proc = mt_hotplug_mechanism_read_test1;
            entry->write_proc = mt_hotplug_mechanism_write_test1;
        }
    }
    
#ifdef CONFIG_HAS_EARLYSUSPEND
    mt_hotplug_mechanism_early_suspend_handler.suspend = mt_hotplug_mechanism_early_suspend;
    mt_hotplug_mechanism_early_suspend_handler.resume = mt_hotplug_mechanism_late_resume;
    register_early_suspend(&mt_hotplug_mechanism_early_suspend_handler);
#endif //#ifdef CONFIG_HAS_EARLYSUSPEND

    r = platform_driver_register(&mt_hotplug_mechanism_pdrv);
    if (r)
        HOTPLUG_INFO("platform_driver_register failed (%d)\n", r);
    
    return r;
}
module_init(mt_hotplug_mechanism_init);



/*******************************
* kernel module exit function
********************************/
static void __exit mt_hotplug_mechanism_exit(void)
{
    HOTPLUG_INFO("mt_hotplug_mechanism_exit");
}
module_exit(mt_hotplug_mechanism_exit);



/**************************************************************
* mt hotplug mechanism control interface for thermal protect
***************************************************************/
void mt_hotplug_mechanism_thermal_protect(int limited_cpus)
{
    HOTPLUG_INFO("mt_hotplug_mechanism_thermal_protect\n");

}
EXPORT_SYMBOL(mt_hotplug_mechanism_thermal_protect);



module_param(g_enable, int, 0644);
#ifdef CONFIG_HAS_EARLYSUSPEND
module_param(g_enable_cpu_rush_boost, int, 0644);
#endif //#ifdef CONFIG_HAS_EARLYSUSPEND
module_param(g_enable_dynamic_cpu_hotplug_at_suspend, int, 0644);



MODULE_DESCRIPTION("MediaTek CPU Hotplug Mechanism");
MODULE_LICENSE("GPL");
