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

# Configuration
BUILD_PATCH  := false
BUILD_WMT_CFG_L2   := false
BUILD_WMT_CFG_L1   := false
BUILD_MT6620 := false
BUILD_MT6628 := false
BUILD_MT6582_CONSYS :=false



ifeq ($(strip $(MTK_COMBO_SUPPORT)), yes)

LOCAL_PATH := $(call my-dir)

BUILD_PATCH := true
BUILD_WMT_CFG_L2 := true

ifeq ($(BUILD_WMT_CFG_L2), true)
    cfg_folder := $(MTK_ROOT_CUSTOM_OUT)/hal/ant
    $(info cfg_folder:$(cfg_folder))
endif

ifeq ($(BUILD_PATCH), true)
    patch_folder := $(MTK_ROOT_CUSTOM_OUT)/hal/combo
    $(info $(patch_folder))
endif

ifneq ($(filter MT6620E3,$(MTK_COMBO_CHIP)),)
    BUILD_MT6620 := true
    BUILD_WMT_CFG_L1 := true
endif

ifneq ($(filter MT6620,$(MTK_COMBO_CHIP)),)
    BUILD_MT6620 := true
    BUILD_WMT_CFG_L1 := true
endif

ifneq ($(filter MT6628,$(MTK_COMBO_CHIP)),)
    BUILD_MT6628 := true
    BUILD_WMT_CFG_L1 := true
endif

ifneq ($(filter CONSYS_6572,$(MTK_COMBO_CHIP)),)
    BUILD_MT6582_CONSYS := true
endif

ifneq ($(filter CONSYS_6582,$(MTK_COMBO_CHIP)),)
    BUILD_MT6582_CONSYS := true
endif

ifneq ($(filter CONSYS_6592,$(MTK_COMBO_CHIP)),)
    BUILD_MT6582_CONSYS := true
endif

##### INSTALL WMT.CFG FOR COMBO CONFIG #####
ifeq ($(BUILD_WMT_CFG_L1), true)
PRODUCT_COPY_FILES += $(cfg_folder)/WMT.cfg:system/etc/firmware/WMT.cfg
endif

ifeq ($(BUILD_MT6620), true)
$(warning building MT6620)
ifneq ($(filter mt6620_ant_m1,$(CUSTOM_HAL_ANT)),)
$(warning building mt6620_ant_m1)
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6620_ant_m1.cfg:system/etc/firmware/mt6620_ant_m1.cfg
endif

ifneq ($(filter mt6620_ant_m2,$(CUSTOM_HAL_ANT)),)
$(warning building mt6620_ant_m2)    
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6620_ant_m2.cfg:system/etc/firmware/mt6620_ant_m2.cfg
endif

ifneq ($(filter mt6620_ant_m3,$(CUSTOM_HAL_ANT)),)
$(warning building mt6620_ant_m3)     
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6620_ant_m3.cfg:system/etc/firmware/mt6620_ant_m3.cfg
endif

ifneq ($(filter mt6620_ant_m4,$(CUSTOM_HAL_ANT)),)
$(warning building mt6620_ant_m4)     
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6620_ant_m4.cfg:system/etc/firmware/mt6620_ant_m4.cfg
endif

ifneq ($(filter mt6620_ant_m5,$(CUSTOM_HAL_ANT)),)
$(warning building mt6620_ant_m5)      
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6620_ant_m5.cfg:system/etc/firmware/mt6620_ant_m5.cfg
endif

ifneq ($(filter mt6620_ant_m6,$(CUSTOM_HAL_ANT)),)
$(warning building mt6620_ant_m6)         
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6620_ant_m6.cfg:system/etc/firmware/mt6620_ant_m6.cfg
endif

ifneq ($(filter mt6620_ant_m7,$(CUSTOM_HAL_ANT)),)
$(warning building mt6620_ant_m7)         
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6620_ant_m7.cfg:system/etc/firmware/mt6620_ant_m7.cfg
endif 

$(warning building mt6620_patch_e3_0_hdr) 
    PRODUCT_COPY_FILES += $(patch_folder)/mt6620_patch_e3_0_hdr.bin:system/etc/firmware/mt6620_patch_e3_0_hdr.bin

$(warning building mt6620_patch_e3_1_hdr) 
    PRODUCT_COPY_FILES += $(patch_folder)/mt6620_patch_e3_1_hdr.bin:system/etc/firmware/mt6620_patch_e3_1_hdr.bin

$(warning building mt6620_patch_e3_2_hdr) 
    PRODUCT_COPY_FILES += $(patch_folder)/mt6620_patch_e3_2_hdr.bin:system/etc/firmware/mt6620_patch_e3_2_hdr.bin

$(warning building mt6620_patch_e3_3_hdr) 
    PRODUCT_COPY_FILES += $(patch_folder)/mt6620_patch_e3_3_hdr.bin:system/etc/firmware/mt6620_patch_e3_3_hdr.bin

$(warning building MT6620 finished)
endif


ifeq ($(BUILD_MT6628), true)
$(warning building MT6628)

ifneq ($(filter mt6628_ant_m1,$(CUSTOM_HAL_ANT)),)
$(warning building mt6628_ant_m1)
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6628_ant_m1.cfg:system/etc/firmware/mt6628_ant_m1.cfg
endif  

ifneq ($(filter mt6628_ant_m2,$(CUSTOM_HAL_ANT)),)
$(warning building mt6628_ant_m2)    
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6628_ant_m2.cfg:system/etc/firmware/mt6628_ant_m2.cfg
endif

