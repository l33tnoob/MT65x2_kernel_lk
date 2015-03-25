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

/*******************************************************************
 * FM JNI core
 * return -1 if error occured. else return needed value. 
 * if return type is char *, return NULL if error occured.
 * Do NOT return value access paramater.
 *
 * FM JNI core should be independent from lower layer, that means
 * there should be no low layer dependent data struct in FM JNI core
 *
 * Naming rule: FMR_n(paramter Micro), FMR_v(functions), fmr_n(global param)
 * pfmr_n(global paramter pointer)
 *
 *******************************************************************/

#include "fmr.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FMLIB_CORE"

#define FMR_MAX_IDX 1

struct fmr_ds fmr_data;
struct fmr_ds *pfmr_data[FMR_MAX_IDX] = {0};
#define FMR_fd(idx) ((pfmr_data[idx])->fd)
#define FMR_err(idx) ((pfmr_data[idx])->err)
#define FMR_chip(idx) ((pfmr_data[idx])->cfg_data.chip)
#define FMR_low_band(idx) ((pfmr_data[idx])->cfg_data.low_band)
#define FMR_high_band(idx) ((pfmr_data[idx])->cfg_data.high_band)
#define FMR_seek_space(idx) ((pfmr_data[idx])->cfg_data.seek_space)
#define FMR_max_scan_num(idx) ((pfmr_data[idx])->cfg_data.max_scan_num)
#define FMR_cbk_tbl(idx) ((pfmr_data[idx])->tbl)
#define FMR_cust_hdler(idx) ((pfmr_data[idx])->custom_handler)
#define FMR_get_cfg(idx) ((pfmr_data[idx])->get_cfg)

int FMR_get_cfgs(int idx)
{
	int ret = -1;
	FMR_cust_hdler(idx) = NULL;
	FMR_get_cfg(idx) = NULL;

	FMR_cust_hdler(idx) = dlopen(CUST_LIB_NAME, RTLD_NOW);
	if (FMR_cust_hdler(idx) == NULL) {
		LOGE("%s failed, %s\n", __FUNCTION__, dlerror());
		//FMR_seterr(ERR_LD_LIB);
	} else {
		*(void **) (&FMR_get_cfg(idx)) = dlsym(FMR_cust_hdler(idx), "CUST_get_cfg");
		if (FMR_get_cfg(idx) == NULL) {
			LOGE("%s failed, %s\n", __FUNCTION__, dlerror());
			//FMR_seterr(ERR_FIND_CUST_FNUC);
		} else {
			LOGI("Go to run cust function\n");
			(*FMR_get_cfg(idx))(&(pfmr_data[idx]->cfg_data));
			LOGI("OK\n");
			ret = 0;
		}
		//dlclose(FMR_cust_hdler(idx));
		FMR_cust_hdler(idx) = NULL;
		FMR_get_cfg(idx) = NULL;
	}
	LOGI("%s succefully. chip: 0x%x, lband: %d, hband: %d, seek_space: %d, max_scan_num: %d\n", __FUNCTION__, FMR_chip(idx), FMR_low_band(idx), FMR_high_band(idx), FMR_seek_space(idx), FMR_max_scan_num(idx));

	return ret;
}

int FMR_chk_cfg_data(int idx)
{
	//TODO Need check? how to check?
	return 0;
}

static void sig_alarm(int sig)
{
	LOGI("+++Receive sig %d\n", sig);
	return;
}

int FMR_init()
{
	int idx;
	int ret = 0;
	//signal(4, sig_alarm);

	for (idx=0;idx<FMR_MAX_IDX;idx++) {
		if (pfmr_data[idx] == NULL) {
			break;
		}
	}
	LOGI("FMR idx = %d\n", idx);
	if (idx == FMR_MAX_IDX) {
		//FMR_seterr(ERR_NO_MORE_IDX);
		return -1;
	}

	/*The best way here is to call malloc to alloc mem for each idx,but
	I do not know where to release it, so use static global param instead*/
	pfmr_data[idx] = &fmr_data;
	memset(pfmr_data[idx], 0, sizeof(struct fmr_ds));

	if (FMR_get_cfgs(idx) < 0) {
		LOGI("FMR_get_cfgs failed\n");
		goto fail;
	}

	if (FMR_chk_cfg_data(idx) < 0) {
		LOGI("FMR_chk_cfg_data failed\n");
		goto fail;
	}

	switch (FMR_chip(idx)) {
		case FM_CHIP_AR1000:
			pfmr_data[idx]->init_handler = dlopen(AR1000_LIB_NAME, RTLD_NOW);
			if (pfmr_data[idx]->init_handler == NULL) {
				LOGE("%s init_handler error, %s\n", __func__, dlerror());
				//FMR_seterr(ERR_LD_LIB);
			} else{
				*(void **) (&pfmr_data[idx]->init_func) = dlsym(pfmr_data[idx]->init_handler, "AR1000_init");
				if (pfmr_data[idx]->init_func == NULL){
					LOGE("%s init_func error, %s\n", __func__, dlerror());
					//FMR_seterr(ERR_FIND_CUST_FNUC);
				}else{
					LOGI("Go to run init function\n");
					(*pfmr_data[idx]->init_func)(&(pfmr_data[idx]->tbl));
					LOGI("OK\n");
					ret = 0;
				}
			}
			//AR1000_init(&FMR_cbk_tbl(idx));
			break;
		case FM_CHIP_MT6616:
			pfmr_data[idx]->init_handler = dlopen(MT6616_LIB_NAME, RTLD_NOW);
			if (pfmr_data[idx]->init_handler == NULL) {
				LOGE("%s init_handler error, %s\n", __func__, dlerror());
				//FMR_seterr(ERR_LD_LIB);
			}else{
				*(void **) (&pfmr_data[idx]->init_func) = dlsym(pfmr_data[idx]->init_handler, "MT6616_init");
				if (pfmr_data[idx]->init_func == NULL){
					LOGE("%s init_func error, %s\n", __func__, dlerror());
					//FMR_seterr(ERR_FIND_CUST_FNUC);
				}else{
					LOGI("Go to run init function\n");
					(*pfmr_data[idx]->init_func)(&(pfmr_data[idx]->tbl));
					LOGI("OK\n");
					ret = 0;
				}
			}
			//MT6616_init(&FMR_cbk_tbl(idx));
			break;
		case FM_CHIP_MT6620:
			pfmr_data[idx]->init_handler = dlopen(MT6620_LIB_NAME, RTLD_NOW);
			LOGI("mt6620[init_handler=%p]\n", pfmr_data[idx]->init_handler);
			if (pfmr_data[idx]->init_handler == NULL){
				LOGE("%s init_handler error, %s\n", __func__, dlerror());
				//FMR_seterr(ERR_LD_LIB);
			}else{
				*(void **) (&pfmr_data[idx]->init_func) = dlsym(pfmr_data[idx]->init_handler, "MT6620_init");
				if (pfmr_data[idx]->init_func == NULL){
					LOGE("%s init_func error, %s\n", __func__, dlerror());
					//FMR_seterr(ERR_FIND_CUST_FNUC);
				}else{
					LOGI("Go to run 6620 init\n");
					(*pfmr_data[idx]->init_func)(&(pfmr_data[idx]->tbl));
					LOGI("OK\n");
					ret = 0;
				}
			}
			//MT6620_init(&FMR_cbk_tbl(idx));
			break;
        case FM_CHIP_MT6626:
			pfmr_data[idx]->init_handler = dlopen(MT6626_LIB_NAME, RTLD_NOW);
			if (pfmr_data[idx]->init_handler == NULL) {
				LOGE("%s init_handler error, %s\n", __func__, dlerror());
			}else{
				*(void **) (&pfmr_data[idx]->init_func) = dlsym(pfmr_data[idx]->init_handler, "MT6626_init");
				if (pfmr_data[idx]->init_func == NULL){
					LOGE("%s init_func error, %s\n", __func__, dlerror());
				}else{
					LOGI("Go to run init function\n");
					(*pfmr_data[idx]->init_func)(&(pfmr_data[idx]->tbl));
					LOGI("OK\n");
					ret = 0;
				}
			}
			break;
		case FM_CHIP_MT6628:
			pfmr_data[idx]->init_handler = dlopen(MT6628_LIB_NAME, RTLD_NOW);
			if (pfmr_data[idx]->init_handler == NULL){
				LOGE("%s init_handler error, %s\n", __func__, dlerror());
			}else{
				*(void **) (&pfmr_data[idx]->init_func) = dlsym(pfmr_data[idx]->init_handler, "MT6628_init");
				if (pfmr_data[idx]->init_func == NULL){
					LOGE("%s init_func error, %s\n", __func__, dlerror());
				}else{
					LOGI("Go to run init function\n");
					(*pfmr_data[idx]->init_func)(&(pfmr_data[idx]->tbl));
					LOGI("OK\n");
					ret = 0;
				}
			}
			break;
		case FM_CHIP_MT6627:
		{
			pfmr_data[idx]->init_handler = dlopen(MT6627_LIB_NAME, RTLD_NOW);
			if (pfmr_data[idx]->init_handler == NULL){
				LOGE("%s init_handler error, %s\n", __func__, dlerror());
			}else{
				*(void **) (&pfmr_data[idx]->init_func) = dlsym(pfmr_data[idx]->init_handler, "MT6627_init");
				if (pfmr_data[idx]->init_func == NULL){
					LOGE("%s init_func error, %s\n", __func__, dlerror());
				}else{
					LOGI("Go to run init function\n");
					(*pfmr_data[idx]->init_func)(&(pfmr_data[idx]->tbl));
					LOGI("OK\n");
					ret = 0;
				}
			}
			break;
		}
		case FM_CHIP_MT6630:
		{
			pfmr_data[idx]->init_handler = dlopen(MT6630_LIB_NAME, RTLD_NOW);
			if (pfmr_data[idx]->init_handler == NULL){
				LOGE("%s init_handler error, %s\n", __func__, dlerror());
			}else{
				*(void **) (&pfmr_data[idx]->init_func) = dlsym(pfmr_data[idx]->init_handler, "MT6630_init");
				if (pfmr_data[idx]->init_func == NULL){
					LOGE("%s init_func error, %s\n", __func__, dlerror());
				}else{
					LOGI("Go to run init function\n");
					(*pfmr_data[idx]->init_func)(&(pfmr_data[idx]->tbl));
					LOGI("OK\n");
					ret = 0;
				}
			}
			break;
		}
		case FM_CHIP_MT5192:
		case FM_CHIP_MT5193:
			pfmr_data[idx]->init_handler = dlopen(MT519X_LIB_NAME, RTLD_NOW);
			if (pfmr_data[idx]->init_handler == NULL){
				LOGE("%s init_handler error, %s\n", __func__, dlerror());
				//FMR_seterr(ERR_LD_LIB);
			}else{
				*(void **) (&pfmr_data[idx]->init_func) = dlsym(pfmr_data[idx]->init_handler, "MT519X_init");
				if (pfmr_data[idx]->init_func == NULL){
					LOGE("%s init_func error, %s\n", __func__, dlerror());
					//FMR_seterr(ERR_FIND_CUST_FNUC);
				}else{
					LOGI("Go to run init function\n");
					(*pfmr_data[idx]->init_func)(&(pfmr_data[idx]->tbl));
					LOGI("OK\n");
					ret = 0;
				}
			}
			//MT5192_init(&FMR_cbk_tbl(idx));
			break;
		default:
			//FMR_seterr(ERR_UNSUPPORT_CHIP);
			LOGI("Unsupported Chip: 0x%x", FMR_chip(idx));
			goto fail;
	}
	return idx;

fail:
	pfmr_data[idx] = NULL;

	return -1;
}

