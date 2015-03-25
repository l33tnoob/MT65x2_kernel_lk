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

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := tests

LOCAL_SRC_FILES := $(call all-java-files-under, src)\
              $(call all-java-files-under, core/runner/src)

LOCAL_JAVA_LIBRARIES := android.test.runner

#LOCAL_JNI_SHARED_LIBRARIES := libcts_jni

# Resource unit tests use a private locale and some densities
LOCAL_AAPT_FLAGS = -c xx_YY -c cs -c 32dpi -c 240dpi -c 160dpi

LOCAL_PACKAGE_NAME := MediatekAnrTestStubs

include $(BUILD_PACKAGE)

# Build the test APK using its own makefile, and any other CTS-related packages
include $(call all-makefiles-under,$(LOCAL_PATH))
