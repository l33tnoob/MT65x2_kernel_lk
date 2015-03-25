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
#define ENABLE_DPI_INTERRUPT        0
#define ENABLE_DPI_REFRESH_RATE_LOG 0

#include <platform/disp_drv_platform.h>
#include <platform/mt_gpt.h>
#include <string.h>
#else

#define ENABLE_DPI_INTERRUPT        1
#define ENABLE_DPI_REFRESH_RATE_LOG 0

#if ENABLE_DPI_REFRESH_RATE_LOG && !ENABLE_DPI_INTERRUPT
#error "ENABLE_DPI_REFRESH_RATE_LOG should be also ENABLE_DPI_INTERRUPT"
#endif

#if defined(MTK_HDMI_SUPPORT) && !ENABLE_DPI_INTERRUPT
#error "enable MTK_HDMI_SUPPORT should be also ENABLE_DPI_INTERRUPT"
#endif

#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/io.h>
#include <disp_drv_log.h>

#include <mach/mt6577_typedefs.h>
#include <mach/mt6577_reg_base.h>
#include <mach/mt6577_irq.h>
#include <mach/mt6577_clock_manager.h>

#include "dpi_reg.h"
#include "dsi_reg.h"
#include "dpi_drv.h"
#include "lcd_drv.h"

#if ENABLE_DPI_INTERRUPT
#include <linux/interrupt.h>
#include <linux/wait.h>

#include <mach/irqs.h>
#include "mtkfb.h"
#endif

#include <linux/module.h>
#endif
//#include <platform/pll.h>
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

#define DPI_OUTREG32_R(type, addr2, addr1) 	                                 \
		{                                                                    \
			union p_regs                                                     \
			{                                                                \
				type p_reg;                                         	     \
				unsigned int * p_uint;                              		 \
			}p_temp1,p_temp2;                                                \
			p_temp1.p_reg  = (type)(addr2);                                  \
			p_temp2.p_reg  = (type)(addr1);                                  \
			OUTREG32(p_temp1.p_uint,INREG32(p_temp2.p_uint));}

#define DPI_OUTREG32_V(type, addr2, var) 	                                 \
		{                                                                    \
			union p_regs                                                     \
			{                                                                \
				type p_reg;                                    			     \
				unsigned int * p_uint;                          		     \
			}p_temp1;                                                        \
			p_temp1.p_reg  = (type)(addr2);                                  \
			OUTREG32(p_temp1.p_uint,var);}


static PDPI_REGS const DPI_REG = (PDPI_REGS)(DPI_BASE);
static PDSI_PHY_REGS const DSI_PHY_REG_DPI = (PDSI_PHY_REGS)(MIPI_CONFIG_BASE + 0x800);
static BOOL s_isDpiPowerOn = FALSE;
static DPI_REGS regBackup;
static void (*dpiIntCallback)(DISP_INTERRUPT_EVENTS);

#define DPI_REG_OFFSET(r)       offsetof(DPI_REGS, r)
#define REG_ADDR(base, offset)  (((BYTE *)(base)) + (offset))

#if !(defined(CONFIG_MT6589_FPGA) || defined(BUILD_UBOOT))
//#define DPI_MIPI_API
#endif

const UINT32 BACKUP_DPI_REG_OFFSETS[] =
{
    DPI_REG_OFFSET(INT_ENABLE),
    DPI_REG_OFFSET(SIZE),
    DPI_REG_OFFSET(CLK_CNTL),
//    DPI_REG_OFFSET(DITHER),

    DPI_REG_OFFSET(TGEN_HWIDTH),
    DPI_REG_OFFSET(TGEN_HPORCH),

	DPI_REG_OFFSET(TGEN_VWIDTH_LODD),
    DPI_REG_OFFSET(TGEN_VPORCH_LODD),

//    DPI_REG_OFFSET(TGEN_VWIDTH_LEVEN),
//    DPI_REG_OFFSET(TGEN_VPORCH_LEVEN),
//    DPI_REG_OFFSET(TGEN_VWIDTH_RODD),
//
//    DPI_REG_OFFSET(TGEN_VPORCH_RODD),
//    DPI_REG_OFFSET(TGEN_VWIDTH_REVEN),

//	DPI_REG_OFFSET(TGEN_VPORCH_REVEN),
//    DPI_REG_OFFSET(ESAV_VTIML),
//    DPI_REG_OFFSET(ESAV_VTIMR),
//	DPI_REG_OFFSET(ESAV_FTIM),
	DPI_REG_OFFSET(BG_HCNTL),
  
  	DPI_REG_OFFSET(BG_VCNTL),
    DPI_REG_OFFSET(BG_COLOR),
//	DPI_REG_OFFSET(TGEN_POL),
//	DPI_REG_OFFSET(EMBSYNC_SETTING),

    DPI_REG_OFFSET(CNTL),
};

