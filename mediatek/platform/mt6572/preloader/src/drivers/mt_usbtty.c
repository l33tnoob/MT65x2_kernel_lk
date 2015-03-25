/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include "platform.h"
#include "circbuf.h"
#include "mt_usbtty.h"

#if 0
#define TTYDBG(fmt,args...) print("[%s] %s %d: "fmt, __FILE__,__FUNCTION__,__LINE__,##args)
#else
#define TTYDBG(fmt,args...) do{}while(0)
#endif

#if 0
#define TTYERR(fmt,args...) print("ERROR![%s] %s %d: "fmt, __FILE__,__FUNCTION__,__LINE__,##args)
#else
#define TTYERR(fmt,args...) do{}while(0)
#endif

/**************************************************************************
 *  USB TTY DEBUG
 **************************************************************************/
#define  mt6573_USB_TTY_DBG_LOG   0

#if mt6573_USB_TTY_DBG_LOG
#define USB_TTY_LOG    print
#else
#define USB_TTY_LOG
#endif

/* USB input/output data buffers */
static circbuf_t usb_input_buffer;
static circbuf_t usb_output_buffer;

static struct mt_dev mt_usb_device[1];
static struct mt_config mt_usb_config[NUM_CONFIGS];
static struct mt_intf *mt_usb_interface[NUM_INTERFACES];
static struct mt_intf mt_usb_data_interface[NUM_DATA_INTERFACES];
static struct mt_altsetting
    mt_usb_data_alternate_interface[NUM_DATA_INTERFACES];
static struct mt_intf mt_usb_comm_interface[NUM_COMM_INTERFACES];
static struct mt_altsetting
    mt_usb_comm_alternate_interface[NUM_COMM_INTERFACES];
static struct mt_ep mt_usb_ep[NUM_ENDPOINTS + 1];       /* one extra for control endpoint */
u16 serialstate;

struct urb mt6573_tx_urb;
struct urb mt6573_rx_urb;
struct urb mt6573_ep0_urb;

#define RX_ENDPOINT 1
#define TX_ENDPOINT 2

int usb_configured = 0;
int tool_exists = 0;

struct string_descriptor **usb_string_table;

/* USB descriptors */

/* string descriptors */
static u8 language[4] = { 4, USB_DESCRIPTOR_TYPE_STRING, 0x9, 0x4 };
static u8 manufacturer[2 + 2 * (sizeof (USBD_MANUFACTURER) - 1)];
static u8 product[2 + 2 * (sizeof (USBD_PRODUCT_NAME) - 1)];
static u8 configuration[2 + 2 * (sizeof (USBD_CONFIGURATION_STR) - 1)];
static u8 dataInterface[2 + 2 * (sizeof (USBD_DATA_INTERFACE_STR) - 1)];
static u8 commInterface[2 + 2 * (sizeof (USBD_COMM_INTERFACE_STR) - 1)];

static struct string_descriptor *usbtty_string_table[] = {
    (struct string_descriptor *) language,
    (struct string_descriptor *) manufacturer,
    (struct string_descriptor *) product,
    (struct string_descriptor *) configuration,
    (struct string_descriptor *) dataInterface,
    (struct string_descriptor *) commInterface,
};

/* device descriptor */
static struct device_descriptor device_descriptor = {
    sizeof (struct device_descriptor),
    USB_DESCRIPTOR_TYPE_DEVICE,
    USB_BCD_VERSION,
    USBDL_DEVICE_CLASS,
    USBDL_DEVICE_SUBCLASS,
    USBDL_DEVICE_PROTOCOL,
    EP0_MAX_PACKET_SIZE,
    USBD_VENDORID,
    USBD_PRODUCTID,
    USBD_BCD_DEVICE,
    STR_MANUFACTURER,
    STR_PRODUCT,
    0,
    NUM_CONFIGS
};

/* device qualifier descriptor */
static struct device_qualifier_descriptor device_qualifier_descriptor = {
    sizeof (struct device_qualifier_descriptor),
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    USB_BCD_VERSION,
    USBDL_DEVICE_CLASS,
    USBDL_DEVICE_SUBCLASS,
    USBDL_DEVICE_PROTOCOL,
    EP0_MAX_PACKET_SIZE_FULL,
    NUM_CONFIGS,
};

