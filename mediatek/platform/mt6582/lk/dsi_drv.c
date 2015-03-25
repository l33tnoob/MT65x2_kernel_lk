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
// ---------------------------------------------------------------------------
// include
// ---------------------------------------------------------------------------
#include <platform/disp_drv_platform.h>

#include <platform/ddp_path.h>
#include <platform/mt_gpt.h>
#include <string.h>

// ---------------------------------------------------------------------------
// define
// ---------------------------------------------------------------------------
#if !(defined(CONFIG_MT6589_FPGA) || defined(BUILD_LK))
//#define DSI_MIPI_API
#endif

#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

#include <platform/sync_write.h>
#ifdef OUTREG32
  #undef OUTREG32
  #define OUTREG32(x, y) mt65xx_reg_sync_writel(y, x)
#endif

#ifndef OUTREGBIT
#define OUTREGBIT(TYPE,REG,bit,value)  \
                    do {    \
                        TYPE r = *((TYPE*)&INREG32(&REG));    \
                        r.bit = value;    \
                        OUTREG32(&REG, AS_UINT32(&r));    \
                    } while (0)
#endif

/*****************************************************************************/
/* fix warning: dereferencing type-punned pointer will break strict-aliasing */
/*              rules [-Wstrict-aliasing]                                    */
/*****************************************************************************/
#define DSI_OUTREG32_R(type, addr2, addr1) DISP_OUTREG32_R(type, addr2, addr1)
#define DSI_OUTREG32_V(type, addr2, var) DISP_OUTREG32_V(type, addr2, var)
#define DSI_OUTREGBIT(TYPE,REG,bit,value) DISP_OUTREGBIT(TYPE,REG,bit,value)
#define DSI_INREG32(type,addr) DISP_INREG32(type,addr)


// ---------------------------------------------------------------------------
// struct
// ---------------------------------------------------------------------------
typedef struct
{
	DSI_REGS regBackup;
	unsigned int cmdq_size;
	DSI_CMDQ_REGS cmdqBackup;
	unsigned int bit_time_ns;
	unsigned int vfp_period_us;
	unsigned int vsa_vs_period_us;
	unsigned int vsa_hs_period_us;
	unsigned int vsa_ve_period_us;
	unsigned int vbp_period_us;
    void (*pIntCallback)(DISP_INTERRUPT_EVENTS);
} DSI_CONTEXT;

typedef enum
{
	NO_TE  = 0,
	BTA_TE = 1,
	EXT_TE = 4,
} DSI_TE_MODE;

// ---------------------------------------------------------------------------
// global var
// ---------------------------------------------------------------------------
static PDSI_REGS const DSI_REG = (PDSI_REGS)(DSI_BASE);
static PDSI_PHY_REGS const DSI_PHY_REG = (PDSI_PHY_REGS)(MIPI_CONFIG_BASE);
static PDSI_CMDQ_REGS const DSI_CMDQ_REG = (PDSI_CMDQ_REGS)(DSI_BASE+0x180);
static PLCD_REGS const LCD_REG = (PLCD_REGS)(LCD_BASE);
static PDSI_VM_CMDQ_REGS const DSI_VM_CMD_REG = (PDSI_VM_CMDQ_REGS)(DSI_BASE + 0x134);

static LCM_PARAMS lcm_params_for_clk_setting;
static BOOL s_isDsiPowerOn = FALSE;
static DSI_CONTEXT _dsiContext;
static volatile bool isTeSetting = false;
static volatile unsigned int dsiTeMode = NO_TE;
static volatile unsigned int teTryNum = 0;
static bool glitch_log_on = false;
extern LCM_PARAMS *lcm_params;
// PLL_clock * 10
#define PLL_TABLE_NUM  (26)
int LCM_DSI_6572_PLL_CLOCK_List[PLL_TABLE_NUM+1]={0,1000,1040,1250,1300,1500,1560,1750,1820,2000,2080,2250,
				2340,2500,2600,2750,2860,3000,3120,3250,3500,3750,4000,4250,4500,4750,5000};

DSI_PLL_CONFIG pll_config[PLL_TABLE_NUM+1] =
		{{0,0,0,0,0,0,0},
		{2,0,0x3D89D89D,1,0x1B1,0x745,0x745},
		{2,0,0x40000000,1,0x1B1,0x790,0x790},
		{1,0,0x26762762,1,0x1B1,0x48B,0x48B},
		{1,0,0x28000000,1,0x1B1,0x4BA,0x4BA},
		{1,0,0x2E276276,1,0x1B1,0x574,0x574},
		{1,0,0x30000000,1,0x1B1,0x5AC,0x5AC},
		{1,0,0x35D89D89,1,0x1B1,0x65D,0x65D},
		{1,0,0x38000000,1,0x1B1,0x69E,0x69E},
		{1,0,0x3D89D89D,1,0x1B1,0x745,0x745},
		{1,0,0x40000000,1,0x1B1,0x790,0x790},
		{1,0,0x453B13B1,1,0x1B1,0x82E,0x82E},
		{1,0,0x48000000,1,0x1B1,0x882,0x882},
		{0,0,0x26762762,1,0x1B1,0x48B,0x48B},
		{0,0,0x28000000,1,0x1B1,0x4BA,0x4BA},
		{0,0,0x2A4EC4EC,1,0x1B1,0x500,0x500},
		{0,0,0x2C000000,1,0x1B1,0x533,0x533},
		{0,0,0x2E276276,1,0x1B1,0x574,0x574},
		{0,0,0x30000000,1,0x1B1,0x5AC,0x5AC},
		{0,0,0x32000000,1,0x1B1,0x5E8,0x5E8},
		{0,0,0x35D89D89,1,0x1B1,0x65D,0x65D},
		{0,0,0x39B13B13,1,0x1B1,0x6D1,0x6D1},
		{0,0,0x3D89D89D,1,0x1B1,0x745,0x745},
		{0,0,0x41627627,1,0x1B1,0x7BA,0x7BA},
		{0,0,0x453B13B1,1,0x1B1,0x82E,0x82E},
		{0,0,0x4913B13B,1,0x1B1,0x8A2,0x8A2},
		{0,0,0x4CEC4EC4,1,0x1B1,0x917,0x917},
};

// ---------------------------------------------------------------------------
// Private Functions
// ---------------------------------------------------------------------------
static long int get_current_time_us(void)
{
    return 0;       ///TODO: fix me
}

static void lcm_mdelay(UINT32 ms)
{
	udelay(1000 * ms);
}

static BOOL _IsEngineBusy(void)
{
	DSI_INT_STATUS_REG status;
	//arch_clean_invalidate_cache_range(*(unsigned int*)(&DSI_REG->DSI_STA), 32);

	status = DSI_REG->DSI_INTSTA;

	if (status.BUSY)
	{
		return TRUE;
	}

	return FALSE;
}

static void _WaitForEngineNotBusy(void)
{
    int timeOut;

	if(DSI_REG->DSI_MODE_CTRL.MODE != CMD_MODE)
	{
		return;
	}

	timeOut = 400;

	while(_IsEngineBusy())
	{
		udelay(100);
		/*printk("xuecheng, dsi wait\n");*/
		if (--timeOut < 0) {
			DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!\n");
			DSI_DumpRegisters();
			DSI_Reset();
			break;
		}
	}

	DSI_OUTREG32_V(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA, 0x0);

}

static void _ResetBackupedDSIRegisterValues(void)
{
    DSI_REGS *regs = &_dsiContext.regBackup;
    memset((void*)regs, 0, sizeof(DSI_REGS));
}

static void _DSI_BackUpCmdQ(void)
{
	unsigned int i;
	DSI_CMDQ_REGS *regs = &(_dsiContext.cmdqBackup);

	_dsiContext.cmdq_size = DSI_INREG32(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE);

	for (i=0; i<_dsiContext.cmdq_size; i++)
	{
		OUTREG32(&regs->data[i], AS_UINT32(&DSI_CMDQ_REG->data[i]));
	}
}


static void _DSI_RestoreCmdQ(void)
{
	unsigned int i;
	DSI_CMDQ_REGS *regs = &(_dsiContext.cmdqBackup);

	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, _dsiContext.cmdq_size);

	for (i=0; i<_dsiContext.cmdq_size; i++)
	{
		OUTREG32(&DSI_CMDQ_REG->data[i], AS_UINT32(&regs->data[i]));
	}
}

static void _DSI_WaitBtaTE(void)
{
	DSI_T0_INS t0;
	long int dsi_wait_time = 0;

	_WaitForEngineNotBusy();

	DSI_clk_HS_mode(0);

	// backup command queue setting.
	_DSI_BackUpCmdQ();

	t0.CONFG = 0x20;		///TE
	t0.Data0 = 0;
	t0.Data_ID = 0;
	t0.Data1 = 0;

	DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);

	// polling TE_RDY
	while(DSI_REG->DSI_INTSTA.TE_RDY == 0)
	{
	    udelay(100);//sleep 50us
		dsi_wait_time++;
		if(dsi_wait_time > 40000)
		{
			DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for TE_RDY timeout!!!\n");

			//DumpRegisters first
			DSI_DumpRegisters();

			// Set DSI_RACK to let DSI idle
			DSI_REG->DSI_RACK.DSI_RACK = 1;

			//do necessary reset here
			DSI_Reset();

			if(teTryNum)
			{
				teTryNum--;
			}
			else
			{
				dsiTeMode = NO_TE;//disable TE
			}

			goto wait_bta_end;
		}
	}

	// Write clear RD_RDY
	DSI_REG->DSI_INTSTA.TE_RDY = 0;

	// Set DSI_RACK to let DSI idle
	DSI_REG->DSI_RACK.DSI_RACK = 1;

	dsi_wait_time = 0;

	// polling CMD_DONE
	while(DSI_REG->DSI_INTSTA.CMD_DONE == 0)
	{
	    udelay(100);//sleep 50us
		dsi_wait_time++;
		if(dsi_wait_time > 40000)
		{
			DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for TE_RDY's CMD_DONE timeout!!!\n");

			//DumpRegisters first
			DSI_DumpRegisters();

			// Set DSI_RACK to let DSI idle
			DSI_REG->DSI_RACK.DSI_RACK = 1;

			///do necessary reset here
			DSI_Reset();

			if(teTryNum)
			{
				teTryNum--;
			}
			else
			{
				dsiTeMode = NO_TE;//disable TE
			}

			goto wait_bta_end;
		}
	}

	// Write clear CMD_DONE
	DSI_REG->DSI_INTSTA.CMD_DONE = 0;

wait_bta_end:
	// restore command queue setting.
	_DSI_RestoreCmdQ();

}

static void _DSI_WaitExternalTE(void)
{
	UINT32 time0, time1;

    long int dsi_wait_time = 0;

	time0 = gpt4_tick2time_us(gpt4_get_current_tick());
    while(DSI_REG->DSI_INTSTA.EXT_TE == 0)	// polling EXT_TE
    {
        udelay(100);//sleep 50us
        dsi_wait_time++;
        if(dsi_wait_time > 40000)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for EXT_TE timeout!!!\n");

			//DumpRegisters first
			DSI_DumpRegisters();

			//HW Limit: need disable ext te here.
			DSI_EnableExtTE(0);

            //do necessary reset here
            DSI_Reset();

            dsiTeMode = NO_TE;

            return;
        }
    }

	time1 = gpt4_tick2time_us(gpt4_get_current_tick());
	printf("[wwy]polling ext te time =%d , DSI_INTSTA = 0x%08x\n", time1 - time0,DSI_INREG32(PDSI_INT_STATUS_REG,&(DSI_REG->DSI_INTSTA)));

	// Write clear EXT_TE
	DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,EXT_TE,0);

}
static bool has_adjusted = false;
static void DSI_PHY_clk_adjusting(void)
{
	MIPITX_DSI_BG_CON_REG temp_BG_CON_REG;
	u32 m_hw_res3;
	u32 temp1, temp2;

	temp_BG_CON_REG = DSI_PHY_REG->MIPITX_DSI_BG_CON;
	
	if(!has_adjusted){
		m_hw_res3 = INREG32(0x10206170);
		temp1 = (m_hw_res3 >> 27) & 0xF;
		temp2 = (m_hw_res3 >> 23) & 0xF;
		printk("DSI_PHY_clk_adjusting: efuse register 0x10206170 = 0x%x, bit27_30 = %d, bit23_26 = %d\n", m_hw_res3, temp1, temp2);
		printk("DSI_BG_CON = 0x%x\n", INREG32(&DSI_PHY_REG->MIPITX_DSI_BG_CON));	
		if(0 != temp1)
		{
			//DSI_PHY_REG->MIPITX_DSI_BG_CON.RG_DSI_BG_R2_TRIM += temp1;
			temp_BG_CON_REG.RG_DSI_BG_R2_TRIM += temp1;
		}
		if(0 != temp2)
		{
			//DSI_PHY_REG->MIPITX_DSI_BG_CON.RG_DSI_BG_R1_TRIM += temp2;
			temp_BG_CON_REG.RG_DSI_BG_R1_TRIM += temp2;
		}
		OUTREG32(&DSI_PHY_REG->MIPITX_DSI_BG_CON, AS_UINT32(&temp_BG_CON_REG));
		printk("after DSI_PHY_clk_adjusting: DSI_BG_CON = 0x%x\n", INREG32(&DSI_PHY_REG->MIPITX_DSI_BG_CON));
		has_adjusted = true;
	}
}

