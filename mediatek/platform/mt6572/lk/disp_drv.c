#include <assert.h>
#include <string.h>
#include <arch/ops.h>
#include <platform/mt_typedefs.h>
#include <platform/mt_gpt.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>
#include <platform/disp_drv.h>

#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>

#include "disp_drv_log.h"

#else
#include <linux/delay.h>

#include "disp_drv.h"
#include <disp_drv_platform.h>
#include "disp_drv_log.h"
#include "lcd_drv.h"
#include "lcm_drv.h"
#include "dpi_drv.h"
#include "dsi_drv.h"

#include <linux/disp_assert_layer.h>

#include <linux/interrupt.h>

#include <linux/semaphore.h>
#include <linux/module.h>
extern unsigned int lcd_fps;
#endif
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static const DISP_DRIVER *disp_drv = NULL;
const LCM_DRIVER  *lcm_drv  = NULL;
static LCM_PARAMS s_lcm_params;
LCM_PARAMS *lcm_params= &s_lcm_params;
static LCD_IF_ID ctrl_if = LCD_IF_PARALLEL_0;

static volatile int direct_link_layer = -1;
static UINT32 disp_fb_bpp = 32;     ///ARGB8888
static UINT32 disp_logo_bpp = 16;     ///RGB565
#ifdef MTK_TRIPLE_FRAMEBUFFER_SUPPORT
static UINT32 disp_fb_pages = 3;     ///TRIPLE buffer
#else
static UINT32 disp_fb_pages = 2;     ///double buffer
#endif

static BOOL is_engine_in_suspend_mode = FALSE;
static BOOL is_lcm_in_suspend_mode    = FALSE;
static UINT32 dal_layerPA;
static UINT32 dal_layerVA;

static unsigned long u4IndexOfLCMList = 0;

DEFINE_SEMAPHORE(sem_update_screen);//linux 3.0 porting
static BOOL isLCMFound 					= FALSE;
/// Some utilities
#define ALIGN_TO_POW_OF_2(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))


#if defined(DISP_DRV_DBG)
    unsigned int disp_log_on = 1;
#else
    unsigned int disp_log_on = 0;
#endif


void disp_log_enable(int enable)
{
   disp_log_on = enable;
   DISP_LOG("disp common log %s\n", enable?"enabled":"disabled");
}
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

static void lcm_set_reset_pin(UINT32 value)
{
   LCD_SetResetSignal(value);
}

static void lcm_set_chip_select(UINT32 value)
{
   LCD_SetChipSelect(value);
}

static void lcm_udelay(UINT32 us)
{
   udelay(us);
}

static void lcm_mdelay(UINT32 ms)
{
   mdelay(ms);
}

static void lcm_send_cmd(UINT32 cmd)
{
   if(lcm_params== NULL)
      return;
   
   ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
                LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);
   
   LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_LOW,
                              cmd, lcm_params->dbi.cpu_write_bits));
}

static void lcm_send_data(UINT32 data)
{
   if(lcm_params== NULL)
      return;
   
   ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
                LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);
   
   LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_HIGH,
                               data, lcm_params->dbi.cpu_write_bits));
}

static UINT32 lcm_read_data(void)
{
   UINT32 data = 0;
   
   if(lcm_params== NULL)
      return 0;
   
   ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
                LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);
   
   LCD_CHECK_RET(LCD_ReadIF(ctrl_if, LCD_IF_A0_HIGH,
                              &data, lcm_params->dbi.cpu_write_bits));
   
   return data;
}

static __inline LCD_IF_WIDTH to_lcd_if_width(LCM_DBI_DATA_WIDTH data_width)
{
   switch(data_width)
   {
      case LCM_DBI_DATA_WIDTH_8BITS  : 
         return LCD_IF_WIDTH_8_BITS;

      case LCM_DBI_DATA_WIDTH_9BITS  : 
         return LCD_IF_WIDTH_9_BITS;
         
      case LCM_DBI_DATA_WIDTH_16BITS : 
         return LCD_IF_WIDTH_16_BITS;

      case LCM_DBI_DATA_WIDTH_18BITS : 
         return LCD_IF_WIDTH_18_BITS;

      case LCM_DBI_DATA_WIDTH_24BITS : 
         return LCD_IF_WIDTH_24_BITS;

      default : 
         ASSERT(0);
   }

   return LCD_IF_WIDTH_18_BITS;
}

static void disp_drv_set_driving_current(LCM_PARAMS *lcm)
{
   LCD_Set_DrivingCurrent(lcm);
}

static void disp_drv_init_io_pad(LCM_PARAMS *lcm)
{
   LCD_Init_IO_pad(lcm);
}

static void disp_drv_init_ctrl_if(void)
{
   const LCM_DBI_PARAMS *dbi = NULL;
   
   if(lcm_params== NULL)
      return;
   
   dbi = &(lcm_params->dbi);
   switch(lcm_params->ctrl)
   {
      case LCM_CTRL_NONE :
      case LCM_CTRL_GPIO : return;
      
      case LCM_CTRL_SERIAL_DBI :
         ASSERT(dbi->port <= 1);
         LCD_CHECK_RET(LCD_Init());
         ctrl_if = LCD_IF_SERIAL_0 + dbi->port;
         
         #if (MTK_LCD_HW_SIF_VERSION == 1)
            LCD_ConfigSerialIF(ctrl_if,
                                            (LCD_IF_SERIAL_BITS)dbi->data_width,
                                            dbi->serial.clk_polarity,
                                            dbi->serial.clk_phase,
                                            dbi->serial.cs_polarity,
                                            dbi->serial.clock_base,
                                            dbi->serial.clock_div,
                                            dbi->serial.is_non_dbi_mode);
         #else    ///(MTK_LCD_HW_SIF_VERSION == 2)
            LCD_ConfigSerialIF(ctrl_if,
                                            (LCD_IF_SERIAL_BITS)dbi->data_width,
                                            dbi->serial.sif_3wire,
                                            dbi->serial.sif_sdi,
                                            dbi->serial.sif_1st_pol,
                                            dbi->serial.sif_sck_def,
                                            dbi->serial.sif_div2,
                                            dbi->serial.sif_hw_cs,
                                            dbi->serial.css,
                                            dbi->serial.csh,
                                            dbi->serial.rd_1st,
                                            dbi->serial.rd_2nd,
                                            dbi->serial.wr_1st,
                                            dbi->serial.wr_2nd);
         #endif
         break;
      
      case LCM_CTRL_PARALLEL_DBI :
         ASSERT(dbi->port <= 2);
         LCD_CHECK_RET(LCD_Init());
         ctrl_if = LCD_IF_PARALLEL_0 + dbi->port;
         
         LCD_ConfigParallelIF(ctrl_if,
                                            (LCD_IF_PARALLEL_BITS)dbi->data_width,
                                            (LCD_IF_PARALLEL_CLK_DIV)dbi->clock_freq,
                                            dbi->parallel.write_setup,
                                            dbi->parallel.write_hold,
                                            dbi->parallel.write_wait,
                                            dbi->parallel.read_setup,
                                            dbi->parallel.read_hold,
                                            dbi->parallel.read_latency,
                                            dbi->parallel.wait_period,
                                            dbi->parallel.cs_high_width);
         break;
      
      default : ASSERT(0);
   }
   
   LCD_CHECK_RET(LCD_SelectWriteIF(ctrl_if));
   
   LCD_CHECK_RET(LCD_ConfigIfFormat(dbi->data_format.color_order,
                                                                 dbi->data_format.trans_seq,
                                                                 dbi->data_format.padding,
                                                                 dbi->data_format.format,
   to_lcd_if_width(dbi->data_format.width)));
}

