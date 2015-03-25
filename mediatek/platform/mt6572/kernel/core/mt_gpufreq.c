/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/xlog.h>
#include <linux/jiffies.h>
#ifdef GPU_CLOCK_RATIO
#include <linux/time.h>
#endif

#include <asm/system.h>
#include <asm/uaccess.h>

#include "mach/mt_typedefs.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_gpufreq.h"
#include "mach/upmu_common.h"
#include "mach/sync_write.h"
#include "mach/mt_cpufreq.h"
#include "platform_pmm.h"
#include "devinfo.h"
/***************************
* debug message
****************************/
#define dprintk(fmt, args...)                                           \
do {                                                                    \
    if (mt_gpufreq_debug) {                                             \
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", fmt, ##args);   \
    }                                                                   \
} while(0)

#define ARRAY_AND_SIZE(x)   (x), ARRAY_SIZE(x)

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend mt_gpufreq_early_suspend_handler =
{
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 200,
    .suspend = NULL,
    .resume  = NULL,
};
#endif
/***************************
* MT6572 GPU Power Table
****************************/
static struct mt_gpufreq_power_info mt_gpufreqs_golden_power[] = {
    {.gpufreq_khz = GPU_DVFS_F0, .gpufreq_volt = GPU_VCORE_V0, .gpufreq_power = 326},
    {.gpufreq_khz = GPU_DVFS_F1, .gpufreq_volt = GPU_VCORE_V0, .gpufreq_power = 276},
    {.gpufreq_khz = GPU_DVFS_F0, .gpufreq_volt = GPU_VCORE_V1, .gpufreq_power = 274},
    {.gpufreq_khz = GPU_DVFS_F1, .gpufreq_volt = GPU_VCORE_V1, .gpufreq_power = 232},
    {.gpufreq_khz = GPU_DVFS_F2, .gpufreq_volt = GPU_VCORE_V0, .gpufreq_power = 206},
    {.gpufreq_khz = GPU_DVFS_F2, .gpufreq_volt = GPU_VCORE_V1, .gpufreq_power = 172},
};
//OPP setting
#define NORMAL_MAX_OPP 0
static const struct mt_gpufreq_info mt6572_gpufreq[] = {
    {.gpufreq_khz = GPU_DVFS_F0,
     .gpufreq_lower_bound = 30,
     .gpufreq_upper_bound = 100,
     .gpufreq_volt = GPU_VCORE_V1,
     .gpufreq_remap = 100,
    },
    {.gpufreq_khz = GPU_DVFS_F2,
     .gpufreq_lower_bound = 0,
     .gpufreq_upper_bound = 30,
     .gpufreq_volt = GPU_VCORE_V1,
     .gpufreq_remap = 60,
    },
};

static const struct mt_gpufreq_info mt6572m_gpufreq[] = {
    {.gpufreq_khz = GPU_DVFS_F1,
     .gpufreq_lower_bound = 0,
     .gpufreq_upper_bound = 100,
     .gpufreq_volt = GPU_VCORE_V1,
     .gpufreq_remap = 100,
    },
};

/**************************
* enable GPU DVFS count
***************************/
#ifdef CONFIG_GPU_DVFS
static int g_gpufreq_dvfs_disable_count = 0;
#endif
static unsigned int g_cur_freq = GPU_DVFS_F0;
static unsigned int g_cur_volt = GPU_VCORE_V1;

static unsigned int g_cur_load = 0;

/* In default settiing, freq_table[0] is max frequency, freq_table[num-1] is min frequency,*/
#ifdef CONFIG_GPU_DVFS
static unsigned int g_gpufreq_max_id = NORMAL_MAX_OPP;
#endif

/* If not limited, it should be set to freq_table[0] (MAX frequency) */
static unsigned int g_limited_max_id;
static unsigned int g_limited_min_id;

static bool mt_gpufreq_debug = false;
static bool mt_gpufreq_pause = false;
#ifdef CONFIG_GPU_DVFS
static bool mt_gpufreq_keep_max_frequency = false;
static bool mt_gpufreq_keep_specific_frequency = false;
static unsigned int mt_gpufreq_fixed_frequency = 0;
static unsigned int mt_gpufreq_fixed_voltage = 0;
#endif

static DEFINE_SPINLOCK(mt_gpufreq_lock);

static unsigned int mt_gpufreqs_num = 0;
static unsigned int mt_gpufreqs_power_num = 0;
static const struct mt_gpufreq_info *mt_gpufreqs;
static struct mt_gpufreq_power_info *mt_gpufreqs_power;
//static struct mt_gpufreq_power_info *mt_gpufreqs_default_power;

//static bool mt_gpufreq_registered = false;
//static bool mt_gpufreq_registered_statewrite = false;
//static bool mt_gpufreq_already_non_registered = false;

/*#ifdef GPU_CLOCK_RATIO
//static DEFINE_SPINLOCK(mt_gpufreq_load_lock);
//static struct mt_gpufreq_info *mt_gpufreqs_test;

#endif

static unsigned int mt_gpufreq_enable_mainpll = 0;
static unsigned int mt_gpufreq_enable_mmpll = 0;
*/
/******************************
* Extern Function Declaration
*******************************/
//extern int mtk_gpufreq_register(struct mt_gpufreq_power_info *freqs, int num);
/**************************
* GPU DVFS timer & thread
***************************/
static struct hrtimer mt_gpufreq_timer;
struct task_struct *mt_gpufreq_thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(mt_gpufreq_timer_waiter);

static int mt_gpufreq_sample_s = 0;
static int mt_gpufreq_sample_ns = 200000000; // 200ms
static int mt_gpufreq_timer_flag = 0;

/******************************
* Extern Function Declaration
*******************************/
extern int pmic_get_gpu_status_bit_info(void);
int mt_gpufreq_target(int idx);
/******************************
* Internal prototypes
*******************************/
enum hrtimer_restart mt_gpufreq_timer_func(struct hrtimer *timer);
static int mt_gpufreq_thread_handler(void *unused);
/**************************************
* check if maximum frequency is needed
***************************************/
#ifdef CONFIG_GPU_DVFS
static int mt_gpufreq_keep_max_freq(void)
{
    if (mt_gpufreq_keep_max_frequency == true)
        return 1;

    return 0;
}
#endif
//Todo: this function should be provided by ME team
unsigned int mt_get_gpu_loading(void)
{
#ifdef MTK_GPU_SUPPORT
    unsigned int loading = gpu_get_current_utilization();
#else
    unsigned int loading = 0;
#endif
    if(loading > 100){
        dprintk("loading > 100%: loading = %d\n", loading);
        return 100;
    }
    else{
        return loading;
    }
}

/*****************************************************************
* Check if gpufreq registration is done
*****************************************************************/
/*bool mt_gpufreq_is_registered_get(void)
{
    if((mt_gpufreq_registered == true) || (mt_gpufreq_already_non_registered == true))
        return true;
    else
        return false;
}
EXPORT_SYMBOL(mt_gpufreq_is_registered_get);
*/
/* Default power table when mt_gpufreq_non_register() */
/*static void mt_setup_gpufreqs_default_power_table(int num)
{
    int j = 0;

    mt_gpufreqs_default_power = kzalloc((1) * sizeof(struct mt_gpufreq_power_info), GFP_KERNEL);
    if (mt_gpufreqs_default_power == NULL)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "GPU default power table memory allocation fail\n");
        return;
    }

    mt_gpufreqs_default_power[0].gpufreq_khz = g_cur_freq;

    for (j = 0; j < ARRAY_SIZE(mt_gpufreqs_golden_power); j++)
    {
        if (g_cur_freq == mt_gpufreqs_golden_power[j].gpufreq_khz)
        {
            mt_gpufreqs_default_power[0].gpufreq_power = mt_gpufreqs_golden_power[j].gpufreq_power;
            break;
        }
    }

    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_default_power[0].gpufreq_khz = %u\n", mt_gpufreqs_default_power[0].gpufreq_khz);
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_default_power[0].gpufreq_power = %u\n", mt_gpufreqs_default_power[0].gpufreq_power);

    #ifdef CONFIG_THERMAL
    mtk_gpufreq_register(mt_gpufreqs_default_power, 1);
    #endif
}
*/
static void mt_setup_gpufreqs_power_table(int num)
{
    int i = 0, j = 0, power_idx = 0;

    mt_gpufreqs_power_num = num * VCORE_NUM;
    mt_gpufreqs_power = kzalloc((mt_gpufreqs_power_num) * sizeof(struct mt_gpufreq_power_info), GFP_KERNEL);

    if (mt_gpufreqs_power == NULL)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "GPU power table memory allocation fail\n");
        return ;
    }

    for(i = 0; i < ARRAY_SIZE(mt_gpufreqs_golden_power); i++){
        for(j = 0; j < num; j++){
            if(mt_gpufreqs_golden_power[i].gpufreq_khz == mt_gpufreqs[j].gpufreq_khz){
                memcpy(mt_gpufreqs_power+power_idx, mt_gpufreqs_golden_power+i, sizeof(struct mt_gpufreq_power_info));
                power_idx++;
            }
        }
    }

    for (i = 0; i < mt_gpufreqs_power_num; i++) {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_power[%d].gpufreq_khz = %u\n", i, mt_gpufreqs_power[i].gpufreq_khz);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_power[%d].gpufreq_volt = %u\n", i, mt_gpufreqs_power[i].gpufreq_volt);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_power[%d].gpufreq_power = %u\n", i, mt_gpufreqs_power[i].gpufreq_power);
    }
}
unsigned int get_gpu_level()
{
    unsigned int ver;
    ver = get_devinfo_with_index(GPU_LEVEL_INDEX);
    ver = (ver >> 27) & 0x00000001;
    return ver;
}
/***********************************************
* register frequency table to gpufreq subsystem
************************************************/
static int mt_setup_gpufreqs_table(const struct mt_gpufreq_info *gpufreqs, int num)
{
    // int i = 0; // <-XXX
    unsigned long flags;

    spin_lock_irqsave(&mt_gpufreq_lock, flags);
    mt_gpufreqs = gpufreqs;
    spin_unlock_irqrestore(&mt_gpufreq_lock, flags);

    mt_gpufreqs_num = num;

    if (mt_gpufreqs == NULL)
        return -ENOMEM;

    /* Initial frequency and voltage was already set in mt_gpufreq_set_initial() */
    g_limited_max_id = 0;
    g_limited_min_id = mt_gpufreqs_num - 1;

    // this api parameter should be changed according to efuse
    mt_setup_gpufreqs_power_table(num);

    return 0;
}

