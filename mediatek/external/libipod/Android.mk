#
# libipod
#
ifneq (,$(filter yes, $(MTK_IPO_SUPPORT) $(MTK_KERNEL_POWER_OFF_CHARGING)))

LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libipod.so

include $(BUILD_MULTI_PREBUILT)

ifeq ($(MTK_IPOH_SUPPORT), yes)

############################################
#### user space program for hibernation ####
############################################
include $(CLEAR_VARS)
LOCAL_PREBUILT_EXECUTABLES := ipohctl
#LOCAL_MODULE_TAGS := user

include $(BUILD_MULTI_PREBUILT)

endif ## ipohctl program

endif