static const LCM_UTIL_FUNCS lcm_utils =
{
   .set_reset_pin      = lcm_set_reset_pin,
   .set_chip_select    = lcm_set_chip_select,
   .udelay             = lcm_udelay,
   .mdelay             = lcm_mdelay,
   .send_cmd           = lcm_send_cmd,
   .send_data          = lcm_send_data,
   .read_data          = lcm_read_data,
   .dsi_set_cmdq		= (void (*)(unsigned int *, unsigned int, unsigned char))DSI_set_cmdq,
   .dsi_set_cmdq_V2	= DSI_set_cmdq_V2,
   .dsi_set_cmdq_V3	= (void (*)(LCM_setting_table_V3 *, unsigned int, unsigned char))DSI_set_cmdq_V3,	
   .dsi_write_cmd		= DSI_write_lcm_cmd,
   .dsi_write_regs 	= DSI_write_lcm_regs,
   .dsi_read_reg		= DSI_read_lcm_reg,
   .dsi_dcs_read_lcm_reg       = DSI_dcs_read_lcm_reg,
   .dsi_dcs_read_lcm_reg_v2    = DSI_dcs_read_lcm_reg_v2,
   /** FIXME: GPIO mode should not be configured in lcm driver
   REMOVE ME after GPIO customization is done    
   */
   .set_gpio_out       = mt_set_gpio_out,
   .set_gpio_mode        = mt_set_gpio_mode,
   .set_gpio_dir         = mt_set_gpio_dir,
   .set_gpio_pull_enable = (int (*)(unsigned int, unsigned char))mt_set_gpio_pull_enable
};



extern LCM_DRIVER* lcm_driver_list[];
extern unsigned int lcm_count;
extern void init_dsi(BOOL isDsiPoweredOn);
const LCM_DRIVER *disp_drv_get_lcm_driver(const char *lcm_name)
{
   LCM_DRIVER *lcm = NULL;

   printk("[LCM Auto Detect], we have %d lcm drivers built in\n", lcm_count);
   printk("[LCM Auto Detect], try to find driver for [%s]\n", (lcm_name==NULL)?"unknown":lcm_name);
   
   if(lcm_count ==1)
   {
      // we need to verify whether the lcm is connected
      // even there is only one lcm type defined
      lcm = lcm_driver_list[0];
      lcm->set_util_funcs(&lcm_utils);
      lcm->get_params(&s_lcm_params);
      u4IndexOfLCMList = 0;
      
      lcm_params = &s_lcm_params;
      lcm_drv = lcm;

      /*
         disp_drv_init_ctrl_if();
         disp_drv_set_driving_current(lcm_params);
         disp_drv_init_io_pad(lcm_params);
         
         if(lcm_drv->compare_id)
         {
            if(LCM_TYPE_DSI == lcm_params->type){
            init_dsi(FALSE);
            }
            
            if(lcm_drv->compare_id() == TRUE)
            {
               printk("[LCM Specified] compare id success\n");
               isLCMFound = TRUE;
            }
            else
            {
               printk("[LCM Specified] compare id fail\n");
               printk("%s, lcm is not connected\n", __func__);
               
               if(LCM_TYPE_DSI == lcm_params->type)
                  DSI_Deinit();
            }
         }
         else
      */
      {
         isLCMFound = TRUE;
      }
      
      printk("[LCM Specified]\t[%s]\n", (lcm->name==NULL)?"unknown":lcm->name);
      
      goto done;
   }
   else
   {
      unsigned int i;
      
      for(i = 0;i < lcm_count;i++)
      {
         lcm_params = &s_lcm_params;
         lcm = lcm_driver_list[i];
         
         printk("[LCM Auto Detect] [%d] - [%s]\t", i, (lcm->name==NULL)?"unknown":lcm->name);
         
         lcm->set_util_funcs(&lcm_utils);
         memset((void*)lcm_params, 0, sizeof(LCM_PARAMS));
         lcm->get_params(lcm_params);
         
         disp_drv_init_ctrl_if();
         disp_drv_set_driving_current(lcm_params);
         disp_drv_init_io_pad(lcm_params);
         
         if(lcm_name != NULL)
         {
            if(!strcmp(lcm_name,lcm->name))
            {
               printk("\t\t[success]\n");
               isLCMFound = TRUE;
               u4IndexOfLCMList = i;
               lcm_drv = lcm;
               
               goto done;
            }
            else
            {
               printk("\t\t[fail]\n");
            }
         }
         else 
         {
            if(LCM_TYPE_DSI == lcm_params->type){
               init_dsi(FALSE);
            }
   
            if(lcm->compare_id != NULL && lcm->compare_id())
            {
               printk("\t\t[success]\n");
               isLCMFound = TRUE;
               lcm_drv = lcm;
               u4IndexOfLCMList = i;
               goto done;
            }
            else
            {
               lcm_drv = lcm;
               if(LCM_TYPE_DSI == lcm_params->type)
               {
                   DSI_Deinit();

                   // disable mipi pll
                   DSI_PHY_clk_switch(FALSE, lcm_params);
               }

               printk("\t\t[fail]\n");
            }
         }
      }
   }
   done:
      return lcm_drv;
}


static void disp_dump_lcm_parameters(LCM_PARAMS *lcm_params)
{
   char *LCM_TYPE_NAME[] = {"DBI", "DPI", "DSI"};
   char *LCM_CTRL_NAME[] = {"NONE", "SERIAL", "PARALLEL", "GPIO"};
   
   if(lcm_params == NULL)
      return;
   
   printk("[mtkfb] LCM TYPE: %s\n", LCM_TYPE_NAME[lcm_params->type]);
   printk("[mtkfb] LCM INTERFACE: %s\n", LCM_CTRL_NAME[lcm_params->ctrl]);
   printk("[mtkfb] LCM resolution: %d x %d\n", lcm_params->width, lcm_params->height);
   
   return;
}

char disp_lcm_name[256] = {0};
BOOL disp_get_lcm_name_boot(char *cmdline)
{
   BOOL ret = FALSE;
   char *p, *q;
   
   p = (char*)strstr(cmdline, "lcm=");
   if(p == NULL)
   {
      // we can't find lcm string in the command line, 
      // the uboot should be old version, or the kernel is loaded by ICE debugger
      return DISP_SelectDeviceBoot(NULL);
   }
   
   p += 4;
   if((unsigned int)(p - cmdline) > strlen(cmdline+1))
   {
      ret = FALSE;
      goto done;
   }
   
   isLCMFound = strcmp(p, "0");
   printk("[mtkfb] LCM is %sconnected\n", ((isLCMFound)?"":"not "));
   p += 2;
   q = p;
   while(*q != ' ' && *q != '\0')
      q++;
   
   memset((void*)disp_lcm_name, 0, sizeof(disp_lcm_name));
   strncpy((char*)disp_lcm_name, (const char*)p, (int)(q-p));
   
   if(DISP_SelectDeviceBoot(disp_lcm_name))
      ret = TRUE;
   
   done:
      return ret;
}

static BOOL disp_drv_init_context(void)
{
   if (disp_drv != NULL && lcm_drv != NULL){
      return TRUE;
   }
   
   if(!isLCMFound)
      DISP_DetectDevice();
   
   switch(lcm_params->type)
   {
      case LCM_TYPE_DBI : 
         disp_drv = DISP_GetDriverDBI(); 
         break;
      case LCM_TYPE_DPI : 
         disp_drv = DISP_GetDriverDPI(); 
         break;
      case LCM_TYPE_DSI : 
         disp_drv = DISP_GetDriverDSI(); 
         break;
      default : 
         ASSERT(0);
   }
   
   if (!disp_drv) 
      return FALSE;
   
   return TRUE;
}

