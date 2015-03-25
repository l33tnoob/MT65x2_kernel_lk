#if defined(MTK_HDMI_SUPPORT)
#define _tx_c_
#include <linux/autoconf.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/switch.h>
#include <linux/leds-mt65xx.h>

#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <mach/dma.h>
#include <mach/irqs.h>
#include <asm/tlbflush.h>
#include <asm/page.h>

#include <mach/m4u.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_boot.h>

#include "mtkfb_info.h"
#include "hdmitx_drv.h"
#include "hdmi_utils.h"
#include "hdmi_types.h"
#include "hdmi_buffers_manager.h"
#include "hdmi_dpi_config.h"
#include "hdmitx_platform.h"

#include "mach/eint.h"
#include "mach/irqs.h"

#include "disp_drv_platform.h"
#include "ddp_reg.h"

#ifdef MTK_SMARTBOOK_SUPPORT
#include <linux/sbsuspend.h>
#include "smartbook.h"
#endif

#ifdef I2C_DBG
#include "tmbslHdmiTx_types.h"
#include "tmbslTDA9989_local.h"
#endif

//#include "hdmi_customization.h"
//~~~~~~~~~~~~~~~~~~~~~~~the static variable~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static HDMI_DRIVER *hdmi_drv = NULL;

static bool factory_mode = false;
static bool otg_enable_status = false;
static int dp_mutex_src = -1;
static int dp_mutex_dst = -1;
static int hdmi_log_on  = 1;
static int hdmi_bufferdump_on = 7;
static unsigned long hdmi_reschange = HDMI_VIDEO_RESOLUTION_NUM;

static wait_queue_head_t hdmi_vsync_wq;
static bool hdmi_vsync_flag = false;
static int hdmi_vsync_cnt   = 0;

static struct switch_dev hdmi_switch_data;
static struct switch_dev hdmires_switch_data;

static atomic_t hdmi_fake_in         = ATOMIC_INIT(false);
static atomic_t hdmi_video_mode_flag = ATOMIC_INIT(0);

static _t_hdmi_context hdmi_context;
static _t_hdmi_context *p = &hdmi_context;

static dev_t hdmi_devno;
static struct cdev *hdmi_cdev;
static struct class *hdmi_class = NULL;

static struct task_struct *hdmi_rdma_config_task = NULL;
static wait_queue_head_t hdmi_rdma_config_wq;
static atomic_t hdmi_rdma_config_event = ATOMIC_INIT(0);

static struct task_struct *hdmi_rdma_update_task = NULL;
static wait_queue_head_t hdmi_rdma_update_wq;
static atomic_t hdmi_rdma_update_event = ATOMIC_INIT(0);

#ifdef MTK_SMARTBOOK_SUPPORT
static struct task_struct *hdmi_status_update_task = NULL;
static wait_queue_head_t hdmi_status_update_wq;
static atomic_t hdmi_status_update_event = ATOMIC_INIT(0);
#endif

DEFINE_SEMAPHORE(hdmi_video_mode_mutex);
static DEFINE_SEMAPHORE(hdmi_update_mutex);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~the gloable variable~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOL hdmi_is_active = FALSE;
bool smarbook_enable = FALSE;

int mmp_image_scale = 10;
HDMI_MMP_Events_t HDMI_MMP_Events;

HDMI_PARAMS _s_hdmi_params = {0};
HDMI_PARAMS *hdmi_params = &_s_hdmi_params;

#ifdef MTK_HDMI_SCREEN_CAPTURE
bool capture_screen = false;
unsigned long capture_addr;
#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~the definition~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define IS_HDMI_ON()            (HDMI_POWER_STATE_ON == atomic_read(&p->state))
#define IS_HDMI_OFF()           (HDMI_POWER_STATE_OFF == atomic_read(&p->state))
#define IS_HDMI_STANDBY()       (HDMI_POWER_STATE_STANDBY == atomic_read(&p->state))

#define IS_HDMI_NOT_ON()        (HDMI_POWER_STATE_ON != atomic_read(&p->state))
#define IS_HDMI_NOT_OFF()       (HDMI_POWER_STATE_OFF != atomic_read(&p->state))
#define IS_HDMI_NOT_STANDBY()   (HDMI_POWER_STATE_STANDBY != atomic_read(&p->state))

#define SET_HDMI_ON()           atomic_set(&p->state, HDMI_POWER_STATE_ON)
#define SET_HDMI_OFF()          atomic_set(&p->state, HDMI_POWER_STATE_OFF)
#define SET_HDMI_STANDBY()      atomic_set(&p->state, HDMI_POWER_STATE_STANDBY)

#define IS_HDMI_FAKE_PLUG_IN()  (true == atomic_read(&hdmi_fake_in))
#define SET_HDMI_FAKE_PLUG_IN() (atomic_set(&hdmi_fake_in, true))
#define SET_HDMI_NOT_FAKE()     (atomic_set(&hdmi_fake_in, false))

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~extern declare~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern unsigned int gMutexID;

extern const HDMI_DRIVER *HDMI_GetDriver(void);
extern void HDMI_DBG_Init(void);
extern void DBG_OnTriggerHDMI(void);
extern void DBG_OnHDMIDone(void);

extern UINT32 DISP_GetScreenHeight(void);
extern UINT32 DISP_GetScreenWidth(void);

extern int disp_lock_mutex(void);
extern int disp_unlock_mutex(int id);
extern int disp_module_clock_on(DISP_MODULE_ENUM module, char *caller_name);
extern int disp_module_clock_off(DISP_MODULE_ENUM module, char *caller_name);

extern int hdmi_disc_disp_path(void);
extern int hdmi_conn_disp_path(void);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ---------------------------------------------------------------------------
//  function implement
// ---------------------------------------------------------------------------
int hdmi_rdma_address_config(bool enable, hdmi_video_buffer_info buffer_info);
static BOOL hdmi_drv_init_context(void);

/*debug code - enable HDMI_LOG and MMP profile*/
void hdmi_log_enable(int enable)
{
    printk("hdmi log %s\n", enable ? "enabled" : "disabled");
    hdmi_log_on = enable;
    hdmi_drv->log_enable(enable);
}

void hdmi_mmp_enable(int enable)
{
    printk("hdmi log %s\n", enable ? "enabled" : "disabled");
    hdmi_mmp_debug_enable(enable);
    hdmi_bufferdump_on = enable;
}

void init_hdmi_mmp_events(void)
{
    if (HDMI_MMP_Events.HDMI == 0)
    {
        HDMI_MMP_Events.HDMI = MMProfileRegisterEvent(MMP_RootEvent, "HDMI");
        HDMI_MMP_Events.OverlayDone = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "OverlayDone");
        HDMI_MMP_Events.DDPKBitblt = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "DDPKBitblt");
        HDMI_MMP_Events.SwitchRDMABuffer = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "SwitchRDMABuffer");
        HDMI_MMP_Events.SwitchOverlayBuffer = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "SwitchOverlayBuffer");
        HDMI_MMP_Events.WDMA1RegisterUpdated = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "WDMA1RegisterUpdated");
        HDMI_MMP_Events.WaitVSync = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "WaitVSync");
        HDMI_MMP_Events.StopOverlayBuffer = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "StopOverlayBuffer");
        HDMI_MMP_Events.RDMA1RegisterUpdated = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "RDMA1RegisterUpdated");

        HDMI_MMP_Events.FenceCreate = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "FenceCreate");
        HDMI_MMP_Events.BufferPost = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "BufferPost");
        HDMI_MMP_Events.BufferInsert = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "BufferInsert");
        HDMI_MMP_Events.BufferCfg = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "BufferCfg");
        HDMI_MMP_Events.BufferUsed = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "BufferUsed");
        HDMI_MMP_Events.BufferUpdate = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "BufferUpdate");
        HDMI_MMP_Events.BufferRemove = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "BufferRemove");
        HDMI_MMP_Events.FenceSignal = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "FenceSignal");
        HDMI_MMP_Events.HDMIState = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "HDMIState");
        HDMI_MMP_Events.GetDevInfo = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "GetDevInfo");
        HDMI_MMP_Events.ErrorInfo = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "ErrorInfo");
        HDMI_MMP_Events.MutexErr = MMProfileRegisterEvent(HDMI_MMP_Events.HDMI, "MutexErr");

        MMProfileEnableEventRecursive(HDMI_MMP_Events.HDMI, 1);
        MMProfileEnableEvent(HDMI_MMP_Events.DDPKBitblt, 0);
    }
}

