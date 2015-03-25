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

#ifndef __MT_RTC_H__
#define __MT_RTC_H__

#include <typedefs.h>
#define RTC_CALI_BBPU_2SEC_EN				(1U << 8)
#define RTC_CALI_BBPU_2SEC_MODE_SHIFT		9
#define RTC_CALI_BBPU_2SEC_MODE_MSK			(3U << RTC_CALI_BBPU_2SEC_MODE_SHIFT)
#define RTC_CALI_BBPU_2SEC_STAT	(1U << 11)

#define RTC_FQMTR_LOW_BASE	(794 - 2)
#define RTC_FQMTR_HIGH_BASE (794 + 2)

#define FQMTR_FIX_CLK_26M							((0 << PMIC_RG_FQMTR_CKSEL_SHIFT) | ~(PMIC_RG_FQMTR_CKSEL_MASK << PMIC_RG_FQMTR_CKSEL_SHIFT))
#define FQMTR_FIX_CLK_XOSC_32K_DET		(2 << PMIC_RG_FQMTR_CKSEL_SHIFT)
#define FQMTR_FIX_CLK_EOSC_32K				(3 << PMIC_RG_FQMTR_CKSEL_SHIFT)
#define RG_FQMTR_RST						(1U << PMIC_RG_FQMTR_RST_SHIFT)
#define RG_FQMTR_PDN            (1U << PMIC_RG_FQMTR_PDN_SHIFT)
#define FQMTR_EN						(1U << PMIC_FQMTR_EN_SHIFT)
#define FQMTR_BUSY					(1U << PMIC_FQMTR_BUSY_SHIFT)
#define FQMTR_TCKSEL_MSK		PMIC_FQMTR_TCKSEL_MASK
#define FQMTR_XOSC32_CK			0
#define	FQMTR_DCXO_F32K_CK 	1
#define	FQMTR_EOSC32_CK		 	2
#define	FQMTR_XOSC32_CK_DETECTON 	3
#define	FQMTR_AUD26M_CK 	4
#define FQMTR_WINSET    		0x0000
#define CLKSQ_EN						(1U << PMIC_RG_CLKSQ_EN_SHIFT)

#define RTC_HW_DET_BYPASS				(8 << PMIC_RTC_XTAL_DET_RSV_SHIFT)
#define	RTC_HW_XOSC_MODE				(~(2 << PMIC_RTC_XTAL_DET_RSV_SHIFT))
#define	RTC_HW_DCXO_MODE				(2 << PMIC_RTC_XTAL_DET_RSV_SHIFT)

/* we map HW YEA 0 (2000) to 1968 not 1970 because 2000 is the leap year */
#define RTC_MIN_YEAR            1968
#define RTC_NUM_YEARS           128
//#define RTC_MAX_YEAR          (RTC_MIN_YEAR + RTC_NUM_YEARS - 1)

extern void rtc_bbpu_power_on(void);

extern void rtc_mark_bypass_pwrkey(void);

extern U16 rtc_rdwr_uart_bits(U16 *val);

extern bool rtc_boot_check(void);

extern void pl_power_off(void);

extern bool rtc_2sec_reboot_check(void);

#endif /* __MT_RTC_H__ */
