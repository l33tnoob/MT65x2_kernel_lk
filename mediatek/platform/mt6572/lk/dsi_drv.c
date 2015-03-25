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
#ifdef BUILD_LK
#define ENABLE_DSI_INTERRUPT 0 

#include <string.h>
#include <platform/mt_gpt.h>
#include <platform/mt_typedefs.h>
#include <platform/sec_devinfo.h>
#include <platform/disp_drv_platform.h>

#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>
#else

#define ENABLE_DSI_INTERRUPT 1 

#include <linux/delay.h>
#include <disp_drv_log.h>
#include <linux/time.h>
#include <linux/string.h>

#include "disp_drv_platform.h"

#include "lcd_reg.h"
#include "lcd_drv.h"

#include "dsi_reg.h"
#include "dsi_drv.h"
#endif

#if ENABLE_DSI_INTERRUPT
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <mach/irqs.h>
#include "mtkfb.h"
static wait_queue_head_t _dsi_wait_queue;
static wait_queue_head_t _dsi_dcs_read_wait_queue;
#endif

void dsi_udelay(UINT32 ms)
{
    unsigned int i, j;

    for (i = 0; i < ms; i++)
        for (j = 0; j < 0xFF; j ++);
}

void dsi_mdelay(UINT32 ms)
{
    unsigned int i, j;

    for (i = 0; i < ms; i++)
        for (j = 0; j < 0xFF; j ++);
}

/*
#define PLL_BASE			(0xF0060000)
#define DSI_PHY_BASE		(0xF0060B00)
#define DSI_BASE            	(0xF0140000)
*/

#if !(defined(CONFIG_MT6589_FPGA) || defined(BUILD_LK))
//#define DSI_MIPI_API
#endif

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

#define DSI_OUTREG32_R(type, addr2, addr1) DISP_OUTREG32_R(type, addr2, addr1)
#define DSI_OUTREG32_V(type, addr2, var) DISP_OUTREG32_V(type, addr2, var)
#define DSI_OUTREGBIT(TYPE,REG,bit,value) DISP_OUTREGBIT(TYPE,REG,bit,value)
#define DSI_INREG32(type,addr) DISP_INREG32(type,addr)

static PDSI_REGS const DSI_REG = (PDSI_REGS)(DSI_BASE);
static PDSI_PHY_REGS const DSI_PHY_REG = (PDSI_PHY_REGS)(MIPI_CONFIG_BASE);
static PDSI_CMDQ_REGS const DSI_CMDQ_REG = (PDSI_CMDQ_REGS)(DSI_BASE+0x180);
static PLCD_REGS const LCD_REG = (PLCD_REGS)(LCD_BASE);
static PDSI_VM_CMDQ_REGS const DSI_VM_CMD_REG = (PDSI_VM_CMDQ_REGS)(DSI_BASE + 0x134);

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

static BOOL s_isDsiPowerOn = FALSE;
static DSI_CONTEXT _dsiContext;
static volatile bool isTeSetting = false;
static volatile bool dsiTeEnable = false;
static volatile bool dsiTeExtEnable = false;
static bool glitch_log_on = true;
extern LCM_PARAMS *lcm_params;

// PLL_clock
static const DSI_PLL_CONFIG pll_config[] =
{
//   CLK, TXDIV0, TXDIV1, SDM_PCW, SSC_PH_INIT, SSC_PRD, SSC_DELTA1, SSC_DELTA
    { 26, 0x10, 0x10, 0x40000000, 1, 0x01B1, 0x0790, 0x0790}, // 26 * 1
    { 52, 0x10, 0x01, 0x40000000, 1, 0x01B1, 0x0790, 0x0790}, // 26 * 2
    { 78, 0x10, 0x00, 0x30000000, 1, 0x01B1, 0x05AC, 0x05AC}, // 26 * 3
//    {100, 0x10, 0x00, 0x3D89D89D, 1, 0x01B1, 0x0745, 0x0745}, // 25 * 4
    {104, 0x10, 0x00, 0x40000000, 1, 0x01B1, 0x0790, 0x0790}, // 26 * 4
//    {125, 0x01, 0x00, 0x26762762, 1, 0x01B1, 0x048B, 0x048B}, // 25 * 5
    {130, 0x01, 0x00, 0x28000000, 1, 0x01B1, 0x04BA, 0x04BA}, // 26 * 5
//    {150, 0x01, 0x00, 0x2E276276, 1, 0x01B1, 0x0574, 0x0574}, // 25 * 6
    {156, 0x01, 0x00, 0x30000000, 1, 0x01B1, 0x05AC, 0x05AC}, // 26 * 6
//    {175, 0x01, 0x00, 0x35D89D89, 1, 0x01B1, 0x065D, 0x065D}, // 25 * 7
    {182, 0x01, 0x00, 0x38000000, 1, 0x01B1, 0x069E, 0x069E}, // 26 * 7
//    {200, 0x01, 0x00, 0x3D89D89D, 1, 0x01B1, 0x0745, 0x0745}, // 25 * 8
    {208, 0x01, 0x00, 0x40000000, 1, 0x01B1, 0x0790, 0x0790}, // 26 * 8
    {221, 0x01, 0x00, 0x44000000, 1, 0x01B1, 0x066D, 0x066D}, // 26 * 8.5 (default)
//    {225, 0x01, 0x00, 0x453B13B1, 1, 0x01B1, 0x082E, 0x082E}, // 25 * 9
    {234, 0x01, 0x00, 0x48000000, 1, 0x01B1, 0x0882, 0x0882}, // 26 * 9
//    {250, 0x00, 0x00, 0x26762762, 1, 0x01B1, 0x048B, 0x048B}, // 25 * 10
    {260, 0x00, 0x00, 0x28000000, 1, 0x01B1, 0x04BA, 0x04BA}, // 26 * 10
//    {275, 0x00, 0x00, 0x2A4EC4EC, 1, 0x01B1, 0x0500, 0x0500}, // 25 * 11
    {286, 0x00, 0x00, 0x2C000000, 1, 0x01B1, 0x0533, 0x0533}, // 26 * 11
//    {300, 0x00, 0x00, 0x2E276276, 1, 0x01B1, 0x0574, 0x0574}, // 25 * 12
    {312, 0x00, 0x00, 0x30000000, 1, 0x01B1, 0x05AC, 0x05AC}, // 26 * 12
//    {325, 0x00, 0x00, 0x32000000, 1, 0x01B1, 0x05E8, 0x05E8}, // 25 * 13
//    {350, 0x00, 0x00, 0x35D89D89, 1, 0x01B1, 0x065D, 0x065D}, // 25 * 14
//    {375, 0x00, 0x00, 0x39B13B13, 1, 0x01B1, 0x06D1, 0x06D1}, // 25 * 15
//    {400, 0x00, 0x00, 0x3D89D89D, 1, 0x01B1, 0x0745, 0x0745}, // 25 * 16
//    {425, 0x00, 0x00, 0x41627627, 1, 0x01B1, 0x07BA, 0x07BA}, // 25 * 17
//    {450, 0x00, 0x00, 0x453B13B1, 1, 0x01B1, 0x082E, 0x082E}, // 25 * 18
//    {475, 0x00, 0x00, 0x4913B13B, 1, 0x01B1, 0x08A2, 0x08A2}, // 25 * 19
//    {500, 0x00, 0x00, 0x4CEC4EC4, 1, 0x01B1, 0x0917, 0x0917}, // 25 * 20
};

#ifndef BUILD_LK

DEFINE_SPINLOCK(g_handle_esd_lock);

static BOOL dsi_esd_recovery = false;
static BOOL dsi_noncont_clk_enabled = false;
static unsigned int dsi_noncont_clk_period = 1;
static BOOL dsi_int_te_enabled = false;
static unsigned int dsi_int_te_period = 1;
static unsigned int dsi_dpi_isr_count = 0;
unsigned long g_handle_esd_flag;

#endif

#ifdef BUILD_LK
static long int get_current_time_us(void)
{
    return 0;       ///TODO: fix me
}
#else
static long int get_current_time_us(void)
{
    struct timeval t;
    do_gettimeofday(&t);
    return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}
#endif

static int custom_pll_clock_remap(unsigned int mipi_clock)
{
    unsigned int i = 0;

    printf("DSI: custom_pll_clock_remap, mipi clock be %d MHz!!!\n", mipi_clock);

    if (mipi_clock == 0)
        return -1;

    if ((mipi_clock > 0) && (mipi_clock < pll_config[0].CLK))
        return 0;

    while (i < (sizeof(pll_config) / sizeof(DSI_PLL_CONFIG) - 1))
    {
        ASSERT(pll_config[i].CLK < pll_config[i + 1].CLK);
        if ((mipi_clock >= pll_config[i].CLK) && (mipi_clock <= pll_config[i + 1].CLK))
        {
            if ((mipi_clock - pll_config[i].CLK) > (pll_config[i + 1].CLK - mipi_clock))
                i ++;
            break;
        }
        i ++;
    }

    printf("custom_pll_clock_remap, remap clock is %d MHz (%d)!!!\n", pll_config[i].CLK, i);

    return i;
}


#if ENABLE_DSI_INTERRUPT
static irqreturn_t _DSI_InterruptHandler(int irq, void *dev_id)
{   
    DSI_INT_STATUS_REG status = DSI_REG->DSI_INTSTA;

    if (status.RD_RDY)
    {        
        ///write clear RD_RDY interrupt
        DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
        /// write clear RD_RDY interrupt must be before DSI_RACK
        /// because CMD_DONE will raise after DSI_RACK, 
        /// so write clear RD_RDY after that will clear CMD_DONE too
        
		do
        {
            ///send read ACK
            DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
        } while(DSI_REG->DSI_STA.BUSY);

		wake_up_interruptible(&_dsi_dcs_read_wait_queue);
        if(_dsiContext.pIntCallback)
            _dsiContext.pIntCallback(DISP_DSI_READ_RDY_INT);            
    }

    if (status.CMD_DONE)
    {
        DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);
    	// Go back to LP mode.
    	DSI_clk_HS_mode(0);
        wake_up_interruptible(&_dsi_wait_queue);
        if(_dsiContext.pIntCallback)
            _dsiContext.pIntCallback(DISP_DSI_CMD_DONE_INT);            
    }

    return IRQ_HANDLED;
}
#endif


