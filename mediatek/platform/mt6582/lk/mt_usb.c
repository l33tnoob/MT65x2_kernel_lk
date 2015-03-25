/*
 * Copyright (c) 2012 MediaTek Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in
 *	the documentation and/or other materials provided with the
 *	distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <debug.h>
#include <reg.h>
#include <platform/bitops.h>
#include <platform/mt_irq.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_usb.h>
#include <platform/mt_typedefs.h>
#include <platform/timer.h>
#include <kernel/thread.h>
#include <platform/sec_devinfo.h>
#include <dev/udc.h>

#ifdef MACH_FPGA
#define CFG_FPGA_PLATFORM		(1)
#else
#define DBG_PHY_CALIBRATION 1
#endif

#define USB_DOUBLE_BUF

#define USB_GINTR

#ifdef USB_DEBUG
/* DEBUG INFO Sections */
#define DBG_USB_DUMP_DESC 0
#define DBG_USB_DUMP_DATA 0
#define DBG_USB_DUMP_SETUP 1
#define DBG_USB_FIFO 0
#define DBG_USB_GENERAL 1
#define DBG_PHY_CALIBRATION 0
#endif

#define DBG_C(x...) dprintf(CRITICAL, x)
#define DBG_I(x...) dprintf(INFO, x)
#define DBG_S(x...) dprintf(SPEW, x)

#if DBG_USB_GENERAL
#define DBG_IRQ(x...) dprintf(INFO, x)
#else
#define DBG_IRQ(x...) do{} while(0)
#endif

/* bits used in all the endpoint status registers */
#define EPT_TX(n) (1 << ((n) + 16))
#define EPT_RX(n) (1 << (n))

/* udc.h wrapper for usbdcore */

static unsigned char usb_config_value = 0;
EP0_STATE ep0_state = EP0_IDLE;
int set_address = 0;
u32 fifo_addr = FIFO_ADDR_START;

#define EP0	0

/* USB transfer directions */
#define USB_DIR_IN	DEVICE_WRITE	/* val: 0x80 */
#define USB_DIR_OUT	DEVICE_READ	/* val: 0x00 */

#define EP0_MAX_PACKET_SIZE	64

/* Request types */
#define USB_TYPE_STANDARD	(0x00 << 5)
#define USB_TYPE_CLASS		(0x01 << 5)
#define USB_TYPE_VENDOR		(0x02 << 5)
#define USB_TYPE_RESERVED	(0x03 << 5)

/* values used in GET_STATUS requests */
#define USB_STAT_SELFPOWERED	0x01

/* USB recipients */
#define USB_RECIP_DEVICE	0x00
#define USB_RECIP_INTERFACE	0x01
#define USB_RECIP_ENDPOINT	0x02
#define USB_RECIP_OTHER		0x03

/* Endpoints */
#define USB_EP_NUM_MASK	0x0f		/* in bEndpointAddress */
#define USB_EP_DIR_MASK	0x80

#define USB_TYPE_MASK	0x60
#define USB_RECIP_MASK	0x1f

#if CFG_FPGA_PLATFORM
/* for usb phy */
#include <platform/mt_i2c.h>
#endif

#define URB_BUF_SIZE 512

struct urb {
	struct udc_endpoint *endpoint;
	struct udc_device *device;
	struct setup_packet device_request;

	u8 *buffer;
	unsigned int actual_length;
};

static struct udc_endpoint *ep0in, *ep0out;
static struct udc_request *ep0req;
static unsigned char ep0_buf[4096] __attribute__((aligned(32)));
struct urb mt_ep0_urb;
struct urb mt_tx_urb;
struct urb mt_rx_urb;
struct urb *ep0_urb = &mt_ep0_urb;
struct urb *tx_urb = &mt_tx_urb;
struct urb *rx_urb = &mt_rx_urb;

/* endpoint data - mt_ep */
struct udc_endpoint {
	/* rx side */
	struct urb *rcv_urb;	/* active urb */

	/* tx side */
	struct urb *tx_urb;	/* active urb */

	/* info from hsusb */
	struct udc_request *req;
	unsigned int bit;	/* EPT_TX/EPT_RX */
	unsigned char num;
	unsigned char in;
	unsigned short maxpkt;
	int status;	/* status for error handling */

	unsigned int sent;		/* data already sent */
	unsigned int last;		/* data sent in last packet XXX do we need this */
	unsigned char mode;	/* double buffer */
};

/* from mt_usbtty.h */
#define NUM_ENDPOINTS	3

/* origin endpoint_array */
struct udc_endpoint ep_list[NUM_ENDPOINTS + 1];	/* one extra for control endpoint */

static int usb_online = 0;

static u8 dev_address = 0;

static struct udc_device *the_device;
static struct udc_gadget *the_gadget;
/* end from hsusb.c */

/* declare ept_complete handle */
static void handle_ept_complete(struct udc_endpoint *ept);

/* usb phy bring up */
#if CFG_FPGA_PLATFORM
#define USB_I2C_ID	I2C2	/* 0 - 6 */
#define PATH_NORMAL	0
#define PATH_PMIC	1

U32 usb_i2c_read8 (U8 addr, U8 *dataBuffer)
{
	U32 ret_code = I2C_OK;
//	U8 write_data = addr;
	U16 len;

	static struct mt_i2c_t i2c;

	i2c.id = USB_I2C_ID;
	i2c.addr = 0x60;	/* i2c API will shift-left this value to 0xc0 */
	i2c.mode = ST_MODE;
	i2c.speed = 100;
	i2c.dma_en = 0;
	len = 1;

	/* set register command */
#if 0
	ret_code = i2c_write(&i2c, &write_data, len);

	if (ret_code != len)
		return ret_code;

	ret_code = i2c_read(&i2c, dataBuffer, len);
#else
	ret_code = i2c_write_read(&i2c, dataBuffer, len, len);
#endif

	return ret_code;
}

U32 usb_i2c_write8(U8 addr, U8 value)
{
	U32 ret_code = I2C_OK;
	U8 write_data[2];
	U16 len;

	static struct mt_i2c_t i2c;

	write_data[0]= addr;
	write_data[1] = value;

	i2c.id = USB_I2C_ID;
	i2c.addr = 0x60;	/* i2c API will shift-left this value to 0xc0 */
	i2c.mode = ST_MODE;
	i2c.speed = 100;
	i2c.dma_en = 0;
	len = 2;

	ret_code = i2c_write(&i2c, write_data, len);

	return ret_code;
}

#define USBPHY_I2C_READ8(addr, buffer)	usb_i2c_read8(addr, buffer)
#define USBPHY_I2C_WRITE8(addr, value)	usb_i2c_write8(addr, value)
#endif

/* use mt_typedefs.h */
#define USBPHY_READ8(offset)		readb(USB20_PHY_BASE+offset)
#define USBPHY_WRITE8(offset, value)	writeb(value, USB20_PHY_BASE+offset)
#define USBPHY_SET8(offset, mask)	USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)	USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~mask))

#define USB11PHY_READ8(offset)		readb(USB11_PHY_BASE+offset)
#define USB11PHY_WRITE8(offset, value)  writeb(value, USB11_PHY_BASE+offset)
#define USB11PHY_SET8(offset, mask)	USB11PHY_WRITE8(offset, (USB11PHY_READ8(offset)) | (mask))
#define USB11PHY_CLR8(offset, mask)	USB11PHY_WRITE8(offset, (USB11PHY_READ8(offset)) & (~mask))

#if CFG_FPGA_PLATFORM
void mt_usb_phy_poweron(void)
{
	#define PHY_DRIVING   0x3

#if defined(USB_PHY_DRIVING_TUNING)
	UINT8 usbreg8;
	unsigned int i;
#endif

	/* force_suspendm = 0 */
	USBPHY_CLR8(0x6a, 0x04);

	USBPHY_I2C_WRITE8(0xff, 0x00);
	USBPHY_I2C_WRITE8(0x61, 0x04);
	USBPHY_I2C_WRITE8(0x68, 0x00);
	USBPHY_I2C_WRITE8(0x6a, 0x00);
	USBPHY_I2C_WRITE8(0x00, 0x6e);
	USBPHY_I2C_WRITE8(0x1b, 0x0c);
	USBPHY_I2C_WRITE8(0x08, 0x44);
	USBPHY_I2C_WRITE8(0x11, 0x55);
	USBPHY_I2C_WRITE8(0x1a, 0x68);

	#if defined(USB_PHY_DRIVING_TUNING)
	/* driving tuning */
	USBPHY_I2C_READ8(0xab, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xab, usbreg8);

	for(i = 0; i < 16; i++)
	{
		USBPHY_I2C_READ8((0x92+i), &usbreg8);
		usbreg8 &= ~0x3;
		usbreg8 |= PHY_DRIVING;
		USBPHY_I2C_WRITE8((0x92+i), usbreg8);
	}

	USBPHY_I2C_READ8(0xbc, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xbc, usbreg8);

	USBPHY_I2C_READ8(0xbe, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xbe, usbreg8);

	USBPHY_I2C_READ8(0xbf, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xbf, usbreg8);

	USBPHY_I2C_READ8(0xcd, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xcd, usbreg8);

	USBPHY_I2C_READ8(0xf1, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xf1, usbreg8);

	USBPHY_I2C_READ8(0xa7, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xa7, usbreg8);

	USBPHY_I2C_READ8(0xa8, &usbreg8);
	usbreg8 &= ~0x3;
	usbreg8 |= PHY_DRIVING;
	USBPHY_I2C_WRITE8(0xa8, usbreg8);
	#endif

	udelay(800);

}