int FMR_open_dev(int idx)
{
	int ret = 0;
	int real_chip;

	FMR_ASSERT(FMR_cbk_tbl(idx).open_dev);

	ret = FMR_cbk_tbl(idx).open_dev(FM_DEV_NAME, &FMR_fd(idx));
	if (ret || FMR_fd(idx) < 0) {
		LOGE("%s failed, [fd=%d]\n", __func__, FMR_fd(idx));
		return ret;
	}

	//Check if customer's cfg matchs driver.
	ret = FMR_get_chip_id(idx, &real_chip);
	if (FMR_chip(idx) != real_chip) {
		LOGE("%s, Chip config error. 0x%x\n", __func__, real_chip);
		ret = FMR_cbk_tbl(idx).close_dev(FMR_fd(idx));
		return ret;
	}
	
	LOGD("%s, [fd=%d] [chipid=0x%x] [ret=%d]\n", __func__, FMR_fd(idx), real_chip, ret);
	return ret;
}

int FMR_close_dev(int idx)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).close_dev);
	ret = FMR_cbk_tbl(idx).close_dev(FMR_fd(idx));
	LOGD("%s, [fd=%d] [ret=%d]\n", __func__, FMR_fd(idx), ret);
	return ret;
}

int FMR_pwr_up(int idx, int freq)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).pwr_up);

	LOGI("%s,[freq=%d]\n", __func__, freq);
	if (freq < fmr_data.cfg_data.low_band || freq > fmr_data.cfg_data.high_band) {
		LOGE("%s error freq: %d\n", __func__, freq);
		ret = -ERR_INVALID_PARA;
		return ret;
	}
	ret = FMR_cbk_tbl(idx).pwr_up(FMR_fd(idx), fmr_data.cfg_data.band, freq);
	if (ret) {
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}
	fmr_data.cur_freq = freq;
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_pwr_down(int idx, int type)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).pwr_down);
	ret = FMR_cbk_tbl(idx).pwr_down(FMR_fd(idx), type);
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_get_chip_id(int idx, int *chipid)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).get_chip_id);
	FMR_ASSERT(chipid);
	
	ret = FMR_cbk_tbl(idx).get_chip_id(FMR_fd(idx), chipid);
	if (ret){
		LOGE("%s failed, %s\n", __func__, FMR_strerr());
		*chipid = -1;
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_get_rssi(int idx, int *rssi)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).get_rssi);
	FMR_ASSERT(rssi);
	
	ret = FMR_cbk_tbl(idx).get_rssi(FMR_fd(idx), rssi);
	if (ret){
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
		*rssi = -1;
	}
	LOGD("%s, [rssi=%d] [ret=%d]\n", __func__, *rssi, ret);
	return ret;
}

int FMR_get_ps(int idx, uint8_t **ps, int *ps_len)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).get_ps);
	FMR_ASSERT(ps);
	FMR_ASSERT(ps_len);
	ret = FMR_cbk_tbl(idx).get_ps(FMR_fd(idx), &fmr_data.rds, ps, ps_len);
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_get_rt(int idx, uint8_t **rt, int *rt_len)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).get_rt);
	FMR_ASSERT(rt);
	FMR_ASSERT(rt_len);

	ret = FMR_cbk_tbl(idx).get_rt(FMR_fd(idx), &fmr_data.rds, rt, rt_len);
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_get_pi(int idx, uint16_t *pi)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).get_pi);

	ret = FMR_cbk_tbl(idx).get_pi(FMR_fd(idx), &fmr_data.rds, pi);
	if (ret){
		LOGE("%s failed, %s\n", __func__, FMR_strerr());
		*pi = -1;
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_get_pty(int idx, uint8_t *pty)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).get_pty);

	ret = FMR_cbk_tbl(idx).get_pty(FMR_fd(idx), &fmr_data.rds, pty);
	if (ret){
		*pty = -1;
		LOGI("%s failed, %s\n", __func__, FMR_strerr());
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_tune(int idx, int freq)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).tune);

	ret = FMR_cbk_tbl(idx).tune(FMR_fd(idx), freq, fmr_data.cfg_data.band);
	if (ret){
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}
	fmr_data.cur_freq = freq;
	LOGD("%s, [freq=%d] [ret=%d]\n", __func__, freq, ret);
	return ret;
}

#if FMR_SOFT_MUTE_TUEN_SCAN
/*return: fm_true: desense, fm_false: not desene channel*/
fm_bool FMR_DensenseDetect(fm_s32 idx,fm_u16 ChannelNo,fm_s32 RSSI)
{
	fm_u8 bDesenseCh = 0;

	bDesenseCh = FMR_cbk_tbl(idx).desense_check(FMR_fd(idx),ChannelNo,RSSI); 
	if(bDesenseCh == 1)
	{
		return fm_true;
	}
	return fm_false;
}
fm_bool FMR_SevereDensense(fm_u16 ChannelNo,fm_s32 RSSI)
{
	fm_s32 i=0,j=0;
    struct fm_fake_channel_t *chan_info = fmr_data.cfg_data.fake_chan;
    
#ifndef MTK_FM_50KHZ_SUPPORT
	ChannelNo /= 10;
#endif
	for(i=0; i<chan_info->size; i++)
	{
		if(ChannelNo == chan_info->chan[i].freq)
		{
			//if(RSSI < FM_SEVERE_RSSI_TH)
			if(RSSI < chan_info->chan[i].rssi_th)
			{
				LOGI(" SevereDensense[%d] RSSI[%d]\n", ChannelNo,RSSI);
				return fm_true;
			}
			else
			{
				break;
			}
		}
	}
    return fm_false;
}
#if (FMR_NOISE_FLOORT_DETECT==1)
/*return TRUE:get noise floor freq*/
fm_bool FMR_NoiseFloorDetect(fm_bool *rF,fm_s32 rssi,fm_s32 *F_rssi)
{
	if(rF[0]==fm_true)
	{
		if(rF[1]==fm_true)
		{
			F_rssi[2]=rssi;
			rF[2]=fm_true;
			return fm_true;
		}
		else
		{
			F_rssi[1]=rssi;
			rF[1]=fm_true;
		}
	}
	else
	{
		F_rssi[0]=rssi;
		rF[0]=fm_true;
	}
	return fm_false;
}
#endif

