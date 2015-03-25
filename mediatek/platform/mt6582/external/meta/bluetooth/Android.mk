ifeq ($(MTK_BT_SUPPORT), yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
  $(MTK_PATH_SOURCE)/external/meta/common/inc

LOCAL_SRC_FILES := meta_bt.c

LOCAL_MODULE := libmeta_bluetooth
LOCAL_SHARED_LIBRARIES := libcutils libnetutils libc libft
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)

###############################################################################
# SELF TEST
###############################################################################
BUILD_SELF_TEST := false
ifeq ($(BUILD_SELF_TEST), true)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
  $(MTK_PATH_SOURCE)/external/meta/common/inc

LOCAL_SRC_FILES := meta_bt_test.c

LOCAL_MODULE := meta_bt_test
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_SHARED_LIBRARIES := libc libft
LOCAL_STATIC_LIBRARIES := libmeta_bluetooth
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
include $(BUILD_EXECUTABLE)
endif
endif