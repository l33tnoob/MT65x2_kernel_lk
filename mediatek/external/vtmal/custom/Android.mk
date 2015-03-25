ifeq ($(MTK_VT3G324M_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

$(call config-custom-folder,common:hal/vt)
LOCAL_SRC_FILES:= \
		common/custom_vt_video_enc_setting.cpp
#	$(MTK_PATH_CUSTOM)/hal/vt/custom_vt_video_enc_setting.cpp

LOCAL_C_INCLUDES := \
    $(MTK_PATH_CUSTOM)/hal/vt
    		   


LOCAL_PRELINK_MODULE:= false

LOCAL_MODULE:= libvt_custom

include $(BUILD_SHARED_LIBRARY)

endif