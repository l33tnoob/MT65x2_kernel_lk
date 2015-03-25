#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <mach/mt_cirq.h>
#include <mach/mt_reg_base.h>
#include <asm/system.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/types.h>
#include <mach/mt_sleep.h>
#include "mach/sync_write.h"
#include "mach/irqs.h"
#include <asm/mach/irq.h>
#include <asm/hardware/gic.h>

#define INT_POL_CTL0 (MCUSYS_CFGREG_BASE + 0x100)

int mt_cirq_test(void);
void mt_cirq_dump_reg(void);
#define CIRQ_DEBUG   0
#define CIRQ_LOG_LEVEL KERN_DEBUG
#define LDVT

#if(CIRQ_DEBUG == 1)
#ifdef CTP
#define dbgmsg dbg_print
#else
#define dbgmsg printk
#define print_func() do { \
    printk("in %s\n",__func__); \
} while(0)
#endif
#else
#define dbgmsg(...)
#define print_func() do { }while(0)
#endif


#define cirq_num_validate(x) do{\
    if (x > (MT_NR_CIRQ) || x < 0){\
        dbgmsg(CIRQ_LOG_LEVEL "Error in %s [CIRQ] wrong cirq, num %d is larger then 155 or less then 0\n",__func__,x); \
        return -1; \
    }\
}while(0) 


struct mt_cirq_driver{
    struct device_driver driver;
    const struct platform_device_id *id_table;
};

static struct mt_cirq_driver mt_cirq_drv = {
    .driver = {
        .name = "cirq",
        .bus = &platform_bus_type,
        .owner = THIS_MODULE,
    },
    .id_table= NULL,
};

/* 1: this cirq is masked
 * 0: this cirq is umasked
 * -1:cirq num is out of range
 * */
static unsigned int mt_cirq_get_mask(unsigned int cirq_num)
{
    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);
    volatile unsigned int val;
    print_func();
    cirq_num_validate(cirq_num);
    base = (cirq_num / 32) * 4 + CIRQ_MASK0;
    val = readl(IOMEM(base));
    if (val & bit)
        return 1;
    else
        return 0;
}

void mt_cirq_mask_all(void)
{
    int i;

    for(i = 0; i < 5; i++)
        mt65xx_reg_sync_writel(0xFFFFFFFF, CIRQ_MASK_SET0 + i * 4);
}

void mt_cirq_unmask_all(void)
{
    int i;

    for(i = 0; i < 5; i++)
        mt65xx_reg_sync_writel(0xFFFFFFFF, CIRQ_MASK_CLR0 + i * 4);
}

void mt_cirq_ack_all(void)
{
    int i;

    for(i = 0; i < 5; i++)
        mt65xx_reg_sync_writel(0xFFFFFFFF, CIRQ_ACK0 + i * 4);
}

/* 
 * 0: mask success
 * -1:cirq num is out of range
 * */
static int mt_cirq_mask(unsigned int cirq_num)
{
    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);
    print_func();

    cirq_num_validate(cirq_num);
    base = (cirq_num / 32) * 4 + CIRQ_ACK0;
    mt65xx_reg_sync_writel(bit, base);

    base = (cirq_num / 32) * 4 + CIRQ_MASK_SET0;
    mt65xx_reg_sync_writel(bit, base);

    dbgmsg(CIRQ_LOG_LEVEL "[CIRQ] mask addr:%x = %x, after set:0x%x\n", base, bit,readl(IOMEM((cirq_num / 32) * 4 + CIRQ_MASK0)));
    return 0;

}

/* 
 * 0: umask success
 * -1:cirq num is out of range
 * */
