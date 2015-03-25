#
# ipo daemon
#

ifneq (,$(filter yes, $(MTK_IPO_SUPPORT) $(MTK_KERNEL_POWER_OFF_CHARGING)))

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= ipodmain.cpp \
                  ipodcommon.cpp \
                  ipodlights.cpp \
                  bootlogo.cpp 


ifeq ($(MTK_TB_WIFI_3G_MODE), WIFI_ONLY)
LOCAL_CFLAGS += -DMTK_TB_WIFI_ONLY
endif

ifeq ($(strip $(MTK_IPO_MDRST_SUPPORT)),yes)
#$(warning reset_modem = 1)
	LOCAL_CFLAGS += -DMTK_RESET_MODEM=1
else
#$(warning reset_modem = 0)
	LOCAL_CFLAGS += -DMTK_RESET_MODEM=0
endif

ifeq ($(MTK_KERNEL_POWER_OFF_CHARGING), yes)
LOCAL_CFLAGS += -DMTK_KERNEL_POWER_OFF_CHARGING
endif

ifeq ($(MTK_IPOH_SUPPORT), yes)
LOCAL_CFLAGS += -DMTK_IPOH_SUPPORT
endif

ifneq ($(MTK_TABLET_HARDWARE), )
LOCAL_CFLAGS += -DMTK_TABLET_HARDWARE
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libshowlogo/                  
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/lk/include/target
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/kernel/dct/
LOCAL_C_INCLUDES += $(MTK_PATH_PLATFORM)/lk/include/target
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(TOP)/external/zlib/


LOCAL_MODULE:= ipod

LOCAL_SHARED_LIBRARIES := libcutils libutils libc libstdc++ libz libdl liblog libgui libui libshowlogo


LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)

endif
