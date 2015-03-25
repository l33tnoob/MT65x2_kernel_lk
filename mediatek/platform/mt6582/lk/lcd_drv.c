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
#include <platform/disp_drv_platform.h>
#include <platform/ddp_ovl.h>
#include <platform/ddp_path.h>
#include <platform/sync_write.h>
#ifdef OUTREG32
  #undef OUTREG32
  #define OUTREG32(x, y) mt65xx_reg_sync_writel(y, x)
#endif
#ifdef OUTREG16
  #undef OUTREG16
  #define OUTREG16(x, y) mt65xx_reg_sync_writew(y, x)
#endif
#ifdef OUTREG8
  #undef OUTREG8
  #define OUTREG8(x, y) mt65xx_reg_sync_writeb(y, x)
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
#define LCD_OUTREG32_R(type, addr2, addr1) DISP_OUTREG32_R(type, addr2, addr1)
#define LCD_OUTREG32_V(type, addr2, var)   DISP_OUTREG32_V(type, addr2, var)
#define LCD_OUTREG16_R(type, addr2, addr1) DISP_OUTREG16_R(type, addr2, addr1)
#define LCD_MASKREG32_T(type, addr, mask, data)	 DISP_MASKREG32_T(type, addr, mask, data)
#define LCD_INREG32(type, addr)                  DISP_INREG32(type,addr)


#define LCD_OUTREG32(addr, data)	\
		{\
		OUTREG32(addr, data);}

#define LCD_OUTREG8(addr, data)	\
		{\
		OUTREG8(addr, data);}


#define LCD_OUTREG16(addr, data)	\
		{\
		OUTREG16(addr, data);}

#define LCD_MASKREG32(addr, mask, data)	\
		{\
		MASKREG32(addr, mask, data);}


static size_t dbi_log_on = FALSE;
#define DBI_LOG(fmt, arg...) \
    do { \
        if (dbi_log_on) DISP_LOG_PRINT(ANDROID_LOG_WARN, "LCD", fmt, ##arg);    \
    }while (0)

#define DBI_FUNC()	\
	do { \
		if(dbi_log_on) DISP_LOG_PRINT(ANDROID_LOG_INFO, "LCD", "[Func]%s\n", __func__);  \
	}while (0)

void dbi_log_enable(int enable)
{
    dbi_log_on = enable;
	DBI_LOG("lcd log %s\n", enable?"enabled":"disabled");
}

#include <platform/mt_gpt.h>
#include <string.h>

static PLCD_REGS const LCD_REG = (PLCD_REGS)(LCD_BASE);
static const UINT32 TO_BPP[LCD_FB_FORMAT_NUM] = {2, 3, 4};
unsigned int wait_time = 0;
typedef struct
{
    LCD_FB_FORMAT fbFormat;
    UINT32 fbPitchInBytes;
    LCD_REG_SIZE roiWndSize;
    LCD_OUTPUT_MODE outputMode;
    LCD_REGS regBackup;
    void (*pIntCallback)(DISP_INTERRUPT_EVENTS);
} LCD_CONTEXT;

static LCD_CONTEXT _lcdContext;/* _lcdContext memset set in LCD_init */
static int wst_step_LCD = -1;//for LCD&FM de-sense
static BOOL is_get_default_write_cycle = FALSE;
static unsigned int default_write_cycle = 0;
static UINT32 default_wst = 0;
static BOOL limit_w2m_speed = FALSE;
static BOOL limit_w2tvr_speed = FALSE;
static OVL_CONFIG_STRUCT layer_config[4] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {1,0,0,0,0,0,0,0,0,0,0,0,0,0},
											{2,0,0,0,0,0,0,0,0,0,0,0,0,0},
											{3,0,0,0,0,0,0,0,0,0,0,0,0,0}};
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
LCD_STATUS LCD_Init_IO_pad(LCM_PARAMS *lcm_params)
{
//    #warning "LCD_Init_IO_pad not implement for 6589 yes"
	
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_Set_DrivingCurrent(LCM_PARAMS *lcm_params)
{
//    #warning "LCD_Set_DrivingCurrent not implement for 6589 yes"
	return LCD_STATUS_OK;
}


#if 1
static BOOL _IsEngineBusy(void)
{
    LCD_REG_STATUS status;

    status = LCD_REG->STATUS;
    if (status.BUSY) 
        return TRUE;

    return FALSE;
}
#else
static BOOL _IsEngineBusy(void)
{
    LCD_REG_INTERRUPT status;

    status = LCD_REG->INT_STATUS;
    if (status.COMPLETED){
		MASKREG32(&LCD_REG->INT_STATUS, 0x1, 0x1);
		return TRUE;
    }

    return FALSE;
}

#endif


BOOL LCD_IsBusy(void)
{
	return _IsEngineBusy();
}


static void _WaitForEngineNotBusy(void)
{
	while(_IsEngineBusy()) {
		msleep(1);//sleep 1ms
		wait_time++;
		if(wait_time>4000){ //timeout
			printk("[WARNING] Wait for LCD engine not busy timeout!!!\n");
			LCD_DumpRegisters();

			if(LCD_REG->STATUS.WAIT_SYNC){
				printk("reason is LCD can't wait TE signal!!!\n");
				LCD_TE_Enable(FALSE);
			}
			LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0);
			LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0x1);
			wait_time = 0;
		}
    }
    wait_time = 0;

}

unsigned int vsync_wait_time = 0;
void LCD_WaitTE(void)
{
#ifndef BUILD_LK	
	wait_lcd_vsync = TRUE;
	hrtimer_start(&hrtimer_vsync, ktime_set(0, VSYNC_US_TO_NS(vsync_timer)), HRTIMER_MODE_REL);
	wait_event_interruptible(_vsync_wait_queue, lcd_vsync);
	lcd_vsync = FALSE;
	wait_lcd_vsync = FALSE;
#endif
}