void mt_usb_phy_savecurrent (void)
{
	/* no need */
}

void mt_usb_phy_recover (void)
{
	/* no need */
}

void mt_usb11_phy_savecurrent(void)
{
	/* no need */
}

void Charger_Detect_Init(void)
{
	/* no need */
}

void Charger_Detect_Release(void)
{
	/* no need */
}

#else

void mt_usb_phy_poweron(void)
{
	/*
	 * swtich to USB function.
	 * (system register, force ip into usb mode).
	 */
	USBPHY_CLR8(0x6b, 0x04);
	USBPHY_CLR8(0x6e, 0x01);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);

	/* RG_USB20_DP_100K_EN = 1'b0 */
	/* RG_USB20_DP_100K_EN = 1'b0 */
	USBPHY_CLR8(0x22, 0x03);

	/* release force suspendm */
	USBPHY_CLR8(0x6a, 0x04);

	udelay(800);

	/* force enter device mode */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2E);
	USBPHY_SET8(0x6d, 0x3E);

	return;
}

void mt_usb_phy_savecurrent(void)
{
	/*
	 * swtich to USB function.
	 * (system register, force ip into usb mode).
	 */
	USBPHY_CLR8(0x6b, 0x04);
	USBPHY_CLR8(0x6e, 0x01);

	/* release force suspendm */
	USBPHY_CLR8(0x6a, 0x04);
	/* RG_DPPULLDOWN./RG_DMPULLDOWN. */
	USBPHY_SET8(0x68, 0xc0);
	/* RG_XCVRSEL[1:0] = 2'b01 */
	USBPHY_CLR8(0x68, 0x30);
	USBPHY_SET8(0x68, 0x10);
	/* RG_TERMSEL = 1'b1 */
	USBPHY_SET8(0x68, 0x04);
	/* RG_DATAIN[3:0] = 4'b0000 */
	USBPHY_CLR8(0x69, 0x3c);

	/*
	 * force_dp_pulldown, force_dm_pulldown,
	 * force_xcversel, force_termsel.
	 */
	USBPHY_SET8(0x6a, 0xba);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	/* RG_USB20_OTG_VBUSSCMP_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x10);

	udelay(800);

	/* rg_usb20_pll_stable = 1 */
	USBPHY_SET8(0x63, 0x02);

	udelay(1);

	/* force suspendm = 1 */
	USBPHY_SET8(0x6a, 0x04);

	udelay(1);

	return;
}

void mt_usb_phy_recover(void)
{

	/* clean PUPD_BIST_EN */
	/* PUPD_BIST_EN = 1'b0 */
	/* PMIC will use it to detect charger type */
	USBPHY_CLR8(0x1d, 0x10);

	/* force_uart_en = 1'b0 */
	USBPHY_CLR8(0x6b, 0x04);
	/* RG_UART_EN = 1'b0 */
	USBPHY_CLR8(0x6e, 0x01);
	/* force_uart_en = 1'b0 */
	USBPHY_CLR8(0x6a, 0x04);

	USBPHY_CLR8(0x68, 0xf4);

	/* RG_DATAIN[3:0] = 4'b0000 */
	USBPHY_CLR8(0x69, 0x3c);

	USBPHY_CLR8(0x6a, 0xba);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	/* RG_USB20_OTG_VBUSSCMP_EN = 1'b1 */
	USBPHY_SET8(0x1a, 0x10);

	udelay(800);

	/* force enter device mode */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2E);
	USBPHY_SET8(0x6d, 0x3E);
}

void mt_usb11_phy_savecurrent(void)
{
#if 0
	USB11PHY_SET8(0xca, 0x10);
	USB11PHY_SET8(0xcb, 0x3c);
	USB11PHY_CLR8(0xc1, 0x08);
	USB11PHY_CLR8(0xc7, 0x06);
	USB11PHY_SET8(0xc6, 0x06);

#endif
}

void Charger_Detect_Init(void)
{
	/* RG_USB20_BC11_SW_EN = 1'b1 */
	USBPHY_SET8(0x1a, 0x80);
}

void Charger_Detect_Release(void)
{
	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
}
#endif
/* usb phy bring up end */

#if !CFG_FPGA_PLATFORM
//ALPS00427972, implement the analog register formula
void mt_usb_phy_calibraion (int case_set, int input_reg)
{
#if 0
	int temp_added=0;
	int temp_test=0;
	int temp_mask;

#if DBG_PHY_CALIBRATION
	DBG_I("%s: case_set %d, input_reg = 0x%x \n", __func__, case_set, input_reg);
#endif

	switch(case_set)
	{
	case 1:
		//case  1
		//If M_HW_RES3[15:13] !=0
		//RG_USB20_TERM_VREF_SEL[2:0] <= RG_USB20_TERM_VREF_SEL[2:0] + M_HW_RES3[15:13]
		temp_mask = 0x07;
		temp_test = USBPHY_READ8(0x05);
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_test = 0x%x \n", __func__, temp_test);
#endif
		temp_added = (USBPHY_READ8(0x05)& temp_mask) + input_reg;
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_added = 0x%x \n", __func__, temp_added);
#endif
		temp_added &= 0x07;
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_added = 0x%x \n", __func__, temp_added);
#endif

		USBPHY_CLR8(0x05, temp_mask);
		USBPHY_SET8(0x05, temp_added);

		temp_test = USBPHY_READ8(0x05);
#if DBG_PHY_CALIBRATION
		DBG_I("%s: final temp_test = 0x%x \n", __func__, temp_test);
#endif
		break;
	case 2:
		//case 2
		//If M_HW_RES3[12:10] !=0
		//RG_USB20_CLKREF_REF[2:0]<= RG_USB20_CLKREF_REF[2:0]+ M_HW_RES3[12:10]
		temp_mask = 0x07;

		temp_test = USBPHY_READ8(0x07);
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_test = 0x%x \n", __func__, temp_test);
#endif
		temp_added = (USBPHY_READ8(0x07)& temp_mask) + input_reg;
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_added = 0x%x \n", __func__, temp_added);
#endif
		temp_added &= 0x07;
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_added = 0x%x \n", __func__, temp_added);
#endif

		USBPHY_CLR8(0x07, temp_mask);
		USBPHY_SET8(0x07, temp_added);

		temp_test = USBPHY_READ8(0x07);
#if DBG_PHY_CALIBRATION
		DBG_I("%s: final temp_test = 0x%x \n", __func__, temp_test);
#endif
		break;
	case 3:
		//case 3
		//If M_HW_RES3[9:7] !=0
			//RG_USB20_VRT_VREF_SEL[2:0]<=RG_USB20_VRT_VREF_SEL[2:0]+ M_HW_RES3[9:7]
		temp_mask = 0x70;

		temp_test = USBPHY_READ8(0x05);
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_test = 0x%x \n", __func__, temp_test);
#endif
		temp_added = (USBPHY_READ8(0x05)& temp_mask) >> 4;
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_added = 0x%x \n", __func__, temp_added);
#endif
		temp_added += input_reg;
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_added = 0x%x \n", __func__, temp_added);
#endif
		temp_added &= 0x07;
#if DBG_PHY_CALIBRATION
		DBG_I("%s: temp_added = 0x%x \n", __func__, temp_added);
#endif

		USBPHY_CLR8(0x05, temp_mask);
		USBPHY_SET8(0x05, temp_added<<4);

		temp_test = USBPHY_READ8(0x05);
#if DBG_PHY_CALIBRATION
		DBG_I("%s: final temp_test = 0x%x \n", __func__, temp_test);
#endif
		break;
	}
#endif
}

//ALPS00427972, implement the analog register formula
#endif

void board_usb_init(void)
{
	mt_usb_phy_poweron();
}

struct udc_descriptor {
	struct udc_descriptor *next;
	unsigned short tag;	/* ((TYPE << 8) | NUM) */
	unsigned short len;	/* total length */
	unsigned char data[0];
};