/* configuration descriptor */
static struct configuration_descriptor config_descriptors[NUM_CONFIGS] = {
    {
     sizeof (struct configuration_descriptor),
     USB_DESCRIPTOR_TYPE_CONFIGURATION,
     (sizeof (struct configuration_descriptor) * NUM_CONFIGS) +
     (sizeof (struct interface_descriptor) * NUM_INTERFACES) +
     (sizeof (struct cdcacm_class_header_function_descriptor)) +
     (sizeof (struct cdcacm_class_abstract_control_descriptor)) +
     (sizeof (struct cdcacm_class_union_function_descriptor)) +
     (sizeof (struct cdcacm_class_call_management_descriptor)) +
     (sizeof (struct endpoint_descriptor) * NUM_ENDPOINTS),
     NUM_INTERFACES,
     1,
     STR_CONFIG,
     0xc0,
     USBD_MAXPOWER},
};


/* interface_descriptors[0]: data interface          *
 * interface_descriptors[1]: communication interface */
static struct interface_descriptor interface_descriptors[NUM_INTERFACES] = {
    {
     sizeof (struct interface_descriptor),
     USB_DESCRIPTOR_TYPE_INTERFACE,
     0,
     0,
     NUM_DATA_ENDPOINTS,
     USBDL_DATA_INTERFACE_CLASS,
     USBDL_DATA_INTERFACE_SUBCLASS,
     USBDL_DATA_INTERFACE_PROTOCOL,
     STR_DATA_INTERFACE},
    {
     sizeof (struct interface_descriptor),
     USB_DESCRIPTOR_TYPE_INTERFACE,
     1,
     0,
     NUM_COMM_ENDPOINTS,
     USBDL_COMM_INTERFACE_CLASS,
     USBDL_COMM_INTERFACE_SUBCLASS,
     USBDL_COMM_INTERFACE_PROTOCOL,
     STR_COMM_INTERFACE},
};

static struct cdcacm_class_header_function_descriptor
    header_function_descriptor = {
    0x05,
    0x24,
    0x00,                       /* 0x00 for header functional descriptor */
    0x0110,
};

static struct cdcacm_class_abstract_control_descriptor
    abstract_control_descriptor = {
    0x04,
    0x24,
    0x02,                       /* 0x02 for abstract control descriptor */
    0x0f,
};

struct cdcacm_class_union_function_descriptor union_function_descriptor = {
    0x05,
    0x24,
    0x06,                       /* 0x06 for union functional descriptor */
    0x01,
    0x00,
};

struct cdcacm_class_call_management_descriptor call_management_descriptor = {
    0x05,
    0x24,
    0x01,                       /* 0x01 for call management descriptor */
    0x03,
    0x00,
};

static struct endpoint_descriptor hs_ep_descriptors[NUM_ENDPOINTS] = {
    {
     sizeof (struct endpoint_descriptor),
     USB_DESCRIPTOR_TYPE_ENDPOINT,
     USBD_SERIAL_OUT_ENDPOINT | USB_DIR_OUT,
     USB_EP_XFER_BULK,
     USBD_SERIAL_OUT_HS_PKTSIZE,
     0},
    {
     sizeof (struct endpoint_descriptor),
     USB_DESCRIPTOR_TYPE_ENDPOINT,
     USBD_SERIAL_IN_ENDPOINT | USB_DIR_IN,
     USB_EP_XFER_BULK,
     USBD_SERIAL_IN_HS_PKTSIZE,
     0},
    {
     sizeof (struct endpoint_descriptor),
     USB_DESCRIPTOR_TYPE_ENDPOINT,
     USBD_INT_IN_ENDPOINT | USB_DIR_IN,
     USB_EP_XFER_INT,
     USBD_INT_IN_HS_PKTSIZE,
     0x10                       /* polling interval is every 16 frames */
     },
};

static struct endpoint_descriptor fs_ep_descriptors[NUM_ENDPOINTS] = {
    {
     sizeof (struct endpoint_descriptor),
     USB_DESCRIPTOR_TYPE_ENDPOINT,
     USBD_SERIAL_OUT_ENDPOINT | USB_DIR_OUT,
     USB_EP_XFER_BULK,
     USBD_SERIAL_OUT_FS_PKTSIZE,
     0},
    {
     sizeof (struct endpoint_descriptor),
     USB_DESCRIPTOR_TYPE_ENDPOINT,
     USBD_SERIAL_IN_ENDPOINT | USB_DIR_IN,
     USB_EP_XFER_BULK,
     USBD_SERIAL_IN_FS_PKTSIZE,
     0},
    {
     sizeof (struct endpoint_descriptor),
     USB_DESCRIPTOR_TYPE_ENDPOINT,
     USBD_INT_IN_ENDPOINT | USB_DIR_IN,
     USB_EP_XFER_INT,
     USBD_INT_IN_FS_PKTSIZE,
     0x10                       /* polling interval is every 16 frames */
     },
};

static struct endpoint_descriptor
    *hs_data_ep_descriptor_ptrs[NUM_DATA_ENDPOINTS] = {
    &(hs_ep_descriptors[0]),
    &(hs_ep_descriptors[1]),
};

static struct endpoint_descriptor
    *hs_comm_ep_descriptor_ptrs[NUM_COMM_ENDPOINTS] = {
    &(hs_ep_descriptors[2]),
};

static struct endpoint_descriptor
    *fs_data_ep_descriptor_ptrs[NUM_DATA_ENDPOINTS] = {
    &(fs_ep_descriptors[0]),
    &(fs_ep_descriptors[1]),
};

static struct endpoint_descriptor
    *fs_comm_ep_descriptor_ptrs[NUM_COMM_ENDPOINTS] = {
    &(fs_ep_descriptors[2]),
};


static void
str2wide (char *str, u16 * wide)
{
    int i;

    for (i = 0; i < strlen (str) && str[i]; i++)
        wide[i] = (u16) str[i];
}

int usbdl_configured (void);

static void buf_to_ep (circbuf_t * buf);
static int ep_to_buf (circbuf_t * buf);

void usbdl_poll (void);
void service_interrupts (void);

struct urb *
usb_alloc_urb (struct mt_dev *device, struct mt_ep *endpoint)
{
    struct urb *urb;
    int ep_num = 0;
    int dir = 0;

    ep_num = endpoint->endpoint_address & USB_EP_NUM_MASK;
    dir = endpoint->endpoint_address & USB_EP_DIR_MASK;

    if (ep_num == 0)
      {
          urb = &mt6573_ep0_urb;
      }
    else if (dir)
      {                         // tx
          urb = &mt6573_tx_urb;
      }
    else
      {                         // rx
          urb = &mt6573_rx_urb;
      }

    memset (urb, 0, sizeof (struct urb));
    urb->endpoint = endpoint;
    urb->device = device;
    urb->buffer = (u8 *) urb->buffer_data;
    urb->buffer_length = sizeof (urb->buffer_data);

    return urb;
}

/*
 * Initialize the usb client port.
 *
 */
