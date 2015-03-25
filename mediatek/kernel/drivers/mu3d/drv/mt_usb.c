/*
 * MUSB OTG controller driver for Blackfin Processors
 *
 * Copyright 2006-2008 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/xlog.h>
#include <mach/irqs.h>
#include <mach/eint.h>

#ifndef CONFIG_MTK_FPGA
#include <cust_gpio_usage.h>
#endif
//#include <mach/mt_pm_ldo.h>
//#include <mach/mt_clkmgr.h>
//#include <linux/i2c.h>
#include "mach/emi_mpu.h"

#include <linux/mu3d/hal/mu3d_hal_osal.h>
#include "musb_core.h"

extern struct platform_device *mu3d_musb;

#ifndef CONFIG_MTK_FPGA
extern void upmu_interrupt_chrdet_int_en(kal_uint32 val);
#endif

typedef enum
{
    CABLE_MODE_CHRG_ONLY = 0,
    CABLE_MODE_NORMAL,
    CABLE_MODE_HOST_ONLY,
    CABLE_MODE_MAX
} CABLE_MODE;

u32 cable_mode = CABLE_MODE_NORMAL;

/* ================================ */
/* connect and disconnect functions */
/* ================================ */
bool mt_usb_is_device(void)
{
	os_printk(K_DEBUG,"[MUSB] mt_usb_is_device\n");
#ifdef NEVER
	if(!mtk_musb){
		DBG(0,"mtk_musb is NULL\n");
		return false; // don't do charger detection when usb is not ready
	} else {
		DBG(4,"is_host=%d\n",mtk_musb->is_host);
	}
  return !mtk_musb->is_host;
#endif /* NEVER */
	return true;
}

bool mt_usb_is_ready(void)
{
	printk("[MUSB] USB is ready or not\n");
#ifdef NEVER
	if(!mtk_musb || !mtk_musb->is_ready)
        return false;
    else
        return true;
#endif /* NEVER */
	return true;
}

void mt_usb_connect(void)
{
	os_printk(K_DEBUG,"%s+\n", __func__);

	musb_start(mu3d_musb);

	os_printk(K_DEBUG,"%s-\n", __func__);
}
EXPORT_SYMBOL_GPL(mt_usb_connect);

void mt_usb_disconnect(void)
{
	os_printk(K_DEBUG,"%s\n", __func__);

	musb_stop(mu3d_musb);

	os_printk(K_DEBUG,"%s-\n", __func__);
}
EXPORT_SYMBOL_GPL(mt_usb_disconnect);

