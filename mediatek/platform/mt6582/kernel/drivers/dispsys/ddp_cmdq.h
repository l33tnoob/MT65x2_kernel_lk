#ifndef __DDP_CMDQ_H__
#define __DDP_CMDQ_H__

#ifdef __KERNEL__

#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/xlog.h>
#include <linux/aee.h>

#include <mach/mt_clkmgr.h>

#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT) && defined(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)
#define __CMDQ_IWC_IMPL__
#endif

static atomic_t gCmdqDebugLevel = ATOMIC_INIT(0);

#define CMDQ_MAX_TASK_COUNT           (512)
#define CMDQ_MAX_ENGINE_COUNT          (12)
#define CMDQ_MAX_THREAD_COUNT          (14)
#define CMDQ_MAX_SECURE_THREAD_COUNT    (1) // support executing secure task 
#define CMDQ_MAX_RECORD_COUNT        (1024)
#define CMDQ_MAX_ERROR_COUNT           (1)
#define CMDQ_MAX_WARNING_COUNT         (20)
#define CMDQ_MAX_RETRY_COUNT           (1)

#define CMDQ_MAX_FIXED_TASK            (70)
#define CMDQ_MAX_BLOCK_SIZE     (32 * 1024)
#define CMDQ_MAX_DMA_BUF_SIZE   (CMDQ_MAX_FIXED_TASK * CMDQ_MAX_BLOCK_SIZE)
#define CMDQ_EXTRA_MARGIN       (512)

#define CMDQ_MAX_LOOP_COUNT     (0x100000)
#define CMDQ_MAX_INST_CYCLE     (0) // no HW timeout
#define CMDQ_MIN_AGE_VALUE      (5)

#define CMDQ_DEFAULT_TIMEOUT_MS            (1200)
#define CMDQ_DEFAULT_PREDUMP_START_TIME_MS  (400)
#define CMDQ_DEFAULT_PREDUMP_TIMEOUT_MS     (100)
#define CMDQ_DEFAULT_PREDUMP_RETRY_COUNT      (8)


#define CMDQ_INVALID_THREAD     (-1)

