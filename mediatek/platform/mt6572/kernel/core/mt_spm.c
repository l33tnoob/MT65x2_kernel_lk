#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <mach/irqs.h>
#include <mach/mt_spm.h>
#include <mach/mt_clkmgr.h>


/**************************************
 * for General
 **************************************/
DEFINE_SPINLOCK(spm_lock);

extern irqreturn_t (*spm_wdt_irq_bark)(int irq, void *dev_id);
extern void	(*spm_wdt_fiq_bark)(void *arg, void *regs, void *svc_sp);

irqreturn_t spm0_irq_handler(int irq, void *dev_id)
{
    spm_error("!!! SPM ISR[0] SHOULD NOT BE EXECUTED !!!\n");

    spin_lock(&spm_lock);
    /* clean ISR status */
    spm_write(SPM_SLEEP_ISR_MASK, 0x0008);
    spm_write(SPM_SLEEP_ISR_STATUS, 0x0018);
    spin_unlock(&spm_lock);

    return IRQ_HANDLED;
}

irqreturn_t spm1_irq_handler(int irq, void *dev_id)
{
    #ifdef CONFIG_KICK_SPM_WDT
        //spin_lock(&spm_lock);
        
        spm_error("Orz.Orz.Orz...SPM WDT Timeout @ IRQ\n");
        
        if(spm_wdt_irq_bark)
         (*spm_wdt_irq_bark)(irq, dev_id);  
    #else
        spm_error("GENE", "!!! SPM ISR[0] SHOULD NOT BE EXECUTED !!!\n");

        spin_lock(&spm_lock);
        /* clean ISR status */
        spm_write(SPM_SLEEP_ISR_MASK, 0x0008);
        spm_write(SPM_SLEEP_ISR_STATUS, 0x0018);
        spin_unlock(&spm_lock);    
    #endif
    
    return IRQ_HANDLED;
}

#ifdef CONFIG_KICK_SPM_WDT
 void spm1_fiq_handler(void *arg, void *regs, void *svc_sp)
{   
    //aee_wdt_printf("Orz.Orz.Orz...SPM WDT Timeout @ FRQ\n");

    if(spm_wdt_fiq_bark)
     (*spm_wdt_fiq_bark)(arg, regs, svc_sp);

    return;
}
#endif

void spm_module_init(void)
{
    int r;
    unsigned long flags;
    
    spm_fs_init();
        
    spin_lock_irqsave(&spm_lock, flags);

#if 1//def SPM_CLOCK_INIT
    /*Only set during bringup init. No need to be changed.*/
    if(clock_is_on(MT_CG_SPM_52M_SW_CG))
        disable_clock(MT_CG_SPM_52M_SW_CG, "SPM");
    if(!clock_is_on(MT_CG_SC_26M_CK_SEL_EN))
       enable_clock(MT_CG_SC_26M_CK_SEL_EN, "SPM");//Enable the feature that SPM can switch bus and audio clock to be 26Mhz
    if(clock_is_on(MT_CG_SC_MEM_CK_OFF_EN))  
        disable_clock(MT_CG_SC_MEM_CK_OFF_EN, "SPM");
    /*Dynamic on/off before entering suspend/DPidle and after leaving suspend/DPidle*/
    if(!clock_is_on(MT_CG_MEMSLP_DLYER_SW_CG))
        enable_clock(MT_CG_MEMSLP_DLYER_SW_CG, "SPM");
    if(!clock_is_on(MT_CG_SPM_SW_CG))//need check with mtcmos owner for spm clk init gating
        enable_clock(MT_CG_SPM_SW_CG, "SPM");

#endif

    spin_unlock_irqrestore(&spm_lock, flags);
    r = request_irq(MT_SPM0_IRQ_ID, spm0_irq_handler, IRQF_TRIGGER_LOW,"mt-spm", NULL);
    if (r) {
        spm_error("SPM IRQ[0] register failed (%d)\n", r);
        WARN_ON(1);
 
   }
    #ifdef CONFIG_KICK_SPM_WDT
        #ifndef CONFIG_FIQ_GLUE 
            //printk("******** MTK WDT register irq ********\n" );
            r = request_irq(MT_SPM1_IRQ_ID, spm1_irq_handler, IRQF_TRIGGER_LOW,"[SPM WDT]", NULL);
        #else
            //printk("******** MTK WDT register fiq ********\n" );
            r = request_fiq(MT_SPM1_IRQ_ID, spm1_fiq_handler, IRQF_TRIGGER_LOW, NULL);
        #endif
    #else
         r = request_irq(MT_SPM1_IRQ_ID, spm1_irq_handler, IRQF_TRIGGER_LOW,"mt-spm", NULL);
    #endif 

    if (r) {
        spm_error("SPM IRQ[1] register failed (%d)\n", r);
        WARN_ON(1);}    
}

bool spm_is_md_sleep(void)
{
    return !(spm_read(SPM_PCM_REG13_DATA) & R13_MD_STATE);
}

bool spm_is_conn_sleep(void)
{
    //return !(spm_read(SPM_PCM_REG13_DATA) & R13_CONN_STATE);
    return !(spm_read(SPM_PCM_REG13_DATA) & R13_CONN_APSRC_REQ);
}


MODULE_DESCRIPTION("MT6589 SPM Driver v0.1");
