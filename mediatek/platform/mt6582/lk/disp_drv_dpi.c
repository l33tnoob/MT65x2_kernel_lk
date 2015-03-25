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
#include <string.h>
#else

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/semaphore.h>

#include "disp_drv.h"
#include "disp_drv_platform.h"
#include "disp_drv_log.h"

#include "lcd_drv.h"
#include "dpi_drv.h"
#include "dsi_drv.h"

#include "lcm_drv.h"
#endif
// ---------------------------------------------------------------------------
//  Private Variables
// ---------------------------------------------------------------------------

extern LCM_DRIVER *lcm_drv;
extern LCM_PARAMS *lcm_params;

static DPI_FB_FORMAT dpiTmpBufFormat = DPI_FB_FORMAT_RGB888;
static LCD_FB_FORMAT lcdTmpBufFormat = LCD_FB_FORMAT_RGB888;
static UINT32 dpiTmpBufBpp = 0;
typedef struct
{
    UINT32 pa;
    UINT32 pitchInBytes;
} TempBuffer;

static TempBuffer s_tmpBuffers[3];


// ---------------------------------------------------------------------------
//  Private Functions
// ---------------------------------------------------------------------------

static __inline DPI_FB_FORMAT get_tmp_buffer_dpi_format(void)
{
    switch(lcm_params->dpi.format)
    {
    case LCM_DPI_FORMAT_RGB565 : return DPI_FB_FORMAT_RGB565;
    case LCM_DPI_FORMAT_RGB666 :
    case LCM_DPI_FORMAT_RGB888 : return DPI_FB_FORMAT_RGB888;
    default : ASSERT(0);
    }
    return DPI_FB_FORMAT_RGB888;
}


static __inline UINT32 get_tmp_buffer_bpp(void)
{
    static const UINT32 TO_BPP[] = {2, 3};
    return TO_BPP[dpiTmpBufFormat];
}


static __inline LCD_FB_FORMAT get_tmp_buffer_lcd_format(void)
{
    static const UINT32 TO_LCD_FORMAT[] = {
        LCD_FB_FORMAT_RGB565,
        LCD_FB_FORMAT_RGB888
    };
    
    return TO_LCD_FORMAT[dpiTmpBufFormat];
}


static BOOL disp_drv_dpi_init_context(void)
{
    dpiTmpBufFormat = get_tmp_buffer_dpi_format();
    lcdTmpBufFormat = get_tmp_buffer_lcd_format();
    dpiTmpBufBpp    = get_tmp_buffer_bpp();

    if (lcm_drv != NULL && lcm_params!= NULL) 
        return TRUE;
    else 
        printk("%s, lcm_drv=0x%08x, lcm_params=0x%08x\n", __func__, (unsigned int)lcm_drv, (unsigned int)lcm_params);

    if (NULL == lcm_drv) {
        printk("%s, lcm_drv is NULL\n", __func__);

        return FALSE;
    }

 
    return TRUE;
}

static void init_intermediate_buffers(UINT32 fbPhysAddr)
{
    UINT32 tmpFbStartPA = fbPhysAddr;

    UINT32 tmpFbPitchInBytes = DISP_GetScreenWidth() * dpiTmpBufBpp;
    UINT32 tmpFbSizeInBytes  = tmpFbPitchInBytes * DISP_GetScreenHeight();

    UINT32 i;
    
    for (i = 0; i < lcm_params->dpi.intermediat_buffer_num; ++ i)
    {
        TempBuffer *b = &s_tmpBuffers[i];
        
#ifdef BUILD_LK
        // clean the intermediate buffers as black to prevent from noise display
        memset((void *)tmpFbStartPA, 0, tmpFbSizeInBytes);
#endif
        b->pitchInBytes = tmpFbPitchInBytes;
        b->pa = tmpFbStartPA;
        ASSERT((tmpFbStartPA & 0x7) == 0);  // check if 8-byte-aligned
        tmpFbStartPA += tmpFbSizeInBytes;
    }

}


static void init_mipi_pll(void)
{
    DPI_CHECK_RET(DPI_Init_PLL(lcm_params->dpi.mipi_pll_clk_ref,
	           lcm_params->dpi.mipi_pll_clk_div1,lcm_params->dpi.mipi_pll_clk_div2));
}


static void init_io_pad(void)
{
    LCD_CHECK_RET(LCD_Init_IO_pad(lcm_params));
    
}

static void init_io_driving_current(void)
{
	DPI_CHECK_RET(DPI_Set_DrivingCurrent(lcm_params));
}

