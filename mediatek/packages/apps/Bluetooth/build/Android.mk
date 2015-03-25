# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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

#############################################
# Build Java Package
#############################################
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

$(info [BlueAngel][PKG] LOCAL_PATH=$(LOCAL_PATH))

### define build path
MY_BUILD_PATH := ${LOCAL_PATH}
MY_BUILD_PREFIX := ..

### clean first
#$(shell rm -rf $(LOCAL_PATH)/AndroidManifest.xml)
#$(shell rm -rf $(LOCAL_PATH)/res)
#$(shell mkdir -p $(LOCAL_PATH)/res)

### generate AndroidManifest.xml
#"mediatek/config/" + $(FULL_PROJECT) + "/ProjectConfig.mk"
#$(info executing blueangel.py: project[$(FULL_PROJECT)], PYTHONPATH[$(PYTHONPATH)])
#PY_RES := $(shell python $(LOCAL_PATH)/blueangel.py)
INTERMEDIATES := $(call intermediates-dir-for,APPS,MtkBt,,COMMON)
GEN := $(INTERMEDIATES)/AndroidManifest.xml
$(GEN): PRIVATE_CUSTOM_TOOL := python $(LOCAL_PATH)/blueangel.py $(INTERMEDIATES)
$(GEN): $(LOCAL_PATH)/blueangel.py $(LOCAL_PATH)/AndroidManifest.tpl $(filter-out $(GEN),$(shell find $(MY_MODULE_PATH) -name AndroidManifest.xml))
	$(transform-generated-source)

LOCAL_EMMA_COVERAGE_FILTER := @$(LOCAL_PATH)/emma_filter.txt,--$(LOCAL_PATH)/emma_filter_method.txt

LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += telephony-common

src_list := 
### include modules' mk file
ifeq ($(MTK_BT_SUPPORT), yes)
include $(MY_MODULE_PATH)/common/bt40/Android.mk
src_list += common/bt40
endif
include $(MY_MODULE_PATH)/profiles/prxm/Android.mk
include $(MY_MODULE_PATH)/profiles/prxr/Android.mk
include $(MY_MODULE_PATH)/profiles/simap/Android.mk
include $(MY_MODULE_PATH)/profiles/ftp/Android.mk
include $(MY_MODULE_PATH)/profiles/bpp/Android.mk
include $(MY_MODULE_PATH)/profiles/bip/Android.mk
include $(MY_MODULE_PATH)/profiles/dun/Android.mk
src_list += profiles/prxm profiles/prxr profiles/simap profiles/ftp profiles/bpp profiles/bip profiles/dun
### config package and build

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := MtkBt
LOCAL_CERTIFICATE := platform
#LOCAL_PROGUARD_FLAGS := -include $(LOCAL_PATH)/proguard.flags
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_FULL_MANIFEST_FILE := $(INTERMEDIATES)/AndroidManifest.xml

$(call mtk-bluetooth-copy-rule,$(MY_MODULE_PATH),$(src_list),$(INTERMEDIATES))
LOCAL_RESOURCE_DIR := $(INTERMEDIATES)/res $(LOCAL_PATH)/res
include $(BUILD_PACKAGE)

#############################################
# End of file
#############################################
