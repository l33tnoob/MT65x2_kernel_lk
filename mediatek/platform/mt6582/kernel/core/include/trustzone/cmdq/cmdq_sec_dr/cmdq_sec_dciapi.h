#ifndef __CMDQ_SEC_DCIAPI_H__
#define __CMDQ_SEC_DCIAPI_H__

#include "dci.h"
//#include "cmdq_sec_iwc_common.h"


/**
 * Command ID's
  */
#define DCI_CMDQ_TASK_SUBMIT  0
#define DCI_CMDQ_TASK_DUMP    1
#define DCI_CMDQ_TEST         2


/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to be processed
 */
typedef struct {
    dciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process */
} cmdqCmd_t;


/**
 * Response structure
 */
typedef struct {
    dciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} cmdqRsp_t;

/**
 * DCI message 
 */
typedef struct {
    union {
        cmdqCmd_t cmd;
        cmdqRsp_t rsp;
    };

    //iwcCmdqMessage_t  cmdqCmdParam;
} dciCmdQMessage_t;


/**
 * Driver UUID. Update accordingly after reserving UUID
 */

#define DRV_CMDQ_SEC_UUID { { 2, 0xb, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


#endif // __CMDQ_SEC_DCIAPI_H__
