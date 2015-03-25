
#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 1,
	.polling_mode_ps =1,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
//    .i2c_addr   = {0x90, 0x00, 0x00, 0x00},	/*STK31xx*/
    .als_level  = {5,  9, 36, 59, 82, 132, 205, 273, 500, 845, 1136, 1545, 2364, 4655, 6982},	/* als_code */
    .als_value  = {0, 10, 40, 65, 90, 145, 225, 300, 550, 930, 1250, 1700, 2600, 5120, 7680, 10240},    /* lux */
    .als_cmd_val = 0x49,	/*ALS_GAIN=1, IT_ALS=400ms*/
    .ps_cmd_val = 0x23,	/*SLP=30ms, IT_PS=0.2ms*/
    .ps_gain_setting = 0x09, /*PS_GAIN=8X */
    .ps_threshold_high = 27,
    .ps_threshold_low = 17,
};
struct alsps_hw_stk *get_cust_alsps_hw_stk(void) {
    return &cust_alsps_hw;
}

// 3 1000 0 27 17 100