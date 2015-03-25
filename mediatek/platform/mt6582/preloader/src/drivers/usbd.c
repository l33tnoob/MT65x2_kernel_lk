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

/* ================================================================== */
/* HEADER FILES                                                       */
/* ================================================================== */

#include "typedefs.h"
#include "platform.h"
#include "usbd.h"
#include "usbtty.h"
#include "usbphy.h"

/**************************************************************************
*  USB DEBUG
**************************************************************************/
#define  MT_USB_DBG_LOG   0

#if MT_USB_DBG_LOG
#define USB_LOG    print
#define USB_BUG() do { \
    printf("U-Boot BUG at %s:%d!\n", __FILE__, __LINE__); \
    } while (0)
#define USB_BUG_ON(condition) do { if ((condition)==FALSE) BUG(); } while(0)
#else
#define USB_LOG
#define USB_BUG() do {} while (0)
#define USB_BUG_ON(condition) do {} while(0)
//#define printf(a,...)
#endif

/* ================================================================== */
/* GLOBAL VARIABLES                                                   */
/* ================================================================== */
static struct urb *ep0_urb = NULL;
static struct mt_dev *udc_device = NULL;
EP0_STATE ep0_state = EP0_IDLE;
int set_address = 0;
int g_usbphy_ok = 0;
int g_usb_port_state = 0;
u32 fifo_addr = FIFO_ADDR_START;
extern struct string_descriptor **usb_string_table;

static struct usb_acm_line_coding g_line_coding = {
    921600, 0, 0, 8,
};

/* ================================================================== */
/* STATIC FUNCTION DECLARATIONS                                       */
/* ================================================================== */
static void copy_desc (struct urb *urb, void *data, int max_length,
                       int max_buf);
static int usb_get_descriptor (struct mt_dev *device,
struct urb *urb, int max, int descriptor_type,
    int index);
static int ep0_standard_setup (struct urb *urb);
static int ep0_class_setup (struct urb *urb);
static int mt_read_fifo (struct mt_ep *endpoint);
static int mt_write_fifo (struct mt_ep *endpoint);
static struct mt_ep *mt_find_ep (int ep_num, u8 dir);
static void mt_udc_flush_fifo (u8 ep_num, u8 dir);
static void mt_udc_reset (void);
static void mt_udc_ep0_write (void);
static void mt_udc_ep0_read (void);
static void mt_udc_ep0_setup (void);
static void mt_udc_ep0_handler (void);
static void mt_udc_epx_handler (u8 ep_num, u8 dir);
static void udc_stall_ep (unsigned int ep_num, u8 dir);
static void USB_UART_Share (u8 usb_mode);
static void USB_Charger_Detect_Init (void);
static void USB_Charger_Detect_Release (void);
static void USB_Check_Standard_Charger (void);

extern void mt_usb_phy_poweron (void);
extern void mt_usb_phy_savecurrent (void);
extern void mt_usb_phy_recover (void);

/* ================================================================== */
/* FUNCTION DEFINITIONS                                               */
/* ================================================================== */

static void copy_desc (struct urb *urb, void *data, int max_length, int max_buf)
{
    int available;
    int length;

    length = *(unsigned char *) data;

    if (length > max_length)
        length = max_length;

    if ((available = max_buf - urb->actual_length) <= 0)
    {
        return;
    }

    if (length > available)
    {
        length = available;
    }

    memcpy (urb->buffer + urb->actual_length, data, length);
    urb->actual_length += length;
}

static int usb_get_descriptor (struct mt_dev *device,
struct urb *urb, int max, int descriptor_type, int index)
{
    int port = 0;               /* XXX compound device */
    char *cp;

    if (!urb || !urb->buffer || !urb->buffer_length
        || (urb->buffer_length < 255 || !device))
    {
        return -1L;
    }

    /* setup tx urb */
    urb->actual_length = 0;
    cp = urb->buffer;

