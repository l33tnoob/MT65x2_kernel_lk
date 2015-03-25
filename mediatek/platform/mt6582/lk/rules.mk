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
	   
ifeq ($(MTK_SEC_FASTBOOT_UNLOCK_SUPPORT), yes)
	DEFINES += MTK_SEC_FASTBOOT_UNLOCK_SUPPORT
endif	   

INCLUDES += -I$(LOCAL_DIR)/include \
            -Ilib/libshowlogo \
	    -Iout/lk/lcm/inc \
	    -Iout/lk/inc \
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
	$(LOCAL_DIR)/mmc_core.o \
	$(LOCAL_DIR)/mmc_test.o \
	$(LOCAL_DIR)/msdc_utils.o \
	$(LOCAL_DIR)/msdc.o \
	$(LOCAL_DIR)/factory.o \
	$(LOCAL_DIR)/mt_pmic.o \
	$(LOCAL_DIR)/upmu_common.o \
	$(LOCAL_DIR)/mt_gpt.o\
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
	$(LOCAL_DIR)/fpc_sw_repair2sw.o\
	
ifeq ($(MTK_MT6333_SUPPORT),yes)
  OBJS += $(LOCAL_DIR)/mtk_pmic_6333.o
endif

ifeq ($(MTK_FAN5405_SUPPORT),yes)
	OBJS += $(LOCAL_DIR)/fan5405.o
	OBJS += $(LOCAL_DIR)/mt_battery.o
else
  ifeq ($(MTK_BQ24196_SUPPORT),yes)
      OBJS += $(LOCAL_DIR)/bq24196.o
      OBJS += $(LOCAL_DIR)/mt_battery.o        
  else
    ifeq ($(MTK_NCP1851_SUPPORT),yes)
        OBJS += $(LOCAL_DIR)/ncp1851.o
        OBJS += $(LOCAL_DIR)/mt_battery.o     
    else
        ifeq ($(MTK_BQ24158_SUPPORT),yes)
            OBJS += $(LOCAL_DIR)/bq24158.o
	OBJS += $(LOCAL_DIR)/mt_battery.o
        else
          ifeq ($(MTK_NCP1854_SUPPORT),yes)
            OBJS += $(LOCAL_DIR)/ncp1854.o
            OBJS += $(LOCAL_DIR)/mt_battery.o        
          else
            ifeq ($(MTK_BQ24296_SUPPORT),yes)
            OBJS += $(LOCAL_DIR)/bq24296.o
            OBJS += $(LOCAL_DIR)/mt_battery.o        
            else
            OBJS += $(LOCAL_DIR)/mt_battery.o
            endif
          endif
        endif
    endif
  endif
endif

ifneq ($(MTK_EMMC_SUPPORT),yes)
	OBJS +=$(LOCAL_DIR)/mtk_nand.o
	OBJS +=$(LOCAL_DIR)/bmt.o
endif


ifeq ($(MTK_MT8193_SUPPORT),yes)
OBJS +=$(LOCAL_DIR)/mt8193_init.o
OBJS +=$(LOCAL_DIR)/mt8193_ckgen.o
OBJS +=$(LOCAL_DIR)/mt8193_i2c.o
ifneq ($(MTK_TB_WIFI_3G_MODE),WIFI_ONLY)
CFLAGS += -DMT8193_DISABLE_EXT_CLK_BUF
endif
endif

ifeq ($(MTK_KERNEL_POWER_OFF_CHARGING),yes)
OBJS +=$(LOCAL_DIR)/mt_kernel_power_off_charging.o
endif

ifeq ($(DUMMY_AP),yes)
OBJS +=$(LOCAL_DIR)/dummy_ap.o
endif

ifeq ($(CUSTOM_SEC_AUTH_SUPPORT), yes)
LIBSEC := -L$(LOCAL_DIR)/lib -lsec
else
LIBSEC := -L$(LOCAL_DIR)/lib -lsec -lauth
endif
LIBSEC_PLAT := -lsplat -ldevinfo

LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld
