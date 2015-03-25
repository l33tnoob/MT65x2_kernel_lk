#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>

#include <linux/xlog.h>
#include <linux/proc_fs.h>  //proc file use
//ION
#include <linux/ion.h>
#include <linux/ion_drv.h>
#include <mach/m4u.h>

#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>

#include <asm/io.h>


#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h> // ????
#include <mach/mt_irq.h>
#include <mach/sync_write.h>

#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_hal.h"
#include "ddp_path.h"
#include "ddp_debug.h"
#include "ddp_color.h"
#include "disp_drv_ddp.h"
#include "ddp_wdma.h"
#include "ddp_cmdq.h"
#include "ddp_bls.h"

//#include <asm/tcm.h>

#define DDP_GAMMA_SUPPORT


unsigned int dbg_log = 0;
unsigned int irq_log = 0;
#define DISP_DEVNAME "mtk_disp"
// device and driver
static dev_t disp_devno;
static struct cdev *disp_cdev;
static struct class *disp_class = NULL;

//ION

unsigned char ion_init=0;
unsigned char dma_init=0;

//NCSTool for Color Tuning
unsigned char ncs_tuning_mode = 0;

//flag for gamma lut update
unsigned char bls_gamma_dirty = 0;

struct ion_client *cmdqIONClient;
struct ion_handle *cmdqIONHandle;
struct ion_mm_data mm_data;
unsigned long * cmdq_pBuffer;
unsigned int cmdq_pa;
unsigned int cmdq_pa_len;
struct ion_sys_data sys_data;
M4U_PORT_STRUCT m4uPort;
//irq
#define DISP_REGISTER_IRQ(irq_num){\
    if(request_irq( irq_num , (irq_handler_t)disp_irq_handler, IRQF_TRIGGER_LOW, DISP_DEVNAME , NULL))\
    { DDP_DRV_ERR("ddp register irq failed! %d\n", irq_num); }}

//-------------------------------------------------------------------------------//
// global variables
typedef struct
{
    spinlock_t irq_lock;
    unsigned int irq_src;  //one bit represent one module
} disp_irq_struct;

typedef struct
{
    pid_t open_pid;
    pid_t open_tgid;
    struct list_head testList;
    unsigned int u4LockedResource;
    unsigned int u4Clock;
    spinlock_t node_lock;
} disp_node_struct;

#define DISP_MAX_IRQ_CALLBACK   10
static DDP_IRQ_CALLBACK g_disp_irq_table[DISP_MODULE_MAX][DISP_MAX_IRQ_CALLBACK];

disp_irq_struct g_disp_irq;
static DECLARE_WAIT_QUEUE_HEAD(g_disp_irq_done_queue);

// cmdq thread

unsigned char cmdq_thread[CMDQ_THREAD_NUM] = {1, 1, 1, 1, 1, 1, 1};
spinlock_t gCmdqLock;
extern spinlock_t gCmdqMgrLock;
extern unsigned int gMutexID;
extern unsigned char aal_debug_flag;

wait_queue_head_t cmq_wait_queue[CMDQ_THREAD_NUM];
//unsigned char cmq_status[CMDQ_THREAD_NUM];
static wait_queue_head_t cmq_exec_wait_queue;
unsigned char cmq_exec_status = 1;

//G2d Variables
spinlock_t gResourceLock;
unsigned int gLockedResource;//lock dpEngineType_6572

static DECLARE_WAIT_QUEUE_HEAD(gResourceWaitQueue);

// Overlay Variables
spinlock_t gOvlLock;
int disp_run_dp_framework = 0;
int disp_layer_enable = 0;
int disp_mutex_status = 0;

DISP_OVL_INFO disp_layer_info[DDP_OVL_LAYER_MUN];

//AAL variables
static unsigned long u4UpdateFlag = 0;

//Register update lock
spinlock_t gRegisterUpdateLock;
spinlock_t gPowerOperateLock;
#ifdef DDP_82_72_TODO
//Clock gate management
static unsigned long g_u4ClockOnTbl = 0;
#endif

//PQ variables
extern UINT32 fb_width;
extern UINT32 fb_height;

// IRQ log print kthread
static struct task_struct *disp_irq_log_task = NULL;
static wait_queue_head_t disp_irq_log_wq;
static int disp_irq_log_module = 0;

static DISPLAY_TDSHP_T g_TDSHP_Index; 

DISPLAY_TDSHP_T *get_TDSHP_index(void)  
{    
    return &g_TDSHP_Index;
}


// internal function
static int disp_wait_intr(DISP_MODULE_ENUM module, unsigned int timeout_ms);
#if 0
static int disp_set_overlay_roi(int layer, int x, int y, int w, int h, int pitch);
static int disp_set_overlay_addr(int layer, unsigned int addr, DpColorFormat fmt);
static int disp_set_overlay(int layer, int enable);
static int disp_is_dp_framework_run(void);
static int disp_set_mutex_status(int enable);
#endif
static int disp_get_mutex_status(void);
static int disp_set_needupdate(DISP_MODULE_ENUM eModule , unsigned long u4En);
static void disp_power_off(DISP_MODULE_ENUM eModule , unsigned int * pu4Record);
static void disp_power_on(DISP_MODULE_ENUM eModule , unsigned int * pu4Record);
extern void DpEngine_COLORonConfig(unsigned int srcWidth,unsigned int srcHeight);
extern void DpEngine_COLORonInit(void);

extern void cmdqForceFreeAll(int cmdqThread);
extern void cmdqForceFree_SW(int taskID);

#if 0
struct disp_path_config_struct
{
    DISP_MODULE_ENUM srcModule;
    unsigned int addr; 
    unsigned int inFormat; 
    unsigned int pitch;
    struct DISP_REGION srcROI;        // ROI

    unsigned int layer;
    bool layer_en;
    enum OVL_LAYER_SOURCE source; 
    struct DISP_REGION bgROI;         // background ROI
    unsigned int bgColor;  // background color
    unsigned int key;     // color key
    bool aen;             // alpha enable
    unsigned char alpha;

    DISP_MODULE_ENUM dstModule;
    unsigned int outFormat; 
    unsigned int dstAddr;  // only take effect when dstModule=DISP_MODULE_WDMA1
};
int disp_path_enable();
int disp_path_config(struct disp_path_config_struct* pConfig);
#endif

unsigned int* pRegBackup = NULL;

//-------------------------------------------------------------------------------//
// functions
#if 0
static void cmdq_ion_init(void)
{

    //ION
        DDP_DRV_DBG("DISP ION: ion_client_create 1:%x",(unsigned int)g_ion_device);

        cmdqIONClient = ion_client_create(g_ion_device, "cmdq");
        DDP_DRV_DBG("DISP ION: ion_client_create cmdqIONClient...%x.\n", (unsigned int)cmdqIONClient);
        if (IS_ERR_OR_NULL(cmdqIONClient))
        {
            DDP_DRV_ERR("DISP ION: Couldn't create ion client\n");
        }

        DDP_DRV_DBG("DISP ION: Create ion client done\n");
        
        cmdqIONHandle = ion_alloc(cmdqIONClient, CMDQ_ION_BUFFER_SIZE, 4, ION_HEAP_MULTIMEDIA_MASK, 0);
        
        if (IS_ERR_OR_NULL(cmdqIONHandle))
        {
            DDP_DRV_ERR("DISP ION: Couldn't alloc ion buffer\n");
        }

        DDP_DRV_DBG("DISP ION:  ion alloc done\n");

        mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
        mm_data.config_buffer_param.handle = cmdqIONHandle;
        mm_data.config_buffer_param.eModuleID = M4U_CLNTMOD_CMDQ;
        mm_data.config_buffer_param.security = 0;
        mm_data.config_buffer_param.coherent = 0;

        if(ion_kernel_ioctl(cmdqIONClient, ION_CMD_MULTIMEDIA, (unsigned long)&mm_data))
        {
            DDP_DRV_ERR("DISP ION: Couldn't config ion buffer\n");
        }

        DDP_DRV_DBG("DISP ION:  ion config done\n");

        cmdq_pBuffer = ion_map_kernel(cmdqIONClient, cmdqIONHandle);
        if(IS_ERR_OR_NULL(cmdq_pBuffer))
        {        
            DDP_DRV_ERR("DISP ION: Couldn't get ion buffer VA\n");
        }

        DDP_DRV_DBG("DISP ION:  ion VA done\n");

        if(ion_phys(cmdqIONClient,cmdqIONHandle, (unsigned long)&cmdq_pa, &cmdq_pa_len))
        {
            DDP_DRV_ERR("DISP ION: Couldn't get ion buffer MVA\n");
        }

        DDP_DRV_DBG("DISP ION:  ion MVA done\n");

        m4uPort.ePortID = M4U_PORT_CMDQ;
        m4uPort.Virtuality = 1;
        m4uPort.Security = 0;
        m4uPort.Distance = 1;
        m4uPort.Direction = 0;
        //m4uPort.Domain = 3;
        m4u_config_port(&m4uPort);

        DDP_DRV_DBG("DISP ION:  Config MVA port done\n");
        
        //ION TEST
        DDP_DRV_DBG("CMDQ ION buffer VA: 0x%lx MVA: 0x%x \n", (unsigned long)cmdq_pBuffer, (unsigned int)cmdq_pa);

        cmdqBufferTbl_init((unsigned long) cmdq_pBuffer, (unsigned int) cmdq_pa);

}
#endif


void cmdq_ion_flush(void)
{
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = cmdqIONHandle;
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_ALL;

    if(ion_kernel_ioctl(cmdqIONClient, ION_CMD_SYSTEM ,(unsigned long)&sys_data))
    {
        DDP_DRV_ERR("CMDQ ION flush fail\n");
    }
}


static void cmdq_dma_init(void)
{
    cmdq_pBuffer= dma_alloc_coherent(NULL, CMDQ_ION_BUFFER_SIZE, &cmdq_pa, GFP_KERNEL);

    if(!cmdq_pBuffer)
    {
        DDP_DRV_ERR("dma_alloc_coherent error!  dma memory not available.\n");
        return;
    }

    memset((void*)cmdq_pBuffer, 0, CMDQ_ION_BUFFER_SIZE);
    DDP_DRV_DBG("dma_alloc_coherent success VA:%x PA:%x \n", (unsigned int)cmdq_pBuffer, (unsigned int)cmdq_pa);


    cmdqBufferTbl_init((unsigned long) cmdq_pBuffer, (unsigned int) cmdq_pa);
    
}

static int disp_irq_log_kthread_func(void *data)
{
    unsigned int i=0;
    while(1)
    {
        wait_event_interruptible(disp_irq_log_wq, disp_irq_log_module);
        DDP_DRV_INFO("disp_irq_log_kthread_func dump intr register: disp_irq_log_module=%d \n", disp_irq_log_module);
        for(i=0;i<DISP_MODULE_MAX;i++)
        {
            if( (disp_irq_log_module&(1<<i))!=0 )
            {
                disp_dump_reg(i);
            }
        }
        // reset wakeup flag
        disp_irq_log_module = 0;
    }

    return 0;
}

unsigned int disp_ms2jiffies(unsigned long ms)
{
    return ((ms*HZ + 512) >> 10);
}

int disp_lock_cmdq_thread(void)
{
    int i=0;

    DDP_DRV_INFO("disp_lock_cmdq_thread()called \n");
    
    spin_lock(&gCmdqLock);
    for (i = 0; i < CMDQ_THREAD_NUM; i++)
    {
        if (cmdq_thread[i] == 1) 
        {
            cmdq_thread[i] = 0;
            break;
        }
    } 
    spin_unlock(&gCmdqLock);

    DDP_DRV_INFO("disp_lock_cmdq_thread(), i=%d \n", i);

    return (i>=CMDQ_THREAD_NUM)? -1 : i;
    
}

int disp_unlock_cmdq_thread(unsigned int idx)
{
    if(idx >= CMDQ_THREAD_NUM)
        return -1;

    spin_lock(&gCmdqLock);        
    cmdq_thread[idx] = 1;  // free thread availbility
    spin_unlock(&gCmdqLock);

    return 0;
}

// if return is not 0, should wait again
static int disp_wait_intr(DISP_MODULE_ENUM module, unsigned int timeout_ms)
{
    int ret;
    unsigned long flags;

    unsigned long long end_time = 0;
        
    MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagStart, 0, module);
    // wait until irq done or timeout
    ret = wait_event_interruptible_timeout(
                    g_disp_irq_done_queue, 
                    g_disp_irq.irq_src & (1<<module), 
                    disp_ms2jiffies(timeout_ms) );
                    
    /*wake-up from sleep*/
    if(ret==0) // timeout
    {
        MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagPulse, 0, module);
        MMProfileLog(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd);
        DDP_DRV_ERR("Wait Done Timeout! pid=%d, module=%d \n", current->pid ,module);
        if(module==DISP_MODULE_WDMA0)
        {
            DDP_DRV_ERR("======== WDMA0 timeout, dump all registers! \n");
            disp_dump_reg(DISP_MODULE_WDMA0);
            disp_dump_reg(DISP_MODULE_CONFIG);
        }
        else
        {
            disp_dump_reg(module);
        }
        ASSERT(0);

        return -EAGAIN;        
    }
    else if(ret<0) // intr by a signal
    {
        MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagPulse, 1, module);
        MMProfileLog(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd);
        DDP_DRV_ERR("Wait Done interrupted by a signal! pid=%d, module=%d \n", current->pid ,module);
        disp_dump_reg(module);
        ASSERT(0);
        return -EAGAIN;                 
    }

    MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd, 0, module);
    spin_lock_irqsave( &g_disp_irq.irq_lock , flags );
    g_disp_irq.irq_src &= ~(1<<module);    
    spin_unlock_irqrestore( &g_disp_irq.irq_lock , flags );

    end_time = sched_clock();
   	//DDP_DRV_INFO("**ROT_SCL_WDMA0 execute %d us\n", ((unsigned int)end_time-(unsigned int)start_time)/1000);
    
              
    return 0;
}

