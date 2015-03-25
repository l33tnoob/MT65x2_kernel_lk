LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_MODULE:= libhwcTest
LOCAL_SRC_FILES:= hwcTestLib.cpp
LOCAL_C_INCLUDES += system/extras/tests/include \
    bionic \
    bionic/libstdc++/include \
    external/stlport/stlport \
	$(call include-path-for, opengl-tests-includes)

LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_SHARED_LIBRARIES += libcutils libutils libstlport
LOCAL_STATIC_LIBRARIES += libglTest


include $(BUILD_STATIC_LIBRARY)



include $(CLEAR_VARS)
LOCAL_SRC_FILES:= surface.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libEGL \
    libGLESv2 \
    libui \
    libhardware \
    libskia \

LOCAL_STATIC_LIBRARIES := \
    libtestUtil \
    libglTest \
    libhwcTest \
    libsurfaceTest	\

LOCAL_C_INCLUDES += \
    system/extras/tests/include \
    hardware/libhardware/include \
    external/skia/include/core \
		external/skia/include/effects \
		external/skia/include/images \
		external/skia/src/ports \
		external/skia/include/utils \
	$(call include-path-for, opengl-tests-includes)

LOCAL_MODULE:= hwcomposer_test

LOCAL_MODULE_TAGS := tests

LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

include $(BUILD_NATIVE_TEST)


