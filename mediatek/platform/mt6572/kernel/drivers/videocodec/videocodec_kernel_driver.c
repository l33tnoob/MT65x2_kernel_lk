#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/wait.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/android_pmem.h>
#include <mach/dma.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#ifdef CONFIG_MTK_HIBERNATION
#include "mach/mtk_hibernate_dpm.h"
#endif

#include "videocodec_kernel_driver.h"
//#include "mt_clock_manager.h" // MT6572 EP
#include <mach/mt_clkmgr.h>     // For clock mgr APIS. enable_clock()/disable_clock().
#include "mach/sync_write.h"

#include <asm/cacheflush.h>
#include <asm/io.h>
#include "val_types_private.h"
#include "hal_types_private.h"
#include "val_api_private.h"
#include "val_log.h"
#include "mt_irq.h"
#include "mt_smi.h" 


#if defined(CONFIG_SMP)
//#include "mach/mtk_cpu_management.h"  // MT6572 EP

#define VCODEC_MULTI_THREAD
#endif

#define ONLY_FOR_EARLY_PORTING

#define VDO_HW_WRITE(ptr,data)     mt65xx_reg_sync_writel(data,ptr)
#define VDO_HW_READ(ptr)           (*((volatile unsigned int * const)ptr))

VAL_EVENT_T          MT6572_HWLockEvent;    //mutex : HWLockEventTimeoutLock

#define MFLEXVIDEO_DEVNAME     "Vcodec"
#define MT6572_VCODEC_DEV_MAJOR_NUMBER 160   //189

//#define MT6572_MFV_DEBUG
#ifdef MT6572_MFV_DEBUG
#undef MFV_DEBUG
#define MFV_DEBUG MFV_LOGE
#undef MFV_LOGD
#define MFV_LOGD  MFV_LOGE
#else
#define MFV_DEBUG(...)
#undef MFV_LOGD
#define MFV_LOGD(...) 
#endif

#define VCODEC_PAGE_TABLE_SIZE  4096

#define PREALLOCATE_CMD_MEM 1

#if PREALLOCATE_CMD_MEM
#define CMD_SIZE 1024*sizeof(MFV_DRV_CMD_T)
#else
#define CMD_SIZE 512*16
#endif

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
int video_cpu_opp_mask[11] = {1,1,1,0,0,1,1,1,1,1,1};
#endif

typedef struct
{
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4VCodecThreadNum;
    VAL_UINT32_T u4VCodecThreadID[VCODEC_THREAD_MAX_NUM];
#else
    VAL_UINT32_T u4TID1;
    VAL_UINT32_T u4TID2;
#endif
} VAL_VCODEC_SYSRAM_USER_T;

typedef struct
{
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4VCodecThreadNum;
    VAL_UINT32_T u4VCodecThreadID[VCODEC_THREAD_MAX_NUM];
#else
    VAL_UINT32_T u4LockHWTID1;
    VAL_UINT32_T u4LockHWTID2;
#endif
    VAL_TIME_T rLockedTime;
} VAL_VCODEC_HW_LOCK_T;

typedef struct
{
    VAL_UINT32_T u4KVA;
    VAL_UINT32_T u4KPA;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4VCodecThreadNum;
    VAL_UINT32_T u4VCodecThreadID[VCODEC_THREAD_MAX_NUM];
#else    
    VAL_UINT32_T u4TID1;
    VAL_UINT32_T u4TID2;
#endif
    VAL_UINT32_T u4Size;
} VAL_NON_CACHE_MEMORY_LIST_T;

static VAL_INTMEM_T grTempIntMem;   //mutex : IntMemLock
//static VAL_VCODEC_HW_LOCK_T grVcodecHWLock; //mutex : VcodecHWLock

typedef enum
{
    VCODEC_IRQ_VDEC = 0,    // 0
    VCODEC_IRQ_VENC,
    VCODEC_IRQ_MAX
} VCODEC_IRQ_ENUM;

#define VENC_IRQ_RET_REG_NUM   3  //mpeg4 enc
#define VDEC_IRQ_RET_REG_NUM   0  //h264 dec, vp8 dec

static DEFINE_MUTEX(SysramUserLock);
static DEFINE_MUTEX(EncEMILock);
static DEFINE_MUTEX(DecEMILock);
//static DEFINE_MUTEX(VcodecHWLock);
static DEFINE_MUTEX(PWRLock);
static DEFINE_MUTEX(HWLockEventTimeoutLock);
static DEFINE_MUTEX(IntMemLock);
static DEFINE_MUTEX(IsOpenedLock);
static DEFINE_MUTEX(DriverOpenCountLock);
static DEFINE_MUTEX(NonCacheMemoryListLock);
static DEFINE_MUTEX(VcodecSysramUserLock);
static DEFINE_MUTEX(VcodecHWUserLock);

static DEFINE_SPINLOCK(OalHWContextLock);
static DEFINE_SPINLOCK(LockHWCountLock);
static DEFINE_SPINLOCK(ISRCountLock);
static DEFINE_SPINLOCK(VcodecHWLock);


static VAL_BOOL_T bIsOpened = VAL_FALSE;    //mutex : IsOpenedLock

static dev_t mflexvideo_devno = MKDEV(MT6572_VCODEC_DEV_MAJOR_NUMBER,0);
static struct cdev * mflexvideo_cdev;
static VAL_UINT32_T gu4INTMEMCounter = 0;   //mutex : SysramUserLock
static VAL_UINT32_T gu4EncEMICounter = 0;   //mutex : EncEMILock
static VAL_UINT32_T gu4DecEMICounter = 0;   //mutex : DecEMILock
static VAL_UINT32_T gu4PWRCounter = 0;      //mutex : PWRLock 
static int MT6572Driver_Open_Count;         //mutex : DriverOpenCountLock

#if 1 // Morris Yang 20120112 mark temporarily
extern unsigned long long mt_get_cpu_idle(int cpu);
extern unsigned long long mt_get_thread_cputime(pid_t pid);
extern unsigned long long mt_sched_clock(void);
#endif

//extern int nr_cpu_ids; // start from 1 instead of 0
extern unsigned long get_cpu_load(int cpu);

//============================================================
//  HW LOCK
#undef VENC_BASE
#define VENC_BASE  0x14016000
#define VENC_REGION (0x1000)
#undef VDEC_BASE
#define VDEC_BASE  0x14017000
#define VDEC_REGION (0x2000)

#define HW_BASE 0x7fff000
#define HW_REGION (0x2000)

//Ckgen
#define HW_TABLET_BASE 0x10000000
#define HW_TABLET_REGION (0x1000)

//dram
#define HW_TABLET_BASE2 0x10004000
#define HW_TABLET_REGION2 (0x1000)

#define VENC_IRQ_ACK_addr         VENC_BASE + 0x678 
#define DEC_VDEC_INT_STA_ADDR     VDEC_BASE + 0x10
#define DEC_VDEC_INT_ACK_ADDR     VDEC_BASE + 0xc

#define VENC_IRQ_STATUS_addr        VENC_BASE + 0x67C
#define VENC_ZERO_COEF_COUNT_addr   VENC_BASE + 0x688
#define VENC_BYTE_COUNT_addr        VENC_BASE + 0x680

#define VCODEC_MULTIPLE_INSTANCE_NUM 16
#define VCODEC_MULTIPLE_INSTANCE_NUM_x_10 (VCODEC_MULTIPLE_INSTANCE_NUM * 10)
static VAL_VCODEC_OAL_HW_CONTEXT_T  oal_hw_context[VCODEC_MULTIPLE_INSTANCE_NUM];               //spinlock : OalHWContextLock
static VAL_NON_CACHE_MEMORY_LIST_T  grNonCacheMemoryList[VCODEC_MULTIPLE_INSTANCE_NUM_x_10];    //mutex : NonCacheMemoryListLock
static VAL_VCODEC_SYSRAM_USER_T grVcodecSysramUser[VCODEC_MULTIPLE_INSTANCE_NUM];               //mutex : VcodecSysramUserLock
static VAL_VCODEC_THREAD_ID_T   grVcodecHWUser[VCODEC_MULTIPLE_INSTANCE_NUM];                   //mutex : VcodecHWUserLock
static VAL_UINT32_T             grVcodecDriverUser[VCODEC_MULTIPLE_INSTANCE_NUM];               //mutex : VcodecHWUserLock

static VAL_UINT32_T             gu4LockHWCount; //spinlock : LockHWCountLock
static VAL_UINT32_T             gu4ISRCount;    //spinlock : ISRCountLock
static VAL_VCODEC_HW_LOCK_T     grVcodecHWLock; //spinlock : VcodecHWLock

#if defined(CONFIG_SMP)
static VAL_UINT32_T   gPowerSavingCount = 0;
#endif

int KVA_ENC_INT_ACK_ADDR, KVA_VDEC_INT_STA_ADDR, KVA_VDEC_INT_ACK_ADDR;
int KVA_VENC_IRQ_STATUS_ADDR, KVA_VENC_ZERO_COEF_COUNT_ADDR, KVA_VENC_BYTE_COUNT_ADDR;

int search_HWLockSlot_ByTID(int pa, int curr_tid);

extern unsigned int pmem_user_v2p_video(unsigned int va);
extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq, unsigned int polarity);

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
void Add_SysramUserList(VAL_UINT32_T a_u4VCodecThreadNum, VAL_UINT32_T* a_pu4VCodecThreadID)
#else
void Add_SysramUserList(VAL_UINT32_T a_u4Tid1, VAL_UINT32_T a_u4Tid2)
#endif
{
    VAL_UINT32_T u4I = 0;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4J = 0;
    MFV_LOGD("Add_SysramUserList +\n");
#else
    MFV_LOGD("Add_SysramUserList +, a_u4Tid1 = %d, a_u4Tid2 = %d\n", a_u4Tid1, a_u4Tid2);
#endif

    for(u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM; u4I++)
    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        if (grVcodecSysramUser[u4I].u4VCodecThreadNum == VCODEC_THREAD_MAX_NUM)
        {
            grVcodecSysramUser[u4I].u4VCodecThreadNum = a_u4VCodecThreadNum;
            for (u4J = 0; u4J < grVcodecSysramUser[u4I].u4VCodecThreadNum; u4J++)
            {
                grVcodecSysramUser[u4I].u4VCodecThreadID[u4J] = *(a_pu4VCodecThreadID + u4J);
                MFV_LOGD("[Add_SysramUserList] VCodecThreadNum = %d\n", grVcodecSysramUser[u4I].u4VCodecThreadNum);
                MFV_LOGD("[Add_SysramUserList] VCodecThreadID = %d\n", grVcodecSysramUser[u4I].u4VCodecThreadID[u4J]);
            }
            gu4INTMEMCounter++;
            MFV_LOGE("[Add_SysramUserList] gu4INTMEMCounter = %d\n", gu4INTMEMCounter);
            break;
        }
#else
        if ((grVcodecSysramUser[u4I].u4TID1 == 0xffffffff) && (grVcodecSysramUser[u4I].u4TID2 == 0xffffffff))
        {
            MFV_LOGD("Add_SysramUserList, index = %d, u4Tid1 = %d, u4Tid2 = %d\n", u4I, a_u4Tid1, a_u4Tid2);
            grVcodecSysramUser[u4I].u4TID1 = a_u4Tid1;
            grVcodecSysramUser[u4I].u4TID2 = a_u4Tid2;
            gu4INTMEMCounter++;
            break;
        }
#endif
    }

    if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM)
    {
        MFV_LOGE("[ERROR] CAN'T ADD Add_SysramUserList, List is FULL!!\n");
    }
    
    MFV_LOGD("Add_SysramUserList -\n");
}

void Free_SysramUserList(VAL_UINT32_T a_u4CurrTid, VAL_BOOL_T a_bForceFree)
{    
    VAL_UINT32_T u4I = 0;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4J = 0;
#endif
    MFV_LOGD("Free_SysramUserList +, a_u4CurrTid = %d\n", a_u4CurrTid);        

    for(u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM; u4I++)
    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        if (grVcodecSysramUser[u4I].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM)
        {
            if (a_bForceFree == VAL_TRUE)
            {
                for (u4J = 0; u4J < grVcodecSysramUser[u4I].u4VCodecThreadNum; u4J++)
                {
                    if (grVcodecSysramUser[u4I].u4VCodecThreadID[u4J] == a_u4CurrTid)
                    {
                        for (u4J = 0; u4J < grVcodecSysramUser[u4I].u4VCodecThreadNum; u4J++)
                        {
                            grVcodecSysramUser[u4I].u4VCodecThreadID[u4J] = 0xffffffff;
                        }
                        grVcodecSysramUser[u4I].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
                        gu4INTMEMCounter--;
                        break;
                    }
                }
                MFV_LOGE("[Free_SysramUserList][ForceFree] gu4INTMEMCounter = %d\n", gu4INTMEMCounter);
            }
            else
            {
                for (u4J = 0; u4J < grVcodecSysramUser[u4I].u4VCodecThreadNum; u4J++)
                {
                    if (grVcodecSysramUser[u4I].u4VCodecThreadID[u4J] == a_u4CurrTid)
                    {
                        for (u4J = 0; u4J < grVcodecSysramUser[u4I].u4VCodecThreadNum; u4J++)
                        {
                            grVcodecSysramUser[u4I].u4VCodecThreadID[u4J] = 0xffffffff;
                        }
                        grVcodecSysramUser[u4I].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
                        gu4INTMEMCounter--;
                        break;
                    }
                }
                
                if (u4J != grVcodecSysramUser[u4I].u4VCodecThreadNum)
                {
                    MFV_LOGE("[Free_SysramUserList] gu4INTMEMCounter = %d\n", gu4INTMEMCounter);
                    break;
                }
            }
        }
#else
        if ((grVcodecSysramUser[u4I].u4TID1 == a_u4CurrTid) || (grVcodecSysramUser[u4I].u4TID2 == a_u4CurrTid))
        {
            if (a_bForceFree == VAL_TRUE)
            {
                MFV_LOGE("[WARNING] Force Free_SysramUserList, index = %d, u4Tid1 = %d, u4Tid2 = %d\n", 
                    u4I, grVcodecSysramUser[u4I].u4TID1, grVcodecSysramUser[u4I].u4TID2);
                grVcodecSysramUser[u4I].u4TID1 = 0xffffffff;
                grVcodecSysramUser[u4I].u4TID2 = 0xffffffff;
                gu4INTMEMCounter--;
            }
            else
            {
                MFV_LOGD("Free_SysramUserList, index = %d, u4Tid1 = %d, u4Tid2 = %d\n", 
                    u4I, grVcodecSysramUser[u4I].u4TID1, grVcodecSysramUser[u4I].u4TID2);
                grVcodecSysramUser[u4I].u4TID1 = 0xffffffff;
                grVcodecSysramUser[u4I].u4TID2 = 0xffffffff;
                gu4INTMEMCounter--;
                break;
            }
        }
#endif        
    }

    if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM)
    {
        if (a_bForceFree == VAL_FALSE)
        {
            MFV_LOGE("[ERROR] CAN'T FREE Free_SysramUserList, List is FULL!!\n");
        }
    }

    MFV_LOGD("Free_SysramUserList -\n");
}

#if 0
VAL_VCODEC_OAL_HW_CONTEXT_T* get_HWLockSlot(void)
{	
	int i;
	
	for(i=0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++)
	{
		if( oal_hw_context[i].ObjId == -1)
        {
			MFV_DEBUG("getFree_HWLock slot =%d\n", i);
			return &oal_hw_context[i];		
		}
	}	
	
	MFV_LOGE("[ERROR] getFree_HWLock All %d Slot unavaliable\n", VCODEC_MULTIPLE_INSTANCE_NUM);
	
	return 0;
} 
#endif

int getCurInstanceCount (void) {
    int i;
    int _empty_count = 0;
    for(i=0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {
        if(oal_hw_context[i].ObjId == -1)
        {
            _empty_count++;
        }
    }	
    return (VCODEC_MULTIPLE_INSTANCE_NUM - _empty_count);
}

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
void Add_NonCacheMemoryList(VAL_UINT32_T a_u4KVA, VAL_UINT32_T a_u4KPA, VAL_UINT32_T a_u4Size, VAL_UINT32_T a_u4VCodecThreadNum, VAL_UINT32_T* a_pu4VCodecThreadID)
#else
void Add_NonCacheMemoryList(VAL_UINT32_T a_u4KVA, VAL_UINT32_T a_u4KPA, VAL_UINT32_T a_u4Size, VAL_UINT32_T a_u4Tid1, VAL_UINT32_T a_u4Tid2)
#endif
{
    VAL_UINT32_T u4I = 0;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4J = 0;
#endif
    
    MFV_LOGD("Add_NonCacheMemoryList +, KVA = 0x%x, KPA = 0x%x, Size = 0x%x\n", a_u4KVA, a_u4KPA, a_u4Size);

    for(u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++)
    {
        if ((grNonCacheMemoryList[u4I].u4KVA == 0xffffffff) && (grNonCacheMemoryList[u4I].u4KPA == 0xffffffff))
        {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            MFV_LOGD("ADD Add_NonCacheMemoryList index = %d, VCodecThreadNum = %d, curr_tid = %d\n", 
                u4I, a_u4VCodecThreadNum, current->pid);
#else
            MFV_LOGD("ADD Add_NonCacheMemoryList index = %d, tid1 = %d, tid2 = %d, curr_tid = %d\n", 
                u4I, a_u4Tid1, a_u4Tid2, current->pid);
#endif
            
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            grNonCacheMemoryList[u4I].u4VCodecThreadNum = a_u4VCodecThreadNum;
            for (u4J = 0; u4J < grNonCacheMemoryList[u4I].u4VCodecThreadNum; u4J++)
            {
                grNonCacheMemoryList[u4I].u4VCodecThreadID[u4J] = *(a_pu4VCodecThreadID + u4J);
                MFV_LOGD("[Add_NonCacheMemoryList] VCodecThreadNum = %d, VCodecThreadID = %d\n", 
                    grNonCacheMemoryList[u4I].u4VCodecThreadNum, grNonCacheMemoryList[u4I].u4VCodecThreadID[u4J]);
            }
#else
            grNonCacheMemoryList[u4I].u4TID1 = a_u4Tid1;
            grNonCacheMemoryList[u4I].u4TID2 = a_u4Tid2;
#endif
            grNonCacheMemoryList[u4I].u4KVA = a_u4KVA;
            grNonCacheMemoryList[u4I].u4KPA = a_u4KPA;
            grNonCacheMemoryList[u4I].u4Size = a_u4Size;
            break;
        }
    }

    if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM_x_10)
    {
        MFV_LOGE("[ERROR] CAN'T ADD Add_NonCacheMemoryList, List is FULL!!\n");
    }

    MFV_LOGD("Add_NonCacheMemoryList -\n");
}

void Free_NonCacheMemoryList(VAL_UINT32_T a_u4KVA, VAL_UINT32_T a_u4KPA)
{
    VAL_UINT32_T u4I = 0;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4J = 0;
#endif
    
    MFV_LOGD("Free_NonCacheMemoryList +, KVA = 0x%x, KPA = 0x%x\n", a_u4KVA, a_u4KPA);

    for(u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++)
    {
        if ((grNonCacheMemoryList[u4I].u4KVA == a_u4KVA) && (grNonCacheMemoryList[u4I].u4KPA == a_u4KPA))
        {
            MFV_LOGD("Free Free_NonCacheMemoryList index = %d\n", u4I);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            grNonCacheMemoryList[u4I].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
            for (u4J = 0; u4J <VCODEC_THREAD_MAX_NUM; u4J++)
            {
                grNonCacheMemoryList[u4I].u4VCodecThreadID[u4J] = 0xffffffff;
            }
#else
            grNonCacheMemoryList[u4I].u4TID1 = 0xffffffff;
            grNonCacheMemoryList[u4I].u4TID2 = 0xffffffff;
#endif
            grNonCacheMemoryList[u4I].u4KVA = 0xffffffff;
            grNonCacheMemoryList[u4I].u4KPA = 0xffffffff;
            grNonCacheMemoryList[u4I].u4Size = 0xffffffff;
            break;
        }
    }

    if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM_x_10)
    {
        MFV_LOGE("[ERROR] CAN'T Free Free_NonCacheMemoryList, Address is not find!!\n");
    }

    MFV_LOGD("Free_NonCacheMemoryList -\n");
}

void Force_Free_NonCacheMemoryList(VAL_UINT32_T a_u4Tid)
{
    VAL_UINT32_T u4I = 0;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4J = 0;
    VAL_UINT32_T u4K = 0;
#endif
    
    MFV_LOGD("Force_Free_NonCacheMemoryList +, curr_id = %d", a_u4Tid);

    for(u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++)
    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        if (grNonCacheMemoryList[u4I].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM)
        {
            for (u4J = 0; u4J < grNonCacheMemoryList[u4I].u4VCodecThreadNum; u4J++)
            {
                if (grNonCacheMemoryList[u4I].u4VCodecThreadID[u4J] == a_u4Tid)
                {
                    MFV_LOGE("[WARNING] Force_Free_NonCacheMemoryList index = %d, tid = %d, KVA = 0x%x, KPA = 0x%x, Size = %d\n", 
                        u4I, a_u4Tid, grNonCacheMemoryList[u4I].u4KVA, grNonCacheMemoryList[u4I].u4KPA, grNonCacheMemoryList[u4I].u4Size);
                
                    dma_free_coherent(0, grNonCacheMemoryList[u4I].u4Size, (void *)grNonCacheMemoryList[u4I].u4KVA, (dma_addr_t)grNonCacheMemoryList[u4I].u4KPA);
                
                    grNonCacheMemoryList[u4I].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
                    for (u4K = 0; u4K <VCODEC_THREAD_MAX_NUM; u4K++)
                    {
                        grNonCacheMemoryList[u4I].u4VCodecThreadID[u4K] = 0xffffffff;
                    }
                    grNonCacheMemoryList[u4I].u4KVA = 0xffffffff;
                    grNonCacheMemoryList[u4I].u4KPA = 0xffffffff;
                    grNonCacheMemoryList[u4I].u4Size = 0xffffffff;
                    break;
                }
            }
        }
#else
        if ((grNonCacheMemoryList[u4I].u4TID1 == a_u4Tid) || (grNonCacheMemoryList[u4I].u4TID2 == a_u4Tid))
        {
            MFV_LOGE("[WARNING] Force_Free_NonCacheMemoryList index = %d, tid1 = %d, tid2 = %d, KVA = 0x%x, KPA = 0x%x, Size = %d\n", 
                u4I, grNonCacheMemoryList[u4I].u4TID1, grNonCacheMemoryList[u4I].u4TID2,
                grNonCacheMemoryList[u4I].u4KVA, grNonCacheMemoryList[u4I].u4KPA, grNonCacheMemoryList[u4I].u4Size);
            
            dma_free_coherent(0, grNonCacheMemoryList[u4I].u4Size, (void *)grNonCacheMemoryList[u4I].u4KVA, (dma_addr_t)grNonCacheMemoryList[u4I].u4KPA);
            
            grNonCacheMemoryList[u4I].u4TID1 = 0xffffffff;
            grNonCacheMemoryList[u4I].u4TID2 = 0xffffffff;
            grNonCacheMemoryList[u4I].u4KVA = 0xffffffff;
            grNonCacheMemoryList[u4I].u4KPA = 0xffffffff;
            grNonCacheMemoryList[u4I].u4Size = 0xffffffff;
        }
#endif
    }

    MFV_LOGD("Force_Free_NonCacheMemoryList -, curr_id = %d", a_u4Tid);
}

VAL_UINT32_T Search_NonCacheMemoryList_By_KPA(VAL_UINT32_T a_u4KPA)
{
    VAL_UINT32_T u4I = 0;
    VAL_UINT32_T u4VA_Offset = 0;
    
    u4VA_Offset = a_u4KPA & 0x00000fff;
        
    MFV_LOGD("Search_NonCacheMemoryList_By_KPA +, KPA = 0x%x, u4VA_Offset = 0x%x\n", a_u4KPA, u4VA_Offset);

    for(u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++)
    {
        if (grNonCacheMemoryList[u4I].u4KPA == (a_u4KPA - u4VA_Offset))
        {
            MFV_LOGD("Find Search_NonCacheMemoryList_By_KPA index = %d\n", u4I);
            break;
        }
    }

    if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM_x_10)
    {
        MFV_LOGE("[ERROR] CAN'T Find Search_NonCacheMemoryList_By_KPA, Address is not find!!\n");
        return (grNonCacheMemoryList[0].u4KVA + u4VA_Offset);    
    }

    MFV_LOGD("Search_NonCacheMemoryList_By_KPA, u4VA = 0x%x -\n", (grNonCacheMemoryList[u4I].u4KVA + u4VA_Offset));

    return (grNonCacheMemoryList[u4I].u4KVA + u4VA_Offset);
}

VAL_VCODEC_OAL_HW_CONTEXT_T *setCurr_HWLockSlot_Thread_ID(VAL_VCODEC_THREAD_ID_T a_prVcodecThreadID, VAL_UINT32_T *a_prIndex)
{	
	int i;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    int j;
    int k;
#endif

	// Dump current tid1 tid2
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        if (oal_hw_context[i].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM)
        {
            for(j = 0;j < oal_hw_context[i].u4VCodecThreadNum; j++)
            {
                MFV_LOGD("[setCurr_HWLockSlot_Thread_ID] Dump curr slot %d, ThreadID[%d] = %d\n", i, j, oal_hw_context[i].u4VCodecThreadID[j]);
            }
        }
#else
			MFV_DEBUG("Dump curr slot %d ThreadID_1 %d \n", i, oal_hw_context[i].tid1);
            MFV_DEBUG("Dump curr slot %d ThreadID_2 %d \n", i, oal_hw_context[i].tid2);
#endif
	}
	
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    for (i = 0; i < a_prVcodecThreadID.u4VCodecThreadNum; i++)
    {
        MFV_LOGD("[setCurr_HWLockSlot_Thread_ID] VCodecThreadNum = %d, VCodecThreadID = %d\n",
            a_prVcodecThreadID.u4VCodecThreadNum, 
            a_prVcodecThreadID.u4VCodecThreadID[i]
            );
    }
#endif

	// check if current tid1 or tid2 exist in oal_hw_context[i].ObjId 
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {   
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        if (oal_hw_context[i].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM)
        {
            for (j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++)
            {
                for (k = 0; k < a_prVcodecThreadID.u4VCodecThreadNum; k++)
                {
                    if (oal_hw_context[i].u4VCodecThreadID[j] == a_prVcodecThreadID.u4VCodecThreadID[k])
                    {
                        MFV_LOGE("[setCurr_HWLockSlot_Thread_ID] Curr Already exist in %d Slot\n", i);
                        *a_prIndex = i;
    			        return &oal_hw_context[i];
                    }
                }
            }
        }
#else
		if ((oal_hw_context[i].tid1 == a_prVcodecThreadID.u4tid1) ||
            (oal_hw_context[i].tid1 == a_prVcodecThreadID.u4tid2) ||
            (oal_hw_context[i].tid2 == a_prVcodecThreadID.u4tid1) ||
            (oal_hw_context[i].tid2 == a_prVcodecThreadID.u4tid2))
        {
			MFV_LOGE("Curr Already exist in %d Slot\n", i);
            *a_prIndex = i;
			return &oal_hw_context[i];
		}				
#endif
	}	

	// if not exist in table,  find a new free slot and put it
	for(i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        if (oal_hw_context[i].u4VCodecThreadNum == VCODEC_THREAD_MAX_NUM)
        {
            oal_hw_context[i].u4VCodecThreadNum = a_prVcodecThreadID.u4VCodecThreadNum;
            for(j = 0; j < a_prVcodecThreadID.u4VCodecThreadNum; j++)
            {
                oal_hw_context[i].u4VCodecThreadID[j] = a_prVcodecThreadID.u4VCodecThreadID[j];
                MFV_LOGD("[setCurr_HWLockSlot_Thread_ID] setCurr %d Slot, %d\n", i, oal_hw_context[i].u4VCodecThreadID[j]);
            }            
            *a_prIndex = i;
            return &oal_hw_context[i];
        }
#else
		if ((oal_hw_context[i].tid1 == -1) && (oal_hw_context[i].tid2 == -1))
        {
			oal_hw_context[i].tid1 = a_prVcodecThreadID.u4tid1;
            oal_hw_context[i].tid2 = a_prVcodecThreadID.u4tid2;
			MFV_DEBUG("setCurr %d Slot, %d, %d\n", i, a_prVcodecThreadID.u4tid1, a_prVcodecThreadID.u4tid2);
            *a_prIndex = i;
			return &oal_hw_context[i];
		}				
#endif
	}	
	
	{
	    MFV_LOGE("[VCodec Error][ERROR] setCurr_HWLockSlot_Thread_ID All %d Slots unavaliable\n", VCODEC_MULTIPLE_INSTANCE_NUM);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        oal_hw_context[0].u4VCodecThreadNum = a_prVcodecThreadID.u4VCodecThreadNum;
        for(i = 0; i < oal_hw_context[0].u4VCodecThreadNum; i++)
        {
            oal_hw_context[0].u4VCodecThreadID[i] = a_prVcodecThreadID.u4VCodecThreadID[i];
        }
#else
	    oal_hw_context[0].tid1 = a_prVcodecThreadID.u4tid1;
        oal_hw_context[0].tid2 = a_prVcodecThreadID.u4tid2;
#endif
        *a_prIndex = 0;
        return &oal_hw_context[0];
	}
}

