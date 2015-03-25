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
#ifndef __MT_DCM_C__
#define __MT_DCM_C__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#include <mach/mt_typedefs.h>
#include <mach/sync_write.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>

#include <linux/timer.h>

//#define LPM_MET_ENABLE

#if defined (LPM_MET_ENABLE)
#include "linux/met_drv.h"
extern int met_ext_dev_add(struct metdevice *metdev);
extern int met_ext_dev_del(struct metdevice *metdev);
#endif //#if defined (LPM_MET_ENABLE)

#undef CONFIG_MTK_MET	//- skip MET porting
#ifdef CONFIG_MTK_MET
#include <linux/met_drv.h>
#endif
///////////////////////////////////////////////////////////
// Local Defination
///////////////////////////////////////////////////////////
#ifdef CONFIG_MTK_MET
#define LPM_MET_ENABLE
#endif

#define USING_XLOG

#ifdef __DCM_CTP__
	#define dcm_err		must_print
	#define dcm_warn	must_print
	#define dcm_info    must_print
	#define dcm_dbg     must_print
	#define dcm_ver     must_print

	#define dcm_read_reg(aDDR)			READ_REG(aDDR)
	#define dcm_write_reg(vAL, aDDR)	WRITE_REG(vAL, aDDR)

#else

	#ifdef USING_XLOG

	#include <linux/xlog.h>
	#define TAG     "Power/dcm"

	#define dcm_err(fmt, args...)       \
	    xlog_printk(ANDROID_LOG_ERROR, TAG, fmt, ##args)
	#define dcm_warn(fmt, args...)      \
	    xlog_printk(ANDROID_LOG_WARN, TAG, fmt, ##args)
	#define dcm_info(fmt, args...)      \
	    xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)
	#define dcm_dbg(fmt, args...)       \
	    xlog_printk(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
	#define dcm_ver(fmt, args...)       \
	    xlog_printk(ANDROID_LOG_VERBOSE, TAG, fmt, ##args)

	#else /* !USING_XLOG */

	#define TAG     "[Power/dcm] "

	#define dcm_err(fmt, args...)       \
	    printk(KERN_ERR TAG);           \
	    printk(KERN_CONT fmt, ##args)
	#define dcm_warn(fmt, args...)      \
	    printk(KERN_WARNING TAG);       \
	    printk(KERN_CONT fmt, ##args)
	#define dcm_info(fmt, args...)      \
	    printk(KERN_NOTICE TAG);        \
	    printk(KERN_CONT fmt, ##args)
	#define dcm_dbg(fmt, args...)       \
	    printk(KERN_INFO TAG);          \
	    printk(KERN_CONT fmt, ##args)
	#define dcm_ver(fmt, args...)       \
	    printk(KERN_DEBUG TAG);         \
	    printk(KERN_CONT fmt, ##args)

	#endif //- USING_XLOG

	#define dcm_read_reg(aDDR)						DRV_Reg32(aDDR)
	#define dcm_write_reg(vAL, aDDR)				DRV_WriteReg32(aDDR, vAL)	
#endif //- __DCM_CTP__



#define dcm_clr_field(vAL, rG)					vAL &= (~((rG##_MASK) << (rG##_BIT)));
#define dcm_set_field(vAL, wVAL, rG)			vAL |= ( ((wVAL) & (rG##_MASK)) << (rG##_BIT) )
#define dcm_clr_and_set_field(vAL, wVAL, rG)	dcm_clr_field(vAL, rG);dcm_set_field(vAL, wVAL, rG)

typedef void (*AP_DCM_Handler)(UINT32 option, void* ctrl);

#define AP_DCM_ALL	((1 << NUM_OF_AP_DCM_TYPE) - 1)

#define __REGXfer(tOKEN)	{.name = #tOKEN, .addr = tOKEN}

#define GPIODBGOUT_CHANGE	(1 << 31)
#define GPIODBGOUT_BIT		(24)
#define GPIODBGOUT_MASK		(0x7F)
#define GPIODBGOUT_LPD		(1 << 0)
#define GPIODBGOUT_BPI		(1 << 1)

///////////////////////////////////////////////////////////
// Local variable
///////////////////////////////////////////////////////////
const char AP_dcm_name[NUM_OF_AP_DCM_TYPE][16] = {	"AP_DCM_ARMCORE",
													"AP_DCM_ARML2BUS",
													"AP_DCM_TOPBUS",
													"AP_DCM_EMI",
													"AP_DCM_INFRA",
													"AP_DCM_PERI",
													"AP_DCM_PMIC",
													"AP_DCM_USB",
												 };
const char MM_dcm_name[NUM_OF_MM_DCM_TYPE][32] = {
													"MM_DCM_SMI_COMMON",
													"MM_DCM_SMI_LARB0",
													"MM_DCM_MM_CMDQ",
													"MM_DCM_MUTEX",
													"MM_DCM_DISP_COLOR",
													"MM_DCM_DISP_BLS",
													"MM_DCM_DISP_WDMA",
													"MM_DCM_DISP_RDMA",
													"MM_DCM_DISP_OVL",
													"MM_DCM_MDP_TDSHP",
													"MM_DCM_MDP_WROT",
													"MM_DCM_MDP_WDMA",
													"MM_DCM_MDP_RSZ1",
													"MM_DCM_MDP_RSZ0",
													"MM_DCM_MDP_RDMA",
													"MM_DCM_DSI_ENGINE",
													"MM_DCM_DISP_DBI_ENGINE",
													"MM_DCM_DISP_DBI_SMI",
													"MM_DCM_DSI_DIGITAL",
													"MM_DCM_VENC",
													"MM_DCM_VDEC",
													"MM_DCM_CAM",
												 };
const char MFG_dcm_name[NUM_OF_MFG_DCM_TYPE][16] = {"MFG_DCM_BG3D"};


const char LPM_getname[NUM_OF_LPM_CNT_TYPE][16] = {	"total_time",
													"low_to_high",
													"total_high",
													"longest_high",
													"good_duration"
												  };

const char EM_dcm_name[NUM_OF_EM_DCM_TYPE][16]	= {
													"ARM_DCM",
													"EMI_DCM",
													"INFRA_DCM",
													"PERI_DCM",
													"MISC_DCM",
													"MM_DCM",
													"MFG_DCM"
												   };


const DCM_REG_MAP dcm_reg_map[NUM_OF_DCM_REGS]	= {
													__REGXfer(DCM_TOP_CKDIV1				),
													__REGXfer(TOP_DCMCTL                ),
													__REGXfer(TOP_DCMDBC                ),

													__REGXfer(ACLKEN_DIV                ),
													__REGXfer(CA7_MISC_CONFIG           ),
													__REGXfer(CA7_CACHE_CONFIG          ),
 
													__REGXfer(TOPBUS_DCMCTL             ),
													__REGXfer(TOPEMI_DCMCTL             ),

													__REGXfer(DCM_INFRABUS_DCMCTL0          ),
													__REGXfer(DCM_INFRABUS_DCMCTL1          ),

													__REGXfer(MMSYS_HW_DCM_DIS0         ),
													__REGXfer(MMSYS_HW_DCM_DIS1         ),

													__REGXfer(MMSYS_VENC_MP4_DCM_CTRL   ),

													__REGXfer(MMSYS_VDEC_DCM_STA        ),
													__REGXfer(MMSYS_VDEC_DCM_DIS        ),

													__REGXfer(MMSYS_CAM_DCM_DIS         ),
													__REGXfer(MMSYS_CAM_DCM_STATUS      ),

													__REGXfer(MFG_DCM_CON_0             ),
												};

const char LPM_source_name[NUM_OF_LPM_SOURCE][32] =	{	"CONN_SRCCLKENA_ACK",
														"CONN_SRCCLKENA",
														"MD_SRCCLKENA",
														"MD_SRCCLKENA_ACK",
														"CLKSQ_OFF",
														"INFRASYS_TOP_MTCMOS_POWER_ON",
														"CA7_CORE0_MTCMOS_POWER_ON",
														"CA7_CORE1_MTCMOS_POWER_ON",
														"DCM_ALL_IDLE",
														"MDSYS_BUS_PORT_IDLE",
														"EMI_CONTROLLER_IDLE",
														"EMI_BUS_SLAVE_PORT_IDLE",
														"INFRASYS_BUS_IDLE",
														"PERISYS_BUS_IDLE",
														"CA7_CORE0_STANDBYWFI",
														"CA7_CORE1_STANDBYWFI"};

const char FQMTR_source_name[NUM_OF_FQMTR_SOURCE][32] = {	"RESERVED",
															"MAINPLL_DIV8",
															"MAINPLL_DIV12",
															"MAINPLL_DIV24",
															"MAINPLL_DIV20",
															"MAINPLL_DIV7",
															"UNIVPLL_DIV16",
															"UNIVPLL_DIV24",
															"UNIVPLL_DIV20",
															"WHPLL_OUTPUT_CLOCK",
															"WPLL_OUTPUT_CLOCK",
															"SYS_26MHZ_CLOCK",
															"USB_48MHZ_CLOCK",
															"EMI2X_CLOCK",
															"AP_INFRA_FAST_BUS_CLOCK",
															"SMI_CLOCK_OF_MMSYS",
															"UART0_CLOCK",
															"GPU_CLOCK",
															"MSDC1_CLOCK",
															"CAM_SENINF_CLOCK",
															"PWM_OF_MMSYS_CLOCK",
															"SPI_NFI_CLOCK",
															"DBI_CLOCK",
															"PMIC_SPI_CLOCK",
															"AP_PLLGP_TST_CK ",
															"APMCU_CLOCK",
															"RTC_32KHZ_CLOCK"};

const char LPM_mode_string[3][16]	= {"Off", "One-shot", "Periodic"};
const char LPM_onoff_string[2]	= {'X', 'V'};

static UINT32	dcm_initiated = 0;
static UINT32	dcm_pmic_getinit = 1;
static UINT32	dcm_usb_getinit = 1;
static UINT32	fm_last_owner	= 0;
static UINT32	fm_magic_owner	= 1;
static char		fm_kick_last_caller[64];
static char		fm_getresult_last_caller[64];

static UINT32	lpm_last_owner	= 0;
static UINT32	lpm_magic_owner	= 1;
static UINT32	lpm_cat_type	= 1;
static char		lpm_kick_last_caller[64];
static char		lpm_getresult_last_caller[64];

static UINT32	gpiodbg_sel = GPIODBGOUT_LPD;

//-#if (defined(MT_APDCM_DEFAULT_ON) && defined(CONFIG_MTK_LDVT))
#if (defined(MT_APDCM_DEFAULT_ON))
static AP_DCM_ARMCORE_CTRL		ap_dcm_armcore_ctrl_default		= {ARM_CORE_DCM_MODE1};
static AP_DCM_ARML2BUS_CTRL		ap_dcm_arml2bus_ctrl_default	= {AXI_DIV_SEL_D2, 1, 1};
static AP_DCM_TOPBUS_CTRL		ap_dcm_topbus_ctrl_default		= {0, 0, 0} ;
static AP_DCM_EMI_CTRL			ap_dcm_emi_ctrl_default   		= {0, 0, 1, 1, 15, RG_EMIDCM_FSEL_DIV1, RG_EMIDCM_SFSEL_DIV16};
static AP_DCM_INFRA_CTRL		ap_dcm_infra_ctrl_default    	= {0, 0, 0, 1, 1, RG_INFRADCM_SFSEL_DIV32, RG_INFRADCM_FSEL_DIV1, 1, 7};
static AP_DCM_PERI_CTRL			ap_dcm_peri_ctrl_default		= {0, 0, 0, 1, 1, RG_PERIDCM_SFSEL_DIV32, RG_PERIDCM_FSEL_DIV1, 1, 7};
static AP_DCM_PMIC_CTRL			ap_dcm_pmic_ctrl_default		= {0, 0, RG_PMIC_SFSEL_DIV32};
static AP_DCM_USB_CTRL			ap_dcm_usb_ctrl_default 		= {0};
//static UINT32					mm_dcm_ctrl_default 			= 0x3FFFFF;
//static MFG_DCM_BG3D_CTRL		mfg_dcm_ctrl_default			= {1, 1, BG3D_FSEL_DIV64, 0x3F};
#else
static AP_DCM_ARMCORE_CTRL		ap_dcm_armcore_ctrl_default		= {ARM_CORE_DCM_DISABLE};
static AP_DCM_ARML2BUS_CTRL		ap_dcm_arml2bus_ctrl_default	= {AXI_DIV_SEL_D2, 0, 0};
static AP_DCM_TOPBUS_CTRL		ap_dcm_topbus_ctrl_default		= {0, 0, 0} ;
static AP_DCM_EMI_CTRL			ap_dcm_emi_ctrl_default   		= {0, 0, 0, 1, 15, RG_EMIDCM_FSEL_DIV1, RG_EMIDCM_SFSEL_DIV16};
static AP_DCM_INFRA_CTRL		ap_dcm_infra_ctrl_default    	= {0, 0, 0, 0, 0, RG_INFRADCM_SFSEL_DIV32, RG_INFRADCM_FSEL_DIV1, 1, 7};
static AP_DCM_PERI_CTRL			ap_dcm_peri_ctrl_default		= {0, 0, 0, 0, 0, RG_PERIDCM_SFSEL_DIV32, RG_PERIDCM_FSEL_DIV1, 1, 7};
static AP_DCM_PMIC_CTRL			ap_dcm_pmic_ctrl_default		= {0, 0, RG_PMIC_SFSEL_DIV32};
static AP_DCM_USB_CTRL			ap_dcm_usb_ctrl_default 		= {0};
//static UINT32					mm_dcm_ctrl_default 			= 0x3FFFFF;
//static MFG_DCM_BG3D_CTRL		mfg_dcm_ctrl_default			= {1, 1, BG3D_FSEL_DIV64, 0x3F};
#endif




static AP_DCM_ARMCORE_CTRL		ap_dcm_armcore_ctrl		= {ARM_CORE_DCM_DISABLE};
static AP_DCM_ARML2BUS_CTRL		ap_dcm_arml2bus_ctrl	= {AXI_DIV_SEL_D2, 0, 0};
static AP_DCM_TOPBUS_CTRL		ap_dcm_topbus_ctrl		= {0, 0, 0} ;
static AP_DCM_EMI_CTRL			ap_dcm_emi_ctrl   		= {0, 0, 0, 0, 0, 0, 0};
static AP_DCM_INFRA_CTRL		ap_dcm_infra_ctrl    	= {0, 0, 0, 0, 0, 0, 0, 0, 0};
static AP_DCM_PERI_CTRL			ap_dcm_peri_ctrl		= {0, 0, 0, 0, 0, 0, 0, 0, 0};
static AP_DCM_PMIC_CTRL			ap_dcm_pmic_ctrl		= {0, 0, RG_PMIC_SFSEL_DIV32};
static AP_DCM_USB_CTRL			ap_dcm_usb_ctrl 		= {0};
static UINT32					mm_dcm_ctrl 			= 0x3FFFFF;
//static MFG_DCM_BG3D_CTRL		mfg_dcm_ctrl			= {1, 1, BG3D_FSEL_DIV64, 0x3F};

UINT32* ap_dcm_default_setting[NUM_OF_AP_DCM_TYPE] = {(UINT32*)&ap_dcm_armcore_ctrl_default, (UINT32*)&ap_dcm_arml2bus_ctrl_default, (UINT32*)&ap_dcm_topbus_ctrl_default, (UINT32*)&ap_dcm_emi_ctrl_default,
														(UINT32*)&ap_dcm_infra_ctrl_default, (UINT32*)&ap_dcm_peri_ctrl_default, (UINT32*)&ap_dcm_pmic_ctrl_default, (UINT32*)&ap_dcm_usb_ctrl_default};

static UINT32	lpm_cnt_mask[NUM_OF_LPM_CNT_TYPE] = {RG_TOTAL_TIME_MASK, RG_LOW2HIGH_COUNT_MASK, RG_HIGH_DUR_TIME_MASK, RG_LONGEST_HIGHDUR_TIME_MASK, RG_GOOD_DUR_COUNT_MASK};

static UINT32	em_dcm_mode = 0;
static UINT32	em_dcm_en[NUM_OF_EM_DCM_TYPE] = {0, 0, 0, 0, 0, 0, 0};
static UINT32	em_dcm_sta[NUM_OF_EM_DCM_TYPE] = {0, 0, 0, 0, 0, 0, 0};
static UINT32	em_fqmtr_sta = 0;
static FREQMETER_CTRL em_fqmtr_request;
static UINT32	em_lpm_sta = 0;
static LPM_CTRL em_lpm_request;

#ifndef __DCM_CTP__
static DEFINE_SPINLOCK(dcm_spin_lock);
static DEFINE_SPINLOCK(lpm_spin_lock);
static DEFINE_SPINLOCK(freqmeter_spin_lock);
#define DCM_LOCK			spin_lock(&dcm_spin_lock);
#define DCM_UNLOCK			spin_unlock(&dcm_spin_lock);
#define LPM_LOCK(fLAG)			spin_lock_irqsave(&lpm_spin_lock, fLAG);
#define LPM_UNLOCK(fLAG)		spin_unlock_irqrestore(&lpm_spin_lock, fLAG);
#define FREQM_LOCK				spin_lock(&freqmeter_spin_lock);
#define FREQM_UNLOCK			spin_unlock(&freqmeter_spin_lock);
#else	//- __DCM_CTP__
#define DCM_LOCK
#define DCM_UNLOCK
#define LPM_LOCK(fLAG)
#define LPM_UNLOCK(fLAG)
#define FREQM_LOCK
#define FREQM_UNLOCK
#endif

#ifndef __DCM_CTP__
static struct timer_list lpm_timer;
typedef struct{
	UINT32		init;
	UINT32		mode;
	UINT32		threshold_check;
	UINT32		threshold[NUM_OF_LPM_CNT_TYPE][2];
	UINT32		sta_max[NUM_OF_LPM_CNT_TYPE];
	UINT32		sta_min[NUM_OF_LPM_CNT_TYPE];
	UINT32		sta_overflow[NUM_OF_LPM_CNT_TYPE];
    LPM_CTRL	ctrl;
	UINT32		period_in_s;
	UINT32		log_mode;
}LPM_TIMER_DATA;
static LPM_TIMER_DATA lpm_timer_data;

#ifdef LPM_MET_ENABLE
struct metdevice met_lpm_device[];
typedef struct
{
	UINT32		init;
	LPM_CTRL	ctrl;
} MET_LPM_DATA;
static MET_LPM_DATA met_lpm_ctrl;
#endif //- LPM_MET_ENABLE
#endif //- !__DCM_CTP__
///////////////////////////////////////////////////////////
// Local function
///////////////////////////////////////////////////////////
static void ap_dcm_armcore (UINT32 option, void* ctrl);
static void ap_dcm_arml2bus (UINT32 option, void* ctrl);
static void ap_dcm_topbus (UINT32 option, void* ctrl);
static void ap_dcm_emi (UINT32 option, void* ctrl);
static void ap_dcm_infra (UINT32 option, void* ctrl);
static void ap_dcm_peri (UINT32 option, void* ctrl);
static void ap_dcm_pmic (UINT32 option, void* ctrl);
static void ap_dcm_usb (UINT32 option, void* ctrl);
static void mfg_dcm_enable (UINT32 enable);

static AP_DCM_Handler ap_dcm_handler[NUM_OF_AP_DCM_TYPE] = {	ap_dcm_armcore,
																ap_dcm_arml2bus,
																ap_dcm_topbus,
																ap_dcm_emi,
																ap_dcm_infra,
																ap_dcm_peri,
																ap_dcm_pmic,
																ap_dcm_usb,
															};

#ifndef __DCM_CTP__
static void mt_fqmtr_init (void);
static void mt_lpm_init (void);
static void lpm_timer_callback(unsigned long param);
#endif //- !__DCM_CTP__



/////////////////////////////////////////
//- dcm
UINT32 dcm_init(UINT32 option)
{
	INT32	res = AP_DCM_SUCCESS;
	UINT32	i, reg;

	if (dcm_initiated)
	{
		return AP_DCM_SUCCESS;
	}

	dcm_initiated = 1;
	
	{	//- AP MISC DCM
		reg = dcm_read_reg (DCM_INFRABUS_DCMCTL1);
		if (dcm_pmic_getinit)
		{	
			ap_dcm_pmic_ctrl_default.rg_pmic_bclkdcm_en		= (reg >> RG_PMIC_BCLKDCM_EN_BIT) & RG_PMIC_BCLKDCM_EN_MASK;
			ap_dcm_pmic_ctrl_default.rg_pmic_spiclkdcm_en	= (reg >> RG_PMIC_SPICLKDCM_EN_BIT) & RG_PMIC_SPICLKDCM_EN_MASK;
			ap_dcm_pmic_ctrl_default.rg_pmic_sfsel			= (reg >> RG_PMIC_SFSEL_BIT) & RG_PMIC_SFSEL_MASK;
			
			ap_dcm_pmic_ctrl.rg_pmic_bclkdcm_en				= (reg >> RG_PMIC_BCLKDCM_EN_BIT) & RG_PMIC_BCLKDCM_EN_MASK;
			ap_dcm_pmic_ctrl.rg_pmic_spiclkdcm_en			= (reg >> RG_PMIC_SPICLKDCM_EN_BIT) & RG_PMIC_SPICLKDCM_EN_MASK;
			ap_dcm_pmic_ctrl.rg_pmic_sfsel					= (reg >> RG_PMIC_SFSEL_BIT) & RG_PMIC_SFSEL_MASK;			
		}
		
		if (dcm_usb_getinit)
		{
			ap_dcm_usb_ctrl_default.rg_usbdcm_en	= (reg >> RG_USBDCM_EN_BIT) & RG_USBDCM_EN_MASK;
			ap_dcm_usb_ctrl.rg_usbdcm_en 			= (reg >> RG_USBDCM_EN_BIT) & RG_USBDCM_EN_MASK;
		}
	
		em_dcm_sta[EM_MISC_DCM] &= (~0xFF);
		em_dcm_sta[EM_MISC_DCM] |= (reg & 0xFF);
	
		em_dcm_en[EM_MISC_DCM] &= (~0x3);
		em_dcm_en[EM_MISC_DCM] |= ((ap_dcm_pmic_ctrl.rg_pmic_spiclkdcm_en << 1) | (ap_dcm_usb_ctrl.rg_usbdcm_en << 0));
	}
	
	for (i=0; i<NUM_OF_AP_DCM_TYPE; i++)
	{
		if (option & (1 << i))
		{
			if (AP_DCM_SUCCESS != ap_dcm_config (i, (void*)(ap_dcm_default_setting[i])))
			{
				dcm_err("[Err]=> %s in %s()\n", AP_dcm_name[i], __func__);
				res = AP_DCM_FAIL;
			}
		}
	}

	//- update ctrl only, don't overwite reg
	em_dcm_sta[EM_MM_DCM] = mm_dcm_ctrl;
	em_dcm_en[EM_MM_DCM] = mm_dcm_ctrl;


	//- update ctrl only, don't overwite reg
	reg = dcm_read_reg (MFG_DCM_CON_0);
	em_dcm_sta[EM_MFG_DCM] = reg;
	em_dcm_en[EM_MFG_DCM]  = 1;
	
	return res;
}

INT32 ap_dcm_config (UINT32 type, void* ctrl)
{
	if (ap_dcm_handler[type])
	{
		ap_dcm_handler[type](AP_DCM_OP_CONFIG, ctrl);
		
		return AP_DCM_SUCCESS;
	}

	return AP_DCM_FAIL;
}

INT32 ap_dcm_enable(UINT32 mask)
{
	
	UINT32 i;
	void   *ctrl = 0;
	
	for (i=0; i<NUM_OF_AP_DCM_TYPE; i++)
	{
		if (mask & (1 << i))
		{

			ap_dcm_handler[i](AP_DCM_OP_ENABLE, ctrl);			
		}		
	}
	
	return AP_DCM_SUCCESS;
}

INT32 ap_dcm_disable(UINT32 mask)
{
	UINT32 i;
	void   *ctrl = NULL;
	INT32  status = AP_DCM_SUCCESS;
	
	for (i=0; i<NUM_OF_AP_DCM_TYPE; i++)
	{
		if (mask & (1 << i))
		{
			ap_dcm_handler[i](AP_DCM_OP_DISABLE, ctrl);			
		}		
	}
	
	return status;
}

INT32 dcm_enable_usb (UINT32 enable)
{
	AP_DCM_USB_CTRL ctrl;

	ctrl.rg_usbdcm_en = enable;
	ap_dcm_usb (AP_DCM_OP_CONFIG, (void*) &ctrl);

	return 0;
}

INT32 dcm_enable_pmic (UINT32 sfsel, UINT32 spi_dcm_en, UINT32 bclk_dcm_en)
{	
	AP_DCM_PMIC_CTRL ctrl;
	
	ctrl.rg_pmic_sfsel			= sfsel;
	ctrl.rg_pmic_spiclkdcm_en	= spi_dcm_en;
	ctrl.rg_pmic_bclkdcm_en		= bclk_dcm_en;
	ap_dcm_pmic (AP_DCM_OP_CONFIG, (void*) &ctrl);

	return 0;
}

static void ap_dcm_armcore (UINT32 option, void* ctrl)
{
	UINT32 reg;

	DCM_LOCK;

	switch (option)
	{
		case AP_DCM_OP_DISABLE:
			ap_dcm_armcore_ctrl.mode = ARM_CORE_DCM_DISABLE;
			break;
		case AP_DCM_OP_ENABLE:
			ap_dcm_armcore_ctrl.mode = ARM_CORE_DCM_MODE1;
			break;
		case AP_DCM_OP_CONFIG:
			memcpy((void*)&ap_dcm_armcore_ctrl, ctrl, sizeof (AP_DCM_ARMCORE_CTRL));
			break;
		default:
			return; //fixme. need unlock.
			break;
	}


	/* *TOP_DCMDBC = aor(*TOP_DCMDBC, ~(1<<0), 
	  (ap_dcm_armcore_ctrl.mode == ARM_CORE_DCM_MODE1) ? 1 : 0);*/
	reg = dcm_read_reg (TOP_DCMDBC);
	dcm_clr_field (reg, TOPCKGEN_DCM_DBC_CNT);
	if (ap_dcm_armcore_ctrl.mode == ARM_CORE_DCM_MODE1)
	{
		dcm_set_field (reg, 1, TOPCKGEN_DCM_DBC_CNT);
	}
	dcm_write_reg (reg, TOP_DCMDBC);

	/* *TOP_DCMCTL = aor(*TOP_DCMCTL, ~(0x3<<1), 
	 (ap_dcm_armcore_ctrl.mode == ARM_CORE_DCM_MODE2)? 0x3<<1 : 0<<1); */
	reg = dcm_read_reg (TOP_DCMCTL);
	dcm_clr_field (reg, ARM_DCM_WFE_ENABLE);
	dcm_clr_field (reg, ARM_DCM_WFI_ENABLE);
	if (ap_dcm_armcore_ctrl.mode == ARM_CORE_DCM_MODE2)
	{
		dcm_set_field (reg, 1, ARM_DCM_WFE_ENABLE);
		dcm_set_field (reg, 1, ARM_DCM_WFI_ENABLE);
	}
	dcm_write_reg (reg, TOP_DCMCTL);
	
	//- update status
	/* em_dcm_sta[EM_ARM_DCM] = aor(em_dcm_sta[EM_ARM_DCM], ~0x3,  ap_dcm_armcore_ctrl.mode); */
	/* em_dcm_en[EM_ARM_DCM] = aor(em_dcm_en[EM_ARM_DCM], ~1, 
	 (ap_dcm_armcore_ctrl.mode != ARM_CORE_DCM_DISABLE) ? 1 : 0*/
	em_dcm_sta[EM_ARM_DCM] &= (~0x3);
	em_dcm_en[EM_ARM_DCM]  &= (~(1 << 0));
	if (ap_dcm_armcore_ctrl.mode != ARM_CORE_DCM_DISABLE)
	{					
		em_dcm_sta[EM_ARM_DCM] |= ap_dcm_armcore_ctrl.mode;
		em_dcm_en[EM_ARM_DCM]  |= (1 << 0);
	}
	
	DCM_UNLOCK;
}

static void ap_dcm_arml2bus (UINT32 option, void* ctrl)
{
	UINT32 reg;
	
	DCM_LOCK;

	switch (option)
	{
		case AP_DCM_OP_DISABLE:
			ap_dcm_arml2bus_ctrl.cg1_en = 0;
			ap_dcm_arml2bus_ctrl.cg2_en = 0;
			break;
		case AP_DCM_OP_ENABLE:
			ap_dcm_arml2bus_ctrl.cg1_en = 1;
			ap_dcm_arml2bus_ctrl.cg2_en = 1;
			break;
		case AP_DCM_OP_CONFIG:
			memcpy((void*)&ap_dcm_arml2bus_ctrl, ctrl, sizeof (AP_DCM_ARML2BUS_CTRL));
			break;
		default:
			return; //fixme
			break;
	}

	switch (ap_dcm_arml2bus_ctrl.cg0_div)
	{
		case AXI_DIV_SEL_D1:
	    case AXI_DIV_SEL_D2:
	   	case AXI_DIV_SEL_D3:
        case AXI_DIV_SEL_D4:
		case AXI_DIV_SEL_D5:
			/* *ACLKEN_DIV = aor(*ACLKEN_DIV, ~0x1f<<0,  ap_dcm_arml2bus_ctrl.cg0_div); */
			reg = dcm_read_reg (ACLKEN_DIV);
			dcm_clr_and_set_field (reg, ap_dcm_arml2bus_ctrl.cg0_div, AXI_DIV_SEL);
			dcm_write_reg (reg, ACLKEN_DIV);
		break;
		default:
		break;
	}

	/* *CA7_MISC_CONFIG = aor(*CA7_MISC_CONFIG, ~(1<<9),  ap_dcm_arml2bus_ctrl.cg1_en<<9); */
	reg = dcm_read_reg (CA7_MISC_CONFIG);
	dcm_clr_and_set_field (reg, ap_dcm_arml2bus_ctrl.cg1_en, L2_BUS_DCM_EN);
	dcm_write_reg (reg, CA7_MISC_CONFIG);

	/* *CA7_CACHE_CONFIG = aor(*CA7_CACHE_CONFIG, ~(1<<8), ap_dcm_arml2bus_ctrl.cg2_en<<8); */
	reg = dcm_read_reg (CA7_CACHE_CONFIG);
	dcm_clr_and_set_field (reg, ap_dcm_arml2bus_ctrl.cg2_en, L2C_SRAM_MCU_DCM_EN);
	dcm_write_reg (reg, CA7_CACHE_CONFIG);


	em_dcm_sta[EM_ARM_DCM] &= (~0x1F0300);
	em_dcm_sta[EM_ARM_DCM] |= ((ap_dcm_arml2bus_ctrl.cg1_en << 8) | (ap_dcm_arml2bus_ctrl.cg2_en << 9) | (ap_dcm_arml2bus_ctrl.cg0_div << 16));

	em_dcm_en[EM_ARM_DCM] &= (~(1 << 1));
	em_dcm_en[EM_ARM_DCM] |= (ap_dcm_arml2bus_ctrl.cg1_en << 1);
	
	DCM_UNLOCK;
}

static void ap_dcm_topbus (UINT32 option, void* ctrl)
{
	UINT32 reg_0 = 0;
	UINT32 reg_1 = 0;
	
	DCM_LOCK;
	
	switch (option)
	{
		case AP_DCM_OP_DISABLE:
			ap_dcm_topbus_ctrl.force_off = 1;
			break;
		case AP_DCM_OP_ENABLE:
			ap_dcm_topbus_ctrl.force_on = 1;
			break;
		case AP_DCM_OP_CONFIG:
			memcpy((void*)&ap_dcm_topbus_ctrl, ctrl, sizeof (AP_DCM_TOPBUS_CTRL));
			break;
		default:
			return; //fixme
			break;
	}

	dcm_set_field (reg_0, ap_dcm_topbus_ctrl.force_off, RG_BUSDCM_FORCE_OFF);
	dcm_set_field (reg_0, ap_dcm_topbus_ctrl.force_on, RG_BUSDCM_FORCE_ON);
	dcm_set_field (reg_0, ap_dcm_topbus_ctrl.full_sel, RG_BUSDCM_FULL_SEL);
	dcm_set_field (reg_0, 0x9, RG_BUSDCM_APB_SEL);
	reg_1 = reg_0;
	dcm_set_field (reg_1, 0x1, RG_BUSDCM_APB_TOG);

	dcm_write_reg (reg_0, TOPBUS_DCMCTL);
	dsb();
	dcm_write_reg (reg_1, TOPBUS_DCMCTL); // toggle 0->1 to take effect, and parking at 1.
	dsb();
#if 0
	dcm_write_reg (reg_0, TOPBUS_DCMCTL);
#endif
	
	DCM_UNLOCK;	
}

static void ap_dcm_emi (UINT32 option, void* ctrl)
{	
	UINT32 reg_emi;	
	UINT32 reg_0,reg_refresh;
	AP_DCM_EMI_CTRL	*ctl = (AP_DCM_EMI_CTRL*)ctrl;
	
	DCM_LOCK;
	
	switch (option)
	{
		case AP_DCM_OP_DISABLE:
			{
				//- WHQA_00014411
				//- disable auto refresh
				reg_refresh = dcm_read_reg ((EMI_BASE + 0x68));				
				dcm_write_reg (reg_refresh &0xFFFFFFFD, (EMI_BASE + 0x68));
				udelay (10);
				
				/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, 0x0400FE00, 1<<2);; */
				/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, 0x0400FE00, (1<<2) | (1<<0));; */
				reg_emi = (0x0400FE00 & dcm_read_reg (TOPEMI_DCMCTL)) | 0x04;
				dcm_set_field (reg_emi, 0, RG_EMIDCM_ENABLE);
				dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
				dsb();
				dcm_write_reg ((reg_emi | (RG_EMIDCM_APB_TOG_MASK << RG_EMIDCM_APB_TOG_BIT)), TOPEMI_DCMCTL);
				dsb();
				//dcm_write_reg (reg_emi, TOPEMI_DCMCTL); //astone fixed, part toggle bit at 1 to avoid corruption.

				ap_dcm_emi_ctrl.dcm_en = 0;
				//- re-enable auto refresh
				udelay (10);
				dcm_write_reg (reg_refresh, (EMI_BASE + 0x68));
			}break;
		case AP_DCM_OP_ENABLE:
			{
				/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, 0x0400FE00, (1<<8) | (1<<2));; */
				/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, 0x0400FE00, (1<<8) | (1<<2) | (1<<0)); */
				reg_emi = (0x0400FE00 & dcm_read_reg (TOPEMI_DCMCTL)) | 0x04;
				dcm_set_field (reg_emi, 1, RG_EMIDCM_ENABLE);
				dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
				dsb();
				dcm_write_reg ((reg_emi | (RG_EMIDCM_APB_TOG_MASK << RG_EMIDCM_APB_TOG_BIT)), TOPEMI_DCMCTL);
				dsb();
				//dcm_write_reg (reg_emi, TOPEMI_DCMCTL); //astone, to park toggle bit at 1
				
				ap_dcm_emi_ctrl.dcm_en = 1;
			}break;
		case AP_DCM_OP_CONFIG:
			{
				/* fixme, to avoid to toggle each field separately and not to acount about side effect. */
				if (ap_dcm_emi_ctrl.idle_fsel != ctl->idle_fsel)
				{	
					/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, ~(0x1F<<21), (ctl->idle_fsel<<21) | 0x20);  */
					reg_emi = (0x0400FE00 & dcm_read_reg (TOPEMI_DCMCTL)) | 0x20;
					dcm_set_field (reg_emi, ctl->idle_fsel, RG_EMIDCM_IDLE_FSEL);
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg ((reg_emi | (RG_EMIDCM_APB_TOG_MASK << RG_EMIDCM_APB_TOG_BIT)), TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
				}
				if (ap_dcm_emi_ctrl.full_sel != ctl->full_sel)
				{
					/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, ~(0x1F<<16), (ctl->full_sel<<16) | 0x10);  */
					reg_emi = (0x0400FE00 & dcm_read_reg (TOPEMI_DCMCTL)) | 0x10;
					dcm_set_field (reg_emi, ctl->full_sel, RG_EMIDCM_FULL_SEL);
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg ((reg_emi | (RG_EMIDCM_APB_TOG_MASK << RG_EMIDCM_APB_TOG_BIT)), TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
				}
				if (ap_dcm_emi_ctrl.dbc_en != ctl->dbc_en)
				{
					/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, ~(0x1<<8), (ctl->dbc_en<<8) | 0x8);  */
					reg_emi = (0x0400FE00 & dcm_read_reg (TOPEMI_DCMCTL)) | 0x08;
					dcm_set_field (reg_emi, ctl->dbc_en, RG_EMIDCM_DBC_ENABLE);
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg ((reg_emi | (RG_EMIDCM_APB_TOG_MASK << RG_EMIDCM_APB_TOG_BIT)), TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);					
				}
				if (ap_dcm_emi_ctrl.dcm_en != ctl->dcm_en)
				{

					//- WHQA_00014411
					//- disable auto refresh
					reg_refresh = dcm_read_reg ((EMI_BASE + 0x68));				
					dcm_write_reg (reg_refresh &0xFFFFFFFD, (EMI_BASE + 0x68));
					udelay (10);

					/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, ~(0x1<<7), (ctl->dcm_en<<7) | 0x4);  */
					reg_emi = (0x0400FE00 & dcm_read_reg (TOPEMI_DCMCTL)) | 0x04;
					dcm_set_field (reg_emi, ctl->dcm_en, RG_EMIDCM_ENABLE);
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg ((reg_emi | (RG_EMIDCM_APB_TOG_MASK << RG_EMIDCM_APB_TOG_BIT)), TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);

					//- re-enable auto refresh
					udelay (10);
					dcm_write_reg (reg_refresh, (EMI_BASE + 0x68));

				}
				if (ap_dcm_emi_ctrl.force_on != ctl->force_on)			
				{
					/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, ~(0x1<<6), (ctl->force_on<<6) | 0x2);  */
					reg_emi = (0x0400FE00 & dcm_read_reg (TOPEMI_DCMCTL)) | 0x02;
					dcm_set_field (reg_emi, ctl->force_on, RG_EMIDCM_FORCE_ON);
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg ((reg_emi | (RG_EMIDCM_APB_TOG_MASK << RG_EMIDCM_APB_TOG_BIT)), TOPEMI_DCMCTL);
					dsb();
					dcm_write_reg (reg_emi, TOPEMI_DCMCTL);
				}
				memcpy((void*)&ap_dcm_emi_ctrl, ctrl, sizeof (AP_DCM_EMI_CTRL));
			}break;
		default:
			return; //fixme
			break;
	}
	
	//- field exclusive apb_sel
	/* *TOPEMI_DCMCTL = aor(*TOPEMI_DCMCTL, ~(0x1f<<1), 0);  */
	reg_0 = 0;
	dcm_set_field (reg_0, ap_dcm_emi_ctrl.force_off, RG_EMIDCM_FORCE_OFF);
	dcm_set_field (reg_0, ap_dcm_emi_ctrl.force_on, RG_EMIDCM_FORCE_ON);
	dcm_set_field (reg_0, ap_dcm_emi_ctrl.dcm_en, RG_EMIDCM_ENABLE);
	dcm_set_field (reg_0, ap_dcm_emi_ctrl.dbc_en, RG_EMIDCM_DBC_ENABLE);
	dcm_set_field (reg_0, ap_dcm_emi_ctrl.dbc_cnt, RG_EMIDCM_DBC_CNT);
	dcm_set_field (reg_0, ap_dcm_emi_ctrl.full_sel, RG_EMIDCM_FULL_SEL);
	dcm_set_field (reg_0, ap_dcm_emi_ctrl.idle_fsel, RG_EMIDCM_IDLE_FSEL);	

	dcm_write_reg (reg_0, TOPEMI_DCMCTL);
	dsb();
	dcm_write_reg ((reg_0 | (RG_EMIDCM_APB_TOG_MASK << RG_EMIDCM_APB_TOG_BIT)), TOPEMI_DCMCTL);
	dsb();
	dcm_write_reg (reg_0, TOPEMI_DCMCTL);

	em_dcm_sta[EM_EMI_DCM] = reg_0;
	
	em_dcm_en[EM_EMI_DCM] = (ap_dcm_emi_ctrl.dcm_en << 0);
	
	DCM_UNLOCK;
}

static void ap_dcm_infra (UINT32 option, void* ctrl)
{
	UINT32 reg_0 = 0;
	UINT32 reg_1 = 0;
	
	DCM_LOCK;
	
	switch (option)
	{
		case AP_DCM_OP_DISABLE:
			ap_dcm_infra_ctrl.clkoff_en = 0;
			ap_dcm_infra_ctrl.clkslw_en = 0;
			break;
		case AP_DCM_OP_ENABLE:
			ap_dcm_infra_ctrl.clkoff_en = 1;
			ap_dcm_infra_ctrl.clkslw_en = 1;			
			break;
		case AP_DCM_OP_CONFIG:
			memcpy((void*)&ap_dcm_infra_ctrl, ctrl, sizeof (AP_DCM_INFRA_CTRL));
			break;
		default:
			return; //fixme
			break;
	}

	//- force on first
	dcm_write_reg ((1 << RG_INFRADCM_FORCE_ON_BIT), SET_DCM_INFRABUS_DCMCTL0);

	//- clear field ctrl_1
	dcm_set_field (reg_1, RG_INFRADCM_DEBOUNCE_EN_MASK, RG_INFRADCM_DEBOUNCE_EN);
	dcm_set_field (reg_1, RG_INFRADCM_DEBOUNCE_CNT_MASK, RG_INFRADCM_DEBOUNCE_CNT);
	dcm_write_reg (reg_1, CLR_DCM_INFRABUS_DCMCTL1);

	dsb ();

	//- set field ctrl_1
	reg_1 = 0;
	dcm_set_field (reg_1, ap_dcm_infra_ctrl.dbc_en, RG_INFRADCM_DEBOUNCE_EN);
	dcm_set_field (reg_1, ap_dcm_infra_ctrl.dbc_cnt, RG_INFRADCM_DEBOUNCE_CNT);
	dcm_write_reg (reg_1, SET_DCM_INFRABUS_DCMCTL1);

	//- clear field ctrl_0
	reg_0 = 0;
	dcm_set_field (reg_0, RG_INFRADCM_FORCE_CLKOFF_MASK, RG_INFRADCM_FORCE_CLKOFF);
	dcm_set_field (reg_0, RG_INFRADCM_FORCE_CLKSLW_MASK, RG_INFRADCM_FORCE_CLKSLW);
	dcm_set_field (reg_0, RG_INFRADCM_CLKOFF_EN_MASK, RG_INFRADCM_CLKOFF_EN);
	dcm_set_field (reg_0, RG_INFRADCM_CLKSLW_EN_MASK, RG_INFRADCM_CLKSLW_EN);
	dcm_set_field (reg_0, RG_INFRADCM_SFSEL_MASK, RG_INFRADCM_SFSEL);
//	dcm_set_field (reg_0, RG_INFRADCM_FSEL_MASK, RG_INFRADCM_FSEL);
	dcm_write_reg (reg_0, CLR_DCM_INFRABUS_DCMCTL0);

	dsb ();

	//- set field ctrl_0
	reg_0 = 0;
	dcm_set_field (reg_0, ap_dcm_infra_ctrl.force_clkoff, RG_INFRADCM_FORCE_CLKOFF);
	dcm_set_field (reg_0, ap_dcm_infra_ctrl.force_clkslw, RG_INFRADCM_FORCE_CLKSLW);
	dcm_set_field (reg_0, ap_dcm_infra_ctrl.clkoff_en, RG_INFRADCM_CLKOFF_EN);
	dcm_set_field (reg_0, ap_dcm_infra_ctrl.clkslw_en, RG_INFRADCM_CLKSLW_EN);
	dcm_set_field (reg_0, ap_dcm_infra_ctrl.full_sel, RG_INFRADCM_SFSEL);
	dcm_set_field (reg_0, ap_dcm_infra_ctrl.idle_fsel, RG_INFRADCM_FSEL);
	dcm_write_reg (reg_0, SET_DCM_INFRABUS_DCMCTL0);

	dsb ();

	//- write force on field
	if (ap_dcm_infra_ctrl.force_on)
	{
		dcm_write_reg ((1 << RG_INFRADCM_FORCE_ON_BIT), SET_DCM_INFRABUS_DCMCTL0);
	}
	else
	{
		dcm_write_reg ((1 << RG_INFRADCM_FORCE_ON_BIT), CLR_DCM_INFRABUS_DCMCTL0);
	}

	em_dcm_sta[EM_INFRA_DCM] = reg_0 | (reg_1 << 8);

	em_dcm_en[EM_INFRA_DCM] = (ap_dcm_infra_ctrl.clkoff_en << 0);
	
	DCM_UNLOCK;
}

static void ap_dcm_peri (UINT32 option, void* ctrl)
{
	UINT32 reg_0 = 0;
	UINT32 reg_1 = 0;
		
	DCM_LOCK;
	
	switch (option)
	{
		case AP_DCM_OP_DISABLE:
			ap_dcm_peri_ctrl.clkoff_en = 0;
			ap_dcm_peri_ctrl.clkslw_en = 0;
			break;
		case AP_DCM_OP_ENABLE:
			ap_dcm_peri_ctrl.clkoff_en = 1;
			ap_dcm_peri_ctrl.clkslw_en = 1;			
			break;
		case AP_DCM_OP_CONFIG:
			memcpy((void*)&ap_dcm_peri_ctrl, ctrl, sizeof (AP_DCM_PERI_CTRL));
			break;
		default:
			return;
			break;
	}

	//- force on first
	dcm_write_reg ((1 << RG_PERIDCM_FORCE_ON_BIT), SET_DCM_INFRABUS_DCMCTL0);

	//- clear field ctrl_1
	dcm_set_field (reg_1, RG_PERIDCM_DEBOUNCE_EN_MASK, RG_PERIDCM_DEBOUNCE_EN);
	dcm_set_field (reg_1, RG_PERIDCM_DEBOUNCE_CNT_MASK, RG_PERIDCM_DEBOUNCE_CNT);
	dcm_write_reg (reg_1, CLR_DCM_INFRABUS_DCMCTL1);

	dsb ();

	//- set field ctrl_1
	reg_1 = 0;
	dcm_set_field (reg_1, ap_dcm_peri_ctrl.dbc_en, RG_PERIDCM_DEBOUNCE_EN);
	dcm_set_field (reg_1, ap_dcm_peri_ctrl.dbc_cnt, RG_PERIDCM_DEBOUNCE_CNT);
	dcm_write_reg (reg_1, SET_DCM_INFRABUS_DCMCTL1);

	//- clear field ctrl_0
	reg_0 = 0;
	dcm_set_field (reg_0, RG_PERIDCM_FORCE_CLKOFF_MASK, RG_PERIDCM_FORCE_CLKOFF);
	dcm_set_field (reg_0, RG_PERIDCM_FORCE_CLKSLW_MASK, RG_PERIDCM_FORCE_CLKSLW);
	dcm_set_field (reg_0, RG_PERIDCM_CLKOFF_EN_MASK, RG_PERIDCM_CLKOFF_EN);
	dcm_set_field (reg_0, RG_PERIDCM_CLKSLW_EN_MASK, RG_PERIDCM_CLKSLW_EN);
	dcm_set_field (reg_0, RG_PERIDCM_SFSEL_MASK, RG_PERIDCM_SFSEL);
//-	dcm_set_field (reg_0, RG_PERIDCM_FSEL_MASK, RG_PERIDCM_FSEL);
	dcm_write_reg (reg_0, CLR_DCM_INFRABUS_DCMCTL0);

	dsb ();

	//- set field ctrl_0
	reg_0 = 0;
	dcm_set_field (reg_0, ap_dcm_peri_ctrl.force_clkoff, RG_PERIDCM_FORCE_CLKOFF);
	dcm_set_field (reg_0, ap_dcm_peri_ctrl.force_clkslw, RG_PERIDCM_FORCE_CLKSLW);
	dcm_set_field (reg_0, ap_dcm_peri_ctrl.clkoff_en, RG_PERIDCM_CLKOFF_EN);
	dcm_set_field (reg_0, ap_dcm_peri_ctrl.clkslw_en, RG_PERIDCM_CLKSLW_EN);
	dcm_set_field (reg_0, ap_dcm_peri_ctrl.full_sel, RG_PERIDCM_SFSEL);
	dcm_set_field (reg_0, ap_dcm_peri_ctrl.idle_fsel, RG_PERIDCM_FSEL);
	dcm_write_reg (reg_0, SET_DCM_INFRABUS_DCMCTL0);

	dsb ();

	//- write force on field
	if (ap_dcm_peri_ctrl.force_on)
	{
		dcm_write_reg ((1 << RG_PERIDCM_FORCE_ON_BIT), SET_DCM_INFRABUS_DCMCTL0);
	}
	else
	{
		dcm_write_reg ((1 << RG_PERIDCM_FORCE_ON_BIT), CLR_DCM_INFRABUS_DCMCTL0);
	}
	
	em_dcm_sta[EM_PERI_DCM] = (reg_0 >> 16) | (reg_1 << 2);

	em_dcm_en[EM_PERI_DCM] = (ap_dcm_peri_ctrl.clkoff_en << 0);
	
	DCM_UNLOCK;
	
}

static void ap_dcm_pmic (UINT32 option, void* ctrl)
{
	UINT32 reg = 0;
	
	DCM_LOCK;
	
	switch (option)
	{
		case AP_DCM_OP_DISABLE:
			ap_dcm_pmic_ctrl.rg_pmic_spiclkdcm_en	= 0;
			ap_dcm_pmic_ctrl.rg_pmic_bclkdcm_en		= 0;
			break;
		case AP_DCM_OP_ENABLE:
			ap_dcm_pmic_ctrl.rg_pmic_spiclkdcm_en 	= 1;
			ap_dcm_pmic_ctrl.rg_pmic_bclkdcm_en		= 1;			
			break;
		case AP_DCM_OP_CONFIG:
			memcpy((void*)&ap_dcm_pmic_ctrl, ctrl, sizeof (AP_DCM_PMIC_CTRL));
			break;
		default:
			return;
			break;
	}

	//- clr field
	dcm_set_field (reg, RG_PMIC_SFSEL_MASK, RG_PMIC_SFSEL);
	dcm_set_field (reg, RG_PMIC_SPICLKDCM_EN_MASK, RG_PMIC_SPICLKDCM_EN);
	dcm_set_field (reg, RG_PMIC_BCLKDCM_EN_MASK, RG_PMIC_BCLKDCM_EN);
	dcm_write_reg (reg, CLR_DCM_INFRABUS_DCMCTL1);

	dsb();

	//- set field
	reg = 0;
	dcm_set_field (reg, ap_dcm_pmic_ctrl.rg_pmic_sfsel, RG_PMIC_SFSEL);
	dcm_set_field (reg, ap_dcm_pmic_ctrl.rg_pmic_spiclkdcm_en, RG_PMIC_SPICLKDCM_EN);
	dcm_set_field (reg, ap_dcm_pmic_ctrl.rg_pmic_bclkdcm_en, RG_PMIC_BCLKDCM_EN);
	dcm_write_reg (reg, SET_DCM_INFRABUS_DCMCTL1);

	em_dcm_sta[EM_MISC_DCM] &= (~0xFE);
	em_dcm_sta[EM_MISC_DCM] |= reg;

	em_dcm_en[EM_MISC_DCM] &= (~(1 << 1));
	em_dcm_en[EM_MISC_DCM] |= (ap_dcm_pmic_ctrl.rg_pmic_spiclkdcm_en << 1);
	
	DCM_UNLOCK;
}

static void ap_dcm_usb (UINT32 option, void* ctrl)
{	
	DCM_LOCK;
	
	switch (option)
	{
		case AP_DCM_OP_DISABLE:
			ap_dcm_usb_ctrl.rg_usbdcm_en = 0;
			break;
		case AP_DCM_OP_ENABLE:
			ap_dcm_usb_ctrl.rg_usbdcm_en = 1;
			break;
		case AP_DCM_OP_CONFIG:
			memcpy((void*)&ap_dcm_usb_ctrl, ctrl, sizeof (AP_DCM_USB_CTRL));
			break;
		default:
			return;
			break;
	}
	
	if (ap_dcm_usb_ctrl.rg_usbdcm_en)
	{
		dcm_write_reg ((1 << RG_USBDCM_EN_BIT), SET_DCM_INFRABUS_DCMCTL1);
		em_dcm_sta[EM_MISC_DCM] |= 0x1;
		em_dcm_en[EM_MISC_DCM] |= (1 << 0);
	}
	else
	{
		dcm_write_reg ((1 << RG_USBDCM_EN_BIT), CLR_DCM_INFRABUS_DCMCTL1);
		em_dcm_sta[EM_MISC_DCM] &= (~0x1);
		em_dcm_en[EM_MISC_DCM] &= (~(1 << 0));
	}
	
	DCM_UNLOCK;
}

void dcm_mmsys_disable (UINT32 id_mask)
{
	UINT32 reg;

	DCM_LOCK;

	//- MMSYS_HW_DCM_DIS_xxx0
	reg = id_mask & ((1 << MM_DCM_DSI_DIGITAL) - 1);
	dcm_write_reg (reg, MMSYS_HW_DCM_DIS_SET0);

	//- MMSYS_HW_DCM_DIS_xxx1
	if (id_mask & (1 << MM_DCM_DSI_DIGITAL))
	{
		dcm_write_reg ((1 << 0), MMSYS_HW_DCM_DIS_SET1);
	}

	//- MMSYS_VENC_MP4_DCM_CTRL
	if (id_mask & (1 << MM_DCM_VENC))
	{
		reg = dcm_read_reg (MMSYS_VENC_MP4_DCM_CTRL);
		dcm_clr_and_set_field (reg, 1, RG_DISABLE_VENC_MP4_CLOCK_DCM);
		dcm_write_reg (reg, MMSYS_VENC_MP4_DCM_CTRL);
	}

	//- MMSYS_VDEC_DCM_DIS
	if (id_mask & (1 << MM_DCM_VDEC))
	{
		reg = dcm_read_reg (MMSYS_VDEC_DCM_DIS);
		dcm_clr_and_set_field (reg, 1, RG_VDEC_DCMDIS);
		dcm_write_reg (reg, MMSYS_VDEC_DCM_DIS);
	}

	//- RG_CAM_DCM_DIS
	if (id_mask & (1 << MM_DCM_CAM))
	{
		reg = dcm_read_reg (MMSYS_CAM_DCM_DIS);
		dcm_clr_and_set_field (reg, 1, RG_IMGO_DCM_DIS);
		dcm_clr_and_set_field (reg, 1, RG_CDRZ_DCM_DIS);
		dcm_write_reg (reg, MMSYS_CAM_DCM_DIS);
	}

	mm_dcm_ctrl &= (~id_mask);

	em_dcm_sta[EM_MM_DCM] = mm_dcm_ctrl;
	em_dcm_en[EM_MM_DCM]  = mm_dcm_ctrl;
	
	DCM_UNLOCK;	
}

void dcm_mmsys_enable (UINT32 id_mask)
{
	UINT32 reg;

	DCM_LOCK;
	
	//- MMSYS_HW_DCM_DIS_xxx0
	reg = id_mask & ((1 << MM_DCM_DSI_DIGITAL) - 1);
	dcm_write_reg (reg, MMSYS_HW_DCM_DIS_CLR0);

	//- MMSYS_HW_DCM_DIS_xxx1
	if (id_mask & (1 << MM_DCM_DSI_DIGITAL))
	{
		dcm_write_reg ((1 << 0), MMSYS_HW_DCM_DIS_CLR1);
	}

	//- MMSYS_VENC_MP4_DCM_CTRL
	if (id_mask & (1 << MM_DCM_VENC))
	{
		reg = dcm_read_reg (MMSYS_VENC_MP4_DCM_CTRL);
		dcm_clr_and_set_field (reg, 0, RG_DISABLE_VENC_MP4_CLOCK_DCM);
		dcm_write_reg (reg, MMSYS_VENC_MP4_DCM_CTRL);
	}

	//- MMSYS_VDEC_DCM_DIS
	if (id_mask & (1 << MM_DCM_VDEC))
	{
		reg = dcm_read_reg (MMSYS_VDEC_DCM_DIS);
		dcm_clr_and_set_field (reg, 0, RG_VDEC_DCMDIS);
		dcm_write_reg (reg, MMSYS_VDEC_DCM_DIS);
	}

	//- RG_CAM_DCM_DIS
	if (id_mask & (1 << MM_DCM_CAM))
	{
		reg = dcm_read_reg (MMSYS_CAM_DCM_DIS);
		dcm_clr_and_set_field (reg, 0, RG_IMGO_DCM_DIS);
		dcm_clr_and_set_field (reg, 0, RG_CDRZ_DCM_DIS);
		dcm_write_reg (reg, MMSYS_CAM_DCM_DIS);
	}

	mm_dcm_ctrl |= id_mask;

	em_dcm_sta[EM_MM_DCM] = mm_dcm_ctrl;
	em_dcm_en[EM_MM_DCM]  = mm_dcm_ctrl;
	
	DCM_UNLOCK;
}

///////////////////////////////
//- mm set by single id
//- section start
void dcm_mmsys_smi_common(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_SMI_COMMON)) : dcm_mmsys_disable ((1 << MM_DCM_SMI_COMMON));	
}

