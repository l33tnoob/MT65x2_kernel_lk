
ifeq ($(MTK_WLAN_SUPPORT),yes)


ifdef WIFI_DRIVER_MODULE_PATH
LOCAL_CFLAGS += -DWIFI_DRIVER_MODULE_PATH=\"$(WIFI_DRIVER_MODULE_PATH)\"
endif

$(warning check_mt6620)
SUPPORT_MT6620 := $(if $(filter $(MTK_WLAN_CHIP), MT6620),yes,no)
$(warning $(SUPPORT_MT6620))
$(warning check_mt6620_end)

$(warning check_mt6628)
SUPPORT_MT6628 := $(if $(filter $(MTK_WLAN_CHIP), MT6628),yes,no)
$(warning $(SUPPORT_MT6628))
$(warning check_mt6628_end)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)


ifeq ($(SUPPORT_MT6620),yes)
    LOCAL_CFLAGS += -DSUPPORT_MT6620
    $(warning include mt6620)
endif    

ifeq ($(SUPPORT_MT6628),yes)
    LOCAL_CFLAGS += -DSUPPORT_MT6628
    $(warning include mt6628)
endif    

LOCAL_SHARED_LIBRARIES := libsysutils libcutils

LOCAL_SRC_FILES:= loader.c

LOCAL_MODULE:= wlan_loader

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

endif
