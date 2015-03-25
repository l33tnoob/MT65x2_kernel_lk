#ifndef __MT_DCM_H__
#define __MT_DCM_H__

#include "mach/mt_reg_base.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_cpufreq.h"

#define MT_APDCM_DEFAULT_ON

/////////////////////////////////////////////////////////////////////////
//-
//-		HW SECTION
//-
#ifndef TOPCKGEN_BASE
#define TOPCKGEN_BASE				0x10000000
#endif

#ifndef INFRA_SYS_CFG_AO_BASE
#define INFRA_SYS_CFG_AO_BASE		0x10001000
#endif

#ifndef MCUSYS_CFGREG_BASE
#define MCUSYS_CFGREG_BASE			0x10200000
#endif

#ifndef MMSYS_CONFIG_BASE
#define MMSYS_CONFIG_BASE			0x14000000
#endif

#ifndef GPIO_BASE
#define GPIO_BASE					0x10005000
#endif

#ifndef CAM_BASE
#define CAM_BASE					0x14013000
#endif

#ifndef VENC_BASE
#define VENC_BASE					0x14016000
#endif

#ifndef VDEC_BASE
#define VDEC_BASE					0x14017000
#endif

#ifndef G3D_CONFIG_BASE
#define G3D_CONFIG_BASE				0x13000000
#endif

#define DCM_TOP_CKDIV1				(INFRA_SYS_CFG_AO_BASE + 0x0008)
#define TOP_DCMCTL				(INFRA_SYS_CFG_AO_BASE + 0x0010)
#define TOP_DCMDBC				(INFRA_SYS_CFG_AO_BASE + 0x0014)

#define ACLKEN_DIV				(MCUSYS_CFGREG_BASE + 0x0060)
#define CA7_MISC_CONFIG			(MCUSYS_CFGREG_BASE + 0x005C)
#define CA7_CACHE_CONFIG		(MCUSYS_CFGREG_BASE + 0x0000)

#define TOPBUS_DCMCTL			(TOPCKGEN_BASE + 0x0008)

#define TOPEMI_DCMCTL			(TOPCKGEN_BASE + 0x000C)

#define DCM_INFRABUS_DCMCTL0    	(TOPCKGEN_BASE + 0x0028)
#define DCM_INFRABUS_DCMCTL1    	(TOPCKGEN_BASE + 0x002C)
#define SET_DCM_INFRABUS_DCMCTL0	(TOPCKGEN_BASE + 0x0058)
#define SET_DCM_INFRABUS_DCMCTL1    (TOPCKGEN_BASE + 0x005C)
#define CLR_DCM_INFRABUS_DCMCTL0    (TOPCKGEN_BASE + 0x0088)
#define CLR_DCM_INFRABUS_DCMCTL1    (TOPCKGEN_BASE + 0x008C)

#define MMSYS_HW_DCM_DIS0		(MMSYS_CONFIG_BASE + 0x0120)
#define MMSYS_HW_DCM_DIS_SET0	(MMSYS_CONFIG_BASE + 0x0124)
#define MMSYS_HW_DCM_DIS_CLR0   (MMSYS_CONFIG_BASE + 0x0128)
#define MMSYS_HW_DCM_DIS1       (MMSYS_CONFIG_BASE + 0x012C)
#define MMSYS_HW_DCM_DIS_SET1   (MMSYS_CONFIG_BASE + 0x0130)
#define MMSYS_HW_DCM_DIS_CLR1	(MMSYS_CONFIG_BASE + 0x0134)

#define MMSYS_VENC_MP4_DCM_CTRL	(VENC_BASE + 0x06F0)

#define MMSYS_VDEC_DCM_STA		(VDEC_BASE + 0x00CC)
#define MMSYS_VDEC_DCM_DIS		(VDEC_BASE + 0x00D0)

#define MMSYS_CAM_DCM_DIS		(CAM_BASE + 0x0080)
#define MMSYS_CAM_DCM_STATUS	(CAM_BASE + 0x0084)

#define MFG_DCM_CON_0			(G3D_CONFIG_BASE + 0x0010)

#define NUM_OF_DCM_REGS			18
/////////////////////////////////////////
//- arm core dcm
#define CLKDIV1_SEL_BIT				0
#define CLKDIV1_SEL_MASK			0x1F

#define ARM_DCM_WFE_ENABLE_BIT		2
#define ARM_DCM_WFI_ENABLE_BIT		1
#define ARM_DCM_WFE_ENABLE_MASK		0x1
#define ARM_DCM_WFI_ENABLE_MASK		0x1

#define TOPCKGEN_DCM_DBC_CNT_BIT	0
#define TOPCKGEN_DCM_DBC_CNT_MASK	0x1

/////////////////////////////////////////
//- arm l2bus dcm
#define AXI_DIV_SEL_BIT				0
#define AXI_DIV_SEL_MASK			0x1F

#define L2_BUS_DCM_EN_BIT			9
#define L2_BUS_DCM_EN_MASK			0x1

#define L2C_SRAM_MCU_DCM_EN_BIT		8
#define L2C_SRAM_MCU_DCM_EN_MASK	0x1

/////////////////////////////////////////
//- top bus dcm
#define RG_BUSDCM_FORCE_OFF_BIT		26
#define RG_BUSDCM_FULL_SEL_BIT		16
#define RG_BUSDCM_FORCE_ON_BIT		6
#define	RG_BUSDCM_APB_SEL_BIT		1
#define RG_BUSDCM_APB_TOG_BIT		0

#define RG_BUSDCM_FORCE_OFF_MASK	0x1
#define RG_BUSDCM_FULL_SEL_MASK		0x1F
#define RG_BUSDCM_FORCE_ON_MASK		0x1
#define	RG_BUSDCM_APB_SEL_MASK		0x1F
#define RG_BUSDCM_APB_TOG_MASK		0x1

/////////////////////////////////////////
//- emi bus dcm
#define RG_EMIDCM_FORCE_OFF_BIT		26
#define RG_EMIDCM_IDLE_FSEL_BIT		21
#define RG_EMIDCM_FULL_SEL_BIT		16
#define RG_EMIDCM_DBC_CNT_BIT		9
#define RG_EMIDCM_DBC_ENABLE_BIT	8
#define RG_EMIDCM_ENABLE_BIT		7
#define RG_EMIDCM_FORCE_ON_BIT		6
#define RG_EMIDCM_APB_SEL_BIT		1
#define RG_EMIDCM_APB_TOG_BIT		0

#define RG_EMIDCM_FORCE_OFF_MASK	0x1
#define RG_EMIDCM_IDLE_FSEL_MASK	0x1F
#define RG_EMIDCM_FULL_SEL_MASK		0x1F
#define RG_EMIDCM_DBC_CNT_MASK		0x7F
#define RG_EMIDCM_DBC_ENABLE_MASK	0x1
#define RG_EMIDCM_ENABLE_MASK		0x1
#define RG_EMIDCM_FORCE_ON_MASK		0x1
#define RG_EMIDCM_APB_SEL_MASK		0x1F
#define RG_EMIDCM_APB_TOG_MASK		0x1

/////////////////////////////////////////
//- infra/peri bus dcm
#define RG_PERIDCM_FORCE_ON_BIT			31
#define RG_PERIDCM_FORCE_CLKOFF_BIT		29
#define RG_PERIDCM_FORCE_CLKSLW_BIT		28
#define RG_PERIDCM_CLKOFF_EN_BIT		27
#define RG_PERIDCM_CLKSLW_EN_BIT		26
#define RG_PERIDCM_SFSEL_BIT			21
#define RG_PERIDCM_FSEL_BIT				16
#define RG_INFRADCM_FORCE_ON_BIT        15
#define RG_INFRADCM_FORCE_CLKOFF_BIT    13
#define RG_INFRADCM_FORCE_CLKSLW_BIT    12
#define RG_INFRADCM_CLKOFF_EN_BIT       11
#define RG_INFRADCM_CLKSLW_EN_BIT       10
#define RG_INFRADCM_SFSEL_BIT           5
#define RG_INFRADCM_FSEL_BIT            0

#define RG_PERIDCM_FORCE_ON_MASK      	0x1
#define RG_PERIDCM_FORCE_CLKOFF_MASK    0x1
#define RG_PERIDCM_FORCE_CLKSLW_MASK    0x1
#define RG_PERIDCM_CLKOFF_EN_MASK       0x1
#define RG_PERIDCM_CLKSLW_EN_MASK       0x1
#define RG_PERIDCM_SFSEL_MASK           0x1F
#define RG_PERIDCM_FSEL_MASK          	0x1F
#define RG_INFRADCM_FORCE_ON_MASK       0x1
#define RG_INFRADCM_FORCE_CLKOFF_MASK   0x1
#define RG_INFRADCM_FORCE_CLKSLW_MASK   0x1
#define RG_INFRADCM_CLKOFF_EN_MASK      0x1
#define RG_INFRADCM_CLKSLW_EN_MASK      0x1
#define RG_INFRADCM_SFSEL_MASK          0x1F
#define RG_INFRADCM_FSEL_MASK           0x1F


#define RG_BUSDCM_PLLCK_SEL_BIT			31
#define RG_MEMPSRV_DIS_BIT				30
#define RG_PERIDCM_DEBOUNCE_CNT_BIT		15
#define RG_PERIDCM_DEBOUNCE_EN_BIT		14
#define RG_INFRADCM_DEBOUNCE_CNT_BIT	9
#define RG_INFRADCM_DEBOUNCE_EN_BIT		8
#define RG_PMIC_SFSEL_BIT				3
#define RG_PMIC_SPICLKDCM_EN_BIT		2
#define RG_PMIC_BCLKDCM_EN_BIT			1
#define RG_USBDCM_EN_BIT				0

#define RG_BUSDCM_PLLCK_SEL_MASK 		0x1
#define RG_MEMPSRV_DIS_MASK          	0x1
#define RG_PERIDCM_DEBOUNCE_CNT_MASK  	0x1F
#define RG_PERIDCM_DEBOUNCE_EN_MASK   	0x1
#define RG_INFRADCM_DEBOUNCE_CNT_MASK  	0x1F
#define RG_INFRADCM_DEBOUNCE_EN_MASK   	0x1
#define RG_PMIC_SFSEL_MASK            	0x1F
#define RG_PMIC_SPICLKDCM_EN_MASK    	0x1
#define RG_PMIC_BCLKDCM_EN_MASK      	0x1
#define RG_USBDCM_EN_MASK 				0x1

/////////////////////////////////////////
//- mmsys dcm
#define RG_SMI_COMMON_BIT				0
#define RG_SMI_LARB0_BIT				1
#define RG_MM_CMDQ_BIT					2
#define RG_MUTEX_BIT					3
#define RG_DISP_COLOR_BIT				4
#define RG_DISP_BLS_BIT					5
#define RG_DISP_WDMA_BIT				6
#define RG_DISP_RDMA_BIT				7
#define RG_DISP_OVL_BIT					8
#define RG_MDP_TDSHP_BIT				9
#define RG_MDP_WROT_BIT					10
#define RG_MDP_WDMA_BIT                 11
#define RG_MDP_RSZ1_BIT                 12
#define RG_MDP_RSZ0_BIT                 13
#define RG_MDP_RDMA_BIT                 14
#define RG_DSI_ENGINE_BIT               15
#define RG_DISP_DBI_ENGINE_BIT          16
#define RG_DISP_DBI_SMI_BIT             17

