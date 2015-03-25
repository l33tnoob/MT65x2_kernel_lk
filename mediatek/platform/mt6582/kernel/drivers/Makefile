include $(MTK_PATH_BUILD)/common.mk
#Fix-me: modified for early porting
#$(call all-subdir-src-or-makefile)
obj-y   += uart/
obj-y   += gpio/
obj-y   += hdmitx/
obj-y   += pwm/
obj-y   += dual_ccci/
obj-y   += btcvsd/
obj-y   += pmic_wrap/
obj-y   += power/
obj-y   += power_gs/
obj-y   += m4u/
obj-y   += smi/
obj-y   += usb20/
obj-y   += mmc-host/
obj-y   += jpeg/
obj-y   += i2c/
obj-y   += spi/
obj-y   += keypad/
obj-y   += ../../../common/kernel/drivers/video/
obj-y   += video/
obj-y   += dispsys/
obj-y   += rtc/
obj-y   += wdt/
obj-y   += sound/
obj-y   += devinfo/
obj-y   += devapc/
obj-$(CONFIG_MTK_BTIF)   += btif/
obj-y   += mrdump/

#ifneq ($(MTK_PROJECT), mt6582fpga_ca7_ldvt)

#endif
#ifneq ($(CONFIG_EARLY_LINUX_PORTING),y)
#obj-y   += nand/
obj-y   += auxadc/
#obj-y   += touchpanel/
#obj-y   += spi/
#obj-y   += ccci/
obj-y   += ldvt/
#obj-y   += mmc-host/
obj-y   += videocodec/
#obj-y   += rtc/
#
#obj-y   += jpeg/
#obj-y   += gdma/
obj-y   += masp/
#endif
#
obj-y   += accdet/
obj-y   += vibrator/
obj-y   += leds/
obj-y   += wmt/
obj-y   += thermal/

include $(MTK_PATH_BUILD)/common.mk
obj-y   += $(call all-subdirs-with-makefile,cpu_stress)
obj-y   += pmt/
ifeq ($(TRUSTONIC_TEE_SUPPORT),yes)
obj-$(CONFIG_TRUSTONIC_TEE_SUPPORT)   += gud/
endif

obj-y	+= $(call all-subdirs-with-makefile,met)

#ifneq   ($(MTK_GPU_SUPPORT),no)
obj-y   += gpu/
#endif

ifeq ($(MTK_NFC_SUPPORT),yes)
obj-y	+= nfc/
endif
