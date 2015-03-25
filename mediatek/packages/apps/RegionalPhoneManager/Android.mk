LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

    LOCAL_MODULE_TAGS := optional
    
    LOCAL_SRC_FILES := $(call all-java-files-under, src)
    LOCAL_JAVA_LIBRARIES += mediatek-framework

    LOCAL_STATIC_JAVA_LIBRARIES := \
        android-common \
        com.mediatek.rpm.ext

    LOCAL_PACKAGE_NAME := RegionalPhoneManager
    LOCAL_CERTIFICATE := platform

    LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res

    LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
