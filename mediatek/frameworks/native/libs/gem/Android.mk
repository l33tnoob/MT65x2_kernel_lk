# build libgem.so


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


# check flags
LOCAL_CFLAGS := \
	-DLOG_TAG=\"gem\" \
	-DATRACE_TAG=ATRACE_TAG_GRAPHICS \
    -DEGL_EGLEXT_PROTOTYPES \

ifeq ($(strip $(TARGET_BUILD_VARIANT)), user)
    LOCAL_CFLAGS += -DMTK_USER_BUILD
endif


# code build
LOCAL_C_INCLUDES += \
	$(TOP)/frameworks/native/services/surfaceflinger \

LOCAL_SRC_FILES := \
	gem.cpp \
	queue.cpp \
	program.cpp \
	thread.cpp \


# link for output
LOCAL_SHARED_LIBRARIES := \
	libdl \
	libutils \
	libcutils \
	libui \
	libEGL \

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libgem
include $(BUILD_SHARED_LIBRARY)
