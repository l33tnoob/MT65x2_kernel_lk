#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/aee.h>

#include <mach/irqs.h>
#include <mach/mt_cirq.h>
#include <mach/mt_spm.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>
#include <mach/mt_dormant.h>
#include <mach/mt_mci.h>
//#include <mach/eint.h>
#include <mach/mtk_ccci_helper.h>
#include <board-custom.h>

//#define SPM_DPIDLE_CLK_DBG_OUT

extern void spm_dpidle_before_wfi(void);        /* can be redefined */
extern void spm_dpidle_after_wfi(void);         /* can be redefined */
extern int mtk_is_wdt_enable(void);
extern SPM_PCM_CONFIG pcm_config_wdt;
#ifdef SPM_DPIDLE_CLK_DBG_OUT
extern void gpiodbg_emi_dbg_out(void);
extern void gpiodbg_armcore_dbg_out(void);
#endif

#define SPM_DPIDLE_FIRMWARE_VER "v16-24 @ 2013-04-26"
 
static const u32 spm_pcm_dpidle[] = {
    0x80c01801, 0xd82000a3, 0x17c07c1f, 0x1800001f, 0x17cf0f1e, 0xd8000123,
    0x17c07c1f, 0x1800001f, 0x17cf0f16, 0xc0c01660, 0x10c0041f, 0x1b00001f,
    0x7ffff7ff, 0xf0000000, 0x17c07c1f, 0xc0c01660, 0x10c07c1f, 0x1800001f,
    0x17cf0f36, 0x80c01801, 0xd8200303, 0x17c07c1f, 0x1800001f, 0x17cf0f3e,
    0x1b00001f, 0x3fffefff, 0xf0000000, 0x17c07c1f, 0x19c0001f, 0x001c4ba7,
    0x1b80001f, 0x20000030, 0xe8208000, 0x10006354, 0xffffff97, 0x18c0001f,
    0x10006240, 0x1200041f, 0xc0c00e80, 0x17c07c1f, 0x1800001f, 0x07cb0f16,
    0x1b80001f, 0x20000300, 0x1800001f, 0x06cb0f16, 0x1b80001f, 0x20000300,
    0x1800001f, 0x06cb0b16, 0x1800001f, 0x06cb0b12, 0x19c0001f, 0x00144ba7,
    0x19c0001f, 0x00144ba5, 0xc0c01cc0, 0x17c07c1f, 0xe8208000, 0x10006354,
    0xfffff806, 0x19c0001f, 0x00104ba5, 0x19c0001f, 0x00114aa5, 0x1b00001f,
    0xbfffe7ff, 0xf0000000, 0x17c07c1f, 0x1b80001f, 0x20000fdf, 0x8880000d,
    0x00000024, 0x1b00001f, 0xbfffe7ff, 0xd8000e42, 0x17c07c1f, 0xe8208000,
    0x10006354, 0xffffff97, 0x19c0001f, 0x00184be5, 0x1b80001f, 0x20000008,
    0x19c0001f, 0x001c4be5, 0x1b80001f, 0x2000000a, 0x1880001f, 0x10006320,
    0xc0c01540, 0xe080000f, 0xd8000e43, 0x17c07c1f, 0xe080001f, 0xc0c01ac0,
    0x17c07c1f, 0xc0c01820, 0x17c07c1f, 0x18c0001f, 0x10006240, 0x1200041f,
    0xc0c01040, 0x17c07c1f, 0x1800001f, 0x06cb0b16, 0x1800001f, 0x06cb0f16,
    0x1800001f, 0x07cb0f16, 0x1800001f, 0x17cf0f16, 0x1b00001f, 0x7ffff7ff,
    0xf0000000, 0x17c07c1f, 0xe0e00f16, 0x1380201f, 0xe0e00f1e, 0x1380201f,
    0xe0e00f0e, 0x1380201f, 0xe0e00f0c, 0xe0e00f0d, 0xe0e00e0d, 0xe0e00c0d,
    0xe0e0080d, 0xe0e0000d, 0xf0000000, 0x17c07c1f, 0xe0e00f0d, 0xe0e00f1e,
    0xe0e00f12, 0xf0000000, 0x17c07c1f, 0xd80011ea, 0x17c07c1f, 0xe2e00036,
    0x1380201f, 0xe2e0003e, 0x1380201f, 0xe2e0002e, 0x1380201f, 0xd82012ea,
    0x17c07c1f, 0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f, 0x20000020,
    0xe2e0004d, 0xf0000000, 0x17c07c1f, 0xd80013aa, 0x17c07c1f, 0xe2e0006d,
    0xe2e0002d, 0xd820144a, 0x17c07c1f, 0xe2e0002f, 0xe2e0003e, 0xe2e00032,
    0xf0000000, 0x17c07c1f, 0xa1d10407, 0x1b80001f, 0x20000020, 0x10c07c1f,
    0xf0000000, 0x17c07c1f, 0xa1d08407, 0x1b80001f, 0x20000080, 0x80eab401,
    0x1a00001f, 0x10006814, 0xe2000003, 0xf0000000, 0x17c07c1f, 0x1a00001f,
    0x10006604, 0xd8001783, 0x17c07c1f, 0xe2200002, 0x1b80001f, 0x20000020,
    0xd82017e3, 0x17c07c1f, 0xe2200001, 0x1b80001f, 0x20000020, 0xf0000000,
    0x17c07c1f, 0x1a10001f, 0x11004058, 0x1a80001f, 0x11004058, 0xaa000008,
    0x80000000, 0xe2800008, 0x1a10001f, 0x1100406c, 0x1a80001f, 0x1100406c,
    0xaa000008, 0x80000000, 0xe2800008, 0xf0000000, 0x17c07c1f, 0xa1d40407,
    0x1391841f, 0xa1d90407, 0xf0000000, 0x17c07c1f, 0x1900001f, 0x10004050,
    0x18d0001f, 0x10004050, 0xa8c00003, 0x00000010, 0xe1000003, 0x1900001f,
    0x10004078, 0x18d0001f, 0x10004078, 0x88c00003, 0xfffffffe, 0xe1000003,
    0xf0000000, 0x17c07c1f, 0x1b80001f, 0x20000300, 0x1900001f, 0x10004050,
    0x18d0001f, 0x10004050, 0x88c00003, 0xffffffef, 0xe1000003, 0x1900001f,
    0x10004078, 0x18d0001f, 0x10004078, 0xa8c00003, 0x00000001, 0xe1000003,
    0xf0000000, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0xa1d48407, 0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0xd00f0000, 0x8880000c,
    0x3fffe7ff, 0xd8003b02, 0x1140041f, 0xe8208000, 0x10006354, 0xfffff806,
    0xc0c01480, 0x17c07c1f, 0x1950001f, 0x10006400, 0x80d70405, 0xd80026a3,
    0x17c07c1f, 0x89c00007, 0xffffefff, 0x18c0001f, 0x10006200, 0xc0c01320,
    0x12807c1f, 0xe8208000, 0x1000625c, 0x00000001, 0x1890001f, 0x1000625c,
    0x81040801, 0xd82023c4, 0x17c07c1f, 0xc0c01320, 0x1280041f, 0x18c0001f,
    0x10006208, 0xc0c01320, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000000,
    0x1890001f, 0x10006248, 0x81040801, 0xd8002584, 0x17c07c1f, 0x1b80001f,
    0x20000020, 0xc0c01320, 0x1280041f, 0x1b00001f, 0xffffffff, 0x1b80001f,
    0xd0010000, 0x8880000c, 0x3fffe7ff, 0xd8003622, 0x17c07c1f, 0x8880000c,
    0x40000000, 0xd80026a2, 0x17c07c1f, 0x80823401, 0xd80026a2, 0x17c07c1f,
    0x1880001f, 0x10006320, 0x1990001f, 0x10006600, 0xa1d40407, 0x1b80001f,
    0x20000008, 0xa1d90407, 0xe8208000, 0x10006354, 0xfffff817, 0xc0c01480,
    0xe080000f, 0xd80026a3, 0x17c07c1f, 0xc0c01540, 0xe080000f, 0xd80026a3,
    0x17c07c1f, 0xc0c01ac0, 0x17c07c1f, 0xe8208000, 0x10006310, 0x0b1600f8,
    0xe080001f, 0x19c0001f, 0x001c4be7, 0x1b80001f, 0x20000030, 0xc0c01820,
    0x17c07c1f, 0x18c0001f, 0x10006240, 0xc0c01040, 0x17c07c1f, 0x1800001f,
    0x00000016, 0x1800001f, 0x00000f16, 0x1800001f, 0x07c00f16, 0x1800001f,
    0x17cf0f16, 0x8080b401, 0xd8002e82, 0x17c07c1f, 0xc0c01660, 0x10c07c1f,
    0x80c01801, 0xd8202f23, 0x17c07c1f, 0x1800001f, 0x17cf0f1e, 0x1b00001f,
    0x3fffefff, 0x1b80001f, 0x90100000, 0x80881c01, 0xd8003442, 0x17c07c1f,
    0x1800001f, 0x07cf0f16, 0x18c0001f, 0x10006240, 0x1200041f, 0xc0c00e80,
    0x17c07c1f, 0xc0c01660, 0x10c0041f, 0x1800001f, 0x07c00f16, 0x1b80001f,
    0x20000300, 0x1800001f, 0x04000f16, 0x1b80001f, 0x20000300, 0x1800001f,
    0x00000f16, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000016, 0x10007c1f,
    0x1b80001f, 0x2000049c, 0x19c0001f, 0x00004b25, 0xc0c01cc0, 0x17c07c1f,
    0x19c0001f, 0x00014a25, 0xd8203622, 0x17c07c1f, 0x1800001f, 0x03800e12,
    0x1b80001f, 0x20000300, 0x1800001f, 0x00000e12, 0x1b80001f, 0x20000300,
    0x1800001f, 0x00000012, 0x10007c1f, 0x1b80001f, 0x2000079e, 0x19c0001f,
    0x00014a25, 0xe8208000, 0x10006354, 0xfffff806, 0x19c0001f, 0x00014a25,
    0x80d70405, 0xd8003b03, 0x17c07c1f, 0x18c0001f, 0x10006208, 0x1212841f,
    0xc0c010e0, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000001, 0x1890001f,
    0x10006248, 0x81040801, 0xd8203824, 0x17c07c1f, 0x1b80001f, 0x20000020,
    0xc0c010e0, 0x1280041f, 0x18c0001f, 0x10006200, 0xc0c010e0, 0x12807c1f,
    0xe8208000, 0x1000625c, 0x00000000, 0x1890001f, 0x1000625c, 0x81040801,
    0xd8003a24, 0x17c07c1f, 0xc0c010e0, 0x1280041f, 0x19c0001f, 0x00015820,
    0x10007c1f, 0x80cab001, 0x808cb401, 0x80800c02, 0xd8203c22, 0x17c07c1f,
    0xa1d78407, 0x8880000c, 0x00002000, 0xd8203d42, 0x17c07c1f, 0xe8208000,
    0x100063e0, 0x00000002, 0x1b80001f, 0x00001000, 0x1ac0001f, 0x55aa55aa,
    0xf0000000

};
#define PCM_DPIDLE_BASE         __pa(spm_pcm_dpidle)
#define PCM_DPIDLE_LEN          (493)
#define PCM_DPIDLE_VEC0         EVENT_VEC(WAKE_ID_26M_WAKE, 1, 0, 0)      /* MD-wake event */
#define PCM_DPIDLE_VEC1         EVENT_VEC(WAKE_ID_26M_SLP, 1, 0, 15)     /* MD-sleep event */
#define PCM_DPIDLE_VEC2         EVENT_VEC(WAKE_ID_AP_WAKE, 1, 0, 28)      /* MD-wake event */
#define PCM_DPIDLE_VEC3         EVENT_VEC(WAKE_ID_AP_SLEEP, 1, 0, 69)     /* MD-sleep event */