void dcm_mmsys_smi_larb0(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_SMI_LARB0)) : dcm_mmsys_disable ((1 << MM_DCM_SMI_LARB0));	
}

void dcm_mmsys_mm_cmdq(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_MM_CMDQ)) : dcm_mmsys_disable ((1 << MM_DCM_MM_CMDQ));	
}

void dcm_mmsys_mutex(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_MUTEX)) : dcm_mmsys_disable ((1 << MM_DCM_MUTEX));	
}

void dcm_mmsys_disp_color(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DISP_COLOR)) : dcm_mmsys_disable ((1 << MM_DCM_DISP_COLOR));	
}

void dcm_mmsys_disp_bls(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DISP_BLS)) : dcm_mmsys_disable ((1 << MM_DCM_DISP_BLS));	
}

void dcm_mmsys_disp_wdma(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DISP_RDMA)) : dcm_mmsys_disable ((1 << MM_DCM_DISP_RDMA));	
}

void dcm_mmsys_disp_rdma(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DISP_RDMA)) : dcm_mmsys_disable ((1 << MM_DCM_DISP_RDMA));	
}

void dcm_mmsys_disp_ovl(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DISP_OVL)) : dcm_mmsys_disable ((1 << MM_DCM_DISP_OVL));	
}

void dcm_mmsys_mdp_tdshp(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_MDP_TDSHP)) : dcm_mmsys_disable ((1 << MM_DCM_MDP_TDSHP));	
}