    switch (descriptor_type)
    {
        case USB_DESCRIPTOR_TYPE_DEVICE:
            {
                struct device_descriptor *dev_descriptor;

                if (!(dev_descriptor = device->device_descriptor))
                {
                    return -1;
                }
                /* copy device descriptor for this device */
                copy_desc (urb, dev_descriptor,
                    sizeof (struct device_descriptor), max);
            }

            break;
        case USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
            {
                struct device_qualifier_descriptor *dev_qualifier_descriptor;

                if (!
                    (dev_qualifier_descriptor =
                    device->device_qualifier_descriptor))
                {
                    return -1;
                }

                /* copy device qualifier descriptor for this device */
                copy_desc (urb, dev_qualifier_descriptor,
                    sizeof (struct device_qualifier_descriptor), max);
            }
            break;
        case USB_DESCRIPTOR_TYPE_CONFIGURATION:
            {
                int interfaceNum;
                int configNum;
                struct mt_config *config = NULL;
                struct configuration_descriptor *config_descriptor = NULL;

                configNum = (index == 0 ? 0 : index - 1);
                if (configNum >= device->configurations)
                    return -1;

                if (config = device->configuration_array + configNum)
                {
                    config_descriptor = config->configuration_descriptor;
                }
                else
                {
                    return -1;
                }

                copy_desc (urb, config_descriptor,
                    sizeof (struct configuration_descriptor), max);

                /* loop over all interfaces */
                for (interfaceNum = 0;
                    interfaceNum < config_descriptor->bNumInterfaces;
                    interfaceNum++)
                {

                    int alternateNum;
                    struct mt_intf *interface;
                    struct interface_descriptor *intf_descriptor;

                    interface = config->interface_array[interfaceNum];

                    if (!interface)
                    {
                        return -1;
                    }

                    /* loop over all interface alternates */
                    for (alternateNum = 0;
                        alternateNum < interface->alternates; alternateNum++)
                    {

                        int endpoint;
                        struct mt_altsetting *altsetting;

                        if (!
                            (altsetting =
                            interface->altsetting_array + alternateNum))
                        {
                            return -1;
                        }

                        if (!
                            (intf_descriptor =
                            altsetting->interface_descriptor))
                        {
                            return -1;
                        }

                        /* copy descriptor for this interface */
                        copy_desc (urb, intf_descriptor,
                            sizeof (struct interface_descriptor),
                            max);

                        if (altsetting->header_function_descriptor)
                        {
                            copy_desc (urb,
                                altsetting->
                                header_function_descriptor,
                                sizeof (struct
                                cdcacm_class_header_function_descriptor),
                                max);
                        }

                        if (altsetting->abstract_control_descriptor)
                        {
                            copy_desc (urb,
                                altsetting->
                                abstract_control_descriptor,
                                sizeof (struct
                                cdcacm_class_abstract_control_descriptor),
                                max);
                        }

                        if (altsetting->union_function_descriptor)
                        {
                            copy_desc (urb,
                                altsetting->
                                union_function_descriptor,
                                sizeof (struct
                                cdcacm_class_union_function_descriptor),
                                max);
                        }

                        if (altsetting->call_management_descriptor)
                        {
                            copy_desc (urb,
                                altsetting->
                                call_management_descriptor,
                                sizeof (struct
                                cdcacm_class_call_management_descriptor),
                                max);
                        }

                        /* iterate across endpoints for this alternate interface */
                        for (endpoint = 0;
                            endpoint < altsetting->endpoints; endpoint++)
                        {
                            struct endpoint_descriptor *ep_descriptor;

                            ep_descriptor =
                                *(altsetting->endpoints_descriptor_array +
                                endpoint);
                            if (!ep_descriptor)
                            {
                                return -1;
                            }

                            /* copy descriptor for this endpoint */
                            copy_desc (urb, ep_descriptor,
                                sizeof (struct
                                endpoint_descriptor), max);
                        }
                    }
                }
            }
            break;

        case USB_DESCRIPTOR_TYPE_STRING:
            {
                struct string_descriptor *string_descriptor;

                string_descriptor = usb_string_table[index];

                copy_desc (urb, string_descriptor, string_descriptor->bLength,
                    max);
            }
            break;

        default:
            return -1;
    }

    return 0;

}


/* Service standard usb requests, not all requests required for USBCV are
supported here */

static int ep0_standard_setup (struct urb *urb)
{
    struct device_request *request;
    struct mt_dev *device;
    char *cp = urb->buffer;

    if (!urb || !urb->device)
    {
        USB_LOG ("\n!urb || !urb->device\n");
        return -1;
    }

    request = &urb->device_request;
    device = urb->device;

    USB_LOG ("\nUSB: Device Request\n");
    USB_LOG ("       bmRequestType = %x\n", request->bmRequestType);
    USB_LOG ("       bRequest = %x\n", request->bRequest);
    USB_LOG ("       wValue = %x\n", request->wValue);
    USB_LOG ("       wIndex = %x\n", request->wIndex);
    USB_LOG ("       wLength = %x\n\n", request->wLength);

    if ((request->bmRequestType & USB_TYPE_MASK) != 0)
    {
        return -1;            /* Class-specific requests are handled elsewhere */
    }

    /* handle all requests that return data (direction bit set on bm RequestType) */
    if ((request->bmRequestType & USB_DIR_MASK))
    {

        ep0_state = EP0_TX;

        switch (request->bRequest)
        {
                /* data stage: from device to host */
            case STDREQ_GET_STATUS:

                urb->actual_length = 2;
                cp[0] = cp[1] = 0;
                switch (request->bmRequestType & USB_RECIP_MASK)
                {
                case USB_RECIP_DEVICE:
                    cp[0] = USB_STAT_SELFPOWERED;
                    break;
                case USB_RECIP_OTHER:
                    urb->actual_length = 0;
                    break;
                default:
                    break;
                }

                return 0;

            case STDREQ_GET_DESCRIPTOR:
                return usb_get_descriptor (device, urb,
                    (request->wLength),
                    (request->wValue) >> 8,
                    (request->wValue) & 0xff);

            case STDREQ_GET_CONFIGURATION:

                urb->actual_length = 1;
                ((char *) urb->buffer)[0] = device->configuration;
                return 0;

            case STDREQ_GET_INTERFACE:

                urb->actual_length = 1;
                ((char *) urb->buffer)[0] = device->alternate;
                return 0;

            default:
                USB_LOG ("Unsupported command with TX data stage\n");
                break;
        }
    }
    else
    {

        switch (request->bRequest)
        {

            case STDREQ_SET_ADDRESS:
                device->address = (request->wValue);
                set_address = 1;
                return 0;

            case STDREQ_SET_CONFIGURATION:

                device->configuration = (request->wValue) & 0x7f;
                device->interface = device->alternate = 0;
                config_usbtty (device);

                return 0;

            default:
                USB_LOG ("Unsupported command with RX data stage\n");
                break;

        }
    }
    //USB_LOG("USB: ep0_recv_setup(3)\n");
    return -1;
}

