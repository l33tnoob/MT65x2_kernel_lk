#define ENABLE_LCD_INTERRUPT 1 


#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>

#include "ddp_reg.h"
#include "ddp_debug.h"
#include "ddp_hal.h"
#include "lcd_reg.h"
#include "lcd_drv.h"

#include "disp_drv_log.h"
#include "disp_drv_platform.h"

#include <linux/hrtimer.h>

#if ENABLE_LCD_INTERRUPT
//#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include "ddp_drv.h"
//#include <asm/tcm.h>
#include <mach/irqs.h>
#include "mtkfb.h"
static wait_queue_head_t _lcd_wait_queue;
#endif
static wait_queue_head_t _vsync_wait_queue;
static bool lcd_vsync = false;
static bool wait_lcd_vsync = false;
static struct hrtimer hrtimer_vsync;
#define VSYNC_US_TO_NS(x) (x * 1000)
unsigned int vsync_timer = 0;
#include "debug.h"
#include <asm/current.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#define LCD_OUTREG32(addr, data)	\
   {\
   OUTREG32(addr, data);}

#define LCD_OUTREG8(addr, data)	\
   {\
   OUTREG8(addr, data);}

#define LCD_OUTREG16(addr, data)	\
   {\
   OUTREG16(addr, data);}

#define LCD_MASKREG32(addr, mask, data)	\
   {\
   MASKREG32(addr, mask, data);}

#ifndef OUTREGBIT
#define OUTREGBIT(TYPE,REG,bit,value)  \
                    do {    \
                        TYPE r = *((TYPE*)&INREG32(&REG));    \
                        r.bit = value;    \
                        OUTREG32(&REG, AS_UINT32(&r));    \
                    } while (0)
#endif

#ifdef MTK_LCDC_ENABLE_M4U
    #undef MTK_LCDC_ENABLE_M4U
#endif


void dbi_log_enable(int enable)
{
    dbi_drv_dbg_log = enable;
    dbi_drv_dbg_func_log = enable;
    DBI_DRV_INFO("lcd log %s\n", enable?"enabled":"disabled");
}

static PLCD_REGS const LCD_REG = (PLCD_REGS)(DISP_DBI_BASE);
static const UINT32 TO_BPP[LCD_FB_FORMAT_NUM] = {2, 3, 4};
unsigned int wait_time = 0;
typedef struct
{
   LCD_FB_FORMAT fbFormat;
   UINT32 fbPitchInBytes;
   LCD_REG_SIZE roiWndSize;
   LCD_OUTPUT_MODE outputMode;
   LCD_REGS regBackup;
   void (*pIntCallback)(DISP_INTERRUPT_EVENTS);
} LCD_CONTEXT;

static LCD_CONTEXT _lcdContext;
static int wst_step_LCD = -1;//for LCD&FM de-sense
static bool is_get_default_write_cycle = FALSE;
static unsigned int default_write_cycle = 0;
static UINT32 default_wst = 0;
static bool limit_w2m_speed = false;
static bool limit_w2tvr_speed = false;
// UI layer, default set to 3
extern OVL_CONFIG_STRUCT cached_layer_config[DDP_OVL_LAYER_MUN];
extern OVL_CONFIG_STRUCT* realtime_layer_config;
extern LCM_PARAMS *lcm_params;
extern LCM_DRIVER *lcm_drv;


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
LCD_STATUS LCD_Init_IO_pad(LCM_PARAMS *lcm_params)
{
   //    #warning "LCD_Init_IO_pad not implement for 6589 yes"
   return LCD_STATUS_OK;
}

LCD_STATUS LCD_Set_DrivingCurrent(LCM_PARAMS *lcm_params)
{
    kal_uint32 data_driving;
    kal_uint32 control_driving;
    kal_uint32 driving_current = 0;


    if (lcm_params->type == LCM_TYPE_DPI)
    {
        driving_current = lcm_params->dpi.io_driving_current;
    }
    else if (lcm_params->type == LCM_TYPE_DBI)
    {
        driving_current = lcm_params->dbi.io_driving_current;
    }
    else
    {
        return LCD_STATUS_OK;
    }
    
    data_driving = *((volatile u32 *)(IO_CFG_LEFT_BASE+0x0080));
    control_driving = *((volatile u32 *)(IO_CFG_RIGHT_BASE+0x0060));

    switch (driving_current) 
    {
        case LCM_DRIVING_CURRENT_6575_4MA:
           *((volatile u32 *)(IO_CFG_LEFT_BASE+0x0080)) = (data_driving & (~0xC00)) | (LCD_IO_DATA_DRIVING_4MA << 10);
           *((volatile u32 *)(IO_CFG_RIGHT_BASE+0x0060)) = (control_driving & (~0x7)) | (LCD_IO_CTL_DRIVING_9MA);
            break;
        case LCM_DRIVING_CURRENT_6575_8MA:
           *((volatile u32 *)(IO_CFG_LEFT_BASE+0x0080)) = (data_driving & (~0xC00)) | (LCD_IO_DATA_DRIVING_8MA << 10);
           *((volatile u32 *)(IO_CFG_RIGHT_BASE+0x0060)) = (control_driving & (~0x7)) | (LCD_IO_CTL_DRIVING_12MA);
            break;
        case LCM_DRIVING_CURRENT_6575_12MA:
           *((volatile u32 *)(IO_CFG_LEFT_BASE+0x0080)) = (data_driving & (~0xC00)) | (LCD_IO_DATA_DRIVING_12MA << 10);
           *((volatile u32 *)(IO_CFG_RIGHT_BASE+0x0060)) = (control_driving & (~0x7)) | (LCD_IO_CTL_DRIVING_18MA);
            break;
        case LCM_DRIVING_CURRENT_6575_16MA:
           *((volatile u32 *)(IO_CFG_LEFT_BASE+0x0080)) = (data_driving & (~0xC00)) | (LCD_IO_DATA_DRIVING_16MA << 10);
           *((volatile u32 *)(IO_CFG_RIGHT_BASE+0x0060)) = (control_driving & (~0x7)) | (LCD_IO_CTL_DRIVING_21MA);
            break;
        default:  // 8mA
           *((volatile u32 *)(IO_CFG_LEFT_BASE+0x0080)) = (data_driving & (~0xC00)) | (LCD_IO_DATA_DRIVING_8MA << 10);
           *((volatile u32 *)(IO_CFG_RIGHT_BASE+0x0060)) = (control_driving & (~0x7)) | (LCD_IO_CTL_DRIVING_12MA);
            break;
    }

   return LCD_STATUS_OK;
}


static void _LCD_RDMA0_IRQ_Handler(unsigned int param)
{
    if(_lcdContext.pIntCallback)
    {
        if (param & 4)
        {
            // frame end interrupt
            MMProfileLogEx(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagEnd, param, 0);
            _lcdContext.pIntCallback(DISP_LCD_SCREEN_UPDATE_END_INT);
        }
        if (param & 8)
        {
            // abnormal EOF interrupt
            MMProfileLogEx(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagEnd, param, 0);
        }
        if (param & 2)
        {
            // frame start interrupt
            MMProfileLogEx(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagStart, param, 0);
            _lcdContext.pIntCallback(DISP_LCD_SCREEN_UPDATE_START_INT);
        }
        if (param & 0x20)
        {
            // target line interrupt
            _lcdContext.pIntCallback(DISP_LCD_TARGET_LINE_INT);
            _lcdContext.pIntCallback(DISP_LCD_VSYNC_INT);
        }
    }
}


static void _LCD_MUTEX_IRQ_Handler(unsigned int param)
{
    if(_lcdContext.pIntCallback)
    {
        if (param & 1)
        {
            // mutex0 register update interrupt
            _lcdContext.pIntCallback(DISP_LCD_REG_COMPLETE_INT);
        }
    }
}


#if ENABLE_LCD_INTERRUPT
static irqreturn_t _LCD_InterruptHandler(int irq, void *dev_id)
{   
   LCD_REG_INTERRUPT status = LCD_REG->INT_STATUS;
   MMProfileLogEx(DDP_MMP_Events.ROT_IRQ, MMProfileFlagPulse, AS_UINT32(&status), 0);
   
   if (status.COMPLETED)
   {
      ///write clear COMPLETED interrupt
      status.COMPLETED = 1;

#ifdef CONFIG_MTPROF_APPLAUNCH  // eng enable, user disable   	
      LOG_PRINT(ANDROID_LOG_INFO, "AppLaunch", "LCD frame buffer update done !\n");
#endif
      wake_up_interruptible(&_lcd_wait_queue);
      
      if(_lcdContext.pIntCallback)
         _lcdContext.pIntCallback(DISP_LCD_TRANSFER_COMPLETE_INT);
      
      DBG_OnLcdDone();
   }
   
   if (status.CMDQ_COMPLETED)
   {
      // The last screen update has finished.
      if(_lcdContext.pIntCallback)
         _lcdContext.pIntCallback(DISP_LCD_CDMQ_COMPLETE_INT);

      DBG_OnLcdDone();
      
      //LCD_REG->INT_STATUS.CMDQ_COMPLETED = 0;
      status.CMDQ_COMPLETED = 1;
      
      wake_up_interruptible(&_lcd_wait_queue);

      //if(_lcdContext.pIntCallback)
      //    _lcdContext.pIntCallback(DISP_LCD_CDMQ_COMPLETE_INT);   
      //MASKREG32(&LCD_REG->INT_STATUS, 0x2, 0x0);
   }
   
   if (status.TE)// this is TE mode 0 interrupt
   {
      DBG_OnTeDelayDone();
      
      // Write clear TE
      status.TE = 1;

      if(_lcdContext.pIntCallback)
         _lcdContext.pIntCallback(DISP_LCD_SYNC_INT);
#ifndef BUILD_UBOOT    
      if(wait_lcd_vsync)//judge if wait vsync
      {
         if(-1 != hrtimer_try_to_cancel(&hrtimer_vsync)){
            lcd_vsync = true;
            
            //			hrtimer_try_to_cancel(&hrtimer_vsync);
            wake_up_interruptible(&_vsync_wait_queue);
         }
         //			printk("TE signal, and wake up\n");
      }
#endif  
      DBG_OnTeDelayDone();
   }

   if (status.HTT)
   {
      status.HTT = 1;
   }
   if (status.SYNC)
   {
      status.SYNC = 1;
   }
   LCD_OUTREG32(&LCD_REG->INT_STATUS, ~(AS_UINT32(&status)));
   
   return IRQ_HANDLED;
}
#endif


