/******************************************************************************
 * mt_gpio_base.c - MTKLinux GPIO Device Driver
 * 
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *     This file provid the other drivers GPIO relative functions
 *
 ******************************************************************************/

#include <mach/sync_write.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>
#include <mach/mt_gpio_base.h>
//autogen
#include <mach/gpio_cfg.h>


static GPIO_REGS *gpio_reg = (GPIO_REGS*)(GPIO_BASE);
/*---------------------------------------------------------------------------*/
int mt_set_gpio_dir_base(unsigned long pin, unsigned long dir)
{
    unsigned long pos;
    unsigned long bit;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    if (dir == GPIO_DIR_IN)
        GPIO_SET_BITS((1L << bit), &reg->dir[pos].rst);
    else
        GPIO_SET_BITS((1L << bit), &reg->dir[pos].set);
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_dir_base(unsigned long pin)
{    
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    data = GPIO_RD32(&reg->dir[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_pull_enable_base(unsigned long pin, unsigned long enable)
{

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
/*---------------------------------------------------------------------------*/
int mt_get_gpio_pull_enable_base(unsigned long pin)
{
    unsigned long data;

    if((pin >= 67) && (pin < 87)){
	  return GPIO_PULL_EN_UNSUPPORTED;
    }
    else{   
	  data = GPIO_RD32(PULLEN_addr[pin].addr);

          return (((data & (1L << (PULLEN_offset[pin].offset))) != 0)? 1: 0);
    }        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_ies_base(unsigned long pin, unsigned long enable)
{
#if 0
    unsigned long pos;
    unsigned long bit;
    GPIO_REGS *reg = gpio_reg;


	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (enable == GPIO_IES_DISABLE)
		GPIO_SET_BITS((1L << bit), &reg->ies[pos].rst);
	else
		GPIO_SET_BITS((1L << bit), &reg->ies[pos].set);
#endif
    GPIOLOG("GPIO IES UNSUPPORTED\n");

    return GPIO_IES_UNSUPPORTED;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_ies_base(unsigned long pin)
{
#if 0
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	data = GPIO_RD32(&reg->ies[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
#endif
    GPIOLOG("GPIO IES UNSUPPORTED\n");

    return GPIO_IES_UNSUPPORTED;
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_pull_select_base(unsigned long pin, unsigned long select)
{

    if((pin >= 67) && (pin < 73)){
	  if (select == GPIO_PULL_DOWN)
		GPIO_SET_BITS((1L << (PD_offset[pin].offset)), PD_addr[pin].addr);
	  else
		GPIO_SET_BITS ((1L << (PU_offset[pin].offset)), PU_addr[pin].addr);
    }
    else if((pin >= 73) && (pin < 87)){
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
int mt_get_gpio_pull_select_base(unsigned long pin)
{
    unsigned long data;

    if((pin >= 67) && (pin < 73)){
	  data = GPIO_RD32(PU_addr[pin].addr);

    	  return (((data & (1L << (PU_offset[pin].offset))) != 0)? 1: 0);
    }
    else if((pin >= 73) && (pin < 87)){
	  return GPIO_PULL_UNSUPPORTED;
    }
    else{
	  data = GPIO_RD32(PULL_addr[pin].addr);

    	  return (((data & (1L << (PULL_offset[pin].offset))) != 0)? 1: 0);
    }
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_inversion_base(unsigned long pin, unsigned long enable)
{
		
	if (enable == GPIO_DATA_UNINV)
		GPIO_SET_BITS((1L << (DATAINV_offset[pin].offset)), DATAINV_addr[pin].addr + 8);
	else
		GPIO_SET_BITS ((1L << (DATAINV_offset[pin].offset)), DATAINV_addr[pin].addr + 4);
	
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_inversion_base(unsigned long pin)
{

    unsigned long data;

	data = GPIO_RD32(DATAINV_addr[pin].addr);

    return (((data & (1L << (DATAINV_offset[pin].offset))) != 0)? 1: 0);        

}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_out_base(unsigned long pin, unsigned long output)
{
    unsigned long pos;
    unsigned long bit;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    if (output == GPIO_OUT_ZERO)
        GPIO_SET_BITS((1L << bit), &reg->dout[pos].rst);
    else
        GPIO_SET_BITS((1L << bit), &reg->dout[pos].set);
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_out_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    data = GPIO_RD32(&reg->dout[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_in_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    data = GPIO_RD32(&reg->din[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_mode_base(unsigned long pin, unsigned long mode)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    //unsigned long mask = (1L << GPIO_MODE_BITS) - 1;    
    GPIO_REGS *reg = gpio_reg;

	
    pos = pin / MAX_GPIO_MODE_PER_REG;
    bit = pin % MAX_GPIO_MODE_PER_REG;

#if 0    
    data = GPIO_RD32(&reg->mode[pos].val);
    
    data &= ~(mask << (GPIO_MODE_BITS*bit));
    data |= (mode << (GPIO_MODE_BITS*bit));
    
    GPIO_WR32(&reg->mode[pos].val, data);
#endif

    data = ((1L << (GPIO_MODE_BITS*bit)) << 3) | (mode << (GPIO_MODE_BITS*bit));

    GPIO_WR32(&reg->mode[pos]._align1, data);
	
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_mode_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    unsigned long mask = (1L << GPIO_MODE_BITS) - 1;    
    GPIO_REGS *reg = gpio_reg;

	
    pos = pin / MAX_GPIO_MODE_PER_REG;
    bit = pin % MAX_GPIO_MODE_PER_REG;
    
    data = GPIO_RD32(&reg->mode[pos].val);
    
    return ((data >> (GPIO_MODE_BITS*bit)) & mask);
	
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef CONFIG_PM 
/*---------------------------------------------------------------------------*/
void mt_gpio_suspend(void)
{
    u32 val;
    u32 mask;

    mask = 0x0f;
    val = GPIO_RD32(0xF0208060);
    val &= ~(mask);
    val |= (0x0f & mask);
    GPIO_WR32(0xF0208060, val);

    mask = 0x0f;
    val = GPIO_RD32(0xF020A030);
    val &= ~(mask);
    val |= (0x0f & mask);
    GPIO_WR32(0xF020A030, val);

    mask = 0xf0;
    val = GPIO_RD32(0xF020B030);
    val &= ~(mask);
    val |= (0xf0 & mask);
    GPIO_WR32(0xF020B030, val);
}
/*---------------------------------------------------------------------------*/
void mt_gpio_resume(void)
{
    u32 val;
    u32 mask;

    mask = 0x0f;
    val = GPIO_RD32(0xF0208060);
    val &= ~(mask);
    val |= (0x00 & mask);
    GPIO_WR32(0xF0208060, val);

    mask = 0x0f;
    val = GPIO_RD32(0xF020A030);
    val &= ~(mask);
    val |= (0x00 & mask);
    GPIO_WR32(0xF020A030, val);

    mask = 0xf0;
    val = GPIO_RD32(0xF020B030);
    val &= ~(mask);
    val |= (0x00 & mask);
    GPIO_WR32(0xF020B030, val);
}
/*---------------------------------------------------------------------------*/
#endif /*CONFIG_PM*/
/*---------------------------------------------------------------------------*/
