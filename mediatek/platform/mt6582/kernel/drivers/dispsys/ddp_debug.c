#include <linux/string.h>
#include <linux/uaccess.h>

#include <linux/debugfs.h>

#include <linux/aee.h>
#include <linux/disp_assert_layer.h>
//#include <linux/unistd.h>
//#include <linux/fcntl.h>

#include <linux/dma-mapping.h>
#include "ddp_debug.h"
#include "ddp_reg.h"
#include "ddp_bls.h"
#include "ddp_color.h"
#include "ddp_drv.h"
//#include "ddp_dpfd.h"
//#include "ddp_rot.h"
//#include "ddp_scl.h"
#include "ddp_wdma.h"
#include "ddp_hal.h"
#include "ddp_path.h"

#include "disp_drv_ddp.h"
//#include "ddp_dpfd.h"
#include <mach/m4u.h>
#include "disp_drv.h"

#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT) && defined(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)
#include "mobicore_driver_api.h"
extern unsigned int tplay_handle_virt_addr;
extern unsigned int init_tplay_handle(void);
extern unsigned int get_tplay_handle();
extern int set_tplay_handle_addr_request(void);

extern enum mc_result late_init_session_tl(void);
extern enum mc_result late_init_session_drv(void);
extern enum mc_result late_open_mobicore_device(void);
#endif

// ---------------------------------------------------------------------------
//  External variable declarations
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//  Debug Options
// ---------------------------------------------------------------------------

struct DDP_MMP_Events_t DDP_MMP_Events;

struct dentry *debugfs = NULL;
unsigned int gUltraLevel = 4; // RDMA ultra aggressive level
unsigned int gEnableUltra = 0;

static const long int DEFAULT_LOG_FPS_WND_SIZE = 30;


unsigned char pq_debug_flag=0;
unsigned char aal_debug_flag=0;


static char STR_HELP[] =
    "\n"
    "USAGE\n"
    "        echo [ACTION]... > dispsys\n"
    "\n"
    "ACTION\n"
    "       regr:addr\n"
    "\n"
    "       regw:addr,value\n"
    "\n"
    "       dbg_log:0|1\n"
    "\n"
    "       irq_log:0|1\n"
    "\n"    
    "       irq_err_log:0|1\n"
    "\n"       
    "       backlight:level\n"
    "\n"
    "       dump_aal:arg\n"
    "\n"
    "       mmp\n"
    "\n"    
    "       dump_reg:moduleID\n"
    "\n"    
    "       dpfd_ut1:channel\n"
    ;

void ddp_enable_bls(int BLS_switch)
{   

    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x00000003);
    
    if(BLS_switch == FALSE)
        DISP_REG_SET(DISP_REG_BLS_EN, 0x00010000);
    else
        DISP_REG_SET(DISP_REG_BLS_EN, 0x00010001);
    
    DISP_MSG("aal_debug_flag changed,aal_debug_flag=%d, set BLS_EN=%x", aal_debug_flag, DISP_REG_GET(DISP_REG_BLS_EN));
    DISP_REG_SET(DISP_REG_BLS_DEBUG, 0x00000000);
    
}

void init_ddp_mmp_events(void)
{
    if (DDP_MMP_Events.DDP == 0)
    {
        DDP_MMP_Events.DDP = MMProfileRegisterEvent(MMP_RootEvent, "DDP");
        DDP_MMP_Events.MutexParent = MMProfileRegisterEvent(DDP_MMP_Events.DDP, "Mutex");
        DDP_MMP_Events.Mutex[0] = MMProfileRegisterEvent(DDP_MMP_Events.MutexParent, "Mutex0");
        DDP_MMP_Events.Mutex[1] = MMProfileRegisterEvent(DDP_MMP_Events.MutexParent, "Mutex1");
        DDP_MMP_Events.Mutex[2] = MMProfileRegisterEvent(DDP_MMP_Events.MutexParent, "Mutex2");
        DDP_MMP_Events.Mutex[3] = MMProfileRegisterEvent(DDP_MMP_Events.MutexParent, "Mutex3");
        DDP_MMP_Events.Mutex[4] = MMProfileRegisterEvent(DDP_MMP_Events.MutexParent, "Mutex4");
        DDP_MMP_Events.Mutex[5] = MMProfileRegisterEvent(DDP_MMP_Events.MutexParent, "Mutex5");
        DDP_MMP_Events.BackupReg = MMProfileRegisterEvent(DDP_MMP_Events.DDP, "BackupReg");
        DDP_MMP_Events.DDP_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP, "DDP_IRQ");
        DDP_MMP_Events.SCL_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "SCL_IRQ");
        DDP_MMP_Events.ROT_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "ROT_IRQ");
        DDP_MMP_Events.OVL_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "OVL_IRQ");
        DDP_MMP_Events.WDMA0_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "WDMA0_IRQ");
        DDP_MMP_Events.WDMA1_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "WDMA1_IRQ");
        DDP_MMP_Events.RDMA0_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "RDMA0_IRQ");
        DDP_MMP_Events.RDMA1_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "RDMA1_IRQ");
        DDP_MMP_Events.COLOR_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "COLOR_IRQ");
        DDP_MMP_Events.BLS_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "BLS_IRQ");
        DDP_MMP_Events.TDSHP_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "TDSHP_IRQ");
        DDP_MMP_Events.CMDQ_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "CMDQ_IRQ");
        DDP_MMP_Events.Mutex_IRQ = MMProfileRegisterEvent(DDP_MMP_Events.DDP_IRQ, "Mutex_IRQ");
        DDP_MMP_Events.WAIT_INTR = MMProfileRegisterEvent(DDP_MMP_Events.DDP, "WAIT_IRQ");
        DDP_MMP_Events.Debug = MMProfileRegisterEvent(DDP_MMP_Events.DDP, "Debug");

        MMProfileEnableEventRecursive(DDP_MMP_Events.MutexParent, 1);
        MMProfileEnableEventRecursive(DDP_MMP_Events.BackupReg, 1);
        //MMProfileEnableEventRecursive(DDP_MMP_Events.DDP_IRQ, 1);
        MMProfileEnableEventRecursive(DDP_MMP_Events.WAIT_INTR, 1);
    }
}