static void _DSI_PHY_clk_setting(LCM_PARAMS *lcm_params)
{
	//unsigned int pll_index = 0;
	unsigned int data_Rate = lcm_params->dsi.PLL_CLOCK*2;
	unsigned int txdiv,pcw;
//	unsigned int fmod = 30;//Fmod = 30KHz by default
	unsigned int delta1 = 5;//Delta1 is SSC range, default is 0%~-5%
	unsigned int pdelta1;

	OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_IMP_CAL_CODE,8);
	OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_HS_BIAS_EN,1);

	OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V032_SEL,4);
	OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V04_SEL,4);
	OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V072_SEL,4);
	OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V10_SEL,4);
	OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V12_SEL,4);
	OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CKEN,1);
	OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CORE_EN,1);
    	DSI_PHY_clk_adjusting();
	mdelay(10);

	OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_CKG_LDOOUT_EN,1);
	OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_LDOCORE_EN,1);

	OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_PWR_ON,1);
	OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_ISO_EN,1);
	mdelay(10);

	OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_ISO_EN,0);

	OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PREDIV,0);
	OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_POSDIV,0);

	if(0!=data_Rate){//if lcm_params->dsi.dsi_clock=0, use other method
		if(data_Rate > 1250){
			printk("[dsi_drv.c error]Data Rate exceed limitation\n");
			ASSERT(0);
		}
		else if(data_Rate >= 500)
			txdiv = 1;
		else if(data_Rate >= 250)
			txdiv = 2;
		else if(data_Rate >= 125)
			txdiv = 4;
		else if(data_Rate > 62)
			txdiv = 8;
		else if(data_Rate >= 50)
			txdiv = 16;
		else{
			printk("[dsi_drv.c Error]: dataRate is too low,%d!!!\n", __LINE__);
			ASSERT(0);
		}
		//PLL txdiv config
		switch(txdiv)
		{
			case 1:
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,0);
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,0);
				break;
			case 2:
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,1);
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,0);
				break;
			case 4:
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,2);
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,0);
				break;
			case 8:
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,2);
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,1);
				break;
			case 16:
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,2);
				OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,2);
				break;
			default:
				break;
		}

		// PLL PCW config
		/*
		PCW bit 24~30 = floor(pcw)
		PCW bit 16~23 = (pcw - floor(pcw))*256
		PCW bit 8~15 = (pcw*256 - floor(pcw)*256)*256
		PCW bit 8~15 = (pcw*256*256 - floor(pcw)*256*256)*256
		*/
		//	pcw = data_Rate*4*txdiv/(26*2);//Post DIV =4, so need data_Rate*4
		pcw = data_Rate*txdiv/13;

		OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_H,(pcw & 0x7F));
		OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_16_23,((256*(data_Rate*txdiv%13)/13) & 0xFF));
		OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_8_15,((256*(256*(data_Rate*txdiv%13)%13)/13) & 0xFF));
		OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_0_7,((256*(256*(256*(data_Rate*txdiv%13)%13)%13)/13) & 0xFF));

		//SSC config
//		pmod = ROUND(1000*26MHz/fmod/2);fmod default is 30Khz, and this value not be changed
//		pmod = 433.33;
		if(1 != lcm_params->dsi.ssc_disable){
			OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_PH_INIT,1);
			OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_PRD,0x1B1);//PRD=ROUND(pmod) = 433;
			if(0 != lcm_params->dsi.ssc_range)
				delta1 = lcm_params->dsi.ssc_range;
			
			ASSERT(delta1<=8);
			pdelta1 = (delta1*data_Rate*txdiv*262144+281664)/563329;
			OUTREGBIT(MIPITX_DSI_PLL_CON3_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON3,RG_DSI0_MPPLL_SDM_SSC_DELTA,pdelta1);
			OUTREGBIT(MIPITX_DSI_PLL_CON3_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON3,RG_DSI0_MPPLL_SDM_SSC_DELTA1,pdelta1);
			//OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,1);
			printk("[dsi_drv.c] PLL config:data_rate=%d,txdiv=%d,pcw=%d,delta1=%d,pdelta1=0x%x\n",
				data_Rate,txdiv,INREG32(&DSI_PHY_REG->MIPITX_DSI_PLL_CON2),delta1,pdelta1);
		}
	}
	else{
		OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,lcm_params->dsi.pll_div1);
		OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,lcm_params->dsi.pll_div2);

		OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_H,((lcm_params->dsi.fbk_div)<< 2));
		OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_16_23,0);
		OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_8_15,0);
		OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_0_7,0);

		//OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,0);
	}
        OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,1);

	OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_RT_CODE,0x8);
	OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_PHI_SEL,0x1);
	OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_LDOOUT_EN,1);
	if(lcm_params->dsi.LANE_NUM > 0)
	{
		OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_RT_CODE,0x8);
		OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_LDOOUT_EN,1);
	}

	if(lcm_params->dsi.LANE_NUM > 1)
	{
		OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_RT_CODE,0x8);
		OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_LDOOUT_EN,1);
	}

	if(lcm_params->dsi.LANE_NUM > 2)
	{
		OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_RT_CODE,0x8);
		OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_LDOOUT_EN,1);
	}

	if(lcm_params->dsi.LANE_NUM > 3)
	{
		OUTREGBIT(MIPITX_DSI0_DATA_LANE3_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE3,RG_DSI0_LNT3_RT_CODE,0x8);
		OUTREGBIT(MIPITX_DSI0_DATA_LANE3_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE3,RG_DSI0_LNT3_LDOOUT_EN,1);
	}
	printk("after efuse adjust, DSI_CLK_REG = 0x%x, DSI_DAT0_REG=0x%x,DSI_DAT1_REG=0x%x,DSI_DAT2_REG=0x%x,DSI_DAT3_REG=0x%x\n",
		INREG32(&DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE),
		INREG32(&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0),
		INREG32(&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1),
		INREG32(&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2),
		INREG32(&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE3));

	OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PLL_EN,1);
	mdelay(1);
	if((0 != data_Rate) && (1 != lcm_params->dsi.ssc_disable))
		OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_EN,1);
	else
		OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_EN,0);

	// default POSDIV by 4
    OUTREGBIT(MIPITX_DSI_PLL_TOP_REG,DSI_PHY_REG->MIPITX_DSI_PLL_TOP,RG_MPPLL_PRESERVE_L,3);
	OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_PAD_TIE_LOW_EN, 0);
}

// ---------------------------------------------------------------------------
// Public Functions
// ---------------------------------------------------------------------------
void DSI_MEM_CONTI_Setting(UINT32 dsi_mem_conti)
{
	DSI_OUTREG32_V(PDSI_MEM_CONTI_REG,&DSI_REG->DSI_MEM_CONTI, dsi_mem_conti);
}

void DSI_EnableExtTE_Interrupt(BOOL enable)
{
	/* External TE Setting */
	DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,EXT_TE,enable?1:0);
}

void DSI_Enable_CG(void)
{
	MASKREG32(DISPSYS_BASE+0x118, 0x3, 0x3);
}

void DSI_Disable_CG(void)
{
	MASKREG32(DISPSYS_BASE+0x114, 0x3, 0x3);
}

unsigned int DSI_Get_CG(void)
{
	return INREG32(DISPSYS_BASE+0x110);
}

void DSI_EnableExtTE(BOOL enable)
{
	/* Enable External TE TXRX */
	DSI_OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EN,enable?1:0);
}


DSI_STATUS DSI_BackupRegisters(void)
{
    DSI_REGS *regs = &(_dsiContext.regBackup);

    //memcpy((void*)&(_dsiContext.regBackup), (void*)DSI_BASE, sizeof(DSI_REGS));

    DSI_OUTREG32_R(PDSI_INT_ENABLE_REG,&regs->DSI_INTEN, &DSI_REG->DSI_INTEN);
    DSI_OUTREG32_R(PDSI_MODE_CTRL_REG,&regs->DSI_MODE_CTRL, &DSI_REG->DSI_MODE_CTRL);
    DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&regs->DSI_TXRX_CTRL, &DSI_REG->DSI_TXRX_CTRL);
    DSI_OUTREG32_R(PDSI_PSCTRL_REG,&regs->DSI_PSCTRL, &DSI_REG->DSI_PSCTRL);

    DSI_OUTREG32_R(PDSI_VSA_NL_REG,&regs->DSI_VSA_NL, &DSI_REG->DSI_VSA_NL);
    DSI_OUTREG32_R(PDSI_VBP_NL_REG,&regs->DSI_VBP_NL, &DSI_REG->DSI_VBP_NL);
    DSI_OUTREG32_R(PDSI_VFP_NL_REG,&regs->DSI_VFP_NL, &DSI_REG->DSI_VFP_NL);
    DSI_OUTREG32_R(PDSI_VACT_NL_REG,&regs->DSI_VACT_NL, &DSI_REG->DSI_VACT_NL);

    DSI_OUTREG32_R(PDSI_HSA_WC_REG,&regs->DSI_HSA_WC, &DSI_REG->DSI_HSA_WC);
    DSI_OUTREG32_R(PDSI_HBP_WC_REG,&regs->DSI_HBP_WC, &DSI_REG->DSI_HBP_WC);
    DSI_OUTREG32_R(PDSI_HFP_WC_REG,&regs->DSI_HFP_WC, &DSI_REG->DSI_HFP_WC);
    DSI_OUTREG32_R(PDSI_BLLP_WC_REG,&regs->DSI_BLLP_WC, &DSI_REG->DSI_BLLP_WC);

    OUTREG32(&regs->DSI_HSTX_CKL_WC, AS_UINT32(&DSI_REG->DSI_HSTX_CKL_WC));


	DSI_OUTREG32_R(PDSI_MEM_CONTI_REG,&regs->DSI_MEM_CONTI, &DSI_REG->DSI_MEM_CONTI);


    OUTREG32(&regs->DSI_PHY_TIMECON0, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON0));
    OUTREG32(&regs->DSI_PHY_TIMECON1, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON1));
    OUTREG32(&regs->DSI_PHY_TIMECON2, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON2));
    OUTREG32(&regs->DSI_PHY_TIMECON3, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON3));

    DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG,&regs->DSI_VM_CMD_CON, &DSI_REG->DSI_VM_CMD_CON);

	return DSI_STATUS_OK;

}

DSI_STATUS DSI_RestoreRegisters(void)
{
    DSI_REGS *regs = &(_dsiContext.regBackup);

    DSI_OUTREG32_R(PDSI_INT_ENABLE_REG,&DSI_REG->DSI_INTEN, &regs->DSI_INTEN);
    DSI_OUTREG32_R(PDSI_MODE_CTRL_REG,&DSI_REG->DSI_MODE_CTRL, &regs->DSI_MODE_CTRL);
    DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&DSI_REG->DSI_TXRX_CTRL, &regs->DSI_TXRX_CTRL);
    DSI_OUTREG32_R(PDSI_PSCTRL_REG,&DSI_REG->DSI_PSCTRL, &regs->DSI_PSCTRL);

    DSI_OUTREG32_R(PDSI_VSA_NL_REG,&DSI_REG->DSI_VSA_NL, &regs->DSI_VSA_NL);
    DSI_OUTREG32_R(PDSI_VBP_NL_REG,&DSI_REG->DSI_VBP_NL, &regs->DSI_VBP_NL);
    DSI_OUTREG32_R(PDSI_VFP_NL_REG,&DSI_REG->DSI_VFP_NL, &regs->DSI_VFP_NL);
    DSI_OUTREG32_R(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, &regs->DSI_VACT_NL);

    DSI_OUTREG32_R(PDSI_HSA_WC_REG,&DSI_REG->DSI_HSA_WC, &regs->DSI_HSA_WC);
    DSI_OUTREG32_R(PDSI_HBP_WC_REG,&DSI_REG->DSI_HBP_WC, &regs->DSI_HBP_WC);
    DSI_OUTREG32_R(PDSI_HFP_WC_REG,&DSI_REG->DSI_HFP_WC, &regs->DSI_HFP_WC);
    DSI_OUTREG32_R(PDSI_BLLP_WC_REG,&DSI_REG->DSI_BLLP_WC, &regs->DSI_BLLP_WC);


    OUTREG32(&DSI_REG->DSI_HSTX_CKL_WC, AS_UINT32(&regs->DSI_HSTX_CKL_WC));

    DSI_OUTREG32_R(PDSI_MEM_CONTI_REG,&DSI_REG->DSI_MEM_CONTI, &regs->DSI_MEM_CONTI);

    OUTREG32(&DSI_REG->DSI_PHY_TIMECON0, AS_UINT32(&regs->DSI_PHY_TIMECON0));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON1, AS_UINT32(&regs->DSI_PHY_TIMECON1));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON2, AS_UINT32(&regs->DSI_PHY_TIMECON2));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON3, AS_UINT32(&regs->DSI_PHY_TIMECON3));

    DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG,&DSI_REG->DSI_VM_CMD_CON, &regs->DSI_VM_CMD_CON);

	return DSI_STATUS_OK;

}