#if DBG_USB_DUMP_SETUP
static void dump_setup_packet(char *str, struct setup_packet *sp) {
	DBG_I("\n");
	DBG_I(str);
	DBG_I("	   bmRequestType = %x\n", sp->type);
	DBG_I("	   bRequest = %x\n", sp->request);
	DBG_I("	   wValue = %x\n", sp->value);
	DBG_I("	   wIndex = %x\n", sp->index);
	DBG_I("	   wLength = %x\n", sp->length);
}
#else
static void dump_setup_packet(char *str, struct setup_packet *sp) {}
#endif

static void copy_desc(struct urb *urb, void *data, int length) {

#if DBG_USB_FIFO
	DBG_I("%s: urb: %x, data %x, length: %d, actual_length: %d\n",
		__func__, urb->buffer, data, length, urb->actual_length);
#endif

	//memcpy(urb->buffer + urb->actual_length, data, length);
	memcpy(urb->buffer, data, length);
	//urb->actual_length += length;
	urb->actual_length = length;
#if DBG_USB_FIFO
	DBG_I("%s: urb: %x, data %x, length: %d, actual_length: %d\n",
		__func__, urb, data, length, urb->actual_length);
#endif
}


struct udc_descriptor *udc_descriptor_alloc(unsigned type, unsigned num,
						unsigned len)
{
	struct udc_descriptor *desc;
	if ((len > 255) || (len < 2) || (num > 255) || (type > 255))
		return 0;

	if (!(desc = malloc(sizeof(struct udc_descriptor) + len)))
		return 0;

	desc->next = 0;
	desc->tag = (type << 8) | num;
	desc->len = len;
	desc->data[0] = len;
	desc->data[1] = type;

	return desc;
}

static struct udc_descriptor *desc_list = 0;
static unsigned next_string_id = 1;

void udc_descriptor_register(struct udc_descriptor *desc) {
	desc->next = desc_list;
	desc_list = desc;
}

unsigned udc_string_desc_alloc(const char *str)
{
	unsigned len;
	struct udc_descriptor *desc;
	unsigned char *data;

	if (next_string_id > 255)
		return 0;

	if (!str)
		return 0;

	len = strlen(str);
	desc = udc_descriptor_alloc(TYPE_STRING, next_string_id, len * 2 + 2);
	if (!desc)
		return 0;
	next_string_id++;

	/* expand ascii string to utf16 */
	data = desc->data + 2;
	while (len-- > 0) {
		*data++ = *str++;
		*data++ = 0;
	}

	udc_descriptor_register(desc);
	return desc->tag & 0xff;
}

static int mt_read_fifo(struct udc_endpoint *endpoint) {

	struct urb *urb = endpoint->rcv_urb;
	int len = 0, count = 0;
	int ep_num = endpoint->num;
	int index;
	unsigned char *cp;
	u32 *wp;
#if !CFG_FPGA_PLATFORM
	u16 dma_cntl = 0;
#endif

	if (ep_num == EP0)
		urb = ep0_urb;

	if (urb) {
		index = readb(INDEX);
		writeb(ep_num, INDEX);

		cp = (u8 *) (urb->buffer + urb->actual_length);
		wp = (u32 *) cp;
#if DBG_USB_FIFO
		DBG_I("%s: ep_num: %d, urb: %x, urb->buffer: %x, urb->actual_length = %d\n",
			__func__, ep_num, urb, urb->buffer, urb->actual_length);
#endif

		count = len = readw(IECSR + RXCOUNT);
		if (ep_num != 0) {
#if DBG_USB_FIFO
			DBG_I("%s: ep_num: %d count = %d\n",
			__func__, ep_num, count);
#endif
		}

		/* FIX: DMA has problem write now */

#if !CFG_FPGA_PLATFORM
		arch_clean_invalidate_cache_range((addr_t) cp, count);

		if (ep_num != 0) {
			writel((u32)wp, USB_DMA_ADDR (ep_num));
			writel(count, USB_DMA_COUNT (ep_num));
			dma_cntl =
				USB_DMA_BURST_MODE_3 | (ep_num << USB_DMA_ENDPNT_OFFSET) |
				USB_DMA_EN;
			writew(dma_cntl, USB_DMA_CNTL (ep_num));
			while (readw(USB_DMA_CNTL (ep_num)) & USB_DMA_EN);
		}
		else

#endif
		{
			while (len > 0) {
				if (len >= 4) {
					*wp++ = readl(FIFO(ep_num));
					cp = (unsigned char *) wp;
					//DBG_I("USB READ FIFO: wp = %lu, cp = %lu\n", wp, cp);
					len -= 4;
				} else {
					*cp++ = readb(FIFO(ep_num));
					//DBG_I("USB READ FIFO: wp = %lu, cp = %lu\n", wp, cp);
					len--;
				}
			}
		}

#if DBG_USB_DUMP_DATA
		if (ep_num != 0) {
			DBG_I("%s: &urb->buffer: %x\n", __func__, urb->buffer);
			DBG_I("[USB] dump data:\n");
			hexdump8(urb->buffer, count);
		}
#endif

		urb->actual_length += count;

		writeb(index, INDEX);
	}

	return count;
}

static int mt_write_fifo(struct udc_endpoint *endpoint)
{
	struct urb *urb = endpoint->tx_urb;
	int last = 0, count = 0;
	int ep_num = endpoint->num;
	int index;
	unsigned char *cp = NULL;
#ifdef USB_TX_DMA_MODE_0
	u32 *wp;
	u16 dma_cntl = 0;
#endif

	if (ep_num == EP0)
		urb = ep0_urb;

	if (urb) {
		index = readb(INDEX);
		writeb(ep_num, INDEX);

#if DBG_USB_DUMP_DESC
	DBG_I("%s: dump desc\n", __func__);
	hexdump8(urb->buffer, urb->actual_length);
#endif


#if DBG_USB_FIFO
	DBG_I("%s: ep_num: %d urb: %x, actual_length: %d\n",
		__func__, ep_num, urb, urb->actual_length);
	DBG_I("%s: sent: %d, tx_pkt_size: %d\n", __func__, endpoint->sent, endpoint->maxpkt);
#endif

		count = last = MIN (urb->actual_length - endpoint->sent,  endpoint->maxpkt);
		//count = last = urb->actual_length;

#if DBG_USB_FIFO
		DBG_I("%s: count: %d\n", __func__, count);
		DBG_I("%s: urb->actual_length = %d\n", __func__, urb->actual_length);
		DBG_I("%s: endpoint->sent = %d\n", __func__, endpoint->sent);
#endif

		if (count < 0) {
			DBG_C("%s: something is wrong, count < 0", __func__);
		}

		if (count) {
			cp = urb->buffer + endpoint->sent;
#ifdef USB_TX_DMA_MODE_0
			wp = (u32 *)cp;

			arch_clean_invalidate_cache_range((addr_t) cp, count);

			if (ep_num != 0) {
				writel(wp, USB_DMA_ADDR(ep_num));
				writel(count, USB_DMA_COUNT(ep_num));
				dma_cntl =
					USB_DMA_BURST_MODE_3 | (ep_num << USB_DMA_ENDPNT_OFFSET) |
					USB_DMA_EN | USB_DMA_DIR;
				writew(dma_cntl, USB_DMA_CNTL(ep_num));
				while (readw(USB_DMA_CNTL (ep_num)) & USB_DMA_EN);
			} else
#endif
			{
			//DBG("---------write USB fifo---------\n");
				while (count > 0) {
					//hexdump8(cp, 1);
					writeb(*cp, FIFO (ep_num));
					cp++;
					count--;
				}
			}
		}

		endpoint->last = last;
		endpoint->sent += last;

		writeb(index, INDEX);
	}

	return last;
}

static struct udc_endpoint * mt_find_ep(int ep_num, u8 dir) {
	int i;
	u8 in = 0;

	/* convert dir to in */
	if (dir == USB_DIR_IN) /* dir == USB_DIR_IN */
		in = 1;

	/* for (i = 0; i < udc_device->max_endpoints; i++) */
	/* for (i = 0; i < the_gadget->ifc_endpoints; i++) */
	for (i = 0; i < MT_EP_NUM; i++) {
		if ((ep_list[i].num == ep_num) && (ep_list[i].in == in)) {
#if DBG_USB_GENERAL
			DBG_I("%s: find ep!\n", __func__);
#endif
			return &ep_list[i];
		}
	}
	return NULL;
}

