ifeq ($(MTK_DX_HDCP_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := meta_hdcp.c

LOCAL_C_INCLUDES += \
		$(MTK_PATH_SOURCE)/external/meta/common/inc \
		$(TOP)/frameworks/av/media/libstagefright/wifi-display/dxhdcp

LOCAL_MODULE := libmeta_hdcp

LOCAL_SHARED_LIBRARIES := libDxHdcp libcutils


include $(BUILD_STATIC_LIBRARY)


endif

