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

#ifndef __LCD_DRV_H__
#define __LCD_DRV_H__

#include "disp_drv.h"
#include "lcm_drv.h"
#ifdef __cplusplus
extern "C" {
#endif


// ---------------------------------------------------------------------------

#define LCD_CHECK_RET(expr)             \
    do {                                \
        LCD_STATUS ret = (expr);        \
        ASSERT(LCD_STATUS_OK == ret);   \
    } while (0)

// ---------------------------------------------------------------------------

typedef unsigned int S2_8;

// ---------------------------------------------------------------------------

typedef enum
{	
    LCD_STATUS_OK = 0,

    LCD_STATUS_ERROR,
} LCD_STATUS;


typedef enum
{	
    LCD_STATE_IDLE = 0,
    LCD_STATE_BUSY,
    LCD_STATE_POWER_OFF,
} LCD_STATE;


typedef enum
{
    LCD_IF_PARALLEL_0 = 0,
    LCD_IF_PARALLEL_1 = 1,
    LCD_IF_PARALLEL_2 = 2,
    LCD_IF_SERIAL_0   = 3,
    LCD_IF_SERIAL_1   = 4,

    LCD_IF_ALL = 0xFF,
} LCD_IF_ID;


typedef enum
{
    LCD_IF_PARALLEL_8BITS  = 0,
    LCD_IF_PARALLEL_9BITS  = 1,
    LCD_IF_PARALLEL_16BITS = 2,
    LCD_IF_PARALLEL_18BITS = 3,
    LCD_IF_PARALLEL_24BITS = 4,
} LCD_IF_PARALLEL_BITS;


typedef enum
{
    LCD_IF_SERIAL_8BITS = 0,
    LCD_IF_SERIAL_9BITS = 1,
    LCD_IF_SERIAL_16BITS = 2,
    LCD_IF_SERIAL_18BITS = 3,
    LCD_IF_SERIAL_24BITS = 4,
    LCD_IF_SERIAL_32BITS = 5,
} LCD_IF_SERIAL_BITS;


typedef enum
{
    LCD_IF_PARALLEL_CLK_DIV_1 = 0,
    LCD_IF_PARALLEL_CLK_DIV_2,
    LCD_IF_PARALLEL_CLK_DIV_4,
} LCD_IF_PARALLEL_CLK_DIV;

/*
typedef enum
{
    LCD_IF_SERIAL_CLK_DIV_1 = 0,
    LCD_IF_SERIAL_CLK_DIV_2,
    LCD_IF_SERIAL_CLK_DIV_4,
    LCD_IF_SERIAL_CLK_DIV_8,
    LCD_IF_SERIAL_CLK_DIV_16,
} LCD_IF_SERIAL_CLK_DIV;
*/

typedef enum
{
    LCD_IF_A0_LOW  = 0,
    LCD_IF_A0_HIGH = 1,
} LCD_IF_A0_MODE;


typedef enum
{
    LCD_IF_MCU_WRITE_8BIT  = 8,
    LCD_IF_MCU_WRITE_16BIT = 16,
    LCD_IF_MCU_WRITE_32BIT = 32,
} LCD_IF_MCU_WRITE_BITS;


typedef enum
{
    LCD_IF_FORMAT_RGB332 = 0,
    LCD_IF_FORMAT_RGB444 = 1,
    LCD_IF_FORMAT_RGB565 = 2,
    LCD_IF_FORMAT_RGB666 = 3,
    LCD_IF_FORMAT_RGB888 = 4,
} LCD_IF_FORMAT;


typedef enum
{
    LCD_IF_WIDTH_8_BITS  = 0,
    LCD_IF_WIDTH_9_BITS  = 2,
    LCD_IF_WIDTH_16_BITS = 1,
    LCD_IF_WIDTH_18_BITS = 3,
    LCD_IF_WIDTH_24_BITS = 4,
	LCD_IF_WIDTH_32_BITS = 5,
} LCD_IF_WIDTH;

/*
typedef enum
{
    LCD_CMDQ_0 = 0,
    LCD_CMDQ_1 = 1,
} LCD_CMDQ_ID;
*/

typedef enum
{
    LCD_FB_0 = 0,
    LCD_FB_1 = 1,
    LCD_FB_2 = 2,
    LCD_FB_NUM,
} LCD_FB_ID;


typedef enum
{
    LCD_FB_FORMAT_RGB565   = 0,
    LCD_FB_FORMAT_RGB888   = 1,
    LCD_FB_FORMAT_ARGB8888 = 2,
    LCD_FB_FORMAT_NUM,
} LCD_FB_FORMAT;


typedef enum
{
    LCD_OUTPUT_TO_LCM   = (1 << 0),
    LCD_OUTPUT_TO_MEM   = (1 << 1),
    LCD_OUTPUT_TO_TVROT = (1 << 2),
} LCD_OUTPUT_MODE;


typedef enum {
   LCD_LAYER_0 = 0,
   LCD_LAYER_1 = 1,
   LCD_LAYER_2 = 2,
   LCD_LAYER_3 = 3,
   LCD_LAYER_NUM,
   LCD_LAYER_ALL = 0xFFFFFFFF,
} LCD_LAYER_ID;

/* Layer Designation */
#define ASSERT_LAYER    (LCD_LAYER_3)
#define FB_LAYER        (LCD_LAYER_3)

typedef enum {
    LCD_LAYER_FORMAT_RGB888     = 0,
    LCD_LAYER_FORMAT_RGB565     = 1,
    LCD_LAYER_FORMAT_ARGB8888   = 2,
    LCD_LAYER_FORMAT_PARGB8888  = 3,
    LCD_LAYER_FORMAT_xRGB8888  = 4,
    LCD_LAYER_FORMAT_YUYV422    = 8,
    LCD_LAYER_FORMAT_UYVY       = 9,
    LCD_LAYER_FORMAT_YVYU       = 10,
    LCD_LAYER_FORMAT_VYUY       = 11,
    LCD_LAYER_FORMAT_YUV444     = 15,
    LCD_LAYER_FORMAT_NUM,
} LCD_LAYER_FORMAT;


typedef enum {
    LCD_LAYER_ROTATE_NONE       = 0,
    LCD_LAYER_ROTATE_0          = 0,
    LCD_LAYER_ROTATE_90         = 1,
    LCD_LAYER_ROTATE_180        = 2,
    LCD_LAYER_ROTATE_270        = 3,
    LCD_LAYER_ROTATE_MIRROR_0   = 4,
    LCD_LAYER_ROTATE_MIRROR_90  = 5,
    LCD_LAYER_ROTATE_MIRROR_180 = 6,
    LCD_LAYER_ROTATE_MIRROR_270 = 7,
} LCD_LAYER_ROTATION;


typedef enum {
    LCD_SW_TRIGGER = 0,
    LCD_HW_TRIGGER_BUFFERING,
    LCD_HW_TRIGGER_DIRECT_COUPLE,
} LCD_LAYER_TRIGGER_MODE;


typedef enum {
    LCD_HW_TRIGGER_SRC_IRT1 = 0,
    LCD_HW_TRIGGER_SRC_IBW1 = 1,
    LCD_HW_TRIGGER_SRC_IBW2 = 3,
} LCD_HW_TRIGGER_SRC;


typedef enum {
    LCD_TE_MODE_VSYNC_ONLY     = 0,
    LCD_TE_MODE_VSYNC_OR_HSYNC = 1,
} LCD_TE_MODE;


typedef enum
{
    LCD_TE_VS_WIDTH_CNT_DIV_8  = 0,
    LCD_TE_VS_WIDTH_CNT_DIV_16 = 1,
    LCD_TE_VS_WIDTH_CNT_DIV_32 = 2,
    LCD_TE_VS_WIDTH_CNT_DIV_64 = 3,
} LCD_TE_VS_WIDTH_CNT_DIV;


typedef enum
{
    LCD_IF_FMT_COLOR_ORDER_RGB = 0,
    LCD_IF_FMT_COLOR_ORDER_BGR = 1,
} LCD_IF_FMT_COLOR_ORDER;


typedef enum
{
    LCD_IF_FMT_TRANS_SEQ_MSB_FIRST = 0,
    LCD_IF_FMT_TRANS_SEQ_LSB_FIRST = 1,
} LCD_IF_FMT_TRANS_SEQ;


typedef enum
{
    LCD_IF_FMT_PADDING_ON_LSB = 0,
    LCD_IF_FMT_PADDING_ON_MSB = 1,
} LCD_IF_FMT_PADDING;


typedef enum
{
    LCD_DSI_IF_FMT_COLOR_ORDER_RGB = 0,
    LCD_DSI_IF_FMT_COLOR_ORDER_BGR = 1,
} LCD_DSI_IF_FMT_COLOR_ORDER;


typedef enum
{
    LCD_DSI_IF_FMT_TRANS_SEQ_MSB_FIRST = 0,
    LCD_DSI_IF_FMT_TRANS_SEQ_LSB_FIRST = 1,
} LCD_DSI_IF_FMT_TRANS_SEQ;


typedef enum
{
    LCD_DSI_IF_FMT_PADDING_ON_LSB = 0,
    LCD_DSI_IF_FMT_PADDING_ON_MSB = 1,
} LCD_DSI_IF_FMT_PADDING;


typedef enum
{
    LCD_DSI_IF_FORMAT_RGB565 = 0,
    LCD_DSI_IF_FORMAT_RGB666 = 1,
    LCD_DSI_IF_FORMAT_RGB888 = 2,
} LCD_DSI_IF_FORMAT;


// ---------------------------------------------------------------------------

// Configurations
LCD_STATUS LCD_Init(void);
LCD_STATUS LCD_Deinit(void);

LCD_STATUS LCD_Set_DrivingCurrent(LCM_PARAMS *lcm_params);
LCD_STATUS LCD_PowerOn(void);
LCD_STATUS LCD_PowerOff(void);
LCD_STATUS LCD_M4UPowerOn(void);
LCD_STATUS LCD_M4UPowerOff(void);
LCD_STATUS LCD_WaitForNotBusy(void);


LCD_STATUS LCD_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS eventID));

