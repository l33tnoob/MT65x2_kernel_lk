#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_power_gs.h>

extern unsigned int *mt6582_power_gs_dpidle;
extern unsigned int mt6582_power_gs_dpidle_len;

extern unsigned int *mt6323_power_gs_dpidle;
extern unsigned int mt6323_power_gs_dpidle_len;

unsigned int mt6333_power_gs_dpidle[] = {
    // Buck
    0x009F, 0x0080, 0x0000,
    0x00A0, 0x0007, 0x0004,
    0x006D, 0x007f, 0x0010,
};

static bool mt_dpidle_chk_golden = 0;


void mt_power_gs_dump_dpidle(void)
{
    if (TRUE == mt_dpidle_chk_golden)
    {
        mt_power_gs_compare("DPIdle",                                           \
                            mt6582_power_gs_dpidle, mt6582_power_gs_dpidle_len, \
                            mt6323_power_gs_dpidle, mt6323_power_gs_dpidle_len, \
                            mt6333_power_gs_dpidle, sizeof(mt6333_power_gs_dpidle));
    }
}

static int dump_dpidle_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "mt_power_gs : dpidle\n");

    mt_power_gs_dump_dpidle();

    len = p - buf;
    return len;
}

static void __exit mt_power_gs_dpidle_exit(void)
{
    //return 0;
}
static int __init mt_power_gs_dpidle_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;

    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("dump_dpidle", S_IRUGO | S_IWUSR | S_IWGRP, mt_power_gs_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = dump_dpidle_read;
        }
    }

    return 0;
}


module_param(mt_dpidle_chk_golden, bool, 0644);
module_init(mt_power_gs_dpidle_init);
module_exit(mt_power_gs_dpidle_exit);

MODULE_DESCRIPTION("MT6582 Power Golden Setting - dpidle");