static BOOL _IsEngineBusy(void)
{
    LCD_REG_STATUS status;

    status = LCD_REG->STATUS;
    if (status.BUSY) 
        return TRUE;

    return FALSE;
}


BOOL LCD_IsBusy(void)
{
   return _IsEngineBusy();
}


static void _WaitForLCDEngineComplete(void)
{
   do
   {
      if ((DISP_REG_GET(&LCD_REG->INT_STATUS)& 0x1) == 0x1)
      {
         break;
      }
   } while(1);
}


static void _WaitForEngineNotBusy(void)
{
#if ENABLE_LCD_INTERRUPT
   static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
   
   if (in_interrupt())
   {
      // perform busy waiting if in interrupt context
      if(disp_delay_timeout(((DISP_REG_GET(&LCD_REG->STATUS)& 0x10) != 0x10), 5)) {}
   }
   else
   {
      if(disp_delay_timeout(((DISP_REG_GET(&LCD_REG->STATUS)& 0x10) != 0x10), 5))
      {
         long ret = wait_event_interruptible_timeout(_lcd_wait_queue, 
                                                                         !_IsEngineBusy(),
                                                                         WAIT_TIMEOUT);
         if (0 == ret) {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "LCD", "[WARNING] Wait for LCD engine not busy timeout!!!\n"); 
            LCD_DumpRegisters();

            if(LCD_REG->STATUS.WAIT_SYNC){
               DISP_LOG_PRINT(ANDROID_LOG_WARN, "LCD", "reason is LCD can't wait TE signal!!!\n"); 
               LCD_TE_Enable(FALSE);
            }

            OUTREG16(&LCD_REG->START, 0);
            OUTREG16(&LCD_REG->START, 0x1);
         }
      }
   }
#else
   if(disp_delay_timeout(((DISP_REG_GET(&LCD_REG->STATUS)& 0x10) != 0x10), 5)){
      printk("[WARNING] Wait for LCD engine not busy timeout!!!\n");
      LCD_DumpRegisters();
      
      if(LCD_REG->STATUS.WAIT_SYNC){
         printk("reason is LCD can't wait TE signal!!!\n");
         LCD_TE_Enable(FALSE);
         return;
      }

      OUTREG16(&LCD_REG->START, 0);
      OUTREG16(&LCD_REG->START, 0x1);
   }
#endif    
}

unsigned int vsync_wait_time = 0;
void LCD_WaitTE(void)
{
   wait_lcd_vsync = true;
   hrtimer_start(&hrtimer_vsync, ktime_set(0, VSYNC_US_TO_NS(vsync_timer)), HRTIMER_MODE_REL);
   wait_event_interruptible(_vsync_wait_queue, lcd_vsync);
   lcd_vsync = false;
   wait_lcd_vsync = false;
}

void LCD_GetVsyncCnt()
{
}
enum hrtimer_restart lcd_te_hrtimer_func(struct hrtimer *timer)
{
   //	long long ret;
   if(wait_lcd_vsync)
   {
      lcd_vsync = true;
      wake_up_interruptible(&_vsync_wait_queue);
      //		printk("hrtimer Vsync, and wake up\n");
   }

   //	ret = hrtimer_forward_now(timer, ktime_set(0, VSYNC_US_TO_NS(vsync_timer)));
   //	printk("hrtimer callback\n");
   return HRTIMER_NORESTART;
}

