// ---------------------------------------------------------------------------
// include
// ---------------------------------------------------------------------------
#include <platform/disp_drv_platform.h>
#include <platform/ddp_path.h>
#include <platform/mt_gpt.h>
#include <string.h>

// ---------------------------------------------------------------------------
// struct
// ---------------------------------------------------------------------------
typedef struct
{
    UINT32 pa;
    UINT32 pitchInBytes;
} TempBuffer;


// ---------------------------------------------------------------------------
//  Global Variables
// ---------------------------------------------------------------------------
extern LCM_DRIVER *lcm_drv;
extern LCM_PARAMS *lcm_params;

static UINT32 dsiTmpBufBpp = 0;
static TempBuffer s_tmpBuffers[3];
static BOOL	dsi_vdo_streaming = FALSE;
static BOOL needStartDSI = TRUE;
BOOL DDMS_capturing=0;

// ---------------------------------------------------------------------------
//  Private Functions
// ---------------------------------------------------------------------------
void init_dsi(BOOL isDsiPoweredOn);

static void init_lcd_te_control(void)
{
    const LCM_DBI_PARAMS *dbi = &(lcm_params->dbi);

    LCD_CHECK_RET(LCD_TE_Enable(FALSE));
	if(!DISP_IsLcmFound())
        return;
#ifdef BUILD_LK
    {
        extern BOOTMODE g_boot_mode;
        printf("boot_mode = %d\n",g_boot_mode);
        if(g_boot_mode == META_BOOT)
            return;
    }
#endif
    if (LCM_DBI_TE_MODE_DISABLED == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_Enable(FALSE));
        return;
    }

    if (LCM_DBI_TE_MODE_VSYNC_ONLY == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_SetMode(LCD_TE_MODE_VSYNC_ONLY));
    } else if (LCM_DBI_TE_MODE_VSYNC_OR_HSYNC == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_SetMode(LCD_TE_MODE_VSYNC_OR_HSYNC));
        LCD_CHECK_RET(LCD_TE_ConfigVHSyncMode(dbi->te_hs_delay_cnt,
                                              dbi->te_vs_width_cnt,
                     (LCD_TE_VS_WIDTH_CNT_DIV)dbi->te_vs_width_cnt_div));
    } else ASSERT(0);

    LCD_CHECK_RET(LCD_TE_SetEdgePolarity(dbi->te_edge_polarity));
    LCD_CHECK_RET(LCD_TE_Enable(TRUE));
}

__inline DPI_FB_FORMAT get_dsi_tmp_buffer_format(void)
{
    switch(lcm_params->dsi.data_format.format)
    {
    case LCM_DSI_FORMAT_RGB565 : return 0;
    case LCM_DSI_FORMAT_RGB666 :
    case LCM_DSI_FORMAT_RGB888 : return 1;
    default : ASSERT(0);
    }
    return 1;
}


__inline UINT32 get_dsi_tmp_buffer_bpp(void)
{
    static const UINT32 TO_BPP[] = {2, 3};
    return TO_BPP[get_dsi_tmp_buffer_format()];
}


__inline LCD_FB_FORMAT get_lcd_tmp_buffer_format(void)
{
    static const UINT32 TO_LCD_FORMAT[] = {
        LCD_FB_FORMAT_RGB565,
        LCD_FB_FORMAT_RGB888
    };
    
    return TO_LCD_FORMAT[get_dsi_tmp_buffer_format()];
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

static BOOL disp_drv_dsi_init_context(void)
{
    if (NULL == lcm_drv)
    {
        return FALSE;
    }

    if (NULL == lcm_params)
    {
    	lcm_drv->get_params(lcm_params);
    }

	dsiTmpBufBpp = get_dsi_tmp_buffer_bpp();

    return TRUE;
}

static void init_intermediate_buffers(UINT32 fbPhysAddr)
{
    UINT32 tmpFbStartPA = fbPhysAddr;

    UINT32 tmpFbPitchInBytes = DISP_GetScreenWidth() * dsiTmpBufBpp;
    UINT32 tmpFbSizeInBytes  = tmpFbPitchInBytes * DISP_GetScreenHeight();

    UINT32 i;

	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "init_intermediate_buffers \n");
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "tmpFbStartPA=%x, fbPhysAddr=%x \n", tmpFbStartPA, fbPhysAddr);
    
    for (i = 0; i < lcm_params->dsi.intermediat_buffer_num; ++ i)
    {
        TempBuffer *b = &s_tmpBuffers[i];
        #ifdef BUILD_LK
        // clean the intermediate buffers as black to prevent from noise display
        memset((void*)tmpFbStartPA, 0, tmpFbSizeInBytes);
        #endif
        b->pitchInBytes = tmpFbPitchInBytes;
        b->pa = tmpFbStartPA;
        ASSERT((tmpFbStartPA & 0x7) == 0);  // check if 8-byte-aligned
        tmpFbStartPA += tmpFbSizeInBytes;
    }

}

