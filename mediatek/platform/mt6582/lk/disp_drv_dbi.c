/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
//#ifdef BUILD_LK
#if 1
#include <platform/disp_drv_platform.h>
#include <platform/ddp_path.h>
#else

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/semaphore.h>

#include "disp_drv.h"
#include "disp_drv_platform.h"
#include "disp_drv_log.h"

#include "lcd_drv.h"
#include "lcm_drv.h"
#include "dpi_drv.h"
#include "dsi_drv.h"

#include "lcm_drv.h"
#endif
// ---------------------------------------------------------------------------
//  Private Variables
// ---------------------------------------------------------------------------

extern LCM_DRIVER *lcm_drv;
extern LCM_PARAMS *lcm_params;
extern void * memset(void *,int,unsigned int);

// ---------------------------------------------------------------------------
//  Private Functions
// ---------------------------------------------------------------------------

static BOOL disp_drv_dbi_init_context(void)
{
    if (lcm_drv != NULL && lcm_params!= NULL) 
        return TRUE;
    else 
        printk("%s, lcm_drv=0x%08x, lcm_params=0x%08x\n", __func__, (unsigned int)lcm_drv, (unsigned int)lcm_params);

    printk("%s, lcm_drv=0x%08x\n", __func__, (unsigned int)lcm_drv);
    if (NULL == lcm_drv) {
        printk("%s, lcm_drv is NULL\n", __func__);
        return FALSE;
    }

    lcm_drv->get_params(lcm_params);


    return TRUE;
}

static void init_lcd(void)
{
    // Config LCD Controller

    LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_ALL, FALSE));
    LCD_CHECK_RET(LCD_LayerSetTriggerMode(LCD_LAYER_ALL, LCD_SW_TRIGGER));
    LCD_CHECK_RET(LCD_EnableHwTrigger(FALSE));

    LCD_CHECK_RET(LCD_SetBackgroundColor(0));
    LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, lcm_params->width, lcm_params->height));

    LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_LCM));
    LCD_CHECK_RET(LCD_WaitDPIIndication(FALSE));
    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0, FALSE));
    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_1, FALSE));
    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_2, FALSE));
}

static void init_lcd_te_control(void)
{
    const LCM_DBI_PARAMS *dbi = &(lcm_params->dbi);

    /* The board may not connect to LCM in META test mode,
       force disalbe TE to avoid blocked in LCD controller
    */
    // but for uboot, the boot mode selection is done after lcd init, so we have to disable te always in uboot.
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

static void init_io_driving_current(void)
{
	LCD_CHECK_RET(LCD_Set_DrivingCurrent(lcm_params));
}

// ---------------------------------------------------------------------------
//  DBI Display Driver Public Functions
// ---------------------------------------------------------------------------
static void init_io_pad(void)
{
   LCD_CHECK_RET(LCD_Init_IO_pad(lcm_params));
}

static DISP_STATUS dbi_init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
    if (!disp_drv_dbi_init_context()) 
        return DISP_STATUS_NOT_IMPLEMENTED;
//#if 0
//#ifdef MT65XX_NEW_DISP
#if 1
		{
			struct disp_path_config_struct config;
			memset(&config, 0x0, sizeof(config));

			config.srcModule = DISP_MODULE_OVL;
	
			if(config.srcModule == DISP_MODULE_RDMA)
			{
				config.inFormat = RDMA_INPUT_FORMAT_RGB565;
				config.addr = fbPA; 
				config.pitch = DISP_GetScreenWidth()*2;
				config.srcROI.x = 0;config.srcROI.y = 0;
				config.srcROI.height= DISP_GetScreenHeight();config.srcROI.width= DISP_GetScreenWidth();
			}
			else
			{			
				config.bgROI.x = 0;
				config.bgROI.y = 0;
				config.bgROI.width = DISP_GetScreenWidth();
				config.bgROI.height = DISP_GetScreenHeight();
				config.bgColor = 0x0;	// background color
				config.pitch = DISP_GetScreenWidth()*2;

				config.srcROI.x = 0;config.srcROI.y = 0;
				config.srcROI.height= DISP_GetScreenHeight();config.srcROI.width= DISP_GetScreenWidth();
				config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
#if 0
				config.ovl_config.layer = 2;
				config.ovl_config.layer_en = 1; 
				config.ovl_config.fmt = OVL_INPUT_FORMAT_RGB565;
				config.ovl_config.addr = fbPA;	
				config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
				config.ovl_config.x = 0;		// ROI
				config.ovl_config.y = 0;  
				config.ovl_config.w = DISP_GetScreenWidth();  
				config.ovl_config.h = DISP_GetScreenHeight(); 
				config.ovl_config.pitch = DISP_GetScreenWidth()*2; //pixel number
				config.ovl_config.keyEn = 1;
				config.ovl_config.key = 0xFFFFFFFF; 	// color key
				config.ovl_config.aen = 0;			   // alpha enable
				config.ovl_config.alpha = 0;  
#endif
			}
			
			config.dstModule = DISP_MODULE_DBI;// DISP_MODULE_WDMA1
			if(config.dstModule == DISP_MODULE_DBI)
				config.outFormat = RDMA_OUTPUT_FORMAT_ARGB; 
			else
				config.outFormat = WDMA_OUTPUT_FORMAT_ARGB; 		
			disp_path_get_mutex();
			disp_path_config(&config);

			disp_bls_config();
		}
#endif


    init_io_pad();
	init_io_driving_current();
    init_lcd();

    if (NULL != lcm_drv->init && !isLcmInited) {
        lcm_drv->init();
    }

    init_lcd_te_control();
    DPI_PowerOn();
    DPI_PowerOff();
    DSI_PowerOn();
    DSI_PowerOff();

    return DISP_STATUS_OK;
}

