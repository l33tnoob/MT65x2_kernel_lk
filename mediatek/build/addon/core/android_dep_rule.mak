ifneq ($(strip $(CUSTOM_MODEM)),)
MTK_MODEM_PATH := $(foreach p,$(CUSTOM_MODEM),mediatek/custom/common/modem/$(p))
MTK_MODEM_FILE := $(foreach p,$(MTK_MODEM_PATH),$(wildcard $(p)/modem*.mak))
# note: this file is include by $(eval include xxx) in makemtk.mk
$(foreach m,$(CUSTOM_MODEM),\
	$(eval s := $(filter-out mediatek/custom/common/modem/$(m)/modem_feature_%.mak,$(wildcard mediatek/custom/common/modem/$(m)/modem*.mak)))\
	$(eval d := $(MTK_ROOT_CUSTOM_OUT)/modem/$(m)/modem_feature_$(m).mak)\
 	$(shell mkdir -p $(MTK_ROOT_CUSTOM_OUT)/modem/$(m)) \
	$(if $(s),\
		$(shell cat $(s) | sed -e '/^[ \t]*#/'d | awk -F# '{print $1}' | sed -n '/^\S\+[ \t]*=[ \t]*\S\+/'p | sed -e 's/^/MODEM_/' > $(d).tmp)\
		$(shell if [ -r $(d) ] && cmp -s $(d) $(d).tmp; then rm -f $(d).tmp; else mv -f $(d).tmp $(d); fi)\
		$(eval include $(d))\
	,\
		$(warning *** Invalid CUSTOM_MODEM = $(CUSTOM_MODEM))\
		$(warning Cannot find mediatek/custom/common/modem/$(m)/modem*.mak)\
	)\
)
endif

#######################################################
# dependency check between AP side & modem side

ifeq (yes,$(strip $(MTK_TTY_SUPPORT)))
   ifeq (FALSE, $(strip $(MODEM_CTM_SUPPORT)))
    $(call dep-err-seta-or-setb,MTK_TTY_SUPPORT,no,CTM_SUPPORT,true)
   endif
endif

ifneq (yes,$(strip $(MTK_TLR_SUPPORT)))
    ifeq (yes,$(strip $(MTK_VT3G324M_SUPPORT)))
        ifeq (FALSE,$(MODEM_SP_VIDEO_CALL_SUPPORT))
            $(call dep-err-ona-or-offb,SP_VIDEO_CALL_SUPPORT,MTK_VT3G324M_SUPPORT)
        endif
    endif
endif

ifeq (yes,$(strip $(MTK_VT3G324M_SUPPORT)))
  ifeq ($(findstring _3g,$(MTK_MODEM_SUPPORT) $(MTK_MD2_SUPPORT)),)
#     $(call dep-err-common, please turn off MTK_VT3G324M_SUPPORT or set MTK_MODEM_SUPPORT/MTK_MD2_SUPPORT as modem_3g_tdd/modem_3g_fdd)
  endif
endif

ifneq ($(findstring modem_3g,$(MTK_MODEM_SUPPORT)),)
  ifeq ($(findstring hspa,$(CUSTOM_MODEM)),)
#    $(call dep-err-seta-or-setb,CUSTOM_MODEM,xxx_hspa_xxx,MTK_MODEM_SUPPORT,none 3g)
  endif
endif

ifeq (yes,$(strip $(MTK_ENABLE_MD2)))
  ifeq (,$(strip $(MTK_MD2_SUPPORT)))
#     $(call dep-err-common,please set MTK_MD2_SUPPORT when MTK_ENABLE_MD2 is enabled)
  endif
endif

# temp fpr MT6573
ifeq (MT6573,$(strip $(MTK_PLATFORM)))
  ifeq (UMTS_FDD_MODE_SUPPORT,$(strip $(MODEM_UMTS_MODE_SUPPORT)))
    ifneq (modem_3g_fdd,$(strip $(MTK_MODEM_SUPPORT)))
      $(call dep-err-common,please set MTK_MODEM_SUPPORT=modem_3g_fdd when UMTS_MODE_SUPPORT=$(UMTS_MODE_SUPPORT))
    endif
  endif

  ifeq (UMTS_TDD128_MODE_SUPPORT,$(strip $(MODEM_UMTS_MODE_SUPPORT)))
    ifneq (modem_3g_tdd,$(strip $(MTK_MODEM_SUPPORT)))
      $(call dep-err-common,please set MTK_MODEM_SUPPORT=modem_3g_tdd when UMTS_MODE_SUPPORT=$(UMTS_MODE_SUPPORT))
    endif
  endif
endif

ifeq (MT6572,$(strip $(MTK_PLATFORM)))
  ifneq (yes,$(strip $(MTK_EMMC_SUPPORT)))
    ifneq (yes,$(strip $(MTK_CACHE_MERGE_SUPPORT)))
      $(call dep-err-ona-or-onb,MTK_CACHE_MERGE_SUPPORT,MTK_EMMC_SUPPORT)
    endif
  endif
endif
###############################################################
ifeq (yes, $(strip $(MTK_FLIGHT_MODE_POWER_OFF_MD)))
  ifneq (yes, $(strip $(MTK_MD_SHUT_DOWN_NT)))
    $(call dep-err-ona-or-offb, MTK_MD_SHUT_DOWN_NT, MTK_FLIGHT_MODE_POWER_OFF_MD)
  endif
endif
###########################################################
ifeq (yes,$(strip $(MTK_DIALER_SEARCH_SUPPORT)))
  ifneq (yes, $(strip $(MTK_SEARCH_DB_SUPPORT)))
     $(call dep-err-ona-or-offb,MTK_SEARCH_DB_SUPPORT,MTK_DIALER_SEARCH_SUPPORT)
  endif
endif
############################################################
# for wapi feature

ifeq (yes,$(strip $(MTK_WAPI_SUPPORT)))
  ifneq (yes, $(strip $(MTK_WLAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_WLAN_SUPPORT, MTK_WAPI_SUPPORT)
  endif
endif

ifeq (yes,$(strip $(MTK_CTA_SUPPORT)))
  ifneq (yes, $(strip $(MTK_WAPI_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_WAPI_SUPPORT, MTK_CTA_SUPPORT)
  endif
endif
###########################################################
#for lca ram & rom 
ifeq (yes,$(strip $(MTK_LCA_ROM_OPTIMIZE)))
  ifeq ( ,$(filter LCA_rom,$(RESOURCE_OVERLAY_SUPPORT)))
     $(call dep-err-common, Please add value LCA_rom to RESOURCE_OVERLAY_SUPPORT or turn off MTK_LCA_ROM_OPTIMIZE)
  endif
endif
ifneq (yes,$(strip $(MTK_LCA_ROM_OPTIMIZE)))
  ifneq ( ,$(filter LCA_rom,$(RESOURCE_OVERLAY_SUPPORT)))
     $(call dep-err-common, Please remove LCA_rom from RESOURCE_OVERLAY_SUPPORT or turn on MTK_LCA_ROM_OPTIMIZE)
  endif
endif
ifeq (yes,$(strip $(MTK_LCA_RAM_OPTIMIZE)))
  ifeq ( ,$(filter LCA_ram,$(RESOURCE_OVERLAY_SUPPORT)))
    $(call dep-err-common, Please add value LCA_ram to RESOURCE_OVERLAY_SUPPORT or turn off MTK_LCA_RAM_OPTIMIZE)
  endif
endif
ifneq (yes,$(strip $(MTK_LCA_RAM_OPTIMIZE)))
  ifneq ( ,$(filter LCA_ram,$(RESOURCE_OVERLAY_SUPPORT)))
    $(call dep-err-common, Please remove LCA_ram from RESOURCE_OVERLAY_SUPPORT or turn on MTK_LCA_RAM_OPTIMIZE)
  endif
endif
ifeq (yes,$(strip $(MTK_LCA_ROM_OPTIMIZE)))
  ifneq (yes,$(strip $(MTK_TABLET_PLATFORM)))
      ifeq ($(filter -sw600dp,$(MTK_PRODUCT_AAPT_CONFIG)),)
         $(call dep-err-common, pelase add -sw600dp in MTK_PRODUCT_AAPT_CONFIG or turn on MTK_TABLET_PLATFORM or turn off MTK_LCA_ROM_OPTIMIZE)
      endif
      ifeq ($(filter -sw720dp,$(MTK_PRODUCT_AAPT_CONFIG)),)
         $(call dep-err-common, pelase add -sw720dp in MTK_PRODUCT_AAPT_CONFIG or turn on MTK_TABLET_PLATFORM or turn off MTK_LCA_ROM_OPTIMIZE)
      endif
  endif
endif
ifneq (yes,$(strip $(MTK_LCA_ROM_OPTIMIZE)))
  ifeq (yes,$(strip $(MTK_TABLET_PLATFORM)))
      ifneq ($(filter -sw600dp,$(MTK_PRODUCT_AAPT_CONFIG)),)
         $(call dep-err-common, pelase removed -sw600dp in MTK_PRODUCT_AAPT_CONFIG or turn off MTK_TABLET_PLATFORM or turn on MTK_LCA_ROM_OPTIMIZE)
      endif      
      ifneq ($(filter -sw720dp,$(MTK_PRODUCT_AAPT_CONFIG)),)
         $(call dep-err-common, pelase removed -sw720dp in MTK_PRODUCT_AAPT_CONFIG or turn off MTK_TABLET_PLATFORM or turn on MTK_LCA_ROM_OPTIMIZE)
      endif
  endif
endif

############################################################
# for wifi_hotspot feature

ifeq (yes,$(strip $(MTK_WIFI_HOTSPOT_SUPPORT)))
  ifneq (yes, $(strip $(MTK_WLAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_WLAN_SUPPORT, MTK_WIFI_HOTSPOT_SUPPORT)
  endif
endif

##############################################################
# for matv feature

ifeq (yes,$(strip $(HAVE_MATV_FEATURE)))
  ifeq (,$(strip $(CUSTOM_HAL_MATV)))
     $(call dep-err-common, PLEASE turn off HAVE_MATV_FEATURE or set CUSTOM_HAL_MATV)
  endif
endif

ifeq (yes,$(strip $(HAVE_MATV_FEATURE)))
  ifeq (,$(strip $(CUSTOM_KERNEL_MATV)))
     $(call dep-err-common, PLEASE turn off HAVE_MATV_FEATURE or set CUSTOM_KERNEL_MATV)
  endif
endif

##############################################################
# for gps feature

ifeq (yes,$(strip $(MTK_AGPS_APP)))
  ifeq (no,$(strip $(MTK_GPS_SUPPORT)))
     $(call dep-err-ona-or-offb, MTK_GPS_SUPPORT, MTK_AGPS_APP)
  endif
endif

##############################################################
# for GEMINI feature
ifeq (yes, $(strip $(MTK_GEMINI_3G_SWITCH)))
  ifneq (yes, $(strip $(GEMINI)))
    $(call dep-err-ona-or-offb, GEMINI, MTK_GEMINI_3G_SWITCH)
  endif
  ifeq (0, $(strip $(MTK_GEMINI_SMART_3G_SWITCH)))
    $(call dep-err-seta-or-offb, MTK_GEMINI_SMART_3G_SWITCH,>=1,MTK_GEMINI_3G_SWITCH)
  endif
else
  ifneq (0, $(strip $(MTK_GEMINI_SMART_3G_SWITCH)))
    $(call dep-err-seta-or-onb, MTK_GEMINI_SMART_3G_SWITCH,0,MTK_GEMINI_3G_SWITCH)
  endif
endif

ifeq (yes, $(strip $(MTK_GEMINI_ENHANCEMENT)))
  ifneq (yes, $(strip $(GEMINI)))
    $(call dep-err-ona-or-offb, GEMINI, MTK_GEMINI_ENHANCEMENT)
  endif
endif

ifeq (yes, $(strip $(MTK_MULTISIM_RINGTONE_SUPPORT)))
  ifneq (yes, $(strip $(GEMINI)))
    $(call dep-err-ona-or-offb, GEMINI, MTK_MULTISIM_RINGTONE_SUPPORT)
  endif
endif

##############################################################
# for MTK_FOTA_SUPPORT feature

ifneq (yes, $(strip $(MTK_DM_APP)))
  ifneq (yes, $(strip $(MTK_MDM_SCOMO)))
    ifeq (yes, $(strip $(MTK_FOTA_SUPPORT)))
      $(call dep-err-ona-or-offb, MTK_DM_APP, MTK_FOTA_SUPPORT)
    endif
  endif
endif

ifeq (no, $(strip $(MTK_DM_APP)))
  ifeq (yes, $(strip $(MTK_SCOMO_ENTRY)))
    $(call dep-err-ona-or-offb, MTK_DM_APP, MTK_SCOMO_ENTRY)
  endif
endif

ifeq (no, $(strip $(MTK_FOTA_SUPPORT)))
  ifeq (yes, $(strip $(MTK_FOTA_ENTRY)))
    $(call dep-err-ona-or-offb, MTK_FOTA_SUPPORT, MTK_FOTA_ENTRY)
  endif
endif

ifeq (no, $(strip $(MTK_SMSREG_APP)))
  ifeq (yes, $(strip $(MTK_FOTA_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_SMSREG_APP, MTK_FOTA_SUPPORT)
  endif
endif

ifeq (no, $(strip $(MTK_SMSREG_APP)))
  ifeq (yes, $(strip $(MTK_DM_APP)))
    $(call dep-err-ona-or-offb, MTK_SMSREG_APP, MTK_DM_APP)
  endif
endif

ifeq (yes, $(strip $(MTK_DM_APP)))
  ifeq (yes, $(strip $(MTK_MDM_APP)))
    $(call dep-err-offa-or-offb, MTK_DM_APP, MTK_MDM_APP)
  endif
  ifeq (yes, $(strip $(MTK_RSDM_APP)))
    $(call dep-err-offa-or-offb, MTK_DM_APP, MTK_RSDM_APP)
  endif
endif

ifeq (yes, $(strip $(MTK_RSDM_APP)))
  ifeq (yes, $(strip $(MTK_MDM_APP)))
    $(call dep-err-offa-or-offb, MTK_RSDM_APP, MTK_MDM_APP)
  endif
endif

##############################################################
# for MtkWeatherProvider

ifeq (yes,$(MTK_WEATHER_WIDGET_APP))
  ifeq (no,$(MTK_WEATHER_PROVIDER_APP))
    $(call dep-err-ona-or-offb, MTK_WEATHER_PROVIDER_APP, MTK_WEATHER_WIDGET_APP)
  endif
endif

##############################################################
# for FD feature

ifeq (yes,$(MTK_FD_SUPPORT))
  ifeq ($(findstring _3g,$(MTK_MODEM_SUPPORT) $(MTK_MD2_SUPPORT)),)
#     $(call dep-err-common, please turn off MTK_FD_SUPPORT or set MTK_MODEM_SUPPORT/MTK_MD2_SUPPORT as modem_3g_tdd/modem_3g_fdd)
  endif
endif

ifeq (no,$(strip $(MTK_FD_SUPPORT)))
  ifeq (yes,$(strip $(MTK_FD_FORCE_REL_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_FD_SUPPORT, MTK_FD_FORCE_REL_SUPPORT)
  endif
endif

##############################################################
# for SNS feature

ifeq (yes,$(MTK_SNS_SINAWEIBO_APP))
  ifeq (no,$(MTK_SNS_SUPPORT))
     $(call dep-err-ona-or-offb, MTK_SNS_SUPPORT,MTK_SNS_SINAWEIBO_APP)
  endif
endif
#############################################################
# VOLD for partition generation
ifeq (yes, $(strip $(MTK_FAT_ON_NAND)))
  ifneq (yes,$(strip $(MTK_2SDCARD_SWAP)))
     $(call dep-err-ona-or-offb, MTK_2SDCARD_SWAP,MTK_FAT_ON_NAND)
  endif 
  ifneq (yes, $(strip $(MTK_MULTI_STORAGE_SUPPORT)))
     $(call dep-err-ona-or-offb, MTK_MULTI_STORAGE_SUPPORT, MTK_FAT_ON_NAND)
  endif
endif
##############################################################
# for VT voice answer feature

ifeq (OP01_SPEC0200_SEGC,$(OPTR_SPEC_SEG_DEF))
  ifeq (yes , $(strip $(MTK_VT3G324M_SUPPORT)))
    ifneq (yes,$(MTK_PHONE_VT_VOICE_ANSWER))
       $(call dep-err-common,pelease set OPTR_SPEC_SEG_DEF as non OP01_SPEC0200_SEGC or set MTK_VT3G324M_SUPPORT as no or turn off MTK_PHONE_VT_VOICE_ANSWER)
    endif
  endif
endif

ifeq (yes,$(MTK_PHONE_VT_VOICE_ANSWER))
  ifneq (OP01_SPEC0200_SEGC,$(OPTR_SPEC_SEG_DEF))
     $(call dep-err-seta-or-offb, OPTR_SPEC_SEG_DEF,OP01_SPEC0200_SEGC,MTK_PHONE_VT_VOICE_ANSWER)
  endif
  ifneq (yes, $(strip $(MTK_VT3G324M_SUPPORT)))
     $(call dep-err-ona-or-offb, MTK_VT3G324M_SUPPORT,MTK_PHONE_VT_VOICE_ANSWER)
  endif
endif


##############################################################
# for VT multimedia ringtone

ifeq (OP01_SPEC0200_SEGC,$(OPTR_SPEC_SEG_DEF))
  ifeq (yes, $(strip $(MTK_VT3G324M_SUPPORT)))
    ifneq (yes,$(MTK_PHONE_VT_MM_RINGTONE))
       $(call dep-err-common, please set OPTR_SPEC_SEG_DEF as non OP01_SPEC0200_SEGC or set MTK_VT3G324M_SUPPORT as no or turn off MTK_PHONE_VT_MM_RINGTONE)
    endif
  endif
endif

ifeq (yes,$(MTK_PHONE_VT_MM_RINGTONE))
  ifneq (OP01_SPEC0200_SEGC,$(OPTR_SPEC_SEG_DEF))
     $(call dep-err-seta-or-offb, OPTR_SPEC_SEG_DEF,OP01_SPEC0200_SEGC,MTK_PHONE_VT_MM_RINGTONE)
  endif
  ifneq (yes,$(strip $(MTK_VT3G324M_SUPPORT)))
     $(call dep-err-ona-or-offb, MTK_VT3G324M_SUPPORT,MTK_PHONE_VT_MM_RINGTONE)
  endif 
endif

##############################################################
# for BT 

ifeq (no,$(strip $(MTK_BT_SUPPORT)))
  ifeq (yes,$(strip $(MTK_BT_21_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_21_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_BT_30_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_30_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_BT_30_HS_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_30_HS_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_BT_40_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_40_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_BT_FM_OVER_BT_VIA_CONTROLLER)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_FM_OVER_BT_VIA_CONTROLLER)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_A2DP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_A2DP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_AVRCP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_AVRCP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_AVRCP14)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_AVRCP14)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_BIP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_BIP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_BPP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_BPP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_DUN)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_DUN)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_FTP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_FTP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_HFP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_HFP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_HIDH)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_HIDH)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_MANAGER)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_MANAGER)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_MAPC)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_MAPC)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_MAPS)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_MAPS)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_OPP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_OPP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PAN)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_PAN)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PBAP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_PBAP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PRXM)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_PRXM)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PRXR)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_PRXR)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_SIMAP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_SIMAP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_SPP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_SPP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_TIMEC)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_TIMEC)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_TIMES)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_TIMES)
  endif
