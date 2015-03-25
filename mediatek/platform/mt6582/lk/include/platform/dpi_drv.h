/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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

#ifndef __DPI_DRV_H__
#define __DPI_DRV_H__

#include "disp_drv.h"
#include "lcm_drv.h"
#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------

#define DPI_CHECK_RET(expr)             \
    do {                                \
        DPI_STATUS ret = (expr);        \
        ASSERT(DPI_STATUS_OK == ret);   \
    } while (0)

// ---------------------------------------------------------------------------

typedef enum
{	
   DPI_STATUS_OK = 0,

   DPI_STATUS_ERROR,
} DPI_STATUS;


typedef enum
{
    DPI_FB_FORMAT_RGB565 = 0,
    DPI_FB_FORMAT_RGB888 = 1,
    DPI_FB_FORMAT_XRGB888 = 1,
    DPI_FB_FORMAT_RGBX888 = 1,
    DPI_FB_FORMAT_NUM,
} DPI_FB_FORMAT;
        

typedef enum
{
    DPI_RGB_ORDER_RGB = 0,
    DPI_RGB_ORDER_BGR = 1,
} DPI_RGB_ORDER;


typedef enum
{
    DPI_FB_0   = 0,
    DPI_FB_1   = 1,
    DPI_FB_2   = 2,
    DPI_FB_NUM,
} DPI_FB_ID;


typedef enum
{
    DPI_POLARITY_RISING  = 0,
    DPI_POLARITY_FALLING = 1
} DPI_POLARITY;

// ---------------------------------------------------------------------------

DPI_STATUS DPI_Init(BOOL isDpiPoweredOn);
DPI_STATUS DPI_Deinit(void);

DPI_STATUS DPI_Init_PLL(unsigned int mipi_pll_clk_ref,unsigned int mipi_pll_clk_div1,unsigned int mipi_pll_clk_div2);
DPI_STATUS DPI_Set_DrivingCurrent(LCM_PARAMS *lcm_params);

DPI_STATUS DPI_PowerOn(void);
DPI_STATUS DPI_PowerOff(void);

DPI_STATUS DPI_EnableClk(void);
DPI_STATUS DPI_DisableClk(void);

DPI_STATUS DPI_EnableSeqOutput(BOOL enable);
DPI_STATUS DPI_SetRGBOrder(DPI_RGB_ORDER input, DPI_RGB_ORDER output);

DPI_STATUS DPI_ConfigPixelClk(DPI_POLARITY polarity, UINT32 divisor, UINT32 duty);
DPI_STATUS DPI_ConfigDataEnable(DPI_POLARITY polarity);
DPI_STATUS DPI_ConfigVsync(DPI_POLARITY polarity,
                           UINT32 pulseWidth, UINT32 backPorch, UINT32 frontPorch);
DPI_STATUS DPI_ConfigHsync(DPI_POLARITY polarity,
                           UINT32 pulseWidth, UINT32 backPorch, UINT32 frontPorch);

DPI_STATUS DPI_FBSyncFlipWithLCD(BOOL enable);
DPI_STATUS DPI_SetDSIMode(BOOL enable);
BOOL 	   DPI_IsDSIMode(void);
DPI_STATUS DPI_FBSetFormat(DPI_FB_FORMAT format);
DPI_FB_FORMAT DPI_FBGetFormat(void);
DPI_STATUS DPI_FBSetSize(UINT32 width, UINT32 height);
DPI_STATUS DPI_FBEnable(DPI_FB_ID id, BOOL enable);
DPI_STATUS DPI_FBSetAddress(DPI_FB_ID id, UINT32 address);
DPI_STATUS DPI_FBSetPitch(DPI_FB_ID id, UINT32 pitchInByte);

DPI_STATUS DPI_SetFifoThreshold(UINT32 low, UINT32 high);

// Debug
DPI_STATUS DPI_DumpRegisters(void);

DPI_STATUS DPI_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp);

//FM De-sense
DPI_STATUS DPI_FMDesense_Query(void);
DPI_STATUS DPI_FM_Desense(unsigned long freq);
DPI_STATUS DPI_Get_Default_CLK(unsigned int *clk);
DPI_STATUS DPI_Get_Current_CLK(unsigned int *clk);
DPI_STATUS DPI_Change_CLK(unsigned int clk);
DPI_STATUS DPI_Reset_CLK(void);

void DPI_mipi_switch(BOOL on);
void DPI_DisableIrq(void);
void DPI_EnableIrq(void);
DPI_STATUS DPI_FreeIRQ(void);

DPI_STATUS DPI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID);
DPI_STATUS DPI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS eventID));
void DPI_WaitVSYNC(void);
void DPI_InitVSYNC(unsigned int vsync_interval);
void DPI_PauseVSYNC(BOOL enable);

DPI_STATUS DPI_ConfigLVDS(LCM_PARAMS *lcm_params);
DPI_STATUS DPI_WaitVsync(void);
// ---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __DPI_DRV_H__
