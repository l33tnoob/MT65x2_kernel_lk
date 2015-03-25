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

#ifndef _MNL_LINUX_6620_H_
#define _MNL_LINUX_6620_H_

/******************************************************************************
* Configuration
******************************************************************************/
#define MNL_PORTING_LAYER //Steve test
#define SUPPORT_HOTSTILL
//#define SUPPORT_DSP_RW
//#define SUPPORT_AGPS
//#define SUPPORT_MP_TEST
//#define MNL_DEBUG
#define GPS_AGENT
#define MTK_GPS_NVRAM
/******************************************************************************
* Dependency
******************************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <linux/mtk_agps_common.h>
#include "mtk_gps.h"
#include "mtk_gps_agps.h"
#include "mnl_common_6620.h"
         

#if defined(ANDROID)/*********************************************************/
#define LOG_TAG "mnl_linux" /*logging in logcat*/
#include <cutils/log.h>     /*logging in logcat*/
#endif
/******************************************************************************
* Macro & Definition
******************************************************************************/
#define DSP_UART_IN_READ_SIZE       (256)
#define PMTK_CMD_BUFFER_SIZE        (256)
#define PMTK_UART_IN_BUFFER_SIZE    (512)
#define PMODE                       (0600)
#define MNL_MQ_NAME                 "/mnl_msg_queue"
#define MNL_AGPS_MQ_NAME            "/mnl_agps_msg_queue"
#define DSP_DEV                     "/dev/stpgps" /* stp-gps char dev */
#define GPS_DEV                     "/dev/gps"
#define DBG_DEV                     "/dev/ttygserial"
#define BEE_PATH                    "/data/misc/"
#define EPO_FILE                    "/data/misc/EPO.DAT"
#define EPO_UPDATE_FILE             "/data/misc/EPOTMP.DAT"
#define EPO_UPDATE_HAL				"/data/misc/EPOHAL.DAT"
#define LOG_FILE                    "/data/misc/gpsdebug.log"
#define DSP_BIN_LOG_FILE            "/data/misc/DSPdebug.log"
/*#define LOG_FILE                    "/sdcard/gpsdebug.log"*/
#define PARM_FILE                   "/data/misc/gpsparm.dat"
#define NV_FILE                     "/data/misc/mtkgps.dat"
#define TCXO_FLAG                   MTK_TCXO_PHONE   //default enable TCXO of phone application
#define AIC_FLAG                    MTK_AIC_OFF      //default disable Anti-jamming

#define CFG_DBG_INFO_UART_OUTPUT   0
#define CFG_DBG_INFO_GPIO_TOGGLE   0
#if CFG_DBG_INFO_UART_OUTPUT
/* MT6516 UART2 */

/*To dump GPS nmeadump with UART3*/
#define DBG_INFO_UART_DEV "/dev/ttyMT2"
#endif

#if defined(ANDROID)/*********************************************************/
/*#define MONITOR_TAG "           "
#define MNL_VER(...) do {} while (0)
#define MNL_MSG(...) do {printf(__VA_ARGS__); LOGE(__VA_ARGS__);} while(0)
#define MNL_MON(...) do {printf(MONITOR_TAG __VA_ARGS__); LOGE(__VA_ARGS__);} while(0)
#define MNL_ERR(...) do {printf(__VA_ARGS__); LOGE(__VA_ARGS__);} while(0)
*/
#define MNL_MSG(fmt, arg ...) XLOGD("%s: " fmt, __FUNCTION__ ,##arg)
#define MNL_ERR(fmt, arg ...) XLOGE("%s: " fmt, __FUNCTION__ ,##arg)
#define MNL_TRC(f)            XLOGD("%s\n", __FUNCTION__) 
#define MNL_VER(...)          do {} while(0) 
#else /***********************************************************************/
#define MNL_VER printf
#define MNL_MON printf
#define MNL_ERR printf
#endif

/******************************************************************************
* Structure & Enumeration
******************************************************************************/
typedef struct
{
    int msg_type;
    MTK_GPS_AGPS_AGENT_MSG *msg_ptr;
}mnl_agps_msg_struct;

/* Ring buffer for message */
typedef struct      // Ring buffer
{
	MTK_GPS_AGPS_AGENT_MSG** next_write;     // next position to write to
	MTK_GPS_AGPS_AGENT_MSG** next_read;      // next position to read from
	MTK_GPS_AGPS_AGENT_MSG** start_buffer;   // start of buffer
	MTK_GPS_AGPS_AGENT_MSG** end_buffer;     // end of buffer + 1
} agps_msg_ring_buf;
/******************************************************************************
* share global variables
******************************************************************************/
#ifdef _MTK_GPS_6620_C_
    #define C_EXT
#else
    #define C_EXT   extern
#endif
/*---------------------------------------------------------------------------*/
C_EXT int dsp_fd
    #ifdef _MTK_GPS_6620_C_
    = C_INVALID_FD
    #endif
    ;
/*---------------------------------------------------------------------------*/
C_EXT int gps_fd
    #ifdef _MTK_GPS_6620_C_
    = C_INVALID_FD
    #endif
    ;
/*---------------------------------------------------------------------------*/
C_EXT int dbg_fd
    #ifdef _MTK_GPS_6620_C_
    = C_INVALID_FD
    #endif
    ;

#if CFG_DBG_INFO_UART_OUTPUT
C_EXT int dbg_info_uart_fd
    #ifdef _MTK_GPS_6620_C_
    = C_INVALID_FD
    #endif
    ;
#endif

/*---------------------------------------------------------------------------*/
#if CFG_DBG_INFO_GPIO_TOGGLE
C_EXT int gpio_fd
	#ifdef _MTK_GPS_6620_C_
	= C_INVALID_FD
	#endif
	;
#endif

/*---------------------------------------------------------------------------*/
C_EXT int nmea_debug_level
    #ifdef _MTK_GPS_6620_C_
    #ifdef MNL_DEBUG
    = MNL_NMEA_DEBUG_FULL
    #else
    = MNL_NMEA_DEBUG_NORMAL
    #endif
#endif
   	 ;
/******************************************************************************
* Function Prototye
******************************************************************************/
C_EXT int mtk_sys_init();
C_EXT int mtk_sys_uninit();
C_EXT INT32 mtk_sys_nmea_output (char* buffer, UINT32 length);
C_EXT void mtk_sys_ttff_handler(int type);
/*---------------------------------------------------------------------------*/
#undef C_EXT
/*---------------------------------------------------------------------------*/
#endif