static void mt_udc_flush_fifo(u8 ep_num, u8 dir)
{
	u16 tmpReg16;
	u8 index;
	struct udc_endpoint *endpoint;

	index = readb(INDEX);
	writeb(ep_num, INDEX);

	if (ep_num == 0) {
		tmpReg16 = readw(IECSR + CSR0);
		tmpReg16 |= EP0_FLUSH_FIFO;
		writew(tmpReg16, IECSR + CSR0);
		writew(tmpReg16, IECSR + CSR0);
	} else {
		endpoint = mt_find_ep(ep_num, dir);
		if (endpoint->in == 0) { /* USB_DIR_OUT */
			tmpReg16 = readw(IECSR + RXCSR);
			tmpReg16 |= EPX_RX_FLUSHFIFO;
			writew(tmpReg16, IECSR + RXCSR);
			writew(tmpReg16, IECSR + RXCSR);
		} else {
			tmpReg16 = readw(IECSR + TXCSR);
			tmpReg16 |= EPX_TX_FLUSHFIFO;
			writew(tmpReg16, IECSR + TXCSR);
			writew(tmpReg16, IECSR + TXCSR);
		}
	}

	/* recover index register */
	writeb(index, INDEX);
}

/* the endpoint does not support the received command, stall it!! */
static void udc_stall_ep(unsigned int ep_num, u8 dir) {
	struct udc_endpoint *endpoint = mt_find_ep(ep_num, dir);
	u8 index;
	u16 csr;

	DBG_C("[USB] %s\n", __func__);

	index = readb(INDEX);
	writeb(ep_num, INDEX);

	if (ep_num == 0) {
		csr = readw(IECSR + CSR0);
		csr |= EP0_SENDSTALL;
		writew(csr, IECSR + CSR0);
		mt_udc_flush_fifo(ep_num, USB_DIR_OUT);
	} else {
		if (endpoint->in == 0) { /* USB_DIR_OUT */
			csr = readb(IECSR + RXCSR);
			csr |= EPX_RX_SENDSTALL;
			writew(csr, IECSR + RXCSR);
			mt_udc_flush_fifo(ep_num, USB_DIR_OUT);
		} else {
			csr = readb(IECSR + TXCSR);
			csr |= EPX_TX_SENDSTALL;
			writew(csr, IECSR + TXCSR);
			mt_udc_flush_fifo(ep_num, USB_DIR_IN);
		}
	}
	//mt_udc_flush_fifo (ep_num, USB_DIR_OUT);
	//mt_udc_flush_fifo (ep_num, USB_DIR_IN);

	ep0_state = EP0_IDLE;

	writeb(index, INDEX);

	return;
}

/*
 * If abnormal DATA transfer happened, like USB unplugged,
 * we cannot fix this after mt_udc_reset().
 * Because sometimes there will come reset twice.
 */
static void mt_udc_suspend(void) {
	/* handle abnormal DATA transfer if we had any */
	struct udc_endpoint *endpoint;
	int i;

	/* deal with flags */
	usb_online = 0;
	usb_config_value = 0;
	the_gadget->notify(the_gadget, UDC_EVENT_OFFLINE);

	/* error out any pending reqs */
	for (i = 1; i < MT_EP_NUM; i++) {
		/* ensure that ept_complete considers
		 * this to be an error state
		 */
#if DBG_USB_GENERAL
		DBG_I("%s: ep: %i, in: %s, req: %x\n",
			__func__, ep_list[i].num, ep_list[i].in ? "IN" : "OUT", ep_list[i].req);
#endif
		if ((ep_list[i].req && (ep_list[i].in == 0)) || /* USB_DIR_OUT */
			(ep_list[i].req && (ep_list[i].in == 1))) { /* USB_DIR_IN */
			ep_list[i].status = -1;	/* HALT */
			endpoint = &ep_list[i];
			handle_ept_complete(endpoint);
		}
	}
}

static void mt_udc_rxtxmap_recover(void) {
	int i;

	for (i = 1; i < MT_EP_NUM; i++) {
		if (ep_list[i].num != 0) { /* allocated */

			writeb(ep_list[i].num, INDEX);

			if (ep_list[i].in == 0) /* USB_DIR_OUT */
				writel(ep_list[i].maxpkt, (IECSR + RXMAP));
			else
				writel(ep_list[i].maxpkt, (IECSR + TXMAP));
		}
	}
}

static void mt_udc_reset(void) {

	/* MUSBHDRC automatically does the following when reset signal is detected */
	/* 1. Sets FAddr to 0
	 * 2. Sets Index to 0
	 * 3. Flush all endpoint FIFOs
	 * 4. Clears all control/status registers
	 * 5. Enables all endpoint interrupts
	 * 6. Generates a Rest interrupt
	 */

	DBG_I("[USB] %s\n", __func__);

	/* disable all endpoint interrupts */
	writeb(0, INTRTXE);
	writeb(0, INTRRXE);
	writeb(0, INTRUSBE);

	writew(SWRST_SWRST | SWRST_DISUSBRESET, SWRST);

	dev_address = 0;

	/* flush FIFO */
	mt_udc_flush_fifo(0, USB_DIR_OUT);
	mt_udc_flush_fifo(1, USB_DIR_OUT);
	mt_udc_flush_fifo(1, USB_DIR_IN);
	//mt_udc_flush_fifo (2, USB_DIR_IN);

	/* detect USB speed */
	if (readb(POWER) & PWR_HS_MODE) {
		DBG_I("[USB] USB High Speed\n");
//		enable_highspeed();
	} else {
		DBG_I("[USB] USB Full Speed\n");
	}

	/* restore RXMAP and TXMAP if the endpoint has been configured */
	mt_udc_rxtxmap_recover();

	/* enable suspend */
	writeb((INTRUSB_SUSPEND | INTRUSB_RESUME | INTRUSB_RESET |INTRUSB_DISCON), INTRUSBE);

}

static void mt_udc_ep0_write(void) {

	struct udc_endpoint *endpoint = &ep_list[EP0];
	int count = 0;
	u16 csr0 = 0;
	u8 index = 0;

	index = readb(INDEX);
	writeb(0, INDEX);

	csr0 = readw(IECSR + CSR0);
	if (csr0 & EP0_TXPKTRDY) {
		DBG_I("mt_udc_ep0_write: ep0 is not ready to be written\n");
		return;
	}

	count = mt_write_fifo(endpoint);

#if DBG_USB_GENERAL
	DBG_I("%s: count = %d\n", __func__, count);
#endif

	if (count < EP0_MAX_PACKET_SIZE) {
		/* last packet */
		csr0 |= (EP0_TXPKTRDY | EP0_DATAEND);
		ep0_urb->actual_length = 0;
		endpoint->sent = 0;
		ep0_state = EP0_IDLE;
	} else {
		/* more packets are waiting to be transferred */
		csr0 |= EP0_TXPKTRDY;
	}

	writew(csr0, IECSR + CSR0);
	writeb(index, INDEX);

	return;
}

static void mt_udc_ep0_read(void) {

	struct udc_endpoint *endpoint = &ep_list[EP0];
	int count = 0;
	u16 csr0 = 0;
	u8 index = 0;

	index = readb(INDEX);
	writeb(EP0, INDEX);

	csr0 = readw(IECSR + CSR0);

	/* erroneous ep0 interrupt */
	if (!(csr0 & EP0_RXPKTRDY)) {
		return;
	}

	count = mt_read_fifo(endpoint);

	if (count <= EP0_MAX_PACKET_SIZE) {
		/* last packet */
		csr0 |= (EP0_SERVICED_RXPKTRDY | EP0_DATAEND);
		ep0_state = EP0_IDLE;
	} else {
		/* more packets are waiting to be transferred */
		csr0 |= EP0_SERVICED_RXPKTRDY;
	}

	writew(csr0, IECSR + CSR0);

	writeb(index, INDEX);

	return;
}

