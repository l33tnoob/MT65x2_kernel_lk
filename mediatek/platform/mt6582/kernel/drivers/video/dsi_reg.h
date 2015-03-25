#ifndef __DSI_REG_H__
#define __DSI_REG_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    unsigned RG_DSI0_LDOCORE_EN                	: 1;
    unsigned RG_DSI0_CKG_LDOOUT_EN                	: 1;
    unsigned RG_DSI0_BCLK_SEL                	: 2;
    unsigned RG_DSI0_LD_IDX_SEL                	: 3;
    unsigned rsv_7					: 1;    	    	
    unsigned RG_DSI0_PHYCLK_SEL                	: 2;
    unsigned RG_DSI0_DSICLK_FREQ_SEL                	: 1;
    unsigned RG_DSI0_LPTX_CLMP_EN                	: 1;
    unsigned rsv_12					: 20;    	    	
} MIPITX_DSI0_CON_REG, *PMIPITX_DSI0_CON_REG;


typedef struct
{
    unsigned RG_DSI0_LNTC_LDOOUT_EN     		: 1;
    unsigned RG_DSI0_LNTC_LOOPBACK_EN     		: 1;
    unsigned RG_DSI0_LNTC_LPTX_IPLUS1     		: 1;
    unsigned RG_DSI0_LNTC_LPTX_IPLUS2     		: 1;
    unsigned RG_DSI0_LNTC_LPTX_IMINUS     		: 1;
    unsigned RG_DSI0_LNTC_PHI_SEL     		: 1;
    unsigned rsv_6           		: 2;
    unsigned RG_DSI0_LNTC_RT_CODE     		: 4;
    unsigned rsv_12           		: 20;
} MIPITX_DSI0_CLOCK_LANE_REG, *PMIPITX_DSI0_CLOCK_LANE_REG;


typedef struct
{
    unsigned RG_DSI0_LNT0_LDOOUT_EN     		: 1;
    unsigned RG_DSI0_LNT0_LOOPBACK_EN     		: 1;
    unsigned RG_DSI0_LNT0_LPTX_IPLUS1     		: 1;
    unsigned RG_DSI0_LNT0_LPTX_IPLUS2     		: 1;
    unsigned RG_DSI0_LNT0_LPTX_IMINUS     		: 1;
    unsigned RG_DSI0_LNT0_LPCD_IPLUS     		: 1;
    unsigned RG_DSI0_LNT0_LPCD_IMINUS     		: 1;
    unsigned RG_DSI0_LNT0_RT_CODE     		: 4;
    unsigned rsv_11           		: 21;
} MIPITX_DSI0_DATA_LANE0_REG, *PMIPITX_DSI0_DATA_LANE0_REG;


typedef struct
{
    unsigned RG_DSI0_LNT1_LDOOUT_EN     		: 1;
    unsigned RG_DSI0_LNT1_LOOPBACK_EN     		: 1;
    unsigned RG_DSI0_LNT1_LPTX_IPLUS1     		: 1;
    unsigned RG_DSI0_LNT1_LPTX_IPLUS2     		: 1;
    unsigned RG_DSI0_LNT1_LPTX_IMINUS     		: 1;
    unsigned RG_DSI0_LNT1_RT_CODE     		: 4;
    unsigned rsv_9           		: 23;
} MIPITX_DSI0_DATA_LANE1_REG, *PMIPITX_DSI0_DATA_LANE1_REG;


typedef struct
{
    unsigned RG_DSI0_LNT2_LDOOUT_EN     		: 1;
    unsigned RG_DSI0_LNT2_LOOPBACK_EN     		: 1;
    unsigned RG_DSI0_LNT2_LPTX_IPLUS1     		: 1;
    unsigned RG_DSI0_LNT2_LPTX_IPLUS2     		: 1;
    unsigned RG_DSI0_LNT2_LPTX_IMINUS     		: 1;
    unsigned RG_DSI0_LNT2_RT_CODE     		: 4;
    unsigned rsv_9           		: 23;
} MIPITX_DSI0_DATA_LANE2_REG, *PMIPITX_DSI0_DATA_LANE2_REG;

typedef struct
{
    unsigned RG_DSI0_LNT3_LDOOUT_EN     		: 1;
    unsigned RG_DSI0_LNT3_LOOPBACK_EN     		: 1;
    unsigned RG_DSI0_LNT3_LPTX_IPLUS1     		: 1;
    unsigned RG_DSI0_LNT3_LPTX_IPLUS2     		: 1;
    unsigned RG_DSI0_LNT3_LPTX_IMINUS     		: 1;
    unsigned RG_DSI0_LNT3_RT_CODE     		: 4;
    unsigned rsv_9           		: 23;
} MIPITX_DSI0_DATA_LANE3_REG, *PMIPITX_DSI0_DATA_LANE3_REG;

