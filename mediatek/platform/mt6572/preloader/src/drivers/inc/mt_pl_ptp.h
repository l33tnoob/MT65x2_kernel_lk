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
 * MediaTek Inc. (C) 2013. All rights reserved.
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
 
#ifndef __MTK_PL_PTP_H__
#define __MTK_PL_PTP_H__


//#define PTP_GIC_SIGNALS         (32)
//#define PTP_FSM_IRQ_ID          (PTP_GIC_SIGNALS + 30)
//#define PTP_THERM_IRQ_ID        (PTP_GIC_SIGNALS + 25)
//#define MAX_OPP_NUM             (8)

/* ====== 6572 PTP register ====== */
#define PTP_base_addr           (0x1100D000)
#define PTP_ctr_reg_addr        (PTP_base_addr+0x200)

#define PTP_DESCHAR             (PTP_ctr_reg_addr)
#define PTP_TEMPCHAR            (PTP_ctr_reg_addr+0x04)
#define PTP_DETCHAR             (PTP_ctr_reg_addr+0x08)
#define PTP_AGECHAR             (PTP_ctr_reg_addr+0x0c)

#define PTP_DCCONFIG            (PTP_ctr_reg_addr+0x10)
#define PTP_AGECONFIG           (PTP_ctr_reg_addr+0x14)
#define PTP_FREQPCT30           (PTP_ctr_reg_addr+0x18)
#define PTP_FREQPCT74           (PTP_ctr_reg_addr+0x1c)

#define PTP_LIMITVALS           (PTP_ctr_reg_addr+0x20)
#define PTP_VBOOT               (PTP_ctr_reg_addr+0x24)
#define PTP_DETWINDOW           (PTP_ctr_reg_addr+0x28)
#define PTP_PTPCONFIG           (PTP_ctr_reg_addr+0x2c)

#define PTP_TSCALCS             (PTP_ctr_reg_addr+0x30)
#define PTP_RUNCONFIG           (PTP_ctr_reg_addr+0x34)
#define PTP_PTPEN               (PTP_ctr_reg_addr+0x38)
#define PTP_INIT2VALS           (PTP_ctr_reg_addr+0x3c)

#define PTP_DCVALUES            (PTP_ctr_reg_addr+0x40)
#define PTP_AGEVALUES           (PTP_ctr_reg_addr+0x44)
#define PTP_VOP30               (PTP_ctr_reg_addr+0x48)
#define PTP_VOP74               (PTP_ctr_reg_addr+0x4c)

#define PTP_TEMP                (PTP_ctr_reg_addr+0x50)
#define PTP_PTPINTSTS           (PTP_ctr_reg_addr+0x54)
#define PTP_PTPINTSTSRAW        (PTP_ctr_reg_addr+0x58)
#define PTP_PTPINTEN            (PTP_ctr_reg_addr+0x5c)
#define PTP_AGECOUNT            (PTP_ctr_reg_addr+0x7c)


/* ====== 6589 Thermal Controller register ======= */
//#define PTP_Thermal_ctr_reg_addr (PTP_base_addr)

#define TEMPMONCTL0             (PTP_base_addr)
#define TEMPMONCTL1             (PTP_base_addr+0x04)
#define TEMPMONCTL2             (PTP_base_addr+0x08)
#define TEMPMONINT              (PTP_base_addr+0x0c)

#define TEMPMONINTSTS           (PTP_base_addr+0x10)
#define TEMPMONIDET0            (PTP_base_addr+0x14)
#define TEMPMONIDET1            (PTP_base_addr+0x18)
#define TEMPMONIDET2            (PTP_base_addr+0x1c)

#define TEMPH2NTHRE             (PTP_base_addr+0x24)
#define TEMPHTHRE               (PTP_base_addr+0x28)
#define TEMPCTHRE               (PTP_base_addr+0x2c)

#define TEMPOFFSETH             (PTP_base_addr+0x30)
#define TEMPOFFSETL             (PTP_base_addr+0x34)
#define TEMPMSRCTL0             (PTP_base_addr+0x38)
#define TEMPMSRCTL1             (PTP_base_addr+0x3c)

#define TEMPAHBPOLL             (PTP_base_addr+0x40)
#define TEMPAHBTO               (PTP_base_addr+0x44)
#define TEMPADCPNP0             (PTP_base_addr+0x48)
#define TEMPADCPNP1             (PTP_base_addr+0x4c)