DSI_STATUS DSI_TE_Setting(void)
{
    //return DSI_STATUS_OK;
    if(isTeSetting)
    {
        return DSI_STATUS_OK;
    }

    if(lcm_params->dsi.mode != CMD_MODE)
    {
    	goto teSettingEnd;
    }

    if(lcm_params->dsi.lcm_ext_te_enable == TRUE)
    {
        //Enable EXT TE
        dsiTeMode = EXT_TE;

		//Enable EXT TE Interrupt
		DSI_EnableExtTE_Interrupt(TRUE);
    }
    else
    {
        //Enable BTA TE
		dsiTeMode = BTA_TE;

    }

    teTryNum = 5;

teSettingEnd:

    isTeSetting = true;

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Init(BOOL isDsiPoweredOn)
{
    memset(&_dsiContext, 0, sizeof(_dsiContext));

	//isDsiPoweredOn = false in lk
    if (isDsiPoweredOn) {
        DSI_BackupRegisters();
    } else {
        _ResetBackupedDSIRegisterValues();
    }

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Deinit(void)
{
    DSI_STATUS ret = DSI_PowerOff();

    ASSERT(ret == DSI_STATUS_OK);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PowerOn(void)
{
    if (!s_isDsiPowerOn)
    {
        DSI_Enable_CG();

        s_isDsiPowerOn = TRUE;
	    printf("\n[DISP] - DSI_PowerOn. 0x%8x\n", DSI_Get_CG());
    }

    return DSI_STATUS_OK;

}


DSI_STATUS DSI_PowerOff(void)
{
	if (s_isDsiPowerOn)
	{
		DSI_Disable_CG();

		s_isDsiPowerOn = FALSE;
		printf("[DISP] - DSI_PowerOff. 0x%8x\n", DSI_Get_CG());
	}

	return DSI_STATUS_OK;

}

DSI_STATUS DSI_WaitForNotBusy(void)
{
    _WaitForEngineNotBusy();

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_EnableClk(void)
{
	DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_EN,1);
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Start(void)
{
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
	return DSI_STATUS_OK;
}

DSI_STATUS DSI_SleepOut(void)
{
    DSI_OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,SLEEP_MODE,1);
    DSI_OUTREGBIT(DSI_PHY_TIMCON4_REG,DSI_REG->DSI_PHY_TIMECON4,ULPS_WAKEUP,0x22E09);  // cycle to 1ms for 520MHz

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Wakeup(void)
{
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,SLEEPOUT_START,0);
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,SLEEPOUT_START,1);
    mdelay(1);

    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,SLEEPOUT_START,0);
    DSI_OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,SLEEP_MODE,0);

    return DSI_STATUS_OK;
}

void DSI_WaitTE(void)
{
	if(DSI_REG->DSI_MODE_CTRL.MODE != CMD_MODE)
	{
		return;
	}

	switch (dsiTeMode)
	{
		case NO_TE:
			break;
		case BTA_TE:
			_DSI_WaitBtaTE();
			break;
		case EXT_TE:
			_DSI_WaitExternalTE();
			break;
		default:
			break;
	}
}


DSI_STATUS DSI_WaitVsync()
{
	Wait_Rdma_Start(0);

	return DSI_STATUS_OK;
}

DSI_STATUS DSI_RegUpdate(void)
{
	printk("[wwy] enter DSI_RegUpdate\n");
	disp_wait_reg_update();
	printk("[wwy] end DSI_RegUpdate\n");

	return DSI_STATUS_OK;
}

unsigned int glitch_detect_fail_cnt = 0;

unsigned int DSI_Detect_CLK_Glitch_Default(void)
{
	DSI_T0_INS t0;
	char i;
	int read_timeout_cnt=10000;
	int read_timeout_ret = 0;
	unsigned int try_times = 50;
//    MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagStart, 0, 0);
/**********************start******************/
    _WaitForEngineNotBusy();
   // lcdStartTransfer = true;
	if(glitch_detect_fail_cnt>2){
		return 0;
	}

	_DSI_BackUpCmdQ();

	DSI_SetMode(CMD_MODE);
	OUTREG32(&DSI_CMDQ_REG->data[0], 0x00340500);//turn off TE

	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);

	while(DSI_REG->DSI_INTSTA.CMD_DONE == 0);
	DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);

#if 1
	OUTREG32(&DSI_CMDQ_REG->data[0], 0x00ff1500);

	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
	while(DSI_REG->DSI_INTSTA.CMD_DONE == 0);
	DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);

#endif
	for(i=0;i<try_times;i++)
	{
	    if(glitch_log_on)
		  printk("Test log 1: try time i = %d!!\n", i);
		 DSI_clk_HS_mode(0);

	while((DSI_INREG32(PDSI_STATE_DBG0_REG,&DSI_REG->DSI_STATE_DBG0)&0x1) == 0);	 // polling bit0

	DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
	DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);//reset
	DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);

		if(i>0)
			 {
			  MASKREG32(MIPI_CONFIG_BASE + 0x04, 0x20, 0x0);
			 }
		  DSI_clk_HS_mode(1);
			while((DSI_INREG32(PDSI_STATE_DBG0_REG,&DSI_REG->DSI_STATE_DBG0)&0x40000) == 0)	 // polling bit18 start
		  	{
			  if(glitch_log_on)
		         printk("Test log 2: wait for DSI_STATE_DBG0 bit18==1 \n");
		  	}
		  if(i>0)
			 {
			  MASKREG32(MIPI_CONFIG_BASE + 0x04, 0x20, 0x20);
			 }
//			OUTREG32(&DSI_CMDQ_REG->data[0], 0x00290508);

			OUTREG32(&DSI_CMDQ_REG->data[0], 0x00351508);
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

			DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
			DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);

			read_timeout_cnt=10000;
			while(DSI_REG->DSI_INTSTA.BUSY) {
				udelay(1);
				if (--read_timeout_cnt < 0) {
					DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!:%d\n",__LINE__);
					DSI_DumpRegisters();
					DSI_Reset();
					break;
				}
			}

			DSI_OUTREG32_V(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA, 0x0);

		  t0.CONFG = 0x04;
		  t0.Data0 = 0;
		  t0.Data_ID = 0;
		  t0.Data1 = 0;

		DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
		DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

		DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
		DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);

		 DSI_RX_DATA_REG read_data0;
		 DSI_RX_DATA_REG read_data1;

			read_timeout_cnt=10;
		  while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
				 {
					 ///keep polling
				if(glitch_log_on)
		             printk("Test log 3:polling ack & error report \n");
					 udelay(100);
					 read_timeout_cnt--;
//					printk("polling time = %d us\n", ((unsigned int)end_time - (unsigned int)start_time));
					if(read_timeout_cnt==0)
					 {
//					    if(glitch_log_on)
//		                   printk("Test log 4:Polling DSI read ready timeout,%d us\n", (unsigned int)sched_clock() - (unsigned int)start_time);

//						DSI_DumpRegisters();
						DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
						 DSI_Reset();
						 read_timeout_ret = 1;
						 break;
					 }
				 }
		if(1 == read_timeout_ret){
			read_timeout_ret = 0;
			continue;
		}

		DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,0);

		 if(((DSI_REG->DSI_TRIG_STA.TRIG2) )==1)
		 {
			break;
//			continue;
		  }
		 else
			 {
			  //read error report
			  OUTREG32(&read_data0, AS_UINT32(&DSI_REG->DSI_RX_DATA0));
			  OUTREG32(&read_data1, AS_UINT32(&DSI_REG->DSI_RX_DATA1));

			  if(glitch_log_on)
			  	{
			  	  printk("read_data0, %x,%x,%x,%x\n", read_data0.byte0, read_data0.byte1, read_data0.byte2, read_data0.byte3);
	              printk("read_data1, %x,%x,%x,%x\n", read_data1.byte0, read_data1.byte1, read_data1.byte2, read_data1.byte3);
			  	}

			  if(((read_data0.byte1&0x7) != 0)||((read_data0.byte2&0x3)!=0)) //bit 0-3	bit 8-9
				{
				  continue;
				}
			  else
				 {
//	 				continue;
				  break;// jump out the for loop ,go to refresh
				 }

			 }
	 	}
#if 1
	if(i>0)
		printk("detect times:%d\n",i);
#endif
    _DSI_RestoreCmdQ();

#if 1
	switch(lcm_params->dsi.LANE_NUM)
	{
		case LCM_FOUR_LANE:
			OUTREG32(MIPI_CONFIG_BASE + 0x84, 0x3CF3C7B1);
			break;
		case LCM_THREE_LANE:
			OUTREG32(MIPI_CONFIG_BASE + 0x84, 0x00F3C7B1);
			break;
        default:
            OUTREG32(MIPI_CONFIG_BASE + 0x84, 0x0003C7B1);
	}

	 OUTREG32(MIPI_CONFIG_BASE + 0x88, 0x0);
	 OUTREG32(MIPI_CONFIG_BASE + 0x80, 0x1);

     DSI_REG->DSI_COM_CTRL.DSI_RESET = 0;
	 DSI_REG->DSI_COM_CTRL.DSI_RESET = 1;
	 DSI_REG->DSI_COM_CTRL.DSI_RESET = 0;

     DSI_clk_HS_mode(1);

	  if(glitch_log_on)
		     printk("Test log 5:start Polling bit18\n");
	while((DSI_INREG32(PDSI_STATE_DBG0_REG,&DSI_REG->DSI_STATE_DBG0)&0x40000) == 0)	 // polling bit18
		 {
		  	  udelay(1);
		  }

	 if(glitch_log_on)
		    printk("Test log 6:start Polling bit18\n");

     OUTREG32(MIPI_CONFIG_BASE + 0x80, 0x0);
#endif

	read_timeout_cnt=1000000;
	while(DSI_REG->DSI_INTSTA.BUSY) {
		udelay(1);
			/*printk("xuecheng, dsi wait\n");*/
		if (--read_timeout_cnt < 0) {
			DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!:%d\n",__LINE__);
			DSI_DumpRegisters();
			DSI_Reset();
			break;
		}
	}
	DSI_OUTREG32_V(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA, 0x0);
	DSI_SetMode(lcm_params->dsi.mode);
//	if(glitch_log_on)
	if(i == try_times){
		glitch_detect_fail_cnt++;
		return 1;
	}
	glitch_detect_fail_cnt = 0;
	return 0;
}

unsigned int DSI_Detect_CLK_Glitch_Parallel(void)
{
	DSI_T0_INS t0;
	char i;
	int read_timeout_cnt=10000;
	int read_timeout_ret = 0;
	unsigned int try_times = 50;
    int read_IC_ID = 0;

//    MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagStart, 0, 0);
/**********************start******************/
    _WaitForEngineNotBusy();
   // lcdStartTransfer = true;
	if(glitch_detect_fail_cnt>2){
		return 0;
	}

	_DSI_BackUpCmdQ();

	DSI_SetMode(CMD_MODE);

	for(i=0;i<try_times*4;i++)
	{
        if(read_IC_ID == 0) // slave
        {

	    if(glitch_log_on)
		  printk("Test log 1: try time i = %d!!\n", i);
		 DSI_clk_HS_mode(0);

		while((DSI_INREG32(PDSI_STATE_DBG0_REG,&DSI_REG->DSI_STATE_DBG0)&0x1) == 0);	 // polling bit0

		DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
		DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);//reset
		DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
		if(i>0)
			 {
			  MASKREG32(MIPI_CONFIG_BASE + 0x04, 0x20, 0x0);
			 }
		  DSI_clk_HS_mode(1);

		while((DSI_INREG32(PDSI_STATE_DBG0_REG,&DSI_REG->DSI_STATE_DBG0)&0x40000) == 0)	 // polling bit18 start
		  	{
			  if(glitch_log_on)
		         printk("Test log 2: wait for DSI_STATE_DBG0 bit18==1 \n");
		  	}
		  if(i>0)
			 {
			  MASKREG32(MIPI_CONFIG_BASE + 0x04, 0x20, 0x20);
			 }
//			OUTREG32(&DSI_CMDQ_REG->data[0], 0x00290508);

        }

#if 1 // HS command
                  OUTREG32(&DSI_CMDQ_REG->data[0], 0xAA801508);
				DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

				DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
				DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
                  read_timeout_cnt=1000000;

                  while(DSI_REG->DSI_INTSTA.BUSY) {
                      udelay(1);
                      if (--read_timeout_cnt < 0) {
                          DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!:%d\n",__LINE__);
                          DSI_DumpRegisters();
                          DSI_Reset();
                          break;
                      }
                  }

				DSI_OUTREG32_V(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA, 0x0);
#endif
                  // LP command
                  if(read_IC_ID == 0) // slave
                  {
                      //OUTREG32(&DSI_CMDQ_REG->data[0], 0x00023902);
                      //OUTREG32(&DSI_CMDQ_REG->data[1], 0x000010B5);
                      OUTREG32(&DSI_CMDQ_REG->data[0], 0x10B51500);
                  }
                  else // read_IC_ID == 1, master
                  {
                      //OUTREG32(&DSI_CMDQ_REG->data[0], 0x00023902);
                      //OUTREG32(&DSI_CMDQ_REG->data[1], 0x000090B5);
                      OUTREG32(&DSI_CMDQ_REG->data[0], 0x90B51500);
                  }

		DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
		DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
		DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
		while(DSI_REG->DSI_INTSTA.CMD_DONE == 0);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);

		t0.CONFG = 0x04;
		t0.Data0 = 0;
		t0.Data_ID = 0;
		t0.Data1 = 0;

		DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
		DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

		DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
		DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);

		 DSI_RX_DATA_REG read_data0;
		 DSI_RX_DATA_REG read_data1;

			read_timeout_cnt=10;
		  while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
				 {
					 ///keep polling
				if(glitch_log_on)
		             printk("Test log 3:polling ack & error report \n");
					 udelay(100);
					 read_timeout_cnt--;
//					printk("polling time = %d us\n", ((unsigned int)end_time - (unsigned int)start_time));
					if(read_timeout_cnt==0)
					 {
//					    if(glitch_log_on)
//		                   printk("Test log 4:Polling DSI read ready timeout,%d us\n", (unsigned int)sched_clock() - (unsigned int)start_time);

//						DSI_DumpRegisters();
						DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);

						 DSI_Reset();
						 read_timeout_ret = 1;
						 break;
					 }
				 }
		if(1 == read_timeout_ret){
			read_timeout_ret = 0;
            printk("iii detect timeout ID:%d\n",read_IC_ID);
            read_IC_ID = 0;
			continue;
		}
		DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,0);

		 if(((DSI_REG->DSI_TRIG_STA.TRIG2) )==1)
		 {
            if(read_IC_ID == 0)
            {
                read_IC_ID = 1;
                continue;
            }
			break;
//			continue;
		  }
		 else
			 {
			  //read error report
			  OUTREG32(&read_data0, AS_UINT32(&DSI_REG->DSI_RX_DATA0));
			  OUTREG32(&read_data1, AS_UINT32(&DSI_REG->DSI_RX_DATA1));

			  if(glitch_log_on)
			  	{
			  	  printk("read_data0, %x,%x,%x,%x\n", read_data0.byte0, read_data0.byte1, read_data0.byte2, read_data0.byte3);
	              printk("read_data1, %x,%x,%x,%x\n", read_data1.byte0, read_data1.byte1, read_data1.byte2, read_data1.byte3);
			  	}

			  if(((read_data0.byte1&0x7) != 0)||((read_data0.byte2&0x3)!=0)) //bit 0-3	bit 8-9
				{
                    printk("read_data0, %x,%x,%x,%x\n", read_data0.byte0, read_data0.byte1, read_data0.byte2, read_data0.byte3);
		            printk("iii detect error ID:%d\n",read_IC_ID);
                    read_IC_ID = 0;
				  continue;
				}
			  else
				 {
                    if(read_IC_ID == 0)
                    {
                        read_IC_ID = 1;
                        continue;
                    }
//	 				continue;
				  break;// jump out the for loop ,go to refresh
				 }

			 }
	 	}
