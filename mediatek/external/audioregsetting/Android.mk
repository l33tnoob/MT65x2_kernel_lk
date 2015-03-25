ifneq ($(strip $(MTK_PLATFORM)),)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES:=       \
	audioregsetting.cpp
	
LOCAL_C_INCLUDES=       \
       $(MTK_PATH_SOURCE)/frameworks/av/include/media \
       $(MTK_PATH_SOURCE)/frameworks-ext/av/include/media

LOCAL_SHARED_LIBRARIES := libcutils libutils libbinder libmedia libaudioflinger

LOCAL_MODULE:= audioregsetting

include $(BUILD_EXECUTABLE)

endif
