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
#define LOG_TAG "FMLIB_MT6620"

#ifdef __cplusplus
extern "C" {
#endif

int MT6620_open_dev(const char *pname, int *fd)
{
	int ret = 0;
	ret =  COM_open_dev(pname, fd);

	//TODO important!!!, check if it's valid FM chip here.

	return ret;
}

int MT6620_stop_scan(int fd)
{
	int ret = 0;
	if (fd < 0){
        LOGE("FM fd unavailable for stopscan\n");
		ret = ERR_INVALID_FD;
        goto out;
    }
    
	//becare for the impliment of lseek in FM driver char device driver
    if(lseek(fd, 0, SEEK_END) != 0){
    	LOGE("Stopscan failed\n");
		ret = -1;
        goto out;
    }else
        LOGD("Stopscan Success,[fd=%d]\n", fd);
out:
	return ret;   
}

void MT6620_init(struct fm_cbk_tbl *cbk_tbl)
{
	//Basic functions.
	cbk_tbl->open_dev = MT6620_open_dev;
	cbk_tbl->close_dev = COM_close_dev;
	cbk_tbl->pwr_up = COM_pwr_up;
	cbk_tbl->pwr_down = COM_pwr_down;
	cbk_tbl->seek = COM_seek;
	cbk_tbl->scan = COM_hw_scan;	//hw_scan or sw_scan.
        cbk_tbl->scan_new = COM_hw_scan_new;
        cbk_tbl->seek_new = COM_seek_new;
        cbk_tbl->tune_new = COM_tune_new;
	cbk_tbl->fastget_rssi = COM_fastget_rssi;
	cbk_tbl->stop_scan = MT6620_stop_scan;
	cbk_tbl->tune = COM_tune;
	cbk_tbl->set_mute = COM_set_mute;
	cbk_tbl->is_fm_pwrup = COM_is_fm_pwrup;
	cbk_tbl->is_rdsrx_support = COM_is_rdsrx_support;
	cbk_tbl->is_rdstx_support = COM_is_rdstx_support;
	cbk_tbl->turn_on_off_rds = COM_turn_on_off_rds;
	cbk_tbl->get_chip_id = COM_get_chip_id;
	//For RDS RX.
	cbk_tbl->read_rds_data = COM_read_rds_data;
	cbk_tbl->get_pi = COM_get_pi;
	cbk_tbl->get_ps = COM_get_ps;
	cbk_tbl->get_pty = COM_get_pty;
	cbk_tbl->get_rssi = COM_get_rssi;
	cbk_tbl->get_rt = COM_get_rt;
	cbk_tbl->active_af = COM_active_af;
	cbk_tbl->active_ta = COM_active_ta;
	cbk_tbl->deactive_ta = COM_deactive_ta;
	//FOR RDS TX.
	//TODO RDS TX functions.
	cbk_tbl->is_tx_support = COM_tx_support;
	cbk_tbl->tx_pwrup = COM_tx_pwrup;
	cbk_tbl->rdstx_onoff = COM_rdstx_onoff;
	cbk_tbl->tx_tune = COM_tx_tune;
	cbk_tbl->tx_scan = COM_tx_scan;
	cbk_tbl->rdstx = COM_rdstx;
	//FM over BT
	cbk_tbl->fm_over_bt = COM_fm_over_bt; 
	//FM short antenna
	cbk_tbl->ana_switch = COM_ana_switch;
	cbk_tbl->get_caparray = COM_get_caparray;
	//RX EM mode use
	cbk_tbl->get_badratio = COM_get_badratio;
	cbk_tbl->get_stereomono = COM_get_stereomono;
	cbk_tbl->set_stereomono = COM_set_stereomono;
	cbk_tbl->get_hw_info = COM_get_hw_info;
	
	cbk_tbl->soft_mute_tune = COM_Soft_Mute_Tune;
	cbk_tbl->is_dese_chan = COM_is_dese_chan;//COM_dese_chan_check;
    cbk_tbl->desense_check = COM_desense_check;
	cbk_tbl->pre_search = COM_pre_search;
	cbk_tbl->restore_search = COM_restore_search;
    //EM
    cbk_tbl->set_search_threshold = COM_set_search_threshold;
	return;
}

#ifdef __cplusplus
}
#endif

