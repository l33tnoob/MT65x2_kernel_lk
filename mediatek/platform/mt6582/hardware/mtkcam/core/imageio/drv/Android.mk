#
# libimageio_plat_drv
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    $(call all-c-cpp-files-under, cam) \
    $(call all-c-cpp-files-under, cdp) \
    $(call all-c-cpp-files-under, mdp)
    
# camutils: For CameraProfile APIs.
#
LOCAL_C_INCLUDES := $(TOP)/bionic \
    $(TOP)/external/stlport/stlport \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/imageio/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/imageio/drv/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/m4u \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/common/camutils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/drv
#    $(TOP)/$(MTK_PATH_SOURCE)/protect-bsp/platform/mt6582/hardware/dpframework
    
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#ifeq ($(BUILD_MTK_LDVT),true)
#    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
#endif

LOCAL_SHARED_LIBRARIES := \
    libdpframework \
    libstlport \
    libcutils \
    libcamdrv \
    libm4u \
    
#new from Jonas requests 130510
LOCAL_SHARED_LIBRARIES += liblog

#
LOCAL_MODULE := libimageio_plat_drv
LOCAL_MODULE_TAGS := optional

#
#include $(BUILD_STATIC_LIBRARY)

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
include $(BUILD_SHARED_LIBRARY)


#
#include $(call all-makefiles-under, $(LOCAL_PATH))
