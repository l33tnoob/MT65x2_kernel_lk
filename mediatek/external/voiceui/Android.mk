ifeq ($(strip $(MTK_VOICE_UI_SUPPORT)),yes)
LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libvoiceui.a
include $(BUILD_MULTI_PREBUILT)
endif