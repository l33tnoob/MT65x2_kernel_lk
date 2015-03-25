LOCAL_PATH := $(call my-dir)

ifeq ($(wildcard mediatek/protect-private/drm/playready/Android.mk),)

include $(CLEAR_VARS)
LOCAL_MODULE := libtz_app_playready
LOCAL_SRC_FILES := $(LOCAL_MODULE).a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
include $(BUILD_PREBUILT)

endif #ifeq ($(wildcard mediatek/protect-private/drm/playready/Android.mk),)