BOOL DISP_IsLCDBusy(void)
{
   return LCD_IsBusy();
}

BOOL DISP_IsLcmFound(void)
{
   return isLCMFound;
}

BOOL DISP_IsContextInited(void)
{
   if(lcm_params && disp_drv && lcm_drv)
      return TRUE;
   else
      return FALSE;
}

BOOL DISP_SelectDeviceBoot(const char* lcm_name)
{
   LCM_DRIVER *lcm = NULL;
   unsigned int i;
   
   printk("%s\n", __func__);
   if(lcm_name == NULL)
   {
      // we can't do anything in boot stage if lcm_name is NULL
      return FALSE;
   }
   for(i = 0;i < lcm_count;i++)
   {
      lcm_params = &s_lcm_params;
      lcm = lcm_driver_list[i];
      
      printk("[LCM Auto Detect] [%d] - [%s]\t", i, (lcm->name==NULL)?"unknown":lcm->name);
      
      lcm->set_util_funcs(&lcm_utils);
      memset((void*)lcm_params, 0, sizeof(LCM_PARAMS));
      lcm->get_params(lcm_params);
      
      // if lcm type is speficied, we don't need to compare the lcm name
      // in case the uboot is old version, which don't have lcm name in command line
      if(lcm_count == 1)
      {
         lcm_drv = lcm;
         isLCMFound = TRUE;
         break;
      }
      
      if(!strcmp(lcm_name,lcm->name))
      {
         printk("\t\t[success]\n");
         lcm_drv = lcm;
         isLCMFound = TRUE;
         
         break;
      }
      else
      {
         printk("\t\t[fail]\n");
      }
   }
   
   if (NULL == lcm_drv)
   {
      printk("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
      return FALSE;
   }
   
   switch(lcm_params->type)
   {
      case LCM_TYPE_DBI : disp_drv = DISP_GetDriverDBI(); break;
      case LCM_TYPE_DPI : disp_drv = DISP_GetDriverDPI(); break;
      case LCM_TYPE_DSI : disp_drv = DISP_GetDriverDSI(); break;
      default : ASSERT(0);
   }
   
   disp_dump_lcm_parameters(lcm_params);
   return TRUE;
}

BOOL DISP_SelectDevice(const char* lcm_name)
{
#ifndef MT65XX_NEW_DISP
   LCD_STATUS ret;

   ret = LCD_Init();
   printk("ret of LCD_Init() = %d\n", ret);
#endif
   lcm_drv = disp_drv_get_lcm_driver(lcm_name);
   if (NULL == lcm_drv)
   {
      printk("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
      return FALSE;
   }
   
   disp_dump_lcm_parameters(lcm_params);
   return disp_drv_init_context();
}

BOOL DISP_DetectDevice(void)
{
#ifndef MT65XX_NEW_DISP
   LCD_STATUS ret;

   ret = LCD_Init();
   printk("ret of LCD_Init() = %d\n", ret);
#endif
   lcm_drv = disp_drv_get_lcm_driver(NULL);
   if (NULL == lcm_drv)
   {
      printk("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
      return FALSE;
   }
   
   disp_dump_lcm_parameters(lcm_params);
   
   return TRUE;
}

// ---------------------------------------------------------------------------
//  DISP Driver Implementations
// ---------------------------------------------------------------------------

static unsigned int framebuffer_addr_va;
DISP_STATUS DISP_Init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
   DISP_STATUS r = DISP_STATUS_OK;

#ifdef MT65XX_NEW_DISP	
	disp_path_ddp_clock_on();
#endif

   OUTREG32(DISP_REG_CONFIG_OVL_MOUT_EN, 0x1);  // OVL output path, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_COLOR 
   
   if (!disp_drv_init_context()) {
      return DISP_STATUS_NOT_IMPLEMENTED;
   }
   
   //	/* power on LCD before config its registers*/
   //LCD_CHECK_RET(LCD_Init());
   
   disp_drv_init_ctrl_if();
   
   // For DSI PHY current leakage SW workaround.
   ///TODO: HOW!!!
   #if !defined (MTK_HDMI_SUPPORT)
      #ifndef MT65XX_NEW_DISP
         if((lcm_params->type!=LCM_TYPE_DSI) && (lcm_params->type!=LCM_TYPE_DPI)){
            DSI_PHY_clk_switch(TRUE, lcm_params);
            DSI_PHY_clk_switch(FALSE, lcm_params);
         }
      #endif
   #endif
   
   #ifndef MT65XX_NEW_DISP	
      fbVA += DISP_GetFBRamSize();
      fbPA += DISP_GetFBRamSize();
   #endif
   #ifndef BUILD_LK
      DISP_InitVSYNC((100000000/lcd_fps) + 1);//us
   #endif
   framebuffer_addr_va = fbVA;
   r = (disp_drv->init) ?
         (disp_drv->init(fbVA, fbPA, isLcmInited)) :
         DISP_STATUS_NOT_IMPLEMENTED;
   
   {
      DAL_STATUS ret;
      
      /// DAL init here
      fbVA += disp_drv->get_working_buffer_size();
      fbPA += disp_drv->get_working_buffer_size();
      ret = DAL_Init(fbVA, fbPA);
      ASSERT(DAL_STATUS_OK == ret);
      dal_layerPA = fbPA;
      dal_layerVA = fbVA;
   }
   if(lcm_drv->check_status)
       lcm_drv->check_status();
   
   return r;
}


DISP_STATUS DISP_Deinit(void)
{
   DISP_CHECK_RET(DISP_PanelEnable(FALSE));
   DISP_CHECK_RET(DISP_PowerEnable(FALSE));
   
   return DISP_STATUS_OK;
}

// -----

DISP_STATUS DISP_PowerEnable(BOOL enable)
{
    DISP_STATUS ret = DISP_STATUS_OK;
    static BOOL s_enabled = TRUE;
    
    if (enable != s_enabled)
        s_enabled = enable;
    else
        return ret;
    
    if (down_interruptible(&sem_update_screen)) 
    {
        printk("ERROR: Can't get sem_update_screen in DISP_PowerEnable()\n");
        return DISP_STATUS_ERROR;
    }
    
    disp_drv_init_context();
    
    is_engine_in_suspend_mode = enable ? FALSE : TRUE;
    
    ret = (disp_drv->enable_power) ?
            (disp_drv->enable_power(enable)) :
            DISP_STATUS_NOT_IMPLEMENTED;
    
    if (enable) 
    {
        DAL_OnDispPowerOn();
    }
    
    up(&sem_update_screen);
    
    return ret;
}


DISP_STATUS DISP_PanelEnable(BOOL enable)
{
   #ifdef BUILD_LK
      static BOOL s_enabled = FALSE;
   #else
      static BOOL s_enabled = TRUE;
   #endif
   DISP_STATUS ret = DISP_STATUS_OK;
   
   DISP_LOG("panel is %s\n", enable?"enabled":"disabled");
   
   if (down_interruptible(&sem_update_screen)) {
      printk("ERROR: Can't get sem_update_screen in DISP_PanelEnable()\n");
      return DISP_STATUS_ERROR;
   }
   
   disp_drv_init_context();
   
   is_lcm_in_suspend_mode = enable ? FALSE : TRUE;
   
   if (!lcm_drv->suspend || !lcm_drv->resume) {
      ret = DISP_STATUS_NOT_IMPLEMENTED;
      goto End;
   }
   
   if (enable && !s_enabled) {
      s_enabled = TRUE;
      
      if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
      {
         DSI_SetMode(CMD_MODE);
      }
      
      lcm_drv->resume();
      
      if(lcm_drv->check_status)
         lcm_drv->check_status();

      if(lcm_params->type==LCM_TYPE_DSI)
      {
          DSI_LP_Reset();
      }
      if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
      {
         //DSI_clk_HS_mode(1);
         DSI_SetMode(lcm_params->dsi.mode);
         
         //DPI_CHECK_RET(DPI_EnableClk());
         //DSI_CHECK_RET(DSI_EnableClk());
         
         lcm_mdelay(200);
      }
   }
   else if (!enable && s_enabled)
   {
      LCD_CHECK_RET(LCD_WaitForNotBusy());
      if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
         DSI_CHECK_RET(DSI_WaitForNotBusy());
         s_enabled = FALSE;
      
      if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
      {		
         DPI_CHECK_RET(DPI_DisableClk());
         //lcm_mdelay(200);
         DSI_Reset();
         DSI_clk_HS_mode(0);
         DSI_SetMode(CMD_MODE);
      }
      
      lcm_drv->suspend();
   }
   
   End:
      up(&sem_update_screen);
   
   return ret;
}

DISP_STATUS DISP_LCDPowerEnable(BOOL enable)
{
   if (enable)
   {
      LCD_CHECK_RET(LCD_PowerOn());
      #if defined(MTK_M4U_SUPPORT)
         LCD_CHECK_RET(LCD_M4UPowerOn());
      #endif
   }
   else
   {
      LCD_CHECK_RET(LCD_PowerOff());
      #if defined(MTK_M4U_SUPPORT)
         LCD_CHECK_RET(LCD_M4UPowerOff());
      #endif
   }
   
   return DISP_STATUS_OK;
}

bool first_backlight = true;

DISP_STATUS DISP_SetBacklight(UINT32 level)
{
	DISP_STATUS ret = DISP_STATUS_OK;
	if(first_backlight && level == 0){
		first_backlight = false;
		printf("First set backlight = 0, return!\n");
		return ret;
	}
	if (down_interruptible(&sem_update_screen)) {
		printk("ERROR: Can't get sem_update_screen in DISP_SetBacklight()\n");
		return DISP_STATUS_ERROR;
	}

	disp_drv_init_context();

	if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
		DSI_CHECK_RET(DSI_WaitForNotBusy());

	if (!lcm_drv->set_backlight) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	printk("LK:set_backlight = %d\n",level);
	lcm_drv->set_backlight(level);
End:

	up(&sem_update_screen);

	return ret;
}

DISP_STATUS DISP_SetBacklight_mode(UINT32 mode)
{
   DISP_STATUS ret = DISP_STATUS_OK;
   
   if (down_interruptible(&sem_update_screen)) {
      printk("ERROR: Can't get sem_update_screen in DISP_SetBacklight_mode()\n");
      return DISP_STATUS_ERROR;
   }
   
   disp_drv_init_context();

   if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
      DSI_CHECK_RET(DSI_WaitForNotBusy());
   
   if (!lcm_drv->set_backlight) {
      ret = DISP_STATUS_NOT_IMPLEMENTED;
      goto End;
   }

   lcm_drv->set_backlight_mode(mode);

   End:
      up(&sem_update_screen);
   
   return ret;
   
}

DISP_STATUS DISP_SetPWM(UINT32 divider)
{
   DISP_STATUS ret = DISP_STATUS_OK;
   
   if (down_interruptible(&sem_update_screen)) {
      printk("ERROR: Can't get sem_update_screen in DISP_SetPWM()\n");
      return DISP_STATUS_ERROR;
   }
   
   disp_drv_init_context();
   

   if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
      DSI_CHECK_RET(DSI_WaitForNotBusy());
   
   if (!lcm_drv->set_pwm) {
      ret = DISP_STATUS_NOT_IMPLEMENTED;
      goto End;
   }
   
   lcm_drv->set_pwm(divider);


   End:
      up(&sem_update_screen);
   
   return ret;
}

DISP_STATUS DISP_GetPWM(UINT32 divider, unsigned int *freq)
{
   DISP_STATUS ret = DISP_STATUS_OK;
   
   disp_drv_init_context();
   
   if (!lcm_drv->get_pwm) {
      ret = DISP_STATUS_NOT_IMPLEMENTED;
      goto End;
   }
   
   *freq = lcm_drv->get_pwm(divider);
   
   End:
      return ret;
}

#ifndef BUILD_LK
#if defined(MTK_LCD_HW_3D_SUPPORT)
static BOOL is3denabled = FALSE;
static BOOL ispwmenabled = FALSE;
static BOOL ismodechanged = FALSE;

static BOOL gCurrentMode = FALSE;
static BOOL gUsingMode = FALSE;


DISP_STATUS DISP_Set3DPWM(BOOL enable, BOOL landscape)
{
   unsigned int temp_reg;
   
   if (enable && (!ispwmenabled || ismodechanged))
   {
      struct  pwm_easy_config pwm_setting;
      // Set GPIO66, GPIO67, GPIO68 to PWM2, PWM1, PWM3
      mt_set_gpio_mode(GPIO187, GPIO_MODE_GPIO);	
      mt_set_gpio_out(GPIO187, GPIO_OUT_ONE);
      
      mt_set_gpio_mode(GPIO66, GPIO_MODE_07);	
      mt_set_gpio_mode(GPIO67, GPIO_MODE_07);	
      mt_set_gpio_mode(GPIO68, GPIO_MODE_07);	
      
      pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
      pwm_setting.duty = 50;
      pwm_setting.clk_div = CLK_DIV1;
      pwm_setting.duration = 533;
      
      pwm_setting.pwm_no = PWM1;
      pwm_set_easy_config(&pwm_setting);
      pwm_setting.pwm_no = PWM2;
      pwm_set_easy_config(&pwm_setting);
      pwm_setting.pwm_no = PWM3;
      pwm_set_easy_config(&pwm_setting);
      
      temp_reg=INREG32(INFRA_SYS_CFG_BASE+0x700); 
      
      temp_reg&=~0xF000000;
      
      if(landscape)
         temp_reg|=0x9000000;
      else
         temp_reg|=0x3000000;
      
      OUTREG32((INFRA_SYS_CFG_BASE+0x700), temp_reg);
      
      ispwmenabled = TRUE;
      
      DISP_LOG("3D PWM is enabled. landscape:%d ! \n", landscape);
   }
   else if (!enable && ispwmenabled)
   {	
      mt_set_gpio_mode(GPIO187, GPIO_MODE_GPIO);	
      mt_set_gpio_out(GPIO187, GPIO_OUT_ZERO);
      
      mt_set_gpio_mode(GPIO66, GPIO_MODE_GPIO);	
      mt_set_gpio_mode(GPIO67, GPIO_MODE_GPIO);	
      mt_set_gpio_mode(GPIO68, GPIO_MODE_GPIO);	
      
      ispwmenabled = FALSE;
      
      DISP_LOG("3D PWM is disabled ! \n");
   }

   return DISP_STATUS_OK;
}


BOOL DISP_Is3DEnabled(void)
{
   is3denabled = LCD_Is3DEnabled();
   
   return is3denabled;
}

BOOL DISP_is3DLandscapeMode(void)
{
   gCurrentMode = LCD_Is3DLandscapeMode();
   
   if (gCurrentMode != gUsingMode)
      ismodechanged = TRUE;
   else
      ismodechanged = FALSE;
   
   gUsingMode = gCurrentMode;
   
   return LCD_Is3DLandscapeMode();
}
#endif
#endif

// -----

DISP_STATUS DISP_SetFrameBufferAddr(UINT32 fbPhysAddr)
{
   LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER, fbPhysAddr));
   
   return DISP_STATUS_OK;
}

