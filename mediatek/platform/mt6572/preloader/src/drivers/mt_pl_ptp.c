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
 
#include "mt_pl_ptp.h"

static int ptp_init_setup(enum init_type mode, PTP_Init_T* PTP_Init_val)
{
    // config PTP register
    ptp_write(PTP_DESCHAR, ((((PTP_Init_val->BDES)<<8)&0xff00)|((PTP_Init_val->MDES)&0xff)));
    ptp_write(PTP_TEMPCHAR, ((((PTP_Init_val->VCO)<<16)&0xff0000) | (((PTP_Init_val->MTDES)<<8)&0xff00) | ((PTP_Init_val->DVTFIXED)&0xff)));
    ptp_write(PTP_DETCHAR, ((((PTP_Init_val->DCBDET)<<8)&0xff00) | ((PTP_Init_val->DCMDET)&0xff)));
    ptp_write(PTP_AGECHAR, ((((PTP_Init_val->AGEDELTA)<<8)&0xff00)  | ((PTP_Init_val->AGEM)&0xff)));
    ptp_write(PTP_DCCONFIG, ((PTP_Init_val->DCCONFIG)));
    ptp_write(PTP_AGECONFIG, ((PTP_Init_val->AGECONFIG)));
    if(mode == MONITOR_MODE){
        ptp_write(PTP_TSCALCS, ((((PTP_Init_val->BTS)<<12)&0xfff000) | ((PTP_Init_val->MTS)&0xfff)));
    }

    if( PTP_Init_val->AGEM == 0x0 )
    {
        ptp_write(PTP_RUNCONFIG, 0x80000000);
    }
    else
    {
        unsigned int temp_i, temp_filter, temp_value;
       
        temp_value = 0x0; 
        for (temp_i = 0 ; temp_i < 24 ; temp_i += 2 )
        {
            temp_filter = 0x3 << temp_i;
            	
            if( ((PTP_Init_val->AGECONFIG) & temp_filter) == 0x0 ){
                temp_value |= (0x1 << temp_i);
            }
            else{
                temp_value |= ((PTP_Init_val->AGECONFIG) & temp_filter);
            }
        }
        ptp_write(PTP_RUNCONFIG, temp_value);
    }

    ptp_write(PTP_FREQPCT30, ((((PTP_Init_val->FREQPCT3)<<24)&0xff000000) | (((PTP_Init_val->FREQPCT2)<<16)&0xff0000) | (((PTP_Init_val->FREQPCT1)<<8)&0xff00) | ((PTP_Init_val->FREQPCT0) & 0xff)));
    ptp_write(PTP_FREQPCT74, ((((PTP_Init_val->FREQPCT7)<<24)&0xff000000) | (((PTP_Init_val->FREQPCT6)<<16)&0xff0000) | (((PTP_Init_val->FREQPCT5)<<8)&0xff00) | ((PTP_Init_val->FREQPCT4) & 0xff)));
    ptp_write(PTP_LIMITVALS, ((((PTP_Init_val->VMAX)<<24)&0xff000000) | (((PTP_Init_val->VMIN)<<16)&0xff0000) | (((PTP_Init_val->DTHI)<<8)&0xff00) | ((PTP_Init_val->DTLO) & 0xff)));
    ptp_write(PTP_VBOOT, (((PTP_Init_val->VBOOT)&0xff)));
    ptp_write(PTP_DETWINDOW, (((PTP_Init_val->DETWINDOW)&0xffff)));
    ptp_write(PTP_PTPCONFIG, (((PTP_Init_val->DETMAX)&0xffff)));

    // clear all pending PTP interrupt & config PTPINTEN
    ptp_write(PTP_PTPINTSTS, 0xffffffff);

    // enable PTP INIT measurement
    //hw_calc_start = true;
    if(mode == INIT1_MODE){
        ptp_write(PTP_PTPINTEN, 0x00005f01);
        ptp_write(PTP_PTPEN, 0x00000001);
        //ptp_print("init1 start!!!!!\n");
        return 0;
    }
    else if(mode == INIT2_MODE){
        ptp_write(PTP_PTPINTEN, 0x00005f01);
        ptp_write(PTP_INIT2VALS, ((((PTP_Init_val->AGEVOFFSETIN)<<16)&0xffff0000) | ((PTP_Init_val->DCVOFFSETIN)&0xffff)));
        ptp_write(PTP_PTPEN, 0x00000005); 
        return 0;
    }
    else if(mode == MONITOR_MODE){
        ptp_write(PTP_PTPINTEN, 0x00FF0000);
        ptp_write(PTP_PTPEN, 0x00000002);
        return 0;
    }
    else{
        //ptp_print("[ERROR]ptp_init_setup: unknown type\n");
        return -1;
    }
}

void ptp_init1(void)
{
    static unsigned int val_0 = 0x14f76907;
    static unsigned int val_1 = 0xf6AAAAAA;
    static unsigned int val_2 = 0x14AAAAAA;
    static unsigned int val_3 = 0x60260000;

    
    PTP_Init_T PTP_Init_value;
    unsigned int PTPINTSTS;

    // enable thermal clock
    ptp_write(MT_CG_THEM_SET,  0x2);
    
    PTP_Init_value.PTPINITEN = (val_0) & 0x1;
    PTP_Init_value.PTPMONEN = (val_0 >> 1) & 0x1;
    PTP_Init_value.MDES = (val_0 >> 8) & 0xff;
    PTP_Init_value.BDES = (val_0 >> 16) & 0xff;
    PTP_Init_value.DCMDET = (val_0 >> 24) & 0xff;
    
    PTP_Init_value.DCCONFIG = (val_1) & 0xffffff;
    PTP_Init_value.DCBDET = (val_1 >> 24) & 0xff;
    
    PTP_Init_value.AGECONFIG = (val_2) & 0xffffff;
    PTP_Init_value.AGEM = (val_2 >> 24) & 0xff;
    
    PTP_Init_value.AGEDELTA = (val_3) & 0xff;
    PTP_Init_value.DVTFIXED = (val_3 >> 8) & 0xff;
    PTP_Init_value.MTDES = (val_3 >> 16) & 0xff;
    PTP_Init_value.VCO = (val_3 >> 24) & 0xff;

    //need to provide by efuse
   
    /*PTP_Init_value.MDES = 0x3c;
    PTP_Init_value.BDES = 0x28;
    PTP_Init_value.DCMDET = 0x11;    
    PTP_Init_value.DCBDET = 0x0;    

    PTP_Init_value.PTPINITEN = 0x1;
    PTP_Init_value.PTPMONEN = 0x0;
    PTP_Init_value.AGEM = 0x14;
    PTP_Init_value.DCCONFIG = 0xaaaaaa;
    PTP_Init_value.AGECONFIG = 0xaaaaaa;    
    PTP_Init_value.AGEDELTA = 0x0;
    PTP_Init_value.DVTFIXED = 0x0;
    PTP_Init_value.MTDES = 0x26;
    PTP_Init_value.VCO = 0x60;

    PTP_Init_value.FREQPCT0 = 120; // max freq 1200 x 100%
    PTP_Init_value.FREQPCT1 = 100; // 1000
    PTP_Init_value.FREQPCT2 = 80;  // 800
    PTP_Init_value.FREQPCT3 = 60;  // 600
    PTP_Init_value.FREQPCT4 = 0;
    PTP_Init_value.FREQPCT5 = 0;
    PTP_Init_value.FREQPCT6 = 0;
    PTP_Init_value.FREQPCT7 = 0;
    */
    // 40 us. This is special case for init1 (bclk source is switch to 26M, and bclk = 26M also)
    PTP_Init_value.DETWINDOW = 0x514;  // 50us
    PTP_Init_value.VMAX = 0x58; // 1.25v (700mv + n * 6.25mv)    
    PTP_Init_value.VMIN = 0x48; // 1.15v (700mv + n * 6.25mv)    
    PTP_Init_value.DTHI = 0x01; // positive
    PTP_Init_value.DTLO = 0xfe; // negative (2's compliment)
    PTP_Init_value.VBOOT = 0x48; // 115v  (700mv + n * 6.25mv)    
    PTP_Init_value.DETMAX = 0xffff; // This timeout value is in cycles of bclk_ck.

    // set register for init1
    ptp_init_setup(INIT1_MODE, &PTP_Init_value);

    // polling PTP_PTPINTSTS till = 0x1
    PTPINTSTS = ptp_read(PTP_PTPINTSTS);
    while(PTPINTSTS != 0x1){ PTPINTSTS = ptp_read(PTP_PTPINTSTS); };
    
    // Set PTPEN.PTPINITEN/PTPEN.PTPINIT2EN = 0x0 & Clear PTP INIT interrupt PTPINTSTS = 0x00000001
    ptp_write(PTP_PTPEN, 0x0);
    ptp_write(PTP_PTPINTSTS, 0x1);
    
    // disable thermal clock
    ptp_write(MT_CG_THEM_CLR,  0x2);

    //clkmux_sel(MT_CLKMUX_AXIBUS_GFMUX_SEL, MT_CG_MPLL_D12, "PTP_BCLK");
    //}
}

