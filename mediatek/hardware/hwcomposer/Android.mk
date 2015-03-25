LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MTK_HWC_SUPPORT), yes)

LOCAL_SRC_FILES := \
	hwc.cpp

LOCAL_CFLAGS := \
	-DLOG_TAG=\"hwcomposer\"

ifeq ($(MTK_HDMI_SUPPORT), yes)
LOCAL_CFLAGS += -DMTK_EXTERNAL_SUPPORT
endif

ifeq ($(MTK_WFD_SUPPORT), yes)
LOCAL_CFLAGS += -DMTK_VIRTUAL_SUPPORT
endif

ifneq ($(MTK_PQ_SUPPORT), PQ_OFF)
LOCAL_CFLAGS += -DMTK_ENHAHCE_SUPPORT
endif

MTK_HWC_CHIP = $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )

ifeq ($(MTK_HWC_VERSION), 1.2)
LOCAL_CFLAGS += -DMTK_HWC_VER_1_2
endif

ifeq ($(MTK_HWC_VERSION), 1.3)
LOCAL_CFLAGS += -DMTK_HWC_VER_1_3
endif

LOCAL_STATIC_LIBRARIES += \
	hwcomposer.$(MTK_HWC_CHIP).$(MTK_HWC_VERSION) \
	libgralloc_extra

LOCAL_SHARED_LIBRARIES := \
	libEGL \
	libGLESv1_CM \
	libui \
	libutils \
	libcutils \
	libsync \
	libm4u \
	libion \
	libdpframework \
	libhardware \
	#libbwc

# HAL module implemenation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
LOCAL_MODULE := hwcomposer.$(MTK_HWC_CHIP)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
include $(BUILD_SHARED_LIBRARY)

include $(MTK_ROOT)/tablet/symlink.mk

endif # MTK_HWC_SUPPORT