ifneq ($(filter mt6628_ant_m3,$(CUSTOM_HAL_ANT)),)
$(warning building mt6628_ant_m3)       
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6628_ant_m3.cfg:system/etc/firmware/mt6628_ant_m3.cfg
endif

ifneq ($(filter mt6628_ant_m4,$(CUSTOM_HAL_ANT)),)
$(warning building mt6628_ant_m4)        
    PRODUCT_COPY_FILES += $(cfg_folder)/mt6628_ant_m4.cfg:system/etc/firmware/mt6628_ant_m4.cfg
endif

$(warning building mt6628_patch_e1_hdr) 
    PRODUCT_COPY_FILES += $(patch_folder)/mt6628_patch_e1_hdr.bin:system/etc/firmware/mt6628_patch_e1_hdr.bin
    
$(warning building mt6628_patch_e2_hdr) 
    PRODUCT_COPY_FILES += $(patch_folder)/mt6628_patch_e2_0_hdr.bin:system/etc/firmware/mt6628_patch_e2_0_hdr.bin
    PRODUCT_COPY_FILES += $(patch_folder)/mt6628_patch_e2_1_hdr.bin:system/etc/firmware/mt6628_patch_e2_1_hdr.bin
    
$(warning building MT6628 finished)
endif

ifeq ($(BUILD_MT6582_CONSYS), true)
$(warning building SOC_CONSYS)

ifneq ($(filter mt6582_ant_m1,$(CUSTOM_HAL_ANT)),)
$(warning building SOC_CONSYS ant mode)
    PRODUCT_COPY_FILES += $(cfg_folder)/WMT_SOC.cfg:system/etc/firmware/WMT_SOC.cfg
endif  

$(warning building mt6572_82_patch_e1_hdr) 
    PRODUCT_COPY_FILES += $(patch_folder)/mt6572_82_patch_e1_0_hdr.bin:system/etc/firmware/mt6572_82_patch_e1_0_hdr.bin
    PRODUCT_COPY_FILES += $(patch_folder)/mt6572_82_patch_e1_1_hdr.bin:system/etc/firmware/mt6572_82_patch_e1_1_hdr.bin
    
$(warning building SOC_CONSYS finished)
endif

# PRODUCT_PACKAGES part

ADD_MT6620 := false
ADD_MT6628 := false
ADD_MT6582 := false


  PRODUCT_PACKAGES += WMT.cfg \
    6620_launcher \
    6620_wmt_concurrency \
    6620_wmt_lpbk \
    wmt_loader \
    stp_dump3

ifneq ($(filter MT6620E3,$(MTK_COMBO_CHIP)),)
    ADD_MT6620 := true
endif

ifneq ($(filter MT6620,$(MTK_COMBO_CHIP)),)
    ADD_MT6620 := true
endif

ifneq ($(filter CONSYS_6582,$(MTK_COMBO_CHIP)),)
    ADD_MT6582 := true
endif

ifneq ($(filter CONSYS_6572,$(MTK_COMBO_CHIP)),)
    ADD_MT6582 := true
endif

ifneq ($(filter CONSYS_6592,$(MTK_COMBO_CHIP)),)
    ADD_MT6582 := true
endif

ifeq ($(ADD_MT6582), true)
  PRODUCT_PACKAGES += mt6572_82_patch_e1_0_hdr.bin \
                      mt6572_82_patch_e1_1_hdr.bin \
                      WMT_SOC.cfg
endif


ifneq ($(filter MT6628,$(MTK_COMBO_CHIP)),)
    ADD_MT6628 := true
endif


ifeq ($(ADD_MT6620), true)
  PRODUCT_PACKAGES += mt6620_patch_e3_hdr.bin \
    mt6620_patch_e3_0_hdr.bin \
    mt6620_patch_e3_1_hdr.bin \
    mt6620_patch_e3_2_hdr.bin \
    mt6620_patch_e3_3_hdr.bin \
    mt6620_patch_e6_hdr.bin

  ifneq ($(filter mt6620_ant_m1,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m1.cfg
  endif

  ifneq ($(filter mt6620_ant_m2,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m2.cfg
  endif

  ifneq ($(filter mt6620_ant_m3,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m3.cfg
  endif

  ifneq ($(filter mt6620_ant_m4,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m4.cfg
  endif

  ifneq ($(filter mt6620_ant_m5,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m5.cfg
  endif

  ifneq ($(filter mt6620_ant_m6,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m6.cfg
  endif

  ifneq ($(filter mt6620_ant_m7,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m7.cfg
  endif

endif


ifeq ($(ADD_MT6628), true)
  PRODUCT_PACKAGES += mt6628_patch_e1_hdr.bin \
    mt6628_patch_e2_hdr.bin \
    mt6628_patch_e2_0_hdr.bin \
    mt6628_patch_e2_1_hdr.bin

  ifneq ($(filter mt6628_ant_m1,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6628_ant_m1.cfg
  endif

  ifneq ($(filter mt6628_ant_m2,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6628_ant_m2.cfg
  endif

  ifneq ($(filter mt6628_ant_m3,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6628_ant_m3.cfg
  endif

  ifneq ($(filter mt6628_ant_m4,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6628_ant_m4.cfg
  endif

endif


# end PRODUCT_PACKAGES

PRODUCT_PROPERTY_OVERRIDES += persist.mtk.wcn.combo.chipid=-1


endif
