#define ENABLE_DSI_INTERRUPT 1

#include <linux/delay.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/mmprofile.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>

#include "disp_drv_log.h"
#include "disp_drv_platform.h"
#include "debug.h"

#include "lcd_reg.h"
#include "lcd_drv.h"

#include "ddp_debug.h"
#include "dsi_reg.h"
#include "dsi_drv.h"


extern unsigned int EnableVSyncLog;

#if ENABLE_DSI_INTERRUPT
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <mach/irqs.h>
#include "mtkfb.h"
static wait_queue_head_t _dsi_wait_queue;
static wait_queue_head_t _dsi_dcs_read_wait_queue;
static wait_queue_head_t _dsi_wait_bta_te;
static wait_queue_head_t _dsi_wait_ext_te;
static wait_queue_head_t _dsi_wait_vm_done_queue;
#endif
static DECLARE_WAIT_QUEUE_HEAD(_dsi_reg_update_wq);



/*
#define PLL_BASE			(0xF0060000)
#define DSI_PHY_BASE		(0xF0060B00)
#define DSI_BASE            	(0xF0140000)
*/


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

static PDSI_REGS const DSI_REG = (PDSI_REGS)(DSI_BASE);
static PDSI_VM_CMDQ_REGS const DSI_VM_CMD_REG = (PDSI_VM_CMDQ_REGS)(DSI_BASE + 0x134);
static PDSI_PHY_REGS const DSI_PHY_REG = (PDSI_PHY_REGS)(MIPI_CONFIG_BASE);
static PDSI_CMDQ_REGS const DSI_CMDQ_REG = (PDSI_CMDQ_REGS)(DSI_BASE+0x180);
static PLCD_REGS const LCD_REG = (PLCD_REGS)(DISP_DBI_BASE);

extern LCM_DRIVER *lcm_drv;
static bool dsi_log_on = false;
static bool glitch_log_on = false;
static bool force_transfer = false;
extern BOOL is_early_suspended;


typedef struct
{
    DSI_REGS regBackup;
    unsigned int cmdq_size;
    DSI_CMDQ_REGS cmdqBackup;
    unsigned int bit_time_ns;
    unsigned int vfp_period_us;
    unsigned int vsa_vs_period_us;
    unsigned int vsa_hs_period_us;
    unsigned int vsa_ve_period_us;
    unsigned int vbp_period_us;
    void (*pIntCallback)(DISP_INTERRUPT_EVENTS);
} DSI_CONTEXT;

static bool s_isDsiPowerOn = FALSE;
static DSI_CONTEXT _dsiContext;
extern LCM_PARAMS *lcm_params;

// PLL_clock
static const DSI_PLL_CONFIG pll_config[] =
{
//   CLK, TXDIV0, TXDIV1, SDM_PCW, SSC_PH_INIT, SSC_PRD, SSC_DELTA1, SSC_DELTA
    { 26, 0x10, 0x10, 0x40000000, 1, 0x01B1, 0x0790, 0x0790}, // 26 * 1
    { 52, 0x10, 0x01, 0x40000000, 1, 0x01B1, 0x0790, 0x0790}, // 26 * 2
    { 78, 0x10, 0x00, 0x30000000, 1, 0x01B1, 0x05AC, 0x05AC}, // 26 * 3
//    {100, 0x10, 0x00, 0x3D89D89D, 1, 0x01B1, 0x0745, 0x0745}, // 25 * 4
    {104, 0x10, 0x00, 0x40000000, 1, 0x01B1, 0x0790, 0x0790}, // 26 * 4
//    {125, 0x01, 0x00, 0x26762762, 1, 0x01B1, 0x048B, 0x048B}, // 25 * 5
    {130, 0x01, 0x00, 0x28000000, 1, 0x01B1, 0x04BA, 0x04BA}, // 26 * 5
//    {150, 0x01, 0x00, 0x2E276276, 1, 0x01B1, 0x0574, 0x0574}, // 25 * 6
    {156, 0x01, 0x00, 0x30000000, 1, 0x01B1, 0x05AC, 0x05AC}, // 26 * 6
//    {175, 0x01, 0x00, 0x35D89D89, 1, 0x01B1, 0x065D, 0x065D}, // 25 * 7
    {182, 0x01, 0x00, 0x38000000, 1, 0x01B1, 0x069E, 0x069E}, // 26 * 7
//    {200, 0x01, 0x00, 0x3D89D89D, 1, 0x01B1, 0x0745, 0x0745}, // 25 * 8
    {208, 0x01, 0x00, 0x40000000, 1, 0x01B1, 0x0790, 0x0790}, // 26 * 8
    {221, 0x01, 0x00, 0x44000000, 1, 0x01B1, 0x0809, 0x0809}, // 26 * 8.5 (default)
//    {225, 0x01, 0x00, 0x453B13B1, 1, 0x01B1, 0x082E, 0x082E}, // 25 * 9
    {234, 0x01, 0x00, 0x48000000, 1, 0x01B1, 0x0882, 0x0882}, // 26 * 9
//    {250, 0x00, 0x00, 0x26762762, 1, 0x01B1, 0x048B, 0x048B}, // 25 * 10
    {260, 0x00, 0x00, 0x28000000, 1, 0x01B1, 0x04BA, 0x04BA}, // 26 * 10
//    {275, 0x00, 0x00, 0x2A4EC4EC, 1, 0x01B1, 0x0500, 0x0500}, // 25 * 11
    {286, 0x00, 0x00, 0x2C000000, 1, 0x01B1, 0x0533, 0x0533}, // 26 * 11
//    {300, 0x00, 0x00, 0x2E276276, 1, 0x01B1, 0x0574, 0x0574}, // 25 * 12
    {312, 0x00, 0x00, 0x30000000, 1, 0x01B1, 0x05AC, 0x05AC}, // 26 * 12
//    {325, 0x00, 0x00, 0x32000000, 1, 0x01B1, 0x05E8, 0x05E8}, // 25 * 13
//    {350, 0x00, 0x00, 0x35D89D89, 1, 0x01B1, 0x065D, 0x065D}, // 25 * 14
//    {375, 0x00, 0x00, 0x39B13B13, 1, 0x01B1, 0x06D1, 0x06D1}, // 25 * 15
//    {400, 0x00, 0x00, 0x3D89D89D, 1, 0x01B1, 0x0745, 0x0745}, // 25 * 16
//    {425, 0x00, 0x00, 0x41627627, 1, 0x01B1, 0x07BA, 0x07BA}, // 25 * 17
//    {450, 0x00, 0x00, 0x453B13B1, 1, 0x01B1, 0x082E, 0x082E}, // 25 * 18
//    {475, 0x00, 0x00, 0x4913B13B, 1, 0x01B1, 0x08A2, 0x08A2}, // 25 * 19
//    {500, 0x00, 0x00, 0x4CEC4EC4, 1, 0x01B1, 0x0917, 0x0917}, // 25 * 20
};



DEFINE_SPINLOCK(g_handle_esd_lock);

#ifndef MT65XX_NEW_DISP
static bool dsi_esd_recovery = false;
static bool dsi_int_te_enabled = false;
static unsigned int dsi_int_te_period = 1;
static unsigned int dsi_dpi_isr_count = 0;
#endif
static bool dsi_noncont_clk_enabled = false;
static unsigned int dsi_noncont_clk_period = 1;
unsigned long g_handle_esd_flag;

static volatile bool isTeSetting = false;
static volatile bool dsiStartTransfer = false;
static volatile bool dsiTeEnable = true;
static volatile bool dsiTeExtEnable = false;



static long int get_current_time_us(void)
{
    struct timeval t;
    
    do_gettimeofday(&t);
    
    return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}


void DSI_Enable_Log(bool enable)
{
    dsi_log_on = enable;
}


void Glitch_Enable_Log(bool enable)
{
    glitch_log_on = enable;
}


unsigned int try_times = 30;
void Glitch_times(unsigned int times)
{
    try_times = times;
}


static wait_queue_head_t _vsync_wait_queue;
static bool dsi_vsync = false;
static bool wait_dsi_vsync = false;
static struct hrtimer hrtimer_vsync;
#define VSYNC_US_TO_NS(x) (x * 1000)
static unsigned int vsync_timer = 0;
static bool wait_vm_done_irq = false;

#if ENABLE_DSI_INTERRUPT
static irqreturn_t _DSI_InterruptHandler(int irq, void *dev_id)
{   
   DSI_INT_STATUS_REG status = DSI_REG->DSI_INTSTA;
   MMProfileLogEx(MTKFB_MMP_Events.DSIIRQ, MMProfileFlagPulse, *(unsigned int*)&status, dsiStartTransfer);

#if ENABLE_DSI_INTERRUPT
   if (status.RD_RDY)
   {        
      ///write clear RD_RDY interrupt
      DSI_REG->DSI_INTSTA.RD_RDY = 1;   
      
      /// write clear RD_RDY interrupt must be before DSI_RACK
      /// because CMD_DONE will raise after DSI_RACK, 
      /// so write clear RD_RDY after that will clear CMD_DONE too
      
      do
      {
         ///send read ACK
         OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
      } while(DSI_REG->DSI_INTSTA.BUSY);
      
      MASKREG32(&DSI_REG->DSI_INTSTA, 0x1, 0x0);
      wake_up_interruptible(&_dsi_dcs_read_wait_queue);
      if(_dsiContext.pIntCallback)
         _dsiContext.pIntCallback(DISP_DSI_READ_RDY_INT);            
   }
   
   if (status.CMD_DONE)
   {
      if (dsiStartTransfer)
      {
         // The last screen update has finished.
         if(_dsiContext.pIntCallback)
            _dsiContext.pIntCallback(DISP_DSI_CMD_DONE_INT);

         DBG_OnLcdDone();
         //if(1 == lcm_params->dsi.compatibility_for_nvk)
         if(0)
         {
             if(!dsi_noncont_clk_enabled){
                DSI_clk_HS_mode(0);
             }
         }
      }
      
      // clear flag & wait for next trigger
      dsiStartTransfer = false;
      
      //DSI_REG->DSI_INTSTA.CMD_DONE = 0;
      OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);
      
      wake_up_interruptible(&_dsi_wait_queue);
      //if(_dsiContext.pIntCallback)
      //    _dsiContext.pIntCallback(DISP_DSI_CMD_DONE_INT);   
      //MASKREG32(&DSI_REG->DSI_INTSTA, 0x2, 0x0);
   }

   if (status.TE_RDY)
   {
      DBG_OnTeDelayDone();
      
      // Write clear RD_RDY
      OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,TE_RDY,0);
      
      // Set DSI_RACK to let DSI idle
      OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
      
      wake_up_interruptible(&_dsi_wait_bta_te);
      
      #ifndef BUILD_UBOOT    
         if(wait_dsi_vsync)//judge if wait vsync
         {
            if (EnableVSyncLog)
               printk("[DSI] VSync2\n");

            if(-1 != hrtimer_try_to_cancel(&hrtimer_vsync))
            {
               dsi_vsync = true;
               //hrtimer_try_to_cancel(&hrtimer_vsync);
               if (EnableVSyncLog)
                  printk("[DSI] VSync3\n");

               wake_up_interruptible(&_vsync_wait_queue);
            }
            //printk("TE signal, and wake up\n");
         }
      #endif  
   }

   if (status.EXT_TE)
   {
      DBG_OnTeDelayDone();

      // Write clear RD_RDY
      OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,EXT_TE,0);

      wake_up_interruptible(&_dsi_wait_ext_te);

         if(wait_dsi_vsync)//judge if wait vsync
         {
            if (EnableVSyncLog)
               printk("[DSI] VSync2\n");

            if(-1 != hrtimer_try_to_cancel(&hrtimer_vsync))
            {
               dsi_vsync = true;
               //hrtimer_try_to_cancel(&hrtimer_vsync);
               if (EnableVSyncLog)
                  printk("[DSI] VSync3\n");

               wake_up_interruptible(&_vsync_wait_queue);
            }
            //printk("TE signal, and wake up\n");
         }
   }
   
   if (status.VM_DONE)
   {
      //		DBG_OnTeDelayDone();
      OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,VM_DONE,0);

      if(_dsiContext.pIntCallback)
         _dsiContext.pIntCallback(DISP_DSI_VMDONE_INT);

      if(dsi_log_on)
         printk("DSI VM done IRQ!!\n");

      // Write clear VM_Done
      wake_up_interruptible(&_dsi_wait_vm_done_queue);

      //if(1 == lcm_params->dsi.compatibility_for_nvk)
      if(0)
      {
         if(!dsi_noncont_clk_enabled){
            MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 1, 22);
            if(!is_early_suspended && !wait_vm_done_irq){
               if(DSI_STATUS_OK != DSI_Detect_CLK_Glitch()){
                  printk("VM Done detect glitch fail!!,%d\n",__LINE__);
               }
               DSI_EnableClk();
            }
         }
      }
   }

   if (status.SLEEPOUT_DONE)
   {
      OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,SLEEPOUT_DONE,0);
   }
#endif
   
   return IRQ_HANDLED;
}
#endif


#ifndef BUILD_UBOOT
void DSI_GetVsyncCnt(void)
{
}


enum hrtimer_restart dsi_te_hrtimer_func(struct hrtimer *timer)
{
//    long long ret;

    if (EnableVSyncLog)
        printk("[DSI] VSync0\n");

    if(wait_dsi_vsync)
    {
        dsi_vsync = true;
        
        if (EnableVSyncLog)
            printk("[DSI] VSync1\n");
 
        wake_up_interruptible(&_vsync_wait_queue);
    }
//    ret = hrtimer_forward_now(timer, ktime_set(0, VSYNC_US_TO_NS(vsync_timer)));
//    printk("hrtimer callback\n");

    return HRTIMER_NORESTART;
}
#endif


void DSI_WaitTE(void)
{
#ifndef BUILD_UBOOT	
    wait_dsi_vsync = true;
    
    hrtimer_start(&hrtimer_vsync, ktime_set(0, VSYNC_US_TO_NS(vsync_timer)), HRTIMER_MODE_REL);
    if (EnableVSyncLog)
        printk("[DSI] +VSync\n");

    wait_event_interruptible(_vsync_wait_queue, dsi_vsync);

    if (EnableVSyncLog)
        printk("[DSI] -VSync\n");

    dsi_vsync = false;
    wait_dsi_vsync = false;
#endif
}


void DSI_InitVSYNC(unsigned int vsync_interval)
{
#ifndef BUILD_UBOOT
    ktime_t ktime;

    vsync_timer = vsync_interval;
    ktime = ktime_set(0, VSYNC_US_TO_NS(vsync_timer));
    hrtimer_init(&hrtimer_vsync, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hrtimer_vsync.function = dsi_te_hrtimer_func;
//    hrtimer_start(&hrtimer_vsync, ktime, HRTIMER_MODE_REL);
#endif
}


static BOOL _IsEngineBusy(void)
{
   DSI_INT_STATUS_REG status;
   
   status = DSI_REG->DSI_INTSTA;
   
   if (status.BUSY)		
      return TRUE;

   return FALSE;
}


DSI_STATUS DSI_WaitForEngineNotBusy(void)
{
   int timeOut;
#if ENABLE_DSI_INTERRUPT
   long int time;
   static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
#endif
   
   if (DSI_REG->DSI_MODE_CTRL.MODE)
      return DSI_STATUS_OK;
   
   timeOut = 200;
   
#if ENABLE_DSI_INTERRUPT
   time = get_current_time_us();
   
   if (in_interrupt())
   {
      // perform busy waiting if in interrupt context
      while(_IsEngineBusy()) {
         msleep(1);

         if (--timeOut < 0)	{
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!(Wait %d us)\n", get_current_time_us() - time);
            DSI_DumpRegisters();

            DSI_Reset();

            return DSI_STATUS_ERROR;
         }
      }
   }
   else
   {
      if (DSI_REG->DSI_INTSTA.BUSY || DSI_REG->DSI_INTSTA.CMD_DONE)
      {
         long ret = wait_event_interruptible_timeout(_dsi_wait_queue, 
                                                                          !_IsEngineBusy() && !(DSI_REG->DSI_INTSTA.CMD_DONE),
                                                                          WAIT_TIMEOUT);

         if (0 == ret) {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine not busy timeout!!!\n");
            DSI_DumpRegisters();

            DSI_Reset();
            dsiTeEnable = false;
            dsiTeExtEnable = false;

            return DSI_STATUS_ERROR;
         }
      }
   }
#else
   
   while(_IsEngineBusy()) {
      udelay(100);
      /*printk("xuecheng, dsi wait\n");*/

      if (--timeOut < 0) {
         DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!\n");
         DSI_DumpRegisters();

         DSI_Reset();
         dsiTeEnable = false;
         dsiTeExtEnable = false;
         OUTREG32(&DSI_REG->DSI_INTSTA, 0x0);

         return DSI_STATUS_ERROR;
      }
   }
   OUTREG32(&DSI_REG->DSI_INTSTA, 0x0);
#endif    

    return DSI_STATUS_OK;
}


void hdmi_dsi_waitnotbusy(void)
{
    DSI_WaitForEngineNotBusy();
}


DSI_STATUS DSI_BackupRegisters(void)
{
    DSI_REGS *regs = &(_dsiContext.regBackup);
    
    //memcpy((void*)&(_dsiContext.regBackup), (void*)DSI_BASE, sizeof(DSI_REGS));
    
    OUTREG32(&regs->DSI_INTEN, AS_UINT32(&DSI_REG->DSI_INTEN));
    OUTREG32(&regs->DSI_MODE_CTRL, AS_UINT32(&DSI_REG->DSI_MODE_CTRL));
    OUTREG32(&regs->DSI_TXRX_CTRL, AS_UINT32(&DSI_REG->DSI_TXRX_CTRL));
    OUTREG32(&regs->DSI_PSCTRL, AS_UINT32(&DSI_REG->DSI_PSCTRL));
    
    OUTREG32(&regs->DSI_VSA_NL, AS_UINT32(&DSI_REG->DSI_VSA_NL));		
    OUTREG32(&regs->DSI_VBP_NL, AS_UINT32(&DSI_REG->DSI_VBP_NL));		
    OUTREG32(&regs->DSI_VFP_NL, AS_UINT32(&DSI_REG->DSI_VFP_NL));		
    OUTREG32(&regs->DSI_VACT_NL, AS_UINT32(&DSI_REG->DSI_VACT_NL));		
    
    OUTREG32(&regs->DSI_HSA_WC, AS_UINT32(&DSI_REG->DSI_HSA_WC));		
    OUTREG32(&regs->DSI_HBP_WC, AS_UINT32(&DSI_REG->DSI_HBP_WC));		
    OUTREG32(&regs->DSI_HFP_WC, AS_UINT32(&DSI_REG->DSI_HFP_WC));		
    OUTREG32(&regs->DSI_BLLP_WC, AS_UINT32(&DSI_REG->DSI_BLLP_WC));		
    
    OUTREG32(&regs->DSI_HSTX_CKL_WC, AS_UINT32(&DSI_REG->DSI_HSTX_CKL_WC));		
    OUTREG32(&regs->DSI_MEM_CONTI, AS_UINT32(&DSI_REG->DSI_MEM_CONTI));

	OUTREG32(&regs->DSI_VM_CMD_CON, AS_UINT32(&DSI_REG->DSI_VM_CMD_CON));

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_RestoreRegisters(void)
{
    DSI_REGS *regs = &(_dsiContext.regBackup);
    
    OUTREG32(&DSI_REG->DSI_INTEN, AS_UINT32(&regs->DSI_INTEN));	
    OUTREG32(&DSI_REG->DSI_MODE_CTRL, AS_UINT32(&regs->DSI_MODE_CTRL));	
    OUTREG32(&DSI_REG->DSI_TXRX_CTRL, AS_UINT32(&regs->DSI_TXRX_CTRL));	
    OUTREG32(&DSI_REG->DSI_PSCTRL, AS_UINT32(&regs->DSI_PSCTRL));	
    
    OUTREG32(&DSI_REG->DSI_VSA_NL, AS_UINT32(&regs->DSI_VSA_NL));		
    OUTREG32(&DSI_REG->DSI_VBP_NL, AS_UINT32(&regs->DSI_VBP_NL));		
    OUTREG32(&DSI_REG->DSI_VFP_NL, AS_UINT32(&regs->DSI_VFP_NL));		
    OUTREG32(&DSI_REG->DSI_VACT_NL, AS_UINT32(&regs->DSI_VACT_NL));		
    
    OUTREG32(&DSI_REG->DSI_HSA_WC, AS_UINT32(&regs->DSI_HSA_WC));		
    OUTREG32(&DSI_REG->DSI_HBP_WC, AS_UINT32(&regs->DSI_HBP_WC));		
    OUTREG32(&DSI_REG->DSI_HFP_WC, AS_UINT32(&regs->DSI_HFP_WC));	
    OUTREG32(&DSI_REG->DSI_BLLP_WC, AS_UINT32(&regs->DSI_BLLP_WC));		
    
    OUTREG32(&DSI_REG->DSI_HSTX_CKL_WC, AS_UINT32(&regs->DSI_HSTX_CKL_WC));		
    OUTREG32(&DSI_REG->DSI_MEM_CONTI, AS_UINT32(&regs->DSI_MEM_CONTI));		

	OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&regs->DSI_VM_CMD_CON));		

    return DSI_STATUS_OK;
}


static void _ResetBackupedDSIRegisterValues(void)
{
    DSI_REGS *regs = &_dsiContext.regBackup;
    memset((void*)regs, 0, sizeof(DSI_REGS));
}


static void DSI_BackUpCmdQ(void)
{
    unsigned int i;
    DSI_CMDQ_REGS *regs = &(_dsiContext.cmdqBackup);
    
    _dsiContext.cmdq_size = AS_UINT32(&DSI_REG->DSI_CMDQ_SIZE);
    
    for (i=0; i<_dsiContext.cmdq_size; i++)
        OUTREG32(&regs->data[i], AS_UINT32(&DSI_CMDQ_REG->data[i]));
}


static void DSI_RestoreCmdQ(void)
{
    unsigned int i;
    DSI_CMDQ_REGS *regs = &(_dsiContext.cmdqBackup);
    
    OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, AS_UINT32(&_dsiContext.cmdq_size));
    
    for (i=0; i<_dsiContext.cmdq_size; i++)
        OUTREG32(&DSI_CMDQ_REG->data[i], AS_UINT32(&regs->data[i]));
}


static void _DSI_RDMA0_IRQ_Handler(unsigned int param);
spinlock_t dsi_glitch_detect_lock;

static DSI_STATUS DSI_TE_Setting(void)
{
    //return DSI_STATUS_OK;
    if(isTeSetting)
    {
        return DSI_STATUS_OK;
    }

    if(lcm_params->dsi.mode == CMD_MODE && lcm_params->dsi.lcm_ext_te_enable == TRUE)
    {
        //Enable EXT TE
        dsiTeEnable = false;
        dsiTeExtEnable = true;
    }
    else
    {
        //Enable BTA TE
        dsiTeEnable = true;
        dsiTeExtEnable = false;
    }

    isTeSetting = true;

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Init(BOOL isDsiPoweredOn)
{
   DSI_STATUS ret = DSI_STATUS_OK;
   
   memset(&_dsiContext, 0, sizeof(_dsiContext));
   
   if (isDsiPoweredOn) {
      DSI_BackupRegisters();
   } 
   else {
      _ResetBackupedDSIRegisterValues();
   }

   ret = DSI_PowerOn();
   ASSERT(ret == DSI_STATUS_OK);
   
    DSI_TE_Setting();

   OUTREG32(&DSI_REG->DSI_MEM_CONTI, DSI_WMEM_CONTI);	
   OUTREGBIT(DSI_COM_CTRL_REG, DSI_REG->DSI_COM_CTRL, DSI_EN, 1);
   
#if ENABLE_DSI_INTERRUPT
    init_waitqueue_head(&_dsi_wait_queue);
    init_waitqueue_head(&_dsi_dcs_read_wait_queue);	
    init_waitqueue_head(&_dsi_wait_bta_te);
    init_waitqueue_head(&_dsi_wait_ext_te);
    init_waitqueue_head(&_dsi_wait_vm_done_queue);
 
    if (request_irq(MT_DISP_DSI_IRQ_ID,
         _DSI_InterruptHandler, IRQF_TRIGGER_LOW, MTKFB_DRIVER, NULL) < 0)
    {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", "fail to request DSI irq\n"); 
        return DSI_STATUS_ERROR;
    }
 
    //mt65xx_irq_unmask(MT_DISP_DSI_IRQ_ID);
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,TE_RDY,1);
	OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,EXT_TE,1);

    init_waitqueue_head(&_vsync_wait_queue);

    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,VM_DONE,1);
    init_waitqueue_head(&_vsync_wait_queue);

#endif
    if (lcm_params->dsi.mode == CMD_MODE)
        disp_register_irq(DISP_MODULE_RDMA0, _DSI_RDMA0_IRQ_Handler);

    //if(1 == lcm_params->dsi.compatibility_for_nvk)
    if(0)
    {
        spin_lock_init(&dsi_glitch_detect_lock);
    }
   
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Deinit(void)
{
   DSI_STATUS ret = DSI_PowerOff();
   
   ASSERT(ret == DSI_STATUS_OK);
   
   return DSI_STATUS_OK;
}


DSI_STATUS DSI_PowerOn(void)
{
    int ret = 0;
 
    if (!s_isDsiPowerOn)
    {
        if (!clock_is_on(MT_CG_MIPI_26M_DBG_EN))
            ret += enable_clock(MT_CG_MIPI_26M_DBG_EN, "DSI");
        if (!clock_is_on(MT_CG_DISP_DBI_ENGINE_SW_CG))
            ret += enable_clock(MT_CG_DISP_DBI_ENGINE_SW_CG, "LCD");
        if (!clock_is_on(MT_CG_DSI_ENGINE_SW_CG))
            ret += enable_clock(MT_CG_DSI_ENGINE_SW_CG, "DSI");
        if (!clock_is_on(MT_CG_DSI_DIGITAL_SW_CG))
            ret += enable_clock(MT_CG_DSI_DIGITAL_SW_CG, "DSI");

        if(ret > 0)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "DSI power manager API return FALSE\n");
        }

        s_isDsiPowerOn = TRUE;
    }
 
    // DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "%s, line:%d\n", __func__, __LINE__);
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PowerOff(void)
{
    int ret = 0;

    if (s_isDsiPowerOn)
    {
        if (clock_is_on(MT_CG_DSI_DIGITAL_SW_CG))
            ret += disable_clock(MT_CG_DSI_DIGITAL_SW_CG, "DSI");
        if (clock_is_on(MT_CG_DSI_ENGINE_SW_CG))
            ret += disable_clock(MT_CG_DSI_ENGINE_SW_CG, "DSI");
        if (clock_is_on(MT_CG_DISP_DBI_ENGINE_SW_CG))
            ret += disable_clock(MT_CG_DISP_DBI_ENGINE_SW_CG, "LCD");
        if (clock_is_on(MT_CG_MIPI_26M_DBG_EN))
            ret += disable_clock(MT_CG_MIPI_26M_DBG_EN, "DSI");

        if(ret > 0)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "DSI power manager API return FALSE\n");
        }

        s_isDsiPowerOn = FALSE;
    }

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_WaitForNotBusy(void)
{
   DSI_WaitForEngineNotBusy();
   
   return DSI_STATUS_OK;
}


static void DSI_WaitBtaTE(void)
{
    DSI_T0_INS t0;
    #if ENABLE_DSI_INTERRUPT
        long ret;
        static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
    #else
        long int dsi_current_time;
    #endif
    

    if(DSI_REG->DSI_MODE_CTRL.MODE != CMD_MODE)
        return;
    
    DSI_WaitForEngineNotBusy();
    
    // backup command queue setting.
    DSI_BackUpCmdQ();
    
    t0.CONFG = 0x20;		///TE
    t0.Data0 = 0;
    t0.Data_ID = 0;
    t0.Data1 = 0;
    
    OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
    OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
    
    // wait BTA TE command complete.
    DSI_WaitForEngineNotBusy();
    
    // restore command queue setting.
    DSI_RestoreCmdQ();
    
    #if ENABLE_DSI_INTERRUPT
        ret = wait_event_interruptible_timeout(_dsi_wait_bta_te, 
                                                                         !_IsEngineBusy(),
                                                                         WAIT_TIMEOUT);
        
        if (0 == ret) {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for _dsi_wait_bta_te(DSI_INTSTA.TE_RDY) ready timeout!!!\n");
            
            // Set DSI_RACK to let DSI idle
            OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
            
            DSI_DumpRegisters();	
            ///do necessary reset here
            DSI_Reset();
            dsiTeEnable = false;//disable TE

            return;
        }
        
        // After setting DSI_RACK, it needs to wait for CMD_DONE interrupt.
        DSI_WaitForEngineNotBusy();

    #else
        dsi_current_time = get_current_time_us();
        
        while(DSI_REG->DSI_INTSTA.TE_RDY == 0)	// polling TE_RDY
        {
            if(get_current_time_us() - dsi_current_time > 100*1000)
            {
                DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for TE_RDY timeout!!!\n");
                
                // Set DSI_RACK to let DSI idle
                OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
                
                DSI_DumpRegisters();
                
                //do necessary reset here
                DSI_Reset();
                dsiTeEnable = false;//disable TE

                break;
            }
        }
        
        // Write clear RD_RDY
        OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,TE_RDY,0);
        
        // Set DSI_RACK to let DSI idle
        OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);

        if(!dsiTeEnable){
            DSI_LP_Reset();
            return;
        }
        dsi_current_time = get_current_time_us();
        
        while(DSI_REG->DSI_INTSTA.CMD_DONE == 0)	// polling CMD_DONE
        {
            if(get_current_time_us() - dsi_current_time > 100*1000)
            {
                DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for CMD_DONE timeout!!!\n");
                
                // Set DSI_RACK to let DSI idle
                OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
                
                DSI_DumpRegisters();
                
                ///do necessary reset here
                DSI_Reset();
                dsiTeEnable = false;//disable TE

                break;
            }
        }
        
        // Write clear CMD_DONE
        OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);
    #endif

    DSI_LP_Reset();
}


static void DSI_WaitExternalTE(void)
{
#if ENABLE_DSI_INTERRUPT
    long ret;
    static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
#else
    long int dsi_current_time;
#endif


    if(DSI_REG->DSI_MODE_CTRL.MODE != CMD_MODE)
        return;


    OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EN,1);
    OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EDGE,0);

#if ENABLE_DSI_INTERRUPT
    ret = wait_event_interruptible_timeout(_dsi_wait_ext_te,
                                                                     !_IsEngineBusy(),
                                                                     WAIT_TIMEOUT);

    if (0 == ret) {
        DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for _dsi_wait_ext_te(DSI_INTSTA.EXT_TE) ready timeout!!!\n");

        DSI_DumpRegisters();
        ///do necessary reset here
        DSI_Reset();
        dsiTeExtEnable = false;//disable TE

        return;
    }

#else
    dsi_current_time = get_current_time_us();

    while(DSI_REG->DSI_INTSTA.EXT_TE == 0)	// polling EXT_TE
    {
        if(get_current_time_us() - dsi_current_time > 100*1000)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for EXT_TE timeout!!!\n");

            DSI_DumpRegisters();

            //do necessary reset here
            DSI_Reset();
            dsiTeExtEnable = false;//disable TE

            break;
        }
    }

    // Write clear EXT_TE
    OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,EXT_TE,0);

    if(!dsiTeExtEnable){
        return;
    }

#endif
}


