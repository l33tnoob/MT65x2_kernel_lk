#ifndef __MT6572_H__
#define __MT6572_H__

/* disp subsys register */
#define MMSYS_CONFIG_BASE            0x14000000	//MT6572
#define MDP_RDMA_BASE                0x14001000	//MT6572
#define MDP_RSZ0_BASE                0x14002000	//MT6572
#define MDP_RSZ1_BASE                0x14003000	//MT6572
#define MDP_WDMA_BASE                0x14004000	//MT6572
#define MDP_WROT_BASE                0x14005000	//MT6572
#define MDP_TDSHP_BASE               0x14006000	//MT6572
#define DISP_OVL_BASE                0x14007000	//MT6572
#define DISP_RDMA_BASE               0x14008000	//MT6572
#define DISP_WDMA_BASE               0x14009000	//MT6572
#define DISP_BLS_BASE                0x1400A000	//MT6572
#define DISP_COLOR_BASE              0x1400B000	//MT6572
#define DSI_BASE                     0x1400C000	//MT6572
#define DISP_DPI_BASE                0x1400D000	//MT6572
#define MM_MUTEX_BASE                0x1400E000	//MT6572
#define MM_CMDQ_BASE                 0x1400F000	//MT6572
#define SMI_LARB0_BASE               0x14010000	//MT6572
#define SMI_COMMON_BASE              0x14011000	//MT6572
#define DISP_DBI_BASE                0x14012000	//MT6572
#define CAM_BASE                     0x14013000	//MT6572
#define SENINF_BASE                  0x14014000	//MT6572
#define MIPI_RX_CONFIG_BASE          0x14015000	//MT6572
#define VENC_BASE                    0x14016000	//MT6572
#define VDEC_BASE                    0x14017000	//MT6572

/* G3D */
#define G3D_CONFIG_BASE              0x13000000 //MT6572
#define MALI_BASE                    0x13010000 //MT6572

/* perisys */
#define AP_DMA_BASE                  0x11000000	//MT6572
#define NFI_BASE                     0x11001000	//MT6572
#define NFIECC_BASE                  0x11002000	//MT6572
#define AUXADC_BASE                  0x11003000	//MT6572
#define FHCTL_BASE                   0x11004000	//MT6572
#define UART1_BASE                   0x11005000	//MT6572
#define UART2_BASE                   0x11006000	//MT6572
#define PWM_BASE                     0x11008000	//MT6572
#define I2C0_BASE                    0x11009000	//MT6572
#define I2C1_BASE                    0x1100A000	//MT6572
#define SPI_BASE                     0x1100C000	//MT6572
#define THERMAL_BASE                 0x1100D000	//MT6572
#define BTIF_BASE                    0x1100E000	//MT6572
#define USB_BASE                     0x11100000	//MT6572
#define USB_SIF_BASE                 0x11110000	//MT6572
#define MSDC_0_BASE                  0x11120000	//MT6572
#define MSDC_1_BASE                  0x11130000	//MT6572
#define AUDIO_BASE                   0x11140000	//MT6572
#define AHB_MONITOR_BASE             0x11150000	//MT6572

/* infrasys AO */
#define TOPCKGEN_BASE                0x10000000	//MT6572
#define INFRA_SYS_CFG_AO_BASE        0x10001000	//MT6572
#define SRAMROM_BASE                 0x10001400	//MT6572 SYSMEM Controller
#define KP_BASE                      0x10002000	//MT6572
#define PERICFG_BASE                 0x10003000	//MT6572
#define EMI_BASE                     0x10004000	//MT6572
#define GPIO_BASE                    0x10005000	//MT6572
#define SPM_BASE                     0x10006000	//MT6572
#define AP_RGU_BASE                  0x10007000	//MT6572
#define TOPRGU_BASE                 AP_RGU_BASE //MT6572
#define APMCU_GPTIMER_BASE           0x10008000	//MT6572
#define EFUSE_CTR_BASE               0x10009000	//MT6572
#define SEJ_BASE                     0x1000A000	//MT6572
#define EINT_BASE                    0x1000B000	//MT6572
#define AP_CCIF_BASE                 0x1000C000	//MT6572
#define SMI_BASE                     0x1000E000	//MT6572
#define PMIC_WRAP_BASE               0x1000F000	//MT6572
#define DEVICE_APC_AO_BASE           0x10010000	//MT6572
#define MIPI_CONFIG_BASE             0x10011000	//MT6572
#define INFRA_TOP_MBIST_CTRL_BASE    0x10012000	//MT6572

