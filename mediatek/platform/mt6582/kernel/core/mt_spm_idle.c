#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>  
#include <linux/aee.h>

#include <mach/irqs.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_idle.h>
#include <mach/mt_dormant.h>
#include <mach/mt_gpt.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_spm_sleep.h>

#include <asm/hardware/gic.h>

//#include <mach/wd_api.h>

#ifdef SPM_SODI_ENABLED

#define SPM_MCDI_DEBUG 0
#define SPM_MCDI_BYPASS_SYSPWREQ 1


//DEFINE_SPINLOCK(spm_sodi_lock);

s32 gSpm_Sodi_Disable_Counter = 0;
bool gSpm_IsLcmVideoMode = TRUE;
u32 gSPM_SODI_EN = 0;  // flag for idle task

#define PCM_WDT_TIMEOUT         (30 * 32768)    /* 30s */
#define PCM_TIMER_MAX_FOR_WDT   (0xffffffff - PCM_WDT_TIMEOUT)


#define mcdi_wfi_with_sync()                         \
do {                                            \
    isb();                                      \
    dsb();                                      \
    __asm__ __volatile__("wfi" : : : "memory"); \
} while (0)


//TODO: need check
#if SPM_MCDI_BYPASS_SYSPWREQ    
    #define WAKE_SRC_FOR_MCDI                     \
    (WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | WAKE_SRC_CPU0_IRQ | WAKE_SRC_CPU1_IRQ | WAKE_SRC_SYSPWREQ )
     //(WAKE_SRC_PCM_TIMER | WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | WAKE_SRC_CPU0_IRQ | WAKE_SRC_CPU1_IRQ | WAKE_SRC_SYSPWREQ )
#else    
    #define WAKE_SRC_FOR_MCDI                     \
    (WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | WAKE_SRC_CPU0_IRQ | WAKE_SRC_CPU1_IRQ )
    //   (WAKE_SRC_PCM_TIMER | WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | WAKE_SRC_CPU0_IRQ | WAKE_SRC_CPU1_IRQ )
#endif
#define WAKE_SRC_FOR_SODI \
    (WAKE_SRC_KP | WAKE_SRC_GPT | WAKE_SRC_EINT | WAKE_SRC_CCIF_MD |      \
     WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN | WAKE_SRC_AFE |                 \
     WAKE_SRC_SYSPWREQ | WAKE_SRC_MD_WDT | WAKE_SRC_CONN_WDT | WAKE_SRC_CONN | WAKE_SRC_THERM| WAKE_SRC_CPU0_IRQ)


