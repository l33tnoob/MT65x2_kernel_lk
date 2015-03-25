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

#ifndef __FMR_H__
#define __FMR_H__

#include <jni.h>
#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <linux/serial.h> 
#include <linux/fm.h>
#include <signal.h>
#include <errno.h>
#include <dlfcn.h>
#include "../custom/fmlib_cust.h"
 
#define FM_LIB_USE_XLOG

#ifdef FM_LIB_USE_XLOG
#include <cutils/xlog.h>
#undef LOGV
#define LOGV(...) XLOGV(__VA_ARGS__) 
#undef LOGD
#define LOGD(...) XLOGD(__VA_ARGS__) 
#undef LOGI
#define LOGI(...) XLOGI(__VA_ARGS__) 
#undef LOGW
#define LOGW(...) XLOGW(__VA_ARGS__) 
#undef LOGE
#define LOGE(...) XLOGE(__VA_ARGS__) 
#endif

#define CUST_LIB_NAME "libfmcust.so"
#define AR1000_LIB_NAME "libfmar1000.so"
#define MT6616_LIB_NAME "libfmmt6616.so"
#define MT6620_LIB_NAME "libfmmt6620.so"
#define MT6626_LIB_NAME "libfmmt6626.so"
#define MT6628_LIB_NAME "libfmmt6628.so"
#define MT6627_LIB_NAME "libfmmt6627.so"
#define MT6630_LIB_NAME "libfmmt6630.so"
#define MT519X_LIB_NAME "libfmmt519x.so"
#define FM_DEV_NAME "/dev/fm"

#define FM_RDS_PS_LEN 8

struct fm_cbk_tbl {
	//Basic functions.
	int (*open_dev)(const char *pname, int *fd);
	int (*close_dev)(int fd);
	int (*pwr_up)(int fd, int band, int freq);
	int (*pwr_down)(int fd, int type);
	int (*seek)(int fd, int *freq, int band, int dir, int lev);
	int (*scan)(int fd, uint16_t *tbl, int *num, int band, int sort);
	int (*scan_new)(int fd, void **ppdst, int upper, int lower, int space, void *para);
	int (*seek_new)(int fd, int *freq, int upper, int lower, int space, int dir, int *rssi, void *para);
	int (*tune_new)(int fd, int freq, int upper, int lower, int space, void *para);
	int (*fastget_rssi)(int fd, struct fm_rssi_req *rssi_req);
        int (*get_cqi)(int fd, int num, char *buf, int buf_len);
	int (*stop_scan)(int fd);
	int (*tune)(int fd, int freq, int band);
	int (*set_mute)(int fd, int mute);
	int (*is_fm_pwrup)(int fd, int *pwrup);
	int (*is_rdsrx_support)(int fd, int *supt);
	int (*is_rdstx_support)(int fd, int *supt);
	int (*turn_on_off_rds)(int fd, int onoff);
	int (*get_chip_id)(int fd, int *chipid);
	//FOR RDS RX.
	int (*read_rds_data)(int fd, RDSData_Struct *rds, uint16_t *rds_status);
	int (*get_pi)(int fd, RDSData_Struct *rds, uint16_t *pi);
	int (*get_ps)(int fd, RDSData_Struct *rds, uint8_t **ps, int *ps_len);
	int (*get_pty)(int fd, RDSData_Struct *rds, uint8_t *pty);
	int (*get_rssi)(int fd, int *rssi);
	int (*get_rt)(int fd, RDSData_Struct *rds, uint8_t **rt, int *rt_len);
	int (*active_af)(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *ret_freq);
	int (*active_ta)(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq);
	int (*deactive_ta)(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq);
	//FOR RDS TX.
	int (*is_tx_support)(int fd, int *supt);
    int (*tx_pwrup)(int fd, int band, int freq);
    int (*tx_tune)(int fd, int band, int freq);
    int (*tx_scan)(int fd, int band, int start_freq, int dir, int *num, uint16_t *tbl);
    int (*rdstx_onoff)(int fd, int onoff);
    int (*rdstx)(int fd, uint16_t pi, uint8_t *ps, int ps_len, uint16_t *rds, int cnt);
    // For FM new feature
    //FM over BT
    int (*fm_over_bt)(int fd, int onoff);
    	//FM long/short antenna switch
	int (*ana_switch)(int fd, int antenna);
	//For FM RX EM mode
	int (*get_badratio)(int fd, int *badratio);
	int (*get_stereomono)(int fd, int *stemono);
    int (*set_stereomono)(int fd, int stemono);
	int (*get_caparray)(int fd, int *caparray);
    int (*get_hw_info)(int fd, struct fm_hw_info *info);
    int (*is_dese_chan)(int fd, int freq);
	int (*soft_mute_tune)(int fd,fm_softmute_tune_t * para);
	int (*desense_check)(int fd,int freq,int rssi);
	int (*set_search_threshold)(int fd, int th_idx,int th_val);
	int (*full_cqi_logger)(int fd,fm_full_cqi_log_t *log_parm);
	/*New search*/
	int (*pre_search)(int fd);
	int (*restore_search)(int fd);
};

