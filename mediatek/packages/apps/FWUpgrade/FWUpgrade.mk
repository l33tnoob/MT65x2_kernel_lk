ifeq ($(strip $(MTK_FW_UPGRADE)), yes)
PRODUCT_PACKAGES += FWUpgrade \
                    FWUpgradeProvider
PRODUCT_COPY_FILES += mediatek/packages/apps/FWUpgrade/fotabinder:system/bin/fotabinder

PRODUCT_PACKAGES += FWUpgradeInit.rc
PRODUCT_PROPERTY_OVERRIDES += ro.fota.oem=MTK_KK
PRODUCT_PROPERTY_OVERRIDES += ro.fota.platform=MTK_KK
PRODUCT_PROPERTY_OVERRIDES += ro.fota.type=phone
endif
