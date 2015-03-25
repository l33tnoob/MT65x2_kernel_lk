LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	surface.cpp \
    SurfaceUtils.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
    libui \
    libgui

LOCAL_MODULE:= test-surface2

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
