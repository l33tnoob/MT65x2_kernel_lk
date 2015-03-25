#ifndef _MT_DCM_H
#define _MT_DCM_H

#include "mach/mt_reg_base.h"


#if 1
//#define USB0_BASE 					(0x11200000)  0xF1200000
//#define MSDC0_BASE 					(0x11230000)  0xF1230000
//#define MSDC1_BASE 					(0x11240000)  0xF1240000
//#define MSDC2_BASE 					(0x11250000)  0xF1250000

//#define PWRAP_BASE 					(0x1000D000)  0xF000D000
//#define SMI_COMMON_AO_BASE 			(0x1000C000)#define SMI1_BASE (0xF000C000)

// APB Module i2c
//#define I2C0_BASE 					(0x11007000)#define I2C0_BASE (0xF1007000)
//#define I2C1_BASE 					(0x11008000)#define I2C1_BASE (0xF1008000)
//#define I2C2_BASE 					(0x11009000)#define I2C2_BASE (0xF1009000)

//#define CPUSYS_DCM_BASE 			0x10200000#define MCUSYS_CFGREG_BASE (0xF0200000)

//#define TOPCKGEN_DCM_BASE			0x10000000#define INFRA_BASE (0xF0000000)
//#define INFRA_DCM_BASE 				0x10001000#define INFRACFG_AO_BASE (0xF0001000)
//#define PERISYS_DCM_BASE 		 	0x10003000#define PERICFG_BASE (0xF0003000)
//#define DRAMC0_DCM_BASE         	0x10004000#define DRAMC0_BASE (0xF0004000)
//#define M4U_BASE                 	0x10205000#define SMI_MMU_TOP_BASE (0xF0205000)

//#define SMILARB0_BASE           	0x14010000#define SMI_LARB0_BASE (0xF4010000)
//#define SMILARB1_BASE           	0x16010000#define SMI_LARB1_BASE (0xF6010000)
//#define SMILARB2_BASE           	0x15001000#define SMI_LARB3_BASE (0xF5001000)
//#define MFG_TOP_BASE				0x13000000#define G3D_CONFIG_BASE (0xF3000000)
#define CAM_BASE                	0xF5004000//0x15004000
//#define JPGENC_DCM 					0x1500A000#define JPGENC_BASE (0xF500A000)

//#define MMSYS_CONFIG_BASE			0x14000000#define DISPSYS_BASE (0xF4000000)
//#define VENC_DCM_BASE				0x15009000#define VENC_BASE (0xF5009000)
//#define VDEC_DCM_BASE 				0x16000000#define VDEC_GCON_BASE (0xF6000000)

#endif

#if 1


// APB Module usb2
#define	PERI_USB0_DCM		    	(USB_BASE+0x700)//check 82

// APB Module msdc
#define MSDC0_IP_DCM				(MSDC_0_BASE + 0x00B4)//check 82

// APB Module msdc
#define MSDC1_IP_DCM				(MSDC_1_BASE + 0x00B4)//check 82

// APB Module msdc
#define MSDC2_IP_DCM				(MSDC_2_BASE + 0x00B4)//check 82

// APB Module pmic_wrap
#define PMIC_WRAP_DCM_EN			(PWRAP_BASE+0x13C)//check 82


// APB Module i2c
#define I2C0_I2CREG_HW_CG_EN		((I2C0_BASE+0x054))//check 82

// APB Module i2c
#define I2C1_I2CREG_HW_CG_EN		((I2C1_BASE+0x054))//check 82

// APB Module i2c
#define I2C2_I2CREG_HW_CG_EN		((I2C2_BASE+0x054))//check 82


//CPUSYS_dcm
#define CA7_MISC_CONFIG			(MCUSYS_CFGREG_BASE + 0x005C) // check 82
#define MCU_BIU_CON				(MCUSYS_CFGREG_BASE + 0x8000)