#define RG_SMI_COMMON_MASK				0x1
#define RG_SMI_LARB0_MASK               0x1
#define RG_MM_CMDQ_MASK                 0x1
#define RG_MUTEX_MASK                   0x1
#define RG_DISP_COLOR_MASK              0x1
#define RG_DISP_BLS_MASK                0x1
#define RG_DISP_WDMA_MASK               0x1
#define RG_DISP_RDMA_MASK               0x1
#define RG_DISP_OVL_MASK                0x1
#define RG_MDP_TDSHP_MASK               0x1
#define RG_MDP_WROT_MASK                0x1
#define RG_MDP_WDMA_MASK                0x1
#define RG_MDP_RSZ1_MASK                0x1
#define RG_MDP_RSZ0_MASK                0x1
#define RG_MDP_RDMA_MASK                0x1
#define RG_DSI_ENGINE_MASK              0x1
#define RG_DISP_DBI_ENGINE_MASK         0x1
#define RG_DISP_DBI_SMI_MASK

#define RG_DSI_DIGITAL_BIT				0
#define RG_DSI_DIGITAL_MASK				0x1

//- venc
#define RG_DISABLE_VENC_MP4_CLOCK_DCM_BIT	0
#define RG_DISABLE_VENC_MP4_CLOCK_DCM_MASK	0x1

//-vdec
#define	RG_VDEC_DCMDIS_BIT				0
#define	RG_VDEC_DCMDIS_MASK				0x1

#define IDMA_DCM_STATUS_BIT				1
#define VDEC_DCM_STATUS_BIT				0

#define IDMA_DCM_STATUS_MASK			0x1
#define VDEC_DCM_STATUS_MASK			0x1

//- cam
#define RG_IMGO_DCM_DIS_BIT				1
#define RG_CDRZ_DCM_DIS_BIT				0

#define RG_IMGO_DCM_DIS_MASK			0x1
#define RG_CDRZ_DCM_DIS_MASK			0x1

#define RG_IMGO_DCM_ST_BIT				1
#define RG_CDRZ_DCM_ST_BIT				0

#define RG_IMGO_DCM_ST					0x1
#define RG_CDRZ_DCM_ST_MASK				0x1



/////////////////////////////////////////
//- mfgsys dcm
#define RG_BG3D_DCM_EN_BIT				15
#define RG_BG3D_DBC_EN_BIT				14
#define RG_BG3D_FSEL_BIT				8
#define RG_BG3D_DBC_CNT_BIT				0

#define RG_BG3D_DCM_EN_MASK				0x1
#define RG_BG3D_DBC_EN_MASK				0x1
#define RG_BG3D_FSEL_MASK				0x3F
#define RG_BG3D_DBC_CNT_MASK			0x7F



/////////////////////////////////////////
//- freqmeter
#define FREQ_MTR_CTRL			(TOPCKGEN_BASE + 0x0010)
#define FREQ_MTR_DATA			(TOPCKGEN_BASE + 0x0014)

#define RG_FQMTR_CKDIV_BIT			28
#define RG_FQMTR_FIXCLK_SEL_BIT		24
#define RG_FQMTR_MONCLK_SEL_BIT		16
#define RG_FQMTR_EN_BIT				15
#define RG_FQMTR_RST_BIT			14
#define RG_FQMTR_WINDOW_BIT			0

#define RG_FQMTR_CKDIV_MASK			0x3
#define RG_FQMTR_FIXCLK_SEL_MASK	0x3
#define RG_FQMTR_MONCLK_SEL_MASK	0x1F
#define RG_FQMTR_EN_MASK			0x1
#define RG_FQMTR_RST_MASK			0x1
#define RG_FQMTR_WINDOW_MASK		0xFFF

#define RG_FQMTR_BUSY_BIT			31
#define RG_FQMTR_DATA_BIT			0

#define RG_FQMTR_BUSY_MASK			0x1
#define RG_FQMTR_DATA_MASK			0xFFFF




/////////////////////////////////////////
//- debug ctrl
#define TEST_DBG_CTRL       			(TOPCKGEN_BASE + 0x0038)
#define SET_TEST_DBG_CTRL      			(TOPCKGEN_BASE + 0x0068)
#define CLR_TEST_DBG_CTRL      			(TOPCKGEN_BASE + 0x0098)

#define GPIO_AUX_MODE0					(GPIO_BASE + 0x0300)
#define GPIO_AUX_MODE(x)				(GPIO_AUX_MODE0 + (0x10 * x))

#define DBG_CTRL						(MCUSYS_CFGREG_BASE + 0x0080)
#define CA7_MON_SEL_BIT					0
#define CA7_MON_SEL_MASK				0xFF

#define MCU_BIU_CON						(MCUSYS_CFGREG_BASE + 0x040C)
#define CLKMUX_SEL_MON_BIT				2
#define CLKMUX_SEL_MON_MASK				0x1

#define INFRA_AO_DBG_CON0				(INFRA_SYS_CFG_AO_BASE + 0x0500)
#define DEBUG_PIN_SEL_BIT				5
#define INFRA_AO_DEBUG_MON0_BIT			0
#define DEBUG_PIN_SEL_MASK				0x1
#define INFRA_AO_DEBUG_MON0_MASK		0x1F

#define RG_CLK_DBG_EN_BIT				12
#define RG_CLK_DBGOUT_SEL_BIT			10
#define RG_FUNCTEST_MFG_26M_SEL_BIT		9
#define RG_FUNCTEST_MM_26M_SEL_BIT		8
#define RG_ARMCLK_K1_BIT				0