// LCD Controller Interface
LCD_STATUS LCD_ConfigParallelIF(LCD_IF_ID id,
                                LCD_IF_PARALLEL_BITS ifDataWidth,
                                LCD_IF_PARALLEL_CLK_DIV clkDivisor,
                                UINT32 writeSetup,
                                UINT32 writeHold,
                                UINT32 writeWait,
                                UINT32 readSetup,
								UINT32 readHold,
                                UINT32 readLatency,
                                UINT32 waitPeriod,
								UINT32 chw);

LCD_STATUS LCD_ConfigSerialIF(LCD_IF_ID id,
                              LCD_IF_SERIAL_BITS bits,
                              UINT32 three_wire,
                              UINT32 sdi,
                              BOOL   first_pol,
                              BOOL   sck_def,
                              UINT32 div2,
                              UINT32 hw_cs,
                              UINT32 css,
                              UINT32 csh,
                              UINT32 rd_1st,
                              UINT32 rd_2nd,
                              UINT32 wr_1st,
                              UINT32 wr_2nd);


LCD_STATUS LCD_ConfigIfFormat(LCD_IF_FMT_COLOR_ORDER order,
                              LCD_IF_FMT_TRANS_SEQ transSeq,
                              LCD_IF_FMT_PADDING padding,
                              LCD_IF_FORMAT format,
                              LCD_IF_WIDTH busWidth);

