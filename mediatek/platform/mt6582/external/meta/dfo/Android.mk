LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES:= libc libft libutils libnvram
LOCAL_SRC_FILES:=meta_dfo.c
LOCAL_C_INCLUDES:= \
	$(MTK_PATH_SOURCE)/external/meta/common/inc \
	$(MTK_PATH_SOURCE)/external/dfo/boot \
	$(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
	$(MTK_PATH_CUSTOM)/cgen/inc \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
	$(TARGET_OUT_HEADERS)/dfo

ifneq ($(ALWAYSON_DFOSET), yes)
LOCAL_CFLAGS +=-DNOT_SUPPORT_DFO
endif

LOCAL_MODULE:=libmeta_dfo
LOCAL_PRELINK_MODULE:=false

include $(BUILD_STATIC_LIBRARY)

###############################################################################
# SELF TEST
###############################################################################
ifeq ($(META_DFO_TEST), yes)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := libc libft libutils libnvram
LOCAL_STATIC_LIBRARIES := libmeta_dfo
LOCAL_SRC_FILES := meta_dfo_test.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := meta_dfo_test

include $(BUILD_EXECUTABLE)

endif