// ==========================================
// PCM code for SODI (Screen On Deep Idle)  pcm_sodi_v0.15_20130802_MT6582
//
// core 0 : GPT 4
// ==========================================
static u32 __pcm_sodidle[] = {

    0x88000000, 0xfffffffb, 0x89c00007, 0xfffffffd, 0x89c00007, 0xfffbfeff,
    0x1950001f, 0x10006360, 0x02401409, 0xa9c00007, 0x00010400, 0x1b00001f,
    0xbfffe7ff, 0xf0000000, 0x17c07c1f, 0x1b80001f, 0x20000020, 0x8980000d,
    0x00000010, 0xd82002e6, 0x17c07c1f, 0x1b80001f, 0x20000fdf, 0x8880000d,
    0x00000124, 0xd80005a2, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x8880000c,
    0x3fffe7ff, 0xd80005a2, 0x17c07c1f, 0x89c00007, 0xfffeffff, 0xa1d40407,
    0x1b80001f, 0x20000008, 0xa1d90407, 0xc0c00e80, 0x17c07c1f, 0xd80005a3,
    0x17c07c1f, 0xa8000000, 0x00000004, 0x1b00001f, 0x7fffe7ff, 0xf0000000,
    0x17c07c1f, 0xe0e00f16, 0x1380201f, 0xe0e00f1e, 0x1380201f, 0xe0e00f0e,
    0x1380201f, 0xe0e00f0c, 0xe0e00f0d, 0xe0e00e0d, 0xe0e00c0d, 0xe0e0080d,
    0xe0e0000d, 0xf0000000, 0x17c07c1f, 0x6aa00003, 0x10006234, 0xd800092a,
    0x17c07c1f, 0xe0e00f0d, 0xe0e00f0f, 0xe0e00f1e, 0xe0e00f12, 0xd8200aaa,
    0x17c07c1f, 0xe8208000, 0x10006234, 0x000f0f0d, 0xe8208000, 0x10006234,
    0x000f0f0f, 0xe8208000, 0x10006234, 0x000f0f1e, 0xe8208000, 0x10006234,
    0x000f0f12, 0xf0000000, 0x17c07c1f, 0xd8000bea, 0x17c07c1f, 0xe2e00036,
    0x1380201f, 0xe2e0003e, 0x1380201f, 0xe2e0002e, 0x1380201f, 0xd8200cea,
    0x17c07c1f, 0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f, 0x20000020,
    0xe2e0004d, 0xf0000000, 0x17c07c1f, 0xd8000daa, 0x17c07c1f, 0xe2e0006d,
    0xe2e0002d, 0xd8200e4a, 0x17c07c1f, 0xe2e0002f, 0xe2e0003e, 0xe2e00032,
    0xf0000000, 0x17c07c1f, 0x89c00007, 0xfffffbff, 0xa9c00007, 0x00000200,
    0x1950001f, 0x100041dc, 0xba008005, 0xff00ffff, 0x000a0000, 0xe1000008,
    0xa1d08407, 0x1b80001f, 0x20000020, 0x80eab401, 0x1a00001f, 0x10006814,
    0xe2000003, 0xba008005, 0xff00ffff, 0x00640000, 0xe1000008, 0xf0000000,
    0x17c07c1f, 0x1a00001f, 0x10006604, 0xd8001223, 0x17c07c1f, 0xd8201223,
    0x17c07c1f, 0xf0000000, 0x17c07c1f, 0x1a10001f, 0x10002058, 0x1a80001f,
    0x10002058, 0xaa000008, 0x80000000, 0xe2800008, 0xf0000000, 0x17c07c1f,
    0xa1d40407, 0x1b80001f, 0x20000008, 0xa1d90407, 0xf0000000, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0x12407c1f, 0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0xd00f0000, 0x8880000c,
    0x3fffe7ff, 0xd8003242, 0x17c07c1f, 0x1950001f, 0x10006400, 0x80d70405,
    0xd8002683, 0x17c07c1f, 0x89c00007, 0xffffefff, 0x18c0001f, 0x10006200,
    0xc0c00d20, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000001, 0x1b80001f,
    0x20000080, 0xc0c00d20, 0x1280041f, 0x18c0001f, 0x10006208, 0xc0c00d20,
    0x12807c1f, 0xe8208000, 0x10006248, 0x00000000, 0x1b80001f, 0x20000080,
    0xc0c00d20, 0x1280041f, 0xa9c00007, 0x00000080, 0xc0c01260, 0x17c07c1f,
    0xa8000000, 0x00000002, 0xa8000000, 0x00000200, 0xa8000000, 0x00800000,
    0xa8000000, 0x00020000, 0x1890001f, 0x10006400, 0x80868801, 0xd82027e2,
    0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0xd0100000, 0xd0002d00,
    0x17c07c1f, 0x1b00001f, 0xffffffff, 0x8880000c, 0x3fffe7ff, 0xd8002d02,
    0x17c07c1f, 0x8880000c, 0x40000000, 0xd80027e2, 0x17c07c1f, 0x89c00007,
    0xfffeffff, 0xc0c01380, 0x17c07c1f, 0xc0c00e80, 0x17c07c1f, 0xd8002ba3,
    0x17c07c1f, 0xa8000000, 0x00000004, 0xe8208000, 0x10006310, 0x0b160038,
    0x1b00001f, 0x7fffe7ff, 0x1b80001f, 0x90100000, 0xe8208000, 0x10006310,
    0x0b160008, 0x88000000, 0xfffffffb, 0x89c00007, 0xfffffffd, 0x89c00007,
    0xfffbfeff, 0x1950001f, 0x10006360, 0x02401409, 0xa9c00007, 0x00010400,
    0x80d70405, 0xd8003243, 0x17c07c1f, 0x88000000, 0xfffdffff, 0x1b80001f,
    0x20000300, 0x88000000, 0xff7fffff, 0x1b80001f, 0x20000300, 0x88000000,
    0xfffffdff, 0x88000000, 0xfffffffd, 0x1b80001f, 0x2000049c, 0x89c00007,
    0xffffff7f, 0x18c0001f, 0x10006208, 0x1212841f, 0xc0c00ae0, 0x12807c1f,
    0xe8208000, 0x10006248, 0x00000001, 0x1b80001f, 0x20000080, 0xc0c00ae0,
    0x1280041f, 0x18c0001f, 0x10006200, 0xc0c00ae0, 0x12807c1f, 0xe8208000,
    0x1000625c, 0x00000000, 0x1b80001f, 0x20000080, 0xc0c00ae0, 0x1280041f,
    0xa9c00007, 0x00011000, 0xe8208000, 0x10006b10, 0x00000000, 0x19c0001f,
    0x00015820, 0x12c0241f, 0x10007c1f, 0x80cab001, 0x808cb401, 0x80800c02,
    0xd8203422, 0x17c07c1f, 0xa1d78407, 0x1240301f, 0xe8208000, 0x100063e0,
    0x00000001, 0x1b00001f, 0x00202000, 0x1b80001f, 0x80001000, 0x8880000c,
    0x00200000, 0xd8003642, 0x17c07c1f, 0xe8208000, 0x100063e0, 0x00000002,
    0x1b80001f, 0x00001000, 0x809c840d, 0xd82034a2, 0x17c07c1f, 0xa1d78407,
    0x1890001f, 0x10006014, 0x18c0001f, 0x10006014, 0xa0978402, 0xe0c00002,
    0x1b80001f, 0x00001000, 0xf0000000

};

