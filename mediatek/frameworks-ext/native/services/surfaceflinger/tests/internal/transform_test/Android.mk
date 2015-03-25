LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	TransformTest.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libui \
    libgui \
    libbinder \
    libskia

LOCAL_MODULE:= test-transform2

LOCAL_MODULE_TAGS := tests

LOCAL_C_INCLUDES += \
    frameworks/base/services/surfaceflinger \
    external/skia/include/core/

include $(BUILD_EXECUTABLE)
