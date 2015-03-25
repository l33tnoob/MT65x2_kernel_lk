/*****************************************************************************/
/* Copyright (c) 2009 NXP Semiconductors BV                                  */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation, using version 2 of the License.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307       */
/* USA.                                                                      */
/*                                                                           */
/*****************************************************************************/
#if defined(MTK_HDMI_SUPPORT)
#define TMFL_TDA19989

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
#include <linux/list.h>
#include <linux/switch.h>
#ifdef MTK_SMARTBOOK_SUPPORT
#include <linux/sbsuspend.h>
#endif
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
#include <mach/mt_clkmgr.h>
#include <mach/mt_boot.h>

#include "mtkfb_info.h"
#include "hdmitx.h"
#include "hdmi_drv.h"
#include "hdmi_utils.h"

#include "dpi_reg.h"
#include "mach/eint.h"
#include "mach/irqs.h"

#include "disp_drv_platform.h"
#include "ddp_reg.h"
#include "mtkfb.h"
#include "dpi_drv.h"
#ifdef MTK_SMARTBOOK_SUPPORT
#include "smartbook.h"
#endif

#ifdef I2C_DBG
#include "tmbslHdmiTx_types.h"
#include "tmbslTDA9989_local.h"
#endif

#define MTK_HDMI_MULTI_DISPLAY_SUPPORT
#define MTK_HDMI_FENCE_SUPPORT
#define MTK_HDMI_ION_SUPPORT

#define HDMI_DEVNAME "hdmitx"

#undef OUTREG32
#define OUTREG32(x, y) {/*printk("[hdmi]write 0x%08x to 0x%08x\n", (y), (x)); */__OUTREG32((x),(y))}
#define __OUTREG32(x,y) {*(unsigned int*)(x)=(y);}

#define RETIF(cond, rslt)       if ((cond)){HDMI_LOG("return in %d\n",__LINE__);return (rslt);}
#define RET_VOID_IF(cond)       if ((cond)){HDMI_LOG("return in %d\n",__LINE__);return;}
#define RETIF_NOLOG(cond, rslt)       if ((cond)){return (rslt);}
#define RET_VOID_IF_NOLOG(cond)       if ((cond)){return;}
#define RETIFNOT(cond, rslt)    if (!(cond)){HDMI_LOG("return in %d\n",__LINE__);return (rslt);}


#define HDMI_DPI(suffix)        DPI  ## suffix
#define HMID_DEST_DPI           DISP_MODULE_DPI0
static int hdmi_bpp = 4;


#ifdef MTK_HDMI_FENCE_SUPPORT
// Fence Sync Object
#include "mtk_sync.h"
spinlock_t hdmi_lock;
DEFINE_SPINLOCK(hdmi_lock);
#endif

static bool factory_mode = false;

#define ALIGN_TO(x, n)  \
    (((x) + ((n) - 1)) & ~((n) - 1))
#define hdmi_abs(a) (((a) < 0) ? -(a) : (a))

extern const HDMI_DRIVER *HDMI_GetDriver(void);
extern void HDMI_DBG_Init(void);

extern UINT32 DISP_GetScreenHeight(void);
extern UINT32 DISP_GetScreenWidth(void);
extern BOOL DISP_IsVideoMode(void);
extern int disp_lock_mutex(void);
extern int disp_unlock_mutex(int id);
extern int disp_module_clock_on(DISP_MODULE_ENUM module, char *caller_name);
extern int disp_module_clock_off(DISP_MODULE_ENUM module, char *caller_name);

static int hdmi_log_on = 1;
static int hdmi_bufferdump_on = 1;
static unsigned long hdmi_reschange = HDMI_VIDEO_RESOLUTION_NUM;

#if defined (MTK_HDMI_ION_SUPPORT)
#include <linux/ion_drv.h>

static struct ion_client *ion_client;
#endif

static struct switch_dev hdmi_switch_data;
static struct switch_dev hdmires_switch_data;
HDMI_PARAMS _s_hdmi_params = {0};
HDMI_PARAMS *hdmi_params = &_s_hdmi_params;
static HDMI_DRIVER *hdmi_drv = NULL;

void hdmi_log_enable(int enable)
{
    printk("hdmi log %s\n", enable ? "enabled" : "disabled");
    hdmi_log_on = enable;
    hdmi_drv->log_enable(enable);
}

void hdmi_mmp_enable(int enable)
{
    printk("hdmi log %s\n", enable ? "enabled" : "disabled");
    hdmi_bufferdump_on = enable;
}

static DEFINE_SEMAPHORE(hdmi_update_mutex);
typedef struct
{
    bool is_reconfig_needed;    // whether need to reset HDMI memory
    bool is_enabled;    // whether HDMI is enabled or disabled by user
    bool is_force_disable;      //used for camera scenario.
    bool is_clock_on;   // DPI is running or not
    atomic_t state; // HDMI_POWER_STATE state
    int     lcm_width;  // LCD write buffer width
    int     lcm_height; // LCD write buffer height
    bool    lcm_is_video_mode;
    int     hdmi_width; // DPI read buffer width
    int     hdmi_height; // DPI read buffer height
    int     bg_width; // DPI read buffer width
    int     bg_height; // DPI read buffer height
    HDMI_VIDEO_RESOLUTION       output_video_resolution;
    HDMI_AUDIO_FORMAT           output_audio_format;
    int     orientation;    // MDP's orientation, 0 means 0 degree, 1 means 90 degree, 2 means 180 degree, 3 means 270 degree
    HDMI_OUTPUT_MODE    output_mode;
    int     scaling_factor;
} _t_hdmi_context;

typedef enum
{
    create_new,
    insert_new,
    reg_configed,
    reg_updated,
    buf_read_done
} BUFFER_STATE;

typedef struct _hdmi_buffer_list
{
    hdmi_video_buffer_info buffer_info;
    BUFFER_STATE buf_state;

    unsigned int idx;   ///fence count
    int fence;          ///fence fd
    struct ion_handle *hnd;
    unsigned int mva;
    unsigned int va;
    struct list_head list;
} hdmi_video_buffer_list;

static struct list_head hdmi_video_mode_buffer_list;
static struct list_head *hdmi_video_buffer_list_head = &hdmi_video_mode_buffer_list;
DEFINE_SEMAPHORE(hdmi_video_mode_mutex);
static atomic_t hdmi_video_mode_flag = ATOMIC_INIT(0);
///static int hdmi_add_video_buffer(struct hdmi_video_buffer_info *buffer_info, struct file *file);
///static struct hdmi_video_buffer_list *hdmi_search_video_buffer(struct hdmi_video_buffer_info *buffer_info, struct file *file);
///static void hdmi_destory_video_buffer(void);
#define IS_HDMI_IN_VIDEO_MODE()        atomic_read(&hdmi_video_mode_flag)
#define SET_HDMI_TO_VIDEO_MODE()       atomic_set(&hdmi_video_mode_flag, 1)
#define SET_HDMI_LEAVE_VIDEO_MODE()    atomic_set(&hdmi_video_mode_flag, 0)
static wait_queue_head_t hdmi_video_mode_wq;
#define IS_HDMI_VIDEO_MODE_DPI_IN_CHANGING_ADDRESS()    atomic_read(&hdmi_video_mode_dpi_change_address)
#define SET_HDMI_VIDEO_MODE_DPI_CHANGE_ADDRESS()        atomic_set(&hdmi_video_mode_dpi_change_address, 1)
#define SET_HDMI_VIDEO_MODE_DPI_CHANGE_ADDRESS_DONE()   atomic_set(&hdmi_video_mode_dpi_change_address, 0)


static _t_hdmi_context hdmi_context;
static _t_hdmi_context *p = &hdmi_context;
static struct list_head  HDMI_Buffer_List;

#define IS_HDMI_ON()            (HDMI_POWER_STATE_ON == atomic_read(&p->state))
#define IS_HDMI_OFF()           (HDMI_POWER_STATE_OFF == atomic_read(&p->state))
#define IS_HDMI_STANDBY()       (HDMI_POWER_STATE_STANDBY == atomic_read(&p->state))

#define IS_HDMI_NOT_ON()        (HDMI_POWER_STATE_ON != atomic_read(&p->state))
#define IS_HDMI_NOT_OFF()       (HDMI_POWER_STATE_OFF != atomic_read(&p->state))
#define IS_HDMI_NOT_STANDBY()   (HDMI_POWER_STATE_STANDBY != atomic_read(&p->state))

