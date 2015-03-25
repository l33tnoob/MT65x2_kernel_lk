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
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <mach/mt_smi.h>

#include <linux/xlog.h>
#include <linux/proc_fs.h>  //proc file use
//ION
#include <linux/ion.h>
#include <linux/ion_drv.h>
//#include <mach/m4u.h>

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

#define DISP_INDEX_OFFSET 0x0 // must be consistent with ddp_rdma.c

//#include <asm/tcm.h>
unsigned int dbg_log = 0;
unsigned int irq_log = 0;  // must disable irq level log by default, else will block uart output, open it only for debug use
unsigned int irq_err_log = 0;

#if 0  // defined in ddp_debug.h
#define DISP_WRN(string, args...) if(dbg_log) printk("[DSS]"string,##args)
#define DISP_MSG(string, args...) if(0) printk("[DSS]"string,##args)
#define DISP_ERR(string, args...) if(dbg_log) printk("[DSS]error:"string,##args)
#define DISP_IRQ(string, args...) if(irq_log) printk("[DSS]"string,##args)
#endif

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
//M4U_PORT_STRUCT m4uPort;
//irq
#define DISP_REGISTER_IRQ(irq_num){\
    if(request_irq( irq_num , (irq_handler_t)disp_irq_handler, IRQF_TRIGGER_LOW, DISP_DEVNAME , NULL))\
    { DISP_ERR("ddp register irq failed! %d\n", irq_num); }}

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

wait_queue_head_t cmq_wait_queue[CMDQ_THREAD_NUM];

//G2d Variables
spinlock_t gResourceLock;
unsigned int gLockedResource;//lock dpEngineType_6582
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
//Clock gate management
//static unsigned long g_u4ClockOnTbl = 0;

//PQ variables
extern UINT32 fb_width;
extern UINT32 fb_height;
extern unsigned char aal_debug_flag;

// IRQ log print kthread
static struct task_struct *disp_irq_log_task = NULL;
static wait_queue_head_t disp_irq_log_wq;
static volatile int disp_irq_log_module = 0;

static DISPLAY_TDSHP_T g_TDSHP_Index; 

static int g_irq_err_print = 0; // print aee warning 2s one time
#define DDP_ERR_IRQ_INTERVAL_TIME 5
static unsigned int disp_irq_err = 0;
unsigned int cnt_rdma_underflow = 0;
unsigned int cnt_rdma1_underflow = 0;
unsigned int cnt_rdma_abnormal = 0;
unsigned int cnt_rdma1_abnormal = 0;
unsigned int cnt_ovl_underflow = 0;
unsigned int cnt_ovl_abnormal = 0;
unsigned int cnt_wdma_underflow = 0;
unsigned int cnt_mutex_timeout = 0;
#define DDP_IRQ_OVL_L0_ABNORMAL  (1<<0)
#define DDP_IRQ_OVL_L1_ABNORMAL  (1<<1)
#define DDP_IRQ_OVL_L2_ABNORMAL  (1<<2)
#define DDP_IRQ_OVL_L3_ABNORMAL  (1<<3)
#define DDP_IRQ_OVL_L0_UNDERFLOW (1<<4)
#define DDP_IRQ_OVL_L1_UNDERFLOW (1<<5)
#define DDP_IRQ_OVL_L2_UNDERFLOW (1<<6)
#define DDP_IRQ_OVL_L3_UNDERFLOW (1<<7)
#define DDP_IRQ_RDMA_ABNORMAL       (1<<8)
#define DDP_IRQ_RDMA_UNDERFLOW      (1<<9)
#define DDP_IRQ_WDMA_ABNORMAL       (1<<10)
#define DDP_IRQ_WDMA_UNDERFLOW      (1<<11)

static struct timer_list disp_irq_err_timer;
static void disp_irq_err_timer_handler(unsigned long lparam)
{
    g_irq_err_print = 1;
}

DISPLAY_TDSHP_T *get_TDSHP_index(void)  
{    
    return &g_TDSHP_Index;
}


// internal function
static int disp_wait_intr(DISP_MODULE_ENUM module, unsigned int timeout_ms);
static int disp_get_mutex_status(void);
static int disp_set_needupdate(DISP_MODULE_ENUM eModule , unsigned long u4En);
static void disp_power_off(DISP_MODULE_ENUM eModule , unsigned int * pu4Record);
static void disp_power_on(DISP_MODULE_ENUM eModule , unsigned int * pu4Record);
extern void DpEngine_COLORonConfig(unsigned int srcWidth,unsigned int srcHeight);
extern void DpEngine_COLORonInit(void);

extern void cmdqForceFreeAll(int cmdqThread);
extern void cmdqForceFree_SW(int taskID);
bool checkMdpEngineStatus(unsigned int engineFlag);

#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT) && (CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)
extern unsigned int init_tplay_handle(void);
extern int write_tplay_handle(unsigned int handle_value);
extern int set_tplay_handle_addr_request(void);
#endif

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

void disp_check_clock_tree(void) {
	unsigned int reg = 0;
	unsigned int mutexID = 0;
	unsigned int mutex_mod = 0;
	unsigned int mutex_sof = 0;

	DISP_MSG("0xf0000000=0x%x, 0xf0000050=0x%x, 0xf0000040=0x%x\n",
			*(volatile unsigned int*)(0xf0000000),
			*(volatile unsigned int*)(0xf0000050),
			*(volatile unsigned int*)(0xf0000040));
	// All need
	if ((DISP_REG_GET(0xf0000040)&0xff) != 0x01) {
		DISP_ERR("CLK_CFG_0 abnormal: hf_faxi_ck is off! 0xf0000040=0x%x \n", DISP_REG_GET(0xf0000040));
	}
	if ((DISP_REG_GET(0xf0000040)&0xff000000) != 0x1000000) {
		DISP_ERR("CLK_CFG_0 abnormal: hf_fmm_ck is off! 0xf0000040=0x%x \n", DISP_REG_GET(0xf0000040));
	}
	if (DISP_REG_GET(0xf4000100)&(1<<0) != 0) {
		DISP_ERR("MMSYS_CG_CON0 abnormal: SMI_COMMON is off!\n");
	}
	if (DISP_REG_GET(0xf4000100)&(1<<1) != 0) {
		DISP_ERR("MMSYS_CG_CON0 abnormal: SMI_LARB0 is off!\n");
	}
	if (DISP_REG_GET(0xf4000100)&(1<<3) != 0) {
		DISP_ERR("MMSYS_CG_CON0 abnormal: MUTEX is off!\n");
	}
	for (mutexID = 0; mutexID < 4; mutexID++) {
		mutex_mod = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(mutexID));
		mutex_sof = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(mutexID));
		if (mutex_mod&(1<<3)) {
			if (DISP_REG_GET(0xf4000100)&(1<<8) != 0) {
				DISP_ERR("MMSYS_CG_CON0 abnormal: DISP_OVL is off!\n");
			}
		}
		if (mutex_mod&(1<<6)) {
			if (DISP_REG_GET(0xf4000100)&(1<<6) != 0) {
				DISP_MSG("MMSYS_CG_CON0 abnormal: DISP_wdma is off!\n");
			}
		}
		if (mutex_mod&(1<<7)) {
			if (DISP_REG_GET(0xf4000100)&(1<<4) != 0) {
				DISP_ERR("MMSYS_CG_CON0 abnormal: DISP_COLOR is off!\n");
			}
		}
		if (mutex_mod&(1<<9)) {
			// MMSYS_CG_CON0
			if (DISP_REG_GET(0xf4000100)&(1<<5) != 0) {
				DISP_ERR("MMSYS_CG_CON0 abnormal: DISP_BLS is off!\n");
			}
			//CLK_CFG_1
			if (DISP_REG_GET(0xf0000050)&0x0ff == 0) {
				DISP_ERR("CLK_CFG_1 abnormal: fpwm_ck is off!\n");
			}
		}
		if (mutex_mod&(1<<10)) {
			if (DISP_REG_GET(0xf4000100)&(1<<7) != 0) {
				DISP_ERR("MMSYS_CG_CON0 abnormal: DISP_RDMA is off!\n");
			}
		}
		// DSI CMD/VDO
		if (mutex_sof&0x3) {
			// MMSYS_CG_CON1
			if (DISP_REG_GET(0xf4000110)&0x07 != 0) {
				DISP_ERR("MMSYS_CG_CON1 abnormal: DSI is off!\n");
			}
			// CLK_CFG_1
			if (DISP_REG_GET(0xf0000100)&(1<<37) == 0) {
				DISP_ERR("CLK_CFG_1 abnormal: ad_dsi0_intc_dsiclk is off!\n");
			}
		}
		// DPI
		if (mutex_sof&(1<<2)) {
			// CLK_CFG_1
			if (DISP_REG_GET(0xf0000104)&(1<<28) == 0) {
				DISP_ERR("CLK_CFG_1 abnormal: hf_dpi0_ck is off!\n");
			}
		}
	}
}