DSI_STATUS DSI_Start(void)
{
    if (DSI_STATUS_OK == DSI_WaitForEngineNotBusy())
    {
        OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
        OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
    }
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_EnableVM_CMD(void)
{
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,VM_CMD_START,0);
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,VM_CMD_START,1);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_StartTransfer(bool isMutexLocked)
{
    // needStartDSI = 1: For command mode or the first time of video mode.
    // After the first time of video mode. Configuration is applied in ConfigurationUpdateTask.
    extern struct mutex OverlaySettingMutex;

    MMProfileLogMetaStringEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, isMutexLocked, 0, "StartTransfer");

    if (!isMutexLocked)
        disp_path_get_mutex();

    mutex_lock(&OverlaySettingMutex);

    LCD_ConfigOVL();
    // Insert log for trigger point.
    DBG_OnTriggerLcd();

    if (dsiTeEnable)
    {
        DSI_WaitBtaTE();
    }
    if(dsiTeExtEnable)
    {
        DSI_WaitExternalTE();
    }

    //if(1 == lcm_params->dsi.compatibility_for_nvk)
    if(0)
    {
        if(!dsi_noncont_clk_enabled){
            spin_lock_irq(&dsi_glitch_detect_lock);
            if(DSI_STATUS_OK != DSI_Detect_CLK_Glitch()){
                if(!force_transfer){
                    spin_unlock_irq(&dsi_glitch_detect_lock);
                    mutex_unlock(&OverlaySettingMutex);
                    if(_dsiContext.pIntCallback)
                        _dsiContext.pIntCallback(DISP_DSI_CMD_DONE_INT);
                    return DSI_STATUS_OK;
                }
            }
            spin_unlock_irq(&dsi_glitch_detect_lock);
        }
    }

    DSI_WaitForEngineNotBusy();

    // To trigger frame update.
    DSI_Start();
    dsiStartTransfer = true;

    mutex_unlock(&OverlaySettingMutex);
    if (!isMutexLocked)
        disp_path_release_mutex();

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Detect_CLK_Glitch(void)
{
    DSI_T0_INS t0;
    char i;
    int read_timeout_cnt=10000;
    int read_timeout_ret = 0;
    unsigned long long start_time,end_time;

    DSI_RX_DATA_REG read_data0;
    DSI_RX_DATA_REG read_data1;


    while(DSI_REG->DSI_INTSTA.BUSY);
    OUTREG32(&DSI_REG->DSI_INTSTA, 0x0);
    
    DSI_BackUpCmdQ();
    DSI_SetMode(CMD_MODE);
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,0);	
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,0);
    
    OUTREG32(&DSI_CMDQ_REG->data[0], 0x00340500);//turn off TE
    OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
    
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
    while(DSI_REG->DSI_INTSTA.CMD_DONE == 0);
    OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);
    
    MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 0);
    for(i=0;i<try_times;i++)
    {
        DSI_clk_HS_mode(0);
        
        MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 9);

        while((INREG32(&DSI_REG->DSI_STATE_DBG0)&0x1) == 0);	 // polling bit0
        OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
        OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);//reset
        OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
        
        MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 10);
        if(i>0)
        {
            OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_PHI_SEL,0);
        }
        DSI_clk_HS_mode(1);
        MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 1);
        while((INREG32(&DSI_REG->DSI_STATE_DBG0)&0x40000) == 0);	 // polling bit18 start

        MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 2);
        if(i>0)
        {
            OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_PHI_SEL,1);
        }
        //			OUTREG32(&DSI_CMDQ_REG->data[0], 0x00290508);
        OUTREG32(&DSI_CMDQ_REG->data[0], 0x00351508);
        OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
        
        OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
        OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
        read_timeout_cnt=1000000;
        MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 3);

        start_time = sched_clock();
        while(DSI_REG->DSI_INTSTA.BUSY) {
            end_time = sched_clock();

            if(((unsigned int)sched_clock() - (unsigned int)start_time) > 50000){
                DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!:%d\n",__LINE__);
                DSI_Reset();
                break;
            }
        }
        OUTREG32(&DSI_REG->DSI_INTSTA, 0x0);
        //			spin_unlock_irq(&dsi_glitch_detect_lock);
        MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 4);
        
        t0.CONFG = 0x04;
        t0.Data0 = 0;
        t0.Data_ID = 0;
        t0.Data1 = 0;
        
        OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
        OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
        
        OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
        OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
        
        read_timeout_cnt=1000;
        MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 5);
        start_time = sched_clock();
        while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
        {
            end_time = sched_clock();
            if(((unsigned int)sched_clock() - (unsigned int)start_time) > 50000)
            {
                if(glitch_log_on)
                    printk("Test log 4:Polling DSI read ready timeout,%d us\n", (unsigned int)sched_clock() - (unsigned int)start_time);
                
                MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 13);

                OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
                DSI_Reset();
                read_timeout_ret = 1;
                break;
            }
        }
        if(1 == read_timeout_ret){
            read_timeout_ret = 0;
            //			return 1;
            continue;
        }

        MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 6);
        OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
        OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,0);
        
        if(((DSI_REG->DSI_TRIG_STA.TRIG2) )==1)
        {
            break;
            //			continue;			 
        }
        else
        {
            //read error report
            OUTREG32(&read_data0, AS_UINT32(&DSI_REG->DSI_RX_DATA0));
            OUTREG32(&read_data1, AS_UINT32(&DSI_REG->DSI_RX_DATA1));

            if(glitch_log_on)
            {
                printk("read_data0, %x,%x,%x,%x\n", read_data0.byte0, read_data0.byte1, read_data0.byte2, read_data0.byte3);
                printk("read_data1, %x,%x,%x,%x\n", read_data1.byte0, read_data1.byte1, read_data1.byte2, read_data1.byte3);
            }
            if(((read_data0.byte1&0x7) != 0)||((read_data0.byte2&0x3)!=0)) //bit 0-3	bit 8-9
            {
                continue;
            }
            else
            {
                //	 				continue;			 
                break;// jump out the for loop ,go to refresh
            }
        }
    }

    MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 7);

    switch(lcm_params->dsi.LANE_NUM)
    {
        case LCM_FOUR_LANE:
            OUTREG32(&DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0, 0x3CF3C7B1); 
            break;
        case LCM_THREE_LANE:
            OUTREG32(&DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0, 0x00F3C7B1); 
            break;
        default:
            OUTREG32(&DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0, 0x0003C7B1); 
    }	
    
    OUTREG32(&DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON1, 0x0); 
    OUTREG32(&DSI_PHY_REG->MIPITX_DSI_SW_CTRL, 0x1); 

    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);    
    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);    
    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);    
    
    DSI_clk_HS_mode(1);
    
    while((INREG32(&DSI_REG->DSI_STATE_DBG0)&0x40000) == 0);	 // polling bit18
    
    OUTREGBIT(MIPITX_DSI_SW_CTRL_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL,SW_CTRL_EN,0x0);

    start_time = sched_clock();
    while(DSI_REG->DSI_INTSTA.BUSY) {
        end_time = sched_clock();

        if(((unsigned int)sched_clock() - (unsigned int)start_time) > 50000)
        {
            DSI_Reset();
            break;
        }
    }
    OUTREG32(&DSI_REG->DSI_INTSTA, 0x0);
    
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);	
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);
    DSI_RestoreCmdQ();
    DSI_SetMode(lcm_params->dsi.mode);
    MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagPulse, 0, 8);

    if(i == try_times)
        return DSI_STATUS_ERROR;

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Config_VDO_FRM_Mode(void)
{
    try_times = 30;
    force_transfer = true;
    OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,FRM_MODE,1);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Stop(void)
{
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_EnableClk(void)
{
    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_EN,1);
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_DisableClk(void)
{
    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_EN,0);
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_SleepOut(void)
{
    unsigned int data_rate;
    unsigned int wakeup_time;
    
    data_rate = lcm_params->dsi.PLL_CLOCK*2;
    if (0 == data_rate)
    {
        unsigned int div1 = 0;
        unsigned int div2 = 0;
        unsigned int fbk_div = 0;

        div1 = lcm_params->dsi.pll_div1;
        div2 = lcm_params->dsi.pll_div2;
        fbk_div = lcm_params->dsi.fbk_div;
    
        switch(div1)
        {
            case 0:
                div1 = 1;
                break;
            case 1:
                div1 = 2;
                break;
            case 2:
            case 3:
                div1 = 4;
                break;
            default:
                printk("[DISP] div1 should be less than 4!!\n");
                div1 = 4;
                break;
        }
        switch(div2)
        {
            case 0:
                div2 = 1;
                break;
            case 1:
                div2 = 2;
                break;
            case 2:
            case 3:
                div2 = 4;
                break;
            default:
                printk("[DISP] div2 should be less than 4!!\n");
                div2 = 4;
                break;
        }

        data_rate = (fbk_div * 26 * 2) / div1 / div2;
    }

    // cycle to 1.2ms (Mbps * 1200 / 4)
    wakeup_time = data_rate*300;
    if (((1<<20) - 1) < wakeup_time)
    {
        wakeup_time = (1<<20) - 1;
    }

    OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,SLEEP_MODE,1);
    OUTREGBIT(DSI_PHY_TIMCON4_REG,DSI_REG->DSI_PHY_TIMECON4,ULPS_WAKEUP,wakeup_time);
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Wakeup(void)
{
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,SLEEPOUT_START,0);
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,SLEEPOUT_START,1);
    mdelay(1);
    OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,SLEEPOUT_START,0);
    OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,SLEEP_MODE,0);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Reset(void)
{
    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_LP_Reset(void)
{
#if 0
    DSI_WaitForEngineNotBusy();
    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
    OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
#endif
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_SetMode(unsigned int mode)
{
    OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,MODE,mode);
 
    return DSI_STATUS_OK;
}


static void _DSI_RDMA0_IRQ_Handler(unsigned int param)
{
    if(_dsiContext.pIntCallback)
    {
        if (param & 4)
        {
            MMProfileLogEx(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagEnd, param, 0);
        	_dsiContext.pIntCallback(DISP_DSI_SCREEN_UPDATE_END_INT);
        }
        if (param & 8)
        {
            MMProfileLogEx(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagEnd, param, 0);
        }
        if (param & 2)
        {
            MMProfileLogEx(MTKFB_MMP_Events.ScreenUpdate, MMProfileFlagStart, param, 0);
            _dsiContext.pIntCallback(DISP_DSI_SCREEN_UPDATE_START_INT);
        }
        if (param & 0x20)
        {
            _dsiContext.pIntCallback(DISP_DSI_TARGET_LINE_INT);
            _dsiContext.pIntCallback(DISP_DSI_VSYNC_INT);
        }
    }
}


static void _DSI_MUTEX_IRQ_Handler(unsigned int param)
{
    if(_dsiContext.pIntCallback)
    {
        if (param & 1)
        {
            // mutex0 register update interrupt
            _dsiContext.pIntCallback(DISP_DSI_REG_UPDATE_INT);
        }
    }
}


DSI_STATUS DSI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
#if ENABLE_DSI_INTERRUPT
   switch(eventID)
   {
      case DISP_DSI_READ_RDY_INT:
         OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
         break;

      case DISP_DSI_CMD_DONE_INT:
         OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);
         break;

      case DISP_DSI_VMDONE_INT:
         OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,VM_DONE,1);
         break;

      case DISP_DSI_MIX_MODE_DONE_INT:
         OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,VM_CMD_DONE,1);
         break;
         
      case DISP_DSI_SLEEP_OUT_DONE_INT:
         OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,SLEEPOUT_DONE,1);
         break;
         
      case DISP_DSI_VSYNC_INT:
         disp_register_irq(DISP_MODULE_RDMA0, _DSI_RDMA0_IRQ_Handler);
         break;

      case DISP_DSI_TARGET_LINE_INT:
         disp_register_irq(DISP_MODULE_RDMA0, _DSI_RDMA0_IRQ_Handler);
         break;

      case DISP_DSI_SCREEN_UPDATE_START_INT:
         disp_register_irq(DISP_MODULE_RDMA0, _DSI_RDMA0_IRQ_Handler);
         break;

      case DISP_DSI_SCREEN_UPDATE_END_INT:
         disp_register_irq(DISP_MODULE_RDMA0, _DSI_RDMA0_IRQ_Handler);
         break;

      case DISP_DSI_REG_UPDATE_INT:
         //wake_up_interruptible(&_dsi_reg_update_wq);
         disp_register_irq(DISP_MODULE_MUTEX, _DSI_MUTEX_IRQ_Handler);
         break;

      default:
         return DSI_STATUS_ERROR;
   }
   
   return DSI_STATUS_OK;

#else
   ///TODO: warning log here
   return DSI_STATUS_ERROR;

#endif
}


DSI_STATUS DSI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
   _dsiContext.pIntCallback = pCB;
   
   return DSI_STATUS_OK;
}


DSI_STATUS DSI_handle_TE(void)
{
   unsigned int data_array;
   
   //data_array=0x00351504;
   //DSI_set_cmdq(&data_array, 1, 1);
   
   //mdelay(10);
   
   // RACT	
   //data_array=1;
   //OUTREG32(&DSI_REG->DSI_RACK, data_array);
   
   // TE + BTA
   data_array=0x24;
   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI_handle_TE TE + BTA !! \n");
   OUTREG32(&DSI_CMDQ_REG->data, data_array);
   
   //DSI_CMDQ_REG->data.byte0=0x24;
   //DSI_CMDQ_REG->data.byte1=0;
   //DSI_CMDQ_REG->data.byte2=0;
   //DSI_CMDQ_REG->data.byte3=0;
   
   OUTREGBIT(DSI_CMDQ_CTRL_REG,DSI_REG->DSI_CMDQ_SIZE,CMDQ_SIZE,1);
   
   OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
   OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
   
   // wait TE Trigger status
   mdelay(10);
   
   data_array=INREG32(&DSI_REG->DSI_INTSTA);
   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI INT state : %x !! \n", data_array);
   
   data_array=INREG32(&DSI_REG->DSI_TRIG_STA);
   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI TRIG TE status check : %x !! \n", data_array);
   //	} while(!(data_array&0x4));
   
   // RACT	
   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI Set RACT !! \n");
   data_array=1;
   OUTREG32(&DSI_REG->DSI_RACK, data_array);
   
   return DSI_STATUS_OK;
}


void DSI_Set_VM_CMD(LCM_PARAMS *lcm_params)
{
	OUTREGBIT(DSI_VM_CMD_CON_REG,DSI_REG->DSI_VM_CMD_CON,TS_VFP_EN,1);
	OUTREGBIT(DSI_VM_CMD_CON_REG,DSI_REG->DSI_VM_CMD_CON,VM_CMD_EN,1);

	return;
}


void DSI_Config_VDO_Timing(LCM_PARAMS *lcm_params)
{
    unsigned int line_byte;
    unsigned int horizontal_sync_active_byte;
    unsigned int horizontal_backporch_byte;
    unsigned int horizontal_frontporch_byte;
    unsigned int horizontal_blanking_byte;
    unsigned int rgb_byte;
    unsigned int dsiTmpBufBpp;

    #define LINE_PERIOD_US				(8 * line_byte * _dsiContext.bit_time_ns / 1000)
    

    if(lcm_params->dsi.data_format.format == LCM_DSI_FORMAT_RGB565)
        dsiTmpBufBpp = 2;
    else
        dsiTmpBufBpp = 3;
    
    OUTREGBIT(DSI_VSA_NL_REG,DSI_REG->DSI_VSA_NL,VSA_NL,lcm_params->dsi.vertical_sync_active);
    OUTREGBIT(DSI_VBP_NL_REG,DSI_REG->DSI_VBP_NL,VBP_NL,lcm_params->dsi.vertical_backporch);
    OUTREGBIT(DSI_VFP_NL_REG,DSI_REG->DSI_VFP_NL,VFP_NL,lcm_params->dsi.vertical_frontporch);
    OUTREGBIT(DSI_VACT_NL_REG,DSI_REG->DSI_VACT_NL,VACT_NL,lcm_params->dsi.vertical_active_line);
    
    line_byte							=	(lcm_params->dsi.horizontal_sync_active \
                                + lcm_params->dsi.horizontal_backporch \
                                + lcm_params->dsi.horizontal_frontporch \
                                + lcm_params->dsi.horizontal_active_pixel) * dsiTmpBufBpp;
    
    horizontal_sync_active_byte 		=	(lcm_params->dsi.horizontal_sync_active * dsiTmpBufBpp - 4);
    
    if (lcm_params->dsi.mode == SYNC_EVENT_VDO_MODE)
        horizontal_backporch_byte		=	((lcm_params->dsi.horizontal_backporch + lcm_params->dsi.horizontal_sync_active)* dsiTmpBufBpp - 4);
    else
        horizontal_backporch_byte		=	(lcm_params->dsi.horizontal_backporch * dsiTmpBufBpp - 4);
    
    horizontal_frontporch_byte			=	(lcm_params->dsi.horizontal_frontporch * dsiTmpBufBpp - 6);
    rgb_byte							=	(lcm_params->dsi.horizontal_active_pixel * dsiTmpBufBpp + 6);					
    horizontal_blanking_byte 		=	(lcm_params->dsi.horizontal_blanking_pixel * dsiTmpBufBpp - 4);

    /*	
    OUTREG32(&DSI_REG->DSI_LINE_NB, line_byte);
    OUTREG32(&DSI_REG->DSI_HSA_NB, horizontal_sync_active_byte);
    OUTREG32(&DSI_REG->DSI_HBP_NB, horizontal_backporch_byte);
    OUTREG32(&DSI_REG->DSI_HFP_NB, horizontal_frontporch_byte);
    OUTREG32(&DSI_REG->DSI_RGB_NB, rgb_byte);
    */

    OUTREGBIT(DSI_HSA_WC_REG,DSI_REG->DSI_HSA_WC,HSA_WC,(horizontal_sync_active_byte-6));
    OUTREGBIT(DSI_HBP_WC_REG,DSI_REG->DSI_HBP_WC,HBP_WC,(horizontal_backporch_byte-6));
    OUTREGBIT(DSI_HFP_WC_REG,DSI_REG->DSI_HFP_WC,HFP_WC,(horizontal_frontporch_byte-6));
    OUTREGBIT(DSI_BLLP_WC_REG,DSI_REG->DSI_BLLP_WC,BLLP_WC,(horizontal_blanking_byte-6));
    
    _dsiContext.vfp_period_us 		= LINE_PERIOD_US * lcm_params->dsi.vertical_frontporch / 1000;
    _dsiContext.vsa_vs_period_us	= LINE_PERIOD_US * 1 / 1000;
    _dsiContext.vsa_hs_period_us	= LINE_PERIOD_US * (lcm_params->dsi.vertical_sync_active - 2) / 1000;
    _dsiContext.vsa_ve_period_us	= LINE_PERIOD_US * 1 / 1000;
    _dsiContext.vbp_period_us		= LINE_PERIOD_US * lcm_params->dsi.vertical_backporch / 1000;
    
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - video timing, mode = %d \n", lcm_params->dsi.mode);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VSA : %d %d(us)\n", DSI_REG->DSI_VSA_NL, (_dsiContext.vsa_vs_period_us+_dsiContext.vsa_hs_period_us+_dsiContext.vsa_ve_period_us));
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VBP : %d %d(us)\n", DSI_REG->DSI_VBP_NL, _dsiContext.vbp_period_us);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VFP : %d %d(us)\n", DSI_REG->DSI_VFP_NL, _dsiContext.vfp_period_us);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VACT: %d \n", DSI_REG->DSI_VACT_NL);

    /*
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - HLB : %d \n", DSI_REG->DSI_LINE_NB);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - HSA : %d \n", DSI_REG->DSI_HSA_NB);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - HBP : %d \n", DSI_REG->DSI_HBP_NB);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - HFP : %d \n", DSI_REG->DSI_HFP_NB);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - RGB : %d \n", DSI_REG->DSI_RGB_NB);
    */
}


