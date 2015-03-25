LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	flagsEx.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
    libui \
    libgui

LOCAL_MODULE:= test-flagsEx

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
