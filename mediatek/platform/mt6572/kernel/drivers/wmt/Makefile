# WMT HAL driver for MT6582
# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
  ccflags-y += -I$(src)/include -I$(MTK_ROOT)/kernel/drivers/conn_soc/common/linux/include -I$(MTK_ROOT)/kernel/drivers/conn_soc/common/include
  obj-y += mtk_wcn_consys_hw.o
#  obj-y += mtk_wcn_consys_stub_alps.o
  obj-y += wmt_plat_alps.o
# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD  := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