// ---------------------------------------------------------------------------
//  Command Processor
// ---------------------------------------------------------------------------
static char dbg_buf[2048];
extern void mtkfb_dump_layer_info(void);
extern unsigned int gNeedToRecover;
extern unsigned int isAEEEnabled;
static void process_dbg_opt(const char *opt)
{
    char *buf = dbg_buf + strlen(dbg_buf);
    if (0 == strncmp(opt, "regr:", 5))
    {
        char *p = (char *)opt + 5;
        unsigned int addr = (unsigned int) simple_strtoul(p, &p, 16);

        if (addr) 
        {
            unsigned int regVal = DISP_REG_GET(addr);
            DISP_MSG("regr: 0x%08X = 0x%08X\n", addr, regVal);
            sprintf(buf, "regr: 0x%08X = 0x%08X\n", addr, regVal);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "regw:", 5))
    {
        char *p = (char *)opt + 5;
        unsigned int addr = (unsigned int) simple_strtoul(p, &p, 16);
        unsigned int val = (unsigned int) simple_strtoul(p + 1, &p, 16);
        if (addr) 
        {
            unsigned int regVal;
            DISP_REG_SET(addr, val);
            regVal = DISP_REG_GET(addr);
            DISP_DBG("regw: 0x%08X, 0x%08X = 0x%08X\n", addr, val, regVal);
            sprintf(buf, "regw: 0x%08X, 0x%08X = 0x%08X\n", addr, val, regVal);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "dbg_log:", 8))
    {
        char *p = (char *)opt + 8;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (enable)
            dbg_log = 1;
        else
            dbg_log = 0;

        sprintf(buf, "dbg_log: %d\n", dbg_log);
    }
    else if (0 == strncmp(opt, "irq_log:", 8))
    {
        char *p = (char *)opt + 8;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (enable)
            irq_log = 1;
        else
            irq_log = 0;
        
        sprintf(buf, "irq_log: %d\n", irq_log);        
    }
    else if (0 == strncmp(opt, "irq_err_log:", 12))
    {
        char *p = (char *)opt + 12;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (enable)
            irq_err_log = 1;
        else
            irq_err_log = 0;
        
        sprintf(buf, "irq_err_log: %d\n", irq_err_log);        
    }    
    else if (0 == strncmp(opt, "backlight:", 10))
    {
        char *p = (char *)opt + 10;
        unsigned int level = (unsigned int) simple_strtoul(p, &p, 10);

        if (level) 
        {
            disp_bls_set_backlight(level);            
            sprintf(buf, "backlight: %d\n", level); 
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "dump_reg:", 9))
    {
        char *p = (char *)opt + 9;
        unsigned int module = (unsigned int) simple_strtoul(p, &p, 10);
        DISP_MSG("process_dbg_opt, module=%d \n", module);
        if (module<DISP_MODULE_MAX) 
        {
            disp_dump_reg(module);            
            sprintf(buf, "dump_reg: %d\n", module); 
        } else {
            DISP_MSG("process_dbg_opt2, module=%d \n", module);
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "dump_aal:", 9))
    {        
        char *p = (char *)opt + 9;
        unsigned int arg = (unsigned int) simple_strtoul(p, &p, 10);
        if (arg == 0)
        {
            int i;
            unsigned int hist[LUMA_HIST_BIN];
            disp_get_hist(hist);
            for (i = 0; i < LUMA_HIST_BIN; i++)
            {
                DISP_DBG("LUMA_HIST_%02d: %d\n", i, hist[i]);
                sprintf(dbg_buf + strlen(dbg_buf), "LUMA_HIST_%2d: %d\n", i, hist[i]);
            }
        }
        else if (arg == 1)
        {
            int i;
            DISP_AAL_PARAM param;
            
            GetUpdateMutex();
            memcpy(&param, get_aal_config(), sizeof(DISP_AAL_PARAM));
            ReleaseUpdateMutex();

            DISP_DBG("pwmDuty: %lu\n", param.pwmDuty);
            sprintf(dbg_buf + strlen(dbg_buf), "pwmDuty: %lu\n", param.pwmDuty);
            for (i = 0; i < LUMA_CURVE_POINT; i++)
            {
                DISP_DBG("lumaCurve[%02d]: %lu\n", i, param.lumaCurve[i]);
                sprintf(dbg_buf + strlen(dbg_buf), "lumaCurve[%02d]: %lu\n", i, param.lumaCurve[i]);
            }
        }
    }
    else if (0 == strncmp(opt, "debug:", 6))
    {
        char *p = (char *)opt + 6;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if(enable==1)
        {
            printk("[DDP] debug=1, trigger AEE\n");
            aee_kernel_exception("DDP-TEST-ASSERT", "[DDP] DDP-TEST-ASSERT");
        }
        else if(enable==2)
        {
           ddp_mem_test();
        }
        else if(enable==3)
        {
           ddp_mem_test2();
        }
        else if(enable==4)
        {
            DDP_IRQ_ERR("test 4");
        }
        else if(enable==5)
        {
            DISP_MSG("SMI_LARB_MON_REQ0=0x%x, SMI_LARB_MON_REQ1=0x%x, SMI_0=0x%x, SMI_600=0x%x, SMI_604=0x%x, SMI_610=0x%x, SMI_614=0x%x, \
            color_h_cnt=%d, color_line_cnt=%d, ovl_add_con=0x%x, ovl_ctrl_flow=0x%x \n", 
                          *(volatile unsigned int*)0xf4010450, 
                          *(volatile unsigned int*)0xf4010454,
                          *(volatile unsigned int*)0xf4010000,
                          *(volatile unsigned int*)0xf4010600,
                          *(volatile unsigned int*)0xf4010604,
                          *(volatile unsigned int*)0xf4010610,
                          *(volatile unsigned int*)0xf4010614,
                          *(volatile unsigned int*)0xf400b404,
        		          *(volatile unsigned int*)0xf400b408,
        		          DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG),
        		          DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG));
           sprintf(dbg_buf + strlen(dbg_buf), "SMI_LARB_MON_REQ0=0x%x, SMI_LARB_MON_REQ1=0x%x, SMI_0=0x%x, SMI_600=0x%x, SMI_604=0x%x, SMI_610=0x%x, SMI_614=0x%x,"
        		   "color_h_cnt=%d, color_line_cnt=%d, ovl_add_con=0x%x, ovl_ctrl_flow=0x%x \n",
                          *(volatile unsigned int*)0xf4010450, 
                          *(volatile unsigned int*)0xf4010454,
                          *(volatile unsigned int*)0xf4010000,
                          *(volatile unsigned int*)0xf4010600,
                          *(volatile unsigned int*)0xf4010604,
                          *(volatile unsigned int*)0xf4010610,
                          *(volatile unsigned int*)0xf4010614,
                          *(volatile unsigned int*)0xf400b404,
        		          *(volatile unsigned int*)0xf400b408,
        		          DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG),
        		          DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG));               
        }
        else if(enable==6)
        {
            mtkfb_dump_layer_info();
        }
        else if (enable == 7)
        {
        	gNeedToRecover = 1;
        }
        else if(enable==8)
        {
            DISP_MSG("disp_clock_check start! \n");      
            disp_clock_check();
            DISP_MSG("disp_clock_check end! \n");      
        }
        else if(enable==9)
        {
            DISP_MSG("ddp dump info ! \n"); 
            ddp_dump_info(DISP_MODULE_OVL);
            ddp_dump_info(DISP_MODULE_RDMA);
            ddp_dump_info(DISP_MODULE_COLOR);
            ddp_dump_info(DISP_MODULE_BLS);            
            ddp_dump_info(DISP_MODULE_CONFIG);
            ddp_dump_info(DISP_MODULE_MUTEX);
            ddp_dump_info(DISP_MODULE_RDMA1);
            ddp_dump_info(DISP_MODULE_WDMA0);
            ddp_dump_info(DISP_MODULE_DSI_CMD);
            ddp_dump_info(DISP_MODULE_DPI0);
        }
        else if(enable==10)
        {
            DISP_MSG("ddp dump reg ! \n"); 
            disp_dump_reg(DISP_MODULE_OVL);
            disp_dump_reg(DISP_MODULE_RDMA);
            disp_dump_reg(DISP_MODULE_COLOR);
            disp_dump_reg(DISP_MODULE_BLS);            
            disp_dump_reg(DISP_MODULE_CONFIG);
            disp_dump_reg(DISP_MODULE_MUTEX);
            disp_dump_reg(DISP_MODULE_RDMA1);
            disp_dump_reg(DISP_MODULE_WDMA0);
            
            disp_print_reg(DISP_MODULE_OVL);
            disp_print_reg(DISP_MODULE_RDMA);
            disp_print_reg(DISP_MODULE_COLOR);
            disp_print_reg(DISP_MODULE_BLS);            
            disp_print_reg(DISP_MODULE_CONFIG);
            disp_print_reg(DISP_MODULE_MUTEX);
            disp_print_reg(DISP_MODULE_RDMA1);
            disp_print_reg(DISP_MODULE_WDMA0);
            disp_print_reg(DISP_MODULE_DSI_CMD);
            disp_print_reg(DISP_MODULE_DPI0);
        }
        else if((enable>=11)&&(enable<=15))
        {
            gEnableUltra = 1;
            gUltraLevel = enable - 11;
            sprintf(buf, "gUltraLevel: %d, DISP_REG_RDMA_MEM_GMC_SETTING_0=0x%x, DISP_REG_RDMA_FIFO_CON=0x%x \n", 
                gUltraLevel,
                DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0),
                DISP_REG_GET(DISP_REG_RDMA_FIFO_CON)); 
            DISP_MSG("ddp debug set gUltraLevel = %d, DISP_REG_RDMA_MEM_GMC_SETTING_0=0x%x, DISP_REG_RDMA_FIFO_CON=0x%x \n", 
                gUltraLevel,
                DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0),
                DISP_REG_GET(DISP_REG_RDMA_FIFO_CON));
        }
        else if(enable==21)
        {
            sprintf(buf, "base:\n\
            config f4+0 \n\
            ovl 7\n\
            rdma 8\n\
            rdma1 12\n\
            wdma 9\n\
            bls a\n\
            color b\n\
            dsi c\n\
            dpi d\n\
            mm_mutex e\n\
            mm_cmdq f\n\
            smi_larb0 10\n\
            smi_common 11\n");
        }
        else if(enable==22)
        {
            sprintf(buf, "isAEEEnabled=%d \n",isAEEEnabled); 
            DISP_MSG("isAEEEnabled=%d \n", isAEEEnabled);
        }
    }
    
    else if (0 == strncmp(opt, "mmp", 3))
    {
        init_ddp_mmp_events();
    }
    else if (0 == strncmp(opt, "dpfd_ut1:", 9))
    {
        //char *p = (char *)opt + 9;
        //unsigned int channel = (unsigned int) simple_strtoul(p, &p, 10);
        //ddpk_testfunc_1(channel);
    }
    else if (0 == strncmp(opt, "dpfd_ut2:", 9))
    {
        //char *p = (char *)opt + 9;
        //unsigned int channel = (unsigned int) simple_strtoul(p, &p, 10);
        //ddpk_testfunc_2(channel);
    }
    else if (0 == strncmp(opt, "dpfd:log", 8))
    {
    }
    else if (0 == strncmp(opt, "pqon", 4))
    {
        pq_debug_flag=0;
        sprintf(buf, "Turn on PQ %d\n", pq_debug_flag);
    }
    else if (0 == strncmp(opt, "pqoff", 5))
    {
        pq_debug_flag=1;
        sprintf(buf, "Turn off PQ %d\n", pq_debug_flag);        
    }
    else if (0 == strncmp(opt, "pqdemo", 6))
    {
        pq_debug_flag=2;
        sprintf(buf, "Turn on PQ (demo) %d\n", pq_debug_flag);    
    }
    else if (0 == strncmp(opt, "pqstop", 6))
    {
        pq_debug_flag=3;
        sprintf(buf, "Stop mutex update %d\n", pq_debug_flag);    
    }
    else if (0 == strncmp(opt, "aalon", 5))
    {
        aal_debug_flag=0;
        sprintf(buf, "resume aal update %d\n", aal_debug_flag);    
    }
    else if (0 == strncmp(opt, "aaloff", 6))
    {
        aal_debug_flag=1;
        sprintf(buf, "suspend aal update %d\n", aal_debug_flag);    
    }
    else if (0 == strncmp(opt, "color_win:", 10))
    {
        char *p = (char *)opt + 10;
        unsigned int sat_upper, sat_lower, hue_upper, hue_lower;
        sat_upper = (unsigned int) simple_strtoul(p, &p, 10);
        p++;
        sat_lower = (unsigned int) simple_strtoul(p, &p, 10);
        p++;
        hue_upper = (unsigned int) simple_strtoul(p, &p, 10);
        p++;
        hue_lower = (unsigned int) simple_strtoul(p, &p, 10);
        DISP_MSG("Set color_win: %u, %u, %u, %u\n", sat_upper, sat_lower, hue_upper, hue_lower);
        disp_color_set_window(sat_upper, sat_lower, hue_upper, hue_lower);
    }
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT) && defined(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)
    else if (0 == strncmp(opt, "handle_init", 11))
    {
        unsigned int va = init_tplay_handle();
        DISP_MSG("debug handle_init, tplay_handle_virt_addr=0x%x \n", va);
    }
    else if (0 == strncmp(opt, "handle_check", 12))
    {
        unsigned int va = get_tplay_handle();
        DISP_MSG("debug handle_check, tplay_handle_virt_addr=0x%x \n", va);
        unsigned int *x = (unsigned int *)va;
        *x = 0xff;
        DISP_MSG("debug handle_check, write and read 0x%x \n", *x);
    }
    else if (0 == strncmp(opt, "handle_set", 10))
    {
        int ret = set_tplay_handle_addr_request();
        DISP_MSG("debug handle_set, ret=%d \n", ret);
    }
    else if (0 == strncmp(opt, "svp_init", 8))
    {
        DISP_MSG("initialize svp-ddp sessions \n");
        late_open_mobicore_device();
        late_init_session_drv();
        late_init_session_tl();
    }
