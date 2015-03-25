ifeq ($(MTK_TINY_UTIL), yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := tiny_mkswap.c

LOCAL_MODULE := tiny_mkswap

LOCAL_SHARED_LIBRARIES := libc libcutils

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := tiny_swapon.c

LOCAL_MODULE := tiny_swapon

LOCAL_SHARED_LIBRARIES := libc libcutils

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := tiny_swapoff.c

LOCAL_MODULE := tiny_swapoff

LOCAL_SHARED_LIBRARIES := libc

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := tiny_switch.c

LOCAL_MODULE := tiny_switch

LOCAL_SHARED_LIBRARIES := libc libcutils

include $(BUILD_EXECUTABLE)
endif