#define SET_HDMI_ON()           atomic_set(&p->state, HDMI_POWER_STATE_ON)
#define SET_HDMI_OFF()          atomic_set(&p->state, HDMI_POWER_STATE_OFF)
#define SET_HDMI_STANDBY()      atomic_set(&p->state, HDMI_POWER_STATE_STANDBY)

int hdmi_allocate_hdmi_buffer(void);
int hdmi_free_hdmi_buffer(void);
int hdmi_rdma_address_config(bool enable, hdmi_video_buffer_info buffer_info);

#ifdef MTK_HDMI_SCREEN_CAPTURE
bool capture_screen = false;
unsigned long capture_addr;
#endif
static int dp_mutex_src = -1, dp_mutex_dst = -1;
static unsigned int temp_mva_r, temp_mva_w, temp_va, hdmi_va, hdmi_mva_r, hdmi_mva_w;


static dev_t hdmi_devno;
static struct cdev *hdmi_cdev;
static struct class *hdmi_class = NULL;

#include <linux/mmprofile.h>
struct HDMI_MMP_Events_t
{
    MMP_Event HDMI;
    MMP_Event DDPKBitblt;
    MMP_Event OverlayDone;
    MMP_Event SwitchRDMABuffer;
    MMP_Event SwitchOverlayBuffer;
    MMP_Event StopOverlayBuffer;
    MMP_Event RDMA1RegisterUpdated;
    MMP_Event WDMA1RegisterUpdated;
    MMP_Event WaitVSync;
    MMP_Event BufferPost;
    MMP_Event BufferInsert;
    MMP_Event BufferAdd;
    MMP_Event BufferUsed;
    MMP_Event BufferRemove;
    MMP_Event FenceCreate;
    MMP_Event FenceSignal;
    MMP_Event HDMIState;
    MMP_Event GetDevInfo;
    MMP_Event ErrorInfo;
    MMP_Event MutexErr;
    MMP_Event BufferCfg;
    MMP_Event BufferUpdate;
} HDMI_MMP_Events;

typedef enum
{
    insert_Buffer_Err1 = 0xeff0,
    insert_Buffer_Err2 ,
    insert_Buffer_Err3 ,
    insert_Buffer_Err4,
    insert_Buffer_Err5,
    Buffer_INFO_Err,  ///5
    Timeline_Err,
    Buffer_Not_Enough,   ///7
    Buffer_Empt_Err,
    Fence_Err,  ///9
    Mutex_Err1,
    Mutex_Err2,
    Mutex_Err3,
    Buff_Dup_Err1, ///0xeffd
    Buff_ION_Err1

} HDMI_MMP_Err;

typedef enum
{
    Plugout ,
    Plugin ,
    ResChange

} HDMI_State;

// ---------------------------------------------------------------------------
//  Information Dump Routines
// ---------------------------------------------------------------------------

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
    }
}

#define ENABLE_HDMI_FPS_CONTROL_LOG 1
#if ENABLE_HDMI_FPS_CONTROL_LOG
static unsigned int hdmi_fps_control_fps_wdma0 = 0;
static unsigned long hdmi_fps_control_time_base_wdma0 = 0;
static unsigned int hdmi_fps_control_fps_wdma1 = 0;
static unsigned long hdmi_fps_control_time_base_wdma1 = 0;
static unsigned int hdmi_fps_control_fps_rdma1 = 0;
static unsigned long hdmi_fps_control_time_base_rdma1 = 0;
#endif

typedef enum
{
    HDMI_OVERLAY_STATUS_STOPPED,
    HDMI_OVERLAY_STATUS_STOPPING,
    HDMI_OVERLAY_STATUS_STARTING,
    HDMI_OVERLAY_STATUS_STARTED,
} HDMI_OVERLAY_STATUS;

static unsigned int hdmi_fps_control_dpi = 0;
static unsigned int hdmi_fps_control_overlay = 0;
static HDMI_OVERLAY_STATUS hdmi_overlay_status = HDMI_OVERLAY_STATUS_STOPPED;
static unsigned int hdmi_rdma_switch_count = 0;

static DPI_POLARITY clk_pol, de_pol, hsync_pol, vsync_pol;
static unsigned int dpi_clk_div, dpi_clk_duty, hsync_pulse_width, hsync_back_porch, hsync_front_porch, vsync_pulse_width, vsync_back_porch, vsync_front_porch, intermediat_buffer_num;
static HDMI_COLOR_ORDER rgb_order;

static struct task_struct *hdmi_update_task = NULL;
static wait_queue_head_t hdmi_update_wq;
static atomic_t hdmi_update_event = ATOMIC_INIT(0);

static struct task_struct *hdmi_overlay_config_task = NULL;
static wait_queue_head_t hdmi_overlay_config_wq;
static atomic_t hdmi_overlay_config_event = ATOMIC_INIT(0);

static struct task_struct *hdmi_rdma_config_task = NULL;
static struct task_struct *hdmi_rdma_update_task = NULL;

#ifdef MTK_SMARTBOOK_SUPPORT
static struct task_struct *hdmi_status_update_task = NULL;
static wait_queue_head_t hdmi_status_update_wq;
static atomic_t hdmi_status_update_event = ATOMIC_INIT(0);

#endif

static wait_queue_head_t hdmi_rdma_config_wq;
static atomic_t hdmi_rdma_config_event = ATOMIC_INIT(0);

static wait_queue_head_t hdmi_rdma_update_wq;
static atomic_t hdmi_rdma_update_event = ATOMIC_INIT(0);

static wait_queue_head_t reg_update_wq;
static atomic_t reg_update_event = ATOMIC_INIT(0);

static wait_queue_head_t dst_reg_update_wq;
static atomic_t dst_reg_update_event = ATOMIC_INIT(0);

static unsigned int hdmi_resolution_param_table[][3] =
{
    {720,   480,    60},
    {1280,  720,    60},
    {1920,  1080,   30},
};

