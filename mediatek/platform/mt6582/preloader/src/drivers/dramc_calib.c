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

#include <dramc.h>
#include <typedefs.h>
#include <platform.h>
#include <emi_hw.h>
#include <emi.h>
//#define DEBUG_DRAMC_CALIB
#define DEC_DQS_VALUE (0)
#define DEFAULT_DQSSTEP_VALUE 4
#define HW_DQ_DQS_CALIB_TIMEOUT 10000
#define DQ_DQS_DQM_REMAPPING    

#ifdef  DQ_DQS_DQM_REMAPPING        

#if LPDDR == 2 || LPDDR == 3
unsigned char Bit_DQO_Mapping[32] = 
	{0, 1, 2, 3, 8, 9,10,11, 4, 5,  6,  7, 12, 13, 14, 15, 16, 17, 18, 19, 24,25,26,27, 20, 21, 22, 23, 28,29,30,31};
//	{0, 1, 2, 3, 4, 5,  6,  7, 8, 9,10,11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,25,26,27,28,29,30,31};
unsigned char DQSO_Mapping[4] = {0, 1, 2, 3}; 	// Test
unsigned char DQM_Mapping[4] = {0, 1, 2, 3}; 
#else 
unsigned char Bit_DQO_Mapping[32] = 
	{16, 17, 18, 19, 24, 25, 26, 27, 20, 21, 22, 23, 28,29,30,31, 0, 1, 2, 3, 4, 5,  6,  7, 8, 9,10,11, 12, 13, 14, 15};
//	{0, 1, 2, 3, 4, 5,  6,  7, 8, 9,10,11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,25,26,27,28,29,30,31};
//	{16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,29,30,31, 0, 1, 2, 3, 4, 5,  6,  7, 8, 9,10,11, 12, 13, 14, 15};
unsigned char DQSO_Mapping[4] = {2, 3, 0, 1}; 	// This could be by DQSx for CPU/CMP_ERR bit 0~7, bit 8~15, bit 16~23, bit 24~31
//unsigned char DQSO_Mapping[4] = {0, 1, 2, 3}; 	// This could be by DQSx for CPU/CMP_ERR bit 0~7, bit 8~15, bit 16~23, bit 24~31
unsigned char DQM_Mapping[4] = {2, 3, 0, 1}; 
//unsigned char DQM_Mapping[4] = {0, 1, 2, 3}; 
#endif

#endif

int RANK_CURR = 0;
extern char *opt_gw_coarse_value, *opt_gw_fine_value;
extern char *opt_gw_coarse_value0, *opt_gw_fine_value0;
extern char *opt_gw_coarse_value1, *opt_gw_fine_value1;
extern char* opt_dle_value;
extern void dqsi_gw_dly_coarse_factor_handler(char *);
extern void dqsi_gw_dly_coarse_factor_handler_rank1(char *);
extern void dqsi_gw_dly_fine_factor_handler(char *);
extern void dqsi_gw_dly_fine_factor_handler_rank1(char *);
unsigned int opt_rx_dqs0;
unsigned int opt_rx_dqs1;
unsigned int opt_tx_dq[4];
unsigned int opt_tx_dqs;
unsigned int opt_tx_dqm;

typedef struct _RXDQS_PERBIT_DLY_T
{
    S8 min_cur;
    S8 max_cur;
    S8 min_best;
    S8 max_best;
    U8 center;
    U8 dq_dly_last;
} RXDQS_PERBIT_DLY_T;
#define DQ_DATA_WIDTH 32
#define MAX_RX_DQSDLY_TAPS 128
#define MAX_RX_DQDLY_TAPS 16 
#define DQS_NUMBER 4
#define DQS_BIT_NUMBER (DQ_DATA_WIDTH/DQS_NUMBER)

/* Description
  *	RX DQ/DQS per bit calibration.
  * Registers
  *	- DQIDLY[1:8] : one register had 4 settings (4bits: 0~15, unit 20ps) with corresponding DQx
  *	- R0DELDLY : 4 settings for rank 0 DQS0~DQS3. 7 bits (0~127) with unit 30ps. 
  *	- R1DELDLY : 4 settings for rank 1 DQS0~DQS3. 7 bits (0~127) with unit 30ps.
  * Algorithm
  *	- Set DQS/DQ input delay to 0.
  *	- Delay all DQs from 0 to 15 until all failed.
  *	- Delay DQSs from 0 to 127 to find the pass range (min & max) of each DQ. Further find the largest pass range.
  *	- For each DQS, find the largest middle value of corresponding DQ byte. Then use this value to set each DQS input delay.
  *	- For each DQ, find the difference between original middle DQS delay and max DQS delay per byte. Then delay the difference more to align the middle of DQS per byte.
  */