typedef int (*CUST_func_type)(struct CUST_cfg_ds *);
typedef void (*init_func_type)(struct fm_cbk_tbl *);

struct fmr_ds {
	int fd;
	int err;
    uint16_t cur_freq;
    uint16_t backup_freq;
	void *priv;
	void *custom_handler;
	struct CUST_cfg_ds cfg_data;
	struct fm_cbk_tbl tbl;
	CUST_func_type get_cfg;
    void* init_handler;
	init_func_type init_func;
    RDSData_Struct rds;
    struct fm_hw_info hw_info;
    fm_bool scan_stop;
};
/*#define fmr_fd (FMR_data.fd)
#define fmr_err (FMR_data.err)
#define fmr_chip (FMR_data.cfg_data.chip)
#define fmr_low_band (FMR_data.cfg_data.low_band)
#define fmr_high_band (FMR_data.cfg_data.high_band)
#define fmr_seek_space (FMR_data.cfg_data.seek_space)
#define fmr_max_scan_num (FMR_data.cfg_data.max_scan_num)
#define fmr_cbk_tbl (FMR_data.tbl)
#define fmr_cust_hdler (FMR_data.custom_handler)
#define fmr_get_cfg (FMR_data.get_cfg)

extern struct fmr_ds FMR_data;*/

enum fmr_err_em {
    ERR_SUCCESS = 1000, // kernel error begin at here
    ERR_INVALID_BUF,
    ERR_INVALID_PARA,
    ERR_STP,
    ERR_GET_MUTEX,
    ERR_FW_NORES,
    ERR_RDS_CRC,
    ERR_INVALID_FD, //  native error begin at here
	ERR_UNSUPPORT_CHIP,
	ERR_LD_LIB,
	ERR_FIND_CUST_FNUC,
	ERR_UNINIT,
	ERR_NO_MORE_IDX,
	ERR_RDS_NO_DATA,
    ERR_UNSUPT_SHORTANA,
    ERR_MAX
};

enum fmr_rds_onoff
{
  FMR_RDS_ON,
  FMR_RDS_OFF,
  FMR_MAX
};

typedef enum
{
	FM_LONG_ANA = 0,
	FM_SHORT_ANA
}fm_antenna_type;


#define CQI_CH_NUM_MAX 255
#define CQI_CH_NUM_MIN 0


/****************** Function declaration ******************/
//fmr_err.cpp
char *FMR_strerr();
void FMR_seterr(int err);

//fmr_core.cpp
int FMR_init(void);
int FMR_get_cfgs(int idx);
int FMR_open_dev(int idx);
int FMR_close_dev(int idx);
int FMR_pwr_up(int idx, int freq);
int FMR_pwr_down(int idx, int type);
int FMR_seek(int idx, int start_freq, int dir, int *ret_freq);
int FMR_scan(int idx, uint16_t *tbl, int *num);
int FMR_scan_new(int idx, uint16_t *scan_tbl, int *max_cnt, int upper, int lower, int space, void *para);
int FMR_seek_new(int idx, int *freq, int upper, int lower, int space, int dir, int *rssi, void *para);
int FMR_tune_new(int idx, int freq, int upper, int lower, int space, void *para);
int FMR_stop_scan(int idx);
int FMR_tune(int idx, int freq);
int FMR_set_mute(int idx, int mute);
int FMR_is_fm_pwrup(int idx, int *pwrup);
int FMR_is_rdsrx_support(int idx, int *supt);
int FMR_turn_on_off_rds(int idx, int onoff);
int FMR_get_chip_id(int idx, int *chipid);
int FMR_read_rds_data(int idx, uint16_t *rds_status);
int FMR_get_pi(int idx, uint16_t *pi);
int FMR_get_ps(int idx, uint8_t **ps, int *ps_len);
int FMR_get_pty(int idx, uint8_t *pty);
int FMR_get_rssi(int idx, int *rssi);
int FMR_get_rt(int idx, uint8_t **rt, int *rt_len);
int FMR_active_af(int idx, uint16_t *ret_freq);
int FMR_active_ta(int idx, uint16_t *ret_freq);
int FMR_deactive_ta(int idx, uint16_t *ret_freq);

