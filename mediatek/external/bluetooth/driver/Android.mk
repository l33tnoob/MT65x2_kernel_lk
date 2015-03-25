ifeq ($(MTK_BT_SUPPORT), yes)
LOCAL_PATH := $(call my-dir)

ifeq ($(MTK_COMBO_SUPPORT), yes)
include $(LOCAL_PATH)/combo/Android.mk
else
include $(LOCAL_PATH)/standalone/Android.mk
endif

endif