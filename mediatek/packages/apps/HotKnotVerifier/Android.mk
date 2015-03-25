ifeq ($(MTK_HOTKNOT_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := mediatek-framework

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_MODULE_TAGS := debug
LOCAL_PACKAGE_NAME := HotKnotVerifier
LOCAL_PRIVILEGED_MODULE := true

LOCAL_CERTIFICATE := platform


include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
# include $(call all-makefiles-under,$(LOCAL_PATH))


endif