static void _BackupDPIRegisters(void)
{
    DPI_REGS *reg = &regBackup;
    UINT32 i;
    
    for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i])));
    }
}

static void _RestoreDPIRegisters(void)
{
    DPI_REGS *reg = &regBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i])));
    }
}

static void _ResetBackupedDPIRegisterValues(void)
{
    DPI_REGS *regs = &regBackup;
    memset((void*)regs, 0, sizeof(DPI_REGS));

    DPI_OUTREG32_V(PDPI_REG_CLKCNTL,&regs->CLK_CNTL, 0x00000101);
}


#if ENABLE_DPI_REFRESH_RATE_LOG
static void _DPI_LogRefreshRate(DPI_REG_INTERRUPT status)
{
    static unsigned long prevUs = 0xFFFFFFFF;

    if (status.VSYNC)
    {
        struct timeval curr;
        do_gettimeofday(&curr);

        if (prevUs < curr.tv_usec)
        {
            DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "Receive 1 vsync in %lu us\n", 
                   curr.tv_usec - prevUs);
        }
        prevUs = curr.tv_usec;
    }
}
#else
#define _DPI_LogRefreshRate(x)  do {} while(0)
#endif

extern void dsi_handle_esd_recovery(void);

void DPI_DisableIrq(void)
{
#if ENABLE_DPI_INTERRUPT
		DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;
		enInt.FIFO_EMPTY = 0;
		enInt.FIFO_FULL = 0;
		enInt.OUT_EMPTY = 0;
		enInt.CNT_OVERFLOW = 0;
		enInt.LINE_ERR = 0;
		enInt.VSYNC = 0;
		OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
#endif
}
void DPI_EnableIrq(void)
{
#if ENABLE_DPI_INTERRUPT
		DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;
		enInt.FIFO_EMPTY = 1;
		enInt.FIFO_FULL = 0;
		enInt.OUT_EMPTY = 0;
		enInt.CNT_OVERFLOW = 0;
		enInt.LINE_ERR = 0;
		enInt.VSYNC = 1;
		OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
#endif
}

#if ENABLE_DPI_INTERRUPT
static irqreturn_t _DPI_InterruptHandler(int irq, void *dev_id)
{   
    static int counter = 0;
    DPI_REG_INTERRUPT status = DPI_REG->INT_STATUS;
//    if (status.FIFO_EMPTY) ++ counter;

    if(status.VSYNC)
    {
        if(dpiIntCallback)
           dpiIntCallback(DISP_DPI_VSYNC_INT);
#ifndef BUILD_UBOOT
		if(wait_dpi_vsync){
			if(-1 != hrtimer_try_to_cancel(&hrtimer_vsync_dpi)){
				dpi_vsync = true;
//			hrtimer_try_to_cancel(&hrtimer_vsync_dpi);
				wake_up_interruptible(&_vsync_wait_queue_dpi);
			}
		}
#endif
    }

    if (status.VSYNC && counter) {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "[Error] DPI FIFO is empty, "
               "received %d times interrupt !!!\n", counter);
        counter = 0;
    }

    _DPI_LogRefreshRate(status);
	OUTREG32(&DPI_REG->INT_STATUS, 0);
    return IRQ_HANDLED;
}
#endif

#define VSYNC_US_TO_NS(x) (x * 1000)
unsigned int vsync_timer_dpi = 0;
void DPI_WaitVSYNC(void)
{
#ifndef BUILD_UBOOT
	wait_dpi_vsync = true;
	hrtimer_start(&hrtimer_vsync_dpi, ktime_set(0, VSYNC_US_TO_NS(vsync_timer_dpi)), HRTIMER_MODE_REL);
	wait_event_interruptible(_vsync_wait_queue_dpi, dpi_vsync);
	dpi_vsync = false;
	wait_dpi_vsync = false;
#endif
}

void DPI_PauseVSYNC(BOOL enable)
{
}

