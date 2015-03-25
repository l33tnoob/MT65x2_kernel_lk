# Copyright (C) 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_ARCH),arm)
    WITH_BACKUP_RESTORE := true
else
    WITH_BACKUP_RESTORE := false
endif

include $(CLEAR_VARS)

##########################################
# Feature option
##########################################

# SPECIAL_FACTORY_RESET will backup /data/app when do factory reset if SD is existed
ifeq ($(MTK_SPECIAL_FACTORY_RESET),yes)
    SPECIAL_FACTORY_RESET := true
else
    SPECIAL_FACTORY_RESET := false
endif

ifeq ($(MTK_SECURITY_SW_SUPPORT), yes)
    WITH_SBOOT_UPDATE := true
else
    WITH_SBOOT_UPDATE := false
endif

ifdef MTK_FOTA_SUPPORT
    ifeq ($(MTK_FOTA_SUPPORT),yes)
        WITH_FOTA := true
    else
        WITH_FOTA := false
    endif
else
    WITH_FOTA := false
endif

ifeq ($(MTK_CACHE_MERGE_SUPPORT),yes)
    CACHE_MERGE_SUPPORT := true
else
    CACHE_MERGE_SUPPORT := false
endif

##########################################
# Specify source files
##########################################


LOCAL_SRC_FILES := \
    recovery.cpp \
    bootloader.cpp \
    install.cpp \
    roots.cpp \
    ui.cpp \
    screen_ui.cpp \
    verifier.cpp \
    adb_install.cpp

ifeq ($(WITH_FOTA),true)
LOCAL_SRC_FILES += \
    fota/fota.cpp \
    fota/fota_fs.cpp \
    fota/fota_common.cpp \
    fota/fota_dev.cpp
endif

ifeq ($(WITH_SBOOT_UPDATE),true)
LOCAL_SRC_FILES += \
    sec/sec.c
ifeq ($(CUSTOM_SEC_AUTH_SUPPORT),yes)
$(call config-custom-folder,custom:security/sbchk)
LOCAL_SRC_FILES += custom/cust_auth.c
else
LOCAL_SRC_FILES += auth/sec_wrapper.c
endif
endif

##########################################
# Module initialization
##########################################
LOCAL_MODULE := recovery

LOCAL_FORCE_STATIC_EXECUTABLE := true

RECOVERY_API_VERSION := 3
RECOVERY_FSTAB_VERSION := 2
LOCAL_CFLAGS += -DRECOVERY_API_VERSION=$(RECOVERY_API_VERSION)

ifeq ($(WITH_BACKUP_RESTORE),true)
LOCAL_CFLAGS += -DSUPPORT_DATA_BACKUP_RESTORE
endif

ifeq ($(SPECIAL_FACTORY_RESET),true)
LOCAL_CFLAGS += -DSPECIAL_FACTORY_RESET
endif

ifeq ($(WITH_FOTA),true)
LOCAL_CFLAGS += -DSUPPORT_FOTA -DFOTA_SELF_UPGRADE -DFOTA_SELF_UPGRADE_REBOOT -DFOTA_PHONE_UPGRADE  -DFOTA_UI_MESSAGE
LOCAL_CFLAGS += -fno-short-enums
#LOCAL_CFLAGS += -DVERIFY_BOOT_SOURCE -DVERIFY_BOOT_TARGET
#LOCAL_CFLAGS += -DVERIFY_SYSTEM_SOURCE -DVERIFY_SYSTEM_TARGET
#LOCAL_CFLAGS += -DVERIFY_RECOVERY_SOURCE -DVERIFY_RECOVERY_TARGET
#LOCAL_CFLAGS += -DVERIFY_CUSTOM_SOURCE -DVERIFY_CUSTOM_TARGET
endif

ifeq ($(WITH_SBOOT_UPDATE),true)
LOCAL_CFLAGS += -DSUPPORT_SBOOT_UPDATE
endif

ifeq ($(WITH_FOTA),true)
LOCAL_CFLAGS += -DFOTA_FIRST
# LOCAL_CFLAGS += -DMOTA_FIRST
endif

