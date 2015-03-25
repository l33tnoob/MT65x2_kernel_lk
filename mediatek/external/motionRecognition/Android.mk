ifeq ($(strip $(MTK_VOICE_UI_SUPPORT)),yes)
LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PREBUILT_LIBS := libmotionrecognition.so
include $(BUILD_MULTI_PREBUILT)
endif