void dcm_mmsys_mdp_wrot(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_MDP_WROT)) : dcm_mmsys_disable ((1 << MM_DCM_MDP_WROT));	
}

void dcm_mmsys_mdp_wdma(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_MDP_WDMA)) : dcm_mmsys_disable ((1 << MM_DCM_MDP_WDMA));	
}

void dcm_mmsys_mdp_rsz1(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_MDP_RSZ1)) : dcm_mmsys_disable ((1 << MM_DCM_MDP_RSZ1));	
}

void dcm_mmsys_mdp_rsz0(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_MDP_RSZ0)) : dcm_mmsys_disable ((1 << MM_DCM_MDP_RSZ0));	
}

void dcm_mmsys_mdp_rdma(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_MDP_RDMA)) : dcm_mmsys_disable ((1 << MM_DCM_MDP_RDMA));	
}

void dcm_mmsys_dsi_engine(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DISP_DBI_ENGINE)) : dcm_mmsys_disable ((1 << MM_DCM_DISP_DBI_ENGINE));	
}

void dcm_mmsys_disp_dbi_engine(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DISP_DBI_ENGINE)) : dcm_mmsys_disable ((1 << MM_DCM_DISP_DBI_ENGINE));	
}

void dcm_mmsys_disp_dbi_smi(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DISP_DBI_SMI)) : dcm_mmsys_disable ((1 << MM_DCM_DISP_DBI_SMI));	
}

