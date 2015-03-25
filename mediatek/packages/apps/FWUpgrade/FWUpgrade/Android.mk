LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := FWUpgrade
LOCAL_MODULE_TAGS := optional
ifeq ($(strip $(MTK_FW_UPGRADE_APP)), yes)
LOCAL_SRC_FILES := ./withIcon/FWUpgrade.apk
else
LOCAL_SRC_FILES := ./noIcon/FWUpgrade.apk
endif
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := platform
include $(BUILD_PREBUILT)

