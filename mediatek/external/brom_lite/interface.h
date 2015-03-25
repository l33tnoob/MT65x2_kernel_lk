/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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
*  interface.h
*
* Project:
* --------
*   Standalone Flash downloader sample code
*
* Description:
* ------------
*   This module contains port interface header
*
* Author:
* -------
*  Kevin Lim (mtk60022)
*
*==============================================================================
*           HISTORY
* Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*------------------------------------------------------------------------------
* $Revision$
* $Modtime$
* $Log$
*
* 05 03 2013 monster.huang
* [STP100006902]  [FlashTool] v5.1312/v5.1314 maintenance
* [BROM Lite]
* Support SF/SPI NAND download flow via compiler option.
* Support USB/UART download.
* Support arbitrary number of download items.
* Receive data with retry 1000 times per 1ms.
*
* 04 12 2013 monster.huang
* [STP100006902]  [FlashTool] v5.1312/v5.1314 maintenance
* [BROM LITE] Support combo memory and automatic generate eppParam.
* Fetch bmt address by acquiring linux partitions.
*
* 02 07 2013 stanley.song
* [STP100006748]  Update Brom Lite for SV5
* Update BROM Lite to support MT6255
 *
 * 10 01 2010 kevin.lim
 * [STP100004187]  Upload the BROM-lite source code on P4 server.
 * 10 01 2010 Kevin Lim
 * [STP100004187] BROM Lite v1.1037.2 release
*
*
*------------------------------------------------------------------------------
* Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*==============================================================================
*******************************************************************************/
#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <stdio.h>
#if defined(_MSC_VER)
    #include <windows.h>
#endif

//added by xiaohui 4-19
#define PIPE_BUF_SIZE 64

//
// GSM manipulation
//
void gsm_reset();


//
// Communication
//
typedef enum
{
    COM_STATUS_DONE = 0,
    COM_STATUS_READ_TIMEOUT,
    COM_STATUS_WRITE_TIMEOUT,
    COM_STATUS_ERROR
} COM_STATUS;

typedef enum
{
    FLOW_CONTROL_DISABLED = 0,
    FLOW_CONTROL_HARDWARE
} FLOW_CONTROL;

#if defined(_MSC_VER)
    typedef HANDLE COM_HANDLE;
    #define INVALID_COM_HANDLE INVALID_HANDLE_VALUE
#elif defined(__GNUC__)
    typedef void* COM_HANDLE;
    #define INVALID_COM_HANDLE (NULL)
#else
    #error Please provide COM_HANLDE definition for your own platform
    //typedef void* COM_HANDLE;
    //#define INVALID_COM_HANDLE (NULL)
#endif

COM_STATUS com_open(COM_HANDLE *com_handle, unsigned int baudrate);
COM_STATUS com_close(COM_HANDLE *com_handle);
COM_STATUS com_change_timeout(COM_HANDLE com_handle,
                              unsigned int read_timeout_in_ms,
                              unsigned int write_timeout_in_ms);
COM_STATUS com_enable_hardware_flow_control(COM_HANDLE com_handle);
COM_STATUS com_change_baudrate(COM_HANDLE com_handle, unsigned int baudrate);

COM_STATUS com_send_data(COM_HANDLE com_handle,
                         const unsigned char *data, unsigned int len);

COM_STATUS com_send_byte(COM_HANDLE com_handle, unsigned char data);
COM_STATUS com_send_word(COM_HANDLE com_handle, unsigned short data);
COM_STATUS com_send_dword(COM_HANDLE com_handle, unsigned int data);


COM_STATUS com_recv_data(COM_HANDLE com_handle,
                         unsigned char *data, unsigned int len);

COM_STATUS com_recv_data_chk_len(COM_HANDLE com_handle,
                         unsigned char *data, unsigned int len);

COM_STATUS com_recv_byte_without_retry(COM_HANDLE com_handle, unsigned char *data);
COM_STATUS com_recv_byte(COM_HANDLE com_handle, unsigned char *data);
COM_STATUS com_recv_word(COM_HANDLE com_handle, unsigned short *data);
COM_STATUS com_recv_dword(COM_HANDLE com_handle, unsigned int *data);


//
// Logging
//
void log_output(const char *format, ...);

//
// feedback information to recovery
//add by xiaohui 4-22
void log_feedback(const char *format,...);

//
// Misc.
//
#if !defined(__GNUC__)
void sleep(unsigned int ms);
#endif

#endif /* _INTERFACE_H */