#define ENABLE_HDMI_BUFFER_LOG 1
#if ENABLE_HDMI_BUFFER_LOG
bool enable_hdmi_buffer_log = 0;
#define HDMI_BUFFER_LOG(fmt, arg...) \
    do { \
        if(enable_hdmi_buffer_log){printk("[hdmi_buffer] "); printk(fmt, ##arg);} \
    }while (0)
#else
bool enable_hdmi_buffer_log = 0;
#define HDMI_BUFFER_LOG(fmt, arg...)
#endif

int hdmi_rdma_buffer_switch_mode = 0; // 0: switch in rdma1 frame done interrupt, 1: switch after DDPK_Bitblt done
static int hdmi_buffer_num = 0;
static int *hdmi_buffer_available = 0;
static int *hdmi_buffer_queue = 0;
static int hdmi_buffer_end = 0;
static int hdmi_buffer_start = 0;
static int hdmi_buffer_fill_count = 0;
static bool otg_enable_status = false;

static DEFINE_SEMAPHORE(hdmi_buffer_mutex);


static unsigned int hdmi_query_buf_mva(unsigned int idx)
{
    hdmi_video_buffer_list *buf = 0;
    ///spin_lock_bh(&hdmi_lock);
    list_for_each_entry(buf, &HDMI_Buffer_List, list)
    {
        if (buf->idx == idx)
        {
            HDMI_LOG("query buf list=%x, idx=%d, mva=0x%08x\n", buf, idx, buf->mva);
            return buf;
        }
    }
    ///spin_unlock_bh(&hdmi_lock);
    return 0;
}


#ifdef MTK_HDMI_ION_SUPPORT
static void hdmi_ion_init()
{
    struct ion_mm_data mm_data;

    if (!ion_client && g_ion_device)
    {
        ion_client = ion_client_create(g_ion_device, "hdmi");

        if (!ion_client)
        {
            HDMI_LOG("create ion client failed!\n");
            return;
        }

        HDMI_LOG("create ion client 0x%p\n", ion_client);
    }
}

static void hdmi_ion_deinit()
{
    if (ion_client)
    {
        ion_client_destroy(ion_client);
        ion_client = NULL;
        HDMI_LOG("destroy ion client 0x%p\n", ion_client);
    }
}

static struct ion_handle *hdmi_ion_import_handle(struct ion_client *client, int fd)
{
    struct ion_handle *handle = NULL;
    struct ion_mm_data mm_data;

    // If no need Ion support, do nothing!
    if (fd == MTK_HDMI_NO_ION_FD)
    {
        HDMI_LOG("NO NEED ion support");
        return handle;
    }

    if (!ion_client)
    {
        HDMI_LOG("invalid ion client!\n");
        return handle;
    }

    if (fd == MTK_HDMI_NO_FENCE_FD)
    {
        HDMI_LOG("invalid ion fd!\n");
        return handle;
    }

    handle = ion_import_dma_buf(client, fd);

    if (IS_ERR_OR_NULL(handle))
    {
        HDMI_LOG("import ion handle failed!\n");
        handle = 0;
        return handle;
    }

    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = handle;
    mm_data.config_buffer_param.eModuleID = 0;
    mm_data.config_buffer_param.security = 0;
    mm_data.config_buffer_param.coherent = 0;

    if (ion_kernel_ioctl(ion_client, ION_CMD_MULTIMEDIA, &mm_data))
    {
        HDMI_LOG("configure ion buffer failed!\n");
    }

    return handle;
}

static void hdmi_ion_free_handle(struct ion_client *client, struct ion_handle *handle)
{
    if (!ion_client)
    {
        HDMI_LOG("invalid ion client!\n");
        return ;
    }

    if (!handle)
    {
        return ;
    }

    ion_free(client, handle);
    HDMI_LOG("free ion handle 0x%p\n",  handle);
}

static size_t hdmi_ion_phys_mmu_addr(struct ion_client *client, struct ion_handle *handle, unsigned int *mva)
{
    size_t size;

    if (!ion_client)
    {
        HDMI_LOG("invalid ion client!\n");
        return 0;
    }

    if (!handle)
    {
        return 0;
    }

    ion_phys(client, handle, (ion_phys_addr_t *)mva, &size);
    HDMI_LOG("alloc mmu addr hnd=0x%p,mva=0x%08x\n",  handle, (unsigned int)*mva);
    return size;
}
#endif


#ifdef MTK_HDMI_FENCE_SUPPORT
#define FENCE_STEP_COUNTER 1
DEFINE_MUTEX(FenceMutex);
static atomic_t timeline_counter = ATOMIC_INIT(0);
static atomic_t fence_counter = ATOMIC_INIT(0);
static struct sw_sync_timeline *hdmi_timeline ;
static unsigned int hdmi_get_fence_counter()
{
    return atomic_add_return(FENCE_STEP_COUNTER, &fence_counter);
}

static unsigned int hdmi_get_timeline_counter_inc()
{
    return atomic_read(&timeline_counter);
}

static struct sw_sync_timeline *hdmi_create_timeline()
{
    char name[32];
    const char *prefix = "hdmi_timeline";
    sprintf(name, "%s", prefix);

    hdmi_timeline = timeline_create(name);

    if (hdmi_timeline == NULL)
    {
        printk(" error: cannot create timeline! \n");
    }
    else
    {
        HDMI_LOG(" hdmi_create_timeline() %s\n", name);
    }

    return hdmi_timeline;
}

static struct fence_data hdmi_create_fence()
{
    int fenceFd = MTK_HDMI_NO_FENCE_FD;
    struct fence_data data;
    const char *prefix = "hdmi_fence";

    spin_lock_bh(&hdmi_lock);
    data.value = hdmi_get_fence_counter();
    spin_unlock_bh(&hdmi_lock);
    sprintf(data.name, "%s-%d", prefix,  data.value);

    if (hdmi_timeline != NULL)
    {
        if (fence_create(hdmi_timeline, &data))
        {
            printk(" error: cannot create Fence Object! \n");
        }
        else
        {
            fenceFd = data.fence;
        }

        ///HDMI_LOG(" name %s, fenceFd=%d\n", data.name, fenceFd);
    }
    else
    {
        printk(" error: there is no Timeline to create Fence! \n");
    }

    MMProfileLogEx(HDMI_MMP_Events.FenceCreate, MMProfileFlagPulse, fenceFd, data.value);
    return data;
}


static void hdmi_release_fence()
{
    int inc = atomic_read(&fence_counter) - atomic_read(&timeline_counter);

    if (inc <= 0)
    {
        return ;
    }

    if (hdmi_timeline != NULL)
    {
        timeline_inc(hdmi_timeline, inc);
    }

    atomic_add(inc, &timeline_counter);
    MMProfileLogEx(HDMI_MMP_Events.FenceSignal, MMProfileFlagPulse, atomic_read(&fence_counter), inc);

}

unsigned int hdmi_timeline_inc()
{
    unsigned int fence_cnt, timeline_cnt, inc;

    spin_lock_bh(&hdmi_lock);
    fence_cnt = atomic_read(&fence_counter);
    timeline_cnt = atomic_read(&timeline_counter);
    inc = fence_cnt - timeline_cnt;
    spin_unlock_bh(&hdmi_lock);

    if (inc < 0 || inc > 5)
    {
        if (hdmi_bufferdump_on > 0)
        {
            MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Timeline_Err, inc);
        }

        HDMI_LOG("fence error: inc=%d, fence_cnt=%d, timeline_cnt=%d! \n", inc, fence_cnt, timeline_cnt);
        inc = 0;
    }

    spin_lock_bh(&hdmi_lock);
    atomic_add(1, &timeline_counter);
    spin_unlock_bh(&hdmi_lock);
    return atomic_read(&timeline_counter);
}

/**
 * step forward timeline
 * all fence(sync_point) will be signaled prior to it's counter
 * refer to {@link sw_sync_timeline_inc}
 */
static void hdmi_signal_fence()
{
    unsigned inc = 0;

    if (hdmi_timeline != NULL)
    {
        inc = 1;  ///hdmi_get_timeline_counter_inc();
        timeline_inc(hdmi_timeline, inc);

        if (hdmi_bufferdump_on > 0)
        {
            MMProfileLogEx(HDMI_MMP_Events.FenceSignal, MMProfileFlagPulse, atomic_read(&timeline_counter), atomic_read(&fence_counter));
        }

        ///HDMI_LOG("  %s:%d, tl %d, fd %d\n", hdmi_timeline->obj.name, hdmi_timeline->value, hdmi_timeline, fence_counter);
    }
    else
    {
        HDMI_LOG(" no Timeline to inc tl %d, fd %d\n", atomic_read(&timeline_counter), atomic_read(&fence_counter));
    }


}

static void hdmi_sync_init()
{

    ///spin_lock_init(&hdmi_lock);
    hdmi_create_timeline();
    // Reset all counter to 0
    atomic_set(&timeline_counter, 0);
    atomic_set(&fence_counter, 0);
}

static void hdmi_sync_destroy()
{

    if (hdmi_timeline != NULL)
    {
        HDMI_LOG(" destroy timeline %s:%d\n", hdmi_timeline->obj.name, hdmi_timeline->value);
        timeline_destroy(hdmi_timeline);
        hdmi_timeline = NULL;
    }

    // Reset all counter to 0
    atomic_set(&timeline_counter, 0);
    atomic_set(&fence_counter, 0);
}


#endif

static void hdmi_buffer_init(int num)
{
    int i;
    return;

    if (down_interruptible(&hdmi_buffer_mutex))
    {
        HDMI_LOG("Can't get semaphore in %s()\n", __func__);
        return;
    }

    HDMI_FUNC();

    hdmi_buffer_num = num;
    hdmi_buffer_start = 0;
    hdmi_buffer_end = 0;
    hdmi_buffer_fill_count = 0;

    hdmi_buffer_available = (int *)vmalloc(hdmi_buffer_num * sizeof(int));
    hdmi_buffer_queue = (int *)vmalloc(hdmi_buffer_num * sizeof(int));

    for (i = 0; i < hdmi_buffer_num; i++)
    {
        hdmi_buffer_available[i] = 1;
        hdmi_buffer_queue[i] = -1;
    }

    up(&hdmi_buffer_mutex);
}

static void hdmi_buffer_deinit(void)
{
    return;

    if (down_interruptible(&hdmi_buffer_mutex))
    {
        HDMI_LOG("Can't get semaphore in %s()\n", __func__);
        return;
    }

    HDMI_FUNC();

    hdmi_buffer_start = 0;
    hdmi_buffer_end = 0;
    hdmi_buffer_fill_count = 0;

    if (hdmi_buffer_available)
    {
        vfree((const void *)hdmi_buffer_available);
        hdmi_buffer_available = 0;
    }

    if (hdmi_buffer_queue)
    {
        vfree((const void *)hdmi_buffer_queue);
        hdmi_buffer_queue = 0;
    }

    up(&hdmi_buffer_mutex);
}

static void hdmi_dump_buffer_queue(void)
{
    HDMI_BUFFER_LOG("[hdmi] available={%d,%d,%d,%d} queue={%d,%d,%d,%d}, {start,end}={%d,%d} count=%d\n",
                    hdmi_buffer_available[0], hdmi_buffer_available[1], hdmi_buffer_available[2], hdmi_buffer_available[3],
                    hdmi_buffer_queue[hdmi_buffer_start],
                    hdmi_buffer_queue[(hdmi_buffer_start + 1) % hdmi_buffer_num],
                    hdmi_buffer_queue[(hdmi_buffer_start + 2) % hdmi_buffer_num],
                    hdmi_buffer_queue[(hdmi_buffer_start + 3) % hdmi_buffer_num],
                    hdmi_buffer_start, hdmi_buffer_end, hdmi_buffer_fill_count);
}

static int hdmi_is_buffer_empty(void)
{
    return hdmi_buffer_fill_count == 0;
}

static void hdmi_release_buffer(int index)
{
    //down_interruptible(&hdmi_buffer_mutex);

    HDMI_BUFFER_LOG("[hdmi] hdmi_release_buffer: %d\n", index);
    hdmi_buffer_available[index] = 1;
    hdmi_dump_buffer_queue();

    //up(&hdmi_buffer_mutex);
}

static int hdmi_acquire_buffer(void)
{
    int index = -1;

    if (down_interruptible(&hdmi_buffer_mutex))
    {
        HDMI_LOG("Can't get semaphore in %s()\n", __func__);
        return -1;
    }

    if (!hdmi_is_buffer_empty())
    {
        index = hdmi_buffer_queue[hdmi_buffer_start];

        HDMI_BUFFER_LOG("[hdmi] hdmi_acquire_buffer: %d\n", index);

        hdmi_buffer_queue[hdmi_buffer_start] = -1;
        hdmi_buffer_start = (hdmi_buffer_start + 1) % hdmi_buffer_num;

        hdmi_buffer_fill_count--;

        hdmi_dump_buffer_queue();
    }

    up(&hdmi_buffer_mutex);
    return index;
}

static int hdmi_dequeue_buffer(void)
{
    int i;

    if (down_interruptible(&hdmi_buffer_mutex))
    {
        HDMI_LOG("Can't get semaphore in %s()\n", __func__);
        return -1;
    }

    for (i = 0; i < hdmi_buffer_num; i++)
    {
        if (hdmi_buffer_available[i])
        {
            hdmi_buffer_available[i] = 0;
            HDMI_BUFFER_LOG("[hdmi] hdmi_dequeue_buffer: %d\n", i);

            hdmi_dump_buffer_queue();
            up(&hdmi_buffer_mutex);
            return i;
        }
    }

    // if no available buffer, return the last buffer in queue
    if (!hdmi_is_buffer_empty())
    {
        int index = hdmi_buffer_queue[hdmi_buffer_end];
        HDMI_BUFFER_LOG("[hdmi] hdmi_dequeue_buffer(last): %d\n", index);

        hdmi_buffer_queue[hdmi_buffer_end] = -1;
        hdmi_buffer_end = (hdmi_buffer_end + (hdmi_buffer_num - 1)) % hdmi_buffer_num;
        hdmi_buffer_fill_count--;

        hdmi_dump_buffer_queue();
        up(&hdmi_buffer_mutex);
        return index;
    }

    up(&hdmi_buffer_mutex);
    HDMI_LOG("no available buffer\n");
    return -1;
}

static void hdmi_enqueue_buffer(int index)
{
    if (down_interruptible(&hdmi_buffer_mutex))
    {
        HDMI_LOG("Can't get semaphore in %s()\n", __func__);
        hdmi_release_buffer(index);
        return;
    }

    HDMI_BUFFER_LOG("[hdmi] hdmi_enqueue_buffer: %d\n", index);

    //hdmi_buffer_available[index] = 1;
    hdmi_buffer_end = (hdmi_buffer_start + hdmi_buffer_fill_count) % hdmi_buffer_num;
    hdmi_buffer_queue[hdmi_buffer_end] = index;

    if (hdmi_buffer_fill_count == hdmi_buffer_num)
    {
        hdmi_buffer_start = (hdmi_buffer_start + 1) % hdmi_buffer_num;
    }
    else
    {
        hdmi_buffer_fill_count++;
    }

    hdmi_dump_buffer_queue();

    up(&hdmi_buffer_mutex);
}

static unsigned long get_current_time_us(void)
{
    struct timeval t;
    do_gettimeofday(&t);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

static void hdmi_udelay(unsigned int us)
{
    udelay(us);
}

static void hdmi_mdelay(unsigned int ms)
{
    msleep(ms);
}

static unsigned int hdmi_get_width(HDMI_VIDEO_RESOLUTION r)
{
    ASSERT(r < HDMI_VIDEO_RESOLUTION_NUM);
    return hdmi_resolution_param_table[r][0];
}

static unsigned int hdmi_get_height(HDMI_VIDEO_RESOLUTION r)
{
    ASSERT(r < HDMI_VIDEO_RESOLUTION_NUM);
    return hdmi_resolution_param_table[r][1];
}


static atomic_t hdmi_fake_in = ATOMIC_INIT(false);
#define IS_HDMI_FAKE_PLUG_IN()  (true == atomic_read(&hdmi_fake_in))
#define SET_HDMI_FAKE_PLUG_IN() (atomic_set(&hdmi_fake_in, true))
#define SET_HDMI_NOT_FAKE()     (atomic_set(&hdmi_fake_in, false))

// For Debugfs
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

// For Debugfs
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

void hdmi_read_reg(unsigned char u8Reg)
{
    hdmi_drv->read(u8Reg);
}

void hdmi_write_reg(unsigned char u8Reg, unsigned char u8Data)
{
    hdmi_drv->write(u8Reg, u8Data);
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

static int hdmi_rdma_config_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    for (;;)
    {
        wait_event_interruptible(hdmi_rdma_config_wq, atomic_read(&hdmi_rdma_config_event));
        atomic_set(&hdmi_rdma_config_event, 0);

#ifdef MTK_HDMI_FENCE_SUPPORT
        MMProfileLogEx(HDMI_MMP_Events.BufferCfg, MMProfileFlagStart, p->is_clock_on, 1);

        if (down_interruptible(&hdmi_update_mutex))
        {
            MMProfileLogEx(HDMI_MMP_Events.MutexErr, MMProfileFlagPulse, Mutex_Err1, Mutex_Err1);
            MMProfileLogEx(HDMI_MMP_Events.BufferCfg, MMProfileFlagEnd, p->is_clock_on, 1);
            HDMI_LOG("[HDMI] can't get semaphore in\n");
            continue; /// continue  return EAGAIN
        }

        int buf_sequence = 1;

        if (p->is_clock_on == true) ///remove the first head here
        {
            if (!list_empty(&HDMI_Buffer_List))
            {
                hdmi_video_buffer_list *pBuffList = NULL;
                hdmi_video_buffer_list *NexList = NULL;

                spin_lock_bh(&hdmi_lock);
                pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);

                if (pBuffList->buf_state != insert_new)
                {
                    if (!list_is_last(&pBuffList->list, &HDMI_Buffer_List))
                    {
                        buf_sequence++;
                        NexList = list_entry(pBuffList->list.next, hdmi_video_buffer_list, list);

                        if (NexList->buf_state != insert_new)
                        {
                            hdmi_video_buffer_list *LastList = NULL;

                            if (!list_is_last(&NexList->list, &HDMI_Buffer_List))
                            {
                                buf_sequence++;
                                LastList = list_entry(NexList->list.next, hdmi_video_buffer_list, list);

                                if (LastList->buf_state == insert_new)
                                {
                                    NexList = LastList;
                                }
                            }
                        }

                        pBuffList = NexList;
                    }

                }

                spin_unlock_bh(&hdmi_lock);

                if ((pBuffList == NULL) || (pBuffList->buf_state != insert_new)
                        || (hdmi_rdma_address_config(true, pBuffList->buffer_info) < 0))
                {
                    ///HDMI_LOG(" rdma config(pBuffList %x) error to exit\n", pBuffList);

                    if (pBuffList && (hdmi_bufferdump_on > 0))
                    {
                        MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buffer_Not_Enough, pBuffList);
                    }
                    else if (hdmi_bufferdump_on > 0)
                    {
                        MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buffer_Not_Enough, buf_sequence);
                    }

                    if ((pBuffList != NULL) && (pBuffList->buf_state == insert_new))
                    {
                        pBuffList->buf_state = buf_read_done;
                        HDMI_LOG(" buffer config error to configed %x, state %d, idx %d\n", pBuffList, pBuffList->buf_state,  pBuffList->idx);
                    }
                }
                else
                {
                    spin_lock_bh(&hdmi_lock);
                    pBuffList->buf_state = reg_configed;
                    spin_unlock_bh(&hdmi_lock);

                    if (hdmi_bufferdump_on > 0)
                    {
                        MMProfileLogEx(HDMI_MMP_Events.BufferUsed, MMProfileFlagPulse, pBuffList, buf_sequence);
                    }
                }

            }
            else
            {
                MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buffer_Empt_Err, Buffer_Empt_Err);
                HDMI_LOG(" rdma config buffer is NULL\n");
            }
        }

        up(&hdmi_update_mutex);
        MMProfileLogEx(HDMI_MMP_Events.BufferCfg, MMProfileFlagEnd, p->is_clock_on, 1);
