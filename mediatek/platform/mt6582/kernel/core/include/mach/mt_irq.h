#ifndef __MT_IRQ_H
#define __MT_IRQ_H

#define GIC_PRIVATE_SIGNALS     (32)
#define NR_GIC_SGI              (16)
#define NR_GIC_PPI              (16)
#define GIC_PPI_OFFSET          (27)
#define MT_NR_PPI               (5)
#define MT_NR_SPI               (188)
#define NR_MT_IRQ_LINE          (GIC_PPI_OFFSET + MT_NR_PPI + MT_NR_SPI)

#define GIC_PPI_GLOBAL_TIMER    (GIC_PPI_OFFSET + 0)
#define GIC_PPI_LEGACY_FIQ      (GIC_PPI_OFFSET + 1)
#define GIC_PPI_PRIVATE_TIMER   (GIC_PPI_OFFSET + 2)
#define GIC_PPI_NS_PRIVATE_TIMER            (GIC_PPI_OFFSET + 3)
#define GIC_PPI_LEGACY_IRQ      (GIC_PPI_OFFSET + 4)

#define MT_BTIF_IRQ_ID                  (GIC_PRIVATE_SIGNALS + 50)
#define MT_DMA_BTIF_TX_IRQ_ID              (GIC_PRIVATE_SIGNALS + 71)
#define MT_DMA_BTIF_RX_IRQ_ID              (GIC_PRIVATE_SIGNALS + 72)

#if !defined(CONFIG_MT6582_FPGA)

#if !defined(__ASSEMBLY__)
#define X_DEFINE_IRQ(__name, __num, __pol, __sens)  __name = __num,
enum 
{
#include "x_define_irq.h"
};
#undef X_DEFINE_IRQ
#define MT6582_AHB_SLAVE_HIF_IRQ_ID         WF_HIF_IRQ_ID /* FIXME */

#endif

#else

