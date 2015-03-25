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

#include "mt6582.h"

#include "pll.h"
#include "timer.h"
#include "spm.h"

#include "project.h"
/***********************
 * MEMPLL Configuration
 ***********************/

#define r_bias_en_stb_time              (0x00000000 << 24)
#define r_bias_lpf_en_stb_time          (0x00000000 << 16)
#define r_mempll_en_stb_time            (0x00000000 << 8)
#define r_dmall_ck_en_stb_time          (0x00000000 << 0)

#define r_dds_en_stb_time               (0x00000000 << 24)
#define r_div_en_stb_time               (0x00000000 << 16)
#define r_dmpll2_ck_en_stb_time         (0x00000000 << 8)
#define r_iso_en_stb_time               (0x00000000 << 0)

#define r_bias_en_stb_dis               (0x00000001 << 28)
#define r_bias_en_src_sel               (0x00000001 << 24)
#define r_bias_lpf_en_stb_dis           (0x00000001 << 20)
#define r_bias_lpf_en_src_sel           (0x00000001 << 16)
#define r_mempll4_en_stb_dis            (0x00000001 << 15)
#define r_mempll3_en_stb_dis            (0x00000001 << 14)
#define r_mempll2_en_stb_dis            (0x00000001 << 13)
#define r_mempll_en_stb_dis             (0x00000001 << 12)
#define r_mempll4_en_src_sel            (0x00000001 << 11)
#define r_mempll3_en_src_sel            (0x00000001 << 10)
#define r_mempll2_en_src_sel            (0x00000001 << 9)
#define r_mempll_en_src_sel             (0x00000001 << 8)
#define r_dmall_ck_en_stb_dis           (0x00000001 << 4)
#define r_dmall_ck_en_src_sel           (0x00000001 << 0)

#define r_dds_en_stb_dis                (0x00000001 << 28)
#define r_dds_en_src_sel                (0x00000001 << 24)
#define r_div_en_stb_dis                (0x00000001 << 20)
#define r_div_en_src_sel                (0x00000001 << 16)
#define r_dmpll2_ck_en_stb_dis          (0x00000001 << 12)
#define r_dmpll2_ck_en_src_sel          (0x00000001 << 8)
#define r_iso_en_stb_dis                (0x00000001 << 4)
#define r_iso_en_src_sel                (0x00000001 << 0)

#define r_dmbyp_pll4                    (0x00000001 << 0)
#define r_dmbyp_pll3                    (0x00000001 << 1)
#define r_dm1pll_sync_mode              (0x00000001 << 2)
#define r_dmall_ck_en                   (0x00000001 << 4)
#define r_dmpll2_clk_en                 (0x00000001 << 5)

#define pllc1_prediv_1_0_n1             (0x00000000 << 16)
#define pllc1_vco_div_sel_n2            (0x00000000 << 13)

#ifdef DDR_533
    // DDR533
    #ifdef DDRPHY_3PLL_MODE
        #define pllc1_mempll_n_info_30_24_n4        (0x00000046 << 25)
        #define pllc1_mempll_n_info_23_0_n4         (0x000aaaaa << 1)
    #else
        #define pllc1_mempll_n_info_30_24_n4        (0x00000050 << 25)
        #define pllc1_mempll_n_info_23_0_n4         (0x000c30c3 << 1)
    #endif
#else
    // DDR800
    #ifdef DDR_800
        #ifdef DDRPHY_3PLL_MODE
            #define pllc1_mempll_n_info_30_24_n4    (0x0000004e << 25)
            #define pllc1_mempll_n_info_23_0_n4     (0x00d89d89 << 1)
        #else
            #define pllc1_mempll_n_info_30_24_n4    (0x00000051 << 25)
            #define pllc1_mempll_n_info_23_0_n4     (0x0063bb65 << 1)
        #endif
    #else
        // DDR1172
        #ifdef DDR_1172
            #ifdef DDRPHY_3PLL_MODE
                #define pllc1_mempll_n_info_30_24_n4    (0x0000004d << 25)
                #define pllc1_mempll_n_info_23_0_n4     (0x0001a41a << 1)            
            #else
                #define pllc1_mempll_n_info_30_24_n4    (0x00000050 << 25)
                #define pllc1_mempll_n_info_23_0_n4     (0x005ac17f << 1)
            #endif
        #else
            //DDR1110
            #ifdef DDR_1110
                #ifdef DDRPHY_3PLL_MODE
                    #define pllc1_mempll_n_info_30_24_n4    (0x0000004f << 25)
                    #define pllc1_mempll_n_info_23_0_n4     (0x00901ca4 << 1)
                #else
                    #define pllc1_mempll_n_info_30_24_n4    (0x00000051 << 25)
                    #define pllc1_mempll_n_info_23_0_n4     (0x0069c9f5 << 1)
                #endif
            #else
                //DDR1000
                #ifdef DDR_1000
                    #ifdef DDRPHY_3PLL_MODE
                        #define pllc1_mempll_n_info_30_24_n4    (0x0000004e << 25)
                        #define pllc1_mempll_n_info_23_0_n4     (0x00d89d89 << 1)
                    #else
                        #define pllc1_mempll_n_info_30_24_n4    (0x00000050 << 25)
                        #define pllc1_mempll_n_info_23_0_n4     (0x00de2b6c << 1)
                    #endif
                #else
                    //DDR912
                    #ifdef DDR_912
                        #ifdef DDRPHY_3PLL_MODE
                            #define pllc1_mempll_n_info_30_24_n4    (0x0000004f << 25)
                            #define pllc1_mempll_n_info_23_0_n4     (0x00e5be5b << 1)
                        #else
                            #define pllc1_mempll_n_info_30_24_n4    (0x00000052 << 25)
                            #define pllc1_mempll_n_info_23_0_n4     (0x002e22e2 << 1)
                        #endif
                    #else
                        // DDR1066
                        #ifdef DDRPHY_3PLL_MODE
                            #define pllc1_mempll_n_info_30_24_n4    (0x0000004c << 25)
                            #define pllc1_mempll_n_info_23_0_n4     (0x0068ba2e << 1)
                        #else
                            #define pllc1_mempll_n_info_30_24_n4    (0x00000050 << 25)
                            #define pllc1_mempll_n_info_23_0_n4     (0x000c30c3 << 1)
                        #endif
                    #endif
                #endif
            #endif
        #endif
    #endif
#endif

#define pllc1_postdiv_1_0_n5            (0x00000000 << 14)

#define pllc1_mempll_n_info_chg         (0x00000001 << 0)

#define pllc1_mempll_div_en             (0x00000001 << 24)
#define pllc1_mempll_div_6_0            (0x00000052 << 25)
#define pllc1_mempll_reserve_2          (0x00000001 << 18)

#define pllc1_mempll_top_reserve_2_0    (0x00000000 << 16)
#define pllc1_mempll_bias_en            (0x00000001 << 14)
#define pllc1_mempll_bias_lpf_en        (0x00000001 << 15)
#define pllc1_mempll_en                 (0x00000001 << 2)

