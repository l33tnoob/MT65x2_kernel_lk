#ifndef _MT_POWER_GS_H
#define _MT_POWER_GS_H

#include <linux/module.h>
#include <linux/proc_fs.h>

/*****************
* extern variable 
******************/
extern struct proc_dir_entry *mt_power_gs_dir;

/*****************
* extern function 
******************/
extern void mt_power_gs_compare(char *scenario, \
                                unsigned int *mt6582_power_gs, unsigned int mt6582_power_gs_len, \
                                unsigned int *mt6323_power_gs, unsigned int mt6323_power_gs_len, \
                                unsigned int *mt6333_power_gs, unsigned int mt6333_power_gs_len);

extern void mt_power_gs_dump_dpidle(void);
extern void mt_power_gs_dump_idle(void);

#endif
