ifeq ($(MTK_GPS_SUPPORT), yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_gps.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc \
                   $(MTK_PATH_SOURCE)/external/nvram/libnvram \
                   $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
                   $(MTK_PATH_CUSTOM)/cgen/cfgdefault \
                   $(MTK_PATH_CUSTOM)/cgen/inc  
LOCAL_MODULE := libmeta_gps
LOCAL_SHARED_LIBRARIES := libcutils libnetutils libc libft libnvram
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)

###############################################################################
# SELF TEST
###############################################################################
BUILD_SELF_TEST := false
ifeq ($(BUILD_SELF_TEST), true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_gps_test.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc
LOCAL_MODULE := meta_gps_test
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_SHARED_LIBRARIES := libmeta_wifi libc libft
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
include $(BUILD_EXECUTABLE)
endif
endif