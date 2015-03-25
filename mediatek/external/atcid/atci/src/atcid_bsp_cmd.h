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

#ifndef ATCID_BSP_CMD_H
#define ATCID_BSP_CMD_H

#include "atcid_cmd_dispatch.h"
#include <linux/android_alarm.h>
#include "mtkfb.h"
//#include "leds-ioctl.h"

#define ARALM_DEV_PATH "/dev/alarm"
#define MSDC_DEV_PATH "/dev/mt6573-sd0"
#define FB_DEV_PATCH  "/dev/graphics/fb0"
#define LEDS_DEV_PATH "/dev/mt6573_leds"

typedef struct bsp_cmd_type
{
    char cmdName[MAX_AT_COMMAND_LEN];                           //The prefix of AT command name
    ATOP_t opType;                                              //The supported operator type 
    char devPath[MAX_DEVICE_PATH_LEN];                          //The FD of device
    unsigned int cmdId;                                         //The IOControl command
} bsp_cmd_type;

typedef struct cmd_data_type
{
    char cmdDataRequest[MAX_DATA_SIZE];                         //The AT command request
    ATOP_t opType;
    char cmdDataResponse[MAX_AT_RESPONSE];                      //The AT command response
} cmd_data_type;

static bsp_cmd_type bsp_cmd_table[] = {
        {"AT%ACCEL", AT_ACTION_OP, "", 0},
        {"AT%ACS", AT_ACTION_OP, "", 0},
        {"AT%ALARM", AT_ACTION_OP, ARALM_DEV_PATH, ANDROID_ALARM_SET_RTC},
        {"AT%EMT", AT_READ_OP|AT_TEST_OP|AT_SET_OP|AT_ACTION_OP, MSDC_DEV_PATH, 7}, //MSDC_ATCMD_EMT},
//	    {"AT%LCD", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, FB_DEV_PATCH, MTKFB_ATCI_TEST_LCD}, 
//	    {"AT%KCAL", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, FB_DEV_PATCH, MTKFB_ATCI_TEST_KCAL},
//        {"AT%BOFF", AT_ACTION_OP,  LEDS_DEV_PATH, LEDS_IO_LCD_BACKLIGHT_OFF},

    };

#endif
