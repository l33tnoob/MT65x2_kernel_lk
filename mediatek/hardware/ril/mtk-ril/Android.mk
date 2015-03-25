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
ifeq ($(GOOGLE_RELEASE_RIL), yes)
#do nothing
else
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq (foo,foo)
  #Mediatek Configuration
  #ifeq ($(TARGET_PRODUCT),generic)
  ifeq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
    LOCAL_SRC_FILES:= \
      ril_callbacks_emu.c \
      ril_cc.c \
      ril_ss.c \
      ril_sim.c \
      ril_stk.c \
      ril_nw.c \
      ril_data.c \
      ril_sms.c \
      ril_oem.c \
      atchannels_emu.c \
      misc.c \
      at_tok.c  \
      usim_fcp_parser.c
  else
  LOCAL_SRC_FILES:= \
    ril_callbacks.c \
    ril_cc.c \
    ril_ss.c \
    ril_sim.c \
    ril_stk.c \
    ril_nw.c \
    ril_data.c \
    ril_sms.c \
    ril_oem.c \
    atchannels.c \
    misc.c \
    at_tok.c  \
    usim_fcp_parser.c
  endif
else
  #Original  	
  LOCAL_SRC_FILES:= \
    reference-ril.c \
    atchannel.c \
    misc.c \
    at_tok.c
endif

