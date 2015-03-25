# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
#

ifneq ($(strip $(MTK_PLATFORM)),)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= atci_service
LOCAL_MODULE_TAGS:=eng

LOCAL_SRC_FILES:= \
    src/atci_service.c \
    src/atci_generic_cmd_dispatch.c \
    src/atci_audio_cmd.cpp \
    src/atci_telephony_cmd.c \
    src/atci_system_cmd.c \
    src/atci_pq_cmd.c \
    src/atci_mjc_cmd.c \
    ../atci/src/atcid_util.c \
    ../atci/src/at_tok.c

 
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libmedia \
    libbinder \
    liblog

LOCAL_C_INCLUDES += \
        $(KERNEL_HEADERS) \
        $(TOP)/frameworks/base/include \
        
LOCAL_C_INCLUDES += ${LOCAL_PATH}/../atci/src

ifeq ($(MTK_CLEARMOTION_SUPPORT),yes)
LOCAL_C_INCLUDES += $(MTK_PATH_PLATFORM)/kernel/drivers/mjc
endif

ifeq ($(MTK_GPS_SUPPORT),yes)

LOCAL_SRC_FILES += \
    src/atci_gps_cmd.c

LOCAL_CFLAGS += \
    -DENABLE_GPS_AT_CMD

endif

ifeq ($(MTK_NFC_SUPPORT),yes)

LOCAL_SRC_FILES += \
    src/atci_nfc_cmd.c

LOCAL_CFLAGS += \
    -DENABLE_NFC_AT_CMD

LOCAL_C_INCLUDES += \
    $(TOP)/mediatek/external/mtknfc/inc \
    $(TOP)/packages/apps/Nfc/mtk-nfc/jni-dload \

LOCAL_SHARED_LIBRARIES += \
    libmtknfc_dynamic_load_jni
    
endif


#Haman changed for Arima version
#ifeq ($(MTK_WLAN_SUPPORT),yes)
#  ifeq ($(MTK_WLAN_CHIP),MT6620)
#
#LOCAL_SRC_FILES += \
#    src/atci_wlan_cmd.c
#
#LOCAL_SHARED_LIBRARIES += \
#	liblgerft
#
#LOCAL_C_INCLUDES += \
#    $(MTK_PATH_SOURCE)/external/liblgerft
#
#LOCAL_CFLAGS += \
#    -DENABLE_WLAN_AT_CMD
#
#  endif
#endif
#change end

# Add Flags and source code for MMC AT Command
LOCAL_SRC_FILES += \
    src/atci_mmc_cmd.c
LOCAL_CFLAGS += \
    -DENABLE_MMC_AT_CMD

# Add Flags and source code for CODECRC AT Command
LOCAL_SRC_FILES += \
    src/atci_code_cmd.c
LOCAL_CFLAGS += \
    -DENABLE_CODECRC_AT_CMD

#Add Flags and source code for  backlight and  vibrator AT Command
LOCAL_SRC_FILES += \
    src/atci_lcdbacklight_vibrator_cmd.c 
LOCAL_CFLAGS += \
    -DENABLE_BLK_VIBR_AT_CMD

#Add Flags and source code for kpd AT Command
LOCAL_SRC_FILES += \
    src/atci_kpd_cmd.c
LOCAL_CFLAGS += \
    -DENABLE_KPD_AT_CMD

include $(BUILD_EXECUTABLE)
endif




