/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MNL_COMMON_6620_H_
#define _MNL_COMMON_6620_H_
/*******************************************************************************
* Dependency
*******************************************************************************/
#include <time.h>
#include <cutils/xlog.h>
#include "mtk_gps_type.h"

#define LOG_TAG "mnl_linux"   /*logging in logcat*/
#if defined(ANDROID)
#define MNL_MSG(fmt, arg ...) XLOGD("%s: " fmt, __FUNCTION__ ,##arg)
#define MNL_ERR(fmt, arg ...) XLOGE("%s: " fmt, __FUNCTION__ ,##arg)
#define MNL_TRC(f)            XLOGD("%s\n", __FUNCTION__) 
#define MNL_VER(...)          do {} while(0) 
#else
#define MNL_MSG(...) printf(LOG_TAG":" __VA_ARGS__)
#define MNL_ERR(...) printf(LOG_TAG":" __VA_ARGS__)
#endif

#define GPS_DELETE_EPHEMERIS        0x0001
#define GPS_DELETE_ALMANAC          0x0002
#define GPS_DELETE_POSITION         0x0004
#define GPS_DELETE_TIME             0x0008
#define GPS_DELETE_IONO             0x0010
#define GPS_DELETE_UTC              0x0020
#define GPS_DELETE_HEALTH           0x0040
#define GPS_DELETE_SVDIR            0x0080
#define GPS_DELETE_SVSTEER          0x0100
#define GPS_DELETE_SADATA           0x0200
#define GPS_DELETE_RTI              0x0400
#define GPS_DELETE_CLK				0x0800
#define GPS_DELETE_CELLDB_INFO      0x8000
#define GPS_DELETE_ALL              0xFFFF


#define FLAG_HOT_START	0x0
#define FLAG_WARM_START GPS_DELETE_EPHEMERIS	 
#define FLAG_COLD_START (GPS_DELETE_EPHEMERIS | GPS_DELETE_POSITION)
#define FLAG_FULL_START (GPS_DELETE_ALL)
#define FLAG_AGPS_START (GPS_DELETE_ALL&(~GPS_DELETE_CLK))


extern UINT32 assist_data_bit_map;
/****************************************************************************** 
 * Definition
******************************************************************************/
#define C_INVALID_PID  -1   /*invalid process id*/
#define C_INVALID_TID  -1   /*invalid thread id*/
#define C_INVALID_FD   -1   /*invalid file handle*/
#define C_INVALID_SOCKET -1 /*invalid socket id*/
#define C_INVALID_TIMER -1  /*invalid timer */
/*---------------------------------------------------------------------------*/
enum { 
    MNL_CMD_UNKNOWN = -1,
    /*command send from GPS HAL*/    
    MNL_CMD_INIT            = 0x00,
    MNL_CMD_CLEANUP         = 0x01,
    MNL_CMD_STOP            = 0x02,
    MNL_CMD_START           = 0x03,
    MNL_CMD_RESTART         = 0x04,    /*restart MNL process*/
    MNL_CMD_RESTART_HOT     = 0x05,    /*restart MNL by PMTK command: hot start*/
    MNL_CMD_RESTART_WARM    = 0x06,    /*restart MNL by PMTK command: warm start*/
    MNL_CMD_RESTART_COLD    = 0x07,    /*restart MNL by PMTK command: cold start*/
    MNL_CMD_RESTART_FULL    = 0x08,    /*restart MNL by PMTK command: full start*/
    MNL_CMD_RESTART_AGPS    = 0x09,    /*restart MNL by PMTK command: AGPS start*/

    /*timeout command used in MNLD*/
    MNL_CMD_TIMEOUT         = 0x10,
    MNL_CMD_TIMEOUT_INIT    = 0x11,
    MNL_CMD_TIMEOUT_MONITOR = 0x12,
    MNL_CMD_TIMEOUT_WAKEUP  = 0x13,
    MNL_CMD_TIMEOUT_TTFF    = 0x14, 

    /*control command used in MNLD*/
    MNL_CMD_DEC_FREQ        = 0x20,   
    MNL_CMD_SLEEP           = 0x21,
    MNL_CMD_WAKEUP          = 0x22,
    MNL_CMD_PWROFF          = 0x23,
    MNL_CMD_QUIT            = 0x24, /*quit process*/

    /*command: libmnlx -> MNLD*/
    MNL_CMD_ACTIVE          = 0x30,
    MNL_CMD_SLEPT           = 0x31, /*notify MNLD that mnl_process is already slept*/
    MNL_CMD_RCV_TTFF        = 0x32, /*notify MNLD that mnl_process received TTFF command*/