#define RG_CLK_DBG_EN_MASK				0x1
#define RG_CLK_DBGOUT_SEL_MASK			0x3
#define RG_FUNCTEST_MFG_26M_SEL_MASK	0x1
#define RG_FUNCTEST_MM_26M_SEL_MASK		0x1
#define RG_ARMCLK_K1_MASK				0xFF

typedef enum
{
		TOP_CLOCK_CONTROL			= 0,
		PMIC_SPI_CONTROL_SIGNALS,
		CONNSYS_DEBUG_PINS_MSB,
		CONNSYS_DEBUG_PINS_LSB,
		INFRASYS_GLOBAL_CON_MSB,
		INFRASYS_GLOBAL_CON_LSB	   	= 5,
		MCUSYS_DEBUG_PINS_MSB,
		MUCSYS_DEBUG_PINS_LSB,
		MDSYS_DEBUG_PINS_MSB,
		MDSYS_DEBUG_PINS_LSB,
		INFRA_SYS_POWER_DOWN_DEBUG_PIN =10,
		PMIC_WRAP,
		MIPI_TX_DEBUG_PINS,
		MIPI_RX_DEBUG_PINS,
		MMSYS_DEBUG_PINS_MSB,
		MMSYS_DEBUG_PINS_LSB		=15,
		MFG_DEBUG_PINS_MSB,
		MFG_DEBUG_PINS_LSB,
		SPM_DEBUG_PINS_MSB,
		SPM_DEBUG_PINS_LSB,
		APMIXED_SYS_DEBUG_PINS_MSB 	=20,
		APMIXED_SYS_DEBUG_PINS_LSB,
		USB_DEBUG_PINS				=22,

} INFRA_AO_DEBUG_MON0_TYPE;



/////////////////////////////////////////
//- LPM
//- LPM_CTRL
#define LPM_CTRL_REG			(TOPCKGEN_BASE + 0x0100)
#define LPM_TOTAL_TIME          (TOPCKGEN_BASE + 0x0104)
#define LPM_LOW2HIGH_COUNT      (TOPCKGEN_BASE + 0x0108)
#define LPM_HIGH_DUR_TIME       (TOPCKGEN_BASE + 0x010C)
#define LPM_LONGEST_HIGHTIME    (TOPCKGEN_BASE + 0x0110)
#define LPM_GOODDUR_COUNT		(TOPCKGEN_BASE + 0x0114)

#define LPM_CNTx_BASE(x)		(LPM_TOTAL_TIME + (x * 4))

#define RG_GOOD_DUR_CRITERIA_BIT		16
#define RG_MON_SRC_SEL_BIT				4
#define RG_DBG_MON_START_BIT			2
#define RG_32K_CK_SEL_BIT				1
#define RG_SOFT_RSTB_BIT				0

#define RG_GOOD_DUR_CRITERIA_MASK		0xFF
#define RG_MON_SRC_SEL_MASK				0xF
#define RG_DBG_MON_START_MASK			0x1
#define RG_32K_CK_SEL_MASK				0x1
#define RG_SOFT_RSTB_MASK				0x1

//- LPM_TOTAL_TIME
#define RG_TOTAL_TIME_BIT				0
#define RG_TOTAL_TIME_MASK				0x7FFFFFFF

//- LPM_TOTAL_TIME
#define RG_LOW2HIGH_COUNT_BIT			0
#define RG_LOW2HIGH_COUNT_MASK			0x3FFFFFFF

//- LPM_HIGH_DUR_TIME
#define RG_HIGH_DUR_TIME_BIT			0
#define RG_HIGH_DUR_TIME_MASK			0x7FFFFFFF

//- LPM_LONGEST_HIGHTIME
#define RG_GOODDUR_CNT_OVERFLOW_BIT		31
#define RG_LNGHI_TIME_OVERFLOW_BIT		30
#define RG_HIDUR_TIME_OVERFLOW_BIT		29
#define RG_L2H_CNT_OVERFLOW_BIT			28
#define RG_TOTAL_TIME_OVERFLOW_BIT		27
#define RG_LONGEST_HIGHDUR_TIME_BIT		0

#define RG_GOODDUR_CNT_OVERFLOW_MASK	0x1
#define RG_LNGHI_TIME_OVERFLOW_MASK		0x1
#define RG_HIDUR_TIME_OVERFLOW_MASK		0x1
#define RG_L2H_CNT_OVERFLOW_MASK		0x1
#define RG_TOTAL_TIME_OVERFLOW_MASK		0x1
#define RG_LONGEST_HIGHDUR_TIME_MASK	0xFFFF

//- LPM_GOODDUR_COUNT
#define RG_GOOD_DUR_COUNT_BIT			0
#define RG_GOOD_DUR_COUNT_MASK			0x3FFFFFFF



/////////////////////////////////////////////////////////////////////////
//-
//-		SW SECTION
//-
#ifdef __MT_DCM_C__
#define DCM_EXTERN
#else
#define DCM_EXTERN	extern
#endif

////////////////////////////////////////////////
//- dcm
typedef enum
{
	EM_ARM_DCM				= 0,	//- bit[1:0] : armcode mode, bit[8]:cg2, bit[9]:cg3, bit[20:16]:cg0
	EM_EMI_DCM,                     //-
	EM_INFRA_DCM,                   //-
	EM_PERI_DCM,                    //-
	EM_MISC_DCM,                    //-
	EM_MM_DCM,                      //-
	EM_MFG_DCM,                     //-
	NUM_OF_EM_DCM_TYPE              //-
}  EM_DCM_TYPE;

