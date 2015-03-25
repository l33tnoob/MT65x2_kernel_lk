ifneq ($(strip $(MTK_PLATFORM)),)
LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
MtkAudioBitConverter.cpp \
MtkAudioSrc.cpp \
MtkAudioLoud.cpp

LOCAL_C_INCLUDES := \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter

LOCAL_PRELINK_MODULE := false 

LOCAL_SHARED_LIBRARIES := \
    libbessound_mtk \
    libaudiocompensationfilter \
    libnvram \
    libnativehelper \
    libcutils \
    libutils \
    libblisrc32 \
    libbessound_hd_mtk \
    libmtklimiter \
    libmtkshifter
    
    
	
LOCAL_MODULE := libaudiocomponentengine

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
endif