ifeq ($(MTK_SHARED_SDCARD),yes)
LOCAL_CFLAGS += -DMTK_SHARED_SDCARD
endif

ifeq ($(MTK_2SDCARD_SWAP),yes)
LOCAL_CFLAGS += -DMTK_2SDCARD_SWAP
endif

ifeq ($(CACHE_MERGE_SUPPORT),true)
LOCAL_CFLAGS += -DCACHE_MERGE_SUPPORT
endif

ifeq ($(MTK_LCA_ROM_OPTIMIZE),true)
LOCAL_CFLAGS += -DMTK_LCA_ROM_OPTIMIZE
endif

LOCAL_STATIC_LIBRARIES := \
    libext4_utils_static \
    libsparse_static \
    libminzip \
    libz \
    libmtdutils \
    libmincrypt \
    libminadbd \
    libminui \
    libpixelflinger_static \
    libpng \
    libfs_mgr \
    libcutils \
    liblog \
    libselinux \
    libstdc++ \
    libm \
    libc

ifeq ($(TARGET_ARCH),arm)
ifeq ($(strip $(MTK_FW_UPGRADE)), yes)
LOCAL_CFLAGS += -DMTK_SYS_FW_UPGRADE
LOCAL_STATIC_LIBRARIES += libfwupgrade
endif
endif

ifeq ($(TARGET_USERIMAGES_USE_EXT4), true)
    LOCAL_CFLAGS += -DUSE_EXT4
    LOCAL_C_INCLUDES += system/extras/ext4_utils
    LOCAL_STATIC_LIBRARIES += libext4_utils_static libz
endif

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
LOCAL_CFLAGS += -DUBIFS_SUPPORT
LOCAL_STATIC_LIBRARIES += ubi_ota_update
endif

ifeq ($(PURE_AP_USE_EXTERNAL_MODEM),yes)
LOCAL_CFLAGS += -DEXTERNAL_MODEM_UPDATE
endif

LOCAL_C_INCLUDES += external/libselinux/include
LOCAL_STATIC_LIBRARIES += libselinux

# This binary is in the recovery ramdisk, which is otherwise a copy of root.
# It gets copied there in config/Makefile.  LOCAL_MODULE_TAGS suppresses
# a (redundant) copy of the binary in /system/bin for user builds.
# TODO: Build the ramdisk image in a more principled way.
LOCAL_MODULE_TAGS := eng

ifeq ($(WITH_BACKUP_RESTORE),true)
LOCAL_SRC_FILES += $(PRODUCT_OUT)/obj/STATIC_LIBRARIES/libcrypto_static_intermediates/crypto/des/set_key.o
LOCAL_SRC_FILES += $(PRODUCT_OUT)/obj/STATIC_LIBRARIES/libcrypto_static_intermediates/crypto/des/cfb64enc.o
LOCAL_SRC_FILES += $(PRODUCT_OUT)/obj/STATIC_LIBRARIES/libcrypto_static_intermediates/crypto/evp/e_des.o
LOCAL_SRC_FILES += $(PRODUCT_OUT)/obj/STATIC_LIBRARIES/libcrypto_static_intermediates/crypto/des/des_enc.o
LOCAL_STATIC_LIBRARIES += libbackup_restore libcrypto_static
endif

ifeq ($(TARGET_RECOVERY_UI_LIB),)
  LOCAL_SRC_FILES += default_device.cpp
else
  LOCAL_STATIC_LIBRARIES += $(TARGET_RECOVERY_UI_LIB)
endif


ifeq ($(WITH_FOTA),true)
LOCAL_STATIC_LIBRARIES += upi_v7
endif


