LOCAL_PATH:= $(call my-dir)

ifneq ($(strip $(MTK_PLATFORM)),)
ifeq ($(strip $(MTK_SECURITY_SW_SUPPORT)), yes)

###############################################################################
# SEC DYNAMIC LIBRARY
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PREBUILT_LIBS += libsec.so
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# HEVC (OR SIMILIAR) DYNAMIC LIBRARY
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PREBUILT_LIBS += libmtk_cipher.so
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# SEC STATIC LIBRARY
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PREBUILT_LIBS += libsbchk.a
ifneq ($(CUSTOM_SEC_AUTH_SUPPORT),yes)
LOCAL_PREBUILT_LIBS += libauth.a
endif
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# SEC SBCHK APPLICATION
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := sbchk
LOCAL_SRC_FILES := sbchk.c
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libsbchk
ifeq ($(CUSTOM_SEC_AUTH_SUPPORT),yes)
$(call config-custom-folder,custom:security/sbchk)
LOCAL_SRC_FILES += custom/cust_auth.c
else
LOCAL_SRC_FILES += auth/sec_wrapper.c
LOCAL_STATIC_LIBRARIES += libauth
endif
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)/sbchk/
include $(BUILD_EXECUTABLE)

# for S_ANDRO_SFL.ini
include $(CLEAR_VARS)
LOCAL_MODULE := sec_chk.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)/sbchk/
include $(BUILD_PREBUILT)

###############################################################################
# SEC FILE LIST
###############################################################################
# for S_ANDRO_SFL.ini
#include $(CLEAR_VARS)
#LOCAL_MODULE := S_ANDRO_SFL.ini
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := ETC
#LOCAL_SRC_FILES := $(LOCAL_MODULE)
#LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
#include $(BUILD_PREBUILT)

#SOURCE_ANDRO_SFL := $(PRODUCT_OUT)/secro/S_ANDRO_SFL.ini
#DEST_ANDRO_SFL : = $(TARGET_OUT_ETC)/firmware/S_ANDRO_SFL.ini
#PRODUCT_COPY_FILES += $(SOURCE_ANDRO_SFL):$(DEST_ANDRO_SFL)

# for S_SECRO_SFL.ini
#include $(CLEAR_VARS)
#LOCAL_MODULE := S_SECRO_SFL.ini
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := ETC
#LOCAL_SRC_FILES := $(LOCAL_MODULE)
#LOCAL_MODULE_PATH := $(PRODUCT_OUT)/secro
#include $(BUILD_PREBUILT)

# for AC_REGION
#LOCAL_PATH := mediatek/custom/$(TARGET_PRODUCT)/secro
#include $(CLEAR_VARS)
#LOCAL_MODULE := AC_REGION
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := ETC
#LOCAL_SRC_FILES := $(LOCAL_MODULE)
#LOCAL_MODULE_PATH := $(PRODUCT_OUT)/secro
#include $(BUILD_PREBUILT)

endif
endif
