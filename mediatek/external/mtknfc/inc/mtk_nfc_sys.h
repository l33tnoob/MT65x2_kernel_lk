/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2012
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*******************************************************************************
 * Filename:
 * ---------
 *  mtk_nfc_sys.h
 *
 * Project:
 * --------
 *  NFC
 *
 * Description:
 * ------------
 *  Operation System Abstration Layer Implementation
 *
 * Author:
 * -------
 *  Hiki Chen, ext 25281, hiki.chen@mediatek.com, 2012-05-10
 * 
 *******************************************************************************/

#ifndef MTK_NFC_SYS_H
#define MTK_NFC_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************** 
 * Include
 *****************************************************************************/ 
#include "mtk_nfc_sys_type.h"

#define DEBUG_LOG

#define SUPPORT_BLOCKING_READ_MECHANISM

/***************************************************************************** 
 * Define
 *****************************************************************************/
#define NFC_DLL_EXEC_SOCKET_CONN

#define MTK_NFC_TIMER_INVALID_ID    0xFFFFFFFF

#define MTK_NFC_MAX_ATR_LENGTH  (48)

#define MTK_P2P_LLCP_DM_OPCODE_DISCONNECTED               0x00
#define MTK_P2P_LLCP_DM_OPCODE_SAP_NOT_ACTIVE             0x01
#define MTK_P2P_LLCP_DM_OPCODE_SAP_NOT_FOUND              0x02
#define MTK_P2P_LLCP_DM_OPCODE_CONNECT_REJECTED           0x03
#define MTK_P2P_LLCP_DM_OPCODE_CONNECT_NOT_ACCEPTED       0x20
#define MTK_P2P_LLCP_DM_OPCODE_SOCKET_NOT_AVAILABLE       0x21


// Service socket error code

#define MTK_P2P_LLCP_ERR_DISCONNECTED               0x00
#define MTK_P2P_LLCP_ERR_FRAME_REJECTED             0x01
#define MTK_P2P_LLCP_ERR_BUSY_CONDITION             0x02
#define MTK_P2P_LLCP_ERR_NOT_BUSY_CONDITION         0x03

/***************************************************************************** 
 * NFC Return Value for APIs
 *****************************************************************************/
#define MTK_NFC_SUCCESS     (0)
#define MTK_NFC_ERROR       (-1)
#define MTK_NFC_TIMEOUT     (-2)


/***************************************************************************** 
 * NFC specific types
 *****************************************************************************/
typedef UINT32  NFCSTATUS;  /* NFC return values */
typedef unsigned int MTK_NFC_HANDLER;

typedef void (*ppCallBck_t)(UINT32 TimerId, void *pContext);

/***************************************************************************** 
 * Enum
 *****************************************************************************/
typedef enum
{
    MTK_NFC_MUTEX_MAIN = 0x0,
    MTK_MUTEX_MSG_Q,
    MTK_MUTEX_MSG_CNT,
    MTK_MUTEX_SERVICE_MSG_Q,
    MTK_MUTEX_SOCKET_MSG_Q,
    MTK_NFC_MUTEX_TIMER,
    MTK_NFC_MUTEX_RESERVE_1,
    MTK_NFC_MUTEX_RESERVE_2,     
    MTK_NFC_MUTEX_MAX_NUM
} MTK_NFC_MUTEX_E;


typedef enum
{
  MTK_NFC_EVENT_2SERV = 0,
  MTK_NFC_EVENT_2MAIN,
  MTK_NFC_EVENT_2SOCKET, 
  MTK_NFC_EVENT_END
} MTK_NFC_EVENT_E;


typedef enum 
{
    MTK_NFC_TASKID_MAIN = 0x0,
    MTK_NFC_TASKID_SERVICE,
    MTK_NFC_TASKID_SOCKET,  // sned to Socket 
    MTK_NFC_TASKID_RXHDLR,
    MTK_NFC_TASKID_ADAPT,   // handle msg from APP
    MTK_NFC_TASKID_CLIENT,  // handle msg from NFC MW
    MTK_NFC_TASKID_APP,    
    MTK_NFC_TASKID_RESERVE_END
} MTK_NFC_TASKID_E;

typedef enum 
{
    MTK_NFC_TIMER_BOOT = 0x0,
    MTK_NFC_TIMER_SHDLC_T1,
    MTK_NFC_TIMER_SHDLC_T2_0,
    MTK_NFC_TIMER_SHDLC_T2_1,
    MTK_NFC_TIMER_SHDLC_T2_2,
    MTK_NFC_TIMER_SHDLC_T2_3,
    MTK_NFC_TIMER_SHDLC_T3,
    MTK_NFC_TIMER_NCI_CMD,
    MTK_NFC_TIMER_NCI_DATA,
    MTK_NFC_TIMER_LLCP_SYMM,     
    MTK_NFC_TIMER_CHECK_PRESENCE,     
    MTK_NFC_TIMER_RESERVE_0,    
    MTK_NFC_TIMER_RESERVE_1,
    MTK_NFC_TIMER_LLCP_RETRY,
    MTK_NFC_TIMER_DEP_WATCHDOG,
    MTK_NFC_TIMER_RECONN,
    MTK_NFC_TIMER_MAX_NUM   
} MTK_NFC_TIMER_E;

