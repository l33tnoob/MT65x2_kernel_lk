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

#include "platform.h"
#include "usbphy.h"
#include "usbd.h"

#if CFG_FPGA_PLATFORM
#include "i2c.h"

static struct mt_i2c_t usb_i2c;

U32 usb_i2c_write8(U8 addr, U8 value)
{
        U32 ret_code = I2C_OK;
        U8 write_data[2];
        U16 len;

        write_data[0]= addr;
        write_data[1] = value;

        usb_i2c.id = USB_I2C_ID;
		/* Since i2c will left shift 1 bit, we need to set USB I2C address to 0x60 (0xC0>>1) */
        usb_i2c.addr = 0x60;
        usb_i2c.mode = ST_MODE;
        usb_i2c.speed = 100;
        len = 2;

        ret_code = i2c_write(&usb_i2c, write_data, len);
        printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

        return ret_code;
}

U32 usb_i2c_read8 (U8 addr, U8 *dataBuffer) {
        U32 ret_code = I2C_OK;
        U16 len;
		*dataBuffer = addr;

        usb_i2c.id = USB_I2C_ID;
		/* Since i2c will left shift 1 bit, we need to set USB I2C address to 0x60 (0xC0>>1) */
        usb_i2c.addr = 0x60;
        usb_i2c.mode = ST_MODE;
        usb_i2c.speed = 100;
        len = 1;

        ret_code = i2c_write_read(&usb_i2c, dataBuffer, len, len);
        printf("%s: i2c_read: ret_code: %d\n", __func__, ret_code);

        return ret_code;
}
#endif

#if CFG_FPGA_PLATFORM
void mt_usb_phy_poweron (void)
{

	#define PHY_DRIVING   0x3

	UINT8 usbreg8;
	unsigned int i;

	/* force_suspendm = 0 */
	USBPHY_CLR8(0x6a, 0x04);

	USBPHY_I2C_WRITE8(0xff, 0x00);
	USBPHY_I2C_WRITE8(0x61, 0x04);
	USBPHY_I2C_WRITE8(0x68, 0x00);
	USBPHY_I2C_WRITE8(0x6a, 0x00);
	USBPHY_I2C_WRITE8(0x00, 0x6e);
	USBPHY_I2C_WRITE8(0x1b, 0x0c);
	USBPHY_I2C_WRITE8(0x08, 0x44);
	USBPHY_I2C_WRITE8(0x11, 0x55);
	USBPHY_I2C_WRITE8(0x1a, 0x68);

	#if defined(USB_PHY_DRIVING_TUNING)
	/* driving tuning */
	USBPHY_I2C_READ8(0xab, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xab, usbreg8);

	for(i = 0; i < 16; i++)
	{
		USBPHY_I2C_READ8((0x92+i), &usbreg8);
		usbreg8 &= ~0x3;
		usbreg8 |= PHY_DRIVING;
		USBPHY_I2C_WRITE8((0x92+i), usbreg8);
	}

	USBPHY_I2C_READ8(0xbc, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xbc, usbreg8);

	USBPHY_I2C_READ8(0xbe, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xbe, usbreg8);

	USBPHY_I2C_READ8(0xbf, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xbf, usbreg8);

	USBPHY_I2C_READ8(0xcd, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xcd, usbreg8);

	USBPHY_I2C_READ8(0xf1, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xf1, usbreg8);

	USBPHY_I2C_READ8(0xa7, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xa7, usbreg8);

	USBPHY_I2C_READ8(0xa8, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xa8, usbreg8);
	#endif

	udelay(800);
}
void mt_usb_phy_savecurrent (void)
{
	/* no need */
}
void mt_usb_phy_recover (void)
{
	/* no need */
}
void mt_usb11_phy_savecurrent(void)
{
	/* no need */
}
void mt_usb_calibraion ()
{
	/* no need */
}

void Charger_Detect_Init(void)
{
	/* no need */
}

void Charger_Detect_Release(void)
{
	/* no need */
}	

#else
void mt_usb_phy_poweron (void)
{
	/*
	 * swtich to USB function.
	 * (system register, force ip into usb mode).
	 */
	USBPHY_CLR8(0x6b, 0x04);
	USBPHY_CLR8(0x6e, 0x01);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);

	/* RG_USB20_DP_100K_EN = 1'b0 */
	/* RG_USB20_DM_100K_EN = 1'b0 */
	USBPHY_CLR8(0x22, 0x03);

	/* release force suspendm */
	USBPHY_CLR8(0x6a, 0x04);

	udelay(800);

    /* force enter device mode */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2E);
	USBPHY_SET8(0x6d, 0x3E);
}