static void init_lcd(void)
{
	UINT32 i;		

	// Config LCD Controller
	LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_ALL, FALSE));
	LCD_CHECK_RET(LCD_LayerSetTriggerMode(LCD_LAYER_ALL, LCD_SW_TRIGGER));
	LCD_CHECK_RET(LCD_EnableHwTrigger(FALSE));

	LCD_CHECK_RET(LCD_SetBackgroundColor(0));

	if(lcm_params->dsi.mode == CMD_MODE)
		LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, lcm_params->width, lcm_params->height));
	else
		LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, DISP_GetScreenWidth(), DISP_GetScreenHeight()));

	if(lcm_params->dsi.mode == CMD_MODE) {
		LCD_CHECK_RET(LCD_ConfigDSIIfFormat(lcm_params->dsi.data_format.color_order,
								lcm_params->dsi.data_format.trans_seq,
								lcm_params->dsi.data_format.padding,
								lcm_params->dsi.data_format.format,
								lcm_params->dsi.packet_size,
								TRUE));

		LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_LCM));
		LCD_CHECK_RET(LCD_WaitDPIIndication(FALSE));
	    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0, FALSE));
	    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_1, FALSE));
	    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_2, FALSE));
	} else {
		LCD_CHECK_RET(LCD_FBSetFormat(get_lcd_tmp_buffer_format()));
		LCD_CHECK_RET(LCD_FBSetPitch(s_tmpBuffers[0].pitchInBytes));
		LCD_CHECK_RET(LCD_FBSetStartCoord(0, 0));

		for (i = 0; i < lcm_params->dsi.intermediat_buffer_num; ++ i)
		{
			LCD_CHECK_RET(LCD_FBSetAddress(LCD_FB_0 + i, s_tmpBuffers[i].pa));
			LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0 + i, TRUE));
		}
	
		LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_MEM));
		LCD_CHECK_RET(LCD_WaitDPIIndication(TRUE));
	}
}


static void init_dpi(BOOL isDpiPoweredOn)
{
    const LCM_DSI_PARAMS *dsi = &(lcm_params->dsi);
    UINT32 i;

    DPI_CHECK_RET(DPI_Init(isDpiPoweredOn));

    DPI_CHECK_RET(DPI_EnableSeqOutput(FALSE));

    DPI_CHECK_RET(DPI_FBSetSize(DISP_GetScreenWidth(), DISP_GetScreenHeight()));
    
    for (i = 0; i < lcm_params->dsi.intermediat_buffer_num; ++ i)
    {
        DPI_CHECK_RET(DPI_FBSetAddress(DPI_FB_0 + i, s_tmpBuffers[i].pa));
        DPI_CHECK_RET(DPI_FBSetPitch(DPI_FB_0 + i, s_tmpBuffers[i].pitchInBytes));
        DPI_CHECK_RET(DPI_FBEnable(DPI_FB_0 + i, TRUE));
    }
    DPI_CHECK_RET(DPI_FBSetFormat(get_dsi_tmp_buffer_format()));
    DPI_CHECK_RET(DPI_FBSyncFlipWithLCD(TRUE));

    if (LCM_COLOR_ORDER_BGR == dsi->data_format.color_order) {
        DPI_CHECK_RET(DPI_SetRGBOrder(DPI_RGB_ORDER_RGB, DPI_RGB_ORDER_BGR));
    } else {
        DPI_CHECK_RET(DPI_SetRGBOrder(DPI_RGB_ORDER_RGB, DPI_RGB_ORDER_RGB));
    }

	DPI_CHECK_RET(DPI_SetDSIMode(TRUE));

}

static DISP_STATUS _dsi_init_cmd_mode(BOOL isLcmInited)
{
	//lcm power on
	if (NULL != lcm_drv->init_power && !isLcmInited) {
		lcm_drv->init_power();
	}

	//init dsi
	init_dsi(isLcmInited);

	//init lcm
	DSI_clk_HS_mode(0);
	if (NULL != lcm_drv->init && !isLcmInited)
	{
		lcm_drv->init();
	}

	//set mode
	DSI_SetMode(lcm_params->dsi.mode);

	return DISP_STATUS_OK;

}

