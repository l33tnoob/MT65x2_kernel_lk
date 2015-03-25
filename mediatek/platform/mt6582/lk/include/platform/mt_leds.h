#ifndef _MT65XX_LEDS_H
#define _MT65XX_LEDS_H

#define ISINK_CHOP_CLK
//#include <platform/cust_leds.h>
#include <cust_leds.h>

#define ERROR_BL_LEVEL 0xFFFFFFFF

enum led_color {
	LED_RED,
	LED_GREEN,
	LED_BLUE,
};

enum led_brightness {
	LED_OFF		= 0,
	LED_HALF	= 127,
	LED_FULL	= 255,
};

typedef enum{      
	ISINK_PWM_MODE = 0,      
	ISINK_BREATH_MODE = 1,      
	ISINK_REGISTER_MODE = 2
} MT65XX_PMIC_ISINK_MODE;

int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level);
void leds_battery_full_charging(void);
void leds_battery_low_charging(void);
void leds_battery_medium_charging(void);
void mt65xx_backlight_on(void);
void mt65xx_backlight_off(void);
void leds_init(void);
void leds_deinit(void);

#endif // !_MT65XX_LEDS_H
