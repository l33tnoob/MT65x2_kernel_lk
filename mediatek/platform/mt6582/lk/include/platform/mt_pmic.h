#ifndef _MT_PMIC_LK_SW_H_
#define _MT_PMIC_LK_SW_H_

#include <platform/mt_typedefs.h>

//==============================================================================
// PMIC6323 Define
//==============================================================================
#define PMIC6323_E1_CID_CODE    0x1023
#define PMIC6323_E2_CID_CODE    0x2023

#define VBAT_CHANNEL_NUMBER      7
#define ISENSE_CHANNEL_NUMBER	 6
#define VCHARGER_CHANNEL_NUMBER  4
#define VBATTEMP_CHANNEL_NUMBER  5

#define VOLTAGE_FULL_RANGE     1800
#define ADC_PRECISE         32768 // 10 bits


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
// PMIC6323 Exported Function
//==============================================================================
extern U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT);
extern U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT);
extern U32 pmic_IsUsbCableIn (void);
extern kal_bool upmu_is_chr_det(void);
extern int pmic_detect_powerkey(void);
extern int pmic_detect_powerkey(void);
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern void PMIC_DUMP_ALL_Register(void);
extern U32 pmic6323_init (void);
extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);
extern int get_bat_sense_volt(int times);
extern int get_i_sense_volt(int times);
extern int get_charger_volt(int times);
extern int get_tbat_volt(int times);
extern CHARGER_TYPE mt_charger_type_detection(void);

//==============================================================================
// PMIC6323 Status Code
//==============================================================================
#define PMIC_TEST_PASS               0x0000
#define PMIC_TEST_FAIL               0xB001
#define PMIC_EXCEED_I2C_FIFO_LENGTH  0xB002
#define PMIC_CHRDET_EXIST            0xB003
#define PMIC_CHRDET_NOT_EXIST        0xB004

//==============================================================================
// PMIC6323 Register Index
//==============================================================================
//register number
#include "upmu_hw.h"

//==============================================================================
// MT6323 APIs
//==============================================================================
#include "upmu_common.h"

#endif // _MT_PMIC_LK_SW_H_

