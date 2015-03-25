LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= Meta_CCAP_Para.cpp

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/include \
    $(MTK_PATH_SOURCE)/external/meta/common/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/acdk \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/acdk/inc/cct \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/acdk/inc/acdk \
    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
    $(MTK_PATH_SOURCE)/external/mhal/inc \
    $(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM)/hal/inc

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libacdk_entry_cctif
LOCAL_WHOLE_STATIC_LIBRARIES += libacdk_entry_mdk

#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES := libcutils libc libstdc++

LOCAL_MODULE := libccap

#
# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include

# End of common part ---------------------------------------
#

include $(BUILD_STATIC_LIBRARY)
