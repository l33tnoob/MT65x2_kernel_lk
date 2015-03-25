// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/**
 * @file mt_spm_mtcmos_internal.h
 * @brief SPM MTCMOS driver private interface
 */

#ifndef __MT_SPM_MTCMOS_INTERNAL_H__
#define __MT_SPM_MTCMOS_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================*/
// Include files
/*=============================================================*/

// system includes
#include <linux/kernel.h>

// project includes
#include <mach/mt_spm.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_reg_base.h>
#include <mach/sync_write.h>

// local includes

// forward references


/*=============================================================*/
// Macro definition
/*=============================================================*/

//
// Register Base Address
//

//
// SPM
//
/*
#define SPM_MFG_PWR_CON             (SPM_BASE + 0x0214)
#define SPM_DIS_PWR_CON             (SPM_BASE + 0x023c)
#define SPM_CONN_PWR_CON            (SPM_BASE + 0x0280)
#define SPM_MD_PWR_CON              (SPM_BASE + 0x0284)
#define SPM_PCM_REG13_DATA          (SPM_BASE + 0x03b4)
#define SPM_PWR_STATUS              (SPM_BASE + 0x060c)
#define SPM_PWR_STATUS_S            (SPM_BASE + 0x0610)
*/
//
// INFRACFG_AO
//

#define INFRACFG_AO_BASE            INFRA_SYS_CFG_AO_BASE

#define TOPAXI_SI0_CTL              (INFRACFG_AO_BASE + 0x0200) // TODO: review it
#define INFRA_TOPAXI_PROTECTEN      (INFRACFG_AO_BASE + 0x0220)
#define INFRA_TOPAXI_PROTECTSTA1    (INFRACFG_AO_BASE + 0x0228)


#define PWR_RST_B_BIT               BIT(0)          // @ SPM_XXX_PWR_CON
#define PWR_ISO_BIT                 BIT(1)          // @ SPM_XXX_PWR_CON
#define PWR_ON_BIT                  BIT(2)          // @ SPM_XXX_PWR_CON
#define PWR_ON_S_BIT                BIT(3)          // @ SPM_XXX_PWR_CON
#define PWR_CLK_DIS_BIT             BIT(4)          // @ SPM_XXX_PWR_CON
#define SRAM_CKISO_BIT              BIT(5)          // @ SPM_FC0_PWR_CON or SPM_CPU_PWR_CON
#define SRAM_ISOINT_B_BIT           BIT(6)          // @ SPM_FC0_PWR_CON or SPM_CPU_PWR_CON
#define SRAM_PDN_BITS               BITMASK(11:8)   // @ SPM_XXX_PWR_CON
#define SRAM_PDN_ACK_BITS           BITMASK(15:12)  // @ SPM_XXX_PWR_CON


/*=============================================================*/
// Type definition
/*=============================================================*/


/*=============================================================*/
// Global variable definition
/*=============================================================*/


/*=============================================================*/
// Global function definition
/*=============================================================*/


#ifdef __cplusplus
}
#endif

#endif // __MT_SPM_MTCMOS_INTERNAL_H__

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/**
 * @file mt_spm_mtcmos.h
 * @brief SPM MTCMOS driver interface
 */

#ifndef __MT_SPM_MTCMOS_H__
#define __MT_SPM_MTCMOS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================*/
// Include files
/*=============================================================*/

// system includes

// project includes

// local includes

// forward references


/*=============================================================*/
// Macro definition
/*=============================================================*/

#define STA_POWER_DOWN  0
#define STA_POWER_ON    1

#define MD1_PWR_STA_MASK    BIT(0)
#define CON_PWR_STA_MASK    BIT(1)
#define DPY_PWR_STA_MASK    BIT(2)
#define DIS_PWR_STA_MASK    BIT(3)
#define MFG_PWR_STA_MASK    BIT(4)
#define ISP_PWR_STA_MASK    BIT(5)
#define IFR_PWR_STA_MASK    BIT(6)
#define VDE_PWR_STA_MASK    BIT(7)
#define CPU_PWR_STA_MASK    BIT(8)
#define FC1_PWR_STA_MASK    BIT(11)
#define FC0_PWR_STA_MASK    BIT(12)
#define MCU_PWR_STA_MASK    BIT(13)


/*=============================================================*/
// Type definition
/*=============================================================*/


/*=============================================================*/
// Global variable definition
/*=============================================================*/


/*=============================================================*/
// Global function definition
/*=============================================================*/

#ifdef __MT_SPM_MTCMOS_C__
  #define SPM_EXTERN
#else
  #define SPM_EXTERN extern
#endif

/*
 * 1. for CPU MTCMOS: CPU0, CPU1, DBG, CPUSYS
 * 2. call spm_mtcmos_cpu_lock/unlock() before/after any operations
 */
extern int spm_mtcmos_ctrl_cpu0(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu1(int state, int chkWfiBeforePdn);

#if 0
/*
 * 1. for non-CPU MTCMOS: VDEC, VENC, ISP, DISP, MFG, INFRA, DDRPHY, MDSYS1, MDSYS2
 * 2. call spm_mtcmos_noncpu_lock/unlock() before/after any operations
 */
//extern void spm_mtcmos_noncpu_lock(unsigned long *flags);
//extern void spm_mtcmos_noncpu_unlock(unsigned long *flags);

SPM_EXTERN int spm_mtcmos_ctrl_mdsys1(int state);
SPM_EXTERN int spm_mtcmos_ctrl_connsys(int state);
SPM_EXTERN int spm_mtcmos_ctrl_disp(int state);
SPM_EXTERN int spm_mtcmos_ctrl_mfg(int state);
#endif

SPM_EXTERN int test_spm_gpu_power_on(void);

#undef SPM_EXTERN

#ifdef __cplusplus
}
#endif

// TODO: which headfile is better for these definitions

#endif // __MT_SPM_MTCMOS_H__