#define MT6582_USB0_IRQ_ID                  (GIC_PRIVATE_SIGNALS + 32)
#define MT_PTP_THERM_IRQ_ID                  (GIC_PRIVATE_SIGNALS + 38)
#define MT_MSDC0_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 39)
#define MT_MSDC1_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 40)
//#define MT_MSDC2_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 41) //6582 take off
//#define MT_MSDC3_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 42) //6582 take off
#define MT6582_AP_HIF_IRQ_ID                (GIC_PRIVATE_SIGNALS + 43)
#define MT_I2C0_IRQ_ID                  (GIC_PRIVATE_SIGNALS + 44)
#define MT_I2C1_IRQ_ID                  (GIC_PRIVATE_SIGNALS + 45)
#define MT_I2C2_IRQ_ID                  (GIC_PRIVATE_SIGNALS + 46)
#define MT_UART1_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 51)
#define MT_UART2_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 52)
#define MT_UART3_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 53)
#define MT_UART4_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 54)
#define MT_GDMA1_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 57)
#define MT_GDMA2_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 58)
#define MT_DMA_UART0_TX_IRQ_ID              (GIC_PRIVATE_SIGNALS + 63)
#define MT_DMA_UART0_RX_IRQ_ID              (GIC_PRIVATE_SIGNALS + 64)
#define MT_DMA_UART1_TX_IRQ_ID              (GIC_PRIVATE_SIGNALS + 65)
#define MT_DMA_UART1_RX_IRQ_ID              (GIC_PRIVATE_SIGNALS + 66)
#define MT_DMA_UART2_TX_IRQ_ID              (GIC_PRIVATE_SIGNALS + 67)
#define MT_DMA_UART2_RX_IRQ_ID              (GIC_PRIVATE_SIGNALS + 68)
#define MT6582_SPI1_IRQ_ID                  (GIC_PRIVATE_SIGNALS + 78)
//#define MT_MSDC4_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 83) //6582 take off
#define MT_PTP_FSM_IRQ_ID                   (GIC_PRIVATE_SIGNALS + 85)
#define MT_WDT_IRQ_ID                       (GIC_PRIVATE_SIGNALS + 88)//TBD:For build pass
#define MT_APARM_DOMAIN_IRQ_ID              (GIC_PRIVATE_SIGNALS + 94)
#define MT_APARM_DECERR_IRQ_ID              (GIC_PRIVATE_SIGNALS + 95)
#if 1 //cliff
#define MT6582_GPT_IRQ_ID                   (GIC_PRIVATE_SIGNALS + 112)//10.2 update
#define MT_EINT_IRQ_ID                      (GIC_PRIVATE_SIGNALS + 113)//10.2 update
#else
#define MT6582_GPT_IRQ_ID                   (GIC_PRIVATE_SIGNALS + 113)//10.2 update
#define MT_EINT_IRQ_ID                      (GIC_PRIVATE_SIGNALS + 116)//10.2 update
#endif
#define MT6582_PMIC_WRAP_IRQ_ID             (GIC_PRIVATE_SIGNALS + 115)//0x80
#define MT_KP_IRQ_ID			(GIC_PRIVATE_SIGNALS + 116)
#define MT_SPM_IRQ_ID                       (GIC_PRIVATE_SIGNALS + 117)
#define MT_VENC_IRQ_ID                      (GIC_PRIVATE_SIGNALS + 139)
#define MT_VDEC_IRQ_ID                      (GIC_PRIVATE_SIGNALS + 140)
#define CAMERA_ISP_IRQ0_ID                  (GIC_PRIVATE_SIGNALS + 143) // cam_irq_b
#define CAMERA_ISP_IRQ1_ID                  (GIC_PRIVATE_SIGNALS + 144) // cam_irq1_b
#define CAMERA_ISP_IRQ2_ID                  (GIC_PRIVATE_SIGNALS + 145) // cam_irq2_b
//#define CAMERA_ISP_IRQ3_ID                  (GIC_PRIVATE_SIGNALS + 144) // cam_irq3_b 6582 take off
#define MT6582_JPEG_ENC_IRQ_ID              (GIC_PRIVATE_SIGNALS + 141)
//#define MT6582_JPEG_DEC_IRQ_ID              (GIC_PRIVATE_SIGNALS + 148) //6582 take off
/* Not sure and comments for early porting */
#define MT_EINT_DIRECT0_IRQ_ID              (GIC_PRIVATE_SIGNALS + 121)

#define MT_MFG_IRQ_GP_ID                     (GIC_PRIVATE_SIGNALS + 170)
#define MT_MFG_IRQ_GPMMU_ID                  (GIC_PRIVATE_SIGNALS + 171)
#define MT_MFG_IRQ_PP0_ID                    (GIC_PRIVATE_SIGNALS + 172)
#define MT_MFG_IRQ_PPMMU0_ID                 (GIC_PRIVATE_SIGNALS + 173)
#define MT_MFG_IRQ_PP1_ID                    (GIC_PRIVATE_SIGNALS + 174)
#define MT_MFG_IRQ_PPMMU1_ID                 (GIC_PRIVATE_SIGNALS + 175)