int FMR_is_tx_support(int idx, int *supt);
int FMR_is_rdstx_support(int idx, int *supt);
int FMR_tx_pwrup(int idx, int freq);
int FMR_tx_tune(int idx, int freq);
int FMR_tx_scan(int idx, int start_freq, int dir, int *num, uint16_t *tbl);
int FMR_rdstx_onoff(int idx, int onoff);
int FMR_rdstx(int idx, uint16_t pi, uint8_t *ps, int ps_len, uint16_t *rds, int cnt);

int FMR_fm_over_bt(int idx, int onoff);
int FMR_ana_switch(int idx, int antenna);
int FMR_get_badratio(int idx, int *badratio);
int FMR_get_stereomono(int idx, int *stemono);
int FMR_set_stereomono(int idx, int stemono);
int FMR_get_caparray(int idx, int *caparray);
int FMR_get_hw_info(int idx, int **info, int *info_len);
int FMR_Pre_Search(int idx);
int FMR_Restore_Search(int idx);
int FMR_EMSetTH(int idx, int th_idx, int th_val);
int FMR_EM_CQI_logger(int idx,uint16_t cycle);


//common part
int COM_open_dev(const char *pname, int *fd);
int COM_close_dev(int fd);
int COM_pwr_up(int fd, int band, int freq);
int COM_pwr_down(int fd, int type);
int COM_seek(int fd, int *freq, int band, int dir, int lev);
int COM_Soft_Mute_Tune(int fd,fm_softmute_tune_t * para);
//int COM_dese_chan_check(int fd, int freq);
int COM_hw_scan(int fd, uint16_t *tbl, int *num, int band, int sort);
int COM_hw_scan_new(int fd, void **ppdst, int upper, int lower, int space, void *para);
int COM_seek_new(int fd, int *freq, int upper, int lower, int space, int dir, int *rssi, void *para);
int COM_tune_new(int fd, int freq, int upper, int lower, int space, void *para);
int COM_fastget_rssi(int fd, struct fm_rssi_req *rssi_req);
int COM_get_cqi(int fd, int num, char *buf, int buf_len);
int COM_sw_scan(int fd, uint16_t *tbl, int *num, int band, int sort);
int COM_stop_scan(int fd);
int COM_tune(int fd, int freq, int band);
int COM_set_mute(int fd, int mute);
int COM_is_fm_pwrup(int fd, int *pwrup);
int COM_is_rdsrx_support(int fd, int *supt);
int COM_is_rdstx_support(int fd, int *supt);
int COM_turn_on_off_rds(int fd, int onoff);
int COM_get_chip_id(int fd, int *chipid);
int COM_read_rds_data(int fd, RDSData_Struct *rds, uint16_t *rds_status);
int COM_get_pi(int fd, RDSData_Struct *rds, uint16_t *pi);
int COM_get_ps(int fd, RDSData_Struct *rds, uint8_t **ps, int *ps_len);
int COM_get_pty(int fd, RDSData_Struct *rds, uint8_t *pty);
int COM_get_rssi(int fd, int *rssi);
int COM_get_rt(int fd, RDSData_Struct *rds, uint8_t **rt, int *rt_len);
int COM_active_af(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *ret_freq);
int COM_active_ta(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq);
int COM_deactive_ta(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq);

int COM_is_tx_support(int fd, int *supt);
int COM_tx_support(int fd, int *supt);
int COM_tx_pwrup(int fd, int band, int freq);
int COM_tx_tune(int fd, int band, int freq);
int COM_tx_scan(int fd, int band, int start_freq, int dir, int *num, uint16_t *tbl);
int COM_rdstx_onoff(int fd, int onoff);
int COM_rdstx(int fd, uint16_t pi, uint8_t *ps, int ps_len, uint16_t *rds, int cnt);

int COM_fm_over_bt(int fd, int onoff);
int COM_ana_switch(int fd, int antenna);
int COM_get_badratio(int fd, int *badratio);
int COM_get_stereomono(int fd, int *stemono);
int COM_set_stereomono(int fd, int stemono);
int COM_get_caparray(int fd, int *caparray);
int COM_get_hw_info(int fd, struct fm_hw_info *info);
int COM_is_dese_chan(int fd, int freq);
int COM_desense_check(int fd, int freq, int rssi);
int COM_pre_search(int fd);
int COM_restore_search(int fd);
int COM_set_search_threshold(int fd, int th_idx,int th_val);
int COM_full_cqi_logger(int fd, fm_full_cqi_log_t *log_parm);

int bt_set_controller_force_sleep(int chip);

#define FMR_ASSERT(a) { \
			if ((a) == NULL) { \
				LOGE("%s,invalid buf\n", __func__);\
				return -ERR_INVALID_BUF; \
			} \
		}
#endif
