LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifneq ($(TARGET_BUILD_VARIANT), eng)
LOCAL_MANIFEST_FILE := user/AndroidManifest.xml
endif

LOCAL_MODULE_TAGS := optional
#use this to distinguish android version which support or not support ActionBar and Switch
LOCAL_SUPPORT_ACTION_BAR := yes

SRC_ROOT := src/com/mediatek/mtklogger
LOCAL_MODULE := MTKLogger
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS

ifeq ($(MTK_LOG2SERVER_APP), yes)
LOCAL_MODULE := MTKLogger
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS
LOCAL_STATIC_JAVA_LIBRARIES := ftpLib1 \
                               lib11 \
			                   lib21
else
LOG2SERVER_SRC := $(call all-java-files-under, $(SRC_ROOT)/exceptionreporter)
LOCAL_MODULE := MTKLogger
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS
endif

ifeq ($(LOCAL_SUPPORT_ACTION_BAR), yes)
LOCAL_MODULE := MTKLogger
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS
else
LOCAL_MODULE := MTKLogger
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS
endif

LOCAL_MODULE := MTKLogger
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS



LOCAL_CERTIFICATE := platform


LOCAL_JAVA_LIBRARIES := mediatek-framework

LOCAL_EMMA_COVERAGE_FILTER := @$(LOCAL_PATH)/emma_filter.txt,--$(LOCAL_PATH)/emma_filter_method.txt
#EMMA_INSTRUMENT := true

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PREBUILT)

ifeq ($(MTK_LOG2SERVER_APP),  yes)
include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := ftpLib1:lib/commons-net-3.1.jar \
                                        lib11:lib/activation.jar \
					                              lib21:lib/additionnal.jar
endif

# Use the folloing include to make our test apk.