endif

ifeq (no,$(strip $(MTK_BT_40_SUPPORT)))
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PRXM)))
    $(call dep-err-ona-or-offb, MTK_BT_40_SUPPORT, MTK_BT_PROFILE_PRXM)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PRXR)))
    $(call dep-err-ona-or-offb, MTK_BT_40_SUPPORT, MTK_BT_PROFILE_PRXR)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_TIMEC)))
    $(call dep-err-ona-or-offb, MTK_BT_40_SUPPORT, MTK_BT_PROFILE_TIMEC)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_TIMES)))
    $(call dep-err-ona-or-offb, MTK_BT_40_SUPPORT, MTK_BT_PROFILE_TIMES)
  endif
endif

ifeq (yes,$(strip $(MTK_BT_FM_OVER_BT_VIA_CONTROLLER)))
  ifeq (no,$(strip $(MTK_BT_PROFILE_A2DP)))
    $(call dep-err-ona-or-offb, MTK_BT_PROFILE_A2DP, MTK_BT_FM_OVER_BT_VIA_CONTROLLER)
  endif
endif

##############################################################
# for emmc feature
ifneq (yes,$(strip $(MTK_EMMC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_FSCK_TUNE)))
    $(call dep-err-ona-or-offb, MTK_EMMC_SUPPORT, MTK_FSCK_TUNE)
  endif
