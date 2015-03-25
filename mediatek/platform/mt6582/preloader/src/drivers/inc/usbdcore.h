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

#ifndef __USBDCORE_H__
#define __USBDCORE_H__

#include "typedefs.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

/* Request types */
#define USB_TYPE_STANDARD           (0x00 << 5)
#define USB_TYPE_CLASS              (0x01 << 5)
#define USB_TYPE_VENDOR             (0x02 << 5)
#define USB_TYPE_RESERVED           (0x03 << 5)

/* USB recipients */
#define USB_RECIP_DEVICE            0x00
#define USB_RECIP_INTERFACE         0x01
#define USB_RECIP_ENDPOINT          0x02
#define USB_RECIP_OTHER             0x03

/* USB transfer directions */
#define USB_DIR_OUT                 0
#define USB_DIR_IN                  0x80

#define USB_DT_DEVICE_QUALIFIER     0x06

/* Endpoints */
#define USB_EP_NUM_MASK             0x0f        /* in bEndpointAddress */
#define USB_EP_DIR_MASK             0x80

#define USB_EP_XFER_BULK            2
#define USB_EP_XFER_INT             3

/* Standard requests */
#define STDREQ_GET_STATUS           0x00
#define STDREQ_SET_ADDRESS          0x05
#define STDREQ_GET_DESCRIPTOR       0x06
#define STDREQ_GET_CONFIGURATION    0x08
#define STDREQ_SET_CONFIGURATION    0x09
#define STDREQ_GET_INTERFACE        0x0A

/* CDC ACM Class-specific requests */

#define CDCACM_REQ_SET_LINE_CODING          0x20
#define CDCACM_REQ_GET_LINE_CODING          0x21
#define CDCACM_REQ_SET_CONTROL_LINE_STATE   0x22
#define CDCACM_REQ_SEND_BREAK               0x23

/* USB release number (2.0 does not mean high speed!) */
#define USB_BCD_VERSION             0x0200

#define USB_DIR_MASK                0x80
#define USB_TYPE_MASK               0x60
#define USB_RECIP_MASK              0x1f

/* values used in GET_STATUS requests */
#define USB_STAT_SELFPOWERED        0x01

/* Descriptor types */
#define USB_DESCRIPTOR_TYPE_DEVICE                      0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION               0x02
#define USB_DESCRIPTOR_TYPE_STRING                      0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE                   0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT                    0x05
#define USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER            0x06

/* USB Requests
*
*/

struct device_request
{
    u8 bmRequestType;
    u8 bRequest;
    u16 wValue;
    u16 wIndex;
    u16 wLength;
} __attribute__ ((packed));

#define URB_BUF_SIZE 512

struct urb
{

    struct mt_ep *endpoint;
    struct mt_dev *device;
    struct device_request device_request;

    u8 *buffer;
    unsigned int buffer_length;
    unsigned int actual_length;

    u16 buffer_data[URB_BUF_SIZE];      /* buffer for data */
};

/* endpoint data */
struct mt_ep
{
    int endpoint_address;       /* endpoint address */

    /* rx side */
    struct urb *rcv_urb;        /* active urb */
    int rcv_packetSize;         /* maximum packet size from endpoint descriptor */

    /* tx side */
    struct urb *tx_urb;         /* active urb */
    int tx_packetSize;          /* maximum packet size from endpoint descriptor */

    int sent;                   /* data already sent */
    int last;                   /* data sent in last packet XXX do we need this */
};

struct mt_altsetting
{
    struct interface_descriptor *interface_descriptor;
    /* communication class specific interface descriptors */
    /* only communication interfaces have these fields */
    struct cdcacm_class_header_function_descriptor
        *header_function_descriptor;
    struct cdcacm_class_abstract_control_descriptor
        *abstract_control_descriptor;
    struct cdcacm_class_union_function_descriptor *union_function_descriptor;
    struct cdcacm_class_call_management_descriptor
        *call_management_descriptor;
    int endpoints;
    struct endpoint_descriptor **endpoints_descriptor_array;
};

struct mt_intf
{
    int alternates;
    struct mt_altsetting *altsetting_array;
};

struct mt_config
{
    int interfaces;
    struct configuration_descriptor *configuration_descriptor;
    struct mt_intf **interface_array;
};

struct mt_dev
{

    char *name;
    struct device_descriptor *device_descriptor;        /* per device descriptor */
    struct device_qualifier_descriptor *device_qualifier_descriptor;

    /* configuration descriptors */
    int configurations;
    struct mt_config *configuration_array;

    u8 address;                 /* function address, 0 by default */
    u8 configuration;           /* configuration, 0 by default, means unconfigured */
    u8 interface;               /* interface, 0 by default */
    u8 alternate;               /* alternate setting */
    u8 speed;
    struct mt_ep *endpoint_array;
    int max_endpoints;
    unsigned char maxpacketsize;
};

struct device_descriptor
{
    u8 bLength;
    u8 bDescriptorType;
    u16 bcdUSB;
    u8 bDeviceClass;
    u8 bDeviceSubClass;
    u8 bDeviceProtocol;
    u8 bMaxPacketSize0;
    u16 idVendor;
    u16 idProduct;
    u16 bcdDevice;
    u8 iManufacturer;
    u8 iProduct;
    u8 iSerialNumber;
    u8 bNumConfigurations;
} __attribute__ ((packed));

struct device_qualifier_descriptor
{
    u8 bLength;
    u8 bDescriptorType;
    u16 bcdUSB;
    u8 bDeviceClass;
    u8 bDeviceSubClass;
    u8 bDeviceProtocol;
    u8 bMaxPacketSize0;
    u8 bNumConfigurations;
} __attribute__ ((packed));

struct configuration_descriptor
{
    u8 bLength;
    u8 bDescriptorType;
    u16 wTotalLength;
    u8 bNumInterfaces;
    u8 bConfigurationValue;
    u8 iConfiguration;
    u8 bmAttributes;
    u8 bMaxPower;
} __attribute__ ((packed));

struct interface_descriptor
{
    u8 bLength;
    u8 bDescriptorType;
    u8 bInterfaceNumber;
    u8 bAlternateSetting;
    u8 bNumEndpoints;
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
    u8 iInterface;
} __attribute__ ((packed));

struct endpoint_descriptor
{
    u8 bLength;
    u8 bDescriptorType;
    u8 bEndpointAddress;
    u8 bmAttributes;
    u16 wMaxPacketSize;
    u8 bInterval;
};// __attribute__ ((packed));

struct string_descriptor
{
    u8 bLength;
    u8 bDescriptorType;         /* 0x03 */
    u16 wData[256];
};                              //__attribute__ ((packed));

/* Descriptors used by CDC ACM */
struct cdcacm_class_header_function_descriptor
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u16 bcdCDC;
} __attribute__ ((packed));

struct cdcacm_class_call_management_descriptor
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bmCapabilities;
    u8 bDataInterface;
} __attribute__ ((packed));

struct cdcacm_class_abstract_control_descriptor
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bmCapabilities;
} __attribute__ ((packed));

struct cdcacm_class_union_function_descriptor
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bMasterInterface;
    u8 bSlaveInterface0;
};                              //__attribute__ ((packed));

#endif
