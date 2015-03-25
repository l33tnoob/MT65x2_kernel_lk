#ifndef _MT_GPUFREQ_H
#define _MT_GPUFREQ_H

#include <linux/module.h>

/*********************
* GPU Frequency List
**********************/
#define GPU_DVFS_D0     (565500)   // KHz
#define GPU_DVFS_F0     (500500)   // KHz
#define GPU_DVFS_F1     (416000)   // KHz
#define GPU_DVFS_F2     (299000)   // KHz

#define GPU_LEVEL_INDEX (16)
#define GPU_LEVEL_0     (0x1)
/******************************
* MFG Power Voltage Selection
*******************************/
#define GPU_VCORE_V0 (1250)
#define GPU_VCORE_V1 (1150)

/*****************************************
* PMIC settle time, should not be changed
******************************************/
#define PMIC_SETTLE_TIME (40) // us

/****************************************************************
* enable this option to calculate clock on ration in period time.
*****************************************************************/
/****************************************************************
* Default disable gpu dvfs.
*****************************************************************/
//#define GPU_DVFS_DEFAULT_DISABLED

/********************************************
* enable this option to adjust buck voltage
*********************************************/
#define MT_BUCK_ADJUST

struct mt_gpufreq_info
{
    unsigned int gpufreq_khz;
    unsigned int gpufreq_lower_bound;
    unsigned int gpufreq_upper_bound;
    unsigned int gpufreq_volt;
    unsigned int gpufreq_remap;
};

struct mt_gpufreq_power_info
{
    unsigned int gpufreq_khz;
    unsigned int gpufreq_volt;
    unsigned int gpufreq_power;
};

/*****************
* extern function 
******************/
extern int mt_gpufreq_state_set(int enabled);
extern void mt_gpufreq_thermal_protect(unsigned int limited_power);
extern bool mt_gpufreq_is_registered_get(void);
extern unsigned int mt_gpufreq_min_power(void);
extern unsigned int mt_gpufreq_max_power(void);
extern unsigned int mt_gpufreq_cur_volt(void);
extern unsigned int mt_gpufreq_cur_freq(void);
extern unsigned int mt_gpufreq_cur_load(void);
extern unsigned int get_gpu_level(void);
extern int mt_gpufreq_non_register(void);
extern void mt_gpufreq_set_initial(unsigned int freq_new, unsigned int volt_new);
#endif
