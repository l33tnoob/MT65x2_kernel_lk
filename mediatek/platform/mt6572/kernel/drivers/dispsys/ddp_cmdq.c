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
#include <linux/proc_fs.h>
#include <linux/xlog.h>

//ION
#include <linux/ion.h>
#include <linux/ion_drv.h>
#include <mach/m4u.h>


#include <asm/io.h>


#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_irq.h>
#include <mach/sync_write.h>
#include <mach/mt_smi.h>


#include "ddp_cmdq.h"
#include "ddp_drv.h"
#include "ddp_color.h"
#include "ddp_reg.h"
#include "ddp_path.h"

extern unsigned long * cmdq_pBuffer;
extern wait_queue_head_t cmq_wait_queue[CMDQ_THREAD_NUM];
//extern unsigned char cmq_status[CMDQ_THREAD_NUM];
extern unsigned int dbg_log;

cmdq_buff_t cmdqBufTbl[CMDQ_BUFFER_NUM];

int taskIDStatusTbl[MAX_CMDQ_TASK_ID];

task_resource_t taskResOccuTbl[MAX_CMDQ_TASK_ID]; //0~255

unsigned int hwResTbl = 0; //0~20 bit 0-> usable HW, 1-> in use
unsigned int cmdqThreadResTbl[CMDQ_THREAD_NUM];

unsigned int cmdqThreadTaskList_R[CMDQ_THREAD_NUM]; //Read pointer for each thread
unsigned int cmdqThreadTaskList_W[CMDQ_THREAD_NUM];//Write pointer for each thread
int cmdqThreadTaskList[CMDQ_THREAD_NUM][CMDQ_THREAD_LIST_LENGTH]; //each Thread's Current Job


// mdp error counter
int totalMdpResetCnt = 0;
int mdpMutexFailCnt = 0;
int mdpRdmaFailCnt = 0;
int mdpWrotFailCnt = 0;

// mdp RDMA register backup
int mdpRdmaPrevStatus[6];

// cmdq timeout counter
int totalTimeoutCnt = 0;
int totalExecCnt = 0;
int waitQueueTimeoutCnt = 0;
int cmdqDoneMdpBusyCnt = 0;
int cmdqIsrLossCnt = 0;
int mdpBusyLongCnt = 0;

// cmdq execution time monitor
struct timeval prevFrameExecTime;


spinlock_t gCmdqMgrLock;

#define CMDQ_WRN(string, args...) if(dbg_log) printk("[CMDQ]"string,##args)
//#define CMDQ_MSG(string, args...) printk(string,##args)
#define CMDQ_ERR(string, args...) if(1) printk("\n\n\n~~~~~~~~~Oops ![CMDQ] ERROR: "string,##args)
#define CMDQ_MSG(string, args...) if(dbg_log) printk(string,##args)
//#define CMDQ_ERR(string, args...) if(dbg_log) printk(string,##args)
#define CMDQ_IRQ(string, args...) if(dbg_log) printk("[CMDQ]"string,##args)

void cmdqForceFreeAll(int cmdqThread);
void cmdqForceFree_SW(int taskID);


#if 1   // move to proc
// Hardware Mutex Variables
#define ENGINE_MUTEX_NUM 8
static DEFINE_SPINLOCK(gMutexLock);
int mutex_used[ENGINE_MUTEX_NUM] = {1, 0, 1, 1, 0, 0, 0, 0};    // 0 for FB, 1 for Bitblt, 2 for HDMI, 3 for BLS
static DECLARE_WAIT_QUEUE_HEAD(gMutexWaitQueue);

extern DISPLAY_TDSHP_T *get_TDSHP_index(void);

typedef struct
{
    pid_t open_pid;
    pid_t open_tgid;
    unsigned int u4LockedMutex;
    spinlock_t node_lock;
} cmdq_proc_node_struct;

int disp_lock_mutex(void)
{
    int id = -1;
    int i;
    spin_lock(&gMutexLock);

    for(i = 0 ; i < ENGINE_MUTEX_NUM ; i++)
        if(mutex_used[i] == 0)
        {
            id = i;
            mutex_used[i] = 1;
            //DISP_REG_SET_FIELD((1 << i) , DISP_REG_CONFIG_MUTEX_INTEN , 1);
            break;
        }
    spin_unlock(&gMutexLock);

    return id;
}

int disp_unlock_mutex(int id)
{
    if(id < 0 && id >= ENGINE_MUTEX_NUM) 
        return -1;

    spin_lock(&gMutexLock);
    
    mutex_used[id] = 0;
    //DISP_REG_SET_FIELD((1 << id) , DISP_REG_CONFIG_MUTEX_INTEN , 0);
    
    spin_unlock(&gMutexLock);

    return 0;
}

static long cmdq_proc_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    DISP_EXEC_COMMAND cParams = {0};
    int mutex_id = 0;
    DISP_PQ_PARAM * pq_param = NULL;
    DISPLAY_TDSHP_T * tdshp_index = NULL;
    unsigned long flags = 0;
    int taskID = 0;
    cmdq_buff_t * pCmdqAddr = NULL;
    cmdq_proc_node_struct *pNode = (cmdq_proc_node_struct*)file->private_data;
    struct timeval tv;
    
    switch(cmd)
    {
        case DISP_IOCTL_RESOURCE_REQUIRE:

            CMDQ_MSG("\n=========DISP_IOCTL_RESOURCE_REQUIRE start!==========\n");

            spin_lock_irqsave(&gCmdqMgrLock,flags);
            taskID = cmdqResource_required();
            spin_unlock_irqrestore(&gCmdqMgrLock,flags);
            
            if (copy_to_user((void*)arg, &taskID , sizeof(int)))
            {
                CMDQ_ERR("DISP_IOCTL_RESOURCE_REQUIRE, copy_to_user failed\n");
                return -EFAULT;            
            }
  
            CMDQ_MSG("\n=========DISP_IOCTL_RESOURCE_REQUIRE taskID:%d==========\n",taskID);
            
            break;
            
        case DISP_IOCTL_EXEC_COMMAND:

            //CS++++++++++++++++++++++++++++++++++++++++++++++++++
    
           //spin_lock_irqsave(&gCmdqMgrLock,flags); 
            
            do_gettimeofday(&tv);

            if(copy_from_user(&cParams, (void *)arg, sizeof(DISP_EXEC_COMMAND)))
            {
                CMDQ_ERR("disp driver : Copy from user error\n");
           //     spin_unlock_irqrestore(&gCmdqMgrLock,flags);              
                return -EFAULT;
            }
            CMDQ_MSG(KERN_DEBUG "==========DISP_IOCTL_EXEC_COMMAND Task: %d start at SEC: %d MS: %d===========\n",cParams.taskID, (int)tv.tv_sec, (int)tv.tv_usec);

            //Get Related buffer VA/MVA

            pCmdqAddr = cmdqBufAddr(cParams.taskID);
            if(NULL == pCmdqAddr)
            {
                CMDQ_ERR("CmdQ buf address is NULL in DISP_IOCTL_EXEC_COMMAND ioctl\n");
                cmdqForceFree_SW(cParams.taskID);
                return -EFAULT;
            }

            CMDQ_MSG("CMDQ task %x buffer VA: 0x%x MVA: 0x%x \n", pCmdqAddr->Owner, (unsigned int)pCmdqAddr->VA, (unsigned int)pCmdqAddr->MVA);

            CMDQ_MSG(KERN_DEBUG "==========DISP_IOCTL_EXEC_COMMAND Params: FrameBase = 0x%08x, size = %x ===========\n",(unsigned int)cParams.pFrameBaseSW, cParams.blockSize);

            if (copy_from_user((unsigned long*)(pCmdqAddr->VA) ,cParams.pFrameBaseSW, cParams.blockSize))
            {
                CMDQ_ERR("disp driver : Copy from user error\n");
                return -EFAULT;
            }

            //spin_unlock_irqrestore(&gCmdqMgrLock,flags);

            //CS--------------------------------------------------
            
/*
            //ION Flush
*/
           // cmdq_ion_flush(); //FIXME!

            if (false == cmdqTaskAssigned(cParams.taskID, cParams.priority, cParams.engineFlag, cParams.blockSize))
            {
                do_gettimeofday(&tv);
                CMDQ_ERR(KERN_DEBUG "==========DISP_IOCTL_EXECUTE_COMMANDS Task: %d fail at SEC: %d MS: %d===========\n",cParams.taskID, (int)tv.tv_sec, (int)tv.tv_usec);
                return -EFAULT;
            }

            do_gettimeofday(&tv);
            CMDQ_MSG(KERN_DEBUG "==========DISP_IOCTL_EXEC_COMMAND Task: %d done at SEC: %d MS: %d===========\n",cParams.taskID, (int)tv.tv_sec, (int)tv.tv_usec);
            
            
            break;

            
        case DISP_IOCTL_LOCK_MUTEX:
        {
            wait_event_interruptible_timeout(
            gMutexWaitQueue, 
            (mutex_id = disp_lock_mutex()) != -1, 
            msecs_to_jiffies(200) );             

            if((-1) != mutex_id)
            {
                spin_lock(&pNode->node_lock);
                pNode->u4LockedMutex |= (1 << mutex_id);
                spin_unlock(&pNode->node_lock);
            }
            
            if(copy_to_user((void *)arg, &mutex_id, sizeof(int)))
            {
                CMDQ_ERR("disp driver : Copy to user error (mutex)\n");
                return -EFAULT;            
            }
            break;
        }
        case DISP_IOCTL_UNLOCK_MUTEX:
            if(copy_from_user(&mutex_id, (void*)arg , sizeof(int)))
            {
                CMDQ_ERR("DISP_IOCTL_UNLOCK_MUTEX, copy_from_user failed\n");
                return -EFAULT;
            }
            disp_unlock_mutex(mutex_id);

            if((-1) != mutex_id)
            {
                spin_lock(&pNode->node_lock);
                pNode->u4LockedMutex &= ~(1 << mutex_id);
                spin_unlock(&pNode->node_lock);
            }

            wake_up_interruptible(&gMutexWaitQueue);             

            break;
               
        case DISP_IOCTL_GET_TDSHPINDEX:
            // this is duplicated to disp_unlocked_ioctl
            // be careful when modify the definition
            tdshp_index = get_TDSHP_index();
            if(copy_to_user((void *)arg, tdshp_index, sizeof(DISPLAY_TDSHP_T)))
            {
                CMDQ_ERR("disp driver : DISP_IOCTL_GET_TDSHPINDEX Copy to user failed\n");
                return -EFAULT;            
            }  
            break;
            
        case DISP_IOCTL_GET_PQPARAM:
            // this is duplicated to cmdq_proc_unlocked_ioctl
            // be careful when modify the definition
            pq_param = get_Color_config();
            if(copy_to_user((void *)arg, pq_param, sizeof(DISP_PQ_PARAM)))
            {
                CMDQ_ERR("disp driver : DISP_IOCTL_GET_PQPARAM Copy to user failed\n");
                return -EFAULT;            
            }

            break;

        case DISP_IOCTL_GET_PQ_CAM_PARAM:
            // this is duplicated to disp_unlocked_ioctl
            // be careful when modify the definition
            pq_param = get_Color_Cam_config();
            if(copy_to_user((void *)arg, pq_param, sizeof(DISP_PQ_PARAM)))
            {
                CMDQ_ERR("disp driver : DISP_IOCTL_GET_PQ_CAM_PARAM Copy to user failed\n");
                return -EFAULT;            
            }
            
            break;        
       
        case DISP_IOCTL_GET_PQ_GAL_PARAM:
            // this is duplicated to disp_unlocked_ioctl
            // be careful when modify the definition
            pq_param = get_Color_Gal_config();
            if(copy_to_user((void *)arg, pq_param, sizeof(DISP_PQ_PARAM)))
            {
                CMDQ_ERR("disp driver : DISP_IOCTL_GET_PQ_GAL_PARAM Copy to user failed\n");
                return -EFAULT;            
            }
            
            break;        
            
        case DISP_IOCTL_SET_PQPARAM:
            {
                int ret;
                ret = disp_color_set_pq_param((void*)arg);

                if (ret < 0)
                {
                    return ret;
                }
            }
            break;
    }

    return 0;
}

