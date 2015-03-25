# Build the unit tests,
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := ringbuffer_test

LOCAL_MODULE_TAGS := tests

LOCAL_SRC_FILES := \
    ringbuffer_test.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils

LOCAL_C_INCLUDES := \
    bionic \
    bionic/libstdc++/include

include $(BUILD_EXECUTABLE)
