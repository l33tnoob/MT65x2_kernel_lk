ifneq ($(filter banyan_addon banyan_addon_x86,$(TARGET_PRODUCT)),)

# include MTK SDK toolset version file
include $(MTK_PATH_SOURCE)/frameworks/banyan/TOOLSET_VERSION

ifeq ($(strip $(MTK_BSP_PACKAGE)),no)
# make dependency between banyan_addon/sdk_addon and checkmtkapi
sdk_addon: checkmtkapi mtk-clean-temp
endif

mtk-clean-temp:
	@rm -rf $(TARGET_PRODUCT_OUT_ROOT)/mediatek

endif

