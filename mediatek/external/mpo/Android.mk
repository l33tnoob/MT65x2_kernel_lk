#
# libmpo
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    MpoStream.cpp \
    IFDWrapper.cpp \
    MpoCommon.cpp

LOCAL_C_INCLUDES += \
    $(MTK_ROOT)/external/mpo \
    $(MTK_ROOT)/external/mpo/inc \
    $(MTK_ROOT)/external/mpo/mpoencoder \
    $(MTK_ROOT)/external/mpo/mpodecoder \
    $(MTK_ROOT)/external/mpo/mpodecoder/inc \

LOCAL_SHARED_LIBRARIES:= \
	libcutils 

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libmpo

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