#ifndef BUILD_LK
void LCD_GetVsyncCnt()
{
}
enum hrtimer_restart lcd_te_hrtimer_func(struct hrtimer *timer)
{
//	long long ret;
	if(wait_lcd_vsync)
	{
		lcd_vsync = TRUE;
		wake_up_interruptible(&_vsync_wait_queue);
//		printk("hrtimer Vsync, and wake up\n");
	}
//	ret = hrtimer_forward_now(timer, ktime_set(0, VSYNC_US_TO_NS(vsync_timer)));
//	printk("hrtimer callback\n");
    return HRTIMER_NORESTART;
}
#endif
void LCD_InitVSYNC(unsigned int vsync_interval)
{
#ifndef BUILD_LK
    ktime_t ktime;
	vsync_timer = vsync_interval;
	ktime = ktime_set(0, VSYNC_US_TO_NS(vsync_timer));
	hrtimer_init(&hrtimer_vsync, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_vsync.function = lcd_te_hrtimer_func;
//	hrtimer_start(&hrtimer_vsync, ktime, HRTIMER_MODE_REL);
#endif
}
void LCD_PauseVSYNC(BOOL enable)
{
}

static void _BackupLCDRegisters(void)
{
//    memcpy((void*)&(_lcdContext.regBackup), (void*)LCD_REG, sizeof(LCD_REGS));
    LCD_REGS *regs = &(_lcdContext.regBackup);
    UINT32 i;
    LCD_OUTREG32_R(PLCD_REG_INTERRUPT,&regs->INT_ENABLE, &LCD_REG->INT_ENABLE);
    LCD_OUTREG32_R(PLCD_REG_SCNF,&regs->SERIAL_CFG, &LCD_REG->SERIAL_CFG);
	for(i = 0; i < ARY_SIZE(LCD_REG->SIF_TIMING); ++i)
    {
        LCD_OUTREG32_R(PLCD_REG_SIF_TIMING, &regs->SIF_TIMING[i], &LCD_REG->SIF_TIMING[i]);
    }

    for(i = 0; i < ARY_SIZE(LCD_REG->PARALLEL_CFG); ++i)
    {
        LCD_OUTREG32_R(PLCD_REG_PCNF, &regs->PARALLEL_CFG[i], &LCD_REG->PARALLEL_CFG[i]);
    }

    LCD_OUTREG32_R(PLCD_REG_TECON,&regs->TEARING_CFG, &LCD_REG->TEARING_CFG);
    LCD_OUTREG32_R(PLCD_REG_PCNFDW,&regs->PARALLEL_DW, &LCD_REG->PARALLEL_DW);
	LCD_OUTREG32_R(PLCD_REG_CALC_HTT,&regs->CALC_HTT, &LCD_REG->CALC_HTT);
	LCD_OUTREG32_R(PLCD_REG_SYNC_LCM_SIZE,&regs->SYNC_LCM_SIZE, &LCD_REG->SYNC_LCM_SIZE);
	LCD_OUTREG32_R(PLCD_REG_SYNC_CNT,&regs->SYNC_CNT, &LCD_REG->SYNC_CNT);
	LCD_OUTREG32_R(PLCD_REG_SMICON,&regs->SMI_CON, &LCD_REG->SMI_CON);

    LCD_OUTREG32_R(PLCD_REG_WROI_CON,&regs->WROI_CONTROL, &LCD_REG->WROI_CONTROL);
    LCD_OUTREG32_R(PLCD_REG_CMD_ADDR,&regs->WROI_CMD_ADDR, &LCD_REG->WROI_CMD_ADDR);
    LCD_OUTREG32_R(PLCD_REG_DAT_ADDR,&regs->WROI_DATA_ADDR, &LCD_REG->WROI_DATA_ADDR);
    LCD_OUTREG32_R(PLCD_REG_SIZE,&regs->WROI_SIZE, &LCD_REG->WROI_SIZE);

//	LCD_OUTREG32(&regs->DITHER_CON, AS_UINT32(&LCD_REG->DITHER_CON));
	LCD_OUTREG32_R(PLCD_SRC_CON,&regs->SRC_CON, &LCD_REG->SRC_CON);

	LCD_OUTREG32(&regs->SRC_ADD, AS_UINT32(&LCD_REG->SRC_ADD));
	LCD_OUTREG32(&regs->SRC_PITCH, AS_UINT32(&LCD_REG->SRC_PITCH));
	
	LCD_OUTREG32_R(PLCD_REG_ULTRA_CON,&regs->ULTRA_CON, &LCD_REG->ULTRA_CON);
	LCD_OUTREG32_R(PLCD_REG_DBI_ULTRA_TH,&regs->DBI_ULTRA_TH, &LCD_REG->DBI_ULTRA_TH);

	LCD_OUTREG32_R(PLCD_REG_GMC_ULTRA_TH,&regs->GMC_ULTRA_TH, &LCD_REG->GMC_ULTRA_TH);
}


static void _RestoreLCDRegisters(void)
{
    LCD_REGS *regs = &(_lcdContext.regBackup);
    UINT32 i;
    LCD_OUTREG32_R(PLCD_REG_INTERRUPT,&LCD_REG->INT_ENABLE, &regs->INT_ENABLE);
    LCD_OUTREG32_R(PLCD_REG_SCNF,&LCD_REG->SERIAL_CFG, &regs->SERIAL_CFG);
	for(i = 0; i < ARY_SIZE(LCD_REG->SIF_TIMING); ++i)
    {
        LCD_OUTREG32_R(PLCD_REG_SIF_TIMING, &LCD_REG->SIF_TIMING[i], &regs->SIF_TIMING[i]);
    }

    for(i = 0; i < ARY_SIZE(LCD_REG->PARALLEL_CFG); ++i)
    {
        LCD_OUTREG32_R(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[i], &regs->PARALLEL_CFG[i]);
    }

    LCD_OUTREG32_R(PLCD_REG_TECON,&LCD_REG->TEARING_CFG, &regs->TEARING_CFG);
    LCD_OUTREG32_R(PLCD_REG_PCNFDW,&LCD_REG->PARALLEL_DW, &regs->PARALLEL_DW);
	LCD_OUTREG32_R(PLCD_REG_CALC_HTT,&LCD_REG->CALC_HTT, &regs->CALC_HTT);
	LCD_OUTREG32_R(PLCD_REG_SYNC_LCM_SIZE,&LCD_REG->SYNC_LCM_SIZE, &regs->SYNC_LCM_SIZE);
	LCD_OUTREG32_R(PLCD_REG_SYNC_CNT,&LCD_REG->SYNC_CNT, &regs->SYNC_CNT);
	LCD_OUTREG32_R(PLCD_REG_SMICON,&LCD_REG->SMI_CON, &regs->SMI_CON);

    LCD_OUTREG32_R(PLCD_REG_WROI_CON,&LCD_REG->WROI_CONTROL, &regs->WROI_CONTROL);
    LCD_OUTREG32_R(PLCD_REG_CMD_ADDR,&LCD_REG->WROI_CMD_ADDR, &regs->WROI_CMD_ADDR);
    LCD_OUTREG32_R(PLCD_REG_DAT_ADDR,&LCD_REG->WROI_DATA_ADDR, &regs->WROI_DATA_ADDR);
    LCD_OUTREG32_R(PLCD_REG_SIZE,&LCD_REG->WROI_SIZE, &regs->WROI_SIZE);

//	LCD_OUTREG32(&LCD_REG->DITHER_CON, AS_UINT32(&regs->DITHER_CON));

	LCD_OUTREG32_R(PLCD_SRC_CON,&LCD_REG->SRC_CON, &regs->SRC_CON);
	LCD_OUTREG32(&LCD_REG->SRC_ADD, AS_UINT32(&regs->SRC_ADD));
	LCD_OUTREG32(&LCD_REG->SRC_PITCH, AS_UINT32(&regs->SRC_PITCH));

	LCD_OUTREG32_R(PLCD_REG_ULTRA_CON,&LCD_REG->ULTRA_CON, &regs->ULTRA_CON);
	LCD_OUTREG32_R(PLCD_REG_DBI_ULTRA_TH,&LCD_REG->DBI_ULTRA_TH, &regs->DBI_ULTRA_TH);

	LCD_OUTREG32_R(PLCD_REG_GMC_ULTRA_TH,&LCD_REG->GMC_ULTRA_TH, &regs->GMC_ULTRA_TH);
}


static void _ResetBackupedLCDRegisterValues(void)
{
    LCD_REGS *regs = &_lcdContext.regBackup;
    memset((void*)regs, 0, sizeof(LCD_REGS));

    LCD_OUTREG32_V(PLCD_REG_SCNF,&regs->SERIAL_CFG, 0x00003000);
    LCD_OUTREG32_V(PLCD_REG_PCNF,&regs->PARALLEL_CFG[0], 0x00FC0000);
    LCD_OUTREG32_V(PLCD_REG_PCNF, &regs->PARALLEL_CFG[1], 0x00300000);
    LCD_OUTREG32_V(PLCD_REG_PCNF, &regs->PARALLEL_CFG[2], 0x00300000);
}


// ---------------------------------------------------------------------------
//  LCD Controller API Implementations
// ---------------------------------------------------------------------------

LCD_STATUS LCD_Init(void)
{
    LCD_STATUS ret = LCD_STATUS_OK;

    memset(&_lcdContext, 0, sizeof(_lcdContext));

    // LCD controller would NOT reset register as default values
    // Do it by SW here
    //
    _ResetBackupedLCDRegisterValues();

    ret = LCD_PowerOn();

	LCD_OUTREG32_V(PLCD_REG_SYNC_LCM_SIZE,&LCD_REG->SYNC_LCM_SIZE, 0x00010001);
	LCD_OUTREG32_V(PLCD_REG_SYNC_CNT,&LCD_REG->SYNC_CNT, 0x1);

    ASSERT(ret == LCD_STATUS_OK);
	//MASKREG32(DISPSYS_BASE + 0x60, 0x1, 0x0);
    
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_Deinit(void)
{
    LCD_STATUS ret = LCD_PowerOff();

    ASSERT(ret == LCD_STATUS_OK);

    return LCD_STATUS_OK;
}

static BOOL s_isLcdPowerOn = FALSE;

#ifdef BUILD_LK
LCD_STATUS LCD_PowerOn(void)
{
    if (!s_isLcdPowerOn)
    {
		LCD_MASKREG32(0x14000110, 0x307, 0x0);
		printf("0x%8x\n", INREG32(0x14000110));
        _RestoreLCDRegisters();
        s_isLcdPowerOn = TRUE;
    }

    return LCD_STATUS_OK;
}


LCD_STATUS LCD_PowerOff(void)
{
    if (s_isLcdPowerOn)
    {
        _WaitForEngineNotBusy();
        _BackupLCDRegisters();
#if 1   // FIXME
		LCD_MASKREG32(0x14000110, 0x307, 0x307);
		printf("0x%8x\n", INREG32(0x14000110));
#endif        
        s_isLcdPowerOn = FALSE;
    }

    return LCD_STATUS_OK;
}

#else
LCD_STATUS LCD_PowerOn(void)
{
    DBI_FUNC();
#ifndef CONFIG_MT6589_FPGA
#if 1
    if (!s_isLcdPowerOn)
    {
		int ret = 0;
#if 0   // FIXME
	DBI_LOG("lcd will be power on\n");
      	ret = enable_clock(MT65XX_PDN_MM_SMI_LARB1_EMI, "LCD");
		ret += enable_clock(MT65XX_PDN_MM_SMI_LARB1, "LCD");
		ret += enable_clock(MT65XX_PDN_MM_LCD_EMI, "LCD");
		ret += enable_clock(MT65XX_PDN_MM_LCD, "LCD");
		if(ret > 0)
		{
			DBI_LOG("[LCD]power manager API return FALSE\n");
		}
#endif        
        _RestoreLCDRegisters();
        s_isLcdPowerOn = TRUE;
    }

#endif
#endif
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_PowerOff(void)
{
    DBI_FUNC();
#ifndef CONFIG_MT6589_FPGA
    if (s_isLcdPowerOn)
    {
        int ret = 1;
        _WaitForEngineNotBusy();
        _BackupLCDRegisters();
	DBI_LOG("lcd will be power off\n");
#if 0   // FIXME
        ret = disable_clock(MT65XX_PDN_MM_LCD, "LCD");
		ret += disable_clock(MT65XX_PDN_MM_LCD_EMI, "LCD");
		ret += disable_clock(MT65XX_PDN_MM_SMI_LARB1, "LCD");
		ret += disable_clock(MT65XX_PDN_MM_SMI_LARB1_EMI, "LCD");
		if(ret > 0)
		{
			DBI_LOG("[LCD]power manager API return FALSE\n");
		}
#endif        
        s_isLcdPowerOn = FALSE;
    }

#endif
    return LCD_STATUS_OK;
}
#endif

LCD_STATUS LCD_WaitForNotBusy(void)
{
    _WaitForEngineNotBusy();
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
    _lcdContext.pIntCallback = pCB;

    return LCD_STATUS_OK;
}


// -------------------- LCD Controller Interface --------------------

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
								UINT32 chw)
{
    ASSERT(id <= LCD_IF_PARALLEL_2);

    ASSERT(writeSetup <= 16U);
    ASSERT(writeHold <= 16U);
    ASSERT(writeWait <= 64U);
    ASSERT(readSetup <= 16U);
	ASSERT(readHold <= 16U);
    ASSERT(readLatency <= 64U);
    ASSERT(chw <= 16U);

    if (0 == writeHold)   writeHold = 1;
    if (0 == writeWait)   writeWait = 1;
    if (0 == readLatency) readLatency = 1;

    _WaitForEngineNotBusy();

    // (1) Config Data Width
    {
        LCD_REG_PCNFDW pcnfdw = LCD_REG->PARALLEL_DW;

        switch(id)
        {
        case LCD_IF_PARALLEL_0: pcnfdw.PCNF0_DW = (UINT32)ifDataWidth; pcnfdw.PCNF0_CHW = chw;break;
        case LCD_IF_PARALLEL_1: pcnfdw.PCNF1_DW = (UINT32)ifDataWidth; pcnfdw.PCNF1_CHW = chw;break;
        case LCD_IF_PARALLEL_2: pcnfdw.PCNF2_DW = (UINT32)ifDataWidth; pcnfdw.PCNF2_CHW = chw;break;
        default : ASSERT(0);
        };

        LCD_OUTREG32_R(PLCD_REG_PCNFDW,&LCD_REG->PARALLEL_DW, &pcnfdw);
    }

    // (2) Config Timing
    {
        UINT32 i;
        LCD_REG_PCNF config;
        
        i = (UINT32)id - LCD_IF_PARALLEL_0;
        config = LCD_REG->PARALLEL_CFG[i];

        config.C2WS = writeSetup;
        config.C2WH = writeHold - 1;
        config.WST  = writeWait - 1;
        config.C2RS = readSetup;
        config.C2RH = readHold;
        config.RLT  = readLatency - 1;

        LCD_OUTREG32_R(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[i], &config);

    }

    return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigIfFormat(LCD_IF_FMT_COLOR_ORDER order,
                              LCD_IF_FMT_TRANS_SEQ transSeq,
                              LCD_IF_FMT_PADDING padding,
                              LCD_IF_FORMAT format,
                              LCD_IF_WIDTH busWidth)
{
    LCD_REG_WROI_CON ctrl = LCD_REG->WROI_CONTROL;


    ctrl.RGB_ORDER  = order;
    ctrl.BYTE_ORDER = transSeq;
    ctrl.PADDING    = padding;
    ctrl.DATA_FMT   = (UINT32)format;
    ctrl.IF_FMT   = (UINT32)busWidth;
    ctrl.IF_24 = 0;
    if(busWidth == LCD_IF_WIDTH_24_BITS)
	{
	    ctrl.IF_24 = 1;
	}

    LCD_OUTREG32_R(PLCD_REG_WROI_CON,&LCD_REG->WROI_CONTROL, &ctrl);


    return LCD_STATUS_OK;
}

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
                              UINT32 wr_2nd)
{
    LCD_REG_SCNF config;
	LCD_REG_SIF_TIMING sif_timing;
	unsigned int offset = 0;
	unsigned int sif_id = 0;
    
	ASSERT(id >= LCD_IF_SERIAL_0 && id <= LCD_IF_SERIAL_1);

    _WaitForEngineNotBusy();

    memset(&config, 0, sizeof(config));
	
	if(id == LCD_IF_SERIAL_1){
		offset = 8;
		sif_id = 1;
	}
	
	LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x07 << offset, bits << offset);
	LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x08 << offset, three_wire << (offset + 3));
	LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x10 << offset, sdi << (offset + 4));
	LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x20 << offset, first_pol << (offset + 5));
	LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x40 << offset, sck_def << (offset + 6));
	LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x80 << offset, div2 << (offset + 7));

	config.HW_CS = hw_cs;