/*check the cur_freq->freq is valid or not
return fm_true : need check cur_freq->valid
         fm_false: check faild, should stop seek
*/
static fm_bool FMR_Seek_TuneCheck(int idx,fm_softmute_tune_t *cur_freq)
{
	int ret=0;
	if(fmr_data.scan_stop == fm_true)
	{
		ret = FMR_tune(idx,fmr_data.cur_freq);
		LOGI("seek stop!!! tune ret=%d",ret);
		return fm_false;
	}
	ret = FMR_cbk_tbl(idx).soft_mute_tune(FMR_fd(idx),cur_freq);
	if(ret)
	{
		LOGE("soft mute tune, failed:[%d]\n",ret);
		cur_freq->valid = fm_false;
		return fm_true;
	}
	if(cur_freq->valid == fm_true)/*get valid channel*/
	{
		if(FMR_DensenseDetect(idx,cur_freq->freq,cur_freq->rssi)==fm_true)
		{
			LOGI("desense channel detected:[%d] \n", cur_freq->freq);
			cur_freq->valid = fm_false;
			return fm_true;
		}
		if(FMR_SevereDensense(cur_freq->freq,cur_freq->rssi)==fm_true)
		{
			LOGI("sever desense channel detected:[%d] \n", cur_freq->freq);
			cur_freq->valid = fm_false;
			return fm_true;
		}
		LOGI("seek result freq:[%d] \n", cur_freq->freq);
	}
	return fm_true;
}
/*
check more 2 freq, curfreq: current freq, seek_dir: 1,forward. 0,backword
*/
static int FMR_Seek_More(int idx,fm_softmute_tune_t *validfreq,fm_u8 seek_dir,fm_u8 step,fm_u16 min_freq,fm_u16 max_freq)
{
	fm_s32 i;
	fm_softmute_tune_t cur_freq;
	cur_freq.freq = validfreq->freq;

	for(i=0; i<2; i++)
	{
		if(seek_dir)//forward
		{
			if(cur_freq.freq + step > max_freq)
			{
				return 0;
			}
			cur_freq.freq += step;
		}
		else//backward
		{
			if(cur_freq.freq - step < min_freq)
			{
				return 0;
			}
			cur_freq.freq -= step;
		}
		if(FMR_Seek_TuneCheck(idx,&cur_freq) == fm_true)
		{
			if(cur_freq.valid == fm_true)
			{
				if(cur_freq.rssi > validfreq->rssi)
				{
					validfreq->freq = cur_freq.freq;
					validfreq->rssi = cur_freq.rssi;
					LOGI("seek cover last by freq=%d",cur_freq.freq);
				}
			}
		}
		else
		{
			return -1;
		}
	}
	return 0;
}

/*check the a valid channel
return -1 : seek fail
         0: seek success
*/
int FMR_seek_Channel(int idx, int start_freq, int min_freq, int max_freq, int band_channel_no, int seek_space, int dir, int *ret_freq, int *rssi_tmp)
{
	fm_s32 i, ret = 0;
	fm_softmute_tune_t cur_freq;
	
	if(dir==1)/*forward*/
	{
		for (i=((start_freq-min_freq)/seek_space+1); i<band_channel_no; i++)
		{
			cur_freq.freq = min_freq + seek_space*i;
			LOGI("i=%d, freq=%d-----1",i,cur_freq.freq);
			ret = FMR_Seek_TuneCheck(idx,&cur_freq);
			if(ret == fm_false)
			{
				return -1;
			}
			else
			{
				if(cur_freq.valid == fm_false)
				{
					continue;
				}
				else
				{
					if(FMR_Seek_More(idx,&cur_freq,dir,seek_space,min_freq,max_freq) == 0)
					{
						*ret_freq = cur_freq.freq;
						*rssi_tmp = cur_freq.rssi;
						return 0;
					}
					else
					{
						return -1;
					}
				}
			}
		}
		for (i=0; i<((start_freq-min_freq)/seek_space); i++)
		{
			cur_freq.freq = min_freq + seek_space*i;
			LOGI("i=%d, freq=%d-----2",i,cur_freq.freq);
			ret = FMR_Seek_TuneCheck(idx,&cur_freq);
			if(ret == fm_false)
			{
				return -1;
			}
			else
			{
				if(cur_freq.valid == fm_false)
				{
					continue;
				}
				else
				{
					if(FMR_Seek_More(idx,&cur_freq,dir,seek_space,min_freq,max_freq) == 0)
					{
						*ret_freq = cur_freq.freq;
						*rssi_tmp = cur_freq.rssi;
						return 0;
					}
					else
					{
						return -1;
					}
				}
			}
		}
	}
	else/*backward*/
	{
		for (i=((start_freq-min_freq)/seek_space-1); i>=0; i--)
		{
			cur_freq.freq = min_freq + seek_space*i;
			LOGI("i=%d, freq=%d-----3",i,cur_freq.freq);
			ret = FMR_Seek_TuneCheck(idx,&cur_freq);
			if(ret == fm_false)
			{
				return -1;
			}
			else
			{
				if(cur_freq.valid == fm_false)
				{
					continue;
				}
				else
				{
					if(FMR_Seek_More(idx,&cur_freq,dir,seek_space,min_freq,max_freq) == 0)
					{
						*ret_freq = cur_freq.freq;
						*rssi_tmp = cur_freq.rssi;
						return 0;
					}
					else
					{
						return -1;
					}
				}
			}
		}	
		for (i=(band_channel_no-1); i>((start_freq-min_freq)/seek_space); i--)
		{
			cur_freq.freq = min_freq + seek_space*i;
			LOGI("i=%d, freq=%d-----4",i,cur_freq.freq);
			ret = FMR_Seek_TuneCheck(idx,&cur_freq);
			if(ret == fm_false)
			{
				return -1;
			}
			else
			{
				if(cur_freq.valid == fm_false)
				{
					continue;
				}
				else
				{
					if(FMR_Seek_More(idx,&cur_freq,dir,seek_space,min_freq,max_freq) == 0)
					{
						*ret_freq = cur_freq.freq;
						*rssi_tmp = cur_freq.rssi;
						return 0;
					}
					else
					{
						return -1;
					}
				}
			}
		}	
	}
	
	*ret_freq = start_freq;
	return 0;
}

