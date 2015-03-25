#include "ddp_cmdq_sec.h"

static int32_t gDebugSecSwCopy = 0; //1; 
static DEFINE_MUTEX(gDebugSecSwCopyLock);


#if defined(__CMDQ_IWC_IMPL__)

#include "mobicore_driver_api.h"
#include "cmdq_sec_dciapi.h"
#include "cmdq_sec_iwc_common.h"
#include "cmdqSecTl_Api.h"
#include "dci.h"

#define CMDQ_DR_UUID { { 2, 0xb, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
#define CMDQ_TL_UUID { { 9, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

static DEFINE_MUTEX(gCmdqIwcLock); 
static iwcCmdqMessage_t* gIwc      = NULL;
static struct mc_session_handle gSessionHandle;


static DEFINE_MUTEX(gCmdqMobicoreLock); 
static bool alreadlyOpenMobicoreByOther = false; 


typedef enum
{
    IWC_INIT              = 0,   
    IWC_MOBICORE_OPENED   = 1,
    IWC_WSM_ALLOCATED     = 2,
    IWC_SES_OPENED        = 3,
    IWC_SES_MSG_PACKAGED  = 4,
    IWC_SES_TRANSACTED    = 5,
    IWC_SES_ON_TRANSACTED = 6,
    IWC_END_OF_ENUM       = 7,
}IWC_STATE_ENUM;


static cmdq_sec_set_mobicore_open_state(bool alreadyOpen)
{
    mutex_lock(&gCmdqMobicoreLock);
    smp_mb();

    alreadlyOpenMobicoreByOther = alreadyOpen;
    mutex_unlock(&gCmdqMobicoreLock); 
}

int32_t cmdq_sec_open_mobicore_impl(uint32_t deviceId)
{
    int32_t status = 0; 
    enum mc_result mcRet = mc_open_device(deviceId);
    printk("[CMDQ][SEC] mc_open_device returned: %d\n", mcRet);

#if 1
    // Currently, a process context limits to open movicore device once,
    // and mc_open_device dose not support reference cout 
    // so skip the false alarm error....
    if(MC_DRV_ERR_INVALID_OPERATION== mcRet)
    {
        printk("[CMDQ][SEC] mc_open_device already opened, continue to execution\n");
        cmdq_sec_set_mobicore_open_state(true); 
    }
    else if(MC_DRV_OK != mcRet) 
    {
        status = -1; 
    }
    return status;
#else
    return ((MC_DRV_OK != mcRet) ? (-1) : (0));
#endif
}


int32_t cmdq_sec_close_mobicore_impl(uint32_t deviceId)
{
    int32_t status = 0; 
    enum mc_result mcRet = 0; 

#if 1
    if(alreadlyOpenMobicoreByOther)
    {
        // do nothing
        // let last user to close mobicore....
        printk("[CMDQ][SEC] mobicore opened by other, skip the close device operation\n");
    }
    else
    {
        mcRet = mc_close_device(deviceId);
        printk("[CMDQ][SEC] mc_close_device returned: %d\n", mcRet);

        status = ((MC_DRV_OK != mcRet) ? (-1) : (0)); 
    }
    return status; 
#else    
    mcRet = mc_close_device(deviceId);
    printk("[CMDQ][sessionTest] mc_close_device returned: %d\n", mcRet);
    return ((MC_DRV_OK != mcRet) ? (-1) : (0)); 
#endif
}


int32_t cmdq_sec_allocate_wsm_impl(uint32_t deviceId, uint8_t **ppWsm, uint32_t wsmSize)
{
    int32_t status = 0; 
    enum mc_result mcRet = MC_DRV_OK;  

    printk("[CMDQ][SEC] -->cmdq_sec_allocate_wsm_impl\n");
    do
    {
        if((*ppWsm) != NULL)
        {
            status = -1;
            printk("[CMDQ][SEC] pWsm is not NULL");
            break;
        }
    
        // because world shared mem(WSM) will ba managed by mobicore device, not linux kernel
        // instead of vmalloc/kmalloc, call mc_malloc_wasm to alloc WSM to prvent error...
        // such as "can not resolve tcl physicall address" 
        mcRet = mc_malloc_wsm(deviceId, 0, wsmSize, ppWsm, 0);
        printk("[CMDQ][SEC] mc_malloc_wsm returned: %d\n", mcRet);
        if (MC_DRV_OK != mcRet)
        {
            status = -1;
            break;
        }
    }while(0);        
    printk("[CMDQ][SEC] <--cmdq_sec_allocate_wsm_impl[%d], *ppWsm: %080x\n", status, (*ppWsm));
    return status; 
}


