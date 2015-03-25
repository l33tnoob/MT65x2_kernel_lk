/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include "cust_fmtx.h"

#ifdef FEATURE_FTM_FM
#include <linux/fm.h>
#include "ftm_audio_Common.h" 

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "FMTX"
#endif

#define FM_AUDIO_TX
/*
Global variable
*/
static int g_txfreq_item = 0;
static int g_txsetfreq = 0;
static int g_fmtx_fd = -1;

/*
Function description
*/
extern char *ftm_get_prop(const char *name);
static void read_preferred_freq();
//static void fm_updateTextInfo(char* output_buf, int buf_len);
static void *fmtx_update_thread(void *priv);

static int fmtx_open_dev(char *pname, int *fd);
static int fmtx_close_dev(int fd);
static int fmtx_pwrup(int fd, int band, int freq);
static int fmtx_pwr_down(int fd);
static int fmtx_tune(int fd, int band, int freq);
static int fm_is_rdstx_support(int fd, int *supt);
static int fmrdstx_onoff(int fd, int onoff);
static int fmrdstx_send(int fd, uint16_t pi, uint8_t *ps, int ps_len, uint16_t *rds, int cnt);


enum {
    ITEM_FREQ0,
    ITEM_FREQ1,
    ITEM_FREQ2,
    ITEM_FREQ3,
    ITEM_FREQ4,
    ITEM_FREQ5,
    ITEM_FREQ6,
    ITEM_FREQ7,
    ITEM_FREQ8,
    ITEM_FREQ9,
    ITEM_PASS,
    ITEM_FAIL,
//  ITEM_RETURN,
};

static item_t fmtx_items[] = {
    item(ITEM_FREQ0,  uistr_info_fmt_freq0),
    item(ITEM_FREQ1,  uistr_info_fmt_freq1),
    item(ITEM_FREQ2,  uistr_info_fmt_freq2),
    item(ITEM_FREQ3,  uistr_info_fmt_freq3),
    item(ITEM_FREQ4,  uistr_info_fmt_freq4),
    item(ITEM_FREQ5,  uistr_info_fmt_freq5),
    item(ITEM_FREQ6,  uistr_info_fmt_freq6),
    item(ITEM_FREQ7,  uistr_info_fmt_freq7),
    item(ITEM_FREQ8,  uistr_info_fmt_freq8),
    item(ITEM_FREQ9,  uistr_info_fmt_freq9),
    item(ITEM_PASS,   uistr_info_fmt_pass),
    item(ITEM_FAIL,   uistr_info_fmt_failed),
    //item(ITEM_RETURN, "Return"),
    item(-1, NULL),
};

struct fmtx_factory {
    char  info[1024];
    bool  exit_thd;
    int   result;

    /* for UI display */
    text_t    title;
    text_t    text;

    pthread_t update_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
};


#define mod_to_fmtx(p)     (struct fmtx_factory*)((char*)(p) + sizeof(struct ftm_module))

int fmtx_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct fmtx_factory *fmtx = (struct fmtx_factory *)priv;
    struct textview *tv;
    struct itemview *iv;

    LOGD("%s\n", __FUNCTION__);

    memset(&fmtx->info[0], 0, sizeof(fmtx->info));
    memset(&fmtx->info[0], '\n', 10);

    init_text(&fmtx->title, param->name, COLOR_YELLOW);
    init_text(&fmtx->text, &fmtx->info[0], COLOR_YELLOW);

    /* show text view */
    if (!fmtx->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD("No memory for item view");
            return -1;
        }
        fmtx->iv = iv;
    }

    iv = fmtx->iv;
    iv->set_title(iv, &fmtx->title);
    iv->set_items(iv, fmtx_items, 0);
    iv->set_text(iv, &fmtx->text);
	 #ifdef MTK_AUDIO
    Common_Audio_init();
	 #endif

    /* initialize thread condition */
    read_preferred_freq();
    fmtx->exit_thd = false;
    fmtx->result = false;
    pthread_create(&fmtx->update_thd, NULL, fmtx_update_thread, priv);

    /* process procedure */
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_FREQ0:
            g_txsetfreq = 1;
            g_txfreq_item = 0;
            break;
        case ITEM_FREQ1:
            g_txsetfreq = 1;
            g_txfreq_item = 1;
            break;
        case ITEM_FREQ2:
            g_txsetfreq = 1;
            g_txfreq_item = 2;
            break;
        case ITEM_FREQ3:
            g_txsetfreq = 1;
            g_txfreq_item = 3;
            break;
        case ITEM_FREQ4:
            g_txsetfreq = 1;
            g_txfreq_item = 4;
            break;
        case ITEM_FREQ5:
            g_txsetfreq = 1;
            g_txfreq_item = 5;
            break;
        case ITEM_FREQ6:
            g_txsetfreq = 1;
            g_txfreq_item = 6;
            break;
        case ITEM_FREQ7:
            g_txsetfreq = 1;
            g_txfreq_item = 7;
            break;
        case ITEM_FREQ8:
            g_txsetfreq = 1;
            g_txfreq_item = 8;
            break;
        case ITEM_FREQ9:
            g_txsetfreq = 1;
            g_txfreq_item = 9;
            break;
        case ITEM_PASS:
        case ITEM_FAIL:
