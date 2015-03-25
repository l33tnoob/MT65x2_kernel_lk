#include <linux/init.h>        /* For init/exit macros */
#include <linux/module.h>      /* For MODULE_ marcros  */
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>

#include <asm/uaccess.h>
#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_wdt.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/aee.h>
#include <mach/sync_write.h>
#include <mach/ext_wd_drv.h>

#include <mach/wd_api.h>





//#undef  DRV_WriteReg32
//#define DRV_WriteReg32(a, v)			mt65xx_reg_sync_writel(v, a) //write32

extern void aee_wdt_printf(const char *fmt, ...);

/**---------------------------------------------------------------------
 * Sub feature switch region
 *----------------------------------------------------------------------
 */
#define NO_DEBUG 1

/*----------------------------------------------------------------------
 *   IRQ ID 
 *--------------------------------------------------------------------*/
#define AP_RGU_WDT_IRQ_ID    MT_WDT_IRQ_ID

/* 
 * internal variables 
 */
//static char expect_close; // Not use
//static spinlock_t rgu_reg_operation_spinlock = SPIN_LOCK_UNLOCKED;
static DEFINE_SPINLOCK(rgu_reg_operation_spinlock);
#ifndef CONFIG_KICK_SPM_WDT
static unsigned int timeout;
#endif
static volatile BOOL  rgu_wdt_intr_has_trigger; // For test use
static int g_last_time_time_out_value = 0;
static int g_wdt_enable= 1;
#ifdef CONFIG_KICK_SPM_WDT
#include <mach/mt_spm.h>
extern void spm_wdt_set_irq_cb(irq_handler_t FuncPtr);
extern void spm_wdt_set_fiq_cb(fiq_isr_handler FuncPtr);
static void spm_wdt_init(void);
unsigned int ap_rgu_remap_base;
unsigned int ap_rgu_remap_rst_only;
#endif

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
/*
    this function set the timeout value.
    value: second
*/
void mtk_wdt_set_time_out_value(unsigned int value)
{
	/*
	 * TimeOut = BitField 15:5
	 * Key	   = BitField  4:0 = 0x08
	 */	
	spin_lock(&rgu_reg_operation_spinlock);
	
#ifdef CONFIG_KICK_SPM_WDT		
	spm_wdt_set_reset_length(1000*value);
#else	
	// 1 tick means 512 * T32K -> 1s = T32/512 tick = 64
	// --> value * (1<<6)
	timeout = (unsigned int)(value * ( 1 << 6) );
	timeout = timeout << 5; 
	DRV_WriteReg32(MTK_WDT_LENGTH, (timeout | MTK_WDT_LENGTH_KEY) );
#endif
	spin_unlock(&rgu_reg_operation_spinlock);
}
/*
    watchdog mode:
    debug_en:   debug module reset enable. 
    irq:        generate interrupt instead of reset
    ext_en:     output reset signal to outside
    ext_pol:    polarity of external reset signal
    wdt_en:     enable watch dog timer
*/
void mtk_wdt_mode_config(	BOOL dual_mode_en, 
					BOOL irq, 
					BOOL ext_en, 
					BOOL ext_pol, 
					BOOL wdt_en )
{  
	spin_lock(&rgu_reg_operation_spinlock);

#ifdef CONFIG_KICK_SPM_WDT	
	spm_wdt_config(wdt_en);
#else
	unsigned int tmp;	
	//printk(" mtk_wdt_mode_config  mode value=%x,pid=%d\n",DRV_Reg32(MTK_WDT_MODE),current->pid);
	tmp = DRV_Reg32(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;

	// Bit 0 : Whether enable watchdog or not
	if(wdt_en == TRUE)
		tmp |= MTK_WDT_MODE_ENABLE;
	else
		tmp &= ~MTK_WDT_MODE_ENABLE;

	// Bit 1 : Configure extern reset signal polarity.
	if(ext_pol == TRUE)
		tmp |= MTK_WDT_MODE_EXT_POL;
	else
		tmp &= ~MTK_WDT_MODE_EXT_POL;

	// Bit 2 : Whether enable external reset signal
	if(ext_en == TRUE)
		tmp |= MTK_WDT_MODE_EXTEN;
	else
		tmp &= ~MTK_WDT_MODE_EXTEN;

	// Bit 3 : Whether generating interrupt instead of reset signal
	if(irq == TRUE)
		tmp |= MTK_WDT_MODE_IRQ;
	else
		tmp &= ~MTK_WDT_MODE_IRQ;

	// Bit 6 : Whether enable debug module reset
	if(dual_mode_en == TRUE)
		tmp |= MTK_WDT_MODE_DUAL_MODE;
	else
		tmp &= ~MTK_WDT_MODE_DUAL_MODE;

	// Bit 4: WDT_Auto_restart, this is a reserved bit, we use it as bypass powerkey flag.
	//		Because HW reboot always need reboot to kernel, we set it always.
	tmp |= MTK_WDT_MODE_AUTO_RESTART;

	DRV_WriteReg32(MTK_WDT_MODE,tmp);
	//dual_mode(1); //always dual mode
	//mdelay(100);
	//printk(KERN_INFO " mtk_wdt_mode_config  mode value=%x, tmp:%x,pid=%d\n",DRV_Reg32(MTK_WDT_MODE), tmp,current->pid);
#endif		
	spin_unlock(&rgu_reg_operation_spinlock);
}

int mtk_wdt_enable(enum wk_wdt_en en)
{  
	spin_lock(&rgu_reg_operation_spinlock);

#ifdef CONFIG_KICK_SPM_WDT
	spm_wdt_config(en);
	if(WK_WDT_EN == en)
	  	g_wdt_enable = 1;
	if(WK_WDT_DIS == en)
	  	g_wdt_enable = 0;

#else
	unsigned int tmp;
	tmp = DRV_Reg32(MTK_WDT_MODE);
	
	tmp |= MTK_WDT_MODE_KEY;
	if(WK_WDT_EN == en)
	{
	  tmp |= MTK_WDT_MODE_ENABLE;
	  g_wdt_enable = 1;
	}
	if(WK_WDT_DIS == en)
	{
	  tmp &= ~MTK_WDT_MODE_ENABLE;
	  g_wdt_enable = 0;
	}
    printk("mtk_wdt_enable value=%x,pid=%d\n",tmp,current->pid);
	DRV_WriteReg32(MTK_WDT_MODE,tmp);
#endif			//CONFIG_KICK_SPM_WDT
    
	spin_unlock(&rgu_reg_operation_spinlock);

	return 0;
}
 
int  mtk_wdt_confirm_hwreboot(void)
{
    //aee need confirm wd can hw reboot
    aee_sram_fiq_log("only hw reboot mode ++\n");
    mtk_wdt_mode_config(FALSE, FALSE, TRUE, FALSE, TRUE);
		aee_sram_fiq_log("only hw reboot mode --\n");
    return 0;
}

int mtk_is_wdt_enable(void)	//used by spm
{
	return g_wdt_enable;
}

void mtk_wdt_restart(enum wd_restart_type type)
	{
	if(type == WD_TYPE_NORMAL) 
	{     
	      //printk("WDT:ext restart\n" );
	      spin_lock(&rgu_reg_operation_spinlock);
#ifdef CONFIG_KICK_SPM_WDT
				spm_wdt_restart();
#else	      
	      DRV_WriteReg32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
#endif
	      spin_unlock(&rgu_reg_operation_spinlock);
	}
	else if(type == WD_TYPE_NOLOCK) 
	{
#ifdef CONFIG_KICK_SPM_WDT
				spm_wdt_restart();
#else			
	      *(volatile u32 *)( MTK_WDT_RESTART) =MTK_WDT_RESTART_KEY ;
#endif
	}
	else
	{
	      //printk("WDT:[mtk_wdt_restart] type error pid =%d\n",type,current->pid);
	}
}

void wdt_dump_reg(void)
{
	printk("****************dump wdt reg start*************\n");
	printk("MTK_WDT_MODE:0x%x\n", DRV_Reg32(MTK_WDT_MODE));
	printk("MTK_WDT_LENGTH:0x%x\n", DRV_Reg32(MTK_WDT_LENGTH));
	printk("MTK_WDT_RESTART:0x%x\n", DRV_Reg32(MTK_WDT_RESTART));
	printk("MTK_WDT_STATUS:0x%x\n", DRV_Reg32(MTK_WDT_STATUS));
	printk("MTK_WDT_INTERVAL:0x%x\n", DRV_Reg32(MTK_WDT_INTERVAL));
	printk("MTK_WDT_SWRST:0x%x\n", DRV_Reg32(MTK_WDT_SWRST));
	printk("MTK_WDT_NONRST_REG:0x%x\n", DRV_Reg32(MTK_WDT_NONRST_REG));
	printk("MTK_WDT_NONRST_REG2:0x%x\n", DRV_Reg32(MTK_WDT_NONRST_REG2));
	printk("MTK_WDT_REQ_MODE:0x%x\n", DRV_Reg32(MTK_WDT_REQ_MODE));
	printk("MTK_WDT_REQ_IRQ_EN:0x%x\n", DRV_Reg32(MTK_WDT_REQ_IRQ_EN));
	printk("MTK_WDT_DRAMC_CTL:0x%x\n", DRV_Reg32(MTK_WDT_DRAMC_CTL));
	printk("****************dump wdt reg end*************\n");
	
}


void wdt_arch_reset(char mode)
{

    unsigned int wdt_mode_val;
	printk("wdt_arch_reset called@Kernel mode =%c\n",mode);
	
	spin_lock(&rgu_reg_operation_spinlock);
	
#ifdef CONFIG_KICK_SPM_WDT
	ap_rgu_remap_base = ap_rgu_remap_rst_only;
#endif

	/* Watchdog Rest */
	DRV_WriteReg32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
	wdt_mode_val = DRV_Reg32(MTK_WDT_MODE);
	printk("wdt_arch_reset called MTK_WDT_MODE =%x \n",wdt_mode_val);
	/* clear autorestart bit: autoretart: 1, bypass power key, 0: not bypass power key */
	wdt_mode_val &=(~MTK_WDT_MODE_AUTO_RESTART);
	/* make sure WDT mode is hw reboot mode, can not config isr mode  */
	wdt_mode_val &=(~(MTK_WDT_MODE_IRQ|MTK_WDT_MODE_ENABLE | MTK_WDT_MODE_DUAL_MODE));
	if(mode)
	{
		/* mode != 0 means by pass power key reboot, We using auto_restart bit as by pass power key flag */
		 wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN|MTK_WDT_MODE_AUTO_RESTART);
		 
	}else
	{
	       wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN);
		 
	}

	wdt_mode_val &= ~MTK_WDT_MODE_DDRRSV_MODE;
	DRV_WriteReg32(MTK_WDT_MODE,wdt_mode_val);
	printk("wdt_arch_reset called end  MTK_WDT_MODE =%x \n",wdt_mode_val);
	udelay(100);
	DRV_WriteReg32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
        printk("wdt_arch_reset: SW_reset happen\n");
        