#define TEMPADCPNP2             (PTP_base_addr+0x50)
#define TEMPADCMUX              (PTP_base_addr+0x54)
#define TEMPADCEXT              (PTP_base_addr+0x58)
#define TEMPADCEXT1             (PTP_base_addr+0x5c)

#define TEMPADCEN               (PTP_base_addr+0x60)
#define TEMPPNPMUXADDR          (PTP_base_addr+0x64)
#define TEMPADCMUXADDR          (PTP_base_addr+0x68)
#define TEMPADCEXTADDR          (PTP_base_addr+0x6c)

#define TEMPADCEXT1ADDR         (PTP_base_addr+0x70)
#define TEMPADCENADDR           (PTP_base_addr+0x74)
#define TEMPADCVALIDADDR        (PTP_base_addr+0x78)
#define TEMPADCVOLTADDR         (PTP_base_addr+0x7c)

#define TEMPRDCTRL              (PTP_base_addr+0x80)
#define TEMPADCVALIDMASK        (PTP_base_addr+0x84)
#define TEMPADCVOLTAGESHIFT     (PTP_base_addr+0x88)
#define TEMPADCWRITECTRL        (PTP_base_addr+0x8c)

#define TEMPMSR0                (PTP_base_addr+0x90)
#define TEMPMSR1                (PTP_base_addr+0x94)
#define TEMPMSR2                (PTP_base_addr+0x98)

#define TEMPIMMD0               (PTP_base_addr+0xa0)
#define TEMPIMMD1               (PTP_base_addr+0xa4)
#define TEMPIMMD2               (PTP_base_addr+0xa8)

#define TEMPMONIDET3            (PTP_base_addr+0xb0)
#define TEMPADCPNP3             (PTP_base_addr+0xb4)
#define TEMPMSR3                (PTP_base_addr+0xb8)
#define TEMPIMMD3               (PTP_base_addr+0xbc)

#define TEMPPROTCTL             (PTP_base_addr+0xc0)
#define TEMPPROTTA              (PTP_base_addr+0xc4)
#define TEMPPROTTB              (PTP_base_addr+0xc8)
#define TEMPPROTTC              (PTP_base_addr+0xcc)

#define TEMPSPARE0              (PTP_base_addr+0xf0)
#define TEMPSPARE1              (PTP_base_addr+0xf4)
#define TEMPSPARE2              (PTP_base_addr+0xf8)
#define TEMPSPARE3              (PTP_base_addr+0xfc)

#define MT_CG_THEM_SET          (0x10000084)
#define MT_CG_THEM_CLR          (0x10000054)

#define ptp_print print

#define ptp_read(addr)		(*(volatile unsigned int *)(addr))
#define ptp_write(addr, val)  {	(*(volatile unsigned int *)(addr) = (unsigned int)(val)); } while(0)

typedef struct {
    unsigned int ADC_CALI_EN;
    unsigned int PTPINITEN;
    unsigned int PTPMONEN;
    
    unsigned int MDES;
    unsigned int BDES;
    unsigned int DCCONFIG;
    unsigned int DCMDET;
    unsigned int DCBDET;
    unsigned int AGECONFIG;
    unsigned int AGEM;
    unsigned int AGEDELTA;
    unsigned int DVTFIXED;
    unsigned int VCO;
    unsigned int MTDES;
    unsigned int MTS;
    unsigned int BTS;

    char FREQPCT0;
    char FREQPCT1;
    char FREQPCT2;
    char FREQPCT3;
    char FREQPCT4;
    char FREQPCT5;
    char FREQPCT6;
    char FREQPCT7;

    unsigned int DETWINDOW;
    unsigned int VMAX;
    unsigned int VMIN;
    unsigned int DTHI;
    unsigned int DTLO;
    unsigned int VBOOT;
    unsigned int DETMAX;

    unsigned int DCVOFFSETIN;
    unsigned int AGEVOFFSETIN;
} PTP_Init_T;

enum init_type{
    INIT1_MODE = 0,
    INIT2_MODE,
    MONITOR_MODE,
};

void ptp_init1(void);

#endif /* __MTK_PL_PTP_H__  */