typedef enum
{
	AP_DCM_ARMCORE		= 0,
	AP_DCM_ARML2BUS,
	AP_DCM_TOPBUS,
	AP_DCM_EMI,
	AP_DCM_INFRA,
	AP_DCM_PERI,
	AP_DCM_PMIC,
	AP_DCM_USB,
	NUM_OF_AP_DCM_TYPE,

	AP_DCM_ARMCORE_MASK		= (1 <<  AP_DCM_ARMCORE),
	AP_DCM_ARML2BUS_MASK	= (1 <<  AP_DCM_ARML2BUS),
	AP_DCM_TOPBUS_MASK      = (1 <<  AP_DCM_TOPBUS),
	AP_DCM_EMI_MASK         = (1 <<  AP_DCM_EMI),
	AP_DCM_INFRA_MASK       = (1 <<  AP_DCM_INFRA),
	AP_DCM_PERI_MASK        = (1 <<  AP_DCM_PERI),
	AP_DCM_PMIC_MASK        = (1 <<  AP_DCM_PMIC),
	AP_DCM_USB_MASK 	    = (1 <<  AP_DCM_USB)
} AP_DCM_TYPE;



#define AP_DCM_ON_ARM_ALLBUS	(	AP_DCM_ARMCORE_MASK 	| \
                                    AP_DCM_ARML2BUS_MASK	| \
                                    AP_DCM_EMI_MASK         | \
                                    AP_DCM_INFRA_MASK       | \
                                    AP_DCM_PERI_MASK 		  \
								)
typedef enum
{
	MM_DCM_SMI_COMMON,
	MM_DCM_SMI_LARB0,
	MM_DCM_MM_CMDQ,
	MM_DCM_MUTEX,
	MM_DCM_DISP_COLOR,
	MM_DCM_DISP_BLS,
	MM_DCM_DISP_WDMA,
	MM_DCM_DISP_RDMA,
	MM_DCM_DISP_OVL,
	MM_DCM_MDP_TDSHP,
	MM_DCM_MDP_WROT,
	MM_DCM_MDP_WDMA,
	MM_DCM_MDP_RSZ1,
	MM_DCM_MDP_RSZ0,
	MM_DCM_MDP_RDMA,
	MM_DCM_DSI_ENGINE,
	MM_DCM_DISP_DBI_ENGINE,
	MM_DCM_DISP_DBI_SMI,
	MM_DCM_DSI_DIGITAL,
	MM_DCM_VENC,
	MM_DCM_VDEC,
	MM_DCM_CAM,
	NUM_OF_MM_DCM_TYPE,

	MM_DCM_SMI_COMMON_MASK		= (1 << MM_DCM_SMI_COMMON		),
	MM_DCM_SMI_LARB0_MASK       = (1 << MM_DCM_SMI_LARB0        ),
	MM_DCM_MM_CMDQ_MASK         = (1 << MM_DCM_MM_CMDQ          ),
	MM_DCM_MUTEX_MASK           = (1 << MM_DCM_MUTEX            ),
	MM_DCM_DISP_COLOR_MASK      = (1 << MM_DCM_DISP_COLOR       ),
	MM_DCM_DISP_BLS_MASK        = (1 << MM_DCM_DISP_BLS         ),
	MM_DCM_DISP_WDMA_MASK       = (1 << MM_DCM_DISP_WDMA        ),
	MM_DCM_DISP_RDMA_MASK       = (1 << MM_DCM_DISP_RDMA        ),
	MM_DCM_DISP_OVL_MASK        = (1 << MM_DCM_DISP_OVL         ),
	MM_DCM_MDP_TDSHP_MASK       = (1 << MM_DCM_MDP_TDSHP        ),
	MM_DCM_MDP_WROT_MASK        = (1 << MM_DCM_MDP_WROT         ),
	MM_DCM_MDP_WDMA_MASK        = (1 << MM_DCM_MDP_WDMA         ),
	MM_DCM_MDP_RSZ1_MASK        = (1 << MM_DCM_MDP_RSZ1         ),
	MM_DCM_MDP_RSZ0_MASK        = (1 << MM_DCM_MDP_RSZ0         ),
	MM_DCM_MDP_RDMA_MASK        = (1 << MM_DCM_MDP_RDMA         ),
	MM_DCM_DSI_ENGINE_MASK      = (1 << MM_DCM_DSI_ENGINE       ),
	MM_DCM_DISP_DBI_ENGINE_MASK = (1 << MM_DCM_DISP_DBI_ENGINE  ),
	MM_DCM_DISP_DBI_SMI_MASK    = (1 << MM_DCM_DISP_DBI_SMI     ),
	MM_DCM_DSI_DIGITAL_MASK     = (1 << MM_DCM_DSI_DIGITAL      ),
	MM_DCM_VENC_MASK            = (1 << MM_DCM_VENC             ),
	MM_DCM_VDEC_MASK            = (1 << MM_DCM_VDEC             ),
	MM_DCM_CAM_MASK             = (1 << MM_DCM_CAM              ),
	NUM_OF_MM_DCM_TYPE_MASK		= (1 << NUM_OF_MM_DCM_TYPE      )
} MM_DCM_TYPE;

typedef enum
{
	MFG_DCM_BG3D,

	NUM_OF_MFG_DCM_TYPE
} MFG_DCM_TYPE;

//- ARM CORE DCM

#define AP_DCM_OP_DISABLE		0x00
#define AP_DCM_OP_ENABLE		0x01
#define AP_DCM_OP_CONFIG		0x02

