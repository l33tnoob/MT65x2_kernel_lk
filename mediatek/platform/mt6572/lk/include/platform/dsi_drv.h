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

#ifndef __DSI_DRV_H__
#define __DSI_DRV_H__

#include <platform/mt_typedefs.h>

#include "lcm_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------

#define DSI_CHECK_RET(expr)             \
    do {                                \
        DSI_STATUS ret = (expr);        \
        ASSERT(DSI_STATUS_OK == ret);   \
    } while (0)

// ---------------------------------------------------------------------------

#define		DSI_DCS_SHORT_PACKET_ID_0			0x05
#define		DSI_DCS_SHORT_PACKET_ID_1			0x15
#define		DSI_DCS_LONG_PACKET_ID				0x39
#define		DSI_DCS_READ_PACKET_ID				0x06
		
#define		DSI_GERNERIC_SHORT_PACKET_ID_1		0x13
#define		DSI_GERNERIC_SHORT_PACKET_ID_2		0x23
#define		DSI_GERNERIC_LONG_PACKET_ID			0x29
#define		DSI_GERNERIC_READ_LONG_PACKET_ID	0x14


#define		DSI_WMEM_CONTI						(0x3C)
#define		DSI_RMEM_CONTI						(0x3E)

// ESD recovery method for video mode LCM
#define		METHOD_NONCONTINUOUS_CLK			(0x1)
#define		METHOD_BUS_TURN_AROUND				(0x2)

// State of DSI engine
#define		DSI_VDO_VSA_VS_STATE				(0x008)
#define		DSI_VDO_VSA_HS_STATE				(0x010)
#define		DSI_VDO_VSA_VE_STATE				(0x020)
#define		DSI_VDO_VBP_STATE					(0x040)
#define		DSI_VDO_VACT_STATE					(0x080)
#define		DSI_VDO_VFP_STATE					(0x100)

// Common control of DSI engine
#define		DSI_COM_CON_DSI_EN						(0x2)

// ---------------------------------------------------------------------------

typedef enum
{	
    DSI_STATUS_OK = 0,
    DSI_STATUS_ERROR,
} DSI_STATUS;


typedef enum
{
    SHORT_PACKET_RW = 0,
    FB_WRITE 		= 1,
    LONG_PACKET_W	= 2,
    FB_READ 		= 3,
} DSI_INS_TYPE;


typedef enum
{
    DISABLE_BTA = 0,
    ENABLE_BTA 	= 1,
} DSI_CMDQ_BTA;


typedef enum
{
    LOW_POWER 	= 0,
    HIGH_SPEED 	= 1,
} DSI_CMDQ_HS;


typedef enum
{
    CL_8BITS 	= 0,
    CL_16BITS 	= 1,
} DSI_CMDQ_CL;


typedef enum
{
    DISABLE_TE 	= 0,
    ENABLE_TE	= 1,
} DSI_CMDQ_TE;


typedef enum
{
    DISABLE_RPT = 0,
    ENABLE_RPT	= 1,
} DSI_CMDQ_RPT;


typedef struct
{
    unsigned		type	: 2;
    unsigned		BTA		: 1;
    unsigned	 	HS		: 1;
    unsigned		CL		: 1;
    unsigned		TE		: 1;
    unsigned 		Rsv		: 1;
    unsigned		RPT		: 1;
} DSI_CMDQ_CONFG, *PDSI_CMDQ_CONFIG;


typedef struct
{
    unsigned CONFG			: 8;
    unsigned Data_ID		: 8;
    unsigned Data0			: 8;
    unsigned Data1			: 8;
} DSI_T0_INS, *PDSI_T0_INS;

typedef struct
{
    unsigned CONFG			: 8;
    unsigned Data_ID		: 8;
    unsigned mem_start0		: 8;
    unsigned mem_start1		: 8;
} DSI_T1_INS, *PDSI_T1_INS;

typedef struct
{
    unsigned CONFG			: 8;
    unsigned Data_ID		: 8;
    unsigned WC16			: 16;
    unsigned int *pdata;
} DSI_T2_INS, *PDSI_T2_INS;

typedef struct
{
    unsigned CONFG			: 8;
    unsigned Data_ID		: 8;
    unsigned mem_start0		: 8;
    unsigned mem_start1		: 8;
} DSI_T3_INS, *PDSI_T3_INS;

typedef struct
{
    UINT16 CLK;
    UINT8  TXDIV0;
    UINT8  TXDIV1;
    UINT32 SDM_PCW;
    UINT8  SSC_PH_INIT;
    UINT16 SSC_PRD;
    UINT16 SSC_DELTA1;
    UINT16 SSC_DELTA;
}DSI_PLL_CONFIG;

DSI_STATUS DSI_Init(BOOL isDsiPoweredOn);
DSI_STATUS DSI_Deinit(void);

