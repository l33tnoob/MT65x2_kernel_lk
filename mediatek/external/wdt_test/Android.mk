LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= wdt_test.c
LOCAL_MODULE := wdt_test
LOCAL_SHARED_LIBRARIES:= libc
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
