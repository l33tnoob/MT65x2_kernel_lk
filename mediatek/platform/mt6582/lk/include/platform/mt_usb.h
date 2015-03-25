/*
 * Copyright (c) 2012 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _MT6577_USB_H_
#define _MT6577_USB_H_

/* USB PHY registers */
#define USB20_PHY_BASE	(USBSIF_BASE + 0x0800)
#define USB11_PHY_BASE	(USBSIF_BASE + 0x0900)

/* hardware spec */
#define MT_EP_NUM 4
#define MT_CHAN_NUM 4
#define MT_EP0_FIFOSIZE 64

#define FIFO_ADDR_START  512

#define MT_BULK_MAXP 512
#define MT_INT_MAXP  1024

/* USB common registers */
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

/* POWER fields */
#define PWR_ISO_UPDATE       (1<<7)
#define PWR_SOFT_CONN        (1<<6)
#define PWR_HS_ENAB          (1<<5)
#define PWR_HS_MODE          (1<<4)
#define PWR_RESET            (1<<3)
#define PWR_RESUME           (1<<2)
#define PWR_SUSPEND_MODE     (1<<1)
#define PWR_ENABLE_SUSPENDM  (1<<0)

/* INTRUSB fields */
#define INTRUSB_VBUS_ERROR (1<<7)
#define INTRUSB_SESS_REQ   (1<<6)
#define INTRUSB_DISCON     (1<<5)
#define INTRUSB_CONN       (1<<4)
#define INTRUSB_SOF        (1<<3)
#define INTRUSB_RESET      (1<<2)
#define INTRUSB_RESUME     (1<<1)
#define INTRUSB_SUSPEND    (1<<0)

/* DMA control registers */
#define USB_DMA_INTR (USB_BASE + 0x0200)
#define USB_DMA_INTR_UNMASK_SET_OFFSET (24)

#define USB_DMA_CNTL(chan)	(USB_BASE + 0x0204 + 0x10*(chan-1))
#define USB_DMA_ADDR(chan)	(USB_BASE + 0x0208 + 0x10*(chan-1))
#define USB_DMA_COUNT(chan)	(USB_BASE + 0x020c + 0x10*(chan-1))

/* Endpoint Control/Status Registers */
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

/* control status register fields */
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

/* CONFIGDATA fields */
#define MP_RXE         (1<<7)
#define MP_TXE         (1<<6)
#define BIGENDIAN      (1<<5)
#define HBRXE          (1<<4)
#define HBTXE          (1<<3)
#define DYNFIFOSIZING  (1<<2)
#define SOFTCONE       (1<<1)
#define UTMIDATAWIDTH  (1<<0)

/* FIFO register */
/*
 * for endpint 1 ~ 4, writing to these addresses = writing to the
 * corresponding TX FIFO, reading from these addresses = reading from
 * corresponding RX FIFO
 */

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

/* FIFO size register fields and available packet size values */
#define DOUBLE_BUF	1
#define FIFOSZ_DPB	(1 << 4)
#define PKTSZ		0x0f

#define PKTSZ_8		(1<<3)
#define PKTSZ_16	(1<<4)
#define PKTSZ_32	(1<<5)
#define PKTSZ_64	(1<<6)
#define PKTSZ_128	(1<<7)
#define PKTSZ_256	(1<<8)
#define PKTSZ_512	(1<<9)
#define PKTSZ_1024	(1<<10)

#define FIFOSZ_8	(0x0)
#define FIFOSZ_16	(0x1)
#define FIFOSZ_32	(0x2)
#define FIFOSZ_64	(0x3)
#define FIFOSZ_128	(0x4)
#define FIFOSZ_256	(0x5)
#define FIFOSZ_512	(0x6)
#define FIFOSZ_1024	(0x7)
#define FIFOSZ_2048	(0x8)
#define FIFOSZ_4096	(0x9)
#define FIFOSZ_3072	(0xF)

/* SWRST fields */
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

/* USB level 1 interrupt registers */

#define USB_L1INTS (USB_BASE + 0xa0)  /* USB level 1 interrupt status register */
#define USB_L1INTM (USB_BASE + 0xa4)  /* USB level 1 interrupt mask register  */
#define USB_L1INTP (USB_BASE + 0xa8)  /* USB level 1 interrupt polarity register  */

#define TX_INT_STATUS		(1 << 0)
#define RX_INT_STATUS		(1 << 1)
#define USBCOM_INT_STATUS	(1 << 2)
#define DMA_INT_STATUS		(1 << 3)
#define PSR_INT_STATUS		(1 << 4)
#define QINT_STATUS		(1 << 5)
#define QHIF_INT_STATUS		(1 << 6)
#define DPDM_INT_STATUS		(1 << 7)
#define VBUSVALID_INT_STATUS	(1 << 8)
#define IDDIG_INT_STATUS	(1 << 9)
#define DRVVBUS_INT_STATUS	(1 << 10)
#define POWERDWN_INT_STATUS	(1 << 11)

#define VBUSVALID_INT_POL	(1 << 8)
#define IDDIG_INT_POL		(1 << 9)
#define DRVVBUS_INT_POL		(1 << 10)

/* mt_usb defines */
typedef enum
{
	EP0_IDLE = 0,
	EP0_RX,
	EP0_TX,
} EP0_STATE;

#if 0
typedef enum
{
	CHARGER_UNKNOWN,
	STANDARD_HOST,
	STANDARD_CHARGER,
	NONSTANDARD_CHARGER,
} USB_CHARGER_TYPE;
#endif

/* some macros */
#define EPMASK(X)	(1 << X)
#define CHANMASK(X)	(1 << X)

void mt_usb_phy_poweron(void);
void mt_usb_phy_savecurrent(void);

#endif
