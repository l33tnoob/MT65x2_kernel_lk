#
# libmpoencoder
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= MpoEncoder.cpp

LOCAL_C_INCLUDES += $(MTK_ROOT)/external/mpo \
	$(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += \
        $(MTK_ROOT)/external/mpo \
        $(MTK_ROOT)/external/mpo/mpoencoder \
        $(MTK_ROOT)/external/mpo/inc \

LOCAL_SHARED_LIBRARIES:= \
	libcutils \
        libmpo

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libmpoencoder

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

