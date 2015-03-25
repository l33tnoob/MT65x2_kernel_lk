/******************************************************************************
 * gpio_init.c - MT6516 Linux GPIO Device Driver
 * 
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *     default GPIO init
 *
 ******************************************************************************/

#include <platform/mt_gpio.h>

#if defined(MT_GPIO_ENABLED)
#include <cust_gpio_boot.h>
#include <cust_power.h>
#endif

#include <platform/mt_reg_base.h>
#include <string.h>

#include <debug.h>
#define GPIO_INIT_DEBUG 1        
/*----------------------------------------------------------------------------*/
#define GPIOTAG "[GPIO] "
#define GPIODBG(fmt, arg...)    printf(GPIOTAG "%s: " fmt, __FUNCTION__ ,##arg)
#define GPIOERR(fmt, arg...)    printf(GPIOTAG "%s: " fmt, __FUNCTION__ ,##arg)
#define GPIOVER(fmt, arg...)    printf(GPIOTAG "%s: " fmt, __FUNCTION__ ,##arg)

#define GPIO_WR32(addr, data)   DRV_WriteReg16(addr,data)
#define GPIO_RD32(addr)         DRV_Reg16(addr)

/*----------------------------------------------------------------------------*/
#if !defined(MT_GPIO_ENABLED)
void mt_gpio_set_default(void)
{   
	return;
}
#else
u16 gpio_init_dir_data[] = {
    ((GPIO0_DIR       <<  0) |(GPIO1_DIR       <<  1) |(GPIO2_DIR       <<  2) |(GPIO3_DIR       <<  3) |
     (GPIO4_DIR       <<  4) |(GPIO5_DIR       <<  5) |(GPIO6_DIR       <<  6) |(GPIO7_DIR       <<  7) |
     (GPIO8_DIR       <<  8) |(GPIO9_DIR       <<  9) |(GPIO10_DIR      << 10) |(GPIO11_DIR      << 11) |
     (GPIO12_DIR      << 12) |(GPIO13_DIR      << 13) |(GPIO14_DIR      << 14) |(GPIO15_DIR      << 15)),

    ((GPIO16_DIR      <<  0) |(GPIO17_DIR      <<  1) |(GPIO18_DIR      <<  2) |(GPIO19_DIR      <<  3) |
     (GPIO20_DIR      <<  4) |(GPIO21_DIR      <<  5) |(GPIO22_DIR      <<  6) |(GPIO23_DIR      <<  7) |
     (GPIO24_DIR      <<  8) |(GPIO25_DIR      <<  9) |(GPIO26_DIR      << 10) |(GPIO27_DIR      << 11) |
     (GPIO28_DIR      << 12) |(GPIO29_DIR      << 13) |(GPIO30_DIR      << 14) |(GPIO31_DIR      << 15)),

    ((GPIO32_DIR      <<  0) |(GPIO33_DIR      <<  1) |(GPIO34_DIR      <<  2) |(GPIO35_DIR      <<  3) |
     (GPIO36_DIR      <<  4) |(GPIO37_DIR      <<  5) |(GPIO38_DIR      <<  6) |(GPIO39_DIR      <<  7) |
     (GPIO40_DIR      <<  8) |(GPIO41_DIR      <<  9) |(GPIO42_DIR      << 10) |(GPIO43_DIR      << 11) |
     (GPIO44_DIR      << 12) |(GPIO45_DIR      << 13) |(GPIO46_DIR      << 14) |(GPIO47_DIR      << 15)),

    ((GPIO48_DIR      <<  0) |(GPIO49_DIR      <<  1) |(GPIO50_DIR      <<  2) |(GPIO51_DIR      <<  3) |
     (GPIO52_DIR      <<  4) |(GPIO53_DIR      <<  5) |(GPIO54_DIR      <<  6) |(GPIO55_DIR      <<  7) |
     (GPIO56_DIR      <<  8) |(GPIO57_DIR      <<  9) |(GPIO58_DIR      << 10) |(GPIO59_DIR      << 11) |
     (GPIO60_DIR      << 12) |(GPIO61_DIR      << 13) |(GPIO62_DIR      << 14) |(GPIO63_DIR      << 15)),

    ((GPIO64_DIR      <<  0) |(GPIO65_DIR      <<  1) |(GPIO66_DIR      <<  2) |(GPIO67_DIR      <<  3) |
     (GPIO68_DIR      <<  4) |(GPIO69_DIR      <<  5) |(GPIO70_DIR      <<  6) |(GPIO71_DIR      <<  7) |
     (GPIO72_DIR      <<  8) |(GPIO73_DIR      <<  9) |(GPIO74_DIR      << 10) |(GPIO75_DIR      << 11) |
     (GPIO76_DIR      << 12) |(GPIO77_DIR      << 13) |(GPIO78_DIR      << 14) |(GPIO79_DIR      << 15)),

    ((GPIO80_DIR      <<  0) |(GPIO81_DIR      <<  1) |(GPIO82_DIR      <<  2) |(GPIO83_DIR      <<  3) |
     (GPIO84_DIR      <<  4) |(GPIO85_DIR      <<  5) |(GPIO86_DIR      <<  6) |(GPIO87_DIR      <<  7) |
     (GPIO88_DIR      <<  8) |(GPIO89_DIR      <<  9) |(GPIO90_DIR      << 10) |(GPIO91_DIR      << 11) |
     (GPIO92_DIR      << 12) |(GPIO93_DIR      << 13) |(GPIO94_DIR      << 14) |(GPIO95_DIR      << 15)),

    ((GPIO96_DIR      <<  0) |(GPIO97_DIR      <<  1) |(GPIO98_DIR      <<  2) |(GPIO99_DIR      <<  3) |
     (GPIO100_DIR     <<  4) |(GPIO101_DIR     <<  5) |(GPIO102_DIR     <<  6) |(GPIO103_DIR     <<  7) |
     (GPIO104_DIR     <<  8) |(GPIO105_DIR     <<  9) |(GPIO106_DIR     << 10) |(GPIO107_DIR     << 11) |
     (GPIO108_DIR     << 12) |(GPIO109_DIR     << 13) |(GPIO110_DIR     << 14) |(GPIO111_DIR     << 15)),

    ((GPIO112_DIR     <<  0) |(GPIO113_DIR     <<  1) |(GPIO114_DIR     <<  2) |(GPIO115_DIR     <<  3) |
     (GPIO116_DIR     <<  4) |(GPIO117_DIR     <<  5) |(GPIO118_DIR     <<  6) |(GPIO119_DIR     <<  7) |
     (GPIO120_DIR     <<  8) |(GPIO121_DIR     <<  9) |(GPIO122_DIR     << 10) |(GPIO123_DIR     << 11) |
     (GPIO124_DIR     << 12) |(GPIO125_DIR     << 13) |(GPIO126_DIR     << 14) |(GPIO127_DIR     << 15)),

    ((GPIO128_DIR     <<  0) |(GPIO129_DIR     <<  1) |(GPIO130_DIR     <<  2) |(GPIO131_DIR     <<  3) |
     (GPIO132_DIR     <<  4) |(GPIO133_DIR     <<  5) |(GPIO134_DIR     <<  6) |(GPIO135_DIR     <<  7) |
     (GPIO136_DIR     <<  8) |(GPIO137_DIR     <<  9) |(GPIO138_DIR     << 10) |(GPIO139_DIR     << 11) |
     (GPIO140_DIR     << 12) |(GPIO141_DIR     << 13) |(GPIO142_DIR     << 14) |(GPIO143_DIR     << 15)),

    ((GPIO144_DIR     <<  0) |(GPIO145_DIR     <<  1) |(GPIO146_DIR     <<  2) |(GPIO147_DIR     <<  3) |
     (GPIO148_DIR     <<  4) |(GPIO149_DIR     <<  5) |(GPIO150_DIR     <<  6) |(GPIO151_DIR     <<  7) |
     (GPIO152_DIR     <<  8) |(GPIO153_DIR     <<  9) |(GPIO154_DIR     << 10) |(GPIO155_DIR     << 11) |
     (GPIO156_DIR     << 12) |(GPIO157_DIR     << 13) |(GPIO158_DIR     << 14) |(GPIO159_DIR     << 15)),

    ((GPIO160_DIR     <<  0) |(GPIO161_DIR     <<  1) |(GPIO162_DIR     <<  2) |(GPIO163_DIR     <<  3) |
     (GPIO164_DIR     <<  4) |(GPIO165_DIR     <<  5) |(GPIO166_DIR     <<  6) |(GPIO167_DIR     <<  7) |
     (GPIO168_DIR     <<  8)),
}; /*end of gpio_init_dir_data*/
/*----------------------------------------------------------------------------*/
u16 gpio_init_pullen_data[] = {
    ((GPIO0_PULLEN    <<  0) |(GPIO1_PULLEN    <<  1) |(GPIO2_PULLEN    <<  2) |(GPIO3_PULLEN    <<  3) |
     (GPIO4_PULLEN    <<  4) |(GPIO5_PULLEN    <<  5) |(GPIO6_PULLEN    <<  6) |(GPIO7_PULLEN    <<  7) |
     (GPIO8_PULLEN    <<  8) |(GPIO9_PULLEN    <<  9) |(GPIO10_PULLEN   << 10) |(GPIO11_PULLEN   << 11) |
     (GPIO12_PULLEN   << 12) |(GPIO13_PULLEN   << 13) |(GPIO14_PULLEN   << 14) |(GPIO15_PULLEN   << 15)),

    ((GPIO16_PULLEN   <<  0) |(GPIO17_PULLEN   <<  1) |(GPIO18_PULLEN   <<  2) |(GPIO19_PULLEN   <<  3) |
     (GPIO20_PULLEN   <<  4) |(GPIO21_PULLEN   <<  5) |(GPIO22_PULLEN   <<  6) |(GPIO23_PULLEN   <<  7) |
     (GPIO24_PULLEN   <<  8) |(GPIO25_PULLEN   <<  9) |(GPIO26_PULLEN   << 10) |(GPIO27_PULLEN   << 11) |
     (GPIO28_PULLEN   << 12) |(GPIO29_PULLEN   << 13) |(GPIO30_PULLEN   << 14) |(GPIO31_PULLEN   << 15)),

    ((GPIO32_PULLEN   <<  0) |(GPIO33_PULLEN   <<  1) |(GPIO34_PULLEN   <<  2) |(GPIO35_PULLEN   <<  3) |
     (GPIO36_PULLEN   <<  4) |(GPIO37_PULLEN   <<  5) |(GPIO38_PULLEN   <<  6) |(GPIO39_PULLEN   <<  7) |
     (GPIO40_PULLEN   <<  8) |(GPIO41_PULLEN   <<  9) |(GPIO42_PULLEN   << 10) |(GPIO43_PULLEN   << 11) |
     (GPIO44_PULLEN   << 12) |(GPIO45_PULLEN   << 13) |(GPIO46_PULLEN   << 14) |(GPIO47_PULLEN   << 15)),

    ((GPIO48_PULLEN   <<  0) |(GPIO49_PULLEN   <<  1) |(GPIO50_PULLEN   <<  2) |(GPIO51_PULLEN   <<  3) |
     (GPIO52_PULLEN   <<  4) |(GPIO53_PULLEN   <<  5) |(GPIO54_PULLEN   <<  6) |(GPIO55_PULLEN   <<  7) |
     (GPIO56_PULLEN   <<  8) |(GPIO57_PULLEN   <<  9) |(GPIO58_PULLEN   << 10) |(GPIO59_PULLEN   << 11) |
     (GPIO60_PULLEN   << 12) |(GPIO61_PULLEN   << 13) |(GPIO62_PULLEN   << 14) |(GPIO63_PULLEN   << 15)),

    ((GPIO64_PULLEN   <<  0) |(GPIO65_PULLEN   <<  1) |(GPIO66_PULLEN   <<  2) |(GPIO67_PULLEN   <<  3) |
     (GPIO68_PULLEN   <<  4) |(GPIO69_PULLEN   <<  5) |(GPIO70_PULLEN   <<  6) |(GPIO71_PULLEN   <<  7) |
     (GPIO72_PULLEN   <<  8) |(GPIO73_PULLEN   <<  9) |(GPIO74_PULLEN   << 10) |(GPIO75_PULLEN   << 11) |
     (GPIO76_PULLEN   << 12) |(GPIO77_PULLEN   << 13) |(GPIO78_PULLEN   << 14) |(GPIO79_PULLEN   << 15)),

    ((GPIO80_PULLEN   <<  0) |(GPIO81_PULLEN   <<  1) |(GPIO82_PULLEN   <<  2) |(GPIO83_PULLEN   <<  3) |
     (GPIO84_PULLEN   <<  4) |(GPIO85_PULLEN   <<  5) |(GPIO86_PULLEN   <<  6) |(GPIO87_PULLEN   <<  7) |
     (GPIO88_PULLEN   <<  8) |(GPIO89_PULLEN   <<  9) |(GPIO90_PULLEN   << 10) |(GPIO91_PULLEN   << 11) |
     (GPIO92_PULLEN   << 12) |(GPIO93_PULLEN   << 13) |(GPIO94_PULLEN   << 14) |(GPIO95_PULLEN   << 15)),

    ((GPIO96_PULLEN   <<  0) |(GPIO97_PULLEN   <<  1) |(GPIO98_PULLEN   <<  2) |(GPIO99_PULLEN   <<  3) |
     (GPIO100_PULLEN  <<  4) |(GPIO101_PULLEN  <<  5) |(GPIO102_PULLEN  <<  6) |(GPIO103_PULLEN  <<  7) |
     (GPIO104_PULLEN  <<  8) |(GPIO105_PULLEN  <<  9) |(GPIO106_PULLEN  << 10) |(GPIO107_PULLEN  << 11) |
     (GPIO108_PULLEN  << 12) |(GPIO109_PULLEN  << 13) |(GPIO110_PULLEN  << 14) |(GPIO111_PULLEN  << 15)),

    ((GPIO112_PULLEN  <<  0) |(GPIO113_PULLEN  <<  1) |(GPIO114_PULLEN  <<  2) |(GPIO115_PULLEN  <<  3) |
     (GPIO116_PULLEN  <<  4) |(GPIO117_PULLEN  <<  5) |(GPIO118_PULLEN  <<  6) |(GPIO119_PULLEN  <<  7) |
     (GPIO120_PULLEN  <<  8) |(GPIO121_PULLEN  <<  9) |(GPIO122_PULLEN  << 10) |(GPIO123_PULLEN  << 11) |
     (GPIO124_PULLEN  << 12) |(GPIO125_PULLEN  << 13) |(GPIO126_PULLEN  << 14) |(GPIO127_PULLEN  << 15)),

    ((GPIO128_PULLEN  <<  0) |(GPIO129_PULLEN  <<  1) |(GPIO130_PULLEN  <<  2) |(GPIO131_PULLEN  <<  3) |
     (GPIO132_PULLEN  <<  4) |(GPIO133_PULLEN  <<  5) |(GPIO134_PULLEN  <<  6) |(GPIO135_PULLEN  <<  7) |
     (GPIO136_PULLEN  <<  8) |(GPIO137_PULLEN  <<  9) |(GPIO138_PULLEN  << 10) |(GPIO139_PULLEN  << 11) |
     (GPIO140_PULLEN  << 12) |(GPIO141_PULLEN  << 13) |(GPIO142_PULLEN  << 14) |(GPIO143_PULLEN  << 15)),

    ((GPIO144_PULLEN  <<  0) |(GPIO145_PULLEN  <<  1) |(GPIO146_PULLEN  <<  2) |(GPIO147_PULLEN  <<  3) |
     (GPIO148_PULLEN  <<  4) |(GPIO149_PULLEN  <<  5) |(GPIO150_PULLEN  <<  6) |(GPIO151_PULLEN  <<  7) |
     (GPIO152_PULLEN  <<  8) |(GPIO153_PULLEN  <<  9) |(GPIO154_PULLEN  << 10) |(GPIO155_PULLEN  << 11) |
     (GPIO156_PULLEN  << 12) |(GPIO157_PULLEN  << 13) |(GPIO158_PULLEN  << 14) |(GPIO159_PULLEN  << 15)),

    ((GPIO160_PULLEN  <<  0) |(GPIO161_PULLEN  <<  1) |(GPIO162_PULLEN  <<  2) |(GPIO163_PULLEN  <<  3) |
     (GPIO164_PULLEN  <<  4) |(GPIO165_PULLEN  <<  5) |(GPIO166_PULLEN  <<  6) |(GPIO167_PULLEN  <<  7) |
     (GPIO168_PULLEN  <<  8)),
}; /*end of gpio_init_pullen_data*/
/*----------------------------------------------------------------------------*/
u16 gpio_init_pullsel_data[] = {
    ((GPIO0_PULL      <<  0) |(GPIO1_PULL      <<  1) |(GPIO2_PULL      <<  2) |(GPIO3_PULL      <<  3) |
     (GPIO4_PULL      <<  4) |(GPIO5_PULL      <<  5) |(GPIO6_PULL      <<  6) |(GPIO7_PULL      <<  7) |
     (GPIO8_PULL      <<  8) |(GPIO9_PULL      <<  9) |(GPIO10_PULL     << 10) |(GPIO11_PULL     << 11) |
     (GPIO12_PULL     << 12) |(GPIO13_PULL     << 13) |(GPIO14_PULL     << 14) |(GPIO15_PULL     << 15)),

    ((GPIO16_PULL     <<  0) |(GPIO17_PULL     <<  1) |(GPIO18_PULL     <<  2) |(GPIO19_PULL     <<  3) |
     (GPIO20_PULL     <<  4) |(GPIO21_PULL     <<  5) |(GPIO22_PULL     <<  6) |(GPIO23_PULL     <<  7) |
     (GPIO24_PULL     <<  8) |(GPIO25_PULL     <<  9) |(GPIO26_PULL     << 10) |(GPIO27_PULL     << 11) |
     (GPIO28_PULL     << 12) |(GPIO29_PULL     << 13) |(GPIO30_PULL     << 14) |(GPIO31_PULL     << 15)),

    ((GPIO32_PULL     <<  0) |(GPIO33_PULL     <<  1) |(GPIO34_PULL     <<  2) |(GPIO35_PULL     <<  3) |
     (GPIO36_PULL     <<  4) |(GPIO37_PULL     <<  5) |(GPIO38_PULL     <<  6) |(GPIO39_PULL     <<  7) |
     (GPIO40_PULL     <<  8) |(GPIO41_PULL     <<  9) |(GPIO42_PULL     << 10) |(GPIO43_PULL     << 11) |
     (GPIO44_PULL     << 12) |(GPIO45_PULL     << 13) |(GPIO46_PULL     << 14) |(GPIO47_PULL     << 15)),

    ((GPIO48_PULL     <<  0) |(GPIO49_PULL     <<  1) |(GPIO50_PULL     <<  2) |(GPIO51_PULL     <<  3) |
     (GPIO52_PULL     <<  4) |(GPIO53_PULL     <<  5) |(GPIO54_PULL     <<  6) |(GPIO55_PULL     <<  7) |
     (GPIO56_PULL     <<  8) |(GPIO57_PULL     <<  9) |(GPIO58_PULL     << 10) |(GPIO59_PULL     << 11) |
     (GPIO60_PULL     << 12) |(GPIO61_PULL     << 13) |(GPIO62_PULL     << 14) |(GPIO63_PULL     << 15)),

    ((GPIO64_PULL     <<  0) |(GPIO65_PULL     <<  1) |(GPIO66_PULL     <<  2) |(GPIO67_PULL     <<  3) |
     (GPIO68_PULL     <<  4) |(GPIO69_PULL     <<  5) |(GPIO70_PULL     <<  6) |(GPIO71_PULL     <<  7) |
     (GPIO72_PULL     <<  8) |(GPIO73_PULL     <<  9) |(GPIO74_PULL     << 10) |(GPIO75_PULL     << 11) |
     (GPIO76_PULL     << 12) |(GPIO77_PULL     << 13) |(GPIO78_PULL     << 14) |(GPIO79_PULL     << 15)),

    ((GPIO80_PULL     <<  0) |(GPIO81_PULL     <<  1) |(GPIO82_PULL     <<  2) |(GPIO83_PULL     <<  3) |
     (GPIO84_PULL     <<  4) |(GPIO85_PULL     <<  5) |(GPIO86_PULL     <<  6) |(GPIO87_PULL     <<  7) |
     (GPIO88_PULL     <<  8) |(GPIO89_PULL     <<  9) |(GPIO90_PULL     << 10) |(GPIO91_PULL     << 11) |
     (GPIO92_PULL     << 12) |(GPIO93_PULL     << 13) |(GPIO94_PULL     << 14) |(GPIO95_PULL     << 15)),

    ((GPIO96_PULL     <<  0) |(GPIO97_PULL     <<  1) |(GPIO98_PULL     <<  2) |(GPIO99_PULL     <<  3) |
     (GPIO100_PULL    <<  4) |(GPIO101_PULL    <<  5) |(GPIO102_PULL    <<  6) |(GPIO103_PULL    <<  7) |
     (GPIO104_PULL    <<  8) |(GPIO105_PULL    <<  9) |(GPIO106_PULL    << 10) |(GPIO107_PULL    << 11) |
     (GPIO108_PULL    << 12) |(GPIO109_PULL    << 13) |(GPIO110_PULL    << 14) |(GPIO111_PULL    << 15)),

    ((GPIO112_PULL    <<  0) |(GPIO113_PULL    <<  1) |(GPIO114_PULL    <<  2) |(GPIO115_PULL    <<  3) |
     (GPIO116_PULL    <<  4) |(GPIO117_PULL    <<  5) |(GPIO118_PULL    <<  6) |(GPIO119_PULL    <<  7) |
     (GPIO120_PULL    <<  8) |(GPIO121_PULL    <<  9) |(GPIO122_PULL    << 10) |(GPIO123_PULL    << 11) |
     (GPIO124_PULL    << 12) |(GPIO125_PULL    << 13) |(GPIO126_PULL    << 14) |(GPIO127_PULL    << 15)),

    ((GPIO128_PULL    <<  0) |(GPIO129_PULL    <<  1) |(GPIO130_PULL    <<  2) |(GPIO131_PULL    <<  3) |
     (GPIO132_PULL    <<  4) |(GPIO133_PULL    <<  5) |(GPIO134_PULL    <<  6) |(GPIO135_PULL    <<  7) |
     (GPIO136_PULL    <<  8) |(GPIO137_PULL    <<  9) |(GPIO138_PULL    << 10) |(GPIO139_PULL    << 11) |
     (GPIO140_PULL    << 12) |(GPIO141_PULL    << 13) |(GPIO142_PULL    << 14) |(GPIO143_PULL    << 15)),

    ((GPIO144_PULL    <<  0) |(GPIO145_PULL    <<  1) |(GPIO146_PULL    <<  2) |(GPIO147_PULL    <<  3) |
     (GPIO148_PULL    <<  4) |(GPIO149_PULL    <<  5) |(GPIO150_PULL    <<  6) |(GPIO151_PULL    <<  7) |
     (GPIO152_PULL    <<  8) |(GPIO153_PULL    <<  9) |(GPIO154_PULL    << 10) |(GPIO155_PULL    << 11) |
     (GPIO156_PULL    << 12) |(GPIO157_PULL    << 13) |(GPIO158_PULL    << 14) |(GPIO159_PULL    << 15)),

    ((GPIO160_PULL    <<  0) |(GPIO161_PULL    <<  1) |(GPIO162_PULL    <<  2) |(GPIO163_PULL    <<  3) |
     (GPIO164_PULL    <<  4) |(GPIO165_PULL    <<  5) |(GPIO166_PULL    <<  6) |(GPIO167_PULL    <<  7) |
     (GPIO168_PULL    <<  8)),
}; /*end of gpio_init_pullsel_data*/
/*----------------------------------------------------------------------------*/
u16 gpio_init_dout_data[] = {
    ((GPIO0_DATAOUT   <<  0) |(GPIO1_DATAOUT   <<  1) |(GPIO2_DATAOUT   <<  2) |(GPIO3_DATAOUT   <<  3) |
     (GPIO4_DATAOUT   <<  4) |(GPIO5_DATAOUT   <<  5) |(GPIO6_DATAOUT   <<  6) |(GPIO7_DATAOUT   <<  7) |
     (GPIO8_DATAOUT   <<  8) |(GPIO9_DATAOUT   <<  9) |(GPIO10_DATAOUT  << 10) |(GPIO11_DATAOUT  << 11) |
     (GPIO12_DATAOUT  << 12) |(GPIO13_DATAOUT  << 13) |(GPIO14_DATAOUT  << 14) |(GPIO15_DATAOUT  << 15)),

    ((GPIO16_DATAOUT  <<  0) |(GPIO17_DATAOUT  <<  1) |(GPIO18_DATAOUT  <<  2) |(GPIO19_DATAOUT  <<  3) |
     (GPIO20_DATAOUT  <<  4) |(GPIO21_DATAOUT  <<  5) |(GPIO22_DATAOUT  <<  6) |(GPIO23_DATAOUT  <<  7) |
     (GPIO24_DATAOUT  <<  8) |(GPIO25_DATAOUT  <<  9) |(GPIO26_DATAOUT  << 10) |(GPIO27_DATAOUT  << 11) |
     (GPIO28_DATAOUT  << 12) |(GPIO29_DATAOUT  << 13) |(GPIO30_DATAOUT  << 14) |(GPIO31_DATAOUT  << 15)),

    ((GPIO32_DATAOUT  <<  0) |(GPIO33_DATAOUT  <<  1) |(GPIO34_DATAOUT  <<  2) |(GPIO35_DATAOUT  <<  3) |
     (GPIO36_DATAOUT  <<  4) |(GPIO37_DATAOUT  <<  5) |(GPIO38_DATAOUT  <<  6) |(GPIO39_DATAOUT  <<  7) |
     (GPIO40_DATAOUT  <<  8) |(GPIO41_DATAOUT  <<  9) |(GPIO42_DATAOUT  << 10) |(GPIO43_DATAOUT  << 11) |
     (GPIO44_DATAOUT  << 12) |(GPIO45_DATAOUT  << 13) |(GPIO46_DATAOUT  << 14) |(GPIO47_DATAOUT  << 15)),

    ((GPIO48_DATAOUT  <<  0) |(GPIO49_DATAOUT  <<  1) |(GPIO50_DATAOUT  <<  2) |(GPIO51_DATAOUT  <<  3) |
     (GPIO52_DATAOUT  <<  4) |(GPIO53_DATAOUT  <<  5) |(GPIO54_DATAOUT  <<  6) |(GPIO55_DATAOUT  <<  7) |
     (GPIO56_DATAOUT  <<  8) |(GPIO57_DATAOUT  <<  9) |(GPIO58_DATAOUT  << 10) |(GPIO59_DATAOUT  << 11) |
     (GPIO60_DATAOUT  << 12) |(GPIO61_DATAOUT  << 13) |(GPIO62_DATAOUT  << 14) |(GPIO63_DATAOUT  << 15)),

    ((GPIO64_DATAOUT  <<  0) |(GPIO65_DATAOUT  <<  1) |(GPIO66_DATAOUT  <<  2) |(GPIO67_DATAOUT  <<  3) |
     (GPIO68_DATAOUT  <<  4) |(GPIO69_DATAOUT  <<  5) |(GPIO70_DATAOUT  <<  6) |(GPIO71_DATAOUT  <<  7) |
     (GPIO72_DATAOUT  <<  8) |(GPIO73_DATAOUT  <<  9) |(GPIO74_DATAOUT  << 10) |(GPIO75_DATAOUT  << 11) |
     (GPIO76_DATAOUT  << 12) |(GPIO77_DATAOUT  << 13) |(GPIO78_DATAOUT  << 14) |(GPIO79_DATAOUT  << 15)),

    ((GPIO80_DATAOUT  <<  0) |(GPIO81_DATAOUT  <<  1) |(GPIO82_DATAOUT  <<  2) |(GPIO83_DATAOUT  <<  3) |
     (GPIO84_DATAOUT  <<  4) |(GPIO85_DATAOUT  <<  5) |(GPIO86_DATAOUT  <<  6) |(GPIO87_DATAOUT  <<  7) |
     (GPIO88_DATAOUT  <<  8) |(GPIO89_DATAOUT  <<  9) |(GPIO90_DATAOUT  << 10) |(GPIO91_DATAOUT  << 11) |
     (GPIO92_DATAOUT  << 12) |(GPIO93_DATAOUT  << 13) |(GPIO94_DATAOUT  << 14) |(GPIO95_DATAOUT  << 15)),

    ((GPIO96_DATAOUT  <<  0) |(GPIO97_DATAOUT  <<  1) |(GPIO98_DATAOUT  <<  2) |(GPIO99_DATAOUT  <<  3) |
     (GPIO100_DATAOUT <<  4) |(GPIO101_DATAOUT <<  5) |(GPIO102_DATAOUT <<  6) |(GPIO103_DATAOUT <<  7) |
     (GPIO104_DATAOUT <<  8) |(GPIO105_DATAOUT <<  9) |(GPIO106_DATAOUT << 10) |(GPIO107_DATAOUT << 11) |
     (GPIO108_DATAOUT << 12) |(GPIO109_DATAOUT << 13) |(GPIO110_DATAOUT << 14) |(GPIO111_DATAOUT << 15)),

    ((GPIO112_DATAOUT <<  0) |(GPIO113_DATAOUT <<  1) |(GPIO114_DATAOUT <<  2) |(GPIO115_DATAOUT <<  3) |
     (GPIO116_DATAOUT <<  4) |(GPIO117_DATAOUT <<  5) |(GPIO118_DATAOUT <<  6) |(GPIO119_DATAOUT <<  7) |
     (GPIO120_DATAOUT <<  8) |(GPIO121_DATAOUT <<  9) |(GPIO122_DATAOUT << 10) |(GPIO123_DATAOUT << 11) |
     (GPIO124_DATAOUT << 12) |(GPIO125_DATAOUT << 13) |(GPIO126_DATAOUT << 14) |(GPIO127_DATAOUT << 15)),

    ((GPIO128_DATAOUT <<  0) |(GPIO129_DATAOUT <<  1) |(GPIO130_DATAOUT <<  2) |(GPIO131_DATAOUT <<  3) |
     (GPIO132_DATAOUT <<  4) |(GPIO133_DATAOUT <<  5) |(GPIO134_DATAOUT <<  6) |(GPIO135_DATAOUT <<  7) |
     (GPIO136_DATAOUT <<  8) |(GPIO137_DATAOUT <<  9) |(GPIO138_DATAOUT << 10) |(GPIO139_DATAOUT << 11) |
     (GPIO140_DATAOUT << 12) |(GPIO141_DATAOUT << 13) |(GPIO142_DATAOUT << 14) |(GPIO143_DATAOUT << 15)),

    ((GPIO144_DATAOUT <<  0) |(GPIO145_DATAOUT <<  1) |(GPIO146_DATAOUT <<  2) |(GPIO147_DATAOUT <<  3) |
     (GPIO148_DATAOUT <<  4) |(GPIO149_DATAOUT <<  5) |(GPIO150_DATAOUT <<  6) |(GPIO151_DATAOUT <<  7) |
     (GPIO152_DATAOUT <<  8) |(GPIO153_DATAOUT <<  9) |(GPIO154_DATAOUT << 10) |(GPIO155_DATAOUT << 11) |
     (GPIO156_DATAOUT << 12) |(GPIO157_DATAOUT << 13) |(GPIO158_DATAOUT << 14) |(GPIO159_DATAOUT << 15)),

    ((GPIO160_DATAOUT <<  0) |(GPIO161_DATAOUT <<  1) |(GPIO162_DATAOUT <<  2) |(GPIO163_DATAOUT <<  3) |
     (GPIO164_DATAOUT <<  4) |(GPIO165_DATAOUT <<  5) |(GPIO166_DATAOUT <<  6) |(GPIO167_DATAOUT <<  7) |
     (GPIO168_DATAOUT <<  8)),
}; /*end of gpio_init_dout_data*/
/*----------------------------------------------------------------------------*/
u16 gpio_init_mode_data[] = {
    ((GPIO0_MODE      <<  0) |(GPIO1_MODE      <<  3) |(GPIO2_MODE      <<  6) |(GPIO3_MODE      <<  9) |(GPIO4_MODE      << 12)),
    ((GPIO5_MODE      <<  0) |(GPIO6_MODE      <<  3) |(GPIO7_MODE      <<  6) |(GPIO8_MODE      <<  9) |(GPIO9_MODE      << 12)),
    ((GPIO10_MODE     <<  0) |(GPIO11_MODE     <<  3) |(GPIO12_MODE     <<  6) |(GPIO13_MODE     <<  9) |(GPIO14_MODE     << 12)),
    ((GPIO15_MODE     <<  0) |(GPIO16_MODE     <<  3) |(GPIO17_MODE     <<  6) |(GPIO18_MODE     <<  9) |(GPIO19_MODE     << 12)),
    ((GPIO20_MODE     <<  0) |(GPIO21_MODE     <<  3) |(GPIO22_MODE     <<  6) |(GPIO23_MODE     <<  9) |(GPIO24_MODE     << 12)),
    ((GPIO25_MODE     <<  0) |(GPIO26_MODE     <<  3) |(GPIO27_MODE     <<  6) |(GPIO28_MODE     <<  9) |(GPIO29_MODE     << 12)),
    ((GPIO30_MODE     <<  0) |(GPIO31_MODE     <<  3) |(GPIO32_MODE     <<  6) |(GPIO33_MODE     <<  9) |(GPIO34_MODE     << 12)),
    ((GPIO35_MODE     <<  0) |(GPIO36_MODE     <<  3) |(GPIO37_MODE     <<  6) |(GPIO38_MODE     <<  9) |(GPIO39_MODE     << 12)),
    ((GPIO40_MODE     <<  0) |(GPIO41_MODE     <<  3) |(GPIO42_MODE     <<  6) |(GPIO43_MODE     <<  9) |(GPIO44_MODE     << 12)),
    ((GPIO45_MODE     <<  0) |(GPIO46_MODE     <<  3) |(GPIO47_MODE     <<  6) |(GPIO48_MODE     <<  9) |(GPIO49_MODE     << 12)),
    ((GPIO50_MODE     <<  0) |(GPIO51_MODE     <<  3) |(GPIO52_MODE     <<  6) |(GPIO53_MODE     <<  9) |(GPIO54_MODE     << 12)),
    ((GPIO55_MODE     <<  0) |(GPIO56_MODE     <<  3) |(GPIO57_MODE     <<  6) |(GPIO58_MODE     <<  9) |(GPIO59_MODE     << 12)),
    ((GPIO60_MODE     <<  0) |(GPIO61_MODE     <<  3) |(GPIO62_MODE     <<  6) |(GPIO63_MODE     <<  9) |(GPIO64_MODE     << 12)),
    ((GPIO65_MODE     <<  0) |(GPIO66_MODE     <<  3) |(GPIO67_MODE     <<  6) |(GPIO68_MODE     <<  9) |(GPIO69_MODE     << 12)),
    ((GPIO70_MODE     <<  0) |(GPIO71_MODE     <<  3) |(GPIO72_MODE     <<  6) |(GPIO73_MODE     <<  9) |(GPIO74_MODE     << 12)),
    ((GPIO75_MODE     <<  0) |(GPIO76_MODE     <<  3) |(GPIO77_MODE     <<  6) |(GPIO78_MODE     <<  9) |(GPIO79_MODE     << 12)),
    ((GPIO80_MODE     <<  0) |(GPIO81_MODE     <<  3) |(GPIO82_MODE     <<  6) |(GPIO83_MODE     <<  9) |(GPIO84_MODE     << 12)),
    ((GPIO85_MODE     <<  0) |(GPIO86_MODE     <<  3) |(GPIO87_MODE     <<  6) |(GPIO88_MODE     <<  9) |(GPIO89_MODE     << 12)),
    ((GPIO90_MODE     <<  0) |(GPIO91_MODE     <<  3) |(GPIO92_MODE     <<  6) |(GPIO93_MODE     <<  9) |(GPIO94_MODE     << 12)),
    ((GPIO95_MODE     <<  0) |(GPIO96_MODE     <<  3) |(GPIO97_MODE     <<  6) |(GPIO98_MODE     <<  9) |(GPIO99_MODE     << 12)),
    ((GPIO100_MODE    <<  0) |(GPIO101_MODE    <<  3) |(GPIO102_MODE    <<  6) |(GPIO103_MODE    <<  9) |(GPIO104_MODE    << 12)),
    ((GPIO105_MODE    <<  0) |(GPIO106_MODE    <<  3) |(GPIO107_MODE    <<  6) |(GPIO108_MODE    <<  9) |(GPIO109_MODE    << 12)),
    ((GPIO110_MODE    <<  0) |(GPIO111_MODE    <<  3) |(GPIO112_MODE    <<  6) |(GPIO113_MODE    <<  9) |(GPIO114_MODE    << 12)),
    ((GPIO115_MODE    <<  0) |(GPIO116_MODE    <<  3) |(GPIO117_MODE    <<  6) |(GPIO118_MODE    <<  9) |(GPIO119_MODE    << 12)),
    ((GPIO120_MODE    <<  0) |(GPIO121_MODE    <<  3) |(GPIO122_MODE    <<  6) |(GPIO123_MODE    <<  9) |(GPIO124_MODE    << 12)),
    ((GPIO125_MODE    <<  0) |(GPIO126_MODE    <<  3) |(GPIO127_MODE    <<  6) |(GPIO128_MODE    <<  9) |(GPIO129_MODE    << 12)),
    ((GPIO130_MODE    <<  0) |(GPIO131_MODE    <<  3) |(GPIO132_MODE    <<  6) |(GPIO133_MODE    <<  9) |(GPIO134_MODE    << 12)),
    ((GPIO135_MODE    <<  0) |(GPIO136_MODE    <<  3) |(GPIO137_MODE    <<  6) |(GPIO138_MODE    <<  9) |(GPIO139_MODE    << 12)),
    ((GPIO140_MODE    <<  0) |(GPIO141_MODE    <<  3) |(GPIO142_MODE    <<  6) |(GPIO143_MODE    <<  9) |(GPIO144_MODE    << 12)),
    ((GPIO145_MODE    <<  0) |(GPIO146_MODE    <<  3) |(GPIO147_MODE    <<  6) |(GPIO148_MODE    <<  9) |(GPIO149_MODE    << 12)),
    ((GPIO150_MODE    <<  0) |(GPIO151_MODE    <<  3) |(GPIO152_MODE    <<  6) |(GPIO153_MODE    <<  9) |(GPIO154_MODE    << 12)),
    ((GPIO155_MODE    <<  0) |(GPIO156_MODE    <<  3) |(GPIO157_MODE    <<  6) |(GPIO158_MODE    <<  9) |(GPIO159_MODE    << 12)),
    ((GPIO160_MODE    <<  0) |(GPIO161_MODE    <<  3) |(GPIO162_MODE    <<  6) |(GPIO163_MODE    <<  9) |(GPIO164_MODE    << 12)),
    ((GPIO165_MODE    <<  0) |(GPIO166_MODE    <<  3) |(GPIO167_MODE    <<  6) |(GPIO168_MODE    <<  9)),
}; /*end of gpio_init_mode_more_data*/

