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
#include <linux/cpu.h>
#include <linux/cpufreq.h>
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
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/aee.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#include "mach/mt_typedefs.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_cpufreq.h"
#include "mach/mt_gpufreq.h"
#include "mach/sync_write.h"
#include "mach/mt_spm.h"
#include "mach/mt_pmic_wrap.h"
#include "mach/mt_dcm.h"
#include "mach/mt_ptp.h"
/**************************************************
* enable for DVFS random test
***************************************************/
//#define MT_DVFS_RANDOM_TEST
/**************************************************
* enable this option to adjust buck voltage
***************************************************/
#define MT_BUCK_ADJUST

/***************************
* debug message
****************************/
#define dprintk(fmt, args...)                                       \
do {                                                                \
    if (mt_cpufreq_debug) {                                         \
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", fmt, ##args);   \
    }                                                               \
} while(0)

#define ARRAY_AND_SIZE(x)	(x), ARRAY_SIZE(x)

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend mt_cpufreq_early_suspend_handler =
{
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 200,
    .suspend = NULL,
    .resume  = NULL,
};
#endif

/**************************************************
* enable DVFS function
***************************************************/
static int g_dvfs_disable_count = 0;
static unsigned int g_cur_OPP;
static unsigned int g_normal_max_OPP;
static unsigned int g_fix_OPP;
static unsigned int g_max_power_OPP;
static unsigned int g_vcore = DVFS_V1;
static unsigned int g_limited_power = 0;

static unsigned int g_limited_max_ncpu;
static unsigned int g_limited_max_freq;
//static unsigned int g_limited_min_freq;
static unsigned int g_cpufreq_get_ptp_level = 0;
static unsigned int g_cpufreq_power_tbl_num = 0;
//static unsigned int g_max_freq_by_ptp = DVFS_F1; /* default 1 GHz */

static int g_ramp_down_count = 0;

static bool mt_cpufreq_debug = false;
static bool mt_cpufreq_ready = false;
static bool mt_cpufreq_pause = false;
static bool mt_cpufreq_ptpod_disable = true;
static bool mt_cpufreq_fix = false;
static bool mt_cpufreq_fixdds = false;
static bool mt_cpufreq_usb_raise = true;
static DEFINE_SPINLOCK(mt_cpufreq_lock);
static DEFINE_MUTEX(power_mutex);

/***************************
* Operate Point Definition
****************************/
#define OP(khz, volt)       \
{                           \
    .cpufreq_khz = khz,     \
    .cpufreq_volt = volt,   \
}

struct mt_cpu_freq_info
{
    unsigned int cpufreq_khz;
    unsigned int cpufreq_volt;
};

struct mt_cpu_tbl_info
{
    unsigned int cpufreq_khz;
    unsigned int cpufreq_volt;
    unsigned int tbl_idx;
};

struct mt_cpu_power_info
{
    unsigned int cpufreq_khz;
    unsigned int cpufreq_volt;
    unsigned int cpufreq_ncpu;
    unsigned int cpufreq_power;
};

/***************************
* MT6572 E1 DVFS Table
****************************/
static struct mt_cpu_freq_info mt6572_freqs_e1[] = {
    OP(DVFS_D2, DVFS_V0),
    OP(DVFS_D3, DVFS_V0),
    OP(DVFS_F1, DVFS_V1),
    OP(DVFS_F2, DVFS_V1),
    OP(DVFS_F3, DVFS_V1),
};

static struct mt_cpu_freq_info mt6572_freqs_e1_1[] = {
    OP(DVFS_D3, DVFS_V0),
    OP(DVFS_F1, DVFS_V1),
    OP(DVFS_F2, DVFS_V1),
    OP(DVFS_F3, DVFS_V1),
};

static struct mt_cpu_freq_info mt6572m_freqs_e1[] = {
    OP(DVFS_F1, DVFS_V1),
    OP(DVFS_F2, DVFS_V1),
    OP(DVFS_F3, DVFS_V1),
};

static unsigned int mt_cpu_freqs_num;
static struct mt_cpu_freq_info *mt_cpu_freqs = NULL;
static struct cpufreq_frequency_table *mt_cpu_freqs_table;
static struct mt_cpu_tbl_info spm_pmic_config[MAX_SPM_PMIC_TBL];
static unsigned int cpu_num = 0;
/* Power Golden table */
static struct mt_cpu_power_info mt_cpu_golden_power[] = {
    {.cpufreq_khz = DVFS_D0, .cpufreq_volt = DVFS_V0, .cpufreq_ncpu = 2, .cpufreq_power = 724 },
    {.cpufreq_khz = DVFS_D1, .cpufreq_volt = DVFS_V0, .cpufreq_ncpu = 2, .cpufreq_power = 653 },
    {.cpufreq_khz = DVFS_D2, .cpufreq_volt = DVFS_V0, .cpufreq_ncpu = 2, .cpufreq_power = 618 },
    {.cpufreq_khz = DVFS_D3, .cpufreq_volt = DVFS_V0, .cpufreq_ncpu = 2, .cpufreq_power = 582 },
    {.cpufreq_khz = DVFS_D0, .cpufreq_volt = DVFS_V0, .cpufreq_ncpu = 1, .cpufreq_power = 423 },
    {.cpufreq_khz = DVFS_F1, .cpufreq_volt = DVFS_V1, .cpufreq_ncpu = 2, .cpufreq_power = 422 },
    {.cpufreq_khz = DVFS_D1, .cpufreq_volt = DVFS_V0, .cpufreq_ncpu = 1, .cpufreq_power = 383 },
    {.cpufreq_khz = DVFS_D2, .cpufreq_volt = DVFS_V0, .cpufreq_ncpu = 1, .cpufreq_power = 363 },
    {.cpufreq_khz = DVFS_F2, .cpufreq_volt = DVFS_V1, .cpufreq_ncpu = 2, .cpufreq_power = 362 },
    {.cpufreq_khz = DVFS_D3, .cpufreq_volt = DVFS_V0, .cpufreq_ncpu = 1, .cpufreq_power = 343 },
    {.cpufreq_khz = DVFS_F3, .cpufreq_volt = DVFS_V1, .cpufreq_ncpu = 2, .cpufreq_power = 302 },
    {.cpufreq_khz = DVFS_F1, .cpufreq_volt = DVFS_V1, .cpufreq_ncpu = 1, .cpufreq_power = 249 },
    {.cpufreq_khz = DVFS_F2, .cpufreq_volt = DVFS_V1, .cpufreq_ncpu = 1, .cpufreq_power = 215 },
    {.cpufreq_khz = DVFS_F3, .cpufreq_volt = DVFS_V1, .cpufreq_ncpu = 1, .cpufreq_power = 181 },
};

static struct mt_cpu_power_info *mt_cpu_power = NULL;

/******************************
* Extern Function Declaration
*******************************/
extern void hp_limited_cpu_num(int num);
extern unsigned int PTP_get_ptp_level(void);
extern int mt_cpufreq_cur_load(void);
extern void mt_cpufreq_thermal_protect(unsigned int limited_power);
extern unsigned int get_normal_max_opp_idx(void);
extern  kal_bool charging_type_detection_done(void);
/* Look for MAX frequency in number of DVS. */
unsigned int mt_cpufreq_max_frequency_by_DVS(unsigned int num)
{
    /* int voltage_change_num = 0; */
    int i = 0;

    /* Assume mt6572_freqs_e1 voltage will be put in order, and freq will be put from high to low.*/
    for(i = num; i < MAX_SPM_PMIC_TBL; i++){
        if(spm_pmic_config[i].cpufreq_volt > DVFS_MIN_VCORE){
        	  xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PTPOD: freq need PTPOD: %d\n", spm_pmic_config[i].cpufreq_khz);
            return spm_pmic_config[i].cpufreq_khz;
        }
    }

    return 0;
}
EXPORT_SYMBOL(mt_cpufreq_max_frequency_by_DVS);

void restore_default_volt(void)
{
    unsigned int i, j;
    for(i = 0; i < MAX_SPM_PMIC_TBL; i++){
        // make sure which entry need to restore
        if(spm_pmic_config[i].cpufreq_khz > NOR_MAX_FREQ){
            // set spm_pmic_config & mt_cpu_freqs.cpufreq_volt up-to-date
            spm_pmic_config[i].cpufreq_volt = DVFS_V0;
        	  for(j = 0; j < mt_cpu_freqs_num;j++) {
                if(mt_cpu_freqs[j].cpufreq_khz == spm_pmic_config[i].cpufreq_khz){
                    mt_cpu_freqs[j].cpufreq_volt = DVFS_V0;
                }
        	  }
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PTPOD: restore entry[], volt[%d] \n",i, spm_pmic_config[i].cpufreq_khz);
            mt65xx_reg_sync_writel(VOLT_TO_PMIC_VAL(spm_pmic_config[i].cpufreq_volt), PMIC_WRAP_DVFS_WDATA3 + 8 * i);
        }
    }
    //set vcore again to make sure value take effect
    mt_vcore_freq_volt_set(mt_cpu_freqs[g_cur_OPP].cpufreq_volt, mt_cpu_freqs[g_cur_OPP].cpufreq_khz);
}

void set_spm_tbl(unsigned int pmic_value, unsigned int freq)
{
	  unsigned int i, j;
	  unsigned long flags;
	  spin_lock_irqsave(&mt_cpufreq_lock, flags);

    for(i = 0; i < MAX_SPM_PMIC_TBL; i++)	{
        if(spm_pmic_config[i].cpufreq_khz == freq){
            switch(spm_pmic_config[i].tbl_idx){
                case 3:
                    mt65xx_reg_sync_writel(pmic_value, PMIC_WRAP_DVFS_WDATA3);
                    break;
                case 4:
                    mt65xx_reg_sync_writel(pmic_value, PMIC_WRAP_DVFS_WDATA4);
                    break;
                case 5:
                    mt65xx_reg_sync_writel(pmic_value, PMIC_WRAP_DVFS_WDATA5);
                    break;
                default:
                    dprintk("There is no entry [%d] in spm table for ptpod", spm_pmic_config[i].tbl_idx);
                    break;
            }
            // set spm_pmic_config & mt_cpu_freqs.cpufreq_volt up-to-date
            spm_pmic_config[i].cpufreq_volt = PMIC_VAL_TO_VOLT(pmic_value);
            for(j = 0; j < mt_cpu_freqs_num;j++) {
                if(mt_cpu_freqs[j].cpufreq_khz == freq){
                    mt_cpu_freqs[j].cpufreq_volt = PMIC_VAL_TO_VOLT(pmic_value);
                }
            }
            dprintk("Set spm_pmic_config[%d].cpufreq_volt = %d", i, spm_pmic_config[i].cpufreq_volt);
            //set vcore again to make sure value take effect
            mt_vcore_freq_volt_set(mt_cpu_freqs[g_cur_OPP].cpufreq_volt, mt_cpu_freqs[g_cur_OPP].cpufreq_khz);
        }
    }

    spin_unlock_irqrestore(&mt_cpufreq_lock, flags);
}

void check_cur_power(unsigned int idx){
	  unsigned int i;
    for(i = 0; i < g_cpufreq_power_tbl_num; i++){
        if((mt_cpu_power[i].cpufreq_khz == mt_cpu_freqs[idx].cpufreq_khz) && (mt_cpu_power[i].cpufreq_ncpu == cpu_num)){
            if(mt_cpu_power[i].cpufreq_power > mt_cpu_power[g_max_power_OPP].cpufreq_power){
                dprintk("%d > %d", mt_cpu_power[i].cpufreq_power, mt_cpu_power[g_max_power_OPP].cpufreq_power);
                aee_kernel_exception("DVFS", "CPU power over thermal limitation: %d > %d", mt_cpu_power[i].cpufreq_power, mt_cpu_power[g_max_power_OPP].cpufreq_power);
            }
        }
    }
}

unsigned int mt_cpufreq_min_power(void) {
    return mt_cpu_power[g_cpufreq_power_tbl_num-1].cpufreq_power;
}
unsigned int mt_cpufreq_max_power(void) {
    return mt_cpu_power[0].cpufreq_power;
}

static void mt_setup_power_table(int num)
{
    int i, j, power_idx = 0;

    g_cpufreq_power_tbl_num = num * CORE_NUM;
    mt_cpu_power = kzalloc((g_cpufreq_power_tbl_num) * sizeof(struct mt_cpu_power_info), GFP_KERNEL);

    for(i = 0; i < ARRAY_SIZE(mt_cpu_golden_power); i++){
        for(j = 0; j < num; j++){
            if(mt_cpu_golden_power[i].cpufreq_khz == mt_cpu_freqs[j].cpufreq_khz){
                memcpy(mt_cpu_power+power_idx, mt_cpu_golden_power+i, sizeof(struct mt_cpu_power_info));
                power_idx++;
            }
        }
    }

    for (i = 0; i <(g_cpufreq_power_tbl_num); i++)
    {
        dprintk("mt_cpu_power[%d].cpufreq_khz = %d, ", i, mt_cpu_power[i].cpufreq_khz);
        dprintk("mt_cpu_power[%d].cpufreq_ncpu = %d, ", i, mt_cpu_power[i].cpufreq_ncpu);
        dprintk("mt_cpu_power[%d].cpufreq_power = %d\n", i, mt_cpu_power[i].cpufreq_power);
    }
}

void mt_dvfs_power_dispatch(void)
{
    unsigned int cpu_loading, gpu_loading, cpu_power_budget, gpu_power_budget, total_power_budget, total_loading, diff, i;

    total_power_budget = g_limited_power;

    //do nothing and return
    if(total_power_budget == 0)
        return ;

    cpu_power_budget = mt_cpufreq_min_power();
    gpu_power_budget = mt_gpufreq_min_power();

    if(total_power_budget > (cpu_power_budget + gpu_power_budget)){

        total_power_budget -= (cpu_power_budget + gpu_power_budget);
        cpu_loading = mt_cpufreq_cur_load() + 1;
        gpu_loading = mt_gpufreq_cur_load() + 1;
        total_loading = gpu_loading + cpu_loading;
        cpu_power_budget += (total_power_budget * cpu_loading / total_loading);
        gpu_power_budget += (total_power_budget * gpu_loading / total_loading);

        //give power budget to gpu
        if(cpu_power_budget > mt_cpufreq_max_power() && gpu_power_budget < mt_gpufreq_max_power()) {
            diff = cpu_power_budget - mt_cpufreq_max_power();
            gpu_power_budget += diff;
            cpu_power_budget -= diff;
        }
        //give power budget to cpu
        else if(gpu_power_budget > mt_gpufreq_max_power() && cpu_power_budget < mt_cpufreq_max_power()) {
            diff = gpu_power_budget - mt_gpufreq_max_power();
            cpu_power_budget += diff;
            gpu_power_budget -= diff;
        }
        dprintk("cpu_loading: %d , gpu_loading = %d\n", cpu_loading, gpu_loading);
        dprintk("cpu_power_budget: %d , gpu_power_budget = %d\n", cpu_power_budget, gpu_power_budget);

        for(i = 0; i < (mt_cpu_freqs_num * CORE_NUM); i++){
            if(mt_cpu_power[i].cpufreq_power <= cpu_power_budget){
                g_max_power_OPP = i;
                break;
            }
        }
        mt_cpufreq_thermal_protect(cpu_power_budget);
        mt_gpufreq_thermal_protect(gpu_power_budget);
        if(cpu_power_budget + gpu_power_budget > g_limited_power){
            aee_kernel_exception("DVFS", "power budget overflow: %d + %d > %d", cpu_power_budget, gpu_power_budget, g_limited_power);
        }
    }
}

void set_dvfs_thermal_limit(unsigned int limited_power)
{
    // set global power info
    mutex_lock(&power_mutex);

    g_limited_power = limited_power;

    if(g_limited_power == 0){
        g_max_power_OPP = 0;
        mt_cpufreq_thermal_protect(0);
        mt_gpufreq_thermal_protect(0);
    }
    else if(g_limited_power < (mt_cpufreq_min_power()+mt_gpufreq_min_power())){
        dprintk("[MIN] cpu_power_budget: %d , gpu_power_budget = %d\n", mt_cpufreq_min_power(), mt_gpufreq_min_power());
        g_max_power_OPP = g_cpufreq_power_tbl_num;
        mt_cpufreq_thermal_protect(mt_cpufreq_min_power());
        mt_gpufreq_thermal_protect(mt_gpufreq_min_power());
    }
    else{
        mt_dvfs_power_dispatch();
    }
    mutex_unlock(&power_mutex);
}

void mt_dvfs_power_dispatch_safe(void)
{
    mutex_lock(&power_mutex);

    mt_dvfs_power_dispatch();

    mutex_unlock(&power_mutex);
}


// for debug purpose
#define D_TEST_DBG_CTRL_MASK      (0xff)
#define D_TEST_DBG_CTRL_BIT       (0)
#define D_INFRA_AO_DBG_CON0_MASK  (0x1f)
#define D_INFRA_AO_DBG_CON0_BIT   (0)
#define D_DBG_CTRL_MASK           (0xff)
#define D_DBG_CTRL_BIT            (0)

#define TEST_DBG_CTRL_REG       (0xf0000038)
#define DBG_CTRL_REG            (0xf0200080)
#define INFRA_AO_DBG_CON0_REG   (0xf0001500)

#define CLR_FIELD(VAL, REG)                      VAL &= (~((REG##_MASK) << (REG##_BIT)));
#define SET_FIELD(VAL, WVAL, REG)                VAL |= ( ((WVAL) & (REG##_MASK)) << (REG##_BIT) );
#define CLR_AND_SET_FIELD(VAL, WVAL, REG)        CLR_FIELD(VAL, REG); SET_FIELD(VAL, WVAL, REG);

int CPU_freq_output()
{
    FREQMETER_CTRL fqmtr;
    unsigned int test_dbg_ctrl, dbg_ctrl, infra_dbg;
    int res = 0;

    // set debug register before init freq meter for APMCU
    test_dbg_ctrl = DRV_Reg32(TEST_DBG_CTRL_REG);
    CLR_AND_SET_FIELD(test_dbg_ctrl, (unsigned int)0x0, D_TEST_DBG_CTRL);
    DRV_WriteReg32(TEST_DBG_CTRL_REG, test_dbg_ctrl);

    dbg_ctrl = DRV_Reg32(DBG_CTRL_REG);
    CLR_AND_SET_FIELD(dbg_ctrl, (unsigned int)0x7, D_DBG_CTRL);
    DRV_WriteReg32(DBG_CTRL_REG, dbg_ctrl);

    infra_dbg = DRV_Reg32(INFRA_AO_DBG_CON0_REG);
    CLR_AND_SET_FIELD(infra_dbg, 0x26, D_INFRA_AO_DBG_CON0);
    DRV_WriteReg32(INFRA_AO_DBG_CON0_REG, infra_dbg);
    // Init freq meter parameter
    fqmtr.divider            = RG_FQMTR_CKDIV_D8;
    fqmtr.ref_clk_sel        = RG_FQMTR_FIXCLK_SEL_26MHZ;
    fqmtr.mon_sel            = FQMTR_SRC_APMCU_CLOCK;
    fqmtr.mon_len_in_ref_clk = 0x400;
    res = freqmeter_kick (&fqmtr);

    if (res != FREQMETER_SUCCESS)
    {
        //- check error code
        dprintk("CPU_freq_output: freq meter kick fail!! \n");
        return -1;
    }
    else
    {
        //- polling mode
        fqmtr.polling_to_getresult = 1;
        // should be blocking call here
        res = freqmeter_getresult (&fqmtr);
        if (res != FREQMETER_SUCCESS)
        {
            //- check error code
            dprintk("CPU_freq_output: freqmeter get result fail!! \n");
            return -1;
        }
        else
        {
            //- success
             return ((fqmtr.result_in_count * 26) / 0x400);
        }
    }
}
int GPU_freq_output()
{
    FREQMETER_CTRL fqmtr;
    int res = 0;

    // Init freq meter parameter
    fqmtr.divider            = RG_FQMTR_CKDIV_D8;
    fqmtr.ref_clk_sel        = RG_FQMTR_FIXCLK_SEL_26MHZ;
    fqmtr.mon_sel            = FQMTR_SRC_GPU_CLOCK;
    fqmtr.mon_len_in_ref_clk = 0x400;
    res = freqmeter_kick (&fqmtr);

    if (res != FREQMETER_SUCCESS)
    {
        // check error code
        dprintk("GPU_freq_output: freq meter kick fail!! \n");
        return -1;
    }
    else
    {
        // polling mode
        fqmtr.polling_to_getresult = 1;
        // should be blocking call here
        res = freqmeter_getresult (&fqmtr);
        if (res != FREQMETER_SUCCESS)
        {
            // check error code
            dprintk("GPU_freq_output: freqmeter get result fail!! \n");
            return -1;
        }
    else
    {
            // success
            return ((fqmtr.result_in_count * 26) / 0x400);
        }
    }
}
/**************************************
 * CPU DVFS control vcore by spm to pmic_wrapper
 **************************************/
#define MAX_RETRY_COUNT (100)

int spm_dvfs_ctrl_volt(unsigned int value)
{
    unsigned int ap_dvfs_con;
    int retry = 0;

    ap_dvfs_con = spm_read(SPM_AP_DVFS_CON_SET);
    spm_write(SPM_AP_DVFS_CON_SET, (ap_dvfs_con & ~(0x7)) | value);
    udelay(5);

    while ((spm_read(SPM_AP_DVFS_CON_SET) & (0x1 << 31)) == 0)
    {
        if (retry >= MAX_RETRY_COUNT)
        {
            printk("FAIL: no response from PMIC wrapper\n");
            return -1;
        }

        retry++;
        printk("wait for ACK signal from PMIC wrapper, retry = %d\n", retry);

        udelay(5);
    }

    return 0;
}

/***********************************************
* register frequency table to cpufreq subsystem
************************************************/
static int mt_setup_freqs_table(struct cpufreq_policy *policy, struct mt_cpu_freq_info *freqs, int num)
{
    struct cpufreq_frequency_table *table;
    int i, ret;

    if(mt_cpu_freqs_table == NULL){
        table = kzalloc((num + 1) * sizeof(*table), GFP_KERNEL);
        if (table == NULL)
            return -ENOMEM;

        for (i = 0; i < num; i++) {
            table[i].index = i;
            table[i].frequency = freqs[i].cpufreq_khz;
        }

        table[num].frequency = CPUFREQ_TABLE_END;

        mt_cpu_freqs = freqs;
        mt_cpu_freqs_num = num;
        mt_cpu_freqs_table = table;
    }

    ret = cpufreq_frequency_table_cpuinfo(policy, mt_cpu_freqs_table);
    if (!ret)
        cpufreq_frequency_table_get_attr(mt_cpu_freqs_table, policy->cpu);

    if (mt_cpu_power == NULL)
        mt_setup_power_table(num);

    return 0;
}

/*****************************
* set CPU DVFS status
******************************/
int mt_cpufreq_state_set(int enabled)
{
    struct cpufreq_policy *policy;
    policy = cpufreq_cpu_get(0);

    if (enabled)
    {
        if (!mt_cpufreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "cpufreq already enabled\n");
            return 0;
        }

        /*************
        * enable DVFS
        **************/
        g_dvfs_disable_count--;
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "enable DVFS: g_dvfs_disable_count = %d\n", g_dvfs_disable_count);

        /***********************************************
        * enable DVFS if no any module still disable it
        ************************************************/
        if (g_dvfs_disable_count <= 0)
        {
            mt_cpufreq_pause = false;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "someone still disable cpufreq, cannot enable it\n");
        }
    }
    else
    {
        /**************
        * disable DVFS
        ***************/
        g_dvfs_disable_count++;
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "disable DVFS: g_dvfs_disable_count = %d\n", g_dvfs_disable_count);

        if (mt_cpufreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "cpufreq already disabled\n");
            return 0;
        }

        mt_cpufreq_pause = true;
        if (mt_cpu_freqs)
            cpufreq_driver_target(policy, mt_cpu_freqs[g_normal_max_OPP].cpufreq_khz, CPUFREQ_RELATION_L);
    }

    return 0;
}
EXPORT_SYMBOL(mt_cpufreq_state_set);

static int mt_cpufreq_verify(struct cpufreq_policy *policy)
{
    dprintk("call mt_cpufreq_verify!\n");
    return cpufreq_frequency_table_verify(policy, mt_cpu_freqs_table);
}

static unsigned int mt_cpufreq_get(unsigned int cpu)
{
    dprintk("call mt_cpufreq_get: %d!\n", mt_cpu_freqs[g_cur_OPP].cpufreq_khz);
    return mt_cpu_freqs[g_cur_OPP].cpufreq_khz;
}

void mt_cpu_clock_switch(enum cpu_src src){
    unsigned int clkmux = 0;

    clkmux = DRV_Reg32(TOP_CKMUXSEL) & ~0xC;

    switch(src){
        case CK26M:
            mt65xx_reg_sync_writel((clkmux | (0x0 << 2)), TOP_CKMUXSEL); // 26M
            break;
        case ARMPLL_SRC:
            mt65xx_reg_sync_writel((clkmux | (0x1 << 2)), TOP_CKMUXSEL); // ARMPLL
            break;
        case UNIVPLL_SRC:
            mt65xx_reg_sync_writel((clkmux | (0x2 << 2)), TOP_CKMUXSEL); // UPLL
            break;
        case MAINPLL_SRC:
            mt65xx_reg_sync_writel((clkmux | (0x3 << 2)), TOP_CKMUXSEL); // MAINPLL
            break;
    }
}

