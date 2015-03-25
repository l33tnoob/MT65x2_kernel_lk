LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	fence.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libui \
	libgui \
	libsync

LOCAL_MODULE:= test-fence

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
