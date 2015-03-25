LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	blank.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libgui

LOCAL_MODULE:= test-blank

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
