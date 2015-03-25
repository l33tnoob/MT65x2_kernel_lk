#ifndef _MT_SPM_
#define _MT_SPM_

#include <linux/kernel.h>
#include <linux/xlog.h>
#include <linux/spinlock.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_spm_pcm.h>
#include <mach/sync_write.h>
#include <linux/aee.h>
/*
 * for SPM register control
 */
#define  SPM_POWERON_CONFIG_SET      (SPM_BASE + 0x0000)
#define  SPM_POWER_ON_VAL0           (SPM_BASE + 0x0010)
#define  SPM_POWER_ON_VAL1           (SPM_BASE + 0x0014)
#define  SPM_CLK_SETTLE              (SPM_BASE + 0x0100)
#define  SPM_FC0_PWR_CON             (SPM_BASE + 0x0200)
#define  SPM_DBG_PWR_CON             (SPM_BASE + 0x0204)
#define  SPM_CPU_PWR_CON             (SPM_BASE + 0x0208)
#define  SPM_NEON0_PWR_CON           (SPM_BASE + 0x020C)
#define  SPM_VDE_PWR_CON             (SPM_BASE + 0x0210)
#define  SPM_MFG_PWR_CON             (SPM_BASE + 0x0214)
#define  SPM_FC1_PWR_CON             (SPM_BASE + 0x0218)
#define  SPM_FC2_PWR_CON             (SPM_BASE + 0x021C)
#define  SPM_FC3_PWR_CON             (SPM_BASE + 0x0220)
#define  SPM_NEON1_PWR_CON           (SPM_BASE + 0x0224)
#define  SPM_NEON2_PWR_CON           (SPM_BASE + 0x0228)
#define  SPM_NEON3_PWR_CON           (SPM_BASE + 0x022C)
#define  SPM_VEN_PWR_CON             (SPM_BASE + 0x0230)
#define  SPM_IFR_PWR_CON             (SPM_BASE + 0x0234)
#define  SPM_ISP_PWR_CON             (SPM_BASE + 0x0238)
#define  SPM_DIS_PWR_CON             (SPM_BASE + 0x023C)
#define  SPM_DPY_PWR_CON             (SPM_BASE + 0x0240)
#define  SPM_CONN_PWR_CON            (SPM_BASE + 0x0280)
#define  SPM_MD1_PWR_CON             (SPM_BASE + 0x0284)
#define  SPM_CPU_L2_DAT_PDN          (SPM_BASE + 0x0244)
#define  SPM_CPU_L2_DAT_SLEEP_B      (SPM_BASE + 0x0248)
#define  SPM_MP_CORE0_AUX            (SPM_BASE + 0x024C)
#define  SPM_MP_CORE1_AUX            (SPM_BASE + 0x0250)
#define  SPM_MP_CORE2_AUX            (SPM_BASE + 0x0254)
#define  SPM_MP_CORE3_AUX            (SPM_BASE + 0x0258)
#define  SPM_CPU_FC0_L1_PDN          (SPM_BASE + 0x025C)
#define  SPM_CPU_FC0_L1_SLEEP_B      (SPM_BASE + 0x0260)
#define  SPM_CPU_FC1_L1_PDN          (SPM_BASE + 0x0264)
#define  SPM_CPU_FC1_L1_SLEEP_B      (SPM_BASE + 0x0268)
#define  SPM_CPU_FC2_L1_PDN          (SPM_BASE + 0x026C)
#define  SPM_CPU_FC2_L1_SLEEP_B      (SPM_BASE + 0x0270)
#define  SPM_CPU_FC3_L1_PDN          (SPM_BASE + 0x0274)
#define  SPM_CPU_FC3_L1_SLEEP_B      (SPM_BASE + 0x0278)
#define  SPM_IFR_FH_SRAM_CTRL        (SPM_BASE + 0x027C)
#define  SPM_CONN_PWR_CON            (SPM_BASE + 0x0280)
#define  SPM_MD_PWR_CON              (SPM_BASE + 0x0284)
#define  SPM_GSTA_PWR_CON            (SPM_BASE + 0x0288)
#define  SPM_GSTA_PWR_STA            (SPM_BASE + 0x028C)
#define  SPM_MCU_PWR_CON             (SPM_BASE + 0x0290)
#define  SPM_IFR_SRAMROM_CON         (SPM_BASE + 0x0294)
#define  SPM_PCM_CON0                (SPM_BASE + 0x0310)
#define  SPM_PCM_CON1                (SPM_BASE + 0x0314)
#define  SPM_PCM_IM_PTR              (SPM_BASE + 0x0318)
#define  SPM_PCM_IM_LEN              (SPM_BASE + 0x031C)
#define  SPM_PCM_REG_DATA_INI        (SPM_BASE + 0x0320)
#define  SPM_PCM_REG0_DATA           (SPM_BASE + 0x0380)
#define  SPM_PCM_REG1_DATA           (SPM_BASE + 0x0384)
#define  SPM_PCM_REG2_DATA           (SPM_BASE + 0x0388)
#define  SPM_PCM_REG3_DATA           (SPM_BASE + 0x038C)
#define  SPM_PCM_REG4_DATA           (SPM_BASE + 0x0390)
#define  SPM_PCM_REG5_DATA           (SPM_BASE + 0x0394)
#define  SPM_PCM_REG6_DATA           (SPM_BASE + 0x0398)
#define  SPM_PCM_REG7_DATA           (SPM_BASE + 0x039C)
#define  SPM_PCM_REG8_DATA           (SPM_BASE + 0x03A0)
#define  SPM_PCM_REG9_DATA           (SPM_BASE + 0x03A4)
#define  SPM_PCM_REG10_DATA          (SPM_BASE + 0x03A8)
#define  SPM_PCM_REG11_DATA          (SPM_BASE + 0x03AC)
#define  SPM_PCM_REG12_DATA          (SPM_BASE + 0x03B0)
#define  SPM_PCM_REG13_DATA          (SPM_BASE + 0x03B4)
#define  SPM_PCM_REG14_DATA          (SPM_BASE + 0x03B8)
#define  SPM_PCM_REG15_DATA          (SPM_BASE + 0x03BC)
#define  SPM_PCM_EVENT_REG_STA       (SPM_BASE + 0x03C0)
#define  SPM_PCM_FSM_STA             (SPM_BASE + 0x03C4)
#define  SPM_PCM_IM_HOST_RW_PTR      (SPM_BASE + 0x03C8)
#define  SPM_PCM_IM_HOST_RW_DAT      (SPM_BASE + 0x03CC)
#define  SPM_PCM_EVENT_VECTOR0       (SPM_BASE + 0x0340)
#define  SPM_PCM_EVENT_VECTOR1       (SPM_BASE + 0x0344)
#define  SPM_PCM_EVENT_VECTOR2       (SPM_BASE + 0x0348)
#define  SPM_PCM_EVENT_VECTOR3       (SPM_BASE + 0x034C)
#define  SPM_PCM_EVENT_VECTOR4       (SPM_BASE + 0x03D0)
#define  SPM_PCM_EVENT_VECTOR5       (SPM_BASE + 0x03D4)
#define  SPM_PCM_EVENT_VECTOR6       (SPM_BASE + 0x03D8)
#define  SPM_PCM_EVENT_VECTOR7       (SPM_BASE + 0x03DC)
#define  SPM_PCM_MAS_PAUSE_MASK      (SPM_BASE + 0x0354)
#define  SPM_PCM_PWR_IO_EN           (SPM_BASE + 0x0358)
#define  SPM_PCM_TIMER_VAL           (SPM_BASE + 0x035C)
#define  SPM_PCM_TIMER_OUT           (SPM_BASE + 0x0360)
#define  SPM_PCM_SW_INT_SET          (SPM_BASE + 0x03E0)
#define  SPM_PCM_SW_INT_CLEAR        (SPM_BASE + 0x03E4)
#define  SPM_CLK_CON                 (SPM_BASE + 0x0400)
#define  SPM_APMCU_PWRCTL            (SPM_BASE + 0x0600)
#define  SPM_AP_DVFS_CON_SET         (SPM_BASE + 0x0604)
#define  SPM_AP_STANBY_CON           (SPM_BASE + 0x0608)
#define  SPM_PWR_STATUS              (SPM_BASE + 0x060C)
#define  SPM_PWR_STATUS_S            (SPM_BASE + 0x0610)
#define  SPM_SLEEP_TIMER_STA         (SPM_BASE + 0x0720)
#define  SPM_SLEEP_TWAM_CON          (SPM_BASE + 0x0760)
#define  SPM_SLEEP_TWAM_STATUS0      (SPM_BASE + 0x0764)
#define  SPM_SLEEP_TWAM_STATUS1      (SPM_BASE + 0x0768)
#define  SPM_SLEEP_TWAM_STATUS2      (SPM_BASE + 0x076C)
#define  SPM_SLEEP_TWAM_STATUS3      (SPM_BASE + 0x0770)
#define  SPM_SLEEP_WAKEUP_EVENT_MASK (SPM_BASE + 0x0810)
#define  SPM_SLEEP_CPU_WAKEUP_EVENT  (SPM_BASE + 0x0814)
#define  SPM_PCM_WDT_TIMER_VAL       (SPM_BASE + 0x0824)
#define  SPM_SLEEP_ISR_MASK          (SPM_BASE + 0x0900)
#define  SPM_SLEEP_ISR_STATUS        (SPM_BASE + 0x0904)
#define  SPM_SLEEP_WAKEUP_DBG_EDGE   (SPM_BASE + 0x0908)
#define  SPM_ATB_LMU_CON0            (SPM_BASE + 0x090C)
#define  SPM_SLEEP_ISR_RAW_STA       (SPM_BASE + 0x0910)
#define  SPM_CORE0_WFI_SEL           (SPM_BASE + 0x0F00)
#define  SPM_CORE1_WFI_SEL           (SPM_BASE + 0x0F04)
#define  SPM_CORE2_WFI_SEL           (SPM_BASE + 0x0F08)
#define  SPM_CORE3_WFI_SEL           (SPM_BASE + 0x0F0C)
#define  SPM_PCM_RESERVE             (SPM_BASE + 0x0B00)
#define  SPM_PCM_SRC_REQ             (SPM_BASE + 0x0B04)
#define  SPM_SLEEP_CPU_IRQ_MASK      (SPM_BASE + 0x0B10)
#define  SPM_PCM_DEBUG_CON           (SPM_BASE + 0x0B20)