#define CK_DIV1_SEL_4D4		0x08
#define CK_DIV1_SEL_3D4		0x09
#define CK_DIV1_SEL_2D4		0x0A
#define CK_DIV1_SEL_1D4		0x0B
#define CK_DIV1_SEL_5D5		0x10
#define CK_DIV1_SEL_4D5		0x11
#define CK_DIV1_SEL_3D5		0x12
#define CK_DIV1_SEL_2D5		0x13
#define CK_DIV1_SEL_1D5		0x14
#define CK_DIV1_SEL_6D6		0x18
#define CK_DIV1_SEL_5D6		0x19
#define CK_DIV1_SEL_4D6		0x1A
#define CK_DIV1_SEL_3D6		0x1B
#define CK_DIV1_SEL_2D6		0x1C
#define CK_DIV1_SEL_1D6		0x1D

#define ARM_CORE_DCM_DISABLE	0
#define ARM_CORE_DCM_MODE1		1
#define ARM_CORE_DCM_MODE2		2

typedef struct
{
	UINT32	mode;
} AP_DCM_ARMCORE_CTRL;


//- ARM L2BUS DCM
#define AXI_DIV_SEL_UNCHANGE	0x00
#define AXI_DIV_SEL_D1			0x11
#define AXI_DIV_SEL_D2			0x12
#define AXI_DIV_SEL_D3      	0x13
#define AXI_DIV_SEL_D4      	0x14
#define AXI_DIV_SEL_D5      	0x15

typedef struct
{
	UINT32	cg0_div;
	UINT32	cg1_en;
	UINT32	cg2_en;
} AP_DCM_ARML2BUS_CTRL;


//- TOP BUS
typedef struct
{
	UINT32	force_off;
	UINT32	force_on;
	UINT32	full_sel;
} AP_DCM_TOPBUS_CTRL;


//- EMI DCM
#define RG_EMIDCM_FSEL_DIV1			0
#define RG_EMIDCM_SFSEL_DIV16		15
#define RG_EMIDCM_SFSEL_DIV32		31
typedef struct
{
	UINT32	force_off;
	UINT32	force_on;
	UINT32	dcm_en;
	UINT32	dbc_en;
	UINT32	dbc_cnt;
	UINT32	full_sel;
	UINT32	idle_fsel;
} AP_DCM_EMI_CTRL;

//- INFRA DCM
#define RG_INFRADCM_SFSEL_DIV1		0x10
#define RG_INFRADCM_SFSEL_DIV2		0x08
#define RG_INFRADCM_SFSEL_DIV4		0x04
#define RG_INFRADCM_SFSEL_DIV8		0x02
#define RG_INFRADCM_SFSEL_DIV16		0x01
#define RG_INFRADCM_SFSEL_DIV32		0x00

#define RG_INFRADCM_FSEL_DIV1		0x10
#define RG_INFRADCM_FSEL_DIV2		0x08
#define RG_INFRADCM_FSEL_DIV4		0x04
#define RG_INFRADCM_FSEL_DIV8		0x02
#define RG_INFRADCM_FSEL_DIV16		0x01
#define RG_INFRADCM_FSEL_DIV32		0x00

#define RG_PMICDCM_SFSEL_DIV1		0x10
#define RG_PMICDCM_SFSEL_DIV2		0x08
#define RG_PMICDCM_SFSEL_DIV4		0x04
#define RG_PMICDCM_SFSEL_DIV8		0x02
#define RG_PMICDCM_SFSEL_DIV16		0x01
#define RG_PMICDCM_SFSEL_DIV32		0x00


typedef struct
{
	UINT32	force_on;
	UINT32	force_clkoff;
	UINT32	force_clkslw;
	UINT32	clkoff_en;
	UINT32	clkslw_en;
	UINT32	full_sel;
	UINT32	idle_fsel;

	UINT32	dbc_en;
	UINT32	dbc_cnt;
} AP_DCM_INFRA_CTRL;

//- PERI DCM
#define RG_PERIDCM_SFSEL_DIV1		0x10
#define RG_PERIDCM_SFSEL_DIV2		0x08
#define RG_PERIDCM_SFSEL_DIV4		0x04
#define RG_PERIDCM_SFSEL_DIV8		0x02
#define RG_PERIDCM_SFSEL_DIV16		0x01
#define RG_PERIDCM_SFSEL_DIV32		0x00

#define RG_PERIDCM_FSEL_DIV1		0x10
#define RG_PERIDCM_FSEL_DIV2		0x08
#define RG_PERIDCM_FSEL_DIV4		0x04
#define RG_PERIDCM_FSEL_DIV8		0x02
#define RG_PERIDCM_FSEL_DIV16		0x01
#define RG_PERIDCM_FSEL_DIV32		0x00
typedef struct
{
	UINT32	force_on;
	UINT32	force_clkoff;
	UINT32	force_clkslw;
	UINT32	clkoff_en;
	UINT32	clkslw_en;
	UINT32	full_sel;
	UINT32	idle_fsel;

	UINT32	dbc_en;
	UINT32	dbc_cnt;
} AP_DCM_PERI_CTRL;

//- PMIC DCM
#define RG_PMIC_SFSEL_DIV1		0x10
#define RG_PMIC_SFSEL_DIV2		0x08
#define RG_PMIC_SFSEL_DIV4		0x04
#define RG_PMIC_SFSEL_DIV5		0x02
#define RG_PMIC_SFSEL_DIV16		0x01
#define RG_PMIC_SFSEL_DIV32		0x00

typedef struct
{
	UINT32	rg_pmic_spiclkdcm_en;
	UINT32	rg_pmic_bclkdcm_en;
	UINT32	rg_pmic_sfsel;
} AP_DCM_PMIC_CTRL;


//- USB DCM
typedef struct
{
	UINT32	rg_usbdcm_en;
} AP_DCM_USB_CTRL;


//- MMSYS DCM