/* infrasys */
#define MCUSYS_CFGREG_BASE           0x10200000	//MT6572
//#define INFRA_SYS_CFG_BASE           0x10201000	//MT6572
#define SYS_CIRQ_BASE                0x10202000	//MT6572
#define M4U_CFG_BASE                 0x10203000	//MT6572
#define DEVICE_APC_BASE              0x10204000	//MT6572
#define APMIXED_BASE                 0x10205000	//MT6572
#define IO_CFG_TOP_BASE              0x10208000	//MT6572
#define IO_CFG_BOTTOM_BASE           0x10209000	//MT6572
#define IO_CFG_LEFT_BASE             0x1020A000	//MT6572
#define IO_CFG_RIGHT_BASE            0x1020B000	//MT6572
#define CORTEXA7MP_BASE              0x10210000	//MT6572

/* Ram Console */
#define RAM_CONSOLE_BASE             0x01000000 //MT6572

/* Device Info */
#define DEVINFO_BASE                 0x08000000 //MT6572

#ifndef RTC_BASE
#define RTC_BASE 		     0x8000 //MT6572
#endif

//======================================================
//         following are OLD, to be removed
//======================================================

#define CONFIG_BASE                  INFRA_SYS_CFG_AO_BASE
#define TOP_RGU_BASE                 AP_RGU_BASE
#define PERI_CON_BASE                PERICFG_BASE
#define GIC_BASE                     CORTEXA7MP_BASE
#define GIC_CPU_BASE                 (GIC_BASE + 0x2000)
#define GIC_DIST_BASE                (GIC_BASE + 0x1000)
#define MSDC0_BASE                   MSDC_0_BASE
#define MSDC1_BASE                   MSDC_1_BASE
#define USB0_BASE                    USB_BASE
#define USBSIF_BASE                  USB_SIF_BASE
#define APMIXEDSYS_BASE              APMIXED_BASE
#define INFRA_SYS_CFG_BASE	          INFRA_SYS_CFG_AO_BASE
#define SLEEP_BASE                   SPM_BASE
#define PERI_PWRAP_BRIDGE_BASE       PMIC_WRAP_BASE

/* disp subsys register */
#define DISPSYS_BASE                 MMSYS_CONFIG_BASE
#define RDMA0_BASE                   DISP_RDMA_BASE
#define WDMA0_BASE                   DISP_WDMA_BASE
#define ROT_BASE                     MDP_WROT_BASE
#define TDSHP_BASE                   MDP_TDSHP_BASE
#define OVL_BASE                     DISP_OVL_BASE
#define BLS_BASE                     DISP_BLS_BASE
#define COLOR_BASE                   DISP_COLOR_BASE
#define DPI_BASE                     DISP_DPI_BASE
#define LCD_BASE                   	 DISP_DBI_BASE
#define SCL_BASE                     MDP_RSZ0_BASE    //[TODO] need confirm
#define DISP_MUTEX_BASE              MM_MUTEX_BASE
#define DISP_CMDQ_BASE               MM_CMDQ_BASE    //[TODO] sure to be removed
#define DISP_GPIO_BASE               GPIO_BASE
#define DISP_GPIO_CFG_LEFT_BASE      IO_CFG_LEFT_BASE
#define DISP_GPIO_CFG_RIGHT_BASE     IO_CFG_RIGHT_BASE


/**************************************************
 *                  For PDN                       *
 **************************************************/
#define ClK_GATING_CTRL0     (TOPCKGEN_BASE+0x20)
#define CLK_GATING_CTRL1     (TOPCKGEN_BASE+0x24)

#define APGPT_PDN_ADDR       CLK_GATING_CTRL1
#define APGPT_PDN_MASK       (1<<24)
#define APUART0_PDN_ADDR     CLK_GATING_CTRL1
#define APUART0_PDN_MASK     (1<<10)
#define APUART1_PDN_ADDR     CLK_GATING_CTRL1
#define APUART1_PDN_MASK     (1<<11)


/**************************************************
 *                   mcusys_cfgreg                *
 **************************************************/
#define INT_POL_CTL0                 (MCUSYS_CFGREG_BASE + 0x100)


/**************************************************
 *                   Chip ID/HW, SW Version       *
 **************************************************/

#define APHW_VER                     (DEVINFO_BASE)
#define APSW_VER                     (DEVINFO_BASE + 0x04)
#define APHW_CODE                    (DEVINFO_BASE + 0x08)
#define APHW_SUBCODE                 (DEVINFO_BASE + 0x0C)

/**************************************************
 *                   eFuse                        *
 **************************************************/
#define SERIAL_KEY_HI	         	 (EFUSE_CTR_BASE + 0x0144)
#define SERIAL_KEY_LO	         	 (EFUSE_CTR_BASE + 0x0140)


/**************************************************
 *                   SW difine                    *
 **************************************************/