/*****************************
* set GPU DVFS status
******************************/
int mt_gpufreq_state_set(int enabled)
{
#ifdef CONFIG_GPU_DVFS
    ktime_t ktime = ktime_set(mt_gpufreq_sample_s, mt_gpufreq_sample_ns);

    if (enabled)
    {
        if (!mt_gpufreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "gpufreq already enabled\n");
            return 0;
        }

        /*****************
        * enable GPU DVFS
        ******************/
        g_gpufreq_dvfs_disable_count--;
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "enable GPU DVFS: g_gpufreq_dvfs_disable_count = %d\n", g_gpufreq_dvfs_disable_count);

        /***********************************************
        * enable DVFS if no any module still disable it
        ************************************************/
        if (g_gpufreq_dvfs_disable_count <= 0)
        {
            mt_gpufreq_pause = false;
            hrtimer_start(&mt_gpufreq_timer, ktime, HRTIMER_MODE_REL);

        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "someone still disable gpufreq, cannot enable it\n");
        }
    }
    else
    {
        /******************
        * disable GPU DVFS
        *******************/
        g_gpufreq_dvfs_disable_count++;
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "disable GPU DVFS: g_gpufreq_dvfs_disable_count = %d\n", g_gpufreq_dvfs_disable_count);

        if (mt_gpufreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "gpufreq already disabled\n");
            return 0;
        }
        mt_gpufreq_pause = true;
        mt_gpufreq_target(g_gpufreq_max_id);
        hrtimer_cancel(&mt_gpufreq_timer);
    }