typedef enum 
{
    MTK_NFC_POWER_STATE_INIT = 0x0,
    MTK_NFC_POWER_STATE_ON,
    MTK_NFC_POWER_STATE_OFF,
    MTK_NFC_POWER_STATE_RESET,    
    MTK_NFC_POWER_STATE_SUSPEND,
    MTK_NFC_POWER_STATE_STANDBY,    
    MTK_NFC_POWER_STATE_MAX_NUM
} MTK_NFC_POWER_STATE_E;

typedef enum 
{
    MTK_NFC_GPIO_EN_B = 0x0,
    MTK_NFC_GPIO_SYSRST_B,
    MTK_NFC_GPIO_EINT,
    MTK_NFC_GPIO_IRQ,
    MTK_NFC_GPIO_MAX_NUM
} MTK_NFC_GPIO_E;

typedef enum 
{
    MTK_NFC_PULL_LOW  = 0x0,
    MTK_NFC_PULL_HIGH,
    MTK_NFC_PULL_INVALID,
} MTK_NFC_PULL_E;

typedef enum
{
    MTK_NFC_IOCTL_READ = 0x0,
    MTK_NFC_IOCTL_WRITE,
    MTK_NFC_IOCTL_MAX_NUM
} MTK_NFC_IOCTL_E;

typedef struct 
{
    UINT32 pin;
    UINT32 highlow;
} MTK_NFC_IOCTL_ARG_T;

typedef enum
{
    MTK_NFC_FS_READ = 0,     // open file for reading (r)
    MTK_NFC_FS_WRITE,        // create file for writing, discard previous contents if any (w)
    MTK_NFC_FS_APPEND,       // open or create file for writing at end of file (a)
    MTK_NFC_FS_RW,           // open file for reading and writing (r+)
    MTK_NFC_FS_RW_DISCARD,   // create file for reading and writing, discard previous contents if any (w+)
    MTK_NFC_FS_RW_APPEND     // open or create file for reading and writing at end of file (a+)
}   MTK_NFC_FMODE;

typedef enum
{
    MTK_NFC_FS_SEEK_SET = 0, // seek from beginning of file
    MTK_NFC_FS_SEEK_CUR,     // seek from current position
    MTK_NFC_FS_SEEK_END      // seek from end of file
} MTK_NFC_FSEEK;

/***************************************************************************** 
 * Data Structure
 *****************************************************************************/
typedef struct 
{
    UINT32 type;
    UINT32 length;
} MTK_NFC_MSG_T;

typedef struct
{
  UINT16    year;           /* years since 1900 */
  UINT8     month;          /* 0-11 */
  UINT8     mday;           /* 1-31 */
  UINT8     hour;           /* 0-23 */
  UINT8     min;            /* 0-59 */
  UINT8     sec;            /* 0-59 */
  UINT8     pad1;
  UINT16    msec;           /* 0-999 */
  UINT16    pad2;
} MTK_TIME_T;

typedef struct      // Ring buffer
{
    MTK_NFC_MSG_T** next_write;     // next position to write to
    MTK_NFC_MSG_T** next_read;      // next position to read from
    MTK_NFC_MSG_T** start_buffer;   // start of buffer
    MTK_NFC_MSG_T** end_buffer;     // end of buffer + 1
} MTK_NFC_MSG_RING_BUF;


typedef struct MTK_NFC_LINK_PARAM
{
    UINT16  miu;    /* Maximum Information Unit*/
    UINT16  wks;    /* Well-Known Services*/
    UINT8   lto;    /* Link TimeOut (in 10ms)*/
    UINT8   option; /* Options*/
} MTK_NFC_LINK_PARAM;

typedef struct MTK_NFC_ATR_INFO 
{
    UINT8  atr_buf[MTK_NFC_MAX_ATR_LENGTH];  /* ATR Info */
    UINT8  atr_buf_len;
} MTK_NFC_ATR_INFO;


typedef struct MTK_NFC_BUF_OPTION
{
   UINT16 miu; /* MIU*/
   UINT8  rw;    /* Receive Window size (4 bits)*/
}MTK_NFC_BUF_OPTION;


typedef struct MTK_NFC_DATA
{
    UINT32  length;
    UINT8  buffer[1];     
} MTK_NFC_DATA;