VAL_VCODEC_OAL_HW_CONTEXT_T *setCurr_HWLockSlot(int pa, int tid){
	
	int i;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    int j;
#endif

	// Dump current ObjId
	
	for(i=0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {
			MFV_DEBUG("Dump curr slot %d ObjId %x \n", i, oal_hw_context[i].ObjId);
	}
	
	// check if current ObjId exist in oal_hw_context[i].ObjId 
	for(i=0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {   
		if( oal_hw_context[i].ObjId == pa)
        {
			MFV_DEBUG("Curr Already exist in %d Slot\n", i);
			return &oal_hw_context[i];
		}				
	}	

	// if not exist in table,  find a new free slot and put it
	for(i=0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++){
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        if (oal_hw_context[i].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM)
        {
            for (j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++)
            {
                if (oal_hw_context[i].u4VCodecThreadID[j] == current->pid)
                {
                    oal_hw_context[i].ObjId = pa;
			        MFV_LOGD("[setCurr_HWLockSlot] setCurr %d Slot\n", i);
			        return &oal_hw_context[i];
                }
            }
        }
#else
		if( (oal_hw_context[i].tid1 == current->pid) || (oal_hw_context[i].tid2 == current->pid))
        {
			oal_hw_context[i].ObjId = pa;
			MFV_DEBUG("setCurr %d Slot\n", i);
			return &oal_hw_context[i];
		}				
#endif
	}	
	
	MFV_LOGE("[VCodec Error][ERROR] setCurr_HWLockSlot All %d Slots unavaliable\n", VCODEC_MULTIPLE_INSTANCE_NUM);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        oal_hw_context[0].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM - 1;
        for(i = 0; i < oal_hw_context[0].u4VCodecThreadNum; i++)
        {
            oal_hw_context[0].u4VCodecThreadID[i] = current->pid;
        }
#else
	oal_hw_context[0].ObjId = pa;
	oal_hw_context[0].tid1 = current->pid;
    oal_hw_context[0].tid2 = current->pid;
#endif    
	return &oal_hw_context[0];
} 

VAL_VCODEC_OAL_HW_CONTEXT_T *freeCurr_HWLockSlot(int pa)
{	
	int i;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    int j;
#endif
	
	// check if current ObjId exist in oal_hw_context[i].ObjId 
	
	for(i=0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {	
		if( oal_hw_context[i].ObjId == pa)
        {
			oal_hw_context[i].ObjId = -1;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            for(j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++)
            {
                oal_hw_context[i].u4VCodecThreadID[j] = -1;
            }
            oal_hw_context[i].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
#else
			oal_hw_context[i].tid1 = -1;
            oal_hw_context[i].tid2 = -1;
#endif
			oal_hw_context[i].Oal_HW_reg = (VAL_VCODEC_OAL_HW_REGISTER_T  *)0;
			MFV_DEBUG("freeCurr_HWLockSlot %d Slot\n", i);
			return &oal_hw_context[i];
		}
		
	}		
	
	MFV_LOGE("[VCodec Error][ERROR] freeCurr_HWLockSlot can't find pid in HWLockSlot\n");
	return 0;
} 

int search_HWLockSlot_ByTID(int pa, int curr_tid)
{
	int i;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    int j;
#endif
	
	for(i=0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {	
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        if (oal_hw_context[i].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM)
        {
            for(j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++)
            {
                if (oal_hw_context[i].u4VCodecThreadID[j] == curr_tid)
                {
                    MFV_LOGD("[search_HWLockSlot_ByTID] Lookup curr HW Locker is ObjId %d in index%d\n", curr_tid, i);
    			    return i;
                }
            }
        }
#else
		if ((oal_hw_context[i].tid1 == curr_tid) || (oal_hw_context[i].tid2 == curr_tid))
        {
			//MFV_LOGE("Lookup curr HW Locker is ObjId %d in index%d\n", curr_tid, i);
			return i;
		}	
#endif
	}		
	
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    if (grVcodecHWLock.u4VCodecThreadNum == VCODEC_THREAD_MAX_NUM)
    {
        MFV_LOGE("[search_HWLockSlot_ByTID][ERROR] grVcodecHWLock.u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM\n");   
    }
    else
    {
        for(i = 0; i < grVcodecHWLock.u4VCodecThreadNum; i++)
        {
            MFV_LOGE("[VCodec Error][ERROR] Can't find HW_Locker owner tid = %d, curr_tid = %d\n",
                grVcodecHWLock.u4VCodecThreadID[i], curr_tid);
        }
    }
#else
	MFV_LOGE("[VCodec Error][ERROR] Can't find HW_Locker owner tid1 = %d, tid2 = %d, curr_tid = %d\n", 
        grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2, curr_tid);
#endif
	return -1;
}

//============================================================

void MT6572_Video_ISR(VCODEC_IRQ_ENUM a_VcodecIRQ)
{
    VAL_RESULT_T  eValRet;
    int index, i, maxnum;
    unsigned int reg_val;
    unsigned long ulFlags, ulFlagsISR, ulFlagsLockHW, ulFlagsVcodecHWLock;
    VAL_UINT32_T u4IRQStatus = 0;

    VAL_UINT32_T u4TempISRCount = 0;
    VAL_UINT32_T u4TempLockHWCount = 0;
    VAL_RESULT_T  eValHWLockRet = VAL_RESULT_INVALID_ISR;
    //----------------------
    VAL_UINT32_T u4I;

    spin_lock_irqsave(&ISRCountLock, ulFlagsISR);
    gu4ISRCount++;
    u4TempISRCount = gu4ISRCount;
    spin_unlock_irqrestore(&ISRCountLock, ulFlagsISR);

    spin_lock_irqsave(&LockHWCountLock, ulFlagsLockHW);
    u4TempLockHWCount = gu4LockHWCount;
    spin_unlock_irqrestore(&LockHWCountLock, ulFlagsLockHW);

    if (u4TempISRCount != u4TempLockHWCount)
    {
        //MFV_LOGE("[INFO] ISRCount: 0x%x, LockHWCount:0x%x\n", u4TempISRCount, u4TempLockHWCount);
    }

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    if (grVcodecHWLock.u4VCodecThreadNum == VCODEC_THREAD_MAX_NUM)
    {
        MFV_LOGE("[MT6572_Video_ISR][ERROR] grVcodecHWLock.u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM\n"); 

        MFV_LOGE("[ERROR] NO one Lock HW, please check!!\n");

        // ACK interrupt 
	    // decoder
  	    reg_val = VDO_HW_READ(KVA_VDEC_INT_STA_ADDR);
        VDO_HW_WRITE(KVA_VDEC_INT_ACK_ADDR, reg_val);
  	    // encoder
        VDO_HW_WRITE(KVA_ENC_INT_ACK_ADDR, 1);
    
        return;
    }
    else
    {
        for(i = 0; i < grVcodecHWLock.u4VCodecThreadNum; i++)
        {
            if (grVcodecHWLock.u4VCodecThreadID[i] == -1)
            {
                MFV_LOGE("[ERROR] NO one Lock HW, please check!!\n");

                // ACK interrupt 
    		    // decoder
          	    reg_val = VDO_HW_READ(KVA_VDEC_INT_STA_ADDR);
                VDO_HW_WRITE(KVA_VDEC_INT_ACK_ADDR, reg_val);
          	    // encoder
    	        VDO_HW_WRITE(KVA_ENC_INT_ACK_ADDR, 1);
    	    
                return;
            }
        }
    }
#else
    if ((grVcodecHWLock.u4LockHWTID1 == -1) || (grVcodecHWLock.u4LockHWTID2 == -1))
    {
        MFV_LOGE("[ERROR] NO one Lock HW, please check!!\n");

        // ACK interrupt 
		// decoder
      	reg_val = VDO_HW_READ(KVA_VDEC_INT_STA_ADDR);
        VDO_HW_WRITE(KVA_VDEC_INT_ACK_ADDR, reg_val);
      	// encoder
	    VDO_HW_WRITE(KVA_ENC_INT_ACK_ADDR, 1);
	    
        return;
    }
#endif

    //MFV_DEBUG("MT6572_Video_ISR+  LockHWTID1 = %d, LockHWTID2 = %d\n", grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2);

    spin_lock_irqsave(&OalHWContextLock, ulFlags);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    index = search_HWLockSlot_ByTID(0, grVcodecHWLock.u4VCodecThreadID[0]);
#else
    index = search_HWLockSlot_ByTID(0, grVcodecHWLock.u4LockHWTID1);
#endif

    //MFV_DEBUG("index = %d\n", index);

    // in case, if the process is killed first, 
    // then receive an ISR from HW, the event information already cleared.
    if(index == -1)
    {
    	MFV_LOGE("[ERROR][ISR] Can't find any index in ISR\n");   	  		  
        
		// ACK interrupt 
		// decoder
      	reg_val = VDO_HW_READ(KVA_VDEC_INT_STA_ADDR);
        VDO_HW_WRITE(KVA_VDEC_INT_ACK_ADDR, reg_val);
      	// encoder
	    VDO_HW_WRITE(KVA_ENC_INT_ACK_ADDR, 1);
	    spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        
        return;
    }
						
    // get address from context			
    //MFV_DEBUG("ISR: Total %d u4NumOfRegister\n", oal_hw_context[index].u4NumOfRegister);

    maxnum = oal_hw_context[index].u4NumOfRegister;
    if(oal_hw_context[index].u4NumOfRegister > VCODEC_MULTIPLE_INSTANCE_NUM)
    {
    	MFV_LOGE("[ERROR] oal_hw_context[index].u4NumOfRegister =%d\n", oal_hw_context[index].u4NumOfRegister);
    	maxnum = VCODEC_MULTIPLE_INSTANCE_NUM;
    }

    if (maxnum == VENC_IRQ_RET_REG_NUM)     //encode
    {
        if(a_VcodecIRQ == VCODEC_IRQ_VDEC)  //decode
        {
            MFV_LOGE("[INFO] HW locked by VENC, but we have VDEC ISR\n");
		    // decoder
      	    reg_val = VDO_HW_READ(KVA_VDEC_INT_STA_ADDR);
            VDO_HW_WRITE(KVA_VDEC_INT_ACK_ADDR, reg_val);
            spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
            
            return;
        }
    }

    if (maxnum == VDEC_IRQ_RET_REG_NUM)     //decode
    {
        if(a_VcodecIRQ == VCODEC_IRQ_VENC)  //encode
        {
            MFV_LOGE("[INFO] HW locked by VDEC, but we have VENC ISR\n");
      	    // encoder
	        VDO_HW_WRITE(KVA_ENC_INT_ACK_ADDR, 1);
            spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
            
            return;
        }
    }

    //MFV_DEBUG("oal_hw_context[index].kva_u4HWIsCompleted 0x%x value=%d \n", oal_hw_context[index].kva_u4HWIsCompleted, *((volatile VAL_UINT32_T*)oal_hw_context[index].kva_u4HWIsCompleted)); 
    if ((((volatile VAL_UINT32_T*)oal_hw_context[index].kva_u4HWIsCompleted) == NULL) || (((volatile VAL_UINT32_T*)oal_hw_context[index].kva_u4HWIsTimeout) == NULL))
    {
        MFV_LOGE(" @@ [ERROR][ISR] index = %d, please check!!\n", index);
        // ACK interrupt 
		// decoder
      	reg_val = VDO_HW_READ(KVA_VDEC_INT_STA_ADDR);
        VDO_HW_WRITE(KVA_VDEC_INT_ACK_ADDR, reg_val);
      	// encoder
	    VDO_HW_WRITE(KVA_ENC_INT_ACK_ADDR, 1);
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
	    
        return;
    }
    *((volatile VAL_UINT32_T*)oal_hw_context[index].kva_u4HWIsCompleted) = 1;
    *((volatile VAL_UINT32_T*)oal_hw_context[index].kva_u4HWIsTimeout) = 0;

    for(i=0; i < maxnum ; i++ )
    {                   
        //MFV_DEBUG("[BEFORE] ISR read: [%d]  User_va=0x%x kva=0x%x 0x%x \n", i ,
    	//*((volatile VAL_UINT32_T*)oal_hw_context[index].kva_Oal_HW_mem_reg + i*2),
    	//oal_hw_context[index].oalmem_status[i].u4ReadAddr, 
    	//*((volatile VAL_UINT32_T*)oal_hw_context[index].kva_Oal_HW_mem_reg + i*2 + 1));
            
    	*((volatile VAL_UINT32_T*)oal_hw_context[index].kva_Oal_HW_mem_reg + i*2 + 1) = *((volatile VAL_UINT32_T*)oal_hw_context[index].oalmem_status[i].u4ReadAddr);

        if (maxnum == 3)
        {
            if (i == 0)
            {
                u4IRQStatus = (*((volatile VAL_UINT32_T*)oal_hw_context[index].kva_Oal_HW_mem_reg + i*2 + 1));
                if (u4IRQStatus != 2)
                {                    
                    MFV_LOGE("[ERROR][ISR] IRQ status error u4IRQStatus = %d\n", u4IRQStatus);
                }
            }

            if (u4IRQStatus != 2)
            {
                MFV_LOGE("[ERROR] %d, %x, %d, %d, %d, %d\n", 
                    i, 
                    ((volatile VAL_UINT32_T*)oal_hw_context[index].oalmem_status[i].u4ReadAddr),
                    (*((volatile VAL_UINT32_T*)oal_hw_context[index].kva_Oal_HW_mem_reg + i*2 + 1)),
                    VDO_HW_READ(KVA_VENC_IRQ_STATUS_ADDR),
                    VDO_HW_READ(KVA_VENC_ZERO_COEF_COUNT_ADDR),
                    VDO_HW_READ(KVA_VENC_BYTE_COUNT_ADDR));
            }
        }
        
        //MFV_DEBUG("[AFTER] ISR read: [%d]  User_va=0x%x kva=0x%x 0x%x \n", i ,
    	//*((volatile VAL_UINT32_T*)oal_hw_context[index].kva_Oal_HW_mem_reg + i*2),
    	//oal_hw_context[index].oalmem_status[i].u4ReadAddr, 
    	//*((volatile VAL_UINT32_T*)oal_hw_context[index].kva_Oal_HW_mem_reg + i*2 + 1) /*oal_hw_context[index].oalmem_status[i].u4ReadData*/);
    }
    
    eValRet = eVideoSetEvent(&oal_hw_context[index].IsrEvent, sizeof(VAL_EVENT_T));
    spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
    
    if(VAL_RESULT_NO_ERROR != eValRet)
    {
        MFV_LOGE("[MFV][ERROR] ISR set IsrEvent error\n");
        //return;
    }            	

    // ACK interrupt 
    // decoder
    reg_val = VDO_HW_READ(KVA_VDEC_INT_STA_ADDR);
    VDO_HW_WRITE(KVA_VDEC_INT_ACK_ADDR, reg_val);
    // encoder
    VDO_HW_WRITE(KVA_ENC_INT_ACK_ADDR, 1);



#if 1
        //mutex_lock(&VcodecHWLock);
        spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);

#if 1   //VCODEC_MULTI_THREAD
        //for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
        //{
            //if (grVcodecHWLock.u4VCodecThreadID[u4I] == current->pid)
            //{
                for (u4I = 0; u4I < VCODEC_THREAD_MAX_NUM; u4I++)
                {
                    grVcodecHWLock.u4VCodecThreadID[u4I] = -1;
                }
                grVcodecHWLock.u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
                grVcodecHWLock.rLockedTime.u4Sec = 0;
                grVcodecHWLock.rLockedTime.u4uSec = 0;
                disable_irq_nosync(MT_VENC_IRQ_ID); 
                disable_irq_nosync(MT_VDEC_IRQ_ID);                     

                eValHWLockRet = eVideoSetEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
                if(VAL_RESULT_NO_ERROR != eValHWLockRet)
                {
                    MFV_LOGE("[MFV][ERROR] ISR set MT6572_HWLockEvent error\n");
                }
                //break;
            //}
        //}
#else
        //if ((grVcodecHWLock.u4LockHWTID1 == current->pid) || (grVcodecHWLock.u4LockHWTID2 == current->pid)) //normal case
        //{
            grVcodecHWLock.u4LockHWTID1 = -1;
            grVcodecHWLock.u4LockHWTID2 = -1;
            grVcodecHWLock.rLockedTime.u4Sec = 0;
            grVcodecHWLock.rLockedTime.u4uSec = 0;
            disable_irq_nosync(MT_VENC_IRQ_ID); 
            disable_irq_nosync(MT_VDEC_IRQ_ID);

            eValHWLockRet = eVideoSetEvent(&MT6575_HWLockEvent, sizeof(VAL_EVENT_T));
            if(VAL_RESULT_NO_ERROR != eValHWLockRet)
            {
                MFV_LOGE("[MFV][ERROR] ISR set MT6575_HWLockEvent error\n");
            }
        //}
#endif        

        //mutex_unlock(&VcodecHWLock);
        spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);
#endif

    //MFV_DEBUG("MT6572_Video_ISR-\n");    
}

static irqreturn_t mt6572_video_intr_dlr(int irq, void *dev_id)
{   
    MT6572_Video_ISR(VCODEC_IRQ_VDEC);     //decode
    return IRQ_HANDLED;
}

static irqreturn_t mt6572_video_intr_dlr2(int irq, void *dev_id)
{   
    MT6572_Video_ISR(VCODEC_IRQ_VENC);     //encode
    return IRQ_HANDLED;
}

#if 0
int __cache_maint_all(int direction)
{
        void (*outer_op_all)(void);
      
        switch (direction) {
        case DMA_FROM_DEVICE:           /* invalidate only, HW write to memory */
                outer_op_all = outer_flush_all;  //outer_inv_all is not allowed !!!!
                break;
        case DMA_TO_DEVICE:             /* writeback only, HW read from memory */
                outer_op_all = outer_clean_all;
                break;
        case DMA_BIDIRECTIONAL:         /* writeback and invalidate */
                outer_op_all = outer_flush_all;
                break;
        default:
                BUG();
        }    

        // L1 cache maintenance when going to devices
        __cpuc_flush_kern_all();

        // L2 cache maintenance by physical pages
        outer_op_all();
  
        return 0;
}

static int pmem_remap_pte_range_video(struct mm_struct *mm, pmd_t *pmd,
			unsigned long addr, unsigned long end,
			unsigned long pfn, pgprot_t prot)
{
	pte_t *pte;
	spinlock_t *ptl;

	pte = pte_offset_map_lock(mm, pmd, addr, &ptl);
	if (!pte_present(*pte)) {
		BUG_ON("pte doesn't exist\n");
		return -1;
	}
	arch_enter_lazy_mmu_mode();
	do {
		set_pte_at(mm, addr, pte, pte_mkspecial(pfn_pte(pfn, prot)));
		pfn++;
	} while (pte++, addr += PAGE_SIZE, addr != end);
	arch_leave_lazy_mmu_mode();
	pte_unmap_unlock(pte - 1, ptl);
	return 0;
}

static inline int pmem_remap_pmd_range_video(struct mm_struct *mm, pud_t *pud,
			unsigned long addr, unsigned long end,
			unsigned long pfn, pgprot_t prot)
{
	pmd_t *pmd;
	unsigned long next;

	pfn -= addr >> PAGE_SHIFT;
	pmd = pmd_offset(pud, addr);
	if(pmd_none(*pmd)||pmd_bad(*pmd)) {
		return -1;
	}
	do {
		next = pmd_addr_end(addr, end);
		MFV_LOGD("pmem_remap_pte_range: %08x\n", (unsigned int)addr);
		if (pmem_remap_pte_range_video(mm, pmd, addr, next,
				pfn + (addr >> PAGE_SHIFT), prot))
			return -ENOMEM;
	} while (pmd++, addr = next, addr != end);
	return 0;
}

static inline int pmem_remap_pud_range_video(struct mm_struct *mm, pgd_t *pgd,
			unsigned long addr, unsigned long end,
			unsigned long pfn, pgprot_t prot)
{
	pud_t *pud;
	unsigned long next;

	pfn -= addr >> PAGE_SHIFT;
	pud = pud_offset(pgd, addr);
	if(pud_none(*pud)||pud_bad(*pud)) {
		return -1;
	}
	do {
		next = pud_addr_end(addr, end);
		MFV_LOGD("pmem_remap_pmd_range: %08x\n", (unsigned int)addr);
		if (pmem_remap_pmd_range_video(mm, pud, addr, next,
				pfn + (addr >> PAGE_SHIFT), prot))
			return -ENOMEM;
	} while (pud++, addr = next, addr != end);
	return 0;
}

int pmem_remap_dist_pfn_range_video(struct vm_area_struct *vma, unsigned long addr,
             unsigned long pfn, unsigned long size, pgprot_t prot)
{
     pgd_t *pgd;
     unsigned long next;
     unsigned long end = addr + PAGE_ALIGN(size);
     struct mm_struct *mm = vma->vm_mm;
     int err;

     /*
      * Physically remapped pages are special. Tell the
      * rest of the world about it:
      *   VM_IO tells people not to look at these pages
      *  (accesses can have side effects).
      *   VM_RESERVED is specified all over the place, because
      *  in 2.4 it kept swapout's vma scan off this vma; but
      *  in 2.6 the LRU scan won't even find its pages, so this
      *  flag means no more than count its pages in reserved_vm,
      *  and omit it from core dump, even when VM_IO turned off.
      *   VM_PFNMAP tells the core MM that the base pages are just
      *  raw PFN mappings, and do not have a "struct page" associated
      *  with them.
      *
      * There's a horrible special case to handle copy-on-write
      * behaviour that some programs depend on. We mark the "original"
      * un-COW'ed pages by matching them up with "vma->vm_pgoff".
      */
     /*
     if (addr == vma->vm_start && end == vma->vm_end) {
         vma->vm_pgoff = pfn;
         vma->vm_flags |= VM_PFN_AT_MMAP;
     } else if (is_cow_mapping(vma->vm_flags))
         return -EINVAL;
     */

     vma->vm_flags |= VM_IO | VM_RESERVED;

     //err = track_pfn_vma_new(vma, &prot, pfn, PAGE_ALIGN(size));
     //if (err) {
         /*
          * To indicate that track_pfn related cleanup is not
          * needed from higher level routine calling unmap_vmas
          */
     /*  vma->vm_flags &= ~(VM_IO | VM_RESERVED | VM_PFNMAP);
         vma->vm_flags &= ~VM_PFN_AT_MMAP;
         return -EINVAL;
     }*/

     BUG_ON(addr >= end);
     pfn -= addr >> PAGE_SHIFT;
     pgd = pgd_offset(mm, addr);
     //flush_cache_range(vma, addr, end);
     dmac_map_area((unsigned char*)addr, (end - addr), DMA_TO_DEVICE);  //L1
     outer_flush_range(addr, end);  //L2
     do {
         next = pgd_addr_end(addr, end);
         MFV_LOGD("pmem_remap_pud_range: %08x\n", (unsigned int)addr);
         err = pmem_remap_pud_range_video(mm, pgd, addr, next,
                 pfn + (addr >> PAGE_SHIFT), prot);
         if (err)
             break;
     } while (pgd++, addr = next, addr != end);

     /*if (err)
         untrack_pfn_vma(vma, pfn, PAGE_ALIGN(size));
     */

     return err;
}

unsigned int pmem_user_v2p_video(unsigned int va)
{
    unsigned int pageOffset = (va & (PAGE_SIZE - 1)); 
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;
    unsigned int pa;
    
    pgd = pgd_offset(current->mm, va); /* what is tsk->mm */
    if(pgd_none(*pgd)||pgd_bad(*pgd))
    {    
        MFV_LOGD("warning: pmem_user_v2p(), va=0x%x, pgd invalid! \n", va); 
        return 0;
    }    
    
    pmd = pmd_offset(pgd, va); 
    if(pmd_none(*pmd)||pmd_bad(*pmd))
    {    
        MFV_LOGD("warning: pmem_user_v2p(), va=0x%x, pmd invalid! \n", va); 
        return 0;
    }    
      
    pte = pte_offset_map(pmd, va); 
    if(pte_present(*pte)) 
    {    
        pa=(pte_val(*pte) & (PAGE_MASK)) | pageOffset; 
        return pa;  
    }     

    MFV_LOGD("warning: pmem_user_v2p(), va=0x%x, pte invalid! \n", va); 
    return 0;
}

int __cache_maint_range(const void *start, size_t size, int direction)
{
        void (*outer_op)(unsigned long, unsigned long);
        //void (*outer_op_all)(void);
        unsigned int page_start, page_num;        
        unsigned int phy_start;
      
        switch (direction) {
        case DMA_FROM_DEVICE:           /* invalidate only, HW write to memory */
                outer_op = outer_inv_range;
                break;
        case DMA_TO_DEVICE:             /* writeback only, HW read from memory */
                outer_op = outer_clean_range;
                break;
        case DMA_BIDIRECTIONAL:         /* writeback and invalidate */
                outer_op = outer_flush_range;
                break;
        default:
                BUG();
        }    

        // L1 cache maintenance when going to devices
        if (direction != DMA_FROM_DEVICE) 
                dmac_map_area(start, size, direction);

        // L2 cache maintenance by physical pages
        page_start = (unsigned int)start & 0xfffff000;
        page_num = (size + ((unsigned int)start & 0xfff)) / PAGE_SIZE;
        if(((unsigned int)start + size) & 0xfff)
                page_num++;

        page_start = (unsigned int)start & 0xfffff000;
        phy_start = pmem_user_v2p_video(page_start);
        if(0 == phy_start)
                MFV_LOGE("!Error: pmem_user_v2p() return 0 in cache_maint(), start=0x%x", (unsigned int)start);

        //MFV_LOGD("outer_op: phy_start=0x%08x page_num: %d\n", phy_start, page_num);
        outer_op(phy_start, phy_start + page_num*PAGE_SIZE);
  
        // L1 cache maintenance when going back from devices
        if (direction != DMA_TO_DEVICE) 
            dmac_unmap_area(start, size, direction);

        return 0;
}
#endif

static long mflexvideo_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{   
    int ret;
    VAL_UINT8_T *user_data_addr;
    VAL_RESULT_T  eValRet;
    VAL_RESULT_T  eValHWLockRet = VAL_RESULT_INVALID_ISR;
    VAL_MEMORY_T  rTempMem;
    VAL_VCODEC_THREAD_ID_T rTempTID;
    VAL_TIME_T rCurTime;
    VAL_UINT32_T u4TimeInterval;
    VAL_HW_LOCK_T rHWLock;
    VAL_BOOL_T  bLockedHW = VAL_FALSE;
    VAL_UINT32_T u4Index = 0xff;
    VAL_INT32_T i4Index;
    int index;
    int FirstUseHW = 0;
    VAL_ISR_T  val_isr;
    unsigned long ulFlags, ulFlagsISR, ulFlagsLockHW, ulFlagsVcodecHWLock;
    VAL_VCODEC_CORE_LOADING_T rTempCoreLoading;
#if defined(CONFIG_SMP)    
    VAL_VCODEC_CPU_OPP_LIMIT_T rCpuOppLimit;
#endif
    int temp_nr_cpu_ids;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4TempVCodecThreadNum;
    VAL_UINT32_T u4TempVCodecThreadID[VCODEC_THREAD_MAX_NUM];    
#else
    VAL_UINT32_T u4oal_hw_context_tid1;
    VAL_UINT32_T u4oal_hw_context_tid2;
#endif
    VAL_UINT32_T *pu4TempKVA;
    VAL_UINT32_T u4TempKPA;
    unsigned int reg_val;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
    VAL_UINT32_T u4I;
#endif
    
	switch(cmd)
    {
    case VCODEC_SET_SYSRAM_INFO:
        MFV_LOGD("VCODEC_SET_SYSRAM_INFO + tid = %d\n", current->pid);

        mutex_lock(&IntMemLock);
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_from_user(&grTempIntMem, user_data_addr, sizeof(VAL_INTMEM_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_SET_SYSRAM_INFO, copy_from_user failed: %d\n", ret);
            mutex_unlock(&IntMemLock);
        	return -EFAULT;
        }

        MFV_LOGD("[VCODEC_SET_SYSRAM_INFO] pvMemVa = 0x%x, pvMemPa = 0x%x, pvReserved = %d, u4MemSize = %d",
            (VAL_UINT32_T)grTempIntMem.pvMemVa, (VAL_UINT32_T)grTempIntMem.pvMemPa, 
            (VAL_UINT32_T)grTempIntMem.pvReserved, (VAL_UINT32_T)grTempIntMem.u4MemSize);
        mutex_unlock(&IntMemLock);

        MFV_LOGD("VCODEC_SET_SYSRAM_INFO - tid = %d\n", current->pid);
    break;

    case VCODEC_GET_SYSRAM_INFO:
        MFV_LOGD("VCODEC_GET_SYSRAM_INFO + tid = %d\n", current->pid);

        mutex_lock(&IntMemLock);
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_to_user(user_data_addr, &grTempIntMem, sizeof(VAL_INTMEM_T));
        if (ret)
        {
        	MFV_LOGD("[ERROR] VCODEC_GET_SYSRAM_INFO, copy_from_user failed: %d\n", ret);
            mutex_unlock(&IntMemLock);
        	return -EFAULT;
        }

        MFV_LOGD("[VCODEC_GET_SYSRAM_INFO] pvMemVa = 0x%x, pvMemPa = 0x%x, pvReserved = %d, u4MemSize = %d",
            (VAL_UINT32_T)grTempIntMem.pvMemVa, (VAL_UINT32_T)grTempIntMem.pvMemPa, 
            (VAL_UINT32_T)grTempIntMem.pvReserved, (VAL_UINT32_T)grTempIntMem.u4MemSize);
        mutex_unlock(&IntMemLock);

        MFV_LOGD("VCODEC_GET_SYSRAM_INFO - tid = %d\n", current->pid);
    break;

    case VCODEC_SET_THREAD_ID:
        MFV_LOGD("VCODEC_SET_THREAD_ID + tid = %d\n", current->pid);

        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_from_user(&rTempTID, user_data_addr, sizeof(VAL_VCODEC_THREAD_ID_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_SET_THREAD_ID, copy_from_user failed: %d\n", ret);
        	return -EFAULT;
        }

        spin_lock_irqsave(&OalHWContextLock, ulFlags);
        setCurr_HWLockSlot_Thread_ID(rTempTID, &u4Index);
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);

        if (u4Index == 0xff)
        {
            MFV_LOGE("[ERROR] MT6572_VCODEC_SET_THREAD_ID error, u4Index = %d\n", u4Index);
        }
        mutex_lock(&VcodecHWUserLock);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        grVcodecHWUser[u4Index].u4VCodecThreadNum = rTempTID.u4VCodecThreadNum;
        for (u4I = 0; u4I < grVcodecHWUser[u4Index].u4VCodecThreadNum; u4I++)
        {
            grVcodecHWUser[u4Index].u4VCodecThreadID[u4I] = rTempTID.u4VCodecThreadID[u4I];
            MFV_LOGW("[INFO] VCodecThreadIDNum = %d, VCodecThreadID[%d] = %d\n",
                grVcodecHWUser[u4Index].u4VCodecThreadNum,
                u4I,
                grVcodecHWUser[u4Index].u4VCodecThreadID[u4I]
                );
        }
#else
        grVcodecHWUser[u4Index].u4tid1 = rTempTID.u4tid1;
        grVcodecHWUser[u4Index].u4tid2 = rTempTID.u4tid2;
#endif
        mutex_unlock(&VcodecHWUserLock);
        
        MFV_LOGD("VCODEC_SET_THREAD_ID - tid = %d\n", current->pid);
    break;

    case VCODEC_ALLOC_NON_CACHE_BUFFER:
        MFV_LOGD("[M4U]! VCODEC_ALLOC_NON_CACHE_BUFFER + tid = %d\n", current->pid);
        	
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_from_user(&rTempMem, user_data_addr, sizeof(VAL_MEMORY_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_ALLOC_NON_CACHE_BUFFER, copy_from_user failed: %d\n", ret);
        	return -EFAULT;
        } 

        rTempMem.u4ReservedSize /*kernel va*/ = (unsigned int)dma_alloc_coherent(0, rTempMem.u4MemSize, (dma_addr_t *)&rTempMem.pvMemPa, GFP_KERNEL);
        if((0 == rTempMem.u4ReservedSize) || (0 == rTempMem.pvMemPa))
        {
        	  MFV_LOGE("[ERROR] dma_alloc_coherent fail in VCODEC_ALLOC_NON_CACHE_BUFFER\n");
        	  return -EFAULT;
        }   

        MFV_LOGD("kernel va = 0x%x, kernel pa = 0x%x, memory size = %d\n", 
            (unsigned int)rTempMem.u4ReservedSize, (unsigned int)rTempMem.pvMemPa, (unsigned int)rTempMem.u4MemSize);            

        spin_lock_irqsave(&OalHWContextLock, ulFlags);
        u4Index = search_HWLockSlot_ByTID(0, current->pid);
        if (u4Index == -1)
        {
            MFV_LOGE("[ERROR] Add_NonCacheMemoryList error, u4Index = -1\n");
            break;
        }
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        u4TempVCodecThreadNum = oal_hw_context[u4Index].u4VCodecThreadNum;
        for (u4I = 0; u4I < u4TempVCodecThreadNum; u4I++)
        {
            u4TempVCodecThreadID[u4I] = oal_hw_context[u4Index].u4VCodecThreadID[u4I];
        }
#else
        u4oal_hw_context_tid1 = oal_hw_context[u4Index].tid1;
        u4oal_hw_context_tid2 = oal_hw_context[u4Index].tid2;
#endif
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);

        mutex_lock(&NonCacheMemoryListLock);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        Add_NonCacheMemoryList(rTempMem.u4ReservedSize, (VAL_UINT32_T)rTempMem.pvMemPa, (VAL_UINT32_T)rTempMem.u4MemSize, u4TempVCodecThreadNum, u4TempVCodecThreadID);
#else
        Add_NonCacheMemoryList(rTempMem.u4ReservedSize, (VAL_UINT32_T)rTempMem.pvMemPa, (VAL_UINT32_T)rTempMem.u4MemSize, u4oal_hw_context_tid1, u4oal_hw_context_tid2);
#endif
        mutex_unlock(&NonCacheMemoryListLock);
        
        ret = copy_to_user(user_data_addr, &rTempMem, sizeof(VAL_MEMORY_T));
        if(ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_ALLOC_NON_CACHE_BUFFER, copy_to_user failed: %d\n", ret);
        	return -EFAULT;
        }  

        MFV_LOGD("[M4U]! VCODEC_ALLOC_NON_CACHE_BUFFER - tid = %d\n", current->pid);
    break;

    case VCODEC_FREE_NON_CACHE_BUFFER:
        MFV_LOGD("[M4U]! VCODEC_FREE_NON_CACHE_BUFFER + tid = %d\n", current->pid);
        	
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_from_user(&rTempMem, user_data_addr, sizeof(VAL_MEMORY_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_FREE_NON_CACHE_BUFFER, copy_from_user failed: %d\n", ret);
        	return -EFAULT;
        } 

        dma_free_coherent(0, rTempMem.u4MemSize, (void *)rTempMem.u4ReservedSize, (dma_addr_t)rTempMem.pvMemPa);

        mutex_lock(&NonCacheMemoryListLock);
        Free_NonCacheMemoryList(rTempMem.u4ReservedSize, (VAL_UINT32_T)rTempMem.pvMemPa);
        mutex_unlock(&NonCacheMemoryListLock);

        rTempMem.u4ReservedSize = 0;
        rTempMem.pvMemPa = NULL;

        ret = copy_to_user(user_data_addr, &rTempMem, sizeof(VAL_MEMORY_T));
        if(ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_FREE_NON_CACHE_BUFFER, copy_to_user failed: %d\n", ret);
        	return -EFAULT;
        }  

        MFV_LOGD("[M4U]! VCODEC_FREE_NON_CACHE_BUFFER - tid = %d\n", current->pid);
    break;

    case VCODEC_INC_ENC_EMI_USER:
        MFV_LOGD("VCODEC_INC_ENC_EMI_USER + tid = %d\n", current->pid);

        mutex_lock(&EncEMILock);
        gu4EncEMICounter++;
        MFV_LOGE("ENC_EMI_USER = %d\n", gu4EncEMICounter);
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_to_user(user_data_addr, &gu4EncEMICounter, sizeof(VAL_UINT32_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_INC_ENC_EMI_USER, copy_to_user failed: %d\n", ret);
            mutex_unlock(&EncEMILock);
        	return -EFAULT;
        }
        mutex_unlock(&EncEMILock);

        MFV_LOGD("VCODEC_INC_ENC_EMI_USER - tid = %d\n", current->pid);            
    break;

    case VCODEC_DEC_ENC_EMI_USER:
        MFV_LOGD("VCODEC_DEC_ENC_EMI_USER + tid = %d\n", current->pid);

        mutex_lock(&EncEMILock);
        gu4EncEMICounter--;
        MFV_LOGE("ENC_EMI_USER = %d\n", gu4EncEMICounter);
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_to_user(user_data_addr, &gu4EncEMICounter, sizeof(VAL_UINT32_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_DEC_ENC_EMI_USER, copy_to_user failed: %d\n", ret);
            mutex_unlock(&EncEMILock);
        	return -EFAULT;
        }
        mutex_unlock(&EncEMILock);

        MFV_LOGD("VCODEC_DEC_ENC_EMI_USER - tid = %d\n", current->pid);            
    break;

    case VCODEC_INC_DEC_EMI_USER:
        MFV_LOGD("VCODEC_INC_DEC_EMI_USER + tid = %d\n", current->pid);

        mutex_lock(&DecEMILock);
        gu4DecEMICounter++;
        MFV_LOGE("DEC_EMI_USER = %d\n", gu4DecEMICounter);
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_to_user(user_data_addr, &gu4DecEMICounter, sizeof(VAL_UINT32_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_INC_DEC_EMI_USER, copy_to_user failed: %d\n", ret);
            mutex_unlock(&DecEMILock);
        	return -EFAULT;
        }
        mutex_unlock(&DecEMILock);

        MFV_LOGD("VCODEC_INC_DEC_EMI_USER - tid = %d\n", current->pid);            
    break;

    case VCODEC_DEC_DEC_EMI_USER:
        MFV_LOGD("VCODEC_DEC_DEC_EMI_USER + tid = %d\n", current->pid);

        mutex_lock(&DecEMILock);
        gu4DecEMICounter--;
        MFV_LOGE("DEC_EMI_USER = %d\n", gu4DecEMICounter);
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_to_user(user_data_addr, &gu4DecEMICounter, sizeof(VAL_UINT32_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_DEC_DEC_EMI_USER, copy_to_user failed: %d\n", ret);
            mutex_unlock(&DecEMILock);
        	return -EFAULT;
        }
        mutex_unlock(&DecEMILock);

        MFV_LOGD("VCODEC_DEC_DEC_EMI_USER - tid = %d\n", current->pid);            
    break;

    case VCODEC_INC_SYSRAM_USER:
        MFV_LOGD("VCODEC_INC_SYSRAM_USER + tid = %d\n", current->pid);

        mutex_lock(&SysramUserLock);

        spin_lock_irqsave(&OalHWContextLock, ulFlags);
        i4Index = search_HWLockSlot_ByTID(0, current->pid);
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        if (i4Index == -1)
        {
            MFV_LOGE("[ERROR] Never call VCODEC_SET_THREAD_ID before, please check!!\n");
            mutex_unlock(&SysramUserLock);
            return -EFAULT;
        }
        
        spin_lock_irqsave(&OalHWContextLock, ulFlags);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        u4TempVCodecThreadNum = oal_hw_context[i4Index].u4VCodecThreadNum;
        MFV_LOGD("u4TempVCodecThreadNum = %d\n", u4TempVCodecThreadNum);
        for (u4I = 0; u4I < u4TempVCodecThreadNum; u4I++)
        {
            u4TempVCodecThreadID[u4I] = oal_hw_context[i4Index].u4VCodecThreadID[u4I];
            MFV_LOGD("u4TempVCodecThreadID[u4I] = %d\n", u4TempVCodecThreadID[u4I]);
        }
#else
        u4oal_hw_context_tid1 = oal_hw_context[i4Index].tid1;
        u4oal_hw_context_tid2 = oal_hw_context[i4Index].tid2;
#endif
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        
        mutex_lock(&VcodecSysramUserLock);

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        Add_SysramUserList(u4TempVCodecThreadNum, u4TempVCodecThreadID);
#else
        Add_SysramUserList(u4oal_hw_context_tid1, u4oal_hw_context_tid2);   
#endif
        mutex_unlock(&VcodecSysramUserLock);
        
        MFV_LOGE("[VCODEC_INC_SYSRAM_USER] SYSRAM_USER = %d\n", gu4INTMEMCounter);
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_to_user(user_data_addr, &gu4INTMEMCounter, sizeof(VAL_UINT32_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_INC_SYSRAM_USER, copy_to_user failed: %d\n", ret);
            mutex_unlock(&SysramUserLock);
        	return -EFAULT;
        }
        mutex_unlock(&SysramUserLock);

        MFV_LOGD("VCODEC_INC_SYSRAM_USER - tid = %d\n", current->pid);            
    break;

    case VCODEC_DEC_SYSRAM_USER:
        MFV_LOGD("VCODEC_DEC_SYSRAM_USER + tid = %d\n", current->pid);

        mutex_lock(&SysramUserLock);
        mutex_lock(&VcodecSysramUserLock);
        Free_SysramUserList(current->pid, VAL_FALSE);
        mutex_unlock(&VcodecSysramUserLock);
        MFV_LOGE("[VCODEC_DEC_SYSRAM_USER] SYSRAM_USER = %d\n", gu4INTMEMCounter);
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_to_user(user_data_addr, &gu4INTMEMCounter, sizeof(VAL_UINT32_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_DEC_SYSRAM_USER, copy_to_user failed: %d\n", ret);
            mutex_unlock(&SysramUserLock);
        	return -EFAULT;
        }
        mutex_unlock(&SysramUserLock);

        MFV_LOGD("VCODEC_DEC_SYSRAM_USER - tid = %d\n", current->pid);            
    break;

    case VCODEC_LOCKHW:
    	MFV_LOGD("VCODEC_LOCKHW + tid = %d\n", current->pid); 
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_from_user(&rHWLock, user_data_addr, sizeof(VAL_HW_LOCK_T));
        if (ret)
        {
            MFV_LOGE("[ERROR] VCODEC_LOCKHW, copy_from_user failed: %d\n", ret);
            return -EFAULT;
        } 

        while (bLockedHW == VAL_FALSE)
        {
            mutex_lock(&HWLockEventTimeoutLock);
            if (MT6572_HWLockEvent.u4TimeoutMs == 1)
            {
                MFV_LOGE("[NOT ERROR][VCODEC_LOCKHW] First Use HW!!\n");   
                FirstUseHW = 1;
                eValHWLockRet = eVideoWaitEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
                MT6572_HWLockEvent.u4TimeoutMs = 1000;
            }
            else
            {
                FirstUseHW = 0;
            }
            mutex_unlock(&HWLockEventTimeoutLock);

            //mutex_lock(&VcodecHWLock);
            spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            // one process try to lock twice
            if (grVcodecHWLock.u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM)
            {
                for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
                {
                    if (grVcodecHWLock.u4VCodecThreadID[u4I] == current->pid)
                    {
                        MFV_LOGE("[WARNING] one process try to lock twice, may cause lock HW timeout!! LockHWTID = %d, CurrentTID = %d\n", 
                        grVcodecHWLock.u4VCodecThreadID[u4I], current->pid);
                    }
                }
            }
#else
            // one process try to lock twice
            if ((grVcodecHWLock.u4LockHWTID1 == current->pid) || (grVcodecHWLock.u4LockHWTID2 == current->pid))
            {
                MFV_LOGE("[WARNING] one process try to lock twice, may cause lock HW timeout!! LockHWTID1 = %d, LockHWTID2 = %d, CurrentTID = %d\n", 
                    grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2, current->pid);
            }
#endif
            //mutex_unlock(&VcodecHWLock);
            spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);

            if (FirstUseHW == 0)
            {
                eValHWLockRet = eVideoWaitEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
            }

            if(VAL_RESULT_INVALID_ISR == eValHWLockRet)
            {
                MFV_LOGE("[ERROR][VCODEC_LOCKHW] MT6572_HWLockEvent TimeOut, CurrentTID = %d\n", current->pid);
                if (FirstUseHW == 1)
                {                    
                }
                else
                {   
                    //mutex_lock(&VcodecHWLock);
                    spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                    if (grVcodecHWLock.u4VCodecThreadNum == VCODEC_THREAD_MAX_NUM)
#else
                    if ((grVcodecHWLock.u4LockHWTID1 == -1) && (grVcodecHWLock.u4LockHWTID2 == -1))
#endif
                    {
                        MFV_LOGE("[WARNING] maybe mediaserver restart before, please check!!\n");
                    }
                    else
                    {
                        MFV_LOGE("[WARNING] someone use HW, and check timeout value!!\n");
                    }
                    //mutex_unlock(&VcodecHWLock);
                    spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);
                }
            }
            else if (VAL_RESULT_RESTARTSYS == eValHWLockRet)
            {
                MFV_LOGE("[WARNING] mediaserver is signaled and need to restart system call!!\n");
                return -ERESTARTSYS;
            }

            //mutex_lock(&VcodecHWLock);
            spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            if (grVcodecHWLock.u4VCodecThreadNum == VCODEC_THREAD_MAX_NUM)     //No process use HW, so current process can use HW
#else
            if ((grVcodecHWLock.u4LockHWTID1 == -1) && (grVcodecHWLock.u4LockHWTID2 == -1))   //No process use HW, so current process can use HW
#endif
            {
                spin_lock_irqsave(&OalHWContextLock, ulFlags);
                u4Index = search_HWLockSlot_ByTID(0, current->pid);
                spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
                
                if (u4Index == -1)
                {
                    MFV_LOGE("[ERROR][VCODEC_LOCKHW] No process use HW, so current process can use HW, u4Index = -1\n");
                    //mutex_unlock(&VcodecHWLock);
                    spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);
                    return -EFAULT;
                }

                spin_lock_irqsave(&OalHWContextLock, ulFlags);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                grVcodecHWLock.u4VCodecThreadNum = oal_hw_context[u4Index].u4VCodecThreadNum;
                for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
                {
                    grVcodecHWLock.u4VCodecThreadID[u4I] = oal_hw_context[u4Index].u4VCodecThreadID[u4I];
                }
#else
                grVcodecHWLock.u4LockHWTID1 = oal_hw_context[u4Index].tid1;
                grVcodecHWLock.u4LockHWTID2 = oal_hw_context[u4Index].tid2;
#endif
                spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
                eVideoGetTimeOfDay(&grVcodecHWLock.rLockedTime, sizeof(VAL_TIME_T));

                MFV_LOGD("No process use HW, so current process can use HW\n");
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
                {
                    MFV_LOGD("LockHWTID = %d, CurrentTID = %d, rLockedTime(s, us) = %d, %d\n",
                        grVcodecHWLock.u4VCodecThreadID[u4I], current->pid, grVcodecHWLock.rLockedTime.u4Sec, grVcodecHWLock.rLockedTime.u4uSec);
                }
#else                
                MFV_LOGD("LockHWTID1 = %d, LockHWTID2 = %d, CurrentTID = %d, rLockedTime(s, us) = %d, %d\n",
                    grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2, current->pid, grVcodecHWLock.rLockedTime.u4Sec, grVcodecHWLock.rLockedTime.u4uSec);
#endif                
                
                bLockedHW = VAL_TRUE;
                enable_irq(MT_VDEC_IRQ_ID);    
    		    enable_irq(MT_VENC_IRQ_ID);
            }
            else    //someone use HW, and check timeout value
            {
                eVideoGetTimeOfDay(&rCurTime, sizeof(VAL_TIME_T));
                u4TimeInterval = (((((rCurTime.u4Sec - grVcodecHWLock.rLockedTime.u4Sec) * 1000000) + rCurTime.u4uSec) 
                    - grVcodecHWLock.rLockedTime.u4uSec) / 1000);

                MFV_LOGD("someone use HW, and check timeout value\n");
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
                {
                    MFV_LOGD("LockHWTID = %d, CurrentTID = %d, TimeInterval(ms) = %d, TimeOutValue(ms)) = %d\n",
                        grVcodecHWLock.u4VCodecThreadID[u4I], current->pid, u4TimeInterval, rHWLock.u4TimeoutMs);
                }
#else 
                MFV_LOGD("LockHWTID1 = %d, LockHWTID2 = %d, CurrentTID = %d, TimeInterval(ms) = %d, TimeOutValue(ms)) = %d\n",
                    grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2, current->pid, u4TimeInterval, rHWLock.u4TimeoutMs);

                MFV_LOGD("LockHWTID1 = %d, LockHWTID2 = %d, CurrentTID = %d, rLockedTime(s, us) = %d, %d, rCurTime(s, us) = %d, %d\n",
                    grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2, current->pid, 
                    grVcodecHWLock.rLockedTime.u4Sec, grVcodecHWLock.rLockedTime.u4uSec,
                    rCurTime.u4Sec, rCurTime.u4uSec
                    );
#endif                

                if (u4TimeInterval >= rHWLock.u4TimeoutMs)  
                //Locked process(A) timeout, so release HW and let currnet process(B) to use HW 
                {
                    MFV_LOGE("[INFO][VCODEC_LOCKHW] Locked process(A) timeout, so release HW and let currnet process(B) to use HW\n");

                    spin_lock_irqsave(&OalHWContextLock, ulFlags);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                    ret = search_HWLockSlot_ByTID(0, grVcodecHWLock.u4VCodecThreadID[0]);
#else
                    ret = search_HWLockSlot_ByTID(0, grVcodecHWLock.u4LockHWTID1);
#endif
                    spin_unlock_irqrestore(&OalHWContextLock, ulFlags);

                    if (ret == -1)
                    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                        for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
                        {
                            MFV_LOGE("[ERROR] Locked process - ID %d fail, didn't call InitHWLock \n", grVcodecHWLock.u4VCodecThreadID[u4I]);
                        }
#else
        	            MFV_LOGE("[ERROR] Locked process - ID %d or %d fail, didn't call InitHWLock \n", 
                        grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2);
#endif
                        //mutex_unlock(&VcodecHWLock);
                        spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);
                        return -EFAULT;
                    }
                    else
                    {
                        spin_lock_irqsave(&OalHWContextLock, ulFlags);
                        *((volatile VAL_UINT32_T*)oal_hw_context[ret].kva_u4HWIsCompleted) = 1;
                        *((volatile VAL_UINT32_T*)oal_hw_context[ret].kva_u4HWIsTimeout) = 1;
                        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);

                        spin_lock_irqsave(&ISRCountLock, ulFlagsISR);
                        gu4ISRCount++;
                        spin_unlock_irqrestore(&ISRCountLock, ulFlagsISR);
                    }

                    spin_lock_irqsave(&OalHWContextLock, ulFlags);
                    u4Index = search_HWLockSlot_ByTID(0, current->pid);
                    spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
                    
                    if (u4Index == -1)
                    {
                        MFV_LOGE("[ERROR] Locked process - ID %d fail, didn't call InitHWLock\n", current->pid);
                        //mutex_unlock(&VcodecHWLock);
                        spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);
                        return -EFAULT;
                    }

                    spin_lock_irqsave(&OalHWContextLock, ulFlags);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                    grVcodecHWLock.u4VCodecThreadNum = oal_hw_context[u4Index].u4VCodecThreadNum;
                    for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
                    {
                        grVcodecHWLock.u4VCodecThreadID[u4I] = oal_hw_context[u4Index].u4VCodecThreadID[u4I];
                    }
#else
                    grVcodecHWLock.u4LockHWTID1 = oal_hw_context[u4Index].tid1;
                    grVcodecHWLock.u4LockHWTID2 = oal_hw_context[u4Index].tid2;
#endif
                    spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
                    
                    eVideoGetTimeOfDay(&grVcodecHWLock.rLockedTime, sizeof(VAL_TIME_T));

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                    for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
                    {
                        MFV_LOGD("LockHWTID = %d, CurrentTID = %d, rLockedTime(s, us) = %d, %d\n",
                            grVcodecHWLock.u4VCodecThreadID[u4I], current->pid, grVcodecHWLock.rLockedTime.u4Sec, grVcodecHWLock.rLockedTime.u4uSec);   
                    }
#else
                    MFV_LOGD("LockHWTID1 = %d, LockHWTID2 = %d, CurrentTID = %d, rLockedTime(s, us) = %d, %d\n",
                        grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2, current->pid, grVcodecHWLock.rLockedTime.u4Sec, grVcodecHWLock.rLockedTime.u4uSec);
#endif

                    bLockedHW = VAL_TRUE;
                }
            }

            if (bLockedHW == VAL_TRUE)
            {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
                for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
                {
                    MFV_LOGD("grVcodecHWLock.u4LockHWTID = %d\n", grVcodecHWLock.u4VCodecThreadID[u4I]);
                }
#else
                MFV_LOGD("grVcodecHWLock.u4LockHWTID1 = %d, grVcodecHWLock.u4LockHWTID2 = %d\n", 
                    grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2);
#endif
            }

            //mutex_unlock(&VcodecHWLock);
            spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);
        }

        spin_lock_irqsave(&LockHWCountLock, ulFlagsLockHW);
        gu4LockHWCount++;
        spin_unlock_irqrestore(&LockHWCountLock, ulFlagsLockHW);

    	MFV_LOGD("get locked - ObjId =%d\n", current->pid);

        MFV_LOGD("VCODEC_LOCKHWed - tid = %d\n", current->pid); 
        
        // add for debugging checking
        spin_lock_irqsave(&OalHWContextLock, ulFlags);
        ret = search_HWLockSlot_ByTID(0, current->pid);
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        
        if(ret == -1)
        {
        	MFV_LOGE("VCODEC_LOCKHW - ID %d  fail, didn't call InitHWLock \n", current->pid); 
        	return -EFAULT;
        }
        	      	  		
    break;

#if 0
    case VCODEC_PMEM_FLUSH: 
    	//MFV_LOGD("VCODEC_PMEM_FLUSH\n");
    	__cache_maint_all(DMA_BIDIRECTIONAL);	
	break;

    case VCODEC_PMEM_CLEAN: 
		//MFV_LOGD("VCODEC_PMEM_CLEAN\n");
		__cache_maint_all(DMA_TO_DEVICE);
		
	break;
#endif

    case VCODEC_INC_PWR_USER:
    	MFV_LOGD("[MT6572] VCODEC_INC_PWR_USER + tid = %d\n", current->pid);

        mutex_lock(&PWRLock);
        gu4PWRCounter++;
        MFV_LOGE("PWR_USER = %d\n", gu4PWRCounter);
        if (gu4PWRCounter == 1)
        {
            MFV_LOGE("[VCODEC_INC_PWR_USER] First Use HW, Enable Power!\n");
#if 0
            //(1) enable IRQ
            enable_irq(MT_VENC_IRQ_ID); 
            enable_irq(MT_VDEC_IRQ_ID);
#endif
            //(2) enable clock
			enable_clock(MT_CG_MM_CODEC_SW_CG, "VideoClock");     
			larb_clock_on(0, "VideoClock");

        }
        mutex_unlock(&PWRLock);

        MFV_LOGD("[MT6572] VCODEC_INC_PWR_USER - tid = %d\n", current->pid);
	break;

    case VCODEC_DEC_PWR_USER:
    	MFV_LOGD("[MT6572] VCODEC_DEC_PWR_USER + tid = %d\n", current->pid);

        mutex_lock(&PWRLock);
        gu4PWRCounter--;
        MFV_LOGE("PWR_USER = %d\n", gu4PWRCounter);
        if (gu4PWRCounter == 0)
        {
            MFV_LOGE("[VCODEC_DEC_PWR_USER] No One Use HW, Disable Power!\n");
#if 0
            //(1) disable IRQ
            disable_irq_nosync(MT_VENC_IRQ_ID); 
            disable_irq_nosync(MT_VDEC_IRQ_ID);
#endif
            //(2) ack interrupt            
            // decoder
            reg_val = VDO_HW_READ(KVA_VDEC_INT_STA_ADDR);
            VDO_HW_WRITE(KVA_VDEC_INT_ACK_ADDR, reg_val);
            // encoder
            VDO_HW_WRITE(KVA_ENC_INT_ACK_ADDR, 1);

            //(3) disable clock
			disable_clock(MT_CG_MM_CODEC_SW_CG, "VideoClock");     
			larb_clock_off(0, "VideoClock");
        }
        mutex_unlock(&PWRLock);
        
        MFV_LOGD("[MT6572] VCODEC_DEC_PWR_USER - tid = %d\n", current->pid);
	break;

    case VCODEC_WAITISR:
        MFV_LOGD("[MT6572] VCODEC_WAITISR + tid = %d\n", current->pid);

        spin_lock_irqsave(&OalHWContextLock, ulFlags);
        index = search_HWLockSlot_ByTID(0, current->pid);
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        
        if (index == -1)
        {
            MFV_LOGE("[ERROR] VCODEC_WAITISR Fail, tid = %d, index = -1\n", current->pid); 
            return -EFAULT;
        }
        
		MFV_LOGD("index = %d, start wait VCODEC_WAITISR TID %d \n", index, current->pid);

#if 0
        //mutex_lock(&VcodecHWLock);
        spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
        {
            MFV_LOGD("grVcodecHWLock.u4LockHWTID = %d\n", grVcodecHWLock.u4VCodecThreadID[u4I]);
        }
#else
        MFV_LOGD("grVcodecHWLock.u4LockHWTID1 = %d, grVcodecHWLock.u4LockHWTID2 = %d\n", 
            grVcodecHWLock.u4LockHWTID1, grVcodecHWLock.u4LockHWTID2);
#endif

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
        {
            if (grVcodecHWLock.u4VCodecThreadID[u4I] == current->pid)
            {
                bLockedHW = VAL_TRUE;
                break;
            }
        }
        
        if (u4I == grVcodecHWLock.u4VCodecThreadNum)
        {
            //do not disable irq, becaule other process is using irq 
        }
#else
        if ((grVcodecHWLock.u4LockHWTID1 == current->pid) || (grVcodecHWLock.u4LockHWTID2 == current->pid)) //normal case
        {
            bLockedHW = VAL_TRUE;
        }
        else    //current process can not use HW
        {
            //do not disable irq, becaule other process is using irq 
        }
#endif
      
        //mutex_unlock(&VcodecHWLock);
        spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);

        if (bLockedHW == VAL_FALSE)
        {
            MFV_LOGE("[ERROR] DO NOT have HWLock, so return fail\n");
            spin_lock_irqsave(&OalHWContextLock, ulFlags);
            oal_hw_context[index].IsrEvent.u4TimeoutMs = 10;
            spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
			//MFV_LOGD(" isrevent timeout =%x\n", oal_hw_context[index].IsrEvent.u4TimeoutMs);
			//MFV_LOGD("MT6572_Video_ISR+\n");
            eValRet = eVideoWaitEvent(&oal_hw_context[index].IsrEvent, sizeof(VAL_EVENT_T));

            if(VAL_RESULT_INVALID_ISR == eValRet)
            {
                MFV_LOGE("[ERROR][bLockedHW == VAL_FALSE] WAIT_ISR_CMD TimeOut\n");
            }
            break;
        }
#endif

    	if(copy_from_user(&val_isr, (void __user *)arg, sizeof(VAL_ISR_T)))
        {

    		MFV_LOGE("[ERROR] copy_from_user fail\n");
#if 0
            //mutex_lock(&VcodecHWLock);
            spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
            {
                if (grVcodecHWLock.u4VCodecThreadID[u4I] == current->pid)
                {
                    for (u4I = 0; u4I < VCODEC_THREAD_MAX_NUM; u4I++)
                    {
                        grVcodecHWLock.u4VCodecThreadID[u4I] = -1;
                    }
                    grVcodecHWLock.u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
                    grVcodecHWLock.rLockedTime.u4Sec = 0;
                    grVcodecHWLock.rLockedTime.u4uSec = 0;
        			disable_irq_nosync(MT_VENC_IRQ_ID); 
        			disable_irq_nosync(MT_VDEC_IRQ_ID);           			

                    eValHWLockRet = eVideoSetEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
            	    if(VAL_RESULT_NO_ERROR != eValHWLockRet)
            	    {
            	        MFV_LOGE("[MFV][ERROR] ISR set MT6572_HWLockEvent error\n");
            	    }
                    break;
                }
            }
#else
            if ((grVcodecHWLock.u4LockHWTID1 == current->pid) || (grVcodecHWLock.u4LockHWTID2 == current->pid)) //normal case
            {
                grVcodecHWLock.u4LockHWTID1 = -1;
                grVcodecHWLock.u4LockHWTID2 = -1;
                grVcodecHWLock.rLockedTime.u4Sec = 0;
                grVcodecHWLock.rLockedTime.u4uSec = 0;
    			disable_irq_nosync(MT_VENC_IRQ_ID); 
    			disable_irq_nosync(MT_VDEC_IRQ_ID);           			

                eValHWLockRet = eVideoSetEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
        	    if(VAL_RESULT_NO_ERROR != eValHWLockRet)
        	    {
        	        MFV_LOGE("[MFV][ERROR] ISR set MT6572_HWLockEvent error\n");
        	    }
            }

#endif
            //mutex_unlock(&VcodecHWLock);
            spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);
#endif
            return -EFAULT;        			
    	} 
    	
    	//MFV_LOGD(" isrevent timeout =%x\n", oal_hw_context[index].IsrEvent.u4TimeoutMs);
    	spin_lock_irqsave(&OalHWContextLock, ulFlags);
    	oal_hw_context[index].IsrEvent.u4TimeoutMs = val_isr.u4TimeoutMs;
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
		//MFV_LOGD(" isrevent timeout =%x\n", oal_hw_context[index].IsrEvent.u4TimeoutMs);
		//MFV_LOGD("MT6572_Video_ISR+\n");
        eValRet = eVideoWaitEvent(&oal_hw_context[index].IsrEvent, sizeof(VAL_EVENT_T));
        MFV_LOGD("waitdone VCODEC_WAITISR TID %d \n", current->pid);

#if 0
        //mutex_lock(&VcodecHWLock);
        spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);

#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
        {
            if (grVcodecHWLock.u4VCodecThreadID[u4I] == current->pid)
            {
                for (u4I = 0; u4I < VCODEC_THREAD_MAX_NUM; u4I++)
                {
                    grVcodecHWLock.u4VCodecThreadID[u4I] = -1;
                }
                grVcodecHWLock.u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
                grVcodecHWLock.rLockedTime.u4Sec = 0;
                grVcodecHWLock.rLockedTime.u4uSec = 0;
                disable_irq_nosync(MT_VENC_IRQ_ID); 
                disable_irq_nosync(MT_VDEC_IRQ_ID);                     

                eValHWLockRet = eVideoSetEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
                if(VAL_RESULT_NO_ERROR != eValHWLockRet)
                {
                    MFV_LOGE("[MFV][ERROR] ISR set MT6572_HWLockEvent error\n");
                }
                break;
            }
        }
#else
        if ((grVcodecHWLock.u4LockHWTID1 == current->pid) || (grVcodecHWLock.u4LockHWTID2 == current->pid)) //normal case
        {
            grVcodecHWLock.u4LockHWTID1 = -1;
            grVcodecHWLock.u4LockHWTID2 = -1;
            grVcodecHWLock.rLockedTime.u4Sec = 0;
            grVcodecHWLock.rLockedTime.u4uSec = 0;
            disable_irq_nosync(MT_VENC_IRQ_ID); 
            disable_irq_nosync(MT_VDEC_IRQ_ID);

            eValHWLockRet = eVideoSetEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
            if(VAL_RESULT_NO_ERROR != eValHWLockRet)
            {
                MFV_LOGE("[MFV][ERROR] ISR set MT6572_HWLockEvent error\n");
            }
        }
#endif        

        //mutex_unlock(&VcodecHWLock);
        spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);
