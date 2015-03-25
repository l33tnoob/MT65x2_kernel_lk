#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <mach/m4u.h>
#include "disp_drv_log.h"

#include "disp_drv_platform.h"
#include "lcd_drv.h"
#include "dsi_drv.h"

#include "lcm_drv.h"
#include "disp_hal.h"


#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

// ---------------------------------------------------------------------------
//  Private Variables
// ---------------------------------------------------------------------------
static UINT32 dsiTmpBufBpp = 0;
static BOOL DDMS_capturing = FALSE;

extern LCM_DRIVER *lcm_drv;
extern LCM_PARAMS *lcm_params;
extern unsigned int is_video_mode_running;
extern BOOL DISP_IsDecoupleMode(void);

typedef struct
{
    UINT32 pa;
    UINT32 pitchInBytes;
} TempBuffer;

#ifndef MT65XX_NEW_DISP
static TempBuffer s_tmpBuffers[3];
#endif
static bool needStartDSI = true;

// ---------------------------------------------------------------------------
//  Private Functions
// ---------------------------------------------------------------------------

static void init_lcd_te_control(void)
{
    const LCM_DBI_PARAMS *dbi = &(lcm_params->dbi);

    LCD_CHECK_RET(LCD_TE_Enable(FALSE));

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

__inline UINT32 get_dsi_tmp_buffer_format(void)
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


static BOOL disp_drv_dsi_init_context(void)
{
    if (lcm_drv != NULL && lcm_params != NULL){
        dsiTmpBufBpp=get_dsi_tmp_buffer_bpp();
        return TRUE;
    }

    if (NULL == lcm_drv) {
        return FALSE;
    }

    lcm_drv->get_params(lcm_params);

    dsiTmpBufBpp=get_dsi_tmp_buffer_bpp();
    
    return TRUE;
}


void init_dsi(BOOL isDsiPoweredOn)
{
    DSI_PHY_clk_setting(lcm_params);

    // DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "%s, line:%d\n", __func__, __LINE__);
    DSI_CHECK_RET(DSI_Init(isDsiPoweredOn));

    //if(1 == lcm_params->dsi.compatibility_for_nvk){
    if(0){
        DSI_CHECK_RET(DSI_TXRX_Control(TRUE,                    //cksm_en
                                   TRUE,                    //ecc_en
                                   lcm_params->dsi.LANE_NUM, //ecc_en
                                   0,                       //vc_num
                                   FALSE,                   //null_packet_en
                                   FALSE,                   //err_correction_en
                                   FALSE,                   //dis_eotp_en
                                   0));                     //max_return_size
        DSI_set_noncont_clk(false,0);
    }
    else
    {
        DSI_CHECK_RET(DSI_TXRX_Control(TRUE,                    //cksm_en
                                       TRUE,                    //ecc_en
                                       lcm_params->dsi.LANE_NUM, //ecc_en
                                       0,                       //vc_num
                                       FALSE,                   //null_packet_en
                                       FALSE,                   //err_correction_en
                                       FALSE,                   //dis_eotp_en
                                       0));                     //max_return_size
    }

    
    //initialize DSI_PHY
    DSI_PHY_clk_switch(TRUE, lcm_params);
    DSI_PHY_TIMCONFIG(lcm_params);

    DSI_CHECK_RET(DSI_PS_Control(lcm_params->dsi.PS, lcm_params->height, lcm_params->width * dsiTmpBufBpp));

    if(lcm_params->dsi.mode != CMD_MODE)
    {
        DSI_Config_VDO_Timing(lcm_params);
        DSI_Set_VM_CMD(lcm_params);
        //if(1 == lcm_params->dsi.compatibility_for_nvk)
        if(0)
            DSI_Config_VDO_FRM_Mode();
    }
    
    DSI_CHECK_RET(DSI_enable_MIPI_txio(TRUE));
}

// ---------------------------------------------------------------------------
//  DBI Display Driver Public Functions
// ---------------------------------------------------------------------------
static DISP_STATUS dsi_config_ddp(UINT32 fbPA)
{
    struct disp_path_config_struct config = {0};

    if (DISP_IsDecoupleMode()) 
    {
        config.srcModule = DISP_MODULE_RDMA0;
    } 
    else 
    {
        config.srcModule = DISP_MODULE_OVL;
    }
    
    config.bgROI.x = 0;
    config.bgROI.y = 0;
    config.bgROI.width = lcm_params->width;
    config.bgROI.height = lcm_params->height;
    config.bgColor = 0x0;	// background color

    config.pitch = ALIGN_TO(lcm_params->width, MTK_FB_ALIGNMENT)*2;
    config.srcROI.x = 0;config.srcROI.y = 0;
    config.srcROI.height= lcm_params->height;
    config.srcROI.width= lcm_params->width;
    config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
    
    if(lcm_params->dsi.mode != CMD_MODE)
    {
        config.ovl_config.layer = DDP_OVL_LAYER_MUN-1;
        config.ovl_config.layer_en = 0;
        //disp_path_get_mutex();
        disp_path_config_layer(&config.ovl_config);
        //disp_path_release_mutex();
        //disp_path_wait_reg_update();
    }

    #if 1
    // Disable LK UI layer (Layer2)
    if(lcm_params->dsi.mode != CMD_MODE)
    {
        config.ovl_config.layer = DDP_OVL_LAYER_MUN-1-1;
        config.ovl_config.layer_en = 0;
        //disp_path_get_mutex();
        disp_path_config_layer(&config.ovl_config);
        //disp_path_release_mutex();
        //disp_path_wait_reg_update();
    }
    #endif

    config.ovl_config.layer = DDP_OVL_LAYER_MUN-1;
    config.ovl_config.layer_en = 1; 
    config.ovl_config.fmt = eRGB565;
    config.ovl_config.addr = fbPA;	
    config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
    config.ovl_config.src_x = 0;
    config.ovl_config.src_y = 0;
    config.ovl_config.dst_x = 0;	   // ROI
    config.ovl_config.dst_y = 0;
    config.ovl_config.dst_w = lcm_params->width;
    config.ovl_config.dst_h = lcm_params->height;
    config.ovl_config.src_pitch = ALIGN_TO(lcm_params->width, MTK_FB_ALIGNMENT)*2; //pixel number
    config.ovl_config.keyEn = 0;
    config.ovl_config.key = 0xFF;	   // color key
    config.ovl_config.aen = 0;			  // alpha enable
    config.ovl_config.alpha = 0;	
    
    LCD_LayerSetAddress(DDP_OVL_LAYER_MUN-1, fbPA);
    LCD_LayerSetFormat(DDP_OVL_LAYER_MUN-1, LCD_LAYER_FORMAT_RGB565);
    LCD_LayerSetOffset(DDP_OVL_LAYER_MUN-1, 0, 0);
    LCD_LayerSetSize(DDP_OVL_LAYER_MUN-1,lcm_params->width,lcm_params->height);
    LCD_LayerSetPitch(DDP_OVL_LAYER_MUN-1, ALIGN_TO(lcm_params->width, MTK_FB_ALIGNMENT) * 2);
    LCD_LayerEnable(DDP_OVL_LAYER_MUN-1, TRUE); 
    
    if(lcm_params->dsi.mode == CMD_MODE)
    {
        config.dstModule = DISP_MODULE_DSI_CMD;// DISP_MODULE_WDMA1
    }
    else
    {
        config.dstModule = DISP_MODULE_DSI_VDO;// DISP_MODULE_WDMA1
    }
    config.outFormat = RDMA_OUTPUT_FORMAT_ARGB; 
    disp_path_config(&config);
    
    if(lcm_params->dsi.mode != CMD_MODE)
    {
        //DSI_Wait_VDO_Idle();
        disp_path_get_mutex();
    }
    
    // Config FB_Layer port to be physical.
    {
        M4U_PORT_STRUCT portStruct;
        
        portStruct.ePortID = M4U_PORT_LCD_OVL;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
        portStruct.Virtuality = 1;
        portStruct.Security = 0;
        portStruct.domain = 3;			  //domain : 0 1 2 3
        portStruct.Distance = 1;
        portStruct.Direction = 0;
        m4u_config_port(&portStruct);
    }
    
    if(lcm_params->dsi.mode != CMD_MODE)
    {
        disp_path_release_mutex();
        //if(1 == lcm_params->dsi.ufoe_enable)
        //	UFOE_Start();
        //DSI_Start();
    }
    printk("%s, config done\n", __func__);

    return DISP_STATUS_OK;
}


static DISP_STATUS dsi_init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
    // DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "%s, line:%d\n", __func__, __LINE__);
    if (!disp_drv_dsi_init_context()) 
        return DISP_STATUS_NOT_IMPLEMENTED;

    // DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "%s, line:%d\n", __func__, __LINE__);
    if(lcm_params->dsi.mode == CMD_MODE) 
    {
        #ifndef MT65XX_NEW_DISP
            init_lcd();
        #endif

        init_dsi(isLcmInited);
        mdelay(1);
        
        // DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "%s, line:%d\n", __func__, __LINE__);
        if (NULL != lcm_drv->init && !isLcmInited) 
        {
            lcm_drv->init();
            DSI_LP_Reset();
        }
        // DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "%s, line:%d\n", __func__, __LINE__);
        #ifndef MT65XX_NEW_DISP
            DSI_clk_HS_mode(0);
        #else
            DSI_clk_HS_mode(1);
        #endif

        // DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "%s, line:%d\n", __func__, __LINE__);
        DSI_SetMode(lcm_params->dsi.mode);
        #ifndef MT65XX_NEW_DISP
            init_lcd_te_control();
        #endif
    }
    else 
    {
        #ifndef MT65XX_NEW_DISP
            init_intermediate_buffers(fbPA);
            
            init_lcd();
            init_dpi(isLcmInited);
        #endif

        if (!isLcmInited)
        {
            DSI_SetMode(0);
            mdelay(100);
            DSI_Stop();
        }
        else
        {
            is_video_mode_running = true;
        }
        init_dsi(isLcmInited);
        mdelay(1);
        
        if (NULL != lcm_drv->init && !isLcmInited) 
        {
            lcm_drv->init();
            DSI_LP_Reset();
        }
        
        DSI_SetMode(lcm_params->dsi.mode);
        
        #ifndef BUILD_UBOOT	
            if(lcm_params->dsi.lcm_ext_te_monitor)
            {
                is_video_mode_running = false;
                LCD_TE_SetMode(LCD_TE_MODE_VSYNC_ONLY);
                LCD_TE_SetEdgePolarity(LCM_POLARITY_RISING);
                LCD_TE_Enable(FALSE);
            }
            
            if(lcm_params->dsi.noncont_clock)
                DSI_set_noncont_clk(false, lcm_params->dsi.noncont_clock_period);
            
            if(lcm_params->dsi.lcm_int_te_monitor)
                DSI_set_int_TE(false, lcm_params->dsi.lcm_int_te_period);			
        #endif			
    }

    dsi_config_ddp(fbPA);

    printk("%s, config done\n", __func__);

    return DISP_STATUS_OK;
}