/*----------------------------------------------------------------------------*/
#if defined(GPIO_INIT_DEBUG)
static GPIO_REGS saved;
#endif 

void mt_gpio_set_default_chip(void)
{
    GPIO_REGS *pReg = (GPIO_REGS*)(GPIO_BASE);
    unsigned int idx;
    u32 val;
    
    for (idx = 0; idx < sizeof(pReg->dir)/sizeof(pReg->dir[0]); idx++){
		val = gpio_init_dir_data[idx];
        GPIO_WR32(&pReg->dir[idx],val);
    }
    
    for (idx = 0; idx < sizeof(pReg->pullen)/sizeof(pReg->pullen[0]); idx++){
		val = gpio_init_pullen_data[idx];
		GPIO_WR32(&pReg->pullen[idx],val);
    }
    
    for (idx = 0; idx < sizeof(pReg->pullsel)/sizeof(pReg->pullsel[0]); idx++){ 
		val = gpio_init_pullsel_data[idx];
		GPIO_WR32(&pReg->pullsel[idx],val);
    }
	/*for keypad special register*/
	mt_set_gpio_pull_select(GPIO74, GPIO74_PULL);
	mt_set_gpio_pull_select(GPIO75, GPIO75_PULL);
	mt_set_gpio_pull_select(GPIO92, GPIO92_PULL);
	mt_set_gpio_pull_select(GPIO93, GPIO93_PULL);
	mt_set_gpio_pull_select(GPIO167, GPIO167_PULL);
	mt_set_gpio_pull_select(GPIO168, GPIO168_PULL);
    
    for (idx = 0; idx < sizeof(pReg->dout)/sizeof(pReg->dout[0]); idx++) {
		val = gpio_init_dout_data[idx];
        GPIO_WR32(&pReg->dout[idx],val);
    }
    
    for (idx = 0; idx < sizeof(pReg->mode)/sizeof(pReg->mode[0]); idx++) {
		val = gpio_init_mode_data[idx];
		GPIO_WR32(&pReg->mode[idx],val);
    }

	//set BSI driving 2ma->4ma: 0x10005B30[7:4]=0x2
	GPIO_WR32(&pReg->drv_mode[3].set, 0x20);
    
    GPIOVER("mt_gpio_set_default() done\n");        
}