#endif
    else
    {
	    goto Error;
    }

    return;

Error:
    DISP_ERR("parse command error!\n%s\n\n%s", opt, STR_HELP);
}


static void process_dbg_cmd(char *cmd)
{
    char *tok;
    
    DISP_DBG("cmd: %s\n", cmd);
    memset(dbg_buf, 0, sizeof(dbg_buf));
    while ((tok = strsep(&cmd, " ")) != NULL)
    {
        process_dbg_opt(tok);
    }
}


// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------

static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}


static char cmd_buf[512];

static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    if (strlen(dbg_buf))
        return simple_read_from_buffer(ubuf, count, ppos, dbg_buf, strlen(dbg_buf));
    else
        return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));
        
}


static ssize_t debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax) 
        count = debug_bufmax;

	if (copy_from_user(&cmd_buf, ubuf, count))
		return -EFAULT;

	cmd_buf[count] = 0;

    process_dbg_cmd(cmd_buf);

    return ret;
}


static struct file_operations debug_fops = {
	.read  = debug_read,
    .write = debug_write,
	.open  = debug_open,
};


void ddp_debug_init(void)
{
#if 1 
debugfs = debugfs_create_file("dispsys",
        S_IFREG|S_IRUGO, NULL, (void *)0, &debug_fops);
#endif
}