void DSI_PHY_clk_setting(LCM_PARAMS *lcm_params)
{
    unsigned int data_rate = 0;
    unsigned int ssc_disable = 0;
    unsigned int ssc_range = 0;
    unsigned int txdiv = 0;
    unsigned int delta = 4;//Delta is SSC range, default is 0%~-4%
    unsigned int pll_sdm_fra_en = 0;
    unsigned int pll_sdm_ssc_en = 0;
    DSI_PLL_CONFIG pll_config_value = {0, 0, 0, 0, 0, 0, 0, 0};


    if (lcm_params->type == LCM_TYPE_DPI)
    {
        data_rate = lcm_params->dpi.PLL_CLOCK*2;
    }
    else if (lcm_params->type == LCM_TYPE_DSI)
    {
        data_rate = lcm_params->dsi.PLL_CLOCK*2;
    }
    else
    {
        return;
    }

    if(0!=data_rate)
    {
        //if lcm_params->dsi.PLL_CLOCK=0, use other method
        if(data_rate > 1250)
        {
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", "[DISP] data_rate exceed limitation \n");
            ASSERT(0);
        }
        else if(data_rate >= 500)
        {
            txdiv = 1;
        }
        else if(data_rate >= 250)
        {
            txdiv = 2;
        }
        else if(data_rate >= 125)
        {
            txdiv = 4;
        }
        else if(data_rate > 62)
        {
            txdiv = 8;
        }
        else if(data_rate >= 50)
        {
            txdiv = 16;
        }
        else
        {
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", "[DISP] data_rate is too low,%d!!!\n", __LINE__);
            ASSERT(0);
        }

        //PLL txdiv config
        switch(txdiv)
        {
            case 1:
                pll_config_value.TXDIV0 = 0;
                pll_config_value.TXDIV1 = 0;
                break;
            case 2:
                pll_config_value.TXDIV0 = 1;
                pll_config_value.TXDIV1 = 0;
                break;
            case 4:
                pll_config_value.TXDIV0 = 2;
                pll_config_value.TXDIV1 = 0;
                break;
            case 8:
                pll_config_value.TXDIV0 = 2;
                pll_config_value.TXDIV1 = 1;
                break;
            case 16:
                pll_config_value.TXDIV0 = 2;
                pll_config_value.TXDIV1 = 2;
                break;
            default:
                break;
        }

        // PLL PCW config
        //pcw = data_Rate*4*txdiv/(26*2);//Post DIV =4, so need data_Rate*4
        pll_config_value.SDM_PCW = data_rate*txdiv/13;
    
        //SSC config
        //pmod = ROUND(1000*26MHz/fmod/2);fmod default is 30Khz, and this value not be changed
        //pmod = 433.33;
        if (lcm_params->type == LCM_TYPE_DPI)
        {
            ssc_disable = lcm_params->dpi.ssc_disable;
            ssc_range = lcm_params->dpi.ssc_range;
        }
        else
        {
            ssc_disable = lcm_params->dsi.ssc_disable;
            ssc_range = lcm_params->dsi.ssc_range;
        }

        if(1 != ssc_disable)
        {
            pll_config_value.SSC_PH_INIT = 1;
            pll_config_value.SSC_PRD = 0x1B1;//PRD=ROUND(pmod) = 433;

            //pll_sdm_delta = ROUND((2^18)*delta(10000)*pcw*0.000001/pmod)=ROUND(262144*delta(10000)*data_rate*txdiv*0.000001/(433.33*13))
            //=ROUND(262144*delta*data_rate*txdiv/(43333*13))=(262144*delta*data_rate*txdiv+43333*13/2)/(43333*13)
            if(0 != ssc_range)
            {
                delta = ssc_range;
            }
            ASSERT(delta<=8);
            pll_config_value.SSC_DELTA = (delta*data_rate*txdiv*262144+281664)/563329;
            pll_sdm_fra_en = 1;
            pll_sdm_ssc_en = 1;
            DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] PLL config:data_rate=%d,txdiv=%d,delta=%d,ssc_delta=0x%x\n",
                                         data_rate,txdiv,delta,pll_config_value.SSC_DELTA);
        }
    }
    else
    {
        if (lcm_params->type == LCM_TYPE_DPI)
        {
            pll_config_value.TXDIV1 = lcm_params->dpi.mipi_pll_clk_div2;  // div1
            pll_config_value.TXDIV0 = lcm_params->dpi.mipi_pll_clk_div1;  // div0
            pll_config_value.SDM_PCW = (lcm_params->dpi.mipi_pll_clk_fbk_div)<< 2;
        }
        else if (lcm_params->type == LCM_TYPE_DSI)
        {
            pll_config_value.TXDIV1 = lcm_params->dsi.pll_div2;  // div1
            pll_config_value.TXDIV0 = lcm_params->dsi.pll_div1;  // div0
            pll_config_value.SDM_PCW = (lcm_params->dsi.fbk_div)<< 2;
        }
        pll_config_value.SSC_PH_INIT = 1;
        pll_config_value.SSC_PRD = 0x1B1;//PRD=ROUND(pmod) = 433;
        pll_config_value.SSC_DELTA = 0x048B;

        pll_sdm_fra_en = 1;
        pll_sdm_ssc_en = 1;
    }


    OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_HS_BIAS_EN,1);

    OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CKEN,1);
    OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CORE_EN,1);
    //	msleep(10);
    mdelay(1);

    OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_CKG_LDOOUT_EN,1);
    OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_LDOCORE_EN,1);

    OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_PWR_ON,1);
    //	msleep(1);
    mdelay(1);

    OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_ISO_EN,0);

    OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PREDIV,0);
    OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_POSDIV,0);
    OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,pll_config_value.TXDIV1);
    OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,pll_config_value.TXDIV0);

    OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,pll_sdm_fra_en);
    if (0 != pll_config_value.SSC_PRD)
    {
        OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_PRD,pll_config_value.SSC_PRD);
    }
    if (0 != pll_config_value.SSC_PH_INIT)
    {
        OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_PH_INIT,pll_config_value.SSC_PH_INIT);
    }

    /*
        PCW bit 24~30 = floor(pcw)
        PCW bit 16~23 = (pcw - floor(pcw))*256
        PCW bit 8~15 = (pcw*256 - floor(pcw)*256)*256
        PCW bit 8~15 = (pcw*256*256 - floor(pcw)*256*256)*256
    */
    OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_H,pll_config_value.SDM_PCW & 0x7F);
    OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_16_23,(256*(data_rate*txdiv%13)/13) & 0xFF);
    OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_8_15,(256*(256*(data_rate*txdiv%13)%13)/13) & 0xFF);
    OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_0_7,(256*(256*(256*(data_rate*txdiv%13)%13)%13)/13) & 0xFF);

    if (0 != pll_config_value.SSC_DELTA)
    {
        OUTREGBIT(MIPITX_DSI_PLL_CON3_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON3,RG_DSI0_MPPLL_SDM_SSC_DELTA,pll_config_value.SSC_DELTA);
        OUTREGBIT(MIPITX_DSI_PLL_CON3_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON3,RG_DSI0_MPPLL_SDM_SSC_DELTA1,pll_config_value.SSC_DELTA);
    }

    if (lcm_params->type == LCM_TYPE_DSI)
    {
        OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_LDOOUT_EN,1);

        if(lcm_params->dsi.LANE_NUM > 0)
        {
            OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_LDOOUT_EN,1);
        }
        if(lcm_params->dsi.LANE_NUM > 1)
        {
            OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_LDOOUT_EN,1);
        }
        if(lcm_params->dsi.LANE_NUM > 2)
        {
            OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_LDOOUT_EN,1);
        }
    }

    OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PLL_EN,1);
    //	msleep(20);
    mdelay(1);

    OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_EN,pll_sdm_ssc_en);
    
    // default POSDIV by 4
    OUTREGBIT(MIPITX_DSI_PLL_TOP_REG,DSI_PHY_REG->MIPITX_DSI_PLL_TOP,RG_MPPLL_PRESERVE_L,3);
    
    //Special case for internal phone card detection pin mux with MIPI data lane 2 TX
    #if defined(GPIO_SDHC_EINT_PIN)
    if (GPIO_SDHC_EINT_PIN == GPIO86) 
    {
        if ((lcm_params->type == LCM_TYPE_DSI) && (lcm_params->dsi.LANE_NUM > 2))
        {
            OUTREGBIT(MIPITX_DSI_GPIO_EN_REG,DSI_PHY_REG->MIPITX_DSI_GPIO_EN,RG_DSI0_GPI6_EN,1);
            OUTREGBIT(MIPITX_DSI_GPIO_EN_REG,DSI_PHY_REG->MIPITX_DSI_GPIO_EN,RG_DSI0_GPI7_EN,1);
        }
    }
    #endif
}


#ifdef BUILD_UBOOT
void DSI_PHY_clk_switch(bool on, LCM_PARAMS *lcm_params)
{
   MIPITX_DSI0_CON_REG mipitx_dsi0_con=DSI_PHY_REG->MIPITX_DSI0_CON;
   
   if(on)
      mipitx_dsi0_con.RG_DSI0_LDOCORE_EN=1;
   else
      mipitx_dsi0_con.RG_DSI0_LDOCORE_EN=0;
   
   OUTREG32(&DSI_PHY_REG->MIPITX_DSI0_CON,AS_UINT32(&mipitx_dsi0_con));
}

#else
void DSI_PHY_clk_switch(bool on, LCM_PARAMS *lcm_params)
{
    if(on){//workaround: do nothing
        if (lcm_params->type == LCM_TYPE_DSI)
        {
            //pad_tie_low_en = 0
            OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_PAD_TIE_LOW_EN,0);
            
            // switch to mipi tx dsi mode
            OUTREGBIT(MIPITX_DSI_SW_CTRL_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL,SW_CTRL_EN,0);
        }
    }
    else
    {
        // pre_oe/oe = 1
        OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNTC_LPTX_PRE_OE,1);
        OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNTC_LPTX_OE,1);
        OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT0_LPTX_PRE_OE,1);
        OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT0_LPTX_OE,1);
        OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT1_LPTX_PRE_OE,1);
        OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT1_LPTX_OE,1);
    
        //Special case for internal phone card detection pin mux with MIPI data lane 2 TX
        #if defined(GPIO_SDHC_EINT_PIN)
        if (GPIO_SDHC_EINT_PIN == GPIO86) 
        {
            if ((lcm_params->type == LCM_TYPE_DSI) && (lcm_params->dsi.LANE_NUM > 2))
            {
                OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT2_LPTX_PRE_OE,0);
                OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT2_LPTX_OE,0);    
            }
            else
            {
                OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT2_LPTX_PRE_OE,1);
                OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT2_LPTX_OE,1);
            }
        }
        else
        #endif
        {
            OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT2_LPTX_PRE_OE,1);
            OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT2_LPTX_OE,1);
        }

        if (lcm_params->type == LCM_TYPE_DSI)
        {
            // switch to mipi tx sw mode
            OUTREGBIT(MIPITX_DSI_SW_CTRL_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL,SW_CTRL_EN,1);
        }

        // disable mipi clock
        OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PLL_EN,0);
        mdelay(1);

        if (lcm_params->type == LCM_TYPE_DSI)
        {
            OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_PAD_TIE_LOW_EN,1);
        }

        OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_HS_BIAS_EN,0);
        if (lcm_params->type == LCM_TYPE_DSI)
        {
            OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_LDOOUT_EN,0);
            OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_LDOOUT_EN,0);
            OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_LDOOUT_EN,0);
            OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_LDOOUT_EN,0);
        }
        mdelay(1);

        OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_CKG_LDOOUT_EN,0);
        OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_LDOCORE_EN,0);

        OUTREGBIT(MIPITX_DSI_PLL_TOP_REG,DSI_PHY_REG->MIPITX_DSI_PLL_TOP,RG_MPPLL_PRESERVE_L,0);
        
        OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,0);
        OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_ISO_EN,1);
        OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_PWR_ON,0);        
        mdelay(1);

        OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CKEN,0);
        OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CORE_EN,0);
        mdelay(1);
    }
}
#endif


void DSI_PHY_TIMCONFIG(LCM_PARAMS *lcm_params)
{
    DSI_PHY_TIMCON0_REG timcon0;
    DSI_PHY_TIMCON1_REG timcon1;	
    DSI_PHY_TIMCON2_REG timcon2;
    DSI_PHY_TIMCON3_REG timcon3;
    unsigned int div1 = 0;
    unsigned int div2 = 0;
    unsigned int fbk_div = 0;
    unsigned int lane_no = lcm_params->dsi.LANE_NUM;
    
    //	unsigned int div2_real;
    unsigned int cycle_time;
    unsigned int ui;
    unsigned int hs_trail_m, hs_trail_n;

    unsigned int data_rate;
    unsigned int txdiv = 0;


    data_rate = lcm_params->dsi.PLL_CLOCK*2;
    if(0 != data_rate)
    {
        //if lcm_params->dsi.PLL_CLOCK=0, use other method
        if(data_rate > 1250)
        {
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", "[DISP] data_rate exceed limitation \n");
            ASSERT(0);
        }
        else if(data_rate >= 500)
        {
            txdiv = 1;
        }
        else if(data_rate >= 250)
        {
            txdiv = 2;
        }
        else if(data_rate >= 125)
        {
            txdiv = 4;
        }
        else if(data_rate > 62)
        {
            txdiv = 8;
        }
        else if(data_rate >= 50)
        {
            txdiv = 16;
        }
        else
        {
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", "[DISP] data_rate is too low,%d!!!\n", __LINE__);
            ASSERT(0);
        }

        //PLL txdiv config
        switch(txdiv)
        {
            case 1:
                div1 = 0;
                div2 = 0;
                break;
            case 2:
                div1 = 1;
                div2 = 0;
                break;
            case 4:
                div1 = 2;
                div2 = 0;
                break;
            case 8:
                div1 = 2;
                div2 = 1;
                break;
            case 16:
                div1 = 2;
                div2 = 2;
                break;
            default:
                break;
        }

        fbk_div = data_rate / 26;
    }
    else
    {
        div1 = lcm_params->dsi.pll_div1;
        div2 = lcm_params->dsi.pll_div2;
        fbk_div = lcm_params->dsi.fbk_div;
    }

    switch(div1)
    {
        case 0:
            div1 = 1;
            break;
        case 1:
            div1 = 2;
            break;
        case 2:
        case 3:
            div1 = 4;
            break;
        default:
            printk("[DISP] div1 should be less than 4!!\n");
            div1 = 4;
            break;
    }
    
    switch(div2)
    {
        case 0:
            div2 = 1;
            break;
        case 1:
            div2 = 2;
            break;
        case 2:
        case 3:
            div2 = 4;
            break;
        default:
            printk("[DISP] div2 should be less than 4!!\n");
            div2 = 4;
            break;
    }
    
    cycle_time=(1000*4*div2*div1)/(fbk_div*26)+0x01;
    ui=(1000*div2*div1)/(fbk_div*26*0x2)+0x01;
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, Cycle Time = %d(ns), Unit Interval = %d(ns). div1 = %d, div2 = %d, fbk_div = %d, lane# = %d \n", cycle_time, ui, div1, div2, fbk_div, lane_no);


    #define NS_TO_CYCLE(n, c)	((n) / (c))
    
    hs_trail_m=1;
    hs_trail_n= (lcm_params->dsi.HS_TRAIL == 0) ? NS_TO_CYCLE(((hs_trail_m * 0x4) + 60), cycle_time) : lcm_params->dsi.HS_TRAIL;
    // +3 is recommended from designer becauase of HW latency
    timcon0.HS_TRAIL	= ((hs_trail_m > hs_trail_n) ? hs_trail_m : hs_trail_n) + 0x0a;
    
    timcon0.HS_PRPR 	= (lcm_params->dsi.HS_PRPR == 0) ? NS_TO_CYCLE((0x40 + 0x5 * ui), cycle_time) : lcm_params->dsi.HS_PRPR;
    // HS_PRPR can't be 1.
    if (timcon0.HS_PRPR == 0)
       timcon0.HS_PRPR = 1;
    
    timcon0.HS_ZERO 	= (lcm_params->dsi.HS_ZERO == 0) ? NS_TO_CYCLE((0xC8 + 0x0a * ui), cycle_time) : lcm_params->dsi.HS_ZERO;
    if (timcon0.HS_ZERO > timcon0.HS_PRPR)
       timcon0.HS_ZERO -= timcon0.HS_PRPR;
    
    timcon0.LPX 		= (lcm_params->dsi.LPX == 0) ? NS_TO_CYCLE(0x50, cycle_time) : lcm_params->dsi.LPX;
    if(timcon0.LPX == 0) 
       timcon0.LPX = 1;
 
    //	timcon1.TA_SACK 	= (lcm_params->dsi.TA_SACK == 0) ? 1 : lcm_params->dsi.TA_SACK;
    timcon1.TA_GET 		= (lcm_params->dsi.TA_GET == 0) ? (0x5 * timcon0.LPX) : lcm_params->dsi.TA_GET;
    timcon1.TA_SURE 	= (lcm_params->dsi.TA_SURE == 0) ? (0x3 * timcon0.LPX / 0x2) : lcm_params->dsi.TA_SURE;
    timcon1.TA_GO 		= (lcm_params->dsi.TA_GO == 0) ? (0x4 * timcon0.LPX) : lcm_params->dsi.TA_GO;
    // --------------------------------------------------------------
    //  NT35510 need fine tune timing
    //  Data_hs_exit = 60 ns + 128UI 
    //  Clk_post = 60 ns + 128 UI. 
    // --------------------------------------------------------------
    timcon1.DA_HS_EXIT  = (lcm_params->dsi.DA_HS_EXIT == 0) ? NS_TO_CYCLE((0x3c + 0x80 * ui), cycle_time) : lcm_params->dsi.DA_HS_EXIT;
    
    timcon2.CLK_TRAIL 	= ((lcm_params->dsi.CLK_TRAIL == 0) ? NS_TO_CYCLE(0x64, cycle_time) : lcm_params->dsi.CLK_TRAIL) + 0x0a;
    // CLK_TRAIL can't be 1.
    if (timcon2.CLK_TRAIL < 2)
       timcon2.CLK_TRAIL = 2;
    
    //	timcon2.LPX_WAIT 	= (lcm_params->dsi.LPX_WAIT == 0) ? 1 : lcm_params->dsi.LPX_WAIT;
    timcon2.CONT_DET 	= lcm_params->dsi.CONT_DET;
    timcon2.CLK_ZERO	= (lcm_params->dsi.CLK_ZERO == 0) ? NS_TO_CYCLE(0x190, cycle_time) : lcm_params->dsi.CLK_ZERO;
    
    timcon3.CLK_HS_PRPR	= (lcm_params->dsi.CLK_HS_PRPR == 0) ? NS_TO_CYCLE(0x40, cycle_time) : lcm_params->dsi.CLK_HS_PRPR;
    if(timcon3.CLK_HS_PRPR == 0) 
       timcon3.CLK_HS_PRPR = 1;
    timcon3.CLK_HS_EXIT= (lcm_params->dsi.CLK_HS_EXIT == 0) ? (2 * timcon0.LPX) : lcm_params->dsi.CLK_HS_EXIT;
    timcon3.CLK_HS_POST= (lcm_params->dsi.CLK_HS_POST == 0) ? NS_TO_CYCLE((0x3c + 0x80 * ui), cycle_time) : lcm_params->dsi.CLK_HS_POST;
    
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, HS_TRAIL = %d, HS_ZERO = %d, HS_PRPR = %d, LPX = %d, TA_GET = %d, TA_SURE = %d, TA_GO = %d, CLK_TRAIL = %d, CLK_ZERO = %d, CLK_HS_PRPR = %d \n", \
    timcon0.HS_TRAIL, timcon0.HS_ZERO, timcon0.HS_PRPR, timcon0.LPX, timcon1.TA_GET, timcon1.TA_SURE, timcon1.TA_GO, timcon2.CLK_TRAIL, timcon2.CLK_ZERO, timcon3.CLK_HS_PRPR);		
    
    OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,LPX,timcon0.LPX);
    OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_PRPR,timcon0.HS_PRPR);
    OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_ZERO,timcon0.HS_ZERO);
    OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_TRAIL,timcon0.HS_TRAIL);
 
    OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GO,timcon1.TA_GO);
    OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_SURE,timcon1.TA_SURE);
    OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GET,timcon1.TA_GET);
    OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,DA_HS_EXIT,timcon1.DA_HS_EXIT);
 
    OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CONT_DET,timcon2.CONT_DET);
    OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_ZERO,timcon2.CLK_ZERO);
    OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_TRAIL,timcon2.CLK_TRAIL);
 
    OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_PRPR,timcon3.CLK_HS_PRPR);
    OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_POST,timcon3.CLK_HS_POST);
    OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_EXIT,timcon3.CLK_HS_EXIT);
}


