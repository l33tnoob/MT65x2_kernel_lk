#define ENABLE_DPI_INTERRUPT        1
#define ENABLE_DPI_REFRESH_RATE_LOG 0

#if ENABLE_DPI_REFRESH_RATE_LOG && !ENABLE_DPI_INTERRUPT
#error "ENABLE_DPI_REFRESH_RATE_LOG should be also ENABLE_DPI_INTERRUPT"
#endif

#if defined(MTK_HDMI_SUPPORT) && !ENABLE_DPI_INTERRUPT
//#error "enable MTK_HDMI_SUPPORT should be also ENABLE_DPI_INTERRUPT"
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
#include "dpi_drv.h"
#include "lcd_drv.h"
#include "dsi_drv.h"
#include <mach/mt_clkmgr.h>
#include "debug.h"

#if ENABLE_DPI_INTERRUPT
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

static PDSI_PHY_REGS const DSI_PHY_REG = (PDSI_PHY_REGS)(MIPI_CONFIG_BASE);
static PDPI_REGS const DPI_REG = (PDPI_REGS)(DISP_DPI_BASE);
static BOOL s_isDpiPowerOn = FALSE;
static BOOL s_isDpiMipiPowerOn = FALSE;
static DPI_REGS regBackup;
static void (*dpiIntCallback)(DISP_INTERRUPT_EVENTS);

#define DPI_REG_OFFSET(r)       offsetof(DPI_REGS, r)
#define REG_ADDR(base, offset)  (((BYTE *)(base)) + (offset))

#if !(defined(CONFIG_MT6572_FPGA) || defined(BUILD_UBOOT))
//#define DPI_MIPI_API
#endif


extern LCM_PARAMS *lcm_params;
extern LCM_DRIVER *lcm_drv;

const UINT32 BACKUP_DPI_REG_OFFSETS[] =
{
   DPI_REG_OFFSET(INT_ENABLE),
   DPI_REG_OFFSET(CON),
   DPI_REG_OFFSET(OUTPUT_SETTING),
   DPI_REG_OFFSET(SIZE),
   
   DPI_REG_OFFSET(TGEN_HWIDTH),
   DPI_REG_OFFSET(TGEN_HPORCH),
   DPI_REG_OFFSET(TGEN_VWIDTH),
   DPI_REG_OFFSET(TGEN_VPORCH),
   
   DPI_REG_OFFSET(BG_HCNTL),
   DPI_REG_OFFSET(BG_VCNTL),
   DPI_REG_OFFSET(BG_COLOR),

   DPI_REG_OFFSET(FIFO_CTL),
   DPI_REG_OFFSET(TMODE),
};


DPI_STATUS DPI_BackupRegisters(void)
{
    DPI_REGS *reg = &regBackup;
    UINT32 i;
    
    for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i])));
    }

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_RestoreRegisters(void)
{
    DPI_REGS *reg = &regBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i])));
    }

    return DPI_STATUS_OK;
}


static void _ResetBackupedDPIRegisterValues(void)
{
    DPI_REGS *regs = &regBackup;

    memset((void*)regs, 0, sizeof(DPI_REGS));
}


#if ENABLE_DPI_REFRESH_RATE_LOG
static void _DPI_LogRefreshRate(DPI_REG_INTERRUPT status)
{
   static unsigned long prevUs = 0xFFFFFFFF;
   
   if (status.VSYNC)
   {
      struct timeval curr;
      do_gettimeofday(&curr);
      
      if (prevUs < curr.tv_usec)
      {
         DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "Receive 1 vsync in %lu us\n", 
                                      curr.tv_usec - prevUs);
      }
      prevUs = curr.tv_usec;
   }
}
#else
#define _DPI_LogRefreshRate(x)  do {} while(0)
#endif

extern void dsi_handle_esd_recovery(void);


void DPI_DisableIrq(void)
{
#if ENABLE_DPI_INTERRUPT
   DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;

   enInt.VSYNC = 0;
   enInt.VDE = 0;
   enInt.UNDERFLOW = 0;
   OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
#endif
}


void DPI_EnableIrq(void)
{
#if ENABLE_DPI_INTERRUPT
   DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;

   enInt.VSYNC = 1;
   enInt.VDE = 0;
   enInt.UNDERFLOW = 1;
   OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
#endif
}