#endif //CONFIG_GPU_DVFS
    return 0;
}
EXPORT_SYMBOL(mt_gpufreq_state_set);


/***********************************************************
* 1. 3D driver will check efuse and set initial frequency and voltage
* 2. When GPU idle in intial, voltage could be set directly.
************************************************************/
/*void mt_gpufreq_set_initial(unsigned int freq_new, unsigned int volt_new)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_set_initial, freq_new = %d, volt_new = %d, g_cur_freq = %d\n", freq_new, volt_new, g_cur_freq);
    g_freq_new_init_keep = freq_new;
    g_volt_new_init_keep = volt_new;

    mt_gpufreq_check_freq_and_set_pll();

    if(freq_new == g_cur_freq)
    {
        g_volt_set_init_step_1 = 1;
        return;
    }

    if (freq_new > g_cur_freq)
    {
        #ifdef MT_BUCK_ADJUST
        if (pmic_get_gpu_status_bit_info() == 0) // 1: VCORE, 0: VRF18_2
        {
            g_volt_set_init_step_2 = 1;
            mt_gpu_volt_switch_initial(volt_new);
            udelay(PMIC_SETTLE_TIME);
        }
        #endif

        mt_gpu_clock_switch(freq_new);
    }
    else
    {
        mt_gpu_clock_switch(freq_new);

        #ifdef MT_BUCK_ADJUST
        if (pmic_get_gpu_status_bit_info() == 0) // 1: VCORE, 0: VRF18_2
        {
            g_volt_set_init_step_3 = 1;
            mt_gpu_volt_switch_initial(volt_new);
        }
        #endif
    }

    g_cur_freq = freq_new;
    g_cur_volt = volt_new;
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_set_initial, g_cur_freq = %d, g_cur_volt = %d\n", g_cur_freq, g_cur_volt);
}
EXPORT_SYMBOL(mt_gpufreq_set_initial);
*/