void dcm_mmsys_dsi_digital(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_DSI_DIGITAL)) : dcm_mmsys_disable ((1 << MM_DCM_DSI_DIGITAL));	
}

void dcm_mmsys_venc(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_VENC)) : dcm_mmsys_disable ((1 << MM_DCM_VENC));	
}

void dcm_mmsys_vdec(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_VDEC)) : dcm_mmsys_disable ((1 << MM_DCM_VDEC));	
}

void dcm_mmsys_cam(UINT32 enable)
{
	enable ? dcm_mmsys_enable ((1 << MM_DCM_CAM)) : dcm_mmsys_disable ((1 << MM_DCM_CAM));	
}
//- mm set by single id
//- section end
///////////////////////////////
static void mfg_dcm_enable(UINT32 enable)
{
	UINT32 reg;
	
	DCM_LOCK;	
	reg = dcm_read_reg (MFG_DCM_CON_0);
		
	dcm_clr_and_set_field (reg, enable, RG_BG3D_DCM_EN);

	dcm_write_reg (reg, MFG_DCM_CON_0);

	em_dcm_sta[EM_MFG_DCM] = reg;
	em_dcm_en[EM_MFG_DCM]  = (enable << 0);
	DCM_UNLOCK;
}

void dcm_mfgsys_gpu(MFG_DCM_BG3D_CTRL* ctrl)
{
	UINT32 reg;
	
	DCM_LOCK;
	reg = dcm_read_reg (MFG_DCM_CON_0);
	dcm_clr_and_set_field (reg, ctrl->dcm_en, 	RG_BG3D_DCM_EN);
	dcm_clr_and_set_field (reg, ctrl->dcm_fsel,	RG_BG3D_FSEL);
	dcm_clr_and_set_field (reg, ctrl->dbc_en,	RG_BG3D_DBC_EN);
	dcm_clr_and_set_field (reg, ctrl->dbc_cnt,	RG_BG3D_DBC_CNT);
	dcm_write_reg (reg, MFG_DCM_CON_0);

	em_dcm_sta[EM_MFG_DCM] = reg;
	em_dcm_en[EM_MFG_DCM]  = (ctrl->dcm_en << 0);
	DCM_UNLOCK;
}

