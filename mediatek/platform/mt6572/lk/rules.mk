LOCAL_DIR := $(GET_LOCAL_DIR)

ARCH    := arm
ARM_CPU := cortex-a7
CPU     := generic

MMC_SLOT         := 1

DEFINES += PERIPH_BLK_BLSP=1 
DEFINES += WITH_CPU_EARLY_INIT=0 WITH_CPU_WARM_BOOT=0 \
	   MMC_SLOT=$(MMC_SLOT)

ifeq ($(MTK_SECURITY_SW_SUPPORT), yes)
	DEFINES += MTK_SECURITY_SW_SUPPORT
endif	   

$(info libshowlogo new path ------- $(LOCAL_DIR)/../../../../bootable/bootloader/lk/lib/libshowlogo)

INCLUDES += -I$(LOCAL_DIR)/include \
	    -I$(LOCAL_DIR)/include/platform \
            -I$(LOCAL_DIR)/../../../../bootable/bootloader/lk/lib/libshowlogo \
	    -Iout/lk/include/target \
	    -Iout/lk/lcm/inc \
	    -Iout/lk/inc \
	    -Iout/common \
	    -Iout/kernel/dct/ \
	    -I$(BUILDDIR)/include/dfo
#for ptgen
INCLUDES += -I../../../$(MTK_ROOT_OUT)/PTGEN/lk/inc
INCLUDES += -I../../../$(MTK_ROOT_OUT)/PTGEN/common
#for nandgen
INCLUDES += -I../../../$(MTK_ROOT_OUT)/NANDGEN/common

OBJS += \
	$(LOCAL_DIR)/bitops.o \
	$(LOCAL_DIR)/platform.o \
	$(LOCAL_DIR)/pwm.o \
	$(LOCAL_DIR)/uart.o \
	$(LOCAL_DIR)/interrupts.o \
	$(LOCAL_DIR)/timer.o \
	$(LOCAL_DIR)/debug.o \
	$(LOCAL_DIR)/mt_i2c.o \
	$(LOCAL_DIR)/boot_mode.o \
	$(LOCAL_DIR)/load_image.o \
	$(LOCAL_DIR)/atags.o \
	$(LOCAL_DIR)/mt_partition.o \
	$(LOCAL_DIR)/addr_trans.o \
	$(LOCAL_DIR)/mmc_common_inter.o \
	$(LOCAL_DIR)/mmc_core.o \
	$(LOCAL_DIR)/mmc_test.o \
	$(LOCAL_DIR)/mmc_dbg.o \
	$(LOCAL_DIR)/msdc.o \
	$(LOCAL_DIR)/msdc_dma.o \
	$(LOCAL_DIR)/msdc_irq.o \
	$(LOCAL_DIR)/msdc_utils.o \
	$(LOCAL_DIR)/pll.o \
	$(LOCAL_DIR)/factory.o \
	$(LOCAL_DIR)/mt_pmic.o \
	$(LOCAL_DIR)/upmu_common.o \
	$(LOCAL_DIR)/mt_gpt.o\
	$(LOCAL_DIR)/mtk_sleep.o\
	$(LOCAL_DIR)/mt_rtc.o\
	$(LOCAL_DIR)/mt_usb.o\
	$(LOCAL_DIR)/mtk_auxadc.o \
	$(LOCAL_DIR)/mtk_key.o \
	$(LOCAL_DIR)/mt_disp_drv.o\
	$(LOCAL_DIR)/disp_drv.o\
	$(LOCAL_DIR)/disp_assert_layer.o\
	$(LOCAL_DIR)/disp_drv_dbi.o\
	$(LOCAL_DIR)/disp_drv_dpi.o\
	$(LOCAL_DIR)/disp_drv_dsi.o\
	$(LOCAL_DIR)/lcd_drv.o\
	$(LOCAL_DIR)/dpi_drv.o\
	$(LOCAL_DIR)/dsi_drv.o\
	$(LOCAL_DIR)/partition_mt.o\
  $(LOCAL_DIR)/mt_get_dl_info.o \
	$(LOCAL_DIR)/mtk_wdt.o\
	$(LOCAL_DIR)/mt_leds.o\
	$(LOCAL_DIR)/recovery.o\
	$(LOCAL_DIR)/meta.o\
	$(LOCAL_DIR)/mt_logo.o\
	$(LOCAL_DIR)/mt_gpio.o\
	$(LOCAL_DIR)/mt_gpio_init.o\
	$(LOCAL_DIR)/boot_mode_menu.o\
	$(LOCAL_DIR)/mt_pmic_wrap_init.o\
	$(LOCAL_DIR)/ddp_rdma.o\
	$(LOCAL_DIR)/ddp_wdma.o\
	$(LOCAL_DIR)/ddp_ovl.o\
	$(LOCAL_DIR)/ddp_bls.o\
	$(LOCAL_DIR)/ddp_path.o\
	$(LOCAL_DIR)/env.o\
	

ifeq ($(MTK_FAN5405_SUPPORT),yes)
	OBJS += $(LOCAL_DIR)/fan5405.o
	OBJS += $(LOCAL_DIR)/mt_battery.o
else
  ifeq ($(MTK_BQ24196_SUPPORT),yes)
      OBJS += $(LOCAL_DIR)/bq24196.o
        ifeq ($(MTK_BQ27541_SUPPORT),yes)
          OBJS += $(LOCAL_DIR)/bq27541.o      
        endif
      OBJS += $(LOCAL_DIR)/mt_bat_bq24196.o        
  else
    ifeq ($(MTK_NCP1851_SUPPORT),yes)
        OBJS += $(LOCAL_DIR)/ncp1851.o
        OBJS += $(LOCAL_DIR)/mt_bat_ncp1851.o     
    else
	OBJS += $(LOCAL_DIR)/mt_battery.o
    endif
  endif
endif

ifneq ($(MTK_EMMC_SUPPORT),yes)
	ifeq ($(MTK_SPI_NAND_SUPPORT),yes)
		OBJS +=$(LOCAL_DIR)/mtk_snand_lk.o
	else
	OBJS +=$(LOCAL_DIR)/mtk_nand.o
	endif
	OBJS +=$(LOCAL_DIR)/bmt.o
endif


ifeq ($(MTK_MT8193_SUPPORT),yes)
OBJS +=$(LOCAL_DIR)/mt8193_init.o
OBJS +=$(LOCAL_DIR)/mt8193_ckgen.o
OBJS +=$(LOCAL_DIR)/mt8193_i2c.o
endif

ifeq ($(MTK_KERNEL_POWER_OFF_CHARGING),yes)
OBJS +=$(LOCAL_DIR)/mt_kernel_power_off_charging.o
endif

ifeq ($(CUSTOM_SEC_AUTH_SUPPORT), yes)
LIBSEC := -L$(LOCAL_DIR)/lib -lsec
else
LIBSEC := -L$(LOCAL_DIR)/lib -lsec -lauth
endif
LIBSEC_PLAT := -lsplat -ldevinfo

LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld
