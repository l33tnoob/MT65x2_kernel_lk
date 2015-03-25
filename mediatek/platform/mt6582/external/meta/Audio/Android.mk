LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := AudioMeta.cpp

PLATFORM_PATH := $(MTK_PATH_PLATFORM)/external/meta

LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc \
                    $(PLATFORM_PATH)/include \
                    $(PLATFORM_PATH)/meta_lock \
                    $(PLATFORM_PATH)/Meta_APEditor \
                    $(PLATFORM_PATH)/lcd \
                    $(PLATFORM_PATH)/LCDBK \
                    $(PLATFORM_PATH)/cpu \
                    $(PLATFORM_PATH)/sdcard \
                    $(PLATFORM_PATH)/gpio \
                    $(PLATFORM_PATH)/keypadbk \
                    $(PLATFORM_PATH)/ADC \
                    $(PLATFORM_PATH)/BatteryIC \
                    $(PLATFORM_PATH)/pmic \
                    $(PLATFORM_PATH)/vibrator \
                    $(PLATFORM_PATH)/msensor \
                    $(PLATFORM_PATH)/alsps \
                    $(PLATFORM_PATH)/gsensor \
                    $(PLATFORM_PATH)/gyroscope \
                    $(PLATFORM_PATH)/touch \
                    $(PLATFORM_PATH)/cameratool/CCAP \
                    $(MTK_PATH_SOURCE)/platform/mt6589/hardware/camera/inc/acdk \
                    $(MTK_PATH_SOURCE)/platform/mt6589/hardware/camera/acdk/inc \
                    $(MTK_PATH_SOURCE)/platform/mt6589/hardware/camera/acdk/inc/acdk \
                    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
                    $(MTK_PATH_SOURCE)/external/mhal/inc \
                    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
                    $(MTK_PATH_CUSTOM)/hal/inc \
                    $(PLATFORM_PATH)/Audio 
					
#FTM include
LOCAL_C_INCLUDES += ./$(TOPDIR)/mediatek/frameworks-ext/av \
					$(TOPDIR)/hardware/libhardware_legacy/include \
					$(TOPDIR)/frameworks/av/include \
					$(call include-path-for, audio-utils) \
					$(call include-path-for, audio-effects) \
					$(MTK_PATH_SOURCE)/platform/common/hardware/audio/include \
					$(MTK_PATH_SOURCE)/platform/common/hardware/audio/V2/include \
					$(MTK_PATH_PLATFORM)/hardware/audio/include \
					$(MTK_PATH_SOURCE)/external/nvram/libnvram \
					$(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
                    $(MTK_PATH_SOURCE)/external/AudioComponentEngine \
 			        $(MTK_PATH_SOURCE)/external/cvsd_plc_codec \
 			        $(MTK_PATH_SOURCE)/external/msbc_codec \
					$(MTK_PATH_SOURCE)/frameworks/av/include/media \
					$(MTK_PATH_SOURCE)/frameworks-ext/av/include/media \
					$(MTK_PATH_SOURCE)/frameworks-ext/av/include \
					$(MTK_PATH_SOURCE)/frameworks/av/include \
					$(MTK_PATH_SOURCE)/external/audiodcremoveflt \
					$(MTK_PATH_SOURCE)/external/audiocustparam \
					$(MTK_PATH_SOURCE)/external/AudioSpeechEnhancement/inc \
					$(MTK_PATH_SOURCE)/external/fft \
					$(MTK_PATH_SOURCE)/kernel/include \
					$(LOCAL_PATH)/custom \
					$(LOCAL_PATH)/custom/audio
					

LOCAL_SHARED_LIBRARIES := libft

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaed
    LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/external/aee/binary/inc
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

LOCAL_MODULE := libmeta_audio
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)

###############################################################################
# SELF TEST
###############################################################################
BUILD_SELF_TEST := false
# BUILD_SELF_TEST := true
ifeq ($(BUILD_SELF_TEST), true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := AudioMeta.cpp
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc
LOCAL_MODULE := meta_battery_test
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_SHARED_LIBRARIES := libmeta_battery libft

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaed
    LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/external/aee/binary/inc
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
include $(BUILD_EXECUTABLE)
endif
