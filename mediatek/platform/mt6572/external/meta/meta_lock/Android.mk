LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:= libc  libft
LOCAL_SRC_FILES:=Meta_Lock.c
LOCAL_C_INCLUDES:= \
        $(MTK_PATH_SOURCE)/external/meta/common/inc
LOCAL_MODULE:=libmeta_lock
LOCAL_PRELINK_MODULE:=false
include $(BUILD_STATIC_LIBRARY)