#if 1
	if(i>1)
		printk("detect times:%d\n",i);
#endif
    _DSI_RestoreCmdQ();

#if 1
	switch(lcm_params->dsi.LANE_NUM)
	{
		case LCM_FOUR_LANE:
			OUTREG32(MIPI_CONFIG_BASE + 0x84, 0x3CF3C7B1);
			break;
		case LCM_THREE_LANE:
			OUTREG32(MIPI_CONFIG_BASE + 0x84, 0x00F3C7B1);
			break;
        default:
            OUTREG32(MIPI_CONFIG_BASE + 0x84, 0x0003C7B1);
	}

	 OUTREG32(MIPI_CONFIG_BASE + 0x88, 0x0);
	 OUTREG32(MIPI_CONFIG_BASE + 0x80, 0x1);

     DSI_REG->DSI_COM_CTRL.DSI_RESET = 0;
	 DSI_REG->DSI_COM_CTRL.DSI_RESET = 1;
	 DSI_REG->DSI_COM_CTRL.DSI_RESET = 0;

     DSI_clk_HS_mode(1);

	  if(glitch_log_on)
		     printk("Test log 5:start Polling bit18\n");

		while((DSI_INREG32(PDSI_STATE_DBG0_REG,&DSI_REG->DSI_STATE_DBG0)&0x40000) == 0)	 // polling bit18
		 {
		  	  udelay(1);
		  }

	 if(glitch_log_on)
		    printk("Test log 6:start Polling bit18\n");

     OUTREG32(MIPI_CONFIG_BASE + 0x80, 0x0);
#endif

	read_timeout_cnt=1000000;
	while(DSI_REG->DSI_INTSTA.BUSY) {
		udelay(1);
			/*printk("xuecheng, dsi wait\n");*/
		if (--read_timeout_cnt < 0) {
			DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!:%d\n",__LINE__);
			DSI_DumpRegisters();
			DSI_Reset();
			break;
		}
	}

	DSI_OUTREG32_V(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA, 0x0);
	DSI_SetMode(lcm_params->dsi.mode);
//	if(glitch_log_on)
	if(i == try_times*4){
		glitch_detect_fail_cnt++;
		return 1;
	}
	glitch_detect_fail_cnt = 0;
	return 0;
}

unsigned int DSI_Detect_CLK_Glitch(void)
{
    if (lcm_params->dsi.compatibility_for_nvk == 1)
    {
        return DSI_Detect_CLK_Glitch_Default();
    }
    else if (lcm_params->dsi.compatibility_for_nvk == 2)
    {
        return DSI_Detect_CLK_Glitch_Parallel();
    }
    else
    {
        return DSI_Detect_CLK_Glitch_Default();
    }
}


DSI_STATUS DSI_DisableClk(void)
{
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Reset(void)
{
	DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
	lcm_mdelay(5);
	DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_SetMode(unsigned int mode)
{
	DSI_OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,MODE,mode);

	return DSI_STATUS_OK;
}

DSI_STATUS DSI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
    return DSI_STATUS_ERROR;
}

//not need any more
DSI_STATUS DSI_handle_TE(void)
{

	unsigned int data_array;

	//data_array=0x00351504;
	//DSI_set_cmdq(&data_array, 1, 1);

	//lcm_mdelay(10);

	// RACT
	//data_array=1;
	//OUTREG32(&DSI_REG->DSI_RACK, data_array);

	// TE + BTA
	data_array=0x24;
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI_handle_TE TE + BTA !! \n");
	OUTREG32(&DSI_CMDQ_REG->data, data_array);

	//DSI_CMDQ_REG->data.byte0=0x24;
	//DSI_CMDQ_REG->data.byte1=0;
	//DSI_CMDQ_REG->data.byte2=0;
	//DSI_CMDQ_REG->data.byte3=0;

	//DSI_REG->DSI_CMDQ_SIZE.CMDQ_SIZE=1;
	DSI_OUTREGBIT(DSI_CMDQ_CTRL_REG,DSI_REG->DSI_CMDQ_SIZE,CMDQ_SIZE,1);

	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);

	// wait TE Trigger status
//	do
//	{
		lcm_mdelay(10);
		data_array=DSI_INREG32(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA);

		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI INT state : %x !! \n", data_array);

		data_array=DSI_INREG32(PDSI_TRIG_STA_REG,&DSI_REG->DSI_TRIG_STA);

		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI TRIG TE status check : %x !! \n", data_array);
//	} while(!(data_array&0x4));

	// RACT
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI Set RACT !! \n");
	data_array=1;
	DSI_OUTREG32_V(PDSI_RACK_REG,&DSI_REG->DSI_RACK, data_array);

	return DSI_STATUS_OK;

}

void DSI_Set_VM_CMD(LCM_PARAMS *lcm_params)
{
	DSI_OUTREGBIT(DSI_VM_CMD_CON_REG,DSI_REG->DSI_VM_CMD_CON,TS_VFP_EN,1);
	DSI_OUTREGBIT(DSI_VM_CMD_CON_REG,DSI_REG->DSI_VM_CMD_CON,VM_CMD_EN,1);
}

void DSI_Config_VDO_Timing(LCM_PARAMS *lcm_params)
{
	unsigned int line_byte;
	unsigned int horizontal_sync_active_byte = 0;
	unsigned int horizontal_backporch_byte;
	unsigned int horizontal_frontporch_byte;
	unsigned int horizontal_bllp_byte;
	unsigned int dsiTmpBufBpp;

	#define LINE_PERIOD_US				(8 * line_byte * _dsiContext.bit_time_ns / 1000)

	if(lcm_params->dsi.data_format.format == LCM_DSI_FORMAT_RGB565)
	{
		dsiTmpBufBpp = 2;
	}
	else
	{
		dsiTmpBufBpp = 3;
	}

	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[dsi_drv.c] LK config VDO Timing: %d\n", __LINE__);

	DSI_OUTREG32_V(PDSI_VSA_NL_REG,&DSI_REG->DSI_VSA_NL, lcm_params->dsi.vertical_sync_active);
	DSI_OUTREG32_V(PDSI_VBP_NL_REG,&DSI_REG->DSI_VBP_NL, lcm_params->dsi.vertical_backporch);
	DSI_OUTREG32_V(PDSI_VFP_NL_REG,&DSI_REG->DSI_VFP_NL, lcm_params->dsi.vertical_frontporch);
	DSI_OUTREG32_V(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, lcm_params->dsi.vertical_active_line);

	line_byte							=	(lcm_params->dsi.horizontal_sync_active \
											+ lcm_params->dsi.horizontal_backporch \
											+ lcm_params->dsi.horizontal_frontporch \
											+ lcm_params->dsi.horizontal_active_pixel) * dsiTmpBufBpp;

	if (lcm_params->dsi.mode == SYNC_EVENT_VDO_MODE || lcm_params->dsi.mode == BURST_VDO_MODE )
	{
		ASSERT((lcm_params->dsi.horizontal_backporch + lcm_params->dsi.horizontal_sync_active) * dsiTmpBufBpp> 9);
		horizontal_backporch_byte		=	((lcm_params->dsi.horizontal_backporch + lcm_params->dsi.horizontal_sync_active)* dsiTmpBufBpp - 10);
	}
	else
	{
		ASSERT(lcm_params->dsi.horizontal_sync_active * dsiTmpBufBpp > 9);
		horizontal_sync_active_byte 		=	(lcm_params->dsi.horizontal_sync_active * dsiTmpBufBpp - 10);

		ASSERT(lcm_params->dsi.horizontal_backporch * dsiTmpBufBpp > 9);
		horizontal_backporch_byte		=	(lcm_params->dsi.horizontal_backporch * dsiTmpBufBpp - 10);
	}

	ASSERT(lcm_params->dsi.horizontal_frontporch * dsiTmpBufBpp > 11);
	horizontal_frontporch_byte			=	(lcm_params->dsi.horizontal_frontporch * dsiTmpBufBpp - 12);
	horizontal_bllp_byte				=	(lcm_params->dsi.horizontal_bllp * dsiTmpBufBpp);
//	ASSERT(lcm_params->dsi.horizontal_frontporch * dsiTmpBufBpp > ((300/dsi_cycle_time) * lcm_params->dsi.LANE_NUM));
//	horizontal_frontporch_byte -= ((300/dsi_cycle_time) * lcm_params->dsi.LANE_NUM);

	DSI_OUTREG32_V(PDSI_HSA_WC_REG,&DSI_REG->DSI_HSA_WC, ALIGN_TO((horizontal_sync_active_byte), 4));
	DSI_OUTREG32_V(PDSI_HBP_WC_REG,&DSI_REG->DSI_HBP_WC, ALIGN_TO((horizontal_backporch_byte), 4));
	DSI_OUTREG32_V(PDSI_HFP_WC_REG,&DSI_REG->DSI_HFP_WC, ALIGN_TO((horizontal_frontporch_byte), 4));
	DSI_OUTREG32_V(PDSI_BLLP_WC_REG,&DSI_REG->DSI_BLLP_WC, ALIGN_TO((horizontal_bllp_byte), 4));

	_dsiContext.vfp_period_us 		= LINE_PERIOD_US * lcm_params->dsi.vertical_frontporch / 1000;
	_dsiContext.vsa_vs_period_us	= LINE_PERIOD_US * 1 / 1000;
	_dsiContext.vsa_hs_period_us	= LINE_PERIOD_US * (lcm_params->dsi.vertical_sync_active - 2) / 1000;
	_dsiContext.vsa_ve_period_us	= LINE_PERIOD_US * 1 / 1000;
	_dsiContext.vbp_period_us		= LINE_PERIOD_US * lcm_params->dsi.vertical_backporch / 1000;

	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - video timing, mode = %d \n", lcm_params->dsi.mode);
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VSA : %d %d(us)\n", DSI_REG->DSI_VSA_NL, (_dsiContext.vsa_vs_period_us+_dsiContext.vsa_hs_period_us+_dsiContext.vsa_ve_period_us));
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VBP : %d %d(us)\n", DSI_REG->DSI_VBP_NL, _dsiContext.vbp_period_us);
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VFP : %d %d(us)\n", DSI_REG->DSI_VFP_NL, _dsiContext.vfp_period_us);
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VACT: %d \n", DSI_REG->DSI_VACT_NL);
}

DSI_STATUS DSI_EnableVM_CMD(void)
{
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,VM_CMD_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,VM_CMD_START,1);
    return DSI_STATUS_OK;
}