// protected by sem_early_suspend, sem_update_screen
static DISP_STATUS dsi_enable_power(BOOL enable)
{
    disp_drv_dsi_init_context();
    
    if(lcm_params->dsi.mode == CMD_MODE) {
    
        if (enable) {
            #if 0
                // Switch bus to MIPI TX.
                DSI_CHECK_RET(DSI_enable_MIPI_txio(TRUE));
                DSI_PHY_clk_switch(TRUE, lcm_params);
                DSI_PHY_clk_setting(lcm_params->dsi.pll_div1, lcm_params->dsi.pll_div2, lcm_params->dsi.LANE_NUM);
                DSI_CHECK_RET(DSI_PowerOn());
                DSI_WaitForNotBusy();		
                DSI_clk_HS_mode(0);
                DSI_clk_ULP_mode(0);			
                DSI_lane0_ULP_mode(0);	
                DSI_Reset();
                LCD_CHECK_RET(LCD_PowerOn());

            #else
                #ifndef MT65XX_NEW_DISP
                    DSI_PHY_clk_switch(TRUE, lcm_params); 
                    DSI_CHECK_RET(DSI_PowerOn());
                    if(Need_Wait_ULPS())
                        Wait_ULPS_Mode();
                    
                    DSI_PHY_clk_setting(lcm_params->dsi.pll_div1, lcm_params->dsi.pll_div2, lcm_params->dsi.LANE_NUM);

                #else
                    // enable MMSYS CG
                    DSI_CHECK_RET(DSI_PowerOn());

                    // initialize clock setting
                    DSI_PHY_clk_setting(lcm_params);

                    // initialize dsi timing
                    DSI_PHY_TIMCONFIG(lcm_params);

                    // restore dsi register
                    DSI_CHECK_RET(DSI_RestoreRegisters());

                    // enable sleep-out mode
                    DSI_CHECK_RET(DSI_SleepOut());

                    // enter HS mode
                    DSI_PHY_clk_switch(TRUE, lcm_params); 

                    // enter wakeup
                    DSI_CHECK_RET(DSI_Wakeup());

                    // enable clock
                    DSI_CHECK_RET(DSI_EnableClk());

                    // engine reset
                    DSI_Reset();
                #endif
                DSI_CHECK_RET(DSI_enable_MIPI_txio(TRUE));
                
                #ifndef MT65XX_NEW_DISP
                    Wait_WakeUp();
                #endif
            
            #endif
        }
        else {
            // backup dsi register
            DSI_CHECK_RET(DSI_WaitForEngineNotBusy());
            DSI_CHECK_RET(DSI_BackupRegisters());

            // enter ULPS mode
            DSI_clk_ULP_mode(1);
            DSI_lane0_ULP_mode(1);
            mdelay(1);
            
            // disable engine clock
            DSI_CHECK_RET(DSI_DisableClk());
            
            // disable CG
            DSI_CHECK_RET(DSI_PowerOff());

            // disable mipi pll
            DSI_PHY_clk_switch(FALSE, lcm_params);

            // Switch bus to GPIO, then power level will be decided by GPIO setting.
            DSI_CHECK_RET(DSI_enable_MIPI_txio(FALSE));
        }
    } 
    else {
        if (enable) {
            needStartDSI = true;

            // enable MMSYS CG
            DSI_CHECK_RET(DSI_PowerOn());

            // initialize clock setting
            DSI_PHY_clk_setting(lcm_params);

            // initialize dsi timing
            DSI_PHY_TIMCONFIG(lcm_params);

            // restore dsi register
            DSI_CHECK_RET(DSI_RestoreRegisters());

            // enable sleep-out mode
            DSI_CHECK_RET(DSI_SleepOut());
            
            // enter HS mode
            DSI_PHY_clk_switch(TRUE, lcm_params); 

            // enter wakeup
            DSI_CHECK_RET(DSI_Wakeup());

            // enable clock
            DSI_CHECK_RET(DSI_EnableClk());

            // engine reset
            DSI_Reset();

            DSI_CHECK_RET(DSI_enable_MIPI_txio(TRUE));
        }
        else {
            is_video_mode_running = false;
                
            if(lcm_params->dsi.noncont_clock)
                DSI_set_noncont_clk(false, lcm_params->dsi.noncont_clock_period);
                
            if(lcm_params->dsi.lcm_int_te_monitor)
                DSI_set_int_TE(false, lcm_params->dsi.lcm_int_te_period);

            // backup dsi register
            DSI_CHECK_RET(DSI_WaitForEngineNotBusy());
            DSI_CHECK_RET(DSI_BackupRegisters());

            // enter ULPS mode
            DSI_clk_ULP_mode(1);
            DSI_lane0_ULP_mode(1);
            mdelay(1);

            // disable engine clock
            DSI_CHECK_RET(DSI_DisableClk());

            // disable CG
            DSI_CHECK_RET(DSI_PowerOff());

            // disable mipi pll
            DSI_PHY_clk_switch(FALSE, lcm_params);

            // Switch bus to GPIO, then power level will be decided by GPIO setting.
            DSI_CHECK_RET(DSI_enable_MIPI_txio(FALSE));
        }
    }
    
    return DISP_STATUS_OK;
}


