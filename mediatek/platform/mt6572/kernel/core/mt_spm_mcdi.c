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

#include <mach/irqs.h>
#include <mach/mt_spm.h>
#include <mach/mt_dormant.h>
#include <mach/mt_gpt.h>
#include <mach/mt_spm_pcm.h>
#include <mach/mt_irq.h>
#include <mach/mt_spm_api.h>
#include <mach/mt_clkmgr.h>


//#include <mach/env.h> // require from hibboot flag
#include <asm/hardware/gic.h>


//===================================
#if defined (CONFIG_MT6572_FPGA_CA7)
#define SPM_MCDI_BYPASS_SYSPWREQ 0//for FPGA attach jtag
#else
#define SPM_MCDI_BYPASS_SYSPWREQ 1
#endif
#ifdef SPM_MCDI_FUNC
#define MCDI_KICK_PCM 1
#else
#define MCDI_KICK_PCM 0
#endif

#define SPM_MCDI_LDVT_EN

#define clc_debug spm_debug
#define clc_notice spm_notice
// ===================================


DEFINE_SPINLOCK(spm_sodi_lock);

s32 spm_sodi_disable_counter = 0;
u32 MCDI_Test_Mode = 0;


#define WFI_OP        4
#define WFI_L2C      5
#define WFI_SCU      6
#define WFI_MM      16
#define WFI_MD      19


