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

#include <typedefs.h>
#include <platform.h>
#include <emi.h>
#include <dramc.h>
#include <platform.h>
#include <pll.h>
#include "wdt.h"
#include "custom_emi.h"


// Macro definition.
/* These PLL config is defined in pll.h */
//#define DDRPHY_3PLL_MODE
#define REAL_MEMPLL_MDL
/* To define DDRTYPE for bring up */
#define DDRTYPE_LPDDR2	
//#define DDRTYPE_LPDDR3
//#define DDRTYPE_DDR3

#if 0
#define APMIXED_BASE 			(0x10209000)
#ifdef AP_PLL_CON1
#undef AP_PLL_CON1
#endif
#ifdef AP_PLL_CON2
#undef AP_PLL_CON2
#endif
#define AP_PLL_CON1	((volatile unsigned int *)(APMIXED_BASE+0x0004))
#define AP_PLL_CON2	((volatile unsigned int *)(APMIXED_BASE+0x0008))
#endif

static int enable_combo_dis = 0;
#define REPAIR_SRAM
#ifdef REPAIR_SRAM
extern int repair_sram(void);
#endif

extern int num_of_emi_records;
extern EMI_SETTINGS emi_settings[];
extern u32 g_secure_dram_size;

#define COMBO_MCP
#ifndef COMBO_MCP
#ifdef DDRTYPE_LPDDR2
#define    EMI_CONA_VAL         0x0002A3AE
#define    DRAMC_ACTIM_VAL      0x44584493
#define    DRAMC_GDDR3CTL1_VAL  0x01000000
#define    DRAMC_CONF1_VAL      0xF0048683
#define    DRAMC_DDR2CTL_VAL    0xA00632D1
#define    DRAMC_MODE_REG_63    0x0000003F
#define    DRAMC_MODE_REG_10    0x00FF000A
#define    DRAMC_MODE_REG_1     0x00C30001
#define    DRAMC_MODE_REG_2     0x00060002
#define    DRAMC_MODE_REG_3     0x00020003
#define    DRAMC_TEST2_3_VAL    0xBF080401
#define    DRAMC_CONF2_VAL      0x0340633F
#define    DRAMC_PD_CTRL_VAL    0x51642342
#define    DRAMC_ACTIM1_VAL     0x01000510
#define    DRAMC_ACTIM05T_VAL   0x04002600
#define    DRAMC_MISCTL0_VAL    0x07800000


#define DRAMC_DRVCTL0_VAL 0xAA00AA00    /* not set in 6582 init code due to depending on waveform when chip is back. TX I/O driving = 0xA */
#define DRAMC_DRVCTL1_VAL 0xAA00AA00    /* not set in 6582 init code due to depending on waveform when chip is back. TX I/O driving = 0xA */
#define DRAMC_PADCTL3_VAL 0x0 /* not set in 6582 init code due to setting in calibration */
#define DRAMC_DQODLY_VAL 0xEEEEEEEE
#define CMD_ADDR_OUTPUT_DLY 0x0
#define CLK_OUTPUT_DLY 0x0
#endif

#ifdef DDRTYPE_LPDDR3
#define    EMI_CONA_VAL         0x2A3AE
#define    DRAMC_ACTIM_VAL      0x44584493
#define    DRAMC_GDDR3CTL1_VAL  0x01000000
#define    DRAMC_CONF1_VAL      0xF0048683
#define    DRAMC_DDR2CTL_VAL    0xA00632F1
#define    DRAMC_MODE_REG_63    0x0000003F
#define    DRAMC_MODE_REG_10    0x00FF000A
#define    DRAMC_MODE_REG_1     0x00C30001
#define    DRAMC_MODE_REG_2     0x00060002
#define    DRAMC_MODE_REG_3     0x00020003
#define    DRAMC_TEST2_3_VAL    0xBF080401
#define    DRAMC_CONF2_VAL      0x0340633F
#define    DRAMC_PD_CTRL_VAL    0x51642342
#define    DRAMC_ACTIM1_VAL     0x11000510
#define    DRAMC_ACTIM05T_VAL   0x04002600
#define    DRAMC_MISCTL0_VAL    0x07800000

#define DRAMC_PADCTL3_VAL 0x0 /* not set in 6582 init code due to setting in calibration */
#define DRAMC_DQODLY_VAL 0xEEEEEEEE
#define CMD_ADDR_OUTPUT_DLY 0x0
#define CLK_OUTPUT_DLY 0x0
#define DRAMC_DRVCTL0_VAL 0xAA22AA22    /* not set in 6582 init code due to depending on waveform when chip is back. TX I/O driving = 0xA */
#define DRAMC_DRVCTL1_VAL 0xAA22AA22    /* not set in 6582 init code due to depending on waveform when chip is back. TX I/O driving = 0xA */
#endif 

#endif

/* select the corresponding memory devices */
extern int dramc_calib(void);

/*
 * init_dram: Do initialization for LPDDR.
 */
static void init_lpddr1(EMI_SETTINGS *emi_setting) 
{
    return;
}
EMI_SETTINGS emi_setting_default_lpddr2 =
{

        //default
                0x0,            /* sub_version */
                0x0002,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x0000212E,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0x51642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x10000000,0,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000006,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
};
EMI_SETTINGS emi_setting_default_lpddr3 =
{

        //default
                0x0,            /* sub_version */
                0x0003,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x0000212E,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632F1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0x51642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x11000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x10000000,0,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */
                0x00C30001,             /* LPDDR3_MODE_REG1 */
                0x00060002,             /* LPDDR3_MODE_REG2 */
                0x00020003,             /* LPDDR3_MODE_REG3 */
                0x00000006,             /* LPDDR3_MODE_REG5 */
                0x00FF000A,             /* LPDDR3_MODE_REG10 */
                0x0000003F,             /* LPDDR3_MODE_REG63 */
};
static int mt_get_dram_type_for_dis(void)
{
    int i;
    int type = 2;
    type = (emi_settings[0].type & 0xF);
    for (i = 0 ; i < num_of_emi_records; i++)
    {
      //print("[EMI][%d] type%d\n",i,type);
      if (type != (emi_settings[0].type & 0xF))
      {
          print("It's not allow to combine two type dram when combo discrete dram enable\n");
          ASSERT(0);
          break;
      }
    }
    return type;
}