#else

#endif

        if (kthread_should_stop())
        {
            break;
        }
    }

    return 0;
}

#ifdef MTK_SMARTBOOK_SUPPORT
bool callback_plug = false;
static int hdmi_status_update_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    for (;;)
    {
        wait_event_interruptible(hdmi_status_update_wq, atomic_read(&hdmi_status_update_event));
        atomic_set(&hdmi_status_update_event, 0);

        printk("[hdmi]%s, state = %d\n", __func__);

        if (callback_plug == false)
        {
            continue;
        }

        hdmi_resume();

        hdmi_drv->get_params(hdmi_params);
        switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);

        if (hdmi_params->cabletype == MHL_SMB_CABLE)
        {
#ifdef CONFIG_HAS_SBSUSPEND
            sb_plug_in();
#endif
        }

        printk("[hdmi]%s,  out!\n", __func__);

        if (kthread_should_stop())
        {
            break;
        }
    }
}
#endif

#ifdef MTK_HDMI_FENCE_SUPPORT
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

        int buf_sequence = 1;
        hdmi_video_buffer_list *pUpdateList = NULL;

        if (p->is_clock_on == true) ///remove the first head here
        {
            if (!list_empty(&HDMI_Buffer_List))
            {
                hdmi_video_buffer_list *pBuffList = NULL;
                hdmi_video_buffer_list *NexList = NULL;

                spin_lock_bh(&hdmi_lock);
                pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);

                while (pBuffList)
                {
                    if (pBuffList->buf_state == insert_new)
                    {
                        break;
                    }
                    else if (pBuffList->buf_state == reg_configed)
                    {
                        pBuffList->buf_state = reg_updated;
                        pUpdateList = pBuffList;
                    }
                    else if (pBuffList->buf_state == reg_updated)
                    {
                        pBuffList->buf_state = buf_read_done;
                    }

                    hdmi_video_buffer_list *NexList = NULL;

                    if (!list_is_last(&pBuffList->list, &HDMI_Buffer_List))
                    {
                        NexList = list_entry(pBuffList->list.next, hdmi_video_buffer_list, list);
                        pBuffList = NexList;
                    }
                    else
                    {
                        pBuffList = NULL;
                    }

                }

                pBuffList = NULL;
                int remove_buffer_cnt = 0;
                pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);
                spin_unlock_bh(&hdmi_lock);

                while (pBuffList)
                {
                    if (pBuffList && (pBuffList->buf_state == buf_read_done)
                            && (!list_is_last(&pBuffList->list, &HDMI_Buffer_List)))
                    {
                        if (hdmi_bufferdump_on > 0)
                        {
                            MMProfileLogEx(HDMI_MMP_Events.BufferRemove, MMProfileFlagPulse, pBuffList, pBuffList->buffer_info.src_phy_addr);
                        }
                        else
                        {
                            HDMI_LOG("remove list %x-->buffer %x \n", pBuffList, pBuffList->buffer_info.src_phy_addr);
                        }

#ifdef MTK_HDMI_ION_SUPPORT

                        if (pBuffList->va)
                        {
                            ion_unmap_kernel(ion_client, pBuffList->hnd);
                        }

                        hdmi_ion_free_handle(ion_client, pBuffList->hnd);
#endif
                        spin_lock_bh(&hdmi_lock);
                        list_del(&pBuffList->list);
                        kfree(pBuffList);
                        pBuffList = NULL;
                        spin_unlock_bh(&hdmi_lock);

                        hdmi_timeline_inc();
                        hdmi_signal_fence();

                        remove_buffer_cnt++;

                        spin_lock_bh(&hdmi_lock);
                        pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);
                        spin_unlock_bh(&hdmi_lock);
                    }
                    else
                    {
                        break;
                    }

                }

                if (remove_buffer_cnt > 1)
                {
                    if (hdmi_bufferdump_on > 0)
                    {
                        MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buff_Dup_Err1, remove_buffer_cnt);
                    }

                    printk("[hdmi] %s, %d remove two buffer one time", __func__, __LINE__);

                }
            }

        }
        else
        {
            HDMI_LOG("fence stop rdma \n");
            hdmi_video_buffer_list *pBuffList = NULL;

            while (!list_empty(&HDMI_Buffer_List))
            {
                spin_lock_bh(&hdmi_lock);
                pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);
                spin_unlock_bh(&hdmi_lock);

                if (hdmi_bufferdump_on > 0)
                {
                    MMProfileLogEx(HDMI_MMP_Events.BufferRemove, MMProfileFlagPulse, pBuffList, pBuffList->buffer_info.src_phy_addr);
                }
                else
                {
                    HDMI_LOG("delete list %x-->buffer %x \n", pBuffList, pBuffList->buffer_info.src_phy_addr);
                }

#ifdef MTK_HDMI_ION_SUPPORT

                if (pBuffList->va)
                {
                    ion_unmap_kernel(ion_client, pBuffList->hnd);
                }

                hdmi_ion_free_handle(ion_client, pBuffList->hnd);
#endif

                spin_lock_bh(&hdmi_lock);
                list_del(&pBuffList->list);
                kfree(pBuffList);
                pBuffList = NULL;
                spin_unlock_bh(&hdmi_lock);
            }

            hdmi_release_fence();
            HDMI_LOG("fence stop rdma done\n");
        }

        up(&hdmi_update_mutex);
        MMProfileLogEx(HDMI_MMP_Events.BufferUpdate, MMProfileFlagEnd, p->is_clock_on, pUpdateList);

        if (kthread_should_stop())
        {
            break;
        }

    }
}
#endif