static int ep0_class_setup (struct urb *urb)
{

    struct device_request *request;
    struct mt_dev *device;

    request = &urb->device_request;
    device = urb->device;

    switch (request->bRequest)
    {
        case CDCACM_REQ_SET_LINE_CODING:
            USB_LOG ("CDC ACM Request: SET_LINE_CODING\n");
            ep0_state = EP0_RX;   // ??
            tool_state_update (1);
            break;
        case CDCACM_REQ_GET_LINE_CODING:
            USB_LOG ("CDC ACM Request: GET_LINE_CODING\n");
            memcpy (urb->buffer, &g_line_coding,
                sizeof (struct usb_acm_line_coding));
            urb->actual_length = sizeof (struct usb_acm_line_coding);
            //USB_LOG("urb->actual_length = %d\n", urb->actual_length);
            //USB_LOG("sizeof(struct USB_ACM_LINE_CODING = %d\n)", sizeof(struct USB_ACM_LINE_CODING));
            ep0_state = EP0_TX;
            break;
        case CDCACM_REQ_SET_CONTROL_LINE_STATE:
            USB_LOG ("CDC ACM Request: SET_CONTROL_LINE_STATE, request->wValue=%x\n",
                     request->wValue);
            g_usb_port_state = request->wValue;
            break;
        case CDCACM_REQ_SEND_BREAK:
            /* do nothing */
            USB_LOG ("CDC ACM Request: SEND_BREAK\n");
            break;
        default:
            return -1;
    }

    return 0;
}

static int mt_read_fifo (struct mt_ep *endpoint)
{
    struct urb *urb = endpoint->rcv_urb;
    int len = 0, count = 0;
    int ep_num = ((endpoint->endpoint_address) & USB_EP_NUM_MASK);
    int index;
    unsigned char *cp;
    unsigned char c;
    u32 *wp;
    u16 dma_cntl = 0;

    if (ep_num == 0)
        urb = ep0_urb;

    if (urb)
    {
        index = __raw_readb (INDEX);
        __raw_writeb (ep_num, INDEX);

        cp = (u8 *) (urb->buffer + urb->actual_length);
        wp = (u32 *) cp;
        //USB_LOG("USB: urb->actual_length = %d\n", urb->actual_length);

        count = len = __raw_readw (IECSR + RXCOUNT);
        if (ep_num != 0)
        {
            USB_LOG ("count = %d\n", count);
        }
        #if !CFG_FPGA_PLATFORM
        if (ep_num != 0)
        {
            __raw_writel (wp, USB_DMA_ADDR (ep_num));
            __raw_writel (count, USB_DMA_COUNT (ep_num));
            dma_cntl =
                USB_DMA_BURST_MODE_3 | (ep_num << USB_DMA_ENDPNT_OFFSET) |
                USB_DMA_EN;
            __raw_writew (dma_cntl, USB_DMA_CNTL (ep_num));
            while (__raw_readw (USB_DMA_CNTL (ep_num)) & USB_DMA_EN);
        }
        else
        #endif
        {
            while (len > 0)
            {
                if (len >= 4)
                {
                    *wp++ = __raw_readl (FIFO (ep_num));
                    cp = (unsigned char *) wp;
                    //printf("USB READ FIFO: wp = %lu, cp = %lu\n", wp, cp);
                    len -= 4;
                }
                else
                {
                    *cp++ = __raw_readb (FIFO (ep_num));
                    //printf("USB READ FIFO: wp = %lu, cp = %lu\n", wp, cp);
                    len--;
                }
            }
        }

        urb->actual_length += count;

        __raw_writeb (index, INDEX);
    }

    return count;
}

static int mt_write_fifo (struct mt_ep *endpoint)
{
    struct urb *urb = endpoint->tx_urb;
    int last = 0, count = 0;
    int ep_num = ((endpoint->endpoint_address) & USB_EP_NUM_MASK);
    int index;
    unsigned char *cp = NULL;

    if (ep_num == 0)
        urb = ep0_urb;

    if (urb)
    {
        index = __raw_readb (INDEX);
        __raw_writeb (ep_num, INDEX);
        count = last = MIN (urb->actual_length - endpoint->sent,  endpoint->tx_packetSize);
        USB_LOG ("[mt_write_fifo] urb->actual_length = %d\n", urb->actual_length);
        USB_LOG ("[mt_write_fifo] endpoint->sent = %d\n", endpoint->sent);
        if (count < 0)
        {
            print ("something is wrong");
        }
        if (count)
        {
            cp = urb->buffer + endpoint->sent;

            //USB_LOG("---------write USB fifo---------\n");
            while (count > 0)
            {
                //USB_LOG("%c",*cp);
                //print("%c",*cp);
                __raw_writeb (*cp, FIFO (ep_num));
                cp++;
                count--;
            }
        }

        endpoint->last = last;
        endpoint->sent += last;

        __raw_writeb (index, INDEX);
    }

    return last;
}

static struct mt_ep * mt_find_ep (int ep_num, u8 dir)
{
    int i;

    for (i = 0; i < udc_device->max_endpoints; i++)
    {
        if (((udc_device->endpoint_array[i].endpoint_address & USB_EP_NUM_MASK) == ep_num) && ((udc_device->endpoint_array[i].
            endpoint_address & USB_EP_DIR_MASK) == dir))
            return &udc_device->endpoint_array[i];
    }
    return NULL;
}