static int cmdq_proc_open(struct inode *inode, struct file *file)
{
    cmdq_proc_node_struct *pNode = NULL;

    CMDQ_MSG("enter cmdq_proc_open() process:%s\n", current->comm);

    //Allocate and initialize private data
    file->private_data = kmalloc(sizeof(cmdq_proc_node_struct), GFP_ATOMIC);
    if(NULL == file->private_data)
    {
        CMDQ_MSG("Not enough entry for DDP open operation\n");
        return -ENOMEM;
    }
   
    pNode = (cmdq_proc_node_struct *)file->private_data;
    pNode->open_pid = current->pid;
    pNode->open_tgid = current->tgid;
    pNode->u4LockedMutex = 0;
    spin_lock_init(&pNode->node_lock);
    return 0;
}

static int cmdq_proc_release(struct inode *inode, struct file *file)
{
    cmdq_proc_node_struct *pNode = NULL;
    unsigned int index = 0;
    CMDQ_MSG("enter cmdq_proc_release() process:%s\n",current->comm);
    
    pNode = (cmdq_proc_node_struct *)file->private_data;

    spin_lock(&pNode->node_lock);

    if(pNode->u4LockedMutex)
    {
        CMDQ_ERR("Proccess terminated[Mutex] ! :%s , mutex:%u\n" 
            , current->comm , pNode->u4LockedMutex);

        for(index = 0 ; index < ENGINE_MUTEX_NUM ; index += 1)
        {
            if((1 << index) & pNode->u4LockedMutex)
            {
                disp_unlock_mutex(index);
                CMDQ_MSG("unlock index = %d ,mutex_used[ %d %d %d %d ]\n",index,mutex_used[0],mutex_used[1] ,mutex_used[2],mutex_used[3]);
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

static ssize_t cmdq_proc_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
    return 0;
}

static int cmdq_proc_flush(struct file * file , fl_owner_t a_id)
{
    return 0;
}

static struct file_operations cmdq_proc_fops = {
    .owner      = THIS_MODULE,
    .open		= cmdq_proc_open,
    .read       = cmdq_proc_read,
    .flush      = cmdq_proc_flush,
	.release	= cmdq_proc_release,
    .unlocked_ioctl = cmdq_proc_unlocked_ioctl,
};

#endif


void dumpDebugInfo(void)
{
    int i = 0;

    printk(KERN_DEBUG "\n\n\ncmdqTaskAssigned R: %d W: %d \n\n\n",cmdqThreadTaskList_R[0], cmdqThreadTaskList_W[0]);

    for(i=0;i<CMDQ_BUFFER_NUM;i++)
    {
        printk(KERN_DEBUG "cmdqBufTbl %x : [%x %x %x] \n",i ,cmdqBufTbl[i].Owner, (unsigned int)cmdqBufTbl[i].VA, (unsigned int)cmdqBufTbl[i].MVA);
    }

}

bool checkMdpEngineStatus(unsigned int engineFlag)
{
    if (engineFlag & (0x1 << tRDMA0))
    {
        if (0x100 != (DISP_REG_GET(0xF4001408) & 0x7FF00))
            return true;
    }

    if (engineFlag & (0x1 << tWROT))
    {
        DISP_REG_SET(0xF4005018, 0xB00);
        if (0x0 != (DISP_REG_GET(0xF40050D0) & 0x1F))
            return true;
    }

    if (engineFlag & (0x1 << tWDMA1))
    {
        if (0x1 != (DISP_REG_GET(0xF40040A0) & 0x3FF))
            return true;
    }

    return false;
}


void resetMdpEngine(unsigned int engineFlag)
{
    int loop_count = 0;

    totalMdpResetCnt++;
    if (engineFlag & (0x1 << tRDMA0))
    {
        DISP_REG_SET(0xF4001008, 0x1);
        while(loop_count <= 1000)
        {
            if (0x100 == (DISP_REG_GET(0xF4001408) & 0x7FF00))
                break;

            loop_count++;
        }
        DISP_REG_SET(0xF4001008, 0x0);
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        DISP_REG_SET(0xF4002000, 0x0);
        DISP_REG_SET(0xF4002000, 0x10000);
        DISP_REG_SET(0xF4002000, 0x0);
    }

    if (engineFlag & (0x1 << tSCL1))
    {
        DISP_REG_SET(0xF4003000, 0x0);
        DISP_REG_SET(0xF4003000, 0x10000);
        DISP_REG_SET(0xF4003000, 0x0);
    }

    if (engineFlag & (0x1 << tTDSHP))
    {
        DISP_REG_SET(0xF4006100, 0x0);
        DISP_REG_SET(0xF4006100, 0x2);
        DISP_REG_SET(0xF4006100, 0x0);
    }

    loop_count = 0;
    if (engineFlag & (0x1 << tWROT))
    {
        DISP_REG_SET(0xF4005010, 0x1);
        while(loop_count <= 1000)
        {
            if (0x0 == (DISP_REG_GET(0xF4005014) & 0x1))
                break;

            loop_count++;
        }
        DISP_REG_SET(0xF4005010, 0x0);
    }

    loop_count = 0;
    if (engineFlag & (0x1 << tWDMA1))
    {
        DISP_REG_SET(0xF400400C, 0x1);
        while(loop_count <= 1000)
        {
            if (0x1 == (DISP_REG_GET(0xF40040A0) & 0x3FF))
                break;

            loop_count++;
        }
        DISP_REG_SET(0xF400400C, 0x0);
    }
}


void dumpMDPRegInfo(void)
{
    int reg_temp1, reg_temp2, reg_temp3;

    printk(KERN_DEBUG "[CMDQ]RDMA_SRC_CON: 0x%08x, RDMA_SRC_BASE_0: 0x%08x, RDMA_MF_BKGD_SIZE_IN_BYTE: 0x%08x\n",
           DISP_REG_GET(0xF4001030),
           DISP_REG_GET(0xF4001040),
           DISP_REG_GET(0xF4001060));
    printk(KERN_DEBUG "[CMDQ]RDMA_MF_SRC_SIZE: 0x%08x, RDMA_MF_CLIP_SIZE: 0x%08x, RDMA_MF_OFFSET_1: 0x%08x\n",
           DISP_REG_GET(0xF4001070),
           DISP_REG_GET(0xF4001078),
           DISP_REG_GET(0xF4001080));
    printk(KERN_DEBUG "[CMDQ]RDMA_SRC_END_0: 0x%08x, RDMA_SRC_OFFSET_0: 0x%08x, RDMA_SRC_OFFSET_W_0: 0x%08x\n",
           DISP_REG_GET(0xF4001100),
           DISP_REG_GET(0xF4001118),
           DISP_REG_GET(0xF4001130));
    printk(KERN_DEBUG "[CMDQ]RDMA_MON_STA_0: 0x%08x, RDMA_MON_STA_1: 0x%08x, RDMA_MON_STA_2: 0x%08x\n",
           DISP_REG_GET(0xF4001400),
           DISP_REG_GET(0xF4001408),
           DISP_REG_GET(0xF4001410));
    printk(KERN_DEBUG "[CMDQ]RDMA_MON_STA_4: 0x%08x, RDMA_MON_STA_6: 0x%08x, RDMA_MON_STA_26: 0x%08x\n",
           DISP_REG_GET(0xF4001420),
           DISP_REG_GET(0xF4001430),
           DISP_REG_GET(0xF40014D0));

    printk(KERN_DEBUG "[CMDQ]WDMA_CFG: 0x%08x, WDMA_SRC_SIZE: 0x%08x, WDMA_DST_W_IN_BYTE = 0x%08x\n",
           DISP_REG_GET(0xF4004014),
           DISP_REG_GET(0xF4004018),
           DISP_REG_GET(0xF4004028));
    printk(KERN_DEBUG "[CMDQ]WDMA_DST_ADDR0: 0x%08x, WDMA_DST_UV_PITCH: 0x%08x, WDMA_DST_ADDR_OFFSET0 = 0x%08x\n",
           DISP_REG_GET(0xF4004024),
           DISP_REG_GET(0xF4004078),
           DISP_REG_GET(0xF4004080));
    printk(KERN_DEBUG "[CMDQ]WDMA_STATUS: 0x%08x, WDMA_INPUT_CNT: 0x%08x\n",
           DISP_REG_GET(0xF40040A0),
           DISP_REG_GET(0xF40040A8));

    printk(KERN_DEBUG "[CMDQ]VIDO_CTRL: 0x%08x, VIDO_MAIN_BUF_SIZE: 0x%08x, VIDO_SUB_BUF_SIZE: 0x%08x\n",
           DISP_REG_GET(0xF4005000),
           DISP_REG_GET(0xF4005008),
           DISP_REG_GET(0xF400500C));

    printk(KERN_DEBUG "[CMDQ]VIDO_TAR_SIZE: 0x%08x, VIDO_BASE_ADDR: 0x%08x, VIDO_OFST_ADDR: 0x%08x\n",
           DISP_REG_GET(0xF4005024),
           DISP_REG_GET(0xF4005028),
           DISP_REG_GET(0xF400502C));

    printk(KERN_DEBUG "[CMDQ]VIDO_DMA_PERF: 0x%08x, VIDO_STRIDE: 0x%08x, VIDO_IN_SIZE: 0x%08x\n",
           DISP_REG_GET(0xF4005004),
           DISP_REG_GET(0xF4005030),
           DISP_REG_GET(0xF4005078));

    DISP_REG_SET(0xF4005018, 0x00000100);
    reg_temp1 = DISP_REG_GET(0xF40050D0);
    DISP_REG_SET(0xF4005018, 0x00000200);
    reg_temp2 = DISP_REG_GET(0xF40050D0);
    DISP_REG_SET(0xF4005018, 0x00000300);
    reg_temp3 = DISP_REG_GET(0xF40050D0);
    printk(KERN_DEBUG "[CMDQ]VIDO_DBG1: 0x%08x, VIDO_DBG2: 0x%08x, VIDO_DBG3: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);

    DISP_REG_SET(0xF4005018, 0x00000500);
    reg_temp1 = DISP_REG_GET(0xF40050D0);
    DISP_REG_SET(0xF4005018, 0x00000800);
    reg_temp2 = DISP_REG_GET(0xF40050D0);
    DISP_REG_SET(0xF4005018, 0x00000B00);
    reg_temp3 = DISP_REG_GET(0xF40050D0);
    printk(KERN_DEBUG "[CMDQ]VIDO_DBG5: 0x%08x, VIDO_DBG8: 0x%08x, VIDO_DBGB: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);
}

void dumpRegDebugInfo(unsigned int engineFlag, int cmdqIndex, cmdq_buff_t bufferAddr)
{
    int reg_temp1, reg_temp2, reg_temp3;

    totalTimeoutCnt++;

    // dump MMSYS clock setting
    printk("[CMDQ]engineFlag = %x, MMSYS_CG_CON0 = 0x%08x\n", engineFlag, DISP_REG_GET(0xF4000100));

    // dump mutex status
    printk("[CMDQ]DISP_MUTEX_INTSTA = 0x%08x, DISP_REG_COMMIT = 0x%08x\n",
           DISP_REG_GET(0xF400E004),
           DISP_REG_GET(0xF400E00C));
    if (0 != DISP_REG_GET(0xF400E00C))
    {
        mdpMutexFailCnt++;
    }

    // dump current instruction
    reg_temp1 = DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqIndex));
    reg_temp2 = bufferAddr.VA + (reg_temp1 - bufferAddr.MVA);

    // dump CMDQ status
    printk("[CMDQ]CMDQ_THR%d_PC = 0x%08x, CMDQ_THR%d_END_ADDR = 0x%08x, CMDQ_THR%d_WAIT_TOKEN = 0x%08x\n",
           cmdqIndex, reg_temp1,
           cmdqIndex, DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqIndex)),
           cmdqIndex, DISP_REG_GET(DISP_REG_CMDQ_THRx_WAIT_EVENTS0(cmdqIndex)));

    printk("[CMDQ]CMD buffer VA=0x%08lx MVA=0x%08lx\n", 
           bufferAddr.VA,
           bufferAddr.MVA);

    if((bufferAddr.VA <= reg_temp2) && ((bufferAddr.VA + bufferAddr.blocksize) >= reg_temp2))
    {
        // dump current instruction
        if (reg_temp2 != (bufferAddr.VA + bufferAddr.blocksize))
        {
            if (reg_temp2 >= (bufferAddr.VA + 8))
            {
                printk("[CMDQ]CMDQ current inst0 = 0x%08x0x%08x, inst1 = 0x%08x0x%08x\n",
                       DISP_REG_GET(reg_temp2-8),
                       DISP_REG_GET(reg_temp2-4),
                       DISP_REG_GET(reg_temp2),
                       DISP_REG_GET(reg_temp2+4));
            }
            else
            {
                // first instruction
                printk("[CMDQ]CMDQ current inst1 = 0x%08x0x%08x\n",
                       DISP_REG_GET(reg_temp2),
                       DISP_REG_GET(reg_temp2+4));
            }
        }
        else
        {
            printk("[CMDQ]CMDQ current inst0 = 0x%08x0x%08x, inst1 = 0x%08x0x%08x\n",
                   DISP_REG_GET(reg_temp2-16),
                   DISP_REG_GET(reg_temp2-12),
                   DISP_REG_GET(reg_temp2-8),
                   DISP_REG_GET(reg_temp2-4));
        }
    }

    // dump RDMA debug registers
    if (engineFlag & (0x1 << tRDMA0))
    {
        printk("[CMDQ]RDMA_SRC_CON: 0x%08x, RDMA_SRC_BASE_0: 0x%08x, RDMA_MF_BKGD_SIZE_IN_BYTE: 0x%08x\n",
               DISP_REG_GET(0xF4001030),
               DISP_REG_GET(0xF4001040),
               DISP_REG_GET(0xF4001060));
        printk("[CMDQ]RDMA_MF_SRC_SIZE: 0x%08x, RDMA_MF_CLIP_SIZE: 0x%08x, RDMA_MF_OFFSET_1: 0x%08x\n",
               DISP_REG_GET(0xF4001070),
               DISP_REG_GET(0xF4001078),
               DISP_REG_GET(0xF4001080));
        printk("[CMDQ]RDMA_SRC_END_0: 0x%08x, RDMA_SRC_OFFSET_0: 0x%08x, RDMA_SRC_OFFSET_W_0: 0x%08x\n",
               DISP_REG_GET(0xF4001100),
               DISP_REG_GET(0xF4001118),
               DISP_REG_GET(0xF4001130));
        printk("[CMDQ](R)RDMA_MON_STA_0: 0x%08x, RDMA_MON_STA_1: 0x%08x, RDMA_MON_STA_2: 0x%08x\n",
               mdpRdmaPrevStatus[0],
               mdpRdmaPrevStatus[1],
               mdpRdmaPrevStatus[2]);
        printk("[CMDQ](R)RDMA_MON_STA_4: 0x%08x, RDMA_MON_STA_6: 0x%08x, RDMA_MON_STA_26: 0x%08x\n",
               mdpRdmaPrevStatus[3],
               mdpRdmaPrevStatus[4],
               mdpRdmaPrevStatus[5]);
        printk("[CMDQ]RDMA_MON_STA_0: 0x%08x, RDMA_MON_STA_1: 0x%08x, RDMA_MON_STA_2: 0x%08x\n",
               DISP_REG_GET(0xF4001400),
               DISP_REG_GET(0xF4001408),
               DISP_REG_GET(0xF4001410));
        printk("[CMDQ]RDMA_MON_STA_4: 0x%08x, RDMA_MON_STA_6: 0x%08x, RDMA_MON_STA_26: 0x%08x\n",
               DISP_REG_GET(0xF4001420),
               DISP_REG_GET(0xF4001430),
               DISP_REG_GET(0xF40014D0));

        if (0x100 != (DISP_REG_GET(0xF4001408) & 0x100))
        {
            mdpRdmaFailCnt++;
        }
    }

    // dump RSZ debug registers
    if (engineFlag & (0x1 << tSCL0))
    {
        DISP_REG_SET(0xF4002040, 0x00000003);
        reg_temp1 = DISP_REG_GET(0xF4002044);

        printk("[CMDQ]RSZ0_CFG: 0x%08x, RSZ0_INPUT_CNT: 0x%08x, RSZ0_HORIZONTAL_COEFF_STEP = 0x%08x\n",
               DISP_REG_GET(0xF4002004),
               reg_temp1,
               DISP_REG_GET(0xF4002014));

        printk("[CMDQ]RSZ0_IN_SIZE: 0x%08x, RSZ0_OUT_SIZE: 0x%08x, RSZ0_VERTICAL_COEFF_STEP = 0x%08x\n",
               DISP_REG_GET(0xF400200C),
               DISP_REG_GET(0xF4002010),
               DISP_REG_GET(0xF4002018));
    }

    if (engineFlag & (0x1 << tSCL1))
    {
        DISP_REG_SET(0xF4003040, 0x00000003);
        reg_temp1 = DISP_REG_GET(0xF4003044);

        printk("[CMDQ]RSZ1_CFG: 0x%08x, RSZ1_INPUT_CNT: 0x%08x, RSZ1_HORIZONTAL_COEFF_STEP = 0x%08x\n",
               DISP_REG_GET(0xF4003004),
               reg_temp1,
               DISP_REG_GET(0xF4003014));

        printk("[CMDQ]RSZ1_IN_SIZE: 0x%08x, RSZ1_OUT_SIZE: 0x%08x, RSZ1_VERTICAL_COEFF_STEP = 0x%08x\n",
               DISP_REG_GET(0xF400300C),
               DISP_REG_GET(0xF4003010),
               DISP_REG_GET(0xF4003018));
    }

    // dump WDMA debug registers
    if (engineFlag & (0x1 << tWDMA1))
    {
        printk("[CMDQ]WDMA_CFG: 0x%08x, WDMA_SRC_SIZE: 0x%08x, WDMA_DST_W_IN_BYTE = 0x%08x\n",
               DISP_REG_GET(0xF4004014),
               DISP_REG_GET(0xF4004018),
               DISP_REG_GET(0xF4004028));
        printk("[CMDQ]WDMA_DST_ADDR0: 0x%08x, WDMA_DST_UV_PITCH: 0x%08x, WDMA_DST_ADDR_OFFSET0 = 0x%08x\n",
               DISP_REG_GET(0xF4004024),
               DISP_REG_GET(0xF4004078),
               DISP_REG_GET(0xF4004080));
        printk("[CMDQ]WDMA_STATUS: 0x%08x, WDMA_INPUT_CNT: 0x%08x\n",
               DISP_REG_GET(0xF40040A0),
               DISP_REG_GET(0xF40040A8));
    }

    // dump WROT debug registers
    if (engineFlag & (0x1 << tWROT))
    {
        printk("[CMDQ]VIDO_CTRL: 0x%08x, VIDO_MAIN_BUF_SIZE: 0x%08x, VIDO_SUB_BUF_SIZE: 0x%08x\n",
               DISP_REG_GET(0xF4005000),
               DISP_REG_GET(0xF4005008),
               DISP_REG_GET(0xF400500C));

        printk("[CMDQ]VIDO_TAR_SIZE: 0x%08x, VIDO_BASE_ADDR: 0x%08x, VIDO_OFST_ADDR: 0x%08x\n",
               DISP_REG_GET(0xF4005024),
               DISP_REG_GET(0xF4005028),
               DISP_REG_GET(0xF400502C));

        printk("[CMDQ]VIDO_DMA_PERF: 0x%08x, VIDO_STRIDE: 0x%08x, VIDO_IN_SIZE: 0x%08x\n",
               DISP_REG_GET(0xF4005004),
               DISP_REG_GET(0xF4005030),
               DISP_REG_GET(0xF4005078));

        DISP_REG_SET(0xF4005018, 0x00000100);
        reg_temp1 = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000200);
        reg_temp2 = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000300);
        reg_temp3 = DISP_REG_GET(0xF40050D0);
        printk("[CMDQ]VIDO_DBG1: 0x%08x, VIDO_DBG2: 0x%08x, VIDO_DBG3: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);

        DISP_REG_SET(0xF4005018, 0x00000500);
        reg_temp1 = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000800);
        reg_temp2 = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000B00);
        reg_temp3 = DISP_REG_GET(0xF40050D0);
        printk("[CMDQ]VIDO_DBG5: 0x%08x, VIDO_DBG8: 0x%08x, VIDO_DBGB: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);

        if (0x0 != (reg_temp3 & 0x1F))
        {
            mdpWrotFailCnt++;
        }
    }
}