static int mt_cirq_unmask(unsigned int cirq_num)
{
    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);

    print_func();

    cirq_num_validate(cirq_num);
    base = (cirq_num / 32) * 4 + CIRQ_ACK0;
    mt65xx_reg_sync_writel(bit, base);
    dbgmsg(CIRQ_LOG_LEVEL "[CIRQ] ack :%x, bit: %x\n",  base, bit);

    base = (cirq_num / 32) * 4 + CIRQ_MASK_CLR0;
    mt65xx_reg_sync_writel(bit, base);

    dbgmsg(CIRQ_LOG_LEVEL "[CIRQ] unmask addr:%x = %x, after set:0x%x\n", base, bit,readl(IOMEM((cirq_num / 32) * 4 + CIRQ_MASK0)));
    return 0;

}

/* 
 * 0: set pol success
 * -1:cirq num is out of range
 * */
static int mt_cirq_set_pol(unsigned int cirq_num, unsigned int pol)
{
    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);

    print_func();

    cirq_num_validate(cirq_num);
    if (pol == MT_CIRQ_POL_NEG) {
        base = (cirq_num / 32) * 4 + CIRQ_POL_CLR0;
    } else if (pol == MT_CIRQ_POL_POS){
        base = (cirq_num / 32) * 4 + CIRQ_POL_SET0;
    } else {
        dbgmsg(KERN_CRIT"%s invalid polarity value\n", __func__);
        return -1 ;
    }
    mt65xx_reg_sync_writel(bit, base);
    dbgmsg(CIRQ_LOG_LEVEL "[CIRQ] set pol:%d :%x, bit: %x, after set:0x%x\n",pol, base, bit,readl(IOMEM((cirq_num / 32) * 4 + CIRQ_POL0)));
    return 0;

}

/* 
 * 0: set sens success
 * -1:cirq num is out of range
 * */
static int  mt_cirq_set_sens(unsigned int cirq_num, unsigned int sens)
{
    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);

    print_func();
    cirq_num_validate(cirq_num);
    if (sens == MT_EDGE_SENSITIVE) {
        base = (cirq_num / 32) * 4 + CIRQ_SENS_CLR0;
    } else if (sens == MT_LEVEL_SENSITIVE) {
        base = (cirq_num / 32) * 4 + CIRQ_SENS_SET0;
    } else {
        dbgmsg(KERN_CRIT"%s invalid sensitivity value\n", __func__);
        return -1;
    }
    mt65xx_reg_sync_writel(bit, base);
    dbgmsg(CIRQ_LOG_LEVEL "[CIRQ] %s,sens:%d :%x, bit: %x, after set:0x%x\n", __func__,sens, base, bit,readl(IOMEM((cirq_num / 32) * 4 + CIRQ_SENS0)));
    return 0;
}

/* 1: this cirq is MT_LEVEL_SENSITIVE
 * 0: this cirq is MT_EDGE_SENSITIVE
 * -1:cirq num is out of range
 * */
static unsigned int mt_cirq_get_sens(unsigned int cirq_num)
{

    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);
    volatile unsigned int val;
    print_func();
    cirq_num_validate(cirq_num);
    base = (cirq_num / 32) * 4 + CIRQ_SENS0;
    val = readl(IOMEM(base));
    if (val & bit)
    {
        return MT_LEVEL_SENSITIVE;
    }
    else
    {
        return MT_EDGE_SENSITIVE;
    }

}

/* 1: this cirq is MT_CIRQ_POL_POS
 * 0: this cirq is MT_CIRQ_POL_NEG
 * -1:cirq num is out of range
 * */
static unsigned int mt_cirq_get_pol(unsigned int cirq_num)
{
    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);
    volatile unsigned int val;
    print_func();
    cirq_num_validate(cirq_num);
    base = (cirq_num / 32) * 4 + CIRQ_POL0;
    val = readl(IOMEM(base));
    if (val & bit)
    {
        return MT_CIRQ_POL_POS;
    }
    else
    {
        return MT_CIRQ_POL_NEG;
    }
}
#if 0
/* 
 * 0: ack success
 * -1:cirq num is out of range
 * */
