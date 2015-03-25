/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 * Infinity Chen <infinity.chen@mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/

//#include <platform/mt65xx.h>
//#include <platform/mt_gpio.h>
#include <string.h>
#include <video_fb.h>
#include <platform/mt_gpt.h>
#include <platform/disp_drv_platform.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>
#include <target/board.h>
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Export Functions - Display
// ---------------------------------------------------------------------------

static void  *fb_addr      = NULL;
static void  *logo_db_addr = NULL;
static UINT32 fb_size      = 0;
static UINT32 fb_offset_logo = 0; // counter of fb_size
static UINT32 fb_isdirty   = 0;

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
extern LCM_PARAMS *lcm_params;


UINT32 mt_disp_get_vram_size(void)
{
   return DISP_GetVRamSize();
}

extern void disp_log_enable(int enable);
extern void dbi_log_enable(int enable);


#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

void mt_disp_init(void *lcdbase)
{
   UINT32 boot_mode_addr = 0;

   fb_size = ALIGN_TO(CFG_DISPLAY_WIDTH, 32) * ALIGN_TO(CFG_DISPLAY_HEIGHT, 32) * CFG_DISPLAY_BPP / 8;
   boot_mode_addr = (UINT32)((UINT32)lcdbase + fb_size);
   logo_db_addr = (void *)((UINT32)lcdbase - 4 * 1024 * 1024);
   //    fb_addr      = (void *)((UINT32)lcdbase + fb_size);
   fb_addr  =   lcdbase;
   fb_offset_logo = 3;
   
   ///for debug prupose
   disp_log_enable(1);
   dbi_log_enable(1);
   
   DISP_CHECK_RET(DISP_Init((UINT32)lcdbase, (UINT32)lcdbase, FALSE));
   
   memset((void*)lcdbase, 0x0, DISP_GetVRamSize());
    /* transparent front buffer for fb_console display */
#if 1
    LCD_CHECK_RET(LCD_LayerEnable(FB_LAYER, TRUE));
    LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER, (UINT32)boot_mode_addr));
    LCD_CHECK_RET(LCD_LayerSetFormat(FB_LAYER, LCD_LAYER_FORMAT_RGB565));
    LCD_CHECK_RET(LCD_LayerSetPitch(FB_LAYER, CFG_DISPLAY_WIDTH*2));
    LCD_CHECK_RET(LCD_LayerSetOffset(FB_LAYER, 0, 0));
    LCD_CHECK_RET(LCD_LayerSetSize(FB_LAYER, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT));
    LCD_CHECK_RET(LCD_LayerSetSourceColorKey(FB_LAYER, TRUE, 0xff000000));
#endif
   
   /* background buffer for uboot logo display */
   LCD_CHECK_RET(LCD_LayerEnable(FB_LAYER - 1, TRUE));
   LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER - 1, (UINT32)fb_addr));
   LCD_CHECK_RET(LCD_LayerSetFormat(FB_LAYER - 1, LCD_LAYER_FORMAT_RGB565));
   LCD_CHECK_RET(LCD_LayerSetOffset(FB_LAYER - 1, 0, 0));
   LCD_CHECK_RET(LCD_LayerSetSize(FB_LAYER - 1, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT));
   LCD_CHECK_RET(LCD_LayerSetPitch(FB_LAYER - 1, ALIGN_TO(CFG_DISPLAY_WIDTH, 32)*2));
   
   if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3))
   {
      LCD_CHECK_RET(LCD_LayerSetRotation(FB_LAYER, LCD_LAYER_ROTATE_180));
      LCD_CHECK_RET(LCD_LayerSetRotation(FB_LAYER - 1, LCD_LAYER_ROTATE_180));
   }
}


void mt_disp_power(BOOL on)
{
    if (on) {
        disp_path_ddp_clock_on();
        DISP_PowerEnable(TRUE);
        DISP_PanelEnable(TRUE);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    } else {
        DISP_PanelEnable(FALSE);
        DISP_PowerEnable(FALSE);
        disp_path_ddp_clock_off();
    }
}


void* mt_get_logo_db_addr(void)
{
   return logo_db_addr;
}


void* mt_get_fb_addr(void)
{
    fb_isdirty = 1;
    return (void*)((UINT32)fb_addr + fb_offset_logo * fb_size);
}

void* mt_get_tempfb_addr(void)
{
    //use offset = 2 as tempfb for decompress logo
    return (void*)((UINT32)fb_addr + 2 * fb_size);
}

UINT32 mt_get_fb_size(void)
{
   return fb_size;
}