#ifdef CONFIG_KICK_SPM_WDT
	iounmap((void*)ap_rgu_remap_base);
#endif	
        
	spin_unlock(&rgu_reg_operation_spinlock);

	while (1)
	{
	    wdt_dump_reg();
		printk("wdt_arch_reset error\n");
	}
	
}
EXPORT_SYMBOL(wdt_arch_reset);

int mtk_wdt_swsysret_config(int bit,int set_value)
{
    unsigned int wdt_sys_val;
	spin_lock(&rgu_reg_operation_spinlock);
	wdt_sys_val = DRV_Reg32(MTK_WDT_SWSYSRST);
	printk("fwq2 before set wdt_sys_val =%x\n",wdt_sys_val);
    wdt_sys_val |= MTK_WDT_SWSYS_RST_KEY;
	switch(bit)
	{
	  case MTK_WDT_SWSYS_RST_MD_RST:
	  	   if(1==set_value)
	  	   {
	  	     wdt_sys_val |= MTK_WDT_SWSYS_RST_MD_RST;
	  	   }
		   if(0==set_value)
		   {
		     wdt_sys_val &= ~MTK_WDT_SWSYS_RST_MD_RST;
		   }
	  	break;
	 case MTK_WDT_SWSYS_RST_MD_LITE_RST:
	  	   if(1==set_value)
	  	   {
	  	     wdt_sys_val |= MTK_WDT_SWSYS_RST_MD_LITE_RST;
	  	   }
		   if(0==set_value)
		   {
		     wdt_sys_val &= ~MTK_WDT_SWSYS_RST_MD_LITE_RST;
		   }
	  	break;
	  	   
	}
	DRV_WriteReg32(MTK_WDT_SWSYSRST,wdt_sys_val);
	spin_unlock(&rgu_reg_operation_spinlock);

	mdelay(10);
	printk("after set wdt_sys_val =%x,wdt_sys_val=%x\n",DRV_Reg32(MTK_WDT_SWSYSRST),wdt_sys_val);
    return 0;
}

