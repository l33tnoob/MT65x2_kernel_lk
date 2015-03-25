# drivers/video/mtk/Makefile
#
# Makefile for MT6516 video driver.
#

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/hdmitx/  \
             -I$(MTK_PATH_PLATFORM)/drivers/video/   \
             -I$(MTK_PATH_PLATFORM)/drivers/videox/  \
             -I$(MTK_PATH_PLATFORM)/drivers/dispsys/ \
             -I$(MTK_PATH_PLATFORM)/../../common/kernel/drivers/video/ \
             -I$(MTK_PATH_SOURCE)/drivers/video/     \
             -I$(MTK_PATH_SOURCE)/drivers/sync/ \
             -I$(MTK_PATH_SOURCE)/drivers/mhl/driver/mhl_tx_drv/ \
             -I$(MTK_ROOT_CUSTOM_OUT)/kernel/hdmi/inc/ 

ifeq ($(CUSTOM_KERNEL_HDMI),nxp_tda19989)
	ccflags-y += 	-DHDMI_TDA19989
endif

ccflags-y += 	-DMTK_HDMI_FENCE_SUPPORT
ccflags-y += 	-DMTK_HDMI_ION_SUPPORT

obj-y	:= hdmi_buffers_manager.o hdmi_dpi_config.o hdmitx_platform.o hdmitx.o debug.o hdmitx_test.o