static void mt_udc_flush_fifo (u8 ep_num, u8 dir)
{

    u16 tmpReg16;
    u8 index;
    struct mt_ep *endpoint;

    index = __raw_readb (INDEX);
    __raw_writeb (ep_num, INDEX);

    if (ep_num == 0)
    {
        tmpReg16 = __raw_readw (IECSR + CSR0);
        tmpReg16 |= EP0_FLUSH_FIFO;
        __raw_writew (tmpReg16, IECSR + CSR0);
        __raw_writew (tmpReg16, IECSR + CSR0);
    }
    else
    {
        endpoint = mt_find_ep (ep_num, dir);
        if ((endpoint->endpoint_address & USB_EP_DIR_MASK) == USB_DIR_OUT)
        {
            tmpReg16 = __raw_readw (IECSR + RXCSR);
            tmpReg16 |= EPX_RX_FLUSHFIFO;
            __raw_writew (tmpReg16, IECSR + RXCSR);
            __raw_writew (tmpReg16, IECSR + RXCSR);
        }
        else
        {
            tmpReg16 = __raw_readw (IECSR + TXCSR);
            tmpReg16 |= EPX_TX_FLUSHFIFO;
            __raw_writew (tmpReg16, IECSR + TXCSR);
            __raw_writew (tmpReg16, IECSR + TXCSR);
        }
    }

    /* recover index register */
    __raw_writeb (index, INDEX);
}

static void mt_udc_rxtxmap_recover(void) {
	int i;

	for (i = 1; i < udc_device->max_endpoints; i++) {
		__raw_writeb(i, INDEX);

		if ((udc_device->endpoint_array[i].endpoint_address & USB_EP_DIR_MASK) == USB_DIR_IN)
			__raw_writel(udc_device->endpoint_array[i].rcv_packetSize, (IECSR + RXMAP));
		else
			__raw_writel(udc_device->endpoint_array[i].tx_packetSize, (IECSR + TXMAP));
	}
}

void mt_udc_reset (void)
{

    /* MUSBHDRC automatically does the following when reset signal is detected */
    /* 1. Sets FAddr to 0                                                      *
    * 2. Sets Index to 0                                                      *
    * 3. Flush all endpoint FIFOs                                             *
    * 4. Clears all control/status registers                                  *
    * 5. Enables all endpoint interrupts                                      *
    * 6. Generates a Rest interrupt                                           *
    */

    USB_LOG ("USB: mt_udc_reset\n");

    /* disable all endpoint interrupts */
    __raw_writeb (0, INTRTXE);
    __raw_writeb (0, INTRRXE);
    __raw_writeb (0, INTRUSBE);

    __raw_writew(SWRST_SWRST | SWRST_DISUSBRESET, SWRST);

    udc_device->address = 0;

    mt_udc_flush_fifo (0, USB_DIR_OUT);
    mt_udc_flush_fifo (1, USB_DIR_OUT);
    mt_udc_flush_fifo (1, USB_DIR_IN);
    //mt_udc_flush_fifo (2, USB_DIR_IN);

    if (__raw_readb (POWER) & PWR_HS_MODE)
    {
        print ("[USBD] USB High Speed\n");
        enable_highspeed ();
    }
    else
    {
        print ("[USBD] USB Full Speed\n");
    }

    /* restore RXMAP and TXMAP if the endpoint has been configured */
    mt_udc_rxtxmap_recover();
}

static void mt_udc_ep0_write (void)
{

    struct mt_ep *endpoint =
        (struct mt_ep *) (udc_device->endpoint_array + 0);
    int count = 0;
    u16 csr0 = 0;
    u8 index = 0;

    index = __raw_readb (INDEX);
    __raw_writeb (0, INDEX);

    csr0 = __raw_readw (IECSR + CSR0);
    if (csr0 & EP0_TXPKTRDY)
    {
        USB_LOG ("mt_udc_ep0_write: ep0 is not ready to be written\n");
        return;
    }

    count = mt_write_fifo (endpoint);

    //USB_LOG("ep0 write fifo, count = %d\n", count);

    if (count < EP0_MAX_PACKET_SIZE)
    {
        /* last packet */
        csr0 |= (EP0_TXPKTRDY | EP0_DATAEND);
        ep0_urb->actual_length = 0;
        endpoint->sent = 0;
        ep0_state = EP0_IDLE;
    }
    else
    {
        /* more packets are waiting to be transferred */
        csr0 |= EP0_TXPKTRDY;
    }

    __raw_writew (csr0, IECSR + CSR0);

    __raw_writeb (index, INDEX);

    return;
}

static void mt_udc_ep0_read (void)
{

    struct mt_ep *endpoint = (struct mt_ep *) (udc_device->endpoint_array + 0);
    int count = 0;
    u16 csr0 = 0;
    u8 index = 0;

    index = __raw_readb (INDEX);
    __raw_writeb (0, INDEX);

    csr0 = __raw_readw (IECSR + CSR0);

    /* erroneous ep0 interrupt */
    if (!(csr0 & EP0_RXPKTRDY))
    {
        return;
    }

    count = mt_read_fifo (endpoint);

    if (count <= EP0_MAX_PACKET_SIZE)
    {
        /* last packet */
        csr0 |= (EP0_SERVICED_RXPKTRDY | EP0_DATAEND);
        ep0_state = EP0_IDLE;
    }
    else
    {
        /* more packets are waiting to be transferred */
        csr0 |= EP0_SERVICED_RXPKTRDY;
    }

    __raw_writew (csr0, IECSR + CSR0);

    __raw_writeb (index, INDEX);

    return;
}

