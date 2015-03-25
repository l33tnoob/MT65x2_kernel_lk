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

#ifndef MT_EMI_H
#define MT_EMI_H

#include "typedefs.h"

//typedef unsigned int            kal_uint32;
//typedef int			kal_int32;

//extern void mt6516_set_emi (void);
//extern void mt6516_256M_mem_setting (void);
int get_dram_rank_nr (void);
void get_dram_rank_size (int dram_rank_size[]);
#define DDR1  1
#define DDR2  2
#define PCDDR3	3
#define LPDDR3  4
typedef struct
{
    int  type;                /* 0x0000 : Invalid
                                 0x0001 : Discrete DDR1
                                 0x0002 : Discrete DDR2
                                 0x0003 : Discrete PCDDR3
                                 0x0004 : Discrete LPDDR3
                                 0x0101 : MCP(NAND+LPDDR1)
                                 0x0102 : MCP(NAND+LPDDR2)
                                 0x0103 : MCP(NAND+PCDDR3)
                                 0x0104 : MCP(NAND+LPDDR3)
                                 0x0201 : MCP(eMMC+LPDDR1)
                                 0x0202 : MCP(eMMC+LPDDR2)
                                 0x0203 : MCP(eMMC+PCDDR3)
                                 0x0204 : MCP(eMMC+LPDDR3)
                              */
    char  Flash_ID[12];
    int   flash_id_length;              // EMMC and NAND ID/FW ID checking length
    kal_uint32 EMI_Freq;              //200 / 266 /333 Mhz
    kal_uint32 EMI_DRVA_value;
    kal_uint32 EMI_DRVB_value;
 
    kal_uint32 EMI_ODLA_value;
    kal_uint32 EMI_ODLB_value;
    kal_uint32 EMI_ODLC_value;
    kal_uint32 EMI_ODLD_value;
    kal_uint32 EMI_ODLE_value;
    kal_uint32 EMI_ODLF_value;
    kal_uint32 EMI_ODLG_value;
    kal_uint32 EMI_ODLH_value;
    kal_uint32 EMI_ODLI_value;
    kal_uint32 EMI_ODLJ_value;
    kal_uint32 EMI_ODLK_value;
    kal_uint32 EMI_ODLL_value;
    kal_uint32 EMI_ODLM_value;
    kal_uint32 EMI_ODLN_value;
 
    kal_uint32 EMI_CONI_value;
    kal_uint32 EMI_CONJ_value;
    kal_uint32 EMI_CONK_value;
    kal_uint32 EMI_CONL_value;
    kal_uint32 EMI_CONN_value;
 
    kal_uint32 EMI_DUTA_value;
    kal_uint32 EMI_DUTB_value;
    kal_uint32 EMI_DUTC_value;
 
    kal_uint32 EMI_DUCA_value;
    kal_uint32 EMI_DUCB_value;
    kal_uint32 EMI_DUCE_value;
 
    kal_uint32 EMI_IOCL_value;

    kal_uint32 EMI_GEND_value;

    int   DRAM_RANK_SIZE[4];    
    kal_uint32 DRAM_ID;     
    int   match_flag;
} EMI_SETTINGS;

typedef struct
{
 	kal_uint32 dram_type; 
 	kal_uint32 vendor_id; 
    int      rank0_size;  	
    int      rank1_size;  	
} DRAM_INFO;

int mt_get_dram_type (void); 
/* 0: invalid */
/* 1: mDDR1 */
/* 2: mDDR2 */
/* 3: mDDR3 */

/* SRAM repair HW module*/
typedef enum
{
	MFG_MMSYS = 0,
	MDSYS,
	HSPA,
	TDDSYS	
}REPAIR_MODULE;

int SRAM_repair(REPAIR_MODULE module);

#endif