// AXI bus dcm
//TOPCKGen_dcm
#define DCM_CFG                 (INFRA_BASE + 0x0004)//DCM_CFG//check 82
#define CLK_SCP_CFG_0			(INFRA_BASE + 0x0200)//82 Y,89 N//check 82
#define CLK_SCP_CFG_1			(INFRA_BASE + 0x0204)//82 Y,89 N//check 82

//CA7 DCM
#define TOP_CKDIV1				(INFRACFG_AO_BASE + 0x008) //Only 82 support//check 82
#define TOP_DCMCTL              (INFRACFG_AO_BASE + 0x0010) //INFRA_TOPCKGEN_DCMCTL//check 82
#define TOP_DCMDBC              (INFRACFG_AO_BASE + 0x0014) //INFRA_TOPCKGEN_DCMDBC//check 82

//82 N,89 Y
//#define TOP_CA7DCMFSEL          (INFRA_DCM_BASE + 0x0018)//INFRA_TOPCKGEN_DCMFSEL

//infra dcm
#define INFRA_DCMCTL            (INFRACFG_AO_BASE + 0x0050) //INFRA_GLOBALCON_DCMCTL//check 82
#define INFRA_DCMDBC            (INFRACFG_AO_BASE + 0x0054) //INFRA_GLOBALCON_DCMDBC//check 82
#define INFRA_DCMFSEL           (INFRACFG_AO_BASE + 0x0058) //INFRA_GLOBALCON_DCMFSEL//check 82


//peri dcm
#define PERI_GLOBALCON_DCMCTL        (PERICFG_BASE + 0x0050) //PERI_GLOBALCON_DCMCTL//check 82
#define PERI_GLOBALCON_DCMDBC        (PERICFG_BASE + 0x0054) //PERI_GLOBALCON_DCMDBC//check 82
#define PERI_GLOBALCON_DCMFSEL       (PERICFG_BASE + 0x0058) //PERI_GLOBALCON_DCMFSEL//check 82


#define DRAMC_PD_CTRL           (DRAMC0_BASE + 0x01DC) //not found in 82 API.c//check 82

//m4u dcm
#define MMU_DCM					(SMI_MMU_TOP_BASE+0x5f0)//check 82//check 82

//smi_common dcm
//#define SMI_COMMON_DCM          0x10202300 //HW_DCM API_17


//Smi_common dcm
#define SMI_DCM_CONTROL				0xF4011300//check 82//0x14011300//check 82


// APB Module smi
//Smi_secure dcm
#define SMI_COMMON_AO_SMI_CON		(SMI1_BASE+0x010)//SMI_CON//check 82
#define SMI_COMMON_AO_SMI_CON_SET	(SMI1_BASE+0x014)//SMI_CON_SET//check 82
#define SMI_COMMON_AO_SMI_CON_CLR	(SMI1_BASE+0x018)//SMI_CON_CLR//check 82



// APB Module smi_larb
#define SMILARB0_DCM_STA        (SMI_LARB0_BASE + 0x00)//SMI_LARB0_STAT//check 82
#define SMILARB0_DCM_CON        (SMI_LARB0_BASE + 0x10)//SMI_LARB0_CON//check 82
#define SMILARB0_DCM_SET        (SMI_LARB0_BASE + 0x14)//SMI_LARB0_CON_SET//check 82
#define SMILARB0_DCM_CLR        (SMI_LARB0_BASE + 0x18)//SMI_LARB0_CON_CLR//check 82

#define SMILARB1_DCM_STA        (SMI_LARB1_BASE + 0x00)//SMI_LARB1_STAT//check 82
#define SMILARB1_DCM_CON        (SMI_LARB1_BASE + 0x10)//SMI_LARB1_CON//check 82
#define SMILARB1_DCM_SET        (SMI_LARB1_BASE + 0x14)//SMI_LARB1_CON_SET//check 82
#define SMILARB1_DCM_CLR        (SMI_LARB1_BASE + 0x18)//SMI_LARB1_CON_CLR//check 82