#ifndef BUILD_UBOOT
enum hrtimer_restart dpi_vsync_hrtimer_func(struct hrtimer *timer)
{
//	long long ret;
	if(wait_dpi_vsync)
	{
		dpi_vsync = true;
		wake_up_interruptible(&_vsync_wait_queue_dpi);
//		printk("hrtimer Vsync, and wake up\n");
	}
//	ret = hrtimer_forward_now(timer, ktime_set(0, VSYNC_US_TO_NS(vsync_timer_dpi)));
//	printk("hrtimer callback\n");
    return HRTIMER_NORESTART;
}
#endif
void DPI_InitVSYNC(unsigned int vsync_interval)
{
#ifndef BUILD_UBOOT
    ktime_t ktime;
	vsync_timer_dpi = vsync_interval;
	ktime = ktime_set(0, VSYNC_US_TO_NS(vsync_timer_dpi));
	hrtimer_init(&hrtimer_vsync_dpi, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_vsync_dpi.function = dpi_vsync_hrtimer_func;
//	hrtimer_start(&hrtimer_vsync_dpi, ktime, HRTIMER_MODE_REL);
#endif
}
void  DPI_MIPI_clk_setting(unsigned int mipi_pll_clk_ref,unsigned int mipi_pll_clk_div1,unsigned int mipi_pll_clk_div2){

	UINT32 i=0;
	//UINT32 j=0;

	UINT32 txmul  = 0;  
	UINT32 txdiv0  = 0;  
	UINT32 txdiv1  = 0;
	UINT32 posdiv  = 0;
	UINT32 prediv  = 0;

	UINT32 sdm_ssc_en     = 0;
	UINT32 sdm_ssc_prd    = 0;  // 0~65535
	UINT32 sdm_ssc_delta1 = 0;  // 0~65535
	UINT32 sdm_ssc_delta  = 0;  // 0~65535  
	
       UINT32   loopback_en = 0;
       UINT32   lane0_en = 1;
       UINT32   lane1_en = 1;
       UINT32   lane2_en = 1;
       UINT32   lane3_en = 1; 

       txmul = mipi_pll_clk_ref ;
       posdiv = mipi_pll_clk_div1;
	prediv   = mipi_pll_clk_div2;

	   
	//initial MIPI PLL
	OUTREG32((MIPI_CONFIG_BASE+0x050), 0x0);
	OUTREG32((MIPI_CONFIG_BASE+0x068), 0x2);
	OUTREG32((MIPI_CONFIG_BASE+0x044), 0x88492480);
	OUTREG32((MIPI_CONFIG_BASE+0x000), 0x400);
	OUTREG32((MIPI_CONFIG_BASE+0x054), 0x2);
	OUTREG32((MIPI_CONFIG_BASE+0x058), 0x0);
	OUTREG32((MIPI_CONFIG_BASE+0x05C), 0x0);

	OUTREG32((MIPI_CONFIG_BASE+0x004), 0x820);
	OUTREG32((MIPI_CONFIG_BASE+0x008), 0x400);
	OUTREG32((MIPI_CONFIG_BASE+0x00C), 0x100);
	OUTREG32((MIPI_CONFIG_BASE+0x010), 0x100);
	OUTREG32((MIPI_CONFIG_BASE+0x014), 0x100);
	
	OUTREG32((MIPI_CONFIG_BASE+0x040), 0x80);
	OUTREG32((MIPI_CONFIG_BASE+0x064), 0x0);
	OUTREG32((MIPI_CONFIG_BASE+0x074), 0x0);
	OUTREG32((MIPI_CONFIG_BASE+0x080), 0x0);
	OUTREG32((MIPI_CONFIG_BASE+0x084), 0x0);
	OUTREG32((MIPI_CONFIG_BASE+0x088), 0x0);
	OUTREG32((MIPI_CONFIG_BASE+0x090), 0x0);

	OUTREG32((MIPI_CONFIG_BASE+0x064), 0x300);

	printk("[DPI] MIPIPLL Initialed 222\n");

	//Setting MIPI PLL
	OUTREG32((MIPI_CONFIG_BASE+0x068), 0x3);
	OUTREG32((MIPI_CONFIG_BASE+0x068), 0x1);
	OUTREG32((MIPI_CONFIG_BASE+0x044), INREG32((MIPI_CONFIG_BASE+0x044)) | 0x00000013);
	OUTREG32((MIPI_CONFIG_BASE+0x040), (INREG32((MIPI_CONFIG_BASE+0x040)) | 0x00000002));
	OUTREG32((MIPI_CONFIG_BASE+0x000), ((INREG32((MIPI_CONFIG_BASE+0x000)) & 0xfffffbff ) | 0x00000003));
	OUTREG32((MIPI_CONFIG_BASE+0x050), ((prediv << 1) |(txdiv0 << 3) |(txdiv1 << 5) |(posdiv << 7)) );
	OUTREG32((MIPI_CONFIG_BASE+0x054), (0x3 | (sdm_ssc_en<<2) | (sdm_ssc_prd<<16)) );
	OUTREG32((MIPI_CONFIG_BASE+0x058), txmul);
	OUTREG32((MIPI_CONFIG_BASE+0x05C), ((sdm_ssc_delta<<16) | sdm_ssc_delta1));

	OUTREG32((MIPI_CONFIG_BASE+0x004), INREG32(MIPI_CONFIG_BASE+0x004) |0x00000001 |(loopback_en<<1));
	if(lane0_en)
		OUTREG32((MIPI_CONFIG_BASE+0x008), INREG32(MIPI_CONFIG_BASE+0x008) |0x00000001 |(loopback_en<<1));
	if(lane1_en)
		OUTREG32((MIPI_CONFIG_BASE+0x00C), INREG32(MIPI_CONFIG_BASE+0x00C) |0x00000001 |(loopback_en<<1));
	if(lane2_en)
		OUTREG32((MIPI_CONFIG_BASE+0x010), INREG32(MIPI_CONFIG_BASE+0x010) |0x00000001 |(loopback_en<<1));
	if(lane3_en)
		OUTREG32((MIPI_CONFIG_BASE+0x014), INREG32(MIPI_CONFIG_BASE+0x014) |0x00000001 |(loopback_en<<1));
	
	OUTREG32((MIPI_CONFIG_BASE+0x050), (INREG32((MIPI_CONFIG_BASE+0x050)) | 0x1));
	OUTREG32((MIPI_CONFIG_BASE+0x060), 0);
	OUTREG32((MIPI_CONFIG_BASE+0x060), 1);

	for(i=0; i<100; i++)   // wait for PLL stable
	{
		//j = INREG32((MIPI_CONFIG_BASE+0x050));
		INREG32((MIPI_CONFIG_BASE+0x050));
	}
	printk("[DPI] MIPIPLL Exit\n");
}


DPI_STATUS DPI_Init(BOOL isDpiPoweredOn)
{
    //DPI_REG_CNTL cntl;

    if (isDpiPoweredOn) {
        _BackupDPIRegisters();
    } else {
        _ResetBackupedDPIRegisterValues();
    }

    DPI_PowerOn();
	
//    unsigned int reg_value = 0;

    OUTREG32(0x10005000 + 0xCC4, 0x0000000f);
    OUTREG32(0x10005000 + 0xB18, 0x00000ff0);
    OUTREG32(0x10005000 + 0xB14, 0x00000660);
    OUTREG32(0x10005000 + 0xB28, 0x000000ff);
    OUTREG32(0x10005000 + 0xB24, 0x00000066);
//	OUTREG32(0x1400f000,0x1);
//	OUTREG32(0x1400f000,0x41);
	
	OUTREG32(0x1400f000,0x1);
	OUTREG32(0x1400f000,0x41);
	
/*
	OUTREG32(DPI_BASE+ 0x64, 0x400);//
	OUTREG32(DPI_BASE+ 0x6C, 0x400);//
	OUTREG32(DPI_BASE+ 0x74, 0x400);//
	OUTREG32(DPI_BASE+ 0x8C, 0x0FFF0000);//
	OUTREG32(DPI_BASE+ 0x90, 0x0FFF0000);//
	MASKREG32(DISPSYS_BASE + 0x60, 0x1, 0x1);
	*/
#if ENABLE_DPI_INTERRUPT
    if (request_irq(MT6589_DPI_IRQ_ID,
        _DPI_InterruptHandler, IRQF_TRIGGER_LOW, "mtkdpi", NULL) < 0)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "[ERROR] fail to request DPI irq\n"); 
        return DPI_STATUS_ERROR;
    }

    {
        DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;
        enInt.VSYNC = 1;
        OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
    }
