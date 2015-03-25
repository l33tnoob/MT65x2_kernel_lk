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
#include "boot_device.h"
#include "nand.h"
#include "mmc_common_inter.h"

#include "uart.h"
#include "nand_core.h"
#include "pll.h"
#include "i2c.h"
#include "rtc.h"
#include "emi.h"
#include "pmic.h"
#include "wdt.h"
#include "ram_console.h"
#include "cust_sec_ctrl.h"
#include "gpio.h"
#include "pmic_wrap_init.h"
#include "keypad.h"
#include "usbphy.h"
#include "timer.h"
#include "dram_buffer.h"
#include "project.h"

/*============================================================================*/
/* CONSTAND DEFINITIONS                                                       */
/*============================================================================*/
#define MOD "[PLFM]"

/*============================================================================*/
/* GLOBAL VARIABLES                                                           */
/*============================================================================*/
unsigned int sys_stack[CFG_SYS_STACK_SZ >> 2];
const unsigned int sys_stack_sz = CFG_SYS_STACK_SZ;
boot_mode_t g_boot_mode;
boot_dev_t g_boot_dev;
meta_com_t g_meta_com_type = META_UNKNOWN_COM;
u32 g_meta_com_id = 0;
boot_reason_t g_boot_reason;
ulong g_boot_time;
u32 g_ddr_reserve_enable = 0;
u32 g_ddr_reserve_success = 0;

bl_param_t g_bl_param;

extern unsigned int part_num;
extern part_hdr_t   part_info[PART_MAX_NUM];

#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
kal_bool kpoc_flag = false;
#endif

#if (CFG_USB_UART_SWITCH)
kal_bool uart_mode = false;
#endif

#if CFG_BOOT_ARGUMENT
#define bootarg g_dram_buf->bootarg
#endif

#if CFG_USB_AUTO_DETECT
bool g_usbdl_flag;		
#endif


/*============================================================================*/
/* EXTERNAL FUNCTIONS                                                         */
/*============================================================================*/
static u32 boot_device_init(void)
{
    #if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    return (u32)mmc_init_device();
    #else
    return (u32)nand_init_device();
    #endif
}

int usb_accessory_in(void)
{
#if !CFG_FPGA_PLATFORM
    int exist = 0;

    if (PMIC_CHRDET_EXIST == pmic_IsUsbCableIn()) {
        exist = 1;
        #if !CFG_USBIF_COMPLIANCE
        /* enable charging current as early as possible to avoid can't enter 
         * following battery charging flow when low battery
         */
        platform_set_chrg_cur(450);
        #endif
    }
    return exist;
#else
    return 1;
#endif
}

extern bool is_uart_cable_inserted(void);

int usb_cable_in(void)
{
#if !CFG_FPGA_PLATFORM
    int exist = 0;
    CHARGER_TYPE ret;

    if ((g_boot_reason == BR_USB) || usb_accessory_in()) {
        ret = mt_charger_type_detection();
        if (ret == STANDARD_HOST || ret == CHARGING_HOST) {
            print("\n%s USB cable in\n", MOD);
            mt_usb_phy_poweron();
            mt_usb_phy_savecurrent();

            /* enable pmic hw charger detection */
            #if CFG_BATTERY_DETECT
            if (hw_check_battery())
                pl_hw_ulc_det();
            #endif

            exist = 1;
        } else if (ret == NONSTANDARD_CHARGER || ret == STANDARD_CHARGER) {
            print("\n%s USB cable in - NONSTANDARD CHARGER or STANDARD CHARGER\n", MOD);
            #if CFG_USBIF_COMPLIANCE
            platform_set_chrg_cur(450);
            #endif

            if (uart_mode) {
		        print("\n%s Switch back to USB Mode\n", MOD);
                set_to_usb_mode();
            }
        }
    }

    return exist;
#else
    print("\n%s USB cable in\n", MOD);
    mt_usb_phy_poweron();
    mt_usb_phy_savecurrent();

    return 1;
#endif
}

#if CFG_FPGA_PLATFORM
void show_tx(void)
{
	UINT8 var;
	USBPHY_I2C_READ8(0x6E, &var);
	UINT8 var2 = (var >> 3) & ~0xFE; 
	print("[USB]addr: 0x6E (TX), value: %x - %x\n", var, var2);
}

void store_tx(UINT8 value)
{
	UINT8 var;
	UINT8 var2;
	USBPHY_I2C_READ8(0x6E, &var);

	if (value == 0) {
		var2 = var & ~(1 << 3); 
	} else {
		var2 = var | (1 << 3); 
	}

	USBPHY_I2C_WRITE8(0x6E, var2);
	USBPHY_I2C_READ8(0x6E, &var);
	var2 = (var >> 3) & ~0xFE;

	print("[USB]addr: 0x6E TX [AFTER WRITE], value after: %x - %x\n", var, var2);
}

