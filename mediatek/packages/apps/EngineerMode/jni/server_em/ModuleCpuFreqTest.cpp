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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "ModuleCpuFreqTest.h"
#include "RPCClient.h"

int ModuleCpuFreqTest::bRunning = 0;
int ModuleCpuFreqTest::iCurrentSpeed = 0;

static void *run_thr(void *p)
{
	RPCClient* msgSender = (RPCClient*)p;    
	
	const char *device_file = "/sys/bus/platform/drivers/arm_pwr_test/arm_pwr_test_gui";
	int fd = -1;
	size_t s = 0;
	
	char result[PAGE_SIZE];
	char freq[10];
	
	int i;

	for(i=ModuleCpuFreqTest::iCurrentSpeed; i<ModuleCpuFreqTest::iCurrentSpeed+1; i++)// step by step --> 520 ~ 806 MHZ
	{
		if(ModuleCpuFreqTest::bRunning == 0)
		{
			break;
		}
		fd = open(device_file, O_RDWR);// open device file
		if(fd < 0)
		{
			msgSender->PostMsg((char*)"fail to open device");
			return 0;
		}  
		sprintf(freq, "%d", i);
		write(fd, freq, strlen(freq)); // start test
		s = read(fd, (void*)result, PAGE_SIZE); // read back result
		if(s <= 0)
		{
			msgSender->PostMsg((char*)"could not read arm_pwr_test sys file\n");
			return 0;
		}
		 //output to screen here...
		msgSender->PostMsg(result);
		close(fd);		
	}
	
	
    pthread_exit(NULL);    
    return NULL;
}

int ModuleCpuFreqTest::StartTest(RPCClient* msgSender)
{
	int paraNum = msgSender->ReadInt();		
	if(1 != paraNum)
	{
		msgSender->PostMsg((char*)"Parameter number is not 1");		
		return -1;
	}
	int idx = 0;
	for(idx=0; idx<paraNum; idx++)
	{
		int T = msgSender->ReadInt();
		if(T != PARAM_TYPE_INT)
		{
			//error
			return -1;
		}
		int L = msgSender->ReadInt();
		int V = msgSender->ReadInt();
		if(V>23 ||V<0)
		{
			msgSender->PostMsg((char*)"Parameter range [0,23)");	
			return -1;
		}
		iCurrentSpeed = V;	
	}

	if(bRunning != 0) //first time to run
	{
		msgSender->PostMsg((char*)"Still running, retry later.");	
		return -1;
	}	

	bRunning = 1;
	pthread_t tid = 0;
	pthread_create(&tid, NULL, run_thr, (void*)msgSender);
    
    pthread_join(tid, NULL);
	
	bRunning = 0;
	return 0;
}




int ModuleCpuFreqTest::StopTest(RPCClient* msgSender)
{
	bRunning = 0;
	iCurrentSpeed = 0;
	int paraNum = msgSender->ReadInt();		
	if(0 != paraNum)
	{
		msgSender->PostMsg((char*)"Too many parameters");		
		return -1;
	}

	
	return 0;
}

int ModuleCpuFreqTest::SetCurrent(RPCClient* msgSender)
{

	int paraNum = msgSender->ReadInt();		
	if(0 != paraNum)
	{
		msgSender->PostMsg((char*)"Too many parameters");		
		return -1;
	}
	
	const char *device_file_gui = "/sys/bus/platform/drivers/arm_pwr_test/arm_pwr_test_gui";
	int fd = open(device_file_gui, O_RDWR);// open device file
	if(fd < 0)
	{
		msgSender->PostMsg((char*)"fail to open device_gui");
		return 0;
	} 
	
	char freq[10];
	sprintf(freq, "%d", 23);
	write(fd, freq, strlen(freq)); // start test
	
	return 0;
}