int FMR_seek(int idx, int start_freq, int dir, int *ret_freq)
{
	fm_s32 ret=0,i,j;
	fm_softmute_tune_t cur_freq;
	fm_s32 band_channel_no=0;
	fm_u8 seek_space=10;
    fm_u16 min_freq,max_freq;
	int rssi;
    
    if ((start_freq < 7600) || (start_freq > 10800)) /*need replace by macro*/
	{
		LOGE("%s error start_freq: %d\n", __func__, start_freq);
		return -ERR_INVALID_PARA;
    }
	
	//FM radio seek space,5:50KHZ; 1:100KHZ; 2:200KHZ
	if(fmr_data.cfg_data.seek_space == 5)
	{
		seek_space=5;
	}
	else if (fmr_data.cfg_data.seek_space == 2)
	{
		seek_space=20;
	}
	else
	{
		seek_space=10;
	}
	if (fmr_data.cfg_data.band == FM_BAND_JAPAN)// Japan band	   76MHz   ~ 90MHz
	{
		band_channel_no = (9600-7600)/seek_space + 1;
		min_freq = 7600;
		max_freq = 9600;
	}
	else if (fmr_data.cfg_data.band == FM_BAND_JAPANW)// Japan wideband  76MHZ	 ~ 108MHz
	{
		band_channel_no = (10800-7600)/seek_space + 1;
		min_freq = 7600;
		max_freq = 10800;
	}
	else// US/Europe band  87.5MHz ~ 108MHz (DEFAULT)
	{
		band_channel_no = (10800-8750)/seek_space + 1;
		min_freq = 8750;
		max_freq = 10800;
	}

	fmr_data.scan_stop = fm_false;
    LOGD("seek start freq %d band_channel_no=[%d], seek_space=%d band[%d - %d] dir=%d\n", start_freq, band_channel_no,seek_space,min_freq,max_freq,dir);

	ret = FMR_seek_Channel(idx, start_freq, min_freq, max_freq, band_channel_no, seek_space, dir, ret_freq, &rssi);
	
	return ret;
}
#else
int FMR_seek(int idx, int start_freq, int dir, int *ret_freq)
{
    int ret = 0;
    int freq = start_freq;
    int rssi;
    struct fm_fake_channel_t *chan_info = fmr_data.cfg_data.fake_chan;
    struct fm_fake_channel *channle = NULL;
    int j;

    FMR_ASSERT(FMR_cbk_tbl(idx).seek);
    FMR_ASSERT(ret_freq);

    *ret_freq = 0;

    if ((start_freq < fmr_data.cfg_data.low_band) || (start_freq > fmr_data.cfg_data.high_band)) {
        LOGE("%s error start_freq: %d\n", __func__, start_freq);
        return -ERR_INVALID_PARA;
    }

redo:
    LOGD("freq %d\n", freq);
    ret = FMR_cbk_tbl(idx).seek(FMR_fd(idx),
                                &freq,
                                fmr_data.cfg_data.band,
                                dir,
                                fmr_data.cfg_data.seek_lev);

    if (ret < 0) {
        //Here we just need to return the start_freq.
        *ret_freq = start_freq;
        LOGE("%s failed, %s\n", __func__, FMR_strerr());
        goto out;
    }

    if ((FMR_cbk_tbl(idx).is_dese_chan) && (FMR_cbk_tbl(idx).is_dese_chan(FMR_fd(idx), freq))) {
        FMR_cbk_tbl(idx).get_rssi(FMR_fd(idx), &rssi);

        if (rssi < fmr_data.cfg_data.rssi_th_l2) {
            // an invalid channel
            LOGW("cur rssi %d < TH_L2, delete %d \n", rssi, freq);

            if (dir) {
                freq++;
                freq = (freq > fmr_data.cfg_data.high_band) ? fmr_data.cfg_data.low_band : freq;
            } else {
                freq--;
                freq = (freq < fmr_data.cfg_data.low_band) ? fmr_data.cfg_data.high_band : freq;
            }

            FMR_cbk_tbl(idx).tune(FMR_fd(idx), freq, fmr_data.cfg_data.band);
            LOGD("restart with %d\n", freq);
            goto redo;
        }
    }

    if (chan_info && chan_info->chan) {
        channle = chan_info->chan;

        for (j = 0; j < chan_info->size; j++) {
            if (freq == channle[j].freq) {
                FMR_cbk_tbl(idx).get_rssi(FMR_fd(idx), &rssi);

                if (rssi < channle[j].rssi_th) {
                    // an invalid channel
                    LOGW("cur rssi %d < TH_L3, delete %d \n", rssi, freq);

                    if (dir) {
                        freq++;
                        freq = (freq > fmr_data.cfg_data.high_band) ? fmr_data.cfg_data.low_band : freq;
                    } else {
                        freq--;
                        freq = (freq < fmr_data.cfg_data.low_band) ? fmr_data.cfg_data.high_band : freq;
                    }

                    FMR_cbk_tbl(idx).tune(FMR_fd(idx), freq, fmr_data.cfg_data.band);
                    LOGD("restart with %d\n", freq);
                    goto redo;
                }
            }
        }
        *ret_freq = freq;
        fmr_data.cur_freq = freq;
    }

out:
    LOGD("%s, [freq=%d] [ret=%d]\n", __func__, freq, ret);
    return ret;
}
#endif

int FMR_set_mute(int idx, int mute)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).set_mute)

	if ((mute < 0) || (mute > 1)){
		LOGE("%s error param mute:  %d\n", __func__, mute);
	}

	ret = FMR_cbk_tbl(idx).set_mute(FMR_fd(idx), mute);
	if (ret){
		LOGE("%s failed, %s\n", __func__, FMR_strerr());
	}
	LOGD("%s, [mute=%d] [ret=%d]\n", __func__, mute, ret);
	return ret;
}

int FMR_is_fm_pwrup(int idx, int *pwrup)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).is_fm_pwrup);
	FMR_ASSERT(pwrup);
	
	ret = FMR_cbk_tbl(idx).is_fm_pwrup(FMR_fd(idx), pwrup);
	if (ret){
		*pwrup = 0;
		LOGE("%s failed, %s\n", __func__, FMR_strerr());
	}
	
	LOGD("%s, [pwrup=%d] [ret=%d]\n", __func__, *pwrup, ret);
	return ret;
}

int FMR_is_rdsrx_support(int idx, int *supt)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).is_rdsrx_support);
	FMR_ASSERT(supt);

	ret = FMR_cbk_tbl(idx).is_rdsrx_support(FMR_fd(idx), supt);
	if (ret){
		*supt = 0;
		LOGE("%s, failed\n", __func__);
	}
	LOGD("%s, [supt=%d] [ret=%d]\n", __func__, *supt, ret);
	return ret;
}
int FMR_Pre_Search(int idx)
{
	//avoid scan stop flag clear if stop cmd send before pre-search finish
	fmr_data.scan_stop = fm_false;
#if FMR_SOFT_MUTE_TUEN_SCAN
	FMR_ASSERT(FMR_cbk_tbl(idx).pre_search);
	FMR_cbk_tbl(idx).pre_search(FMR_fd(idx));
	return 0;
#else
	return 0;
#endif
}
int FMR_Restore_Search(int idx)
{
#if FMR_SOFT_MUTE_TUEN_SCAN
	FMR_ASSERT(FMR_cbk_tbl(idx).restore_search);
	FMR_cbk_tbl(idx).restore_search(FMR_fd(idx));
	return 0;
#else
	return 0;
#endif
}

#if FMR_SOFT_MUTE_TUEN_SCAN
int FMR_scan_Channels(int idx, uint16_t *scan_tbl, int *max_cnt, fm_s32 band_channel_no, fm_u16 Start_Freq, fm_u8 seek_space, fm_u8 NF_Space)
{
	fm_s32 ret=0,Num=0,i,j;
	fm_u32 ChannelNo=0;
	fm_softmute_tune_t cur_freq;
	static struct fm_cqi SortData[CQI_CH_NUM_MAX];
	fm_bool LastExist=fm_false;
	struct fm_cqi swap;
#if (FMR_NOISE_FLOORT_DETECT==1)
	fm_s32 Pacc=0,Nacc=0;
	fm_s32 NF=0;
	fm_bool F[3]={fm_false,fm_false,fm_false};
	fm_s32 F_Rssi[3]={0};
	fm_u8 NF_Idx=0;
#endif

	memset (SortData, 0, CQI_CH_NUM_MAX*sizeof(struct fm_cqi));
	LOGI("band_channel_no=[%d], seek_space=%d, start freq=%d\n", band_channel_no,seek_space,Start_Freq);
	for(i=0; i<band_channel_no; i++)
	{
		if(fmr_data.scan_stop == fm_true)
		{
			FMR_Restore_Search(idx);
			ret = FMR_tune(idx,fmr_data.cur_freq);
	        LOGI("scan stop!!! tune ret=%d",ret);
			return -1;
		}
		cur_freq.freq = Start_Freq + seek_space*i;
		ret = FMR_cbk_tbl(idx).soft_mute_tune(FMR_fd(idx),&cur_freq);
	    if(ret)
		{
			LOGE("soft mute tune, failed:[%d]\n",ret);
			LastExist=fm_false;
	        continue;
	    }
		if(cur_freq.valid == fm_true)/*get valid channel*/
		{
#if (FMR_NOISE_FLOORT_DETECT==1)
			memset(F,fm_false,sizeof(F));
#endif
			if(FMR_DensenseDetect(idx,cur_freq.freq,cur_freq.rssi)==fm_true)
			{
				LOGI("desense channel detected:[%d] \n", cur_freq.freq);
				LastExist=fm_false;
				continue;
			}
			if((LastExist==fm_true)&&(Num>0)) /*neighbor channel*/
			{
				if(cur_freq.rssi>SortData[Num-1].rssi)/*save current freq and cover last channel*/
				{
					if(FMR_SevereDensense(cur_freq.freq,cur_freq.rssi)==fm_true)
					{
						LastExist=fm_false;
						continue;
					}
					SortData[Num-1].ch=cur_freq.freq;
					SortData[Num-1].rssi=cur_freq.rssi;
					SortData[Num-1].reserve = 1;
					LOGI("cover last channel \n");
				}
				else/*ignore current freq*/
				{
					LastExist=fm_false;
					continue;
				}
			}
			else/*save current*/
			{
				if(FMR_SevereDensense(cur_freq.freq,cur_freq.rssi)==fm_true)
				{
					LastExist=fm_false;
					continue;
				}
				SortData[Num].ch=cur_freq.freq;
				SortData[Num].rssi=cur_freq.rssi;
				SortData[Num].reserve = 1;
				Num++;
				LastExist=fm_true;
				LOGI("Num++:[%d] \n", Num);
			}
		}
		else
		{
#if (FMR_NOISE_FLOORT_DETECT==1)
			if(FMR_DensenseDetect(idx,cur_freq.freq,cur_freq.rssi)==fm_false)
			{
				if(FMR_NoiseFloorDetect(F,cur_freq.rssi,F_Rssi)==fm_true)
				{
					Pacc+=F_Rssi[1];
					Nacc++;
					/*check next freq*/
					F[0]=F[1];
					F_Rssi[0]=F_Rssi[1];
					F[1]=F[2];
					F_Rssi[1]=F_Rssi[2];
					F[2]=fm_false;
					F_Rssi[2]=0;
					LOGI("FM Noise FLoor:Pacc=[%d] Nacc=[%d] \n", Pacc,Nacc);
				}
			}
			else
			{
				memset(F,fm_false,sizeof(F));
			}
#endif			
			LastExist=fm_false;
		}	
#if (FMR_NOISE_FLOORT_DETECT==1)
		if(((i%NF_Space)==0)&&(i!=0))
		{
			if(Nacc>0)
			{
				NF=Pacc/Nacc;
			}
			else
			{
				NF=RSSI_TH-FM_NOISE_FLOOR_OFFSET;
			}
			Pacc=0;
			Nacc=0;
			for(j=NF_Idx; j<Num; j++)
			{
				if(SortData[j].rssi<(NF+FM_NOISE_FLOOR_OFFSET))
				{
					LOGI("FM Noise FLoor Detected:freq=[%d] NF=[%d] \n", SortData[j].ch,NF);
					SortData[j].reserve=0;
				}
			}
			NF_Idx=j;
			LOGI("FM Noise FLoor NF_Idx[%d] \n", NF_Idx);
		}
#endif		
	}
	LOGI("get channel no.[%d] \n", Num);
	if(Num==0)/*get nothing*/
	{
		*max_cnt = 0;
		FMR_Restore_Search(idx);
		return -1;
	}
	for(i=0; i<Num; i++)//debug
	{
		LOGI("[%d]:%d \n", i,SortData[i].ch);
	}

	switch (fmr_data.cfg_data.scan_sort) 
	{
		case FM_SCAN_SORT_UP:
		case FM_SCAN_SORT_DOWN:
		{
			LOGI("Start sort \n");
			//do sort: insert sort algorithm 
			for (i = 1; i < Num; i++) 
			{
				for (j = i; (j > 0) && ((FM_SCAN_SORT_DOWN == fmr_data.cfg_data.scan_sort) ? (SortData[j-1].rssi \
					< SortData[j].rssi) : (SortData[j-1].rssi > SortData[j].rssi)); j--) 
				{
					memcpy(&swap,&SortData[j],sizeof(struct fm_cqi));
					memcpy(&SortData[j],&SortData[j-1],sizeof(struct fm_cqi));
					memcpy(&SortData[j-1],&swap,sizeof(struct fm_cqi));
				}
			}
			LOGI("End sort \n");
			break;
		}
		default:
			break;
	}

	ChannelNo = 0;
	for(i=0; i<Num; i++)
	{
		if(SortData[i].reserve == 1)
		{
#ifndef MTK_FM_50KHZ_SUPPORT
			SortData[i].ch /= 10;
#endif
			scan_tbl[ChannelNo]=SortData[i].ch;
			ChannelNo++;
		}
	}
	*max_cnt=ChannelNo;

	LOGI("return channel no.[%d] \n", ChannelNo);
	return 0;

}

int FMR_scan(int idx, uint16_t *scan_tbl, int *max_cnt)
{
	fm_s32 ret=0;
	fm_s32 band_channel_no=0;
	fm_u8 seek_space=10;
	fm_u16 Start_Freq=8750;
#if (FMR_NOISE_FLOORT_DETECT==1)
	fm_u8 NF_Space=41;
#endif

	//FM radio seek space,5:50KHZ; 1:100KHZ; 2:200KHZ
	if(fmr_data.cfg_data.seek_space == 5)
	{
		seek_space=5;
	}
	else if (fmr_data.cfg_data.seek_space == 2)
	{
		seek_space=20;
	}
	else
	{
		seek_space=10;
	}
	if (fmr_data.cfg_data.band == FM_BAND_JAPAN)// Japan band      76MHz   ~ 90MHz
	{
		band_channel_no = (9600-7600)/seek_space + 1;
		Start_Freq = 7600;
#if (FMR_NOISE_FLOORT_DETECT==1)
		NF_Space = 400/seek_space;
#endif
	}
	else if (fmr_data.cfg_data.band == FM_BAND_JAPANW)// Japan wideband  76MHZ   ~ 108MHz
	{
		band_channel_no = (10800-7600)/seek_space + 1;
		Start_Freq = 7600;
#if (FMR_NOISE_FLOORT_DETECT==1)
		NF_Space = 640/seek_space;
#endif
	}
	else// US/Europe band  87.5MHz ~ 108MHz (DEFAULT)
	{
		band_channel_no = (10800-8750)/seek_space + 1;
		Start_Freq = 8750;
#if (FMR_NOISE_FLOORT_DETECT==1)
		NF_Space = 410/seek_space;
#endif
	}
	
	ret = FMR_scan_Channels(idx, scan_tbl, max_cnt, band_channel_no, Start_Freq, seek_space, NF_Space);

	return ret;
}
#else
int FMR_scan(int idx, uint16_t *scan_tbl, int *max_cnt)
{
	int ret = 0;
    int i,j,k,m,n;
    int flag = 0;
    int hw_sort_en = FM_SCAN_SORT_NON;
    struct fm_rssi_req req;
    struct fm_fake_channel_t *chan_info = fmr_data.cfg_data.fake_chan;
    struct fm_fake_channel *channle = NULL;
    static char cqi_buf[CQI_CH_NUM_MAX*sizeof(struct fm_cqi)] = {0};
	struct fm_cqi swap;
    struct fm_cqi *cqi = NULL;
	
    FMR_ASSERT(FMR_cbk_tbl(idx).scan);

#ifdef MT6620
	hw_sort_en = fmr_data.cfg_data.scan_sort;
#else
	hw_sort_en = FM_SCAN_SORT_NON;
#endif

    ret = FMR_cbk_tbl(idx).scan(FMR_fd(idx), scan_tbl, max_cnt, 
            fmr_data.cfg_data.band, hw_sort_en);
    if(ret){
        LOGE("%s, failed\n", __func__);
        return ret;
    }

    //MT6620 will use this way
	if (FMR_cbk_tbl(idx).fastget_rssi) {
    //find desense channels
    req.num = 0;
    if(chan_info && chan_info->chan){
        channle = chan_info->chan;
        for(j=0; j<chan_info->size; j++){
	        for(i=0; (i<*max_cnt) && (flag==0); i++){
	            if(scan_tbl[i] == channle[j].freq){
                    req.cr[req.num].freq = channle[j].freq; //add to req list
                    channle[j].reserve = i; //record position
                    req.num++;
                    flag = 1;
	            }
	        }
            flag = 0; //reset flag
        }
    }
    
    //read RSSI
	    if (req.num > 0) {
        req.read_cnt = 1;
        ret = FMR_cbk_tbl(idx).fastget_rssi(FMR_fd(idx), &req);
        if(ret){
            LOGE("Can't get RSSI\n");
                goto way_1;
        }

        //Decide delete a channel or not
        j = 0;
        for(i=0; i<chan_info->size; i++){
            if(channle[i].reserve >= 0){
                if(channle[i].rssi_th > req.cr[j].rssi){
                    //delete this channel from scan table
                    LOGI("delete %d, RSSI %d, RSSI_TH %d\n", channle[i].freq, req.cr[j].rssi, channle[i].rssi_th);
                    for(k=channle[i].reserve; k<*max_cnt; k++){
                        scan_tbl[k] = scan_tbl[k+1];
                    }
                    (*max_cnt)--;
                }
                j++;
            }
        }
    }
    }

 way_1:
    
    //MT6626/28 will use this way
    if (FMR_cbk_tbl(idx).get_cqi) {
        memset (cqi_buf, 0, CQI_CH_NUM_MAX*sizeof(struct fm_cqi));
		ret = FMR_cbk_tbl(idx).get_cqi(FMR_fd(idx), *max_cnt, cqi_buf, CQI_CH_NUM_MAX*sizeof(struct fm_cqi));
		if (ret < 0) {
			LOGE("Can't get CQI\n");
			ret = 0;
			goto out;
		}
        cqi = (struct fm_cqi*)cqi_buf;
        //show result
        LOGD("before check\n");
		for (i = 0; i < *max_cnt; i++) {
#ifndef MTK_FM_50KHZ_SUPPORT
			cqi[i].ch /= 10;
#endif
			scan_tbl[i] = cqi[i].ch;
			LOGD("NO.%d: %d --> %d(dbm)\n", i, cqi[i].ch, cqi[i].rssi);
		}

		//check chip-level de-sense list
		LOGD("chip-level de-sense check\n");
		if (FMR_cbk_tbl(idx).is_dese_chan) {
			for (i = 0; i < *max_cnt; i++) {
	            if ((FMR_cbk_tbl(idx).is_dese_chan(FMR_fd(idx), cqi[i].ch)) \
                    && (cqi[i].rssi < fmr_data.cfg_data.rssi_th_l2)) {
                    //mark for later delete 
					cqi[i].reserve = 0;
                    LOGD("del %d\n", (int)cqi[i].ch);
    }else{
                	// mark as a valid channel
                	cqi[i].reserve = 1;
                }
	        }
            
            // flush invalid channels
			for (i = 0; i < *max_cnt; i++) {
                if (0 == cqi[i].reserve) {
                    //delete this channel
                    for (k = i; k < *max_cnt; k++) {
						memcpy(&cqi[k], &cqi[k+1], sizeof(struct fm_cqi));
					}
					(*max_cnt)--;
                }
            }
        }
        

		//neighbor channel check
        LOGD("neighbor channel check\n");
        m = 0;
        if (cqi[0].ch == fmr_data.cfg_data.low_band) {
            m++;
        }
#ifdef MTK_FM_50KHZ_SUPPORT
    #define STEP_OFFEST 5
#else
#define STEP_OFFEST 1
#endif
		for (i = m; i < *max_cnt; i++) {
            n = i + 1;
            if (cqi[n-1].reserve && cqi[n].reserve && ((cqi[n-1].ch + STEP_OFFEST) == cqi[n].ch)) {
				LOGD("%d:(%d) <--v.s--> %d:(%d)\n", cqi[n-1].ch, cqi[n-1].rssi, cqi[n].ch, cqi[n].rssi);
                if (cqi[n].rssi > cqi[n-1].rssi) {
                    cqi[n-1].reserve = 0;
                    LOGD("del %d\n", cqi[n-1].ch);
                    if (((cqi[n].ch + STEP_OFFEST) < fmr_data.cfg_data.high_band) \
                        && ((cqi[n].ch + STEP_OFFEST) == cqi[n+1].ch)) {
                        cqi[n+1].reserve = 0;
                        LOGD("del %d\n", cqi[n+1].ch);
                    }
                } else {
                	cqi[n].reserve = 0;
                    LOGD("del %d\n", cqi[n].ch);
                }
                
            }
        }

        // flush invalid channels
        for (i = 0; i < *max_cnt; i++) {
			if (0 == cqi[i].reserve) {
 				//delete this channel
				for (k = i; k < *max_cnt; k++) {
					memcpy(&cqi[k], &cqi[k+1], sizeof(struct fm_cqi));
				}
				(*max_cnt)--;
			}
		}

        
        //check phone-level de-sense list
        LOGD("phone-level de-sense check\n");
        channle = chan_info->chan;
        n = 0;
        for (i = 0; i < *max_cnt; i++) {
            for (j = n; j < chan_info->size; j++) {
                if (cqi[i].ch == channle[j].freq) {
                    n++;
                    if (cqi[i].rssi < channle[j].rssi_th) {
                        // mark for later delete
                        cqi[i].reserve = 0;
                        LOGD("del %d\n", (int)cqi[i].ch);
                    } else {
                    	cqi[i].reserve = 1;
                    }
                }
            }
    }
    
		// flush invalid channels
        for (i = 0; i < *max_cnt; i++) {
			if (0 == cqi[i].reserve) {
 				//delete this channel
				for (k = i; k < *max_cnt; k++) {
					memcpy(&cqi[k], &cqi[k+1], sizeof(struct fm_cqi));
				}
				(*max_cnt)--;
			}
		}

        
        //RSSI sort if need
        LOGD("RSSI sort way %d\n", fmr_data.cfg_data.scan_sort);
		switch (fmr_data.cfg_data.scan_sort) {
			case FM_SCAN_SORT_NON:
				break;
			case FM_SCAN_SORT_UP:
			case FM_SCAN_SORT_DOWN:
				//do sort: insert sort algorithm 
				for (i = 1; i < *max_cnt; i++) {
					for (j = i; (j > 0) && ((FM_SCAN_SORT_DOWN == fmr_data.cfg_data.scan_sort) ? (cqi[j-1].rssi \
						< cqi[j].rssi) : (cqi[j-1].rssi > cqi[j].rssi)); j--) {
						swap.ch = cqi[j].ch;
						swap.rssi = cqi[j].rssi;
						cqi[j].ch = cqi[j-1].ch;
						cqi[j].rssi = cqi[j-1].rssi;
						cqi[j-1].ch = swap.ch;
						cqi[j-1].rssi = swap.rssi;
					}
				}
				break;
			default:
				break;
    }
    
		//get result
		LOGD("after check\n");
		for (i = 0; i < *max_cnt; i++) {
            scan_tbl[i] = cqi[i].ch;
            LOGD("NO.%d: %d --> %d(dbm)\n", i, cqi[i].ch, cqi[i].rssi);
        }
	} 
    
	// tune to original channel if scan result num is 0
    if (0 == *max_cnt) {
        ret = FMR_cbk_tbl(idx).tune(FMR_fd(idx), fmr_data.cur_freq, fmr_data.cfg_data.band);
		if (ret < 0) {
			LOGE("scan tune failed, [ret=%d]\n", ret);
		} else {
			LOGE("scan tuned to %d, [ret=%d]\n", fmr_data.cur_freq, ret);
		}
    }
    
out:
    LOGD("%s, [cnt=%d] [ret=%d]\n", __func__, *max_cnt, ret);
    return ret;
}
#endif

/*
 * FMR_scan_new
 * @idx - contex index
 * @ppdst - target buffer
 * @upper - scan upper band
 * @lower - scan lower band
 * @space - scan space
 * return channel number(>=0) if success, else error code 
 */
#if FMR_SOFT_MUTE_TUEN_SCAN
int FMR_seek_new(int idx, int *freq, int upper, int lower, int space, int dir, int *rssi, void *para)
{
    int ret = 0;
    int rssi_tmp = 0;
    int start_freq = *freq;
	fm_s32 band_channel_no=0;
    fm_u16 min_freq,max_freq;
    
    FMR_ASSERT(freq);

    if ((start_freq < lower) || (start_freq > upper)) {
        LOGE("start freq err: %d\n", start_freq);
        return -ERR_INVALID_PARA;
    }

    if (rssi) {
        rssi_tmp = *rssi; // rssi th
    }

	band_channel_no = (upper-lower)/space + 1;
	min_freq = lower;
	max_freq = upper;

    LOGD("seek_new start freq %d band_channel_no=[%d], seek_space=%d band[%d - %d] dir=%d\n", start_freq, band_channel_no,space,min_freq,max_freq,dir);
	
	ret = FMR_seek_Channel(idx, start_freq, min_freq, max_freq, band_channel_no, space, dir, freq, &rssi_tmp);
    
    if (ret < 0) {
        //Here we just need to return the start_freq.
        *freq = start_freq;
        LOGE("seek err, %d\n", ret);
    } else {
        fmr_data.cur_freq = *freq;
    }
	
	// return rssi to JNI if need
	if (rssi) {
        *rssi = rssi_tmp;
    }
    
    LOGD("seek_new, start %d, result %d, rssi %d, ret %d\n", start_freq, *freq, rssi_tmp, ret);
    return ret;
}

int FMR_scan_new(int idx, uint16_t *scan_tbl, int *max_cnt, int upper, int lower, int space, void *para)
{
	fm_s32 ret=0;
	fm_s32 band_channel_no=0;
	fm_u16 Start_Freq=8750;
#if (FMR_NOISE_FLOORT_DETECT==1)
	fm_u8 NF_Space=41;
#endif

	band_channel_no = (upper-lower)/space + 1;
	Start_Freq = lower;
#if (FMR_NOISE_FLOORT_DETECT==1)
	NF_Space = 410/space;
#endif

	ret = FMR_scan_Channels(idx, scan_tbl, max_cnt, band_channel_no, Start_Freq, space, NF_Space);

	return 0;
}