void mt_usb_phy_savecurrent (void)
{
	/*
	 * swtich to USB function.
	 * (system register, force ip into usb mode).
	 */
	USBPHY_CLR8(0x6b, 0x04);
	USBPHY_CLR8(0x6e, 0x01);

	/* release force suspendm */
	USBPHY_CLR8(0x6a, 0x04);
	/* RG_DPPULLDOWN./RG_DMPULLDOWN. */
	USBPHY_SET8(0x68, 0xc0);
	/* RG_XCVRSEL[1:0] = 2'b01 */
	USBPHY_CLR8(0x68, 0x30);
	USBPHY_SET8(0x68, 0x10);
	/* RG_TERMSEL = 1'b1 */
	USBPHY_SET8(0x68, 0x04);
	/* RG_DATAIN[3:0] = 4'b0000 */
	USBPHY_CLR8(0x69, 0x3c);

	/*
	 * force_dp_pulldown, force_dm_pulldown,
	 * force_xcversel, force_termsel.
	 */
	USBPHY_SET8(0x6a, 0xba);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	/* RG_USB20_OTG_VBUSSCMP_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x10);

	udelay(800);

	/* rg_usb20_pll_stable = 1 */
	USBPHY_SET8(0x63, 0x02);

	udelay(1);

	/* force suspendm = 1 */
	USBPHY_SET8(0x6a, 0x04);

	udelay(1);
}

void mt_usb_phy_recover (void)
{
	/* clean PUPD_BIST_EN */
	/* PUPD_BIST_EN = 1'b0 */
	/* PMIC will use it to detect charger type */
	USBPHY_CLR8(0x1d, 0x10);

	/* force_uart_en = 1'b0 */
	USBPHY_CLR8(0x6b, 0x04);
	/* RG_UART_EN = 1'b0 */
	USBPHY_CLR8(0x6e, 0x01);
	/* force_uart_en = 1'b0 */
	USBPHY_CLR8(0x6a, 0x04);

	USBPHY_CLR8(0x68, 0xf4);

	/* RG_DATAIN[3:0] = 4'b0000 */
	USBPHY_CLR8(0x69, 0x3c);

	USBPHY_CLR8(0x6a, 0xba);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	/* RG_USB20_OTG_VBUSSCMP_EN = 1'b1 */
	USBPHY_SET8(0x1a, 0x10);

	udelay(800);

	/* force enter device mode */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2E);
	USBPHY_SET8(0x6d, 0x3E);

	print("USB HW reg: index14=0x%x\n", seclib_get_devinfo_with_index(14));
	if (seclib_get_devinfo_with_index(14) & (0x01<<22))
	{
		print("USB HW reg: read RG_USB20_INTR_EN 0x%x\n", USBPHY_READ8(0x00));
		USBPHY_CLR8(0x00, 0x20);
		print("USB HW reg: write RG_USB20_INTR_EN 0x%x\n", USBPHY_READ8(0x00));
	}
	
	if (seclib_get_devinfo_with_index(14) & (0x07<<19))
	{
		//RG_USB20_VRT_VREF_SEL[2:0]=5 (ori:4) (0x11110804[14:12])
		print("USB HW reg: read RG_USB20_VRT_VREF_SEL 0x%x\n", USBPHY_READ8(0x05));
		USBPHY_CLR8(0x05, 0x70);
  		USBPHY_SET8(0x05, ((seclib_get_devinfo_with_index(7)>>19)<<4)&0x70);
  		print("USB HW reg: overwrite RG_USB20_VRT_VREF_SEL 0x%x\n", USBPHY_READ8(0x05));
	}
}

//ALPS00427972, implement the analog register formula
void mt_usb_calibraion ()
{
#if 0
    //Set the calibration after power on
    //Add here for eFuse, chip version checking -> analog register calibration
    int input_reg = INREG16(M_HW_RES3);
    print("%s: input_reg = 0x%x \n", __func__, input_reg);
    int term_vref 	= (input_reg & RG_USB20_TERM_VREF_SEL_MASK) >> 13;     //0xE000      //0b 1110,0000,0000,0000     15~13
    int clkref 		= (input_reg & RG_USB20_CLKREF_REF_MASK)    >> 10;     //0x1C00      //0b 0001,1100,0000,0000     12~10
    int vrt_vref	= (input_reg & RG_USB20_VRT_VREF_SEL_MASK)  >> 7;      //0x0380      //0b 0000,0011,1000,0000     9~7

    print("%s: term_vref = 0x%x,  clkref = 0x%x, vrt_vref = 0x%x,\n", __func__, term_vref, clkref, vrt_vref);

    if(term_vref)
            mt_usb_phy_calibraion(1, term_vref);
    if(clkref)
            mt_usb_phy_calibraion(2, clkref);
    if(vrt_vref)
            mt_usb_phy_calibraion(3, vrt_vref);
#endif
}