// protected by sem_flipping, sem_early_suspend, sem_overlay_buffer, sem_update_screen
static DISP_STATUS dsi_update_screen(BOOL isMuextLocked)
{
    disp_drv_dsi_init_context();

    DSI_CHECK_RET(DSI_enable_MIPI_txio(TRUE));

    //DSI_CHECK_RET(DSI_handle_TE());
    
    DSI_SetMode(lcm_params->dsi.mode);
#ifndef MT65XX_NEW_DISP
    LCD_CHECK_RET(LCD_StartTransfer(FALSE, isMuextLocked));
#endif

    if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE && !DDMS_capturing) {
        //if(1 != lcm_params->dsi.compatibility_for_nvk)
        if(1)
        {
            DSI_clk_HS_mode(1);
        }

        #ifndef MT65XX_NEW_DISP
            DSI_CHECK_RET(DSI_Start());
        #else
            DSI_CHECK_RET(DSI_StartTransfer(isMuextLocked));
        #endif
    }
    else if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE && !DDMS_capturing)
    {
        DSI_clk_HS_mode(1);
        DSI_CHECK_RET(DSI_StartTransfer(isMuextLocked));
        #ifndef BUILD_UBOOT
            is_video_mode_running = true;
            
            if(lcm_params->dsi.noncont_clock)
                DSI_set_noncont_clk(true, lcm_params->dsi.noncont_clock_period);
            
            if(lcm_params->dsi.lcm_int_te_monitor)
                DSI_set_int_TE(true, lcm_params->dsi.lcm_int_te_period);
        
        #endif		
    }
    
    if (DDMS_capturing)
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - dsi_update_screen. DDMS is capturing. Skip one frame. \n");		
    
    return DISP_STATUS_OK;
}


