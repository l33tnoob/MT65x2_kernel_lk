/**
* @file    du_timer_observer.c
* @brief   debug utility - timer observer
*
*/

/*=============================================================*/
// Include files
/*=============================================================*/
// system includes
#include <linux/kernel.h>   //printk
#include <linux/errno.h>    //EINVAL
#include <linux/string.h>   //strlen, strcpy
#include <linux/jiffies.h>  //jiffies
#include <linux/module.h>   //MODULE_DESCRIPTION
#include <linux/proc_fs.h>  //proc_mkdir
#include <asm/uaccess.h>    //copy_from_user

// project includes

// local includes

// forward references


/*=============================================================*/
// Macro definition
/*=============================================================*/

//
// CONFIG
//
#define TIMER_OBSERVER_CTXTS_NUM            32
#define TIMER_OBSERVER_CTXT_NAME_LENGTH     32

//
// LOG
//
#define to_emerg(fmt, args...)              printk(KERN_EMERG "[TO] " fmt, ##args)
#define to_alert(fmt, args...)              printk(KERN_ALERT "[TO] " fmt, ##args)
#define to_crit(fmt, args...)               printk(KERN_CRIT "[TO] " fmt, ##args)
#define to_error(fmt, args...)              printk(KERN_ERR "[TO] " fmt, ##args)
#define to_warning(fmt, args...)            printk(KERN_WARNING "[TO] " fmt, ##args)
#define to_notice(fmt, args...)             printk(KERN_NOTICE "[TO] " fmt, ##args)
#define to_info(fmt, args...)               printk(KERN_INFO "[TO] " fmt, ##args)
#define to_debug(fmt, args...)              printk(KERN_DEBUG "[TO] " fmt, ##args)

//
// REG ACCESS
//
#define to_read(addr)                       (*(volatile u32 *)(addr))
#define to_write(addr, val)                 mt65xx_reg_sync_writel(val, addr)


/*=============================================================*/
// Local type definition
/*=============================================================*/
//extern unsigned long volatile __jiffy_data jiffies;
typedef struct _timer_observer_ctxt
{
    char name[TIMER_OBSERVER_CTXT_NAME_LENGTH];
    unsigned long interval; //ms
    unsigned long last_jiffies;
    void (*callback)(void);
} timer_observer_ctxt;

/*=============================================================*/
// Local variable definition
/*=============================================================*/
static timer_observer_ctxt timer_observer_ctxts[TIMER_OBSERVER_CTXTS_NUM]= {0};

/*=============================================================*/
// Local function definition
/*=============================================================*/
/**************************************************************
* timer observer control interface for procfs interval
***************************************************************/
static int timer_observer_read_interval(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    char *p = buf;
    int len = 0;

    p += sprintf(p, "unsupport\n");

    *start = buf + off;

    len = p - buf;
    if (len > off)
        len -= off;
    else
        len = 0;

    *eof = 1;
    return len < count ? len  : count;
}

static int timer_observer_write_interval(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int len = 0, index = 0, interval = 0;
    char desc[32];

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
    if (copy_from_user(desc, buffer, len))
    {
        return 0;
    }
    desc[len] = '\0';

    if (sscanf(desc, "%d %d", &index, &interval) == 2)
    {
        if ((index >= TIMER_OBSERVER_CTXTS_NUM) || (index < 0))
            return -EINVAL;
        
        timer_observer_ctxts[index].interval = interval;
        return count;
    }
    else
    {
        to_info("timer_observer_write_interval, bad argument\n");
    }

    return -EINVAL;
}

/**************************************************************
* timer observer control interface for procfs debug
***************************************************************/
static int timer_observer_read_debug(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    char *p = buf;
    int len = 0;
    int i;

    for (i = 0; i < TIMER_OBSERVER_CTXTS_NUM; ++i)
    {
        //if (strlen(timer_observer_ctxts[i].name) != 0)
            p += sprintf(p, "%02d %08d %s %lu\n", i, timer_observer_ctxts[i].interval, timer_observer_ctxts[i].name, timer_observer_ctxts[i].last_jiffies);
    }

    *start = buf + off;

    len = p - buf;
    if (len > off)
        len -= off;
    else
        len = 0;

    *eof = 1;
    return len < count ? len  : count;
}

