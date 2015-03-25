# 
# Copyright 2008 The Android Open Source Project
#
# Zip alignment tool
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	CheckSum_Generate.cpp \
	FileUltilty.cpp \
	FileUnit.cpp \
	Ini_Unit.cpp

LOCAL_MODULE := CKSM_Gen
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS += -fexceptions

include $(BUILD_HOST_EXECUTABLE)