void DSI_PHY_clk_switch(bool on)
{
	if(on)
	{
		_DSI_PHY_clk_setting(lcm_params);
	}
	else
	{
		// pre_oe/oe = 1
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNTC_LPTX_PRE_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNTC_LPTX_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT0_LPTX_PRE_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT0_LPTX_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT1_LPTX_PRE_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT1_LPTX_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT2_LPTX_PRE_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT2_LPTX_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT3_LPTX_PRE_OE,1);
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0,SW_LNT3_LPTX_OE,1);

		// switch to mipi tx sw mode
		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL,SW_CTRL_EN,1);

		// disable mipi clock
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PLL_EN,0);
		mdelay(1);
		DSI_OUTREGBIT(MIPITX_DSI_PLL_TOP_REG, DSI_PHY_REG->MIPITX_DSI_PLL_TOP, RG_MPPLL_PRESERVE_L, 0);

		DSI_OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_PAD_TIE_LOW_EN, 0);
		DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_LDOOUT_EN,0);
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_LDOOUT_EN,0);
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_LDOOUT_EN,0);
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_LDOOUT_EN,0);
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE3_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE3,RG_DSI0_LNT3_LDOOUT_EN,0);
		mdelay(1);

		DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG, DSI_PHY_REG->MIPITX_DSI_PLL_PWR, DA_DSI0_MPPLL_SDM_ISO_EN, 1);
		DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG, DSI_PHY_REG->MIPITX_DSI_PLL_PWR, DA_DSI0_MPPLL_SDM_PWR_ON, 0);
		DSI_OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_HS_BIAS_EN, 0);

		DSI_OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_CKG_LDOOUT_EN,0);
		DSI_OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_LDOCORE_EN,0);

		DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CKEN,0);
		DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CORE_EN,0);

		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0, RG_DSI0_MPPLL_PREDIV,0);
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0, RG_DSI0_MPPLL_TXDIV0,0);
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0, RG_DSI0_MPPLL_TXDIV1,0);
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0, RG_DSI0_MPPLL_POSDIV,0);


		DSI_OUTREG32_V(PMIPITX_DSI_PLL_CON1_REG,&DSI_PHY_REG->MIPITX_DSI_PLL_CON1, 0x00000000);
		DSI_OUTREG32_V(PMIPITX_DSI_PLL_CON2_REG,&DSI_PHY_REG->MIPITX_DSI_PLL_CON2, 0x50000000);

		DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL,SW_CTRL_EN,0);
		mdelay(1);
	}
}

void DSI_PHY_TIMCONFIG(LCM_PARAMS *lcm_params)
{
	DSI_PHY_TIMCON0_REG timcon0;
	DSI_PHY_TIMCON1_REG timcon1;
	DSI_PHY_TIMCON2_REG timcon2;
	DSI_PHY_TIMCON3_REG timcon3;
	unsigned int div1 = 0;
	unsigned int div2 = 0;
	unsigned int pre_div = 0;
	unsigned int post_div = 0;
	unsigned int fbk_sel = 0;
	unsigned int fbk_div = 0;
	unsigned int lane_no = lcm_params->dsi.LANE_NUM;

	//	unsigned int div2_real;
	unsigned int cycle_time;
	unsigned int ui;
	unsigned int hs_trail_m, hs_trail_n;

	if(0 != lcm_params->dsi.PLL_CLOCK){
		ui= 1000/(lcm_params->dsi.PLL_CLOCK*2)+0x01;
		cycle_time=8000/(lcm_params->dsi.PLL_CLOCK*2)+0x01;
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - _DSI_PHY_TIMCONFIG, Cycle Time = %d(ns), Unit Interval = %d(ns). , lane# = %d \n", cycle_time, ui, lane_no);
	}
	else{
		div1 = lcm_params->dsi.pll_div1;
		div2 = lcm_params->dsi.pll_div2;
		fbk_div = lcm_params->dsi.fbk_div;
   switch(div1)
   {
	  case 0:
		 div1 = 1;
		 break;

	  case 1:
		 div1 = 2;
		 break;

	  case 2:
	  case 3:
		 div1 = 4;
		 break;

	  default:
		 printk("div1 should be less than 4!!\n");
		 div1 = 4;
		 break;
   }

   switch(div2)
   {
	  case 0:
		 div2 = 1;
		 break;

	  case 1:
		 div2 = 2;
		 break;

	  case 2:
	  case 3:
		 div2 = 4;
		 break;

	  default:
		 printk("div2 should be less than 4!!\n");
		 div2 = 4;
		 break;
   }

   switch(pre_div)
   {
	  case 0:
		 pre_div = 1;
		 break;

	  case 1:
		 pre_div = 2;
		 break;

	  case 2:
	  case 3:
		 pre_div = 4;
		 break;

	  default:
		 printk("pre_div should be less than 4!!\n");
		 pre_div = 4;
		 break;
   }

   switch(post_div)
   {
	  case 0:
		 post_div = 1;
		 break;

	  case 1:
		 post_div = 2;
		 break;

	  case 2:
	  case 3:
		 post_div = 4;
		 break;

	  default:
		 printk("post_div should be less than 4!!\n");
		 post_div = 4;
		 break;
   }

   switch(fbk_sel)
   {
	  case 0:
		 fbk_sel = 1;
		 break;

	  case 1:
		 fbk_sel = 2;
		 break;

	  case 2:
	  case 3:
		 fbk_sel = 4;
		 break;

	  default:
		 printk("fbk_sel should be less than 4!!\n");
		 fbk_sel = 4;
		 break;
   }
	cycle_time=(1000*4*div2*div1)/(fbk_div*26)+0x01;

	ui=(1000*div2*div1)/(fbk_div*26*0x2)+0x01;
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, Cycle Time = %d(ns), Unit Interval = %d(ns). div1 = %d, div2 = %d, fbk_div = %d, lane# = %d \n", cycle_time, ui, div1, div2, fbk_div, lane_no);
	}

   //	div2_real=div2 ? div2*0x02 : 0x1;
   //cycle_time = (1000 * div2 * div1 * pre_div * post_div)/ (fbk_sel * (fbk_div+0x01) * 26) + 1;
   //ui = (1000 * div2 * div1 * pre_div * post_div)/ (fbk_sel * (fbk_div+0x01) * 26 * 2) + 1;
#define NS_TO_CYCLE(n, c)	((n) / (c))

   hs_trail_m=1;
   hs_trail_n= (lcm_params->dsi.HS_TRAIL == 0) ? NS_TO_CYCLE(((hs_trail_m * 0x4) + 0x60), cycle_time) : lcm_params->dsi.HS_TRAIL;
   // +3 is recommended from designer becauase of HW latency
   timcon0.HS_TRAIL = ((hs_trail_m > hs_trail_n) ? hs_trail_m : hs_trail_n) + 0x0a;

   timcon0.HS_PRPR	= (lcm_params->dsi.HS_PRPR == 0) ? NS_TO_CYCLE((0x40 + 0x5 * ui), cycle_time) : lcm_params->dsi.HS_PRPR;
   // HS_PRPR can't be 1.
   if (timcon0.HS_PRPR == 0)
	  timcon0.HS_PRPR = 1;

   timcon0.HS_ZERO	= (lcm_params->dsi.HS_ZERO == 0) ? NS_TO_CYCLE((0xC8 + 0x0a * ui), cycle_time) : lcm_params->dsi.HS_ZERO;
   if (timcon0.HS_ZERO > timcon0.HS_PRPR)
	  timcon0.HS_ZERO -= timcon0.HS_PRPR;

   timcon0.LPX		= (lcm_params->dsi.LPX == 0) ? NS_TO_CYCLE(0x50, cycle_time) : lcm_params->dsi.LPX;
   if(timcon0.LPX == 0)
	  timcon0.LPX = 1;

   //	timcon1.TA_SACK 	= (lcm_params->dsi.TA_SACK == 0) ? 1 : lcm_params->dsi.TA_SACK;
   timcon1.TA_GET		= (lcm_params->dsi.TA_GET == 0) ? (0x5 * timcon0.LPX) : lcm_params->dsi.TA_GET;
   timcon1.TA_SURE	= (lcm_params->dsi.TA_SURE == 0) ? (0x3 * timcon0.LPX / 0x2) : lcm_params->dsi.TA_SURE;
   timcon1.TA_GO		= (lcm_params->dsi.TA_GO == 0) ? (0x4 * timcon0.LPX) : lcm_params->dsi.TA_GO;
   // --------------------------------------------------------------
   //  NT35510 need fine tune timing
   //  Data_hs_exit = 60 ns + 128UI
   //  Clk_post = 60 ns + 128 UI.
   // --------------------------------------------------------------
   timcon1.DA_HS_EXIT  = (lcm_params->dsi.DA_HS_EXIT == 0) ? NS_TO_CYCLE((0x3c + 0x80 * ui), cycle_time) : lcm_params->dsi.DA_HS_EXIT;

   timcon2.CLK_TRAIL	= ((lcm_params->dsi.CLK_TRAIL == 0) ? NS_TO_CYCLE(0x64, cycle_time) : lcm_params->dsi.CLK_TRAIL) + 0x0a;
   // CLK_TRAIL can't be 1.
   if (timcon2.CLK_TRAIL < 2)
	  timcon2.CLK_TRAIL = 2;

   //	timcon2.LPX_WAIT	= (lcm_params->dsi.LPX_WAIT == 0) ? 1 : lcm_params->dsi.LPX_WAIT;
   timcon2.CONT_DET 	= lcm_params->dsi.CONT_DET;
   timcon2.CLK_ZERO = (lcm_params->dsi.CLK_ZERO == 0) ? NS_TO_CYCLE(0x190, cycle_time) : lcm_params->dsi.CLK_ZERO;

   timcon3.CLK_HS_PRPR	= (lcm_params->dsi.CLK_HS_PRPR == 0) ? NS_TO_CYCLE(0x40, cycle_time) : lcm_params->dsi.CLK_HS_PRPR;
   if(timcon3.CLK_HS_PRPR == 0)
	  timcon3.CLK_HS_PRPR = 1;
   timcon3.CLK_HS_EXIT= (lcm_params->dsi.CLK_HS_EXIT == 0) ? (2 * timcon0.LPX) : lcm_params->dsi.CLK_HS_EXIT;
   timcon3.CLK_HS_POST= (lcm_params->dsi.CLK_HS_POST == 0) ? NS_TO_CYCLE((0x3c + 0x80 * ui), cycle_time) : lcm_params->dsi.CLK_HS_POST;

   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, HS_TRAIL = %d, HS_ZERO = %d, HS_PRPR = %d, LPX = %d, TA_GET = %d, TA_SURE = %d, TA_GO = %d, CLK_TRAIL = %d, CLK_ZERO = %d, CLK_HS_PRPR = %d \n", \
   timcon0.HS_TRAIL, timcon0.HS_ZERO, timcon0.HS_PRPR, timcon0.LPX, timcon1.TA_GET, timcon1.TA_SURE, timcon1.TA_GO, timcon2.CLK_TRAIL, timcon2.CLK_ZERO, timcon3.CLK_HS_PRPR);

   OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,LPX,timcon0.LPX);
   OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_PRPR,timcon0.HS_PRPR);
   OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_ZERO,timcon0.HS_ZERO);
   OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_TRAIL,timcon0.HS_TRAIL);

   OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GO,timcon1.TA_GO);
   OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_SURE,timcon1.TA_SURE);
   OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GET,timcon1.TA_GET);
   OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,DA_HS_EXIT,timcon1.DA_HS_EXIT);

   OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CONT_DET,timcon2.CONT_DET);
   OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_ZERO,timcon2.CLK_ZERO);
   OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_TRAIL,timcon2.CLK_TRAIL);

   OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_PRPR,timcon3.CLK_HS_PRPR);
   OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_POST,timcon3.CLK_HS_POST);
   OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_EXIT,timcon3.CLK_HS_EXIT);
	printk("%s, 0x%08x,0x%08x,0x%08x,0x%08x\n", __func__, INREG32(DSI_BASE+0x110),INREG32(DSI_BASE+0x114),INREG32(DSI_BASE+0x118),INREG32(DSI_BASE+0x11c));
}


void DSI_clk_ULP_mode(BOOL enter)
{
   if(enter)
   {
      DSI_OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_HS_TX_EN, 0);
      lcm_mdelay(1);

      DSI_OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_ULPM_EN, 1);
      lcm_mdelay(1);
   }
   else
   {
      DSI_OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_ULPM_EN, 0);
      lcm_mdelay(1);

      DSI_OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_WAKEUP_EN, 1);
      lcm_mdelay(1);

      DSI_OUTREGBIT(DSI_PHY_LCCON_REG, DSI_REG->DSI_PHY_LCCON, LC_WAKEUP_EN, 0);
      lcm_mdelay(1);
   }
}


void DSI_clk_HS_mode(BOOL enter)
{
	DSI_PHY_LCCON_REG tmp_reg1 = DSI_REG->DSI_PHY_LCCON;

	if(enter && !DSI_clk_HS_state())
	{
		tmp_reg1.LC_HS_TX_EN=1;
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
	}
	else if (!enter && DSI_clk_HS_state())
	{
		tmp_reg1.LC_HS_TX_EN=0;
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
	}
}

void DSI_Continuous_HS(void)
{
    DSI_TXRX_CTRL_REG tmp_reg = DSI_REG->DSI_TXRX_CTRL;

    tmp_reg.HSTX_CKLP_EN = 0;
    DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&DSI_REG->DSI_TXRX_CTRL, &tmp_reg);
}

BOOL DSI_clk_HS_state(void)
{
	return DSI_REG->DSI_PHY_LCCON.LC_HS_TX_EN ? TRUE : FALSE;
}

void DSI_lane0_ULP_mode(BOOL enter)
{
	DSI_PHY_LD0CON_REG tmp_reg1;

	tmp_reg1=DSI_REG->DSI_PHY_LD0CON;

	if(enter)
	{
		// suspend
		tmp_reg1.L0_HS_TX_EN=0;
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
		lcm_mdelay(1);

		tmp_reg1.L0_ULPM_EN=1;
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
		lcm_mdelay(1);
	}
	else
	{
		// resume
		tmp_reg1.L0_ULPM_EN=0;
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
		lcm_mdelay(1);

		tmp_reg1.L0_WAKEUP_EN=1;
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
		lcm_mdelay(1);

		tmp_reg1.L0_WAKEUP_EN=0;
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
		lcm_mdelay(1);
	}
}

