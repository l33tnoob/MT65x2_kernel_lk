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
	mmc_common_inter.c \
	mmc_core.c \
	mmc_test.c \
	mmc_dbg.c \
	msdc.c \
	msdc_irq.c \
	msdc_utils.c \
	mt_i2c.c \
	mtk_key.c \
	mt_usbd.c \
	mt_usbphy.c \
	mt_usbtty.c \
	mt_emi.c \
	mtk_timer.c \
	mtk_wdt.c \
	partition_mt.c \
	platform.c \
	mtk_pll.c \
	device_apc.c \
	gpio.c \
	mt_gpio_init.c \
	uart.c \
 	mt_pmic_wrap_init.c \
	mtk_pmic_6323.c \
	mt_rtc.c \
	mt_pl_ptp.c \
	mt_mmsys_mfg_onoff.c   \
	mt_mmmfg_sram_repair.c
	
ifneq ("$(MTK_EMMC_SUPPORT)","yes")
	ifeq ("$(MTK_SPI_NAND_SUPPORT)","yes")
		MOD_SRC  += mtk_snand_pl.c 
	else
	MOD_SRC  += 	mtk_nand.c 
	endif
	MOD_SRC  +=	bmt.c
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
