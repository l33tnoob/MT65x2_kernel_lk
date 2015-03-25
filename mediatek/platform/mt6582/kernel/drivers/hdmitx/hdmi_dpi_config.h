#ifndef     _HDMI_DPI_CONFIG_H_
#define     _HDMI_DPI_CONFIG_H_

#include "hdmi_types.h"
#include "ddp_drv.h"

#ifdef MTK_MT8193_HDMI_SUPPORT
#define HDMI_DPI(suffix)        DPI1 ## suffix
#define HMID_DEST_DPI           DISP_MODULE_DPI1
#else
#define HDMI_DPI(suffix)        DPI  ## suffix
#define HMID_DEST_DPI           DISP_MODULE_DPI0
#endif

#define HDMI_RDMA_ADDR 0x0000
#define HDMI_SRC_RDMA  DISP_MODULE_RDMA

void hdmi_dpi_config_hdmi();
void hdmi_dpi_config_bg(bool enable, int bg_width, int bg_height);
void hdmi_dpi_colorbar_enable(bool enable);
void hdmi_dpi_dump_register();
void hdmi_dpi_free_irq();
void hdmi_dpi_clk_enable(bool enable);
void hdmi_dpi_disable_MipiClk();
void hdmi_dpi_config_update(int hdmi_width, int hdmi_height);
void hdmi_dpi_config_clock(HDMI_PARAMS *hdmi_params);
void hdmi_dpi_power_enable(bool enable, bool clock_on);
void hdmi_dpi_config_DualEdge(bool enable);
void hdmi_dpi_setting_res(u8 arg, HDMI_CABLE_TYPE cable_type, _t_hdmi_context *hdmi_context_unstatic);
int hdmi_config_pll(HDMI_VIDEO_RESOLUTION resolution);
unsigned int hdmi_get_width(HDMI_VIDEO_RESOLUTION r);
unsigned int hdmi_get_height(HDMI_VIDEO_RESOLUTION r);

#endif