void mt_vcore_freq_volt_set(unsigned int target_volt, unsigned int target_freq)
{
	  unsigned int i;
	  if(target_freq == 0 || target_volt <= DVFS_MIN_VCORE){
		    switch (target_volt)
		    {
		        case DVFS_V0:
		            dprintk("switch vcore to fix DVS0: %d mV\n", DVFS_V0);
		            spm_dvfs_ctrl_volt(0);
		            g_vcore = target_volt;
		            break;
		        case DVFS_V1:
		            dprintk("switch vcore to fix DVS1: %d mV\n", DVFS_V1);
		            spm_dvfs_ctrl_volt(1);
		            g_vcore = target_volt;
		            break;
		        default:
		            break;
		    }
    }
    else{
        for(i = 0; i < MAX_SPM_PMIC_TBL; i++){
            if(spm_pmic_config[i].cpufreq_khz == target_freq){
                dprintk("switch vcore to var DVS%d: %d mV\n", spm_pmic_config[i].tbl_idx, spm_pmic_config[i].cpufreq_volt);
                spm_dvfs_ctrl_volt(spm_pmic_config[i].tbl_idx);
                g_vcore = spm_pmic_config[i].cpufreq_volt;
            }
        }
    }
}

/***********************************************************
* [note]
* 1. frequency ramp up need to wait voltage settle
* 2. frequency ramp down do not need to wait voltage settle
************************************************************/
static void mt_cpufreq_set(unsigned int old_OPP, unsigned int new_OPP)
{
    unsigned int armpll_dds = 0, target_freq = mt_cpu_freqs[new_OPP].cpufreq_khz;
    unsigned int cur_vcore, new_vcore;
    unsigned int max_delay = (PLL_SETTLE_TIME > PMIC_SETTLE_TIME? PLL_SETTLE_TIME:PMIC_SETTLE_TIME);

    cur_vcore = get_cur_vcore();
    new_vcore = get_max_vcore(mt_cpu_freqs[new_OPP].cpufreq_volt, mt_gpufreq_cur_volt());

    if( target_freq> PLL_MAX_FREQ || target_freq < PLL_MIN_FREQ){
        dprintk("set_armpll_freq: freq [%d] out of range\n", target_freq);
        return;
    }
    else
    {
        if (target_freq >= PLL_DIV1_FREQ)
        {
            armpll_dds = 0x8009A000;
            armpll_dds = armpll_dds + ((target_freq - PLL_DIV1_FREQ) / 13000) * 0x2000;
        }
        else if (target_freq >= PLL_DIV2_FREQ)
        {
            armpll_dds = 0x810A0000;
            armpll_dds = armpll_dds + ((target_freq - PLL_DIV2_FREQ) * 2 / 13000) * 0x2000;
        }
        else if (target_freq >= PLL_DIV4_FREQ)
        {
            armpll_dds = 0x820A0000;
            armpll_dds = armpll_dds + ((target_freq - PLL_DIV4_FREQ) * 4 / 13000) * 0x2000;
        }
        else if (target_freq >= PLL_DIV8_FREQ)
        {
            armpll_dds = 0x830A0000;
            armpll_dds = armpll_dds + ((target_freq - PLL_DIV8_FREQ) * 8 / 13000) * 0x2000;
        }
        else
        {
            dprintk("set_armpll_freq: unsupport frequency!!\n");
        }
    }

    enable_clock(MT_CG_MPLL_D2, "CPU_DVFS");

    if (mt_cpu_freqs[new_OPP].cpufreq_khz > mt_cpu_freqs[old_OPP].cpufreq_khz)
    {
        // switch CPU clock to mainpll in the very beginning
        mt_cpu_clock_switch(MAINPLL_SRC);

        if(cur_vcore != new_vcore){
            mt_vcore_freq_volt_set(new_vcore, mt_cpu_freqs[new_OPP].cpufreq_khz);
        }

        mt65xx_reg_sync_writel(armpll_dds, ARMPLL_CON1);

        if(cur_vcore != new_vcore){
            udelay(max_delay);
        }
        else{
            // only wait pll stable if vcore is not changed
            udelay(PLL_SETTLE_TIME);
        }
        // switch CPU clock back when all ready
        mt_cpu_clock_switch(ARMPLL_SRC);
    }
    else
    {
        // switch CPU clock to mainpll in the very beginning
        mt_cpu_clock_switch(MAINPLL_SRC);

        mt65xx_reg_sync_writel(armpll_dds, ARMPLL_CON1);

        if(cur_vcore != new_vcore){
            mt_vcore_freq_volt_set(new_vcore, mt_cpu_freqs[new_OPP].cpufreq_khz);
            udelay(max_delay);
        }
        else{
            // only wait pll stable if vcore is not changed
            udelay(PLL_SETTLE_TIME);
        }

        // switch CPU clock back when all ready
        mt_cpu_clock_switch(ARMPLL_SRC);
    }

    disable_clock(MT_CG_MPLL_D2, "CPU_DVFS");
    g_cur_OPP = new_OPP;
    dprintk("ARMPLL_CON0 = 0x%x, ARMPLL_CON1 = 0x%x, current_freq = %d\n", DRV_Reg32(ARMPLL_CON0), DRV_Reg32(ARMPLL_CON1), mt_cpu_freqs[g_cur_OPP].cpufreq_khz);
    dprintk("[CPU freq]: %d\n", CPU_freq_output());
}

