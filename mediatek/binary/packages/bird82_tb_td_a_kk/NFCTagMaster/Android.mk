LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := NFCTagMaster
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS
LOCAL_STATIC_JAVA_LIBRARIES := com.android.vcard


LOCAL_CERTIFICATE := platform


include $(BUILD_PREBUILT)

# Use the folloing include to make our test apk.
