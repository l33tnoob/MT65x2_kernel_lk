MTK_CPU = arm_cortexa7

MTK_SEC_SECRO_AC_SUPPORT = yes 


TARGET_ARCH_VARIANT = armv7-a-neon


MTK_ENGINEERMODE_INTERNAL_APP = yes

MTK_FIRST_MD = 1
MTK_FOTA_ENTRY = no
MTK_FLIGHT_MODE_POWER_OFF_MD = yes

MTK_SCOMO_ENTRY = no

MTK_OGM_PLAYBACK_SUPPORT = no
MTK_LCM_PHYSICAL_ROTATION = 0

MTK_MTKPS_PLAYBACK_SUPPORT = no
MTK_CHKIMGSIZE_SUPPORT = yes
MTK_MMPROFILE_SUPPORT=yes

# To enable HWC
MTK_HWC_SUPPORT = yes
MTK_HWC_VERSION = 1.2

MTK_DP_FRAMEWORK = no

MTK_ION_SUPPORT = yes
MTK_YAML_SCATTER_FILE_SUPPORT = yes
MTK_ENABLE_MD1 = yes
MTK_ENABLE_MD2 = no

MTK_FD_SUPPORT=yes
MTK_FD_FORCE_REL_SUPPORT=yes

CUSTOM_KERNEL_SSW=ssw_single

MTK_SWIP_VORBIS=yes

MULTI_CH_PLAYBACK_SUPPORT=no


MTK_PRODUCT_INFO_SUPPORT=yes


NATIVE_AUDIO_PREPROCESS_ENABLE=yes

MTK_NATIVE_FENCE_SUPPORT = yes

MTK_WAIT_SYNC_SUPPORT = no 

MTK_USES_VR_DYNAMIC_QUALITY_MECHANISM=yes
MTK_PQ_SUPPORT=PQ_HW_VER_2    
MTK_FM_RX_AUDIO=FM_DIGITAL_INPUT

MTK_BWC_SUPPORT = yes



MTK_LCEEFT_SUPPORT = yes 

MTK_WFD_SUPPORT=yes




MTK_AUDIO_BLOUD_CUSTOMPARAMETER_REV = MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4

FEATURE_FTM_AUDIO_AUTOTEST = yes



# Capability of the underlay modem
#  single or gemini
MTK_SHARE_MODEM_SUPPORT=2

MTK_MD_SHUT_DOWN_NT = yes 

MTK_MASS_STORAGE=yes

# common part
MTK_COMBO_SUPPORT=yes
CUSTOM_HAL_COMBO=mt6572_82
CUSTOM_HAL_ANT=mt6582_ant_m1
MTK_COMBO_CHIP=CONSYS_6582
MTK_COMBO_CORE_DUMP_SUPPORT=no
MTK_COMBO_QUICK_SLEEP_SUPPORT=no

MTK_WLAN_SUPPORT=yes
MTK_WLAN_CHIP = CONSYS_MT6582

#
# MTK Chip
#
MTK_BT_CHIP = MTK_CONSYS_MT6582

# When this option set to yes, the Bluetooth stack will comply to Bluetooth Sepc 3.0 (no High Speed).
MTK_BT_30_HS_SUPPORT=no

# When this option set to yes, the Bluetooth stack will comply to Bluetooth Sepc 4.0 (Low Energy).
MTK_BT_40_SUPPORT=yes

MTK_BT_PROFILE_AVRCP13 = no
MTK_BT_PROFILE_AVRCP14= no

# When this option set to yes, the Bluetooth "SIM Access Profile" (SIMAP) will be enabled.
MTK_BT_PROFILE_SIMAP=no

# When this option set to yes, the Bluetooth Low Energy "Proximity Profile - Monitor Role" (PRXM) will be enabled.
MTK_BT_PROFILE_PRXM=no

# When this option set to yes, the Bluetooth Low Energy "Proximity Profile - Reporter Role" (PRXR) will be enabled.
MTK_BT_PROFILE_PRXR=no

# When this option set to yes, the Bluetooth "File Transfer Profile" (FTP) will be enabled.
MTK_BT_PROFILE_FTP=no

# When this option set to yes, the Bluetooth "Basic Printing Profile" (BPP) will be enabled.
MTK_BT_PROFILE_BPP=no

# When this option set to yes, the Bluetooth "Basic Imaging Profile" (BIP) will be enabled.
MTK_BT_PROFILE_BIP=no

# When this option set to yes, the Bluetooth "Dial-Up Networking Profile" (DUN) will be enabled.
MTK_BT_PROFILE_DUN=no

# When this option set to yes, the Bluetooth "Message Access Profile - Server Role" (MAPS) will be enabled.
MTK_BT_PROFILE_MAPS=no

# When this option set to yes, the Bluetooth "Message Access Profile - Client Role" (MAPC) will be enabled.
MTK_BT_PROFILE_MAPC=no

MTK_GPS_SUPPORT=yes

MTK_YGPS_APP=yes

MTK_GPS_CHIP=MTK_GPS_MT6582

MTK_UART_USB_SWITCH = yes

MTK_AUDENH_SUPPORT = yes 

# display framework impl
MTK_DP_FRAMEWORK = yes

MTK_HANDSFREE_DMNR_SUPPORT = yes 




MTK_SW_BTCVSD = yes 

MTK_VT3G324M_SUPPORT= yes

MTK_VOICE_UI_SUPPORT = yes

MTK_TINY_UTIL = yes 


MTK_KERNEL_POWER_OFF_CHARGING = yes 


MTK_DMNR_TUNING_AT_MD = no 


MTK_VIDEO_THUMBNAIL_PLAY_SUPPORT = yes

MTK_VIDEO_VP9_SUPPORT = yes

MTK_PRODUCT_AAPT_CONFIG = hdpi 

MTK_PLATFORM_OPTIMIZE = yes  

MTK_CTP_RESET_CONFIG = yes

MTK_ENABLE_DIVX_DECODER = yes 


MTK_ENABLE_S263_DECODER = yes 
LINUX_KERNEL_VERSION = kernel-3.4

MTK_MOBILE_MANAGEMENT = yes 

MTK_PERMISSION_CONTROL = yes
MTK_LIVE_PHOTO_SUPPORT=yes

MTK_MOTION_TRACK_SUPPORT=yes
LEGACY_DFO_GEN=yes