#if ENABLE_DPI_INTERRUPT
static irqreturn_t _DPI_InterruptHandler(int irq, void *dev_id)
{   
   static int counter = 0;
   DPI_REG_INTERRUPT status = DPI_REG->INT_STATUS;

   //    if (status.FIFO_EMPTY) ++ counter;
   
   OUTREG32(&DPI_REG->INT_STATUS, 0);
   if(status.VSYNC)
   {
      if(dpiIntCallback)
         dpiIntCallback(DISP_DPI_VSYNC_INT);
      #ifndef BUILD_UBOOT
         if(wait_dpi_vsync){
            if(-1 != hrtimer_try_to_cancel(&hrtimer_vsync_dpi)){
               dpi_vsync = true;
               //			hrtimer_try_to_cancel(&hrtimer_vsync_dpi);
               wake_up_interruptible(&_vsync_wait_queue_dpi);
            }
         }
      #endif
   }
   
   if (status.VSYNC && counter) {
      DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "[Error] DPI FIFO is empty, "
      "received %d times interrupt !!!\n", counter);
      counter = 0;
   }
   
   _DPI_LogRefreshRate(status);
   OUTREG32(&DPI_REG->INT_STATUS, 0);

   return IRQ_HANDLED;
}
#endif


#define VSYNC_US_TO_NS(x) (x * 1000)
unsigned int vsync_timer_dpi = 0;


void DPI_WaitVSYNC(void)
{
   wait_dpi_vsync = true;
   hrtimer_start(&hrtimer_vsync_dpi, ktime_set(0, VSYNC_US_TO_NS(vsync_timer_dpi)), HRTIMER_MODE_REL);
   wait_event_interruptible(_vsync_wait_queue_dpi, dpi_vsync);
   dpi_vsync = false;
   wait_dpi_vsync = false;
}


void DPI_PauseVSYNC(bool enable)
{
}


enum hrtimer_restart dpi_vsync_hrtimer_func(struct hrtimer *timer)
{
   //	long long ret;

   if(wait_dpi_vsync)
   {
      dpi_vsync = true;
      wake_up_interruptible(&_vsync_wait_queue_dpi);
      //		printk("hrtimer Vsync, and wake up\n");
   }
   //	ret = hrtimer_forward_now(timer, ktime_set(0, VSYNC_US_TO_NS(vsync_timer_dpi)));
   //	printk("hrtimer callback\n");

   return HRTIMER_NORESTART;
}


void DPI_InitVSYNC(unsigned int vsync_interval)
{
   ktime_t ktime;

   vsync_timer_dpi = vsync_interval;
   ktime = ktime_set(0, VSYNC_US_TO_NS(vsync_timer_dpi));
   hrtimer_init(&hrtimer_vsync_dpi, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
   hrtimer_vsync_dpi.function = dpi_vsync_hrtimer_func;
   //	hrtimer_start(&hrtimer_vsync_dpi, ktime, HRTIMER_MODE_REL);
}

DPI_STATUS DPI_Init(BOOL isDpiPoweredOn)
{
    //DPI_REG_CNTL cntl;
    //DPI_REG_EMBSYNC_SETTING embsync;

    if (isDpiPoweredOn) {
        DPI_BackupRegisters();
    } else {
        _ResetBackupedDPIRegisterValues();
    }

    DPI_PowerOn();

#if ENABLE_DPI_INTERRUPT
    if (request_irq(MT_DISP_DPI_IRQ_ID,
        _DPI_InterruptHandler, IRQF_TRIGGER_LOW, "mtkdpi", NULL) < 0)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "[ERROR] fail to request DPI irq\n"); 
        return DPI_STATUS_ERROR;
    }

    {
        DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;
        enInt.VSYNC = 1;
        OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
    }
#endif

   LCD_W2M_NeedLimiteSpeed(TRUE);

   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_Init);


DPI_STATUS DPI_FreeIRQ(void)
{
#if ENABLE_DPI_INTERRUPT
    free_irq(MT_DISP_DPI_IRQ_ID, NULL);
#endif
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FreeIRQ);


DPI_STATUS DPI_Deinit(void)
{
   DPI_DisableClk();
   DPI_PowerOff();
   
   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_Deinit);


void DPI_mipi_switch(bool on, LCM_PARAMS *lcm_params)
{
   DSI_PHY_clk_switch(on, lcm_params); 
}

#ifndef BULID_UBOOT
extern UINT32 FB_Addr;
#endif
#define HJ101NA02A 1
#define BP101WX1   2
#define AT070TNA2  3


DPI_STATUS DPI_Init_PLL(LCM_PARAMS *lcm_params)
{
    DSI_PHY_clk_setting(lcm_params);

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_Init_PLL);


DPI_STATUS DPI_Set_DrivingCurrent(LCM_PARAMS *lcm_params)
{
    LCD_Set_DrivingCurrent(lcm_params);

    return DPI_STATUS_OK;
}