void ddp_debug_exit(void)
{
#if 1
debugfs_remove(debugfs);
#endif
}

   
#include <linux/vmalloc.h>
#define DDP_TEST_WIDTH 64
#define DDP_TEST_HEIGHT 64
#define DDP_TEST_BPP 3
#define DDP_MUTEX_FOR_ROT_SCL_WDMA 1
extern unsigned char data_rgb888_64x64[12288];
extern unsigned char data_rgb888_64x64_golden[12288];
int ddp_mem_test2(void)
{  
    int result = 0;
#if 0      
    unsigned int* pSrc;
    unsigned int* pDst;
    DdpkBitbltConfig pddp;
    
    pSrc= vmalloc(DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP);
    if(pSrc==0)
    {
        printk("[DDP] error: dma_alloc_coherent error!  dma memory not available.\n");
        return 0;
    }
    else
    {
        printk("[ddp] pSrc=0x%x \n", (unsigned int)pSrc);
    }
    memcpy((void*)pSrc, data_rgb888_64x64, DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP);
    
    pDst= vmalloc(DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP);
    if(pDst==0)
    {
        printk("[DDP] error: dma_alloc_coherent error!  dma memory not available.\n");
        return 0;
    }
    else
    {
        printk("[ddp] pDst=0x%x\n", (unsigned int)pDst);
    }
    memset((void*)pDst, 0, DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP);

    /*
    disp_power_on(DISP_MODULE_ROT);
    disp_power_on(DISP_MODULE_SCL);
    disp_power_on(DISP_MODULE_WDMA0); 
   */
    // config port to virtual
    {
        M4U_PORT_STRUCT sPort;
        sPort.ePortID = M4U_PORT_ROT_EXT;
        sPort.Virtuality = 1; 					   
        sPort.Security = 0;
        sPort.Distance = 1;
        sPort.Direction = 0;
        m4u_config_port(&sPort);
        
        sPort.ePortID = M4U_PORT_WDMA0;
        sPort.Virtuality = 1; 					   
        sPort.Security = 0;
        sPort.Distance = 1;
        sPort.Direction = 0;
        m4u_config_port(&sPort);
	  }
	      
    //config

    pddp.srcX = 0;
    pddp.srcY = 0;
    pddp.srcW = DDP_TEST_WIDTH;
    pddp.srcWStride = DDP_TEST_WIDTH;
    pddp.srcH = DDP_TEST_HEIGHT;
    pddp.srcHStride = DDP_TEST_HEIGHT;
    pddp.srcAddr[0] = (unsigned int)pSrc;
    pddp.srcFormat = eRGB888_K;
    pddp.srcPlaneNum = 1;
    pddp.srcBufferSize[0] = DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP;
    
    pddp.dstX = 0;
    pddp.dstY = 0;
    pddp.dstW = DDP_TEST_WIDTH;
    pddp.dstWStride = DDP_TEST_WIDTH;
    pddp.dstH = DDP_TEST_HEIGHT;
    pddp.dstHStride = DDP_TEST_HEIGHT;
    pddp.dstAddr[0] = (unsigned int)pDst;
    pddp.dstFormat = eRGB888_K;
    pddp.pitch = DDP_TEST_WIDTH;
    pddp.dstPlaneNum = 1;
    pddp.dstBufferSize[0] = DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP;
    pddp.orientation = 0;   
    result = DDPK_Bitblt_Config( DDPK_CH_HDMI_0, &pddp );
    if(result)
    {
        printk("[DDP] error: DDPK_Bitblt_Config fail!, ret=%d\n", result);
    }

    printk("DDP, DDPK_Bitblt module setting: \n");
    disp_dump_reg(DISP_MODULE_ROT);
    disp_dump_reg(DISP_MODULE_SCL);
    disp_dump_reg(DISP_MODULE_WDMA0);
    disp_dump_reg(DISP_MODULE_CONFIG);
    
    result = DDPK_Bitblt( DDPK_CH_HDMI_0);
    if(result)
    {
        printk("[DDP] error: DDPK_Bitblt() fail, result=%d \n", result);
    }

        
    // result verify
    {
        unsigned int diff_cnt = 0;
        unsigned int t=0;
        unsigned int size = DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP;
        for(t=0;t<size;t++)
        {
            if( *((unsigned char*)pSrc+t) != *((unsigned char*)data_rgb888_64x64+t) )
            {
                diff_cnt++;
                printk("t=%d, diff_cnt=%d, dst=0x%x, gold=0x%x \n", 
                t, 
                diff_cnt, 
                *((unsigned char*)pSrc+t), 
                *((unsigned char*)data_rgb888_64x64+t) );
            }
    
        }
        if(diff_cnt == 0)
            printk("ddp_mem_test src compare result: success \n");
        else
        {
            printk("[DDP] error: ddp_mem_test src compare result: fail \n");
            printk("detail, %d, %d, %%%d \n", diff_cnt, size, diff_cnt*100/size);  
            result = -1;
        }              
    }
    
    {
        unsigned int diff_cnt = 0;
        unsigned int t=0;
        unsigned int size = DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP;
        for(t=0;t<size;t++)
        {
            if( *((unsigned char*)pDst+t) != *((unsigned char*)data_rgb888_64x64_golden+t) )
            {
                diff_cnt++;
                printk("t=%d, diff_cnt=%d, dst=0x%x, gold=0x%x \n", 
                t, 
                diff_cnt, 
                *((unsigned char*)pDst+t), 
                *((unsigned char*)data_rgb888_64x64_golden+t) );
            }
    
        }
        if(diff_cnt == 0)
            printk("ddp_mem_test result: success \n");
        else
        {
            printk("[DDP] error: ddp_mem_test result: fail \n");
            printk("detail, %d, %d, %%%d \n", diff_cnt, size, diff_cnt*100/size); 
            result = -1;
        }              
    }
    /*
    disp_power_off(DISP_MODULE_ROT);
    disp_power_off(DISP_MODULE_SCL);
    disp_power_off(DISP_MODULE_WDMA0);
    */
    //dealloc memory
    vfree(pSrc);
    vfree(pDst);   

#endif
    return result;

}

