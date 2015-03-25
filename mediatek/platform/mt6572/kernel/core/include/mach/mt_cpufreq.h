#ifndef _MT_CPUFREQ_H
#define _MT_CPUFREQ_H

#include <linux/module.h>
#include "mt_reg_base.h"

/*********************
* Clock Mux Register
**********************/
#define TOP_CKMUXSEL    (INFRA_SYS_CFG_AO_BASE)
#define TOP_CKDIV1      (INFRA_SYS_CFG_AO_BASE + 0x8)

#define PLL_MAX_FREQ         (1989000) //KHZ
#define PLL_MIN_FREQ         (130000)  //KHZ
#define PLL_DIV1_FREQ        (1001000) //KHZ
#define PLL_DIV2_FREQ        (520000)  //KHZ
#define PLL_DIV4_FREQ        (260000)  //KHZ
#define PLL_DIV8_FREQ        (PLL_MIN_FREQ)  //KHZ


#define DVFS_D0              (1599000)   // KHz, OD
#define DVFS_D1              (1404000)   // KHz, OD
#define DVFS_D2              (1300000)   // KHz, OD
#define DVFS_D3              (1209000)   // KHz, OD
#define DVFS_F1              (1001000)   // KHz
#define DVFS_F2              (806000)    // KHz
#define DVFS_F3              (598000)    // KHz

#define DVFS_V0              (1250)  // mV, OD
#define DVFS_V1              (1150)  // mV

#define DVFS_MIN_VCORE       (1150)
#define DEFAULT_FREQ         (1001000)   // KHz
#define NOR_MAX_FREQ         (1001000)   // KHz

#define CORE_NUM             (2)
#define VCORE_NUM            (2)
#define MAX_SPM_PMIC_TBL     (3)
//#define DEFAULT_OPP          (1)
//#define MAX_OPP              (0)
//#define NORMAL_MAX_OPP       (1)

/*****************************************
 * * PMIC settle time, should not be changed
 * ******************************************/
#define PMIC_SETTLE_TIME (40) // us
/*****************************************
 * * PLL settle time, should not be changed
 * ******************************************/
#define PLL_SETTLE_TIME (20) // us
/***********************************************
 * * RMAP DOWN TIMES to postpone frequency degrade
 * ************************************************/
#define RAMP_DOWN_TIMES (2)

/****************************
* PMIC Wrapper DVFS Register
*****************************/
#define PMIC_WRAP_DVFS_ADR0     (PMIC_WRAP_BASE + 0xE4)
#define PMIC_WRAP_DVFS_WDATA0   (PMIC_WRAP_BASE + 0xE8)
#define PMIC_WRAP_DVFS_ADR1     (PMIC_WRAP_BASE + 0xEC)
#define PMIC_WRAP_DVFS_WDATA1   (PMIC_WRAP_BASE + 0xF0)
#define PMIC_WRAP_DVFS_ADR2     (PMIC_WRAP_BASE + 0xF4)
#define PMIC_WRAP_DVFS_WDATA2   (PMIC_WRAP_BASE + 0xF8)
#define PMIC_WRAP_DVFS_ADR3     (PMIC_WRAP_BASE + 0xFC)
#define PMIC_WRAP_DVFS_WDATA3   (PMIC_WRAP_BASE + 0x100)
#define PMIC_WRAP_DVFS_ADR4     (PMIC_WRAP_BASE + 0x104)
#define PMIC_WRAP_DVFS_WDATA4   (PMIC_WRAP_BASE + 0x108)
#define PMIC_WRAP_DVFS_ADR5     (PMIC_WRAP_BASE + 0x10C)
#define PMIC_WRAP_DVFS_WDATA5   (PMIC_WRAP_BASE + 0x110)
#define PMIC_WRAP_DVFS_ADR6     (PMIC_WRAP_BASE + 0x114)
#define PMIC_WRAP_DVFS_WDATA6   (PMIC_WRAP_BASE + 0x118)
#define PMIC_WRAP_DVFS_ADR7     (PMIC_WRAP_BASE + 0x11C)
#define PMIC_WRAP_DVFS_WDATA7   (PMIC_WRAP_BASE + 0x120)
#define VPROC_VOSEL_ON          (0x220)
#define VPROC_VOSEL_CTRL        (0x216)
#define TOP_CKPDN0_SET          (0x104) 
#define TOP_CKPDN0_CLR          (0x106)
#define STARTUP_AUXADC          (0x4E)

#define VOLT_TO_PMIC_VAL(VOLT)  ((VOLT - 700) * 100 / 625)
#define PMIC_VAL_TO_VOLT(PMIC)  (PMIC *625 / 100 + 700 + 1)

enum cpu_src{
    CPU_SRC_MIN = 0,
    CK26M = CPU_SRC_MIN,
    ARMPLL_SRC = 1,
    UNIVPLL_SRC = 2,
    MAINPLL_SRC = 3,
    CPU_SRC_MAX = MAINPLL_SRC,
};

#define dvfs_read(addr)		    (*(volatile u32 *)(addr))

/*****************
* extern function 
******************/
extern int mt_cpufreq_state_set(int enabled);
extern void set_spm_tbl(unsigned int volt, unsigned int freq);
extern void set_dvfs_thermal_limit(unsigned int limited_power);
extern void mt_cpufreq_enable_by_ptpod(void);
extern unsigned int mt_cpufreq_disable_by_ptpod(void);
extern unsigned int mt_cpufreq_max_frequency_by_DVS(unsigned int num);
extern void mt_cpufreq_return_default_DVS_by_ptpod(void);
extern void mt_vcore_freq_volt_set(unsigned int target_volt, unsigned int target_freq);
extern unsigned int get_max_vcore(unsigned int cpu_volt, unsigned int gpu_volt);
extern unsigned int get_cur_vcore(void);
extern unsigned int mt_cpufreq_cur_volt(void);
extern int GPU_freq_output(void);
extern int CPU_freq_output(void);
extern void restore_default_volt(void);
#endif