int get_hdmi_support_info(void)
{
	int value = 0;
	
#ifdef USING_SCALE_ADJUSTMENT
	value |= HDMI_SCALE_ADJUSTMENT_SUPPORT;
#endif

#ifdef USING_ONE_RDMA
    value |= HDMI_ONE_RDMA_LIMITATION;
#endif

	HDMI_LOG("value is 0x%x\n", value);
	return value;
}
/*provide the debug method, emulate the mhl cable plug-in and plug-out*/
void hdmi_cable_fake_plug_in(void)
{
    SET_HDMI_FAKE_PLUG_IN();
    HDMI_LOG("[HDMIFake]Cable Plug In\n");

    if (p->is_force_disable == false)
    {
        if (IS_HDMI_STANDBY())
        {
            hdmi_resume();
            ///msleep(1000);
            switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
            hdmi_reschange = HDMI_VIDEO_RESOLUTION_NUM;
        }
    }
}

void hdmi_cable_fake_plug_out(void)
{
    SET_HDMI_NOT_FAKE();
    HDMI_LOG("[HDMIFake]Disable\n");

    if (p->is_force_disable == false)
    {
        if (IS_HDMI_ON())
        {
            if (hdmi_drv->get_state() != HDMI_STATE_ACTIVE)
            {
                hdmi_suspend();
                switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
            }
        }
    }
}

/* Will be called in LCD Interrupt handler to check whether HDMI is actived */
bool is_hdmi_active(void)
{
    return IS_HDMI_ON();
}

int get_hdmi_dev_info(HDMI_QUERY_TYPE type)
{
    switch (type)
    {
        case HDMI_CHARGE_CURRENT:
        {
            if ((p->is_enabled == false)
                || hdmi_params->cabletype == HDMI_CABLE)
            {
                return 0;
            }
            else if (hdmi_params->cabletype == MHL_CABLE)
            {
                return 500;
            }
            else if (hdmi_params->cabletype == MHL_2_CABLE)
            {
                return 900;
            }

        }
        default:
            return 0;
    }

}

/*provide the basic functions to get current time and sleep functions*/
static unsigned long get_current_time_us(void)
{
    struct timeval t;
    do_gettimeofday(&t);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

/*this kthread will configure the mva addres stored image to RDMA register*/
static int hdmi_rdma_config_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    for (;;)
    {
        wait_event_interruptible(hdmi_rdma_config_wq, atomic_read(&hdmi_rdma_config_event));
        atomic_set(&hdmi_rdma_config_event, 0);

        MMProfileLogEx(HDMI_MMP_Events.BufferCfg, MMProfileFlagStart, p->is_clock_on, 1);

        if (down_interruptible(&hdmi_update_mutex))
        {
            MMProfileLogEx(HDMI_MMP_Events.MutexErr, MMProfileFlagPulse, Mutex_Err1, Mutex_Err1);
            MMProfileLogEx(HDMI_MMP_Events.BufferCfg, MMProfileFlagEnd, p->is_clock_on, 1);
            HDMI_LOG("[HDMI] can't get semaphore in\n");
            continue; /// continue  return EAGAIN
        }

        if (p->is_clock_on == true) ///remove the first head here
        {
            hdmi_buffer_to_RDMA();
        }

        up(&hdmi_update_mutex);
        MMProfileLogEx(HDMI_MMP_Events.BufferCfg, MMProfileFlagEnd, p->is_clock_on, 1);

        if (kthread_should_stop())
        {
            break;
        }
    }

    return 0;
}

#ifdef MTK_HDMI_FENCE_SUPPORT
/*update buffer state or remove buffers from buffer list*/
static int hdmi_rdma_update_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    for (;;)
    {
        wait_event_interruptible(hdmi_rdma_update_wq, atomic_read(&hdmi_rdma_update_event));
        atomic_set(&hdmi_rdma_update_event, 0);

        MMProfileLogEx(HDMI_MMP_Events.BufferUpdate, MMProfileFlagStart, p->is_clock_on, 1);


        if (down_interruptible(&hdmi_update_mutex))
        {
            MMProfileLogEx(HDMI_MMP_Events.MutexErr, MMProfileFlagPulse, Mutex_Err1, Mutex_Err1);
            MMProfileLogEx(HDMI_MMP_Events.BufferUpdate, MMProfileFlagEnd, p->is_clock_on, 1);
            HDMI_LOG("[HDMI] can't get semaphore in\n");
            continue;
        }

        if (p->is_clock_on == true) ///remove the first head here
        {
            hdmi_buffer_state_update();
        }
        else
        {
            hdmi_remove_buffers();
        }

        up(&hdmi_update_mutex);
        MMProfileLogEx(HDMI_MMP_Events.BufferUpdate, MMProfileFlagEnd, p->is_clock_on, /*pUpdateList*/0);

        if (kthread_should_stop())
        {
            break;
        }

    }
}
#endif

#ifdef MTK_SMARTBOOK_SUPPORT
bool callback_plug = false;
struct timer_list smb_timer;
static bool smb_timer_started = false;

static int hdmi_status_update_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    for (;;)
    {
        wait_event_interruptible(hdmi_status_update_wq, atomic_read(&hdmi_status_update_event));
        atomic_set(&hdmi_status_update_event, 0);

        printk("[hdmi]%s, state = %d\n", __func__, callback_plug);

        if (callback_plug == false)
        {
            continue;
        }

        hdmi_drv->get_params(hdmi_params);
        if (hdmi_params->cabletype == MHL_SMB_CABLE)
        {
#ifdef CONFIG_HAS_SBSUSPEND
            sb_plug_in();
#endif
        }

        hdmi_resume();
        switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);

        printk("[hdmi]%s,  out!\n", __func__);

        if (kthread_should_stop())
        {
            break;
        }
    }

    return 0;
}

/* Smart book plug in callback*/
void smartbook_state_callback()
{
    printk("[hdmi]%s \n", __func__);

    if (smb_timer_started) {
        del_timer(&smb_timer);
        smb_timer_started = false;
    }
	
    atomic_set(&hdmi_status_update_event, 1);
    wake_up_interruptible(&hdmi_status_update_wq);

}

static void __smb_timer_isr(unsigned long n)
{
    HDMI_FUNC();
    del_timer(&smb_timer);

    smb_timer_started = false;
    smartbook_state_callback();
}

int hdmi_smb_notify_delay(int ms_latency)
{
    HDMI_FUNC();

    if (smb_timer_started == true)
    {
        return 0;
    }

    memset((void *)&smb_timer, 0, sizeof(smb_timer));
    smb_timer.expires = jiffies + msecs_to_jiffies(ms_latency);;
    smb_timer.function = __smb_timer_isr;
    init_timer(&smb_timer);
    add_timer(&smb_timer);

    smb_timer_started = true;
    return 0;
}
#endif

void hdmi_waitVsync(void)
{
    MMProfileLogEx(HDMI_MMP_Events.WaitVSync, MMProfileFlagStart, hdmi_vsync_cnt, p->is_clock_on);

    if (p->is_clock_on == false)
    {
        printk("[hdmi]:hdmi has suspend, return directly\n");
        msleep(19);
        MMProfileLogEx(HDMI_MMP_Events.WaitVSync, MMProfileFlagEnd, hdmi_vsync_cnt, p->is_clock_on);
        return;
    }

    hdmi_vsync_cnt++;

    hdmi_vsync_flag = 0;

    if (wait_event_interruptible_timeout(hdmi_vsync_wq, hdmi_vsync_flag, HZ / 10) == 0)
    {
        printk("[hdmi] Wait VSync timeout. early_suspend=%d\n", p->is_clock_on);
    }

    MMProfileLogEx(HDMI_MMP_Events.WaitVSync, MMProfileFlagEnd, hdmi_vsync_cnt, p->is_clock_on);
    hdmi_vsync_cnt--;
    return;
}
EXPORT_SYMBOL(hdmi_waitVsync);