#define pllc1_mempll_sdm_prd_1          (0x00000001 << 11)

#define mempll2_prediv_1_0              (0x00000000 << 0)
#define mempll2_vco_div_sel             (0x00000001 << 29)
#define mempll2_br                      (0x00000001 << 26)
#define mempll2_bp                      (0x00000001 << 27)

#ifdef DDR_533
    // DDR533
    #define mempll2_m4pdiv_1_0          (0x00000001 << 10)
    #ifdef DDRPHY_3PLL_MODE
        #define mempll2_fbdiv_6_0       (0x00000006 << 2)
    #else  //DDRPHY_3PLL_MODE
        #define mempll2_fbdiv_6_0       (0x0000002a << 2)
    #endif //DDRPHY_3PLL_MODE
#else
    // DDR800
    #ifdef DDR_800
        #define mempll2_m4pdiv_1_0      (0x00000000 << 10)
        #ifdef DDRPHY_3PLL_MODE
            #define mempll2_fbdiv_6_0   (0x00000008 << 2)
        #else  //DDRPHY_3PLL_MODE
            #define mempll2_fbdiv_6_0   (0x0000001f << 2)
        #endif //DDRPHY_3PLL_MODE
   #else
       // DDR1172
       #ifdef DDR_1172
           #define mempll2_m4pdiv_1_0       (0x00000000 << 10)
           #ifdef DDRPHY_3PLL_MODE
               #define mempll2_fbdiv_6_0    (0x0000000c << 2)
           #else
               #define mempll2_fbdiv_6_0    (0x0000002e << 2)
           #endif
       #else
           //DDR1110
           #ifdef DDR_1110
               #define mempll2_m4pdiv_1_0       (0x00000000 << 10)
               #ifdef DDRPHY_3PLL_MODE
                   #define mempll2_fbdiv_6_0    (0x0000000b << 2)
               #else
                   #define mempll2_fbdiv_6_0    (0x0000002b << 2)
               #endif
           #else
               //DDR1000
               #ifdef DDR_1000
                   #define mempll2_m4pdiv_1_0       (0x00000000 << 10)
                   #ifdef DDRPHY_3PLL_MODE
                       #define mempll2_fbdiv_6_0    (0x0000000a << 2)
                   #else
                       #define mempll2_fbdiv_6_0    (0x00000027 << 2)
                   #endif
               #else
                   //DDR912
                   #ifdef DDR_912
                       #define mempll2_m4pdiv_1_0       (0x00000000 << 10)
                       #ifdef DDRPHY_3PLL_MODE
                           #define mempll2_fbdiv_6_0    (0x00000009 << 2)
                       #else
                           #define mempll2_fbdiv_6_0    (0x00000023 << 2)
                       #endif
                   #else
                       // DDR1066
                       #define mempll2_m4pdiv_1_0       (0x00000000 << 10)
                       #ifdef DDRPHY_3PLL_MODE
                           #define mempll2_fbdiv_6_0    (0x0000000b << 2)
                       #else
                           #define mempll2_fbdiv_6_0    (0x0000002a << 2)
                       #endif
                   #endif
               #endif
           #endif
       #endif
   #endif
#endif

#ifdef DDRPHY_3PLL_MODE
#define mempll2_fb_mck_sel              (0x00000001 << 9)
#else
#define mempll2_fb_mck_sel              (0x00000000 << 9)
#endif
#define mempll2_fbksel_1_0              (0x00000000 << 10)
#define mempll2_posdiv_1_0              (0x00000000 << 30)
#define mempll2_ref_dl_4_0              (0x00000000 << 27)
#define mempll2_fb_dl_4_0               (0x00000000 << 22)
#define mempll2_en                      (0x00000001 << 18)

#define mempll3_prediv_1_0              (0x00000000 << 0)
#define mempll3_vco_div_sel             (0x00000001 << 29)
#define mempll3_br                      (0x00000001 << 26)
#define mempll3_bp                      (0x00000001 << 27)

#ifdef DDR_533
    // DDR533
    #define mempll3_m4pdiv_1_0          (0x00000001 << 10)
    #ifdef DDRPHY_3PLL_MODE
        #define mempll3_fbdiv_6_0       (0x00000006 << 2)
    #else  //DDRPHY_3PLL_MODE
        #define mempll3_fbdiv_6_0       (0x0000002a << 2)
    #endif //DDRPHY_3PLL_MODE
#else
    #ifdef DDR_800
    // DDR800
        #define mempll3_m4pdiv_1_0      (0x00000000 <<10)
        #ifdef DDRPHY_3PLL_MODE
            #define mempll3_fbdiv_6_0   (0x00000008 << 2)
        #else  //DDRPHY_3PLL_MODE
            #define mempll3_fbdiv_6_0   (0x0000001f << 2)
        #endif //DDRPHY_3PLL_MODE
    #else
        // DDR1172
        #ifdef DDR_1172            
            #define mempll3_m4pdiv_1_0      (0x00000000 << 10)
            #ifdef DDRPHY_3PLL_MODE
                #define mempll3_fbdiv_6_0   (0x0000000c << 2)
            #else
                #define mempll3_fbdiv_6_0   (0x0000002e << 2)
            #endif
        #else
            //DDR1110
            #ifdef DDR_1110
                #define mempll3_m4pdiv_1_0      (0x00000000 << 10)
                #ifdef DDRPHY_3PLL_MODE
                    #define mempll3_fbdiv_6_0   (0x0000000b << 2)
                #else
                    #define mempll3_fbdiv_6_0   (0x0000002b << 2)
                #endif
            #else
                //DDR1000
                #ifdef DDR_1000
                    #define mempll3_m4pdiv_1_0      (0x00000000 << 10)
                    #ifdef DDRPHY_3PLL_MODE
                        #define mempll3_fbdiv_6_0   (0x0000000a << 2)
                    #else
                        #define mempll3_fbdiv_6_0   (0x00000027 << 2)
                    #endif
                #else
                    //DDR912
                    #ifdef DDR_912
                        #define mempll3_m4pdiv_1_0      (0x00000000 << 10)
                        #ifdef DDRPHY_3PLL_MODE
                            #define mempll3_fbdiv_6_0   (0x00000009 << 2)
                        #else
                            #define mempll3_fbdiv_6_0   (0x00000023 << 2)
                        #endif
                    #else
                        // DDR1066
                        #define mempll3_m4pdiv_1_0      (0x00000000 << 10)
                        #ifdef DDRPHY_3PLL_MODE
                            #define mempll3_fbdiv_6_0   (0x0000000b << 2)
                        #else
                            #define mempll3_fbdiv_6_0   (0x0000002a << 2)
                        #endif
                    #endif
                #endif
            #endif    
        #endif
    #endif
#endif

#ifdef DDRPHY_3PLL_MODE
    #define mempll3_fb_mck_sel          (0x00000001 << 9)
#else
    #define mempll3_fb_mck_sel          (0x00000000 << 9)