void disp_print_reg(DISP_MODULE_ENUM module);
static int disp_irq_log_kthread_func(void *data)
{
    unsigned int i=0;
    while(1)
    {
        wait_event_interruptible(disp_irq_log_wq, disp_irq_log_module);
        DISP_MSG("disp_irq_log_kthread_func dump intr register: disp_irq_log_module=%d \n", disp_irq_log_module);
        for(i=0;i<DISP_MODULE_MAX;i++)
        {
            if( (disp_irq_log_module&(1<<i))!=0 )
            {
                disp_print_reg(i);
            }
        }
        // reset wakeup flag
        disp_irq_log_module = 0;

        if((disp_irq_err==1) && (g_irq_err_print==1))
        {
            #if 0
            if(disp_irq_err&DDP_IRQ_OVL_L0_ABNORMAL)
            {
               DDP_IRQ_ERR("OVL_RDMA0_ABNORMAL");
            }
            if(disp_irq_err&DDP_IRQ_OVL_L1_ABNORMAL)
            {
               DDP_IRQ_ERR("OVL_RDMA1_ABNORMAL");
            }
            if(disp_irq_err&DDP_IRQ_OVL_L2_ABNORMAL)
            {
               DDP_IRQ_ERR("OVL_RDMA2_ABNORMAL");
            }
            if(disp_irq_err&DDP_IRQ_OVL_L3_ABNORMAL)
            {
               DDP_IRQ_ERR("OVL_RDMA3_ABNORMAL");
            }
            if(disp_irq_err&DDP_IRQ_OVL_L0_UNDERFLOW)
            {
               DDP_IRQ_ERR("OVL_RDMA0_ABNORMAL");
            }
            if(disp_irq_err&DDP_IRQ_OVL_L1_UNDERFLOW)
            {
               DDP_IRQ_ERR("OVL_RDMA1_ABNORMAL");
            }
            if(disp_irq_err&DDP_IRQ_OVL_L2_UNDERFLOW)
            {
               DDP_IRQ_ERR("OVL_RDMA2_ABNORMAL");
            }
            if(disp_irq_err&DDP_IRQ_OVL_L3_UNDERFLOW)
            {
               DDP_IRQ_ERR("OVL_RDMA3_ABNORMAL");
            }
            #endif            
            if(disp_irq_err&DDP_IRQ_RDMA_ABNORMAL)
            {
               DDP_IRQ_ERR("RDMA_ABNORMAL");
            }
            if(disp_irq_err&DDP_IRQ_RDMA_UNDERFLOW)
            {
               DDP_IRQ_ERR("RDMA_UNDERFLOW");
            }
            if(disp_irq_err&DDP_IRQ_WDMA_UNDERFLOW)
            {
               DDP_IRQ_ERR("WDMA_UNDERFLOW");
            }
            disp_irq_err = 0;
            
            g_irq_err_print = 0;  // at most, 5s print one frame
            mod_timer(&disp_irq_err_timer, jiffies + DDP_ERR_IRQ_INTERVAL_TIME*HZ);
        }
        
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

    printk("disp_lock_cmdq_thread()called \n");
    
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

    printk("disp_lock_cmdq_thread(), i=%d \n", i);

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
    unsigned long long start_time = sched_clock();
        
    MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagStart, 0, module);
    // wait until irq done or timeout
    ret = wait_event_interruptible_timeout(
                    g_disp_irq_done_queue, 
                    g_disp_irq.irq_src & (1<<module), 
                    msecs_to_jiffies(timeout_ms) );
                    
    /*wake-up from sleep*/
    if(ret==0) // timeout
    {
        MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagPulse, 0, module);
        MMProfileLog(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd);
        DISP_ERR("Wait Done Timeout! pid=%d, module=%d \n", current->pid ,module);
        ddp_dump_info(module);
        ddp_dump_info(DISP_MODULE_CONFIG);
        ddp_dump_info(DISP_MODULE_MUTEX);
        disp_check_clock_tree();
        return -EAGAIN;
    }
    else if(ret<0) // intr by a signal
    {
        MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagPulse, 1, module);
        MMProfileLog(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd);
        DISP_ERR("Wait Done interrupted by a signal! pid=%d, module=%d \n", current->pid ,module);
        ddp_dump_info(module);
        return -EAGAIN;                 
    }

    MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd, 0, module);
    spin_lock_irqsave( &g_disp_irq.irq_lock , flags );
    g_disp_irq.irq_src &= ~(1<<module);    
    spin_unlock_irqrestore( &g_disp_irq.irq_lock , flags );

    end_time = sched_clock();
   	DISP_DBG("disp_wait_intr wait %d us\n", ((unsigned int)end_time-(unsigned int)start_time)/1000);

    return 0;
}

int disp_register_irq(DISP_MODULE_ENUM module, DDP_IRQ_CALLBACK cb)
{
    int i;
    if (module >= DISP_MODULE_MAX)
    {
        DISP_ERR("Register IRQ with invalid module ID. module=%d\n", module);
        return -1;
    }
    if (cb == NULL)
    {
        DISP_ERR("Register IRQ with invalid cb.\n");
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
        DISP_ERR("No enough callback entries for module %d.\n", module);
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
        DISP_ERR("Try to unregister callback function with was not registered. module=%d cb=0x%08X\n", module, (unsigned int)cb);
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
            //DISP_ERR("Invoke callback function. module=%d param=0x%X\n", module, param);
            g_disp_irq_table[module][i](param);
        }
    }
}


