
ifeq ($(strip $(MTK_AIV_SUPPORT)),yes)
ifeq ($(MTK_DRM_PLAYREADY_SUPPORT), yes)
ifeq ($(wildcard mediatek/protect-private/drm/playreadyplugin/decryptor/Android.mk),)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libdrmplayreadydecryptor
LOCAL_SRC_FILES := libdrmplayreadydecryptor.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .so
LOCAL_REQUIRED_MODULES := libdrmplayreadyplugin libplayready

include $(BUILD_PREBUILT)
$(warning   $(LOCAL_PATH): $(LOCAL_SRC_FILES) )

endif #ifeq ($(wildcard mediatek/protect-private/drm/playreadyplugin/decryptor/Android.mk),)
endif
endif