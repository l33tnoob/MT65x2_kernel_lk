ifneq ($(TARGET_PRODUCT),generic)
ifneq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_ARCH),arm)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif   # TARGET_ARCH == arm
endif	# !TARGET_SIMULATOR
endif # TARGET_PRODUCT!=generic