#endif
#define mempll3_fbksel_1_0              (0x00000000 << 10)
#define mempll3_posdiv_1_0              (0x00000000 << 30)
#define mempll3_ref_dl_4_0              (0x00000000 << 27)
#define mempll3_fb_dl_4_0               (0x00000000 << 22)
#define mempll3_en                      (0x00000001 << 18)

#define mempll4_prediv_1_0              (0x00000000 << 0)
#define mempll4_vco_div_sel             (0x00000001 << 29)
#define mempll4_br                      (0x00000001 << 26)
#define mempll4_bp                      (0x00000001 << 27)

#ifdef DDR_533
    // DDR533
    #define mempll4_m4pdiv_1_0          (0x00000001 << 10)
    #ifdef DDRPHY_3PLL_MODE
        #define mempll4_fbdiv_6_0       (0x00000006 << 2)
    #else  //DDRPHY_3PLL_MODE
        #define mempll4_fbdiv_6_0       (0x0000002a << 2)
    #endif //DDRPHY_3PLL_MODE
#else
    #ifdef DDR_800
    // DDR800
        #define mempll4_m4pdiv_1_0      (0x00000000 << 10)
        #ifdef DDRPHY_3PLL_MODE
            #define mempll4_fbdiv_6_0   (0x00000008 << 2)
        #else  //DDRPHY_3PLL_MODE
            #define mempll4_fbdiv_6_0   (0x0000001f << 2)
        #endif //DDRPHY_3PLL_MODE
    #else
        // DDR1172
        #ifdef DDR_1172
            #define mempll4_m4pdiv_1_0      (0x00000000 << 10)
            #ifdef DDRPHY_3PLL_MODE
                #define mempll4_fbdiv_6_0   (0x0000000c << 2)
            #else
                #define mempll4_fbdiv_6_0   (0x0000002e << 2)
            #endif 
        #else
            //DDR1110
            #ifdef DDR_1110
                #define mempll4_m4pdiv_1_0      (0x00000000 << 10)
                #ifdef DDRPHY_3PLL_MODE
                    #define mempll4_fbdiv_6_0   (0x0000000b << 2)
                #else
                    #define mempll4_fbdiv_6_0   (0x0000002b << 2)
                #endif
            #else
                //DDR1000
                #ifdef DDR_1000
                    #define mempll4_m4pdiv_1_0      (0x00000000 << 10)
                    #ifdef DDRPHY_3PLL_MODE
                        #define mempll4_fbdiv_6_0   (0x0000000a << 2)
                    #else
                        #define mempll4_fbdiv_6_0   (0x00000027 << 2)
                    #endif
                #else
                    //DDR912
                    #ifdef DDR_912
                        #define mempll4_m4pdiv_1_0      (0x00000000 << 10)
                        #ifdef DDRPHY_3PLL_MODE
                            #define mempll4_fbdiv_6_0   (0x00000009 << 2)
                        #else
                            #define mempll4_fbdiv_6_0   (0x00000023 << 2)
                        #endif
                    #else
                        // DDR1066
                        #define mempll4_m4pdiv_1_0      (0x00000000 << 10)
                        #ifdef DDRPHY_3PLL_MODE
                            #define mempll4_fbdiv_6_0   (0x0000000b << 2)
                        #else
                            #define mempll4_fbdiv_6_0   (0x0000002a << 2)
                        #endif
                    #endif
                #endif
            #endif    
        #endif
    #endif
#endif

#ifdef DDRPHY_3PLL_MODE
    #define mempll4_fb_mck_sel          (0x00000001 << 9)
#else
    #define mempll4_fb_mck_sel          (0x00000000 << 9)
#endif
#define mempll4_fbksel_1_0              (0x00000000 << 10)
#define mempll4_posdiv_1_0              (0x00000000 << 30)
#define mempll4_ref_dl_4_0              (0x00000000 << 27)
#define mempll4_fb_dl_4_0               (0x00000000 << 22)
#define mempll4_en                      (0x00000001 << 18)

