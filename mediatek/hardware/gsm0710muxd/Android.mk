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


# Copyright 2008 Texas Instruments
#
#Author(s) Mikkel Christensen (mlc@ti.com) and Ulrik Bech Hald (ubh@ti.com)

#
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= gsm0710muxd

LOCAL_SRC_FILES:= \
	src/gsm0710muxd.c \
	src/gsm0710muxd_fc.c \

LOCAL_SHARED_LIBRARIES := \
	libcutils

ifeq ($(TELEPHONY_DFOSET),yes)
LOCAL_SHARED_LIBRARIES += libdfo
endif

LOCAL_CFLAGS := \
	-DMUX_ANDROID \
	-D__CCMNI_SUPPORT__ \
	-D__MUXD_FLOWCONTROL__ \

ifeq ($(GEMINI),yes)
	LOCAL_CFLAGS += -DMTK_GEMINI
else
    ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
   LOCAL_CFLAGS += -DMTK_GEMINI 
  endif
endif


ifeq ($(MTK_DT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DT_SUPPORT
endif


ifneq ($(MTK_INTERNAL),yes)
	LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

ifeq ($(MTK_VT3G324M_SUPPORT),yes)
  LOCAL_CFLAGS += -D__ANDROID_VT_SUPPORT__
endif

ifeq ($(MT6280_SUPER_DONGLE),yes)
  LOCAL_CFLAGS += -DPOLL_MODEM_LONGER
endif

LOCAL_CFLAGS += -DMTK_RIL_MD1

LOCAL_C_INCLUDES := $(KERNEL_HEADERS) \
	$(TOPDIR)/hardware/libhardware_legacy/include \
	$(TOPDIR)/hardware/libhardware/include \
	$(MTK_PATH_SOURCE)/external/dfo/featured/ \
	$(TARGET_OUT_HEADERS)/dfo/

LOCAL_LDLIBS := -lpthread

include $(BUILD_EXECUTABLE)

#===========================
include $(CLEAR_VARS)

LOCAL_MODULE:= gsm0710muxdmd2

LOCAL_SRC_FILES:= \
	src/gsm0710muxd.c \
	src/gsm0710muxd_fc.c \

LOCAL_SHARED_LIBRARIES := \
	libcutils

ifeq ($(TELEPHONY_DFOSET),yes)
LOCAL_SHARED_LIBRARIES += libdfo
endif

LOCAL_CFLAGS := \
	-DMUX_ANDROID \
	-D__CCMNI_SUPPORT__ \
	-D__MUXD_FLOWCONTROL__ \

ifeq ($(GEMINI),yes)
	LOCAL_CFLAGS += -DMTK_GEMINI
else
    ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
   LOCAL_CFLAGS += -DMTK_GEMINI 
  endif
endif

ifeq ($(MTK_DT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DT_SUPPORT
endif


ifneq ($(MTK_INTERNAL),yes)
	LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

ifeq ($(MTK_VT3G324M_SUPPORT),yes)
  LOCAL_CFLAGS += -D__ANDROID_VT_SUPPORT__
endif

LOCAL_CFLAGS += -DMTK_RIL_MD2

LOCAL_C_INCLUDES := $(KERNEL_HEADERS) \
	$(TOPDIR)/hardware/libhardware_legacy/include \
	$(TOPDIR)/hardware/libhardware/include \
	$(MTK_PATH_SOURCE)/external/dfo/featured/ \
	$(TARGET_OUT_HEADERS)/dfo/
                    
LOCAL_LDLIBS := -lpthread

include $(BUILD_EXECUTABLE)
