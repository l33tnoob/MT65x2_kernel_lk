/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <debug.h>
#include <platform/mt_typedefs.h>
#include <platform/mt_rtc.h>
#include <platform/boot_mode.h>
#include <platform/mt_pmic_wrap_init.h>
#include <target/board.h>
#include <platform/mtk_wdt.h>
#include "../../kernel/core/include/mach/mt_rtc_hw.h"

#define DBG_RTC_C(x...) dprintf(CRITICAL, x)
#define DBG_RTC_I(x...) dprintf(INFO, x)
#define DBG_RTC_S(x...) dprintf(SPEW, x)

#define RTC_RELPWR_WHEN_XRST	1	/* BBPU = 0 when xreset_rstb goes low */

extern BOOT_ARGUMENT *g_boot_arg;

static U16 RTC_Read(U16 addr)
{
	U32 rdata=0;
	pwrap_read((U32)addr, &rdata);
	return (U16)rdata;
}

static void RTC_Write(U16 addr, U16 data)
{
	pwrap_write((U32)addr, (U32)data);
}



#define rtc_busy_wait()					\
do {							\
	while (RTC_Read(RTC_BBPU) & RTC_BBPU_CBUSY);	\
} while (0)

static unsigned long rtc_mktime(int yea, int mth, int dom, int hou, int min, int sec)
{
	unsigned long d1, d2, d3;

	mth -= 2;
	if (mth <= 0) {
		mth += 12;
		yea -= 1;
	}

	d1 = (yea - 1) * 365 + (yea / 4 - yea / 100 + yea / 400);
	d2 = (367 * mth / 12 - 30) + 59;
	d3 = d1 + d2 + (dom - 1) - 719162;

	return ((d3 * 24 + hou) * 60 + min) * 60 + sec;
}

static void rtc_write_trigger(void)
{
	RTC_Write(RTC_WRTGR, 1);
	rtc_busy_wait();
}

void rtc_writeif_unlock(void)
{
	RTC_Write(RTC_PROT, RTC_PROT_UNLOCK1);
	rtc_write_trigger();
	RTC_Write(RTC_PROT, RTC_PROT_UNLOCK2);
	rtc_write_trigger();
}

void rtc_writeif_lock(void)
{
	RTC_Write(RTC_PROT, 0);
	rtc_write_trigger();
}

void rtc_bbpu_power_down(void)
{
	U16 bbpu;

	/* pull PWRBB low */
	bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_PWREN;
	rtc_writeif_unlock();
	RTC_Write(RTC_BBPU, bbpu);
	rtc_write_trigger();
}

U16 rtc_rdwr_uart_bits(U16 *val)
{
	U16 pdn2;

	if (val) {
		pdn2 = RTC_Read(RTC_PDN2) & ~RTC_PDN2_UART_MASK;
		pdn2 |= (*val & (RTC_PDN2_UART_MASK >> RTC_PDN2_UART_SHIFT)) << RTC_PDN2_UART_SHIFT;
		RTC_Write(RTC_PDN2, pdn2);
		rtc_write_trigger();
	}

	return (RTC_Read(RTC_PDN2) & RTC_PDN2_UART_MASK) >> RTC_PDN2_UART_SHIFT;
}