int mtk_wdt_request_en_set(int mark_bit,WD_REQ_CTL en)
{
    int res=0;
	spin_lock(&rgu_reg_operation_spinlock);

#ifndef CONFIG_KICK_SPM_WDT
	unsigned int tmp;	
	tmp = DRV_Reg32(MTK_WDT_REQ_MODE);
	tmp |=  MTK_WDT_REQ_MODE_KEY;

    if(MTK_WDT_REQ_MODE_SPM_SCPSYS == mark_bit)
{
	 	if(WD_REQ_EN == en){
            tmp |= (MTK_WDT_REQ_MODE_SPM_SCPSYS);
	 	}
		if(WD_REQ_DIS == en){
			tmp &=~(MTK_WDT_REQ_MODE_SPM_SCPSYS);
		}
    }
	else if(MTK_WDT_REQ_MODE_SPM_THERMAL == mark_bit)
	{
	 	if(WD_REQ_EN == en){
            tmp |= (MTK_WDT_REQ_MODE_SPM_THERMAL);
	 	}
		if(WD_REQ_DIS == en){
			tmp &=~(MTK_WDT_REQ_MODE_SPM_THERMAL);
		}
	}
	else		
	{
		    res =-1;
}

		DRV_WriteReg32(MTK_WDT_REQ_MODE,tmp);
#else
		res =-1;
#endif

   spin_unlock(&rgu_reg_operation_spinlock);
   return res;

}