// ==========================================
// PCM code for MCDI (Multi Core Deep Idle)  v503 2013/01/31
//
// core 0 : local timer
// core 1 : GPT 4
// ==========================================
static u32 spm_pcm_mcdi[] = {
    0x808ab401, 0xd8200102, 0x17c07c1f, 0x10007c1f, 0x19c0001f, 0x00204800,
    0x19c0001f, 0x00204800, 0xa1d80407, 0x1b00001f, 0xbffff7ff, 0xf0000000,
    0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0x20000fdf, 0x810d3401,
    0x71200404, 0xb1043481, 0xb100b481, 0xb1023481, 0x8880000c, 0x3fffe7ff,
    0xa0801002, 0x1b00001f, 0x3fffe7ff, 0xd8000662, 0x17c07c1f, 0x1b00001f,
    0xbffff7ff, 0xd8000662, 0x17c07c1f, 0x19c0001f, 0x00204822, 0x1b00001f,
    0x3fffe7ff, 0x1b80001f, 0x2000000a, 0x809a840d, 0xd80005e2, 0x17c07c1f,
    0x18c0001f, 0x10006814, 0xe0c00001, 0xd8200662, 0x17c07c1f, 0xa8000000,
    0x00000004, 0x1b00001f, 0x00000000, 0xf0000000, 0x17c07c1f, 0xd800080a,
    0x17c07c1f, 0xe2e00036, 0x1b80001f, 0x20000020, 0xe2e0003e, 0x1b80001f,
    0x20000020, 0xe2e0002e, 0x1b80001f, 0x20000020, 0xd820090a, 0x17c07c1f,
    0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f, 0x20000020, 0xe2e0004d,
    0xf0000000, 0x17c07c1f, 0xd80009ca, 0x17c07c1f, 0xe2e0006d, 0xe2e0002d,
    0xd8200a6a, 0x17c07c1f, 0xe2e0002f, 0xe2e0003e, 0xe2e00032, 0xf0000000,
    0x17c07c1f, 0x1a10001f, 0x10006608, 0xa24e3401, 0x82c02408, 0xf0000000,
    0x17c07c1f, 0x1a50001f, 0x1000624c, 0x80c02401, 0x1180a41f, 0x1a50001f,
    0x10006250, 0x82002401, 0xa0d0a003, 0xa180a406, 0x1a50001f, 0x10006254,
    0x82002401, 0xa0d12003, 0xa180a406, 0x1a50001f, 0x10006258, 0x82002401,
    0xa0d1a003, 0xa180a406, 0x124e341f, 0xb0c024a3, 0x1a10001f, 0x10006400,
    0x8206a001, 0x6a600003, 0x0000000f, 0x82e02009, 0x82c0040b, 0xf0000000,
    0x17c07c1f, 0x820cb401, 0xd8200fa8, 0x17c07c1f, 0xa1d78407, 0xf0000000,
    0x17c07c1f, 0x1a10001f, 0x10008004, 0x8241a001, 0x1090a41f, 0x82402001,
    0xa0802402, 0x82422001, 0xa0802402, 0x8240a001, 0xa0802402, 0x82412001,
    0xa0802402, 0x8242a001, 0xa0802402, 0x1a40001f, 0x10006600, 0xe2400002,
    0xf0000000, 0x17c07c1f, 0x19c0001f, 0x00204822, 0x18c0001f, 0x10006320,
    0xe0c0001f, 0x1b80001f, 0x20000014, 0x82da840d, 0xd800140b, 0x17c07c1f,
    0xe0c0000f, 0x18c0001f, 0x10006814, 0xe0c00001, 0xf0000000, 0x17c07c1f,
    0x10007c1f, 0x19c0001f, 0x00204800, 0x19c0001f, 0x00204800, 0xf0000000,
    0x17c07c1f, 0x1890001f, 0x10006f0c, 0x1090881f, 0xa8900002, 0x10006f08,
    0x1090881f, 0xa8900002, 0x10006f04, 0x1090881f, 0xa8900002, 0x10006f00,
    0xa8900002, 0x10006b10, 0x1a00001f, 0x10006b10, 0xe2000002, 0xf0000000,
    0x17c07c1f, 0x1a10001f, 0x10006b10, 0x82002c08, 0x1880001f, 0x10006b10,
    0xe0800008, 0xf0000000, 0x17c07c1f, 0x1880001f, 0x1000625c, 0xe080000a,
    0x1890001f, 0x1000625c, 0x9124080a, 0xd80018c4, 0x17c07c1f, 0xf0000000,
    0x17c07c1f, 0x1880001f, 0x10006264, 0xe080000a, 0x1890001f, 0x10006264,
    0x9124080a, 0xd8001a04, 0x17c07c1f, 0xf0000000, 0x17c07c1f, 0x1880001f,
    0x1000626c, 0xe080000a, 0x1890001f, 0x1000626c, 0x9124080a, 0xd8001b44,
    0x17c07c1f, 0xf0000000, 0x17c07c1f, 0x1880001f, 0x10006274, 0xe080000a,
    0x1890001f, 0x10006274, 0x9124080a, 0xd8001c84, 0x17c07c1f, 0xf0000000,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0x11407c1f, 0x1b00001f, 0x3d200011, 0x1b80001f, 0xf0000010, 0xc0c00fe0,
    0x17c07c1f, 0xc0801520, 0x10807c1f, 0x1890001f, 0x100063c0, 0x108f081f,
    0xd8202062, 0x17c07c1f, 0x808ab001, 0xc8800f22, 0x17c07c1f, 0xc1000aa0,
    0x11007c1f, 0x80e01404, 0x60a07c05, 0x88900002, 0x10006814, 0xd80036a2,
    0x17c07c1f, 0x81000403, 0xd82023e4, 0x17c07c1f, 0xa1400405, 0x81008c01,
    0xd82025a4, 0x17c07c1f, 0x1900001f, 0x10006218, 0xc1000940, 0x12807c1f,
    0xc28019a0, 0x1280041f, 0x1900001f, 0x10006218, 0xc1000940, 0x1280041f,
    0xa1508405, 0x81010c01, 0xd8202624, 0x17c07c1f, 0xa1510405, 0x81018c01,
    0xd82026a4, 0x17c07c1f, 0xa1518405, 0xd820308c, 0x17c07c1f, 0xc1000aa0,
    0x11007c1f, 0x80c01404, 0x1890001f, 0x10006600, 0xa0800806, 0x82000c01,
    0xd82029a8, 0x17c07c1f, 0x824d3001, 0xb2403121, 0xb24c3121, 0xb2400921,
    0xd82029a9, 0x17c07c1f, 0x89400005, 0xfffffffe, 0xe8208000, 0x10006f00,
    0x00000000, 0xc1401760, 0x17c07c1f, 0x81008c01, 0xd8202cc4, 0x17c07c1f,
    0x810db001, 0xb1008881, 0xb1003081, 0xd8202cc4, 0x17c07c1f, 0x89400005,
    0xfffffffd, 0xe8208000, 0x10006f04, 0x00000000, 0xc1401760, 0x17c07c1f,
    0x1900001f, 0x10006218, 0xc10006a0, 0x12807c1f, 0xc28019a0, 0x12807c1f,
    0x1900001f, 0x10006218, 0xc10006a0, 0x1280041f, 0x81010c01, 0xd8202ea4,
    0x17c07c1f, 0x810e3001, 0xb1010881, 0xb1003081, 0xd8202ea4, 0x17c07c1f,
    0x89400005, 0xfffffffb, 0xe8208000, 0x10006f08, 0x00000000, 0xc1401760,
    0x17c07c1f, 0x81018c01, 0xd8203084, 0x17c07c1f, 0x810eb001, 0xb1018881,
    0xb1003081, 0xd8203084, 0x17c07c1f, 0x89400005, 0xfffffff7, 0xe8208000,
    0x10006f0c, 0x00000000, 0xc1401760, 0x17c07c1f, 0xc0800b60, 0x10807c1f,
    0xd8202062, 0x17c07c1f, 0x1b00001f, 0x7fffefff, 0x1b80001f, 0xd0010000,
    0x8098840d, 0x810d3401, 0x71200404, 0xb0c43481, 0xb0c0b461, 0xb0c23461,
    0xa0800c02, 0xd8203362, 0x17c07c1f, 0x8880000c, 0x3d200011, 0xd8002062,
    0x17c07c1f, 0xd0003100, 0x17c07c1f, 0xe8208000, 0x10006310, 0x0b1600f8,
    0xc0801240, 0x10807c1f, 0xd8202062, 0x17c07c1f, 0x1b00001f, 0xffffffff,
    0x1b80001f, 0x90100000, 0x1b00001f, 0x3d200011, 0xe8208000, 0x10006310,
    0x0b160008, 0x80810001, 0x810ab401, 0xa0801002, 0xd8202062, 0x17c07c1f,
    0xc1001440, 0x17c07c1f, 0xa1d80407, 0xd0002060, 0x17c07c1f, 0x19c0001f,
    0x00215800, 0x10007c1f, 0xf0000000



};