void cmdqBufferTbl_init(unsigned long va_base, unsigned long mva_base)
{
    int i = 0;

    spin_lock(&gCmdqMgrLock);

    for(i=0;i<CMDQ_BUFFER_NUM;i++)
    {
        cmdqBufTbl[i].Owner = -1; //free buffer
        cmdqBufTbl[i].VA = va_base + (i*CMDQ_BUFFER_SIZE);
        cmdqBufTbl[i].MVA =  mva_base + (i*CMDQ_BUFFER_SIZE);

        CMDQ_MSG("cmdqBufferTbl_init %x : [%x %x %x] \n",i ,cmdqBufTbl[i].Owner, (unsigned int)cmdqBufTbl[i].VA, (unsigned int)cmdqBufTbl[i].MVA);
    }

    for(i=0;i<MAX_CMDQ_TASK_ID;i++)
    {
        taskIDStatusTbl[i] = -1; //mark as free ID
        taskResOccuTbl[i].cmdBufID = -1;
        taskResOccuTbl[i].cmdqThread= -1;
    }

    for(i=0;i<CMDQ_THREAD_NUM;i++)
    {
        cmdqThreadResTbl[i] = 0;
        cmdqThreadTaskList_R[i] = 0;
        cmdqThreadTaskList_W[i] = 0;
    }


    spin_unlock(&gCmdqMgrLock);

}

