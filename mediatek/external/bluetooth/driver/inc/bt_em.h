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

#ifndef __EM_BT_H__
#define __EM_BT_H__

#include <stdbool.h>
#ifndef BOOL
#define BOOL  bool
#endif

typedef enum {
  BT_CHIP_ID_MT6611 = 0,
  BT_CHIP_ID_MT6612,
  BT_CHIP_ID_MT6616,
  BT_CHIP_ID_MT6620,
  BT_CHIP_ID_MT6622,
  BT_CHIP_ID_MT6626,
  BT_CHIP_ID_MT6628
} BT_CHIP_ID;

typedef enum {
  BT_HW_ECO_UNKNOWN = 0,
  BT_HW_ECO_E1,
  BT_HW_ECO_E2,
  BT_HW_ECO_E3,
  BT_HW_ECO_E4,
  BT_HW_ECO_E5,
  BT_HW_ECO_E6,
  BT_HW_ECO_E7
} BT_HW_ECO;


#ifdef __cplusplus
extern "C"
{
#endif
BOOL EM_BT_init(void);
void EM_BT_deinit(void);
BOOL EM_BT_write(unsigned char *peer_buf, int peer_len);
BOOL EM_BT_read(unsigned char *peer_buf, int peer_len, int *piResultLen);
void EM_BT_polling_start(void);
void EM_BT_polling_stop(void);

void EM_BT_getChipInfo(BT_CHIP_ID *chip_id, BT_HW_ECO *eco_num);
void EM_BT_getPatchInfo(char *patch_id, unsigned long *patch_len);
#ifdef __cplusplus
}
#endif

#endif