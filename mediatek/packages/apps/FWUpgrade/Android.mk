LOCAL_PATH := $(call my-dir)
BASE_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := FWUpgradeInit.rc
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)
include $(BUILD_PREBUILT)

include $(BASE_PATH)/FWUpgrade/Android.mk
include $(BASE_PATH)/FWUpgradeProvider/Android.mk