/**************************************
* check if maximum frequency is needed
***************************************/
static int mt_cpufreq_keep_org_freq(unsigned int old_OPP, unsigned int new_OPP)
{
    //if (mt_cpufreq_pause)
    //    return 1;

    /* check if system is going to ramp down */
    if (mt_cpu_freqs[new_OPP].cpufreq_khz < mt_cpu_freqs[old_OPP].cpufreq_khz){
        g_ramp_down_count++;
        if (g_ramp_down_count < RAMP_DOWN_TIMES)
            return 1;
        else
            return 0;
    }
    else
    {
        g_ramp_down_count = 0;
        return 0;
    }
}

#ifdef MT_DVFS_RANDOM_TEST
static int mt_cpufreq_idx_get(int num)
{
    int random = 0, mult = 0, idx;
    random = jiffies & 0xF;

    while (1)
    {
        if ((mult * num) >= random)
        {
            idx = (mult * num) - random;
            break;
        }
        mult++;
    }
    return idx;
}
#endif

static unsigned int mt_thermal_limited_verify(unsigned int OPP_index)
{
    int i = 0;
    struct cpufreq_policy *policy;

    policy = cpufreq_cpu_get(0);
    cpu_num = num_online_cpus();

    for (i = g_max_power_OPP; i < (g_cpufreq_power_tbl_num); i++)
    {
        // search some OPPs on power table which cpu number = online
        if(mt_cpu_power[i].cpufreq_ncpu == cpu_num)
        {
                //legal table entry
                if( mt_cpu_power[i].cpufreq_khz >= mt_cpu_freqs[OPP_index].cpufreq_khz ){
                    return OPP_index;
                }
                else {
                    // need to do throttling
                    cpufreq_frequency_table_target(policy, mt_cpu_freqs_table, mt_cpu_power[i].cpufreq_khz, CPUFREQ_RELATION_L, &OPP_index);
                    dprintk("[verified] target_freq = %d, ncpu = %d\n", mt_cpu_freqs[OPP_index].cpufreq_khz, cpu_num);
                    return OPP_index;
                }
        }
    }
    dprintk("Can't find suitable OPP for thermal throttling!\n");
    dprintk("g_max_power_OPP = %d, freq = %d, ncpu = %d\n", g_max_power_OPP, mt_cpu_freqs[g_cur_OPP].cpufreq_khz / 1000, cpu_num);
    return OPP_index;
}

unsigned int get_max_vcore(unsigned int cpu_volt, unsigned int gpu_volt)
{
    unsigned int max_vcore;

    max_vcore = cpu_volt > gpu_volt ? cpu_volt : gpu_volt;

    return max_vcore;
}

unsigned int get_cur_vcore()
{
    return g_vcore;
}

unsigned int mt_cpufreq_cur_volt(void)
{
    return mt_cpu_freqs[g_cur_OPP].cpufreq_volt;
}

