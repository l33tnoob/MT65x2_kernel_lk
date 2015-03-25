###################################################################
# flag
###################################################################

MOD_SRC  = 
MOD_OBJ  = $(MTK_ROOT_OUT)/PRELOADER_OBJ/obj

###################################################################
# source files
# - add your file here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
###################################################################

MOD_SRC  := \
	circbuf.c \
	dramc_dqs_gw.c  \
	dramc_dle_cal.c \
	dramc_calib.c \
	mmc_common_inter.c \
	mmc_core.c \
	mmc_test.c \
	msdc.c \
	msdc_utils.c \
	i2c.c \
	keypad.c \
	usbd.c \
	usbphy.c \
	usbtty.c \
	emi.c \
	timer.c \
	wdt.c \
	partition.c \
	platform.c \
	pll.c \
	device_apc.c \
	gpio.c \
	uart.c \
 	pmic_wrap_init.c \
	pmic_6323.c \
	rtc.c \
	fpc_sw_repair2sw.c \
	memory.c \
	memory_test.s \

ifeq ("$(MTK_MT6333_SUPPORT)","yes")	
	MOD_SRC  +=	pmic_6333.c
endif

ifeq ("$(MTK_FAN5405_SUPPORT)","yes")	
	MOD_SRC  +=	fan5405.c
endif

ifneq ("$(MTK_EMMC_SUPPORT)","yes")
	MOD_SRC  += 	nand.c 
	MOD_SRC  +=	bmt.c
endif

ifeq ("$(PL_MODE)","DUMMY_AP")
	MOD_SRC  += gpio_init.c
endif

ifeq ("$(PL_MODE)","TINY")
	MOD_SRC  += gpio_init.c
endif

ifeq ("$(MTK_MT8193_SUPPORT)","yes")
  MOD_SRC  += mt8193_init.c
	MOD_SRC  += mt8193_ckgen.c
	MOD_SRC  += mt8193_i2c.c
endif


###################################################################
# target
###################################################################

include $(COMMON_FILE_MK)