LCD_STATUS LCD_SetResetSignal(BOOL high);
LCD_STATUS LCD_ConfigDSIIfFormat(LCD_DSI_IF_FMT_COLOR_ORDER order,
                              LCD_DSI_IF_FMT_TRANS_SEQ transSeq,
                              LCD_DSI_IF_FMT_PADDING padding,
                              LCD_DSI_IF_FORMAT format,
                              UINT32 packet_size,
                              BOOL DC_DSI);

// LCD Command Queue
LCD_STATUS LCD_CmdQueueEnable(BOOL enabled);
LCD_STATUS LCD_CmdQueueWrite(UINT32 *cmds, UINT32 cmdCount);

// Layer Configurations
LCD_STATUS LCD_LayerEnable(LCD_LAYER_ID id, BOOL enable);
BOOL LCD_IsLayerEnable(LCD_LAYER_ID id);
LCD_STATUS LCD_LayerSetAddress(LCD_LAYER_ID id, UINT32 address);
UINT32 LCD_LayerGetAddress(LCD_LAYER_ID id);
LCD_STATUS LCD_LayerSetSize(LCD_LAYER_ID id, UINT32 width, UINT32 height);

// 3D Layer Configurations
LCD_STATUS LCD_Layer3D_Enable(LCD_LAYER_ID id, BOOL enable);
LCD_STATUS LCD_Layer3D_R1st(LCD_LAYER_ID id, BOOL r_first);
LCD_STATUS LCD_Layer3D_landscape(LCD_LAYER_ID id, BOOL landscape);
LCD_STATUS LCD_LayerSet3D(LCD_LAYER_ID id, BOOL enable, BOOL r_first, BOOL landscape);
BOOL LCD_Is3DEnabled(void);
BOOL LCD_Is3DLandscapeMode(void);


