LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# We only want this apk build for tests.
LOCAL_MODULE_TAGS := tests

# Build all java files in the java subdirectory
LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := android.test.runner

# Name of the APK to build
LOCAL_PACKAGE_NAME := EmptyScreen

# Tell it to build an APK 
include $(BUILD_PACKAGE)