#ifndef __DCM_CTP__
static INT32 dcm_proc_dbg_read(char *page, char **start, off_t off, int count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 sta[NUM_OF_EM_DCM_TYPE];
	UINT32 *p_em_dcm;
	UINT32 len = 0;
	UINT32 i;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);
	
	DCM_LOCK;
	p_em_dcm = (em_dcm_mode == 1) ? &(em_dcm_sta[0]) : &(em_dcm_en[0]);
	for(i=0; i<NUM_OF_EM_DCM_TYPE; i++)
	{
		sta[i] = *p_em_dcm ++;
	}
	DCM_UNLOCK;
	
	for (i=0; i<NUM_OF_EM_DCM_TYPE; i++)
	{
		p += sprintf(p, "%s=0x%08X,\n", EM_dcm_name[i], sta[i]);
	}

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;
}

static INT32 dcm_proc_dbg_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	INT32 		ret;
	char 		kbuf[256];
	UINT32	 	len = 0;
	UINT32		p1,p2,p3,p4;

	dcm_info("Enter: %s\n",__func__);

	len = min(count, (typeof(count))(sizeof(kbuf)-1));

	if (count == 0)		return -1;
	if(count > 255)		count = 255;

	ret = copy_from_user(kbuf, buffer, count);
	if (ret < 0)		return -1;

	kbuf[count] = '\0';

	sscanf(kbuf, "%x %x %x %x", &p1, &p2, &p3, &p4);

	if (p2 == 1)
	{	//- enable all AP DCM
		ap_dcm_enable (AP_DCM_ON_ARM_ALLBUS);
	}
	else if (p2 == 2)
	{	//- disable all AP DCM
		ap_dcm_disable (AP_DCM_ON_ARM_ALLBUS);		
	}
	else
	{
		switch (p3)
		{
			case EM_ARM_DCM:{
					AP_DCM_ARMCORE_CTRL		core_ctrl;
					AP_DCM_ARML2BUS_CTRL	l2bus_ctrl;
					
					{	//- check : need to switch em mode ???
						if (p4 == 0x5AA00000)			{em_dcm_mode = 0; return count;}
						else if (p4 == 0x5AA10000)		{em_dcm_mode = 1; return count;}											
					}
	
					if (em_dcm_mode)
					{
						if(p4 & 0x1)		core_ctrl.mode = ARM_CORE_DCM_MODE1;
						else if (p4 & 0x6)	core_ctrl.mode = ARM_CORE_DCM_MODE2;
						else				core_ctrl.mode = ARM_CORE_DCM_DISABLE;
		
						ap_dcm_armcore(AP_DCM_OP_CONFIG, (void*) &core_ctrl);
		
						l2bus_ctrl.cg0_div	= AXI_DIV_SEL_UNCHANGE;
						l2bus_ctrl.cg1_en	= (p4 >> 9) & 0x1;
						l2bus_ctrl.cg2_en	= (p4 >> 8) & 0x1;
						
						ap_dcm_arml2bus(AP_DCM_OP_CONFIG, (void*) &l2bus_ctrl);
					}
					else
					{
						(p4 & 0x1) ? ap_dcm_enable (AP_DCM_ARMCORE_MASK) : ap_dcm_disable (AP_DCM_ARMCORE_MASK);
						(p4 & 0x2) ? ap_dcm_enable (AP_DCM_ARML2BUS_MASK) : ap_dcm_disable (AP_DCM_ARML2BUS_MASK);
					}
				}break;
			case EM_EMI_DCM:{
					AP_DCM_EMI_CTRL	ctrl;
	
					if (em_dcm_mode)
					{
						ctrl.force_off	= (p4 >> RG_EMIDCM_FORCE_OFF_BIT)	& RG_EMIDCM_FORCE_OFF_MASK;
						ctrl.force_on	= (p4 >> RG_EMIDCM_FORCE_ON_BIT) 	& RG_EMIDCM_FORCE_ON_MASK;
						ctrl.dcm_en 	= (p4 >> RG_EMIDCM_ENABLE_BIT) 		& RG_EMIDCM_ENABLE_MASK;
						ctrl.dbc_en 	= (p4 >> RG_EMIDCM_DBC_ENABLE_BIT)	& RG_EMIDCM_DBC_ENABLE_MASK;
						ctrl.dbc_cnt 	= (p4 >> RG_EMIDCM_DBC_CNT_BIT)		& RG_EMIDCM_DBC_CNT_MASK;
						ctrl.full_sel 	= (p4 >> RG_EMIDCM_FULL_SEL_BIT) 	& RG_EMIDCM_FULL_SEL_MASK;
						ctrl.idle_fsel 	= (p4 >> RG_EMIDCM_IDLE_FSEL_BIT) 	& RG_EMIDCM_IDLE_FSEL_MASK;
		
						ap_dcm_emi (AP_DCM_OP_CONFIG, (void*) &ctrl);
					}
					else
					{
						(p4 & 0x1) ? ap_dcm_enable (AP_DCM_EMI_MASK) : ap_dcm_disable (AP_DCM_EMI_MASK);
					}
					
				}break;
			case EM_INFRA_DCM:{
					AP_DCM_INFRA_CTRL	ctrl;
	
					if (em_dcm_mode)
					{
						ctrl.force_on		= (p4 >> RG_INFRADCM_FORCE_ON_BIT)			& RG_INFRADCM_FORCE_ON_MASK;
						ctrl.force_clkoff	= (p4 >> RG_INFRADCM_FORCE_CLKOFF_BIT)		& RG_INFRADCM_FORCE_CLKOFF_MASK;
						ctrl.force_clkslw	= (p4 >> RG_INFRADCM_FORCE_CLKSLW_BIT)		& RG_INFRADCM_FORCE_CLKSLW_MASK;
						ctrl.clkoff_en		= (p4 >> RG_INFRADCM_CLKOFF_EN_BIT)			& RG_INFRADCM_CLKOFF_EN_MASK;
						ctrl.clkslw_en		= (p4 >> RG_INFRADCM_CLKSLW_EN_BIT)			& RG_INFRADCM_CLKSLW_EN_MASK;
						ctrl.full_sel		= (p4 >> RG_INFRADCM_SFSEL_BIT)				& RG_INFRADCM_SFSEL_MASK;
						ctrl.idle_fsel		= (p4 >> RG_INFRADCM_FSEL_BIT)				& RG_INFRADCM_FSEL_MASK;
		
						ctrl.dbc_en			= (p4 >> (RG_INFRADCM_DEBOUNCE_EN_BIT  + 8))& RG_INFRADCM_DEBOUNCE_EN_MASK;
						ctrl.dbc_cnt		= (p4 >> (RG_INFRADCM_DEBOUNCE_CNT_BIT + 8))& RG_INFRADCM_DEBOUNCE_CNT_MASK;
		
						ap_dcm_infra (AP_DCM_OP_CONFIG, (void*) &ctrl);
					}
					else
					{
						(p4 & 0x1) ? ap_dcm_enable (AP_DCM_INFRA_MASK) : ap_dcm_disable (AP_DCM_INFRA_MASK);
					}
				}break;
			case EM_PERI_DCM:{
					AP_DCM_PERI_CTRL	ctrl;
	
					if (em_dcm_mode)
					{
						ctrl.force_on		= (p4 >> (RG_PERIDCM_FORCE_ON_BIT 		- 16))	& RG_PERIDCM_FORCE_ON_MASK;
						ctrl.force_clkoff	= (p4 >> (RG_PERIDCM_FORCE_CLKOFF_BIT 	- 16))	& RG_PERIDCM_FORCE_CLKOFF_MASK;
						ctrl.force_clkslw	= (p4 >> (RG_PERIDCM_FORCE_CLKSLW_BIT 	- 16))	& RG_PERIDCM_FORCE_CLKSLW_MASK;
						ctrl.clkoff_en		= (p4 >> (RG_PERIDCM_CLKOFF_EN_BIT 		- 16))	& RG_PERIDCM_CLKOFF_EN_MASK;
						ctrl.clkslw_en		= (p4 >> (RG_PERIDCM_CLKSLW_EN_BIT 		- 16))	& RG_PERIDCM_CLKSLW_EN_MASK;
						ctrl.full_sel		= (p4 >> (RG_PERIDCM_SFSEL_BIT 			- 16))	& RG_PERIDCM_SFSEL_MASK;
						ctrl.idle_fsel		= (p4 >> (RG_PERIDCM_FSEL_BIT 			- 16))	& RG_PERIDCM_FSEL_MASK;
		
						ctrl.dbc_en			= (p4 >> (RG_PERIDCM_DEBOUNCE_EN_BIT 	+ 2))	& RG_PERIDCM_DEBOUNCE_EN_MASK;
						ctrl.dbc_cnt		= (p4 >> (RG_PERIDCM_DEBOUNCE_CNT_BIT	+ 2))	& RG_PERIDCM_DEBOUNCE_CNT_MASK;
		
						ap_dcm_peri (AP_DCM_OP_CONFIG, (void*) &ctrl);
					}
					else
					{
						(p4 & 0x1) ? ap_dcm_enable (AP_DCM_PERI_MASK) : ap_dcm_disable (AP_DCM_PERI_MASK);
					}
				}break;
			case EM_MISC_DCM:{
					AP_DCM_PMIC_CTRL	pmic_ctrl;
					AP_DCM_USB_CTRL		usb_ctrl;
	
					if (em_dcm_mode)
					{
						pmic_ctrl.rg_pmic_sfsel			= (p4 >> RG_PMIC_SFSEL_BIT) 		& RG_PMIC_SFSEL_MASK;
						pmic_ctrl.rg_pmic_spiclkdcm_en	= (p4 >> RG_PMIC_SPICLKDCM_EN_BIT)	& RG_PMIC_SPICLKDCM_EN_MASK;
						pmic_ctrl.rg_pmic_bclkdcm_en	= (p4 >> RG_PMIC_BCLKDCM_EN_BIT)	& RG_PMIC_BCLKDCM_EN_MASK;
		
						ap_dcm_pmic (AP_DCM_OP_CONFIG, (void*) &pmic_ctrl);
		
						usb_ctrl.rg_usbdcm_en = (p4 >> RG_USBDCM_EN_BIT) & RG_USBDCM_EN_MASK;
		
						ap_dcm_usb (AP_DCM_OP_CONFIG, (void*) &usb_ctrl);
					}
					else
					{
						(p4 & 0x1) ? ap_dcm_enable (AP_DCM_USB_MASK) : ap_dcm_disable (AP_DCM_USB_MASK);
						(p4 & 0x2) ? ap_dcm_enable (AP_DCM_PMIC_MASK) : ap_dcm_disable (AP_DCM_PMIC_MASK);
					}
				}break;
			case EM_MM_DCM:
					dcm_mmsys_enable (p4);
					dcm_mmsys_disable (~p4);
				break;
			case EM_MFG_DCM:{
					MFG_DCM_BG3D_CTRL ctrl;
	
					if (em_dcm_mode)
					{
						ctrl.dcm_en 	= (p4 >> RG_BG3D_DCM_EN_BIT) & RG_BG3D_DCM_EN_MASK;
						ctrl.dcm_fsel	= (p4 >> RG_BG3D_FSEL_BIT) & RG_BG3D_FSEL_MASK;
						ctrl.dbc_en		= (p4 >> RG_BG3D_DBC_EN_BIT) & RG_BG3D_DBC_EN_MASK;
						ctrl.dbc_cnt	= (p4 >> RG_BG3D_DBC_CNT_BIT) & RG_BG3D_DBC_CNT_MASK;
		
						dcm_mfgsys_gpu (&ctrl);
					}
					else
					{
						(p4 & 0x1) ? mfg_dcm_enable (1) : mfg_dcm_enable (0);					
					}
				}break;
			default:
				break;
		}
	}
	
	return count;
}

