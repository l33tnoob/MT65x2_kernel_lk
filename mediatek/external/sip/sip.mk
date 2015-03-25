#
# SIP VoIP
# 
# Include this file in a product makefile to include SIP VoIP configuration xml files
#
# Enable SIP VoIP: Use the xml files located in the enable_sip folder
# Disable SIP VoIP:Use the xml files located in the disable_sip folder
# 

LOCAL_PATH:= mediatek/external/sip

ifeq ($(MTK_SIP_SUPPORT),yes)
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/enable_sip/android.software.sip.xml:system/etc/permissions/android.software.sip.xml
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/enable_sip/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml
else
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/disable_sip/android.software.sip.xml:system/etc/permissions/android.software.sip.xml
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/disable_sip/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml
endif