#define SPM_PROJECT_CODE            0xb16

#define SPM_CORE0_WFI_SEL_SW_MASK ( 1U << 0 )
#define SPM_CORE1_WFI_SEL_SW_MASK ( 1U << 1 )


#define spm_read(addr)              (*(volatile u32 *)(addr))
//#define spm_write(addr, val)        (*(volatile u32 *)(addr) = (u32)(val))
//#define spm_write(addr, val)        do{mt65xx_reg_sync_writel(val, addr);mdelay(1);}while(0)
#define spm_write(addr, val)        mt65xx_reg_sync_writel(val, addr)
#define spm_write_sync()            mb()


//#define spm_is_wakesrc_invalid(wakesrc)     (!!((u32)(wakesrc) & 0x33820000))

#if 0
#define spm_check_dramc_for_pcm()                               \
    BUG_ON((spm_read(0xf00041dc) & 0x00ff0000) != 0x00670000 || \
           (spm_read(0xf0004044) & 0xff000000) != 0x9f000000)
#endif

#define spm_emerg(fmt, args...)     printk(KERN_EMERG "[SPM] " fmt, ##args)
#define spm_alert(fmt, args...)     printk(KERN_ALERT "[SPM] " fmt, ##args)
#define spm_crit(fmt, args...)      printk(KERN_CRIT "[SPM] " fmt, ##args)
#define spm_error(fmt, args...)     printk(KERN_ERR "[SPM] " fmt, ##args)
//#define spm_error(fmt, args...)
#define spm_warning(fmt, args...)   printk(KERN_WARNING "[SPM] " fmt, ##args)
#define spm_notice(fmt, args...)    printk(KERN_NOTICE "[SPM] " fmt, ##args)
#define spm_info(fmt, args...)      printk(KERN_INFO "[SPM] " fmt, ##args)
#define spm_debug(fmt, args...)     printk(KERN_DEBUG "[SPM] " fmt, ##args)