static INT32 dcm_proc_dumpregs_read(char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;
	UINT32 i;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);
	
	for (i=0; i<NUM_OF_DCM_REGS; i++)
	{
		p += sprintf(p, "0x%08X, %s\r\n", dcm_read_reg(dcm_reg_map[i].addr), dcm_reg_map[i].name);
	}

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;
}

static INT32 dcm_proc_dumpregs_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	dcm_info("Enter: %s\n",__func__);

	return 0;
}

static INT32 dcm_proc_help_read(char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;
	UINT32 i;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);
	for (i=0; i<NUM_OF_EM_DCM_TYPE; i++)
	{
		p += sprintf(p, "\r\nhelp!!!\r\n,");
	}

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;
}

static INT32 dcm_proc_help_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	INT32 		ret;
	char 		kbuf[256];
	UINT32	 	len = 0;
	UINT32		p1;

	dcm_info("Enter: %s\n",__func__);

	len = min(count, (typeof(count))(sizeof(kbuf)-1));

	if (count == 0)		return -1;
	if(count > 255)		count = 255;

	ret = copy_from_user(kbuf, buffer, count);
	if (ret < 0)		return -1;

	kbuf[count] = '\0';

	sscanf(kbuf, "%x", &p1);

	if (p1 & GPIODBGOUT_CHANGE)
	{
		gpiodbg_sel	= (p1 >> GPIODBGOUT_BIT) & GPIODBGOUT_MASK;
	}

	switch (p1 & 0xFF)
	{
		case AP_DCM_ARMCORE:	gpiodbg_armcore_dbg_out ();		break;
		case AP_DCM_ARML2BUS:	gpiodbg_arml2bus_dbg_out ();	break;
		case AP_DCM_EMI:		gpiodbg_emi_dbg_out ();			break;
		case AP_DCM_INFRA:		gpiodbg_infra_dbg_out ();		break;
		case AP_DCM_PERI:		gpiodbg_peri_dbg_out ();		break;
		default:												break;			
	}

		

	return count;
}

#define dcm_create_proc(fILENAME)															\
do {                                                                                        \
		struct proc_dir_entry *pEntry;			                                            \
		                                                                                    \
		pEntry = create_proc_entry(#fILENAME,  S_IRUGO | S_IWUSR | S_IWGRP, proc_dir); 		\
		if(pEntry)                                                                          \
		{                                                                                   \
			pEntry->read_proc  = dcm_proc_##fILENAME##_read;                               	\
			pEntry->write_proc = dcm_proc_##fILENAME##_write;                               \
			dcm_info("[%s]: successfully create /proc/dcm/%s", __func__, #fILENAME);        \
		}else{                                                                              \
			dcm_info("[%s]: failed to create /proc/dcm/%s", __func__, #fILENAME);           \
		}																					\
} while (0)

void mt_dcm_init(void)
{
	struct proc_dir_entry *proc_dir = NULL;

	//TODO: check the permission!!

	dcm_info("Enter: %s\n",__func__);

	proc_dir = proc_mkdir("dcm", NULL);
	if (!proc_dir)
	{
		dcm_err("[Err] proc_mkdir dcm fail!\n");
		return;
	}
	else
	{
		dcm_create_proc (dbg);
		dcm_create_proc (dumpregs);
		dcm_create_proc (help);
	}

	dcm_init (AP_DCM_ON_ARM_ALLBUS);

	//- mt frwq meter init
	mt_fqmtr_init();
	//- mt lpm init
	mt_lpm_init();
}
#endif //- !__DCM_CTP__












////////////////////////////////////////////////
//- gpiodbg monitor
void gpiodbg_monitor(void)
{
	UINT32 reg;

	if (gpiodbg_sel & GPIODBGOUT_LPD)
	{

		reg = 0x88FFFFFF & dcm_read_reg (GPIO_AUX_MODE(1));
		reg |= 0x66000000;
		dcm_write_reg(reg, GPIO_AUX_MODE(1));

		dcm_write_reg(0x66666666, GPIO_AUX_MODE(2));

		reg = 0xFF888888 & dcm_read_reg (GPIO_AUX_MODE(3));
		reg |= 0x00666666;
		dcm_write_reg(reg, GPIO_AUX_MODE(3));
	}

	if (gpiodbg_sel & GPIODBGOUT_BPI)
	{
		reg = 0x888888FF & dcm_read_reg (GPIO_AUX_MODE(16));
		reg |= 0x66666600;
		dcm_write_reg(reg, GPIO_AUX_MODE(16));
	
		dcm_write_reg(0x66666666, GPIO_AUX_MODE(17));
	
	
		reg = 0xFFFFFF88 & dcm_read_reg (GPIO_AUX_MODE(18));
		reg |= 0x00000066;
		dcm_write_reg(reg, GPIO_AUX_MODE(18));
	}
}

void gpiodbg_armcore_dbg_out(void)
{
	UINT32 reg;

	/* *MCU_BIU_CON = aor(*MCU_BIU_CON, ~(1<<2), 0); */
	reg = dcm_read_reg (MCU_BIU_CON);
	dcm_clr_and_set_field (reg, 0, CLKMUX_SEL_MON);
	dcm_write_reg (reg, MCU_BIU_CON);

	/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~0x0ff, 0x0ff); */
	reg = dcm_read_reg (TEST_DBG_CTRL);
	dcm_clr_and_set_field (reg, 0xFF, RG_ARMCLK_K1);
	dcm_write_reg (reg, TEST_DBG_CTRL);

	/* *DBG_CTRL = aor(*DBG_CTRL, ~0x0ff, 7); */
	reg = dcm_read_reg (DBG_CTRL);
	dcm_clr_and_set_field (reg, 0x07, CA7_MON_SEL);
	dcm_write_reg (reg, DBG_CTRL);

	/* *INFRA_AO_DBG_CON0 = aor(*INFRA_AO_DBG_CON0, ~0x3f, 0x27); */
	reg = dcm_read_reg (INFRA_AO_DBG_CON0);
	dcm_clr_and_set_field (reg, 1, DEBUG_PIN_SEL);
	dcm_clr_and_set_field (reg, MUCSYS_DEBUG_PINS_LSB, INFRA_AO_DEBUG_MON0);
	dcm_write_reg (reg, INFRA_AO_DBG_CON0);

	//- GPIO setting
	gpiodbg_monitor();
}

void gpiodbg_arml2bus_dbg_out(void)
{
	UINT32 reg;

	/* *MCU_BIU_CON = aor(*MCU_BIU_CON, ~(1<<2), (1<<2)); */
	reg = dcm_read_reg (MCU_BIU_CON);
	dcm_clr_and_set_field (reg, 0x01, CLKMUX_SEL_MON);
	dcm_write_reg (reg, MCU_BIU_CON);

	/* *DBG_CTRL = aor(*DBG_CTRL, ~0x0ff, 7); */
	reg = dcm_read_reg (DBG_CTRL);
	dcm_clr_and_set_field (reg, 0x07, CA7_MON_SEL);
	dcm_write_reg (reg, DBG_CTRL);

	/* *INFRA_AO_DBG_CON0 = aor(*INFRA_AO_DBG_CON0, ~0x3f, 0x27); */
	reg = dcm_read_reg (INFRA_AO_DBG_CON0);
	dcm_clr_and_set_field (reg, 1, DEBUG_PIN_SEL);
	dcm_clr_and_set_field (reg, MUCSYS_DEBUG_PINS_LSB, INFRA_AO_DEBUG_MON0);
	dcm_write_reg (reg, INFRA_AO_DBG_CON0);

	//- GPIO setting
	gpiodbg_monitor();
}


void gpiodbg_spm_csw_dbg_out(void)
{
	UINT32 reg;

	/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~(1<<12), 1<<12); */
	reg = dcm_read_reg (TEST_DBG_CTRL);
	dcm_clr_and_set_field (reg, 0x1, RG_CLK_DBG_EN);
	dcm_write_reg (reg, TEST_DBG_CTRL);

	/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~(0x3f<<10), 0<<10); */
	reg = dcm_read_reg (TEST_DBG_CTRL);
	dcm_clr_and_set_field (reg, 0x0, RG_CLK_DBGOUT_SEL);
	dcm_write_reg (reg, TEST_DBG_CTRL);

	/* *INFRA_AO_DBG_CON0 = aor(*INFRA_AO_DBG_CON0, ~(0x3f<<0), 0x20); */
	reg = dcm_read_reg (INFRA_AO_DBG_CON0);
	dcm_clr_and_set_field (reg, 0x1, DEBUG_PIN_SEL);
	dcm_clr_and_set_field (reg, TOP_CLOCK_CONTROL, INFRA_AO_DEBUG_MON0); /* fixme, pupa ignored this? */
	dcm_write_reg (reg, INFRA_AO_DBG_CON0);

	//- GPIO setting
	gpiodbg_monitor();
}


void gpiodbg_emi_dbg_out(void)
{
	UINT32 reg;

	/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~(1<<12), 1<<12); */
	reg = dcm_read_reg (TEST_DBG_CTRL);
	dcm_clr_and_set_field (reg, 0x1, RG_CLK_DBG_EN);
	dcm_write_reg (reg, TEST_DBG_CTRL);

	/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~(0x3<<10), 2<<10); */
	reg = dcm_read_reg (TEST_DBG_CTRL);
	dcm_clr_and_set_field (reg, 0x2, RG_CLK_DBGOUT_SEL);
	dcm_write_reg (reg, TEST_DBG_CTRL);

	/* *INFRA_AO_DBG_CON0 = aor(*INFRA_AO_DBG_CON0, ~(0x3f<<0), 0x20; */
	reg = dcm_read_reg (INFRA_AO_DBG_CON0);
	dcm_clr_and_set_field (reg, 0x1, DEBUG_PIN_SEL);
	dcm_clr_and_set_field (reg, TOP_CLOCK_CONTROL, INFRA_AO_DEBUG_MON0);	
	dcm_write_reg (reg, INFRA_AO_DBG_CON0);

	//- GPIO setting
	gpiodbg_monitor();
}


void gpiodbg_infra_dbg_out(void)
{
	UINT32 reg;

	/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~(1<<12), 1<<12); */
	reg = dcm_read_reg (TEST_DBG_CTRL);
	dcm_clr_and_set_field (reg, 0x1, RG_CLK_DBG_EN);
	dcm_write_reg (reg, TEST_DBG_CTRL);

	/* *INFRA_AO_DBG_CON0 = aor(*INFRA_AO_DBG_CON0, ~(0x3f<<0), 0x25); */
	reg = dcm_read_reg (INFRA_AO_DBG_CON0);
	dcm_clr_and_set_field (reg, 0x1, DEBUG_PIN_SEL);
	dcm_clr_and_set_field (reg, INFRASYS_GLOBAL_CON_LSB, INFRA_AO_DEBUG_MON0);
	dcm_write_reg (reg, INFRA_AO_DBG_CON0);

	//- GPIO setting
	gpiodbg_monitor();
}


void gpiodbg_peri_dbg_out(void)
{
	UINT32 reg;

	/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~(1<<12), 1<<12);*/
	reg = dcm_read_reg (TEST_DBG_CTRL);
	dcm_clr_and_set_field (reg, 0x1, RG_CLK_DBG_EN);
	dcm_write_reg (reg, TEST_DBG_CTRL);

	/* *INFRA_AO_DBG_CON0 = aor(*INFRA_AO_DBG_CON0, ~(0x3f<<0), 0x24); */
	reg = dcm_read_reg (INFRA_AO_DBG_CON0);
	dcm_clr_and_set_field (reg, 0x1, DEBUG_PIN_SEL);
	dcm_clr_and_set_field (reg, INFRASYS_GLOBAL_CON_MSB, INFRA_AO_DEBUG_MON0);
	dcm_write_reg (reg, INFRA_AO_DBG_CON0);

	//- GPIO setting
	gpiodbg_monitor();
}



/////////////////////////////////////////
//- freq meter

#define FQMTR_APMCU_CLOCK_PRE_DIV (4)  //
void freqm_reset (void)
{
	dcm_write_reg ((1 << RG_FQMTR_RST_BIT), FREQ_MTR_CTRL);
	dsb();
	dcm_write_reg (0, FREQ_MTR_CTRL);
}

INT32 freqm_getresult (FREQMETER_CTRL* ctl, const char* caller)
{
	UINT32	reg;
	INT32	status = FREQMETER_SUCCESS;

	FREQM_LOCK;

	//- check right
	if (ctl->owner != fm_last_owner)
	{
		status = FREQMETER_NOT_OWNER;
	}

	while (1)
	{
		reg = dcm_read_reg (FREQ_MTR_DATA);

		if (reg & (RG_FQMTR_BUSY_MASK << RG_FQMTR_BUSY_BIT))
		{
			if (!ctl->polling_to_getresult)
			{
				status = FREQMETER_COUNTING;
				goto err;
			}
		}
		else
		{
			ctl->result_in_count = ((reg & RG_FQMTR_DATA_MASK) >> RG_FQMTR_DATA_BIT);
			if (ctl->mon_sel == FQMTR_SRC_APMCU_CLOCK) {		
				ctl->result_in_count *= (FQMTR_APMCU_CLOCK_PRE_DIV);
			}
			break;
		}
	}

	fm_last_owner = 0;

	FREQM_UNLOCK;
	
	if (status == FREQMETER_NOT_OWNER)
	{
		dcm_warn("[WARN]=> FREQMETER_GET_COLLISION in %s(), older %s()\n", caller, fm_getresult_last_caller);	
	}

	strcpy(fm_getresult_last_caller, caller);

 err:
	return status;
}

