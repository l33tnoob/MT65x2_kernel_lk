LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_CFLAGS := -fstack-protector \

LOCAL_SRC_FILES := fbconfig.c
LOCAL_STATIC_LIBRARIES := 
LOCAL_SHARED_LIBRARIES := libc 
LOCAL_WHOLE_STATIC_LIBRARIES := libc_common
LOCAL_C_INCLUDES := 
LOCAL_MODULE := fbconfig
LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)

