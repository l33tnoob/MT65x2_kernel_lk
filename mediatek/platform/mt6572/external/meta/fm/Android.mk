ifeq ($(MTK_FM_SUPPORT), yes)
##### FM META library #####
BUILD_FM_META_LIB := true

ifeq ($(BUILD_FM_META_LIB), true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_fm.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc
LOCAL_MODULE := libmeta_fm
LOCAL_SHARED_LIBRARIES := libcutils libnetutils libc libft
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)
endif
endif