int do_sw_rx_dq_dqs_calib(void)
{
#define dbg_print 
    int result;
    unsigned int data, backup;
    int temp, timeout;
    unsigned int dqsi_dly0, dqsi_dly1, dqsi_dly2, dqsi_dly3;
    unsigned int test_len = 0x100;
    unsigned int dqidly1, dqidly2, dqidly3, dqidly4,dqidly5,dqidly6,dqidly7;
    unsigned int i,j;
    unsigned int dqs_input_delay;
    unsigned int cmp_err;
    unsigned int max;

    unsigned int dq_dly_max;
    char dqs_delay[DQS_NUMBER];
    char dq_delay_per_bit[DQ_DATA_WIDTH];
    unsigned int dqidly[DQ_DATA_WIDTH/DQS_NUMBER];
    unsigned int dq_tap;
    unsigned int dq_delay_done[DQ_DATA_WIDTH];
    RXDQS_PERBIT_DLY_T dqs_perbit_dly[DQ_DATA_WIDTH];
    result = 0;

    dbg_print("in do_sw_rx_dq_dqs_calib()\n");

#ifndef RELEASE
    dbg_print("*DQIDLY1 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY1));
    dbg_print("*DQIDLY2 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY2));
    dbg_print("*DQIDLY3 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY3));
    dbg_print("*DQIDLY4 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY4));
    dbg_print("*DQIDLY5 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY5));
    dbg_print("*DQIDLY6 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY6));
    dbg_print("*DQIDLY7 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY7));
    dbg_print("*DQIDLY8 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY8));
    dbg_print("*DRAMC_R0DELDLY = 0x%x\n", DRAMC_READ_REG(DRAMC_R0DELDLY));


#endif


    /*1. set DQS delay to 0 first*/
  
        DRAMC_WRITE_REG(0x0,DRAMC_R0DELDLY);
   

    // set DQ delay to 0x0.
    for (i = 0; i < 8; i++)
    {
        DRAMC_WRITE_REG(0x0,DRAMC_DQIDLY1+4*i);
    }

    // set DQ delay structure to 0x0.
    for (i = 0; i < DQ_DATA_WIDTH; i++)
    {
        dq_delay_per_bit[i] = 0x0; 
        dq_delay_done[i] = 0x0;
    }

    // delay DQ to find all failed 
    for(dq_tap = 0 ; dq_tap < MAX_RX_DQDLY_TAPS; dq_tap++ ){
        /* set test patern length*/
       
        DRAMC_WRITE_REG(0x55000000,0x3C);
       
        
        data = DRAMC_READ_REG(0x40);
        DRAMC_WRITE_REG((data & 0xAA000000) | test_len, 0x40);
        //Test Agent 2 write enabling, Test Agent 2 read enabling
        DRAMC_WRITE_SET((1 << 30) | (1 << 31),DRAMC_CONF2); 
        
        while(!(DRAMC_READ_REG(DRAMC_TESTRPT)&(1 << 10)));

        delay_a_while(400);

        cmp_err = DRAMC_READ_REG(DRAMC_CMP_ERR);
        dbg_print("cmp_err:%x\n",cmp_err);
        DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)),DRAMC_CONF2); //disable test agent2 r/w
        if (cmp_err == 0xFFFFFFFF) break; 


	/* Bit i compare result
	  * 	-Compare success & never fail before, record the delay value. (dq_delay_per_bit[i] = delay value)
	  *	-Compare fail. Record fail. (dq_delay_done[i] = 1)
           */

        for (i = 0; i < DQ_DATA_WIDTH; i++)
        {
            if (!(cmp_err&(0x1<<i)) && dq_delay_done[i] == 0)
            {
                dq_delay_per_bit[i] = dq_tap; 
            }
            else
            {
                dq_delay_done[i] = 1;
            }
                dbg_print("%d)0x%x \n",i,dq_delay_per_bit[i]);
        }
        dbg_print("\n");

        for (i = 0; i < DQ_DATA_WIDTH; i+=4)
        {
            dqidly[i/4] = (dq_delay_per_bit[i]) + (dq_delay_per_bit[i+1] << 8) + (dq_delay_per_bit[i+2] << 16) + (dq_delay_per_bit[i+3] << 24);

            dbg_print("dqidly[%d]=0x%x\n",i/4,dqidly[i/4]);
        }
        
        for (i = 0; i < 8; i++)
        {
            DRAMC_WRITE_REG(dqidly[i],DRAMC_DQIDLY1+4*i);
        }
        dbg_print("*DQIDLY1 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY1));
        dbg_print("*DQIDLY2 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY2));
        dbg_print("*DQIDLY3 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY3));
        dbg_print("*DQIDLY4 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY4));
        dbg_print("*DQIDLY5 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY5));
        dbg_print("*DQIDLY6 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY6));
        dbg_print("*DQIDLY7 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY7));
        dbg_print("*DQIDLY8 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY8)); 
    
    }
    // After loop, dq_delay_per_bit[0:31] value non-zero mean the last valid settings when DQS input delay is 0. dq_delay_per_bit[0:31] value 0 means it is already  failed when DQS input delay is 0. Also, current DQIDLY[1:8] settings is the setting of dq_delay_per_bit[0:31].
    // We got the dq input delay in dq_delay_per_bit[i]
    /* 2. initialize parameters */
    for (i = 0; i < DQ_DATA_WIDTH; i++)
    {
        dqs_perbit_dly[i].min_cur = -1;
        dqs_perbit_dly[i].max_cur = -1;
        dqs_perbit_dly[i].min_best = -1;
        dqs_perbit_dly[i].max_best = -1;
        dqs_perbit_dly[i].center = 0;
        dqs_perbit_dly[i].dq_dly_last = dq_delay_per_bit[i];
    }
    /* find the minimum and maximum DQS input delay*/
    for (i = 0; i < MAX_RX_DQSDLY_TAPS; i++)
    {
        dqs_input_delay = (i) + (i << 8) + (i << 16) + (i << 24);
        
        DRAMC_WRITE_REG(dqs_input_delay,DRAMC_R0DELDLY);
      
        
        /* set test patern length*/
        data = DRAMC_READ_REG(0x40);
        DRAMC_WRITE_REG((data & 0xFF000000) | test_len, 0x40);
        //Test Agent 2 write enabling, Test Agent 2 read enabling
        DRAMC_WRITE_SET((1 << 30) | (1 << 31),DRAMC_CONF2); 
        
        while(!(DRAMC_READ_REG(DRAMC_TESTRPT)&(1 << 10)));

        delay_a_while(400);

        cmp_err = DRAMC_READ_REG(DRAMC_CMP_ERR);
        DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)),DRAMC_CONF2); //disable test agent2 r/w


	/* if bit x test pass the first time, record to min input delay. (dqs_per_bit[x].min_cur = delay value.)
	  * If bit x already had min value and no max value and pass fail => max value is this delay-1. (dqs_per_bit[x].max_cur = delay value-1)
	  * If bit x already had min value and no max value and pass and delay value = 127 => max value = 127 (dqs_per_bit[x].max_cur = 127)
           */
        
        for (j = 0; j < DQ_DATA_WIDTH; j++)
        {
            if ((dqs_perbit_dly[j].min_cur == -1) && ((cmp_err&((U32)1<<j)) == 0x0))
            {
                // min pass delay
                dqs_perbit_dly[j].min_cur = i;
            }
            if ((dqs_perbit_dly[j].min_cur != -1) && (dqs_perbit_dly[j].max_cur == -1) && (((cmp_err&((U32)1<<j)) != 0x0) || (i == (MAX_RX_DQSDLY_TAPS-1))) )
            {
                // we get the dqs_perbit_dly pass max
                if ((i == (MAX_RX_DQSDLY_TAPS-1)) && ((cmp_err&((U32)1<<j)) == 0x0))
                {
                    dqs_perbit_dly[j].max_cur = MAX_RX_DQSDLY_TAPS-1;
                }
                else
                {
                    dqs_perbit_dly[j].max_cur = i - 1;
                }

                // there may be more than 1 pass range, find the max range
                // ex: x00xxxxxx00000000000000xx...(get the second one)
                if ((dqs_perbit_dly[j].max_cur-dqs_perbit_dly[j].min_cur) > (dqs_perbit_dly[j].max_best-dqs_perbit_dly[j].min_best))
                {
                    dqs_perbit_dly[j].max_best = dqs_perbit_dly[j].max_cur;
                    dqs_perbit_dly[j].min_best = dqs_perbit_dly[j].min_cur;
                }
                // clear to find the next pass range if it has
                dqs_perbit_dly[j].max_cur = -1;
                dqs_perbit_dly[j].min_cur = -1;
            }

        }
    }
    // 3
    // get dqs delay center per bit
    for (j = 0; j < DQ_DATA_WIDTH; j++)
    {
        if ((dqs_perbit_dly[j].max_best != -1) && (dqs_perbit_dly[j].min_best != -1))
        {
            dqs_perbit_dly[j].center = (dqs_perbit_dly[j].max_best + dqs_perbit_dly[j].min_best) / 2;
            dbg_print("dqs_perbit_dly[%d].center=0x%x\n",j,dqs_perbit_dly[j].center);
        }
    }

    // we get the delay value of the 4 DQS (min of center)
    for (i = 0; i < DQS_NUMBER; i++)
    {
        max = 0;
        // find the max of center
        for (j = 0; j < DQS_BIT_NUMBER; j++)
        {
            if (dqs_perbit_dly[i*DQS_BIT_NUMBER+j].center > max)
            {
                max = dqs_perbit_dly[i*DQS_BIT_NUMBER+j].center;
            }
        }
        // save dqs delay
        dqs_delay[i] = max;
        dbg_print("dqs_delay[%d]=0x%x\n",i,max);
    }
    data = ((U32) dqs_delay[0]) + (((U32)dqs_delay[1])<<8) + (((U32)dqs_delay[2])<<16) + (((U32)dqs_delay[3])<<24);
    /*set dqs input delay*/
    DRAMC_WRITE_REG(data,DRAMC_R0DELDLY);


    // delay DQ ,let dqsdly_ok_center == DQS_delay
    for (i = 0; i < DQ_DATA_WIDTH; i = i+4)
    {
        // every 4-bit dq have the same delay register address
        // dq_dly_max: taps for dq delay to be add
        for (j = 0; j < 4; j++)
        {
            dq_dly_max =  dqs_delay[i/DQS_BIT_NUMBER] - dqs_perbit_dly[i+j].center;
            dbg_print("1.bit:%d)dq_per_bit_dly:0x%x,dq_dly:0x%x\n",i+j,dqs_perbit_dly[i+j].dq_dly_last,dq_dly_max);
            data = dqs_perbit_dly[i+j].dq_dly_last + dq_dly_max;
            data = ((data > (MAX_RX_DQDLY_TAPS-1)) ? (MAX_RX_DQDLY_TAPS-1) : data);
            dqs_perbit_dly[i+j].dq_dly_last = data;

            dbg_print("2.bit:%d)dq_per_bit_dly:0x%x\n",i+j,dqs_perbit_dly[i+j].dq_dly_last);
        }

        data = ((U32) dqs_perbit_dly[i].dq_dly_last) + (((U32)dqs_perbit_dly[i+1].dq_dly_last)<<8) + (((U32)dqs_perbit_dly[i+2].dq_dly_last)<<16) + (((U32)dqs_perbit_dly[i+3].dq_dly_last)<<24);

        DRAMC_WRITE_REG(data,DRAMC_DQIDLY1+i);
    }
    for (j = 0; j < DQ_DATA_WIDTH; j++)
    {
        dbg_print("%d)min:0x%x,max:0x%x\n",j, dqs_perbit_dly[j].min_best, dqs_perbit_dly[j].max_best);
    }
#if defined(DEBUG_DRAMC_CALIB)
    print("*DQIDLY1 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY1));
    print("*DQIDLY2 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY2));
    print("*DQIDLY3 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY3));
    print("*DQIDLY4 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY4));
    print("*DQIDLY5 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY5));
    print("*DQIDLY6 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY6));
    print("*DQIDLY7 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY7));
    print("*DQIDLY8 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY8));
    print("*DRAMC_R0DELDLY = 0x%x\n", DRAMC_READ_REG(DRAMC_R0DELDLY));


#endif
#if defined(DEBUG_DRAMC_CALIB)
    print("*DQIDLY1 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY1));
    // finish we can put result now .
    print("==================================================================\n");
    print("		RX	DQS perbit delay software calibration \n");
    print("==================================================================\n");        
    print("1.0-31 bit dq delay value\n");
    print("==================================================================\n");        
    print("bit|     0  1  2  3  4  5  6  7  8  9\n");
    print("--------------------------------------");
    for (i = 0; i < DQ_DATA_WIDTH; i++)
    {
        j = i / 10;
        if (i == (j*10))
        {
            print("\n");
            print("%d |    ", i);
        }
        print("%d ", dq_delay_per_bit[i]);
    }
    print("\n--------------------------------------\n\n");
    print("==================================================================\n");
    print("2.dqs window\nx=pass dqs delay value (min~max)center \ny=0-7bit DQ of every group\n");
    print("input delay:DQS0 =%d DQS1 = %d DQS2 =%d DQS3 = %d\n", dqs_delay[0], dqs_delay[1], dqs_delay[2], dqs_delay[3]);
    print("==================================================================\n");
    print("bit	DQS0	 bit      DQS1     bit     DQS2     bit     DQS3\n");
    for (i = 0; i < DQS_BIT_NUMBER; i++)
    {
        print("%d  (%d~%d)%d  %d  (%d~%d)%d  %d  (%d~%d)%d  %d  (%d~%d)%d\n", \
            i,    dqs_perbit_dly[i].min_best, dqs_perbit_dly[i].max_best, dqs_perbit_dly[i].center, \
            i+8,  dqs_perbit_dly[i+8].min_best, dqs_perbit_dly[i+8].max_best, dqs_perbit_dly[i+8].center, \
            i+16, dqs_perbit_dly[i+16].min_best, dqs_perbit_dly[i+16].max_best, dqs_perbit_dly[i+16].center, \
            i+24, dqs_perbit_dly[i+24].min_best, dqs_perbit_dly[i+24].max_best, dqs_perbit_dly[i+24].center);
    }
    print("==================================================================\n");
    print("3.dq delay value last\n");
    print("==================================================================\n");
    print("bit|    0  1  2  3  4  5  6  7  8   9\n");
    print("--------------------------------------");
    for (i = 0; i < DQ_DATA_WIDTH; i++)
    {
        j = i / 10;
        if (i == (j*10))
        {
            print("\n");
            print("%d |    ", i);
        }
        print("%d ", dqs_perbit_dly[i].dq_dly_last);
    }
    print("\n==================================================================\n");

#endif

    return 0;
}
typedef struct _TXDQS_PERBYTE_DLY_T
{
    S8 first_dqdly_pass;
    S8 last_dqdly_pass;
    S8 total_dqdly_pass;
    S8 first_dqsdly_pass;
    S8 last_dqsdly_pass;
    S8 total_dqsdly_pass;
    U8 best_dqdly;
    U8 best_dqsdly;
    U8 dq;
    U8 dqs;
} TXDQS_PERBYTE_DLY_T;
static void  calib_clk_output_dly_factor_handler(unsigned int value)
{
        unsigned int data;
        //adjust CLK output delay
        data = DRAMC_READ_REG(DRAMC_PADCTL1);
        data = (data & 0xF0FFFFFF) | (value << 24);
        DRAMC_WRITE_REG(data,DRAMC_PADCTL1);
}

static void  calib_cmd_addr_output_dly_factor_handler(unsigned int value)
{
        unsigned int data;
        //adjust CMD delay
        data = DRAMC_READ_REG(DRAMC_CMDDLY0);
        data &= 0xE0E0E0E0;
        data |= (value  << 24) | (value << 16) | (value << 8) | value;
        DRAMC_WRITE_REG(data, DRAMC_CMDDLY0);

        data = DRAMC_READ_REG(DRAMC_CMDDLY1);
        data &= 0xE0E0E0E0;
        data |= (value  << 24) | (value << 16) | (value << 8) | value;
        DRAMC_WRITE_REG(data, DRAMC_CMDDLY1);

        data = DRAMC_READ_REG(DRAMC_CMDDLY2);
        data &= 0xE0E0E0E0;
        data |= (value  << 24) | (value << 16) | (value << 8) | value;
        DRAMC_WRITE_REG(data, DRAMC_CMDDLY2);

        data = DRAMC_READ_REG(DRAMC_CMDDLY3);
        data &= 0xE0E0E0E0;
        data |= (value  << 24) | (value << 16) | (value << 8) | value;
        DRAMC_WRITE_REG(data, DRAMC_CMDDLY3);

        data = DRAMC_READ_REG(DRAMC_CMDDLY4);
        data &= 0xE0E0E0E0;
        data |= (value  << 24) | (value << 16) | (value << 8) | value;
        DRAMC_WRITE_REG(data, DRAMC_CMDDLY4);

        data = DRAMC_READ_REG(DRAMC_CMDDLY5);
        data &= 0xE0E0E0EF;
        data |= (value << 24) | (value << 16) | (value << 8);
        DRAMC_WRITE_REG(data, DRAMC_CMDDLY5);

        data = DRAMC_READ_REG(DRAMC_DQSCAL0);
        data &= 0xF0F0FFFF;
        data |= (value << 24) | (value << 16);
        DRAMC_WRITE_REG(data, DRAMC_DQSCAL0);

}


#define MAX_TX_DQDLY_TAPS 16 
#define MAX_TX_DQSDLY_TAPS 16 
#define TX_STEPS_LPDDR2 8   // This value is obtained by measurement of waveform for the relationship between DQS and CLK.
#define TX_STEPS_DDR3 2

/* Description
  *	TX DQ/DQS per byte calibration
  * Related registers
  *	- PADCTL3 : DQS[0:3]ODLY (0x10011014)
  *		- 4 bits (0~15)
  *		- 1 step 20ps.
  *	- PADCTL2 : DQM[0:3]DLY (0x10011010)
  *		- 4 bits (0~15)
  *		- 1 step 20ps.
  *	- DQODLY[1:4] : DQ[0:31]DLY (0x10011200~0x1001120c)
  *		- 4 bits (0~15)
  *		- 1 step 20ps
  *	- DRAMC_PADCTL1 
  * 		- bit 27~bit 24 : clock delay.
  *	- DRAMC_CMDDLY[0:5], DRAMC_DQSCAL0
  *		- Address/cmd delay
  * Algorithm
  *	- Set DQ/DQM output delay to 0 and loop DQS output delay value from 0 to 15. Check the pass range.
  *	- Set DQS output delay value to 0 and loop DQ/DQM output delay value from 0 to 15. Check the pass range.
  *	- If two pass range equal, set both output delay to 8.
  *	- If not equal, add (delay difference)/2 to the one with larger pass range.
  */
int do_sw_tx_dq_dqs_calib(void){
    TXDQS_PERBYTE_DLY_T dqs_perbyte_dly[4];
    unsigned int data;
    unsigned int byte;
    int i,j;
    unsigned int mask;
    unsigned int test_len = 0x3FF;
    unsigned int cmp_err_1,cmp_err_2;
    unsigned int dq,dqs;
    unsigned int finish_count;
    unsigned int tx_steps ;

    if((*(V_UINT32P)(DRAMC0_BASE + (0x00e4)) >> 7) & 0x1) {
        tx_steps = TX_STEPS_DDR3;
    } else {
        tx_steps = TX_STEPS_LPDDR2;
    }
     

 
    dbg_print("in do_sw_tx_dq_dqs_calib()\n");
    for(i = 0; i < 4; i++)
    {
        //dqs_perbyte_dly[i].first_dqdly_pass = -1; 
        dqs_perbyte_dly[i].first_dqdly_pass = 0; 
        dqs_perbyte_dly[i].last_dqdly_pass = -1; 
        //dqs_perbyte_dly[i].first_dqsdly_pass = -1; 
        dqs_perbyte_dly[i].first_dqsdly_pass = 0; 
        dqs_perbyte_dly[i].last_dqsdly_pass = -1; 
        dqs_perbyte_dly[i].best_dqdly= -1; 
        dqs_perbyte_dly[i].best_dqsdly= -1; 
        dqs_perbyte_dly[i].dq= 0; 
        dqs_perbyte_dly[i].dqs= 0; 
        dqs_perbyte_dly[i].total_dqdly_pass= 0; 
        dqs_perbyte_dly[i].total_dqsdly_pass= 0; 
    }
    // used for early break
    finish_count = 0;
    
    /* 1. Tx DQS/DQ all zero*/
    /* 1.1 DQM*/
    //DRAMC_WRITE_CLEAR(0x0000FFFF,DRAMC_PADCTL3);
    DRAMC_WRITE_REG(0x0,DRAMC_PADCTL2);
    /* 1.2 DQ*/
    for (i = 0 ; i < 4; i ++)
    {
        //dbg_print("addr:%x\n",DRAMC_DQODLY1+4*i);
        DRAMC_WRITE_REG(0x0, (DRAMC_DQODLY1+4*i)); 
        
    }
    //for (byte = 0; byte < 4; byte ++)
    //{
        /*for (i = 0 ; i < 4; i ++)
        {
            //dbg_print("addr:%x\n",DRAMC_DQODLY1+4*i);
            DRAMC_WRITE_REG(0x0, (DRAMC_DQODLY1+4*i)); 

        }*/
        //dqs_perbyte_dly[byte].first_dqsdly_pass = 0;
        //dbg_print("dqs_perbyte_dly.first_dqsdly_pass=%x \n",dqs_perbyte_dly[byte].first_dqsdly_pass);
        //dqs_perbyte_dly[byte].last_dqsdly_pass =  MAX_TX_DQSDLY_TAPS - 1;
        /* 2. fix DQ delay = 0, delay DQS to find the pass range  */
        for (i = MAX_TX_DQSDLY_TAPS-1; i >= 0 ; i--)
        {
            data = DRAMC_READ_REG(DRAMC_PADCTL3);
            mask = (0xFFFF0000);
            data = (data & mask) | ((i <<0) | (i<<4) | (i <<8) | (i <<12));
            //dbg_print("mask:%x, data:%x\n",mask,data);
            DRAMC_WRITE_REG(data,DRAMC_PADCTL3);

			if(mt_get_dram_type() != TYPE_PCDDR3)
			{//PCDDR3 no need 
            //adjust CMD addr output delay
            calib_cmd_addr_output_dly_factor_handler(i);
            //adjust CLK delay
            calib_clk_output_dly_factor_handler(i);
            //gating window calibration
            do_dqs_gw_calib_1();
			}
            /* 2.2 use test agent to find the pass range */
            /* set test patern length*/
            DRAMC_WRITE_REG(0x55000000,0x3C);
            data = DRAMC_READ_REG(0x40);
            DRAMC_WRITE_REG((data & 0xAA000000) | test_len, 0x40);

            //Test Agent 2 write enabling
            DRAMC_WRITE_SET( (1 << 31),DRAMC_CONF2); //enable write
            DRAMC_WRITE_SET( (1 << 30)| (1 << 31),DRAMC_CONF2); //enable read
            while(!(DRAMC_READ_REG(DRAMC_TESTRPT)&(1 << 10)));
            delay_a_while(400);
            cmp_err_1 = DRAMC_READ_REG(DRAMC_CMP_ERR);
            DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)),DRAMC_CONF2); //disable test agent2 r/w
            //print("DQS loop = %d, cmp_err_1 = %x \n",i, cmp_err_1);

            for (byte=0; byte < 4; byte++)
            {
                if (dqs_perbyte_dly[byte].last_dqsdly_pass == -1)
                {
                    if (!((cmp_err_1) & (0xFF<<(byte*8))))
                    {
                        dqs_perbyte_dly[byte].last_dqsdly_pass = i;
                        finish_count++;
                        //print("dqs_perbyte_dly.last_dqsdly_pass[%d]=%d,  finish count=%d \n",byte, dqs_perbyte_dly[byte].last_dqsdly_pass, finish_count);                        
                        //break;
                    }
                }
            }

            if (finish_count==4)
            {
                break;
            }
        }
        
        /* 3. fix DQS delay = 0, delay DQ to find the pass range  */
        DRAMC_WRITE_CLEAR(0xFFFF ,DRAMC_PADCTL3);

		if(mt_get_dram_type() != TYPE_PCDDR3)
		{//PCDDR3 no need 
        //adjust CMD addr output delay
        calib_cmd_addr_output_dly_factor_handler(0);
        //adjust CLK delay
        calib_clk_output_dly_factor_handler(0);
        //gating window calibration
        do_dqs_gw_calib_1();
		}

        //dqs_perbyte_dly[byte].first_dqdly_pass = 0;
        //dbg_print("dqs_perbyte_dly.first_dqdly_pass=%x \n",dqs_perbyte_dly[byte].first_dqdly_pass);

        finish_count = 0;
        
        for (i = MAX_TX_DQDLY_TAPS-1; i >= 0; i--)
        {

            /* 3.1 delay DQ output delay */
            data = 
                ((i & 0xF) << 0)       /* DQM0DLY: DRAMC_PADCTL2[0:3],         4 bits */
              | ((i & 0xF) << 4)       /* DQM1DLY: DRAMC_PADCTL2[4:7], 4 bits */
              | ((i & 0xF) << 8)       /* DQM2DLY: DRAMC_PADCTL2[8:11],        4 bits */
              | ((i & 0xF) << 12);
            DRAMC_WRITE_REG(data , DRAMC_PADCTL2/* 0x10 */);
            data =    
                ((i & 0xF) << 0)             /* DQ0DLY: DRAMC_DQODLY1[0:3],          4 bits */
              | ((i & 0xF) << 4)               /* DQ1DLY: DRAMC_DQODLY1[4:7],          4 bits */
              | ((i & 0xF) << 8)               /* DQ2DLY: DRAMC_DQODLY1[8:11],         4 bits */
              | ((i & 0xF) << 12)      /* DQ3DLY: DRAMC_DQODLY1[12:15],        4 bits */
              | ((i & 0xF) << 16)      /* DQ4DLY: DRAMC_DQODLY1[16:19],        4 bits */
              | ((i & 0xF) << 20)      /* DQ5DLY: DRAMC_DQODLY1[20:23],        4 bits */
              | ((i & 0xF) << 24)      /* DQ6DLY: DRAMC_DQODLY1[24:27],        4 bits */
              | ((i & 0xF) << 28); 
            DRAMC_WRITE_REG(data,DRAMC_DQODLY1);
            DRAMC_WRITE_REG(data,DRAMC_DQODLY2);
            DRAMC_WRITE_REG(data,DRAMC_DQODLY3);
            DRAMC_WRITE_REG(data,DRAMC_DQODLY4);


            //dbg_print("%x = %x\n",DRAMC_DQODLY1+(4*byte),DRAMC_READ_REG(DRAMC_DQODLY1+(4*byte)));
            /* 3.2 use test agent to find the pass range */
            /* set test patern length*/
            DRAMC_WRITE_REG(0x55000000,0x3C);
            data = DRAMC_READ_REG(0x40);
            DRAMC_WRITE_REG((data & 0xAA000000) | test_len, 0x40);
            //Test Agent 2 write enabling
            DRAMC_WRITE_SET( (1 << 31),DRAMC_CONF2); //enable write
            DRAMC_WRITE_SET( (1 << 30)| (1 << 31),DRAMC_CONF2); //enable read
            while(!(DRAMC_READ_REG(DRAMC_TESTRPT)&(1 << 10)));
            delay_a_while(400);

            cmp_err_1 = DRAMC_READ_REG(DRAMC_CMP_ERR);
            //print("DQ loop=%d, cmp_err_1 = %x\n",i, cmp_err_1);
            DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)),DRAMC_CONF2); //disable test agent2 r/w
 
            for (byte=0; byte < 4; byte++)
            {
                if (dqs_perbyte_dly[byte].last_dqdly_pass == -1)
                {
                    if (!(cmp_err_1 &(0xFF<<(byte*8))))
                    {
                        dqs_perbyte_dly[byte].last_dqdly_pass = i;                        
                        finish_count++;
                        //print("dqs_perbyte_dly.last_dqdly_pass[%d]=%d,  finish count=%d \n",byte, dqs_perbyte_dly[byte].last_dqdly_pass, finish_count);   
                        //break;
                    }                    
                }
            }

            if (finish_count==4)
            {
                break;
            }
        }

        for (byte=0; byte < 4; byte++)
        {

        dqs_perbyte_dly[byte].total_dqsdly_pass = dqs_perbyte_dly[byte].last_dqsdly_pass - dqs_perbyte_dly[byte].first_dqsdly_pass + 1;
        //print("total_dqsdly_pass:%x\n", dqs_perbyte_dly[byte].total_dqsdly_pass);
        dqs_perbyte_dly[byte].total_dqdly_pass = dqs_perbyte_dly[byte].last_dqdly_pass - dqs_perbyte_dly[byte].first_dqdly_pass + 1;
        //dbg_print("total_dqdly_pass:%x\n", dqs_perbyte_dly[byte].total_dqdly_pass);
        /* 4. find the middle of the pass range of DQ and DQS*/
        /* 5. if the middle of the pass range is in the DQ, the delay of (DQS,DQ) is (0,DQ), 
         *    if the middle of the pass range is in the DQS the delay of (DQS,DQ) is (DQS,0)*/
        if (dqs_perbyte_dly[byte].total_dqdly_pass == dqs_perbyte_dly[byte].total_dqsdly_pass)
        {
            dqs_perbyte_dly[byte].dqs = dqs_perbyte_dly[byte].dq = tx_steps; 
        }
        else if (dqs_perbyte_dly[byte].total_dqdly_pass > dqs_perbyte_dly[byte].total_dqsdly_pass) 
        {
            dqs_perbyte_dly[byte].dqs = 0 + tx_steps; 
            dqs_perbyte_dly[byte].dq += (dqs_perbyte_dly[byte].total_dqdly_pass - dqs_perbyte_dly[byte].total_dqsdly_pass)/2 + tx_steps; 
            if (dqs_perbyte_dly[byte].dq > 0xf){ 
				dbg_print("warning:byte %d,dq:%x",byte,dqs_perbyte_dly[byte].dq);
				dqs_perbyte_dly[byte].dq = 0xf;
			}
        } 
        else
        {
            dqs_perbyte_dly[byte].dqs += (dqs_perbyte_dly[byte].total_dqsdly_pass - dqs_perbyte_dly[byte].total_dqdly_pass)/2 + tx_steps; 
            if (dqs_perbyte_dly[byte].dqs > 0xf) {
				dbg_print("warning:byte %d,dqs:%x",byte,dqs_perbyte_dly[byte].dqs);
				dqs_perbyte_dly[byte].dqs = 0xf;
			}
            dqs_perbyte_dly[byte].dq = 0 + tx_steps; 
        }
        print("byte:%x, (DQS,DQ)=(%x,%x)\n",byte,dqs_perbyte_dly[byte].dqs, dqs_perbyte_dly[byte].dq);
        /* 6. fix the (DQS,DQ) for this byte, find the next byte */
        
    }
        
    //}

#ifdef DQ_DQS_DQM_REMAPPING        
    data = 0;
    for (byte = 0 ; byte < 4; byte ++)
    {
    	data |= (dqs_perbyte_dly[byte].dq << (DQM_Mapping[byte]*4));
    }

    DRAMC_WRITE_REG(data , DRAMC_PADCTL2/* 0x10 */);
    opt_tx_dqm = data;

    for (byte = 0 ; byte < 4; byte ++)
    {
    	opt_tx_dq[byte] = 0;
    }
    for (byte = 0 ; byte < 4; byte ++)
    {
         unsigned int uiRemapDQ;
         dq = dqs_perbyte_dly[byte].dq;
    	for (i=0; i<8; i++) {
    		uiRemapDQ = Bit_DQO_Mapping[(byte<<3)+i];
    		opt_tx_dq[uiRemapDQ>>3] |= ((dq & 0xF) << ((uiRemapDQ & 0x07)<<2));
    	}
    }
    
    for (byte = 0 ; byte < 4; byte ++)
    {
    	   //dbg_print("DRAMC_DQODLY%d=%xh\n", byte+1, opt_tx_dq[byte]);
            DRAMC_WRITE_REG(opt_tx_dq[byte], DRAMC_DQODLY1+(4*byte));
    }

    data = 0;
    for (byte = 0 ; byte < 4; byte ++)
    {
    	data |= (dqs_perbyte_dly[byte].dqs << (DQSO_Mapping[byte]*4));
    }

    DRAMC_WRITE_REG(data , DRAMC_PADCTL3/* 0x14 */);
    opt_tx_dqs = data;
    //print("*(%d):%x\n",DRAMC_PADCTL3,data); 
    DRAMC_WRITE_REG(data,DRAMC_PADCTL3);

#else
    data = 
        ((dqs_perbyte_dly[0].dq & 0xF) << 0)             /* DQM0DLY: DRAMC_PADCTL2[0:3],         4 bits */
      | ((dqs_perbyte_dly[1].dq & 0xF) << 4)       /* DQM1DLY: DRAMC_PADCTL2[4:7], 4 bits */
      | ((dqs_perbyte_dly[2].dq & 0xF) << 8)       /* DQM2DLY: DRAMC_PADCTL2[8:11],        4 bits */
      | ((dqs_perbyte_dly[3].dq & 0xF) << 12);
    DRAMC_WRITE_REG(data , DRAMC_PADCTL2/* 0x10 */);
    opt_tx_dqm = data;
    for (byte = 0 ; byte < 4; byte ++)
    {
            dq = dqs_perbyte_dly[byte].dq;
            data =    
                ((dq & 0xF) << 0)             /* DQ0DLY: DRAMC_DQODLY1[0:3],          4 bits */
              | ((dq & 0xF) << 4)               /* DQ1DLY: DRAMC_DQODLY1[4:7],          4 bits */
              | ((dq & 0xF) << 8)               /* DQ2DLY: DRAMC_DQODLY1[8:11],         4 bits */
              | ((dq & 0xF) << 12)      /* DQ3DLY: DRAMC_DQODLY1[12:15],        4 bits */
              | ((dq & 0xF) << 16)      /* DQ4DLY: DRAMC_DQODLY1[16:19],        4 bits */
              | ((dq & 0xF) << 20)      /* DQ5DLY: DRAMC_DQODLY1[20:23],        4 bits */
              | ((dq & 0xF) << 24)      /* DQ6DLY: DRAMC_DQODLY1[24:27],        4 bits */
              | ((dq & 0xF) << 28); 
            DRAMC_WRITE_REG(data,DRAMC_DQODLY1+(4*byte));
            opt_tx_dq[byte] = data;

    }
    data =(dqs_perbyte_dly[0].dqs) | (dqs_perbyte_dly[1].dqs<<4 | dqs_perbyte_dly[2].dqs << 8) | (dqs_perbyte_dly[3].dqs << 12);
    DRAMC_WRITE_REG(data , DRAMC_PADCTL3/* 0x14 */);
    opt_tx_dqs = data;
    //print("%d,data:%x\n",DRAMC_PADCTL3,data); DRAMC_WRITE_REG(data,DRAMC_PADCTL3);

#endif 
    return 0;
}


/*
 * dramc_calib: Do DRAMC calibration.
 * Return error code;
 */
int dramc_calib(void)
{
    int err;
    int reg_val;
    int rank0_col,rank1_col,bak_cona,bak_conf1;
//#define DEBUG_DRAMC_CALIB
#if defined(DEBUG_DRAMC_CALIB)
    int temp;

    /* enable ARM CPU PMU */
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "BIC %0, %0, #1 << 0\n"   /* disable */
        "ORR %0, %0, #1 << 2\n"   /* reset cycle count */
        "BIC %0, %0, #1 << 3\n"   /* count every clock cycle */
        "MCR p15, 0, %0, c9, c12, 0\n"
        : "+r"(temp)
        :
        : "cc"
    );
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "ORR %0, %0, #1 << 0\n"   /* enable */
        "MCR p15, 0, %0, c9, c12, 0\n"
        "MRC p15, 0, %0, c9, c12, 1\n"
        "ORR %0, %0, #1 << 31\n"
        "MCR p15, 0, %0, c9, c12, 1\n"
        : "+r"(temp)
        :
        : "cc"
    );
#endif
    RANK_CURR = 0;
    /*modify MA type*/
    bak_cona = *(volatile unsigned *)(EMI_CONA);
    bak_conf1 = *((volatile unsigned *)(DRAMC0_BASE + 0x04));

    //printf("before:0x04:%x\n",*((volatile unsigned *)(DRAMC0_BASE + 0x04)));
    rank0_col = (*(volatile unsigned *)(EMI_CONA) & (0x3<<4))>>4;
    *((volatile unsigned *)(DRAMC0_BASE + 0x04)) = (*((volatile unsigned *)(DRAMC0_BASE + 0x04)) & (~(0x3 << 8)))| (rank0_col << 8);
    //printf("after:reg_val:%x,0x04:%x\n",rank0_col,*((volatile unsigned *)(DRAMC0_BASE + 0x04)));
    /* do calibration for DQS gating window (phase 1) for rank 0*/
    err = do_dqs_gw_calib_1();
    if (err < 0) {
        goto dramc_calib_exit;
    }

    opt_gw_coarse_value0 = opt_gw_coarse_value; 
    opt_gw_fine_value0 = opt_gw_fine_value;
    print("rank 0 coarse = %s\n", opt_gw_coarse_value0 );
    print("rank 0 fine = %s\n", opt_gw_fine_value0  );
    /* do DLE calibration */
    err = do_dle_calib();
    if (err < 0) {
        goto dramc_calib_exit;
    }
    /* do SW RX calibration for DQ/DQS input delay */
    err = do_sw_rx_dq_dqs_calib();
    if (err < 0) {
        goto dramc_calib_exit;
    }
    opt_rx_dqs0 = DRAMC_READ_REG(DRAMC_R0DELDLY);
    if ( *(volatile unsigned *)(EMI_CONA)& 0x20000)      // Check dual ranks.
    {
        /*modify MA type*/
        //printf("before:0x04:%x\n",*((volatile unsigned *)(DRAMC0_BASE + 0x04)));
        RANK_CURR = 1;
        rank1_col = (*(volatile unsigned *)(EMI_CONA) & (0x3<<6))>>6;
        *((volatile unsigned *)(DRAMC0_BASE + 0x04)) = (*((volatile unsigned *)(DRAMC0_BASE + 0x04)) & (~(0x3 << 8)))| (rank1_col << 8);
        *((volatile unsigned *)(EMI_CONA)) = (*((volatile unsigned *)(EMI_CONA)) & (~(0x3 << 4)))| (rank1_col << 4);
        //print("after:rank1_col:%x,0x04:%x,%x\n",rank1_col,*((volatile unsigned *)(DRAMC0_BASE + 0x04)), *((volatile unsigned *)(EMI_CONA)));

        //make sure we in chip select 1. Swap CS0 and CS1.
        *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) |= (0x8);
        *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) |= (0x8);
        /* do calibration for DQS gating window (phase 1) for rank 1*/
        err = do_dqs_gw_calib_1();
        if (err < 0) {
            goto dramc_calib_exit;
        }

        opt_gw_coarse_value1 = opt_gw_coarse_value; 
        opt_gw_fine_value1 = opt_gw_fine_value;
        print("rank 1 coarse = %s\n", opt_gw_coarse_value1 );
        print("rank 1 fine = %s\n", opt_gw_fine_value1  );
            /* do DLE calibration */
        err = do_dle_calib();
        if (err < 0) {
            goto dramc_calib_exit;
        }
        /* do SW RX calibration for DQ/DQS input delay */
        err = do_sw_rx_dq_dqs_calib();
        if (err < 0) {
            goto dramc_calib_exit;
        }
        opt_rx_dqs1 = DRAMC_READ_REG(DRAMC_R0DELDLY);

        dqsi_gw_dly_coarse_factor_handler_rank1(opt_gw_coarse_value1);
        dqsi_gw_dly_fine_factor_handler_rank1(opt_gw_fine_value1);

        DRAMC_WRITE_REG(opt_rx_dqs1,DRAMC_R1DELDLY);
        //swap back
        //make sure we in chip select 0 now
        *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) &= (~0x8);
        *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) &= (~0x8);
    }
    RANK_CURR = 0;
    *(volatile unsigned *)(EMI_CONA) = bak_cona;
    *((volatile unsigned *)(DRAMC0_BASE + 0x04))= bak_conf1;
    dqsi_gw_dly_coarse_factor_handler(opt_gw_coarse_value0);
    dqsi_gw_dly_fine_factor_handler(opt_gw_fine_value0);
    DRAMC_WRITE_REG(opt_rx_dqs0,DRAMC_R0DELDLY);

    //print("[MEM]CONA%x,conf1:%x\n",*((volatile unsigned *)(EMI_CONA)),*((volatile unsigned *)(DRAMC0_BASE + 0x04)));
     /* do SW TX calibration for DQ/DQS output delay */
    err = do_sw_tx_dq_dqs_calib();
    if (err < 0) {
        goto dramc_calib_exit;
    }
    dqsi_gw_dly_coarse_factor_handler(opt_gw_coarse_value0);
    dqsi_gw_dly_fine_factor_handler(opt_gw_fine_value0);    

dramc_calib_exit:

#if defined(DEBUG_DRAMC_CALIB)
    /* get CPU cycle count from the ARM CPU PMU */
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "BIC %0, %0, #1 << 0\n"   /* disable */
        "MCR p15, 0, %0, c9, c12, 0\n"
        "MRC p15, 0, %0, c9, c13, 0\n"
        : "+r"(temp)
        :
        : "cc"
    );
    print("DRAMC calibration takes %d CPU cycles\n\r", temp);
#endif
    return err;
}



