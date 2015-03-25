LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_MODULE:= libsurfaceTest
LOCAL_SRC_FILES:= surfaceTestLib.cpp
LOCAL_C_INCLUDES += system/extras/tests/include \
    bionic \
    bionic/libstdc++/include \
    external/stlport/stlport \
    external/skia/include/core \
		external/skia/include/effects \
		external/skia/include/images \
		external/skia/src/ports \
		external/skia/include/utils \
	$(call include-path-for, opengl-tests-includes)

LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_SHARED_LIBRARIES += libcutils libutils libstlport	\
    libskia
LOCAL_STATIC_LIBRARIES += libglTest


include $(BUILD_STATIC_LIBRARY)