unsigned int DRAM_MRR(int MRR_num)
{
    unsigned int MRR_value = 0x0;
    unsigned int bak_1e8;
    unsigned int MRR_DQ_map[3];
    unsigned int dram_type;
    bak_1e8 = DRAMC_READ_REG(0x01E8);

    dram_type = mt_get_dram_type_for_dis();

    if (TYPE_LPDDR2 == dram_type)
    {
        MRR_DQ_map[0] = 0x000A0B09;
        MRR_DQ_map[1] = 0x000E0C08;
        MRR_DQ_map[2] = 0x00000D0F;
    }
    else if (TYPE_LPDDR3 == dram_type)
    {
        MRR_DQ_map[0] = 0x00080B09;
        MRR_DQ_map[1] = 0x000D0F0A;
        MRR_DQ_map[2] = 0x00000C0E;
    }

    //DRAMC_WRITE_CLEAR(0x00320000,0x1E8);
    //Setup DQ swap for bit[2:0]
    DRAMC_WRITE_REG(MRR_DQ_map[0],0x1F4);

    //Set MRSMA
    DRAMC_WRITE_REG(MRR_num,0x088);

    /*Enable MRR read for reading LPDDR2 refresh rate*/
    DRAMC_WRITE_SET(0x00010000,0x1E8);
    print("Enable MRR read: 0x1E8:%x\n",DRAMC_READ_REG(0x01E8));
    gpt_busy_wait_us(1000);//Wait > 1000000us
    MRR_value = (DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8;
    print("0x1f4:%x,refresh_rate[10:8]:%d\n",DRAMC_READ_REG(0x01F4),(DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8);

    //Setup DQ swap for bit[5:3]
    DRAMC_WRITE_REG(MRR_DQ_map[1],0x1F4);

    gpt_busy_wait_us(1000);//Wait > 1000000us
    MRR_value |= ((DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8) << 0x3;
    print("0x1f4:%x,refresh_rate[10:8]:%d\n",DRAMC_READ_REG(0x01F4),(DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8);
    //Setup DQ swap for bit[9:6]
    DRAMC_WRITE_REG(MRR_DQ_map[2],0x1F4);

    gpt_busy_wait_us(1000);//Wait > 1000000us
    MRR_value |= (((DRAMC_READ_REG(0x03B8) & 0x000000300) >> 8) << 0x6);
    print("0x1f4:%x,refresh_rate[10:8]:%d\n",DRAMC_READ_REG(0x01F4),(DRAMC_READ_REG(0x03B8) & 0x000000300) >> 8);
    //print("MRR_value:%x\n",MRR_value);
    /*Disable MRR read for reading LPDDR2 refresh rate*/
    DRAMC_WRITE_REG(bak_1e8,0x1E8);
    print("Disable MRR read: 0x1E8:%x\n",DRAMC_READ_REG(0x01E8));

    /* reset infra*/
    print("Before infra reset, 0x3B8:%x,0x10001034:%x\n",(((DRAMC_READ_REG(0x03B8) & 0x000000300) >> 8) ),*(volatile unsigned *)(0x10001034));
    *(volatile unsigned *)(0x10001034) |= (1 << 0x2);
    *(volatile unsigned *)(0x10001034) &= ~(1 << 0x2);

    *(volatile unsigned *)(0x100040E4) |= (1 << 0x2);
    *(volatile unsigned *)(0x100040E4) &= ~(1 << 0x2);
    print("After infra reset, 0x3B8:%x,0x10001034:%x\n",(((DRAMC_READ_REG(0x03B8) & 0x000000300) >> 8) ),*(volatile unsigned *)(0x10001034));

 
    return MRR_value;
}


#define TOPRGU_BASE                 RGU_BASE
#define UNLOCK_KEY_MODE (0x22000000)
#define UNLOCK_KEY_SWRST (0x00001209)
#define UNLOCK_KEY_DEBUG_CTL (0x59000000)
#define TIMEOUT 3
extern u32 g_ddr_reserve_enable;
extern u32 g_ddr_reserve_success;


// EMI address definition.
#define EMI_CONA 		((volatile unsigned int *)(EMI_BASE+0x000))
#define EMI_CONB		((volatile unsigned int *)(EMI_BASE+0x008))
#define EMI_CONC		((volatile unsigned int *)(EMI_BASE+0x010))
#define EMI_COND		((volatile unsigned int *)(EMI_BASE+0x018))
#define EMI_CONE		((volatile unsigned int *)(EMI_BASE+0x020))
#define EMI_CONF                ((volatile unsigned int *)(EMI_BASE+0x028))
#define EMI_CONG		((volatile unsigned int *)(EMI_BASE+0x030))
#define EMI_CONH		((volatile unsigned int *)(EMI_BASE+0x038))
#define EMI_CONM		((volatile unsigned int *)(EMI_BASE+0x060))
#define EMI_DFTB		((volatile unsigned int *)(EMI_BASE+0x0E8))		// TESTB
#define EMI_DFTD		((volatile unsigned int *)(EMI_BASE+0x0F8))		// TESTD
#define EMI_SLCT		((volatile unsigned int *)(EMI_BASE+0x158))
#define EMI_ARBA		((volatile unsigned int *)(EMI_BASE+0x100))
#define EMI_ARBB		((volatile unsigned int *)(EMI_BASE+0x108))
#define EMI_ARBC		((volatile unsigned int *)(EMI_BASE+0x110))
#define EMI_ARBD		((volatile unsigned int *)(EMI_BASE+0x118))
#define EMI_ARBE		((volatile unsigned int *)(EMI_BASE+0x120))
#define EMI_ARBF		((volatile unsigned int *)(EMI_BASE+0x128))
#define EMI_ARBG		((volatile unsigned int *)(EMI_BASE+0x130))
#define EMI_ARBI		((volatile unsigned int *)(EMI_BASE+0x140))
#define EMI_BMEN		((volatile unsigned int *)(EMI_BASE+0x400))

typedef volatile unsigned int *V_UINT32P;
/*
 * init_lpddr2: Do initialization for LPDDR2.
 */
static void init_lpddr2(EMI_SETTINGS *emi_setting) 
{
#if COMBO_LPDDR2 > 0
        print("[EMI]in init_lpddr2\n");
	// The following settings are from Yi-Chih.
	/* TINFO="Star mm_cosim_emi_config" */
	//----------------EMI Setting--------------------
	*((V_UINT32P)(EMI_BASE   +0x0060))=0x0000051f;
	*((V_UINT32P)(EMI_BASE   +0x0140))=0x20406188;//0x12202488;   // 83 for low latency
	*((V_UINT32P)(EMI_BASE   +0x0144))=0x20406188;//0x12202488; //new add
	*((V_UINT32P)(EMI_BASE   +0x0100))=0x40105806;//0x40105808; //m0 cpu ori:0x8020783f
	*((V_UINT32P)(EMI_BASE   +0x0108))=0x808050ea;//0xa0a05028; //m1 ori:0x80200000 // ultra can over limit
	*((V_UINT32P)(EMI_BASE   +0x0110))=0xffff50c4;//0xa0a05001; //m2 arm9
	*((V_UINT32P)(EMI_BASE   +0x0118))=0x0810d84a;//0x030fd80d; // ???????????????????????????????????????????????????????????//m3 mdmcu ori:0x80807809 ori:0x07007010 bit[12]:enable bw limiter
	*((V_UINT32P)(EMI_BASE   +0x0120))=0x40405042; //m4 fcore ori:0x8080781a
	// *((V_UINT32P)(EMI_BASE   +0x0128))=0xa0a05001; //m5 peri
	// *((V_UINT32P)(EMI_BASE   +0x0130))=0xa0a05028; //m6 vcodec ori:0x8080381a
	*((V_UINT32P)(EMI_BASE   +0x0148))=0x9719595e;//0323 chg, ori :0x00462f2f
	*((V_UINT32P)(EMI_BASE   +0x014c))=0x9719595e; // new add
	//-----------------------------------
	//-- modified by liya for EMI address mapping setting
	*EMI_CONA = emi_setting->EMI_CONA_VAL;
	//*((UINT32P)(EMI_BASE   +0x0000))=0x1130112e;
	//-----------------------------------
	*((V_UINT32P)(EMI_BASE   +0x00f8))=0x00000000;
	*((V_UINT32P)(EMI_BASE   +0x0400))=0x007f0001;
	*((V_UINT32P)(EMI_BASE   +0x0008))=0x05121624;// Chuan:0x181e1e2b Edwin:0x12161620
	*((V_UINT32P)(EMI_BASE   +0x0010))=0x04070506;// Chuan:0x18181818 Edwin:0x12121212
	*((V_UINT32P)(EMI_BASE   +0x0018))=0x05121624;// Chuan:0x1e262b33 Edwin:0x161c2026
	*((V_UINT32P)(EMI_BASE   +0x0020))=0x04070506;// Chuan:0x1818181b Edwin:0x12121214
	*((V_UINT32P)(EMI_BASE   +0x0030))=0x04050516;
	*((V_UINT32P)(EMI_BASE   +0x0038))=0x04050516;
	*((V_UINT32P)(EMI_BASE   +0x0158))=0x00010800;// ???????????????????????0x08090800; 
	*((V_UINT32P)(EMI_BASE   +0x0150))=0x6470fc79;
	*((V_UINT32P)(EMI_BASE   +0x0154))=0x6470fc79;
	*((V_UINT32P)(EMI_BASE   +0x00e8))=0x00000100;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
	//============== Defer WR threthold
	*((V_UINT32P)(EMI_BASE   +0x00f0))=0x38470000;
	//============== Reserve bufer
	//
	//MDMCU don't always ULTRA, but small age
	*((V_UINT32P)(EMI_BASE   +0x0078))=0x23110003;// ???????????0x00030F4d; 
	// Turn on M1 Ultra and all port DRAMC hi enable
	*((V_UINT32P)(EMI_BASE   +0x0158))=0x1f011f00;// ???????????????????????0x08090800; 
	// RFF)_PBC_MASK
	*((V_UINT32P)(EMI_BASE   +0x00e8))=0x00000004;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
	// Page hit is high
	*((V_UINT32P)(EMI_BASE   +0x0060))=0x0000051f;
	*((V_UINT32P)(EMI_BASE   +0x00d0))=0x84848c84;//R/8 W/8 outstanding
	*((V_UINT32P)(EMI_BASE   +0x00d8))=0x00480084;//R/8 W/8 outstanding
	//===========END===========================================

#ifdef DRAMC_ASYNC
	*((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;  //ASYNC   // Edward : Modify MODE18V from 1 (1.8V) to 0(1.2V).
	*((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;  //ASYNC    // Edward : TXP value is 2? 0??
#else
	*((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;
	*((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;
#endif

	*((V_UINT32P)(DRAMC0_BASE + 0x0048)) = 0x0001110d;
	*((V_UINT32P)(DDRPHY_BASE + 0x0048)) = 0x0001110d;

	*((V_UINT32P)(DRAMC0_BASE + 0x00d8)) = 0x40500900;  
	*((V_UINT32P)(DDRPHY_BASE + 0x00d8)) = 0x40500900;   // Edward : Reg.0d8h.[31:30] PINMUX  01b:LPDDR2  10b:LPDDR3  11b:DDR3_16B

	*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000000;

	*((V_UINT32P)(DRAMC0_BASE + 0x008c)) = 0x00000001;
	*((V_UINT32P)(DDRPHY_BASE + 0x008c)) = 0x00000001;

	*((V_UINT32P)(DRAMC0_BASE + 0x0090)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x0090)) = 0x00000000;

	*((V_UINT32P)(DRAMC0_BASE + 0x0094)) = 0x80000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x0094)) = 0x80000000;

	*((V_UINT32P)(DRAMC0_BASE + 0x00dc)) = 0x83004004;   // Edward : 6589 is 0x83100100. It is DQS0CTL & DQS1CTL. Now is no use according to CC Wen.
	*((V_UINT32P)(DDRPHY_BASE + 0x00dc)) = 0x83004004;

	//*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x19004004;
	//*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x19004004;
	
#ifdef SYSTEM_26M //Edwin
	*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1c004004;
	*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1c004004;
	*((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080022;
	*((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080022;
#else
    #ifdef REAL_MEMPLL_MDL //Edwin
	#ifdef DDRPHY_3PLL_MODE
		*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1c004004;
		*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1c004004;
	#else
		*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
		*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;
	#endif

	#ifdef DDRPHY_3PLL_MODE
		*((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080011;
		*((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080011;
	#else
		*((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080033;
		*((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080033;
	#endif
    #else
	//*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1c004004;  //orig
	//*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1c004004;  //orig
	*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
	*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;
        #ifndef DDR_POSTSIM
		*((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080033;
		*((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080033;
        #else
		*((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080011;    //tuning DQS gating window
		*((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080011;    //tuning DQS gating window
		*((V_UINT32P)(DRAMC0_BASE + 0x0018)) = 0x10101010;    //delay DQ
		*((V_UINT32P)(DDRPHY_BASE + 0x0018)) = 0x10101010;    //delay DQ
        #endif
    #endif
#endif

	*((V_UINT32P)(DRAMC0_BASE + 0x00f0)) = 0x80000000;	// Edward : According to Brian, DQ4BMUX bit 31 should set to 1 for all dram type..
	*((V_UINT32P)(DDRPHY_BASE + 0x00f0)) = 0x80000000;

	//*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01000000;
	//*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01000000;
    #ifdef DRAMC_ASYNC
	*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;      // PHYSYNCM, default to 0 -->  not PHY sync mode (1T more)
	*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;       // Edward : 6589 bit 28 PHYSYNCM is 1 (PHY sync mode). 6582 is supposed to be 0. (sync mode).
    #else
	*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
	*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #endif

#ifdef PALLADIUM
	*((V_UINT32P)(DRAMC0_BASE + 0x0168)) = 0x00000000;  // Edward : bit[7:0] MAXPENDCNT is different. 6589 is  0x10. The max number of data pending in DRAMC.
	*((V_UINT32P)(DDRPHY_BASE + 0x0168)) = 0x00000000;	   //Edward : Yi-Chih comment ¥?«e?³¸?¤£»?¥dMAX PENDING COUT¤F§?­?·|¦bEMI¥dPUSH ®?¶¡, ¤£º?³] 0x80 ©?¬O 0 ³£¥i¥H
#else
	*((V_UINT32P)(DRAMC0_BASE + 0x0168)) = 0x00000080;
	*((V_UINT32P)(DDRPHY_BASE + 0x0168)) = 0x00000080;
#endif

	*((V_UINT32P)(DRAMC0_BASE + 0x00d8)) = 0x40700900;	// Edward : Reg.0d8h.[31:30] PINMUX  01b:LPDDR2  10b:LPDDR3  11b:DDR3_16B
	*((V_UINT32P)(DDRPHY_BASE + 0x00d8)) = 0x40700900;

	*((V_UINT32P)(DRAMC0_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;    // Edward : MATYPE[9:8] column address width different. 6589 is 10b. TRRD is loose.
	*((V_UINT32P)(DDRPHY_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;	    // MATYPE is for test engine use, set to 0 is safe.

	//*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = 0xc00646d1;
	//*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = 0xc00646d1;
#ifdef DRAMC_ASYNC
	*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;    //DLE  Edward : TRTP seems loose.
	*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;    // Edward : WLAT calculation is different (-2) compared to 6589 which is confirmed by CC Wen.
#else
	*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;
	*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;
#endif

	*((V_UINT32P)(DRAMC0_BASE + 0x0028)) = 0xf1200f01;
	*((V_UINT32P)(DDRPHY_BASE + 0x0028)) = 0xf1200f01;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e0)) = 0x3001ebff;   //**Edward : 6589 setting is 0x30003fff. It is related to pin mux and need to check with  Brian.
	*((V_UINT32P)(DDRPHY_BASE + 0x01e0)) = 0x3001ebff;

	*((V_UINT32P)(DRAMC0_BASE + 0x0158)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x0158)) = 0x00000000;

#ifdef PALLADIUM
	*((V_UINT32P)(DRAMC0_BASE + 0x0400)) = 0x00111100;	//** Edward : 0x400~0x410. No such registers according to CC Wen. Need to check with Brian.
	*((V_UINT32P)(DDRPHY_BASE + 0x0400)) = 0x00111100;

	*((V_UINT32P)(DRAMC0_BASE + 0x0404)) = 0x00000003;
	*((V_UINT32P)(DDRPHY_BASE + 0x0404)) = 0x00000003;

	*((V_UINT32P)(DRAMC0_BASE + 0x0408)) = 0x00222222;
	*((V_UINT32P)(DDRPHY_BASE + 0x0408)) = 0x00222222;

	*((V_UINT32P)(DRAMC0_BASE + 0x040c)) = 0x03330000;
	*((V_UINT32P)(DDRPHY_BASE + 0x040c)) = 0x03330000;

	*((V_UINT32P)(DRAMC0_BASE + 0x0410)) = 0x03330000;
	*((V_UINT32P)(DDRPHY_BASE + 0x0410)) = 0x03330000;
#else
	*((V_UINT32P)(DRAMC0_BASE + 0x0400)) = 0x00111100;
	*((V_UINT32P)(DDRPHY_BASE + 0x0400)) = 0x00111100;

	*((V_UINT32P)(DRAMC0_BASE + 0x0404)) = 0x00000002;
	*((V_UINT32P)(DDRPHY_BASE + 0x0404)) = 0x00000002;

	*((V_UINT32P)(DRAMC0_BASE + 0x0408)) = 0x00222222;
	*((V_UINT32P)(DDRPHY_BASE + 0x0408)) = 0x00222222;

	*((V_UINT32P)(DRAMC0_BASE + 0x040c)) = 0x33330000;
	*((V_UINT32P)(DDRPHY_BASE + 0x040c)) = 0x33330000;

	*((V_UINT32P)(DRAMC0_BASE + 0x0410)) = 0x33330000;
	*((V_UINT32P)(DDRPHY_BASE + 0x0410)) = 0x33330000;
#endif
	*((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x0b051311;   // Edward : RKSIZE[26:24] is used for TA and is not used. TA now uses CS swap to test rank 1.
	*((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x0b051311;				// This is dual rank setting. MRS send to both.

	//*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000004;
	//*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000004;
	*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL
	*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL

	__asm__ __volatile__ ("dsb" : : : "memory");	// Edward : add this according to 6589.
	gpt_busy_wait_us(200);//Wait > 200us              // Edward : add this according to 6589. Check with designer if it is needed.
    
	*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_63;	// issue reset to DRAM
	*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_63;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	__asm__ __volatile__ ("dsb" : : : "memory");
	gpt_busy_wait_us(10);//Wait > 10us				// Edward : add this according to DRAM spec, should wait at least 10us if not checking DAI.

	*((V_UINT32P)(DRAMC0_BASE + 0x0110)) &= (~0x7);	// Edward : Add this to disable two rank support for ZQ calibration tempority. 
	*((V_UINT32P)(DDRPHY_BASE + 0x0110)) &= (~0x7);
        
	*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_10;		// Edward : calibration command after init (just comment).
	*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_10;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	gpt_busy_wait_us(1);		//Wait > 1us. Edward : Add this because tZQINIT min value is 1us.

	// Edward : Add this for dual ranks support.
	if ( *(volatile unsigned *)(EMI_CONA)& 0x20000)  {
	        //for chip select 1: ZQ calibration
	        *((V_UINT32P)(DRAMC0_BASE + 0x0110)) |= (0x8);
	        *((V_UINT32P)(DDRPHY_BASE + 0x0110)) |= (0x8);
	  
	        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_10;
	        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_10;
	  
	        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	        __asm__ __volatile__ ("dsb" : : : "memory");
	        gpt_busy_wait_us(1);//Wait > 100us

	        //swap back
	        *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) &= (~0x8);
	        *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) &= (~0x8);
	        *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) |= (0x1);
	        *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) |= (0x1);
	}
    
	*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_1;
	*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_1;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	gpt_busy_wait_us(1);//Wait > 1us			// Edward : 6589 has this delay. Seems no need.

	*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_2;
	*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_2;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	*((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;		
	*((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	gpt_busy_wait_us(1);//Wait > 1us			// Edward : 6589 has this delay. Seems no need.

	*((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_3;		//Edward : no dram driving setting. Add this. May be not needed because default value is the same.
	*((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_3;


	*((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	*((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00001100;		
	*((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00001100;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	gpt_busy_wait_us(1);//Wait > 1us		// Edward : 6589 has this delay. Seems no need.

	// Edward : add two rank enable here.
	if ( *(volatile unsigned *)(EMI_CONA)& 0x20000) {
		*((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112391;	
		*((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112391;
	} else {
		*((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112390;	// Edward : per bank refresh enable here. 
		*((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112390;
	}

	*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL
	*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL

	*((V_UINT32P)(DRAMC0_BASE + 0x01ec)) = 0x00000001 ;    // Edward : Add this to enable dual scheduler according to CC Wen. Should enable this for all DDR type. 
	*((V_UINT32P)(DDRPHY_BASE + 0x01ec)) = 0x00000001;

	*((V_UINT32P)(DRAMC0_BASE + 0x0084)) = 0x00000a56;
	*((V_UINT32P)(DDRPHY_BASE + 0x0084)) = 0x00000a56;

	*((V_UINT32P)(DRAMC0_BASE + 0x000c)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x000c)) = 0x00000000;

	*((V_UINT32P)(DRAMC0_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL; // Edward : tRAS[3:0] 3h, TRC[7:4] 9h, TWTR[11:8] 4h,  TWR[19:16] 8h, 
	*((V_UINT32P)(DDRPHY_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;  // Edward : TFAW[23:20] 5h, TRP[27:24] 4h, TRCD[31:28] 4h

	// Edward : AC_TIME_0.5T control (new for 6582)
	*((V_UINT32P)(DRAMC0_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;  
	*((V_UINT32P)(DDRPHY_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;   //TWTR_M[26]=1,TFAW[13]=1,TWR_M[10]=1,TRAS[9]=1

	//*((V_UINT32P)(DRAMC0_BASE + 0x0044)) = 0x280e0401;	
	//*((V_UINT32P)(DDRPHY_BASE + 0x0044)) = 0x280e0401;	
	*((V_UINT32P)(DRAMC0_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;	// Edward : Modify to enable advance precharge and audio pattern.
	*((V_UINT32P)(DDRPHY_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;	// Edward : TRFC=18h, TRFC[19:16]=8h

	*((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;	// Edward : RFCpb = 90 -> d, RFCpb = 60 -> 5?? TRPAB? TRFC?
	*((V_UINT32P)(DDRPHY_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;

	//333Mhz
	//*((V_UINT32P)(DRAMC0_BASE + 0x0004)) = 0xf00487c3;
	//*((V_UINT32P)(DRAMC0_BASE + 0x0000)) = 0x55d84608;
	//*((V_UINT32P)(DRAMC0_BASE + 0x0044)) = 0x28000401;
	//*((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = 0x00001010;

	*((V_UINT32P)(DRAMC0_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;	
	*((V_UINT32P)(DDRPHY_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;

	*((V_UINT32P)(DRAMC0_BASE + 0x0010)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x0010)) = 0x00000000;

	*((V_UINT32P)(DRAMC0_BASE + 0x00f8)) = 0xedcb000f;
	*((V_UINT32P)(DDRPHY_BASE + 0x00f8)) = 0xedcb000f;

	*((V_UINT32P)(DRAMC0_BASE + 0x0020)) = 0x00000000;
	*((V_UINT32P)(DDRPHY_BASE + 0x0020)) = 0x00000000;
	// Edward : no definition in 640 [31:16]??
#ifdef DRAMC_ASYNC
	*((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xfb8<<20);  //enable DDRPHY dynamic clk gating, // disable D/C_PHY_M_CK dynamic gating
	*((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfb8<<20);  //enable DDRPHY dynamic clk gating, // disable D/C_PHY_M_CK dynamic gating
#else
	*((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xff8<<20);  //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
	*((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xff8<<20);  //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
#endif
	*((V_UINT32P)(DRAMC0_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //enable DDRPHY dynamic clk gating.  
	*((V_UINT32P)(DDRPHY_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //Edward : REFCNT_FR_CLK[23:16]=67h, REFFRERUN[24]=1. TXREFCNT[15:8]=23h.

        *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0x3f00);  //enable Pre-Emphasis
        *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0x3f00);  //enable Pre-Emphasis

	// Before calibration, set default settings to avoid some problems happened during calibration.
#if 1
	// GW coarse tune 15 (fh -> 1111b -> 11b, 11b) 
	*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = (*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) & 0xf8ffffff) | (0x03<<24);
	*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = (*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) & 0xf8ffffff) | (0x03<<24);

	*((V_UINT32P)(DRAMC0_BASE + 0x0124)) = (*((V_UINT32P)(DRAMC0_BASE + 0x0124)) & 0xffffff00) | 0xff;
	*((V_UINT32P)(DDRPHY_BASE + 0x0124)) = (*((V_UINT32P)(DDRPHY_BASE + 0x0124)) & 0xffffff00) | 0xff;

	// GW fine tune 56 (38h)
	*((V_UINT32P)(DRAMC0_BASE + 0x0094)) = 0x38383838;
	*((V_UINT32P)(DDRPHY_BASE + 0x0094)) = 0x38383838;	

	*((V_UINT32P)(DRAMC0_BASE + 0x0098)) = 0x38383838;
	*((V_UINT32P)(DDRPHY_BASE + 0x0098)) = 0x38383838;		

	//  DLE 8
	*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = (*((V_UINT32P)(DRAMC0_BASE + 0x007c)) & 0xFFFFFF8F) | ((8 & 0x07) <<4);
	*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = (*((V_UINT32P)(DDRPHY_BASE + 0x007c)) & 0xFFFFFF8F) |	((8 & 0x07) <<4);

	*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = (*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) & 0xFFFFFFEF) | (((8 >> 3) & 0x01) << 4);
	*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) =  (*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) & 0xFFFFFFEF) | (((8 >> 3) & 0x01) << 4);

	// RX DQ, DQS input delay
	*((V_UINT32P)(DRAMC0_BASE + 0x0210)) = 0xE09080B;
	*((V_UINT32P)(DDRPHY_BASE + 0x0210)) = 0xE09080B;	
	*((V_UINT32P)(DRAMC0_BASE + 0x0214)) = 0x908090C;
	*((V_UINT32P)(DDRPHY_BASE + 0x0214)) = 0x908090C;	
	*((V_UINT32P)(DRAMC0_BASE + 0x0218)) = 0x8070809;
	*((V_UINT32P)(DDRPHY_BASE + 0x0218)) = 0x8070809;	
	*((V_UINT32P)(DRAMC0_BASE + 0x021c)) = 0x7070608;
	*((V_UINT32P)(DDRPHY_BASE + 0x021c)) = 0x7070608;	
	*((V_UINT32P)(DRAMC0_BASE + 0x0220)) = 0xA0B080C;
	*((V_UINT32P)(DDRPHY_BASE + 0x0220)) = 0xA0B080C;	
	*((V_UINT32P)(DRAMC0_BASE + 0x0224)) = 0x8070A08;
	*((V_UINT32P)(DDRPHY_BASE + 0x0224)) = 0x8070A08;	
	*((V_UINT32P)(DRAMC0_BASE + 0x0228)) = 0x6050807;
	*((V_UINT32P)(DDRPHY_BASE + 0x0228)) = 0x6050807;	
	*((V_UINT32P)(DRAMC0_BASE + 0x022c)) = 0x6060306;
	*((V_UINT32P)(DDRPHY_BASE + 0x022c)) = 0x6060306;	

	*((V_UINT32P)(DRAMC0_BASE + 0x0018)) = 0x19191919;
	*((V_UINT32P)(DDRPHY_BASE + 0x0018)) = 0x19191919;	
	*((V_UINT32P)(DRAMC0_BASE + 0x001c)) = 0x19191919;
	*((V_UINT32P)(DDRPHY_BASE + 0x001c)) = 0x19191919;	

	// TX DQ, DQS output delay
	*((V_UINT32P)(DRAMC0_BASE + 0x0014)) = 0x00008888;	// DQS output delay
	*((V_UINT32P)(DDRPHY_BASE + 0x0014)) = 0x00008888;	

	*((V_UINT32P)(DRAMC0_BASE + 0x0010)) = 0x0000aaaa;	// DQM output delay
	*((V_UINT32P)(DDRPHY_BASE + 0x0010)) = 0x0000aaaa;	

	*((V_UINT32P)(DRAMC0_BASE + 0x0200)) = 0xaaaaaaaa;	// DQ
	*((V_UINT32P)(DDRPHY_BASE + 0x0200)) = 0xaaaaaaaa;	
	*((V_UINT32P)(DRAMC0_BASE + 0x0204)) = 0xaaaaaaaa;	
	*((V_UINT32P)(DDRPHY_BASE + 0x0204)) = 0xaaaaaaaa;	
	*((V_UINT32P)(DRAMC0_BASE + 0x0208)) = 0xaaaaaaaa;	
	*((V_UINT32P)(DDRPHY_BASE + 0x0208)) = 0xaaaaaaaa;	
	*((V_UINT32P)(DRAMC0_BASE + 0x020c)) = 0xaaaaaaaa;	
	*((V_UINT32P)(DDRPHY_BASE + 0x020c)) = 0xaaaaaaaa;	

        *((V_UINT32P)(DDRPHY_BASE + 0x00bc)) &= (~0xff00ff00);
        *((V_UINT32P)(DDRPHY_BASE + 0x00bc)) |= (0xee << 24 | 0xcc << 8);
        *((V_UINT32P)(DDRPHY_BASE + 0x01a8)) = 0x05050505;    //bit0-3
        *((V_UINT32P)(DDRPHY_BASE + 0x01ac)) = 0x05050505;    //bit4-7
        *((V_UINT32P)(DDRPHY_BASE + 0x01b0)) = 0x05050505;    //bit8-11
        *((V_UINT32P)(DDRPHY_BASE + 0x01b4)) |= 0x5;          //bit12
        *((V_UINT32P)(DDRPHY_BASE + 0x01bc)) |= (0x5<<16);    //bit13
        *((V_UINT32P)(DDRPHY_BASE + 0x01c0)) = (0x0505<<16);  //bit14,15
	
#endif
#else
    printf("enter the wrong part\n");
#endif //!COMBO_LPDDR2
}


/*
 * init_dram: Do initialization for DDR3.
 */
#define DDR3_16B   				// Edward : 6582 only supports x16.
//#define DDR3_SCRAMBLE_SUPPORT
#define DDR3_TEST_PATTERN	2  //0: ISO pattern, 1: Audio pattern, 2: Cross-talk pattern

static void init_ddr3(EMI_SETTINGS *emi_setting) 
{
#if COMBO_PCDDR3 > 0
#ifdef DDR3_16B
	print("mt6582 ddr3 x16 init :\n");
#else
	print("mt6582 ddr3 x32 init :\n");
#endif

	// The following settings are from Yi-Chih.
	/* TINFO="Star mm_cosim_emi_config" */
	//----------------EMI Setting--------------------
	*((volatile unsigned int *)(EMI_BASE + 0x0060))=0x0000051f;
	*((volatile unsigned int *)(EMI_BASE + 0x0140))=0x20406188;//0x12202488;   // 83 for low latency
	*((volatile unsigned int *)(EMI_BASE + 0x0144))=0x20406188;//0x12202488; //new add
	*((volatile unsigned int *)(EMI_BASE + 0x0100))=0x40105906;//0x40105808; //m0 cpu ori:0x8020783f
	*((volatile unsigned int *)(EMI_BASE + 0x0108))=0x808050ea;//0xa0a05028; //m1 ori:0x80200000 // ultra can over limit
	*((volatile unsigned int *)(EMI_BASE + 0x0110))=0xffff50c4;//0xa0a05001; //m2 arm9
	*((volatile unsigned int *)(EMI_BASE + 0x0118))=0x0810d84a;//0x030fd80d; // ???????????????????????????????????????????????????????????//m3 mdmcu ori:0x80807809 ori:0x07007010 bit[12]:enable bw limiter
	*((volatile unsigned int *)(EMI_BASE + 0x0120))=0x40405042; //m4 fcore ori:0x8080781a
	// *((volatile unsigned int *)(EMI_BASE +0x0128))=0xa0a05001; //m5 peri
	// *((volatile unsigned int *)(EMI_BASE +0x0130))=0xa0a05028; //m6 vcodec ori:0x8080381a
	*((volatile unsigned int *)(EMI_BASE + 0x0148))=0x9719595e;//0323 chg, ori :0x00462f2f
	*((volatile unsigned int *)(EMI_BASE + 0x014c))=0x9719595e; // new add
	//-----------------------------------
	//-- modified by liya for EMI address mapping setting
	//*EMI_CONA = 0x31303130;		// Edward : Need to set bit 1 DW32_EN to 0 because DDR3 only support x16.
	*(volatile unsigned int*)(EMI_CONA) = emi_setting->EMI_CONA_VAL;//0x00003120;			// Edward 4Gb(256bx16) settings (may use  Hynix H5TC4G64AFR), 8 banks/15 rows/10 columns.	
	//*((volatile unsigned int *)(EMI_BASE +0x0000))=0x1130112e;
	//-----------------------------------
	*((volatile unsigned int *)(EMI_BASE + 0x00f8))=0x00000000;
	*((volatile unsigned int *)(EMI_BASE + 0x0400))=0x007f0001;
	*((volatile unsigned int *)(EMI_BASE + 0x0008))=0x0404040d;// Chuan:0x181e1e2b Edwin:0x12161620
	*((volatile unsigned int *)(EMI_BASE + 0x0010))=0x05050505;// Chuan:0x18181818 Edwin:0x12121212
	*((volatile unsigned int *)(EMI_BASE + 0x0018))=0x0404040d;// Chuan:0x1e262b33 Edwin:0x161c2026
	*((volatile unsigned int *)(EMI_BASE + 0x0020))=0x05050505;// Chuan:0x1818181b Edwin:0x12121214
	*((volatile unsigned int *)(EMI_BASE + 0x0030))=0x05050404;
	*((volatile unsigned int *)(EMI_BASE + 0x0038))=0x05050404;
	*((volatile unsigned int *)(EMI_BASE + 0x0158))=0x00010800;// ???????????????????????0x08090800; 
	*((volatile unsigned int *)(EMI_BASE + 0x0150))=0x6470fc79;
	*((volatile unsigned int *)(EMI_BASE + 0x0154))=0x6470fc79;
	//============== Defer WR threthold
	*((volatile unsigned int *)(EMI_BASE + 0x00f0))=0x38470000;
	//============== Reserve bufer
	//
	//MDMCU don't always ULTRA, but small age
	*((volatile unsigned int *)(EMI_BASE + 0x0078))=0x23110003;// ???????????0x00030F4d; 
	// Turn on M1 Ultra and all port DRAMC hi enable
	*((volatile unsigned int *)(EMI_BASE + 0x0158))=0x1f011f00;// ???????????????????????0x08090800; 
	// RFF)_PBC_MASK
	*((volatile unsigned int *)(EMI_BASE + 0x00e8))=0x00000084;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
	// Page hit is high
	*((volatile unsigned int *)(EMI_BASE + 0x0060))=0x0000051f;
	*((volatile unsigned int *)(EMI_BASE + 0x00d0))=0x84848c84;//R/8 W/8 outstanding
	*((volatile unsigned int *)(EMI_BASE + 0x00d8))=0x00480084;//R/8 W/8 outstanding
	//===========END===========================================
	#ifdef DDR3_SCRAMBLE_SUPPORT
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0100)) = 0xDC000000; //bit28 data scramble enable
	#endif

#ifdef DRAMC_ASYNC
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00FC)) = emi_setting->DRAMC_MISCTL0_VAL;//0x07900000;   // Edward : TXP[30:28]=0,  
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00FC)) = emi_setting->DRAMC_MISCTL0_VAL;//0x07900000;   //ASYNC
#else
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00FC)) = emi_setting->DRAMC_MISCTL0_VAL;//0x07800000;	// Edward : MODE18V[16] 1->0
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00FC)) = emi_setting->DRAMC_MISCTL0_VAL;//0x07800000;	// Edward :  Yi-Chih/CC Wen comment that PBC_ARB_E1T[23] set to 1.
#endif

	#if(DDR3_TEST_PATTERN == 0) 
	//iso pattern
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0048) = 0x1e00110d;		// Edward : tZQCS[31:24]=1Eh. -> ZQCS enable is off in reg[39h<<2]
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0048) = 0x1e00110d;      // iso pattern
    #elif(DDR3_TEST_PATTERN == 1)
	//audio pattern
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0048) = 0x1e00110d;		// Edward : tZQCS[31:24]=1Eh. -> ZQCS enable is off in reg[39h<<2]
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0048) = 0x1e00110d;      // audio pattern
	#else
    //default cross-talk pattern
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0048) = 0x1e01110d;		// Edward : tZQCS[31:24]=1Eh. -> ZQCS enable is off in reg[39h<<2]
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0048) = 0x1e01110d;      // cross-talk pattern  
	#endif

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00D8) = 0xC0100900;	 	//PINMUX0 & 1==> 1 	
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00D8) = 0xC0100900;		// Edward : Reg.0d8h.[31:30] PINMUX  01b:LPDDR2  10b:LPDDR3  11b:DDR3_16B

	//*(volatile unsigned int *)(DRAMC0_BASE + 0x00E4) = 0x000000a2;
	//*(volatile unsigned int *)(DDRPHY_BASE + 0x00E4) = 0x000000a2;
	*(volatile unsigned int *)(DRAMC0_BASE + 0x00E4) = 0x000000a3;	//CKEBYCTL
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00E4) = 0x000000a3;
	 delay_a_while(11500); // Reset need delay 11.5us.
         delay_a_while(11500); // Reset need delay 100.5us. 
         delay_a_while(11500); // Reset need delay 100.5us. 
         delay_a_while(11500); // Reset need delay 100.5us. 
         delay_a_while(11500); // Reset need delay 100.5us. 
         delay_a_while(11500); // Reset need delay 100.5us.

	*(volatile unsigned int *)(DRAMC0_BASE + 0x008C) = 0x00100008;	  //[20]DQ16COM1=1, [3]dqcmd=1, WTLATC=000, only pinmux1 use // Edward : DQCMD[3] is on here (use DQ16~31 as command/address) but 6589 is off. 6582 should set this.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x008C) = 0x00100008;	  //[20]DQ16COM1=1, [3]dqcmd=1, WTLATC=000, only pinmux1 use

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0090) = 0x00000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0090) = 0x00000000;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0094) = 0x80000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0094) = 0x80000000;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00DC) = 0x83080080;		// Edward : DQS1CTL[23:12] DQS0CTL[11:0] value different compared with 6589 (0x83000000). These fields are not used now..
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00DC) = 0x83080080;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00E0) = 0x13080080;		// Edward : DQS3CTL/DQS2 value is different with 6589 (0x13000000). These fields are not used now.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00E0) = 0x13080080;

#ifdef DDR3_16B
	*(volatile unsigned int *)(DRAMC0_BASE + 0x00F0) = 0x80000000;	 //pinmux2=0  // // Edward : According to Brian, DQ4BMUX should set to 1 for all dram type.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00F0) = 0x80000000;	 //pinmux2=0
#else
	*(volatile unsigned int *)(DRAMC0_BASE + 0x00F0) = 0x80000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00F0) = 0x80000000;
#endif //DDR3_16B

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00F4) = emi_setting->DRAMC_GDDR3CTL1_VAL;//0x01000000;	//bit24 8 bank device enable	// Edward : PHYSYNCM[28](sync mode use inverted PHY_M_CLK)	is on in 6589 when sync mode.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00F4) = emi_setting->DRAMC_GDDR3CTL1_VAL;//0x01000000;	//bit24 8 bank device enable  Edward : 6589 bit 28 PHYSYNCM is 1 (PHY sync mode). 6582 is supposed to be 0. (sync mode).

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0168) = 0x00000080;		// Edward : MAXPENDCNT[7:0] is 0x10 in 6589? Yi-Chih said this is taken cared by EMI and 0 or 0x80 are OK.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0168) = 0x00000080;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0130) = 0x30000000;	 //dram_clk_en0 & 1
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0130) = 0x30000000;	 //dram_clk_en0 & 1
	delay_a_while(1000);									// Edward : add this because 6589 has this.

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00D8) = 0xC0300900;	 //PINMUX0 & 1 ==> 1		//	Edward : Reg.0d8h.[31:30] PINMUX  01b:LPDDR2  10b:LPDDR3  11b:DDR3_16B
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00D8) = 0xC0300900;

#ifdef DDR3_16B
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0004) = emi_setting->DRAMC_CONF1_VAL;//0xf07484a2;	// Edward : TRRD[7:6]=2h 
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0004) = emi_setting->DRAMC_CONF1_VAL;//0xf07484a2;	// Edward : MATYPE[9:8] set to 0 for safe.
#else
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0004) = emi_setting->DRAMC_CONF1_VAL;//0xf07486a3;	//DM64BIT=1
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0004) = emi_setting->DRAMC_CONF1_VAL;//0xf07486a3;
#endif
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0124) = 0x80000011;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0124) = 0x80000011;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0094) = 0x40404040;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0094) = 0x40404040;

	//*(volatile unsigned int *)(DRAMC0_BASE + 0x01C0) = 0x8000c8b8;
	//*(volatile unsigned int *)(DDRPHY_BASE + 0x01C0) = 0x8000c8b8;
	*(volatile unsigned int *)(DRAMC0_BASE + 0x01C0) = 0x00000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x01C0) = 0x00000000;	

	#if 0
	//write ODT enable, read ODT enable
	*(volatile unsigned int *)(DRAMC0_BASE + 0x007C) = emi_setting->DRAMC_DDR2CTL_VAL;//0x918e11cd;	// Edward : WLAT[30:28]=1, TR2W[15:12]=1, TRTP[10:8]=1, DATLAT[6:4]
	*(volatile unsigned int *)(DDRPHY_BASE + 0x007C) = emi_setting->DRAMC_DDR2CTL_VAL;//0x918e11cd;	// Edward : WDATGO is on, so WL=WLAT+3+2. WL=6, so WLAT=1
	#else
	//write ODT disable, read ODT disable
	*(volatile unsigned int *)(DRAMC0_BASE + 0x007C) = emi_setting->DRAMC_DDR2CTL_VAL;//0x918e11c1;	// Edward : WLAT[30:28]=1, TR2W[15:12]=1, TRTP[10:8]=1, DATLAT[6:4]
	*(volatile unsigned int *)(DDRPHY_BASE + 0x007C) = emi_setting->DRAMC_DDR2CTL_VAL;//0x918e11c1;	// Edward : WDATGO is on, so WL=WLAT+3+2. WL=6, so WLAT=1
	#endif

#ifdef DDR3_16B
	*(volatile unsigned int *)(DRAMC0_BASE + 0x008C) = 0x00100008;	  //[20]DQ16COM1=1, [3]dqcmd=1, WTLATC=000, only pinmux1 use
	*(volatile unsigned int *)(DDRPHY_BASE + 0x008C) = 0x00100008;	  //[20]DQ16COM1=1, [3]dqcmd=1, WTLATC=000, only pinmux1 use
#endif

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0028) = 0xF1200f01;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0028) = 0xF1200f01;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0158) = 0x00000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0158) = 0x00000000;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x01E0) = 0x08000000;	// **Edward : WDATRGO[27] will influenece timing setting. For high frequency, delay 1T i nDRAMC. Will impact WLAT also.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x01E0) = 0x08000000;

	//*((volatile unsigned int *)(DRAMC0_BASE + 0x00E4)) = 0x000000a6;
	//*((volatile unsigned int *)(DDRPHY_BASE + 0x00E4)) = 0x000000a6;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00E4)) = 0x000000a7;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00E4)) = 0x000000a7;
	delay_a_while(2000);				// Edward : add this because 6589 has this. Need to ask designer if it is needed?

	//--------------------------------------------------------------------
	//CLK, CMD driving
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00BC)) = emi_setting->DRAMC_DRVCTL1_VAL;//0xaa00aa00;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00BC)) = emi_setting->DRAMC_DRVCTL1_VAL;//0xaa00aa00;

	//DQS driving
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00B8)) = emi_setting->DRAMC_DRVCTL0_VAL;//0xaa00aa00;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00B8)) = emi_setting->DRAMC_DRVCTL0_VAL;//0xaa00aa00;
	
	//DQ driving
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00B4)) = emi_setting->DRAMC_DRVCTL0_VAL;//0xaa00aa00;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00B4)) = emi_setting->DRAMC_DRVCTL0_VAL;//0xaa00aa00;

	//Pre-emphassis enable
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0640)) |= 0x3F00;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0640)) |= 0x3F00;

	//CLK delay
	*((volatile unsigned int *)(DRAMC0_BASE + 0x000C)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x000C)) = 0x00000000;	

	//CMD/ADDR delay
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01A8)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01A8)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01AC)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01AC)) = 0x00000000;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01B0)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01B0)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01B4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01B4)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01B8)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01B8)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01BC)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01BC)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01C0)) &= 0xE0E0FFFF;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01C0)) &= 0xE0E0FFFF;
	delay_a_while(1000);
	
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0110)) = 0x0b051100;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0110)) = 0x0b051100;	
	//--------------------------------------------------------------------
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG2;//0x00004008;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG2;//0x00004008;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG3;//0x00006000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG3;//0x00006000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG1;//0x00002000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG1;//0x00002000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG0;//0x00001941;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG0;//0x00001941;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = 0x00000400;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = 0x00000400;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000010;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000010;

	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	 delay_a_while(1000);		// Edward : Add this for ZQinit=Max(512nCK, 640ns) .

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00001100;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00001100;

	//*((volatile unsigned int *)(DRAMC0_BASE + 0x00E4)) = 0x000000a2;
	//*((volatile unsigned int *)(DDRPHY_BASE + 0x00E4)) = 0x000000a2;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00E4)) = 0x000007a3;  //CKEBYCTL
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00E4)) = 0x000007a3;

	//*((volatile unsigned int *)(DRAMC0_BASE + 0x01ec)) = 0x00000001;	   // Edward : Disable dual scheduler because DQCMD == 1. This is PHY limitation according to CC Wen. 
	//*((volatile unsigned int *)(DDRPHY_BASE + 0x01ec)) = 0x00000001;	  

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E0)) = 0x08000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E0)) = 0x08000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = 0x0000ffff;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = 0x0000ffff;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E4)) = 0x00000020;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E4)) = 0x00000020;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E4)) = 0x00000000;