typedef enum MTK_NFC_LLCP_CONNECT_TYPE
{
   MTK_NFC_LLCP_CONNECTION_DEFAULT,
   MTK_NFC_LLCP_CONNECTION_LESS,
   MTK_NFC_LLCP_CONNECTION_ORIENTED
}MTK_NFC_LLCP_CONNECT_TYPE;


typedef enum MTK_NFC_LLCP_SERVICE_ROLE
{
   MTK_NFC_LLCP_SERVICEROLE_DEFAULT,
   MTK_NFC_LLCP_SERVICEROLE_CLIENT,
   MTK_NFC_LLCP_SERVICEROLE_SERVER
}MTK_NFC_LLCP_SERVICE_ROLE;


typedef struct MTK_NFC_CALLBACK_DATA
{
   NFCSTATUS status;
   VOID* pContext;
} MTK_NFC_CALLBACK_DATA;

typedef struct MTK_NFC_LLCP_CREATE_SERVICE
{
    MTK_NFC_BUF_OPTION buffer_options;
    MTK_NFC_LLCP_CONNECT_TYPE connection_type;
    UINT8 sap;
    MTK_NFC_DATA llcp_sn;
} MTK_NFC_LLCP_CREATE_SERVICE;


typedef struct MTK_NFC_LLCP_RSP_SERVICE
{
    INT32  ret;
    MTK_NFC_HANDLER llcp_handle;
} MTK_NFC_LLCP_RSP_SERVICE;


typedef struct MTK_NFC_LLCP_ACCEPT_SERVICE
{
    MTK_NFC_BUF_OPTION buffer_options;
    MTK_NFC_HANDLER incoming_handle;
    UINT32 remote_device_handle;
} MTK_NFC_LLCP_ACCEPT_SERVICE;


typedef struct MTK_NFC_LLCP_CONN_SERVICE
{
    UINT8 sap;
    MTK_NFC_HANDLER client_handle;
    UINT32 remote_device_handle;
    MTK_NFC_DATA llcp_sn;
} MTK_NFC_LLCP_CONN_SERVICE;

typedef struct MTK_NFC_LLCP_SOKCET
{
    UINT32 remote_dev_handle;
    MTK_NFC_HANDLER llcp_service_handle;
}MTK_NFC_LLCP_SOKCET;


typedef struct MTK_NFC_GET_REM_SOCKET_RSP
{
    INT32  ret;
    MTK_NFC_HANDLER service_handle;    
    MTK_NFC_BUF_OPTION buffer_options;
}MTK_NFC_GET_REM_SOCKET_RSP;

typedef struct MTK_NFC_LLCP_SEND_DATA
{
    UINT32 remote_dev_handle;
    MTK_NFC_HANDLER service_handle;
    UINT8 sap;
    MTK_NFC_DATA llcp_data_send_buf;    
}MTK_NFC_LLCP_SEND_DATA;


typedef struct MTK_NFC_LISTEN_SERVICE_NTF
{
    INT32  ret;
    MTK_NFC_HANDLER incoming_handle;
    MTK_NFC_HANDLER service_handle;
    UINT32 remote_miu;    
}MTK_NFC_LISTEN_SERVICE_NTF;


typedef struct MTK_NFC_CONNECTION_NTF
{
    INT32  ret;
    UINT8  errcode;
    UINT32 remote_miu;    
}MTK_NFC_CONNECTION_NTF;


typedef struct MTK_NFC_RECVDATA_RSP
{
    INT32 ret;
    MTK_NFC_HANDLER service_handle;    
    UINT8 sap;
    MTK_NFC_DATA llcp_data_recv_buf;
}MTK_NFC_RECVDATA_RSP;

typedef struct MTK_NFC_LLCP_RSP
{
    INT32 ret;
    MTK_NFC_HANDLER service_handle;
}MTK_NFC_LLCP_RSP;

typedef struct MTK_NFC_P2P_TRANSCEIVE_DATA
{
    INT32 ret;
    MTK_NFC_DATA data_buf;    
}MTK_NFC_P2P_TRANSCEIVE_DATA;

typedef struct MTK_NFC_LLCP_SERVICE
{
    MTK_NFC_HANDLER handler;        
    UINT8 type;
    UINT8 sap;
    MTK_NFC_BUF_OPTION buf_opt;
    MTK_NFC_LLCP_CONNECT_TYPE conn_type;
    UINT8 snlth;
    UINT8 sn[64];
}MTK_NFC_LLCP_SERVICE;

/***************************************************************************** 
 * Extern Area
 *****************************************************************************/ 

/***************************************************************************** 
 * Function Prototypes
 *****************************************************************************/