static void init_lcd(void)
{
    UINT32 i;

    LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_ALL, FALSE));
    LCD_CHECK_RET(LCD_LayerSetTriggerMode(LCD_LAYER_ALL, LCD_SW_TRIGGER));
    LCD_CHECK_RET(LCD_EnableHwTrigger(FALSE));

    LCD_CHECK_RET(LCD_SetBackgroundColor(0));
    LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, DISP_GetScreenWidth(), DISP_GetScreenHeight()));

    LCD_CHECK_RET(LCD_FBSetFormat(lcdTmpBufFormat));
    LCD_CHECK_RET(LCD_FBSetPitch(s_tmpBuffers[0].pitchInBytes));
    LCD_CHECK_RET(LCD_FBSetStartCoord(0, 0));

    for (i = 0; i < lcm_params->dpi.intermediat_buffer_num; ++ i)
    {
        LCD_CHECK_RET(LCD_FBSetAddress(LCD_FB_0 + i, s_tmpBuffers[i].pa));
        LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0 + i, TRUE));
    }
    
    LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_MEM));
    /**
       "LCD Delay Enable" function should be used when there is only
       single buffer between LCD and DPI.
       Double buffer even triple buffer need not enable it.
    */
    LCD_CHECK_RET(LCD_WaitDPIIndication(TRUE));
}


static void init_dpi(BOOL isDpiPoweredOn)
{
    const LCM_DPI_PARAMS *dpi = &(lcm_params->dpi);
    UINT32 i;

    DPI_CHECK_RET(DPI_Init(isDpiPoweredOn));

    DPI_CHECK_RET(DPI_EnableSeqOutput(FALSE));

    DPI_CHECK_RET(DPI_ConfigPixelClk((DPI_POLARITY)dpi->clk_pol,
                                     dpi->dpi_clk_div, dpi->dpi_clk_duty));

    DPI_CHECK_RET(DPI_ConfigDataEnable((DPI_POLARITY)dpi->de_pol));

    DPI_CHECK_RET(DPI_ConfigHsync((DPI_POLARITY)dpi->hsync_pol,
                                  dpi->hsync_pulse_width,
                                  dpi->hsync_back_porch,
                                  dpi->hsync_front_porch));

    DPI_CHECK_RET(DPI_ConfigVsync((DPI_POLARITY)dpi->vsync_pol,
                                  dpi->vsync_pulse_width,
                                  dpi->vsync_back_porch,
                                  dpi->vsync_front_porch));

#ifdef MT65XX_NEW_DISP
	DPI_CHECK_RET(DPI_ConfigLVDS(lcm_params));
#endif

    DPI_CHECK_RET(DPI_FBSetSize(DISP_GetScreenWidth(), DISP_GetScreenHeight()));
    
    for (i = 0; i < dpi->intermediat_buffer_num; ++ i)
    {
        DPI_CHECK_RET(DPI_FBSetAddress(DPI_FB_0 + i, s_tmpBuffers[i].pa));
        DPI_CHECK_RET(DPI_FBSetPitch(DPI_FB_0 + i, s_tmpBuffers[i].pitchInBytes));
        DPI_CHECK_RET(DPI_FBEnable(DPI_FB_0 + i, TRUE));
    }
    DPI_CHECK_RET(DPI_FBSetFormat(dpiTmpBufFormat));
    DPI_CHECK_RET(DPI_FBSyncFlipWithLCD(TRUE));

    if (LCM_COLOR_ORDER_BGR == dpi->rgb_order) {
        DPI_CHECK_RET(DPI_SetRGBOrder(DPI_RGB_ORDER_RGB, DPI_RGB_ORDER_BGR));
    } else {
        DPI_CHECK_RET(DPI_SetRGBOrder(DPI_RGB_ORDER_RGB, DPI_RGB_ORDER_RGB));
    }

    DPI_CHECK_RET(DPI_EnableClk());
}


// ---------------------------------------------------------------------------
//  DPI Display Driver Public Functions
// ---------------------------------------------------------------------------

static DISP_STATUS dpi_init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
    if (!disp_drv_dpi_init_context()) 
        return DISP_STATUS_NOT_IMPLEMENTED;
#ifndef MT65XX_NEW_DISP
    init_intermediate_buffers(fbPA);
#else
	{
		struct disp_path_config_struct config;
		memset(&config, 0x0, sizeof(config));

		config.srcModule = DISP_MODULE_OVL;

		if(config.srcModule == DISP_MODULE_RDMA)
        {
            config.inFormat = RDMA_INPUT_FORMAT_RGB565;
            config.addr = fbPA; 
			
			config.pitch = ALIGN_TO(DISP_GetScreenWidth()*2, MTK_FB_ALIGNMENT);
			
			config.srcROI.x = 0;config.srcROI.y = 0;
			config.srcROI.height= DISP_GetScreenHeight();config.srcROI.width= DISP_GetScreenWidth();
	    }
		else
		{			
	        config.bgROI.x = 0;
	        config.bgROI.y = 0;
	        config.bgROI.width = DISP_GetScreenWidth();
	        config.bgROI.height = DISP_GetScreenHeight();
	        config.bgColor = 0x0;  // background color

			config.srcROI.x = 0;config.srcROI.y = 0;
			config.srcROI.height= DISP_GetScreenHeight();config.srcROI.width= DISP_GetScreenWidth();
			config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
#if 1
	        config.ovl_config.layer = 2;
			config.ovl_config.layer_en = 1;	
			config.ovl_config.fmt = OVL_INPUT_FORMAT_RGB565;
			config.ovl_config.addr = fbPA;	
	        config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
			config.ovl_config.x = 0;        // ROI
	        config.ovl_config.y = 0;  
	        config.ovl_config.w = DISP_GetScreenWidth();  
	        config.ovl_config.h = DISP_GetScreenHeight(); 
			config.ovl_config.pitch =  ALIGN_TO(DISP_GetScreenWidth()*2, MTK_FB_ALIGNMENT);
		    config.ovl_config.keyEn = 0;
	        config.ovl_config.key = 0xff;     // color key
	        config.ovl_config.aen = 0;             // alpha enable
	        config.ovl_config.alpha = 0;  
#endif
		}
		
		config.dstModule = DISP_MODULE_DPI0;// DISP_MODULE_WDMA1
		if(config.dstModule == DISP_MODULE_DPI0)
            config.outFormat = RDMA_OUTPUT_FORMAT_ARGB; 
		else
			config.outFormat = WDMA_OUTPUT_FORMAT_ARGB;         
        disp_path_get_mutex();
        disp_path_config(&config);
		disp_path_release_mutex();
        
		disp_bls_config();
	}
