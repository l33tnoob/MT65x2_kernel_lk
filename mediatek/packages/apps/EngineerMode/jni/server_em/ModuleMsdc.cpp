/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "EMMSDC"

#include <cutils/xlog.h>
 
#include <jni.h>
#include <linux/mmc/sd_misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "ModuleMsdc.h"
#include "RPCClient.h"

using namespace android;

/* define host max index */
#define MSDC_MAX_HOST_MUM msdc_max_host_num()
#define ERROR "ERROR"
#define SUCCESS "SUCCESS"
#define RESULT_SIZE 100

static int driving[MSDC_ODC_COUNT] = {
    	MSDC_ODC_4MA, 
    	MSDC_ODC_8MA, 
    	MSDC_ODC_12MA, 
    	MSDC_ODC_16MA
};
static int msdc_max_host_num() {
    int chipid = em_jni_get_chip_id();
    /* UI selection confirm the max value correct, here just for compatible */
    if (chipid < MTK_6589_SUPPORT) {
        return 3;
    } else {
        return 4;
    }
}

int ModuleMsdc::setCurrent(RPCClient* msgSender) {
    int paraNum = msgSender->ReadInt();
    int param[10];
    if (paraNum != 10) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	for(int i = 0; i < 10; i++){
    	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
    		msgSender->PostMsg((char*) ERROR);
    		return -1;
    	}
    	msgSender->ReadInt();
    	param[i] = msgSender->ReadInt();
    }
	int ret = setCurrentInternal(param[0], param[1], param[2], param[3], param[4], param[5],
	    param[6], param[7], param[8], param[9]);
	    
	if (ret == -1) {
	    msgSender->PostMsg((char*) ERROR);
        return -1;
	} else {
	    msgSender->PostMsg((char*) SUCCESS);
	    return 1;
	}
}

int ModuleMsdc::setCurrentInternal(int hostNum, int clkPU, int clkPD, int cmdPU, 
	int cmdPD, int datPU, int datPD, int hopBit , int hopTime,int opcode)
{
	if(hostNum > MSDC_MAX_HOST_MUM || clkPU >7 || clkPD >7 || cmdPU >7 || cmdPD >7 || datPU >7 || datPD >7 || hopBit >3 || hopTime > 5)
	{
		XLOGD("----error: hostNum or currentIdx too large.\n");
		return -1;
	}

	int sd_fd, ret;
	struct msdc_ioctl command;

	sd_fd = open("/dev/misc-sd", O_RDONLY); 
	if(sd_fd < 0)
	{
		XLOGD("----error: can't open misc-sd----, error code:%d\n", sd_fd);
		return -1;
	}
	XLOGD("set: clk_pu=%d,  clk_pd = %d, cmd_pu=%d,  cmd_pd = %d, dat_pu=%d,  dat_pd = %d\r\n",
			clkPU, clkPD, cmdPU, cmdPD, datPU, datPD);
	XLOGD("set: l_hopBitIdx=%d,  l_hopTimeIdx = %d, opcode = %d\r\n",hopBit, hopTime, opcode);
	if(opcode == 0)
	{
	    command.opcode = MSDC_DRIVING_SETTING;
	}else
	{
	    command.opcode = MSDC_HOPPING_SETTING;
	}
	command.host_num =hostNum; //0~3
	command.clk_pu_driving = clkPU;
	command.clk_pd_driving = clkPD;
	command.cmd_pu_driving = cmdPU;
	command.cmd_pd_driving = cmdPD;
	command.dat_pu_driving = datPU;
	command.dat_pd_driving = datPD;
	command.hopping_bit = hopBit;
	command.hopping_time = hopTime;
	command.iswrite =1; //0: read, 1:write
	command.clock_freq =0;
	command.result = -1;
	ret = ioctl(sd_fd, -1, (void *)&command);

	if(ret < 0 || -1 == command.result)
	{
		XLOGD("----error: can't call misc-sd----, error:%d, fd:%d\n",ret, sd_fd);
		return -1;
	}

	close(sd_fd);

	return 1;
}