typedef struct
{
    unsigned RG_DSI_LNT_INTR_EN			: 1;
    unsigned RG_DSI_LNT_HS_BIAS_EN			: 1;
    unsigned RG_DSI_LNT_IMP_CAL_EN			: 1;
    unsigned RG_DSI_LNT_TESTMODE_EN			: 1;
    unsigned RG_DSI_LNT_IMP_CAL_CODE			: 4;
    unsigned RG_DSI_LNT_AIO_SEL			: 3;
    unsigned RG_DSI_PAD_TIE_LOW_EN			: 1;
    unsigned RG_DSI_DEBUG_INPUT_EN			: 1;
    unsigned RG_DSI_PRESERVE			: 3;
    unsigned rsv_16					: 16;
} MIPITX_DSI_TOP_CON_REG, *PMIPITX_DSI_TOP_CON_REG;


typedef struct
{
    unsigned RG_DSI_BG_CORE_EN			: 1;
    unsigned RG_DSI_BG_CKEN			: 1;
    unsigned RG_DSI_BG_DIV			: 2;
    unsigned RG_DSI_BG_FAST_CHARGE			: 1;
    unsigned RG_DSI_V12_SEL			: 3;
    unsigned RG_DSI_V10_SEL			: 3;
    unsigned RG_DSI_V072_SEL			: 3;
    unsigned RG_DSI_V04_SEL			: 3;
    unsigned RG_DSI_V032_SEL			: 3;
    unsigned RG_DSI_V02_SEL			: 3;
    unsigned rsv_23					: 1;
    unsigned RG_DSI_BG_R1_TRIM			: 4;
    unsigned RG_DSI_BG_R2_TRIM			: 4;
} MIPITX_DSI_BG_CON_REG, *PMIPITX_DSI_BG_CON_REG;


typedef struct
{
    unsigned RG_DSI0_MPPLL_PLL_EN		: 1;
    unsigned RG_DSI0_MPPLL_PREDIV		: 2;
    unsigned RG_DSI0_MPPLL_TXDIV0		: 2;
    unsigned RG_DSI0_MPPLL_TXDIV1		: 2;
    unsigned RG_DSI0_MPPLL_POSDIV		: 3;
    unsigned RG_DSI0_MPPLL_MONVC_EN		: 1;
    unsigned RG_DSI0_MPPLL_MONREF_EN		: 1;
    unsigned RG_DSI0_MPPLL_VDO_EN		: 1;
    unsigned rsv_13					: 19;
} MIPITX_DSI_PLL_CON0_REG, *PMIPITX_DSI_PLL_CON0_REG;


typedef struct
{
    unsigned RG_DSI0_MPPLL_SDM_FRA_EN			: 1;
    unsigned RG_DSI0_MPPLL_SDM_SSC_PH_INIT			: 1;
    unsigned RG_DSI0_MPPLL_SDM_SSC_EN			: 1;
    unsigned rsv_3					: 13;
    unsigned RG_DSI0_MPPLL_SDM_SSC_PRD			: 16;
} MIPITX_DSI_PLL_CON1_REG, *PMIPITX_DSI_PLL_CON1_REG;

typedef struct
{
    unsigned RG_DSI0_MPPLL_SDM_PCW_0_7			: 8;
	unsigned RG_DSI0_MPPLL_SDM_PCW_8_15			: 8; 
	unsigned RG_DSI0_MPPLL_SDM_PCW_16_23			: 8;
    unsigned RG_DSI0_MPPLL_SDM_PCW_H			: 7;
    unsigned rsv_31					: 1;
} MIPITX_DSI_PLL_CON2_REG, *PMIPITX_DSI_PLL_CON2_REG;

typedef struct
{
    unsigned RG_DSI0_MPPLL_SDM_SSC_DELTA1			: 16;
    unsigned RG_DSI0_MPPLL_SDM_SSC_DELTA			: 16;
} MIPITX_DSI_PLL_CON3_REG, *PMIPITX_DSI_PLL_CON3_REG;


typedef struct
{
    unsigned RG_DSI0_MPPLL_SDM_PCW_CHG		: 1;
    unsigned rsv_1					: 31;		
} MIPITX_DSI_PLL_CHG_REG, *PMIPITX_DSI_PLL_CHG_REG;


typedef struct
{
    unsigned RG_MPPLL_TST_EN		: 1;
    unsigned RG_MPPLL_TSTCK_EN		: 1;
    unsigned RG_MPPLL_TSTSEL		: 2;
    unsigned rsv_4 				: 4;
    unsigned RG_MPPLL_PRESERVE_L		: 2;
    unsigned RG_MPPLL_PRESERVE_H		: 6;
    unsigned rsv_16 				: 16;
} MIPITX_DSI_PLL_TOP_REG, *PMIPITX_DSI_PLL_TOP_REG;


typedef struct
{
    unsigned DA_DSI0_MPPLL_SDM_PWR_ON		: 1;
    unsigned DA_DSI0_MPPLL_SDM_ISO_EN		: 1;
    unsigned rsv_2 				: 6;
    unsigned AD_DSI0_MPPLL_SDM_PWR_ACK		: 1;
    unsigned rsv_9 				: 23;
} MIPITX_DSI_PLL_PWR_REG, *PMIPITX_DSI_PLL_PWR_REG;


