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

/******************************************************************************
 * gpio.c - MTK Linux GPIO Device Driver
 *
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 *
 * DESCRIPTION:
 *     This file provid the other drivers GPIO relative functions
 *
 ******************************************************************************/

#include <typedefs.h>
#include <gpio.h>
#include <platform.h>
//#include <mt_pmic_wrap_init.h>
//autogen
#include <gpio_cfg.h>

/******************************************************************************
 MACRO Definition
******************************************************************************/
//#define  GIO_SLFTEST
#define GPIO_DEVICE "mt-gpio"
#define VERSION     "$Revision$"
/*---------------------------------------------------------------------------*/
#define GPIO_WR32(addr, data)   __raw_writel(data, addr)
#define GPIO_RD32(addr)         __raw_readl(addr)
#define GPIO_SET_BITS(BIT,REG)   ((*(volatile u32*)(REG)) = (u32)(BIT))
#define GPIO_CLR_BITS(BIT,REG)   ((*(volatile u32*)(REG)) &= ~((u32)(BIT)))
//S32 pwrap_read( U32  adr, U32 *rdata ){return 0;}
//S32 pwrap_write( U32  adr, U32  wdata ){return 0;}

/*---------------------------------------------------------------------------*/
#define TRUE                   1
#define FALSE                  0
/*---------------------------------------------------------------------------*/
//#define MAX_GPIO_REG_BITS      16
//#define MAX_GPIO_MODE_PER_REG  5
//#define GPIO_MODE_BITS         3
/*---------------------------------------------------------------------------*/
#define GPIOTAG                "[GPIO] "
#define GPIOLOG(fmt, arg...)   printf(GPIOTAG fmt, ##arg)
#define GPIOMSG(fmt, arg...)   printf(fmt, ##arg)
#define GPIOERR(fmt, arg...)   printf(GPIOTAG "%5d: "fmt, __LINE__, ##arg)
#define GPIOFUC(fmt, arg...)   //printf(GPIOTAG "%s\n", __FUNCTION__)
#define GIO_INVALID_OBJ(ptr)   ((ptr) != gpio_obj)
/******************************************************************************
Enumeration/Structure
******************************************************************************/
#if (CFG_FPGA_PLATFORM)
		s32 mt_set_gpio_dir(u32 pin, u32 dir)			{return RSUCCESS;}
		s32 mt_get_gpio_dir(u32 pin)				{return GPIO_DIR_UNSUPPORTED;}
		s32 mt_set_gpio_pull_enable(u32 pin, u32 enable)	{return RSUCCESS;}
		s32 mt_get_gpio_pull_enable(u32 pin)			{return GPIO_PULL_EN_UNSUPPORTED;}
		s32 mt_set_gpio_pull_select(u32 pin, u32 select)	{return RSUCCESS;}
		s32 mt_get_gpio_pull_select(u32 pin)			{return GPIO_PULL_UNSUPPORTED;}
		s32 mt_set_gpio_inversion(u32 pin, u32 enable)		{return RSUCCESS;}
		s32 mt_get_gpio_inversion(u32 pin)			{return GPIO_DATA_INV_UNSUPPORTED;}
		s32 mt_set_gpio_out(u32 pin, u32 output)		{return RSUCCESS;}
		s32 mt_get_gpio_out(u32 pin)				{return GPIO_OUT_UNSUPPORTED;}
		s32 mt_get_gpio_in(u32 pin) 				{return GPIO_IN_UNSUPPORTED;}
		s32 mt_set_gpio_mode(u32 pin, u32 mode) 		{return RSUCCESS;}
		s32 mt_get_gpio_mode(u32 pin)				{return GPIO_MODE_UNSUPPORTED;}
		s32 mt_set_clock_output(u32 num, u32 src, u32 div)	{return RSUCCESS;}
		s32 mt_get_clock_output(u32 num, u32 *src, u32 *div)	{return CLK_SRC_UNSUPPORTED;}

#else