void DSI_clk_ULP_mode(bool enter)
{
   if(enter && !DSI_clk_ULP_state()) {
      // make sure leave high speed mode first
      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_HS_TX_EN, 0);
      mdelay(1);
      
      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_ULPM_EN, 0);
      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_ULPM_EN, 1);
      mdelay(1);
   }
   else if (!enter && DSI_clk_ULP_state()) {
      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_ULPM_EN, 0);
      mdelay(1);

      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_WAKEUP_EN, 1);
      mdelay(1);

      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_WAKEUP_EN, 0);
      mdelay(1);
   }
}


void DSI_clk_HS_mode(bool enter)
{
   if(enter && !DSI_clk_HS_state()) {
      // make sure leave ultra low power mode first
      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_ULPM_EN, 0);
      mdelay(1);

      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_HS_TX_EN, 1);
      mdelay(1);
   }
   else if (!enter && DSI_clk_HS_state()) {
      OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_HS_TX_EN, 0);
      //mdelay(1);
   }
}

DSI_STATUS DSI_Wait_VDO_Idle()
{
    static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
    long ret;
    DSI_SetMode(0);
    
    ret = wait_event_interruptible_timeout(_dsi_wait_vm_done_queue, 
                                                                    !_IsEngineBusy(),
                                                                    WAIT_TIMEOUT);
    
    if (0 == ret) {
        xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine read ready timeout!!!\n");
        
        DSI_DumpRegisters();
        ///do necessary reset here
        DSI_Reset();
    }
    DSI_SetMode(lcm_params->dsi.mode);

    return DSI_STATUS_OK;
}

void DSI_lane0_ULP_mode(bool enter)
{
   if(enter && !DSI_lane0_ULP_state()) {
      // suspend
      OUTREGBIT(DSI_PHY_LD0CON_REG, DSI_REG->DSI_PHY_LD0CON, L0_ULPM_EN, 0);
      OUTREGBIT(DSI_PHY_LD0CON_REG, DSI_REG->DSI_PHY_LD0CON, L0_ULPM_EN, 1);
      mdelay(1);
   }
   else if (!enter && DSI_lane0_ULP_state()) {
      // resume
      OUTREGBIT(DSI_PHY_LD0CON_REG, DSI_REG->DSI_PHY_LD0CON, L0_ULPM_EN, 0);
      mdelay(1);

      OUTREGBIT(DSI_PHY_LD0CON_REG, DSI_REG->DSI_PHY_LD0CON, L0_WAKEUP_EN, 1);
      mdelay(1);

      OUTREGBIT(DSI_PHY_LD0CON_REG, DSI_REG->DSI_PHY_LD0CON, L0_WAKEUP_EN, 0);
      mdelay(1);
   }
}


bool DSI_clk_ULP_state(void)
{
   return DSI_REG->DSI_PHY_LCCON.LC_ULPM_EN ? TRUE : FALSE;
}


bool DSI_clk_HS_state(void)
{
   return DSI_REG->DSI_PHY_LCCON.LC_HS_TX_EN ? TRUE : FALSE;
}


bool DSI_lane0_ULP_state(void)
{
   return DSI_REG->DSI_PHY_LD0CON.L0_ULPM_EN ? TRUE : FALSE;
}


// called by DPI ISR
void DSI_handle_esd_recovery(void)
{
}


// called by "esd_recovery_kthread"
bool DSI_esd_check(void)
{
#ifndef MT65XX_NEW_DISP
    bool result = false;
    

    if(dsi_esd_recovery)
        result = true;
    else
        result = false;
    
    dsi_esd_recovery = false;

#else
    DSI_MODE_CTRL_REG mode_ctl, mode_ctl_backup;
    bool result = false;


    //backup video mode
    OUTREG32(&mode_ctl_backup, AS_UINT32(&DSI_REG->DSI_MODE_CTRL));
    OUTREG32(&mode_ctl, AS_UINT32(&DSI_REG->DSI_MODE_CTRL));
    //set to cmd mode
    mode_ctl.MODE = 0;
    OUTREG32(&DSI_REG->DSI_MODE_CTRL, AS_UINT32(&mode_ctl));

    #if ENABLE_DSI_INTERRUPT 			//wait video mode done
    {
        static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
        long ret;
        
        wait_vm_done_irq = true;
        ret = wait_event_interruptible_timeout(_dsi_wait_vm_done_queue, 
                                                                         !_IsEngineBusy(),
                                                                         WAIT_TIMEOUT);

        if (0 == ret) {
            xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine read ready timeout!!!\n");
            
            DSI_DumpRegisters();
            ///do necessary reset here
            DSI_Reset();
            wait_vm_done_irq = false;

            return 0;
        }
    }
    #else
    {
        unsigned int read_timeout_ms = 100;

        #ifdef DDI_DRV_DEBUG_LOG_ENABLE
            DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Start polling VM done ready!!!\n");
        #endif

        while(DSI_REG->DSI_INTSTA.VM_DONE== 0)  //clear
        {
            ///keep polling
            msleep(1);
            read_timeout_ms --;
            
            if(read_timeout_ms == 0)
            {
                DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Polling DSI VM done timeout!!!\n");
                DSI_DumpRegisters();
                
                DSI_Reset();
                return 0;
            }
        }
        OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,VM_DONE,0);

        #ifdef DDI_DRV_DEBUG_LOG_ENABLE
            DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " End polling DSI VM done ready!!!\n");
        #endif
    }
    #endif

    //read DriverIC and check ESD
    result = lcm_drv->esd_check();
    //restore video mode
    if(!result)
        OUTREG32(&DSI_REG->DSI_MODE_CTRL, AS_UINT32(&mode_ctl_backup));

#endif
    wait_vm_done_irq = false;

    return result;
}


void DSI_set_int_TE(bool enable, unsigned int period)
{
#ifndef MT65XX_NEW_DISP
    dsi_int_te_enabled = enable;
 
    if(period<1)
        period = 1;
 
    dsi_int_te_period = period;
    dsi_dpi_isr_count = 0;
#endif
}


// called by DPI ISR.
bool DSI_handle_int_TE(void)
{
#ifndef MT65XX_NEW_DISP
   DSI_T0_INS t0;
   long int dsi_current_time;
   
   if (!DSI_REG->DSI_MODE_CTRL.MODE)
      return false;
   
   dsi_current_time = get_current_time_us();
   
   if(DSI_REG->DSI_STATE_DBG3.TCON_STATE == DSI_VDO_VFP_STATE)
   {
      udelay(_dsiContext.vfp_period_us / 2);
      
      if ((DSI_REG->DSI_STATE_DBG3.TCON_STATE == DSI_VDO_VFP_STATE) && DSI_REG->DSI_STATE_DBG0.CTL_STATE_0 == 0x1)
      {
         // Can't do int. TE check while INUSE FB number is not 0 because later disable/enable DPI will set INUSE FB to number 0.
         if(DPI_REG->STATUS.FB_INUSE != 0)
            return false;
         
         DSI_clk_HS_mode(0);
         
         OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
         DPI_DisableClk();
         DSI_SetMode(CMD_MODE);
         OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
         //DSI_Reset();
         
         t0.CONFG = 0x20;		///TE
         t0.Data0 = 0;
         t0.Data_ID = 0;
         t0.Data1 = 0;
         
         OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
         OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
         
         // Enable RD_RDY INT for polling it's status later
         OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
         DSI_Start();
         
         while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  // polling RD_RDY
         {
            if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us)
            {
               xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for internal TE time-out for %d (us)!!!\n", _dsiContext.vfp_period_us);
               
               ///do necessary reset here
               OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
               DSI_Reset();
               
               return true;
            }
         }
         
         // Write clear RD_RDY
         OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
         OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
         // Write clear CMD_DONE
         OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);
         
         // Restart video mode. (with VSA ahead)
         DSI_SetMode(SYNC_PULSE_VDO_MODE);
         DSI_clk_HS_mode(1);
         DPI_EnableClk();
         DSI_Start();
      }
   }
#endif

   return false;
}


void DSI_set_noncont_clk(bool enable, unsigned int period)
{
    dsi_noncont_clk_enabled = enable;
    dsi_noncont_clk_period = period;
}


// called by DPI ISR.
void DSI_handle_noncont_clk(void)
{
#ifndef MT65XX_NEW_DISP
   unsigned int state;
   long int dsi_current_time;
   
   if (!DSI_REG->DSI_MODE_CTRL.MODE)
      return ;
   
   state = DSI_REG->DSI_STATE_DBG3.TCON_STATE;
   dsi_current_time = get_current_time_us();
   
   switch(state)
   {
      case DSI_VDO_VSA_VS_STATE:
         while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_HS_STATE)
         {
            if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_vs_period_us)
            {
               xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_HS_STATE, _dsiContext.vsa_vs_period_us);
               return ;
            }
         }
         break;
   
      case DSI_VDO_VSA_HS_STATE:
         while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VE_STATE)
         {
            if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_hs_period_us)
            {
               xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VE_STATE, _dsiContext.vsa_hs_period_us);
               return ;
            }
         }
         break;
      
      case DSI_VDO_VSA_VE_STATE:
         while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VBP_STATE)
         {
            if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_ve_period_us)
            {
               xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VBP_STATE, _dsiContext.vsa_ve_period_us);
               return ;
            }
         }
         break;
      
      case DSI_VDO_VBP_STATE:
         xlog_printk(ANDROID_LOG_WARN, "DSI", "Can't do clock switch in DSI_VDO_VBP_STATE !!!\n");
         break;
      
      case DSI_VDO_VACT_STATE:
         while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VFP_STATE)
         {
            if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
            {
               xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VFP_STATE, _dsiContext.vfp_period_us );
               return ;
            }
         }
         break;
      
      case DSI_VDO_VFP_STATE:
         while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VS_STATE)
         {
            if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
            {
               xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VS_STATE, _dsiContext.vfp_period_us );
               return ;
            }
         }
         break;
      
      default :
         xlog_printk(ANDROID_LOG_ERROR, "DSI", "invalid state = %x \n", state);
         return ;
   }
   
   // Clock switch HS->LP->HS
   DSI_clk_HS_mode(0);
   udelay(1);
   DSI_clk_HS_mode(1);
#endif
}