ifeq ($(WITH_SBOOT_UPDATE),true)
LOCAL_STATIC_LIBRARIES += libsbup
ifneq ($(CUSTOM_SEC_AUTH_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES += libsbauth
endif
endif
LOCAL_C_INCLUDES += system/extras/ext4_utils \
        mediatek/custom/$(TARGET_PRODUCT)/recovery/inc \
        kernel \
        $(LOCAL_PATH)/fota/include \
        $(MTK_ROOT_CUSTOM_OUT)/kernel/dct

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := verifier_test
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_TAGS := tests
LOCAL_SRC_FILES := \
    verifier_test.cpp \
    verifier.cpp \
    ui.cpp
LOCAL_STATIC_LIBRARIES := \
    libmincrypt \
    libminui \
    libcutils \
    libstdc++ \
    libc

LOCAL_C_INCLUDES +=  \
        mediatek/custom/$(TARGET_PRODUCT)/recovery/inc

include $(BUILD_EXECUTABLE)


##########################################
# Static library - UBIFS_SUPPORT
##########################################

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := roots.cpp \
				   
LOCAL_MODULE := ubiutils

LOCAL_C_INCLUDES += system/extras/ext4_utils \
	                kernel
					
LOCAL_STATIC_LIBRARIES += libz ubi_ota_update

LOCAL_CFLAGS += -DUBIFS_SUPPORT
LOCAL_MODULE_TAGS := eng

include $(BUILD_STATIC_LIBRARY)
endif

##########################################
# Static library - WITH_FOTA
##########################################

ifeq ($(WITH_FOTA),true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := fota/upi_v7.a
include $(BUILD_MULTI_PREBUILT)
endif

##########################################
# Static library - WITH_SBOOT_UPDATE
##########################################

ifeq ($(WITH_SBOOT_UPDATE),true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS += sec/libsbup.a
ifneq ($(CUSTOM_SEC_AUTH_SUPPORT),yes)
LOCAL_PREBUILT_LIBS += sec/libsbauth.a
endif
include $(BUILD_MULTI_PREBUILT)
endif

#############################################################################
ifeq ($(WITH_FOTA),true)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= system/extras/ext4_utils kernel $(LOCAL_CUST_INC_PATH) $(LOCAL_PATH)/fota/include $(LOCAL_PATH)/fota
LOCAL_SRC_FILES := fota/fota_main.cpp \
                   fota/fota.cpp \
                   fota/fota_common.cpp \
                   fota/fota_fs.cpp \
                   fota/fota_dev.cpp \
                   bootloader.cpp \
                   roots.cpp
#LOCAL_CFLAGS += -fshort-enums -g
LOCAL_CFLAGS += -fno-short-enums
LOCAL_CFLAGS += -DSUPPORT_FOTA -DFOTA_SELF_UPGRADE
LOCAL_CFLAGS += -DVERIFY_BOOT_SOURCE -DVERIFY_BOOT_TARGET
LOCAL_CFLAGS += -DVERIFY_SYSTEM_SOURCE -DVERIFY_SYSTEM_TARGET
LOCAL_CFLAGS += -DVERIFY_RECOVERY_SOURCE -DVERIFY_RECOVERY_TARGET
LOCAL_CFLAGS += -DVERIFY_CUSTOM_SOURCE -DVERIFY_CUSTOM_TARGET
LOCAL_MODULE := fota1
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES :=  libext4_utils_static libmincrypt libcutils libstdc++ libc libmtdutils upi_v7 libfs_mgr
LOCAL_SHARED_LIBRARIES := libcutils libc
include $(BUILD_EXECUTABLE)
endif

#############################################################################

ifeq ($(TARGET_ARCH),arm)
ifeq ($(strip $(MTK_FW_UPGRADE)), yes)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS += libfwupgrade.a
include $(BUILD_MULTI_PREBUILT)
endif
endif

include $(LOCAL_PATH)/minui/Android.mk \
    $(LOCAL_PATH)/minelf/Android.mk \
    $(LOCAL_PATH)/minzip/Android.mk \
    $(LOCAL_PATH)/minadbd/Android.mk \
    $(LOCAL_PATH)/mtdutils/Android.mk \
    $(LOCAL_PATH)/tools/Android.mk \
    $(LOCAL_PATH)/edify/Android.mk \
    $(LOCAL_PATH)/updater/Android.mk \
    $(LOCAL_PATH)/applypatch/Android.mk
