/******************************************************************************
 * mt_gpio.c - MTKLinux GPIO Device Driver
 * 
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *     This file provid the other drivers GPIO relative functions
 *
 ******************************************************************************/

//#include <common.h>
//#include <asm/arch/mt65xx_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_gpio.h>
//#include <asm/io.h>
//#include <asm/mach-types.h>
//#include <platform/mt_pmic_wrap_uboot.h>
#include <debug.h>
/******************************************************************************
 MACRO Definition
******************************************************************************/
//#define  GIO_SLFTEST            
#define GPIO_DEVICE "mt-gpio"
#define VERSION     GPIO_DEVICE
/*---------------------------------------------------------------------------*/
#define GPIO_WR32(addr, data)   DRV_WriteReg16(addr,data)
#define GPIO_RD32(addr)         DRV_Reg16(addr)
#define GPIO_SET_BITS(BIT,REG)   ((*REG) = (u16)(BIT))
#define GPIO_CLR_BITS(BIT,REG)   ((*REG) &= ~((u16)(BIT)))

/*---------------------------------------------------------------------------*/
#define TRUE                   1
#define FALSE                  0
/*---------------------------------------------------------------------------*/
#define MAX_GPIO_REG_BITS      16
#define MAX_GPIO_MODE_PER_REG  5
#define GPIO_MODE_BITS         3 
/*---------------------------------------------------------------------------*/
#define GPIOTAG                "[GPIO] "
#define GPIOLOG(fmt, arg...)   dprintf(INFO,GPIOTAG fmt, ##arg)
#define GPIOMSG(fmt, arg...)   dprintf(INFO,fmt, ##arg)
#define GPIOERR(fmt, arg...)   dprintf(INFO,GPIOTAG "%5d: "fmt, __LINE__, ##arg)
#define GPIOFUC(fmt, arg...)   //dprintk(INFO,GPIOTAG "%s\n", __FUNCTION__)
#define GIO_INVALID_OBJ(ptr)   ((ptr) != gpio_obj)

//#ifndef GPIO_BASE
//	#define GPIO_BASE 0x10005000
//#endif
/******************************************************************************
Enumeration/Structure
******************************************************************************/
#if  !defined(MT_GPIO_ENABLED)
		s32 mt_set_gpio_dir(u32 pin, u32 dir)				{return RSUCCESS;}
		s32 mt_get_gpio_dir(u32 pin)						{return GPIO_DIR_UNSUPPORTED;}	
		s32 mt_set_gpio_pull_enable(u32 pin, u32 enable)	{return RSUCCESS;}
		s32 mt_get_gpio_pull_enable(u32 pin)				{return GPIO_PULL_EN_UNSUPPORTED;}
		s32 mt_set_gpio_pull_select(u32 pin, u32 select)	{return RSUCCESS;}
		s32 mt_get_gpio_pull_select(u32 pin)				{return GPIO_PULL_UNSUPPORTED;}
		s32 mt_set_gpio_inversion(u32 pin, u32 enable)		{return RSUCCESS;}
		s32 mt_get_gpio_inversion(u32 pin)					{return GPIO_DATA_INV_UNSUPPORTED;}
		s32 mt_set_gpio_out(u32 pin, u32 output)			{return RSUCCESS;}
		s32 mt_get_gpio_out(u32 pin)						{return GPIO_OUT_UNSUPPORTED;}
		s32 mt_get_gpio_in(u32 pin) 						{return GPIO_IN_UNSUPPORTED;}
		s32 mt_set_gpio_mode(u32 pin, u32 mode) 			{return RSUCCESS;}
		s32 mt_get_gpio_mode(u32 pin)						{return GPIO_MODE_UNSUPPORTED;}
		s32 mt_set_clock_output(u32 num, u32 src, u32 div)	  {return RSUCCESS;}
		s32 mt_get_clock_output(u32 num, u32 *src, u32 *div)  {return CLK_SRC_UNSUPPORTED;}
#else