int mtk_wdt_request_mode_set(int mark_bit,WD_REQ_MODE mode)
{
    int res=0;
	spin_lock(&rgu_reg_operation_spinlock);
	
#ifndef CONFIG_KICK_SPM_WDT		
	unsigned int tmp;
	tmp = DRV_Reg32(MTK_WDT_REQ_IRQ_EN);
	tmp |=  MTK_WDT_REQ_IRQ_KEY;
    if(MTK_WDT_REQ_MODE_SPM_SCPSYS == mark_bit)
	{
	 	if(WD_REQ_IRQ_MODE == mode){
            tmp |= (MTK_WDT_REQ_IRQ_SPM_SCPSYS_EN);
	 	}
		if(WD_REQ_RST_MODE == mode){
			tmp &=~(MTK_WDT_REQ_IRQ_SPM_SCPSYS_EN);
	}
}
	else if ( MTK_WDT_REQ_MODE_SPM_THERMAL == mark_bit)
{
	 	if(WD_REQ_IRQ_MODE == mode){
            tmp |= (MTK_WDT_REQ_IRQ_SPM_THERMAL_EN);
	 	}
		if(WD_REQ_RST_MODE == mode){
			tmp &=~(MTK_WDT_REQ_IRQ_SPM_THERMAL_EN);
		}
}
	else
{
		res =-1;
    }

   DRV_WriteReg32(MTK_WDT_REQ_IRQ_EN,tmp);
#else
		res =-1;
#endif   
   spin_unlock(&rgu_reg_operation_spinlock);
   return res;
}

