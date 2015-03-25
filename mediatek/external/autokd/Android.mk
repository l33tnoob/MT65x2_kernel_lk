LOCAL_PATH := $(call my-dir)
BUILD_SELF_TEST := true

ifeq ($(BUILD_SELF_TEST), true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_autok_test.cpp node_ops.cpp nodes_data.cpp param_utility.cpp autok_flow.cpp nvram_utility.cpp
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc $(MTK_PATH_CUSTOM)/hal/inc \
	external/stlport/stlport \
	bionic \
    bionic/libstdc++/include \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(MTK_PATH_CUSTOM)/cgen/cfgdefault \
    $(MTK_PATH_CUSTOM)/cgen/inc \
  
LOCAL_SHARED_LIBRARIES := libft libstlport libnvram libcutils libdl liblog
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := autokd
include $(BUILD_EXECUTABLE)
endif