// -----

static BOOL is_overlaying = FALSE;

DISP_STATUS DISP_EnterOverlayMode(void)
{
   DISP_FUNC();
   if (is_overlaying) {
      return DISP_STATUS_ALREADY_SET;
   } 
   else {
      is_overlaying = TRUE;
   }
   
   return DISP_STATUS_OK;
}


DISP_STATUS DISP_LeaveOverlayMode(void)
{
   DISP_FUNC();
   if (!is_overlaying) {
      return DISP_STATUS_ALREADY_SET;
   } 
   else {
      is_overlaying = FALSE;
   }
   
   return DISP_STATUS_OK;
}


// -----

DISP_STATUS DISP_EnableDirectLinkMode(UINT32 layer)
{
   ///since MT6573 we do not support DC mode
   return DISP_STATUS_OK;
}


DISP_STATUS DISP_DisableDirectLinkMode(UINT32 layer)
{
   ///since MT6573 we do not support DC mode
   return DISP_STATUS_OK;
}

// -----

extern int MT6516IDP_EnableDirectLink(void);

const char* mt65xx_disp_get_lcm_id(void)
{
   if(lcm_drv)
      return lcm_drv->name;
   else
      return NULL;
}

DISP_STATUS DISP_UpdateScreen(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
   arch_clean_cache_range(framebuffer_addr_va, DISP_GetFBRamSize());
   
   if (down_interruptible(&sem_update_screen)) {
      printk("ERROR: Can't get sem_update_screen in DISP_UpdateScreen()\n");
      return DISP_STATUS_ERROR;
   }
   
   if (is_lcm_in_suspend_mode || is_engine_in_suspend_mode) goto End;

   #ifndef MT65XX_NEW_DISP
      LCD_CHECK_RET(LCD_WaitForNotBusy());
      printf("%s, %d\n", __func__, __LINE__);
      
      if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
         DSI_CHECK_RET(DSI_WaitForNotBusy());
   #endif
   
   if (lcm_drv->update) {
      lcm_drv->update(x, y, width, height);
      if (lcm_params->type==LCM_TYPE_DSI)
      {
         DSI_LP_Reset();
      }
   }
   
   #ifndef MT65XX_NEW_DISP
      LCD_CHECK_RET(LCD_SetRoiWindow(x, y, width, height));
      printf("%s, %d\n", __func__, __LINE__);
      
      LCD_CHECK_RET(LCD_FBSetStartCoord(x, y));
      printf("%s, %d\n", __func__, __LINE__);
   #endif
   if (-1 != direct_link_layer) {
      //MT6516IDP_EnableDirectLink();     // FIXME
   } 
   else {
      disp_drv->update_screen();
   }
   End:
      up(&sem_update_screen);
   
   return DISP_STATUS_OK;
}