static unsigned int mt_cirq_ack(unsigned int cirq_num)
{
    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);
    print_func();

    cirq_num_validate(cirq_num);
    base = (cirq_num / 32) * 4 + CIRQ_ACK0;
    mt65xx_reg_sync_writel(bit, base);


    dbgmsg(CIRQ_LOG_LEVEL "[CIRQ] ack addr:%x = %x\n", base, bit);

    return 0;
}
#endif
#if 0
/* 
 * 0: get sta success
 * -1:cirq num is out of range
 * */
unsigned int mt_cirq_read_status(unsigned int cirq_num)
{
    unsigned int sta;
    unsigned int base;
    unsigned int bit = 1 << (cirq_num % 32);
    volatile unsigned int val;
    print_func();
    cirq_num_validate(cirq_num);

    base = (cirq_num / 32) * 4 + CIRQ_STA0;
    val = readl(IOMEM(base));
    sta = (val & bit);
    return sta;

}
#endif
void mt_cirq_enable(void){
    unsigned int base=CIRQ_CON;
    volatile unsigned int val;
    val = readl(IOMEM(CIRQ_CON));
    val |= (0x3); //enable edge only mode
    mt65xx_reg_sync_writel(val,base);
}
void mt_cirq_disable(void){
    unsigned int base=CIRQ_CON;
    volatile unsigned int val;
    val = readl(IOMEM(CIRQ_CON));
    val &= (~0x1);
    mt65xx_reg_sync_writel(val,base);
}
void mt_cirq_flush(void){

    unsigned int irq;
    volatile unsigned int val;
    print_func();
    
    /*make edge interrupt shows in the STA*/
    mt_cirq_unmask_all();
    for (irq = 64; irq < (NR_MT_IRQ_LINE); irq+=32)
    {
        val = readl(IOMEM(((irq-64) / 32) * 4 + CIRQ_STA0));
        //printk("irq:%d,pending bit:%x\n",irq,val);

        mt65xx_reg_sync_writel(val,(GIC_DIST_BASE + GIC_DIST_PENDING_SET + irq / 32 * 4));
        dsb();
        //printk("irq:%d,pending bit:%x,%x\n",irq,val,readl(GIC_DIST_BASE + GIC_DIST_PENDING_SET + irq / 32 * 4));
    }
    mt_cirq_mask_all();
    dsb();
}
static void mt_cirq_clone_pol(void)
{
    unsigned int irq,irq_offset;
    volatile unsigned int value;
    volatile unsigned int value_cirq;
    int ix;
    print_func();
    for (irq = 64; irq < (NR_MT_IRQ_LINE); irq+=32)
    {
        value = readl(IOMEM(INT_POL_CTL0 + ((irq-GIC_PRIVATE_SIGNALS) / 32 * 4)));
        irq_offset = (irq-64) / 32;
        dbgmsg(CIRQ_LOG_LEVEL "irq:%d,gic_pol_address:0x%8x,gic_pol_value:0x%08x\n",irq,INT_POL_CTL0 + ((irq-GIC_PRIVATE_SIGNALS) / 32 * 4),value);
        for (ix = 0; ix < 32; ix++)
        {
            if (value & (0x1)) //high trigger 
                mt_cirq_set_pol(irq+ix-64,MT_CIRQ_POL_NEG);
            else//low trigger
                mt_cirq_set_pol(irq+ix-64,MT_CIRQ_POL_POS);

            value >>= 1;
        }
        value_cirq = readl(IOMEM(CIRQ_POL0 + irq_offset*4));
        dbgmsg(CIRQ_LOG_LEVEL "irq:%d,cirq_value:0x%08x\n",irq,value_cirq);
    }
}

