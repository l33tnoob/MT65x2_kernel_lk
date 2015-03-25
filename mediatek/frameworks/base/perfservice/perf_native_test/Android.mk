# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	perf_native_test.cpp

#LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_CFLAGS += -static
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks/base/perfservice/perfservicenative
LOCAL_SHARED_LIBRARIES += libc libdl
LOCAL_MODULE := perf_native_test

LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)

#$(call dist-for-goals,dist_files,$(LOCAL_BUILT_MODULE))