int hdmi_rdma_address_config(bool enable, hdmi_video_buffer_info buffer_info)
{
    //HDMI_FUNC();

    if (enable)
    {
        if (p->is_clock_on == false)
        {
            HDMI_LOG(" clock stoped enable(%d), is_clock_on(%d)\n", enable, p->is_clock_on);
            return -1;
        }

        int rdmaInputFormat = 16;
        int rdmaInputsize = 3;

        if (buffer_info.src_fmt == MTK_FB_FORMAT_ARGB8888)
        {
            rdmaInputsize = 4;
            rdmaInputFormat = 16;
        }
        else if (buffer_info.src_fmt == MTK_FB_FORMAT_BGR888)
        {
            rdmaInputsize = 3;
            rdmaInputFormat = 8;
        }

        unsigned int offset = 0;///(buffer_info.src_pitch - buffer_info.src_width) / 2 * rdmaInputsize;
        unsigned int hdmiSourceAddr = (unsigned int)buffer_info.src_phy_addr
                                      + buffer_info.src_offset_y * buffer_info.src_pitch * rdmaInputsize
                                      + buffer_info.src_offset_x * rdmaInputsize + offset;

        struct disp_path_config_struct config = {0};

#if 0
        if ((buffer_info.src_height != ALIGN_TO(hdmi_height, 4))
            || (buffer_info.src_width != ALIGN_TO(hdmi_width, 4)))
        {
            HDMI_DPI_LOG("info: fmt %d, h %d, w %d, x_o %d, y_o %d, pitch %d hdmi_h %d -w %d\n", buffer_info.src_fmt, buffer_info.src_height, buffer_info.src_width,
                         buffer_info.src_offset_x, buffer_info.src_offset_y, buffer_info.src_pitch, hdmi_height, hdmi_width);
            return -1;
        }
#endif

        // Config RDMA1->DPI
        config.addr = hdmiSourceAddr;
        config.srcWidth = buffer_info.src_width;
        config.srcHeight = buffer_info.src_height;
        config.bgROI.width = buffer_info.src_width;
        config.bgROI.height = buffer_info.src_height;
        config.srcROI.width = buffer_info.src_width;
        config.srcROI.height = buffer_info.src_height;
        config.srcROI.x = 0;
        config.srcROI.y = 0;
        config.bgROI.x = 0;
        config.bgROI.y = 0;
        config.bgColor = 0x0;   // background color

        config.srcModule = DISP_MODULE_RDMA1;
        config.inFormat = rdmaInputFormat;
        config.pitch = buffer_info.src_pitch * 4;

        config.outFormat = RDMA_OUTPUT_FORMAT_ARGB;
        config.dstModule = HMID_DEST_DPI;

#if 1
        if ((hdmi_abs(buffer_info.src_height - p->hdmi_height) > 32)
            || (hdmi_abs(buffer_info.src_width - p->hdmi_width) > 32))
        {
            HDMI_LOG("info: fmt %d, h %d, w %d, x_o %d, y_o %d, pitch %d hdmi_h %d -w %d\n", buffer_info.src_fmt, buffer_info.src_height, buffer_info.src_width,
                     buffer_info.src_offset_x, buffer_info.src_offset_y, buffer_info.src_pitch, p->hdmi_height, p->hdmi_width);

            return -1;
        }
#endif
        bool need_config = true;
        if (dp_mutex_dst <= 0)
        {
            dp_mutex_dst = 2;
        }
        else
        {
            need_config = false;
        }

        if (true == need_config)
        {
            M4U_PORT_STRUCT m4uport;
            hdmi_m4u_port_config(&m4uport);
            disp_path_config_(&config, dp_mutex_dst); 

            hdmi_dpi_clk_enable(true);
			//hdmi_dpi_dump_register();
        }
		
        disp_path_get_mutex_(dp_mutex_dst);
        DISP_REG_SET(HDMI_RDMA_ADDR + DISP_REG_RDMA_MEM_START_ADDR, config.addr); 
        disp_path_release_mutex_(dp_mutex_dst);
        
    }
    else
    {
        if (-1 != dp_mutex_dst)
        {
            //FIXME: release mutex timeout
            HDMI_LOG("Stop RDMA1>DPI\n");
            disp_path_get_mutex_(dp_mutex_dst);

            RDMAStop(1);
            RDMAReset(1);
            disp_path_clear_config(dp_mutex_dst);
            disp_path_release_mutex_(dp_mutex_dst);

            dp_mutex_dst = -1;
        }
    }

    return 0;
}

/* Switch DPI Power for HDMI Driver */
/*static*/ void hdmi_dpi_power_switch(bool enable)
{
    int ret;
    HDMI_LOG("DPI clock:%d\n", enable);

    RET_VOID_IF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS);

    if (enable)
    {
        if (p->is_clock_on == true)
        {
            HDMI_LOG("power on request while already powered on!\n");
            return;
        }

        hdmi_dpi_power_enable(true, p->is_clock_on);
        p->is_clock_on = true;
    }
    else
    {
        if (p->is_clock_on == false)
        {
            HDMI_LOG("power off request while already powered off!\n");
        }

        bool clock_on = p->is_clock_on;
        p->is_clock_on = false;
        atomic_set(&hdmi_rdma_update_event, 1); ///release buffer
        wake_up_interruptible(&hdmi_rdma_update_wq);

        hdmi_dpi_power_enable(false, clock_on);
    }
}

/*this function set DPI clock polarity and hpw, hbp, hfp, vpw, vbp, vfp...*/
static void dpi_setting_res(u8 arg)
{
    _t_hdmi_context hdmi_context_unstatic;
    memcpy(&hdmi_context_unstatic, p, sizeof(_t_hdmi_context));

    hdmi_dpi_setting_res(arg, hdmi_params->cabletype, &hdmi_context_unstatic);

    p->bg_height   = hdmi_context_unstatic.bg_height;
    p->bg_width    = hdmi_context_unstatic.bg_width;
    p->hdmi_width  = hdmi_context_unstatic.hdmi_width;
    p->hdmi_height = hdmi_context_unstatic.hdmi_height;
    p->output_video_resolution = hdmi_context_unstatic.output_video_resolution;
}

/*functions to call mhl driver*/
void hdmi_set_mode(unsigned char ucMode)
{
    HDMI_FUNC();
    hdmi_drv->set_mode(ucMode);

    return;
}

void hdmi_reg_dump(void)
{
    hdmi_drv->dump();
}

void hdmi_write_reg(unsigned char u8Reg, unsigned char u8Data)
{
    hdmi_drv->write(u8Reg, u8Data);
}

void hdmi_read_reg(unsigned char u8Reg, unsigned int *p4Data)
{
    hdmi_read_reg_platform(u8Reg, p4Data);
}

/* Configure video attribute */
static int hdmi_video_config(HDMI_VIDEO_RESOLUTION vformat, HDMI_VIDEO_INPUT_FORMAT vin, HDMI_VIDEO_OUTPUT_FORMAT vout)
{
    HDMI_FUNC();
    RETIF(IS_HDMI_NOT_ON(), 0);


    return hdmi_drv->video_config(vformat, vin, vout);
}

/* Configure audio attribute, will be called by audio driver */
int hdmi_audio_config(int samplerate)
{
    HDMI_FUNC();
    RETIF(!p->is_enabled, 0);
    RETIF(IS_HDMI_NOT_ON(), 0);

    HDMI_LOG("sample rate=%d\n", samplerate);

    if (samplerate == 48000)
    {
        p->output_audio_format = HDMI_AUDIO_PCM_16bit_48000;
    }
    else if (samplerate == 44100)
    {
        p->output_audio_format = HDMI_AUDIO_PCM_16bit_44100;
    }
    else if (samplerate == 32000)
    {
        p->output_audio_format = HDMI_AUDIO_PCM_16bit_32000;
    }
    else
    {
        HDMI_LOG("samplerate not support:%d\n", samplerate);
    }


    hdmi_drv->audio_config(p->output_audio_format);

    return 0;
}

/* No one will use this function */
/*static*/ int hdmi_video_enable(bool enable)
{
    HDMI_FUNC();


    return hdmi_drv->video_enable(enable);
}

/* No one will use this function */
/*static*/ int hdmi_audio_enable(bool enable)
{
    HDMI_FUNC();


    return hdmi_drv->audio_enable(enable);
}

struct timer_list timer;
void __timer_isr(unsigned long n)
{
    HDMI_FUNC();

    if (hdmi_drv->audio_enable)
    {
        hdmi_drv->audio_enable(true);
    }

    del_timer(&timer);
}

int hdmi_audio_delay_mute(int latency)
{
    HDMI_FUNC();
    memset((void *)&timer, 0, sizeof(timer));
    timer.expires = jiffies + (latency * HZ / 1000);
    timer.function = __timer_isr;
    init_timer(&timer);
    add_timer(&timer);

    if (hdmi_drv->audio_enable)
    {
        hdmi_drv->audio_enable(false);
    }

    return 0;
}

/* Reset HDMI Driver state */
static void hdmi_state_reset(void)
{
    HDMI_FUNC();

    if (hdmi_drv->get_state() == HDMI_STATE_ACTIVE)
    {
        switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
        hdmi_reschange = HDMI_VIDEO_RESOLUTION_NUM;
    }
    else
    {
        switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
        switch_set_state(&hdmires_switch_data, 0);
    }
}

/*callback functions, used in hdmi_drv*/
static void hdmi_udelay(unsigned int us)
{
    udelay(us);
}

static void hdmi_mdelay(unsigned int ms)
{
    msleep(ms);
}

/* HDMI Driver state callback function */
void hdmi_state_callback(HDMI_STATE state)
{

    printk("[hdmi]%s, state = %d\n", __func__, state);

    RET_VOID_IF((p->is_force_disable == true));
    RET_VOID_IF(IS_HDMI_FAKE_PLUG_IN());

    switch (state)
    {
        case HDMI_STATE_NO_DEVICE:
        {
#ifdef MTK_SMARTBOOK_SUPPORT
            callback_plug = false;
#endif
            hdmi_suspend();
            switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE); 
            switch_set_state(&hdmires_switch_data, 0);
                    
#ifdef MTK_SMARTBOOK_SUPPORT
#ifdef CONFIG_HAS_SBSUSPEND

            if (hdmi_params->cabletype == MHL_SMB_CABLE)
            {
                int loop_cnt = 0;
                while(smarbook_enable) {
                    msleep(50);
                    if(++loop_cnt > 100) {
                        break;
                    }
                }
                sb_plug_out();
            }

            printk("[hdmi]%s, state = %d out!\n", __func__, state);
#endif
#endif
            break;
        }

        case HDMI_STATE_ACTIVE:
        {

#ifndef MTK_SMARTBOOK_SUPPORT

            hdmi_resume();

            //force update screen
            if (HDMI_OUTPUT_MODE_LCD_MIRROR == p->output_mode)
            {
                msleep(1000);
            }

            switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
          
#else
			

            callback_plug = true;
            hdmi_smb_notify_delay(3500);
#endif
            hdmi_reschange = HDMI_VIDEO_RESOLUTION_NUM;
            break;
        }

        case HDMI_STATE_PLUGIN_ONLY:
        {
            switch_set_state(&hdmi_switch_data, HDMI_STATE_PLUGIN_ONLY);

            break;
        }
        case HDMI_STATE_EDID_UPDATE:
        {
            switch_set_state(&hdmi_switch_data, HDMI_STATE_EDID_UPDATE);

            break;
        }
        case HDMI_STATE_CEC_UPDATE:
        {
            switch_set_state(&hdmi_switch_data, HDMI_STATE_CEC_UPDATE);

            break;
        }

        default:
        {
            printk("[hdmi]%s, state not support\n", __func__);
            break;
        }
    }

    return;
}

static void _rdma1_irq_handler(unsigned int param)
{
    RET_VOID_IF_NOLOG(!is_hdmi_active());

    if (param & 0x20) // taget line
    {
        MMProfileLogEx(HDMI_MMP_Events.RDMA1RegisterUpdated, MMProfileFlagPulse, param, 0x20);

        atomic_set(&hdmi_rdma_config_event, 1);
        wake_up_interruptible(&hdmi_rdma_config_wq);
    }

    if ((param & 2) && (hdmi_params->cabletype == MHL_SMB_CABLE)) // rdma1 register updated
    {
        MMProfileLogEx(HDMI_MMP_Events.RDMA1RegisterUpdated, MMProfileFlagPulse, param, 2);

        hdmi_vsync_flag = 1;
        wake_up_interruptible(&hdmi_vsync_wq);

    }

    if (param & 1) // rdma1 register updated
    {
        MMProfileLogEx(HDMI_MMP_Events.RDMA1RegisterUpdated, MMProfileFlagPulse, param, 1);
        atomic_set(&hdmi_rdma_update_event, 1);
        wake_up_interruptible(&hdmi_rdma_update_wq);

    }
}

/* Allocate memory, set M4U, LCD, MDP, DPI */
/* LCD overlay to memory -> MDP resize and rotate to memory -> DPI read to HDMI */
/* Will only be used in ioctl(MTK_HDMI_AUDIO_VIDEO_ENABLE) */
static HDMI_STATUS hdmi_drv_init(void)
{
    int lcm_width, lcm_height;

    HDMI_FUNC();

    RETIF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS, 0);

    p->hdmi_width = hdmi_get_width(hdmi_params->init_config.vformat);
    p->hdmi_height = hdmi_get_height(hdmi_params->init_config.vformat);
    p->bg_width = 0;
    p->bg_height = 0;

    lcm_width = DISP_GetScreenWidth();
    lcm_height = DISP_GetScreenHeight();
    HDMI_LOG("lcm_width=%d, lcm_height=%d\n", lcm_width, lcm_height);

    p->lcm_width = lcm_width;
    p->lcm_height = lcm_height;
    p->output_video_resolution = hdmi_params->init_config.vformat;
    p->output_audio_format = hdmi_params->init_config.aformat;
    p->scaling_factor = hdmi_params->scaling_factor < 10 ? hdmi_params->scaling_factor : 10;

    hdmi_dpi_config_clock(hdmi_params); // configure dpi clock
    hdmi_dpi_power_switch(false);   // but dpi power is still off

    disp_register_irq(HDMI_SRC_RDMA, _rdma1_irq_handler);

#ifdef MTK_HDMI_FENCE_SUPPORT
    if (!hdmi_rdma_config_task)
    {
        hdmi_rdma_config_task = kthread_create(hdmi_rdma_config_kthread, NULL, "hdmi_rdma_config_kthread");
        wake_up_process(hdmi_rdma_config_task);
    }

    if (!hdmi_rdma_update_task)
    {
        hdmi_rdma_update_task = kthread_create(hdmi_rdma_update_kthread, NULL, "hdmi_rdma_update_kthread");
        wake_up_process(hdmi_rdma_update_task);
    }
#endif

#ifdef MTK_SMARTBOOK_SUPPORT
    if (!hdmi_status_update_task)
    {
        hdmi_status_update_task = kthread_create(hdmi_status_update_kthread, NULL, "hdmi_status_update_kthread");
        wake_up_process(hdmi_status_update_task);
    }

#endif

    return HDMI_STATUS_OK;
}

/* Release memory */
/* Will only be used in ioctl(MTK_HDMI_AUDIO_VIDEO_ENABLE) */
static  HDMI_STATUS hdmi_drv_deinit(void)
{

    HDMI_FUNC();
    RETIF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS, 0);

    disp_unregister_irq(HDMI_SRC_RDMA, _rdma1_irq_handler);

    hdmi_dpi_power_switch(false);
    hdmi_dpi_free_irq();

    return HDMI_STATUS_OK;
}

/*this function power on mhl and set hdmi state, will be called by ioctl(MTK_HDMI_POWER_ON, MTK_HDMI_AUDIO_VIDEO_ENABLE)*/
/*static*/ void hdmi_power_on(void)
{
    HDMI_FUNC();

    RET_VOID_IF(IS_HDMI_NOT_OFF());

    if (down_interruptible(&hdmi_update_mutex))
    {
        printk("[hdmi][HDMI] can't get semaphore in %s()\n", __func__);
        return;
    }

    // Why we set power state before calling hdmi_drv->power_on()?
    // Because when power on, the hpd irq will come immediately, that means hdmi_resume will be called before hdmi_drv->power_on() retuen here.
    // So we have to ensure the power state is STANDBY before hdmi_resume() be called.
    SET_HDMI_STANDBY();

    hdmi_drv->power_on();

    // When camera is open, the state will only be changed when camera exits.
    // So we bypass state_reset here, if camera is open.
    // The related scenario is: suspend in camera with hdmi enabled.
    // Why need state_reset() here?
    // When we suspend the phone, and then plug out hdmi cable, the hdmi chip status will change immediately
    // But when we resume the phone and check hdmi status, the irq will never come again
    // So we have to reset hdmi state manually, to ensure the status is the same between the host and hdmi chip.
#ifndef MTK_MT8193_HDMI_SUPPORT
    if (p->is_force_disable == false)
    {
        if (IS_HDMI_FAKE_PLUG_IN())
        {
            //FixMe, deadlock may happened here, due to recursive use mutex
            hdmi_resume();
            msleep(1000);
            switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
            hdmi_reschange = HDMI_VIDEO_RESOLUTION_NUM;
        }
        else
        {
            hdmi_state_reset();
            // this is just a ugly workaround for some tv sets...
            //if(hdmi_drv->get_state() == HDMI_STATE_ACTIVE)
            //  hdmi_resume();
        }
    }
#endif
    up(&hdmi_update_mutex);
    return;
}

/*this function power off mhl/DPI and set hdmi state, will be called by ioctl(MTK_HDMI_POWER_ON, MTK_HDMI_AUDIO_VIDEO_ENABLE)*/
/*static*/ void hdmi_power_off(void)
{
    HDMI_FUNC();

    switch_set_state(&hdmires_switch_data, 0);

    RET_VOID_IF(IS_HDMI_OFF());

    if (down_interruptible(&hdmi_update_mutex))
    {
        printk("[hdmi][HDMI] can't get semaphore in %s()\n", __func__);
        return;
    }

    hdmi_drv->power_off();
    hdmi_dpi_power_switch(false);
    SET_HDMI_OFF();
    up(&hdmi_update_mutex);
    return;
}

/*static*/ void hdmi_suspend(void)
{
    HDMI_FUNC();
    RET_VOID_IF(IS_HDMI_NOT_ON());

    if (hdmi_bufferdump_on > 0)
    {
        MMProfileLogEx(HDMI_MMP_Events.HDMIState, MMProfileFlagStart, Plugout, 0);
    }

    if (down_interruptible(&hdmi_update_mutex))
    {
        printk("[hdmi][HDMI] can't get semaphore in %s()\n", __func__);
        return;
    }

    hdmi_dpi_power_switch(false);
    hdmi_video_buffer_info temp;
    memset(&temp, 0, sizeof(temp));
    hdmi_rdma_address_config(false, temp);

    hdmi_drv->suspend();
    SET_HDMI_STANDBY();

    disp_module_clock_off(HDMI_SRC_RDMA, "HDMI");
    up(&hdmi_update_mutex);
    if (hdmi_bufferdump_on > 0)
    {
        MMProfileLogEx(HDMI_MMP_Events.HDMIState, MMProfileFlagEnd, Plugout, 0);
    }

    hdmi_dpi_disable_MipiClk();
    hdmi_conn_disp_path();
    hdmi_config_main_disp();
}

/*static*/ void hdmi_resume(void)
{
    HDMI_FUNC();
    
    RET_VOID_IF(IS_HDMI_NOT_STANDBY());
    SET_HDMI_ON();

    hdmi_disc_disp_path();
    disp_path_clear_config(gMutexID);

    if (hdmi_bufferdump_on > 0)
    {
        MMProfileLogEx(HDMI_MMP_Events.HDMIState, MMProfileFlagStart, Plugin, 0);
    }

    if (down_interruptible(&hdmi_update_mutex))
    {
        printk("[hdmi][HDMI] can't get semaphore in %s()\n", __func__);
        return;
    }

    disp_module_clock_on(HDMI_SRC_RDMA, "HDMI");

    hdmi_dpi_power_switch(true);
    hdmi_drv->resume();
    up(&hdmi_update_mutex);
    if (hdmi_bufferdump_on > 0)
    {
        MMProfileLogEx(HDMI_MMP_Events.HDMIState, MMProfileFlagEnd, Plugin, 0);
    }
}

/* Set HDMI orientation, will be called in mtkfb_ioctl(SET_ORIENTATION) */
/*static*/ void hdmi_setorientation(int orientation)
{
    HDMI_FUNC();
    ///RET_VOID_IF(!p->is_enabled);

    if (down_interruptible(&hdmi_update_mutex))
    {
        printk("[hdmi][HDMI] can't get semaphore in %s\n", __func__);
        return;
    }

    p->orientation = orientation;
    p->is_reconfig_needed = true;

    //done:
    up(&hdmi_update_mutex);
}

static int hdmi_release(struct inode *inode, struct file *file)
{
    HDMI_FUNC();
    return 0;
}

static int hdmi_open(struct inode *inode, struct file *file)
{
    HDMI_FUNC();
    return 0;
}

void    MTK_HDMI_Set_Security_Output(int enable)
{
    return;
    RETIF(!p->is_enabled, 0);
    RETIF(IS_HDMI_OFF(), 0);

    if (enable)
    {
        if (hdmi_drv->get_state() == HDMI_STATE_ACTIVE)
        {
            hdmi_resume();
            msleep(1000);
            switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
        }
    }
    else
    {
        if (hdmi_drv->get_state() == HDMI_STATE_ACTIVE)
        {
            hdmi_suspend();
            switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
            switch_set_state(&hdmires_switch_data, 0);
        }
    }
}