int cmdqResource_required(void)
{
    int i = 0;
    int assignedTaskID = -1;

//    spin_lock(&gCmdqMgrLock);

    //Find free ID
    for(i=0;i<MAX_CMDQ_TASK_ID;i++)
    {
        if(taskIDStatusTbl[i]==-1)
        {
            assignedTaskID = i;
            taskIDStatusTbl[assignedTaskID] = 1; //mark as occupied
            break;
        }
    }

    if(assignedTaskID == -1)
    {
        CMDQ_ERR("No useable ID !!!\n");
        dumpDebugInfo();
        return -1;
    }
    //Find free Buffer
    for(i=0;i<CMDQ_BUFFER_NUM;i++)
    {
        if(cmdqBufTbl[i].Owner == -1)
        {
            cmdqBufTbl[i].Owner = assignedTaskID;
            taskResOccuTbl[assignedTaskID].cmdBufID = i;

            CMDQ_MSG("\n=========CMDQ Buffer %x is owned by %x==========\n", taskResOccuTbl[assignedTaskID].cmdBufID, cmdqBufTbl[i].Owner);
            break;
        }
    }

    //DEBUG
    //dumpDebugInfo();

    if(taskResOccuTbl[assignedTaskID].cmdBufID == -1)
    {
        CMDQ_ERR("No Free Buffer !!! Total reset CMDQ Driver\n");
        dumpDebugInfo();
        //taskIDStatusTbl[assignedTaskID] = -1; //return ID, resource allocation fail
        cmdqForceFreeAll(0);

        for(i=0;i<MAX_CMDQ_TASK_ID;i++)
        {
            taskIDStatusTbl[i] = -1; //mark as CANCEL
            taskResOccuTbl[i].cmdBufID = -1;
            taskResOccuTbl[i].cmdqThread= -1;
        }
        return -1;
    }

//    spin_unlock(&gCmdqMgrLock);

    return assignedTaskID;
}