#if defined (CONFIG_MT6572_FPGA_CA7)
#define WAKE_SRC_FOR_DPIDLE  (WAKE_SRC_KP | WAKE_SRC_EINT | WAKE_SRC_CCIF | WAKE_SRC_USB_CD | WAKE_SRC_MD_WDT)
#else
#define WAKE_SRC_FOR_DPIDLE                                                 \
    (WAKE_SRC_KP | WAKE_SRC_GPT | WAKE_SRC_EINT | WAKE_SRC_CCIF |      \
     WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN | WAKE_SRC_AFE |                 \
     WAKE_SRC_SYSPWREQ | WAKE_SRC_MD_WDT | WAKE_SRC_CONN_WDT | WAKE_SRC_CONN | WAKE_SRC_THERM)
#endif

SPM_PCM_CONFIG pcm_config_dpidle={
    .scenario = SPM_PCM_DEEP_IDLE,
    .ver = SPM_DPIDLE_FIRMWARE_VER,
	.spm_turn_off_26m = false,
	.pcm_firmware_addr =  PCM_DPIDLE_BASE,
	.pcm_firmware_len = PCM_DPIDLE_LEN,
	.pcm_pwrlevel = PWR_LV0,
	.spm_request_uart_sleep = true,
    .pcm_vsr = {PCM_DPIDLE_VEC0,PCM_DPIDLE_VEC1,PCM_DPIDLE_VEC2,PCM_DPIDLE_VEC3,0,0,0,0},
    
    /*Wake up event mask*/
    .md_mask = MDCONN_UNMASK,   /* unmask MD1 and MD2 */
#if defined (CONFIG_MT6572_FPGA_CA7)    
    .mm_mask = MMALL_MASK,   /* unmask DISP and MFG */
#else
    .mm_mask = MMALL_UNMASK,   /* unmask DISP and MFG */
#endif
    
    .sync_r0r7=true,
    
    /*AP Sleep event mask*/
    .wfi_scu_mask=false,  /* check SCU idle */
	.wfi_l2c_mask=false,  /* check L2C idle */
	.wfi_op=REDUCE_AND,
#if defined (CONFIG_MT6572_FPGA_CA7)   	
	.wfi_sel = {true,false},
#else
	.wfi_sel = {true,true},
#endif
#if defined (CONFIG_MT6572_FPGA_CA7)
	.timer_val_ms = 10*1000,
#else
    .timer_val_ms = 0,
#endif
    .wake_src = WAKE_SRC_FOR_DPIDLE,
    .infra_pdn = false,  /* keep INFRA/DDRPHY power */
    .reserved = 0x0,
    .coclock_en = false,
    .lock_infra_dcm=true,
    
    /* debug related*/
    .dbg_wfi_cnt=0,
    .wakesta_idx=0
	 };

