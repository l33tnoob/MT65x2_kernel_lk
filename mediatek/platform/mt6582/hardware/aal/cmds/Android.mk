
ifeq ($(MTK_AAL_SUPPORT),yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_aal.cpp 

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libbinder \
    libaal \

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/aal/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(call lc, $(MTK_PLATFORM))/kernel/drivers/dispsys \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc

LOCAL_MODULE:= aal

include $(BUILD_EXECUTABLE)
endif