void DSI_set_cmdq_V2(unsigned cmd, unsigned char count, unsigned char *para_list, unsigned char force_update)
{
	UINT32 i;
	UINT32 goto_addr, mask_para, set_para;
	DSI_T0_INS t0;
	DSI_T2_INS t2;

    _WaitForEngineNotBusy();
#if 0
	if (count > 59)
	{
		UINT32 pixel = count/3 + ((count%3) ? 1 : 0);

		LCD_REG_LAYER 	fb_layer_info;
		LCD_REG_DSI_DC	dsi_info;

		// backup layer state.
		layer_state = AS_UINT32(&LCD_REG->WROI_CONTROL) & 0xFC000000;

		// backup FB layer info.
		memcpy((void *)&fb_layer_info, (void *)&LCD_REG->LAYER[FB_LAYER], sizeof(LCD_REG_LAYER));

		// backup LCD-DSI I/F configuration.
		dsi_info = LCD_REG->DS_DSI_CON;

		// backup lane number.
		lane_num = DSI_REG->DSI_TXRX_CTRL.LANE_NUM;

		// HW limitation.
		// LP type-1 command can't go with 2 lanes. So we must switch to lane mode.
		DSI_REG->DSI_TXRX_CTRL.LANE_NUM = 1;
		DSI_PHY_REG->MIPITX_CON1.RG_DSI_CK_SEL = 0;

		// Modify LCD-DSI configuration
		LCD_REG->DS_DSI_CON.DC_DSI = TRUE;
		// Let LSB of RGB(BGR in buffer) first.
		LCD_REG->DS_DSI_CON.RGB_SWAP = LCD_DSI_IF_FMT_COLOR_ORDER_BGR;
		// Let parameters be in unit of byte.
		LCD_REG->DS_DSI_CON.CLR_FMT = LCD_DSI_IF_FORMAT_RGB888;
		// HW limitation
		// It makes package numbers > 1.
		LCD_REG->DS_DSI_CON.PACKET_SIZE = 30;

		// Start of Enable only one layer (FB layer) to push data to DSI
		LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_ALL, FALSE));
		LCD_CHECK_RET(LCD_LayerEnable(FB_LAYER, TRUE));
		LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, pixel, 1));
		LCD_CHECK_RET(LCD_SetBackgroundColor(0));

		// operates on FB layer
		{
		    extern void disp_get_fb_address(UINT32 *fbVirAddr, UINT32 *fbPhysAddr);
            disp_get_fb_address(&fbVirAddr ,&fbPhysAddr);

		    // copy parameters to FB layer buffer.
		    memcpy((void *)fbVirAddr, (void *)para_list, count);
		    LCD_REG->LAYER[FB_LAYER].ADDRESS = fbPhysAddr;
		}

		LCD_CHECK_RET(LCD_LayerSetFormat(FB_LAYER, LCD_LAYER_FORMAT_RGB888));
		LCD_CHECK_RET(LCD_LayerSetPitch(FB_LAYER, pixel*3));
		LCD_CHECK_RET(LCD_LayerSetOffset(FB_LAYER, 0, 0));
		LCD_CHECK_RET(LCD_LayerSetSize(FB_LAYER, pixel, 1));
		// End of Enable only one layer (FB layer) to push data to DSI

		t1.CONFG = 1;
		t1.Data_ID = DSI_DCS_LONG_PACKET_ID;
		t1.mem_start0 = (cmd&0xFF);
		t1.mem_start1 = (cmd>>8);

		OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t1));
		OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);

		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_set_cmdq_V2. command(0x%x) parameter count = %d > 59, pixel = %d \n", cmd, count, pixel);
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - command queue only support 16 x 4 bytes. Header used 4 byte. DCS used 1 byte. If parameter > 59 byte, work around by Type-1 command. \n");
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "para_list[%d] = {", count);
	    for (i = 0; i < count; i++)
	        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "0x%02x, ", para_list[i]);
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "} \n");

#if defined(MTK_M4U_SUPPORT)
		LCD_M4U_On(0);
#endif
		if(force_update)
		{
			LCD_CHECK_RET(LCD_StartTransfer(FALSE));
			DSI_EnableClk();
		}

		_WaitForEngineNotBusy();
#if defined(MTK_M4U_SUPPORT)
		LCD_M4U_On(1);
#endif
		// restore FB layer info.
		memcpy((void *)&LCD_REG->LAYER[FB_LAYER], (void *)&fb_layer_info, sizeof(LCD_REG_LAYER));

		// restore LCD-DSI I/F configuration.
		LCD_REG->DS_DSI_CON = dsi_info;

		// restore lane number.
		DSI_REG->DSI_TXRX_CTRL.LANE_NUM = lane_num;
		DSI_PHY_REG->MIPITX_CON1.RG_DSI_CK_SEL = (lane_num - 1);

		// restore layer state.
		for(layer=LCD_LAYER_0; layer<LCD_LAYER_NUM; layer++)
		{
			if(layer_state&(0x80000000>>layer))
				LCD_CHECK_RET(LCD_LayerEnable(layer, TRUE));
			else
				LCD_CHECK_RET(LCD_LayerEnable(layer, FALSE));
		}

	}
	else
#endif

	if (cmd < 0xB0)
	{
		if (count > 1)
		{
			t2.CONFG = 2;
			t2.Data_ID = DSI_DCS_LONG_PACKET_ID;
			t2.WC16 = count+1;

			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t2);

			goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
			mask_para = (0xFF<<((goto_addr&0x3)*8));
			set_para = (cmd<<((goto_addr&0x3)*8));
			MASKREG32(goto_addr&(~0x3), mask_para, set_para);

			for(i=0; i<count; i++)
			{
				goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (para_list[i]<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);
			}

			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);
		}
		else
		{
			t0.CONFG = 0;
			t0.Data0 = cmd;
			if (count)
			{
				t0.Data_ID = DSI_DCS_SHORT_PACKET_ID_1;
				t0.Data1 = para_list[0];
			}
			else
			{
				t0.Data_ID = DSI_DCS_SHORT_PACKET_ID_0;
				t0.Data1 = 0;
			}
			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

		}
	}
	else
	{
		if (count > 1)
		{
			t2.CONFG = 2;
			t2.Data_ID = DSI_GERNERIC_LONG_PACKET_ID;
			t2.WC16 = count+1;

			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t2);

			goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
			mask_para = (0xFF<<((goto_addr&0x3)*8));
			set_para = (cmd<<((goto_addr&0x3)*8));
			MASKREG32(goto_addr&(~0x3), mask_para, set_para);

			for(i=0; i<count; i++)
			{
				goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (para_list[i]<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);
			}

			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);

		}
		else
		{
			t0.CONFG = 0;
			t0.Data0 = cmd;
			if (count)
			{
				t0.Data_ID = DSI_GERNERIC_SHORT_PACKET_ID_2;
				t0.Data1 = para_list[0];
			}
			else
			{
				t0.Data_ID = DSI_GERNERIC_SHORT_PACKET_ID_1;
				t0.Data1 = 0;
			}
			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
		}
	}

	//for (i = 0; i < AS_UINT32(&DSI_REG->DSI_CMDQ_SIZE); i++)
    //    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V2. DSI_CMDQ+%04x : 0x%08x\n", i*4, INREG32(DSI_BASE + 0x180 + i*4));

	if(force_update)
	{
		DSI_Start();
	}


}

void DSI_set_cmdq_V3(LCM_setting_table_V3 *para_tbl, unsigned int size, unsigned char force_update)
{
	UINT32 i;
	UINT32 goto_addr, mask_para, set_para;
	DSI_T0_INS t0;
	DSI_T2_INS t2;
	UINT32 index = 0;
	unsigned char data_id, cmd, count;
	unsigned char *para_list;

	do {
		data_id = para_tbl[index].id;
		cmd = para_tbl[index].cmd;
		count = para_tbl[index].count;
		para_list = para_tbl[index].para_list;

		if (data_id == REGFLAG_ESCAPE_ID && cmd == REGFLAG_DELAY_MS_V3)
		{
			udelay(1000*count);
			DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V3[%d]. Delay %d (ms) \n", index, count);

			continue;
		}

		_WaitForEngineNotBusy();
#if 0
		if (count > 59)
		{
			UINT32 pixel = count/3 + ((count%3) ? 1 : 0);

			LCD_REG_LAYER	fb_layer_info;
			LCD_REG_DSI_DC	dsi_info;
			BOOL			lcd_w2m, lcm_te_enable;

			// backup layer state.
			layer_state = AS_UINT32(&LCD_REG->WROI_CONTROL) & 0xFC000000;

			// backup FB layer info.
			memcpy((void *)&fb_layer_info, (void *)&LCD_REG->LAYER[FB_LAYER], sizeof(LCD_REG_LAYER));

			// backup LCD-DSI I/F configuration.
			dsi_info = LCD_REG->DS_DSI_CON;

			// backup lane number.
			lane_num = DSI_REG->DSI_TXRX_CTRL.LANE_NUM;

			// backup lcd_w2m
			lcd_w2m = LCD_REG->WROI_CONTROL.W2M;

			// backup TE enable
			lcm_te_enable = LCD_REG->TEARING_CFG.ENABLE;

			// HW limitation.
			// LP type-1 command can't go with 2 lanes. So we must switch to lane mode.
			DSI_REG->DSI_TXRX_CTRL.LANE_NUM = 1;
			DSI_PHY_REG->MIPITX_CON1.RG_DSI_CK_SEL = 0;

			// Modify LCD-DSI configuration
			LCD_REG->DS_DSI_CON.DC_DSI = TRUE;
			// Let LSB of RGB(BGR in buffer) first.
			LCD_REG->DS_DSI_CON.RGB_SWAP = LCD_DSI_IF_FMT_COLOR_ORDER_BGR;
			// Let parameters be in unit of byte.
			LCD_REG->DS_DSI_CON.CLR_FMT = LCD_DSI_IF_FORMAT_RGB888;
			// Disable W2M
			LCD_REG->WROI_CONTROL.W2M = 0;

			// HW limitation
			// It makes package numbers > 1.
			LCD_REG->DS_DSI_CON.PACKET_SIZE = 256;

			// Disable TE
			LCD_TE_Enable(0);

			// Start of Enable only one layer (FB layer) to push data to DSI
			LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_ALL, FALSE));
			LCD_CHECK_RET(LCD_LayerEnable(FB_LAYER, TRUE));
			LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, pixel, 1));
			LCD_CHECK_RET(LCD_SetBackgroundColor(0));
#ifdef BUILD_LK
			LCD_REG->LAYER[FB_LAYER].ADDRESS = para_list;
#else
			// operates on FB layer
			{
				extern void disp_get_fb_address(UINT32 *fbVirAddr, UINT32 *fbPhysAddr);
				disp_get_fb_address(&fbVirAddr ,&fbPhysAddr);

				// copy parameters to FB layer buffer.
				memcpy((void *)fbVirAddr, (void *)para_list, count);
				//LCD_REG->LAYER[FB_LAYER].ADDRESS = fbPhysAddr;
			}
#endif
			LCD_CHECK_RET(LCD_LayerSetFormat(FB_LAYER, LCD_LAYER_FORMAT_RGB888));
			LCD_CHECK_RET(LCD_LayerSetPitch(FB_LAYER, pixel*3));
			LCD_CHECK_RET(LCD_LayerSetOffset(FB_LAYER, 0, 0));
			LCD_CHECK_RET(LCD_LayerSetSize(FB_LAYER, pixel, 1));
			// End of Enable only one layer (FB layer) to push data to DSI

			t1.CONFG = 1;
			t1.Data_ID = data_id;
			t1.mem_start0 = (cmd&0xFF);
			t1.mem_start1 = (cmd>>8);

			OUTREG32(&DSI_CMDQ_REG->data0[0], AS_UINT32(&t1));
			OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);

			DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_set_cmdq_V3[%d]. command(0x%x) parameter count = %d > 59, pixel = %d \n", index, cmd, count, pixel);
			DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - command queue only support 16 x 4 bytes. Header used 4 byte. DCS used 1 byte. If parameter > 59 byte, work around by Type-1 command. \n");
			DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "para_list[%d] = {", count);
			for (i = 0; i < count; i++)
				DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "0x%02x, ", para_list[i]);
			DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "} \n");

#ifdef BUILD_LK
			if(force_update)
			{
				LCD_CHECK_RET(LCD_StartTransfer(FALSE));
				DSI_EnableClk();

				while(DSI_REG->DSI_STA.BUSY || DSI_REG->DSI_STATE_DBG4.EXE_STATE != 1) {
					printk("[DISP - uboot - DSI Busy : %d, DSI_STATE_DBG4.EXE_STATE = %d, LCD Busy : %d \n", DSI_REG->DSI_STA.BUSY, DSI_REG->DSI_STATE_DBG4.EXE_STATE, LCD_REG->STATUS.BUSY);
					if (DSI_REG->DSI_STATE_DBG4.EXE_STATE == 2 && !LCD_REG->STATUS.BUSY) {
						DSI_Reset();
					}
				}

			}
#else
			//LCD_M4U_On(0);

			if(force_update)
			{
				LCD_CHECK_RET(LCD_StartTransfer(FALSE));
				DSI_EnableClk();

				while (_IsEngineBusy())
				{
					long ret = wait_event_interruptible_timeout(_dsi_wait_queue,
																!_IsEngineBusy(),
																1 * HZ);
					if (0 == ret) {
						DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "[%s] Wait for DSI engine not busy timeout!!! Busy : %d, DSI_STATE_DBG4.EXE_STATE = %d \n", __func__, DSI_REG->DSI_STA.BUSY, DSI_REG->DSI_STATE_DBG4.EXE_STATE);
						if (DSI_REG->DSI_STATE_DBG4.EXE_STATE == 2) {
							DSI_Reset();
						}
					}
				}
			}

			//LCD_M4U_On(1);