DPI_STATUS DPI_PowerOn()
{
    if (!s_isDpiPowerOn)
    {
        int ret = 0;

        if (!clock_is_on(MT_CG_DISP_DPI_ENGINE_SW_CG))
            ret += enable_clock(MT_CG_DISP_DPI_ENGINE_SW_CG, "DPI");
        if (!clock_is_on(MT_CG_DISP_DPI_IF_SW_CG))
            ret += enable_clock(MT_CG_DISP_DPI_IF_SW_CG, "DPI");
        if (ret > 0)
        {
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "power manager API return FALSE\n");
        }

        s_isDpiPowerOn = TRUE;
   }

   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_PowerOn);


DPI_STATUS DPI_PowerOff()
{
    if (s_isDpiPowerOn)
    {
        int ret = 0;

        if (clock_is_on(MT_CG_DISP_DPI_IF_SW_CG))
            ret += disable_clock(MT_CG_DISP_DPI_IF_SW_CG, "DPI");
        if (clock_is_on(MT_CG_DISP_DPI_ENGINE_SW_CG))
            ret += disable_clock(MT_CG_DISP_DPI_ENGINE_SW_CG, "DPI");
        if (ret > 0)
        {
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "power manager API return FALSE\n");
        }

        s_isDpiPowerOn = FALSE;
    }

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_PowerOff);


DPI_STATUS DPI_MIPI_PowerOn()
{
    if (!s_isDpiMipiPowerOn)
    {
        int ret = 0;

        if (!clock_is_on(MT_CG_MIPI_26M_DBG_EN))
            ret += enable_clock(MT_CG_MIPI_26M_DBG_EN, "DSI");
        if (ret > 0)
        {
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "power manager API return FALSE\n");
        }

        s_isDpiMipiPowerOn = TRUE;
   }

   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_MIPI_PowerOn);


DPI_STATUS DPI_MIPI_PowerOff()
{
    if (s_isDpiMipiPowerOn)
    {
        int ret = 0;

        if (clock_is_on(MT_CG_MIPI_26M_DBG_EN))
            ret += disable_clock(MT_CG_MIPI_26M_DBG_EN, "DSI");
        if (ret > 0)
        {
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "power manager API return FALSE\n");
        }

        s_isDpiMipiPowerOn = FALSE;
    }

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_MIPI_PowerOff);


DPI_STATUS DPI_EnableClk()
{
   DPI_REG_EN en = DPI_REG->DPI_EN;

   en.EN = 1;
   OUTREG32(&DPI_REG->DPI_EN, AS_UINT32(&en));
   //release mutex0

   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_EnableClk);


DPI_STATUS DPI_DisableClk()
{
   #define DPI_TIMEOUT  500  // 500 ms
   DPI_REG_EN en = DPI_REG->DPI_EN;
   unsigned int dpi_timeout_cnt = 0;
   unsigned int reg;
   

   en.EN = 0;
   OUTREG32(&DPI_REG->DPI_EN, AS_UINT32(&en));

   // wait for DPI back to idle
   while (dpi_timeout_cnt < DPI_TIMEOUT)
   {
       reg = AS_UINT32(&DPI_REG->STATUS);
       if (0x0 == (reg & 0x10000))
       {
           break;
       }
       mdelay(1);
       dpi_timeout_cnt++;
   }
   if (DPI_TIMEOUT <= dpi_timeout_cnt)
   {
       ASSERT(0);
   }
   printk("[DISP] cnt:%d \n", dpi_timeout_cnt);
   
   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_DisableClk);


DPI_STATUS DPI_StartTransfer(bool isMutexLocked)
{
    // needStartDSI = 1: For command mode or the first time of video mode.
    // After the first time of video mode. Configuration is applied in ConfigurationUpdateTask.
    extern struct mutex OverlaySettingMutex;

    MMProfileLogMetaStringEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, isMutexLocked, 0, "StartTransfer");

    if (!isMutexLocked)
        disp_path_get_mutex();

    mutex_lock(&OverlaySettingMutex);

    LCD_CHECK_RET(LCD_ConfigOVL());
    // Insert log for trigger point.
    DBG_OnTriggerLcd();

    // To trigger frame update.
    DPI_EnableClk();

    mutex_unlock(&OverlaySettingMutex);

    if (!isMutexLocked)
        disp_path_release_mutex();

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_StartTransfer);


DPI_STATUS DPI_EnableSeqOutput(BOOL enable)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_EnableSeqOutput);


DPI_STATUS DPI_SetRGBOrder(DPI_RGB_ORDER input, DPI_RGB_ORDER output)
{
   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_SetRGBOrder);


