LOCAL_PATH:= $(call my-dir)

ifeq ($(HAVE_AEE_FEATURE),yes)
include $(CLEAR_VARS)

   ifneq ($(MTK_CHIPTEST_INT),yes)
      ifeq ($(PARTIAL_BUILD),true)
         LOCAL_MODULE := init.aee.customer.rc
      else
         LOCAL_MODULE := init.aee.mtk.rc
      endif  # PARTIAL_BUILD
   else
   LOCAL_MODULE := init.aee.customer.rc
   endif  # MTK_CHIPTEST_INT

LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)

include $(BUILD_PREBUILT)
endif   # HAVE_AEE_FEATURE