LOCAL_SHARED_LIBRARIES := \
	libcutils libutils libnetutils librilmtk 

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaed
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifeq ($(MTK_LTE_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_LTE_SUPPORT
endif


	# for asprinf
	
LOCAL_STATIC_LIBRARIES := \
	mtk-ril-prop-md1
		
ifeq ($(GEMINI),yes)	
  LOCAL_CFLAGS := -D_GNU_SOURCE -DMTK_RIL -DMTK_GEMINI -D__CCMNI_SUPPORT__
else
  LOCAL_CFLAGS := -D_GNU_SOURCE -DMTK_RIL -D__CCMNI_SUPPORT__

  ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
        LOCAL_CFLAGS += -DMTK_GEMINI     
  endif
endif

ifeq ($(MTK_IPV6_SUPPORT),yes)	
  LOCAL_CFLAGS += -DMTK_IPV6_SUPPORT -DINET6
endif

ifeq ($(MTK_GEMINI_3G_SWITCH),yes)	
  LOCAL_CFLAGS += -DMTK_GEMINI_3G_SWITCH
endif

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)

ifeq ($(TELEPHONY_DFOSET),yes)
LOCAL_SHARED_LIBRARIES += libdfo
endif  
LOCAL_C_INCLUDES += $(KERNEL_HEADERS) \
	$(LOCAL_PATH)/../../../external/dfo/featured \
	$(TARGET_OUT_HEADERS)/dfo \
	$(TOPDIR)/hardware/libhardware_legacy/include \
	$(TOPDIR)/hardware/libhardware/include

ifeq ($(TARGET_DEVICE),sooner)
  LOCAL_CFLAGS += -DOMAP_CSMI_POWER_CONTROL -DUSE_TI_COMMANDS 
endif

ifeq ($(TARGET_DEVICE),surf)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

ifeq ($(TARGET_DEVICE),dream)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

ifneq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DFATAL_ERROR_HANDLE
endif

ifeq ($(OPTR_SPEC_SEG_DEF), OP01_SPEC0200_SEGC)
	LOCAL_CFLAGS += -DMTK_CMCC_WORLD_PHONE_TEST
endif

LOCAL_CFLAGS += -DMTK_RIL_MD1

ifeq (foo,foo)
  #build shared library
  LOCAL_SHARED_LIBRARIES += \
	libcutils libutils
	LOCAL_STATIC_LIBRARIES := \
	mtk-ril-prop-md1
  LOCAL_LDLIBS += -lpthread
  LOCAL_CFLAGS += -DRIL_SHLIB
  LOCAL_MODULE:= mtk-ril
  include $(BUILD_SHARED_LIBRARY)
else
  #build executable
  LOCAL_SHARED_LIBRARIES += \
	librilmtk
	LOCAL_STATIC_LIBRARIES := \
	mtk-ril-prop-md1
  LOCAL_MODULE:= mtk-ril
  include $(BUILD_EXECUTABLE)
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaed
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

#====================================

include $(CLEAR_VARS)

ifeq (foo,foo)
  #Mediatek Configuration
  #ifeq ($(TARGET_PRODUCT),generic)
  ifeq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
    LOCAL_SRC_FILES:= \
      ril_callbacks_emu.c \
      ril_cc.c \
      ril_ss.c \
      ril_sim.c \
      ril_stk.c \
      ril_nw.c \
      ril_data.c \
      ril_sms.c \
      ril_oem.c \
      atchannels_emu.c \
      misc.c \
      at_tok.c  \
      usim_fcp_parser.c
  else
  LOCAL_SRC_FILES:= \
    ril_callbacks.c \
    ril_cc.c \
    ril_ss.c \
    ril_sim.c \
    ril_stk.c \
    ril_nw.c \
    ril_data.c \
    ril_sms.c \
    ril_oem.c \
    atchannels.c \
    misc.c \
    at_tok.c  \
    usim_fcp_parser.c
  endif
else
  #Original  	
  LOCAL_SRC_FILES:= \
    reference-ril.c \
    atchannel.c \
    misc.c \
    at_tok.c
endif

LOCAL_SHARED_LIBRARIES := \
	libcutils libutils libnetutils librilmtkmd2 


ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaed
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifeq ($(MTK_LTE_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_LTE_SUPPORT
endif


	# for asprinf
	
LOCAL_STATIC_LIBRARIES := \
	mtk-ril-prop-md2	
	
ifeq ($(GEMINI),yes)	
  LOCAL_CFLAGS := -D_GNU_SOURCE -DMTK_RIL -DMTK_GEMINI -D__CCMNI_SUPPORT__
else
  LOCAL_CFLAGS := -D_GNU_SOURCE -DMTK_RIL -D__CCMNI_SUPPORT__

  ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
        LOCAL_CFLAGS += -DMTK_GEMINI     
  endif
endif

ifeq ($(MTK_IPV6_SUPPORT),yes)	
  LOCAL_CFLAGS += -DMTK_IPV6_SUPPORT -DINET6
endif

ifeq ($(MTK_GEMINI_3G_SWITCH),yes)	
  LOCAL_CFLAGS += -DMTK_GEMINI_3G_SWITCH
endif

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)

ifeq ($(TELEPHONY_DFOSET),yes)
LOCAL_SHARED_LIBRARIES += libdfo
endif  
LOCAL_C_INCLUDES += $(KERNEL_HEADERS) \
	$(LOCAL_PATH)/../../../external/dfo/featured \
	$(TARGET_OUT_HEADERS)/dfo \
	$(TOPDIR)/hardware/libhardware_legacy/include \
	$(TOPDIR)/hardware/libhardware/include

ifeq ($(TARGET_DEVICE),sooner)
  LOCAL_CFLAGS += -DOMAP_CSMI_POWER_CONTROL -DUSE_TI_COMMANDS 
endif

ifeq ($(TARGET_DEVICE),surf)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

ifeq ($(TARGET_DEVICE),dream)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

ifeq ($(OPTR_SPEC_SEG_DEF), OP01_SPEC0200_SEGC)
	LOCAL_CFLAGS += -DMTK_CMCC_WORLD_PHONE_TEST
endif

LOCAL_CFLAGS += -DMTK_RIL_MD2

ifeq (foo,foo)
  #build shared library
  LOCAL_SHARED_LIBRARIES += \
	libcutils libutils
	LOCAL_STATIC_LIBRARIES := \
	mtk-ril-prop-md2
  LOCAL_LDLIBS += -lpthread
  LOCAL_CFLAGS += -DRIL_SHLIB
  LOCAL_MODULE:= mtk-rilmd2
  include $(BUILD_SHARED_LIBRARY)
else
  #build executable
  LOCAL_SHARED_LIBRARIES += \
	librilmtkmd2
	LOCAL_STATIC_LIBRARIES := \
	mtk-ril-prop-md2
  LOCAL_MODULE:= mtk-rilmd2
  include $(BUILD_EXECUTABLE)
endif

endif #GOOGLE_RELEASE_RIL