#define PCM_MCDI_BASE            __pa(spm_pcm_mcdi)
#define PCM_MCDI_LEN              ( 441 )
#define MCDI_pcm_pc_0      0
#define MCDI_pcm_pc_1      13
#define MCDI_pcm_pc_2      MCDI_pcm_pc_0
#define MCDI_pcm_pc_3      MCDI_pcm_pc_1

#define PCM_MCDI_VEC0        EVENT_VEC(WAKE_ID_26M_WAKE, 1, 0, MCDI_pcm_pc_0)      /* MD-wake event */
#define PCM_MCDI_VEC1        EVENT_VEC(WAKE_ID_26M_SLP, 1, 0, MCDI_pcm_pc_1)      /* MD-sleep event */
#define PCM_MCDI_VEC2        EVENT_VEC(WAKE_ID_AP_WAKE, 1, 0, MCDI_pcm_pc_2)      
#define PCM_MCDI_VEC3        EVENT_VEC(WAKE_ID_AP_SLEEP, 1, 0, MCDI_pcm_pc_3)      


//extern int mt_irq_mask_all(struct mtk_irq_mask *mask);
//extern int mt_irq_mask_restore(struct mtk_irq_mask *mask);
//extern int mt_SPI_mask_all(struct mtk_irq_mask *mask);
//extern int mt_SPI_mask_restore(struct mtk_irq_mask *mask);
extern void mt_irq_mask_for_sleep(unsigned int irq);
extern void mt_irq_unmask_for_sleep(unsigned int irq);
#if defined( SPM_MCDI_LDVT_EN )
extern void spm_mcdi_LDVT_mcdi(void);
extern void spm_mcdi_LDVT_sodi(void);
#endif
extern char *get_env(char *name);

/*
extern void //mt_cirq_enable(void);
extern void //mt_cirq_disable(void);
extern void //mt_cirq_clone_gic(void);
extern void //mt_cirq_flush(void);
extern void //mt_cirq_mask(unsigned int cirq_num);
*/
extern spinlock_t spm_lock;
extern u32 En_SPM_MCDI;