#define SMILARB2_DCM_STA        (SMI_LARB3_BASE + 0x00)//SMI_LARB2_STAT//check 82
#define SMILARB2_DCM_CON        (SMI_LARB3_BASE + 0x10)//SMI_LARB2_CON//check 82
#define SMILARB2_DCM_SET        (SMI_LARB3_BASE + 0x14)//SMI_LARB2_CON_SET//check 82
#define SMILARB2_DCM_CLR        (SMI_LARB3_BASE + 0x18)//SMI_LARB2_CON_CLR//check 82


//MFG
//MFG_DCM
// APB Module mfg_top
#define MFG_DCM_CON_0            (G3D_CONFIG_BASE + 0x10) //MFG_DCM_CON_0//check 82

//smi_isp_dcm
#define CAM_CTL_RAW_DCM         (CAM_BASE + 0x190)//CAM_CTL_RAW_DCM_DIS//check 82
#define CAM_CTL_RGB_DCM         (CAM_BASE + 0x194)//CAM_CTL_RGB_DCM_DIS//check 82
#define CAM_CTL_YUV_DCM         (CAM_BASE + 0x198)//CAM_CTL_YUV_DCM_DIS//check 82
#define CAM_CTL_CDP_DCM         (CAM_BASE + 0x19C)//CAM_CTL_CDP_DCM_DIS//check 82
#define CAM_CTL_DMA_DCM			(CAM_BASE + 0x1B0)//CAM_CTL_DMA_DCM_DIS//check 82

#define CAM_CTL_RAW_DCM_STA     (CAM_BASE + 0x1A0)//CAM_CTL_RAW_DCM_STATUS//check 82
#define CAM_CTL_RGB_DCM_STA     (CAM_BASE + 0x1A4)//CAM_CTL_RGB_DCM_STATUS//check 82
#define CAM_CTL_YUV_DCM_STA     (CAM_BASE + 0x1A8)//CAM_CTL_YUV_DCM_STATUS//check 82
#define CAM_CTL_CDP_DCM_STA     (CAM_BASE + 0x1AC)//CAM_CTL_CDP_DCM_STATUS//check 82
#define CAM_CTL_DMA_DCM_STA     (CAM_BASE + 0x1B4)//CAM_CTL_DMA_DCM_STATUS//check 82


//#define JPGDEC_DCM_CTRL         0x15009300 //not found in 82 API.c
#define JPGENC_DCM_CTRL         (JPGENC_BASE + 0x300) //not found in 82 API.c//check 82

//#define SMI_ISP_COMMON_DCMCON   0x15003010  	//82 N 89 Y
//#define SMI_ISP_COMMON_DCMSET   0x15003014	//82 N 89 Y
//#define SMI_ISP_COMMON_DCMCLR   0x15003018	//82 N 89 Y

//display sys
//mmsys_dcm
// APB Module mmsys_config
#define DISP_HW_DCM_DIS0        (DISPSYS_BASE + 0x120)//MMSYS_HW_DCM_DIS0//check 82
#define DISP_HW_DCM_DIS_SET0    (DISPSYS_BASE + 0x124)//MMSYS_HW_DCM_DIS_SET0//check 82
#define DISP_HW_DCM_DIS_CLR0    (DISPSYS_BASE + 0x128)//MMSYS_HW_DCM_DIS_CLR0//check 82

#define DISP_HW_DCM_DIS1        (DISPSYS_BASE + 0x12C)//MMSYS_HW_DCM_DIS1//check 82
#define DISP_HW_DCM_DIS_SET1    (DISPSYS_BASE + 0x130)//MMSYS_HW_DCM_DIS_SET1//check 82
#define DISP_HW_DCM_DIS_CLR1    (DISPSYS_BASE + 0x134)//MMSYS_HW_DCM_DIS_CLR1//check 82