/*static void mt_vcore_volt_set(unsigned int target_volt)
{
    switch (target_volt)
    {
      case GPU_POWER_VCORE_OD:
        dprintk("switch to GPU_POWER_VCORE_OD: %d mV\n", GPU_POWER_VCORE_OD);
        spm_dvfs_ctrl_volt(0);
        //udelay(PMIC_SETTLE_TIME);
        break;
      case GPU_POWER_VCORE_NORMAL:
        dprintk("switch to GPU_POWER_VCORE_NORMAL: %d mV\n", GPU_POWER_VCORE_NORMAL);
        spm_dvfs_ctrl_volt(2);
        //udelay(PMIC_SETTLE_TIME);
        break;
      default:
        break;
    }
}*/
/*****************************************
* frequency ramp up and ramp down handler
******************************************/
/***********************************************************
* [note]
* 1. frequency ramp up need to wait voltage settle
* 2. frequency ramp down do not need to wait voltage settle
************************************************************/
#ifdef CONFIG_GPU_DVFS
static void mt_gpufreq_set(unsigned int old_freq, unsigned int target_freq, unsigned int old_volt, unsigned int target_volt)
{
    unsigned int whpll_dds = 0, cur_vcore, new_vcore;

    cur_vcore = get_cur_vcore();
    new_vcore = get_max_vcore(mt_cpufreq_cur_volt(), target_volt);

    if( target_freq > PLL_MAX_FREQ || target_freq < PLL_MIN_FREQ){
        dprintk("set_whpll_freq: freq [%d] out of range\n", target_freq);
        return;
    }
    else
    {
        if (target_freq >= PLL_DIV1_FREQ)
        {
            whpll_dds = 0x8009A000;
            whpll_dds = whpll_dds + ((target_freq - PLL_DIV1_FREQ) / 13000) * 0x2000;
        }
        else if (target_freq >= PLL_DIV2_FREQ)
        {
            whpll_dds = 0x810A0000;
            whpll_dds = whpll_dds + ((target_freq - PLL_DIV2_FREQ) * 2 / 13000) * 0x2000;
        }
        else if (target_freq >= PLL_DIV4_FREQ)
        {
            whpll_dds = 0x820A0000;
            whpll_dds = whpll_dds + ((target_freq - PLL_DIV4_FREQ) * 4 / 13000) * 0x2000;
        }
        else if (target_freq >= PLL_DIV8_FREQ)
        {
            whpll_dds = 0x830A0000;
            whpll_dds = whpll_dds + ((target_freq - PLL_DIV8_FREQ) * 8 / 13000) * 0x2000;
        }
        else
        {
            dprintk("set_whpll_freq: unsupport frequency!!\n");
        }
    }

    if(old_freq >= target_freq){
        if(cur_vcore != new_vcore) {
            mt_vcore_freq_volt_set(target_volt, 0);
        }
        // pll_fsel includes PMIC + PLL settle time
        pll_fsel(WHPLL, whpll_dds);
    }
    else{
        // pll_fsel includes PMIC + PLL settle time
        pll_fsel(WHPLL, whpll_dds);
        if(cur_vcore != new_vcore){
            mt_vcore_freq_volt_set(target_volt, 0);
            udelay(PMIC_SETTLE_TIME);
        }
    }
    dprintk("GPU freq: %d\n", GPU_freq_output());
    g_cur_freq = target_freq;
    g_cur_volt = target_volt;
}
#endif /* CONFIG_GPU_DVFS */

static int mt_gpufreq_look_up(unsigned int load)
{
    int i = 0, remap = 100;

    /**************************
    * look up the remap value
    ***************************/
    for (i = 0; i < mt_gpufreqs_num; i++)
    {
        if (mt_gpufreqs[i].gpufreq_khz == g_cur_freq)
        {
            remap = mt_gpufreqs[i].gpufreq_remap;
            break;
        }
    }

    load = (load * remap) / 100;
    g_cur_load = load;
    dprintk("GPU Loading = %d\n", load);

    /******************************
    * look up the target frequency
    *******************************/
    for (i = 0; i < mt_gpufreqs_num; i++)
    {
        if (load > mt_gpufreqs[i].gpufreq_lower_bound && load <= mt_gpufreqs[i].gpufreq_upper_bound)
        {
            return i;
        }
    }

    return (mt_gpufreqs_num - 1);
}

