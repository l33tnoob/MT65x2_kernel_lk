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
*  bootrom_stage.h
*
* Project:
* --------
*   Standalone Flash downloader sample code
*
* Description:
* ------------
*   This module contains bootrom stage header
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
#ifndef _BOOTROM_STAGE_H
#define _BOOTROM_STAGE_H

#include "interface.h"
#include "_External/include/mtk_status.h"

struct image;
struct ExternalMemorySetting;
struct EPP_PARAM;


STATUS_E bootrom_connect(COM_HANDLE com_handle);
STATUS_E bootrom_disable_watchdog(COM_HANDLE com_handle);
STATUS_E bootrom_latch_powerkey(COM_HANDLE com_handle);
STATUS_E bootrom_SetReg_LockPowerKey(COM_HANDLE com_handle);
STATUS_E bootrom_SetRemap(COM_HANDLE com_handle);
STATUS_E bootrom_SendEPP(COM_HANDLE com_handle,
                const struct image *download_EPP, const struct ExternalMemorySetting *externalMemorySetting);
STATUS_E bootrom_send_download_agent(COM_HANDLE com_handle,
                                     const struct image *download_agent,int isUSB);
STATUS_E bootrom_jump_to_download_agent(COM_HANDLE com_handle,
                                        const struct image *download_agent);


//STATUS_E bootloader_disable_watchdog_MT6280(COM_HANDLE com_handle);
//STATUS_E bootloader_latch_powerkey_MT6280(COM_HANDLE com_handle);
//STATUS_E bootloader_GetBootLoaderVer(COM_HANDLE com_handle);
#endif  /* _BOOTROM_STAGE_H */

