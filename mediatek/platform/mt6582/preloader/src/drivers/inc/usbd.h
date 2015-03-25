/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __USBD_mt6573_H__
#define __USBD_mt6573_H__

#include <usbdcore.h>

/* ============= */
/* hardware spec */
/* ============= */
//#define CONFIG_USBD_MANUFACTURER "MediaTek"
//#define CONFIG_USBD_PRODUCT_NAME "mt6573"
//#define CONFIG_USBD_VENDORID  0x18D1
//#define CONFIG_USBD_PRODUCTID 0xDEED

#define MT_EP_NUM 4
#define MT_CHAN_NUM 4
#define MT_EP0_FIFOSIZE 64

#define FIFO_ADDR_START  512

#define MT_BULK_MAXP 512
#define MT_INT_MAXP  1024

/* =================== */
/* USB common register */
/* =================== */

#define FADDR    (USB_BASE + 0x0000)    /* Function Address Register */
#define POWER    (USB_BASE + 0x0001)    /* Power Management Register */
#define INTRTX   (USB_BASE + 0x0002)    /* TX Interrupt Status Register */
#define INTRRX   (USB_BASE + 0x0004)    /* RX Interrupt Status Register */
#define INTRTXE  (USB_BASE + 0x0006)    /* TX Interrupt Status Enable Register */
#define INTRRXE  (USB_BASE + 0x0008)    /* RX Interrupt Status Enable Register */
#define INTRUSB  (USB_BASE + 0x000a)    /* Common USB Interrupt Register */
#define INTRUSBE (USB_BASE + 0x000b)    /* Common USB Interrupt Enable Register */
#define FRAME    (USB_BASE + 0x000c)    /* Frame Number Register */
#define INDEX    (USB_BASE + 0x000e)    /* Endpoint Selecting Index Register */
#define TESTMODE (USB_BASE + 0x000f)    /* Test Mode Enable Register */
#define DBG_PRB0 (USB_BASE + 0x0620)    /* USB Debug Probe Register 0 */
#define DBG_PRB4 (USB_BASE + 0x0630)    /* USB Debug Probe Register 4 */

/* ============ */
/* POWER fields */
/* ============ */

#define PWR_ISO_UPDATE       (1<<7)
#define PWR_SOFT_CONN        (1<<6)
#define PWR_HS_ENAB          (1<<5)
#define PWR_HS_MODE          (1<<4)
#define PWR_RESET            (1<<3)
#define PWR_RESUME           (1<<2)
#define PWR_SUSPEND_MODE     (1<<1)
#define PWR_ENABLE_SUSPENDM  (1<<0)

/* ============== */
/* INTRUSB fields */
/* ============== */

#define INTRUSB_VBUS_ERROR (1<<7)
#define INTRUSB_SESS_REQ   (1<<6)
#define INTRUSB_DISCON     (1<<5)
#define INTRUSB_CONN       (1<<4)
#define INTRUSB_SOF        (1<<3)
#define INTRUSB_RESET      (1<<2)
#define INTRUSB_RESUME     (1<<1)
#define INTRUSB_SUSPEND    (1<<0)

/* ===================== */
/* DMA control registers */
/* ===================== */

#define DMA_INTR (USB_BASE + 0x0200)

#define USB_DMA_CNTL(chan)  (USB_BASE + 0x0204 + 0x10*(chan-1))
#define USB_DMA_ADDR(chan)  (USB_BASE + 0x0208 + 0x10*(chan-1))
#define USB_DMA_COUNT(chan) (USB_BASE + 0x020c + 0x10*(chan-1))

/* ================================= */
/* Endpoint Control/Status Registers */
/* ================================= */

#define IECSR (USB_BASE + 0x0010)
/* for EP0 */
#define CSR0         0x2        /* EP0 Control Status Register */
                          /* For Host Mode, it would be 0x2 */
#define COUNT0       0x8        /* EP0 Received Bytes Register */
#define NAKLIMIT0    0xB        /* NAK Limit Register */
#define CONFIGDATA   0xF        /* Core Configuration Register */
/* for other endpoints */
#define TXMAP        0x0        /* TXMAP Register: Max Packet Size for TX */
#define TXCSR        0x2        /* TXCSR Register: TX Control Status Register */
#define RXMAP        0x4        /* RXMAP Register: Max Packet Size for RX */
#define RXCSR        0x6        /* RXCSR Register: RX Control Status Register */
#define RXCOUNT      0x8        /* RXCOUNT Register */
#define TXTYPE       0xa        /* TX Type Register */
#define TXINTERVAL   0xb        /* TX Interval Register */
#define RXTYPE       0xc        /* RX Type Register */
#define RXINTERVAL   0xd        /* RX Interval Register */
#define FIFOSIZE     0xf        /* configured FIFO size register */

