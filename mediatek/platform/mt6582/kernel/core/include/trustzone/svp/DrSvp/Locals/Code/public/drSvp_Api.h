/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * @file   drSvp_Api.h
 * @brief  Contains DCI command definitions and data structures
 *
 */

#ifndef __DRSVPAPI_H__
#define __DRSVPAPI_H__

#include "dci.h"


/**
 * Command ID's
  */
#define CMD_ID_01       1
#define CMD_ID_02       2
/*... add more command ids when needed */
#define CMD_SVP_DRV_DUMP_OVL_REGISTER 3
#define CMD_SVP_DRV_DUMP_DISP_REGISGER 4

#define CMD_SVP_DRV_SWITCH_OVL_LAYER 5
#define CMD_SVP_DRV_CONFIG_OVL_LAYER 6

#define CMD_SVP_DRV_DUMMY 11

/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to be processed
 */
typedef struct {
    dciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process */
} dci_cmd_t;


/**
 * Response structure
 */
typedef struct {
    dciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} dci_rsp_t;



/**
 * Sample 01 data structure
 */
typedef struct {
    uint32_t len;
    uint32_t addr;
} sample01_t;


/**
 * Sample 02 data structure
 */
typedef struct {
    uint32_t data;
} sample02_t;


/**
 * DCI message data.
 */
typedef struct {
    union {
        dci_cmd_t     command;
        dci_rsp_t     response;
    };

    union {
        sample01_t  sample01;
        sample02_t  sample02;
    };
    uint32_t    Value1;
    uint32_t    Value2;
    uint32_t    ResultData;
} dciMessage_t;

/**
 * Driver UUID. Update accordingly after reserving UUID
 */
#define SVP_DRV_DBG_UUID { { 8, 0xc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


#endif // __DRSVPAPI_H__