/*-------for special kpad pupd-----------*/
struct kpad_pupd {
	unsigned char 	pin;
	unsigned char	reg;
	unsigned char	bit;
};
static struct kpad_pupd kpad_pupd_spec[] = {
	{GPIO74,	0,	2},
	{GPIO75,	1,	2},
	{GPIO92,	0,	6},
	{GPIO93,	0,	10},
	{GPIO167,	1,	6},
	{GPIO168,	1,	10}
};
/*-------for msdc pupd-----------*/
struct msdc_pupd {
	unsigned char 	pin;
	unsigned char	reg;
	unsigned char	bit;
};
static struct msdc_pupd msdc_pupd_spec[2][6] = {
	{
	{GPIO124,	1,	1},
	{GPIO125,	0,	1},
	{GPIO126,	2,	1},
	{GPIO127,	2,	3},
	{GPIO128,	2,	5},
	{GPIO129,	2,	7}},
	{
	{GPIO114,	1,	1},
	{GPIO115,	0,	1},
	{GPIO116,	2,	1},
	{GPIO117,	2,	3},
	{GPIO118,	2,	5},
	{GPIO119,	2,	7}}
};
/*---------------------------------------*/
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
    u32 pos;
    u32 bit;
	u32 i,j;

    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;
    
    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if (enable >= GPIO_PULL_EN_MAX)
        return -ERINVAL;

	/*for special kpad pupd*/
	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin){
			if (enable == GPIO_PULL_ENABLE)
				GPIO_SET_BITS((3L << (kpad_pupd_spec[i].bit-2)), &obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].set);
			else
				GPIO_SET_BITS((3L << (kpad_pupd_spec[i].bit-2)), &obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].rst);
			return RSUCCESS;
		}
	}

	if (((pin >= GPIO114)&&(pin <= GPIO119))||((pin >= GPIO124)&&(pin <= GPIO129))){
		/* msdc IO */
		for(i = 0; i < sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]); i++){
			for(j = 0; j < sizeof(msdc_pupd_spec[0])/sizeof(msdc_pupd_spec[0][0]); j++){
				if (pin == msdc_pupd_spec[i][j].pin){
					if (enable == GPIO_PULL_DISABLE){
						if (i == 0) {
							GPIO_SET_BITS((3L << (msdc_pupd_spec[i][j].bit - 1)), &obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].rst);
						} else if (i == 1) {
							GPIO_SET_BITS((3L << (msdc_pupd_spec[i][j].bit - 1)), &obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].rst);
						}
					}else{
						if (i == 0) {
							GPIO_SET_BITS((1L << (msdc_pupd_spec[i][j].bit)), &obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].set);
							GPIO_SET_BITS((1L << (msdc_pupd_spec[i][j].bit - 1)), &obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].rst);
						} else if (i == 1) {
							GPIO_SET_BITS((1L << (msdc_pupd_spec[i][j].bit)), &obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].set);
							GPIO_SET_BITS((1L << (msdc_pupd_spec[i][j].bit - 1)), &obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].rst);
						}
					}
					return RSUCCESS;
				}
			}
		}
	}else if((pin >= GPIO141)&&(pin <= GPIO166)){
		return GPIO_PULL_EN_UNSUPPORTED;	
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

		if (enable == GPIO_PULL_DISABLE)
			GPIO_SET_BITS((1L << bit), &obj->reg->pullen[pos].rst);
		else
			GPIO_SET_BITS((1L << bit), &obj->reg->pullen[pos].set);
	}
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_pull_enable_chip(u32 pin)
{
    u32 pos=0;
    u32 bit=0;
    u32 reg=0;
	u32 i,j;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;
    
    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

	/*for special kpad pupd*/
	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin){
			reg = GPIO_RD32(&obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].val);
			return (((reg & (3L << (kpad_pupd_spec[i].bit-2))) == 0)? 0: 1);
		}
	}
   
	if (((pin >= GPIO114)&&(pin <= GPIO119))||((pin >= GPIO124)&&(pin <= GPIO129))){
		/* msdc IO */
		for(i = 0; i < sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]); i++){
			for(j = 0; j < sizeof(msdc_pupd_spec[0])/sizeof(msdc_pupd_spec[0][0]); j++){
				if (pin == msdc_pupd_spec[i][j].pin){
					if (i == 0) {
						reg = GPIO_RD32(&obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].val);
					} else if (i == 1) {
						reg = GPIO_RD32(&obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].val);
					}

					return (((reg & (3L << (msdc_pupd_spec[i][j].bit - 1))) == 0)? 0: 1);
				}
			}
		}
	}else if((pin >= GPIO141)&&(pin <= GPIO166)){
		return GPIO_PULL_EN_UNSUPPORTED;
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

		reg = GPIO_RD32(&obj->reg->pullen[pos].val);
	}
	return (((reg & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_pull_select_chip(u32 pin, u32 select)
{
    u32 pos;
    u32 bit;
	u32 i,j;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;
    
    if (select >= GPIO_PULL_MAX)
        return -ERINVAL;

	/*for special kpad pupd*/
	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin){
			if (select == GPIO_PULL_DOWN)
				GPIO_SET_BITS((1L << kpad_pupd_spec[i].bit), &obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].set);
			else
				GPIO_SET_BITS((1L << kpad_pupd_spec[i].bit), &obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].rst);
			return RSUCCESS;
		}
	}

	if (((pin >= GPIO114)&&(pin <= GPIO119))||((pin >= GPIO124)&&(pin <= GPIO129))){
		/* msdc IO */
		for(i = 0; i < sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]); i++){
			for(j = 0; j < sizeof(msdc_pupd_spec[0])/sizeof(msdc_pupd_spec[0][0]); j++){
				if (pin == msdc_pupd_spec[i][j].pin){
					if (select == GPIO_PULL_DOWN){
						if (i == 0) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].rst);
							GPIO_SET_BITS((1L << (msdc_pupd_spec[i][j].bit - 1)), &obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].set);
						} else if (i == 1) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].rst);
							GPIO_SET_BITS((1L << (msdc_pupd_spec[i][j].bit - 1)), &obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].set);
						}
					}else{
						if (i == 0) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].set);
							GPIO_SET_BITS((1L << (msdc_pupd_spec[i][j].bit - 1)), &obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].rst);
						} else if (i == 1) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].set);
							GPIO_SET_BITS((1L << (msdc_pupd_spec[i][j].bit - 1)), &obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].rst);
						}
					}
					return RSUCCESS;
				}
			}
		}
	}else if((pin >= GPIO141)&&(pin <= GPIO166)){
		return GPIO_PULL_EN_UNSUPPORTED;
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;
		
		if (select == GPIO_PULL_DOWN)
			GPIO_SET_BITS((1L << bit), &obj->reg->pullsel[pos].rst);
		else
			GPIO_SET_BITS((1L << bit), &obj->reg->pullsel[pos].set);
	}
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_pull_select_chip(u32 pin)
{
    u32 pos=0;
    u32 bit=0;
    u32 reg=0;
	u32 i,j;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

	/*for special kpad pupd*/
	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin){
			reg = GPIO_RD32(&obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].val);
			return (((reg & (1L << kpad_pupd_spec[i].bit)) != 0)? 0: 1);
		}
	}
    
	if (((pin >= GPIO114)&&(pin <= GPIO119))||((pin >= GPIO124)&&(pin <= GPIO129))){
		/* msdc IO */
		for(i = 0; i < sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]); i++){
			for(j = 0; j < sizeof(msdc_pupd_spec[0])/sizeof(msdc_pupd_spec[0][0]); j++){
				if (pin == msdc_pupd_spec[i][j].pin){
					if (i == 0) {
						reg = GPIO_RD32(&obj->reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].val);
					} else if (i == 1) {
						reg = GPIO_RD32(&obj->reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].val);
					}

					return (((reg & (1L << msdc_pupd_spec[i][j].bit)) != 0)? 1: 0);
				}
			}
		}
	}else if((pin >= GPIO141)&&(pin <= GPIO166)){
		return GPIO_PULL_EN_UNSUPPORTED;
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

		reg = GPIO_RD32(&obj->reg->pullsel[pos].val);
	}
    return (((reg & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_inversion_chip(u32 pin, u32 enable)
{/*
    u32 pos;
    u32 bit;
    u32 reg;
    u32 mask;
    struct mt_gpio_obj *obj = gpio_obj;

    if (!obj)
        return -ERACCESS;

    if (pin >= MAX_GPIO_PIN)
        return -ERINVAL;

    if (enable >= GPIO_DATA_INV_MAX)
        return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;
	
	if (enable == GPIO_DATA_UNINV)
		GPIO_SET_BITS((1L << bit), &obj->reg->dinv[pos].rst);
	else
		GPIO_SET_BITS((1L << bit), &obj->reg->dinv[pos].set);

    return RSUCCESS;*/
	return -ERINVAL;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_inversion_chip(u32 pin)
{/*
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

	reg = GPIO_RD32(&obj->reg->dinv[pos].val);

    return (((reg & (1L << bit)) != 0)? 1: 0);   */
	return -ERINVAL;	
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
   
	reg = GPIO_RD32(&obj->reg->mode[pos].val);

	reg &= ~(mask << (GPIO_MODE_BITS*bit));
	reg |= (mode << (GPIO_MODE_BITS*bit));
	
	GPIO_WR32(&obj->reg->mode[pos].val, reg);

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
{/*
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
	GPIO_WR32(pin_reg,reg_value);*/
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
s32 mt_get_clock_output(u32 num, u32 * src, u32 *div)
{/*
    u32 reg_value;
    u32 pin_reg;
  
    if (num >= CLK_MAX)
        return -ERINVAL;

    pin_reg = clkout_reg_addr[num];
	reg_value = GPIO_RD32(pin_reg);
	*src = reg_value >> 4;
        printk("src==%d\n", *src);
	*div = (reg_value & 0x0f) + 1;    
	printk("div==%d\n", *div);*/
	return RSUCCESS;
}

void mt_gpio_pin_decrypt(u32 *cipher)
{
	//just for debug, find out who used pin number directly
	if((*cipher & (0x80000000)) == 0){
		GPIOERR("Pin %u decrypt warning! \n",(unsigned int)*cipher);	
		//dump_stack();
		//return;
	}

	//GPIOERR("Pin magic number is %x\n",*cipher);
	*cipher &= ~(0x80000000);
	return;
}
//set GPIO function in fact
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_dir(u32 pin, u32 dir)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_set_gpio_dir_chip(pin,dir);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_dir(u32 pin)
{    
	mt_gpio_pin_decrypt(&pin);
    return mt_get_gpio_dir_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_pull_enable(u32 pin, u32 enable)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_set_gpio_pull_enable_chip(pin,enable);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_pull_enable(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_get_gpio_pull_enable_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_pull_select(u32 pin, u32 select)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_set_gpio_pull_select_chip(pin,select);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_pull_select(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_get_gpio_pull_select_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_inversion(u32 pin, u32 enable)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_set_gpio_inversion_chip(pin,enable);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_inversion(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_get_gpio_inversion_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_out(u32 pin, u32 output)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_set_gpio_out_chip(pin,output);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_out(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_get_gpio_out_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_in(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_get_gpio_in_chip(pin);
}
/*---------------------------------------------------------------------------*/
s32 mt_set_gpio_mode(u32 pin, u32 mode)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_set_gpio_mode_chip(pin,mode);
}
/*---------------------------------------------------------------------------*/
s32 mt_get_gpio_mode(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
    return mt_get_gpio_mode_chip(pin);
}
/*****************************************************************************/
/* sysfs operation                                                           */
/*****************************************************************************/
void mt_gpio_self_test(void)
{
    int i, val;
    for (i = 0; i < GPIO_MAX; i++)
    {
        s32 res,old;
        GPIOMSG("GPIO-%3d test\n", i);
        /*direction test*/
        old = mt_get_gpio_dir(i);
        if (old == 0 || old == 1) {
            GPIOLOG(" dir old = %d\n", old);
        } else {
            GPIOERR(" test dir fail: %d\n", old);
            break;
        }
        if ((res = mt_set_gpio_dir(i, GPIO_DIR_OUT)) != RSUCCESS) {
            GPIOERR(" set dir out fail: %d\n", res);
            break;
        } else if ((res = mt_get_gpio_dir(i)) != GPIO_DIR_OUT) {
            GPIOERR(" get dir out fail: %d\n", res);
            break;
        } else {
            /*output test*/
            s32 out = mt_get_gpio_out(i);
            if (out != 0 && out != 1) {
                GPIOERR(" get out fail = %d\n", old);
                break;
            } 
            for (val = 0; val < GPIO_OUT_MAX; val++) {
                if ((res = mt_set_gpio_out(i,0)) != RSUCCESS) {
                    GPIOERR(" set out[%d] fail: %d\n", val, res);
                    break;
                } else if ((res = mt_get_gpio_out(i)) != 0) {
                    GPIOERR(" get out[%d] fail: %d\n", val, res);
                    break;
                }
            }
            if ((res = mt_set_gpio_out(i,out)) != RSUCCESS)
            {
                GPIOERR(" restore out fail: %d\n", res);
                break;
            }
        }
            
        if ((res = mt_set_gpio_dir(i, GPIO_DIR_IN)) != RSUCCESS) {
            GPIOERR(" set dir in fail: %d\n", res);
            break;
        } else if ((res = mt_get_gpio_dir(i)) != GPIO_DIR_IN) {
            GPIOERR(" get dir in fail: %d\n", res);
            break;
        } else {
            GPIOLOG(" input data = %d\n", res);
        }
        
        if ((res = mt_set_gpio_dir(i, old)) != RSUCCESS) {
            GPIOERR(" restore dir fail: %d\n", res);
            break;
        }
        for (val = 0; val < GPIO_PULL_EN_MAX; val++) {
            if ((res = mt_set_gpio_pull_enable(i,val)) != RSUCCESS) {
                GPIOERR(" set pullen[%d] fail: %d\n", val, res);
                break;
            } else if ((res = mt_get_gpio_pull_enable(i)) != val) {
                GPIOERR(" get pullen[%d] fail: %d\n", val, res);
                break;
            }
        }        
        if ((res = mt_set_gpio_pull_enable(i, old)) != RSUCCESS) {
            GPIOERR(" restore pullen fail: %d\n", res);
            break;
        }

        /*pull select test*/
        old = mt_get_gpio_pull_select(i);
        if (old == 0 || old == 1)
            GPIOLOG(" pullsel old = %d\n", old);
        else {
            GPIOERR(" pullsel fail: %d\n", old);
            break;
        }
        for (val = 0; val < GPIO_PULL_MAX; val++) {
            if ((res = mt_set_gpio_pull_select(i,val)) != RSUCCESS) {
                GPIOERR(" set pullsel[%d] fail: %d\n", val, res);
                break;
            } else if ((res = mt_get_gpio_pull_select(i)) != val) {
                GPIOERR(" get pullsel[%d] fail: %d\n", val, res);
                break;
            }
        } 
        if ((res = mt_set_gpio_pull_select(i, old)) != RSUCCESS)
        {
            GPIOERR(" restore pullsel fail: %d\n", res);
            break;
        }     

        /*data inversion
        old = mt_get_gpio_inversion(i);
        if (old == 0 || old == 1)
            GPIOLOG(" inv old = %d\n", old);
        else {
            GPIOERR(" inv fail: %d\n", old);
            break;
        }
        for (val = 0; val < GPIO_DATA_INV_MAX; val++) {
            if ((res = mt_set_gpio_inversion(i,val)) != RSUCCESS) {
                GPIOERR(" set inv[%d] fail: %d\n", val, res);
                break;
            } else if ((res = mt_get_gpio_inversion(i)) != val) {
                GPIOERR(" get inv[%d] fail: %d\n", val, res);
                break;
            }
        } 
        if ((res = mt_set_gpio_inversion(i, old)) != RSUCCESS) {
            GPIOERR(" restore inv fail: %d\n", res);
            break;
        }     */

        /*mode control*/
		old = mt_get_gpio_mode(i);
		if ((old >= GPIO_MODE_00) && (val < GPIO_MODE_MAX)){
			GPIOLOG(" mode old = %d\n", old);
		}
		else{
			GPIOERR(" get mode fail: %d\n", old);
			break;
		}
		for (val = 0; val < GPIO_MODE_MAX; val++) {
			if ((res = mt_set_gpio_mode(i, val)) != RSUCCESS) {
				GPIOERR("set mode[%d] fail: %d\n", val, res);
				break;
			} else if ((res = mt_get_gpio_mode(i)) != val) {
				GPIOERR("get mode[%d] fail: %d\n", val, res);
				break;
			}            
		}        
		if ((res = mt_set_gpio_mode(i,old)) != RSUCCESS) {
			GPIOERR(" restore mode fail: %d\n", res);
			break;
		}      
    }
    GPIOLOG("GPIO test done\n");
}
/*----------------------------------------------------------------------------*/
void mt_gpio_dump(void) 
{
    GPIO_REGS *regs = (GPIO_REGS*)(GPIO_BASE);
    unsigned int idx; 

    GPIOMSG("---# dir #-----------------------------------------------------------------\n");
    for (idx = 0; idx < sizeof(regs->dir)/sizeof(regs->dir[0]); idx++) {
        GPIOMSG("0x%04X ", regs->dir[idx].val);
        if (7 == (idx % 8)) GPIOMSG("\n");
    }
    GPIOMSG("\n---# pullen #--------------------------------------------------------------\n");        
    for (idx = 0; idx < sizeof(regs->pullen)/sizeof(regs->pullen[0]); idx++) {
        GPIOMSG("0x%04X ", regs->pullen[idx].val);    
        if (7 == (idx % 8)) GPIOMSG("\n");
    }
    GPIOMSG("\n---# pullsel #-------------------------------------------------------------\n");   
    for (idx = 0; idx < sizeof(regs->pullsel)/sizeof(regs->pullsel[0]); idx++) {
        GPIOMSG("0x%04X ", regs->pullsel[idx].val);     
        if (7 == (idx % 8)) GPIOMSG("\n");
    }
/*    GPIOMSG("\n---# dinv #----------------------------------------------------------------\n");   
    for (idx = 0; idx < sizeof(regs->dinv)/sizeof(regs->dinv[0]); idx++) {
        GPIOMSG("0x%04X ", regs->dinv[idx].val);     
        if (7 == (idx % 8)) GPIOMSG("\n");
    }*/
    GPIOMSG("\n---# dout #----------------------------------------------------------------\n");   
    for (idx = 0; idx < sizeof(regs->dout)/sizeof(regs->dout[0]); idx++) {
        GPIOMSG("0x%04X ", regs->dout[idx].val);     
        if (7 == (idx % 8)) GPIOMSG("\n");
    }
    GPIOMSG("\n---# din  #----------------------------------------------------------------\n");   
    for (idx = 0; idx < sizeof(regs->din)/sizeof(regs->din[0]); idx++) {
        GPIOMSG("0x%04X ", regs->din[idx].val);     
        if (7 == (idx % 8)) GPIOMSG("\n");
    }
    GPIOMSG("\n---# mode #----------------------------------------------------------------\n");   
    for (idx = 0; idx < sizeof(regs->mode)/sizeof(regs->mode[0]); idx++) {
        GPIOMSG("0x%04X ", regs->mode[idx].val);     
        if (7 == (idx % 8)) GPIOMSG("\n");
    }    
    GPIOMSG("\n---------------------------------------------------------------------------\n");    
}
/*---------------------------------------------------------------------------*/
void mt_gpio_read_pin(GPIO_CFG* cfg, int method)
{
    if (method == 0) {
        GPIO_REGS *cur = (GPIO_REGS*)GPIO_BASE;    
        u32 mask = (1L << GPIO_MODE_BITS) - 1;        
        int num, bit,idx=cfg->no; 
		num = idx / MAX_GPIO_REG_BITS;
		bit = idx % MAX_GPIO_REG_BITS;
		cfg->pullsel= (cur->pullsel[num].val & (1L << bit)) ? (1) : (0);
		cfg->din    = (cur->din[num].val & (1L << bit)) ? (1) : (0);
		cfg->dout   = (cur->dout[num].val & (1L << bit)) ? (1) : (0);
		cfg->pullen = (cur->pullen[num].val & (1L << bit)) ? (1) : (0);
		cfg->dir    = (cur->dir[num].val & (1L << bit)) ? (1) : (0);
/*		cfg->dinv   = (cur->dinv[num].val & (1L << bit)) ? (1) : (0);*/
		num = idx / MAX_GPIO_MODE_PER_REG;        
		bit = idx % MAX_GPIO_MODE_PER_REG;
		cfg->mode   = (cur->mode[num].val >> (GPIO_MODE_BITS*bit)) & mask;
    } else if (method == 1) {
		int idx=cfg->no; 
        cfg->pullsel= mt_get_gpio_pull_select(idx);
        cfg->din    = mt_get_gpio_in(idx);
        cfg->dout   = mt_get_gpio_out(idx);
        cfg->pullen = mt_get_gpio_pull_enable(idx);
        cfg->dir    = mt_get_gpio_dir(idx);
/*        cfg->dinv   = mt_get_gpio_inversion(idx);*/
        cfg->mode   = mt_get_gpio_mode(idx);
    }
}
/*---------------------------------------------------------------------------*/
void mt_gpio_dump_addr(void)
{
    unsigned int idx;
    struct mt_gpio_obj *obj = gpio_obj;
    GPIO_REGS *reg = obj->reg;

    GPIOMSG("# direction\n");
    for (idx = 0; idx < sizeof(reg->dir)/sizeof(reg->dir[0]); idx++)
        GPIOMSG("val[%2d] %p\nset[%2d] %p\nrst[%2d] %p\n", idx, &reg->dir[idx].val, idx, &reg->dir[idx].set, idx, &reg->dir[idx].rst);
    GPIOMSG("# pull enable\n");
    for (idx = 0; idx < sizeof(reg->pullen)/sizeof(reg->pullen[0]); idx++)
        GPIOMSG("val[%2d] %p\nset[%2d] %p\nrst[%2d] %p\n", idx, &reg->pullen[idx].val, idx, &reg->pullen[idx].set, idx, &reg->pullen[idx].rst);
    GPIOMSG("# pull select\n");
    for (idx = 0; idx < sizeof(reg->pullsel)/sizeof(reg->pullsel[0]); idx++)
        GPIOMSG("val[%2d] %p\nset[%2d] %p\nrst[%2d] %p\n", idx, &reg->pullsel[idx].val, idx, &reg->pullsel[idx].set, idx, &reg->pullsel[idx].rst);
    GPIOMSG("# data inversion\n");
/*    for (idx = 0; idx < sizeof(reg->dinv)/sizeof(reg->dinv[0]); idx++)
        GPIOMSG("val[%2d] %p\nset[%2d] %p\nrst[%2d] %p\n", idx, &reg->dinv[idx].val, idx, &reg->dinv[idx].set, idx, &reg->dinv[idx].rst);*/
    GPIOMSG("# data output\n");
    for (idx = 0; idx < sizeof(reg->dout)/sizeof(reg->dout[0]); idx++)
        GPIOMSG("val[%2d] %p\nset[%2d] %p\nrst[%2d] %p\n", idx, &reg->dout[idx].val, idx, &reg->dout[idx].set, idx, &reg->dout[idx].rst);
    GPIOMSG("# data input\n");
    for (idx = 0; idx < sizeof(reg->din)/sizeof(reg->din[0]); idx++)
        GPIOMSG("val[%2d] %p\n", idx, &reg->din[idx].val);
    GPIOMSG("# mode\n");
    for (idx = 0; idx < sizeof(reg->mode)/sizeof(reg->mode[0]); idx++)
        GPIOMSG("val[%2d] %p\nset[%2d] %p\nrst[%2d] %p\n", idx, &reg->mode[idx].val, idx, &reg->mode[idx].set, idx, &reg->mode[idx].rst);    
}
#endif