void DSI_set_cmdq_V2(unsigned cmd, unsigned char count, unsigned char *para_list, unsigned char force_update)
{
    UINT32 i;
    UINT32 goto_addr, mask_para, set_para;

    DSI_T0_INS t0;	
    DSI_T2_INS t2;	


    if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
        DSI_VM_CMD_CON_REG vm_cmdq;

        OUTREG32(&vm_cmdq, AS_UINT32(&DSI_REG->DSI_VM_CMD_CON));
        printk("set cmdq in VDO mode in set_cmdq_V2\n");
        if (cmd < 0xB0)
        {
            if (count > 1)
            {
                vm_cmdq.LONG_PKT = 1;
                vm_cmdq.CM_DATA_ID = DSI_DCS_LONG_PACKET_ID;
                vm_cmdq.CM_DATA_0 = count+1;
                OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
                
                goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
                mask_para = (0xFF<<((goto_addr&0x3)*8));
                set_para = (cmd<<((goto_addr&0x3)*8));
                MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                
                for(i=0; i<count; i++)
                {
                    goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (para_list[i]<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                }
            }
            else
            {
                vm_cmdq.LONG_PKT = 0;
                vm_cmdq.CM_DATA_0 = cmd;
                if (count)
                {
                    vm_cmdq.CM_DATA_ID = DSI_DCS_SHORT_PACKET_ID_1;
                    vm_cmdq.CM_DATA_1 = para_list[0];
                }
                else
                {
                    vm_cmdq.CM_DATA_ID = DSI_DCS_SHORT_PACKET_ID_0;
                    vm_cmdq.CM_DATA_1 = 0;
                }
                OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
            }
        }
        else{
            if (count > 1)
            {
                vm_cmdq.LONG_PKT = 1;
                vm_cmdq.CM_DATA_ID = DSI_GERNERIC_LONG_PACKET_ID;
                vm_cmdq.CM_DATA_0 = count+1;
                OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
                
                goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
                mask_para = (0xFF<<((goto_addr&0x3)*8));
                set_para = (cmd<<((goto_addr&0x3)*8));
                MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                
                for(i=0; i<count; i++)
                {
                    goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (para_list[i]<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                }
            }
            else
            {
                vm_cmdq.LONG_PKT = 0;
                vm_cmdq.CM_DATA_0 = cmd;
                if (count)
                {
                    vm_cmdq.CM_DATA_ID = DSI_GERNERIC_SHORT_PACKET_ID_2;
                    vm_cmdq.CM_DATA_1 = para_list[0];
                }
                else
                {
                    vm_cmdq.CM_DATA_ID = DSI_GERNERIC_SHORT_PACKET_ID_1;
                    vm_cmdq.CM_DATA_1 = 0;
                }
                OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
            }
        }

        //start DSI VM CMDQ
        if(force_update){
            MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagStart, *(unsigned int*)(&DSI_VM_CMD_REG->data[0]), *(unsigned int*)(&DSI_VM_CMD_REG->data[1]));
            DSI_EnableVM_CMD();
            
            //must wait VM CMD done?
            MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagEnd, *(unsigned int*)(&DSI_VM_CMD_REG->data[2]), *(unsigned int*)(&DSI_VM_CMD_REG->data[3]));
        }
    }
    else{
        #ifdef ENABLE_DSI_ERROR_REPORT
        if ((para_list[0] & 1))
        {
            memset(_dsi_cmd_queue, 0, sizeof(_dsi_cmd_queue));
            memcpy(_dsi_cmd_queue, para_list, count);
            _dsi_cmd_queue[(count+3)/4*4] = 0x4;
            count = (count+3)/4*4 + 4;
            para_list = (unsigned char*) _dsi_cmd_queue;
        }
        else
        {
            para_list[0] |= 4;
        }
        #endif

        DSI_WaitForEngineNotBusy();
        if (cmd < 0xB0)
        {
            if (count > 1)
            {
                t2.CONFG = 2;
                t2.Data_ID = DSI_DCS_LONG_PACKET_ID;
                t2.WC16 = count+1;
                
                OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t2));
                
                goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
                mask_para = (0xFF<<((goto_addr&0x3)*8));
                set_para = (cmd<<((goto_addr&0x3)*8));
                MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                
                for(i=0; i<count; i++)
                {
                    goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (para_list[i]<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                }
                
                OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4); 		
            }
            else
            {
                t0.CONFG = 0;
                t0.Data0 = cmd;
                if (count)
                {
                    t0.Data_ID = DSI_DCS_SHORT_PACKET_ID_1;
                    t0.Data1 = para_list[0];
                }
                else
                {
                    t0.Data_ID = DSI_DCS_SHORT_PACKET_ID_0;
                    t0.Data1 = 0;
                }
                OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
                OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
            }
        }
        else
        {
            if (count > 1)
            {
                t2.CONFG = 2;
                t2.Data_ID = DSI_GERNERIC_LONG_PACKET_ID;
                t2.WC16 = count+1;
                
                OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t2));
                
                goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
                mask_para = (0xFF<<((goto_addr&0x3)*8));
                set_para = (cmd<<((goto_addr&0x3)*8));
                MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                
                for(i=0; i<count; i++)
                {
                    goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (para_list[i]<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                }
                
                OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);
            }
            else
            {
                t0.CONFG = 0;
                t0.Data0 = cmd;
                if (count)
                {
                    t0.Data_ID = DSI_GERNERIC_SHORT_PACKET_ID_2;
                    t0.Data1 = para_list[0];
                }
                else
                {
                    t0.Data_ID = DSI_GERNERIC_SHORT_PACKET_ID_1;
                    t0.Data1 = 0;
                }
                OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
                OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
            }
        }
        
        //	for (i = 0; i < AS_UINT32(&DSI_REG->DSI_CMDQ_SIZE); i++)
        //        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V2. DSI_CMDQ+%04x : 0x%08x\n", i*4, INREG32(DSI_BASE + 0x180 + i*4));
        
        if(force_update)
        {
            MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagStart, *(unsigned int*)(&DSI_CMDQ_REG->data[0]), *(unsigned int*)(&DSI_CMDQ_REG->data[1]));
            DSI_Start();
            for(i=0; i<10; i++) ;
            DSI_WaitForEngineNotBusy();
            MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagEnd, *(unsigned int*)(&DSI_CMDQ_REG->data[2]), *(unsigned int*)(&DSI_CMDQ_REG->data[3]));
        }
    }
    
}


void DSI_set_cmdq_V3(LCM_setting_table_V3 *para_tbl, unsigned int size, unsigned char force_update)
{
    UINT32 i;
    UINT32 goto_addr, mask_para, set_para;
    DSI_T0_INS t0;	
    DSI_T2_INS t2;	
    
    UINT32 index = 0;
    
    unsigned char data_id, cmd, count;
    unsigned char *para_list;
    
    do {
        data_id = para_tbl[index].id;
        cmd = para_tbl[index].cmd;
        count = para_tbl[index].count;
        para_list = para_tbl[index].para_list;
        
        if (data_id == REGFLAG_ESCAPE_ID && cmd == REGFLAG_DELAY_MS_V3)
        {
            mdelay(count);
            DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V3[%d]. Delay %d (ms) \n", index, count);
            
            continue;
        }
        
        if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
            DSI_VM_CMD_CON_REG vm_cmdq;

            OUTREG32(&vm_cmdq, AS_UINT32(&DSI_REG->DSI_VM_CMD_CON));
            printk("set cmdq in VDO mode\n");
            if (count > 1)
            {
                vm_cmdq.LONG_PKT = 1;
                vm_cmdq.CM_DATA_ID = data_id;
                vm_cmdq.CM_DATA_0 = count+1;
                OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
                
                goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
                mask_para = (0xFF<<((goto_addr&0x3)*8));
                set_para = (cmd<<((goto_addr&0x3)*8));
                MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                
                for(i=0; i<count; i++)
                {
                    goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (para_list[i]<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                }
            }
            else
            {
                vm_cmdq.LONG_PKT = 0;
                vm_cmdq.CM_DATA_0 = cmd;
                if (count)
                {
                    vm_cmdq.CM_DATA_ID = data_id;
                    vm_cmdq.CM_DATA_1 = para_list[0];
                }
                else
                {
                    vm_cmdq.CM_DATA_ID = data_id;
                    vm_cmdq.CM_DATA_1 = 0;
                }
                OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
            }
            //start DSI VM CMDQ
            if(force_update){
                MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagStart, *(unsigned int*)(&DSI_VM_CMD_REG->data[0]), *(unsigned int*)(&DSI_VM_CMD_REG->data[1]));
                DSI_EnableVM_CMD();
                
                //must wait VM CMD done?
                MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagEnd, *(unsigned int*)(&DSI_VM_CMD_REG->data[2]), *(unsigned int*)(&DSI_VM_CMD_REG->data[3]));
            }
        }
        else{
            DSI_WaitForEngineNotBusy();

            {
                //for(i = 0; i < sizeof(DSI_CMDQ_REG->data0) / sizeof(DSI_CMDQ); i++)
                //	OUTREG32(&DSI_CMDQ_REG->data0[i], 0);
                //memset(&DSI_CMDQ_REG->data[0], 0, sizeof(DSI_CMDQ_REG->data[0]));
                OUTREG32(&DSI_CMDQ_REG->data[0], 0);
                
                if (count > 1)
                {
                    t2.CONFG = 2;
                    t2.Data_ID = data_id;
                    t2.WC16 = count+1;
                    
                    OUTREG32(&DSI_CMDQ_REG->data[0].byte0, AS_UINT32(&t2));
                    
                    goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (cmd<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                    
                    for(i=0; i<count; i++)
                    {
                        goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
                        mask_para = (0xFF<<((goto_addr&0x3)*8));
                        set_para = (para_list[i]<<((goto_addr&0x3)*8));
                        MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                    }
                    
                    OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4); 		
                }
                else
                {
                    t0.CONFG = 0;
                    t0.Data0 = cmd;
                    if (count)
                    {
                        t0.Data_ID = data_id;
                        t0.Data1 = para_list[0];
                    }
                    else
                    {
                        t0.Data_ID = data_id;
                        t0.Data1 = 0;
                    }
                    OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
                    OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
                }
                
                for (i = 0; i < AS_UINT32(&DSI_REG->DSI_CMDQ_SIZE); i++)
                    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V3[%d]. DSI_CMDQ+%04x : 0x%08x\n", index, i*4, INREG32(DSI_BASE + 0x180 + i*4));
                
                if(force_update)
                {
                    MMProfileLog(MTKFB_MMP_Events.DSICmd, MMProfileFlagStart);
                    DSI_Start();
                    for(i=0; i<10; i++) ;
                    DSI_WaitForEngineNotBusy();
                    MMProfileLog(MTKFB_MMP_Events.DSICmd, MMProfileFlagEnd);
                }
            }
        }
    } while (++index < size);
    
}


void DSI_set_cmdq(unsigned int *pdata, unsigned int queue_size, unsigned char force_update)
{
    UINT32 i;
    
    //	_WaitForEngineNotBusy();
    
    if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
        DSI_VM_CMD_CON_REG vm_cmdq;

        OUTREG32(&vm_cmdq, AS_UINT32(&DSI_REG->DSI_VM_CMD_CON));
        printk("set cmdq in VDO mode\n");
        if(queue_size > 1){//long packet
            unsigned int i = 0;

            vm_cmdq.LONG_PKT = 1;
            vm_cmdq.CM_DATA_ID = ((pdata[0] >> 8) & 0xFF);
            vm_cmdq.CM_DATA_0 = ((pdata[0] >> 16) & 0xFF);
            vm_cmdq.CM_DATA_1 = 0;
            OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
            for(i=0;i<queue_size-1;i++)
                OUTREG32(&DSI_VM_CMD_REG->data[i], AS_UINT32((pdata+i+1)));
        }
        else{
            vm_cmdq.LONG_PKT = 0;
            vm_cmdq.CM_DATA_ID = ((pdata[0] >> 8) & 0xFF);
            vm_cmdq.CM_DATA_0 = ((pdata[0] >> 16) & 0xFF);
            vm_cmdq.CM_DATA_1 = ((pdata[0] >> 24) & 0xFF);
            OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
        }
        //start DSI VM CMDQ
        if(force_update){
            MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagStart, *(unsigned int*)(&DSI_VM_CMD_REG->data[0]), *(unsigned int*)(&DSI_VM_CMD_REG->data[1]));
            DSI_EnableVM_CMD();
            
            //must wait VM CMD done?
            MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagEnd, *(unsigned int*)(&DSI_VM_CMD_REG->data[2]), *(unsigned int*)(&DSI_VM_CMD_REG->data[3]));
        }
    }
    else{
        ASSERT(queue_size<=32);
        DSI_WaitForEngineNotBusy();
        #ifdef ENABLE_DSI_ERROR_REPORT
        if ((pdata[0] & 1))
        {
            memcpy(_dsi_cmd_queue, pdata, queue_size*4);
            _dsi_cmd_queue[queue_size++] = 0x4;
            pdata = (unsigned int*) _dsi_cmd_queue;
        }
        else
        {
            pdata[0] |= 4;
        }
        #endif
        for(i=0; i<queue_size; i++)
            OUTREG32(&DSI_CMDQ_REG->data[i], AS_UINT32((pdata+i)));
        
        OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, queue_size);
        
        //    for (i = 0; i < queue_size; i++)
        //        printk("[DISP] - kernel - DSI_set_cmdq. DSI_CMDQ+%04x : 0x%08x\n", i*4, INREG32(DSI_BASE + 0x180 + i*4));
        
        if(force_update)
        {
            MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagStart, *(unsigned int*)(&DSI_CMDQ_REG->data[0]), *(unsigned int*)(&DSI_CMDQ_REG->data[1]));
            DSI_Start();
            for(i=0; i<10; i++) ;
            DSI_WaitForEngineNotBusy();
            MMProfileLogEx(MTKFB_MMP_Events.DSICmd, MMProfileFlagEnd, *(unsigned int*)(&DSI_CMDQ_REG->data[2]), *(unsigned int*)(&DSI_CMDQ_REG->data[3]));
        }
    }
}


DSI_STATUS DSI_Write_T0_INS(DSI_T0_INS *t0)
{
   OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t0));	
   
   OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
   OUTREG32(&DSI_REG->DSI_START, 0);
   OUTREG32(&DSI_REG->DSI_START, 1);
   
   return DSI_STATUS_OK;	
}


DSI_STATUS DSI_Write_T1_INS(DSI_T1_INS *t1)
{
   OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t1));	
   
   OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
   OUTREG32(&DSI_REG->DSI_START, 0);
   OUTREG32(&DSI_REG->DSI_START, 1);
   
   return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T2_INS(DSI_T2_INS *t2)
{
   unsigned int i;
   
   OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t2));
   
   for(i=0;i<((t2->WC16-1)>>2)+1;i++)
      OUTREG32(&DSI_CMDQ_REG->data[1+i], AS_UINT32((t2->pdata+i)));
   
   OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, (((t2->WC16-1)>>2)+2));
   OUTREG32(&DSI_REG->DSI_START, 0);
   OUTREG32(&DSI_REG->DSI_START, 1);
   
   return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T3_INS(DSI_T3_INS *t3)
{
   OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t3));	
   
   OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
   OUTREG32(&DSI_REG->DSI_START, 0);
   OUTREG32(&DSI_REG->DSI_START, 1);
   
   return DSI_STATUS_OK;
}


DSI_STATUS DSI_TXRX_Control(bool cksm_en, 
                                                         bool ecc_en, 
                                                         unsigned char lane_num, 
                                                         unsigned char vc_num,
                                                         bool null_packet_en,
                                                         bool err_correction_en,
                                                         bool dis_eotp_en,
                                                         unsigned int  max_return_size)
{
   DSI_TXRX_CTRL_REG tmp_reg;
   
   tmp_reg=DSI_REG->DSI_TXRX_CTRL;
   
   ///TODO: parameter checking
   //    tmp_reg.CKSM_EN=cksm_en;
   //    tmp_reg.ECC_EN=ecc_en;

   switch(lane_num)
   {
      case LCM_ONE_LANE:
         tmp_reg.LANE_NUM = 1;
         break;

      case LCM_TWO_LANE:
         tmp_reg.LANE_NUM = 3;
         break;

      case LCM_THREE_LANE:
         tmp_reg.LANE_NUM = 0x7;
         break;

      case LCM_FOUR_LANE:
         tmp_reg.LANE_NUM = 0xF;
         break;
   }

   tmp_reg.VC_NUM=vc_num;
   //    tmp_reg.CORR_EN = err_correction_en;
   tmp_reg.DIS_EOT = dis_eotp_en;
   tmp_reg.NULL_EN = null_packet_en;
   tmp_reg.MAX_RTN_SIZE = max_return_size;
   tmp_reg.HSTX_CKLP_EN = 0;//need customization???
   OUTREG32(&DSI_REG->DSI_TXRX_CTRL, AS_UINT32(&tmp_reg));
   
   return DSI_STATUS_OK;
}


DSI_STATUS DSI_PS_Control(unsigned int ps_type, unsigned int vact_line, unsigned int ps_wc)
{
    DSI_PSCTRL_REG tmp_reg;
    UINT32 tmp_hstx_cklp_wc;
 
    tmp_reg=DSI_REG->DSI_PSCTRL;
    
    ///TODO: parameter checking
    ASSERT(ps_type <= PACKED_PS_18BIT_RGB666);
    if(ps_type>LOOSELY_PS_18BIT_RGB666)
        tmp_reg.DSI_PS_SEL=(5 - ps_type);
    else
        tmp_reg.DSI_PS_SEL=ps_type;
 
    tmp_reg.DSI_PS_WC=ps_wc;
    tmp_hstx_cklp_wc = ps_wc;
    
    OUTREG32(&DSI_REG->DSI_VACT_NL, AS_UINT32(&vact_line));
    OUTREG32(&DSI_REG->DSI_PSCTRL, AS_UINT32(&tmp_reg));   
    OUTREG32(&DSI_REG->DSI_HSTX_CKL_WC, tmp_hstx_cklp_wc);
 
    return DSI_STATUS_OK;
}


void DSI_write_lcm_cmd(unsigned int cmd)
{
    DSI_T0_INS *t0_tmp=0;
    DSI_CMDQ_CONFG CONFG_tmp;
    
    CONFG_tmp.type=SHORT_PACKET_RW;
    CONFG_tmp.BTA=DISABLE_BTA;
    CONFG_tmp.HS=LOW_POWER;
    CONFG_tmp.CL=CL_8BITS;
    CONFG_tmp.TE=DISABLE_TE;
    CONFG_tmp.RPT=DISABLE_RPT;
    
    t0_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
    t0_tmp->Data_ID= (cmd&0xFF);
    t0_tmp->Data0 = 0x0;
    t0_tmp->Data1 = 0x0;	
    
    DSI_Write_T0_INS(t0_tmp);
}


void DSI_write_lcm_regs(unsigned int addr, unsigned int *para, unsigned int nums)
{
    DSI_T2_INS *t2_tmp=0;
    DSI_CMDQ_CONFG CONFG_tmp;
    
    CONFG_tmp.type=LONG_PACKET_W;
    CONFG_tmp.BTA=DISABLE_BTA;
    CONFG_tmp.HS=LOW_POWER;
    CONFG_tmp.CL=CL_8BITS;
    CONFG_tmp.TE=DISABLE_TE;
    CONFG_tmp.RPT=DISABLE_RPT;
    
    t2_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
    t2_tmp->Data_ID = (addr&0xFF);
    t2_tmp->WC16 = nums;	
    t2_tmp->pdata = para;	
    
    DSI_Write_T2_INS(t2_tmp);
}


UINT32 DSI_dcs_read_lcm_reg(UINT8 cmd)
{
   UINT32 recv_data = 0;

   return recv_data;
}