//extern void hdmi_test_switch_buffer(void);
static /*__tcmfunc*/ irqreturn_t disp_irq_handler(int irq, void *dev_id)
{
    unsigned long reg_val; 
    //unsigned long index;
    unsigned long value;
    int i;
    //struct timeval tv;
    int taskid;
    unsigned int index;
    /*1. Process ISR*/
    switch(irq)
    {
            
        case MT6582_DISP_OVL_IRQ_ID:  
                reg_val = DISP_REG_GET(DISP_REG_OVL_INTSTA);
                if(reg_val&(1<<0))
                {
                      DISP_IRQ("IRQ: OVL reg update done! \n");
                }    
                if(reg_val&(1<<1))
                {
                      DISP_IRQ("IRQ: OVL frame done! \n");
                      g_disp_irq.irq_src |= (1<<DISP_MODULE_OVL);
                }
                if(reg_val&(1<<2))
                {
                	if (cnt_ovl_underflow % 256 == 0) {
                		DISP_ERR("IRQ: OVL frame underrun! cnt=%d \n", cnt_ovl_underflow++);
                		disp_check_clock_tree();
                        disp_irq_log_module |= (1<<DISP_MODULE_OVL);
                        disp_irq_log_module |= (1<<DISP_MODULE_RDMA);   
                        disp_irq_log_module |= (1<<DISP_MODULE_CONFIG);
                        disp_irq_log_module |= (1<<DISP_MODULE_MUTEX);
                        
                        ddp_dump_info(DISP_MODULE_OVL);
                        ddp_dump_info(DISP_MODULE_RDMA);
                        ddp_dump_info(DISP_MODULE_COLOR);
                        ddp_dump_info(DISP_MODULE_BLS);
                        ddp_dump_info(DISP_MODULE_CONFIG);
                        ddp_dump_info(DISP_MODULE_MUTEX); 
                        ddp_dump_info(DISP_MODULE_DSI_CMD);
                                               
                        disp_dump_reg(DISP_MODULE_OVL);
                        disp_dump_reg(DISP_MODULE_RDMA);  
                        disp_dump_reg(DISP_MODULE_CONFIG);
                        disp_dump_reg(DISP_MODULE_MUTEX);
                    }
                    else
                    {
                        cnt_ovl_underflow++;
                    }
                      
                }
                if(reg_val&(1<<3))
                {
                      DISP_IRQ("IRQ: OVL SW reset done! \n");
                }
                if(reg_val&(1<<4))
                {
                      DISP_IRQ("IRQ: OVL HW reset done! \n");
                }
                if(reg_val&(1<<5))
                {
                      DISP_ERR("IRQ: OVL-L0 not complete untill EOF! \n");
                      disp_irq_err |= DDP_IRQ_OVL_L0_ABNORMAL;
                      disp_check_clock_tree();
                }      
                if(reg_val&(1<<6))
                {
                      DISP_ERR("IRQ: OVL-L1 not complete untill EOF! \n");
                      disp_irq_err |= DDP_IRQ_OVL_L1_ABNORMAL;
                      disp_check_clock_tree();
                }
                if(reg_val&(1<<7))
                {
                      DISP_ERR("IRQ: OVL-L2 not complete untill EOF! \n");
                      disp_irq_err |= DDP_IRQ_OVL_L2_ABNORMAL;
                      disp_check_clock_tree();
                }        
                if(reg_val&(1<<8))
                {
                      DISP_ERR("IRQ: OVL-L3 not complete untill EOF! \n");
                      disp_irq_err |= DDP_IRQ_OVL_L3_ABNORMAL;
                      disp_check_clock_tree();
                }
                if(reg_val&(1<<9))
                {
                      if(cnt_ovl_underflow%256==0)
                      {
                          DISP_ERR("IRQ: OVL-L0 fifo underflow! \n");
                      }
                      disp_irq_err |= DDP_IRQ_OVL_L0_UNDERFLOW;
                      disp_check_clock_tree();
                }      
                if(reg_val&(1<<10))
                {
                      if(cnt_ovl_underflow%256==0)
                      {
                          DISP_ERR("IRQ: OVL-L1 fifo underflow! \n");
                      }
                      disp_irq_err |= DDP_IRQ_OVL_L1_UNDERFLOW;
                      disp_check_clock_tree();
                }
                if(reg_val&(1<<11))
                {
                      if(cnt_ovl_underflow%256==0)
                      {
                          DISP_ERR("IRQ: OVL-L2 fifo underflow! \n");
                      }
                      disp_irq_err |= DDP_IRQ_OVL_L2_UNDERFLOW;
                      disp_check_clock_tree();
                }    
                if(reg_val&(1<<12))
                {
                      if(cnt_ovl_underflow%256==0)
                      {
                          DISP_ERR("IRQ: OVL-L3 fifo underflow! \n");
                      }
                      disp_irq_err |= DDP_IRQ_OVL_L3_UNDERFLOW;
                      disp_check_clock_tree();
                }                                                                                                  
                //clear intr
                DISP_REG_SET(DISP_REG_OVL_INTSTA, ~reg_val);     
                MMProfileLogEx(DDP_MMP_Events.OVL_IRQ, MMProfileFlagPulse, reg_val, 0);
                disp_invoke_irq_callbacks(DISP_MODULE_OVL, reg_val);
            break;
            
        case MT6582_DISP_WDMA_IRQ_ID:
                reg_val = DISP_REG_GET(DISP_REG_WDMA_INTSTA);
                if(reg_val&(1<<0))
                {
                    DISP_IRQ("IRQ: WDMA0 frame done! cnt=%d \n", cnt_wdma_underflow++);
                    g_disp_irq.irq_src |= (1<<DISP_MODULE_WDMA0);
                }    
                if(reg_val&(1<<1))
                {
                      DISP_ERR("IRQ: WDMA0 underrun! \n");
                      disp_irq_err |= DDP_IRQ_WDMA_UNDERFLOW;
                      disp_check_clock_tree();
                }  
                //clear intr
                DISP_REG_SET(DISP_REG_WDMA_INTSTA, ~reg_val);           
                MMProfileLogEx(DDP_MMP_Events.WDMA0_IRQ, MMProfileFlagPulse, reg_val, DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE));
                disp_invoke_irq_callbacks(DISP_MODULE_WDMA0, reg_val);
            break;
            

        case MT6582_DISP_RDMA_IRQ_ID:
        		index = 0;
                reg_val = DISP_REG_GET(DISP_REG_RDMA_INT_STATUS + index*DISP_INDEX_OFFSET);
                if(reg_val&(1<<0))
                {
                      DISP_IRQ("IRQ: RDMA reg update done! \n");
                }    
                if(reg_val&(1<<1))
                {
                      DISP_IRQ("IRQ: RDMA frame start! \n");
//	                      if(disp_needWakeUp())
//	                      {
//	                          disp_update_hist();
//	                          disp_wakeup_aal();
//	                      }
                      on_disp_aal_alarm_set();
                }
                if(reg_val&(1<<2))
                {
                      DISP_IRQ("IRQ: RDMA1 frame done! \n");
                      g_disp_irq.irq_src |= (1<<DISP_MODULE_RDMA);
                }
                if(reg_val&(1<<3))
                {
                	if (cnt_rdma1_abnormal % 256 == 0) {
                        DISP_ERR("IRQ: RDMA1 abnormal! cnt=%d \n", cnt_rdma1_abnormal++);
                	    disp_check_clock_tree();
                		disp_irq_log_module |= (1<<DISP_MODULE_CONFIG);
                        disp_irq_log_module |= (1<<DISP_MODULE_MUTEX);
                        disp_irq_log_module |= (1<<DISP_MODULE_OVL);
                        disp_irq_log_module |= (1<<DISP_MODULE_RDMA);
                        
                        ddp_dump_info(DISP_MODULE_CONFIG);
                        ddp_dump_info(DISP_MODULE_MUTEX); 
                        ddp_dump_info(DISP_MODULE_OVL); 
                        ddp_dump_info(DISP_MODULE_RDMA);
                        ddp_dump_info(DISP_MODULE_DSI_CMD); 
                        
                        disp_dump_reg(DISP_MODULE_CONFIG);
                        disp_dump_reg(DISP_MODULE_MUTEX);
                        disp_dump_reg(DISP_MODULE_OVL);
                        disp_dump_reg(DISP_MODULE_RDMA);
                       
                	    disp_irq_err |= DDP_IRQ_RDMA_ABNORMAL;
                    }
                	else
                	{
                	    cnt_rdma1_abnormal++;                  	    
                	}
                }
                if(reg_val&(1<<4))
                {
             	    if (cnt_rdma1_underflow % 256 == 0) {
                        DISP_ERR("IRQ: RDMA1 underflow! cnt=%d \n", cnt_rdma1_underflow++);
                	    disp_check_clock_tree();
                        disp_irq_log_module |= (1<<DISP_MODULE_RDMA);
                        disp_irq_log_module |= (1<<DISP_MODULE_CONFIG);
                        disp_irq_log_module |= (1<<DISP_MODULE_MUTEX);
                        disp_irq_log_module |= (1<<DISP_MODULE_OVL);
                        
                        ddp_dump_info(DISP_MODULE_RDMA);
                        ddp_dump_info(DISP_MODULE_CONFIG);
                        ddp_dump_info(DISP_MODULE_MUTEX);
                        ddp_dump_info(DISP_MODULE_OVL); 
                        ddp_dump_info(DISP_MODULE_DSI_CMD);
                        
                        disp_dump_reg(DISP_MODULE_CONFIG);
                        disp_dump_reg(DISP_MODULE_MUTEX); 
                        disp_dump_reg(DISP_MODULE_OVL); 
                        disp_dump_reg(DISP_MODULE_RDMA);                                               
                        
                        disp_irq_err |= DDP_IRQ_RDMA_UNDERFLOW;
                	}
                	else
                	{
                	    cnt_rdma1_underflow++; 
                	}

                }
                //clear intr
                DISP_REG_SET(DISP_REG_RDMA_INT_STATUS + index*DISP_INDEX_OFFSET, ~reg_val);
                MMProfileLogEx(DDP_MMP_Events.RDMA1_IRQ, MMProfileFlagPulse, reg_val, 0);
                disp_invoke_irq_callbacks(DISP_MODULE_RDMA, reg_val);
            break;  

        case MT6582_DISP_COLOR_IRQ_ID:
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
                        
        case MT6582_DISP_BLS_IRQ_ID:
            reg_val = DISP_REG_GET(DISP_REG_BLS_INTSTA);

            // read LUMA & MAX(R,G,B) histogram
            if (reg_val & 0x1)
            {
                  disp_update_hist();
                  disp_wakeup_aal();
            }

            //clear intr
            DISP_REG_SET(DISP_REG_BLS_INTSTA, ~reg_val);
            MMProfileLogEx(DDP_MMP_Events.BLS_IRQ, MMProfileFlagPulse, reg_val, 0);
            break;

        case MT6582_DISP_MUTEX_IRQ_ID:  // can not do reg update done status after release mutex(for ECO requirement), 
                                        // so we have to check update timeout intr here
            reg_val = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & 0x0FF0F;
            
            if(reg_val & 0x0FF00) // udpate timeout intr triggered
            {
                unsigned int reg = 0;
                unsigned int mutexID = 0;
                
                for(mutexID=0;mutexID<4;mutexID++)
                {
                    if((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<(mutexID+8))) == (1<<(mutexID+8)))
                    {
                        if (cnt_mutex_timeout % 256 == 0)
                        {
                            DISP_ERR("disp_path_release_mutex() timeout! \n");
                            disp_irq_log_module |= (1<<DISP_MODULE_CONFIG);
                            disp_irq_log_module |= (1<<DISP_MODULE_MUTEX);
                            ddp_dump_info(DISP_MODULE_CONFIG);
                            ddp_dump_info(DISP_MODULE_MUTEX);
                            reg = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
                            //print error engine
                            if(reg!=0)
                            {
                                if(reg&(1<<3)) {
                                	DISP_ERR(" OVL update reg timeout! cnt=%d\n", cnt_mutex_timeout);
                                	disp_irq_log_module |= (1<<DISP_MODULE_OVL);
                                	disp_dump_reg(DISP_MODULE_OVL);
                                }
                                if(reg&(1<<7)) {
                                	DISP_ERR(" COLOR update reg timeout! cnt=%d\n", cnt_mutex_timeout);
                                	disp_irq_log_module |= (1<<DISP_MODULE_COLOR);
                                	disp_dump_reg(DISP_MODULE_COLOR);
                                }
                                if(reg&(1<<6)) {
                                	DISP_ERR(" WDMA0 update reg timeout! cnt=%d\n", cnt_mutex_timeout);
                                	disp_irq_log_module |= (1<<DISP_MODULE_WDMA0);
                                	disp_dump_reg(DISP_MODULE_WDMA0);
                                }
                                if(reg&(1<<10)) {
                                	DISP_ERR(" RDMA1 update reg timeout! cnt=%d\n", cnt_mutex_timeout);
                                	disp_irq_log_module |= (1<<DISP_MODULE_RDMA);
                                	disp_dump_reg(DISP_MODULE_RDMA);
                                }
                                if(reg&(1<<9)) {
                                	DISP_ERR(" BLS update reg timeout! cnt=%d\n", cnt_mutex_timeout);
                                	disp_irq_log_module |= (1<<DISP_MODULE_BLS);
                                	disp_dump_reg(DISP_MODULE_BLS);
                                }
                            }
                        }
                        cnt_mutex_timeout++;
                        disp_check_clock_tree();
                        //reset mutex
                        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(mutexID), 1);
                        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(mutexID), 0);
                        DISP_MSG("mutex reset done! \n");
                    }
                 }
            }            
            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, ~reg_val);      
            disp_invoke_irq_callbacks(DISP_MODULE_MUTEX, reg_val);
            break;

            
        case MT6582_DISP_CMDQ_IRQ_ID:
            
            reg_val = DISP_REG_GET(DISP_REG_CMDQ_IRQ_FLAG) & 0x00003fff; 
            for(i = 0; ((0x00003fff != reg_val) && (i < CMDQ_MAX_THREAD_COUNT)); i++)
            {
                 // STATUS bit set to 0 means IRQ asserted
                if (0x0 == (reg_val & (1 << i)))
                {
                    value = DISP_REG_GET(DISP_REG_CMDQ_THRx_IRQ_FLAG(i));
                    if(value & 0x12)
                    {
                        cmdqHandleError(i, (uint32_t)value);
                    }    
                    else if (value & 0x01)
                    {
                        cmdqHandleDone(i, (uint32_t)value);
                    }
                    // mark flag to 1 to denote finished processing
                    // and we can early-exit if no more threads being asserted
                    reg_val |= (1 << i);
                }
                MMProfileLogEx(DDP_MMP_Events.CMDQ_IRQ, MMProfileFlagPulse, reg_val, i);
            }
            break;
#if 0      
        case MT6582_G2D_IRQ_ID:
            reg_val = DISP_REG_GET(DISP_REG_G2D_IRQ);
            if(reg_val&G2D_IRQ_STA_BIT)
            {
				  unsigned long set_val = reg_val & ~(G2D_IRQ_STA_BIT); 
                  DISP_IRQ("IRQ: G2D done! \n");
				  g_disp_irq.irq_src |= (1<<DISP_MODULE_G2D);
				  //clear intr
				  DISP_REG_SET(DISP_REG_G2D_IRQ, set_val);
            }                                   
            
            disp_invoke_irq_callbacks(DISP_MODULE_G2D, reg_val);
            break;			
#endif
        default: DISP_ERR("invalid irq=%d \n ", irq); break;
    }        

    // Wakeup event
    mb();   // Add memory barrier before the other CPU (may) wakeup
    wake_up_interruptible(&g_disp_irq_done_queue);    

    if((disp_irq_log_module!=0) || (disp_irq_err!=0))
    {
        wake_up_interruptible(&disp_irq_log_wq);
        //DISP_MSG("disp_irq_log_wq waked!, %d, %d \n", disp_irq_log_module, disp_irq_err);
    }

             
    return IRQ_HANDLED;
}