//	config.SIZE_0 = bits;
    LCD_OUTREG32_R(PLCD_REG_SCNF,&LCD_REG->SERIAL_CFG, &config);


	sif_timing.WR_2ND = wr_2nd;
	sif_timing.WR_1ST = wr_1st;
	sif_timing.RD_2ND = rd_2nd;
	sif_timing.RD_1ST = rd_1st;
	sif_timing.CSH = csh;
	sif_timing.CSS = css;
	
	LCD_OUTREG32_R(PLCD_REG_SIF_TIMING,&LCD_REG->SIF_TIMING[sif_id], &sif_timing);

    return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetResetSignal(BOOL high)
{
	UINT32 reset = high ? 1 : 0;
    //LCD_REG->RESET = high ? 1 : 0;
    OUTREG32(&LCD_REG->RESET,reset);

    return LCD_STATUS_OK;
}


LCD_STATUS LCD_ConfigDSIIfFormat(LCD_DSI_IF_FMT_COLOR_ORDER order,
                              LCD_DSI_IF_FMT_TRANS_SEQ transSeq,
                              LCD_DSI_IF_FMT_PADDING padding,
                              LCD_DSI_IF_FORMAT format,
                              UINT32 packet_size,
                              BOOL DC_DSI)
{
	//MT6583 not support
	return LCD_STATUS_OK;
}


// -------------------- Command Queue --------------------

LCD_STATUS LCD_CmdQueueEnable(BOOL enabled)
{
    LCD_REG_WROI_CON ctrl;

//    _WaitForEngineNotBusy();

    ctrl = LCD_REG->WROI_CONTROL;
    ctrl.ENC = enabled ? 1 : 0;
    LCD_OUTREG32_R(PLCD_REG_WROI_CON,&LCD_REG->WROI_CONTROL, &ctrl);

    return LCD_STATUS_OK;
}

LCD_STATUS LCD_CmdQueueWrite(UINT32 *cmds, UINT32 cmdCount)
{
    LCD_REG_WROI_CON ctrl;
    UINT32 i;

    ASSERT(cmdCount < ARY_SIZE(LCD_REG->CMDQ));

//    _WaitForEngineNotBusy();
    ctrl = LCD_REG->WROI_CONTROL;
    ctrl.COMMAND = cmdCount - 1;

    LCD_OUTREG32_R(PLCD_REG_WROI_CON,&LCD_REG->WROI_CONTROL, &ctrl);

    for (i = 0; i < cmdCount; ++ i)
    {
        //LCD_REG->CMDQ[i] = cmds[i];
        OUTREG32(&LCD_REG->CMDQ[i],cmds[i]);
    }

    return LCD_STATUS_OK;
}


