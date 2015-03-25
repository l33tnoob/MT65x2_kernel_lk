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

void TGT_init(struct fm_cbk_tbl *cbk_tbl)
{
	//Basic functions.
	cbk_table->open_dev = TGT_open_dev;
	cbk_table->close_dev = TGT_close_dev;
	cbk_table->pwr_up = TGT_pwr_up;
	cbk_table->pwr_down = TGT_pwr_down;
	cbk_table->seek = TGT_seek;
	cbk_table->scan = TGT_hw_scan;	//hw_scan or sw_scan.
	cbk_table->stop_scan = TGT_stop_scan;
	cbk_table->tune = TGT_tune;
	cbk_table->set_mute = TGT_set_mute;
	cbk_table->is_fm_pwrup = TGT_is_fm_pwrup;
	cbk_table->is_rdsrx_support = TGT_is_rdsrx_support;
	cbk_table->is_rdstx_support = TGT_is_rdstx_support;
	cbk_table->turn_on_off_rds = TGT_turn_on_off_rds;
	cbk_table->get_chip_id = TGT_get_chip_id;
	//For RDS RX.
	cbk_table->read_rds_data = TGT_read_rds_data;
	cbk_table->get_pi = TGT_get_pi;
	cbk_table->get_ps = TGT_get_ps;
	cbk_table->get_pty = TGT_get_pty;
	cbk_table->get_rssi = TGT_get_rssi;
	cbk_table->get_rt = TGT_get_rt;
	cbk_table->active_af = TGT_active_af;
	cbk_table->active_ta = TGT_active_ta;
	cbk_table->deactive_ta = TGT_deactive_ta;
	//FOR RDS TX.
	//TODO RDS TX functions.
	return;
}

int TGT_open_dev(const char *pname, int *fd)
{
	int ret = 0;;
	ret =  COM_open_dev(pname, fd);
	//TODO important!!!, check if it's valid FM chip here.

	return ret;
}

int TGT_close_dev(int fd, int freq)
{
	return COM_close_dev(fd, freq);
}

int TGT_pwr_up(int fd)
{
	return COM_pwr_up(fd);
}

int TGT_pwr_down(int fd, int type)
{
	return COM_pwr_down(fd, type);
}

int TGT_get_chip_id(int fd, int *chipid)
{
	return COM_get_chip_id(fd, chipid);
}

int TGT_get_rssi(int fd, int *rssi)
{
	return COM_get_rssi(fd, rssi);
}

char *TGT_get_ps(int fd)
{
	return COM_get_ps(fd);
}

char *TGT_get_rt(int fd)
{
	return COM_get_rt(fd);
}

int TGT_get_pi(int fd)
{
	return COM_get_pi(fd);
}

int TGT_get_pty(int fd)
{
	return COM_get_pty(fd);
}

int TGT_tune(int fd, int freq, int band)
{
	return COM_tune(fd, freq, band);
}

int TGT_seek(int fd, int start_freq, int dir)
{
	return COM_seek(fd, start_freq, dir);
}

int TGT_set_mute(int fd, int mute)
{
	return COM_set_mute(fd, mute);
}

int TGT_is_fm_pwrup(int fd)
{
	return COM_is_fm_pwrup(fd);
}

int TGT_is_rdsrx_support(int fd)
{
	return COM_is_rdsrx_support(fd);
}

int TGT_is_rdstx_support(int fd)
{
	return COM_is_rdstx_support(fd);
}

int TGT_sw_scan(int fd, uint16_t *scan_tbl, int max_cnt, int band)
{
	return COM_sw_scan(fd, scan_tbl, max_cnt, band);
}

int TGT_hw_scan(int fd,  uint16_t *scan_tbl, int max_cnt, int band)
{
	return COM_hw_scan(fd, scan_tbl, max_cnt, band);
}

int TGT_stop_scan(int fd)
{
	return COM_stop_scan();
}

int TGT_turn_on_off_rds(int fd, int onoff)
{
	return COM_turn_on_off_rds(fd, onoff);
}

uint16_t TGT_read_rds_data(int fd)
{
	return COM_read_rds_data(fd);
}

int TGT_active_af(int fd)
{
	return COM_active_af(fd);
}

int TGT_active_ta(int fd)
{
	return COM_active_ta(fd);
}

int TGT_deactive_ta(int fd)
{
	return COM_deactive_ta(fd);
}