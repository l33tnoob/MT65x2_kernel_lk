#ifndef __MTK_KEY_H__
#define __MTK_KEY_H__

#include <platform/mt_reg_base.h>

#define KP_STA		(KP_BASE + 0x0000)
#define KP_MEM1		(KP_BASE + 0x0004)
#define KP_MEM2		(KP_BASE + 0x0008)
#define KP_MEM3		(KP_BASE + 0x000c)
#define KP_MEM4		(KP_BASE + 0x0010)
#define KP_MEM5		(KP_BASE + 0x0014)
#define KP_DEBOUNCE	(KP_BASE + 0x0018)
#define KP_SCAN_TIMING	(KP_BASE + 0x001C)
#define KP_SEL		(KP_BASE + 0x0020)
#define KP_EN		(KP_BASE + 0x0024)

#define KPD_NUM_MEMS	5
#define KPD_MEM5_BITS	8

#define KPD_NUM_KEYS	72	/* 4 * 16 + KPD_MEM5_BITS */

void set_kpd_pmic_mode(void);
void disable_PMIC_kpd_clock(void);
void enable_PMIC_kpd_clock(void);
BOOL mtk_detect_key(unsigned short key);
BOOL mtk_detect_pmic_just_rst(void);

#endif /* __MTK_KEY_H__ */
