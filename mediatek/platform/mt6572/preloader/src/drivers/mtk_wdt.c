/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "typedefs.h"
#include "platform.h"
#include "mtk_wdt.h"

//#ifdef CONFIG_HW_WATCHDOG
//#ifdef CFG_HW_WATCHDOG
#if CFG_HW_WATCHDOG
static unsigned int timeout;
static unsigned int reboot_from = RE_BOOT_FROM_UNKNOW; 
static unsigned int rgu_mode = 0;
unsigned int g_rgu_status = RE_BOOT_REASON_UNKNOW;


static void mtk_wdt_reset_deglitch_enable(void)
{
    DRV_WriteReg32(MTK_WDT_RSTDEG_EN1, MTK_WDT_RSTDEG_EN1_KEY);
    DRV_WriteReg32(MTK_WDT_RSTDEG_EN2, MTK_WDT_RSTDEG_EN2_KEY);
}

static void mtk_wdt_disable(void)
{
    u32 tmp;

    tmp = DRV_Reg32(MTK_WDT_MODE);
    tmp &= ~MTK_WDT_MODE_ENABLE;       /* disable watchdog */
    tmp |= (MTK_WDT_MODE_KEY);         /* need key then write is allowed */
    DRV_WriteReg32(MTK_WDT_MODE,tmp);
}

static void mtk_wdt_reset(char mode)
{
    /* Watchdog Rest */
    unsigned int wdt_mode_val;
    DRV_WriteReg32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY); 

    wdt_mode_val = DRV_Reg32(MTK_WDT_MODE);
    /* clear autorestart bit: autoretart: 1, bypass power key, 0: not bypass power key */
    wdt_mode_val &=(~MTK_WDT_MODE_AUTO_RESTART);
	/* make sure WDT mode is hw reboot mode, can not config isr mode  */
	//wdt_mode_val &=(~(MTK_WDT_MODE_IRQ|MTK_WDT_MODE_ENABLE | MTK_WDT_MODE_DUAL_MODE));
	
		wdt_mode_val &= ~MTK_WDT_MODE_DDRRSV_MODE;

    if(mode){ /* mode != 0 means by pass power key reboot, We using auto_restart bit as by pass power key flag */
        wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN|MTK_WDT_MODE_AUTO_RESTART);
        DRV_WriteReg32(MTK_WDT_MODE, wdt_mode_val);
       
    }else{
         wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN);
         DRV_WriteReg32(MTK_WDT_MODE,wdt_mode_val); 
        
    }
    
    gpt_busy_wait_us(100);
    DRV_WriteReg32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
}


static unsigned int mtk_wdt_check_status(void)
{
    static unsigned int status=0;

    /**
     * Note:
     *   Because WDT_STA register will be cleared after writing WDT_MODE,
     *   we use a static variable to store WDT_STA.
     *   After reset, static varialbe will always be clear to 0,
     *   so only read WDT_STA when static variable is 0 is OK
     */
    if(0 == status)
        status = DRV_Reg32(MTK_WDT_STATUS);

    return status;
}

/**
 * For Power off and power on reset, the INTERVAL default value is 0x7FF.
 * We set Interval[1:0] to different value to distinguish different stage.
 * Enter pre-loader, we will set it to 0x0
 * Enter u-boot, we will set it to 0x1
 * Enter kernel, we will set it to 0x2
 * And the default value is 0x3 which means reset from a power off and power on reset
 */
#define POWER_OFF_ON_MAGIC	(0x3)
#define PRE_LOADER_MAGIC	(0x0)
#define U_BOOT_MAGIC		(0x1)
#define KERNEL_MAGIC		(0x2)
#define MAGIC_NUM_MASK		(0x3)
/**
 * If the reset is trigger by RGU(Time out or SW trigger), we hope the system can boot up directly;
 * we DO NOT hope we must press power key to reboot system after reset.
 * This message should tell pre-loader and u-boot, and we use Interval[2] to store this information.
 * And this information will be cleared after uboot check it.
 */
#define IS_POWER_ON_RESET	(0x1<<2)
#define RGU_TRIGGER_RESET_MASK	(0x1<<2)

int mtk_wdt_boot_check(void);