#else 
//-------------------------------------------------------------------------------------------------
//      Dummy functions
//-------------------------------------------------------------------------------------------------
void mtk_wdt_set_time_out_value(unsigned int value){}
void mtk_wdt_mode_config(BOOL dual_mode_en,BOOL irq,	BOOL ext_en, BOOL ext_pol, BOOL wdt_en){}
int mtk_wdt_enable(enum wk_wdt_en en){ return 0;}
void mtk_wdt_restart(enum wd_restart_type type){}
void wdt_arch_reset(char mode){}
int  mtk_wdt_confirm_hwreboot(void){return 0;}
void mtk_wd_suspend(void){}
void mtk_wd_resume(void){}
void wdt_dump_reg(void){}
int mtk_wdt_swsysret_config(int bit,int set_value){ return 0;}
int mtk_wdt_request_mode_set(int mark_bit,WD_REQ_MODE mode){return 0;}
int mtk_wdt_request_en_set(int mark_bit,WD_REQ_CTL en){return 0;}

#endif //#ifndef __USING_DUMMY_WDT_DRV__

#ifndef CONFIG_FIQ_GLUE
static void wdt_report_info (void)
{
    //extern struct task_struct *wk_tsk;
    struct task_struct *task ;
    task = &init_task ;
    
    printk ("Qwdt: -- watchdog time out\n") ;
    for_each_process (task)
    {
        if (task->state == 0)
        {
            printk ("PID: %d, name: %s\n backtrace:\n", task->pid, task->comm) ;
            show_stack (task, NULL) ;
            printk ("\n") ;
        }
    }
    
    
    printk ("backtrace of current task:\n") ;
    show_stack (NULL, NULL) ;
    
       
    printk ("Qwdt: -- watchdog time out\n") ;    
}
#endif

#ifdef CONFIG_FIQ_GLUE
static void wdt_fiq(void *arg, void *regs, void *svc_sp)
{
		struct wd_api*wd_api = NULL;
		get_wd_api(&wd_api);

     #ifdef  CONFIG_MTK_WD_KICKER
     aee_wdt_printf("kick=0x%08x,check=0x%08x,0xf0008000=0x%x, 0xf0008004=0x%x, 0xf0008008=0x%x, 0xf0008040=0x%x, 0xf0008044=0x%x, 0xf0008048=0x%x, 0xf000804c=0x%x, \
            0xf0008010=0x%x, 0xf0008014=0x%x, 0xf0008018=0x%x, 0xf000801c=0x%x\n",wd_api->wd_get_kick_bit(),wd_api->wd_get_check_bit(),DRV_Reg32(0xf0008000), DRV_Reg32(0xf0008004), DRV_Reg32(0xf0008008),DRV_Reg32(0xf0008040),DRV_Reg32(0xf0008044), DRV_Reg32(0xf0008048), DRV_Reg32(0xf000804c),DRV_Reg32(0xf0008010),
            DRV_Reg32(0xf0008014),DRV_Reg32(0xf0008018),DRV_Reg32(0xf000801c));
	 #endif 
     aee_wdt_fiq_info(arg, regs, svc_sp);
#if 0
     asm volatile("mov %0, %1\n\t"
                  "mov fp, %2\n\t"
                 : "=r" (sp)
                 : "r" (svc_sp), "r" (preg[11])
                 );

     *((volatile unsigned int*)(0x00000000)); /* trigger exception */
	 #endif
}
#else //CONFIG_FIQ_GLUE
static irqreturn_t mtk_wdt_isr(int irq, void *dev_id)
{
    printk("fwq mtk_wdt_isr\n" );
#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
	//mt65xx_irq_mask(AP_RGU_WDT_IRQ_ID);
	rgu_wdt_intr_has_trigger = 1;
	wdt_report_info () ;
	BUG();

#endif	
	return IRQ_HANDLED;
}
#endif //CONFIG_FIQ_GLUE

/* 
 * Device interface 
 */