static BOOL _IsEngineBusy(void)
{
    DSI_INT_STATUS_REG status;
    //arch_clean_invalidate_cache_range(*(unsigned int*)(&DSI_REG->DSI_STA), 32);
    
    status = DSI_REG->DSI_INTSTA;
    
    if (status.BUSY)		
        return TRUE;
    
    return FALSE;
}

#if 0
static BOOL _IsCMDQBusy(void)
{
	DSI_INT_STATUS_REG INT_status;

	INT_status=DSI_REG->DSI_INTSTA;

	if (!INT_status.CMD_DONE)
	{
		DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI CMDQ status BUSY !!\n");
	
		return TRUE;
	}

	return FALSE;
}
#endif


static void DSI_WaitForEngineNotBusy(void)
{
    int timeOut;
#if ENABLE_DSI_INTERRUPT
    long int time;
    static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
#endif

    if (DSI_REG->DSI_MODE_CTRL.MODE)
        return ;
    
    timeOut = 400;

#if ENABLE_DSI_INTERRUPT
    time = get_current_time_us();

    if (in_interrupt())
    {
        // perform busy waiting if in interrupt context
        while(_IsEngineBusy()) {
            msleep(1);
            if (--timeOut < 0)	{
                DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!(Wait %d us)\n", get_current_time_us() - time);
                DSI_DumpRegisters();

                DSI_Reset();
                dsiTeEnable = false;//disable TE
                dsiTeExtEnable = false;//disable TE
                break;
            }
        }
    }
    else
    {
        while (_IsEngineBusy())
        {
            long ret = wait_event_interruptible_timeout(_dsi_wait_queue, 
                                                        !_IsEngineBusy(),
                                                        WAIT_TIMEOUT);
            if (0 == ret) {
                DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine not busy timeout, and reset DSI!!!\n");
                DSI_DumpRegisters();

                DSI_Reset();
                dsiTeEnable = false;//disable TE
                dsiTeExtEnable = false;//disable TE
            }
        }
    }
#else

    while(_IsEngineBusy()) {
        mdelay(1);
        if (--timeOut < 0)	{
            DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!\n");
            DSI_DumpRegisters();

            DSI_Reset();
            dsiTeEnable = false;//disable TE
            dsiTeExtEnable = false;//disable TE
            break;
        }
    }
    DSI_OUTREG32_V(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA, 0x0);
#endif    
}


static void _BackupDSIRegisters(void)
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
	
    DSI_OUTREG32_R(PDSI_HSTX_CKLP_WC_REG,&regs->DSI_HSTX_CKLP_WC, &DSI_REG->DSI_HSTX_CKLP_WC);		
    
    DSI_OUTREG32_R(PDSI_MEM_CONTI_REG,&regs->DSI_MEM_CONTI, &DSI_REG->DSI_MEM_CONTI);

    OUTREG32(&regs->DSI_PHY_TIMECON0, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON0));
    OUTREG32(&regs->DSI_PHY_TIMECON1, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON1));
    OUTREG32(&regs->DSI_PHY_TIMECON2, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON2));
    OUTREG32(&regs->DSI_PHY_TIMECON3, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON3));

    DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG,&regs->DSI_VM_CMD_CON, &DSI_REG->DSI_VM_CMD_CON);
}


static void _RestoreDSIRegisters(void)
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

    DSI_OUTREG32_R(PDSI_HSTX_CKLP_WC_REG,&DSI_REG->DSI_HSTX_CKLP_WC, &regs->DSI_HSTX_CKLP_WC);		

    DSI_OUTREG32_R(PDSI_MEM_CONTI_REG,&DSI_REG->DSI_MEM_CONTI, &regs->DSI_MEM_CONTI);

    OUTREG32(&DSI_REG->DSI_PHY_TIMECON0, AS_UINT32(&regs->DSI_PHY_TIMECON0));		
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON1, AS_UINT32(&regs->DSI_PHY_TIMECON1));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON2, AS_UINT32(&regs->DSI_PHY_TIMECON2));		
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON3, AS_UINT32(&regs->DSI_PHY_TIMECON3));		

    DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG,&DSI_REG->DSI_VM_CMD_CON, &regs->DSI_VM_CMD_CON);
}

static void _ResetBackupedDSIRegisterValues(void)
{
    DSI_REGS *regs = &_dsiContext.regBackup;
    memset((void*)regs, 0, sizeof(DSI_REGS));
}

static DSI_STATUS DSI_TE_Setting(void)
{
    //return DSI_STATUS_OK;
    if(isTeSetting)
    {
        return DSI_STATUS_OK;
    }

    if(lcm_params->dsi.mode == CMD_MODE && lcm_params->dsi.lcm_ext_te_enable == TRUE)
    {
        //Enable EXT TE
        dsiTeEnable = false;
        dsiTeExtEnable = true;
    }
    else
    {
        //Enable BTA TE
        dsiTeEnable = true;
        dsiTeExtEnable = false;
    }

    isTeSetting = true;

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Init(BOOL isDsiPoweredOn)
{
    DSI_STATUS ret = DSI_STATUS_OK;

    memset(&_dsiContext, 0, sizeof(_dsiContext));

    if (isDsiPoweredOn) {
        _BackupDSIRegisters();
    } else {
        _ResetBackupedDSIRegisterValues();
    }
    ret = DSI_PowerOn();
    ASSERT(ret == DSI_STATUS_OK);

    DSI_TE_Setting();
    DSI_OUTREG32_V(PDSI_MEM_CONTI_REG,&DSI_REG->DSI_MEM_CONTI, DSI_WMEM_CONTI);
    DSI_OUTREGBIT(DSI_COM_CTRL_REG, DSI_REG->DSI_COM_CTRL, DSI_EN, 1);
    DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,EXT_TE,1);

#if ENABLE_DSI_INTERRUPT
    init_waitqueue_head(&_dsi_wait_queue);
    init_waitqueue_head(&_dsi_dcs_read_wait_queue);	
    if (request_irq(MT6589_DSI_IRQ_ID,
        _DSI_InterruptHandler, IRQF_TRIGGER_LOW, MTKFB_DRIVER, NULL) < 0)
    {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", "fail to request DSI irq\n"); 
        return DSI_STATUS_ERROR;
    }
    //mt65xx_irq_unmask(MT65XX_DSI_IRQ_ID);
    DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);
    DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
#endif
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Deinit(void)
{
    DSI_STATUS ret = DSI_PowerOff();

    ASSERT(ret == DSI_STATUS_OK);

    return DSI_STATUS_OK;
}


#ifdef BUILD_LK
DSI_STATUS DSI_PowerOn(void)
{
    if (!s_isDsiPowerOn)
    {
        MASKREG32(DISP_REG_CONFIG_CG_CLR1, 0x13, 0x13);    // must turn on DBI engine for DSI reset
        _RestoreDSIRegisters();

        s_isDsiPowerOn = TRUE;
        printf("[DISP] - DSI_PowerOn. 0x%8x\n", INREG32(DISP_REG_CONFIG_CG_CON1));
    }

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PowerOff(void)
{
    if (s_isDsiPowerOn)
    {
        _BackupDSIRegisters();
        MASKREG32(DISP_REG_CONFIG_CG_SET1, 0x13, 0x13);    // must turn off DBI engine for DSI reset

        s_isDsiPowerOn = FALSE;
        printf("[DISP] - DSI_PowerOff. 0x%8x\n", INREG32(DISP_REG_CONFIG_CG_CON1));
    }

    return DSI_STATUS_OK;
}

#else
DSI_STATUS DSI_PowerOn(void)
{
#ifndef CONFIG_MT6589_FPGA
    if (!s_isDsiPowerOn)
    {
#if 0   // FIXME
        //BOOL ret = hwEnableClock(MT65XX_PDN_MM_DSI, "DSI");
        BOOL ret = enable_clock(MT65XX_PDN_MM_DSI, "DSI");
        ASSERT(!ret);
#endif        
        _RestoreDSIRegisters();
        s_isDsiPowerOn = TRUE;
    }
#endif
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PowerOff(void)
{
#ifndef CONFIG_MT6589_FPGA
    if (s_isDsiPowerOn)
    {
        BOOL ret = TRUE;
        DSI_WaitForEngineNotBusy();
        _BackupDSIRegisters();
#if 0   // FIXME
        //ret = hwDisableClock(MT65XX_PDN_MM_DSI, "DSI");
        ret = disable_clock(MT65XX_PDN_MM_DSI, "DSI");
        ASSERT(!ret);
#endif        
        s_isDsiPowerOn = FALSE;
    }
#endif
    return DSI_STATUS_OK;
}
#endif


DSI_STATUS DSI_WaitForNotBusy(void)
{
    DSI_WaitForEngineNotBusy();

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_EnableClk(void)
{
    DSI_WaitForEngineNotBusy();

    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);

    return DSI_STATUS_OK;
}


static void DSI_BackUpCmdQ(void)
{
    unsigned int i;
    DSI_CMDQ_REGS *regs = &(_dsiContext.cmdqBackup);
    
    _dsiContext.cmdq_size = DSI_INREG32(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE);
    
    for (i=0; i<_dsiContext.cmdq_size; i++)
        OUTREG32(&regs->data[i], AS_UINT32(&DSI_CMDQ_REG->data[i]));
}


static void DSI_RestoreCmdQ(void)
{
    unsigned int i;
    DSI_CMDQ_REGS *regs = &(_dsiContext.cmdqBackup);
    
    DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, _dsiContext.cmdq_size);
    
    for (i=0; i<_dsiContext.cmdq_size; i++)
        OUTREG32(&DSI_CMDQ_REG->data[i], AS_UINT32(&regs->data[i]));
}


void DSI_WaitBtaTE(void)
{
    DSI_T0_INS t0;
#if ENABLE_DSI_INTERRUPT
    long ret;
    static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
#else
    long int dsi_wait_time = 0;
#endif
    
    if(DSI_REG->DSI_MODE_CTRL.MODE != CMD_MODE)
        return;
    
    DSI_WaitForEngineNotBusy();
    
    // backup command queue setting.
    DSI_BackUpCmdQ();
    
    t0.CONFG = 0x20;		///TE
    t0.Data0 = 0;
    t0.Data_ID = 0;
    t0.Data1 = 0;
    
    DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
    DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
    
#if ENABLE_DSI_INTERRUPT
    
    ret = wait_event_interruptible_timeout(_dsi_wait_bta_te, 
    !_IsEngineBusy(),
    WAIT_TIMEOUT);
    
    if (0 == ret) {
        DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for _dsi_wait_bta_te(DSI_INTSTA.TE_RDY) ready timeout!!!\n");
        
        // Set DSI_RACK to let DSI idle
        DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
        
        DSI_DumpRegisters();	
        ///do necessary reset here
        DSI_Reset();
        dsiTeEnable = false;//disable TE
        return;
    }
    
    // After setting DSI_RACK, it needs to wait for CMD_DONE interrupt.
    DSI_WaitForEngineNotBusy();
    
#else
    
    while(DSI_REG->DSI_INTSTA.TE_RDY == 0)	// polling TE_RDY
    {
        udelay(100);//sleep 50us
        dsi_wait_time++;
        if(dsi_wait_time > 40000)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for TE_RDY timeout!!!\n");
            
            // Set DSI_RACK to let DSI idle
            DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
            
            DSI_DumpRegisters();
            
            //do necessary reset here
            DSI_Reset();
            dsiTeEnable = false;//disable TE
            break;
        }
    }
    dsi_wait_time = 0;
    // Write clear RD_RDY
    DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,TE_RDY,0);
    
    // Set DSI_RACK to let DSI idle
    DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);

    if(!dsiTeEnable){
        DSI_RestoreCmdQ();
        DSI_LP_Reset();
        return;
    }
    
    while(DSI_REG->DSI_INTSTA.CMD_DONE == 0)	// polling CMD_DONE
    {
        udelay(100);//sleep 50us
        dsi_wait_time++;
        if(dsi_wait_time > 40000)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for CMD_DONE timeout!!!\n");
            
            // Set DSI_RACK to let DSI idle
            DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
            
            DSI_DumpRegisters();
            
            ///do necessary reset here
            DSI_Reset();
            dsiTeEnable = false;//disable TE
            break;
        }
    }
    
    // Write clear CMD_DONE
    DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);
    
    #endif
    // restore command queue setting.
    DSI_RestoreCmdQ();
    DSI_LP_Reset();
}

