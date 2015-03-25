LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= $(call all-subdir-c-files)

LOCAL_CFLAGS := -ffunction-sections -DNO_CONSOLE_IO
	
LOCAL_MODULE:= libleptonica
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