static DISP_INTERRUPT_CALLBACK_STRUCT DISP_CallbackArray[DISP_LCD_INTERRUPT_EVENTS_NUMBER + DISP_DSI_INTERRUPT_EVENTS_NUMBER + DISP_DPI_INTERRUPT_EVENTS_NUMBER];

static void _DISP_InterruptCallbackProxy(DISP_INTERRUPT_EVENTS eventID)
{
   UINT32 offset;
   
   if(eventID >= DISP_LCD_INTERRUPT_EVENTS_START && eventID <= DISP_LCD_INTERRUPT_EVENTS_END )
   {
      offset = eventID - DISP_LCD_INTERRUPT_EVENTS_START;
      if(DISP_CallbackArray[offset].pFunc)
      {
         DISP_CallbackArray[offset].pFunc(DISP_CallbackArray[offset].pParam);
      }
   }
   else if(eventID >= DISP_DSI_INTERRUPT_EVENTS_START && eventID <= DISP_DSI_INTERRUPT_EVENTS_END )
   {
      offset = eventID - DISP_DSI_INTERRUPT_EVENTS_START + DISP_LCD_INTERRUPT_EVENTS_NUMBER;
      if(DISP_CallbackArray[offset].pFunc)
      {
         DISP_CallbackArray[offset].pFunc(DISP_CallbackArray[offset].pParam);
      }
   }
   else if(eventID >= DISP_DPI_INTERRUPT_EVENTS_START && eventID <= DISP_DPI_INTERRUPT_EVENTS_END )
   {
      offset = eventID - DISP_DPI_INTERRUPT_EVENTS_START + DISP_LCD_INTERRUPT_EVENTS_NUMBER + DISP_DSI_INTERRUPT_EVENTS_NUMBER;
      if(DISP_CallbackArray[offset].pFunc)
      {
         DISP_CallbackArray[offset].pFunc(DISP_CallbackArray[offset].pParam);
      }
   }
   else
   {
      DISP_LOG("Invalid event id: %d\n", eventID);
      ASSERT(0);
   }
   
   return;
}

DISP_STATUS DISP_SetInterruptCallback(DISP_INTERRUPT_EVENTS eventID, DISP_INTERRUPT_CALLBACK_STRUCT *pCBStruct)
{
   UINT32 offset;
   ASSERT(pCBStruct != NULL);
   
   disp_drv_init_context();
   
   if(eventID >= DISP_LCD_INTERRUPT_EVENTS_START && eventID <= DISP_LCD_INTERRUPT_EVENTS_END )
   {
      ///register callback
      offset = eventID - DISP_LCD_INTERRUPT_EVENTS_START;
      DISP_CallbackArray[offset].pFunc = pCBStruct->pFunc;
      DISP_CallbackArray[offset].pParam = pCBStruct->pParam;
      
      LCD_CHECK_RET(LCD_SetInterruptCallback(_DISP_InterruptCallbackProxy));
      LCD_CHECK_RET(LCD_EnableInterrupt(eventID));
   }
   else if(eventID >= DISP_DSI_INTERRUPT_EVENTS_START && eventID <= DISP_DSI_INTERRUPT_EVENTS_END )
   {
      ///register callback
      offset = eventID - DISP_DSI_INTERRUPT_EVENTS_START + DISP_LCD_INTERRUPT_EVENTS_NUMBER;
      DISP_CallbackArray[offset].pFunc = pCBStruct->pFunc;
      DISP_CallbackArray[offset].pParam = pCBStruct->pParam;
      
      DSI_CHECK_RET(DSI_SetInterruptCallback(_DISP_InterruptCallbackProxy));
      DSI_CHECK_RET(DSI_EnableInterrupt(eventID));
   }
   else if(eventID >= DISP_DPI_INTERRUPT_EVENTS_START && eventID <= DISP_DPI_INTERRUPT_EVENTS_END )
   {
      offset = eventID - DISP_DPI_INTERRUPT_EVENTS_START + DISP_LCD_INTERRUPT_EVENTS_NUMBER + DISP_DSI_INTERRUPT_EVENTS_NUMBER;
      DISP_CallbackArray[offset].pFunc = pCBStruct->pFunc;
      DISP_CallbackArray[offset].pParam = pCBStruct->pParam;
      
      DPI_CHECK_RET(DPI_SetInterruptCallback(_DISP_InterruptCallbackProxy));
      DPI_CHECK_RET(DPI_EnableInterrupt(eventID));
   }
   else
   {
      DISP_LOG("Invalid event id: %d\n", eventID);
      ASSERT(0);
      return DISP_STATUS_ERROR;        ///TODO: error log
   }

   return DISP_STATUS_OK;
}


DISP_STATUS DISP_WaitForLCDNotBusy(void)
{
   LCD_WaitForNotBusy();
   return DISP_STATUS_OK;
}
void DISP_WaitRegUpdate(void)
{
    if((LCM_TYPE_DPI == lcm_params->type) ||
        (LCM_TYPE_DSI == lcm_params->type && lcm_params->dsi.mode != CMD_MODE)){
        DSI_RegUpdate();
    }
}

