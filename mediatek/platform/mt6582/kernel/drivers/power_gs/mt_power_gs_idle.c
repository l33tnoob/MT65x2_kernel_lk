#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_power_gs.h>

extern unsigned int *mt6582_power_gs_idle;
extern unsigned int mt6582_power_gs_idle_len;

extern unsigned int *mt6323_power_gs_idle;
extern unsigned int mt6323_power_gs_idle_len;

unsigned int mt6333_power_gs_idle[] = {
    // Buck
    0x009F, 0x0080, 0x0000,
    0x00A0, 0x0007, 0x0003,
    0x006D, 0x007f, 0x0010,
};

static bool mt_idle_chk_golden = 0;

void mt_power_gs_dump_idle(void)
{
    if (TRUE == mt_idle_chk_golden)
    {
        mt_power_gs_compare("Idle",                                         \
                            mt6582_power_gs_idle, mt6582_power_gs_idle_len, \
                            mt6323_power_gs_idle, mt6323_power_gs_idle_len, \
                            mt6333_power_gs_idle, sizeof(mt6333_power_gs_idle));
    }
}

static int dump_idle_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "mt_power_gs : idle\n");

    mt_power_gs_dump_idle();

    len = p - buf;
    return len;
}

static void __exit mt_power_gs_idle_exit(void)
{
    //return 0;
}

static int __init mt_power_gs_idle_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;

    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("dump_idle", S_IRUGO | S_IWUSR | S_IWGRP, mt_power_gs_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = dump_idle_read;
        }
    }

    return 0;
}

module_param(mt_idle_chk_golden, bool, 0644);
module_init(mt_power_gs_idle_init);
module_exit(mt_power_gs_idle_exit);

MODULE_DESCRIPTION("MT6582 Power Golden Setting - idle");