static void disp_power_on(DISP_MODULE_ENUM eModule , unsigned int * pu4Record)
{  
    unsigned long flag;
    //unsigned int ret = 0;
    spin_lock_irqsave(&gPowerOperateLock , flag);

#ifdef DDP_82_72_TODO
    if((1 << eModule) & g_u4ClockOnTbl)
    {
        DISP_MSG("DDP power %lu is already enabled\n" , (unsigned long)eModule);
    }
    else
    {
        switch(eModule)
        {

            case DISP_MODULE_WDMA0 :
                enable_clock(MT_CG_DISP0_DISP_WDMA , "DDP_DRV");
                //enable_clock(MT_CG_DISP0_WDMA0_SMI , "DDP_DRV");
            break;
 
            case DISP_MODULE_G2D :
                //enable_clock(MT_CG_DISP0_G2D_ENGINE , "DDP_DRV");
				//enable_clock(MT_CG_DISP0_G2D_SMI , "DDP_DRV");
            break;
            default :
                DISP_ERR("disp_power_on:unknown module:%d\n" , eModule);
                ret = -1;
            break;
        }

        if(0 == ret)
        {
            if(0 == g_u4ClockOnTbl)
            {
                enable_clock(MT_CG_DISP0_SMI_LARB0 , "DDP_DRV");
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
    //unsigned int ret = 0;
    spin_lock_irqsave(&gPowerOperateLock , flag);
    
#ifdef DDP_82_72_TODO
//    DISP_MSG("power off : %d\n" , eModule);

    if((1 << eModule) & g_u4ClockOnTbl)
    {
        switch(eModule)
        {
            case DISP_MODULE_WDMA0 :
            	  WDMAStop(0);
            	  WDMAReset(0);
                disable_clock(MT_CG_DISP0_DISP_WDMA , "DDP_DRV");
                //disable_clock(MT_CG_DISP0_WDMA0_SMI , "DDP_DRV");
            break;
            case DISP_MODULE_G2D :
                //disable_clock(MT_CG_DISP0_G2D_ENGINE , "DDP_DRV");
                //disable_clock(MT_CG_DISP0_G2D_SMI , "DDP_DRV");
            break;            
            default :
                DISP_ERR("disp_power_off:unsupported format:%d\n" , eModule);
                ret = -1;
            break;
        }

        if(0 == ret)
        {
            g_u4ClockOnTbl &= (~(1 << eModule));
            *pu4Record &= (~(1 << eModule));

            if(0 == g_u4ClockOnTbl)
            {
                disable_clock(MT_CG_DISP0_SMI_LARB0 , "DDP_DRV");
            }

        }
    }
    else
    {
        DISP_MSG("DDP power %lu is already disabled\n" , (unsigned long)eModule);
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

extern int g_AAL_NewFrameUpdate;

int ConfAALFunc(int i4IsNewFrame)
{
    /*
     * [ALPS01197868]
     * When phone resume, if AALService did not calculate the Y curve yet
     * but the screen refresh is triggered, this function will be called
     * while the Y curve/backlight may be not valid.
     * We should enable the Y curve only if the valid Y curve/backlight
     * is re-calculated, i.e., the BLS update flag is set.
     */
    if (i4IsNewFrame)
        g_AAL_NewFrameUpdate = 1;
     
    if ((1 << DISP_MODULE_BLS) & u4UpdateFlag) {
        disp_onConfig_aal(i4IsNewFrame);
    }
    
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
    return (((1 << DISP_MODULE_COLOR) & u4UpdateFlag) || bls_gamma_dirty) ? 1 : 0;
}

int ConfColorFunc(int i4NotUsed)
{
    DISP_MSG("ConfColorFunc: BLS_EN=0x%x, bls_gamma_dirty=%d\n", DISP_REG_GET(DISP_REG_BLS_EN), bls_gamma_dirty);
    if(bls_gamma_dirty != 0)
    {
        // disable BLS
        if (DISP_REG_GET(DISP_REG_BLS_EN) & 0x1)
        {
            DISP_MSG("ConfColorFunc: Disable BLS\n");
            DISP_REG_SET(DISP_REG_BLS_EN, 0x00010000);
        }
    } else {
        // enable BLS
        DISP_REG_SET(DISP_REG_BLS_EN, 0x00010001);
    }

    if ((1 << DISP_MODULE_COLOR) & u4UpdateFlag)
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
        disp_set_needupdate(DISP_MODULE_COLOR , 0);
    }
    DISP_MSG("ConfColorFunc done: BLS_EN=0x%x, bls_gamma_dirty=%d\n", DISP_REG_GET(DISP_REG_BLS_EN), bls_gamma_dirty);
    return 0;
}

static int COLOR_init = 0;
int disp_color_set_pq_param(void* arg)
{
    DISP_PQ_PARAM * pq_param;
    
    if (COLOR_init == 0)
    {
        DISP_RegisterExTriggerSource(CheckColorUpdateFunc, ConfColorFunc);
        COLOR_init = 1;
    }

    GetUpdateMutex();

    pq_param = get_Color_config();
    if(copy_from_user(pq_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
    {
        printk("disp driver : DISP_IOCTL_SET_PQPARAM Copy from user failed\n");
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
    DISP_READ_REG_TABLE rTableParams;
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
//    unsigned int status;
    unsigned long flags;
    int count;

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
        DISP_MSG("inAddr=0x%x \n", inAddr);
    }
    if(outAddr==0)
    {
        outAddr = kmalloc(800*480*4, GFP_KERNEL);
        memset((void*)outAddr, 0xff, 800*480*4);
        DISP_MSG("outAddr=0x%x \n", outAddr);
    }
#endif
    DISP_DBG("cmd=0x%x, arg=0x%x \n", cmd, (unsigned int)arg);
    switch(cmd)
    {   
        case DISP_IOCTL_WRITE_REG:
            
            if(copy_from_user(&wParams, (void *)arg, sizeof(DISP_WRITE_REG )))
            {
                DISP_ERR("DISP_IOCTL_WRITE_REG, copy_from_user failed\n");
                return -EFAULT;
            }

            DISP_DBG("write  0x%x = 0x%x (0x%x)\n", wParams.reg, wParams.val, wParams.mask);
            if(wParams.reg>DISPSYS_REG_ADDR_MAX || wParams.reg<DISPSYS_REG_ADDR_MIN)
            {
                DISP_ERR("reg write, addr invalid, addr min=0x%x, max=0x%x, addr=0x%x \n", 
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
                DISP_ERR("DISP_IOCTL_READ_REG, copy_from_user failed\n");
                return -EFAULT;
            }
            if(rParams.reg>DISPSYS_REG_ADDR_MAX || rParams.reg<DISPSYS_REG_ADDR_MIN)
            {
                DISP_ERR("reg read, addr invalid, addr min=0x%x, max=0x%x, addr=0x%x \n", 
                    DISPSYS_REG_ADDR_MIN, 
                    DISPSYS_REG_ADDR_MAX, 
                    rParams.reg);
                return -EFAULT;
            }

            value = (*(volatile unsigned int*)rParams.reg) & rParams.mask;

            DISP_DBG("read 0x%x = 0x%x (0x%x)\n", rParams.reg, value, rParams.mask);
            
            if(copy_to_user(rParams.val, &value, sizeof(unsigned int)))
            {
                DISP_ERR("DISP_IOCTL_READ_REG, copy_to_user failed\n");
                return -EFAULT;            
            }
            break;

        case DISP_IOCTL_READ_REG_TABLE:
            if(copy_from_user(&rTableParams, (void *)arg, sizeof(DISP_READ_REG_TABLE)))
            {
                DISP_ERR("DISP_IOCTL_READ_REG_TABLE, copy_from_user failed\n");
                return -EFAULT;
            }

            for (count=0; count<rTableParams.count; count++)
            {
                if(rTableParams.reg[count]>DISPSYS_REG_ADDR_MAX || rTableParams.reg[count]<DISPSYS_REG_ADDR_MIN)
                {
                    DISP_ERR("reg read, addr invalid, addr min=0x%x, max=0x%x, addr=0x%x \n",
                        DISPSYS_REG_ADDR_MIN,
                        DISPSYS_REG_ADDR_MAX,
                        rTableParams.reg[count]);
                    continue;
                }

                rTableParams.val[count] = (*(volatile unsigned int*)rTableParams.reg[count]) & rTableParams.mask[count];
            }

            break;

        case DISP_IOCTL_WAIT_IRQ:
            if(copy_from_user(&wait_irq_struct, (void*)arg , sizeof(wait_irq_struct)))
            {
                DISP_ERR("DISP_IOCTL_WAIT_IRQ, copy_from_user failed\n");
                return -EFAULT;
            }  
            ret = disp_wait_intr(wait_irq_struct.module, wait_irq_struct.timeout_ms);            
            break;  

        case DISP_IOCTL_DUMP_REG:
            if(copy_from_user(&module, (void*)arg , sizeof(module)))
            {
                DISP_ERR("DISP_IOCTL_DUMP_REG, copy_from_user failed\n");
                return -EFAULT;
            }  
            ret = disp_dump_reg(module);            
            ddp_dump_info(module);
            break;  

        case DISP_IOCTL_LOCK_THREAD:
            printk("DISP_IOCTL_LOCK_THREAD! \n");
            value = disp_lock_cmdq_thread();  
            if (copy_to_user((void*)arg, &value , sizeof(unsigned int)))
            {
                DISP_ERR("DISP_IOCTL_LOCK_THREAD, copy_to_user failed\n");
                return -EFAULT;
            }
            break; 
            
        case DISP_IOCTL_UNLOCK_THREAD:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DISP_ERR("DISP_IOCTL_UNLOCK_THREAD, copy_from_user failed\n");
                    return -EFAULT;
            }  
            ret = disp_unlock_cmdq_thread(value);  
            break;

        case DISP_IOCTL_MARK_CMQ:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DISP_ERR("DISP_IOCTL_MARK_CMQ, copy_from_user failed\n");
                    return -EFAULT;
            }
            if(value >= CMDQ_THREAD_NUM) return -EFAULT;
//            cmq_status[value] = 1;
            break;
            
        case DISP_IOCTL_WAIT_CMQ:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DISP_ERR("DISP_IOCTL_WAIT_CMQ, copy_from_user failed\n");
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
                DISP_ERR("DISP_IOCTL_LOCK_RESOURCE, copy_from_user failed\n");
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
                    DISP_ERR("DISP_IOCTL_LOCK_RESOURCE, mutex_id 0x%x failed\n",gLockedResource);
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
                DISP_ERR("DISP_IOCTL_LOCK_RESOURCE, mutex_id = -1 failed\n");
                return -EFAULT;
            }
            break;

            
        case DISP_IOCTL_UNLOCK_RESOURCE:
            if(copy_from_user(&mutex_id, (void*)arg , sizeof(int)))
            {
                DISP_ERR("DISP_IOCTL_UNLOCK_RESOURCE, copy_from_user failed\n");
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
                DISP_ERR("DISP_IOCTL_UNLOCK_RESOURCE, mutex_id = -1 failed\n");
                return -EFAULT;
            }            
            break;

        case DISP_IOCTL_SYNC_REG:
            mb();
            break;

        case DISP_IOCTL_SET_INTR:
            DISP_DBG("DISP_IOCTL_SET_INTR! \n");
            if(copy_from_user(&value, (void*)arg , sizeof(int)))
            {
                DISP_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed\n");
                return -EFAULT;
            }  

            // enable intr
            if( (value&0xffff0000) !=0)
            {
                disable_irq(value&0xff);
                printk("disable_irq %d \n", value&0xff);
            }
            else
            {
                DISP_REGISTER_IRQ(value&0xff);
                printk("enable irq: %d \n", value&0xff);
            }            
            break; 

        case DISP_IOCTL_RUN_DPF:
            DISP_DBG("DISP_IOCTL_RUN_DPF! \n");
            if(copy_from_user(&value, (void*)arg , sizeof(int)))
            {
                DISP_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed, %d\n", ret);
                return -EFAULT;
            }
            
            spin_lock(&gOvlLock);

            disp_run_dp_framework = value;
    
            spin_unlock(&gOvlLock);

            if(value == 1)
            {
                while(disp_get_mutex_status() != 0)
                {
                    DISP_ERR("disp driver : wait fb release hw mutex\n");
                    msleep(3);
                }
            }
            break;

        case DISP_IOCTL_CHECK_OVL:
            DISP_DBG("DISP_IOCTL_CHECK_OVL! \n");
            value = disp_layer_enable;
            
            if(copy_to_user((void *)arg, &value, sizeof(int)))
            {
                DISP_ERR("disp driver : Copy to user error (result)\n");
                return -EFAULT;            
            }
            break;

        case DISP_IOCTL_GET_OVL:
            DISP_DBG("DISP_IOCTL_GET_OVL! \n");
            if(copy_from_user(&ovl_info, (void*)arg , sizeof(DISP_OVL_INFO)))
            {
                DISP_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed, %d\n", ret);
                return -EFAULT;
            } 

            layer = ovl_info.layer;
            
            spin_lock(&gOvlLock);
            ovl_info = disp_layer_info[layer];
            spin_unlock(&gOvlLock);
            
            if(copy_to_user((void *)arg, &ovl_info, sizeof(DISP_OVL_INFO)))
            {
                DISP_ERR("disp driver : Copy to user error (result)\n");
                return -EFAULT;            
            }
            
            break;

        case DISP_IOCTL_AAL_EVENTCTL:
#if !defined(MTK_AAL_SUPPORT)
            printk("Invalid operation DISP_IOCTL_AAL_EVENTCTL since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
            if(copy_from_user(&value, (void *)arg, sizeof(int)))
            {
                printk("disp driver : DISP_IOCTL_AAL_EVENTCTL Copy from user failed\n");
                return -EFAULT;            
            }
            disp_set_aal_alarm(value);
            disp_set_needupdate(DISP_MODULE_BLS , 1);
            ret = 0;
#endif
            break;

        case DISP_IOCTL_GET_AALSTATISTICS:
#if !defined(MTK_AAL_SUPPORT)
            printk("Invalid operation DISP_IOCTL_GET_AALSTATISTICS since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
            // 1. Wait till new interrupt comes
            if(disp_wait_hist_update(60))
            {
                printk("disp driver : DISP_IOCTL_GET_AALSTATISTICS wait time out\n");
                return -EFAULT;
            }

            // 2. read out color engine histogram
            disp_set_hist_readlock(1);
            if(copy_to_user((void*)arg, (void *)(disp_get_hist_ptr()) , sizeof(DISP_AAL_STATISTICS)))
            {
                printk("disp driver : DISP_IOCTL_GET_AALSTATISTICS Copy to user failed\n");
                return -EFAULT;
            }
            disp_set_hist_readlock(0);
            ret = 0;
#endif
            break;

        case DISP_IOCTL_SET_AALPARAM:
#if !defined(MTK_AAL_SUPPORT)
            printk("Invalid operation : DISP_IOCTL_SET_AALPARAM since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
//            disp_set_needupdate(DISP_MODULE_BLS , 0);

            disp_aal_lock();

            aal_param = get_aal_config();

            if(copy_from_user(aal_param , (void *)arg, sizeof(DISP_AAL_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_SET_AALPARAM Copy from user failed\n");
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
                printk("disp driver : DISP_IOCTL_SET_PQINDEX Copy from user failed\n");
                return -EFAULT;
            }

            break;    
            
        case DISP_IOCTL_GET_PQPARAM:
            // this is duplicated to cmdq_proc_unlocked_ioctl
            // be careful when modify the definition
            pq_param = get_Color_config();
            if(copy_to_user((void *)arg, pq_param, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_GET_PQPARAM Copy to user failed\n");
                return -EFAULT;            
            }

            break;

        case DISP_IOCTL_SET_TDSHPINDEX:
            // this is duplicated to cmdq_proc_unlocked_ioctl
            // be careful when modify the definition
            tdshp_index = get_TDSHP_index();
            if(copy_from_user(tdshp_index, (void *)arg, sizeof(DISPLAY_TDSHP_T)))
            {
                printk("disp driver : DISP_IOCTL_SET_TDSHPINDEX Copy from user failed\n");
                return -EFAULT;
            }
        
            break;           
        
        case DISP_IOCTL_GET_TDSHPINDEX:
            
                tdshp_index = get_TDSHP_index();
                if(copy_to_user((void *)arg, tdshp_index, sizeof(DISPLAY_TDSHP_T)))
                {
                    printk("disp driver : DISP_IOCTL_GET_TDSHPINDEX Copy to user failed\n");
                    return -EFAULT;            
                }
        
                break;       
        
         case DISP_IOCTL_SET_GAMMALUT:
        
            DISP_MSG("DISP_IOCTL_SET_GAMMALUT\n");
            
            gamma_index = get_gamma_index();
            if(copy_from_user(gamma_index, (void *)arg, sizeof(DISPLAY_GAMMA_T)))
            {
                printk("disp driver : DISP_IOCTL_SET_GAMMALUT Copy from user failed\n");
                return -EFAULT;
            }

            // disable BLS and suspend AAL
            GetUpdateMutex();
            bls_gamma_dirty = 1;
            aal_debug_flag = 1;
            ReleaseUpdateMutex();

            count = 0;
            while(DISP_REG_GET(DISP_REG_BLS_EN) & 0x1) {
                msleep(1);
                count++;
                if (count > 1000) {
                    DISP_ERR("fail to disable BLS (0x%x)\n", DISP_REG_GET(DISP_REG_BLS_EN));
                    break;
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

         case DISP_IOCTL_SET_CLKON:
            if(copy_from_user(&module, (void *)arg, sizeof(DISP_MODULE_ENUM)))
            {
                printk("disp driver : DISP_IOCTL_SET_CLKON Copy from user failed\n");
                return -EFAULT;            
            }

            disp_power_on(module , &(pNode->u4Clock));
            break;

        case DISP_IOCTL_SET_CLKOFF:
            if(copy_from_user(&module, (void *)arg, sizeof(DISP_MODULE_ENUM)))
            {
                printk("disp driver : DISP_IOCTL_SET_CLKOFF Copy from user failed\n");
                return -EFAULT;            
            }

            disp_power_off(module , &(pNode->u4Clock));
            break;

        case DISP_IOCTL_MUTEX_CONTROL:
            if(copy_from_user(&value, (void *)arg, sizeof(int)))
            {
                printk("disp driver : DISP_IOCTL_MUTEX_CONTROL Copy from user failed\n");
                return -EFAULT;            
            }

            DISP_MSG("DISP_IOCTL_MUTEX_CONTROL: %d, BLS_EN = %d\n", value, DISP_REG_GET(DISP_REG_BLS_EN));

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
                        DISP_ERR("fail to disable BLS (0x%x)\n", DISP_REG_GET(DISP_REG_BLS_EN));
                        break;
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
                printk("disp driver : DISP_IOCTL_MUTEX_CONTROL invalid control\n");
                return -EFAULT;            
            }

            DISP_MSG("DISP_IOCTL_MUTEX_CONTROL done: %d, BLS_EN = %d\n", value, DISP_REG_GET(DISP_REG_BLS_EN));
            
            break;    
            
        case DISP_IOCTL_GET_LCMINDEX:
            
                lcmindex = DISP_GetLCMIndex();
                if(copy_to_user((void *)arg, &lcmindex, sizeof(unsigned long)))
                {
                    printk("disp driver : DISP_IOCTL_GET_LCMINDEX Copy to user failed\n");
                    return -EFAULT;            
                }

                break;       

            break;        
            
        case DISP_IOCTL_SET_PQ_CAM_PARAM:

            pq_cam_param = get_Color_Cam_config();
            if(copy_from_user(pq_cam_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_SET_PQ_CAM_PARAM Copy from user failed\n");
                return -EFAULT;            
            }

            break;
            
        case DISP_IOCTL_GET_PQ_CAM_PARAM:
            
            pq_cam_param = get_Color_Cam_config();
            if(copy_to_user((void *)arg, pq_cam_param, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_GET_PQ_CAM_PARAM Copy to user failed\n");
                return -EFAULT;            
            }
            
            break;        
            
        case DISP_IOCTL_SET_PQ_GAL_PARAM:

            pq_gal_param = get_Color_Gal_config();
            if(copy_from_user(pq_gal_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_SET_PQ_GAL_PARAM Copy from user failed\n");
                return -EFAULT;            
            }
            
            break;

        case DISP_IOCTL_GET_PQ_GAL_PARAM:
            
            pq_gal_param = get_Color_Gal_config();
            if(copy_to_user((void *)arg, pq_gal_param, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_GET_PQ_GAL_PARAM Copy to user failed\n");
                return -EFAULT;            
            }
            
            break;          

        case DISP_IOCTL_TEST_PATH:
#ifdef DDP_DBG_DDP_PATH_CONFIG
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DISP_ERR("DISP_IOCTL_MARK_CMQ, copy_from_user failed\n");
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
            DISP_MSG("value=%d \n", value);
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
                config.srcModule = DISP_MODULE_RDMA;
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
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT) && defined(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)
        case DISP_IOCTL_SET_TPLAY_HANDLE:
            if(copy_from_user(&value, (void*)arg , sizeof(unsigned int)))
            {
                DISP_ERR("DISP_IOCTL_SET_TPLAY_HANDLE, copy_from_user failed\n");
                return -EFAULT;
            }
            set_tplay_handle_addr_request();
            write_tplay_handle(value);
            break;
#endif
        default :
            DISP_ERR("Ddp drv dose not have such command : %d\n" , cmd);
            break; 
    }
    
    return ret;
}

static int disp_open(struct inode *inode, struct file *file)
{
    disp_node_struct *pNode = NULL;

    DISP_DBG("enter disp_open() process:%s\n",current->comm);

    //Allocate and initialize private data
    file->private_data = kmalloc(sizeof(disp_node_struct) , GFP_ATOMIC);
    if(NULL == file->private_data)
    {
        DISP_MSG("Not enough entry for DDP open operation\n");
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
    DISP_DBG("enter disp_release() process:%s\n",current->comm);
    
    pNode = (disp_node_struct *)file->private_data;

    spin_lock(&pNode->node_lock);

    if(pNode->u4LockedResource)
    {
        DISP_ERR("Proccess terminated[REsource] ! :%s , resource:%d\n" 
            , current->comm , pNode->u4LockedResource);
        spin_lock(&gResourceLock);
        gLockedResource = 0;
        spin_unlock(&gResourceLock);
    }

    if(pNode->u4Clock)
    {
        DISP_ERR("Process safely terminated [Clock] !:%s , clock:%u\n" 
            , current->comm , pNode->u4Clock);

        for(index  = 0 ; index < DISP_MODULE_MAX; index += 1)
        {
            if((1 << index) & pNode->u4Clock)
            {
                disp_power_off((DISP_MODULE_ENUM)index , &pNode->u4Clock);
            }
        }
    }

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

    a_pstVMArea->vm_page_prot = pgprot_noncached(a_pstVMArea->vm_page_prot);
    if(remap_pfn_range(a_pstVMArea , 
                 a_pstVMArea->vm_start , 
                 a_pstVMArea->vm_pgoff , 
                 (a_pstVMArea->vm_end - a_pstVMArea->vm_start) , 
                 a_pstVMArea->vm_page_prot))
    {
        DISP_MSG("MMAP failed!!\n");
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
    
    DISP_MSG("\ndisp driver probe...\n\n");
	ret = alloc_chrdev_region(&disp_devno, 0, 1, DISP_DEVNAME);

	if(ret)
	{
	    DISP_ERR("Error: Can't Get Major number for DISP Device\n");
	}
	else
	{
	    DISP_MSG("Get DISP Device Major number (%d)\n", disp_devno);
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

        // enable CMDQ interrupt
        DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(i),0x13); //SL TEST CMDQ time out
    }
        
    // Register IRQ                      
    DISP_REGISTER_IRQ(MT6582_DISP_COLOR_IRQ_ID);
    DISP_REGISTER_IRQ(MT6582_DISP_BLS_IRQ_ID);
    DISP_REGISTER_IRQ(MT6582_DISP_OVL_IRQ_ID);
    DISP_REGISTER_IRQ(MT6582_DISP_WDMA_IRQ_ID);
    DISP_REGISTER_IRQ(MT6582_DISP_RDMA_IRQ_ID);
    DISP_REGISTER_IRQ(MT6582_DISP_CMDQ_IRQ_ID);
    DISP_REGISTER_IRQ(MT6582_DISP_CMDQ_SECURE_IRQ_ID);
    DISP_REGISTER_IRQ(MT6582_DISP_MUTEX_IRQ_ID);
    //DISP_REGISTER_IRQ(MT6582_G2D_IRQ_ID);
    
    init_waitqueue_head(&disp_irq_log_wq);
    disp_irq_log_task = kthread_create(disp_irq_log_kthread_func, NULL, "disp_config_update_kthread");
    if (IS_ERR(disp_irq_log_task)) 
    {
        DISP_ERR("DISP_InitVSYNC(): Cannot create disp_irq_log_task kthread\n");
    }
    wake_up_process(disp_irq_log_task);
   
    // init error log timer
    init_timer(&disp_irq_err_timer);
    disp_irq_err_timer.expires = jiffies + 5*HZ;
    disp_irq_err_timer.function = disp_irq_err_timer_handler;    
    add_timer(&disp_irq_err_timer);
                
	DISP_MSG("DISP Probe Done\n");
	NOT_REFERENCED(class_dev);
	return 0;
}

static int disp_remove(struct platform_device *pdev)
{
    disable_irq(MT6582_DISP_OVL_IRQ_ID);
    disable_irq(MT6582_DISP_WDMA_IRQ_ID);
    disable_irq(MT6582_DISP_RDMA_IRQ_ID);
    disable_irq(MT6582_DISP_CMDQ_IRQ_ID);
    disable_irq(MT6582_DISP_CMDQ_SECURE_IRQ_ID);
    //disable_irq(MT6582_DISP_COLOR_IRQ_ID);
    disable_irq(MT6582_DISP_BLS_IRQ_ID);
    //disable_irq(MT6582_G2D_IRQ_ID);
    return 0;
}

static void disp_shutdown(struct platform_device *pdev)
{
	/* Nothing yet */
}


/* PM suspend */
static int disp_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    printk("\n\n==== DISP suspend is called ====\n");

    return cmdqSuspendTask();
}

/* PM resume */
static int disp_resume(struct platform_device *pdev)
{
    return cmdqResumeTask();
}

#if 0
int disp_pm_restore_noirq(struct device *device)
{
    return 0; 
}

struct dev_pm_ops disp_pm_ops =
{
    .suspend       = disp_suspend,
    .resume        = disp_resume,
    .freeze        = disp_suspend,
    .thaw          = disp_resume,
    .poweroff      = disp_suspend,
    .restore       = disp_resume,
    .restore_noirq = disp_pm_restore_noirq,
};
#endif // 0


static struct platform_driver disp_driver =
{
	.probe		= disp_probe,
	.remove		= disp_remove,
	.shutdown	= disp_shutdown,
	.suspend	= disp_suspend,
	.resume		= disp_resume,
	.driver     =
	{
	    .name = DISP_DEVNAME,
	    //.pm   = &disp_pm_ops,
	},
};


#if 0
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
#endif // 0

static int __init disp_init(void)
{
    int ret;

    spin_lock_init(&gCmdqLock);
    spin_lock_init(&gResourceLock);
    spin_lock_init(&gOvlLock);
    spin_lock_init(&gRegisterUpdateLock);
    spin_lock_init(&gPowerOperateLock);
    spin_lock_init(&g_disp_irq.irq_lock);

#if 0
    DISP_MSG("Register disp device\n");
	if(platform_device_register(&disp_device))
	{
        DISP_ERR("failed to register disp device\n");
        ret = -ENODEV;
        return ret;
	}
#endif // 0

    DISP_MSG("Register the disp driver\n");    
    if(platform_driver_register(&disp_driver))
    {
        DISP_ERR("failed to register disp driver\n");
        //platform_device_unregister(&disp_device);
        ret = -ENODEV;
        return ret;
    }

    ddp_debug_init();

    pRegBackup = kmalloc(DDP_BACKUP_REG_NUM*sizeof(int), GFP_KERNEL);
    ASSERT(pRegBackup!=NULL);
    *pRegBackup = DDP_UNBACKED_REG_MEM;

    cmdqInitialize();

#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT) && (CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)
// secure video path implementation: a physical address is allocated to place a handle for decryption buffer.
    if (0 != init_tplay_handle()) // non-zero value for valid VA
    {
        ;
    }
#endif

    return 0;
}

static void __exit disp_exit(void)
{
    cmdqDeInitialize();

    cdev_del(disp_cdev);
    unregister_chrdev_region(disp_devno, 1);
	
    platform_driver_unregister(&disp_driver);
    //platform_device_unregister(&disp_device);
	
    device_destroy(disp_class, disp_devno);
    class_destroy(disp_class);

    ddp_debug_exit();

    DISP_MSG("Done\n");
}


static int disp_get_mutex_status()
{
    return disp_mutex_status;
}

unsigned int g_reg_cfg[50];
unsigned int g_reg_mtx[50];
unsigned int g_reg_ovl[100];
unsigned int g_reg_clr[20];
unsigned int g_reg_bls[50];
unsigned int g_reg_wdma[50];
unsigned int g_reg_rdma[50];
int disp_dump_reg(DISP_MODULE_ENUM module)
{
	unsigned int i = 0;
	unsigned int index;
	switch(module)
	{
	case DISP_MODULE_OVL:
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_STA);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_INTEN);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_INTSTA);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_EN);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_TRIG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RST);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_ROI_SIZE);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_DATAPATH_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_ROI_BGCLR);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_SRC_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L0_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L0_SRCKEY);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L0_SRC_SIZE);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L0_OFFSET);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L0_ADDR);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L0_PITCH);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA0_CTRL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_START_TRIG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_SLOW_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA0_FIFO_CTRL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L1_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L1_SRCKEY);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L1_SRC_SIZE);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L1_OFFSET);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L1_ADDR);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L1_PITCH);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA1_CTRL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_START_TRIG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_SLOW_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA1_FIFO_CTRL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L2_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L2_SRCKEY);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L2_SRC_SIZE);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L2_OFFSET);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L2_ADDR);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L2_PITCH);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA2_CTRL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_START_TRIG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_SLOW_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA2_FIFO_CTRL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L3_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L3_SRCKEY);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L3_SRC_SIZE);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L3_OFFSET);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L3_ADDR);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_L3_PITCH);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA3_CTRL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_START_TRIG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_SLOW_CON);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA3_FIFO_CTRL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_DEBUG_MON_SEL);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_OUTMUX_DBG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA0_DBG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA1_DBG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA2_DBG);
		g_reg_ovl[i++] = DISP_REG_GET(DISP_REG_OVL_RDMA3_DBG);
		break;

	case DISP_MODULE_COLOR:
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_START);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_INTEN);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_INTSTA);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_OUT_SEL);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_FRAME_DONE_DEL);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_CRC);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_SW_SCRATCH);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_RDY_SEL);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_RDY_SEL_EN);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_CK_ON);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_INTERNAL_IP_WIDTH);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_INTERNAL_IP_HEIGHT);
		g_reg_clr[i++] = DISP_REG_GET(DISP_REG_COLOR_CM1_EN);
		break;

	case DISP_MODULE_BLS:
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_EN);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_RST);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_INTEN);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_INTSTA);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_BLS_SETTING);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_FANA_SETTING);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_SRC_SIZE);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_GAIN_SETTING);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_MANUAL_GAIN);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_MANUAL_MAXCLR);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_GAMMA_SETTING);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_GAMMA_BOUNDARY);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_LUT_UPDATE);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_MAXCLR_THD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_DISTPT_THD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_MAXCLR_LIMIT);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_DISTPT_LIMIT);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_AVE_SETTING);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_AVE_LIMIT);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_DISTPT_SETTING);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_HIS_CLEAR);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_SC_DIFF_THD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_SC_BIN_THD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_MAXCLR_GRADUAL);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_DISTPT_GRADUAL);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_FAST_IIR_XCOEFF);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_FAST_IIR_YCOEFF);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_XCOEFF);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_YCOEFF);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_PWM_DUTY);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_PWM_GRADUAL);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_PWM_CON);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_PWM_MANUAL);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_DEBUG);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_PATTERN);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_CHKSUM);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_PWM_DUTY_RD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_FRAME_AVE_RD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_MAXCLR_RD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_DISTPT_RD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_GAIN_RD);
		g_reg_bls[i++] = DISP_REG_GET(DISP_REG_BLS_SC_RD);

		break;

	case DISP_MODULE_WDMA0:
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_INTEN);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_INTSTA);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_EN);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_RST);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_SMI_CON);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_CFG);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_CLIP_COORD);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_DST_ADDR);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_DST_W_IN_BYTE);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_ALPHA);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_BUF_ADDR);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_STA);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_BUF_CON1);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_BUF_CON2);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_PRE_ADD0);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_PRE_ADD2);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_POST_ADD0);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_POST_ADD2);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_DST_U_ADDR);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_DST_V_ADDR);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_DST_UV_PITCH);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_DITHER_CON);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_FLOW_CTRL_DBG);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_EXEC_DBG);
		g_reg_wdma[i++] = DISP_REG_GET(DISP_REG_WDMA_CLIP_DBG);
		break;

	case DISP_MODULE_RDMA:
	case DISP_MODULE_RDMA1:
		if (module == DISP_MODULE_RDMA1) {
			index = 1;
		} else {
			index = 0;
		}
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE 		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_INT_STATUS    		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON			+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1			+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_TARGET_LINE 		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_MEM_CON 			+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH 		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0	+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1 	+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_FIFO_CON			+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD0		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD1		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD2		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD0		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD1		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD2		+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_DUMMY				+ index*DISP_INDEX_OFFSET);
		g_reg_rdma[i++] = DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL		+ index*DISP_INDEX_OFFSET);

		break;

	case DISP_MODULE_DPI0:
		break;

	case DISP_MODULE_DSI_VDO:
	case DISP_MODULE_DSI_CMD:
		break;

	case DISP_MODULE_CONFIG:
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_CAM_MDP_MOUT_EN);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_RDMA_MOUT_EN);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ0_MOUT_EN);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ1_MOUT_EN);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_TDSHP_MOUT_EN);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MOUT_RST);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ0_SEL);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ1_SEL);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_TDSHP_SEL);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_SEL);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MDP_WDMA_SEL);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_DISP_OUT_SEL);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_SET0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CLR0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_SET1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CLR1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_SW_RST_B);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_LCM_RST_B);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_DONE);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_FAIL0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_FAIL1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_HOLDB);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_MODE);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_BSEL0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_BSEL1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_CON);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL0);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL1);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL2);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL3);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DEBUG_OUT_SEL);
		g_reg_cfg[i++] = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DUMMY);
        g_reg_cfg[i++] = DISP_REG_GET(0xf0206040);
        g_reg_cfg[i++] = DISP_REG_GET(DISPSYS_CONFIG_BASE + 0x860);
        g_reg_cfg[i++] = DISP_REG_GET(DISPSYS_CONFIG_BASE + 0x868);
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010000);
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010010);
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010060);
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010064);
        g_reg_cfg[i++] = DISP_REG_GET(0xf401008c);
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010450);  
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010454); 
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010600);
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010604);
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010610);
        g_reg_cfg[i++] = DISP_REG_GET(0xf4010614);
		break;

	case DISP_MODULE_MUTEX:
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTEN);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_REG_UPD_TIMEOUT);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_EN);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX0);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_RST);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_MOD);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_SOF);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_EN);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX1);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_RST);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_MOD);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_SOF);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_EN);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX2);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_RST);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_MOD);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_SOF);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_EN);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX3);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_RST);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_MOD);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_SOF);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_EN);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX4);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_RST);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_MOD);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_SOF);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_EN);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX5);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_RST);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_MOD);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_SOF);
		g_reg_mtx[i++] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_DEBUG_OUT_SEL);
		break;

	default:
		DISP_MSG("error, reg_dump, unknow module=%d \n", module);
		break;
	}

	if (disp_irq_log_module == 0) {
		disp_irq_log_module |= (1<<module);
		wake_up_interruptible(&disp_irq_log_wq);
	}
	return 0;
}

