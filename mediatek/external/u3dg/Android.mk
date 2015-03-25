#ifeq ($(MTK_MDLOGGER_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := u3dg
LOCAL_SRC_FILES := u3dg.c

LOCAL_SHARED_LIBRARIES := libutils libcutils libusbhost
include $(BUILD_EXECUTABLE)

#endif