void DISP_InitVSYNC(unsigned int vsync_interval)
{
   if((LCM_TYPE_DBI == lcm_params->type) || 
      (LCM_TYPE_DSI == lcm_params->type && lcm_params->dsi.mode == CMD_MODE)){
      LCD_InitVSYNC(vsync_interval);
   }
   else if((LCM_TYPE_DPI == lcm_params->type) || 
      (LCM_TYPE_DSI == lcm_params->type && lcm_params->dsi.mode != CMD_MODE)){
      DPI_InitVSYNC(vsync_interval);
   }
   else
   {
      DISP_LOG("DISP_FMDesense_Query():unknown interface\n");
   }
}

void DISP_WaitVSYNC(void)
{
   if((LCM_TYPE_DBI == lcm_params->type) || 
       (LCM_TYPE_DSI == lcm_params->type && lcm_params->dsi.mode == CMD_MODE)){
      LCD_WaitTE();
   }
   else if((LCM_TYPE_DPI == lcm_params->type) || 
              (LCM_TYPE_DSI == lcm_params->type && lcm_params->dsi.mode != CMD_MODE)){
      DPI_WaitVSYNC();
   }
   else
   {
      DISP_LOG("DISP_WaitVSYNC():unknown interface\n");
   }
}

DISP_STATUS DISP_PauseVsync(BOOL enable)
{
   if((LCM_TYPE_DBI == lcm_params->type) || 
        (LCM_TYPE_DSI == lcm_params->type && lcm_params->dsi.mode == CMD_MODE)){
      LCD_PauseVSYNC(enable);
   }
   else if((LCM_TYPE_DPI == lcm_params->type) || 
               (LCM_TYPE_DSI == lcm_params->type && lcm_params->dsi.mode != CMD_MODE)){
      DPI_PauseVSYNC(enable);
   }
   else
   {
      DISP_LOG("DISP_PauseVSYNC():unknown interface\n");
   }
   return DISP_STATUS_OK;
}

DISP_STATUS DISP_ConfigDither(int lrs, int lgs, int lbs, int dbr, int dbg, int dbb)
{
   DISP_LOG("DISP_ConfigDither lrs:0x%x, lgs:0x%x, lbs:0x%x, dbr:0x%x, dbg:0x%x, dbb:0x%x \n", lrs, lgs, lbs, dbr, dbg, dbb);
   
   if(LCD_STATUS_OK == LCD_ConfigDither(lrs, lgs, lbs, dbr, dbg, dbb))
   {
      return DISP_STATUS_OK;
   }
   else
   {
      DISP_LOG("DISP_ConfigDither error \n");
      return DISP_STATUS_ERROR;
   }
}


// ---------------------------------------------------------------------------
//  Retrieve Information
// ---------------------------------------------------------------------------

UINT32 DISP_GetScreenWidth(void)
{
   disp_drv_init_context();
   if(lcm_params)
      return lcm_params->width;
   else
   {
      printk("WARNING!! get screen width before display driver inited!\n");
      return 0;
   }
}

UINT32 DISP_GetScreenHeight(void)
{
   disp_drv_init_context();
   if(lcm_params)
      return lcm_params->height;
   else
   {
      printk("WARNING!! get screen height before display driver inited!\n");
      return 0;
   }
}

DISP_STATUS DISP_SetScreenBpp(UINT32 bpp)
{
   ASSERT(bpp != 0);
   
   if( bpp != 16   &&          \
        bpp != 24   &&          \
        bpp != 32   &&          \
        1 )
   {
      DISP_LOG("DISP_SetScreenBpp error, not support %d bpp\n", bpp);
      return DISP_STATUS_ERROR;
   }
   
   disp_fb_bpp = bpp;
   DISP_LOG("DISP_SetScreenBpp %d bpp\n", bpp);
   
   return DISP_STATUS_OK; 
}

UINT32 DISP_GetScreenBpp(void)
{
   return disp_fb_bpp; 
}

UINT32 DISP_GetLogoBpp(void)
{
   return disp_logo_bpp; 
}

DISP_STATUS DISP_SetPages(UINT32 pages)
{
   ASSERT(pages != 0);
   
   disp_fb_pages = pages;
   DISP_LOG("DISP_SetPages %d pages\n", pages);
   
   return DISP_STATUS_OK; 
}

UINT32 DISP_GetPages(void)
{
   return disp_fb_pages;   // Double Buffers
}


BOOL DISP_IsDirectLinkMode(void)
{
   return (-1 != direct_link_layer) ? TRUE : FALSE;
}


BOOL DISP_IsInOverlayMode(void)
{
   return is_overlaying;
}

UINT32 DISP_GetFBRamSize(void)
{
    return ALIGN_TO(DISP_GetScreenWidth(), 32) * 
               ALIGN_TO(DISP_GetScreenHeight(), 32) * 
               ((DISP_GetScreenBpp() + 7) >> 3) * 
               DISP_GetPages();
}

#if defined(MTK_OVL_DECOUPLE_SUPPORT)
static unsigned int ovl_buffer_num = 3;
#define BPP				 3
UINT32 DISP_GetOVLRamSize(void)
{
	return ALIGN_TO(DISP_GetScreenWidth(), MTK_FB_ALIGNMENT) *
	           ALIGN_TO(DISP_GetScreenHeight(), MTK_FB_ALIGNMENT) * BPP * ovl_buffer_num;
}
#endif

UINT32 DISP_GetVRamSize(void)
{
   // Use a local static variable to cache the calculated vram size
   //    
   static UINT32 vramSize = 0;
   
   if (0 == vramSize)
   {
      disp_drv_init_context();
      
      ///get framebuffer size
      vramSize = DISP_GetFBRamSize();
      
      ///get DXI working buffer size
      vramSize += disp_drv->get_working_buffer_size();
      
      // get assertion layer buffer size
      vramSize += DAL_GetLayerSize();

#if defined(MTK_OVL_DECOUPLE_SUPPORT)
        // get ovl-wdma buffer size
        vramSize += DISP_GetOVLRamSize();
#endif
      
      // Align vramSize to 1MB
      //
      vramSize = ALIGN_TO_POW_OF_2(vramSize, 0x100000);
      
      DISP_LOG("DISP_GetVRamSize: %u bytes\n", vramSize);
   }
   
   return vramSize;
}

UINT32 DISP_GetVRamSizeBoot(char *cmdline)
{
   static UINT32 vramSize = 0;
   
   if(vramSize)
   {
      return vramSize;
   }
   
   disp_get_lcm_name_boot(cmdline);
   
   // if can't get the lcm type from uboot, we will return 0x800000 for a safe value
   if(disp_drv)
      vramSize = DISP_GetVRamSize();
   else
   {
      printk("%s, can't get lcm type, reserved memory size will be set as 0x800000\n", __func__);
      return 0xC00000;
   }   
   // Align vramSize to 1MB
   //
   vramSize = ALIGN_TO_POW_OF_2(vramSize, 0x100000);
   
   printk("DISP_GetVRamSizeBoot: %u bytes[%dMB]\n", vramSize, (vramSize>>20));
   
   return vramSize;
}



PANEL_COLOR_FORMAT DISP_GetPanelColorFormat(void)
{
   disp_drv_init_context();
   
   return (disp_drv->get_panel_color_format) ?
               (disp_drv->get_panel_color_format()) :
               DISP_STATUS_NOT_IMPLEMENTED;
}

