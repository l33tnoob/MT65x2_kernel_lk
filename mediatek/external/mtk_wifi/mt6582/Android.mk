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
# MT6628 Wi-Fi Specific Files
# --------------------------------------------------------------------
  #ifeq ($(MTK_WLAN_CHIP),MT6628)

# --------------------------------------------------------------------
# MT6628 Wi-Fi E1 Firmware
# --------------------------------------------------------------------
    include $(CLEAR_VARS)
    LOCAL_MODULE := WIFI_RAM_CODE_MT6582
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
    LOCAL_SRC_FILES := $(LOCAL_MODULE)
    include $(BUILD_PREBUILT)
######################################################################

  #endif
endif  