int ModuleMsdc::getCurrent(RPCClient* msgSender) {
    int paraNum = msgSender->ReadInt();
    if (paraNum != 2) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	int hostNum, opcode;
	
	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
    }
    msgSender->ReadInt();
    hostNum = msgSender->ReadInt();
    if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
    }
    msgSender->ReadInt();
    opcode = msgSender->ReadInt();
    
    int ret = getCurrentInternal(hostNum, opcode);
    
    if(ret == -1) {
        msgSender->PostMsg((char*) ERROR);
		return -1;
	} else {
	    char result[RESULT_SIZE] = { 0 };
    	sprintf(result, "%d", ret);
    	msgSender->PostMsg(result);
    	return 1;
	}
}
int ModuleMsdc::getCurrentInternal(int hostNum, int opcode)
{
    int chipid; 
	if (hostNum > MSDC_MAX_HOST_MUM)
	{
		XLOGD("----error: hostNum or currentIdx too large.\n");
		return -1;
	}

	int sd_fd, ret, idx;
	struct msdc_ioctl command;

    XLOGD("1 before open misc-sd\n");
	sd_fd = open("/dev/misc-sd", O_RDONLY);     
	if(sd_fd < 0)
	{
		XLOGD("----error: 1 can't open misc-sd----, error code:%d\n", sd_fd);
		return -1;
	}
    XLOGD("2 after open misc-sd:%d\n", sd_fd);
    if(opcode == 0)
    {
	    command.opcode = MSDC_DRIVING_SETTING;
    }else{
        command.opcode = MSDC_HOPPING_SETTING;
    }
	command.host_num =hostNum; //0~3
	command.clk_pu_driving = 0xF;
	command.clk_pd_driving = 0xF;
	command.cmd_pu_driving = 0xF;
	command.cmd_pd_driving = 0xF;
	command.dat_pu_driving = 0xF;
	command.dat_pd_driving = 0xF;
	command.hopping_bit = 0xF;
	command.hopping_time = 0xF;
	command.iswrite =0; //0: read, 1:write
	command.clock_freq =0;
	command.result = -1;

	ret = ioctl(sd_fd, -1, (void *)&command);
    XLOGD("3 after ioctl\n");
	if(ret < 0 || -1 == command.result)
	{
		XLOGD("----error: can't call misc-sd----, return:%d, fd:%d\n",ret, sd_fd);
		return -1;
	}

	close(sd_fd);

	int l_clkIdx_pu = 0xF;
	int l_clkIdx_pd = 0xF;
	int l_cmdIdx_pu = 0xF;
	int l_cmdIdx_pd = 0xF;
	int l_datIdx_pu = 0xF;
	int l_datIdx_pd = 0xF;
    int l_hopBitIdx = 0xF;
    int l_hopTimeIdx = 0xF;
	l_clkIdx_pu = command.clk_pu_driving;
	l_clkIdx_pd = command.clk_pd_driving;
	l_cmdIdx_pu = command.cmd_pu_driving;
	l_cmdIdx_pd = command.cmd_pd_driving;
	l_datIdx_pu = command.dat_pu_driving;
	l_datIdx_pd = command.dat_pd_driving;
	l_hopBitIdx = command.hopping_bit;
	l_hopTimeIdx = command.hopping_time;
    if(opcode == 0){
      chipid = em_jni_get_chip_id();

	  if (chipid >= MTK_6589_SUPPORT) {
	      if (l_clkIdx_pu != 0xF && l_cmdIdx_pu != 0xF && l_datIdx_pu != 0xF)
	  	  {
	  	      XLOGD("6589 success: clk_pu=%d,  clk_pd = %d, cmd_pu=%d,  cmd_pd = %d, dat_pu=%d,  dat_pd = %d\r\n",
	  				l_clkIdx_pu, l_clkIdx_pd, l_cmdIdx_pu, l_cmdIdx_pd, l_datIdx_pu, l_datIdx_pd);
	  				return ((l_datIdx_pd << 20) | (l_datIdx_pu << 16) | (l_cmdIdx_pd << 12) | (l_cmdIdx_pu << 8) | (l_clkIdx_pd << 4) | l_clkIdx_pu);
	  	  } else {
		      XLOGD("error: clk_pu=%d,  clk_pd = %d, cmd_pu=%d,  cmd_pd = %d, dat_pu=%d,  dat_pd = %d\r\n", 
				l_clkIdx_pu, l_clkIdx_pd, l_cmdIdx_pu, l_cmdIdx_pd, l_datIdx_pu, l_datIdx_pd);
		      return -1;
	      }
	  }	else {
	      if (l_clkIdx_pu != 0xF && l_clkIdx_pd != 0xF &&
			l_cmdIdx_pu != 0xF && l_cmdIdx_pd != 0xF &&
			l_datIdx_pu != 0xF && l_datIdx_pd != 0xF )
	      {
		      XLOGD("success: clk_pu=%d,  clk_pd = %d, cmd_pu=%d,  cmd_pd = %d, dat_pu=%d,  dat_pd = %d\r\n", 
				l_clkIdx_pu, l_clkIdx_pd, l_cmdIdx_pu, l_cmdIdx_pd, l_datIdx_pu, l_datIdx_pd);
				return ((l_datIdx_pd << 20) | (l_datIdx_pu << 16) | (l_cmdIdx_pd << 12) | (l_cmdIdx_pu << 8) | (l_clkIdx_pd << 4) | l_clkIdx_pu);
	      } else {
		      XLOGD("error: clk_pu=%d,  clk_pd = %d, cmd_pu=%d,  cmd_pd = %d, dat_pu=%d,  dat_pd = %d\r\n", 
				l_clkIdx_pu, l_clkIdx_pd, l_cmdIdx_pu, l_cmdIdx_pd, l_datIdx_pu, l_datIdx_pd);
		      return -1;
	    }
	  }

	}else {
			if (l_hopBitIdx !=0xF && l_hopTimeIdx != 0xF)
			{
				XLOGD("success: l_hopBitIdx=%d,  l_hopTimeIdx = %d\r\n",l_hopBitIdx, l_hopTimeIdx);
				return (l_hopTimeIdx << 28)|(l_hopBitIdx << 24);
			}
			else
			{
				XLOGD("error: l_hopBitIdx=%d,  l_hopTimeIdx = %d\r\n",l_hopBitIdx, l_hopTimeIdx);
				return -1;
			}
	}
}