#else
int FMR_scan_new(int idx, void **ppdst, int upper, int lower, int space, void *para)
{
	int ret = 0;
    struct fm_ch_rssi *ch_buf = NULL;
    struct fm_ch_rssi swap;
    int num;
    int i, j, n;
	struct fm_fake_channel_t *fake_info = fmr_data.cfg_data.fake_chan;
    struct fm_fake_channel *fake_ch = NULL;
    int flag = 0;
    
	FMR_ASSERT(FMR_cbk_tbl(idx).scan_new);

	// do scan
    ret = FMR_cbk_tbl(idx).scan_new(FMR_fd(idx), (void**)&ch_buf, upper, lower, space, para);
    if ((ret < 0) || (NULL == ch_buf)) {
        LOGE("%s, failed\n", __func__);
        return ret;
    }

    num = ret;

    //check chip-level de-sense list
	LOGD("chip-level de-sense check\n");
	if (FMR_cbk_tbl(idx).is_dese_chan) {
		for (i = 0; i < num; i++) {
			if ((FMR_cbk_tbl(idx).is_dese_chan(FMR_fd(idx), ch_buf[i].freq)) \
					&& (ch_buf[i].rssi < fmr_data.cfg_data.rssi_th_l2)) {
				//mark for later delete 
				LOGD("del %d\n", (int)ch_buf[i].freq);
                ch_buf[i].freq = 0;
			} 
		}
	}

    //neighbor channel check
	LOGD("neighbor channel check\n");
	for (i = 0; i < num; i++) {
		n = i + 1;
		if (ch_buf[n-1].freq && ch_buf[n].freq && ((ch_buf[n-1].freq + space) == ch_buf[n].freq)) {
			LOGD("%d:(%d) <--v.s--> %d:(%d)\n", ch_buf[n-1].freq, ch_buf[n-1].rssi, ch_buf[n].freq, ch_buf[n].rssi);
			if (ch_buf[n].rssi > ch_buf[n-1].rssi) {
				LOGD("del %d\n", ch_buf[n-1].freq);
                ch_buf[n-1].freq = 0;
				if (((ch_buf[n].freq + space) < upper) \
					&& ((ch_buf[n].freq + space) == ch_buf[n+1].freq)) {
					LOGD("del %d\n", ch_buf[n+1].freq);
                    ch_buf[n+1].freq = 0;
				}
			} else {
				LOGD("del %d\n", ch_buf[n].freq);
                ch_buf[n].freq = 0;
			} 
		}
	}    
    
    // fake channel delete
    if (fake_info && fake_info->chan) {
        fake_ch = fake_info->chan;

        for (j = 0; j < fake_info->size; j++) {
            for (i = 0; (i < num) && (flag == 0); i++) {
                if (ch_buf[i].freq == fake_ch[j].freq) {
                    flag = 1;
                    if (ch_buf[i].rssi < fake_ch[j].rssi_th) {
                        ch_buf[i].freq = 0; // mark for delete
                    }
                }
            }
            flag = 0; //reset flag
        }
    }

    //RSSI sort if need
	LOGD("RSSI sort way %d\n", fmr_data.cfg_data.scan_sort);
	switch (fmr_data.cfg_data.scan_sort) {
		case FM_SCAN_SORT_NON:
			break;
		case FM_SCAN_SORT_UP:
		case FM_SCAN_SORT_DOWN:
		//do sort: insert sort algorithm 
			for (i = 1; i < num; i++) {
				for (j = i; (j > 0) && ((FM_SCAN_SORT_DOWN == fmr_data.cfg_data.scan_sort) ? (ch_buf[j-1].rssi \
						< ch_buf[j].rssi) : (ch_buf[j-1].rssi > ch_buf[j].rssi)); j--) {
					swap.freq = ch_buf[j].freq;
					swap.rssi = ch_buf[j].rssi;
					ch_buf[j].freq = ch_buf[j-1].freq;
					ch_buf[j].rssi = ch_buf[j-1].rssi;
					ch_buf[j-1].freq = swap.freq;
					ch_buf[j-1].rssi = swap.rssi;
				}
			}
			break;
		default:
			break;
	}
    
	*ppdst = (void*)ch_buf;
    return num;
}


int FMR_seek_new(int idx, int *freq, int upper, int lower, int space, int dir, int *rssi, void *para)
{
    int ret = 0;
    int rssi_tmp = 0;
    int start_freq = *freq;

    FMR_ASSERT(FMR_cbk_tbl(idx).seek_new);
    FMR_ASSERT(freq);

    if ((start_freq < lower) || (start_freq > upper)) {
        LOGE("start freq err: %d\n", start_freq);
        return -ERR_INVALID_PARA;
    }

    if (rssi) {
        rssi_tmp = *rssi; // rssi th
    }
    
    ret = FMR_cbk_tbl(idx).seek_new(FMR_fd(idx), freq, upper, lower, space, dir, &rssi_tmp, para);
    if (ret < 0) {
        //Here we just need to return the start_freq.
        *freq = start_freq;
        LOGE("seek err, %d\n", ret);
    } else {
        fmr_data.cur_freq = *freq;
    }

	// FIXED_ME fake channel handle
	
	// return rssi to JNI if need
	if (rssi) {
        *rssi = rssi_tmp;
    }
    
    LOGD("seek, start %d, result %d, rssi %d, ret %d\n", start_freq, *freq, rssi_tmp, ret);
    return ret;
}
#endif

int FMR_tune_new(int idx, int freq, int upper, int lower, int space, void *para)
{
    int ret = 0;

    FMR_ASSERT(FMR_cbk_tbl(idx).tune_new);

    if ((freq < lower) || (freq > upper)) {
        LOGE("freq err: %d\n", freq);
        return -ERR_INVALID_PARA;
    }
    
    ret = FMR_cbk_tbl(idx).tune_new(FMR_fd(idx), freq, upper, lower, space, para);
    if (ret < 0) {
        LOGE("tune err, %d\n", ret);
    } else {
    	fmr_data.cur_freq = freq;
    }
    
    LOGD("tune, freq %d, ret %d\n", freq, ret);
    return ret;
}
#if FMR_SOFT_MUTE_TUEN_SCAN
int FMR_stop_scan(int idx)
{
	fmr_data.scan_stop = fm_true;
	return 0;
}
#else
int FMR_stop_scan(int idx)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).stop_scan);

	ret = FMR_cbk_tbl(idx).stop_scan(FMR_fd(idx));
	if (ret){
		LOGE("%s, failed\n", __func__);
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}
#endif
int FMR_turn_on_off_rds(int idx, int onoff)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).turn_on_off_rds)
	ret = FMR_cbk_tbl(idx).turn_on_off_rds(FMR_fd(idx), onoff);
	if (ret){
		LOGE("%s, failed\n", __func__);
	}
	LOGD("%s, [onoff=%d] [ret=%d]\n", __func__, onoff, ret);
	return ret;
}
	
int FMR_read_rds_data(int idx, uint16_t *rds_status)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).read_rds_data);
	FMR_ASSERT(rds_status);

	ret = FMR_cbk_tbl(idx).read_rds_data(FMR_fd(idx), &fmr_data.rds, rds_status);
	/*if (ret){
		LOGE("%s, get no event\n", __func__);
	}*/
	LOGD("%s, [status=%d] [ret=%d]\n", __func__, *rds_status, ret);
	return ret;
}

int FMR_active_af(int idx, uint16_t *ret_freq)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).active_af);
	FMR_ASSERT(ret_freq);
	ret = FMR_cbk_tbl(idx).active_af(FMR_fd(idx),
									&fmr_data.rds,
									fmr_data.cfg_data.band, 
									fmr_data.cur_freq,
									ret_freq);
	if((ret == 0) && (*ret_freq != fmr_data.cur_freq)){
		fmr_data.cur_freq = *ret_freq;
		LOGI("active AF OK, new channel[freq=%d]\n", fmr_data.cur_freq);
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_active_ta(int idx, uint16_t *ret_freq)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).active_ta);
	FMR_ASSERT(ret_freq);
	ret = FMR_cbk_tbl(idx).active_ta(FMR_fd(idx),
									&fmr_data.rds,
									fmr_data.cfg_data.band, 
									fmr_data.cur_freq,
									&fmr_data.backup_freq,
									ret_freq);
	if((ret == 0) && (*ret_freq != fmr_data.cur_freq)){
		fmr_data.cur_freq = *ret_freq;
		LOGI("active TA OK, new channel[freq=%d]\n", fmr_data.cur_freq);
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_deactive_ta(int idx, uint16_t *ret_freq)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).deactive_ta);
	FMR_ASSERT(ret_freq);
	ret = FMR_cbk_tbl(idx).deactive_ta(FMR_fd(idx),
									&fmr_data.rds,
									fmr_data.cfg_data.band, 
									fmr_data.cur_freq,
									&fmr_data.backup_freq,
									ret_freq);
	if((ret == 0) && (*ret_freq != fmr_data.cur_freq)){
		fmr_data.cur_freq = *ret_freq;
		LOGI("deactive TA OK, new channel[freq=%d]\n", fmr_data.cur_freq);
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_is_tx_support(int idx, int *supt)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).is_tx_support);
	FMR_ASSERT(supt);

	ret = FMR_cbk_tbl(idx).is_tx_support(FMR_fd(idx), supt);
	if (ret){
		LOGE("%s, failed\n", __func__);
	}
	LOGD("%s, [supt=%d] [ret=%d]\n", __func__, *supt, ret);
	return ret;
}