endif
##############################################################
# for emmc otp
ifeq (yes,$(strip $(MTK_EMMC_SUPPORT_OTP)))
  ifneq (yes,$(strip $(MTK_EMMC_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_EMMC_SUPPORT, MTK_EMMC_SUPPORT_OTP)
  endif
endif

ifeq (yes,$(strip $(MTK_EMMC_SUPPORT_OTP)))
  ifeq (FALSE,$(strip $(MODEM_OTP_SUPPORT)))
    $(call dep-err-ona-or-offb, MODEM_OTP_SUPPORT, MTK_EMMC_SUPPORT_OTP)
  endif
endif

ifeq (TRUE,$(strip $(MODEM_OTP_SUPPORT)))
  ifneq (yes,$(strip $(MTK_EMMC_SUPPORT_OTP)))
    ifneq (yes,$(strip $(NAND_OTP_SUPPORT)))
      $(call dep-err-ona-or-onb, MTK_EMMC_SUPPORT_OTP,NAND_OTP_SUPPORT)
    endif
  endif
endif

ifeq (yes, $(strip $(MTK_COMBO_NAND_SUPPORT)))
  ifeq (yes, $(strip $(MTK_EMMC_SUPPORT)))
    $(call dep-err-common, Please turn off MTK_COMBO_NAND_SUPPORT or turn off MTK_EMMC_SUPPORT)
  endif
endif
##############################################################
# for NFC feature
ifeq (yes,$(strip $(MTK_NFC_SUPPORT)))
  ifeq (no,$(strip $(CONFIG_NFC_PN544)))
    $(call dep-err-ona-or-offb, CONFIG_NFC_PN544, MTK_NFC_SUPPORT)
  endif
endif

ifeq (no,$(strip $(MTK_NFC_SUPPORT)))
  ifeq (yes,$(strip $(CONFIG_NFC_PN544)))
    $(call dep-err-ona-or-offb, MTK_NFC_SUPPORT, CONFIG_NFC_PN544)
  endif
endif

ifeq (yes,$(strip $(MTK_NFC_SUPPORT)))
  ifeq (no,$(strip $(CONFIG_MTK_NFC)))
    $(call dep-err-ona-or-offb, CONFIG_MTK_NFC, MTK_NFC_SUPPORT)
  endif
endif

ifeq (no,$(strip $(MTK_NFC_SUPPORT)))
  ifeq (yes,$(strip $(CONFIG_MTK_NFC)))
    $(call dep-err-ona-or-offb, MTK_NFC_SUPPORT, CONFIG_MTK_NFC)
  endif
endif

ifeq (no,$(strip $(MTK_NFC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_BEAM_PLUS_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_NFC_SUPPORT, MTK_BEAM_PLUS_SUPPORT)
  endif
endif

ifneq (yes,$(strip $(MTK_NFC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_NFC_APP_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_NFC_SUPPORT, MTK_NFC_APP_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_NFC_ADDON_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_NFC_SUPPORT, MTK_NFC_ADDON_SUPPORT)
  endif
endif

ifeq (yes,$(strip $(MTK_WIFIWPSP2P_NFC_SUPPORT)))
  ifneq (yes,$(strip $(MTK_WIFI_P2P_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_WIFI_P2P_SUPPORT, MTK_WIFIWPSP2P_NFC_SUPPORT)
  endif
  ifneq (yes,$(strip $(MTK_NFC_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_NFC_SUPPORT, MTK_WIFIWPSP2P_NFC_SUPPORT)
  endif
endif
##############################################################
# for fm feature
ifeq (no,$(strip $(MTK_FM_SUPPORT)))
  ifeq (yes,$(strip $(MTK_FM_RECORDING_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_FM_SUPPORT, MTK_FM_RECORDING_SUPPORT)
  endif
endif

##############################################################
# for Brazil customization
ifeq (no,$(strip $(MTK_BRAZIL_CUSTOMIZATION)))
  ifeq (yes,$(strip $(MTK_BRAZIL_CUSTOMIZATION_TIM)))
    $(call dep-err-ona-or-offb, MTK_BRAZIL_CUSTOMIZATION, MTK_BRAZIL_CUSTOMIZATION_TIM)
  endif
endif

##############################################################
# for mtk brazil customization feature

ifeq (no,$(strip $(MTK_BRAZIL_CUSTOMIZATION)))
  ifeq (yes,$(strip $(MTK_BRAZIL_CUSTOMIZATION_VIVO)))
    $(call dep-err-common, please set MTK_BRAZIL_CUSTOMIZATION_VIVO=no when MTK_BRAZIL_CUSTOMIZATION=no)
  endif
  ifeq (yes,$(strip $(MTK_BRAZIL_CUSTOMIZATION_CLARO)))
    $(call dep-err-common, please set MTK_BRAZIL_CUSTOMIZATION_CLARO=no when MTK_BRAZIL_CUSTOMIZATION=no)
  endif
endif

##############################################################
# for HAVE_CMMB_FEATURE and CUSTOM_KERNEL_CMMB

ifeq (yes,$(HAVE_CMMB_FEATURE))
  ifneq (cmmb,$(CUSTOM_KERNEL_CMMB))
    $(call dep-err-seta-or-offb,CUSTOM_KERNEL_CMMB,cmmb,HAVE_CMMB_FEATURE)
  endif
endif

ifeq (no,$(HAVE_CMMB_FEATURE))
  ifdef CUSTOM_KERNEL_CMMB
    $(call dep-err-common,CUSTOM_KERNEL_CMMB could only be defined when HAVE_CMMB_FEATURE = yes)
  endif
endif

##############################################################
# for MD DB
ifeq (no,$(strip $(MTK_MDLOGGER_SUPPORT)))
  ifeq (yes,$(strip $(MTK_INCLUDE_MODEM_DB_IN_IMAGE)))
     $(call dep-err-ona-or-offb, MTK_MDLOGGER_SUPPORT, MTK_INCLUDE_MODEM_DB_IN_IMAGE)
  endif
endif

##############################################################
# for phone number geo-description
ifneq ($(filter OP01% OP02%, $(OPTR_SPEC_SEG_DEF)),)
  ifeq (no,$(strip $(MTK_PHONE_NUMBER_GEODESCRIPTION)))
    $(call dep-err-seta-or-onb, OPTR_SPEC_SEG_DEF,none OP01/OP02,MTK_PHONE_NUMBER_GEODESCRIPTION)
  endif
endif
###########################################################
#for customer open OP02 in single sim mode
ifneq ($(filter OP02%, $(OPTR_SPEC_SEG_DEF)),)
   ifneq (yes, $(strip $(GEMINI)))
      $(call dep-err-seta-or-onb, OPTR_SPEC_SEG_DEF,none OP02,GEMINI)
   endif
endif
#############################################################
# MTK_MDM_APP, MTK_DM_APP and MTK_RSDM_APP are exclusive. (Only one can be enabled at the same time.)
ifeq ($(strip $(MTK_MDM_APP)),yes)
  ifeq ($(strip $(MTK_DM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_MDM_APP, MTK_DM_APP)
  endif
  ifeq ($(strip $(MTK_RSDM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_MDM_APP, MTK_RSDM_APP)
  endif
endif

ifeq ($(strip $(MTK_DM_APP)),yes)
  ifeq ($(strip $(MTK_MDM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_DM_APP, MTK_MDM_APP)
  endif
  ifeq ($(strip $(MTK_RSDM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_DM_APP, MTK_RSDM_APP)
  endif
endif

ifeq ($(strip $(MTK_RSDM_APP)),yes)
  ifeq ($(strip $(MTK_MDM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_RSDM_APP, MTK_MDM_APP)
  endif
  ifeq ($(strip $(MTK_DM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_RSDM_APP, MTK_DM_APP)
  endif
endif
###########################################################
ifeq (yes, $(strip $(MTK_MDM_LAWMO)))
  ifneq (yes,$(strip $(MTK_MDM_APP)))
     $(call dep-err-ona-or-offb,MTK_MDM_APP,MTK_MDM_LAWMO)
  endif
endif
ifeq (yes, $(strip $(MTK_MDM_FUMO)))
  ifneq (yes,$(strip $(MTK_MDM_APP)))
     $(call dep-err-ona-or-offb,MTK_MDM_APP,MTK_MDM_FUMO)
  endif
  ifneq (yes,$(strip $(MTK_FOTA_SUPPORT)))
     $(call dep-err-ona-or-offb,MTK_FOTA_SUPPORTP,MTK_MDM_FUMO)
  endif
endif
ifeq (yes, $(strip $(MTK_MDM_SCOMO)))
  ifneq (yes,$(strip $(MTK_MDM_APP)))
     $(call dep-err-ona-or-offb,MTK_MDM_APP,MTK_MDM_SCOMO)
  endif
endif

#############################################################
ifneq (yes,$(strip $(MTK_TABLET_PLATFORM)))
  ifeq (240,$(strip $(LCM_WIDTH)))
    ifeq (320,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter ldpi,$(MTK_PRODUCT_AAPT_CONFIG)),)
        $(call dep-err-common, Please add ldpi to MTK_PRODUCT_AAPT_CONFIG or set different LCM_WIDTH and LCM_HEIGHT)
      endif       
    endif
  endif
  ifeq (320,$(strip $(LCM_WIDTH)))
    ifeq (480,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter mdpi,$(MTK_PRODUCT_AAPT_CONFIG)),)
        $(call dep-err-common, Please add mdpi to MTK_PRODUCT_AAPT_CONFIG or set different LCM_WIDTH and LCM_HEIGHT)
      endif       
    endif
  endif
  ifeq (480,$(strip $(LCM_WIDTH)))
    ifeq (800,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter hdpi,$(MTK_PRODUCT_AAPT_CONFIG)),)
        $(call dep-err-common, Please add hdpi to MTK_PRODUCT_AAPT_CONFIG or set different LCM_WIDTH and LCM_HEIGHT)
      endif       
    endif
  endif
  ifeq (540,$(strip $(LCM_WIDTH)))
    ifeq (960,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter hdpi,$(MTK_PRODUCT_AAPT_CONFIG)),)
        $(call dep-err-common, Please add hdpi to MTK_PRODUCT_AAPT_CONFIG or set different LCM_WIDTH and LCM_HEIGHT)
      endif       
    endif
  endif
  ifeq (720,$(strip $(LCM_WIDTH)))
    ifeq (1280,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter hdpi,$(MTK_PRODUCT_AAPT_CONFIG)),)
        $(call dep-err-common, Please add hdpi to MTK_PRODUCT_AAPT_CONFIG or set different LCM_WIDTH and LCM_HEIGHT)
      endif  
      ifeq ($(filter xhdpi,$(MTK_PRODUCT_AAPT_CONFIG)),)
        $(call dep-err-common, Please add xhdpi to MTK_PRODUCT_AAPT_CONFIG or set different LCM_WIDTH and LCM_HEIGHT)
      endif 
    endif
  endif
endif

############################################################
ifeq (yes,$(MTK_WEATHER3D_WIDGET))
  ifeq (no,$(MTK_WEATHER_PROVIDER_APP))
        $(call dep-err-ona-or-offb, MTK_WEATHER_PROVIDER_APP, MTK_WEATHER3D_WIDGET)
  endif
endif

############################################################
ifeq ($(strip $(MTK_COMBO_CHIP)),MT6628)
  ifneq ($(strip $(MTK_BT_FM_OVER_BT_VIA_CONTROLLER)),no)
    $(call dep-err-seta-or-setb,MTK_BT_FM_OVER_BT_VIA_CONTROLLER,no,MTK_COMBO_CHIP,none MT6628)
  endif
endif
ifeq ($(strip $(MTK_BT_CHIP)),MTK_MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
     $(call dep-err-seta-or-setb,MTK_BT_CHIP, none MTK_MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif
ifeq ($(strip $(MTK_FM_CHIP)),MT6628_FM)  
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
    $(call dep-err-seta-or-setb,MTK_FM_CHIP, none MT6628_FM,MTK_COMBO_CHIP,MT6628)
  endif
endif
ifeq ($(strip $(MTK_WLAN_CHIP)),MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
    $(call dep-err-seta-or-setb,MTK_WLAN_CHIP, none MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif
ifeq ($(strip $(MTK_GPS_CHIP)),MTK_GPS_MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
    $(call dep-err-seta-or-setb,MTK_GPS_CHIP, none MTK_GPS_MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif

############################################################
ifeq ($(strip $(MTK_AP_SPEECH_ENHANCEMENT)),yes)
  ifneq ($(strip $(MTK_AUDIO_HD_REC_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_AUDIO_HD_REC_SUPPORT, MTK_AP_SPEECH_ENHANCEMENT)
  endif
endif
############################################################
ifneq ($(strip $(MTK_LOG2SERVER_APP)),yes)
  ifeq ($(strip $(MTK_LOG2SERVER_INTERNAL)),yes)
    $(call dep-err-ona-or-offb, MTK_LOG2SERVER_APP, MTK_LOG2SERVER_INTERNAL)
  endif
endif
############################################################
ifeq ($(strip $(MTK_INTERNAL_HDMI_SUPPORT)),yes)
  ifeq ($(strip $(MTK_INTERNAL_MHL_SUPPORT)),yes)
    $(call dep-err-offa-or-offb, MTK_INTERNAL_HDMI_SUPPORT, MTK_INTERNAL_MHL_SUPPORT)
  endif
  ifneq ($(strip $(MTK_HDMI_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_HDMI_SUPPORT, MTK_INTERNAL_HDMI_SUPPORT)
  endif
else
  ifeq ($(strip $(MTK_INTERNAL_MHL_SUPPORT)),yes)
    ifneq ($(strip $(MTK_HDMI_SUPPORT)),yes)
      $(call dep-err-ona-or-offb, MTK_HDMI_SUPPORT, MTK_INTERNAL_MHL_SUPPORT)
    endif
  endif
endif
############################################################
ifneq ($(strip $(OPTR_SPEC_SEG_DEF)), NONE)
  ifneq ($(strip $(MTK_NETWORK_TYPE_ALWAYS_ON)), no)
     $(call dep-err-common, Please set OPTR_SPEC_SEG_DEF as NONE or set MTK_NETWORK_TYPE_ALWAYS_ON as no)
  endif
endif
############################################################
ifeq ($(strip $(MTK_S3D_SUPPORT)),yes)
  ifneq ($(strip $(MTK_STEREO3D_WALLPAPER_APP)),yes)
    $(call dep-err-ona-or-offb, MTK_STEREO3D_WALLPAPER_APP, MTK_S3D_SUPPORT)
  endif
endif
ifneq ($(strip $(MTK_S3D_SUPPORT)),yes)
  ifeq ($(strip $(MTK_STEREO3D_WALLPAPER_APP)),yes)
    $(call dep-err-ona-or-offb, MTK_S3D_SUPPORT, MTK_STEREO3D_WALLPAPER_APP)
  endif
endif
############################################################
ifeq (yes,$(strip $(MTK_FD_FORCE_REL_SUPPORT)))
  ifneq (yes,$(strip $(MTK_FD_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_FD_SUPPORT, MTK_FD_FORCE_REL_SUPPORT)
  endif
endif
############################################################
ifeq ($(strip $(MTK_GEMINI_3SIM_SUPPORT)),yes)
  ifneq ($(strip $(GEMINI)),yes)
    $(call dep-err-ona-or-offb, GEMINI, MTK_GEMINI_3SIM_SUPPORT)
  endif
endif
ifeq ($(strip $(MTK_GEMINI_4SIM_SUUPORT)),yes)
  ifneq ($(strip $(GEMINI)),yes)
    $(call dep-err-ona-or-offb, GEMINI, MTK_GEMINI_4SIM_SUUPORT)
  endif
endif
############################################################
ifneq ($(filter OP02%, $(OPTR_SPEC_SEG_DEF)),)
  ifeq ($(strip $(MTK_GEMINI_3G_SWITCH)),yes)
    $(call dep-err-common, Please do not set OPTR_SPEC_SEG_DEF as OP02* or set MTK_GEMINI_3G_SWITCH as no)
  endif
endif
############################################################
ifeq ($(strip $(MTK_MT8193_HDCP_SUPPORT)),yes)
  ifneq ($(strip $(MTK_MT8193_HDMI_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_MT8193_HDMI_SUPPORT, MTK_MT8193_HDCP_SUPPORT)
  endif
endif
ifeq ($(strip $(MTK_MT8193_HDMI_SUPPORT)),yes)
  ifneq ($(strip $(MTK_MT8193_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_MT8193_SUPPORT, MTK_MT8193_HDMI_SUPPORT)
  endif
endif
ifeq ($(strip $(MTK_MT8193_NFI_SUPPORT)),yes)
  ifneq ($(strip $(MTK_MT8193_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_MT8193_SUPPORT, MTK_MT8193_NFI_SUPPORT)
  endif
endif
############################################################
ifeq (yes, $(strip $(MTK_SIM_HOT_SWAP)))
  ifneq (no, $(strip $(MTK_RADIOOFF_POWER_OFF_MD)))
    $(call dep-err-ona-or-offb, MTK_RADIOOFF_POWER_OFF_MD, MTK_SIM_HOT_SWAP)
  endif
endif
############################################################
ifneq ($(filter OP02%, $(OPTR_SPEC_SEG_DEF)),)
  ifeq ($(strip $(MTK_SIP_SUPPORT)),yes)
    $(call dep-err-common, Please do not set OPTR_SPEC_SEG_DEF as OP02* or set MTK_SIP_SUPPORT as no)
  endif
endif
############################################################
ifeq (yes, $(strip $(MTK_WVDRM_L1_SUPPORT)))
  ifneq (yes , $(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    ifneq (yes, $(strip $(TRUSTONIC_TEE_SUPPORT)))
      $(call dep-err-common, Please turn on MTK_IN_HOUSE_TEE_SUPPORT or turn on TRUSTONIC_TEE_SUPPORT when MTK_WVDRM_L1_SUPPORT set as yes)
    endif
  endif
  ifneq (yes , $(strip $(MTK_DRM_KEY_MNG_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_DRM_KEY_MNG_SUPPORT,MTK_WVDRM_L1_SUPPORT)
  endif
  ifneq (yes , $(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_SEC_VIDEO_PATH_SUPPORT,MTK_WVDRM_L1_SUPPORT)
  endif
endif
ifeq (yes, $(strip $(MTK_DRM_KEY_MNG_SUPPORT)))
  ifneq (yes, $(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    ifneq (yes, $(strip $(TRUSTONIC_TEE_SUPPORT)))
      $(call dep-err-common, Please turn on MTK_IN_HOUSE_TEE_SUPPORT or turn on TRUSTONIC_TEE_SUPPORT when MTK_DRM_KEY_MNG_SUPPORT set as yes)
    endif
  endif
endif
ifeq (yes , $(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)))
  ifneq (yes, $(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    ifneq (yes, $(strip $(TRUSTONIC_TEE_SUPPORT)))
      $(call dep-err-common, Please turn on MTK_IN_HOUSE_TEE_SUPPORT or turn on TRUSTONIC_TEE_SUPPORT when MTK_SEC_VIDEO_PATH_SUPPORT set as yes)
    endif
  endif
endif

ifneq (yes,$(strip $(MTK_AUDIO_HD_REC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_VOIP_ENHANCEMENT_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_AUDIO_HD_REC_SUPPORT, MTK_VOIP_ENHANCEMENT_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_HANDSFREE_DMNR_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_AUDIO_HD_REC_SUPPORT, MTK_HANDSFREE_DMNR_SUPPORT)
  endif
endif

ifeq ($(filter zh_CN,$(MTK_PRODUCT_LOCALES)),)
  ifeq (yes,$(strip $(MTK_QQBROWSER_SUPPORT)))
    $(call dep-err-common, pelase add zh_CN in MTK_PRODUCT_LOCALES or turn off MTK_QQBROWSER_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_TENCENT_MOBILE_MANAGER_SLIM_SUPPORT)))
    $(call dep-err-common, pelase add zh_CN in MTK_PRODUCT_LOCALES or turn off MTK_TENCENT_MOBILE_MANAGER_SLIM_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_TENCENT_MOBILE_MANAGER_NORMAL_SUPPORT)))
    $(call dep-err-common, pelase add zh_CN in MTK_PRODUCT_LOCALES or turn off MTK_TENCENT_MOBILE_MANAGER_NORMAL_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_SINA_WEIBO_SUPPORT)))
    $(call dep-err-common, pelase add zh_CN in MTK_PRODUCT_LOCALES or turn off MTK_SINA_WEIBO_SUPPORT)
  endif
endif

################################################################
ifeq (yes, $(strip $(MTK_AUTO_SANITY)))
  ifneq (yes, $(strip $(HAVE_AEE_FEATURE)))
     $(call dep-err-ona-or-offb,HAVE_AEE_FEATURE,MTK_AUTO_SANITY)
  endif
endif
###############################################################
###############################################################
#for mp release  check release package OPTR_SPEC_SEG_DEF 
ifneq ($(filter OP01%,$(strip $(OPTR_SPEC_SEG_DEF))),)
  ifneq ($(filter rel_customer_operator_cmcc, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_cmcc)
    $(call dep-err-common, please use rel_customer_operator_cmcc as optr release package in MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP01)
  endif
endif
ifneq ($(filter OP02%,$(strip $(OPTR_SPEC_SEG_DEF))),)
  ifneq ($(filter rel_customer_operator_cu, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_cu)
    $(call dep-err-common, please use rel_customer_operator_cu as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP02)
  endif
endif
ifneq ($(filter OP03%,$(strip $(OPTR_SPEC_SEG_DEF))),)
  ifneq ($(filter rel_customer_operator_orange, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_orange)
    $(call dep-err-common, please use rel_customer_operator_orange as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP03)
  endif
endif

ifneq ($(filter OP06%,$(strip $(OPTR_SPEC_SEG_DEF))),)
  ifneq ($(filter rel_customer_operator_vodafone, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_vodafone)
    $(call dep-err-common, please use rel_customer_operator_vodafone as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP06)
  endif
endif
ifneq ($(filter OP07%,$(strip $(OPTR_SPEC_SEG_DEF))),)
  ifneq ($(filter rel_customer_operator_att, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_att)
    $(call dep-err-common, please use rel_customer_operator_att as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP07)
  endif
endif
ifneq ($(filter OP08%,$(strip $(OPTR_SPEC_SEG_DEF))),)
  ifneq ($(filter rel_customer_operator_tmo_us, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_tmo_us)
    $(call dep-err-common, please use rel_customer_operator_tmo_us as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP08)
  endif
endif
ifneq ($(filter OP09%,$(strip $(OPTR_SPEC_SEG_DEF))),)
  ifneq ($(filter rel_customer_operator_ct, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_ct)
    $(call dep-err-common, please use rel_customer_operator_ct as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP09)
  endif
endif


ifeq ($(strip $(MTK_BSP_PACKAGE)),yes)
  ifneq ($(filter rel_customer_bsp, $(MTK_RELEASE_PACKAGE)),rel_customer_bsp)
     $(call dep-err-common, please add rel_customer_bsp in to MTK_RELEASE_PACKAGE When MTK_BSP_PACKAGE is yes)
  endif
endif
ifneq (,$(strip $(MTK_PLATFORM)))
  ifeq ($(filter rel_customer_platform%,$(MTK_RELEASE_PACKAGE)),)
     $(call dep-err-common, please add rel_customer_platform_xxx in to MTK_RELEASE_PACKAGE)
  endif
endif
#############################################################
ifeq (yes,$(strip $(MTK_AIV_SUPPORT)))
  ifneq (yes, $(strip $(MTK_DRM_PLAYREADY_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_AIV_SUPPORT,MTK_DRM_PLAYREADY_SUPPORT)
  endif
endif
################################################################
ifneq (yes, $(strip $(MTK_DUAL_MIC_SUPPORT)))
    ifeq (yes, $(strip $(MTK_ASR_SUPPORT)))
       $(call dep-err-ona-or-offb,MTK_DUAL_MIC_SUPPORT,MTK_ASR_SUPPORT)
    endif
    ifeq (yes, $(strip $(MTK_VOIP_NORMAL_DMNR)))
       $(call dep-err-ona-or-offb,MTK_DUAL_MIC_SUPPORT,MTK_VOIP_NORMAL_DMNR)
    endif 
    ifeq (yes, $(strip $(MTK_VOIP_NORMAL_DMNR)))
       $(call dep-err-ona-or-offb,MTK_DUAL_MIC_SUPPORT,MTK_VOIP_NORMAL_DMNR)
    endif
    ifeq (yes, $(strip $(MTK_VOIP_HANDSFREE_DMNR)))
       $(call dep-err-ona-or-offb,MTK_DUAL_MIC_SUPPORT,MTK_VOIP_HANDSFREE_DMNR)
    endif
    ifeq (yes, $(strip $(MTK_INCALL_HANDSFREE_DMNR)))
       $(call dep-err-ona-or-offb,MTK_DUAL_MIC_SUPPORT,MTK_INCALL_HANDSFREE_DMNR)
    endif
    ifeq (yes, $(strip $(MTK_INCALL_NORMAL_DMNR)))
       $(call dep-err-ona-or-offb,MTK_DUAL_MIC_SUPPORT,MTK_INCALL_NORMAL_DMNR)
    endif
else
    ifneq (yes, $(strip $(MTK_INCALL_NORMAL_DMNR)))
       $(call dep-err-ona-or-offb,MTK_INCALL_NORMAL_DMNR,MTK_DUAL_MIC_SUPPORT)
    endif
endif
ifneq (yes, $(strip $(MTK_VOIP_ENHANCEMENT_SUPPORT)))
   ifeq (yes, $(strip $(MTK_VOIP_NORMAL_DMNR)))
       $(call dep-err-ona-or-offb,MTK_VOIP_ENHANCEMENT_SUPPORT,MTK_VOIP_NORMAL_DMNR)
    endif 
   ifeq (yes, $(strip $(MTK_VOIP_HANDSFREE_DMNR)))
       $(call dep-err-ona-or-offb,MTK_VOIP_ENHANCEMENT_SUPPORT,MTK_VOIP_HANDSFREE_DMNR)
    endif
endif
################################################################
ifeq (yes,$(strip $(MTK_WFD_HDCP_TX_SUPPORT)))
  ifneq (yes, $(strip $(MTK_DRM_KEY_MNG_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_DRM_KEY_MNG_SUPPORT,MTK_WFD_HDCP_TX_SUPPORT)
  endif
  ifneq (yes, $(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_IN_HOUSE_TEE_SUPPORT,MTK_WFD_HDCP_TX_SUPPORT)
  endif
endif
ifneq (yes, $(strip $(MTK_REGIONALPHONE_SUPPORT)))
   ifeq (yes, $(strip $(MTK_TER_SERVICE)))
     $(call dep-err-ona-or-offb,TK_REGIONALPHONE_SUPPORT,MTK_TER_SERVICE)
   endif
endif

ifeq (yes,$(strip $(MTK_SEC_WFD_VIDEO_PATH_SUPPORT)))
  ifneq (yes,$(strip $(MTK_WFD_HDCP_TX_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_WFD_HDCP_TX_SUPPORT,MTK_SEC_WFD_VIDEO_PATH_SUPPORT)
  endif
  ifneq (yes,$(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_SEC_VIDEO_PATH_SUPPORT,MTK_SEC_WFD_VIDEO_PATH_SUPPORT)
  endif
  ifneq (yes,$(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_IN_HOUSE_TEE_SUPPORT,MTK_SEC_WFD_VIDEO_PATH_SUPPORT)
  endif
endif
#######################################################################
ifeq (yes, $(strip $(MTK_SIM_HOT_SWAP_COMMON_SLOT)))
  ifneq (yes, $(strip $(MTK_SIM_HOT_SWAP)))
    $(call dep-err-ona-or-offb,MTK_SIM_HOT_SWAP,MTK_SIM_HOT_SWAP_COMMON_SLOT)
  endif
endif

ifeq (yes,$(strip $(MTK_INTERNAL)))
  ifeq ($(strip $(OPTR_SPEC_SEG_DEF)), OP01_SPEC0200_SEGC)
     ifneq (yes,$(strip $(MTK_CTSC_MTBF_INTERNAL_SUPPORT)))
       $(call dep-err-ona-or-offb, MTK_CTSC_MTBF_INTERNAL_SUPPORT, MTK_INTERNAL)
     endif
 else
     ifeq (yes,$(strip $(MTK_CTSC_MTBF_INTERNAL_SUPPORT)))
       $(call dep-err-common, turn off MTK_CTSC_MTBF_INTERNAL_SUPPORT or set OPTR_SPEC_SEG_DEF as OP01_SPEC0200_SEGC)
     endif
 endif
else
    ifeq (yes,$(strip $(MTK_CTSC_MTBF_INTERNAL_SUPPORT)))
      $(call dep-err-ona-or-offb, MTK_INTERNAL, MTK_CTSC_MTBF_INTERNAL_SUPPORT)
    endif
endif
############################
ifeq ($(strip $(MTK_SWIP_WMAPRO)), yes)
  ifneq (yes, $(strip $(MTK_WMV_PLAYBACK_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_WMV_PLAYBACK_SUPPORT,MTK_SWIP_WMAPRO)
  endif
endif
################################################
ifeq (yes,$(strip $(MTK_TENCENT_MOBILE_MANAGER_SLIM_SUPPORT)))
  ifeq (yes, $(strip $(MTK_TENCENT_MOBILE_MANAGER_NORMAL_SUPPORT)))
    $(call dep-err-common,,Please turn off MTK_TENCENT_MOBILE_MANAGER_SLIM_SUPPORT or turn off MTK_TENCENT_MOBILE_MANAGER_NORMAL_SUPPORT) 
  endif
endif
################################################
ifeq (OP01_SPEC0200_SEGC, $(strip $(OPTR_SPEC_SEG_DEF)))
   ifeq ($(filter cmcc_%, $(BOOT_LOGO)),)
      $(call dep-err-common, Please set BOOT_LOGO as cmcc_% or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifneq ($(MTK_UMTS_TDD128_MODE),yes)
      $(call dep-err-common, Please turn on MTK_UMTS_TDD128_MODE or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifneq ($(MTK_NOTEBOOK_SUPPORT),yes)
      $(call dep-err-common, Please turn on MTK_NOTEBOOK_SUPPORT or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifneq ($(MTK_WML_SUPPORT),yes)
      $(call dep-err-common, Please turn on MTK_WML_SUPPORT or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifneq ($(MTK_WORLD_CLOCK_WIDGET_APP),yes)
      $(call dep-err-common, Please turn on MTK_WORLD_CLOCK_WIDGET_APP or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifeq ($(MTK_FLIGHT_MODE_POWER_OFF_MD),yes)
      $(call dep-err-common, Please turn off MTK_FLIGHT_MODE_POWER_OFF_MD or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifeq ($(MTK_APKINSTALLER_APP),yes)
      $(call dep-err-common, Please turn off MTK_APKINSTALLER_APP or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifeq ($(MTK_DATA_TRANSFER_APP),yes)
      $(call dep-err-common, Please turn off   or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifneq ($(MTK_RTSP_BITRATE_ADAPTATION_SUPPORT),yes)
      $(call dep-err-common, Please turn on MTK_RTSP_BITRATE_ADAPTATION_SUPPORT  or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifeq ($(MTK_SHARED_SDCARD),yes)
      $(call dep-err-common, Please turn off MTK_SHARED_SDCARD  or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
   ifeq ($(MTK_STREAMING_VIDEO_SUPPORT),yes)
      $(call dep-err-common, Please turn off MTK_STREAMING_VIDEO_SUPPORT or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
   endif
  ifeq ($(filter zh_CN en_US, $(MTK_PRODUCT_LOCALES)),)
      $(call dep-err-common, Please add zh_CN en_US to MTK_PRODUCT_LOCALES or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC)
  endif
ifeq ($(MTK_INTERNAL), yes)
   ifeq ($(MTK_DM_APP),yes)
      $(call dep-err-common, Please turn off MTK_DM_APP or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC When MTK_INTERNAL set as yes)
   endif
   ifeq ($(MTK_QQBROWSER_SUPPORT),yes)
      $(call dep-err-common, Please turn off MTK_QQBROWSER_SUPPORT or set OPTR_SPEC_SEG_DEF as not OP01_SPEC0200_SEGC When MTK_INTERNAL set as yes) 
   endif
endif
endif

ifeq (yes,$(strip $(MTK_PERMISSION_CONTROL)))
  ifneq (yes,$(strip $(MTK_MOBILE_MANAGEMENT)))
    $(call dep-err-ona-or-offb,MTK_MOBILE_MANAGEMENT,MTK_PERMISSION_CONTROL)
  endif
endif

ifeq (yes,$(strip $(MTK_GAMELOFT_GLL_ULC_CN_APP)))
  ifeq (yes,$(strip $(MTK_GAMELOFT_GLL_ULC_WW_APP)))
    $(call dep-err-offa-or-offb,MTK_GAMELOFT_GLL_ULC_CN_APP,MTK_GAMELOFT_GLL_ULC_WW_APP)
  endif
endif
ifeq (yes,$(strip $(MTK_GAMELOFT_AVENGERS_ULC_CN_APP)))
  ifeq (yes,$(strip $(MTK_GAMELOFT_AVENGERS_ULC_WW_APP)))
    $(call dep-err-offa-or-offb,MTK_GAMELOFT_AVENGERS_ULC_CN_APP,MTK_GAMELOFT_AVENGERS_ULC_WW_APP)
  endif
endif
ifeq (yes,$(strip $(MTK_GAMELOFT_LBC_ULC_CN_APP)))
  ifeq (yes,$(strip $(MTK_GAMELOFT_LBC_ULC_WW_APP)))
    $(call dep-err-offa-or-offb,MTK_GAMELOFT_LBC_ULC_CN_APP,MTK_GAMELOFT_LBC_ULC_WW_APP)
  endif
endif
ifeq (yes,$(strip $(MTK_GAMELOFT_WONDERZOO_ULC_CN_APP)))
  ifeq (yes,$(strip $(MTK_GAMELOFT_WONDERZOO_ULC_WW_APP)))
    $(call dep-err-offa-or-offb,MTK_GAMELOFT_WONDERZOO_ULC_CN_APP,MTK_GAMELOFT_WONDERZOO_ULC_WW_APP)
  endif
endif
##############################################################################################################
ifeq (yes, $(strip $(MTK_GAMELOFT_KINGDOMANDLORDS_CN_APP)))
  ifeq (yes, $(strip $(MTK_GAMELOFT_KINGDOMANDLORDS_WW_APP)))
        $(call dep-err-common, Please turn off MTK_GAMELOFT_KINGDOMANDLORDS_CN_APP or turn off MTK_GAMELOFT_KINGDOMANDLORDS_WW_APP)
  endif
endif
ifeq (yes, $(strip $(MTK_GAMELOFT_UNOANDFRIENDS_CN_APP)))
  ifeq (yes, $(strip $(MTK_GAMELOFT_UNOANDFRIENDS_WW_APP)))
        $(call dep-err-common, Please turn off MTK_GAMELOFT_UNOANDFRIENDS_CN_APP or turn off MTK_GAMELOFT_UNOANDFRIENDS_WW_APP)
  endif
endif
ifeq (yes, $(strip $(MTK_GAMELOFT_WONDERZOO_CN_APP)))
  ifeq (yes, $(strip $(MTK_GAMELOFT_WONDERZOO_WW_APP)))
        $(call dep-err-common, Please turn off MTK_GAMELOFT_WONDERZOO_CN_APP or turn off MTK_GAMELOFT_WONDERZOO_WW_APP)
  endif
endif
###############################################################################################################
ifeq ($(strip $(MTK_GEMINI_SMART_3G_SWITCH)),2)
  ifneq ($(strip $(MTK_RILD_READ_IMSI)),yes)
          $(call dep-err-common, Please turn on MTK_RILD_READ_IMSI or set MTK_RILD_READ_IMSI not 2)
  endif
endif
############################################################
ifeq (yes,$(strip $(MTK_EMMC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_COMBO_NAND_SUPPORT)))
    $(call dep-err-offa-or-offb,MTK_EMMC_SUPPORT,MTK_COMBO_NAND_SUPPORT)
  endif
endif
#############################################################
ifeq (yes,$(strip $(MTK_PLAYREADY_SUPPORT)))
  ifneq (yes, $(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_SEC_VIDEO_PATH_SUPPORT,MTK_PLAYREADY_SUPPORT)
  endif
endif
#############################################################
ifeq (yes,$(strip $(MTK_DX_HDCP_SUPPORT)))
  ifneq (yes, $(strip $(MTK_PERSIST_PARTITION_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_PERSIST_PARTITION_SUPPORT,MTK_DX_HDCP_SUPPORT)
  endif
endif
ifeq (yes,$(strip $(MTK_PLAYREADY_SUPPORT)))
  ifneq (yes, $(strip $(MTK_PERSIST_PARTITION_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_PERSIST_PARTITION_SUPPORT,MTK_PLAYREADY_SUPPORT)
  endif
endif
#########################################################
ifneq ($(filter yes,$(MTK_DM_APP) $(MTK_DEVREG_APP) $(MTK_MDM_APP) $(MTK_SMSREG_APP)),)
  ifneq ($(strip $(MTK_DM_AGENT_SUPPORT)), yes)
    $(call dep-err-common, please set MTK_DM_APP and MTK_DEVREG_APP and MTK_MDM_APP and MTK_SMSREG_APP as no or set MTK_DM_AGENT_SUPPORT as yes)
  endif
endif
ifeq ($(strip $(MTK_DM_AGENT_SUPPORT)), yes)
  ifeq ($(filter yes,$(MTK_DM_APP) $(MTK_DEVREG_APP) $(MTK_MDM_APP) $(MTK_SMSREG_APP)),)
    $(call dep-err-common, please set MTK_DM_APP or MTK_DEVREG_APP or MTK_MDM_APP or MTK_SMSREG_APP as yes or set MTK_DM_AGENT_SUPPORT as no)
  endif
endif
