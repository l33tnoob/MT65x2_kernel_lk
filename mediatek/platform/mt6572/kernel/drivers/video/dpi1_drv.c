#define ENABLE_DPI1_INTERRUPT        0
#define ENABLE_DPI1_REFRESH_RATE_LOG 0

#if ENABLE_DPI1_REFRESH_RATE_LOG && !ENABLE_DPI1_INTERRUPT
#error "ENABLE_DPI1_REFRESH_RATE_LOG should be also ENABLE_DPI1_INTERRUPT"
#endif

#if defined(MTK_HDMI_SUPPORT) && !ENABLE_DPI1_INTERRUPT
//#error "enable MTK_HDMI_SUPPORT should be also ENABLE_DPI1_INTERRUPT"
#endif

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/hrtimer.h>
#include <asm/io.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include "disp_drv_log.h"
#include "disp_drv_platform.h"

#include "dpi_reg.h"
#include "dsi_reg.h"
#include "dpi1_drv.h"
#include "lcd_drv.h"
#include <mach/mt_clkmgr.h>


#if ENABLE_DPI1_INTERRUPT
//#include <linux/interrupt.h>
//#include <linux/wait.h>

#include <mach/irqs.h>
#include "mtkfb.h"
#endif
static wait_queue_head_t _vsync_wait_queue_dpi;
static bool dpi_vsync = false;
static bool wait_dpi_vsync = false;
static struct hrtimer hrtimer_vsync_dpi;
#include <linux/module.h>

#include <mach/sync_write.h>
#ifdef OUTREG32
  #undef OUTREG32
  #define OUTREG32(x, y) mt65xx_reg_sync_writel(y, x)
#endif

#ifndef OUTREGBIT
#define OUTREGBIT(TYPE,REG,bit,value)  \
                    do {    \
                        TYPE r = *((TYPE*)&INREG32(&REG));    \
                        r.bit = value;    \
                        OUTREG32(&REG, AS_UINT32(&r));    \
                    } while (0)
#endif



DPI_STATUS DPI1_DumpRegisters(void)
{
    return DPI_STATUS_OK;
}