void mt_mempll_init(void)
{
    unsigned int temp;

    /*********************************
    * (1) Setup DDRPHY operation mode
    **********************************/

    *((UINT32P)(DRAMC0_BASE + 0x007c)) |= 0x00000001; //DFREQ_DIV2=1
    *((UINT32P)(DDRPHY_BASE + 0x007c)) |= 0x00000001;

    #ifdef DRAMC_ASYNC
        #ifdef DDRPHY_3PLL_MODE
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000020; //3PLL mode, OK
        #else
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000003; //1PLL async mode, OK
        #endif
    #else
        #ifdef DDRPHY_3PLL_MODE
            #ifdef DDRPHY_2PLL
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000022; 
            #else
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000020; //3PLL sync mode, OK
            #endif
        #else
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000007; //1PLL sync mode, OK
        #endif
    #endif

    /*****************************************************************************************
    * (2) Setup MEMPLL operation case & frequency. May set according to dram type & frequency
    ******************************************************************************************/

    *((UINT32P)(DDRPHY_BASE + (0x0170 <<2))) = r_bias_en_stb_time | r_bias_lpf_en_stb_time | r_mempll_en_stb_time | r_dmall_ck_en_stb_time;
    *((UINT32P)(DDRPHY_BASE + (0x0171 <<2))) = r_dds_en_stb_time | r_div_en_stb_time | r_dmpll2_ck_en_stb_time | r_iso_en_stb_time;
    *((UINT32P)(DDRPHY_BASE + (0x0172 <<2))) = r_bias_en_stb_dis| r_bias_en_src_sel | r_bias_lpf_en_stb_dis| r_bias_lpf_en_src_sel | r_mempll4_en_stb_dis| r_mempll3_en_stb_dis| r_mempll2_en_stb_dis| r_mempll_en_stb_dis| r_mempll4_en_src_sel | r_mempll3_en_src_sel | r_mempll2_en_src_sel | r_mempll_en_src_sel | r_dmall_ck_en_stb_dis | r_dmall_ck_en_src_sel;
    *((UINT32P)(DDRPHY_BASE + (0x0173 <<2))) = r_dds_en_stb_dis| r_dds_en_src_sel | r_div_en_stb_dis| r_div_en_src_sel | r_dmpll2_ck_en_stb_dis| r_dmpll2_ck_en_src_sel | r_iso_en_stb_dis| r_iso_en_src_sel | r_dmbyp_pll4 | r_dmbyp_pll3 | r_dm1pll_sync_mode | r_dmall_ck_en | r_dmpll2_clk_en;

    *((UINT32P)(DDRPHY_BASE + (0x0180 <<2))) = pllc1_prediv_1_0_n1  | pllc1_vco_div_sel_n2 | pllc1_postdiv_1_0_n5 ;

    *((UINT32P)(DDRPHY_BASE + (0x0181 <<2))) |= pllc1_mempll_div_6_0 | pllc1_mempll_reserve_2;
    *((UINT32P)(DDRPHY_BASE + (0x0183 <<2))) = mempll2_vco_div_sel | mempll2_m4pdiv_1_0 | mempll2_fb_mck_sel | mempll2_posdiv_1_0 | mempll2_br | mempll2_bp;
    *((UINT32P)(DDRPHY_BASE + (0x0182 <<2))) = pllc1_mempll_top_reserve_2_0 | mempll2_prediv_1_0 | mempll2_fbdiv_6_0 | mempll2_fbksel_1_0;
    *((UINT32P)(DDRPHY_BASE + (0x0185 <<2))) = mempll3_vco_div_sel | mempll3_m4pdiv_1_0 | mempll3_fb_mck_sel | mempll3_posdiv_1_0 | mempll3_br | mempll3_bp;
    *((UINT32P)(DDRPHY_BASE + (0x0184 <<2))) = mempll2_ref_dl_4_0 | mempll2_fb_dl_4_0 | mempll3_prediv_1_0 | mempll3_fbdiv_6_0 | mempll3_fbksel_1_0;
    *((UINT32P)(DDRPHY_BASE + (0x0187 <<2))) = mempll4_vco_div_sel | mempll4_m4pdiv_1_0 | mempll4_fb_mck_sel | mempll4_posdiv_1_0 | mempll4_br | mempll4_bp;
    *((UINT32P)(DDRPHY_BASE + (0x0186 <<2))) = mempll3_fb_dl_4_0 | mempll3_ref_dl_4_0 | mempll4_prediv_1_0 | mempll4_fbdiv_6_0 | mempll4_fbksel_1_0;
    *((UINT32P)(DDRPHY_BASE + (0x0188 <<2))) = mempll4_ref_dl_4_0 | mempll4_fb_dl_4_0;
    *((UINT32P)(DDRPHY_BASE + (0x0189 <<2))) = pllc1_mempll_n_info_23_0_n4 | pllc1_mempll_n_info_30_24_n4  | pllc1_mempll_n_info_chg;

    #ifdef DRAMC_ASYNC
        #ifdef DDRPHY_3PLL_MODE
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000020 | r_dmpll2_clk_en; //3PLL mode
        #else
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000003 ; //1PLL async mode
        #endif
    #else
        #ifdef DDRPHY_3PLL_MODE
            #ifdef DDRPHY_2PLL
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000022  | r_dmpll2_clk_en;; 
            #else
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000020 | r_dmpll2_clk_en; //3PLL sync mode
            #endif
        #else
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000007 ; //1PLL sync mode
        #endif
    #endif

    /***********************************
    * (3) Setup MEMPLL power on sequence
    ************************************/

    gpt_busy_wait_us(2);

    *((UINT32P)(DDRPHY_BASE + (0x0181 <<2))) |= pllc1_mempll_div_6_0 | pllc1_mempll_reserve_2 | (pllc1_mempll_bias_en );
    gpt_busy_wait_us(2);

    *((UINT32P)(DDRPHY_BASE + (0x0181 <<2))) |= pllc1_mempll_div_6_0 | pllc1_mempll_reserve_2 | (pllc1_mempll_bias_en | pllc1_mempll_bias_lpf_en);
    gpt_busy_wait_us(1000);

    *((UINT32P)(DDRPHY_BASE + (0x0180 <<2))) = pllc1_prediv_1_0_n1  | pllc1_vco_div_sel_n2 | pllc1_postdiv_1_0_n5  | (pllc1_mempll_en);
    gpt_busy_wait_us(20);

    *((UINT32P)(DDRPHY_BASE + (0x018a <<2))) = 0x10000000;
    gpt_busy_wait_us(2);

    *((UINT32P)(DDRPHY_BASE + (0x018a <<2))) = 0x18000000;

    *((UINT32P)(DDRPHY_BASE + (0x0188 <<2))) = mempll4_ref_dl_4_0 | mempll4_fb_dl_4_0 | ( pllc1_mempll_sdm_prd_1);
    gpt_busy_wait_us(2);

    *((UINT32P)(DDRPHY_BASE + (0x0180 <<2))) =  (pllc1_prediv_1_0_n1  | pllc1_vco_div_sel_n2  | pllc1_postdiv_1_0_n5  | (pllc1_mempll_en));

    *((UINT32P)(DDRPHY_BASE + (0x0181 <<2))) |= pllc1_mempll_div_en | (pllc1_mempll_div_6_0 | pllc1_mempll_reserve_2 | (pllc1_mempll_bias_en | pllc1_mempll_bias_lpf_en));
    gpt_busy_wait_us(23);

    #ifdef DDRPHY_3PLL_MODE
        *((UINT32P)(DDRPHY_BASE + (0x0183 <<2))) = mempll2_en | mempll2_vco_div_sel | mempll2_m4pdiv_1_0 | mempll2_fb_mck_sel | mempll2_posdiv_1_0;
        #ifdef DDRPHY_2PLL
            *((UINT32P)(DDRPHY_BASE + (0x0185 <<2))) = (~mempll3_en) & (mempll3_vco_div_sel | mempll3_m4pdiv_1_0 | mempll3_fb_mck_sel | mempll3_posdiv_1_0);
        #else
            *((UINT32P)(DDRPHY_BASE + (0x0185 <<2))) = mempll3_en | mempll3_vco_div_sel | mempll3_m4pdiv_1_0 | mempll3_fb_mck_sel | mempll3_posdiv_1_0;
        #endif
        *((UINT32P)(DDRPHY_BASE + (0x0187 <<2))) = mempll4_en | mempll4_vco_div_sel | mempll4_m4pdiv_1_0 | mempll4_fb_mck_sel | mempll4_posdiv_1_0;
    #else
        *((UINT32P)(DDRPHY_BASE + (0x0183 <<2))) = mempll2_en | mempll2_vco_div_sel | mempll2_m4pdiv_1_0 | mempll2_fb_mck_sel | mempll2_posdiv_1_0;
        *((UINT32P)(DDRPHY_BASE + (0x0185 <<2))) =              mempll3_vco_div_sel | mempll3_m4pdiv_1_0 | mempll3_fb_mck_sel | mempll3_posdiv_1_0;
        *((UINT32P)(DDRPHY_BASE + (0x0187 <<2))) =              mempll4_vco_div_sel | mempll4_m4pdiv_1_0 | mempll4_fb_mck_sel | mempll4_posdiv_1_0;
    #endif

    gpt_busy_wait_us(23);

    #ifdef DRAMC_ASYNC
        #ifdef DDRPHY_3PLL_MODE
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000020 | r_dmpll2_clk_en | r_dmall_ck_en; //3PLL mode
        #else
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000003 | r_dmpll2_clk_en | r_dmall_ck_en; // 1PLL async mode
        #endif
    #else
        #ifdef DDRPHY_3PLL_MODE
            #ifdef DDRPHY_2PLL
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000022  | r_dmpll2_clk_en | r_dmall_ck_en;
            #else        
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000020 | r_dmpll2_clk_en | r_dmall_ck_en; //3PLL sync mode
            #endif
        #else
            *((UINT32P)(DDRPHY_BASE + (0x0190 <<2))) = 0x00000007 | r_dmpll2_clk_en | r_dmall_ck_en; //1PLL sync mode
        #endif
    #endif

    /**********************************
    * (4) MEMPLL control switch to SPM
    ***********************************/

    // for ISO_EN zero delay
    #ifdef DDRPHY_3PLL_MODE
        *((UINT32P)(DDRPHY_BASE + (0x0173 <<2))) = 0x00101010; //[0]ISO_EN_SRC=0,[22]DIV_EN_SC_SRC=0 (pll2off),[16]DIV_EN_SRC=0,[8]PLL2_CK_EN_SRC=1(1pll),[8]PLL2_CK_EN_SRC=0(3pll)
    #else
        *((UINT32P)(DDRPHY_BASE + (0x0173 <<2))) = 0x00101010; //[0]ISO_EN_SRC=0,[22]DIV_EN_SC_SRC=0 (pll2off),[16]DIV_EN_SRC=0,[8]PLL2_CK_EN_SRC=1(1pll),[8]PLL2_CK_EN_SRC=0(3pll)
    #endif

    // setting for MEMPLL_EN control by sc_mempll3_off
    #ifdef DDRPHY_3PLL_MODE
        *((UINT32P)(DDRPHY_BASE + (0x0172 <<2))) = 0x0000F010; //[24]BIAS_EN_SRC=0,[16]BIAS_LPF_EN_SRC=0,[8]MEMPLL_EN,[9][10][11]MEMPLL2,3,4_EN_SRC,[0]ALL_CK_EN_SRC=0
    #else
        *((UINT32P)(DDRPHY_BASE + (0x0172 <<2))) = 0x0000FC10; //1PLL mode, MEMPLL3,4_EN no change to spm controller. always keep to 1'b0 for power saving.
    #endif
    *((UINT32P)(DDRPHY_BASE + (0x0170 <<2))) = 0x003C1B96; //setting for delay time
}