UINT32 DISP_GetPanelBPP(void)
{
   PANEL_COLOR_FORMAT fmt;

   disp_drv_init_context();
   
   if(disp_drv->get_panel_color_format == NULL) 
   {
      return DISP_STATUS_NOT_IMPLEMENTED;
   }
   
   fmt = disp_drv->get_panel_color_format();
   switch(fmt)
   {
      case PANEL_COLOR_FORMAT_RGB332:
         return 8;
      case PANEL_COLOR_FORMAT_RGB444:
         return 12;
      case PANEL_COLOR_FORMAT_RGB565:
         return 16;
      case PANEL_COLOR_FORMAT_RGB666:
         return 18;
      case PANEL_COLOR_FORMAT_RGB888:
         return 24;
      default:
         return 0;
   }
}

UINT32 DISP_GetOutputBPPforDithering(void)
{
   disp_drv_init_context();
   
   return (disp_drv->get_dithering_bpp) ?
               (disp_drv->get_dithering_bpp()) :
               DISP_STATUS_NOT_IMPLEMENTED;
}

DISP_STATUS DISP_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp)
{
   DISP_FUNC();
   disp_drv_init_context();
   
   return (disp_drv->capture_framebuffer) ?
               (disp_drv->capture_framebuffer(pvbuf, bpp)) :
               DISP_STATUS_NOT_IMPLEMENTED;
}

DISP_STATUS DISP_Capture_Videobuffer(unsigned int pvbuf, unsigned int bpp, unsigned int video_rotation)
{
   DISP_FUNC();
   if (down_interruptible(&sem_update_screen)) {
      printk("ERROR: Can't get sem_update_screen in DISP_Capture_Videobuffer()\n");
      return DISP_STATUS_ERROR;
   }

   disp_drv_init_context();
   LCD_Capture_Videobuffer(pvbuf, bpp, video_rotation);
   up(&sem_update_screen);

   return DISP_STATUS_OK;
}
// xuecheng, 2010-09-19
// this api is for mATV signal interfere workaround.
// immediate update == (TE disabled + delay update in overlay mode disabled)
static BOOL is_immediateupdate = FALSE;
DISP_STATUS DISP_ConfigImmediateUpdate(BOOL enable)
{
   disp_drv_init_context();
   
   if(enable == TRUE)
   {
      LCD_TE_Enable(FALSE);
   }
   else
   {
      if(disp_drv->init_te_control)
         disp_drv->init_te_control();
      else
         return DISP_STATUS_NOT_IMPLEMENTED;
   }
   
   is_immediateupdate = enable;
   
   return DISP_STATUS_OK;
}

BOOL DISP_IsImmediateUpdate(void)
{
   return is_immediateupdate;
}

DISP_STATUS DISP_FMDesense_Query()
{
   if(LCM_TYPE_DBI == lcm_params->type){//DBI
      return (DISP_STATUS)LCD_FMDesense_Query();
   }
   else if(LCM_TYPE_DPI == lcm_params->type){//DPI
      return (DISP_STATUS)DPI_FMDesense_Query();
   }
   else if(LCM_TYPE_DSI == lcm_params->type){// DSI
      return (DISP_STATUS)DSI_FMDesense_Query();
   }
   else
   {
      DISP_LOG("DISP_FMDesense_Query():unknown interface\n");
   }
   
   return DISP_STATUS_ERROR;
}

DISP_STATUS DISP_FM_Desense(unsigned long freq)
{
   DISP_STATUS ret = DISP_STATUS_OK;
   
   if (down_interruptible(&sem_update_screen)) {
      DISP_LOG("ERROR: Can't get sem_update_screen in DISP_FM_Desense()\n");
      return DISP_STATUS_ERROR;
   }
   
   if(LCM_TYPE_DBI == lcm_params->type){//DBI
      DISP_LOG("DISP_FM_Desense():DBI interface\n");        
      LCD_CHECK_RET(LCD_FM_Desense(ctrl_if, freq));
   }
   else if(LCM_TYPE_DPI == lcm_params->type){//DPI
      DISP_LOG("DISP_FM_Desense():DPI interface\n");
      DPI_CHECK_RET(DPI_FM_Desense(freq));
   }
   else if(LCM_TYPE_DSI == lcm_params->type){// DSI
      DISP_LOG("DISP_FM_Desense():DSI interface\n");
      DSI_CHECK_RET(DSI_FM_Desense(freq));
   }
   else
   {
      DISP_LOG("DISP_FM_Desense():unknown interface\n");
      ret = DISP_STATUS_ERROR;
   }
   
   up(&sem_update_screen);
   return ret;
}

DISP_STATUS DISP_Reset_Update()
{
   DISP_STATUS ret = DISP_STATUS_OK;
   
   if (down_interruptible(&sem_update_screen)) {
      DISP_LOG("ERROR: Can't get sem_update_screen in DISP_Reset_Update()\n");
      return DISP_STATUS_ERROR;
   }
   
   if(LCM_TYPE_DBI == lcm_params->type){//DBI
      DISP_LOG("DISP_Reset_Update():DBI interface\n");        
      LCD_CHECK_RET(LCD_Reset_WriteCycle(ctrl_if));
   }
   else if(LCM_TYPE_DPI == lcm_params->type){//DPI
      DISP_LOG("DISP_Reset_Update():DPI interface\n");
      DPI_CHECK_RET(DPI_Reset_CLK());
   }
   else if(LCM_TYPE_DSI == lcm_params->type){// DSI
      DISP_LOG("DISP_Reset_Update():DSI interface\n");
      DSI_CHECK_RET(DSI_Reset_CLK());
   }
   else
   {
      DISP_LOG("DISP_Reset_Update():unknown interface\n");
      ret = DISP_STATUS_ERROR;
   }
   
   up(&sem_update_screen);
   
   return ret;
}

DISP_STATUS DISP_Get_Default_UpdateSpeed(unsigned int *speed)
{
   DISP_STATUS ret = DISP_STATUS_OK;

   if(LCM_TYPE_DBI == lcm_params->type){//DBI
      DISP_LOG("DISP_Get_Default_UpdateSpeed():DBI interface\n");        
      LCD_CHECK_RET(LCD_Get_Default_WriteCycle(ctrl_if, speed));
   }
   else if(LCM_TYPE_DPI == lcm_params->type){//DPI
      DISP_LOG("DISP_Get_Default_UpdateSpeed():DPI interface\n");
      DPI_CHECK_RET(DPI_Get_Default_CLK(speed));
   }
   else  if(LCM_TYPE_DSI == lcm_params->type){// DSI
      DISP_LOG("DISP_Get_Default_UpdateSpeed():DSI interface\n");
      DSI_CHECK_RET(DSI_Get_Default_CLK(speed));
   }
   else
   {
      DISP_LOG("DISP_Reset_Update():unknown interface\n");
      ret = DISP_STATUS_ERROR;
   }
   
   return ret;
}

DISP_STATUS DISP_Get_Current_UpdateSpeed(unsigned int *speed)
{
   DISP_STATUS ret = DISP_STATUS_OK;

   if(LCM_TYPE_DBI == lcm_params->type){//DBI
      DISP_LOG("DISP_Get_Current_UpdateSpeed():DBI interface\n");        
      LCD_CHECK_RET(LCD_Get_Current_WriteCycle(ctrl_if, speed));
   }
   else if(LCM_TYPE_DPI == lcm_params->type){//DPI
      DISP_LOG("DISP_Get_Current_UpdateSpeed():DPI interface\n");
      DPI_CHECK_RET(DPI_Get_Current_CLK(speed));
   }
   else if(LCM_TYPE_DSI == lcm_params->type){// DSI
      DISP_LOG("DISP_Get_Current_UpdateSpeed():DSI interface\n");
      DSI_CHECK_RET(DSI_Get_Current_CLK(speed));
   }
   else
   {
      DISP_LOG("DISP_Reset_Update():unknown interface\n");
      ret = DISP_STATUS_ERROR;
   }
   
   return ret;
}

