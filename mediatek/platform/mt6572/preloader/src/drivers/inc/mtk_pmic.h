
#ifndef _MT65xx_PMIC_PL_SW_H_
#define _MT65xx_PMIC_PL_SW_H_

//==============================================================================
// PMIC6323 define
//==============================================================================
typedef enum {
    CHARGER_UNKNOWN = 0,
    STANDARD_HOST,          // USB : 450mA
    CHARGING_HOST,
    NONSTANDARD_CHARGER,    // AC : 450mA~1A 
    STANDARD_CHARGER,       // AC : ~1A
    APPLE_2_1A_CHARGER,     // 2.1A apple charger
    APPLE_1_0A_CHARGER,     // 1A apple charger
    APPLE_0_5A_CHARGER,     // 0.5A apple charger
} CHARGER_TYPE;


//==============================================================================
// PMIC6320 Register Index
//==============================================================================
//register number
#include <upmu_hw.h>


//==============================================================================
// PMIC6320 Status Code
//==============================================================================
#define PMIC_TEST_PASS               0x0000
#define PMIC_TEST_FAIL               0xB001
#define PMIC_EXCEED_I2C_FIFO_LENGTH  0xB002
#define PMIC_CHRDET_EXIST            0xB003
#define PMIC_CHRDET_NOT_EXIST        0xB004

//==============================================================================
// PMIC6320 Exported Function
//==============================================================================
extern CHARGER_TYPE mt_charger_type_detection(void);
extern U32 pmic6320_IsUsbCableIn (void);
extern int pmic_detect_powerkey(void);
extern void hw_set_cc(int cc_val);
extern int hw_check_battery(void);
extern void pl_charging(int en_chr);
extern void pl_kick_chr_wdt(void);
extern void pl_close_pre_chr_led(void);
extern void pl_hw_ulc_det(void);
extern U32 pmic_init (void);

#endif // _MT65xx_PMIC_PL_SW_H_