void cmdqResource_free(int taskID)
{
    int bufID = -1;

    //spin_lock(&gCmdqMgrLock);

    if(taskID == -1 ||taskID>=MAX_CMDQ_TASK_ID)
    {
        CMDQ_ERR("\n=================Free Invalid Task ID================\n");
        dumpDebugInfo();
        return;
    }

    bufID = taskResOccuTbl[taskID].cmdBufID;

    CMDQ_MSG("\n=============Free Buf %x own by [%x=%x]===============\n",bufID,taskID,cmdqBufTbl[bufID].Owner);

    if(bufID != -1) //Free All resource and return ID
    {
        taskIDStatusTbl[taskID] = 3; //mark for complete
        taskResOccuTbl[taskID].cmdBufID = -1;
        taskResOccuTbl[taskID].cmdqThread= -1;
        cmdqBufTbl[bufID].Owner = -1;
    }
    else
    {
        CMDQ_ERR("\n=================Free Invalid Buffer ID================\n");
        dumpDebugInfo();
    }

    //spin_unlock(&gCmdqMgrLock);

}


cmdq_buff_t * cmdqBufAddr(int taskID)
{
    int bufID = -1;

    if((-1 == taskID) || (taskID >= MAX_CMDQ_TASK_ID))
    {
        CMDQ_ERR("cmdqBufAddr Invalid ID %d\n", taskID);
        return NULL;
    }

    bufID = taskResOccuTbl[taskID].cmdBufID;

    if((CMDQ_BUFFER_NUM < bufID) || (bufID < 0))
    {
        return NULL;
    }

    return &cmdqBufTbl[bufID];

}


void cmdqHwClockOn(unsigned int engineFlag, bool firstTask)
{
    //Start! Power on clock!
    //M4U

    if(firstTask)
    {
        //larb_clock_on(0, "MDP");
    // enable_clock(MT_CG_MM_CMDQ_SW_CG, "MM_CMDQ");
    // enable_clock(MT_CG_MM_CMDQ_SMI_IF_SW_CG, "MM_CMDQ_SMI_IF");
    //MDP
    }

    if (engineFlag & (0x1 << tRDMA0))
    {
        if(clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        {
            CMDQ_ERR("RDMA is already on , enable twice , 0x%x\n" , engineFlag);
        }
        else
        {
            enable_clock(MT_CG_MDP_RDMA_SW_CG, "MDP_RDMA");
        }
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        if(clock_is_on(MT_CG_MDP_RSZ0_SW_CG))
        {
            CMDQ_ERR("SCL0 is already on , enable twice , 0x%x\n" , engineFlag);
        }
        else
        {
            enable_clock(MT_CG_MDP_RSZ0_SW_CG, "MDP_RSZ0");
        }
    }

    if (engineFlag & (0x1 << tSCL1))
    {
        if(clock_is_on(MT_CG_MDP_RSZ1_SW_CG))
        {
            CMDQ_ERR("SCL1 is already on , enable twice , 0x%x\n" , engineFlag);
        }
        else
        {
            enable_clock(MT_CG_MDP_RSZ1_SW_CG, "MDP_RSZ1");
        }
    }

    if (engineFlag & (0x1 << tTDSHP))
    {
        if(clock_is_on(MT_CG_MDP_TDSHP_SW_CG))
        {
            CMDQ_ERR("TDSHP is already on , enable twice , 0x%x\n" , engineFlag);
        }
        else
        {
            enable_clock(MT_CG_MDP_TDSHP_SW_CG, "MDP_TDSHP");
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        if(clock_is_on(MT_CG_MDP_WROT_SW_CG))
        {
            CMDQ_ERR("WROT is already on , enable twice , 0x%x\n" , engineFlag);
        }
        else
        {
            enable_clock(MT_CG_MDP_WROT_SW_CG, "MDP_WROT");
        }
    }

    if (engineFlag & (0x1 << tWDMA1))
    {
        if(clock_is_on(MT_CG_MDP_WDMA_SW_CG))
        {
            CMDQ_ERR("WDMA1 is already on , enable twice , 0x%x\n" , engineFlag);
        }
        else
        {
            enable_clock(MT_CG_MDP_WDMA_SW_CG, "MDP_WDMA");
        }
    }

    //printk("\n\n\n=========== Power On %x ==============\n\n\n",engineFlag);
}

void cmdqHwClockOff(unsigned int engineFlag)
{
    //Finished! Power off clock!

    //MDP
    if(engineFlag & (0x1 << tRDMA0))
    {
        if(clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        {
            disable_clock(MT_CG_MDP_RDMA_SW_CG, "MDP_RDMA");
        }
        else
        {
            CMDQ_ERR("RDMA is already off , disable twice , 0x%x\n" , engineFlag);
        }
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        if(clock_is_on(MT_CG_MDP_RSZ0_SW_CG))
        {
            disable_clock(MT_CG_MDP_RSZ0_SW_CG, "MDP_RSZ0");
        }
        else
        {
            CMDQ_ERR("SCL0 is already off , disable twice , 0x%x\n" , engineFlag);
        }
    }

    if (engineFlag & (0x1 << tSCL1))
    {
        if(clock_is_on(MT_CG_MDP_RSZ1_SW_CG))
        {
            disable_clock(MT_CG_MDP_RSZ1_SW_CG, "MDP_RSZ1");
        }
        else
        {
            CMDQ_ERR("SCL1 is already off , disable twice , 0x%x\n" , engineFlag);
        }
    }

    if (engineFlag & (0x1 << tTDSHP))
    {
        if(clock_is_on(MT_CG_MDP_TDSHP_SW_CG))
        {
            disable_clock(MT_CG_MDP_TDSHP_SW_CG, "MDP_TDSHP");
        }
        else
        {
            CMDQ_ERR("TDSHP is already off , disable twice, 0x%x\n" , engineFlag);
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        if(clock_is_on(MT_CG_MDP_WROT_SW_CG))
        {
            disable_clock(MT_CG_MDP_WROT_SW_CG, "MDP_WROT");
        }
        else
        {
            CMDQ_ERR("TDSHP is already off , disable twice, 0x%x\n" , engineFlag);
        }
    }

    if (engineFlag & (0x1 << tWDMA1))
    {
        if(clock_is_on(MT_CG_MDP_WDMA_SW_CG))
        {
            disable_clock(MT_CG_MDP_WDMA_SW_CG, "MDP_WDMA");
        }
        else
        {
            CMDQ_ERR("WDMA is already off , disable twice,0x%x\n" , engineFlag);
        }
    }

    // disable_clock(MT_CG_MM_CMDQ_SW_CG, "MM_CMDQ");
    // disable_clock(MT_CG_MM_CMDQ_SMI_IF_SW_CG, "MM_CMDQ_SMI_IF");

    //M4U
    //larb_clock_off(0, "MDP");

    //printk("\n\n\n===========Power Off %x ==============\n\n\n",engineFlag);
}

bool cmdqTaskAssigned(int taskID, unsigned int priority, unsigned int engineFlag, unsigned int blocksize)
{
    int i = 0;
    int cmdqThread = -1;
    cmdq_buff_t * pCmdqAddr = NULL;
    unsigned long flags;
    unsigned long ins_leng = 0;
    unsigned long *cmdq_pc_head = 0;
    int buf_id;
    int pre_task_id;
    int pre_w_ptr;
    cmdq_buff_t * pPre_cmdqAddr = NULL;
    bool ret = true;
    int cmdq_polling_timeout = 0;
    long wq_ret = 0;
    struct timeval start_t, end_t;


    ins_leng = blocksize>>2; //CMDQ instruction: 4 byte

    totalExecCnt++;
    CMDQ_MSG("CMDQ_INFO: tExec: %d, tTimeout: %d, cmdqDoneMdpBusy: %d, waitQ: %d, cmdqIsr: %d, mdpBusyL: %d\n",
           totalExecCnt, totalTimeoutCnt, cmdqDoneMdpBusyCnt, waitQueueTimeoutCnt, cmdqIsrLossCnt, mdpBusyLongCnt);
    CMDQ_MSG("CMDQ_INFO: prevExec: %d us, MdpInfo: mdpMutex: %d, mdpRdma: %d, mdpWrot: %d, mdpReset: %d\n",
           (int)prevFrameExecTime.tv_usec, mdpMutexFailCnt, mdpRdmaFailCnt, mdpWrotFailCnt, totalMdpResetCnt);

    //CS++++++++++++++++++++++++++++++++++++++++++++++++++
    spin_lock_irqsave(&gCmdqMgrLock,flags);

    do_gettimeofday(&start_t);

    CMDQ_MSG("\n\n\n==============cmdqTaskAssigned  %d %d %x %d =============\n\n\n", taskID, priority, engineFlag, blocksize);

    if((engineFlag & hwResTbl) == 0) //Free HW available
    {

        for(i=0;i<CMDQ_THREAD_NUM;i++) //Find new free thread
        {
            if(cmdqThreadResTbl[i] == 0)
            {
                cmdqThread = i;
                break;
            }
        }

        if(cmdqThread != -1)
        {
            cmdqThreadResTbl[cmdqThread] = engineFlag;
            taskResOccuTbl[taskID].cmdqThread = cmdqThread;
        }
        else
        {
            CMDQ_ERR("Cannot find CMDQ thread\n");
            cmdqForceFree_SW(taskID);
            spin_unlock_irqrestore(&gCmdqMgrLock,flags);
            return false;
        }

        //Update HE resource TBL
        hwResTbl |= engineFlag;


        //Get Buffer info
        pCmdqAddr = cmdqBufAddr(taskID); //new Thread, current taskID must be first
        if(NULL == pCmdqAddr)
        {
            CMDQ_ERR("CmdQ buf address is NULL\n");
            cmdqForceFree_SW(taskID);
            spin_unlock_irqrestore(&gCmdqMgrLock,flags);
            return false;
        }

        //Insert job to CMDQ thread
        cmdqThreadTaskList[cmdqThread][cmdqThreadTaskList_W[cmdqThread]] = taskID; //assign task to T' write pointer
        cmdqThreadTaskList_W[cmdqThread] = (cmdqThreadTaskList_W[cmdqThread] + 1) % CMDQ_THREAD_LIST_LENGTH; //increase write pointer


        CMDQ_MSG("\n\n\ncmdqTaskAssigned R: %d W: %d \n\n\n",cmdqThreadTaskList_R[cmdqThread], cmdqThreadTaskList_W[cmdqThread]);

       // CMDQ_MSG("======cmdqTaskAssigned====\n");
       // dumpDebugInfo();
       // CMDQ_MSG("======cmdqTaskAssigned====\n");

        //Update CMDQ buffer parameter (CMDQ size / tail pointer)
        buf_id = taskResOccuTbl[taskID].cmdBufID;
        cmdq_pc_head = (unsigned long *)pCmdqAddr->VA;
        cmdqBufTbl[buf_id].blocksize = blocksize;
        cmdqBufTbl[buf_id].blockTailAddr = (cmdq_pc_head+ins_leng-1);

        //DBG message
        CMDQ_MSG("==========DISP_IOCTL_EXEC_COMMAND Task: %d ,Thread: %d, PC[0x%x], EOC[0x%x] =========\n",taskID, cmdqThread,(unsigned int)pCmdqAddr->MVA ,(unsigned int)(pCmdqAddr->MVA) + cmdqBufTbl[buf_id].blocksize);

        //Start! Power on(TODO)!

        cmdqHwClockOn(cmdqThreadResTbl[cmdqThread], true);

        // record RDMA status before start to run next frame
        mdpRdmaPrevStatus[0] = DISP_REG_GET(0xF4001400);
        mdpRdmaPrevStatus[1] = DISP_REG_GET(0xF4001408);
        mdpRdmaPrevStatus[2] = DISP_REG_GET(0xF4001410);
        mdpRdmaPrevStatus[3] = DISP_REG_GET(0xF4001420);
        mdpRdmaPrevStatus[4] = DISP_REG_GET(0xF4001430);
        mdpRdmaPrevStatus[5] = DISP_REG_GET(0xF40014D0);

        // enable CMDQ interrupt and set timeout cycles
        DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);

        DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(cmdqThread),0x1);  //Enable Each IRQ

        DISP_REG_SET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(cmdqThread), CMDQ_TIMEOUT);  //Set time out IRQ: 2^20 cycle

        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread),1);
        cmdq_polling_timeout = 0;
        while((DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(cmdqThread))&0x2) == 0)
        {
            cmdq_polling_timeout++;
            if(cmdq_polling_timeout>1000)
            {
                break;
            }
        }

        //Execuction
        DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
        //printk("1 Set PC to 0x%x",pCmdqAddr->MVA);
        DISP_REG_SET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread), pCmdqAddr->MVA + cmdqBufTbl[buf_id].blocksize);

        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread),0);



        spin_unlock_irqrestore(&gCmdqMgrLock,flags);
        //CS--------------------------------------------------