bool usb_cable_connected(void)
{
	os_printk(K_DEBUG,"[MUSB] %s\n", __func__);
#ifdef NEVER
#ifdef CONFIG_POWER_EXT
	if (upmu_get_rgs_chrdet()
#else
	if (upmu_is_chr_det()
#endif
	   && (mt_charger_type_detection() == STANDARD_HOST)) {
		return true;
	} else {
		return false;
	}
#endif /* NEVER */
	return true;
}
EXPORT_SYMBOL_GPL(usb_cable_connected);

#ifdef NEVER
void musb_platform_reset(struct musb *musb)
{
	u16 swrst = 0;
	void __iomem	*mbase = musb->mregs;
	swrst = musb_readw(mbase,MUSB_SWRST);
	swrst |= (MUSB_SWRST_DISUSBRESET | MUSB_SWRST_SWRST);
	musb_writew(mbase, MUSB_SWRST,swrst);
}
#endif /* NEVER */

void usb_check_connect(void)
{
	os_printk(K_DEBUG,"[MUSB] usb_check_connect\n");
#ifdef NEVER
#ifndef CONFIG_MTK_FPGA
	if (usb_cable_connected())
		mt_usb_connect();
#endif
#endif /* NEVER */
}

void musb_sync_with_bat(struct musb *musb, int usb_state)
{
	os_printk(K_DEBUG,"[MUSB] musb_sync_with_bat\n");
#ifdef NEVER
#ifndef CONFIG_MTK_FPGA
	BATTERY_SetUSBState(usb_state);
	wake_up_bat();
#endif
#endif /* NEVER */
}

/*--FOR INSTANT POWER ON USAGE--------------------------------------------------*/
static ssize_t mt_usb_show_cmode(struct device* dev, struct device_attribute *attr, char *buf)
{
	if (!dev) {
		os_printk(K_DEBUG,"dev is null!!\n");
		return 0;
	}
	return scnprintf(buf, PAGE_SIZE, "%d\n", cable_mode);
}

static ssize_t mt_usb_store_cmode(struct device* dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int cmode;

	if (!dev) {
		os_printk(K_DEBUG,"dev is null!!\n");
		return count;
	} else if (1 == sscanf(buf, "%d", &cmode)) {
		os_printk(K_DEBUG,"cmode=%d, cable_mode=%d\n", cmode, cable_mode);

#ifdef NEVER
		if (cmode >= CABLE_MODE_MAX)
			cmode = CABLE_MODE_NORMAL;

		if (cable_mode != cmode) {
			if(mtk_musb) {
				if(down_interruptible(&mtk_musb->musb_lock))
					xlog_printk(ANDROID_LOG_ERROR, "USB20", "%s: busy, Couldn't get musb_lock\n", __func__);
			}
			if(cmode == CABLE_MODE_CHRG_ONLY) { // IPO shutdown, disable USB
				if(mtk_musb) {
					mtk_musb->in_ipo_off = true;
				}

			} else { // IPO bootup, enable USB
				if(mtk_musb) {
					mtk_musb->in_ipo_off = false;
				}
			}

			mt_usb_disconnect();
			cable_mode = cmode;
			msleep(10);
			//ALPS00114502
			//check that "if USB cable connected and than call mt_usb_connect"
			//Then, the Bat_Thread won't be always wakeup while no USB/chatger cable and IPO mode
			//mt_usb_connect();
			usb_check_connect();
			//ALPS00114502

#ifdef CONFIG_USB_MTK_OTG
			if(cmode == CABLE_MODE_CHRG_ONLY) {
				if(mtk_musb && mtk_musb->is_host) { // shut down USB host for IPO
					musb_stop(mtk_musb);
					/* Think about IPO shutdown with A-cable, then switch to B-cable and IPO bootup. We need a point to clear session bit */
					musb_writeb(mtk_musb->mregs, MUSB_DEVCTL, (~MUSB_DEVCTL_SESSION)&musb_readb(mtk_musb->mregs,MUSB_DEVCTL));
				} else {
					switch_int_to_host_and_mask(); // mask ID pin interrupt even if A-cable is not plugged in
				}
			} else {
				switch_int_to_host(); // resotre ID pin interrupt
			}
#endif
			if(mtk_musb) {
				up(&mtk_musb->musb_lock);
			}
		}
#endif /* NEVER */

	}
	return count;
}

DEVICE_ATTR(cmode,  0664, mt_usb_show_cmode, mt_usb_store_cmode);

#ifdef NEVER
#ifdef CONFIG_MTK_FPGA
static struct i2c_client *usb_i2c_client = NULL;
static const struct i2c_device_id usb_i2c_id[] = {{"mtk-usb",0},{}};

static struct i2c_board_info __initdata usb_i2c_dev = { I2C_BOARD_INFO("mtk-usb", 0x60)};


void USB_PHY_Write_Register8(UINT8 var,  UINT8 addr)
{
	char buffer[2];
	buffer[0] = addr;
	buffer[1] = var;
	i2c_master_send(usb_i2c_client, &buffer, 2);
}

UINT8 USB_PHY_Read_Register8(UINT8 addr)
{
	UINT8 var;
	i2c_master_send(usb_i2c_client, &addr, 1);
	i2c_master_recv(usb_i2c_client, &var, 1);
	return var;
}

static int usb_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	printk("[MUSB]usb_i2c_probe, start\n");

	usb_i2c_client = client;

	//disable usb mac suspend
	DRV_WriteReg8(USB_SIF_BASE + 0x86a, 0x00);

	//usb phy initial sequence
	USB_PHY_Write_Register8(0x00, 0xFF);
	USB_PHY_Write_Register8(0x04, 0x61);
	USB_PHY_Write_Register8(0x00, 0x68);
	USB_PHY_Write_Register8(0x00, 0x6a);
	USB_PHY_Write_Register8(0x6e, 0x00);
	USB_PHY_Write_Register8(0x0c, 0x1b);
	USB_PHY_Write_Register8(0x44, 0x08);
	USB_PHY_Write_Register8(0x55, 0x11);
	USB_PHY_Write_Register8(0x68, 0x1a);


	printk("[MUSB]addr: 0xFF, value: %x\n", USB_PHY_Read_Register8(0xFF));
	printk("[MUSB]addr: 0x61, value: %x\n", USB_PHY_Read_Register8(0x61));
	printk("[MUSB]addr: 0x68, value: %x\n", USB_PHY_Read_Register8(0x68));
	printk("[MUSB]addr: 0x6a, value: %x\n", USB_PHY_Read_Register8(0x6a));
	printk("[MUSB]addr: 0x00, value: %x\n", USB_PHY_Read_Register8(0x00));
	printk("[MUSB]addr: 0x1b, value: %x\n", USB_PHY_Read_Register8(0x1b));
	printk("[MUSB]addr: 0x08, value: %x\n", USB_PHY_Read_Register8(0x08));
	printk("[MUSB]addr: 0x11, value: %x\n", USB_PHY_Read_Register8(0x11));
	printk("[MUSB]addr: 0x1a, value: %x\n", USB_PHY_Read_Register8(0x1a));


	printk("[MUSB]usb_i2c_probe, end\n");
    return 0;

}

static int usb_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, "mtk-usb");
    return 0;
}

static int usb_i2c_remove(struct i2c_client *client) {return 0;}


struct i2c_driver usb_i2c_driver = {
    .probe = usb_i2c_probe,
    .remove = usb_i2c_remove,
    .detect = usb_i2c_detect,
    .driver = {
    	.name = "mtk-usb",
    },
    .id_table = usb_i2c_id,
};

int add_usb_i2c_driver()
{
	i2c_register_board_info(0, &usb_i2c_dev, 1);
	if(i2c_add_driver(&usb_i2c_driver)!=0)
	{
		printk("[MUSB]usb_i2c_driver initialization failed!!\n");
		return -1;
	}
	else
	{
		printk("[MUSB]usb_i2c_driver initialization succeed!!\n");
	}
	return 0;
}
#endif //End of CONFIG_MTK_FPGA
#endif /* NEVER */