#endif
			// restore FB layer info.
			memcpy((void *)&LCD_REG->LAYER[FB_LAYER], (void *)&fb_layer_info, sizeof(LCD_REG_LAYER));

			// restore LCD-DSI I/F configuration.
			LCD_REG->DS_DSI_CON = dsi_info;

			// restore lane number.
			DSI_REG->DSI_TXRX_CTRL.LANE_NUM = lane_num;
			DSI_PHY_REG->MIPITX_CON1.RG_DSI_CK_SEL = (lane_num - 1);

			// restore layer state.
			for(layer=LCD_LAYER_0; layer<LCD_LAYER_NUM; layer++)
			{
				if(layer_state&(0x80000000>>layer))
					LCD_CHECK_RET(LCD_LayerEnable(layer, TRUE));
				else
					LCD_CHECK_RET(LCD_LayerEnable(layer, FALSE));
			}

			// restore lcd_w2m
			if (lcd_w2m)
				LCD_REG->WROI_CONTROL.W2M = 1;

			// restore TE
			if(lcm_te_enable)
				LCD_TE_Enable(1);

		}
		else
#endif

		//for(i = 0; i < sizeof(DSI_CMDQ_REG->data0) / sizeof(DSI_CMDQ); i++)
		//	OUTREG32(&DSI_CMDQ_REG->data0[i], 0);
		//memset(&DSI_CMDQ_REG->data[0], 0, sizeof(DSI_CMDQ_REG->data[0]));
		OUTREG32(&DSI_CMDQ_REG->data[0], 0);

		if (count > 1)
		{
			t2.CONFG = 2;
			t2.Data_ID = data_id;
			t2.WC16 = count+1;

			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t2);

			goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
			mask_para = (0xFF<<((goto_addr&0x3)*8));
			set_para = (cmd<<((goto_addr&0x3)*8));
			MASKREG32(goto_addr&(~0x3), mask_para, set_para);

			for(i=0; i<count; i++)
			{
				goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (para_list[i]<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);
			}
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);

		}
		else
		{
			t0.CONFG = 0;
			t0.Data0 = cmd;
			if (count)
			{
				t0.Data_ID = data_id;
				t0.Data1 = para_list[0];
			}
			else
			{
				t0.Data_ID = data_id;
				t0.Data1 = 0;
			}
			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

		}

		//for (i = 0; i < AS_UINT32(&DSI_REG->DSI_CMDQ_SIZE); i++)
		//	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V3[%d]. DSI_CMDQ+%04x : 0x%08x\n", index, i*4, INREG32(DSI_BASE + 0x180 + i*4));

		if(force_update)
		{
			DSI_Start();
		}


	} while (++index < size);

}

void DSI_set_cmdq(unsigned int *pdata, unsigned int queue_size, unsigned char force_update)
{
	UINT32 i;

	ASSERT(queue_size<=16);

    _WaitForEngineNotBusy();

	for(i=0; i<queue_size; i++)
	{
		OUTREG32(&DSI_CMDQ_REG->data[i], AS_UINT32((pdata+i)));
	}

	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, queue_size);

    //for (i = 0; i < queue_size; i++)
     //   printf("[DISP] - kernel - DSI_set_cmdq. DSI_CMDQ+%04x : 0x%08x\n", i*4, INREG32(DSI_BASE + 0x180 + i*4));
	if(force_update)
	{
		DSI_Start();
		_WaitForEngineNotBusy();
	}
}


DSI_STATUS DSI_Write_T0_INS(DSI_T0_INS *t0)
{
    OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t0));
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);

	return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T1_INS(DSI_T1_INS *t1)
{
    OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t1));
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);

	return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T2_INS(DSI_T2_INS *t2)
{
	unsigned int i;

	OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t2));

	for(i=0;i<(unsigned int)((t2->WC16-1)>>2)+1;i++)
	{
	    OUTREG32(&DSI_CMDQ_REG->data[1+i], AS_UINT32((t2->pdata+i)));
	}

	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, (((t2->WC16-1)>>2)+2));
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);

	return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T3_INS(DSI_T3_INS *t3)
{
    OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t3));
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);

	return DSI_STATUS_OK;
}

DSI_STATUS DSI_TXRX_Control(BOOL cksm_en,
                                  BOOL ecc_en,
                                  unsigned char lane_num,
                                  unsigned char vc_num,
                                  BOOL null_packet_en,
                                  BOOL err_correction_en,
                                  BOOL dis_eotp_en,
								  BOOL hstx_cklp_en,
                                  unsigned int  max_return_size)
{
    DSI_TXRX_CTRL_REG tmp_reg;

    tmp_reg=DSI_REG->DSI_TXRX_CTRL;

//    tmp_reg.CKSM_EN=cksm_en;
//    tmp_reg.ECC_EN=ecc_en;
	switch(lane_num)
	{
		case LCM_ONE_LANE:tmp_reg.LANE_NUM = 1;break;
		case LCM_TWO_LANE:tmp_reg.LANE_NUM = 3;break;
		case LCM_THREE_LANE:tmp_reg.LANE_NUM = 0x7;break;
		case LCM_FOUR_LANE:tmp_reg.LANE_NUM = 0xF;break;
	}
    tmp_reg.VC_NUM=vc_num;
//    tmp_reg.CORR_EN = err_correction_en;
    tmp_reg.DIS_EOT = dis_eotp_en;
    tmp_reg.NULL_EN = null_packet_en;
    tmp_reg.MAX_RTN_SIZE = max_return_size;
	tmp_reg.HSTX_CKLP_EN = hstx_cklp_en;

	//EXT TE Setting
	if(EXT_TE == dsiTeMode)
	{
		tmp_reg.EXT_TE_EN   = 1;
		tmp_reg.EXT_TE_EDGE = 0;
	}


	DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&DSI_REG->DSI_TXRX_CTRL, &tmp_reg);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PS_Control(unsigned int ps_type, unsigned int vact_line, unsigned int ps_wc)
{
    DSI_PSCTRL_REG tmp_reg;
	UINT32 tmp_hstx_cklp_wc;
    tmp_reg=DSI_REG->DSI_PSCTRL;

    ASSERT(ps_type <= PACKED_PS_18BIT_RGB666);
	if(ps_type>LOOSELY_PS_18BIT_RGB666)
	{
    	tmp_reg.DSI_PS_SEL=(5 - ps_type);
    }
	else
	{
		tmp_reg.DSI_PS_SEL=ps_type;
	}
    tmp_reg.DSI_PS_WC=ps_wc;
    tmp_hstx_cklp_wc = ps_wc;

	DSI_OUTREG32_V(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, AS_UINT32(&vact_line));
	DSI_OUTREG32_R(PDSI_PSCTRL_REG,&DSI_REG->DSI_PSCTRL, &tmp_reg);

	OUTREG32(&DSI_REG->DSI_HSTX_CKL_WC, tmp_hstx_cklp_wc);
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_ConfigVDOTiming(unsigned int dsi_vsa_nl,
                                    unsigned int dsi_vbp_nl,
                                    unsigned int dsi_vfp_nl,
                                    unsigned int dsi_vact_nl,
                                    unsigned int dsi_line_nb,
                                    unsigned int dsi_hsa_nb,
                                    unsigned int dsi_hbp_nb,
                                    unsigned int dsi_hfp_nb,
                                    unsigned int dsi_rgb_nb,
                                    unsigned int dsi_hsa_wc,
                                    unsigned int dsi_hbp_wc,
                                    unsigned int dsi_hfp_wc)
{
	DSI_OUTREG32_V(PDSI_VSA_NL_REG,&DSI_REG->DSI_VSA_NL, AS_UINT32(&dsi_vsa_nl));
	DSI_OUTREG32_V(PDSI_VBP_NL_REG,&DSI_REG->DSI_VBP_NL, AS_UINT32(&dsi_vbp_nl));
	DSI_OUTREG32_V(PDSI_VFP_NL_REG,&DSI_REG->DSI_VFP_NL, AS_UINT32(&dsi_vfp_nl));
	DSI_OUTREG32_V(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, AS_UINT32(&dsi_vact_nl));

	DSI_OUTREG32_V(PDSI_HSA_WC_REG,&DSI_REG->DSI_HSA_WC, AS_UINT32(&dsi_hsa_wc));
	DSI_OUTREG32_V(PDSI_HBP_WC_REG,&DSI_REG->DSI_HBP_WC, AS_UINT32(&dsi_hbp_wc));
	DSI_OUTREG32_V(PDSI_HFP_WC_REG,&DSI_REG->DSI_HFP_WC, AS_UINT32(&dsi_hfp_wc));
    return DSI_STATUS_OK;
}

void DSI_write_lcm_cmd(unsigned int cmd)
{
	DSI_T0_INS t0_tmp;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=SHORT_PACKET_RW;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=LOW_POWER;
	CONFG_tmp.CL=CL_8BITS;
	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;

	t0_tmp.CONFG = *((unsigned char *)(&CONFG_tmp));
	t0_tmp.Data_ID= (cmd&0xFF);
	t0_tmp.Data0 = 0x0;
	t0_tmp.Data1 = 0x0;

	DSI_Write_T0_INS(&t0_tmp);
}


void DSI_write_lcm_regs(unsigned int addr, unsigned int *para, unsigned int nums)
{
	DSI_T2_INS t2_tmp;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=LONG_PACKET_W;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=LOW_POWER;
	CONFG_tmp.CL=CL_8BITS;
	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;

	t2_tmp.CONFG = *((unsigned char *)(&CONFG_tmp));
	t2_tmp.Data_ID = (addr&0xFF);
	t2_tmp.WC16 = nums;
	t2_tmp.pdata = para;

	DSI_Write_T2_INS(&t2_tmp);

}

UINT32 DSI_dcs_read_lcm_reg(UINT8 cmd)
{
    UINT32 recv_data = 0;
#if 0
    DSI_T0_INS t0;
#if ENABLE_DSI_INTERRUPT
    static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
    long ret;
#endif

	if (DSI_REG->DSI_MODE_CTRL.MODE)
		return 0;

    do
    {
       if(max_try_count == 0)
          return 0;

       max_try_count--;
       recv_data = 0;
       recv_data_cnt = 0;
       read_timeout_ms = 20;

       _WaitForEngineNotBusy();

       t0.CONFG = 0x04;        ///BTA
       t0.Data0 = cmd;
       t0.Data_ID = DSI_DCS_READ_PACKET_ID;
       t0.Data1 = 0;

       OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
       OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);

       ///clear read ACK
       DSI_REG->DSI_RACK.DSI_RACK = 1;
       DSI_REG->DSI_INTSTA.RD_RDY = 1;
       DSI_REG->DSI_INTSTA.CMD_DONE = 1;
       DSI_REG->DSI_INTEN.RD_RDY =  1;
       DSI_REG->DSI_INTEN.CMD_DONE=  1;

       OUTREG32(&DSI_REG->DSI_START, 0);
       OUTREG32(&DSI_REG->DSI_START, 1);

       /// the following code is to
       /// 1: wait read ready
       /// 2: ack read ready
       /// 3: wait for CMDQ_DONE
       /// 3: read data
#if ENABLE_DSI_INTERRUPT
        ret = wait_event_interruptible_timeout(_dsi_dcs_read_wait_queue,
                                                       !_IsEngineBusy(),
                                                       WAIT_TIMEOUT);
        if (0 == ret) {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine read ready timeout!!!\n");

				DSI_DumpRegisters();

				///do necessary reset here
				DSI_REG->DSI_RACK.DSI_RACK = 1;
				DSI_Reset();

                return 0;
            }
#else
    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Start polling DSI read ready!!!\n");
    #endif
        while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
        {
            ///keep polling
            msleep(1);
            read_timeout_ms --;

            if(read_timeout_ms == 0)
            {
                DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Polling DSI read ready timeout!!!\n");
                DSI_DumpRegisters();

                ///do necessary reset here
                DSI_REG->DSI_RACK.DSI_RACK = 1;
                DSI_Reset();
                return 0;
            }
        }
    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " End polling DSI read ready!!!\n");
    #endif

        DSI_REG->DSI_RACK.DSI_RACK = 1;

       while(DSI_REG->DSI_STA.BUF_UNDERRUN || DSI_REG->DSI_STA.ESC_ENTRY_ERR || DSI_REG->DSI_STA.LPDT_SYNC_ERR || DSI_REG->DSI_STA.CTRL_ERR || DSI_REG->DSI_STA.CONTENT_ERR)
       {
           ///DSI READ ACK HW bug workaround
    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
           DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI is busy: 0x%x !!!\n", DSI_REG->DSI_STA.BUSY);
    #endif
           DSI_REG->DSI_RACK.DSI_RACK = 1;
       }


       ///clear interrupt status
       DSI_REG->DSI_INTSTA.RD_RDY = 1;
       ///STOP DSI
       OUTREG32(&DSI_REG->DSI_START, 0);

#endif

       DSI_REG->DSI_INTEN.RD_RDY =  0;

    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_STA : 0x%x \n", DSI_REG->DSI_RX_STA);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_SIZE : 0x%x \n", DSI_REG->DSI_CMDQ_SIZE.CMDQ_SIZE);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA0 : 0x%x \n", DSI_CMDQ_REG->data[0].byte0);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA1 : 0x%x \n", DSI_CMDQ_REG->data[0].byte1);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA2 : 0x%x \n", DSI_CMDQ_REG->data[0].byte2);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA3 : 0x%x \n", DSI_CMDQ_REG->data[0].byte3);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE0 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE0);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE1 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE1);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE2 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE2);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE3 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE3);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE4 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE4);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE5 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE5);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE6 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE6);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE7 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE7);
    #endif
       packet_type = DSI_REG->DSI_RX_DATA.BYTE0;

    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI read packet_type is 0x%x \n",packet_type);
    #endif
       if(DSI_REG->DSI_RX_STA.LONG == 1)
       {
           recv_data_cnt = DSI_REG->DSI_RX_DATA.BYTE1 + DSI_REG->DSI_RX_DATA.BYTE2 * 16;
           if(recv_data_cnt > 4)
           {
    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
              DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read long packet data  exceeds 4 bytes \n");
    #endif
              recv_data_cnt = 4;
           }
           memcpy((void*)&recv_data, (void*)&DSI_REG->DSI_RX_DATA.BYTE4, recv_data_cnt);
       }
       else
       {
           memcpy((void*)&recv_data, (void*)&DSI_REG->DSI_RX_DATA.BYTE1, 2);
       }

    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI read 0x%x data is 0x%x \n",cmd,  recv_data);
    #endif
   }while(packet_type != 0x1C && packet_type != 0x21 && packet_type != 0x22);
   /// here: we may receive a ACK packet which packet type is 0x02 (incdicates some error happened)
   /// therefore we try re-read again until no ACK packet
   /// But: if it is a good way to keep re-trying ???