unsigned int get_defcur_opp_idx(void)
{
    int i;
    for(i = 0; i < mt_cpu_freqs_num; i++){
        if(mt_cpu_freqs[i].cpufreq_khz == DEFAULT_FREQ){
            return i;
        }
    }
    return 0;
}
/*unsigned int get_defmax_opp_idx()
{
    return MAX_OPP;
}*/
unsigned int get_normal_max_opp_idx(void)
{
    int i;
    for(i = 0; i < mt_cpu_freqs_num; i++) {
        if(mt_cpu_freqs[i].cpufreq_khz == NOR_MAX_FREQ){
            return i;
        }
    }
    return 0;
}
/*unsigned int get_defmin_opp_idx()
{
    return MIN_OPP;
}*/
unsigned int get_normal_max_freq(void){
    return mt_cpu_freqs[g_normal_max_OPP].cpufreq_khz;
}
/**********************************
* cpufreq target callback function
***********************************/
/*************************************************
* [note]
* 1. handle frequency change request
* 2. call mt_cpufreq_set to set target frequency
**************************************************/
static int mt_cpufreq_target(struct cpufreq_policy *policy, unsigned int target_freq, unsigned int relation)
{
    int i, new_OPP_idx; /* , fix_OPP; */
    unsigned int cpu;
    unsigned long flags;

    /* struct mt_cpu_freq_info next; */
    struct cpufreq_freqs freqs;

    if (!mt_cpufreq_ready)
        return -ENOSYS;

    if (policy->cpu >= num_possible_cpus())
        return -EINVAL;



    if (mt_cpufreq_fix == true || mt_cpufreq_fixdds == true){
        //fix opp gets higher priority, so we check if system need to fix freq
        if(mt_cpufreq_fixdds == true){
            // do nothing and exit
            return 0;
        }
        else if(mt_cpufreq_fix == true){
            new_OPP_idx = g_fix_OPP;
        }
    }
    else if (mt_cpufreq_pause == true){
        // early suspend should keep 1G/1.15V for SPM to restore
        new_OPP_idx = g_normal_max_OPP;
    }
    else{
        /******************************
         * look up the target frequency
         *******************************/
        if (cpufreq_frequency_table_target(policy, mt_cpu_freqs_table, target_freq, relation, &new_OPP_idx))
        {
            return -EINVAL;
        }
#ifdef MT_DVFS_RANDOM_TEST
        new_OPP_idx = mt_cpufreq_idx_get(4);
#endif

#ifdef MT_DVFS_RANDOM_TEST
        //dprintk("idx = %d, freqs.old = %d, freqs.new = %d\n", idx, policy->cur, next.cpufreq_khz);
        dprintk("new_OPP_idx = %d, freqs.old = %d, freqs.new = %d\n", new_OPP_idx, policy->cur, mt_cpu_freqs[new_OPP_idx].cpufreq_khz);
#endif

#ifndef MT_DVFS_RANDOM_TEST
        // don't do ramp down until there are "two" continuous request to ramp down
        if (mt_cpufreq_keep_org_freq(g_cur_OPP, new_OPP_idx))
        {
            //keep org opp
            new_OPP_idx = g_cur_OPP;
        }
        //
        if(clock_is_on(MT_CG_USB_SW_CG) == PWR_ON && mt_cpufreq_usb_raise == true && charging_type_detection_done() == KAL_TRUE)
        {
        	  new_OPP_idx = 0;
        }
        // dynamic control power budget for CPU
        if(g_limited_power != 0){
            // throttling OPP
            mutex_lock(&power_mutex);
            if (num_online_cpus() > g_limited_max_ncpu)
            {
                for (i = num_online_cpus(); i > g_limited_max_ncpu; i--)
                {
                    dprintk("turn off CPU%d due to thermal protection\n", (i - 1));
                    cpu_down((i - 1));
                }
            }
            new_OPP_idx = mt_thermal_limited_verify(new_OPP_idx);
            check_cur_power(new_OPP_idx);
            mutex_unlock(&power_mutex);
            dprintk("CPU freq after thermal verified: %d MHZ\n", mt_cpu_freqs[new_OPP_idx].cpufreq_khz / 1000);
        }
        // error check
        if (mt_cpu_freqs[new_OPP_idx].cpufreq_khz < mt_cpu_freqs[mt_cpu_freqs_num-1].cpufreq_khz)
        {
            dprintk("cannot switch CPU frequency to %d Mhz due to voltage limitation\n", mt_cpu_freqs[mt_cpu_freqs_num-1].cpufreq_khz / 1000);
            new_OPP_idx = mt_cpu_freqs_num-1;
        }
#endif

        /************************************************
         * DVFS keep at 1001Mhz/1.15V when PTPOD initial
         *************************************************/
        if (mt_cpufreq_ptpod_disable)
        {
            //freqs.new = DVFS_F1;
            //new_OPP_idx = DEFAULT_OPP;
        }
    }
    /************************************************
    * target frequency == existing frequency, skip it
    *************************************************/
    if (new_OPP_idx == g_cur_OPP)
    {
        dprintk("CPU frequency from %d MHz to %d MHz (skipped) due to same frequency\n", mt_cpu_freqs[g_cur_OPP].cpufreq_khz / 1000, mt_cpu_freqs[new_OPP_idx].cpufreq_khz / 1000);
        return 0;
    }

    if(new_OPP_idx < 0 || new_OPP_idx >= mt_cpu_freqs_num)
    {
        dprintk("error new_OPP_idx: %d\n", new_OPP_idx);
        return 0;
    }

    /**************************************
    * search for OPP index, and the corresponding voltage
    ***************************************/
    /*next.cpufreq_volt = 0;

    for (idx = 0; idx < mt_cpu_freqs_num; idx++)
    {
        dprintk("freqs.new = %d, mt_cpu_freqs[%d].cpufreq_khz = %d\n", freqs.new, idx, mt_cpu_freqs[idx].cpufreq_khz);
        if (freqs.new == mt_cpu_freqs[idx].cpufreq_khz)
        {
            next.cpufreq_volt = mt_cpu_freqs[idx].cpufreq_volt;
            dprintk("next.cpufreq_volt = %d, mt_cpu_freqs[%d].cpufreq_volt = %d\n", next.cpufreq_volt, idx, mt_cpu_freqs[idx].cpufreq_volt);
            break;
        }
    }

    if (mt_cpu_freqs[idx].cpufreq_volt == 0)
    {
        dprintk("Error!! Cannot find corresponding voltage at %d Mhz\n", freqs.new / 1000);
        return 0;
    }*/

    freqs.old = policy->cur;
    freqs.new = mt_cpu_freqs[new_OPP_idx].cpufreq_khz;
    freqs.cpu = policy->cpu;

    for_each_online_cpu(cpu)
    {
        freqs.cpu = cpu;
        cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
    }

    spin_lock_irqsave(&mt_cpufreq_lock, flags);

    /******************************
    * set to the target freeuency
    *******************************/
    mt_cpufreq_set(g_cur_OPP, new_OPP_idx);

    spin_unlock_irqrestore(&mt_cpufreq_lock, flags);

    for_each_online_cpu(cpu)
    {
        freqs.cpu = cpu;
        cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
    }

    return 0;
}

/*********************************************************
* set up frequency table and register to cpufreq subsystem
**********************************************************/
static int mt_cpufreq_init(struct cpufreq_policy *policy)
{
    int ret = -EINVAL;
    /* unsigned int ver; */

    if (policy->cpu >= num_possible_cpus())
        return -EINVAL;

    policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
    cpumask_setall(policy->cpus);

    /*******************************************************
    * 1 us, assumed, will be overwrited by min_sampling_rate
    ********************************************************/
    policy->cpuinfo.transition_latency = 1000;

    /*********************************************
    * set default policy and cpuinfo, unit : Khz
    **********************************************/
    //Efuse Table Entry Point
    g_cpufreq_get_ptp_level = get_ptp_level();
    xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "g_cpufreq_get_ptp_level = %d\n", g_cpufreq_get_ptp_level);

    if(g_cpufreq_get_ptp_level == PTP_LEVEL_0){
        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6572m_freqs_e1));
    }
    else if(g_cpufreq_get_ptp_level == PTP_LEVEL_1){
  	    ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6572_freqs_e1_1));
  	}
    else if(g_cpufreq_get_ptp_level == PTP_LEVEL_2){
  	    ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6572_freqs_e1));
  	}
    else {
        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6572_freqs_e1));
    }

    //policy->cpuinfo.max_freq = g_max_freq_by_ptp;

    policy->cpuinfo.max_freq = mt_cpu_freqs[0].cpufreq_khz;
    policy->cpuinfo.min_freq = mt_cpu_freqs[mt_cpu_freqs_num-1].cpufreq_khz;

    policy->cur = DEFAULT_FREQ;
    policy->max = mt_cpu_freqs[0].cpufreq_khz;
    policy->min = mt_cpu_freqs[mt_cpu_freqs_num-1].cpufreq_khz;


    if (ret) {
        xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "failed to setup frequency table\n");
        return ret;
    }

    return 0;
}