#endif
        if(VAL_RESULT_INVALID_ISR == eValRet)
        {
            MFV_LOGE("[ERROR] WAIT_ISR_CMD TimeOut\n");

            spin_lock_irqsave(&OalHWContextLock, ulFlags);
            *((volatile VAL_UINT32_T*)oal_hw_context[index].kva_u4HWIsCompleted) = 0;
            *((volatile VAL_UINT32_T*)oal_hw_context[index].kva_u4HWIsTimeout) = 1;
            spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
            
            spin_lock_irqsave(&ISRCountLock, ulFlagsISR);
            gu4ISRCount++;
            spin_unlock_irqrestore(&ISRCountLock, ulFlagsISR);
            
            // unlock HW
			spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);

#if 1   //VCODEC_MULTI_THREAD
		    for (u4I = 0; u4I < grVcodecHWLock.u4VCodecThreadNum; u4I++)
		    {
		        if (grVcodecHWLock.u4VCodecThreadID[u4I] == current->pid)
		        {
		            for (u4I = 0; u4I < VCODEC_THREAD_MAX_NUM; u4I++)
		            {
		                grVcodecHWLock.u4VCodecThreadID[u4I] = -1;
		            }
		            grVcodecHWLock.u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
		            grVcodecHWLock.rLockedTime.u4Sec = 0;
		            grVcodecHWLock.rLockedTime.u4uSec = 0;
		            disable_irq_nosync(MT_VENC_IRQ_ID); 
		            disable_irq_nosync(MT_VDEC_IRQ_ID);                     

		            eValHWLockRet = eVideoSetEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
		            if(VAL_RESULT_NO_ERROR != eValHWLockRet)
		            {
		                MFV_LOGE("[MFV][ERROR] ISR set MT6572_HWLockEvent error\n");
		            }
		            break;
		        }
		    }
