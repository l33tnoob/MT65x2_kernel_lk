LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libthermalstress_jni
LOCAL_SRC_FILES := thermalstress_jni.c

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)
LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES +=  libcutils

include $(BUILD_SHARED_LIBRARY)