int
usbdl_init (void)
{
    /* initialize usb variables */
    extern int usb_configured;
    usb_configured = 0;
    extern EP0_STATE ep0_state;
    ep0_state = EP0_IDLE;
    extern int set_address;
    set_address = 0;
    extern u32 fifo_addr;
    fifo_addr = FIFO_ADDR_START;

    int i;
    struct string_descriptor *string;
    tool_exists = 0;

    /* prepare buffers... */
    buf_input_init (&usb_input_buffer, USBTTY_BUFFER_SIZE);
    buf_output_init (&usb_output_buffer, USBTTY_BUFFER_SIZE);

    /* initialize string descriptor array */
    string = (struct string_descriptor *) manufacturer;
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    string->bLength = sizeof (manufacturer);
    str2wide (USBD_MANUFACTURER, string->wData);

    string = (struct string_descriptor *) product;
    string->bLength = sizeof (product);
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    str2wide (USBD_PRODUCT_NAME, string->wData);

    string = (struct string_descriptor *) configuration;
    string->bLength = sizeof (configuration);
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    str2wide (USBD_CONFIGURATION_STR, string->wData);

    string = (struct string_descriptor *) dataInterface;
    string->bLength = sizeof (dataInterface);
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    str2wide (USBD_DATA_INTERFACE_STR, string->wData);

    string = (struct string_descriptor *) commInterface;
    string->bLength = sizeof (commInterface);
    string->bDescriptorType = USB_DESCRIPTOR_TYPE_STRING;
    str2wide (USBD_COMM_INTERFACE_STR, string->wData);

    /* Now, initialize the string table for ep0 handling */
    usb_string_table = usbtty_string_table;

    /* device instance initialization */
    memset (mt_usb_device, 0, sizeof (struct mt_dev));
    mt_usb_device->device_descriptor = &device_descriptor;
    mt_usb_device->device_qualifier_descriptor = &device_qualifier_descriptor;
    mt_usb_device->configurations = NUM_CONFIGS;
    mt_usb_device->configuration_array = mt_usb_config;
    mt_usb_device->speed = 0;   //1: high-speed, 0: full-speed
    mt_usb_device->endpoint_array = mt_usb_ep;
    mt_usb_device->max_endpoints = 1;
    mt_usb_device->maxpacketsize = 64;

    /* configuration instance initialization */
    memset (mt_usb_config, 0, sizeof (struct mt_config));
    mt_usb_config->interfaces = NUM_INTERFACES;
    mt_usb_config->configuration_descriptor = config_descriptors;
    mt_usb_config->interface_array = mt_usb_interface;

    mt_usb_interface[0] = mt_usb_data_interface;
    mt_usb_interface[1] = mt_usb_comm_interface;

    /* data interface instance */
    memset (mt_usb_data_interface, 0,
            NUM_DATA_INTERFACES * sizeof (struct mt_intf));
    mt_usb_data_interface->alternates = 1;
    mt_usb_data_interface->altsetting_array = mt_usb_data_alternate_interface;

    /* data alternates instance */
    memset (mt_usb_data_alternate_interface, 0,
            NUM_DATA_INTERFACES * sizeof (struct mt_altsetting));
    mt_usb_data_alternate_interface->interface_descriptor =
        &interface_descriptors[0];
    mt_usb_data_alternate_interface->endpoints = NUM_DATA_ENDPOINTS;
    mt_usb_data_alternate_interface->endpoints_descriptor_array =
        fs_data_ep_descriptor_ptrs;

    /* communication interface instance */
    memset (mt_usb_comm_interface, 0,
            NUM_COMM_INTERFACES * sizeof (struct mt_intf));
    mt_usb_comm_interface->alternates = 1;
    mt_usb_comm_interface->altsetting_array = mt_usb_comm_alternate_interface;

    /* communication alternates instance */
    /* contains communication class specific interface descriptors */
    memset (mt_usb_comm_alternate_interface, 0,
            NUM_COMM_INTERFACES * sizeof (struct mt_altsetting));
    mt_usb_comm_alternate_interface->interface_descriptor =
        &interface_descriptors[1];
    mt_usb_comm_alternate_interface->header_function_descriptor =
        &header_function_descriptor;
    mt_usb_comm_alternate_interface->abstract_control_descriptor =
        &abstract_control_descriptor;
    mt_usb_comm_alternate_interface->union_function_descriptor =
        &union_function_descriptor;
    mt_usb_comm_alternate_interface->call_management_descriptor =
        &call_management_descriptor;
    mt_usb_comm_alternate_interface->endpoints = NUM_COMM_ENDPOINTS;
    mt_usb_comm_alternate_interface->endpoints_descriptor_array =
        fs_comm_ep_descriptor_ptrs;

    /* endpoint instances */
    memset (&mt_usb_ep[0], 0, sizeof (struct mt_ep));
    mt_usb_ep[0].endpoint_address = 0;
    mt_usb_ep[0].rcv_packetSize = EP0_MAX_PACKET_SIZE;
    mt_usb_ep[0].tx_packetSize = EP0_MAX_PACKET_SIZE;
    mt_setup_ep (mt_usb_device, 0, &mt_usb_ep[0]);

    for (i = 1; i <= NUM_ENDPOINTS; i++)
      {
          memset (&mt_usb_ep[i], 0, sizeof (struct mt_ep));

          mt_usb_ep[i].endpoint_address =
              fs_ep_descriptors[i - 1].bEndpointAddress;

          mt_usb_ep[i].rcv_packetSize =
              fs_ep_descriptors[i - 1].wMaxPacketSize;

          mt_usb_ep[i].tx_packetSize =
              fs_ep_descriptors[i - 1].wMaxPacketSize;

          if (mt_usb_ep[i].endpoint_address & USB_DIR_IN)
              mt_usb_ep[i].tx_urb =
                  usb_alloc_urb (mt_usb_device, &mt_usb_ep[i]);
          else
              mt_usb_ep[i].rcv_urb =
                  usb_alloc_urb (mt_usb_device, &mt_usb_ep[i]);
      }

    udc_enable (mt_usb_device);
    return 0;
}