/* MT storage boot type definitions */
#define NON_BOOTABLE                0
#define RAW_BOOT                    1
#define FAT_BOOT                    2

#define CONFIG_STACKSIZE	    (128*1024)	  /* regular stack */

// xuecheng, define this because we use zlib for boot logo compression
#define CONFIG_ZLIB 	1

/**************************************************
 *         Memory Preserved Mode pc, fp, sp       *
 **************************************************/
#define DBG_CORE0_PC        (MCUSYS_CFGREG_BASE + 0x0300)
#define DBG_CORE0_FP        (MCUSYS_CFGREG_BASE + 0x0304)
#define DBG_CORE0_SP        (MCUSYS_CFGREG_BASE + 0x0308)
#define DBG_CORE1_PC        (MCUSYS_CFGREG_BASE + 0x0310)
#define DBG_CORE1_FP        (MCUSYS_CFGREG_BASE + 0x0314)
#define DBG_CORE1_SP        (MCUSYS_CFGREG_BASE + 0x0318)

#define AHBABT_ADDR1        (AHB_MONITOR_BASE   + 0x0020)
#define AHBABT_ADDR2        (AHB_MONITOR_BASE   + 0x0024)
#define AHBABT_ADDR3        (AHB_MONITOR_BASE   + 0x0028)
#define AHBABT_ADDR4        (AHB_MONITOR_BASE   + 0x002C)
#define AHBABT_RDY_CNT1     (AHB_MONITOR_BASE   + 0x0030)
#define AHBABT_RDY_CNT2     (AHB_MONITOR_BASE   + 0x0034)
#define AHBABT_RDY_CNT3     (AHB_MONITOR_BASE   + 0x0038)
#define AHBABT_RDY_CNT4     (AHB_MONITOR_BASE   + 0x003C)

#define CARD_DETECT_PIN     (86)
// =======================================================================
// UBOOT DEBUG CONTROL
// =======================================================================
//#define UBOOT_DEBUG_TRACER			(0)

/*MTK Memory layout configuration*/
#define MAX_NR_BANK    4

#define DRAM_PHY_ADDR   0x80000000

#if 0
#if defined (MODEM_3G)
 #define RIL_SIZE		0x1600000
#elif defined (MODEM_2G)
 #define RIL_SIZE		0x0A00000
#else
 #define RIL_SIZE		0x1600000
#endif
#endif
#define RIL_SIZE	0x100000 //for connsys memory

#define MEM_PRELOADER_START             (DRAM_PHY_ADDR) //placed mem in RIL 256KB
#define MEM_PRELOADER_SIZE              (0x20000)

#define MEM_SRAM_PRELOADER_START         (0x01002000)
#define MEM_SRAM_PRELOADER_SIZE         (0x1000)
// only build in Eng build
#if defined(MTK_MEM_PRESERVED_MODE_ENABLE) && !defined(USER_BUILD)
//#define MEM_PRESERVED_MODE_ENABLE
//#define MEM_PRESERVED_MODE_VIDEO_PRINT
#endif

#define RESERVE_MEM_SIZE                (RIL_SIZE)

#define CFG_RAMDISK_LOAD_ADDR           (DRAM_PHY_ADDR + RESERVE_MEM_SIZE + 0x4000000)
#define CFG_BOOTIMG_LOAD_ADDR           (DRAM_PHY_ADDR + RESERVE_MEM_SIZE + 0x8000)
#define CFG_BOOTARGS_ADDR               (DRAM_PHY_ADDR + RESERVE_MEM_SIZE + 0x100)

/*Command passing to Kernel */
#ifdef MACH_FPGA
#define COMMANDLINE_TO_KERNEL  "console=tty0 console=ttyMT1,921600n1 root=/dev/ram"
#else
#define COMMANDLINE_TO_KERNEL  "console=tty0 console=ttyMT1,921600n1 root=/dev/ram"
#endif
#define CFG_FACTORY_NAME	"factory.img"
#define HAVE_LK_TEXT_MENU

//ALPS00427972, implement the analog register formula
//Add here for eFuse, chip version checking -> analog register calibration
#define M_HW_RES3	                    0x10009170
//#define M_HW_RES3_PHY                   IO_PHYS+M_HW_RES3
#define RG_USB20_TERM_VREF_SEL_MASK     0xE000      //0b 1110,0000,0000,0000     15~13
#define RG_USB20_CLKREF_REF_MASK        0x1C00      //0b 0001,1100,0000,0000     12~10
#define RG_USB20_VRT_VREF_SEL_MASK      0x0380      //0b 0000,0011,1000,0000     9~7
//ALPS00427972, implement the analog register formula

#endif