#if 0
#define MT6582_DISP_MUTEX_IRQ_ID            (GIC_PRIVATE_SIGNALS + 160)
#define MT6582_DISP_ROT_IRQ_ID              (GIC_PRIVATE_SIGNALS + 161)
#define MT6582_DISP_SCL_IRQ_ID              (GIC_PRIVATE_SIGNALS + 162)
#define MT6582_DISP_OVL_IRQ_ID              (GIC_PRIVATE_SIGNALS + 163)
#define MT6582_DISP_WDMA0_IRQ_ID            (GIC_PRIVATE_SIGNALS + 164)
#define MT6582_DISP_WDMA1_IRQ_ID            (GIC_PRIVATE_SIGNALS + 165)
#define MT6582_DISP_RDMA0_IRQ_ID            (GIC_PRIVATE_SIGNALS + 166)
#define MT6582_DISP_RDMA1_IRQ_ID            (GIC_PRIVATE_SIGNALS + 167)
#define MT6582_DISP_BLS_IRQ_ID              (GIC_PRIVATE_SIGNALS + 168)
#define MT6582_DISP_COLOR_IRQ_ID            (GIC_PRIVATE_SIGNALS + 169)
#define MT6582_DISP_TDSHP_IRQ_ID            (GIC_PRIVATE_SIGNALS + 170)
#define MT6582_DISP_DBI_IRQ_ID              (GIC_PRIVATE_SIGNALS + 171)
#define MT6582_DISP_DSI_IRQ_ID              (GIC_PRIVATE_SIGNALS + 172)
#define MT6582_DISP_DPI0_IRQ_ID             (GIC_PRIVATE_SIGNALS + 173)
#define MT6582_DISP_DPI1_IRQ_ID             (GIC_PRIVATE_SIGNALS + 174)
#define MT6582_DISP_CMDQ_IRQ_ID             (GIC_PRIVATE_SIGNALS + 176)
#else
#define MT6582_DISP_MDP_RDMA_IRQ_ID       (GIC_PRIVATE_SIGNALS+146)  
#define MT6582_DISP_MDP_RSZ0_IRQ_ID       (GIC_PRIVATE_SIGNALS+147)  
#define MT6582_DISP_MDP_RSZ1_IRQ_ID       (GIC_PRIVATE_SIGNALS+148)  
#define MT6582_DISP_MDP_TDSHP_IRQ_ID      (GIC_PRIVATE_SIGNALS+149)  
#define MT6582_DISP_MDP_WDMA_IRQ_ID       (GIC_PRIVATE_SIGNALS+150)  
#define MT6582_DISP_MDP_WROT_IRQ_ID       (GIC_PRIVATE_SIGNALS+151)  
#define MT6582_DISP_RDMA_IRQ_ID           (GIC_PRIVATE_SIGNALS+152)  
#define MT6582_DISP_OVL_IRQ_ID            (GIC_PRIVATE_SIGNALS+153)  
#define MT6582_DISP_WDMA_IRQ_ID           (GIC_PRIVATE_SIGNALS+154)  
#define MT6582_DISP_BLS_IRQ_ID            (GIC_PRIVATE_SIGNALS+155)  
#define MT6582_DISP_COLOR_IRQ_ID          (GIC_PRIVATE_SIGNALS+156)  
#define MT6582_DISP_DSI_IRQ_ID            (GIC_PRIVATE_SIGNALS+157)  
#define MT6582_DISP_DPI0_IRQ_ID           (GIC_PRIVATE_SIGNALS+158)  
#define MT6582_DISP_CMDQ_IRQ_ID           (GIC_PRIVATE_SIGNALS+159)  
#define MT6582_DISP_CMDQ_SECURE_IRQ_ID    (GIC_PRIVATE_SIGNALS+160)  
#define MT6582_DISP_MUTEX_IRQ_ID          (GIC_PRIVATE_SIGNALS+161)  
#define MT6582_DISP_SMI_LARB0_IRQ_ID      (GIC_PRIVATE_SIGNALS+162)
#define MT_CIRQ_IRQ_ID                      (GIC_PRIVATE_SIGNALS+187)
#endif
#define MT6582_APARM_GPTTIMER_IRQ_LINE      MT6582_GPT_IRQ_ID

// MT6582 Wifi AHB Slave HIF
#define MT6582_AHB_SLAVE_HIF_IRQ_ID         (GIC_PRIVATE_SIGNALS + 160)
#define MT6582_HIF_PDMA_IRQ_ID              (GIC_PRIVATE_SIGNALS + 59)

/* These are defined for solving compile errors only. They are not existing on FPGA */
#define TS_IRQ_ID                         (GIC_PRIVATE_SIGNALS + 163)
#define CONN_WDT_IRQ_ID                   (GIC_PRIVATE_SIGNALS + 163)
#define LOWBATTERY_IRQ_ID                 (GIC_PRIVATE_SIGNALS + 163)
#define MD_WDT_IRQ_ID                     (GIC_PRIVATE_SIGNALS + 163)
#define CCIF0_AP_IRQ_ID                   (GIC_PRIVATE_SIGNALS + 100)
#endif

#endif
