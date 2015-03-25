#if defined(MTK_HDMI_SUPPORT)
#include <mach/mt_typedefs.h>
#include <linux/types.h>
//#include <linux/leds-mt65xx.h>

#include "lcm_drv.h"
#include "ddp_hal.h"
#include "ddp_rdma.h"
#include "disp_drv.h"

//#include "disp_sync.h"
//#include "ddp_path.h"

#include "hdmi_types.h"
#include "hdmitx_platform.h"

static HDMI_DRIVER *mhl_drv = NULL;

//extern unsigned int isAEEEnabled;
//extern OVL_CONFIG_STRUCT cached_layer_config[DDP_OVL_LAYER_MUN];

// ---------------------------------------------------------------------------
//  function implement
// ---------------------------------------------------------------------------
void hdmi_read_reg_platform(unsigned char u8Reg, unsigned int *p4Data)
{
#ifdef MTK_MT8193_HDMI_SUPPORT
    mhl_drv->read(u8Reg, p4Data);
#else
    mhl_drv->read(u8Reg);
#endif
}

void deliver_driver_interface(HDMI_DRIVER *hdmi_drv)
{
    if (hdmi_drv == NULL)
    {
        printk("[HDMI_Platform] hdmi driver interface is null\n");
    }

    mhl_drv = hdmi_drv;
}

void hdmi_m4u_port_config(M4U_PORT_STRUCT *m4u_port)
{
    if (m4u_port)
    {
        memset((void *)m4u_port, 0, sizeof(M4U_PORT_STRUCT));
        m4u_port->ePortID = DISP_RDMA;
        m4u_port->Virtuality = 1;
        m4u_port->domain = 0;
        m4u_port->Security = 0;
        m4u_port->Distance = 1;
        m4u_port->Direction = 0;
        m4u_config_port(m4u_port);
    }
}

int check_edid_header()
{
    int ret = 0;
#ifdef MTK_MT8193_HDMI_SUPPORT
    if (mhl_drv->checkedidheader != NULL)
    {
        ret = mhl_drv->checkedidheader();
    }
#else
    ret = -1;
#endif

    return ret;
}

void hdmi_config_main_disp()
{
    extern LCM_PARAMS *lcm_params;

    switch(lcm_params->type)
    {
        case LCM_TYPE_DPI : 
        {
            break;
        }
        case LCM_TYPE_DSI :
        {
            struct disp_path_config_struct config = {0};

            RDMASetTargetLine(0, lcm_params->height*4/5);

            if (DISP_IsDecoupleMode()) 
            {
                config.srcModule = DISP_MODULE_RDMA;
            }else {
                config.srcModule = DISP_MODULE_OVL;
            }

            config.bgROI.x = 0;
            config.bgROI.y = 0;
            config.bgROI.width  = lcm_params->width;
            config.bgROI.height = lcm_params->height;
            config.bgColor = 0x0;	// background color

            config.pitch = lcm_params->width * 3;
            config.srcROI.x = 0;
            config.srcROI.y = 0;
            config.srcROI.height = lcm_params->height;
            config.srcROI.width  = lcm_params->width;
            config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
	
            if(lcm_params->dsi.mode == CMD_MODE)
            {
                config.dstModule = DISP_MODULE_DSI_CMD; // DISP_MODULE_WDMA1
            }else {
                config.dstModule = DISP_MODULE_DSI_VDO; // DISP_MODULE_WDMA1
            }
			
            config.outFormat = RDMA_OUTPUT_FORMAT_ARGB; 
            disp_path_config(&config);
            break;
        }
        default :
        {
            break;
        }
    }
}