#define MCDI_PCM_PC_0      0
#define MCDI_PCM_PC_1      15
    
static const pcm_desc_t pcm_sodidle = {
    .base   = __pcm_sodidle,
    .size   = 445,
    .vec0   = EVENT_VEC(30, 1, 0, MCDI_PCM_PC_0),       /* AP-wake event */
    .vec1   = EVENT_VEC(31, 1, 0, MCDI_PCM_PC_1),       /* AP-sleep event */
};


#define spm_error2(fmt, args...)    \
do {                                \
    aee_sram_printk(fmt, ##args);   \
    spm_error(fmt, ##args);         \
} while (0)


typedef struct {
    u32 debug_reg;      /* PCM_REG_DATA_INI */
    u32 r12;            /* PCM_REG12_DATA */
    u32 raw_sta;        /* SLEEP_ISR_RAW_STA */
    u32 cpu_wake;       /* SLEEP_CPU_WAKEUP_EVENT */
    u32 timer_out;      /* PCM_TIMER_OUT */
    u32 event_reg;      /* PCM_EVENT_REG_STA */
    u32 isr;            /* SLEEP_ISR_STATUS */
    u32 r13;            /* PCM_REG13_DATA */
} wake_status_t;


//extern void mtk_wdt_suspend(void);
//extern void mtk_wdt_resume(void);
extern int mt_irq_mask_all(struct mtk_irq_mask *mask);
extern int mt_irq_mask_restore(struct mtk_irq_mask *mask);
//extern int mt_SPI_mask_all(struct mtk_irq_mask *mask);
//extern int mt_SPI_mask_restore(struct mtk_irq_mask *mask);
//extern int mt_PPI_mask_all(struct mtk_irq_mask *mask);
//extern int mt_PPI_mask_restore(struct mtk_irq_mask *mask);
//extern void mt_irq_mask_for_sleep(unsigned int irq);
extern void mt_irq_unmask_for_sleep(unsigned int irq);
extern void mt_cirq_enable(void);
extern void mt_cirq_disable(void);
extern void mt_cirq_clone_gic(void);
extern void mt_cirq_flush(void);
//extern void mt_cirq_mask(unsigned int cirq_num);
//extern void mt_cirq_mask_all(void);

extern void mcidle_before_wfi(int cpu);
extern void mcidle_after_wfi(int cpu);

void __attribute__((weak)) mcidle_before_wfi(int cpu)
{
}

void __attribute__((weak)) mcidle_after_wfi(int cpu)
{
}

extern spinlock_t spm_lock;

#if SPM_MCDI_DEBUG
static void spm_mcdi_dump_regs(void)
{
    /* SPM register */
    clc_notice("POWER_ON_VAL0   0x%x = 0x%x\n", SPM_POWER_ON_VAL0          , spm_read(SPM_POWER_ON_VAL0));
    clc_notice("POWER_ON_VAL1   0x%x = 0x%x\n", SPM_POWER_ON_VAL1          , spm_read(SPM_POWER_ON_VAL1));
    clc_notice("PCM_PWR_IO_EN   0x%x = 0x%x\n", SPM_PCM_PWR_IO_EN          , spm_read(SPM_PCM_PWR_IO_EN));
    clc_notice("CLK_CON         0x%x = 0x%x\n", SPM_CLK_CON                , spm_read(SPM_CLK_CON));
    clc_notice("AP_DVFS_CON     0x%x = 0x%x\n", SPM_AP_DVFS_CON_SET        , spm_read(SPM_AP_DVFS_CON_SET));
    clc_notice("PWR_STATUS      0x%x = 0x%x\n", SPM_PWR_STATUS             , spm_read(SPM_PWR_STATUS));
    clc_notice("PWR_STATUS_S    0x%x = 0x%x\n", SPM_PWR_STATUS_S           , spm_read(SPM_PWR_STATUS_S));
    clc_notice("SLEEP_TIMER_STA 0x%x = 0x%x\n", SPM_SLEEP_TIMER_STA        , spm_read(SPM_SLEEP_TIMER_STA));
    clc_notice("WAKE_EVENT_MASK 0x%x = 0x%x\n", SPM_SLEEP_WAKEUP_EVENT_MASK, spm_read(SPM_SLEEP_WAKEUP_EVENT_MASK));
    clc_notice("SPM_SLEEP_CPU_WAKEUP_EVENT 0x%x = 0x%x\n", SPM_SLEEP_CPU_WAKEUP_EVENT, spm_read(SPM_SLEEP_CPU_WAKEUP_EVENT));
    clc_notice("SPM_PCM_RESERVE   0x%x = 0x%x\n", SPM_PCM_RESERVE          , spm_read(SPM_PCM_RESERVE));  
    clc_notice("SPM_AP_STANBY_CON   0x%x = 0x%x\n", SPM_AP_STANBY_CON          , spm_read(SPM_AP_STANBY_CON));  
    clc_notice("SPM_PCM_TIMER_OUT   0x%x = 0x%x\n", SPM_PCM_TIMER_OUT          , spm_read(SPM_PCM_TIMER_OUT));
    clc_notice("SPM_PCM_CON1   0x%x = 0x%x\n", SPM_PCM_CON1          , spm_read(SPM_PCM_CON1));
    
    
    // PCM register
    clc_notice("PCM_REG0_DATA   0x%x = 0x%x\n", SPM_PCM_REG0_DATA          , spm_read(SPM_PCM_REG0_DATA));
//    clc_notice("PCM_REG1_DATA   0x%x = 0x%x\n", SPM_PCM_REG1_DATA          , spm_read(SPM_PCM_REG1_DATA));
//    clc_notice("PCM_REG2_DATA   0x%x = 0x%x\n", SPM_PCM_REG2_DATA          , spm_read(SPM_PCM_REG2_DATA));
//    clc_notice("PCM_REG3_DATA   0x%x = 0x%x\n", SPM_PCM_REG3_DATA          , spm_read(SPM_PCM_REG3_DATA));
//    clc_notice("PCM_REG4_DATA   0x%x = 0x%x\n", SPM_PCM_REG4_DATA          , spm_read(SPM_PCM_REG4_DATA));
//    clc_notice("PCM_REG5_DATA   0x%x = 0x%x\n", SPM_PCM_REG5_DATA          , spm_read(SPM_PCM_REG5_DATA));
//    clc_notice("PCM_REG6_DATA   0x%x = 0x%x\n", SPM_PCM_REG6_DATA          , spm_read(SPM_PCM_REG6_DATA));
    clc_notice("PCM_REG7_DATA   0x%x = 0x%x\n", SPM_PCM_REG7_DATA          , spm_read(SPM_PCM_REG7_DATA));
//    clc_notice("PCM_REG8_DATA   0x%x = 0x%x\n", SPM_PCM_REG8_DATA          , spm_read(SPM_PCM_REG8_DATA));
    clc_notice("PCM_REG9_DATA   0x%x = 0x%x\n", SPM_PCM_REG9_DATA          , spm_read(SPM_PCM_REG9_DATA));
//    clc_notice("PCM_REG10_DATA   0x%x = 0x%x\n", SPM_PCM_REG10_DATA          , spm_read(SPM_PCM_REG10_DATA));
    clc_notice("PCM_REG11_DATA   0x%x = 0x%x\n", SPM_PCM_REG11_DATA          , spm_read(SPM_PCM_REG11_DATA));
    clc_notice("PCM_REG12_DATA   0x%x = 0x%x\n", SPM_PCM_REG12_DATA          , spm_read(SPM_PCM_REG12_DATA));
    clc_notice("PCM_REG13_DATA   0x%x = 0x%x\n", SPM_PCM_REG13_DATA          , spm_read(SPM_PCM_REG13_DATA));
//    clc_notice("PCM_REG14_DATA   0x%x = 0x%x\n", SPM_PCM_REG14_DATA          , spm_read(SPM_PCM_REG14_DATA));
    clc_notice("PCM_REG15_DATA   0x%x = 0x%x\n", SPM_PCM_REG15_DATA          , spm_read(SPM_PCM_REG15_DATA));  
    
}
#endif


static void spm_direct_disable_sodi(void)
{
    u32 clc_temp;

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp |= (0x1<<13);
    
    spm_write(SPM_CLK_CON, clc_temp);  
}


static void spm_direct_enable_sodi(void)
{
    u32 clc_temp;

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp &= 0xffffdfff; // ~(0x1<<13);

    spm_write(SPM_CLK_CON, clc_temp);  
}


static void spm_reset_and_init_pcm(void)
{
    u32 con1;

    /* reset PCM */
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY);

    /* init PCM_CON0 (disable event vector) */
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);

    /* init PCM_CON1 (disable PCM timer but keep PCM WDT setting) */
    con1 = spm_read(SPM_PCM_CON1) & (CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
    spm_write(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_SPM_SRAM_ISO_B |
                            CON1_SPM_SRAM_SLP_B | CON1_IM_NONRP_EN | CON1_MIF_APBEN);
}


static void spm_kick_im_to_fetch(const pcm_desc_t *pcmdesc)
{
    u32 ptr, len, con0;

    /* tell IM where is PCM code (use slave mode if code existed) */
    ptr = spm_get_base_phys(pcmdesc->base);
    len = pcmdesc->size - 1;
    if (spm_read(SPM_PCM_IM_PTR) != ptr || spm_read(SPM_PCM_IM_LEN) != len) {
        spm_write(SPM_PCM_IM_PTR, ptr);
        spm_write(SPM_PCM_IM_LEN, len);
    } else {
        spm_write(SPM_PCM_CON1, spm_read(SPM_PCM_CON1) | CON1_CFG_KEY | CON1_IM_SLAVE);
    }

    /* kick IM to fetch (only toggle IM_KICK) */
    con0 = spm_read(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_IM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}


static void spm_init_pcm_register(void)
{
    /* init r0 with POWER_ON_VAL0 */
    spm_write(SPM_PCM_REG_DATA_INI, spm_read(SPM_POWER_ON_VAL0));
    spm_write(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
    spm_write(SPM_PCM_PWR_IO_EN, 0);


    /* init r7 with POWER_ON_VAL1 */
    spm_write(SPM_PCM_REG_DATA_INI, spm_read(SPM_POWER_ON_VAL1));
    spm_write(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
    spm_write(SPM_PCM_PWR_IO_EN, 0);

    /* clear REG_DATA_INI for PCM after init rX */
    spm_write(SPM_PCM_REG_DATA_INI, 0);
}


static void spm_init_event_vector(const pcm_desc_t *pcmdesc)
{
    /* init event vector register */
    spm_write(SPM_PCM_EVENT_VECTOR0, pcmdesc->vec0);
    spm_write(SPM_PCM_EVENT_VECTOR1, pcmdesc->vec1);
#if 0    
    spm_write(SPM_PCM_EVENT_VECTOR2, pcmdesc->vec2);
    spm_write(SPM_PCM_EVENT_VECTOR3, pcmdesc->vec3);
    spm_write(SPM_PCM_EVENT_VECTOR4, pcmdesc->vec4);
    spm_write(SPM_PCM_EVENT_VECTOR5, pcmdesc->vec5);
    spm_write(SPM_PCM_EVENT_VECTOR6, pcmdesc->vec6);
    spm_write(SPM_PCM_EVENT_VECTOR7, pcmdesc->vec7);
#endif
    /* event vector will be enabled by PCM itself */
}


static void spm_set_ap_standbywfi(bool IsMcdi)
{

    spm_write(SPM_AP_STANBY_CON, (1 << 19) |  /* unmask MD*/
                                 (1 << 20) |  /* unmask CONN */
                                 (1 << 16) |    /* unmask DISP */
                                 (0 << 17) |    /* mask MFG */
                                 (0 << 6) |     /* check SCU idle */
                                 (0 << 5) |     /* check L2C idle */
                                 (1U << 4));    /* Reduce AND */
    if (IsMcdi == TRUE)
    {
        spm_write(SPM_CORE0_WFI_SEL, 0x1);
        spm_write(SPM_CORE1_WFI_SEL, 0x1);
        spm_write(SPM_CORE2_WFI_SEL, 0x1);
        spm_write(SPM_CORE3_WFI_SEL, 0x1);
    }
    else
    {
    #if 0
        spm_write(SPM_CORE0_WFI_SEL, 0x0);
        //spm_write(SPM_CORE1_WFI_SEL, 0x0);
        //spm_write(SPM_CORE2_WFI_SEL, 0x0);
        //spm_write(SPM_CORE3_WFI_SEL, 0x0);    
    #endif
    }

}


/*
 * timer_val: PCM timer value (0 = disable)
 * wake_src : WAKE_SRC_XXX
 */
static void spm_set_wakeup_event(u32 wake_src)
{
    u32 isr;
    
    /* unmask wakeup source */
#if SPM_MCDI_BYPASS_SYSPWREQ
    wake_src &= ~WAKE_SRC_SYSPWREQ;     /* make 26M off when attach ICE */
#endif
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, ~wake_src);

    /* unmask SPM ISR (keep TWAM setting) */
    isr = spm_read(SPM_SLEEP_ISR_MASK) & ISR_TWAM;
    spm_write(SPM_SLEEP_ISR_MASK, isr | ISRM_PCM_IRQ_AUX);
    

    //mask CPU IRQ
    spm_write(SPM_SLEEP_CPU_IRQ_MASK,0xf);
}


static void spm_kick_pcm_to_run(bool cpu_pdn, bool infra_pdn)
{
    u32 clk, con0;

    /*enable SODI*/
    //spm_direct_enable_sodi();
    
    /* keep CPU or INFRA/DDRPHY power if needed and lock INFRA DCM */
    clk = spm_read(SPM_CLK_CON) & ~(CC_DISABLE_DORM_PWR | CC_DISABLE_INFRA_PWR);
    if (!cpu_pdn)
        clk |= CC_DISABLE_DORM_PWR;
    if (!infra_pdn)
        clk |= CC_DISABLE_INFRA_PWR;
    spm_write(SPM_CLK_CON, clk | CC_LOCK_INFRA_DCM);

    /* init pause request mask for PCM */
    spm_write(SPM_PCM_MAS_PAUSE_MASK, 0xffffffff);

    /* enable r0 and r7 to control power */
    spm_write(SPM_PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);

    /* SRCLKENA: r7 (PWR_IO_EN[7]=1) */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) | CC_SRCLKENA_MASK);

    /* kick PCM to run (only toggle PCM_KICK) */
    con0 = spm_read(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}


static void spm_trigger_wfi_for_mcdi(bool cpu_pdn)
{
    if (cpu_pdn) {
        if (!cpu_power_down(DORMANT_MODE)) {
            switch_to_amp();
            mcdi_wfi_with_sync();
        }
        switch_to_smp();
        cpu_check_dormant_abort();
    } else {
        mcdi_wfi_with_sync();
    }
}


static void spm_get_wakeup_status(wake_status_t *wakesta)
{
    /* get PC value if PCM assert (pause abort) */
    wakesta->debug_reg = spm_read(SPM_PCM_REG_DATA_INI);

    /* get wakeup event */
    wakesta->r12 = spm_read(SPM_PCM_REG9_DATA);     /* r9 = r12 for pcm_normal */
    wakesta->raw_sta = spm_read(SPM_SLEEP_ISR_RAW_STA);
    wakesta->cpu_wake = spm_read(SPM_SLEEP_CPU_WAKEUP_EVENT);

    /* get sleep time */
    wakesta->timer_out = spm_read(SPM_PCM_TIMER_OUT);

    /* get special pattern (0xf0000 or 0x10000) if sleep abort */
    wakesta->event_reg = spm_read(SPM_PCM_EVENT_REG_STA);

    /* get ISR status */
    wakesta->isr = spm_read(SPM_SLEEP_ISR_STATUS);

    /* get MD/CONN and co-clock status */
    wakesta->r13 = spm_read(SPM_PCM_REG13_DATA);
}


static void spm_clean_after_wakeup(void)
{
    /* PCM has cleared uart_clk_off_req and now clear it in POWER_ON_VAL1 */
    spm_write(SPM_POWER_ON_VAL1, spm_read(SPM_POWER_ON_VAL1) & ~R7_UART_CLK_OFF_REQ);

    /* SRCLKENA: POWER_ON_VAL1|r7 (PWR_IO_EN[7]=1) */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) & ~CC_SRCLKENA_MASK);

    /* re-enable POWER_ON_VAL0/1 to control power */
    spm_write(SPM_PCM_PWR_IO_EN, 0);

    /* unlock INFRA DCM */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) & ~CC_LOCK_INFRA_DCM);

    /* clean CPU wakeup event (pause abort) */
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0);

    /* clean wakeup event raw status (except THERM) */
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, ~WAKE_SRC_THERM);

    /* clean ISR status (except TWAM) */
    spm_write(SPM_SLEEP_ISR_MASK, spm_read(SPM_SLEEP_ISR_MASK) | ISRM_ALL_EXC_TWAM);
    spm_write(SPM_SLEEP_ISR_STATUS, ISRC_ALL_EXC_TWAM);
    spm_write(SPM_PCM_SW_INT_CLEAR, PCM_SW_INT0);
}