void DSI_WaitExternalTE(void)
{
#if ENABLE_DSI_INTERRUPT
    long ret;
    static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
#else
    long int dsi_wait_time = 0;
#endif

    if(DSI_REG->DSI_MODE_CTRL.MODE != CMD_MODE)
        return;

    //No need to wait dsi not busy
    //DSI_WaitForEngineNotBusy();

    DSI_OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EN,1);
    DSI_OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EDGE,0);

#if ENABLE_DSI_INTERRUPT

    //can not go to this line

#else

    while(DSI_REG->DSI_INTSTA.EXT_TE == 0)	// polling EXT_TE
    {
        udelay(100);//sleep 50us
        dsi_wait_time++;
        if(dsi_wait_time > 40000)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for EXT_TE timeout!!!\n");

            DSI_DumpRegisters();

            //do necessary reset here
            DSI_Reset();
            dsiTeExtEnable = false;//disable TE
            break;
        }
    }

	// Write clear EXT_TE
	DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,EXT_TE,0);

    if(!dsiTeExtEnable){
        DSI_LP_Reset();
        return;
    }

    #endif

    DSI_LP_Reset();
}

DSI_STATUS DSI_WaitVsync()
{
    UINT32 dsi_wait_time = 0;

    MASKREG32(DISP_REG_RDMA_INT_STATUS, 0x2, 0x0);
    while((INREG32(DISP_REG_RDMA_INT_STATUS)&0x2) != 0x2)	// polling RDMA start
    {
        udelay(50);//sleep 50us
        dsi_wait_time++;
        if(dsi_wait_time > 40000){
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for RDMA0 Start IRQ timeout!!!\n");
            break;
        }
    }
    MASKREG32(DISP_REG_RDMA_INT_STATUS, 0x2, 0x0);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_RegUpdate(void)
{
    UINT32 dsi_wait_time = 0;
    
    printk("[wwy] enter DSI_RegUpdate\n");
    //MASKREG32(0x14011000, 0x1, 0x1); //Enable DISP MUTEX0
    //MASKREG32(0x14011004, 0x1, 0x0);
    while((INREG32(DISP_REG_CONFIG_MUTEX_INTSTA)&0x1) != 0x1) // polling DISP MUTEX0
    {
        printk("[wwy] DSI_RegUpdate dsi_wait_time = %d\n",dsi_wait_time);
        udelay(50);//sleep 50us
        dsi_wait_time++;
        if(dsi_wait_time > 40000){
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for DISP MUTEX0 IRQ timeout!!!\n");
            break;
        }
    }
    MASKREG32(DISP_REG_CONFIG_MUTEX_INTSTA, 0x1, 0x0);
    printk("[wwy] end DSI_RegUpdate\n");
    //mdelay(500);//sleep 50us

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_StartTransfer(BOOL needStartDSI)
{
    if (lcm_params->dsi.mode == CMD_MODE)
    {
        DSI_WaitForEngineNotBusy();
    }

    disp_path_get_mutex();

    LCD_ConfigOVL();

    if (dsiTeEnable)
    {
        DSI_WaitBtaTE();
    }

    if (dsiTeExtEnable)
    {
        DSI_WaitExternalTE();
    }

    if(needStartDSI){
        if(1 == lcm_params->dsi.compatibility_for_nvk){
            if(DSI_STATUS_OK!= DSI_Detect_CLK_Glitch()){
                return DSI_STATUS_OK;
            }
        }
        DSI_EnableClk();
    }

    disp_path_release_mutex();

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Detect_CLK_Glitch(void)
{
    DSI_T0_INS t0;
    char i;
    int read_timeout_cnt=10000;
    int read_timeout_ret = 0;
    unsigned int try_times = 50;
    DSI_RX_DATA_REG read_data0;
    DSI_RX_DATA_REG read_data1;
        
    //    MMProfileLogEx(MTKFB_MMP_Events.Debug, MMProfileFlagStart, 0, 0);
    /**********************start******************/


    DSI_WaitForEngineNotBusy();
   // lcdStartTransfer = true;

    DSI_BackUpCmdQ();
    
    DSI_SetMode(CMD_MODE);
    OUTREG32(&DSI_CMDQ_REG->data[0], 0x00340500);//turn off TE
    DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
    while(DSI_REG->DSI_INTSTA.CMD_DONE == 0);
    DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);

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
            DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_PHI_SEL,0);
        }
        DSI_clk_HS_mode(1);
        while((DSI_INREG32(PDSI_STATE_DBG0_REG,&DSI_REG->DSI_STATE_DBG0)&0x40000) == 0)	 // polling bit18 start
        {
            if(glitch_log_on)
                printk("Test log 2: wait for DSI_STATE_DBG0 bit18==1 \n");
        }
        if(i>0)
        {
            DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_PHI_SEL,1);
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
        
        read_timeout_cnt=1000;
        while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
        {
            ///keep polling
            if(glitch_log_on)
                printk("Test log 3:polling ack & error report \n");

            udelay(1);
            read_timeout_cnt--;
            //					printk("polling time = %d us\n", ((unsigned int)end_time - (unsigned int)start_time));
            if(read_timeout_cnt==0)
            {
                //					    if(glitch_log_on)
                //		                   printk("Test log 4:Polling DSI read ready timeout,%d us\n", (unsigned int)sched_clock() - (unsigned int)start_time);
                
                DSI_DumpRegisters();
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
    DSI_RestoreCmdQ();
    
    DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_PHI_SEL,0);

    switch(lcm_params->dsi.LANE_NUM)
    {
        case LCM_FOUR_LANE:
            DSI_OUTREG32_V(PMIPITX_DSI_SW_CTRL_CON0_REG,&DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0, 0x3CF3C7B1); 
            break;
        case LCM_THREE_LANE:
            DSI_OUTREG32_V(PMIPITX_DSI_SW_CTRL_CON0_REG,&DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0, 0x00F3C7B1); 
            break;
        default:
            DSI_OUTREG32_V(PMIPITX_DSI_SW_CTRL_CON0_REG,&DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON0, 0x0003C7B1); 
    }	
    
    DSI_OUTREG32_V(PMIPITX_DSI_SW_CTRL_CON1_REG,&DSI_PHY_REG->MIPITX_DSI_SW_CTRL_CON1, 0x0); 
    DSI_OUTREG32_V(PMIPITX_DSI_SW_CTRL_REG,&DSI_PHY_REG->MIPITX_DSI_SW_CTRL, 0x1); 
    
    DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
    DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
    DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
    
    DSI_clk_HS_mode(1);
    
    if(glitch_log_on)
        printk("Test log 5:start Polling bit18\n");
    
    while((DSI_INREG32(PDSI_STATE_DBG0_REG,&DSI_REG->DSI_STATE_DBG0)&0x40000) == 0)	 // polling bit18
    {
        udelay(1);
    }
    
    if(glitch_log_on)
        printk("Test log 6:start Polling bit18\n");

    DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL,SW_CTRL_EN,0x0);

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
    if(i == try_times)
        return DSI_STATUS_ERROR;

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_DisableClk(void)
{
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Reset(void)
{
    DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
    mdelay(5);
    DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_LP_Reset(void)
{
    DSI_WaitForEngineNotBusy();
    DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
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
#if ENABLE_DSI_INTERRUPT
    switch(eventID)
    {
        case DISP_DSI_READ_RDY_INT:
            DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
            break;
        case DISP_DSI_CMD_DONE_INT:
            DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);
            break;
        default:
            return DSI_STATUS_ERROR;
    }

    return DSI_STATUS_OK;
#else
    ///TODO: warning log here
    return DSI_STATUS_ERROR;
#endif
}


DSI_STATUS DSI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
    _dsiContext.pIntCallback = pCB;

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_handle_TE(void)
{
    unsigned int data_array;
    
    //data_array=0x00351504;
    //DSI_set_cmdq(&data_array, 1, 1);
    
    //mdelay(10);
    
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
    
    DSI_OUTREGBIT(DSI_CMDQ_CTRL_REG,DSI_REG->DSI_CMDQ_SIZE,CMDQ_SIZE,1);
    
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
    
    // wait TE Trigger status
    //	do
    //	{
    mdelay(10);
    
    data_array=DSI_INREG32(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI INT state : %x !! \n", data_array);
    
    data_array=DSI_INREG32(PDSI_TRIG_STA_REG,&DSI_REG->DSI_TRIG_STA);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI TRIG TE status check : %x !! \n", data_array);
    //	} while(!(data_array&0x4));
    
    // RACT	
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI Set RACT !! \n");
    DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
    
    return DSI_STATUS_OK;
}

void DSI_Set_VM_CMD(LCM_PARAMS *lcm_params)
{
    DSI_OUTREGBIT(DSI_VM_CMD_CON_REG,DSI_REG->DSI_VM_CMD_CON,TS_VFP_EN,1);
    DSI_OUTREGBIT(DSI_VM_CMD_CON_REG,DSI_REG->DSI_VM_CMD_CON,VM_CMD_EN,1);

    return;
}

void DSI_Config_VDO_Timing(LCM_PARAMS *lcm_params)
{
    unsigned int line_byte;
    unsigned int horizontal_sync_active_byte;
    unsigned int horizontal_backporch_byte;
    unsigned int horizontal_frontporch_byte;
    unsigned int horizontal_blanking_byte;
    unsigned int dsiTmpBufBpp;

    #define LINE_PERIOD_US				(8 * line_byte * _dsiContext.bit_time_ns / 1000)
    

    if(lcm_params->dsi.data_format.format == LCM_DSI_FORMAT_RGB565)
        dsiTmpBufBpp = 2;
    else
        dsiTmpBufBpp = 3;
    
    DSI_OUTREGBIT(DSI_VSA_NL_REG,DSI_REG->DSI_VSA_NL,VSA_NL,lcm_params->dsi.vertical_sync_active);
    DSI_OUTREGBIT(DSI_VBP_NL_REG,DSI_REG->DSI_VBP_NL,VBP_NL,lcm_params->dsi.vertical_backporch);
    DSI_OUTREGBIT(DSI_VFP_NL_REG,DSI_REG->DSI_VFP_NL,VFP_NL,lcm_params->dsi.vertical_frontporch);
    DSI_OUTREGBIT(DSI_VACT_NL_REG,DSI_REG->DSI_VACT_NL,VACT_NL,lcm_params->dsi.vertical_active_line);
    
    line_byte							=	(lcm_params->dsi.horizontal_sync_active \
                                + lcm_params->dsi.horizontal_backporch \
                                + lcm_params->dsi.horizontal_frontporch \
                                + lcm_params->dsi.horizontal_active_pixel) * dsiTmpBufBpp;
    
    horizontal_sync_active_byte 		=	(lcm_params->dsi.horizontal_sync_active * dsiTmpBufBpp - 4);
    
    if (lcm_params->dsi.mode == SYNC_EVENT_VDO_MODE)
        horizontal_backporch_byte		=	((lcm_params->dsi.horizontal_backporch + lcm_params->dsi.horizontal_sync_active)* dsiTmpBufBpp - 4);
    else
        horizontal_backporch_byte		=	(lcm_params->dsi.horizontal_backporch * dsiTmpBufBpp - 4);
    
    horizontal_frontporch_byte			=	(lcm_params->dsi.horizontal_frontporch * dsiTmpBufBpp - 6);
    horizontal_blanking_byte 		=	(lcm_params->dsi.horizontal_blanking_pixel * dsiTmpBufBpp - 4);

    /*	
    OUTREG32(&DSI_REG->DSI_LINE_NB, line_byte);
    OUTREG32(&DSI_REG->DSI_HSA_NB, horizontal_sync_active_byte);
    OUTREG32(&DSI_REG->DSI_HBP_NB, horizontal_backporch_byte);
    OUTREG32(&DSI_REG->DSI_HFP_NB, horizontal_frontporch_byte);
    OUTREG32(&DSI_REG->DSI_RGB_NB, rgb_byte);
    */

    DSI_OUTREGBIT(DSI_HSA_WC_REG,DSI_REG->DSI_HSA_WC,HSA_WC,(horizontal_sync_active_byte-6));
    DSI_OUTREGBIT(DSI_HBP_WC_REG,DSI_REG->DSI_HBP_WC,HBP_WC,(horizontal_backporch_byte-6));
    DSI_OUTREGBIT(DSI_HFP_WC_REG,DSI_REG->DSI_HFP_WC,HFP_WC,(horizontal_frontporch_byte-6));
    DSI_OUTREGBIT(DSI_BLLP_WC_REG,DSI_REG->DSI_BLLP_WC,BLLP_WC,(horizontal_blanking_byte-6));
    
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

    /*
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - HLB : %d \n", DSI_REG->DSI_LINE_NB);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - HSA : %d \n", DSI_REG->DSI_HSA_NB);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - HBP : %d \n", DSI_REG->DSI_HBP_NB);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - HFP : %d \n", DSI_REG->DSI_HFP_NB);
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - RGB : %d \n", DSI_REG->DSI_RGB_NB);
    */
}


void DSI_PHY_clk_setting(LCM_PARAMS *lcm_params)
{
    MIPITX_DSI_BG_CON_REG tmp_reg1 = DSI_PHY_REG->MIPITX_DSI_BG_CON;
    unsigned int dsi_bg_r2_trim;
    unsigned int dsi_bg_r1_trim;

    unsigned int data_rate = 0;
    unsigned int txdiv = 0;
    unsigned int delta = 4;//Delta is SSC range, default is 0%~-4%
    unsigned int pll_sdm_fra_en = 0;
    unsigned int pll_sdm_ssc_en = 0;
    DSI_PLL_CONFIG pll_config_value = {0, 0, 0, 0, 0, 0, 0, 0};


    if (lcm_params->type == LCM_TYPE_DPI)
    {
        data_rate = lcm_params->dpi.PLL_CLOCK*2;
    }
    else if (lcm_params->type == LCM_TYPE_DSI)
    {
        data_rate = lcm_params->dsi.PLL_CLOCK*2;
    }
    else
    {
        ASSERT(0);
    }

    if(0!=data_rate)
    {
        //if lcm_params->dsi.PLL_CLOCK=0, use other method
        if(data_rate > 1250)
        {
            printk("[DISP] data_rate exceed limitation \n");
            ASSERT(0);
        }
        else if(data_rate >= 500)
        {
            txdiv = 1;
        }
        else if(data_rate >= 250)
        {
            txdiv = 2;
        }
        else if(data_rate >= 125)
        {
            txdiv = 4;
        }
        else if(data_rate > 62)
        {
            txdiv = 8;
        }
        else if(data_rate >= 50)
        {
            txdiv = 16;
        }
        else
        {
            printk("[DISP] data_rate is too low,%d!!!\n", __LINE__);
            ASSERT(0);
        }

        //PLL txdiv config
        switch(txdiv)
        {
            case 1:
                pll_config_value.TXDIV0 = 0;
                pll_config_value.TXDIV1 = 0;
                break;
            case 2:
                pll_config_value.TXDIV0 = 1;
                pll_config_value.TXDIV1 = 0;
                break;
            case 4:
                pll_config_value.TXDIV0 = 2;
                pll_config_value.TXDIV1 = 0;
                break;
            case 8:
                pll_config_value.TXDIV0 = 2;
                pll_config_value.TXDIV1 = 1;
                break;
            case 16:
                pll_config_value.TXDIV0 = 2;
                pll_config_value.TXDIV1 = 2;
                break;
            default:
                break;
        }

        // PLL PCW config
        //pcw = data_Rate*4*txdiv/(26*2);//Post DIV =4, so need data_Rate*4
        pll_config_value.SDM_PCW = data_rate*txdiv/13;
    
        //SSC config
        //pmod = ROUND(1000*26MHz/fmod/2);fmod default is 30Khz, and this value not be changed
        //pmod = 433.33;
        if(1 != lcm_params->dsi.ssc_disable)
        {
            pll_config_value.SSC_PH_INIT = 1;
            pll_config_value.SSC_PRD = 0x1B1;//PRD=ROUND(pmod) = 433;

            //pll_sdm_delta = ROUND((2^18)*delta(10000)*pcw*0.000001/pmod)=ROUND(262144*delta(10000)*data_rate*txdiv*0.000001/(433.33*13))
            //=ROUND(262144*delta*data_rate*txdiv/(43333*13))=(262144*delta*data_rate*txdiv+43333*13/2)/(43333*13)
            if(0 != lcm_params->dsi.ssc_range)
            {
                delta = lcm_params->dsi.ssc_range;
            }
            ASSERT(delta<=8);
            pll_config_value.SSC_DELTA = (delta*data_rate*txdiv*262144+281664)/563329;
            pll_sdm_fra_en = 1;
            pll_sdm_ssc_en = 1;
            printk("[DISP] PLL config:data_rate=%d,txdiv=%d,delta=%d,ssc_delta=0x%x\n",
                        data_rate,txdiv,delta,pll_config_value.SSC_DELTA);
        }
    }
    else
    {
        if (lcm_params->type == LCM_TYPE_DPI)
        {
            pll_config_value.TXDIV1 = lcm_params->dpi.mipi_pll_clk_div2;  // div1
            pll_config_value.TXDIV0 = lcm_params->dpi.mipi_pll_clk_div1;  // div0
            pll_config_value.SDM_PCW = (lcm_params->dpi.mipi_pll_clk_fbk_div)<< 2;
        }
        else if (lcm_params->type == LCM_TYPE_DSI)
        {
            pll_config_value.TXDIV1 = lcm_params->dsi.pll_div2;  // div1
            pll_config_value.TXDIV0 = lcm_params->dsi.pll_div1;  // div0
            pll_config_value.SDM_PCW = (lcm_params->dsi.fbk_div)<< 2;
        }
        pll_config_value.SSC_PH_INIT = 1;
        pll_config_value.SSC_PRD = 0x1B1;//PRD=ROUND(pmod) = 433;
        pll_config_value.SSC_DELTA = 0x048B;

        pll_sdm_fra_en = 1;
        pll_sdm_ssc_en = 1;
    }


    if(0!=(get_devinfo_with_index(18)&0x000000F0))
    {       
        dsi_bg_r2_trim = tmp_reg1.RG_DSI_BG_R2_TRIM;
        dsi_bg_r2_trim += (get_devinfo_with_index(18)&0x000000F0)>>4;
        dsi_bg_r2_trim = dsi_bg_r2_trim & 0xF;
        DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_R2_TRIM,dsi_bg_r2_trim);
    }
    if(0!=(get_devinfo_with_index(18)&0x0000000F))
    {       
        dsi_bg_r1_trim = tmp_reg1.RG_DSI_BG_R1_TRIM;
        dsi_bg_r1_trim += get_devinfo_with_index(18)&0x0000000F;
        dsi_bg_r1_trim = dsi_bg_r1_trim & 0xF;
        DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_R1_TRIM,dsi_bg_r1_trim);
    }

    DSI_OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_HS_BIAS_EN,1);

    DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CKEN,1);
    DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CORE_EN,1);
    //	msleep(10);
    mdelay(1);

    DSI_OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_CKG_LDOOUT_EN,1);
    DSI_OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_LDOCORE_EN,1);

    DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_PWR_ON,1);
    //	msleep(1);
    mdelay(1);

    DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_ISO_EN,0);

    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PREDIV,0);
    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_POSDIV,0);
    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,pll_config_value.TXDIV1);
    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,pll_config_value.TXDIV0);

    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,pll_sdm_fra_en);
    if (0 != pll_config_value.SSC_PRD)
    {
        DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_PRD,pll_config_value.SSC_PRD);
    }
    if (0 != pll_config_value.SSC_PH_INIT)
    {
        DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_PH_INIT,pll_config_value.SSC_PH_INIT);
    }

    /*
        PCW bit 24~30 = floor(pcw)
        PCW bit 16~23 = (pcw - floor(pcw))*256
        PCW bit 8~15 = (pcw*256 - floor(pcw)*256)*256
        PCW bit 8~15 = (pcw*256*256 - floor(pcw)*256*256)*256
    */
    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_H,pll_config_value.SDM_PCW & 0x7F);
    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_16_23,(256*(data_rate*txdiv%13)/13) & 0xFF);
    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_8_15,(256*(256*(data_rate*txdiv%13)%13)/13) & 0xFF);
    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_0_7,(256*(256*(256*(data_rate*txdiv%13)%13)%13)/13) & 0xFF);

    if (0 != pll_config_value.SSC_DELTA)
    {
        DSI_OUTREGBIT(MIPITX_DSI_PLL_CON3_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON3,RG_DSI0_MPPLL_SDM_SSC_DELTA,pll_config_value.SSC_DELTA);
        DSI_OUTREGBIT(MIPITX_DSI_PLL_CON3_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON3,RG_DSI0_MPPLL_SDM_SSC_DELTA1,pll_config_value.SSC_DELTA);
    }

    if (lcm_params->type == LCM_TYPE_DSI)
    {
        DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_LDOOUT_EN,1);
    
        if(lcm_params->dsi.LANE_NUM > 0)
        {
            DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_LDOOUT_EN,1);
        }
        if(lcm_params->dsi.LANE_NUM > 1)
        {
            DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_LDOOUT_EN,1);
        }
        if(lcm_params->dsi.LANE_NUM > 2)
        {
            DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_LDOOUT_EN,1);
        }
    }

    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PLL_EN,1);
    //	msleep(20);
    mdelay(1);

    DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_EN,pll_sdm_ssc_en);

    // default POSDIV by 4
    DSI_OUTREGBIT(MIPITX_DSI_PLL_TOP_REG,DSI_PHY_REG->MIPITX_DSI_PLL_TOP,RG_MPPLL_PRESERVE_L,3);
}