// -------------------- Layer Configurations --------------------
LCD_STATUS LCD_LayerEnable(LCD_LAYER_ID id, BOOL enable)
{
	//MT6583 not support, if be called, it should call OVL function
	if(LCD_LAYER_ALL == id)return LCD_STATUS_OK;
	layer_config[id].layer= id;
	layer_config[id].layer_en= enable;
//	disp_path_config_layer(&layer_config[id]);
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigDither(int lrs, int lgs, int lbs, int dbr, int dbg, int dbb)
{
/*
	LCD_REG_DITHER_CON ctrl;

//	_WaitForEngineNotBusy();

	ctrl = LCD_REG->DITHER_CON;

	ctrl.LFSR_R_SEED = lrs;
	ctrl.LFSR_G_SEED = lgs;
	ctrl.LFSR_B_SEED = lbs;
	ctrl.DB_R = dbr;
	ctrl.DB_G = dbg;
	ctrl.DB_B = dbb;

	OUTREG32(&LCD_REG->DITHER_CON, AS_UINT32(&ctrl));
*/
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerEnableDither(LCD_LAYER_ID id, UINT32 enable)
{
    ASSERT(id < LCD_LAYER_NUM || LCD_LAYER_ALL == id);

//    _WaitForEngineNotBusy();
#if 0
    if (LCD_LAYER_ALL == id)
    {
        return LCD_STATUS_OK;
    }
    else if (id < LCD_LAYER_NUM)
    {
		LCD_REG_LAYER_CON ctrl = LCD_REG->LAYER[id].CONTROL;
        ctrl.DITHER_EN = enable;
        OUTREG32(&LCD_REG->LAYER[id].CONTROL, AS_UINT32(&ctrl));
    }
#endif    
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerSetAddress(LCD_LAYER_ID id, UINT32 address)
{
	layer_config[id].addr = address;
//	if(FB_LAYER == id || (FB_LAYER + 1) == id)
//		disp_path_config_layer_addr((unsigned int)id,address);
    return LCD_STATUS_OK;

}


UINT32 LCD_LayerGetAddress(LCD_LAYER_ID id)
{
    return layer_config[id].addr;
}


LCD_STATUS LCD_LayerSetSize(LCD_LAYER_ID id, UINT32 width, UINT32 height)
{   
	layer_config[id].w = width;
	layer_config[id].h = height;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetPitch(LCD_LAYER_ID id, UINT32 pitch)
{
	layer_config[id].pitch = pitch;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetOffset(LCD_LAYER_ID id, UINT32 x, UINT32 y)
{
	layer_config[id].x = x;
	layer_config[id].y = y;
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerSetWindowOffset(LCD_LAYER_ID id, UINT32 x, UINT32 y)
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerSetFormat(LCD_LAYER_ID id, LCD_LAYER_FORMAT format)
{
	layer_config[id].fmt = format;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerEnableByteSwap(LCD_LAYER_ID id, BOOL enable)
{
	//mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetRotation(LCD_LAYER_ID id, LCD_LAYER_ROTATION rotation)
{
	//mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetAlphaBlending(LCD_LAYER_ID id, BOOL enable, UINT8 alpha)
{
	layer_config[id].alpha = (unsigned char)alpha;
	layer_config[id].aen = enable;
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetSourceColorKey(LCD_LAYER_ID id, BOOL enable, UINT32 colorKey)
{
	layer_config[id].key = colorKey;
	layer_config[id].keyEn = enable;
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_LayerSetDestColorKey(LCD_LAYER_ID id, BOOL enable, UINT32 colorKey)
{
	//mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_Layer3D_Enable(LCD_LAYER_ID id, BOOL enable)
{
	//mt6588 OVL driver not yet export this function
	return LCD_STATUS_OK;

}

LCD_STATUS LCD_Layer3D_R1st(LCD_LAYER_ID id, BOOL r_first)
{
    //mt6588 OVL driver not yet export this function
	return LCD_STATUS_OK;

}

LCD_STATUS LCD_Layer3D_landscape(LCD_LAYER_ID id, BOOL landscape)
{
	//mt6588 OVL driver not yet export this function
	return LCD_STATUS_OK;

}


LCD_STATUS LCD_LayerSet3D(LCD_LAYER_ID id, BOOL enable, BOOL r_first, BOOL landscape)
{
	//mt6588 OVL driver not yet export this function
    return LCD_STATUS_OK;
}


BOOL LCD_Is3DEnabled(void)
{
	//mt6588 OVL driver not yet export this function
	return FALSE;
}


BOOL LCD_Is3DLandscapeMode(void)
{
	//mt6588 OVL driver not yet export this function
	return FALSE;
}


// -------------------- HW Trigger Configurations --------------------

LCD_STATUS LCD_LayerSetTriggerMode(LCD_LAYER_ID id, LCD_LAYER_TRIGGER_MODE mode)
{
	//mt6583 not support
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_EnableHwTrigger(BOOL enable)
{
	//mt6583 not support
    return LCD_STATUS_OK;
}


// -------------------- ROI Window Configurations --------------------

LCD_STATUS LCD_SetBackgroundColor(UINT32 bgColor)
{
	//mt6583 not support
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetRoiWindow(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
    LCD_REG_SIZE size;
    
    size.WIDTH = (UINT16)width;
    size.HEIGHT = (UINT16)height;

//    _WaitForEngineNotBusy();
    LCD_OUTREG32_R(PLCD_REG_SIZE,&LCD_REG->WROI_SIZE, &size);

    _lcdContext.roiWndSize = size;
        
    return LCD_STATUS_OK;
}

// DSI Related Configurations
LCD_STATUS LCD_EnableDCtoDsi(BOOL enable)
{
	//mt6583 not support
    return LCD_STATUS_OK;
}

// -------------------- Output to Memory Configurations --------------------

LCD_STATUS LCD_SetOutputMode(LCD_OUTPUT_MODE mode)
{
	//mt6583 not support, call OVL
    return LCD_STATUS_OK;    
}

LCD_STATUS LCD_SetOutputAlpha(unsigned int alpha)
{
	//mt6583 not support
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_WaitDPIIndication(BOOL enable)
{
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_FBSetFormat(LCD_FB_FORMAT format)
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_FBSetPitch(UINT32 pitchInByte)
{
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_FBEnable(LCD_FB_ID id, BOOL enable)
{
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_FBReset(void)
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_FBSetAddress(LCD_FB_ID id, UINT32 address)
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_FBSetStartCoord(UINT32 x, UINT32 y)
{
    return LCD_STATUS_OK;
}

// -------------------- Color Matrix --------------------

LCD_STATUS LCD_EnableColorMatrix(LCD_IF_ID id, BOOL enable)
{
    return LCD_STATUS_OK;
}

/** Input: const S2_8 mat[9], fixed ponit signed 2.8 format
           |                      |
           | mat[0] mat[1] mat[2] |
           | mat[3] mat[4] mat[5] |
           | mat[6] mat[7] mat[8] |
           |                      |
*/
LCD_STATUS LCD_SetColorMatrix(const S2_8 mat[9])
{
    return LCD_STATUS_OK;
}

// -------------------- Tearing Control --------------------

LCD_STATUS LCD_TE_Enable(BOOL enable)
{
    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
    tecon.ENABLE = enable ? 1 : 0;

    LCD_OUTREG32_R(PLCD_REG_TECON,&LCD_REG->TEARING_CFG, &tecon);

    return LCD_STATUS_OK;
}


LCD_STATUS LCD_TE_SetMode(LCD_TE_MODE mode)
{
    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
    tecon.MODE = (LCD_TE_MODE_VSYNC_OR_HSYNC == mode) ? 1 : 0;
    LCD_OUTREG32_R(PLCD_REG_TECON,&LCD_REG->TEARING_CFG, &tecon);


    return LCD_STATUS_OK;
}


LCD_STATUS LCD_TE_SetEdgePolarity(BOOL polarity)
{
    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
    tecon.EDGE_SEL = (polarity ? 1 : 0);
    LCD_OUTREG32_R(PLCD_REG_TECON,&LCD_REG->TEARING_CFG, &tecon);

    return LCD_STATUS_OK;
}


LCD_STATUS LCD_TE_ConfigVHSyncMode(UINT32 hsDelayCnt,
                                   UINT32 vsWidthCnt,
                                   LCD_TE_VS_WIDTH_CNT_DIV vsWidthCntDiv)
{
/*    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
    tecon.HS_MCH_CNT = (hsDelayCnt ? hsDelayCnt - 1 : 0);
    tecon.VS_WLMT = (vsWidthCnt ? vsWidthCnt - 1 : 0);
    tecon.VS_CNT_DIV = vsWidthCntDiv;
    LCD_OUTREG32(&LCD_REG->TEARING_CFG, AS_UINT32(&tecon));
*/
    return LCD_STATUS_OK;
}

// -------------------- Operations --------------------

LCD_STATUS LCD_SelectWriteIF(LCD_IF_ID id)
{
    LCD_REG_CMD_ADDR cmd_addr;
	LCD_REG_DAT_ADDR dat_addr;

    switch(id)
    {
    case LCD_IF_PARALLEL_0 : cmd_addr.addr = 0; break;
    case LCD_IF_PARALLEL_1 : cmd_addr.addr = 2; break;
    case LCD_IF_PARALLEL_2 : cmd_addr.addr = 4; break;
    case LCD_IF_SERIAL_0   : cmd_addr.addr = 8; break;
    case LCD_IF_SERIAL_1   : cmd_addr.addr = 0xA; break;
    default:
        ASSERT(0);
    }
	dat_addr.addr = cmd_addr.addr + 1;
    LCD_OUTREG16_R(PLCD_REG_CMD_ADDR,&LCD_REG->WROI_CMD_ADDR, &cmd_addr);
    LCD_OUTREG16_R(PLCD_REG_DAT_ADDR,&LCD_REG->WROI_DATA_ADDR, &dat_addr);
    return LCD_STATUS_OK;
}


__inline static void _LCD_WriteIF(DWORD baseAddr, UINT32 value, LCD_IF_MCU_WRITE_BITS bits)
{
    switch(bits)
    {
    case LCD_IF_MCU_WRITE_8BIT :
        LCD_OUTREG8(baseAddr, value);
        break;
        
    case LCD_IF_MCU_WRITE_16BIT :
        LCD_OUTREG16(baseAddr, value);
        break;
        
    case LCD_IF_MCU_WRITE_32BIT :
        LCD_OUTREG32(baseAddr, value);
        break;

    default:
        ASSERT(0);
    }
}


LCD_STATUS LCD_WriteIF(LCD_IF_ID id, LCD_IF_A0_MODE a0,
                       UINT32 value, LCD_IF_MCU_WRITE_BITS bits)
{
    DWORD baseAddr = 0;

    switch(id)
    {
    case LCD_IF_PARALLEL_0 : baseAddr = (DWORD)&LCD_REG->PCMD0; break;
    case LCD_IF_PARALLEL_1 : baseAddr = (DWORD)&LCD_REG->PCMD1; break;
    case LCD_IF_PARALLEL_2 : baseAddr = (DWORD)&LCD_REG->PCMD2; break;
    case LCD_IF_SERIAL_0   : baseAddr = (DWORD)&LCD_REG->SCMD0; break;
    case LCD_IF_SERIAL_1   : baseAddr = (DWORD)&LCD_REG->SCMD1; break;
    default:
        ASSERT(0);
    }

    if (LCD_IF_A0_HIGH == a0)
    {
        baseAddr += LCD_A0_HIGH_OFFSET;
    }

    _LCD_WriteIF(baseAddr, value, bits);

    return LCD_STATUS_OK;
}


__inline static UINT32 _LCD_ReadIF(DWORD baseAddr, LCD_IF_MCU_WRITE_BITS bits)
{
    switch(bits)
    {
    case LCD_IF_MCU_WRITE_8BIT :
        return (UINT32)INREG8(baseAddr);
        
    case LCD_IF_MCU_WRITE_16BIT :
        return (UINT32)INREG16(baseAddr);
        
    case LCD_IF_MCU_WRITE_32BIT :
        return (UINT32)INREG32(baseAddr);

    default:
        ASSERT(0);
    }
}


LCD_STATUS LCD_ReadIF(LCD_IF_ID id, LCD_IF_A0_MODE a0,
                      UINT32 *value, LCD_IF_MCU_WRITE_BITS bits)
{
    DWORD baseAddr = 0;

    if (NULL == value) return LCD_STATUS_ERROR;

    switch(id)
    {
    case LCD_IF_PARALLEL_0 : baseAddr = (DWORD)&LCD_REG->PCMD0; break;
    case LCD_IF_PARALLEL_1 : baseAddr = (DWORD)&LCD_REG->PCMD1; break;
    case LCD_IF_PARALLEL_2 : baseAddr = (DWORD)&LCD_REG->PCMD2; break;
    case LCD_IF_SERIAL_0   : baseAddr = (DWORD)&LCD_REG->SCMD0; break;
    case LCD_IF_SERIAL_1   : baseAddr = (DWORD)&LCD_REG->SCMD1; break;
    default:
        ASSERT(0);
    }

    if (LCD_IF_A0_HIGH == a0)
    {
        baseAddr += LCD_A0_HIGH_OFFSET;
    }

    *value = _LCD_ReadIF(baseAddr, bits);

    return LCD_STATUS_OK;
}

BOOL LCD_IsLayerEnable(LCD_LAYER_ID id)
{
	ASSERT(id <= LCD_LAYER_NUM);
    return (BOOL)(layer_config[id].layer_en);
}

LCD_STATUS LCD_StartTransfer(BOOL blocking)
{
	unsigned int i;
	DBI_FUNC();
    _WaitForEngineNotBusy();
    DBG_OnTriggerLcd();
	disp_path_get_mutex();
	for(i = 0;i<DDP_OVL_LAYER_MUN;i++){
//		if(LCD_IsLayerEnable(i))
			disp_path_config_layer(&layer_config[i]);
	}

	LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0);
	LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, (1 << 15));

//	_WaitForEngineNotBusy();
	disp_path_release_mutex();
    if (blocking)
    {
        _WaitForEngineNotBusy();
    }
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigOVL()
{
	unsigned int i;
	DBI_FUNC();

	for(i = 0;i<DDP_OVL_LAYER_MUN;i++){
		if(LCD_IsLayerEnable(i))
			disp_path_config_layer(&layer_config[i]);
	}
    return LCD_STATUS_OK;
}



// -------------------- Retrieve Information --------------------

LCD_OUTPUT_MODE LCD_GetOutputMode(void)
{
	return _lcdContext.outputMode;
}
LCD_STATE  LCD_GetState(void)
{
    if (!s_isLcdPowerOn)
    {
        return LCD_STATE_POWER_OFF;
    }

    if (_IsEngineBusy())
    {
        return LCD_STATE_BUSY;
    }

    return LCD_STATE_IDLE;
}


LCD_STATUS LCD_DumpRegisters(void)
{
    UINT32 i;

    DISP_LOG_PRINT(ANDROID_LOG_WARN, "LCD", "---------- Start dump LCD registers ----------\n");
    
    for (i = 0; i < offsetof(LCD_REGS, GMC_ULTRA_TH); i += 4)
    {
        printk("LCD+%04x : 0x%08x\n", i, INREG32(LCD_BASE + i));
    }
	
    return LCD_STATUS_OK;
}

#if !defined(MTK_M4U_SUPPORT)
#ifdef BUILD_LK
static unsigned long v2p(unsigned long va)
{
    return va;
}
#else
static unsigned long v2p(unsigned long va)
{
    unsigned long pageOffset = (va & (PAGE_SIZE - 1));
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long pa;

    pgd = pgd_offset(current->mm, va); /* what is tsk->mm */
    pmd = pmd_offset(pgd, va);
    pte = pte_offset_map(pmd, va);
    
    pa = (pte_val(*pte) & (PAGE_MASK)) | pageOffset;


    return pa;
}
#endif
#endif
LCD_STATUS LCD_Get_VideoLayerSize(unsigned int id, unsigned int *width, unsigned int *height)
{
#if 0
    ASSERT(id < LCD_LAYER_NUM || LCD_LAYER_ALL == id);

    _WaitForEngineNotBusy();

    if (LCD_LAYER_ALL == id)
    {
        NOT_IMPLEMENTED();
    }
    else if (id < LCD_LAYER_NUM)
    {
        *width = LCD_REG->LAYER[id].SIZE.WIDTH;
        *height = LCD_REG->LAYER[id].SIZE.HEIGHT;
    }
#endif
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_Capture_Layerbuffer(unsigned int layer_id, unsigned int pvbuf, unsigned int bpp)
{
#if 0
    unsigned int ppbuf = 0;
    UINT16 offset_x, offset_y, w, h;
    LCD_OUTPUT_MODE mode;
    if(pvbuf == 0 || bpp == 0)
    {
        DBI_LOG("LCD_Capture_Layerbuffer, ERROR, parameters wrong: pvbuf=0x%08x, bpp=%d\n", pvbuf, bpp);
        return LCD_STATUS_ERROR;
    }

    if(layer_id >= LCD_LAYER_NUM)
    {
        DBI_LOG("LCD_Capture_Layerbuffer, ERROR, parameters wrong: layer_id=%d\n", layer_id);
        return LCD_STATUS_ERROR;
    }

    if(!LCD_IsLayerEnable(layer_id))
    {
        DBI_LOG("LCD_Capture_Layerbuffer, ERROR, the layer%d is not enabled\n", layer_id);
        return LCD_STATUS_ERROR;
    }

    LCD_WaitForNotBusy();

    _BackupLCDRegisters();

    // begin capture
    if(bpp == 32)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_ARGB8888));
    else if(bpp == 16)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_RGB565));
    else if(bpp == 24)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_RGB888));
    else
        DBI_LOG("mtkfb_capture_Videobuffer, fb color format not support\n");

	offset_x = LCD_REG->LAYER[layer_id].OFFSET.X;
	offset_y = LCD_REG->LAYER[layer_id].OFFSET.Y;
	w = LCD_REG->LAYER[layer_id].SIZE.WIDTH;
	h = LCD_REG->LAYER[layer_id].SIZE.HEIGHT;
#if defined(MTK_M4U_SUPPORT)
	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("[LCDC init M4U] M4U not been init for LCDC\n");
		return LCD_STATUS_ERROR;
	}
	
	_m4u_lcdc_func.m4u_alloc_mva(M4U_CLNTMOD_LCDC, pvbuf, w*h*bpp/8, &ppbuf);
#else
    ppbuf = v2p(pvbuf);
#endif
	ASSERT(ppbuf != 0);

	mode = LCD_GetOutputMode();
	
	LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_ALL, 0));
	LCD_CHECK_RET(LCD_LayerEnable(layer_id, 1));
	LCD_CHECK_RET(LCD_LayerSetRotation(layer_id, LCD_LAYER_ROTATE_0));
	LCD_CHECK_RET(LCD_SetRoiWindow(offset_x, offset_y, w, h));
	LCD_CHECK_RET(LCD_FBSetPitch((LCD_REG->LAYER[layer_id].SIZE.WIDTH)*bpp/8));

	LCD_CHECK_RET(LCD_FBSetStartCoord(0, 0));
	LCD_CHECK_RET(LCD_FBReset());
    
	LCD_CHECK_RET(LCD_FBSetAddress(LCD_FB_0, ppbuf));
	LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0, TRUE));

    LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_MEM));
    LCD_TE_Enable(FALSE);

	LCD_SetOutputAlpha(0xff);

//	LCD_CHECK_RET(LCD_SetRoiWindow(offset_x, offset_y, h, w));

	LCD_MASKREG32(0xf20a10a0, 0x1, 0); //disable DC to DSI when write to memory partially
#if defined(MTK_M4U_SUPPORT)
	_m4u_lcdc_func.m4u_dma_cache_maint(M4U_CLNTMOD_LCDC, (void *)pvbuf, w*h*bpp/8, DMA_BIDIRECTIONAL);
#endif

    LCD_CHECK_RET(LCD_StartTransfer(TRUE));
 
	LCD_SetOutputMode(mode);
    // capture end
    _RestoreLCDRegisters();
#if defined(MTK_M4U_SUPPORT)
    _m4u_lcdc_func.m4u_dealloc_mva(M4U_CLNTMOD_LCDC, pvbuf, w*h*bpp/8, ppbuf);
#endif
#endif
    return LCD_STATUS_OK;   
}


LCD_STATUS LCD_Capture_Videobuffer(unsigned int pvbuf, unsigned int bpp, unsigned int video_rotation)
{
#if 0
    unsigned int ppbuf = 0;
    UINT16 offset_x, offset_y, w, h;
    LCD_OUTPUT_MODE mode;
    if(pvbuf == 0 || bpp == 0)
    {
        DBI_LOG("LCD_Capture_Videobuffer, ERROR, parameters wrong: pvbuf=0x%08x, bpp=%d\n", pvbuf, bpp);
        return LCD_STATUS_OK;
    }
    
    LCD_WaitForNotBusy();

    _BackupLCDRegisters();

    // begin capture
    if(bpp == 32)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_ARGB8888));
    else if(bpp == 16)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_RGB565));
    else if(bpp == 24)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_RGB888));
    else
        DBI_LOG("mtkfb_capture_Videobuffer, fb color format not support\n");

	offset_x = LCD_REG->LAYER[LCD_LAYER_2].OFFSET.X;
	offset_y = LCD_REG->LAYER[LCD_LAYER_2].OFFSET.Y;
	w = LCD_REG->LAYER[LCD_LAYER_2].SIZE.WIDTH;
	h = LCD_REG->LAYER[LCD_LAYER_2].SIZE.HEIGHT;