//static struct mtk_irq_mask MCDI_cpu_irq_mask;

//TODO: need check
#if SPM_MCDI_BYPASS_SYSPWREQ    
    #define WAKE_SRC_FOR_MCDI                     \
        (WAKE_SRC_PCM_TIMER | WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | WAKE_SRC_CPU0_IRQ | WAKE_SRC_CPU1_IRQ | WAKE_SRC_SYSPWREQ )
#else    
    #define WAKE_SRC_FOR_MCDI                     \
        (WAKE_SRC_PCM_TIMER | WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | WAKE_SRC_CPU0_IRQ | WAKE_SRC_CPU1_IRQ )
#endif


SPM_PCM_CONFIG pcm_config_mcdi={
    .scenario = SPM_PCM_MCDI,
    .spm_turn_off_26m = false,
    .pcm_firmware_addr =  PCM_MCDI_BASE,
    .pcm_firmware_len = PCM_MCDI_LEN,
    .pcm_pwrlevel = PWR_LVNA,
    .spm_request_uart_sleep = false,
    .sodi_en = false,
    .pcm_vsr = {PCM_MCDI_VEC0,PCM_MCDI_VEC1,PCM_MCDI_VEC2,PCM_MCDI_VEC3,0,0,0,0},

    //spm_write(SPM_AP_STANBY_CON, ((0x0<<WFI_OP) | (0x1<<WFI_L2C) | (0x1<<WFI_SCU)));  // operand or, mask l2c, mask scu 

    /*Wake up event mask*/
    .md_mask = MDCONN_MASK,   /* mask MD1 and MD2 */
    .mm_mask = MMALL_MASK,   /* mask DISP and MFG */

    /*AP Sleep event mask*/
    .wfi_scu_mask=true,  /* check SCU idle */
    .wfi_l2c_mask=true,  /* check L2C idle */
    .wfi_op=REDUCE_OR,
    .wfi_sel = {true,true},

    .timer_val_ms = 0*1000,
    .wake_src = WAKE_SRC_FOR_MCDI,
    .infra_pdn = false,  /* keep INFRA/DDRPHY power */
    .cpu_pdn = false,
    /* debug related*/
    .reserved = 0x0,
    .dbg_wfi_cnt=0,
    .wakesta_idx=0
	 };
#if 0
static void spm_go_to_MCDI(void)
{
    unsigned long flags;
    spin_lock_irqsave(&spm_lock, flags);

    En_SPM_MCDI = 2;
    
    // mask SPM IRQ =======================================
    mt_irq_mask_for_sleep(MT_SPM0_IRQ_ID); // mask spm    
    mt_irq_mask_for_sleep(MT_SPM1_IRQ_ID); // mask spm   

    // init PCM ============================================
    spm_init_pcm(&pcm_config_mcdi); 

#if 1//def SPM_CLOCK_INIT
    //if(!clock_is_on(MT_CG_SPM_SW_CG))
        enable_clock(MT_CG_SPM_SW_CG, "SPM_MCDI");
#endif


    // print spm debug log =====================================
    //spm_pcm_dump_regs();
    
    // Kick PCM and IM =======================================
    BUG_ON(PCM_MCDI_BASE & 0x00000003);     // check 4-byte alignment 
    //KICK_IM_PCM(PCM_MCDI_BASE, PCM_MCDI_LEN); 
    spm_kick_pcm(&pcm_config_mcdi);
    
//    //clc_notice("Kick PCM and IM OK.\r\n");

    En_SPM_MCDI = 1;

    spin_unlock_irqrestore(&spm_lock, flags);
}

static u32 spm_leave_MCDI(void)
{ 
    unsigned long flags;

    /* Mask ARM i bit */
    //asm volatile("cpsid i @ arch_local_irq_disable" : : : "memory", "cc"); // set i bit to disable interrupt    
    local_irq_save(flags);

    En_SPM_MCDI = 2;
  
    spm_mcdi_clean();

#if 1//def SPM_CLOCK_INIT
    //if(clock_is_on(MT_CG_SPM_SW_CG))
        disable_clock(MT_CG_SPM_SW_CG, "SPM_MCDI");
#endif    
    
    clc_notice("spm_leave_MCDI : OK.\r\n");

    En_SPM_MCDI = 0;


    /* Un-Mask ARM i bit */
    //asm volatile("cpsie i @ arch_local_irq_enable" : : : "memory", "cc"); // clear i bit to enable interrupt
    local_irq_restore(flags);
    return 0;
}
#endif

