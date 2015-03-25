#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <linux/interrupt.h>
#include <mach/mt_reg_base.h>
#include <mach/systracker.h>
#include <mach/sync_write.h>


#define TRACKER_DEBUG 1


/* Some chip do not have reg dump, define a weak to avoid build error */
int __weak mt_reg_dump(char *buf) { return 1; }
EXPORT_SYMBOL(mt_reg_dump);

int enable_watch_point(void);
int disable_watch_point(void);
static int set_watch_point_address(unsigned int wp_phy_address);

static int systracker_probe(struct platform_device *pdev);
static int systracker_remove(struct platform_device *pdev);
static int systracker_suspend(struct platform_device *pdev, pm_message_t state);
static int systracker_resume(struct platform_device *pdev);

static int test_systracker(void);

struct systracker_config_t
{
        int state;
        int enable_timeout;
        int enable_slave_err;
        int enable_wp;
        int enable_irq;
        int timeout_ms;
        int wp_phy_address;
};
static struct systracker_config_t track_config;

static struct platform_driver systracker_driver =
{
    .probe = systracker_probe,
    .remove = systracker_remove,
    .suspend = systracker_suspend,
    .resume = systracker_resume,
    .driver = {
        .name = "systracker",
        .bus = &platform_bus_type,
        .owner = THIS_MODULE,
    },
};

unsigned int is_systracker_device_registered = 0;
unsigned int is_systracker_irq_registered = 0;


int tracker_dump(char *buf)
{
    int ret = -1;
    char *ptr = buf;
    unsigned int reg_value;
    unsigned long size = 0;
    unsigned long offset = 0;
    char str[KSYM_SYMBOL_LEN];
    int i;
    unsigned int entry_valid;
    unsigned int entry_tid;
    unsigned int entry_id;
    unsigned int entry_address;
    unsigned int entry_data_size;
    unsigned int entry_burst_length;


    //if(is_systracker_device_registered)
    {
        /* Get tracker info and save to buf */

        /* BUS_DBG_AR_TRACK_L(__n)
         * [31:0] ARADDR: DBG read tracker entry read address
         */

        /* BUS_DBG_AR_TRACK_H(__n)
         * [14] Valid:DBG read tracker entry valid
         * [13:7] ARID:DBG read tracker entry read ID
         * [6:4] ARSIZE:DBG read tracker entry read data size
         * [3:0] ARLEN: DBG read tracker entry read burst length
         */

        /* BUS_DBG_AR_TRACK_TID(__n)
         * [2:0] BUS_DBG_AR_TRANS0_ENTRY_ID: DBG read tracker entry ID of 1st transaction
         */

#ifdef TRACKER_DEBUG
        printk("Sys Tracker Dump\n");
#endif

        for (i = 0; i < BUS_DBG_NUM_TRACKER; i++) {
            entry_address       = readl(IOMEM(BUS_DBG_AR_TRACK_L(i)));
            reg_value           = readl(IOMEM(BUS_DBG_AR_TRACK_H(i)));
            entry_valid         = extract_n2mbits(reg_value,19,19);
            entry_id            = extract_n2mbits(reg_value,7,18);
            entry_data_size     = extract_n2mbits(reg_value,4,6);
            entry_burst_length  = extract_n2mbits(reg_value,0,3);
            entry_tid           = readl(IOMEM(BUS_DBG_AR_TRANS_TID(i)));

            ptr += sprintf(ptr, " \
read entry = %d, \
valid = 0x%x, \
tid = 0x%x, \
read id = 0x%x, \
address = 0x%x, \
data_size = 0x%x, \
burst_length = 0x%x\n",
                        i,
                        entry_valid,
                        entry_tid,
                        entry_id,
                        entry_address,
                        entry_data_size,
                        entry_burst_length);

#ifdef TRACKER_DEBUG
            printk("\
read entry = %d, \
valid = 0x%x, \
tid = 0x%x, \
read id = 0x%x, \
address = 0x%x, \
data_size = 0x%x, \
burst_length = 0x%x\n",
                        i,
                        entry_valid,
                        entry_tid,
                        entry_id,
                        entry_address,
                        entry_data_size,
                        entry_burst_length);
#endif
        }

        /* BUS_DBG_AW_TRACK_L(__n)
         * [31:0] AWADDR: DBG write tracker entry write address
         */

        /* BUS_DBG_AW_TRACK_H(__n)
         * [14] Valid:DBG   write tracker entry valid
         * [13:7] ARID:DBG  write tracker entry write ID
         * [6:4] ARSIZE:DBG write tracker entry write data size
         * [3:0] ARLEN: DBG write tracker entry write burst length
         */

        /* BUS_DBG_AW_TRACK_TID(__n)
         * [2:0] BUS_DBG_AW_TRANS0_ENTRY_ID: DBG write tracker entry ID of 1st transaction
         */
      for (i = 0; i < BUS_DBG_NUM_TRACKER; i++) {
            entry_address       = readl(IOMEM(BUS_DBG_AW_TRACK_L(i)));
            reg_value           = readl(IOMEM(BUS_DBG_AW_TRACK_H(i)));
            entry_valid         = extract_n2mbits(reg_value,19,19);
            entry_id            = extract_n2mbits(reg_value,7,18);
            entry_data_size     = extract_n2mbits(reg_value,4,6);
            entry_burst_length  = extract_n2mbits(reg_value,0,3);
            entry_tid           = readl(IOMEM(BUS_DBG_AW_TRANS_TID(i)));

            ptr += sprintf(ptr, " \
write entry = %d, \
valid = 0x%x, \
tid = 0x%x, \
write id = 0x%x, \
address = 0x%x, \
data_size = 0x%x, \
burst_length = 0x%x\n",
                        i,
                        entry_valid,
                        entry_tid,
                        entry_id,
                        entry_address,
                        entry_data_size,
                        entry_burst_length);

#ifdef TRACKER_DEBUG
            printk("\
write entry = %d, \
valid = 0x%x, \
tid = 0x%x, \
write id = 0x%x, \
address = 0x%x, \
data_size = 0x%x, \
burst_length = 0x%x\n",
                        i,
                        entry_valid,
                        entry_tid,
                        entry_id,
                        entry_address,
                        entry_data_size,
                        entry_burst_length);
#endif
      }

      return strlen(buf);
    }

    return -1;
}