void mt_usb_phy_calibraion (int case_set, int input_reg)
{
#if 0
    int temp_added=0;
    int temp_test=0;
    int temp_mask;

	print("%s: case_set %d, input_reg = 0x%x \n", __func__, case_set, input_reg);

    switch(case_set)
    {
    case 1:
        //case  1
        //If M_HW_RES3[15:13] !=0
            //RG_USB20_TERM_VREF_SEL[2:0] <= RG_USB20_TERM_VREF_SEL[2:0] + M_HW_RES3[15:13]
        temp_mask = 0x07;
        temp_test = USBPHY_READ8(0x05);
		print("%s: temp_test = 0x%x \n", __func__, temp_test);
        temp_added = (USBPHY_READ8(0x05)& temp_mask) + input_reg;
		print("%s: temp_added = 0x%x \n", __func__, temp_added);
        temp_added &= 0x07;
		print("%s: temp_added = 0x%x \n", __func__, temp_added);

        USBPHY_CLR8(0x05, temp_mask);
        USBPHY_SET8(0x05, temp_added);

        temp_test = USBPHY_READ8(0x05);
        print("%s: final temp_test = 0x%x \n", __func__, temp_test);
        break;
    case 2:
        //case 2
        //If M_HW_RES3[12:10] !=0
            //RG_USB20_CLKREF_REF[2:0]<= RG_USB20_CLKREF_REF[2:0]+ M_HW_RES3[12:10]
        temp_mask = 0x07;

        temp_test = USBPHY_READ8(0x07);
		print("%s: temp_test = 0x%x \n", __func__, temp_test);
        temp_added = (USBPHY_READ8(0x07)& temp_mask) + input_reg;
		print("%s: temp_added = 0x%x \n", __func__, temp_added);
        temp_added &= 0x07;
		print("%s: temp_added = 0x%x \n", __func__, temp_added);

        USBPHY_CLR8(0x07, temp_mask);
        USBPHY_SET8(0x07, temp_added);

        temp_test = USBPHY_READ8(0x07);
        print("%s: final temp_test = 0x%x \n", __func__, temp_test);
        break;
    case 3:
        //case 3
        //If M_HW_RES3[9:7] !=0
            //RG_USB20_VRT_VREF_SEL[2:0]<=RG_USB20_VRT_VREF_SEL[2:0]+ M_HW_RES3[9:7]
        temp_mask = 0x70;

        temp_test = USBPHY_READ8(0x05);
		print("%s: temp_test = 0x%x \n", __func__, temp_test);
        temp_added = (USBPHY_READ8(0x05)& temp_mask) >> 4;
		print("%s: temp_added = 0x%x \n", __func__, temp_added);
        temp_added += input_reg;
		print("%s: temp_added = 0x%x \n", __func__, temp_added);
        temp_added &= 0x07;
		print("%s: temp_added = 0x%x \n", __func__, temp_added);

        USBPHY_CLR8(0x05, temp_mask);
        USBPHY_SET8(0x05, temp_added<<4);

        temp_test = USBPHY_READ8(0x05);
        print("%s: final temp_test = 0x%x \n", __func__, temp_test);
        break;
    }

#endif
}


void mt_usb11_phy_savecurrent(void)
{
/* 82 only have USB port0 */
#if 0
    //4 1. swtich to USB function. (system register, force ip into usb mode.
    USB11PHY_CLR8(0x6b, 0x04);
    USB11PHY_CLR8(0x6e, 0x01);

    //4 2. release force suspendm.
    USB11PHY_CLR8(0x6a, 0x04);
    //4 3. RG_DPPULLDOWN./RG_DMPULLDOWN.
    USB11PHY_SET8(0x68, 0xc0);
    //4 4. RG_XCVRSEL[1:0] =2'b01.
    USB11PHY_CLR8(0x68, 0x30);
    USB11PHY_SET8(0x68, 0x10);
    //4 5. RG_TERMSEL = 1'b1
    USB11PHY_SET8(0x68, 0x04);
    //4 6. RG_DATAIN[3:0]=4'b0000
    USB11PHY_CLR8(0x69, 0x3c);
    //4 7.force_dp_pulldown, force_dm_pulldown, force_xcversel,force_termsel.
    USB11PHY_SET8(0x6a, 0xba);

    //4 8.RG_USB20_BC11_SW_EN 1'b0
    USB11PHY_CLR8(0x1a, 0x80);
    //4 9.RG_USB20_OTG_VBUSSCMP_EN 1'b0
    USB11PHY_CLR8(0x1a, 0x10);
    //4 10. delay 800us.
    udelay(800);
    //4 11. rg_usb20_pll_stable = 1
    USB11PHY_SET8(0x63, 0x02);

    udelay(1);
    //4 12.  force suspendm = 1.
    USB11PHY_SET8(0x6a, 0x04);

    USB11PHY_CLR8(0x6C, 0x2C);
    USB11PHY_SET8(0x6C, 0x10);
    USB11PHY_CLR8(0x6D, 0x3C);

    //4 13.  wait 1us
    udelay(1);

#endif
}

void Charger_Detect_Init(void)
{
	/* RG_USB20_BC11_SW_EN = 1'b1 */
	USBPHY_SET8(0x1a, 0x80);
}

void Charger_Detect_Release(void)
{
	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
}

#endif

