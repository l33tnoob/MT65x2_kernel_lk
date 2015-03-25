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
#include <cust_rtc.h>

//#include <platform.h>
/*
#define TOP_CKPDN		0x004E
#define TOP_CKTST2      0x012E
#define FQMTR_CON0      0x0188
#define FQMTR_CON1      0x018A
#define FQMTR_CON2      0x018C
*/
/* RTC registers */
#define RTC_BASE	(0x8000)
#define RTC_BBPU	(RTC_BASE + 0x0000)
#define RTC_IRQ_STA	(RTC_BASE + 0x0002)
#define RTC_IRQ_EN	(RTC_BASE + 0x0004)
#define RTC_CII_EN	(RTC_BASE + 0x0006)
#define RTC_AL_MASK	(RTC_BASE + 0x0008)
#define RTC_TC_SEC	(RTC_BASE + 0x000a)
#define RTC_TC_MIN	(RTC_BASE + 0x000c)
#define RTC_TC_HOU	(RTC_BASE + 0x000e)
#define RTC_TC_DOM	(RTC_BASE + 0x0010)
#define RTC_TC_DOW	(RTC_BASE + 0x0012)
#define RTC_TC_MTH	(RTC_BASE + 0x0014)
#define RTC_TC_YEA	(RTC_BASE + 0x0016)
#define RTC_AL_SEC	(RTC_BASE + 0x0018)
#define RTC_AL_MIN	(RTC_BASE + 0x001a)
#define RTC_AL_HOU	(RTC_BASE + 0x001c)
#define RTC_AL_DOM	(RTC_BASE + 0x001e)
#define RTC_AL_DOW	(RTC_BASE + 0x0020)
#define RTC_AL_MTH	(RTC_BASE + 0x0022)
#define RTC_AL_YEA	(RTC_BASE + 0x0024)
#define RTC_OSC32CON	(RTC_BASE + 0x0026)
#define RTC_POWERKEY1	(RTC_BASE + 0x0028)
#define RTC_POWERKEY2	(RTC_BASE + 0x002a)
#define RTC_PDN1	(RTC_BASE + 0x002c)
#define RTC_PDN2	(RTC_BASE + 0x002e)
#define RTC_SPAR0	(RTC_BASE + 0x0030)
#define RTC_SPAR1	(RTC_BASE + 0x0032)
#define RTC_PROT	(RTC_BASE + 0x0036)
#define RTC_DIFF	(RTC_BASE + 0x0038)
#define RTC_CALI	(RTC_BASE + 0x003a)
#define RTC_WRTGR	(RTC_BASE + 0x003c)
#define RTC_CON		(RTC_BASE + 0x003e)

#define RTC_BBPU_PWREN          (1U << 0)   /* BBPU = 1 when alarm occurs */
#define RTC_BBPU_BBPU           (1U << 2)   /* 1: power on, 0: power down */
#define RTC_BBPU_AUTO           (1U << 3)   /* BBPU = 0 when xreset_rstb goes low */
#define RTC_BBPU_CLRPKY         (1U << 4)
#define RTC_BBPU_RELOAD         (1U << 5)
#define RTC_BBPU_CBUSY          (1U << 6)
#define RTC_BBPU_KEY            (0x43 << 8)

#define RTC_IRQ_STA_AL          (1U << 0)
#define RTC_IRQ_STA_TC          (1U << 1)
#define RTC_IRQ_STA_LP          (1U << 3)

#define RTC_IRQ_EN_AL           (1U << 0)
#define RTC_IRQ_EN_TC           (1U << 1)
#define RTC_IRQ_EN_ONESHOT      (1U << 2)
#define RTC_IRQ_EN_LP           (1U << 3)
#define RTC_IRQ_EN_ONESHOT_AL   (RTC_IRQ_EN_ONESHOT | RTC_IRQ_EN_AL)

#define RTC_OSC32CON_AMPEN      (1U << 8)   /* enable low power mode */
#define RTC_OSC32CON_LNBUFEN    (1U << 11)  /* ungate 32K to ABB */

#define RTC_POWERKEY1_KEY       0xa357
#define RTC_POWERKEY2_KEY       0x67d2

#define RTC_CON_LPEN            (1U << 2)
#define RTC_CON_LPRST           (1U << 3)
#define RTC_CON_CDBO            (1U << 4)
#define RTC_CON_F32KOB          (1U << 5)   /* 0: RTC_GPIO exports 32K */
#define RTC_CON_GPO             (1U << 6)
#define RTC_CON_GOE             (1U << 7)   /* 1: GPO mode, 0: GPI mode */
#define RTC_CON_GSR             (1U << 8)
#define RTC_CON_GSMT            (1U << 9)
#define RTC_CON_GPEN            (1U << 10)
#define RTC_CON_GPU             (1U << 11)
#define RTC_CON_GE4             (1U << 12)
#define RTC_CON_GE8             (1U << 13)
#define RTC_CON_GPI             (1U << 14)
#define RTC_CON_LPSTA_RAW       (1U << 15)  /* 32K was stopped */

#define RTC_CALI_BBPU_2SEC_EN       (1U << 8)
#define RTC_CALI_BBPU_2SEC_MODE_SHIFT		9
#define RTC_CALI_BBPU_2SEC_MODE_MSK			(3U << RTC_CALI_BBPU_2SEC_MODE_SHIFT)
#define RTC_CALI_BBPU_2SEC_STAT	  (1U << 11)

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
