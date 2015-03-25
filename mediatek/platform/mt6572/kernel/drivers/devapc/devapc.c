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
#include "mach/mtk_hibernate_dpm.h"
#endif
#include "devapc.h"


/* 
 * Define global variables
 */
static DEFINE_SPINLOCK(g_devapc_lock);
static struct cdev* g_devapc_ctrl = NULL;
static unsigned long g_devapc_flags;
static BOOL g_usb_protected = FALSE;


/*
 * Define Function
 */
/*
 * set_module_apc: set module permission on device apc.
 * @module: the moudle to specify permission
 * @domain_num: domain index number (AP or MD domain)
 * @permission_control: specified permission
 * no return value.
 */
static void set_module_apc(unsigned int module, DEVAPC_DOM domain_num , DEVAPC_ATTR permission_control)
{
    unsigned int base;
    unsigned int clr_bit = 0x3 << ((module % 16) * 2);
    unsigned int set_bit = permission_control << ((module % 16) * 2);

    #if 0
    if( module >= DEVAPC_DEVICE_NUMBER )
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] ERROR, device number %d exceeds the max number!\n", module);
        return;
    }
    #endif

    if (E_DOM_AP == domain_num)
    {
        base = DEVAPC_D0_APC_0 + (module / 16) * 4;
    }
    else if (E_DOM_MD == domain_num)
    {
        base = DEVAPC_D1_APC_0 + (module / 16) * 4;
    }
    else if (E_DOM_CONN == domain_num)
    {
        base = DEVAPC_D2_APC_0 + (module / 16) * 4;
    }
    else
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] ERROR, The setting is error, please check if domain master setting is correct or not !\n");
        return;
    }

    mt65xx_reg_sync_writel(readl(base) & ~clr_bit, base);
    mt65xx_reg_sync_writel(readl(base) | set_bit, base);
    
    return;
}

/*
 * clear_vio_status: clear violation status for each module.
 * @module: the moudle to clear violation status
 * no return value.
 */
static void clear_vio_status(unsigned int module)
{
    unsigned int base = DEVAPC_D0_VIO_STA_0 + (module / 32) * 4;
    unsigned int bit = 0x1 << (module % 32);

    #if 0
    if( module >= DEVAPC_DEVICE_NUMBER )
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] ERROR, device number %d exceeds the max number!\n", module);
        return;
    }
    #endif

    mt65xx_reg_sync_writel(readl(base) | bit, base);

    return;
}

/*
 * unmask_module_irq: unmask device apc irq for specified module.
 * @module: the moudle to unmask
 * no return value.
 */
static void unmask_module_irq(unsigned int module)
{
    unsigned int base = DEVAPC_D0_VIO_MASK_0 + (module / 32) * 4;
    unsigned int bit = 0x1 << (module % 32);

    #if 0
    if( module >= DEVAPC_DEVICE_NUMBER )
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] ERROR, device number %d exceeds the max number!\n", module);
        return;
    }
    #endif

    mt65xx_reg_sync_writel(readl(base) & ~bit, base);
    
    return;
}