#define wfi_with_sync()                         \
do {                                            \
    isb();                                      \
    dsb();                                      \
    __asm__ __volatile__("wfi" : : : "memory"); \
} while (0)

#define spm_crit2(fmt, args...)     \
do {                                \
    aee_sram_printk(fmt, ##args);   \
    spm_crit(fmt, ##args);          \
} while (0)

#define spm_error2(fmt, args...)    \
do {                                \
    aee_sram_printk(fmt, ##args);   \
    spm_error(fmt, ##args);         \
} while (0)

extern spinlock_t spm_lock;

extern void mtk_wdt_suspend(void);
extern void mtk_wdt_resume(void);
extern int mt_irq_mask_all(struct mtk_irq_mask *mask);
extern int mt_irq_mask_restore(struct mtk_irq_mask *mask);
extern void mt_irq_unmask_for_sleep(unsigned int irq);
extern void mtk_uart_restore(void);
extern void spm_clean_after_wakeup(void);

extern bool spm_is_md_sleep(void);
extern bool spm_is_conn_sleep(void);
extern void spm_fs_init(void);
extern void spm_module_init(void);
extern void spm_mcdi_wfi(void);
extern void spm_direct_disable_sodi(void);
extern void spm_direct_enable_sodi(void);

extern void spm_wdt_restart(void); //used to kick wdt.
extern void spm_wdt_config(bool enable);//used to enable/disable spm wdt.
extern void spm_wdt_set_reset_length(unsigned int value);//used to set wdt timeout interval
extern void spm_wdt_set_cb(int (*FuncPtr)(void));

/**************************LDVT Options for Suspend*******************************/
#ifdef CONFIG_MTK_LDVT
    //#define LDVT_SPM_SUSPEND_STRESS_TEST
        #define LDVT_SPM_SUSPEND_GPT_TEST
        #define LDVT_SPM_SUSPEND_KPT_TEST
        
    #define LDVT_SPM_SUSPEND_WDT_TEST
#endif

//#define ADVT_SPM_SUSPEND_STRESS_TEST

#endif