static long hdmi_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;

    int r = 0;
    HDMI_LOG("[HDMI] hdmi ioctl= %s(%d), arg = %lu\n", _hdmi_ioctl_spy(cmd), cmd & 0xff, arg);

    switch (cmd)
    {
        case MTK_HDMI_AUDIO_VIDEO_ENABLE:
        {
            if (arg)
            {
                if (p->is_enabled)
                {
                    return 0;
                }

                HDMI_CHECK_RET(hdmi_drv_init());

                if (hdmi_drv->enter)
                {
                    hdmi_drv->enter();
                }

                hdmi_power_on();
                p->is_enabled = true;
            }
            else
            {
                if (!p->is_enabled)
                {
                    return 0;
                }

                //when disable hdmi, HPD is disabled
                switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);

                //wait hdmi finish update
                if (down_interruptible(&hdmi_update_mutex))
                {
                    printk("[hdmi][HDMI] can't get semaphore in %s()\n", __func__);
                    return -EFAULT;
                }

                hdmi_video_buffer_info temp;
                hdmi_rdma_address_config(false, temp);
                up(&hdmi_update_mutex);
                hdmi_power_off();

                //wait hdmi finish update
                if (down_interruptible(&hdmi_update_mutex))
                {
                    printk("[hdmi][HDMI] can't get semaphore in %s()\n", __func__);
                    return -EFAULT;
                }

                HDMI_CHECK_RET(hdmi_drv_deinit());
                p->is_enabled = false;
                up(&hdmi_update_mutex);

                if (hdmi_drv->exit)
                {
                    hdmi_drv->exit();
                }
            }

            break;
        }
        case MTK_HDMI_FORCE_FULLSCREEN_ON:
        case MTK_HDMI_FORCE_CLOSE:
        {
            RETIF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS, 0);
            RETIF(!p->is_enabled, 0);
            RETIF(IS_HDMI_OFF(), 0);

            if ((p->is_force_disable == true) || (hdmi_params->cabletype == MHL_SMB_CABLE))
            {
                break;
            }

            if (IS_HDMI_FAKE_PLUG_IN())
            {
                hdmi_suspend();
                switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
                switch_set_state(&hdmires_switch_data, 0);
            }
            else
            {
                if (hdmi_drv->get_state() == HDMI_STATE_ACTIVE)
                {
                    hdmi_suspend();
                    switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
                    switch_set_state(&hdmires_switch_data, 0);
                }
            }

            p->is_force_disable = true;

            break;
        }
        case MTK_HDMI_FORCE_FULLSCREEN_OFF:
        case MTK_HDMI_FORCE_OPEN:
        {
            RETIF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS, 0);
            RETIF(!p->is_enabled, 0);
            RETIF(IS_HDMI_OFF(), 0);

            if ((p->is_force_disable == false) || (hdmi_params->cabletype == MHL_SMB_CABLE))
            {
                break;
            }

            if (IS_HDMI_FAKE_PLUG_IN())
            {
                hdmi_resume();
                msleep(1000);
                switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
                hdmi_reschange = HDMI_VIDEO_RESOLUTION_NUM;
            }
            else
            {
                if (hdmi_drv->get_state() == HDMI_STATE_ACTIVE)
                {
                    hdmi_resume();
                    msleep(1000);
                    switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
                    hdmi_reschange = HDMI_VIDEO_RESOLUTION_NUM;
                }
            }

            p->is_force_disable = false;

            break;
        }

        /* Shutdown thread(No matter IPO), system suspend/resume will go this way... */
        case MTK_HDMI_POWER_ENABLE:
        {
            RETIF(!p->is_enabled, 0);

            if (arg)
            {
                RETIF(otg_enable_status, 0);
                hdmi_power_on();
            }
            else
            {
                hdmi_power_off();
                hdmi_video_buffer_info temp;
                memset(&temp, 0, sizeof(temp));
                hdmi_rdma_address_config(false, temp);
                switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
            }

            break;
        }

        case MTK_HDMI_USBOTG_STATUS:
        {
            HDMI_LOG("MTK_HDMI_USBOTG_STATUS, arg=%d, enable %d\n", arg, p->is_enabled);

            RETIF(!p->is_enabled, 0);
            RETIF((hdmi_params->cabletype != MHL_CABLE), 0);

            if (arg)
            {
                otg_enable_status = true;
            }
            else
            {
                otg_enable_status = false;
                RETIF(p->is_force_disable, 0);
                hdmi_power_on();

            }

            break;
        }

        case MTK_HDMI_AUDIO_ENABLE:
        {
            RETIF(!p->is_enabled, 0);

            if (arg)
            {
                HDMI_CHECK_RET(hdmi_audio_enable(true));
            }
            else
            {
                HDMI_CHECK_RET(hdmi_audio_enable(false));
            }

            break;
        }

        case MTK_HDMI_VIDEO_ENABLE:
        {
            RETIF(!p->is_enabled, 0);
            break;
        }

        case MTK_HDMI_VIDEO_CONFIG:
        {
            HDMI_LOG("video resolution configuration, arg=%ld, %d\n", arg, hdmi_reschange);

            RETIF(!p->is_enabled, 0);
            RETIF(IS_HDMI_NOT_ON(), 0);

            if (hdmi_reschange == arg)
            {
                HDMI_LOG("hdmi_reschange=%ld\n", hdmi_reschange);
                break;
            }

            hdmi_reschange = arg;
            p->is_clock_on = false;
            atomic_set(&hdmi_rdma_update_event, 1);
            wake_up_interruptible(&hdmi_rdma_update_wq);
            int tmp = 0;

            while (1)
            {
                if (hdmi_buffer_list_empty() || (tmp > 15))
                {
                    if (tmp > 15)
                    {
                        HDMI_LOG(" Error HDMI_Buffer_List is not empty\n");
                    }

                    break;
                }
                else
                {
                    msleep(2);
                }

                tmp++;
            }

            RETIF(!p->is_enabled, 0);
            RETIF(IS_HDMI_NOT_ON(), 0);

            if (hdmi_bufferdump_on > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.HDMIState, MMProfileFlagStart, ResChange, arg);
            }

            if (down_interruptible(&hdmi_update_mutex))
            {
                HDMI_LOG("[HDMI] can't get semaphore in\n");
                return -EFAULT;
            }

            hdmi_video_buffer_info temp;
            hdmi_rdma_address_config(false, temp);

            hdmi_config_pll(arg);
            dpi_setting_res((u8)arg);
            hdmi_video_config(p->output_video_resolution, HDMI_VIN_FORMAT_RGB888, HDMI_VOUT_FORMAT_RGB888);

            RDMASetTargetLine(1, p->hdmi_height * 4 / 5);

            hdmi_dpi_clk_enable(false);
            hdmi_dpi_config_hdmi();
            hdmi_dpi_config_bg(true, p->bg_width, p->bg_height);
            hdmi_dpi_config_update(p->hdmi_width, p->hdmi_height);
            hdmi_dpi_config_DualEdge(true);
            //hdmi_dpi_colorbar_enable(true);
			//hdmi_dpi_clk_enable(true);
            //hdmi_dpi_dump_register();

            up(&hdmi_update_mutex);

            p->is_clock_on = true;

            if (factory_mode == false)
            {
                switch_set_state(&hdmires_switch_data, hdmi_reschange + 1);
            }

            if (hdmi_bufferdump_on > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.HDMIState, MMProfileFlagEnd, ResChange, hdmi_reschange + 1);
            }

            HDMI_LOG("video resolution (%d) configuration done\n", hdmi_reschange + 1);

            break;
        }
        case MTK_HDMI_AUDIO_CONFIG:
        {
            RETIF(!p->is_enabled, 0);

            break;
        }
        case MTK_HDMI_IS_FORCE_AWAKE:
        {
            if (!hdmi_drv_init_context())
            {
                printk("[hdmi]%s, hdmi_drv_init_context fail\n", __func__);
                return HDMI_STATUS_NOT_IMPLEMENTED;
            }

            r = copy_to_user(argp, &hdmi_params->is_force_awake, sizeof(hdmi_params->is_force_awake)) ? -EFAULT : 0;
            break;
        }
#if 1
        case MTK_HDMI_ENTER_VIDEO_MODE:
        {
            RETIF(!p->is_enabled, 0);
            RETIF(HDMI_OUTPUT_MODE_VIDEO_MODE != p->output_mode, 0);
            //FIXME
            break;
        }
        case MTK_HDMI_REGISTER_VIDEO_BUFFER:
        {
#if 0
            struct hdmi_video_buffer_info video_buffer_info;
            RETIF(!p->is_enabled, 0);
            RETIF(HDMI_OUTPUT_MODE_VIDEO_MODE != p->output_mode, 0);

            if (copy_from_user(&video_buffer_info, (void __user *)argp, sizeof(video_buffer_info)))
            {
                HDMI_LOG("copy_from_user failed! line\n");
                r = -EFAULT;
                break;
            }

            if (video_buffer_info.src_vir_addr == 0)
            {
                HDMI_LOG("[Error]HDMI_REGISTER_VIDEO_BUFFER VA should not be NULL\n");
                break;
            }

            if (down_interruptible(&hdmi_video_mode_mutex))
            {
                HDMI_LOG("[HDMI] can't get semaphore in\n");
                break;
            }

            if (hdmi_add_video_buffer(&video_buffer_info, file) < 0)
            {
                r = -ENOMEM;
            }

            up(&hdmi_video_mode_mutex);
#endif
            break;

        }
        case MTK_HDMI_POST_VIDEO_BUFFER:
        {
            hdmi_video_buffer_info video_buffer_info;
            video_buffer_info.src_fmt = MTK_FB_FORMAT_ARGB8888;

            ///struct hdmi_video_buffer_list *buffer_list;
            if ((p->is_enabled == false) || (p->is_clock_on == false) || IS_HDMI_NOT_ON())
            {
                MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, insert_Buffer_Err1, insert_Buffer_Err1);
                RETIF(!p->is_enabled, 0);
                RETIF(!p->is_clock_on, -1);
                RETIF(IS_HDMI_NOT_ON(), 0);
            }

            if (copy_from_user(&video_buffer_info, (void __user *)argp, sizeof(video_buffer_info)))
            {
                MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, insert_Buffer_Err2, insert_Buffer_Err2);
                HDMI_LOG("copy_from_user failed! line\n");
                r = -EFAULT;
                break;
            }

            if (hdmi_bufferdump_on > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.BufferPost, MMProfileFlagStart, p->is_enabled, p->is_clock_on);
            }

            DBG_OnTriggerHDMI();

            if (hdmi_post_buffer(&video_buffer_info, p->is_clock_on) < 0)
            {
                r = -EFAULT;
                break;
            }

            if (p->is_clock_on)
            {
                if (dp_mutex_dst <= 0)
                {
                    atomic_set(&hdmi_rdma_config_event, 1);
                    wake_up_interruptible(&hdmi_rdma_config_wq);
                }

                MMP_MetaDataBitmap_t Bitmap;
                if ((capture_screen == true) || (hdmi_bufferdump_on >= 0x7))
                {
                    Bitmap.data1 = video_buffer_info.src_width;
                    Bitmap.data2 = video_buffer_info.src_pitch;
                    Bitmap.width = video_buffer_info.src_width;
                    Bitmap.height = video_buffer_info.src_height;
                    Bitmap.format = MMProfileBitmapBGRA8888;
                    Bitmap.start_pos = 0;// (video_buffer_info.src_pitch - video_buffer_info.src_width) / 2 * 4;
                    Bitmap.pitch = video_buffer_info.src_pitch * 4;

                    Bitmap.data_size = Bitmap.pitch * Bitmap.height;
                    Bitmap.down_sample_x = mmp_image_scale;
                    Bitmap.down_sample_y = mmp_image_scale;
                    Bitmap.pData = video_buffer_info.src_base_addr;
                    Bitmap.bpp = 32;
                }

                if (hdmi_bufferdump_on >= 0x7)
                {
                    MMProfileLogMetaBitmap(HDMI_MMP_Events.DDPKBitblt, MMProfileFlagPulse, &Bitmap);
                }

                if (capture_screen == true)
                {
                    memcpy(capture_addr, video_buffer_info.src_base_addr, Bitmap.data_size);
                    capture_screen = false;
                }
            }