int disp_register_irq(DISP_MODULE_ENUM module, DDP_IRQ_CALLBACK cb)
{
    int i;
    if (module >= DISP_MODULE_MAX)
    {
        DDP_DRV_ERR("Register IRQ with invalid module ID. module=%d\n", module);
        return -1;
    }
    if (cb == NULL)
    {
        DDP_DRV_ERR("Register IRQ with invalid cb.\n");
        return -1;
    }
    for (i=0; i<DISP_MAX_IRQ_CALLBACK; i++)
    {
        if (g_disp_irq_table[module][i] == cb)
            break;
    }
    if (i < DISP_MAX_IRQ_CALLBACK)
    {
        // Already registered.
        return 0;
    }
    for (i=0; i<DISP_MAX_IRQ_CALLBACK; i++)
    {
        if (g_disp_irq_table[module][i] == NULL)
            break;
    }
    if (i == DISP_MAX_IRQ_CALLBACK)
    {
        DDP_DRV_ERR("No enough callback entries for module %d.\n", module);
        return -1;
    }
    g_disp_irq_table[module][i] = cb;
    return 0;
}

int disp_unregister_irq(DISP_MODULE_ENUM module, DDP_IRQ_CALLBACK cb)
{
    int i;
    for (i=0; i<DISP_MAX_IRQ_CALLBACK; i++)
    {
        if (g_disp_irq_table[module][i] == cb)
        {
            g_disp_irq_table[module][i] = NULL;
            break;
        }
    }
    if (i == DISP_MAX_IRQ_CALLBACK)
    {
        DDP_DRV_ERR("Try to unregister callback function with was not registered. module=%d cb=0x%08X\n", module, (unsigned int)cb);
        return -1;
    }
    return 0;
}

void disp_invoke_irq_callbacks(DISP_MODULE_ENUM module, unsigned int param)
{
    int i;
    for (i=0; i<DISP_MAX_IRQ_CALLBACK; i++)
    {
        if (g_disp_irq_table[module][i])
        {
            //DDP_DRV_ERR("Invoke callback function. module=%d param=0x%X\n", module, param);
            g_disp_irq_table[module][i](param);
        }
    }
}
#if defined(MTK_HDMI_SUPPORT)
extern void hdmi_setorientation(int orientation);
void hdmi_power_on(void);
void hdmi_power_off(void);
extern void hdmi_update_buffer_switch(void);
extern bool is_hdmi_active(void);
extern void hdmi_update(void);
extern void hdmi_source_buffer_switch(void);
#endif
#if  defined(MTK_WFD_SUPPORT)
extern void hdmi_setorientation(int orientation);
void hdmi_power_on(void);
void hdmi_power_off(void);
extern void wfd_update_buffer_switch(void);
extern bool is_wfd_active(void);
extern void wfd_update(void);
extern void wfd_source_buffer_switch(void);
#endif

//extern void hdmi_test_switch_buffer(void);

unsigned int cnt_rdma_underflow = 1;
unsigned int cnt_rdma_abnormal = 1;
unsigned int cnt_ovl_underflow = 1;
unsigned int cnt_ovl_eof = 1;

static /*__tcmfunc*/ irqreturn_t disp_irq_handler(int irq, void *dev_id)
{
    unsigned long reg_val; 
    int i;
    int taskid;
    extern int cmdqThreadTaskList[CMDQ_THREAD_NUM][CMDQ_THREAD_LIST_LENGTH];
    extern unsigned int cmdqThreadTaskList_R[CMDQ_THREAD_NUM]; 

    DDP_DRV_IRQ("irq=%d, 0x%x, 0x%x, 0x%x, 0x%x \n", 
                     irq, 
                     DISP_REG_GET(DISP_REG_OVL_INTSTA), 
                     DISP_REG_GET(DISP_REG_WDMA_INTSTA), 
                     DISP_REG_GET(DISP_REG_RDMA_INT_STATUS),
                     DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA));

       
    /*1. Process ISR*/
    switch(irq)
    {
            
        case MT_DISP_OVL_IRQ_ID:  
                reg_val = DISP_REG_GET(DISP_REG_OVL_INTSTA);
                if(reg_val&(1<<0))
                {
                      DDP_DRV_IRQ("IRQ: OVL reg update done! \n");
                }    
                if(reg_val&(1<<1))
                {
                      DDP_DRV_IRQ("IRQ: OVL frame done! \n");
                      g_disp_irq.irq_src |= (1<<DISP_MODULE_OVL);
                }
                if(reg_val&(1<<2))
                {
                      DDP_DRV_ERR("IRQ: OVL frame underrun! \n");
                }
                if(reg_val&(1<<3))
                {
                      DDP_DRV_IRQ("IRQ: OVL SW reset done! \n");
                }
                if(reg_val&(1<<4))
                {
                      DDP_DRV_IRQ("IRQ: OVL HW reset done! \n");
                }
                if(reg_val&(1<<5))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA0 not complete untill EOF! \n");
                }      
                if(reg_val&(1<<6))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA1 not complete untill EOF! \n");
                }
                if(reg_val&(1<<7))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA2 not complete untill EOF! \n");
                }        
                if(reg_val&(1<<8))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA3 not complete untill EOF! \n");
                }
                if(reg_val&(1<<9))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA0 fifo underflow! \n");
                }      
                if(reg_val&(1<<10))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA1 fifo underflow! \n");
                }
                if(reg_val&(1<<11))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA2 fifo underflow! \n");
                }    
                if(reg_val&(1<<12))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA3 fifo underflow! \n");
                }                                                                                                  
                //clear intr
                DISP_REG_SET(DISP_REG_OVL_INTSTA, ~reg_val);     
                MMProfileLogEx(DDP_MMP_Events.OVL_IRQ, MMProfileFlagPulse, reg_val, 0);
                disp_invoke_irq_callbacks(DISP_MODULE_OVL, reg_val);
            break;
            
        case MT_DISP_WDMA_IRQ_ID:
                reg_val = DISP_REG_GET(DISP_REG_WDMA_INTSTA);
                if(reg_val&(1<<0))
                {
                    DDP_DRV_IRQ("IRQ: WDMA0 frame done! \n");
                    g_disp_irq.irq_src |= (1<<DISP_MODULE_WDMA0);
                }    
                if(reg_val&(1<<1))
                {
                      DDP_DRV_ERR("IRQ: WDMA0 underrun! \n");

                }  
                //clear intr
                DISP_REG_SET(DISP_REG_WDMA_INTSTA, ~reg_val);           
                MMProfileLogEx(DDP_MMP_Events.WDMA0_IRQ, MMProfileFlagPulse, reg_val, DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE));
                disp_invoke_irq_callbacks(DISP_MODULE_WDMA0, reg_val);
            break;
            

        case MT_DISP_RDMA_IRQ_ID:
                reg_val = DISP_REG_GET(DISP_REG_RDMA_INT_STATUS);
                if(reg_val&(1<<0))
                {
                      DDP_DRV_IRQ("IRQ: RDMA0 reg update done! \n");
                }    
                if(reg_val&(1<<1))
                {
                      DDP_DRV_IRQ("IRQ: RDMA0 frame start! \n");
                      if(disp_needWakeUp())
                      {
                          disp_update_hist();
                          disp_wakeup_aal();
                      }
                      on_disp_aal_alarm_set();
                }
                if(reg_val&(1<<2))
                {
                      DDP_DRV_IRQ("IRQ: RDMA0 frame done! \n");
                      g_disp_irq.irq_src |= (1<<DISP_MODULE_RDMA0);
                }
                if(reg_val&(1<<3))
                {
                      if(cnt_rdma_abnormal)
                      {
                          DDP_DRV_ERR("IRQ: RDMA0 abnormal! \n");
                          cnt_rdma_abnormal = 0;
                      }
                }
                if(reg_val&(1<<4))
                {
                      if(cnt_rdma_underflow)
                      {
                          DDP_DRV_ERR("IRQ: RDMA0 underflow! \n");
                          cnt_rdma_underflow = 0;
                      }
                }
                //clear intr
                DISP_REG_SET(DISP_REG_RDMA_INT_STATUS, ~reg_val);           
                MMProfileLogEx(DDP_MMP_Events.RDMA0_IRQ, MMProfileFlagPulse, reg_val, 0);
                disp_invoke_irq_callbacks(DISP_MODULE_RDMA0, reg_val);
            break;  

        case MT_DISP_COLOR_IRQ_ID:
            reg_val = DISP_REG_GET(DISPSYS_COLOR_BASE+0x0F08);

            // read LUMA histogram
            if (reg_val & 0x2)
            {
//TODO : might want to move to other IRQ~ -S       
                //disp_update_hist();
                //disp_wakeup_aal();
//TODO : might want to move to other IRQ~ -E
            }

            //clear intr
            DISP_REG_SET(DISPSYS_COLOR_BASE+0x0F08, ~reg_val);
            MMProfileLogEx(DDP_MMP_Events.COLOR_IRQ, MMProfileFlagPulse, reg_val, 0);
//            disp_invoke_irq_callbacks(DISP_MODULE_COLOR, reg_val);
            break;
                        
        case MT_DISP_BLS_IRQ_ID:
            reg_val = DISP_REG_GET(DISP_REG_BLS_INTSTA);

            // read LUMA & MAX(R,G,B) histogram
            if (reg_val & 0x1)
            {
                //disp_update_hist();
                //disp_wakeup_aal();
            }

            //clear intr
            DISP_REG_SET(DISP_REG_BLS_INTSTA, ~reg_val);
            MMProfileLogEx(DDP_MMP_Events.BLS_IRQ, MMProfileFlagPulse, reg_val, 0);
            break;

        case MT_MM_MUTEX_IRQ_ID:  // can not do reg update done status after release mutex(for ECO requirement), 
                                        // so we have to check update timeout intr here
            reg_val = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA);
            
#if 0
            if(reg_val&0xFF00) // udpate timeout intr triggered
            {
                unsigned int reg = 0;
                unsigned int mutexID = 0;

                for(mutexID=0;mutexID<4;mutexID++)
                {
                    if((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<(mutexID+8))) == (1<<(mutexID+8)))
                    {
                        DDP_DRV_ERR("disp_path_release_mutex() timeout! \n");
                        disp_dump_reg(DISP_MODULE_CONFIG);
                        disp_dump_reg(DISP_MODULE_MUTEX);
                        //print error engine
                        reg = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
                        if(reg!=0)
                        {
                            if(reg&(1<<3))  { DDP_DRV_INFO(" OVL update reg timeout! \n"); disp_dump_reg(DISP_MODULE_OVL); }
                            if(reg&(1<<7))  { DDP_DRV_INFO(" COLOR update reg timeout! \n");    disp_dump_reg(DISP_MODULE_COLOR); }
                            if(reg&(1<<6))  { DDP_DRV_INFO(" WDMA0 update reg timeout! \n"); disp_dump_reg(DISP_MODULE_WDMA0); }
                            if(reg&(1<<10))  { DDP_DRV_INFO(" RDMA1 update reg timeout! \n"); disp_dump_reg(DISP_MODULE_RDMA0); }
                            if(reg&(1<<9))  { DDP_DRV_INFO(" BLS update reg timeout! \n"); disp_dump_reg(DISP_MODULE_BLS); }
                        }  
                        ASSERT(0);

                        //reset mutex
                        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(mutexID), 1);
                        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(mutexID), 0);
                        DDP_DRV_INFO("mutex reset done! \n");
                    }
                 }
            }            
#else
            if(reg_val&0xFF00) // udpate timeout intr triggered
            {
                unsigned int reg = 0;

                 if((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<(gMutexID+8))) == (1<<(gMutexID+8)))
                 {
                     DDP_DRV_ERR("disp_path_release_mutex() timeout! \n");
                     //disp_dump_reg(DISP_MODULE_CONFIG);
                     
                     //disp_dump_reg(DISP_MODULE_MUTEX);
                     //print error engine
                     reg = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
                     if(reg!=0)
                     {
                         if(reg&(1<<3))  { DDP_DRV_INFO(" OVL update reg timeout! \n"); }
                         if(reg&(1<<7))  { DDP_DRV_INFO(" COLOR update reg timeout! \n"); }
                         if(reg&(1<<6))  { DDP_DRV_INFO(" WDMA0 update reg timeout! \n"); }
                         if(reg&(1<<10))  { DDP_DRV_INFO(" RDMA1 update reg timeout! \n"); }
                         if(reg&(1<<9))  { DDP_DRV_INFO(" BLS update reg timeout! \n"); }
                     }  
                     //ASSERT(0);

                     //reset ovl
                     OVLReset();

                     //reset mutex
                     DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 1);
                     DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 0);

                     reg = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA);
                     reg &= ~(1 << gMutexID);
                     reg &= ~(1 << (gMutexID + 8));
                     DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, reg);

                     DDP_DRV_INFO("mutex reset done! \n");
                 }
            }            
#endif

            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, ~reg_val);      
            MMProfileLogEx(DDP_MMP_Events.Mutex_IRQ, MMProfileFlagPulse, reg_val, 0);
            disp_invoke_irq_callbacks(DISP_MODULE_MUTEX, reg_val);
            break;
            
        case MT_CMDQ_IRQ_ID:
            for(i = 0 ; i < CMDQ_THREAD_NUM ; i++)
            {
                reg_val = DISP_REG_GET(DISP_REG_CMDQ_THRx_IRQ_FLAG(i)); 
                DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG(i), ~reg_val);
                if( reg_val != 0 )
                {
                  
                    taskid = cmdqThreadTaskList[i][cmdqThreadTaskList_R[i]];               
                    //printk("\n\n!!!!!!!!!!!!!!!!!!!!!CMQ Thread %d Complete Task %d!!!!!!!!!!!!!!!!!!!\n\n\n", i, taskid);
                    if(reg_val & (1 << 1))
                    {
                        DDP_DRV_ERR("\n\n\n!!!!!!!!!!!!!!!IRQ: CMQ %d Time out! !!!!!!!!!!!!!!!!\n\n\n", i);
                        spin_lock(&gCmdqMgrLock);
                        cmdqForceFreeAll(i);
                        smp_wmb();
                        spin_unlock(&gCmdqMgrLock);
                        
                    }
                    else if(reg_val & (1 << 4))
                    {
                        DDP_DRV_ERR("IRQ: CMQ thread%d Invalid Command Instruction! \n", i);                        
                        spin_lock(&gCmdqMgrLock);
                        cmdqForceFreeAll(i);
                        smp_wmb();
                        spin_unlock(&gCmdqMgrLock);

                    }
                    else if(reg_val == 0x1)// Normal EOF end
                    {
                        unsigned long long end_time = 0;
                        unsigned long long start_time = sched_clock();
                        unsigned long cost = 0;
   	
                        spin_lock(&gCmdqMgrLock);
                        //CMDQ_CHECK_TIME(spin_lock_gCmdqMgrLock);
                        
                        cmdqThreadComplete(i, true); //Thread i complete!

                        CMDQ_CHECK_TIME(cmdqThreadComplete);
                        
                        smp_wmb();

                        //CMDQ_CHECK_TIME(smp_wmb);
                        
                        wake_up_interruptible(&cmq_wait_queue[i]);

                        //CMDQ_CHECK_TIME(wake_up_interruptible);
                        
                        spin_unlock(&gCmdqMgrLock);
                    }     
//                    cmq_status[i] = 1;

                    MMProfileLogEx(DDP_MMP_Events.CMDQ_IRQ, MMProfileFlagPulse, reg_val, i);
                }
            }
            break;