void LCD_InitVSYNC(unsigned int vsync_interval)
{
   ktime_t ktime;

   vsync_timer = vsync_interval;
   ktime = ktime_set(0, VSYNC_US_TO_NS(vsync_timer));
   hrtimer_init(&hrtimer_vsync, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
   hrtimer_vsync.function = lcd_te_hrtimer_func;
   //	hrtimer_start(&hrtimer_vsync, ktime, HRTIMER_MODE_REL);
}
void LCD_PauseVSYNC(bool enable)
{
}

static void _BackupLCDRegisters(void)
{
   //    memcpy((void*)&(_lcdContext.regBackup), (void*)LCD_REG, sizeof(LCD_REGS));
   LCD_REGS *regs = &(_lcdContext.regBackup);
   UINT32 i;

   LCD_OUTREG32(&regs->INT_ENABLE, AS_UINT32(&LCD_REG->INT_ENABLE));
   LCD_OUTREG32(&regs->SERIAL_CFG, AS_UINT32(&LCD_REG->SERIAL_CFG));
   
   for(i = 0; i < ARY_SIZE(LCD_REG->SIF_TIMING); ++i)
   {
      LCD_OUTREG32(&regs->SIF_TIMING[i], AS_UINT32(&LCD_REG->SIF_TIMING[i]));
   }
   
   for(i = 0; i < ARY_SIZE(LCD_REG->PARALLEL_CFG); ++i)
   {
      LCD_OUTREG32(&regs->PARALLEL_CFG[i], AS_UINT32(&LCD_REG->PARALLEL_CFG[i]));
   }
   
   LCD_OUTREG32(&regs->TEARING_CFG, AS_UINT32(&LCD_REG->TEARING_CFG));
   LCD_OUTREG32(&regs->PARALLEL_DW, AS_UINT32(&LCD_REG->PARALLEL_DW));
   LCD_OUTREG32(&regs->CALC_HTT, AS_UINT32(&LCD_REG->CALC_HTT));
   LCD_OUTREG32(&regs->SYNC_LCM_SIZE, AS_UINT32(&LCD_REG->SYNC_LCM_SIZE));
   LCD_OUTREG32(&regs->SYNC_CNT, AS_UINT32(&LCD_REG->SYNC_CNT));
   LCD_OUTREG32(&regs->SMI_CON, AS_UINT32(&LCD_REG->SMI_CON));
   
   LCD_OUTREG32(&regs->WROI_CONTROL, AS_UINT32(&LCD_REG->WROI_CONTROL));
   LCD_OUTREG32(&regs->WROI_CMD_ADDR, AS_UINT32(&LCD_REG->WROI_CMD_ADDR));
   LCD_OUTREG32(&regs->WROI_DATA_ADDR, AS_UINT32(&LCD_REG->WROI_DATA_ADDR));
   LCD_OUTREG32(&regs->WROI_SIZE, AS_UINT32(&LCD_REG->WROI_SIZE));
   
   //	LCD_OUTREG32(&regs->DITHER_CON, AS_UINT32(&LCD_REG->DITHER_CON));
   LCD_OUTREG32(&regs->SRC_CON, AS_UINT32(&LCD_REG->SRC_CON));
   
   LCD_OUTREG32(&regs->SRC_ADD, AS_UINT32(&LCD_REG->SRC_ADD));
   LCD_OUTREG32(&regs->SRC_PITCH, AS_UINT32(&LCD_REG->SRC_PITCH));
   
   LCD_OUTREG32(&regs->ULTRA_CON, AS_UINT32(&LCD_REG->ULTRA_CON));
   LCD_OUTREG32(&regs->DBI_ULTRA_TH, AS_UINT32(&LCD_REG->DBI_ULTRA_TH));
   
   LCD_OUTREG32(&regs->GMC_ULTRA_TH, AS_UINT32(&LCD_REG->GMC_ULTRA_TH));
}


static void _RestoreLCDRegisters(void)
{
   LCD_REGS *regs = &(_lcdContext.regBackup);
   UINT32 i;

   LCD_OUTREG32(&LCD_REG->INT_ENABLE, AS_UINT32(&regs->INT_ENABLE));
   LCD_OUTREG32(&LCD_REG->SERIAL_CFG, AS_UINT32(&regs->SERIAL_CFG));
   
   for(i = 0; i < ARY_SIZE(LCD_REG->SIF_TIMING); ++i)
   {
      LCD_OUTREG32(&LCD_REG->SIF_TIMING[i], AS_UINT32(&regs->SIF_TIMING[i]));
   }
   
   for(i = 0; i < ARY_SIZE(LCD_REG->PARALLEL_CFG); ++i)
   {
      LCD_OUTREG32(&LCD_REG->PARALLEL_CFG[i], AS_UINT32(&regs->PARALLEL_CFG[i]));
   }
   
   LCD_OUTREG32(&LCD_REG->TEARING_CFG, AS_UINT32(&regs->TEARING_CFG));
   LCD_OUTREG32(&LCD_REG->PARALLEL_DW, AS_UINT32(&regs->PARALLEL_DW));
   LCD_OUTREG32(&LCD_REG->CALC_HTT, AS_UINT32(&regs->CALC_HTT));
   LCD_OUTREG32(&LCD_REG->SYNC_LCM_SIZE, AS_UINT32(&regs->SYNC_LCM_SIZE));
   LCD_OUTREG32(&LCD_REG->SYNC_CNT, AS_UINT32(&regs->SYNC_CNT));
   LCD_OUTREG32(&LCD_REG->SMI_CON, AS_UINT32(&regs->SMI_CON));
   
   LCD_OUTREG32(&LCD_REG->WROI_CONTROL, AS_UINT32(&regs->WROI_CONTROL));
   LCD_OUTREG32(&LCD_REG->WROI_CMD_ADDR, AS_UINT32(&regs->WROI_CMD_ADDR));
   LCD_OUTREG32(&LCD_REG->WROI_DATA_ADDR, AS_UINT32(&regs->WROI_DATA_ADDR));
   LCD_OUTREG32(&LCD_REG->WROI_SIZE, AS_UINT32(&regs->WROI_SIZE));
   
   //	LCD_OUTREG32(&LCD_REG->DITHER_CON, AS_UINT32(&regs->DITHER_CON));
   
   LCD_OUTREG32(&LCD_REG->SRC_CON, AS_UINT32(&regs->SRC_CON));
   
   LCD_OUTREG32(&LCD_REG->SRC_ADD, AS_UINT32(&regs->SRC_ADD));
   LCD_OUTREG32(&LCD_REG->SRC_PITCH, AS_UINT32(&regs->SRC_PITCH));
   
   LCD_OUTREG32(&LCD_REG->ULTRA_CON, AS_UINT32(&regs->ULTRA_CON));
   LCD_OUTREG32(&LCD_REG->DBI_ULTRA_TH, AS_UINT32(&regs->DBI_ULTRA_TH));
   
   LCD_OUTREG32(&LCD_REG->GMC_ULTRA_TH, AS_UINT32(&regs->GMC_ULTRA_TH));
}


LCD_STATUS LCD_BackupRegisters(void)
{
    LCD_REGS *regs = &(_lcdContext.regBackup);
    UINT32 i;

    //memcpy((void*)&(_lcdContext.regBackup), (void*)LCD_BASE, sizeof(LCD_REGS));

    LCD_OUTREG32(&regs->INT_ENABLE, AS_UINT32(&LCD_REG->INT_ENABLE));
    LCD_OUTREG32(&regs->SERIAL_CFG, AS_UINT32(&LCD_REG->SERIAL_CFG));

    for(i = 0; i < ARY_SIZE(LCD_REG->SIF_TIMING); ++i)
    {
        LCD_OUTREG32(&regs->SIF_TIMING[i], AS_UINT32(&LCD_REG->SIF_TIMING[i]));
    }
    
    for(i = 0; i < ARY_SIZE(LCD_REG->PARALLEL_CFG); ++i)
    {
        LCD_OUTREG32(&regs->PARALLEL_CFG[i], AS_UINT32(&LCD_REG->PARALLEL_CFG[i]));
    }

    LCD_OUTREG32(&regs->TEARING_CFG, AS_UINT32(&LCD_REG->TEARING_CFG));
    LCD_OUTREG32(&regs->PARALLEL_DW, AS_UINT32(&LCD_REG->PARALLEL_DW));
    LCD_OUTREG32(&regs->CALC_HTT, AS_UINT32(&LCD_REG->CALC_HTT));
    LCD_OUTREG32(&regs->SYNC_LCM_SIZE, AS_UINT32(&LCD_REG->SYNC_LCM_SIZE));
    LCD_OUTREG32(&regs->SYNC_CNT, AS_UINT32(&LCD_REG->SYNC_CNT));
    LCD_OUTREG32(&regs->SMI_CON, AS_UINT32(&LCD_REG->SMI_CON));

    LCD_OUTREG32(&regs->WROI_CONTROL, AS_UINT32(&LCD_REG->WROI_CONTROL));
    LCD_OUTREG32(&regs->WROI_CMD_ADDR, AS_UINT32(&LCD_REG->WROI_CMD_ADDR));
    LCD_OUTREG32(&regs->WROI_DATA_ADDR, AS_UINT32(&LCD_REG->WROI_DATA_ADDR));
    LCD_OUTREG32(&regs->WROI_SIZE, AS_UINT32(&LCD_REG->WROI_SIZE));

    LCD_OUTREG32(&regs->SRC_CON, AS_UINT32(&LCD_REG->SRC_CON));

    LCD_OUTREG32(&regs->SRC_ADD, AS_UINT32(&LCD_REG->SRC_ADD));
    LCD_OUTREG32(&regs->SRC_PITCH, AS_UINT32(&LCD_REG->SRC_PITCH));

    LCD_OUTREG32(&regs->ULTRA_CON, AS_UINT32(&LCD_REG->ULTRA_CON));
    LCD_OUTREG32(&regs->DBI_ULTRA_TH, AS_UINT32(&LCD_REG->DBI_ULTRA_TH));
    LCD_OUTREG32(&regs->GMC_ULTRA_TH, AS_UINT32(&LCD_REG->GMC_ULTRA_TH));

    return LCD_STATUS_OK;
}


static void _ResetBackupedLCDRegisterValues(void)
{
    LCD_REGS *regs = &_lcdContext.regBackup;

    memset((void*)regs, 0, sizeof(LCD_REGS));
}


// ---------------------------------------------------------------------------
//  LCD Controller API Implementations
// ---------------------------------------------------------------------------
LCD_STATUS LCD_Init(BOOL isLcdPoweredOn)
{
   LCD_STATUS ret = LCD_STATUS_OK;
   
   memset(&_lcdContext, 0, sizeof(_lcdContext));
   
   // LCD controller would NOT reset register as default values
   // Do it by SW here
   //
   if (isLcdPoweredOn) {
      LCD_BackupRegisters();
   } 
   else {
      _ResetBackupedLCDRegisterValues();
   }
   
   ret = LCD_PowerOn();
   ASSERT(ret == LCD_STATUS_OK);
   
   LCD_OUTREG32(&LCD_REG->SYNC_LCM_SIZE, 0x00010001);
   LCD_OUTREG32(&LCD_REG->SYNC_CNT, 0x1);
   
#if ENABLE_LCD_INTERRUPT
   init_waitqueue_head(&_lcd_wait_queue);
   init_waitqueue_head(&_vsync_wait_queue);

   if (request_irq(MT_DISP_DBI_IRQ_ID,
       _LCD_InterruptHandler, IRQF_TRIGGER_LOW, MTKFB_DRIVER, NULL) < 0)
   {
      DBI_DRV_WRAN("[LCD][ERROR] fail to request LCD irq\n"); 
      ASSERT(0);
      return LCD_STATUS_ERROR;
   }

   OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,COMPLETED,1);
   OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,CMDQ_COMPLETED,1);
   OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,HTT,1);
   OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,SYNC,1);
   OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,TE,1);

   disp_register_irq(DISP_MODULE_RDMA0, _LCD_RDMA0_IRQ_Handler);
#endif
    
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_Deinit(void)
{
   LCD_STATUS ret = LCD_PowerOff();
   
   ASSERT(ret == LCD_STATUS_OK);
   return LCD_STATUS_OK;
}

static BOOL s_isLcdPowerOn = FALSE;