#else
		    //if ((grVcodecHWLock.u4LockHWTID1 == current->pid) || (grVcodecHWLock.u4LockHWTID2 == current->pid)) //normal case
		    //{
		        grVcodecHWLock.u4LockHWTID1 = -1;
		        grVcodecHWLock.u4LockHWTID2 = -1;
		        grVcodecHWLock.rLockedTime.u4Sec = 0;
		        grVcodecHWLock.rLockedTime.u4uSec = 0;
		        disable_irq_nosync(MT_VENC_IRQ_ID); 
		        disable_irq_nosync(MT_VDEC_IRQ_ID);

		        eValHWLockRet = eVideoSetEvent(&MT6575_HWLockEvent, sizeof(VAL_EVENT_T));
		        if(VAL_RESULT_NO_ERROR != eValHWLockRet)
		        {
		            MFV_LOGE("[MFV][ERROR] ISR set MT6575_HWLockEvent error\n");
		        }
		    //}
#endif        
		    spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);			
            
            return -2;
            //bIsrEventInit = VAL_FALSE;
        }
        else if (VAL_RESULT_RESTARTSYS == eValHWLockRet)
        {
            MFV_LOGE("[WARNING] mediaserver is signaled and need to restart system call!!\n");
            return -ERESTARTSYS;
        }
            
        MFV_LOGD("[MT6572] VCODEC_WAITISR - tid = %d\n", current->pid);
    break;

    case VCODEC_INITHWLOCK:
    {
    	VAL_VCODEC_OAL_HW_CONTEXT_T *context;
    	VAL_VCODEC_OAL_HW_REGISTER_T  hwoal_reg;
        VAL_VCODEC_OAL_HW_REGISTER_T  *kva_TempReg;
    	VAL_VCODEC_OAL_MEM_STAUTS_T  oal_mem_status[OALMEM_STATUS_NUM];      		
    	unsigned int addr_pa, ret, i, pa_u4HWIsCompleted, pa_u4HWIsTimeout;
    	VAL_UINT8_T *user_data_addr;
        
        MFV_LOGD("[MT6572] VCODEC_INITHWLOCK + - tid = %d\n", current->pid);
    	    	
    	////////////// Start to get content
    	/////////////// take VAL_VCODEC_OAL_HW_REGISTER_T content
    	user_data_addr = (VAL_UINT8_T *)arg;
    	ret = copy_from_user(&hwoal_reg, user_data_addr, sizeof(VAL_VCODEC_OAL_HW_REGISTER_T));

    	addr_pa = pmem_user_v2p_video((unsigned int )user_data_addr); 

        spin_lock_irqsave(&OalHWContextLock, ulFlags);
		context = setCurr_HWLockSlot(addr_pa, current->pid);
		context->Oal_HW_reg = (VAL_VCODEC_OAL_HW_REGISTER_T *)arg;
		context->Oal_HW_mem_reg = (VAL_UINT32_T*)(((VAL_VCODEC_OAL_HW_REGISTER_T *)user_data_addr)->pHWStatus);
        if (hwoal_reg.u4NumOfRegister != 0)
        {
		    context->pa_Oal_HW_mem_reg =  pmem_user_v2p_video( (int)( ((VAL_VCODEC_OAL_HW_REGISTER_T *)user_data_addr)->pHWStatus ) );
        }
		MFV_LOGD("user_data_addr 0x%x , pa =0x%x\n", (unsigned int)user_data_addr, (unsigned int)addr_pa);
		pa_u4HWIsCompleted =  pmem_user_v2p_video( (int)&( ((VAL_VCODEC_OAL_HW_REGISTER_T *)user_data_addr)->u4HWIsCompleted ) );
        pa_u4HWIsTimeout =  pmem_user_v2p_video( (int)&( ((VAL_VCODEC_OAL_HW_REGISTER_T *)user_data_addr)->u4HWIsTimeout ) );
    	MFV_LOGD("user_data_addr->u4HWIsCompleted ua = 0x%x pa= 0x%x\n", (int)&( ((VAL_VCODEC_OAL_HW_REGISTER_T *)user_data_addr)->u4HWIsCompleted ), pa_u4HWIsCompleted );
        MFV_LOGD("user_data_addr->u4HWIsTimeout ua = 0x%x pa= 0x%x\n", (int)&( ((VAL_VCODEC_OAL_HW_REGISTER_T *)user_data_addr)->u4HWIsTimeout ), pa_u4HWIsTimeout );

    	
    	ret = copy_from_user(&oal_mem_status[0], ((VAL_VCODEC_OAL_HW_REGISTER_T *)user_data_addr)->pHWStatus, hwoal_reg.u4NumOfRegister*sizeof(VAL_VCODEC_OAL_MEM_STAUTS_T));
		context->u4NumOfRegister = hwoal_reg.u4NumOfRegister;
    	MFV_LOGW("[VCODEC_INITHWLOCK] ToTal %d u4NumOfRegister\n", hwoal_reg.u4NumOfRegister);
#if 0                        
		context->kva_Oal_HW_mem_reg = (VAL_UINT32_T *)ioremap(context->pa_Oal_HW_mem_reg, context->u4NumOfRegister*sizeof(VAL_VCODEC_OAL_MEM_STAUTS_T)); // need to remap addr + data addr
#else
        if (hwoal_reg.u4NumOfRegister != 0)
        {
            u4TempKPA = context->pa_Oal_HW_mem_reg;
            spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
            mutex_lock(&NonCacheMemoryListLock);
            pu4TempKVA = (VAL_UINT32_T *)Search_NonCacheMemoryList_By_KPA(u4TempKPA);
            mutex_unlock(&NonCacheMemoryListLock);
            spin_lock_irqsave(&OalHWContextLock, ulFlags);
            context->kva_Oal_HW_mem_reg = pu4TempKVA;
            MFV_LOGD("context->ua = 0x%x  pa_Oal_HW_mem_reg = 0x%x \n", (int)( ((VAL_VCODEC_OAL_HW_REGISTER_T *)user_data_addr)->pHWStatus ), context->pa_Oal_HW_mem_reg);
        }
#endif
#if 0
		context->kva_u4HWIsCompleted = (VAL_UINT32_T)ioremap(pa_u4HWIsCompleted, 4); // need to remap addr + data addr
		context->kva_u4HWIsTimeout = (VAL_UINT32_T)ioremap(pa_u4HWIsTimeout, 4); // need to remap addr + data addr						
#else
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        mutex_lock(&NonCacheMemoryListLock);
        kva_TempReg = (VAL_VCODEC_OAL_HW_REGISTER_T *)Search_NonCacheMemoryList_By_KPA(addr_pa);
        mutex_unlock(&NonCacheMemoryListLock);
        spin_lock_irqsave(&OalHWContextLock, ulFlags);
        context->kva_u4HWIsCompleted = (VAL_UINT32_T)(&(kva_TempReg->u4HWIsCompleted));
        context->kva_u4HWIsTimeout = (VAL_UINT32_T)(&(kva_TempReg->u4HWIsTimeout));
        MFV_LOGD("kva_TempReg = 0x%x, kva_u4HWIsCompleted = 0x%x, kva_u4HWIsTimeout = 0x%x\n", 
            (VAL_UINT32_T)kva_TempReg, context->kva_u4HWIsCompleted, context->kva_u4HWIsTimeout);
#endif
    	for(i=0; i < hwoal_reg.u4NumOfRegister ; i++ )
        { 
    		int kva;       			
    		MFV_LOGE("[REG_INFO_1] [%d] 0x%x 0x%x \n", i ,
    		oal_mem_status[i].u4ReadAddr, oal_mem_status[i].u4ReadData);
    		
    		addr_pa = pmem_user_v2p_video((unsigned int )oal_mem_status[i].u4ReadAddr);	        		
    		spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
    		kva = (VAL_UINT32_T)ioremap(addr_pa, 8); // need to remap addr + data addr
    		spin_lock_irqsave(&OalHWContextLock, ulFlags);
    		MFV_LOGE("[REG_INFO_2] [%d] pa = 0x%x  kva = 0x%x \n", i , addr_pa, kva);
    		context->oalmem_status[i].u4ReadAddr = kva; //oal_mem_status[i].u4ReadAddr;
    	}
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);

        MFV_DEBUG(" VCODEC_INITHWLOCK addr1 0x%x addr2 0x%x \n", 
    	(unsigned int )arg, (unsigned int ) ((VAL_VCODEC_OAL_HW_REGISTER_T *)arg)->pHWStatus);
        MFV_LOGD("[MT6572] VCODEC_INITHWLOCK - - tid = %d\n", current->pid);
    }
    break;
    
    case VCODEC_DEINITHWLOCK:    			
    {
        VAL_UINT8_T *user_data_addr;   
        int addr_pa;     

        MFV_LOGD("[MT6572] VCODEC_DEINITHWLOCK + - tid = %d\n", current->pid);
        
    	user_data_addr = (VAL_UINT8_T *)arg;
    	addr_pa = pmem_user_v2p_video((unsigned int )user_data_addr);

        MFV_DEBUG("VCODEC_DEINITHWLOCK ObjId=%d\n", current->pid);
        spin_lock_irqsave(&OalHWContextLock, ulFlags);
        freeCurr_HWLockSlot(addr_pa);
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        MFV_LOGD("[MT6572] VCODEC_DEINITHWLOCK - - tid = %d\n", current->pid);
    }
    break;

    case VCODEC_GET_CPU_LOADING_INFO:
    {
        VAL_UINT8_T *user_data_addr;
        VAL_VCODEC_CPU_LOADING_INFO_T _temp;
        
        MFV_LOGD("+VCODEC_GET_CPU_LOADING_INFO\n");
        user_data_addr = (VAL_UINT8_T *)arg;

#if 1 // Morris Yang 20120112 mark temporarily
        _temp._cpu_idle_time = mt_get_cpu_idle(0);
        _temp._thread_cpu_time = mt_get_thread_cputime(0);
        spin_lock_irqsave(&OalHWContextLock, ulFlags);
        _temp._inst_count = getCurInstanceCount();
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        _temp._sched_clock = mt_sched_clock();
#endif        
        ret = copy_to_user(user_data_addr, &_temp, sizeof(VAL_VCODEC_CPU_LOADING_INFO_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_GET_CPU_LOADING_INFO, copy_to_user failed: %d\n", ret);
        	return -EFAULT;
        }
        
        MFV_LOGD("-VCODEC_GET_CPU_LOADING_INFO\n");        
        break;
    }

    case VCODEC_GET_CORE_LOADING:
    {
        MFV_LOGD("VCODEC_GET_CORE_LOADING + - tid = %d\n", current->pid);

        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_from_user(&rTempCoreLoading, user_data_addr, sizeof(VAL_VCODEC_CORE_LOADING_T));
        if (ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_GET_CORE_LOADING, copy_from_user failed: %d\n", ret);
        	return -EFAULT;
        }
        rTempCoreLoading.Loading = get_cpu_load(rTempCoreLoading.CPUid);

        ret = copy_to_user(user_data_addr, &rTempCoreLoading, sizeof(VAL_VCODEC_CORE_LOADING_T));
        if(ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_GET_CORE_LOADING, copy_to_user failed: %d\n", ret);
        	return -EFAULT;
        }  
        
        MFV_LOGD("VCODEC_GET_CORE_LOADING - - tid = %d\n", current->pid);
        break;
    }

    case VCODEC_GET_CORE_NUMBER:
    {
        MFV_LOGD("VCODEC_GET_CORE_NUMBER + - tid = %d\n", current->pid);

        user_data_addr = (VAL_UINT8_T *)arg;
        temp_nr_cpu_ids = nr_cpu_ids;
        ret = copy_to_user(user_data_addr, &temp_nr_cpu_ids, sizeof(int));
        if(ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_GET_CORE_NUMBER, copy_to_user failed: %d\n", ret);
        	return -EFAULT;
        }

        MFV_LOGD("VCODEC_GET_CORE_NUMBER - - tid = %d\n", current->pid);
        break;
    }
#if defined(CONFIG_SMP)
    case VCODEC_SET_CPU_OPP_LIMIT:
        user_data_addr = (VAL_UINT8_T *)arg;
        ret = copy_from_user(&rCpuOppLimit, user_data_addr, sizeof(VAL_VCODEC_CPU_OPP_LIMIT_T));
        if(ret)
        {
        	MFV_LOGE("[ERROR] VCODEC_SET_CPU_OPP_LIMIT, copy_from_user failed: %d\n", ret);
        	return -EFAULT;
        }
        MFV_LOGD ("+VCODEC_SET_CPU_OPP_LIMIT (%d, %d, %d), tid = %d\n", rCpuOppLimit.limited_freq, rCpuOppLimit.limited_cpu, rCpuOppLimit.enable, current->pid);


        if (rCpuOppLimit.enable) {
            if (gPowerSavingCount == 0) {
                //ret = cpu_opp_limit(EVENT_VIDEO, rCpuOppLimit.limited_freq, rCpuOppLimit.limited_cpu, rCpuOppLimit.enable); // 0: PASS, other: FAIL
                //printk ("@@ cpu_opp_mask video (%d)", rCpuOppLimit.enable);
                // Cheng-Jung 20120717 JB migration mark temporarily [
                #ifndef ONLY_FOR_EARLY_PORTING      
                ret = cpu_opp_mask("video", video_cpu_opp_mask, rCpuOppLimit.enable); // 0: PASS, other: FAIL
                //]
                if(ret) {
        	        MFV_LOGE("[ERROR] cpu_opp_limit failed: %d\n", ret);
        	        return -EFAULT;
                }
                #endif    
                gPowerSavingCount++;
            }
            else {
                MFV_LOGE("[WARNING] ## gPowerSavingCount has already set (%d)\n", gPowerSavingCount);
            }
        }
        else { // disable
            if (gPowerSavingCount > 0) {
                //ret = cpu_opp_limit(EVENT_VIDEO, rCpuOppLimit.limited_freq, rCpuOppLimit.limited_cpu, rCpuOppLimit.enable); // 0: PASS, other: FAIL
                //printk ("@@ cpu_opp_mask video (%d)", rCpuOppLimit.enable);
                // Cheng-Jung 20120717 JB migration mark temporarily [
                #ifndef ONLY_FOR_EARLY_PORTING   
                ret = cpu_opp_mask("video", video_cpu_opp_mask, rCpuOppLimit.enable); // 0: PASS, other: FAIL
                // ]
                if(ret) {
                    MFV_LOGE("[ERROR] cpu_opp_limit failed: %d\n", ret);
        	        return -EFAULT;
                }
                #endif
                gPowerSavingCount--;
            }
            else {
                MFV_LOGE("[WARNING] ## gPowerSavingCount has already clear (%d)\n", gPowerSavingCount);
            }
        }
        MFV_LOGD ("-VCODEC_SET_CPU_OPP_LIMIT tid = %d, ret = %d\n", current->pid, ret);
        break;
#endif
    default:
        MFV_LOGE("========[ERROR] mflexvideo_ioctl default case========\n");
    break;
    }
    return 0xFF;
}