#if 0      
        case MT6572_G2D_IRQ_ID:
            reg_val = DISP_REG_GET(DISP_REG_G2D_IRQ);
            if(reg_val&G2D_IRQ_STA_BIT)
            {
				  unsigned long set_val = reg_val & ~(G2D_IRQ_STA_BIT); 
                  DDP_DRV_IRQ("IRQ: G2D done! \n");
				  g_disp_irq.irq_src |= (1<<DISP_MODULE_G2D);
				  //clear intr
				  DISP_REG_SET(DISP_REG_G2D_IRQ, set_val);
            }                                   
            
            disp_invoke_irq_callbacks(DISP_MODULE_G2D, reg_val);
            break;			
#endif
        default: DDP_DRV_ERR("invalid irq=%d \n ", irq); break;
    }        

    // Wakeup event
    mb();   // Add memory barrier before the other CPU (may) wakeup
    wake_up_interruptible(&g_disp_irq_done_queue);    
             
    return IRQ_HANDLED;
}


static void disp_power_on(DISP_MODULE_ENUM eModule , unsigned int * pu4Record)
{  
    unsigned long flag;
#ifdef DDP_82_72_TODO
    unsigned int ret = 0;
#endif

    spin_lock_irqsave(&gPowerOperateLock , flag);

#ifdef DDP_82_72_TODO
    if((1 << eModule) & g_u4ClockOnTbl)
    {
        DDP_DRV_INFO("DDP power %lu is already enabled\n" , (unsigned long)eModule);
    }
    else
    {
        switch(eModule)
        {

            case DISP_MODULE_WDMA0 :
                enable_clock(MT_CG_DISP0_WDMA0_ENGINE , "DDP_DRV");
                enable_clock(MT_CG_DISP0_WDMA0_SMI , "DDP_DRV");
            break;
 
            case DISP_MODULE_G2D :
                enable_clock(MT_CG_DISP0_G2D_ENGINE , "DDP_DRV");
                enable_clock(MT_CG_DISP0_G2D_SMI , "DDP_DRV");
            break;
            default :
                DDP_DRV_ERR("disp_power_on:unknown module:%d\n" , eModule);
                ret = -1;
            break;
        }

        if(0 == ret)
        {
            if(0 == g_u4ClockOnTbl)
            {
                enable_clock(MT_CG_DISP0_LARB2_SMI , "DDP_DRV");
            }
            g_u4ClockOnTbl |= (1 << eModule);
            *pu4Record |= (1 << eModule);
        }
    }
#endif

    spin_unlock_irqrestore(&gPowerOperateLock , flag);
}

static void disp_power_off(DISP_MODULE_ENUM eModule , unsigned int * pu4Record)
{  
    unsigned long flag;
#ifdef DDP_82_72_TODO
    unsigned int ret = 0;
#endif

    spin_lock_irqsave(&gPowerOperateLock , flag);
    
#ifdef DDP_82_72_TODO
//    DDP_DRV_INFO("power off : %d\n" , eModule);

    if((1 << eModule) & g_u4ClockOnTbl)
    {
        switch(eModule)
        {
            case DISP_MODULE_WDMA0 :
                WDMAStop(0);
                WDMAReset(0);
                disable_clock(MT_CG_DISP0_WDMA0_ENGINE , "DDP_DRV");
                disable_clock(MT_CG_DISP0_WDMA0_SMI , "DDP_DRV");
            break;
            case DISP_MODULE_G2D :
                disable_clock(MT_CG_DISP0_G2D_ENGINE , "DDP_DRV");
                disable_clock(MT_CG_DISP0_G2D_SMI , "DDP_DRV");
            break;            
            default :
                DDP_DRV_ERR("disp_power_off:unsupported format:%d\n" , eModule);
                ret = -1;
            break;
        }

        if(0 == ret)
        {
            g_u4ClockOnTbl &= (~(1 << eModule));
            *pu4Record &= (~(1 << eModule));

            if(0 == g_u4ClockOnTbl)
            {
                disable_clock(MT_CG_DISP0_LARB2_SMI , "DDP_DRV");
            }

        }
    }
    else
    {
        DDP_DRV_INFO("DDP power %lu is already disabled\n" , (unsigned long)eModule);
    }

#endif

    spin_unlock_irqrestore(&gPowerOperateLock , flag);
}

unsigned int inAddr=0, outAddr=0;

static int disp_set_needupdate(DISP_MODULE_ENUM eModule , unsigned long u4En)
{
    unsigned long flag;
    spin_lock_irqsave(&gRegisterUpdateLock , flag);

    if(u4En)
    {
        u4UpdateFlag |= (1 << eModule);
    }
    else
    {
        u4UpdateFlag &= ~(1 << eModule);
    }

    spin_unlock_irqrestore(&gRegisterUpdateLock , flag);

    return 0;
}

void DISP_REG_SET_FIELD(unsigned long field, unsigned long reg32, unsigned long val)
{
    unsigned long flag;
    spin_lock_irqsave(&gRegisterUpdateLock , flag);
    //*(volatile unsigned int*)(reg32) = ((*(volatile unsigned int*)(reg32) & ~(REG_FLD_MASK(field))) |  REG_FLD_VAL((field), (val)));
     mt65xx_reg_sync_writel( (*(volatile unsigned int*)(reg32) & ~(REG_FLD_MASK(field)))|REG_FLD_VAL((field), (val)), reg32);
     spin_unlock_irqrestore(&gRegisterUpdateLock , flag);
}

int CheckAALUpdateFunc(int i4IsNewFrame)
{
    return (((1 << DISP_MODULE_BLS) & u4UpdateFlag) || i4IsNewFrame || is_disp_aal_alarm_on()) ? 1 : 0;
}

int ConfAALFunc(int i4IsNewFrame)
{
    disp_onConfig_aal(i4IsNewFrame);
    disp_set_needupdate(DISP_MODULE_BLS , 0);
    return 0;
}

static int AAL_init = 0;
void disp_aal_lock()
{
    if(0 == AAL_init)
    {
        //printk("disp_aal_lock: register update func\n");
        DISP_RegisterExTriggerSource(CheckAALUpdateFunc , ConfAALFunc);
        AAL_init = 1;
    }
    GetUpdateMutex();
}

void disp_aal_unlock()
{
    ReleaseUpdateMutex();
    disp_set_needupdate(DISP_MODULE_BLS , 1);
}

int CheckColorUpdateFunc(int i4NotUsed)
{
#if defined(DDP_GAMMA_SUPPORT)
    return (((1 << DISP_MODULE_COLOR) & u4UpdateFlag) || bls_gamma_dirty) ? 1 : 0;
#else
    return ((1 << DISP_MODULE_COLOR) & u4UpdateFlag) ? 1 : 0;
#endif
}

int ConfColorFunc(int i4NotUsed)
{
#if defined(DDP_GAMMA_SUPPORT)
    DDP_DRV_DBG("ConfColorFunc: BLS_EN=0x%x, bls_gamma_dirty=%d\n", DISP_REG_GET(DISP_REG_BLS_EN), bls_gamma_dirty);
    if(bls_gamma_dirty != 0)
    {
        // disable BLS
        if (DISP_REG_GET(DISP_REG_BLS_EN) & 0x1)
        {
            DDP_DRV_DBG("ConfColorFunc: Disable BLS\n");
            DISP_REG_SET(DISP_REG_BLS_EN, 0x00010000);
        }
    }
    else
    {
        if(ncs_tuning_mode == 0) //normal mode
        {
            DpEngine_COLORonInit();
            DpEngine_COLORonConfig(fb_width,fb_height);
        }
        else
        {
            ncs_tuning_mode = 0;
        }
        // enable BLS
        DISP_REG_SET(DISP_REG_BLS_EN, 0x00010001);
        disp_set_needupdate(DISP_MODULE_COLOR , 0);
    }
    DDP_DRV_DBG("ConfColorFunc done: BLS_EN=0x%x, bls_gamma_dirty=%d\n", DISP_REG_GET(DISP_REG_BLS_EN), bls_gamma_dirty);

    return 0;

#else
    if(ncs_tuning_mode == 0) //normal mode
    {
        DpEngine_COLORonInit();
        DpEngine_COLORonConfig(fb_width,fb_height);
    }
    else
    {
        ncs_tuning_mode = 0;
    }
    disp_set_needupdate(DISP_MODULE_COLOR , 0);

    return 0;
#endif
}

