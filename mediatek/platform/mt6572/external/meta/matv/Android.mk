LOCAL_PATH := $(call my-dir)

## ==> build this lib only when HAVE_MATV_FEATURE is yes
ifeq ($(HAVE_MATV_FEATURE),yes)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := meta_matv.c
LOCAL_C_INCLUDES += \
	$(MTK_PATH_SOURCE)/external/meta/common/inc \
	$(MTK_PATH_SOURCE)/external/matvctrl
LOCAL_MODULE := libmeta_matv
#LOCAL_STATIC_LIBRARIES := libmatvctrl
ifeq ($(MTK_ATV_CHIP),MTK_MT5192)
LOCAL_STATIC_LIBRARIES += libmatvctrl
else
LOCAL_STATIC_LIBRARIES += libmatvctrl_93
endif
LOCAL_SHARED_LIBRARIES := libcutils libc libft libmatv_cust
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)

endif
## <== build this lib only when HAVE_MATV_FEATURE is yes