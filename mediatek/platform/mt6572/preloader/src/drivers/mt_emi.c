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

#include <typedefs.h>
#include <platform.h>
#include <mt_emi.h>
#include <emi_hw.h>
#include "custom_emi.h"

//#include <dramc.h>
//#include <platform.h>
#include <mtk_pmic.h> 
#include "mtk_pll.h"

/****************************************************************************
*
* Macro.
*
****************************************************************************/
#define IMPROVE_DQS_STROBE
#define DQSI_MAX_DELAY_TAP_BY_DLL
//#define SAMPLE_BACK_USE
#define SW_WORKAROUND_FOR_DUALRANKS
#define US_CLOCKS 100
#define GRAY_ENCODED(a) ((a)^((a)>>1))

#define __EMI_DelayLoop(dly_val) \
    do { \
        volatile unsigned int dly; \
        for(dly = dly_val; dly != 0; dly--); \
    } while(0)

//#define EMI_DEBUG_LOG
//#define DUAL_RANK_LP1_LOG
		
#define REGION_0_KEY 0x12345678
#define REGION_1_KEY 0x23456788
#define REGION_2_KEY 0x34567898
#define REGION_3_KEY 0x45678908
#define REGION_4_KEY 0x56789018
#define REGION_5_KEY 0x67890128
#define REGION_6_KEY 0x78901238
#define REGION_7_KEY 0x89012348

/****************************************************************************
*
* Struct
*
****************************************************************************/

typedef struct
{
	unsigned long EMI_DLLV_regval;
	unsigned long EMI_DQSU_regval;
	unsigned long EMI_CONN_regval;
	unsigned long EMI_DQSA_regval;
	unsigned long EMI_DQSB_regval;
	unsigned long EMI_DQSC_regval;
	unsigned long EMI_DQSD_regval;
	unsigned long EMI_DQSI_regval;
	unsigned long EMI_IDLA_regval;
	unsigned long EMI_IDLB_regval;
	unsigned long EMI_IDLC_regval;
	unsigned long EMI_IDLD_regval;
	unsigned long EMI_IDLE_regval;
	unsigned long EMI_IDLF_regval;
	unsigned long EMI_IDLG_regval;
	unsigned long EMI_IDLH_regval;
	unsigned long EMI_IDLI_regval;
	unsigned long EMI_IDLJ_regval;
	unsigned long EMI_CALA_regval;
	unsigned long EMI_CALB_regval;
	unsigned long EMI_CALE_regval;
       unsigned long EMI_CALF_regval;
	unsigned long EMI_CALI_regval;
	unsigned long EMI_CALJ_regval;
	unsigned long EMI_CALP_regval;
} EMI_DATA_TRAIN_REG_t;

extern int num_of_emi_records;
extern EMI_SETTINGS emi_settings[];
extern int mtk_pll_init_emi(unsigned int);
DRAM_INFO board_dram_info = {0x0, 0x0, 0x0, 0x0};

static void gpt_wait_us_dsb(U32 time)
{
	__asm__ __volatile__ ("dsb" : : : "memory");
	gpt_busy_wait_us(time);
}

#if 0
static void set_emi_clk_26M(U32 enable)
{
	U32 EMI_reg;

	EMI_reg = DRV_Reg32(CLK_SEL_0); // CLK_SEL_0
	if (enable == 1)
	{	
	    DRV_WriteReg32(CLK_SEL_0, EMI_reg & 0xFFFFFFEF);
	}
	else if (enable == 0)
	{
	    DRV_WriteReg32(CLK_SEL_0, EMI_reg | 0x10);
	}
	gpt_wait_us_dsb(10);	
}
#endif

kal_int32 get_mode_reg(U32 rank_num, U32 MR_num)
{
	U32 MR_value;
	
	*EMI_DDRV |= 1<< (4 + rank_num);	
	*EMI_CONI = (MR_num << 16) | (1 <<15);	
	gpt_wait_us_dsb(1);	 	
				
	*EMI_CONN |= 0x40000000;			
	gpt_wait_us_dsb(10);	 	
	*EMI_CONN &= (~0x40000000);		
		
	gpt_wait_us_dsb(20);	 	
	MR_value = *EMI_DDRV;
	print("[EMI] bank%d, Mode Reg%d=0x%X\n", rank_num, MR_num, MR_value);	 	
	 	
	*EMI_DDRV &= ~(1<< (4 + rank_num));			
	return ((MR_value & 0x00FF0000) >>16);
}

int get_board_dram_info(void)
{
	U32 EMI_reg, MR_tmp, i;
	U32 MR32_r0, MR32_r1, MR40_r0, MR40_r1, MR8_r0, MR8_r1, MR5_r0, MR5_r1;
	U32 retry_cnt;
	
		
	if ((*EMI_CONM & 0x80000000) == 0x80000000) // if DRAMC detect PCDDR3
	{
		board_dram_info.dram_type = PCDDR3;
		return 0;		
	}	
	else if ((*EMI_CONM & 0x10000000) == 0x10000000) // if DRAMC detect LPDDR1
	{
		board_dram_info.dram_type = DDR1;
		return 0;			
	}
	else if ((*EMI_CONM & 0x20000000) == 0x20000000) // if DRAMC detect LPDDR2/LPDDR3
       {
       
       	//set_emi_clk_26M(1);
       
               /**
       	* Disable Dummy Read.
       	*/
       	*EMI_DRCT = 0x0;
       
               /* Add for MT6571 EMI DCM */
               //*EMI_CONM |= 0x00000100;  
       
       	/**
       	* Set DRAM rank.
       	*/
       	*EMI_GEND = 0x00030000; // enable rank0 and rank1
       
       	/**
              * EMI Driving, it needs to be set before LPDDR being initialized.
       	*/
       	*EMI_DRVA = 0xCCCCCCCC;
       	*EMI_DRVB = 0x00CC0000;
       
       	*EMI_IOCL = 0x00030000;  //MT6280 remove "Pad swap function mode"
       
       	/**
              * Set AC Timing Parameters for according memory device.
       	*/
       	*EMI_CONJ = 0x00021011;
       	*EMI_CONK = 0x02050120;
       	
       	*EMI_CONL = 0x004230B7;	//RL = 6, WL = 3, rd_del_sel = 8T
       	
       	// set dram type to lpddr3_32, also can set to lpddr2-32, since we just need to read mode register 	
       	*EMI_CONN = 0x00410800;	
       	
       	*EMI_ODLJ = 0x08080808;
       	*EMI_ODLN = 0x08080808;	
       
       	/**
       	   * LPDDR Initial Flow.
       	   */
       	/**
       	   * [Initial Flow 1]:Power Ramp
       	   */
       
       	/**
       	   * [Initial Flow 2]: clock -> 5x tCK -> CKE
        	   * Enable Clock ( DRAM clk out / delay-line HCLKX2_CK/ SRAM clk center-align /CKE_EN )
       	   */
       	   *EMI_GENA = 0x00000202; //enable  DCLK_EN
       
       	  /* Delay 5 tCK */
       	   
       	   gpt_wait_us_dsb(5);
       
       	   *EMI_GENA |= 0x00000010; //enable CKE
       
       	   /* Delay 200us */
       	   
       	   gpt_wait_us_dsb(300);

	         /* precharge all */
	#if 1  
	    EMI_reg = *EMI_CONN; 	  
	   *EMI_CONN = EMI_reg | 0x1 | 0x10000000;
	    gpt_wait_us_dsb(10);
	   *EMI_CONN = EMI_reg | 0x1;	   
	   gpt_wait_us_dsb(10);
	#endif
	
       	/**
       	   * [Initial Flow 3]:Reset Command
       	   */
       	EMI_reg = *EMI_CONN;	   
       	*EMI_CONI = 0x003F0000;
       	*EMI_CONN = EMI_reg | 0x1 | 0x20000000;
       	 
       	 gpt_wait_us_dsb(10);
       	*EMI_CONN = EMI_reg | 0x1;
       
       	/**
       	   * [Initial Flow 4]:Mode Registers Reads and Device Auto-Initialization (DAI) polling or wait 10us
       	   */
       
       	gpt_wait_us_dsb(20);
       
       #if 1
       	/**
       	   * [Initial Flow 5]:ZQ Calibration
       	   */
       	*EMI_DDRV &= (~0x32);
       	EMI_reg = *EMI_CONN;
       	//Rank 0 ZQ calibration
       	*EMI_DDRV |= 0x12;			
       	*EMI_CONI = 0xFF0A0000;
       	*EMI_CONN = EMI_reg | 0x1 | 0x20000000;
       
       	gpt_wait_us_dsb(10);
       	*EMI_CONN = EMI_reg | 0x1;
       	/* Wait 1us */
       
       	gpt_wait_us_dsb(10);
       	
       	*EMI_DDRV &= (~0x32);
       	
       	// Rank 1 ZQ calibration
       	*EMI_DDRV |= 0x22;			 
          	*EMI_CONI = 0xFF0A0000;
          	*EMI_CONN = EMI_reg | 0x1 | 0x20000000;
              
           gpt_wait_us_dsb(10);
       	*EMI_CONN = EMI_reg | 0x1;
       	/* Wait 1us */
              
        	gpt_wait_us_dsb(10);
       
       	*EMI_DDRV &= (~0x32);
       #endif
       
       	/**
       	  * Set Device Feature
       	  */
       	// Set Device Feature1 - nWR=3, WC=Wrap, BT=sequential, BL=8
       	*EMI_CONI = 0x23010000;
       	*EMI_CONN = EMI_reg | 0x1 | 0x20000000; //MRW to mode register 1
       
       	gpt_wait_us_dsb(10);
       	*EMI_CONN = EMI_reg | 0x1;
       
       	gpt_wait_us_dsb(10);
       
           /* RL = 6, WL = 3 */
           *EMI_CONI = 0x04020000;    
           
       	*EMI_CONN = EMI_reg | 0x1 | 0x20000000; //MRW to mode register 2
       
       	gpt_wait_us_dsb(10);
       	*EMI_CONN = EMI_reg | 0x1;
       
       	gpt_wait_us_dsb(10);
       
       	*EMI_PPCT = 0x00000008;		
       	*EMI_GENA = 0x0000821A;
       	*EMI_CALE = 0x1F1F1F1F;
       
       	// Clear Initial Bits and disable auto refresh, since any EMI command may influence MRR reading
       	*EMI_CONN &= ~0xFF000002; 
       
       	*EMI_DQSA = *EMI_DQSB = *EMI_DQSC = *EMI_DQSD = 0x00200020;
       	/**
              * Enable 1/5 PLL.
              */
              //*EMI_CONN |= 0x00000100;
       
             // disabe dummy read , since any EMI command to device will  influence MRR reading
            *EMI_DRCT &= (~0x1);    
       
            gpt_wait_us_dsb(1000);
                
       #if 0
       	for(i=0; i<=1; i++)
       	{
       	 print("[EMI] Dump bank 0x%d Mode Register \n", i);	
       	get_mode_reg(i, 0);	
       	get_mode_reg(i, 4);	
       	get_mode_reg(i, 5);
       	get_mode_reg(i, 6);
       	get_mode_reg(i, 7);	
       	get_mode_reg(i, 8);
       	get_mode_reg(i, 32);
       	get_mode_reg(i, 40);
       	print("\n");
       	}
       #endif	
	       for (retry_cnt = 0; retry_cnt < 3; retry_cnt++)
	       {
              	MR8_r0 = 	get_mode_reg(0, 8);  //type¡BDensity¡BIO width
              	MR32_r0 = 	get_mode_reg(0, 32);  //DQ calibration pattern A, should be 0x1
              	MR40_r0 = 	get_mode_reg(0, 40);  //DQ calibration pattern B, should be 0x0
              	MR5_r0 = 	get_mode_reg(0, 5); //vendor ID
              	MR32_r1 = 	get_mode_reg(1, 32);
              	MR40_r1 = 	get_mode_reg(1, 40);
              	MR_tmp = 	get_mode_reg(0, 8);	
              	MR8_r1 = 	get_mode_reg(1, 8);	
              	MR5_r1 = 	get_mode_reg(1, 5);	
              
              	if (((MR32_r0 & 0x1) == 0x1) && ((MR40_r0 & 0x1) == 0x0) && (MR8_r0 == MR_tmp)) // MR32 bit 0 should be 1, MR40 bit 0 should be 0
              	{
                            // board_dram_info.vendor_id = get_mode_reg(0, 5);
              		board_dram_info.vendor_id = MR5_r0;
              		board_dram_info.rank0_size = 1 << ( ((MR8_r0 & 0x3C) >> 2) + 23);
              		board_dram_info.rank1_size = 0x0;
              		if ((MR8_r0 & 0x3) == 0x3) // 0x3 of MR8 is for S8 SDRAM
              		{
              			board_dram_info.dram_type = LPDDR3;
              		}
              		else
              		{
              			board_dram_info.dram_type = DDR2;
              		}
              		
              		if((MR32_r0 == MR32_r1) && (MR40_r0 == MR40_r1) && (MR5_r0 == MR5_r1))
              		{
              			board_dram_info.rank1_size = 1 << ( ((MR8_r1 & 0x3C) >> 2) + 23);		
              		}
                            break;
			}
              	else
              	{
              		print("[EMI] get DRAM Info fail, retry=%d \n", retry_cnt);
				*((volatile unsigned int *)(0x80000000)) = 0x5a5aa5a5 + retry_cnt;
				*((volatile unsigned int *)(0x80000010)) = 0xa5a55a5a + retry_cnt;
                            //#ifdef CM_COMBO_TEST
				//print("[EMI] 0x80000000=0x%X, 0x80000010=0x%X\n", *((volatile unsigned int *)(0x80000000)) ,*((volatile unsigned int *)(0x80000010)));
                            //#endif
				gpt_wait_us_dsb(1000);					
              	}
	       }
           	//set_emi_clk_26M(0);
           	*EMI_GENA &= ~(0x00000210); //disable  CLK and CKE
           	gpt_wait_us_dsb(50);

		if (retry_cnt >= 3)
		{
			return -1;
		}
           	return 0;
       }
	else
	{
		return -1;	
	}
}


/*
 * init_dram: Do initialization for LPDDR.
 */
