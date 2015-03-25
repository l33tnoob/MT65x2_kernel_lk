#include "mach/mt_reg_base.h"
#include "mach/mt_reg_dump.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <mach/sync_write.h>
#include <mach/dbg_dump.h>
#include <linux/kallsyms.h>
#include <linux/init.h>

static struct reg_dump_driver_data reg_dump_driver_data =
{
    .mcu_regs = (MCUSYS_CFGREG_BASE + 0x300),
};

static struct platform_device reg_dump_device = 
{    
    .name = "dbg_reg_dump",
    .dev = 
    {
        .platform_data = &(reg_dump_driver_data),
    },
};

/*
 * mt_reg_dump_init: initialize driver.
 * Always return 0.
 */

static int __init mt_reg_dump_init(void)
{
	int err;
	
  err = platform_device_register(&(reg_dump_device));
  if (err) {
      pr_err("Fail to register reg_dump_device");
      return err;
  }  
	
  return 0;
}

int reg_dump_platform(char *buf)
{
  /* Get core numbers */
        int ret = -1, cnt = num_possible_cpus();
        char *ptr = buf;
        unsigned int pc_value;
        unsigned int fp_value;
        unsigned int sp_value;
        unsigned long size = 0;
        unsigned long offset = 0;
        char str[KSYM_SYMBOL_LEN];
        int i;
        
  if(cnt < 0)
    return ret;
  
  /* Get PC, FP, SP and save to buf */
  for (i = 0; i < cnt; i++) {
      pc_value = readl(IOMEM(reg_dump_driver_data.mcu_regs + (i << 4)));
      fp_value = readl(IOMEM((reg_dump_driver_data.mcu_regs+0x4) + (i << 4)));
      sp_value = readl(IOMEM((reg_dump_driver_data.mcu_regs+0x8) + (i << 4)));
      kallsyms_lookup((unsigned long)pc_value, &size, &offset, NULL, str);          
      ptr += sprintf(ptr, "CORE_%d PC = 0x%x(%s + 0x%lx), FP = 0x%x, SP = 0x%x\n", i, pc_value, str, offset, fp_value, sp_value);
      //printk("CORE_%d PC = 0x%x(%s), FP = 0x%x, SP = 0x%x\n", i, pc_value, str, fp_value, sp_value);
  }
  return 0;     
                
}

arch_initcall(mt_reg_dump_init);
