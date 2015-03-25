# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= sn
LOCAL_MODULE_TAGS:= optional 

LOCAL_SRC_FILES:= \
    sn.c 
 
LOCAL_SHARED_LIBRARIES := libcutils libc

include $(BUILD_EXECUTABLE)
