
#ifndef _TZ_UTILS_H_
#define _TZ_UTILS_H_

#include <stdint.h>

typedef uint32_t u32;
typedef volatile unsigned int* P_U32;

#define READ_REGISTER_UINT32(reg) \
    (*(volatile uint32_t * const)(reg))

#define WRITE_REGISTER_UINT32(reg, val) \
    (*(volatile uint32_t * const)(reg)) = (val)

#define READ_REGISTER_UINT16(reg) \
    (*(volatile uint16_t * const)(reg))

#define WRITE_REGISTER_UINT16(reg, val) \
    (*(volatile uint16_t * const)(reg)) = (val)

#define READ_REGISTER_UINT8(reg) \
    (*(volatile uint8_t * const)(reg))

#define WRITE_REGISTER_UINT8(reg, val) \
    (*(volatile uint8_t * const)(reg)) = (val)

#define __raw_readb(REG)            READ_REGISTER_UINT8(REG)
#define __raw_readw(REG)            READ_REGISTER_UINT16(REG)
#define __raw_readl(REG)            READ_REGISTER_UINT32(REG)
#define __raw_writeb(VAL, REG)      WRITE_REGISTER_UINT8(REG,VAL)
#define __raw_writew(VAL, REG)      WRITE_REGISTER_UINT16(REG,VAL)
#define __raw_writel(VAL, REG)      WRITE_REGISTER_UINT32(REG,VAL)

#define readb __raw_readb
#define readw(addr) (__raw_readw(addr))
#define readl(addr) (__raw_readl(addr))

#define writeb __raw_writeb
#define writew(b,addr) __raw_writew(b,addr)
#define writel(b,addr) __raw_writel(b,addr)

#define set_field(r,f,v)                tz_set_field((volatile u32*)r,f,v)
#define get_field(r,f,v)                tz_get_field((volatile u32*)r,f,&v)

#define TZ_SET_FIELD(reg,field,val)     set_field(reg,field,val)
#define TZ_GET_FIELD(reg,field,val)     get_field(reg,field,val)


void tz_set_field(volatile u32 *reg, u32 field, u32 val);
void tz_get_field(volatile u32 *reg, u32 field, u32 *val);

#endif
