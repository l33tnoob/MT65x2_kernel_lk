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

#ifndef __BT_DRV_IF_H__
#define __BT_DRV_IF_H__

typedef enum {
  BT_COLD_OP_GET_ADDR = 0,
  BT_HOT_OP_SET_FWASSERT,
  BT_AUDIO_OP_GET_CONFIG
} BT_OP;

/* Audio config related defination */
typedef enum {
  PCM = 0,         // PCM 4 pins interface
  I2S,             // I2S interface
  MERGE_INTERFACE, // PCM & I2S merge interface
  CVSD_REMOVAL     // SOC consys
} AUDIO_IF;

typedef enum {
  SYNC_8K = 0,
  SYNC_16K
} SYNC_CLK;        // DAIBT sample rate

typedef enum {
  SHORT_FRAME = 0,
  LONG_FRAME
} SYNC_FORMAT;     // DAIBT sync

typedef struct {
  AUDIO_IF           hw_if;
  SYNC_CLK           sample_rate;
  SYNC_FORMAT        sync_format;
  unsigned int       bit_len; // bit-length of sync frame in long frame sync
} AUDIO_CONFIG;

/* Information carring for all OPs (In/Out) */
typedef union {
  unsigned char      addr[7];
  struct {
    int              fd;
    int              reason;
  } assert;
  AUDIO_CONFIG       audio_conf;
} BT_INFO;

typedef struct {
  BT_OP              op;
  BT_INFO            param;
} BT_REQ;

typedef struct {
  unsigned char      status;
  BT_INFO            param;
} BT_RESULT;


int mtk_bt_enable(int flag, void *func_cb);
int mtk_bt_disable(int bt_fd);
int mtk_bt_write(int bt_fd, unsigned char *buffer, unsigned long len);
int mtk_bt_read(int bt_fd, unsigned char *buffer, unsigned long len);
void mtk_bt_op(BT_REQ req, BT_RESULT *result);

#endif