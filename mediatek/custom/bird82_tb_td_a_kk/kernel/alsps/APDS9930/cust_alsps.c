#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
//#include <mach/mt6577_pm_ldo.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
    /*Lenovo-sw chenlj2 add 2011-06-03,modify parameter below two lines*/
    //.als_level  = { 4, 40,  80,   120,   160, 250,  400, 800, 1200,  1600, 2000, 3000, 5000, 10000, 65535},
    //.als_value  = {10, 20,20,  120, 120, 280,  280,  280, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
    .als_level  = { 5, 10,  25,   50,  100, 150,  200, 400, 1000,  1500, 2000, 3000, 5000, 8000, 10000},
    .als_value  = {10, 50,  100,  150, 200, 250,  280,  280, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
    .ps_threshold_high = 120,
    .ps_threshold_low = 100,
    .ps_threshold = 900,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
int APDS9930_CMM_PPCOUNT_VALUE = 0x08;
int APDS9930_CMM_CONTROL_VALUE = 0xE4;
int ZOOM_TIME = 4;
