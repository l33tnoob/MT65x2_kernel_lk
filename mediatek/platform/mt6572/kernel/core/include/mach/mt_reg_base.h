#ifndef __MT_REG_BASE__
#define __MT_REG_BASE__
/* MM sub-system */
#define MMSYS_CONFIG_BASE            0xF4000000	//MT6572
#define MDP_RDMA_BASE                0xF4001000	//MT6572
#define MDP_RSZ0_BASE                0xF4002000	//MT6572
#define MDP_RSZ1_BASE                0xF4003000	//MT6572
#define MDP_WDMA_BASE                0xF4004000	//MT6572
#define MDP_WROT_BASE                0xF4005000	//MT6572
#define MDP_TDSHP_BASE               0xF4006000	//MT6572
#define DISP_OVL_BASE                0xF4007000	//MT6572
#define DISP_RDMA_BASE               0xF4008000	//MT6572
#define DISP_WDMA_BASE               0xF4009000	//MT6572
#define DISP_BLS_BASE                0xF400A000	//MT6572
#define DISP_COLOR_BASE              0xF400B000	//MT6572
#define DSI_BASE                     0xF400C000	//MT6572
#define DISP_DPI_BASE                0xF400D000	//MT6572
#define MM_MUTEX_BASE                0xF400E000	//MT6572
#define MM_CMDQ_BASE                 0xF400F000	//MT6572
#define SMI_LARB0_BASE               0xF4010000	//MT6572
#define SMI_COMMON_BASE              0xF4011000	//MT6572
#define DISP_DBI_BASE                0xF4012000 //MT6572
#define CAM_BASE                     0xF4013000	//MT6572
#define SENINF_BASE                  0xF4014000	//MT6572
#define MIPI_RX_CONFIG_BASE          0xF4015000	//MT6572
#define VENC_BASE                    0xF4016000	//MT6572
#define VDEC_BASE                    0xF4017000	//MT6572

/* G3D */
#define G3D_CONFIG_BASE              0xF3000000 //MT6572
#define MALI_BASE                    0xF3010000 //MT6572

/* perisys */
#define AP_DMA_BASE                  0xF1000000	//MT6572
#define NFI_BASE                     0xF1001000	//MT6572
#define NFIECC_BASE                  0xF1002000	//MT6572
#define AUXADC_BASE                  0xF1003000	//MT6572
#define FHCTL_BASE                   0xF1004000	//MT6572
#define UART1_BASE                   0xF1005000	//MT6572
#define UART2_BASE                   0xF1006000	//MT6572
#define PWM_BASE                     0xF1008000	//MT6572
#define I2C0_BASE                    0xF1009000	//MT6572
#define I2C1_BASE                    0xF100A000	//MT6572
#define SPI_BASE                     0xF100C000	//MT6572
#define THERMAL_BASE                 0xF100D000	//MT6572
#define BTIF_BASE                    0xF100E000	//MT6572
#define USB_BASE                     0xF1100000	//MT6572
#define USB_SIF_BASE                 0xF1110000	//MT6572
#define MSDC_0_BASE                  0xF1120000	//MT6572
#define MSDC_1_BASE                  0xF1130000	//MT6572
#ifdef CONFIG_MT6572_FPGA_CA7
#define AUDIO_BASE                   0xF6000000 //MT6572 FPGA
#else //CONFIG_MT6572_FPGA_CA7
#define AUDIO_BASE                   0xF1140000	//MT6572
#endif //CONFIG_MT6572_FPGA_CA7

/* infrasys AO */
#define TOPCKGEN_BASE                0xF0000000	//MT6572
#define INFRA_SYS_CFG_AO_BASE        0xF0001000	//MT6572
#define SRAMROM_BASE                 0xF0001400	//MT6572 SYSMEM Controller
#define KP_BASE                      0xF0002000	//MT6572
#define PERICFG_BASE                 0xF0003000	//MT6572
#define EMI_BASE                     0xF0004000	//MT6572
#define GPIO_BASE                    0xF0005000	//MT6572
#define SPM_BASE                     0xF0006000	//MT6572
#define AP_RGU_BASE                  0xF0007000	//MT6572
#define TOPRGU_BASE                 AP_RGU_BASE //MT6572
#define APMCU_GPTIMER_BASE           0xF0008000	//MT6572
#define HACC_BASE                    0xF000A000 //MT6572
#define EINT_BASE                    0xF000B000	//MT6572
#define AP_CCIF_BASE                 0xF000C000	//MT6572
#define SMI_BASE                     0xF000E000	//MT6572
#define PMIC_WRAP_BASE               0xF000F000	//MT6572
#define DEVICE_APC_AO_BASE           0xF0010000	//MT6572
#define MIPI_CONFIG_BASE             0xF0011000	//MT6572
#define INFRA_TOP_MBIST_CTRL_BASE    0xF0012000	//MT6572

/* infrasys */
#define APARM_BASE                   0xF0170000 //MT6572
#define MCUSYS_CFGREG_BASE           0xF0200000	//MT6572
#define INFRA_SYS_CFG_BASE           0xF0201000	//MT6572
#define SYS_CIRQ_BASE                0xF0202000	//MT6572
#define M4U_CFG_BASE                 0xF0203000	//MT6572
#define DEVICE_APC_BASE              0xF0204000	//MT6572
#define APMIXED_BASE                 0xF0205000	//MT6572
#define IO_CFG_TOP_BASE              0xF0208000	//MT6572
#define IO_CFG_BOTTOM_BASE           0xF0209000	//MT6572
#define IO_CFG_LEFT_BASE             0xF020A000	//MT6572
#define IO_CFG_RIGHT_BASE            0xF020B000	//MT6572
#define GIC_BASE                     0xF0210000	//MT6572

/* CONNSYS */
#define CONN_BT_PKV_BASE             0xF8000000 //MT6572
#define CONN_BT_TIMCON_BASE          0xF8010000 //MT6572
#define CONN_BT_RF_CONTROL_BASE      0xF8020000 //MT6572
#define CONN_BT_MODEM_BASE           0xF8030000 //MT6572
#define CONN_BT_CONFIG_BASE          0xF8040000 //MT6572
#define CONN_MCU_CONFIG_BASE         0xF8070000 //MT6572
#define CONN_SYSRAM_BANK2_BASE       0xF8080000 //MT6572
#define CONN_SYSRAM_BANK3_BASE       0xF8090000 //MT6572
#define CONN_SYSRAM_BANK4_BASE       0xF80A0000 //MT6572
#define CONN_TOP_CR_BASE             0xF80B0000 //MT6572
#define CONN_HIF_BASE                0xF80F0000 //MT6572

/* Ram Console */
#define RAM_CONSOLE_BASE             0xF2000000 //MT6572

/* Device Info */
#define DEVINFO_BASE                 0xF5000000 //MT6572


#endif
