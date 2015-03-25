obj-$(CONFIG_MTK_FB) := dsi_drv.o dpi_drv.o lcd_drv.o disp_drv_dbi.o disp_drv_dpi.o disp_drv_dsi.o disp_hal.o disp_intr.o

ccflags-y += -I$(MTK_PATH_SOURCE)/drivers/video/ 							\
             -I$(MTK_PATH_PLATFORM)/../../common/kernel/drivers/video/ 		\
			 -I$(MTK_PATH_PLATFORM)/drivers/dispsys/

ifeq ($(MTK_GPU_SUPPORT), yes)
ccflags-y += -DHWGPU_SUPPORT
endif