int ddp_mem_test(void)
{    
    int result = 0;
#if 0    
    struct disp_path_config_struct config;
    unsigned int* pSrc;
    unsigned char* pSrcPa;
    unsigned int* pDst;
    unsigned char* pDstPa;
    
    pSrc= dma_alloc_coherent(NULL, DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP, (dma_addr_t *)&pSrcPa, GFP_KERNEL);
    if(pSrc==0 || pSrcPa==0)
    {
        printk("dma_alloc_coherent error!  dma memory not available.\n");
        return 0;
    }
    else
    {
        printk("[ddp] pSrc=0x%x, pSrcPa=0x%x \n", (unsigned int)pSrc, (unsigned int)pSrcPa);
    }
    memcpy((void*)pSrc, data_rgb888_64x64, DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP);
    
    pDst= dma_alloc_coherent(NULL, DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP, (dma_addr_t *)&pDstPa, GFP_KERNEL);
    if(pDst==0 || pDstPa==0)
    {
        printk("dma_alloc_coherent error!  dma memory not available.\n");
        return 0;
    }
    else
    {
        printk("[ddp] pDst=0x%x, pDstPa=0x%x \n",(unsigned int) pDst, (unsigned int)pDstPa);
    }
    memset((void*)pDst, 0, DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP);


    // config port to physical
    {
        M4U_PORT_STRUCT sPort;
        sPort.ePortID = M4U_PORT_ROT_EXT;
        sPort.Virtuality = 0; 					   
        sPort.Security = 0;
        sPort.Distance = 1;
        sPort.Direction = 0;
        m4u_config_port(&sPort);
        
        sPort.ePortID = M4U_PORT_WDMA0;
        sPort.Virtuality = 0; 					   
        sPort.Security = 0;
        sPort.Distance = 1;
        sPort.Direction = 0;
        m4u_config_port(&sPort);
	  }
	  
    config.srcModule = DISP_MODULE_SCL;
    config.addr = (unsigned int)pSrcPa; 
    config.inFormat = eRGB888; 
    config.pitch = DDP_TEST_WIDTH;
    config.srcROI.x = 0;
    config.srcROI.y = 0; 
    config.srcROI.width = DDP_TEST_WIDTH; 
    config.srcROI.height = DDP_TEST_HEIGHT; 
    config.srcWidth = DDP_TEST_WIDTH;
    config.srcHeight = DDP_TEST_HEIGHT;
    config.dstModule = DISP_MODULE_WDMA0;
    config.outFormat = eRGB888; 
    config.dstAddr = (unsigned int)pDstPa; 
    config.dstWidth = DDP_TEST_WIDTH; 
    config.dstHeight = DDP_TEST_HEIGHT;
    config.dstPitch = DDP_TEST_WIDTH;
    /*
    disp_power_on(DISP_MODULE_ROT);
    disp_power_on(DISP_MODULE_SCL);
    disp_power_on(DISP_MODULE_WDMA0);
    */
    disp_path_get_mutex_(DDP_MUTEX_FOR_ROT_SCL_WDMA);
    disp_path_config_(&config, DDP_MUTEX_FOR_ROT_SCL_WDMA);
    
    printk("*after ddp test config start: -------------------\n");
    disp_dump_reg(DISP_MODULE_ROT);
    disp_dump_reg(DISP_MODULE_SCL);
    disp_dump_reg(DISP_MODULE_WDMA0);
    disp_dump_reg(DISP_MODULE_CONFIG);
    printk("*after ddp test config end: ---------------------\n");
    
    disp_path_release_mutex_(DDP_MUTEX_FOR_ROT_SCL_WDMA);
    if(*(volatile unsigned int*)DISP_REG_CONFIG_MUTEX1 != 0)
    {
        *(volatile unsigned int*)DISP_REG_CONFIG_MUTEX1 = 0;
    }
    
    printk("ddp_mem_test wdma wait done... \n"); 
    WDMAWait(0);
    printk("ddp_mem_test wdma done! \n");            
    
    if(0) //compare source
    {
        unsigned int diff_cnt = 0;
        unsigned int t=0;
        unsigned int size = DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP;
        for(t=0;t<size;t++)
        {
            if( *((unsigned char*)pSrc+t) != *((unsigned char*)data_rgb888_64x64+t) )
            {
                diff_cnt++;
                printk("t=%d, diff_cnt=%d, dst=0x%x, gold=0x%x \n", 
                t, 
                diff_cnt, 
                *((unsigned char*)pSrc+t), 
                *((unsigned char*)data_rgb888_64x64+t) );
            }
    
        }
        if(diff_cnt == 0)
            printk("ddp_mem_test src compare result: success \n");
        else
        {
            printk("ddp_mem_test src compare result: fail \n");
            printk("detail, %d, %d, %%%d \n", diff_cnt, size, diff_cnt*100/size);  
            result = -1;
        }              
    }
    
    if(1)  //compare dst
    {
        unsigned int diff_cnt = 0;
        unsigned int t=0;
        unsigned int size = DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP;
        for(t=0;t<size;t++)
        {
            if( *((unsigned char*)pDst+t) != *((unsigned char*)data_rgb888_64x64_golden+t) )
            {
                diff_cnt++;
                printk("t=%d, diff_cnt=%d, dst=0x%x, gold=0x%x \n", 
                t, 
                diff_cnt, 
                *((unsigned char*)pDst+t), 
                *((unsigned char*)data_rgb888_64x64_golden+t) );
            }
    
        }
        if(diff_cnt == 0)
            printk("ddp_mem_test result: success \n");
        else
        {
            printk("ddp_mem_test result: fail \n");
            printk("detail, %d, %d, %%%d \n", diff_cnt, size, diff_cnt*100/size); 
            result = -1;
        }              
    }
    
    // print out dst buffer to save as golden
    if(0)
    {
  	      unsigned int t=0;
	        unsigned int size = DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP;
	        for(t=0;t<size;t++)
	        {
		    	  printk("0x%x, ", *((unsigned char*)pDst+t));
		    	  if((t+1)%12==0)
		    	  {
		    	  	  printk("\n%05d: ", (t+1)/12);
		    	  }
	        }
    }
    /*
    disp_power_off(DISP_MODULE_ROT);
    disp_power_off(DISP_MODULE_SCL);
    disp_power_off(DISP_MODULE_WDMA0);
    */
    //dealloc memory
    dma_free_coherent(NULL, DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP, pSrc, (dma_addr_t)&pSrcPa);
    dma_free_coherent(NULL, DDP_TEST_WIDTH*DDP_TEST_HEIGHT*DDP_TEST_BPP, pDst, (dma_addr_t)&pDstPa);   
#endif

    return result;

}


char* ddp_get_module_name(DISP_MODULE_ENUM module)
{
    switch(module)
    {
        case DISP_MODULE_OVL:     return "ovl";
        case DISP_MODULE_COLOR:   return "color";
        case DISP_MODULE_BLS:     return "bls";
        case DISP_MODULE_WDMA0:   return "wdma0";
        case DISP_MODULE_RDMA:    return "rdma";
        case DISP_MODULE_DPI0:    return "dpi0";
        case DISP_MODULE_DBI:     return "dbi";
        case DISP_MODULE_DSI_CMD: 
        case DISP_MODULE_DSI_VDO: 
                                  return "dsi";
        case DISP_MODULE_CONFIG:  return "config";
        case DISP_MODULE_MUTEX:   return "mutex";
        case DISP_MODULE_CMDQ:    return "cmdq";
        case DISP_MODULE_G2D:     return "g2d";
        case DISP_MODULE_SMI:     return "smi";
        default:
             DISP_MSG("invalid module id=%d", module);
             return "unknown";    	
    }
}