int hdmi_ioctl_platform(unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;

    int r = 0;
#ifdef MTK_MT8193_HDMI_SUPPORT
    hdmi_device_write w_info;
    hdmi_hdcp_key key;
    send_slt_data send_sltdata;
    CEC_SLT_DATA get_sltdata;
    hdmi_para_setting data_info;
    HDMI_EDID_INFO_T pv_get_info;
    CEC_FRAME_DESCRIPTION cec_frame;
    CEC_ADDRESS cecaddr;
    CEC_DRV_ADDR_CFG_T cecsetAddr;
    CEC_SEND_MSG_T cecsendframe;
    READ_REG_VALUE regval;
#endif

    switch (cmd)
    {
#ifdef MTK_MT8193_HDMI_SUPPORT
        case MTK_HDMI_WRITE_DEV:
        {
            if (copy_from_user(&w_info, (void __user *)arg, sizeof(w_info)))
            {
                printk("copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            else
            {
                mhl_drv->write(w_info.u4Addr & 0xFFFF, w_info.u4Data);
            }
            break;
        }

        case MTK_HDMI_INFOFRAME_SETTING:
        {
            if (copy_from_user(&data_info, (void __user *)arg, sizeof(data_info)))
            {
                printk("copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            else
            {
                mhl_drv->InfoframeSetting(data_info.u4Data1 & 0xFF, data_info.u4Data2 & 0xFF);
            }
            break;
        }

        case MTK_HDMI_HDCP_KEY:
        {
            if (copy_from_user(&key, (void __user *)arg, sizeof(key)))
            {
                printk("copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            else
            {
                mhl_drv->hdcpkey((UINT8 *)&key);
            }
            break;
        }

        case MTK_HDMI_SETLA:
        {
            if (copy_from_user(&cecsetAddr, (void __user *)arg, sizeof(cecsetAddr)))
            {
                printk("copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            else
            {
                mhl_drv->setcecla(&cecsetAddr);
            }
            break;
        }

        case MTK_HDMI_SENDSLTDATA:
        {
            if (copy_from_user(&send_sltdata, (void __user *)arg, sizeof(send_sltdata)))
            {
                printk("copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            else
            {
                mhl_drv->sendsltdata((UINT8 *)&send_sltdata);
            }
            break;
        }

        case MTK_HDMI_SET_CECCMD:
        {
            if (copy_from_user(&cecsendframe, (void __user *)arg, sizeof(cecsendframe)))
            {
                printk("copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            else
            {
                mhl_drv->setceccmd(&cecsendframe);
            }
            break;
        }

        case MTK_HDMI_CEC_ENABLE:
        {
            mhl_drv->cecenable(arg & 0xFF);
            break;
        }


        case MTK_HDMI_GET_EDID:
        {
            mhl_drv->getedid(&pv_get_info);
            if (copy_to_user((void __user *)arg, &pv_get_info, sizeof(pv_get_info)))
            {
                printk("copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            break;
        }

        case MTK_HDMI_GET_CECCMD:
        {
            mhl_drv->getceccmd(&cec_frame);
            if (copy_to_user((void __user *)arg, &cec_frame, sizeof(cec_frame)))
            {
                printk("copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            break;
        }

        case MTK_HDMI_GET_SLTDATA:
        {
            mhl_drv->getsltdata(&get_sltdata);
            if (copy_to_user((void __user *)arg, &get_sltdata, sizeof(get_sltdata)))
            {
                printk("copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            break;
        }

        case MTK_HDMI_GET_CECADDR:
        {
            mhl_drv->getcecaddr(&cecaddr);
            if (copy_to_user((void __user *)arg, &cecaddr, sizeof(cecaddr)))
            {
                printk("copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            break;
        }

        case MTK_HDMI_COLOR_DEEP:
        {
            if (copy_from_user(&data_info, (void __user *)arg, sizeof(data_info)))
            {
                printk("copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            else
            {
                mhl_drv->colordeep(data_info.u4Data1 & 0xFF, data_info.u4Data2 & 0xFF);
            }
            break;
        }

        case MTK_HDMI_READ_DEV:
        {
            if (copy_from_user(&regval, (void __user *)arg, sizeof(regval)))
            {
                printk("copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            else
            {
                mhl_drv->read(regval.u1adress & 0xFFFF, &regval.pu1Data);
            }

            if (copy_to_user((void __user *)arg, &regval, sizeof(regval)))
            {
                printk("copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            break;
        }

        case MTK_HDMI_ENABLE_LOG:
        {
            mhl_drv->log_enable(arg & 0xFFFF);
            break;
        }

        case MTK_HDMI_ENABLE_HDCP:
        {
            mhl_drv->enablehdcp(arg & 0xFFFF);
            break;
        }

        case MTK_HDMI_CECRX_MODE:
        {
            mhl_drv->setcecrxmode(arg & 0xFFFF);
            break;
        }

        case MTK_HDMI_STATUS:
        {
            mhl_drv->hdmistatus();
            break;
        }

        case MTK_HDMI_CHECK_EDID:
        {
            mhl_drv->checkedid(arg & 0xFF);
            break;
        }
#endif
        default:
        {
            printk("[HDMI_Platform] arguments error %x\n", cmd);
            break;
        }
    }
}
#endif
