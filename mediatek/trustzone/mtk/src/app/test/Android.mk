LOCAL_PATH := $(call my-dir)

include mediatek/trustzone/mtk/Setting.mk #this is common setting
include $(LOCAL_PATH)/../../custom/custom.mk #this is custom setting

include $(CLEAR_VARS)
LOCAL_MODULE := libtz_app_test
LOCAL_SRC_FILES := test.c
LOCAL_C_INCLUDES += \
	$(TZ_C_INCLUDES)

LOCAL_CFLAGS += ${TZ_CFLAG}
LOCAL_CFLAGS += \
        -DMEMBASE=$(MEMBASE) \
        -DMEMSIZE=$(MEMSIZE) \
        -DFBSIZE=$(FBSIZE) \
        -DMACH_TYPE=$(MACH_TYPE)

include $(BUILD_RAW_STATIC_LIBRARY)
