
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Module name should match lib name
LOCAL_MODULE := libgameocem.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib

include $(BUILD_PREBUILT)

