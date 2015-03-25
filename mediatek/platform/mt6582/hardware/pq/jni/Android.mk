ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)

ifeq ($(MTK_PLATFORM), $(filter $(MTK_PLATFORM) , MT6582))

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	mhal_jni.cpp
	 
LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += \
        $(KERNEL_HEADERS) \
        $(TOP)/frameworks/base/include \
        $(TOP)/mediatek/platform/mt6582/kernel/drivers/dispsys \
        $(MTK_PATH_PLATFORM)/../../hardware/dpframework/inc
	
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libPQjni

LOCAL_MODULE_TAGS := eng

include $(BUILD_SHARED_LIBRARY)

endif

endif
