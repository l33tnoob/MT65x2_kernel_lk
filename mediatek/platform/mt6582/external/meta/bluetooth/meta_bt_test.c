/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "meta_bt.h"

static void bt_info_callback(BT_CNF *cnf, void *buf, unsigned int size)
{

    unsigned int i;
    char *type[] = { 
        "BT_OP_HCI_SEND_COMMAND", "BT_OP_HCI_CLEAN_COMMAND", "BT_OP_HCI_SEND_DATA", "BT_OP_HCI_TX_PURE_TEST",
        "BT_OP_HCI_RX_TEST_START", "BT_OP_HCI_RX_TEST_END", "BT_OP_HCI_TX_PURE_TEST_V2", "BT_OP_HCI_RX_TEST_START_V2",
        "BT_OP_ENABLE_NVRAM_ONLINE_UPDATE", "BT_OP_DISABLE_NVRAM_ONLINE_UPDATE", "BT_OP_ENABLE_PCM_CLK_SYNC_SIGNAL", "BT_OP_DISABLE_PCM_CLK_SYNC_SIGNAL"};

    printf("[META_BT] <CNF> %s, bt status: %d, event: %d, Status: %d\n", 
        type[cnf->op],     
        cnf->bt_status, cnf->eventtype, cnf->status);

    if(cnf->eventtype == 1){
        /* unsigned char status;
        unsigned char parms[256]; */
        printf("[META_BT] hci event %d len: %d\n", cnf->bt_result.hcievent.event, cnf->bt_result.hcievent.len);
        printf("[META_BT] hci event %02x-%02x-%02x-%02x\n", 
            (int)cnf->bt_result.hcievent.parms[0], (int)cnf->bt_result.hcievent.parms[1],
            (int)cnf->bt_result.hcievent.parms[2], (int)cnf->bt_result.hcievent.parms[3]);

    }else if(cnf->eventtype == 3){
        printf("[META_BT] hci acl con_hdl %d len: %d\n", cnf->bt_result.hcibuf.con_hdl, cnf->bt_result.hcibuf.len);
    }else{
        printf("[META_BT] Unexpected eventtype\n"); 
    }
}

int main(int argc, const char** argv)
{
    BT_REQ req;

    memset(&req, 0, sizeof(BT_REQ));

    META_BT_Register(bt_info_callback);

    if (META_BT_init() == false) {
        printf("BT init failed\n");
        return -1;
    }
#if 0
    req.op = BT_OP_HCI_SEND_COMMAND;
    req.cmd.hcicmd.opcode = 0x0c03;
    req.cmd.hcicmd.len = 0;
    req.cmd.hcicmd.parms[0] = 0;
    META_BT_OP(&req, NULL, 0);

    sleep(1);

    req.op = BT_OP_HCI_CLEAN_COMMAND;
    META_BT_OP(&req, NULL, 0);

    sleep(1);

    req.op = BT_OP_HCI_SEND_COMMAND;
    req.cmd.hcicmd.opcode = 0xfc72;
    req.cmd.hcicmd.len = 1;
    req.cmd.hcicmd.parms[0] = 0x23;
    META_BT_OP(&req, NULL, 0);

    sleep(1);

    req.op = BT_OP_HCI_CLEAN_COMMAND;
    META_BT_OP(&req, NULL, 0);
#endif
    sleep(1);
    /* 1,4,5,33,8B,9E,30,A */
    req.op = BT_OP_HCI_SEND_COMMAND;
    req.cmd.hcicmd.opcode = 0x0401;
    req.cmd.hcicmd.len = 5;
    req.cmd.hcicmd.parms[0] = 0x33;
    req.cmd.hcicmd.parms[1] = 0x8B;
    req.cmd.hcicmd.parms[2] = 0x9E;
    req.cmd.hcicmd.parms[3] = 0x05;
    req.cmd.hcicmd.parms[4] = 0x0A;
    META_BT_OP(&req, NULL, 0);

    sleep(20);

    req.op = BT_OP_HCI_CLEAN_COMMAND;
    META_BT_OP(&req, NULL, 0);
    
    sleep(1);
    
    META_BT_deinit();
    META_BT_Register(NULL);
    
    return 0;
}