// - Memory
VOID *mtk_nfc_sys_mem_alloc (UINT32 u4Size);
VOID mtk_nfc_sys_mem_free (VOID *pMem);

// - Synchronization (Semaphore or Mutex)
INT32 mtk_nfc_sys_mutex_initialize ();
INT32 mtk_nfc_sys_mutex_create (MTK_NFC_MUTEX_E mutex_id);
INT32 mtk_nfc_sys_mutex_take (MTK_NFC_MUTEX_E mutex_id);
INT32 mtk_nfc_sys_mutex_give (MTK_NFC_MUTEX_E mutex_id);
INT32 mtk_nfc_sys_mutex_destory (MTK_NFC_MUTEX_E mutex_id);

// - Task Communication
INT32 mtk_nfc_sys_msg_initialize ();
MTK_NFC_MSG_T *mtk_nfc_sys_msg_alloc (UINT16 u2Size);
INT32 mtk_nfc_sys_msg_send (MTK_NFC_TASKID_E task_id, const MTK_NFC_MSG_T *msg);
INT32 mtk_nfc_sys_msg_recv (MTK_NFC_TASKID_E task_id, MTK_NFC_MSG_T **msg);
VOID mtk_nfc_sys_msg_free (MTK_NFC_MSG_T *msg);

// - Debug
VOID mtk_nfc_sys_dbg_string (const CH *pString);
VOID mtk_nfc_sys_dbg_trace (UINT8 pData[], UINT32 u4Len);
VOID mtk_nfc_sys_dbg_trx_to_file(BOOL fgIsTx, UINT8 pData[], UINT32  u4Len);
VOID mtk_nfc_sys_dbg_to_file(const CH *data, ...);

// - Timer
INT32 mtk_nfc_sys_timer_init ();
UINT32 mtk_nfc_sys_timer_create (MTK_NFC_TIMER_E selected_timer);
VOID mtk_nfc_sys_timer_start (UINT32 timer_slot, UINT32 period, ppCallBck_t timer_expiry, VOID *arg);
VOID mtk_nfc_sys_timer_stop (UINT32 timer_slot);
VOID mtk_nfc_sys_timer_delete (UINT32 timer_slot);

// - Sleep Function
VOID mtk_nfc_sys_sleep (UINT32 u4MilliSeconds);

// - Assert Function
VOID mtk_nfc_sys_assert ( INT32 value );

// - Time Utitiles
INT32 mtk_nfc_sys_time_read (MTK_TIME_T* utctime);

// - Communication Interface
VOID *mtk_nfc_sys_interface_init   (const CH *strDevPort, const INT32 i4Baud);
INT32 mtk_nfc_sys_interface_read   (UINT8 *pBuffer, UINT16 nNbBytesToRead);
INT32 mtk_nfc_sys_interface_write  (UINT8 *pBuffer, UINT16 nNbBytesToWrite);
VOID  mtk_nfc_sys_interface_flush  (VOID *pLinkHandle);
VOID  mtk_nfc_sys_interface_uninit (VOID *pLinkHandle);

// - GPIO Interface
VOID mtk_nfc_sys_gpio_write(MTK_NFC_GPIO_E ePin, MTK_NFC_PULL_E eHighLow);
MTK_NFC_PULL_E mtk_nfc_sys_gpio_read(MTK_NFC_GPIO_E ePin);

// - nfc read input
INT32 mtk_nfc_data_input (const CH* pBuff, UINT32 u4Len);

// - nfc main proc
INT32 mtk_nfc_main_proc (MTK_NFC_MSG_T *prmsg);

// - nfc service proc
INT32 mtk_nfc_service_proc (UINT8* pBuf);

// - File 
NFC_FILE mtk_nfc_sys_file_open(const CHAR *szFileName, UINT32 i4Mode);
VOID mtk_nfc_sys_file_close (NFC_FILE hFile);
UINT32 mtk_nfc_sys_file_read (NFC_FILE hFile, void *DstBuf, UINT32 u4Length);
UINT32 mtk_nfc_sys_file_seek (NFC_FILE hFile, UINT32 u4OffSet, UINT32 u4Origin);
UINT32 mtk_nfc_sys_file_tell (NFC_FILE hFile);
VOID mtk_nfc_sys_file_rewind (NFC_FILE hFile);

// - Event
INT32 mtk_nfc_sys_event_delete(MTK_NFC_EVENT_E event_idx);
INT32 mtk_nfc_sys_event_create(MTK_NFC_EVENT_E event_idx);
INT32 mtk_nfc_sys_event_set(MTK_NFC_EVENT_E event_idx);
INT32 mtk_nfc_sys_event_wait(MTK_NFC_EVENT_E event_idx, MTK_NFC_MUTEX_E mutex_idx);


#ifdef __cplusplus
   }  /* extern "C" */
#endif

#endif