//venc sys
#define VENC_CE                 (VENC_BASE + 0xEC)//not found in 82 API.c//check 82
#define VENC_CLK_DCM_CTRL       (VENC_BASE + 0xF4)//not found in 82 API.c//check 82
#define VENC_CLK_CG_CTRL        (VENC_BASE + 0x94)//not found in 82 API.c//check 82
//#define VENC_MP4_DCM_CTRL       0x170026F0

//vdec
//VDEC_dcm
#define VDEC_DCM_CON            (VDEC_GCON_BASE + 0x18)// found in 82 API.c,VDECSYS//check 82


#else
// AXI bus dcm
#define DCM_CFG                 (TOPRGU_BASE + 0x0104)

//CA7 DCM
#define TOP_DCMCTL              (INFRACFG_BASE + 0x0010)
#define TOP_DCMDBC              (INFRACFG_BASE + 0x0014)
#define TOP_CA7DCMFSEL          (INFRACFG_BASE + 0x0018)

//infra dcm
#define INFRA_DCMCTL            (INFRACFG_BASE + 0x0050)
#define INFRA_DCMDBC            (INFRACFG_BASE + 0x0054)
#define INFRA_DCMFSEL           (INFRACFG_BASE + 0x0058)

#define DRAMC_PD_CTRL           (DRAMC0_BASE + 0x01DC)

//peri dcm
#define PERI_GCON_DCMCTL        (PERICFG_BASE + 0x0050)
#define PERI_GCON_DCMDBC        (PERICFG_BASE + 0x0054)
#define PERI_GCON_DCMFSEL       (PERICFG_BASE + 0x0058)

//m4u dcm
#define M4U_DCM                 (SMI_MMU_TOP_BASE + 0x001C)

//smi_common dcm
#define SMI_COMMON_DCM          0xF0202300

//smi_secure dcm
#define SMI_SECURE_DCMCON       0xF000E010
#define SMI_SECURE_DCMSET       0xF000E014
#define SMI_SECURE_DCMCLR       0xF000E018

#define SMILARB0_BASE           0xF7001000
#define SMILARB1_BASE           0xF6010000
#define SMILARB3_BASE           0xF5001000
#define SMILARB4_BASE           0xF5002000


#define SMILARB0_DCM_STA        (SMILARB0_BASE + 0x00)
#define SMILARB0_DCM_CON        (SMILARB0_BASE + 0x10)
#define SMILARB0_DCM_SET        (SMILARB0_BASE + 0x14)
#define SMILARB0_DCM_CLR        (SMILARB0_BASE + 0x18)

#define SMILARB1_DCM_STA        (SMILARB1_BASE + 0x00)
#define SMILARB1_DCM_CON        (SMILARB1_BASE + 0x10)
#define SMILARB1_DCM_SET        (SMILARB1_BASE + 0x14)
#define SMILARB1_DCM_CLR        (SMILARB1_BASE + 0x18)

#define SMILARB2_DCM_STA        (SMILARB2_BASE + 0x00)
#define SMILARB2_DCM_CON        (SMILARB2_BASE + 0x10)
#define SMILARB2_DCM_SET        (SMILARB2_BASE + 0x14)
#define SMILARB2_DCM_CLR        (SMILARB2_BASE + 0x18)

#define SMILARB3_DCM_STA        (SMILARB3_BASE + 0x00)
#define SMILARB3_DCM_CON        (SMILARB3_BASE + 0x10)
#define SMILARB3_DCM_SET        (SMILARB3_BASE + 0x14)
#define SMILARB3_DCM_CLR        (SMILARB3_BASE + 0x18)

#define SMILARB4_DCM_STA        (SMILARB4_BASE + 0x00)
#define SMILARB4_DCM_CON        (SMILARB4_BASE + 0x10)
#define SMILARB4_DCM_SET        (SMILARB4_BASE + 0x14)
#define SMILARB4_DCM_CLR        (SMILARB4_BASE + 0x18)

//MFG
#define MFG_DCM_CON0            0xF0206010
#define MFG_DCM_CON1            0xF0206014

