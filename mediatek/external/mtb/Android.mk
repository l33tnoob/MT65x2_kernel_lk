# Copyright Statement:
#
#
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

ifneq ($(TARGET_BUILD_VARIANT),user)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_SRC_FILES := mtb.cpp \
                   mtb_tracer_factory.c \
                   met_vp.c \
                   mmp_vp.c \
                   mmp_wfd.c \
                   mmp_hash.c

LOCAL_CFLAGS    := -DMTB_SUPPORT  # -DMET_USER_EVENT_SUPPORT
LOCAL_SHARED_LIBRARIES:= \
                         libcutils \
                         libutils 


ifeq ($(strip $(MTK_MMPROFILE_SUPPORT)),yes)
LOCAL_SHARED_LIBRARIES += libmmprofile
endif

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/kernel/include \
                    $(TOPDIR)/kernel/include \
                    $(TOPDIR)/system/core/include 
                    
ifeq ($(strip $(MET_USER_EVENT_SUPPORT)),yes)
LOCAL_SHARED_LIBRARIES += libmet-tag
LOCAL_C_INCLUDES += $(TOP)/mediatek/external/met/met-tag
endif

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE    := libmtb

include $(BUILD_SHARED_LIBRARY)

########################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= mtk_setprop.cpp

LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libcutils \
    libutils \

LOCAL_MODULE:= mtk_setprop
    
LOCAL_MODULE_TAGS := eng

include $(BUILD_EXECUTABLE)

endif


