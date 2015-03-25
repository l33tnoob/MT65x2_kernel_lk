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

#include "addr_trans.h"

int virt_to_phys_addr(addr_trans_tbl_t *tbl, virt_addr_t *virt, phys_addr_t *phys)
{
    u32 i;
    u32 num = tbl->num;
    u64 base_addr = 0, boundary_addr = 0;
    addr_trans_info_t *info = tbl->info;

    if (!info) {
        phys->id   = -1;
        phys->addr = virt->addr;
        return 0;
    }

    for (i = 0; i < num; i++, info++) {
        if (info->len) {
            boundary_addr += info->len;
            if (boundary_addr < base_addr)
                break; /* overflow */
            if ((base_addr <= virt->addr) && (virt->addr < boundary_addr)) {
                phys->id   = info->id;
                phys->addr = virt->addr - base_addr;
                return 0;
            }
            base_addr += info->len;
        }
    }

    return -1;
}

int phys_to_virt_addr(addr_trans_tbl_t *tbl, phys_addr_t *phys, virt_addr_t *virt)
{
    u32 i;
    u32 num = tbl->num;
    u64 base_addr = 0;
    addr_trans_info_t *info = tbl->info;

    if (!info) {
        virt->addr = phys->addr;
        return 0;
    }

    for (i = 0; i < num; i++, info++) {
        if (info->id == phys->id && info->len && phys->addr <= info->len) {
            virt->addr = phys->addr + base_addr;
            return 0;            
        }
        base_addr += info->len;
    }

    return -1;
}


