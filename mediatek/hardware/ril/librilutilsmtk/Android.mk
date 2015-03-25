# Copyright 2013 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    librilutils.c

LOCAL_CFLAGS :=

LOCAL_MODULE:= librilutilsmtk

LOCAL_LDLIBS += -lpthread

include $(BUILD_SHARED_LIBRARY)


# Create static library for those that want it
# =========================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    librilutils.c

LOCAL_STATIC_LIBRARIES :=

LOCAL_CFLAGS :=

LOCAL_MODULE:= librilutilsmtk_static

LOCAL_LDLIBS += -lpthread

include $(BUILD_STATIC_LIBRARY)
