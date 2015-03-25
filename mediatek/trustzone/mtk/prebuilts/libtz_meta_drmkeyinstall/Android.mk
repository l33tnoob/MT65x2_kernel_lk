ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)

LOCAL_PATH := $(call my-dir)

ifeq ($(wildcard mediatek/protect-private/sec_drm/mtee/app/drmkey/Android.mk),)

include $(CLEAR_VARS)
LOCAL_MODULE := libtz_meta_drmkeyinstall
LOCAL_SRC_FILES := $(LOCAL_MODULE).a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
include $(BUILD_PREBUILT)

endif #ifeq ($(wildcard mediatek/protect-private/sec_drm/mtee/platform/mt8135/drmkey/Android.mk),)

endif
endif
