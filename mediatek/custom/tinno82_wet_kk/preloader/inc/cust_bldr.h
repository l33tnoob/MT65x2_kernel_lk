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

#ifndef CUST_BLDR_H
#define CUST_BLDR_H

#include "boot_device.h"
#include "uart.h"

/* cust_bldr.h is not used any more, please refer to cust_bldr.mak */

#if 0
/*=======================================================================*/
/* Pre-Loader Features                                                   */
/*=======================================================================*/
#define CFG_FPGA_PLATFORM           (1)

#if CFG_FPGA_PLATFORM
#define CFG_BOOT_DEV                (BOOTDEV_SDMMC)
#else
#ifdef MTK_EMMC_SUPPORT
#define CFG_BOOT_DEV                (BOOTDEV_SDMMC)
#else
#define CFG_BOOT_DEV                (BOOTDEV_NAND)
#endif
#endif

#define CFG_BATTERY_DETECT          (0)

#define CFG_UART_TOOL_HANDSHAKE     (1)
#define CFG_USB_TOOL_HANDSHAKE      (1)
#define CFG_USB_DOWNLOAD            (1)
#define CFG_PMT_SUPPORT             (0)

#define CFG_LOG_BAUDRATE            (921600)
#define CFG_META_BAUDRATE           (115200)
#define CFG_UART_LOG                (UART1)
#define CFG_UART_META               (UART1)

#define CFG_EMERGENCY_DL_SUPPORT    (0)
#define CFG_EMERGENCY_DL_TIMEOUT_MS (1000 * 60 * 5) /* 5 mins */

#define CFG_SWITCH_USB_UART_SUPPORT (0)
#define CFG_SWITCH_FORCE_USB        (0) // only active when CFG_SWITCH_USB_UART_SUPPORT is (1)
#define CFG_SWITCH_FORCE_UART       (0) // only active when CFG_SWITCH_USB_UART_SUPPORT is (1)

/*=======================================================================*/
/* Misc Options                                                          */	
/*=======================================================================*/
#define FEATURE_MMC_ADDR_TRANS
#endif

#endif /* CUST_BLDR_H */
