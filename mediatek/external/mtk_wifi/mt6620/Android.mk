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

LOCAL_PATH := $(call my-dir)

ifeq ($(MTK_WLAN_SUPPORT),yes)
# --------------------------------------------------------------------
# MT6620 Wi-Fi Specific Files
# --------------------------------------------------------------------
  #ifeq ($(MTK_WLAN_CHIP),MT6620)

# --------------------------------------------------------------------
# MT6620 Wi-Fi E1~E5 Firmware
# --------------------------------------------------------------------
    include $(CLEAR_VARS)
    LOCAL_MODULE := WIFI_RAM_CODE
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
    LOCAL_SRC_FILES := E3/$(LOCAL_MODULE)
    include $(BUILD_PREBUILT)
######################################################################

# --------------------------------------------------------------------
# MT6620 Wi-Fi E6 Firmware
# --------------------------------------------------------------------
    include $(CLEAR_VARS)
    LOCAL_MODULE := WIFI_RAM_CODE_E6
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
    LOCAL_SRC_FILES := E6/$(LOCAL_MODULE)
    include $(BUILD_PREBUILT)    
######################################################################

# --------------------------------------------------------------------
# MT6620 Wi-Fi P2P module
# --------------------------------------------------------------------
#    include $(CLEAR_VARS)
#    LOCAL_MODULE := p2p.ko
#    LOCAL_MODULE_CLASS := LIB
#    LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/modules
#    ifeq ($(MTK_PLATFORM), MT6573)
#      LOCAL_SRC_FILES := mt6573/$(LOCAL_MODULE)
#    endif
#    ifeq ($(MTK_PLATFORM), MT6575)
#      LOCAL_SRC_FILES := mt6575/$(LOCAL_MODULE)
#    endif
#    include $(BUILD_PREBUILT)
###################################################################### 
 
  #endif
endif  


