LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := libcutils libnvram
# LOCAL_STATIC_LIBRARIES := libiw
# LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := libwifitest.c iwlib.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(MTK_PATH_CUSTOM)/cgen/cfgdefault
LOCAL_CFLAGS += -Wall -Werror
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libwifitest
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

BUILD_TEST_APP = false
ifeq ($(BUILD_TEST_APP),true)
  include $(CLEAR_VARS)
  LOCAL_SHARED_LIBRARIES := libnvram libcutils libwifitest
# LOCAL_STATIC_LIBRARIES := libiw
  LOCAL_MODULE_TAGS := eng
  LOCAL_SRC_FILES := main.c
  LOCAL_CFLAGS += -Wall -Werror
  LOCAL_MODULE := wifitest
  LOCAL_MODULE_TAGS := optional

  include $(BUILD_EXECUTABLE)
endif
