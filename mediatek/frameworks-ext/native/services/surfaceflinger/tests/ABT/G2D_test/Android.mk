LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	surface.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libskia \
    libui \
    libgui
    
LOCAL_STATIC_LIBRARIES := \
    libsurfaceTest	\
    
LOCAL_MODULE:= g2d_test

LOCAL_MODULE_TAGS := tests

LOCAL_C_INCLUDES += \
	external/skia/include/core \
	external/skia/include/effects \
	external/skia/include/images \
	external/skia/src/ports \
	external/skia/include/utils
	
include $(BUILD_EXECUTABLE)