extern void DBG_OnTriggerHDMI(void);
extern void DBG_OnHDMIDone(void);

static void hdmi_update_impl(void)
{
}

//--------------------------FIXME-------------------------------
DPI_STATUS hdmi_config_pll(HDMI_VIDEO_RESOLUTION resolution)
{
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

    return DPI_STATUS_OK;
}

static wait_queue_head_t hdmi_vsync_wq;
static bool hdmi_vsync_flag = false;
static int hdmi_vsync_cnt = 0;

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

static void _rdma1_irq_handler(unsigned int param)
{
    RET_VOID_IF_NOLOG(!is_hdmi_active());

    ///RET_VOID_IF_NOLOG(!p->lcm_is_video_mode);
    ///HDMI_LOG("_rdma1_irq_handler irq %x\n", param);
    if (param & 0x20) // taget line 0x20
    {
        MMProfileLogEx(HDMI_MMP_Events.RDMA1RegisterUpdated, MMProfileFlagPulse, param, 0x20);

        ///hdmi_update_buffer_switch();

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
    int tmpBufferSize;
    M4U_PORT_STRUCT m4uport;

    HDMI_FUNC();

    RETIF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS, 0);

    p->hdmi_width = hdmi_get_width(hdmi_params->init_config.vformat);
    p->hdmi_height = hdmi_get_height(hdmi_params->init_config.vformat);
    p->bg_width = 0;
    p->bg_height = 0;

    lcm_width = DISP_GetScreenWidth();
    lcm_height = DISP_GetScreenHeight();

    //printk("[hdmi]%s, hdmi_width=%d, hdmi_height=%d\n", __func__, p->hdmi_width, p->hdmi_height);
    HDMI_LOG("lcm_width=%d, lcm_height=%d\n", lcm_width, lcm_height);

    tmpBufferSize = lcm_width * lcm_height * hdmi_bpp * hdmi_params->intermediat_buffer_num;

    temp_va = (unsigned int) vmalloc(tmpBufferSize);

    if (((void *) temp_va) == NULL)
    {
        HDMI_LOG("vmalloc %d bytes fail\n", tmpBufferSize);
        return -1;
    }


    HDMI_LOG("temp_va=0x%08x, temp_mva_w=0x%08x\n", temp_va, temp_mva_w);


    p->lcm_width = lcm_width;
    p->lcm_height = lcm_height;
    p->output_video_resolution = hdmi_params->init_config.vformat;
    p->output_audio_format = hdmi_params->init_config.aformat;
    p->scaling_factor = hdmi_params->scaling_factor < 10 ? hdmi_params->scaling_factor : 10;

    ///if (p->lcm_is_video_mode)
    {
        hdmi_buffer_init(hdmi_params->intermediat_buffer_num);
    }

    hdmi_dpi_config_clock(); // configure dpi clock

    hdmi_dpi_power_switch(false);   // but dpi power is still off

    ///if (p->lcm_is_video_mode)
    {
        disp_register_irq(DISP_MODULE_RDMA1, _rdma1_irq_handler);

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
    }

    return HDMI_STATUS_OK;
}