static int mtk_wdt_probe(struct platform_device *dev)
{
	int ret=0;
	unsigned int interval_val;
	
	printk("******** MTK WDT driver probe!! ********\n" );

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */

#ifndef CONFIG_FIQ_GLUE	
    printk("******** MTK WDT register irq ********\n" );
#ifdef CONFIG_KICK_SPM_WDT
		ap_rgu_remap_base = (unsigned int)ioremap_nocache(0x10007000, 0x90);
		spm_wdt_set_irq_cb(mtk_wdt_isr);
#else    
		ret = request_irq(AP_RGU_WDT_IRQ_ID, (irq_handler_t)mtk_wdt_isr, IRQF_TRIGGER_FALLING, "mtk_watchdog", NULL);
#endif		//CONFIG_KICK_SPM_WDT		
#else
    printk("******** MTK WDT register fiq ********\n" );
#ifdef CONFIG_KICK_SPM_WDT
		ap_rgu_remap_base = (unsigned int)ioremap_nocache(0x10007000, 0x90);
		spm_wdt_set_fiq_cb(wdt_fiq);
#else    
	ret = request_fiq(AP_RGU_WDT_IRQ_ID, wdt_fiq, IRQF_TRIGGER_FALLING, NULL);
#endif		//CONFIG_KICK_SPM_WDT
#endif		

   if(ret != 0)
	{
		printk( "mtk_wdt_probe : failed to request irq (%d)\n", ret);
		return ret;
	}
	printk("mtk_wdt_probe : Success to request irq\n");

	/* Set timeout vale and restart counter */
	g_last_time_time_out_value=30;
	mtk_wdt_set_time_out_value(g_last_time_time_out_value);
		
	mtk_wdt_restart(WD_TYPE_NORMAL);

	/**
	 * Set the reset lenght: we will set a special magic key.
	 * For Power off and power on reset, the INTERVAL default value is 0x7FF.
	 * We set Interval[1:0] to different value to distinguish different stage.
	 * Enter pre-loader, we will set it to 0x0
	 * Enter u-boot, we will set it to 0x1
	 * Enter kernel, we will set it to 0x2
	 * And the default value is 0x3 which means reset from a power off and power on reset
	 */
	#define POWER_OFF_ON_MAGIC	(0x3)
	#define PRE_LOADER_MAGIC	(0x0)
	#define U_BOOT_MAGIC		(0x1)
	#define KERNEL_MAGIC		(0x2)
	#define MAGIC_NUM_MASK		(0x3)


#ifdef  CONFIG_MTK_WD_KICKER	// Initialize to dual mode
	printk("mtk_wdt_probe : Initialize to dual mode \n");
	mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, TRUE);
	#else				// Initialize to disable wdt
	printk("mtk_wdt_probe : Initialize to disable wdt \n");
	mtk_wdt_mode_config(FALSE, FALSE, TRUE, FALSE, FALSE);
	g_wdt_enable =0;
	#endif


	/* Update interval register value and check reboot flag */
	interval_val = DRV_Reg32(MTK_WDT_INTERVAL);
	interval_val &= ~(MAGIC_NUM_MASK);
	interval_val |= (KERNEL_MAGIC);
	/* Write back INTERVAL REG */
	DRV_WriteReg32(MTK_WDT_INTERVAL, interval_val);
#endif
   udelay(100);
   printk("mtk_wdt_probe : done WDT_MODE(%x)\n",DRV_Reg32(MTK_WDT_MODE));
	 printk("mtk_wdt_probe : done MTK_WDT_REQ_MODE(%x)\n",DRV_Reg32(MTK_WDT_REQ_MODE));
	 printk("mtk_wdt_probe : done MTK_WDT_REQ_IRQ_EN(%x)\n",DRV_Reg32(MTK_WDT_REQ_IRQ_EN));

#ifdef CONFIG_KICK_SPM_WDT
	iounmap((void*)ap_rgu_remap_base);
#endif	
	return ret;
}

static int mtk_wdt_remove(struct platform_device *dev)
{
	printk("******** MTK wdt driver remove!! ********\n" );

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
	free_irq(AP_RGU_WDT_IRQ_ID, NULL);
#endif
	return 0;
}