/*********************************************************************************/

static void
buf_to_ep (circbuf_t * buf)
{
    int i;

    if (!usbdl_configured ())
      {
          return;
      }

    if (buf->size)
      {

          struct mt_ep *endpoint = &mt_usb_ep[TX_ENDPOINT];
          struct urb *current_urb = endpoint->tx_urb;

          int space_avail;
          int popnum;

          /* Break buffer into urb sized pieces, and link each to the endpoint */
          while (buf->size > 0)
            {

                if (!current_urb
                    || (space_avail =
                        current_urb->buffer_length -
                        current_urb->actual_length) <= 0)
                  {
                      print ("write_buffer, no available urbs\n");
                      return;
                  }

                //buf_pop (buf, dest, popnum);
                popnum = buf_pop (buf, current_urb->buffer +
                                  current_urb->actual_length,
                                  MIN (space_avail, buf->size));

                /* update the used space of current_urb */
                current_urb->actual_length += popnum;

                /* nothing is in the buffer or the urb can hold no more data */
                if (popnum == 0)
                    break;

                /* if the endpoint is idle, trigger the tx transfer */
                if (endpoint->last == 0)
                  {
                      mt_ep_write (endpoint);
                  }

            }                   /* end while */
      }                         /* end if buf->size */

    return;
}

static int
ep_to_buf (circbuf_t * buf)
{
    struct mt_ep *endpoint;
    int nb;

    if (!usbdl_configured)
        return 0;

    endpoint = &mt_usb_ep[RX_ENDPOINT];
    nb = endpoint->rcv_urb->actual_length;

    if (endpoint->rcv_urb && nb)
      {
          buf_push (buf, (char *) endpoint->rcv_urb->buffer,
                    endpoint->rcv_urb->actual_length);
          endpoint->rcv_urb->actual_length = 0;

          return nb;
      }

    return 0;
}

int
tool_is_present (void)
{
    return tool_exists;
}

void
tool_state_update (int state)
{
    tool_exists = state;

    return;
}

int
usbdl_configured (void)
{
    return usb_configured;
}

void
enable_highspeed (void)
{

    int i;

    mt_usb_device->speed = 1;   //1: high-speed, 0: full-speed
    mt_usb_data_alternate_interface->endpoints_descriptor_array =
        hs_data_ep_descriptor_ptrs;
    mt_usb_comm_alternate_interface->endpoints_descriptor_array =
        hs_comm_ep_descriptor_ptrs;

    for (i = 1; i <= NUM_ENDPOINTS; i++)
      {

          mt_usb_ep[i].endpoint_address =
              hs_ep_descriptors[i - 1].bEndpointAddress;

          mt_usb_ep[i].rcv_packetSize =
              hs_ep_descriptors[i - 1].wMaxPacketSize;

          mt_usb_ep[i].tx_packetSize =
              hs_ep_descriptors[i - 1].wMaxPacketSize;
      }

    return;
}

//#define usbtty_event_log print
#define usbtty_event_log

/*********************************************************************************/
void
config_usbtty (struct mt_dev *device)
{

    int i;

    usb_configured = 1;
    mt_usb_device->max_endpoints = NUM_ENDPOINTS + 1;
    for (i = 0; i <= NUM_ENDPOINTS; i++)
      {
          mt_setup_ep (mt_usb_device, mt_usb_ep[i].endpoint_address & (~USB_DIR_IN), &mt_usb_ep[i]);
      }

    return;
}

/*********************************************************************************/



/* Used to emulate interrupt handling */
void
usbdl_poll (void)
{
    /* New interrupts? */
    service_interrupts ();

    /* Write any output data to host buffer (do this before checking interrupts to avoid missing one) */
    buf_to_ep (&usb_output_buffer);

    /* Check for new data from host.. (do this after checking interrupts to get latest data) */
    ep_to_buf (&usb_input_buffer);
}

