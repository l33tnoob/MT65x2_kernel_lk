LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= hello_world.c
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_MODULE := xlog_helloworld
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