#ifndef MTK_HDMI_FENCE_SUPPORT
            HDMI_LOG("wait hdmi_rdma_config_wq \n");
            wait_event_interruptible(hdmi_rdma_config_wq, atomic_read(&hdmi_rdma_config_event));
            atomic_set(&hdmi_rdma_config_event, 0);
#endif

            //disp_dump_reg(DISP_MODULE_RDMA);
            HDMI_LOG("MTK_HDMI_POST_VIDEO_BUFFER done\n");
            break;
        }
        case MTK_HDMI_LEAVE_VIDEO_MODE:
        {
            RETIF(!p->is_enabled, 0);
            break;
        }

#endif

        case MTK_HDMI_FACTORY_MODE_ENABLE:
        {
            if (hdmi_drv->power_on())
            {
                r = -EAGAIN;
                HDMI_LOG("Error factory mode test fail\n");
            }
            else
            {
                HDMI_LOG("before power off\n");
                hdmi_drv->power_off();
                HDMI_LOG("after power off\n");
            }

            break;
        }

        case MTK_HDMI_FACTORY_GET_STATUS:
        {
            bool hdmi_status = false;

            if (p->is_clock_on == true)
            {
                hdmi_status = true;
            }

            int ret = check_edid_header();
            if (ret >= 0)
            {
                hdmi_status &= ret;
            }

            HDMI_LOG("MTK_HDMI_FACTORY_GET_STATUS is %d \n", p->is_clock_on);

            if (copy_to_user((void __user *)arg, &hdmi_status, sizeof(hdmi_status)))
            {
                HDMI_LOG("copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }

            break;
        }

        case MTK_HDMI_FACTORY_DPI_TEST:
        {
            if (down_interruptible(&hdmi_update_mutex))
            {
                HDMI_LOG("[HDMI] can't get semaphore in\n");
                return EAGAIN;
            }

            hdmi_dpi_config_update(p->hdmi_width, p->hdmi_height);
            hdmi_dpi_config_DualEdge(true);
            hdmi_dpi_colorbar_enable(true);
            hdmi_dpi_clk_enable(true);
            hdmi_dpi_dump_register();

            up(&hdmi_update_mutex);
            break;
        }

        case MTK_HDMI_GET_DEV_INFO:
        {
            int displayid = 0;

            if (hdmi_bufferdump_on > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagStart, p->is_enabled, p->is_clock_on);
            }

            HDMI_LOG("DEV_INFO configuration get + \n");

            if (copy_from_user(&displayid, (void __user *)arg, sizeof(displayid)))
            {
                if (hdmi_bufferdump_on > 0)
                {
                    MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagEnd, 0xff, 0xff1);
                }

                HDMI_LOG(": copy_from_user failed! line:%d \n", __LINE__);
                return -EAGAIN;
            }

            if (displayid != MTKFB_DISPIF_HDMI)
            {
                if (hdmi_bufferdump_on > 0)
                {
                    MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagPulse, 0xff, 0xff2);
                }

                HDMI_LOG(": invalid display id:%d \n", displayid);
                ///return -EAGAIN;
            }

            mtk_dispif_info_t hdmi_info;
            memset(&hdmi_info, 0, sizeof(hdmi_info));
            hdmi_info.displayFormat = DISPIF_FORMAT_RGB888;
            hdmi_info.displayHeight = p->hdmi_height;
            hdmi_info.displayWidth = p->hdmi_width;
            hdmi_info.display_id = displayid;
            hdmi_info.isConnected = 1;
            hdmi_info.displayMode = DISPIF_MODE_COMMAND;

            if (hdmi_params->cabletype == MHL_SMB_CABLE)
            {
                hdmi_info.displayType = HDMI_SMARTBOOK;
                if (IS_HDMI_OFF())
                {
                    hdmi_info.displayType = MHL;
                }
            }
            else if (hdmi_params->cabletype == MHL_CABLE)
            {
                hdmi_info.displayType = MHL;
            }
            else
            {
                hdmi_info.displayType = HDMI;
            }

            hdmi_info.isHwVsyncAvailable = 1;
            hdmi_info.vsyncFPS = 60;

            if (copy_to_user((void __user *)arg, &hdmi_info, sizeof(hdmi_info)))
            {
                if (hdmi_bufferdump_on > 0)
                {
                    MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagEnd, 0xff, 0xff2);
                }

                HDMI_LOG("copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }

            if (hdmi_bufferdump_on > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagEnd, p->is_enabled, hdmi_info.displayType);
            }

            HDMI_LOG("DEV_INFO configuration get displayType-%d \n", hdmi_info.displayType);

            break;
        }

        case MTK_HDMI_PREPARE_BUFFER:
        {
            hdmi_buffer_info hdmi_buffer;

#ifdef MTK_HDMI_FENCE_SUPPORT
            struct fence_data data;

            if (hdmi_bufferdump_on > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.FenceCreate, MMProfileFlagStart, 0, 0);
            }
            if (copy_from_user(&hdmi_buffer, (void __user *)arg, sizeof(hdmi_buffer)))
            {
                printk("[HDMI]: copy_from_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
                break;
            }
            if (down_interruptible(&hdmi_update_mutex))
            {
                MMProfileLogEx(HDMI_MMP_Events.MutexErr, MMProfileFlagPulse, Mutex_Err2, Mutex_Err2);
                HDMI_LOG("[HDMI] Warning can't get semaphore in\n");
                r = -EFAULT;
                break;
            }
            hdmi_buffer.fence_fd = MTK_HDMI_NO_FENCE_FD;

            if (p->is_clock_on)
            {
                if (hdmi_insert_buffer(&hdmi_buffer) < 0)
                {
                    r = -EFAULT;
                    up(&hdmi_update_mutex);
                    break;
                }
            }
            else
            {
                MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Fence_Err, 0);
            }

            up(&hdmi_update_mutex);
            if (copy_to_user((void __user *)arg, &hdmi_buffer, sizeof(hdmi_buffer)))
            {
                HDMI_LOG(": copy_to_user error! line:%d \n", __LINE__);
                r = -EFAULT;
            }
