LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= superumount.c
LOCAL_MODULE:= superumount

LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libcutils 

include $(BUILD_EXECUTABLE)