typedef struct
{
    unsigned RGS_DSI_LNT_IMP_CAL_OUTPUT		: 1;
    unsigned rsv_1 				: 31;
} MIPITX_DSI_RGS_REG, *PMIPITX_DSI_RGS_REG;


typedef struct
{
    unsigned RG_DSI0_GPI0_EN		: 1;
    unsigned RG_DSI0_GPI1_EN		: 1;
    unsigned RG_DSI0_GPI2_EN		: 1;
    unsigned RG_DSI0_GPI3_EN		: 1;
    unsigned RG_DSI0_GPI4_EN		: 1;
    unsigned RG_DSI0_GPI5_EN		: 1;
    unsigned RG_DSI0_GPI6_EN		: 1;
    unsigned RG_DSI0_GPI7_EN		: 1;
    unsigned RG_DSI0_GPI_SMT_EN		: 1;
    unsigned RG_DSI0_GPI_DRIVE_EN		: 1;
    unsigned rsv_10 				: 22;
} MIPITX_DSI_GPIO_EN_REG, *PMIPITX_DSI_GPIO_EN_REG;


typedef struct
{
    unsigned AD_DSI0_GPI0_OUT		: 1;
    unsigned AD_DSI0_GPI1_OUT		: 1;
    unsigned AD_DSI0_GPI2_OUT		: 1;
    unsigned AD_DSI0_GPI3_OUT		: 1;
    unsigned AD_DSI0_GPI4_OUT		: 1;
    unsigned AD_DSI0_GPI5_OUT		: 1;
    unsigned AD_DSI0_GPI6_OUT		: 1;
    unsigned AD_DSI0_GPI7_OUT		: 1;
    unsigned rsv_8 				: 24;
} MIPITX_DSI_GPIO_OUT_REG, *PMIPITX_DSI_GPIO_OUT_REG;


typedef struct
{
    unsigned SW_CTRL_EN		: 1;
    unsigned rsv_1 				: 31;
} MIPITX_DSI_SW_CTRL_REG, *PMIPITX_DSI_SW_CTRL_REG;


typedef struct
{
    unsigned SW_LNTC_LPTX_PRE_OE		: 1;
    unsigned SW_LNTC_LPTX_OE		: 1;
    unsigned SW_LNTC_LPTX_P		: 1;
    unsigned SW_LNTC_LPTX_N		: 1;
    unsigned SW_LNTC_HSTX_PRE_OE		: 1;
    unsigned SW_LNTC_HSTX_OE		: 1;
    unsigned SW_LNTC_HSTX_ZEROCLK		: 1;
    unsigned SW_LNT0_LPTX_PRE_OE		: 1;
    unsigned SW_LNT0_LPTX_OE		: 1;
    unsigned SW_LNT0_LPTX_P		: 1;
    unsigned SW_LNT0_LPTX_N		: 1;
    unsigned SW_LNT0_HSTX_PRE_OE		: 1;
    unsigned SW_LNT0_HSTX_OE		: 1;
    unsigned SW_LNT0_LPRX_EN		: 1;
    unsigned SW_LNT1_LPTX_PRE_OE		: 1;
    unsigned SW_LNT1_LPTX_OE		: 1;
    unsigned SW_LNT1_LPTX_P		: 1;
    unsigned SW_LNT1_LPTX_N		: 1;
    unsigned SW_LNT1_HSTX_PRE_OE		: 1;
    unsigned SW_LNT1_HSTX_OE		: 1;
    unsigned SW_LNT2_LPTX_PRE_OE		: 1;
    unsigned SW_LNT2_LPTX_OE		: 1;
    unsigned SW_LNT2_LPTX_P		: 1;
    unsigned SW_LNT2_LPTX_N		: 1;
    unsigned SW_LNT2_HSTX_PRE_OE		: 1;
    unsigned SW_LNT2_HSTX_OE		: 1;
    unsigned SW_LNT3_LPTX_PRE_OE		: 1;
    unsigned SW_LNT3_LPTX_OE		: 1;
    unsigned SW_LNT3_LPTX_P		: 1;
    unsigned SW_LNT3_LPTX_N		: 1;
    unsigned SW_LNT3_HSTX_PRE_OE		: 1;
    unsigned SW_LNT3_HSTX_OE		: 1;	
} MIPITX_DSI_SW_CTRL_CON0_REG, *PMIPITX_DSI_SW_CTRL_CON0_REG;


typedef struct
{
    unsigned SW_LNT_HSTX_DATA		: 8;
    unsigned SW_LNT_HSTX_DRDY		: 1;
    unsigned rsv_9 				: 23;
} MIPITX_DSI_SW_CTRL_CON1_REG, *PMIPITX_DSI_SW_CTRL_CON1_REG;


