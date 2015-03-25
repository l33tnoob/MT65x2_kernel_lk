/*
 * Copyright (C) 2011 MediaTek, Inc.
 *
 * Author: Pupa Chen <pupa.chen@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MT_FREQHOPPING_H__
#define __MT_FREQHOPPING_H__

#include "mach/mt_typedefs.h"
#include "mach/mt_fhreg.h"

#define MT_FREQHOP_DEFAULT_ON

#ifdef __MT_FREQHOPPING_C__
#define FREQHOP_EXTERN
#else
#define FREQHOP_EXTERN extern
#endif

#define FHCTL_SUCCESS		0
#define FHCTL_FAIL			1

#define FHDMA_MODE_IDENTICAL	0
#define FHDMA_MODE_SEPARATED	1

#define FHCTL_SM				(1 << 0)
#define FHCTL_SSC				(1 << 1)
#define FHCTL_HOPPING			(1 << 2)
#define FHCTL_MODE_MASK			(0x7)

#define FHCTL_SR_LSB			0
#define FHCTL_SR_MSB			1

#define FHCTL_NO_PAUSE			0
#define FHCTL_PAUSE				1

#define FHCTLx_EN				0
#define FRDDSx_EN				1 //free-run
#define SFSTRx_EN				2 //soft-start
#define SFSTRx_BP				4
#define FHCTLx_SRHMODE			5
#define FHCTLx_PAUSE			8
#define FRDDSx_DTS				16
#define FRDDSx_DYS				20 

#define SWITCH_FHCTL2PLLCON	0
#define SWITCH_PLLCON2FHCTL	1

#define FREQHOP_PLLID2SRAMOFFSET(pLLID)		(pLLID * SRAM_TABLE_SIZE_BY_PLL)

//- HAL porting start
#define FHTAG "[FH]"

#define VERBOSE_DEBUG 0

#if VERBOSE_DEBUG
#define FH_MSG(fmt, args...) \
do {    \
		printk( FHTAG""fmt" <- %s(): L<%d>  PID<%s><%d>\n", \
            	##args ,__FUNCTION__,__LINE__,current->comm,current->pid); \
} while(0);
#else

#if 1 //log level is 6 xlog
#define FH_MSG(fmt, args...) \
do {    \
		xlog_printk(ANDROID_LOG_DEBUG, FHTAG, fmt, \
            	##args ); \
} while(0);
#else //log level is 4 (printk)
#define FH_MSG(fmt, args...) \
do {    \
		printk( FHTAG""fmt" \n", \
            	##args ); \
} while(0);
#endif

#endif

//keep track the status of each PLL 
//TODO: do we need another "uint mode" for Dynamic FH
typedef struct{
	unsigned int	fh_status;
	unsigned int	pll_status;
	unsigned int	setting_id;
	unsigned int	curr_freq;
	unsigned int	user_defined;
}fh_pll_t;

struct freqhopping_ssc {
	unsigned int	 freq;
	unsigned int	 dt;
	unsigned int	 df;
	unsigned int	 upbnd;
	unsigned int 	 lowbnd;
	unsigned int	 dds;
};

struct freqhopping_ioctl {
	unsigned int  pll_id;
	struct freqhopping_ssc ssc_setting; //used only when user-define
	int  result;
};

enum FH_CMD{
 FH_CMD_ENABLE = 1,
 FH_CMD_DISABLE,
 FH_CMD_ENABLE_USR_DEFINED,
 FH_CMD_DISABLE_USR_DEFINED,
 FH_CMD_INTERNAL_MAX_CMD,
/* TODO:  do we need these cmds ?
 FH_CMD_PLL_ENABLE,
 FH_CMD_PLL_DISABLE,
 FH_CMD_EXT_ALL_FULL_RANGE_CMD,
 FH_CMD_EXT_ALL_HALF_RANGE_CMD,
 FH_CMD_EXT_DISABLE_ALL_CMD,
 FH_CMD_EXT_DESIGNATED_PLL_FULL_RANGE_CMD,
 FH_CMD_EXT_DESIGNATED_PLL_AND_SETTING_CMD
*/ 
};

enum FH_PLL_ID {
 MT658X_FH_MINIMUMM_PLL = 0,		
// MT658X_FH_ARM_PLL	= MT658X_FH_MINIMUMM_PLL,
// MT658X_FH_MAIN_PLL	= 1,
 MT658X_FH_MEM_PLL	= 2,
 MT658X_FH_MSDC_PLL	= 3,
 MT658X_FH_MM_PLL	= 4, //MT658X_FH_TVD_PLL	= 4,
 MT658X_FH_VENC_PLL	= 5, //MT658X_FH_LVDS_PLL	= 5,
 MT658X_FH_MAXIMUMM_PLL = MT658X_FH_VENC_PLL,
 MT658X_FH_PLL_TOTAL_NUM
};