void DSI_PHY_clk_switch(BOOL on, LCM_PARAMS *lcm_params)
{
    if(on){//workaround: do nothing
        if (lcm_params->type == LCM_TYPE_DSI)
        {
            //pad_tie_low_en = 0
            DSI_OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_PAD_TIE_LOW_EN,0);
            
            // switch to mipi tx dsi mode
            DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL,SW_CTRL_EN,0);
        }
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
    
        if (lcm_params->type == LCM_TYPE_DSI)
        {
            // switch to mipi tx sw mode
            DSI_OUTREGBIT(MIPITX_DSI_SW_CTRL_REG,DSI_PHY_REG->MIPITX_DSI_SW_CTRL,SW_CTRL_EN,1);
        }

        // disable mipi clock
        DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PLL_EN,0);
        mdelay(1);
        
        if (lcm_params->type == LCM_TYPE_DSI)
        {
            DSI_OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_PAD_TIE_LOW_EN,1);
        }

        DSI_OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_HS_BIAS_EN,0);
        if (lcm_params->type == LCM_TYPE_DSI)
        {
            DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_LDOOUT_EN,0);
            DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_LDOOUT_EN,0);
            DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_LDOOUT_EN,0);
            DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_LDOOUT_EN,0);
        }
        mdelay(1);

        DSI_OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_CKG_LDOOUT_EN,0);
        DSI_OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_LDOCORE_EN,0);

        DSI_OUTREGBIT(MIPITX_DSI_PLL_TOP_REG,DSI_PHY_REG->MIPITX_DSI_PLL_TOP,RG_MPPLL_PRESERVE_L,0);
        
        DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,0);
        DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_ISO_EN,1);
        DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_PWR_ON,0);        
        mdelay(1);

        DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CKEN,0);
        DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CORE_EN,0);
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
    unsigned int fbk_div = 0;
    unsigned int lane_no = lcm_params->dsi.LANE_NUM;
    
    //	unsigned int div2_real;
    unsigned int cycle_time;
    unsigned int ui;
    unsigned int hs_trail_m, hs_trail_n;

    unsigned int data_rate;
    unsigned int txdiv = 0;


    data_rate = lcm_params->dsi.PLL_CLOCK*2;
    if(0 != data_rate)
    {
        //if lcm_params->dsi.PLL_CLOCK=0, use other method
        if(data_rate > 1250)
        {
            printk("[DISP] data_rate exceed limitation \n");
            ASSERT(0);
        }
        else if(data_rate >= 500)
        {
            txdiv = 1;
        }
        else if(data_rate >= 250)
        {
            txdiv = 2;
        }
        else if(data_rate >= 125)
        {
            txdiv = 4;
        }
        else if(data_rate > 62)
        {
            txdiv = 8;
        }
        else if(data_rate >= 50)
        {
            txdiv = 16;
        }
        else
        {
            printk("[DISP] data_rate is too low,%d!!!\n", __LINE__);
            ASSERT(0);
        }

        //PLL txdiv config
        switch(txdiv)
        {
            case 1:
                div1 = 0;
                div2 = 0;
                break;
            case 2:
                div1 = 1;
                div2 = 0;
                break;
            case 4:
                div1 = 2;
                div2 = 0;
                break;
            case 8:
                div1 = 2;
                div2 = 1;
                break;
            case 16:
                div1 = 2;
                div2 = 2;
                break;
            default:
                break;
        }

        fbk_div = data_rate / 26;
    }
    else
    {
        div1 = lcm_params->dsi.pll_div1;
        div2 = lcm_params->dsi.pll_div2;
        fbk_div = lcm_params->dsi.fbk_div;
    }

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
            printk("[DISP] div1 should be less than 4!!\n");
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
            printk("[DISP] div2 should be less than 4!!\n");
            div2 = 4;
            break;
    }
    
    cycle_time=(1000*4*div2*div1)/(fbk_div*26)+0x01;
    ui=(1000*div2*div1)/(fbk_div*26*0x2)+0x01;
    printk("[DISP] - kernel - DSI_PHY_TIMCONFIG, Cycle Time = %d(ns), Unit Interval = %d(ns). div1 = %d, div2 = %d, fbk_div = %d, lane# = %d \n", cycle_time, ui, div1, div2, fbk_div, lane_no);


    #define NS_TO_CYCLE(n, c)	((n) / (c))
    
    hs_trail_m=1;
    hs_trail_n= (lcm_params->dsi.HS_TRAIL == 0) ? NS_TO_CYCLE(((hs_trail_m * 0x4) + 60), cycle_time) : lcm_params->dsi.HS_TRAIL;
    // +3 is recommended from designer becauase of HW latency
    timcon0.HS_TRAIL	= ((hs_trail_m > hs_trail_n) ? hs_trail_m : hs_trail_n) + 0x0a;
    
    timcon0.HS_PRPR 	= (lcm_params->dsi.HS_PRPR == 0) ? NS_TO_CYCLE((0x40 + 0x5 * ui), cycle_time) : lcm_params->dsi.HS_PRPR;
    // HS_PRPR can't be 1.
    if (timcon0.HS_PRPR == 0)
       timcon0.HS_PRPR = 1;
    
    timcon0.HS_ZERO 	= (lcm_params->dsi.HS_ZERO == 0) ? NS_TO_CYCLE((0xC8 + 0x0a * ui), cycle_time) : lcm_params->dsi.HS_ZERO;
    if (timcon0.HS_ZERO > timcon0.HS_PRPR)
       timcon0.HS_ZERO -= timcon0.HS_PRPR;
    
    timcon0.LPX 		= (lcm_params->dsi.LPX == 0) ? NS_TO_CYCLE(0x50, cycle_time) : lcm_params->dsi.LPX;
    if(timcon0.LPX == 0) 
       timcon0.LPX = 1;
 
    //	timcon1.TA_SACK 	= (lcm_params->dsi.TA_SACK == 0) ? 1 : lcm_params->dsi.TA_SACK;
    timcon1.TA_GET 		= (lcm_params->dsi.TA_GET == 0) ? (0x5 * timcon0.LPX) : lcm_params->dsi.TA_GET;
    timcon1.TA_SURE 	= (lcm_params->dsi.TA_SURE == 0) ? (0x3 * timcon0.LPX / 0x2) : lcm_params->dsi.TA_SURE;
    timcon1.TA_GO 		= (lcm_params->dsi.TA_GO == 0) ? (0x4 * timcon0.LPX) : lcm_params->dsi.TA_GO;
    // --------------------------------------------------------------
    //  NT35510 need fine tune timing
    //  Data_hs_exit = 60 ns + 128UI 
    //  Clk_post = 60 ns + 128 UI. 
    // --------------------------------------------------------------
    timcon1.DA_HS_EXIT  = (lcm_params->dsi.DA_HS_EXIT == 0) ? NS_TO_CYCLE((0x3c + 0x80 * ui), cycle_time) : lcm_params->dsi.DA_HS_EXIT;
    
    timcon2.CLK_TRAIL 	= ((lcm_params->dsi.CLK_TRAIL == 0) ? NS_TO_CYCLE(0x64, cycle_time) : lcm_params->dsi.CLK_TRAIL) + 0x0a;
    // CLK_TRAIL can't be 1.
    if (timcon2.CLK_TRAIL < 2)
       timcon2.CLK_TRAIL = 2;
    
    //	timcon2.LPX_WAIT 	= (lcm_params->dsi.LPX_WAIT == 0) ? 1 : lcm_params->dsi.LPX_WAIT;
    timcon2.CONT_DET 	= lcm_params->dsi.CONT_DET;
    timcon2.CLK_ZERO	= (lcm_params->dsi.CLK_ZERO == 0) ? NS_TO_CYCLE(0x190, cycle_time) : lcm_params->dsi.CLK_ZERO;
    
    timcon3.CLK_HS_PRPR	= (lcm_params->dsi.CLK_HS_PRPR == 0) ? NS_TO_CYCLE(0x40, cycle_time) : lcm_params->dsi.CLK_HS_PRPR;
    if(timcon3.CLK_HS_PRPR == 0) 
       timcon3.CLK_HS_PRPR = 1;
    timcon3.CLK_HS_EXIT= (lcm_params->dsi.CLK_HS_EXIT == 0) ? (2 * timcon0.LPX) : lcm_params->dsi.CLK_HS_EXIT;
    timcon3.CLK_HS_POST= (lcm_params->dsi.CLK_HS_POST == 0) ? NS_TO_CYCLE((0x3c + 0x80 * ui), cycle_time) : lcm_params->dsi.CLK_HS_POST;
    
    printk("[DISP] - kernel - DSI_PHY_TIMCONFIG, HS_TRAIL = %d, HS_ZERO = %d, HS_PRPR = %d, LPX = %d, TA_GET = %d, TA_SURE = %d, TA_GO = %d, CLK_TRAIL = %d, CLK_ZERO = %d, CLK_HS_PRPR = %d \n", \
    timcon0.HS_TRAIL, timcon0.HS_ZERO, timcon0.HS_PRPR, timcon0.LPX, timcon1.TA_GET, timcon1.TA_SURE, timcon1.TA_GO, timcon2.CLK_TRAIL, timcon2.CLK_ZERO, timcon3.CLK_HS_PRPR);		
    
    DSI_OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,LPX,timcon0.LPX);
    DSI_OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_PRPR,timcon0.HS_PRPR);
    DSI_OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_ZERO,timcon0.HS_ZERO);
    DSI_OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_TRAIL,timcon0.HS_TRAIL);
 
    DSI_OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GO,timcon1.TA_GO);
    DSI_OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_SURE,timcon1.TA_SURE);
    DSI_OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GET,timcon1.TA_GET);
    DSI_OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,DA_HS_EXIT,timcon1.DA_HS_EXIT);
 
    DSI_OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CONT_DET,timcon2.CONT_DET);
    DSI_OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_ZERO,timcon2.CLK_ZERO);
    DSI_OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_TRAIL,timcon2.CLK_TRAIL);
 
    DSI_OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_PRPR,timcon3.CLK_HS_PRPR);
    DSI_OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_POST,timcon3.CLK_HS_POST);
    DSI_OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_EXIT,timcon3.CLK_HS_EXIT);
}


