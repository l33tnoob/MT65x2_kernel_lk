LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:= libc libnvram libft
LOCAL_SRC_FILES:=Meta_APEditor_Para.c
LOCAL_C_INCLUDES:= \
	$(MTK_PATH_SOURCE)/external/meta/common/inc\
    $(MTK_PATH_SOURCE)/external/nvram/libnvram
LOCAL_MODULE:=libmeta_apeditor
LOCAL_PRELINK_MODULE:=false
include $(BUILD_STATIC_LIBRARY)


