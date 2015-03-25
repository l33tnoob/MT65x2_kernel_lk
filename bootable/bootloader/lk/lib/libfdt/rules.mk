LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/include

OBJS += \
	$(LOCAL_DIR)/fdt.o \
	$(LOCAL_DIR)/fdt_ro.o \
	$(LOCAL_DIR)/fdt_rw.o \
	$(LOCAL_DIR)/fdt_strerror.o \
	$(LOCAL_DIR)/fdt_sw.o \
	$(LOCAL_DIR)/fdt_wip.o \
