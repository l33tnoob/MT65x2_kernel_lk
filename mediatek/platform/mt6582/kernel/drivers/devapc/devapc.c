#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/xlog.h>
#include <linux/platform_device.h>


#include "mach/mt_reg_base.h"
#include "mach/mt_device_apc.h"
#include "mach/mt_typedefs.h"
#include "mach/sync_write.h"
#include "mach/irqs.h"
#ifdef CONFIG_MTK_HIBERNATION
#include <mach/mtk_hibernate_dpm.h>
#endif
#include "devapc.h"
#include "mach/mt_clkmgr.h"


static DEFINE_SPINLOCK(g_devapc_lock);
static unsigned long g_devapc_flags;
static BOOL g_usb_protected = FALSE;

static struct cdev* g_devapc_ctrl = NULL;

static void clear_vio_status(unsigned int module);



/*
 * set_module_apc: set module permission on device apc.
 * @module: the moudle to specify permission
 * @devapc_num: device apc index number (device apc 0 or 1)
 * @domain_num: domain index number (AP or MD domain)
 * @permission_control: specified permission
 * no return value.
 */
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)

int start_devapc(void)
{
	mt65xx_reg_sync_writel(readl(DEVAPC0_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC0_PD_APC_CON);
}

#else


static void set_module_apc(unsigned int module, E_MASK_DOM domain_num , APC_ATTR permission_control)
{

    unsigned int apc_index = 0;
    unsigned int apc_bit_index = 0;

    apc_index = module / MOD_NO_IN_1_DEVAPC;
    apc_bit_index = module % MOD_NO_IN_1_DEVAPC;

    if(domain_num == E_DOMAIN_0){
       switch (apc_index){
            case 0: 
                *DEVAPC0_D0_APC_0 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D0_APC_0 |= permission_control << (2 * apc_bit_index);
                break;
            case 1:
                *DEVAPC0_D0_APC_1 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D0_APC_1 |= permission_control << (2 * apc_bit_index);
                break;
            case 2:
                *DEVAPC0_D0_APC_2 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D0_APC_2 |= permission_control << (2 * apc_bit_index);
                break;
            case 3:
                *DEVAPC0_D0_APC_3 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D0_APC_3 |= permission_control << (2 * apc_bit_index);
                break;
            case 4:
                *DEVAPC0_D0_APC_4 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D0_APC_4 |= permission_control << (2 * apc_bit_index);
                break;
            case 5:
                *DEVAPC0_D0_APC_5 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D0_APC_5 |= permission_control << (2 * apc_bit_index);
                break;
            default:
                xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG , "Set_Module_APC : The setting is error, please check if setting is correct or not !\n");
         }
    }
    else if(domain_num == E_DOMAIN_1){

          switch (apc_index){
            case 0: 
                *DEVAPC0_D1_APC_0 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D1_APC_0 |= permission_control << (2 * apc_bit_index);
                break;
            case 1:
                *DEVAPC0_D1_APC_1 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D1_APC_1 |= permission_control << (2 * apc_bit_index);
                break;
            case 2:
                *DEVAPC0_D1_APC_2 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D1_APC_2 |= permission_control << (2 * apc_bit_index);
                break;
            case 3:
                *DEVAPC0_D1_APC_3 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D1_APC_3 |= permission_control << (2 * apc_bit_index);
                break;
            case 4:
                *DEVAPC0_D1_APC_4 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D1_APC_4 |= permission_control << (2 * apc_bit_index);
                break;
            case 5:
                *DEVAPC0_D1_APC_5 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D1_APC_5 |= permission_control << (2 * apc_bit_index);
                break;
            default:
                xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG , "Set_Module_APC : The setting is error, please check if setting is correct or not !\n");
         }

    }
    else if(domain_num == E_DOMAIN_2){
          switch (apc_index){
            case 0: 
                *DEVAPC0_D2_APC_0 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D2_APC_0 |= permission_control << (2 * apc_bit_index);
                break;
            case 1:
                *DEVAPC0_D2_APC_1 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D2_APC_1 |= permission_control << (2 * apc_bit_index);
                break;
            case 2:
                *DEVAPC0_D2_APC_2 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D2_APC_2 |= permission_control << (2 * apc_bit_index);
                break;
            case 3:
                *DEVAPC0_D2_APC_3 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D2_APC_3 |= permission_control << (2 * apc_bit_index);
                break;
            case 4:
                *DEVAPC0_D2_APC_4 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D2_APC_4 |= permission_control << (2 * apc_bit_index);
                break;
            case 5:
                *DEVAPC0_D2_APC_5 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D2_APC_5 |= permission_control << (2 * apc_bit_index);
                break;
            default:
                xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG , "Set_Module_APC : The setting is error, please check if setting is correct or not !\n");
         }
    
    }
    else if(domain_num == E_DOMAIN_3){
        switch (apc_index){
            case 0: 
                *DEVAPC0_D3_APC_0 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D3_APC_0 |= permission_control << (2 * apc_bit_index);
                break;
            case 1:
                *DEVAPC0_D3_APC_1 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D3_APC_1 |= permission_control << (2 * apc_bit_index);
                break;
            case 2:
                *DEVAPC0_D3_APC_2 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D3_APC_2 |= permission_control << (2 * apc_bit_index);
                break;
            case 3:
                *DEVAPC0_D3_APC_3 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D3_APC_3 |= permission_control << (2 * apc_bit_index);
                break;
            case 4:
                *DEVAPC0_D3_APC_4 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D3_APC_4 |= permission_control << (2 * apc_bit_index);
                break;
            case 5:
                *DEVAPC0_D3_APC_5 &= ~(0x3 << (2 * apc_bit_index));
                *DEVAPC0_D3_APC_5 |= permission_control << (2 * apc_bit_index);
                break;
            default:
                xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG , "Set_Module_APC : The setting is error, please check if setting is correct or not !\n");
         }
    }

   
}


/*
 * unmask_module_irq: unmask device apc irq for specified module.
 * @module: the moudle to unmask
 * @devapc_num: device apc index number (device apc 0 or 1)
 * @domain_num: domain index number (AP or MD domain)
 * no return value.
 */
static void unmask_module_irq(unsigned int module)
{
      
    unsigned int apc_index = 0;
    unsigned int apc_bit_index = 0;

    apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
    apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);

    switch (apc_index){
       case 0: 
           *DEVAPC0_D0_VIO_MASK_0 &= ~(0x1 << apc_bit_index);
           break;
       case 1:
           *DEVAPC0_D0_VIO_MASK_1 &= ~(0x1 << apc_bit_index);
           break;
       case 2:
           *DEVAPC0_D0_VIO_MASK_2 &= ~(0x1 << apc_bit_index);
           break;

       default:
           xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG , "UnMask_Module_IRQ : The setting is error, please check if domain master setting is correct or not !\n");
           break;
    }


}

/*
 * start_devapc: start device apc for MD
 */
void start_devapc(void)
{

    int module_index = 0;

   mt65xx_reg_sync_writel(0x80000000, DEVAPC0_VIO_DBG0); // clear apc0 dbg info if any
   
   mt65xx_reg_sync_writel(readl(DEVAPC0_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC0_APC_CON);

   mt65xx_reg_sync_writel(readl(DEVAPC0_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC0_PD_APC_CON);

    for (module_index = 0; module_index<(sizeof(D_APC0_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        if (NULL == D_APC0_Devices[module_index].device_name)
            break;
                
        if (TRUE == D_APC0_Devices[module_index].forbidden)
        {
            clear_vio_status(module_index);
            unmask_module_irq(module_index);
            set_module_apc(module_index, E_DOMAIN_1, E_L3);

        }
    }
    
}

#endif

/*
 * clear_vio_status: clear violation status for each module.
 * @module: the moudle to clear violation status
 * @devapc_num: device apc index number (device apc 0 or 1)
 * @domain_num: domain index number (AP or MD domain)
 * no return value.
 */
static void clear_vio_status(unsigned int module)
{
    
    unsigned int apc_index = 0;
    unsigned int apc_bit_index = 0;
    
    apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
    apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);


    switch (apc_index){
        case 0: 
           *DEVAPC0_D0_VIO_STA_0 = (0x1 << apc_bit_index);
           break;
        case 1:
           *DEVAPC0_D0_VIO_STA_1 = (0x1 << apc_bit_index);
           break;
        case 2:
           *DEVAPC0_D0_VIO_STA_2 = (0x1 << apc_bit_index);
            break;
        case 3:
           *DEVAPC0_D0_VIO_STA_3 = (0x1 << apc_bit_index);
            break;
        default:
           break;
    }
}


static irqreturn_t devapc_violation_irq(int irq, void *dev_id)
{  
    unsigned int dbg0 = 0, dbg1 = 0;
    unsigned int master_ID;
    unsigned int domain_ID;
    unsigned int r_w_violation;
   //unsigned int timeout = 0;

    int module_index;
    

    spin_lock_irqsave(&g_devapc_lock, g_devapc_flags);
        
    dbg0 = readl(DEVAPC0_VIO_DBG0);
    dbg1 = readl(DEVAPC0_VIO_DBG1);              
    master_ID = dbg0 & 0x00000FF;
    domain_ID = (dbg0 >>12) & 0x00000003;
    r_w_violation = (dbg0 >> 28) & 0x00000003;
       
    if(r_w_violation == 1)
    {
      xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"Vio Addr:0x%x , Master ID:0x%x , Dom ID:0x%x, W\n", dbg1, master_ID, domain_ID);
    }
    else
    {
      xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"Vio Addr:0x%x , Master ID:0x%x , Dom ID:0x%x, R\n", dbg1, master_ID, domain_ID);
    }


    for (module_index = 0; module_index< (sizeof(D_APC0_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        if (NULL == D_APC0_Devices[module_index].device_name)
            break;
               
        if (TRUE == D_APC0_Devices[module_index].forbidden)
            clear_vio_status(module_index);
    }
       
    mt65xx_reg_sync_writel(0x80000000 , DEVAPC0_VIO_DBG0);
    dbg0 = readl(DEVAPC0_VIO_DBG0);
    dbg1 = readl(DEVAPC0_VIO_DBG1);
          
    if ((dbg0 !=0) || (dbg1 !=0)) 
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] FAILED!\n");
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] DBG0 = %x, DBG1 = %x\n", dbg0, dbg1);
    }

    spin_unlock_irqrestore(&g_devapc_lock, g_devapc_flags);
  
    return IRQ_HANDLED;
}





static int devapc_probe(struct platform_device *dev)
{
    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"[DEVAPC] module probe. \n");
    start_devapc();
    return 0;
}


static int devapc_remove(struct platform_device *dev)
{
    return 0;
}

static int devapc_suspend(struct platform_device *dev, pm_message_t state)
{
    return 0;
}

static int devapc_resume(struct platform_device *dev)
{
    //xlog_printk(ANDROID_LOG_DEBUG, DEVAPC_TAG ,"[DEVAPC] module resume. \n");
    //start_devapc(); 
    return 0;
}

#ifdef CONFIG_MTK_HIBERNATION
extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq, unsigned int polarity);
int devapc_pm_restore_noirq(struct device *device)
{
    mt_irq_set_sens(MT_APARM_DOMAIN_IRQ_ID, MT_LEVEL_SENSITIVE);
    mt_irq_set_polarity(MT_APARM_DOMAIN_IRQ_ID, MT_POLARITY_LOW);

    return 0;
}
#endif

struct platform_device devapc_device = {
    .name   = "devapc",
    .id        = -1,
};

static struct platform_driver devapc_driver = {
    .probe        = devapc_probe,
    .remove        = devapc_remove,
    .suspend    = devapc_suspend,
    .resume        = devapc_resume,
    .driver     = {
        .name = "devapc",
        .owner = THIS_MODULE,
    },
};



/*
 * devapc_init: module init function.
 */

static int __init devapc_init(void)
{

    int ret;
    /*OPEN DAPC CLOCK*/
    enable_clock(MT_CG_INFRA_EFUSE,"DEVAPC");
    
    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG , "[DEVAPC] module init. \n");

    ret = platform_device_register(&devapc_device);
    if (ret) {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG , "[DEVAPC] Unable to do device register(%d)\n", ret);
        return ret;
    }
    ret = platform_driver_register(&devapc_driver);
    if (ret) {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] Unable to register driver (%d)\n", ret);
        return ret;
    }

    g_devapc_ctrl = cdev_alloc();
    g_devapc_ctrl->owner = THIS_MODULE;

    if(ret != 0)
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] Failed to add devapc device! (%d)\n", ret);
        return ret;
    }

    /* 
     * NoteXXX: Interrupts of vilation (including SPC in SMI, or EMI MPU) are triggered by the device APC.
     *          Need to share the interrupt with the SPC driver. 
     */
    ret = request_irq(MT_APARM_DOMAIN_IRQ_ID, (irq_handler_t)devapc_violation_irq, IRQF_TRIGGER_LOW | IRQF_SHARED, "mt6582_devapc", &g_devapc_ctrl);    
    disable_irq(MT_APARM_DOMAIN_IRQ_ID);
    enable_irq(MT_APARM_DOMAIN_IRQ_ID);
    
    if(ret != 0)
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] Failed to request irq! (%d)\n", ret);
        return ret;
    }
 
#ifdef CONFIG_MTK_HIBERNATION
    register_swsusp_restore_noirq_func(ID_M_DEVAPC, devapc_pm_restore_noirq, NULL);
#endif

    return 0;
}


/*
 * devapc_exit: module exit function.
 */
static void __exit devapc_exit(void)
{
    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"[DEVAPC] DEVAPC module exit\n");

#ifdef CONFIG_MTK_HIBERNATION
    unregister_swsusp_restore_noirq_func(ID_M_DEVAPC);
#endif
}

late_initcall(devapc_init);
MODULE_LICENSE("GPL");

