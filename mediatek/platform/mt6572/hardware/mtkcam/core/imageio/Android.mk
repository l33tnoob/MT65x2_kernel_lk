#
# libimageio
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(HAVE_AEE_FEATURE),yes)
	LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)

#
LOCAL_SHARED_LIBRARIES := \
    libstlport \
    libcutils \
    libcamdrv \
    libdpframework \
#    libm4u \

# libcam.utils: For CameraProfile APIs. Only used when non-LDVT.
ifneq ($(BUILD_MTK_LDVT),true)
    LOCAL_SHARED_LIBRARIES += libcam.utils
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaed
endif

#
LOCAL_STATIC_LIBRARIES := \
#    libimageio_plat_pipe \
#    libimageio_plat_drv \
#    libimageio_plat_pipe_mgr \

LOCAL_WHOLE_STATIC_LIBRARIES := \
    libimageio_plat_pipe \
    
#
LOCAL_MODULE := libimageio

#
LOCAL_PRELINK_MODULE := false

#
LOCAL_MODULE_TAGS := optional

#

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
include $(call all-makefiles-under, $(LOCAL_PATH))