void DSI_clk_ULP_mode(BOOL enter)
{
    DSI_PHY_LCCON_REG tmp_reg1;
    //DSI_PHY_REG_ANACON0	tmp_reg2;
    
    tmp_reg1=DSI_REG->DSI_PHY_LCCON;
    //tmp_reg2=DSI_PHY_REG->ANACON0;
    
    if(enter) {
        tmp_reg1.LC_HS_TX_EN=0;
        DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
        mdelay(1);
        tmp_reg1.LC_ULPM_EN=1;
        DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
        mdelay(1);
        //tmp_reg2.PLL_EN=0;
        //OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&tmp_reg2));
    }
    else {
        //tmp_reg2.PLL_EN=1;
        //OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&tmp_reg2));
        mdelay(1);
        tmp_reg1.LC_ULPM_EN=0;
        DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
        mdelay(1);
        tmp_reg1.LC_WAKEUP_EN=1;
        DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
        mdelay(1);
        tmp_reg1.LC_WAKEUP_EN=0;
        DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
        mdelay(1);
    }
}


void DSI_clk_HS_mode(BOOL enter)
{
    DSI_PHY_LCCON_REG tmp_reg1 = DSI_REG->DSI_PHY_LCCON;
    
    if(enter && !DSI_clk_HS_state()) {
        tmp_reg1.LC_HS_TX_EN=1;
        DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
        //mdelay(1);
    }
    else if (!enter && DSI_clk_HS_state()) {
        tmp_reg1.LC_HS_TX_EN=0;
        DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
        //mdelay(1);
    }
}	


BOOL DSI_clk_HS_state(void)
{
    return DSI_REG->DSI_PHY_LCCON.LC_HS_TX_EN ? TRUE : FALSE;
}


void DSI_lane0_ULP_mode(BOOL enter)
{
    DSI_PHY_LD0CON_REG tmp_reg1;
    
    tmp_reg1=DSI_REG->DSI_PHY_LD0CON;
    
    if(enter) {
        // suspend
        tmp_reg1.L0_HS_TX_EN=0;
        DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
        mdelay(1);
        tmp_reg1.L0_ULPM_EN=1;
        DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
        mdelay(1);
    }
    else {
        // resume
        tmp_reg1.L0_ULPM_EN=0;
        DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
        mdelay(1);
        tmp_reg1.L0_WAKEUP_EN=1;
        DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
        mdelay(1);
        tmp_reg1.L0_WAKEUP_EN=0;
        DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
        mdelay(1);
    }
}

#ifndef BUILD_LK

// called by DPI ISR
void DSI_handle_esd_recovery(void)
{
#ifndef MT65XX_NEW_DISP
	long int dsi_current_time;
	unsigned int state;
	
	dsi_dpi_isr_count++;

	if (dsi_noncont_clk_enabled && (dsi_dpi_isr_count % dsi_noncont_clk_period)==0) {
		//DSI_handle_noncont_clk();
		{		
			if (!DSI_REG->DSI_MODE_CTRL.MODE)
				return ;
			
			dsi_current_time = get_current_time_us();

			state = DSI_REG->DSI_STATE_DBG3.TCON_STATE;	
		
			switch(state)
			{
				case DSI_VDO_VSA_VS_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_HS_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_vs_period_us)
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_HS_STATE, _dsiContext.vsa_vs_period_us);
							return ;
						}			
					}
					break;
			
				case DSI_VDO_VSA_HS_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VE_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_hs_period_us)
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VE_STATE, _dsiContext.vsa_hs_period_us);
							return ;
						}			
					}
					break;
			
				case DSI_VDO_VSA_VE_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VBP_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_ve_period_us)
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VBP_STATE, _dsiContext.vsa_ve_period_us);
							return ;
						}			
					}
					break;
			
				case DSI_VDO_VBP_STATE:
					xlog_printk(ANDROID_LOG_WARN, "DSI", "Can't do clock switch in DSI_VDO_VBP_STATE !!!\n");
					break;
			
				case DSI_VDO_VACT_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VFP_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VFP_STATE, _dsiContext.vfp_period_us );
							return ;
						}			
					}
					break;
			
				case DSI_VDO_VFP_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VS_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VS_STATE, _dsiContext.vfp_period_us );
							return ;
						}			
					}
					break;
			
				default :
					xlog_printk(ANDROID_LOG_ERROR, "DSI", "invalid state = %x \n", state);
					return ;
			}
			
			// Clock switch HS->LP->HS
			if(DSI_REG->DSI_STATE_DBG0.CTL_STATE_0 == 0x1)
			{
				spin_lock_irqsave(&g_handle_esd_lock, g_handle_esd_flag);
				
				DSI_clk_HS_mode(0);
				DSI_clk_HS_mode(1);

				spin_unlock_irqrestore(&g_handle_esd_lock, g_handle_esd_flag);

			}		

			return ;
		}		
	}

	if (dsi_int_te_enabled && (dsi_dpi_isr_count % dsi_int_te_period)==0) {
		spin_lock_irqsave(&g_handle_esd_lock, g_handle_esd_flag);

		//mt65xx_irq_mask(MT6575_DSI_IRQ_ID);
		disable_irq(MT_DSI_IRQ_ID);

		dsi_esd_recovery = DSI_handle_int_TE();

		//mt65xx_irq_unmask(MT6575_DSI_IRQ_ID);
		enable_irq(MT_DSI_IRQ_ID);

		spin_unlock_irqrestore(&g_handle_esd_lock, g_handle_esd_flag);
	}	   
