LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Module name should match library/file name to be installed.
LOCAL_MODULE := disableapplist.txt
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE)
# set class according to lib/file attribute
LOCAL_MODULE_CLASS := ETC
# $(TARGET_OUT) points to system/ folder
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc
include $(BUILD_PREBUILT)