static UINT32 dsi_get_working_buffer_size(void)
{
    disp_drv_dsi_init_context();

    if(lcm_params->dsi.mode != CMD_MODE) {
    
        return 
                    lcm_params->width *
                    lcm_params->height *
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


// protected by sem_early_suspend
DISP_STATUS dsi_capture_framebuffer(UINT32 pvbuf, UINT32 bpp)
{
    DSI_CHECK_RET(DSI_WaitForNotBusy());
    
    DDMS_capturing=1;
    
    if(lcm_params->dsi.mode == CMD_MODE)
    {
        LCD_CHECK_RET(LCD_EnableDCtoDsi(FALSE));
        #ifndef MT65XX_NEW_DISP
            LCD_CHECK_RET(LCD_Capture_Framebuffer(pvbuf, bpp));
        #else
            DSI_CHECK_RET(DSI_Capture_Framebuffer(pvbuf, bpp, true));//cmd mode
        #endif
    }
    else
    {
        DSI_CHECK_RET(DSI_Capture_Framebuffer(pvbuf, bpp, false));//video mode
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
    BOOL result = false;
    
    if(lcm_params->dsi.mode == CMD_MODE)
    {
        result = lcm_drv->esd_check();
        return result;
    }
    else
    {
        #ifndef BUILD_UBOOT
            #ifndef MT65XX_NEW_DISP
                if(lcm_params->dsi.lcm_int_te_monitor)
                    result = DSI_esd_check();
                
                if(result)
                    return true;
                
                if(lcm_params->dsi.lcm_ext_te_monitor)
                    result = LCD_esd_check();
            #else
                result = DSI_esd_check();
                DSI_LP_Reset();
                needStartDSI = true;
                if(!result)
                    dsi_update_screen(true);
            #endif
            return result;
        #endif	
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
    
    }
    
}

const DISP_IF_DRIVER *DISP_GetDriverDSI(void)
{
    static const DISP_IF_DRIVER DSI_DISP_DRV =
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

