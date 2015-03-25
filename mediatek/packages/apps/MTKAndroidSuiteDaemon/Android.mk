LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES := libutil

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_EMMA_COVERAGE_FILTER := @$(LOCAL_PATH)/emma_filter.txt,--$(LOCAL_PATH)/emma_method_filter.txt

LOCAL_PACKAGE_NAME := MTKAndroidSuiteDaemon
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES += mediatek-framework telephony-common mediatek-telephony-common

include $(BUILD_PACKAGE)
##################################################
include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libutil:APSTEntity.jar

include $(BUILD_MULTI_PREBUILT)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