//smi_isp_dcm
#define CAM_BASE                0xF5004000
#define CAM_CTL_RAW_DCM         (CAM_BASE + 0x190)
#define CAM_CTL_RGB_DCM         (CAM_BASE + 0x194)
#define CAM_CTL_YUV_DCM         (CAM_BASE + 0x198)
#define CAM_CTL_CDP_DCM         (CAM_BASE + 0x19C)

#define CAM_CTL_RAW_DCM_STA     (CAM_BASE + 0x1A0)
#define CAM_CTL_RGB_DCM_STA     (CAM_BASE + 0x1A4)
#define CAM_CTL_YUV_DCM_STA     (CAM_BASE + 0x1A8)
#define CAM_CTL_CDP_DCM_STA     (CAM_BASE + 0x1AC)

#define JPGDEC_DCM_CTRL         0xF5009300
#define JPGENC_DCM_CTRL         0xF500A300

#define SMI_ISP_COMMON_DCMCON   0xF5003010
#define SMI_ISP_COMMON_DCMSET   0xF5003014
#define SMI_ISP_COMMON_DCMCLR   0xF5003018

//display sys
#define DISP_HW_DCM_DIS0        0xF4000120
#define DISP_HW_DCM_DIS_SET0    0xF4000124
#define DISP_HW_DCM_DIS_CLR0    0xF4000128

#define DISP_HW_DCM_DIS1        0xF4000130
#define DISP_HW_DCM_DIS_SET1    0xF4000134
#define DISP_HW_DCM_DIS_CLR1    0xF4000138

//venc sys
#define VENC_CE                 0xF70020EC
#define VENC_CLK_DCM_CTRL       0xF70020F4
#define VENC_CLK_CG_CTRL        0xF7002094
#define VENC_MP4_DCM_CTRL       0xF70026F0

//vdec
#define VDEC_DCM_CON            (VDEC_GCON_BASE + 0x0018)
#endif

#if 0
//#define CPU_DCM                 (0x01)
//#define BUS_DCM                 (0x02)
//#define TOP_DCM                 (0x03)
#define IFRA_DCM                (0x04)
#define PERI_DCM                (0x05)
#define SMI_DCM                 (0x06)
#define MFG_DCM                 (0x07)
#define DIS_DCM                 (0x08)
#define JPEG_DCM                (0x09)
#define VDE_DCM                 (0x10)
#define VEN_DCM                 (0x11)
//#define DRAM_DCM                (0x12)
#define TOPCKGEN_DCM            (0x13)
#define CPUSYS_DCM				(0x14)
#define MMSYS_DCM				(0x15)
#define CAM_DCM					(0x16)
#define CA7_DCM					(0x17)
//#define M4U_DCM					(0x18)
#define SMILARB_DCM				(0x19)
#else
#define CPU_DCM                 (1U << 0)
#define IFR_DCM                 (1U << 1)
#define PER_DCM                 (1U << 2)
#define SMI_DCM                 (1U << 3)
#define MFG_DCM                 (1U << 4)
#define DIS_DCM                 (1U << 5)
#define ISP_DCM                 (1U << 6)
#define VDE_DCM                 (1U << 7)
//#define SMILARB_DCM				(1U << 8)
#define TOPCKGEN_DCM			(1U << 8)
#define ALL_DCM                 (CPU_DCM|IFR_DCM|PER_DCM|SMI_DCM|MFG_DCM|DIS_DCM|ISP_DCM|VDE_DCM|TOPCKGEN_DCM)
#define NR_DCMS                 (0x9)
#endif

//extern void dcm_get_status(unsigned int type);
extern void dcm_enable(unsigned int type);
extern void dcm_disable(unsigned int type);

extern void bus_dcm_enable(void);
extern void bus_dcm_disable(void);

extern void disable_infra_dcm(void);
extern void restore_infra_dcm(void);

extern void disable_peri_dcm(void);
extern void restore_peri_dcm(void);

extern void mt_dcm_init(void);

#endif