INT32 freqm_kick (FREQMETER_CTRL* ctl, const char* caller)
{
	UINT32	reg;
	INT32	status = FREQMETER_SUCCESS;
	int saved_test_dbg_ctrl = 0 , saved_dbg_ctrl = 0;

	FREQM_LOCK;

	//- to get right
	if (fm_last_owner)
	{
		status = FREQMETER_NO_RESOURCE;		
	}
	fm_last_owner = fm_magic_owner ++;

	freqm_reset ();
	if (ctl->mon_sel == FQMTR_SRC_APMCU_CLOCK) {
		u32 reg;
		/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~0x0ff, 0x0ff); */
		saved_test_dbg_ctrl = reg = dcm_read_reg (TEST_DBG_CTRL);
		dcm_clr_and_set_field (reg, FQMTR_APMCU_CLOCK_PRE_DIV-1, RG_ARMCLK_K1);
		dcm_write_reg (reg, TEST_DBG_CTRL);

		/* *DBG_CTRL = aor(*DBG_CTRL, ~0x0ff, 7); */
		saved_dbg_ctrl = reg = dcm_read_reg (DBG_CTRL);
		dcm_clr_and_set_field (reg, 0x07, CA7_MON_SEL);
		dcm_write_reg (reg, DBG_CTRL);

		ctl->mon_len_in_ref_clk = 0x400;
	}
	//- fill field
	reg = ( ((ctl->divider & RG_FQMTR_CKDIV_MASK) << RG_FQMTR_CKDIV_BIT) |
			((ctl->ref_clk_sel & RG_FQMTR_FIXCLK_SEL_MASK) << RG_FQMTR_FIXCLK_SEL_BIT) |
			((ctl->mon_sel & RG_FQMTR_MONCLK_SEL_MASK) << RG_FQMTR_MONCLK_SEL_BIT) |
			((ctl->mon_len_in_ref_clk & RG_FQMTR_WINDOW_MASK) << RG_FQMTR_WINDOW_BIT) |
			 (1 << RG_FQMTR_EN_BIT)	);


	dcm_write_reg (reg, FREQ_MTR_CTRL);
	ctl->owner = fm_last_owner;

	if (ctl->mon_sel == FQMTR_SRC_APMCU_CLOCK) {		
		u32 reg;
		while (1) {
			reg = dcm_read_reg (FREQ_MTR_DATA);
			dmb();
			if ((reg & (RG_FQMTR_BUSY_MASK << RG_FQMTR_BUSY_BIT)) == 0)
				break;
		}

		/* *TEST_DBG_CTRL = aor(*TEST_DBG_CTRL, ~0x0ff, 0x0ff); */
		reg = saved_test_dbg_ctrl;
		dcm_write_reg (reg, TEST_DBG_CTRL);

		/* *DBG_CTRL = aor(*DBG_CTRL, ~0x0ff, 7); */
		reg = saved_dbg_ctrl;
		dcm_write_reg (reg, DBG_CTRL);
	}


	FREQM_UNLOCK;
	
	if (status == FREQMETER_NO_RESOURCE)
	{
		dcm_warn("[WARN]=> FREQMETER_KICK_COLLISION in %s(), older %s()\n", caller, fm_kick_last_caller);	
	}

	strcpy(fm_kick_last_caller, caller);

 	return status;

}



#ifndef __DCM_CTP__
static INT32 fqmtr_proc_sta_write (struct file *file, const char *buffer, unsigned long count, void *data)
{
	dcm_info("Enter: %s\n",__func__);

	return 0;
}

static INT32 fqmtr_proc_sta_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);

	p += sprintf(p, "\rstatus = %d\r\n", em_fqmtr_sta);

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;
}

static INT32 fqmtr_proc_dbg_write (struct file *file, const char *buffer, unsigned long count, void *data)
{
	INT32 		ret;
	char 		kbuf[256];
	UINT32	 	len = 0;
	UINT32		p1,p2,p3;

	dcm_info("Enter: %s\n",__func__);
	len = min(count, (typeof(count))(sizeof(kbuf)-1));
	if (count == 0)		return -1;
	if(count > 255)		count = 255;
	ret = copy_from_user(kbuf, buffer, count);
	if (ret < 0)		return -1;
	kbuf[count] = '\0';
	
	sscanf(kbuf, "%d %d %d", &p1, &p2, &p3);
	
	switch (p1)
	{
		case 1:
		{			

			em_fqmtr_request.divider 				= RG_FQMTR_CKDIV_D1;
			em_fqmtr_request.ref_clk_sel			= RG_FQMTR_FIXCLK_SEL_26MHZ;
			em_fqmtr_request.mon_sel 				= p2;
			em_fqmtr_request.mon_len_in_ref_clk		= (p3 > 157) ? 157*26 : p3*26;	
			em_fqmtr_request.polling_to_getresult	= 0;
			em_fqmtr_request.result_in_count		= 0;
			
			em_fqmtr_sta = freqmeter_kick (&em_fqmtr_request);
				
		} break;
		
		default:
			dcm_info("[WARR] : %s() Unsupported cpmmand!\n",__func__);
		break;
	}
	return count;
}

static INT32 fqmtr_proc_dbg_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;
	UINT32 freq;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);

	em_fqmtr_sta = freqmeter_getresult (&em_fqmtr_request);
	
	if (em_fqmtr_sta == FREQMETER_COUNTING)
	{
		p += sprintf(p, "FQMTR : %s = counting...\n", FQMTR_source_name[em_fqmtr_request.mon_sel]);
	}
	else if (em_fqmtr_sta == FREQMETER_SUCCESS)
	{
		if (em_fqmtr_request.result_in_count != FREQMETER_OVERFLOW)
		{
			freq = em_fqmtr_request.result_in_count * 1000 / (em_fqmtr_request.mon_len_in_ref_clk / 26);
			p += sprintf(p, "FQMTR : %s = %d kHz\n", FQMTR_source_name[em_fqmtr_request.mon_sel], freq);
		}
		else
		{
			p += sprintf(p, "FQMTR : %s = overflow!\n", FQMTR_source_name[em_fqmtr_request.mon_sel]);				
		}
	}
	else
	{
		p += sprintf(p, "%s() em_fqmtr_sta %d\n", __func__, em_fqmtr_sta);	
	}
	
	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;
}


static INT32 fqmtr_proc_help_write (struct file *file, const char *buffer, unsigned long count, void *data)
{
	dcm_info("Enter: %s\n",__func__);

	return 0;
}

static INT32 fqmtr_proc_help_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);

	p += sprintf(p, "\r\nhelp!!!\r\n");

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;

}

#define fqmtr_create_proc(fILENAME)															\
do {                                                                                        \
		struct proc_dir_entry *pEntry;		                                                \
		                                                                                    \
		pEntry = create_proc_entry(#fILENAME,  S_IRUGO | S_IWUSR | S_IWGRP, proc_dir); 		\
		if(pEntry)                                                                          \
		{                                                                                   \
			pEntry->read_proc  = fqmtr_proc_##fILENAME##_read;                              \
			pEntry->write_proc = fqmtr_proc_##fILENAME##_write;                             \
			dcm_info("[%s]: successfully create /proc/fqmtr/%s", __func__, #fILENAME);      \
		}else{                                                                              \
			dcm_info("[%s]: failed to create /proc/fqmtr/%s", __func__, #fILENAME);         \
		}																					\
} while (0)

static void mt_fqmtr_init (void)
{
	struct proc_dir_entry *proc_dir = NULL;

	//TODO: check the permission!!

	dcm_info("Enter: %s\n",__func__);

	proc_dir = proc_mkdir("fqmtr", NULL);
	if (!proc_dir)
	{
		dcm_err("[Err] proc_mkdir fqmtr fail!\n");
		return;
	}
	else
	{
		fqmtr_create_proc (sta);
		fqmtr_create_proc (dbg);
		fqmtr_create_proc (help);
	}
}
#endif //-__DCM_CTP__



/////////////////////////////////////////
//- LPM
void lpm_reset (void)
{
	volatile UINT32 reg;

	//- use 104MHz clock to reset (32KHz need to sync, spend to much time)
	enable_clock (MT_CG_UPLL_D12, "LPM 104MHz");
	
	dcm_write_reg (0, LPM_CTRL_REG);
	reg = dcm_read_reg (LPM_CTRL_REG);
	//- reset de-assert
	dcm_write_reg (1, LPM_CTRL_REG);
	reg = dcm_read_reg (LPM_CTRL_REG);
	
	disable_clock (MT_CG_UPLL_D12, "LPM 104MHz");

/*
	//- 104M
	//- reset assert
	dcm_write_reg (0, LPM_CTRL_REG);
	reg = dcm_read_reg (LPM_CTRL_REG);
	//- reset de-assert
	dcm_write_reg (1, LPM_CTRL_REG);
	reg = dcm_read_reg (LPM_CTRL_REG);
	
	//- 32K
	//- reset assert 32K
	dcm_write_reg (2, LPM_CTRL_REG);
	reg = dcm_read_reg (LPM_CTRL_REG);
	//- reset de-assert
	dcm_write_reg (3, LPM_CTRL_REG);
*/
}

INT32 lpm_getresult (LPM_CTRL* ctl, const char* caller)
{
	UINT32	reg, i;
	unsigned long flags;
	UINT32	overflow;
	INT32	status = LPM_SUCCESS;

	LPM_LOCK(flags);

	//- check right
	if (ctl->owner != lpm_last_owner)
	{	//- corrupt occur
		status = LPM_GET_COLLISION;
	}

	{	//- stop counting and get result
		reg = dcm_read_reg (LPM_CTRL_REG);
		dcm_clr_field (reg, RG_DBG_MON_START);
		dcm_write_reg (reg, LPM_CTRL_REG);

		overflow = dcm_read_reg (LPM_LONGEST_HIGHTIME) >> RG_TOTAL_TIME_OVERFLOW_BIT;
		for (i=0; i<NUM_OF_LPM_CNT_TYPE; i++)
		{
			if (overflow & (0x1<<i))
			{
				ctl->result[i] = LPM_COUNT_OVERFLOW_MASK;
			}
			else
			{
				ctl->result[i] = dcm_read_reg (LPM_CNTx_BASE(i)) & lpm_cnt_mask[i];
			}
		}
	}

	if (ctl->ref_clk_sel == LPM_REF_CLOCK_104M)
	{	//- disable 104M clock
		disable_clock (MT_CG_UPLL_D12, "LPM 104MHz");
	}

	lpm_last_owner = 0;

	LPM_UNLOCK(flags);
	
	if (status == LPM_GET_COLLISION)
	{
		dcm_warn("[WARN]=> LPM_GET_COLLISION in %s(), older %s()\n", caller, lpm_getresult_last_caller);
	}

	strcpy(lpm_getresult_last_caller, caller);

	return status;
}

INT32 lpm_kick (LPM_CTRL* ctl, const char* caller)
{
	UINT32	reg;
	unsigned long flags;
	INT32	status = LPM_SUCCESS;

	LPM_LOCK(flags);

	//- to get right
	if (lpm_last_owner)
	{
		status = LPM_KICK_COLLISION;		
	}
	lpm_last_owner = lpm_magic_owner ++;	

	if (ctl->ref_clk_sel == LPM_REF_CLOCK_104M)
	{	//- prepare LPM param and disable 104M clock CG
		enable_clock (MT_CG_UPLL_D12, "LPM 104MHz");
	}

	lpm_reset ();

	//- fill field
	reg = ( ((ctl->ref_clk_sel & RG_32K_CK_SEL_MASK) << RG_32K_CK_SEL_BIT) |
			((ctl->mon_sel & RG_MON_SRC_SEL_MASK) << RG_MON_SRC_SEL_BIT) |
			((ctl->good_duration_criteria & RG_GOOD_DUR_CRITERIA_MASK) << RG_GOOD_DUR_CRITERIA_BIT) |
			 (1 << RG_DBG_MON_START_BIT) |
			 (1 << RG_SOFT_RSTB_BIT)
		  );


	dcm_write_reg (reg, LPM_CTRL_REG);
	ctl->owner = lpm_last_owner;

	LPM_UNLOCK(flags);
	
	if (status == LPM_KICK_COLLISION)
	{
		dcm_warn("[WARN]=> LPM_KICK_COLLISION caller in %s(), older %s()\n", caller, lpm_kick_last_caller);	
	}
	
	strcpy(lpm_kick_last_caller, caller);
	
 	return status;

}

#ifndef __DCM_CTP__
static INT32 lpm_proc_sta_write (struct file *file, const char *buffer, unsigned long count, void *data)
{
	dcm_info("Enter: %s\n",__func__);

	return 0;
}

static INT32 lpm_proc_sta_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{	
	char *p = page;
	UINT32 len = 0;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);

	p += sprintf(p, "\rstatus = %d\r\n", em_lpm_sta);

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;
}

static INT32 lpm_proc_dbg_write (struct file *file, const char *buffer, unsigned long count, void *data)
{
	INT32 		ret;
	char 		kbuf[256];
	UINT32	 	len = 0;
	UINT32		p1,p2,p3,p4,p5,p6,p7;

	dcm_info("Enter: %s\n",__func__);
	len = min(count, (typeof(count))(sizeof(kbuf)-1));
	if (count == 0)		return -1;
	if(count > 255)		count = 255;
	ret = copy_from_user(kbuf, buffer, count);
	if (ret < 0)		return -1;
	kbuf[count] = '\0';
	
	sscanf(kbuf, "%x", &p1);
	
	switch (p1)
	{
		case 0:
		{	//- switch cat /proc/lpm/dbg
			sscanf(kbuf, "%x %x", &p1, &p2);			
			lpm_cat_type = p2;
		} break;
		
		case 1:
		{	//- manual measurement
			sscanf(kbuf, "%x %x %x %x %x", &p1, &p2, &p3, &p4, &p5);
		
			if (p2 == 0)
			{	//- stop
				em_lpm_sta = LPM_getresult (&em_lpm_request);
			}
			else if (p2 == 1)
			{	//- start
				em_lpm_request.ref_clk_sel 				= p3;
				em_lpm_request.mon_sel 					= p4;
				em_lpm_request.good_duration_criteria 	= p5;
				em_lpm_sta = LPM_kick (&em_lpm_request);
			}
			else
			{	//- error
				em_lpm_sta = LPM_EM_OPCODE_ERR;
			}
		} break;
		
		case 2:
		{	//- periodic measurement
			sscanf(kbuf, "%x %x %x %x %x %d %x", &p1, &p2, &p3, &p4, &p5, &p6, &p7);
			
			lpm_timer_data.mode = p2;
			
			switch (p2)
			{				
				case 0: 
				{	//- stop
					em_lpm_sta = LPM_SUCCESS;				
				} break;
				case 1:	//- one shot
				case 2:	//- periodic
				{	
					lpm_timer_data.ctrl.ref_clk_sel 				= p3;
					lpm_timer_data.ctrl.mon_sel 					= p4;
					lpm_timer_data.ctrl.good_duration_criteria 		= p5;
					lpm_timer_data.period_in_s						= p6;
					lpm_timer_data.log_mode							= p7;
					
					if (!lpm_timer_data.init)
					{
						lpm_timer.expires = jiffies +  HZ * p6;
						add_timer(&lpm_timer);
						lpm_timer_data.init = 1;
					}
					
					mod_timer(&lpm_timer, jiffies +  HZ * p6);
					
					em_lpm_sta = LPM_kick (&(lpm_timer_data.ctrl));
				}		
				default :
				{	//- error
					em_lpm_sta = LPM_EM_OPCODE_ERR;
				}						
			}
		} break;
		
		case 3:
		{
			sscanf(kbuf, "%x %x %d %d %d", &p1, &p2, &p3, &p4, &p5);

			switch (p2)
			{
				case 0:
				{	//- clear
					lpm_timer_data.threshold_check &= (~(1 << p3));
					lpm_timer_data.sta_overflow[p3] = 0;
					lpm_timer_data.sta_max[p3] 		= 0;
					lpm_timer_data.sta_min[p3] 		= 100;
				} break;
				case 1:
				{	//- set
					lpm_timer_data.threshold_check |= (1 << p3);
				} break;
				case 2:
				{	//- change parameter
					lpm_timer_data.threshold[p3][0] = p4;
					lpm_timer_data.threshold[p3][1] = p5;					
				} break;
			
				default:
				break;				
			}	
		} break;
		default:
			dcm_info("[WARR] : %s() Unsupported cpmmand!\n",__func__);
		break;
	}
	return count;
}

static INT32 lpm_proc_dbg_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;
	UINT32 i, ison;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);

	switch (lpm_cat_type)
	{
		case 1:
		{
			for (i=0; i<NUM_OF_LPM_CNT_TYPE; i++)
			{
				if (em_lpm_request.result[i] & LPM_COUNT_OVERFLOW_MASK)
				{	//- overflow
					p += sprintf(p, "Overflow, ");
				}
				else
				{
					p += sprintf(p, "%d, ", em_lpm_request.result[i]);
				}
			}
			p += sprintf(p, "\n");
		} break;
		case 2:
		{	
			p += sprintf(p, "lpm mode %s\n", LPM_mode_string[lpm_timer_data.mode]);
		} break;
		case 3:
		{	
			for (i=1; i<NUM_OF_LPM_CNT_TYPE; i++)
			{
				ison = !!(lpm_timer_data.threshold_check & (1 << i));
				p += sprintf(p, "%s\t%s\t%c\t%d%%\t%d%%\t%d\t%d\t%d\n", LPM_source_name[lpm_timer_data.ctrl.mon_sel],
																		LPM_getname[i],
																		LPM_onoff_string[ison],
																		lpm_timer_data.threshold[i][0],
																		lpm_timer_data.threshold[i][1],
																		lpm_timer_data.sta_overflow[i],
																		lpm_timer_data.sta_max[i],
																		lpm_timer_data.sta_min[i]);
			}
		} break;		
		
		default:
			p += sprintf(p, "Invalid lpm_cat_type %d\n", lpm_cat_type);
		break;
	}	
	
	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;
}