/**********************************
* gpufreq target callback function
***********************************/
/*************************************************
* [note]
* 1. handle frequency change request
* 2. call mt_gpufreq_set to set target frequency
**************************************************/
int mt_gpufreq_target(int idx)
{
#ifdef CONFIG_GPU_DVFS
    unsigned long flags, target_freq, target_volt;

    spin_lock_irqsave(&mt_gpufreq_lock, flags);
    if (mt_gpufreqs == NULL)
    {
        spin_unlock_irqrestore(&mt_gpufreq_lock, flags);
        return -1;
    }

    /**********************************
    * look up for the target GPU OPP
    ***********************************/
    target_freq = mt_gpufreqs[idx].gpufreq_khz;
    target_volt = mt_gpufreqs[idx].gpufreq_volt;


    /************************************************
    * If /proc command fix the frequency.
    *************************************************/
    if(mt_gpufreq_keep_specific_frequency == true)
    {
        target_freq = mt_gpufreq_fixed_frequency;
        target_volt = mt_gpufreq_fixed_voltage;
        dprintk("Fixed! fixed frequency %d, fixed voltage %d\n", target_freq, target_volt);
    }
    else {
        /**********************************
         * Check if need to keep max frequency
         ***********************************/
        if (mt_gpufreq_keep_max_freq())
        {
            target_freq = mt_gpufreqs[g_gpufreq_max_id].gpufreq_khz;
            target_volt = mt_gpufreqs[g_gpufreq_max_id].gpufreq_volt;
            dprintk("Keep MAX frequency %d !\n", target_freq);
        }

        /****************************************************
         * If need to raise frequency, and under normal OPP. Raise to max frequency
         *****************************************************/
        if(target_freq > g_cur_freq && target_freq < mt_gpufreqs[g_gpufreq_max_id].gpufreq_khz)
        {
            target_freq = mt_gpufreqs[g_gpufreq_max_id].gpufreq_khz;
            target_volt = mt_gpufreqs[g_gpufreq_max_id].gpufreq_volt;
            dprintk("Need to raise frequency, raise to MAX frequency %d !\n", target_freq);
        }

        if (target_freq > mt_gpufreqs[g_limited_max_id].gpufreq_khz)
        {
            /*********************************************
             * target_freq > limited_freq, need to adjust
             **********************************************/
            target_freq = mt_gpufreqs[g_limited_max_id].gpufreq_khz;
            target_volt = mt_gpufreqs[g_limited_max_id].gpufreq_volt;
            dprintk("Limit! Target freq %d > Thermal limit frequency %d\n", target_freq, mt_gpufreqs[g_limited_max_id].gpufreq_khz);
        }
    }


    /************************************************
    * target frequency == current frequency, skip it
    *************************************************/
    if (g_cur_freq == target_freq)
    {
        spin_unlock_irqrestore(&mt_gpufreq_lock, flags);
        dprintk("GPU frequency from %d MHz to %d MHz (skipped) due to same frequency\n", g_cur_freq / 1000, target_freq / 1000);
        return 0;
    }

    dprintk("GPU current frequency %d MHz, target frequency %d MHz\n", g_cur_freq / 1000, target_freq / 1000);

    /******************************
    * set to the target freeuency
    *******************************/
    mt_gpufreq_set(g_cur_freq, target_freq, g_cur_volt, target_volt);

    spin_unlock_irqrestore(&mt_gpufreq_lock, flags);
#endif /* CONFIG_GPU_DVFS */
    return 0;
}

/*********************************
* early suspend callback function
**********************************/
void mt_gpufreq_early_suspend(struct early_suspend *h)
{
    mt_gpufreq_state_set(0);
}

/*******************************
* late resume callback function
********************************/
void mt_gpufreq_late_resume(struct early_suspend *h)
{
    mt_gpufreq_state_set(1);
}

/************************************************
* frequency adjust interface for thermal protect
*************************************************/
/******************************************************
* parameter: target power
*******************************************************/
void mt_gpufreq_thermal_protect(unsigned int limited_power)
{
#ifdef CONFIG_GPU_DVFS
    int i, j;
    //unsigned int limited_freq = 0;

    if (mt_gpufreqs_num == 0)
        return;

    if (limited_power == 0)
    {
        g_limited_max_id = 0;
    }
    else
    {
        for (i = 0; i < mt_gpufreqs_power_num; i++)
        {
            if( (mt_gpufreqs_power[i].gpufreq_volt == get_cur_vcore()) && (mt_gpufreqs_power[i].gpufreq_power <= limited_power))
            {
                //limited_freq = mt_gpufreqs[i].gpufreq_khz;
                dprintk("power table: %d \n", mt_gpufreqs_power[i].gpufreq_power);

                for(j = 0; j < mt_gpufreqs_num; j++){
                    if(mt_gpufreqs[j].gpufreq_khz == mt_gpufreqs_power[i].gpufreq_khz){
                        g_limited_max_id = j;
                        dprintk("[thermal] frequency upperbound = %d\n", mt_gpufreqs[g_limited_max_id].gpufreq_khz);
                        mt_gpufreq_target(g_limited_max_id);
                        return ;
                    }
                }
            }
        }

        g_limited_max_id = mt_gpufreqs_num - 1;
        dprintk("fail to set thermal throttling for GPU: %u\n", limited_power);
        dprintk("GPU cap to minimum freq\n");
        //mt_gpufreq_target(g_limited_max_id);
    }
#endif //CONFIG_GPU_DVFS
    return ;
}
EXPORT_SYMBOL(mt_gpufreq_thermal_protect);
/************************************************
* return current GPU frequency
*************************************************/
unsigned int mt_gpufreq_max_power(void)
{
    int i;
#ifdef CONFIG_GPU_DVFS
    for(i = 0; i < mt_gpufreqs_power_num ; i++){
        if(mt_gpufreqs_power[i].gpufreq_volt == get_cur_vcore()){
            return mt_gpufreqs_power[i].gpufreq_power;
        }
    }
    dprintk("Fail to get GPU MAX power!\n");
    return mt_gpufreqs_power[0].gpufreq_power;
#else
    printk(KERN_WARNING "GPU DVFS is NOT enabled\n");
    for(i = 0; i < ARRAY_SIZE(mt_gpufreqs_golden_power); i++){
        if(mt_gpufreqs_golden_power[i].gpufreq_volt == get_cur_vcore() && \
           mt_gpufreqs_golden_power[i].gpufreq_khz == GPU_DVFS_F1){
            return mt_gpufreqs_golden_power[i].gpufreq_power;
        }
    }

    dprintk("Fail to get GPU MAX power!\n");
    return mt_gpufreqs_golden_power[0].gpufreq_power;
#endif
}

