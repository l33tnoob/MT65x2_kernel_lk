# config.mk
#
# Product-specific compile-time definitions.
#
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_VARIANT := cortex-a7
# eMMC support
ifeq ($(MTK_EMMC_SUPPORT),yes)
TARGET_USERIMAGES_USE_EXT4:=true
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := false
endif

TARGET_CPU_SMP := true
USE_CAMERA_STUB := true

TARGET_NO_FACTORYIMAGE := true

# for migrate build system
# temporarily open this two options
HAVE_HTC_AUDIO_DRIVER := true
#BOARD_USES_GENERIC_AUDIO := true 
BOARD_USES_MTK_AUDIO := true

BOARD_EGL_CFG := $(BOARD_CONFIG_DIR)/egl.cfg

BOARD_MTK_LIBSENSORS_NAME :=
BOARD_MTK_LIB_SENSOR :=

# MTK, Baochu Wang, 20101130, Add A-GPS {
ifeq ($(MTK_AGPS_APP), yes)
   BOARD_AGPS_SUPL_LIBRARIES := true
else
   BOARD_AGPS_SUPL_LIBRARIES := false
endif
# MTK, Baochu Wang, 20101130, Add A-GPS }

ifeq ($(MTK_GPS_SUPPORT), yes)
  BOARD_GPS_LIBRARIES := true
else
  BOARD_GPS_LIBRARIES := false
endif

# MTK, Infinity, 20090720, Add WiFi {
ifeq ($(MTK_WLAN_SUPPORT), yes)
BOARD_CONNECTIVITY_VENDOR := MediaTek
BOARD_CONNECTIVITY_MODULE := conn_soc

WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_mt66xx
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_mt66xx
WIFI_DRIVER_FW_PATH_PARAM:="/dev/wmtWifi"
WIFI_DRIVER_FW_PATH_STA:=STA
WIFI_DRIVER_FW_PATH_AP:=AP
WIFI_DRIVER_FW_PATH_P2P:=P2P
#HAVE_CUSTOM_WIFI_DRIVER_2 := true
#HAVE_INTERNAL_WPA_SUPPLICANT_CONF := true
#HAVE_CUSTOM_WIFI_HAL := mediatek
endif
# MTK, Infinity, 20090720, Add WiFi }

TARGET_KMODULES := true

TARGET_ARCH_VARIANT := armv7-a-neon

ifeq ($(strip $(MTK_NAND_PAGE_SIZE)), 4K)
  BOARD_NAND_PAGE_SIZE := 4096 -s 128
else
  BOARD_NAND_PAGE_SIZE := 2048 -s 64   # default 2K
endif

ifeq ($(strip $(MTK_NAND_UBIFS_SUPPORT)),yes)
TARGET_USERIMAGES_USE_UBIFS := true
endif

WITH_DEXPREOPT := false

ifeq ($(strip $(FLAVOR)),nand_mlc)
# for UBIFS
TARGET_USERIMAGES_USE_UBIFS := true

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
# for KF94G16Q4X-AEB (512MB)
#-m, --min-io-size=<bytes>
BOARD_UBIFS_MIN_IO_SIZE:=4096
#-p, --peb-size=<bytes>  size of the physical eraseblock
BOARD_FLASH_BLOCK_SIZE:=262144
#-s, --sub-page-size=<bytes>  minimum input/output unit used for UBI headers,
#e.g. sub-page size in case of NAND flash (equivalent to the minimum input/output unit size by default)
BOARD_UBIFS_SUB_PAGE_SIZE:=2048
#-O, --vid-hdr-offset=<num>   offset if the VID header from start of the physical eraseblock
#(default is the next minimum I/O unit or sub-page after the EC header)
BOARD_UBIFS_VID_HDR_OFFSET:=${BOARD_UBIFS_MIN_IO_SIZE}
#-e, --leb-size=SIZE      logical erase block size
#BOARD_FLASH_BLOCK_SIZE*1024-BOARD_UBIFS_MIN_IO_SIZE*2
BOARD_UBIFS_LOGICAL_ERASEBLOCK_SIZE:=253952 
#-c, --max-leb-cnt=COUNT  maximum logical erase block count
BOARD_UBIFS_USERDATA_MAX_LOGICAL_ERASEBLOCK_COUNT:=250
BOARD_UBIFS_SYSTEM_MAX_LOGICAL_ERASEBLOCK_COUNT:=1800
BOARD_UBIFS_SECRO_MAX_LOGICAL_ERASEBLOCK_COUNT:=22
#for $(PRODUCT_OUT)/ubi_userdata.ini and $(PRODUCT_OUT)/ubi_system.ini  and $(PRODUCT_OUT)/ubi_secro.ini
BOARD_USERDATAIMAGE_PARTITION_SIZE:=67551232
BOARD_SYSTEMIMAGE_PARTITION_SIZE:=323788800
BOARD_SECROIMAGE_PARTITION_SIZE:=4825088

#seeting for UBINIZE and MKUBIFS parameter
UBINIZE_FLAGS:=-m $(BOARD_UBIFS_MIN_IO_SIZE) -p $(BOARD_FLASH_BLOCK_SIZE) -s ${BOARD_UBIFS_SUB_PAGE_SIZE} -O $(BOARD_UBIFS_VID_HDR_OFFSET) -v
MKUBIFS_SYSTEM_FLAGS:=-m $(BOARD_UBIFS_MIN_IO_SIZE) -e ${BOARD_UBIFS_LOGICAL_ERASEBLOCK_SIZE} -c ${BOARD_UBIFS_SYSTEM_MAX_LOGICAL_ERASEBLOCK_COUNT} -v
MKUBIFS_USERDATA_FLAGS:=-m $(BOARD_UBIFS_MIN_IO_SIZE) -e ${BOARD_UBIFS_LOGICAL_ERASEBLOCK_SIZE} -c ${BOARD_UBIFS_USERDATA_MAX_LOGICAL_ERASEBLOCK_COUNT} -v
MKUBIFS_SECRO_FLAGS:=-m $(BOARD_UBIFS_MIN_IO_SIZE) -e ${BOARD_UBIFS_LOGICAL_ERASEBLOCK_SIZE} -c ${BOARD_UBIFS_SECRO_MAX_LOGICAL_ERASEBLOCK_COUNT} -v
endif #ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
endif

# include all config files
include $(BOARD_CONFIG_DIR)/configs/*.mk