static void mtk_wdt_check_trig_reboot_reason(void)
{
    unsigned int interval_val = DRV_Reg32(MTK_WDT_INTERVAL);

    reboot_from = RE_BOOT_FROM_UNKNOW;

    /* 1. Get reboot reason */
    if(0 != mtk_wdt_check_status()){
        /* Enter here means this reset is triggered by RGU(WDT) */
        print("PL RGU RST: ");
        switch(interval_val&MAGIC_NUM_MASK)
        {
        case PRE_LOADER_MAGIC:
            reboot_from = RE_BOOT_FROM_PRE_LOADER;
            print("P\n");
            break;
        case U_BOOT_MAGIC:
            reboot_from = RE_BOOT_FROM_U_BOOT;
            print("U\n");
            break;
        case KERNEL_MAGIC:
            reboot_from = RE_BOOT_FROM_KERNEL;
            print("K\n");
            break;
        default:
            print("??\n"); // RGU reset, but not pr-loader, u-boot, kernel, from where???
            break;
        }
    }else{
        /* Enter here means reset may triggered by power off power on */
        if( (interval_val&MAGIC_NUM_MASK) == POWER_OFF_ON_MAGIC ){
            reboot_from = RE_BOOT_FROM_POWER_ON;
            print("PL P ON\n");
        }else{
            print("PL ?!\n"); // Not RGU trigger reset, and not defautl value, why?
        }
    }

    /* 2. Update interval register value and set reboot flag for u-boot */
    interval_val &= ~(RGU_TRIGGER_RESET_MASK|MAGIC_NUM_MASK);
    interval_val |= PRE_LOADER_MAGIC;
    if(reboot_from == RE_BOOT_FROM_POWER_ON)
    	interval_val |= IS_POWER_ON_RESET; // bit2==0 means RGU reset
    DRV_WriteReg32(MTK_WDT_INTERVAL, interval_val);

    /* 3. By pass power key info */
    if (mtk_wdt_boot_check() == WDT_BY_PASS_PWK_REBOOT) {
        print("Find bypass powerkey flag\n");
    } else if (mtk_wdt_boot_check() == WDT_NORMAL_REBOOT) {
        print("No bypass powerkey flag\n");
    } else {
        print("WDT does not trigger reboot\n");
    }
}


static void mtk_wdt_mode_config(	BOOL dual_mode_en, 
					BOOL irq, 
					BOOL ext_en, 
					BOOL ext_pol, 
					BOOL wdt_en )
{
	unsigned int tmp;
    
	//print(" mtk_wdt_mode_config  mode value=%x\n",DRV_Reg32(MTK_WDT_MODE));
	tmp = DRV_Reg32(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;

	// Bit 0 : Whether enable watchdog or not
	if(wdt_en == TRUE)
		tmp |= MTK_WDT_MODE_ENABLE;
	else
		tmp &= ~MTK_WDT_MODE_ENABLE;

	// Bit 1 : Configure extern reset signal polarity.
	if(ext_pol == TRUE)
		tmp |= MTK_WDT_MODE_EXT_POL;
	else
		tmp &= ~MTK_WDT_MODE_EXT_POL;

	// Bit 2 : Whether enable external reset signal
	if(ext_en == TRUE)
		tmp |= MTK_WDT_MODE_EXTEN;
	else
		tmp &= ~MTK_WDT_MODE_EXTEN;

	// Bit 3 : Whether generating interrupt instead of reset signal
	if(irq == TRUE)
		tmp |= MTK_WDT_MODE_IRQ;
	else
		tmp &= ~MTK_WDT_MODE_IRQ;

	// Bit 6 : Whether enable debug module reset
	if(dual_mode_en == TRUE)
		tmp |= MTK_WDT_MODE_DUAL_MODE;
	else
		tmp &= ~MTK_WDT_MODE_DUAL_MODE;

	// Bit 4: WDT_Auto_restart, this is a reserved bit, we use it as bypass powerkey flag.
	//		Because HW reboot always need reboot to kernel, we set it always.
	tmp |= MTK_WDT_MODE_AUTO_RESTART;

	DRV_WriteReg32(MTK_WDT_MODE,tmp);
	//dual_mode(1); //always dual mode
	//mdelay(100);
	print(" mtk_wdt_mode_config  mode value=%x, tmp:%x\n",DRV_Reg32(MTK_WDT_MODE), tmp);

}
//EXPORT_SYMBOL(mtk_wdt_mode_config);

static void mtk_wdt_set_time_out_value(UINT32 value)
{
    /*
    * TimeOut = BitField 15:5
    * Key      = BitField  4:0 = 0x08
    */

    // sec * 32768 / 512 = sec * 64 = sec * 1 << 6
    timeout = (unsigned int)(value * ( 1 << 6) );
    timeout = timeout << 5; 
    DRV_WriteReg32(MTK_WDT_LENGTH, (timeout | MTK_WDT_LENGTH_KEY) );  
}

void mtk_wdt_restart(void)
{
    // Reset WatchDogTimer's counting value to time out value
    // ie., keepalive()

    DRV_WriteReg32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
}

void mtk_wdt_sw_reset (void)
{
    print ("WDT SW RESET\n");
    //DRV_WriteReg32 (0x70025000, 0x2201);
    //DRV_WriteReg32 (0x70025008, 0x1971);
    //DRV_WriteReg32 (0x7002501C, 0x1209);
    mtk_wdt_reset(1);/* NOTE here, this reset will cause by pass power key */

    // system will reset 

    while (1)
    {
        print ("SW reset fail ... \n");
    };
}

static void mtk_wdt_hw_reset(void)
{
    print("WDT_HW_Reset_test\n");

    // 1. set WDT timeout 1 secs, 1*64*512/32768 = 1sec
    mtk_wdt_set_time_out_value(1);

    // 2. enable WDT debug reset enable, generating irq disable, ext reset disable
    //    ext reset signal low, wdt enalbe
    mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, TRUE);

    // 3. reset the watch dog timer to the value set in WDT_LENGTH register
    mtk_wdt_restart();

    // 4. system will reset
    while(1);
} 

