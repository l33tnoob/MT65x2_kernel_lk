LOCAL_PATH := $(call my-dir)

include mediatek/trustzone/mtk/Setting.mk #this is common setting
include $(LOCAL_PATH)/../custom/custom.mk #this is custom setting

include $(CLEAR_VARS)
LOCAL_MODULE := libtz_sys
LOCAL_SRC_FILES := memcfg.c debug.c tz_utils.c


LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/../custom/mtk/include \
        mediatek/platform/$(call lc,$(MTK_PLATFORM))/trustzone/include \
	$(TZ_C_INCLUDES)

LOCAL_CFLAGS += ${TZ_CFLAG}
LOCAL_CFLAGS += \
        -DMEMSIZE=$(MEMSIZE) \
        -DFBSIZE=$(FBSIZE) \
        -DBOOTSHARE=$(BOOTSHARE) \
        -DMACH_TYPE=$(MACH_TYPE) \
        -DSECURE_FUNC_STACK_NUM=$(SECURE_FUNC_STACK_NUM) \
        -DSECURE_FUNC_STACK_SIZE=$(SECURE_FUNC_STACK_SIZE) \
        -DTZMEM_RELEASECM_SIZE=$(TZMEM_RELEASECM_SIZE) \

include $(BUILD_RAW_STATIC_LIBRARY)