static DISP_STATUS _dsi_init_vdo_mode(BOOL isLcmInited)
{
	//lcm power on
	if (NULL != lcm_drv->init_power && !isLcmInited) {
		lcm_drv->init_power();
	}

	//init dsi
	init_dsi(isLcmInited);

	//init lcm
	DSI_clk_HS_mode(0);
	if (NULL != lcm_drv->init && !isLcmInited)
	{
		lcm_drv->init();
	}

	//set mode
	DSI_SetMode(lcm_params->dsi.mode);

	return DISP_STATUS_OK;

}
static void _dsi_config_ddp(unsigned int srcModule,UINT32 fbPA)
{
	struct disp_path_config_struct config;
	memset(&config, 0x0, sizeof(config));

	config.srcModule = srcModule;

	if(config.srcModule == DISP_MODULE_RDMA)
	{
		config.inFormat = RDMA_INPUT_FORMAT_RGB565;
		config.addr = fbPA;
		config.pitch = ALIGN_TO(DISP_GetScreenWidth()*2, MTK_FB_ALIGNMENT);
		config.srcROI.x = 0;config.srcROI.y = 0;
		config.srcROI.height= DISP_GetScreenHeight();
		config.srcROI.width= DISP_GetScreenWidth();
	}
	else
	{
		config.bgROI.x = 0;
		config.bgROI.y = 0;
		config.bgROI.width = DISP_GetScreenWidth();
		config.bgROI.height = DISP_GetScreenHeight();
		config.bgColor = 0x0;	// background color

		config.pitch = ALIGN_TO(DISP_GetScreenWidth()*2, MTK_FB_ALIGNMENT);
		config.srcROI.x = 0;config.srcROI.y = 0;
		config.srcROI.height= DISP_GetScreenHeight();
		config.srcROI.width= DISP_GetScreenWidth();
		config.ovl_config.source = OVL_LAYER_SOURCE_MEM;

		if(lcm_params->dsi.mode != CMD_MODE){
			config.ovl_config.layer = FB_LAYER-1;
			config.ovl_config.layer_en = 1;
			config.ovl_config.fmt = OVL_INPUT_FORMAT_RGB565;
			config.ovl_config.addr = fbPA;
			config.ovl_config.source = OVL_LAYER_SOURCE_MEM;
			config.ovl_config.x = 0;	   // ROI
			config.ovl_config.y = 0;
			config.ovl_config.w = DISP_GetScreenWidth();
			config.ovl_config.h = DISP_GetScreenHeight();
			config.ovl_config.pitch = (ALIGN_TO(DISP_GetScreenWidth(),MTK_FB_ALIGNMENT))*2; //pixel number
			config.ovl_config.keyEn = 0;
			config.ovl_config.key = 0xFF;	   // color key
			config.ovl_config.aen = 0;			  // alpha enable
			config.ovl_config.alpha = 0;
		}
	}
	if(lcm_params->dsi.mode == CMD_MODE)
	{
		config.dstModule = DISP_MODULE_DSI_CMD;// DISP_MODULE_WDMA1
	}
	else
	{
		config.dstModule = DISP_MODULE_DSI_VDO;// DISP_MODULE_WDMA1
	}

	if(config.dstModule == DISP_MODULE_DSI_CMD || config.dstModule == DISP_MODULE_DSI_VDO)
	{
		config.outFormat = RDMA_OUTPUT_FORMAT_ARGB;
	}
	else
	{
		config.outFormat = WDMA_OUTPUT_FORMAT_ARGB;
	}

	disp_path_get_mutex();
	disp_path_config(&config);

	if(lcm_params->dsi.mode != CMD_MODE)
	{
		disp_path_release_mutex();
	}

	disp_bls_config();
}

static DISP_STATUS dsi_init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
	if (!disp_drv_dsi_init_context())
	{
		return DISP_STATUS_NOT_IMPLEMENTED;
	}

	if(lcm_params->dsi.mode == CMD_MODE)
	{
		_dsi_init_cmd_mode(isLcmInited);
	}
	else
	{
		_dsi_init_vdo_mode(isLcmInited);
	}

		DPI_PowerOn();
		DPI_PowerOff();

	_dsi_config_ddp(DISP_MODULE_OVL,fbPA);

	return DISP_STATUS_OK;
}


static void _dsi_enable_power_cmd_mode(BOOL enable)
{
	if (enable)
	{
		// enable MMSYS CG
		DSI_CHECK_RET(DSI_PowerOn());

		// initialize clock setting
		DSI_PHY_clk_switch(true);

		// restore dsi register
		DSI_CHECK_RET(DSI_RestoreRegisters());

		// enable sleep-out mode
		DSI_CHECK_RET(DSI_SleepOut());

		// enter wakeup
		DSI_CHECK_RET(DSI_Wakeup());

		// enable clock
		DSI_CHECK_RET(DSI_EnableClk());

		DSI_Reset();

	}
	else
	{
		// backup dsi register
		DSI_CHECK_RET(DSI_WaitForNotBusy());
		DSI_CHECK_RET(DSI_BackupRegisters());

		// enter ULPS mode
		DSI_clk_ULP_mode(1);
		DSI_lane0_ULP_mode(1);
		DSI_clk_HS_mode(0);
		// disable clock
		DSI_CHECK_RET(DSI_DisableClk());
		DSI_CHECK_RET(DSI_PowerOff());

		// disable mipi pll
		DSI_PHY_clk_switch(false);

	}

}

static void _dsi_enable_power_vdo_mode(BOOL enable)
{
	if (enable)
	{
		// enable MMSYS CG
		DSI_CHECK_RET(DSI_PowerOn());

		// initialize clock setting
		DSI_PHY_clk_switch(true);

		// restore dsi register
		DSI_CHECK_RET(DSI_RestoreRegisters());

		// enable sleep-out mode
		DSI_CHECK_RET(DSI_SleepOut());

		// enter wakeup
		DSI_CHECK_RET(DSI_Wakeup());
		DSI_clk_HS_mode(0);
		// enable clock
		DSI_CHECK_RET(DSI_EnableClk());

		DSI_Reset();

		needStartDSI = true;

	}
	else
	{
		// backup dsi register
		DSI_CHECK_RET(DSI_WaitForNotBusy());
		DSI_CHECK_RET(DSI_BackupRegisters());

		// enter ULPS mode
		DSI_clk_ULP_mode(1);
		DSI_lane0_ULP_mode(1);

		// disable clock
		DSI_CHECK_RET(DSI_DisableClk());
		DSI_CHECK_RET(DSI_PowerOff());

		// disable mipi pll
		DSI_PHY_clk_switch(false);

	}

}

// protected by sem_early_suspend, sem_update_screen
static DISP_STATUS dsi_enable_power(BOOL enable)
{
	disp_drv_dsi_init_context();

	if(lcm_params->dsi.mode == CMD_MODE)
	{
		_dsi_enable_power_cmd_mode(enable);
	}
	else
	{
		_dsi_enable_power_vdo_mode(enable);
	}

    return DISP_STATUS_OK;
}


// protected by sem_flipping, sem_early_suspend, sem_overlay_buffer, sem_update_screen
static DISP_STATUS dsi_update_screen(void)
{

	DSI_SetMode(lcm_params->dsi.mode);

	DSI_StartTransfer(needStartDSI);

	if (lcm_params->dsi.mode != CMD_MODE && needStartDSI)
	{
		needStartDSI = FALSE;
	}

	return DISP_STATUS_OK;
}


static UINT32 dsi_get_working_buffer_size(void)
{
    disp_drv_dsi_init_context();

	if(lcm_params->dsi.mode != CMD_MODE) {
        
            return 
            DISP_GetScreenWidth() *
            DISP_GetScreenHeight() *
            dsiTmpBufBpp *
            lcm_params->dsi.intermediat_buffer_num;
	}
    
    return 0;
}

static UINT32 dsi_get_working_buffer_bpp(void)
{
    disp_drv_dsi_init_context();

	if(lcm_params->dsi.mode != CMD_MODE) {
            return dsiTmpBufBpp;
	}
    
    return 0;
}

static PANEL_COLOR_FORMAT dsi_get_panel_color_format(void)
{
    disp_drv_dsi_init_context();

	{

	    switch(lcm_params->dsi.data_format.format)
	    {
		    case LCM_DSI_FORMAT_RGB565 : return PANEL_COLOR_FORMAT_RGB565;
		    case LCM_DSI_FORMAT_RGB666 : return PANEL_COLOR_FORMAT_RGB666;
		    case LCM_DSI_FORMAT_RGB888 : return PANEL_COLOR_FORMAT_RGB888;
		    default : ASSERT(0);
	    }
		
	}
}

static UINT32 dsi_get_dithering_bpp(void)
{
	return PANEL_COLOR_FORMAT_TO_BPP(dsi_get_panel_color_format());
}