unsigned int mt_gpufreq_min_power(void)
{
    int i;
#ifdef CONFIG_GPU_DVFS
    for(i = mt_gpufreqs_power_num-1; i >=0 ; i--){
        if(mt_gpufreqs_power[i].gpufreq_volt == get_cur_vcore()){
            return mt_gpufreqs_power[i].gpufreq_power;
        }
    }
    dprintk("Fail to get GPU MIN power!\n");
    return mt_gpufreqs_power[mt_gpufreqs_power_num-1].gpufreq_power;
#else
    printk(KERN_WARNING "GPU DVFS is NOT enabled\n");
    for(i = 0; i < ARRAY_SIZE(mt_gpufreqs_golden_power); i++){
        if(mt_gpufreqs_golden_power[i].gpufreq_volt == get_cur_vcore() && \
           mt_gpufreqs_golden_power[i].gpufreq_khz == GPU_DVFS_F1){
            return mt_gpufreqs_golden_power[i].gpufreq_power;
        }
    }

    dprintk("Fail to get GPU MIN power!\n");
    return mt_gpufreqs_golden_power[0].gpufreq_power;
#endif
}

unsigned int mt_gpufreq_cur_volt(void)
{
    dprintk("Current GPU volt setting is %d MV\n", g_cur_volt);
    return g_cur_volt;
}
EXPORT_SYMBOL(mt_gpufreq_cur_volt);
/************************************************
* return current GPU frequency
*************************************************/
unsigned int mt_gpufreq_cur_freq(void)
{
    dprintk("current GPU frequency is %d MHz\n", g_cur_freq / 1000);
    return g_cur_freq;
}
EXPORT_SYMBOL(mt_gpufreq_cur_freq);

/************************************************
* return current GPU loading
*************************************************/
unsigned int mt_gpufreq_cur_load(void)
{
    dprintk("current GPU load is %d\n", g_cur_load);
    return g_cur_load;
}
EXPORT_SYMBOL(mt_gpufreq_cur_load);

/******************************
* show current GPU DVFS stauts
*******************************/
#ifdef CONFIG_GPU_DVFS
static int mt_gpufreq_state_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (!mt_gpufreq_pause)
        p += sprintf(p, "GPU DVFS enabled\n");
    else
        p += sprintf(p, "GPU DVFS disabled\n");

    len = p - buf;
    return len;
}

/****************************************
* set GPU DVFS stauts by sysfs interface
*****************************************/
static ssize_t mt_gpufreq_state_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if ((sscanf(buffer, "%d", &enabled) == 1) &&  (GPU_LEVEL_0 != get_gpu_level()))
    {
        if (enabled == 1)
        {
            mt_gpufreq_keep_max_frequency = false;
            mt_gpufreq_state_set(1);
        }
        else if (enabled == 0)
        {
            /* Keep MAX frequency when GPU DVFS disabled. */
            mt_gpufreq_keep_max_frequency = true;
            mt_gpufreq_state_set(0);
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
        }
    }
    else
    {
        if(GPU_LEVEL_0 == get_gpu_level()){
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "Not Support \n");
        }
        else
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
    }

    return count;
}

/****************************
* show current limited power
*****************************/
static int mt_gpufreq_limited_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "g_limited_max_id = %d, frequency = %d\n", g_limited_max_id, mt_gpufreqs[g_limited_max_id].gpufreq_khz);

    len = p - buf;
    return len;
}

/**********************************
* limited power for thermal protect
***********************************/
static ssize_t mt_gpufreq_limited_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int power = 0;

    if (sscanf(buffer, "%u", &power) == 1)
    {
        mt_gpufreq_thermal_protect(power);
        return count;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! please provide the maximum limited power\n");
    }

    return -EINVAL;
}

/***************************
* show current debug status
****************************/
static int mt_gpufreq_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (mt_gpufreq_debug)
        p += sprintf(p, "gpufreq debug enabled\n");
    else
        p += sprintf(p, "gpufreq debug disabled\n");

    len = p - buf;
    return len;
}

