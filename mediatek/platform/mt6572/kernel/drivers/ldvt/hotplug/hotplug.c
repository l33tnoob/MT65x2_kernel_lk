#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/cnt32_to_63.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>

//#include <asm/uaccess.h>
//#include <asm/tcm.h>
//#include <mach/timer.h>
//#include <mach/irqs.h>
//

/*
    This is a Kernel driver used by user space. 
    This driver will using the interface of the GPT production driver.  
    We implement some IOCTLs: 
    1) UP CPU1
    2) DOWN CPU1
    3) STRESS 1 UP DOWN CPUS
*/

#define hotplugname                             "uvvp_hotplug"

/*IOCTL code Define*/
#define UVVP_HOTPLUG_UP_CPU1                    _IOW('k', 0, int)
#define UVVP_HOTPLUG_DOWN_CPU1                  _IOW('k', 1, int)
#define UVVP_HOTPLUG_STRESS_1_UP_DOWN_CPUS      _IOW('k', 2, int)


#define STRESS_TEST_1_COUNT                     1000
#define STRESS_TEST_1_DELAY_MS                  11

//Let's use struct GPT_CONFIG for all IOCTL: 
static long uvvp_hotplug_ioctl(struct file *file,
                            unsigned int cmd, unsigned long arg)
{
    #ifdef Lv_debug
    printk("\r\n******** uvvp_hotplug_ioctl cmd[%d]********\r\n",cmd);
    #endif 
    
    /*
     * 20121101 marc.huang 
     * mark to fix build warning
     */
    //void __user *argp = (void __user *)arg;
    //int __user *p = argp;
    
    int i;

    switch (cmd) {
        default:
            return -1;

        case UVVP_HOTPLUG_UP_CPU1:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_up(1) ********\r\n");
            cpu_up(1);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_CPU1:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_down(1) ********\r\n");
            cpu_down(1);
            return 0;
        
        case UVVP_HOTPLUG_STRESS_1_UP_DOWN_CPUS:
            printk("\r\n******** uvvp_hotplug_ioctl stress_test_1 cpu_up/cpu_down(1/2/3) ********\r\n");
            
            //0. turn on cpu 1
            cpu_up(1);
            
            //1. turn off cpu 1
            for (i = 0; i < STRESS_TEST_1_COUNT; ++i)
            {
                cpu_down(1);
                msleep(STRESS_TEST_1_DELAY_MS);
                cpu_up(1);
                msleep(STRESS_TEST_1_DELAY_MS);
            }
            
            return 0;
        
    }

    return 0;    
}

static int uvvp_hotplug_open(struct inode *inode, struct file *file)
{
    return 0;
}


static struct file_operations uvvp_hotplug_fops = {
    .owner              = THIS_MODULE,

    .open               = uvvp_hotplug_open,
    .unlocked_ioctl     = uvvp_hotplug_ioctl,
    .compat_ioctl       = uvvp_hotplug_ioctl,
};

static struct miscdevice uvvp_hotplug_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = hotplugname,
    .fops = &uvvp_hotplug_fops,
};

static int __init uvvp_hotplug_init(void)
{
    misc_register(&uvvp_hotplug_dev);
    return 0;
}

static void __exit uvvp_hotplug_exit(void)
{

}

module_init(uvvp_hotplug_init);
module_exit(uvvp_hotplug_exit);