static int mflexvideo_open(struct inode *inode, struct file *file)
{
    int i;
    MFV_LOGD("[MFV_DEBUG] mflexvideo_open\n");   

    mutex_lock(&DriverOpenCountLock);
    MT6572Driver_Open_Count++;

    MFV_LOGE("mflexvideo_open pid = %d, MT6572Driver_Open_Count %d\n", current->pid, MT6572Driver_Open_Count);
    mutex_unlock(&DriverOpenCountLock);

    mutex_lock(&VcodecHWUserLock);
    for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {
        if (grVcodecDriverUser[i] == 0)
        {
            grVcodecDriverUser[i] = current->pid;
            break;
        }
    }
    
    if (i == VCODEC_MULTIPLE_INSTANCE_NUM)
    {
        MFV_LOGE("[ERROR] mflexvideo_open is %d\n", VCODEC_MULTIPLE_INSTANCE_NUM);
    }
    mutex_unlock(&VcodecHWUserLock);
    
    return 0;
}

static int mflexvideo_flush(struct file *file, fl_owner_t id)
{
    int i, j;
    unsigned long ulFlags, ulFlagsLockHW, ulFlagsISR, ulFlagsVcodecHWLock;
    
    //dump_stack();
    MFV_LOGD("[MFV_DEBUG] mflexvideo_flush, curr_tid =%d\n", current->pid);
    mutex_lock(&DriverOpenCountLock);
    MFV_LOGE("mflexvideo_flush pid = %d, MT6572Driver_Open_Count %d\n", current->pid, MT6572Driver_Open_Count);

    mutex_lock(&VcodecHWUserLock);
    for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        for (j = 0; j < grVcodecHWUser[i].u4VCodecThreadNum; j++)
        {
            if (grVcodecHWUser[i].u4VCodecThreadID[j] == current->pid)
            {
                MFV_LOGD("[FLUSH] i = %d, j = %d, tid = %d\n", i, j, current->pid);
                break;
            }
        }
        if (j != grVcodecHWUser[i].u4VCodecThreadNum)
        {
            MFV_LOGD("[FLUSH][FIND]\n");
            break;
        }
#else
        if ((grVcodecHWUser[i].u4tid1 == current->pid) || (grVcodecHWUser[i].u4tid2 == current->pid))
        {
            break;
        }
#endif        
    }

    for (j = 0; j < VCODEC_MULTIPLE_INSTANCE_NUM; j++)
    {
        if (grVcodecDriverUser[j] == current->pid)
        {
            grVcodecDriverUser[j] = 0;
            break;
        }
    }
    mutex_unlock(&VcodecHWUserLock);
    
    if ((i == VCODEC_MULTIPLE_INSTANCE_NUM) && (j == VCODEC_MULTIPLE_INSTANCE_NUM))
    {
        /*
        MFV_LOGE("==== [MAYBE ERROR] BACKTRACE FOR DEGUG ====\n");
        dump_stack();
        MFV_LOGE("==== [MAYBE ERROR] BACKTRACE FOR DEGUG ====\n");
        */
    }
    
    MT6572Driver_Open_Count--;
    
	mutex_lock(&NonCacheMemoryListLock);	
    Force_Free_NonCacheMemoryList(current->pid);
    mutex_unlock(&NonCacheMemoryListLock);

    mutex_lock(&SysramUserLock);
    mutex_lock(&VcodecSysramUserLock);
    Free_SysramUserList(current->pid, VAL_TRUE);
    mutex_unlock(&VcodecSysramUserLock);
    mutex_unlock(&SysramUserLock);        

    // $$$$  note :  it's doesn't check the multiple process close issue

	if(MT6572Driver_Open_Count == 0)
	{
        mutex_lock(&VcodecHWUserLock);
        for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++)
        {
            grVcodecDriverUser[i] = 0;
        }
        mutex_unlock(&VcodecHWUserLock);
        
        //mutex_lock(&VcodecHWLock);
        spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        grVcodecHWLock.u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
        for (i = 0; i < VCODEC_THREAD_MAX_NUM; i++)
        {
            grVcodecHWLock.u4VCodecThreadID[i] = -1;
        }
