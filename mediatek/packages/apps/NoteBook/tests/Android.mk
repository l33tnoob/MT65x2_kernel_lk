ifeq ($(strip $(MTK_AUTO_TEST)), yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# We only want this apk build for tests.
LOCAL_MODULE_TAGS := tests

LOCAL_JAVA_LIBRARIES := android.test.runner

LOCAL_STATIC_JAVA_LIBRARIES := librobotium4

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := NotebookTests

LOCAL_CERTIFICATE := platform

LOCAL_INSTRUMENTATION_FOR := NoteBook

include $(BUILD_PACKAGE)

endif
