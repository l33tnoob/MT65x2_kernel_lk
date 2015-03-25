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

/*
 * (C) Copyright 2011
 * MediaTek <www.mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "sec_platform.h"
#include "sec_region.h"
#include "sec.h"

/******************************************************************************
 * MODULE
 ******************************************************************************/
#define MOD                         "SEC_REGION"

/******************************************************************************
 * DEBUG
 ******************************************************************************/
#define SMSG                        dbg_print

/******************************************************************************
 *  EXTERNAL VARIABLES
 ******************************************************************************/

/******************************************************************************
 *  SECURITY REGION CHECK
 ******************************************************************************/
void sec_region_check (U32 addr, U32 len)
{
    U32 ret = SEC_OK;   
    U32 tmp = addr + len;

    /* check if it does access AHB/APB register */
    if ((IO_PHYS != (addr & REGION_MASK)) || (IO_PHYS != (tmp & REGION_MASK))) {
        SMSG("[%s] 0x%x Not AHB/APB Address\n", MOD, addr);   
        ASSERT(0);
    }

    if (len >= REGION_BANK) {
        SMSG("[%s] Overflow\n",MOD);
        ASSERT(0);
    }    

#ifdef MTK_SECURITY_SW_SUPPORT
    /* check platform security region */
    if (SEC_OK != (ret = seclib_region_check(addr,len))) {
        SMSG("[%s] ERR '0x%x' ADDR: 0x%x, LEN: %d\n", MOD, ret, addr, len);
        ASSERT(0);
    }
#endif
}

/******************************************************************************
 *  DA REGION CHECK
 ******************************************************************************/
U32 da_region_check (U32 addr, U32 len)
{
    U32 ret = SEC_OK;

    if(DA_DOWNLOAD_LOC != addr)
    {
        ret = ERR_DA_INVALID_LOCATION;
        goto _exit;
    }

    if(DA_DOWNLOAD_MAX_SZ < len)
    {
        ret = ERR_DA_INVALID_LENGTH;
        goto _exit;        
    }

_exit:

    return ret;
}