/* ============================== */
/* control status register fields */
/* ============================== */

/* CSR0_DEV */
#define EP0_FLUSH_FIFO           (1<<8)
#define EP0_SERVICE_SETUP_END    (1<<7)
#define EP0_SERVICED_RXPKTRDY    (1<<6)
#define EP0_SENDSTALL            (1<<5)
#define EP0_SETUPEND             (1<<4)
#define EP0_DATAEND              (1<<3)
#define EP0_SENTSTALL            (1<<2)
#define EP0_TXPKTRDY             (1<<1)
#define EP0_RXPKTRDY             (1<<0)

/* TXCSR_DEV */
#define EPX_TX_AUTOSET           (1<<15)
#define EPX_TX_ISO               (1<<14)
#define EPX_TX_MODE              (1<<13)
#define EPX_TX_DMAREQEN          (1<<12)
#define EPX_TX_FRCDATATOG        (1<<11)
#define EPX_TX_DMAREQMODE        (1<<10)
#define EPX_TX_AUTOSETEN_SPKT    (1<<9)
#define EPX_TX_INCOMPTX          (1<<7)
#define EPX_TX_CLRDATATOG        (1<<6)
#define EPX_TX_SENTSTALL         (1<<5)
#define EPX_TX_SENDSTALL         (1<<4)
#define EPX_TX_FLUSHFIFO         (1<<3)
#define EPX_TX_UNDERRUN          (1<<2)
#define EPX_TX_FIFONOTEMPTY      (1<<1)
#define EPX_TX_TXPKTRDY          (1<<0)

/* RXCSR_DEV */
#define EPX_RX_AUTOCLEAR         (1<<15)
#define EPX_RX_ISO               (1<<14)
#define EPX_RX_DMAREQEN          (1<<13)
#define EPX_RX_DISNYET           (1<<12)
#define EPX_RX_PIDERR            (1<<12)
#define EPX_RX_DMAREQMODE        (1<<11)
#define EPX_RX_AUTOCLRENSPKT     (1<<10)
#define EPX_RX_INCOMPRXINTREN    (1<<9)
#define EPX_RX_INCOMPRX          (1<<8)
#define EPX_RX_CLRDATATOG        (1<<7)
#define EPX_RX_SENTSTALL         (1<<6)
#define EPX_RX_SENDSTALL         (1<<5)
#define EPX_RX_FLUSHFIFO         (1<<4)
#define EPX_RX_DATAERR           (1<<3)
#define EPX_RX_OVERRUN           (1<<2)
#define EPX_RX_FIFOFULL          (1<<1)
#define EPX_RX_RXPKTRDY          (1<<0)

/* ================= */
/* CONFIGDATA fields */
/* ================= */

#define MP_RXE         (1<<7)
#define MP_TXE         (1<<6)
#define BIGENDIAN      (1<<5)
#define HBRXE          (1<<4)
#define HBTXE          (1<<3)
#define DYNFIFOSIZING  (1<<2)
#define SOFTCONE       (1<<1)
#define UTMIDATAWIDTH  (1<<0)

/* ============= */
/* FIFO register */
/* ============= */

/* for endpint 1 ~ 4, writing to these addresses = writing to the */
/* corresponding TX FIFO, reading from these addresses = reading from */
/* corresponding RX FIFO */

#define FIFO(ep_num)     (USB_BASE + 0x0020 + ep_num*0x0004)

/* ============================ */
/* additional control registers */
/* ============================ */

#define DEVCTL       (USB_BASE + 0x0060)        /* OTG Device Control Register */
#define PWRUPCNT     (USB_BASE + 0x0061)        /* Power Up Counter Register */
#define TXFIFOSZ     (USB_BASE + 0x0062)        /* TX FIFO Size Register */
#define RXFIFOSZ     (USB_BASE + 0x0063)        /* RX FIFO Size Register */
#define TXFIFOADD    (USB_BASE + 0x0064)        /* TX FIFO Address Register */
#define RXFIFOADD    (USB_BASE + 0x0066)        /* RX FIFO Address Register */
#define HWVERS       (USB_BASE + 0x006c)        /* H/W Version Register */
#define SWRST        (USB_BASE + 0x0074)        /* Software Reset Register */
#define EPINFO       (USB_BASE + 0x0078)        /* TX and RX Information Register */
#define RAM_DMAINFO  (USB_BASE + 0x0079)        /* RAM and DMA Information Register */
#define LINKINFO     (USB_BASE + 0x007a)        /* Delay Time Information Register */
#define VPLEN        (USB_BASE + 0x007b)        /* VBUS Pulse Charge Time Register */
#define HSEOF1       (USB_BASE + 0x007c)        /* High Speed EOF1 Register */
#define FSEOF1       (USB_BASE + 0x007d)        /* Full Speed EOF1 Register */
#define LSEOF1       (USB_BASE + 0x007e)        /* Low Speed EOF1 Register */
#define RSTINFO      (USB_BASE + 0x007f)        /* Reset Information Register */

