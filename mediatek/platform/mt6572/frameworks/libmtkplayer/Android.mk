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

$(warning "lw1")

LOCAL_PATH:= $(call my-dir)
LOCAL_COMMON_PATH:=../../../common

#
# libmtkplayer(depend on libaudiosetting and audio hal)
#

#ifneq ($(strip $(BOARD_USES_GENERIC_AUDIO)),true)

#ifneq ($(strip $(HAVE_MATV_FEATURE))_$(strip $(MTK_FM_SUPPORT)), no_no)

include $(CLEAR_VARS)

$(warning "lw1")
LOCAL_SRC_FILES+= \
    $(LOCAL_COMMON_PATH)/frameworks/libmtkplayer/mATVAudioPlayer.cpp
ifneq ($(strip $(MTK_FM_SUPPORT)), no)
    LOCAL_SRC_FILES+= \
    $(LOCAL_COMMON_PATH)/frameworks/libmtkplayer/FMAudioPlayer.cpp
endif
ifeq ($(strip $(BOARD_USES_MTK_AUDIO)),true)
    LOCAL_SRC_FILES+= \
    $(LOCAL_COMMON_PATH)/frameworks/libmtkplayer/VibSpkAudioPlayer.cpp
endif

ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl -lpthread
endif

LOCAL_SHARED_LIBRARIES :=     \
	libcutils             \
	libutils              \
	libbinder             \
	libmedia              \
	libaudiosetting       

ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

LOCAL_C_INCLUDES :=  \
	$(JNI_H_INCLUDE)                                              \
	$(TOP)/frameworks/av/services/audioflinger                    \
	$(TOP)/frameworks/av/include/media                            \
  $(TOP)/frameworks/av/include                                  \
  $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av                   \
	$(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/include           \
	$(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/include/media     \
	$(TOP)/$(MTK_PATH_PLATFORM)/frameworks/av/media/libmtkplayer	\
	$(TOP)/$(MTK_PATH_SOURCE)/external/matvctrl                   \
	
ifeq ($(strip $(BOARD_USES_MTK_AUDIO)),true)
LOCAL_C_INCLUDES+= \
   $(TOP)/$(MTK_PATH_SOURCE)/platform/common/hardware/audio/include \
   $(TOP)/$(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
   $(TOP)/$(MTK_PATH_SOURCE)/external/cvsd_plc_codec \
   $(TOP)/$(MTK_PATH_SOURCE)/external/msbc_codec \
   $(call include-path-for, audio-utils) \
   $(call include-path-for, audio-effects) \
   $(TOP)/$(MTK_PATH_SOURCE)/external/audiodcremovefl \
   $(MTK_PATH_SOURCE)/external/AudioSpeechEnhancement/inc \
   $(MTK_PATH_SOURCE)/external/audiodcremoveflt \
   $(MTK_PATH_SOURCE)/external/audiocustparam \
   $(MTK_PATH_SOURCE)/external/AudioComponentEngine
   
LOCAL_SHARED_LIBRARIES += libaudio.primary.default

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaed
    LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/external/aee/binary/inc
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif
else
LOCAL_CFLAGS += -DFAKE_FM
LOCAL_CFLAGS += -DFAKE_MATV
LOCAL_CFLAGS += -DFAKE_VIBSPK
endif

ifeq ($(MTK_VIBSPK_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_VIBSPK_SUPPORT
endif

$(warning "lw2")
LOCAL_MODULE:= libmtkplayer

LOCAL_PRELINK_MODULE := no

include $(BUILD_SHARED_LIBRARY)

#endif