static ssize_t tracker_run_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%x\n", track_config.state);
}

static ssize_t tracker_run_store(struct device_driver * driver, const char *buf, size_t count)
{
    unsigned int value;

    if (unlikely(sscanf(buf, "%u", &value) != 1))
        return -EINVAL;

    if (value == 1) {
        enable_systracker();
    } else if(value == 0) {
        disable_systracker();
    } else {
        return -EINVAL;
    }

    return count;
}

DRIVER_ATTR(tracker_run, 0644, tracker_run_show, tracker_run_store);

static ssize_t enable_wp_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%x\n", track_config.enable_wp);
}

static ssize_t enable_wp_store(struct device_driver * driver, const char *buf, size_t count)
{
    unsigned int value;

    if (unlikely(sscanf(buf, "%u", &value) != 1))
        return -EINVAL;

    if (value == 1) {
        enable_watch_point();
    } else if(value == 0) {
        disable_watch_point();
    } else {
        return -EINVAL;
    }

    return count;
}

DRIVER_ATTR(enable_wp, 0644, enable_wp_show, enable_wp_store);

static ssize_t set_wp_address_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%x\n", track_config.wp_phy_address);
}

static ssize_t set_wp_address_store(struct device_driver * driver, const char *buf, size_t count)
{
    unsigned int value;

    sscanf(buf, "0x%x", &value);
    printk("watch address:0x%x\n",value);
    set_watch_point_address(value);

    return count;
}

DRIVER_ATTR(set_wp_address, 0644, set_wp_address_show, set_wp_address_store);

static ssize_t tracker_entry_dump_show(struct device_driver *driver, char *buf)
{
    int ret = tracker_dump(buf);
    if (ret == -1)
        printk(KERN_CRIT "Dump error in %s, %d\n", __func__, __LINE__);

    ///*FOR test*/
    //test_systracker();

    return strlen(buf);;
}

static ssize_t tracker_entry_dump_store(struct device_driver * driver, const char *buf, size_t count)
{
    return count;
}

DRIVER_ATTR(tracker_entry_dump, 0664, tracker_entry_dump_show, tracker_entry_dump_store);

static irqreturn_t systracker_isr(void)
{
    unsigned int con;
    static char reg_buf[512];

    printk("Sys Tracker ISR\n");

    con = readl(IOMEM(BUS_DBG_CON));
    writel(con | BUS_DBG_CON_IRQ_CLR, IOMEM(BUS_DBG_CON));
    dsb();

    if (con & BUS_DBG_CON_IRQ_WP_STA) {
        printk("[TRACKER] Watch address: 0x%x was touched\n", track_config.wp_phy_address);
        if (mt_reg_dump(reg_buf) == 0) {
            printk("%s\n", reg_buf);
        }
    }

    return IRQ_HANDLED;
}

int enable_watch_point(void)
{
    /* systracker interrupt registration */
    if (!is_systracker_irq_registered) {
        if (request_irq(BUS_DBG_TRACKER_IRQ_BIT_ID, systracker_isr, IRQF_TRIGGER_LOW, "SYSTRACKER", NULL)) {
            printk(KERN_ERR "SYSTRACKER IRQ LINE NOT AVAILABLE!!\n");
        }
        else {
            is_systracker_irq_registered = 1;
        }
    }
    writel(track_config.wp_phy_address, IOMEM(BUS_DBG_WP));
    writel(0x0000000F, IOMEM(BUS_DBG_WP_MASK));
    track_config.enable_wp = 1;
    writel(readl(IOMEM(BUS_DBG_CON)) | BUS_DBG_CON_WP_EN, IOMEM(BUS_DBG_CON));
    dsb();

    return 0;
}

