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

# Build for a3m makes only /tests
LOCAL_MAKE_FILES :=$(call all-named-subdir-makefiles, tests demos)

#################################################
# built as system java library

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_JAVA_LIBRARIES := com.mediatek.ja3m-static
LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_MODULE := com.mediatek.ngin3d
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_JAVA_LIBRARY)

#################################################
# built as static java library

include $(CLEAR_VARS)

LOCAL_MODULE := com.mediatek.ngin3d-static
LOCAL_STATIC_JAVA_LIBRARIES := com.mediatek.ja3m-static
LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_PROGUARD_SOURCE=javaclassfile
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_STATIC_JAVA_LIBRARY)

#################################################
# permission file

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := com.mediatek.ngin3d.xml
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

##################################################
# Use the folloing include to make our test apk.

include $(LOCAL_MAKE_FILES)