#endif
}


// called by "esd_recovery_kthread"
BOOL DSI_esd_check(void)
{
#ifndef MT65XX_NEW_DISP
    BOOL result = false;
    
    if(dsi_esd_recovery)
        result = true;
    else
        result = false;
    
    dsi_esd_recovery = false;
    
    return result;
#endif
}


void DSI_set_int_TE(BOOL enable, unsigned int period)
{
#ifndef MT65XX_NEW_DISP
    dsi_int_te_enabled = enable;
    if(period<1)
        period = 1;
    dsi_int_te_period = period;
    dsi_dpi_isr_count = 0;
#endif
}


// called by DPI ISR.
BOOL DSI_handle_int_TE(void)
{
#ifndef MT65XX_NEW_DISP
	DSI_T0_INS t0;
	long int dsi_current_time;

	if (!DSI_REG->DSI_MODE_CTRL.MODE)
		return false;

	dsi_current_time = get_current_time_us();

	if(DSI_REG->DSI_STATE_DBG3.TCON_STATE == DSI_VDO_VFP_STATE)
	{
		udelay(_dsiContext.vfp_period_us / 2);

		if ((DSI_REG->DSI_STATE_DBG3.TCON_STATE == DSI_VDO_VFP_STATE) && DSI_REG->DSI_STATE_DBG0.CTL_STATE_0 == 0x1)
		{			
			// Can't do int. TE check while INUSE FB number is not 0 because later disable/enable DPI will set INUSE FB to number 0.
			if(DPI_REG->STATUS.FB_INUSE != 0)
				return false;
			
			DSI_clk_HS_mode(0);

			//DSI_REG->DSI_COM_CTRL.DSI_RESET = 1;
			OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
			DPI_DisableClk();
			DSI_SetMode(CMD_MODE);
			//DSI_REG->DSI_COM_CTRL.DSI_RESET = 0;
			OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
			//DSI_Reset();

			t0.CONFG = 0x20;		///TE
			t0.Data0 = 0;
			t0.Data_ID = 0;
			t0.Data1 = 0;
			
			OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
			OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);

			// Enable RD_RDY INT for polling it's status later
			//DSI_REG->DSI_INTEN.RD_RDY =  1;
			OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);

			DSI_EnableClk();

	        while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  // polling RD_RDY
	        {
				if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us)
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for internal TE time-out for %d (us)!!!\n", _dsiContext.vfp_period_us);

					///do necessary reset here
					//DSI_REG->DSI_RACK.DSI_RACK = 1;
					OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
					DSI_Reset();

					return true;
				}
	        }

			// Write clear RD_RDY
			//DSI_REG->DSI_INTSTA.RD_RDY = 1;
			//DSI_REG->DSI_RACK.DSI_RACK = 1;
			OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
			OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
			// Write clear CMD_DONE
			//DSI_REG->DSI_INTSTA.CMD_DONE = 1;
			OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);

			// Restart video mode. (with VSA ahead)
			DSI_SetMode(SYNC_PULSE_VDO_MODE);
			DSI_clk_HS_mode(1);
			DPI_EnableClk();
			DSI_EnableClk();
		}
	
	}
#endif
	return false;

}


void DSI_set_noncont_clk(BOOL enable, unsigned int period)
{
#ifndef MT65XX_NEW_DISP
	dsi_noncont_clk_enabled = enable;
	if(period<1)
		period = 1;
	dsi_noncont_clk_period = period;
	dsi_dpi_isr_count = 0;
#endif
}


// called by DPI ISR.
void DSI_handle_noncont_clk(void)
{
#ifndef MT65XX_NEW_DISP
	unsigned int state;
	long int dsi_current_time;

	if (!DSI_REG->DSI_MODE_CTRL.MODE)
		return ;

	state = DSI_REG->DSI_STATE_DBG3.TCON_STATE;

	dsi_current_time = get_current_time_us();

	switch(state)
	{
		case DSI_VDO_VSA_VS_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_HS_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_vs_period_us)
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_HS_STATE, _dsiContext.vsa_vs_period_us);
					return ;
				}			
			}
			break;

		case DSI_VDO_VSA_HS_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VE_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_hs_period_us)
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VE_STATE, _dsiContext.vsa_hs_period_us);
					return ;
				}			
			}
			break;

		case DSI_VDO_VSA_VE_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VBP_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_ve_period_us)
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VBP_STATE, _dsiContext.vsa_ve_period_us);
					return ;
				}			
			}
			break;

		case DSI_VDO_VBP_STATE:
			xlog_printk(ANDROID_LOG_WARN, "DSI", "Can't do clock switch in DSI_VDO_VBP_STATE !!!\n");
			break;

		case DSI_VDO_VACT_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VFP_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VFP_STATE, _dsiContext.vfp_period_us );
					return ;
				}			
			}
			break;

		case DSI_VDO_VFP_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VS_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VS_STATE, _dsiContext.vfp_period_us );
					return ;
				}			
			}
			break;

		default :
			xlog_printk(ANDROID_LOG_ERROR, "DSI", "invalid state = %x \n", state);
			return ;
	}

	// Clock switch HS->LP->HS
	DSI_clk_HS_mode(0);
	udelay(1);
	DSI_clk_HS_mode(1);
#endif
}
#endif

DSI_STATUS DSI_EnableVM_CMD(void)
{
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,VM_CMD_START,0);
    DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,VM_CMD_START,1);

    return DSI_STATUS_OK;
}

void DSI_set_cmdq_V2(unsigned cmd, unsigned char count, unsigned char *para_list, unsigned char force_update)
{
    UINT32 i;
    UINT32 goto_addr, mask_para, set_para;
    DSI_T0_INS t0;	
    DSI_T2_INS t2;	

    if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
        DSI_VM_CMD_CON_REG vm_cmdq;
        DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &vm_cmdq, &DSI_REG->DSI_VM_CMD_CON);
        printf("set cmdq in VDO mode in set_cmdq_V2\n");

        if (cmd < 0xB0)
        {
            if (count > 1)
            {
                vm_cmdq.LONG_PKT = 1;
                vm_cmdq.CM_DATA_ID = DSI_DCS_LONG_PACKET_ID;
                vm_cmdq.CM_DATA_0 = count+1;
                DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
                
                goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
                mask_para = (0xFF<<((goto_addr&0x3)*8));
                set_para = (cmd<<((goto_addr&0x3)*8));
                MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                
                for(i=0; i<count; i++)
                {
                    goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (para_list[i]<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                }
            }
            else
            {
                vm_cmdq.LONG_PKT = 0;
                vm_cmdq.CM_DATA_0 = cmd;
                if (count)
                {
                    vm_cmdq.CM_DATA_ID = DSI_DCS_SHORT_PACKET_ID_1;
                    vm_cmdq.CM_DATA_1 = para_list[0];
                }
                else
                {
                    vm_cmdq.CM_DATA_ID = DSI_DCS_SHORT_PACKET_ID_0;
                    vm_cmdq.CM_DATA_1 = 0;
                }
                DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
            }
        }
        else{
            if (count > 1)
            {
                vm_cmdq.LONG_PKT = 1;
                vm_cmdq.CM_DATA_ID = DSI_GERNERIC_LONG_PACKET_ID;
                vm_cmdq.CM_DATA_0 = count+1;
                DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
                
                goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
                mask_para = (0xFF<<((goto_addr&0x3)*8));
                set_para = (cmd<<((goto_addr&0x3)*8));
                MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                
                for(i=0; i<count; i++)
                {
                    goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (para_list[i]<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                }
            }
            else
            {
                vm_cmdq.LONG_PKT = 0;
                vm_cmdq.CM_DATA_0 = cmd;
                if (count)
                {
                    vm_cmdq.CM_DATA_ID = DSI_GERNERIC_SHORT_PACKET_ID_2;
                    vm_cmdq.CM_DATA_1 = para_list[0];
                }
                else
                {
                    vm_cmdq.CM_DATA_ID = DSI_GERNERIC_SHORT_PACKET_ID_1;
                    vm_cmdq.CM_DATA_1 = 0;
                }
                DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
            }
        }
        //start DSI VM CMDQ
        if(force_update){
            DSI_EnableVM_CMD();
        }
    }
    else{
        DSI_WaitForEngineNotBusy();
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
            DSI_EnableClk();
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
            mdelay(count);
            DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V3[%d]. Delay %d (ms) \n", index, count);
            
            continue;
        }
        if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
            DSI_VM_CMD_CON_REG vm_cmdq;
            DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &vm_cmdq, &DSI_REG->DSI_VM_CMD_CON);
            printk("set cmdq in VDO mode\n");
            if (count > 1)
            {
                vm_cmdq.LONG_PKT = 1;
                vm_cmdq.CM_DATA_ID = data_id;
                vm_cmdq.CM_DATA_0 = count+1;
                DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
                
                goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
                mask_para = (0xFF<<((goto_addr&0x3)*8));
                set_para = (cmd<<((goto_addr&0x3)*8));
                MASKREG32(goto_addr&(~0x3), mask_para, set_para);
                
                for(i=0; i<count; i++)
                {
                    goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
                    mask_para = (0xFF<<((goto_addr&0x3)*8));
                    set_para = (para_list[i]<<((goto_addr&0x3)*8));
                    MASKREG32(goto_addr&(~0x3), mask_para, set_para);			
                }
            }
            else
            {
                vm_cmdq.LONG_PKT = 0;
                vm_cmdq.CM_DATA_0 = cmd;
                if (count)
                {
                    vm_cmdq.CM_DATA_ID = data_id;
                    vm_cmdq.CM_DATA_1 = para_list[0];
                }
                else
                {
                    vm_cmdq.CM_DATA_ID = data_id;
                    vm_cmdq.CM_DATA_1 = 0;
                }
                DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
            }
            //start DSI VM CMDQ
            if(force_update){
                DSI_EnableVM_CMD();
            }
        }
        else{
            DSI_WaitForEngineNotBusy();
            {
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
                    DSI_EnableClk();
            }
        }
    } while (++index < size);
}