char* ddp_get_fmt_name(DISP_MODULE_ENUM module, unsigned int fmt)
{
   if(module==DISP_MODULE_WDMA0)
   {
       switch(fmt)
       {
           case 0x00: return "rgb565";
           case 0x01: return "rgb888";
           case 0x02: return "rgba8888";
           case 0x03: return "argb8888";
           case 0x04: return "uyvy";
           case 0x05: return "yuyv";
           case 0x07: return "yonly";
           case 0x08: return "iyuv";
           case 0x0c: return "nv12";
           default: 
               DISP_MSG("ddp_get_fmt_name, unknown fmt=%d, module=%d \n", fmt, module);
               return "unknown";
       }
   }
   else if(module==DISP_MODULE_OVL)
   {
       switch(fmt)
       {
           case 0:  return "rgb888";
           case 1:  return "rgb565";
           case 2:  return "argb8888";
           case 3:  return "pargb8888";
           case 4:  return "xrgb8888";           	
           case 8:  return "yuyv";
           case 9:  return "uvvy";
           case 10: return "yvyu"; 
           case 11: return "vyuy";
           case 15: return "yuv444";               	            	         	
           default: 
               DISP_MSG("ddp_get_fmt_name, unknown fmt=%d, module=%d \n", fmt, module);
               return "unknown"; 
       }   
   }
   else if(module==DISP_MODULE_RDMA) // todo: confirm with designers
   {
       switch(fmt)
       {
           case 0:  return "yuyv";
           case 1:  return "uyvy";
           case 2:  return "yvyu";
           case 3:  return "vyuy";
           case 4:  return "rgb565";
           case 8:  return "rgb888";
           case 16: return "argb8888";
           case 32: return "yuv420_3p";
           case 40: return "yuv422_3p";
           case 48: return "yuv420_2p";
           case 56: return "yuv422_2p";
           default: 
               DISP_MSG("ddp_get_fmt_name, unknown fmt=%d, module=%d \n", fmt, module);
               return "unknown"; 
       }   
   }  
   else if(module==DISP_MODULE_MUTEX)
   {
       switch(fmt)
       {
           case 0: return "single";
           case 1: return "dsi_vdo";
           case 2: return "dpi";
           default: 
               DISP_MSG("ddp_get_fmt_name, unknown fmt=%d, module=%d \n", fmt, module);
               return "unknown"; 
       }
   } 
   else
   {
       DISP_MSG("ddp_get_fmt_name, unknown module=%d \n", module);
   }
   
   return "unknown";
}

static char* ddp_get_mutex_module_name(unsigned int bit)
{           
       switch(bit)
       {
           case 3: return "ovl";
           case 6: return "wdma0";
           case 7: return "color";
           case 9: return "bls";
           case 10: return "rdma0";
           case 12: return "rdma1";
           default: 
             printk("error_bit=%d, ", bit);
             return "mutex-unknown";
       }
}

static char* ddp_clock_0(int bit)
{
    switch(bit)
    {
        case  0: return "smi common";
        case  1: return "smi larb0";
        case  2: return "mm cmdq";
        case  3: return "mutex";
        case  4: return "disp_color"; 
        case  5: return "disp_bls";
        case  6: return "disp_wdma0";
        case  7: return "disp_rdma";
        case  8: return "disp_ovl";
        case 17: return "fake_eng";
        case 18: return "mutex_32k";
        case 19: return "disp_rdma1";
        default : return "";
    }
}

static char* ddp_clock_1(int bit)
{
    switch(bit)
    {
        case 0: return "dsi_engine"; 
        case 1: return "dsi_digital"; 
        case 2: return "dpi_digital_lane";
        case 3: return "dpi_engine"; 
        default : return "";
    }
}

char* ddp_ovl_get_status(unsigned int status)
{
    DISP_MSG("ovl_status: ");
    if(status&(1<<10))  printk("addcon_idle,");
    if(status&(1<<11))  printk("blend_idle,");
    if(status&(1<<15))  printk("out_idle,");
    if(status&(1<<16))  printk("rdma3_idle,");
    if(status&(1<<17))  printk("rdma2_idle,");
    if(status&(1<<18))  printk("rdma1_idle,");
    if(status&(1<<19))  printk("rdma0_idle,");
    if(status&(1<<20))  printk("rst,");
    if(status&(1<<21))  printk("trig,");
    if(status&(1<<23))  printk("frame_hwrst_done,");
    if(status&(1<<24))  printk("frame_swrst_done,");
    if(status&(1<<25))  printk("frame_underrun,");
    if(status&(1<<26))  printk("frame_done,");
    if(status&(1<<27))  printk("ovl_running,");
    if(status&(1<<28))  printk("ovl_start,");
    if(status&(1<<29))  printk("ovl_clr,");
    if(status&(1<<30))  printk("reg_update,");
    if(status&(1<<31))  printk("ovl_upd_reg,");  
    printk("\n");
    
    DISP_MSG("ovl_state_machine: ");
    switch(status&0x3ff)
    {
        case 0x1:  printk("idle\n"); break;
        case 0x2:  printk("wait_SOF\n"); break;
        case 0x4:  printk("prepare\n"); break;
        case 0x8:  printk("reg_update\n"); break;
        case 0x10: printk("eng_clr\n"); break;
        case 0x20: printk("processing\n"); break;
        case 0x40: printk("s_wait_w_rst\n"); break;
        case 0x80: printk("s_w_rst\n"); break;
        case 0x100:printk("h_wait_w_rst\n"); break;
        case 0x200:printk("h_w_rst\n"); break;
        default:   printk("unknown\n");
    }

}


char* ddp_wdma_get_status(unsigned int status)
{
    switch(status)
    {
        case 0x1:   return "idle"; 
        case 0x2:   return "clear"; 
        case 0x4:   return "prepare"; 
        case 0x8:   return "prepare"; 
        case 0x10:  return "data_running"; 
        case 0x20:  return "eof_wait"; 
        case 0x40:  return "soft_reset_wait"; 
        case 0x80:  return "eof_done"; 
        case 0x100: return "soft_reset_done";
        case 0x200: return "frame_complete";
        default:    return "unknown";
    }

}

char* ddp_rdma_get_status(unsigned int status)
{
    switch(status)
    {
        case 0x1:   return "idle"; 
        case 0x2:   return "wait_sof"; 
        case 0x4:   return "reg_update"; 
        case 0x8:   return "clear0"; 
        case 0x10:  return "clear1"; 
        case 0x20:  return "int0"; 
        case 0x40:  return "int1"; 
        case 0x80:  return "data_running"; 
        case 0x100: return "wait_done";
        case 0x200: return "warm_reset";
        case 0x400: return "wait_reset";
        default:    return "unknown";
    }
}

char* ddp_dsi_get_status(unsigned int status)
{
    switch(status)
    {
       		case 0x0001: return "Idle (wait for command)";
       		case 0x0002: return "Reading command queue for header";
       		case 0x0004: return "Sending type-0 command";
       		case 0x0008: return "Waiting frame data from RDMA for type-1 command";
       		case 0x0010: return "Sending type-1 command";
       		case 0x0020: return "Sending type-2 command";
       		case 0x0040: return "Reading command queue for data";
       		case 0x0080: return "Sending type-3 command";
       		case 0x0100: return "Sending BTA";
       		case 0x0200: return "Waiting RX-read data";
       		case 0x0400: return "Waiting SW RACK for RX-read data";
       		case 0x0800: return "Waiting TE)";
       		case 0x1000: return "Get TE";
       		case 0x2000: return "Waiting external TE";
       		case 0x4000: return "Waiting SW RACK for TE";
       		default: return "unknown";
    }
}

char* ddp_dsi_get_err(unsigned int reg)
{
    DISP_MSG("error_state=");
    if((reg&0xf2)!=0)
    {
        if((reg>>7)&0x1)
          printk("contention_err, ");
        if((reg>>6)&0x1)
          printk("false_ctrl_err, ");
        if((reg>>5)&0x1)
          printk("lpdt_sync_err, "); 
        if((reg>>4)&0x1)
          printk("esc_entry_err, ");
        if((reg>>1)&0x1)
          printk("buffer_underrun");

        printk("\n");
    }
    else
    {  
        printk("none. \n");
    }
}

