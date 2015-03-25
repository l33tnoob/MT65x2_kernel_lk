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
#else
#include "disp_drv.h"
#include "disp_drv_platform.h"
#include "disp_drv_log.h"
#include <linux/disp_assert_layer.h>
#endif

///common part
#define DAL_BPP             (2)
#define DAL_WIDTH           (DISP_GetScreenWidth())
#define DAL_HEIGHT          (DISP_GetScreenHeight())

#ifdef CONFIG_MTK_FB_SUPPORT_ASSERTION_LAYER

#include <linux/string.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>
#include <linux/module.h>

#include "lcd_drv.h"
#include "mtkfb_console.h"

// ---------------------------------------------------------------------------
#define DAL_FORMAT          (LCD_LAYER_FORMAT_RGB565)
#define DAL_BG_COLOR        (dal_bg_color)
#define DAL_FG_COLOR        (dal_fg_color)

#define RGB888_To_RGB565(x) ((((x) & 0xF80000) >> 8) |                      \
                             (((x) & 0x00FC00) >> 5) |                      \
                             (((x) & 0x0000F8) >> 3))

#define MAKE_TWO_RGB565_COLOR(high, low)  (((low) << 16) | (high))

#define DAL_LOCK()                                                          \
    do {                                                                    \
        if (down_interruptible(&dal_sem)) {                                 \
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DAL", "Can't get semaphore in %s()\n",          \
                   __FUNCTION__);                                           \
            return DAL_STATUS_LOCK_FAIL;                                    \
        }                                                                   \
    } while (0)
    
#define DAL_UNLOCK()                                                        \
    do {                                                                    \
        up(&dal_sem);                                                       \
    } while (0)


#define DAL_CHECK_MFC_RET(expr)                                             \
    do {                                                                    \
        MFC_STATUS ret = (expr);                                            \
        if (MFC_STATUS_OK != ret) {                                         \
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DAL", "Warning: call MFC_XXX function failed "           \
                   "in %s(), line: %d, ret: %x\n",                          \
                   __FUNCTION__, __LINE__, ret);                            \
            return ret;                                                     \
        }                                                                   \
    } while (0)


#define DAL_CHECK_LCD_RET(expr)                                             \
    do {                                                                    \
        LCD_STATUS ret = (expr);                                            \
        if (LCD_STATUS_OK != ret) {                                         \
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DAL", "Warning: call LCD_XXX function failed "           \
                   "in %s(), line: %d, ret: %x\n",                          \
                   __FUNCTION__, __LINE__, ret);                            \
            return ret;                                                     \
        }                                                                   \
    } while (0)


#define DAL_CHECK_DISP_RET(expr)                                            \
    do {                                                                    \
        DISP_STATUS ret = (expr);                                           \
        if (DISP_STATUS_OK != ret) {                                        \
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DAL", "Warning: call DISP_XXX function failed "          \
                   "in %s(), line: %d, ret: %x\n",                          \
                   __FUNCTION__, __LINE__, ret);                            \
            return ret;                                                     \
        }                                                                   \
    } while (0)

#define DAL_LOG(fmt, arg...) \
    do { \
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DAL", fmt, ##arg); \
    }while (0)
// ---------------------------------------------------------------------------

static MFC_HANDLE mfc_handle = NULL;

static void *dal_fb_addr = NULL;    
static BOOL  dal_shown   = FALSE;
static BOOL  dal_enable_when_resume = FALSE;
static BOOL  dal_disable_when_resume = FALSE;
static unsigned int dal_fg_color = RGB888_To_RGB565(DAL_COLOR_WHITE);
static unsigned int dal_bg_color = RGB888_To_RGB565(DAL_COLOR_RED);

#define DAL_LOWMEMORY_ASSERT

#ifdef DAL_LOWMEMORY_ASSERT

static unsigned int dal_lowmemory_fg_color = RGB888_To_RGB565(DAL_COLOR_PINK);
static unsigned int dal_lowmemory_bg_color = RGB888_To_RGB565(DAL_COLOR_YELLOW);
static BOOL  dal_enable_when_resume_lowmemory = FALSE;
static BOOL  dal_disable_when_resume_lowmemory = FALSE;
static BOOL  dal_lowMemory_shown = FALSE;
static const char low_memory_msg[] = {"Low Memory!"};
#define DAL_LOWMEMORY_FG_COLOR (dal_lowmemory_fg_color)
#define DAL_LOWMEMORY_BG_COLOR (dal_lowmemory_bg_color)
#endif
//DECLARE_MUTEX(dal_sem);
DEFINE_SEMAPHORE(dal_sem);

static char dal_print_buffer[1024];
// ---------------------------------------------------------------------------
typedef struct
{
    struct semaphore sem;

    unsigned char *fb_addr;
    unsigned int fb_width;
    unsigned int fb_height;
    unsigned int fb_bpp;
    unsigned int fg_color;
    unsigned int bg_color;
    unsigned int rows;
    unsigned int cols;
    unsigned int cursor_row;
    unsigned int cursor_col;
	unsigned int font_width;
	unsigned int font_height;
} MFC_CONTEXT;

#ifdef DAL_LOWMEMORY_ASSERT
static DAL_STATUS Show_LowMemory(void)
{
	UINT32 update_width, update_height;
	MFC_CONTEXT *ctxt = (MFC_CONTEXT *)mfc_handle;

	if(!dal_shown){//only need show lowmemory assert
			update_width = ctxt->font_width * strlen(low_memory_msg);
			update_height = ctxt->font_height;
			DISP_LOG_PRINT(ANDROID_LOG_INFO, "DAL", "update size:%d,%d",update_width, update_height);
			DAL_CHECK_LCD_RET(LCD_LayerSetOffset(ASSERT_LAYER, DAL_WIDTH - update_width,0));
			DAL_CHECK_LCD_RET(LCD_LayerSetWindowOffset(ASSERT_LAYER, DAL_WIDTH - update_width,0));
    		DAL_CHECK_LCD_RET(LCD_LayerSetSize(ASSERT_LAYER,
                                       update_width,update_height));
			DAL_CHECK_LCD_RET(LCD_LayerEnable(ASSERT_LAYER, TRUE));
	}
/*
	if(!dal_lowMemory_shown)
		DAL_CHECK_DISP_RET(DISP_UpdateScreen(0, 0, 
                                         DAL_WIDTH,
                                         DAL_HEIGHT));
*/
    return DAL_STATUS_OK;
}
#endif

UINT32 DAL_GetLayerSize(void)
{
	// xuecheng, avoid lcdc read buffersize+1 issue
    return DAL_WIDTH * DAL_HEIGHT * DAL_BPP + 4096;
}


DAL_STATUS DAL_Init(UINT32 layerVA, UINT32 layerPA)
{
    dal_fb_addr = (void *)layerVA;
    DAL_CHECK_MFC_RET(MFC_Open(&mfc_handle, dal_fb_addr,
                               DAL_WIDTH, DAL_HEIGHT, DAL_BPP,
                               DAL_FG_COLOR, DAL_BG_COLOR));

    DAL_Clean();

    DAL_CHECK_LCD_RET(LCD_LayerSetAddress(ASSERT_LAYER, layerPA));
    DAL_CHECK_LCD_RET(LCD_LayerSetFormat(ASSERT_LAYER, DAL_FORMAT));
    DAL_CHECK_LCD_RET(LCD_LayerSetAlphaBlending(ASSERT_LAYER, TRUE, 0x80));
    DAL_CHECK_LCD_RET(LCD_LayerSetOffset(ASSERT_LAYER, 0, 0));
    DAL_CHECK_LCD_RET(LCD_LayerSetSize(ASSERT_LAYER,
                                       DAL_WIDTH,
                                       DAL_HEIGHT));
    DAL_CHECK_LCD_RET(LCD_LayerSetPitch(ASSERT_LAYER, DAL_WIDTH * DAL_BPP));

    return DAL_STATUS_OK;
}


DAL_STATUS DAL_SetColor(unsigned int fgColor, unsigned int bgColor)
{
    if (NULL == mfc_handle) 
        return DAL_STATUS_NOT_READY;

    DAL_LOCK();
    dal_fg_color = RGB888_To_RGB565(fgColor);
    dal_bg_color = RGB888_To_RGB565(bgColor);
    DAL_CHECK_MFC_RET(MFC_SetColor(mfc_handle, dal_fg_color, dal_bg_color));
    DAL_UNLOCK();

    return DAL_STATUS_OK;
}


DAL_STATUS DAL_Clean(void)
{
    const UINT32 BG_COLOR = MAKE_TWO_RGB565_COLOR(DAL_BG_COLOR, DAL_BG_COLOR);
	DAL_STATUS ret = DAL_STATUS_OK;
    UINT32 i, *ptr;
    
    if (NULL == mfc_handle) 
        return DAL_STATUS_NOT_READY;

//    if (LCD_STATE_POWER_OFF == LCD_GetState())
//        return DAL_STATUS_LCD_IN_SUSPEND;

    DAL_LOCK();

    DAL_CHECK_MFC_RET(MFC_ResetCursor(mfc_handle));
 
    ptr = (UINT32 *)dal_fb_addr;
   	for(i = 0; i < DAL_GetLayerSize() / sizeof(UINT32); ++ i) {
       	*ptr ++ = BG_COLOR;
   	}

    if (LCD_STATE_POWER_OFF == LCD_GetState()) {
	    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DAL", "dal_clean in power off\n");
        dal_disable_when_resume = TRUE;
		ret = DAL_STATUS_LCD_IN_SUSPEND;
        goto End;
    }

	DAL_CHECK_LCD_RET(LCD_LayerEnable(ASSERT_LAYER, FALSE));
    dal_shown = FALSE;
#ifdef DAL_LOWMEMORY_ASSERT
   	if (dal_lowMemory_shown) {//only need show lowmemory assert
		UINT32 LOWMEMORY_FG_COLOR = MAKE_TWO_RGB565_COLOR(DAL_LOWMEMORY_FG_COLOR, DAL_LOWMEMORY_FG_COLOR);
		UINT32 LOWMEMORY_BG_COLOR = MAKE_TWO_RGB565_COLOR(DAL_LOWMEMORY_BG_COLOR, DAL_LOWMEMORY_BG_COLOR);

		DAL_CHECK_MFC_RET(MFC_LowMemory_Printf(mfc_handle, low_memory_msg, LOWMEMORY_FG_COLOR, LOWMEMORY_BG_COLOR));
		Show_LowMemory();
   	}
	dal_enable_when_resume_lowmemory = FALSE;
	dal_disable_when_resume_lowmemory = FALSE;
#endif
	dal_disable_when_resume = FALSE;
End:
	DAL_UNLOCK();
    return ret;
}


DAL_STATUS DAL_Printf(const char *fmt, ...)
{
	va_list args;
	uint i;
    DAL_STATUS ret = DAL_STATUS_OK;

    if (NULL == mfc_handle) 
        return DAL_STATUS_NOT_READY;

    if (NULL == fmt)
        return DAL_STATUS_INVALID_ARGUMENT;

    DAL_LOCK();

	va_start (args, fmt);
	i = vsprintf(dal_print_buffer, fmt, args);
        BUG_ON(i>=ARRAY_SIZE(dal_print_buffer));
	va_end (args);

    DAL_CHECK_MFC_RET(MFC_Print(mfc_handle, dal_print_buffer));

    flush_cache_all();

    if (LCD_STATE_POWER_OFF == LCD_GetState()) {
        ret = DAL_STATUS_LCD_IN_SUSPEND;
        dal_enable_when_resume = TRUE;
        goto End;
    }

    if (!dal_shown) {
		DAL_CHECK_LCD_RET(LCD_LayerSetWindowOffset(ASSERT_LAYER,0,0));
		DAL_CHECK_LCD_RET(LCD_LayerSetOffset(ASSERT_LAYER, 0,0));
   		DAL_CHECK_LCD_RET(LCD_LayerSetSize(ASSERT_LAYER,
                                       DAL_WIDTH,
                                       DAL_HEIGHT));
        DAL_CHECK_LCD_RET(LCD_LayerEnable(ASSERT_LAYER, TRUE));
        dal_shown = TRUE;
    }

    DAL_CHECK_DISP_RET(DISP_UpdateScreen(0, 0, 
                                         DAL_WIDTH,
                                         DAL_HEIGHT));
End:
    DAL_UNLOCK();

    return ret;
}


DAL_STATUS DAL_OnDispPowerOn(void)
{
    DAL_LOCK();

    /* Re-enable assertion layer when display resumes */
    
    if (LCD_STATE_POWER_OFF != LCD_GetState()){
		if(dal_enable_when_resume) {
        	dal_enable_when_resume = FALSE;
        	if (!dal_shown) {
				DAL_CHECK_LCD_RET(LCD_LayerSetWindowOffset(ASSERT_LAYER,0,0));
				DAL_CHECK_LCD_RET(LCD_LayerSetOffset(ASSERT_LAYER, 0,0));
   				DAL_CHECK_LCD_RET(LCD_LayerSetSize(ASSERT_LAYER,
                                       DAL_WIDTH,
                                       DAL_HEIGHT));
            	DAL_CHECK_LCD_RET(LCD_LayerEnable(ASSERT_LAYER, TRUE));
            	dal_shown = TRUE;
        	}
#ifdef DAL_LOWMEMORY_ASSERT
			dal_enable_when_resume_lowmemory = FALSE;
			dal_disable_when_resume_lowmemory = FALSE;
#endif
			goto End;
    	}
		else if(dal_disable_when_resume){
			dal_disable_when_resume = FALSE;
   			DAL_CHECK_LCD_RET(LCD_LayerEnable(ASSERT_LAYER, FALSE));
			dal_shown = FALSE;
#ifdef DAL_LOWMEMORY_ASSERT
        	if (dal_lowMemory_shown) {//only need show lowmemory assert	
				UINT32 LOWMEMORY_FG_COLOR = MAKE_TWO_RGB565_COLOR(DAL_LOWMEMORY_FG_COLOR, DAL_LOWMEMORY_FG_COLOR);
				UINT32 LOWMEMORY_BG_COLOR = MAKE_TWO_RGB565_COLOR(DAL_LOWMEMORY_BG_COLOR, DAL_LOWMEMORY_BG_COLOR);

				DAL_CHECK_MFC_RET(MFC_LowMemory_Printf(mfc_handle, low_memory_msg, LOWMEMORY_FG_COLOR, LOWMEMORY_BG_COLOR));
				Show_LowMemory();			
        	}
			dal_enable_when_resume_lowmemory = FALSE;
			dal_disable_when_resume_lowmemory = FALSE;
#endif
			goto End;

		}
#ifdef DAL_LOWMEMORY_ASSERT
		if(dal_enable_when_resume_lowmemory){
			dal_enable_when_resume_lowmemory = FALSE;
			if(!dal_shown){//only need show lowmemory assert
				Show_LowMemory();
			}
		}
		else if(dal_disable_when_resume_lowmemory){
			if(!dal_shown){// only low memory assert shown on screen
				DAL_CHECK_LCD_RET(LCD_LayerEnable(ASSERT_LAYER, FALSE));
				DAL_CHECK_LCD_RET(LCD_LayerSetOffset(ASSERT_LAYER, 0,0));
				DAL_CHECK_LCD_RET(LCD_LayerSetWindowOffset(ASSERT_LAYER, 0,0));
   				DAL_CHECK_LCD_RET(LCD_LayerSetSize(ASSERT_LAYER,
                                       DAL_WIDTH,
                                       DAL_HEIGHT));
			}
		}
		else{}
#endif
	}

End:
    DAL_UNLOCK();

    return DAL_STATUS_OK;
}

#ifdef DAL_LOWMEMORY_ASSERT
DAL_STATUS DAL_LowMemoryOn(void)
{
	UINT32 LOWMEMORY_FG_COLOR = MAKE_TWO_RGB565_COLOR(DAL_LOWMEMORY_FG_COLOR, DAL_LOWMEMORY_FG_COLOR);
	UINT32 LOWMEMORY_BG_COLOR = MAKE_TWO_RGB565_COLOR(DAL_LOWMEMORY_BG_COLOR, DAL_LOWMEMORY_BG_COLOR);
	DAL_LOG("Enter DAL_LowMemoryOn()\n");

	DAL_CHECK_MFC_RET(MFC_LowMemory_Printf(mfc_handle, low_memory_msg, LOWMEMORY_FG_COLOR, LOWMEMORY_BG_COLOR));
	
	if (LCD_STATE_POWER_OFF == LCD_GetState()) {
        dal_enable_when_resume_lowmemory = TRUE;
		DAL_LOG("LCD is off\n");
        goto End;
    }

	Show_LowMemory();
//	dal_lowMemory_shown = TRUE;
End:
	dal_lowMemory_shown = TRUE;
	DAL_LOG("Leave DAL_LowMemoryOn()\n");
	return DAL_STATUS_OK;
}

DAL_STATUS DAL_LowMemoryOff(void)
{
	UINT32 BG_COLOR = MAKE_TWO_RGB565_COLOR(DAL_BG_COLOR, DAL_BG_COLOR);
	DAL_LOG("Enter DAL_LowMemoryOff()\n");

	DAL_CHECK_MFC_RET(MFC_SetMem(mfc_handle, low_memory_msg, BG_COLOR));
	
	if (LCD_STATE_POWER_OFF == LCD_GetState()) {
        dal_disable_when_resume_lowmemory = TRUE;
        goto End;
    }
//what about LCM_PHYSICAL_ROTATION = 180
	if(!dal_shown){// only low memory assert shown on screen
		DAL_CHECK_LCD_RET(LCD_LayerEnable(ASSERT_LAYER, FALSE));
		DAL_CHECK_LCD_RET(LCD_LayerSetOffset(ASSERT_LAYER, 0,0));
		DAL_CHECK_LCD_RET(LCD_LayerSetWindowOffset(ASSERT_LAYER, 0,0));
   		DAL_CHECK_LCD_RET(LCD_LayerSetSize(ASSERT_LAYER,
                                       DAL_WIDTH,
                                       DAL_HEIGHT));

	}
/*
	DAL_CHECK_DISP_RET(DISP_UpdateScreen(0, 0, 
                                         DAL_WIDTH,
                                         DAL_HEIGHT));
*/
End:			
	dal_lowMemory_shown = FALSE;
	DAL_LOG("Leave DAL_LowMemoryOff()\n");
    return DAL_STATUS_OK;
}
#endif
// ##########################################################################
//  !CONFIG_MTK_FB_SUPPORT_ASSERTION_LAYER
// ##########################################################################
#else

UINT32 DAL_GetLayerSize(void)
{
	// xuecheng, avoid lcdc read buffersize+1 issue
    return DAL_WIDTH * DAL_HEIGHT * DAL_BPP + 4096;
}

DAL_STATUS DAL_Init(UINT32 layerVA, UINT32 layerPA)
{
    NOT_REFERENCED(layerVA);
    NOT_REFERENCED(layerPA);
    
    return DAL_STATUS_OK;
}
DAL_STATUS DAL_SetColor(unsigned int fgColor, unsigned int bgColor)
{
    NOT_REFERENCED(fgColor);
    NOT_REFERENCED(bgColor);

    return DAL_STATUS_OK;
}
DAL_STATUS DAL_Clean(void)
{
    return DAL_STATUS_OK;
}
DAL_STATUS DAL_Printf(const char *fmt, ...)
{
    NOT_REFERENCED(fmt);
    return DAL_STATUS_OK;
}
DAL_STATUS DAL_OnDispPowerOn(void)
{
    return DAL_STATUS_OK;
}
#ifdef DAL_LOWMEMORY_ASSERT
DAL_STATUS DAL_LowMemoryOn(void)
{
    return DAL_STATUS_OK;
}
DAL_STATUS DAL_LowMemoryOff(void)
{
    return DAL_STATUS_OK;
}
#endif
#endif  // CONFIG_MTK_FB_SUPPORT_ASSERTION_LAYER

EXPORT_SYMBOL(DAL_SetColor);
EXPORT_SYMBOL(DAL_Printf);
EXPORT_SYMBOL(DAL_Clean);
#ifdef DAL_LOWMEMORY_ASSERT
EXPORT_SYMBOL(DAL_LowMemoryOn);
EXPORT_SYMBOL(DAL_LowMemoryOff);
#endif