#if defined(MTK_M4U_SUPPORT)
	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("[LCDC init M4U] M4U not been init for LCDC\n");
		return LCD_STATUS_ERROR;
	}
	
	_m4u_lcdc_func.m4u_alloc_mva(M4U_CLNTMOD_LCDC, pvbuf, w*h*bpp/8, &ppbuf);
#else
    ppbuf = v2p(pvbuf);
#endif
	mode = LCD_GetOutputMode();
	
	LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_ALL, 0));
	LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_2, 1));
	LCD_CHECK_RET(LCD_LayerSetRotation(LCD_LAYER_2, LCD_LAYER_ROTATE_0));
	LCD_CHECK_RET(LCD_SetRoiWindow(offset_x, offset_y, w, h));
	LCD_CHECK_RET(LCD_FBSetPitch((LCD_REG->LAYER[LCD_LAYER_2].SIZE.WIDTH)*bpp/8));

	switch(video_rotation)
	{
		case 0:break;
		case 1:
			LCD_CHECK_RET(LCD_LayerSetRotation(LCD_LAYER_2, LCD_LAYER_ROTATE_90));
			LCD_CHECK_RET(LCD_SetRoiWindow(offset_x, offset_y, h, w));
			LCD_CHECK_RET(LCD_FBSetPitch((LCD_REG->LAYER[LCD_LAYER_2].SIZE.HEIGHT)*bpp/8));
			break;
		case 2:
			LCD_CHECK_RET(LCD_LayerSetRotation(LCD_LAYER_2, LCD_LAYER_ROTATE_180));break;
		case 3:
			LCD_CHECK_RET(LCD_LayerSetRotation(LCD_LAYER_2, LCD_LAYER_ROTATE_270));
			LCD_CHECK_RET(LCD_SetRoiWindow(offset_x, offset_y, h, w));
			LCD_CHECK_RET(LCD_FBSetPitch((LCD_REG->LAYER[LCD_LAYER_2].SIZE.HEIGHT)*bpp/8));
			break;
		default:
		    DBI_LOG("[LCD_Capture_videobuffer] video_rotation = %d error\n", video_rotation);
	}

	LCD_CHECK_RET(LCD_FBSetStartCoord(0, 0));
	LCD_CHECK_RET(LCD_FBReset());
    
	LCD_CHECK_RET(LCD_FBSetAddress(LCD_FB_0, ppbuf));
	LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0, TRUE));

    LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_MEM));
    LCD_TE_Enable(FALSE);

	LCD_SetOutputAlpha(0xff);