#endif
            break;
        }
        case MTK_HDMI_SCREEN_CAPTURE:
        {
            int capture_wait_times = 0;
            capture_screen = true;

            if (copy_from_user(&capture_addr, (void __user *)arg, sizeof(capture_addr)))
            {
                HDMI_LOG(": copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }

            while (capture_wait_times < 3)
            {
                msleep(20);
                capture_wait_times++;

                if (capture_screen == false)
                {
                    break;
                }
            }

            if (capture_screen == true)
            {
                HDMI_LOG("capture scree fail,is_enabled(%d), wait_times(%d)\n", p->is_clock_on, capture_wait_times);
            }
            else
            {
                HDMI_LOG("screen_capture done,is_enabled(%d), wait_times(%d)\n", capture_wait_times);
            }

            capture_screen = false;
            break;
        }
        case MTK_HDMI_GET_CAPABILITY:
        {
            int query_type = 0;
            if (copy_from_user(&query_type, (void __user *)arg, sizeof(int)))
            {
                HDMI_LOG(": copy_to_user failed! line:%d \n", __LINE__);
                r = -EFAULT;
            }
			query_type = get_hdmi_support_info();

            if (copy_to_user((void __user *)arg, &query_type, sizeof(query_type)))
            {
                HDMI_LOG(": copy_to_user error! line:%d \n", __LINE__);
                r = -EFAULT;
            }
            HDMI_LOG("[hdmi][HDMI] query_type  done %x\n", query_type);                

			break;
        }
        default:
        {
            r = hdmi_ioctl_platform(cmd, arg);
            break;
        }
    }

    return r;
}


static int hdmi_remove(struct platform_device *pdev)
{
    return 0;
}

/*register callback to sbsuspend to control backlight*/
static void sb_backlight_turnoff()
{
    printk("[Donglei][hdmi] turn off backlight.\n");
    mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_OFF);
    hdmi_is_active = TRUE;
    msleep(50);
}

static void sb_backlight_turnon()
{
    printk("[Donglei][hdmi] turn on backlight.\n");
    hdmi_is_active = FALSE;
    mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_HALF);
}

static void hdmi_sb_enable()
{
    printk("[Donglei][hdmi] sb_enable.\n");
    smarbook_enable = true;
}

static void hdmi_sb_disable()
{
    printk("[Donglei][hdmi] sb_disable.\n");
    smarbook_enable = false;
}

#ifdef MTK_SMARTBOOK_SUPPORT
#ifdef CONFIG_HAS_SBSUSPEND
static struct sb_handler sb_backlight_control_desc = {
    .level    = SB_LEVEL_DISABLE_TOUCH,
    .plug_in  = sb_backlight_turnoff,
    .plug_out = sb_backlight_turnon,
    .enable   = hdmi_sb_enable,
    .disable  = hdmi_sb_disable,
};
#endif
#endif

static BOOL hdmi_drv_init_context(void)
{
    static const HDMI_UTIL_FUNCS hdmi_utils =
    {
        .udelay                 = hdmi_udelay,
        .mdelay                 = hdmi_mdelay,
        .state_callback         = hdmi_state_callback,
    };

    if (hdmi_drv != NULL)
    {
        return TRUE;
    }


    hdmi_drv = (HDMI_DRIVER *)HDMI_GetDriver();

    if (NULL == hdmi_drv)
    {
        return FALSE;
    }

    hdmi_drv->set_util_funcs(&hdmi_utils);
    hdmi_drv->get_params(hdmi_params);
    deliver_driver_interface(hdmi_drv);

    return TRUE;
}

static void __exit hdmi_exit(void)
{

#ifdef MTK_HDMI_FENCE_SUPPORT
    hdmi_sync_destroy();
#endif
    device_destroy(hdmi_class, hdmi_devno);
    class_destroy(hdmi_class);
    cdev_del(hdmi_cdev);
    unregister_chrdev_region(hdmi_devno, 1);

#ifdef MTK_SMARTBOOK_SUPPORT
#ifdef CONFIG_HAS_SBSUSPEND
	unregister_sb_handler(&sb_backlight_control_desc);
#endif
#endif

}

struct file_operations hdmi_fops =
{
    .owner   = THIS_MODULE,
    .unlocked_ioctl   = hdmi_ioctl,
    .open    = hdmi_open,
    .release = hdmi_release,
};

static int hdmi_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct class_device *class_dev = NULL;

    printk("[hdmi]%s\n", __func__);

    /* Allocate device number for hdmi driver */
    ret = alloc_chrdev_region(&hdmi_devno, 0, 1, HDMI_DEVNAME);

    if (ret)
    {
        printk("[hdmi]alloc_chrdev_region fail\n");
        return -1;
    }

    /* For character driver register to system, device number binded to file operations */
    hdmi_cdev = cdev_alloc();
    hdmi_cdev->owner = THIS_MODULE;
    hdmi_cdev->ops = &hdmi_fops;
    ret = cdev_add(hdmi_cdev, hdmi_devno, 1);

    /* For device number binded to device name(hdmitx), one class is corresponeded to one node */
    hdmi_class = class_create(THIS_MODULE, HDMI_DEVNAME);
    /* mknod /dev/hdmitx */
    class_dev = (struct class_device *)device_create(hdmi_class, NULL, hdmi_devno, NULL, HDMI_DEVNAME);

    printk("[hdmi][%s] current=0x%08x\n", __func__, (unsigned int)current);

    if (!hdmi_drv_init_context())
    {
        printk("[hdmi]%s, hdmi_drv_init_context fail\n", __func__);
        return HDMI_STATUS_NOT_IMPLEMENTED;
    }

    init_waitqueue_head(&hdmi_rdma_config_wq);
    init_waitqueue_head(&hdmi_rdma_update_wq);

    init_waitqueue_head(&hdmi_vsync_wq);

#ifdef MTK_SMARTBOOK_SUPPORT
    init_waitqueue_head(&hdmi_status_update_wq);
#endif
    return 0;
}

static struct platform_driver hdmi_driver =
{
    .probe  = hdmi_probe,
    .remove = hdmi_remove,
    .driver = { .name = HDMI_DEVNAME }
};

static int __init hdmi_init(void)
{
    int ret = 0;
    printk("[hdmi]%s\n", __func__);


    if (platform_driver_register(&hdmi_driver))
    {
        printk("[hdmi]failed to register mtkfb driver\n");
        return -1;
    }

    memset((void *)&hdmi_context, 0, sizeof(_t_hdmi_context));
    SET_HDMI_OFF();

    init_hdmi_mmp_events();

    if (!hdmi_drv_init_context())
    {
        printk("[hdmi]%s, hdmi_drv_init_context fail\n", __func__);
        return HDMI_STATUS_NOT_IMPLEMENTED;
    }

    p->output_mode = hdmi_params->output_mode;
    p->orientation = 0;
    hdmi_drv->init();
    HDMI_LOG("Output mode is %s\n", (hdmi_params->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS) ? "HDMI_OUTPUT_MODE_DPI_BYPASS" : "HDMI_OUTPUT_MODE_LCD_MIRROR");

    if (hdmi_params->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS)
    {
        p->output_video_resolution = HDMI_VIDEO_RESOLUTION_NUM;
    }

    HDMI_DBG_Init();

    hdmi_switch_data.name = "hdmi";
    hdmi_switch_data.index = 0;
    hdmi_switch_data.state = NO_DEVICE;

    // for support hdmi hotplug, inform AP the event
    ret = switch_dev_register(&hdmi_switch_data);

    hdmires_switch_data.name = "res_hdmi";
    hdmires_switch_data.index = 0;
    hdmires_switch_data.state = 0;

    // for support hdmi hotplug, inform AP the event
    ret = switch_dev_register(&hdmires_switch_data);

    if (ret)
    {
        printk("[hdmi][HDMI]switch_dev_register returned:%d!\n", ret);
        return 1;
    }

#ifdef MTK_HDMI_FENCE_SUPPORT
    hdmi_sync_init();
    hdmi_buffer_list_init();
#endif
    int tmp_boot_mode = get_boot_mode();

    if ((tmp_boot_mode == FACTORY_BOOT) || (tmp_boot_mode == ATE_FACTORY_BOOT))
    {
        factory_mode = true;
    }

#ifdef MTK_SMARTBOOK_SUPPORT
#ifdef CONFIG_HAS_SBSUSPEND
	register_sb_handler(&sb_backlight_control_desc);
#endif
#endif

    return 0;
}

module_init(hdmi_init);
module_exit(hdmi_exit);
MODULE_AUTHOR("Xuecheng, Zhang <xuecheng.zhang@mediatek.com>");
MODULE_DESCRIPTION("HDMI Driver");
MODULE_LICENSE("GPL");

#endif
