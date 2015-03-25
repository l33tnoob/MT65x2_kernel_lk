#ifndef __MT_RTC_H__
#define __MT_RTC_H__

//#include <asm/arch/mt65xx.h>
#include <platform/mt_pmic.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_typedefs.h>
#include <sys/types.h>

/* we map HW YEA 0 (2000) to 1968 not 1970 because 2000 is the leap year */
#define RTC_MIN_YEAR		1968
#define RTC_NUM_YEARS		128
//#define RTC_MAX_YEAR		(RTC_MIN_YEAR + RTC_NUM_YEARS - 1)


extern void rtc_writeif_unlock(void);
extern void rtc_writeif_lock(void);

extern void rtc_bbpu_power_down(void);

extern U16 rtc_rdwr_uart_bits(U16 *val);

extern bool rtc_boot_check(bool can_alarm_boot);

extern void Set_Clr_RTC_PDN1_bit13(bool flag);
extern bool Check_RTC_PDN1_bit13(void);
extern bool Check_RTC_Recovery_Mode(void);
extern void Set_RTC_Recovery_Mode(bool flag);
#ifndef NO_POWER_OFF
extern void mt6575_power_off(void);
#else
#define mt6575_power_off()	do {} while (0)
#endif

#endif /* __MT_RTC_H__ */