LCD_STATUS LCD_PowerOn(void)
{
    DBI_DRV_FUNC("[%s]:enter \n", __func__);
 
    if (!s_isLcdPowerOn)
    {
        int ret = 0;
  
        DBI_DRV_INFO("lcd will be power on\n");
        if (!clock_is_on(MT_CG_DBI_BCLK_SW_CG))
            ret += enable_clock(MT_CG_DBI_BCLK_SW_CG, "LCD");
        if (!clock_is_on(MT_CG_DISP_DBI_IF_SW_CG))
            ret = enable_clock(MT_CG_DISP_DBI_IF_SW_CG, "LCD");
        if (!clock_is_on(MT_CG_DBI_PAD0_SW_CG))
            ret += enable_clock(MT_CG_DBI_PAD0_SW_CG, "LCD");
        if (!clock_is_on(MT_CG_DBI_PAD1_SW_CG))
            ret += enable_clock(MT_CG_DBI_PAD1_SW_CG, "LCD");
        if (!clock_is_on(MT_CG_DBI_PAD2_SW_CG))
            ret += enable_clock(MT_CG_DBI_PAD2_SW_CG, "LCD");
        if (!clock_is_on(MT_CG_DBI_PAD3_SW_CG))
            ret += enable_clock(MT_CG_DBI_PAD3_SW_CG, "LCD");
        if (!clock_is_on(MT_CG_DISP_DBI_ENGINE_SW_CG))
            ret += enable_clock(MT_CG_DISP_DBI_ENGINE_SW_CG, "LCD");
        if (!clock_is_on(MT_CG_DISP_DBI_SMI_SW_CG))
            ret += enable_clock(MT_CG_DISP_DBI_SMI_SW_CG, "LCD");
        if(ret > 0)
        {
            DBI_DRV_WRAN("[LCD]power manager API return FALSE\n");
            ASSERT(0);
        }      
        _RestoreLCDRegisters();
        s_isLcdPowerOn = TRUE;
    }
 
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_PowerOff(void)
{
    DBI_DRV_FUNC("[%s]:enter \n", __func__);
 
    if (s_isLcdPowerOn)
    {
        int ret = 0;
        
        _WaitForEngineNotBusy();
        _BackupLCDRegisters();
  
        DBI_DRV_INFO("lcd will be power off\n");
        if (clock_is_on(MT_CG_DISP_DBI_SMI_SW_CG))
            ret += disable_clock(MT_CG_DISP_DBI_SMI_SW_CG, "LCD");
        if (clock_is_on(MT_CG_DISP_DBI_ENGINE_SW_CG))
            ret += disable_clock(MT_CG_DISP_DBI_ENGINE_SW_CG, "LCD");
        if (clock_is_on(MT_CG_DBI_PAD3_SW_CG))
            ret = disable_clock(MT_CG_DBI_PAD3_SW_CG, "LCD");
        if (clock_is_on(MT_CG_DBI_PAD2_SW_CG))
            ret += disable_clock(MT_CG_DBI_PAD2_SW_CG, "LCD");
        if (clock_is_on(MT_CG_DBI_PAD1_SW_CG))
            ret += disable_clock(MT_CG_DBI_PAD1_SW_CG, "LCD");
        if (clock_is_on(MT_CG_DBI_PAD0_SW_CG))
            ret += disable_clock(MT_CG_DBI_PAD0_SW_CG, "LCD");
        if (clock_is_on(MT_CG_DISP_DBI_IF_SW_CG))
            ret += disable_clock(MT_CG_DISP_DBI_IF_SW_CG, "LCD");
        if (clock_is_on(MT_CG_DBI_BCLK_SW_CG))
            ret += disable_clock(MT_CG_DBI_BCLK_SW_CG, "LCD");
        if(ret > 0)
        {
            DBI_DRV_WRAN("[LCD]power manager API return FALSE\n");
            ASSERT(0);
        } 			
  
        s_isLcdPowerOn = FALSE;
    }
 
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_WaitForNotBusy(void)
{
    _WaitForEngineNotBusy();
    return LCD_STATUS_OK;
}
EXPORT_SYMBOL(LCD_WaitForNotBusy);


LCD_STATUS LCD_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
#if ENABLE_LCD_INTERRUPT
   switch(eventID)
   {
      case DISP_LCD_TRANSFER_COMPLETE_INT:
         LCD_REG->INT_ENABLE.COMPLETED = 1;
         break;

      case DISP_LCD_CDMQ_COMPLETE_INT:
         LCD_REG->INT_ENABLE.CMDQ_COMPLETED = 1;
         break;

      case DISP_LCD_HTT_INT:
         LCD_REG->INT_ENABLE.HTT = 1;
         break;

      case DISP_LCD_SYNC_INT:
         LCD_REG->INT_ENABLE.SYNC = 1;
         break;

      case DISP_LCD_TE_INT:
         LCD_REG->INT_ENABLE.TE = 1;
         break;

      case DISP_LCD_VSYNC_INT:
         disp_register_irq(DISP_MODULE_RDMA0, _LCD_RDMA0_IRQ_Handler);
         break;

      case DISP_LCD_SCREEN_UPDATE_START_INT:
         disp_register_irq(DISP_MODULE_RDMA0, _LCD_RDMA0_IRQ_Handler);
         break;

      case DISP_LCD_SCREEN_UPDATE_END_INT:
         disp_register_irq(DISP_MODULE_RDMA0, _LCD_RDMA0_IRQ_Handler);
         break;

      case DISP_LCD_TARGET_LINE_INT:
         disp_register_irq(DISP_MODULE_RDMA0, _LCD_RDMA0_IRQ_Handler);
         break;

      case DISP_LCD_REG_COMPLETE_INT:
         //wake_up_interruptible(&_dsi_reg_update_wq);
         disp_register_irq(DISP_MODULE_MUTEX, _LCD_MUTEX_IRQ_Handler);
         break;

      default:
         return LCD_STATUS_ERROR;
   }
#endif
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
    _lcdContext.pIntCallback = pCB;
    return LCD_STATUS_OK;
}


// -------------------- LCD Controller Interface --------------------
LCD_STATUS LCD_ConfigParallelIF(LCD_IF_ID id,
                                                              LCD_IF_PARALLEL_BITS ifDataWidth,
                                                              LCD_IF_PARALLEL_CLK_DIV clkDivisor,
                                                              UINT32 writeSetup,
                                                              UINT32 writeHold,
                                                              UINT32 writeWait,
                                                              UINT32 readSetup,
                                                              UINT32 readHold,
                                                              UINT32 readLatency,
                                                              UINT32 waitPeriod,
                                                              UINT32 chw)
{
   ASSERT(id <= LCD_IF_PARALLEL_2);
   ASSERT(writeSetup <= 16U);
   ASSERT(writeHold <= 16U);
   ASSERT(writeWait <= 64U);
   ASSERT(readSetup <= 16U);
   ASSERT(readHold <= 16U);
   ASSERT(readLatency <= 64U);
   ASSERT(chw <= 16U);
   
   if (0 == writeHold)   writeHold = 1;
   if (0 == writeWait)   writeWait = 1;
   if (0 == readLatency) readLatency = 1;
   
   _WaitForEngineNotBusy();
   
   // (1) Config Data Width
   {
      LCD_REG_PCNFDW pcnfdw = LCD_REG->PARALLEL_DW;
      
      switch(id)
      {
         case LCD_IF_PARALLEL_0: 
            pcnfdw.PCNF0_DW = (UINT32)ifDataWidth; 
            pcnfdw.PCNF0_CHW = chw;
            break;

         case LCD_IF_PARALLEL_1: 
            pcnfdw.PCNF1_DW = (UINT32)ifDataWidth; 
            pcnfdw.PCNF1_CHW = chw;
            break;
            
         case LCD_IF_PARALLEL_2: 
            pcnfdw.PCNF2_DW = (UINT32)ifDataWidth; 
            pcnfdw.PCNF2_CHW = chw;
            break;

         default : 
            ASSERT(0);
      };
      
      LCD_OUTREG32(&LCD_REG->PARALLEL_DW, AS_UINT32(&pcnfdw));
   }

   // (2) Config Timing
   {
      UINT32 i;
      LCD_REG_PCNF config;
      
      i = (UINT32)id - LCD_IF_PARALLEL_0;
      config = LCD_REG->PARALLEL_CFG[i];
      
      config.C2WS = writeSetup;
      config.C2WH = writeHold - 1;
      config.WST  = writeWait - 1;
      config.C2RS = readSetup;
      config.C2RH = readHold;
      config.RLT  = readLatency - 1;
      
      LCD_OUTREG32(&LCD_REG->PARALLEL_CFG[i], AS_UINT32(&config));
   }
   
   return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigIfFormat(LCD_IF_FMT_COLOR_ORDER order,
                              LCD_IF_FMT_TRANS_SEQ transSeq,
                              LCD_IF_FMT_PADDING padding,
                              LCD_IF_FORMAT format,
                              LCD_IF_WIDTH busWidth)
{
   LCD_REG_WROI_CON ctrl = LCD_REG->WROI_CONTROL;
   
   ctrl.RGB_ORDER  = order;
   ctrl.BYTE_ORDER = transSeq;
   ctrl.PADDING    = padding;
   ctrl.DATA_FMT   = (UINT32)format;
   ctrl.IF_FMT   = (UINT32)busWidth;
   ctrl.IF_24 = 0;

   if(busWidth == LCD_IF_WIDTH_24_BITS)
   {
      ctrl.IF_24 = 1;
   }
   LCD_OUTREG32(&LCD_REG->WROI_CONTROL, AS_UINT32(&ctrl));
   
   return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigSerialIF(LCD_IF_ID id,
                              LCD_IF_SERIAL_BITS bits,
                              UINT32 three_wire,
                              UINT32 sdi,
                              BOOL   first_pol,
                              BOOL   sck_def,
                              UINT32 div2,
                              UINT32 hw_cs,
                              UINT32 css,
                              UINT32 csh,
                              UINT32 rd_1st,
                              UINT32 rd_2nd,
                              UINT32 wr_1st,
                              UINT32 wr_2nd)
{
   LCD_REG_SCNF config;
   LCD_REG_SIF_TIMING sif_timing;
   unsigned int offset = 0;
   unsigned int sif_id = 0;
   
   ASSERT(id >= LCD_IF_SERIAL_0 && id <= LCD_IF_SERIAL_1);
   
   _WaitForEngineNotBusy();
   
   memset(&config, 0, sizeof(config));
   
   if(id == LCD_IF_SERIAL_1){
      offset = 8;
      sif_id = 1;
   }
   
   LCD_MASKREG32(&config, 0x07 << offset, bits << offset);
   LCD_MASKREG32(&config, 0x08 << offset, three_wire << (offset + 3));
   LCD_MASKREG32(&config, 0x10 << offset, sdi << (offset + 4));
   LCD_MASKREG32(&config, 0x20 << offset, first_pol << (offset + 5));
   LCD_MASKREG32(&config, 0x40 << offset, sck_def << (offset + 6));
   LCD_MASKREG32(&config, 0x80 << offset, div2 << (offset + 7));
   
   config.HW_CS = hw_cs;
   //	config.SIZE_0 = bits;
   LCD_OUTREG32(&LCD_REG->SERIAL_CFG, AS_UINT32(&config));
   
   sif_timing.WR_2ND = wr_2nd;
   sif_timing.WR_1ST = wr_1st;
   sif_timing.RD_2ND = rd_2nd;
   sif_timing.RD_1ST = rd_1st;
   sif_timing.CSH = csh;
   sif_timing.CSS = css;
   
   LCD_OUTREG32(&LCD_REG->SIF_TIMING[sif_id], AS_UINT32(&sif_timing));
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetSwReset(void)
{
   OUTREGBIT(LCD_REG_START,LCD_REG->START,RESET,1);
   OUTREGBIT(LCD_REG_START,LCD_REG->START,RESET,0);

   return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetResetSignal(BOOL high)
{
   LCD_REG->RESET = high ? 1 : 0;
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetChipSelect(BOOL high)
{
   LCD_REG->SIF_CS.CS0 = high ? 1 : 0;
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_ConfigDSIIfFormat(LCD_DSI_IF_FMT_COLOR_ORDER order,
                              LCD_DSI_IF_FMT_TRANS_SEQ transSeq,
                              LCD_DSI_IF_FMT_PADDING padding,
                              LCD_DSI_IF_FORMAT format,
                              UINT32 packet_size,
                              bool DC_DSI)
{
   //MT6583 not support
   return LCD_STATUS_OK;
}


// -------------------- Command Queue --------------------

LCD_STATUS LCD_CmdQueueEnable(BOOL enabled)
{
   LCD_REG_WROI_CON ctrl;
   
   //    _WaitForEngineNotBusy();
   
   ctrl = LCD_REG->WROI_CONTROL;
   ctrl.ENC = enabled ? 1 : 0;
   LCD_OUTREG32(&LCD_REG->WROI_CONTROL, AS_UINT32(&ctrl));
   
   return LCD_STATUS_OK;
}

LCD_STATUS LCD_CmdQueueWrite(UINT32 *cmds, UINT32 cmdCount)
{
    LCD_REG_WROI_CON ctrl;
    UINT32 i;

   ASSERT(cmdCount < ARY_SIZE(LCD_REG->CMDQ));
    
//    _WaitForEngineNotBusy();
    ctrl = LCD_REG->WROI_CONTROL;
    ctrl.COMMAND = cmdCount - 1;
    LCD_OUTREG32(&LCD_REG->WROI_CONTROL, AS_UINT32(&ctrl));

    for (i = 0; i < cmdCount; ++ i)
    {
        LCD_REG->CMDQ[i] = cmds[i];
    }

    return LCD_STATUS_OK;
}


// -------------------- Layer Configurations --------------------
LCD_STATUS LCD_LayerEnable(LCD_LAYER_ID id, BOOL enable)
{
   //MT6583 not support, if be called, it should call OVL function
   if(LCD_LAYER_ALL == id)return LCD_STATUS_OK;

   cached_layer_config[id].layer_en= enable;
   cached_layer_config[id].isDirty = true;

   return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigDither(int lrs, int lgs, int lbs, int dbr, int dbg, int dbb)
{
   /*
      LCD_REG_DITHER_CON ctrl;
      
      //	_WaitForEngineNotBusy();
      
      ctrl = LCD_REG->DITHER_CON;
      
      ctrl.LFSR_R_SEED = lrs;
      ctrl.LFSR_G_SEED = lgs;
      ctrl.LFSR_B_SEED = lbs;
      ctrl.DB_R = dbr;
      ctrl.DB_G = dbg;
      ctrl.DB_B = dbb;
      
      LCD_OUTREG32(&LCD_REG->DITHER_CON, AS_UINT32(&ctrl));
   */

   return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerEnableDither(LCD_LAYER_ID id, UINT32 enable)
{
   ASSERT(id < LCD_LAYER_NUM || LCD_LAYER_ALL == id);
   
   return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerSetAddress(LCD_LAYER_ID id, UINT32 address)
{
   cached_layer_config[id].addr = address;
   cached_layer_config[id].isDirty = true;

   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerEnableTdshp(LCD_LAYER_ID id, UINT32 en)
{
    cached_layer_config[id].isTdshp = en;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetNextBuffIdx(LCD_LAYER_ID id, INT32 idx)
{
    cached_layer_config[id].buff_idx = idx;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerGetInfo(LCD_LAYER_ID id, UINT32 *enabled, INT32 *curr_idx, INT32 *next_idx)
{
    *enabled = realtime_layer_config[id].layer_en;
    *curr_idx = realtime_layer_config[id].buff_idx;
    *next_idx = cached_layer_config[id].buff_idx;
    return LCD_STATUS_OK;
}


UINT32 LCD_DisableAllLayer(UINT32 vram_start, UINT32 vram_end)
{
    int id;
    int layer_enable = 0;
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "LCD", "%s(%d, %d)\n", __func__, vram_start, vram_end);

    for (id = 0; id < DDP_OVL_LAYER_MUN; id++) {
        if (cached_layer_config[id].layer_en == 0)
            continue;

        if (cached_layer_config[id].addr >= vram_start &&
            cached_layer_config[id].addr < vram_end)
        {
            DISP_LOG_PRINT(ANDROID_LOG_INFO, "LCD", "  not disable(%d)\n", id);
            layer_enable |= (1 << id);
            continue;
        }

        DISP_LOG_PRINT(ANDROID_LOG_INFO, "LCD", "  disable(%d)\n", id);
        cached_layer_config[id].layer_en = 0;
        cached_layer_config[id].isDirty = true;
    }
    return layer_enable;
}


UINT32 LCD_LayerGetAddress(LCD_LAYER_ID id)
{
    return cached_layer_config[id].addr;
}


LCD_STATUS LCD_LayerSetSize(LCD_LAYER_ID id, UINT32 width, UINT32 height)
{   
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetPitch(LCD_LAYER_ID id, UINT32 pitch)
{
    cached_layer_config[id].src_pitch = pitch;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetOffset(LCD_LAYER_ID id, UINT32 x, UINT32 y)
{
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetWindowOffset(LCD_LAYER_ID id, UINT32 x, UINT32 y)
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerSetFormat(LCD_LAYER_ID id, LCD_LAYER_FORMAT format)
{
    cached_layer_config[id].fmt = format;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerEnableByteSwap(LCD_LAYER_ID id, BOOL enable)
{
    //mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetRotation(LCD_LAYER_ID id, LCD_LAYER_ROTATION rotation)
{
    //mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetAlphaBlending(LCD_LAYER_ID id, BOOL enable, UINT8 alpha)
{
    cached_layer_config[id].alpha = (unsigned char)alpha;
    cached_layer_config[id].aen = enable;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetSourceColorKey(LCD_LAYER_ID id, BOOL enable, UINT32 colorKey)
{
    cached_layer_config[id].key = colorKey;
    cached_layer_config[id].keyEn = enable;
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerSetDestColorKey(LCD_LAYER_ID id, BOOL enable, UINT32 colorKey)
{
    //mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_Layer3D_Enable(LCD_LAYER_ID id, BOOL enable)
{
    //mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_Layer3D_R1st(LCD_LAYER_ID id, BOOL r_first)
{
    //mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_Layer3D_landscape(LCD_LAYER_ID id, BOOL landscape)
{
    //mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSet3D(LCD_LAYER_ID id, BOOL enable, BOOL r_first, BOOL landscape)
{
    //mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


BOOL LCD_Is3DEnabled(void)
{
    //mt6588 OVL driver not yet export this function
    return FALSE;
}


BOOL LCD_Is3DLandscapeMode(void)
{
    //mt6588 OVL driver not yet export this function
    return FALSE;
}


// -------------------- HW Trigger Configurations --------------------
LCD_STATUS LCD_LayerSetTriggerMode(LCD_LAYER_ID id, LCD_LAYER_TRIGGER_MODE mode)
{
    //mt6583 not support
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_EnableHwTrigger(BOOL enable)
{
    //mt6583 not support
    return LCD_STATUS_OK;
}


// -------------------- ROI Window Configurations --------------------

LCD_STATUS LCD_SetBackgroundColor(UINT32 bgColor)
{
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetRoiWindow(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
    LCD_REG_SIZE size;
    
    size.WIDTH = (UINT16)width;
    size.HEIGHT = (UINT16)height;

    LCD_OUTREG32(&LCD_REG->WROI_SIZE, AS_UINT32(&size));
    _lcdContext.roiWndSize = size;
        
    return LCD_STATUS_OK;
}

// DSI Related Configurations
LCD_STATUS LCD_EnableDCtoDsi(BOOL enable)
{
    //mt6583 not support
    return LCD_STATUS_OK;
}

// -------------------- Output to Memory Configurations --------------------

LCD_STATUS LCD_SetOutputMode(LCD_OUTPUT_MODE mode)
{
    //mt6583 not support, call OVL
    return LCD_STATUS_OK;    
}
EXPORT_SYMBOL(LCD_SetOutputMode);

LCD_STATUS LCD_SetOutputAlpha(unsigned int alpha)
{
    //mt6583 not support
    return LCD_STATUS_OK;    
}

LCD_STATUS LCD_WaitDPIIndication(BOOL enable)
{
    return LCD_STATUS_OK;
}
EXPORT_SYMBOL(LCD_WaitDPIIndication);


LCD_STATUS LCD_FBSetFormat(LCD_FB_FORMAT format)
{
    return LCD_STATUS_OK;
}
EXPORT_SYMBOL(LCD_FBSetFormat);

LCD_STATUS LCD_FBSetPitch(UINT32 pitchInByte)
{
    return LCD_STATUS_OK;
}
EXPORT_SYMBOL(LCD_FBSetPitch);

LCD_STATUS LCD_FBEnable(LCD_FB_ID id, BOOL enable)
{
    return LCD_STATUS_OK;
}
EXPORT_SYMBOL(LCD_FBEnable);

LCD_STATUS LCD_FBReset(void)
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_FBSetAddress(LCD_FB_ID id, UINT32 address)
{
    return LCD_STATUS_OK;
}
EXPORT_SYMBOL(LCD_FBSetAddress);

LCD_STATUS LCD_FBSetStartCoord(UINT32 x, UINT32 y)
{
    return LCD_STATUS_OK;
}
EXPORT_SYMBOL(LCD_FBSetStartCoord);
// -------------------- Color Matrix --------------------

LCD_STATUS LCD_EnableColorMatrix(LCD_IF_ID id, BOOL enable)
{
    return LCD_STATUS_OK;
}

/** Input: const S2_8 mat[9], fixed ponit signed 2.8 format
           |                      |
           | mat[0] mat[1] mat[2] |
           | mat[3] mat[4] mat[5] |
           | mat[6] mat[7] mat[8] |
           |                      |
*/
LCD_STATUS LCD_SetColorMatrix(const S2_8 mat[9])
{
    return LCD_STATUS_OK;
}

// -------------------- Tearing Control --------------------

LCD_STATUS LCD_TE_Enable(BOOL enable)
{
    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
 
    tecon.ENABLE = enable ? 1 : 0;
    LCD_OUTREG32(&LCD_REG->TEARING_CFG, AS_UINT32(&tecon));
    
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_TE_SetMode(LCD_TE_MODE mode)
{
    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
 
    tecon.MODE = (LCD_TE_MODE_VSYNC_OR_HSYNC == mode) ? 1 : 0;
    LCD_OUTREG32(&LCD_REG->TEARING_CFG, AS_UINT32(&tecon));
    
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_TE_SetEdgePolarity(BOOL polarity)
{
    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
 
    tecon.EDGE_SEL = (polarity ? 1 : 0);
    LCD_OUTREG32(&LCD_REG->TEARING_CFG, AS_UINT32(&tecon));
    
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_TE_ConfigVHSyncMode(UINT32 hsDelayCnt,
                                   UINT32 vsWidthCnt,
                                   LCD_TE_VS_WIDTH_CNT_DIV vsWidthCntDiv)
{
/*    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
    tecon.HS_MCH_CNT = (hsDelayCnt ? hsDelayCnt - 1 : 0);
    tecon.VS_WLMT = (vsWidthCnt ? vsWidthCnt - 1 : 0);
    tecon.VS_CNT_DIV = vsWidthCntDiv;
    LCD_OUTREG32(&LCD_REG->TEARING_CFG, AS_UINT32(&tecon));
*/

     return LCD_STATUS_OK;
}

// -------------------- Operations --------------------

LCD_STATUS LCD_SelectWriteIF(LCD_IF_ID id)
{
    LCD_REG_CMD_ADDR cmd_addr;
    LCD_REG_DAT_ADDR dat_addr;
    
    switch(id)
    {
        case LCD_IF_PARALLEL_0 : cmd_addr.addr = 0; break;
        case LCD_IF_PARALLEL_1 : cmd_addr.addr = 2; break;
        case LCD_IF_PARALLEL_2 : cmd_addr.addr = 4; break;
        case LCD_IF_SERIAL_0   : cmd_addr.addr = 8; break;
        case LCD_IF_SERIAL_1   : cmd_addr.addr = 0xA; break;
        default:
            ASSERT(0);
    }
    dat_addr.addr = cmd_addr.addr + 1;
    LCD_OUTREG16(&LCD_REG->WROI_CMD_ADDR, AS_UINT16(&cmd_addr));
    LCD_OUTREG16(&LCD_REG->WROI_DATA_ADDR, AS_UINT16(&dat_addr));
    
    return LCD_STATUS_OK;
}


__inline static void _LCD_WriteIF(DWORD baseAddr, UINT32 value, LCD_IF_MCU_WRITE_BITS bits)
{
    switch(bits)
    {
        case LCD_IF_MCU_WRITE_8BIT :
            LCD_OUTREG8(baseAddr, value);
            break;
        
        case LCD_IF_MCU_WRITE_16BIT :
            LCD_OUTREG16(baseAddr, value);
            break;
        
        case LCD_IF_MCU_WRITE_32BIT :
            LCD_OUTREG32(baseAddr, value);
            break;
        
        default:
            ASSERT(0);
    }
}


LCD_STATUS LCD_WriteIF(LCD_IF_ID id, LCD_IF_A0_MODE a0,
                       UINT32 value, LCD_IF_MCU_WRITE_BITS bits)
{
    DWORD baseAddr = 0;
    
    switch(id)
    {
        case LCD_IF_PARALLEL_0 : baseAddr = (DWORD)&LCD_REG->PCMD0; break;
        case LCD_IF_PARALLEL_1 : baseAddr = (DWORD)&LCD_REG->PCMD1; break;
        case LCD_IF_PARALLEL_2 : baseAddr = (DWORD)&LCD_REG->PCMD2; break;
        case LCD_IF_SERIAL_0   : baseAddr = (DWORD)&LCD_REG->SCMD0; break;
        case LCD_IF_SERIAL_1   : baseAddr = (DWORD)&LCD_REG->SCMD1; break;
        default:
            ASSERT(0);
    }
    
    if (LCD_IF_A0_HIGH == a0)
    {
        baseAddr += LCD_A0_HIGH_OFFSET;
    }
    
    _LCD_WriteIF(baseAddr, value, bits);
    
    return LCD_STATUS_OK;
}


__inline static UINT32 _LCD_ReadIF(DWORD baseAddr, LCD_IF_MCU_WRITE_BITS bits)
{
    switch(bits)
    {
        case LCD_IF_MCU_WRITE_8BIT :
            return (UINT32)INREG8(baseAddr);
        
        case LCD_IF_MCU_WRITE_16BIT :
            return (UINT32)INREG16(baseAddr);
        
        case LCD_IF_MCU_WRITE_32BIT :
            return (UINT32)INREG32(baseAddr);
        
        default:
            ASSERT(0);
    }
}


LCD_STATUS LCD_ReadIF(LCD_IF_ID id, LCD_IF_A0_MODE a0,
                      UINT32 *value, LCD_IF_MCU_WRITE_BITS bits)
{
    DWORD baseAddr = 0;
    
    if (NULL == value) return LCD_STATUS_ERROR;
    
    switch(id)
    {
        case LCD_IF_PARALLEL_0 : baseAddr = (DWORD)&LCD_REG->PCMD0; break;
        case LCD_IF_PARALLEL_1 : baseAddr = (DWORD)&LCD_REG->PCMD1; break;
        case LCD_IF_PARALLEL_2 : baseAddr = (DWORD)&LCD_REG->PCMD2; break;
        case LCD_IF_SERIAL_0   : baseAddr = (DWORD)&LCD_REG->SCMD0; break;
        case LCD_IF_SERIAL_1   : baseAddr = (DWORD)&LCD_REG->SCMD1; break;
        default:
            ASSERT(0);
    }
    
    if (LCD_IF_A0_HIGH == a0)
    {
        baseAddr += LCD_A0_HIGH_OFFSET;
    }
    
    *value = _LCD_ReadIF(baseAddr, bits);
    
    return LCD_STATUS_OK;
}


bool LCD_IsLayerEnable(LCD_LAYER_ID id)
{
    ASSERT(id <= LCD_LAYER_NUM);
    return (bool)(cached_layer_config[id].layer_en);
}


LCD_STATUS LCD_Dump_Layer_Info(void)
{
    int i=0;

    printk("LCD_Dump_Layer_Info: \n");
    for(i=0;i<4;i++)
    {
        printk("layer=%d, en=%d, src=%d, fmt=%d, addr=0x%x, (%d, %d, %d, %d), pitch=%d, keyEn=%d, key=0x%x, aen=%d, alpha=0x%x, isTdshp=%d, curr_buff_idx=%d, next_buff_idx=%d\n", 
                   cached_layer_config[i].layer,
                   cached_layer_config[i].layer_en,
                   cached_layer_config[i].source,
                   cached_layer_config[i].fmt,
                   cached_layer_config[i].addr, 
                   cached_layer_config[i].dst_x,
                   cached_layer_config[i].dst_y,
                   cached_layer_config[i].dst_w,
                   cached_layer_config[i].dst_h,
                   cached_layer_config[i].src_pitch,
                   cached_layer_config[i].keyEn,
                   cached_layer_config[i].key, 
                   cached_layer_config[i].aen, 
                   cached_layer_config[i].alpha,  
                   cached_layer_config[i].isTdshp,
                   realtime_layer_config[i].buff_idx,
                   cached_layer_config[i].buff_idx );
    }
    
    return LCD_STATUS_OK;
}


extern struct mutex OverlaySettingMutex;
LCD_STATUS LCD_StartTransfer(BOOL blocking, BOOL isMutexLocked)
{
    DBI_DRV_FUNC("[%s]:enter \n", __func__);

    _WaitForEngineNotBusy();
    DBG_OnTriggerLcd();

    if (!isMutexLocked)
        disp_path_get_mutex();
    mutex_lock(&OverlaySettingMutex);

    LCD_ConfigOVL();

    LCD_OUTREG32(&LCD_REG->START, 0);
    LCD_OUTREG32(&LCD_REG->START, (1 << 15));

    mutex_unlock(&OverlaySettingMutex);
    if (!isMutexLocked)
        disp_path_release_mutex();

    if (blocking)
    {
        _WaitForLCDEngineComplete();
    }

    return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigOVL()
{
    unsigned int i;
    unsigned int dirty = 0;

    DBI_DRV_FUNC("[%s]:enter \n", __func__);

    MMProfileLogEx(MTKFB_MMP_Events.ConfigOVL, MMProfileFlagStart, 1, 0);
    for(i = 0;i<DDP_OVL_LAYER_MUN;i++)
    {
        if (cached_layer_config[i].isDirty)
        {
            dirty |= 1<<i;
            disp_path_config_layer(&cached_layer_config[i]);
            cached_layer_config[i].isDirty = false;
        }
    }
    MMProfileLogEx(MTKFB_MMP_Events.ConfigOVL, MMProfileFlagEnd, 1, 0);

    return LCD_STATUS_OK;
}


// -------------------- Retrieve Information --------------------
LCD_OUTPUT_MODE LCD_GetOutputMode(void)
{
    return _lcdContext.outputMode;
}


LCD_STATE  LCD_GetState(void)
{
    if (!s_isLcdPowerOn)
    {
        return LCD_STATE_POWER_OFF;
    }
    
    if (_IsEngineBusy())
    {
        return LCD_STATE_BUSY;
    }
    
    return LCD_STATE_IDLE;
}


LCD_STATUS LCD_DumpRegisters(void)
{
    UINT32 i;
    
    DISP_LOG_PRINT(ANDROID_LOG_WARN, "LCD", "---------- Start dump LCD registers ----------\n");
    
    for (i = 0; i < offsetof(LCD_REGS, GMC_ULTRA_TH); i += 4)
    {
        printk("LCD+%04x : 0x%08x\n", i, INREG32(DISP_DBI_BASE + i));
    }
    
    return LCD_STATUS_OK;
}

void LCD_DumpLayer()
{
    unsigned int i;

    for(i=0;i<4;i++){
        printk("LayerInfo in LCD driver, layer=%d,layer_en=%d, source=%d, fmt=%d, addr=0x%x, x=%d, y=%d \n\
                   w=%d, h=%d, pitch=%d, keyEn=%d, key=%d, aen=%d, alpha=%d \n ", 
                   cached_layer_config[i].layer,   // layer
                   cached_layer_config[i].layer_en,
                   cached_layer_config[i].source,   // data source (0=memory)
                   cached_layer_config[i].fmt, 
                   cached_layer_config[i].addr, // addr 
                   cached_layer_config[i].dst_x,  // x
                   cached_layer_config[i].dst_y,  // y
                   cached_layer_config[i].dst_w, // width
                   cached_layer_config[i].dst_h, // height
                   cached_layer_config[i].src_pitch, //pitch, pixel number
                   cached_layer_config[i].keyEn,  //color key
                   cached_layer_config[i].key,  //color key
                   cached_layer_config[i].aen, // alpha enable
                   cached_layer_config[i].alpha);	
    }
}


#if !defined(MTK_M4U_SUPPORT)
static unsigned long v2p(unsigned long va)
{
   unsigned long pageOffset = (va & (PAGE_SIZE - 1));
   pgd_t *pgd;
   pmd_t *pmd;
   pte_t *pte;
   unsigned long pa;
   
   pgd = pgd_offset(current->mm, va); /* what is tsk->mm */
   pmd = pmd_offset(pgd, va);
   pte = pte_offset_map(pmd, va);
   pa = (pte_val(*pte) & (PAGE_MASK)) | pageOffset;

   return pa;
}
#endif

LCD_STATUS LCD_Get_VideoLayerSize(unsigned int id, unsigned int *width, unsigned int *height)
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_Capture_Layerbuffer(unsigned int layer_id, unsigned int pvbuf, unsigned int bpp)
{
    return LCD_STATUS_OK;   
}


LCD_STATUS LCD_Capture_Videobuffer(unsigned int pvbuf, unsigned int bpp, unsigned int video_rotation)
{
    return LCD_STATUS_OK;   
}

#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

LCD_STATUS LCD_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp)
{
   return LCD_STATUS_OK;    
}

LCD_STATUS LCD_FMDesense_Query()
{
   return LCD_STATUS_OK;
}

LCD_STATUS LCD_FM_Desense(LCD_IF_ID id, unsigned long freq)
{
   UINT32 a,b;
   UINT32 c,d;
   UINT32 wst,c2wh,chw,write_cycles;
   LCD_REG_PCNF config;
   //	LCD_REG_WROI_CON ctrl;    
   LCD_REG_PCNFDW pcnfdw;
   
   LCD_OUTREG32(&config, AS_UINT32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));
   DBI_DRV_INFO("[enter LCD_FM_Desense]:parallel IF = 0x%x, ctrl = 0x%x\n",
   INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));   
   wst = config.WST;
   c2wh = config.C2WH;
   // Config Delay Between Commands
   //	LCD_OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));
   LCD_OUTREG32(&pcnfdw, AS_UINT32(&LCD_REG->PARALLEL_DW));
   
   switch(id)
   {
      case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
      case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
      case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
      default : ASSERT(0);
   }
   
   a = 13000 - freq * 10 - 20;
   b = 13000 - freq * 10 + 20;
   write_cycles = wst + c2wh + chw + 2;//this is 6573 E1, E2 will change
   c = (a * write_cycles)%13000;
   d = (b * write_cycles)%13000;
   a = (a * write_cycles)/13000;
   b = (b * write_cycles)/13000;
   
   if((b > a)||(c == 0)||(d == 0)){//need modify setting to avoid interference
      DBI_DRV_INFO("[LCD_FM_Desense] need to modify lcd setting, freq = %ld\n",freq);
      wst -= wst_step_LCD;
      wst_step_LCD = 0 - wst_step_LCD;
      
      config.WST = wst;
      LCD_WaitForNotBusy();
      LCD_OUTREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id], AS_UINT32(&config));
   }
   else{
      DBI_DRV_INFO("[LCD_FM_Desense] not need to modify lcd setting, freq = %ld\n",freq);
   }
   DBI_DRV_INFO("[leave LCD_FM_Desense]:parallel = 0x%x\n", INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));   

   return LCD_STATUS_OK;  
}

LCD_STATUS LCD_Reset_WriteCycle(LCD_IF_ID id)
{
   LCD_REG_PCNF config;
   UINT32 wst;

   DBI_DRV_INFO("[enter LCD_Reset_WriteCycle]:parallel = 0x%x\n", INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));   

   if(wst_step_LCD > 0){//have modify lcd setting, so when fm turn off, we must decrease wst to default setting
      DBI_DRV_INFO("[LCD_Reset_WriteCycle] need to reset lcd setting\n");
      LCD_OUTREG32(&config, AS_UINT32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));
      wst = config.WST;
      wst -= wst_step_LCD;
      wst_step_LCD = 0 - wst_step_LCD;
      
      config.WST = wst;
      LCD_WaitForNotBusy();
      LCD_OUTREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id], AS_UINT32(&config));
   }
   else{
      DBI_DRV_INFO("[LCD_Reset_WriteCycle] parallel is default setting, not need to reset it\n");
   }
   DBI_DRV_INFO("[leave LCD_Reset_WriteCycle]:parallel = 0x%x\n", INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));   

   return LCD_STATUS_OK; 
}

LCD_STATUS LCD_Get_Default_WriteCycle(LCD_IF_ID id, unsigned int *write_cycle)
{
   UINT32 wst,c2wh,chw;
   LCD_REG_PCNF config;
   //	LCD_REG_WROI_CON ctrl;    
   LCD_REG_PCNFDW pcnfdw;
   
   if(is_get_default_write_cycle){
      *write_cycle = default_write_cycle;
      return LCD_STATUS_OK;
   }
   
   LCD_OUTREG32(&config, AS_UINT32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));
   DBI_DRV_INFO("[enter LCD_Get_Default_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n",
   INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));   
   wst = config.WST;
   c2wh = config.C2WH;
   // Config Delay Between Commands
   //	LCD_OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));
   LCD_OUTREG32(&pcnfdw, AS_UINT32(&LCD_REG->PARALLEL_DW));

   switch(id)
   {
      case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
      case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
      case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
      default : ASSERT(0);
   }
   *write_cycle = wst + c2wh + chw + 2;
   default_write_cycle = *write_cycle;
   default_wst = wst;
   is_get_default_write_cycle = TRUE;
   DBI_DRV_INFO("[leave LCD_Get_Default_WriteCycle]:Default_Write_Cycle = %d\n", *write_cycle);   

   return LCD_STATUS_OK;  
}

LCD_STATUS LCD_Get_Current_WriteCycle(LCD_IF_ID id, unsigned int *write_cycle)
{
   UINT32 wst,c2wh,chw;
   LCD_REG_PCNF config;
   //	LCD_REG_WROI_CON ctrl;       
   LCD_REG_PCNFDW pcnfdw;
   
   LCD_OUTREG32(&config, AS_UINT32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));
   DBI_DRV_INFO("[enter LCD_Get_Current_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n",
   INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));   
   wst = config.WST;
   c2wh = config.C2WH;
   // Config Delay Between Commands
   //	LCD_OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));
   LCD_OUTREG32(&pcnfdw, AS_UINT32(&LCD_REG->PARALLEL_DW));
   switch(id)
   {
      case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
      case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
      case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
      default : ASSERT(0);
   }
   
   *write_cycle = wst + c2wh + chw + 2;//this is 6573 E1, E2 will change
   DBI_DRV_INFO("[leave LCD_Get_Current_WriteCycle]:Default_Write_Cycle = %d\n", *write_cycle);   

   return LCD_STATUS_OK;  
}

LCD_STATUS LCD_Change_WriteCycle(LCD_IF_ID id, unsigned int write_cycle)
{
   UINT32 wst;
   LCD_REG_PCNF config;
   
   LCD_OUTREG32(&config, AS_UINT32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));
   DBI_DRV_INFO("[enter LCD_Change_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n",
   INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]),INREG32(&LCD_REG->WROI_CONTROL));   
   
   DBI_DRV_INFO("[LCD_Change_WriteCycle] modify lcd setting\n");
   wst = write_cycle - default_write_cycle + default_wst;
   
   config.WST = wst;
   LCD_WaitForNotBusy();
   LCD_OUTREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id], AS_UINT32(&config));
   DBI_DRV_INFO("[leave LCD_Change_WriteCycle]:parallel = 0x%x\n", INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]));   

   return LCD_STATUS_OK;  
}

#if defined(MTK_M4U_SUPPORT)
LCD_STATUS LCD_InitM4U()
{
#ifdef MTK_LCDC_ENABLE_M4U
   M4U_PORT_STRUCT M4uPort;
   DBI_DRV_INFO("[LCDC driver]%s\n", __func__);

   M4uPort.ePortID = M4U_PORT_LCD_R;
   M4uPort.Virtuality = 1; 					   
   M4uPort.Security = 0;
   M4uPort.Distance = 1;
   M4uPort.Direction = 0;
   
   m4u_config_port(&M4uPort); //cloud
   
   M4uPort.ePortID = M4U_PORT_LCD_W;
   M4uPort.Virtuality = 1; 					   
   M4uPort.Security = 0;
   M4uPort.Distance = 1;
   M4uPort.Direction = 0;
   
   m4u_config_port(&M4uPort); //cloud
   //    _m4u_lcdc_func.m4u_dump_reg(M4U_CLNTMOD_LCDC);
#endif

   return LCD_STATUS_OK;
}

LCD_STATUS LCD_AllocUIMva(unsigned int va, unsigned int *mva, unsigned int size)
{
#ifdef MTK_LCDC_ENABLE_M4U
   m4u_alloc_mva(M4U_CLNTMOD_LCDC_UI, va, size, false, false, mva);
#endif

   return LCD_STATUS_OK;
}

LCD_STATUS LCD_AllocOverlayMva(unsigned int va, unsigned int *mva, unsigned int size)
{
#ifdef MTK_LCDC_ENABLE_M4U
    m4u_alloc_mva(M4U_CLNTMOD_LCDC_UI, va, size, mva);
    m4u_insert_tlb_range(M4U_CLNTMOD_LCDC_UI,
                                     *mva,
                                     *mva + size - 1,
                                     RT_RANGE_HIGH_PRIORITY,
                                     0);
#endif

   return LCD_STATUS_OK;
}

LCD_STATUS LCD_DeallocMva(unsigned int va, unsigned int mva, unsigned int size)
{
#ifdef MTK_LCDC_ENABLE_M4U
    if (!_m4u_lcdc_func.isInit)
    {
        DBI_DRV_WRAN("[TV]Error, M4U has not init func for TV-out\n");
        return LCD_STATUS_ERROR;
    }

    _m4u_lcdc_func.m4u_invalid_tlb_range(M4U_CLNTMOD_LCDC_UI, mva, mva + size - 1);
    _m4u_lcdc_func.m4u_dealloc_mva(M4U_CLNTMOD_LCDC_UI, va, size, mva);
#endif

    return LCD_STATUS_OK;
}

LCD_STATUS LCD_M4UPowerOn(void)
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_M4UPowerOff(void)
{
    return LCD_STATUS_OK;
}

int m4u_alloc_mva_stub(M4U_MODULE_ID_ENUM eModuleID, const unsigned int BufAddr, const unsigned int BufSize, unsigned int *pRetMVABuf)
{
    return m4u_alloc_mva(eModuleID, BufAddr, BufSize, 0, 0, pRetMVABuf);
}
EXPORT_SYMBOL(m4u_alloc_mva_stub);
  
int m4u_dealloc_mva_stub(M4U_MODULE_ID_ENUM eModuleID, const unsigned int BufAddr, const unsigned int BufSize, const unsigned int MVA)
{
    return m4u_dealloc_mva(eModuleID, BufAddr, BufSize, MVA);
}
EXPORT_SYMBOL(m4u_dealloc_mva_stub);

int m4u_insert_tlb_range_stub(M4U_MODULE_ID_ENUM eModuleID, 
                  unsigned int MVAStart, 
                  const unsigned int MVAEnd, 
                  unsigned int entryCount)
{
    return m4u_insert_seq_range(eModuleID, MVAStart, MVAEnd, entryCount);				  
}
EXPORT_SYMBOL(m4u_insert_tlb_range_stub);
                        

int m4u_invalid_tlb_range_stub(M4U_MODULE_ID_ENUM eModuleID, 
                  unsigned int MVAStart, 
                  unsigned int MVAEnd)
{
    return m4u_invalid_seq_range(eModuleID, MVAStart, MVAEnd);				  
}
EXPORT_SYMBOL(m4u_invalid_tlb_range_stub);
             
             
int m4u_invalid_tlb_all_stub(M4U_MODULE_ID_ENUM eModuleID);  
int m4u_manual_insert_entry_stub(M4U_MODULE_ID_ENUM eModuleID, unsigned int EntryMVA, bool Lock); 
  
int m4u_config_port_stub(M4U_PORT_STRUCT* pM4uPort)
{
    return m4u_config_port(pM4uPort);
}
EXPORT_SYMBOL(m4u_config_port_stub);

LCD_STATUS LCD_M4U_On(bool enable)
{
#ifdef MTK_LCDC_ENABLE_M4U
   M4U_PORT_STRUCT M4uPort;

   DBI_DRV_INFO("[LCDC driver]%s\n", __func__);
   
   M4uPort.ePortID = M4U_PORT_LCD_R;
   M4uPort.Virtuality = enable; 					   
   M4uPort.Security = 0;
   M4uPort.Distance = 1;
   M4uPort.Direction = 0;
   
   m4u_config_port(&M4uPort);
#endif

   return LCD_STATUS_OK;
}

LCD_STATUS LCD_DumpM4U(void)
{
    //	_m4u_lcdc_func.m4u_dump_reg(M4U_CLNTMOD_LCDC);
    m4u_dump_info();
 
    return LCD_STATUS_OK;
}
#endif

LCD_STATUS LCD_W2M_NeedLimiteSpeed(BOOL enable)
{
    limit_w2m_speed = enable;
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_W2TVR_NeedLimiteSpeed(BOOL enable)
{
    limit_w2tvr_speed = enable;
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_SetGMCThrottle()
{
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_read_lcm_fb(unsigned char *buffer)
{
    LCD_WaitForNotBusy();
    
    // if read_fb not impl, should return info
    if(lcm_drv->ata_check)
        lcm_drv->ata_check(buffer);
    
    return LCD_STATUS_OK;
}


unsigned int LCD_Check_LCM(UINT32 color)
{
    unsigned int ret = 1;
    unsigned char buffer[60];
    unsigned int i=0;
    
    LCD_read_lcm_fb(buffer);
    for(i=0;i<60;i++)
        printk("%d\n",buffer[i]);	
    
    for(i=0;i<60;i+=3){
        printk("read pixel = 0x%x,",(buffer[i]<<16)|(buffer[i+1]<<8)|(buffer[i+2]));
        if(((buffer[i]<<16)|(buffer[i+1]<<8)|(buffer[i+2])) != (color&0xFFFFFF)){
            ret = 0;
            break;
        }
    }

    return ret;
}

// called by "esd_recovery_kthread"
BOOL LCD_esd_check(void)
{
#ifndef MT65XX_NEW_DISP
    UINT32 x, y, width, height;
    
    // Enable TE interrupt
    //LCD_TE_SetMode(LCD_TE_MODE_VSYNC_ONLY);
    //LCD_TE_SetEdgePolarity(LCM_POLARITY_RISING);
    LCD_TE_Enable(TRUE);
    
    // Backup ROI
    LCD_CHECK_RET(LCD_GetRoiWindow(&x, &y, &width, &height));
    
    // Set ROI = 0
    LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, 0, 0));
    
    // Switch to unuse port
    LCD_CHECK_RET(LCD_SelectWriteIF(LCD_IF_PARALLEL_2));
    
    // Write to LCM
    LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_LCM));
    
    // Blocking Trigger
    // This is to cheat LCDC to wait TE interrupt 
    LCD_CHECK_RET(LCD_StartTransfer(TRUE));
    
    // Restore ROI
    LCD_CHECK_RET(LCD_SetRoiWindow(x, y, width, height));
    
    // Disable TE interrupt
    LCD_TE_Enable(FALSE);
    
    // Write to memory	
    LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_MEM));
    
    return lcd_esd_check;
#endif	
    return TRUE;
}