int32_t cmdq_sec_free_wsm_impl(uint32_t deviceId, uint8_t **ppWsm)
{
     enum mc_result mcRet = mc_free_wsm(deviceId, (*ppWsm));
     printk("[CMDQ][SEC] mc_free_wsm, returned: %d, *ppWsm: %080x\n", mcRet, (*ppWsm));

     (*ppWsm) = (MC_DRV_OK == mcRet) ? (NULL) : (*ppWsm);
     printk("[CMDQ][SEC] cmdq_sec_free_wsm_impl *ppWsm: %080x\n", (*ppWsm));
     return ((MC_DRV_OK != mcRet) ? (-1) : (0)); 
}


int32_t cmdq_sec_open_session_impl(
            uint32_t deviceId,  
            const struct mc_uuid_t *uuid, 
            uint8_t *pWsm, 
            uint32_t wsmSize,
            struct mc_session_handle* pSessionHandle)
{
    int32_t status = 0;
    enum mc_result mcRet = MC_DRV_OK;
    do
    {
        if(NULL == pWsm || NULL == pSessionHandle)
        {
            status = -1;
            CMDQ_ERR("cmdq_sec_open_session_impl invalid param, pWsm[0x%08x], pSessionHandle[0x%08x]\n", pWsm, pSessionHandle); 
            break;
        }
    
        memset(pSessionHandle, 0, sizeof(*pSessionHandle));
        pSessionHandle->device_id = deviceId;
        mcRet = mc_open_session(pSessionHandle,
                              uuid, 
                              pWsm,
                              wsmSize);
        printk("[CMDQ][SEC] mc_open_session returned: %d", mcRet);
        if (MC_DRV_OK != mcRet)
        {
            status = -1;
            break;
        }        
    }while(0);

    printk("[CMDQ][SEC] mc_close_session returned: %d\n", mcRet);

    return status; 
}


int32_t cmdq_sec_close_session_impl(struct mc_session_handle* pSessionHandle)
{
    enum mc_result mcRet = mc_close_session(pSessionHandle);
    printk("[CMDQ][SEC] mc_close_session returned: %d\n", mcRet);

    return ((MC_DRV_OK != mcRet) ? (-1) : (0)); 
}


void cmdq_sec_udump_uuid(const struct mc_uuid_t *uuid)
{
    uint32_t i;        
#if 1
    printk("[CMDQ][SEC] wsm[size = %d], uuid: ");
    for(i =0; i < 16; i++)
    {
        printk("%x", uuid->value[i]);
    }
    printk("\n");
#endif
}


int32_t cmdq_sec_init_session(
            const struct mc_uuid_t *uuid, 
            uint8_t** ppWsm, 
            uint32_t wsmSize, 
            struct mc_session_handle* pSessionHandle, 
            IWC_STATE_ENUM *pIwcState)
{
    enum mc_result mcRet;
    int32_t status = 0;
    uint32_t deviceId = MC_DEVICE_ID_DEFAULT;

    printk("[CMDQ][SEC] --> sec init\n");
    cmdq_sec_udump_uuid(uuid);
    do
    {
    	printk("[CMDQ][SEC] wsmSize[%d], pSessionHandle: %080x, sizeof(*pSessionHandle): %d, line:%d\n", 
            wsmSize, pSessionHandle, sizeof(*pSessionHandle),  __LINE__);

        // open mobicore device 
        if(0 > cmdq_sec_open_mobicore_impl(deviceId))
        {
            status = -1;
            break;
        }
        (*pIwcState) = IWC_MOBICORE_OPENED;


        // allocate world shared memory
        if(0 > cmdq_sec_allocate_wsm_impl(deviceId, ppWsm, wsmSize))
        {
             status = -1;
            break;
        }
        (*pIwcState) = IWC_WSM_ALLOCATED;
        printk("[CMDQ][SEC] *ppWsm: %080x\n", (*ppWsm));

        // open a secure session
        if(0 > cmdq_sec_open_session_impl(deviceId, uuid, (*ppWsm), wsmSize, pSessionHandle))
        {
             status = -1;
            break;
        }
        (*pIwcState) = IWC_SES_OPENED;
        
    } while (0);
    
    printk("[CMDQ][SEC] <-- sec init [%d]\n", status);
    return status;
}