static DISP_STATUS dbi_init_old(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
	printf("%s\n", __func__);
    if (!disp_drv_dbi_init_context()) 
        return DISP_STATUS_NOT_IMPLEMENTED;
    init_io_pad();
	init_io_driving_current();
    init_lcd();
		printf("%s, %d\n", __func__, __LINE__);

    if (NULL != lcm_drv->init && !isLcmInited) {
        lcm_drv->init();
    }
		printf("%s, %d\n", __func__, __LINE__);

    init_lcd_te_control();
		// what the hell???
		#if 0
    DPI_PowerOn();
    DPI_PowerOff();
    DSI_PowerOn();
    DSI_PowerOff();
		#endif
		printf("%s, %d\n", __func__, __LINE__);

    return DISP_STATUS_OK;
}


static DISP_STATUS dbi_enable_power(BOOL enable)
{
    if (enable) {
	LCD_CHECK_RET(LCD_PowerOn());
        init_io_pad();
    } else {
        LCD_CHECK_RET(LCD_PowerOff());
    }
    return DISP_STATUS_OK;
}


static DISP_STATUS dbi_update_screen(void)
{

    LCD_CHECK_RET(LCD_StartTransfer(TRUE));

    return DISP_STATUS_OK;
}


static UINT32 dbi_get_working_buffer_size(void)
{
    return 0;
}

static UINT32 dbi_get_working_buffer_bpp(void)
{
    return 0;
}



static PANEL_COLOR_FORMAT dbi_get_panel_color_format(void)
{
    disp_drv_dbi_init_context();

    switch(lcm_params->dbi.data_format.format)
    {
    case LCM_DBI_FORMAT_RGB332 : return PANEL_COLOR_FORMAT_RGB332;
    case LCM_DBI_FORMAT_RGB444 : return PANEL_COLOR_FORMAT_RGB444;
    case LCM_DBI_FORMAT_RGB565 : return PANEL_COLOR_FORMAT_RGB565;
    case LCM_DBI_FORMAT_RGB666 : return PANEL_COLOR_FORMAT_RGB666;
    case LCM_DBI_FORMAT_RGB888 : return PANEL_COLOR_FORMAT_RGB888;
    default : ASSERT(0);
    }
    return PANEL_COLOR_FORMAT_RGB888;
}


static UINT32 dbi_get_dithering_bpp(void)
{
	return PANEL_COLOR_FORMAT_TO_BPP(dbi_get_panel_color_format());
}

DISP_STATUS dbi_capture_framebuffer(UINT32 pvbuf, UINT32 bpp)
{
    LCD_CHECK_RET(LCD_Capture_Framebuffer(pvbuf, bpp));

	return DISP_STATUS_OK;	
}

const DISP_DRIVER *DISP_GetDriverDBI()
{
    static const DISP_DRIVER DBI_DISP_DRV =
    {
        .init                   = dbi_init,
        .enable_power           = dbi_enable_power,
        .update_screen          = dbi_update_screen,
        
        .get_working_buffer_size = dbi_get_working_buffer_size,
        .get_working_buffer_bpp = dbi_get_working_buffer_bpp,
        .get_panel_color_format = dbi_get_panel_color_format,
        .init_te_control        = init_lcd_te_control,
        .get_dithering_bpp		= dbi_get_dithering_bpp,
		.capture_framebuffer	= dbi_capture_framebuffer, 
    };

    return &DBI_DISP_DRV;
}