enum FH_FH_STATUS{
 FH_FH_DISABLE = 0,
 FH_FH_ENABLE_SSC,	
 FH_FH_ENABLE_DFH,
 FH_FH_ENABLE_DVFS,
};

enum FH_PLL_STATUS{
 FH_PLL_DISABLE = 0,
 FH_PLL_ENABLE = 1
};

void mt_freqhopping_pll_init(void);
//- HAL porting end

//- PLL Index
enum FHCTL_PLL_ID
{
    ARMPLL_ID		= 0,
    MAINPLL_ID		= 1,    
    NUM_OF_PLL_ID,
    
    MT658X_FH_ARM_PLL	= ARMPLL_ID,
    MT658X_FH_MAIN_PLL	= MAINPLL_ID,
};

//- default on section

#define FHCTL_ARMPLL_SSC_ON			(1 << ARMPLL_ID)
#define FHCTL_MAINPLL_SSC_ON		(1 << MAINPLL_ID)

enum FHCTL_RF_ID
{
	RF_2G1_ID = 0,
	RF_2G2_ID,
	RF_INTGMD_ID,
	RF_EXTMD_ID,
	RF_BT_ID,
	RF_WF_ID,
	RF_FM_ID,
	NUM_OF_RF_ID
};

#define WORD_L	0
#define WORD_H	1
typedef struct
{
	U64		map64[NUM_OF_PLL_ID];
}FHCTL_dds_dram;


//-Registers Setting
typedef struct 
{
	UINT32	target_vco_freq;
	UINT32	dt;
	UINT32	df;
	UINT32	uplimit_percent_10;
	UINT32	downlimit_percent_10;
	UINT32	uplimit;
	UINT32	downlimit;
	UINT32	dds_val;	
} FREQHOP_PLLSettings;
#define FREQHOP_PLLSETTINGS_MAXNUMBER 10

typedef struct
{
	char	name[32];
	UINT32	addr;
} FREQHOP_REG_MAP;


#ifdef __FHCTL_CTP__
FREQHOP_EXTERN INT32 freqhop_init(UINT32 option);
FREQHOP_EXTERN const char rfid_to_rfname[NUM_OF_RF_ID][4];
FREQHOP_EXTERN const char pllid_to_pllname[NUM_OF_PLL_ID][8];
FREQHOP_EXTERN FREQHOP_PLLSettings freqhop_pll_settings[NUM_OF_PLL_ID][FREQHOP_PLLSETTINGS_MAXNUMBER];

FREQHOP_EXTERN void freqhop_delay_task(UINT32 time_in_ticks);
FREQHOP_EXTERN void freqhop_sram_blkcpy(UINT32 pll_id, UINT32 *pDDS);
FREQHOP_EXTERN void freqhop_rf_src_hopping_enable(UINT32 rf_id, UINT32 enable);
FREQHOP_EXTERN void freqhop_set_priority(UINT32 order, UINT32 order_md);
FREQHOP_EXTERN void freqhop_set_dma_mode(UINT32 mode);
FREQHOP_EXTERN void freqhop_fhctlx_set_DVFS(UINT32 pll_id, UINT32 dds);
FREQHOP_EXTERN void freqhop_rf_src_trigger_ch(UINT32 rf_id, UINT32 channel);
FREQHOP_EXTERN UINT32 freqhop_get_pll_mon_dss(UINT32 pll_id);
FREQHOP_EXTERN UINT32 freqhop_get_pll_fhctlx_dss(UINT32 pll_id);
FREQHOP_EXTERN void freqhop_sram_init(UINT32 *pDDS);
FREQHOP_EXTERN void freqhop_sram_blkcpy(UINT32 pll_id, UINT32 *pDDS);
FREQHOP_EXTERN void freqhop_setbit_FHCTLx_cfg(UINT32 pll_id, UINT32 field, UINT32 mode);
FREQHOP_EXTERN void freqhop_set_fhctlx_updnlmt(UINT32 pll_id, UINT32 uplimit, UINT32 downlimit);
FREQHOP_EXTERN void freqhop_set_fhctlx_slope(UINT32 pll_id, UINT32 dts, UINT32 dys);
#else	//- __FHCTL_CTP__
FREQHOP_EXTERN void mt_freqhop_init(void);
FREQHOP_EXTERN int freqhop_config(unsigned int pll_id, unsigned long vco_freq, unsigned int enable);
FREQHOP_EXTERN void mt_freqhop_popod_save(void);
FREQHOP_EXTERN void mt_freqhop_popod_restore(void);
FREQHOP_EXTERN void mt_fh_query_SSC_boundary (UINT32 pll_id, UINT32* uplmt_10, UINT32* dnlmt_10);
#endif //- !__FHCTL_CTP__


#endif/* !__MT_FREQHOPPING_H__ */

