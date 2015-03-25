# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2011. All rights reserved.
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

#
# Copyright (C) 2008 The Android Open Source Project
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
#

# This makefile shows how to build your own shared library that can be
# shipped on the system of a phone, and included additional examples of
# including JNI code with the library and writing client applications against it.

LOCAL_PATH := $(call my-dir)

# MediaTek resource dependency definition.
#mediatek-res-source-path := APPS/mediatek-res_intermediates/src
#mediatek_res_R_stamp := \
#	$(call intermediates-dir-for,APPS,mediatek-res,,COMMON)/src/R.stamp

# MediaTek framework library.
# ============================================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := mediatek-framework

ifeq ($(strip $(MTK_SDK_EMMA_COVERAGE)), yes)
EMMA_INSTRUMENT := true
LOCAL_NO_EMMA_INSTRUMENT := false
LOCAL_NO_EMMA_COMPILE := false
LOCAL_EMMA_COVERAGE_FILTER := +com.mediatek.camcorder.* +com.mediatek.hardware.CameraEx* +com.mediatek.media.* +com.mediatek.telephony.SmsManagerEx* +com.mediatek.telephony.TelephonyManagerEx* +com.mediatek.build.* +com.mediatek.content.*
endif

LOCAL_JAVA_LIBRARIES := telephony-common voip-common mediatek-common

LOCAL_SRC_FILES := $(call all-java-files-under,$(MTK_FRAMEWORKS_BASE_JAVA_SRC_DIRS))
#LOCAL_INTERMEDIATE_SOURCES := \
#	$(mediatek-res-source-path)/com/mediatek/R.java \
#	$(mediatek-res-source-path)/com/mediatek/Manifest.java 

ANDROID_BT_JB_MR1 := yes

ifeq (yes,$(ANDROID_BT_JB_MR1))
LOCAL_SRC_FILES := $(filter-out bluetooth/java/com/mediatek/bluetooth/4.1/BluetoothAdapterEx.java, $(LOCAL_SRC_FILES))
else
LOCAL_SRC_FILES := $(filter-out bluetooth/java/com/mediatek/bluetooth/4.2/BluetoothAdapterEx.java, $(LOCAL_SRC_FILES))
endif

LOCAL_REQUIRED_MODULES := libmpojni libjni_pq libgifEncoder_jni

ifeq ($(strip $(MTK_BSP_PACKAGE)),no)
LOCAL_STATIC_JAVA_LIBRARIES := static_pluginmanager
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.amsplus
endif

ifeq ($(strip $(MTK_MOBILE_MANAGEMENT)),yes)
LOCAL_STATIC_JAVA_LIBRARIES += mobile_manager
endif

ifeq ($(strip $(MTK_BG_POWER_SAVING_SUPPORT)),yes)
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.amplus
endif
include $(BUILD_JAVA_LIBRARY)

ifeq ($(strip $(BUILD_MTK_API_DEP)), yes)
# mediatek-framework API table.
# ============================================================
LOCAL_MODULE := mediatek-framework-api

LOCAL_STATIC_JAVA_LIBRARIES := 
LOCAL_MODULE_CLASS := JAVA_LIBRARIES

LOCAL_DROIDDOC_OPTIONS:= \
		-stubs $(TARGET_OUT_COMMON_INTERMEDIATES)/JAVA_LIBRARIES/mediatek-framework-api_intermediates/src \
		-api $(TARGET_OUT_COMMON_INTERMEDIATES)/PACKAGING/mediatek-framework-api.txt \
		-nodocs \
        -hidden

include $(BUILD_DROIDDOC)
endif

# ============================================================

# Variables for stub API.
# ============================================================
stub_intermediates := \
	$(TARGET_OUT_COMMON_INTERMEDIATES)/JAVA_LIBRARIES/mediatek-api-stubs_current_intermediates
stub_src_dir := $(stub_intermediates)/src
stub_classes_dir := $(stub_intermediates)/classes
stub_full_target := $(stub_intermediates)/classes.jar
stub_jar := $(HOST_OUT_JAVA_LIBRARIES)/mediatek-android.jar
mediatek_framework_res_package := $(call intermediates-dir-for,APPS,mediatek-res,,COMMON)/package-export.apk

ifeq ($(strip $(MTK_BSP_PACKAGE)),no)
compatibility_full_target := $(TARGET_OUT_COMMON_INTERMEDIATES)/JAVA_LIBRARIES/mediatek-compatibility_intermediates/classes.jar
compatibility_jar := $(HOST_OUT_JAVA_LIBRARIES)/mediatek-compatibility.jar
endif

$(stub_full_target): MEDIATEK_FRAMEWORK_RES_PACKAGE := $(mediatek_framework_res_package)

# ===== Used for MediaTek SDK, please do not modify without SDK group reviewing =====
LOCAL_API_SRC_DIRS := telephony/java
LOCAL_API_SRC_DIRS += core/java/com/mediatek/hardware
LOCAL_API_SRC_DIRS += media/camcorder/java
LOCAL_API_SRC_DIRS += media/java/com/mediatek/media
LOCAL_API_SRC_DIRS += sdkversion/java
LOCAL_API_SRC_DIRS += ../opt/telephony/src/java/com/mediatek/telephony
LOCAL_API_SRC_DIRS += ../../frameworks-ext/base/core/java/com/mediatek/hotknot/
# ===== Used for MediaTek SDK, please do not modify without SDK group reviewing =====