//	LCD_CHECK_RET(LCD_SetRoiWindow(offset_x, offset_y, h, w));

	LCD_MASKREG32(0xf20a10a0, 0x1, 0); //disable DC to DSI when write to memory partially
#if defined(MTK_M4U_SUPPORT)
	_m4u_lcdc_func.m4u_dma_cache_maint(M4U_CLNTMOD_LCDC, (void *)pvbuf, w*h*bpp/8, DMA_BIDIRECTIONAL);
#endif

	if(dbi_log_on)
		LCD_DumpRegisters();

    LCD_CHECK_RET(LCD_StartTransfer(TRUE));
 
	LCD_SetOutputMode(mode);
    // capture end
    _RestoreLCDRegisters();
#if defined(MTK_M4U_SUPPORT)
    _m4u_lcdc_func.m4u_dealloc_mva(M4U_CLNTMOD_LCDC, pvbuf, w*h*bpp/8, ppbuf);
#endif
#endif
    return LCD_STATUS_OK;   
}

#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

LCD_STATUS LCD_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp)
{
#if 0
    unsigned int ppbuf = 0;
    LCD_OUTPUT_MODE mode;
	UINT16 offset_x, offset_y, w, h, i;
    if(pvbuf == 0 || bpp == 0)
    {
        DBI_LOG("LCD_Capture_Framebuffer, ERROR, parameters wrong: pvbuf=0x%08x, bpp=%d\n", pvbuf, bpp);
        return LCD_STATUS_OK;
    }

#if defined(MTK_M4U_SUPPORT)
	if (!_m4u_lcdc_func.isInit)
	{
        unsigned int t;
		unsigned int i,j;
		unsigned int w_xres = DISP_GetScreenWidth();
	    unsigned int w = ALIGN_TO(DISP_GetScreenWidth(), 32);
	    unsigned int h = DISP_GetScreenHeight();
		unsigned int pixel_bpp = bpp/8;
    	unsigned int fbsize = w*h*pixel_bpp;
		unsigned int fbv = (unsigned int)ioremap_cached((unsigned int)LCD_REG->LAYER[FB_LAYER].ADDRESS, fbsize);
		unsigned short* fbvt = (unsigned short*)fbv;

		DBI_LOG("[LCDC init M4U] M4U not been init for LCDC, this should only happened in recovery mode\n");
		for(i = 0;i < h; i++){
			for(j = 0;j < w_xres; j++){
	    		t = fbvt[j + i * w];
 	         	*(unsigned short*)(pvbuf+i*w_xres*pixel_bpp+j*pixel_bpp) = t;
    		}
		}
		iounmap((void *)fbv);
		return LCD_STATUS_OK;
	}
	else
		_m4u_lcdc_func.m4u_alloc_mva(M4U_CLNTMOD_LCDC, pvbuf, DISP_GetScreenHeight()*DISP_GetScreenWidth()*bpp/8, &ppbuf);
#else
    ppbuf = v2p(pvbuf);
#endif

    LCD_WaitForNotBusy();

    _BackupLCDRegisters();

    // begin capture
    if(bpp == 32)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_ARGB8888));
    else if(bpp == 16)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_RGB565));
    else if(bpp == 24)
        LCD_CHECK_RET(LCD_FBSetFormat(LCD_FB_FORMAT_RGB888));
    else
        DBI_LOG("mtkfb_capture_framebuffer, fb color format not support\n");

	mode = LCD_GetOutputMode();
    LCD_CHECK_RET(LCD_LayerSetRotation(LCD_LAYER_ALL, LCD_LAYER_ROTATE_0));
	
	//zaikuo wang, add to capture framebuffer for LCM PHYSICAL rotate 180
	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
		for(i = LCD_LAYER_0;i < LCD_LAYER_NUM;i++){
			offset_x = LCD_REG->LAYER[i].OFFSET.X;
			offset_y = LCD_REG->LAYER[i].OFFSET.Y;
			w = LCD_REG->LAYER[i].SIZE.WIDTH;
			h = LCD_REG->LAYER[i].SIZE.HEIGHT;
			offset_x = DISP_GetScreenWidth() - (offset_x + w);
			offset_y = DISP_GetScreenHeight() - (offset_y + h);
			LCD_CHECK_RET(LCD_LayerSetOffset(i, offset_x, offset_y));
		}
	}
    LCD_CHECK_RET(LCD_FBSetPitch(DISP_GetScreenWidth()*bpp/8));
    LCD_CHECK_RET(LCD_FBSetStartCoord(0, 0));
    LCD_CHECK_RET(LCD_FBSetAddress(LCD_FB_0, ppbuf));
    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0, TRUE));
    
    LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_MEM));

    LCD_TE_Enable(FALSE);
	LCD_SetOutputAlpha(0xff);