   /*command sent from GPS HAL*/
   MNL_CMD_READ_EPO_TIME = 0x33,  /*HAL -> MNLD -> libmnlp to read EPO file time */
   MNL_CMD_UPDATE_EPO_FILE = 0x34, /*HAL -> MNLD -> libmnlp to update EPO file after EPO client download done*/

   MNL_CMD_READ_EPO_TIME_DONE = 0x35, /*libmnlp -> MNLD*/
   MNL_CMD_UPDATE_EPO_FILE_DONE = 0x36, /*libmnlp -> MNLD*/

   MNL_CMD_READ_EPO_TIME_FAIL = 0x37,
   MNL_CMD_UPDATE_EPO_FILE_FAIL = 0x38,
   
   MNL_CMD_GPS_LOG_WRITE = 0x40,
};
/*enumeration for NEMA debug level*/
typedef enum
{
    MNL_NMEA_DEBUG_NONE         = 0x0000,
    MNL_NEMA_DEBUG_SENTENCE     = 0x0001, /*only output sentence*/
    MNL_NMEA_DEBUG_RX_PARTIAL   = 0x0002, 
    MNL_NMEA_DEBUG_RX_FULL      = 0x0004,
    MNL_NMEA_DEBUG_TX_FULL      = 0x0008,
    MNL_NMEA_DEBUG_STORAGE      = 0x0010,
    MNL_NMEA_DEBUG_MESSAGE      = 0x0020, /*mtk_sys_msg_send/mtk_sys_msg_recv*/

    MNL_NMEA_DISABLE_NOTIFY     = 0x1000,
    /*output sentence and brief RX data*/
    MNL_NMEA_DEBUG_NORMAL       = MNL_NEMA_DEBUG_SENTENCE | MNL_NMEA_DEBUG_RX_PARTIAL, 
    /*output sentence and full RX/TX data*/
    MNL_NMEA_DEBUG_FULL         = 0x00FF,
} MNL_NMEA_DEBUG;
/*---------------------------------------------------------------------------*/
#define PMTK_CONNECTION_SOCKET 1
#define PMTK_CONNECTION_SERIAL 2
/****************************************************************************** 
 * Function Configuration
******************************************************************************/
#define READ_PROPERTY_FROM_FILE
/****************************************************************************** 
 * structure
******************************************************************************/
/* configruation property */
typedef struct
{
    /*used by mnl_process.c*/
    int init_speed;
    int link_speed;    
    int debug_nmea;
    int debug_mnl;
    int pmtk_conn;          /*PMTK_CONNECTION_SOCKET | PMTK_CONNECTION_SERIAL*/
    int socket_port;        /*PMTK_CONNECTION_SOCKET*/
    char dev_dbg[32];       /*PMTK_CONNECTION_SERAIL*/
    char dev_dsp[32];
    char dev_gps[32];
    char bee_path[32];
    char epo_file[30];
    char epo_update_file[30];
    int delay_reset_dsp;    /*the delay time after issuing MTK_PARAM_CMD_RESET_DSP*/
    int nmea2file;
    int dbg2file;
    int nmea2socket;
    int dbg2socket;
    int test_mode;

    /*used by mnld.c*/   
    int timeout_init;
    int timeout_monitor;
    int timeout_wakeup;    
    int timeout_sleep;
    int timeout_pwroff;
    int timeout_ttff;

    /*used by agent*/
    int EPO_enabled;
    int BEE_enabled;
    int SUPL_enabled;
    int SUPLSI_enabled;
    int EPO_priority;
    int BEE_priority;
    int SUPL_priority;    

    /*for FM TX*/
    int AVAILIABLE_AGE;
    int RTC_DRIFT;
    int TIME_INTERVAL;    
    int ACCURACY_SNR; 
    char u1AgpsMachine;  
} MNL_CONFIG_T;
/*---------------------------------------------------------------------------*/
typedef struct
{
    time_t uSecond_start;
    time_t uSecond_expire;
}MNL_EPO_TIME_T;
/*---------------------------------------------------------------------------*/
typedef struct
{
    int notify_fd;
    int port;
} MNL_DEBUG_SOCKET_T;
/*---------------------------------------------------------------------------*/
#ifdef _MNL_COMMON_6620_C_
    #define C_EXT   
#else
    #define C_EXT   extern
#endif    
/*---------------------------------------------------------------------------*/
C_EXT int  mnl_utl_load_property(MNL_CONFIG_T* prConfig); 
/*---------------------------------------------------------------------------*/    
#undef C_EXT
/*---------------------------------------------------------------------------*/
#endif
