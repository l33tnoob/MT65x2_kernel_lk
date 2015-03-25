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

#include <jni.h>
#include "fmr.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FMLIB_JNI"

static int g_idx = -1;
extern struct fmr_ds fmr_data;

jboolean opendev(JNIEnv *env, jobject thiz)
{
    int ret = 0;

    ret = FMR_open_dev(g_idx); // if success, then ret = 0; else ret < 0

	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

jboolean closedev(JNIEnv *env, jobject thiz)
{
	int ret = 0;

	ret = FMR_close_dev(g_idx);
	
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

jboolean powerup(JNIEnv *env, jobject thiz, jfloat freq)
{
	int ret = 0;
	int tmp_freq;

	LOGI("%s, [freq=%d]\n", __func__, (int)freq);
#ifdef MTK_FM_50KHZ_SUPPORT
	tmp_freq = (int)(freq * 100);  	//Eg, 87.55 * 100 --> 8755
#else
	tmp_freq = (int)(freq * 10);	//Eg, 87.5 * 10 --> 875
#endif
	ret = FMR_pwr_up(g_idx, tmp_freq);

	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

jboolean powerdown(JNIEnv *env, jobject thiz, jint type)
{
	int ret = 0;

	ret = FMR_pwr_down(g_idx, type);

	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

jint readRssi(JNIEnv *env, jobject thiz)  //jint = int
{
	int ret = 0;
	int rssi = -1;

	ret = FMR_get_rssi(g_idx, &rssi);

	LOGD("%s, [ret=%d]\n", __func__, ret);
	return rssi;
}

jboolean tune(JNIEnv *env, jobject thiz, jfloat freq)
{
	int ret = 0;
	int tmp_freq;

#ifdef MTK_FM_50KHZ_SUPPORT
	tmp_freq = (int)(freq * 100);  	//Eg, 87.55 * 100 --> 8755
#else
	tmp_freq = (int)(freq * 10);	//Eg, 87.5 * 10 --> 875
#endif
	ret = FMR_tune(g_idx, tmp_freq);

	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

jfloat seek(JNIEnv *env, jobject thiz, jfloat freq, jboolean isUp) //jboolean isUp;
{
	int ret = 0;
	int tmp_freq;
	int ret_freq;
    float val;

#ifdef MTK_FM_50KHZ_SUPPORT
        tmp_freq = (int)(freq * 100);   //Eg, 87.55 * 100 --> 8755
#elif FMR_SOFT_MUTE_TUEN_SCAN
	tmp_freq = (int)(freq * 100);	//Eg, 87.55 * 100 --> 8755
	ret = FMR_set_mute(g_idx, 1);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [mute] [ret=%d]\n", __func__, ret);
#else
        tmp_freq = (int)(freq * 10);    //Eg, 87.5 * 10 --> 875
#endif

	ret = FMR_seek(g_idx, tmp_freq, (int)isUp, &ret_freq);
	if (ret){
		ret_freq = tmp_freq; //seek error, so use original freq 
	}

	LOGD("%s, [freq=%d] [ret=%d]\n", __func__, ret_freq, ret);
    
#ifdef MTK_FM_50KHZ_SUPPORT
        val = (float)ret_freq/100;   //Eg, 8755 / 100 --> 87.55
#elif FMR_SOFT_MUTE_TUEN_SCAN
	val = (float)ret_freq/100;	 //Eg, 8755 / 100 --> 87.55
#else
        val = (float)ret_freq/10;    //Eg, 875 / 10 --> 87.5
#endif    
	return val;
}

jshortArray autoscan(JNIEnv *env, jobject thiz)
{
#define FM_SCAN_CH_SIZE_MAX 200
	int ret = 0;
	jshortArray scanChlarray;
	int chl_cnt = FM_SCAN_CH_SIZE_MAX;
	uint16_t ScanTBL[FM_SCAN_CH_SIZE_MAX];

	LOGI("%s, [tbl=%p]\n", __func__, ScanTBL);
	FMR_Pre_Search(g_idx);
	ret = FMR_scan(g_idx, ScanTBL, &chl_cnt);
	if (ret < 0) {
		LOGE("scan failed!\n");
		scanChlarray = NULL;
		goto out;
	}
	if (chl_cnt > 0) {
		scanChlarray = env->NewShortArray(chl_cnt);
		env->SetShortArrayRegion(scanChlarray, 0, chl_cnt, (const jshort*)&ScanTBL[0]);
	} else {
		LOGE("cnt error, [cnt=%d]\n", chl_cnt);
		scanChlarray = NULL;
	}
	FMR_Restore_Search(g_idx);

	if(fmr_data.scan_stop == fm_true)
	{
		ret = FMR_tune(g_idx,fmr_data.cur_freq);
		LOGI("scan stop!!! tune ret=%d",ret);
		scanChlarray = NULL;
		ret = -1;
	}
	
out:
	LOGD("%s, [cnt=%d] [ret=%d]\n", __func__, chl_cnt, ret);
	return scanChlarray;
}


/* 
 * scannew
 * @env - davlink VM
 * @thiz - current object
 * @upper - upper band, Eg: 10800 (108.00Mhz)
 * @lower - lower band, Eg: 7600 (76.00Mhz)
 * @space - scan space, 5: 50Khz, 10: 100Khz, 20: 200Khz
 * Return channel list if success, else NULL
 */
jshortArray scannew(JNIEnv *env, jobject thiz, jint upper, jint lower, jint space)
{
	#define FM_SCAN_CH_SIZE_MAX 200
    int ret = 0;
    jshortArray scanChlarray;
	int chl_cnt = FM_SCAN_CH_SIZE_MAX;
	uint16_t ScanTBL[FM_SCAN_CH_SIZE_MAX];
    int i, j;
    
	LOGI("%s, [tbl=%p]\n", __func__, ScanTBL);
	FMR_Pre_Search(g_idx);
    ret = FMR_scan_new(g_idx, ScanTBL, &chl_cnt, upper, lower, space, NULL);
    if (ret < 0) {
        LOGE("scan failed!\n");
        scanChlarray = NULL;
        goto out;
    }

	if (chl_cnt > 0){
        scanChlarray = env->NewShortArray(chl_cnt);
		env->SetShortArrayRegion(scanChlarray, 0, chl_cnt, (const jshort*)&ScanTBL[0]);
        }
    else {
        LOGE("cnt error, [cnt=%d]\n", ret);
        scanChlarray = NULL;
    }

	FMR_Restore_Search(g_idx);
	
out:
	LOGD("%s, [cnt=%d] [ret=%d]\n", __func__, chl_cnt, ret);
    return scanChlarray;
}


/* 
 * seeknew
 * @env - davlink VM
 * @thiz - current object
 * @upper - upper band, Eg: 10800 (108.00Mhz)
 * @lower - lower band, Eg: 7600 (76.00Mhz)
 * @space - seek space, 5: 50Khz, 10: 100Khz, 20: 200Khz
 * @start - start freq, Eg, 9000 (90.00Mhz)
 * @dir - seek direction, 0: up; 1, down
 * @lev - seek rssi threshold, eg -95dbm
 * Return channel if success, else -1
 */
jint seeknew(JNIEnv *env, jobject thiz, jint upper, jint lower, jint space, jint start, jint dir, jint lev)
{
    int ret = 0;
    int tmp_freq = start;
    int rssi_th = lev;

    ret = FMR_seek_new(g_idx, &tmp_freq, upper, lower, space, dir, &rssi_th, NULL);
    if (ret < 0) {
        tmp_freq = -1;
    }

    LOGD("seeknew result %d, ret %d\n", tmp_freq, ret);
    return tmp_freq;
}


/* 
 * tunenew
 * @env - davlink VM
 * @thiz - current object
 * @upper - upper band, Eg: 10800 (108.00Mhz)
 * @lower - lower band, Eg: 7600 (76.00Mhz)
 * @space - seek space, 5: 50Khz, 10: 100Khz, 20: 200Khz
 * @freq - target freq, Eg, 9000 (90.00Mhz)
 * Return JNI_TRUE if success, else JNI_FALSE
 */
jboolean tunenew(JNIEnv *env, jobject thiz, jint upper, jint lower, jint space, jint freq)
{
    int ret = 0;

    ret = FMR_tune_new(g_idx, freq, upper, lower, space, NULL);

    LOGD("tunenew ret %d\n", ret);
    return (ret < 0) ? JNI_FALSE : JNI_TRUE;
}

jboolean emsetth(JNIEnv *env, jobject thiz, jint index, jint value)
{
    int ret = 0;

    ret = FMR_EMSetTH(g_idx, index, value);
	
    LOGD("emsetth ret %d\n", ret);
    return (ret < 0) ? JNI_FALSE : JNI_TRUE;
}
jshortArray emcmd(JNIEnv *env, jobject thiz, jshortArray val)
{
	jshortArray eventarray;
	uint16_t eventtbl[20]={0};
	uint16_t* cmdtbl=NULL;

	cmdtbl = (uint16_t*) env->GetShortArrayElements(val, NULL);
	if(cmdtbl == NULL)
	{
		LOGE("%s:get cmdtbl error\n", __func__);
		goto out;
	}
	LOGI("EM cmd:=%x %x %x %x %x",cmdtbl[0],cmdtbl[1],cmdtbl[2],cmdtbl[3],cmdtbl[4]);

	if(!cmdtbl[0])	//LANT
	{
	}
	else			//SANT
	{
	}

	switch(cmdtbl[1])
	{
		case 0x02:	//CQI log tool
		{
			FMR_EM_CQI_logger(g_idx, cmdtbl[2]);
		}
			break;
		default:
			break;
	}
	
	eventarray = env->NewShortArray(20);
	env->SetShortArrayRegion(eventarray, 0, 20, (const jshort*)&eventtbl[0]);
    //LOGD("emsetth ret %d\n", ret);
out:    
    return eventarray;
}

jshort readrds(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	uint16_t status = 0;

	ret = FMR_read_rds_data(g_idx, &status);

	if(ret){
		//LOGE("%s,status = 0,[ret=%d]\n", __func__, ret);
		status = 0; //there's no event or some error happened
	}
	
	return status;
}

jshort getPI(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	uint16_t pi = 0;

	ret = FMR_get_pi(g_idx, &pi);

	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		pi = -1; //there's some error happened
	}
	return pi;
}

jbyte getPTY(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	uint8_t pty = 0;

	ret = FMR_get_pty(g_idx, &pty);

	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		pty = -1; //there's some error happened
	}
	return pty;
}

jbyteArray getPS(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	jbyteArray PSname;
	uint8_t *ps = NULL;
	int ps_len = 0;

	ret = FMR_get_ps(g_idx, &ps, &ps_len);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		return NULL;
	}
	PSname = env->NewByteArray(ps_len);
	env->SetByteArrayRegion(PSname, 0, ps_len, (const jbyte*)ps);
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return PSname;    
}

jbyteArray getLRText(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	jbyteArray LastRadioText;
	uint8_t *rt = NULL;
	int rt_len = 0;

	ret = FMR_get_rt(g_idx, &rt, &rt_len);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		return NULL;
	}
	LastRadioText = env->NewByteArray(rt_len);
	env->SetByteArrayRegion(LastRadioText, 0, rt_len, (const jbyte*)rt);
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return LastRadioText;
}

jshort activeAF(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	jshort ret_freq = 0;
	
	ret = FMR_active_af(g_idx, (uint16_t*)&ret_freq);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		return 0;
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret_freq;
}

jshortArray getAFList(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	jshortArray AFList;
	char *af = NULL;
	int af_len = 0;

	//ret = FMR_get_af(g_idx, &af, &af_len); // If need, we should implemate this API
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		return NULL;
	}
	AFList = env->NewShortArray(af_len);
	env->SetShortArrayRegion(AFList, 0, af_len, (const jshort*)af);
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return AFList;   
}

jshort activeTA(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	jshort ret_freq = 0;
	
	ret = FMR_active_ta(g_idx, (uint16_t*)&ret_freq);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		return 0;
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret_freq;
}

jshort deactiveTA(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	jshort ret_freq = 0;
	
	ret = FMR_deactive_ta(g_idx, (uint16_t*)&ret_freq);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		return 0;
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret_freq;
}

jint rdsset(JNIEnv *env, jobject thiz, jboolean rdson)
{
	int ret = 0;
	int onoff = -1;

	if(rdson == JNI_TRUE){
		onoff = FMR_RDS_ON;
	}else{
		onoff = FMR_RDS_OFF;
	}
	ret = FMR_turn_on_off_rds(g_idx, onoff);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [onoff=%d] [ret=%d]\n", __func__, onoff, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

jboolean stopscan(JNIEnv *env, jobject thiz)
{
	int ret = 0;

	ret = FMR_stop_scan(g_idx);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

jint setmute(JNIEnv *env, jobject thiz, jboolean mute)
{
	int ret = 0;

	ret = FMR_set_mute(g_idx, (int)mute);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [mute=%d] [ret=%d]\n", __func__, (int)mute, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

/******************************************
 * Used to get chip ID.
 *Parameter: 
 *	None
 *Return value
 *	1000: chip AR1000
 *	6616: chip mt6616
 *	-1: error
 ******************************************/
jint getchipid(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	int chipid = -1;
	ret = FMR_get_chip_id(g_idx, &chipid);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [chipid=%x] [ret=%d]\n", __func__, chipid, ret);
	return chipid;
}

/******************************************
 * Inquiry if RDS is support in driver.
 * Parameter:
 *	None
 *Return Value:
 *	1: support
 *	0: NOT support
 *	-1: error
 ******************************************/
jint isRDSsupport(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	int supt = -1;

	ret = FMR_is_rdsrx_support(g_idx, &supt);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [supt=%d] [ret=%d]\n", __func__, supt, ret);
	return supt;
}

/******************************************
 * Inquiry if FM is powered up.
 * Parameter:
 *	None
 *Return Value:
 *	1: Powered up
 *	0: Did NOT powered up
 ******************************************/
jint isFMPoweredUp(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	int pwrup = -1;
	
	ret = FMR_is_fm_pwrup(g_idx, &pwrup);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [pwrup=%d] [ret=%d]\n", __func__, pwrup, ret);
	return pwrup;
}

/******************************************
 * Inquiry if TX is support in driver.
 * Parameter:
 *	None
 *Return Value:
 *	1: support
 *	0: NOT support
 *	-1: error
 ******************************************/
jint isTXSupport(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	int support = -1;

	ret = FMR_is_tx_support(g_idx, &support);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [support=%d] [ret=%d]\n", __func__, support, ret);

	return support;
}

/******************************************
 * Start to transmit
 * Parameter:
 *	freq: Which freqency to work on.
 *Return Value:
 *	True: Sucess
 *	False: Failed
 ******************************************/
jboolean powerupTX(JNIEnv *env, jobject thiz, jfloat freq)
{
	int ret = 0;
	int tmp_freq;

#ifdef MTK_FM_50KHZ_SUPPORT
	tmp_freq = (int)(freq * 100);   //Eg, 87.55 * 100 --> 8755
#else
	tmp_freq = (int)(freq * 10);    //Eg, 87.5 * 10 --> 875
#endif

	LOGI("%s, [freq=%d]\n", __func__, tmp_freq);
	
	ret = FMR_tx_pwrup(g_idx, tmp_freq);
	if(ret){
		LOGE("%s, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [freq=%d] [ret=%d]\n", __func__, tmp_freq, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

/*********************************************************
*Description:	Tune FM to a desired  channel in TX mode
*
*Date:		2010.12.01
*
*Parameter:	freq, the freqency will be tune 
*
*Return:		TRUE, if success; FALSE, if failed
*********************************************************/
jboolean tuneTX(JNIEnv *env, jobject thiz, jfloat freq)
{
	int ret = 0;
	int tmp_freq;

#ifdef MTK_FM_50KHZ_SUPPORT
	tmp_freq = (int)(freq * 100);   //Eg, 87.55 * 100 --> 8755
#else
	tmp_freq = (int)(freq * 10);    //Eg, 87.5 * 10 --> 875
#endif

	LOGI("%s, [freq=%d]\n", __func__, tmp_freq);
	
	ret = FMR_tx_tune(g_idx, tmp_freq);
	if(ret){
		LOGE("%s, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [freq=%d] [ret=%d]\n", __func__, tmp_freq, ret);
	return ret?JNI_FALSE:JNI_TRUE;
}

/******************************************
 * Get useable TX frequency list.
 * Parameter:
 *	freq: Start frenquency.
 * num: max number to scan
 * Return Value:	TX frequency list.
 ******************************************/
jshortArray getTXFreqList(JNIEnv *env, jobject thiz, jfloat freq, jint scandir, jint num)
{
	int ret = 0;
	uint16_t tbl[TX_SCAN_MAX+1] = {0};
	int start_freq;
	int dir = scandir;
	int scan_num = num;

#ifdef MTK_FM_50KHZ_SUPPORT
	start_freq = (int)(freq * 100);   //Eg, 87.55 * 100 --> 8755
#else
	start_freq = (int)(freq * 10);    //Eg, 87.5 * 10 --> 875
#endif

	LOGI("%s, [freq=%d] [dir=%d] [num=%d]\n", __func__, start_freq, dir, scan_num);
	ret = FMR_tx_scan(g_idx, start_freq, dir, &scan_num, &tbl[0]);
	if(ret){
		LOGE("%s:failed, [ret=%d]\n", __func__, ret);
		goto err;
	}
	
	if ((scan_num > 0) && (scan_num < (TX_SCAN_MAX+1))){
        jshortArray scanChlarray ;
		int i = 0;
        scanChlarray = env->NewShortArray(scan_num);
        env->SetShortArrayRegion(scanChlarray, 0, scan_num, (const jshort*)&tbl[0]);
        LOGD("Success,[chl_cnt=%d]\n", scan_num);
		for(i = 0; i < scan_num; i++){
			LOGI("NO.%d:%d\n", i+1, tbl[i]);
		}
        return scanChlarray;
    }
        
err:
	LOGE("%s:Failed,[chl_cnt=%d] [ret=%d]\n", __func__, scan_num, ret);
 	return NULL;
}

/******************************************
 * Inquiry if RDS TX is support in driver.
 * Parameter:
 *	None
 *Return Value:
 *	1: support
 *	0: NOT support
 *	-1: error
 ******************************************/
jint isRDSTXsupport(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	int support = -1;

	ret = FMR_is_rdstx_support(g_idx, &support);
	if(ret){
		LOGE("%s, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [support=%d] [ret=%d]\n", __func__, support, ret);

	return support;
}

/******************************************
 * Enable/Disable RDS TX
 * Parameter:
 *	Enable:
 		JNI_TRUE : To enable RDS TX
 		JNI_FALSE: To disable RDS TX
 *Return Value:
 *	JNI_TRUE: Success
 *	JNI_FALSE: Failed
 ******************************************/
jboolean setRDSTXEnabled(JNIEnv *env, jobject thiz, jboolean enable)
{
	int ret = 0;
	jboolean jret = JNI_FALSE;
	int onoff = -1;

	if(JNI_FALSE == enable){
		onoff = 0;
	}else if(JNI_TRUE == enable){
		onoff = 1;
	}else{
		LOGE("%s:fail, para error\n", __func__);
		jret = JNI_FALSE;
		goto out;
	}
	ret = FMR_rdstx_onoff(g_idx, onoff);
	if(ret){
		jret = JNI_FALSE;
	}else{
		jret = JNI_TRUE;
	}
out:
	LOGD("%s: [enable=%d] [ret=%d]\n", __func__, onoff, ret);
	return jret;
}

/******************************************
 * This function is used to set RDS data. The blockA is stored in parameter pi, The rest three block(B, C, D) is stored in parameter rds,
 * parameter rds can also contains many pakage.
 * Parameter:
 *	pi: PI code
 *	ps: PS name, make sure it's 8 bytes.
 *	rds: RDS blockB, C and D
 *	rdscnt: The number of pakages papameter rds contains.
 *Return Value:
 *	JNI_TRUE: Success
 *	JNI_FALSE: Failed
 ******************************************/
jboolean setRDSTX(JNIEnv *env, jobject thiz, jshort pi, jcharArray ps, jshortArray rds, jint rdscnt)
{
	int ret = 0;
	jboolean jret = JNI_TRUE;
	uint16_t rds_pi = pi;
	uint8_t *rds_ps = NULL;
	int rds_ps_len = 8;
	uint16_t *rdsdata = NULL;
	int rds_data_len = rdscnt;

	if(ps){
		rds_ps = (unsigned char*) env->GetCharArrayElements(ps, NULL);
		if(rds_ps == NULL){
			LOGE("%s:get ps error\n", __func__);
			jret = JNI_FALSE;
			goto out;
		}
	}else{
		LOGE("%s:NULL java array\n", __func__);
		jret = JNI_FALSE;
		goto out;	
	}

	if(rds){
		rdsdata = (uint16_t*) env->GetShortArrayElements(rds, NULL);
		if(rdsdata == NULL){
			LOGE("%s:get rdsdata error\n", __func__);
			jret = JNI_FALSE;
			goto out;
		}
	}else{
		LOGE("%s:NULL java array\n", __func__);
		jret = JNI_FALSE;
		goto out;	
	}

	ret = FMR_rdstx(g_idx, rds_pi, rds_ps, rds_ps_len, rdsdata, rds_data_len);
	if(ret){
		LOGE("%s:failed, [ret=%d]\n", __func__, ret);
		jret = JNI_FALSE;
	}
	
out:
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return jret;
}

/******************************************
 * FM over BT 
 * Parameter:
 *	Enable:
 		JNI_TRUE : To enable FM over BT
 		JNI_FALSE: To disable FM over BT
 *Return Value:
 *	JNI_TRUE: Success
 *	JNI_FALSE: Failed
 ******************************************/
jboolean setFMViaBTController(JNIEnv *env, jobject thiz, jboolean enable)
{
	int ret = 0;
    int jret = JNI_FALSE;
	int onoff = -1;

	if(JNI_FALSE == enable){
		onoff = 0;
	}else if(JNI_TRUE == enable){
		onoff = 1;
	}else{
		LOGE("%s:fail, para error\n", __func__);
		jret = JNI_FALSE;
		goto out;
	}
	ret = FMR_fm_over_bt(g_idx, onoff);
    if(ret){
        LOGE("%s:fail\n", __func__);
        jret = JNI_FALSE;
    }else{
        jret = JNI_TRUE;
    }
out:
	LOGD("%s: [enable=%d] [ret=%d]\n", __func__, onoff, ret);
	return jret;
}

/******************************************
 * SwitchAntenna 
 * Parameter:
 *	antenna:
 		0 : switch to long antenna
 		1: switch to short antenna
 *Return Value:
 *          0: Success
 *          1: Failed
 *          2: Not support
 ******************************************/
jint switchAntenna(JNIEnv *env, jobject thiz, jint antenna)
{
	int ret = 0;
    jint jret = 0;
	int ana = -1;

	if(0 == antenna){
		ana = FM_LONG_ANA;
	}else if(1 == antenna){
		ana = FM_SHORT_ANA;
	}else{
		LOGE("%s:fail, para error\n", __func__);
		jret = JNI_FALSE;
		goto out;
	}
	ret = FMR_ana_switch(g_idx, ana);
    if(ret == -ERR_UNSUPT_SHORTANA){
        LOGW("Not support switchAntenna\n");
        jret = 2;
    }else if(ret){
        LOGE("switchAntenna(), error\n");
        jret = 1;
    }else{
        jret = 0;
    }
out:
	LOGD("%s: [antenna=%d] [ret=%d]\n", __func__, ana, ret);
	return jret;
}

/******************************************
 * Inquiry if FM is stereoMono.
 * Parameter:
 *	None
 *Return Value:
 *	JNI_TRUE: stereo
 *	JNI_FALSE: mono
 ******************************************/
jboolean stereoMono(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	int stemono = -1;
	
	ret = FMR_get_stereomono(g_idx, &stemono);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [stemono=%d] [ret=%d]\n", __func__, stemono, ret);
	
	return stemono?JNI_TRUE:JNI_FALSE;
}

/******************************************
 * Force set to stero/mono mode.
 * Parameter:
 *	type: JNI_TRUE, mono; JNI_FALSE, stero
 *Return Value:
 *	JNI_TRUE: success
 *	JNI_FALSE: failed
 ******************************************/
jboolean setStereoMono(JNIEnv *env, jobject thiz, jboolean type)
{
	int ret = 0;
	ret = FMR_set_stereomono(g_idx, ((type == JNI_TRUE) ? 1 : 0));
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s,[ret=%d]\n", __func__, ret);
	
	return (ret==0) ? JNI_TRUE : JNI_FALSE;
}

/******************************************
 * Read cap array of short antenna.
 * Parameter:
 *	None
 *Return Value:
 *	CapArray 
 ******************************************/
jshort readCapArray(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	int caparray = -1;
	
	ret = FMR_get_caparray(g_idx, &caparray);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}
	LOGD("%s, [caparray=%d] [ret=%d]\n", __func__, caparray, ret);
	
	return (jshort)caparray;
}

/******************************************
 * Read cap array of short antenna.
 * Parameter:
 *	None
 *Return Value:
 *	CapArray : 0~100
 ******************************************/
jshort readRdsBler(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	int badratio = -1;
	
	ret = FMR_get_badratio(g_idx, &badratio);
	if(ret){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
	}

	if(badratio > 100){
		badratio = 100;
		LOGW("badratio value error, give a max value!");
	}else if(badratio < 0){
		badratio = 0;
		LOGW("badratio value error, give a min value!");
	}
	LOGD("%s, [badratio=%d] [ret=%d]\n", __func__, badratio, ret);
	
	return (jshort)badratio;
}

jintArray getHardwareVersion(JNIEnv *env, jobject thiz)
{
	int ret = 0;
	jintArray hw_info;
	int *info = NULL;
	int info_len = 0;

	ret = FMR_get_hw_info(g_idx, &info, &info_len);
	if(ret < 0){
		LOGE("%s, error, [ret=%d]\n", __func__, ret);
		return NULL;
	}
	hw_info = env->NewIntArray(info_len);
	env->SetIntArrayRegion(hw_info, 0, info_len, (const jint*)info);
	LOGD("%s, [ret=%d]\n", __func__, ret);
	return hw_info; 
}

static const char *classPathNameRx = "com/mediatek/FMRadio/FMRadioNative";
static const char *classPathNameTx = "com/mediatek/FMTransmitter/FMTransmitterNative";


static JNINativeMethod methodsRx[] = {
  {"opendev", "()Z", (void*)opendev },  //1
  {"closedev", "()Z", (void*)closedev }, //2
  {"powerup", "(F)Z", (void*)powerup },  //3
  {"powerdown", "(I)Z", (void*)powerdown }, //4
  {"tune", "(F)Z", (void*)tune },          //5
  {"seek", "(FZ)F", (void*)seek },         //6
  {"autoscan",  "()[S", (void*)autoscan }, //7
  {"stopscan",  "()Z", (void*)stopscan },  //8
  {"readRssi",   "()I", (void*)readRssi },   //9
  {"rdsset",    "(Z)I", (void*)rdsset  },  //10
  {"readrds",   "()S", (void*)readrds },  //11 will pending here for get event status
  {"getPI",     "()S", (void*)getPI   },  //12
  {"getPTY",    "()B", (void*)getPTY  },  //13
  {"getPS",     "()[B", (void*)getPS  },  //14
  {"getLRText", "()[B", (void*)getLRText}, //15
  {"activeAF",  "()S", (void*)activeAF},   //16
  {"getAFList", "()[S", (void*)getAFList }, //17 not need show actually
  {"activeTA",  "()S", (void*)activeTA},    //18
  {"deactiveTA","()S", (void*)deactiveTA},  //19
  {"setmute",	"(Z)I", (void*)setmute},  //20
  {"getchipid",	"()I", (void*)getchipid},  //21
  {"isRDSsupport",	"()I", (void*)isRDSsupport},  //22
  {"isFMPoweredUp", "()I", (void*)isFMPoweredUp}, //23
  {"setFMViaBTController", "(Z)Z", (void*)setFMViaBTController}, //24
  {"switchAntenna", "(I)I", (void*)switchAntenna}, //25
  {"stereoMono", "()Z", (void*)stereoMono}, //26
  {"readCapArray", "()S", (void*)readCapArray}, //27
  {"readRdsBler", "()S", (void*)readRdsBler}, //28
  {"setStereoMono", "(Z)Z", (void*)setStereoMono}, //29
  {"getHardwareVersion", "()[I", (void*)getHardwareVersion}, //30
    {"scannew", "(III)[S", (void*)scannew}, //31
    {"seeknew", "(IIIIII)I", (void*)seeknew}, //32
    {"tunenew", "(IIII)Z", (void*)tunenew}, //33
	{"emsetth","(II)Z",(void*)emsetth},//34 
	{"emcmd","([S)[S",(void*)emcmd},//35 
};

static JNINativeMethod methodsTx[] = {
  {"opendev", "()Z", (void*)opendev },  //1
  {"closedev", "()Z", (void*)closedev }, //2
  {"powerup", "(F)Z", (void*)powerup },  //3
  {"powerdown", "(I)Z", (void*)powerdown }, //4
  {"tune", "(F)Z", (void*)tune },          //5
  {"seek", "(FZ)F", (void*)seek },         //6
  {"autoscan",  "()[S", (void*)autoscan }, //7
  {"stopscan",  "()Z", (void*)stopscan },  //8
  //{"getrssi",   "()I", (void*)getrssi },   //9
  {"rdsset",    "(Z)I", (void*)rdsset  },  //10
  {"readrds",   "()S", (void*)readrds },  //11 will pending here for get event status
  //{"getPI",     "()S", (void*)getPI   },  //12
  //{"getPTY",    "()B", (void*)getPTY  },  //13
  {"getPS",     "()[B", (void*)getPS  },  //14
  {"getLRText", "()[B", (void*)getLRText}, //15
  {"activeAF",  "()S", (void*)activeAF},   //16
  //{"getAFList", "()[S", (void*)getAFList }, //17 not need show actually
  {"activeTA",  "()S", (void*)activeTA},    //18
  {"deactiveTA","()S", (void*)deactiveTA},  //19
  {"setmute",	"(Z)I", (void*)setmute},  //20
  //{"getchipid",	"()I", (void*)getchipid},  //21
  {"isRDSsupport",	"()I", (void*)isRDSsupport},  //23
  {"isTXSupport",	"()I", (void*)isTXSupport},  //24
  {"powerupTX",	"(F)Z", (void*)powerupTX},  //25
  {"getTXFreqList",  "(FII)[S", (void*)getTXFreqList }, //26
  {"isRDSTXSupport",	"()I", (void*)isRDSTXsupport},  //27
  {"setRDSTXEnabled",	"(Z)Z", (void*)setRDSTXEnabled},  //28
  {"setRDSTX",	"(S[C[SI)Z", (void*)setRDSTX},  //29
  {"tuneTX", "(F)Z", (void*)tuneTX}, //30
};


/*
 * Register several native methods for one class.
 */
static jint registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
	if(env->ExceptionCheck()){
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

	LOGD("%s, success\n", __func__);
    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static jint registerNatives(JNIEnv* env)
{
	jint ret = JNI_FALSE;
	
	if (registerNativeMethods(env, classPathNameRx,methodsRx, 
								sizeof(methodsRx) / sizeof(methodsRx[0]))){
		ret = JNI_TRUE;
	}

	if (registerNativeMethods(env, classPathNameTx,methodsTx, 
									sizeof(methodsTx) / sizeof(methodsTx[0]))){
		ret = JNI_TRUE;
	}

	LOGD("%s, done\n", __func__);
	return ret;
}


// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */
 
typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;

    LOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        goto bail;
    }

	if ((g_idx = FMR_init()) < 0) {
		goto bail;
	}
    result = JNI_VERSION_1_4;

bail:
    return result;
}