DPI_STATUS DPI_ConfigPixelClk(DPI_POLARITY polarity, UINT32 divisor, UINT32 duty)
{
   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigPixelClk);


DPI_STATUS DPI_ConfigLVDS(LCM_PARAMS *lcm_params)
{
   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigLVDS);


DPI_STATUS DPI_ConfigDataEnable(DPI_POLARITY polarity)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigDataEnable);


DPI_STATUS DPI_ConfigVsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
   DPI_REG_TGEN_VPORCH vporch = DPI_REG->TGEN_VPORCH;
   
   vporch.VBP = backPorch;
   vporch.VFP = frontPorch;
   
   OUTREG32(&DPI_REG->TGEN_VWIDTH, AS_UINT32(&pulseWidth));
   OUTREG32(&DPI_REG->TGEN_VPORCH, AS_UINT32(&vporch));
    
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigVsync);


DPI_STATUS DPI_ConfigHsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
   DPI_REG_TGEN_HPORCH hporch = DPI_REG->TGEN_HPORCH;
   
   hporch.HBP = backPorch;
   hporch.HFP = frontPorch;
   
   OUTREG32(&DPI_REG->TGEN_HWIDTH, AS_UINT32(&pulseWidth));
   OUTREG32(&DPI_REG->TGEN_HPORCH, AS_UINT32(&hporch));

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigHsync);


DPI_STATUS DPI_FBEnable(DPI_FB_ID id, BOOL enable)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBEnable);


DPI_STATUS DPI_FBSyncFlipWithLCD(BOOL enable)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBSyncFlipWithLCD);


DPI_STATUS DPI_SetDSIMode(BOOL enable)
{
    return DPI_STATUS_OK;
}


BOOL DPI_IsDSIMode(void)
{
   //	return DPI_REG->CNTL.DSI_MODE ? TRUE : FALSE;
   return FALSE;
}


DPI_STATUS DPI_FBSetFormat(DPI_FB_FORMAT format)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBSetFormat);


DPI_FB_FORMAT DPI_FBGetFormat(void)
{
    return 0;
}
EXPORT_SYMBOL(DPI_FBGetFormat);


DPI_STATUS DPI_OutputSetting(void)
{
   DPI_REG_OUTPUT_SETTING output_setting = DPI_REG->OUTPUT_SETTING;

   output_setting.OUT_CH_SWAP = 0;
   output_setting.OUT_BIT_SWAP = 0;
   output_setting.B_MASK = 0;
   output_setting.G_MASK = 0;
   output_setting.R_MASK = 0;
   output_setting.DE_MASK = 0;
   output_setting.HS_MASK = 0;
   output_setting.VS_MASK = 0;
   output_setting.DE_POL = 0;
   output_setting.HSYNC_POL = 0;
   output_setting.VSYNC_POL = 0;
   output_setting.DPI_CK_POL = 0;
   output_setting.DPI_OEN_OFF = 0;
   output_setting.DUAL_EDGE_SEL = 0;

   OUTREG32(&DPI_REG->OUTPUT_SETTING, AS_UINT32(&output_setting));

   return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_OutputSetting);


DPI_STATUS DPI_FBSetSize(UINT32 width, UINT32 height)
{
    DPI_REG_SIZE size;
    size.WIDTH = width;
    size.HEIGHT = height;
    
    OUTREG32(&DPI_REG->SIZE, AS_UINT32(&size));

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBSetSize);


DPI_STATUS DPI_FBSetAddress(DPI_FB_ID id, UINT32 address)
{
    return DPI_STATUS_OK;
}    
EXPORT_SYMBOL(DPI_FBSetAddress);


DPI_STATUS DPI_FBSetPitch(DPI_FB_ID id, UINT32 pitchInByte)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBSetPitch);


DPI_STATUS DPI_SetFifoThreshold(UINT32 low, UINT32 high)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_SetFifoThreshold);


DPI_STATUS DPI_DumpRegisters(void)
{
    UINT32 i;

    DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "---------- Start dump DPI registers ----------\n");
    
    for (i = 0; i < sizeof(DPI_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "DPI+%04x : 0x%08x\n", i, INREG32(DISP_DPI_BASE + i));
    }

    for (i = 0; i < sizeof(DSI_PHY_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "DPI_PHY+%04x(%p) : 0x%08x\n", i, (UINT32*)(MIPI_CONFIG_BASE+i), INREG32((MIPI_CONFIG_BASE+i)));
    }

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_DumpRegisters);


UINT32 DPI_GetCurrentFB(void)
{
   return 0;
}
EXPORT_SYMBOL(DPI_GetCurrentFB);