bool rtc_boot_check(bool can_alarm_boot)
{
	U16 irqsta, pdn1, pdn2, spar0, spar1;

	irqsta = RTC_Read(RTC_IRQ_STA);	/* read clear */
	pdn1 = RTC_Read(RTC_PDN1);
	pdn2 = RTC_Read(RTC_PDN2);
	spar0 = RTC_Read(RTC_SPAR0);
	spar1 = RTC_Read(RTC_SPAR1);
	/*printf("irqsta = 0x%x, pdn1 = 0x%x, pdn2 = 0x%x, spar0 = 0x%x, spar1 = 0x%x\n",
	       irqsta, pdn1, pdn2, spar0, spar1);*/

	if (irqsta & RTC_IRQ_STA_AL) {
#if RTC_RELPWR_WHEN_XRST
		/* set AUTO bit because AUTO = 0 when PWREN = 1 and alarm occurs */
		U16 bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_AUTO;
		RTC_Write(RTC_BBPU, bbpu);
		rtc_write_trigger();
#endif

		if (pdn1 & RTC_PDN1_PWRON_TIME) {	/* power-on time is available */
			U16 now_sec, now_min, now_hou, now_dom, now_mth, now_yea;
			U16 irqen, sec, min, hou, dom, mth, yea;
			unsigned long now_time, time;

			now_sec = RTC_Read(RTC_TC_SEC);
			now_min = RTC_Read(RTC_TC_MIN);
			now_hou = RTC_Read(RTC_TC_HOU);
			now_dom = RTC_Read(RTC_TC_DOM);
			now_mth = RTC_Read(RTC_TC_MTH);
			now_yea = RTC_Read(RTC_TC_YEA) + RTC_MIN_YEAR;
			if (RTC_Read(RTC_TC_SEC) < now_sec) {	/* SEC has carried */
				now_sec = RTC_Read(RTC_TC_SEC);
				now_min = RTC_Read(RTC_TC_MIN);
				now_hou = RTC_Read(RTC_TC_HOU);
				now_dom = RTC_Read(RTC_TC_DOM);
				now_mth = RTC_Read(RTC_TC_MTH);
				now_yea = RTC_Read(RTC_TC_YEA) + RTC_MIN_YEAR;
			}

			sec = ((spar0 & RTC_SPAR0_PWRON_SEC_MASK) >> RTC_SPAR0_PWRON_SEC_SHIFT);
			min = ((spar1 & RTC_SPAR1_PWRON_MIN_MASK) >> RTC_SPAR1_PWRON_MIN_SHIFT);
			hou = ((spar1 & RTC_SPAR1_PWRON_HOU_MASK) >> RTC_SPAR1_PWRON_HOU_SHIFT);
			dom = ((spar1 & RTC_SPAR1_PWRON_DOM_MASK) >> RTC_SPAR1_PWRON_DOM_SHIFT);
			mth = ((pdn2  & RTC_PDN2_PWRON_MTH_MASK) >> RTC_PDN2_PWRON_MTH_SHIFT);
			yea = ((pdn2  & RTC_PDN2_PWRON_YEA_MASK) >> RTC_PDN2_PWRON_YEA_SHIFT) + RTC_MIN_YEAR;

			now_time = rtc_mktime(now_yea, now_mth, now_dom, now_hou, now_min, now_sec);
			time = rtc_mktime(yea, mth, dom, hou, min, sec);

			DBG_RTC_I("now = %d/%d/%d %d:%d:%d (%lu)\n",
			       now_yea, now_mth, now_dom, now_hou, now_min, now_sec, now_time);
			DBG_RTC_I("power-on = %d/%d/%d %d:%d:%d (%lu)\n",
			       yea, mth, dom, hou, min, sec, time);

			if (now_time >= time - 1 && now_time <= time + 4) {	/* power on */
				pdn1 &= ~(RTC_PDN1_PWRON_TIME | RTC_PDN1_FAC_RESET | RTC_PDN1_BYPASS_PWR);
				RTC_Write(RTC_PDN1, pdn1);
				RTC_Write(RTC_PDN2, pdn2 | RTC_PDN2_PWRON_ALARM);
				rtc_write_trigger();
				if (can_alarm_boot &&
				    !(pdn2 & RTC_PDN2_PWRON_LOGO)) {		/* no logo means ALARM_BOOT */
					g_boot_mode = ALARM_BOOT;
				}
				return true;
			} else if (now_time < time) {	/* set power-on alarm */
				RTC_Write(RTC_AL_YEA, yea - RTC_MIN_YEAR);
				RTC_Write(RTC_AL_MTH, (RTC_Read(RTC_AL_MTH)&RTC_NEW_SPARE3)|mth);
				RTC_Write(RTC_AL_DOM, (RTC_Read(RTC_AL_DOM)&RTC_NEW_SPARE1)|dom);
				RTC_Write(RTC_AL_HOU, (RTC_Read(RTC_AL_HOU)&RTC_NEW_SPARE_FG_MASK)|hou);
				RTC_Write(RTC_AL_MIN, min);
				RTC_Write(RTC_AL_SEC, sec);
				RTC_Write(RTC_AL_MASK, RTC_AL_MASK_DOW);	/* mask DOW */
				rtc_write_trigger();
				irqen = RTC_Read(RTC_IRQ_EN) | RTC_IRQ_EN_ONESHOT_AL;
				RTC_Write(RTC_IRQ_EN, irqen);
				rtc_write_trigger();
			}
		}
	}

	if ((pdn1 & RTC_PDN1_RECOVERY_MASK) == RTC_PDN1_FAC_RESET) {	/* factory data reset */
		RTC_Write(RTC_PDN1, pdn1 & ~RTC_PDN1_FAC_RESET);
		rtc_write_trigger();
		return true;
	}

	if (pdn1 & RTC_PDN1_BYPASS_PWR) {	/* bypass power key detection */
		RTC_Write(RTC_PDN1, pdn1 & ~RTC_PDN1_BYPASS_PWR);
		rtc_write_trigger();
		return true;
	}

	return false;
}