u32 gpt_0xf0008000 = 0;
u32 gpt_0xf0008004 = 0;
u32 gpt_0xf0008008 = 0;
u32 gpt_0xf0008040 = 0;
u32 gpt_0xf0008044 = 0;
u32 gpt_0xf0008048 = 0;
u32 gpt_0xf000804c = 0;
u32 gpt_0xf0008010 = 0;
u32 gpt_0xf0008014 = 0;
u32 gpt_0xf0008018 = 0;
u32 gpt_0xf000801c = 0;
u32 spm_deepidle_wdt_val = 0;



wake_reason_t spm_go_to_dpidle(bool cpu_pdn, u8 pwrlevel)
{
    wake_status_t *wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    wake_reason_t wr = WR_NONE;

#if defined(CONFIG_KICK_SPM_WDT)
    mtk_wdt_suspend();
#endif

    spin_lock_irqsave(&spm_lock, flags);
    mt_irq_mask_all(&mask);
    mt_irq_unmask_for_sleep(MT_SPM0_IRQ_ID);
    
    //spm_wdt_config(false);
    
    mt_cirq_clone_gic();
    mt_cirq_enable();

    if(pwrlevel>2) 
    {
        spm_crit2("Hey!! wrong PWR Level: %x",pwrlevel);//ASSERT Wrong Para!!
        goto RESTORE_IRQ;
    }
    pcm_config_dpidle.pcm_pwrlevel = 1 << pwrlevel ;
    pcm_config_dpidle.spm_request_uart_sleep = (pwrlevel == 0 ? true : false);
    pcm_config_dpidle.cpu_pdn = cpu_pdn;
    
    if(mtk_is_wdt_enable())
        pcm_config_dpidle.wdt_val_ms = pcm_config_wdt.wdt_val_ms;
    else
        pcm_config_dpidle.wdt_val_ms = 0;
    //pcm_config_dpidle.infra_pdn = false;  /* keep INFRA/DDRPHY power */
        
    if (spm_init_pcm(&pcm_config_dpidle)==false)
      goto  RESTORE_IRQ;

#ifdef SPM_CLOCK_INIT
    enable_clock(MT_CG_MEMSLP_DLYER_SW_CG, "SPM_DPIDLE");
    enable_clock(MT_CG_SPM_SW_CG, "SPM_DPIDLE");
#endif

#ifdef SPM_DPIDLE_CLK_DBG_OUT
        //gpiodbg_monitor();
        gpiodbg_emi_dbg_out();
        gpiodbg_armcore_dbg_out();
#endif

        spm_kick_pcm(&pcm_config_dpidle);
        
        spm_dpidle_before_wfi();
                
        snapshot_golden_setting(__FUNCTION__, __LINE__);

        spm_deepidle_wdt_val = spm_read(SPM_PCM_WDT_TIMER_VAL);


        //spm_trigger_wfi_for_dpidle(cpu_pdn);
        spm_trigger_wfi(&pcm_config_dpidle);

        gpt_0xf0008000 = spm_read(0xf0008000);
        gpt_0xf0008004 = spm_read(0xf0008004);
        gpt_0xf0008008 = spm_read(0xf0008008);        
        gpt_0xf0008040 = spm_read(0xf0008040);
        gpt_0xf0008044 = spm_read(0xf0008044);
        gpt_0xf0008048 = spm_read(0xf0008048);  
        gpt_0xf000804c = spm_read(0xf000804c);
        gpt_0xf0008010 = spm_read(0xf0008010);
        gpt_0xf0008014 = spm_read(0xf0008014);  
        gpt_0xf0008018 = spm_read(0xf0008018);
        gpt_0xf000801c = spm_read(0xf000801c);
 

        spm_dpidle_after_wfi();

        wakesta = spm_get_wakeup_status(&pcm_config_dpidle);

        wr = wakesta->wake_reason;

        spm_clean_after_wakeup();

#ifdef SPM_CLOCK_INIT
    disable_clock(MT_CG_SPM_SW_CG, "SPM_DPIDLE");
    disable_clock(MT_CG_MEMSLP_DLYER_SW_CG, "SPM_DPIDLE");
#endif

RESTORE_IRQ:
    mt_cirq_flush();
    mt_cirq_disable();
    mt_irq_mask_restore(&mask);
    spin_unlock_irqrestore(&spm_lock, flags);
#if defined(CONFIG_KICK_SPM_WDT)    
    mtk_wdt_resume();
#endif
    return wr;
}