//free IRQ
/*static*/ void hdmi_dpi_free_irq(void)
{
    RET_VOID_IF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS);
    DPI_CHECK_RET(HDMI_DPI(_FreeIRQ)());
}

/* Release memory */
/* Will only be used in ioctl(MTK_HDMI_AUDIO_VIDEO_ENABLE) */
static  HDMI_STATUS hdmi_drv_deinit(void)
{
    int temp_va_size;

    HDMI_FUNC();
    RETIF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS, 0);

    disp_unregister_irq(DISP_MODULE_RDMA1, _rdma1_irq_handler);

    hdmi_dpi_power_switch(false);
    hdmi_buffer_deinit();
    hdmi_free_hdmi_buffer();
    hdmi_dpi_free_irq();

    //free temp_va & temp_mva
    HDMI_LOG("Free temp_va and temp_mva\n");
    temp_va_size = p->lcm_width * p->lcm_height * hdmi_bpp * hdmi_params->intermediat_buffer_num;

    if (temp_va)
    {
        vfree((void *) temp_va);
        temp_va = 0;
    }

    return HDMI_STATUS_OK;
}

static void hdmi_dpi_config_update(void)
{
    HDMI_FUNC();

    DPI_CHECK_RET(HDMI_DPI(_ConfigPixelClk)(clk_pol, dpi_clk_div, dpi_clk_duty));

    DPI_CHECK_RET(HDMI_DPI(_ConfigDataEnable)(de_pol)); // maybe no used

    DPI_CHECK_RET(HDMI_DPI(_ConfigHsync)(hsync_pol, hsync_pulse_width, hsync_back_porch, hsync_front_porch));

    DPI_CHECK_RET(HDMI_DPI(_ConfigVsync)(vsync_pol, vsync_pulse_width, vsync_back_porch, vsync_front_porch));

    DPI_CHECK_RET(HDMI_DPI(_FBSetSize)(p->hdmi_width, p->hdmi_height));

    DPI_CHECK_RET(HDMI_DPI(_FBSetPitch)(DPI_FB_0, p->hdmi_width * 3)); // do nothing
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
}


/* Will only be used in hdmi_drv_init(), this means that will only be use in ioctl(MTK_HDMI_AUDIO_VIDEO_ENABLE) */
/*static*/ void hdmi_dpi_config_clock(void)
{
    int ret = 0;

    RET_VOID_IF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS);

    switch (hdmi_params->init_config.vformat)
    {
        case HDMI_VIDEO_720x480p_60Hz:
        {
            printk("[hdmi]480p\n");
            //ret = pll_fsel(TVDPLL, 0x1C7204C7);
            ASSERT(!ret);

            dpi_clk_div = 2;
            dpi_clk_duty = 1;

            break;
        }

        case HDMI_VIDEO_1280x720p_60Hz:
        {
            printk("[hdmi]720p 60Hz\n");

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
    intermediat_buffer_num = hdmi_params->intermediat_buffer_num;


    DPI_CHECK_RET(HDMI_DPI(_Init)(FALSE));
}


int hdmi_allocate_hdmi_buffer(void)
{
    M4U_PORT_STRUCT m4uport;
    return 0;

    int hdmiPixelSize = p->hdmi_width * p->hdmi_height;
    int hdmiDataSize = hdmiPixelSize * 4;////hdmi_bpp;
    int hdmiBufferSize = hdmiDataSize * hdmi_params->intermediat_buffer_num;

    HDMI_FUNC();

    RETIF(hdmi_va, 0);

    hdmi_va = (unsigned int) vmalloc(hdmiBufferSize);

    if (((void *) hdmi_va) == NULL)
    {
        HDMI_LOG("vmalloc %d bytes fail!!!\n", hdmiBufferSize);
        return -1;
    }

    memset((void *) hdmi_va, 0, hdmiBufferSize);

    //RDMA1
    if (m4u_alloc_mva(DISP_RDMA1, hdmi_va, hdmiBufferSize, 0, 0, &hdmi_mva_r))
    {
        HDMI_LOG("m4u_alloc_mva for hdmi_mva_r fail\n");
        return -1;
    }

    memset((void *) &m4uport, 0, sizeof(M4U_PORT_STRUCT));
    m4uport.ePortID = DISP_RDMA1;
    m4uport.Virtuality = 1;
    m4uport.domain = 0;
    m4uport.Security = 0;
    m4uport.Distance = 1;
    m4uport.Direction = 0;
    m4u_config_port(&m4uport);

    HDMI_LOG("hdmi_va=0x%08x, hdmi_mva_r=0x%08x\n", hdmi_va, hdmi_mva_r);

    return 0;
}

int hdmi_free_hdmi_buffer(void)
{
    int hdmi_va_size = p->hdmi_width * p->hdmi_height * hdmi_bpp * hdmi_params->intermediat_buffer_num;
    return;

    //free hdmi_va & hdmi_mva
    HDMI_LOG("Free hdmi_va and hdmi_mva\n");

    if (hdmi_mva_r)
    {
        M4U_PORT_STRUCT m4uport;
        m4uport.ePortID =  DISP_RDMA1;
        m4uport.Virtuality = 0;
        m4uport.domain = 0;
        m4uport.Security = 0;
        m4uport.Distance = 1;
        m4uport.Direction = 0;
        m4u_config_port(&m4uport);

        m4u_dealloc_mva(DISP_RDMA1,
                        hdmi_va,
                        hdmi_va_size,
                        hdmi_mva_r);
        hdmi_mva_r = 0;
    }

    if (hdmi_va)
    {
        vfree((void *) hdmi_va);
        hdmi_va = 0;
    }

    return 0;
}
static int rdmafpscnt = 0;
int hdmi_rdma_address_config(bool enable, hdmi_video_buffer_info buffer_info)
{
    ///HDMI_FUNC();

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

        unsigned int offset = (buffer_info.src_pitch - buffer_info.src_width) / 2 * rdmaInputsize;
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

#else
        ///HDMI_LOG(" %d info: idx %d, fmt %d, h %d, w %d, x_o %d, y_o %d, pitch %d mutex %d - %d\n", eARGB8888, buffer_info.next_buff_idx, buffer_info.src_fmt, buffer_info.src_height, buffer_info.src_width,
        ///                             buffer_info.src_offset_x, buffer_info.src_offset_y,buffer_info.src_pitch, dp_mutex_dst, rdmafpscnt);
#endif
#if 0
        // Config RDMA->DPI1
        int ifps = buffer_info.next_buff_idx;

        if (((ifps / 10) % 2) == 0)
        {
            memset((void *) hdmi_va, 0x38, 1280 * 720 * 4);
            config.addr = hdmi_mva_r; ///hdmiSourceAddr;
        }
        else
        {
            memset((void *) hdmi_va, 0, 1280 * 720 * 4);
            config.addr = hdmi_mva_r; ///hdmiSourceAddr;
        }

#else
        config.addr = hdmiSourceAddr;
#endif
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
            rdmafpscnt = 0;
        }
        else
        {
            need_config = false;
        }

        rdmafpscnt++;

        ///disp_path_get_mutex_(dp_mutex_dst);

        if (true == need_config)
        {
            M4U_PORT_STRUCT m4uport;
            memset((void *) &m4uport, 0, sizeof(M4U_PORT_STRUCT));
            m4uport.ePortID = DISP_RDMA1;
            m4uport.Virtuality = 1;
            m4uport.domain = 0;
            m4uport.Security = 0;
            m4uport.Distance = 1;
            m4uport.Direction = 0;
            m4u_config_port(&m4uport);
            disp_path_config_(&config, dp_mutex_dst);

            ///DPI_CHECK_RET(HDMI_DPI(_EnableColorBar)());
            DPI_CHECK_RET(HDMI_DPI(_EnableClk)());
#if 0
            disp_dump_reg(DISP_MODULE_OVL);
            disp_dump_reg(DISP_MODULE_RDMA1);
            disp_dump_reg(DISP_MODULE_RDMA);
            disp_dump_reg(DISP_MODULE_COLOR);
            disp_dump_reg(DISP_MODULE_BLS);
            disp_dump_reg(DISP_MODULE_WDMA0);
            disp_dump_reg(DISP_MODULE_CONFIG);
            disp_dump_reg(DISP_MODULE_MUTEX);
            DPI_CHECK_RET(HDMI_DPI(_DumpRegisters)());
#endif
            DPI_CHECK_RET(HDMI_DPI(_DumpRegisters)());

        }

        disp_path_get_mutex_(dp_mutex_dst);
        DISP_REG_SET(0xa000 + DISP_REG_RDMA_MEM_START_ADDR, config.addr);
        disp_path_release_mutex_(dp_mutex_dst);

        ///disp_path_release_mutex_(dp_mutex_dst);


    }
    else
    {
        if (-1 != dp_mutex_dst)
        {
            //FIXME: release mutex timeout
            HDMI_LOG("Stop RDMA1>DPI\n");
            disp_path_get_mutex_(dp_mutex_dst);

            ///DISP_REG_SET_FIELD(1 << dp_mutex_src , DISP_REG_CONFIG_MUTEX_INTEN,  1);
            RDMAStop(1);
            RDMAReset(1);
            disp_path_release_mutex_(dp_mutex_dst);

            //disp_unlock_mutex(dp_mutex_dst);
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

        HDMI_DPI(_PowerOn)();
        HDMI_DPI(_EnableIrq)();
        DPI_CHECK_RET(HDMI_DPI(_EnableClk)());

        p->is_clock_on = true;
    }
    else
    {
#if 0

        if (p->is_clock_on == false)
        {
            HDMI_LOG("power off request while already powered off!\n");
            return;
        }

#endif
        p->is_clock_on = false;
        atomic_set(&hdmi_rdma_update_event, 1); ///release buffer
        wake_up_interruptible(&hdmi_rdma_update_wq);

        HDMI_DPI(_DisableIrq)();
        HDMI_DPI(_DisableClk)();
        HDMI_DPI(_PowerOff)();


    }
}

