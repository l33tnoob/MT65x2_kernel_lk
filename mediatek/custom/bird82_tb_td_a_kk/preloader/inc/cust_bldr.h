#ifndef CUST_BLDR_H
#define CUST_BLDR_H

#include "boot_device.h"
#include "uart.h"

/* cust_bldr.h is not used any more, please refer to cust_bldr.mak */

#if 0
/*=======================================================================*/
/* Pre-Loader Features                                                   */
/*=======================================================================*/
#define CFG_FPGA_PLATFORM           (1)

#if CFG_FPGA_PLATFORM
#define CFG_BOOT_DEV                (BOOTDEV_SDMMC)
#else
#ifdef MTK_EMMC_SUPPORT
#define CFG_BOOT_DEV                (BOOTDEV_SDMMC)
#else
#define CFG_BOOT_DEV                (BOOTDEV_NAND)
#endif
#endif

#define CFG_BATTERY_DETECT          (0)

#define CFG_UART_TOOL_HANDSHAKE     (1)
#define CFG_USB_TOOL_HANDSHAKE      (1)
#define CFG_USB_DOWNLOAD            (1)
#define CFG_PMT_SUPPORT             (0)

#define CFG_LOG_BAUDRATE            (921600)
#define CFG_META_BAUDRATE           (115200)
#define CFG_UART_LOG                (UART1)
#define CFG_UART_META               (UART1)

#define CFG_EMERGENCY_DL_SUPPORT    (0)
#define CFG_EMERGENCY_DL_TIMEOUT_MS (1000 * 60 * 5) /* 5 mins */

#define CFG_SWITCH_USB_UART_SUPPORT (0)
#define CFG_SWITCH_FORCE_USB        (0) // only active when CFG_SWITCH_USB_UART_SUPPORT is (1)
#define CFG_SWITCH_FORCE_UART       (0) // only active when CFG_SWITCH_USB_UART_SUPPORT is (1)

/*=======================================================================*/
/* Misc Options                                                          */	
/*=======================================================================*/
#define FEATURE_MMC_ADDR_TRANS
#endif

#endif /* CUST_BLDR_H */
