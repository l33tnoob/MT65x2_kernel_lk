/******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2011
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
******************************************************************************/

//*****************************************************************************
// [File] MTK_NFC_Sys_FP.h : function pointers to fit SO lib porting on Android system
// [Version] v1.0
// [Revision Date] 2012-12-12
// [Author] LiangChi Huang, LiangChi.Huang@mediatek.com, 25609
// [Description]
//*****************************************************************************


#ifndef MTK_NFC_SYS_FP_H
#define MTK_NFC_SYS_FP_H

#ifdef __cplusplus
   extern "C" {
#endif
#include "mtk_nfc_sys_type.h"
#include "mtk_nfc_sys.h"



/***************************************************************************** 
 * For Share library
 *****************************************************************************/
#ifdef SUPPORT_SHARED_LIBRARY

// - Memory
typedef void* (*fpmtk_nfc_sys_mem_alloc) (UINT32 u4Size);
typedef void (*fpmtk_nfc_sys_mem_free) (VOID *pMem);
typedef INT32 (*fpmtk_nfc_sys_mutex_create )(MTK_NFC_MUTEX_E mutex_id);
typedef INT32 (*fpmtk_nfc_sys_mutex_take )(MTK_NFC_MUTEX_E mutex_id);
typedef INT32 (*fpmtk_nfc_sys_mutex_give) (MTK_NFC_MUTEX_E mutex_id);
typedef INT32 (*fpmtk_nfc_sys_mutex_destory) (MTK_NFC_MUTEX_E mutex_id);
// - Task Communication
typedef MTK_NFC_MSG_T *(*fpmtk_nfc_sys_msg_alloc) (UINT16 u2Size);
typedef INT32 (*fpmtk_nfc_sys_msg_send) (MTK_NFC_TASKID_E task_id, const MTK_NFC_MSG_T *msg);
typedef INT32 (*fpmtk_nfc_sys_msg_recv) (MTK_NFC_TASKID_E task_id, MTK_NFC_MSG_T **msg);
typedef VOID (*fpmtk_nfc_sys_msg_free) (MTK_NFC_MSG_T *msg);
// - Debug
typedef VOID (*fpmtk_nfc_sys_dbg_string) (const CH *pString);
typedef VOID (*fpmtk_nfc_sys_dbg_trace) (UINT8 pData[], UINT32 u4Len);
typedef VOID (*fpmtk_nfc_sys_dbg_trx_to_file)(BOOL fgIsTx, UINT8 pData[], UINT32  u4Len);
// - Timer
typedef UINT32 (*fpmtk_nfc_sys_timer_create) (MTK_NFC_TIMER_E selected_timer);
typedef VOID (*fpmtk_nfc_sys_timer_start) (UINT32 timer_slot, UINT32 period, ppCallBck_t timer_expiry, VOID *arg);
typedef VOID (*fpmtk_nfc_sys_timer_stop) (UINT32 timer_slot);
typedef VOID (*fpmtk_nfc_sys_timer_delete) (UINT32 timer_slot);
// - Sleep Function
typedef VOID (*fpmtk_nfc_sys_sleep) (UINT32 u4MilliSeconds);
// - Assert Function
typedef VOID (*fpmtk_nfc_sys_assert) ( INT32 value );
// - Communication Interface
typedef INT32 (*fpmtk_nfc_sys_interface_write)  (UINT8 *pBuffer, UINT16 nNbBytesToWrite);
typedef VOID  (*fpmtk_nfc_sys_interface_uninit) (VOID *pLinkHandle);
// - GPIO Interface
typedef VOID (*fpmtk_nfc_sys_gpio_write)(MTK_NFC_GPIO_E ePin, MTK_NFC_PULL_E eHighLow);
typedef MTK_NFC_PULL_E (*fpmtk_nfc_sys_gpio_read)(MTK_NFC_GPIO_E ePin);

typedef NFC_FILE (*fpmtk_nfc_sys_file_open) (const CHAR *szFileName, UINT32 i4Mode);
typedef VOID (*fpmtk_nfc_sys_file_close) (NFC_FILE hFile);
typedef UINT32 (*fpmtk_nfc_sys_file_read) (NFC_FILE hFile, void *DstBuf, UINT32 u4Length);
typedef UINT32 (*fpmtk_nfc_sys_file_seek) (NFC_FILE hFile, UINT32 u4OffSet, UINT32 u4Origin);
typedef UINT32 (*fpmtk_nfc_sys_file_tell) (NFC_FILE hFile);
typedef VOID (*fpmtk_nfc_sys_file_rewind) (NFC_FILE hFile);




typedef struct MTK_NFC_SYS_FUNCTION_POINT
{
   fpmtk_nfc_sys_mem_alloc             sys_mem_alloc;
   fpmtk_nfc_sys_mem_free              sys_mem_free;
   fpmtk_nfc_sys_mutex_create          sys_mutex_create;
   fpmtk_nfc_sys_mutex_take            sys_mutex_take;
   fpmtk_nfc_sys_mutex_give            sys_mutex_give;
   fpmtk_nfc_sys_mutex_destory         sys_mutex_destory;
   fpmtk_nfc_sys_msg_alloc             sys_msg_alloc;
   fpmtk_nfc_sys_msg_send              sys_msg_send;
   fpmtk_nfc_sys_msg_recv              sys_msg_recv;
   fpmtk_nfc_sys_msg_free              sys_msg_free;
   fpmtk_nfc_sys_dbg_string            sys_dbg_string;
   fpmtk_nfc_sys_dbg_trace             sys_dbg_trace;
   fpmtk_nfc_sys_dbg_trx_to_file       sys_dbg_trx_to_file;
   fpmtk_nfc_sys_timer_create          sys_timer_create;
   fpmtk_nfc_sys_timer_start           sys_timer_start;
   fpmtk_nfc_sys_timer_stop            sys_timer_stop;
   fpmtk_nfc_sys_timer_delete          sys_timer_delete;
   fpmtk_nfc_sys_sleep                 sys_sleep;
   fpmtk_nfc_sys_assert                sys_assert;
   fpmtk_nfc_sys_interface_write       sys_interface_write;
   fpmtk_nfc_sys_gpio_write            sys_gpio_write;
   fpmtk_nfc_sys_gpio_read             sys_gpio_read;
   fpmtk_nfc_sys_file_open             sys_file_open;
   fpmtk_nfc_sys_file_close            sys_file_close;
   fpmtk_nfc_sys_file_read             sys_file_read ;
   fpmtk_nfc_sys_file_seek             sys_file_seek;
   fpmtk_nfc_sys_file_tell             sys_file_tell;
   fpmtk_nfc_sys_file_rewind           sys_file_rewind;


}MTK_NFC_SYS_FUNCTION_POINT_T;


INT32 mtk_nfc_sys_function_registry (MTK_NFC_SYS_FUNCTION_POINT_T *fp_t);

#endif


#ifdef __cplusplus
   }
#endif

#endif /* MTK_MNL_SYS_FP_H */