//- MFG DCM
#define BG3D_FSEL_DIV1			0x20
#define BG3D_FSEL_DIV2			0x10
#define BG3D_FSEL_DIV4          0x08
#define BG3D_FSEL_DIV8          0x04
#define BG3D_FSEL_DIV16         0x02
#define BG3D_FSEL_DIV32         0x01
#define BG3D_FSEL_DIV64         0x00

typedef struct
{
	UINT32	dcm_en;
	UINT32	dcm_fsel;
	UINT32	dbc_en;
	UINT32	dbc_cnt;
} MFG_DCM_BG3D_CTRL;

typedef struct
{
	char	name[32];
	UINT32	addr;
} DCM_REG_MAP;


#define AP_DCM_SUCCESS			0
#define AP_DCM_FAIL				1
#define AP_DCM_NO_TYPE			2

#ifdef __DCM_CTP__
#define DCM_CHECK_POINT_IN_END()	dcm_check_idle_time (LPM_CNT_TYPE_MASK, 0x10000000)
DCM_EXTERN const char AP_dcm_name[NUM_OF_AP_DCM_TYPE][16];
extern	   INT32 dcm_check_idle_time (UINT32 option, UINT32 criteria);
DCM_EXTERN UINT32 dcm_init(UINT32 option);
#endif	//- __DCM_CTP__

#ifndef __DCM_CTP__
DCM_EXTERN void mt_dcm_init(void);
#endif //- !__DCM_CTP__

DCM_EXTERN INT32 ap_dcm_config (UINT32 type, void* ctrl);
DCM_EXTERN INT32 dcm_enable_usb (UINT32 enable);
DCM_EXTERN INT32 dcm_enable_pmic (UINT32 sfsel, UINT32 spi_dcm_en, UINT32 bclk_dcm_en);

DCM_EXTERN void dcm_mmsys_enable (UINT32 id_mask);
DCM_EXTERN void dcm_mmsys_disable (UINT32 id_mask);
DCM_EXTERN void dcm_mmsys_smi_common(UINT32 enable);
DCM_EXTERN void dcm_mmsys_smi_larb0(UINT32 enable);
DCM_EXTERN void dcm_mmsys_mm_cmdq(UINT32 enable);
DCM_EXTERN void dcm_mmsys_mutex(UINT32 enable);
DCM_EXTERN void dcm_mmsys_disp_color(UINT32 enable);
DCM_EXTERN void dcm_mmsys_disp_bls(UINT32 enable);
DCM_EXTERN void dcm_mmsys_disp_wdma(UINT32 enable);
DCM_EXTERN void dcm_mmsys_disp_rdma(UINT32 enable);
DCM_EXTERN void dcm_mmsys_disp_ovl(UINT32 enable);
DCM_EXTERN void dcm_mmsys_mdp_tdshp(UINT32 enable);
DCM_EXTERN void dcm_mmsys_mdp_wrot(UINT32 enable);
DCM_EXTERN void dcm_mmsys_mdp_wdma(UINT32 enable);
DCM_EXTERN void dcm_mmsys_mdp_rsz1(UINT32 enable);
DCM_EXTERN void dcm_mmsys_mdp_rsz0(UINT32 enable);
DCM_EXTERN void dcm_mmsys_mdp_rdma(UINT32 enable);
DCM_EXTERN void dcm_mmsys_dsi_engine(UINT32 enable);
DCM_EXTERN void dcm_mmsys_disp_dbi_engine(UINT32 enable);
DCM_EXTERN void dcm_mmsys_disp_dbi_smi(UINT32 enable);
DCM_EXTERN void dcm_mmsys_dsi_digital(UINT32 enable);
DCM_EXTERN void dcm_mmsys_venc(UINT32 enable);
DCM_EXTERN void dcm_mmsys_vdec(UINT32 enable);
DCM_EXTERN void dcm_mmsys_cam(UINT32 enable);

DCM_EXTERN void dcm_mfgsys_gpu(MFG_DCM_BG3D_CTRL* ctrl);

////////////////////////////////////////////////
//- gpiodbg monitor
DCM_EXTERN void gpiodbg_armcore_dbg_out(void);
DCM_EXTERN void gpiodbg_arml2bus_dbg_out(void);
DCM_EXTERN void gpiodbg_spm_csw_dbg_out(void);
DCM_EXTERN void gpiodbg_emi_dbg_out(void);
DCM_EXTERN void gpiodbg_infra_dbg_out(void);
DCM_EXTERN void gpiodbg_peri_dbg_out(void);
DCM_EXTERN void gpiodbg_monitor(void);


