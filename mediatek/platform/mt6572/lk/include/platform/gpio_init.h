#ifndef _GPIO_INIT_H_
#define _GPIO_INIT_H_

/******************************************************************************
 * mt_gpio_init_value.c - MT6516 Linux GPIO Device Driver
 * 
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *     default GPIO init value
 *
 ******************************************************************************/
//#if defined(MT_GPIO_ENABLED)
//#include <cust_gpio_boot.h>
//#include <platform/gpio_const.h>
//#endif

const UINT32 gpio_init_value[][3] = {
{
IO_CFG_L_BASE+0x0000
,(GPIO39_IES << 6) | (GPIO38_IES << 6) | (GPIO37_IES << 6) | (GPIO36_IES << 6) | (GPIO35_IES << 6) | (GPIO34_IES << 6) | (GPIO33_IES << 6) | (GPIO32_IES << 6) | (GPIO31_IES << 5) | (GPIO30_IES << 5) | (GPIO29_IES << 5) | (GPIO28_IES << 5) | (GPIO27_IES << 5) | (GPIO26_IES << 5) | (GPIO25_IES << 5) | (GPIO24_IES << 5) | (GPIO23_IES << 5) | (GPIO22_IES << 5) | (GPIO21_IES << 5) | (GPIO20_IES << 5) | (GPIO19_IES << 5) | (GPIO18_IES << 5) | (GPIO17_IES << 5) | (GPIO16_IES << 5) | (GPIO15_IES << 5) | (GPIO14_IES << 5) | (GPIO13_IES << 4) | (GPIO12_IES << 4) | (GPIO11_IES << 3) | (GPIO10_IES << 3) | (GPIO9_IES << 3) | (GPIO8_IES << 3) | (GPIO7_IES << 4) | (GPIO6_IES << 2) | (GPIO5_IES << 2) | (GPIO4_IES << 2) | (GPIO3_IES << 1) | (GPIO2_IES << 1) | (GPIO1_IES << 0) | (GPIO0_IES << 0)
,(0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 5) | (0x1 << 4) | (0x1 << 4) | (0x1 << 3) | (0x1 << 3) | (0x1 << 3) | (0x1 << 3) | (0x1 << 4) | (0x1 << 2) | (0x1 << 2) | (0x1 << 2) | (0x1 << 1) | (0x1 << 1) | (0x1 << 0) | (0x1 << 0)
},
{
IO_CFG_B_BASE+0x0000
,(GPIO54_IES << 0) | (GPIO53_IES << 1) | (GPIO52_IES << 1) | (GPIO51_IES << 1) | (GPIO50_IES << 1) | (GPIO49_IES << 0) | (GPIO48_IES << 0) | (GPIO47_IES << 0) | (GPIO46_IES << 0) | (GPIO45_IES << 0) | (GPIO44_IES << 0) | (GPIO43_IES << 2) | (GPIO42_IES << 2) | (GPIO41_IES << 2) | (GPIO40_IES << 2)
,(0x1 << 0) | (0x1 << 1) | (0x1 << 1) | (0x1 << 1) | (0x1 << 1) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0) | (0x1 << 2) | (0x1 << 2) | (0x1 << 2) | (0x1 << 2)
},
{
IO_CFG_R_BASE+0x0000
,(GPIO106_IES << 14) | (GPIO105_IES << 14) | (GPIO104_IES << 13) | (GPIO103_IES << 13) | (GPIO102_IES << 12) | (GPIO101_IES << 11) | (GPIO100_IES << 10) | (GPIO99_IES << 9) | (GPIO98_IES << 8) | (GPIO97_IES << 7) | (GPIO96_IES << 6) | (GPIO95_IES << 6) | (GPIO94_IES << 6) | (GPIO93_IES << 6) | (GPIO92_IES << 6) | (GPIO91_IES << 6) | (GPIO90_IES << 5) | (GPIO89_IES << 5) | (GPIO88_IES << 4) | (GPIO87_IES << 4) | (GPIO66_IES << 3) | (GPIO65_IES << 3) | (GPIO64_IES << 3) | (GPIO63_IES << 3) | (GPIO62_IES << 2) | (GPIO61_IES << 2) | (GPIO60_IES << 1) | (GPIO59_IES << 1) | (GPIO58_IES << 0) | (GPIO57_IES << 0) | (GPIO56_IES << 0) | (GPIO55_IES << 0)
,(0x1 << 14) | (0x1 << 14) | (0x1 << 13) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 6) | (0x1 << 5) | (0x1 << 5) | (0x1 << 4) | (0x1 << 4) | (0x1 << 3) | (0x1 << 3) | (0x1 << 3) | (0x1 << 3) | (0x1 << 2) | (0x1 << 2) | (0x1 << 1) | (0x1 << 1) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0)
},
{
MIPI_CFG_BASE+0x084C
,((GPIO71_MODE==GPIO_MODE_01)?0:((GPIO71_PULLEN&~GPIO71_PULL) << 27)) | ((GPIO70_MODE==GPIO_MODE_01)?0:((GPIO70_PULLEN&~GPIO70_PULL) << 21)) | ((GPIO69_MODE==GPIO_MODE_01)?0:((GPIO69_PULLEN&~GPIO69_PULL) << 15)) | ((GPIO68_MODE==GPIO_MODE_01)?0:((GPIO68_PULLEN&~GPIO68_PULL) << 9)) | ((GPIO67_MODE==GPIO_MODE_01)?0:((GPIO67_PULLEN&~GPIO67_PULL) << 3)) | ((GPIO71_MODE==GPIO_MODE_01)?0:((GPIO71_PULLEN&GPIO71_PULL) << 26)) | ((GPIO70_MODE==GPIO_MODE_01)?0:((GPIO70_PULLEN&GPIO70_PULL) << 20)) | ((GPIO69_MODE==GPIO_MODE_01)?0:((GPIO69_PULLEN&GPIO69_PULL) << 14)) | ((GPIO68_MODE==GPIO_MODE_01)?0:((GPIO68_PULLEN&GPIO68_PULL) << 8)) | ((GPIO67_MODE==GPIO_MODE_01)?0:((GPIO67_PULLEN&GPIO67_PULL) << 2)) | ((GPIO71_MODE==GPIO_MODE_01)?0:(GPIO71_IES << 24)) | ((GPIO70_MODE==GPIO_MODE_01)?0:(GPIO70_IES << 18)) | ((GPIO69_MODE==GPIO_MODE_01)?0:(GPIO69_IES << 12)) | ((GPIO68_MODE==GPIO_MODE_01)?0:(GPIO68_IES << 6)) | ((GPIO67_MODE==GPIO_MODE_01)?0:(GPIO67_IES << 0))
,((GPIO71_MODE==GPIO_MODE_01)?0:(0x1 << 27)) | ((GPIO70_MODE==GPIO_MODE_01)?0:(0x1 << 21)) | ((GPIO69_MODE==GPIO_MODE_01)?0:(0x1 << 15)) | ((GPIO68_MODE==GPIO_MODE_01)?0:(0x1 << 9)) | ((GPIO67_MODE==GPIO_MODE_01)?0:(0x1 << 3)) | ((GPIO71_MODE==GPIO_MODE_01)?0:(0x1 << 26)) | ((GPIO70_MODE==GPIO_MODE_01)?0:(0x1 << 20)) | ((GPIO69_MODE==GPIO_MODE_01)?0:(0x1 << 14)) | ((GPIO68_MODE==GPIO_MODE_01)?0:(0x1 << 8)) | ((GPIO67_MODE==GPIO_MODE_01)?0:(0x1 << 2)) | ((GPIO71_MODE==GPIO_MODE_01)?0:(0x1 << 24)) | ((GPIO70_MODE==GPIO_MODE_01)?0:(0x1 << 18)) | ((GPIO69_MODE==GPIO_MODE_01)?0:(0x1 << 12)) | ((GPIO68_MODE==GPIO_MODE_01)?0:(0x1 << 6)) | ((GPIO67_MODE==GPIO_MODE_01)?0:(0x1 << 0))
},
{
MIPI_CFG_BASE+0x0850
,((GPIO72_MODE==GPIO_MODE_01)?0:((GPIO72_PULLEN&~GPIO72_PULL) << 3)) | ((GPIO72_MODE==GPIO_MODE_01)?0:((GPIO72_PULLEN&GPIO72_PULL) << 2)) | ((GPIO72_MODE==GPIO_MODE_01)?0:(GPIO72_IES << 0))
,((GPIO72_MODE==GPIO_MODE_01)?0:(0x1 << 3)) | ((GPIO72_MODE==GPIO_MODE_01)?0:(0x1 << 2)) | ((GPIO72_MODE==GPIO_MODE_01)?0:(0x1 << 0))
},
{
MIPI_CFG_BASE+0x0848
,((GPIO78_MODE==GPIO_MODE_01)?0:(GPIO78_IES << 5)) | ((GPIO77_MODE==GPIO_MODE_01)?0:(GPIO77_IES << 4)) | ((GPIO76_MODE==GPIO_MODE_01)?0:(GPIO76_IES << 3)) | ((GPIO75_MODE==GPIO_MODE_01)?0:(GPIO75_IES << 2)) | ((GPIO74_MODE==GPIO_MODE_01)?0:(GPIO74_IES << 1)) | ((GPIO73_MODE==GPIO_MODE_01)?0:(GPIO73_IES << 0))
,((GPIO78_MODE==GPIO_MODE_01)?0:(0x1 << 5)) | ((GPIO77_MODE==GPIO_MODE_01)?0:(0x1 << 4)) | ((GPIO76_MODE==GPIO_MODE_01)?0:(0x1 << 3)) | ((GPIO75_MODE==GPIO_MODE_01)?0:(0x1 << 2)) | ((GPIO74_MODE==GPIO_MODE_01)?0:(0x1 << 1)) | ((GPIO73_MODE==GPIO_MODE_01)?0:(0x1 << 0))
},
{
MIPI_CFG_BASE+0x0074
,((GPIO86_MODE==GPIO_MODE_01)?0:(GPIO86_IES << 7)) | ((GPIO85_MODE==GPIO_MODE_01)?0:(GPIO85_IES << 6)) | ((GPIO84_MODE==GPIO_MODE_01)?0:(GPIO84_IES << 5)) | ((GPIO83_MODE==GPIO_MODE_01)?0:(GPIO83_IES << 4)) | ((GPIO82_MODE==GPIO_MODE_01)?0:(GPIO82_IES << 3)) | ((GPIO81_MODE==GPIO_MODE_01)?0:(GPIO81_IES << 2)) | ((GPIO80_MODE==GPIO_MODE_01)?0:(GPIO80_IES << 1)) | ((GPIO79_MODE==GPIO_MODE_01)?0:(GPIO79_IES << 0))
,((GPIO86_MODE==GPIO_MODE_01)?0:(0x1 << 7)) | ((GPIO85_MODE==GPIO_MODE_01)?0:(0x1 << 6)) | ((GPIO84_MODE==GPIO_MODE_01)?0:(0x1 << 5)) | ((GPIO83_MODE==GPIO_MODE_01)?0:(0x1 << 4)) | ((GPIO82_MODE==GPIO_MODE_01)?0:(0x1 << 3)) | ((GPIO81_MODE==GPIO_MODE_01)?0:(0x1 << 2)) | ((GPIO80_MODE==GPIO_MODE_01)?0:(0x1 << 1)) | ((GPIO79_MODE==GPIO_MODE_01)?0:(0x1 << 0))
},
{
IO_CFG_T_BASE+0x0000
,(GPIO153_IES << 12) | (GPIO152_IES << 12) | (GPIO151_IES << 12) | (GPIO150_IES << 12) | (GPIO149_IES << 12) | (GPIO148_IES << 11) | (GPIO147_IES << 11) | (GPIO146_IES << 11) | (GPIO145_IES << 10) | (GPIO144_IES << 10) | (GPIO143_IES << 10) | (GPIO142_IES << 9) | (GPIO141_IES << 9) | (GPIO140_IES << 9) | (GPIO139_IES << 9) | (GPIO138_IES << 9) | (GPIO137_IES << 9) | (GPIO136_IES << 8) | (GPIO135_IES << 8) | (GPIO134_IES << 8) | (GPIO133_IES << 7) | (GPIO132_IES << 7) | (GPIO131_IES << 7) | (GPIO130_IES << 7) | (GPIO129_IES << 14) | (GPIO128_IES << 13) | (GPIO114_IES << 6) | (GPIO113_IES << 6) | (GPIO112_IES << 5) | (GPIO111_IES << 4) | (GPIO110_IES << 3) | (GPIO109_IES << 2) | (GPIO108_IES << 1) | (GPIO107_IES << 0)
,(0x1 << 12) | (0x1 << 12) | (0x1 << 12) | (0x1 << 12) | (0x1 << 12) | (0x1 << 11) | (0x1 << 11) | (0x1 << 11) | (0x1 << 10) | (0x1 << 10) | (0x1 << 10) | (0x1 << 9) | (0x1 << 9) | (0x1 << 9) | (0x1 << 9) | (0x1 << 9) | (0x1 << 9) | (0x1 << 8) | (0x1 << 8) | (0x1 << 8) | (0x1 << 7) | (0x1 << 7) | (0x1 << 7) | (0x1 << 7) | (0x1 << 14) | (0x1 << 13) | (0x1 << 6) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x0010
,(GPIO127_IES << 2) | (GPIO126_IES << 2) | (GPIO124_IES << 1) | (GPIO123_IES << 1) | (GPIO122_IES << 1) | (GPIO121_IES << 1) | (GPIO120_IES << 0) | (GPIO119_IES << 0) | (GPIO118_IES << 0) | (GPIO117_IES << 0) | (GPIO116_IES << 0) | (GPIO115_IES << 0)
,(0x1 << 2) | (0x1 << 2) | (0x1 << 1) | (0x1 << 1) | (0x1 << 1) | (0x1 << 1) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0) | (0x1 << 0)
},
{
IO_CFG_L_BASE+0x0040
,(0 << 14) | (GPIO13_PULLEN << 13) | (GPIO12_PULLEN << 12) | (GPIO11_PULLEN << 10) | (GPIO10_PULLEN << 9) | (GPIO9_PULLEN << 8) | (GPIO8_PULLEN << 7) | (GPIO7_PULLEN << 11) | (GPIO6_PULLEN << 6) | (GPIO5_PULLEN << 5) | (GPIO4_PULLEN << 4) | (GPIO3_PULLEN << 3) | (GPIO2_PULLEN << 2) | (GPIO1_PULLEN << 1) | (GPIO0_PULLEN << 0)
,(0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 11) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_L_BASE+0x0050
,(GPIO39_PULLEN << 25) | (GPIO38_PULLEN << 24) | (GPIO37_PULLEN << 23) | (GPIO36_PULLEN << 22) | (GPIO35_PULLEN << 21) | (GPIO34_PULLEN << 20) | (GPIO33_PULLEN << 19) | (GPIO32_PULLEN << 18) | (GPIO31_PULLEN << 17) | (GPIO30_PULLEN << 16) | (GPIO29_PULLEN << 15) | (GPIO28_PULLEN << 14) | (GPIO27_PULLEN << 13) | (GPIO26_PULLEN << 12) | (GPIO25_PULLEN << 11) | (GPIO24_PULLEN << 10) | (GPIO23_PULLEN << 9) | (GPIO22_PULLEN << 8) | (GPIO21_PULLEN << 7) | (GPIO20_PULLEN << 6) | (GPIO19_PULLEN << 5) | (GPIO18_PULLEN << 4) | (GPIO17_PULLEN << 3) | (GPIO16_PULLEN << 2) | (GPIO15_PULLEN << 1) | (GPIO14_PULLEN << 0)
,(0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_B_BASE+0x0040
,(GPIO54_PULLEN << 20) | (GPIO53_PULLEN << 18) | (GPIO52_PULLEN << 16) | (GPIO51_PULLEN << 14) | (GPIO50_PULLEN << 12) | (GPIO49_PULLEN << 10) | (GPIO48_PULLEN << 8) | (GPIO47_PULLEN << 6) | (GPIO46_PULLEN << 4) | (GPIO45_PULLEN << 2) | (GPIO44_PULLEN << 0) | (GPIO43_PULLEN << 24) | (GPIO42_PULLEN << 23) | (GPIO41_PULLEN << 22) | (GPIO40_PULLEN << 21)
,(0x1 << 20) | (0x3 << 18) | (0x3 << 16) | (0x3 << 14) | (0x3 << 12) | (0x3 << 10) | (0x3 << 8) | (0x3 << 6) | (0x3 << 4) | (0x3 << 2) | (0x3 << 0) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21)
},
{
IO_CFG_R_BASE+0x0040
,(GPIO106_PULLEN << 31) | (GPIO105_PULLEN << 30) | (GPIO104_PULLEN << 29) | (GPIO103_PULLEN << 28) | (GPIO102_PULLEN << 27) | (GPIO101_PULLEN << 26) | (GPIO100_PULLEN << 25) | (GPIO99_PULLEN << 24) | (GPIO98_PULLEN << 23) | (GPIO97_PULLEN << 22) | (GPIO96_PULLEN << 21) | (GPIO95_PULLEN << 20) | (GPIO94_PULLEN << 19) | (GPIO93_PULLEN << 18) | (GPIO92_PULLEN << 17) | (GPIO91_PULLEN << 16) | (GPIO90_PULLEN << 15) | (GPIO89_PULLEN << 14) | (GPIO88_PULLEN << 13) | (GPIO87_PULLEN << 12) | (GPIO66_PULLEN << 11) | (GPIO65_PULLEN << 10) | (GPIO64_PULLEN << 9) | (GPIO63_PULLEN << 8) | (GPIO62_PULLEN << 7) | (GPIO61_PULLEN << 6) | (GPIO60_PULLEN << 5) | (GPIO59_PULLEN << 4) | (GPIO58_PULLEN << 3) | (GPIO57_PULLEN << 2) | (GPIO56_PULLEN << 1) | (GPIO55_PULLEN << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x0070
,(GPIO153_PULLEN << 31) | (GPIO152_PULLEN << 30) | (GPIO151_PULLEN << 29) | (GPIO150_PULLEN << 28) | (GPIO149_PULLEN << 27) | (GPIO148_PULLEN << 26) | (GPIO147_PULLEN << 25) | (GPIO146_PULLEN << 24) | (GPIO145_PULLEN << 23) | (GPIO144_PULLEN << 22) | (GPIO143_PULLEN << 21) | (GPIO142_PULLEN << 20) | (GPIO141_PULLEN << 19) | (GPIO140_PULLEN << 18) | (GPIO139_PULLEN << 17) | (GPIO138_PULLEN << 16) | (GPIO137_PULLEN << 15) | (GPIO136_PULLEN << 14) | (GPIO135_PULLEN << 13) | (GPIO134_PULLEN << 12) | (GPIO133_PULLEN << 11) | (GPIO132_PULLEN << 10) | (GPIO131_PULLEN << 9) | (GPIO130_PULLEN << 8) | (GPIO114_PULLEN << 7) | (GPIO113_PULLEN << 6) | (GPIO112_PULLEN << 5) | (GPIO111_PULLEN << 4) | (GPIO110_PULLEN << 3) | (GPIO109_PULLEN << 2) | (GPIO108_PULLEN << 1) | (GPIO107_PULLEN << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x0090
,(GPIO127_PULLEN << 12) | (GPIO126_PULLEN << 11) | (GPIO124_PULLEN << 9) | (GPIO123_PULLEN << 8) | (GPIO122_PULLEN << 7) | (GPIO121_PULLEN << 6) | (GPIO120_PULLEN << 5) | (GPIO119_PULLEN << 4) | (GPIO118_PULLEN << 3) | (GPIO117_PULLEN << 2) | (GPIO116_PULLEN << 1) | (GPIO115_PULLEN << 0)
,(0x1 << 12) | (0x1 << 11) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x0080
,(GPIO129_PULLEN << 1) | (GPIO128_PULLEN << 0)
,(0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_L_BASE+0x0060
,(0 << 16) | (0 << 15) | (GPIO13_PULL << 13) | (GPIO12_PULL << 12) | (GPIO11_PULL << 10) | (GPIO10_PULL << 9) | (GPIO9_PULL << 8) | (GPIO8_PULL << 7) | (GPIO7_PULL << 11) | (GPIO6_PULL << 6) | (GPIO5_PULL << 5) | (GPIO4_PULL << 4) | (GPIO3_PULL << 3) | (GPIO2_PULL << 2) | (GPIO1_PULL << 1) | (GPIO0_PULL << 0)
,(0x1 << 16) | (0x1 << 15) | (0x1 << 13) | (0x1 << 12) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 11) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_L_BASE+0x0070
,(GPIO39_PULL << 25) | (GPIO38_PULL << 24) | (GPIO37_PULL << 23) | (GPIO36_PULL << 22) | (GPIO35_PULL << 21) | (GPIO34_PULL << 20) | (GPIO33_PULL << 19) | (GPIO32_PULL << 18) | (GPIO31_PULL << 17) | (GPIO30_PULL << 16) | (GPIO29_PULL << 15) | (GPIO28_PULL << 14) | (GPIO27_PULL << 13) | (GPIO26_PULL << 12) | (GPIO25_PULL << 11) | (GPIO24_PULL << 10) | (GPIO23_PULL << 9) | (GPIO22_PULL << 8) | (GPIO21_PULL << 7) | (GPIO20_PULL << 6) | (GPIO19_PULL << 5) | (GPIO18_PULL << 4) | (GPIO17_PULL << 3) | (GPIO16_PULL << 2) | (GPIO15_PULL << 1) | (GPIO14_PULL << 0)
,(0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_B_BASE+0x0050
,(GPIO54_PULL << 10) | (GPIO53_PULL << 9) | (GPIO52_PULL << 8) | (GPIO51_PULL << 7) | (GPIO50_PULL << 6) | (GPIO49_PULL << 5) | (GPIO48_PULL << 4) | (GPIO47_PULL << 3) | (GPIO46_PULL << 2) | (GPIO45_PULL << 1) | (GPIO44_PULL << 0) | (GPIO43_PULL << 24) | (GPIO42_PULL << 23) | (GPIO41_PULL << 22) | (GPIO40_PULL << 21)
,(0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21)
},
{
IO_CFG_R_BASE+0x0050
,(GPIO106_PULL << 31) | (GPIO105_PULL << 30) | (GPIO104_PULL << 29) | (GPIO103_PULL << 28) | (GPIO102_PULL << 27) | (GPIO101_PULL << 26) | (GPIO100_PULL << 25) | (GPIO99_PULL << 24) | (GPIO98_PULL << 23) | (GPIO97_PULL << 22) | (GPIO96_PULL << 21) | (GPIO95_PULL << 20) | (GPIO94_PULL << 19) | (GPIO93_PULL << 18) | (GPIO92_PULL << 17) | (GPIO91_PULL << 16) | (GPIO90_PULL << 15) | (GPIO89_PULL << 14) | (GPIO88_PULL << 13) | (GPIO87_PULL << 12) | (GPIO66_PULL << 11) | (GPIO65_PULL << 10) | (GPIO64_PULL << 9) | (GPIO63_PULL << 8) | (GPIO62_PULL << 7) | (GPIO61_PULL << 6) | (GPIO60_PULL << 5) | (GPIO59_PULL << 4) | (GPIO58_PULL << 3) | (GPIO57_PULL << 2) | (GPIO56_PULL << 1) | (GPIO55_PULL << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x00A0
,(GPIO153_PULL << 31) | (GPIO152_PULL << 30) | (GPIO151_PULL << 29) | (GPIO150_PULL << 28) | (GPIO149_PULL << 27) | (GPIO148_PULL << 26) | (GPIO147_PULL << 25) | (GPIO146_PULL << 24) | (GPIO145_PULL << 23) | (GPIO144_PULL << 22) | (GPIO143_PULL << 21) | (GPIO142_PULL << 20) | (GPIO141_PULL << 19) | (GPIO140_PULL << 18) | (GPIO139_PULL << 17) | (GPIO138_PULL << 16) | (GPIO137_PULL << 15) | (GPIO136_PULL << 14) | (GPIO135_PULL << 13) | (GPIO134_PULL << 12) | (GPIO133_PULL << 11) | (GPIO132_PULL << 10) | (GPIO131_PULL << 9) | (GPIO130_PULL << 8) | (GPIO114_PULL << 7) | (GPIO113_PULL << 6) | (GPIO112_PULL << 5) | (GPIO111_PULL << 4) | (GPIO110_PULL << 3) | (GPIO109_PULL << 2) | (GPIO108_PULL << 1) | (GPIO107_PULL << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x00C0
,(GPIO127_PULL << 12) | (GPIO126_PULL << 11) | (GPIO124_PULL << 9) | (GPIO123_PULL << 8) | (GPIO122_PULL << 7) | (GPIO121_PULL << 6) | (GPIO120_PULL << 5) | (GPIO119_PULL << 4) | (GPIO118_PULL << 3) | (GPIO117_PULL << 2) | (GPIO116_PULL << 1) | (GPIO115_PULL << 0)
,(0x1 << 12) | (0x1 << 11) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x00B0
,(GPIO129_PULL << 1) | (GPIO128_PULL << 0)
,(0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0300
,(GPIO7_MODE << 28) | (GPIO6_MODE << 24) | (GPIO5_MODE << 20) | (GPIO4_MODE << 16) | (GPIO3_MODE << 12) | (GPIO2_MODE << 8) | (GPIO1_MODE << 4) | (GPIO0_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0310
,(GPIO15_MODE << 28) | (GPIO14_MODE << 24) | (GPIO13_MODE << 20) | (GPIO12_MODE << 16) | (GPIO11_MODE << 12) | (GPIO10_MODE << 8) | (GPIO9_MODE << 4) | (GPIO8_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0320
,(GPIO23_MODE << 28) | (GPIO22_MODE << 24) | (GPIO21_MODE << 20) | (GPIO20_MODE << 16) | (GPIO19_MODE << 12) | (GPIO18_MODE << 8) | (GPIO17_MODE << 4) | (GPIO16_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0330
,(GPIO31_MODE << 28) | (GPIO30_MODE << 24) | (GPIO29_MODE << 20) | (GPIO28_MODE << 16) | (GPIO27_MODE << 12) | (GPIO26_MODE << 8) | (GPIO25_MODE << 4) | (GPIO24_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0340
,(GPIO39_MODE << 28) | (GPIO38_MODE << 24) | (GPIO37_MODE << 20) | (GPIO36_MODE << 16) | (GPIO35_MODE << 12) | (GPIO34_MODE << 8) | (GPIO33_MODE << 4) | (GPIO32_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0350
,(GPIO47_MODE << 28) | (GPIO46_MODE << 24) | (GPIO45_MODE << 20) | (GPIO44_MODE << 16) | (GPIO43_MODE << 12) | (GPIO42_MODE << 8) | (GPIO41_MODE << 4) | (GPIO40_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0360
,(GPIO55_MODE << 28) | (GPIO54_MODE << 24) | (GPIO53_MODE << 20) | (GPIO52_MODE << 16) | (GPIO51_MODE << 12) | (GPIO50_MODE << 8) | (GPIO49_MODE << 4) | (GPIO48_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0370
,(GPIO63_MODE << 28) | (GPIO62_MODE << 24) | (GPIO61_MODE << 20) | (GPIO60_MODE << 16) | (GPIO59_MODE << 12) | (GPIO58_MODE << 8) | (GPIO57_MODE << 4) | (GPIO56_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0380
,((GPIO71_MODE==GPIO_MODE_01)?0:(GPIO71_MODE << 28)) | ((GPIO70_MODE==GPIO_MODE_01)?0:(GPIO70_MODE << 24)) | ((GPIO69_MODE==GPIO_MODE_01)?0:(GPIO69_MODE << 20)) | ((GPIO68_MODE==GPIO_MODE_01)?0:(GPIO68_MODE << 16)) | ((GPIO67_MODE==GPIO_MODE_01)?0:(GPIO67_MODE << 12)) | (GPIO66_MODE << 8) | (GPIO65_MODE << 4) | (GPIO64_MODE << 0)
,((GPIO71_MODE==GPIO_MODE_01)?0:(0xF << 28)) | ((GPIO70_MODE==GPIO_MODE_01)?0:(0xF << 24)) | ((GPIO69_MODE==GPIO_MODE_01)?0:(0xF << 20)) | ((GPIO68_MODE==GPIO_MODE_01)?0:(0xF << 16)) | ((GPIO67_MODE==GPIO_MODE_01)?0:(0xF << 12)) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0390
,((GPIO79_MODE==GPIO_MODE_01)?0:(GPIO79_MODE << 28)) | ((GPIO78_MODE==GPIO_MODE_01)?0:(GPIO78_MODE << 24)) | ((GPIO77_MODE==GPIO_MODE_01)?0:(GPIO77_MODE << 20)) | ((GPIO76_MODE==GPIO_MODE_01)?0:(GPIO76_MODE << 16)) | ((GPIO75_MODE==GPIO_MODE_01)?0:(GPIO75_MODE << 12)) | ((GPIO74_MODE==GPIO_MODE_01)?0:(GPIO74_MODE << 8)) | ((GPIO73_MODE==GPIO_MODE_01)?0:(GPIO73_MODE << 4)) | ((GPIO72_MODE==GPIO_MODE_01)?0:(GPIO72_MODE << 0))
,((GPIO79_MODE==GPIO_MODE_01)?0:(0xF << 28)) | ((GPIO78_MODE==GPIO_MODE_01)?0:(0xF << 24)) | ((GPIO77_MODE==GPIO_MODE_01)?0:(0xF << 20)) | ((GPIO76_MODE==GPIO_MODE_01)?0:(0xF << 16)) | ((GPIO75_MODE==GPIO_MODE_01)?0:(0xF << 12)) | ((GPIO74_MODE==GPIO_MODE_01)?0:(0xF << 8)) | ((GPIO73_MODE==GPIO_MODE_01)?0:(0xF << 4)) | ((GPIO72_MODE==GPIO_MODE_01)?0:(0xF << 0))
},
{
GPIO_BASE+0x03A0
,(GPIO87_MODE << 28) | ((GPIO86_MODE==GPIO_MODE_01)?0:(GPIO86_MODE << 24)) | ((GPIO85_MODE==GPIO_MODE_01)?0:(GPIO85_MODE << 20)) | ((GPIO84_MODE==GPIO_MODE_01)?0:(GPIO84_MODE << 16)) | ((GPIO83_MODE==GPIO_MODE_01)?0:(GPIO83_MODE << 12)) | ((GPIO82_MODE==GPIO_MODE_01)?0:(GPIO82_MODE << 8)) | ((GPIO81_MODE==GPIO_MODE_01)?0:(GPIO81_MODE << 4)) | ((GPIO80_MODE==GPIO_MODE_01)?0:(GPIO80_MODE << 0))
,(0xF << 28) | ((GPIO86_MODE==GPIO_MODE_01)?0:(0xF << 24)) | ((GPIO85_MODE==GPIO_MODE_01)?0:(0xF << 20)) | ((GPIO84_MODE==GPIO_MODE_01)?0:(0xF << 16)) | ((GPIO83_MODE==GPIO_MODE_01)?0:(0xF << 12)) | ((GPIO82_MODE==GPIO_MODE_01)?0:(0xF << 8)) | ((GPIO81_MODE==GPIO_MODE_01)?0:(0xF << 4)) | ((GPIO80_MODE==GPIO_MODE_01)?0:(0xF << 0))
},
{
GPIO_BASE+0x03B0
,(GPIO95_MODE << 28) | (GPIO94_MODE << 24) | (GPIO93_MODE << 20) | (GPIO92_MODE << 16) | (GPIO91_MODE << 12) | (GPIO90_MODE << 8) | (GPIO89_MODE << 4) | (GPIO88_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x03C0
,(GPIO103_MODE << 28) | (GPIO102_MODE << 24) | (GPIO101_MODE << 20) | (GPIO100_MODE << 16) | (GPIO99_MODE << 12) | (GPIO98_MODE << 8) | (GPIO97_MODE << 4) | (GPIO96_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x03D0
,(GPIO111_MODE << 28) | (GPIO110_MODE << 24) | (GPIO109_MODE << 20) | (GPIO108_MODE << 16) | (GPIO107_MODE << 12) | (GPIO106_MODE << 8) | (GPIO105_MODE << 4) | (GPIO104_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x03E0
,(GPIO119_MODE << 28) | (GPIO118_MODE << 24) | (GPIO117_MODE << 20) | (GPIO116_MODE << 16) | (GPIO115_MODE << 12) | (GPIO114_MODE << 8) | (GPIO113_MODE << 4) | (GPIO112_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x03F0
,(GPIO127_MODE << 28) | (GPIO126_MODE << 24) | (GPIO124_MODE << 16) | (GPIO123_MODE << 12) | (GPIO122_MODE << 8) | (GPIO121_MODE << 4) | (GPIO120_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0400
,(GPIO135_MODE << 28) | (GPIO134_MODE << 24) | (GPIO133_MODE << 20) | (GPIO132_MODE << 16) | (GPIO131_MODE << 12) | (GPIO130_MODE << 8) | (GPIO129_MODE << 4) | (GPIO128_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0410
,(GPIO143_MODE << 28) | (GPIO142_MODE << 24) | (GPIO141_MODE << 20) | (GPIO140_MODE << 16) | (GPIO139_MODE << 12) | (GPIO138_MODE << 8) | (GPIO137_MODE << 4) | (GPIO136_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0420
,(GPIO151_MODE << 28) | (GPIO150_MODE << 24) | (GPIO149_MODE << 20) | (GPIO148_MODE << 16) | (GPIO147_MODE << 12) | (GPIO146_MODE << 8) | (GPIO145_MODE << 4) | (GPIO144_MODE << 0)
,(0xF << 28) | (0xF << 24) | (0xF << 20) | (0xF << 16) | (0xF << 12) | (0xF << 8) | (0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0430
,(GPIO153_MODE << 4) | (GPIO152_MODE << 0)
,(0xF << 4) | (0xF << 0)
},
{
GPIO_BASE+0x0100
,(GPIO31_DATAOUT << 31) | (GPIO30_DATAOUT << 30) | (GPIO29_DATAOUT << 29) | (GPIO28_DATAOUT << 28) | (GPIO27_DATAOUT << 27) | (GPIO26_DATAOUT << 26) | (GPIO25_DATAOUT << 25) | (GPIO24_DATAOUT << 24) | (GPIO23_DATAOUT << 23) | (GPIO22_DATAOUT << 22) | (GPIO21_DATAOUT << 21) | (GPIO20_DATAOUT << 20) | (GPIO19_DATAOUT << 19) | (GPIO18_DATAOUT << 18) | (GPIO17_DATAOUT << 17) | (GPIO16_DATAOUT << 16) | (GPIO15_DATAOUT << 15) | (GPIO14_DATAOUT << 14) | (GPIO13_DATAOUT << 13) | (GPIO12_DATAOUT << 12) | (GPIO11_DATAOUT << 11) | (GPIO10_DATAOUT << 10) | (GPIO9_DATAOUT << 9) | (GPIO8_DATAOUT << 8) | (GPIO7_DATAOUT << 7) | (GPIO6_DATAOUT << 6) | (GPIO5_DATAOUT << 5) | (GPIO4_DATAOUT << 4) | (GPIO3_DATAOUT << 3) | (GPIO2_DATAOUT << 2) | (GPIO1_DATAOUT << 1) | (GPIO0_DATAOUT << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0110
,(GPIO63_DATAOUT << 31) | (GPIO62_DATAOUT << 30) | (GPIO61_DATAOUT << 29) | (GPIO60_DATAOUT << 28) | (GPIO59_DATAOUT << 27) | (GPIO58_DATAOUT << 26) | (GPIO57_DATAOUT << 25) | (GPIO56_DATAOUT << 24) | (GPIO55_DATAOUT << 23) | (GPIO54_DATAOUT << 22) | (GPIO53_DATAOUT << 21) | (GPIO52_DATAOUT << 20) | (GPIO51_DATAOUT << 19) | (GPIO50_DATAOUT << 18) | (GPIO49_DATAOUT << 17) | (GPIO48_DATAOUT << 16) | (GPIO47_DATAOUT << 15) | (GPIO46_DATAOUT << 14) | (GPIO45_DATAOUT << 13) | (GPIO44_DATAOUT << 12) | (GPIO43_DATAOUT << 11) | (GPIO42_DATAOUT << 10) | (GPIO41_DATAOUT << 9) | (GPIO40_DATAOUT << 8) | (GPIO39_DATAOUT << 7) | (GPIO38_DATAOUT << 6) | (GPIO37_DATAOUT << 5) | (GPIO36_DATAOUT << 4) | (GPIO35_DATAOUT << 3) | (GPIO34_DATAOUT << 2) | (GPIO33_DATAOUT << 1) | (GPIO32_DATAOUT << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0120
,(GPIO95_DATAOUT << 31) | (GPIO94_DATAOUT << 30) | (GPIO93_DATAOUT << 29) | (GPIO92_DATAOUT << 28) | (GPIO91_DATAOUT << 27) | (GPIO90_DATAOUT << 26) | (GPIO89_DATAOUT << 25) | (GPIO88_DATAOUT << 24) | (GPIO87_DATAOUT << 23) | (GPIO86_DATAOUT << 22) | (GPIO85_DATAOUT << 21) | (GPIO84_DATAOUT << 20) | (GPIO83_DATAOUT << 19) | (GPIO82_DATAOUT << 18) | (GPIO81_DATAOUT << 17) | (GPIO80_DATAOUT << 16) | (GPIO79_DATAOUT << 15) | (GPIO78_DATAOUT << 14) | (GPIO77_DATAOUT << 13) | (GPIO76_DATAOUT << 12) | (GPIO75_DATAOUT << 11) | (GPIO74_DATAOUT << 10) | (GPIO73_DATAOUT << 9) | (GPIO72_DATAOUT << 8) | (GPIO71_DATAOUT << 7) | (GPIO70_DATAOUT << 6) | (GPIO69_DATAOUT << 5) | (GPIO68_DATAOUT << 4) | (GPIO67_DATAOUT << 3) | (GPIO66_DATAOUT << 2) | (GPIO65_DATAOUT << 1) | (GPIO64_DATAOUT << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0130
,(GPIO127_DATAOUT << 31) | (GPIO126_DATAOUT << 30) | (GPIO124_DATAOUT << 28) | (GPIO123_DATAOUT << 27) | (GPIO122_DATAOUT << 26) | (GPIO121_DATAOUT << 25) | (GPIO120_DATAOUT << 24) | (GPIO119_DATAOUT << 23) | (GPIO118_DATAOUT << 22) | (GPIO117_DATAOUT << 21) | (GPIO116_DATAOUT << 20) | (GPIO115_DATAOUT << 19) | (GPIO114_DATAOUT << 18) | (GPIO113_DATAOUT << 17) | (GPIO112_DATAOUT << 16) | (GPIO111_DATAOUT << 15) | (GPIO110_DATAOUT << 14) | (GPIO109_DATAOUT << 13) | (GPIO108_DATAOUT << 12) | (GPIO107_DATAOUT << 11) | (GPIO106_DATAOUT << 10) | (GPIO105_DATAOUT << 9) | (GPIO104_DATAOUT << 8) | (GPIO103_DATAOUT << 7) | (GPIO102_DATAOUT << 6) | (GPIO101_DATAOUT << 5) | (GPIO100_DATAOUT << 4) | (GPIO99_DATAOUT << 3) | (GPIO98_DATAOUT << 2) | (GPIO97_DATAOUT << 1) | (GPIO96_DATAOUT << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0140
,(GPIO153_DATAOUT << 25) | (GPIO152_DATAOUT << 24) | (GPIO151_DATAOUT << 23) | (GPIO150_DATAOUT << 22) | (GPIO149_DATAOUT << 21) | (GPIO148_DATAOUT << 20) | (GPIO147_DATAOUT << 19) | (GPIO146_DATAOUT << 18) | (GPIO145_DATAOUT << 17) | (GPIO144_DATAOUT << 16) | (GPIO143_DATAOUT << 15) | (GPIO142_DATAOUT << 14) | (GPIO141_DATAOUT << 13) | (GPIO140_DATAOUT << 12) | (GPIO139_DATAOUT << 11) | (GPIO138_DATAOUT << 10) | (GPIO137_DATAOUT << 9) | (GPIO136_DATAOUT << 8) | (GPIO135_DATAOUT << 7) | (GPIO134_DATAOUT << 6) | (GPIO133_DATAOUT << 5) | (GPIO132_DATAOUT << 4) | (GPIO131_DATAOUT << 3) | (GPIO130_DATAOUT << 2) | (GPIO129_DATAOUT << 1) | (GPIO128_DATAOUT << 0)
,(0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0000
,(GPIO31_DIR << 31) | (GPIO30_DIR << 30) | (GPIO29_DIR << 29) | (GPIO28_DIR << 28) | (GPIO27_DIR << 27) | (GPIO26_DIR << 26) | (GPIO25_DIR << 25) | (GPIO24_DIR << 24) | (GPIO23_DIR << 23) | (GPIO22_DIR << 22) | (GPIO21_DIR << 21) | (GPIO20_DIR << 20) | (GPIO19_DIR << 19) | (GPIO18_DIR << 18) | (GPIO17_DIR << 17) | (GPIO16_DIR << 16) | (GPIO15_DIR << 15) | (GPIO14_DIR << 14) | (GPIO13_DIR << 13) | (GPIO12_DIR << 12) | (GPIO11_DIR << 11) | (GPIO10_DIR << 10) | (GPIO9_DIR << 9) | (GPIO8_DIR << 8) | (GPIO7_DIR << 7) | (GPIO6_DIR << 6) | (GPIO5_DIR << 5) | (GPIO4_DIR << 4) | (GPIO3_DIR << 3) | (GPIO2_DIR << 2) | (GPIO1_DIR << 1) | (GPIO0_DIR << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0010
,(GPIO63_DIR << 31) | (GPIO62_DIR << 30) | (GPIO61_DIR << 29) | (GPIO60_DIR << 28) | (GPIO59_DIR << 27) | (GPIO58_DIR << 26) | (GPIO57_DIR << 25) | (GPIO56_DIR << 24) | (GPIO55_DIR << 23) | (GPIO54_DIR << 22) | (GPIO53_DIR << 21) | (GPIO52_DIR << 20) | (GPIO51_DIR << 19) | (GPIO50_DIR << 18) | (GPIO49_DIR << 17) | (GPIO48_DIR << 16) | (GPIO47_DIR << 15) | (GPIO46_DIR << 14) | (GPIO45_DIR << 13) | (GPIO44_DIR << 12) | (GPIO43_DIR << 11) | (GPIO42_DIR << 10) | (GPIO41_DIR << 9) | (GPIO40_DIR << 8) | (GPIO39_DIR << 7) | (GPIO38_DIR << 6) | (GPIO37_DIR << 5) | (GPIO36_DIR << 4) | (GPIO35_DIR << 3) | (GPIO34_DIR << 2) | (GPIO33_DIR << 1) | (GPIO32_DIR << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0020
,(GPIO95_DIR << 31) | (GPIO94_DIR << 30) | (GPIO93_DIR << 29) | (GPIO92_DIR << 28) | (GPIO91_DIR << 27) | (GPIO90_DIR << 26) | (GPIO89_DIR << 25) | (GPIO88_DIR << 24) | (GPIO87_DIR << 23) | (GPIO86_DIR << 22) | (GPIO85_DIR << 21) | (GPIO84_DIR << 20) | (GPIO83_DIR << 19) | (GPIO82_DIR << 18) | (GPIO81_DIR << 17) | (GPIO80_DIR << 16) | (GPIO79_DIR << 15) | (GPIO78_DIR << 14) | (GPIO77_DIR << 13) | (GPIO76_DIR << 12) | (GPIO75_DIR << 11) | (GPIO74_DIR << 10) | (GPIO73_DIR << 9) | (GPIO72_DIR << 8) | (GPIO71_DIR << 7) | (GPIO70_DIR << 6) | (GPIO69_DIR << 5) | (GPIO68_DIR << 4) | (GPIO67_DIR << 3) | (GPIO66_DIR << 2) | (GPIO65_DIR << 1) | (GPIO64_DIR << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0030
,(GPIO127_DIR << 31) | (GPIO126_DIR << 30) | (GPIO124_DIR << 28) | (GPIO123_DIR << 27) | (GPIO122_DIR << 26) | (GPIO121_DIR << 25) | (GPIO120_DIR << 24) | (GPIO119_DIR << 23) | (GPIO118_DIR << 22) | (GPIO117_DIR << 21) | (GPIO116_DIR << 20) | (GPIO115_DIR << 19) | (GPIO114_DIR << 18) | (GPIO113_DIR << 17) | (GPIO112_DIR << 16) | (GPIO111_DIR << 15) | (GPIO110_DIR << 14) | (GPIO109_DIR << 13) | (GPIO108_DIR << 12) | (GPIO107_DIR << 11) | (GPIO106_DIR << 10) | (GPIO105_DIR << 9) | (GPIO104_DIR << 8) | (GPIO103_DIR << 7) | (GPIO102_DIR << 6) | (GPIO101_DIR << 5) | (GPIO100_DIR << 4) | (GPIO99_DIR << 3) | (GPIO98_DIR << 2) | (GPIO97_DIR << 1) | (GPIO96_DIR << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
GPIO_BASE+0x0040
,(GPIO153_DIR << 25) | (GPIO152_DIR << 24) | (GPIO151_DIR << 23) | (GPIO150_DIR << 22) | (GPIO149_DIR << 21) | (GPIO148_DIR << 20) | (GPIO147_DIR << 19) | (GPIO146_DIR << 18) | (GPIO145_DIR << 17) | (GPIO144_DIR << 16) | (GPIO143_DIR << 15) | (GPIO142_DIR << 14) | (GPIO141_DIR << 13) | (GPIO140_DIR << 12) | (GPIO139_DIR << 11) | (GPIO138_DIR << 10) | (GPIO137_DIR << 9) | (GPIO136_DIR << 8) | (GPIO135_DIR << 7) | (GPIO134_DIR << 6) | (GPIO133_DIR << 5) | (GPIO132_DIR << 4) | (GPIO131_DIR << 3) | (GPIO130_DIR << 2) | (GPIO129_DIR << 1) | (GPIO128_DIR << 0)
,(0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_L_BASE+0x0090
,(GPIO13_DATAINV << 13) | (GPIO12_DATAINV << 12) | (GPIO11_DATAINV << 10) | (GPIO10_DATAINV << 9) | (GPIO9_DATAINV << 8) | (GPIO8_DATAINV << 7) | (GPIO7_DATAINV << 11) | (GPIO6_DATAINV << 6) | (GPIO5_DATAINV << 5) | (GPIO4_DATAINV << 4) | (GPIO3_DATAINV << 3) | (GPIO2_DATAINV << 2) | (GPIO1_DATAINV << 1) | (GPIO0_DATAINV << 0)
,(0x1 << 13) | (0x1 << 12) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 11) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_L_BASE+0x00A0
,(GPIO39_DATAINV << 25) | (GPIO38_DATAINV << 24) | (GPIO37_DATAINV << 23) | (GPIO36_DATAINV << 22) | (GPIO35_DATAINV << 21) | (GPIO34_DATAINV << 20) | (GPIO33_DATAINV << 19) | (GPIO32_DATAINV << 18) | (GPIO31_DATAINV << 17) | (GPIO30_DATAINV << 16) | (GPIO29_DATAINV << 15) | (GPIO28_DATAINV << 14) | (GPIO27_DATAINV << 13) | (GPIO26_DATAINV << 12) | (GPIO25_DATAINV << 11) | (GPIO24_DATAINV << 10) | (GPIO23_DATAINV << 9) | (GPIO22_DATAINV << 8) | (GPIO21_DATAINV << 7) | (GPIO20_DATAINV << 6) | (GPIO19_DATAINV << 5) | (GPIO18_DATAINV << 4) | (GPIO17_DATAINV << 3) | (GPIO16_DATAINV << 2) | (GPIO15_DATAINV << 1) | (GPIO14_DATAINV << 0)
,(0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_B_BASE+0x0070
,(GPIO54_DATAINV << 10) | (GPIO53_DATAINV << 9) | (GPIO52_DATAINV << 8) | (GPIO51_DATAINV << 7) | (GPIO50_DATAINV << 6) | (GPIO49_DATAINV << 5) | (GPIO48_DATAINV << 4) | (GPIO47_DATAINV << 3) | (GPIO46_DATAINV << 2) | (GPIO45_DATAINV << 1) | (GPIO44_DATAINV << 0) | (GPIO43_DATAINV << 24) | (GPIO42_DATAINV << 23) | (GPIO41_DATAINV << 22) | (GPIO40_DATAINV << 21)
,(0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21)
},
{
IO_CFG_R_BASE+0x0070
,(GPIO106_DATAINV << 31) | (GPIO105_DATAINV << 30) | (GPIO104_DATAINV << 29) | (GPIO103_DATAINV << 28) | (GPIO102_DATAINV << 27) | (GPIO101_DATAINV << 26) | (GPIO100_DATAINV << 25) | (GPIO99_DATAINV << 24) | (GPIO98_DATAINV << 23) | (GPIO97_DATAINV << 22) | (GPIO96_DATAINV << 21) | (GPIO95_DATAINV << 20) | (GPIO94_DATAINV << 19) | (GPIO93_DATAINV << 18) | (GPIO92_DATAINV << 17) | (GPIO91_DATAINV << 16) | (GPIO90_DATAINV << 15) | (GPIO89_DATAINV << 14) | (GPIO88_DATAINV << 13) | (GPIO87_DATAINV << 12) | (GPIO66_DATAINV << 11) | (GPIO65_DATAINV << 10) | (GPIO64_DATAINV << 9) | (GPIO63_DATAINV << 8) | (GPIO62_DATAINV << 7) | (GPIO61_DATAINV << 6) | (GPIO60_DATAINV << 5) | (GPIO59_DATAINV << 4) | (GPIO58_DATAINV << 3) | (GPIO57_DATAINV << 2) | (GPIO56_DATAINV << 1) | (GPIO55_DATAINV << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_R_BASE+0x0080
,((GPIO86_MODE==GPIO_MODE_01)?0:(GPIO86_DATAINV << 7)) | ((GPIO85_MODE==GPIO_MODE_01)?0:(GPIO85_DATAINV << 6)) | ((GPIO84_MODE==GPIO_MODE_01)?0:(GPIO84_DATAINV << 5)) | ((GPIO83_MODE==GPIO_MODE_01)?0:(GPIO83_DATAINV << 4)) | ((GPIO82_MODE==GPIO_MODE_01)?0:(GPIO82_DATAINV << 3)) | ((GPIO81_MODE==GPIO_MODE_01)?0:(GPIO81_DATAINV << 2)) | ((GPIO80_MODE==GPIO_MODE_01)?0:(GPIO80_DATAINV << 1)) | ((GPIO79_MODE==GPIO_MODE_01)?0:(GPIO79_DATAINV << 0)) | ((GPIO78_MODE==GPIO_MODE_01)?0:(GPIO78_DATAINV << 13)) | ((GPIO77_MODE==GPIO_MODE_01)?0:(GPIO77_DATAINV << 12)) | ((GPIO76_MODE==GPIO_MODE_01)?0:(GPIO76_DATAINV << 11)) | ((GPIO75_MODE==GPIO_MODE_01)?0:(GPIO75_DATAINV << 10)) | ((GPIO74_MODE==GPIO_MODE_01)?0:(GPIO74_DATAINV << 9)) | ((GPIO73_MODE==GPIO_MODE_01)?0:(GPIO73_DATAINV << 8)) | ((GPIO72_MODE==GPIO_MODE_01)?0:(GPIO72_DATAINV << 19)) | ((GPIO71_MODE==GPIO_MODE_01)?0:(GPIO71_DATAINV << 18)) | ((GPIO70_MODE==GPIO_MODE_01)?0:(GPIO70_DATAINV << 17)) | ((GPIO69_MODE==GPIO_MODE_01)?0:(GPIO69_DATAINV << 16)) | ((GPIO68_MODE==GPIO_MODE_01)?0:(GPIO68_DATAINV << 15)) | ((GPIO67_MODE==GPIO_MODE_01)?0:(GPIO67_DATAINV << 14))
,((GPIO86_MODE==GPIO_MODE_01)?0:(0x1 << 7)) | ((GPIO85_MODE==GPIO_MODE_01)?0:(0x1 << 6)) | ((GPIO84_MODE==GPIO_MODE_01)?0:(0x1 << 5)) | ((GPIO83_MODE==GPIO_MODE_01)?0:(0x1 << 4)) | ((GPIO82_MODE==GPIO_MODE_01)?0:(0x1 << 3)) | ((GPIO81_MODE==GPIO_MODE_01)?0:(0x1 << 2)) | ((GPIO80_MODE==GPIO_MODE_01)?0:(0x1 << 1)) | ((GPIO79_MODE==GPIO_MODE_01)?0:(0x1 << 0)) | ((GPIO78_MODE==GPIO_MODE_01)?0:(0x1 << 13)) | ((GPIO77_MODE==GPIO_MODE_01)?0:(0x1 << 12)) | ((GPIO76_MODE==GPIO_MODE_01)?0:(0x1 << 11)) | ((GPIO75_MODE==GPIO_MODE_01)?0:(0x1 << 10)) | ((GPIO74_MODE==GPIO_MODE_01)?0:(0x1 << 9)) | ((GPIO73_MODE==GPIO_MODE_01)?0:(0x1 << 8)) | ((GPIO72_MODE==GPIO_MODE_01)?0:(0x1 << 19)) | ((GPIO71_MODE==GPIO_MODE_01)?0:(0x1 << 18)) | ((GPIO70_MODE==GPIO_MODE_01)?0:(0x1 << 17)) | ((GPIO69_MODE==GPIO_MODE_01)?0:(0x1 << 16)) | ((GPIO68_MODE==GPIO_MODE_01)?0:(0x1 << 15)) | ((GPIO67_MODE==GPIO_MODE_01)?0:(0x1 << 14))
},
{
IO_CFG_T_BASE+0x00E0
,(GPIO153_DATAINV << 31) | (GPIO152_DATAINV << 30) | (GPIO151_DATAINV << 29) | (GPIO150_DATAINV << 28) | (GPIO149_DATAINV << 27) | (GPIO148_DATAINV << 26) | (GPIO147_DATAINV << 25) | (GPIO146_DATAINV << 24) | (GPIO145_DATAINV << 23) | (GPIO144_DATAINV << 22) | (GPIO143_DATAINV << 21) | (GPIO142_DATAINV << 20) | (GPIO141_DATAINV << 19) | (GPIO140_DATAINV << 18) | (GPIO139_DATAINV << 17) | (GPIO138_DATAINV << 16) | (GPIO137_DATAINV << 15) | (GPIO136_DATAINV << 14) | (GPIO135_DATAINV << 13) | (GPIO134_DATAINV << 12) | (GPIO133_DATAINV << 11) | (GPIO132_DATAINV << 10) | (GPIO131_DATAINV << 9) | (GPIO130_DATAINV << 8) | (GPIO114_DATAINV << 7) | (GPIO113_DATAINV << 6) | (GPIO112_DATAINV << 5) | (GPIO111_DATAINV << 4) | (GPIO110_DATAINV << 3) | (GPIO109_DATAINV << 2) | (GPIO108_DATAINV << 1) | (GPIO107_DATAINV << 0)
,(0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x1 << 25) | (0x1 << 24) | (0x1 << 23) | (0x1 << 22) | (0x1 << 21) | (0x1 << 20) | (0x1 << 19) | (0x1 << 18) | (0x1 << 17) | (0x1 << 16) | (0x1 << 15) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x0100
,(GPIO127_DATAINV << 12) | (GPIO126_DATAINV << 11) | (GPIO124_DATAINV << 9) | (GPIO123_DATAINV << 8) | (GPIO122_DATAINV << 7) | (GPIO121_DATAINV << 6) | (GPIO120_DATAINV << 5) | (GPIO119_DATAINV << 4) | (GPIO118_DATAINV << 3) | (GPIO117_DATAINV << 2) | (GPIO116_DATAINV << 1) | (GPIO115_DATAINV << 0)
,(0x1 << 12) | (0x1 << 11) | (0x1 << 9) | (0x1 << 8) | (0x1 << 7) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_T_BASE+0x00F0
,(GPIO129_DATAINV << 1) | (GPIO128_DATAINV << 0)
,(0x1 << 1) | (0x1 << 0)
},
{
IO_CFG_R_BASE+0x0030
,(((GPIO_DVDD3_LCD==GPIO_VIO18)?0x0:0x5) << 0) | (((MSDC_POWER_MC1==MSDC_VIO18_MC1)?0x0:0x5) << 8)
,(0xF << 0) | (0xF << 8)
},
{
IO_CFG_T_BASE+0x0064
,(((GPIO_DVDD28_BPI==GPIO_VIO18)?0x0:0xC) << 2)
,(0x3F << 2)
},
{
IO_CFG_R_BASE+0x0034
,(((GPIO_DVDD3_LCD==GPIO_VIO18)?0x0:0xC) << 0) | (((MSDC_POWER_MC1==MSDC_VIO18_MC1)?0x0:0xC) << 10)
,(0x3F << 0) | (0x3F << 10)
},
{
IO_CFG_T_BASE+0x0060
,(((GPIO_DVDD28_BPI==GPIO_VIO18)?0x0:0x5) << 4)
,(0xF << 4)
}
};

#endif //_GPIO_INIT_H_