void mt_gpio_set_dsel(u8 bpi_1v8)
{
    GPIO_REGS *pReg = (GPIO_REGS*)(GPIO_BASE);
	
	if (bpi_1v8 == GPIO_VIO18) {
		/*bpi_ctrl[9:4] = b`000000, [3:0] = b`0000*/
		GPIO_WR32(&pReg->bpi_ctrl[0].rst, 0x3FF);
	} else if (bpi_1v8 == GPIO_VIO28){
		/*bpi_ctrl[9:4] = b`001100, [3:0] = b`0101*/
		GPIO_WR32(&pReg->bpi_ctrl[0].set, 0xC5);
		GPIO_WR32(&pReg->bpi_ctrl[0].rst, 0x33A);
	}

	GPIOVER("BPI 0x%x\n",GPIO_RD32(&pReg->bpi_ctrl[0].val)); 		
}	
void mt_gpio_set_default(void)
{   
	mt_gpio_set_default_chip();
	mt_gpio_set_dsel(GPIO_DVDD28_BPI);
}
/*----------------------------------------------------------------------------*/
void mt_gpio_checkpoint_save(void)
{
#if defined(GPIO_INIT_DEBUG)    
    GPIO_REGS *pReg = (GPIO_REGS*)(GPIO_BASE);
    GPIO_REGS *cur = &saved;
    unsigned int idx;
    
    memset(cur, 0x00, sizeof(*cur));
    for (idx = 0; idx < sizeof(pReg->dir)/sizeof(pReg->dir[0]); idx++)
        cur->dir[idx].val = GPIO_RD32(&pReg->dir[idx]);
    for (idx = 0; idx < sizeof(pReg->pullen)/sizeof(pReg->pullen[0]); idx++)
        cur->pullen[idx].val = GPIO_RD32(&pReg->pullen[idx]);
    for (idx = 0; idx < sizeof(pReg->pullsel)/sizeof(pReg->pullsel[0]); idx++)
        cur->pullsel[idx].val =GPIO_RD32(&pReg->pullsel[idx]);
/*    for (idx = 0; idx < sizeof(pReg->dinv)/sizeof(pReg->dinv[0]); idx++)
        cur->dinv[idx].val =GPIO_RD32(&pReg->dinv[idx]);*/
    for (idx = 0; idx < sizeof(pReg->dout)/sizeof(pReg->dout[0]); idx++)
        cur->dout[idx].val = GPIO_RD32(&pReg->dout[idx]);
    for (idx = 0; idx < sizeof(pReg->mode)/sizeof(pReg->mode[0]); idx++)
        cur->mode[idx].val = GPIO_RD32(&pReg->mode[idx]);    

#endif     
}
/*----------------------------------------------------------------------------*/
void mt_gpio_dump_diff(GPIO_REGS* pre, GPIO_REGS* cur)
{
#if defined(GPIO_INIT_DEBUG)        
    GPIO_REGS *pReg = (GPIO_REGS*)(GPIO_BASE);
	unsigned int idx;
    unsigned char* p = (unsigned char*)pre;
    unsigned char* q = (unsigned char*)cur;
    
    GPIOVER("------ dumping difference between %p and %p ------\n", pre, cur);
    for (idx = 0; idx < sizeof(pReg->dir)/sizeof(pReg->dir[0]); idx++) {
        if (pre->dir[idx].val != cur->dir[idx].val)
            GPIOVER("diff: dir[%2d]    : 0x%08X <=> 0x%08X\n", idx, pre->dir[idx].val, cur->dir[idx].val);
    }
    for (idx = 0; idx < sizeof(pReg->pullen)/sizeof(pReg->pullen[0]); idx++) {
        if (pre->pullen[idx].val != cur->pullen[idx].val)
            GPIOVER("diff: pullen[%2d] : 0x%08X <=> 0x%08X\n", idx, pre->pullen[idx].val, cur->pullen[idx].val);
    }
    for (idx = 0; idx < sizeof(pReg->pullsel)/sizeof(pReg->pullsel[0]); idx++) {
        if (pre->pullsel[idx].val != cur->pullsel[idx].val)
            GPIOVER("diff: pullsel[%2d]: 0x%08X <=> 0x%08X\n", idx, pre->pullsel[idx].val, cur->pullsel[idx].val);
    }
/*    for (idx = 0; idx < sizeof(pReg->dinv)/sizeof(pReg->dinv[0]); idx++) {
        if (pre->dinv[idx].val != cur->dinv[idx].val)
            GPIOVER("diff: dinv[%2d]   : 0x%08X <=> 0x%08X\n", idx, pre->dinv[idx].val, cur->dinv[idx].val);
    }*/
    for (idx = 0; idx < sizeof(pReg->dout)/sizeof(pReg->dout[0]); idx++) {
        if (pre->dout[idx].val != cur->dout[idx].val)
            GPIOVER("diff: dout[%2d]   : 0x%08X <=> 0x%08X\n", idx, pre->dout[idx].val, cur->dout[idx].val);
    }
    for (idx = 0; idx < sizeof(pReg->mode)/sizeof(pReg->mode[0]); idx++) {
        if (pre->mode[idx].val != cur->mode[idx].val)
            GPIOVER("diff: mode[%2d]   : 0x%08X <=> 0x%08X\n", idx, pre->mode[idx].val, cur->mode[idx].val);
    }
    
    for (idx = 0; idx < sizeof(*pre); idx++) {
        if (p[idx] != q[idx])
            GPIOVER("diff: raw[%2d]: 0x%02X <=> 0x%02X\n", idx, p[idx], q[idx]);
    }
    GPIOVER("memcmp(%p, %p, %d) = %d\n", p, q, sizeof(*pre), memcmp(p, q, sizeof(*pre)));
    GPIOVER("------ dumping difference end --------------------------------\n");
#endif 
}
/*----------------------------------------------------------------------------*/
void mt_gpio_checkpoint_compare(void)
{
#if defined(GPIO_INIT_DEBUG)        
    GPIO_REGS *pReg = (GPIO_REGS*)(GPIO_BASE);
    GPIO_REGS latest;
    GPIO_REGS *cur = &latest;
    unsigned int idx;
    
    memset(cur, 0x00, sizeof(*cur));
    for (idx = 0; idx < sizeof(pReg->dir)/sizeof(pReg->dir[0]); idx++)
        cur->dir[idx].val = GPIO_RD32(&pReg->dir[idx]);
    for (idx = 0; idx < sizeof(pReg->pullen)/sizeof(pReg->pullen[0]); idx++)
        cur->pullen[idx].val = GPIO_RD32(&pReg->pullen[idx]);
    for (idx = 0; idx < sizeof(pReg->pullsel)/sizeof(pReg->pullsel[0]); idx++)
        cur->pullsel[idx].val =GPIO_RD32(&pReg->pullsel[idx]);
/*    for (idx = 0; idx < sizeof(pReg->dinv)/sizeof(pReg->dinv[0]); idx++)
        cur->dinv[idx].val =GPIO_RD32(&pReg->dinv[idx]);*/
    for (idx = 0; idx < sizeof(pReg->dout)/sizeof(pReg->dout[0]); idx++)
        cur->dout[idx].val = GPIO_RD32(&pReg->dout[idx]);
    for (idx = 0; idx < sizeof(pReg->mode)/sizeof(pReg->mode[0]); idx++)
        cur->mode[idx].val = GPIO_RD32(&pReg->mode[idx]);    
 
    //mt_gpio_dump_diff(&latest, &saved);
    //GPIODBG("memcmp(%p, %p, %d) = %d\n", &latest, &saved, sizeof(GPIO_REGS), memcmp(&latest, &saved, sizeof(GPIO_REGS)));
    if (memcmp(&latest, &saved, sizeof(GPIO_REGS))) {
        GPIODBG("checkpoint compare fail!!\n");
        GPIODBG("dump checkpoint....\n");
        //mt_gpio_dump(&saved);
        GPIODBG("\n\n");
        GPIODBG("dump current state\n");
        //mt_gpio_dump(&latest);
        GPIODBG("\n\n");
        mt_gpio_dump_diff(&saved, &latest);        
        //WARN_ON(1);
    } else {
        GPIODBG("checkpoint compare success!!\n");
    }
#endif    
}
/*----------------------------------------------------------------------------*/
#endif
