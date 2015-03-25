ifneq ($(strip $(MTK_PLATFORM)),)
LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
AudioCompensationFilter.cpp \
AudioCompFltCustParam.cpp

LOCAL_C_INCLUDES := \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram


LOCAL_PRELINK_MODULE := false 

LOCAL_SHARED_LIBRARIES := \
    libbessound_mtk \
    libnvram \
    libnativehelper \
    libcutils \
    libutils 

ifeq ($(MTK_STEREO_SPK_ACF_TUNING_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_STEREO_SPK_ACF_TUNING_SUPPORT
endif
	
	
LOCAL_MODULE := libaudiocompensationfilter

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
endif