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
    libgui \
    libskia

LOCAL_MODULE:= test-surface3

LOCAL_MODULE_TAGS := tests

LOCAL_C_INCLUDES += \
    frameworks/base/services/surfaceflinger \
    external/skia/include/core \
    external/skia/include/images

include $(BUILD_EXECUTABLE)