int mt_mempll_cali(void)
{
    int one_count = 0, zero_count = 0;
    int pll2_done = 0, pll3_done = 0, pll4_done = 0, ret = 0;

    unsigned int temp = 0, pll2_dl = 0, pll3_dl = 0, pll4_dl = 0;

    /***********************************************
    * 1. Set jitter meter clock to internal FB path
    ************************************************/
    //temp = DRV_Reg32(0x1000F60C);
    //DRV_WriteReg32(0x1000F60C, temp & ~0x200); // 0x1000F60C[9] = 0 PLL2

    //temp = DRV_Reg32(0x1000F614);
    //DRV_WriteReg32(0x1000F614, temp & ~0x200); // 0x1000F614[9] = 0 PLL3

    //temp = DRV_Reg32(0x1000F61C);
    //DRV_WriteReg32(0x1000F61C, temp & ~0x200); // 0x1000F61C[9] = 0 PLL4

    /***********************************************
    * 2. Set jitter meter count number
    ************************************************/

    temp = DRV_Reg32(0x1000F1CC) & 0x0000FFFF;
    DRV_WriteReg32(0x1000F1CC, (temp | 0x04000000)); // 0x1000F1CC[31:16] PLL2 0x400 = 1024 count

    temp = DRV_Reg32(0x1000F1D0) & 0x0000FFFF;
    DRV_WriteReg32(0x1000F1D0, (temp | 0x04000000)); // 0x1000F1D0[31:16] PLL3 0x400 = 1024 count

    temp = DRV_Reg32(0x1000F1D4) & 0x0000FFFF;
    DRV_WriteReg32(0x1000F1D4, (temp | 0x04000000)); // 0x1000F1D4[31:16] PLL4 0x400 = 1024 count

    while(1)
    { 
        /***********************************************
        * 3. Adjust delay chain tap number
        ************************************************/

        if (!pll2_done)
        {
            temp = DRV_Reg32(0x1000F610) & ~0xF8000000;
            DRV_WriteReg32(0x1000F610, (temp | (0x10 << 27 ))); // 0x1000F610[31:27] PLL2 REF 0x10 fix

            temp = DRV_Reg32(0x1000F610) & ~0x07C00000;
            DRV_WriteReg32(0x1000F610, (temp | (pll2_dl << 22))); // 0x1000F610[26:22] PLL2 FB inc 1
        }

        if (!pll3_done)
        {
            temp = DRV_Reg32(0x1000F618) & ~0xF8000000;
            DRV_WriteReg32(0x1000F618, (temp | (0x10 << 27 ))); // 0x1000F618[31:27] PLL3 REF 0x10 fix

            temp = DRV_Reg32(0x1000F618) & ~0x07C00000;
            DRV_WriteReg32(0x1000F618, (temp | (pll3_dl << 22))); // 0x1000F618[26:22] PLL3 FB inc 1
        }

        if (!pll4_done)
        {
            temp = DRV_Reg32(0x1000F620) & ~0xF8000000;
            DRV_WriteReg32(0x1000F620, (temp | (0x10 << 27 ))); // 0x1000F620[31:27] PLL4 REF 0x10 fix

            temp = DRV_Reg32(0x1000F620) & ~0x07C00000;
            DRV_WriteReg32(0x1000F620, (temp | (pll4_dl << 22))); // 0x1000F620[26:22] PLL4 FB inc 1
        }

        /***********************************************
        * 4. Enable jitter meter
        ************************************************/

        if (!pll2_done)
        {
            temp = DRV_Reg32(0x1000F1CC);
            DRV_WriteReg32(0x1000F1CC, temp | 0x1); // 0x1000F1CC[0]=1 PLL2
        }

        if (!pll3_done)
        {
            temp = DRV_Reg32(0x1000F1D0);
            DRV_WriteReg32(0x1000F1D0, temp | 0x1); // 0x1000F1D0[0]=1 PLL3
        }

        if (!pll4_done)
        {
            temp = DRV_Reg32(0x1000F1D4);
            DRV_WriteReg32(0x1000F1D4, temp | 0x1); // 0x1000F1D4[0]=1 PLL4
        }

        gpt_busy_wait_us(40); // wait for jitter meter complete

        /***********************************************
        * 5. Check jitter meter counter value
        ************************************************/

        if (!pll2_done)
        {
            one_count = DRV_Reg32(0x1000F320) >> 16; // 0x1000F320[31:16] PLL2 one count
            zero_count = DRV_Reg32(0x1000F320) & 0x0000FFFF; // 0x1000F320[15:0] PLL2 zero count

            if (zero_count > 512)
            {
                //print("[mt_pll_init] PLL2 FB_DL: 0x%x, 1/0 = %d/%d\n", ((DRV_Reg32(0x1000F610)& 0x07C00000) >> 22), one_count, zero_count);
                pll2_done = 1;
            }
        }

        if (!pll3_done)
        {
            one_count = DRV_Reg32(0x1000F324) >> 16; // 0x1000F324[31:16] PLL3 one count
            zero_count = DRV_Reg32(0x1000F324) & 0x0000FFFF; // 0x1000F324[15:0] PLL3 zero count

            if (zero_count > 512)
            {
                //print("[mt_pll_init] PLL3 FB_DL: 0x%x, 1/0 = %d/%d\n", ((DRV_Reg32(0x1000F618) & 0x07C00000) >> 22), one_count, zero_count);
                pll3_done = 1;
            }
        }

        if (!pll4_done)
        {
            one_count = DRV_Reg32(0x1000F328) >> 16; // 0x1000F328[31:16] PLL4 one count
            zero_count = DRV_Reg32(0x1000F328) & 0x0000FFFF; // 0x1000F328[15:0] PLL4 zero count

            if (zero_count > 512)
            {
                //print("[mt_pll_init] PLL4 FB_DL: 0x%x, 1/0 = %d/%d\n", ((DRV_Reg32(0x1000F620) & 0x07C00000) >> 22), one_count, zero_count);
                pll4_done = 1;
            }
        }

        /***********************************************
        * 6. Reset jitter meter value
        ************************************************/

        if (!pll2_done)
        {
            pll2_dl++;
            temp = DRV_Reg32(0x1000F1CC);
            DRV_WriteReg32(0x1000F1CC, temp & ~0x1); // 0x1000F1CC[0]=0 PLL2
        }

        if (!pll3_done)
        {
            pll3_dl++;
            temp = DRV_Reg32(0x1000F1D0);
            DRV_WriteReg32(0x1000F1D0, temp & ~0x1); // 0x1000F1D0[0]=0 PLL3
        }

        if (!pll4_done)
        {
            pll4_dl++;
            temp = DRV_Reg32(0x1000F1D4);
            DRV_WriteReg32(0x1000F1D4, temp & ~0x1); // 0x1000F1D4[0]=0 PLL4
        }

        /*************************************************************
        * Then return to step 1 to adjust next delay chain tap value.
        * Until we have ~ 50% of one or zero count on jitter meter
        **************************************************************/

        if (pll2_done && pll3_done && pll4_done)
        {
            ret = 0;
            break;
        }

        if (pll2_dl >= 32 || pll3_dl >= 32 || pll4_dl >= 32)
        {
            ret = -1;
            break;
        }
    }

    #if 0
    if (ret != 0)
        print("[mt_pll_init] MEMPLL 3PLL mode calibration fail\n");
    #endif 

    /***********************************************
    * 7. Set jitter meter clock to external FB path
    ************************************************/

    temp = DRV_Reg32(0x1000F60C);
    DRV_WriteReg32(0x1000F60C, temp | 0x200); // 0x1000F60C[9] = 1 PLL2

    temp = DRV_Reg32(0x1000F614);
    DRV_WriteReg32(0x1000F614, temp | 0x200); // 0x1000F614[9] = 1 PLL3

    temp = DRV_Reg32(0x1000F61C);
    DRV_WriteReg32(0x1000F61C, temp | 0x200); // 0x1000F61C[9] = 1 PLL4

    return ret;
}