static void init_lpddr1(EMI_SETTINGS *emi_setting)
{
       *EMI_GENA |= (0x0080);	//DCMDQ depth = 4

	/**
	   * Set LPDDR device configuration.
	   */
	*EMI_CONI = emi_setting->EMI_CONI_value;

	/**
	   * Enable external clock (DRAM clk out & HCLKX2_CK).
	   */
	*EMI_GENA |= 0x00000200;

	/**
	   * Remap if necessary.
	   */

	*EMI_GENA &= (~0xB);
	*EMI_GENA |= 0x0A;

	  /**
	   * Delay for 200us.
	   */
	gpt_wait_us_dsb(300);

	/**
	   * LPDDR Initial Flow.
	   */
	*(volatile kal_uint32*)EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	gpt_wait_us_dsb(10);
	*(volatile kal_uint32*)EMI_CONN = emi_setting->EMI_CONN_value | 0x1 |0x10000000;

	gpt_wait_us_dsb(10);
	*(volatile kal_uint32*)EMI_CONN = emi_setting->EMI_CONN_value | 0x1 |0x08000000;

	gpt_wait_us_dsb(10);
	*(volatile kal_uint32*)EMI_CONN = emi_setting->EMI_CONN_value | 0x1 |0x04000000;

	gpt_wait_us_dsb(10);
	*(volatile kal_uint32*)EMI_CONN = emi_setting->EMI_CONN_value | 0x1 |0x02000000;

	gpt_wait_us_dsb(10);
	*(volatile kal_uint32*)EMI_CONN = emi_setting->EMI_CONN_value | 0x1 |0x01000000;

	gpt_wait_us_dsb(10);
	*(volatile kal_uint32*)EMI_CONN = emi_setting->EMI_CONN_value | 0x1 |0x00000000;

	gpt_wait_us_dsb(10);

	return;
}

/*
 * init_dram: Do initialization for LPDDR2.
 */
static void init_lpddr2(EMI_SETTINGS *emi_setting)
{
	/**
	   * Remap if necessary.
	   */

	*EMI_GENA &= (~0xB);
	*EMI_GENA |= 0x0A;

	/**
	   * Delay for a while.
	   */

	gpt_wait_us_dsb(10);

	/**
	   * LPDDR Initial Flow.
	   */
	/**
	   * [Initial Flow 1]:Power Ramp
	   */

	/**
	   * [Initial Flow 2]: clock -> 5x tCK -> CKE
 	   * Enable Clock ( DRAM clk out / delay-line HCLKX2_CK/ SRAM clk center-align /CKE_EN )
	   */
	   *EMI_GENA |= 0x00000200; //enable  DCLK_EN

	  /* Delay 5 tCK */
	   
	   gpt_wait_us_dsb(5);

	   *EMI_GENA |= 0x00000010; //enable CKE

	   /* Delay 200us */
	   
	   gpt_wait_us_dsb(300);

	/**
	   * [Initial Flow 3]:Reset Command
	   */
	*EMI_CONI = 0x003F0000;
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;
	 
	 gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	/**
	   * [Initial Flow 4]:Mode Registers Reads and Device Auto-Initialization (DAI) polling or wait 10us
	   */

	gpt_wait_us_dsb(20);

	/**
	   * [Initial Flow 5]:ZQ Calibration
	   */
	*EMI_DDRV &= (~0x32);

	*EMI_DDRV |= 0x12;			//Rank 0 ZQ calibration
	*EMI_CONI = 0xFF0A0000;
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;

	gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;
	/* Wait 1us */

	gpt_wait_us_dsb(10);

	*EMI_DDRV &= (~0x32);

	if (2 == get_dram_rank_nr())
	{
       	*EMI_DDRV |= 0x22;			 // Rank 1 ZQ calibration
       	*EMI_CONI = 0xFF0A0000;
       	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;
       
       	gpt_wait_us_dsb(10);
       	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;
       	/* Wait 1us */
       
       	gpt_wait_us_dsb(10);

	*EMI_DDRV &= (~0x32);
	}

	/**
	  * Set Device Feature
	  */
	// Set Device Feature1 - nWR=3, WC=Wrap, BT=sequential, BL=8
	*EMI_CONI = 0x23010000;
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;

	gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	gpt_wait_us_dsb(10);

       if (emi_setting->EMI_Freq == 200)
       {
            /* RL = 3, WL = 1 */
           *EMI_CONI = 0x01020000;
       }
	else if (emi_setting->EMI_Freq == 266)
	{
#ifdef SW_WORKAROUND_FOR_DUALRANKS	
            if (2 == get_dram_rank_nr())
            {
		  /* RL = 5, WL = 2 */
		 *EMI_CONI = 0x03020000;
            }
	     else
#endif		 	
	     {	     
        /* RL = 4, WL = 2 */
       *EMI_CONI = 0x02020000;
	}
	}
	else if (emi_setting->EMI_Freq == 333)
	{
	/* RL = 5, WL = 2 */
       *EMI_CONI = 0x03020000;
	}
	else if (emi_setting->EMI_Freq == 26)
       {
        /* RL = 4, WL = 2 */
       *EMI_CONI = 0x02020000;
	}

	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;

	gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	gpt_wait_us_dsb(10);

	// add for LPDDR2 DRAM driving setting
	*EMI_CONI = emi_setting->EMI_CONI_value;
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;

	gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	gpt_wait_us_dsb(10);

	// Clear Initial Bits
	*EMI_CONN &= ~0xFF000000;

	if ((*EMI_CONN & 0x00000800) != 0)
	{
	    //Enable perback auto-refresh for LPDDR2 8 banks
	    *EMI_CONN &=  ~0x00000200;
	}
	else
	{
	    //Disable perback auto-refresh
	    *EMI_CONN &=  ~0x00000200;
	}
}


/*
 * init_dram: Do initialization for LPDDR3.
 */
#define DDR3_MODE_0 0x00000000
#define DDR3_MODE_1 0x00400000
#define DDR3_MODE_2 0x00800000
#define DDR3_MODE_3 0x00c00000
unsigned int ddr_type=0;
static void init_lpddr3(EMI_SETTINGS *emi_setting) //LPDDR3 // CM [20121115] wait for implement
{
	/**
	   * Remap if necessary.
	   */

	*EMI_GENA &= (~0xB);
	*EMI_GENA |= 0x0A;

	/**
	   * Delay for a while.
	   */

	gpt_wait_us_dsb(10);

	/**
	   * LPDDR Initial Flow.
	   */
	/**
	   * [Initial Flow 1]:Power Ramp
	   */

	/**
	   * [Initial Flow 2]: clock -> 5x tCK -> CKE
 	   * Enable Clock ( DRAM clk out / delay-line HCLKX2_CK/ SRAM clk center-align /CKE_EN )
	   */
	   *EMI_GENA |= 0x00000200; //enable  DCLK_EN

	  /* Delay 5 tCK */
	   
	   gpt_wait_us_dsb(5);

	   *EMI_GENA |= 0x00000010; //enable CKE

	   /* Delay 200us */
	   
	   gpt_wait_us_dsb(300);

	/**
	   * [Initial Flow 3]:Reset Command
	   */
	*EMI_CONI = 0x003F0000;
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;
	 
	 gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	/**
	   * [Initial Flow 4]:Mode Registers Reads and Device Auto-Initialization (DAI) polling or wait 10us
	   */

	gpt_wait_us_dsb(20);

	/**
	   * [Initial Flow 5]:ZQ Calibration
	   */
	*EMI_DDRV &= (~0x32);

	*EMI_DDRV |= 0x12;			//Rank 0 ZQ calibration
	*EMI_CONI = 0xFF0A0000;
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;

	gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;
	/* Wait 1us */

	gpt_wait_us_dsb(10);

	*EMI_DDRV &= (~0x32);

	if (2 == get_dram_rank_nr())
	{
       	*EMI_DDRV |= 0x22;			 // Rank 1 ZQ calibration
       	*EMI_CONI = 0xFF0A0000;
       	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;
       
       	gpt_wait_us_dsb(10);
       	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;
       	/* Wait 1us */
       
       	gpt_wait_us_dsb(10);

	*EMI_DDRV &= (~0x32);
	}

	/**
	  * Set Device Feature
	  */
	// Set Device Feature1 - nWR=3, WC=Wrap, BT=sequential, BL=8
	*EMI_CONI = 0x23010000;
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;

	gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	gpt_wait_us_dsb(10);

       /* RL = 6, WL = 3 */
	*EMI_CONI = 0x04020000;

	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;

	gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	gpt_wait_us_dsb(10);

	// add for LPDDR2 DRAM driving setting
	*EMI_CONI = emi_setting->EMI_CONI_value;
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1 | 0x20000000;

	gpt_wait_us_dsb(10);
	*EMI_CONN = emi_setting->EMI_CONN_value | 0x1;

	gpt_wait_us_dsb(10);

	// Clear Initial Bits
	*EMI_CONN &= ~0xFF000000;

#ifdef SW_WORKAROUND_FOR_DUALRANKS
       if (2 == get_dram_rank_nr())
       {
	    *EMI_ODLA = *EMI_ODLB = *EMI_ODLC = *EMI_ODLD = *EMI_ODLE = *EMI_ODLF = *EMI_ODLG = *EMI_ODLH = 0x0F0F0F0F;
	    *EMI_ODLI = *EMI_ODLJ = *EMI_ODLK = *EMI_ODLL = *EMI_ODLM = *EMI_ODLN = 0x0F0F0F0F;
	}
#endif

	if ((*EMI_CONN & 0x00000800) != 0)
	{
	    //Enable perback auto-refresh for LPDDR2 8 banks
	    *EMI_CONN &=  ~0x00000200;
	}
	else
	{
       //Disable perback auto-refresh
	*EMI_CONN &=  ~0x00000200;
	}
}

static void init_pcdram3(EMI_SETTINGS *emi_setting)
{
      if (0x00070000 == (emi_setting->EMI_CONN_value & 0x00070000))	//PCDDR3x32
      {
	   *EMI_GENA &= ~(0x00C0);	 //DCMDQ depth = 6
	}
	else  //PCDDR3x16
	{
		*EMI_GENA &= ~(0x00C0);
		*EMI_GENA |= (0x00C0);	  //DCMDQ depth = 3
	}

	/**
	   * Enable external clock (DRAM clk out & HCLKX2_CK).
	   */
	*EMI_GENA |= 0x00000200;

	/**
	   * Remap if necessary.
	   */

	*EMI_GENA &= (~0xB);
	*EMI_GENA |= 0x0A;

	/**
	   * PCDDR3 Initial Flow.
	   */
	 /* Delay 200us */

	gpt_wait_us_dsb(300);

	 /* disable RESET */
	 *EMI_GENA |= 0x00002000;
	  /* Delay 500us */
	  
	  gpt_wait_us_dsb(750);

	 /* Enable CKE */
	  *EMI_GENA |= 0x00000010;
	  /* Delay TRFC + 10ns */
	  
	  gpt_wait_us_dsb(10);

	 /* Load MR2 */
	 *EMI_CONI = 0x40000000;
	 *EMI_CONN = emi_setting->EMI_CONN_value | 0x02000000;
	 
	 gpt_wait_us_dsb(10);
	 *EMI_CONN = emi_setting->EMI_CONN_value;
	  /* Delay 5us */
	  
	  gpt_wait_us_dsb(10);

	 /* Load MR3 */
	 *EMI_CONI = 0x60000000;
	 *EMI_CONN = emi_setting->EMI_CONN_value | 0x02000000;
	 
	 gpt_wait_us_dsb(10);
	 *EMI_CONN = emi_setting->EMI_CONN_value;
	  /* Delay 5us */
	  
	  gpt_wait_us_dsb(10);

	 /* Load MR1 , Enable DLL */
	 *EMI_CONI = emi_setting->EMI_CONI_value;
	 *EMI_CONN = emi_setting->EMI_CONN_value |0x02000000;
	 
	 gpt_wait_us_dsb(10);
	 *EMI_CONN = emi_setting->EMI_CONN_value;
	  /* Delay 5us */
	  
	  gpt_wait_us_dsb(10);

	 /* Load MR0, Reset DLL */
	 *EMI_CONI = 0x13280000;
	 *EMI_CONN = emi_setting->EMI_CONN_value | 0x02000000;
	 
	 gpt_wait_us_dsb(10);
	 *EMI_CONN = emi_setting->EMI_CONN_value;
	  /* Delay 5us */

         gpt_wait_us_dsb(10);

	  /**
	  * Enable ZQ Calibration
	  */
	 *EMI_DDRV &= (~0x32);
	 *EMI_DDRV |= 0x12; 		 //Rank 0 ZQ calibration
	 *EMI_DDRV |= 0x01; 

	gpt_wait_us_dsb(100);
	 *EMI_DDRV &= (~0x01); 
	 /* Wait 1us */ 

	 gpt_wait_us_dsb(1);
	 *EMI_DDRV &= (~0x32);		

	if (2 == get_dram_rank_nr())
	{
	 *EMI_DDRV |= 0x22; 		  // Rank 1 ZQ calibration
	 *EMI_DDRV |= 0x01; 
	
		gpt_wait_us_dsb(100);
	 *EMI_DDRV &= (~0x01); 
	 /* Wait 1us */ 
	
	 gpt_wait_us_dsb(1); 	
	 *EMI_DDRV &= (~0x32);   
	}
}

