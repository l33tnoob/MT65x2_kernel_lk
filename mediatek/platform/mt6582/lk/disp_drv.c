#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>
#include <platform/mt_gpt.h>
#include <platform/ddp_path.h>
#include <string.h>
#endif

extern void arch_clean_cache_range(addr_t start, size_t len);

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
static UINT32 disp_fb_pages = 3;     ///double buffer

static unsigned long u4IndexOfLCMList = 0;

DEFINE_SEMAPHORE(sem_update_screen);//linux 3.0 porting

// whether LCM Driver/Param is found
static BOOL isLCMFound 					= FALSE;
// whether lcm is connected
static BOOL isLCMConnected 				= FALSE;

/// Some utilities
#define ALIGN_TO_POW_OF_2(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))



static size_t disp_log_on = FALSE;
#define DISP_LOG(fmt, arg...) \
    do { \
        if (disp_log_on) DISP_LOG_PRINT(ANDROID_LOG_WARN, "COMMON", fmt, ##arg); \
    }while (0)

#define DISP_FUNC()	\
	do { \
		if(disp_log_on) DISP_LOG_PRINT(ANDROID_LOG_INFO, "COMMON", "[Func]%s\n", __func__); \
	}while (0)

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
	OUTREG32(DISPSYS_BASE + 0x13C, value);
}

static void lcm_udelay(UINT32 us)
{
	udelay(us);
}

static void lcm_mdelay(UINT32 ms)
{
	msleep(ms);
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
		case LCM_DBI_DATA_WIDTH_8BITS  : return LCD_IF_WIDTH_8_BITS;
		case LCM_DBI_DATA_WIDTH_9BITS  : return LCD_IF_WIDTH_9_BITS;
		case LCM_DBI_DATA_WIDTH_16BITS : return LCD_IF_WIDTH_16_BITS;
		case LCM_DBI_DATA_WIDTH_18BITS : return LCD_IF_WIDTH_18_BITS;
		case LCM_DBI_DATA_WIDTH_24BITS : return LCD_IF_WIDTH_24_BITS;
		default : ASSERT(0);
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
#if 1
	.set_gpio_out       = mt_set_gpio_out,
	.set_gpio_mode        = mt_set_gpio_mode,
	.set_gpio_dir         = mt_set_gpio_dir,
	.set_gpio_pull_enable = (int (*)(unsigned int, unsigned char))mt_set_gpio_pull_enable
#endif
};



extern LCM_DRIVER* lcm_driver_list[];
extern unsigned int lcm_count;
extern void init_dsi(BOOL isDsiPoweredOn);
const LCM_DRIVER *disp_drv_get_lcm_driver(const char *lcm_name)
{
	LCM_DRIVER *lcm = NULL;
	printk("[LCM Auto Detect], we have %d lcm drivers built in\n", lcm_count);
	printk("[LCM Auto Detect], try to find driver for [%s]\n", 
			(lcm_name==NULL)?"unknown":lcm_name);

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
					MASKREG32(DSI_BASE + 0x10, 0x2, 0x2);
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
					if(LCM_TYPE_DSI == lcm_params->type){
						DSI_Deinit();
						DSI_PHY_clk_switch(false);
					}
					printk("\t\t[fail]\n");
				}
			}
		}
	}
done:

	
	if(LCM_TYPE_DSI == lcm_params->type)
	{
		int ret = 0;
		unsigned int data_array[3];
		char buffer[4];
		init_dsi(FALSE);
		MASKREG32(DSI_BASE + 0x10, 0x2, 0x2);

	data_array[0] = 0x00043700;
	DSI_set_cmdq(data_array, 1, 1);

		ret = DSI_dcs_read_lcm_reg_v2(0x0A, &buffer,1);
		if(ret == 0)
		{
			isLCMConnected = 0;
			printk("lcm is not connected\n");
		}
		else
		{
			isLCMConnected = 1;
			printk("lcm is connected\n");
		}
		
		DSI_Deinit();
	}
	
	return lcm_drv;
}


static void disp_dump_lcm_parameters(LCM_PARAMS *lcm_params)
{
	char *LCM_TYPE_NAME[3] = {"DBI", "DPI", "DSI"};
	char *LCM_CTRL_NAME[4] = {"NONE", "SERIAL", "PARALLEL", "GPIO"};

	if(lcm_params == NULL)
		return;

	printk("[mtkfb] LCM TYPE: %s\n", LCM_TYPE_NAME[lcm_params->type]);
	printk("[mtkfb] LCM INTERFACE: %s\n", LCM_CTRL_NAME[lcm_params->ctrl]);
	printk("[mtkfb] LCM resolution: %d x %d\n", lcm_params->width, lcm_params->height);

	return;
}
static BOOL disp_drv_init_context(void)
{
	if (disp_drv != NULL && lcm_drv != NULL)
	{
		return TRUE;
	}

    	if(!isLCMFound)
	    DISP_DetectDevice();
    
	disp_drv_init_ctrl_if();

	switch(lcm_params->type)
	{
		case LCM_TYPE_DBI : disp_drv = DISP_GetDriverDBI(); break;
		case LCM_TYPE_DPI : disp_drv = DISP_GetDriverDPI(); break;
		case LCM_TYPE_DSI : disp_drv = DISP_GetDriverDSI(); break;
		default : ASSERT(0);
	}

	if (!disp_drv)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL DISP_IsLCDBusy(void)
{
	return LCD_IsBusy();
}

BOOL DISP_IsLcmFound(void)
{
	return isLCMConnected;
}

BOOL DISP_IsContextInited(void)
{
	if(lcm_params && disp_drv && lcm_drv)
		return TRUE;
	else
		return FALSE;
}
BOOL DISP_DetectDevice(void)
{
	//LCD_STATUS ret;
	DISP_LOG("shi=>%s, %d\n", __func__, __LINE__);
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
	printf("%s, %d\n", __func__, __LINE__);

	if (!disp_drv_init_context())
	{
        return DISP_STATUS_NOT_IMPLEMENTED;
    }

	DISP_LOG("%s, %d\n", __func__, __LINE__);

    //Need Me?
    //MASKREG32(DISPSYS_BASE + 0x118, 0x3, 0x3); //for adapter lcm

	disp_path_ddp_clock_on();
	DISP_LOG("%s, %d\n", __func__, __LINE__);


    disp_drv_init_ctrl_if();
	DISP_LOG("%s, %d\n", __func__, __LINE__);

	framebuffer_addr_va = fbVA;
    r = (disp_drv->init) ? (disp_drv->init(fbVA, fbPA, isLcmInited)) :
        										DISP_STATUS_NOT_IMPLEMENTED;
	DISP_LOG("%s, %d\n", __func__, __LINE__);

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

    static BOOL s_enabled = FALSE;

	if (enable != s_enabled)
	{
		s_enabled = enable;
	}
	else
	{
		return ret;
	}

	if (down_interruptible(&sem_update_screen)) {
		printk("ERROR: Can't get sem_update_screen in DISP_PowerEnable()\n");
		return DISP_STATUS_ERROR;
	}

	disp_drv_init_context();

	if(enable) //Power On
	{
		//LCD Power on
		if (lcm_drv && lcm_drv->resume_power)
		{
			lcm_drv->resume_power();
		}

		//Disp IF Power on
		ret = (disp_drv->enable_power) ? (disp_drv->enable_power(enable)) :
			                                       DISP_STATUS_NOT_IMPLEMENTED;

	}
	else //Power Off
	{
		//Disp IF Power off
		ret = (disp_drv->enable_power) ? (disp_drv->enable_power(enable)) :
			                                       DISP_STATUS_NOT_IMPLEMENTED;

		//LCD Power off
		if (lcm_drv && lcm_drv->resume_power)
		{
		   lcm_drv->suspend_power();
		}

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
		
		if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		{
			//DSI_clk_HS_mode(1);
			DSI_SetMode(lcm_params->dsi.mode);
			
			//DPI_CHECK_RET(DPI_EnableClk());
			//DSI_CHECK_RET(DSI_EnableClk());

			//msleep(200);
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
			//msleep(200);
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

DISP_STATUS DISP_SetBacklight(UINT32 level)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	if (down_interruptible(&sem_update_screen)) {
		printk("ERROR: Can't get sem_update_screen in DISP_SetBacklight()\n");
		return DISP_STATUS_ERROR;
	}

	disp_drv_init_context();

	LCD_WaitForNotBusy();
	if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
		DSI_CHECK_RET(DSI_WaitForNotBusy());

	if (!lcm_drv->set_backlight) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		DSI_SetMode(CMD_MODE);

	lcm_drv->set_backlight(level);
	if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		DSI_SetMode(lcm_params->dsi.mode);

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

	LCD_WaitForNotBusy();
	if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
		DSI_CHECK_RET(DSI_WaitForNotBusy());

	if (!lcm_drv->set_backlight) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		DSI_SetMode(CMD_MODE);

	lcm_drv->set_backlight_mode(mode);
	if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		DSI_SetMode(lcm_params->dsi.mode);

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

	LCD_WaitForNotBusy();
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
DISP_STATUS DISP_SetFrameBufferAddr(UINT32 fbPhysAddr)
{
	LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER, fbPhysAddr));

    return DISP_STATUS_OK;
}



static BOOL is_overlaying = FALSE;

DISP_STATUS DISP_EnterOverlayMode(void)
{
	DISP_FUNC();
	if (is_overlaying) {
		return DISP_STATUS_ALREADY_SET;
	} else {
		is_overlaying = TRUE;
	}

	return DISP_STATUS_OK;
}


DISP_STATUS DISP_LeaveOverlayMode(void)
{
	DISP_FUNC();
	if (!is_overlaying) {
		return DISP_STATUS_ALREADY_SET;
	} else {
		is_overlaying = FALSE;
	}

	return DISP_STATUS_OK;
}

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

	if (lcm_drv->update) {
		lcm_drv->update(x, y, width, height);
    }

	if (-1 != direct_link_layer) {
		//MT6516IDP_EnableDirectLink();     // FIXME
	} else {
		disp_drv->update_screen();
	}

	up(&sem_update_screen);

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
//EXPORT_SYMBOL(DISP_GetScreenWidth);

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
    return ALIGN_TO(DISP_GetScreenWidth(), MTK_FB_ALIGNMENT) * 
           ALIGN_TO(DISP_GetScreenHeight(), MTK_FB_ALIGNMENT) * 
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
UINT32 DISP_GetOutputBPPforDithering(void)
{
	disp_drv_init_context();

	return (disp_drv->get_dithering_bpp) ?
		(disp_drv->get_dithering_bpp()) :
		DISP_STATUS_NOT_IMPLEMENTED;
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
const char* DISP_GetLCMId(void)
{
    if(lcm_drv)
        return lcm_drv->name;
    else
        return NULL;
}
unsigned long DISP_GetLCMIndex(void)
{
    return u4IndexOfLCMList;
}

BOOL DISP_IsVideoMode(void)
{
    disp_drv_init_context();
    if(lcm_params)
        return lcm_params->type==LCM_TYPE_DPI || (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE);
    else
    {
        dprintf(INFO,"WARNING!! DISP_IsVideoMode is called before display driver inited!\n");
        return 0;
    }
}

DISP_STATUS DISP_Change_LCM_Resolution(unsigned int width, unsigned int height)
{
	if(lcm_params)
	{
		printk("LCM Resolution will be changed, original: %dx%d, now: %dx%d\n", lcm_params->width, lcm_params->height, width, height);
		// align with 4 is the minimal check, to ensure we can boot up into kernel, and could modify dfo setting again using meta tool
		// otherwise we will have a panic in lk(root cause unknown).
		if(width >lcm_params->width || height > lcm_params->height || width == 0 || height == 0 || width %4 || height %4)
		{
			printk("Invalid resolution: %dx%d\n", width, height);
			return DISP_STATUS_ERROR;
		}

		if(DISP_IsVideoMode())
		{
			printk("Warning!!!Video Mode can't support multiple resolution!\n");
			return DISP_STATUS_ERROR;
		}
			
		lcm_params->width = width;
		lcm_params->height = height;

		return DISP_STATUS_OK;
	}
	else
	{
		return DISP_STATUS_ERROR;
	}
}

typedef struct {
	unsigned int width;
	unsigned int height;
}disp_resolution_t;


static disp_resolution_t valid_resolution[] = 
{
	{540, 960},
	{240, 320},
	{480, 800},
	{480, 854},
	{720, 1280},
	{1080, 1920}
};

DISP_STATUS DISP_Check_Resolution(unsigned int width, unsigned height)
{
	int i = 0;
	for(i = 0;i < (int)(sizeof(valid_resolution)/sizeof(disp_resolution_t));i++)
	{
		if(valid_resolution[i].width == width && valid_resolution[i].height == height)
			return DISP_STATUS_OK;
	}

	return DISP_STATUS_ERROR;
}
