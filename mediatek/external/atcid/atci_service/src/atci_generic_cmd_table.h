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

#ifndef ATCI_GENERIC_CMD_TABLE_H
#define ATCI_GENERIC_CMD_TABLE_H

#define MAX_AT_COMMAND_LEN 32
#define MAX_AT_RESPONSE 2048
#define MAX_DATA_SIZE 2048

#include "atci_telephony_cmd.h"
#include "atci_system_cmd.h"
#include "atci_audio_cmd.h"
#if defined(ENABLE_GPS_AT_CMD)
    #include "atci_gps_cmd.h"
#endif
#if defined(ENABLE_WLAN_AT_CMD)
    #include "atci_wlan_cmd.h"
#endif
#if defined(ENABLE_MMC_AT_CMD)
    #include "atci_mmc_cmd.h"
#endif
#if defined(ENABLE_CODECRC_AT_CMD)
    #include "atci_code_cmd.h"
#endif
#if defined(ENABLE_BLK_VIBR_AT_CMD)
#include "atci_lcdbacklight_vibrator_cmd.h"
#endif
#if defined(ENABLE_KPD_AT_CMD)
#include "atci_kpd_cmd.h"
#endif

#if defined(ENABLE_NFC_AT_CMD)
    #include "atci_nfc_cmd.h"
#endif

#include "atci_pq_cmd.h"
#include "atci_mjc_cmd.h"


int process_generic_command(char* line);
char* cut_cmd_line(char* line);

typedef struct mmi_cmd_type
{
    char cmdName[MAX_AT_COMMAND_LEN];   //The prefix of AT command name
    ATOP_t opType;
} mmi_cmd_type;


typedef struct generic_cmd_type
{
    char cmdName[MAX_AT_COMMAND_LEN];                           //The prefix of AT command name    
    ATOP_t opType;                                              //The suppport operation type
    int (*cmd_handle_func)(char* cmdline, ATOP_t opType, char* response);      //Generic command handler function
} generic_cmd_type;


//Handle those commands in ATCI generic service
static generic_cmd_type generic_cmd_table[] = {
	      {"AT%ACS", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, audio_circuit_handler},
		    {"AT%ALB", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, audio_pure_loopback},
//		{"AT%MPT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, audio_mp3_handler},  /*Donglei add */
		    {"AT%SPM", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, audio_speaker_phone_handler},
        {"AT%TEST", AT_TEST_OP, pas_ecall_handler},  //Add for test
        {"AT%VLC", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, audio_volume_control_handler}, 
#if defined(ENABLE_GPS_AT_CMD)
        {"AT%GPS", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_gps_handler}, //Add for GPS test
        {"AT%GNSS", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_gnss_handler},
#endif
//Haman changed for TC3
#if defined(ENABLE_WLAN_AT_CMD) && 0
//changed end
        {"AT%WLANT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, wlan_tx_test_cmd_handler},
        {"AT%WLANR", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, wlan_rx_test_cmd_handler},
        {"AT%WLAN", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, wlan_generic_cmd_handler},
        {"AT%MACCK", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, wlan_macaddr_check_cmd_handler}, 
        {"AT%MAC", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, wlan_macaddr_rw_cmd_handler},
#endif
#if defined(ENABLE_MMC_AT_CMD)
        {"AT%MMCCHK",       AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, mmc_chk_handler},  //Check the SD/MMC Card
        {"AT%MMCFORMAT",    AT_ACTION_OP                                      , mmc_format_handler},  //Format the SD/MMC Card
        {"AT%MMCTOTALSIZE", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, mmc_totalsize_handler},  //Get the total size of SD/MMC Card
        {"AT%MMCUSEDSIZE",  AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, mmc_usedsize_handler},  //Get the used size of SD/MMC Card
#endif
#if defined(ENABLE_CODECRC_AT_CMD)
        {"AT%CODECRC",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, code_crc_handler},  //Get the crc code of system image
#endif
#if defined(ENABLE_BLK_VIBR_AT_CMD)
        {"AT%LEDON",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, lcdbacklight_power_on_cmd_handler},
        {"AT%MOT",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, vibrator_power_off_cmd_handler},
#endif
#if defined(ENABLE_KPD_AT_CMD)
        {"AT%FKPD",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, fkpd_cmd_handler},
        {"AT%GKPD",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, gkpd_cmd_handler},
#endif

#if defined(ENABLE_NFC_AT_CMD)
         {"AT%NFC", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, nfc_cmd_handler}, //For NFC
#endif

		{"AT%FBOOT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, system_fboot_handler},
		{"AT%RESTART", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, system_restart_handler},
		
		{"AT%IMEI", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, misc_imei_cmd_handler},
		{"AT%IMEI2", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, misc_imei2_cmd_handler},
		{"AT%PQ", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pq_cmd_handler},
		{"AT%MJC", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, mjc_cmd_handler},		
    };

static mmi_cmd_type mmi_cmd_table[] = {
        {"AT%ECALL", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT%FLIGHT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT%LANG", AT_ACTION_OP | AT_READ_OP},
        {"AT%INITDB", AT_ACTION_OP | AT_READ_OP},
        {"AT%DBCHK", AT_ACTION_OP | AT_READ_OP},
        {"AT%OSVER", AT_ACTION_OP | AT_READ_OP},
        {"AT%MPT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT%NOSLEEP", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT%CAM", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT%AVR", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT+SHUTDOWN", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT+SN", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT+MODEL", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT+POWERKEY", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT+FACTORYRESET", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT+WITOF", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT%BTAD", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
        {"AT%BTTM", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP},
    };

#endif