extern ulong get_timer(ulong base);

void usbdl_flush(void)
{
    u32 start_time = get_timer(0);

    while (((usb_output_buffer.size) > 0) || mt_ep_busy(&mt_usb_ep[TX_ENDPOINT]))
    {
        usbdl_poll ();

        if(get_timer(start_time) > 300)
        {
            print ("usbdl_flush timeout\n");
            break;
        }
    }

    return;
}

void
service_interrupts (void)
{

    volatile u8 intrtx, intrrx, intrusb;
    /* polling interrupt status for incoming interrupts and service it */
    u16 rxcsr;

    intrtx = __raw_readb (INTRTX);
    __raw_writew(intrtx, INTRTX);
    intrrx = __raw_readb (INTRRX);
    __raw_writew(intrrx, INTRRX);
    intrusb = __raw_readb (INTRUSB);
    __raw_writeb(intrusb, INTRUSB);

    intrusb &= ~INTRUSB_SOF;

    if (intrtx | intrrx | intrusb)
    {
        mt_udc_irq (intrtx, intrrx, intrusb);
    }

}

/* API for preloader download engine */

void mt_usbtty_flush(void) 
{
    usbdl_flush();
}

/*
 * Test whether a character is in the RX buffer
 */
int
mt_usbtty_tstc (void)
{

    usbdl_poll ();
    return (usb_input_buffer.size > 0);
}


/* get a single character and copy it to usb_input_buffer */
int
mt_usbtty_getc (void)
{

    char c;

    while (usb_input_buffer.size <= 0)
      {
          usbdl_poll ();
      }

    buf_pop (&usb_input_buffer, &c, 1);
    return c;
}

/* get n characters and copy it to usb_input_buffer */
int
mt_usbtty_getcn (int count, char *buf)
{

    int data_count = 0;
    int tmp = 0;

    /* wait until received 'count' bytes of data */
    while (data_count < count)
      {
          if (usb_input_buffer.size < 512)
              usbdl_poll ();
          if (usb_input_buffer.size > 0)
            {
                tmp = usb_input_buffer.size;
                if (data_count + tmp > count)
                  {
                      tmp = count - data_count;
                  }

                //print("usb_input_buffer.data = %s\n",usb_input_buffer.data);
                buf_pop (&usb_input_buffer, buf + data_count, tmp);
                data_count += tmp;
            }
      }

    return 0;
}

void
mt_usbtty_putc (const char c, int flush)
{

    buf_push (&usb_output_buffer, &c, 1);

    /* Poll at end to handle new data... */
    if (((usb_output_buffer.size) >= usb_output_buffer.totalsize) || flush)
      {
          usbdl_poll ();
      }

    return;
}

void
mt_usbtty_putcn (int count, char *buf, int flush)
{

    char *cp = buf;

    while (count > 0)
      {
          if (count > 512)
            {
                buf_push (&usb_output_buffer, cp, 512);
                cp += 512;
                count -= 512;
            }
          else
            {
                buf_push (&usb_output_buffer, cp, count);
                cp += count;
                count = 0;
            }

          if (((usb_output_buffer.size) >= usb_output_buffer.totalsize)
              || flush)
            {
                usbdl_poll ();
            }
      }

    return;
}

void
mt_usbtty_puts (const char *str)
{
    int len = strlen (str);
    int maxlen = usb_output_buffer.totalsize;
    int space, n;

    /* break str into chunks < buffer size, if needed */
    while (len > 0)
      {
          space = maxlen - usb_output_buffer.size;

          /* Empty buffer here, if needed, to ensure space... */
#if 0
          if (space <= 0)
            {
                ASSERT (0);
            }
#endif
          n = MIN (space, MIN (len, maxlen));

          buf_push (&usb_output_buffer, str, n);

          str += n;
          len -= n;

          service_interrupts ();
      }

    /* Poll at end to handle new data... */
    usbdl_poll ();
    usbdl_flush();
}

int
mt_usbtty_query_data_size (void)
{

    if (usb_input_buffer.size < 512)
      {
          if (usbdl_configured ())
            {
                service_interrupts ();
                ep_to_buf (&usb_input_buffer);
            }
      }

    return usb_input_buffer.size;
}

