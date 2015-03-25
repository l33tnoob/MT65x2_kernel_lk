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

#ifndef ATCID_CMD_PROCESS_H
#define ATCID_CMD_PROCESS_H
#include <sys/ioctl.h>
#include "atcid_serial.h"
#include "atcid_util.h"

#define MAX_AT_COMMAND_LEN 24
#define MAX_DEVICE_PATH_LEN 20

typedef struct generic_cmd_type
{
    char cmdName[MAX_AT_COMMAND_LEN];                           //The prefix of AT command name
} generic_cmd_type;


static generic_cmd_type generic_cmd_table[] = {
    {"AT%ACS"}, 
	  {"AT%ALB"}, 
    {"AT%ECALL"},
    {"AT%FLIGHT"},
#if defined(MTK_GPS_SUPPORT)
    {"AT%GPS"},
    {"AT%GNSS"},
#endif
    {"AT%MPT"},
    {"AT%SPM"},
    {"AT%TEST"},
    {"AT%FMR"},
#if defined(MTK_WLAN_SUPPORT)
    {"AT%WLANT"},
    {"AT%WLANR"},
    {"AT%WLAN"},
    {"AT%MACCK"},
    {"AT%MAC"},
#endif
    {"AT%LANG"},
    {"AT%VLC"},
    {"AT%MMCCHK"},
    {"AT%MMCFORMAT"},
    {"AT%MMCTOTALSIZE"},
    {"AT%MMCUSEDSIZE"},
    {"AT%CODECRC"},
    {"AT%INITDB"},
    {"AT%DBCHK"},
    {"AT%OSVER"},
#if defined(MTK_NFC_SUPPORT)    
    {"AT%NFC"},
#endif    
    {"AT%FBOOT"},
    {"AT%RESTART"},
    {"AT%NOSLEEP"},
    {"AT%LEDON"},
    {"AT%MOT"},
    {"AT%FKPD"},
    {"AT%GKPD"},
    {"AT%CAM"},
    {"AT%AVR"},
    {"AT%IMEI"},
    {"AT%IMEI2"},
    {"AT%BTAD"},
    {"AT%BTTM"},
    {"AT+SN"},
    {"AT+MODEL"},
    {"AT+SHUTDOWN"},
    {"AT+POWERKEY"},
	{"AT+FACTORYRESET"},
	{"AT%PQ"}, 
	{"AT%MJC"}, 	
};

static generic_cmd_type bat_cmd_table[] = {
        {"AT@GCSF"}, 	// AT command for get charing state flag, 1: yes, 0: no
        {"AT@GBV"}, 		// AT command for get battery voltage
        {"AT@SCC"}, 		// AT command for set charing state
};

int process_cmd_line(char* line);
char* cut_cmd_line(char* line);
int audio_command_hdlr(char* line);

#endif