DPI_STATUS DPI_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp)
{
    return DPI_STATUS_OK;    
}


static void _DPI_RDMA0_IRQ_Handler(unsigned int param)
{
    if (param & 4)
    {
        MMProfileLog(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagEnd);
        dpiIntCallback(DISP_DPI_SCREEN_UPDATE_END_INT);
    }
    if (param & 8)
    {
        MMProfileLog(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagEnd);
    }
    if (param & 2)
    {
        MMProfileLog(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagStart);
        dpiIntCallback(DISP_DPI_SCREEN_UPDATE_START_INT);
#if (ENABLE_DPI_INTERRUPT == 0)
        if(dpiIntCallback)
            dpiIntCallback(DISP_DPI_VSYNC_INT);
#endif
    }
    if (param & 0x20)
    {
        dpiIntCallback(DISP_DPI_TARGET_LINE_INT);
    }
}


static void _DPI_MUTEX_IRQ_Handler(unsigned int param)
{
    if(dpiIntCallback)
    {
        if (param & 1)
        {
            dpiIntCallback(DISP_DPI_REG_UPDATE_INT);
        }
    }
}


DPI_STATUS DPI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
#if ENABLE_DPI_INTERRUPT
    switch(eventID)
    {
        case DISP_DPI_VSYNC_INT:
            //DPI_REG->INT_ENABLE.VSYNC = 1;
            OUTREGBIT(DPI_REG_INTERRUPT,DPI_REG->INT_ENABLE,VSYNC,1);
            break;
        case DISP_DPI_FIFO_EMPTY_INT:
            //DPI_REG->INT_ENABLE.VSYNC = 1;
            OUTREGBIT(DPI_REG_INTERRUPT,DPI_REG->INT_ENABLE,UNDERFLOW,1);
            break;
        case DISP_DPI_TARGET_LINE_INT:
            disp_register_irq(DISP_MODULE_RDMA0, _DPI_RDMA0_IRQ_Handler);
            break;
        case DISP_DPI_SCREEN_UPDATE_START_INT:
            disp_register_irq(DISP_MODULE_RDMA0, _DPI_RDMA0_IRQ_Handler);
            break;
        case DISP_DPI_SCREEN_UPDATE_END_INT:
            disp_register_irq(DISP_MODULE_RDMA0, _DPI_RDMA0_IRQ_Handler);
            break;
        case DISP_DPI_REG_UPDATE_INT:
            disp_register_irq(DISP_MODULE_MUTEX, _DPI_MUTEX_IRQ_Handler);
            break;
        case DISP_DPI_FIFO_FULL_INT:
        case DISP_DPI_OUT_EMPTY_INT:
        case DISP_DPI_CNT_OVERFLOW_INT:
        case DISP_DPI_LINE_ERR_INT:
        default:
            return DPI_STATUS_ERROR;
    }

    return DPI_STATUS_OK;
#else
    switch(eventID)
    {
        case DISP_DPI_VSYNC_INT:
            OUTREGBIT(DPI_REG_INTERRUPT,DPI_REG->INT_ENABLE,VSYNC,1);
            disp_register_irq(DISP_MODULE_RDMA0, _DPI_RDMA0_IRQ_Handler);
            break;
        case DISP_DPI_TARGET_LINE_INT:
            disp_register_irq(DISP_MODULE_RDMA0, _DPI_RDMA0_IRQ_Handler);
            break;
        case DISP_DPI_REG_UPDATE_INT:
            disp_register_irq(DISP_MODULE_MUTEX, _DPI_MUTEX_IRQ_Handler);
            break;
        default:
            return DPI_STATUS_ERROR;
    }

    return DPI_STATUS_OK;
    ///TODO: warning log here
    //return DPI_STATUS_ERROR;
#endif
}


DPI_STATUS DPI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
    dpiIntCallback = pCB;

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_FMDesense_Query(void)
{
    return DPI_STATUS_ERROR;
}


DPI_STATUS DPI_FM_Desense(unsigned long freq)
{
    return DPI_STATUS_OK;
}


DPI_STATUS DPI_Reset_CLK(void)
{
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_Get_Default_CLK(unsigned int *clk)
{
    return DPI_STATUS_OK;
}


DPI_STATUS DPI_Get_Current_CLK(unsigned int *clk)
{
    return DPI_STATUS_OK;
}


DPI_STATUS DPI_Change_CLK(unsigned int clk)
{
    return DPI_STATUS_OK;
}


unsigned int DPI_Check_LCM()
{
    unsigned int ret = 0;

    if(lcm_drv->ata_check)
        ret = lcm_drv->ata_check(NULL);

    return ret;
}