void disp_print_reg(DISP_MODULE_ENUM module)
{
	unsigned int i = 0;
	unsigned int index;
	switch(module)
	{
	case DISP_MODULE_OVL:
		DISP_MSG("===== DISP OVL Reg Dump: ============\n");
		DISP_MSG("(000)O_STA                =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(004)O_INTEN              =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(008)O_INTSTA             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(00C)O_EN                 =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(010)O_TRIG               =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(014)O_RST                =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(020)O_ROI_SIZE           =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(020)O_DATAPATH_CON       =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(028)O_ROI_BGCLR          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(02C)O_SRC_CON            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(030)O_L0_CON             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(034)O_L0_SRCKEY          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(038)O_L0_SRC_SIZE        =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(03C)O_L0_OFFSET          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(040)O_L0_ADDR            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(044)O_L0_PITCH           =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0C0)O_R0_CTRL            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0C4)O_R0_MEM_START_TRIG  =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0C8)O_R0_MEM_GMC_SET     =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0CC)O_R0_MEM_SLOW_CON    =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0D0)O_R0_FIFO_CTRL     	=0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(050)O_L1_CON             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(054)O_L1_SRCKEY          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(058)O_L1_SRC_SIZE        =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(05C)O_L1_OFFSET          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(060)O_L1_ADDR            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(064)O_L1_PITCH           =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0E0)O_R1_CTRL            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0E4)O_R1_MEM_START_TRIG  =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0E8)O_R1_MEM_GMC_SET     =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0EC)O_R1_MEM_SLOW_CON    =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0F0)O_R1_FIFO_CTRL       =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(070)O_L2_CON             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(074)O_L2_SRCKEY          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(078)O_L2_SRC_SIZE        =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(07C)O_L2_OFFSET          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(080)O_L2_ADDR            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(084)O_L2_PITCH           =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(100)O_R2_CTRL            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(104)O_R2_MEM_START_TRIG  =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(108)O_R2_MEM_GMC_SET     =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(10C)O_R2_MEM_SLOW_CON    =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(110)O_R2_FIFO_CTRL       =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(090)O_L3_CON             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(094)O_L3_SRCKEY          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(098)O_L3_SRC_SIZE        =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(09C)O_L3_OFFSET          =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0A0)O_L3_ADDR            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(0A4)O_L3_PITCH           =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(120)O_R3_CTRL            =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(124)O_R3_MEM_START_TRIG  =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(128)O_R3_MEM_GMC_SET     =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(12C)O_R3_MEM_SLOW_CON    =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(130)O_R3_FIFO_CTRL       =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(1C4)O_DEBUG_MON_SEL      =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(1C4)O_R0_MEM_GMC_SET2    =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(1C8)O_R1_MEM_GMC_SET2    =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(1CC)O_R2_MEM_GMC_SET2    =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(1D0)O_R3_MEM_GMC_SET2    =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(240)O_FLOW_CTRL_DBG      =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(244)O_ADDCON_DBG         =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(248)O_OUTMUX_DBG         =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(24C)O_R0_DBG             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(250)O_R1_DBG             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(254)O_R2_DBG             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("(258)O_R3_DBG             =0x%x \n", g_reg_ovl[i++]);
		DISP_MSG("TOTAL dump %d registers\n", i);
		break;

	case DISP_MODULE_COLOR:
		DISP_MSG("===== DISP Color Reg Dump: ============\n");
		DISP_MSG("(0x0F00)C_START             =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F04)C_INTEN             =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F08)C_INTSTA            =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F0C)C_OUT_SEL           =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F10)C_FRAME_DONE_DEL    =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F14)C_CRC               =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F18)C_SW_SCRATCH        =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F20)C_RDY_SEL           =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F24)C_RDY_SEL_EN        =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F28)C_CK_ON             =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F50)C_INTERNAL_IP_WIDTH =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F54)C_INTERNAL_IP_HEIGHT=0x%x \n", g_reg_clr[i++]);
		DISP_MSG("(0x0F60)C_CM1_EN            =0x%x \n", g_reg_clr[i++]);
		DISP_MSG("TOTAL dump %d registers\n", i);
		break;

	case DISP_MODULE_BLS:
		DISP_MSG("===== DISP BLS Reg Dump: ============\n");
		DISP_MSG("(0x0 )B_EN                =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x4 )B_RST               =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x8 )B_INTEN             =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0xC )B_INTSTA            =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x10)B_BLS_SETTING       =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x14)B_FANA_SETTING      =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x18)B_SRC_SIZE          =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x20)B_GAIN_SETTING      =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x24)B_MANUAL_GAIN       =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x28)B_MANUAL_MAXCLR     =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x30)B_GAMMA_SETTING     =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x34)B_GAMMA_BOUNDARY    =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x38)B_LUT_UPDATE        =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x60)B_MAXCLR_THD        =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x64)B_DISTPT_THD        =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x68)B_MAXCLR_LIMIT      =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x6C)B_DISTPT_LIMIT      =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x70)B_AVE_SETTING       =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x74)B_AVE_LIMIT         =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x78)B_DISTPT_SETTING    =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x7C)B_HIS_CLEAR         =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x80)B_SC_DIFF_THD       =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x84)B_SC_BIN_THD        =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x88)B_MAXCLR_GRADUAL    =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x8C)B_DISTPT_GRADUAL    =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x90)B_FAST_IIR_XCOEFF   =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x94)B_FAST_IIR_YCOEFF   =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x98)B_SLOW_IIR_XCOEFF   =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x9C)B_SLOW_IIR_YCOEFF   =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0xA0)B_PWM_DUTY          =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0xA4)B_PWM_GRADUAL       =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0xA8)B_PWM_CON           =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0xAC)B_PWM_MANUAL        =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0xB0)B_DEBUG             =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0xB4)B_PATTERN           =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0xB8)B_CHKSUM            =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x200)B_PWM_DUTY_RD      =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x204)B_FRAME_AVE_RD     =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x208)B_MAXCLR_RD        =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x20C)B_DISTPT_RD        =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x210)B_GAIN_RD          =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("(0x214)B_SC_RD            =0x%x \n", g_reg_bls[i++]);
		DISP_MSG("TOTAL dump %d registers\n", i);
		break;

	case DISP_MODULE_WDMA0:
		DISP_MSG("===== DISP WDMA0 Reg Dump: ============\n");
		DISP_MSG("(000)W_INTEN          =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(004)W_INTSTA         =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(008)W_EN             =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(00C)W_RST            =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(010)W_SMI_CON        =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(014)W_CFG            =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(018)W_SRC_SIZE       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(01C)W_CLIP_SIZE      =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(020)W_CLIP_COORD     =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(024)W_DST_ADDR       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(028)W_DST_W_IN_BYTE  =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(02C)W_ALPHA          =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(030)W_BUF_ADDR       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(034)W_STA            =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(038)W_BUF_CON1       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(03C)W_BUF_CON2       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(058)W_PRE_ADD0       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(05C)W_PRE_ADD2       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(060)W_POST_ADD0      =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(064)W_POST_ADD2      =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(070)W_DST_U_ADDR     =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(074)W_DST_V_ADDR     =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(078)W_DST_UV_PITCH   =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(090)W_DITHER_CON     =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(0A0)W_FLOW_CTRL_DBG  =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(0A4)W_EXEC_DBG       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("(0A8)W_CLIP_DBG       =0x%x \n", g_reg_wdma[i++]);
		DISP_MSG("TOTAL dump %d registers\n", i);
		break;

	case DISP_MODULE_RDMA:
	case DISP_MODULE_RDMA1:
		if (module == DISP_MODULE_RDMA1) {
			index = 1;
		} else {
			index = 0;
		}
		DISP_MSG("===== DISP RDMA%d Reg Dump: ======== \n", index);
		DISP_MSG("(000)R_INTEN             =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(004)R_INT_STATUS        =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(010)R_GLOBAL_CON        =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(014)R_SIZE_CON_0        =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(018)R_SIZE_CON_1        =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(01C)R_TARGET_LINE       =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(024)R_MEM_CON           =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(028)R_MEM_START_ADDR    =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(02C)R_MEM_SRC_PITCH     =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(030)R_MEM_GMC_SET_0 =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(034)R_MEM_SLOW_CON      =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(030)R_MEM_GMC_SET_1 =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(040)R_FIFO_CON          =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(078)R_CF_PRE_ADD0       =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(07C)R_CF_PRE_ADD1       =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(080)R_CF_PRE_ADD2       =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(084)R_CF_POST_ADD0      =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(088)R_CF_POST_ADD1      =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(08C)R_CF_POST_ADD2      =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(090)R_DUMMY             =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("(094)R_DEBUG_OUT_SEL     =0x%x \n", g_reg_rdma[i++]);
		DISP_MSG("TOTAL dump %d registers\n", i);
		break;

	case DISP_MODULE_DPI0:
		break;

	case DISP_MODULE_DSI_VDO:
	case DISP_MODULE_DSI_CMD:
		break;

	case DISP_MODULE_CONFIG:
		DISP_MSG("===== DISP DISP_REG_MM_CONFIG Reg Dump: ============\n");
		DISP_MSG("(0x01c)CAM_MDP_MOUT_EN   =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x020)MDP_RDMA_MOUT_EN  =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x024)MDP_RSZ0_MOUT_EN  =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x028)MDP_RSZ1_MOUT_EN  =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x02c)MDP_TDSHP_MOUT_EN =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x030)DISP_OVL_MOUT_EN  =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x034)MMSYS_MOUT_RST    =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x038)MDP_RSZ0_SEL      =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x03c)MDP_RSZ1_SEL      =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x040)MDP_TDSHP_SEL     =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x044)MDP_WROT_SEL      =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x048)MDP_WDMA_SEL      =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x04c)DISP_OUT_SEL      =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x100)CG_CON0           =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x104)CG_SET0           =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x108)CG_CLR0           =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x110)CG_CON1           =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x114)CG_SET1           =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x118)CG_CLR1           =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x120)HW_DCM_DIS0       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x124)HW_DCM_DIS_SET0   =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x128)HW_DCM_DIS_CLR0   =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x12c)HW_DCM_DIS1       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x130)HW_DCM_DIS_SET1   =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x134)HW_DCM_DIS_CLR1   =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x138)SW_RST_B          =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x13c)LCM_RST_B         =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x800)MBIST_DONE        =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x804)MBIST_FAIL0       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x808)MBIST_FAIL1       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x80C)MBIST_HOLDB       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x810)MBIST_MODE        =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x814)MBIST_BSEL0       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x818)MBIST_BSEL1       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x81c)MBIST_CON         =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x820)MEM_DELSEL0       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x824)MEM_DELSEL1       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x828)MEM_DELSEL2       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x82c)MEM_DELSEL3       =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x830)DEBUG_OUT_SEL     =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(0x840)DUMMY             =0x%x \n", g_reg_cfg[i++]);
		DISP_MSG("(10206040)CONFIG_CLOCK_DUMMY=0x%x \n",g_reg_cfg[i++]);
		DISP_MSG("(14000860)CONFIG_VALID=0x%x \n",  g_reg_cfg[i++]);
		DISP_MSG("(14000868)CONFIG_READY=0x%x \n",  g_reg_cfg[i++]);
		DISP_MSG("(000)SMI_0=0x%x \n",              g_reg_cfg[i++]);
		DISP_MSG("(010)SMI_10=0x%x \n",             g_reg_cfg[i++]);
		DISP_MSG("(060)SMI_60=0x%x \n",             g_reg_cfg[i++]);
		DISP_MSG("(064)SMI_64=0x%x \n",             g_reg_cfg[i++]);
		DISP_MSG("(08c)SMI_8c=0x%x \n",             g_reg_cfg[i++]);
		DISP_MSG("(450)SMI_450_REQ0=0x%x \n",       g_reg_cfg[i++]);
		DISP_MSG("(454)SMI_454_REQ1=0x%x \n",       g_reg_cfg[i++]);
		DISP_MSG("(600)SMI_600=0x%x \n",            g_reg_cfg[i++]);
		DISP_MSG("(604)SMI_604=0x%x \n",            g_reg_cfg[i++]);
		DISP_MSG("(610)SMI_610=0x%x \n",            g_reg_cfg[i++]);
		DISP_MSG("(614)SMI_614=0x%x \n",            g_reg_cfg[i++]);
		DISP_MSG("TOTAL dump %d registers\n", i);
		break;

	case DISP_MODULE_MUTEX:
		DISP_MSG("===== DISP DISP_REG_MUTEX_CONFIG Reg Dump: ============\n");
		DISP_MSG("(0x0  )MUTEX_INTEN    =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x4  )MUTEX_INTSTA   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x8  )UPD_TIMEOUT =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xC  )COMMIT      =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x20)M0_EN    =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x24)M0       =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x28)M0_RST   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x2C)M0_MOD   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x30)M0_SOF   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x40)M1_EN    =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x44)M1       =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x48)M1_RST   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x4C)M1_MOD   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x50)M1_SOF   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x60)M2_EN    =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x64)M2       =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x68)M2_RST   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x6C)M2_MOD   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x70)M2_SOF   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x80)M3_EN    =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x84)M3       =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x88)M3_RST   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x8C)M3_MOD   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x90)M3_SOF   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xA0)M4_EN    =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xA4)M4       =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xA8)M4_RST   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xAC)M4_MOD   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xB0)M4_SOF   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xC0)M5_EN    =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xC4)M5       =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xC8)M5_RST   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xCC)M5_MOD   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0xD0)M5_SOF   =0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("(0x100)M_DBG_OUT_SEL=0x%x \n", g_reg_mtx[i++]);
		DISP_MSG("TOTAL dump %d registers\n", i);
		break;

	default:
		DISP_MSG("error, reg_dump, unknow module=%d \n", module);
		break;
	}
}


int disp_module_clock_on(DISP_MODULE_ENUM module, char* caller_name)
{
    return 0;
}

int disp_module_clock_off(DISP_MODULE_ENUM module, char* caller_name)
{
    return 0;
}

#define DISP_REG_CLK_CFG0 0xf0000040
#define DISP_REG_CLK_CFG1 0xf0000050
int disp_clock_check(void)
{
    int ret = 0;
    
    // 0:SMI COMMON, 1:SMI LARB0, 3:MUTEX, 4:DISP_COLOR 
    // 5:DISP_BLS, 7:DISP_RDMA, 8:DISP_OVL
    if( DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0) & 0x1bb )
    {
        DISP_ERR("clock error(ddp), CONFIG_CG_CON0=0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
        ret = -1;
    }
    
    // Just for DSI, 0:DSI engine, 1:DSI digital
    if(DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON1) & 0x3)
    {
        DISP_ERR("clock error(dsi), CONFIG_CG_CON1=0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON1));
        ret = -1;
    }
    
    // 31: mm, 7:axi
    if(DISP_REG_GET(DISP_REG_CLK_CFG0) & 0x80000080)
    {
        DISP_ERR("clock error(mm and axi), DISP_REG_CLK_CFG0=0x%x \n", DISP_REG_GET(DISP_REG_CLK_CFG0));
        ret = -1;
    }
    
    // for bls, 7:pwm
    if(DISP_REG_GET(DISP_REG_CLK_CFG1) & 0x80)
    {
        DISP_ERR("clock error(pwm), DISP_REG_CLK_CFG1=0x%x \n", DISP_REG_GET(DISP_REG_CLK_CFG1));
        ret = -1;
    }

    // for mm clock freq (optional)
    {

    }
    
    return ret;
}

module_init(disp_init);
module_exit(disp_exit);
MODULE_AUTHOR("Tzu-Meng, Chung <Tzu-Meng.Chung@mediatek.com>");
MODULE_DESCRIPTION("Display subsystem Driver");
MODULE_LICENSE("GPL");
