/*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2011. All rights reserved.
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

/******************************************************************************
*
* Filename:
* ---------
*     buffer.h
*/
#ifndef BUFFER_ADDR_H
#define BUFFER_ADDR_H

#include "platform.h"
#include "dram_buffer.h"


#define SEC_SECRO_BUFFER_START      sec_secro_buf
#define SEC_SECRO_BUFFER_LENGTH     DRAM_SEC_SECRO_BUFFER_LENGTH

#define SEC_WORKING_BUFFER_START    sec_working_buf
#define SEC_WORKING_BUFFER_LENGTH   DRAM_SEC_WORKING_BUFFER_LENGTH

#define SEC_UTIL_BUFFER_START       sec_util_buf
#define SEC_UTIL_BUFFER_LENGTH      DRAM_SEC_UTIL_BUFFER_LENGTH

/*SecLib.a use DRAM*/
#define SEC_LIB_HEAP_START          sec_lib_heap_buf
#define SEC_LIB_HEAP_LENGTH         DRAM_SEC_LIB_HEAP_LENGTH

/*For v3 verify check buffer */
#define SEC_IMG_BUFFER_START        sec_img_buf
#define SEC_IMG_BUFFER_LENGTH       DRAM_SEC_IMG_BUFFER_LENGTH

#define SEC_CHUNK_BUFFER_START      sec_chunk_buf
#define SEC_CHUNK_BUFFER_LENGTH     DRAM_SEC_CHUNK_BUFFER_LENGTH

/************************************/
/*preloader download DA use DRAM*/
#define DA_RAM_ADDR                 (CFG_DA_RAM_ADDR)
#define DA_RAM_LENGTH               (0x30000)
/*preloader validate DA use DRAM*/
#define DA_RAM_RELOCATE_ADDR        (CFG_DA_RAM_ADDR + DA_RAM_LENGTH)
#define DA_RAM_RELOCATE_LENGTH      (DA_RAM_LENGTH)

#if 0
extern u8 sec_secro_buf[SEC_SECRO_BUFFER_LENGTH];
extern u8 sec_working_buf[SEC_WORKING_BUFFER_LENGTH];
extern u8 sec_util_buf[SEC_UTIL_BUFFER_LENGTH];
extern u8 sec_lib_heap_buf[SEC_LIB_HEAP_LENGTH];
extern u8 sec_img_buf[SEC_IMG_BUFFER_LENGTH];
extern u8 sec_chunk_buf[SEC_CHUNK_BUFFER_LENGTH];
#endif

#define  sec_secro_buf    g_dram_buf->sec_secro_buf 
#define  sec_working_buf  g_sec_buf.sram_sec_working_buf
#define  sec_util_buf     g_dram_buf->sec_util_buf
#define  sec_lib_heap_buf g_dram_buf->sec_lib_heap_buf
#define  sec_img_buf      g_sec_buf.sram_sec_img_buf
#define  sec_chunk_buf    g_dram_buf->sec_chunk_buf
#endif



