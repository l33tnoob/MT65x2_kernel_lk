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

/*******************************************************************************
* Nand_powerloss_test.c MVG_Test Module                                                     
*                                                                                             
* Copyright (c) 2010, Media Teck.inc                                           
*                                                                             
* This program is free software; you can redistribute it and/or modify it     
* under the terms and conditions of the GNU General Public Licence,            
* version 2, as publish by the Free Software Foundation.                       
*                                                                              
* This program is distributed and in hope it will be useful, but WITHOUT       
* ANY WARRNTY; without even the implied warranty of MERCHANTABITLITY or        
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for     
* more details.                                                                
*                                                                              
*                                                                              
********************************************************************************
* Author : Jun Shen (jun.shen@mediatek.com)                              
********************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <errno.h>
#include "MVG_Powerloss_App.h"

#define LOG_TAG		"MVG_LOG"
#include <cutils/log.h>

#define SYSTEM_ERR_REPPORT

#define DATA_ROOT_TEST
#ifdef DATA_ROOT_TEST
#define TEST_ROOT_PATH		"/data/mvg_root"
#define TEST_TIMES_COUNTER_FILE		"/data/mvg_root/powerlost_times.txt"
#define TEST_DATA_FILE			"/data/mvg_root/powerlost_data.dat"
#define TEST_DATA_FILE_LARGE			"/data/mvg_root/pwrloststdata.dat"
#define TEST_PATH			"/data/mvg_root/mvgtest_folder"
#define TEST_FILE			"/data/mvg_root/mvgtest_folder/test.txt"
#else
#define TEST_TIMES_COUNTER_FILE		"/data/powerlost_times.txt"
#define TEST_DATA_FILE			"/data/powerlost_data.dat"
#define TEST_DATA_FILE_LARGE			"/data/pwrloststdata.dat"
#define TEST_PATH			"/data/mvgtest_folder"
#define TEST_FILE			"/data/mvgtest_folder/test.txt"
#endif

#define TEST_TIMES_LENGTH	5
#define TEST_DATA_FILE_SIZE	10000		//Real size is TEST_DATA_FILE_SIZE*32 Bytes

extern volatile int errno;

static int g_PowerOffTimes = 0;
static int dev_open_fp = 0;

char err_tempbuf[22] = {0};


int BootTimesCheck()
{
	int i = 0;
	int ret =0;
	int fopCnt = 0;
	FILE *fp = NULL;
	int poweroff_times = 0;
	char Inbuffer[32] = {0};
	char Outbuffer[32] = {0};
	char print_buff[32] = {0};
	
#ifdef DATA_ROOT_TEST
	static int folder_check = 1;
	if(folder_check)
	{
		folder_check = 0;
		mkdir(TEST_ROOT_PATH, 0777);
	}
#endif

	fp = fopen(TEST_TIMES_COUNTER_FILE, "rt");
	if(!fp)
	{
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST OpenCntFileR Failed\n\0");
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif		
		ALOGE("MVG_TEST BootTimesCheck Create powerlost_data.dat !\n");
		fp = fopen(TEST_DATA_FILE, "rb");
		if(!fp)
		{
			memset(print_buff,0,32);
			sprintf(print_buff, "MVG_TEST OpenDatFile Failed\n\0");
			ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif	
			fp = fopen(TEST_DATA_FILE, "wb+");
			if(!fp)
			{
				memset(print_buff,0,32);
				sprintf(print_buff, "MVG_TEST CreatDatFile Failed\n\0");
				ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif	
				ALOGE("MVG_TEST BootTimesCheck Create powerlost_data.dat Failed!\n");
				return -2;
			}
			for(i=0;i<32;i++)
			{
				Inbuffer[i] = i;
			}
			for(i=0;i<TEST_DATA_FILE_SIZE;i++)
			{
				fopCnt = fwrite(Inbuffer, 32, 1, fp);
				ALOGE("MVG_TEST BootTimesCheck Write powerlost_data.dat, data num is:%d",fopCnt);
			}
		}
		fclose(fp);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST CreatDatFile OK\n\0");
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);

		ALOGE("MVG_TEST BootTimesCheck Create powerlost_times.txt !\n");
		fp = fopen(TEST_TIMES_COUNTER_FILE, "wt+");
		if(!fp)
		{
			memset(print_buff,0,32);
			sprintf(print_buff, "MVG_TEST CreatCntFile Failed\n\0");
			ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif
			ALOGE("MVG_TEST BootTimesCheck Create powerlost_times.txt Failed!\n");
			return -1;
		}
		poweroff_times = 0;
		sprintf(Inbuffer, "%5d\n",poweroff_times);
		fopCnt = fwrite(Inbuffer, TEST_TIMES_LENGTH+1, 1, fp);
		ALOGE("MVG_TEST BootTimesCheck Write '0' to powerlost_times.txt char number is: %d!\n", fopCnt);
		fclose(fp);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST CreatCntFile OK\n\0");
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
	}
	else
	{
		fopCnt = fread(Outbuffer, TEST_TIMES_LENGTH, 1, fp);
		ALOGE("MVG_TEST BootTimesCheck Read powerlost_times.txt char number is: %d!\n", fopCnt);
		fclose(fp);
		Outbuffer[TEST_TIMES_LENGTH] = '\0';
		poweroff_times = atoi(Outbuffer);
		g_PowerOffTimes = poweroff_times;
		ALOGE("MVG_TEST ----------Test has passed %d times!-------\n", poweroff_times);
		ret = ioctl(dev_open_fp, PRINT_REBOOT_TIMES, (unsigned long)(&g_PowerOffTimes));
		if(ret)
		{
			ALOGE("MVG_TEST dev file ioctl PRINT_REBOOT_TIMES operation  --------Error!");
		}
		if(poweroff_times > 2000)
		{
			ALOGE("MVG_TEST--------------MVG_TEST Test passed!---------------\n");
		}
		poweroff_times ++;
		sprintf(Inbuffer, "%5d\n",poweroff_times);
		fp = fopen(TEST_TIMES_COUNTER_FILE, "wt+");
		if(!fp)
		{
			memset(print_buff,0,32);
			sprintf(print_buff, "MVG_TEST OpenCntFileW Failed\n\0");
			ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif
			if(ret)
			{
				ALOGE("MVG_TEST dev file ioctl PRINT_REBOOT_TIMES operation  --------Error!");
			}
			ALOGE("MVG_TEST BootTimesCheck Open powerlost_times.txt Failed!\n");
			return -1;
		}
		fopCnt = fwrite(Inbuffer, TEST_TIMES_LENGTH+1, 1, fp);
		ALOGE("MVG_TEST BootTimesCheck Write powerlost_times.txt char number is: %d!\n", fopCnt);
		fclose(fp);
	}
	return 0;
}

int DataFileVerification()
{
	int i = 0;
	int j = 0;
	int ret =0;
	int fopCnt = 0;
	FILE *fp = NULL;
	char Outbuffer[32] = {0};
	char print_buff[32] = {0};
	fp = fopen(TEST_DATA_FILE, "rb");
	if(!fp)
	{
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST OpenDatFileR Failed\n\0");
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif
		if(ret)
		{
			ALOGE("MVG_TEST dev file ioctl PRINT_FILE_OPERATION_ERR operation  --------Error!");
		}
		ALOGE("MVG_TEST DataFileVerification Open powerlost_data.dat Failed!\n");
		return -1;
	}
	for(i=0; i<TEST_DATA_FILE_SIZE; i++ )
	{
		memset(Outbuffer,0,32);
		fopCnt = fread(Outbuffer, 32, 1, fp);
		//ALOGE("MVG_TEST DataFileVerification Read powerlost_data.dat, data num is:%d",fopCnt);
		for(j=0; j<32; j++)
		{
			if(Outbuffer[j] != j)
			{
				ret = ioctl(dev_open_fp, PRINT_DATA_COMPARE_ERR, NULL);
				if(ret == -1)
				{
					ALOGE("MVG_TEST dev file ioctl PRINT_DATA_COMPARE_ERR operation  --------Error!");
				}
				ALOGE("MVG_TEST DadaFileVerification File Compare Error! i=%d, j=%d",i,j);
				return -1;
			}		
		}	
	}
	ALOGE("MVG_TEST ----------------Test has passed %d times!--------------\n", g_PowerOffTimes);
	ret = 0;
	return ret;
}

int Test_1()
{
	int ret;
	int i = 0;
	int j = 0;
	int fopCnt = 0;
	FILE *fp = NULL;
	char print_buff[32] = {0};
	char Inbuffer[256] ={0};
	char Outbuffer[256] = {0};
	
	for(i=0; i<256; i++)
	{
		Inbuffer[i] = i%256;
	}
	mkdir(TEST_PATH, 0777);
	fp = fopen(TEST_FILE, "at+");
	if(!fp)
	{
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST Test_1 CretFileFail\n\0");
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif

		ALOGE("MVG_TEST Test_1 Create Test.txt Failed!\n");
		return -1;
	}
	for(i=0; i<20000; i++)
	{
		fopCnt = fwrite(Inbuffer, 1, 256, fp);
	}
	ALOGE("MVG_TEST Test_1 Write Test.txt char number is: %d!\n", fopCnt*20000);
	fclose(fp);
	
	fp = fopen(TEST_FILE, "at+");
	if(!fp)
	{
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST Test_1 OpenFileFail\n\0");
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif

		ALOGE("MVG_TEST Test_1 Open Test.txt Failed!\n");
		return -1;
	}
	for(i=0; i<20000; i++)
	{
		memset(Outbuffer, 0, 256);
		fopCnt = fread(Outbuffer, 1, 256, fp);
		for(j=0; j<fopCnt; j++)
		{
			if(Inbuffer[j] != Outbuffer[j])
			{
				memset(print_buff,0,32);
				sprintf(print_buff, "MVG_TEST Test1CMP Failed\n\0");
				ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
				ALOGE("MVG_TEST Test_1 data%d compare ERROR!\n", j);
				return -1;
			}
		}
	}
	ALOGE("MVG_TEST Test_1 data compare Successful!\n");
	fclose(fp);

	ret = remove(TEST_FILE);
	if(ret != 0)
	{
		ALOGE("MVG_TEST Test_1 Delete Test file Failed, Err Code: %d\n",ret);
		return ret;
	}
	ret = rmdir(TEST_PATH);
	if(ret != 0)
	{
		ALOGE("MVG_TEST Test_1 Delete Test Folder Failed, Err Code: %d\n",ret);
		return ret;
	}
	ret = 0;
	return ret;
}

int Test_2()				//need more test case
{
	int ret = 0;
	int i = 0;
	static FILE *fp = NULL;
	static unsigned int datacnt = 0;
	static struct statfs fs_status;
	int fopCnt = 0;
	unsigned char data[256] = {0};
	char print_buff[32] = {0};
	for(i=0;i<256;i++)
	{
		data[i] = i;
	}
	if(!fp)
	{
		fp = fopen(TEST_DATA_FILE_LARGE, "wb+");
	}
	if(!fp)
	{
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST Test_2 OpenFileFail\n\0");
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
		memset(err_tempbuf,0,22);
		strerror_r(errno,err_tempbuf, 21);
		memset(print_buff,0,32);
		sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
		ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif
		ALOGE("MVG_TEST Test_2 Open pwrloststdata.txt Failed!\n");
		return -1;
	}
	for(i=0;i<100;i++)
	{
		fopCnt = fwrite(data, 1, 256, fp);
		if(fopCnt<256)
		{
			memset(print_buff,0,32);
			sprintf(print_buff, "MVG_TEST Test2 fopCnt=%d\n\0",fopCnt);
			ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
			return -1;
		}
		datacnt += 256;		
		if(datacnt >= 20000000)
		{
			fclose(fp);
			remove(TEST_DATA_FILE_LARGE);
			datacnt = 0;
			fp = fopen(TEST_DATA_FILE_LARGE, "wb+");
			if(!fp)
			{
				memset(print_buff,0,32);
				sprintf(print_buff, "MVG_TEST Test_2 CretFileFail\n\0");
				ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#ifdef SYSTEM_ERR_REPPORT
				memset(print_buff,0,32);
				sprintf(print_buff, "MVG_TEST errno=0x%x \n\0",errno);
				ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
				memset(err_tempbuf,0,22);
				strerror_r(errno,err_tempbuf, 21);
				memset(print_buff,0,32);
				sprintf(print_buff, "MVG_TEST:%s\n\0",err_tempbuf);
				ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
#endif
				ALOGE("MVG_TEST Test_2 Open pwrloststdata.txt Failed!\n");
				return -1;
			}
		}
	}
	ALOGE("MVG_TEST Test_2 Write pwrloststdata.txt char number is: %d!\n", fopCnt*100);
	//fclose(fp);
	return ret;
}

int main(void)
{
	int ret = 0;
	int i = 0;
	char print_buff[32] = {0};
	dev_open_fp = open("/dev/power_loss_test", O_RDWR);
	if(dev_open_fp < 0)
	{
		ALOGE("MVG_TEST Open nand_mvp dev file Failed!dev_open_fp=%d !\n",dev_open_fp);
		return -1;
	}
	ALOGE("MVG_TEST dev file open, dev_open_fp=%d\n",dev_open_fp);
	sprintf(print_buff, "MVG_TEST Dev File Open OK!\n\0");
	ret = ioctl(dev_open_fp, PRINT_GENERAL_INFO, (unsigned long)print_buff);
	if(ret == -1)
	{
		ALOGE("MVG_TEST dev file ioctl PRINT_GENERAL_INFO operation  --------Error!ret=%d\n",ret);
	}
	ret = BootTimesCheck();
	if(ret == 0)
	{
		ALOGE("MVG_TEST BootTimesCheck ------------OK!\n");
	}
	else
	{
		ALOGE("MVG_TEST BootTimesCheck ------------Error!\n");
	}
	ret = DataFileVerification();
	if(ret == 0)
	{	
		ALOGE("MVG_TEST DataFileVerification -----------OK!\n");
	}
	else
	{
		ALOGE("MVG_TEST DataFileVerification -----------Error!\n");
	}
	while(1)
	{
		i = 1000;
		while(i--)
		{
			ret  = Test_1();
			if(ret != 0)
			{
				ALOGE("MVG_TEST Test_1 ------------- Failed!\n");
			}
			ret = Test_2();
			if(ret != 0)
			{
				ALOGE("MVG_TEST Test_2 ------------ Failed!\n");
			}
		}
		sleep(1);
	}
	return 0;
}