//      case ITEM_RETURN:
            if (chosen == ITEM_PASS) {
                fmtx->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                fmtx->mod->test_result = FTM_TEST_FAIL;
            }
            g_txsetfreq = 0;        
            exit = true;
            break;
        }

        if (exit) {
            fmtx->exit_thd = true;
            break;
        }
    } while (1);
    pthread_join(fmtx->update_thd, NULL);
	 #ifdef MTK_AUDIO
    Common_Audio_deinit();
	 #endif

    return 0;
}

int fmtx_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct fmtx_factory *fmtx;

    LOGD("%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_FMTX, sizeof(struct fmtx_factory));
    if (!mod)
        return -ENOMEM;

    fmtx  = mod_to_fmtx(mod);
    fmtx->mod = mod;

    ret = ftm_register(mod, fmtx_entry, (void*)fmtx);

    return ret;
}

/*
static void fm_updateTextInfo(char* output_buf, int buf_len)
{
    return;
}
*/

static void read_preferred_freq()
{
    uint16_t tmp_freq;
    unsigned int i = 0;
    char *pFreq = NULL;
    char channel_no[64]; //max path
    
    for(i = 0; i < sizeof(fmtx_freq_list)/sizeof(fmtx_freq_list[0]); i++)
    {
        memset(channel_no, 0, sizeof(channel_no));
        sprintf(channel_no, "FMTX.CH%d", i+1);
        pFreq = ftm_get_prop(channel_no);
        
        if(pFreq != NULL){
            fmtx_freq_list[i] = (uint16_t)atoi(pFreq);
	        LOGD("preferred_freq: %d, %d\n", i, fmtx_freq_list[i]);
	    }
	    else
	    {
	        LOGD("preferred_freq %d can't get\n", i);
	    }
    }   
}

static int fmtx_open_dev(char *pname, int *fd)
{
    int ret = 0;
	int flag = 0;
	int tmp = -1;

	FMR_ASSERT(pname);
	FMR_ASSERT(fd);
	
    tmp = open(pname, O_RDWR);
    if (tmp < 0) {
        LOGE("Open %s failed, %s\n", pname, strerror(errno));
		ret = -ERR_INVALID_FD;
	}
	*fd = tmp;

	flag = fcntl(*fd, F_GETFL, tmp);
	ret = fcntl(*fd, F_SETFD, flag|FD_CLOEXEC);
	if(ret < 0){
		LOGE("fcntl %s failed, %s\n", pname, strerror(errno));
	}
	LOGI("%s, [fd=%d] [ret=%d]\n", __func__, *fd, ret);
    return ret;
}

