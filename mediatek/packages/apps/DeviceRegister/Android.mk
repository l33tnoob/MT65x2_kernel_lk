ifeq ($(MTK_DEVREG_APP),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under,src)

LOCAL_PACKAGE_NAME := DeviceRegister
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += CustomProperties \
                        mediatek-framework \
                        telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common
include $(BUILD_PACKAGE)

endif
