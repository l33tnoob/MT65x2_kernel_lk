LOCAL_PATH:= $(call my-dir)

libmtksqlite3_custom_local_src_files := \
	sqlite3_android_custom.cpp

libmtksqlite3_custom_c_includes := external/sqlite/dist


include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(libmtksqlite3_custom_local_src_files)
LOCAL_C_INCLUDES := $(libmtksqlite3_custom_c_includes)

ifeq ($(OPTR_SPEC_SEG_DEF),OP01_SPEC0200_SEGC)
LOCAL_CFLAGS += -D CONFIG_CMCC_SUPPORT
endif

LOCAL_MODULE:= libmtksqlite3_custom
include $(BUILD_STATIC_LIBRARY)