#define CMDQ_MSG(string, args...)                                                                                   \
    if(1 == atomic_read(&gCmdqDebugLevel))                                                                          \
    {                                                                                                               \
        printk(KERN_DEBUG "[CMDQ] "string, ##args);                                                                 \
    }

#define CMDQ_ERR(string, args...)                                                                                   \
    if(1)                                                                                                           \
    {                                                                                                               \
        printk(KERN_DEBUG "[CMDQ][ERR] "string, ##args);                                                            \
    }

#define CMDQ_AEE(tag, string, args...)                                                                              \
    do {                                                                                                            \
        char output[255];                                                                                           \
        sprintf(output, "[CMDQ][AEE] "string, ##args);                                                                   \
	    aee_kernel_warning_api(__FILE__, __LINE__, DB_OPT_DEFAULT | DB_OPT_PROC_CMDQ_INFO, output, string, ##args); \
	    xlog_printk(ANDROID_LOG_ERROR, tag, string, ##args);                                                        \
    } while(0)

#define CMGQ_GET_CURRENT_TIME(value)                                                                                \
{                                                                                                                   \
    do_gettimeofday(&(value));                                                                                      \
}

#define CMDQ_GET_TIME_DURATION(start,                                                                               \
                               end,                                                                                 \
                               diff)                                                                                \
{                                                                                                                   \
    int32_t time1;                                                                                                  \
    int32_t time2;                                                                                                  \
                                                                                                                    \
    time1 = start.tv_sec * 1000000 + start.tv_nsec / 1000;                                                          \
    time2 = end.tv_sec   * 1000000 + end.tv_nsec   / 1000;                                                          \
                                                                                                                    \
    diff = ((time2 - time1) / 1000); /* in ms */                                                                    \
}

#endif // __KERNEL__

typedef enum CMDQ_SCENARIO_ENUM
{
    // 82/92 scenario is sync with MDP
    // from user space, i.e, MDP
    CMDQ_SCENARIO_MDP_STREAM_UNKNOWN = 0,
    CMDQ_SCENARIO_MDP_STREAM_BITBLT,
    CMDQ_SCENARIO_MDP_STREAM_FRAG,
    CMDQ_SCENARIO_MDP_STREAM_MULTI,
    CMDQ_SCENARIO_MDP_STREAM_ISP_IC,
    CMDQ_SCENARIO_MDP_STREAM_ISP_VR,
    CMDQ_SCENARIO_MDP_STREAM_ISP_ZSD,
    CMDQ_SCENARIO_MDP_STREAM_ISP_IP,
    CMDQ_SCENARIO_MDP_STREAM_ISP_VSS,

    // CMDQ_SCENARIO_DEBUG is from kernel
    CMDQ_SCENARIO_DEBUG,
    CMDQ_MAX_SCENARIO_COUNT              // ALWAYS keep at the end
} CMDQ_SCENARIO_ENUM;


typedef enum CMDQ_CODE_ENUM
{
    CMDQ_CODE_READ  = 0x01,
    CMDQ_CODE_MOVE  = 0x02,
    CMDQ_CODE_WRITE = 0x04,
    CMDQ_CODE_POLL  = 0x08,
    CMDQ_CODE_JUMP  = 0x10,
    CMDQ_CODE_WFE   = 0x20,  // wait for event
    CMDQ_CODE_EOC   = 0x40,  // end of command

    // these are pseudo op code defined by SW
    // for instruction generation
    CMDQ_CODE_SET_TOKEN = 0x21,      // set event
    CMDQ_CODE_WAIT_NO_CLEAR = 0x22,  // wait event, but don't clear it
} CMDQ_CODE_ENUM;


typedef enum CMDQ_ENG_ENUM
{
    // CAM
    tIMGI               = 0,
    tIMGO               = 1,
    tIMG2O              = 2,

    // MDP
    tRDMA0              = 3,
    tCAMIN              = 4,
    tSCL0               = 5,
    tSCL1               = 6,
    tTDSHP              = 7,
    tWROT               = 8,
    tWDMA1              = 9,

    tLSCI               = 10,
    tCMDQ               = 11,

    tTotal              = 12,
}CMDQ_ENG_ENUM;

#ifdef __KERNEL__

enum
{
    cbMDP               = 0,
    cbISP               = 1,
    cbMAX               = 2, 
};


/**callback to notify engine timeout/reset
 * params
 *     init32_t engineFlag
 */ 
typedef int (*CMDQ_TIMEOUT_PTR)(int32_t engineFlag, int32_t level);
typedef int (*CMDQ_RESET_PTR)(int32_t engineFlag);

typedef enum TASK_TYPE_ENUM
{
    TASK_TYPE_FIXED     = 0,
    TASK_TYPE_DYNAMIC   = 1
} TASK_TYPE_ENUM;

typedef enum TASK_STATE_ENUM
{
    TASK_STATE_IDLE,        // free task
    TASK_STATE_BUSY,        // task running on a thread
    TASK_STATE_KILLED,      // task process being killed
    TASK_STATE_ERROR,       // task execution error
    TASK_STATE_DONE,        // task finished
    TASK_STATE_WAITING,     // allocated but waiting for available thread
} TASK_STATE_ENUM;

typedef struct TaskStruct
{
    struct list_head    listEntry;

    TASK_TYPE_ENUM      taskType;

    // For buffer state
    TASK_STATE_ENUM     taskState;
    uint32_t            *pVABase;
    uint32_t            MVABase;
    uint32_t            firstEOC;
    uint32_t            bufSize;
    uint32_t            thread; 

    // For execution
    int32_t             scenario;
    int32_t             priority;
    uint32_t            engineFlag;
    int32_t             blockSize;
    uint32_t            *pCMDEnd;
    int32_t             reorder;

    // For seucre execution
    bool                isSecure;         // 1: secure task
    uint32_t            *pSecureFdIndex;  // secure: secure fd index in cmd block 
    uint32_t            *pSecurePortList; 
    uint32_t            *pSecureSizeList;     
    uint32_t            totalSecureFd;    // secure: total count

    // For statistics
    struct timespec     trigger;
    struct timespec     gotIRQ;
    struct timespec     wakedUp;
} TaskStruct;


typedef struct EngineStruct
{
    int32_t             userCount;
    int32_t             currOwner;
    int32_t             resetCount;
    int32_t             failCount;
} EngineStruct;


typedef struct ThreadStruct
{
    uint32_t            taskCount;
    uint32_t            waitCookie;
    uint32_t            nextCookie;
    TaskStruct          *pCurTask[CMDQ_MAX_TASK_COUNT];
} ThreadStruct;


typedef struct RecordStruct
{
    int32_t             scenario;
    int32_t             priority;
    int32_t             reorder;
    bool                isSecure; 
    int32_t             thread; 

    struct timespec     start;
    struct timespec     trigger;
    struct timespec     gotIRQ;
    struct timespec     wakedUp;
    struct timespec     done;
} RecordStruct;


typedef struct ErrorStruct
{
    TaskStruct          *pTask;
} ErrorStruct;


typedef struct WarngingStrcut
{
    void* taskAddr;                        // warning task address. just for warning memo.

    // thread information
    uint32_t            thread;             // the cmdq HW thread which is triggerd IRQ
    uint32_t            taskCount;          // total task count in thread when IRQ received
    uint32_t            cookie;             // hw thread executed counter
    uint32_t            irqFlag;            // IRQ value
    struct timespec     gotIRQ;             // time stamp get IRQ
}WarngingStrcut; 


typedef struct ContextStruct
{
    // Basic information
    TaskStruct          taskInfo[CMDQ_MAX_FIXED_TASK];
    EngineStruct        engine[CMDQ_MAX_ENGINE_COUNT];
    ThreadStruct        thread[CMDQ_MAX_THREAD_COUNT];

    // Profile information
    int32_t             lastID;
    int32_t             recNum;
    RecordStruct        record[CMDQ_MAX_RECORD_COUNT];

    // Error information
    int32_t             errNum;
    ErrorStruct         error[CMDQ_MAX_ERROR_COUNT];
    int32_t             logLevel;

    // Warning information
    int32_t             warningNum; 
    WarngingStrcut      warning[CMDQ_MAX_WARNING_COUNT];

    // SW timeout setting
    uint32_t swTimeoutDurationMS;
    uint32_t predumpStartTimeMS;
    uint32_t predumpDurationMS; 
    uint32_t predumpMaxRetryCount;   
} ContextStruct;


#ifdef __cplusplus
extern "C" {
#endif

int32_t cmdqRegisterCallback(int32_t          index,
                             CMDQ_TIMEOUT_PTR pTimeout,
                             CMDQ_RESET_PTR   pReset);

void cmdqInitialize(void);

int32_t cmdqSuspendTask(void);

int32_t cmdqResumeTask(void);

void cmdqHandleError(int32_t thread, uint32_t value);

void cmdqHandleDone(int32_t thread, uint32_t value);

int32_t cmdqSubmitTask(int32_t  scenario,
                       int32_t  priority,
                       uint32_t engineFlag,
                       void     *pCMDBlock,
                       int32_t  blockSize);
#if 1
int32_t cmdqSubmitTaskSecure(int32_t  scenario,
                       int32_t  priority,
                       uint32_t engineFlag,
                       void     *pCMDBlock,
                       int32_t  blockSize, 
                       uint32_t  *pSecureFdIndex,
                       uint32_t  *pSecurePortList,
                       uint32_t *pSecureSizeList,
                       uint32_t  totalSecurFd);
#endif
void cmdqDeInitialize(void);

#ifdef __cplusplus
}
#endif

#endif // __KERNEL__

#endif  // __DDP_CMDQ_H__