void Set_Clr_RTC_PDN1_bit13(bool flag)
{
	U16 pdn1;
	
	rtc_writeif_unlock();
	//use PDN1 bit13 for LK
	pdn1 = RTC_Read(RTC_PDN1);
	if(flag==true)
		pdn1 = pdn1 | RTC_PDN1_FAST_BOOT;
	else if(flag==false)
		pdn1 = pdn1 & ~RTC_PDN1_FAST_BOOT;
	RTC_Write(RTC_PDN1, pdn1);
	rtc_write_trigger();
}

bool Check_RTC_PDN1_bit13(void)
{
	U16 pdn1;

	pdn1 = RTC_Read(RTC_PDN1);
	if(pdn1 & RTC_PDN1_FAST_BOOT)
		return true;
	else
		return false;
}

bool Check_RTC_Recovery_Mode(void)
{
	U16 pdn1;

	pdn1 = RTC_Read(RTC_PDN1);
	if( (pdn1 & RTC_PDN1_RECOVERY_MASK)==RTC_PDN1_FAC_RESET )
		return true;
	else
		return false;
}
#if 1
bool rtc_2sec_boot_check(void)
{
	int boot_reason;

	if (g_boot_arg->maggic_number == BOOT_ARGUMENT_MAGIC) {
		boot_reason = g_boot_arg->boot_reason;
	
		if (boot_reason == BR_2SEC_REBOOT)
			return true;
	}
	
	return false;
}
#endif
/*
extern kal_bool pmic_chrdet_status(void);
#ifdef CFG_POWER_CHARGING
void mt6575_power_off(void)
{
	printf("mt6575_power_off\n");

	rtc_bbpu_power_down();

	while (1) {
		printf("mt6575_power_off : check charger\n");
		if (pmic_chrdet_status() == KAL_TRUE)
			mtk_arch_reset(0);
	}
}
#endif
*/


void Set_RTC_Recovery_Mode(bool flag)
{
   U16 pdn1;
   rtc_writeif_unlock();
   pdn1 = RTC_Read(RTC_PDN1);
   if(flag==true)
      pdn1 = pdn1 | RTC_PDN1_FAC_RESET;
   else if(flag==false)
      pdn1 = pdn1 & ~RTC_PDN1_FAC_RESET;
   RTC_Write(RTC_PDN1, pdn1);
   rtc_write_trigger();
   DBG_RTC_I("Set_RTC_Fastboot_Mode\n");
}