int32_t cmdq_sec_fill_iwc_buffer(
            iwcCmdqMessage_t* pIwc,
            uint32_t iwcCommand, 
            TaskStruct *pTask, 
            int32_t thread, 
            CmdqSecFillIwcCB iwcFillCB )
{
    int32_t status = 0;
    //uint32_t metadataSize = pTask->totalSecureFd * sizeof(uint32_t); 
    const uint32_t secFdCount  = pTask->totalSecureFd;
    const uint32_t secFdListLength         = sizeof(uint32_t) * CMDQ_IWC_MAX_FD_COUNT;
    const uint32_t secPortListLength       = sizeof(uint32_t) * CMDQ_IWC_PORTLIST_LENGTH;
    const uint32_t secSizeListLength       = sizeof(uint32_t) * CMDQ_IWC_SIZELIST_LENGTH;


    printk("[CMDQ][SEC] cmdq_sec_fill_iwc_buffer, iwcCommand= %d\n", iwcCommand);
    if(CMD_CMDQ_TL_TEST_DUMMY == iwcCommand || CMD_CMDQ_TL_TEST_SIG_WAIT == iwcCommand)
    {
        printk("[CMDQ][SEC] pIwc = 0x%08x\n", pIwc);
        pIwc->cmd = iwcCommand;
        return 0;
    }
    
    // fill IWC message
    if(NULL != pTask && CMDQ_INVALID_THREAD != thread)
    {
        pIwc->cmd             = iwcCommand;
        pIwc->scenario        = pTask->scenario;
        pIwc->thread          = thread;
        pIwc->priority        = pTask->priority;
        pIwc->engineFlag      = pTask->engineFlag;
        pIwc->cmdBlockSize    = pTask->blockSize; 
        pIwc->totalSecureFd   = pTask->totalSecureFd;
        memcpy((pIwc->pCmdBlockBase), (pTask->pVABase), (pTask->blockSize));
        if(0 < secFdCount)
        {
            memcpy((pIwc->pSecureFdIndex), (pTask->pSecureFdIndex), secFdListLength);
            memcpy((pIwc->pSecurePortList), (pTask->pSecurePortList), secPortListLength);
            memcpy((pIwc->pSecureSizeList), (pTask->pSecureSizeList), secSizeListLength);
        }
        uint32_t* pSrcCMD = pTask->pVABase;
        uint32_t* pDstCMD = pIwc->pCmdBlockBase;

        printk("[CMDQ][SEC] pTask->pVABase = 0x%08x, pIwc->pCmdBlockBase = 0x%08x, cmdBlockSize\n", pTask->pVABase, pIwc->pCmdBlockBase, (pTask->blockSize));
        printk("[CMDQ][SEC] pTask->engineFlag = 0x%08x, pIwc->engineFlag = 0x%08x", pTask->engineFlag, pIwc->engineFlag);
        printk("[CMDQ][SEC] fill_iwc_memcpy, dst_instr[0] = 0x%08x, dst_instr[1] = 0x%08x\n", pDstCMD[0], pDstCMD[1]);
        printk("[CMDQ][SEC] fill_iwc_memcpy, dst_instr[2] = 0x%08x, dst_instr[3] = 0x%08x\n", pDstCMD[2], pDstCMD[3]);
        
        printk("[CMDQ][SEC] fill_iwc_memcpy, src_instr[0] = 0x%08x, src_instr[1] = 0x%08x\n", pSrcCMD[0], pSrcCMD[1]);
        printk("[CMDQ][SEC] fill_iwc_memcpy, src_instr[2] = 0x%08x, src_instr[3] = 0x%08x\n", pSrcCMD[2], pSrcCMD[3]);
        
    }
    else if(NULL != iwcFillCB)
    {
        status = (*iwcFillCB)(iwcCommand, (void*)pIwc);
    }
    else
    {            
        status = -1;    
    }

    printk("[CMDQ][SEC] cmdq_sec_fill_iwc_buffer [%d]\n", status);
    return status;
}


