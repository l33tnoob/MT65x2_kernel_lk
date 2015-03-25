#Android forbiden developer use product level variables(PRODUCT_XXX, TARGET_XXX, BOARD_XXX) in Android.mk
#Because AndroidBoard.mk include by build/target/board/Android.mk
#split from AndroidBoard.mk for PRODUCT level variables definition.

#use MTK_ROOT_CONFIG_OUT instead of MTK_ROOT_CONFIG_OUT
TARGET_PROVIDES_INIT_RC := true
PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk:system/data/misc/ProjectConfig.mk

PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/mtk-kpd.kl:system/usr/keylayout/mtk-kpd.kl \
                      $(MTK_ROOT_CONFIG_OUT)/init.rc:root/init.rc \
                      $(MTK_ROOT_CONFIG_OUT)/init.fon.rc:root/init.fon.rc \
                      $(MTK_ROOT_CONFIG_OUT)/init.modem.rc:root/init.modem.rc \
                      $(MTK_ROOT_CONFIG_OUT)/init.usb.rc:root/init.usb.rc \
                      $(MTK_ROOT_CONFIG_OUT)/init.xlog.rc:root/init.xlog.rc \
                      $(MTK_ROOT_CONFIG_OUT)/player.cfg:system/etc/player.cfg \
                      $(MTK_ROOT_CONFIG_OUT)/media_codecs.xml:system/etc/media_codecs.xml \
                      $(MTK_ROOT_CONFIG_OUT)/mtk_omx_core.cfg:system/etc/mtk_omx_core.cfg \
                      $(MTK_ROOT_CONFIG_OUT)/meta_init.rc:root/meta_init.rc \
                      $(MTK_ROOT_CONFIG_OUT)/meta_init.modem.rc:root/meta_init.modem.rc \
                      $(MTK_ROOT_CONFIG_OUT)/factory_init.rc:root/factory_init.rc \
                      $(MTK_ROOT_CONFIG_OUT)/audio_policy.conf:system/etc/audio_policy.conf \
                      $(MTK_ROOT_CONFIG_OUT)/init.protect.rc:root/init.protect.rc \
                      $(MTK_ROOT_CONFIG_OUT)/ACCDET.kl:system/usr/keylayout/ACCDET.kl \
                      $(MTK_ROOT_CONFIG_OUT)/partition_permission.sh:system/etc/partition_permission.sh \
                      $(MTK_ROOT_CONFIG_OUT)/fstab:root/fstab \
                      $(MTK_ROOT_CONFIG_OUT)/fstab.nand:root/fstab.nand \
                      $(MTK_ROOT_CONFIG_OUT)/fstab.fat.nand:root/fstab.fat.nand \
                      $(MTK_ROOT_CONFIG_OUT)/enableswap.sh:root/enableswap.sh



ifeq ($(MTK_KERNEL_POWER_OFF_CHARGING),yes)
PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/init.charging.rc:root/init.charging.rc 
endif

_init_project_rc := $(MTK_ROOT_CONFIG_OUT)/init.project.rc
ifneq ($(wildcard $(_init_project_rc)),)
PRODUCT_COPY_FILES += $(_init_project_rc):root/init.project.rc
endif

_meta_init_project_rc := $(MTK_ROOT_CONFIG_OUT)/meta_init.project.rc
ifneq ($(wildcard $(_meta_init_project_rc)),)
PRODUCT_COPY_FILES += $(_meta_init_project_rc):root/meta_init.project.rc
endif


_factory_init_project_rc := $(MTK_ROOT_CONFIG_OUT)/factory_init.project.rc
ifneq ($(wildcard $(_factory_init_project_rc)),)
PRODUCT_COPY_FILES += $(_factory_init_project_rc):root/factory_init.project.rc
endif

PRODUCT_COPY_FILES += $(strip \
                        $(foreach file,$(wildcard $(MTK_ROOT_CONFIG_OUT)/*.xml), \
                          $(addprefix $(MTK_ROOT_CONFIG_OUT)/$(notdir $(file)):system/etc/permissions/,$(notdir $(file))) \
                         ) \
                       )



ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
  PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/srs_processing.cfg:system/data/srs_processing.cfg
endif

ifeq ($(MTK_SHARED_SDCARD),yes)
ifeq ($(MTK_2SDCARD_SWAP),yes)
  PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/init.ssd_nomuser.rc:root/init.ssd_nomuser.rc
else
  PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/init.ssd.rc:root/init.ssd.rc
endif
else
  PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/init.no_ssd.rc:root/init.no_ssd.rc
endif
#### INSTALL ht120.mtc ##########

_ht120_mtc := $(MTK_ROOT_CONFIG_OUT)/configs/ht120.mtc
ifneq ($(wildcard $(_ht120_mtc)),)
PRODUCT_COPY_FILES += $(_ht120_mtc):system/etc/.tp/.ht120.mtc
endif

##################################

##### INSTALL thermal.conf ##########

_thermal_conf := $(MTK_ROOT_CONFIG_OUT)/configs/thermal.conf
ifneq ($(wildcard $(_thermal_conf)),)
PRODUCT_COPY_FILES += $(_thermal_conf):system/etc/.tp/thermal.conf
endif

##################################

##### INSTALL thermal.off.conf ##########

_thermal_off_conf := $(MTK_ROOT_CONFIG_OUT)/configs/thermal.off.conf
ifneq ($(wildcard $(_thermal_off_conf)),)
PRODUCT_COPY_FILES += $(_thermal_off_conf):system/etc/.tp/thermal.off.conf
endif

##################################

##### INSTALL throttle.sh ##########

_throttle_sh := $(MTK_ROOT_CONFIG_OUT)/configs/throttle.sh
ifneq ($(wildcard $(_throttle_sh)),)
PRODUCT_COPY_FILES += $(_throttle_sh):system/etc/throttle.sh
endif

##################################

_notimeout := $(MTK_ROOT_CONFIG_OUT)/configs/notimeout.sh
ifneq ($(wildcard $(_notimeout)),)
PRODUCT_COPY_FILES += $(_notimeout):system/etc/notimeout.sh
endif