//extern u32 cpu_pdn_cnt;
void spm_mcdi_wfi(void)
{   
        volatile u32 core_id;
        //u32 clc_counter;
        //unsigned long flags;  
        //u32 temp_address;

        core_id = (u32)smp_processor_id();
        
        
        if(core_id == 0)
        {

            if(MCDI_Test_Mode == 1)
            {
                //clc_notice("SPM_FC1_PWR_CON %x, cpu_pdn_cnt %d.\n",spm_read(SPM_FC1_PWR_CON),cpu_pdn_cnt);
                clc_notice("core_%d set wfi_sel.\n", core_id);   
            }

            spm_wfi_sel(pcm_config_mcdi.wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK );

            spm_mcdi_poll_mask(core_id,pcm_config_mcdi.wfi_sel);  
            if(MCDI_Test_Mode == 1)
            {
                clc_notice("core_%d mask polling done.\n", core_id);   
            }
            wfi_with_sync(); // enter wfi 

            //spm_get_wakeup_status(&pcm_config_mcdi);
            
            if(MCDI_Test_Mode == 1)
                clc_notice("core_%d exit wfi.\n", core_id);                 

            //if(MCDI_Test_Mode == 1)
                //mdelay(10);  // delay 10 ms    
       

        }
        else // Core 1 Keep original IRQ
        {
            if(MCDI_Test_Mode == 1)
            {
                clc_notice("core_%d set wfi_sel.\n", core_id);   
            }
            spm_wfi_sel(pcm_config_mcdi.wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );

            ////clc_notice("core_%d enter wfi.\n", core_id);
            spm_mcdi_poll_mask(core_id,pcm_config_mcdi.wfi_sel); 
            if(MCDI_Test_Mode == 1)
            {
                clc_notice("core_%d mask polling done.\n", core_id);   
            }            
            if (!cpu_power_down(DORMANT_MODE)) 
            {
                switch_to_amp();  
                
                /* do not add code here */
                wfi_with_sync();
            }
            switch_to_smp(); 
            spm_get_wakeup_status(&pcm_config_mcdi);
            cpu_check_dormant_abort();
            if(MCDI_Test_Mode == 1)
                clc_notice("core_%d exit wfi.\n", core_id);
#if 0
            if(MCDI_Test_Mode == 1)
            {
                // read/clear XGPT status: 72 need to confirm
                if(core_id == 1)
                {
                    gpt_check_and_ack_irq(GPT4);
                    #if 0
                    if(((spm_read(0xf0008004)>>0) & 0x1) == 0x1 )
                    {
                        spm_write(0xf0008008, (0x1<<0));
                    }
                    #endif
                }
                //mdelay(10);  // delay 10 ms
             }
#endif             
        }

}       



// ==============================================================================

void spm_disable_sodi(void)
{
    spin_lock(&spm_sodi_lock);

    spm_sodi_disable_counter++;
    clc_debug("spm_disable_sodi() : spm_sodi_disable_counter = 0x%x\n", spm_sodi_disable_counter);    

    if(spm_sodi_disable_counter > 0)
    {
        spm_direct_disable_sodi();
    }

    spin_unlock(&spm_sodi_lock);
}

void spm_enable_sodi(void)
{
    spin_lock(&spm_sodi_lock);

    spm_sodi_disable_counter--;
    clc_debug("spm_enable_sodi() : spm_sodi_disable_counter = 0x%x\n", spm_sodi_disable_counter);    
    
    if(spm_sodi_disable_counter <= 0)
    {
        spm_direct_enable_sodi();
    }

    spin_unlock(&spm_sodi_lock);
}

