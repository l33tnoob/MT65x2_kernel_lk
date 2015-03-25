LOCAL_PATH := $(call my-dir)

$(call config-custom-folder,custom:trustzone)

include mediatek/trustzone/mtk/Setting.mk #this is common setting
include $(LOCAL_PATH)/custom/custom.mk #this is custom setting

include $(CLEAR_VARS)

# For TEE system/platform
TZ_SYSTEM_LIBRARIES += libtz_sys libtz_mtee_sys_$(call lc,$(MTK_PLATFORM))

# Platform library, mtee part & bsp part
TZ_SYSTEM_LIBRARIES += libtz_bsp_platform_$(call lc,$(MTK_PLATFORM)) libtz_platform_$(call lc,$(MTK_PLATFORM))

# Put TA libraries here.
TZ_TA_LIBRARIES += libtz_mtee_app_pm_$(call lc,$(MTK_PLATFORM)) libtz_mtee_app_irq
TZ_TA_LIBRARIES += libtz_app_test
TZ_TA_LIBRARIES += libtz_mtee_app_mem
TZ_TA_LIBRARIES += libtz_gcpu
TZ_TA_LIBRARIES += libtz_app_dbg
ifneq ($(call lc,$(MTK_PLATFORM)),mt8127)
TZ_TA_LIBRARIES += libtz_m4u
endif

ifeq ($(MTK_SEC_VIDEO_PATH_SUPPORT), yes)
TZ_TA_LIBRARIES += libtz_app_ddp
TZ_TA_LIBRARIES += libtz_ddp
endif

ifeq ($(MTK_WFD_HDCP_TX_SUPPORT), yes)
TZ_TA_LIBRARIES += libtz_app_hdcp2_tx
endif

ifeq ($(MTK_WVDRM_L1_SUPPORT), yes)
TZ_TA_LIBRARIES += libtz_app_widevine
endif

ifeq ($(MTK_SEC_VIDEO_PATH_SUPPORT), yes)
TZ_TA_LIBRARIES += libtz_app_vdec
endif

ifeq ($(strip $(MTK_SEC_WFD_VIDEO_PATH_SUPPORT)),yes)
TZ_TA_LIBRARIES += libtz_app_venc
endif

TZ_TA_LIBRARIES += libtz_crypto
TZ_TA_LIBRARIES += libtz_drv_dapc
TZ_TA_LIBRARIES += libtz_devinfo
TZ_TA_LIBRARIES += libtz_drv_icnt
ifneq ($(call lc,$(MTK_PLATFORM)),mt8127)
TZ_TA_LIBRARIES += libtz_drv_emi
endif

ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)
TZ_TA_LIBRARIES += libtz_meta_drmkeyinstall
endif
endif

ifeq ($(MTK_AIV_SUPPORT),yes)
ifeq ($(MTK_DRM_PLAYREADY_SUPPORT),yes)
TZ_TA_LIBRARIES += libtz_app_playready
endif
endif

# Platform dependent TA
TZ_TA_LIBRARIES_PLATFORM_DEP += libtz_$(call lc,$(MTK_PLATFORM))_mtee_img_prot_inf_gen
TZ_TA_LIBRARIES_PLATFORM_DEP += libtz_$(call lc,$(MTK_PLATFORM))_dev_prot
TZ_TA_LIBRARIES_PLATFORM_DEP += libtz_$(call lc,$(MTK_PLATFORM))_drv_dcm
TZ_TA_LIBRARIES_PLATFORM_DEP += libtz_$(call lc,$(MTK_PLATFORM))_drv_trng

ifeq ($(filter $(MTK_INTERNAL_MHL_SUPPORT) $(MTK_INTERNAL_HDMI_SUPPORT),yes),yes)
TZ_TA_LIBRARIES_PLATFORM_DEP += libtz_bsp_$(call lc,$(MTK_PLATFORM))_hdmi_mhl
endif

# Library/utility (ex, openssl/libcrypt)
TZ_UTILITY_LIBRARIES += libtommath_tz libtomcrypt_tz  libtz_ndbg_tz

# libc library name
TZ_C_LIBRARIES += libc_tz libm_tz

LOCAL_MODULE := tz.img
#LOCAL_CFLAGS += ${TZ_CFLAG}
LOCAL_LDFLAGS += --gc-sections
#LOCAL_ASFLAGS += -DASSEMBLY
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_STATIC_LIBRARIES += $(TZ_SYSTEM_LIBRARIES) $(TZ_TA_LIBRARIES) $(TZ_TA_LIBRARIES_PLATFORM_DEP) $(TZ_UTILITY_LIBRARIES) $(TZ_C_LIBRARIES)
PRIVATE_LINK_SCRIPT := $(LOCAL_PATH)/system-onesegment.ld
LOCAL_TRUSTZONE_BIN := true
include $(BUILD_RAW_EXECUTABLE)
droid: $(LOCAL_BUILT_MODULE)
include $(call all-makefiles-under,$(LOCAL_PATH))