#else
        grVcodecHWLock.u4LockHWTID1 = -1;
        grVcodecHWLock.u4LockHWTID2 = -1;
#endif
        grVcodecHWLock.rLockedTime.u4Sec = 0;        
        grVcodecHWLock.rLockedTime.u4uSec = 0;
        //mutex_unlock(&VcodecHWLock);
        spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);

        mutex_lock(&SysramUserLock);
        gu4INTMEMCounter = 0;
        mutex_unlock(&SysramUserLock);

        mutex_lock(&EncEMILock);
        gu4EncEMICounter = 0;
        mutex_unlock(&EncEMILock);

        mutex_lock(&DecEMILock);
        gu4DecEMICounter = 0;
        mutex_unlock(&DecEMILock);

        mutex_lock(&PWRLock);
        gu4PWRCounter = 0; 
        mutex_unlock(&PWRLock);

        mutex_lock(&VcodecSysramUserLock);
        for(i =0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++)
        {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            grVcodecSysramUser[i].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
            for(j = 0; j < VCODEC_THREAD_MAX_NUM; j++)
            {
                grVcodecSysramUser[i].u4VCodecThreadID[j] = 0xffffffff;
            }
#else
            grVcodecSysramUser[i].u4TID1 = 0xffffffff;
            grVcodecSysramUser[i].u4TID2 = 0xffffffff;
#endif
        }
        mutex_unlock(&VcodecSysramUserLock);

        mutex_lock(&NonCacheMemoryListLock);
        for(i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; i++)
        {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            grNonCacheMemoryList[i].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
            for(j = 0; j < VCODEC_THREAD_MAX_NUM; j++)
            {
                grNonCacheMemoryList[i].u4VCodecThreadID[j] = 0xffffffff;
            }
#else            
            grNonCacheMemoryList[i].u4TID1 = 0xffffffff;
            grNonCacheMemoryList[i].u4TID2 = 0xffffffff;
#endif
            grNonCacheMemoryList[i].u4KVA = 0xffffffff;
            grNonCacheMemoryList[i].u4KPA = 0xffffffff;
        }
        mutex_unlock(&NonCacheMemoryListLock);

        spin_lock_irqsave(&OalHWContextLock, ulFlags);
		for(i =0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++)
        {
		    oal_hw_context[i].Oal_HW_reg = (VAL_VCODEC_OAL_HW_REGISTER_T  *)0;
			oal_hw_context[i].ObjId = -1;
			oal_hw_context[i].slotindex = i;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
            oal_hw_context[i].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
            for(j = 0; j < VCODEC_THREAD_MAX_NUM; j++)
            {
                oal_hw_context[i].u4VCodecThreadID[j] = -1;
            }
#else
			oal_hw_context[i].tid1 = -1;
            oal_hw_context[i].tid2 = -1;
#endif
			oal_hw_context[i].u4NumOfRegister = 0;
			
			for(j=0; j< OALMEM_STATUS_NUM ; j ++)
            {
				oal_hw_context[i].oalmem_status[j].u4ReadAddr = 0;
				oal_hw_context[i].oalmem_status[j].u4ReadData = 0;
			}
			
			// event part
			oal_hw_context[i].IsrEvent.pvHandle = "ISR_EVENT";
			oal_hw_context[i].IsrEvent.u4HandleSize = sizeof("ISR_EVENT")+1;
			oal_hw_context[i].IsrEvent.u4TimeoutMs = 0xFFFFFFFF;  //1000;			
		}
        spin_unlock_irqrestore(&OalHWContextLock, ulFlags);

        spin_lock_irqsave(&LockHWCountLock, ulFlagsLockHW);
        gu4LockHWCount = 0;
        spin_unlock_irqrestore(&LockHWCountLock, ulFlagsLockHW);
        
        spin_lock_irqsave(&ISRCountLock, ulFlagsISR);
        gu4ISRCount = 0;
        spin_unlock_irqrestore(&ISRCountLock, ulFlagsISR);
#if defined(CONFIG_SMP)     
        if (gPowerSavingCount > 0) {
            //cpu_opp_limit(EVENT_VIDEO, 1001000, 1, 0); // 0: PASS, other: FAIL
            //printk ("@@ cpu_opp_mask video (%d)", 0);
            // Cheng-Jung 20120717 JB migration mark temporarily [
            #ifndef ONLY_FOR_EARLY_PORTING
            cpu_opp_mask("video", video_cpu_opp_mask, 0);
            #endif
            // ]
            gPowerSavingCount--;
        }
#endif        
	}
    mutex_unlock(&DriverOpenCountLock);

    return 0;
}

static int mflexvideo_release(struct inode *inode, struct file *file)
{		
    MFV_LOGD("[MFV_DEBUG] mflexvideo_release, curr_tid =%d\n", current->pid); 
    
    return 0;
}

#include "mt_reg_base.h"


static int MT6572_Vcodec_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
    return 0;
}