int32_t cmdq_sec_execute_session(
            struct mc_session_handle* pSessionHandle, 
            IWC_STATE_ENUM *pIwcState)
{
    uint32_t deviceId = MC_DEVICE_ID_DEFAULT;
    enum mc_result mcRet;
    int32_t status = 0;
    int32_t i;

    do
    {
        // notify to secure world    
        mcRet = mc_notify(pSessionHandle);
        printk("[CMDQ][SEC] mc_notify[%d]\n", mcRet);
        if (MC_DRV_OK != mcRet)
        {
            status = -1; 
            break;
        }
        (*pIwcState) = IWC_SES_TRANSACTED; 


        // wait respond
        mcRet = mc_wait_notification(pSessionHandle, MC_INFINITE_TIMEOUT); 
        printk("[CMDQ][SEC] mc_wait_notification[%d]\n", mcRet);
        if (MC_DRV_OK != mcRet)
        {
            status = -1; 
            break;
        }
        (*pIwcState) = IWC_SES_ON_TRANSACTED; 
    }while(0);

    return status; 
}


void cmdq_sec_release_session(
        uint8_t **ppWsm, 
        struct mc_session_handle* pSessionHandle, 
        const IWC_STATE_ENUM iwcState)
{
    uint32_t deviceId = MC_DEVICE_ID_DEFAULT;
    enum mc_result mcRet;
    int32_t status = 0;

    printk("[CMDQ][SEC] --> release secssion\n");
    do
    {        
        switch(iwcState)
        {
        case IWC_SES_ON_TRANSACTED: 
        case IWC_SES_TRANSACTED:
        case IWC_SES_MSG_PACKAGED:
             // continue next clean-up
        case IWC_SES_OPENED:
            cmdq_sec_close_session_impl(pSessionHandle);
            // continue next clean-up
        case IWC_WSM_ALLOCATED:
            cmdq_sec_free_wsm_impl(deviceId, ppWsm);
            // continue next clean-up
        case IWC_MOBICORE_OPENED:
            cmdq_sec_close_mobicore_impl(deviceId); 
            // continue next clean-up
            break;
        case IWC_INIT:
            //CMDQ_ERR("open secure driver failed\n");
            break;
        default: 
            break;
        }

    }while(0);

    printk("[CMDQ][SEC] <-- release secssion\n");
    return status; 
}


int32_t cmdq_sec_handle_iwc_result(TaskStruct *pTask, int32_t executeResult)
{
    int32_t status = 0;
    printk("[CMDQ][SEC] %s[%d], pTask[0x%08x]\n", __FUNCTION__, executeResult, pTask);
    
    switch(executeResult)
    {
    case 0:
        status = 0;
        break; 
    default:
        // error case...
        status = -EFAULT;
        break;
    }

    if(pTask)
    {
        pTask->taskState = (0 == status) ? TASK_STATE_DONE: TASK_STATE_ERROR;
    }

    return status;
}