static void mtk_wdt_shutdown(struct platform_device *dev)
{	
	printk("******** MTK WDT driver shutdown!! ********\n" );

	//mtk_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	//kick external wdt
	//mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, FALSE);

	mtk_wdt_restart(WD_TYPE_NORMAL);

   printk("******** MTK WDT driver shutdown done ********\n" );
}

void mtk_wd_suspend(void)
{
	if ( g_wdt_enable == 1 ) 
	{	
	//mtk_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	// en debug, dis irq, dis ext, low pol, dis wdt
	mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, FALSE);

			mtk_wdt_restart(WD_TYPE_NORMAL);
	}
	aee_sram_printk("[WDT] suspend\n");
	printk("[WDT] suspend\n");
}

void mtk_wd_resume(void)
{
	if ( g_wdt_enable == 1 ) 
	{
		mtk_wdt_set_time_out_value(g_last_time_time_out_value);
			mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, TRUE);
		mtk_wdt_restart(WD_TYPE_NORMAL);
	}

	aee_sram_printk("[WDT] resume(%d)\n", g_wdt_enable);
	printk("[WDT] resume(%d)\n", g_wdt_enable);
}

void mtk_wdt_suspend(void)
{
		mtk_wd_suspend();
}

void mtk_wdt_resume(void)
{
		mtk_wd_resume();
}

static struct platform_driver mtk_wdt_driver =
{
	.driver     = {
		.name	= "mtk-wdt",
	},
	.probe	= mtk_wdt_probe,
	.remove	= mtk_wdt_remove,
	.shutdown	= mtk_wdt_shutdown,
//	.suspend	= mtk_wdt_suspend,
//	.resume	= mtk_wdt_resume,
};

struct platform_device mtk_device_wdt = {
		.name		= "mtk-wdt",
		.id		= 0,
		.dev		= {
		}
};

#ifdef CONFIG_KICK_SPM_WDT
static void spm_wdt_init(void)
{
	unsigned int tmp;
	ap_rgu_remap_base = (unsigned int)ioremap_nocache(0x10007000, 0x90);
	ap_rgu_remap_rst_only = (unsigned int)ioremap_nocache(0x10007000, 0x90);
	
	xlog_printk(ANDROID_LOG_ERROR, "wdk/mtk_wdt", "******** [MTK_WDT] CONFIG_KICK_SPM_WDT!! ********\n" );
	printk(KERN_ALERT "mtk_wdt_init:  enable CONFIG_KICK_SPM_WDT  QQ!\n");
	
	tmp = DRV_Reg32(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;
	tmp &= ~MTK_WDT_MODE_ENABLE;
	
	//enable output reset signal for PMIC
	tmp |= MTK_WDT_MODE_EXTEN;
	tmp &= ~MTK_WDT_MODE_EXT_POL;
	
	// Bit 4: WDT_Auto_restart, this is a reserved bit, we use it as bypass powerkey flag.
	//		Because HW reboot always need reboot to kernel, we set it always.
	tmp |= MTK_WDT_MODE_AUTO_RESTART;

	DRV_WriteReg32(MTK_WDT_MODE,tmp);	
		
	iounmap((void*)ap_rgu_remap_base);
}
#endif

/*
 * init and exit function
 */
static int __init mtk_wdt_init(void)
{
	int ret;

#ifdef CONFIG_KICK_SPM_WDT
	spm_wdt_init();
#endif	
	
	ret = platform_device_register(&mtk_device_wdt);
	if (ret) {
		printk("****[mtk_wdt_driver] Unable to device register(%d)\n", ret);
		return ret;
	}

	ret = platform_driver_register(&mtk_wdt_driver);
	if (ret) {
		printk("****[mtk_wdt_driver] Unable to register driver (%d)\n", ret);
		return ret;
	}
    printk("mtk_wdt_init ok\n");
	return 0;	
}

static void __exit mtk_wdt_exit (void)
{
}

module_init(mtk_wdt_init);
module_exit(mtk_wdt_exit);

MODULE_AUTHOR("MTK");
MODULE_DESCRIPTION("MT6572 Watchdog Device Driver");
MODULE_LICENSE("GPL");