typedef struct
{
    unsigned MIPI_TX_DBG_SEL		: 3;
    unsigned MIPI_TX_DBG_OUT_EN		: 1;
    unsigned rsv_4 				: 28;
} MIPITX_DSI_DBG_CON_REG, *PMIPITX_DSI_DBG_CON_REG;


typedef struct
{
    MIPITX_DSI0_CON_REG		MIPITX_DSI0_CON;		// 0000
    MIPITX_DSI0_CLOCK_LANE_REG		MIPITX_DSI0_CLOCK_LANE;		// 0004
    MIPITX_DSI0_DATA_LANE0_REG		MIPITX_DSI0_DATA_LANE0;		// 0008
    MIPITX_DSI0_DATA_LANE1_REG		MIPITX_DSI0_DATA_LANE1;		// 000C
    MIPITX_DSI0_DATA_LANE2_REG		MIPITX_DSI0_DATA_LANE2;		// 0010    
    MIPITX_DSI0_DATA_LANE3_REG		MIPITX_DSI0_DATA_LANE3;		// 0014    
    UINT32                  rsv_18[10];               // 0018..003C

    MIPITX_DSI_TOP_CON_REG		MIPITX_DSI_TOP_CON;		// 0040
    MIPITX_DSI_BG_CON_REG		MIPITX_DSI_BG_CON;		// 0044
    UINT32                  rsv_48[2];               // 0048..004C
    MIPITX_DSI_PLL_CON0_REG		MIPITX_DSI_PLL_CON0;		// 0050
    MIPITX_DSI_PLL_CON1_REG		MIPITX_DSI_PLL_CON1;		// 0054
    MIPITX_DSI_PLL_CON2_REG		MIPITX_DSI_PLL_CON2;		// 0058        
    MIPITX_DSI_PLL_CON3_REG		MIPITX_DSI_PLL_CON3;		// 005C        
    MIPITX_DSI_PLL_CHG_REG		MIPITX_DSI_PLL_CHG;		// 0060        
    MIPITX_DSI_PLL_TOP_REG		MIPITX_DSI_PLL_TOP;		// 0064        
    MIPITX_DSI_PLL_PWR_REG		MIPITX_DSI_PLL_PWR;		// 0068        
    UINT32                  rsv_6C;               // 006C
    MIPITX_DSI_RGS_REG		MIPITX_DSI_RGS;		// 0070        
    MIPITX_DSI_GPIO_EN_REG		MIPITX_DSI_GPIO_EN;		// 0074        
    MIPITX_DSI_GPIO_OUT_REG		MIPITX_DSI_GPIO_OUT;		// 0078        
    UINT32                  rsv_7C;               // 007C
    MIPITX_DSI_SW_CTRL_REG		MIPITX_DSI_SW_CTRL;		// 0080        
    MIPITX_DSI_SW_CTRL_CON0_REG		MIPITX_DSI_SW_CTRL_CON0;		// 0084        
    MIPITX_DSI_SW_CTRL_CON1_REG		MIPITX_DSI_SW_CTRL_CON1;		// 0088        
    UINT32                  rsv_8C;               // 008C
    MIPITX_DSI_DBG_CON_REG		MIPITX_DSI_DBG_CON;		// 0090        
} volatile DSI_PHY_REGS, *PDSI_PHY_REGS;


typedef struct
{
    unsigned DSI_START	: 1;
    unsigned rsv_1		: 1;
    unsigned SLEEPOUT_START	: 1;
    unsigned rsv_3		: 13;
    unsigned VM_CMD_START	: 1;
    unsigned rsv_17		: 15;
} DSI_START_REG, *PDSI_START_REG;


typedef struct
{
    unsigned rsv_0		   : 1;
    unsigned BUF_UNDERRUN  : 1;
    unsigned rsv_2		   : 2;
    unsigned ESC_ENTRY_ERR : 1;
    unsigned LPDT_SYNC_ERR : 1;
    unsigned CTRL_ERR      : 1;
    unsigned CONTENT_ERR   : 1;
    unsigned rsv_8		   : 24;
} DSI_STATUS_REG, *PDSI_STATUS_REG;


typedef struct
{
    unsigned RD_RDY			: 1;
    unsigned CMD_DONE		: 1;
    unsigned TE_RDY         : 1;
    unsigned VM_DONE        : 1;
    unsigned EXT_TE         : 1;
    unsigned VM_CMD_DONE    : 1;
    unsigned SLEEPOUT_DONE    : 1;
    unsigned rsv_7			: 25;
} DSI_INT_ENABLE_REG, *PDSI_INT_ENABLE_REG;


typedef struct
{
    unsigned RD_RDY			: 1;
    unsigned CMD_DONE		: 1;
    unsigned TE_RDY         : 1;
    unsigned VM_DONE        : 1;
    unsigned EXT_TE         : 1;
    unsigned VM_CMD_DONE    : 1;
    unsigned SLEEPOUT_DONE    : 1;
    unsigned rsv_7			: 24;
    unsigned BUSY           : 1;	
} DSI_INT_STATUS_REG, *PDSI_INT_STATUS_REG;


