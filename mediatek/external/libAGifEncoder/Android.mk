LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    GifEncoder.cpp \
    AGifEncoder.cpp \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libskia \

LOCAL_STATIC_LIBRARIES := \
    libgif \
    libleptonica \

LOCAL_C_INCLUDES += \
  $(TOP)/external/skia/include/core \
	external/giflib \
	mediatek/external/leptonica \
	
LOCAL_MODULE:= libAGifEncoder
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