static struct cpufreq_driver mt_cpufreq_driver = {
    .verify = mt_cpufreq_verify,
    .target = mt_cpufreq_target,
    .init   = mt_cpufreq_init,
    .get    = mt_cpufreq_get,
    .name   = "mt-cpufreq",
};

/*********************************
* early suspend callback function
**********************************/
void mt_cpufreq_early_suspend(struct early_suspend *h)
{
    #ifndef MT_DVFS_RANDOM_TEST

    mt_cpufreq_state_set(0);

    #endif

    return;
}

/*******************************
* late resume callback function
********************************/
void mt_cpufreq_late_resume(struct early_suspend *h)
{
    #ifndef MT_DVFS_RANDOM_TEST

    mt_cpufreq_state_set(1);

    #endif

    return;
}

/************************************************
* API to switch back default voltage setting for PTPOD disabled
*************************************************/
void mt_cpufreq_return_default_DVS_by_ptpod(void)
{
    /*if(g_cpufreq_get_ptp_level == 0)
    {
        mt65xx_reg_sync_writel(0x50, PMIC_WRAP_DVFS_WDATA0); // 1.20V VPROC
        mt65xx_reg_sync_writel(0x48, PMIC_WRAP_DVFS_WDATA1); // 1.15V VPROC
        mt65xx_reg_sync_writel(0x38, PMIC_WRAP_DVFS_WDATA2); // 1.05V VPROC
        mt65xx_reg_sync_writel(0x28, PMIC_WRAP_DVFS_WDATA3); // 0.95V VPROC
        mt65xx_reg_sync_writel(0x18, PMIC_WRAP_DVFS_WDATA4); // 0.85V VPROC
    }
    else if((g_cpufreq_get_ptp_level == 2) || (g_cpufreq_get_ptp_level == 4))
    {
        mt65xx_reg_sync_writel(0x58, PMIC_WRAP_DVFS_WDATA0); // 1.25V VPROC
        mt65xx_reg_sync_writel(0x50, PMIC_WRAP_DVFS_WDATA1); // 1.20V VPROC
        mt65xx_reg_sync_writel(0x48, PMIC_WRAP_DVFS_WDATA2); // 1.15V VPROC
        mt65xx_reg_sync_writel(0x38, PMIC_WRAP_DVFS_WDATA3); // 1.05V VPROC
        mt65xx_reg_sync_writel(0x28, PMIC_WRAP_DVFS_WDATA4); // 0.95V VPROC
    }
    else
    {
        mt65xx_reg_sync_writel(0x50, PMIC_WRAP_DVFS_WDATA0); // 1.20V VPROC
        mt65xx_reg_sync_writel(0x48, PMIC_WRAP_DVFS_WDATA1); // 1.15V VPROC
        mt65xx_reg_sync_writel(0x38, PMIC_WRAP_DVFS_WDATA2); // 1.05V VPROC
        mt65xx_reg_sync_writel(0x28, PMIC_WRAP_DVFS_WDATA3); // 0.95V VPROC
        mt65xx_reg_sync_writel(0x18, PMIC_WRAP_DVFS_WDATA4); // 0.85V VPROC
    }

    mt65xx_reg_sync_writel(0x38, PMIC_WRAP_DVFS_WDATA5); // 1.05V VCORE
    mt65xx_reg_sync_writel(0x28, PMIC_WRAP_DVFS_WDATA6); // 0.95V VCORE
    mt65xx_reg_sync_writel(0x18, PMIC_WRAP_DVFS_WDATA7); // 0.85V VCORE
    */
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq return default DVS by ptpod\n");
}
EXPORT_SYMBOL(mt_cpufreq_return_default_DVS_by_ptpod);

/************************************************
* DVFS enable API for PTPOD
*************************************************/
void mt_cpufreq_enable_by_ptpod(void)
{
    mt_cpufreq_ptpod_disable = false;
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq enabled by ptpod\n");
}
EXPORT_SYMBOL(mt_cpufreq_enable_by_ptpod);

/************************************************
* DVFS disable API for PTPOD
*************************************************/
/*unsigned int mt_cpufreq_disable_by_ptpod(void)
{
    struct cpufreq_policy *policy;

    mt_cpufreq_ptpod_disable = true;

    policy = cpufreq_cpu_get(0);

    if (!policy)
        goto no_policy;

    cpufreq_driver_target(policy, mt_cpu_freqs[0].cpufreq_khz, CPUFREQ_RELATION_L);

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq disabled by ptpod, limited freq. at %d\n", DVFS_F2);

    cpufreq_cpu_put(policy);

no_policy:
    return mt_cpu_freqs[g_cur_OPP].cpufreq_khz;
}
EXPORT_SYMBOL(mt_cpufreq_disable_by_ptpod);
*/
/************************************************
* frequency adjust interface for thermal protect
*************************************************/
/******************************************************
* parameter: target power
*******************************************************/
void mt_cpufreq_thermal_protect(unsigned int limited_power)
{
    int i = 0, ncpu = 0, found = 0;

    struct cpufreq_policy *policy;

    policy = cpufreq_cpu_get(0);

    if (!policy)
        goto no_policy;

    ncpu = num_possible_cpus();

    if (limited_power == 0) // no limitation
    {
        //restore max_ncpu and freq to maximum
        g_limited_max_ncpu = num_possible_cpus();
        g_limited_max_freq = mt_cpu_freqs[0].cpufreq_khz;
        g_max_power_OPP = 0;

        //cpufreq_driver_target(policy, g_limited_max_freq, CPUFREQ_RELATION_L);
        // set cpu number upperbound (notify CPU governor)
        hp_limited_cpu_num(g_limited_max_ncpu);

        dprintk("[thermal] max_freq = %d, max_ncpu = %d\n", g_limited_max_freq, g_limited_max_ncpu);
    }
    else
    {
        while (ncpu)
        {
            // table is sorted by power, from maximum to minimum
            for (i = 0; i < (g_cpufreq_power_tbl_num); i++)
            {
                if (mt_cpu_power[i].cpufreq_ncpu == ncpu)
                {
                    if (mt_cpu_power[i].cpufreq_power <= limited_power)
                    {
                        g_limited_max_ncpu = mt_cpu_power[i].cpufreq_ncpu;
                        g_limited_max_freq = mt_cpu_power[i].cpufreq_khz;
                        g_max_power_OPP = i;
                        found = 1;
                        break;
                    }
                }
            }

            if (found)
                break;

            ncpu--;
        }

        if (!found)
        {
            dprintk("thermal limit fail, not found suitable DVFS OPP\n");
        }
        else
        {
            dprintk("[thermal] max_freq = %d, max_ncpu = %d\n", g_limited_max_freq, g_limited_max_ncpu);
            hp_limited_cpu_num(g_limited_max_ncpu);

            if (num_online_cpus() > g_limited_max_ncpu)
            {
                for (i = num_online_cpus(); i > g_limited_max_ncpu; i--)
                {
                    dprintk("turn off CPU%d due to thermal protection\n", (i - 1));
                    cpu_down((i - 1));
                }
            }

            //cpufreq_driver_target(policy, g_limited_max_freq, CPUFREQ_RELATION_L);
        }
    }

    cpufreq_cpu_put(policy);

no_policy:
    return;
}
EXPORT_SYMBOL(mt_cpufreq_thermal_protect);

/***************************
* show current DVFS stauts
****************************/
static int mt_cpufreq_state_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (!mt_cpufreq_pause)
        p += sprintf(p, "DVFS enabled\n");
    else
        p += sprintf(p, "DVFS disabled\n");

    len = p - buf;
    return len;
}

/************************************
* set DVFS stauts by sysfs interface
*************************************/
static ssize_t mt_cpufreq_state_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 1)
        {
            mt_cpufreq_state_set(1);
        }
        else if (enabled == 0)
        {
            mt_cpufreq_state_set(0);
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
    }

    return count;
}

/****************************
* show current limited freq
*****************************/
static int mt_cpufreq_limited_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "g_limited_max_freq = %d, g_limited_max_ncpu = %d\n", g_limited_max_freq, g_limited_max_ncpu);

    len = p - buf;
    return len;
}

/**********************************
* limited power for thermal protect
***********************************/
static ssize_t mt_cpufreq_limited_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int power = 0;

    if (sscanf(buffer, "%u", &power) == 1)
    {
        //mt_cpufreq_thermal_protect(power);
        return count;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! please provide the maximum limited power\n");
    }

    return -EINVAL;
}

