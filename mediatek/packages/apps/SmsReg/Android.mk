ifeq ($(MTK_SMSREG_APP),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

#LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SRC_FILES := $(call all-java-files-under,src)

LOCAL_PACKAGE_NAME := SmsReg

#LOCAL_PROGUARD_ENABLED := full

LOCAL_JAVA_LIBRARIES += CustomProperties
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif
