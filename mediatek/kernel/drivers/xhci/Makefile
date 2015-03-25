#
# Makefile for USB Host Lib
#

ccflags-y := -I$(src)/../../../../kernel/drivers/usb/host

ifeq ($(CONFIG_USB_DEBUG),y)
	EXTRA_CFLAGS		+= -DDEBUG
endif

obj-y	+= xhci-mtk-power.o xhci-mtk-scheduler.o xhci-mtk.o