static void mt_udc_ep0_setup (void)
{
    struct mt_ep *endpoint = (struct mt_ep *) (udc_device->endpoint_array + 0);
    u8 index;
    u16 csr0;
    u16 count;
    u8 stall = 0;
    struct device_request *request;

    //USB_LOG("USB: mt_udc_ep0_setup\n");

    index = __raw_readb (INDEX);
    __raw_writeb (0, INDEX);
    /* Read control status register for endpiont 0 */
    csr0 = __raw_readw (IECSR + CSR0);

    /* check whether RxPktRdy is set? */
    if (!(csr0 & EP0_RXPKTRDY))
        return;
    /* unload fifo */
    ep0_urb->actual_length = 0;
    count = mt_read_fifo (endpoint);

    //USB_LOG("USB: mt_udc_ep0_setup, count = %d\n", count);
    /* decode command */
    request = &ep0_urb->device_request;
    memcpy (request, ep0_urb->buffer, sizeof (struct device_request));

    if (((request->bmRequestType) & USB_TYPE_MASK) == USB_TYPE_STANDARD)
    {
        USB_LOG ("USB: Standard Request\n");
        stall = ep0_standard_setup (ep0_urb);
        if (stall)
        {
            print ("[USB] STANDARD REQUEST NOT SUPPORTED\n");
            print ("       bmRequestType = %x\n",
                request->bmRequestType);
            print ("       bRequest = %x\n", request->bRequest);
            print ("       wValue = %x\n", request->wValue);
            print ("       wIndex = %x\n", request->wIndex);
            print ("       wLength = %x\n\n", request->wLength);
        }
    }
    else if (((request->bmRequestType) & USB_TYPE_MASK) == USB_TYPE_CLASS)
    {
        USB_LOG ("USB: Class-Specific Request\n");
        stall = ep0_class_setup (ep0_urb);
        if (stall)
        {
            print ("[USB] CLASS REQUEST NOT SUPPORTED\n");
            print ("       bmRequestType = %x\n",
                request->bmRequestType);
            print ("       bRequest = %x\n", request->bRequest);
            print ("       wValue = %x\n", request->wValue);
            print ("       wIndex = %x\n", request->wIndex);
            print ("       wLength = %x\n\n", request->wLength);
        }
    }
    else if (((request->bmRequestType) & USB_TYPE_MASK) == USB_TYPE_VENDOR)
    {
        USB_LOG ("USB: Vendor-Specific Request\n");
        /* do nothing now */
        print
            ("[USB] ALL VENDOR-SPECIFIC REQUESTS ARE NOT SUPPORTED!!\n");
    }

    if (stall)
    {
        /* the received command is not supported */
        udc_stall_ep (0, USB_DIR_OUT);
        return;
    }

    switch (ep0_state)
    {
    case EP0_TX:
        /* data stage: from device to host */
        csr0 = __raw_readw (IECSR + CSR0);
        csr0 |= (EP0_SERVICED_RXPKTRDY);
        __raw_writew (csr0, IECSR + CSR0);

        mt_udc_ep0_write ();

        break;
    case EP0_RX:
        /* data stage: from host to device */
        csr0 = __raw_readw (IECSR + CSR0);
        csr0 |= (EP0_SERVICED_RXPKTRDY);
        __raw_writew (csr0, IECSR + CSR0);

        break;
    case EP0_IDLE:
        /* no data stage */

        csr0 = __raw_readw (IECSR + CSR0);
        csr0 |= (EP0_SERVICED_RXPKTRDY | EP0_DATAEND);
        __raw_writew (csr0, IECSR + CSR0);
        break;
    default:
        break;
    }

    __raw_writeb (index, INDEX);
    return;
}

static void mt_udc_ep0_handler (void)
{

    u16 csr0;
    u8 index = 0;

    index = __raw_readb (INDEX);
    __raw_writeb (0, INDEX);

    csr0 = __raw_readw (IECSR + CSR0);

    if (csr0 & EP0_SENTSTALL)
    {
        USB_LOG ("USB: [EP0] SENTSTALL\n");
        csr0 &= ~EP0_SENTSTALL;
        __raw_writew (csr0, IECSR + CSR0);
        ep0_state = EP0_IDLE;
    }

    if (csr0 & EP0_SETUPEND)
    {
        USB_LOG ("USB: [EP0] SETUPEND\n");
        csr0 |= EP0_SERVICE_SETUP_END;
        __raw_writew (csr0, IECSR + CSR0);

        ep0_state = EP0_IDLE;
    }

    switch (ep0_state)
    {
        case EP0_IDLE:
            if (set_address)
            {
                __raw_writeb (udc_device->address, FADDR);
                set_address = 0;
            }
            mt_udc_ep0_setup ();
            break;
        case EP0_TX:
            mt_udc_ep0_write ();
            break;
        case EP0_RX:
            mt_udc_ep0_read ();
            break;
        default:
            break;
    }

    __raw_writeb (index, INDEX);

    return;
}