static int fmtx_close_dev(int fd)
{
	int ret = 0;

	ret = close(fd);
	if (ret){
		LOGE("%s, failed\n", __func__);
	}

	LOGI("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
	return ret;
}

static int fmtx_pwrup(int fd, int band, int freq)
{
	int ret = 0;
	struct fm_tune_parm parm_tune;

	parm_tune.band = band;
#ifdef MTK_FM_50KHZ_SUPPORT
	parm_tune.freq = freq*10;
#else
	parm_tune.freq = freq;
#endif //MTK_FM_50KHZ_SUPPORT
	parm_tune.hilo = FM_AUTO_HILO_OFF;
	parm_tune.space = FM_SPACE_100K;

	ret = ioctl(fd, FM_IOCTL_POWERUP_TX, &parm_tune);	
	if (ret){
		LOGE("%s:fail\n", __func__);
	}
	LOGI("%s:[freq=%d] [ret=%d]\n", __func__, freq, ret);
	
	return ret;
}

static int fmtx_pwr_down(int fd)
{
	int ret = 0;
    int type = -1;

    type = FM_TX;
	ret = ioctl(fd, FM_IOCTL_POWERDOWN, &type);
	if (ret){
		LOGE("%s, failed\n", __func__);
	}
	LOGI("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
	return ret;
}

static int fmtx_tune(int fd, int band, int freq)
{
	int ret = 0;
	struct fm_tune_parm parm_tune;

	parm_tune.band = band;
#ifdef MTK_FM_50KHZ_SUPPORT
	parm_tune.freq = freq*10;
#else
	parm_tune.freq = freq;
#endif //MTK_FM_50KHZ_SUPPORT
	parm_tune.hilo = FM_AUTO_HILO_OFF;
	parm_tune.space = FM_SPACE_100K;

	ret = ioctl(fd, FM_IOCTL_TUNE_TX, &parm_tune);	
	if (ret){
		LOGE("%s:fail\n", __func__);
	}
	LOGI("%s:[freq=%d] [ret=%d]\n", __func__, freq, ret);
	
	return ret;
}

static int fm_is_rdstx_support(int fd, int *supt)
{
	int ret = 0;

	ret = ioctl(fd, FM_IOCTL_RDSTX_SUPPORT, supt);
	if (ret < 0){
		LOGE("%s: fail\n", __func__);
		//FM don't support RDS Tx
		*supt = 0;
	}
	LOGI("%s:[supt=%d] [ret=%d]\n", __func__, *supt, ret);
	return ret;
}

static int fmrdstx_onoff(int fd, int onoff)
{
	int ret = 0;

	ret = ioctl(fd, FM_IOCTL_RDSTX_ENABLE, &onoff);
	if (ret < 0){
		LOGE("%s: fail, ret = %d\n", __func__, ret);
	}

	LOGI("%s: [ret=%d]\n", __func__, ret);
	return ret;
}

static int fmrdstx_send(int fd, uint16_t pi, uint8_t *ps, int ps_len, uint16_t *rds, int cnt)
{
	int ret = 0;
	struct fm_rds_tx_parm param_rds_tx;
    struct fm_rds_tx_req rds_req;
    int i = 0;

	bzero(&rds_req, sizeof(struct fm_rds_tx_req));    
	bzero(&param_rds_tx, sizeof(struct fm_rds_tx_parm));
    
    memcpy(rds_req.ps_buf, ps, ps_len); // fill ps name
    rds_req.pi_code = pi; // fill pi 
    //fill other fileds if have any 
    //rds_req.af = 1000;

    //make pakect for Tx
    for(i = 0; i < 4; i++){
		uint16_t blk_B = 0x0;
		uint16_t blk_C = 0x0;

		if(rds_req.tp == 1){
			blk_B |= 0x0400;	// blk_B[10] = TP
		}
		blk_B |= (rds_req.pty << 5);	// blk_B[9:5] = PTY
		if(rds_req.ta == 1){
			blk_B |= 0x0010;	// blk_B[4] = TA
		}
		if(rds_req.speech == 1){
			blk_B |= 0x0008;	// blk_B[3] = TA
		}
		switch(i){
			case 0:
				if(rds_req.dyn_pty == 1){
					blk_B |= 0x0004;	// blk_B[2] = d3 = dynamic pty
				}
				blk_B |= 0x0000;	// blk_B[1:0] = 00
				break;
				case 1:
				if(rds_req.compress == 1){
					blk_B |= 0x0004;	// blk_B[2] = d2 = compressed
				}
				blk_B |= 0x0001;	// blk_B[1:0] = 01
				break;
				case 2:
				if(rds_req.ah == 1){
					blk_B |= 0x0004;	// blk_B[2] = d1 = artificial head
				}
				blk_B |= 0x0002;	// blk_B[1:0] = 10
				break;
				case 3:
				if(rds_req.stereo == 1){
					blk_B |= 0x0004;	// blk_B[2] = d0 = mono/stereo
				}
				blk_B |= 0x0003;	// blk_B[1:0] = 11
				break;
		}
		if(rds_req.af == 0){
			blk_C = 0xE000;
		}else{
			blk_C = 0xE100 | rds_req.af;
		}
		
		param_rds_tx.ps[i*3+0] = blk_B;	// block B
		param_rds_tx.ps[i*3+1] = blk_C;	// block C
		param_rds_tx.ps[i*3+2] = (rds_req.ps_buf[2*i] << 8) + rds_req.ps_buf[2*i+1];// block D
	}
	param_rds_tx.pi = rds_req.pi_code;
    param_rds_tx.other_rds_cnt = 0;

	/*if(rdscnt > 87)
	{	
		LOGE("FM JNI: setRDSTX get a invalid rdscnt[%d]\n", rdscnt);
		rdscnt = 87;
	}
	memcpy(&param_rds_tx.other_rds[0], rds, rdscnt);
    param_rds_tx.other_rds_cnt = rdscnt;*/
          
    ret = ioctl(fd, FM_IOCTL_RDS_TX, &param_rds_tx);
    if (ret){
        LOGE("FM_IOCTL_RDS_TX failed\n");
    }

    LOGI("%s: [ret=%d]\n", __func__, ret);
	return ret;
}

static void *fmtx_update_thread(void *priv)
{
    int ret = -1;
    struct fmtx_factory *fmtx = (struct fmtx_factory *)priv;
    struct itemview *iv = fmtx->iv;
    float f_freq;
    int i = 0;
    int sup = -1;

    LOGD("%s: Start\n", __FUNCTION__);
    memset(fmtx->info, 0x00, sizeof(fmtx->info));

    ret = fmtx_open_dev(FM_DEVICE_NAME, &g_fmtx_fd);
    if (ret){
        sprintf(fmtx->info, "%s", uistr_info_fmt_open_fail); 
        iv->redraw(iv);
        return NULL;
    }
    
    ret = fmtx_pwrup(g_fmtx_fd, FM_FMD_BAND, fmtx_freq_list[0]);
    if(ret){
        fmtx_close_dev(g_fmtx_fd);
        g_fmtx_fd = -1;
        sprintf(fmtx->info, "%s", uistr_info_fmt_poweron_fail); 
        iv->redraw(iv);
        return NULL;
    }
    sprintf(fmtx->info, "%s", uistr_info_fmt_poweron_ok);
    iv->redraw(iv);
    //set audio path
#ifdef FM_AUDIO_TX
    #ifdef MTK_AUDIO
    Audio_FMTX_Play(true, FMTX_1K_TONE);
	 #endif
    sprintf(fmtx->info+strlen(fmtx->info), uistr_info_fmt_audio_out);
    iv->redraw(iv);
#endif

    while(1){
        usleep(200000); // update every 200ms
        if (fmtx->exit_thd){
            break; //we get the exit message
        }

        if(g_txsetfreq){
            g_txsetfreq = 0;
            memset(fmtx->info, 0x0, sizeof(fmtx->info));

            f_freq = (float)fmtx_freq_list[g_txfreq_item]/10;
            sprintf(fmtx->info, "%s %g%s\n", uistr_info_fmt_setfreq,  f_freq, uistr_info_fmt_mhz);
            iv->redraw(iv);

            //tune to desiered channel
            ret = fmtx_tune(g_fmtx_fd, FM_FMD_BAND, fmtx_freq_list[g_txfreq_item]);
            if (ret){
                sprintf(fmtx->info+strlen(fmtx->info), uistr_info_fmt_fail);
                iv->redraw(iv);
            }else{
                sprintf(fmtx->info+strlen(fmtx->info), uistr_info_fmt_success);
                iv->redraw(iv);

                //check if RDS TX is supported
                ret = fm_is_rdstx_support(g_fmtx_fd, &sup);
                if(ret){
                    sprintf(fmtx->info+strlen(fmtx->info), uistr_info_fmt_check_rds_fail);
                    iv->redraw(iv);
                }
                
                //we need enable FM RDS TX before send RDS data
                ret = fmrdstx_onoff(g_fmtx_fd, FM_RDS_TX_ENABLE);
                if(ret){
                    sprintf(fmtx->info+strlen(fmtx->info), uistr_info_fmt_enable_rds_fail);
                    iv->redraw(iv);
                }

                //fill RDS data structure, then send RDS data to chip
                for(i = 0; i < 8; i++){
                    fmtx_ps[i] = 0x20;
                }
                memcpy(fmtx_ps, FM_TX_RDS, (strlen(FM_TX_RDS)>8 ? 8 : strlen(FM_TX_RDS)));
                ret = fmrdstx_send(g_fmtx_fd, 0x0001, fmtx_ps, 8, NULL, 0);
                if(ret){
                    sprintf(fmtx->info+strlen(fmtx->info), uistr_info_fmt_set_rds_fail);
                    iv->redraw(iv);
                }else{
                    sprintf(fmtx->info+strlen(fmtx->info), uistr_info_fmt_rds);
                    sprintf(fmtx->info+strlen(fmtx->info), FM_TX_RDS);
                    sprintf(fmtx->info+strlen(fmtx->info), "\n");
                    iv->redraw(iv);
                }
            }
        }
    }

    //set Audio path
#ifdef FM_AUDIO_TX
    #ifdef MTK_AUDIO 
    Audio_FMTX_Play(false, FMTX_1K_TONE);
	 #endif
#endif
    fmtx_pwr_down(g_fmtx_fd);
    fmtx_close_dev(g_fmtx_fd);
    g_fmtx_fd = -1;

    LOGI("%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
	return NULL;
}
#endif
