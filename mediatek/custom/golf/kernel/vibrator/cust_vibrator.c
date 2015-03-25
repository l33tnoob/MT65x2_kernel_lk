#include <cust_vibrator.h>
#include <linux/types.h>

// Vibrator Volatge Selection
// 3'b000: 1.2V, 3'b001: 1.3V, 3'b010: 1.5V, 3'b011: 1.8V, 3'b100: 2.0V, 3'b101: 2.8V, 3'b110: 3.0V, 3'b111: 3.3V

/*
typedef enum MT65XX_POWER_VOL_TAG 
{
    VOL_DEFAULT, 
    VOL_0900 = 900,
    VOL_1000 = 1000,
    VOL_1100 = 1100,
    VOL_1200 = 1200,	
    VOL_1300 = 1300,   
    VOL_1350 = 1350,   
    VOL_1500 = 1500,    
    VOL_1800 = 1800,    
    VOL_2000 = 2000,
    VOL_2100 = 2100,
    VOL_2500 = 2500,    
    VOL_2800 = 2800, 
    VOL_3000 = 3000,
    VOL_3300 = 3300,
    VOL_3400 = 3400, 
    VOL_3500 = 3500,
    VOL_3600 = 3600        
} MT65XX_POWER_VOLTAGE;	
*/

static struct vibrator_hw cust_vibrator_hw = {
	.vib_timer = 50,
  #ifdef CUST_VIBR_LIMIT
	.vib_limit = 10,
  #endif
  	.vosel = VOL_2800,
};

struct vibrator_hw *get_cust_vibrator_hw(void)
{
    return &cust_vibrator_hw;
}