typedef struct
{
    unsigned DSI_RESET		: 1;
    unsigned DSI_EN		: 1;
    unsigned rsv_2			: 30;	
} DSI_COM_CTRL_REG, *PDSI_COM_CTRL_REG;


typedef enum
{
    DSI_CMD_MODE 			= 0,
    DSI_SYNC_PULSE_VDO_MODE = 1,
    DSI_SYNC_EVENT_VDO_MODE = 2,
    DSI_BURST_VDO_MODE 		= 3	
} DSI_MODE_CTRL;


typedef struct
{
    unsigned MODE		: 2;
    unsigned rsv_2	    : 14;
    unsigned FRM_MODE   : 1;
    unsigned MIX_MODE   : 1;
    unsigned V2C_SWITCH_ON   : 1;
    unsigned C2V_SWITCH_ON   : 1;
    unsigned SLEEP_MODE   : 1;
    unsigned rsv_21     : 11;
} DSI_MODE_CTRL_REG, *PDSI_MODE_CTRL_REG;


typedef enum
{
    ONE_LANE = 1,
    TWO_LANE = 2
} DSI_LANE_NUM;


typedef struct
{
    unsigned 		VC_NUM			: 2;
    unsigned		LANE_NUM		: 4;
    unsigned		DIS_EOT			: 1;
    unsigned		NULL_EN			: 1;
    unsigned        TE_FREERUN      : 1;
    unsigned		EXT_TE_EN   	: 1;
    unsigned        EXT_TE_EDGE     : 1;
    unsigned        TE_AUTO_SYNC     : 1;
    unsigned		MAX_RTN_SIZE	: 4;
    unsigned        HSTX_CKLP_EN    : 1;
    unsigned		rsv_17			: 15;
} DSI_TXRX_CTRL_REG, *PDSI_TXRX_CTRL_REG;


typedef enum
{
    PACKED_PS_16BIT_RGB565=0,
    LOOSELY_PS_18BIT_RGB666=1,	
    PACKED_PS_24BIT_RGB888=2,
    PACKED_PS_18BIT_RGB666=3		
} DSI_PS_TYPE;


typedef struct
{
    unsigned 	DSI_PS_WC	: 14;
    unsigned	rsv_14		: 2;
    unsigned	DSI_PS_SEL	: 2;
    unsigned	rsv_18		: 14;
} DSI_PSCTRL_REG, *PDSI_PSCTRL_REG;


typedef struct
{
    unsigned 	VSA_NL		: 7;
    unsigned 	rsv_7		: 25;
} DSI_VSA_NL_REG, *PDSI_VSA_NL_REG;


typedef struct
{
    unsigned 	VBP_NL		: 7;
    unsigned 	rsv_7		: 25;	
} DSI_VBP_NL_REG, *PDSI_VBP_NL_REG;


typedef struct
{
    unsigned 	VFP_NL		: 7;
    unsigned 	rsv_7		: 25;	
} DSI_VFP_NL_REG, *PDSI_VFP_NL_REG;


typedef struct
{
    unsigned 	VACT_NL		: 12;
    unsigned 	rsv_12		: 20;	
} DSI_VACT_NL_REG, *PDSI_VACT_NL_REG;


typedef struct
{
    unsigned 	HSA_WC		: 12;
    unsigned 	rsv_12		: 20;		
} DSI_HSA_WC_REG, *PDSI_HSA_WC_REG;


typedef struct
{
    unsigned 	HBP_WC		: 12;
    unsigned 	rsv_12		: 20;		
} DSI_HBP_WC_REG, *PDSI_HBP_WC_REG;


typedef struct
{
    unsigned 	HFP_WC		: 12;
    unsigned 	rsv_12		: 20;		
} DSI_HFP_WC_REG, *PDSI_HFP_WC_REG;

typedef struct
{
    unsigned 	BLLP_WC		: 12;
    unsigned 	rsv_12		: 20;		
} DSI_BLLP_WC_REG, *PDSI_BLLP_WC_REG;

typedef struct
{
    unsigned 	CMDQ_SIZE	: 6;
    unsigned 	rsv_6		: 26;		
} DSI_CMDQ_CTRL_REG, *PDSI_CMDQ_CTRL_REG;

typedef struct
{
    unsigned char byte0;
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;	
} DSI_RX_DATA_REG, *PDSI_RX_DATA_REG;


typedef struct
{
    unsigned DSI_RACK	        : 1;
    unsigned DSI_RACK_BYPASS	: 1;
    unsigned rsv2		: 30;	
} DSI_RACK_REG, PDSI_RACK_REG;


typedef struct
{
    unsigned TRIG0		: 1;//remote rst
    unsigned TRIG1		: 1;//TE
    unsigned TRIG2		: 1;//ack	
    unsigned TRIG3		: 1;//rsv
    unsigned RX_ULPS    : 1;
    unsigned DIRECTION  : 1;
    unsigned rsv6		: 26;
} DSI_TRIG_STA_REG, *PDSI_TRIG_STA_REG;


