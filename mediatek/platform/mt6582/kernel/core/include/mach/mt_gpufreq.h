#ifndef _MT_GPUFREQ_H
#define _MT_GPUFREQ_H

#include <linux/module.h>

/*********************
* Clock Mux Register
**********************/
#define CLK26CALI       (0xF00001C0) // FIX ME, No this register
#define CLK_MISC_CFG_0  (0xF0000210)
#define CLK_MISC_CFG_1  (0xF0000214)
#define CLK26CALI_0     (0xF0000220)
#define CLK26CALI_1     (0xF0000224)
#define CLK26CALI_2     (0xF0000228)
#define MBIST_CFG_0     (0xF0000308)
#define MBIST_CFG_1     (0xF000030C)
#define MBIST_CFG_2     (0xF0000310)
#define MBIST_CFG_3     (0xF0000314)

/*********************
* GPU Frequency List
**********************/
#define GPU_DVFS_F1     (476666)   // KHz
#define GPU_DVFS_F2     (403000)   // KHz
#define GPU_DVFS_F3     (357500)   // KHz
#define GPU_DVFS_F4     (312000)   // KHz
#define GPU_DVFS_F5     (286000)   // KHz
#define GPU_DVFS_F6     (268666)   // KHz
#define GPU_DVFS_F7     (238333)   // KHz
#define GPU_DVFS_F8     (156000)   // KHz

/**************************
* MFG Clock Mux Selection
***************************/
#define GPU_MMPLL_D3       (GPU_DVFS_F1)
#define GPU_SYSPLL_D2      (GPU_DVFS_F2)
#define GPU_MMPLL_D4       (GPU_DVFS_F3)
#define GPU_UNIVPLL1_D2    (GPU_DVFS_F4)
#define GPU_MMPLL_D5       (GPU_DVFS_F5)
#define GPU_SYSPLL_D3      (GPU_DVFS_F6)
#define GPU_MMPLL_D6       (GPU_DVFS_F7)
#define GPU_UNIVPLL1_D4    (GPU_DVFS_F8)

/******************************
* MFG Power Voltage Selection
*******************************/
#define GPU_POWER_VCORE_1_05V   (64)
#define GPU_POWER_VRF18_1_05V   (0)
#define GPU_POWER_VRF18_1_10V   (2)
#define GPU_POWER_VRF18_1_15V   (4)

/*****************************************
* PMIC settle time, should not be changed
******************************************/
#define PMIC_SETTLE_TIME (40) // us


/****************************************************************
* enable this option to calculate clock on ration in period time.
*****************************************************************/
#define GPU_CLOCK_RATIO
#ifdef GPU_CLOCK_RATIO
#define GPU_COUNT_OVERFLOW 1000000 // 1 sec
#endif

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
    unsigned int gpufreq_power;
};

#ifdef GPU_CLOCK_RATIO
/* GPU DVFS clock on ratio */
enum mt_gpufreq_clock_ratio {
	GPU_DVFS_CLOCK_RATIO_OFF = 0,  /* GPU clock turned off */
	GPU_DVFS_CLOCK_RATIO_ON	    ,  /* GPU clock turned on */
	GPU_DVFS_CLOCK_RATIO_GET       /* Get GPU clock on ratio in a period time */
};
#endif

/*****************
* extern function 
******************/
extern int mt_gpufreq_state_set(int enabled);
extern void mt_gpufreq_thermal_protect(unsigned int limited_power);
extern int mt_gpufreq_register(struct mt_gpufreq_info *freqs, int num);
extern bool mt_gpufreq_is_registered_get(void);
#ifdef GPU_CLOCK_RATIO
extern unsigned int mt_gpufreq_gpu_clock_ratio(enum mt_gpufreq_clock_ratio act);
#endif
extern unsigned int mt_gpufreq_cur_freq(void);
extern unsigned int mt_gpufreq_cur_load(void);
extern int mt_gpufreq_non_register(void);
extern void mt_gpufreq_set_initial(unsigned int freq_new, unsigned int volt_new);
#endif
