#ifndef __MT_FHREG_H__
#define __MT_FHREG_H__

#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>

///////////////////////////////////////////////////////////////
//- Register Definition

//- DMA
#define FHDMA_CFG			(FHCTL_BASE + 0x0000)
#define FHDMA_2G1BASE		(FHCTL_BASE + 0x0004)
#define FHDMA_2G2BASE		(FHCTL_BASE + 0x0008)
#define FHDMA_INTMDBASE		(FHCTL_BASE + 0x000C)
#define FHDMA_EXTMDBASE		(FHCTL_BASE + 0x0010)
#define FHDMA_BTBASE		(FHCTL_BASE + 0x0014)
#define FHDMA_WFBASE		(FHCTL_BASE + 0x0018)
#define FHDMA_FMBASE		(FHCTL_BASE + 0x001C)

#define FHDMA_RFx_BASE(rFID)	(FHCTL_BASE + 0x0004 + (rFID * 4))

#define RG_FHDMA_MODE_BIT		8
#define RG_FHDMA_MODE_MASK		0x1

//- SRAM
#define FHSRAM_CON			(FHCTL_BASE + 0x0020)
#define FHSRAM_WR			(FHCTL_BASE + 0x0024)
#define FHSRAM_RD			(FHCTL_BASE + 0x0028)

//- FHCTL
//- Common Part
#define FHCTL_CFG			(FHCTL_BASE + 0x002C)
#define FHCTL_CON			(FHCTL_BASE + 0x0030)
#define FHCTL_2G1_CH		(FHCTL_BASE + 0x0034)
#define FHCTL_2G2_CH		(FHCTL_BASE + 0x0038)
#define FHCTL_INTMD_CH		(FHCTL_BASE + 0x003C)
#define FHCTL_EXTMD_CH		(FHCTL_BASE + 0x0040)
#define FHCTL_BT_CH			(FHCTL_BASE + 0x0044)
#define FHCTL_WF_CH			(FHCTL_BASE + 0x0048)
#define FHCTL_FM_CH			(FHCTL_BASE + 0x004C)

#define FHCTL_RFx_CH(rFID)	(FHCTL_BASE + 0x0034 + (rFID * 4))

//- FHCTL
//- PLLx CTL
#define FHCTL0_CFG 			(FHCTL_BASE + 0x0050)
#define FHCTL0_UPDNLMT		(FHCTL_BASE + 0x0054)
#define FHCTL0_DDS 			(FHCTL_BASE + 0x0058)
#define FHCTL0_DVFS 		(FHCTL_BASE + 0x005C)
#define FHCTL0_MON			(FHCTL_BASE + 0x0060)
#define FHCTL1_CFG 			(FHCTL_BASE + 0x0064)
#define FHCTL1_UPDNLMT		(FHCTL_BASE + 0x0068)
#define FHCTL1_DDS	 		(FHCTL_BASE + 0x006C)
#define FHCTL1_DVFS 		(FHCTL_BASE + 0x0070)
#define FHCTL1_MON			(FHCTL_BASE + 0x0074)

#define FHCTLx_CFG(pLLID) 		(FHCTL_BASE + 0x0050 + (pLLID * 0x14))
#define FHCTLx_UPDNLMT(pLLID)	(FHCTL_BASE + 0x0054 + (pLLID * 0x14))
#define FHCTLx_DDS(pLLID) 		(FHCTL_BASE + 0x0058 + (pLLID * 0x14))
#define FHCTLx_DVFS(pLLID) 		(FHCTL_BASE + 0x005C + (pLLID * 0x14))
#define FHCTLx_MON(pLLID)		(FHCTL_BASE + 0x0060 + (pLLID * 0x14))

#define RG_FRDDSx_UPLMT_BIT			0
#define RG_FRDDSx_DNLMT_BIT			16

#define RG_FRDDSx_UPLMT_MASK		0xFFFF
#define RG_FRDDSx_DNLMT_MASK		0xFFFF

#define NUM_OF_FHCTL_REGS			30
///////////////////////////////////////////////////////////////
//- Registers Default Value

#define FHDMA_CFG_DEFAULT				0x00200000
#define FHDMA_2G1BASE_DEFAULT			0x80000000
#define	FHDMA_2G2BASE_DEFAULT         	0x80000000
#define	FHDMA_INTMDBASE_DEFAULT      	0x80000000
#define	FHDMA_EXTMDBASE_DEFAULT       	0x80000000
#define	FHDMA_BTBASE_DEFAULT			0x80000000
#define	FHDMA_WFBASE_DEFAULT			0x80000000
#define	FHDMA_FMBASE_DEFAULT			0x80000000
//SRAM
#define	FHSRAM_CON_DEFAULT				0x00000000
#define	FHSRAM_WR_DEFAULT				0x00000000
#define	FHSRAM_RO_DEFAULT				0x00000000
//- Common Part
#define	FHCTL_CFG_DEFAULT				0x00000000
#define FHCTL_CON_DEFAULT				0x06003C97
#define	FHCTL_2G1_CH_DEFAULT			0x00000000
#define	FHCTL_2G2_CH_DEFAULT			0x00000000
#define	FHCTL_INTMD_CH_DEFAULT			0x00000000
#define	FHCTL_EXTMD_CH_DEFAULT			0x00000000
#define	FHCTL_BT_CH_DEFAULT				0x00000000
#define	FHCTL_WF_CH_DEFAULT				0x00000000
#define	FHCTL_FM_CH_DEFAULT				0x00000000
	//--- FHCTLx