typedef struct
{
    unsigned RWMEM_CONTI	: 16;
    unsigned rsv16          : 16;
} DSI_MEM_CONTI_REG, *PDSI_MEM_CONTI_REG;


typedef struct
{
    unsigned FRM_BC		: 21;
    unsigned rsv21		: 11;
} DSI_FRM_BC_REG, *PDSI_FRM_BC_REG;


typedef struct
{
    unsigned PHY_RST	: 1;
    unsigned rsv1		: 4;
    unsigned HTXTO_RST	: 1;
    unsigned LRXTO_RST	: 1;
    unsigned BTATO_RST	: 1;	
    unsigned rsv8		: 24;	
} DSI_PHY_CON_REG, *PDSI_PHY_CON_REG;


typedef struct
{
    unsigned LC_HS_TX_EN	: 1;
    unsigned LC_ULPM_EN		: 1;
    unsigned LC_WAKEUP_EN	: 1;	
    unsigned rsv3			: 29;
} DSI_PHY_LCCON_REG, *PDSI_PHY_LCCON_REG;


typedef struct
{
    unsigned L0_HS_TX_EN	: 1;
    unsigned L0_ULPM_EN		: 1;
    unsigned L0_WAKEUP_EN	: 1;	
    unsigned rsv3			: 29;
} DSI_PHY_LD0CON_REG, *PDSI_PHY_LD0CON_REG;


typedef struct
{
    unsigned char LPX;
    unsigned char HS_PRPR;
    unsigned char HS_ZERO;
    unsigned char HS_TRAIL;
} DSI_PHY_TIMCON0_REG, *PDSI_PHY_TIMCON0_REG;


typedef struct
{
    unsigned char TA_GO;
    unsigned char TA_SURE;
    unsigned char TA_GET;
    unsigned char DA_HS_EXIT;
} DSI_PHY_TIMCON1_REG, *PDSI_PHY_TIMCON1_REG;


typedef struct
{
    unsigned char CONT_DET;
    unsigned char rsv8;
    unsigned char CLK_ZERO;
    unsigned char CLK_TRAIL;
} DSI_PHY_TIMCON2_REG, *PDSI_PHY_TIMCON2_REG;


typedef struct
{
    unsigned char CLK_HS_PRPR;
    unsigned char CLK_HS_POST;
    unsigned char CLK_HS_EXIT;
    unsigned 	  rsv24		: 8;
} DSI_PHY_TIMCON3_REG, *PDSI_PHY_TIMCON3_REG;


typedef struct
{
    unsigned ULPS_WAKEUP	: 20;	
    unsigned rsv20			: 12;
} DSI_PHY_TIMCON4_REG, *PDSI_PHY_TIMCON4_REG;


typedef struct
{
    DSI_PHY_TIMCON0_REG	CTRL0;
    DSI_PHY_TIMCON1_REG	CTRL1;
    DSI_PHY_TIMCON2_REG	CTRL2;
    DSI_PHY_TIMCON3_REG	CTRL3;
} DSI_PHY_TIMCON_REG, *PDSI_PHY_TIMCON_REG;


typedef struct
{
    unsigned			CHECK_SUM	: 16;
    unsigned			rsv16       	: 16;
} DSI_CKSM_OUT_REG, *PDSI_CKSM_OUT_REG;


typedef struct
{
    unsigned			DPHY_CTL_STATE_C	: 9;
    unsigned            rsv9        : 7;
    unsigned			DPHY_HS_TX_STATE_C	: 5;
    unsigned 	  	rsv21				: 11;	
} DSI_STATE_DBG0_REG, *PDSI_STATE_DBG0_REG;


typedef struct
{
    unsigned			CTL_STATE_C	: 15;
    unsigned 	  	rsv15					: 1;				
    unsigned			HS_TX_STATE_0	: 5;
    unsigned			rsv21       	: 3;
    unsigned			ESC_STATE_0			: 8;							
} DSI_STATE_DBG1_REG, *PDSI_STATE_DBG1_REG;


typedef struct
{
    unsigned			RX_ESC_STATE			: 10;
    unsigned 	  	rsv10					: 6;
    unsigned			TA_T2R_STATE	: 5;
    unsigned 	  	rsv21					: 3;					
    unsigned			TA_R2T_STATE	: 5;
    unsigned 	  	rsv29					: 3;					
} DSI_STATE_DBG2_REG, *PDSI_STATE_DBG2_REG;


typedef struct
{
    unsigned			CTL_STATE_1			: 5;
    unsigned 	  	rsv5						: 3;	
    unsigned			HS_TX_STATE_1				: 5;	
    unsigned 	  	rsv13						: 3;
    unsigned			CTL_STATE_2 : 5;		
    unsigned 	  	rsv21						: 3;
    unsigned			HS_TX_STATE_2			: 5;
    unsigned 	  	rsv29						: 3;
} DSI_STATE_DBG3_REG, *PDSI_STATE_DBG3_REG;