int disp_color_set_pq_param(void* arg)
{
    DISP_PQ_PARAM * pq_param;
    
    DISP_RegisterExTriggerSource(CheckColorUpdateFunc, ConfColorFunc);

    GetUpdateMutex();

    pq_param = get_Color_config();
    if(copy_from_user(pq_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
    {
        DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_PQPARAM Copy from user failed\n");
        ReleaseUpdateMutex();
        return -EFAULT;            
    }

    ReleaseUpdateMutex();

    disp_set_needupdate(DISP_MODULE_COLOR, 1);
    
    return 0;
}


static long disp_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    DISP_WRITE_REG wParams;
    DISP_READ_REG rParams;
    unsigned int ret = 0;
    unsigned int value;
    DISP_MODULE_ENUM module;
    DISP_OVL_INFO ovl_info;
    DISP_PQ_PARAM * pq_param;
    DISP_PQ_PARAM * pq_cam_param;
    DISP_PQ_PARAM * pq_gal_param;
    DISPLAY_PQ_T * pq_index;
    DISPLAY_TDSHP_T * tdshp_index;
    DISPLAY_GAMMA_T * gamma_index;
    //DISPLAY_PWM_T * pwm_lut;
    int layer, mutex_id;
    disp_wait_irq_struct wait_irq_struct;
    unsigned long lcmindex = 0;
#if defined(DDP_GAMMA_SUPPORT)
    int count = 0;
#endif

#if defined(MTK_AAL_SUPPORT)
    DISP_AAL_PARAM * aal_param;
#endif

#ifdef DDP_DBG_DDP_PATH_CONFIG
    struct disp_path_config_struct config;
#endif

    disp_node_struct *pNode = (disp_node_struct *)file->private_data;

#if 0
    if(inAddr==0)
    {
        inAddr = kmalloc(800*480*4, GFP_KERNEL);
        memset((void*)inAddr, 0x55, 800*480*4);
        DDP_DRV_INFO("inAddr=0x%x \n", inAddr);
    }
    if(outAddr==0)
    {
        outAddr = kmalloc(800*480*4, GFP_KERNEL);
        memset((void*)outAddr, 0xff, 800*480*4);
        DDP_DRV_INFO("outAddr=0x%x \n", outAddr);
    }
#endif

#if 0
    DDP_DRV_DBG("cmd=0x%x, arg=0x%x \n", cmd, (unsigned int)arg);
#endif

    switch(cmd)
    {   
        case DISP_IOCTL_WRITE_REG:
            
            if(copy_from_user(&wParams, (void *)arg, sizeof(DISP_WRITE_REG )))
            {
                DDP_DRV_ERR("DISP_IOCTL_WRITE_REG, copy_from_user failed\n");
                return -EFAULT;
            }

            DDP_DRV_DBG("write  0x%x = 0x%x (0x%x)\n", wParams.reg, wParams.val, wParams.mask);
            if(wParams.reg>DISPSYS_REG_ADDR_MAX || wParams.reg<DISPSYS_REG_ADDR_MIN)
            {
                DDP_DRV_ERR("reg write, addr invalid, addr min=0x%x, max=0x%x, addr=0x%x \n", 
                    DISPSYS_REG_ADDR_MIN, 
                    DISPSYS_REG_ADDR_MAX, 
                    wParams.reg);
                return -EFAULT;
            }
            
            *(volatile unsigned int*)wParams.reg = (*(volatile unsigned int*)wParams.reg & ~wParams.mask) | (wParams.val & wParams.mask);
            //mt65xx_reg_sync_writel(wParams.reg, value);
            break;
            
        case DISP_IOCTL_READ_REG:
            if(copy_from_user(&rParams, (void *)arg, sizeof(DISP_READ_REG)))
            {
                DDP_DRV_ERR("DISP_IOCTL_READ_REG, copy_from_user failed\n");
                return -EFAULT;
            }
#if defined(DDP_GAMMA_SUPPORT)
            if(rParams.reg>DISPSYS_REG_ADDR_MAX || rParams.reg<DISPSYS_REG_ADDR_MIN)
            {
                DDP_DRV_ERR("reg read, addr invalid, addr min=0x%x, max=0x%x, addr=0x%x \n", 
                    DISPSYS_REG_ADDR_MIN, 
                    DISPSYS_REG_ADDR_MAX, 
                    rParams.reg);
                return -EFAULT;
            }
#else
            if(0) //wParams.reg>DISPSYS_REG_ADDR_MAX || wParams.reg<DISPSYS_REG_ADDR_MIN)
            {
                DDP_DRV_ERR("reg read, addr invalid, addr min=0x%x, max=0x%x, addr=0x%x \n", 
                    DISPSYS_REG_ADDR_MIN, 
                    DISPSYS_REG_ADDR_MAX, 
                    wParams.reg);
                return -EFAULT;
            }
#endif

            value = (*(volatile unsigned int*)rParams.reg) & rParams.mask;

            DDP_DRV_DBG("read 0x%x = 0x%x (0x%x)\n", rParams.reg, value, rParams.mask);
            
            if(copy_to_user(rParams.val, &value, sizeof(unsigned int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_READ_REG, copy_to_user failed\n");
                return -EFAULT;            
            }
            break;

        case DISP_IOCTL_WAIT_IRQ:
            if(copy_from_user(&wait_irq_struct, (void*)arg , sizeof(wait_irq_struct)))
            {
                DDP_DRV_ERR("DISP_IOCTL_WAIT_IRQ, copy_from_user failed\n");
                return -EFAULT;
            }  
            ret = disp_wait_intr(wait_irq_struct.module, wait_irq_struct.timeout_ms);            
            break;  

        case DISP_IOCTL_DUMP_REG:
            if(copy_from_user(&module, (void*)arg , sizeof(module)))
            {
                DDP_DRV_ERR("DISP_IOCTL_DUMP_REG, copy_from_user failed\n");
                return -EFAULT;
            }  
            ret = disp_dump_reg(module);            
            break;  

        case DISP_IOCTL_LOCK_THREAD:
            DDP_DRV_DBG("DISP_IOCTL_LOCK_THREAD! \n");
            value = disp_lock_cmdq_thread();  
            if (copy_to_user((void*)arg, &value , sizeof(unsigned int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_LOCK_THREAD, copy_to_user failed\n");
                return -EFAULT;
            }
            break; 
            
        case DISP_IOCTL_UNLOCK_THREAD:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DDP_DRV_ERR("DISP_IOCTL_UNLOCK_THREAD, copy_from_user failed\n");
                    return -EFAULT;
            }  
            ret = disp_unlock_cmdq_thread(value);  
            break;

        case DISP_IOCTL_MARK_CMQ:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DDP_DRV_ERR("DISP_IOCTL_MARK_CMQ, copy_from_user failed\n");
                    return -EFAULT;
            }
            if(value >= CMDQ_THREAD_NUM) return -EFAULT;
//            cmq_status[value] = 1;
            break;
            
        case DISP_IOCTL_WAIT_CMQ:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DDP_DRV_ERR("DISP_IOCTL_WAIT_CMQ, copy_from_user failed\n");
                    return -EFAULT;
            }
            if(value >= CMDQ_THREAD_NUM) return -EFAULT;
            /*
            wait_event_interruptible_timeout(cmq_wait_queue[value], cmq_status[value], 3 * HZ);
            if(cmq_status[value] != 0)
            {
                cmq_status[value] = 0;
                return -EFAULT;
            }
        */
            break;

        case DISP_IOCTL_LOCK_RESOURCE:
            if(copy_from_user(&mutex_id, (void*)arg , sizeof(int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_LOCK_RESOURCE, copy_from_user failed\n");
                return -EFAULT;
            }
            if((-1) != mutex_id)
            {
                int ret = wait_event_interruptible_timeout(
                gResourceWaitQueue, 
                (gLockedResource & (1 << mutex_id)) == 0, 
                disp_ms2jiffies(50) ); 
                
                if(ret <= 0)
                {
                    DDP_DRV_ERR("DISP_IOCTL_LOCK_RESOURCE, mutex_id 0x%x failed\n",gLockedResource);
                    return -EFAULT;
                }
                
                spin_lock(&gResourceLock);
                gLockedResource |= (1 << mutex_id);
                spin_unlock(&gResourceLock);
                
                spin_lock(&pNode->node_lock);
                pNode->u4LockedResource = gLockedResource;
                spin_unlock(&pNode->node_lock);                 
            }
            else
            {
                DDP_DRV_ERR("DISP_IOCTL_LOCK_RESOURCE, mutex_id = -1 failed\n");
                return -EFAULT;
            }
            break;

            
        case DISP_IOCTL_UNLOCK_RESOURCE:
            if(copy_from_user(&mutex_id, (void*)arg , sizeof(int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_UNLOCK_RESOURCE, copy_from_user failed\n");
                return -EFAULT;
            }
            if((-1) != mutex_id)
            {
                spin_lock(&gResourceLock);
                gLockedResource &= ~(1 << mutex_id);
                spin_unlock(&gResourceLock);
                
                spin_lock(&pNode->node_lock);
                pNode->u4LockedResource = gLockedResource;
                spin_unlock(&pNode->node_lock);   

                wake_up_interruptible(&gResourceWaitQueue); 
            } 
            else
            {
                DDP_DRV_ERR("DISP_IOCTL_UNLOCK_RESOURCE, mutex_id = -1 failed\n");
                return -EFAULT;
            }            
            break;

        case DISP_IOCTL_SYNC_REG:
            mb();
            break;

        case DISP_IOCTL_SET_INTR:
            DDP_DRV_DBG("DISP_IOCTL_SET_INTR! \n");
            if(copy_from_user(&value, (void*)arg , sizeof(int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed\n");
                return -EFAULT;
            }  

            // enable intr
            if( (value&0xffff0000) !=0)
            {
                disable_irq(value&0xff);
                DDP_DRV_INFO("disable_irq %d \n", value&0xff);
            }
            else
            {
                DISP_REGISTER_IRQ(value&0xff);
                DDP_DRV_INFO("enable irq: %d \n", value&0xff);
            }            
            break; 

        case DISP_IOCTL_RUN_DPF:
            DDP_DRV_DBG("DISP_IOCTL_RUN_DPF! \n");
            if(copy_from_user(&value, (void*)arg , sizeof(int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed, %d\n", ret);
                return -EFAULT;
            }
            
            spin_lock(&gOvlLock);

            disp_run_dp_framework = value;
    
            spin_unlock(&gOvlLock);

            if(value == 1)
            {
                while(disp_get_mutex_status() != 0)
                {
                    DDP_DRV_ERR("disp driver : wait fb release hw mutex\n");
                    msleep(3);
                }
            }
            break;

        case DISP_IOCTL_CHECK_OVL:
            DDP_DRV_DBG("DISP_IOCTL_CHECK_OVL! \n");
            value = disp_layer_enable;
            
            if(copy_to_user((void *)arg, &value, sizeof(int)))
            {
                DDP_DRV_ERR("disp driver : Copy to user error (result)\n");
                return -EFAULT;            
            }
            break;

        case DISP_IOCTL_GET_OVL:
            DDP_DRV_DBG("DISP_IOCTL_GET_OVL! \n");
            if(copy_from_user(&ovl_info, (void*)arg , sizeof(DISP_OVL_INFO)))
            {
                DDP_DRV_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed, %d\n", ret);
                return -EFAULT;
            } 

            layer = ovl_info.layer;
            
            spin_lock(&gOvlLock);
            ovl_info = disp_layer_info[layer];
            spin_unlock(&gOvlLock);
            
            if(copy_to_user((void *)arg, &ovl_info, sizeof(DISP_OVL_INFO)))
            {
                DDP_DRV_ERR("disp driver : Copy to user error (result)\n");
                return -EFAULT;            
            }
            
            break;

        case DISP_IOCTL_AAL_EVENTCTL:
#if !defined(MTK_AAL_SUPPORT)
            DDP_DRV_ERR("Invalid operation DISP_IOCTL_AAL_EVENTCTL since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
            if(copy_from_user(&value, (void *)arg, sizeof(int)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_AAL_EVENTCTL Copy from user failed\n");
                return -EFAULT;            
            }
            disp_set_aal_alarm(value);
            disp_set_needupdate(DISP_MODULE_BLS , 1);
            ret = 0;
#endif
            break;

        case DISP_IOCTL_GET_AALSTATISTICS:
#if !defined(MTK_AAL_SUPPORT)
            DDP_DRV_ERR("Invalid operation DISP_IOCTL_GET_AALSTATISTICS since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
            // 1. Wait till new interrupt comes
            if(disp_wait_hist_update(60))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_GET_AALSTATISTICS wait time out\n");
                return -EFAULT;
            }

            // 2. read out color engine histogram
            disp_set_hist_readlock(1);
            if(copy_to_user((void*)arg, (void *)(disp_get_hist_ptr()) , sizeof(DISP_AAL_STATISTICS)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_GET_AALSTATISTICS Copy to user failed\n");
                return -EFAULT;
            }
            disp_set_hist_readlock(0);
            ret = 0;
#endif
            break;

        case DISP_IOCTL_SET_AALPARAM:
#if !defined(MTK_AAL_SUPPORT)
            DDP_DRV_ERR("Invalid operation : DISP_IOCTL_SET_AALPARAM since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
//            disp_set_needupdate(DISP_MODULE_BLS , 0);

            disp_aal_lock();

            aal_param = get_aal_config();

            if(copy_from_user(aal_param , (void *)arg, sizeof(DISP_AAL_PARAM)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_AALPARAM Copy from user failed\n");
                return -EFAULT;            
            }

            disp_aal_unlock();
#endif
            break;

        case DISP_IOCTL_SET_PQPARAM:

            ret = disp_color_set_pq_param((void*)arg);

            break;

        case DISP_IOCTL_SET_PQINDEX:

            pq_index = get_Color_index();
            if(copy_from_user(pq_index, (void *)arg, sizeof(DISPLAY_PQ_T)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_PQINDEX Copy from user failed\n");
                return -EFAULT;
            }

            break;    
            
        case DISP_IOCTL_GET_PQPARAM:
            // this is duplicated to cmdq_proc_unlocked_ioctl
            // be careful when modify the definition
            pq_param = get_Color_config();
            if(copy_to_user((void *)arg, pq_param, sizeof(DISP_PQ_PARAM)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_GET_PQPARAM Copy to user failed\n");
                return -EFAULT;            
            }

            break;    
            
        case DISP_IOCTL_SET_TDSHPINDEX:
        
            tdshp_index = get_TDSHP_index();
            if(copy_from_user(tdshp_index, (void *)arg, sizeof(DISPLAY_TDSHP_T)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_TDSHPINDEX Copy from user failed\n");
                return -EFAULT;
            }
        
            break;           
        
        case DISP_IOCTL_GET_TDSHPINDEX:
            // this is duplicated to cmdq_proc_unlocked_ioctl
            // be careful when modify the definition
            tdshp_index = get_TDSHP_index();
            if(copy_to_user((void *)arg, tdshp_index, sizeof(DISPLAY_TDSHP_T)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_GET_TDSHPINDEX Copy to user failed\n");
                return -EFAULT;            
            }
    
            break;       
        
         case DISP_IOCTL_SET_GAMMALUT:
        
#if defined(DDP_GAMMA_SUPPORT)
            DDP_DRV_DBG("DISP_IOCTL_SET_GAMMALUT\n");
            
            gamma_index = get_gamma_index();
            if(copy_from_user(gamma_index, (void *)arg, sizeof(DISPLAY_GAMMA_T)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_GAMMALUT Copy from user failed\n");
                return -EFAULT;
            }

            // disable BLS and suspend AAL
            GetUpdateMutex();
            bls_gamma_dirty = 1;
            aal_debug_flag = 1;
            ReleaseUpdateMutex();

            disp_set_needupdate(DISP_MODULE_COLOR, 1);

            while(DISP_REG_GET(DISP_REG_BLS_EN) & 0x1) {
                msleep(1);
                count++;
                if (count > 1000) {
                    DDP_DRV_ERR("fail to disable BLS (0x%x)\n", DISP_REG_GET(DISP_REG_BLS_EN));
                }
            }

            // update gamma lut
            // enable BLS and resume AAL
            GetUpdateMutex();
            disp_bls_update_gamma_lut();
            bls_gamma_dirty = 0;
            aal_debug_flag = 0;
            ReleaseUpdateMutex(); 

            disp_set_needupdate(DISP_MODULE_COLOR, 1);

            break;

#else
            gamma_index = get_gamma_index();
            if(copy_from_user(gamma_index, (void *)arg, sizeof(DISPLAY_GAMMA_T)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_GAMMALUT Copy from user failed\n");
                return -EFAULT;
            }
        
            disp_bls_update_gamma_lut();
            
            break;
#endif

         case DISP_IOCTL_SET_CLKON:
            if(copy_from_user(&module, (void *)arg, sizeof(DISP_MODULE_ENUM)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_CLKON Copy from user failed\n");
                return -EFAULT;            
            }

            disp_power_on(module , &(pNode->u4Clock));
            break;

        case DISP_IOCTL_SET_CLKOFF:
            if(copy_from_user(&module, (void *)arg, sizeof(DISP_MODULE_ENUM)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_CLKOFF Copy from user failed\n");
                return -EFAULT;            
            }

            disp_power_off(module , &(pNode->u4Clock));
            break;

        case DISP_IOCTL_MUTEX_CONTROL:

#if defined(DDP_GAMMA_SUPPORT)
            if(copy_from_user(&value, (void *)arg, sizeof(int)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_MUTEX_CONTROL Copy from user failed\n");
                return -EFAULT;            
            }

            DDP_DRV_DBG("DISP_IOCTL_MUTEX_CONTROL: %d, BLS_EN = %d\n", value, DISP_REG_GET(DISP_REG_BLS_EN));

            if(value == 1)
            {
            
                // disable BLS and suspend AAL
                GetUpdateMutex();
                bls_gamma_dirty = 1;
                aal_debug_flag = 1;
                ReleaseUpdateMutex();

                disp_set_needupdate(DISP_MODULE_COLOR, 1);

                count = 0;
                while(DISP_REG_GET(DISP_REG_BLS_EN) & 0x1) {
                    msleep(1);
                    count++;
                    if (count > 1000) {
                        DDP_DRV_ERR("fail to disable BLS (0x%x)\n", DISP_REG_GET(DISP_REG_BLS_EN));
                    }
                }
                
                ncs_tuning_mode = 1;
                GetUpdateMutex();
            }
            else if(value == 2)
            {
                // enable BLS and resume AAL
                bls_gamma_dirty = 0;
                aal_debug_flag = 0;
                ReleaseUpdateMutex();
                
                disp_set_needupdate(DISP_MODULE_COLOR, 1);
            }
            else
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_MUTEX_CONTROL invalid control\n");
                return -EFAULT;            
            }

            DDP_DRV_DBG("DISP_IOCTL_MUTEX_CONTROL done: %d, BLS_EN = %d\n", value, DISP_REG_GET(DISP_REG_BLS_EN));
            
            break;    

#else
            if(copy_from_user(&value, (void *)arg, sizeof(int)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_MUTEX_CONTROL Copy from user failed\n");
                return -EFAULT;            
            }
            
            if(value == 1)
            {
                ncs_tuning_mode = 1;
                GetUpdateMutex();
            }
            else if(value == 2)
            {   
                ReleaseUpdateMutex();
                
                disp_set_needupdate(DISP_MODULE_COLOR, 1);
                
                //ReleaseUpdateMutex();
            }
            else
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_MUTEX_CONTROL invalid control\n");
                return -EFAULT;            
            }
            
            break;    
#endif
            
        case DISP_IOCTL_GET_LCMINDEX:
            
                lcmindex = DISP_GetLCMIndex();
                if(copy_to_user((void *)arg, &lcmindex, sizeof(unsigned long)))
                {
                    DDP_DRV_ERR("disp driver : DISP_IOCTL_GET_LCMINDEX Copy to user failed\n");
                    return -EFAULT;            
                }

                break;       

            break;        
            
        case DISP_IOCTL_SET_PQ_CAM_PARAM:

            pq_cam_param = get_Color_Cam_config();
            if(copy_from_user(pq_cam_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_PQ_CAM_PARAM Copy from user failed\n");
                return -EFAULT;            
            }

            break;
            
        case DISP_IOCTL_GET_PQ_CAM_PARAM:
            
            pq_cam_param = get_Color_Cam_config();
            if(copy_to_user((void *)arg, pq_cam_param, sizeof(DISP_PQ_PARAM)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_GET_PQ_CAM_PARAM Copy to user failed\n");
                return -EFAULT;            
            }
            
            break;        
            
        case DISP_IOCTL_SET_PQ_GAL_PARAM:

            pq_gal_param = get_Color_Gal_config();
            if(copy_from_user(pq_gal_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_SET_PQ_GAL_PARAM Copy from user failed\n");
                return -EFAULT;            
            }
            
            break;

        case DISP_IOCTL_GET_PQ_GAL_PARAM:
            
            pq_gal_param = get_Color_Gal_config();
            if(copy_to_user((void *)arg, pq_gal_param, sizeof(DISP_PQ_PARAM)))
            {
                DDP_DRV_ERR("disp driver : DISP_IOCTL_GET_PQ_GAL_PARAM Copy to user failed\n");
                return -EFAULT;            
            }
            
            break;          

        case DISP_IOCTL_TEST_PATH:
#ifdef DDP_DBG_DDP_PATH_CONFIG
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DDP_DRV_ERR("DISP_IOCTL_MARK_CMQ, copy_from_user failed\n");
                    return -EFAULT;
            }

            config.layer = 0;
            config.layer_en = 1;
            config.source = OVL_LAYER_SOURCE_MEM; 
            config.addr = virt_to_phys(inAddr); 
            config.inFormat = OVL_INPUT_FORMAT_RGB565; 
            config.pitch = 480;
            config.srcROI.x = 0;        // ROI
            config.srcROI.y = 0;  
            config.srcROI.width = 480;  
            config.srcROI.height = 800;  
            config.bgROI.x = config.srcROI.x;
            config.bgROI.y = config.srcROI.y;
            config.bgROI.width = config.srcROI.width;
            config.bgROI.height = config.srcROI.height;
            config.bgColor = 0xff;  // background color
            config.key = 0xff;     // color key
            config.aen = 0;             // alpha enable
            config.alpha = 0;  
            DDP_DRV_INFO("value=%d \n", value);
            if(value==0) // mem->ovl->rdma0->dpi0
            {
                config.srcModule = DISP_MODULE_OVL;
                config.outFormat = RDMA_OUTPUT_FORMAT_ARGB; 
                config.dstModule = DISP_MODULE_DPI0;
                config.dstAddr = 0;
            }
            else if(value==1) // mem->ovl-> wdma1->mem
            {
                config.srcModule = DISP_MODULE_OVL;
                config.outFormat = WDMA_OUTPUT_FORMAT_RGB888; 
                config.dstModule = DISP_MODULE_WDMA0;
                config.dstAddr = virt_to_phys(outAddr);
            }
            else if(value==2)  // mem->rdma0 -> dpi0
            {
                config.srcModule = DISP_MODULE_RDMA0;
                config.outFormat = RDMA_OUTPUT_FORMAT_ARGB; 
                config.dstModule = DISP_MODULE_DPI0;
                config.dstAddr = 0;
            }
            disp_path_config(&config);
            disp_path_enable();
#endif			
            break;            
#if 0
        case DISP_IOCTL_G_WAIT_REQUEST:
            ret = ddp_bitblt_ioctl_wait_reequest(arg);
            break;

        case DISP_IOCTL_T_INFORM_DONE:
            ret = ddp_bitblt_ioctl_inform_done(arg);
            break;
#endif
            
        default :
            DDP_DRV_ERR("Ddp drv dose not have such command : %d\n" , cmd);
            break; 
    }
    
    return ret;
}

static int disp_open(struct inode *inode, struct file *file)
{
    disp_node_struct *pNode = NULL;

    DDP_DRV_DBG("enter disp_open() process:%s\n",current->comm);

    //Allocate and initialize private data
    file->private_data = kmalloc(sizeof(disp_node_struct) , GFP_ATOMIC);
    if(NULL == file->private_data)
    {
        DDP_DRV_INFO("Not enough entry for DDP open operation\n");
        return -ENOMEM;
    }
    
    pNode = (disp_node_struct *)file->private_data;
    pNode->open_pid = current->pid;
    pNode->open_tgid = current->tgid;
    INIT_LIST_HEAD(&(pNode->testList));
    pNode->u4LockedResource = 0;
    pNode->u4Clock = 0;
    spin_lock_init(&pNode->node_lock);

    return 0;

}

static ssize_t disp_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
    return 0;
}

static int disp_release(struct inode *inode, struct file *file)
{
    disp_node_struct *pNode = NULL;
    unsigned int index = 0;
    DDP_DRV_DBG("enter disp_release() process:%s\n",current->comm);
    
    pNode = (disp_node_struct *)file->private_data;

    spin_lock(&pNode->node_lock);

    if(pNode->u4LockedResource)
    {
        DDP_DRV_ERR("Proccess terminated[REsource] ! :%s , resource:%d\n" 
            , current->comm , pNode->u4LockedResource);
        spin_lock(&gResourceLock);
        gLockedResource = 0;
        spin_unlock(&gResourceLock);
    }

    if(pNode->u4Clock)
    {
        DDP_DRV_ERR("Process safely terminated [Clock] !:%s , clock:%u\n" 
            , current->comm , pNode->u4Clock);

        for(index  = 0 ; index < DISP_MODULE_MAX; index += 1)
        {
            if((1 << index) & pNode->u4Clock)
            {
                disp_power_off((DISP_MODULE_ENUM)index , &pNode->u4Clock);
            }
        }
    }

    cmdqTerminated();

    spin_unlock(&pNode->node_lock);

    if(NULL != file->private_data)
    {
        kfree(file->private_data);
        file->private_data = NULL;
    }
    
    return 0;
}

static int disp_flush(struct file * file , fl_owner_t a_id)
{
    return 0;
}

// remap register to user space
static int disp_mmap(struct file * file, struct vm_area_struct * a_pstVMArea)
{
    unsigned long size = a_pstVMArea->vm_end - a_pstVMArea->vm_start;
    unsigned long paStart = a_pstVMArea->vm_pgoff << PAGE_SHIFT;
    unsigned long paEnd = paStart + size;
    unsigned long MAX_SIZE = DISPSYS_REG_ADDR_MAX - DISPSYS_REG_ADDR_MIN;

    if (size > MAX_SIZE)
    {
        DISP_MSG("MMAP Size Range OVERFLOW!!\n");
        return -1;
    }
    if (paStart < (DISPSYS_REG_ADDR_MIN-0xE0000000) || paEnd > (DISPSYS_REG_ADDR_MAX-0xE0000000)) 
    {
        DISP_MSG("MMAP Address Range OVERFLOW!!\n");
        return -1;
    }

    a_pstVMArea->vm_page_prot = pgprot_noncached(a_pstVMArea->vm_page_prot);
    if(remap_pfn_range(a_pstVMArea , 
                 a_pstVMArea->vm_start , 
                 a_pstVMArea->vm_pgoff , 
                 (a_pstVMArea->vm_end - a_pstVMArea->vm_start) , 
                 a_pstVMArea->vm_page_prot))
    {
        DDP_DRV_INFO("MMAP failed!!\n");
        return -1;
    }

    return 0;
}


/* Kernel interface */
static struct file_operations disp_fops = {
    .owner		= THIS_MODULE,
    .unlocked_ioctl = disp_unlocked_ioctl,
    .open		= disp_open,
    .release	= disp_release,
    .flush		= disp_flush,
    .read       = disp_read,
    .mmap       = disp_mmap
};

static int disp_probe(struct platform_device *pdev)
{
    struct class_device;
    
    int ret;
    int i;
    struct class_device *class_dev = NULL;
    
    DDP_DRV_INFO("\ndisp driver probe...\n\n");
    ret = alloc_chrdev_region(&disp_devno, 0, 1, DISP_DEVNAME);
    
    if(ret)
    {
        DDP_DRV_ERR("Error: Can't Get Major number for DISP Device\n");
    }
    else
    {
        DDP_DRV_INFO("Get DISP Device Major number (%d)\n", disp_devno);
    }

    disp_cdev = cdev_alloc();
    disp_cdev->owner = THIS_MODULE;
    disp_cdev->ops = &disp_fops;
    
    ret = cdev_add(disp_cdev, disp_devno, 1);

    disp_class = class_create(THIS_MODULE, DISP_DEVNAME);
    class_dev = (struct class_device *)device_create(disp_class, NULL, disp_devno, NULL, DISP_DEVNAME);

    // initial wait queue
    for(i = 0 ; i < CMDQ_THREAD_NUM ; i++)
    {
        init_waitqueue_head(&cmq_wait_queue[i]);
        //cmq_status[i] = 0;
        // enable CMDQ interrupt
    }
    init_waitqueue_head(&cmq_exec_wait_queue);
        
    // Register IRQ
    DISP_REGISTER_IRQ(MT_DISP_COLOR_IRQ_ID);
    DISP_REGISTER_IRQ(MT_DISP_OVL_IRQ_ID  );
    DISP_REGISTER_IRQ(MT_DISP_WDMA_IRQ_ID);
    DISP_REGISTER_IRQ(MT_DISP_RDMA_IRQ_ID);
    DISP_REGISTER_IRQ(MT_CMDQ_IRQ_ID);
    DISP_REGISTER_IRQ(MT_MM_MUTEX_IRQ_ID);
    //DISP_REGISTER_IRQ(MT6572_G2D_IRQ_ID);
    
    spin_lock_init(&gCmdqLock);
    spin_lock_init(&gResourceLock);
    spin_lock_init(&gOvlLock);
    spin_lock_init(&gRegisterUpdateLock);
    spin_lock_init(&gPowerOperateLock);
    spin_lock_init(&g_disp_irq.irq_lock);
    

    init_waitqueue_head(&disp_irq_log_wq);
    disp_irq_log_task = kthread_create(disp_irq_log_kthread_func, NULL, "disp_config_update_kthread");
    if (IS_ERR(disp_irq_log_task)) 
    {
        DDP_DRV_ERR("DISP_InitVSYNC(): Cannot create disp_irq_log_task kthread\n");
    }
    wake_up_process(disp_irq_log_task);
   
    spin_lock_init(&gCmdqMgrLock);
    
    DDP_DRV_INFO("DISP Probe Done\n");
    
    NOT_REFERENCED(class_dev);
    return 0;
}

static int disp_remove(struct platform_device *pdev)
{
    disable_irq(MT_DISP_OVL_IRQ_ID);
    disable_irq(MT_DISP_WDMA_IRQ_ID);
    disable_irq(MT_DISP_RDMA_IRQ_ID);
    disable_irq(MT_CMDQ_IRQ_ID);
    //disable_irq(MT_DISP_COLOR_IRQ_ID);
    disable_irq(MT_DISP_BLS_IRQ_ID);
    //disable_irq(MT6572_G2D_IRQ_ID);
    return 0;
}

static void disp_shutdown(struct platform_device *pdev)
{
	/* Nothing yet */
}

/* PM suspend */
static int disp_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    DDP_DRV_INFO("\n\n\n!!!!!!!disp_suspend: cmdqForceFreeAll !!!!!!!!!!!\n\n\n");

    //Only Thread 0 available in 6572
    cmdqForceFreeAll(0);

    return 0;
}

/* PM resume */
static int disp_resume(struct platform_device *pdev)
{
    // clear cmdq event for MDP engine
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x14);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x15);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x16);

    return 0;
}


static struct platform_driver disp_driver = {
    .probe		= disp_probe,
    .remove		= disp_remove,
    .shutdown	= disp_shutdown,
    .suspend	= disp_suspend,
    .resume		= disp_resume,
    .driver     = {
    .name = DISP_DEVNAME,
    },
};

static void disp_device_release(struct device *dev)
{
    // Nothing to release? 
}

static u64 disp_dmamask = ~(u32)0;

static struct platform_device disp_device = {
    .name	 = DISP_DEVNAME,
    .id      = 0,
    .dev     = {
    .release = disp_device_release,
    .dma_mask = &disp_dmamask,
    .coherent_dma_mask = 0xffffffff,
    },
    .num_resources = 0,
};

static int __init disp_init(void)
{
    int ret;

    DDP_DRV_INFO("Register disp device\n");
    if(platform_device_register(&disp_device))
    {
        DDP_DRV_ERR("failed to register disp device\n");
        ret = -ENODEV;
        return ret;
    }

    DDP_DRV_INFO("Register the disp driver\n");    
    if(platform_driver_register(&disp_driver))
    {
        DDP_DRV_ERR("failed to register disp driver\n");
        platform_device_unregister(&disp_device);
        ret = -ENODEV;
        return ret;
    }

    ddp_debug_init();

    pRegBackup = kmalloc(DDP_BACKUP_REG_NUM*sizeof(int), GFP_KERNEL);
    ASSERT(pRegBackup!=NULL);
    *pRegBackup = DDP_UNBACKED_REG_MEM;

    cmdq_dma_init();
    // clear cmdq event for MDP engine
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x14);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x15);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x16);

    return 0;
}

static void __exit disp_exit(void)
{
    cdev_del(disp_cdev);
    unregister_chrdev_region(disp_devno, 1);

    platform_driver_unregister(&disp_driver);
    platform_device_unregister(&disp_device);

    device_destroy(disp_class, disp_devno);
    class_destroy(disp_class);

    ddp_debug_exit();

    DDP_DRV_INFO("Done\n");
}

#if 0
static int disp_set_overlay_roi(int layer, int x, int y, int w, int h, int pitch)
{
    // DDP_DRV_INFO(" disp_set_overlay_roi %d\n", layer );
    
    if(layer < 0 || layer >= DDP_OVL_LAYER_MUN) return -1;
    spin_lock(&gOvlLock);

    disp_layer_info[layer].x = x;
    disp_layer_info[layer].y = y;
    disp_layer_info[layer].w = w;
    disp_layer_info[layer].h = h;
    disp_layer_info[layer].pitch = pitch;
    
    spin_unlock(&gOvlLock);

    return 0;
}

static int disp_set_overlay_addr(int layer, unsigned int addr, DpColorFormat fmt)
{
    // DDP_DRV_INFO(" disp_set_overlay_addr %d\n", layer );
    if(layer < 0 || layer >= DDP_OVL_LAYER_MUN) return -1;
    
    spin_lock(&gOvlLock);

    disp_layer_info[layer].addr = addr;
    disp_layer_info[layer].fmt = fmt;
    
    spin_unlock(&gOvlLock);

    return 0;
}

static int disp_set_overlay(int layer, int enable)
{
    // DDP_DRV_INFO(" disp_set_overlay %d %d\n", layer, enable );
    if(layer < 0 || layer >= DDP_OVL_LAYER_MUN) return -1;
    
    spin_lock(&gOvlLock);

    if(enable == 0)
        disp_layer_enable = disp_layer_enable & ~(1 << layer);
    else
        disp_layer_enable = disp_layer_enable | (1 << layer);

    spin_unlock(&gOvlLock);

    return 0;
}

static int disp_is_dp_framework_run()
{
    // DDP_DRV_INFO(" disp_is_dp_framework_run " );
    return disp_run_dp_framework;
}

static int disp_set_mutex_status(int enable)
{
    // DDP_DRV_INFO(" disp_set_mutex_status %d\n", enable );
    spin_lock(&gOvlLock);

    disp_mutex_status = enable;
    
    spin_unlock(&gOvlLock);
    return 0;
}
#endif

static int disp_get_mutex_status()
{
    return disp_mutex_status;
}

int disp_dump_reg(DISP_MODULE_ENUM module)
{
        unsigned int index;

        switch(module)
        {       
            case DISP_MODULE_CONFIG:
                DDP_DRV_INFO("===== DISP DISP_REG_MM_CONFIG Reg Dump: ============\n");
                DDP_DRV_INFO("(0x020)MDP_RDMA_MOUT_EN        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RDMA_MOUT_EN         ));    
                DDP_DRV_INFO("(0x024)MDP_RSZ0_MOUT_EN        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RSZ0_MOUT_EN         ));    
                DDP_DRV_INFO("(0x028)MDP_RSZ1_MOUT_EN        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RSZ1_MOUT_EN         ));    
                DDP_DRV_INFO("(0x02c)MDP_TDSHP_MOUT_EN       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_TDSHP_MOUT_EN        ));    
                DDP_DRV_INFO("(0x030)DISP_OVL_MOUT_EN        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN         ));    
                DDP_DRV_INFO("(0x034)MMSYS_MOUT_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MOUT_RST           ));    
                DDP_DRV_INFO("(0x038)MDP_RSZ0_SEL            =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RSZ0_SEL             ));    
                DDP_DRV_INFO("(0x03c)MDP_RSZ1_SEL            =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RSZ1_SEL             ));    
                DDP_DRV_INFO("(0x040)MDP_TDSHP_SEL           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_TDSHP_SEL            ));    
                DDP_DRV_INFO("(0x044)MDP_WROT_SEL            =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_WROT_SEL             ));    
                DDP_DRV_INFO("(0x048)MDP_WDMA_SEL            =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_WDMA0_SEL             ));    
                DDP_DRV_INFO("(0x04c)DISP_OUT_SEL            =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DISP_OUT_SEL             ));    
                DDP_DRV_INFO("(0x050)MMSYS_RDMA0_OUT_SEL                  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RDMA0_OUT_SEL                 ));
                DDP_DRV_INFO("(0x054)MMSYS_COLOR_SEL                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_COLOR_SEL                 ));
                DDP_DRV_INFO("(0x058)MMSYS_DSI_SEL                     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DSI_SEL                     ));
                DDP_DRV_INFO("(0x05C)MMSYS_DPI0_SEL                   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DPI0_SEL                  ));
                DDP_DRV_INFO("(0x100)MMSYS_CG_CON0           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0            ));    
                DDP_DRV_INFO("(0x104)MMSYS_CG_SET0           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_SET0            ));    
                DDP_DRV_INFO("(0x108)MMSYS_CG_CLR0           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CLR0            ));    
                DDP_DRV_INFO("(0x110)MMSYS_CG_CON1           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON1            ));    
                DDP_DRV_INFO("(0x114)MMSYS_CG_SET1           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_SET1            ));    
                DDP_DRV_INFO("(0x118)MMSYS_CG_CLR1           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CLR1            ));    
                DDP_DRV_INFO("(0x120)MMSYS_HW_DCM_DIS0       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN0        ));    
                DDP_DRV_INFO("(0x124)MMSYS_HW_DCM_DIS_SET0   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_SET0    ));    
                DDP_DRV_INFO("(0x128)MMSYS_HW_DCM_DIS_CLR0   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_CLR0    ));    
                DDP_DRV_INFO("(0x130)MMSYS_HW_DCM_DIS_SET1   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN1    ));    
                DDP_DRV_INFO("(0x134)MMSYS_HW_DCM_DIS_CLR1   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_SET1    ));    
                DDP_DRV_INFO("(0x138)MMSYS_SW_RST_B          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_CLR1           ));    
                DDP_DRV_INFO("(0x800)MMSYS_MBIST_DONE        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_DONE0         ));    
                DDP_DRV_INFO("(0x804)MMSYS_MBIST_FAIL0       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_FAIL0        ));    
                DDP_DRV_INFO("(0x808)MMSYS_MBIST_FAIL1       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_FAIL1        ));    
                DDP_DRV_INFO("(0x80C)MMSYS_MBIST_HOLDB       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_HOLDB0        ));    
                DDP_DRV_INFO("(0x810)MMSYS_MBIST_MODE        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_MODE0         ));    
                DDP_DRV_INFO("(0x814)MMSYS_MBIST_BSEL0       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL0        ));    
                DDP_DRV_INFO("(0x818)MMSYS_MBIST_BSEL1       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL1        ));    
                DDP_DRV_INFO("(0x81c)MMSYS_MBIST_CON         =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL2          ));  
                DDP_DRV_INFO("(0x820)MMSYS_MEM_DELSEL0       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_CON        ));    
                DDP_DRV_INFO("(0x824)MMSYS_MEM_DELSEL1       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL0        ));    
                DDP_DRV_INFO("(0x828)MMSYS_MEM_DELSEL2       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL1        ));    
                DDP_DRV_INFO("(0x82c)MMSYS_MEM_DELSEL3       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL2        ));    
                DDP_DRV_INFO("(0x830)MMSYS_DEBUG_OUT_SEL     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL3      ));    
                DDP_DRV_INFO("(0x834)MMSYS_MEM_DELSEL4              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL4             ));
                DDP_DRV_INFO("(0x838)MMSYS_MEM_DELSEL5                =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL5                 ));
                DDP_DRV_INFO("(0x83C)MMSYS_MEM_DELSEL6                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL6                 ));
                DDP_DRV_INFO("(0x840)MMSYS_DEBUG_OUT_SEL             =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DEBUG_OUT_SEL              ));    
                DDP_DRV_INFO("(0x844)MMSYS_CONFIG_DUMMY                  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DUMMY                 ));
                DDP_DRV_INFO("(0x850)MMSYS_MROT_MBISR_RESET                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_WROT_MBISR_RESET                 ));
                DDP_DRV_INFO("(0x854)MMSYS_MROT_MBISR_FAIL                     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_WROT_MBISR_FAIL                     ));
                DDP_DRV_INFO("(0x858)MMSYS_MROT_MBISR_OK                   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_WROT_MBISR_OK                  ));
                DDP_DRV_INFO("(0x860)MMSYS_DL_VALID0                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DL_VALID0                     ));
                DDP_DRV_INFO("(0x864)MMSYS_DL_VALID1               =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DL_VALID1              ));
                DDP_DRV_INFO("(0x868)MMSYS_DL_READY0           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DL_READY0          ));
                DDP_DRV_INFO("(0x86C)MMSYS_DL_READY1              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DL_READY1             ));
                break;
                
            case DISP_MODULE_OVL: 
                DDP_DRV_INFO("===== DISP OVL Reg Dump: ============\n");          
                DDP_DRV_INFO("(000)OVL_STA                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_STA                   ));
                DDP_DRV_INFO("(004)OVL_INTEN                   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_INTEN                 ));
                DDP_DRV_INFO("(008)OVL_INTSTA              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_INTSTA                ));
                DDP_DRV_INFO("(00C)OVL_EN                  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_EN                    ));
                DDP_DRV_INFO("(010)OVL_TRIG                    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_TRIG                  ));
                DDP_DRV_INFO("(014)OVL_RST                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RST                   ));
                DDP_DRV_INFO("(020)OVL_ROI_SIZE                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ROI_SIZE              ));
                DDP_DRV_INFO("(024)OVL_DATAPATH_CON            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_DATAPATH_CON          ));
                DDP_DRV_INFO("(028)OVL_ROI_BGCLR               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ROI_BGCLR             ));
                DDP_DRV_INFO("(02C)OVL_SRC_CON             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_SRC_CON               ));
                DDP_DRV_INFO("(030)OVL_L0_CON              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_CON                ));
                DDP_DRV_INFO("(034)OVL_L0_SRCKEY               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_SRCKEY             ));
                DDP_DRV_INFO("(038)OVL_L0_SRC_SIZE         =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_SRC_SIZE           ));
                DDP_DRV_INFO("(03C)OVL_L0_OFFSET               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_OFFSET             ));
                DDP_DRV_INFO("(040)OVL_L0_ADDR             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_ADDR               ));
                DDP_DRV_INFO("(044)OVL_L0_PITCH                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_PITCH              ));
                DDP_DRV_INFO("(0C0)OVL_RDMA0_CTRL          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_CTRL            ));
                DDP_DRV_INFO("(0C4)OVL_RDMA0_MEM_START_TRIG    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_START_TRIG  ));
                DDP_DRV_INFO("(0C8)OVL_RDMA0_MEM_GMC_SETTING   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING ));
                DDP_DRV_INFO("(0CC)OVL_RDMA0_MEM_SLOW_CON  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_SLOW_CON    ));
                DDP_DRV_INFO("(0D0)OVL_RDMA0_FIFO_CTRL     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_FIFO_CTRL       ));
                DDP_DRV_INFO("(050)OVL_L1_CON              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_CON                ));
                DDP_DRV_INFO("(054)OVL_L1_SRCKEY               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_SRCKEY             ));
                DDP_DRV_INFO("(058)OVL_L1_SRC_SIZE         =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_SRC_SIZE           ));
                DDP_DRV_INFO("(05C)OVL_L1_OFFSET               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_OFFSET             ));
                DDP_DRV_INFO("(060)OVL_L1_ADDR             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_ADDR               ));
                DDP_DRV_INFO("(064)OVL_L1_PITCH                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_PITCH              ));
                DDP_DRV_INFO("(0E0)OVL_RDMA1_CTRL          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_CTRL            ));
                DDP_DRV_INFO("(0E4)OVL_RDMA1_MEM_START_TRIG    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_START_TRIG  ));
                DDP_DRV_INFO("(0E8)OVL_RDMA1_MEM_GMC_SETTING   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING ));
                DDP_DRV_INFO("(0EC)OVL_RDMA1_MEM_SLOW_CON  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_SLOW_CON    ));
                DDP_DRV_INFO("(0F0)OVL_RDMA1_FIFO_CTRL     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_FIFO_CTRL       ));
                DDP_DRV_INFO("(070)OVL_L2_CON              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_CON                ));
                DDP_DRV_INFO("(074)OVL_L2_SRCKEY               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_SRCKEY             ));
                DDP_DRV_INFO("(078)OVL_L2_SRC_SIZE         =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_SRC_SIZE           ));
                DDP_DRV_INFO("(07C)OVL_L2_OFFSET               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_OFFSET             ));
                DDP_DRV_INFO("(080)OVL_L2_ADDR             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_ADDR               ));
                DDP_DRV_INFO("(084)OVL_L2_PITCH                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_PITCH              ));
                DDP_DRV_INFO("(100)OVL_RDMA2_CTRL          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_CTRL            ));
                DDP_DRV_INFO("(104)OVL_RDMA2_MEM_START_TRIG    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_START_TRIG  ));
                DDP_DRV_INFO("(108)OVL_RDMA2_MEM_GMC_SETTING   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING ));
                DDP_DRV_INFO("(10C)OVL_RDMA2_MEM_SLOW_CON  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_SLOW_CON    ));
                DDP_DRV_INFO("(110)OVL_RDMA2_FIFO_CTRL     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_FIFO_CTRL       ));
                DDP_DRV_INFO("(090)OVL_L3_CON              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_CON                ));
                DDP_DRV_INFO("(094)OVL_L3_SRCKEY               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_SRCKEY             ));
                DDP_DRV_INFO("(098)OVL_L3_SRC_SIZE         =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_SRC_SIZE           ));
                DDP_DRV_INFO("(09C)OVL_L3_OFFSET               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_OFFSET             ));
                DDP_DRV_INFO("(0A0)OVL_L3_ADDR             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_ADDR               ));
                DDP_DRV_INFO("(0A4)OVL_L3_PITCH                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_PITCH              ));
                DDP_DRV_INFO("(120)OVL_RDMA3_CTRL          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_CTRL            ));
                DDP_DRV_INFO("(124)OVL_RDMA3_MEM_START_TRIG    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_START_TRIG  ));
                DDP_DRV_INFO("(128)OVL_RDMA3_MEM_GMC_SETTING   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING ));
                DDP_DRV_INFO("(12C)OVL_RDMA3_MEM_SLOW_CON  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_SLOW_CON    ));
                DDP_DRV_INFO("(130)OVL_RDMA3_FIFO_CTRL     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_FIFO_CTRL       ));
                DDP_DRV_INFO("(1C4)OVL_DEBUG_MON_SEL           =0x%x \n", DISP_REG_GET(DISP_REG_OVL_DEBUG_MON_SEL         ));
                DDP_DRV_INFO("(1C4)OVL_RDMA0_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2));
                DDP_DRV_INFO("(1C8)OVL_RDMA1_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2));
                DDP_DRV_INFO("(1CC)OVL_RDMA2_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2));
                DDP_DRV_INFO("(1D0)OVL_RDMA3_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2));
                DDP_DRV_INFO("(240)OVL_FLOW_CTRL_DBG           =0x%x \n", DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG         ));
                DDP_DRV_INFO("(244)OVL_ADDCON_DBG          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG            ));
                DDP_DRV_INFO("(248)OVL_OUTMUX_DBG          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_OUTMUX_DBG            ));
                DDP_DRV_INFO("(24C)OVL_RDMA0_DBG               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_DBG             ));
                DDP_DRV_INFO("(250)OVL_RDMA1_DBG               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_DBG             ));
                DDP_DRV_INFO("(254)OVL_RDMA2_DBG               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_DBG             ));
                DDP_DRV_INFO("(258)OVL_RDMA3_DBG               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_DBG             ));
                break;
                 
            case DISP_MODULE_COLOR:  
                DDP_DRV_INFO("===== DISP COLOR Reg Dump: ============\n");  
                DDP_DRV_INFO("(0x0400)DISP_REG_COLOR_CFG_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CFG_MAIN                     ));
                DDP_DRV_INFO("(0x0404)DISP_REG_COLOR_PXL_CNT_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PXL_CNT_MAIN                 ));
                DDP_DRV_INFO("(0x0408)DISP_REG_COLOR_LINE_CNT_MAIN                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LINE_CNT_MAIN                 ));
                DDP_DRV_INFO("(0x040C)DISP_REG_COLOR_WIN_X_MAIN                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_WIN_X_MAIN                     ));
                DDP_DRV_INFO("(0x0410)DISP_REG_COLOR_WIN_Y_MAIN                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_WIN_Y_MAIN                  ));
                DDP_DRV_INFO("(0x0418)DISP_REG_COLOR_TIMING_DETECTION_0                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_TIMING_DETECTION_0                     ));
                DDP_DRV_INFO("(0x041C)DISP_REG_COLOR_TIMING_DETECTION_1                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_TIMING_DETECTION_1                 ));
                DDP_DRV_INFO("(0x0420)DISP_REG_COLOR_DBG_CFG_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_DBG_CFG_MAIN                     ));
                DDP_DRV_INFO("(0x0428)DISP_REG_COLOR_C_BOOST_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_C_BOOST_MAIN                 ));
                DDP_DRV_INFO("(0x042C)DISP_REG_COLOR_C_BOOST_MAIN_2                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_C_BOOST_MAIN_2                 ));
                DDP_DRV_INFO("(0x0430)DISP_REG_COLOR_LUMA_ADJ                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_ADJ                     ));
                DDP_DRV_INFO("(0x0434)DISP_REG_COLOR_G_PIC_ADJ_MAIN_1                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_G_PIC_ADJ_MAIN_1                  ));
                DDP_DRV_INFO("(0x0438)DISP_REG_COLOR_G_PIC_ADJ_MAIN_2                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_G_PIC_ADJ_MAIN_2                     ));
                DDP_DRV_INFO("(0x0440)DISP_REG_COLOR_Y_FTN_1_0_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_1_0_MAIN                 ));
                DDP_DRV_INFO("(0x0444)DISP_REG_COLOR_Y_FTN_3_2_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_3_2_MAIN                     ));
                DDP_DRV_INFO("(0x0448)DISP_REG_COLOR_Y_FTN_5_4_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_5_4_MAIN                 ));
                DDP_DRV_INFO("(0x044C)DISP_REG_COLOR_Y_FTN_7_6_MAIN                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_7_6_MAIN                 ));
                DDP_DRV_INFO("(0x0450)DISP_REG_COLOR_Y_FTN_9_8_MAIN                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_9_8_MAIN                     ));
                DDP_DRV_INFO("(0x0454)DISP_REG_COLOR_Y_FTN_11_10_MAIN                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_11_10_MAIN                  ));
                DDP_DRV_INFO("(0x0458)DISP_REG_COLOR_Y_FTN_13_12_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_13_12_MAIN                     ));
                DDP_DRV_INFO("(0x045C)DISP_REG_COLOR_Y_FTN_15_14_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_15_14_MAIN                 ));
                DDP_DRV_INFO("(0x0460)DISP_REG_COLOR_Y_FTN_17_16_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_17_16_MAIN                     ));
                DDP_DRV_INFO("(0x0484)DISP_REG_COLOR_POS_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_POS_MAIN                 ));
                DDP_DRV_INFO("(0x0488)DISP_REG_COLOR_INK_DATA_MAIN                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INK_DATA_MAIN                 ));
                DDP_DRV_INFO("(0x048C)DISP_REG_COLOR_INK_DATA_MAIN_CR                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INK_DATA_MAIN_CR                     ));
                DDP_DRV_INFO("(0x0490)DISP_REG_COLOR_CAP_IN_DATA_MAIN                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CAP_IN_DATA_MAIN                  ));
                DDP_DRV_INFO("(0x0494)DISP_REG_COLOR_CAP_IN_DATA_MAIN_CR                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CAP_IN_DATA_MAIN_CR                     ));
                DDP_DRV_INFO("(0x0498)DISP_REG_COLOR_LUMA_HIST_00                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_00                 ));
                DDP_DRV_INFO("(0x049C)DISP_REG_COLOR_CAP_OUT_DATA_MAIN_CR                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CAP_OUT_DATA_MAIN_CR                     ));
                DDP_DRV_INFO("(0x0520)DISP_REG_COLOR_LUMA_HIST_00                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_00                 ));
                DDP_DRV_INFO("(0x0524)DISP_REG_COLOR_LUMA_HIST_01                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_01                 ));
                DDP_DRV_INFO("(0x0528)DISP_REG_COLOR_LUMA_HIST_02                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_02                     ));
                DDP_DRV_INFO("(0x052C)DISP_REG_COLOR_LUMA_HIST_03                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_03                  ));
                DDP_DRV_INFO("(0x0530)DISP_REG_COLOR_LUMA_HIST_04                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_04                     ));
                DDP_DRV_INFO("(0x0534)DISP_REG_COLOR_LUMA_HIST_05                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_05                 ));
                DDP_DRV_INFO("(0x0538)DISP_REG_COLOR_LUMA_HIST_06                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_06                     ));
                DDP_DRV_INFO("(0x053C)DISP_REG_COLOR_LUMA_HIST_07                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_07                 ));
                DDP_DRV_INFO("(0x0540)DISP_REG_COLOR_LUMA_HIST_08                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_08                 ));
                DDP_DRV_INFO("(0x0544)DISP_REG_COLOR_LUMA_HIST_09                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_09                     ));
                DDP_DRV_INFO("(0x0548)DISP_REG_COLOR_LUMA_HIST_10                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_10                  ));
                DDP_DRV_INFO("(0x054C)DISP_REG_COLOR_LUMA_HIST_11                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_11                     ));
                DDP_DRV_INFO("(0x0550)DISP_REG_COLOR_LUMA_HIST_12                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_12                 ));
                DDP_DRV_INFO("(0x0554)DISP_REG_COLOR_LUMA_HIST_13                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_13                     ));
                DDP_DRV_INFO("(0x0558)DISP_REG_COLOR_LUMA_HIST_14                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_14                 ));
                DDP_DRV_INFO("(0x055C)DISP_REG_COLOR_LUMA_HIST_15                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_15                 ));
                DDP_DRV_INFO("(0x0560)DISP_REG_COLOR_LUMA_HIST_16                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_16                     ));
                DDP_DRV_INFO("(0x05A4)DISP_REG_COLOR_LUMA_SUM                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_SUM                  ));
                DDP_DRV_INFO("(0x05A8)DISP_REG_COLOR_LUMA_MIN_MAX                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_MIN_MAX                     ));
                DDP_DRV_INFO("(0x0620)DISP_REG_COLOR_LOCAL_HUE_CD_0                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_0                 ));
                DDP_DRV_INFO("(0x0624)DISP_REG_COLOR_LOCAL_HUE_CD_1                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_1                     ));
                DDP_DRV_INFO("(0x0628)DISP_REG_COLOR_LOCAL_HUE_CD_2                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_2                 ));
                DDP_DRV_INFO("(0x062C)DISP_REG_COLOR_LOCAL_HUE_CD_3                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_3                 ));
                DDP_DRV_INFO("(0x0630)DISP_REG_COLOR_LOCAL_HUE_CD_4                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_4                     ));
                DDP_DRV_INFO("(0x0740)DISP_REG_COLOR_TWO_D_WINDOW_1                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_TWO_D_WINDOW_1                  ));
                DDP_DRV_INFO("(0x074C)DISP_REG_COLOR_TWO_D_W1_RESULT                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_TWO_D_W1_RESULT                     ));
                DDP_DRV_INFO("(0x0768)DISP_REG_COLOR_SAT_HIST_X_CFG_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_SAT_HIST_X_CFG_MAIN                 ));
                DDP_DRV_INFO("(0x076C)DISP_REG_COLOR_SAT_HIST_Y_CFG_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_SAT_HIST_Y_CFG_MAIN                     ));
                DDP_DRV_INFO("(0x07E0)DISP_REG_COLOR_CRC                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CRC(0)                 ));
                DDP_DRV_INFO("(0x07FC)DISP_REG_COLOR_PARTIAL_SAT_GAIN1                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_GAIN1(0)                 ));
                DDP_DRV_INFO("(0x0810)DISP_REG_COLOR_PARTIAL_SAT_GAIN2                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_GAIN2(0)                     ));
                DDP_DRV_INFO("(0x0824)DISP_REG_COLOR_PARTIAL_SAT_GAIN3                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_GAIN3(0)                  ));
                DDP_DRV_INFO("(0x0838)DISP_REG_COLOR_PARTIAL_SAT_POINT1                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_POINT1(0)                     ));
                DDP_DRV_INFO("(0x084C)DISP_REG_COLOR_PARTIAL_SAT_POINT2                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_POINT2(0)                 ));
                DDP_DRV_INFO("(0x0F00)DISP_REG_COLOR_START             =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_START           ));
                DDP_DRV_INFO("(0x0F04)DISP_REG_COLOR_INTEN             =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INTEN           ));
                DDP_DRV_INFO("(0x0F08)DISP_REG_COLOR_INTSTA            =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INTSTA          ));
                DDP_DRV_INFO("(0x0F0C)DISP_REG_COLOR_OUT_SEL           =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_OUT_SEL         ));
                DDP_DRV_INFO("(0x0F10)DISP_REG_COLOR_FRAME_DONE_DEL    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_FRAME_DONE_DEL  ));
                DDP_DRV_INFO("(0x0F14)DISP_REG_COLOR_CRC_EN               =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CRC_EN             ));
                DDP_DRV_INFO("(0x0F18)DISP_REG_COLOR_SW_SCRATCH        =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_SW_SCRATCH        ));
                DDP_DRV_INFO("(0x0F20)DISP_REG_COLOR_RDY_SEL           =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_RDY_SEL           ));
                DDP_DRV_INFO("(0x0F24)DISP_REG_COLOR_RDY_SEL_EN        =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_RDY_SEL_EN        ));
                DDP_DRV_INFO("(0x0F28)DISP_REG_COLOR_CK_ON             =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CK_ON             ));
                DDP_DRV_INFO("(0x0F50)DISP_REG_COLOR_INTERNAL_IP_WIDTH =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INTERNAL_IP_WIDTH ));
                DDP_DRV_INFO("(0x0F54)DISP_REG_COLOR_INTERNAL_IP_HEIGHT=0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INTERNAL_IP_HEIGHT));
                DDP_DRV_INFO("(0x0F60)DISP_REG_COLOR_CM1_EN            =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CM1_EN            ));
                DDP_DRV_INFO("(0x0FA0)DISP_REG_COLOR_CM2_EN            =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CM2_EN            ));
                DDP_DRV_INFO("(0x0FF0)DISP_REG_COLOR_R0_CRC            =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_R0_CRC            ));
                break;
                     
            case DISP_MODULE_BLS:    
                DDP_DRV_INFO("===== DISP BLS Reg Dump: ============\n");
                DDP_DRV_INFO("(0x0 )BLS_EN              =0x%x \n", DISP_REG_GET(DISP_REG_BLS_EN               ));
                DDP_DRV_INFO("(0x4 )BLS_RST               =0x%x \n", DISP_REG_GET(DISP_REG_BLS_RST                ));
                DDP_DRV_INFO("(0x8 )BLS_INTEN           =0x%x \n", DISP_REG_GET(DISP_REG_BLS_INTEN              ));
                DDP_DRV_INFO("(0xC )BLS_INTSTA          =0x%x \n", DISP_REG_GET(DISP_REG_BLS_INTSTA           ));
                DDP_DRV_INFO("(0x10)BLS_BLS_SETTING     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_BLS_SETTING      ));
                DDP_DRV_INFO("(0x14)BLS_FANA_SETTING      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_FANA_SETTING     ));
                DDP_DRV_INFO("(0x18)BLS_SRC_SIZE          =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SRC_SIZE         ));
                DDP_DRV_INFO("(0x20)BLS_GAIN_SETTING      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAIN_SETTING     ));
                DDP_DRV_INFO("(0x24)BLS_MANUAL_GAIN       =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MANUAL_GAIN        ));
                DDP_DRV_INFO("(0x28)BLS_MANUAL_MAXCLR   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MANUAL_MAXCLR      ));
                DDP_DRV_INFO("(0x30)BLS_GAMMA_SETTING   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAMMA_SETTING      ));
                DDP_DRV_INFO("(0x34)BLS_GAMMA_BOUNDARY  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAMMA_BOUNDARY   ));
                DDP_DRV_INFO("(0x38)BLS_LUT_UPDATE      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_LUT_UPDATE       ));
                DDP_DRV_INFO("(0x60)BLS_MAXCLR_THD      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_THD       ));
                DDP_DRV_INFO("(0x64)BLS_DISTPT_THD      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_THD       ));
                DDP_DRV_INFO("(0x68)BLS_MAXCLR_LIMIT      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_LIMIT     ));
                DDP_DRV_INFO("(0x6C)BLS_DISTPT_LIMIT      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_LIMIT     ));
                DDP_DRV_INFO("(0x70)BLS_AVE_SETTING       =0x%x \n", DISP_REG_GET(DISP_REG_BLS_AVE_SETTING        ));
                DDP_DRV_INFO("(0x74)BLS_AVE_LIMIT       =0x%x \n", DISP_REG_GET(DISP_REG_BLS_AVE_LIMIT          ));
                DDP_DRV_INFO("(0x78)BLS_DISTPT_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_SETTING   ));
                DDP_DRV_INFO("(0x7C)BLS_HIS_CLEAR       =0x%x \n", DISP_REG_GET(DISP_REG_BLS_HIS_CLEAR          ));
                DDP_DRV_INFO("(0x80)BLS_SC_DIFF_THD     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SC_DIFF_THD      ));
                DDP_DRV_INFO("(0x84)BLS_SC_BIN_THD      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SC_BIN_THD       ));
                DDP_DRV_INFO("(0x88)BLS_MAXCLR_GRADUAL  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_GRADUAL   ));
                DDP_DRV_INFO("(0x8C)BLS_DISTPT_GRADUAL  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_GRADUAL   ));
                DDP_DRV_INFO("(0x90)BLS_FAST_IIR_XCOEFF =0x%x \n", DISP_REG_GET(DISP_REG_BLS_FAST_IIR_XCOEFF  ));
                DDP_DRV_INFO("(0x94)BLS_FAST_IIR_YCOEFF =0x%x \n", DISP_REG_GET(DISP_REG_BLS_FAST_IIR_YCOEFF  ));
                DDP_DRV_INFO("(0x98)BLS_SLOW_IIR_XCOEFF =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_XCOEFF  ));
                DDP_DRV_INFO("(0x9C)BLS_SLOW_IIR_YCOEFF =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_YCOEFF  ));
                DDP_DRV_INFO("(0xA0)BLS_PWM_DUTY          =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY         ));
                DDP_DRV_INFO("(0xA4)BLS_PWM_GRADUAL     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_GRADUAL      ));
                DDP_DRV_INFO("(0xA8)BLS_PWM_CON         =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_CON          ));
                DDP_DRV_INFO("(0xAC)BLS_PWM_MANUAL      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_MANUAL       ));
                DDP_DRV_INFO("(0xB0)BLS_DEBUG           =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DEBUG              ));
                DDP_DRV_INFO("(0xB4)BLS_PATTERN         =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PATTERN          ));
                DDP_DRV_INFO("(0xB8)BLS_CHKSUM          =0x%x \n", DISP_REG_GET(DISP_REG_BLS_CHKSUM           ));
                //DDP_DRV_INFO("(0x100)BLS_HIS_BIN        =0x%x \n", DISP_REG_GET(DISP_REG_BLS_HIS_BIN          ));
                DDP_DRV_INFO("(0x200)BLS_PWM_DUTY_RD    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY_RD      ));
                DDP_DRV_INFO("(0x204)BLS_FRAME_AVE_RD   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_FRAME_AVE_RD     ));
                DDP_DRV_INFO("(0x208)BLS_MAXCLR_RD      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_RD          ));
                DDP_DRV_INFO("(0x20C)BLS_DISTPT_RD      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_RD          ));
                DDP_DRV_INFO("(0x210)BLS_GAIN_RD        =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAIN_RD          ));
                DDP_DRV_INFO("(0x214)BLS_SC_RD          =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SC_RD            ));
                //DDP_DRV_INFO("(0x300)BLS_LUMINANCE      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_LUMINANCE        ));
                //DDP_DRV_INFO("(0x384)BLS_LUMINANCE_255  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_LUMINANCE_255    ));
                //DDP_DRV_INFO("(0x400)BLS_GAMMA_LUT      =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAMMA_LUT        ));
                DDP_DRV_INFO("(0xE00)BLS_DITHER_0                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER(0)                 ));
                DDP_DRV_INFO("(0xF00)BLS_DUMMY                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DUMMY                  ));
            break;
                     
            case DISP_MODULE_WDMA0:
                index = 0;
                DDP_DRV_INFO("===== DISP WDMA%d Reg Dump: ============\n", index);   
                DDP_DRV_INFO("(000)WDMA_INTEN      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_INTEN        ));
                DDP_DRV_INFO("(004)WDMA_INTSTA     =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_INTSTA       ));
                DDP_DRV_INFO("(008)WDMA_EN         =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_EN           ));
                DDP_DRV_INFO("(00C)WDMA_RST            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_RST          ));
                DDP_DRV_INFO("(010)WDMA_SMI_CON        =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_SMI_CON      ));
                DDP_DRV_INFO("(014)WDMA_CFG            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CFG          ));
                DDP_DRV_INFO("(018)WDMA_SRC_SIZE       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE     ));
                DDP_DRV_INFO("(01C)WDMA_CLIP_SIZE  =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE    ));
                DDP_DRV_INFO("(020)WDMA_CLIP_COORD =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_COORD   ));
                DDP_DRV_INFO("(024)WDMA_DST_ADDR       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR     ));
                DDP_DRV_INFO("(028)WDMA_DST_W_IN_BYTE =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_W_IN_BYTE));
                DDP_DRV_INFO("(02C)WDMA_ALPHA      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_ALPHA        ));
                DDP_DRV_INFO("(030)WDMA_BUF_ADDR       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_ADDR     ));
                DDP_DRV_INFO("(034)WDMA_STA            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_STA          ));
                DDP_DRV_INFO("(038)WDMA_BUF_CON1       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON1     ));
                DDP_DRV_INFO("(03C)WDMA_BUF_CON2       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON2     ));
                DDP_DRV_INFO("(040)WDMA_C00            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C00          ));
                DDP_DRV_INFO("(044)WDMA_C02            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C02          ));
                DDP_DRV_INFO("(048)WDMA_C10            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C10          ));
                DDP_DRV_INFO("(04C)WDMA_C12            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C12          ));
                DDP_DRV_INFO("(050)WDMA_C20            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C20          ));
                DDP_DRV_INFO("(054)WDMA_C22            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C22          ));
                DDP_DRV_INFO("(058)WDMA_PRE_ADD0       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD0     ));
                DDP_DRV_INFO("(05C)WDMA_PRE_ADD2       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD2     ));
                DDP_DRV_INFO("(060)WDMA_POST_ADD0  =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD0    ));
                DDP_DRV_INFO("(064)WDMA_POST_ADD2  =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD2    ));
                DDP_DRV_INFO("(070)WDMA_DST_U_ADDR =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_U_ADDR   ));
                DDP_DRV_INFO("(074)WDMA_DST_V_ADDR =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_V_ADDR   ));
                DDP_DRV_INFO("(078)WDMA_DST_UV_PITCH   =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_UV_PITCH     ));
                DDP_DRV_INFO("(090)WDMA_DITHER_CON =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DITHER_CON   ));
                DDP_DRV_INFO("(0A0)WDMA_FLOW_CTRL_DBG =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_FLOW_CTRL_DBG));
                DDP_DRV_INFO("(0A4)WDMA_EXEC_DBG       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_EXEC_DBG     ));
                DDP_DRV_INFO("(0A8)WDMA_CLIP_DBG       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_DBG     ));
                break;    
               
            case DISP_MODULE_RDMA0:  
                index = 0;
                DDP_DRV_INFO("===== DISP RDMA%d Reg Dump: ======== \n", index);
                DDP_DRV_INFO("(000)RDMA_INT_ENABLE   =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE     ));
                DDP_DRV_INFO("(004)RDMA_INT_STATUS   =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_INT_STATUS     ));
                DDP_DRV_INFO("(010)RDMA_GLOBAL_CON   =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON     ));
                DDP_DRV_INFO("(014)RDMA_SIZE_CON_0   =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0     ));
                DDP_DRV_INFO("(018)RDMA_SIZE_CON_1   =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1     ));
                DDP_DRV_INFO("(024)RDMA_MEM_CON         =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_CON         ));
                DDP_DRV_INFO("(028)RDMA_MEM_START_ADDR  =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR ));
                DDP_DRV_INFO("(02C)RDMA_MEM_SRC_PITCH   =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH  ));
                DDP_DRV_INFO("(030)RDMA_MEM_GMC_SETTING_0 =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0));
                DDP_DRV_INFO("(034)RDMA_MEM_SLOW_CON    =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON   ));
                DDP_DRV_INFO("(030)RDMA_MEM_GMC_SETTING_1 =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1));
                DDP_DRV_INFO("(040)RDMA_FIFO_CON        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_FIFO_CON    ));
                DDP_DRV_INFO("(054)RDMA_CF_00        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_00          ));
                DDP_DRV_INFO("(058)RDMA_CF_01        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_01          ));
                DDP_DRV_INFO("(05C)RDMA_CF_02        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_02          ));
                DDP_DRV_INFO("(060)RDMA_CF_10        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_10          ));
                DDP_DRV_INFO("(064)RDMA_CF_11        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_11          ));
                DDP_DRV_INFO("(068)RDMA_CF_12        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_12          ));
                DDP_DRV_INFO("(06C)RDMA_CF_20        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_20          ));
                DDP_DRV_INFO("(070)RDMA_CF_21        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_21          ));
                DDP_DRV_INFO("(074)RDMA_CF_22        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_22          ));
                DDP_DRV_INFO("(078)RDMA_CF_PRE_ADD0      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD0    ));
                DDP_DRV_INFO("(07C)RDMA_CF_PRE_ADD1      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD1    ));
                DDP_DRV_INFO("(080)RDMA_CF_PRE_ADD2      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD2    ));
                DDP_DRV_INFO("(084)RDMA_CF_POST_ADD0     =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD0   ));
                DDP_DRV_INFO("(088)RDMA_CF_POST_ADD1     =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD1   ));
                DDP_DRV_INFO("(08C)RDMA_CF_POST_ADD2     =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD2   ));      
                DDP_DRV_INFO("(090)RDMA_DUMMY            =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_DUMMY          ));
                DDP_DRV_INFO("(094)RDMA_DEBUG_OUT_SEL    =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL  ));
                break;
    
            case DISP_MODULE_DPI0:   
            case DISP_MODULE_DSI_VDO:   
            case DISP_MODULE_DSI_CMD:
                break;
    
            case DISP_MODULE_MUTEX:
                DDP_DRV_INFO("===== DISP DISP_REG_MUTEX_CONFIG Reg Dump: ============\n");
                DDP_DRV_INFO("(0x0  )DISP_MUTEX_INTEN        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTEN         ));
                DDP_DRV_INFO("(0x4  )CONFIG_MUTEX_INTSTA       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA        ));
                DDP_DRV_INFO("(0x8  )CONFIG_REG_UPD_TIMEOUT    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_REG_UPD_TIMEOUT     ));
                DDP_DRV_INFO("(0xC  )CONFIG_REG_COMMIT         =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT          ));
                DDP_DRV_INFO("(0x20)CONFIG_MUTEX0_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_EN           ));
                DDP_DRV_INFO("(0x24)CONFIG_MUTEX0              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0              ));
                DDP_DRV_INFO("(0x28)CONFIG_MUTEX0_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_RST          ));
                DDP_DRV_INFO("(0x2C)CONFIG_MUTEX0_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_MOD          ));
                DDP_DRV_INFO("(0x30)CONFIG_MUTEX0_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_SOF          ));
                DDP_DRV_INFO("(0x40)CONFIG_MUTEX1_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_EN           ));
                DDP_DRV_INFO("(0x44)CONFIG_MUTEX1              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1              ));
                DDP_DRV_INFO("(0x48)CONFIG_MUTEX1_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_RST          ));
                DDP_DRV_INFO("(0x4C)CONFIG_MUTEX1_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_MOD          ));
                DDP_DRV_INFO("(0x50)CONFIG_MUTEX1_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_SOF          ));
                DDP_DRV_INFO("(0x60)CONFIG_MUTEX2_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_EN           ));
                DDP_DRV_INFO("(0x64)CONFIG_MUTEX2              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2              ));
                DDP_DRV_INFO("(0x68)CONFIG_MUTEX2_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_RST          ));
                DDP_DRV_INFO("(0x6C)CONFIG_MUTEX2_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_MOD          ));
                DDP_DRV_INFO("(0x70)CONFIG_MUTEX2_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_SOF          ));
                DDP_DRV_INFO("(0x80)CONFIG_MUTEX3_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_EN           ));
                DDP_DRV_INFO("(0x84)CONFIG_MUTEX3              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3              ));
                DDP_DRV_INFO("(0x88)CONFIG_MUTEX3_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_RST          ));
                DDP_DRV_INFO("(0x8C)CONFIG_MUTEX3_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_MOD          ));
                DDP_DRV_INFO("(0x90)CONFIG_MUTEX3_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_SOF          ));
                DDP_DRV_INFO("(0xA0)CONFIG_MUTEX4_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_EN           ));
                DDP_DRV_INFO("(0xA4)CONFIG_MUTEX4              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4              ));
                DDP_DRV_INFO("(0xA8)CONFIG_MUTEX4_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_RST          ));
                DDP_DRV_INFO("(0xAC)CONFIG_MUTEX4_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_MOD          ));
                DDP_DRV_INFO("(0xB0)CONFIG_MUTEX4_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_SOF          ));
                DDP_DRV_INFO("(0xC0)CONFIG_MUTEX5_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_EN           ));
                DDP_DRV_INFO("(0xC4)CONFIG_MUTEX5              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5              ));
                DDP_DRV_INFO("(0xC8)CONFIG_MUTEX5_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_RST          ));
                DDP_DRV_INFO("(0xCC)CONFIG_MUTEX5_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_MOD          ));
                DDP_DRV_INFO("(0xD0)CONFIG_MUTEX5_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_SOF          ));
                DDP_DRV_INFO("(0x100)CONFIG_MUTEX_DEBUG_OUT_SEL=0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_DEBUG_OUT_SEL ));
            break;
            
            default: DDP_DRV_INFO("error, reg_dump, unknow module=%d \n", module);
    
        }
    
        return 0;
}

void disp_m4u_dump_reg(void)
{
    // dump display info
    disp_dump_reg(DISP_MODULE_OVL);
    disp_dump_reg(DISP_MODULE_WDMA0);
    disp_dump_reg(DISP_MODULE_MUTEX);
    disp_dump_reg(DISP_MODULE_CONFIG);

    // dump mdp info
    //dumpMDPRegInfo();
}

int disp_module_clock_on(DISP_MODULE_ENUM module, char* caller_name)
{
    return 0;
}

int disp_module_clock_off(DISP_MODULE_ENUM module, char* caller_name)
{
    return 0;
}


module_init(disp_init);
module_exit(disp_exit);
MODULE_AUTHOR("Tzu-Meng, Chung <Tzu-Meng.Chung@mediatek.com>");
MODULE_DESCRIPTION("Display subsystem Driver");
MODULE_LICENSE("GPL");
