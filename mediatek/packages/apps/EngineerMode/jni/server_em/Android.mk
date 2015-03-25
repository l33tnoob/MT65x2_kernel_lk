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

ifeq ($(MTK_ENGINEERMODE_APP), yes)


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

  
LOCAL_SRC_FILES := main.cpp \
		AFMThread.cpp \
		AFMSync.cpp \
		AFMSocket.cpp \
		server.cpp \
		RPCClient.cpp \
		Modules.cpp \
		ModuleBasebandRegDump.cpp \
		ModuleCpuFreqTest.cpp \
		ModuleFB0.cpp \
		ModuleCpuStress.cpp \
		ModuleSensor.cpp \
		ModuleMsdc.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
    mediatek/external/sensor-tools \
		$(MTK_PATH_SOURCE)/kernel/drivers/video \
		$(TOP)/frameworks/base/include/media \
		$(LOCAL_PATH)/../chip
		
LOCAL_SHARED_LIBRARIES := \
	  libnativehelper \
    libandroid_runtime \
    libem_support_jni \
	  libutils \
    libhwm \
		libbinder \
		libgui \
    libcutils

ifeq ($(MTK_DFO_RESOLUTION_SUPPORT), yes)

LOCAL_CFLAGS += -DEM_DFO_SUPPORT

LOCAL_SRC_FILES += ModuleDfo.cpp

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/dfo \
		$(PLATFORM_PATH)/dfo \
		$(TOP)/mediatek/external/meta/common/inc \
		$(TOP)/$(MTK_ROOT)/kernel/drivers/video

LOCAL_SHARED_LIBRARIES += libcutils \
		libnvram \
		libft

LOCAL_STATIC_LIBRARIES := libmeta_dfo

endif

ifneq ($(MTK_DFO_RESOLUTION_SUPPORT), yes)
LOCAL_SRC_FILES += ModuleDfoDummy.cpp
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := em_svr
LOCAL_PRELINK_MODULE := false
include $(BUILD_EXECUTABLE)

endif


