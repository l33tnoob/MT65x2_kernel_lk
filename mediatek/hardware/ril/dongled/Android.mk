ifeq ($(MTK_3GDONGLE_SUPPORT),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    main.cpp    \
    UsbSelect.cpp \
	NetlinkManager.cpp \
	NetlinkHandler.cpp 
	

LOCAL_SHARED_LIBRARIES := \
	libsysutils \
	libcutils
	
LOCAL_C_INCLUDES := \
	$(KERNEL_HEADERS)
	

ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif # arm

LOCAL_MODULE:= dongled
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

endif #(($(MTK_3GDONGLE_SUPPORT),yes)