/***************************
* show current debug status
****************************/
static int mt_cpufreq_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (mt_cpufreq_debug)
        p += sprintf(p, "cpufreq debug enabled\n");
    else
        p += sprintf(p, "cpufreq debug disabled\n");

    len = p - buf;
    return len;
}

/***********************
* enable debug message
************************/
static ssize_t mt_cpufreq_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int debug = 0;

    if (sscanf(buffer, "%d", &debug) == 1)
    {
        if (debug == 0)
        {
            mt_cpufreq_debug = 0;
            return count;
        }
        else if (debug == 1)
        {
            mt_cpufreq_debug = 1;
            return count;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
    }

    return -EINVAL;
}

/***************************
* show cpufreq power info
****************************/
static int mt_cpufreq_power_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int i = 0, len = 0, pmic_setting;
    char *p = buf;
    if(off > 0){
        *eof = 1;
        return 0;
    }
    else{
        for (i = 0; i < (g_cpufreq_power_tbl_num); i++)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpu_power[%d].cpufreq_khz = %d\n", i, mt_cpu_power[i].cpufreq_khz);
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpu_power[%d].cpufreq_ncpu = %d\n", i, mt_cpu_power[i].cpufreq_ncpu);
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpu_power[%d].cpufreq_power = %d\n", i, mt_cpu_power[i].cpufreq_power);
        }
        pwrap_read(VPROC_VOSEL_CTRL, &pmic_setting);
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "VPROC_VOSEL_CTRL(0x%x) = %d\n", VPROC_VOSEL_CTRL, pmic_setting);

        for(i = 0; i < MAX_SPM_PMIC_TBL; i++){
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "spm_pmic_config[%d].tbl_idx = %d\n", i, spm_pmic_config[i].tbl_idx);
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "spm_pmic_config[%d].cpufreq_khz = %d\n", i, spm_pmic_config[i].cpufreq_khz);
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "spm_pmic_config[%d].cpufreq_volt = %d\n", i, spm_pmic_config[i].cpufreq_volt);
        }
        for(i = 0; i < mt_cpu_freqs_num; i++) {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpu_freqs[%d].cpufreq_khz = %d\n", i, mt_cpu_freqs[i].cpufreq_khz);
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpu_freqs[%d].cpufreq_volt = %d\n", i, mt_cpu_freqs[i].cpufreq_volt);
        }

        p += sprintf(p, "done\n");

        len = p - buf;
        return len;
    }
}
static int mt_cpufreq_freq_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int i, len;
    char *p = buf;

    // set eof = 1 to indicate end of file
    if(off > 0){
        *eof = 1;
        return 0;
    }
    else{
        p += sprintf(p, "Current Freq: %u \n", mt_cpu_freqs[g_cur_OPP].cpufreq_khz);
        for(i = 0; i < mt_cpu_freqs_num; i++){
            p += sprintf(p, "[%d] %u KHZ\n", i, mt_cpu_freqs[i].cpufreq_khz);
        }
        len = p - buf;
        return len;
    }
}
static ssize_t mt_cpufreq_freq_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int freq;
    struct cpufreq_policy *policy;

    policy = cpufreq_cpu_get(0);

    if (sscanf(buffer, "%u", &freq) == 1){
        if(freq >= policy->min && freq <= policy->max){
            mt_cpufreq_fix = true;
            if (cpufreq_frequency_table_target(policy, mt_cpu_freqs_table, freq, CPUFREQ_RELATION_L, &g_fix_OPP))
            {
                return -EINVAL;
            }
            cpufreq_driver_target(policy, freq, CPUFREQ_RELATION_L);
            return count;
        }
        else if(freq == 0){
            mt_cpufreq_fix = false;
        }
    }
    return -EINVAL;

}

static int mt_cpufreq_get_power_budget(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int /* i, */ len;
    char *p = buf;

    // set eof = 1 to indicate end of file
    if(off > 0){
        *eof = 1;
        return 0;
    }
    else{
        p += sprintf(p, "Current PowerBudget: %u \n", g_limited_power);
        len = p - buf;
        return len;
    }
}
static ssize_t mt_cpufreq_set_power_budget(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int power_budget;
    if (sscanf(buffer, "%u", &power_budget) == 1){

        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "Set PowerBudget: [%u]\n", power_budget);
        set_dvfs_thermal_limit(power_budget);

        return count;
    }
    else
        return -EINVAL;

}
static int mt_cpufreq_get_armpll_dds(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int /* i, */ len;
    char *p = buf;

    // set eof = 1 to indicate end of file
    if(off > 0){
        *eof = 1;
        return 0;
    }
    else{
        p += sprintf(p, "ARMPLL_CON1: [0x%x] \n", DRV_Reg32(ARMPLL_CON1));
        p += sprintf(p, "ARMPLL_CON2: [0x%x] \n", DRV_Reg32(ARMPLL_CON1+4));
        len = p - buf;
        return len;
    }

}
static ssize_t mt_cpufreq_set_armpll_dds(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int armpll_dds;
    if (sscanf(buffer, "0x%x", &armpll_dds) == 1){

        if(armpll_dds == 0){
           mt_cpufreq_fixdds = false;
        }
        else{
            mt_cpu_clock_switch(MAINPLL_SRC);

            mt65xx_reg_sync_writel(armpll_dds, ARMPLL_CON1);
            udelay(PLL_SETTLE_TIME);

            mt_cpu_clock_switch(ARMPLL_SRC);
            mt_cpufreq_fixdds = true;
        }
        return count;
    }
    else
        return -EINVAL;

}