// MT6573 added
LCD_STATUS LCD_LayerSetPitch(LCD_LAYER_ID id, UINT32 pitch);
LCD_STATUS LCD_LayerSetOffset(LCD_LAYER_ID id, UINT32 x, UINT32 y);
LCD_STATUS LCD_LayerSetWindowOffset(LCD_LAYER_ID id, UINT32 x, UINT32 y);
LCD_STATUS LCD_LayerSetFormat(LCD_LAYER_ID id, LCD_LAYER_FORMAT format);
LCD_STATUS LCD_LayerEnableByteSwap(LCD_LAYER_ID id, BOOL enable);
LCD_STATUS LCD_LayerSetRotation(LCD_LAYER_ID id, LCD_LAYER_ROTATION rotation);
LCD_STATUS LCD_LayerSetAlphaBlending(LCD_LAYER_ID id, BOOL enable, UINT8 alpha);
LCD_STATUS LCD_LayerSetSourceColorKey(LCD_LAYER_ID id, BOOL enable, UINT32 colorKey);

// HW Trigger Configurations
LCD_STATUS LCD_LayerSetTriggerMode(LCD_LAYER_ID id, LCD_LAYER_TRIGGER_MODE mode);
LCD_STATUS LCD_LayerSetHwTriggerSrc(LCD_LAYER_ID id, LCD_HW_TRIGGER_SRC src);
LCD_STATUS LCD_EnableHwTrigger(BOOL enable);

// ROI Window Configurations
LCD_STATUS LCD_SetBackgroundColor(UINT32 bgColor);
LCD_STATUS LCD_SetRoiWindow(UINT32 x, UINT32 y, UINT32 width, UINT32 height);

// DSI Related Configurations
LCD_STATUS LCD_EnableDCtoDsi(BOOL dc_dsi_en);

// Output to Memory Configurations
LCD_STATUS LCD_SetOutputMode(LCD_OUTPUT_MODE mode);
LCD_STATUS LCD_WaitDPIIndication(BOOL enable);
LCD_STATUS LCD_FBSetFormat(LCD_FB_FORMAT format);
LCD_STATUS LCD_FBSetPitch(UINT32 pitchInByte);
LCD_STATUS LCD_FBReset(void);
LCD_STATUS LCD_FBEnable(LCD_FB_ID id, BOOL enable);
LCD_STATUS LCD_FBSetAddress(LCD_FB_ID id, UINT32 address);
LCD_STATUS LCD_FBSetStartCoord(UINT32 x, UINT32 y);

