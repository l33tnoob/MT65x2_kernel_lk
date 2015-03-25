LOCAL_PATH:= $(call my-dir)

UNIT_TEST=no

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
  icusb_util.c \
  icusb_ccci.c \
  icusb_ccid.c \
  icusb_storage.c

ifeq ($(UNIT_TEST),yes)
  LOCAL_CFLAGS += -DICUSB_UT_NO_CCCI=1
  #LOCAL_CFLAGS += -DICUSB_UT_NO_USB=1

  LOCAL_SRC_FILES += icusb_main.c 
  LOCAL_MODULE := icusbd_ut
else
  LOCAL_SRC_FILES += icusb_main.c 
  LOCAL_MODULE := icusbd
endif

#LOCAL_MODULE_TAGS = eng 
LOCAL_PRELINK_MODULE := false
LOCAL_C_INCLUDES += \
 $(TOP)/external/libusb/ \
 $(LOCAL_PATH)/ \
 $(TOPDIR)/hardware/libhardware_legacy/include \
 $(TOPDIR)/hardware/libhardware/include

LOCAL_SHARED_LIBRARIES += libc libusb libcutils
LOCAL_STATIC_LIBRARIES += libc libcutils
ifeq ($(MTK_ICUSB_SUPPORT),yes)
    include $(BUILD_EXECUTABLE)
endif
