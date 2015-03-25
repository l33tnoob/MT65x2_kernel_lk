#ifndef _MT_GPIO_BASE_H_
#define _MT_GPIO_BASE_H_

#include <mach/sync_write.h>
#include <mach/gpio_const.h>

#define GPIO_WR32(addr, data)   mt65xx_reg_sync_writel(data, addr)
#define GPIO_RD32(addr)         __raw_readl(addr)
//#define GPIO_SET_BITS(BIT,REG)   ((*(volatile unsigned long*)(REG)) = (unsigned long)(BIT))
//#define GPIO_CLR_BITS(BIT,REG)   ((*(volatile unsigned long*)(REG)) &= ~((unsigned long)(BIT)))
#define GPIO_SET_BITS(BIT,REG)   GPIO_WR32(REG, (unsigned long)(BIT))
#define GPIO_CLR_BITS(BIT,REG)   GPIO_WR32(REG,GPIO_RD32(REG) & ~((unsigned long)(BIT)))

/*----------------------------------------------------------------------------*/
typedef struct {         /*FIXME: check GPIO spec*/   
    unsigned long val;        
    unsigned long set;
    unsigned long rst;
    unsigned long _align1;
} VAL_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    VAL_REGS    dir[5];             /*0x0000 ~ 0x004F: 80  bytes*/
    u8          rsv00[176];         /*0x0050 ~ 0x00FF: 176 bytes*/    
    VAL_REGS    dout[5];            /*0x0100 ~ 0x014F: 80  bytes*/
    u8          rsv01[176];         /*0x0150 ~ 0x01FF: 176 bytes*/
    VAL_REGS    din[5];             /*0x0200 ~ 0x024F: 80  bytes*/
    u8          rsv02[176];         /*0x0250 ~ 0x02FF: 176 bytes*/
    VAL_REGS    mode[20];           /*0x0300 ~ 0x043F: 320 bytes*/  
} GPIO_REGS;

#endif //_MT_GPIO_BASE_H_