static void mt_udc_epx_handler (u8 ep_num, u8 dir)
{
    u8 index;
    u16 csr;
    u32 count;
    struct mt_ep *endpoint;
    struct urb *urb;

    endpoint = mt_find_ep (ep_num, dir);

    index = __raw_readb (INDEX);
    __raw_writeb (ep_num, INDEX);

    USB_LOG ("EP%d Interrupt\n", ep_num);

    switch (dir)
    {
        case USB_DIR_OUT:
            /* transfer direction is from host to device */
            /* from the view of usb device, it's RX */
            csr = __raw_readw (IECSR + RXCSR);

            if (csr & EPX_RX_SENTSTALL)
            {
                USB_LOG ("EP %d(RX): STALL\n", ep_num);
                /* exception handling: implement this!! */
                return;
            }

            if (!(csr & EPX_RX_RXPKTRDY))
            {
                USB_LOG ("EP %d: ERRONEOUS INTERRUPT\n", ep_num);
                return;
            }

            //printf("mt_read_fifo, start\n");
            count = mt_read_fifo (endpoint);
            //printf("mt_read_fifo, end\n");

            USB_LOG ("EP%d(RX), count = %d\n", ep_num, count);

            csr &= ~EPX_RX_RXPKTRDY;
            __raw_writew (csr, IECSR + RXCSR);
            if (__raw_readw (IECSR + RXCSR) & EPX_RX_RXPKTRDY)
            {
                USB_LOG ("epx handler: rxpktrdy clear failed\n");
            }
            else
            {
                USB_LOG ("epx handler: normal\n");
            }

            break;
        case USB_DIR_IN:
            /* transfer direction is from device to host */
            /* from the view of usb device, it's tx */
            //printf("Interrupt: EPX TX\n");

            csr = __raw_readw (IECSR + TXCSR);

            if (csr & EPX_TX_SENTSTALL)
            {
                USB_LOG ("EP %d(TX): STALL\n", ep_num);
                /* exception handling: implement this!! */
                return;
            }

            if (csr & EPX_TX_TXPKTRDY)
            {
                USB_LOG
                    ("mt_udc_epx_handler: ep%d is not ready to be written\n",
                    ep_num);
                return;
            }

            count = mt_write_fifo (endpoint);
            USB_LOG ("EP%d(TX), count = %d\n", ep_num, endpoint->sent);

            if (count != 0)
            {
                /* not the interrupt generated by the last tx packet of the transfer */
                csr |= EPX_TX_TXPKTRDY;
                __raw_writew (csr, IECSR + TXCSR);
            }

            urb = endpoint->tx_urb;

            if (endpoint->tx_urb->actual_length - endpoint->sent <= 0)
            {
                urb->actual_length = 0;
                endpoint->sent = 0;
                endpoint->last = 0;
            }

            break;
        default:
            break;
    }


    __raw_writeb (index, INDEX);

    return;
}

void mt_udc_irq (u8 intrtx, u8 intrrx, u8 intrusb)
{

    int i;

    USB_LOG ("INTERRUPT\n");

    if (intrusb)
    {
        if (intrusb & INTRUSB_RESUME)
        {
            USB_LOG ("INTRUSB: RESUME\n");
        }

        if (intrusb & INTRUSB_SESS_REQ)
        {
            USB_LOG ("INTRUSB: SESSION REQUEST\n");
        }

        if (intrusb & INTRUSB_VBUS_ERROR)
        {
            USB_LOG ("INTRUSB: VBUS ERROR\n");
        }

        if (intrusb & INTRUSB_SUSPEND)
        {
            USB_LOG ("INTRUSB: SUSPEND\n");
        }

        if (intrusb & INTRUSB_CONN)
        {
            USB_LOG ("INTRUSB: CONNECT\n");
        }

        if (intrusb & INTRUSB_DISCON)
        {
            USB_LOG ("INTRUSB: DISCONNECT\n");
        }

        if (intrusb & INTRUSB_RESET)
        {
            USB_LOG ("INTRUSB: RESET\n");
    	    g_usbphy_ok = 1;
            mt_udc_reset ();
        }

        if (intrusb & INTRUSB_SOF)
        {
            USB_LOG ("INTRUSB: SOF\n");
        }
    }

    /* endpoint 0 interrupt? */
    if (intrtx & EPMASK (0))
    {
        mt_udc_ep0_handler ();
        intrtx &= ~0x1;
    }

    if (intrtx)
    {
        for (i = 1; i < MT_EP_NUM; i++)
        {
            if (intrtx & EPMASK (i))
            {
                mt_udc_epx_handler (i, USB_DIR_IN);
            }
        }
    }

    if (intrrx)
    {
        for (i = 1; i < MT_EP_NUM; i++)
        {
            if (intrrx & EPMASK (i))
            {
                mt_udc_epx_handler (i, USB_DIR_OUT);
            }
        }
    }

}

/* **************************************************************************
*
* Start of public functions.
*
* ************************************************************************** */

/* Called to start packet transmission. */
void mt_ep_write (struct mt_ep *endpoint)
{
    int ep_num = (endpoint->endpoint_address & USB_EP_NUM_MASK);
    u8 index;
    u16 csr;

    //printf("udc_endpoint_write\n");

    index = __raw_readb (INDEX);
    __raw_writeb (ep_num, INDEX);

    if (ep_num == 0)
    {
        USB_LOG ("udc_endpoint_write: ep0 cannot be written\n");
        return;
    }
    else
    {
        if ((endpoint->endpoint_address & USB_EP_DIR_MASK) == USB_DIR_OUT)
        {
            USB_LOG ("udc_endpoint_write: ep%d is RX endpoint\n", ep_num);
        }
        else
        {
            csr = __raw_readw (IECSR + TXCSR);
            if (csr & EPX_TX_TXPKTRDY)
            {
                USB_LOG
                    ("udc_endpoint_write: ep%d is not ready to be written\n",
                    ep_num);
                return;
            }
            mt_write_fifo (endpoint);

            csr |= EPX_TX_TXPKTRDY;
            __raw_writew (csr, IECSR + TXCSR);
        }
    }

    __raw_writeb (index, INDEX);

    return;
}