/// return value: the data length we got
UINT32 DSI_dcs_read_lcm_reg_v2(UINT8 cmd, UINT8 *buffer, UINT8 buffer_size)
{
   UINT32 max_try_count = 5;
   UINT32 recv_data_cnt;
   unsigned int read_timeout_ms;
   unsigned char packet_type;
   DSI_RX_DATA_REG read_data0;
   DSI_RX_DATA_REG read_data1;
   DSI_RX_DATA_REG read_data2;
   DSI_RX_DATA_REG read_data3;
   DSI_T0_INS t0;  
   #if ENABLE_DSI_INTERRUPT
      static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
      long ret;
   #endif

   if (DSI_REG->DSI_MODE_CTRL.MODE)
      return 0;
   
   if (buffer == NULL || buffer_size == 0)
      return 0;
   
   do
   {
      if(max_try_count == 0)
         return 0;
      max_try_count--;
      recv_data_cnt = 0;
      read_timeout_ms = 20;
      
      DSI_WaitForEngineNotBusy();
      
      t0.CONFG = 0x04;        ///BTA
      t0.Data0 = cmd;

      if (buffer_size < 0x3)
         t0.Data_ID = DSI_DCS_READ_PACKET_ID;
      else
         t0.Data_ID = DSI_GERNERIC_READ_LONG_PACKET_ID;

      t0.Data1 = 0;
      
      OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
      OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
      
      ///clear read ACK 
      OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
      OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
      OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);
      OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
      OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);
      
      OUTREG32(&DSI_REG->DSI_START, 0);
      OUTREG32(&DSI_REG->DSI_START, 1);
      
      /// the following code is to
      /// 1: wait read ready
      /// 2: ack read ready
      /// 3: wait for CMDQ_DONE
      /// 3: read data
      #if ENABLE_DSI_INTERRUPT
         ret = wait_event_interruptible_timeout(_dsi_dcs_read_wait_queue, 
                                                                 !_IsEngineBusy(),
                                                                 WAIT_TIMEOUT);

         if (0 == ret) {
            xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine read ready timeout!!!\n");
            
            DSI_DumpRegisters();
            
            ///do necessary reset here
            OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
            DSI_Reset();
            
            return 0;
         }
      #else

         DSI_DRV_FUNC(" Start polling DSI read ready!!!\n");

         while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
         {
            ///keep polling
            msleep(1);
            read_timeout_ms --;
            
            if(read_timeout_ms == 0)
            {
               DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Polling DSI read ready timeout!!!\n");
               DSI_DumpRegisters();
               
               ///do necessary reset here
               OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
               DSI_Reset();
               return 0;
            }
         }
         DSI_DRV_FUNC(" End polling DSI read ready!!!\n");
         
         OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
         
         ///clear interrupt status
         OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
         ///STOP DSI
         OUTREG32(&DSI_REG->DSI_START, 0);
      #endif
   
      OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
      
      OUTREG32(&read_data0, AS_UINT32(&DSI_REG->DSI_RX_DATA0));
      OUTREG32(&read_data1, AS_UINT32(&DSI_REG->DSI_RX_DATA1));
      OUTREG32(&read_data2, AS_UINT32(&DSI_REG->DSI_RX_DATA2));
      OUTREG32(&read_data3, AS_UINT32(&DSI_REG->DSI_RX_DATA3));
      {
         //       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_STA : 0x%x \n", DSI_REG->DSI_RX_STA);
         DSI_DRV_FUNC(" DSI_CMDQ_SIZE : 0x%x \n", DSI_REG->DSI_CMDQ_SIZE.CMDQ_SIZE);
         DSI_DRV_FUNC(" DSI_CMDQ_DATA0 : 0x%x \n", DSI_CMDQ_REG->data[0].byte0);
         DSI_DRV_FUNC(" DSI_CMDQ_DATA1 : 0x%x \n", DSI_CMDQ_REG->data[0].byte1);
         DSI_DRV_FUNC(" DSI_CMDQ_DATA2 : 0x%x \n", DSI_CMDQ_REG->data[0].byte2);
         DSI_DRV_FUNC(" DSI_CMDQ_DATA3 : 0x%x \n", DSI_CMDQ_REG->data[0].byte3);

         DSI_DRV_FUNC(" DSI_RX_DATA0 : 0x%x \n", DSI_REG->DSI_RX_DATA0);
         DSI_DRV_FUNC(" DSI_RX_DATA1 : 0x%x \n", DSI_REG->DSI_RX_DATA1);
         DSI_DRV_FUNC(" DSI_RX_DATA2 : 0x%x \n", DSI_REG->DSI_RX_DATA2);
         DSI_DRV_FUNC(" DSI_RX_DATA3 : 0x%x \n", DSI_REG->DSI_RX_DATA3);
         
         DSI_DRV_FUNC("read_data0, %x,%x,%x,%x\n", read_data0.byte0, read_data0.byte1, read_data0.byte2, read_data0.byte3);
         DSI_DRV_FUNC("read_data1, %x,%x,%x,%x\n", read_data1.byte0, read_data1.byte1, read_data1.byte2, read_data1.byte3);
         DSI_DRV_FUNC("read_data2, %x,%x,%x,%x\n", read_data2.byte0, read_data2.byte1, read_data2.byte2, read_data2.byte3);
         DSI_DRV_FUNC("read_data3, %x,%x,%x,%x\n", read_data3.byte0, read_data3.byte1, read_data3.byte2, read_data3.byte3);
      }
   
      packet_type = read_data0.byte0;
      
      DSI_DRV_FUNC(" DSI read packet_type is 0x%x \n",packet_type);
      
      
      if(packet_type == 0x1A || packet_type == 0x1C)
      {
         recv_data_cnt = read_data0.byte1 + read_data0.byte2 * 16;

         if(recv_data_cnt > 10)
         {
            DSI_DRV_WRAN(" DSI read long packet data  exceeds 4 bytes \n");
            recv_data_cnt = 10;
         }
         
         if(recv_data_cnt > buffer_size)
         {
            DSI_DRV_WRAN(" DSI read long packet data  exceeds buffer size: %d\n", buffer_size);
            recv_data_cnt = buffer_size;
         }

         DSI_DRV_INFO(" DSI read long packet size: %d\n", recv_data_cnt);
         memcpy((void*)buffer, (void*)&read_data1, recv_data_cnt);
      }
      else
      {
         if(recv_data_cnt > buffer_size)
         {
            DSI_DRV_WRAN(" DSI read short packet data  exceeds buffer size: %d\n", buffer_size);
            recv_data_cnt = buffer_size;
         }
         memcpy((void*)buffer,(void*)&read_data0.byte1, 2);
      }
   }while(packet_type != 0x1C && packet_type != 0x21 && packet_type != 0x22 && packet_type != 0x1A);
   /// here: we may receive a ACK packet which packet type is 0x02 (incdicates some error happened)
   /// therefore we try re-read again until no ACK packet
   /// But: if it is a good way to keep re-trying ???

   return recv_data_cnt;
}


UINT32 DSI_read_lcm_reg()
{
   return 0;
}


DSI_STATUS DSI_write_lcm_fb(unsigned int addr, bool long_length)
{
   DSI_T1_INS *t1_tmp=0;
   DSI_CMDQ_CONFG CONFG_tmp;
   
   CONFG_tmp.type=FB_WRITE;
   CONFG_tmp.BTA=DISABLE_BTA;
   CONFG_tmp.HS=HIGH_SPEED;
   
   if(long_length)
      CONFG_tmp.CL=CL_16BITS;
   else
      CONFG_tmp.CL=CL_8BITS;		
   
   CONFG_tmp.TE=DISABLE_TE;
   CONFG_tmp.RPT=DISABLE_RPT;
   
   
   t1_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
   t1_tmp->Data_ID= 0x39;
   t1_tmp->mem_start0 = (addr&0xFF);	
   
   if(long_length)
      t1_tmp->mem_start1 = ((addr>>8)&0xFF);
   
   return DSI_Write_T1_INS(t1_tmp);	
}


DSI_STATUS DSI_read_lcm_fb(unsigned char *buffer)
{
#if 0
    unsigned int array[2];

    DSI_WaitForEngineNotBusy();

    array[0] = 0x000A3700;// read size
    DSI_set_cmdq(array, 1, 1);
    
    DSI_dcs_read_lcm_reg_v2(0x2E,buffer,10);
    DSI_dcs_read_lcm_reg_v2(0x2E,buffer+10,10);
    DSI_dcs_read_lcm_reg_v2(0x2E,buffer+10*2,10);
    DSI_dcs_read_lcm_reg_v2(0x2E,buffer+10*3,10);
    DSI_dcs_read_lcm_reg_v2(0x2E,buffer+10*4,10);
    DSI_dcs_read_lcm_reg_v2(0x2E,buffer+10*5,10);
#else
    DSI_WaitForEngineNotBusy();

    // if read_fb not impl, should return info
    if(lcm_drv->read_fb)
        lcm_drv->read_fb(buffer);
#endif

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_enable_MIPI_txio(bool en)
{
   return DSI_STATUS_OK;
}


bool Need_Wait_ULPS(void)
{
#ifndef MT65XX_NEW_DISP
   if(((INREG32(DSI_BASE + 0x14C)>> 24) & 0xFF) != 0x04) {
      DSI_DRV_FUNC("[%s]:true \n", __func__);

      return TRUE;
   } 
   else {
      DSI_DRV_FUNC("[%s]:false \n", __func__);

      return FALSE;
   }
#endif

   return FALSE;
}


DSI_STATUS Wait_ULPS_Mode(void)
{
#ifndef MT65XX_NEW_DISP
   DSI_PHY_LCCON_REG lccon_reg=DSI_REG->DSI_PHY_LCCON;
   DSI_PHY_LD0CON_REG ld0con=DSI_REG->DSI_PHY_LD0CON;
   
   lccon_reg.LC_ULPM_EN =1;
   ld0con.L0_ULPM_EN=1;
   OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&lccon_reg));
   OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&ld0con));
   
   DSI_DRV_FUNC("[%s]:enter \n", __func__);
   
   while(((INREG32(DSI_BASE + 0x14C)>> 24) & 0xFF) != 0x04)
   {
      mdelay(5);

      DSI_DRV_FUNC("DSI+%04x : 0x%08x \n", DSI_BASE, INREG32(DSI_BASE + 0x14C));
   }
   
   DSI_DRV_FUNC("[%s]:exit \n", __func__);
#endif

   return DSI_STATUS_OK;
}


DSI_STATUS Wait_WakeUp(void)
{
#ifndef MT65XX_NEW_DISP
   DSI_PHY_LCCON_REG lccon_reg=DSI_REG->DSI_PHY_LCCON;
   DSI_PHY_LD0CON_REG ld0con=DSI_REG->DSI_PHY_LD0CON;
   
   DSI_DRV_FUNC("[%s]:enter \n", __func__);
   
   lccon_reg.LC_ULPM_EN =0;
   ld0con.L0_ULPM_EN=0;
   OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&lccon_reg));
   OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&ld0con));
   
   mdelay(1);//Wait 1ms for LCM Spec
   
   lccon_reg.LC_WAKEUP_EN =1;
   ld0con.L0_WAKEUP_EN=1;
   OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&lccon_reg));
   OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&ld0con));
   
   while(((INREG32(DSI_BASE + 0x148)>> 8) & 0xFF) != 0x01)
   {
      mdelay(5);

      DSI_DRV_FUNC("[soso]DSI+%04x : 0x%08x \n", DSI_BASE, INREG32(DSI_BASE + 0x148));
   }
   
   DSI_DRV_FUNC("[%s]:exit \n", __func__);
   
   lccon_reg.LC_WAKEUP_EN =0;
   ld0con.L0_WAKEUP_EN=0;
   OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&lccon_reg));
   OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&ld0con));
#endif

   return DSI_STATUS_OK;
}


// -------------------- Retrieve Information --------------------
DSI_STATUS DSI_DumpRegisters(void)
{
    UINT32 i;
    
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "---------- Start dump DSI registers ----------\n");
   
    for (i = 0; i < sizeof(DSI_REGS); i += 16)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI+%04x : 0x%08x  0x%08x  0x%08x  0x%08x\n", i, INREG32(DSI_BASE + i), INREG32(DSI_BASE + i + 0x4), INREG32(DSI_BASE + i + 0x8), INREG32(DSI_BASE + i + 0xc));
    }

    for (i = 0; i < sizeof(DSI_CMDQ_REGS); i += 16)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_CMD+%04x : 0x%08x  0x%08x  0x%08x  0x%08x\n", i, INREG32((DSI_BASE+0x180+i)), INREG32((DSI_BASE+0x180+i+0x4)), INREG32((DSI_BASE+0x180+i+0x8)), INREG32((DSI_BASE+0x180+i+0xc)));
    }

    for (i = 0; i < sizeof(DSI_PHY_REGS); i += 16)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_PHY+%04x : 0x%08x    0x%08x  0x%08x  0x%08x\n", i, INREG32((MIPI_CONFIG_BASE+i)), INREG32((MIPI_CONFIG_BASE+i+0x4)), INREG32((MIPI_CONFIG_BASE+i+0x8)), INREG32((MIPI_CONFIG_BASE+i+0xc)));
    }
   
    return DSI_STATUS_OK;
}


static LCM_PARAMS lcm_params_for_clk_setting;


DSI_STATUS DSI_FMDesense_Query(void)
{
   return DSI_STATUS_OK;
}


DSI_STATUS DSI_FM_Desense(unsigned long freq)
{
   ///need check
   DSI_Change_CLK(freq);
   return DSI_STATUS_OK;
}


DSI_STATUS DSI_Reset_CLK(void)
{
   extern LCM_PARAMS *lcm_params;
   
   DSI_WaitForEngineNotBusy();
   DSI_PHY_TIMCONFIG(lcm_params);
   DSI_PHY_clk_setting(lcm_params);

   return DSI_STATUS_OK;
}


DSI_STATUS DSI_Get_Default_CLK(unsigned int *clk)
{
   extern LCM_PARAMS *lcm_params;
   unsigned int div2_real = lcm_params->dsi.pll_div2 ? lcm_params->dsi.pll_div2 : 0x1;
   
   *clk = 13 * (lcm_params->dsi.pll_div1 + 1) / div2_real;

   return DSI_STATUS_OK;
}


DSI_STATUS DSI_Get_Current_CLK(unsigned int *clk)
{
    MIPITX_DSI_PLL_TOP_REG mipitx_dsi_pll_top = DSI_PHY_REG->MIPITX_DSI_PLL_TOP;
    MIPITX_DSI_PLL_CON2_REG mipitx_dsi_pll_con2 = DSI_PHY_REG->MIPITX_DSI_PLL_CON2;
    MIPITX_DSI_PLL_CON0_REG mipitx_dsi_pll_con0 = DSI_PHY_REG->MIPITX_DSI_PLL_CON0;

    *clk = ((((26 * mipitx_dsi_pll_con2.RG_DSI0_MPPLL_SDM_PCW_H * 8 / 4)
                     / (mipitx_dsi_pll_top.RG_MPPLL_PRESERVE_L + 1))
                     << mipitx_dsi_pll_con0.RG_DSI0_MPPLL_TXDIV0)
                     << mipitx_dsi_pll_con0.RG_DSI0_MPPLL_TXDIV1);

   return DSI_STATUS_OK;
}


DSI_STATUS DSI_Change_CLK(unsigned int clk)
{
   extern LCM_PARAMS *lcm_params;
   
   if(clk > 1000)
      return DSI_STATUS_ERROR;

   memcpy((void *)&lcm_params_for_clk_setting, (void *)lcm_params, sizeof(LCM_PARAMS));
   
   for(lcm_params_for_clk_setting.dsi.pll_div2 = 15; lcm_params_for_clk_setting.dsi.pll_div2 > 0; lcm_params_for_clk_setting.dsi.pll_div2--)
   {
      for(lcm_params_for_clk_setting.dsi.pll_div1 = 0; lcm_params_for_clk_setting.dsi.pll_div1 < 39; lcm_params_for_clk_setting.dsi.pll_div1++)
      {
         if((13 * (lcm_params_for_clk_setting.dsi.pll_div1 + 1) / lcm_params_for_clk_setting.dsi.pll_div2) >= clk)
            goto end;
      }
   }
   
   if(lcm_params_for_clk_setting.dsi.pll_div2 == 0)
   {
      for(lcm_params_for_clk_setting.dsi.pll_div1 = 0; lcm_params_for_clk_setting.dsi.pll_div1 < 39; lcm_params_for_clk_setting.dsi.pll_div1++)
      {
         if((26 * (lcm_params_for_clk_setting.dsi.pll_div1 + 1)) >= clk)
            goto end;
      }
   }
   
end:
   DSI_WaitForEngineNotBusy();
   DSI_PHY_TIMCONFIG(&lcm_params_for_clk_setting);
   DSI_PHY_clk_setting(&lcm_params_for_clk_setting);
   
   return DSI_STATUS_OK;
}

