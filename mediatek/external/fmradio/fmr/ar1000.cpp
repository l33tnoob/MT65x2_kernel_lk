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

#include "fmr.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FMLIB_AR1000"

#ifdef __cplusplus
extern "C" {
#endif

static int AR1000_open_dev(const char *pname, int *fd)
{
	int ret;
	ret = COM_open_dev(pname, fd);

	//TODO important!!!, check if it's valid FM chip, hard code here.

	return ret;
}

static int AR1000_ana_switch(int fd, int antenna)
{
	return -ERR_UNSUPT_SHORTANA;
}

void AR1000_init(struct fm_cbk_tbl *cbk_tbl)
{
	//Basic functions.
	cbk_tbl->open_dev = AR1000_open_dev;
	cbk_tbl->close_dev = COM_close_dev;
	cbk_tbl->pwr_up = COM_pwr_up;
	cbk_tbl->pwr_down = COM_pwr_down;
	cbk_tbl->seek = COM_seek;
	cbk_tbl->scan = COM_sw_scan;	//hw_scan or sw_scan.
	cbk_tbl->stop_scan = COM_stop_scan;
	cbk_tbl->tune = COM_tune;
	cbk_tbl->set_mute = COM_set_mute;
	cbk_tbl->is_fm_pwrup = COM_is_fm_pwrup;
	cbk_tbl->is_rdsrx_support = COM_is_rdsrx_support;
	cbk_tbl->is_rdstx_support = COM_is_rdstx_support;
	cbk_tbl->turn_on_off_rds = COM_turn_on_off_rds;
	cbk_tbl->get_chip_id = COM_get_chip_id;
	cbk_tbl->ana_switch = AR1000_ana_switch;

	return;
}

#ifdef __cplusplus
}
#endif