static wake_reason_t spm_output_wake_reason(const wake_status_t *wakesta)
{
    if (wakesta->debug_reg != 0) {
        spm_error2("PCM ASSERT AND PC = %u (0x%x)(0x%x)\n",
                   wakesta->debug_reg, wakesta->r13, wakesta->event_reg);
        return WR_PCM_ASSERT;
    }

    return WR_WAKE_SRC;
}


void spm_go_to_sodi(bool cpu_pdn)
{
    wake_status_t wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    wake_reason_t wr = WR_NONE;
    const pcm_desc_t *pcmdesc = &pcm_sodidle;
    
    spin_lock_irqsave(&spm_lock, flags);

    gSPM_SODI_EN =1;

    mt_irq_mask_all(&mask);
    mt_irq_unmask_for_sleep(MT_SPM_IRQ_ID);
    
    mt_cirq_clone_gic();
    mt_cirq_enable();

    spm_reset_and_init_pcm();

    spm_kick_im_to_fetch(pcmdesc);

    spm_init_pcm_register();

    spm_init_event_vector(pcmdesc);

    spm_set_ap_standbywfi(FALSE);

    spm_set_wakeup_event(WAKE_SRC_FOR_SODI);
    
    spm_kick_pcm_to_run(cpu_pdn, false);     /* keep INFRA/DDRPHY power */

#if SPM_MCDI_DEBUG
    //clc_notice("============SODI Before============\n");
    //spm_mcdi_dump_regs(); //dump debug info
#endif
    
    mcidle_before_wfi(0);

    spm_trigger_wfi_for_mcdi(cpu_pdn);

    mcidle_after_wfi(0);

#if SPM_MCDI_DEBUG
    //clc_notice("============SODI After=============\n");
    spm_mcdi_dump_regs();//dump debug info
#endif
    spm_get_wakeup_status(&wakesta);

    spm_clean_after_wakeup();

    wr = spm_output_wake_reason(&wakesta);

    mt_cirq_flush();
    mt_cirq_disable();
    mt_irq_mask_restore(&mask);

    gSPM_SODI_EN =0;
        
    spin_unlock_irqrestore(&spm_lock, flags);

    //return wr;
}