/*************************************************************************
* FUNCTION
*  mt_get_mdl_number()
*
* DESCRIPTION
*   Get combo memory index
*
* PARAMETERS
*   NONE
*
* RETURNS
*  0 : index 0
*  1 : index 1
*  2 : index 2
*
* GLOBALS AFFECTED
*
*************************************************************************/
static char id[22];
static int emmc_nand_id_len=16;
static int fw_id_len;
static int mt_get_mdl_number (void)
{
    static int found = 0;
    static int mdl_number = -1;
    volatile int match_count = 0;
    int i;

    if (!found)
    {
        int result = 0;
#if (CFG_FPGA_PLATFORM)
        mdl_number = 0; found = 1; return mdl_number;
#endif

#if 0 //defined(SAMPLE_BACK_USE)
         mdl_number = 0;  
/*
	 if ((*EMI_CONM & 0x10000000) == 0x10000000)
	 {
	     mdl_number = 1;  //NAND + LPDDR1
	 }
        else
        {
	     mdl_number = 0;  //eMMC + LPDDR2
        }
*/        
        found = 1;
#else
        result = platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);
	if (result != 0)
	{
	    print("[EMI] get flash ID fail\n");
        }

        for (i = 0; i < num_of_emi_records; i++)
        {
            if (emi_settings[i].type != 0) // if valid dram type
            {
                if ((emi_settings[i].type & 0xF00) != 0x000) // if MCP DRAM               
                {
                        if ((emi_settings[i].type & 0xF00) == 0x100)
                        {
                            /* NAND */
                            if (memcmp(id, emi_settings[i].Flash_ID, emi_settings[i].flash_id_length) == 0){
                                memset(id + emi_settings[i].flash_id_length, 0, sizeof(id) - emi_settings[i].flash_id_length);
				    mdl_number = i;
				    //found = 1;
                                //break; /* found */
				    emi_settings[i].match_flag = 1;
				    match_count++;                                
                            }
                        }
                        else
                        {
                            /* eMMC */
                            if (memcmp(id, emi_settings[i].Flash_ID, emi_settings[i].flash_id_length) == 0)
                            {
                                mdl_number = i;
				    //found = 1;
                                //break; /* found */
				    emi_settings[i].match_flag = 1;
				    match_count++;                                
                            }
                        }                    
                }
            }
        }

		if(match_count == 1)
		{
		    found = 1;
                }
		else if(match_count >= 1) // more than 1 flash match,
		{
		      //print("[EMI] more than 1 flash ID match!\n");  
		      result = get_board_dram_info();			
       	       for (i = 0; i < num_of_emi_records; i++)
               	{
               		if ((emi_settings[i].match_flag == 1) && ((emi_settings[i].type & 0xF) == board_dram_info.dram_type))    
               		{
               			if(((emi_settings[i].type & 0xF) == 0x001) ||((emi_settings[i].type & 0xF) == 0x003) )
               			{
               				mdl_number = i;
               				found = 1;
               				break;
                                    }
               			// check lpddr2 and lpddr3
               			else if((emi_settings[i].DRAM_ID == board_dram_info.vendor_id) &&
               			  	(emi_settings[i].DRAM_RANK_SIZE[0] == board_dram_info.rank0_size) &&
               			  	(emi_settings[i].DRAM_RANK_SIZE[1] == board_dram_info.rank1_size))
                       	        {
       		      			mdl_number = i;
       		      			found = 1;
                           	              break;
                        }
                    }
                      }
               	
           		 if (found == 0)
		      {
          		 	print("[EMI] %d flash ID match, no DRAM info match !\r\n", match_count);
          		 	ASSERT(0);
		      	}
                }
		
		else if(match_count == 0) // no flash match, check discrete DRAM
		{
			result = get_board_dram_info();
	              for (i = 0; i < num_of_emi_records; i++)
        		{
              		if (((emi_settings[i].type & 0xF00) == 0x000) && ((emi_settings[i].type & 0xF) == board_dram_info.dram_type))
              		{
              			if(((emi_settings[i].type & 0xF) == 0x001) ||((emi_settings[i].type & 0xF) == 0x003) )
                                  {
              				mdl_number = i;
              				found = 1;
              				break;
              			}				  		      		
              			else if((emi_settings[i].DRAM_ID == board_dram_info.vendor_id) &&
              			  	(emi_settings[i].DRAM_RANK_SIZE[0] == board_dram_info.rank0_size) &&
              			  	(emi_settings[i].DRAM_RANK_SIZE[1] == board_dram_info.rank1_size))
                {
		      mdl_number = i;
		      found = 1;
                    break;
                }
            }
        }
            	
         		if (found == 0)
         		{
        			 	print("[EMI] no flash ID match, no DRAM info match !\r\n");
        			 	ASSERT(0);
         		}            	
               }
#endif
    }

    if (found == 0)
    {
       print("[EMI] no MDL match !\r\n");
    }

    return mdl_number;
}

/*************************************************************************
* FUNCTION
*  mt_get_dram_type()
*
* DESCRIPTION
*   Get DRAM type
*
* PARAMETERS
*   NONE
*
* RETURNS
*  1 : LPDDR1
*  2 : LPDDR2
*  3 : PCDDR3/LPDDR3
*
* GLOBALS AFFECTED
*
*************************************************************************/
int mt_get_dram_type (void)
{
    int n;

    n = mt_get_mdl_number ();

    if (n < 0  || n >= num_of_emi_records)
    {
        return 0; /* invalid */
    }

    return (emi_settings[n].type & 0xF);
}

/*************************************************************************
* FUNCTION
*  get_dram_rank_nr()
*
* DESCRIPTION
*   Get DRAM rank numbers
*
* PARAMETERS
*   NONE
*
* RETURNS
*  1 : 1 rank
*  2 : 2 ranks
* -1 : error
*
* GLOBALS AFFECTED
*
*************************************************************************/
int get_dram_rank_nr (void)
{
    int index;
    int emi_gend;

    index = mt_get_mdl_number ();

    if (index < 0 || index >=  num_of_emi_records)
    {
        return -1;
    }

    emi_gend = (emi_settings[index].EMI_GEND_value) & 0x30000;

    if (0x30000 == emi_gend)  // 2 ranks
    {
        return 2;
    }
    else if ((0 == emi_gend) ||(0x20000 == emi_gend))
    {
	  print("[EMI] rank set wrong !");
	  while(1);
         return -1;
    }
    else   // 1 rank
    {
        return 1;
    }
}

void get_dram_rank_size (int dram_rank_size[])
{
    int index,/* bits,*/ rank_nr, i;
    //int emi_cona;


    index = mt_get_mdl_number ();

    if (index < 0 || index >=  num_of_emi_records)
    {
        return;
    }

    rank_nr = get_dram_rank_nr();

    for(i = 0; i < rank_nr; i++)
        dram_rank_size[i] = emi_settings[index].DRAM_RANK_SIZE[i];
}


static void set_emi_mpu_key(void)
{
	*EMI_MPSW = 0x5A5AA5A5;
	*EMI_MPSW = REGION_0_KEY + 0;

	*EMI_MPSW = 0x5A5AA5A5;
	*EMI_MPSW = REGION_1_KEY + 1;

	*EMI_MPSW = 0x5A5AA5A5;
	*EMI_MPSW = REGION_2_KEY + 2;

	*EMI_MPSW = 0x5A5AA5A5;
	*EMI_MPSW = REGION_3_KEY + 3;

	*EMI_MPSW = 0x5A5AA5A5;
	*EMI_MPSW = REGION_4_KEY + 4;

	*EMI_MPSW = 0x5A5AA5A5;
	*EMI_MPSW = REGION_5_KEY + 5;

	*EMI_MPSW = 0x5A5AA5A5;
	*EMI_MPSW = REGION_6_KEY + 6;

	*EMI_MPSW = 0x5A5AA5A5;
	*EMI_MPSW = REGION_7_KEY + 7;

    return;
}

void __EMI_InitializeLPDDR( EMI_SETTINGS *emi_setting )
{
	kal_uint32 i;

       /* reset EMI HW register */
       *EMI_RBSELA = 0x20;
        
	gpt_wait_us_dsb(10);
	*EMI_RBSELA = 0x00;
        
	gpt_wait_us_dsb(10);

	/**
	* Initial EMI MPU
	*/
	
	if (0 != *EMI_MPUA)   //re-boot, should reset permission setting to avoid some module can't use memory
	{	
		set_emi_mpu_key();

              /*reset Region 0 setting*/
		*EMI_MKEY = 	REGION_0_KEY + 0;
		*EMI_MPUA = 0;
		*EMI_MPUI = 0;		
		*EMI_MKEY = 0;			
		
              /*reset Region 1 setting*/
		*EMI_MKEY = 	REGION_1_KEY + 1;
		*EMI_MPUB = 0;
		*EMI_MPUI = 0;		
		*EMI_MKEY = 1;			

              /*reset Region 2 setting*/
		*EMI_MKEY = 	REGION_2_KEY + 2;
		*EMI_MPUC = 0;
		*EMI_MPUJ = 0;		
		*EMI_MKEY = 2;			
		
              /*reset Region 3 setting*/
		*EMI_MKEY = 	REGION_3_KEY + 3;
		*EMI_MPUD = 0;
		*EMI_MPUJ = 0;		
		*EMI_MKEY = 3;			

		/*reset Region 4 setting*/
		*EMI_MKEY = 	REGION_4_KEY + 4;
		*EMI_MPUE = 0;
		*EMI_MPUK = 0;		
		*EMI_MKEY = 4;			
		
              /*reset Region 5 setting*/
		*EMI_MKEY = 	REGION_5_KEY + 5;
		*EMI_MPUF = 0;
		*EMI_MPUK = 0;		
		*EMI_MKEY = 5;			

              /*reset Region 6 setting*/
		*EMI_MKEY = 	REGION_6_KEY + 6;
		*EMI_MPUG = 0;
		*EMI_MPUL = 0;		
		*EMI_MKEY = 6;			
		
              /*reset Region 7 setting*/
		*EMI_MKEY = 	REGION_7_KEY + 7;
		*EMI_MPUH = 0;
		*EMI_MPUL = 0;		
		*EMI_MKEY = 7;			

	}

	
	/**
	* Disable Dummy Read.
	*/
	*EMI_DRCT = 0x0;

        /* Set non-memory preserve mode */
	 *(volatile kal_uint32*)EMI_PPCT = 0xFFDFBF0A;

	/**
	* Set DRAM rank.
	*/
	*EMI_GEND &= ~0x30000;
	*EMI_GEND |= (emi_setting->EMI_GEND_value & 0x30000);

	/* Apply output delay */
	*EMI_ODLA = emi_setting->EMI_ODLA_value;
	*EMI_ODLB = emi_setting->EMI_ODLB_value;
	*EMI_ODLC = emi_setting->EMI_ODLC_value;
	*EMI_ODLD = emi_setting->EMI_ODLD_value;
	*EMI_ODLE = emi_setting->EMI_ODLE_value;
	*EMI_ODLF = emi_setting->EMI_ODLF_value;
	*EMI_ODLG = emi_setting->EMI_ODLG_value;
	*EMI_ODLH = emi_setting->EMI_ODLH_value;
	*EMI_ODLI = emi_setting->EMI_ODLI_value;
	*EMI_ODLJ = emi_setting->EMI_ODLJ_value;
	*EMI_ODLK = emi_setting->EMI_ODLK_value;	 // only for LPDDR2, 2x CMD
	*EMI_ODLL = emi_setting->EMI_ODLL_value;	        // only for LPDDR2, 2x CMD
	*EMI_ODLM = emi_setting->EMI_ODLM_value;	 // only for LPDDR2, 2x CMD
	*EMI_ODLN = emi_setting->EMI_ODLN_value;

	/* Apply IO duty */
	*EMI_DUTA = emi_setting->EMI_DUTA_value;
	*EMI_DUTB = emi_setting->EMI_DUTB_value;
	*EMI_DUTC = emi_setting->EMI_DUTC_value;

	/* Apply RX duty auto tracking */
	*EMI_DUCA = emi_setting->EMI_DUCA_value;
	*EMI_DUCB = emi_setting->EMI_DUCB_value;
	*EMI_DUCE = emi_setting->EMI_DUCE_value;

	/**
       * EMI Driving, it needs to be set before LPDDR being initialized.
	*/
	*EMI_DRVA = emi_setting->EMI_DRVA_value;
	*EMI_DRVB = emi_setting->EMI_DRVB_value;

	*EMI_IOCL = emi_setting->EMI_IOCL_value;  //MT6280 remove "Pad swap function mode"

	/**
       * Set AC Timing Parameters for according memory device.
	*/
	*EMI_CONJ = emi_setting->EMI_CONJ_value;
	*EMI_CONK = emi_setting->EMI_CONK_value;
	*EMI_CONL = emi_setting->EMI_CONL_value;

#if (CFG_FPGA_PLATFORM)
	*EMI_IDLI = 0x01;
#endif
	/**
       * Set hybrid address depends on MCP
	*/
	/*
	if (2 == get_dram_rank_nr())
       {
           if (emi_setting->DRAM_RANK_SIZE[0] ==  ((emi_setting->DRAM_RANK_SIZE[1]) << 1))
           {
           	emi_setting->EMI_CONN_value |= 0x00800000;  //Enable hybird address
           }
           else if (emi_setting->DRAM_RANK_SIZE[0] == emi_setting->DRAM_RANK_SIZE[1])
           {
           	emi_setting->EMI_CONN_value &= (~0x00800000);  //Disable hybird address
           }
           else
           {
           	print("[EMI] Abnormal DRAM rank size setting");
           	while(1);
           }
	}
	*/
       *EMI_CONN = emi_setting->EMI_CONN_value;

#if (CFG_FPGA_PLATFORM)
       *EMI_GENA = (*EMI_GENA & ~0xC000) ;  //RDDELSEL_OFFSET = 0 for memory preserve use
#else
       *EMI_GENA = (*EMI_GENA & ~0xC000) | 0x8000;  //RDDELSEL_OFFSET = 2 for memory preserve use
#endif

     if (emi_setting->EMI_Freq == 200)
     {
       *EMI_GENA &= (~0x1800);    //1x clock <= 200Mhz, disable 2T for high speed arbitration and CMD pipeline
	   *EMI_DQSI &= (~0x0000FF00);  
            *EMI_DQSI |= 0x00000800;  //DQSI delay tap max. 0x0F
     }
     else if (emi_setting->EMI_Freq == 266)
     {
        *EMI_GENA |= 0x1800;   //1x clock > 200Mhz, enable 2T for high speed arbitration and CMD pipeline
          *EMI_DQSI &= (~0x0000FF00);  
             *EMI_DQSI |= 0x00000800; //0x00001300;  //DQSI delay tap max. 0x0F
     }
     else if (emi_setting->EMI_Freq == 333)
     {
        *EMI_GENA |= 0x1800;   //1x clock > 200Mhz, enable 2T for high speed arbitration and CMD pipeline
          *EMI_DQSI &= (~0x0000FF00);  
             *EMI_DQSI |= 0x00000600;  //DQSI delay tap max. 0x0A
     }

    if ((emi_setting->type & 0xF) == 1)
    {
        init_lpddr1(emi_setting);
    }
    else if ((emi_setting->type & 0xF) == 2)
    {
        init_lpddr2(emi_setting);
    }
    else if ((emi_setting->type & 0xF) == 3)
    {
             init_pcdram3(emi_setting);  //PCDDR3
    }
    else if ((emi_setting->type & 0xF) == 4)
	  {
	      init_lpddr3(emi_setting);	 //LPDDR3
	  }
    else
    {
        print("[EMI] Abnormal DRAM type");
	 while(1);
    }

   /**
   * Enable auto-refresh, fixed-clock (for auto-refresh), and pdn.
   */
   *EMI_CONN |=  0x00000017;

    
    gpt_wait_us_dsb(5);

#ifdef DQSI_MAX_DELAY_TAP_BY_DLL 
	 *EMI_CONN |= 0x100;    //Enable 1/5T DLL
	 
	 gpt_wait_us_dsb(10);
	 while((*EMI_DLLV & 0x80)== 0);  
	 i = *EMI_DLLV & 0x1F;    //Get 1/5T DLL value 
				
	 *EMI_DQSI &= (~0x0000FF00);	
	 *EMI_DQSI |= (i << 8);  //Set 1/5T tap as DQSI 1/4T max. delay tap 
#endif
	
}