unsigned int mt_get_cpu_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_8, clk_misc_cfg_1, clk26cali_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    DRV_WriteReg32(CLK_MISC_CFG_1, 0xFFFF0300); // select divider

    clk_cfg_8 = DRV_Reg32(CLK_CFG_8);
    DRV_WriteReg32(CLK_CFG_8, (39 << 8)); // select abist_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, temp | 0x1); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x1)
    {
        print("wait for frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        //mdelay(10);
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;

    output = ((temp * 26000) / 1024) * 4; // Khz

    DRV_WriteReg32(CLK_CFG_8, clk_cfg_8);
    DRV_WriteReg32(CLK_MISC_CFG_1, clk_misc_cfg_1);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);

    //print("CLK26CALI = 0x%x, cpu frequency = %d Khz\n", temp, output);

    return output;
}

unsigned int mt_get_mem_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_8, clk_misc_cfg_1, clk26cali_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    DRV_WriteReg32(CLK_MISC_CFG_1, 0xFFFFFF00); // select divider

    clk_cfg_8 = DRV_Reg32(CLK_CFG_8);
    DRV_WriteReg32(CLK_CFG_8, (14 << 8)); // select abist_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, temp | 0x1); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x1)
    {
        print("wait for frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        //mdelay(10);
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;

    output = (temp * 26000) / 1024; // Khz

    DRV_WriteReg32(CLK_CFG_8, clk_cfg_8);
    DRV_WriteReg32(CLK_MISC_CFG_1, clk_misc_cfg_1);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);

    //print("CLK26CALI = 0x%x, mem frequency = %d Khz\n", temp, output);

    return output;
}

unsigned int mt_get_bus_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_9, clk_misc_cfg_1, clk26cali_2;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    DRV_WriteReg32(CLK_MISC_CFG_1, 0x00FFFFFF); // select divider

    clk_cfg_9 = DRV_Reg32(CLK_CFG_9);
    DRV_WriteReg32(CLK_CFG_9, (1 << 16)); // select ckgen_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, temp | 0x10); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x10)
    {
        print("wait for frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        //mdelay(10);
    }

    temp = DRV_Reg32(CLK26CALI_2) & 0xFFFF;

    output = (temp * 26000) / 1024; // Khz

    DRV_WriteReg32(CLK_CFG_9, clk_cfg_9);
    DRV_WriteReg32(CLK_MISC_CFG_1, clk_misc_cfg_1);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);

    //print("CLK26CALI = 0x%x, bus frequency = %d Khz\n", temp, output);

    return output;
}