//Schedule out
        //wait_event_interruptible(cmq_wait_queue[cmdqThread], (taskIDStatusTbl[taskID] == -1));
        wq_ret = wait_event_interruptible_timeout(cmq_wait_queue[cmdqThread], (taskIDStatusTbl[taskID] == 3), HZ/5);
        smp_rmb();

        do_gettimeofday(&end_t);

//Clear Status
        spin_lock_irqsave(&gCmdqMgrLock,flags);

        if(wq_ret == 0)//if(taskIDStatusTbl[taskID] != -1)
        {
            CMDQ_ERR("A Task %d [%d] Wait CMDQ interrupt time out, PC: 0x%x 0x%x\n",taskID, taskIDStatusTbl[taskID], DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread)), DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)));

            dumpRegDebugInfo(cmdqThreadResTbl[cmdqThread], cmdqThread, *pCmdqAddr);

            //Warm reset CMDQ!

            // CMDQ FD interrupt received, but wait queue timeout
            if(taskIDStatusTbl[taskID] == 3)
            {
                waitQueueTimeoutCnt++;
                CMDQ_ERR("CPU busy->ISR already come but thread timeout\n");
            }
            else // True Timeout
            {
                if (DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread))==DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)))
                {
                    cmdqIsrLossCnt++;
                }
                else
                {
                    mdpBusyLongCnt++;
                }

                CMDQ_ERR("CMDQ execution failed, thread timeout\n");
                ret = false;

                if(taskIDStatusTbl[taskID]==2)
                {
                    CMDQ_ERR("Time out, but already free all by previous\n");
                }
                else
                {
                    cmdqForceFreeAll(cmdqThread);
                }

            }

        }

        taskIDStatusTbl[taskID] = -1; //free at last

        spin_unlock_irqrestore(&gCmdqMgrLock,flags);

    }
    else // no free HW
    {

 //       //CS++++++++++++++++++++++++++++++++++++++++++++++++++
 //       spin_lock_irqsave(&gCmdqMgrLock,flags);


        CMDQ_MSG("======CMDQ: No Free HW resource====\n");

         // enable CMDQ interrupt and set timeout cycles
        DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(cmdqThread),0x1);  //Enable Each IRQ
        DISP_REG_SET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(cmdqThread), CMDQ_TIMEOUT);  //Set time out IRQ: 2^20 cycle

        //Find Match HW in CMDQ
        for(i=0;i<CMDQ_THREAD_NUM;i++) //Find new free thread
        {
            //CMDQ_ERR("Findind....ThreadRes %x  engineFlag %x=========\n",cmdqThreadResTbl[i] , engineFlag);
            if(cmdqThreadResTbl[i] == engineFlag) //Use Same HW
            {
                cmdqThread = i;


                if(cmdqThreadTaskList_W[cmdqThread]==0)
                    pre_w_ptr = (CMDQ_THREAD_LIST_LENGTH - 1); //round
                else
                    pre_w_ptr = cmdqThreadTaskList_W[cmdqThread]-1;

                 pre_task_id = cmdqThreadTaskList[cmdqThread][pre_w_ptr];

                //Get Buffer info
                pCmdqAddr = cmdqBufAddr(taskID);
                pPre_cmdqAddr = cmdqBufAddr(pre_task_id);
                if((NULL == pCmdqAddr) || (NULL == pPre_cmdqAddr))
                {
                    CMDQ_ERR("CmdQ buf address is NULL , CMDQ 0x%x , PRECMDQ 0x%x\n" , (unsigned int)pCmdqAddr , (unsigned int)pPre_cmdqAddr);
                    cmdqForceFree_SW(taskID);
                    spin_unlock_irqrestore(&gCmdqMgrLock,flags);
                    return false;
                }

                if(DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)) == 0)
                {
                        //Warm reset
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
                        //PC reset
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
                        //printk("2 Set PC to 0x%x",pCmdqAddr->MVA);
                        //EN
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
                }
                else
                {
                    DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread),1);
                    cmdq_polling_timeout = 0;
                    while((DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(cmdqThread))&0x2) == 0)
                    {
                        cmdq_polling_timeout++;
                        if(cmdq_polling_timeout>1000)
                        {
                            CMDQ_ERR("CMDQ SUSPEND fail!%x %x %x %x\n", taskID ,DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x78), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x7c));
                            //Warm reset
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
                            //PC reset
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
                            //printk("3 Set PC to 0x%x",pCmdqAddr->MVA);
                            //EN
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
                            break;
                        }
                    }
                }
                CMDQ_MSG("PC: 0x%x EOC: 0x%x\n", (unsigned int)(DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread))), (unsigned int)(DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread))));
                CMDQ_MSG("======CMDQ: Task %d [%x] Find Matched Thread: %d [%x] and suspend====\n",taskID,engineFlag,cmdqThread, cmdqThreadResTbl[cmdqThread]);
                break;
            }
            else if((cmdqThreadResTbl[i] & engineFlag) != 0) //Overlaped HW
            {
                cmdqThread = i;

                if(cmdqThreadTaskList_W[cmdqThread]==0)
                    pre_w_ptr = (CMDQ_THREAD_LIST_LENGTH - 1); //round
                else
                    pre_w_ptr = cmdqThreadTaskList_W[cmdqThread]-1;

                 pre_task_id = cmdqThreadTaskList[cmdqThread][pre_w_ptr];

                //Get Buffer info
                pCmdqAddr = cmdqBufAddr(taskID);
                pPre_cmdqAddr = cmdqBufAddr(pre_task_id);
                if((NULL == pCmdqAddr) || (NULL == pPre_cmdqAddr))
                {
                    CMDQ_ERR("CmdQ buf address is NULL , CMDQ 0x%x , PRECMDQ 0x%x\n" , (unsigned int)pCmdqAddr , (unsigned int)pPre_cmdqAddr);
                    cmdqForceFree_SW(taskID);
                    spin_unlock_irqrestore(&gCmdqMgrLock,flags);
                    return false;
                }

                if(DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)) == 0)
                {
                        //Warm reset
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
                        //PC reset
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
                        //printk("4 Set PC to 0x%x",pCmdqAddr->MVA);
                        //EN
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
                }
                else
                {
                    DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread),1);
                    cmdq_polling_timeout = 0;
                    while((DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(cmdqThread))&0x2) == 0)
                    {
                        cmdq_polling_timeout++;
                        if(cmdq_polling_timeout>1000)
                        {
                            CMDQ_ERR("CMDQ SUSPEND fail!%x %x %x %x\n", taskID ,DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x78), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x7c));
                            //Warm reset
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
                            //PC reset
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
                            //printk("5 Set PC to 0x%x",pCmdqAddr->MVA);
                            //EN
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
                            break;
                        }
                    }
                }


                CMDQ_MSG("PC: 0x%x EOC: 0x%x\n", (unsigned int)(DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread))), (unsigned int)(DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread))));

                CMDQ_MSG("======CMDQ: Task %d [%x] Find Corresponding Thread: %d [%x] and suspend====\n",taskID,engineFlag,cmdqThread, cmdqThreadResTbl[cmdqThread]);
                if((engineFlag&~(cmdqThreadResTbl[i]))!=0) //More Engine then Current CMDQ T
                {
                   //POWER ON! (TODO)
                    cmdqHwClockOn((engineFlag&~(cmdqThreadResTbl[i])),false);
                   //Update CMDQ Thread Table
                   cmdqThreadResTbl[i] |= engineFlag;
                   //Update HE resource TBL
                   hwResTbl |= engineFlag;

                   CMDQ_MSG("========update CMDQ T %d resource %x=======\n",cmdqThread, cmdqThreadResTbl[cmdqThread]);
                }
                break;

            }
        }

        if(cmdqThread == -1) //cannot find any thread
        {
            CMDQ_ERR("=========CMDQ Job append Error happen!! %x %x=================\n",engineFlag, hwResTbl);

            if((engineFlag & hwResTbl) == 0) //Free HW available
            {
                CMDQ_ERR("=========DAMN!!!!!!!!=================\n");
            }
            for(i=0;i<CMDQ_THREAD_NUM;i++) //Find new free thread
            {
                CMDQ_ERR("ThreadRes %x  engineFlag %x\n",cmdqThreadResTbl[i] , engineFlag);
            }
            cmdqForceFree_SW(taskID);
            spin_unlock_irqrestore(&gCmdqMgrLock,flags);
            return false;
        }

        //Insert job to CMDQ thread


        cmdqThreadTaskList[cmdqThread][cmdqThreadTaskList_W[cmdqThread]] = taskID; //assign task to T' write pointer
        cmdqThreadTaskList_W[cmdqThread] = (cmdqThreadTaskList_W[cmdqThread] + 1) % CMDQ_THREAD_LIST_LENGTH; //increase write pointer

        CMDQ_MSG("\n\n\ncmdqTaskAssigned(Insert) R: %d W: %d \n\n\n",cmdqThreadTaskList_R[cmdqThread], cmdqThreadTaskList_W[cmdqThread]);


        //Update CMDQ buffer parameter (CMDQ size / tail pointer)
        buf_id = taskResOccuTbl[taskID].cmdBufID;
        cmdq_pc_head = (unsigned long *)pCmdqAddr->VA;
        cmdqBufTbl[buf_id].blocksize = blocksize;
        cmdqBufTbl[buf_id].blockTailAddr = (cmdq_pc_head+ins_leng-1);

        //Thread is already complete, but ISR have not coming yet
        if(DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread))==DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)))
        {
            DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
            //printk("6 Set PC to 0x%x",pCmdqAddr->MVA);
            CMDQ_MSG("\n==============Reset %d's PC  to ADDR[0x%x]===================\n",cmdqThread, (unsigned int)pCmdqAddr->MVA);
        }
        else
        {
            *(pPre_cmdqAddr->blockTailAddr) = 0x10000001;//Jump: Absolute
            *(pPre_cmdqAddr->blockTailAddr-1) = pCmdqAddr->MVA; //Jump to here

            CMDQ_MSG("\n==============Modify %d's Pre-ID %d Jump ADDR[0x%x] :0x%x , 0x%x ===================\n",cmdqThread, pre_task_id , (unsigned int)pPre_cmdqAddr->blockTailAddr ,(unsigned int)*(pPre_cmdqAddr->blockTailAddr) ,(unsigned int)*(pPre_cmdqAddr->blockTailAddr-1) );
        }
        //Modify Thread END addr
        DISP_REG_SET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread), pCmdqAddr->MVA + cmdqBufTbl[buf_id].blocksize);

        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread),0);
        spin_unlock_irqrestore(&gCmdqMgrLock,flags);
        //CS--------------------------------------------------

        //Schedule out
        //wait_event_interruptible(cmq_wait_queue[cmdqThread], (taskIDStatusTbl[taskID] == -1));
        wq_ret = wait_event_interruptible_timeout(cmq_wait_queue[cmdqThread], (taskIDStatusTbl[taskID] == 3), HZ/5);
        smp_rmb();

        do_gettimeofday(&end_t);

        //Clear Status
        spin_lock_irqsave(&gCmdqMgrLock,flags);


        if(wq_ret == 0)//if(taskIDStatusTbl[taskID] != -1)
        {
            CMDQ_ERR("A Task %d [%d] Wait CMDQ interrupt time out, PC: 0x%x 0x%x\n",taskID, taskIDStatusTbl[taskID], DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread)), DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)));

            dumpRegDebugInfo(cmdqThreadResTbl[cmdqThread], cmdqThread, *pCmdqAddr);


            //Warm reset CMDQ!

            // CMDQ FD interrupt received, but wait queue timeout
            if(taskIDStatusTbl[taskID] == 3)
            {
                waitQueueTimeoutCnt++;
                CMDQ_ERR("CPU busy->ISR already come but thread timeout\n");
            }
            else // True Timeout
            {
                if (DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread))==DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)))
                {
                    cmdqIsrLossCnt++;
                }
                else
                {
                    mdpBusyLongCnt++;
                }

                CMDQ_ERR("CMDQ execution failed, thread timeout\n");
                ret = false;

                if(taskIDStatusTbl[taskID]==2)
                {
                    CMDQ_ERR("Time out, but already free all by previous\n");
                }
                else
                {
                    cmdqForceFreeAll(cmdqThread);
                }
            }

        }

        taskIDStatusTbl[taskID] = -1; //free at last
        spin_unlock_irqrestore(&gCmdqMgrLock,flags);


    }

    // execution time monitor
    if (end_t.tv_usec < start_t.tv_usec)
        prevFrameExecTime.tv_usec = (end_t.tv_usec + 1000000) - start_t.tv_usec;
    else
        prevFrameExecTime.tv_usec = end_t.tv_usec - start_t.tv_usec;