void __EMI_DataAutoTrackingRegRead( EMI_DATA_TRAIN_REG_t* pREG, int rank_idx)
{
    if( pREG != 0 )
    {
        pREG->EMI_DLLV_regval = *EMI_DLLV;
        pREG->EMI_DQSU_regval = *EMI_DQSU;
        pREG->EMI_CONN_regval = *EMI_CONN;
	 if (rank_idx == 0)
	 {
            pREG->EMI_DQSA_regval = *EMI_DQSA;
            pREG->EMI_DQSB_regval = *EMI_DQSB;
	     pREG->EMI_IDLI_regval = *EMI_IDLI;
	     pREG->EMI_CALA_regval = *EMI_CALA;
	     pREG->EMI_CALE_regval = *EMI_CALE;
	     pREG->EMI_CALI_regval = *EMI_CALI;
	 }
	 else
	 {
	      pREG->EMI_DQSC_regval = *EMI_DQSC;
             pREG->EMI_DQSD_regval = *EMI_DQSD;
	      pREG->EMI_IDLJ_regval = *EMI_IDLJ;
             pREG->EMI_CALB_regval = *EMI_CALB;
             pREG->EMI_CALF_regval = *EMI_CALF;
             pREG->EMI_CALJ_regval = *EMI_CALJ;
	 }
	 pREG->EMI_DQSI_regval = *EMI_DQSI;
        pREG->EMI_IDLA_regval = *EMI_IDLA;
        pREG->EMI_IDLB_regval = *EMI_IDLB;
        pREG->EMI_IDLC_regval = *EMI_IDLC;
        pREG->EMI_IDLD_regval = *EMI_IDLD;
        pREG->EMI_IDLE_regval = *EMI_IDLE;
        pREG->EMI_IDLF_regval = *EMI_IDLF;
        pREG->EMI_IDLG_regval = *EMI_IDLG;
        pREG->EMI_IDLH_regval = *EMI_IDLH;
        pREG->EMI_CALP_regval = *EMI_CALP;
    }
    else
    {
        ASSERT(0);
    }
}


void __EmiDataTrainRegWrite( EMI_DATA_TRAIN_REG_t* pREG, int rank_idx )
{
    if( pREG != 0 )
    {
        *EMI_CONN = pREG->EMI_CONN_regval;
	if (rank_idx == 0)
	{
           *EMI_DQSA = pREG->EMI_DQSA_regval;
           *EMI_DQSB = pREG->EMI_DQSB_regval;
	    *EMI_IDLI = pREG->EMI_IDLI_regval;
           *EMI_CALA = pREG->EMI_CALA_regval;
           *EMI_CALE = pREG->EMI_CALE_regval;
           *EMI_CALI = pREG->EMI_CALI_regval;
	}
	else
	{
           *EMI_DQSC = pREG->EMI_DQSC_regval;
           *EMI_DQSD = pREG->EMI_DQSD_regval;
	    *EMI_IDLJ = pREG->EMI_IDLJ_regval;
           *EMI_CALB = pREG->EMI_CALB_regval;
           *EMI_CALF = pREG->EMI_CALF_regval;
           *EMI_CALJ = pREG->EMI_CALJ_regval;
	}

        *EMI_DQSI = pREG->EMI_DQSI_regval;
        *EMI_IDLA = pREG->EMI_IDLA_regval;
        *EMI_IDLB = pREG->EMI_IDLB_regval;
        *EMI_IDLC = pREG->EMI_IDLC_regval;
        *EMI_IDLD = pREG->EMI_IDLD_regval;
        *EMI_IDLE = pREG->EMI_IDLE_regval;
        *EMI_IDLF = pREG->EMI_IDLF_regval;
        *EMI_IDLG = pREG->EMI_IDLG_regval;
        *EMI_IDLH = pREG->EMI_IDLH_regval;
        *EMI_CALP = pREG->EMI_CALP_regval;
    }

}

kal_int32 __EMI_DataTrackingMbistTestCore(kal_int32 algo_sel, kal_int32 addr_scramble_sel, kal_int32 data_scramble_sel)
{
    //volatile kal_int32 delay;

    /**
       * MBIST reset.
       */
    *EMI_MBISTA = 0x0;
    
    gpt_wait_us_dsb(10);

    /**
       * MBIST data-pattern setting.
       */
    *EMI_MBISTB = 0xFFFF0000 | ((0x0000A55A) >> (algo_sel + addr_scramble_sel + data_scramble_sel));

    /**
       * MBIST starting address.
       */
    *EMI_MBISTC = 0x410000>>10;

    /**
       * MBIST ending address.
       */
    *EMI_MBISTD = 0x410000>>10;

    /**
       * ClearBIST address/data scramble and algorithm.
       */
    *EMI_MBISTA &= 0xFFFF000F;
     
     gpt_wait_us_dsb(1);

    *EMI_MBISTA |= (0x00220000 | algo_sel<<4 | addr_scramble_sel<<12 | data_scramble_sel<<8 | 1);
    
    gpt_wait_us_dsb(50);

    /**
       * Start MBIST test.
       */
    *EMI_MBISTA = *EMI_MBISTA | 2;
     
     gpt_wait_us_dsb(50);

    /**
       * Polling the MBIST test finish status.
       */
    while(!(*EMI_MBISTE&0x0002));

    /**
       * Check the MBIST result.
       */
    if(*EMI_MBISTE&0x0001)
    {
        /**
           * addr[15:00]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0x0 << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_addr = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
#endif

        /**
           * addr[31:16]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0x1 << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
         emi_data_training_err_info.err_addr |= ( *EMI_MBISTE & 0xFFFF0000 );
#endif

        /**
           * wdataL[15:00]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0x4 << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_wdataL= ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
#endif

        /**
           * wdataL[31:16]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0x5 << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_wdataL |= ( *EMI_MBISTE & 0xFFFF0000 );
#endif

        /**
           * wdataH[47:32]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0x6 << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_wdataH = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
#endif

        /**
           * wdataH[63:48]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0x7 << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_wdataH |= ( *EMI_MBISTE & 0xFFFF0000 );
#endif

        /**
           * rdataL[15:00]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0x8 << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_rdataL = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
#endif

        /**
           * rdataL[31:16]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0x9 << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_rdataL |= ( *EMI_MBISTE & 0xFFFF0000 );
#endif

        /**
           * rdataH[47:32]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0xA << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_rdataH = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
#endif

        /**
           * rdataH[63:48]
           */
        *EMI_MBISTA = (*EMI_MBISTA & ~((kal_uint32)0xF << 28)) | ((kal_uint32)0xB << 28) ;
#if defined(__LOG_DATA_TRAIN_ERROR_INFO)
        emi_data_training_err_info.err_rdataH |= ( *EMI_MBISTE & 0xFFFF0000 );
#endif

        /**
           * MBIST reset.
           */
        *EMI_MBISTA = 0x0;
	  
	  gpt_wait_us_dsb(10);

        return -1;
    }
    else
    {
        /**
           * MBIST reset.
           */
        *EMI_MBISTA = 0x0;
	   
	  gpt_wait_us_dsb(10);

        return 0;
    }
}

kal_int32 __EMI_DataAutoTrackingMbistTest(void)
{
    kal_int32 algo_sel, data_scramble_sel;
    // kal_int32 addr_scramble_sel;
    kal_uint32 EMI_DRCT_bakval;

    /**
       * Backup dummy read control.
       */
    EMI_DRCT_bakval = *EMI_DRCT;

    /**
       * Disable dummy read before testing MBIST (must).
       */
    *EMI_DRCT &= ~(0x0D);

    for(algo_sel=0; algo_sel<3; algo_sel++)
    {
        for(data_scramble_sel=0; data_scramble_sel<7; data_scramble_sel++)
        {
            if( __EMI_DataTrackingMbistTestCore( algo_sel, 0x0, data_scramble_sel ) != 0 )
            {
                /**
                    * Restore dummy read control.
                    */
                *EMI_DRCT = EMI_DRCT_bakval;
                return -1;
            }
        }
    }

    /**
        * Restore dummy read control.
        */
   *EMI_DRCT = EMI_DRCT_bakval;

    return 0;
}