////////////////////////////////////////////////
//- freq meter
//- mon_sel
#define  FQMTR_SRC_MAINPLL_DIV8					0x01
#define  FQMTR_SRC_MAINPLL_DIV12				0x02
#define  FQMTR_SRC_MAINPLL_DIV24				0x03
#define  FQMTR_SRC_MAINPLL_DIV20				0x04
#define  FQMTR_SRC_MAINPLL_DIV7					0x05
#define  FQMTR_SRC_UNIVPLL_DIV16				0x06
#define  FQMTR_SRC_UNIVPLL_DIV24				0x07
#define  FQMTR_SRC_UNIVPLL_DIV20				0x08
#define  FQMTR_SRC_WHPLL_OUTPUT_CLOCK			0x09
#define  FQMTR_SRC_WPLL_OUTPUT_CLOCK			0x0A
#define  FQMTR_SRC_SYS_26MHZ_CLOCK				0x0B
#define  FQMTR_SRC_USB_48MHZ_CLOCK 				0x0C
#define  FQMTR_SRC_EMI2X_CLOCK					0x0D
#define  FQMTR_SRC_AP_INFRA_FAST_BUS_CLOCK		0x0E
#define  FQMTR_SRC_SMI_CLOCK_OF_MMSYS			0x0F
#define  FQMTR_SRC_UART0_CLOCK					0x10
#define  FQMTR_SRC_GPU_CLOCK 					0x11
#define  FQMTR_SRC_MSDC1_CLOCK					0x12
#define  FQMTR_SRC_CAM_SENINF_CLOCK				0x13
#define  FQMTR_SRC_PWM_OF_MMSYS_CLOCK			0x14
#define  FQMTR_SRC_SPI_NFI_CLOCK				0x15
#define  FQMTR_SRC_DBI_CLOCK					0x16
#define  FQMTR_SRC_PMIC_SPI_CLOCK				0x17
#define  FQMTR_SRC_AP_PLLGP_TST_CK				0x18
#define  FQMTR_SRC_APMCU_CLOCK					0x19
#define  FQMTR_SRC_RTC_32KHZ_CLOCK				0x1A
#define  NUM_OF_FQMTR_SOURCE					0x1B
//- ref_clk_sel
#define  RG_FQMTR_CKDIV_D1				0x00
#define  RG_FQMTR_CKDIV_D2				0x01
#define  RG_FQMTR_CKDIV_D4				0x02
#define  RG_FQMTR_CKDIV_D8				0x03
#define  RG_FQMTR_CKDIV_D16				0x04
//- divider
#define	 RG_FQMTR_FIXCLK_SEL_26MHZ		0x00
#define	 RG_FQMTR_FIXCLK_SEL_32KHZ		0x02

//- freq meter error code
#define FREQMETER_SUCCESS				0
#define FREQMETER_NO_RESOURCE			-1
#define FREQMETER_NOT_OWNER				-2
#define FREQMETER_COUNTING				-3
#define FREQMETER_OUT_BOUNDARY			-255

#define FREQMETER_OVERFLOW				RG_FQMTR_DATA_MASK

typedef struct
{
	//- input
	UINT32 divider;
	UINT32 ref_clk_sel;
	UINT32 mon_sel;
	UINT32 mon_len_in_ref_clk;
	UINT32 polling_to_getresult;

	//- output
	UINT32 result_in_count;

	//- internal control
	UINT32 owner;
} FREQMETER_CTRL;

DCM_EXTERN INT32 freqm_kick (FREQMETER_CTRL* ctl, const char* caller);
DCM_EXTERN INT32 freqm_getresult (FREQMETER_CTRL* ctl, const char* caller);

#define freqmeter_kick(pARAM)			freqm_kick(pARAM, __func__)
#define freqmeter_getresult(pARAM)		freqm_getresult(pARAM, __func__)

////////////////////////////////////////////////
//- LPM

#define LPM_SRC_CONN_SRCCLKENA_ACK					0
#define LPM_SRC_CONN_SRCCLKENA						1
#define LPM_SRC_MD_SRCCLKENA						2
#define LPM_SRC_MD_SRCCLKENA_ACK					3
#define LPM_SRC_CLKSQ_OFF							4
#define LPM_SRC_INFRASYS_TOP_MTCMOS_POWER_ON		5
#define LPM_SRC_CA7_CORE0_MTCMOS_POWER_ON			6
#define LPM_SRC_CA7_CORE1_MTCMOS_POWER_ON			7
#define LPM_SRC_DCM_ALL_IDLE						8
#define LPM_SRC_MDSYS_BUS_PORT_IDLE					9
#define LPM_SRC_EMI_CONTROLLER_IDLE					10
#define LPM_SRC_EMI_BUS_SLAVE_PORT_IDLE				11
#define LPM_SRC_INFRASYS_BUS_IDLE					12
#define LPM_SRC_PERISYS_BUS_IDLE					13
#define LPM_SRC_CA7_CORE0_STANDBYWFI				14
#define LPM_SRC_CA7_CORE1_STANDBYWFI				15
#define NUM_OF_LPM_SOURCE							16

#define LPM_REF_CLOCK_32K					1
#define LPM_REF_CLOCK_104M					0

//- LPM error code
#define LPM_SUCCESS						0
#define LPM_KICK_COLLISION				-1
#define LPM_GET_COLLISION				-2
#define LPM_EM_OPCODE_ERR				-3

typedef enum
{
	LPM_TOTAL_TIME_TYPE		= 0,
	LPM_LOW2HIGH_COUNT_TYPE,
	LPM_HIGH_DUR_TIME_TYPE,
	LPM_LONGEST_HIGHTIME_TYPE,
	LPM_GOODDUR_COUNT_TYPE,

	NUM_OF_LPM_CNT_TYPE,
	LPM_CNT_TYPE_MASK = ((1 << NUM_OF_LPM_CNT_TYPE) -1)

} LPM_CNT_TYPE;

#define LPM_COUNT_OVERFLOW_MASK		(1 << 31)

typedef struct
{
	//- input
	UINT32	ref_clk_sel;
	UINT32	mon_sel;
	UINT32	good_duration_criteria;

	//- output
	UINT32	result[NUM_OF_LPM_CNT_TYPE]; //- bit[31] indicate overflow

	//- internal control
	UINT32 owner;

} LPM_CTRL;

DCM_EXTERN INT32 lpm_getresult (LPM_CTRL* ctl, const char* caller);
DCM_EXTERN INT32 lpm_kick (LPM_CTRL* ctl, const char* caller);
DCM_EXTERN const char LPM_getname[NUM_OF_LPM_CNT_TYPE][16];

#define LPM_kick(pARAM)				lpm_kick(pARAM, __func__)
#define LPM_getresult(pARAM)		lpm_getresult(pARAM, __func__)

#endif //- __MT_DCM_H__