static void mt_cirq_clone_sens(void)
{
    unsigned int irq,irq_offset;
    volatile unsigned int value;
    volatile unsigned int value_cirq;
    int ix;
    print_func();
    for (irq = 64; irq < (NR_MT_IRQ_LINE); irq+=16)
    {   
        value = readl(IOMEM(GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4));
        dbgmsg(CIRQ_LOG_LEVEL "irq:%d,sens:%08x,value:0x%08x\n",irq,GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4,value);
        irq_offset = (irq-64) / 32;
        for (ix = 0; ix < 16; ix++)
        {
            if (value & (0x2)) //edge trigger 
                mt_cirq_set_sens(irq-64+ix,MT_EDGE_SENSITIVE);
            else//level trigger
                mt_cirq_set_sens(irq-64+ix,MT_LEVEL_SENSITIVE);
            value >>= 2;
        }
        value_cirq = readl(IOMEM(CIRQ_SENS0 + irq_offset*4));
        dbgmsg(CIRQ_LOG_LEVEL "irq:%d,cirq_value:0x%08x\n",irq,value_cirq);
    }
}

static void mt_cirq_clone_mask(void)
{
    unsigned int irq,irq_offset;
    volatile unsigned int value;
    volatile unsigned int value_cirq;
    int ix;
    print_func();
    for (irq = 64; irq < (NR_MT_IRQ_LINE); irq+=32)
    {
        value = readl(IOMEM(GIC_DIST_BASE + GIC_DIST_ENABLE_SET + irq / 32 * 4));
        dbgmsg(CIRQ_LOG_LEVEL "irq:%d,mask:%08x,value:0x%08x\n",irq,(GIC_DIST_BASE + GIC_DIST_ENABLE_SET + irq / 32 * 4) ,value);
        irq_offset = (irq-64) / 32;
        for (ix = 0; ix < 32; ix++)
        {
            if (value & (0x1)) //enable  
                mt_cirq_unmask(irq+ix-64);
            else//disable
                mt_cirq_mask(irq+ix-64);

            value >>= 1;
        }
        value_cirq = readl(IOMEM(CIRQ_MASK0 + irq_offset*4));
        dbgmsg(CIRQ_LOG_LEVEL "irq:%d,cirq_value:0x%08x\n",irq,value_cirq);
    }
}
void mt_cirq_clone_gic(void)
{    
    mt_cirq_clone_pol();
    mt_cirq_clone_sens();
    mt_cirq_clone_mask();
}


#if defined(LDVT)
/*
 * cirq_dvt_show: To show usage.
 */
static ssize_t cirq_dvt_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "CIRQ dvt test\n");
}

/*
 * mci_dvt_store: To select mci test case.
 */
static ssize_t cirq_dvt_store(struct device_driver *driver, const char *buf,
			      size_t count)
{
    char *p = (char *)buf;
    unsigned int num;

    num = simple_strtoul(p, &p, 10);
    switch(num){
        case 1:
            mt_cirq_clone_gic();
            mt_cirq_dump_reg();
            break;
        case 2:
            mt_cirq_test();
            break;
        case 3:
            mt_cirq_disable();
            break;
        default:
            break;
    }

    return count;
}
DRIVER_ATTR(cirq_dvt, 0664, cirq_dvt_show, cirq_dvt_store);
#endif //!LDVT

/*
 * CIRQ interrupt service routine.
 */
static irqreturn_t cirq_irq_handler(int irq, void *dev_id)
{
    printk("CIRQ_Handler\n");
    mt_cirq_ack_all();
    return IRQ_HANDLED;
}

/*
 * always return 0
 * */
static int __init mt_cirq_init(void){
        int ret;
        printk("CIRQ init...\n");
        if (request_irq(MT_CIRQ_IRQ_ID, cirq_irq_handler, IRQF_TRIGGER_LOW, "CIRQ",  NULL)) {
            printk(KERN_ERR"CIRQ IRQ LINE NOT AVAILABLE!!\n");
        }else
        {
            printk("CIRQ handler init success.");
        }
        ret = driver_register(&mt_cirq_drv.driver);
#ifdef LDVT
	ret = driver_create_file(&mt_cirq_drv.driver, &driver_attr_cirq_dvt);
#endif
        if (ret == 0)
        printk("CIRQ init done...\n");
	return 0;

}

