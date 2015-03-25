LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	MtkCodecService.cpp

LOCAL_SHARED_LIBRARIES := \
	libutils \
        libcutils \
	libbinder \
    libBnMtkCodec 

LOCAL_MODULE:= MtkCodecService

include $(BUILD_EXECUTABLE)