int disable_watch_point(void)
{
    track_config.enable_wp = 0;
    writel(readl(IOMEM(BUS_DBG_CON)) & ~BUS_DBG_CON_WP_EN, IOMEM(BUS_DBG_CON));
    dsb();

    return 0;
}

static int set_watch_point_address(unsigned int wp_phy_address)
{
   track_config.wp_phy_address = wp_phy_address;

   return 0;
}

void reset_systracker(void)
{
    writel(BUS_DBG_CON_DEFAULT_VAL, IOMEM(BUS_DBG_CON));
    writel(readl(IOMEM(BUS_DBG_CON)) | BUS_DBG_CON_SW_RST, IOMEM(BUS_DBG_CON));
    writel(readl(IOMEM(BUS_DBG_CON)) | BUS_DBG_CON_IRQ_CLR, IOMEM(BUS_DBG_CON));
    dsb();
}

int enable_systracker(void)
{
    unsigned int con;
    unsigned int timer_control_value;

    /* prescale = (266 * (10 ^ 6)) / 16 = 16625000/s = 16625/ms */
    timer_control_value = (BUS_DBG_BUS_MHZ * 1000 / 16) * track_config.timeout_ms;
    writel(timer_control_value, IOMEM(BUS_DBG_TIMER_CON));

    track_config.state = 1;
    con = BUS_DBG_CON_BUS_DBG_EN | BUS_DBG_CON_BUS_OT_EN;
    if (track_config.enable_timeout) {
        con |= BUS_DBG_CON_TIMEOUT_EN;
    }
    if (track_config.enable_slave_err) {
        con |= BUS_DBG_CON_SLV_ERR_EN;
    }
    if (track_config.enable_irq) {
        con |= BUS_DBG_CON_IRQ_EN;
    }
    writel(con, IOMEM(BUS_DBG_CON));
    dsb();

    return 0;
}

int disable_systracker(void)
{
    track_config.state = 0;
    writel(readl(IOMEM(BUS_DBG_CON)) & ~BUS_DBG_CON_BUS_DBG_EN, IOMEM(BUS_DBG_CON));
    dsb();

    return 0;
}

static int systracker_probe(struct platform_device *pdev)
{
    int ret;

#if 0
    /* FOR test */
    static char buf[4096];
    tracker_dump(buf);
#endif

    printk("systracker probe\n");
    is_systracker_device_registered = 1;

    memset(&track_config, sizeof(struct systracker_config_t), 0);
    /* To latch last PC when tracker timeout, we need to enable interrupt mode */
    track_config.enable_timeout = 1;
    track_config.enable_slave_err = 1;
    track_config.enable_irq = 1;
    track_config.timeout_ms = 100;

    reset_systracker();
    enable_systracker();

    /* Create sysfs entry */
    ret = driver_create_file(&systracker_driver.driver, &driver_attr_tracker_entry_dump);
    ret |= driver_create_file(&systracker_driver.driver, &driver_attr_tracker_run);
    ret |= driver_create_file(&systracker_driver.driver, &driver_attr_enable_wp);
    ret |= driver_create_file(&systracker_driver.driver, &driver_attr_set_wp_address);
    if (ret) {
        pr_err("Fail to create systracker_drv sysfs files");
    }

    /*FOR test*/
    //static char buf[4096];
    //tracker_dump(buf);

    return 0;
}

static int systracker_remove(struct platform_device *pdev)
{
    return 0;
}

static int systracker_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static int systracker_resume(struct platform_device *pdev)
{
    if (track_config.state) {
        enable_systracker();
    }

    if (track_config.enable_wp) {
        enable_watch_point();
    }

    return 0;
}

static int test_systracker(void)
{
#if 0
    *(volatile unsigned int *)0xF0007004 = 0x2008;
    *(volatile unsigned int *)0xF0007000 = 0x22000067;
    *(volatile unsigned int *)0xF0007008 = 0x1971;
    dsb();
    *(volatile unsigned int*)0xF0001220 |= 0x40;
    *(volatile unsigned int*)0xF3050000;
#endif
    *(volatile unsigned int*)0xF011A070 |= 0x0;
    dsb();
    *(volatile unsigned int*)0x10113000;
    while (1);
}

/*
 * driver initialization entry point
 */
static int __init systracker_init(void)
{
    int err;

    err = platform_driver_register(&systracker_driver);
    if (err) {
        return err;
    }
    else {
        printk("systracker init done\n");

        /*FOR test*/
        //test_systracker();
    }

    return 0;
}

/*
 * driver exit point
 */
static void __exit systracker_exit(void)
{
}

module_init(systracker_init);
module_exit(systracker_exit);