void mt_pll_post_init(void)
{
    print("mt_pll_post_init: mt_get_cpu_freq = %dKhz\n", mt_get_cpu_freq());
    print("mt_pll_post_init: mt_get_bus_freq = %dKhz\n", mt_get_bus_freq());
    print("mt_pll_post_init: mt_get_mem_freq = %dKhz\n", mt_get_mem_freq());
    
    #if 0
    print("mt_pll_post_init: AP_PLL_CON0        = 0x%x, GS = 0x00000133\n", DRV_Reg32(AP_PLL_CON0));
    print("mt_pll_post_init: AP_PLL_CON1        = 0x%x, GS = 0x00000000\n", DRV_Reg32(AP_PLL_CON1));
    print("mt_pll_post_init: AP_PLL_CON2        = 0x%x, GS = 0x00000000\n", DRV_Reg32(AP_PLL_CON2));
    print("mt_pll_post_init: AP_PLL_CON3        = 0x%x, GS = 0x00780000\n", DRV_Reg32(AP_PLL_CON3));
    print("mt_pll_post_init: PLL_HP_CON0        = 0x%x, GS = 0x0000003F\n", DRV_Reg32(PLL_HP_CON0));
    print("mt_pll_post_init: CLKSQ_STB_CON0     = 0x%x, GS = 0x00050001\n", DRV_Reg32(CLKSQ_STB_CON0));
    print("mt_pll_post_init: PLL_PWR_CON0       = 0x%x, GS = 0x00033333\n", DRV_Reg32(PLL_PWR_CON0));
    print("mt_pll_post_init: PLL_PWR_CON1       = 0x%x, GS = 0x00011111\n", DRV_Reg32(PLL_PWR_CON1));
    print("mt_pll_post_init: PLL_ISO_CON0       = 0x%x, GS = 0x00088888\n", DRV_Reg32(PLL_ISO_CON0));
    print("mt_pll_post_init: PLL_ISO_CON1       = 0x%x, GS = 0x00011111\n", DRV_Reg32(PLL_ISO_CON1));
    print("mt_pll_post_init: PLL_STB_CON0       = 0x%x, GS = 0x00066666\n", DRV_Reg32(PLL_STB_CON0));
    print("mt_pll_post_init: DIV_STB_CON0       = 0x%x, GS = 0x00969696\n", DRV_Reg32(DIV_STB_CON0));
    print("mt_pll_post_init: PLL_TEST_CON0      = 0x%x, GS = 0xFF000000\n", DRV_Reg32(PLL_TEST_CON0));
    print("mt_pll_post_init: ARMPLL_CON0        = 0x%x, GS = 0x00000101\n", DRV_Reg32(ARMPLL_CON0));
    print("mt_pll_post_init: ARMPLL_CON1        = 0x%x, GS = 0x800A189B\n", DRV_Reg32(ARMPLL_CON1));
    print("mt_pll_post_init: ARMPLL_PWR_CON0    = 0x%x, GS = 0x00000001\n", DRV_Reg32(ARMPLL_PWR_CON0));
    print("mt_pll_post_init: MAINPLL_CON0       = 0x%x, GS = 0x79000101\n", DRV_Reg32(MAINPLL_CON0));
    print("mt_pll_post_init: MAINPLL_CON1       = 0x%x, GS = 0x800A8000\n", DRV_Reg32(MAINPLL_CON1));
    print("mt_pll_post_init: MAINPLL_PWR_CON0   = 0x%x, GS = 0x00000001\n", DRV_Reg32(MAINPLL_PWR_CON0));
    print("mt_pll_post_init: UNIVPLL_CON0       = 0x%x, GS = 0xFD000001\n", DRV_Reg32(UNIVPLL_CON0));
    print("mt_pll_post_init: UNIVPLL_CON1       = 0x%x, GS = 0x800C0000\n", DRV_Reg32(UNIVPLL_CON1));
    print("mt_pll_post_init: UNIVPLL_PWR_CON0   = 0x%x, GS = 0x00000001\n", DRV_Reg32(UNIVPLL_PWR_CON0));
    print("mt_pll_post_init: MMPLL_CON0         = 0x%x, GS = 0x00000121\n", DRV_Reg32(MMPLL_CON0));
    print("mt_pll_post_init: MMPLL_CON1         = 0x%x, GS = 0x80134000\n", DRV_Reg32(MMPLL_CON1));
    print("mt_pll_post_init: MMPLL_PWR_CON0     = 0x%x, GS = 0x00000001\n", DRV_Reg32(MMPLL_PWR_CON0));
    print("mt_pll_post_init: MSDCPLL_CON0       = 0x%x, GS = 0x00000121\n", DRV_Reg32(MSDCPLL_CON0));
    print("mt_pll_post_init: MSDCPLL_CON1       = 0x%x, GS = 0x800F6275\n", DRV_Reg32(MSDCPLL_CON1));
    print("mt_pll_post_init: MSDCPLL_PWR_CON0   = 0x%x, GS = 0x00000001\n", DRV_Reg32(MSDCPLL_PWR_CON0));
    print("mt_pll_post_init: VENCPLL_CON0       = 0x%x, GS = 0x00000001\n", DRV_Reg32(VENCPLL_CON0));
    print("mt_pll_post_init: VENCPLL_CON1       = 0x%x, GS = 0x800B6000\n", DRV_Reg32(VENCPLL_CON1));
    print("mt_pll_post_init: VENCPLL_PWR_CON0   = 0x%x, GS = 0x00000001\n", DRV_Reg32(VENCPLL_PWR_CON0));
    #endif
}