mtk_api_stubs_src_list := $(call all-java-files-under,$(LOCAL_API_SRC_DIRS))

# For now, must specify package names whose sources will be built into the stub library.
stub_package_list := com.mediatek.telephony:com.mediatek.telephony.gemini:com.mediatek.camcorder:com.mediatek.hardware:com.mediatek.telephony.gemini:com.mediatek.media:com.mediatek.build:com.mediatek.hotknot

mediatek-res-source-path := APPS/mediatek-res_intermediates/src

# Stub library source.
# Generate stub source for making MTK SDK shared library.
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(mtk_api_stubs_src_list)
#LOCAL_INTERMEDIATE_SOURCES := \
#	$(mediatek-res-source-path)/com/mediatek/R.java \
#	$(mediatek-res-source-path)/com/mediatek/Manifest.java 
LOCAL_JAVA_LIBRARIES := telephony-common voip-common

LOCAL_MODULE := mediatek-api-stubs

LOCAL_MODULE_CLASS := JAVA_LIBRARIES

LOCAL_DROIDDOC_OPTIONS:= \
		-stubpackages $(stub_package_list) \
		-stubs $(stub_src_dir) \
		-api $(MTK_INTERNAL_PLATFORM_API_FILE) \
		-nodocs

include $(BUILD_DROIDDOC)

#$(full_target): mediatek-res-package-target
$(shell mkdir -p $(OUT_DOCS))

# The target needs mediatek-res files.

$(MTK_INTERNAL_PLATFORM_API_FILE): $(full_target)

# Keep module path for build dependency.
mediatek_api_stubs_src := $(full_target)

# Create stub shared library only on banyan_addon and banyan_addon_x86 build.
ifneq ($(filter banyan_addon banyan_addon_x86,$(TARGET_PRODUCT)),)

# Documentation.
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(mtk_api_stubs_src_list)

LOCAL_JAVA_LIBRARIES := telephony-common voip-common
LOCAL_MODULE:= mediatek-sdk
LOCAL_DROIDDOC_OPTIONS := \
    -title "MediaTek SDK" \
    -offlinemode \
    -hdf android.whichdoc online

LOCAL_MODULE_CLASS := JAVA_LIBRARIES

include $(BUILD_DROIDDOC)
# ============================================================

# Stub library.
# ============================================================

# Reuse class paths from "mediatek-android" module build. Need them to build stub source.
stub_classpath := $(subst $(space),:,$(full_java_libs))

# Build stubs first if in banyan_addon target.

$(stub_full_target): $(mediatek_api_stubs_src)
	@echo Compiling MTK SDK stubs: $@
	rm -rf $(stub_classes_dir)
	mkdir -p $(stub_classes_dir)
	find $(stub_src_dir) -name "*.java" > $(stub_intermediates)/java-source-list
	$(TARGET_JAVAC) -encoding ascii -bootclasspath "" \
		-classpath $(stub_classpath) \
		-g $(xlint_unchecked) \
		-extdirs "" -d $(stub_classes_dir) \
		\@$(stub_intermediates)/java-source-list \
		|| ( rm -rf $(stub_classes_dir) ; exit 41 )
	jar -cf $@ -C $(stub_classes_dir) .
	
$(stub_jar): $(stub_full_target) 
	mkdir -p $(dir $@)
	cp $< $@

ifeq ($(strip $(MTK_BSP_PACKAGE)),no)
$(compatibility_jar): $(compatibility_full_target)
	cp $(compatibility_full_target) $(compatibility_jar)
endif

endif

ifeq ($(strip $(BUILD_MTK_API_MONITOR)), yes)

include $(LOCAL_PATH)/internal_api_src.mk

# MediaTek internal API table.
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(INTERNAL_API_SRC_FILES)
#LOCAL_INTERMEDIATE_SOURCES := $(framework_docs_LOCAL_INTERMEDIATE_SOURCES)
LOCAL_JAVA_LIBRARIES := mediatek-framework telephony-common services voip-common
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_ADDITIONAL_JAVA_DIR := $(INTERNAL_API_ADDITIONAL_SRC_DIR)

LOCAL_MODULE := mediatek-internal-api-stubs

LOCAL_DROIDDOC_OPTIONS:= \
	-api $(MTK_INTERNAL_MONITORING_API_FILE) \
	-nodocs \
	-internal \
	-warning 106 -warning 110

include $(BUILD_DROIDDOC)

$(MTK_INTERNAL_MONITORING_API_FILE): $(full_target)

# MediaTek internal API document.
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(INTERNAL_API_SRC_FILES)
#LOCAL_INTERMEDIATE_SOURCES := $(framework_docs_LOCAL_INTERMEDIATE_SOURCES)
LOCAL_JAVA_LIBRARIES := mediatek-framework telephony-common
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_ADDITIONAL_JAVA_DIR := $(INTERNAL_API_ADDITIONAL_SRC_DIR)

LOCAL_MODULE := mediatek-internal-sdk

LOCAL_DROIDDOC_OPTIONS := \
	-title "MediaTek Internal API Document" \
	-offlinemode \
	-hdf android.whichdoc online \
	-internal \
	-warning 106

include $(BUILD_DROIDDOC)

endif

# ============================================================
ifeq (,$(ONE_SHOT_MAKEFILE))
include $(call first-makefiles-under,$(LOCAL_PATH))
$(mediatek_api_stubs_src):$(ALL_MODULES.mediatek-res.BUILT)
endif