void spm_disable_sodi_user(void)
{
    if( pcm_config_mcdi.sodi_en == false )
        return;
    else
        spm_disable_sodi();
    pcm_config_mcdi.sodi_en=false;
}
void spm_enable_sodi_user(void)
{
    if( pcm_config_mcdi.sodi_en == true )
        return;
    else
        spm_enable_sodi();
    pcm_config_mcdi.sodi_en=true;
}

bool spm_is_sodi_user_en(void)
{
    return pcm_config_mcdi.sodi_en;
}
#if 0
static int spm_mcdi_probe(struct platform_device *pdev)
{
    int hibboot = 0;
    hibboot = get_env("hibboot") == NULL ? 0 : simple_strtol(get_env("hibboot"), NULL, 10);

    // set SPM_MP_CORE0_AUX
    spm_mcdi_init_core_mux();


#if MCDI_KICK_PCM

    clc_notice("spm_mcdi_probe start.\n");        
    if (1 == hibboot)
    {
        clc_notice("[%s] skip spm_go_to_MCDI due to hib boot\n", __func__);
    }
    else
    {
        spm_go_to_MCDI();
        if(pcm_config_mcdi.sodi_en==true)
            spm_direct_enable_sodi();
        else
            spm_disable_sodi();
    } 
    
#endif
    
    return 0;

}

static void spm_mcdi_early_suspend(struct early_suspend *h) 
{
#if MCDI_KICK_PCM
    clc_notice("spm_mcdi_early_suspend start.\n");
    spm_leave_MCDI();    
#endif

}

static void spm_mcdi_late_resume(struct early_suspend *h) 
{
    #if MCDI_KICK_PCM
    clc_notice("spm_mcdi_late_resume start.\n");
    spm_go_to_MCDI();    
    #endif  
}

static struct platform_driver mtk_spm_mcdi_driver = {
    .remove     = NULL,
    .shutdown   = NULL,
    .probe      = spm_mcdi_probe,
    .suspend	= NULL,
    .resume		= NULL,
    .driver     = {
        .name = "mtk-spm-mcdi",
    },
};

static struct early_suspend mtk_spm_mcdi_early_suspend_driver =
{
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 251,
    .suspend = spm_mcdi_early_suspend,
    .resume  = spm_mcdi_late_resume,
};

/***************************
* show current SPM-MCDI stauts
****************************/
static int spm_mcdi_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (En_SPM_MCDI)
        p += sprintf(p, "SPM MCDI+Thermal Protect enabled.\n");
    else
        p += sprintf(p, "SPM MCDI disabled, Thermal Protect only.\n");

    len = p - buf;
    return len;
}

/************************************
* set SPM-MCDI stauts by sysfs interface
*************************************/
static ssize_t spm_mcdi_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 0)
        {
            spm_leave_MCDI();
        }
        else if (enabled == 1)
        {
            spm_go_to_MCDI();  
        }
        else if (enabled == 2)
        {
            clc_notice("spm_mcdi_LDVT_sodi() (argument_0 = %d)\n", enabled);
            spm_mcdi_LDVT_sodi();    
        }
        else if (enabled == 3)
        {
            clc_notice("spm_mcdi_LDVT_mcdi() (argument_0 = %d)\n", enabled);
            spm_mcdi_LDVT_mcdi();    
        }
        else if (enabled == 4)
        {
            En_SPM_MCDI = 1;
        }
        else
        {
            clc_notice("bad argument_0!! (argument_0 = %d)\n", enabled);
        }
    }
    else
    {
            clc_notice("bad argument_1!! \n");
    }

    return count;

}

/************************************
* set SPM-SODI Enable by sysfs interface
*************************************/
static ssize_t spm_user_sodi_en(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 0)
        {
            spm_disable_sodi();  
        }
        else if (enabled == 1)
        {
            spm_enable_sodi();    
        }
    }
    else
    {
            clc_notice("bad argument_1!! \n");
    }

    return count;
}