DSI_STATUS DSI_PowerOn(void);
DSI_STATUS DSI_PowerOff(void);

DSI_STATUS DSI_WaitForNotBusy(void);

DSI_STATUS DSI_StartTransfer(BOOL needStartDSI);
DSI_STATUS DSI_Detect_CLK_Glitch(void);

DSI_STATUS DSI_EnableClk(void);
DSI_STATUS DSI_DisableClk(void);
DSI_STATUS DSI_Reset(void);
DSI_STATUS DSI_LP_Reset(void);
DSI_STATUS DSI_SetMode(unsigned int mode);

DSI_STATUS DSI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID);
DSI_STATUS DSI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS eventID));

DSI_STATUS DSI_handle_TE(void);


DSI_STATUS DSI_Write_T0_INS(DSI_T0_INS *t0);
DSI_STATUS DSI_Write_T1_INS(DSI_T1_INS *t1);
DSI_STATUS DSI_Write_T2_INS(DSI_T2_INS *t2);
DSI_STATUS DSI_Write_T3_INS(DSI_T3_INS *t3);

DSI_STATUS DSI_TXRX_Control(BOOL cksm_en, 
                                  BOOL ecc_en, 
                                  unsigned char lane_num, 
                                  unsigned char vc_num,
                                  BOOL null_packet_en,
                                  BOOL err_correction_en,
                                  BOOL dis_eotp_en,
                                  unsigned int  max_return_size);
//#ifndef MT65XX_NEW_DISP
#if 0
DSI_STATUS DSI_PS_Control(unsigned int ps_type, unsigned int ps_wc);
#else
DSI_STATUS DSI_PS_Control(unsigned int ps_type, unsigned int vact_line, unsigned int ps_wc);
#endif


//void init_mipi_pll(void);
void DSI_Config_VDO_Timing(LCM_PARAMS *lcm_params);

//#ifndef MT65XX_NEW_DISP
void DSI_PHY_clk_setting(LCM_PARAMS *lcm_params);
//void DSI_PHY_clk_setting(unsigned int div1, unsigned int div2, unsigned int lane_no);
//#endif
void DSI_PHY_clk_switch(BOOL on, LCM_PARAMS *lcm_params);

void DSI_PHY_TIMCONFIG(LCM_PARAMS *lcm_params);

void DSI_clk_ULP_mode(BOOL enter);
void DSI_clk_HS_mode(BOOL enter);
BOOL DSI_clk_HS_state(void);
void DSI_lane0_ULP_mode(BOOL enter);

BOOL DSI_esd_check(void);
void DSI_handle_esd_recovery(void);
void DSI_set_int_TE(BOOL enable, unsigned int period);
BOOL DSI_handle_int_TE(void);
void DSI_set_noncont_clk(BOOL enable, unsigned int period);
void DSI_handle_noncont_clk(void);
void DSI_set_cmdq_V3(LCM_setting_table_V3 *para_tbl, unsigned int size, unsigned char force_update);
void DSI_set_cmdq_V2(unsigned cmd, unsigned char count, unsigned char *para_list, unsigned char force_update);
void DSI_set_cmdq(unsigned int *pdata, unsigned int queue_size, unsigned char force_update);
void DSI_write_lcm_cmd(unsigned int cmd);
void DSI_write_lcm_regs(unsigned int addr, unsigned int *para, unsigned int nums);
UINT32 DSI_read_lcm_reg(void);
UINT32 DSI_dcs_read_lcm_reg(unsigned char);
UINT32 DSI_dcs_read_lcm_reg_v2(UINT8 cmd, UINT8 *buffer, UINT8 buffer_size);
DSI_STATUS DSI_write_lcm_fb(unsigned int addr, BOOL long_length);
DSI_STATUS DSI_read_lcm_fb(void);

DSI_STATUS DSI_enable_MIPI_txio(BOOL en);
BOOL Need_Wait_ULPS(void);

DSI_STATUS Wait_ULPS_Mode(void);
DSI_STATUS Wait_WakeUp(void);

DSI_STATUS DSI_DumpRegisters(void);

DSI_STATUS DSI_FMDesense_Query(void);
DSI_STATUS DSI_FM_Desense(unsigned long freq);
DSI_STATUS DSI_Reset_CLK(void);
DSI_STATUS DSI_Get_Default_CLK(unsigned int *clk);
DSI_STATUS DSI_Get_Current_CLK(unsigned int *clk);
DSI_STATUS DSI_Change_CLK(unsigned int clk);

void DSI_Set_VM_CMD(LCM_PARAMS *lcm_params);

DSI_STATUS DSI_EnableVM_CMD(void);

void DSI_WaitBtaTE(void);
DSI_STATUS DSI_WaitVsync(void);
DSI_STATUS DSI_RegUpdate(void);
#ifdef __cplusplus
}
#endif

#endif // __DPI_DRV_H__
