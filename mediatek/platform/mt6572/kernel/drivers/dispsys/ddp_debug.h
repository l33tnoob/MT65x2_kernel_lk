#ifndef __DDP_DEBUG_H__
#define __DDP_DEBUG_H__

#include <linux/kernel.h>
#include <linux/mmprofile.h>

#include <mach/mt_typedefs.h>


extern struct DDP_MMP_Events_t
{
    MMP_Event DDP;
    MMP_Event MutexParent;
    MMP_Event Mutex[6];
    MMP_Event BackupReg;
    MMP_Event DDP_IRQ;
    MMP_Event SCL_IRQ;
    MMP_Event ROT_IRQ;
    MMP_Event OVL_IRQ;
    MMP_Event WDMA0_IRQ;
    MMP_Event WDMA1_IRQ;
    MMP_Event RDMA0_IRQ;
    MMP_Event RDMA1_IRQ;
    MMP_Event COLOR_IRQ;
    MMP_Event BLS_IRQ;
    MMP_Event TDSHP_IRQ;
    MMP_Event CMDQ_IRQ;
    MMP_Event Mutex_IRQ;
    MMP_Event WAIT_INTR;
    MMP_Event Debug;
} DDP_MMP_Events;
//#define DISP_MSG(...)   xlog_printk(ANDROID_LOG_DEBUG, "xlog/disp", __VA_ARGS__)
//#define DISP_DBG(...)   xlog_printk(ANDROID_LOG_WARN,  "xlog/disp", __VA_ARGS__)
//#define DISP_ERR(...)   xlog_printk(ANDROID_LOG_ERROR, "xlog/disp", __VA_ARGS__)


// global debug macro for DDP
#define DDP_DRV_DBG_ON
#define LCD_DRV_DBG_ON


extern unsigned int ddp_drv_dbg_log;
extern unsigned int ddp_drv_irq_log;
extern unsigned int ddp_drv_info_log;
extern unsigned int ddp_drv_err_log;

extern size_t dbi_drv_dbg_log;
extern size_t dbi_drv_dbg_func_log;
extern size_t dsi_drv_dbg_log;
extern size_t dsi_drv_dbg_func_log;


#define DDP_DRV_IRQ(string, args...) \
   do { \
      if (ddp_drv_irq_log) printk("[DDP]"string,##args);    \
   }while (0)

#define DDP_DRV_DBG(string, args...) \
   do { \
      if (ddp_drv_dbg_log) printk("[DDP]"string,##args);    \
   }while (0)

#define DDP_DRV_INFO(string, args...) \
   do { \
      if (ddp_drv_info_log) printk("[DDP]"string,##args);    \
   }while (0)

#define DDP_DRV_ERR(string, args...) \
   do { \
      if (ddp_drv_err_log) printk("[DDP] Error:"string,##args);    \
   }while (0)

#define DBI_DRV_WRAN(fmt, arg...) \
   do { \
      if (dbi_drv_dbg_log) DISP_LOG_PRINT(ANDROID_LOG_WARN, "DBI", fmt, ##arg);    \
   }while (0)

#define DBI_DRV_INFO(fmt, arg...) \
   do { \
      if (dbi_drv_dbg_log) DISP_LOG_PRINT(ANDROID_LOG_INFO, "DBI", fmt, ##arg);    \
   }while (0)

#define DBI_DRV_FUNC(fmt, arg...) \
   do { \
      if(dbi_drv_dbg_func_log) DISP_LOG_PRINT(ANDROID_LOG_INFO, "DBI", "[Func]%s\n", __func__);  \
   }while (0)

#define DSI_DRV_WRAN(fmt, arg...) \
   do { \
      if (dsi_drv_dbg_log) DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", fmt, ##arg);    \
   }while (0)

#define DSI_DRV_INFO(fmt, arg...) \
   do { \
      if (dsi_drv_dbg_log) DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", fmt, ##arg);    \
   }while (0)

#define DSI_DRV_FUNC(fmt, arg...) \
   do { \
      if(dsi_drv_dbg_func_log) DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[Func]%s\n", __func__);  \
   }while (0)


extern unsigned int dbg_log;
extern unsigned int irq_log;
       
#define DISP_IRQ(string, args...) if(irq_log) printk("[DDP]"string,##args)  // default off
#define DISP_DBG(string, args...) if(dbg_log) printk("[DDP]"string,##args)  // default off, use "adb shell "echo dbg_log:1 > sys/kernel/debug/dispsys" to enable
#define DISP_MSG(string, args...) printk("[DDP]"string,##args)  // default on, important msg, not err
#define DISP_ERR(string, args...) printk("[DDP]error:"string,##args)  //default on, err msg


void ddp_debug_init(void);
void ddp_debug_exit(void);
int ddp_mem_test(void);
int ddp_mem_test2(void);


#endif /* __DDP_DEBUG_H__ */