static int __init spm_mcdi_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;    
    struct proc_dir_entry *mt_mcdi_dir = NULL;
    int mcdi_err = 0;

    mt_mcdi_dir = proc_mkdir("mcdi", NULL);
    if (!mt_mcdi_dir)
    {
        clc_notice("[%s]: mkdir /proc/mcdi failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("mcdi_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_mcdi_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = spm_mcdi_debug_read;
            mt_entry->write_proc = spm_mcdi_debug_write;
        }

        mt_entry = create_proc_entry("sodi_en", S_IRUGO | S_IWUSR | S_IWGRP, mt_mcdi_dir);
        if (mt_entry)
        {
            mt_entry->write_proc = spm_user_sodi_en;
        }
    }

    mcdi_err = platform_driver_register(&mtk_spm_mcdi_driver);
    
    if (mcdi_err)
    {
        clc_notice("spm mcdi driver callback register failed..\n");
        return mcdi_err;
    }

    clc_notice("spm mcdi driver callback register OK..\n");

    register_early_suspend(&mtk_spm_mcdi_early_suspend_driver);

    clc_notice("spm mcdi driver early suspend callback register OK..\n");
    
    return 0;

}
#endif
static void __exit spm_mcdi_exit(void)
{
    clc_notice("Exit SPM-MCDI\n\r");
}


u32 En_SPM_MCDI = 0;
void spm_check_core_status_before(u32 target_core)
{
    u32 target_core_temp,hotplug_out_core_id;
    volatile u32 core_id;

    if(En_SPM_MCDI != 1)
    {
        return;
    }
    
    core_id = (u32)smp_processor_id();
    
    target_core_temp = target_core & 0xf;

    hotplug_out_core_id = ((spm_read(SPM_MP_CORE0_AUX) & 0x1)<<0) | ((spm_read(SPM_MP_CORE1_AUX) & 0x1)<<1);    

    target_core_temp &= (~hotplug_out_core_id);

    //clc_notice("issue IPI, spm_check_core_status_before = 0x%x\n", target_core_temp);

    if( target_core_temp == 0x0)
    {
        return;
    }

    // set IPI SPM register ==================================================

    switch (core_id)
    {  
        case 0 : spm_write(SPM_MP_CORE0_AUX, (spm_read(SPM_MP_CORE0_AUX) | (target_core_temp << 1)) );  break;                     
        case 1 : spm_write(SPM_MP_CORE1_AUX, (spm_read(SPM_MP_CORE1_AUX) | (target_core_temp << 1)) );  break;                     

        default : break;
    }    

}


void spm_check_core_status_after(u32 target_core)
{

    u32 target_core_temp, clc_counter, spm_core_pws, hotplug_out_core_id;
    volatile u32 core_id;

    if(En_SPM_MCDI != 1)
    {
        return;
    }
    
    core_id = (u32)smp_processor_id();
    
    target_core_temp = target_core & 0xf;

    hotplug_out_core_id = ((spm_read(SPM_MP_CORE0_AUX) & 0x1)<<0) | ((spm_read(SPM_MP_CORE1_AUX) & 0x1)<<1);

    target_core_temp &= (~hotplug_out_core_id);

    //clc_notice("issue IPI, spm_check_core_status_after = 0x%x\n", target_core_temp);

    if( target_core_temp == 0x0)
    {
        return;
    }

    // check CPU wake up ==============================================

    clc_counter = 0;
    
    while(1)
    {    
        // power_state => 1: power down
        spm_core_pws = ((spm_read(SPM_FC0_PWR_CON) == 0x4d) ? 0 : 1) | ((spm_read(SPM_FC1_PWR_CON) == 0x4d) ? 0 : 2);  // power_state => 1: power down

        if( (target_core_temp & ((~spm_core_pws) & 0xf)) == target_core_temp )
        {
            break;
        }
        
        clc_counter++;

        if(clc_counter >= 100)
        {
            spm_notice("spm_check_core_status_after : check CPU wake up failed.(0x%x, 0x%x)\n", target_core_temp, ((~spm_core_pws) & 0xf));
            break;
        }
    }

    // clear IPI SPM register ==================================================
    
    switch (core_id)
    {
        case 0 : spm_write(SPM_MP_CORE0_AUX, (spm_read(SPM_MP_CORE0_AUX) & (~(target_core_temp << 1))) );  break;                     
        case 1 : spm_write(SPM_MP_CORE1_AUX, (spm_read(SPM_MP_CORE1_AUX) & (~(target_core_temp << 1))) );  break;
        default : break;
    }

}


