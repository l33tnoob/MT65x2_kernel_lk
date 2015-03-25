
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(LCM_HEIGHT), 960)
    LOCAL_PATH := $(LOCAL_PATH)/QHD
endif

# Module name should match apk name to be installed
LOCAL_MODULE := WonderZoo_ULC_WW
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/operator/app
include $(BUILD_PREBUILT)