#if defined(MTK_M4U_SUPPORT)
	_m4u_lcdc_func.m4u_dma_cache_maint(M4U_CLNTMOD_LCDC, (unsigned int*)pvbuf, DISP_GetScreenHeight()*DISP_GetScreenWidth()*bpp/8, DMA_BIDIRECTIONAL);
#endif
	if(dbi_log_on)
		LCD_DumpRegisters();

    LCD_CHECK_RET(LCD_StartTransfer(TRUE));
	LCD_SetOutputMode(mode);
    // capture end
    _RestoreLCDRegisters();
#if defined(MTK_M4U_SUPPORT)
    _m4u_lcdc_func.m4u_dealloc_mva(M4U_CLNTMOD_LCDC, pvbuf, DISP_GetScreenHeight()*DISP_GetScreenWidth()*bpp/8, ppbuf);
#endif
#endif
    return LCD_STATUS_OK;    
}

LCD_STATUS LCD_FMDesense_Query()
{
    return LCD_STATUS_OK;
}

LCD_STATUS LCD_FM_Desense(LCD_IF_ID id, unsigned long freq)
{
    UINT32 a,b;
	UINT32 c,d;
	UINT32 wst,c2wh,chw,write_cycles;
	LCD_REG_PCNF config;
//	LCD_REG_WROI_CON ctrl;    
	LCD_REG_PCNFDW pcnfdw;

	LCD_OUTREG32_R(PLCD_REG_PCNF,&config,&LCD_REG->PARALLEL_CFG[(UINT32)id]);
    DBI_LOG("[enter LCD_FM_Desense]:parallel IF = 0x%x, ctrl = 0x%x\n",
		LCD_INREG32(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id]));
	wst = config.WST;
	c2wh = config.C2WH;
	// Config Delay Between Commands
//	OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));

	LCD_OUTREG32_R(PLCD_REG_PCNFDW,&pcnfdw,&LCD_REG->PARALLEL_DW);

	switch(id)
    {
        case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
        case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
        case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
        default : ASSERT(0);
    }

	a = 13000 - freq * 10 - 20;
	b = 13000 - freq * 10 + 20;
    write_cycles = wst + c2wh + chw + 2;//this is 6573 E1, E2 will change
	c = (a * write_cycles)%13000;
	d = (b * write_cycles)%13000;
	a = (a * write_cycles)/13000;
	b = (b * write_cycles)/13000;
	
	if((b > a)||(c == 0)||(d == 0)){//need modify setting to avoid interference
	    DBI_LOG("[LCD_FM_Desense] need to modify lcd setting, freq = %ld\n",freq);
	    wst -= wst_step_LCD;
		wst_step_LCD = 0 - wst_step_LCD;

		config.WST = wst;
		LCD_WaitForNotBusy();
		LCD_OUTREG32_R(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id], &config);
	}
	else{
		DBI_LOG("[LCD_FM_Desense] not need to modify lcd setting, freq = %ld\n",freq);
	}
	DBI_LOG("[leave LCD_FM_Desense]:parallel = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id]));
    return LCD_STATUS_OK;  
}

LCD_STATUS LCD_Reset_WriteCycle(LCD_IF_ID id)
{
	LCD_REG_PCNF config;
	UINT32 wst;
	DBI_LOG("[enter LCD_Reset_WriteCycle]:parallel = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id]));   
	if(wst_step_LCD > 0){//have modify lcd setting, so when fm turn off, we must decrease wst to default setting
	    DBI_LOG("[LCD_Reset_WriteCycle] need to reset lcd setting\n");
	    LCD_OUTREG32_R(PLCD_REG_PCNF,&config, &LCD_REG->PARALLEL_CFG[(UINT32)id]);
		wst = config.WST;
		wst -= wst_step_LCD;
		wst_step_LCD = 0 - wst_step_LCD;

		config.WST = wst;
		LCD_WaitForNotBusy();

		LCD_OUTREG32_R(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id], &config);

	}
	else{
		DBI_LOG("[LCD_Reset_WriteCycle] parallel is default setting, not need to reset it\n");
	}
	DBI_LOG("[leave LCD_Reset_WriteCycle]:parallel = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id]));
	return LCD_STATUS_OK; 
}

LCD_STATUS LCD_Get_Default_WriteCycle(LCD_IF_ID id, unsigned int *write_cycle)
{
	UINT32 wst,c2wh,chw;
	LCD_REG_PCNF config;
//	LCD_REG_WROI_CON ctrl;    
	LCD_REG_PCNFDW pcnfdw;
    
	if(is_get_default_write_cycle){
	    *write_cycle = default_write_cycle;
	    return LCD_STATUS_OK;
	}

	LCD_OUTREG32_R(PLCD_REG_PCNF,&config, &LCD_REG->PARALLEL_CFG[(UINT32)id]);

    DBI_LOG("[enter LCD_Get_Default_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n",
		LCD_INREG32(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id]));
	wst = config.WST;
	c2wh = config.C2WH;
	// Config Delay Between Commands
//	OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));
	LCD_OUTREG32_R(PLCD_REG_PCNFDW,&pcnfdw, &LCD_REG->PARALLEL_DW);

	switch(id)
    {
        case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
        case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
        case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
        default : ASSERT(0);
    }
    *write_cycle = wst + c2wh + chw + 2;
	default_write_cycle = *write_cycle;
	default_wst = wst;
	is_get_default_write_cycle = TRUE;
	DBI_LOG("[leave LCD_Get_Default_WriteCycle]:Default_Write_Cycle = %d\n", *write_cycle);   
    return LCD_STATUS_OK;  
}

LCD_STATUS LCD_Get_Current_WriteCycle(LCD_IF_ID id, unsigned int *write_cycle)
{
	UINT32 wst,c2wh,chw;
	LCD_REG_PCNF config;
//	LCD_REG_WROI_CON ctrl;       
    LCD_REG_PCNFDW pcnfdw;

	LCD_OUTREG32_R(PLCD_REG_PCNF,&config, &LCD_REG->PARALLEL_CFG[(UINT32)id]);

    DBI_LOG("[enter LCD_Get_Current_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n",
		LCD_INREG32(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id]));
	wst = config.WST;
	c2wh = config.C2WH;
	// Config Delay Between Commands
//	OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));
	LCD_OUTREG32_R(PLCD_REG_PCNFDW,&pcnfdw, &LCD_REG->PARALLEL_DW);
	switch(id)
    {
        case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
        case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
        case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
        default : ASSERT(0);
    }

    *write_cycle = wst + c2wh + chw + 2;//this is 6573 E1, E2 will change
	DBI_LOG("[leave LCD_Get_Current_WriteCycle]:Default_Write_Cycle = %d\n", *write_cycle);   
    return LCD_STATUS_OK;  
}

LCD_STATUS LCD_Change_WriteCycle(LCD_IF_ID id, unsigned int write_cycle)
{
	UINT32 wst;
	LCD_REG_PCNF config;

	LCD_OUTREG32_R(PLCD_REG_PCNF,&config, &LCD_REG->PARALLEL_CFG[(UINT32)id]);
    //DBI_LOG("[enter LCD_Change_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n",
		//INREG32(&LCD_REG->PARALLEL_CFG[(UINT32)id]),INREG32(&LCD_REG->WROI_CONTROL));

    DBI_LOG("[LCD_Change_WriteCycle] modify lcd setting\n");
	wst = write_cycle - default_write_cycle + default_wst;

	config.WST = wst;
	LCD_WaitForNotBusy();
	LCD_OUTREG32_R(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id], &config);

	DBI_LOG("[leave LCD_Change_WriteCycle]:parallel = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id]));
    return LCD_STATUS_OK;  
}

#if defined(MTK_M4U_SUPPORT)
LCD_STATUS LCD_InitM4U()
{
	M4U_PORT_STRUCT M4uPort;
	DBI_LOG("[LCDC driver]%s\n", __func__);
#ifdef MTK_LCDC_ENABLE_M4U
	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("[LCDC init M4U] M4U not been init for LCDC\n");
		return LCD_STATUS_ERROR;
	}

	M4uPort.ePortID = M4U_PORT_LCD_R;
	M4uPort.Virtuality = 1; 					   
	M4uPort.Security = 0;
	M4uPort.Distance = 1;
	M4uPort.Direction = 0;

	_m4u_lcdc_func.m4u_config_port(&M4uPort);

	M4uPort.ePortID = M4U_PORT_LCD_W;
	M4uPort.Virtuality = 1; 					   
	M4uPort.Security = 0;
	M4uPort.Distance = 1;
	M4uPort.Direction = 0;

	_m4u_lcdc_func.m4u_config_port(&M4uPort);
//    _m4u_lcdc_func.m4u_dump_reg(M4U_CLNTMOD_LCDC);
#endif
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_AllocUIMva(unsigned int va, unsigned int *mva, unsigned int size)
{
#ifdef MTK_LCDC_ENABLE_M4U
    if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("[LCDC init M4U] M4U not been init for LCDC\n");
		return LCD_STATUS_ERROR;
	}
    _m4u_lcdc_func.m4u_alloc_mva(M4U_CLNTMOD_LCDC_UI, va, size, mva);
#endif
#if 0
	_m4u_lcdc_func.m4u_insert_tlb_range(M4U_CLNTMOD_LCDC_UI,
                                  		 *mva,
                      					 *mva + size - 1,
                      					 RT_RANGE_HIGH_PRIORITY,
										 0);
#endif
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_AllocOverlayMva(unsigned int va, unsigned int *mva, unsigned int size)
{
#ifdef MTK_LCDC_ENABLE_M4U
    if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("[LCDC init M4U] M4U not been init for LCDC\n");
		return LCD_STATUS_ERROR;
	}
	_m4u_lcdc_func.m4u_alloc_mva(M4U_CLNTMOD_LCDC, va, size, mva);
	_m4u_lcdc_func.m4u_insert_tlb_range(M4U_CLNTMOD_LCDC,
                                  		 *mva,
                      					 *mva + size - 1,
                      					 RT_RANGE_HIGH_PRIORITY,
										 0);
#endif
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_DeallocMva(unsigned int va, unsigned int mva, unsigned int size)
{
#ifdef MTK_LCDC_ENABLE_M4U
    if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("[TV]Error, M4U has not init func for TV-out\n");
		return LCD_STATUS_ERROR;
	}
    _m4u_lcdc_func.m4u_invalid_tlb_range(M4U_CLNTMOD_LCDC, mva, mva + size - 1);
    _m4u_lcdc_func.m4u_dealloc_mva(M4U_CLNTMOD_LCDC, va, size, mva);
	return LCD_STATUS_OK;
#endif
}

LCD_STATUS LCD_M4UPowerOn(void)
{
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_M4UPowerOff(void)
{
	return LCD_STATUS_OK;
}

int m4u_alloc_mva_stub(M4U_MODULE_ID_ENUM eModuleID, const unsigned int BufAddr, const unsigned int BufSize, unsigned int *pRetMVABuf)
{
	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("%s, Error, M4U has not init func\n", __func__);
		return LCD_STATUS_ERROR;
	}
	return _m4u_lcdc_func.m4u_alloc_mva(eModuleID, BufAddr, BufSize, 0, 0, pRetMVABuf);
}

int m4u_dealloc_mva_stub(M4U_MODULE_ID_ENUM eModuleID, const unsigned int BufAddr, const unsigned int BufSize, const unsigned int MVA)
{
	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("%s, Error, M4U has not init func\n", __func__);
		return LCD_STATUS_ERROR;
	}
	return _m4u_lcdc_func.m4u_dealloc_mva(eModuleID, BufAddr, BufSize, MVA);
}

int m4u_insert_tlb_range_stub(M4U_MODULE_ID_ENUM eModuleID, 
                  unsigned int MVAStart, 
                  const unsigned int MVAEnd, 
                  M4U_RANGE_PRIORITY_ENUM ePriority,
                  unsigned int entryCount)
{
	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("%s, Error, M4U has not init func\n", __func__);
		return LCD_STATUS_ERROR;
	}

	return _m4u_lcdc_func.m4u_insert_tlb_range(eModuleID, MVAStart, MVAEnd, ePriority, entryCount);				  
}


int m4u_invalid_tlb_range_stub(M4U_MODULE_ID_ENUM eModuleID,
                  unsigned int MVAStart,
                  unsigned int MVAEnd)
{
	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("%s, Error, M4U has not init func\n", __func__);
		return LCD_STATUS_ERROR;
	}

	return _m4u_lcdc_func.m4u_invalid_tlb_range(eModuleID, MVAStart, MVAEnd);
}

int m4u_invalid_tlb_all_stub(M4U_MODULE_ID_ENUM eModuleID);  
int m4u_manual_insert_entry_stub(M4U_MODULE_ID_ENUM eModuleID, unsigned int EntryMVA, BOOL Lock); 
  
int m4u_config_port_stub(M4U_PORT_STRUCT* pM4uPort)
{
	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("%s, Error, M4U has not init func\n", __func__);
		return LCD_STATUS_ERROR;
	}

	return _m4u_lcdc_func.m4u_config_port(pM4uPort);
}

LCD_STATUS LCD_M4U_On(BOOL enable)
{
#ifdef MTK_LCDC_ENABLE_M4U
	M4U_PORT_STRUCT M4uPort;
	DBI_LOG("[LCDC driver]%s\n", __func__);

	if (!_m4u_lcdc_func.isInit)
	{
		DBI_LOG("[LCDC M4U ON] M4U not been init for LCDC\n");
		return LCD_STATUS_ERROR;
	}

	M4uPort.ePortID = M4U_PORT_LCD_R;
	M4uPort.Virtuality = enable; 					   
	M4uPort.Security = 0;
	M4uPort.Distance = 1;
	M4uPort.Direction = 0;

	_m4u_lcdc_func.m4u_config_port(&M4uPort);
#endif
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_DumpM4U(){
//	_m4u_lcdc_func.m4u_dump_reg(M4U_CLNTMOD_LCDC);
	_m4u_lcdc_func.m4u_dump_info(M4U_CLNTMOD_LCDC_UI);
	return LCD_STATUS_OK;
}

#endif

LCD_STATUS LCD_W2M_NeedLimiteSpeed(BOOL enable)
{
	limit_w2m_speed = enable;
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_W2TVR_NeedLimiteSpeed(BOOL enable)
{
	limit_w2tvr_speed = enable;
	return LCD_STATUS_OK;
}

LCD_STATUS LCD_SetGMCThrottle()
{
#if 0
	UINT32 total_req = 0;
	INT32 throttle_cnt = 0;

	if(!limit_w2m_speed && !limit_w2tvr_speed){
		LCD_OUTREG32(&LCD_REG->SMI_CON, (throttle_cnt << 16) | (0 << 4) | 0x4);
		return LCD_STATUS_OK;
	}

	for(i=0;i<LCD_LAYER_NUM;i++){
		if(LCD_IsLayerEnable((LCD_LAYER_ID)i)){
	  	    pitch = LCD_REG->LAYER[(LCD_LAYER_ID)i].WINDOW_PITCH;
   		    h = LCD_REG->LAYER[(LCD_LAYER_ID)i].SIZE.HEIGHT;
			total_req += pitch * h;
		}
	}

	if(total_req != 0)
		throttle_cnt = 34666666/total_req - 2;//34666666 is calculated by 130MHz/60fps*16
	
	if(throttle_cnt < 0)
		throttle_cnt = 0;	
	LCD_OUTREG32(&LCD_REG->GMC_CON, (throttle_cnt << 16) | (1 << 4) | 0x4);
#endif	
	return LCD_STATUS_OK;
}


// called by "esd_recovery_kthread"
BOOL LCD_esd_check(void)
{
#ifndef MT65XX_NEW_DISP
	static BOOL lcd_esd_check = false;


	UINT32 x, y, width, height;

	// Enable TE interrupt
	//LCD_TE_SetMode(LCD_TE_MODE_VSYNC_ONLY);
	//LCD_TE_SetEdgePolarity(LCM_POLARITY_RISING);
	LCD_TE_Enable(TRUE);

	// Backup ROI
	LCD_CHECK_RET(LCD_GetRoiWindow(&x, &y, &width, &height));

	// Set ROI = 0
	LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, 0, 0));

	// Switch to unuse port
	LCD_CHECK_RET(LCD_SelectWriteIF(LCD_IF_PARALLEL_2));

	// Write to LCM
	LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_LCM));

	// Blocking Trigger
	// This is to cheat LCDC to wait TE interrupt 
	LCD_CHECK_RET(LCD_StartTransfer(TRUE));

	// Restore ROI
	LCD_CHECK_RET(LCD_SetRoiWindow(x, y, width, height));

	// Disable TE interrupt
	LCD_TE_Enable(FALSE);

	// Write to memory	
	LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_MEM));

	return lcd_esd_check;
#endif	

    return TRUE;

}