static int ep0_standard_setup(struct urb *urb) {
	struct setup_packet *request;
	struct udc_descriptor *desc;
	//struct udc_device *device;
	u8 *cp = urb->buffer;
#if 0
	if (!urb || !urb->device) {
		DBG ("\n!urb || !urb->device\n");
		return FALSE;
	}
#endif

	request = &urb->device_request;
	//device = urb->device;

	dump_setup_packet("[USB] Device Request\n", request);

	if ((request->type & USB_TYPE_MASK) != 0) {
		return FALSE;			/* Class-specific requests are handled elsewhere */
	}

	/* handle all requests that return data (direction bit set on bm RequestType) */
	if ((request->type & USB_EP_DIR_MASK)) {
		/* send the descriptor */
		ep0_state = EP0_TX;

		switch (request->request) {
		/* data stage: from device to host */
		case GET_STATUS:
#if DBG_USB_GENERAL
			DBG_I("GET_STATUS\n");
#endif
			urb->actual_length = 2;
			cp[0] = cp[1] = 0;
			switch (request->type & USB_RECIP_MASK) {
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

		case GET_DESCRIPTOR:
#if DBG_USB_GENERAL
			DBG_I("GET_DESCRIPTOR\n");
#endif
			/* usb_highspeed? */

			for (desc = desc_list; desc; desc = desc->next) {
#if DBG_USB_DUMP_DESC
				DBG_I("desc->tag: %x: request->value: %x\n", desc->tag, request->value);
#endif
				if (desc->tag == request->value) {

#if DBG_USB_DUMP_DESC
					DBG_I("Find packet!\n");
#endif
					unsigned len = desc->len;
					if (len > request->length)
						len = request->length;

#if DBG_USB_GENERAL
					DBG_I("%s: urb: %x, cp: %p\n", __func__, urb, cp);
#endif
					copy_desc(urb, desc->data, len);
					return 0;
				}
			}
			/* descriptor lookup failed */
			return FALSE;

		case GET_CONFIGURATION:
#if DBG_USB_GENERAL
			DBG_I("GET_CONFIGURATION\n");
			DBG_I("USB_EP_DIR_MASK\n");
#endif
#if 0
			urb->actual_length = 1;
			((char *) urb->buffer)[0] = device->configuration;
#endif
//			return 0;
			break;

		case GET_INTERFACE:
#if DBG_USB_GENERAL
			DBG_I("GET_INTERFACE\n");
#endif

#if 0
			urb->actual_length = 1;
			((char *) urb->buffer)[0] = device->alternate;
			return 0;
#endif
		default:
			DBG_C("Unsupported command with TX data stage\n");
			break;
		}
	} else {

		switch (request->request) {

		case SET_ADDRESS:
#if DBG_USB_GENERAL
			DBG_I("SET_ADDRESS\n");
#endif

			dev_address = (request->value);
			set_address = 1;
			return 0;

		case SET_CONFIGURATION:
#if DBG_USB_GENERAL
			DBG_I("SET_CONFIGURATION\n");
#endif
#if 0
			device->configuration = (request->value) & 0x7f;
			device->interface = device->alternate = 0;
#endif
			if (request->value == 1) {
				usb_config_value = 1;
				the_gadget->notify(the_gadget, UDC_EVENT_ONLINE);
			} else {
				usb_config_value = 0;
				the_gadget->notify(the_gadget, UDC_EVENT_OFFLINE);
			}

			usb_online = request->value ? 1 : 0;
			//usb_status(request->value ? 1 : 0, usb_highspeed);

			return 0;

		default:
			DBG_C("Unsupported command with RX data stage\n");
			break;

		}
	}
	return FALSE;
}

static void mt_udc_ep0_setup(void) {
	struct udc_endpoint *endpoint = &ep_list[0];
	u8 index;
	u8 stall = 0;
	u16 csr0;
	struct setup_packet *request;

#ifdef USB_DEBUG
	u16 count;
#endif

	index = readb(INDEX);
	writeb(0, INDEX);
	/* Read control status register for endpiont 0 */
	csr0 = readw(IECSR + CSR0);

	/* check whether RxPktRdy is set? */
	if (!(csr0 & EP0_RXPKTRDY))
		return;

	/* unload fifo */
	ep0_urb->actual_length = 0;

#ifndef USB_DEBUG
	mt_read_fifo(endpoint);
#else
	count = mt_read_fifo(endpoint);

#if DBG_USB_FIFO
	DBG_I("%s: mt_read_fifo count = %d\n", __func__, count);
#endif
#endif
	/* decode command */
	request = &ep0_urb->device_request;
	memcpy(request, ep0_urb->buffer, sizeof(struct setup_packet));

	if (((request->type) & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
#if DBG_USB_GENERAL
		DBG_I("[USB] Standard Request\n");
#endif
		stall = ep0_standard_setup(ep0_urb);
		if (stall) {
			dump_setup_packet("[USB] STANDARD REQUEST NOT SUPPORTED\n", request);
		}
	} else if (((request->type) & USB_TYPE_MASK) == USB_TYPE_CLASS) {
#if DBG_USB_GENERAL
		DBG_I("[USB] Class-Specific Request\n");
#endif
//		stall = ep0_class_setup(ep0_urb);
		if (stall) {
			dump_setup_packet("[USB] CLASS REQUEST NOT SUPPORTED\n", request);
		}
	} else if (((request->type) & USB_TYPE_MASK) == USB_TYPE_VENDOR) {
#if DBG_USB_GENERAL
		DBG_I("[USB] Vendor-Specific Request\n");
		/* do nothing now */
		DBG_I("[USB] ALL VENDOR-SPECIFIC REQUESTS ARE NOT SUPPORTED!!\n");
#endif
	}

	if (stall) {
		/* the received command is not supported */
		udc_stall_ep(0, USB_DIR_OUT);
		return;
	}

	switch (ep0_state) {
	case EP0_TX:
		/* data stage: from device to host */
#if DBG_USB_GENERAL
		DBG_I("%s: EP0_TX\n", __func__);
#endif
		csr0 = readw(IECSR + CSR0);
		csr0 |= (EP0_SERVICED_RXPKTRDY);
		writew(csr0, IECSR + CSR0);

		mt_udc_ep0_write();

		break;
	case EP0_RX:
		/* data stage: from host to device */
#if DBG_USB_GENERAL
		DBG_I("%s: EP0_RX\n", __func__);
#endif
		csr0 = readw(IECSR + CSR0);
		csr0 |= (EP0_SERVICED_RXPKTRDY);
		writew(csr0, IECSR + CSR0);

		break;
	case EP0_IDLE:
		/* no data stage */
#if DBG_USB_GENERAL
		DBG_I("%s: EP0_IDLE\n", __func__);
#endif
		csr0 = readw(IECSR + CSR0);
		csr0 |= (EP0_SERVICED_RXPKTRDY | EP0_DATAEND);

		writew(csr0, IECSR + CSR0);
		writew(csr0, IECSR + CSR0);

		break;
	default:
		break;
	}

	writeb(index, INDEX);
	return;

}

static void mt_udc_ep0_handler(void) {

	u16 csr0;
	u8 index = 0;

	index = readb(INDEX);
	writeb(0, INDEX);

	csr0 = readw(IECSR + CSR0);

	if (csr0 & EP0_SENTSTALL) {
#if DBG_USB_GENERAL
		DBG_I("USB: [EP0] SENTSTALL\n");
#endif
		/* needs implementation for exception handling here */
		ep0_state = EP0_IDLE;
	}

	if (csr0 & EP0_SETUPEND) {
#if DBG_USB_GENERAL
		DBG_I("USB: [EP0] SETUPEND\n");
#endif
		csr0 |= EP0_SERVICE_SETUP_END;
		writew(csr0, IECSR + CSR0);

		ep0_state = EP0_IDLE;
	}

	switch (ep0_state) {
	case EP0_IDLE:
#if DBG_USB_GENERAL
		DBG_I("%s: EP0_IDLE\n", __func__);
#endif
		if (set_address) {
			writeb(dev_address, FADDR);
			set_address = 0;
		}
		mt_udc_ep0_setup();
		break;
	case EP0_TX:
#if DBG_USB_GENERAL
		DBG_I("%s: EP0_TX\n", __func__);
#endif
		mt_udc_ep0_write();
		break;
	case EP0_RX:
#if DBG_USB_GENERAL
		DBG_I("%s: EP0_RX\n", __func__);
#endif
		mt_udc_ep0_read();
		break;
	default:
		break;
	}

	writeb(index, INDEX);

	return;
}


/*
 * udc_setup_ep - setup endpoint
 *
 * Associate a physical endpoint with endpoint_instance and initialize FIFO
 */
void mt_setup_ep(unsigned int ep, struct udc_endpoint *endpoint) {
	u8 index;
	u16 csr;
	u16 csr0;
	u16 max_packet_size;
	u8 fifosz = 0;

	/* EP table records in bits hence bit 1 is ep0 */
	index = readb(INDEX);
	writeb(ep, INDEX);

	if (ep == EP0) {
		/* Read control status register for endpiont 0 */
		csr0 = readw(IECSR + CSR0);

		/* check whether RxPktRdy is set? */
		if (!(csr0 & EP0_RXPKTRDY))
			return;
	}

	/* Configure endpoint fifo */
	/* Set fifo address, fifo size, and fifo max packet size */
#if DBG_USB_GENERAL
	DBG_I("%s: endpoint->in: %d, maxpkt: %d\n",
		__func__, endpoint->in, endpoint->maxpkt);
#endif
	if (endpoint->in == 0) { /* USB_DIR_OUT */
		/* Clear data toggle to 0 */
		csr = readw(IECSR + RXCSR);
		/* pangyen 20090911 */
		csr |= EPX_RX_CLRDATATOG | EPX_RX_FLUSHFIFO;
		writew(csr, IECSR + RXCSR);
		/* Set fifo address */
		writew(fifo_addr >> 3, RXFIFOADD);
		/* Set fifo max packet size */
		max_packet_size = endpoint->maxpkt;
		writew(max_packet_size, IECSR + RXMAP);
		/* Set fifo size (double buffering is currently not enabled) */
		switch (max_packet_size) {
			case 8:
			case 16:
			case 32:
			case 64:
			case 128:
			case 256:
			case 512:
			case 1024:
			case 2048:
				if (endpoint->mode == DOUBLE_BUF)
					fifosz |= FIFOSZ_DPB;
			case 4096:
				fifosz |= uffs(max_packet_size >> 4);
				writeb(fifosz, RXFIFOSZ);
				break;
			case 3072:
				fifosz = uffs(4096 >> 4);
				writeb(fifosz, RXFIFOSZ);
				break;

			default:
				DBG_C("The max_packet_size for ep %d is not supported\n", ep);
		}
	} else {
		/* Clear data toggle to 0 */
		csr = readw(IECSR + TXCSR);
		/* pangyen 20090911 */
		csr |= EPX_TX_CLRDATATOG | EPX_TX_FLUSHFIFO;
		writew(csr, IECSR + TXCSR);
		/* Set fifo address */
		writew(fifo_addr >> 3, TXFIFOADD);
		/* Set fifo max packet size */
		max_packet_size = endpoint->maxpkt;
		writew(max_packet_size, IECSR + TXMAP);
		/* Set fifo size(double buffering is currently not enabled) */
		switch (max_packet_size) {
			case 8:
			case 16:
			case 32:
			case 64:
			case 128:
			case 256:
			case 512:
			case 1024:
			case 2048:
				if (endpoint->mode == DOUBLE_BUF)
					fifosz |= FIFOSZ_DPB;
			case 4096:
				fifosz |= uffs(max_packet_size >> 4);
				writeb(fifosz, TXFIFOSZ);
				break;
			case 3072:
				fifosz = uffs(4096 >> 4);
				writeb(fifosz, TXFIFOSZ);
				break;

			default:
				DBG_C("The max_packet_size for ep %d is not supported\n", ep);
		}
	}

	if (endpoint->mode == DOUBLE_BUF)
		fifo_addr += (max_packet_size << 1);
	else
		fifo_addr += max_packet_size;

	/* recover INDEX register */
	writeb(index, INDEX);
}

struct udc_endpoint *_udc_endpoint_alloc(unsigned char num, unsigned char in,
					 unsigned short max_pkt) {
	int i;

	/*
	 * find an unused slot in ep_list from EP1 to MAX_EP
	 * for example, EP1 will use 2 slot one for IN and the other for OUT
	 */
	if (num != EP0) {
		for (i = 1; i < MT_EP_NUM; i++) {
			if (ep_list[i].num == 0) /* usable */
				break;
		}

		if (i == MT_EP_NUM)	/* ep has been exhausted. */
			return NULL;

		if (in) { /* usb EP1 tx */
			ep_list[i].tx_urb = tx_urb;
#ifdef USB_DOUBLE_BUF
			ep_list[i].mode = DOUBLE_BUF;
#endif
		} else {	/* usb EP1 rx */
			ep_list[i].rcv_urb = rx_urb;
#ifdef USB_DOUBLE_BUF
			ep_list[i].mode = DOUBLE_BUF;
#endif
		}
	} else {
		i = EP0;	/* EP0 */
	}

	ep_list[i].maxpkt = max_pkt;
	ep_list[i].num = num;
	ep_list[i].in = in;
	ep_list[i].req = NULL;

	/* store EPT_TX/RX info */
	if (ep_list[i].in) {
		ep_list[i].bit = EPT_TX(num);
	} else {
		ep_list[i].bit = EPT_RX(num);
	}

	/* write parameters to this ep (write to hardware) */
	mt_setup_ep(num, &ep_list[i]);

	DBG_I("[USB] ept%d %s @%p/%p max=%d bit=%x\n",
		num, in ? "in" : "out", &ep_list[i], &ep_list, max_pkt, ep_list[i].bit);

	return &ep_list[i];
}

#define SETUP(type,request) (((type) << 8) | (request))

static unsigned long ept_alloc_table = EPT_TX(0) | EPT_RX(0);

struct udc_endpoint *udc_endpoint_alloc(unsigned type, unsigned maxpkt)
{
	struct udc_endpoint *ept;
	unsigned n;
	unsigned in;

	if (type == UDC_TYPE_BULK_IN) {
		in = 1;
	} else if (type == UDC_TYPE_BULK_OUT) {
		in = 0;
	} else {
		return 0;
	}

	/* udc_endpoint_alloc is used for EPx except EP0 */
	for (n = 1; n < 16; n++) {
		unsigned long bit = in ? EPT_TX(n) : EPT_RX(n);
		if (ept_alloc_table & bit)
			continue;
		ept = _udc_endpoint_alloc(n, in, maxpkt);
		if (ept)
			ept_alloc_table |= bit;
		return ept;
	}

	return 0;
}

static void handle_ept_complete(struct udc_endpoint *ept)
{
	unsigned int actual;
	int status;
	struct udc_request *req;

	req = ept->req;
	if (req) {
#if DBG_USB_GENERAL
		DBG_I("%s: req: %x: req->length: %d: status: %d\n", __func__, req, req->length, ept->status);
#endif
		/* release this request for processing next */
		ept->req = NULL;

		if (ept->status == -1) {
			actual = 0;
			status = -1;
			DBG_C("%s: EP%d/%s FAIL status: %x\n",
				__func__, ept->num, ept->in ? "in" : "out", status);
		} else {
			actual = req->length;
			status = 0;
		}
		if (req->complete)
			req->complete(req, actual, status);
	}

}

static void mt_udc_epx_handler(u8 ep_num, u8 dir)
{
	u8 index;
	u16 csr;
	u32 count;
	struct udc_endpoint *endpoint;
	struct urb *urb;
	struct udc_request *req;	/* for event signaling */
	u8 intrrxe;

	endpoint = mt_find_ep(ep_num, dir);

	index = readb(INDEX);
	writeb(ep_num, INDEX);

#if DBG_USB_GENERAL
	DBG_I("EP%d Interrupt\n", ep_num);
	DBG_I("dir: %x\n", dir);
#endif

	switch (dir) {
	case USB_DIR_OUT:
		/* transfer direction is from host to device */
		/* from the view of usb device, it's RX */
		csr = readw(IECSR + RXCSR);

		if (csr & EPX_RX_SENTSTALL) {
			DBG_C("EP %d(RX): STALL\n", ep_num);
			/* exception handling: implement this!! */
			return;
		}

		if (!(csr & EPX_RX_RXPKTRDY)) {
#if DBG_USB_GENERAL
			DBG_I("EP %d: ERRONEOUS INTERRUPT\n", ep_num); // normal
#endif
			return;
		}

		//DBG_C("mt_read_fifo, start\n");
		count = mt_read_fifo(endpoint);
		//DBG_C("mt_read_fifo, end\n");

#if DBG_USB_GENERAL
		DBG_I("EP%d(RX), count = %d\n", ep_num, count);
#endif

		csr &= ~EPX_RX_RXPKTRDY;
		writew(csr, IECSR + RXCSR);
		if (readw(IECSR + RXCSR) & EPX_RX_RXPKTRDY) {
#if DBG_USB_GENERAL
			DBG_I("%s: rxpktrdy clear failed\n", __func__);
#endif
		}

		/* do signaling */
		req = endpoint->req;
		/* workaround: if req->lenth == 64 bytes (not huge data transmission)
		 * do normal return */
#if DBG_USB_GENERAL
		DBG_I("%s: req->length: %x, endpoint->rcv_urb->actual_length: %x\n",
			__func__, req->length, endpoint->rcv_urb->actual_length);
#endif

		/* Deal with FASTBOOT command */
		if ((req->length >= endpoint->rcv_urb->actual_length) && req->length == 64) {
			req->length = count;

			/* mask EPx INTRRXE */
			/* The buffer is passed from the AP caller.
			 * It happens that AP is dealing with the buffer filled data by driver,
			 * but the driver is still receiving the next data packets onto the buffer.
			 * Data corrupted happens if the every request use the same buffer.
			 * Mask the EPx to ensure that AP and driver are not accessing the buffer parallely.
			 */
			intrrxe = readb(INTRRXE);
			writeb((intrrxe &= ~(1 << ep_num)), INTRRXE);
		}

		/* Deal with DATA transfer */
		if ((req->length == endpoint->rcv_urb->actual_length) ||
			((req->length >= endpoint->rcv_urb->actual_length) && req->length == 64)) {
			handle_ept_complete(endpoint);

			/* mask EPx INTRRXE */
			/* The buffer is passed from the AP caller.
			 * It happens that AP is dealing with the buffer filled data by driver,
			 * but the driver is still receiving the next data packets onto the buffer.
			 * Data corrupted happens if the every request use the same buffer.
			 * Mask the EPx to ensure that AP and driver are not accessing the buffer parallely.
			 */
			intrrxe = readb(INTRRXE);
			writeb((intrrxe &= ~(1 << ep_num)), INTRRXE);
		}
		break;
	case USB_DIR_IN:
		/* transfer direction is from device to host */
		/* from the view of usb device, it's tx */
		csr = readw(IECSR + TXCSR);

		if (csr & EPX_TX_SENTSTALL) {
			DBG_C("EP %d(TX): STALL\n", ep_num);
			endpoint->status = -1;
			handle_ept_complete(endpoint);
			/* exception handling: implement this!! */
			return;
		}

		if (csr & EPX_TX_TXPKTRDY) {
			DBG_C
				("mt_udc_epx_handler: ep%d is not ready to be written\n",
				ep_num);
			return;
		}

		urb = endpoint->tx_urb;
		if (endpoint->sent == urb->actual_length) {
			/* do signaling */
			handle_ept_complete(endpoint);
			break;
		}

		/* send next packet of the same urb */
		count = mt_write_fifo(endpoint);
#if DBG_USB_GENERAL
		DBG_I("EP%d(TX), count = %d\n", ep_num, endpoint->sent);
#endif

		if (count != 0) {
			/* not the interrupt generated by the last tx packet of the transfer */
			csr |= EPX_TX_TXPKTRDY;
			writew(csr, IECSR + TXCSR);
		}

		break;
	default:
		break;
	}

	writeb(index, INDEX);

	return;
}


void mt_udc_irq(u8 intrtx, u8 intrrx, u8 intrusb) {

	int i;

	DBG_IRQ("[USB] INTERRUPT\n");

	if (intrusb) {
		if (intrusb & INTRUSB_RESUME) {
			DBG_IRQ("[USB] INTRUSB: RESUME\n");
		}

		if (intrusb & INTRUSB_SESS_REQ) {
			DBG_IRQ("[USB] INTRUSB: SESSION REQUEST\n");
		}

		if (intrusb & INTRUSB_VBUS_ERROR) {
			DBG_IRQ("[USB] INTRUSB: VBUS ERROR\n");
		}

		if (intrusb & INTRUSB_SUSPEND) {
			DBG_IRQ("[USB] INTRUSB: SUSPEND\n");
			mt_udc_suspend();
		}

		if (intrusb & INTRUSB_CONN) {
			DBG_IRQ("[USB] INTRUSB: CONNECT\n");
		}

		if (intrusb & INTRUSB_DISCON) {
			DBG_IRQ("[USB] INTRUSB: DISCONNECT\n");
		}

		if (intrusb & INTRUSB_RESET) {
			DBG_IRQ("[USB] INTRUSB: RESET\n");
			mt_udc_reset();
		}

		if (intrusb & INTRUSB_SOF) {
			DBG_IRQ("[USB] INTRUSB: SOF\n");
		}
	}

	/* endpoint 0 interrupt? */
	if (intrtx & EPMASK (0)) {
		mt_udc_ep0_handler();
		intrtx &= ~0x1;
	}

	if (intrtx) {
		for (i = 1; i < MT_EP_NUM; i++) {
			if (intrtx & EPMASK (i)) {
				mt_udc_epx_handler(i, USB_DIR_IN);
			}
		}
	}

	if (intrrx) {
		for (i = 1; i < MT_EP_NUM; i++) {
			if (intrrx & EPMASK (i)) {
				mt_udc_epx_handler(i, USB_DIR_OUT);
			}
		}
	}

}

void service_interrupts(void)
{

	volatile u8 intrtx, intrrx, intrusb;
	/* polling interrupt status for incoming interrupts and service it */
	intrtx = readb(INTRTX) & readb(INTRTXE);
	intrrx = readb(INTRRX) & readb(INTRRXE);
	intrusb = readb(INTRUSB) & readb(INTRUSBE);

	writeb(intrtx, INTRTX);
	writeb(intrrx, INTRRX);
	writeb(intrusb, INTRUSB);

	intrusb &= ~INTRUSB_SOF;

	if (intrtx | intrrx | intrusb) {
		mt_udc_irq(intrtx, intrrx, intrusb);
	}

}

void lk_usb_scheduler(void)
{
	mt_irq_ack(MT_USB0_IRQ_ID);
	service_interrupts();
#if 0
	static enum handler_return ret;
	ret = INT_RESCHEDULE;

	if (ret == INT_RESCHEDULE) {
		thread_preempt();
	}
#endif
	return;
}

int mt_usb_irq_init(void) {
	/* disable all endpoint interrupts */
	writeb(0, INTRTXE);
	writeb(0, INTRRXE);
	writeb(0, INTRUSBE);

	/* 2. Ack all gpt irq if needed */
	//writel(0x3F, GPT_IRQ_ACK);

	/* 3. Register usb irq */
	mt_irq_set_sens(MT_USB0_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
	mt_irq_set_polarity(MT_USB0_IRQ_ID, MT65xx_POLARITY_LOW);

	return 0;
}

/* Turn on the USB connection by enabling the pullup resistor */
void mt_usb_connect_internal(void)
{
	u8 tmpReg8;

	/* connect */
	tmpReg8 = readb(POWER);
	tmpReg8 |= PWR_SOFT_CONN;
	tmpReg8 |= PWR_ENABLE_SUSPENDM;

#ifdef USB_FORCE_FULL_SPEED
	tmpReg8 &= ~PWR_HS_ENAB;
#else
	tmpReg8 |= PWR_HS_ENAB;
#endif
	writeb(tmpReg8, POWER);
}

/* Turn off the USB connection by disabling the pullup resistor */
void mt_usb_disconnect_internal(void)
{
	u8 tmpReg8;

	/* connect */
	tmpReg8 = readb(POWER);
	tmpReg8 &= ~PWR_SOFT_CONN;
	writeb(tmpReg8, POWER);
}

int udc_init(struct udc_device *dev)
{
	struct udc_descriptor *desc = NULL;
#ifdef USB_GINTR
#ifdef USB_HSDMA_ISR
	u32 usb_dmaintr;
#endif
	u32 usb_l1intm;
#endif

	DBG_I("[USB] %s:\n", __func__);

	DBG_I("[USB] ep0_urb: %p\n", ep0_urb);
	/* RESET */
	mt_usb_disconnect_internal();
	mt_usb_connect_internal();
	thread_sleep(20);

#if 0	/* Wait for HW designer's instruction (depends on tapeout process) */
//#if !CFG_FPGA_PLATFORM
//ALPS00427972, implement the analog register formula
	//Set the calibration after power on
	//Add here for eFuse, chip version checking -> analog register calibration
	int input_reg = INREG16(M_HW_RES3);
#if DBG_PHY_CALIBRATION
	DBG_I("%s: input_reg = 0x%x \n", __func__, input_reg);
#endif
	int term_vref 	= (input_reg & RG_USB20_TERM_VREF_SEL_MASK) >> 13;	//0xE000	//0b 1110,0000,0000,0000	15~13
	int clkref 	= (input_reg & RG_USB20_CLKREF_REF_MASK) >> 10;		//0x1C00	//0b 0001,1100,0000,0000	12~10
	int vrt_vref	= (input_reg & RG_USB20_VRT_VREF_SEL_MASK)  >> 7;	//0x0380	//0b 0000,0011,1000,0000	9~7

#if DBG_PHY_CALIBRATION
	DBG_I("%s: term_vref = 0x%x,  clkref = 0x%x, vrt_vref = 0x%x,\n", __func__, term_vref, clkref, vrt_vref);
#endif

	if (term_vref)
		mt_usb_phy_calibraion(1, term_vref);
	if (clkref)
		mt_usb_phy_calibraion(2, clkref);
	if (vrt_vref)
		mt_usb_phy_calibraion(3, vrt_vref);
//ALPS00427972, implement the analog register formula
//#endif
#endif

	if (get_devinfo_with_index(14) & (0x01<<22))
	{
		USBPHY_CLR8(0x00, 0x20);
	}

	if (get_devinfo_with_index(14) & (0x07<<19))
	{
		//RG_USB20_VRT_VREF_SEL[2:0]=5 (ori:4) (0x11110804[14:12])
		USBPHY_CLR8(0x05, 0x70);
		USBPHY_SET8(0x05, ((get_devinfo_with_index(7)>>19)<<4) & (0x70));
	}

	/* usb phy init */
	board_usb_init();
	mt_usb_phy_recover();

	thread_sleep(20);

	/* allocate ep0 */
	ep0out = _udc_endpoint_alloc(EP0, 0, EP0_MAX_PACKET_SIZE);
	ep0in = _udc_endpoint_alloc(EP0, 1, EP0_MAX_PACKET_SIZE);
	ep0req = udc_request_alloc();
	ep0req->buf = malloc(4096);
	ep0_urb->buffer = ep0_buf;

	{
		/* create and register a language table descriptor */
		/* language 0x0409 is US English */
		desc = udc_descriptor_alloc(TYPE_STRING, EP0, 4);
		desc->data[2] = 0x09;
		desc->data[3] = 0x04;
		udc_descriptor_register(desc);
	}
#ifdef USB_HSDMA_ISR
	/* setting HSDMA interrupt register */
	usb_dmaintr = (0xff | 0xff << USB_DMA_INTR_UNMASK_SET_OFFSET);
	writel(usb_dmaintr, USB_DMA_INTR);
#endif

#ifdef USB_GINTR
	usb_l1intm = (TX_INT_STATUS | RX_INT_STATUS | USBCOM_INT_STATUS | DMA_INT_STATUS);
	writel(usb_l1intm, USB_L1INTM);
#endif

	the_device = dev;
	return 0;
}

void udc_endpoint_free(struct udc_endpoint *ept)
{
	/* todo */
}

struct udc_request *udc_request_alloc(void)
{
	struct udc_request *req;
	req = malloc(sizeof(*req));
	req->buf = NULL;
	req->length = 0;
	return req;
}


void udc_request_free(struct udc_request *req)
{
	free(req);
}

/* Called to start packet transmission. */
/* It must be applied in udc_request_queue when polling mode is used.
 * (When USB_GINTR is undefined).
 * If interrupt mode is used, you can use
 * mt_udc_epx_handler(ept->num, USB_DIR_IN); to replace mt_ep_write make ISR
 * do it for you.
 */
static int mt_ep_write(struct udc_endpoint *endpoint)
{
	int ep_num = endpoint->num;
	int count;
	u8 index;
	u16 csr;

	index = readb(INDEX);
	writeb(ep_num, INDEX);

	/* udc_endpoint_write: cannot write ep0 */
	if (ep_num == 0)
		return FALSE;

	 /* udc_endpoint_write: cannot write USB_DIR_OUT */
	if (endpoint->in == 0)
		return FALSE;

	csr = readw(IECSR + TXCSR);
	if (csr & EPX_TX_TXPKTRDY) {
#if DBG_USB_GENERAL
		DBG_I("[USB]: udc_endpoint_write: ep%d is not ready to be written\n",
			 ep_num);

#endif
		return FALSE;
	}
	count = mt_write_fifo(endpoint);

	csr |= EPX_TX_TXPKTRDY;
	writew(csr, IECSR + TXCSR);

	writeb(index, INDEX);

	return count;
}

int udc_request_queue(struct udc_endpoint *ept, struct udc_request *req) {
	u8 intrrxe;

#if DBG_USB_GENERAL
	DBG_I("[USB] %s: ept%d %s queue req=%p, req->length=%x\n",
		__func__, ept->num, ept->in ? "in" : "out", req, req->length);
	DBG_I("[USB] %s: ept%d: %x, ept->in: %s, ept->rcv_urb->buffer: %x, req->buf: %x\n",
		__func__, ept->num, ept, ept->in ? "IN" : "OUT" , ept->rcv_urb->buffer, req->buf);
#endif

	enter_critical_section();
	ept->req = req;
	ept->status = 0;	/* ACTIVE */

	ept->sent = 0;
	ept->last = 0;

	/* read */
	if (!ept->in) {
		ept->rcv_urb->buffer = req->buf;
		ept->rcv_urb->actual_length = 0;

		/* unmask EPx INTRRXE */
		/*
		 * To avoid the parallely access the buffer,
		 * it is umasked here and umask at complete.
		 */
		intrrxe = readb(INTRRXE);
		intrrxe |= (1 << ept->num);
		writeb(intrrxe, INTRRXE);
	}

	/* write */
	if (ept->in) {
		ept->tx_urb->buffer = req->buf;
		ept->tx_urb->actual_length = req->length;

		mt_ep_write(ept);
	}
	exit_critical_section();
	return 0;
}

#if 0
enum handler_return udc_interrupt(void *arg)
{
	struct udc_endpoint *ept;
	unsigned ret;

	return ret;
}
#endif

int udc_register_gadget(struct udc_gadget *gadget)
{
	if (the_gadget) {
		DBG_C("only one gadget supported\n");
		return FALSE;
	}
	the_gadget = gadget;
	return 0;
}

static void udc_ept_desc_fill(struct udc_endpoint *ept, unsigned char *data)
{
	data[0] = 7;
	data[1] = TYPE_ENDPOINT;
	data[2] = ept->num | (ept->in ? USB_DIR_IN : USB_DIR_OUT);
	data[3] = 0x02;		/* bulk -- the only kind we support */
	data[4] = ept->maxpkt;
	data[5] = ept->maxpkt >> 8;
	data[6] = ept->in ? 0x00 : 0x01;
}

static unsigned udc_ifc_desc_size(struct udc_gadget *g)
{
	return 9 + g->ifc_endpoints * 7;
}

static void udc_ifc_desc_fill(struct udc_gadget *g, unsigned char *data)
{
	unsigned n;

	data[0] = 0x09;
	data[1] = TYPE_INTERFACE;
	data[2] = 0x00;		/* ifc number */
	data[3] = 0x00;		/* alt number */
	data[4] = g->ifc_endpoints;
	data[5] = g->ifc_class;
	data[6] = g->ifc_subclass;
	data[7] = g->ifc_protocol;
	data[8] = udc_string_desc_alloc(g->ifc_string);

	data += 9;
	for (n = 0; n < g->ifc_endpoints; n++) {
		udc_ept_desc_fill(g->ept[n], data);
		data += 7;
	}
}

int udc_start(void)
{
	struct udc_descriptor *desc;
	unsigned char *data;
	unsigned size;

	DBG_C("[USB] %s\n", __func__);

	if (!the_device) {
		DBG_C("udc cannot start before init\n");
		return FALSE;
	}
	if (!the_gadget) {
		DBG_C("udc has no gadget registered\n");
		return FALSE;
	}

	/* create our device descriptor */
	desc = udc_descriptor_alloc(TYPE_DEVICE, EP0, 18);
	data = desc->data;
	data[2] = 0x00;		/* usb spec minor rev */
	data[3] = 0x02;		/* usb spec major rev */
	data[4] = 0x00;		/* class */
	data[5] = 0x00;		/* subclass */
	data[6] = 0x00;		/* protocol */
	data[7] = 0x40;		/* max packet size on ept 0 */
	memcpy(data + 8, &the_device->vendor_id, sizeof(short));
	memcpy(data + 10, &the_device->product_id, sizeof(short));
	memcpy(data + 12, &the_device->version_id, sizeof(short));
	data[14] = udc_string_desc_alloc(the_device->manufacturer);
	data[15] = udc_string_desc_alloc(the_device->product);
	data[16] = udc_string_desc_alloc(the_device->serialno);
	data[17] = 1;		/* number of configurations */
	udc_descriptor_register(desc);

	/* create our configuration descriptor */
	size = 9 + udc_ifc_desc_size(the_gadget);
	desc = udc_descriptor_alloc(TYPE_CONFIGURATION, EP0, size);
	data = desc->data;
	data[0] = 0x09;
	data[2] = size;
	data[3] = size >> 8;
	data[4] = 0x01;		/* number of interfaces */
	data[5] = 0x01;		/* configuration value */
	data[6] = 0x00;		/* configuration string */
	data[7] = 0x80;		/* attributes */
	data[8] = 0x80;		/* max power (250ma) -- todo fix this */

	udc_ifc_desc_fill(the_gadget, data + 9);
	udc_descriptor_register(desc);

#if DBG_USB_DUMP_DESC
	DBG_I("%s: dump desc_list\n", __func__);
	for (desc = desc_list; desc; desc = desc->next) {
		DBG_I("tag: %04x\n", desc->tag);
		DBG_I("len: %d\n", desc->len);
		DBG_I("data:");
		hexdump8(desc->data, desc->len);
	}
#endif

	/* register interrupt handler */
	mt_usb_irq_init();

	/* go to RUN mode */
	mt_usb_phy_recover();

	/* clear INTRTX, INTRRX and INTRUSB */
	writew(0xffff, INTRTX); /* writew */
	writew(0xffff, INTRRX); /* writew */
	writeb(0xff, INTRUSB); /* writeb */

	/* unmask usb irq */
#ifdef USB_GINTR
	mt_irq_unmask(MT_USB0_IRQ_ID);
#endif
	writeb((INTRUSB_SUSPEND | INTRUSB_RESUME | INTRUSB_RESET |INTRUSB_DISCON), INTRUSBE);

	while (1) {
#ifdef USB_GINTR
		thread_sleep(1);
#else
		service_interrupts();
#endif
	}

	return 0;
}

int udc_stop(void)
{
	thread_sleep(10);

	mt_usb_disconnect_internal();
	mt_usb_phy_savecurrent();

	return 0;
}