/* Configure video attribute */
static int hdmi_video_config(HDMI_VIDEO_RESOLUTION vformat, HDMI_VIDEO_INPUT_FORMAT vin, HDMI_VIDEO_OUTPUT_FORMAT vout)
{
    HDMI_FUNC();
    RETIF(IS_HDMI_NOT_ON(), 0);

    hdmi_allocate_hdmi_buffer();
    ///hdmi_dst_display_path_config(true);

    hdmi_fps_control_overlay = 0;
    hdmi_fps_control_dpi = 0;

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

#ifdef MTK_SMARTBOOK_SUPPORT
struct timer_list smb_timer;
static bool smb_timer_started = false;
void smartbook_state_callback();

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

void smartbook_state_callback()
{
    printk("[hdmi]%s \n", __func__);
    atomic_set(&hdmi_status_update_event, 1);
    wake_up_interruptible(&hdmi_status_update_wq);

}
#endif

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

        default:
        {
            printk("[hdmi]%s, state not support\n", __func__);
            break;
        }
    }

    return;
}

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

    up(&hdmi_update_mutex);

    return;
}

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

    disp_module_clock_off(DISP_MODULE_RDMA1, "HDMI");
    up(&hdmi_update_mutex);

    if (hdmi_bufferdump_on > 0)
    {
        MMProfileLogEx(HDMI_MMP_Events.HDMIState, MMProfileFlagEnd, Plugout, 0);
    }
}

