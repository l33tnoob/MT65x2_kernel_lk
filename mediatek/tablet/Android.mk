ifneq ($(MTK_TABLET_HARDWARE),)
ifeq ($(wildcard $(MTK_PATH_PLATFORM)/hardware/gpu),)
ifeq ($(wildcard $(MTK_ROOT)/protect),)

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_PATH := $(call my-dir)
MTK_HWC_CHIP := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
LOCAL_MODULE := gralloc.$(MTK_HWC_CHIP)

include $(MTK_ROOT)/tablet/symlink.mk

endif
endif
endif
