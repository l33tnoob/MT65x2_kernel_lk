#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include "cust_gpio_usage.h"
#include "tpd.h"

void tpd_close_gpio(void)
{
    TPD_DEBUG("GT827 close gpio - original..\n");
    
    mt_set_gpio_mode(GPIO70, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO70, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO70, GPIO_OUT_ZERO);
}

void tpd_open_gpio(void)
{
    TPD_DEBUG("GT827 open gpio - original..\n");
    
    mt_set_gpio_mode(GPIO70, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO70, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO70, GPIO_OUT_ONE);
}