void mt_pll_init(void)
{
    int ret = 0;
    unsigned int temp;

    DRV_WriteReg32(ACLKEN_DIV, 0x12); // MCU Bus DIV2

    DRV_WriteReg32(CLKSQ_STB_CON0, 0x00050001); // reduce CLKSQ disable time

    DRV_WriteReg32(PLL_ISO_CON0, 0x00088888); // extend PWR/ISO control timing to 1us

    DRV_WriteReg32(AP_PLL_CON3, 0x00780000); // set CHG bypass delay

    //DRV_WriteReg32(ACLKEN_DIV, 0x12); // MCU Bus DIV2

    /*************
    * xPLL PWR ON 
    **************/

    temp = DRV_Reg32(ARMPLL_PWR_CON0);
    DRV_WriteReg32(ARMPLL_PWR_CON0, temp | 0x1);

    temp = DRV_Reg32(MAINPLL_PWR_CON0);
    DRV_WriteReg32(MAINPLL_PWR_CON0, temp | 0x1);

    temp = DRV_Reg32(VENCPLL_PWR_CON0);
    DRV_WriteReg32(VENCPLL_PWR_CON0, temp | 0x1);

    temp = DRV_Reg32(MMPLL_PWR_CON0);
    DRV_WriteReg32(MMPLL_PWR_CON0, temp | 0x1);

    temp = DRV_Reg32(UNIVPLL_PWR_CON0);
    DRV_WriteReg32(UNIVPLL_PWR_CON0, temp | 0x1);

    temp = DRV_Reg32(MSDCPLL_PWR_CON0);
    DRV_WriteReg32(MSDCPLL_PWR_CON0, temp | 0x1);

    gpt_busy_wait_us(5); // wait for xPLL_PWR_ON ready (min delay is 1us)


    /******************
    * xPLL ISO Disable
    *******************/

    temp = DRV_Reg32(ARMPLL_PWR_CON0);
    DRV_WriteReg32(ARMPLL_PWR_CON0, temp & ~0x2);

    temp = DRV_Reg32(MAINPLL_PWR_CON0);
    DRV_WriteReg32(MAINPLL_PWR_CON0, temp & ~0x2);

    temp = DRV_Reg32(VENCPLL_PWR_CON0);
    DRV_WriteReg32(VENCPLL_PWR_CON0, temp & ~0x2);

    temp = DRV_Reg32(MMPLL_PWR_CON0);
    DRV_WriteReg32(MMPLL_PWR_CON0, temp & ~0x2);

    temp = DRV_Reg32(UNIVPLL_PWR_CON0);
    DRV_WriteReg32(UNIVPLL_PWR_CON0, temp & ~0x2);

    temp = DRV_Reg32(MSDCPLL_PWR_CON0);
    DRV_WriteReg32(MSDCPLL_PWR_CON0, temp & ~0x2);


    /********************
    * xPLL Frequency Set
    *********************/

    #if 0
    DRV_WriteReg32(ARMPLL_CON1, 0x8109A000); // 500.5Mhz
    #endif
    #if 1
    DRV_WriteReg32(ARMPLL_CON1, 0x800A0000); // 1040Mhz
    #endif
    #if 0
    DRV_WriteReg32(ARMPLL_CON1, 0x800A189B); // 1050Mhz
    #endif
    #if 0
    DRV_WriteReg32(ARMPLL_CON1, 0x800A8000); // 1092Mhz
    #endif

    #ifdef SLT
    DRV_WriteReg32(ARMPLL_CON1, 0x800C8000); // 1300Mhz
    #endif

    DRV_WriteReg32(MAINPLL_CON1, 0x800A8000);

    temp = ((~(DRV_Reg32(VENCPLL_CON1) & 0x80000000)) & 0x80000000);
    DRV_WriteReg32(VENCPLL_CON1, temp | 0x000B6000);

    //DRV_WriteReg32(MMPLL_CON1, 0x80134000);
    DRV_WriteReg32(MMPLL_CON1, 0x8009A000);

    DRV_WriteReg32(MSDCPLL_CON1, 0x800F6275);

    /***********************
    * xPLL Frequency Enable
    ************************/

    temp = DRV_Reg32(ARMPLL_CON0);
    DRV_WriteReg32(ARMPLL_CON0, temp | 0x1);

    temp = DRV_Reg32(MAINPLL_CON0);
    DRV_WriteReg32(MAINPLL_CON0, temp & ~0x70); // restore MAINPLL_POSDIV = 0

    temp = DRV_Reg32(MAINPLL_CON0);
    DRV_WriteReg32(MAINPLL_CON0, temp | 0x1);

    temp = DRV_Reg32(VENCPLL_CON0);
    DRV_WriteReg32(VENCPLL_CON0, temp | 0x1);

    temp = DRV_Reg32(MMPLL_CON0);
    //DRV_WriteReg32(MMPLL_CON0, (temp & ~0x70) | 0x20);  // MMPLL_POSDIV = 2
    DRV_WriteReg32(MMPLL_CON0, (temp & ~0x70) | 0x10);  // MMPLL_POSDIV = 1

    temp = DRV_Reg32(MMPLL_CON0);
    DRV_WriteReg32(MMPLL_CON0, temp | 0x1);

    temp = DRV_Reg32(UNIVPLL_CON0);
    DRV_WriteReg32(UNIVPLL_CON0, temp | 0x1);

    temp = DRV_Reg32(MSDCPLL_CON0);
    DRV_WriteReg32(MSDCPLL_CON0, temp | 0x1);

    gpt_busy_wait_us(40); // wait for PLL stable (min delay is 20us)


    /***************
    * xPLL DIV RSTB
    ****************/

    temp = DRV_Reg32(MAINPLL_CON0);
    DRV_WriteReg32(MAINPLL_CON0, temp | 0x01000000);

    temp = DRV_Reg32(UNIVPLL_CON0);
    DRV_WriteReg32(UNIVPLL_CON0, temp | 0x01000000);


    /*****************
    * xPLL HW Control
    ******************/

    temp = DRV_Reg32(AP_PLL_CON1);
    //DRV_WriteReg32(AP_PLL_CON1, temp & 0xFF00FC0C);
    DRV_WriteReg32(AP_PLL_CON1, temp & 0xFFCCFDCC); // UNIVPLL, MMPLL SW Control
    //DRV_WriteReg32(AP_PLL_CON1, temp & 0xFFEEFFEC); // UNIVPLL, MMPLL, MAINPLL SW Control

    temp = DRV_Reg32(AP_PLL_CON2);
    //DRV_WriteReg32(AP_PLL_CON2, temp & 0xFFFFFFF8);
    DRV_WriteReg32(AP_PLL_CON2, temp & 0xFFFFFFFC); // UNIVPLL, MMPLL SW Control
    //DRV_WriteReg32(AP_PLL_CON2, temp & 0xFFFFFFFE); // UNIVPLL, MMPLL, MAINPLL SW Control


    /*************
    * MEMPLL Init
    **************/

    mt_mempll_init();

    /* MEMPLL Calibration */
    #ifdef DDRPHY_3PLL_MODE
        ret = mt_mempll_cali();
    #endif

    /**************
    * INFRA CLKMUX
    ***************/

    temp = DRV_Reg32(TOP_DCMCTL);
    DRV_WriteReg32(TOP_DCMCTL, temp | 0x1); // enable infrasys DCM

    DRV_WriteReg32(TOP_CKDIV1, 0x0); // CPU clock divide by 1

    temp = DRV_Reg32(TOP_CKMUXSEL) & ~0xC;
    DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x4); // switch CPU clock to ARMPLL


    /************
    * TOP CLKMUX
    *************/

    DRV_WriteReg32(CLK_CFG_0, 0x01010101);

    DRV_WriteReg32(CLK_CFG_1, 0x00010100); // cmtg = pwm = 26Mhz

    DRV_WriteReg32(CLK_CFG_2, 0x01010100); // uart = 26Mhz

    DRV_WriteReg32(CLK_CFG_3, 0x01000101); // audio = 26Mhz

    DRV_WriteReg32(CLK_CFG_4, 0x01030100); // scp = 66Mhz, pmic = 26Mhz

    DRV_WriteReg32(CLK_SCP_CFG_0, 0x3FF); // enable scpsys clock off control

    DRV_WriteReg32(CLK_SCP_CFG_1, 0x11); // enable scpsys clock off control

    /*for MTCMOS*/
    spm_mtcmos_ctrl_disp(STA_POWER_ON);
    
    /*turn on DISP*/
    DRV_WriteReg32(DISP_CG_CLR0, 0x7FFFF);
    DRV_WriteReg32(DISP_CG_CLR1, 0xF);

    /*Turn on LARB0 OSTD*/
    temp = DRV_Reg32(SMI_LARB0_STAT);
    if(0 == temp)
    {
        DRV_WriteReg32(SMI_LARB0_OSTD_CTRL_EN , 0x1ff);//Turn on the OSTD on LARB0
    }
    else
    {
        print("LARB0 is busy , cannot set OSTD 0x%x\n" , temp);
    }

    /*Cautions !!! 
      If more MM engines will be enabled in preloader other than LARB0, 
      please clear LARB clock gate
      and set corresponded LARB# OSTD as following
    DRV_WriteReg32(SMI_LARB1_OSTD_CTRL_EN , 0xffffffff);
    DRV_WriteReg32(SMI_LARB2_OSTD_CTRL_EN , 0xffffffff);
    */
    
}


int spm_mtcmos_ctrl_disp(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | SRAM_PDN);
#if 0
        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK) != DIS_SRAM_ACK) {
        }
#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_DIS_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_DIS_PWR_CON, val);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)) {
        }
#endif
    } else {    /* STA_POWER_ON */
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK) 
                || !(spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)) {
        }
#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~SRAM_PDN);

#if 0
        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK)) {
        }
#endif

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
    }

    return err;
}