#if defined(LDVT)
#define __CHECK_IRQ_TYPE
#if defined(__CHECK_IRQ_TYPE)
#define X_DEFINE_IRQ(__name, __num, __polarity, __sensitivity) \
        { .num = __num, .polarity = __polarity, .sensitivity = __sensitivity, },
#define L 0
#define H 1
#define EDGE MT_EDGE_SENSITIVE
#define LEVEL MT_LEVEL_SENSITIVE
struct __check_irq_type
{
    int num;
    int polarity;
    int sensitivity;
};
struct __check_irq_type __check_irq_type[] =
{
#include <mach/x_define_irq.h>
    { .num = -1, },
};
#undef X_DEFINE_IRQ
#undef L
#undef H
#undef EDGE
#undef LEVEL
#endif

void mt_cirq_dump_reg(void)
{
    int cirq_num;
    int pol,sens,mask;
    int irq_iter;
    
    printk("IRQ:\tPOL\tSENS\tMASK\n");
    for (cirq_num = 0; cirq_num < MT_NR_CIRQ; cirq_num++)
    {
        pol = mt_cirq_get_pol(cirq_num);
        sens = mt_cirq_get_sens(cirq_num);
        mask = mt_cirq_get_mask(cirq_num);
#if defined(__CHECK_IRQ_TYPE)
        //only check unmask irq
        if (0 == mask){
            irq_iter = 0;
            while (__check_irq_type[irq_iter].num >= 0) {
                if (__check_irq_type[irq_iter].num == (cirq_num+64)){
                    if(__check_irq_type[irq_iter].sensitivity != sens){
                        printk("[CIRQ] Error sens in irq:%d\n",cirq_num+64); 
                    }
                    if(__check_irq_type[irq_iter].polarity != (pol)){
                        printk("[CIRQ] Error polarity in irq:%d\n",cirq_num+64); 
                    }
                    break;
                }  
                irq_iter++;
            }
        }
#endif
        printk("IRQ:%d\t%d\t%d\t%d\n",cirq_num+64,pol,sens,mask);

    }
}
int mt_cirq_test(void)
{
    int cirq_num = 100; 
    mt_cirq_enable();

    /*test polarity*/
    mt_cirq_set_pol(cirq_num,MT_CIRQ_POL_NEG);
    if ( MT_CIRQ_POL_NEG != mt_cirq_get_pol(cirq_num))
        printk("mt_cirq_set_pol test failed!!\n");
    mt_cirq_set_pol(cirq_num,MT_CIRQ_POL_POS);
    if ( MT_CIRQ_POL_POS != mt_cirq_get_pol(cirq_num))
        printk("mt_cirq_set_pol test failed!!\n");

    /*test sensitivity*/
    mt_cirq_set_sens(cirq_num,MT_EDGE_SENSITIVE);
    if ( MT_EDGE_SENSITIVE != mt_cirq_get_sens(cirq_num))
        printk("mt_cirq_set_sens test failed!!\n");
    mt_cirq_set_sens(cirq_num,MT_LEVEL_SENSITIVE);
    if ( MT_LEVEL_SENSITIVE != mt_cirq_get_sens(cirq_num))
        printk("mt_cirq_set_sens test failed!!\n");

    /*test mask*/
    mt_cirq_mask(cirq_num);
    if ( 1 != mt_cirq_get_mask(cirq_num))
        printk("mt_cirq_mask test failed!!\n");
    mt_cirq_unmask(cirq_num);
    mt_cirq_set_sens(cirq_num,MT_LEVEL_SENSITIVE);
    if ( 0 != mt_cirq_get_mask(cirq_num))
        printk("mt_cirq_unmask test failed!!\n");


    mt_cirq_clone_gic();
    mt_cirq_dump_reg();

    return 0;
}
#endif //!LDVT
arch_initcall(mt_cirq_init);
EXPORT_SYMBOL(mt_cirq_enable);
EXPORT_SYMBOL(mt_cirq_disable);
EXPORT_SYMBOL(mt_cirq_clone_gic);
EXPORT_SYMBOL(mt_cirq_flush);