void show_rx(void)
{
	UINT8 var;
	USBPHY_I2C_READ8(0x77, &var);
	UINT8 var2 = (var >> 7) & ~0xFE; 
	print("[USB]addr: 0x77 (RX) [AFTER WRITE], value after: %x - %x\n", var, var2);
}

void test_uart(void)
{
	int i=0;
	UINT8 val = 0;
	for (i=0; i<1000; i++)
	{
			show_tx();
            mdelay(300);
			if (val) {
				val = 0;
			}
			else {
				val = 1;
			}
			store_tx(val);
			show_rx();
            mdelay(1000);
	}
}
#endif

void set_to_usb_mode(void)
{
		UINT8 var;
#if !CFG_FPGA_PLATFORM
		/* Turn on USB MCU Bus Clock */
		var = READ_REG(PERI_GLOBALCON_PDN0_SET);
		print("\n[USB]USB bus clock: 0x008, value: %x\n", var);
		USB_CLR_BIT(USB0_PDN, PERI_GLOBALCON_PDN0_SET);
		var = READ_REG(PERI_GLOBALCON_PDN0_SET);
		print("\n[USB]USB bus clock: 0x008, value after: %x\n", var);

		/* Switch from BC1.1 mode to USB mode */
		var = USBPHY_READ8(0x1A);
		print("\n[USB]addr: 0x1A, value: %x\n", var);
		USBPHY_WRITE8(0x1A, var & 0x7f);
		print("\n[USB]addr: 0x1A, value after: %x\n", USBPHY_READ8(0x1A));

		/* Set RG_UART_EN to 0 */
		var = USBPHY_READ8(0x6E);
		print("\n[USB]addr: 0x6E, value: %x\n", var);
		USBPHY_WRITE8(0x6E, var & ~0x01);
		print("\n[USB]addr: 0x6E, value after: %x\n", USBPHY_READ8(0x6E));

		/* Set RG_USB20_DM_100K_EN to 0 */
		var = USBPHY_READ8(0x22);
		print("\n[USB]addr: 0x22, value: %x\n", var);
		USBPHY_WRITE8(0x22, var & ~0x02);
		print("\n[USB]addr: 0x22, value after: %x\n", USBPHY_READ8(0x22));
#else
		/* Set RG_UART_EN to 0 */
		USBPHY_I2C_READ8(0x6E, &var);
		print("\n[USB]addr: 0x6E, value: %x\n", var);
		USBPHY_I2C_WRITE8(0x6E, var & ~0x01);
		USBPHY_I2C_READ8(0x6E, &var);
		print("\n[USB]addr: 0x6E, value after: %x\n", var);

		/* Set RG_USB20_DM_100K_EN to 0 */
		USBPHY_I2C_READ8(0x22, &var);
		print("\n[USB]addr: 0x22, value: %x\n", var);
		USBPHY_I2C_WRITE8(0x22, var & ~0x02);
		USBPHY_I2C_READ8(0x22, &var);
		print("\n[USB]addr: 0x22, value after: %x\n", var);
#endif
		var = READ_REG(UART1_BASE + 0x90);
		print("\n[USB]addr: 0x11002090 (UART1), value: %x\n", var);
		WRITE_REG(var & ~0x01, UART1_BASE + 0x90);
		print("\n[USB]addr: 0x11002090 (UART1), value after: %x\n", READ_REG(UART1_BASE + 0x90));
}