int32_t cmdq_sec_submit_task(
            uint32_t iwcCommand,
            TaskStruct *pTask, 
            int32_t thread,  
            CmdqSecFillIwcCB iwcFillCB )
{
    int32_t status = 0;
    int32_t iwcRsp = 0;
    
    mutex_lock(&gCmdqIwcLock);
    smp_mb();

    const struct mc_uuid_t uuid = CMDQ_TL_UUID;
    IWC_STATE_ENUM iwsState     = IWC_INIT;

    printk("[CMDQ][SEC] --> cmdq_sec_submit_task, gIwc[0x%08x]\n", gIwc);
    do
    {
        // init => expensive
        if(0 > cmdq_sec_init_session(&uuid, (uint8_t**)(&gIwc), sizeof(iwcCmdqMessage_t), &gSessionHandle, &iwsState))
        {
            status = -1;
            break;
        }

        // fill IWC message
        if(0 > cmdq_sec_fill_iwc_buffer(gIwc, iwcCommand, pTask, thread, iwcFillCB))
        {
            status = -1;
            break;
        }

        if(NULL != pTask && NULL != gIwc)
        {
            uint32_t* pSrcCMD = pTask->pVABase;
            uint32_t* pDstCMD = gIwc->pCmdBlockBase;

            printk("[CMDQ][SEC] after fill_iwc, pTask= 0x%08x, pIwc= 0x%08x\n", pTask, gIwc);
            printk("[CMDQ][SEC] after fill_iwc, pTask->pVABase = 0x%08x, pIwc->pCmdBlockBase = 0x%08x\n", pTask->pVABase, gIwc->pCmdBlockBase);
            printk("[CMDQ][SEC] after fill_iwc, dst_instr[0] = 0x%08x, dst_instr[1] = 0x%08x\n", pDstCMD[0], pDstCMD[1]);
            printk("[CMDQ][SEC] after fill_iwc, dst_instr[2] = 0x%08x, dst_instr[3] = 0x%08x\n", pDstCMD[2], pDstCMD[3]);
            printk("[CMDQ][SEC] after fill_iwc, src_instr[0] = 0x%08x, src_instr[1] = 0x%08x\n", pSrcCMD[0], pSrcCMD[1]);
            printk("[CMDQ][SEC] after fill_iwc, src_instr[2] = 0x%08x, src_instr[3] = 0x%08x\n", pSrcCMD[2], pSrcCMD[3]);
        }
        
        // execute        
        cmdq_sec_execute_session(&gSessionHandle, &iwsState);

        // handle iwc respond, and update task state
        iwcRsp = gIwc->rsp;
        cmdq_sec_handle_iwc_result(pTask, iwcRsp);
    }while(0);

    //clean up
    cmdq_sec_release_session((uint8_t**)(&gIwc), &gSessionHandle, iwsState);
    printk("[CMDQ][SEC] <-- cmdq_sec_submit_task, gIwc[0x%08x]\n", gIwc);

    mutex_unlock(&gCmdqIwcLock);
    //spin_lock_irqrestore(&gCmdqIwcLock);
    return status;
}

#endif //__CMDQ_IWC_IMPL__


int32_t cmdq_exec_task_secure_with_retry(
    TaskStruct *pTask, 
    int32_t thread,
    const uint32_t retry)
{
#if defined(__CMDQ_IWC_IMPL__)
    int32_t status = 0;
    int32_t i = 0;
    do 
    {

        uint32_t commandId = (1 == gDebugSecSwCopy) ? (CMD_CMDQ_TL_DEBUG_SW_COPY) : (CMD_CMDQ_TL_SUBMIT_TASK);
        if( 0 > cmdq_sec_submit_task(commandId, pTask, thread, NULL))
        {
            CMDQ_ERR("%s[%d]\n", __FUNCTION__, status);
            status = -EFAULT; 
        }
        i ++; 
    }while(i < retry); 
    
    return status;
#else
    CMDQ_ERR("submit to task when __CMDQ_DCI_IMPL__ disabled\n");   
    return -EFAULT;
#endif //__CMDQ_IWC_IMPL__

}

void cmdq_debug_set_sw_copy(int32_t value)
{
    mutex_lock(&gDebugSecSwCopyLock);
    smp_mb();
    gDebugSecSwCopy = value; 
    mutex_unlock(&gDebugSecSwCopyLock);
}

int32_t cmdq_debug_get_sw_copy(void)
{
    return gDebugSecSwCopy; 
}

