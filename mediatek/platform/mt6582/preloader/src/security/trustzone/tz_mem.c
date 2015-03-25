#include "tz_mem.h"
#include "tz_sec_reg.h"
#include "tz_utils.h"

extern void tz_emi_mpu_init(u32 start, u32 end);

void tz_sec_mem_init(u32 start, u32 end)
{    
    tz_emi_mpu_init(start, end);
}

void tz_sram_sec_init(void)
{
    /* Disable SRAMROM to avoid normal world's accesses */
    TZ_SET_FIELD(BOOTROM_PWR_CTRL, SRAMROM_SEC_SET0, PERMIT_S_RW_NS_BLOCK);
}
