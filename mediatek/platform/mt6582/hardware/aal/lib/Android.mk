LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

$(shell rm -f $(LOCAL_PATH)/custom)

LOCAL_SRC_FILES := \
    cust_aal_main.cpp 

LOCAL_SHARED_LIBRARIES := \
    libcutils \

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(call lc,$(MTK_PLATFORM))/hardware/aal/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(call lc,$(MTK_PLATFORM))/kernel/drivers/dispsys \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc

LOCAL_MODULE:= libaal_cust

LOCAL_MODULE_CLASS := SHARED_LIBRARIES
INTERMEDIATES := $(call local-intermediates-dir)
LOCAL_GENERATED_SOURCES += \
     $(INTERMEDIATES)/cust_aal.cpp

$(INTERMEDIATES)/cust_aal.cpp:$(MTK_ROOT_CUSTOM_OUT)/hal/aal/cust_aal.cpp
	@mkdir -p $(dir $@)
	@cp -f $< $@

include $(BUILD_SHARED_LIBRARY)
