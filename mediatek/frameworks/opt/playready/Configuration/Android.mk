################################################################################
# prebuild DxDrmConfig.txt
################################################################################

ifeq ($(strip $(MTK_PLAYREADY_SUPPORT)),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := DxDrmConfig.txt
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/DxDrm
#LOCAL_STRIP_MODULE := true

include $(BUILD_PREBUILT)

endif