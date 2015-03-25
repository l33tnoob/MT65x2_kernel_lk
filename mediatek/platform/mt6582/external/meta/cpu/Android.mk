LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_cpu.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc
LOCAL_SHARED_LIBRARIES := libft
LOCAL_MODULE := libmeta_cpu
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)

############
# Self Test
############
BUILD_SELF_TEST := false

ifeq ($(BUILD_SELF_TEST), true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_cpu_test.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc $(MTK_PATH_CUSTOM)/hal/inc
LOCAL_STATIC_LIBRARIES := libmeta_cpu
LOCAL_SHARED_LIBRARIES := libft
LOCAL_MODULE := meta_cpu_test
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
include $(BUILD_EXECUTABLE)
endif