void DSI_set_cmdq(unsigned int *pdata, unsigned int queue_size, unsigned char force_update)
{
    UINT32 i;
    
    if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
        DSI_VM_CMD_CON_REG vm_cmdq;

        DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &vm_cmdq, &DSI_REG->DSI_VM_CMD_CON);
        printk("set cmdq in VDO mode\n");

        if(queue_size > 1){//long packet
            unsigned int i = 0;

            vm_cmdq.LONG_PKT = 1;
            vm_cmdq.CM_DATA_ID = ((pdata[0] >> 8) & 0xFF);
            vm_cmdq.CM_DATA_0 = ((pdata[0] >> 16) & 0xFF);
            DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
            for(i=0;i<queue_size-1;i++)
                OUTREG32(&DSI_VM_CMD_REG->data[i], AS_UINT32((pdata+i+1)));
        }
        else{
            vm_cmdq.LONG_PKT = 0;
            vm_cmdq.CM_DATA_ID = ((pdata[0] >> 8) & 0xFF);
            vm_cmdq.CM_DATA_0 = ((pdata[0] >> 16) & 0xFF);
            vm_cmdq.CM_DATA_1 = ((pdata[0] >> 24) & 0xFF);
            DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
        }
        //start DSI VM CMDQ
        if(force_update){
            DSI_EnableVM_CMD();
        }
    }
    else{
        ASSERT(queue_size<=32);
        DSI_WaitForEngineNotBusy();
        
        for(i=0; i<queue_size; i++)
            OUTREG32(&DSI_CMDQ_REG->data[i], AS_UINT32((pdata+i)));
        
        DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, queue_size);
        
        //for (i = 0; i < queue_size; i++)
        //   printf("[DISP] - kernel - DSI_set_cmdq. DSI_CMDQ+%04x : 0x%08x\n", i*4, INREG32(DSI_BASE + 0x180 + i*4));
        
        if(force_update)
            DSI_EnableClk();
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
    
    for(i=0;i<(((unsigned int)t2->WC16-1)>>2)+1;i++)
        OUTREG32(&DSI_CMDQ_REG->data[1+i], AS_UINT32((t2->pdata+i)));
    
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
                                  unsigned int  max_return_size)
{
    DSI_TXRX_CTRL_REG tmp_reg;
    
    tmp_reg=DSI_REG->DSI_TXRX_CTRL;

    ///TODO: parameter checking
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
    tmp_reg.HSTX_CKLP_EN = 0;//need customization???
    DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&DSI_REG->DSI_TXRX_CTRL, &tmp_reg);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PS_Control(unsigned int ps_type, unsigned int vact_line, unsigned int ps_wc)
{
    DSI_PSCTRL_REG tmp_reg;
    tmp_reg=DSI_REG->DSI_PSCTRL;
    
    ///TODO: parameter checking
    ASSERT(ps_type <= PACKED_PS_18BIT_RGB666);
    if(ps_type>LOOSELY_PS_18BIT_RGB666)
        tmp_reg.DSI_PS_SEL=(5 - ps_type);
    else
        tmp_reg.DSI_PS_SEL=ps_type;

    tmp_reg.DSI_PS_WC=ps_wc;

    DSI_OUTREG32_V(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, AS_UINT32(&vact_line));
    DSI_OUTREG32_R(PDSI_PSCTRL_REG,&DSI_REG->DSI_PSCTRL, &tmp_reg);
    DSI_OUTREGBIT(DSI_HSTX_CKLP_WC_REG, DSI_REG->DSI_HSTX_CKLP_WC, HSTX_CKLP_WC_AUTO, 1);

    return DSI_STATUS_OK;
}


void DSI_write_lcm_cmd(unsigned int cmd)
{
    DSI_T0_INS *t0_tmp=0;
    DSI_CMDQ_CONFG CONFG_tmp;
    
    CONFG_tmp.type=SHORT_PACKET_RW;
    CONFG_tmp.BTA=DISABLE_BTA;
    CONFG_tmp.HS=LOW_POWER;
    CONFG_tmp.CL=CL_8BITS;
    CONFG_tmp.TE=DISABLE_TE;
    CONFG_tmp.RPT=DISABLE_RPT;
    
    t0_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
    t0_tmp->Data_ID= (cmd&0xFF);
    t0_tmp->Data0 = 0x0;
    t0_tmp->Data1 = 0x0;	
    
    DSI_Write_T0_INS(t0_tmp);
}


void DSI_write_lcm_regs(unsigned int addr, unsigned int *para, unsigned int nums)
{
    DSI_T2_INS *t2_tmp=0;
    DSI_CMDQ_CONFG CONFG_tmp;
    
    CONFG_tmp.type=LONG_PACKET_W;
    CONFG_tmp.BTA=DISABLE_BTA;
    CONFG_tmp.HS=LOW_POWER;
    CONFG_tmp.CL=CL_8BITS;
    CONFG_tmp.TE=DISABLE_TE;
    CONFG_tmp.RPT=DISABLE_RPT;
    
    t2_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
    t2_tmp->Data_ID = (addr&0xFF);
    t2_tmp->WC16 = nums;	
    t2_tmp->pdata = para;	
    
    DSI_Write_T2_INS(t2_tmp);
}

UINT32 DSI_dcs_read_lcm_reg(UINT8 cmd)
{
    UINT32 recv_data  = 0;
#if 0
    UINT32 max_try_count = 5;
    UINT32 recv_data_cnt;
    unsigned int read_timeout_ms;
    unsigned char packet_type;
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
        
       DSI_WaitForEngineNotBusy();

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

#if ENABLE_DSI_INTERRUPT
    static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
    long ret;
#endif

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
        
       DSI_WaitForEngineNotBusy();

       t0.CONFG = 0x04;        ///BTA
       t0.Data0 = cmd;
       if (buffer_size < 0x3)
           t0.Data_ID = DSI_DCS_READ_PACKET_ID;
       else
           t0.Data_ID = DSI_GERNERIC_READ_LONG_PACKET_ID;
       t0.Data1 = 0;

       DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
       DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

       ///clear read ACK 
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
#if ENABLE_DSI_INTERRUPT
       ret = wait_event_interruptible_timeout(_dsi_dcs_read_wait_queue, 
                                                       !_IsEngineBusy(),
                                                       WAIT_TIMEOUT);
        if (0 == ret) 
        {
            xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine read ready timeout!!!\n");

            DSI_DumpRegisters();
            
            ///do necessary reset here
            DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
            DSI_Reset();

            return 0;
        }
#else
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

       ///clear interrupt status
       DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
       ///STOP DSI
       DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);

#endif

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
	DSI_T1_INS *t1_tmp=0;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=FB_WRITE;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=HIGH_SPEED;

	if(long_length)
		CONFG_tmp.CL=CL_16BITS;
	else
		CONFG_tmp.CL=CL_8BITS;		

	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;


	t1_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
	t1_tmp->Data_ID= 0x39;
	t1_tmp->mem_start0 = (addr&0xFF);	

	if(long_length)
		t1_tmp->mem_start1 = ((addr>>8)&0xFF);

	return DSI_Write_T1_INS(t1_tmp);	

	
}


DSI_STATUS DSI_read_lcm_fb(void)
{
	// TBD
	return DSI_STATUS_OK;
}

DSI_STATUS DSI_enable_MIPI_txio(BOOL en)
{
#if 0
    if(en)
    {
        *(volatile unsigned int *) (INFRACFG_BASE+0x890) |= 0x00000100;    // enable MIPI TX IO    
    }
    else
    {
        *(volatile unsigned int *) (INFRACFG_BASE+0x890) &= ~0x00000100;   // disable MIPI TX IO   
    }
#endif
    return DSI_STATUS_OK;
}


BOOL Need_Wait_ULPS(void)
{
    return FALSE;
}


DSI_STATUS Wait_ULPS_Mode(void)
{
    return DSI_STATUS_OK;
}


DSI_STATUS Wait_WakeUp(void)
{
	return DSI_STATUS_OK;
}

// -------------------- Retrieve Information --------------------

DSI_STATUS DSI_DumpRegisters(void)
{
    UINT32 i;

    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "---------- Start dump DSI registers ----------\n");
    
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


static LCM_PARAMS lcm_params_for_clk_setting;


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

    DSI_WaitForEngineNotBusy();
	DSI_PHY_TIMCONFIG(lcm_params);
	DSI_PHY_clk_setting(lcm_params);
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
    MIPITX_DSI_PLL_TOP_REG mipitx_dsi_pll_top = DSI_PHY_REG->MIPITX_DSI_PLL_TOP;
    MIPITX_DSI_PLL_CON2_REG mipitx_dsi_pll_con2 = DSI_PHY_REG->MIPITX_DSI_PLL_CON2;
    MIPITX_DSI_PLL_CON0_REG mipitx_dsi_pll_con0 = DSI_PHY_REG->MIPITX_DSI_PLL_CON0;

    *clk = ((((26 * mipitx_dsi_pll_con2.RG_DSI0_MPPLL_SDM_PCW_H * 8 / 4)
                     / (mipitx_dsi_pll_top.RG_MPPLL_PRESERVE_L + 1))
                     << mipitx_dsi_pll_con0.RG_DSI0_MPPLL_TXDIV0)
                     << mipitx_dsi_pll_con0.RG_DSI0_MPPLL_TXDIV1);

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
    DSI_WaitForEngineNotBusy();
	DSI_PHY_TIMCONFIG(&lcm_params_for_clk_setting);
	DSI_PHY_clk_setting(&lcm_params_for_clk_setting);

    return DSI_STATUS_OK;
}