#define CLK_NUM 6
static u32 clkout_reg_addr[CLK_NUM] = {
    (0xF0001A00),
    (0xF0001A04),
    (0xF0001A08),
    (0xF0001A0C),
    (0xF0001A10),
    (0xF0001A14)
};
struct mt_gpio_obj {
    GPIO_REGS       *reg;
};
static struct mt_gpio_obj gpio_dat = {
    .reg  = (GPIO_REGS*)(GPIO_BASE),
};
static struct mt_gpio_obj *gpio_obj = &gpio_dat;
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_dir_chip(u32 pin, u32 dir)
{
    u32 pos;
    u32 bit;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if (dir >= GPIO_DIR_MAX)
        return -ERINVAL;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    if (dir == GPIO_DIR_IN)
        GPIO_SET_BITS((1L << bit), &obj->reg->dir[pos].rst);
    else
        GPIO_SET_BITS((1L << bit), &obj->reg->dir[pos].set);
    return RSUCCESS;

}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_dir_chip(u32 pin)
{
    u32 pos;
    u32 bit;
    u32 reg;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    reg = GPIO_RD32(&obj->reg->dir[pos].val);
    return (((reg & (1L << bit)) != 0)? 1: 0);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_pull_enable_chip(u32 pin, u32 enable)
{
    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if((pin >= 67) && (pin < 87)){
	  return GPIO_PULL_EN_UNSUPPORTED;
    }
    else{
    	  if (enable == GPIO_PULL_DISABLE)
		GPIO_SET_BITS((1L << (PULLEN_offset[pin].offset)), PULLEN_addr[pin].addr + 8);
	  else
		GPIO_SET_BITS ((1L << (PULLEN_offset[pin].offset)), PULLEN_addr[pin].addr + 4);
    }

    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_pull_enable_chip(u32 pin)
{
    unsigned long data;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if((pin >= 67) && (pin < 87)){
	  return GPIO_PULL_EN_UNSUPPORTED;
    }
    else{
	  data = GPIO_RD32(PULLEN_addr[pin].addr);

          return (((data & (1L << (PULLEN_offset[pin].offset))) != 0)? 1: 0);
    }
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_pull_select_chip(u32 pin, u32 select)
{
    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if((pin >= 67) && (pin < 87)){
	  return GPIO_PULL_UNSUPPORTED;
    }
    else{
	  if (select == GPIO_PULL_DOWN)
		GPIO_SET_BITS((1L << (PULL_offset[pin].offset)), PULL_addr[pin].addr + 8);
	  else
		GPIO_SET_BITS ((1L << (PULL_offset[pin].offset)), PULL_addr[pin].addr + 4);
    }

    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_pull_select_chip(u32 pin)
{
    unsigned long data;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if((pin >= 67) && (pin < 87)){
	  return GPIO_PULL_UNSUPPORTED;
    }
    else{
	  data = GPIO_RD32(PULL_addr[pin].addr);

    	  return (((data & (1L << (PULL_offset[pin].offset))) != 0)? 1: 0);
    }
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_inversion_chip(u32 pin, u32 enable)
{
    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

	if (enable == GPIO_DATA_UNINV)
		GPIO_SET_BITS((1L << (DATAINV_offset[pin].offset)), DATAINV_addr[pin].addr + 8);
	else
		GPIO_SET_BITS ((1L << (DATAINV_offset[pin].offset)), DATAINV_addr[pin].addr + 4);

    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_inversion_chip(u32 pin)
{

    unsigned long data;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

	data = GPIO_RD32(DATAINV_addr[pin].addr);

    return (((data & (1L << (DATAINV_offset[pin].offset))) != 0)? 1: 0);


}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_out_chip(u32 pin, u32 output)
{
    u32 pos;
    u32 bit;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if (output >= GPIO_OUT_MAX)
        return -ERINVAL;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    if (output == GPIO_OUT_ZERO)
        GPIO_SET_BITS((1L << bit), &obj->reg->dout[pos].rst);
    else
        GPIO_SET_BITS((1L << bit), &obj->reg->dout[pos].set);
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_out_chip(u32 pin)
{
    u32 pos;
    u32 bit;
    u32 reg;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    reg = GPIO_RD32(&obj->reg->dout[pos].val);
    return (((reg & (1L << bit)) != 0)? 1: 0);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_in_chip(u32 pin)
{
    u32 pos;
    u32 bit;
    u32 reg;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    reg = GPIO_RD32(&obj->reg->din[pos].val);
    return (((reg & (1L << bit)) != 0)? 1: 0);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_mode_chip(u32 pin, u32 mode)
{
    u32 pos;
    u32 bit;
    u32 reg;
    u32 mask = (1L << GPIO_MODE_BITS) - 1;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if (mode >= GPIO_MODE_MAX)
        return -ERINVAL;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

#if 0
	reg = GPIO_RD32(&obj->reg->mode[pos].val);

	reg &= ~(mask << (GPIO_MODE_BITS*bit));
	reg |= (mode << (GPIO_MODE_BITS*bit));

	GPIO_WR32(&obj->reg->mode[pos].val, reg);
#endif

    reg = ((1L << (GPIO_MODE_BITS*bit)) << 3) | (mode << (GPIO_MODE_BITS*bit));

    GPIO_WR32(&obj->reg->mode[pos]._align1, reg);

    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_mode_chip(u32 pin)
{
    u32 pos;
    u32 bit;
    u32 reg;
    u32 mask = (1L << GPIO_MODE_BITS) - 1;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	reg = GPIO_RD32(&obj->reg->mode[pos].val);

	return ((reg >> (GPIO_MODE_BITS*bit)) & mask);
}
/*---------------------------------------------------------------------------*/

s32 mt_set_clock_output(u32 num, u32 src, u32 div)
{
    u32 pin_reg;
    u32 reg_value = 0;
    if (num >= CLK_MAX )
        return -ERINVAL;
    if (src >= CLK_SRC_MAX)
        return -ERINVAL;
	if ((div > 16) || (div <= 0))
        return -ERINVAL;

    pin_reg = clkout_reg_addr[num];

    reg_value = div - 1;
    reg_value |= (src << 4);
	GPIO_WR32(pin_reg,reg_value);
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_clock_output(u32 num, u32 * src, u32 *div)
{
    u32 reg_value;
    u32 pin_reg;

    if (num >= CLK_MAX)
        return -ERINVAL;

    pin_reg = clkout_reg_addr[num];
	reg_value = GPIO_RD32(pin_reg);
	*src = reg_value >> 4;
        printk("src==%d\n", *src);
	*div = (reg_value & 0x0f) + 1;
	printk("div==%d\n", *div);
	return RSUCCESS;
}

//set GPIO function in fact
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_dir(u32 pin, u32 dir)
{
    return mt_set_gpio_dir_chip(pin,dir);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_dir(u32 pin)
{    
    return mt_get_gpio_dir_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_pull_enable(u32 pin, u32 enable)
{
    return mt_set_gpio_pull_enable_chip(pin,enable);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_pull_enable(u32 pin)
{
    return mt_get_gpio_pull_enable_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_pull_select(u32 pin, u32 select)
{
    return mt_set_gpio_pull_select_chip(pin,select);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_pull_select(u32 pin)
{
    return mt_get_gpio_pull_select_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_inversion(u32 pin, u32 enable)
{
    return mt_set_gpio_inversion_chip(pin,enable);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_inversion(u32 pin)
{
    return mt_get_gpio_inversion_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_out(u32 pin, u32 output)
{
    return mt_set_gpio_out_chip(pin,output);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_out(u32 pin)
{
    return mt_get_gpio_out_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_in(u32 pin)
{
    return mt_get_gpio_in_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_mode(u32 pin, u32 mode)
{
    return mt_set_gpio_mode_chip(pin,mode);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_mode(u32 pin)
{
    return mt_get_gpio_mode_chip(pin);
}
#endif
void mt_gpio_set_power(void)
{
#if 0 //FIXME: needs porting
	u32 val;
#define GPIO_BIAS_KEY_ADDR (0x1020c000 + 0x920)
	val = GPIO_RD32(GPIO_BASE + 0x900);
	val &= ~((1<<1) | (1<<7) | (1<<13));
	val |= (GPIO_DVDD28_NML2<<1) | (GPIO_DVDD28_BSI<<7) | (GPIO_DVDD28_BPI<<13);
	/*Key unlock*/
	GPIO_WR32(GPIO_BIAS_KEY_ADDR, 0x58);
	GPIO_WR32(GPIO_BIAS_KEY_ADDR, 0xfa);
	GPIO_WR32(GPIO_BIAS_KEY_ADDR, 0x65);
	GPIO_WR32(GPIO_BIAS_KEY_ADDR, 0x83);

    GPIO_WR32(GPIO_BASE + 0x900,val);
	/*Key lock*/
	GPIO_WR32(GPIO_BIAS_KEY_ADDR, 0x00);
	GPIO_WR32(GPIO_BIAS_KEY_ADDR, 0x00);
	GPIO_WR32(GPIO_BIAS_KEY_ADDR, 0x00);
	GPIO_WR32(GPIO_BIAS_KEY_ADDR, 0x00);
#endif
    //GPIOVER("mt_gpio_set_power() done,val %x\n",val);
}

void mt_gpio_init_post(U32 post_init)
{
    /*MT6572 use UART2 as AP console*/
    #ifdef GPIO_UART_URXD2_PIN
    mt_set_gpio_mode(GPIO_UART_URXD2_PIN, GPIO_MODE_01);
    mt_set_gpio_mode(GPIO_UART_UTXD2_PIN, GPIO_MODE_01);
    #endif

    if (1==post_init) {
    /* UART Powr PDN and Reset*/
	//mt_gpio_set_power();
    /*MT6572 use UART1 as MD console*/
        #ifdef GPIO_UART_URXD1_PIN
        mt_set_gpio_mode(GPIO_UART_URXD1_PIN, GPIO_MODE_05);
        mt_set_gpio_mode(GPIO_UART_UTXD1_PIN, GPIO_MODE_05);
        #endif
    /* UART Powr PDN and Reset*/
	//mt_gpio_set_power();
    }
    else {
        #ifdef GPIO_UART_URXD1_PIN
        mt_set_gpio_mode(GPIO_UART_URXD1_PIN, GPIO_MODE_01);
        mt_set_gpio_mode(GPIO_UART_UTXD1_PIN, GPIO_MODE_01);
        #endif
    }
}