DISP_STATUS dsi_capture_framebuffer(UINT32 pvbuf, UINT32 bpp)
{
	DSI_CHECK_RET(DSI_WaitForNotBusy());

	DDMS_capturing=1;

	if(lcm_params->dsi.mode == CMD_MODE)
	{
        LCD_CHECK_RET(LCD_EnableDCtoDsi(FALSE));

	    LCD_CHECK_RET(LCD_Capture_Framebuffer(pvbuf, bpp));
	}
	else
	{
	    DPI_CHECK_RET(DPI_Capture_Framebuffer(pvbuf, bpp));
	}


	if(lcm_params->dsi.mode == CMD_MODE)
	{
        LCD_CHECK_RET(LCD_EnableDCtoDsi(TRUE));
	}

	DDMS_capturing=0;

	return DISP_STATUS_OK;	
}


// called by "esd_recovery_kthread"
// protected by sem_early_suspend, sem_update_screen
BOOL dsi_esd_check(void)
{
	BOOL result = FALSE;

	if(lcm_params->dsi.mode == CMD_MODE || !dsi_vdo_streaming)
		return FALSE;
	else
	{
#ifndef BUILD_LK
		if(lcm_params->dsi.lcm_int_te_monitor)
			result = DSI_esd_check();

		if(result)
			return TRUE;

		if(lcm_params->dsi.lcm_ext_te_monitor)
			result = LCD_esd_check();

		return result;
#endif	
		return result;
	}

}


// called by "esd_recovery_kthread"
// protected by sem_early_suspend, sem_update_screen
void dsi_esd_reset(void)
{
     /// we assume the power is on here
    ///  what we need is some setting for LCM init
    if(lcm_params->dsi.mode == CMD_MODE) {
        DSI_clk_HS_mode(0);
        DSI_clk_ULP_mode(0);            
        DSI_lane0_ULP_mode(0);  
    }
	else {

		DSI_SetMode(CMD_MODE);
        DSI_clk_HS_mode(0);
		// clock/data lane go to Ideal
		DSI_Reset();
		DPI_CHECK_RET(DPI_DisableClk());

	}
	
}

void init_dsi(BOOL isDsiPoweredOn)
{
	BOOL hstx_cklp_en = FALSE;
	//Initialize flow: MIPI clock -> DSI register config -> DSI start

	DSI_CHECK_RET(DSI_PowerOn());

	//MIPI clock
	DSI_PHY_clk_switch(true);

	//DSI register config
    DSI_CHECK_RET(DSI_Init(isDsiPoweredOn));

	//TE Setting => TXRX init
	DSI_CHECK_RET(DSI_TE_Setting());

	if(0 < lcm_params->dsi.compatibility_for_nvk)
	{
		hstx_cklp_en = (BOOL)(1 - lcm_params->dsi.cont_clock);
  	}
	DSI_CHECK_RET(DSI_TXRX_Control(TRUE,                    //cksm_en
                                   TRUE,                    //ecc_en
                                   lcm_params->dsi.LANE_NUM, //ecc_en
                                   0,                       //vc_num
                                   FALSE,                   //null_packet_en
                                   FALSE,                   //err_correction_en
                                   FALSE,                   //dis_eotp_en
							       hstx_cklp_en,
                                   0));                     //max_return_size


	DSI_PHY_TIMCONFIG(lcm_params);

	//PS Control config
	DSI_CHECK_RET(DSI_PS_Control(lcm_params->dsi.PS, lcm_params->height, lcm_params->width * dsiTmpBufBpp));

	//DSI MEM CONTI config
	DSI_MEM_CONTI_Setting(DSI_WMEM_CONTI);

	//video mode special setting
	if(lcm_params->dsi.mode != CMD_MODE)
	{
		DSI_Set_VM_CMD(lcm_params);
		DSI_Config_VDO_Timing(lcm_params);
    }

	//DSI enable
	DSI_EnableClk();

}

const DISP_DRIVER *DISP_GetDriverDSI()
{
    static const DISP_DRIVER DSI_DISP_DRV =
    {
        .init                   = dsi_init,
        .enable_power           = dsi_enable_power,
        .update_screen          = dsi_update_screen,       
        .get_working_buffer_size = dsi_get_working_buffer_size,

        .get_panel_color_format = dsi_get_panel_color_format,
        .get_working_buffer_bpp = dsi_get_working_buffer_bpp,
        .init_te_control        = init_lcd_te_control,
        .get_dithering_bpp	= dsi_get_dithering_bpp,
        .capture_framebuffer	= dsi_capture_framebuffer,
        .esd_reset              = dsi_esd_reset,
        .esd_check				= dsi_esd_check,
    };

    return &DSI_DISP_DRV;
}