/***********************
* enable debug message
************************/
static ssize_t mt_gpufreq_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int debug = 0;

    if (sscanf(buffer, "%d", &debug) == 1)
    {
        if (debug == 0)
        {
            mt_gpufreq_debug = 0;
            return count;
        }
        else if (debug == 1)
        {
            mt_gpufreq_debug = 1;
            return count;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
    }

    return -EINVAL;
}

/********************
* show sampling rate
*********************/
static int mt_gpufreq_sampling_rate_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "get sampling rate = %d (s) %d (ns)\n", mt_gpufreq_sample_s, mt_gpufreq_sample_ns);

    len = p - buf;
    return len;
}

/********************
* set sampling rate
*********************/
static ssize_t mt_gpufreq_sampling_rate_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int len = 0, s = 0, ns = 0;
    char desc[32];

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
    if (copy_from_user(desc, buffer, len))
    {
        return 0;
    }
    desc[len] = '\0';

    if (sscanf(desc, "%d %d", &s, &ns) == 2)
    {
        printk("[%s]: set sampling rate = %d (s), %d (ns)\n", __FUNCTION__, s, ns);
        mt_gpufreq_sample_s = s;
        mt_gpufreq_sample_ns = ns;
        return count;
    }
    else
    {
        printk("[%s]: bad argument!! should be \"[s]\" or \"[ns]\"\n", __FUNCTION__);
    }

    return -EINVAL;
}

/********************
* show GPU OPP table
*********************/
static int mt_gpufreq_opp_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int i = 0, /* j = 0, */ len = 0; // <-XXX
    char *p = buf;

    p += sprintf(p, "Current Freq: %d\n", g_cur_freq);
    p += sprintf(p, "Current Loading: %d\n", mt_get_gpu_loading());
    for (i = 0; i < mt_gpufreqs_num; i++)
    {
        p += sprintf(p, "[%d] ", i);
        p += sprintf(p, "freq = %d, ", mt_gpufreqs[i].gpufreq_khz);
        p += sprintf(p, "lower_bound = %d, ", mt_gpufreqs[i].gpufreq_lower_bound);
        p += sprintf(p, "upper_bound = %d, ", mt_gpufreqs[i].gpufreq_upper_bound);
        p += sprintf(p, "volt = %d, ", mt_gpufreqs[i].gpufreq_volt);
        p += sprintf(p, "remap = %d\n", mt_gpufreqs[i].gpufreq_remap);
    }

    len = p - buf;
    return len;
}

/***************************
* show current specific frequency status
****************************/
static int mt_gpufreq_fixed_frequency_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (mt_gpufreq_keep_specific_frequency)
        p += sprintf(p, "gpufreq fixed frequency enabled\n");
    else
        p += sprintf(p, "gpufreq fixed frequency disabled\n");

    len = p - buf;
    return len;
}

/***********************
* enable specific frequency
************************/
static ssize_t mt_gpufreq_fixed_frequency_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enable = 0;
    int fixed_freq = 0;
    int fixed_volt = 0;

    if (sscanf(buffer, "%d %d %d", &enable, &fixed_freq, &fixed_volt) == 3)
    {
        if (enable == 0)
        {
            mt_gpufreq_keep_specific_frequency = false;
            return count;
        }
        else if (enable == 1)
        {
            mt_gpufreq_keep_specific_frequency = true;
            mt_gpufreq_fixed_frequency = fixed_freq;
            mt_gpufreq_fixed_voltage = fixed_volt;
            mt_gpufreq_set(g_cur_freq, mt_gpufreq_fixed_frequency, g_cur_volt, mt_gpufreq_fixed_voltage);
            return count;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! should be [enable fixed_freq fixed_volt]\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! should be [enable fixed_freq fixed_volt]\n");
    }

    return -EINVAL;
}

/********************
* show variable dump
*********************/
static int mt_gpufreq_var_dump(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;


    //debug purpose, I will implement if needed

    len = p - buf;
    return len;
}
#endif /* CONFIG_GPU_DVFS */

enum hrtimer_restart mt_gpufreq_timer_func(struct hrtimer *timer)
{
    mt_gpufreq_timer_flag = 1; wake_up_interruptible(&mt_gpufreq_timer_waiter);

    return HRTIMER_NORESTART;
}

static int mt_gpufreq_thread_handler(void *unused)
{
    int idx = 0;
    unsigned int load = 0;

    do
    {
        ktime_t ktime = ktime_set(mt_gpufreq_sample_s, mt_gpufreq_sample_ns);

        wait_event_interruptible(mt_gpufreq_timer_waiter, mt_gpufreq_timer_flag != 0);
        mt_gpufreq_timer_flag = 0;

        /**********************************
        * get GPU loading
        ***********************************/
        load = mt_get_gpu_loading();

        idx = mt_gpufreq_look_up(load);

        mt_gpufreq_target(idx);

        if(mt_gpufreq_pause == false)
            hrtimer_start(&mt_gpufreq_timer, ktime, HRTIMER_MODE_REL);

    } while (!kthread_should_stop());

    return 0;
}