#if 0
// TODO: refactor acquire task parameters
extern void cmdq_release_task(TaskStruct *pTask);
extern void cmdq_release_thread(int32_t thread, uint32_t engineFlag);


extern TaskStruct* cmdq_acquire_task(int32_t  scenario,
                                     int32_t  priority,
                                     uint32_t engineFlag,
                                     void     *pCMDBlock,
                                     uint32_t blockSize,
                                     bool     isSecure,
                                     uint32_t *pSecureFdIndex,
                                     uint32_t *pSecurePortList,
                                     uint32_t *pSecureSizeList,
                                     uint32_t totalSecureFd); 
extern int32_t cmdq_acquire_thread(uint32_t engineFlag, bool isSecureTask);

int32_t cmdqSubmitTaskSecure(int32_t  scenario,
                       int32_t  priority,
                       uint32_t engineFlag,
                       void     *pCMDBlock,
                       int32_t  blockSize, 
                       uint32_t *pSecureFdIndex, 
                       uint32_t *pSecurePortList,
                       uint32_t *pSecureSizeList,
                       uint32_t totalSecureFd)
{
    struct timespec start;
    struct timespec done;
    TaskStruct      *pTask;
    int32_t         thread;
    int32_t         retry;
    int32_t         status;
    unsigned long   flags;
    RecordStruct    *pRecord;
    int32_t         execTime;
    bool            isSecureTask = true;

    CMGQ_GET_CURRENT_TIME(start);

    printk("[CMDQ] --> cmdqSubmitTaskSecure, totalSecureFd[%d], pSecureFdIndex[0x%08x], pSecurePortList[0x%08x]\n", totalSecureFd, pSecureFdIndex, pSecurePortList);
    pTask = cmdq_acquire_task(scenario,
                              priority,
                              engineFlag,
                              pCMDBlock,
                              blockSize, 
                              isSecureTask, 
                              pSecureFdIndex,
                              pSecurePortList, 
                              pSecureSizeList,
                              totalSecureFd);
    if (NULL == pTask)
    {
        CMDQ_ERR("Acquire task failed\n");      
        return -EFAULT;
    }

    thread = cmdq_acquire_thread(engineFlag, isSecureTask);
    if (CMDQ_INVALID_THREAD == thread)
    {
        cmdq_release_task(pTask);
        return -EFAULT;
    }

    status = cmdq_exec_task_secure_with_retry(pTask, thread, CMDQ_MAX_RETRY_COUNT); 

    
    cmdq_release_thread(thread, engineFlag);

    CMGQ_GET_CURRENT_TIME(done);

    spin_lock_irqsave(&gCmdqRecordLock, flags);
    smp_mb();

    pRecord = &(gCmdqContext.record[gCmdqContext.lastID]);

    // Record scenario
    pRecord->scenario  = pTask->scenario;
    pRecord->priority  = pTask->priority;
    pRecord->reorder   = pTask->reorder;
    pRecord->isSecure  = true;
    pRecord->thread = thread; 

    // Record time
    pRecord->start     = start;
    pRecord->trigger   = pTask->trigger;
    pRecord->gotIRQ    = pTask->gotIRQ;
    pRecord->wakedUp   = pTask->wakedUp;
    pRecord->done      = done;

    gCmdqContext.lastID++;
    if (gCmdqContext.lastID >= CMDQ_MAX_RECORD_COUNT)
    {
        gCmdqContext.lastID = 0;
    }

    gCmdqContext.recNum++;
    if (gCmdqContext.recNum >= CMDQ_MAX_RECORD_COUNT)
    {
        gCmdqContext.recNum = CMDQ_MAX_RECORD_COUNT;
    }

    spin_unlock_irqrestore(&gCmdqRecordLock, flags);

    CMDQ_GET_TIME_DURATION(pRecord->trigger, pRecord->wakedUp, execTime);
 
    cmdq_release_task(pTask);
    
    printk("[CMDQ] <-- cmdqSubmitTaskSecure, status[%d]", status);
    return status;
}
#endif