int mt_ep_busy(struct mt_ep *endpoint){
    int ep_num = (endpoint->endpoint_address & USB_EP_NUM_MASK);
    int ret = 0;
    u8 index;
    u16 csr = 0;

    index = __raw_readb (INDEX);
    __raw_writeb (ep_num, INDEX);

    if ((endpoint->endpoint_address & USB_EP_DIR_MASK) == USB_DIR_OUT)
        {
            USB_LOG ("udc_endpoint_write: ep%d is RX endpoint\n", ep_num);
        }
        else
        {
            csr = __raw_readw (IECSR + TXCSR);
        }

    __raw_writeb (index, INDEX);

    return (csr & EPX_TX_TXPKTRDY);
}

/* the endpoint does not support the received command, stall it!! */
static void udc_stall_ep (unsigned int ep_num, u8 dir)
{
    struct mt_ep *endpoint = mt_find_ep (ep_num, dir);
    u8 index;
    u16 csr;

    USB_LOG ("USB: udc_stall_ep\n");

    index = __raw_readb (INDEX);
    __raw_writeb (ep_num, INDEX);

    if (ep_num == 0)
    {
        mt_udc_flush_fifo (ep_num, USB_DIR_OUT);
        csr = __raw_readw (IECSR + CSR0);
        csr |= EP0_SENDSTALL;
        __raw_writew (csr, IECSR + CSR0);
    }
    else
    {
        if ((endpoint->endpoint_address & USB_EP_DIR_MASK) == USB_DIR_OUT)
        {
            csr = __raw_readb (IECSR + RXCSR);
            csr |= EPX_RX_SENDSTALL;
            __raw_writew (csr, IECSR + RXCSR);
            mt_udc_flush_fifo (ep_num, USB_DIR_OUT);
        }
        else
        {
            csr = __raw_readb (IECSR + TXCSR);
            csr |= EPX_TX_SENDSTALL;
            __raw_writew (csr, IECSR + TXCSR);
            mt_udc_flush_fifo (ep_num, USB_DIR_IN);
        }
    }

    //mt_udc_flush_fifo (ep_num, USB_DIR_OUT);
    //mt_udc_flush_fifo (ep_num, USB_DIR_IN);

    ep0_state = EP0_IDLE;

    __raw_writeb (index, INDEX);

    return;
}

/*
* udc_setup_ep - setup endpoint
*
* Associate a physical endpoint with endpoint_instance and initialize FIFO
*/
void mt_setup_ep (struct mt_dev *device, unsigned int ep, struct mt_ep *endpoint)
{
    u8 index;
    u16 csr;
    int max_packet_size;

    /* Nothing needs to be done for endpoint 0 */
    if (ep == 0)
        return;

    index = __raw_readb (INDEX);
    __raw_writeb (ep, INDEX);

    /* Configure endpoint fifo */
    /* Set fifo address, fifo size, and fifo max packet size */
    if ((endpoint->endpoint_address & USB_EP_DIR_MASK) == USB_DIR_OUT)
    {
        /* Clear data toggle to 0 */
        csr = __raw_readw (IECSR + RXCSR);
        /* pangyen 20090911 */
        csr |= EPX_RX_CLRDATATOG | EPX_RX_FLUSHFIFO;
        __raw_writew (csr, IECSR + RXCSR);
        /* Set fifo address */
        __raw_writew (fifo_addr >> 3, RXFIFOADD);
        max_packet_size = endpoint->rcv_packetSize;
        /* Set fifo max packet size */
        __raw_writew (max_packet_size, IECSR + RXMAP);
        /* Set fifo size(double buffering is currently not enabled) */
        switch (max_packet_size)
        {
            case 8:
                __raw_writeb (FIFOSZ_8, RXFIFOSZ);
                break;
            case 16:
                __raw_writeb (FIFOSZ_16, RXFIFOSZ);
                break;
            case 32:
                __raw_writeb (FIFOSZ_32, RXFIFOSZ);
                break;
            case 64:
                __raw_writeb (FIFOSZ_64, RXFIFOSZ);
                break;
            case 128:
                __raw_writeb (FIFOSZ_128, RXFIFOSZ);
                break;
            case 256:
                __raw_writeb (FIFOSZ_256, RXFIFOSZ);
                break;
            case 512:
                __raw_writeb (FIFOSZ_512, RXFIFOSZ);
                break;
            case 1024:
                __raw_writeb (FIFOSZ_1024, RXFIFOSZ);
                break;
            case 2048:
                __raw_writeb (FIFOSZ_2048, RXFIFOSZ);
                break;
            case 4096:
                __raw_writeb (FIFOSZ_4096, RXFIFOSZ);
                break;
            case 3072:
                __raw_writeb (FIFOSZ_4096, RXFIFOSZ);
                break;
            default:
                USB_LOG ("The max_packet_size for ep %d is not supported\n", ep);
        }
    }
    else
    {
        /* Clear data toggle to 0 */
        csr = __raw_readw (IECSR + TXCSR);
        /* pangyen 20090911 */
        csr |= EPX_TX_CLRDATATOG | EPX_TX_FLUSHFIFO;
        __raw_writew (csr, IECSR + TXCSR);
        /* Set fifo address */
        __raw_writew (fifo_addr >> 3, TXFIFOADD);
        max_packet_size = endpoint->tx_packetSize;
        /* Set fifo max packet size */
        __raw_writew (max_packet_size, IECSR + TXMAP);
        /* Set fifo size(double buffering is currently not enabled) */
        switch (max_packet_size)
        {
            case 8:
                __raw_writeb (FIFOSZ_8, TXFIFOSZ);
                break;
            case 16:
                __raw_writeb (FIFOSZ_16, TXFIFOSZ);
                break;
            case 32:
                __raw_writeb (FIFOSZ_32, TXFIFOSZ);
                break;
            case 64:
                __raw_writeb (FIFOSZ_64, TXFIFOSZ);
                break;
            case 128:
                __raw_writeb (FIFOSZ_128, TXFIFOSZ);
                break;
            case 256:
                __raw_writeb (FIFOSZ_256, TXFIFOSZ);
                break;
            case 512:
                __raw_writeb (FIFOSZ_512, TXFIFOSZ);
                break;
            case 1024:
                __raw_writeb (FIFOSZ_1024, TXFIFOSZ);
                break;
            case 2048:
                __raw_writeb (FIFOSZ_2048, TXFIFOSZ);
                break;
            case 4096:
                __raw_writeb (FIFOSZ_4096, TXFIFOSZ);
                break;
            case 3072:
                __raw_writeb (FIFOSZ_4096, TXFIFOSZ);
                break;
            default:
                USB_LOG ("The max_packet_size for ep %d is not supported\n", ep);
        }
    }

    fifo_addr += max_packet_size;

    /* recover INDEX register */
    __raw_writeb (index, INDEX);
}