int __EMI_DataAutoTrackingTraining( EMI_DATA_TRAIN_REG_t* pResult, int rank_idx )
{
#define ___EMIDATATRAIN_MBIST_DATATRAIN__
#define DEBUG_MODE 0
#define DQ_IN_DLY_TAPS 0x0E
#define DATA_SETUP_TAPS 0x1F
#define DQS_IN_DLY_TAPS  0x1F
#define DQSI_TUNING_BOUND 0x1FF
#define DATA_TUNING_STEP 2
#define DQSI_TUNING_STEP (1 << 5)   // 1/4 T
#define WINDOW_SIZE_THRESHOLD  6
#define DQSI_TUNING_WINDOW_SIZE_THRESHOLD 6

    //kal_int32 delay, value;
    kal_int32 value;
    kal_int32 bytex_dly_mod, bytex_setup_mod, dqy_in_del, dqsix_dlysel;
    kal_int32 bytex_dly_mod_start = 0, bytex_setup_mod_start = 0, dqy_in_del_start = 0; //dqsix_dlysel_start = 0;
    kal_int32 prev_emi_dqsa = 0;
    kal_int32 prev_emi_idl = 0, prev_emi_cala = 0, prev_emi_cale = 0, prev_emi_cali = 0;
    kal_int32 prev_dwnd_size = 0, dwnd_size = 0;// lbound, rbound;
    kal_int32 lbound_finding;

    kal_uint32 DQSI_center = 0x0, DQSI_start = 0xffffffff, DQSI_end = DQSI_TUNING_BOUND; //Use to record the DQSI start and end value
#ifdef IMPROVE_DQS_STROBE
    kal_int32 prev_bytex_dly_mod_start = 0, prev_bytex_setup_mod_start = 0, prev_dqy_in_del_start = 0;
    kal_int32 temp = 0;
#endif
    kal_int32 test_result;

    int b_wnd_found = 0;
    EMI_DATA_TRAIN_REG_t REG_BAK;

    /*------------------------------------------------------------------------------
        Backup modified register at entry
      ------------------------------------------------------------------------------*/
    __EMI_DataAutoTrackingRegRead( &REG_BAK, rank_idx);

    /*------------------------------------------------------------------------------
        Disable
            1. "1/5T DLL"
            2. "Data auto tracking" & "Setup/Hold max value"
            3. "DQSI auto-tracking"
            before data training
      ------------------------------------------------------------------------------*/
     *EMI_CONN &= ~0x00000100;
     *EMI_CALP &= ~0xFFFF0003;
     *EMI_DQSI &= ~0x000000FF;

     gpt_wait_us_dsb(1);

    for(dqsix_dlysel=0x0; dqsix_dlysel<=DQSI_TUNING_BOUND; dqsix_dlysel+=DQSI_TUNING_STEP/* 8 */)
    {
       if (rank_idx == 0)
       {
	    *EMI_DQSA = *EMI_DQSB = dqsix_dlysel<<16 |dqsix_dlysel;
       }
	else
	{
	    *EMI_DQSC = *EMI_DQSD = dqsix_dlysel<<16 |dqsix_dlysel;
	}
	lbound_finding = 1;

        /*byte_delay = 0*/
        bytex_dly_mod = 0;
       if (rank_idx == 0)
       {
              *EMI_IDLI = 0;
       }
	else
	{
		*EMI_IDLJ = 0;
	}
        /*byte_setup = 0*/
        bytex_setup_mod = 0;
        *EMI_CALE = 0;

        /*Reset CALA/CALI*/
        *EMI_CALA = *EMI_CALB = 0;
        *EMI_CALI = *EMI_CALJ = 0;

	 
	 gpt_wait_us_dsb(1);

        /*Iterate dq_in delay 0x1F ~ 0*/
        for(dqy_in_del=DQ_IN_DLY_TAPS; dqy_in_del>=0; dqy_in_del-=DATA_TUNING_STEP)
        {
            *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH =
            dqy_in_del<<24 | dqy_in_del<<16 | dqy_in_del<<8 | dqy_in_del;

            // Clear DDRFIFO
            *EMI_CALP |= 0x00000100;
	     
	     gpt_wait_us_dsb(1);
            *EMI_CALP &= 0xFFFFFEFF;

            /* do DQS calibration */
#if defined( ___EMIDATATRAIN_MBIST_DATATRAIN__ )
            test_result = __EMI_DataAutoTrackingMbistTest();
#else
                store_8word(&datatraing_cmp_pattern, 0x12345678);
                test_result = load_8word(&datatraing_cmp_pattern, 0x12345678);
#endif

            /* R/W ok & during boundary finding ==> 0->1 , Record the start boundary*/
            if(lbound_finding==1 && test_result == 0)
            {

                #if DEBUG_MODE
                dbg_print("Find L Bound (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                #endif
                dqy_in_del_start = dqy_in_del;
                bytex_dly_mod_start = bytex_dly_mod;
                bytex_setup_mod_start = bytex_setup_mod;

                lbound_finding = 0;

            }
            /* R/W fail & not during boundary finding ==> 1->0 */
            else if(lbound_finding==0 && test_result != 0 )
            {
                /* handle 0001011111111111111......*/
                dwnd_size = (dqy_in_del_start-dqy_in_del)+(bytex_setup_mod-bytex_setup_mod_start)+(bytex_dly_mod-bytex_dly_mod_start);
                if (dwnd_size <= DATA_TUNING_STEP)  //ignore this window, continue find window
                {
                    lbound_finding = 1;
                    #if DEBUG_MODE
                    dbg_print("Ignore this window (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                    #endif
                }
                else
                {
                    #if DEBUG_MODE
                    dbg_print("Find R Bound (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                    #endif

                    goto window_found;
                }
            }
        }

        dqy_in_del = 0; /*This value should be already be 0*/
        *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = 0;

	 gpt_wait_us_dsb(1);

        for(bytex_setup_mod=0; bytex_setup_mod<=DATA_SETUP_TAPS; bytex_setup_mod+=DATA_TUNING_STEP)
        {

	     /*Under disable data tracking, data setup of rank 1 also use EMI_CALE */
            *EMI_CALE = bytex_setup_mod<<24 | bytex_setup_mod<<16 | bytex_setup_mod<<8 | bytex_setup_mod;

            // Clear DDRFIFO
            *EMI_CALP |= 0x00000100;
	     
	     gpt_wait_us_dsb(1);
            *EMI_CALP &= 0xFFFFFEFF;

            /* do DQS calibration */
            #if defined( ___EMIDATATRAIN_MBIST_DATATRAIN__ )
            test_result = __EMI_DataAutoTrackingMbistTest();
            #else
                store_8word(&datatraing_cmp_pattern, 0x12345678);
                test_result = load_8word(&datatraing_cmp_pattern, 0x12345678);
            #endif

            if(lbound_finding==1 && test_result == 0 )
            {
                #if DEBUG_MODE
                dbg_print("Find L Bound (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                #endif

                bytex_dly_mod_start = bytex_dly_mod;
                bytex_setup_mod_start = bytex_setup_mod;
                dqy_in_del_start = dqy_in_del;

                lbound_finding = 0;

            }
            else if(lbound_finding==0 && test_result != 0 )
            {
                /* handle 0001011111111111111......*/
                dwnd_size = (dqy_in_del_start-dqy_in_del)+(bytex_setup_mod-bytex_setup_mod_start)+(bytex_dly_mod-bytex_dly_mod_start);
                if (dwnd_size <= DATA_TUNING_STEP)  //ignore this window, continue find window
                {
                    lbound_finding = 1;
                   #if DEBUG_MODE
                    dbg_print("Ignore this window (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                   #endif
                }
                else
                {
                    #if DEBUG_MODE
                    dbg_print("Find R Bound (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                    #endif

                goto window_found;
            }
        }
        }
        bytex_setup_mod = DATA_SETUP_TAPS; /*This value should be already be 0x1F*/
        /*Under disable data tracking, data setup of rank 1 also use EMI_CALE */
        *EMI_CALE  = bytex_setup_mod<<24 | bytex_setup_mod<<16 | bytex_setup_mod<<8 | bytex_setup_mod;

 	 gpt_wait_us_dsb(1);

        for(bytex_dly_mod=0; bytex_dly_mod<=DQS_IN_DLY_TAPS; bytex_dly_mod+=DATA_TUNING_STEP)
        {
		if (rank_idx == 0)
		{
                   *EMI_IDLI = bytex_dly_mod<<24 | bytex_dly_mod<<16 | bytex_dly_mod<<8 | bytex_dly_mod;
		}
		else
		{
                   *EMI_IDLJ = bytex_dly_mod<<24 | bytex_dly_mod<<16 | bytex_dly_mod<<8 | bytex_dly_mod;
		}

            // Clear DDRFIFO
            *EMI_CALP |= 0x00000100;
	     
	     gpt_wait_us_dsb(1);
            *EMI_CALP &= 0xFFFFFEFF;

            /* do DQS calibration */
            #if defined( ___EMIDATATRAIN_MBIST_DATATRAIN__ )
                test_result = __EMI_DataAutoTrackingMbistTest();
            #else
                store_8word(&datatraing_cmp_pattern, 0x12345678);
                test_result = load_8word(&datatraing_cmp_pattern, 0x12345678);
            #endif


            if(lbound_finding==1 && test_result == 0 )
            {

                #if DEBUG_MODE
                dbg_print("Find L Bound (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                #endif

                bytex_dly_mod_start = bytex_dly_mod;
                bytex_setup_mod_start = bytex_setup_mod;
                dqy_in_del_start = dqy_in_del;

                lbound_finding = 0;

            }
            else if(lbound_finding==0 && test_result != 0 )
            {
                /* handle 0001011111111111111......*/
                dwnd_size = (dqy_in_del_start-dqy_in_del)+(bytex_setup_mod-bytex_setup_mod_start)+(bytex_dly_mod-bytex_dly_mod_start);
                if (dwnd_size <= DATA_TUNING_STEP)  //ignore this window, continue find window
                {
                    lbound_finding = 1;
                    #if DEBUG_MODE
                    dbg_print("Ignore this window (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                    #endif
            }
                else
                {
                    #if DEBUG_MODE
                    dbg_print("Find R Bound (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
                    #endif

                goto window_found;
            }
            }
      }


        /*Find a windows that only have one-end boundary,ex. 000111111...*/
        if(lbound_finding == 0)
        {
            #if DEBUG_MODE
            dbg_print("Not Find R Bound (0x%x, 0x%x, 0x%x)\n\r", dqy_in_del, bytex_dly_mod, bytex_setup_mod);
            #endif

            goto window_found;
        }


        /*window is not found, but previous windows found, it's also a shrink case, goto windows_found*/
        /*In this case, found a window size = 0 , ex. 00000000... */
        if( ( lbound_finding == 1 ) && ( b_wnd_found == 1 ) )
        {
            if (DQSI_end == DQSI_TUNING_BOUND)  //Never find DQSI_end
            {
                DQSI_end = dqsix_dlysel-DQSI_TUNING_STEP;//IvanTseng: record the last DQSI value
            }


            if (rank_idx == 0)
	     {
                *EMI_DQSA = *EMI_DQSB = prev_emi_dqsa;
		  *EMI_CALA =  prev_emi_cala;
		  *EMI_CALE = prev_emi_cale;
		  *EMI_CALI =  prev_emi_cali;
            }
            else
            {
                *EMI_DQSC = *EMI_DQSD = prev_emi_dqsa;
		  *EMI_CALB = prev_emi_cala;
		  *EMI_CALF = prev_emi_cale;
		  *EMI_CALJ = prev_emi_cali;
	     }
            *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = prev_emi_idl;

	      gpt_wait_us_dsb(1);

            #if DEBUG_MODE
                dbg_print("(N/A) Window size = %d, DQSI=0x%x\n\r", dwnd_size, dqsix_dlysel);
                continue;
            #else
                break;
            #endif
       }

        /*window is not found, use next mask setting*/
        continue;

    window_found:

        if(bytex_dly_mod>DQS_IN_DLY_TAPS)
        {
            // This is an unexpected case
            bytex_dly_mod = DQS_IN_DLY_TAPS;
        }

        if(bytex_setup_mod>DATA_SETUP_TAPS)
        {
            // This is an unexpected case
            bytex_setup_mod = DATA_SETUP_TAPS;
        }

        if(dqy_in_del<0)
        {
            // This is an unexpected case
            dqy_in_del = 0;
        }

        if(dqsix_dlysel>DQSI_TUNING_BOUND)
        {
            // This is an unexpected case
            dqsix_dlysel = DQSI_TUNING_BOUND;
        }

        dwnd_size = (dqy_in_del_start-dqy_in_del)+(bytex_setup_mod-bytex_setup_mod_start)+(bytex_dly_mod-bytex_dly_mod_start);

        /*If windows <= 10, ignore this windows found,maybe it's a noise because MBIST is not reliable */
        if( dwnd_size <= DQSI_TUNING_WINDOW_SIZE_THRESHOLD )
        {
            #if DEBUG_MODE
                dbg_print("(SMALL) Window size = %d, DQSI=0x%x\n\r", dwnd_size, dqsix_dlysel);
            #endif
            continue;
        }
        else
        {
            b_wnd_found = 1; //it means the DQSI is found

            #if DEBUG_MODE
                dbg_print("Window size = %d, DQSI=0x%x\n\r", dwnd_size, dqsix_dlysel);
            #endif
        }


        if (DQSI_start==0xffffffff) DQSI_start = dqsix_dlysel; //Record the 1st DQSI value

	print("[EMI] [Train] DQSI=0x%x,Window=%d \n", dqsix_dlysel, dwnd_size);
	print("[EMI] [Train] dq_in_dly_strart=0x%x, byte_setup_start=0x%x, byte_dly_start=0x%x \n", dqy_in_del_start, bytex_setup_mod_start, bytex_dly_mod_start);

        /* Stop tuning when the prev_dwnd_size is greater than current window size */
        if(prev_dwnd_size && (prev_dwnd_size > (dwnd_size+WINDOW_SIZE_THRESHOLD)))
        {
             DQSI_end = dqsix_dlysel-DQSI_TUNING_STEP;//IvanTseng: record the last DQSI value

      	     if (rank_idx == 0)
      	     {
      		   *EMI_DQSA = *EMI_DQSB = prev_emi_dqsa;
      	          *EMI_CALA =  prev_emi_cala;
      	          *EMI_CALE = prev_emi_cale;
      	          *EMI_CALI =  prev_emi_cali;
      	     }
      	     else
      	     {
      		   *EMI_DQSC = *EMI_DQSD = prev_emi_dqsa;
      	          *EMI_CALB = prev_emi_cala;
      	          *EMI_CALF = prev_emi_cale;
      	          *EMI_CALJ = prev_emi_cali;
      	      }
            *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = prev_emi_idl;

	     gpt_wait_us_dsb(1);

            /*------------------------------------------------------------------------------
                Once find a windows less or equal previous one, use:
                    1. Previous delay setting
                    2. current mask setting ( in case the previous one is in mask boundary ) and finish data training.
              ------------------------------------------------------------------------------*/
            #if DEBUG_MODE
                dbg_print("(ESCAPE) Window size = %d, DQSI=0x%x\n\r", dwnd_size, dqsix_dlysel);
                continue;
            #else
                break;
            #endif
        }

        prev_dwnd_size = dwnd_size;

#ifdef IMPROVE_DQS_STROBE
      prev_dqy_in_del_start = dqy_in_del_start;
      prev_bytex_setup_mod_start = bytex_setup_mod_start;
      prev_bytex_dly_mod_start = bytex_dly_mod_start;
#endif 

        /*Use only for a "valid windows size" shrink to "zero windows size" immmediately.*/
	if (rank_idx == 0)
	{
              prev_emi_dqsa = *EMI_DQSA;
	}
	else
	{
		prev_emi_dqsa = *EMI_DQSC;
	}

        /*Align "DQS riging" & "data out"*/
        //value = (dqy_in_del_start > (dwnd_size/2)) ? (dqy_in_del_start-dwnd_size/2) : 0;
        //value<<24 | value<<16 | value<<8 | value;
        *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = prev_emi_idl =
        dqy_in_del_start<<24 | dqy_in_del_start <<16 | dqy_in_del_start <<8 | dqy_in_del_start;

        /*Byte Data Delay*/
        value = (bytex_setup_mod_start+(dwnd_size/2))> DATA_SETUP_TAPS ? (bytex_setup_mod_start+(dwnd_size/2))-DATA_SETUP_TAPS:0;
        //value = (bytex_setup_mod_start+(dwnd_size/2))<31? (bytex_setup_mod_start+(dwnd_size/2)):31; //CLS
        /*rank 2,3 are useless, need to train??*/
        prev_emi_cala = value<<24 | value<<16 | value<<8 | value;

        /*Byte Data Setup*/
        value = (bytex_setup_mod_start+(dwnd_size/2))<DATA_SETUP_TAPS ? (bytex_setup_mod_start+(dwnd_size/2)):DATA_SETUP_TAPS;
        //value = (bytex_setup_mod_start+(dwnd_size/2))>31? (bytex_setup_mod_start+(dwnd_size/2))-31:0; //CLS
        prev_emi_cale = (value<<24 | value<<16 | value<<8 | value);

        /*byte Data Hold*/
        value = ( dwnd_size/2 > DATA_SETUP_TAPS ) ? DATA_SETUP_TAPS : dwnd_size/2;
        prev_emi_cali = value<<24 | value<<16 | value<<8 | value;

	if (rank_idx == 0)
	{
		 *EMI_CALA = prev_emi_cala;
		 *EMI_CALE =  prev_emi_cale;
		 *EMI_CALI = prev_emi_cali;
	}
	else
	{
		 *EMI_CALB = prev_emi_cala;
		 *EMI_CALF = prev_emi_cale;
		 *EMI_CALJ = prev_emi_cali;
	}


	gpt_wait_us_dsb(1);

        /*Go next mask setting*/
    }

    /* IvanTseng : Get the proper DQSI value here */
    DQSI_center = (DQSI_start + DQSI_end)/2;

    //#if DEBUG_MODE
        print("[EMI] DQSI = 0x%x, (0x%x, 0x%x)\n", DQSI_center, DQSI_start, DQSI_end);
	 print("[EMI] Window = %d \n", dwnd_size);
    //#endif

    if ((DQSI_center % DQSI_TUNING_STEP) != 0)   //if DQSI_center is over Max DQSI tap, I will cause DQSI auto-tune value over Max DQSI tap
    {
          DQSI_center = ((DQSI_center / DQSI_TUNING_STEP) * DQSI_TUNING_STEP) + (((*EMI_DQSI & 0xFF00) >> 8) >> 1);
    }
	
    if (rank_idx == 0)
    {
	  *EMI_DQSA = *EMI_DQSB = DQSI_center<<16 | DQSI_center;
    }
    else
    {
	  *EMI_DQSC = *EMI_DQSD = DQSI_center<<16 | DQSI_center;
    }

    gpt_wait_us_dsb(1);

    /*------------------------------------------------------------------------------
        Set up MAX "Data Setup" & " Data Hold"
      ------------------------------------------------------------------------------*/
     //If we do not enable DATA_CAL_EN, we must clear IDLI because IDLI delay will take effetc when DATA_CAL_EN is disabled.
    if (rank_idx == 0)
    {
    #ifdef IMPROVE_DQS_STROBE  	
	  temp = (*EMI_DQSI & (0x00001F00)) >> 8;	  // 1/5T DLL tap	 
	  //value = prev_bytex_setup_mod_start + ((prev_dwnd_size >> 1) - prev_dqy_in_del_start)  - temp ;
	  value = prev_bytex_setup_mod_start + (prev_dwnd_size >> 1) - temp ;
	  if (value >= DQS_IN_DLY_TAPS)
	  {	  
	       print("[EMI] EMI_IDLI=0x%x > 0x1F !\n", value);
	       value = DQS_IN_DLY_TAPS;
	  }	
	  *EMI_IDLI = value<<24 | value <<16 | value <<8 | value;
     #else
           *EMI_IDLI = 0;
      #endif
    }
    else  // rank 1
    {
     #ifdef IMPROVE_DQS_STROBE   	 
	  temp = (*EMI_DQSI & (0x00001F00)) >> 8;	  // 1/5T DLL tap	 
	  //value = prev_bytex_setup_mod_start + ((prev_dwnd_size >> 1) - prev_dqy_in_del_start)  - temp ;
	  value = prev_bytex_setup_mod_start + (prev_dwnd_size >> 1)  - temp ;
	  if (value >= DQS_IN_DLY_TAPS)
    {
	       print("[EMI] EMI_IDLJ=0x%x > 0x1F !\n", value);
	       value = DQS_IN_DLY_TAPS;
	  }	
	  *EMI_IDLJ= value<<24 | value <<16 | value <<8 | value;
     #endif    
	 
	   if ((*EMI_IDLA & 0xFF) > ((pResult->EMI_IDLA_regval) & 0xFF))  //rank 1 DQ_IN_DLY > rank 0 DQ_IN_DLY, use rank 1 DQ_IN_DLY
	   {
		   value = (*EMI_IDLA & 0xFF) - ((pResult->EMI_IDLA_regval) & 0xFF);

	   #ifdef IMPROVE_DQS_STROBE   		   
		   value = value + (pResult->EMI_IDLI_regval & DQS_IN_DLY_TAPS);
		   if (value >= DQS_IN_DLY_TAPS)
	          {	          
		      print("[EMI] EMI_IDLI=0x%x > 0x1F !\n", value);
	              value = DQS_IN_DLY_TAPS;
	          }		 
		   pResult->EMI_IDLI_regval = value<<24 | value <<16 | value <<8 | value;
	  #else
		   pResult->EMI_IDLI_regval  = value<<24 | value <<16 | value <<8 | value;
		   *EMI_IDLJ = 0;
	  #endif
	   }
	   else if ((*EMI_IDLA & 0xFF) < ((pResult->EMI_IDLA_regval) & 0xFF))  //rank 1 DQ_IN_DLY < rank 0 DQ_IN_DLY, use rank 0 DQ_IN_DLY
	   {
       	   value = ((pResult->EMI_IDLA_regval) & 0xFF) - (*EMI_IDLA & 0xFF);
		   
           #ifdef IMPROVE_DQS_STROBE   		   
		   value = value + (*EMI_IDLJ & DQS_IN_DLY_TAPS);
		   if (value >= DQS_IN_DLY_TAPS)
	          {	          
		      print("[EMI] EMI_IDLJ=0x%x > 0x1F !\n", value);
	              value = DQS_IN_DLY_TAPS;
	          }	
           #endif  // ifdef IMPROVE_DQS_STROBE   
		   
       	   *EMI_IDLJ = value<<24 | value <<16 | value <<8 | value;
		   *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = pResult->EMI_IDLA_regval;
	   }
	   else   //rank 1 DQ_IN_DLY =  rank 0 DQ_IN_DLY
	   {
            #ifndef IMPROVE_DQS_STROBE			
		   *EMI_IDLJ = 0;
            #endif
	   }
    }

    /* Enable auto data tracking*/
    value = ((prev_dwnd_size/2) > DATA_SETUP_TAPS) ? DATA_SETUP_TAPS : (prev_dwnd_size/2);
    *EMI_CALP &= 0x0000FFFF;
    
    gpt_wait_us_dsb(1);
    *EMI_CALP |= ( 1 << 31 ) | ( value << 24) | ( 1 << 23 ) | ( value << 16 );

    /* Make sure the CAL_EN is ENABLED */
    //*EMI_CONN |= (0x00000100); //Enable CAL_EN, 1/5T DLL

     /* After enabling CAL_EN, wait for an auto refresh interval around 7.8 us is required.
        The calibration value from DLL circuit can be applied on delay line. Then EMI can work normally.
       */
     //for(delay=0;delay<0xfff;delay++);

     /*------------------------------------------------------------------------------
         Return Training Result and Restore Register
       ------------------------------------------------------------------------------*/
     __EMI_DataAutoTrackingRegRead( pResult, rank_idx );      /*Return Training Result*/
     __EmiDataTrainRegWrite( &REG_BAK, rank_idx );    /*Restore Register*/

     //__EMI_DataAutoTrackingRegRead( &REG_BAK ); //Test


    return  b_wnd_found;

}


int __EMI_EnableDataAutoTracking( EMI_DATA_TRAIN_REG_t* DATA_TRAIN_RESULT_REG, EMI_SETTINGS *emi_setting)
{
    int ret = 1;
#if defined(SW_WORKAROUND_FOR_DUALRANKS)
    kal_uint32 	Rank0_DQS0, Rank0_DQS1, Rank0_DQS2, Rank0_DQS3;
    kal_uint32 	Rank1_DQS0, Rank1_DQS1, Rank1_DQS2, Rank1_DQS3;
#endif


    if( __EMI_DataAutoTrackingTraining(DATA_TRAIN_RESULT_REG, 0) == 0 )
    {
        /**
           * Data training fail.
           */
	print("[EMI] R0 cal fail\n\r");
       while(1);
    }
    else
    {
	print("[EMI] R0 cal pass\n\r");
    }

    if (2 == get_dram_rank_nr())
    {
        *EMI_GENA |= 0x01;  //swap CS0/CS1

        if ( __EMI_DataAutoTrackingTraining(DATA_TRAIN_RESULT_REG, 1) == 0 )
        {
            /**
               * Data training fail.
               */
    	    print("[EMI] R1 cal fail\n\r");
           while(1);
        }
        else
        {
    	     print("[EMI] R1 cal pass\n\r");
        }

	*EMI_GENA &= (~0x01);  //un-swap CS0/CS1
    }
    /**
       * Mask auto tracking initial value.
       */
    *EMI_DQSA = DATA_TRAIN_RESULT_REG->EMI_DQSA_regval;
    *EMI_DQSB = DATA_TRAIN_RESULT_REG->EMI_DQSB_regval;
    if (2 == get_dram_rank_nr())
    {	
        *EMI_DQSC = DATA_TRAIN_RESULT_REG->EMI_DQSC_regval;
        *EMI_DQSD = DATA_TRAIN_RESULT_REG->EMI_DQSD_regval;
    }
#if defined(__EMI_DATA_AUTO_TRACKING_ENABLE)

    /**
        * Data Auto Tracking init value.
        */
    *EMI_CALA = DATA_TRAIN_RESULT_REG->EMI_CALA_regval;
    *EMI_CALB = DATA_TRAIN_RESULT_REG->EMI_CALB_regval;
    *EMI_CALE = DATA_TRAIN_RESULT_REG->EMI_CALE_regval;
    *EMI_CALF = DATA_TRAIN_RESULT_REG->EMI_CALF_regval;
    *EMI_CALI = DATA_TRAIN_RESULT_REG->EMI_CALI_regval;
    *EMI_CALJ = DATA_TRAIN_RESULT_REG->EMI_CALJ_regval;
    *EMI_CALP = DATA_TRAIN_RESULT_REG->EMI_CALP_regval;

    /**
        * Enable auto data tracking.
        */
    *EMI_CALP |= 0x3;

#else

    /**
       * DQ-in delay.
       */

    *EMI_IDLI = DATA_TRAIN_RESULT_REG->EMI_IDLI_regval;
    if (2 == get_dram_rank_nr())
    {
    *EMI_IDLJ = DATA_TRAIN_RESULT_REG->EMI_IDLJ_regval;
    }
    *EMI_IDLA = DATA_TRAIN_RESULT_REG->EMI_IDLA_regval;
    *EMI_IDLB = DATA_TRAIN_RESULT_REG->EMI_IDLB_regval;
    *EMI_IDLC = DATA_TRAIN_RESULT_REG->EMI_IDLC_regval;
    *EMI_IDLD = DATA_TRAIN_RESULT_REG->EMI_IDLD_regval;
    *EMI_IDLE = DATA_TRAIN_RESULT_REG->EMI_IDLE_regval;
    *EMI_IDLF = DATA_TRAIN_RESULT_REG->EMI_IDLF_regval;
    *EMI_IDLG = DATA_TRAIN_RESULT_REG->EMI_IDLG_regval;
    *EMI_IDLH = DATA_TRAIN_RESULT_REG->EMI_IDLH_regval;
    /**
        * Enable 1/5 PLL.
        */
    *EMI_CONN |= 0x00000100;

    /**
       * Add new timing delay to meet new EMI timing constrain that after enabling 1/5 DLL.
       */

     gpt_wait_us_dsb(100);

#endif // __EMI_DATA_AUTO_TRACKING_ENABLE

#ifdef SW_WORKAROUND_FOR_DUALRANKS
		 if (((emi_setting->type & 0xF) == 1) && (2 == get_dram_rank_nr()))  // LPDDR1 with dual ranks
		 {
		        /* Tune rank 0 */
			*EMI_GEND =  ((*EMI_GEND) & (~0x00030000)) | 0x10000;
		
			gpt_wait_us_dsb(10);
               
			*EMI_DQSI |= 0x300000FF;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
		
			gpt_wait_us_dsb(10);
			  
			while((*EMI_DQSI & 0x10000) == 0);	  //wait auto-tuning done
			*EMI_DQSI &= (~0x20000000);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE	
		
			gpt_wait_us_dsb(10);
				
		        /* 2nd time enable DQSI_DCM_SW_TUNE to get final DQSA ~ D*/	 
			*EMI_DQSI |= 0x20000000;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
		
			gpt_wait_us_dsb(10);
				
			while((*EMI_DQSI & 0x10000) == 0);	//wait auto-tuning done

			Rank0_DQS0 = *EMI_DQSU;
			Rank0_DQS1 = (Rank0_DQS0 & 0xffff0000) >> 16;
			Rank0_DQS0 = (Rank0_DQS0 & 0x0000ffff);	  
			Rank0_DQS2 = *EMI_DQSV; 
			Rank0_DQS3 = (Rank0_DQS2 & 0xffff0000) >> 16;
			Rank0_DQS2 = (Rank0_DQS2 & 0x0000ffff);	  
 
	              *EMI_DQSI &= (~0x300000FF);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE  
			 
	              gpt_wait_us_dsb(10);
				  
			 /* Tune rank 1 */ 
			  /* switch to read rank1 DQS */		 
			 *EMI_CONN = (*EMI_CONN & (~0x00004000)) | 0x4000;
			 *EMI_GEND =  ((*EMI_GEND) & (~0x00030000)) | 0x20000;
			 
			 gpt_wait_us_dsb(10);
				
			 *EMI_DQSI |= 0x300000FF;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
			 
			 gpt_wait_us_dsb(10);
			   
			 while((*EMI_DQSI & 0x10000) == 0);    //wait auto-tuning done
			 *EMI_DQSI &= (~0x20000000);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE  
			 
			 gpt_wait_us_dsb(10);
				 
		        /* 2nd time enable DQSI_DCM_SW_TUNE to get final DQSA ~ D*/  
			 *EMI_DQSI |= 0x20000000;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
			 
			 gpt_wait_us_dsb(10);
				 
			 while((*EMI_DQSI & 0x10000) == 0);  //wait auto-tuning done
			 
			 Rank1_DQS0 = *EMI_DQSU;
			 Rank1_DQS1 = (Rank1_DQS0 & 0xffff0000) >> 16;
			 Rank1_DQS0 = (Rank1_DQS0 & 0x0000ffff);	
			 Rank1_DQS2 = *EMI_DQSV; 
			 Rank1_DQS3 = (Rank1_DQS2 & 0xffff0000) >> 16;
			 Rank1_DQS2 = (Rank1_DQS2 & 0x0000ffff);	
			 
			 *EMI_DQSI &= (~0x300000FF);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE  
			 
			 /* switch back to default read rank0 DQS */
			 *EMI_CONN = *EMI_CONN & (~0x00004000);

     			 /* switch back to default read rank0 DQS */
		        *EMI_GEND =  ((*EMI_GEND) & (~0x00030000)) | 0x30000;
	
	               
	               gpt_wait_us_dsb(10);
	
	         #ifdef DUAL_RANK_LP1_LOG
	               print("[EMI] 1st DQSI \n");
			 print("[EMI] R0 DQS0 = 0x%X\n", Rank0_DQS0);
			 print("[EMI] R0 DQS1 = 0x%X\n", Rank0_DQS1);
			 print("[EMI] R0 DQS2 = 0x%X\n", Rank0_DQS2);
			 print("[EMI] R0 DQS3 = 0x%X\n", Rank0_DQS3);
			 print("[EMI] R1 DQS0 = 0x%X\n", Rank1_DQS0);
			 print("[EMI] R1 DQS1 = 0x%X\n", Rank1_DQS1);
			 print("[EMI] R1 DQS2 = 0x%X\n", Rank1_DQS2);
			 print("[EMI] R1 DQS3 = 0x%X\n", Rank1_DQS3);
                 #endif
                       //check DQS of corresponding byte if larger than 0x80, or less then 0x80 for each bytes
                        if ((((Rank0_DQS0 < 0x80) && (Rank1_DQS0 < 0x80)) ||((Rank0_DQS0 >= 0x80) && (Rank1_DQS0 >= 0x80))) &&
				(((Rank0_DQS1 < 0x80) && (Rank1_DQS1 < 0x80)) ||((Rank0_DQS1 >= 0x80) && (Rank1_DQS1 >= 0x80)))	&&		
                            (((Rank0_DQS2 < 0x80) && (Rank1_DQS2 < 0x80)) ||((Rank0_DQS2 >= 0x80) && (Rank1_DQS2 >= 0x80)))	&&
                            (((Rank0_DQS3 < 0x80) && (Rank1_DQS3 < 0x80)) ||((Rank0_DQS3 >= 0x80) && (Rank1_DQS3 >= 0x80))))
                       {
                       	  *EMI_DQSA = Rank0_DQS0 + (Rank0_DQS1 << 16);
                       	  *EMI_DQSB = Rank0_DQS2 + (Rank0_DQS3 << 16);
                       	  *EMI_DQSC = Rank1_DQS0 + (Rank1_DQS1 << 16);
                       	  *EMI_DQSD = Rank1_DQS2 + (Rank1_DQS3 << 16);
                       } 
                       else
                       {
                              *EMI_ODLA = *EMI_ODLB = *EMI_ODLC = *EMI_ODLD = *EMI_ODLE = *EMI_ODLF = *EMI_ODLG = *EMI_ODLH = 0x0F0F0F0F;
                              *EMI_ODLI = *EMI_ODLJ = *EMI_ODLK = *EMI_ODLL = *EMI_ODLM = *EMI_ODLN = 0x0F0F0F0F;
                              //*EMI_DQSA = *EMI_DQSB = *EMI_DQSC = *EMI_DQSD = 0x00800080;
                              *EMI_DQSA = Rank0_DQS0 + (Rank0_DQS1 << 16);
                       	  *EMI_DQSB = Rank0_DQS2 + (Rank0_DQS3 << 16);
                       	  *EMI_DQSC = Rank1_DQS0 + (Rank1_DQS1 << 16);
                       	  *EMI_DQSD = Rank1_DQS2 + (Rank1_DQS3 << 16);
				   
				   gpt_wait_us_dsb(10);	

       		       /* Tune 2nd round */	   
       		       /* Tune rank 0 */
       			*EMI_GEND =  ((*EMI_GEND) & (~0x00030000)) | 0x10000;
       		
       			gpt_wait_us_dsb(10);
                      
       			*EMI_DQSI |= 0x300000FF;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
       		
       			gpt_wait_us_dsb(10);
       			  
       			while((*EMI_DQSI & 0x10000) == 0);	  //wait auto-tuning done
       			*EMI_DQSI &= (~0x20000000);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE	
       		
       			gpt_wait_us_dsb(10);
       				
       		        /* 2nd time enable DQSI_DCM_SW_TUNE to get final DQSA ~ D*/	 
       			*EMI_DQSI |= 0x20000000;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
       		
       			gpt_wait_us_dsb(10);
       				
       			while((*EMI_DQSI & 0x10000) == 0);	//wait auto-tuning done
       
       			Rank0_DQS0 = *EMI_DQSU;
       			Rank0_DQS1 = (Rank0_DQS0 & 0xffff0000) >> 16;
       			Rank0_DQS0 = (Rank0_DQS0 & 0x0000ffff);	  
       			Rank0_DQS2 = *EMI_DQSV; 
       			Rank0_DQS3 = (Rank0_DQS2 & 0xffff0000) >> 16;
       			Rank0_DQS2 = (Rank0_DQS2 & 0x0000ffff);	  
        
       	              *EMI_DQSI &= (~0x300000FF);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE  
       			 
       	              gpt_wait_us_dsb(10);
       				  
       			 /* Tune rank 1 */ 
       			  /* switch to read rank1 DQS */		 
       			 *EMI_CONN = (*EMI_CONN & (~0x00004000)) | 0x4000;
       			 *EMI_GEND =  ((*EMI_GEND) & (~0x00030000)) | 0x20000;
       			 
       			 gpt_wait_us_dsb(10);
       				
       			 *EMI_DQSI |= 0x300000FF;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
       			 
       			 gpt_wait_us_dsb(10);
       			   
       			 while((*EMI_DQSI & 0x10000) == 0);    //wait auto-tuning done
       			 *EMI_DQSI &= (~0x20000000);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE  
       			 
       			 gpt_wait_us_dsb(10);
       				 
       		        /* 2nd time enable DQSI_DCM_SW_TUNE to get final DQSA ~ D*/  
       			 *EMI_DQSI |= 0x20000000;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
       			 
       			 gpt_wait_us_dsb(10);
       				 
       			 while((*EMI_DQSI & 0x10000) == 0);  //wait auto-tuning done
       			 
       			 Rank1_DQS0 = *EMI_DQSU;
       			 Rank1_DQS1 = (Rank1_DQS0 & 0xffff0000) >> 16;
       			 Rank1_DQS0 = (Rank1_DQS0 & 0x0000ffff);	
       			 Rank1_DQS2 = *EMI_DQSV; 
       			 Rank1_DQS3 = (Rank1_DQS2 & 0xffff0000) >> 16;
       			 Rank1_DQS2 = (Rank1_DQS2 & 0x0000ffff);	
       			 
       			 *EMI_DQSI &= (~0x300000FF);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE  
       			 
       			 /* switch back to default read rank0 DQS */
       			 *EMI_CONN = *EMI_CONN & (~0x00004000);
       
            			 /* switch back to default read rank0 DQS */
       		        *EMI_GEND =  ((*EMI_GEND) & (~0x00030000)) | 0x30000;
       	
       	               
       	               gpt_wait_us_dsb(10);
	
                      #ifdef DUAL_RANK_LP1_LOG
	                          print("[EMI] 2nd DQSI \n");
				  print("[EMI] R0 DQS0 = 0x%X\n", Rank0_DQS0);
				  print("[EMI] R0 DQS1 = 0x%X\n", Rank0_DQS1);
				  print("[EMI] R0 DQS2 = 0x%X\n", Rank0_DQS2);
				  print("[EMI] R0 DQS3 = 0x%X\n", Rank0_DQS3);
				  print("[EMI] R1 DQS0 = 0x%X\n", Rank1_DQS0);
				  print("[EMI] R1 DQS1 = 0x%X\n", Rank1_DQS1);
				  print("[EMI] R1 DQS2 = 0x%X\n", Rank1_DQS2);
				  print("[EMI] R1 DQS3 = 0x%X\n", Rank1_DQS3);
                     #endif

				  if (Rank0_DQS0 < 0x80)   
				  {
					Rank0_DQS0 = 0x80;
				   }
				  if (Rank0_DQS1 < 0x80)   
				  {
					Rank0_DQS1 = 0x80;
				   }
				  if (Rank0_DQS2 < 0x80)   
				  {
					Rank0_DQS2 = 0x80;
				   }
				  if (Rank0_DQS3 < 0x80)   
				  {
					Rank0_DQS3 = 0x80;
				   }
				  
				  if (Rank1_DQS0 < 0x80)   
				  {
					Rank1_DQS0 = 0x80;
				   }
				  if (Rank1_DQS1 < 0x80)   
				  {
					Rank1_DQS1 = 0x80;
				   }
				  if (Rank1_DQS2 < 0x80)   
				  {
					Rank1_DQS2 = 0x80;
				   }
				  if (Rank1_DQS3 < 0x80)   
				  {
					Rank1_DQS3 = 0x80;
				   }
                           
                              *EMI_DQSA = Rank0_DQS0 + (Rank0_DQS1 << 16);
                           	  *EMI_DQSB = Rank0_DQS2 + (Rank0_DQS3 << 16);
                           	  *EMI_DQSC = Rank1_DQS0 + (Rank1_DQS1 << 16);
                           	  *EMI_DQSD = Rank1_DQS2 + (Rank1_DQS3 << 16);	
                         }  
                     
               #ifdef DUAL_RANK_LP1_LOG					 
			 print("[EMI] Final DQSI \n");
			 print("[EMI] R0 DQS0 = 0x%X\n", Rank0_DQS0);
			 print("[EMI] R0 DQS1 = 0x%X\n", Rank0_DQS1);
			 print("[EMI] R0 DQS2 = 0x%X\n", Rank0_DQS2);
			 print("[EMI] R0 DQS3 = 0x%X\n", Rank0_DQS3);
			 print("[EMI] R1 DQS0 = 0x%X\n", Rank1_DQS0);
			 print("[EMI] R1 DQS1 = 0x%X\n", Rank1_DQS1);
			 print("[EMI] R1 DQS2 = 0x%X\n", Rank1_DQS2);
			 print("[EMI] R1 DQS3 = 0x%X\n", Rank1_DQS3);
               #endif			  
		 }
else		 
#endif //#ifdef SW_WORKAROUND_FOR_DUALRANKS
{
    /**
        * Setup HW EMI calibration for sleep mode resume.
        */
     if (2 == get_dram_rank_nr())
     {
         *EMI_DQSI |= 0x300000FF;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
     }
     else
     {
         *EMI_DQSI |= 0x3000000F;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
     }
     
      gpt_wait_us_dsb(10);

     while((*EMI_DQSI & 0x10000) == 0);  //wait auto-tuning done
     *EMI_DQSI &= (~0x20000000);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE
}

    /**
       * Enable Dummy Read
       */
   if (2 == get_dram_rank_nr())
   {
	 *EMI_DRCT = 0x00000002;	// toggle CS for dummy read	
   }
   else	
   {
	 *EMI_DRCT = 0x00000000;		
   }
    
    if ((emi_setting->type & 0xF) == 1)   //LPDDR1
    {
#ifdef SW_WORKAROUND_FOR_DUALRANKS        
       if  (2 == get_dram_rank_nr())	// LPDDR1 with dual ranks
       {
	    *EMI_DRCT = 0x00000000;	//disable Dummy read because disable DQSI HW auto-tracking	  
       }
	else   
#endif
       {
	*EMI_DRCT |= 0x00000001;
    }
    }
    else if ((emi_setting->type & 0xF) == 2)  //LPDDR2
    {
	*EMI_DRCT |= 0x00000001;  //0x00000045;  
    }
    else if ((emi_setting->type & 0xF) == 3) //PCDDR3
    {
        *EMI_DRCT |= 0x00000001;
    }
    else if ((emi_setting->type & 0xF) == 4)  //LPDDR3
    {
        *EMI_DRCT |= 0x00000001;   
    }

    return ret;
}


int __EMI_EnablePerformancePowerControl(EMI_SETTINGS *emi_setting)
{
    if ((emi_setting->type & 0xF) == 1)
    {
		/**
		   * Setup Precharge & PDN delay
		   */
		//*(volatile kal_uint32*)EMI_PPCT=0xFFDFBF0A;  // must disable memory preserve mode

		*(volatile kal_uint32*)EMI_SLCT=0x3F183F00;

		/**
			* Setup 1/16 freq for HWDCM mode and enable arbitration controls
			*/
		*(volatile kal_uint32*)EMI_ABCT=0xAA300010;

	       *EMI_ADSA = 0x55555210;
		*EMI_ADSB = 0x66665210;
		*EMI_ADSC = 0x55555210;
		*EMI_ADSD = 0x77775210;
		*EMI_ADSE = 0x77556655;
		*EMI_ADSF = 0x44444421;
		*EMI_ADSG = 0x44444421;
		*EMI_ADSH = 0x44444421;
		*EMI_ADSI = 0x77777521;
		*EMI_ADSJ = 0x77444444;

    }
    else if ((emi_setting->type & 0xF) == 2)
    {
		/**
		   * Setup Precharge & PDN delay
		   */
		//*(volatile kal_uint32*)EMI_PPCT=0xFFDFBF0A;  //  must disable memory preserve mode

		*(volatile kal_uint32*)EMI_SLCT=0x3F183F00;

		/**
			* Setup 1/16 freq for HWDCM mode and enable arbitration controls
			*/
		*(volatile kal_uint32*)EMI_ABCT=0xAA300010;

              if (emi_setting->EMI_Freq == 200)
              {
       		  *EMI_ADSA = 0x44444210;
       		  *EMI_ADSB = 0x66665210;
       		  *EMI_ADSC = 0x44444210;
       		  *EMI_ADSD = 0x77775210;
       		  *EMI_ADSE = 0x77446644;
       		  *EMI_ADSF = 0x44444421;
       		  *EMI_ADSG = 0x44444421;
       		  *EMI_ADSH = 0x44444421;
       		  *EMI_ADSI = 0x77777521;
       		  *EMI_ADSJ = 0x77444444;
              }
       	else if (emi_setting->EMI_Freq == 266)
       	{
       		*EMI_ADSA = 0x44443210;
       		*EMI_ADSB = 0x66643210;
       		*EMI_ADSC = 0x44443210;
       		*EMI_ADSD = 0x77643210;
       		*EMI_ADSE = 0x77446644;
       		*EMI_ADSF = 0x44444321;
       		*EMI_ADSG = 0x44444321;
       		*EMI_ADSH = 0x44444321;
       		*EMI_ADSI = 0x77775321;
       		*EMI_ADSJ = 0x77444444;
    	       }
    	       else if (emi_setting->EMI_Freq == 333)
    	       {
		       *EMI_ADSA = 0x55543210;
			*EMI_ADSB = 0x66543210;
			*EMI_ADSC = 0x55543210;
			*EMI_ADSD = 0x76543210;
			*EMI_ADSE = 0x77556655;
			*EMI_ADSF = 0x44444321;
			*EMI_ADSG = 0x55555321;
			*EMI_ADSH = 0x44444321;
			*EMI_ADSI = 0x77765321;
			*EMI_ADSJ = 0x77445544;
    	       }
    }
    else if ((emi_setting->type & 0xF) == 3)  //PCDDR3
{
    /**
       * Setup Precharge & PDN delay
       */
		 //*(volatile kal_uint32*)EMI_PPCT=0xFFDFBF0A;  //must disable memory preserve mode

           	    *(volatile kal_uint32*)EMI_SLCT=0x3F183F00;

    /**
			 * Setup 1/16 freq for HWDCM mode and enable arbitration controls
        */
		     *(volatile kal_uint32*)EMI_ABCT=0xAA300010;

       		  *EMI_ADSA = 0x33333210;
       		  *EMI_ADSB = 0x66654210;
       		  *EMI_ADSC = 0x33333210;
       		  *EMI_ADSD = 0x77754210;
       		  *EMI_ADSE = 0x77336633;
       		  *EMI_ADSF = 0x66666532;
       		  *EMI_ADSG = 0x44444432;
       		  *EMI_ADSH = 0x66666532;
       		  *EMI_ADSI = 0x66666532;
       		  *EMI_ADSJ = 0x77777532;
         }
    else if ((emi_setting->type & 0xF) == 4)  //LPDDR3
	  {
		  /**
			 * Setup Precharge & PDN delay
			 */
		  //*(volatile kal_uint32*)EMI_PPCT=0xFFDFBF0A;  // must disable memory preserve mode

		 *(volatile kal_uint32*)EMI_SLCT=0x3F183F00;

    /**
        * Setup 1/16 freq for HWDCM mode and enable arbitration controls
        */
		  *(volatile kal_uint32*)EMI_ABCT=0xAA300010;

       		  *EMI_ADSA = 0x44444210;
       		  *EMI_ADSB = 0x66665210;
       		  *EMI_ADSC = 0x44444210;
       		  *EMI_ADSD = 0x77775210;
       		  *EMI_ADSE = 0x77446644;
       		  *EMI_ADSF = 0x44444421;
       		  *EMI_ADSG = 0x44444421;
       		  *EMI_ADSH = 0x44444421;
       		  *EMI_ADSI = 0x77777521;
       		  *EMI_ADSJ = 0x77444444;
    }
    else
    {
        print("[EMI] Abnormal DRAM type");
	 while(1);
    }

    return 0;
}

/*************************************************************************
* FUNCTION
*  __EMI_EnableBandwidthLimiter()
*
* DESCRIPTION
*   This routine aims to set EMI
*
* PARAMETERS
*
* RETURNS
*  None
*
* GLOBALS AFFECTED
*
*************************************************************************/
int __EMI_EnableBandwidthLimiter(EMI_SETTINGS *emi_setting)
{
    if ((emi_setting->type & 0xF) == 1)   //LPDDR1
    {
		*EMI_ARBA = 0x0000541A;
		*EMI_ARBB = 0x00005400;
		*EMI_ARBC = 0xFFFF5444;
		*EMI_ARBD = 0x0101561D;
		*EMI_ARBE = 0x99005404;
		*EMI_ARBF = 0x00005401;
		*EMI_ARBG = 0x00080008;
    }
    else if ((emi_setting->type & 0xF) == 2)    //LPDDR2
    {
              if (emi_setting->EMI_Freq == 200)
              {
                   *EMI_ARBA = 0x0000541A;
                   *EMI_ARBB = 0x00005400;
                   *EMI_ARBC = 0xFFFF5444;
                   *EMI_ARBD = 0x0101561D;
                   *EMI_ARBE = 0x99005404;
                   *EMI_ARBF = 0x00005401;
                   *EMI_ARBG = 0x00080008;
              }
       	else if (emi_setting->EMI_Freq == 266)
       	{
			*EMI_ARBA = 0x00005432;
			*EMI_ARBB = 0x00005400;
			*EMI_ARBC = 0xFFFF5434;
			*EMI_ARBD = 0x03035617;
			*EMI_ARBE = 0xC8005402;
			*EMI_ARBF = 0x00005401;
			*EMI_ARBG = 0x00080008;
    	       }
    	       else if (emi_setting->EMI_Freq == 333)
    	       {
              	*EMI_ARBA = 0x00005443;
              	*EMI_ARBB = 0x00005400;
              	*EMI_ARBC = 0xFFFF5429;
              	*EMI_ARBD = 0x03035612;
              	*EMI_ARBE = 0xC8005401;
              	*EMI_ARBF = 0x00005401;
              	*EMI_ARBG = 0x00080008;
    	       }
    }
    else if ((emi_setting->type & 0xF) == 3)  //PCDDR3
    {
           if (0x00070000 == (*EMI_CONN & 0x00070000))  //PCDDR3x32
           {           
		 *EMI_ARBA = 0x00005443;
		 *EMI_ARBB = 0x00005400;
		 *EMI_ARBC = 0xFFFF5429;
		 *EMI_ARBD = 0x06065612;
		 *EMI_ARBE = 0xFE005401;
		 *EMI_ARBF = 0x00005401;
		 *EMI_ARBG = 0x00080008;
           }
	    else //PCDDR3x16
	    {           
		 *EMI_ARBA = 0x00005433;
		 *EMI_ARBB = 0x00005400;
		 *EMI_ARBC = 0x40605440;
		 *EMI_ARBD = 0x0602560A;
		 *EMI_ARBE = 0x58505601;
		 *EMI_ARBF = 0x00005401;
		 *EMI_ARBG = 0x00080008;
            }
    }
    else if ((emi_setting->type & 0xF) == 4)  //LPDDR3
    {
		  *EMI_ARBA = 0x00005443;
		  *EMI_ARBB = 0x00005400;
		  *EMI_ARBC = 0xFFFF5429;
		  *EMI_ARBD = 0x03035612;
		  *EMI_ARBE = 0xC8005401;
		  *EMI_ARBF = 0x00005401;
		  *EMI_ARBG = 0x00080008;
    }

    return 0;

}

/*
 * mt_set_emi: Set up EMI/DRAMC.
 */
void mt_set_emi (void)
{
    unsigned int EMI_reg;
    int index = 0;
    EMI_SETTINGS *emi_set;
    EMI_DATA_TRAIN_REG_t    DATA_TRAIN_RESULT_REG;

    print("[EMI] DDR%d\r\n", mt_get_dram_type ());

    print("[EMI] FLASH ID=%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8]);

    index = mt_get_mdl_number ();

    if (index < 0 || index >=  num_of_emi_records)
    {
        print("[EMI] EMI Set Fail 0x%x\r\n", index);
        return;
    }

    print("[EMI] MDL number = %d\r\n", index);

    emi_set = &emi_settings[index];

    /*********
      *
      * Initial EMI Clock.
      *
      ********/

    if (0 == mtk_pll_init_emi(emi_set->EMI_Freq))
    {
    	print("[EMI] EMI CLK=%d\r\n", emi_set->EMI_Freq);    
    }
    else
    {
    	print("[EMI] EMI CLK fail!");
    }

    gpt_wait_us_dsb(1000);

    /*********
      *
      * Initial LPDDR.
      *
      ********/

    __EMI_InitializeLPDDR(emi_set);


    /*********
      *
      * Data training.
      *
      ********/
#if (CFG_FPGA_PLATFORM)
    *EMI_DQSA = *EMI_DQSB = *EMI_DQSC = *EMI_DQSD = 0x00400040;
	/**
    * Enable 1/5 PLL.
    */
    *EMI_CONN |= 0x00000100;

    /**
    * Add new timing delay to meet new EMI timing constrain that after enabling 1/5 DLL.
    */
    
     gpt_wait_us_dsb(100);

    *EMI_DRCT = 0x00000001;
#else
    #if defined(SAMPLE_BACK_USE)
        if ((emi_set->type & 0xF) == 1)  //LPDDR1
        {
		*EMI_DQSA = *EMI_DQSB = *EMI_DQSC = *EMI_DQSD = 0x00800080;
        }
        else if ((emi_set->type & 0xF) == 2)   //LPDDR2
        {
		*EMI_DQSA = *EMI_DQSB = *EMI_DQSC = *EMI_DQSD = 0x00840084;
        }

	/**
       * Enable 1/5 PLL.
       */
       *EMI_CONN |= 0x00000100;

       /**
       * Add new timing delay to meet new EMI timing constrain that after enabling 1/5 DLL.
       */
       
        gpt_wait_us_dsb(100);


	if (2 == get_dram_rank_nr())
	{
		*EMI_DQSI |= 0x300000FF;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
	}
	else
	{
		*EMI_DQSI |= 0x3000000F;  //Enable DQSI_DCM_SW_TUNE to update DQS pulse counters for exit sefl-refresh
	}

	 gpt_wait_us_dsb(10);

       while((*EMI_DQSI & 0x10000) == 0);  //wait auto-tuning done

	*EMI_DQSI &= (~0x20000000);  //Disable DQSI_DCM_SW_TUNE, Must mask DQSI_AUTO_TUNE_DONE

        /**
     	  * Enable Dummy Read
     	  */
        if ((emi_set->type & 0xF) == 1)	 //LPDDR1
        {
        *EMI_DRCT = 0x00000001;
        }
        else if ((emi_set->type & 0xF) == 2)  //LPDDR2
        {
        *EMI_DRCT = 0x00000001;	//0x00000045;
        }

   #else
    __EMI_EnableDataAutoTracking(&DATA_TRAIN_RESULT_REG, emi_set);
#endif
#endif

    /*********
      *
      * Enable performance/power related module.
      *
      ********/

    __EMI_EnablePerformancePowerControl(emi_set);


    /*********
      *
      * Enable bandwidth limiter.
      *
      ********/

#if (CFG_FPGA_PLATFORM)

#else
    __EMI_EnableBandwidthLimiter(emi_set);
#endif

print("DQSI= 0x%X\n", *EMI_DQSI);
print("DQSA=0x%X\n", *EMI_DQSA);
print("DQSB=0x%X\n", *EMI_DQSB);
print("DQSC=0x%X\n", *EMI_DQSC);
print("DQSD=0x%X\n", *EMI_DQSD);

print("DQSU=0x%X\n", *EMI_DQSU);
print("DQSV=0x%X\n", *EMI_DQSV);
print("DLLV=0x%X\n", *EMI_DLLV);

print("IDLA=0x%X\n", *EMI_IDLA);
#ifdef EMI_DEBUG_LOG
print("IDLB=0x%X\n", *EMI_IDLB);
print("IDLC=0x%X\n", *EMI_IDLC);
print("IDLD=0x%X\n", *EMI_IDLD);
print("IDLE=0x%X\n", *EMI_IDLE);
print("IDLF=0x%X\n", *EMI_IDLF);
print("IDLG=0x%X\n", *EMI_IDLG);
print("IDLH=0x%X\n", *EMI_IDLH);
#endif

print("IDLI=0x%X\n", *EMI_IDLI);
print("IDLJ=0x%X\n", *EMI_IDLJ);

print("ODLA=0x%X\n", *EMI_ODLA);

#ifdef EMI_DEBUG_LOG
print("ODLB=0x%X\n", *EMI_ODLB);
print("ODLC=0x%X\n", *EMI_ODLC);
print("ODLD=0x%X\n", *EMI_ODLD);
print("ODLE=0x%X\n", *EMI_ODLE);
print("ODLF=0x%X\n", *EMI_ODLF);
print("ODLG=0x%X\n", *EMI_ODLG);
print("ODLH=0x%X\n", *EMI_ODLH);

print("ODLI=0x%X\n", *EMI_ODLI);
print("ODLJ=0x%X\n", *EMI_ODLJ);
print("ODLK=0x%X\n", *EMI_ODLK);
print("ODLL=0x%X\n", *EMI_ODLL);
print("ODLM=0x%X\n", *EMI_ODLM);
print("ODLN=0x%X\n", *EMI_ODLN);
#endif

print("CONI=0x%X\n", *EMI_CONI);
print("CONJ=0x%X\n", *EMI_CONJ);
print("CONK=0x%X\n", *EMI_CONK);
print("CONL=0x%X\n", *EMI_CONL);
print("CONN=0x%X\n", *EMI_CONN);
print("GEND=0x%X\n", *EMI_GEND);
print("DRCT=0x%X\n", *EMI_DRCT);

print("[EMI] DRVA = 0x%X\n", *EMI_DRVA);
#ifdef EMI_DEBUG_LOG
print("DRVB=0x%X\n", *EMI_DRVB);

print("DUTA=0x%X\n", *EMI_DUTA);
print("DUTB=0x%X\n", *EMI_DUTB);
print("DUTC=0x%X\n", *EMI_DUTC);

print("DUCA=0x%X\n", *EMI_DUCA);
print("DUCB=0x%X\n", *EMI_DUCB);
print("DUCE=0x%X\n", *EMI_DUCE);
#endif

}


/*************************************************************************
* FUNCTION
*  SRAM_repair()
*
* DESCRIPTION
*   This routine use to repair SRAM
*
* PARAMETERS
*
* RETURNS
*  -1 : fail
*    0 : cuccess
*
*************************************************************************/
int SRAM_repair(REPAIR_MODULE module)
{
    int result = 0;

#if defined(MT6572) ||defined(MT6582)		
    volatile unsigned int i; 

    switch(module)
    {
     case MFG_MMSYS:
	
	  /* 1st mbist */
	  /* MFG & MMSYS SRAM repair */
	  *MFG_RP_CON = 0x0;	  
	  	  
	  gpt_wait_us_dsb(1);
	  *MFG_RP_CON = 0x1;	  
	  	  
	  gpt_wait_us_dsb(1);
	  *MFG_MBIST_MODE = 0x80000002;
	  	  
	  gpt_wait_us_dsb(1);

	  *MDP_WROT_MBISR_RESET = 0x0;
	  	  
	  gpt_wait_us_dsb(1);
	  *MDP_WROT_MBISR_RESET = 0x1;
	  	  
	  gpt_wait_us_dsb(1);
	  *MMSYS_MBIST_CON = 0x08000;
	   	  
	  gpt_wait_us_dsb(1);
	  *MMSYS_MBIST_MODE = 0x000020;
	  	  
	  gpt_wait_us_dsb(10);

	  // wait for mbist
	  while (*MFG_MBIST_DONE_0 != 0x0007fff8);	
	  while (*MMSYS_MBIST_DONE != 0x020020);
	  
         // check status
         if ((*MFG_MBIST_FAIL_0 != 0x00000000) || (*MFG_RP_MON_0 != 0x0000007f) ||
		(*MMSYS_MBIST_FAIL0 != 0x00000000) || (*MDP_WROT_MBIST_FAIL != 0x0) ||(*MDP_WROT_MBIST_OK != 0x0))
         {             
		print("[SRAM] MFG&MMSYS 1st MBIST fail \r\n");
		/* load fuse */
		*MEM_REPAIR = 0xc0000000;
		
		gpt_wait_us_dsb(10);

		/* 2nd mbist */
		*MFG_RP_CON = 0x0;		
		
		gpt_wait_us_dsb(1);
		*MFG_MBIST_MODE = 0x00000000;		
		
		gpt_wait_us_dsb(1);
		*MFG_MBIST_MODE = 0x80000000;		
		
		gpt_wait_us_dsb(1);
		*MFG_RP_CON = 0x1;		
		
		gpt_wait_us_dsb(1);
		*MFG_MBIST_MODE = 0x80000002;		
		
		gpt_wait_us_dsb(1);

		*MMSYS_MBIST_CON = 0x00000;		
		
		gpt_wait_us_dsb(1);
		*MDP_WROT_MBISR_RESET = 0x0;		
		
		gpt_wait_us_dsb(1);
		*MMSYS_MBIST_MODE = 0x000000;		
		
		gpt_wait_us_dsb(1);
		*MMSYS_MBIST_CON = 0x08000;		
		
		gpt_wait_us_dsb(1);
		*MDP_WROT_MBISR_RESET = 0x1;		
		
		gpt_wait_us_dsb(1);
		*MMSYS_MBIST_MODE = 0x000020;
		
		gpt_wait_us_dsb(10);

		// wait for mbist
		while (*MFG_MBIST_DONE_0 != 0x0007fff8);
       	while (*MMSYS_MBIST_DONE != 0x020020);
		
		// check status
		if ((*MFG_MBIST_FAIL_0 != 0x00000000) || (*MFG_RP_MON_0 != 0x0000007f) ||
		     (*MMSYS_MBIST_FAIL0 != 0x00000000) || (*MDP_WROT_MBIST_FAIL != 0x0) ||(*MDP_WROT_MBIST_OK != 0x0))
		{		     
		     print("[SRAM] MFG&MMSYS 2nd MBIST fail \r\n");
                   result = -1;		     		   
		}
		else
		{
			print("[SRAM] MFG&MMSYS 2nd MBIST pass \r\n");
		}
         }
	  else
	  {
		  print("[SRAM] MFG&MMSYS 1st MBIST pass \r\n");
	  }
  
	  *MFG_RP_CON = 0x0;	  
	  *MFG_MBIST_MODE = 0x0; 	  
	  *MDP_WROT_MBISR_RESET = 0x0;		  
	  *MMSYS_MBIST_CON = 0x0;	  
	  *MMSYS_MBIST_MODE = 0x0;
	  	  
	  gpt_wait_us_dsb(1);
	  
	  break;
	   	
	default:

	  result = -1;
         break;	
	}
#endif   //#if defined(MT6572) ||defined(MT6582)		

    return result;
}