void spm_hot_plug_in_before(u32 target_core)
{

    spm_notice("spm_hot_plug_in_before()........ target_core = 0x%x\n", target_core);

    switch (target_core)
    {
        case 0 : spm_write(SPM_MP_CORE0_AUX, (spm_read(SPM_MP_CORE0_AUX) & (~0x1U)));  break;                     
        case 1 : spm_write(SPM_MP_CORE1_AUX, (spm_read(SPM_MP_CORE1_AUX) & (~0x1U)));  break;
        default : break;
    }

}

void spm_hot_plug_out_after(u32 target_core)
{

    spm_notice("spm_hot_plug_out_after()........ target_core = 0x%x\n", target_core);    

    switch (target_core)
    {

        case 0 : spm_write(SPM_MP_CORE0_AUX, (spm_read(SPM_MP_CORE0_AUX) | 0x1));  break;                     
        case 1 : spm_write(SPM_MP_CORE1_AUX, (spm_read(SPM_MP_CORE1_AUX) | 0x1));  break;                     
        default : break;
    }

}

void spm_direct_disable_sodi(void)
{
    u32 clc_temp;

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp |= (0x1<<13);
    
    spm_write(SPM_CLK_CON, clc_temp);  
}

void spm_direct_enable_sodi(void)
{
    u32 clc_temp;

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp &= 0xffffdfff; // ~(0x1<<13);

    spm_write(SPM_CLK_CON, clc_temp);    
}

void spm_mcdi_init_core_mux(void)
{
    // set SPM_MP_CORE0_AUX
    spm_write(SPM_MP_CORE0_AUX, 0x0);
    spm_write(SPM_MP_CORE1_AUX, 0x0); 

}

void spm_mcdi_clean(void)
{
    u32 spm_counter;
    u32 spm_core_pws, hotplug_out_core_id;
    
    // trigger cpu wake up event
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0x1);   

    // polling SPM_SLEEP_ISR_STATUS ===========================
    spm_counter = 0;
    
    while(((spm_read(SPM_SLEEP_ISR_STATUS)>>3) & 0x1) == 0x0)
    {
        if(spm_counter >= 10000)
        {
            // set cpu wake up event = 0
            spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0x0); 
            return;
        }
        spm_counter++;
    }

    // set cpu wake up event = 0
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0x0);   
   
    // clean SPM_SLEEP_ISR_STATUS ============================
    spm_write(SPM_SLEEP_ISR_MASK, 0x0008);
    spm_write(SPM_SLEEP_ISR_STATUS, 0x0018);

    //disable IO output for regiser 0 and 7
    spm_write(SPM_PCM_PWR_IO_EN, 0x0);  

    // print spm debug log ===================================
    spm_pcm_dump_regs();

    // clean wakeup event raw status
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, 0xffffffff);

    // check dram controller setting ==============================
//    spm_check_dramc_for_pcm();//72 SPM don't modify DRAM setting, so must remive it, remove it latter
    
    // check cpu power ======================================
    spm_core_pws = ((spm_read(SPM_FC0_PWR_CON) == 0x4d) ? 0 : 1) | ((spm_read(SPM_FC1_PWR_CON) == 0x4d) ? 0 : 2); // power_state => 1: power down

    hotplug_out_core_id = ((spm_read(SPM_MP_CORE0_AUX) & 0x1)<<0) | ((spm_read(SPM_MP_CORE1_AUX) & 0x1)<<1); // 1: hotplug out


    if( spm_core_pws != hotplug_out_core_id )
    {
        spm_notice("spm_leave_MCDI : failed_1.(0x%x, 0x%x)\r\n", spm_core_pws, hotplug_out_core_id); 
    }       
}

void spm_mcdi_poll_mask(u32 core_id,bool core_wfi_sel[])
{
    while((spm_read(SPM_SLEEP_CPU_IRQ_MASK) & (0x1 << core_id) )==0);
#if 0        
    {
        if(core_id==0)
            spm_wfi_sel(core_wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK );//check Core IRQ Mask  
        else    
            spm_wfi_sel(core_wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );;//check Core IRQ Mask  
    }
#endif

}


//late_initcall(spm_mcdi_init);

//MODULE_DESCRIPTION("MT6589 SPM-Idle Driver v0.1");