/* ************************************************************************** */

/* Turn on the USB connection by enabling the pullup resistor */
void mt_usb_connect_internal (void)
{
    u8 tmpReg8;

    /* connect */
    tmpReg8 = __raw_readb (POWER);
    tmpReg8 |= PWR_SOFT_CONN;
    tmpReg8 |= PWR_ENABLE_SUSPENDM;
#ifdef USB_FORCE_FULL_SPEED
    tmpReg8 &= ~PWR_HS_ENAB;
#else
    tmpReg8 |= PWR_HS_ENAB;
#endif
    __raw_writeb (tmpReg8, POWER);
}

/* Turn off the USB connection by disabling the pullup resistor */
void mt_usb_disconnect_internal (void)
{
    u8 tmpReg8;

    /* connect */
    tmpReg8 = __raw_readb (POWER);
    tmpReg8 &= ~PWR_SOFT_CONN;
    __raw_writeb (tmpReg8, POWER);
}

/* ************************************************************************** */

/* Switch on the UDC */
void udc_enable (struct mt_dev *device)
{
    struct mt_ep *ep;
    mt_usb_phy_recover ();

    /* clear INTRTX, INTRRX and INTRUSB */
    __raw_writew(0xffff, INTRTX);
    __raw_writew(0xffff, INTRRX);
    __raw_writeb(0xff, INTRUSB);

    udc_device = device;

    ep = &udc_device->endpoint_array[0];
    ep0_urb = usb_alloc_urb (udc_device, ep);

    /* reset USB hardware */
    mt_udc_reset();

    return;
}

/* Switch off the UDC */
void udc_disable (void)
{
    USB_LOG ("USB: DISABLE\n");
}

void usb_service_offline (void)
{

    mt_usb_disconnect_internal ();
    mt_usb_phy_savecurrent ();

    return;
}

/* Get LineState of DP/DM */
bool get_linestate(void)
{
    u8 tmpReg8;

    /* linestate */
    tmpReg8 = __raw_readb (DBG_PRB0);
    print("\n[USBD] USB PRB0 LineState: %x\n", tmpReg8);
    tmpReg8 = __raw_readb (DBG_PRB4);
    print("\n[USBD] USB PRB4 LineState: %x\n", tmpReg8);
}

bool is_uart_cable_inserted(void)
{
    u8 tmpReg8;

	USBPHY_CLR8(0x6b, 0x04);
	USBPHY_CLR8(0x6e, 0x01);
	USBPHY_CLR8(0x1a, 0x80);
	USBPHY_SET8(0x0, 0x20);
	USBPHY_SET8(0x68, 0x08);
	USBPHY_SET8(0x6a, 0x04);
	mdelay(2);

	tmpReg8 = USBPHY_READ8(0x6c);
	USBPHY_WRITE8(0x6c, tmpReg8 & 0xc1 | 0x02 | 0x04 | 0x08 | 0x20);
	tmpReg8 = USBPHY_READ8(0x6d);
	USBPHY_WRITE8(0x6d, tmpReg8 & 0xc1 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20);
	mdelay(2);

    /* linestate */
    tmpReg8 = __raw_readb (DBG_PRB0);
    print("\n[USBD] USB PRB0 LineState: %x\n", tmpReg8);

	if (tmpReg8 == 0xC0 ||
		tmpReg8 == 0x80) {
		/* Prolific/FTDI UART cable inserted */
        print("\n[USBD] Prolific/FTDI UART cable inserted!\n");
		return true;
    }
	else {
        /* Step 1. Power down mode OTG setting */
        tmpReg8 = USBPHY_READ8(0x6c);
        USBPHY_WRITE8(0x6c, tmpReg8 & 0xc1 | 0x02 | 0x10);
        /* Step 2. Do not apply USB IP reset, Wait MAC off ready */
        mdelay(10);
        /* Step 3. Clear OTG Force mode */
        tmpReg8 = USBPHY_READ8(0x6d);
        USBPHY_WRITE8(0x6d, tmpReg8 & 0xc1);
        /* Step 4. */
	    USBPHY_CLR8(0x68, 0x08);
        /* Step 5. */
	    USBPHY_SET8(0x6a, 0x04);
        /* Step 6. Wait 2ms */
	    mdelay(2);
		/* USB cable/ No Cable inserted */
        print("\n[USBD] USB cable/ No Cable inserted!\n");
		return false;
    }
}