char* ddp_get_rdma_mode(unsigned int con)
{
    if(((con>>1)&0x1)==1)
        return "mem_mode";
    else
        return "direct_link";
}

int ddp_dump_info(DISP_MODULE_ENUM module)
{
    unsigned int size;
    unsigned int reg_base;
    unsigned int index=0;
    unsigned int i=0;
    unsigned int j=0;
    unsigned int DISP_INDEX_OFFSET = 0xA000;
    
    switch(module)
    {        
        /* Dump WDMA Reg */
        case DISP_MODULE_WDMA0:  
            DISP_MSG("wdma0: en=%d, w=%d, h=%d, clip=(%d, %d, %d, %d), pitch=(%d,%d), addr=(0x%x,0x%x,0x%x), fmt=%s \n",
              DISP_REG_GET(DISP_REG_WDMA_EN),
              DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE)&0x3fff,
              (DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE)>>16)&0x3fff,
              DISP_REG_GET(DISP_REG_WDMA_CLIP_COORD)&0x3fff, 
              (DISP_REG_GET(DISP_REG_WDMA_CLIP_COORD)>>16)&0x3fff,
              DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE)&0x3fff, 
              (DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE)>>16)&0x3fff,
              DISP_REG_GET(DISP_REG_WDMA_DST_W_IN_BYTE),
              DISP_REG_GET(DISP_REG_WDMA_DST_UV_PITCH),
              DISP_REG_GET(DISP_REG_WDMA_DST_ADDR),
              DISP_REG_GET(DISP_REG_WDMA_DST_U_ADDR),
              DISP_REG_GET(DISP_REG_WDMA_DST_V_ADDR),
              ddp_get_fmt_name(DISP_MODULE_WDMA0, (DISP_REG_GET(DISP_REG_WDMA_CFG)>>4)&0xf));
            DISP_MSG("wdma0: status=%s, total_pix=%d, output_pixel=(l:%d, p:%d), input_pixel=(l:%d, p:%d) \n",
              ddp_wdma_get_status(DISP_REG_GET(DISP_REG_WDMA_FLOW_CTRL_DBG)),
              (DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE)&0x3fff)*((DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE)>>16)&0x3fff),
              (DISP_REG_GET(DISP_REG_WDMA_EXEC_DBG)>>16)&0xffff,
              DISP_REG_GET(DISP_REG_WDMA_EXEC_DBG)&0xffff,
              (DISP_REG_GET(DISP_REG_WDMA_CLIP_DBG)>>16)&0xffff,
              DISP_REG_GET(DISP_REG_WDMA_CLIP_DBG)&0xffff);         
            break;
           
        /* Dump RDMA Reg */
        case DISP_MODULE_RDMA:  
             index = 0;
             DISP_MSG("rdma%d: en=%d, mode=%s, smi_busy=%d, w=%d, h=%d, pitch=%d, addr=0x%x, fmt=%s, fifo_min=%d \n",
              index,
              DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON+DISP_INDEX_OFFSET*index)&0x1,
              ddp_get_rdma_mode(DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON+DISP_INDEX_OFFSET*index)),
              (DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON)>>12)&0x1,
              DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0+DISP_INDEX_OFFSET*index)&0xfff,
              DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1+DISP_INDEX_OFFSET*index)&0xfffff,
              DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH+DISP_INDEX_OFFSET*index), 
              DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR+DISP_INDEX_OFFSET*index),
              ((DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON+DISP_INDEX_OFFSET*index)&0x2)==0) ? "rgb888" : ddp_get_fmt_name(DISP_MODULE_RDMA, (DISP_REG_GET(DISP_REG_RDMA_MEM_CON+DISP_INDEX_OFFSET*index)>>4)&0x3f),
              DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG+DISP_INDEX_OFFSET*index));
            DISP_MSG("rdma%d: ack=%d, req=%d, status=%s, output_x=%d, output_y=%d \n", 
              index, 
              (DISP_REG_GET(DISPSYS_RDMA1_BASE+0x400)>>10)&0x1,
              (DISP_REG_GET(DISPSYS_RDMA1_BASE+0x400)>>11)&0x1,
              ddp_rdma_get_status((DISP_REG_GET(DISPSYS_RDMA1_BASE+0x408)>>8)&0x7ff),
              DISP_REG_GET(DISPSYS_RDMA1_BASE+0x4D0)&0x1fff,
              (DISP_REG_GET(DISPSYS_RDMA1_BASE+0x400)>>16)&0x1fff);
            break;

        /* Dump OVL Reg */
        case DISP_MODULE_OVL:  
            DISP_MSG("ovl%d: en=%d, layer(%d,%d,%d,%d), bg_w=%d, bg_h=%d, cur_x=%d, cur_y=%d, layer_pix_hit(%d, %d, %d, %d) \n",
              index,
              DISP_REG_GET(DISP_REG_OVL_EN),
              DISP_REG_GET(DISP_REG_OVL_SRC_CON)&0x1,
              (DISP_REG_GET(DISP_REG_OVL_SRC_CON)>>1)&0x1,
              (DISP_REG_GET(DISP_REG_OVL_SRC_CON)>>2)&0x1,
              (DISP_REG_GET(DISP_REG_OVL_SRC_CON)>>3)&0x1,
              DISP_REG_GET(DISP_REG_OVL_ROI_SIZE)&0xfff,
              (DISP_REG_GET(DISP_REG_OVL_ROI_SIZE)>>16)&0xfff,
              DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG)&0x1fff,
              (DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG)>>16)&0x1fff,
              (DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG)>>14)&0x1,
              (DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG)>>15)&0x1,
              (DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG)>>30)&0x1,
              (DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG)>>31)&0x1);
              ddp_ovl_get_status(DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG));            
            DISP_MSG("ovl%d: greq(ovl_req_smi)=%d, valid(ovl_send_data)=%d, ready(follow_engine_req_data)=%d \n",
              index,
              DISP_REG_GET(DISP_REG_CONFIG_GREQ)&0x1,
              (DISP_REG_GET(DISP_REG_CONFIG_VALID)>>10)&0x1,
              (DISP_REG_GET(DISP_REG_CONFIG_READY)>>10)&0x1);
            
            if(DISP_REG_GET(DISP_REG_OVL_SRC_CON)&0x1)
            {
                DISP_MSG("layer=%d, w=%d, h=%d, x=%d, y=%d, pitch=%d, addr=0x%x, fmt=%s \n",
                    0,
                    DISP_REG_GET(DISP_REG_OVL_L0_SRC_SIZE)&0xfff,
                    (DISP_REG_GET(DISP_REG_OVL_L0_SRC_SIZE)>>16)&0xfff,
                    DISP_REG_GET(DISP_REG_OVL_L0_OFFSET)&0xfff,
                    (DISP_REG_GET(DISP_REG_OVL_L0_OFFSET)>>16)&0xfff,
                    DISP_REG_GET(DISP_REG_OVL_L0_PITCH),
                    DISP_REG_GET(DISP_REG_OVL_L0_ADDR),
                    ddp_get_fmt_name(DISP_MODULE_OVL, (DISP_REG_GET(DISP_REG_OVL_L0_CON)>>12)&0xf));
            }
            if(DISP_REG_GET(DISP_REG_OVL_SRC_CON)&0x2)
            {          
                DISP_MSG("layer=%d, w=%d, h=%d, x=%d, y=%d, pitch=%d, addr=0x%x, fmt=%s \n",
                    1,
                    DISP_REG_GET(DISP_REG_OVL_L1_SRC_SIZE)&0xfff,
                    (DISP_REG_GET(DISP_REG_OVL_L1_SRC_SIZE)>>16)&0xfff,
                    DISP_REG_GET(DISP_REG_OVL_L1_OFFSET)&0xfff,
                    (DISP_REG_GET(DISP_REG_OVL_L1_OFFSET)>>16)&0xfff,
                    DISP_REG_GET(DISP_REG_OVL_L1_PITCH),
                    DISP_REG_GET(DISP_REG_OVL_L1_ADDR),
                    ddp_get_fmt_name(DISP_MODULE_OVL, (DISP_REG_GET(DISP_REG_OVL_L1_CON)>>12)&0xf));           
            }
            if(DISP_REG_GET(DISP_REG_OVL_SRC_CON)&0x4)
            {             
                DISP_MSG("layer=%d, w=%d, h=%d, x=%d, y=%d, pitch=%d, addr=0x%x, fmt=%s \n",
                    2,
                    DISP_REG_GET(DISP_REG_OVL_L2_SRC_SIZE)&0xfff,
                    (DISP_REG_GET(DISP_REG_OVL_L2_SRC_SIZE)>>16)&0xfff,
                    DISP_REG_GET(DISP_REG_OVL_L2_OFFSET)&0xfff,
                    (DISP_REG_GET(DISP_REG_OVL_L2_OFFSET)>>16)&0xfff,
                    DISP_REG_GET(DISP_REG_OVL_L2_PITCH),
                    DISP_REG_GET(DISP_REG_OVL_L2_ADDR),
                    ddp_get_fmt_name(DISP_MODULE_OVL, (DISP_REG_GET(DISP_REG_OVL_L2_CON)>>12)&0xf));            
            }
            if(DISP_REG_GET(DISP_REG_OVL_SRC_CON)&0x8)
            {            
                DISP_MSG("layer=%d, w=%d, h=%d, x=%d, y=%d, pitch=%d, addr=0x%x, fmt=%s \n",
                    3,
                    DISP_REG_GET(DISP_REG_OVL_L3_SRC_SIZE)&0xfff,
                    (DISP_REG_GET(DISP_REG_OVL_L3_SRC_SIZE)>>16)&0xfff,
                    DISP_REG_GET(DISP_REG_OVL_L3_OFFSET)&0xfff,
                    (DISP_REG_GET(DISP_REG_OVL_L3_OFFSET)>>16)&0xfff,
                    DISP_REG_GET(DISP_REG_OVL_L3_PITCH),
                    DISP_REG_GET(DISP_REG_OVL_L3_ADDR),
                    ddp_get_fmt_name(DISP_MODULE_OVL, (DISP_REG_GET(DISP_REG_OVL_L3_CON)>>12)&0xf));            
            }
            break;
        
        /* Dump DPI0 Reg */
        case DISP_MODULE_DPI0:             
            DISP_MSG("DPI0: \n"); 
            break;

        /* Dump DPI0 Reg */
        case DISP_MODULE_BLS:             
            DISP_MSG("BLS: \n"); 
            break;
            
        /* Dump CONFIG Reg */
        case DISP_MODULE_CONFIG:
            // print whole data path
            DISP_MSG("clock:");
            {
                unsigned int i;
                unsigned int reg;

                reg = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0);
                for(i=0;i<9;i++)
                {
                    if((reg&(1<<i))==0)
                       printk("%s, ", ddp_clock_0(i));
                }
                for(i=17;i<20;i++)
                {
                    if((reg&(1<<i))==0)
                       printk("%s, ", ddp_clock_0(i));
                }
                printk("\n");
                reg = DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON1);
                for(i=0;i<4;i++)
                {
                    if((reg&(1<<i))==0)
                       printk("%s, ", ddp_clock_1(i));
                }
                printk("\n");                
            }
            {
                unsigned int clock = (DISP_REG_GET(DISP_REG_CONFIG_CLOCK_DUMMY)>>7)&0x7;
                if(clock==3)
                    DISP_MSG("clock ok\n");
                else if(clock==0)
                    DISP_MSG("clock 80128\n");
                else if(clock==1)
                    DISP_MSG("clock 6496\n");
                else if(clock==2)
                    DISP_MSG("clock 4886\n");
            }
            
         break;

    case DISP_MODULE_MUTEX:
            DISP_MSG("mutex: \n");
            for(i=0;i<3;i++)
            {
                DISP_DBG("i=%d, mod=0x%x, en=%d \n", 
                  i, 
                  DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(i)), 
                  DISP_REG_GET(DISP_REG_CONFIG_MUTEX_EN(i)));
                  
                if( (DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(i))!=0) &&
                    (DISP_REG_GET(DISP_REG_CONFIG_MUTEX_EN(i))==1) )
                {
                   DISP_MSG("mutex%d, mode=%s, module=(", i, ddp_get_fmt_name(DISP_MODULE_MUTEX, DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(i))));
                   for(j=3;j<=12;j++)
                   {
                       if((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(i))>>j)&0x1)
                          printk("%s,", ddp_get_mutex_module_name(j));
                   }
                   printk(")\n");
                }
            }            
            break;

        /* Dump COLOR Reg */
        case DISP_MODULE_COLOR:
            DISP_MSG("color: w=%d, h=%d, h_cnt=%d, line_cnt=%d \n", 
                DISP_REG_GET(DISP_REG_COLOR_INTERNAL_IP_WIDTH),
                DISP_REG_GET(DISP_REG_COLOR_INTERNAL_IP_HEIGHT),
                DISP_REG_GET(DISP_REG_COLOR_H_CNT)&0xffff,
                DISP_REG_GET(DISP_REG_COLOR_L_CNT)&0xffff);
            break;
        
        /* Dump DSI Reg */
        case DISP_MODULE_DSI_VDO:
        case DISP_MODULE_DSI_CMD:
            DISP_MSG("dsi: w=%d, h=%d, busy=%d, rdma_line_counter=%d, staus=%s, ",    		  
                (DISP_REG_GET(DISPSYS_DSI_BASE+0x1c)&0x3fff)/3, //bpp=3
                DISP_REG_GET(DISPSYS_DSI_BASE+0x2c),
                (DISP_REG_GET(DISPSYS_DSI_BASE+0xc)>>31)&0x1,
    		    DISP_REG_GET(DISPSYS_DSI_BASE+0x16c)&0x3fffff, 
    		    ddp_dsi_get_status(DISP_REG_GET(DISPSYS_DSI_BASE+0x160)&0xffff));
    		    ddp_dsi_get_err(DISP_REG_GET(DISPSYS_DSI_BASE+0x4));
            break;
            
        default:
            DISP_MSG("DDP error, dump_info unknow module=%d \n", module);
    }
    return 0;
}

