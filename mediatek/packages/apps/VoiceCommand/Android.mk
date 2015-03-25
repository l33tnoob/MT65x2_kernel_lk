LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := VoiceCommand
#LOCAL_CERTIFICATE := media

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_EMMA_COVERAGE_FILTER := +com.mediatek.voicecommand.adapter.*\
                              +com.mediatek.voicecommand.business.*\
                              +com.mediatek.voicecommand.cfg.*\
                              +com.mediatek.voicecommand.data.*\
                              +com.mediatek.voicecommand.mgr.*\
                              +com.mediatek.voicecommand.service.*\
                              +com.mediatek.voicecommand.voicesettings.*

#EMMA_INSTRUMENT := true

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))