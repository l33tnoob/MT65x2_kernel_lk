# Copyright 2008, The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(strip $(MTK_AUTO_TEST)), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# We only want this apk build for tests.
# LOCAL_MODULE_TAGS := tests
LOCAL_MODULE_TAGS := optional

LOCAL_JAVA_LIBRARIES := android.test.runner
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.mms.ext
#LOCAL_STATIC_JAVA_LIBRARIES += libjunitreport-for-ISmsService-tests

LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/app
# Include all test java files.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := ISmsServiceTests

LOCAL_INSTRUMENTATION_FOR := ISmsService

include $(BUILD_PACKAGE)

#Junit Test option
include $(CLEAR_VARS)
#LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libjunitreport-for-ISmsService-tests:android-junit-report-1.2.6.jar
include $(BUILD_MULTI_PREBUILT)
endif
