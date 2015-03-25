ifneq ($(filter banyan_addon banyan_addon_x86,$(TARGET_PRODUCT)),)
# Dummy module for mediatek-android.jar binary release support
# ============================================================
ifeq ($(strip $(MTK_BSP_PACKAGE)),no)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
   
LOCAL_MODULE := mediatek-android
LOCAL_SRC_FILES := dummy.java

LOCAL_MODULE_TAGS := optional
  
include $(BUILD_HOST_JAVA_LIBRARY)
endif
endif