/* ========================================================== */
/* FIFO size register fields and available packet size values */
/* ========================================================== */
#define DPB        0x10
#define PKTSZ      0x0f

#define PKTSZ_8    (1<<3)
#define PKTSZ_16   (1<<4)
#define PKTSZ_32   (1<<5)
#define PKTSZ_64   (1<<6)
#define PKTSZ_128  (1<<7)
#define PKTSZ_256  (1<<8)
#define PKTSZ_512  (1<<9)
#define PKTSZ_1024 (1<<10)

#define FIFOSZ_8      (0x0)
#define FIFOSZ_16     (0x1)
#define FIFOSZ_32     (0x2)
#define FIFOSZ_64     (0x3)
#define FIFOSZ_128    (0x4)
#define FIFOSZ_256    (0x5)
#define FIFOSZ_512    (0x6)
#define FIFOSZ_1024   (0x7)
#define FIFOSZ_2048   (0x8)
#define FIFOSZ_4096   (0x9)
#define FIFOSZ_3072   (0xF)

/* ============ */
/* SWRST fields */
/* ============ */

#define SWRST_PHY_RST         (1<<7)
#define SWRST_PHYSIG_GATE_HS  (1<<6)
#define SWRST_PHYSIG_GATE_EN  (1<<5)
#define SWRST_REDUCE_DLY      (1<<4)
#define SWRST_UNDO_SRPFIX     (1<<3)
#define SWRST_FRC_VBUSVALID   (1<<2)
#define SWRST_SWRST           (1<<1)
#define SWRST_DISUSBRESET     (1<<0)

/* DMA_CNTL */
#define USB_DMA_CNTL_ENDMAMODE2            (1 << 13)
#define USB_DMA_CNTL_PP_RST                (1 << 12)
#define USB_DMA_CNTL_PP_EN                 (1 << 11)
#define USB_DMA_BURST_MODE_MASK            (3 << 9)
#define USB_DMA_BURST_MODE_0               (0 << 9)
#define USB_DMA_BURST_MODE_1               (0x1 << 9)
#define USB_DMA_BURST_MODE_2               (0x2 << 9)
#define USB_DMA_BURST_MODE_3               (0x3 << 9)
#define USB_DMA_BUS_ERROR                  (0x1 << 8)
#define USB_DMA_ENDPNT_MASK                (0xf << 4)
#define USB_DMA_ENDPNT_OFFSET              (4)
#define USB_DMA_INTEN                      (1 << 3)
#define USB_DMA_DMAMODE                    (1 << 2)
#define USB_DMA_DIR                        (1 << 1)
#define USB_DMA_EN                         (1 << 0)

/* ======= */
/* typedef */
/* ======= */

typedef enum
{
    EP0_IDLE = 0,
    EP0_RX,
    EP0_TX,
} EP0_STATE;

typedef enum
{
    CHARGER_UNKNOWN,
    STANDARD_HOST,
    STANDARD_CHARGER,
    NONSTANDARD_CHARGER,
} USB_CHARGER_TYPE;

struct usb_acm_line_coding
{
    u32 dwDTERate;
    u8 bCharFormat;
    u8 bParityType;
    u8 bDataBits;
} __attribute__ ((packed));

/* =========== */
/* some macros */
/* =========== */

#define EPMASK(x) (1<<x)
#define CHANMASK(x) (1<<x)

/* ========== */
/* structures */
/* ========== */

/* Higher level functions for abstracting away from specific device */

void mt_udc_irq (u8 intrtx, u8 intrrx, u8 intrusb);
void mt_ep_write (struct mt_ep *endpoint);
int mt_ep_busy (struct mt_ep *endpoint);
void mt_setup_ep (struct mt_dev *device,
                  unsigned int ep, struct mt_ep *endpoint);
void mt_usb_connect_internal (void);
void mt_usb_disconnect_internal (void);
void udc_enable (struct mt_dev *device);
void udc_disable (void);

#endif
