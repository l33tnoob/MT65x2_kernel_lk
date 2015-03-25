ifeq ($(CONFIG_MT6589_FPGA),y)
obj-y	:= mt_gpio_fpga.o
else
obj-y	:= mt_gpio_base.o mt_gpio_affix.o
endif
obj-y	+= mt_gpio_debug.o mt_gpio_ext.o