BOOL mtk_is_rgu_trigger_reset()
{
    if(reboot_from == RE_BOOT_FROM_POWER_ON)
        return FALSE;
    return TRUE;
}

int mtk_wdt_boot_check(void)
{
    unsigned int wdt_sta = mtk_wdt_check_status();

    #if 0
	if (wdt_sta == MTK_WDT_STATUS_HWWDT_RST) { /* For E1 bug, that SW reset value is 0xC000, we using "==" to check */
    	/* Time out reboot always by pass power key */
        print ("TO reset, need bypass power key\n");
        return WDT_BY_PASS_PWK_REBOOT;

    } else if (wdt_sta & MTK_WDT_STATUS_SWWDT_RST) {
    #endif
    /* 
     * For DA download hope to timeout reboot, and boot to u-boot/kernel configurable reason,
     * we set both timeout reboot and software reboot can check whether bypass power key.
     */
    if (wdt_sta & (MTK_WDT_STATUS_HWWDT_RST|MTK_WDT_STATUS_SWWDT_RST|MTK_WDT_STATUS_PCMWDT_RST|MTK_WDT_STATUS_SPMWDT_RST)) {
        if (rgu_mode & MTK_WDT_MODE_AUTO_RESTART) {
            /* HW/SW reboot, and auto restart is set, means bypass power key */
            print ("SW reset with bypass power key flag\n");
            return WDT_BY_PASS_PWK_REBOOT;
        } else {
            print ("SW reset without bypass power key flag\n");
            return WDT_NORMAL_REBOOT;
        }
    }

    return WDT_NOT_WDT_REBOOT;
}

void mtk_wdt_init(void)
{
		unsigned wdt_sta;
    /* Dump RGU regisers */
    print("==== Dump RGU Reg ========\n");
    print("RGU MODE:     %x\n", DRV_Reg32(MTK_WDT_MODE));
    print("RGU LENGTH:   %x\n", DRV_Reg32(MTK_WDT_LENGTH));
    print("RGU STA:      %x\n", DRV_Reg32(MTK_WDT_STATUS));
    print("RGU INTERVAL: %x\n", DRV_Reg32(MTK_WDT_INTERVAL));
    print("RGU SWSYSRST: %x\n", DRV_Reg32(MTK_WDT_SWSYSRST));
    print("==== Dump RGU Reg End ====\n");
    
    rgu_mode = DRV_Reg32(MTK_WDT_MODE);

    wdt_sta = mtk_wdt_check_status(); // This function will store the reset reason: Time out/ SW trigger
	if (((wdt_sta & MTK_WDT_STATUS_HWWDT_RST) || (wdt_sta & MTK_WDT_STATUS_PCMWDT_RST))
				&& (rgu_mode&MTK_WDT_MODE_AUTO_RESTART)) 
	{ /* For E1 bug, that SW reset value is 0xC000, we using "==" to check */
    	/* Time out reboot always by pass power key */
        g_rgu_status = RE_BOOT_BY_WDT_HW;

    } 
	else if (wdt_sta & MTK_WDT_STATUS_SWWDT_RST) 
	{
    	g_rgu_status = RE_BOOT_BY_WDT_SW;
    }
    else 
	{
    	g_rgu_status = RE_BOOT_REASON_UNKNOW;
    }

	if(wdt_sta & MTK_WDT_STATUS_IRQWDT_RST)
	{
	   g_rgu_status |= RE_BOOT_WITH_INTTERUPT;
	}

	if(wdt_sta & MTK_WDT_STATUS_SPMWDT_RST)
	{
	   g_rgu_status |= RE_BOOT_WITH_THERMAL;
	}
	if(wdt_sta & MTK_WDT_STATUS_PCMWDT_RST)
	{
	   g_rgu_status |= RE_BOOT_BY_SPM;
	}

    print ("RGU: g_rgu_satus:%d\n", g_rgu_status);	    
    mtk_wdt_mode_config(FALSE, FALSE, FALSE, FALSE, FALSE); // Wirte Mode register will clear status register
    mtk_wdt_check_trig_reboot_reason();
    /* Setting timeout 10s */
    mtk_wdt_set_time_out_value(16);

    #if !CFG_APWDT_DISABLE
    /* Config HW reboot mode */
    mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, TRUE); 
    mtk_wdt_restart();
    #endif
    
    mtk_wdt_reset_deglitch_enable();

}

void mtk_arch_reset(char mode)
{
    print("mtk_arch_reset at pre-loader!\n");

    //mtk_wdt_hw_reset();
    mtk_wdt_reset(mode);

    while (1);
}

#else // Using dummy WDT functions
void mtk_wdt_init(void)
{
    print("PL WDT Dummy init called\n");
}
BOOL mtk_is_rgu_trigger_reset()
{
    print("PL Dummy mtk_is_rgu_trigger_reset called\n");
    return FALSE;
}
void mtk_arch_reset(char mode)
{
    print("PL WDT Dummy arch reset called\n");
}

int mtk_wdt_boot_check(void)
{
    print("PL WDT Dummy mtk_wdt_boot_check called\n");
    return 0;
}

#endif