void mt_disp_update(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
    if(fb_isdirty)
    {
        fb_isdirty = 0;
        MASKREG32(DISP_REG_CONFIG_MUTEX_INTEN, 0x1, 0x1); //Enable DISP MUTEX0
        MASKREG32(DISP_REG_CONFIG_MUTEX_INTSTA, 0x1, 0x0);
        LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER - 1, (UINT32)fb_addr + fb_offset_logo * fb_size));
        printk("[wwy] hardware address = %x, fb_offset_logo = %d\n",(UINT32)fb_addr + fb_offset_logo * fb_size,fb_offset_logo);
        DISP_CHECK_RET(DISP_UpdateScreen(x, y, width, height));
        //wait reg update to set fb_offset_logo
        DISP_WaitRegUpdate();
        fb_offset_logo = fb_offset_logo ? 0 : 3;

    }
    else
    {
        DISP_CHECK_RET(DISP_UpdateScreen(x, y, width, height));
    }
}


void mt_disp_wait_idle(void)
{
   LCD_CHECK_RET(LCD_WaitForNotBusy());
}

static void mt_disp_adjusting_hardware_addr(void)
{
    printf("[wwy] mt_disp_adjusting_hardware_addr fb_offset_logo = %d\n",fb_offset_logo);
    if(fb_offset_logo == 0)
    {
        mt_get_fb_addr();
        memcpy(fb_addr,(void *)((UINT32)fb_addr + 3 * fb_size),fb_size);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    }
}

UINT32 mt_disp_get_lcd_time(void)
{
   UINT32 time0, time1, lcd_time;

   mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
   
   if(lcm_params->type==LCM_TYPE_DPI)
      DPI_WaitVsync();
   else if(lcm_params->type==LCM_TYPE_DBI)
      LCD_WaitForNotBusy();
   else{//DSI
      if(lcm_params->dsi.mode!=CMD_MODE)
         DSI_WaitVsync();
      else
         DSI_WaitForNotBusy();
   }
   time0 = gpt4_tick2time_us(gpt4_get_current_tick());

   if(lcm_params->type==LCM_TYPE_DPI)
      DPI_WaitVsync();
   else if(lcm_params->type==LCM_TYPE_DBI){
      mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
      LCD_WaitForNotBusy();
   }
   else{//DSI
      if(lcm_params->dsi.mode!=CMD_MODE)
         DSI_WaitVsync();
      else
      {
         DSI_StartTransfer(TRUE);
         DSI_WaitForNotBusy();
      }
      //			DSI_WaitBtaTE();
   }
   
   time1 = gpt4_tick2time_us(gpt4_get_current_tick());
   
   lcd_time = time1 - time0;
#if 0	//workaround for DSI video mode sanity issue
	DSI_SetMode(0);
	msleep(100);
	DSI_DisableClk();
#endif
   printf("lcd one %d \n", lcd_time);
    mt_disp_adjusting_hardware_addr();

   if(0 != lcd_time)	
      return (100000000/lcd_time);
   else
      return (6000);
}

const char* mt_disp_get_lcm_id(void)
{
   return DISP_GetLCMId();
}


void disp_get_fb_address(UINT32 *fbVirAddr, UINT32 *fbPhysAddr)
{
   *fbVirAddr = (UINT32)fb_addr;
   *fbPhysAddr = (UINT32)fb_addr;
}

// ---------------------------------------------------------------------------
//  Export Functions - Console
// ---------------------------------------------------------------------------

#ifdef CONFIG_CFB_CONSOLE

//  video_hw_init -- called by drv_video_init() for framebuffer console

extern UINT32 memory_size(void);

void *video_hw_init (void)
{
   static GraphicDevice s_mt65xx_gd;
   
   memset(&s_mt65xx_gd, 0, sizeof(GraphicDevice));
   //xuecheng, use new calculate formula;
   s_mt65xx_gd.frameAdrs  = (unsigned int)fb_addr+fb_size;//CFG_DISPLAY_WIDTH*2*(CFG_DISPLAY_HEIGHT-80);//fb_size;//memory_size() - mt_disp_get_vram_size() + fb_size;
   s_mt65xx_gd.winSizeX   = CFG_DISPLAY_WIDTH;
   s_mt65xx_gd.winSizeY   = CFG_DISPLAY_HEIGHT;
   s_mt65xx_gd.gdfIndex   = GDF_16BIT_565RGB;
   s_mt65xx_gd.gdfBytesPP = CFG_DISPLAY_BPP / 8;
   s_mt65xx_gd.memSize    = s_mt65xx_gd.winSizeX * s_mt65xx_gd.winSizeY * s_mt65xx_gd.gdfBytesPP;
   
   return &s_mt65xx_gd;
}


void video_set_lut(unsigned int index,  /* color number */
                   unsigned char r,     /* red */
                   unsigned char g,     /* green */
                   unsigned char b)     /* blue */
{
}

#endif  // CONFIG_CFB_CONSOLE

