#ifndef CUST_BLDR_H
#define CUST_BLDR_H

#include "boot_device.h"
#include "uart.h"

/*=======================================================================*/
/* Pre-Loader Features                                                   */
/*=======================================================================*/
#ifdef MTK_EMMC_SUPPORT
#define CFG_BOOT_DEV                (BOOTDEV_SDMMC)
#else
#define CFG_BOOT_DEV                (BOOTDEV_NAND)
#endif
#define CFG_FPGA_PLATFORM           (0)
//#define CONFIG_EARLY_LINUX_PORTING  (1)
#define CFG_BATTERY_DETECT          (1)

#define CFG_UART_TOOL_HANDSHAKE     (1)
#define CFG_USB_TOOL_HANDSHAKE      (1)
#define CFG_USB_DOWNLOAD            (1)
#define CFG_PMT_SUPPORT             (1)

#define CFG_LOG_BAUDRATE            (921600)
#define CFG_META_BAUDRATE           (115200)
#define CFG_UART_LOG                (UART2)
#define CFG_UART_META               (UART1)
#define CFG_USB_UART_SWITCH_PORT    (UART1)

#define CFG_EMERGENCY_DL_SUPPORT    (1)
#define CFG_EMERGENCY_DL_TIMEOUT_MS (1000 * 5) /* 5 s */
//#define CFG_USB_AUTO_DETECT    (1)
#define CFG_USB_AUTO_DETECT_TIMEOUT_MS (1000 * 3) /* 3 s */
/*=======================================================================*/
/* Misc Options                                                          */
/*=======================================================================*/
#define FEATURE_MMC_ADDR_TRANS
//#define CFG_EVB_PLATFORM

#endif /* CUST_BLDR_H */