// Color Matrix
LCD_STATUS LCD_EnableColorMatrix(LCD_IF_ID id, BOOL enable);
LCD_STATUS LCD_SetColorMatrix(const S2_8 mat[9]);

// Tearing Control
LCD_STATUS LCD_TE_Enable(BOOL enable);
LCD_STATUS LCD_TE_SetMode(LCD_TE_MODE mode);
LCD_STATUS LCD_TE_SetEdgePolarity(BOOL polarity);
LCD_STATUS LCD_TE_ConfigVHSyncMode(UINT32 hsDelayCnt,
                                   UINT32 vsWidthCnt,
                                   LCD_TE_VS_WIDTH_CNT_DIV vsWidthCntDiv);

// Operations
LCD_STATUS LCD_SelectWriteIF(LCD_IF_ID id);
LCD_STATUS LCD_WriteIF(LCD_IF_ID id, LCD_IF_A0_MODE a0, UINT32 value, LCD_IF_MCU_WRITE_BITS bits);
LCD_STATUS LCD_ReadIF(LCD_IF_ID id, LCD_IF_A0_MODE a0, UINT32 *value, LCD_IF_MCU_WRITE_BITS bits);
LCD_STATUS LCD_StartTransfer(BOOL blocking);

// Retrieve Information
LCD_STATE LCD_GetState(void);
LCD_OUTPUT_MODE LCD_GetOutputMode(void);

LCD_STATUS LCD_ConfigOVL(void);

BOOL LCD_IsBusy(void);
// Debug
LCD_STATUS LCD_ConfigDither(int lrs, int lgs, int lbs, int dbr, int dbg, int dbb);
LCD_STATUS LCD_LayerEnableDither(LCD_LAYER_ID id, UINT32 enable);
LCD_STATUS LCD_SetOutputAlpha(unsigned int alpha);
LCD_STATUS LCD_DumpRegisters(void);

LCD_STATUS LCD_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp);
LCD_STATUS LCD_Capture_Layerbuffer(unsigned int layer_id, unsigned int pvbuf, unsigned int bpp);
LCD_STATUS LCD_Init_IO_pad(LCM_PARAMS *lcm_params);

//FM De-sense
LCD_STATUS LCD_FMDesense_Query(void);
LCD_STATUS LCD_FM_Desense(LCD_IF_ID id, unsigned long freq);
LCD_STATUS LCD_Get_Default_WriteCycle(LCD_IF_ID id, unsigned int *write_cycle);
LCD_STATUS LCD_Get_Current_WriteCycle(LCD_IF_ID id, unsigned int *write_cycle);
LCD_STATUS LCD_Change_WriteCycle(LCD_IF_ID id, unsigned int write_cycle);
LCD_STATUS LCD_Reset_WriteCycle(LCD_IF_ID id);

LCD_STATUS LCD_Get_VideoLayerSize(unsigned int id, unsigned int *width, unsigned int *height);
LCD_STATUS LCD_Capture_Videobuffer(unsigned int pvbuf, unsigned int bpp, unsigned int video_rotation);

LCD_STATUS LCD_InitM4U(void);
LCD_STATUS LCD_AllocUIMva(unsigned int va, unsigned int *mva, unsigned int size);
LCD_STATUS LCD_AllocOverlayMva(unsigned int va, unsigned int *mva, unsigned int size);
LCD_STATUS LCD_DeallocMva(unsigned int va, unsigned int mva, unsigned int size);

LCD_STATUS LCD_W2M_NeedLimiteSpeed(BOOL enable);
LCD_STATUS LCD_W2TVR_NeedLimiteSpeed(BOOL enable);
LCD_STATUS LCD_SetGMCThrottle(void);


LCD_STATUS LCD_DumpM4U(void);
LCD_STATUS LCD_M4U_On(BOOL enable);

BOOL LCD_esd_check(void);
void LCD_WaitTE(void);
void LCD_InitVSYNC(unsigned int vsync_interval);
void LCD_PauseVSYNC(BOOL enable);
void LCD_GetVsyncCnt(void);
// ---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif


#endif // __LCD_DRV_H__