static int mt_cpufreq_get_usb_raise(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int /* i, */ len;
    char *p = buf;

    // set eof = 1 to indicate end of file
    if(off > 0){
        *eof = 1;
        return 0;
    }
    else{
        p += sprintf(p, "fix highest freq: [%d] \n", mt_cpufreq_usb_raise);
        len = p - buf;
        return len;
    }

}
static ssize_t mt_cpufreq_set_usb_raise(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int raise_freq;
    if (sscanf(buffer, "%d", &raise_freq) == 1){

        if(raise_freq == 0){
            mt_cpufreq_usb_raise = false;
        }
        else if (raise_freq == 1){
            mt_cpufreq_usb_raise = true;
        }
        return count;
    }
    else
        return -EINVAL;

}
/*******************************************
* cpufrqe platform driver callback function
********************************************/
static int mt_cpufreq_pdrv_probe(struct platform_device *pdev)
{
    int /* pmic_ctrl, */ ret, i , pmic_idx = 0;
    #ifdef CONFIG_HAS_EARLYSUSPEND
    mt_cpufreq_early_suspend_handler.suspend = mt_cpufreq_early_suspend;
    mt_cpufreq_early_suspend_handler.resume = mt_cpufreq_late_resume;
    register_early_suspend(&mt_cpufreq_early_suspend_handler);
    #endif

    /************************************************
    * Check PTP level to define default max freq
    *************************************************/
    /*g_cpufreq_get_ptp_level = PTP_get_ptp_level();
    g_max_freq_by_ptp = DVFS_F1;

    if(g_cpufreq_get_ptp_level == 0)
        g_max_freq_by_ptp = DVFS_F1;
    else if(g_cpufreq_get_ptp_level == 2)
        g_max_freq_by_ptp = DVFS_F0_2;
    else if(g_cpufreq_get_ptp_level == 4)
        g_max_freq_by_ptp = DVFS_F0_1;
    else
        g_max_freq_by_ptp = DVFS_F1;*/
	//pwrap_write(VPROC_VOSEL_CTRL, (pmic_ctrl & 0xD)|0x2);
    /************************************************
    * voltage scaling need to wait PMIC driver ready
    *************************************************/

    //g_max_OPP = get_defmax_opp_idx();
    //g_min_OPP = get_defmin_opp_idx();
    //use g_cur_OPP, g_max_OPP and g_min_OPP here. Setup mt_cpu_freqs here
    ret = cpufreq_register_driver(&mt_cpufreq_driver);
    g_cur_OPP = get_defcur_opp_idx();
    g_normal_max_OPP = get_normal_max_opp_idx();

    // setup global information max_freq and max_ncpu are for thermal
    g_limited_max_freq = mt_cpu_freqs[0].cpufreq_khz;
    g_limited_max_ncpu = num_possible_cpus();
    g_max_power_OPP = 0;
    //g_limited_max_freq = g_max_freq_by_ptp;
    //g_limited_min_freq = DVFS_F3;


    /*if(g_cpufreq_get_ptp_level == 0)
        spm_dvfs_ctrl_volt(1); // default set to 1.15V
    else if((g_cpufreq_get_ptp_level == 2) || (g_cpufreq_get_ptp_level == 4))
        spm_dvfs_ctrl_volt(2); // default set to 1.15V
    else
        spm_dvfs_ctrl_volt(1); // default set to 1.15V*/



    // we will use 0,1,3,4,5
    //caution: this table is shared with gpu dvfs, so take care!!
    for(i = 0; i < mt_cpu_freqs_num; i++){
        if(mt_cpu_freqs[i].cpufreq_volt > DVFS_MIN_VCORE){
            spm_pmic_config[pmic_idx].cpufreq_volt = mt_cpu_freqs[i].cpufreq_volt;
            spm_pmic_config[pmic_idx].cpufreq_khz = mt_cpu_freqs[i].cpufreq_khz;
            pmic_idx++;
            // exceed table limit
            if(pmic_idx > MAX_SPM_PMIC_TBL)
            	  aee_kernel_exception("DVFS", "index > MAX_TBL: %d > %d", pmic_idx, MAX_SPM_PMIC_TBL);
        }
    }
    for(i = pmic_idx; i < MAX_SPM_PMIC_TBL; i++){
    	  spm_pmic_config[i].cpufreq_volt = DVFS_MIN_VCORE;
    	  spm_pmic_config[i].cpufreq_khz = NOR_MAX_FREQ;
    }
    for(i = 0; i < MAX_SPM_PMIC_TBL; i++){
        spm_pmic_config[i].tbl_idx = i+3;	// 3 is the start entry of spm to pmic wrapper table
    }

    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR0);
    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR1);
    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR2);
    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR3);
    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR4);
    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR5);
    mt65xx_reg_sync_writel(TOP_CKPDN0_SET, PMIC_WRAP_DVFS_ADR6);
    mt65xx_reg_sync_writel(TOP_CKPDN0_CLR, PMIC_WRAP_DVFS_ADR7);

    mt65xx_reg_sync_writel(0x58, PMIC_WRAP_DVFS_WDATA0); // 1.25V
    mt65xx_reg_sync_writel(0x48, PMIC_WRAP_DVFS_WDATA1); // 1.15V
    mt65xx_reg_sync_writel(0x38, PMIC_WRAP_DVFS_WDATA2); // 1.05V, this is for spm firmware
    mt65xx_reg_sync_writel(VOLT_TO_PMIC_VAL(spm_pmic_config[0].cpufreq_volt), PMIC_WRAP_DVFS_WDATA3); // 1.15V up + variation
    mt65xx_reg_sync_writel(VOLT_TO_PMIC_VAL(spm_pmic_config[1].cpufreq_volt), PMIC_WRAP_DVFS_WDATA4); // 1.15V up + variation
    mt65xx_reg_sync_writel(VOLT_TO_PMIC_VAL(spm_pmic_config[2].cpufreq_volt), PMIC_WRAP_DVFS_WDATA5); // 1.15V up + variation
    mt65xx_reg_sync_writel(0x2, PMIC_WRAP_DVFS_WDATA6); // 1.15V
    mt65xx_reg_sync_writel(0x2, PMIC_WRAP_DVFS_WDATA7); // 1.15V

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA0: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA0));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA1: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA1));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA2: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA2));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA3: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA3));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA4: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA4));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA5: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA5));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA6: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA6));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA7: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA7));

    mt_cpufreq_ready = true;

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mediatek cpufreq initialized\n");

    return ret;
}

/***************************************
* this function should never be called
****************************************/
static int mt_cpufreq_pdrv_remove(struct platform_device *pdev)
{
    cpufreq_unregister_driver(&mt_cpufreq_driver);
    return 0;
}

static int mt_cpufreq_suspend(struct device *device)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq_suspend\n");

    mt65xx_reg_sync_writel(STARTUP_AUXADC, PMIC_WRAP_DVFS_ADR4);
    mt65xx_reg_sync_writel(STARTUP_AUXADC, PMIC_WRAP_DVFS_ADR5);

    mt65xx_reg_sync_writel(0xE0, PMIC_WRAP_DVFS_WDATA4);
    mt65xx_reg_sync_writel(0xF0, PMIC_WRAP_DVFS_WDATA5);

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA3: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA3));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA4: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA4));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA5: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA5));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA6: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA6));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA7: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA7));

    return 0;
}

static int mt_cpufreq_resume(struct device *device)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq_resume\n");

    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR3);
    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR4);
    mt65xx_reg_sync_writel(VPROC_VOSEL_ON, PMIC_WRAP_DVFS_ADR5);

    mt65xx_reg_sync_writel(VOLT_TO_PMIC_VAL(spm_pmic_config[0].cpufreq_volt), PMIC_WRAP_DVFS_WDATA3); // 1.15V up + variation
    mt65xx_reg_sync_writel(VOLT_TO_PMIC_VAL(spm_pmic_config[1].cpufreq_volt), PMIC_WRAP_DVFS_WDATA4); // 1.15V up + variation
    mt65xx_reg_sync_writel(VOLT_TO_PMIC_VAL(spm_pmic_config[2].cpufreq_volt), PMIC_WRAP_DVFS_WDATA5); // 1.15V up + variation

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA3: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA3));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA4: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA4));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA5: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA5));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA6: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA6));
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PMIC_WRAP_DVFS_WDATA7: 0x%x\n", dvfs_read(PMIC_WRAP_DVFS_WDATA7));

    return 0;
}

static struct dev_pm_ops mt_cpufreq_pdrv_pm_ops = {
    .suspend = mt_cpufreq_suspend,
    .resume = mt_cpufreq_resume,
    .freeze = mt_cpufreq_suspend,
    .thaw = mt_cpufreq_resume,
    .poweroff = NULL,
    .restore = mt_cpufreq_resume,
    .restore_noirq = NULL,
};


static struct platform_driver mt_cpufreq_pdrv = {
    .probe      = mt_cpufreq_pdrv_probe,
    .remove     = mt_cpufreq_pdrv_remove,
    .suspend    = NULL,
    .resume     = NULL,
    .driver     = {
#ifdef CONFIG_PM
        .pm     = &mt_cpufreq_pdrv_pm_ops,
#endif
        .name   = "mt-cpufreq",
        .owner  = THIS_MODULE,
    },
};

/***********************************************************
* cpufreq initialization to register cpufreq platform driver
************************************************************/
static int __init mt_cpufreq_pdrv_init(void)
{
    int ret = 0;

    struct proc_dir_entry *mt_entry = NULL;
    struct proc_dir_entry *mt_cpufreq_dir = NULL;

    mt_cpufreq_dir = proc_mkdir("cpufreq", NULL);
    if (!mt_cpufreq_dir)
    {
        pr_err("[%s]: mkdir /proc/cpufreq failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("cpufreq_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_debug_read;
            mt_entry->write_proc = mt_cpufreq_debug_write;
        }

        mt_entry = create_proc_entry("cpufreq_limited_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_limited_power_read;
            mt_entry->write_proc = mt_cpufreq_limited_power_write;
        }

        mt_entry = create_proc_entry("cpufreq_state", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_state_read;
            mt_entry->write_proc = mt_cpufreq_state_write;
        }

        mt_entry = create_proc_entry("cpufreq_power_dump", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_power_dump_read;
        }

        mt_entry = create_proc_entry("cpufreq_cur_freq", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_freq_read;
            mt_entry->write_proc = mt_cpufreq_freq_write;
        }

        mt_entry = create_proc_entry("armpll_dds", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_get_armpll_dds;
            mt_entry->write_proc = mt_cpufreq_set_armpll_dds;
        }

        mt_entry = create_proc_entry("power_budget", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_get_power_budget;
            mt_entry->write_proc = mt_cpufreq_set_power_budget;
        }

        mt_entry = create_proc_entry("usb_raise_freq", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_get_usb_raise;
            mt_entry->write_proc = mt_cpufreq_set_usb_raise;
        }
    }

    ret = platform_driver_register(&mt_cpufreq_pdrv);
    if (ret)
    {
        xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "failed to register cpufreq driver\n");
        return ret;
    }
    else
    {
        xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "cpufreq driver registration done\n");
        return 0;
    }
}

static void __exit mt_cpufreq_pdrv_exit(void)
{
    cpufreq_unregister_driver(&mt_cpufreq_driver);
}

module_init(mt_cpufreq_pdrv_init);
module_exit(mt_cpufreq_pdrv_exit);

MODULE_DESCRIPTION("MediaTek CPU Frequency Scaling driver");
MODULE_LICENSE("GPL");