#endif
	LCD_W2M_NeedLimiteSpeed(TRUE);
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_Init);

DPI_STATUS DPI_Deinit(void)
{
    DPI_DisableClk();
    DPI_PowerOff();

    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_Deinit);

void DPI_mipi_switch(BOOL on)
{
	if(on)
	{
	// may call enable_mipi(), but do this in DPI_Init_PLL
	}
	else
	{
#ifdef DPI_MIPI_API 
		disable_mipi(MT65XX_MIPI_TX, "DPI");
#endif
	}	
}

#ifndef BULID_UBOOT
extern UINT32 FB_Addr;
#endif
DPI_STATUS DPI_Init_PLL(unsigned int mipi_pll_clk_ref,unsigned int mipi_pll_clk_div1,unsigned int mipi_pll_clk_div2)
{

DPI_MIPI_clk_setting( mipi_pll_clk_ref,  mipi_pll_clk_div1, mipi_pll_clk_div2);

    //MASKREG32(CLK_CFG_5, 0x400, 0x400);  // CLK_CFG_5[10] rg_lvds_tv_sel
                                          // 0:dpi0_ck from tvhdmi pll
                                          // 1:dpi0_ck from lvds pll

    //MASKREG32(CLK_CFG_7, 0x07000000, 0x01000000); // CLK_CFG_7[26:24] lvdspll clock divider selection
                                                   // 0: from 26M
                                                   // 1: lvds_pll_ck
                                                   // 2: lvds_pll_ck/2
                                                   // 3: lvds_pll_ck/4
                                                   // 4: lvds_pll_ck/8
                                                   // CLK_CFG_7[31] 0: clock on, 1: clock off

    //DRV_SetReg32(LVDSPLL_PWR_CON0, (0x1 << 0));	//PLL_PWR_ON
	udelay(2);
	//DRV_ClrReg32(LVDSPLL_PWR_CON0, (0x1 << 1));	//PLL_ISO_EN

	//OUTREG32(LVDSPLL_CON1, mipi_pll_clk_div2);
	//OUTREG32(LVDSPLL_CON0, mipi_pll_clk_div1);
	udelay(20);	  

	return DPI_STATUS_OK;
}