void set_to_uart_mode(void)
{
		UINT8 var;
#if !CFG_FPGA_PLATFORM
		/* Turn on USB MCU Bus Clock */
		var = READ_REG(PERI_GLOBALCON_PDN0_SET);
		print("\n[USB]USB bus clock: 0x008, value: %x\n", var);
		USB_CLR_BIT(USB0_PDN, PERI_GLOBALCON_PDN0_SET);
		var = READ_REG(PERI_GLOBALCON_PDN0_SET);
		print("\n[USB]USB bus clock: 0x008, value after: %x\n", var);

		/* Switch from BC1.1 mode to USB mode */
		var = USBPHY_READ8(0x1A);
		print("\n[USB]addr: 0x1A, value: %x\n", var);
		USBPHY_WRITE8(0x1A, var & 0x7f);
		print("\n[USB]addr: 0x1A, value after: %x\n", USBPHY_READ8(0x1A));

		/* Set ru_uart_mode to 2'b01 */
		var = USBPHY_READ8(0x6B);
		print("\n[USB]addr: 0x6B, value: %x\n", var);
		USBPHY_WRITE8(0x6B, var | 0x5C);
		print("\n[USB]addr: 0x6B, value after: %x\n", USBPHY_READ8(0x6B));

		/* Set RG_UART_EN to 1 */
		var = USBPHY_READ8(0x6E);
		print("\n[USB]addr: 0x6E, value: %x\n", var);
		USBPHY_WRITE8(0x6E, var | 0x07);
		print("\n[USB]addr: 0x6E, value after: %x\n", USBPHY_READ8(0x6E));

		/* Set RG_USB20_DM_100K_EN to 1 */
		var = USBPHY_READ8(0x22);
		print("\n[USB]addr: 0x22, value: %x\n", var);
		USBPHY_WRITE8(0x22, var | 0x02);
		print("\n[USB]addr: 0x22, value after: %x\n", USBPHY_READ8(0x22));

		/* Set RG_SUSPENDM to 1 */
		var = USBPHY_READ8(0x68);
		print("\n[USB]addr: 0x68, value: %x\n", var);
		USBPHY_WRITE8(0x68, var | 0x08);
		print("\n[USB]addr: 0x68, value after: %x\n", USBPHY_READ8(0x68));

		/* force suspendm = 1 */
		var = USBPHY_READ8(0x6A);
		print("\n[USB]addr: 0x6A, value: %x\n", var);
		USBPHY_WRITE8(0x6A, var | 0x04);
		print("\n[USB]addr: 0x6A, value after: %x\n", USBPHY_READ8(0x6A));
#else
		/* Set ru_uart_mode to 2'b01 */
		USBPHY_I2C_READ8(0x6B, &var);
		print("\n[USB]addr: 0x6B, value: %x\n", var);
		USBPHY_I2C_WRITE8(0x6B, var | 0x7C);
		USBPHY_I2C_READ8(0x6B, &var);
		print("\n[USB]addr: 0x6B, value after: %x\n", var);

		/* Set RG_UART_EN to 1 */
		USBPHY_I2C_READ8(0x6E, &var);
		print("\n[USB]addr: 0x6E, value: %x\n", var);
		USBPHY_I2C_WRITE8(0x6E, var | 0x07);
		USBPHY_I2C_READ8(0x6E, &var);
		print("\n[USB]addr: 0x6E, value after: %x\n", var);

		/* Set RG_USB20_DM_100K_EN to 1 */
		USBPHY_I2C_READ8(0x22, &var);
		print("\n[USB]addr: 0x22, value: %x\n", var);
		USBPHY_I2C_WRITE8(0x22, var | 0x02);
		USBPHY_I2C_READ8(0x22, &var);
		print("\n[USB]addr: 0x22, value after: %x\n", var);
#endif
		mdelay(100);

        var = DRV_Reg32(UART0_BASE+0x90);
		print("\n[USB]addr: 0x11002090 (UART1), value: %x\n", var);
		DRV_WriteReg32(UART0_BASE+0x90, 0x0001);
		print("\n[USB]addr: 0x11002090 (UART1), value after: %x\n", DRV_Reg32(UART0_BASE+0x90));
}

void platform_vusb_on(void)
{
#if !CFG_FPGA_PLATFORM
    U32 ret=0;

    ret=pmic_config_interface( (U32)(DIGLDO_CON2),
                               (U32)(1),
                               (U32)(PMIC_RG_VUSB_EN_MASK),
                               (U32)(PMIC_RG_VUSB_EN_SHIFT)
	                         );
    if(ret!=0)
    {
        print("[platform_vusb_on] Fail\n");
    }
    else
    {
        print("[platform_vusb_on] PASS\n");
    }    
#endif
    return;
}

void platform_set_boot_args()
{
#if CFG_BOOT_ARGUMENT
    bootarg.magic = BOOT_ARGUMENT_MAGIC;
    bootarg.mode  = g_boot_mode;
    bootarg.e_flag = sp_check_platform();
    bootarg.log_port = CFG_UART_LOG;
    bootarg.log_baudrate = CFG_LOG_BAUDRATE;
    bootarg.log_enable = (u8)log_status();
    bootarg.dram_rank_num = get_dram_rank_nr();
    get_dram_rank_size(bootarg.dram_rank_size);
    bootarg.boot_reason = g_boot_reason;
    bootarg.meta_com_type = (u32)g_meta_com_type;
    bootarg.meta_com_id = g_meta_com_id;
    bootarg.boot_time = get_timer(g_boot_time);

    bootarg.part_num =  g_dram_buf->part_num;
    bootarg.part_info = g_dram_buf->part_info;

    bootarg.ddr_reserve_enable = g_ddr_reserve_enable;
    bootarg.ddr_reserve_success= g_ddr_reserve_success;
    bootarg.chip_ver = DRV_Reg32(APHW_CODE);

#if CFG_WORLD_PHONE_SUPPORT
    print("%s md_type[0] = %d \n", MOD, bootarg.md_type[0]);
    print("%s md_type[1] = %d \n", MOD, bootarg.md_type[1]);
#endif

    print("\n%s boot reason: %d\n", MOD, g_boot_reason);
    print("%s boot mode: %d\n", MOD, g_boot_mode);
    print("%s META COM%d: %d\n", MOD, bootarg.meta_com_id, bootarg.meta_com_type);
    print("%s <0x%x>: 0x%x\n", MOD, &bootarg.e_flag, bootarg.e_flag);
    print("%s boot time: %dms\n", MOD, bootarg.boot_time);
    print("%s DDR reserve mode: enable = %d, success = %d\n", MOD, bootarg.ddr_reserve_enable, bootarg.ddr_reserve_success);
#endif

}

