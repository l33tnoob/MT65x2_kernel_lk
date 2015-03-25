
#include "platform/tz_utils.h"

#ifndef __raw_readw
static inline u16 __raw_readw(const volatile void __iomem *addr)
{
	return *(const volatile u16 __force *) addr;
}
#endif

#ifndef __raw_readl
static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	return *(const volatile u32 __force *) addr;
}
#endif

#ifndef __raw_writew
static inline void __raw_writew(u16 b, volatile void __iomem *addr)
{
	*(volatile u16 __force *) addr = b;
}
#endif

#ifndef __raw_writel
static inline void __raw_writel(u32 b, volatile void __iomem *addr)
{
	*(volatile u32 __force *) addr = b;
}
#endif

static unsigned int tz_uffs(unsigned int x)
{
    return __builtin_ffs(x);
}

void tz_set_field(volatile u32 *reg, u32 field, u32 val)
{
    u32 tv = (u32)*reg;
    tv &= ~(field);
    tv |= ((val) << (tz_uffs((unsigned int)field) - 1));
    *reg = tv;
}

void tz_get_field(volatile u32 *reg, u32 field, u32 *val)
{
    u32 tv = (u32)*reg;
    *val = ((tv & (field)) >> (tz_uffs((unsigned int)field) - 1));
}