unsigned int DSI_Check_LCM(UINT32 color)
{
    unsigned int ret = 1;
    unsigned char buffer[60];
    unsigned int i=0;

    OUTREG32(&DSI_REG->DSI_MEM_CONTI, DSI_RMEM_CONTI);
    DSI_read_lcm_fb(buffer);

    printk("write pixel=0x%x\n", color&0xFFFFFF);
    OUTREG32(&DSI_REG->DSI_MEM_CONTI, DSI_WMEM_CONTI);
    
    printk("read pixel=");
    for(i=0;i<60;i+=3)
    {
        printk("[%d] 0x%x\n", i/3, (buffer[i]<<16)|(buffer[i+1]<<8)|(buffer[i+2]));
        if(((buffer[i]<<16)|(buffer[i+1]<<8)|(buffer[i+2])) != (color&0xFFFFFF))
        {
            ret = 0;
            break;
        }
    }

    return ret;
}


unsigned int DSI_BLS_Query(void)
{
    printk("BLS: 0x%x\n", INREG32(DISP_REG_BLS_EN));
    return ((INREG32(DISP_REG_BLS_EN)&0x1) == 0x1);//if 1, BLS enable
}


void DSI_BLS_Enable(bool enable)
{
    if(enable){
        OUTREG32(DISP_REG_BLS_DEBUG, 0x3);
        OUTREG32(DISP_REG_BLS_EN, 0x00010001);
        OUTREG32(DISP_REG_BLS_DEBUG, 0x0);
    }
    else{
        OUTREG32(DISP_REG_BLS_DEBUG, 0x3);
        OUTREG32(DISP_REG_BLS_EN, 0x00010000);
        OUTREG32(DISP_REG_BLS_DEBUG, 0x0);
    }
}


DSI_STATUS DSI_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp, bool cmd_mode)
{
    unsigned int mva = 0;
    M4U_PORT_STRUCT portStruct;
    struct disp_path_config_mem_out_struct mem_out = {0};


    printk("enter DSI_Capture_FB!\n");
    
    if(bpp == 32)
        mem_out.outFormat = eARGB8888;
    else if(bpp == 16)
        mem_out.outFormat = eRGB565;
    else if(bpp == 24)
        mem_out.outFormat = eRGB888;
    else
        printk("DSI_Capture_FB, fb color format not support\n");
    
    printk("before alloc MVA: va = 0x%x, size = %d\n", pvbuf, lcm_params->height*lcm_params->width*bpp/8);
    printk("addr=0x%x, format=%d \n", mva, mem_out.outFormat);
    
    portStruct.ePortID = M4U_PORT_LCD_W;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
    portStruct.Virtuality = 1;						   
    portStruct.Security = 0;
    portStruct.domain = 0;            //domain : 0 1 2 3
    portStruct.Distance = 1;
    portStruct.Direction = 0; 	
    m4u_config_port(&portStruct);
    
    mem_out.enable = 1;
    mem_out.dstAddr = mva;
    mem_out.srcROI.x = 0;
    mem_out.srcROI.y = 0;
    mem_out.srcROI.height= lcm_params->height;
    mem_out.srcROI.width= lcm_params->width;
    
    DSI_WaitForEngineNotBusy();
    disp_path_get_mutex();
    disp_path_config_mem_out(&mem_out);
    printk("Wait DSI idle \n");
    
    if(cmd_mode)
        DSI_Start();
    
    disp_path_release_mutex();
    
    DSI_WaitForEngineNotBusy();
    //	msleep(20);
    disp_path_get_mutex();
    mem_out.enable = 0;
    disp_path_config_mem_out(&mem_out);
    
    if(cmd_mode)
        DSI_Start();
    
    disp_path_release_mutex();
    
    portStruct.ePortID = M4U_PORT_LCD_W;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
    portStruct.Virtuality = 1;						   
    portStruct.Security = 0;
    portStruct.domain = 0;			  //domain : 0 1 2 3
    portStruct.Distance = 1;
    portStruct.Direction = 0;	
    m4u_config_port(&portStruct);
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_TE_Enable(BOOL enable)
{
    dsiTeEnable = enable;

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_TE_EXT_Enable(BOOL enable)
{
    dsiTeExtEnable = enable;

    if(dsiTeExtEnable == false)
    {
        OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EN,0);
    }


    return DSI_STATUS_OK;
}


BOOL DSI_Get_EXT_TE(void)
{
    return dsiTeExtEnable;
}


BOOL DSI_Get_BTA_TE(void)
{
    return dsiTeEnable;
}


// This part is for Display Customization Tool ********************
DSI_MODE_CTRL_REG fb_config_mode_ctl, fb_config_mode_ctl_backup;

void fbconfig_set_cmd_mode(void)
{
    //backup video mode
    static const long FB_WAIT_TIMEOUT = 2 * HZ;	// 2 sec
    long ret;

    OUTREG32(&fb_config_mode_ctl_backup, AS_UINT32(&DSI_REG->DSI_MODE_CTRL));
    OUTREG32(&fb_config_mode_ctl, AS_UINT32(&DSI_REG->DSI_MODE_CTRL));
    //set to cmd mode
    fb_config_mode_ctl.MODE = 0;
    OUTREG32(&DSI_REG->DSI_MODE_CTRL, AS_UINT32(&fb_config_mode_ctl));
    //DSI_Reset();

    wait_vm_done_irq = true;
    ret = wait_event_interruptible_timeout(_dsi_wait_vm_done_queue, !_IsEngineBusy(), FB_WAIT_TIMEOUT);
    if (0 == ret) {
        xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine read ready timeout!!!\n");
        
        DSI_DumpRegisters();
        ///do necessary reset here
        DSI_Reset();
        wait_vm_done_irq = false;

        return;
    }
}


void fbconfig_set_vdo_mode(void)
{
    OUTREG32(&DSI_REG->DSI_MODE_CTRL, AS_UINT32(&fb_config_mode_ctl_backup));
}


void fbconfig_DSI_Continuous_HS(int enable)
{
    DSI_TXRX_CTRL_REG tmp_reg = DSI_REG->DSI_TXRX_CTRL;

    tmp_reg.HSTX_CKLP_EN = enable;
    OUTREG32(&DSI_REG->DSI_TXRX_CTRL, AS_UINT32(&tmp_reg));
}


unsigned int fbconfig_dsi_clk =0 ;
DSI_STATUS fbconfig_DSI_set_CLK(unsigned int clk)
{
    extern LCM_PARAMS *lcm_params;
    LCM_PARAMS  fb_lcm_params ;

    printk("sxk==>fbconfig_DSI_set_CLK:%d\n",clk);
    
    if(clk > 1000)
        return DSI_STATUS_ERROR;
    memcpy((void *)&fb_lcm_params, (void *)lcm_params, sizeof(LCM_PARAMS));	
    fb_lcm_params.dsi.PLL_CLOCK = clk;
    printk("sxk==>fbconfig_DSI_set_CLK:will wait!!\n");
    
    DSI_WaitForEngineNotBusy();//cmd mode 
    printk("sxk==>will fbconfig_DSI_set_CLK:%d\n",clk);
    
    DSI_PHY_clk_setting(&fb_lcm_params);
    fbconfig_dsi_clk = clk ;
    
    //DSI_PHY_TIMCONFIG(&lcm_params_for_clk_setting);
    DSI_DumpRegisters();

    return DSI_STATUS_OK;
}


 int fbconfig_dsi_lane_num =0 ;
void fbconfig_DSI_set_lane_num(unsigned int lane_num)
{
    DSI_TXRX_CTRL_REG tmp_reg;

    tmp_reg=DSI_REG->DSI_TXRX_CTRL;		
    switch(lane_num)
    {
        case LCM_ONE_LANE:tmp_reg.LANE_NUM = 1;break;
        case LCM_TWO_LANE:tmp_reg.LANE_NUM = 3;break;
        case LCM_THREE_LANE:tmp_reg.LANE_NUM = 0x7;break;
        case LCM_FOUR_LANE:
        default:
            ASSERT(0);
            break;
    }
    OUTREG32(&DSI_REG->DSI_TXRX_CTRL, AS_UINT32(&tmp_reg));

    //DSI Clock setting for accroding lane num ;
    if(lane_num > 0)
    {
        DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0.RG_DSI0_LNT0_RT_CODE = 0x8;
        DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0.RG_DSI0_LNT0_LDOOUT_EN = 1;
    }
    if(lane_num > 1)
    {
        DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1.RG_DSI0_LNT1_RT_CODE = 0x8;
        DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1.RG_DSI0_LNT1_LDOOUT_EN = 1;
    }
    if(lane_num > 2)
    {
        DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2.RG_DSI0_LNT2_RT_CODE = 0x8;
        DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2.RG_DSI0_LNT2_LDOOUT_EN = 1;
    }
    fbconfig_dsi_lane_num = lane_num ;
}


//"TIMCON0_REG:" "HS_PRPR" "HS_ZERO" "HS_TRAIL\n"
//    "TIMCON1_REG:" "TA_GO" "TA_SURE" "TA_GET" "DA_HS_EXIT\n"
//    "TIMCON2_REG:" "CLK_ZERO" "CLK_TRAIL" "CONT_DET\n" 
 //   "TIMCON3_REG:" "CLK_HS_PRPR" "CLK_HS_POST" "CLK_HS_EXIT\n"
static unsigned int g_New_HPW =0;
void fbconfig_DSI_set_timing(MIPI_TIMING timing)
{
    int fbconfig_dsiTmpBufBpp =0;

    if(lcm_params->dsi.data_format.format == LCM_DSI_FORMAT_RGB565)
        fbconfig_dsiTmpBufBpp = 2;
    else
        fbconfig_dsiTmpBufBpp = 3;
    
    switch(timing.type)
    {
        case HS_PRPR:
            OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_PRPR,timing.value);
            break;
        case HS_ZERO:
            OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_ZERO,timing.value);
            break;
        case HS_TRAIL:
            OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_TRAIL,timing.value);
            break;
        case TA_GO:
            OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GO,timing.value);
            break;
        case TA_SURE:
            OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_SURE,timing.value);
            break;
        case TA_GET:
            OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GET,timing.value);
            break;
        case DA_HS_EXIT:
            OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,DA_HS_EXIT,timing.value);
            break;
        case CONT_DET:
            OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CONT_DET,timing.value);
            break;
        case CLK_ZERO:
            OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_ZERO,timing.value);
            break;
        case CLK_TRAIL:
            OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_TRAIL,timing.value);
            break;
        case CLK_HS_PRPR:
            OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_PRPR,timing.value);
            break;
        case CLK_HS_POST:
            OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_POST,timing.value);
            break;
        case CLK_HS_EXIT:
            OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_EXIT,timing.value);
            break;
        case HPW:
            if (lcm_params->dsi.mode != SYNC_EVENT_VDO_MODE && lcm_params->dsi.mode != BURST_VDO_MODE)
            {
                timing.value =	(timing.value * fbconfig_dsiTmpBufBpp - 10);
            }
            g_New_HPW = timing.value ;
            OUTREG32(&DSI_REG->DSI_HSA_WC, ALIGN_TO((timing.value), 4));
            break;
        case HFP:
            timing.value =(timing.value * fbconfig_dsiTmpBufBpp - 12);		
            OUTREG32(&DSI_REG->DSI_HFP_WC, ALIGN_TO((timing.value), 4));
            break;
        case HBP:
            if(g_New_HPW ==0 )
                g_New_HPW = lcm_params->dsi.horizontal_sync_active;
            
            if (lcm_params->dsi.mode == SYNC_EVENT_VDO_MODE || lcm_params->dsi.mode == BURST_VDO_MODE ){		
                timing.value =((timing.value + g_New_HPW)* fbconfig_dsiTmpBufBpp - 10);
            }
            else{
                timing.value =(timing.value * fbconfig_dsiTmpBufBpp - 10);
            }
            OUTREG32(&DSI_REG->DSI_HBP_WC, ALIGN_TO((timing.value), 4));
            break;
        case VPW:
            OUTREG32(&DSI_REG->DSI_VACT_NL,timing.value);
            break;
        case VFP:
            OUTREG32(&DSI_REG->DSI_VFP_NL, timing.value);
            break;
        case VBP:
            OUTREG32(&DSI_REG->DSI_VBP_NL, timing.value);
            break;
        default:
            printk("fbconfig dsi set timing :no such type!!\n");
    }
    DSI_DumpRegisters();
}


int fbconfig_get_Continuous_status(void)
{
    int ret ;
    DSI_TXRX_CTRL_REG tmp_reg ;

    OUTREG32(&tmp_reg, AS_UINT32(&DSI_REG->DSI_TXRX_CTRL));
    ret =tmp_reg.HSTX_CKLP_EN ;	

    return ret ;
}

int fbconfig_get_dsi_CLK(void)
{
    if(fbconfig_dsi_clk ==0)
        return lcm_params->dsi.PLL_CLOCK ;
    else
        return fbconfig_dsi_clk ;
}


int fbconfig_get_dsi_lane_num(void)
{
    if(fbconfig_dsi_lane_num == 0)
        return lcm_params->dsi.LANE_NUM ;
    else 
        return fbconfig_dsi_lane_num;
}


int fbconfig_get_dsi_timing(MIPI_SETTING_TYPE type)
{
    switch(type)
    {
        case HS_PRPR:
            return DSI_REG->DSI_PHY_TIMECON0.HS_PRPR;
        case HS_ZERO:
            return DSI_REG->DSI_PHY_TIMECON0.HS_ZERO;			
        case HS_TRAIL:
            return DSI_REG->DSI_PHY_TIMECON0.HS_TRAIL;			
        case TA_GO:
            return DSI_REG->DSI_PHY_TIMECON1.TA_GO;
            //OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GO,timing.value);
        case TA_SURE:
            return DSI_REG->DSI_PHY_TIMECON1.TA_SURE;
            //OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_SURE,timing.value);
        case TA_GET:
            return DSI_REG->DSI_PHY_TIMECON1.TA_GET;			
        case DA_HS_EXIT:
            return DSI_REG->DSI_PHY_TIMECON1.DA_HS_EXIT;
            //OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,DA_HS_EXIT,timing.value);
        case CONT_DET:
            return 	DSI_REG->DSI_PHY_TIMECON2.CONT_DET;
            //OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CONT_DET,timing.value);
            //break;
        case CLK_ZERO:
            return 	DSI_REG->DSI_PHY_TIMECON2.CLK_ZERO;
            //OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_ZERO,timing.value);
            //break;
        case CLK_TRAIL:
            return 	DSI_REG->DSI_PHY_TIMECON2.CLK_TRAIL;
            //OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_TRAIL,timing.value);
            //break;
        case CLK_HS_PRPR:
            return 	DSI_REG->DSI_PHY_TIMECON3.CLK_HS_PRPR;
            //OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_PRPR,timing.value);
            //break;
        case CLK_HS_POST:
            return DSI_REG->DSI_PHY_TIMECON3.CLK_HS_POST;
            //OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_POST,timing.value);
            //break;
        case CLK_HS_EXIT:
            return DSI_REG->DSI_PHY_TIMECON3.CLK_HS_EXIT;
            //OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_EXIT,timing.value);
            //break;
        case HPW:
        {
            DSI_HSA_WC_REG tmp_reg ;

            OUTREG32(&tmp_reg, AS_UINT32(&DSI_REG->DSI_HSA_WC));
            return tmp_reg.HSA_WC;
            //OUTREG32(&DSI_REG->DSI_HSA_WC, ALIGN_TO((timing.value), 4));
            //break;
        }
        case HFP:
        {
            DSI_HFP_WC_REG tmp_hfp ;

            OUTREG32(&tmp_hfp, AS_UINT32(&DSI_REG->DSI_HFP_WC));
            return tmp_hfp.HFP_WC;
            //OUTREG32(&DSI_REG->DSI_HFP_WC, ALIGN_TO((timing.value), 4));
            //break;
        }
        case HBP:
        {
            DSI_HBP_WC_REG tmp_hbp;

            OUTREG32(&tmp_hbp, AS_UINT32(&DSI_REG->DSI_HBP_WC));
            return tmp_hbp.HBP_WC;
            //OUTREG32(&DSI_REG->DSI_HBP_WC, ALIGN_TO((timing.value), 4));
            //break;
        }
        case VPW:
        {
            DSI_VACT_NL_REG tmp_vpw;

            OUTREG32(&tmp_vpw, AS_UINT32(&DSI_REG->DSI_VACT_NL));
            return tmp_vpw.VACT_NL;
            //OUTREG32(&DSI_REG->DSI_VACT_NL,timing.value);			
            //break;
        }
        case VFP:
        {
            DSI_VFP_NL_REG tmp_vfp;

            OUTREG32(&tmp_vfp, AS_UINT32(&DSI_REG->DSI_VFP_NL));
            return tmp_vfp.VFP_NL;
            //OUTREG32(&DSI_REG->DSI_VFP_NL, timing.value);
            //break;
        }
        case VBP:
        {
            DSI_VBP_NL_REG tmp_vbp;

            OUTREG32(&tmp_vbp, AS_UINT32(&DSI_REG->DSI_VBP_NL));
            return tmp_vbp.VBP_NL;
            //OUTREG32(&DSI_REG->DSI_VBP_NL, timing.value);
            //break;
        }
        default:
            printk("fbconfig dsi set timing :no such type!!\n");
    }
    return 0;
}


int fbconfig_get_TE_enable(void)
{
    return dsiTeEnable ;
}

