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

#ifndef USBPHY_H_
#define USBPHY_H_

#if CFG_FPGA_PLATFORM
#include "i2c.h"
#endif

#define USB20_PHY_BASE					(USBSIF_BASE + 0x0800)
#define USB11_PHY_BASE					(USBSIF_BASE + 0x0900)
#define PERI_GLOBALCON_PDN0_SET 		(PERICFG_BASE+0x008)
#define USB0_PDN						1<<10

#if CFG_FPGA_PLATFORM
#define USB_I2C_ID	I2C2	/* 0 - 6 */
#define PATH_NORMAL	0
#define PATH_PMIC	1

extern U32 usb_i2c_read8 (U8 addr, U8 *dataBuffer);
extern U32 usb_i2c_write8 (U8 addr, U8 value);

#define USBPHY_I2C_READ8(addr, buffer)	 usb_i2c_read8(addr, buffer)
#define USBPHY_I2C_WRITE8(addr, value)	 usb_i2c_write8(addr, value)
#endif

#define USBPHY_READ8(offset)		__raw_readb(USB20_PHY_BASE+offset)
#define USBPHY_WRITE8(offset, value)	__raw_writeb(value, USB20_PHY_BASE+offset)
#define USBPHY_SET8(offset, mask)	USBPHY_WRITE8(offset, USBPHY_READ8(offset) | mask)
#define USBPHY_CLR8(offset, mask)	USBPHY_WRITE8(offset, USBPHY_READ8(offset) & ~mask)

#define USB_SET_BIT(BS,REG)			mt65xx_reg_sync_writel((__raw_readl(REG) | (U32)(BS)), (REG))
#define USB_CLR_BIT(BS,REG)			mt65xx_reg_sync_writel((__raw_readl(REG) & (~(U32)(BS))), (REG))

#define USB11PHY_READ8(offset)		__raw_readb(USB11_PHY_BASE+offset)
#define USB11PHY_WRITE8(offset, value)	__raw_writeb(value, USB11_PHY_BASE+offset)
#define USB11PHY_SET8(offset, mask)	USB11PHY_WRITE8(offset, USB11PHY_READ8(offset) | mask)
#define USB11PHY_CLR8(offset, mask)	USB11PHY_WRITE8(offset, USB11PHY_READ8(offset) & ~mask)

#endif