#ifdef DDR3_16B
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0004)) = 0xf07484a2;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0004)) = 0xf07484a2;
#else
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0004)) = 0xf07486a3;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0004)) = 0xf07486a3;
#endif

	*((volatile unsigned int *)(DRAMC0_BASE + 0x000C)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x000C)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;//0x33584562;	// Edward : TRCD[31:28]=3, TRP[27:24]=3, TFAW[23:20]=5h, TWR[19:16]=8h
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;//0x33584562;		// Edward : TWTR[11:8]=5h,	TRC[7:4]=6, TRAS[3:0]=2h

	// Edward : AC_TIME_0.5T control (new for 6582)
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01f8)) = 0x05002000;  //TWTR_M[26]=1, TRTW[24]=1, TFAW[13]=1
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01f8)) = 0x05002000;   

	#if(DDR3_TEST_PATTERN == 0) 
	//iso pattern
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0044) = emi_setting->DRAMC_TEST2_3_VAL;//0xbf850401;		// Edward : Use iso pattern. TESTAUDPAT[7]=1
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0044) = emi_setting->DRAMC_TEST2_3_VAL;//0xbf850401;      // Edward : TRFC=45h, TRFC[19:16]=5h
	#elif(DDR3_TEST_PATTERN == 1)
	//audio pattern
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0044) = emi_setting->DRAMC_TEST2_3_VAL;//0xbf850481;		// Edward : Use audio pattern. TESTAUDPAT[7]=1
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0044) = emi_setting->DRAMC_TEST2_3_VAL;//0xbf850481;      // Edward : TRFC=45h, TRFC[19:16]=5h
	#else
    //default cross-talk pattern
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0044) = emi_setting->DRAMC_TEST2_3_VAL;//0xbf850401;		// Edward : Use cross-talk pattern. TESTAUDPAT[7]=1
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0044) = emi_setting->DRAMC_TEST2_3_VAL;//0xbf850401;      // Edward : TRFC=45h, TRFC[19:16]=5h  
	#endif

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E8)) = emi_setting->DRAMC_ACTIM1_VAL;//0x00000640;	//Edward : TRPAB[25:24]=NA, TRFCPB[15:8]=NA, TRFC_BIT7_4[7:4]=4h,
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E8)) = emi_setting->DRAMC_ACTIM1_VAL;//0x00000640;	// Edward : TRRD_BIT2[3]=0, TFAW_BIT4[1]=0, TRC_BIT4[0]=0
															// Edward : TRFC=45h, TRRD=2, TFAW=5, TRC=6
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;//0x0304693f;	// Edward : REDFCNT=3fh. But not use.
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;//0x0304693f;
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0010)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0010)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x00F8)) = 0xedcb000f;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00F8)) = 0xedcb000f;

	//*((volatile unsigned int *)(DRAMC0_BASE + 0x00FC)) = 0x27010000;
	//*((volatile unsigned int *)(DDRPHY_BASE + 0x00FC)) = 0x27010000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01D8)) = 0x00c80008;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01D8)) = 0x00c80008;

	//*((volatile unsigned int *)(DRAMC0_BASE + 0x01DC)) = 0x51972842;	//Edward : need to enable DDRPHY dynamic clk gating and FR.
	//*((volatile unsigned int *)(DDRPHY_BASE + 0x01DC)) = 0x51972842;	// REFFRERUN[24]=1, REFCNT_FR_CLK[23:16]=64h, TXREFCNT[15:8]=NA (Fix by HW)
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01DC)) = emi_setting->DRAMC_PD_CTRL_VAL;//0x51972842;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01DC)) = emi_setting->DRAMC_PD_CTRL_VAL;//0x51972842;

	#if 1
	// Before calibration, set default settings to avoid some problems happened during calibration.
	// GW coarse tune 7 (7h -> 0111b -> 001b, 11b) 
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00e0)) = (*((volatile unsigned int *)(DRAMC0_BASE + 0x00e0)) & 0xf8ffffff) | (0x01<<24);
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00e0)) = (*((volatile unsigned int *)(DDRPHY_BASE + 0x00e0)) & 0xf8ffffff) | (0x01<<24);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0124)) = (*((volatile unsigned int *)(DRAMC0_BASE + 0x0124)) & 0xffffff00) | 0x0033;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0124)) = (*((volatile unsigned int *)(DDRPHY_BASE + 0x0124)) & 0xffffff00) | 0x0033;

	// GW fine tune 0 (0h)
	//rank0 fine tune
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0094)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0094)) = 0x00000000;	
	//rank1 fine tune
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0098)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0098)) = 0x00000000;		

	//DLE 4
	*((volatile unsigned int *)(DRAMC0_BASE + 0x007c)) = (*((volatile unsigned int *)(DRAMC0_BASE + 0x007c)) & 0xFFFFFF8F) | ((4 & 0x07) <<4);
	*((volatile unsigned int *)(DDRPHY_BASE + 0x007c)) = (*((volatile unsigned int *)(DDRPHY_BASE + 0x007c)) & 0xFFFFFF8F) | ((4 & 0x07) <<4);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x00e4)) = (*((volatile unsigned int *)(DRAMC0_BASE + 0x00e4)) & 0xFFFFFFEF) | (((4 >> 3) & 0x01) << 4);
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00e4)) = (*((volatile unsigned int *)(DDRPHY_BASE + 0x00e4)) & 0xFFFFFFEF) | (((4 >> 3) & 0x01) << 4);

	// RX DQ, DQS input delay
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0210)) = 0x0F0F0A09;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0210)) = 0x0F0F0A09;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0214)) = 0x0B0A0C0E;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0214)) = 0x0B0A0C0E;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0218)) = 0x03070807;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0218)) = 0x03070807;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x021c)) = 0x06080207;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x021c)) = 0x06080207;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0220)) = 0x0F0F0F0F;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0220)) = 0x0F0F0F0F;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0224)) = 0x0F0F0F0F;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0224)) = 0x0F0F0F0F;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0228)) = 0x0F0F0F0F;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0228)) = 0x0F0F0F0F;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x022c)) = 0x0F0F0F0F;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x022c)) = 0x0F0F0F0F;	

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0018)) = 0x3F3F1B1A;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0018)) = 0x3F3F1B1A;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x001c)) = 0x3F3F1B1A;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x001c)) = 0x3F3F1B1A;	

	// TX DQ, DQS output delay
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0014)) = emi_setting->DRAMC_PADCTL3_VAL;//0x00008888;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0014)) = emi_setting->DRAMC_PADCTL3_VAL;//0x00008888;	
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0010)) = emi_setting->DRAMC_PADCTL3_VAL;//0x00008888;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0010)) = emi_setting->DRAMC_PADCTL3_VAL;//0x00008888;	

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0200)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0200)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0204)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0204)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0208)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0208)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x020c)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DDRPHY_BASE + 0x020c)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;
	
	#endif
	
	print("mt6582 ddr3 init finish!\n");