static void init_devpac(void)
{
    /* Unmask debug mask */
    mt65xx_reg_sync_writel(readl(DEVAPC_APC_CON) & (~(0x1 << 2)), DEVAPC_APC_CON);
    mt65xx_reg_sync_writel(readl(DEVAPC_PD_APC_CON) & (~(0x1 << 2)), DEVAPC_PD_APC_CON);

    /* Clear debug information */
    mt65xx_reg_sync_writel(0x80000000, DEVAPC_VIO_DBG0);

/* From MT6589 */
#if 0
   mt65xx_reg_sync_writel(readl(0xF0001040) &  (0xFFFFFFFF ^ (1<<6)), 0xF0001040);    
   
   // clear the violation
   mt65xx_reg_sync_writel(0x80000000, DEVAPC0_VIO_DBG0); // clear apc0 dbg info if any
   mt65xx_reg_sync_writel(0x80000000, DEVAPC1_VIO_DBG0); // clear apc1 dbg info if any
   mt65xx_reg_sync_writel(0x80000000, DEVAPC2_VIO_DBG0); // clear apc2 dbg info if any
   mt65xx_reg_sync_writel(0x80000000, DEVAPC3_VIO_DBG0); // clear apc3 dbg info if any
   mt65xx_reg_sync_writel(0x80000000, DEVAPC4_VIO_DBG0); // clear apc4 dbg info if any
   
   mt65xx_reg_sync_writel(readl(DEVAPC0_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC0_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC1_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC1_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC2_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC2_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC3_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC3_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC4_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC4_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC0_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC0_PD_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC1_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC1_PD_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC2_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC2_PD_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC3_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC3_PD_APC_CON);
   mt65xx_reg_sync_writel(readl(DEVAPC4_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC4_PD_APC_CON);
   
   // clean violation status & unmask device apc 0 & 1 
   mt65xx_reg_sync_writel(0x0000007F, DEVAPC0_DXS_VIO_STA);
   mt65xx_reg_sync_writel(0x00FF00F0, DEVAPC0_DXS_VIO_MASK);
#endif

    return;
}

/*
 * start_devapc: start device apc for MD
 */
void start_devapc(void)
{
    int module_index;

    init_devpac();

    for (module_index = 0; module_index < (sizeof(DEVAPC_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        if (DEVAPC_Devices[module_index].device_num == -1)
            break;
            
        if (DEVAPC_Devices[module_index].forbidden == TRUE)
        {
            set_module_apc(DEVAPC_Devices[module_index].device_num, E_DOM_AP, DEVAPC_Devices[module_index].d0_attr);
            set_module_apc(DEVAPC_Devices[module_index].device_num, E_DOM_MD, DEVAPC_Devices[module_index].d1_attr);
            set_module_apc(DEVAPC_Devices[module_index].device_num, E_DOM_CONN, DEVAPC_Devices[module_index].d2_attr);
            clear_vio_status(DEVAPC_Devices[module_index].device_num);
            unmask_module_irq(DEVAPC_Devices[module_index].device_num);
        }
    }

/* Original Design */
#if 0
    for (module_index = 0; module_index < (sizeof(DEVAPC_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        if (DEVAPC_Devices[module_index].device_name == NULL)
            break;
            
        if (DEVAPC_Devices[module_index].forbidden == TRUE)
        {
            set_module_apc(module_index, E_DOM_MD, E_ATTR_L3);
            unmask_module_irq(module_index);
            clear_vio_status(module_index);
        }
    }
#endif

    /* for EMI MPU */
    mt65xx_reg_sync_writel(readl(DEVAPC_D0_VIO_STA_3) | DEVAPC_ABORT_EMI, DEVAPC_D0_VIO_STA_3);

    return;
}


/*
 * start_usb_protection: start usb protection 
 */
void start_usb_protection(void)
{
#if 0
    int module_index;

    init_devpac();

    module_index = 22;
    set_module_apc(module_index, E_DOM_AP, E_ATTR_L3);
    set_module_apc(module_index, E_DOM_MD, E_ATTR_L3);
    set_module_apc(module_index, E_DOM_CONN, E_ATTR_L3);
    clear_vio_status(module_index);
    unmask_module_irq(module_index);

    module_index = 23;
    set_module_apc(module_index, E_DOM_AP, E_ATTR_L3);
    set_module_apc(module_index, E_DOM_MD, E_ATTR_L3);
    set_module_apc(module_index, E_DOM_CONN, E_ATTR_L3);
    clear_vio_status(module_index);
    unmask_module_irq(module_index);
        
    module_index = 29;
    set_module_apc(module_index, E_DOM_AP, E_ATTR_L3);
    set_module_apc(module_index, E_DOM_MD, E_ATTR_L3);
    set_module_apc(module_index, E_DOM_CONN, E_ATTR_L3);
    clear_vio_status(module_index);
    unmask_module_irq(module_index);

    g_usb_protected = TRUE;
#endif

    return;
}

/*
 * stop_usb_protection: start usb protection 
 */
void stop_usb_protection(void)
{
#if 0
    int module_index;

    module_index = 22;
    set_module_apc(module_index, E_DOM_AP, E_ATTR_L0);
    set_module_apc(module_index, E_DOM_MD, E_ATTR_L0);
    set_module_apc(module_index, E_DOM_CONN, E_ATTR_L0);

    module_index = 23;
    set_module_apc(module_index, E_DOM_AP, E_ATTR_L0);
    set_module_apc(module_index, E_DOM_MD, E_ATTR_L0);
    set_module_apc(module_index, E_DOM_CONN, E_ATTR_L0);
        
    module_index = 29;
    set_module_apc(module_index, E_DOM_AP, E_ATTR_L0);
    set_module_apc(module_index, E_DOM_MD, E_ATTR_L0);
    set_module_apc(module_index, E_DOM_CONN, E_ATTR_L0);

    g_usb_protected = FALSE;
#endif

    return;
}


/*
 * test_devapc: test device apc mechanism
 */
void test_devapc(void)
{
    int module_index;

    init_devpac();

    for (module_index = 0; module_index < (sizeof(DEVAPC_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        if (DEVAPC_Devices[module_index].device_num == -1)
            break;
            
        if (DEVAPC_Devices[module_index].forbidden == TRUE)
        {
            set_module_apc(DEVAPC_Devices[module_index].device_num, E_DOM_AP, E_ATTR_L3);
            clear_vio_status(DEVAPC_Devices[module_index].device_num);
            unmask_module_irq(DEVAPC_Devices[module_index].device_num);
        }
    }

/* Original Design */
#if 0
    for (module_index = 0; module_index < (sizeof(DEVAPC_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        if (DEVAPC_Devices[module_index].device_name == NULL)
            break;
            
        if (DEVAPC_Devices[module_index].forbidden == TRUE)
        {
            set_module_apc(module_index, E_DOM_AP, E_ATTR_L3);
            clear_vio_status(module_index);
            unmask_module_irq(module_index);
        }
    }
#endif

    return;
}


static irqreturn_t devapc_violation_irq(int irq, void *dev_id)
{
    int module_index;
    unsigned int sta, status = 0;
    unsigned int dbg0, dbg1;
    unsigned int master_ID;
    unsigned int domain_ID;
    unsigned int r_w_violation;

    for (module_index = 0; module_index < DEVAPC_DEVICE_NUMBER; module_index += 32)
    {
        sta = readl(DEVAPC_D0_VIO_STA_0 + (module_index / 32) * 4);
        xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"[DEVAPC] DEVAPC_D0_VIO_STA_%u = 0x%x\n", module_index/32, sta);
        status |= sta;
    }

    /* for EMI MPU */
#if 0
    status |= DEVAPC_D0_VIO_STA_3;
#endif

    if ((status == 0) && (&g_devapc_ctrl != dev_id))
    {
        xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"[DEVAPC] ERROR DEVAPC_D0_VIO_STA not device apc AP/MM violation!\n");
        return IRQ_NONE;
    }

    spin_lock_irqsave(&g_devapc_lock, g_devapc_flags);
    
    dbg0 = readl(DEVAPC_VIO_DBG0);
    dbg1 = readl(DEVAPC_VIO_DBG1);
      
    master_ID = dbg0 & 0x000000FF;
    domain_ID = (dbg0 >>12) & 0x00000003;
    r_w_violation = (dbg0 >> 28) & 0x00000003;
    
    //xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"Current Proc : \"%s \" (pid: %i) \n", current->comm, current->pid);

    if(r_w_violation == 1)
    {
        xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"Vio Status:0x%x , Addr:0x%x , Master ID:0x%x , Dom ID:0x%x, W\n", status, dbg1, master_ID, domain_ID);
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG ,"Vio Status:0x%x , Addr:0x%x , Master ID:0x%x , Dom ID:0x%x, R\n", status, dbg1, master_ID, domain_ID);
    }

    for (module_index = 0; module_index < (sizeof(DEVAPC_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        if (DEVAPC_Devices[module_index].device_num == -1)
            break;
            
        if (DEVAPC_Devices[module_index].forbidden == TRUE)
        {
            clear_vio_status(DEVAPC_Devices[module_index].device_num);
        }
    }

/* Original Design */
#if 0
    for (module_index = 0; module_index < (sizeof(DEVAPC_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        if (DEVAPC_Devices[module_index].device_name == NULL)
            break;

        if (DEVAPC_Devices[module_index].forbidden == TRUE)
            clear_vio_status(module_index);
    }
#endif
        
    mt65xx_reg_sync_writel(0x80000000 , DEVAPC_VIO_DBG0);

    dbg0 = readl(DEVAPC_VIO_DBG0);
    dbg1 = readl(DEVAPC_VIO_DBG1);
    if ((dbg0 != 0) || (dbg1 != 0)) 
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] DBG Clear FAILED!\n");
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

    start_devapc();

    if (g_usb_protected){
        start_usb_protection();
    }else{
        stop_usb_protection();
    }
    
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
    .id     = -1,
};

static struct platform_driver devapc_driver = {
    .probe      = devapc_probe,
    .remove     = devapc_remove,
    .suspend    = devapc_suspend,
    .resume     = devapc_resume,
    .driver     = {
        .name   = "devapc",
        .owner  = THIS_MODULE,
    },
};

/*
 * devapc_init: module init function.
 */
static int __init devapc_init(void)
{
    int ret;

    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG , "[DEVAPC] module init. \n");

    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG , "[DEVAPC] register platform device. \n");
    ret = platform_device_register(&devapc_device);
    if (ret) {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG , "[DEVAPC] Unable to do device register(%d)\n", ret);
        return ret;
    }

    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG , "[DEVAPC] register platform driver. \n");
    ret = platform_driver_register(&devapc_driver);
    if (ret) {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] Unable to register driver (%d)\n", ret);
        return ret;
    }

    g_devapc_ctrl = cdev_alloc();
    g_devapc_ctrl->owner = THIS_MODULE;

    /* 
     * NoteXXX: Interrupts of vilation (including SPC in SMI, or EMI MPU) are triggered by the device APC.
     *          Need to share the interrupt with the SPC driver. 
     */
    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG , "[DEVAPC] request irq. \n");

    ret = request_irq(MT_APARM_DOMAIN_IRQ_ID, (irq_handler_t)devapc_violation_irq, IRQF_TRIGGER_LOW | IRQF_SHARED, "mt6572_devapc", &g_devapc_ctrl);    
    disable_irq(MT_APARM_DOMAIN_IRQ_ID);   
    if(ret != 0)
    {
        xlog_printk(ANDROID_LOG_ERROR, DEVAPC_TAG ,"[DEVAPC] Failed to request irq! (%d)\n", ret);
        return ret;
    }
    enable_irq(MT_APARM_DOMAIN_IRQ_ID);
 
#ifdef CONFIG_MTK_HIBERNATION
    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG , "[DEVAPC] register swsusp restore noirq func. \n");

    register_swsusp_restore_noirq_func(ID_M_DEVAPC, devapc_pm_restore_noirq, NULL);
#endif

    xlog_printk(ANDROID_LOG_INFO, DEVAPC_TAG , "[DEVAPC] module init done. \n");

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

    return;
}

late_initcall(devapc_init);

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(start_usb_protection);
EXPORT_SYMBOL(stop_usb_protection);