/*********************************
* mediatek gpufreq registration
**********************************/
int mt_gpufreq_register(void)
{
    unsigned int ver;
    ktime_t ktime = ktime_set(mt_gpufreq_sample_s, mt_gpufreq_sample_ns);

    //mt_gpufreq_registered = true;


    /**********************
     * setup gpufreq table
     ***********************/
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "setup gpufreqs table\n");

    ver = get_gpu_level();

    if(ver == GPU_LEVEL_0){
        mt_setup_gpufreqs_table(ARRAY_AND_SIZE(mt6572m_gpufreq));
        g_cur_freq = GPU_DVFS_F1;
        mt_gpufreq_pause = true;
    }
    else{

#ifdef CONFIG_HAS_EARLYSUSPEND
        mt_gpufreq_early_suspend_handler.suspend = mt_gpufreq_early_suspend;
        mt_gpufreq_early_suspend_handler.resume = mt_gpufreq_late_resume;
        register_early_suspend(&mt_gpufreq_early_suspend_handler);
#endif
        mt_setup_gpufreqs_table(ARRAY_AND_SIZE(mt6572_gpufreq));
        g_cur_freq = GPU_DVFS_F0;
        /************************************
         * launch a timer for period sampling
         *************************************/
        hrtimer_init(&mt_gpufreq_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        mt_gpufreq_timer.function = mt_gpufreq_timer_func;

        mt_gpufreq_thread = kthread_run(mt_gpufreq_thread_handler, 0, "mt_gpufreq");
        if (IS_ERR(mt_gpufreq_thread))
        {
            printk("[%s]: failed to create mt_gpufreq thread\n", __FUNCTION__);
        }

#ifdef GPU_DVFS_DEFAULT_DISABLED
        mt_gpufreq_state_set(0);
#endif

        if(mt_gpufreq_pause == false)
        {
            hrtimer_start(&mt_gpufreq_timer, ktime, HRTIMER_MODE_REL);
        }

        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mediatek gpufreq registration done\n");
    }

    return 0;
}
EXPORT_SYMBOL(mt_gpufreq_register);

/*********************************
* mediatek gpufreq non registration
**********************************/
/*int mt_gpufreq_non_register(void)
{
    if(mt_gpufreq_already_non_registered == false)
    {
        mt_gpufreq_already_non_registered = true;
        mt_setup_gpufreqs_default_power_table(1);

        hrtimer_init(&mt_gpufreq_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        mt_gpufreq_state_set(0);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_non_register() done\n");
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_non_register() already called !\n");
    }
    return 0;
}
EXPORT_SYMBOL(mt_gpufreq_non_register);*/

/**********************************
* mediatek gpufreq initialization
***********************************/
#ifdef CONFIG_GPU_DVFS
static int __init mt_gpufreq_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;
    struct proc_dir_entry *mt_gpufreq_dir = NULL;

    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_init\n");

    mt_gpufreq_dir = proc_mkdir("gpufreq", NULL);
    if (!mt_gpufreq_dir)
    {
        pr_err("[%s]: mkdir /proc/gpufreq failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("gpufreq_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_debug_read;  //OK
            mt_entry->write_proc = mt_gpufreq_debug_write;
        }

        mt_entry = create_proc_entry("gpufreq_limited_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_limited_power_read;   //OK
            mt_entry->write_proc = mt_gpufreq_limited_power_write;
        }

        mt_entry = create_proc_entry("gpufreq_state", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_state_read;  //OK
            mt_entry->write_proc = mt_gpufreq_state_write;
        }

        mt_entry = create_proc_entry("gpufreq_sampling_rate", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_sampling_rate_read;   //OK
            mt_entry->write_proc = mt_gpufreq_sampling_rate_write;
        }

        mt_entry = create_proc_entry("gpufreq_opp_dump", S_IRUGO, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_opp_dump_read;   //OK
        }

        mt_entry = create_proc_entry("gpufreq_fix_frequency", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_fixed_frequency_read;   //OK
            mt_entry->write_proc = mt_gpufreq_fixed_frequency_write;
        }

        mt_entry = create_proc_entry("gpufreq_var_dump", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_var_dump;   //OK
        }
    }

    //clkmux_sel(MT_CLKMUX_MFG_MUX_SEL, MT_CG_GPU_500P5M_EN, "GPU_DVFS");

    g_cur_freq = GPU_DVFS_F0;
    g_cur_volt = get_cur_vcore();

    mt_gpufreq_register(); // activate hrtimer

    //xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_init, g_cur_freq_init_keep = %d\n", g_cur_freq_init_keep);

    return 0;
}

late_initcall(mt_gpufreq_init);
#endif //CONFIG_GPU_DVFS

MODULE_DESCRIPTION("MediaTek GPU Frequency Scaling driver");
MODULE_LICENSE("GPL");