#endif//!PCDDR3
}
static void init_lpddr3(EMI_SETTINGS *emi_setting) 
{
#if COMBO_LPDDR3 > 0
        // The following settings are from Yi-Chih.
        /* TINFO="Star mm_cosim_emi_config" */
        //----------------EMI Setting--------------------
        *((V_UINT32P)(EMI_BASE   +0x0060))=0x0000051f;
        *((V_UINT32P)(EMI_BASE   +0x0140))=0x20406188;//0x12202488;   // 83 for low latency
        *((V_UINT32P)(EMI_BASE   +0x0144))=0x20406188;//0x12202488; //new add
        *((V_UINT32P)(EMI_BASE   +0x0100))=0x40105806;//0x40105808; //m0 cpu ori:0x8020783f
        *((V_UINT32P)(EMI_BASE   +0x0108))=0x808050ea;//0xa0a05028; //m1 ori:0x80200000 // ultra can over limit
        *((V_UINT32P)(EMI_BASE   +0x0110))=0xffff50c4;//0xa0a05001; //m2 arm9
      *((V_UINT32P)(EMI_BASE   +0x0118))=0x0810d84a;//0x030fd80d; // ???????????????????????????????????????????????????????????//m3 mdmcu ori:0x80807809 ori:0x07007010 bit[12]:enable bw limiter
        *((V_UINT32P)(EMI_BASE   +0x0120))=0x40405042; //m4 fcore ori:0x8080781a
        // *((V_UINT32P)(EMI_BASE   +0x0128))=0xa0a05001; //m5 peri
        // *((V_UINT32P)(EMI_BASE   +0x0130))=0xa0a05028; //m6 vcodec ori:0x8080381a
        *((V_UINT32P)(EMI_BASE   +0x0148))=0x9719595e;//0323 chg, ori :0x00462f2f
        *((V_UINT32P)(EMI_BASE   +0x014c))=0x9719595e; // new add
        //-----------------------------------
        //-- modified by liya for EMI address mapping setting
        //*EMI_CONA = 0x31303132;
        //*EMI_CONA = 0x0002A3AE;                // Edward : Use 8Gb (2 ranks settings)
        *EMI_CONA = emi_setting->EMI_CONA_VAL;             // Edward : Use 8Gb (2 ranks settings)
        //*((UINT32P)(EMI_BASE   +0x0000))=0x1130112e;
        //-----------------------------------
        *((V_UINT32P)(EMI_BASE   +0x00f8))=0x00000000;
        *((V_UINT32P)(EMI_BASE   +0x0400))=0x007f0001;
        *((V_UINT32P)(EMI_BASE   +0x0008))=0x0617192a;// Chuan:0x181e1e2b Edwin:0x12161620
        *((V_UINT32P)(EMI_BASE   +0x0010))=0x07160616;// Chuan:0x18181818 Edwin:0x12121212
        *((V_UINT32P)(EMI_BASE   +0x0018))=0x0617192a;// Chuan:0x1e262b33 Edwin:0x161c2026
        *((V_UINT32P)(EMI_BASE   +0x0020))=0x07160616;// Chuan:0x1818181b Edwin:0x12121214
        *((V_UINT32P)(EMI_BASE   +0x0030))=0x07060619;
        *((V_UINT32P)(EMI_BASE   +0x0038))=0x07060619;
        *((V_UINT32P)(EMI_BASE   +0x0158))=0x00010800;// ???????????????????????0x08090800; 
        *((V_UINT32P)(EMI_BASE   +0x0150))=0x6470fc79;
        *((V_UINT32P)(EMI_BASE   +0x0154))=0x6470fc79;
        *((V_UINT32P)(EMI_BASE   +0x00e8))=0x00000100;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
        //============== Defer WR threthold
        *((V_UINT32P)(EMI_BASE   +0x00f0))=0x38470000;
        //============== Reserve bufer
        //
        //MDMCU don't always ULTRA, but small age
        *((V_UINT32P)(EMI_BASE   +0x0078))=0x23110003;// ???????????0x00030F4d; 
        // Turn on M1 Ultra and all port DRAMC hi enable
        *((V_UINT32P)(EMI_BASE   +0x0158))=0x1f011f00;// ???????????????????????0x08090800; 
        // RFF)_PBC_MASK
        *((V_UINT32P)(EMI_BASE   +0x00e8))=0x00000024;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
        // Page hit is high
        *((V_UINT32P)(EMI_BASE   +0x0060))=0x0000051f;
        *((V_UINT32P)(EMI_BASE   +0x00d0))=0x84848c84;//R/8 W/8 outstanding
        *((V_UINT32P)(EMI_BASE   +0x00d8))=0x00480084;//R/8 W/8 outstanding
        //===========END===========================================

#ifdef DRAMC_ASYNC
//     *((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = 0x07900000;  //ASYNC   // Edward : Modify MODE18V bit 16 from 1 (1.8V) to 0(1.2V).
//     *((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = 0x07900000;  //ASYNC    // Edward : TXP[30:28]=0
        *((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;  //ASYNC   // Edward : Modify MODE18V bit 16 from 1 (1.8V) to 0(1.2V).
        *((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;  //ASYNC    // Edward : TXP[30:28]=0
#else
//     *((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = 0x07800000;
//     *((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = 0x07800000;
        *((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;
        *((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;
#endif

        *((V_UINT32P)(DRAMC0_BASE + 0x0048)) = 0x0001110d;
        *((V_UINT32P)(DDRPHY_BASE + 0x0048)) = 0x0001110d;

        *((V_UINT32P)(DRAMC0_BASE + 0x00d8)) = 0x80500900;   // Edward : Reg.0d8h.[31:30] PINMUX  01b:LPDDR2  10b:LPDDR3  11b:DDR3_16B
        *((V_UINT32P)(DDRPHY_BASE + 0x00d8)) = 0x80500900;

        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x008c)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x008c)) = 0x00000001;

        *((V_UINT32P)(DRAMC0_BASE + 0x0090)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0090)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0094)) = 0x80000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0094)) = 0x80000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x00dc)) = 0x83004004;
        *((V_UINT32P)(DDRPHY_BASE + 0x00dc)) = 0x83004004;

        //*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x19004004;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x19004004;