void platform_set_dl_boot_args(da_info_t *da_info)
{
#if CFG_BOOT_ARGUMENT
    if (da_info->addr != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.addr = da_info->addr;

    if (da_info->arg1 != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.arg1 = da_info->arg1;

    if (da_info->arg2 != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.arg2 = da_info->arg2;

    if (da_info->len != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.len = da_info->len;

    if (da_info->sig_len != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.sig_len = da_info->sig_len;
#endif

    return;
}

void platform_wdt_all_kick(void)
{
    /* kick watchdog to avoid cpu reset */
    mtk_wdt_restart();

#if !CFG_FPGA_PLATFORM
    /* kick PMIC watchdog to keep charging */
    pl_kick_chr_wdt();
#endif
}

void platform_wdt_kick(void)
{
    /* kick hardware watchdog */
    mtk_wdt_restart();
}

#if CFG_DT_MD_DOWNLOAD
void platform_modem_download(void)
{
    print("[%s] modem download...\n", MOD);

    while (1) {
        platform_wdt_all_kick();
    }
}
#endif

#if CFG_USB_AUTO_DETECT
void platform_usbdl_flag_check()
{	
    U32 usbdlreg = 0;
    usbdlreg = DRV_Reg32(SRAMROM_USBDL);
     /*Set global variable to record the usbdl flag*/
    if(usbdlreg & USBDL_BIT_EN)
        g_usbdl_flag = 1;
    else
        g_usbdl_flag = 0;            
}

void platform_usb_auto_detect_flow()
{

    print("USB DL Flag is %d when enter preloader  \n",g_usbdl_flag);

    /*usb download flag haven't set */
	if(g_usbdl_flag == 0 && g_boot_reason != BR_RTC){
        /*set up usbdl flag*/
        platform_safe_mode(1,CFG_USB_AUTO_DETECT_TIMEOUT_MS);
        print("Preloader going reset and trigger BROM usb auto detectiton!!\n");

    /* WDT by pass powerkey reboot */
        /* keep the previous status, pass it into reset function */
        if (WDT_BY_PASS_PWK_REBOOT == mtk_wdt_boot_check())
            mtk_arch_reset(1);
        else
            mtk_arch_reset(0);
       
           
	}else{
    /*usb download flag have been set*/
    }
    
}
#endif


void platform_safe_mode(int en, u32 timeout)
{
    U32 usbdlreg = 0;

    /* if anything is wrong and caused wdt reset, enter bootrom download mode */
    timeout = !timeout ? USBDL_TIMEOUT_MAX : timeout / 1000;
    timeout <<= 2;
    timeout &= USBDL_TIMEOUT_MASK; /* usbdl timeout cannot exceed max value */

    usbdlreg |= timeout;
    if (en)
	    usbdlreg |= USBDL_BIT_EN;
    else
	    usbdlreg &= ~USBDL_BIT_EN;

    usbdlreg &= ~USBDL_BROM ;
    /*Add magic number for MT6582*/
    usbdlreg |= USBDL_MAGIC;
        
    DRV_WriteReg32(SRAMROM_USBDL, usbdlreg);

    return;
}

#if CFG_EMERGENCY_DL_SUPPORT
void platform_emergency_download(u32 timeout)
{
    /* enter download mode */
    print("%s emergency download mode(timeout: %ds).\n", MOD, timeout / 1000);
    platform_safe_mode(1, timeout);

#if !CFG_FPGA_PLATFORM
    mtk_arch_reset(0); /* don't bypass power key */
#endif
    
    while(1);
}
#endif



int platform_get_mcp_id(u8 *id, u32 len, u32 *fw_id_len)
{
    int ret = -1;

    memset(id, 0, len);
    
#if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    ret = mmc_get_device_id(id, len,fw_id_len);
#else
    ret = nand_get_device_id(id, len);
#endif

    return ret;
}

void platform_set_chrg_cur(int ma)
{
    hw_set_cc(ma);
}

static boot_reason_t platform_boot_status(void)
{  
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
	ulong begin = get_timer(0);
	do  {
		if (rtc_boot_check()) {
			print("%s RTC boot!\n", MOD);
			return BR_RTC;
		}
		if(!kpoc_flag)
			break;
	} while (get_timer(begin) < 1000 && kpoc_flag);
#else
    if (rtc_boot_check()) {
        print("%s RTC boot!\n", MOD);
        return BR_RTC;
    }

#endif
    if (mtk_wdt_boot_check() == WDT_NORMAL_REBOOT) {
        print("%s WDT normal boot!\n", MOD);
        return BR_WDT;
    } else if (mtk_wdt_boot_check() == WDT_BY_PASS_PWK_REBOOT){
        print("%s WDT reboot bypass power key!\n", MOD);
        return BR_WDT_BY_PASS_PWK;
    }
#if !CFG_FPGA_PLATFORM
    /* check power key */
    if (mtk_detect_key(8) && hw_check_battery()) {
        print("%s Power key boot!\n", MOD);
        rtc_mark_bypass_pwrkey();
        return BR_POWER_KEY;
    }
#endif

#if !CFG_EVB_PLATFORM
    if (usb_accessory_in()) {
        print("%s USB/charger boot!\n", MOD);
        return BR_USB;
    }
    //need to unlock rtc PROT
    //check after rtc_boot_check() initial finish.
    if (rtc_2sec_reboot_check()) {
        print("%s 2sec reboot!\n", MOD);
        return BR_2SEC_REBOOT;
    }

    print("%s Unknown boot!\n", MOD);
    pl_power_off();
    /* should nerver be reached */
#endif
		
    print("%s Power key boot!\n", MOD);

    return BR_POWER_KEY;
}

#if CFG_LOAD_DSP_ROM || CFG_LOAD_MD_ROM
int platform_is_three_g(void)
{
    u32 tmp = sp_check_platform();

    return (tmp & 0x1) ? 0 : 1;
}
#endif

chip_ver_t platform_chip_ver(void)
{
    #if 0
    unsigned int hw_subcode = DRV_Reg32(APHW_SUBCODE);
    unsigned int sw_ver = DRV_Reg32(APSW_VER);
    #endif

    /* mt6589/mt6583 share the same hw_code/hw_sub_code/hw_version/sw_version currently */

    chip_ver_t ver = CHIP_6583_E1;

    return ver;
}

// ------------------------------------------------ 
// detect download mode
// ------------------------------------------------ 

bool platform_com_wait_forever_check(void)
{
#ifdef USBDL_DETECT_VIA_KEY
    /* check download key */
    if (TRUE == mtk_detect_key(COM_WAIT_KEY)) {
        print("%s COM handshake timeout force disable: Key\n", MOD);
        return TRUE;
    }
#endif

#ifdef USBDL_DETECT_VIA_AT_COMMAND
    print("platform_com_wait_forever_check\n");
    /* check SRAMROM_USBDL_TO_DIS */
    if (USBDL_TO_DIS == (INREG32(SRAMROM_USBDL_TO_DIS) & USBDL_TO_DIS)) {
	print("%s COM handshake timeout force disable: AT Cmd\n", MOD);	
	CLRREG32(SRAMROM_USBDL_TO_DIS, USBDL_TO_DIS);
	return TRUE;
    }
#endif

    return FALSE;
}

void platform_pre_init(void)
{
    u32 i2c_ret, pmic_ret;
    u32 pwrap_ret=0,i=0;

  
    /* init timer */
    mtk_timer_init();

    /* init boot time */
    g_boot_time = get_timer(0);

#if 0 /* FIXME */
    /*
     * NoteXXX: CPU 1 may not be reset clearly after power-ON.
     *          Need to apply a S/W workaround to manualy reset it first.
     */
    {
	U32 val;
	val = DRV_Reg32(0xC0009010);
	DRV_WriteReg32(0xC0009010, val | 0x2);
	gpt_busy_wait_us(10);
	DRV_WriteReg32(0xC0009010, val & ~0x2);
	gpt_busy_wait_us(10);
    }
#ifndef SLT_BOOTLOADER        
    /* power off cpu1 for power saving */
    power_off_cpu1();
#endif
#endif

#if !CFG_FPGA_PLATFORM
    /* init pll */
    mt_pll_init();
#endif

    /*GPIO init*/
    mt_gpio_init();

    /* init uart baudrate when pll on */
    mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);

    #if (CFG_USB_UART_SWITCH) 
	if (is_uart_cable_inserted()) {
		print("\n%s Switch to UART Mode\n", MOD);
		set_to_uart_mode();
        uart_mode = true;
	} else {
		print("\n%s Keep stay in USB Mode\n", MOD);
	}
    #endif

#if !CFG_FPGA_PLATFORM
    /* init pll post */
    mt_pll_post_init();
#endif

    //retry 3 times for pmic wrapper init
    pwrap_init_preloader();

#if !CFG_FPGA_PLATFORM

    pmic_ret = pmic_init();

    usbdl_wo_battery_forced(1); //add for DL wo battery 

//enable long press reboot function***************
#if !CFG_EVB_PLATFORM
#if KPD_PMIC_LPRST_TD!=0
	#if ONEKEY_REBOOT_NORMAL_MODE_PL
			pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
			pmic_config_interface(TOP_RST_MISC, 0x00, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
			pmic_config_interface(STRUP_CON3,0x01, PMIC_RG_FCHR_PU_EN_MASK, PMIC_RG_FCHR_PU_EN_SHIFT);//pull up homekey pin of PMIC for 82 project
			pmic_config_interface(TOP_RST_MISC, (U32)KPD_PMIC_LPRST_TD, PMIC_RG_PWRKEY_RST_TD_MASK, PMIC_RG_PWRKEY_RST_TD_SHIFT);
	#else
			pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
			pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
			pmic_config_interface(STRUP_CON3,0x01, PMIC_RG_FCHR_PU_EN_MASK, PMIC_RG_FCHR_PU_EN_SHIFT);//pull up homekey pin of PMIC for 82 project
			pmic_config_interface(TOP_RST_MISC, (U32)KPD_PMIC_LPRST_TD, PMIC_RG_PWRKEY_RST_TD_MASK, PMIC_RG_PWRKEY_RST_TD_SHIFT);
	#endif
#else
			pmic_config_interface(TOP_RST_MISC, 0x00, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
			pmic_config_interface(TOP_RST_MISC, 0x00, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
			pmic_config_interface(STRUP_CON3,0x01, PMIC_RG_FCHR_PU_EN_MASK, PMIC_RG_FCHR_PU_EN_SHIFT);//pull up homekey pin of PMIC for 82 project
#endif
#endif
#endif
//************************************************

    print("%s Init I2C: %s(%d)\n", MOD, i2c_ret ? "FAIL" : "OK", i2c_ret);   
    print("%s Init PWRAP: %s(%d)\n", MOD, pwrap_ret ? "FAIL" : "OK", pwrap_ret);
    print("%s Init PMIC: %s(%d)\n", MOD, pmic_ret ? "FAIL" : "OK", pmic_ret);

    print("%s chip[%x]\n", MOD, platform_chip_ver());
}


#ifdef MTK_MT8193_SUPPORT
extern int mt8193_init(void);
#endif

int g_io_ring, g_core_ring;

void readRings(int *io_ring, int* core_ring)
{
    *io_ring = g_io_ring;
    *core_ring = g_core_ring;
    return;
}

#define DDRPHY_BASE_OSC (DDRPHY_BASE + (0x79 << 2))
#define DRAMC0_BASE_OSC (DRAMC0_BASE + (0x79 << 2))
#define DRAMC_NAO_BASE_OSC (DRAMC_NAO_BASE + (0xEF << 2))
void readIORingInit(void)
{
    /* Read IO ring osc setting */
    unsigned int io_ring = 0;
    unsigned int core_ring = 0;
    unsigned int infra_globalcon_dcmctl_restore = DRV_Reg32(INFRA_GLOBALCON_DCMCTL);
    unsigned int ddrphy_base_0x79ls2_restore = DRV_Reg32(DDRPHY_BASE_OSC);
    unsigned int dramc0_base_0x79ls2_restore = DRV_Reg32(DRAMC0_BASE_OSC);
    unsigned int infra_globalcon_dcmctl_val = 0;
    unsigned int ddrphy_base_0x79ls2_val = 0;
    unsigned int dramc0_base_0x79ls2_val = 0;

    print("[%s]infra_globalcon_dcmctl_restore = 0x%x\n", __func__, infra_globalcon_dcmctl_restore);
    print("[%s]ddrphy_base_0x79ls2_restore = 0x%x\n", __func__, ddrphy_base_0x79ls2_restore);
    print("[%s]dramc0_base_0x79ls2_restore = 0x%x\n", __func__, dramc0_base_0x79ls2_restore);

    infra_globalcon_dcmctl_val = DRV_Reg32(INFRA_GLOBALCON_DCMCTL);
    DRV_WriteReg32(INFRA_GLOBALCON_DCMCTL, (infra_globalcon_dcmctl_val & ~(0x00000002)));  // Disable hf_fmem_ck DCM

    ddrphy_base_0x79ls2_val = DRV_Reg32(DDRPHY_BASE_OSC);
    DRV_WriteReg32(DDRPHY_BASE_OSC, (ddrphy_base_0x79ls2_val  | 0x2000));  // [13] Enable Ring

    ddrphy_base_0x79ls2_val = DRV_Reg32(DDRPHY_BASE_OSC);
    DRV_WriteReg32(DDRPHY_BASE_OSC, (ddrphy_base_0x79ls2_val  | 0x10000000));  // [28] Enable RDSEL0
    ddrphy_base_0x79ls2_val = DRV_Reg32(DDRPHY_BASE_OSC);
    DRV_WriteReg32(DDRPHY_BASE_OSC, (ddrphy_base_0x79ls2_val  | 0x20000000));  // [29] Enable RDSEL1

    // Exterd Meter Resultion
    ddrphy_base_0x79ls2_val = DRV_Reg32(DDRPHY_BASE_OSC);
    DRV_WriteReg32(DDRPHY_BASE_OSC, (ddrphy_base_0x79ls2_val  | 0x0F000000));  // [27:24] Enable RDSEL1

    // Disable Meter
    dramc0_base_0x79ls2_val = DRV_Reg32(DRAMC0_BASE_OSC);
    DRV_WriteReg32(DRAMC0_BASE_OSC, (dramc0_base_0x79ls2_val  & ~(0x1000)));  // [12] Enable
    // Reset Counter
    dramc0_base_0x79ls2_val = DRV_Reg32(DRAMC0_BASE_OSC);
    DRV_WriteReg32(DRAMC0_BASE_OSC, (dramc0_base_0x79ls2_val  | 0x4000));  // [14] Counter RESET
    dramc0_base_0x79ls2_val = DRV_Reg32(DRAMC0_BASE_OSC);
    DRV_WriteReg32(DRAMC0_BASE_OSC, (dramc0_base_0x79ls2_val  & ~(0x4000)));  // [14] Counter RESET

    // Enable Meter
    dramc0_base_0x79ls2_val = DRV_Reg32(DRAMC0_BASE_OSC);
    DRV_WriteReg32(DRAMC0_BASE_OSC, (dramc0_base_0x79ls2_val  | 0x1000));  // [12] Enable

    mdelay(1);

    // IO_RING
    io_ring = DRV_Reg32(DRAMC_NAO_BASE_OSC) & 0xFFFF0000;
    io_ring = io_ring >> 16;
    g_io_ring = io_ring;
    // CORE_RING
    core_ring = DRV_Reg32(DRAMC_NAO_BASE_OSC) & 0x0000FFFF;
    g_core_ring = core_ring;

    print("OSC: IO RING Counter: 0x%x\n", io_ring);
    print("OSC: CORE RING Counter: 0x%x\n", core_ring);

    // RESTORE
    DRV_WriteReg32(INFRA_GLOBALCON_DCMCTL, infra_globalcon_dcmctl_restore);
    DRV_WriteReg32(DDRPHY_BASE_OSC, ddrphy_base_0x79ls2_restore);
    DRV_WriteReg32(DRAMC0_BASE_OSC, dramc0_base_0x79ls2_restore);

    return;
}

void platform_init(void)
{
    u32 ret, tmp;
    boot_reason_t reason;
	
    /* check DDR-reserve mode */
    check_ddr_reserve_status();
	
    /* init watch dog, will enable AP watch dog */
    mtk_wdt_init();
    /*init kpd PMIC mode support*/
    set_kpd_pmic_mode();


#if CFG_MDWDT_DISABLE 
    /* no need to disable MD WDT, the code here is for backup reason */

    /* disable MD0 watch dog. */
    DRV_WriteReg32(0x20050000, 0x2200); 

    /* disable MD1 watch dog. */
    DRV_WriteReg32(0x30050020, 0x2200); 
#endif

//ALPS00427972, implement the analog register formula
    //Set the calibration after power on
    //Add here for eFuse, chip version checking -> analog register calibration
    mt_usb_calibraion();
//ALPS00427972, implement the analog register formula

    /* make usb11 phy enter savecurrent mode */
    mt_usb11_phy_savecurrent();

#if 1 /* FIXME */
    g_boot_reason = reason = platform_boot_status();

    if (reason == BR_RTC || reason == BR_POWER_KEY || reason == BR_USB || reason == BR_WDT || reason == BR_WDT_BY_PASS_PWK || reason == BR_2SEC_REBOOT)
        rtc_bbpu_power_on();
#else

    g_boot_reason = BR_POWER_KEY;

#endif

    enable_PMIC_kpd_clock();

    /* init io_ring / core_ring */
    readIORingInit();
      
    /* init memory */
    mt_mem_init();
    /*init dram buffer*/
    init_dram_buffer();
    /* switch log buffer to dram */
    log_buf_ctrl(1);
#if 0 /* FIXME */
    /* enable CA9 share bits for USB(30)/NFI(29)/MSDC(28) modules to access ISRAM */
    tmp = DRV_Reg32(0xC1000200);
    tmp |= ((1<<30)|(1<<29)|(1<<28));
    DRV_WriteReg32 (0xC1000200, tmp);
#endif

#ifdef MTK_MT8193_SUPPORT   
	mt8193_init();
#endif

    /* init device storeage */
    ret = boot_device_init();
    print("%s Init Boot Device: %s(%d)\n", MOD, ret ? "FAIL" : "OK", ret);

#if CFG_EMERGENCY_DL_SUPPORT
    /* check if to enter emergency download mode */
    /* Move after dram_inital and boot_device_init. 
      Use excetution time to remove delay time in mtk_kpd_gpio_set()*/
    if (mtk_detect_dl_keys()) {
        platform_emergency_download(CFG_EMERGENCY_DL_TIMEOUT_MS);
    }
#endif

#if CFG_REBOOT_TEST
    mtk_wdt_sw_reset();
    while(1);
#endif

}

void platform_post_init(void)
{
    struct ram_console_buffer *ram_console;
    int io_ring = 0;
    int core_ring = 0;

    usbdl_wo_battery_forced(0);  //add for DL wo battery
    
    readRings(&io_ring, &core_ring); // Get osc setting
    set_io_ring_info((((io_ring & 0x0000FFFF) << 16) | (core_ring & 0x0000FFFF)));       // Set osc setting to TOPRGU register

#if CFG_BATTERY_DETECT
    /* normal boot to check battery exists or not */
    if (g_boot_mode == NORMAL_BOOT && !hw_check_battery() && usb_accessory_in()) {
        print("%s Wait for battery inserted...\n", MOD);
        /* disable pmic pre-charging led */
        pl_close_pre_chr_led();
        /* enable force charging mode */
        pl_charging(1);
        do {
            mdelay(300);
            /* check battery exists or not */
            if (hw_check_battery())
                break;
            /* kick all watchdogs */
            platform_wdt_all_kick();
        } while(1);
        /* disable force charging mode */
        pl_charging(0);
    }
#endif


#if CFG_MDJTAG_SWITCH 
    unsigned int md_pwr_con;

    /* md0 default power on and clock on */
    /* md1 default power on and clock off */

    /* ungate md1 */
    /* rst_b = 0 */
    md_pwr_con = DRV_Reg32(0x10006280);
    md_pwr_con &= ~0x1;
    DRV_WriteReg32(0x10006280, md_pwr_con);

    /* enable clksq2 for md1 */
    DRV_WriteReg32(0x10209000, 0x00001137);
    udelay(200);
    DRV_WriteReg32(0x10209000, 0x0000113f);

    /* rst_b = 1 */
    md_pwr_con = DRV_Reg32(0x10006280);
    md_pwr_con |= 0x1;
    DRV_WriteReg32(0x10006280, md_pwr_con);

    /* switch to MD legacy JTAG */
    /* this step is not essentially required */
#endif

#if CFG_MDMETA_DETECT 
    if (g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT) {
	/* trigger md0 to enter meta mode */
        DRV_WriteReg32(0x20000010, 0x1);
	/* trigger md1 to enter meta mode */
        DRV_WriteReg32(0x30000010, 0x1);
    } else {
	/* md0 does not enter meta mode */
        DRV_WriteReg32(0x20000010, 0x0);
	/* md1 does not enter meta mode */
        DRV_WriteReg32(0x30000010, 0x0);
    }
#endif

#if CFG_RAM_CONSOLE
    ram_console = (struct ram_console_buffer *)RAM_CONSOLE_ADDR;

    if (ram_console->sig == RAM_CONSOLE_SIG) {
        print("%s ram_console->start=0x%x\n", MOD, ram_console->start);
        if (ram_console->start > RAM_CONSOLE_MAX_SIZE)
            ram_console->start = 0;

		ram_console->hw_status = g_rgu_status;

        print("%s ram_console(0x%x)=0x%x (boot reason)\n", MOD, 
            ram_console->hw_status, g_rgu_status);
		    }
#endif

    platform_set_boot_args();
}

void platform_error_handler(void)
{
    /* if log is disabled, re-init log port and enable it */
    if (log_status() == 0) {
        mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);
        log_ctrl(1);
    }
    print("%s preloader fatal error...\n", MOD);
    sec_util_brom_download_recovery_check(); 
    /* enter emergency download mode */

    #if CFG_EMERGENCY_DL_SUPPORT
    platform_emergency_download(CFG_EMERGENCY_DL_TIMEOUT_MS);
    #endif

    while(1);
}

void platform_assert(char *file, int line, char *expr)
{   
    print("<ASSERT> %s:line %d %s\n", file, line, expr); 
    platform_error_handler();
}

