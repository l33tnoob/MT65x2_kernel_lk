# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2013. All rights reserved.
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


ifeq ($(TARGET_ARCH),arm)

include $(call all-subdir-makefiles)

ifeq ($(strip $(MTK_WVDRM_SUPPORT)),yes)
#  $(warning "Widevine DRM is enabled")

ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes)
  BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 1
else
  BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 3
endif 

#$(warning "BOARD_WIDEVINE_OEMCRYPTO_LEVEL = $(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)")

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#########################################################################
# libwvm.so

include $(CLEAR_VARS)
LOCAL_MODULE := libwvm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH:=$(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
LOCAL_STRIP_MODULE := true
LOCAL_WHOLE_STATIC_LIBRARIES := $(LOCAL_MODULE)_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)
LOCAL_STATIC_LIBRARIES+=liboemcrypto_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)
LOCAL_WHOLE_STATIC_LIBRARIES := \
    libwvmcommon_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)

LOCAL_SHARED_LIBRARIES := \
    libstlport \
    libstagefright \
    libWVStreamControlAPI_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL) \
    libdrmframework \
    libcutils \
    liblog \
    libutils \
    libz \
    libdrmmtkutil
    
ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes)
  LOCAL_SHARED_LIBRARIES += \
      lib_uree_mtk_crypto 
endif   
    
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := widevine
include $(BUILD_SHARED_LIBRARY)

#########################################################################
# libdrmwvmplugin.so

include $(CLEAR_VARS)
LOCAL_MODULE := libdrmwvmplugin
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH:=$(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/drm
LOCAL_PRELINK_MODULE:=false
LOCAL_STRIP_MODULE := true
LOCAL_WHOLE_STATIC_LIBRARIES := $(LOCAL_MODULE)_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)
LOCAL_STATIC_LIBRARIES+=liboemcrypto_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)

LOCAL_STATIC_LIBRARIES += \
    libdrmframeworkcommon \
    libdrmwvmcommon \
    libwvocs_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)

LOCAL_SHARED_LIBRARIES += \
    libbinder \
    libutils \
    libcutils \
    libstlport \
    libz \
    libwvdrm_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL) \
    libWVStreamControlAPI_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL) \
    libdl \
    libdrmmtkutil
    
ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes) 
  LOCAL_SHARED_LIBRARIES += \
      lib_uree_mtk_crypto 
endif   
 
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := widevine
include $(BUILD_SHARED_LIBRARY)

#########################################################################
# libdrmdecrypt.so

include $(CLEAR_VARS)
LOCAL_MODULE := libdrmdecrypt
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH:=$(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_PRELINK_MODULE:=false
LOCAL_STRIP_MODULE := true
LOCAL_WHOLE_STATIC_LIBRARIES := $(LOCAL_MODULE)_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)

LOCAL_SHARED_LIBRARIES += \
    libbinder \
    libutils \
    libcutils \
    libstlport \
    libz \
    libdl \
    libstagefright_foundation \
    libcrypto \
    libcutils

ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes)  
  LOCAL_SHARED_LIBRARIES += \
      lib_uree_mtk_crypto 
endif  
    
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libwvdecryptcommon_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)

LOCAL_STATIC_LIBRARIES := \
        liboemcrypto_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)  

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := widevine
include $(BUILD_SHARED_LIBRARY)

else
#  $(warning "Widevine DRM is disabled")
endif

endif
