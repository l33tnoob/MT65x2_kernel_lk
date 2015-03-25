LOCAL_PATH:= $(call my-dir)

bromLite_dir := $(TARGET_OUT)/bin

#
# Build a statically-linked binary to include in OTA packages
#

include $(CLEAR_VARS)

# Build only in eng, so we don't end up with a copy of this in /system
# on user builds.  (TODO: find a better way to build device binaries
# needed only for OTA packages.)
ifeq ($(strip $(PURE_AP_USE_EXTERNAL_MODEM)),yes)
LOCAL_MODULE_TAGS := eng
endif

LOCAL_SRC_FILES := \
	bootrom_stage.c		\
	da_stage.c		\
	download_images.c	\
	GCC_Utility.c		\
	interface.c		\
	main.c	

LOCAL_CFLAGS := -Wall -W -Wmissing-field-initializers -D_CONSOLE

LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_MODULE_PATH := $(bromLite_dir)

LOCAL_STATIC_LIBRARIES := libcutils libc

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE := brom_lite

include $(BUILD_EXECUTABLE)