#ifdef SYSTEM_26M //Edwin
        *((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1c004004;
        *((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1c004004;
        *((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080022;
        *((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080022;
#else
    #ifdef REAL_MEMPLL_MDL //Edwin
        *((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
        *((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;
        *((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080033;
        *((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080033;
    #else
        //*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1c004004;  //orig
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1c004004;  //orig
        *((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
        *((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;
        *((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080033;
        *((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080033;
    #endif
#endif

        *((V_UINT32P)(DRAMC0_BASE + 0x00f0)) = 0x80000000;    // Edward : According to Brian, DQ4BMUX should set to 1 for all dram type.
        *((V_UINT32P)(DDRPHY_BASE + 0x00f0)) = 0x80000000;

        //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01000000;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01000000;
    #ifdef DRAMC_ASYNC
        //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x11100000;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x11100000;
//     *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01000000;      // PHYSYNCM, default to 0
//     *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01000000;
        *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;      // PHYSYNCM, default to 0
        *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #else
        //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01100000;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01100000;
//     *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01000000;
//     *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01000000;
        *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
        *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #endif

#ifdef PALLADIUM
        *((V_UINT32P)(DRAMC0_BASE + 0x0168)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0168)) = 0x00000000;
#else
        *((V_UINT32P)(DRAMC0_BASE + 0x0168)) = 0x00000080;
        *((V_UINT32P)(DDRPHY_BASE + 0x0168)) = 0x00000080;
#endif
        *((V_UINT32P)(DRAMC0_BASE + 0x00d8)) = 0x80700900;   // // Edward : Reg.0d8h.[31:30] PINMUX  01b:LPDDR2  10b:LPDDR3  11b:DDR3_16B
        *((V_UINT32P)(DDRPHY_BASE + 0x00d8)) = 0x80700900;

        //*((V_UINT32P)(DRAMC0_BASE + 0x0004)) = 0xf0048483; // Edward : AC timing change.
        //*((V_UINT32P)(DDRPHY_BASE + 0x0004)) = 0xf0048483;
        *((V_UINT32P)(DRAMC0_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;     // Edward : AC timing change.
        *((V_UINT32P)(DDRPHY_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;

        //*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = 0xc00646d1;
        //*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = 0xc00646d1;
    #ifdef DRAMC_ASYNC
        //*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = 0xa00632f1;    //DLE        // Edward : AC timing change.
        //*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = 0xa00632f1;
        *((V_UINT32P)(DRAMC0_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;    //DLE        // Edward : AC timing change.
        *((V_UINT32P)(DDRPHY_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;
    #else
        //*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = 0xa00632d1;
        //*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = 0xa00632d1;
        *((V_UINT32P)(DRAMC0_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;
        *((V_UINT32P)(DDRPHY_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;
        
    #endif

        *((V_UINT32P)(DRAMC0_BASE + 0x0028)) = 0xf1200f01;
        *((V_UINT32P)(DDRPHY_BASE + 0x0028)) = 0xf1200f01;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e0)) = 0x2001ebff;    //LPDDR2EN set to 0  
        *((V_UINT32P)(DDRPHY_BASE + 0x01e0)) = 0x2001ebff;    //LPDDR2EN set to 0

        *((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = 0x10000c00;    //LPDDR3EN set to 1 
        *((V_UINT32P)(DDRPHY_BASE + 0x01e8)) = 0x10000c00;    //LPDDR3EN set to 1

        *((V_UINT32P)(DRAMC0_BASE + 0x0158)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0158)) = 0x00000000;

#ifdef PALLADIUM

        *((V_UINT32P)(DRAMC0_BASE + 0x0400)) = 0x00111100;
        *((V_UINT32P)(DDRPHY_BASE + 0x0400)) = 0x00111100;

        *((V_UINT32P)(DRAMC0_BASE + 0x0404)) = 0x00000003;
        *((V_UINT32P)(DDRPHY_BASE + 0x0404)) = 0x00000003;

        *((V_UINT32P)(DRAMC0_BASE + 0x0408)) = 0x00222222;
        *((V_UINT32P)(DDRPHY_BASE + 0x0408)) = 0x00222222;

        *((V_UINT32P)(DRAMC0_BASE + 0x040c)) = 0x03330000;
        *((V_UINT32P)(DDRPHY_BASE + 0x040c)) = 0x03330000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0410)) = 0x03330000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0410)) = 0x03330000;

#else
        *((V_UINT32P)(DRAMC0_BASE + 0x0400)) = 0x00111100;
        *((V_UINT32P)(DDRPHY_BASE + 0x0400)) = 0x00111100;

        *((V_UINT32P)(DRAMC0_BASE + 0x0404)) = 0x00000002;
        *((V_UINT32P)(DDRPHY_BASE + 0x0404)) = 0x00000002;

        *((V_UINT32P)(DRAMC0_BASE + 0x0408)) = 0x00222222;
        *((V_UINT32P)(DDRPHY_BASE + 0x0408)) = 0x00222222;

        *((V_UINT32P)(DRAMC0_BASE + 0x040c)) = 0x33330000;
        *((V_UINT32P)(DDRPHY_BASE + 0x040c)) = 0x33330000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0410)) = 0x33330000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0410)) = 0x33330000;
#endif

//DQ16-31 [15:12]DRVP/[11:8]DRVN
#ifdef REXTDN_ENABLE
    reg_val = (drvp << 28) | (drvn << 24) | (drvp << 12) | (drvn << 8);
    *((volatile unsigned *)(DRAMC0_BASE + 0x00b4)) = reg_val;   //[15:12]DRVP/[11:8]DRVN, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00b4)) = reg_val;
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x00b4)) = emi_setting->DRAMC_DRVCTL0_VAL;   //[15:12]DRVP/[11:8]DRVN, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00b4)) = emi_setting->DRAMC_DRVCTL0_VAL;
#endif

    //*((volatile unsigned *)(DRAMC0_BASE + 0x00b4)) = 0xaa229922;   //[15:12]DRVP/[11:8]DRVN, default: 0xaa22aa22
    //*((volatile unsigned *)(DDRPHY_BASE + 0x00b4)) = 0xaa229922;

//DQS [31:28]DRVP/[27:24]DRVN
//DQ0-15 [15:12]DRVP/[11:8]DRVN
#ifdef REXTDN_ENABLE
    *((volatile unsigned *)(DRAMC0_BASE + 0x00b8)) = reg_val;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00b8)) = reg_val;
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x00b8)) = emi_setting->DRAMC_DRVCTL0_VAL;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00b8)) = emi_setting->DRAMC_DRVCTL0_VAL;
#endif    
    //*((volatile unsigned *)(DRAMC0_BASE + 0x00b8)) = 0x99229922;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
    //*((volatile unsigned *)(DDRPHY_BASE + 0x00b8)) = 0x99229922;

//CLK [31:28]DRVP/[27:24]DRVN;
//CMD [15:12]DRVP/[11:8]DRVN
#ifdef REXTDN_ENABLE
    *((volatile unsigned *)(DRAMC0_BASE + 0x00bc)) = reg_val;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00bc)) = reg_val;
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x00bc)) = emi_setting->DRAMC_DRVCTL0_VAL;   //[31:28]CLK DRVP/[27:24]CLK DRVN;[15:12]CMD DRVP/[11:8]CMD DRVN;, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00bc)) = emi_setting->DRAMC_DRVCTL0_VAL;
#endif

        *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x0b051311;
        *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x0b051311;

        //*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000004;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000004;
        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL
        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL

        __asm__ __volatile__ ("dsb" : : : "memory");        // Edward : add this according to 6589.
        gpt_busy_wait_us(200);//Wait > 200us              // Edward : add this according to 6589. Check with designer if it is needed.
    
        //*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = 0x0000003f;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = 0x0000003f;
        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_63;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_63;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(10);//Wait > 10us                              // Edward : add this according to DRAM spec, should wait at least 10us if not checking DAI.

        *((V_UINT32P)(DRAMC0_BASE + 0x0110)) &= (~0x7);  // Edward : Add this to disable  two ranks support for ZQ calibration tempority.
        *((V_UINT32P)(DDRPHY_BASE + 0x0110)) &= (~0x7);
        
        //*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = 0x00ff000a;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = 0x00ff000a;
        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_10;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_10;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);              //Wait > 1us. Edward : Add this because tZQINIT min value is 1us.

        // Edward : Add this for dual ranks support.
        if ( *(volatile unsigned *)(EMI_CONA)& 0x20000)  {
                //for chip select 1: ZQ calibration
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) |= (0x8);
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) |= (0x8);
          
                //*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = 0x00ff000a;
                //*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = 0x00ff000a;
                *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_10;
                *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_10;
          
                *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
                *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

                *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
                *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

                __asm__ __volatile__ ("dsb" : : : "memory");
                gpt_busy_wait_us(1);//Wait > 100us

                //swap back
                *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) &= (~0x8);
                *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) &= (~0x8);
                *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) |= (0x1);
                *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) |= (0x1);
        }       
        
        //*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = 0x00c30001;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = 0x00c30001;
        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_1;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_1;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us                  // Edward : 6589 has this delay. Seems no need.

        //*((V_UINT32P)(DRAMC0_BASE + 0x0088)) = 0x00060002;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0088)) = 0x00060002;
        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_2;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_2;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;           
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us                  // Edward : 6589 has this delay. Seems no need.

        //*((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = 0x00020003;            //Edward : no dram driving setting. Add this. May be not needed because default value is the same.
        //*((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = 0x00020003;
        *((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_3;            //Edward : no dram driving setting. Add this. May be not needed because default value is the same.
        *((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_3;


        *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

        *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00001100;               
        *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00001100;

        __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us          // Edward : 6589 has this delay. Seems no need.

        // Edward : add two rank enable here.
        if ( *(volatile unsigned *)(EMI_CONA)& 0x20000)  {
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112391;   
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112391;
        } else {
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112390;
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112390;
        }

        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL
        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL

        *((V_UINT32P)(DRAMC0_BASE + 0x01ec)) = 0x00000001;    // Edward : Add this to enable dual scheduler according to CC Wen. Should enable this for all DDR type. 
        *((V_UINT32P)(DDRPHY_BASE + 0x01ec)) = 0x00000001;    

        *((V_UINT32P)(DRAMC0_BASE + 0x0084)) = 0x00000a56;
        *((V_UINT32P)(DDRPHY_BASE + 0x0084)) = 0x00000a56;

        *((V_UINT32P)(DRAMC0_BASE + 0x000c)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x000c)) = 0x00000000;

        //*((V_UINT32P)(DRAMC0_BASE + 0x0000)) = 0x555844a3; // Edward : tRAS[3:0] 3h, TRC[7:4] ah, TWTR[11:8] 4h,  TWR[19:16] 8h, 
        //*((V_UINT32P)(DDRPHY_BASE + 0x0000)) = 0x555844a3;  // Edward : TFAW[23:20] 5h, TRP[27:24] 5h, TRCD[31:28] 5h
        *((V_UINT32P)(DRAMC0_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL; // Edward : tRAS[3:0] 3h, TRC[7:4] ah, TWTR[11:8] 4h,  TWR[19:16] 8h, 
        *((V_UINT32P)(DDRPHY_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;  // Edward : TFAW[23:20] 5h, TRP[27:24] 5h, TRCD[31:28] 5h

        // Edward : AC_TIME_0.5T control (new for 6582)
        //*((V_UINT32P)(DRAMC0_BASE + 0x01f8)) = 0x040026c1;  
        //*((V_UINT32P)(DDRPHY_BASE + 0x01f8)) = 0x040026c1;   //TWTR_M[26]=1,TFAW[13]=1,TWR_M[10]=1,TRAS[9]=1,TRP[7]=1,TRCD[6]=1,TRC[0]=1
        *((V_UINT32P)(DRAMC0_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;  
        *((V_UINT32P)(DDRPHY_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;   //TWTR_M[26]=1,TFAW[13]=1,TWR_M[10]=1,TRAS[9]=1,TRP[7]=1,TRCD[6]=1,TRC[0]=1

        //*((V_UINT32P)(DRAMC0_BASE + 0x0044)) = 0x28080401;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0044)) = 0x28080401;
        //*((V_UINT32P)(DRAMC0_BASE + 0x0044)) = 0xbf080481; // Edward : Modify to enable advance precharge and audio pattern.
        //*((V_UINT32P)(DDRPHY_BASE + 0x0044)) = 0xbf080481;  // Edward : TRFC=18h, TRFC[19:16]=8h
        *((V_UINT32P)(DRAMC0_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;  // Edward : Modify to enable advance precharge and audio pattern.
        *((V_UINT32P)(DDRPHY_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;   // Edward : TRFC=18h, TRFC[19:16]=8h
        

        //*((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = 0x11000d10;    //LPDDR3EN set to 1 // Edward : AC timing modification.
        //*((V_UINT32P)(DDRPHY_BASE + 0x01e8)) = 0x11000d10;    //LPDDR3EN set to 1
        *((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;    //LPDDR3EN set to 1 // Edward : AC timing modification.
        *((V_UINT32P)(DDRPHY_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;    //LPDDR3EN set to 1

        //333Mhz
        //*((V_UINT32P)(DRAMC0_BASE + 0x0004)) = 0xf00487c3;
        //*((V_UINT32P)(DRAMC0_BASE + 0x0000)) = 0x55d84608;
        //*((V_UINT32P)(DRAMC0_BASE + 0x0044)) = 0x28000401;
        //*((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = 0x00001010;

        //*((V_UINT32P)(DRAMC0_BASE + 0x0008)) = 0x0340633f; // REFCNT[7:0]=3fh although not use this. Instead, use FR clock.
        //*((V_UINT32P)(DDRPHY_BASE + 0x0008)) = 0x0340633f;
        *((V_UINT32P)(DRAMC0_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;    // REFCNT[7:0]=3fh although not use this. Instead, use FR clock.
        *((V_UINT32P)(DDRPHY_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;

        *((V_UINT32P)(DRAMC0_BASE + 0x0010)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0010)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x00f8)) = 0xedcb000f;
        *((V_UINT32P)(DDRPHY_BASE + 0x00f8)) = 0xedcb000f;

        *((V_UINT32P)(DRAMC0_BASE + 0x0020)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0020)) = 0x00000000;

        //*((V_UINT32P)(DRAMC0_BASE + 0x0640)) = 0xffc00033;    //enable DDRPHY dynamic clk gating  bit[4:5] should be enable for clock. bit[0:1] bypass : 1PLL. else 3PLL.
        //*((V_UINT32P)(DDRPHY_BASE + 0x0640)) = 0xffc00033;    //enable DDRPHY dynamic clk gating

#ifdef DRAMC_ASYNC
        *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xfb8<<20);  //enable DDRPHY dynamic clk gating, // disable D/C_PHY_M_CK dynamic gating
        *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfb8<<20);  //enable DDRPHY dynamic clk gating, // disable D/C_PHY_M_CK dynamic gating
#else
        *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xff8<<20);  //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
        *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xff8<<20);  //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
#endif
        *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0x3f00);  //enable Pre-Emphasis
        *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0x3f00);  //enable Pre-Emphasis

        //*((V_UINT32P)(DRAMC0_BASE + 0x01dc)) = 0x51642342;    //enable DDRPHY dynamic clk gating.  
        //*((V_UINT32P)(DDRPHY_BASE + 0x01dc)) = 0x51642342;    //Edward : REFCNT_FR_CLK[23:16]=64h, REFFRERUN[24]=1. TXREFCNT[15:8]=23h.
        *((V_UINT32P)(DRAMC0_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //enable DDRPHY dynamic clk gating.  
        *((V_UINT32P)(DDRPHY_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //Edward : REFCNT_FR_CLK[23:16]=64h, REFFRERUN[24]=1. TXREFCNT[15:8]=23h.

        *((V_UINT32P)(DDRPHY_BASE + 0x00bc)) &= (~0xff00ff00);
        *((V_UINT32P)(DDRPHY_BASE + 0x00bc)) |= (0xee << 24 | 0xcc << 8);
        *((V_UINT32P)(DDRPHY_BASE + 0x01a8)) = 0x09090909;    //bit0-3
        *((V_UINT32P)(DDRPHY_BASE + 0x01ac)) = 0x09090909;    //bit4-7
        *((V_UINT32P)(DDRPHY_BASE + 0x01b0)) = 0x09090909;    //bit8-11
        *((V_UINT32P)(DDRPHY_BASE + 0x01b4)) |= 0x9;          //bit12
        *((V_UINT32P)(DDRPHY_BASE + 0x01bc)) |= (0x9<<16);    //bit13
        *((V_UINT32P)(DDRPHY_BASE + 0x01c0)) = (0x0909<<16);  //bit14,15

        *((V_UINT32P)(DRAMC0_BASE + 0x00b8)) &= (~0xff00ff00);
        *((V_UINT32P)(DRAMC0_BASE + 0x00b8)) |= (0xcc << 24 | 0xcc << 8);

    return;

#endif //!LPDDR3
}


static char id[22];
static int emmc_nand_id_len=16;
static int fw_id_len;
static int mt_get_mdl_number (void)
{
    static int found = 0;
    static int mdl_number = -1;
    int i;
    int j;
    int has_emmc_nand = 0;
    int discrete_dram_num = 0;
    int mcp_dram_num = 0;

    unsigned int mode_reg_5;
    unsigned int dram_type;
    

    if (!(found))
    {
        int result=0;
        platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);
        for (i = 0 ; i < num_of_emi_records; i++)
        {
            if ((emi_settings[i].type & 0x0F00) == 0x0000) 
            {
                discrete_dram_num ++; 
            }
            else
            {
                mcp_dram_num ++; 
            }
        }

        /*If the number >=2  &&
         * one of them is discrete DRAM
         * enable combo discrete dram parse flow
         * */
        if ((discrete_dram_num > 0) && (num_of_emi_records >= 2))
        {
            /* if we enable combo discrete dram
             * check all dram are all same type and not DDR3
             * */
            enable_combo_dis = 1;
            dram_type = emi_settings[0].type & 0x000F;
            for (i = 0 ; i < num_of_emi_records; i++)
            {
                if (dram_type != (emi_settings[i].type & 0x000F))
                {
                    printf("[EMI] Combo discrete dram only support when combo lists are all same dram type.");
                    ASSERT(0);
                }
                if ((emi_settings[i].type & 0x000F) == TYPE_PCDDR3) 
                {
                    // has PCDDR3, disable combo discrete drame, no need to check others setting 
                    enable_combo_dis = 0; 
                    break;
                }
                dram_type = emi_settings[i].type & 0x000F;
            }
            
        } 
        printf("[EMI] mcp_dram_num:%d,discrete_dram_num:%d,enable_combo_dis:%d\r\n",mcp_dram_num,discrete_dram_num,enable_combo_dis);
        /*
         *
         * 0. if there is only one discrete dram, use index=0 emi setting and boot it.
         * */
        if ((0 == mcp_dram_num) && (1 == discrete_dram_num))
        {
            mdl_number = 0;
            found = 1;
            return mdl_number;
        }
            

        /* 1.
         * if there is MCP dram in the list, we try to find emi setting by emmc ID
         * */
        if (mcp_dram_num > 0)
        {
        result = platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);
    
        for (i = 0; i < num_of_emi_records; i++)
        {
            if (emi_settings[i].type != 0)
            {
                if ((emi_settings[i].type & 0xF00) != 0x000)
                {
                    if (result == 0)
                    {   /* valid ID */

                        if ((emi_settings[i].type & 0xF00) == 0x100)
                        {
                            /* NAND */
                            if (memcmp(id, emi_settings[i].ID, emi_settings[i].id_length) == 0){
                                memset(id + emi_settings[i].id_length, 0, sizeof(id) - emi_settings[i].id_length);                                
                                mdl_number = i;
                                found = 1;
                                break; /* found */
                            }
                        }
                        else
                        {
                            
                            /* eMMC */
                            if (memcmp(id, emi_settings[i].ID, emi_settings[i].id_length) == 0)
                            {
#if 0
                                printf("fw id len:%d\n",emi_settings[i].fw_id_length);
                                if (emi_settings[i].fw_id_length > 0)
                                {
                                    char fw_id[6];
                                    memset(fw_id, 0, sizeof(fw_id));
                                    memcpy(fw_id,id+emmc_nand_id_len,fw_id_len);
                                    for (j = 0; j < fw_id_len;j ++){
                                        printf("0x%x, 0x%x ",fw_id[j],emi_settings[i].fw_id[j]); 
                                    }
                                    if(memcmp(fw_id,emi_settings[i].fw_id,fw_id_len) == 0)
                                    {
                                        mdl_number = i;
                                        found = 1;
                                        break; /* found */
                                    }
                                    else
                                    {
                                        printf("[EMI] fw id match failed\n");
                                    }
                                }
                                else
                                {
                                    mdl_number = i;
                                    found = 1;
                                    break; /* found */
                                }
#else
                                mdl_number = i;
                                found = 1;
                                break; /* found */
#endif
                            }
                            else{
                                  printf("[EMI] index(%d) emmc id match failed\n",i);
                            }
                            
                        }
                    }
                }
            }
        }
        }
#if 1
        /* 2. find emi setting by MODE register 5
         * */
        // if we have found the index from by eMMC ID checking, we can boot android by the setting
        // if not, we try by vendor ID
        if ((0 == found) && (1 == enable_combo_dis))
        {
            unsigned int manu_id;
            /* DDR reserve mode no need to enable memory & test */
            if((g_ddr_reserve_enable==1) && (g_ddr_reserve_success==1))
            {
                unsigned int emi_cona;
                emi_cona = *(volatile unsigned *)(EMI_CONA);
                print("[DDR Reserve mode] EMI dummy read CONA = 0x%x in mt_get_mdl_number()\n", emi_cona);

                /*
                 * NOTE: 
                 * in DDR reserve mode, the DRAM access abnormal after DRAM_MRR(), so
                 * we use 0x10004100[7:0] to store DRAM's vendor ID, skip DRAM_MRR() after reboot;
                 * 0x10004100[7:0] is the dummy register.
                 * */
                print("[EMI] 0x10004100:%x\n\r",*(volatile unsigned *)(0x10004100));
                manu_id = (*(volatile unsigned *)(0x10004100) & 0xff);
            }
            else
            {
                EMI_SETTINGS *emi_set;
                //print_DBG_info();
                //print("-->%x,%x,%x\n",emi_set->DRAMC_ACTIM_VAL,emi_set->sub_version,emi_set->fw_id_length); 
                //print("-->%x,%x,%x\n",emi_setting_default.DRAMC_ACTIM_VAL,emi_setting_default.sub_version,emi_setting_default.fw_id_length); 
                dram_type = mt_get_dram_type_for_dis();
                if (TYPE_LPDDR2 == dram_type)
                {
                    print("[EMI] LPDDR2 discrete dram init\r\n");
                    emi_set = &emi_setting_default_lpddr2;
                    init_lpddr2(emi_set);
                }
                else if (TYPE_LPDDR3 == dram_type)
                {
                    print("[EMI] LPDDR3 discrete dram init\r\n");
                    emi_set = &emi_setting_default_lpddr3;
                    init_lpddr3(emi_set);
                }
                //print_DBG_info();
                if (dramc_calib() < 0) {
                    print("[EMI] Default EMI setting DRAMC calibration failed\n\r");
                } else {
                    print("[EMI] Default EMI setting DRAMC calibration passed\n\r");
                }
                manu_id = DRAM_MRR(0x5);
                /*
                 * NOTE: 
                 * in DDR reserve mode, the DRAM access abnormal after DRAM_MRR(), so
                 * we use 0x10004100[7:0] to store DRAM's vendor ID, skip DRAM_MRR() after reboot;
                 * 0x10004100[7:0] is the dummy register.
                 * */
                *(volatile unsigned *)(0x10004100) = (*(volatile unsigned *)(0x10004100) & (~0xff));
                *(volatile unsigned *)(0x10004100) = (*(volatile unsigned *)(0x10004100)|(manu_id & 0xff));
                print("[EMI] 0x10004100:%x\n\r",*(volatile unsigned *)(0x10004100));
            }


            print("[EMI] MR5:%x\n",manu_id);

            //try to find discrete dram by DDR2_MODE_REG5(vendor ID)
            for (i = 0; i < num_of_emi_records; i++)
            {
                if (TYPE_LPDDR2 == dram_type)
                    mode_reg_5 = emi_settings[i].LPDDR2_MODE_REG_5; 
                else if (TYPE_LPDDR3 == dram_type)
                    mode_reg_5 = emi_settings[i].LPDDR3_MODE_REG_5; 
                print("emi_settings[i].MODE_REG_5:%x,emi_settings[i].type:%x\n",mode_reg_5,emi_settings[i].type);
                //only check discrete dram type
                if ((emi_settings[i].type & 0x0F00) == 0x0000) 
                {
                    //support for compol discrete dram 
                    if ((mode_reg_5 == manu_id) )
                    {
                        mdl_number = i;
                        found = 1;
                        break; 
                    }
                }
            }
        }
#endif
        printf("found:%d,i:%d\n",found,i);    
    }
    

    return mdl_number;

}


int mt_get_dram_type (void)
{

    int n;
   /* if combo discrete is enabled, the dram_type is LPDDR2 or LPDDR3, depend on the emi_setting list*/
    if ( 1 == enable_combo_dis)
    return mt_get_dram_type_for_dis();

    n = mt_get_mdl_number ();

    if (n < 0  || n >= num_of_emi_records)
    {
        return 0; /* invalid */
    }

    return (emi_settings[n].type & 0xF);

}

int get_dram_rank_nr (void)
{

    int index;
    int emi_cona;
    index = mt_get_mdl_number ();
    if (index < 0 || index >=  num_of_emi_records)
    {
        return -1;
    }

#ifdef COMBO_MCP
    emi_cona = emi_settings[index].EMI_CONA_VAL;
#else
    emi_cona = EMI_CONA_VAL;
#if CFG_FPGA_PLATFORM
    return 1;
#endif
#endif
    return (emi_cona & 0x20000) ? 2 : 1;

}

/* This function returns available dram size for normal world. *
 * If it's called before TEE has been loaded, it returns real  * 
 * total dram size. Use it with care!!                         */
int get_dram_rank_size (int dram_rank_size[])
{
 int index,/* bits,*/ rank_nr, i;
    u32 secure_dram_size;

    //int emi_cona;
#if 1
    index = mt_get_mdl_number ();
    secure_dram_size = g_secure_dram_size;

    if (index < 0 || index >=  num_of_emi_records)
    {
        return;
    }

    rank_nr = get_dram_rank_nr();

    for (i = rank_nr - 1; i >= 0; i--) {
        dram_rank_size[i] = emi_settings[index].DRAM_RANK_SIZE[i];
	if (secure_dram_size >= dram_rank_size[i]) {
	    secure_dram_size -= dram_rank_size[i];
	    dram_rank_size[i] = 0;
	}
	else {
	    dram_rank_size[i] -= secure_dram_size;
	    secure_dram_size = 0;
	}
        printf("%d:dram_rank_size:%x\n",i,dram_rank_size[i]);
    }

    return;
#endif
}

unsigned int memory_size(void)
{   
    int nr_bank = 0;
    int i, size = 0;
    int rank_size[4];

    nr_bank = get_dram_rank_nr();

    get_dram_rank_size(rank_size);

    for (i = 0; i < nr_bank; i++)
        size += rank_size[i];

    print("DRAM size is 0x%x\n", size);

    return size;
}

void print_DBG_info(){
    unsigned int addr = 0x0;
        printf("=====================DBG=====================\n");
    for(addr = 0x0; addr < 0x600; addr +=4){
    printf("addr:%x, value:%x\n",addr, DRAMC_READ_REG(addr));
}
    printf("=============================================\n");
}

/*HQA HV LV PMIC Setting*/
//#define HQA
#define Vcore_HV (0x48 + 0xa)
#define Vcore_NV (0x48 + 0x00)
#define Vcore_LV (0x48 - 0xb)
#define Vmem_HV (0x0 + 0Xb)
#define Vmem_NV (0x0 + 0xF)
#define Vmem_LV (0x0 + 0x02)

/*
* mt_set_emi: Set up EMI/DRAMC.
*/
#if CFG_FPGA_PLATFORM
void mt_set_emi (void)
{
}
#else
void mt_set_emi (void)
{
    int index = 0;
    unsigned int val1,val2/*, temp1, temp2*/;
    EMI_SETTINGS *emi_set;
    *EMI_CONF = 0x004210000; // Enable EMI Address Scramble 

#ifdef HQA
    pmic_config_interface(0x21E, Vcore_LV, 0x7F,0);
    pmic_config_interface(0x554, Vmem_LV, 0XF,8);
#endif

#ifdef COMBO_MCP
switch (mt_get_dram_type ())
{
    case TYPE_mDDR:
        print("[EMI] DDR1\r\n");
        break;
    case TYPE_LPDDR2:
        print("[EMI] LPDDR2\r\n");
        break;
    case TYPE_LPDDR3:
        print("[EMI] LPDDR3\r\n");
        break;
    case TYPE_PCDDR3:
        print("[EMI] PCDDR3\r\n");
        break;
    default:
        print("[EMI] unknown dram type:%d\r\n",mt_get_dram_type());
        break;

}
index = mt_get_mdl_number ();
print("[EMI] eMMC/NAND ID = %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8],id[9],id[10],id[11],id[12],id[13],id[14],id[15]);
if (index < 0 || index >=  num_of_emi_records)
{
    print("[EMI] setting failed 0x%x\r\n", index);
    return;
}

print("[EMI] MDL number = %d\r\n", index);
emi_set = &emi_settings[index];

if ((emi_set->type & 0xF) == TYPE_LPDDR2)
{
    init_lpddr2(emi_set);
}
else if ((emi_set->type & 0xF) == TYPE_LPDDR3)
{
    init_lpddr3(emi_set);
}
else if ((emi_set->type & 0xF) == TYPE_PCDDR3)
{
    init_ddr3(emi_set);
}

    //print_DBG_info();
#else
    #ifdef DDRTYPE_LPDDR2	
    init_lpddr2(emi_set);
    #endif

    #ifdef DDRTYPE_DDR3
    init_ddr3(emi_set);
    #endif

    #ifdef DDRTYPE_LPDDR3	
    init_lpddr3(emi_set);
    #endif

#endif
#if 0
	/* save original PLL setting */
	temp1 = DRV_Reg32(AP_PLL_CON1);
	temp2 = DRV_Reg32(AP_PLL_CON2);
	temp1 = temp1 & 0xFF00FC0C;
	temp2 = temp2 & 0xFFFFFFF8;
	
	//SWITCH FROM SW TO HW SPM SLEEP CONTROL
	DRV_WriteReg32(AP_PLL_CON1, 0x0);
	DRV_WriteReg32(AP_PLL_CON2, 0x0);
	//*AP_PLL_CON1 = 0x0;
	//*AP_PLL_CON2 = 0x0;
#endif
	// Ungate dram transaction blockage mechanism after Dram Init.
	// If bit 10 of EMI_CONM is not 1, any transaction to EMI will be ignored.
	*EMI_CONM |= (1<<10);

	//DRAM Translate Off
	// Edward : After discussing with CC Wen, need to test different patterns like following  during calibration to see which one is worse in the future. If possible, do two is good.
	// 					TESTXTALKPAT(REG.48[16])		TESTAUDPAT (REG.44[7]) 
	// Audio pattern				0							1
	// Cross-talk					1							0
    //print_DBG_info();
    if (dramc_calib() < 0) {
        print("[EMI] DRAMC calibration failed\n\r");
    } else {
        print("[EMI] DRAMC calibration passed\n\r");
    }
    
    if ( *(volatile unsigned *)(EMI_CONA)& 0x20000) /* check dual rank support */ {
        unsigned int val1, val2;
        val1 = *((V_UINT32P)(DRAMC0_BASE + 0xe0)) & 0x07000000;
        val1 = ((val1 >> 24)) <<16; 
        val2 = *((V_UINT32P)(DRAMC0_BASE + 0x1c4)) & 0xFFF0FFFF | val1;
        *((V_UINT32P)(DRAMC0_BASE + 0x1c4)) = val2;
    }
    
    // Edward : enable DQS GW HW enable[31].  (Tracking value apply to 2 ranks[30] need to be set to 1 when if DFS is applied.)
    *((V_UINT32P)(DRAMC0_BASE + 0x01c0)) |= (0x80000000);   	// Edward : Need to make sure bit 13 is 0. 
    *((V_UINT32P)(DDRPHY_BASE + 0x01c0)) |= (0x80000000);     

    // Edward : Add following based on CC Wen request.
    //	* WFLUSHEN (REG.1EC[14]): 1
    //	* R_DMRWHPRICTL (REG.1EC[13]): 0
    //	* R_DMEMILLATEN (REG.1EC[11]): 1
    //	R/W aging (REG.1EC[10]) : 1
    //    R/W low latency (REG.1EC[9]) : 1
    //    R/W high priority (REG.1EC[8]) : 1
    //	R/W out of order enable (REG.1EC[4]) : 1
    *((V_UINT32P)(DRAMC0_BASE + 0x01ec)) |= (0x4f10);   	// Edward : Need to make sure bit 13 is 0. 
    *((V_UINT32P)(DDRPHY_BASE + 0x01ec)) |= (0x4f10);     

    /* disable CMPPD: Disable it because of power issue  */ 
    //DRAMC_WRITE_REG((DRAMC_READ_REG(0x1E4) | (0x1 << 13)),0x1E4);

#if 0
    /* revert PLL setting */
    DRV_WriteReg32(AP_PLL_CON1, temp1);
    DRV_WriteReg32(AP_PLL_CON2, temp2);
#endif

	#if 1
	if (mt_get_dram_type() == TYPE_PCDDR3)
	{//A15 enable: EMI_CONA bit17 1, TESTB bit7 1
		if(*((volatile unsigned int *)(EMI_BASE + 0x00e8)) & 0x80) //TESTB bit7
		{
			*(volatile unsigned int*)(EMI_CONA) |= 0x20000; //bit17 2 rank enable for A15
		}
	}
	#endif
	
    //print_DBG_info();
    //pmic_voltage_read(1);//check the pmic setting
    //print("[MEM]mt_get_dram_type:%d\n",mt_get_dram_type());
    /* FIXME: modem exception occurs if set the max pending count */
    /* To Repair SRAM */
    #if defined(CTP) || defined(SLT)
    #ifdef REPAIR_SRAM
    int repair_ret;
    //repair_ret = repair_sram();
    repair_ret = 0;
    if(repair_ret != 0){
        printf("Sram repair failed %d\n", repair_ret);
        while(1);
    }
    printf("Sram repaired ok\n");
    #endif
    #endif
    //print("MR5:%x,MR8:%x\n",DRAM_MRR(0x5),DRAM_MRR(0x8)); 
}
#endif

void in_sref()
{
    volatile unsigned int tmp;
    tmp = DRAMC_READ_REG(DRAMC0_BASE+0x4);
    if(tmp & (0x1 << 26))
        print("Enable sref %x\n", tmp);
    else
        print("Disable sref %x\n", tmp);
    tmp = DRAMC_READ_REG(DRAMC0_BASE+0x3B8);
    if(tmp & (0x1 << 16))
        print("In sref %x\n", tmp);
    else
        print("Not in sref %x\n", tmp);
    tmp = READ_REG(TOPRGU_BASE+0x40);
    if(tmp & 0x10000)
        print("success finish ddr reserved flow. %x\n", tmp);
    else
        print("failed finish ddr reserved flow. %x\n", tmp);
}

void release_dram()
{
#ifdef DDR_RESERVE_MODE  
    int counter = TIMEOUT;
    rgu_release_rg_dramc_conf_iso();
    rgu_release_rg_dramc_iso();
    rgu_release_rg_dramc_sref();
    while(counter)
    {
      if(rgu_is_dram_slf() == 0) /* expect to exit dram-self-refresh */
        break;
      counter--;
    }
    if(counter == 0)
    {
      if(g_ddr_reserve_enable==1 && g_ddr_reserve_success==1)
      {
        print("[DDR Reserve] release dram from self-refresh FAIL!\n");
        g_ddr_reserve_success = 0;
      }
    }
#endif    
}

void check_ddr_reserve_status()
{
#ifdef DDR_RESERVE_MODE  
    int counter = TIMEOUT;
    if(rgu_is_reserve_ddr_enabled())
    {
      g_ddr_reserve_enable = 1;
      if(rgu_is_reserve_ddr_mode_success())
      {
        while(counter)
        {
          if(rgu_is_dram_slf())
          {
            g_ddr_reserve_success = 1;
            break;
          }
          counter--;
        }
        if(counter == 0)
        {
          print("[DDR Reserve] ddr reserve mode success but DRAM not in self-refresh!\n");
          g_ddr_reserve_success = 0;
        }
      }
    else
      {
        print("[DDR Reserve] ddr reserve mode FAIL!\n");
        g_ddr_reserve_success = 0;
      }
    }
    else
    {
      print("[DDR Reserve] ddr reserve mode not be enabled yet\n");
      g_ddr_reserve_enable = 0;
    }
    
    /* release dram, no matter success or failed */
    release_dram();
#endif    
}
