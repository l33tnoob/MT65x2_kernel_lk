#ifndef __CMDQ_SEC_IWC_COMMON_H__
#define __CMDQ_SEC_IWC_COMMON_H__


#define CMDQ_MAX_BLOCK_SIZE     (32 * 1024)

#define CMDQ_IWC_MAX_CMD_LENGTH (32 * 1024 / 4)

#define CMDQ_IWC_ACCESS_ENGINE     (3) // RDMA, WDMA, WROT
#define CMDQ_IWC_MAX_FD_COUNT      (6 * CMDQ_IWC_ACCESS_ENGINE) 
#define CMDQ_IWC_PORTLIST_LENGTH   (3 * CMDQ_IWC_ACCESS_ENGINE) 
#define CMDQ_IWC_SIZELIST_LENGTH   (3 * CMDQ_IWC_ACCESS_ENGINE) 



typedef struct 
{
    uint32_t instrIndex; // _d, indicate x-th instruction 
    uint32_t baseHandle; // _h, secure handle
    uint32_t offset;     // _b, buffser offset to secure handle

    // mva config
    bool     isMav; 
    uint32_t size; 
    uint32_t port; 
} iwcCmdqAddressMetadata_t;


typedef struct 
{
    uint32_t port;       // port id 
} iwcCmdqPortMetadata_t;


typedef struct {
    uint32_t logLevel;
} iwcCmdqDebugConfig_t; 


// linex kernel and mobicore has their own MMU tables, 
// the former's one is used to map world shared memory and physical address
// so mobicore dose not understand linux virtual address mapping.
//
// if we want to transact a large buffer in TCI/DCI, there are 2 method (both need 1 copy): 
// 1. use mc_map, to map normal world buffer to WSM, and pass secure_virt_addr in TCI/DCI buffer
//    note mc_map implies a memcopy to copy content from normal world to WSM
// 2. declare a fixed lenth array in TCI/DCI struct, and its size must be < 1M
// 
typedef struct {
    union
    {
        uint32_t cmd; 
        uint32_t rsp;
    };
    
    uint32_t thread; 
    uint32_t scenario; 
    uint32_t priority; 
    uint32_t engineFlag;
    uint32_t pCmdBlockBase[CMDQ_IWC_MAX_CMD_LENGTH]; //rename as cmdBlock
    uint32_t cmdBlockSize; 

    uint32_t totalSecureFd;
    uint32_t pSecureFdIndex[CMDQ_IWC_MAX_FD_COUNT];  //secHandleIdx, CMDQ_IWC_MAX_FD_COUNT
    uint32_t pSecurePortList[CMDQ_IWC_PORTLIST_LENGTH]; 
    uint32_t pSecureSizeList[CMDQ_IWC_SIZELIST_LENGTH]; 
} iwcCmdqMessage_t, *iwcCmdqMessage_ptr;


// CMDQ secure engine 
// the engine id should be same as the normal CMDQ 
typedef enum CMDQ_ENG_SEC_ENUM
{
    // CAM
    CMDQ_ENG_SEC_ISP_IMGI   = 0,
    CMDQ_ENG_SEC_ISP_IMGO   = 1,
    CMDQ_ENG_SEC_ISP_IMG2O  = 2,

    // MDP
    CMDQ_ENG_SEC_MDP_RDMA0  = 3,
    CMDQ_ENG_SEC_MDP_CAMIN  = 4,
    CMDQ_ENG_SEC_MDP_SCL0   = 5,
    CMDQ_ENG_SEC_MDP_SCL1   = 6,
    CMDQ_ENG_SEC_MDP_TDSHP  = 7,
    CMDQ_ENG_SEC_MDP_WROT   = 8,
    CMDQ_ENG_SEC_MDP_WDMA1  = 9,

    CMDQ_ENG_SEC_LSCI   = 10,
    CMDQ_ENG_SEC_CMDQ   = 11,

    CMDQ_ENG_SEC_ISP_Total  = 12,
}CMDQ_ENG_SEC_ENUM;


// ERROR code

#define CMDQ_ERR_DR_IPC_EXECUTE_SESSION  (-5001)
#define CMDQ_ERR_DR_IPC_CLOSE_SESSION    (-5002)


#endif // __CMDQ_SEC_TLAPI_H__
