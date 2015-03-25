LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	surface.cpp \
	transaction.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libui \
	libgui

LOCAL_MODULE:= test-perf

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