void spm_sodi_lcm_video_mode(bool IsLcmVideoMode)
{
    gSpm_IsLcmVideoMode = IsLcmVideoMode;

#if SPM_SODI_DEBUG    
    printk("spm_sodi_lcm_video_mode() : gSpm_IsLcmVideoMode = %x\n", gSpm_IsLcmVideoMode);    
#endif
    
}

void spm_disable_sodi(void)
{
    //spin_lock(&spm_sodi_lock);

    gSpm_Sodi_Disable_Counter++;
    
#if SPM_SODI_DEBUG    
    printk("spm_disable_sodi() : spm_sodi_disable_counter = 0x%x\n", gSpm_Sodi_Disable_Counter);    
#endif

    if(gSpm_Sodi_Disable_Counter > 0)
    {
        spm_direct_disable_sodi();
    }

    //spin_unlock(&spm_sodi_lock);
}

void spm_enable_sodi(void)
{
    //spin_lock(&spm_sodi_lock);

    gSpm_Sodi_Disable_Counter--;
    
#if SPM_SODI_DEBUG    
    printk("spm_enable_sodi() : spm_sodi_disable_counter = 0x%x\n", gSpm_Sodi_Disable_Counter);    
#endif    

    if(gSpm_Sodi_Disable_Counter <= 0)
    {
        spm_direct_enable_sodi();
    }

    //spin_unlock(&spm_sodi_lock);
}
#endif //SPM_SODI_ENABLED


MODULE_DESCRIPTION("MT6582 SPM-Idle Driver v1.2");