//    spin_unlock(&gCmdqMgrLock);
    return ret;
}



void cmdqThreadComplete(int cmdqThread, bool cmdqIntStatus)
{
    unsigned long long end_time = 0;
    unsigned long long start_time = sched_clock();
    unsigned long cost = 0;

    int taskID = 0;

    taskID = cmdqThreadTaskList[cmdqThread][cmdqThreadTaskList_R[cmdqThread]];

    CMDQ_MSG("============cmdqThreadComplete Task! %d R: %d W: %d \n",taskID,cmdqThreadTaskList_R[cmdqThread], cmdqThreadTaskList_W[cmdqThread]);

    cmdqResource_free(taskID); //task complete

    //CMDQ read pointer ++
    cmdqThreadTaskList[cmdqThread][cmdqThreadTaskList_R[cmdqThread]] = 0; //Mark for complete at T' read pointer
    cmdqThreadTaskList_R[cmdqThread] = (cmdqThreadTaskList_R[cmdqThread] + 1) % CMDQ_THREAD_LIST_LENGTH; //increase Read pointer


    CMDQ_CHECK_TIME(cmdqResource_free);

//0322
    if(cmdqThreadTaskList_R[cmdqThread] == cmdqThreadTaskList_W[cmdqThread]) //no task needed
    {
        //power off!
        CMDQ_MSG("============cmdqThreadComplete Task! %d R: %d W: %d \n",taskID,cmdqThreadTaskList_R[cmdqThread], cmdqThreadTaskList_W[cmdqThread]);

        // check MDP engine busy
        if(true == checkMdpEngineStatus(cmdqThreadResTbl[cmdqThread]))
        {
            CMDQ_CHECK_TIME(checkMdpEngineStatus);
            
            if (true == cmdqIntStatus)
            {
                cmdqDoneMdpBusyCnt++;
            }

            //reset MDP engine
            resetMdpEngine(cmdqThreadResTbl[cmdqThread]);

            CMDQ_CHECK_TIME(resetMdpEngine);
        }

        cmdqHwClockOff(cmdqThreadResTbl[cmdqThread]);
        //TODO!

        CMDQ_CHECK_TIME(cmdqHwClockOff);

        //Return HW resource
        hwResTbl &= ~(cmdqThreadResTbl[cmdqThread]);

        //clear T' res table
        cmdqThreadResTbl[cmdqThread] = 0;
        CMDQ_MSG("\n======All job complete in cmdqThread: %d ====\n", cmdqThread);
    }

    //dumpDebugInfo();

}