static int MT6572_Vcodec_mmap(struct file *file, struct vm_area_struct *vma)
{
    VAL_UINT32_T u4I = 0;
	long length;
	unsigned int pfn;
	
    MFV_LOGD("[MT6572_vcodec_mmap] \n");

	MFV_LOGD("start = 0x%x, pgoff= 0x%x vm_end= 0x%x, vm_page_prot= 0x%x\n", 
	    (unsigned int)vma->vm_start, (unsigned int)vma->vm_pgoff, (unsigned int)vma->vm_end, (unsigned int)vma->vm_page_prot );
		
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);            

	
	length = vma->vm_end - vma->vm_start;
	pfn = vma->vm_pgoff<<PAGE_SHIFT;
	
	if(((length > VENC_REGION) || (pfn < VENC_BASE) || (pfn > VENC_BASE+VENC_REGION)) &&
	   ((length > VDEC_REGION) || (pfn < VDEC_BASE) || (pfn > VDEC_BASE+VDEC_REGION)) &&
	   ((length > HW_REGION) || (pfn < HW_BASE) || (pfn > HW_BASE+HW_REGION)) &&
           ((length > HW_TABLET_REGION) || (pfn < HW_TABLET_BASE) || (pfn > HW_TABLET_BASE+HW_TABLET_REGION)) &&
           ((length > HW_TABLET_REGION2) || (pfn < HW_TABLET_BASE2) || (pfn > HW_TABLET_BASE2+HW_TABLET_REGION2))
      )
	{
	    VAL_UINT32_T u4Addr, u4Size;
	    for(u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++)
        {
            if ((grNonCacheMemoryList[u4I].u4KVA != 0xffffffff) && (grNonCacheMemoryList[u4I].u4KPA != 0xffffffff))
            {
                u4Addr = grNonCacheMemoryList[u4I].u4KPA;
				u4Size = (grNonCacheMemoryList[u4I].u4Size + 0x1000 -1) & ~(0x1000-1);
                if((length == u4Size) && (pfn == u4Addr))
                {
                    MFV_LOGD(" cache idx %d \n", u4I);
                    break;
                }
            }
	    }

		if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM_x_10)
		{
		    MFV_LOGE("[ERROR] mmap region error: Length(0x%x), pfn(0x%x)\n", length, pfn);
		    return -EAGAIN;
		}
	}	 

    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, vma->vm_end - vma->vm_start, vma->vm_page_prot)) 
    {
        return -EAGAIN;
    }

    return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void MT6572_VCodec_early_suspend(struct early_suspend *h)
{
    mutex_lock(&PWRLock);
    MFV_LOGE("MT6572_VCodec_early_suspend, tid = %d, PWR_USER = %d\n", current->pid, gu4PWRCounter);
    mutex_unlock(&PWRLock);
/*
    if (gu4PWRCounter != 0)
    {
        MFV_LOGE("[MT6572_VCodec_early_suspend] Someone Use HW, Disable Power!\n");
    	disable_clock(MT65XX_PDN_MM_VBUF, "Video_VBUF");
    	disable_clock(MT65XX_PDN_MM_VDEC, "VideoDec");
    	disable_clock(MT65XX_PDN_MM_VENC, "VideoEnc");    			
    	disable_clock(MT65XX_PDN_MM_GDC_SHARE_MACRO, "VideoEnc");
    }
*/
    MFV_LOGD("MT6572_VCodec_early_suspend - tid = %d\n", current->pid);
}

static void MT6572_VCodec_late_resume(struct early_suspend *h)
{
    mutex_lock(&PWRLock);
    MFV_LOGE("MT6572_VCodec_late_resume, tid = %d, PWR_USER = %d\n", current->pid, gu4PWRCounter);
    mutex_unlock(&PWRLock);
/*
    if (gu4PWRCounter != 0)
    {
        MFV_LOGE("[MT6572_VCodec_late_resume] Someone Use HW, Enable Power!\n");
    	enable_clock(MT65XX_PDN_MM_VBUF, "Video_VBUF");
    	enable_clock(MT65XX_PDN_MM_VDEC, "VideoDec");
    	enable_clock(MT65XX_PDN_MM_VENC, "VideoEnc");    			
    	enable_clock(MT65XX_PDN_MM_GDC_SHARE_MACRO, "VideoEnc");
    }
*/
    MFV_LOGD("MT6572_VCodec_late_resume - tid = %d\n", current->pid);
}

static struct early_suspend vcodec_early_suspend_handler = 
{
	.level = (EARLY_SUSPEND_LEVEL_DISABLE_FB - 1),
	.suspend = MT6572_VCodec_early_suspend,
	.resume = MT6572_VCodec_late_resume,
};
#endif

static struct file_operations mflexvideo_fops = {
    .owner      = THIS_MODULE,
    .unlocked_ioctl = mflexvideo_unlocked_ioctl,
    .open       = mflexvideo_open,
    .flush      = mflexvideo_flush,
    .release    = mflexvideo_release,
    .mmap       = MT6572_Vcodec_mmap,
    .write      = MT6572_Vcodec_write,
};

static int mflexvideo_probe(struct platform_device *dev) 
{
    int ret;
    MFV_LOGD("+mflexvideo_probe\n");
    MFV_LOGD("[MFV_DEBUG] mflexvideo_probe\n");

    mutex_lock(&SysramUserLock);
    gu4INTMEMCounter = 0;
    mutex_unlock(&SysramUserLock);

    mutex_lock(&EncEMILock);
    gu4EncEMICounter = 0;
    mutex_unlock(&EncEMILock);

    mutex_lock(&DecEMILock);
    gu4DecEMICounter = 0;
    mutex_unlock(&DecEMILock);

    mutex_lock(&PWRLock);
    gu4PWRCounter = 0;
    mutex_unlock(&PWRLock);

    ret = register_chrdev_region(mflexvideo_devno, 1, MFLEXVIDEO_DEVNAME);	
    if(ret)
    {
        MFV_LOGD("[MFV_DEBUG][ERROR] Can't Get Major number for MFleXVideo Device\n");
    }

    mflexvideo_cdev = cdev_alloc();
    mflexvideo_cdev->owner = THIS_MODULE;
    mflexvideo_cdev->ops = &mflexvideo_fops;

    ret = cdev_add(mflexvideo_cdev, mflexvideo_devno, 1);

	mt_irq_set_polarity(MT_VDEC_IRQ_ID, MT65xx_POLARITY_LOW);
	mt_irq_set_polarity(MT_VENC_IRQ_ID, MT65xx_POLARITY_LOW);
		
    mt_irq_set_sens(MT_VDEC_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    mt_irq_set_sens(MT_VENC_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    
    //Register Interrupt 
    if (request_irq(MT_VDEC_IRQ_ID , (irq_handler_t)mt6572_video_intr_dlr, 0, MFLEXVIDEO_DEVNAME, NULL) < 0)
    {
       MFV_LOGD("[MFV_DEBUG][ERROR] error to request MFlexVideo irq\n"); 
    }
    else
    {
       MFV_LOGD("[MFV_DEBUG] success to request MFlexVideo irq\n");
    }

    if (request_irq(MT_VENC_IRQ_ID , (irq_handler_t)mt6572_video_intr_dlr2, 0, MFLEXVIDEO_DEVNAME, NULL) < 0)
    {
       MFV_LOGD("[MFV_DEBUG][ERROR] error to request MFlexVideo irq\n"); 
    }
    else
    {
       MFV_LOGD("[MFV_DEBUG] success to request MFlexVideo irq\n");
    }
    disable_irq(MT_VDEC_IRQ_ID);
    disable_irq(MT_VENC_IRQ_ID);

    MFV_LOGD("[MFV_DEBUG] mflexvideo_probe Done\n");
    
    return 0;
}

#ifdef CONFIG_MTK_HIBERNATION
extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq, unsigned int polarity);
static int vcodec_pm_restore_noirq(struct device *device)
{
    // vdec : IRQF_TRIGGER_LOW
    mt_irq_set_sens(MT_VDEC_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    mt_irq_set_polarity(MT_VDEC_IRQ_ID, MT65xx_POLARITY_LOW);
    // venc: IRQF_TRIGGER_LOW
    mt_irq_set_sens(MT_VENC_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    mt_irq_set_polarity(MT_VENC_IRQ_ID, MT65xx_POLARITY_LOW);

    return 0;
}
#endif

static int __init mflexvideo_driver_init(void)
{
    int i, j;
    VAL_RESULT_T  eValRet;
    VAL_RESULT_T  eValHWLockRet;
    unsigned long ulFlags, ulFlagsISR, ulFlagsLockHW, ulFlagsVcodecHWLock;
    
    MFV_LOGD("+mflexvideo_driver_init\n");

    MFV_LOGD("[MFV_DEBUG] mflexvideo_driver_init\n");
    
    MT6572Driver_Open_Count = 0;

    KVA_ENC_INT_ACK_ADDR  = (int)ioremap(VENC_IRQ_ACK_addr, 4);
    KVA_VDEC_INT_STA_ADDR = (int)ioremap(DEC_VDEC_INT_STA_ADDR,4);
    KVA_VDEC_INT_ACK_ADDR = (int)ioremap(DEC_VDEC_INT_ACK_ADDR,4);

    KVA_VENC_IRQ_STATUS_ADDR =  (int)ioremap(VENC_IRQ_STATUS_addr, 4);
    KVA_VENC_ZERO_COEF_COUNT_ADDR = (int)ioremap(VENC_ZERO_COEF_COUNT_addr, 4);
    KVA_VENC_BYTE_COUNT_ADDR = (int)ioremap(VENC_BYTE_COUNT_addr, 4);

    spin_lock_irqsave(&LockHWCountLock, ulFlagsLockHW);
    gu4LockHWCount = 0;
    spin_unlock_irqrestore(&LockHWCountLock, ulFlagsLockHW);
    
    spin_lock_irqsave(&ISRCountLock, ulFlagsISR);
    gu4ISRCount = 0;
    spin_unlock_irqrestore(&ISRCountLock, ulFlagsISR);

    mutex_lock(&IsOpenedLock);
    if (VAL_FALSE == bIsOpened) 
    {
		bIsOpened = VAL_TRUE;
		mflexvideo_probe(NULL);
    }
    mutex_unlock(&IsOpenedLock);

    //mutex_lock(&VcodecHWLock);
    spin_lock_irqsave(&VcodecHWLock, ulFlagsVcodecHWLock);
#if 1   //VCODEC_MULTI_THREAD
        grVcodecHWLock.u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
        for (i = 0; i < VCODEC_THREAD_MAX_NUM; i++)
        {
            grVcodecHWLock.u4VCodecThreadID[i] = -1;
        }
#else    
    grVcodecHWLock.u4LockHWTID1 = -1;
    grVcodecHWLock.u4LockHWTID2 = -1;
#endif
    grVcodecHWLock.rLockedTime.u4Sec = 0;
    grVcodecHWLock.rLockedTime.u4uSec = 0;
    //mutex_unlock(&VcodecHWLock);
    spin_unlock_irqrestore(&VcodecHWLock, ulFlagsVcodecHWLock);

    mutex_lock(&VcodecHWUserLock);
    for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM;i++)
    {
        grVcodecDriverUser[i] = 0;
    }
    mutex_unlock(&VcodecHWUserLock);

    mutex_lock(&VcodecSysramUserLock);
    for(i =0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        grVcodecSysramUser[i].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
        for(j = 0; j < VCODEC_THREAD_MAX_NUM; j++)
        {
            grVcodecSysramUser[i].u4VCodecThreadID[j] = 0xffffffff;
        }
#else
        grVcodecSysramUser[i].u4TID1 = 0xffffffff;
        grVcodecSysramUser[i].u4TID2 = 0xffffffff;
#endif
    }
    mutex_unlock(&VcodecSysramUserLock);

    mutex_lock(&NonCacheMemoryListLock);
    for(i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; i++)
    {
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        grNonCacheMemoryList[i].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
        for(j = 0; j < VCODEC_THREAD_MAX_NUM; j++)
        {
            grNonCacheMemoryList[i].u4VCodecThreadID[j] = 0xffffffff;
        }
#else 
        grNonCacheMemoryList[i].u4TID1 = 0xffffffff;
        grNonCacheMemoryList[i].u4TID2 = 0xffffffff;
#endif
        grNonCacheMemoryList[i].u4KVA = 0xffffffff;
        grNonCacheMemoryList[i].u4KPA = 0xffffffff;
    }
    mutex_unlock(&NonCacheMemoryListLock);

	for(i =0; i<VCODEC_MULTIPLE_INSTANCE_NUM; i++)
    {
        spin_lock_irqsave(&OalHWContextLock, ulFlags);
	    oal_hw_context[i].Oal_HW_reg = (VAL_VCODEC_OAL_HW_REGISTER_T  *)0;
		oal_hw_context[i].ObjId = -1;
		oal_hw_context[i].slotindex = i;
#if defined(CONFIG_SMP)   //VCODEC_MULTI_THREAD
        oal_hw_context[i].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
        for(j = 0; j < VCODEC_THREAD_MAX_NUM; j++)
        {
            oal_hw_context[i].u4VCodecThreadID[j] = -1;
        }
#else
		oal_hw_context[i].tid1 = -1;
        oal_hw_context[i].tid2 = -1;
#endif
		oal_hw_context[i].u4NumOfRegister = 0;
		
		for(j=0; j< OALMEM_STATUS_NUM ; j ++)
        {
			oal_hw_context[i].oalmem_status[j].u4ReadAddr = 0;
			oal_hw_context[i].oalmem_status[j].u4ReadData = 0;
		}
		
		// event part
		oal_hw_context[i].IsrEvent.pvHandle = "ISR_EVENT";
		oal_hw_context[i].IsrEvent.u4HandleSize = sizeof("ISR_EVENT")+1;
		oal_hw_context[i].IsrEvent.u4TimeoutMs = 0xFFFFFFFF;  //1000;	
		spin_unlock_irqrestore(&OalHWContextLock, ulFlags);
        eValRet = eVideoCreateEvent(&(oal_hw_context[i].IsrEvent), sizeof(VAL_EVENT_T));
        if(VAL_RESULT_NO_ERROR != eValRet)
        {
            MFV_LOGE("[MFV][ERROR] create isr event error\n"); 
        }		
	}

    //MT6572_HWLockEvent part
    mutex_lock(&HWLockEventTimeoutLock);
    MT6572_HWLockEvent.pvHandle = "HWLOCK_EVENT";
    MT6572_HWLockEvent.u4HandleSize = sizeof("HWLOCK_EVENT")+1;
    MT6572_HWLockEvent.u4TimeoutMs = 1;
    mutex_unlock(&HWLockEventTimeoutLock);
    eValHWLockRet = eVideoCreateEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
    if(VAL_RESULT_NO_ERROR != eValHWLockRet)
    {
        MFV_LOGE("[MFV][ERROR] create hwlock event error\n");
    }
    
    MFV_LOGD("[MFV_DEBUG] mflexvideo_driver_init Done\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
    register_early_suspend(&vcodec_early_suspend_handler);
#endif

#ifdef CONFIG_MTK_HIBERNATION
    register_swsusp_restore_noirq_func(ID_M_VCODEC, vcodec_pm_restore_noirq, NULL);
#endif
    
    return 0;
}

static void __exit mflexvideo_driver_exit(void)
{
    int  i;
    VAL_RESULT_T  eValRet;
    VAL_RESULT_T  eValHWLockRet;
	  
    MFV_LOGD("[MFV_DEBUG] mflexvideo_driver_exit\n");

    mutex_lock(&IsOpenedLock);
	if (VAL_TRUE == bIsOpened) 
	{
		bIsOpened = VAL_FALSE;
	}
    mutex_unlock(&IsOpenedLock);
   
    cdev_del(mflexvideo_cdev);
    unregister_chrdev_region(mflexvideo_devno, 1);
    
	free_irq(MT_VENC_IRQ_ID, NULL); 
	free_irq(MT_VDEC_IRQ_ID, NULL);    
		
    for(i=0; i< OALMEM_STATUS_NUM ; i++)
    {
    	eValRet = eVideoCloseEvent(&(oal_hw_context[i].IsrEvent), sizeof(VAL_EVENT_T) );
        if(VAL_RESULT_NO_ERROR != eValRet)
        {
            MFV_LOGE("[MFV][ERROR] close isr event error\n"); 
        }
    }			

    //MT6572_HWLockEvent part
    eValHWLockRet = eVideoCloseEvent(&MT6572_HWLockEvent, sizeof(VAL_EVENT_T));
    if(VAL_RESULT_NO_ERROR != eValHWLockRet)
    {
        MFV_LOGE("[MFV][ERROR] close hwlock event error\n");
    }

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&vcodec_early_suspend_handler);
#endif    

#ifdef CONFIG_MTK_HIBERNATION
    unregister_swsusp_restore_noirq_func(ID_M_VCODEC);
#endif
}

module_init(mflexvideo_driver_init);
module_exit(mflexvideo_driver_exit);
MODULE_AUTHOR("Charlie, Lu <charlie.lu@mediatek.com>");
MODULE_DESCRIPTION("MT6573 MFleVideo Driver");
MODULE_LICENSE("GPL");
  
