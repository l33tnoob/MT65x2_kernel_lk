ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)

LOCAL_PATH := $(call my-dir)
include $(call all-makefiles-under,$(LOCAL_PATH))

endif

