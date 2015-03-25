#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_power_gs.h>

extern unsigned int *mt6582_power_gs_video_record;
extern unsigned int mt6582_power_gs_video_record_len;

extern unsigned int *mt6323_power_gs_video_record;
extern unsigned int mt6323_power_gs_video_record_len;

unsigned int mt6333_power_gs_video_record[] = {
    // Buck
    0x009F, 0x0080, 0x0000,
    0x00A0, 0x0007, 0x0000,
    0x006D, 0x007f, 0x0010,
};

void mt_power_gs_dump_video_record(void)
{
    mt_power_gs_compare("Video Record",                                                 \
                        mt6582_power_gs_video_record, mt6582_power_gs_video_record_len, \
                        mt6323_power_gs_video_record, mt6323_power_gs_video_record_len, \
                        mt6333_power_gs_video_record, sizeof(mt6333_power_gs_video_record));
}

static int dump_video_record_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "mt_power_gs : video_record\n");

    mt_power_gs_dump_video_record();

    len = p - buf;
    return len;
}

static void __exit mt_power_gs_video_record_exit(void)
{
    //return 0;
}

static int __init mt_power_gs_video_record_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;

    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("dump_video_record", S_IRUGO | S_IWUSR | S_IWGRP, mt_power_gs_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = dump_video_record_read;
        }
    }

    return 0;
}

module_init(mt_power_gs_video_record_init);
module_exit(mt_power_gs_video_record_exit);

MODULE_DESCRIPTION("MT6582 Power Golden Setting - Video Record");