int ModuleMsdc::setSd30Mode(RPCClient* msgSender)
{
    int paraNum = msgSender->ReadInt();
    int param[5];
    if (paraNum != 5) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	for(int i = 0; i < 5; i++){
    	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
    		msgSender->PostMsg((char*) ERROR);
    		return -1;
    	}
    	msgSender->ReadInt();
    	param[i] = msgSender->ReadInt();
    }
    int ret = setSd30ModeInternal(param[0], param[1], param[2], param[3], param[4]);
    
	if (ret == -1) {
	    msgSender->PostMsg((char*) ERROR);
        return -1;
	} else {
	    msgSender->PostMsg((char*) SUCCESS);
	    return 1;
	}
}

int ModuleMsdc::setSd30ModeInternal(int hostNum, int sd30Mode, int sd30MaxCurrent, int sd30Drive, int sd30PowerControl)
{
	XLOGD("hostNum=%d,  sd30Mode = %d, sd30MaxCurrent=%d,  sd30Drive = %d, sd30PowerControl=%d",
			hostNum, sd30Mode, sd30MaxCurrent, sd30Drive, sd30PowerControl);
	if(hostNum > MSDC_MAX_HOST_MUM || sd30Mode > 5 || sd30MaxCurrent > 3 || sd30Drive > 3 || sd30PowerControl > 1)
	{
		XLOGD("----error: hostNum or index error.\n");
		return -1;
	}
	int sd_fd, ret;
	struct msdc_ioctl command;
	sd_fd = open("/dev/misc-sd", O_RDONLY);
	if(sd_fd < 0)
	{
		XLOGD("----error: can't open misc-sd----, error code:%d\n", sd_fd);
		return -1;
	}
	command.host_num = hostNum;
	command.opcode = MSDC_SD30_MODE_SWITCH;
	command.sd30_mode = sd30Mode;
	command.sd30_max_current = sd30MaxCurrent;
	command.sd30_drive = sd30Drive;
	command.sd30_power_control = sd30PowerControl;
	command.result = -1;
	ret = ioctl(sd_fd, -1, (void *)&command);
	if(ret < 0 || -1 == command.result)
	{
		XLOGD("----error: can't call misc-sd----, error:%d, fd:%d\n",ret, sd_fd);
		return -1;
	}
	close(sd_fd);
	return 1;
}