#endif
    init_mipi_pll();
    init_io_pad();
	init_io_driving_current();

    init_lcd();
    init_dpi(isLcmInited);

    if (NULL != lcm_drv->init && !isLcmInited) {
        lcm_drv->init();
    }
    DSI_PowerOn();
    DSI_PowerOff();

    return DISP_STATUS_OK;
}


static DISP_STATUS dpi_enable_power(BOOL enable)
{
    if (enable) {
        DPI_CHECK_RET(DPI_PowerOn());

        init_mipi_pll();//for MT6573 and later chip, Must re-init mipi pll for dpi, because pll register have located in
		                //MMSYS1 except MT6516
        init_io_pad();
        LCD_CHECK_RET(LCD_PowerOn());
        DPI_CHECK_RET(DPI_EnableClk());
        DPI_EnableIrq();
    } else {
        DPI_DisableIrq();
        DPI_CHECK_RET(DPI_DisableClk());
        DPI_CHECK_RET(DPI_PowerOff());
        LCD_CHECK_RET(LCD_PowerOff());
        DPI_mipi_switch(FALSE);
    }
    return DISP_STATUS_OK;
}


static DISP_STATUS dpi_update_screen(void)
{
#ifndef MT65XX_NEW_DISP
    LCD_CHECK_RET(LCD_StartTransfer(FALSE));
#else
	disp_path_get_mutex();
	LCD_CHECK_RET(LCD_ConfigOVL());
	disp_path_release_mutex();
#endif
    return DISP_STATUS_OK;
}

static UINT32 dpi_get_working_buffer_size(void)
{
    UINT32 size = 0;
    UINT32 framePixels = 0;

    disp_drv_dpi_init_context();

    framePixels = DISP_GetScreenWidth() * DISP_GetScreenHeight();

    size = framePixels * dpiTmpBufBpp * 
           lcm_params->dpi.intermediat_buffer_num;

    return size;
}

static UINT32 dpi_get_working_buffer_bpp(void)
{
    disp_drv_dpi_init_context();

    return dpiTmpBufBpp;
}
static PANEL_COLOR_FORMAT dpi_get_panel_color_format(void)
{
    disp_drv_dpi_init_context();

    switch(lcm_params->dpi.format)
    {
    case LCM_DPI_FORMAT_RGB565 : return PANEL_COLOR_FORMAT_RGB565;
    case LCM_DPI_FORMAT_RGB666 : return PANEL_COLOR_FORMAT_RGB666;
    case LCM_DPI_FORMAT_RGB888 : return PANEL_COLOR_FORMAT_RGB888;
    default : ASSERT(0);
    }
    return PANEL_COLOR_FORMAT_RGB888;
}



DISP_STATUS dpi_capture_framebuffer(UINT32 pvbuf, UINT32 bpp)
{
    LCD_CHECK_RET(DPI_Capture_Framebuffer(pvbuf, bpp));

	return DISP_STATUS_OK;	
}


#define MIN(x,y) ((x)>(y)?(y):(x))
static UINT32 dpi_get_dithering_bpp(void)
{
	return MIN(get_tmp_buffer_bpp() * 8,PANEL_COLOR_FORMAT_TO_BPP(dpi_get_panel_color_format()));
}

const DISP_DRIVER *DISP_GetDriverDPI()
{
    static const DISP_DRIVER DPI_DISP_DRV =
    {
        .init                   = dpi_init,
        .enable_power           = dpi_enable_power,
        .update_screen          = dpi_update_screen,
        
        .get_working_buffer_size = dpi_get_working_buffer_size,
        .get_working_buffer_bpp = dpi_get_working_buffer_bpp,
        .get_panel_color_format = dpi_get_panel_color_format,
        .get_dithering_bpp		= dpi_get_dithering_bpp,
		.capture_framebuffer	= dpi_capture_framebuffer, 
    };

    return &DPI_DISP_DRV;
}