static INT32 lpm_proc_help_write (struct file *file, const char *buffer, unsigned long count, void *data)
{
	dcm_info("Enter: %s\n",__func__);

	return 0;
}

static INT32 lpm_proc_help_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	dcm_info("Enter: %s\n",__func__);

	p += sprintf(p, "\r\nhelp!!!\r\n");

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;

}

#define lpm_create_proc(fILENAME)															\
do {                                                                                        \
		struct proc_dir_entry *pEntry;		                                                \
		                                                                                    \
		pEntry = create_proc_entry(#fILENAME,  S_IRUGO | S_IWUSR | S_IWGRP, proc_dir); 		\
		if(pEntry)                                                                          \
		{                                                                                   \
			pEntry->read_proc  = lpm_proc_##fILENAME##_read;                               	\
			pEntry->write_proc = lpm_proc_##fILENAME##_write;                               \
			dcm_info("[%s]: successfully create /proc/lpm/%s", __func__, #fILENAME);        \
		}else{                                                                              \
			dcm_info("[%s]: failed to create /proc/lpm/%s", __func__, #fILENAME);           \
		}																					\
} while (0)

static void mt_lpm_init (void)
{
	struct proc_dir_entry *proc_dir = NULL;
	UINT32 i;

	//TODO: check the permission!!

	dcm_info("Enter: %s\n",__func__);

	proc_dir = proc_mkdir("lpm", NULL);
	if (!proc_dir)
	{
		dcm_err("[Err] proc_mkdir lpm fail!\n");
		return;
	}
	else
	{
		lpm_create_proc (sta);
		lpm_create_proc (dbg);
		lpm_create_proc (help);
	}
	
	init_timer(&lpm_timer);
	lpm_timer.expires = jiffies + 5 * HZ;	//- 1s
	lpm_timer.function = &lpm_timer_callback;
	lpm_timer.data = (UINT32) &lpm_timer_data;
	memset(&lpm_timer_data, 0, sizeof(lpm_timer_data));
	for(i=LPM_LOW2HIGH_COUNT_TYPE; i<NUM_OF_LPM_CNT_TYPE; i++)
	{
		lpm_timer_data.sta_min[i] = 100;
	}
	
	#ifdef LPM_MET_ENABLE
	met_ext_dev_add(met_lpm_device);
	#endif //- LPM_MET_ENABLE
}

static void lpm_timer_callback(unsigned long param)
{
	UINT32 i, denominator, tmp, tt;
//	LPM_TIMER_DATA *dp = (LPM_TIMER_DATA*) param;
   	
   	em_lpm_sta = LPM_getresult (&(lpm_timer_data.ctrl));
	if (lpm_timer_data.mode != 0)
	{		
		if (lpm_timer_data.log_mode	& 0x01)
		{
		    dcm_info("%s(): %s\t%d,\t%d,\t%d,\t%d,\t%d\n",	__FUNCTION__,	
		    												LPM_source_name[lpm_timer_data.ctrl.mon_sel],
				    										lpm_timer_data.ctrl.result[LPM_TOTAL_TIME_TYPE],
															lpm_timer_data.ctrl.result[LPM_LOW2HIGH_COUNT_TYPE],
															lpm_timer_data.ctrl.result[LPM_HIGH_DUR_TIME_TYPE],
															lpm_timer_data.ctrl.result[LPM_LONGEST_HIGHTIME_TYPE],
															lpm_timer_data.ctrl.result[LPM_GOODDUR_COUNT_TYPE]);
		}
		
		if (lpm_timer_data.threshold_check)					
		{
			if (lpm_timer_data.ctrl.result[LPM_TOTAL_TIME_TYPE] & LPM_COUNT_OVERFLOW_MASK)
			{
				lpm_timer_data.sta_overflow[LPM_TOTAL_TIME_TYPE] ++;
			}
			else
			{
				tt = lpm_timer_data.ctrl.result[LPM_TOTAL_TIME_TYPE];
				denominator = tt / 100;
				//-dcm_info("%s():	%x\n", __FUNCTION__, lpm_timer_data.threshold_check);
				for(i=LPM_LOW2HIGH_COUNT_TYPE; i<NUM_OF_LPM_CNT_TYPE; i++)
				{
					if ((1 << i) & lpm_timer_data.threshold_check)
					{	//- need check
						if (lpm_timer_data.ctrl.result[i] & LPM_COUNT_OVERFLOW_MASK)
						{	//- overflow
							lpm_timer_data.sta_overflow[i] ++;
						}
						else
						{							
							if (i != LPM_GOODDUR_COUNT_TYPE)
							{
								tmp = lpm_timer_data.ctrl.result[i];							
							}
							else
							{
								tmp = lpm_timer_data.ctrl.result[i] * lpm_timer_data.ctrl.good_duration_criteria;
							}
							
							
							if (tmp*100 > tmp)
							{
								tmp = tmp * 100 / tt;
							}
							else
							{	//- overflow
								tmp = tmp / denominator;
							}

							
							if (lpm_timer_data.log_mode	& 0x04)
							{							
								if (tmp > lpm_timer_data.threshold[i][0])
								{
									dcm_info("%s() : %s\t%d%% > %d%%\n", __FUNCTION__, LPM_getname[i], tmp, lpm_timer_data.threshold[i][0]);
								}
								if (tmp < lpm_timer_data.threshold[i][1])
								{
									dcm_info("%s() : %s\t%d%% < %d%%\n", __FUNCTION__, LPM_getname[i], tmp, lpm_timer_data.threshold[i][1]);
								}
							}
							if (tmp > lpm_timer_data.sta_max[i])	lpm_timer_data.sta_max[i] = tmp;
							if (tmp < lpm_timer_data.sta_min[i])	lpm_timer_data.sta_min[i] = tmp;
						}
					}
				}
				
				for (i=LPM_LOW2HIGH_COUNT_TYPE; i<NUM_OF_LPM_CNT_TYPE; i++)
				{
					if (lpm_timer_data.log_mode	& 0x02)
					{
						dcm_info("%s() : %s\t\t%d\t%d%%\t%d%%\n", __FUNCTION__,	LPM_getname[i],	
																			lpm_timer_data.sta_overflow[i],
																			lpm_timer_data.sta_max[i],
																			lpm_timer_data.sta_min[i]);
					}
	
				}
			}		
		}
	}
	
	switch (lpm_timer_data.mode)
	{
		case 2:
		em_lpm_sta = LPM_kick (&(lpm_timer_data.ctrl));
		mod_timer(&lpm_timer, jiffies +  HZ * lpm_timer_data.period_in_s);
		break;

		default:
		lpm_timer_data.mode = 0;
		break;
	}
}

#ifdef LPM_MET_ENABLE

void LowPowerMonitor(void)
{	
	UINT32	i, cpu, tt, nano_rem, denominator, tmp;
	unsigned long long stamp;
	UINT32	data[NUM_OF_LPM_CNT_TYPE][3]; //- raw, overflow, percent
	INT32	status;

	cpu = smp_processor_id();
	stamp = cpu_clock(cpu);
	nano_rem = do_div(stamp, 1000000000);

	memset (data, 0, 3*NUM_OF_LPM_CNT_TYPE*sizeof(UINT32));
	
	{	
		status = LPM_getresult(&met_lpm_ctrl.ctrl);
		if (status == LPM_SUCCESS)
		{						
			tt = met_lpm_ctrl.ctrl.result[LPM_TOTAL_TIME_TYPE];
			if (tt && (!(tt & LPM_COUNT_OVERFLOW_MASK)))
			{				
				denominator = tt / 100;
				for (i=0; i<NUM_OF_LPM_CNT_TYPE; i++)
				{
					if (!(met_lpm_ctrl.ctrl.result[i] & LPM_COUNT_OVERFLOW_MASK))
					{			
						data[i][0] = met_lpm_ctrl.ctrl.result[i];					
					}
					else
					{
						data[i][1] = 1;
					}
					tmp = data[i][0];
					if (i == LPM_GOODDUR_COUNT_TYPE)
					{
						tmp *= met_lpm_ctrl.ctrl.good_duration_criteria;
					}
					
					if (tmp*100 > tmp)
					{					
						data[i][2] = tmp * 100 / tt;
					}
					else
					{	//- overflow
						data[i][2] = tmp / denominator;
					}
				}
			}
		}
		trace_printk("%5u.%06u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", (UINT32)(stamp), nano_rem/1000, (0-status),
																				data[0][0],data[1][0],data[2][0],data[3][0],data[4][0],
																							data[1][1],data[2][1],data[3][1],data[4][1],
																							data[1][2],data[2][2],data[3][2],data[4][2]																				
																				);
	}
	
	LPM_kick (&met_lpm_ctrl.ctrl);
}

//It will be called back when run "met-cmd --start"
static void met_lpm_start(void)
{
	dcm_info("[Info] %s\n", __func__);
	
	LPM_kick (&met_lpm_ctrl.ctrl);
	
	met_lpm_ctrl.init = 1;
	
	return;
}

//It will be called back when run "met-cmd --stop"
static void met_lpm_stop(void)
{
	dcm_info("[Info] %s\n", __func__);
	
	if (met_lpm_ctrl.init)
	{
		LPM_getresult(&met_lpm_ctrl.ctrl);
		
		met_lpm_ctrl.init = 0;
	}

	return;
}

//This callback function was called from timer. It cannot sleep or wait for
//others. Use wrokqueue to do the waiting
static void met_lpm_polling(unsigned long long stamp, int cpu)
{
	LowPowerMonitor ();
}

static char met_lpm_header[] =
"#met-info [000] 0.0: LowPowerMonitor:type=%d:%d:%d:%x (interval_ms, ref_clk, mon_sel, good_duration)\n"
"met-info [000] 0.0: ms_ud_sys_header:LowPowerMonitor,timestamp,"
"Pollution,Total_time,Low_to_high,Total_high,Longest_high,Good_duration,Low_to_high(ov),Total_high(ov),Longest_high(ov),Good_duration(ov),Low_to_high(%%),Total_high(%%),Longest_high(%%),Good_duration(%%), d,d,d,d,d,d,d,d,d,d,d,d,d,d\n";
static char help[] = "  --lpm=interval_ms(d),ref_clk(d),mon_sel(d),good_duration(x)                 monitor lpm\n";


//It will be called back when run "met-cmd -h"
static int met_lpm_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

//It will be called back when run "met-cmd --extract" and mode is 1
static int met_lpm_print_header(char *buf, int len)
{
	met_lpm_device->mode = 0;
	return snprintf(buf, PAGE_SIZE, met_lpm_header, met_lpm_device->polling_interval,
													met_lpm_ctrl.ctrl.ref_clk_sel,
													met_lpm_ctrl.ctrl.mon_sel,
													met_lpm_ctrl.ctrl.good_duration_criteria);
}

//It will be called back when run "met-cmd --start --lpm aaa=1"
//and arg is "aaa=1"
static int met_lpm_process_argument(const char *arg, int len)
{
	UINT32 p1;
	

	sscanf(arg, "%d,%d,%d,%x",	&met_lpm_device->polling_interval,
								&met_lpm_ctrl.ctrl.ref_clk_sel,
								&met_lpm_ctrl.ctrl.mon_sel,
								&met_lpm_ctrl.ctrl.good_duration_criteria);
	
	dcm_info("====MET LPM Argument(len=%d):%s %d,%d,%d,%x\n", len, arg, met_lpm_device->polling_interval,
												met_lpm_ctrl.ctrl.ref_clk_sel,
												met_lpm_ctrl.ctrl.mon_sel,
												met_lpm_ctrl.ctrl.good_duration_criteria);
												
	met_lpm_device->mode = 1;
	return 0;
	#ifdef LPM_MET_ENABLE
	met_ext_dev_add(met_lpm_device);
	#endif //- LPM_MET_ENABLE
}


struct metdevice met_lpm_device[] = {
	{
		.name = "lpm",
		.owner = THIS_MODULE,
		.type = MET_TYPE_BUS,
		.cpu_related = 0,
		.start = met_lpm_start,
		.stop = met_lpm_stop,
		.polling_interval = 10,//ms
		.timed_polling = met_lpm_polling,
		.tagged_polling = met_lpm_polling,
		.print_help = met_lpm_print_help,
		.print_header = met_lpm_print_header,
		.process_argument = met_lpm_process_argument
	}
};

EXPORT_SYMBOL(met_lpm_device);
#endif //- LPM_MET_ENABLE
#endif //- !__DCM_CTP__
#endif //- __MT_DCM_C__
