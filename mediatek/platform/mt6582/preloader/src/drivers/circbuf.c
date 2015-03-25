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

#include "circbuf.h"
#include "usbtty.h"
#include "platform.h"

static char g_usb_input_buf[USBTTY_BUFFER_SIZE];
static char g_usb_output_buf[USBTTY_BUFFER_SIZE];

int buf_input_init (circbuf_t * buf, unsigned int size)
{
    ASSERT (buf != NULL);

    buf->size = 0;
    buf->totalsize = size;
    buf->data = (char *) g_usb_input_buf;

    ASSERT (buf->data != NULL);
    {
        int i = 0;
        for (i = 0; i < USBTTY_BUFFER_SIZE; i++)
            buf->data[i] = 0x00;
    }

    buf->top = buf->data;
    buf->tail = buf->data;
    buf->end = &(buf->data[size]);

    return 1;
}

int buf_output_init (circbuf_t * buf, unsigned int size)
{
    ASSERT (buf != NULL);

    buf->size = 0;
    buf->totalsize = size;
    buf->data = (char *) g_usb_output_buf;

    ASSERT (buf->data != NULL);

    {
        int i = 0;
        for (i = 0; i < USBTTY_BUFFER_SIZE; i++)
            buf->data[i] = 0x00;
    }

    buf->top = buf->data;
    buf->tail = buf->data;
    buf->end = &(buf->data[size]);

    return 1;
}

int buf_pop (circbuf_t * buf, char *dest, unsigned int len)
{
    unsigned int i;
    char *p = buf->top;
    char *end = buf->end;
    char *data = buf->data;
    char *q = dest;
    //u32 dma_con = 0;

    if (len == 0)
        return 0;

    /* Cap to number of bytes in buffer */
    if (len > buf->size)
        len = buf->size;

#if 0
    /* dma setting */
    __raw_writel (p, DMA_SRC (USB_FULL_DMA1_BASE));     /* SOURCE */
    __raw_writel (dest, DMA_DST (USB_FULL_DMA1_BASE));  /* DESTINATION */
    __raw_writel (end - p, DMA_WPPT (USB_FULL_DMA1_BASE));      /* wrapping point */
    __raw_writel (data, DMA_WPTO (USB_FULL_DMA1_BASE)); /* wrapping destination */

    __raw_writel (len, DMA_COUNT (USB_FULL_DMA1_BASE));
    dma_con = DMA_CON_WPEN | DMA_CON_BURST_16BEAT | DMA_CON_SINC
        | DMA_CON_DINC | DMA_CON_SIZE_BYTE;
    __raw_writel (dma_con, DMA_CON (USB_FULL_DMA1_BASE));

    __raw_writel (DMA_START_BIT, DMA_START (USB_FULL_DMA1_BASE));
    //printf("USB DMA Start!\n");
    while (__raw_readl (DMA_GLBSTA_L) & DMA_GLBSTA_RUN (1));
    //printf("USB DMA Complete\n");
    __raw_writel (DMA_STOP_BIT, DMA_START (USB_FULL_DMA1_BASE));
    __raw_writel (DMA_ACKINT_BIT, DMA_ACKINT (USB_FULL_DMA1_BASE));

    p += len;
    if (p >= end)
        p = data + (p - end);
#else
    for (i = 0; i < len; i++)
    {
        *q = *p;
        p++;
        if (p == end)
            p = data;
        q++;
    }
#endif

    /* Update 'top' pointer */
    buf->top = p;
    buf->size -= len;

    return len;
}

int buf_push (circbuf_t * buf, const char *src, unsigned int len)
{
    /* NOTE:  this function allows push to overwrite old data. */
    unsigned int i;
    //u32 dma_con = 0;
    char *p = buf->tail;
    char *end = buf->end;
    char *data = buf->data;
    char *q = (char *)src;
#if 0
    /* dma setting */
    __raw_writel (src, DMA_SRC (USB_FULL_DMA0_BASE));   /* source */
    __raw_writel (p, DMA_DST (USB_FULL_DMA0_BASE));     /* destination */
    __raw_writel (end - p, DMA_WPPT (USB_FULL_DMA0_BASE));      /* wrapping point */
    __raw_writel (data, DMA_WPTO (USB_FULL_DMA0_BASE)); /* wrapping destination */


    __raw_writel (len, DMA_COUNT (USB_FULL_DMA0_BASE));
    dma_con =
        DMA_CON_WPEN | DMA_CON_WPSD | DMA_CON_BURST_16BEAT | DMA_CON_SINC |
        DMA_CON_DINC | DMA_CON_SIZE_BYTE;

    __raw_writel (dma_con, DMA_CON (USB_FULL_DMA0_BASE));

    __raw_writel (DMA_START_BIT, DMA_START (USB_FULL_DMA0_BASE));
    //printf("USB DMA Start!\n");
    while (__raw_readl (DMA_GLBSTA_L) & DMA_GLBSTA_RUN (0));
    //printf("USB DMA Complete\n");
    __raw_writel (DMA_STOP_BIT, DMA_START (USB_FULL_DMA0_BASE));
    __raw_writel (DMA_ACKINT_BIT, DMA_ACKINT (USB_FULL_DMA0_BASE));

    p += len;
    if (p >= end)
    {
        p = data + (p - end);
    }
#else
    for (i = 0; i < len; i++)
    {
        *p = *q;
        p++;
        if (p == end)
            p = data;
        q++;
    }
#endif

    buf->size += len;

    /* Update 'tail' pointer */
    buf->tail = p;

    return len;
}