#define FHCTLx_CFG_DEFAULT     			0x00000000
#define FHCTLx_UPDNLMT_DEFAULT 			0x00000000
#define FHCTLx_DDS_DEFAULT     			0x00000000
#define FHCTLx_DVFS_DEFAULT    			0x00000000
#define FHCTLx_MON_DEFAULT     			0x00200000

#define RG_FHCTLx_PLL_TGL_ORG_BIT		31
#define RG_FHCTLx_PLL_ORG_BIT			0

#define RG_FHCTLx_PLL_TGL_ORG_MASK		0x1
#define RG_FHCTLx_PLL_ORG_MASK			0x1FFFFF

#define RG_FHCTLx_PLL_DVFS_TRI_BIT		31
#define RG_FHCTLx_PLL_DVFS_BIT			0

#define RG_FHCTLx_PLL_DVFS_TRI_MASK		0x1
#define RG_FHCTLx_PLL_DVFS_MASK			0x1FFFFF

#define RG_FHCTLx_DDS_MASK				0x1FFFFF


///////////////////////////////////////////////////////////////
//- Bit-field of Register Definition

//- register FHSRAM_CON
#define RG_UPSRAM_CE_BIT			9
#define RG_UPSRAM_RW_ADDR_BIT		0

#define RG_UPSRAM_CE_MASK			0x1
#define RG_UPSRAM_RW_ADDR_MASK 		0xFF

//- register FHCTL_CON
#define RG_FHCTL_SFDT_BIT			24
#define RG_FHCTL_SFDY_BIT			0

#define RG_FHCTL_SFDT_MASK			0xFF
#define RG_FHCTL_SFDY_MASK			0x1FFFFF

//- register FHCTL_CFG
#define RG_FHCTL_FM_ON_BIT				0
#define RG_FHCTL_WF_ON_BIT				1
#define RG_FHCTL_BT_ON_BIT 				2
#define RG_FHCTL_EXTMD_OLD_ON_BIT		3
#define RG_FHCTL_INTMD_OLD_ON_BIT		4
#define RG_FHCTL_EXTMD_ON_BIT			5
#define RG_FHCTL_INTGMD_ON_BIT			6
#define RG_FHCTL_2G2_ON_BIT				7
#define RG_FHCTL_2G1_ON_BIT				8
#define RG_FHCTL_ORDER_MD_BIT			16
#define RG_FHCTL_ORDER_BIT				24

#define RG_FHCTL_FM_ON_MASK				0x1
#define RG_FHCTL_WF_ON_MASK				0x1
#define RG_FHCTL_BT_ON_MASK 			0x1
#define RG_FHCTL_EXTMD_OLD_ON_MASK		0x1
#define RG_FHCTL_INTMD_OLD_ON_MASK		0x1
#define RG_FHCTL_EXTMD_ON_MASK			0x1
#define RG_FHCTL_INTGMD_ON_MASK			0x1
#define RG_FHCTL_2G2_ON_MASK			0x1
#define RG_FHCTL_2G1_ON_MASK			0x1
#define RG_FHCTL_ORDER_MD_MASK			0x1F
#define RG_FHCTL_ORDER_MASK				0x1F

//- register FHCTLx_CFG
#define RG_FHCTLx_EN_BIT				0
#define RG_FRDDSx_EN_BIT				1 //- free-run
#define RG_SFSTRx_EN_BIT				2 //- soft-start
#define RG_SFSTRx_BP_BIT				4
#define RG_FHCTLx_SRHMODE_BIT			5
#define RG_FHCTLx_PAUSE_BIT				8
#define RG_FRDDSx_DTS_BIT				16
#define RG_FRDDSx_DYS_BIT				20

#define RG_FHCTLx_EN_MASK				0x1
#define RG_FRDDSx_EN_MASK				0x1	//- free-run
#define RG_SFSTRx_EN_MASK				0x1	//- soft-start
#define RG_SFSTRx_BP_MASK				0x1
#define RG_FHCTLx_SRHMODE_MASK			0x1
#define RG_FHCTLx_PAUSE	_MASK			0x1
#define RG_FRDDSx_DTS_MASK				0xF
#define RG_FRDDSx_DYS_MASK				0xF

//- register FHCTLx_UPDNLMT
#define RG_FRDDSx_DNLMT_BIT				16
#define RG_FRDDSx_UPLMT_BIT				0

#define RG_FRDDSx_DNLMT_MASK			0xFFFF
#define RG_FRDDSx_UPLMT_MASK			0xFFFF

///////////////////////////////////////////////////////////////
//- SRAM Info

#define SRAM_TABLE_SIZE_BY_PLL	64 //- unit : 4Bytes (DDS)


//- PLLGP DDS register
#ifndef APMIXED_BASE
#define APMIXED_BASE			0x10205000
#endif
#if 0
#define PLL_HP_CON0 			(APMIXED_BASE + 0x0014)
#define ARMPLL_CON0 			(APMIXED_BASE + 0x0100)
#define ARMPLL_CON1 			(APMIXED_BASE + 0x0104)
#define ARMPLL_CON2 			(APMIXED_BASE + 0x0108)
#define MAINPLL_CON0 			(APMIXED_BASE + 0x0120)
#define MAINPLL_CON1 			(APMIXED_BASE + 0x0124)
#define MAINPLL_CON2 			(APMIXED_BASE + 0x0128)
#endif
#define RG_ARMPLL_SDM_PCW_CHG_BIT		31
#define RG_ARMPLL_POSDIV_BIT			24
#define RG_ARMPLL_SDM_PCW				0

#define RG_ARMPLL_SDM_PCW_CHG_MASK		0x1
#define RG_ARMPLL_POSDIV_MASK			0x7
#define RG_ARMPLL_SDM_PCW_MASK			0x1FFFFF


#endif //- !__MT_FHREG_H__







































