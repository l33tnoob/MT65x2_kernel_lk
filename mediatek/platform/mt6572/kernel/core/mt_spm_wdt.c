#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/aee.h>
#include <linux/interrupt.h>

#include <mach/irqs.h>
#include <mach/mt_spm.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>
#include <mach/mt_dormant.h>
#include <mach/mt_mci.h>
#include <mach/eint.h>
#include <mach/mtk_ccci_helper.h>
#include <board-custom.h>

#ifdef CONFIG_KICK_SPM_WDT 
 
#define SPM_WDT_FIRMWARE_VER   "[01]PCM-RGU-without_NRP-with_Thermal"
 
static const u32 spm_pcm_wdt[24] = {
    0x1b00001f, 0x00202000, 0x1b80001f, 0x90001000, 0x8880000c, 0x00200000,
    0xd80001a2, 0x17c07c1f, 0xe8208000, 0x100063e0, 0x00000002, 0x1b80001f,
    0x10001000, 0x1840001f, 0x00000001, 0x1890001f, 0x10006014, 0x18c0001f,
    0x10006014, 0xa0978402, 0xe0c00002, 0x1b80001f, 0x10001000, 0xf0000000
};

#define PCM_WDT_BASE        __pa(spm_pcm_wdt)
#define PCM_WDT_LEN         (24)                       /* # words */

#define WDT_PCM_TIMER_VAL   (0)
#define WDT_PCM_WDT_VAL     (5*1024) //need larger than PCM Timer

#define WAKE_SRC_FOR_WDT  WAKE_SRC_THERM

irqreturn_t spm_irq_wdt_woof(int irq, void *dev_id)
{
    int i;
    for(i=0;i<20;i++){
       spm_error("!!!!!![SPM IRQ] WDT WOOF WOOF WOOF!!!!!\n");
       mdelay(1);
    }
    
    return 0;
}

void spm_fiq_wdt_woof(void *arg, void *regs, void *svc_sp)
{
    int i;
    for(i=0;i<20;i++){
       spm_error("!!!!!![SPM FIQ] WDT WOOF WOOF WOOF!!!!!\n");
       mdelay(1);
    }
}

irqreturn_t (*spm_wdt_irq_bark)(int irq, void *dev_id) = &spm_irq_wdt_woof;
void	(*spm_wdt_fiq_bark)(void *arg, void *regs, void *svc_sp) = &spm_fiq_wdt_woof;
void spm_wdt_restart(void);

SPM_PCM_CONFIG pcm_config_wdt ={
    .scenario = SPM_PCM_WDT,
    .ver = SPM_WDT_FIRMWARE_VER,
    .spm_turn_off_26m = false,
    .pcm_firmware_addr =  PCM_WDT_BASE,
    .pcm_firmware_len = PCM_WDT_LEN,
    .pcm_pwrlevel = PWR_LVNA,         //no necessary to set pwr level when suspend
    .spm_request_uart_sleep = false,
    .pcm_vsr = {0,0,0,0,0,0,0,0},
    
    .sync_r0r7=false,
    
    /*Wake up event mask*/
    .md_mask = CONN_MASK,   /* unmask MD1 and MD2 */
    .mm_mask = MMALL_MASK,   /* unmask DISP and MFG */
    
    /*AP Sleep event mask*/
    .wfi_scu_mask=false ,   
    .wfi_l2c_mask=false, 
    .wfi_op=REDUCE_OR,//We need to ignore CPU 1 in FPGA platform
    .wfi_sel = {false,false},
    .timer_val_ms = WDT_PCM_TIMER_VAL,
    .wdt_val_ms = WDT_PCM_WDT_VAL,
    .wake_src = WAKE_SRC_FOR_WDT,
    .reserved = 0x0,
    
    .cpu_pdn = false,
    .infra_pdn = false,
    .coclock_en = false,
    .lock_infra_dcm=false,
    
    /* debug related*/
    .dbg_wfi_cnt=0,
    .wakesta_idx=0
     };
     
wake_reason_t spm_let_the_dog_out(void)
{
    static wake_reason_t last_wr = WR_NONE;
    
    //spm_notice("spm_let_the_dog_out\n");

    //spin_lock_irqsave(&spm_lock, flags);
    
    if (spm_init_pcm(&pcm_config_wdt)==false)
      goto  ERROR;
    
    spm_kick_pcm(&pcm_config_wdt);
        
ERROR:
    //spin_unlock_irqrestore(&spm_lock, flags);

    return last_wr;
}

wake_reason_t spm_let_the_dog_home(void)
{
    spm_wdt_restart();
    spm_write(SPM_PCM_SW_INT_CLEAR,0xf);
    spm_write(SPM_PCM_CON1, (spm_read(SPM_PCM_CON1) & ~CON1_PCM_WDT_EN) | CON1_CFG_KEY);
    //spm_notice("spm_let_the_dog_home\n");
    return WR_NONE;
}

/***********************Exposed API*************************/
void spm_wdt_restart(void) //used to kick wdt.
{
    spm_write(SPM_POWER_ON_VAL1,spm_read(SPM_POWER_ON_VAL1)| R7_wdt_kick_p);
    spm_write(SPM_POWER_ON_VAL1,spm_read(SPM_POWER_ON_VAL1)& ~R7_wdt_kick_p);
		//spm_notice("spm_wdt_restart\n");
}

void spm_wdt_config(bool enable)//used to enable/disable spm wdt.
{
    if(enable)
       spm_let_the_dog_out();
    else
       spm_let_the_dog_home();
}
 
void spm_wdt_set_reset_length(unsigned int value)//used to set wdt timeout interval
{
    pcm_config_wdt.wdt_val_ms = value;
    //spm_notice("spm_wdt_set_reset_length:%d\n",pcm_config_wdt.wdt_val_ms);
}

void spm_wdt_set_irq_cb(irq_handler_t FuncPtr)
{
   spm_wdt_irq_bark = FuncPtr;          // C
   //spm_notice("spm_wdt_set_irq_cb:0x%X\n",FuncPtr);
}

void spm_wdt_set_fiq_cb(fiq_isr_handler FuncPtr)
{
   spm_wdt_fiq_bark = FuncPtr;          // C
   //spm_notice("spm_wdt_set_fiq_cb:0x%X\n",FuncPtr);
}
#endif