typedef struct
{
    unsigned			CTL_STATE_3    	: 5;
    unsigned			rsv5						: 3;
    unsigned			HS_TX_STATE_3	: 5;
    unsigned			rsv13						: 19;
} DSI_STATE_DBG4_REG, *PDSI_STATE_DBG4_REG;


typedef struct
{
    unsigned			WAKEUP_CNT    	: 20;
    unsigned			rsv20						: 8;
    unsigned			WAKEUP_STATE	: 4;
} DSI_STATE_DBG5_REG, *PDSI_STATE_DBG5_REG;


typedef struct
{
    unsigned			CMTRL_STATE    	: 14;
    unsigned			rsv14						: 2;
    unsigned			CMDQ_STATE	: 6;
    unsigned			rsv22						: 10;
} DSI_STATE_DBG6_REG, *PDSI_STATE_DBG6_REG;


typedef struct
{
    unsigned			VMCTL_STATE    	: 11;
    unsigned			rsv11						: 1;
    unsigned			VFP_PERIOD	: 1;
    unsigned			VACT_PERIOD	: 1;
    unsigned			VBP_PERIOD	: 1;
    unsigned			VSA_PERIOD	: 1;
    unsigned			rsv16						: 16;
} DSI_STATE_DBG7_REG, *PDSI_STATE_DBG7_REG;


typedef struct
{
    unsigned			WORD_COUNTER    	: 14;
    unsigned			rsv14						: 18;
} DSI_STATE_DBG8_REG, *PDSI_STATE_DBG8_REG;


typedef struct
{
    unsigned			LINE_COUNTER    	: 22;
    unsigned			rsv22						: 10;
} DSI_STATE_DBG9_REG, *PDSI_STATE_DBG9_REG;


typedef struct
{
    unsigned			DEBUG_OUT_SEL    	: 5;
    unsigned			rsv5						: 27;
} DSI_DEBUG_SEL_REG, *PDSI_DEBUG_SEL_REG;


typedef struct
{
    unsigned            BIST_MODE           : 1;
    unsigned            BIST_ENABLE         : 1;
    unsigned            BIST_FIX_PATTERN    : 1;
    unsigned            BIST_SPC_PATTERN    : 1;
    unsigned            BIST_HS_FREE        : 1;
    unsigned            SW_CTL_EN           : 1;
    unsigned            PLL_CK_MON          : 1;
    unsigned            rsv7                : 1;
    unsigned            BIST_LANE_NUM       : 4;
    unsigned            rsv12               : 4;
    unsigned            BIST_TIMING         : 8;
    unsigned            rsv24         : 8;
}DSI_BIST_CON, *PDSI_BIST_CON;

typedef struct
{
    unsigned			VM_CMD_EN			:1;
    unsigned			LONG_PKT			:1;
    unsigned			TIME_SEL			:1;
    unsigned			TS_VSA_EN			:1;
    unsigned			TS_VBP_EN			:1;
    unsigned			TS_VFP_EN			:1;
    unsigned			rsv6				:2;
    unsigned			CM_DATA_ID			:8;
    unsigned			CM_DATA_0			:8;
    unsigned			CM_DATA_1			:8;
}DSI_VM_CMD_CON_REG, *PDSI_VM_CMD_CON_REG;