void cmdqForceFreeAll(int cmdqThread)
{
    //SW force init
    int i = 0;
    unsigned int cmdq_polling_timeout;

    printk("!!!!!!!!!!!cmdqForceFreeAll ! re-init!!!!!!!!!!!!\n");

    for(i=0;i<CMDQ_BUFFER_NUM;i++)
    {
        cmdqBufTbl[i].Owner = -1; //free buffer
    }

    for(i=0;i<MAX_CMDQ_TASK_ID;i++)
    {
        if(taskIDStatusTbl[i]!=-1)
            taskIDStatusTbl[i] = 2; //mark as CANCEL
        taskResOccuTbl[i].cmdBufID = -1;
        taskResOccuTbl[i].cmdqThread= -1;
    }



    //Warm reset CMDQ
    DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
    cmdq_polling_timeout = 0;
    while(DISP_REG_GET(DISP_REG_CMDQ_THRx_RESET(cmdqThread)) == 1)
    {
        cmdq_polling_timeout++;
        if(cmdq_polling_timeout>1000)
        {
            CMDQ_ERR("CMDQ warm reset fail %x %x %x\n",DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x78), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x7c));
            break;
        }
    }

    //Warm reset MDP
    resetMdpEngine(cmdqThreadResTbl[cmdqThread]);

    //Resource Free
    for(i=0;i<CMDQ_THREAD_NUM;i++)
    {
        cmdqThreadResTbl[i] = 0;
        cmdqThreadTaskList_R[i] = 0;
        cmdqThreadTaskList_W[i] = 0;
    }

    hwResTbl = 0;

    if (clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        disable_clock(MT_CG_MDP_RDMA_SW_CG, "MDP_RDMA");
    if (clock_is_on(MT_CG_MDP_RSZ0_SW_CG))
        disable_clock(MT_CG_MDP_RSZ0_SW_CG, "MDP_RSZ0");
    if (clock_is_on(MT_CG_MDP_RSZ1_SW_CG))
        disable_clock(MT_CG_MDP_RSZ1_SW_CG, "MDP_RSZ1");
    if (clock_is_on(MT_CG_MDP_TDSHP_SW_CG))
        disable_clock(MT_CG_MDP_TDSHP_SW_CG, "MDP_TDSHP");
    if (clock_is_on(MT_CG_MDP_WROT_SW_CG))
        disable_clock(MT_CG_MDP_WROT_SW_CG, "MDP_WROT");
    if (clock_is_on(MT_CG_MDP_WDMA_SW_CG))
        disable_clock(MT_CG_MDP_WDMA_SW_CG, "MDP_WDMA");
//    if (clock_is_on(MT_CG_MM_CMDQ_SW_CG))
//        disable_clock(MT_CG_MM_CMDQ_SW_CG, "MM_CMDQ");
//    if (clock_is_on(MT_CG_MM_CMDQ_SMI_IF_SW_CG))
//        disable_clock(MT_CG_MM_CMDQ_SMI_IF_SW_CG, "MM_CMDQ_SMI_IF");

}


void cmdqForceFree_SW(int taskID)
{
    //SW force init
    int bufID = -1;

    if(taskID == -1 ||taskID>=MAX_CMDQ_TASK_ID)
    {
        printk("\n cmdqForceFree_SW Free Invalid Task ID \n");
        dumpDebugInfo();
        return;
    }

    taskIDStatusTbl[taskID] = -1; //mark for free
    taskResOccuTbl[taskID].cmdBufID = -1;
    taskResOccuTbl[taskID].cmdqThread= -1;

    bufID = taskResOccuTbl[taskID].cmdBufID;

    printk("\n cmdqForceFree_SW Free Buf %x own by [%x=%x]\n",bufID,taskID,cmdqBufTbl[bufID].Owner);

    if(bufID != -1) //Free All resource and return ID
    {
        cmdqBufTbl[bufID].Owner = -1;
    }
    else
    {
        CMDQ_ERR("\n cmdqForceFree_SW Free Invalid Buffer ID\n");
        dumpDebugInfo();
    }

    //FIXME! work around in 6572 (only one thread in 6572)
    hwResTbl = 0;

}


void cmdqTerminated(void)
{
    unsigned long flags;

    spin_lock_irqsave(&gCmdqMgrLock,flags);

    if((cmdqThreadTaskList_R[0] == cmdqThreadTaskList_W[0]) &&  (cmdqThreadResTbl[0] !=0)) //no task needed, but resource leaked!
    {
        CMDQ_ERR("\n======CMDQ Process terminated handling : %d ====\n", cmdqThreadResTbl[0]);

        cmdqForceFreeAll(0);
    }

    spin_unlock_irqrestore(&gCmdqMgrLock,flags);
}

static int __init cmdq_mdp_init(void)
{
    struct proc_dir_entry *pEntry = NULL;
    
    // Mount proc entry for non-specific group ioctls
    pEntry = create_proc_entry("mtk_mdp_cmdq", 0, NULL);
    if (NULL != pEntry)
    {
        pEntry->proc_fops = &cmdq_proc_fops;
    }
    else
    {
        printk("[CMDQ] cannot create mtk_mdp_cmdq!!!!!\n");
    }
    return 0;
}

static void __exit cmdq_mdp_exit(void)
{
    return;
}
    
module_init(cmdq_mdp_init);
module_exit(cmdq_mdp_exit);


MODULE_AUTHOR("Pablo Sun <pablo.sun@mediatek.com>");
MODULE_DESCRIPTION("CMDQ device Driver");
MODULE_LICENSE("GPL");
