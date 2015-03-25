LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_matv.cpp 

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libbinder \
	liblog

ifeq ($(HAVE_MATV_FEATURE),yes)
  LOCAL_CFLAGS += -DMTK_MATV_SUPPORT
  LOCAL_SHARED_LIBRARIES += libatvctrlservice
  LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/media/libs  \
  $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/include \
  $(TOP)/$(MTK_PATH_SOURCE)/external/matvctrl        
endif
LOCAL_MODULE:= matv

include $(BUILD_EXECUTABLE)