/*static*/ void hdmi_resume(void)
{
    HDMI_FUNC();

    RET_VOID_IF(IS_HDMI_NOT_STANDBY());
    SET_HDMI_ON();

    if (hdmi_bufferdump_on > 0)
    {
        MMProfileLogEx(HDMI_MMP_Events.HDMIState, MMProfileFlagStart, Plugin, 0);
    }

    if (down_interruptible(&hdmi_update_mutex))
    {
        printk("[hdmi][HDMI] can't get semaphore in %s()\n", __func__);
        return;
    }

    disp_module_clock_on(DISP_MODULE_RDMA1, "HDMI");

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

static BOOL hdmi_drv_init_context(void);

static void dpi_setting_res(u8 arg)
{
    switch (arg)
    {
        case HDMI_VIDEO_720x480p_60Hz:
        {

#if defined(MHL_SII8338)
            clk_pol     = HDMI_POLARITY_FALLING;
#else
            clk_pol     = HDMI_POLARITY_RISING;
#endif
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

            p->bg_height = ((480 * p->scaling_factor) / 100 >> 2) << 2 ;
            p->bg_width = ((720 * p->scaling_factor) / 100 >> 2) << 2 ;
            p->hdmi_width = 720 - p->bg_width;
            p->hdmi_height = 480 - p->bg_height;
            p->output_video_resolution = HDMI_VIDEO_720x480p_60Hz;
            break;
        }

        case HDMI_VIDEO_1280x720p_60Hz:
        {

            clk_pol     = HDMI_POLARITY_RISING;
            de_pol      = HDMI_POLARITY_RISING;
#if defined(HDMI_TDA19989)
            hsync_pol   = HDMI_POLARITY_FALLING;
#else
            hsync_pol   = HDMI_POLARITY_FALLING;
#endif
            vsync_pol   = HDMI_POLARITY_FALLING;;

#if defined(MHL_SII8338)
            clk_pol     = HDMI_POLARITY_FALLING;
            de_pol      = HDMI_POLARITY_RISING;
            hsync_pol   = HDMI_POLARITY_RISING;
            vsync_pol   = HDMI_POLARITY_RISING;
#endif

            dpi_clk_div = 2;

            hsync_pulse_width   = 40;
            hsync_back_porch    = 220;
            hsync_front_porch   = 110;

            vsync_pulse_width   = 5;
            vsync_back_porch    = 20;
            vsync_front_porch   = 5;

            p->bg_height = ((720 * p->scaling_factor) / 100 >> 2) << 2 ;
            p->bg_width = ((1280 * p->scaling_factor) / 100 >> 2) << 2 ;
            p->hdmi_width = 1280 - p->bg_width; //1280  1366
            p->hdmi_height = 720 - p->bg_height;//720;  768

#ifdef MTK_SMARTBOOK_SUPPORT

            if (hdmi_params->cabletype == MHL_SMB_CABLE)
            {
                p->hdmi_width = 1366;
                p->hdmi_height = 768;
                p->bg_height = 0;
                p->bg_width = 0;
            }

#endif
            p->output_video_resolution = HDMI_VIDEO_1280x720p_60Hz;
            break;
        }


        case HDMI_VIDEO_1920x1080p_30Hz:
        {
#if defined(MHL_SII8338)
            clk_pol     = HDMI_POLARITY_FALLING;
            de_pol      = HDMI_POLARITY_RISING;
            hsync_pol   = HDMI_POLARITY_RISING;
            vsync_pol   = HDMI_POLARITY_RISING;
#else
            clk_pol     = HDMI_POLARITY_RISING;
            de_pol      = HDMI_POLARITY_RISING;
            hsync_pol   = HDMI_POLARITY_FALLING;
            vsync_pol   = HDMI_POLARITY_FALLING;
#endif

            dpi_clk_div = 2;

            hsync_pulse_width   = 44;
            hsync_back_porch    = 148;
            hsync_front_porch   = 88;

            vsync_pulse_width   = 5;
            vsync_back_porch    = 36;
            vsync_front_porch   = 4;

            p->hdmi_width = 1920;
            p->hdmi_height = 1080;
            p->output_video_resolution = HDMI_VIDEO_1920x1080p_30Hz;
            break;
        }

        default:
            break;
    }

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
            //case MTK_HDMI_FORCE_CLOSE:
        {
            RETIF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS, 0);
            RETIF(!p->is_enabled, 0);
            RETIF(IS_HDMI_OFF(), 0);

            if (p->is_force_disable == true)
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
            //case MTK_HDMI_FORCE_OPEN:
        {
            RETIF(p->output_mode == HDMI_OUTPUT_MODE_DPI_BYPASS, 0);
            RETIF(!p->is_enabled, 0);
            RETIF(IS_HDMI_OFF(), 0);

            if (p->is_force_disable == false)
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

            if (arg > 1)
            {
                arg = 1;
            }

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
                if ((list_empty(&HDMI_Buffer_List)) || (tmp > 15))
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

            DPI_CHECK_RET(HDMI_DPI(_DisableClk)());
            DPI_CHECK_RET(HDMI_DPI(_ConfigHDMI)());
            DPI_CHECK_RET(HDMI_DPI(_ConfigBG)(true, p->bg_width, p->bg_height));
            hdmi_dpi_config_update();
            DPI_CHECK_RET(HDMI_DPI(_ConfigDualEdge)(true));
            ///DPI_CHECK_RET(HDMI_DPI(_EnableColorBar)());
            ///DPI_CHECK_RET(HDMI_DPI(_EnableClk)());

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
            bool first_post = false;

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
            spin_lock_bh(&hdmi_lock);
            hdmi_video_buffer_list *pBuffList = NULL;

            if (p->is_clock_on)
            {

                MMP_MetaDataBitmap_t Bitmap;
#ifdef MTK_HDMI_ION_SUPPORT
                pBuffList = hdmi_query_buf_mva(video_buffer_info.next_buff_idx);

                if (pBuffList)
                {
                    memcpy(&(pBuffList->buffer_info), &video_buffer_info, sizeof(video_buffer_info));

                    if (pBuffList->hnd != 0)
                    {
                        if (video_buffer_info.src_phy_addr != NULL)
                        {
                            HDMI_LOG("Warning: ion enable, but phy is not null \n");
                            MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buff_ION_Err1, 1);
                        }
                        else
                        {
                            HDMI_LOG("ion disable, bufflist %x, vir %x, phy %x \n", pBuffList, video_buffer_info.src_base_addr, video_buffer_info.src_phy_addr);
                        }

                        video_buffer_info.src_phy_addr = pBuffList->mva;
                        video_buffer_info.src_base_addr = pBuffList->va;
                        pBuffList->buffer_info.src_phy_addr = pBuffList->mva;
                        pBuffList->buffer_info.src_base_addr = pBuffList->va;
                    }
                }
                else
                {
                    spin_unlock_bh(&hdmi_lock);
                    MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buff_ION_Err1, 0);
                    HDMI_LOG("Warning: buffer list no buffers! \n");
                    r = -EFAULT;
                    break;
                }

#endif
                pBuffList->buf_state = insert_new;
                spin_unlock_bh(&hdmi_lock);

                if (dp_mutex_dst <= 0)
                {
                    atomic_set(&hdmi_rdma_config_event, 1);
                    wake_up_interruptible(&hdmi_rdma_config_wq);
                }

                if ((capture_screen == true) || (hdmi_bufferdump_on >= 0x7))
                {
                    Bitmap.data1 = video_buffer_info.src_width;
                    Bitmap.data2 = video_buffer_info.src_pitch;
                    Bitmap.width = video_buffer_info.src_width;
                    Bitmap.height = video_buffer_info.src_height;
                    Bitmap.format = MMProfileBitmapBGRA8888;
                    Bitmap.start_pos = (video_buffer_info.src_pitch - video_buffer_info.src_width) / 2 * 4;
                    Bitmap.pitch = video_buffer_info.src_pitch * 4;

                    Bitmap.data_size = Bitmap.pitch * Bitmap.height;
                    Bitmap.down_sample_x = 10;
                    Bitmap.down_sample_y = 10;
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
            else
            {
                spin_unlock_bh(&hdmi_lock);
            }

#ifndef MTK_HDMI_FENCE_SUPPORT
            HDMI_LOG("wait hdmi_rdma_config_wq \n");
            wait_event_interruptible(hdmi_rdma_config_wq, atomic_read(&hdmi_rdma_config_event));
            atomic_set(&hdmi_rdma_config_event, 0);
#else

            if (hdmi_bufferdump_on > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.BufferInsert, MMProfileFlagPulse, pBuffList, video_buffer_info.next_buff_idx);
            }


            ///up(&hdmi_update_mutex);
            if (hdmi_bufferdump_on > 0)
            {
                if (pBuffList)
                {
                    MMProfileLogEx(HDMI_MMP_Events.BufferPost, MMProfileFlagEnd, pBuffList, pBuffList->buffer_info.src_phy_addr);
                }
                else
                {
                    MMProfileLogEx(HDMI_MMP_Events.BufferPost, MMProfileFlagEnd, pBuffList, 0);
                }
            }

#endif

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
#if 1

            if (down_interruptible(&hdmi_update_mutex))
            {
                HDMI_LOG("[HDMI] can't get semaphore in\n");
                return EAGAIN;
            }

            hdmi_dpi_config_update();
            DPI_CHECK_RET(HDMI_DPI(_ConfigDualEdge(true)));
            ///DPI_CHECK_RET(HDMI_DPI(_ConfigPixelClk(DPI_POLARITY_FALLING, 0, 0)));


            DPI_CHECK_RET(HDMI_DPI(_EnableColorBar()));
            DPI_CHECK_RET(HDMI_DPI(_EnableClk()));

            DPI_CHECK_RET(HDMI_DPI(_DumpRegisters()));

            up(&hdmi_update_mutex);
#endif
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

            hdmi_video_buffer_list *pBuffList = NULL;
            hdmi_buffer.fence_fd = MTK_HDMI_NO_FENCE_FD;

            if (p->is_clock_on)
            {
                data = hdmi_create_fence();

                struct ion_handle *handle = NULL;
                unsigned int mva = 0x0;
                unsigned int va = 0x0;

                pBuffList = kmalloc(sizeof(hdmi_video_buffer_list), GFP_KERNEL);
                memset(pBuffList, 0 , sizeof(hdmi_video_buffer_list));
#ifdef MTK_HDMI_ION_SUPPORT

                if (!ion_client)
                {
                    hdmi_ion_init();

                    if (!ion_client)
                    {
                        HDMI_LOG(": get ion_client fail \n", ion_client);
                        r = -EFAULT;
                        up(&hdmi_update_mutex);
                        break;
                    }
                }

                handle = hdmi_ion_import_handle(ion_client, hdmi_buffer.ion_fd);
                hdmi_ion_phys_mmu_addr(ion_client, handle, &mva);
                va = ion_map_kernel(ion_client, handle);
#endif
                spin_lock_bh(&hdmi_lock);
                pBuffList->buf_state = create_new;
                pBuffList->fence = data.fence;
                pBuffList->idx = data.value;
                pBuffList->hnd = handle;
                pBuffList->mva = mva;
                pBuffList->va = va;
                hdmi_buffer.fence_fd = data.fence;
                hdmi_buffer.index = data.value;
                INIT_LIST_HEAD(&pBuffList->list);
                list_add_tail(&pBuffList->list, &HDMI_Buffer_List);
                spin_unlock_bh(&hdmi_lock);
                HDMI_LOG(": add list :%x, index %d, fd %d\n", pBuffList, pBuffList->idx, pBuffList->fence);

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

            if (hdmi_bufferdump_on > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.FenceCreate, MMProfileFlagEnd, hdmi_buffer.fence_fd, pBuffList);
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

        default:
        {
            HDMI_LOG("[hdmi][HDMI] arguments error %x, %x\n", MTK_HDMI_GET_DEV_INFO, cmd);
            break;
        }
    }

    return r;
}


static int hdmi_remove(struct platform_device *pdev)
{
    return 0;
}

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

    init_waitqueue_head(&hdmi_video_mode_wq);
    INIT_LIST_HEAD(&hdmi_video_mode_buffer_list);

    init_waitqueue_head(&hdmi_rdma_config_wq);
    init_waitqueue_head(&hdmi_rdma_update_wq);
    init_waitqueue_head(&reg_update_wq);
    init_waitqueue_head(&dst_reg_update_wq);

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
    INIT_LIST_HEAD(&HDMI_Buffer_List);
#endif
    int tmp_boot_mode = get_boot_mode();

    if ((tmp_boot_mode == FACTORY_BOOT) || (tmp_boot_mode == ATE_FACTORY_BOOT))
    {
        factory_mode = true;
    }

    return 0;
}

module_init(hdmi_init);
module_exit(hdmi_exit);
MODULE_AUTHOR("Xuecheng, Zhang <xuecheng.zhang@mediatek.com>");
MODULE_DESCRIPTION("HDMI Driver");
MODULE_LICENSE("GPL");

#endif
