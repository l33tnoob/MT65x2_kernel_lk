#if defined(MTK_HDMI_SUPPORT)
#include <mach/mt_clkmgr.h>
#include <asm/ioctl.h>

#include "dpi_reg.h"
#include "dpi1_drv.h"
#include "hdmi_types.h"
#include "mtkfb_info.h"
#include "hdmi_utils.h"
#include "hdmi_dpi_config.h"

#ifdef MTK_MT8193_HDMI_SUPPORT
#include "mt8193edid.h"
#endif

//~~~~~~~~~~~~~~~~~~~~~~~the static variable~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int hdmi_log_on  = 1;

static atomic_t timeline_counter = ATOMIC_INIT(0);
static atomic_t fence_counter = ATOMIC_INIT(0);

static struct sw_sync_timeline *hdmi_timeline ;
static struct ion_client *ion_client;
static struct list_head  HDMI_Buffer_List;

static DPI_POLARITY clk_pol, de_pol, hsync_pol, vsync_pol;
static unsigned int dpi_clk_div, dpi_clk_duty;
static unsigned int hsync_pulse_width, hsync_back_porch, hsync_front_porch;
static unsigned int vsync_pulse_width, vsync_back_porch, vsync_front_porch;

static HDMI_COLOR_ORDER rgb_order;
static unsigned int hdmi_resolution_param_table[][3] =
{
    {720,   480,    60},
#ifdef MTK_SMARTBOOK_SUPPORT
    //{1366,  768,    60},
    {1280,  720,    60},
#else
    {1280,  720,    60},
#endif
    {1920,  1080,   30},
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ---------------------------------------------------------------------------
//  function implement
// ---------------------------------------------------------------------------
//--------------------------FIXME-------------------------------
void hdmi_dpi_config_hdmi()
{
    DPI_CHECK_RET(HDMI_DPI(_ConfigHDMI)());
}

void hdmi_dpi_config_bg(bool enable, int bg_width, int bg_height)
{
    DPI_CHECK_RET(HDMI_DPI(_ConfigBG)(enable, bg_width, bg_height));
}

void hdmi_dpi_colorbar_enable(bool enable)
{
    DPI_CHECK_RET(HDMI_DPI(_EnableColorBar(enable)));
}

void hdmi_dpi_dump_register()
{
    DPI_CHECK_RET(HDMI_DPI(_DumpRegisters()));
}

//free IRQ
void hdmi_dpi_free_irq()
{
    DPI_CHECK_RET(HDMI_DPI(_FreeIRQ)());
}

void hdmi_dpi_clk_enable(bool enable)
{
    if (enable)
    {
        DPI_CHECK_RET(HDMI_DPI(_EnableClk)());
    }
    else
    {
        DPI_CHECK_RET(HDMI_DPI(_DisableClk)());
    }
}

void hdmi_dpi_disable_MipiClk()
{
    DPI_CHECK_RET(HDMI_DPI(_DisableMipiClk)());
}

void hdmi_dpi_config_update(int hdmi_width, int hdmi_height)
{
    HDMI_FUNC();

    DPI_CHECK_RET(HDMI_DPI(_ConfigPixelClk)(clk_pol, dpi_clk_div, dpi_clk_duty));
    DPI_CHECK_RET(HDMI_DPI(_ConfigDataEnable)(de_pol)); // maybe no used
    DPI_CHECK_RET(HDMI_DPI(_ConfigHsync)(hsync_pol, hsync_pulse_width, hsync_back_porch, hsync_front_porch));
    DPI_CHECK_RET(HDMI_DPI(_ConfigVsync)(vsync_pol, vsync_pulse_width, vsync_back_porch, vsync_front_porch));
    DPI_CHECK_RET(HDMI_DPI(_FBSetSize)(hdmi_width, hdmi_height));

    DPI_CHECK_RET(HDMI_DPI(_FBSetPitch)(DPI_FB_0, hdmi_width * 3)); // do nothing
    DPI_CHECK_RET(HDMI_DPI(_FBEnable)(DPI_FB_0, TRUE)); // do nothing

    //OUTREG32(0xF208C090, 0x41);
    DPI_CHECK_RET(HDMI_DPI(_FBSetFormat)(DPI_FB_FORMAT_RGB888)); // do nothing

    if (HDMI_COLOR_ORDER_BGR == rgb_order)
    {
        DPI_CHECK_RET(HDMI_DPI(_SetRGBOrder)(DPI_RGB_ORDER_RGB, DPI_RGB_ORDER_BGR)); // do nothing
    }
    else
    {
        DPI_CHECK_RET(HDMI_DPI(_SetRGBOrder)(DPI_RGB_ORDER_RGB, DPI_RGB_ORDER_RGB)); // do nothing
    }
    //hdmi_dpi_colorbar_enable(true);
    //DPI_CHECK_RET(HDMI_DPI(_ConfigInRBSwap)(true));
}


/* Will only be used in hdmi_drv_init(), this means that will only be use in ioctl(MTK_HDMI_AUDIO_VIDEO_ENABLE) */
void hdmi_dpi_config_clock(HDMI_PARAMS *hdmi_params)
{
    int ret = 0;

    switch (hdmi_params->init_config.vformat)
    {
        case HDMI_VIDEO_720x480p_60Hz:
        {
            printk("[hdmi]480p\n");
            ASSERT(!ret);

            dpi_clk_div = 2;
            dpi_clk_duty = 1;

            break;
        }
        case HDMI_VIDEO_1280x720p_60Hz:
        {
            printk("[hdmi]720p 60Hz\n");
            ASSERT(!ret);

            dpi_clk_div = 2;
            dpi_clk_duty = 1;

            break;
        }
        case HDMI_VIDEO_1920x1080p_30Hz:
        {
            printk("[hdmi]1080p 30Hz\n");
            dpi_clk_div = 2;
            dpi_clk_duty = 1;

            break;
        }
        default:
        {
            printk("[hdmi] not supported format, %s, %d, format = %d\n", __func__, __LINE__, hdmi_params->init_config.vformat);
            break;
        }
    }

    clk_pol     = hdmi_params->clk_pol;
    de_pol      = hdmi_params->de_pol;
    hsync_pol   = hdmi_params->hsync_pol;
    vsync_pol   = hdmi_params->vsync_pol;;

    hsync_pulse_width   = hdmi_params->hsync_pulse_width;
    vsync_pulse_width   = hdmi_params->vsync_pulse_width;
    hsync_back_porch    = hdmi_params->hsync_back_porch;
    vsync_back_porch    = hdmi_params->vsync_back_porch;
    hsync_front_porch   = hdmi_params->hsync_front_porch;
    vsync_front_porch   = hdmi_params->vsync_front_porch;
    rgb_order           = hdmi_params->rgb_order;

    DPI_CHECK_RET(HDMI_DPI(_Init)(FALSE));
}

void hdmi_dpi_power_enable(bool enable, bool clock_on)
{
    HDMI_FUNC();
    if (enable)
    {
        HDMI_DPI(_PowerOn)();
        HDMI_DPI(_EnableIrq)();
        DPI_CHECK_RET(HDMI_DPI(_EnableClk)());
    }
    else
    {
        HDMI_DPI(_DisableIrq)();
        HDMI_DPI(_DisableClk)();
        HDMI_DPI(_PowerOff)();
    }
}

void hdmi_dpi_config_DualEdge(bool enable)
{
    if (enable) {
        DPI_CHECK_RET(HDMI_DPI(_ConfigDualEdge)(true));
    } else {
        DPI_CHECK_RET(HDMI_DPI(_ConfigDualEdge)(false));
    }
}

void hdmi_dpi_setting_res(u8 arg, HDMI_CABLE_TYPE cable_type, _t_hdmi_context *hdmi_context_unstatic)
{
    switch (arg)
    {
        case HDMI_VIDEO_720x480p_60Hz:
        {

            clk_pol     = HDMI_POLARITY_FALLING;
            de_pol      = HDMI_POLARITY_RISING;
            hsync_pol   = HDMI_POLARITY_RISING;
            vsync_pol   = HDMI_POLARITY_RISING;;

            dpi_clk_div = 2;

            hsync_pulse_width   = 62;
            hsync_back_porch    = 60;
            hsync_front_porch   = 16;

            vsync_pulse_width   = 6;
            vsync_back_porch    = 30;
            vsync_front_porch   = 9;

            hdmi_context_unstatic->bg_height = ((480 * hdmi_context_unstatic->scaling_factor) / 100 >> 2) << 2 ;
            hdmi_context_unstatic->bg_width = ((720 * hdmi_context_unstatic->scaling_factor) / 100 >> 2) << 2 ;
            hdmi_context_unstatic->hdmi_width = 720 - hdmi_context_unstatic->bg_width;
            hdmi_context_unstatic->hdmi_height = 480 - hdmi_context_unstatic->bg_height;
            hdmi_context_unstatic->output_video_resolution = HDMI_VIDEO_720x480p_60Hz;
            break;
        }
        case HDMI_VIDEO_1280x720p_60Hz:
        {

            clk_pol      = HDMI_POLARITY_FALLING;
            de_pol       = HDMI_POLARITY_RISING;
            hsync_pol    = HDMI_POLARITY_RISING;
            vsync_pol    = HDMI_POLARITY_RISING;;

            dpi_clk_div = 2;

            hsync_pulse_width    = 40;
            hsync_back_porch     = 220;
            hsync_front_porch    = 110;

            vsync_pulse_width    = 5;
            vsync_back_porch     = 20;
            vsync_front_porch    = 5;

            hdmi_context_unstatic->bg_height = ((720 * hdmi_context_unstatic->scaling_factor) / 100 >> 2) << 2 ;
            hdmi_context_unstatic->bg_width = ((1280 * hdmi_context_unstatic->scaling_factor) / 100 >> 2) << 2 ;
            hdmi_context_unstatic->hdmi_width = 1280 - hdmi_context_unstatic->bg_width; //1280  1366
            hdmi_context_unstatic->hdmi_height = 720 - hdmi_context_unstatic->bg_height;//720;  768
#ifdef MTK_SMARTBOOK_SUPPORT
            if (cable_type == MHL_SMB_CABLE)
            {
                hdmi_context_unstatic->hdmi_width = 1366;
                hdmi_context_unstatic->hdmi_height = 768;
                hdmi_context_unstatic->bg_height = 0;
                hdmi_context_unstatic->bg_width = 0;
            }
#endif
            hdmi_context_unstatic->output_video_resolution = HDMI_VIDEO_1280x720p_60Hz;
            break;
        }
        case HDMI_VIDEO_1920x1080p_30Hz:
        {

            clk_pol     = HDMI_POLARITY_FALLING;
            de_pol      = HDMI_POLARITY_RISING;
            hsync_pol   = HDMI_POLARITY_RISING;
            vsync_pol   = HDMI_POLARITY_RISING;;

            dpi_clk_div = 2;

            hsync_pulse_width   = 44;
            hsync_back_porch    = 148;
            hsync_front_porch   = 88;

            vsync_pulse_width   = 5;
            vsync_back_porch    = 36;
            vsync_front_porch   = 4;

            hdmi_context_unstatic->bg_height = ((1080 * hdmi_context_unstatic->scaling_factor) / 100 >> 2) << 2 ;
            hdmi_context_unstatic->bg_width = ((1920 * hdmi_context_unstatic->scaling_factor) / 100 >> 2) << 2 ;
            hdmi_context_unstatic->hdmi_width = 1920 - hdmi_context_unstatic->bg_width;
            hdmi_context_unstatic->hdmi_height = 1080 - hdmi_context_unstatic->bg_height;
            hdmi_context_unstatic->output_video_resolution = HDMI_VIDEO_1920x1080p_30Hz;
            break;
        }
        default:
            break;
    }

}

int hdmi_config_pll(HDMI_VIDEO_RESOLUTION resolution)
{

    unsigned int TxDiv0, TxDiv1;
    unsigned int TxMul;

    switch (resolution)
    {
        case HDMI_VIDEO_720x480p_60Hz:
        {
            TxDiv0 = 2;
            TxDiv1 = 0;
            TxMul = 1115039586;
            break;
        }
        case HDMI_VIDEO_1280x720p_60Hz:
        {
            TxDiv0 = 0;
            TxDiv1 = 0;
            TxMul = 766589715;	//765825707 for 59.94Hz;
            break;
        }
        case HDMI_VIDEO_1920x1080p_30Hz:
        {
            TxDiv0 = 0;
            TxDiv1 = 0;
            TxMul = 766589715;
            break;
        }
        default:
        {
            printk("[hdmi] not supported format, %s, %d, format = %d\n", __func__, __LINE__, resolution);
            return -1;
        }
    }

    DPI_CHECK_RET(HDMI_DPI(_Init_PLL)(TxMul, TxDiv0, TxDiv1));
/*
		unsigned int con1, con0;
		unsigned int clk_src = 1;
	
		switch (resolution)
		{
			case HDMI_VIDEO_720x480p_60Hz:
			{
				con1 = 0x8010a1ca;
				con0 = 0x141;
				clk_src = 2;
				break;
			}
	
			case HDMI_VIDEO_1920x1080p_30Hz:
			{
				con1 = 0x800B6C4F;
				con0 = 0x131;
				break;
			}
	
			case HDMI_VIDEO_1280x720p_60Hz:
			{
				con1 = 0x800B6C4F;
				con0 = 0x131;
				break;
			}
	
			default:
			{
				printk("[hdmi] not supported format, %s, %d, format = %d\n", __func__, __LINE__, resolution);
				return DPI_STATUS_ERROR;
			}
		}
	
		DPI_CHECK_RET(HDMI_DPI(_Init_PLL)(clk_src, con0, con1));
*/
    return 0;
}

unsigned int hdmi_get_width(HDMI_VIDEO_RESOLUTION r)
{
    ASSERT(r < HDMI_VIDEO_RESOLUTION_NUM);
    return hdmi_resolution_param_table[r][0];
}

unsigned int hdmi_get_height(HDMI_VIDEO_RESOLUTION r)
{
    ASSERT(r < HDMI_VIDEO_RESOLUTION_NUM);
    return hdmi_resolution_param_table[r][1];
}

#endif
