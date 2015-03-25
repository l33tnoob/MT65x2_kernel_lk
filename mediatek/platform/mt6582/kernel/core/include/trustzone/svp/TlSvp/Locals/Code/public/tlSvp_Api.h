#ifndef TLSVP_API_H_
#define TLSVP_API_H_

#include "tci.h"

/**
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */
// TODO: define proper command for normal world caller
#define CMD_SVP_CONFIG_OVL_LAYER 1
#define CMD_SVP_SWITCH_OVL_LAYER 2
#define CMD_SVP_DUMP_OVL_REGISTER 3
#define CMD_SVP_DUMP_DISP_REGISTER 4
#define CMD_SVP_REGISTER_IRQ 5
#define CMD_SVP_UNREGISTER_IRQ 6

#define CMD_SVP_DUMMY 11

#define CMD_SVP_DUMMY_IPC 12
#define CMD_SVP_DUMMY_MEM_ALLOC 13

#define CMD_SVP_CONFIG_OVL_SECURE 14
#define CMD_SVP_CONFIG_OVL_NONSECURE 15

#define CMD_SVP_SWITCH_SECURE_PORT 16
#define CMD_SVP_SWITCH_DEBUG_LAYER 19

#define CMD_SVP_NOTIFY_OVL_LAYER_INFO 17
#define CMD_SVP_NOTIFY_OVL_CONFIG 18

#define CMD_SVP_MAP_OVL_CONFIG 20

#define SECURE_PORT_OVL_0 1
/**
 * Return codes
 */


/**
 * Termination codes
 */
#define EXIT_ERROR                  ((uint32_t)(-1))

/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to processed (cleartext or ciphertext).
 */
typedef struct {
    tciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process or buffer */
    uint32_t            respLen;    /**< Length of response buffer */
} tci_cmd_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */
typedef struct {
    tciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} tci_rsp_t;

/**
 * TCI message data.
 */
typedef struct {
    union {
      tci_cmd_t     cmdSvp;
      tci_rsp_t     rspSvp;
    };
    uint32_t    Value1;
    uint32_t    Value2;
    uint32_t    ResultData;
} tciMessage_t; // TODO: modify this structure to send and receive proper data.

/**
 * Trustlet UUID.
 */
#define SVP_TL_DBG_UUID { { 8, 0xb, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


#endif // TLSVP_API_H_