static int timer_observer_write_debug(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int len = 0, index = 0, debug = 0;
    char desc[32];

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
    if (copy_from_user(desc, buffer, len))
    {
        return 0;
    }
    desc[len] = '\0';

    return count;
    /*
    if (sscanf(desc, "%d %d", &index, &debug) == 2)
    {
        if ((index >= TIMER_OBSERVER_CTXTS_NUM) || (index < 0))
            return -EINVAL;
        
        timer_observer_ctxts[index].debug = debug;
        return count;
    }
    else
    {
        to_info("timer_observer_write_debug, bad argument\n");
    }

    return -EINVAL;
    */
}

/*******************************
* kernel module init function
********************************/
static int __init timer_observer_init(void)
{
    struct proc_dir_entry *entry = NULL;
    struct proc_dir_entry *timer_observer_dir = NULL;
    int r = 0;

    to_info("timer_observer_init");

    timer_observer_dir = proc_mkdir("timer_observer", NULL);
    if (!timer_observer_dir)
    {
        to_info("mkdir /proc/timer_observer failed");
    }
    else
    {
        entry = create_proc_entry("interval", S_IRUGO | S_IWUSR, timer_observer_dir);
        if (entry)
        {
            entry->read_proc = timer_observer_read_interval;
            entry->write_proc = timer_observer_write_interval;
        }

        entry = create_proc_entry("debug", S_IRUGO | S_IWUSR, timer_observer_dir);
        if (entry)
        {
            entry->read_proc = timer_observer_read_debug;
            entry->write_proc = timer_observer_write_debug;
        }
    }

    return r;
}
module_init(timer_observer_init);

/*******************************
* kernel module exit function
********************************/
static void __exit timer_observer_exit(void)
{
    to_info("timer_observer_exit");
}
module_exit(timer_observer_exit);


/*=============================================================*/
// Global variable definition
/*=============================================================*/

/*=============================================================*/
// Gobal function definition
/*=============================================================*/
int to_init(int index, char * name, unsigned long interval, void (*callback)(void))
{
    if ((index >= TIMER_OBSERVER_CTXTS_NUM) || (index < 0))
        return -EINVAL;

    if ((name == NULL) || (strlen(name) >= TIMER_OBSERVER_CTXT_NAME_LENGTH))
        return -EINVAL;

    strcpy(timer_observer_ctxts[index].name, name);
    timer_observer_ctxts[index].interval = interval;
    timer_observer_ctxts[index].last_jiffies = jiffies;
    timer_observer_ctxts[index].callback = callback;
    
    //to_info("%02d %08d %s %lu\n", index, timer_observer_ctxts[index].interval, timer_observer_ctxts[index].name, timer_observer_ctxts[index].last_jiffies);
    return 0;
}

/*
TODO: is to_init_all() necessary? 
int to_init_all(char * name, unsigned long interval)
{
    int i;

    if ((name == NULL) || (strlen(name) >= TIMER_OBSERVER_CTXT_NAME_LENGTH))
        return -EINVAL;

    for (i = 0; i < TIMER_OBSERVER_CTXTS_NUM; ++i)
    {
        strcpy(timer_observer_ctxts[i].name, name);
        timer_observer_ctxts[i].interval = interval;
        timer_observer_ctxts[i].last_jiffies = 0;
    }
    
    return 0;
}
*/

int to_update_jiffies(int index)
{
    unsigned long last_jiffies = timer_observer_ctxts[index].last_jiffies;
    unsigned long diff_ms;

    if ((index >= TIMER_OBSERVER_CTXTS_NUM) || (index < 0))
        return -EINVAL;

    timer_observer_ctxts[index].last_jiffies = jiffies;
    //TODO: use time_after series api to prevent from jiffies overflow in 300s when 1st system bring up?
    diff_ms = jiffies_to_msecs(timer_observer_ctxts[index].last_jiffies - last_jiffies);
    if (diff_ms > timer_observer_ctxts[index].interval)
    {
        //to_emerg("%02d, %s, %lu ms, +%u ms\n", index, timer_observer_ctxts[index].name, timer_observer_ctxts[index].interval, diff_ms);
        to_info("%02d, %s, %lu ms, +%u ms\n", index, timer_observer_ctxts[index].name, timer_observer_ctxts[index].interval, diff_ms);
        if (timer_observer_ctxts[index].callback)
            timer_observer_ctxts[index].callback();
    }

    return 0;
}


MODULE_DESCRIPTION("MediaTek Debug Utility - Timer Observer v0.1");
MODULE_LICENSE("GPL");