DISP_STATUS DISP_Change_Update(unsigned int speed)
{
   DISP_STATUS ret = DISP_STATUS_OK;
   
   if (down_interruptible(&sem_update_screen)) {
      DISP_LOG("ERROR: Can't get sem_update_screen in DISP_Change_Update()\n");
      return DISP_STATUS_ERROR;
   }
   
   if(LCM_TYPE_DBI == lcm_params->type){//DBI
      DISP_LOG("DISP_Change_Update():DBI interface\n");        
      LCD_CHECK_RET(LCD_Change_WriteCycle(ctrl_if, speed));
   }
   else if(LCM_TYPE_DPI == lcm_params->type){//DPI
      DISP_LOG("DISP_Change_Update():DPI interface\n");
      DPI_CHECK_RET(DPI_Change_CLK(speed));
   }
   else if(LCM_TYPE_DSI == lcm_params->type){// DSI
      DISP_LOG("DISP_Change_Update():DSI interface\n");
      DSI_CHECK_RET(DSI_Change_CLK(speed));
   }
   else
   {
      DISP_LOG("DISP_Reset_Update():unknown interface\n");
      ret = DISP_STATUS_ERROR;
   }
   
   up(&sem_update_screen);
   
   return ret;
}

#if defined(MTK_M4U_SUPPORT)
DISP_STATUS DISP_InitM4U()
{
   LCD_InitM4U();
   if(LCM_TYPE_DPI == lcm_params->type){
      //	    DPI_InitM4U();     //DPI not use m4u currently
   }

   return DISP_STATUS_OK;
}

DISP_STATUS DISP_ConfigAssertLayerMva()
{
   unsigned int mva;

   ASSERT(dal_layerPA);
   LCD_CHECK_RET(LCD_AllocUIMva(dal_layerPA, &mva, DAL_GetLayerSize())); 
   DISP_LOG("DAL Layer PA = DAL_layerPA = 0x%x, MVA = 0x%x\n", dal_layerPA, mva);
   LCD_CHECK_RET(LCD_LayerSetAddress(ASSERT_LAYER, mva));

   return DISP_STATUS_OK;
}

DISP_STATUS DISP_AllocUILayerMva(unsigned int pa, unsigned int *mva, unsigned int size)
{
   LCD_CHECK_RET(LCD_AllocUIMva(pa, mva, size)); 
   DISP_LOG("UI Layer PA = 0x%x, MVA = 0x%x\n", pa, *mva);

   if(LCM_TYPE_DPI == lcm_params->type){ //dpi buffer
      unsigned int i,dpi_mva;
      UINT32 dpi_size = DISP_GetScreenWidth()*DISP_GetScreenHeight()*disp_drv->get_working_buffer_bpp();

      for(i=0;i<lcm_params->dpi.intermediat_buffer_num;i++){
         LCD_AllocUIMva(pa + size + i * dpi_size, &dpi_mva, dpi_size);
         DISP_LOG("dpi pa = 0x%x,size = %d, mva = 0x%x\n", pa + size + i * dpi_size, dpi_size, dpi_mva);
         LCD_CHECK_RET(LCD_FBSetAddress(LCD_FB_0 + i, dpi_mva));
      }
   }
   
   if((LCM_TYPE_DSI == lcm_params->type) && (lcm_params->dsi.mode != CMD_MODE)){ //dsi buffer
      unsigned int i,dsi_mva;
      UINT32 dsi_size = DISP_GetScreenWidth()*DISP_GetScreenHeight()*disp_drv->get_working_buffer_bpp();

      for(i=0;i<lcm_params->dsi.intermediat_buffer_num;i++){
         LCD_AllocUIMva(pa + size + i * dsi_size, &dsi_mva, dsi_size);
         DISP_LOG("dsi pa = 0x%x,size = %d, mva = 0x%x\n", pa + size + i * dsi_size, dsi_size, dsi_mva);
         LCD_CHECK_RET(LCD_FBSetAddress(LCD_FB_0 + i, dsi_mva));
      }
   }

   return DISP_STATUS_OK;
}

DISP_STATUS DISP_AllocOverlayMva(unsigned int va, unsigned int *mva, unsigned int size)
{
   LCD_CHECK_RET(LCD_AllocOverlayMva(va, mva, size));
   return DISP_STATUS_OK;
}

DISP_STATUS DISP_DeallocMva(unsigned int va, unsigned int mva, unsigned int size)
{
   LCD_DeallocMva(va, mva, size);
   return DISP_STATUS_OK;
}

DISP_STATUS DISP_M4U_On(BOOL enable)
{
   LCD_M4U_On(enable);
   return DISP_STATUS_OK;
}

DISP_STATUS DISP_DumpM4U(void)
{
   LCD_DumpM4U();
   return DISP_STATUS_OK;
}

#endif

DISP_STATUS DISP_ChangeLCDWriteCycle()// this function is called when bootanimation start and stop in Surfaceflinger on 6573, because 6573 LCDC DBI write speed not stable when bootup
{
    return DISP_STATUS_OK;
}

const char* DISP_GetLCMId(void)
{
   if(lcm_drv)
      return lcm_drv->name;
   else
      return NULL;
}


BOOL DISP_EsdCheck(void)
{
   BOOL result = FALSE;
   
   disp_drv_init_context();
   
   if(lcm_drv->esd_check == NULL && disp_drv->esd_check == NULL)
   {
      return FALSE;
   }
   
   if (down_interruptible(&sem_update_screen)) {
      printk("ERROR: Can't get sem_update_screen in DISP_EsdCheck()\n");
      return FALSE;
   }
   
   if(is_lcm_in_suspend_mode)
   {
      up(&sem_update_screen);
      return FALSE;
   }
   
   if(disp_drv->esd_check)
      result |= disp_drv->esd_check();
   
   LCD_CHECK_RET(LCD_WaitForNotBusy());
   if(lcm_params->type==LCM_TYPE_DSI)
      DSI_CHECK_RET(DSI_WaitForNotBusy());
   
   if(lcm_drv->esd_check)
      result |= lcm_drv->esd_check();
   
   up(&sem_update_screen);
   
   return result;
}


BOOL DISP_EsdRecoverCapbility(void)
{
   if(!disp_drv_init_context())
      return FALSE;
   
   if((lcm_drv->esd_check && lcm_drv->esd_recover) || (lcm_params->dsi.lcm_ext_te_monitor) || (lcm_params->dsi.lcm_int_te_monitor))
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

BOOL DISP_EsdRecover(void)
{
   BOOL result = FALSE;

   DISP_LOG("DISP_EsdRecover enter");
   
   if(lcm_drv->esd_recover == NULL)
   {
      return FALSE;
   }
   
   if (down_interruptible(&sem_update_screen)) {
      printk("ERROR: Can't get sem_update_screen in DISP_EsdRecover()\n");
      return FALSE;
   }
   
   if(is_lcm_in_suspend_mode)
   {
      up(&sem_update_screen);
      return FALSE;
   }
   
   LCD_CHECK_RET(LCD_WaitForNotBusy());
   if(lcm_params->type==LCM_TYPE_DSI)
   {
      DSI_CHECK_RET(DSI_WaitForNotBusy());        
   }
   
   DISP_LOG("DISP_EsdRecover do LCM recover");
   
   // do necessary configuration reset for LCM re-init
   if(disp_drv->esd_reset)
      disp_drv->esd_reset();
   
   /// LCM recover
   result = lcm_drv->esd_recover();
   
   up(&sem_update_screen);
   
   return result;
}

unsigned long DISP_GetLCMIndex(void)
{
   return u4IndexOfLCMList;
}

