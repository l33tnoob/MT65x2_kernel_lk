LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_vtservice.cpp 

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libbinder \
        liblog

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/base/include\
    $(TOP)/system/core/include/private

ifeq ($(MTK_VT3G324M_SUPPORT), yes)
	LOCAL_SHARED_LIBRARIES += libmtk_vt_service
	LOCAL_C_INCLUDES += $(TOP)/mediatek/external/VT/service/inc	
endif
LOCAL_MODULE:= vtservice

include $(BUILD_EXECUTABLE)