//EXPORT_SYMBOL(DPI_Init_PLL);

DPI_STATUS DPI_Set_DrivingCurrent(LCM_PARAMS *lcm_params)
{
	DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "DPI_Set_DrivingCurrent not implement for 6575");
	return DPI_STATUS_OK;
}

#ifdef BUILD_LK
DPI_STATUS DPI_PowerOn()
{
    if (!s_isDpiPowerOn)
    {
#if 1   // FIXME
        MASKREG32(0x14000110, 0x40, 0x0);//dpi0 clock gate clear
#endif        
        _RestoreDPIRegisters();
        s_isDpiPowerOn = TRUE;
    }

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_PowerOff()
{
    if (s_isDpiPowerOn)
    {
        BOOL ret = TRUE;
        _BackupDPIRegisters();
#if 1   // FIXME
        MASKREG32(0x14000110, 0x40, 0x40);//dpi0 clock gate setting
#endif        
        ASSERT(ret);
        s_isDpiPowerOn = FALSE;
    }

    return DPI_STATUS_OK;
}

#else

DPI_STATUS DPI_PowerOn()
{
#ifndef CONFIG_MT6589_FPGA
    if (!s_isDpiPowerOn)
    {
#if 0   // FIXME
        int ret = enable_clock(MT65XX_PDN_MM_DPI, "DPI");
		if(1 == ret)
		{
			DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "power manager API return FALSE\n");
		}
#endif 
		enable_pll(LVDSPLL, "dpi0");
        _RestoreDPIRegisters();
        s_isDpiPowerOn = TRUE;
    }
#endif
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_PowerOff()
{
#ifndef CONFIG_MT6589_FPGA
    if (s_isDpiPowerOn)
    {
        int ret = TRUE;
        _BackupDPIRegisters();
		disable_pll(LVDSPLL, "dpi0");
#if 0   // FIXME
        ret = disable_clock(MT65XX_PDN_MM_DPI, "DPI");
		if(1 == ret)
		{
			DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "power manager API return FALSE\n");
		}
#endif        
        s_isDpiPowerOn = FALSE;
    }
#endif
    return DPI_STATUS_OK;
}
#endif
//EXPORT_SYMBOL(DPI_PowerOn);

//EXPORT_SYMBOL(DPI_PowerOff);

DPI_STATUS DPI_EnableClk()
{
	DPI_REG_EN en = DPI_REG->DPI_EN;
    en.EN = 1;
    DPI_OUTREG32_R(PDPI_REG_EN,&DPI_REG->DPI_EN, &en);
   //release mutex0
//#ifndef BUILD_UBOOT
#if 0
    OUTREG32(DISP_MUTEX_BASE + 0x24, 0);
    while((INREG32(DISP_MUTEX_BASE + 0x24)&0x02)!=0){} // polling until mutex lock complete
#endif
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_EnableClk);

