# Copyright (C) 2009 The Android Open Source Project
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

ifeq ($(MTK_SENSOR_SUPPORT),yes)
LOCAL_PATH := $(call my-dir)

ifeq ($(CUSTOM_KERNEL_MAGNETOMETER), )
$(shell rm -f $(PRODUCT_OUT)/system/etc/permissions/android.hardware.sensor.compass.xml)
endif
ifeq ($(CUSTOM_KERNEL_ACCELEROMETER), )
$(shell rm -f $(PRODUCT_OUT)/system/etc/permissions/android.hardware.sensor.accelerometer.xml)
endif
ifeq ($(CUSTOM_KERNEL_ALSPS), )
$(shell rm -f $(PRODUCT_OUT)/system/etc/permissions/android.hardware.sensor.light.xml)
$(shell rm -f $(PRODUCT_OUT)/system/etc/permissions/android.hardware.sensor.proximity.xml)
endif
ifeq ($(CUSTOM_KERNEL_GYROSCOPE), )
$(shell rm -f $(PRODUCT_OUT)/system/etc/permissions/android.hardware.sensor.gyroscope.xml)
endif

# HAL module implemenation, not prelinked and stored in
# hw/<SENSORS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_SRC_FILES := sensors.c  hwmsen_chip_info.c nusensors.cpp InputEventReader.cpp SensorBase.cpp Hwmsen.cpp Acceleration.cpp Magnetic.cpp sensordebug.cpp
#LOCAL_SRC_FILES := sensors_hwmsen.c  hwmsen_chip_info.c
LOCAL_C_INCLUDES+= \
        $(MTK_PATH_SOURCE)/hardware/sensor/ \
	$(MTK_ROOT_CUSTOM_OUT)/hal/sensors

LOCAL_MODULE := sensors.default
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
endif