typedef struct
{
    DSI_START_REG				DSI_START;				// 0000
    DSI_STATUS_REG  			DSI_STA;					// 0004
    DSI_INT_ENABLE_REG	DSI_INTEN;				// 0008
    DSI_INT_STATUS_REG	DSI_INTSTA;				// 000C
    DSI_COM_CTRL_REG		DSI_COM_CTRL;			// 0010
    DSI_MODE_CTRL_REG		DSI_MODE_CTRL;		// 0014
    DSI_TXRX_CTRL_REG		DSI_TXRX_CTRL;		// 0018
    DSI_PSCTRL_REG			DSI_PSCTRL;				// 001C
    DSI_VSA_NL_REG			DSI_VSA_NL;				// 0020
    DSI_VBP_NL_REG			DSI_VBP_NL;				// 0024
    DSI_VFP_NL_REG			DSI_VFP_NL;				// 0028
    DSI_VACT_NL_REG			DSI_VACT_NL;			// 002C
    UINT32                  rsv_30[8];               // 0030..004C
    DSI_HSA_WC_REG			DSI_HSA_WC;				// 0050
    DSI_HBP_WC_REG			DSI_HBP_WC;				// 0054
    DSI_HFP_WC_REG			DSI_HFP_WC;				// 0058
    DSI_BLLP_WC_REG			DSI_BLLP_WC;				// 005C
    
    DSI_CMDQ_CTRL_REG		DSI_CMDQ_SIZE;		// 0060
    UINT32                  DSI_HSTX_CKL_WC;    // 0064
    UINT32							rsv_0068[3];      // 0068..0070
    DSI_RX_DATA_REG			DSI_RX_DATA0;			// 0074
    DSI_RX_DATA_REG			DSI_RX_DATA1;			// 0078
    DSI_RX_DATA_REG			DSI_RX_DATA2;			// 007c
    DSI_RX_DATA_REG			DSI_RX_DATA3;			// 0080
    DSI_RACK_REG				DSI_RACK;					// 0084
    DSI_TRIG_STA_REG		DSI_TRIG_STA;			// 0088
    UINT32   	        	rsv_008C;      // 008C
    DSI_MEM_CONTI_REG		DSI_MEM_CONTI;		// 0090
    DSI_FRM_BC_REG			DSI_FRM_BC;				// 0094
    UINT32   	        	rsv_0098[27];     // 0098..0100
    DSI_PHY_LCCON_REG		DSI_PHY_LCCON;		// 0104	
    DSI_PHY_LD0CON_REG	DSI_PHY_LD0CON;		// 0108	
    UINT32   	        	rsv_010C;      // 010C
    DSI_PHY_TIMCON0_REG	DSI_PHY_TIMECON0;	// 0110	
    DSI_PHY_TIMCON1_REG	DSI_PHY_TIMECON1;	// 0114
    DSI_PHY_TIMCON2_REG	DSI_PHY_TIMECON2;	// 0118
    DSI_PHY_TIMCON3_REG	DSI_PHY_TIMECON3;	// 011C
    DSI_PHY_TIMCON4_REG	DSI_PHY_TIMECON4;	// 0120
    UINT32   	        	rsv_0124[3];      // 0124..012c
    DSI_VM_CMD_CON_REG      DSI_VM_CMD_CON;		//0130
    UINT32                  DSI_VM_CMD_DATA0;    // 0134
    UINT32                  DSI_VM_CMD_DATA4;    // 0138
    UINT32                  DSI_VM_CMD_DATA8;    // 013C
    UINT32                  DSI_VM_CMD_DATAC;    // 0140
    DSI_CKSM_OUT_REG		DSI_CKSM_OUT;			// 0144
    DSI_STATE_DBG0_REG	DSI_STATE_DBG0;		// 0148
    DSI_STATE_DBG1_REG	DSI_STATE_DBG1;		// 014C
    DSI_STATE_DBG2_REG	DSI_STATE_DBG2;		// 0150
    DSI_STATE_DBG3_REG	DSI_STATE_DBG3;		// 0154
    DSI_STATE_DBG4_REG	DSI_STATE_DBG4;		// 0158
    DSI_STATE_DBG5_REG	DSI_STATE_DBG5;		// 015C
    DSI_STATE_DBG6_REG	DSI_STATE_DBG6;		// 0160
    DSI_STATE_DBG7_REG	DSI_STATE_DBG7;		// 0164
    DSI_STATE_DBG8_REG	DSI_STATE_DBG8;		// 0168
    DSI_STATE_DBG9_REG	DSI_STATE_DBG9;		// 016C
    DSI_DEBUG_SEL_REG	DSI_DEBUG_SEL;		// 0170
    UINT32    rsv174;                        // 0174
    UINT32    BIST_PATTERN;                         // 0178
    DSI_BIST_CON BIST_CON;                          // 017C
} volatile DSI_REGS, *PDSI_REGS;


typedef struct
{
    unsigned char byte0;
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
} DSI_CMDQ, *PDSI_CMDQ;

typedef struct
{
    DSI_CMDQ data[32];
} DSI_CMDQ_REGS, *PDSI_CMDQ_REGS;

typedef struct
{
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
} DSI_VM_CMDQ, *PDSI_VM_CMDQ;

typedef struct
{
	DSI_VM_CMDQ data[4];
} DSI_VM_CMDQ_REGS, *PDSI_VM_CMDQ_REGS;

#ifndef BUILD_LK
STATIC_ASSERT(0x0050 == offsetof(DSI_PHY_REGS, MIPITX_DSI_PLL_CON0));
STATIC_ASSERT(0x0070 == offsetof(DSI_PHY_REGS, MIPITX_DSI_RGS));
STATIC_ASSERT(0x0080 == offsetof(DSI_PHY_REGS, MIPITX_DSI_SW_CTRL));
STATIC_ASSERT(0x0090 == offsetof(DSI_PHY_REGS, MIPITX_DSI_DBG_CON));
STATIC_ASSERT(0x002C == offsetof(DSI_REGS, DSI_VACT_NL));
STATIC_ASSERT(0x0104 == offsetof(DSI_REGS, DSI_PHY_LCCON));
STATIC_ASSERT(0x011C == offsetof(DSI_REGS, DSI_PHY_TIMECON3));
STATIC_ASSERT(0x017C == offsetof(DSI_REGS, BIST_CON));
#endif

#ifdef __cplusplus
}
#endif

#endif // __DSI_REG_H__

