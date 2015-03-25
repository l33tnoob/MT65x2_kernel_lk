#
# libcamdrv_imgsensor
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := $(call all-c-cpp-files-under, .)
ifeq ($(BUILD_MTK_LDVT),true)
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, \
    ./../../../../../external/ldvt/ts/camera/imgsensor )
#    ./../../../../../../../../custom/out/mt6589fpga_ca7_ldvt/hal/imgsensor)
#    ./../../../../../../../../source/external/mhal/src/custom/common/hal/imgsensor)    
endif

#
# Note: "$(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \" is for "imgsensor_drv_ldvt.h". If "imgsensor_drv_ldvt.h" is removed. this line can be deleted.
LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/ \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/imageio \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/drv/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/drv/imgsensor \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/sensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/debug_exif/cam \
    $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/common/camutils \
    $(TOP)/bionic/libc/kernel/arch-arm/asm/arch \
    $(TOP)/mediatek/hardware/bwc/inc \
    $(TOP)/mediatek/hardware/mtkcam/inc/drv

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/common/camexif


#
LOCAL_SHARED_LIBRARIES := \
    libcutils \


# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if this line needed in other module, it
# have to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

ifneq ($(CUSTOM_KERNEL_MAIN2_IMGSENSOR),)  
    LOCAL_CFLAGS += -DMTK_MAIN2_IMGSENSOR
endif

ifneq ($(CUSTOM_KERNEL_SUB_IMGSENSOR),)  
    LOCAL_CFLAGS += -DMTK_SUB_IMGSENSOR
endif

ifeq ($(HAVE_MATV_FEATURE),yes)
    LOCAL_CFLAGS += -DATV_SUPPORT
endif

#
#LOCAL_STATIC_LIBRARIES := \

#ifeq ($(BUILD_MTK_LDVT),true)
#    LOCAL_WHOLE_STATIC_LIBRARIES := libuvvf
#endif

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/external/matv_cust  \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/include/media \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/include \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/media/libs \
    $(TOP)/frameworks/base/include/media \
    $(TOP)/frameworks/base/include/utils \
    $(TOP)/frameworks/base/include

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaed
LOCAL_C_INCLUDES += $(MTK_ROOT)/external/aee/binary/inc
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifneq ($(BUILD_MTK_LDVT),true)
LOCAL_CFLAGS+= -DATVCHIP_MTK_ENABLE
endif

#
#LOCAL_WHOLE_STATIC_LIBRARIES += 

#
LOCAL_MODULE := libcamdrv_imgsensor

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
include $(BUILD_STATIC_LIBRARY)

 

#
#include $(call all-makefiles-under, $(LOCAL_PATH))
