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

#ifndef ATCID_CUST_CMD_PROCESS_H
#define ATCID_CUST_CMD_PROCESS_H

#include "atcid_cmd_dispatch.h"
#include "atcid_cust_cmd.h"
#include "atcid_util.h"

typedef struct custom_cmd_type
{
    char cmdName[MAX_AT_COMMAND_LEN];                                               //The prefix of AT command name    
    ATOP_t opType;
    ATRESPONSE_t (*cmd_handle_func)(char* cmdline, ATOP_t opType, char* response);      //Command Handler function in case of no IOControl support
} customcmd_type;


static customcmd_type custom_cmd_table[] = {
        {"ATQ0V1E", AT_BASIC_OP, pas_echo_handler},                 //Add for device echo
        {"ATE", AT_BASIC_OP, pas_echo_handler},                     //Add for device echo
        {"AT#CLS", AT_TEST_OP, pas_modem_handler},                  //Add for device manager issue
        {"AT+GCI", AT_READ_OP | AT_TEST_OP, pas_modem_handler},     //Add for device manager issue
        {"AT+CCLK", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_cclk_handler},
        {"AT+ESUO", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_esuo_handler},
        {"AT+ATCI", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_atci_handler},
        {"AT%REBOOT", AT_ACTION_OP, pas_reboot_handler},
        {"AT^WIENABLE", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wienable_handler},
        {"AT^WIMODE", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wimode_handler},
        {"AT^WIBAND", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wiband_handler},
        {"AT^WIFREQ", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wifreq_handler},
        {"AT^WIDATARATE", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_widatarate_handler},
        {"AT^WIPOW", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wipow_handler},
        {"AT^WITX", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_witx_handler},
        {"AT^WIRX", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wirx_handler},
        {"AT^WIRPCKG", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wirpckg_handler},
    };


#endif