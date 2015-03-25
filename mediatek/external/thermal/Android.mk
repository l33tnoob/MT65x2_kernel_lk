# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= thermal
LOCAL_MODULE_TAGS:= optional 

LOCAL_SRC_FILES:= \
    thermal.c 
 
LOCAL_SHARED_LIBRARIES := libcutils libc libnetutils

include $(BUILD_EXECUTABLE)