int FMR_is_rdstx_support(int idx, int *supt)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).is_rdstx_support);
	FMR_ASSERT(supt);

	ret = FMR_cbk_tbl(idx).is_rdstx_support(FMR_fd(idx), supt);
	if (ret){
		LOGE("%s, failed\n", __func__);
	}
	LOGD("%s, [supt=%d] [ret=%d]\n", __func__, *supt, ret);
	return ret;
}

int FMR_tx_pwrup(int idx, int freq)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).tx_pwrup);

	LOGI("%s,[freq=%d]\n", __func__, freq);
	if (freq < fmr_data.cfg_data.low_band || freq > fmr_data.cfg_data.high_band){
		LOGE("%s error freq: %d\n", __func__, freq);
		ret = -ERR_INVALID_PARA;
		return ret;
	}
	ret = FMR_cbk_tbl(idx).tx_pwrup(FMR_fd(idx), fmr_data.cfg_data.band, freq);
	if (ret) {
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}
	fmr_data.cur_freq = freq;
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_tx_tune(int idx, int freq)
{
	int ret = 0;
	
	FMR_ASSERT(FMR_cbk_tbl(idx).tx_tune);

	LOGI("%s,[freq=%d]\n", __func__, freq);
	if (freq < fmr_data.cfg_data.low_band || freq > fmr_data.cfg_data.high_band) {
		LOGE("%s error freq: %d\n", __func__, freq);
		ret = -ERR_INVALID_PARA;
		return ret;
	}
	ret = FMR_cbk_tbl(idx).tx_tune(FMR_fd(idx), fmr_data.cfg_data.band, freq);
	if (ret) {
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}
	fmr_data.cur_freq = freq;
	
	LOGD("%s, [freq=%d] [ret=%d]\n", __func__, freq, ret);
	return ret;
}

int FMR_tx_scan(int idx, int start_freq, int dir, int *num, uint16_t *tbl)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).tx_scan);
	FMR_ASSERT(num);
	FMR_ASSERT(tbl);

	if (start_freq < fmr_data.cfg_data.low_band || start_freq > fmr_data.cfg_data.high_band) {
		LOGW("%s:error freq, [freq=%d]\n", __func__, start_freq);
		start_freq = fmr_data.cfg_data.low_band;
	}

	if((*num < 1) || (*num > 10)){
		LOGW("%s:error num, [ScanTBLSize=%d], use 10 as default\n", __func__, *num);
		*num = 10;
	}

    if(dir > 1){
		LOGW("%s:error dir, [scandir=%d], use 0(up) as default\n", __func__, dir);
		dir = 0;
	}
	
	ret = FMR_cbk_tbl(idx).tx_scan(FMR_fd(idx), fmr_data.cfg_data.band, start_freq, dir, num, tbl);
	if (ret) {
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_rdstx_onoff(int idx, int onoff)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).rdstx_onoff);

	ret = FMR_cbk_tbl(idx).rdstx_onoff(FMR_fd(idx), onoff);
	if (ret){
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}

	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_rdstx(int idx, uint16_t pi, uint8_t *ps, int ps_len, uint16_t *rds, int cnt)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).rdstx);
	FMR_ASSERT(ps);
	FMR_ASSERT(rds);

	ret = FMR_cbk_tbl(idx).rdstx(FMR_fd(idx), pi, ps, ps_len, rds, cnt);
	if (ret){
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_fm_over_bt(int idx, int onoff)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).fm_over_bt);

	ret = FMR_cbk_tbl(idx).fm_over_bt(FMR_fd(idx), onoff);
	if (ret){
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}

	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_ana_switch(int idx, int antenna)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).ana_switch);

	if(fmr_data.cfg_data.short_ana_sup == true){
	ret = FMR_cbk_tbl(idx).ana_switch(FMR_fd(idx), antenna);
	if (ret){
		LOGE("%s failed, [ret=%d]\n", __func__, ret);
	}
	}else{
		LOGW("FM antenna switch not support!\n");
		ret = -ERR_UNSUPT_SHORTANA;
	}

	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret;
}

int FMR_get_badratio(int idx, int *badratio)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).get_badratio);
	FMR_ASSERT(badratio);
	
	ret = FMR_cbk_tbl(idx).get_badratio(FMR_fd(idx), badratio);
	if (ret){
		*badratio = 0;
		LOGE("%s failed, %s\n", __func__, FMR_strerr());
	}
	
	LOGD("%s, [badratio=%d] [ret=%d]\n", __func__, *badratio, ret);
	return ret;
}

int FMR_get_stereomono(int idx, int *stemono)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).get_stereomono);
	FMR_ASSERT(stemono);
	
	ret = FMR_cbk_tbl(idx).get_stereomono(FMR_fd(idx), stemono);
	if (ret){
		*stemono = 0;
		LOGE("%s failed, %s\n", __func__, FMR_strerr());
	}
	
	LOGD("%s, [stemono=%d] [ret=%d]\n", __func__, *stemono, ret);
	return ret;
}

int FMR_set_stereomono(int idx, int stemono)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).set_stereomono);
	
	ret = FMR_cbk_tbl(idx).set_stereomono(FMR_fd(idx), stemono);
	if (ret){
		LOGE("%s failed, %s\n", __func__, FMR_strerr());
	}
	
	LOGD("%s, [stemono=%d] [ret=%d]\n", __func__, stemono, ret);
	return ret;
}

int FMR_get_caparray(int idx, int *caparray)
{
	int ret = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).get_caparray);
	FMR_ASSERT(caparray);
	
	ret = FMR_cbk_tbl(idx).get_caparray(FMR_fd(idx), caparray);
	if (ret){
		*caparray = 0;
		LOGE("%s failed, %s\n", __func__, FMR_strerr());
	}
	
	LOGD("%s, [caparray=%d] [ret=%d]\n", __func__, *caparray, ret);
	return ret;
}

int FMR_get_hw_info(int idx, int **info, int *info_len)
{
    int ret = 0;
    static int inited = 0;
    static int info_array[10] = {0};
    
    FMR_ASSERT(FMR_cbk_tbl(idx).get_hw_info);
    FMR_ASSERT(info);
    FMR_ASSERT(info_len);

    if(!inited){
        ret = FMR_cbk_tbl(idx).get_hw_info(FMR_fd(idx), &fmr_data.hw_info);
        if(ret >= 0){
            inited = 1; //get hw info success
        }
    }

    info_array[0] = fmr_data.hw_info.chip_id;
    info_array[1] = fmr_data.hw_info.eco_ver;
    info_array[2] = fmr_data.hw_info.rom_ver;
    info_array[3] = fmr_data.hw_info.patch_ver;
	
    *info = info_array;
    *info_len = sizeof(struct fm_hw_info)/sizeof(int);

    LOGD("chip:0x%08x, eco:0x%08x, rom:0x%08x, patch: 0x%08x\n", info_array[0], info_array[1], info_array[2], info_array[3]);
    LOGD("%s, [ret=%d]\n", __func__, ret);
    return ret;
}
/*
th_idx: 
	threshold type: 0, RSSI. 1,desense RSSI. 2,SMG.
th_val: threshold value*/
int FMR_EMSetTH(int idx, int th_idx, int th_val)
{
    int ret = -1;
	FMR_ASSERT(FMR_cbk_tbl(idx).set_search_threshold);
	ret=FMR_cbk_tbl(idx).set_search_threshold(FMR_fd(idx),th_idx,th_val);

	return ret;
}

int FMR_EM_CQI_logger(int idx,uint16_t cycle)
{
    int ret = -1;
	fm_full_cqi_log_t log_setting;
	uint i = 0;

	FMR_ASSERT(FMR_cbk_tbl(idx).full_cqi_logger);
	
	//log_setting.cycle = cycle;
	log_setting.lower = FM_FREQ_MIN;
	log_setting.upper = FM_FREQ_MAX;
	log_setting.space = 0x2;

	for(i = 0;i < cycle; i++)
	{
	
		log_setting.cycle = i;
		ret = FMR_cbk_tbl(idx).full_cqi_logger(FMR_fd(idx),&log_setting);
		
		LOGD("%s, [%d]\n", __func__, i);
	}

	//ret = FMR_cbk_tbl(idx).full_cqi_logger(FMR_fd(idx),&log_setting);

	return ret;
}

