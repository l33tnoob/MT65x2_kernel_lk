################################################################################
# prebuild libDxPrRecommended.so
################################################################################
#ifeq ($(strip $(MTK_PLAYREADY_SUPPORT)),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libDxPrRecommended
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := $(LOCAL_MODULE).so
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/drm
#LOCAL_STRIP_MODULE := true

include $(BUILD_PREBUILT)

################################################################################
# prebuild libDxDrmServer.so
################################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := libDxDrmServer
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := $(LOCAL_MODULE).so
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
#LOCAL_STRIP_MODULE := true

include $(BUILD_PREBUILT)

#endif