#endif
   return recv_data;
}

/// return value: the data length we got
UINT32 DSI_dcs_read_lcm_reg_v2(UINT8 cmd, UINT8 *buffer, UINT8 buffer_size)
{
    UINT32 max_try_count = 5;
    UINT32 recv_data_cnt;
    unsigned int read_timeout_ms;
    unsigned char packet_type;
	DSI_RX_DATA_REG read_data0;
	DSI_RX_DATA_REG read_data1;
	DSI_RX_DATA_REG read_data2;
	DSI_RX_DATA_REG read_data3;
#if 1
    DSI_T0_INS t0;

	if (DSI_REG->DSI_MODE_CTRL.MODE)
		return 0;

    if (buffer == NULL || buffer_size == 0)
        return 0;

do
    {
       if(max_try_count == 0)
	  return 0;
       max_try_count--;
       recv_data_cnt = 0;
       read_timeout_ms = 20;

       _WaitForEngineNotBusy();

       t0.CONFG = 0x04;        ///BTA
       t0.Data0 = cmd;
       if (buffer_size < 0x3)
           t0.Data_ID = DSI_DCS_READ_PACKET_ID;
       else
           t0.Data_ID = DSI_GERNERIC_READ_LONG_PACKET_ID;
       t0.Data1 = 0;

		DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
		DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

		DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);
		DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
		DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);

		DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
		DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);

       /// the following code is to
       /// 1: wait read ready
       /// 2: ack read ready
       /// 3: wait for CMDQ_DONE
       /// 3: read data

    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Start polling DSI read ready!!!\n");
    #endif
        while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
        {
            ///keep polling
            msleep(1);
            read_timeout_ms --;

            if(read_timeout_ms == 0)
            {
                DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Polling DSI read ready timeout!!!\n");
                DSI_DumpRegisters();

                ///do necessary reset here
				DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);

                DSI_Reset();
                return 0;
            }
        }
    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " End polling DSI read ready!!!\n");
    #endif

		DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
		DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);

		DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);

		OUTREG32(&read_data0, AS_UINT32(&DSI_REG->DSI_RX_DATA0));
		OUTREG32(&read_data1, AS_UINT32(&DSI_REG->DSI_RX_DATA1));
		OUTREG32(&read_data2, AS_UINT32(&DSI_REG->DSI_RX_DATA2));
		OUTREG32(&read_data3, AS_UINT32(&DSI_REG->DSI_RX_DATA3));
    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
    {
       unsigned int i;
//       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_STA : 0x%x \n", DSI_REG->DSI_RX_STA);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_SIZE : 0x%x \n", DSI_REG->DSI_CMDQ_SIZE.CMDQ_SIZE);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA0 : 0x%x \n", DSI_CMDQ_REG->data[0].byte0);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA1 : 0x%x \n", DSI_CMDQ_REG->data[0].byte1);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA2 : 0x%x \n", DSI_CMDQ_REG->data[0].byte2);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA3 : 0x%x \n", DSI_CMDQ_REG->data[0].byte3);
#if 1
	   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA0 : 0x%x \n", DSI_REG->DSI_RX_DATA0);
	   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA1 : 0x%x \n", DSI_REG->DSI_RX_DATA1);
	   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA2 : 0x%x \n", DSI_REG->DSI_RX_DATA2);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA3 : 0x%x \n", DSI_REG->DSI_RX_DATA3);

       printf("read_data0, %x,%x,%x,%x\n", read_data0.byte0, read_data0.byte1, read_data0.byte2, read_data0.byte3);
	   printf("read_data1, %x,%x,%x,%x\n", read_data1.byte0, read_data1.byte1, read_data1.byte2, read_data1.byte3);
	   printf("read_data2, %x,%x,%x,%x\n", read_data2.byte0, read_data2.byte1, read_data2.byte2, read_data2.byte3);
	   printf("read_data3, %x,%x,%x,%x\n", read_data3.byte0, read_data3.byte1, read_data3.byte2, read_data3.byte3);
#endif
    }
    #endif

#if 1
	packet_type = read_data0.byte0;

    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI read packet_type is 0x%x \n",packet_type);
    #endif



	if(packet_type == 0x1A || packet_type == 0x1C)
    {
    	recv_data_cnt = read_data0.byte1 + read_data0.byte2 * 16;
		if(recv_data_cnt > 10)
        {
    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read long packet data  exceeds 4 bytes \n");
    #endif
            recv_data_cnt = 10;
         }

          if(recv_data_cnt > buffer_size)
          {
#ifdef DDI_DRV_DEBUG_LOG_ENABLE
              DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read long packet data  exceeds buffer size: %d\n", buffer_size);
#endif
              recv_data_cnt = buffer_size;
           }
#ifdef DDI_DRV_DEBUG_LOG_ENABLE
          DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read long packet size: %d\n", recv_data_cnt);
#endif
           memcpy((void*)buffer, (void*)&read_data1, recv_data_cnt);
       }
       else
       {
             if(recv_data_cnt > buffer_size)
             {
#ifdef DDI_DRV_DEBUG_LOG_ENABLE
                 DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read short packet data  exceeds buffer size: %d\n", buffer_size);
#endif
                 recv_data_cnt = buffer_size;
             }
           memcpy((void*)buffer,(void*)&read_data0.byte1, 2);
       }
#endif
   }while(packet_type != 0x1C && packet_type != 0x21 && packet_type != 0x22 && packet_type != 0x1A);
   /// here: we may receive a ACK packet which packet type is 0x02 (incdicates some error happened)
   /// therefore we try re-read again until no ACK packet
   /// But: if it is a good way to keep re-trying ???
#endif
   return recv_data_cnt;
}

UINT32 DSI_read_lcm_reg()
{
    return 0;
}


DSI_STATUS DSI_write_lcm_fb(unsigned int addr, BOOL long_length)
{
	DSI_T1_INS t1_tmp;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=FB_WRITE;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=HIGH_SPEED;

	if(long_length)
	{
		CONFG_tmp.CL=CL_16BITS;
	}
	else
	{
		CONFG_tmp.CL=CL_8BITS;
	}

	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;


	t1_tmp.CONFG = *((unsigned char *)(&CONFG_tmp));
	t1_tmp.Data_ID= 0x39;
	t1_tmp.mem_start0 = (addr&0xFF);

	if(long_length)
	{
		t1_tmp.mem_start1 = ((addr>>8)&0xFF);
	}

	return DSI_Write_T1_INS(&t1_tmp);


}


DSI_STATUS DSI_read_lcm_fb(void)
{
	// TBD
	return DSI_STATUS_OK;
}


// -------------------- Retrieve Information --------------------

DSI_STATUS DSI_DumpRegisters(void)
{
    UINT32 i;
/*
description of dsi status
Bit	Value	Description
[0]	0x0001	Idle (wait for command)
[1]	0x0002	Reading command queue for header
[2]	0x0004	Sending type-0 command
[3]	0x0008	Waiting frame data from RDMA for type-1 command
[4]	0x0010	Sending type-1 command
[5]	0x0020	Sending type-2 command
[6]	0x0040	Reading command queue for data
[7]	0x0080	Sending type-3 command
[8]	0x0100	Sending BTA
[9]	0x0200	Waiting RX-read data
[10]	0x0400	Waiting SW RACK for RX-read data
[11]	0x0800	Waiting TE
[12]	0x1000	Get TE
[13]	0x2000	Waiting external TE
[14]	0x4000	Waiting SW RACK for TE

*/
static const char* DSI_DBG_STATUS_DESCRIPTION[] =
{
	"null",
	"Idle (wait for command)",
	"Reading command queue for header",
	"Sending type-0 command",
	"Waiting frame data from RDMA for type-1 command",
	"Sending type-1 command",
	"Sending type-2 command",
	"Reading command queue for data",
	"Sending type-3 command",
	"Sending BTA",
	"Waiting RX-read data ",
	"Waiting SW RACK for RX-read data",
	"Waiting TE",
	"Get TE ",
	"Waiting SW RACK for TE",
	"Waiting external TE",
};
	unsigned int DSI_DBG6_Status = (INREG32(DSI_BASE+0x160))&0xffff;

	int count=0;
	while(DSI_DBG6_Status){DSI_DBG6_Status>>=1; count++;}
	//while((1<<count) != DSI_DBG6_Status) count++;
	//count++;
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "---------- Start dump DSI registers ----------\n");
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_STATE_DBG6=0x%08x, count=%d, means: [%s]\n", DSI_DBG6_Status, count, DSI_DBG_STATUS_DESCRIPTION[count]);
    for (i = 0; i < sizeof(DSI_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI+%04x : 0x%08x\n", i, INREG32(DSI_BASE + i));
    }

    for (i = 0; i < sizeof(DSI_CMDQ_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_CMD+%04x(%p) : 0x%08x\n", i, (UINT32*)(DSI_BASE+0x180+i), INREG32((DSI_BASE+0x180+i)));
    }

    for (i = 0; i < sizeof(DSI_PHY_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_PHY+%04x(%p) : 0x%08x\n", i, (UINT32*)(MIPI_CONFIG_BASE+i), INREG32((MIPI_CONFIG_BASE+i)));
    }

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_FMDesense_Query(void)
{
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_FM_Desense(unsigned long freq)
{
    ///need check
    DSI_Change_CLK(freq);
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Reset_CLK(void)
{
    extern LCM_PARAMS *lcm_params;

    _WaitForEngineNotBusy();
	DSI_PHY_TIMCONFIG(lcm_params);
	_DSI_PHY_clk_setting(lcm_params);
	return DSI_STATUS_OK;
}

DSI_STATUS DSI_Get_Default_CLK(unsigned int *clk)
{
    extern LCM_PARAMS *lcm_params;
	unsigned int div2_real = lcm_params->dsi.pll_div2 ? lcm_params->dsi.pll_div2 : 0x1;

    *clk = 13 * (lcm_params->dsi.pll_div1 + 1) / div2_real;
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Get_Current_CLK(unsigned int *clk)
{
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Change_CLK(unsigned int clk)
{
    extern LCM_PARAMS *lcm_params;

    if(clk > 1000)
        return DSI_STATUS_ERROR;
    memcpy((void *)&lcm_params_for_clk_setting, (void *)lcm_params, sizeof(LCM_PARAMS));

    for(lcm_params_for_clk_setting.dsi.pll_div2 = 15; lcm_params_for_clk_setting.dsi.pll_div2 > 0; lcm_params_for_clk_setting.dsi.pll_div2--)
    {
        for(lcm_params_for_clk_setting.dsi.pll_div1 = 0; lcm_params_for_clk_setting.dsi.pll_div1 < 39; lcm_params_for_clk_setting.dsi.pll_div1++)
        {
            if((13 * (lcm_params_for_clk_setting.dsi.pll_div1 + 1) / lcm_params_for_clk_setting.dsi.pll_div2) >= clk)
                goto end;
        }
    }

    if(lcm_params_for_clk_setting.dsi.pll_div2 == 0)
    {
        for(lcm_params_for_clk_setting.dsi.pll_div1 = 0; lcm_params_for_clk_setting.dsi.pll_div1 < 39; lcm_params_for_clk_setting.dsi.pll_div1++)
        {
            if((26 * (lcm_params_for_clk_setting.dsi.pll_div1 + 1)) >= clk)
                goto end;
        }
    }

end:
    _WaitForEngineNotBusy();
	DSI_PHY_TIMCONFIG(&lcm_params_for_clk_setting);
	_DSI_PHY_clk_setting(&lcm_params_for_clk_setting);

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_StartTransfer(BOOL needStartDSI)
{

	disp_path_get_mutex();

	LCD_ConfigOVL();

	DSI_WaitTE();

	if(needStartDSI)
	{
		DSI_clk_HS_mode(1);
		DSI_Start();
	}

	disp_path_release_mutex();

	return DSI_STATUS_OK;

}


