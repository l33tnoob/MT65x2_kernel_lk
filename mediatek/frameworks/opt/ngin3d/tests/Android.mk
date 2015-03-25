#
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
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := tests

LOCAL_JAVA_LIBRARIES := android.test.runner

LOCAL_STATIC_JAVA_LIBRARIES := libjunitreport-for-ngin3d-tests hamcrest-core-for-ngin3d-tests hamcrest-library-for-ngin3d-tests

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := Ngin3dTest

LOCAL_CERTIFICATE := platform

LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.ngin3d-static

LOCAL_JNI_SHARED_LIBRARIES := libja3m liba3m

LOCAL_EMMA_COVERAGE_FILTER := +com.mediatek.ngin3d.* +com.mediatek.util.*

LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../inflater/res $(LOCAL_PATH)/res

include $(BUILD_PACKAGE)

##################################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libjunitreport-for-ngin3d-tests:libs/android-junit-report-dev.jar hamcrest-core-for-ngin3d-tests:libs/hamcrest-core-SNAPSHOT.jar hamcrest-library-for-ngin3d-tests:libs/hamcrest-library-SNAPSHOT.jar
include $(BUILD_MULTI_PREBUILT)

##################################################
# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

