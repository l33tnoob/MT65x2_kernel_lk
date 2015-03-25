ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	test_program.c

LOCAL_STATIC_LIBRARIES := liburee_meta_drmkeyinstall

LOCAL_SHARED_LIBRARIES := libcutils

LOCAL_C_INCLUDES += \
    $(call include-path-for, trustzone) \
    $(call include-path-for, trustzone-uree) \

LOCAL_MODULE := drmkey_test

include $(BUILD_EXECUTABLE)

endif
endif