DPI_STATUS DPI_DisableClk()
{
    DPI_REG_EN en = DPI_REG->DPI_EN;
    en.EN = 0;
    DPI_OUTREG32_R(PDPI_REG_EN,&DPI_REG->DPI_EN,&en);

    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_DisableClk);

DPI_STATUS DPI_EnableSeqOutput(BOOL enable)
{
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_EnableSeqOutput);

DPI_STATUS DPI_SetRGBOrder(DPI_RGB_ORDER input, DPI_RGB_ORDER output)
{
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_SetRGBOrder);

DPI_STATUS DPI_ConfigPixelClk(DPI_POLARITY polarity, UINT32 divisor, UINT32 duty)
{
    DPI_REG_CLKCNTL ctrl;
/*
    ASSERT(divisor >= 2);
    ASSERT(duty > 0 && duty < divisor);
    
    ctrl.POLARITY = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    ctrl.DIVISOR = divisor - 1;
    ctrl.DUTY = duty;
*/
    ctrl.CLK_POL = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;

    DPI_OUTREG32_R(PDPI_REG_CLKCNTL,&DPI_REG->CLK_CNTL, &ctrl);

    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_ConfigPixelClk);

DPI_STATUS DPI_ConfigLVDS(LCM_PARAMS *lcm_params)
{
	DPI_REG_CLKCNTL ctrl = DPI_REG->CLK_CNTL;
//	DPI_REG_EMBSYNC_SETTING embsync;
//	ctrl.EMBSYNC_EN = lcm_params->dpi.embsync;
    ctrl.DUAL_EDGE_SEL = (lcm_params->dpi.i2x_en>0)?1:0;
	//MASKREG32(DISPSYS_BASE + 0x60, 0x7, (lcm_params->dpi.i2x_en << 1)|(lcm_params->dpi.i2x_edge << 2) | 1);
    DPI_OUTREG32_R(PDPI_REG_CLKCNTL,&DPI_REG->CLK_CNTL, &ctrl);

    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_ConfigLVDS);

DPI_STATUS DPI_WaitVsync()
{
	// FIXME: implement this!
	UINT32 dpi_wait_time = 0;
	MASKREG32(0x14008004, 0x2, 0x0);
	while((INREG32(0x14008004)&0x2) != 0x2)	// polling RDMA start
	{
	   	udelay(50);//sleep 50us
		dpi_wait_time++;
		if(dpi_wait_time > 40000){
			DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "Wait for RDMA0 Start IRQ timeout!!!\n");
			break;
		}
	}
	MASKREG32(0x14008004, 0x2, 0x0);
	
	return DPI_STATUS_OK;
}


DPI_STATUS DPI_ConfigDataEnable(DPI_POLARITY polarity)
{

    DPI_REG_CLKCNTL pol = DPI_REG->CLK_CNTL;
    pol.DE_POL = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    DPI_OUTREG32_R(PDPI_REG_CLKCNTL,&DPI_REG->CLK_CNTL, &pol);
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_ConfigDataEnable);

DPI_STATUS DPI_ConfigVsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
    DPI_REG_TGEN_VWIDTH_LODD vwidth_lodd  = DPI_REG->TGEN_VWIDTH_LODD;
	DPI_REG_TGEN_VPORCH_LODD vporch_lodd  = DPI_REG->TGEN_VPORCH_LODD;
    DPI_REG_CLKCNTL pol = DPI_REG->CLK_CNTL;
    
	pol.VSYNC_POL = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    vwidth_lodd.VPW_LODD = pulseWidth;
    vporch_lodd.VBP_LODD= backPorch;
    vporch_lodd.VFP_LODD= frontPorch;

    DPI_OUTREG32_R(PDPI_REG_CLKCNTL,&DPI_REG->CLK_CNTL, &pol);
    DPI_OUTREG32_R(PDPI_REG_TGEN_VWIDTH_LODD,&DPI_REG->TGEN_VWIDTH_LODD, &vwidth_lodd);
	DPI_OUTREG32_R(PDPI_REG_TGEN_VPORCH_LODD,&DPI_REG->TGEN_VPORCH_LODD, &vporch_lodd);
    
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_ConfigVsync);


DPI_STATUS DPI_ConfigHsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
    DPI_REG_TGEN_HPORCH hporch = DPI_REG->TGEN_HPORCH;
    DPI_REG_CLKCNTL pol = DPI_REG->CLK_CNTL;
    
	pol.HSYNC_POL = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    DPI_REG->TGEN_HWIDTH = pulseWidth;
    hporch.HBP = backPorch;
    hporch.HFP = frontPorch;
    
	pol.HSYNC_POL = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    //DPI_REG->TGEN_HWIDTH = pulseWidth;
    OUTREG32(&DPI_REG->TGEN_HWIDTH,pulseWidth);
    hporch.HBP = backPorch;
    hporch.HFP = frontPorch;
    DPI_OUTREG32_R(PDPI_REG_CLKCNTL,&DPI_REG->CLK_CNTL, &pol);
    DPI_OUTREG32_R(PDPI_REG_TGEN_HPORCH,&DPI_REG->TGEN_HPORCH, &hporch);

    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_ConfigHsync);

DPI_STATUS DPI_FBEnable(DPI_FB_ID id, BOOL enable)
{
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_FBEnable);

DPI_STATUS DPI_FBSyncFlipWithLCD(BOOL enable)
{
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_FBSyncFlipWithLCD);

DPI_STATUS DPI_SetDSIMode(BOOL enable)
{
    return DPI_STATUS_OK;
}


BOOL DPI_IsDSIMode(void)
{
//	return DPI_REG->CNTL.DSI_MODE ? TRUE : FALSE;
	return FALSE;
}


DPI_STATUS DPI_FBSetFormat(DPI_FB_FORMAT format)
{
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_FBSetFormat);

DPI_FB_FORMAT DPI_FBGetFormat(void)
{
    return 0;
}
//EXPORT_SYMBOL(DPI_FBGetFormat);


DPI_STATUS DPI_FBSetSize(UINT32 width, UINT32 height)
{
    DPI_REG_SIZE size;
    size.WIDTH = width;
    size.HEIGHT = height;
    
    DPI_OUTREG32_R(PDPI_REG_SIZE,&DPI_REG->SIZE, &size);

    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_FBSetSize);

DPI_STATUS DPI_FBSetAddress(DPI_FB_ID id, UINT32 address)
{
    return DPI_STATUS_OK;
}    
//EXPORT_SYMBOL(DPI_FBSetAddress);

DPI_STATUS DPI_FBSetPitch(DPI_FB_ID id, UINT32 pitchInByte)
{
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_FBSetPitch);

DPI_STATUS DPI_SetFifoThreshold(UINT32 low, UINT32 high)
{
    return DPI_STATUS_OK;
}
//EXPORT_SYMBOL(DPI_SetFifoThreshold);


DPI_STATUS DPI_DumpRegisters(void)
{
    UINT32 i;

    DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "---------- Start dump DPI registers ----------\n");
    
    for (i = 0; i < sizeof(DPI_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "DPI+%04x : 0x%08x\n", i, INREG32(DPI_BASE + i));
    }

    return DPI_STATUS_OK;
}

//EXPORT_SYMBOL(DPI_DumpRegisters);

UINT32 DPI_GetCurrentFB(void)
{
	return 0;
}
//EXPORT_SYMBOL(DPI_GetCurrentFB);

DPI_STATUS DPI_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp)
{
#if 0
    unsigned int i = 0;
    unsigned char *fbv;
    unsigned int fbsize = 0;
    unsigned int dpi_fb_bpp = 0;
    unsigned int w,h;
	BOOL dpi_needPowerOff = FALSE;
	if(!s_isDpiPowerOn){
		DPI_PowerOn();
		dpi_needPowerOff = TRUE;
		LCD_WaitForNotBusy();
	    LCD_WaitDPIIndication(FALSE);
		LCD_FBReset();
    	LCD_StartTransfer(TRUE);
		LCD_WaitDPIIndication(TRUE);
	}

    if(pvbuf == 0 || bpp == 0)
    {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "DPI_Capture_Framebuffer, ERROR, parameters wrong: pvbuf=0x%08x, bpp=%d\n", pvbuf, bpp);
        return DPI_STATUS_OK;
    }

    if(DPI_FBGetFormat() == DPI_FB_FORMAT_RGB565)
    {
        dpi_fb_bpp = 16;
    }
    else if(DPI_FBGetFormat() == DPI_FB_FORMAT_RGB888)
    {
        dpi_fb_bpp = 24;
    }
    else
    {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "DPI_Capture_Framebuffer, ERROR, dpi_fb_bpp is wrong: %d\n", dpi_fb_bpp);
        return DPI_STATUS_OK;
    }

    w = DISP_GetScreenWidth();
    h = DISP_GetScreenHeight();
    fbsize = w*h*dpi_fb_bpp/8;
	if(dpi_needPowerOff)
    	fbv = (unsigned char*)ioremap_cached((unsigned int)DPI_REG->FB[0].ADDR, fbsize);
	else
    	fbv = (unsigned char*)ioremap_cached((unsigned int)DPI_REG->FB[DPI_GetCurrentFB()].ADDR, fbsize);
 
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "current fb count is %d\n", DPI_GetCurrentFB());

    if(bpp == 32 && dpi_fb_bpp == 24)
    {			
    	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
			unsigned int pix_count = w * h - 1;
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned int*)(pvbuf+ (pix_count - i) * 4) = 0xff000000|fbv[i*3]|(fbv[i*3+1]<<8)|(fbv[i*3+2]<<16);
    		}
		}
		else{
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned int*)(pvbuf+i*4) = 0xff000000|fbv[i*3]|(fbv[i*3+1]<<8)|(fbv[i*3+2]<<16);
    		}
		}
    }
    else if(bpp == 32 && dpi_fb_bpp == 16)
    {
        unsigned int t;
		unsigned short* fbvt = (unsigned short*)fbv;
    	
		if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
			unsigned int pix_count = w * h - 1;
			
    		for(i = 0;i < w*h; i++)
    		{
				t = fbvt[i];
            	*(unsigned int*)(pvbuf+ (pix_count - i) * 4) = 0xff000000|((t&0x001F)<<3)|((t&0x07E0)<<5)|((t&0xF800)<<8);
    		}
		}
		else{
        	for(i = 0;i < w*h; i++)
    		{
	    		t = fbvt[i];
            	*(unsigned int*)(pvbuf+i*4) = 0xff000000|((t&0x001F)<<3)|((t&0x07E0)<<5)|((t&0xF800)<<8);
    		}
		}
    }
    else if(bpp == 16 && dpi_fb_bpp == 16)
    {
		if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
			unsigned int pix_count = w * h - 1;
			unsigned short* fbvt = (unsigned short*)fbv;
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned short*)(pvbuf+ (pix_count - i) * 2) = fbvt[i];
    		}
		}
		else
    		memcpy((void*)pvbuf, (void*)fbv, fbsize);
    }
    else if(bpp == 16 && dpi_fb_bpp == 24)
    {
		if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
			unsigned int pix_count = w * h - 1;
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned short*)(pvbuf+ (pix_count - i) * 2) = ((fbv[i*3+0]&0xF8)>>3)|
	            	                        				((fbv[i*3+1]&0xFC)<<3)|
														    ((fbv[i*3+2]&0xF8)<<8);
    		}
		}
		else{
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned short*)(pvbuf+i*2) = ((fbv[i*3+0]&0xF8)>>3)|
	            	                        ((fbv[i*3+1]&0xFC)<<3)|
						    				((fbv[i*3+2]&0xF8)<<8);
    		}
		}
    }
    else
    {
    	DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "DPI_Capture_Framebuffer, bpp:%d & dpi_fb_bpp:%d is not supported now\n", bpp, dpi_fb_bpp);
    }

    iounmap(fbv);
    	
	if(dpi_needPowerOff){
		DPI_PowerOff();
	}
#endif

    return DPI_STATUS_OK;    
}

DPI_STATUS DPI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
#if ENABLE_DPI_INTERRUPT
    switch(eventID)
    {
        case DISP_DPI_VSYNC_INT:
            //DPI_REG->INT_ENABLE.VSYNC = 1;
            OUTREGBIT(DPI_REG_INTERRUPT,DPI_REG->INT_ENABLE,VSYNC,1);
            break;
        default:
            return DPI_STATUS_ERROR;
    }
    
    return DPI_STATUS_OK;
#else
    ///TODO: warning log here
    return DPI_STATUS_ERROR;
#endif
}


DPI_STATUS DPI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
    dpiIntCallback = pCB;

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_FMDesense_Query(void)
{
    return DPI_STATUS_ERROR;
}

DPI_STATUS DPI_FM_Desense(unsigned long freq)
{
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Reset_CLK(void)
{
	return DPI_STATUS_OK;
}

DPI_STATUS DPI_Get_Default_CLK(unsigned int *clk)
{
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Get_Current_CLK(unsigned int *clk)
{
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Change_CLK(unsigned int clk)
{
    return DPI_STATUS_OK;
}
