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

#define LOG_TAG "EMCPUSTRESS"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/xlog.h>
#include <chip_support.h>
#include "ModuleCpuStress.h"
#include "RPCClient.h"

#ifndef SCHED_IDLE
#define SCHED_IDLE 5
#endif

void * apmcu_test(void * argvoid) {
	struct thread_params_t * arg = (struct thread_params_t *) argvoid;
	int fd = -1;
	char value[10] = { 0 };
	size_t s = 0;
	do {
		fd = open(arg->file, O_RDWR);
		XLOGD("open file: %s", arg->file);
		if (fd < 0) {
			snprintf(arg->result, sizeof(arg->result), "%s",
					"fail to open device");
			XLOGE("fail to open device");
			break;
		}
		snprintf(value, sizeof(value), "%d", 1);
		write(fd, value, strlen(value));
		lseek(fd, 0, SEEK_SET);
		s = read(fd, arg->result, sizeof(arg->result));
		if (s <= 0) {
			snprintf(arg->result, sizeof(arg->result), "%s",
					"could not read response");
			break;
		}
	} while (0);
		if (fd >= 0) {
			close(fd);
		}
	pthread_exit(NULL);
	return NULL;
}

void * swcodec_test(void * argvoid) {
	struct thread_params_t * arg = (struct thread_params_t *) argvoid;
	int tid = gettid();
	XLOGD("tid: %d, Enter swcodec_test: file: %s", tid, arg->file);
	FILE * fp;
	struct timeval timeout;
	struct timeval delay;
	delay.tv_sec = 0;
	delay.tv_usec = 100 * 1000;
	do {
		pthread_mutex_lock(&lock);
		fp = popen(arg->file, "r");
		pthread_mutex_unlock(&lock);
		select(0, NULL, NULL, NULL, &delay);
		if (fp == NULL) {
			XLOGE("popen fail: %s, errno: %d", arg->file, errno);
			strcpy(arg->result, "POPEN FAIL\n");
			break;
		}
		char *ret;
		while(1) {
			pthread_mutex_lock(&lock);
			ret = fgets(arg->result, sizeof(arg->result), fp);
			pthread_mutex_unlock(&lock);
			select(0, NULL, NULL, NULL, &delay);
			if (ret == NULL) {
				XLOGD("tid: %d, get result is null", tid);
				break;
			}
		}
	} while(0);
	if (fp != NULL) {
		pthread_mutex_lock(&lock);
		int closeRet = pclose(fp);
		pthread_mutex_unlock(&lock);
		select(0, NULL, NULL, NULL, &delay);
		while (closeRet == -1) {
			pthread_mutex_lock(&lock);
			closeRet = pclose(fp);
			pthread_mutex_unlock(&lock);
			select(0, NULL, NULL, NULL, &delay);
		}
		XLOGD("after pclose, tid: %d, errno: %d", tid, errno);
	}
	pthread_exit(NULL);
	return NULL;
}

void doApMcuTest(int index, int core_number, RPCClient* msgSender) {
	const char* test_files[][CORE_NUMBER_8] = {
		{FILE_VFP_0,    FILE_VFP_1,    FILE_VFP_2,    FILE_VFP_3,    FILE_VFP_4,    FILE_VFP_5,    FILE_VFP_6,    FILE_VFP_7},
		{FILE_CA7_0,    FILE_CA7_1,    FILE_CA7_2,    FILE_CA7_3,    FILE_CA7_4,    FILE_CA7_5,    FILE_CA7_6,    FILE_CA7_7},
		{FILE_DHRY_0,   FILE_DHRY_1,   FILE_DHRY_2,   FILE_DHRY_3,   FILE_DHRY_4,   FILE_DHRY_5,   FILE_DHRY_6,   FILE_DHRY_7},
		{FILE_MEMCPY_0, FILE_MEMCPY_1, FILE_MEMCPY_2, FILE_MEMCPY_3, FILE_MEMCPY_4, FILE_MEMCPY_5, FILE_MEMCPY_6, FILE_MEMCPY_7},
		{FILE_FDCT_0,   FILE_FDCT_1,   FILE_FDCT_2,   FILE_FDCT_3,   FILE_FDCT_4,   FILE_FDCT_5,   FILE_FDCT_6,   FILE_FDCT_7},
		{FILE_IMDCT_0,  FILE_IMDCT_1,  FILE_IMDCT_2,  FILE_IMDCT_3,  FILE_IMDCT_4,  FILE_IMDCT_5,  FILE_IMDCT_6,  FILE_IMDCT_7},
	};

	int chip = em_jni_get_chip_id();
	XLOGD("chip id: %d", chip);
	if (chip == MTK_6575_SUPPORT || chip == MTK_6577_SUPPORT
			|| chip == MTK_6573_SUPPORT || chip == MTK_6516_SUPPORT) {
		test_files[0][0] = FILE_NEON_0;
		test_files[0][1] = FILE_NEON_1;
		test_files[0][2] = FILE_NEON_2;
		test_files[0][3] = FILE_NEON_3;
		test_files[1][0] = FILE_CA9_0;
		test_files[1][1] = FILE_CA9_1;
		test_files[1][2] = FILE_CA9_2;
		test_files[1][3] = FILE_CA9_3;
	}

	struct thread_status_t test_threads[CORE_NUMBER_8] = {
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
	};
	XLOGD("test_threads size is %d", sizeof(test_threads));

    sched_param param;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_IDLE);
    param.sched_priority = 0;
    pthread_attr_setschedparam(&attr, &param);
      
	for (int i = 0; i < core_number; i++) {
		strcpy(test_threads[i].param.file, (char *) test_files[index][i]);
		test_threads[i].create_result = pthread_create(&test_threads[i].pid,
				&attr, apmcu_test, (void *) &test_threads[i].param);
	}
	for (int i = 0; i < core_number; i++) {
		if (test_threads[i].pid) {
			pthread_join(test_threads[i].pid, NULL);
		}
	}
	char result[CPUTEST_RESULT_SIZE] = { 0 };
	for (int i = 0; i < CORE_NUMBER_8; i++) {
		strncat(result, test_threads[i].param.result, strlen(test_threads[i].param.result)-1);
		strncat(result, ";", 1);
	}
	result[(int) strlen(result) - 1] = 0;
	XLOGD("apmcu result is %s", result);
	msgSender->PostMsg(result);
}

int ModuleCpuStress::ApMcu(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int index = 0, core_num = 0;
	if (paraNum != 2) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	int T = msgSender->ReadInt();
	if (T != PARAM_TYPE_INT) {
		return -1;
	}
	int L = msgSender->ReadInt();
	index = msgSender->ReadInt();
	T = msgSender->ReadInt();
	if (T != PARAM_TYPE_INT) {
		return -1;
	}
	L = msgSender->ReadInt();
	core_num = msgSender->ReadInt();
	if ((index >= INDEX_TEST_NEON) && (index <= INDEX_TEST_IMDCT)
			&& (core_num >= CORE_NUMBER_1) && (core_num <= CORE_NUMBER_8)) {
		doApMcuTest(index, core_num, msgSender);
	} else {
		XLOGE("apmcu unknown index: %d, core_num: %d", index, core_num);
	}
	return 0;
}

void doSwCodecTest(int core_number, int iteration, RPCClient* msgSender) {
	const char* test_files[][CORE_NUMBER_8] = {
		{COMMAND_SWCODEC_TEST_SINGLE},
		{COMMAND_SWCODEC_TEST_DUAL_0, COMMAND_SWCODEC_TEST_DUAL_1},
		{
			COMMAND_SWCODEC_TEST_TRIPLE_0, COMMAND_SWCODEC_TEST_TRIPLE_1,
			COMMAND_SWCODEC_TEST_TRIPLE_2
		},
		{
			COMMAND_SWCODEC_TEST_QUAD_0, COMMAND_SWCODEC_TEST_QUAD_1,
			COMMAND_SWCODEC_TEST_QUAD_2, COMMAND_SWCODEC_TEST_QUAD_3
		},
		{NULL},
		{NULL},
		{NULL},
		{
			COMMAND_SWCODEC_TEST_QUAD_0, COMMAND_SWCODEC_TEST_QUAD_1,
			COMMAND_SWCODEC_TEST_QUAD_2, COMMAND_SWCODEC_TEST_QUAD_3,
			COMMAND_SWCODEC_TEST_OCTA_4, COMMAND_SWCODEC_TEST_OCTA_5,
			COMMAND_SWCODEC_TEST_OCTA_6, COMMAND_SWCODEC_TEST_OCTA_7
		},
	};
	int chip = em_jni_get_chip_id();
	XLOGD("chip id: %d", chip);
	if (chip == MTK_6575_SUPPORT) {
		test_files[0][0] = COMMAND_SWCODEC_TEST_SINGLE_6575;
		test_files[1][0] = COMMAND_SWCODEC_TEST_DUAL_0_6575;
		test_files[1][1] = COMMAND_SWCODEC_TEST_DUAL_1_6575;
	} else if (chip == MTK_6577_SUPPORT) {
		test_files[0][0] = COMMAND_SWCODEC_TEST_SINGLE_6577;
		test_files[1][0] = COMMAND_SWCODEC_TEST_DUAL_0_6577;
		test_files[1][1] = COMMAND_SWCODEC_TEST_DUAL_1_6577;
	}

	XLOGD("Enter doSwCodecTest");
	int index = core_number - 1;
	struct thread_status_t test_threads[CORE_NUMBER_8] = {
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
		{ pid : 0, create_result : -1, },
	};
	for (int i = 0; i < core_number; i++) {
		strcpy(test_threads[i].param.file, (char *) test_files[index][i]);
		test_threads[i].create_result = pthread_create(&test_threads[i].pid,
				NULL, swcodec_test, (void *) &test_threads[i].param);
	}
	for (int i = 0; i < core_number; i++) {
		if (test_threads[i].pid) {
			pthread_join(test_threads[i].pid, NULL);
		}
	}
	char result[CPUTEST_RESULT_SIZE] = { 0 };
	for (int i = 0; i < CORE_NUMBER_8; i++) {
		strncat(result, test_threads[i].param.result, strlen(test_threads[i].param.result)-1);
		strncat(result, ";", 1);
	}
	result[(int) strlen(result) - 1] = 0;
	XLOGD("doSwCodecTest result is %s", result);
	msgSender->PostMsg(result);
}

int ModuleCpuStress::SwCodec(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int core_num = 0;
	int iteration = 0;
	if (paraNum != 2) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	int T = msgSender->ReadInt();
	if (T != PARAM_TYPE_INT) {
		//error
		return -1;
	}
	int L = msgSender->ReadInt();
	core_num = msgSender->ReadInt();
	XLOGD("ModuleCpuStress:SwCodec core_num: %d", core_num);
	T = msgSender->ReadInt();
	if (T != PARAM_TYPE_INT) {
		return -1;
	}
	L = msgSender->ReadInt();
	iteration = msgSender->ReadInt();
	XLOGD("ModuleCpuStress:SwCodec iterate: %d", iteration);
	if ((core_num >= CORE_NUMBER_1) && (core_num <= CORE_NUMBER_8)) {
	    pthread_mutex_init(&lock, NULL);
		doSwCodecTest(core_num, iteration, msgSender);
		pthread_mutex_destroy(&lock);
	} else {
		XLOGE("SwCodec unknown index: %d", index);
	}
	return 0;
}

static void echo(const char* value, const char* file) {
	char command[CPUTEST_RESULT_SIZE] = { 0 };
	snprintf(command, CPUTEST_RESULT_SIZE, "echo %s > %s", value, file);
	XLOGD("command: %s", command);
	system(command);
}

static void echo(char buf[CPUTEST_RESULT_SIZE>>1], const char* file) {
	char command[CPUTEST_RESULT_SIZE] = { 0 };
	strcpy(command, "echo ");
	strncat(command, buf, strlen(buf) - 1);
	strcat(command, " > ");
	strcat(command, file);
	XLOGD("command: %s", command);
	system(command);
}

static void backup(char buf[CPUTEST_RESULT_SIZE>>1], const char* file) {
	char command[CPUTEST_RESULT_SIZE] = { 0 };
	strcpy(command, "cat ");
	FILE * fp = popen(strcat(command, file), "r");
	if (fp == NULL) {
		XLOGE("INDEX_TEST_BACKUP popen fail, errno: %d", errno);
		return;
	}
	fgets(buf, sizeof(buf), fp);
	XLOGD("backup_first: %s", buf);
	pclose(fp);
}

void doBackupRestore(int index) {
	char command[CPUTEST_RESULT_SIZE] = { 0 };
	FILE * fp;
	switch(index) {
		case INDEX_TEST_BACKUP:
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU0_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_first, sizeof(backup_first), fp);
			XLOGD("backup_first: %s", backup_first);
			pclose(fp);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_BACKUP: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_DISABLE);
			break;
		case INDEX_TEST_BACKUP_TEST:
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU1_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_TEST: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU2_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_TEST: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU3_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_TEST: %s", command);
			system(command);
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_BACKUP_TEST: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_DISABLE);
			break;
		case INDEX_TEST_BACKUP_SINGLE:
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU0_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_SINGLE popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_first, sizeof(backup_first), fp);
			XLOGD("backup_first: %s", backup_first);
			pclose(fp);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_BACKUP_SINGLE: %s", command);
			system(command);
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_CPU1_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_SINGLE: %s", command);
			system(command);
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_BACKUP_SINGLE: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_DISABLE);
			break;
		case INDEX_TEST_BACKUP_DUAL:
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU0_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_DUAL popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_first, sizeof(backup_first), fp);
			XLOGD("backup_first: %s", backup_first);
			pclose(fp);
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU1_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_DUAL popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_second, sizeof(backup_second), fp);
			XLOGD("backup_second: %s", backup_second);
			pclose(fp);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_BACKUP_DUAL: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU1_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_DUAL: %s", command);
			system(command);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU1_SCAL);
			XLOGD("INDEX_TEST_BACKUP_DUAL: %s", command);
			system(command);
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_BACKUP_DUAL: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_DISABLE);
			break;
		case INDEX_TEST_BACKUP_TRIPLE:
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU0_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_TRIPLE popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_first, sizeof(backup_first), fp);
			XLOGD("backup_first: %s", backup_first);
			pclose(fp);
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU1_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_TRIPLE popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_second, sizeof(backup_second), fp);
			XLOGD("backup_second: %s", backup_second);
			pclose(fp);
			fp = popen(strcat(command, FILE_CPU2_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_TRIPLE popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_third, sizeof(backup_third), fp);
			XLOGD("backup_third: %s", backup_third);
			pclose(fp);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_BACKUP_TRIPLE: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU1_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_TRIPLE: %s", command);
			system(command);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU1_SCAL);
			XLOGD("INDEX_TEST_BACKUP_TRIPLE: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU2_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_TRIPLE: %s", command);
			system(command);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU2_SCAL);
			XLOGD("INDEX_TEST_BACKUP_TRIPLE: %s", command);
			system(command);
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_BACKUP_TRIPLE: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_DISABLE);
			break;
		case INDEX_TEST_BACKUP_QUAD:
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU0_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_QUAD popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_first, sizeof(backup_first), fp);
			XLOGD("backup_first: %s", backup_first);
			pclose(fp);
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU1_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_QUAD popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_second, sizeof(backup_second), fp);
			XLOGD("backup_second: %s", backup_second);
			pclose(fp);
			fp = popen(strcat(command, FILE_CPU2_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_QUAD popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_third, sizeof(backup_third), fp);
			XLOGD("backup_third: %s", backup_third);
			pclose(fp);
			strcpy(command, "cat ");
			fp = popen(strcat(command, FILE_CPU3_SCAL), "r");
			if (fp == NULL) {
				XLOGE("INDEX_TEST_BACKUP_QUAD popen fail, errno: %d", errno);
				return;
			}
			fgets(backup_fourth, sizeof(backup_fourth), fp);
			XLOGD("backup_fourth: %s", backup_fourth);
			pclose(fp);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_BACKUP_QUAD: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU1_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_QUAD: %s", command);
			system(command);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU1_SCAL);
			XLOGD("INDEX_TEST_BACKUP_QUAD: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU2_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_QUAD: %s", command);
			system(command);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU2_SCAL);
			XLOGD("INDEX_TEST_BACKUP_QUAD: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_CPU3_ONLINE);
			XLOGD("INDEX_TEST_BACKUP_QUAD: %s", command);
			system(command);
			strcpy(command, "echo performance > ");
			strcat(command, FILE_CPU3_SCAL);
			XLOGD("INDEX_TEST_BACKUP_QUAD: %s", command);
			system(command);
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_BACKUP_QUAD: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_DISABLE);
			break;
		case INDEX_TEST_BACKUP_OCTA:
			XLOGD("INDEX_TEST_BACKUP_OCTA start");
			backup(backup_first, FILE_CPU0_SCAL);
			echo("performance", FILE_CPU0_SCAL);
			echo("1", FILE_CPU1_ONLINE);
			echo("1", FILE_CPU2_ONLINE);
			echo("1", FILE_CPU3_ONLINE);
			echo("1", FILE_CPU4_ONLINE);
			echo("1", FILE_CPU5_ONLINE);
			echo("1", FILE_CPU6_ONLINE);
			echo("1", FILE_CPU7_ONLINE);
			echo("0", FILE_HOTPLUG);
			system(COMMAND_HOTPLUG_DISABLE);
			XLOGD("INDEX_TEST_BACKUP_OCTA end");
			break;
		case INDEX_TEST_RESTORE:
			strcpy(command, "echo ");
			strncat(command, backup_first, strlen(backup_first) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_RESTORE: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_ENABLE);
			break;
		case INDEX_TEST_RESTORE_TEST:
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_CPU1_ONLINE);
			XLOGD("INDEX_TEST_RESTORE_TEST: %s", command);
			system(command);
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_CPU2_ONLINE);
			XLOGD("INDEX_TEST_RESTORE_TEST: %s", command);
			system(command);
			strcpy(command, "echo 0 > ");
			strcat(command, FILE_CPU3_ONLINE);
			XLOGD("INDEX_TEST_RESTORE_TEST: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_RESTORE_TEST: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_ENABLE);
			break;
		case INDEX_TEST_RESTORE_SINGLE:
			strcpy(command, "echo ");
			strncat(command, backup_first, strlen(backup_first) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_RESTORE_SINGLE: %s", command);
			system(command);
			//strcpy(command, "echo 1 > ");
			//strcat(command, FILE_CPU1_ONLINE);
			//XLOGD("INDEX_TEST_RESTORE_SINGLE: %s", command);
			//system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_RESTORE_SINGLE: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_ENABLE);
			break;
		case INDEX_TEST_RESTORE_DUAL:
			strcpy(command, "echo ");
			strncat(command, backup_first, strlen(backup_first) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo ");
			strncat(command, backup_second, strlen(backup_second) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU1_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_ENABLE);
			break;
		case INDEX_TEST_RESTORE_TRIPLE:
			strcpy(command, "echo ");
			strncat(command, backup_first, strlen(backup_first) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo ");
			strncat(command, backup_second, strlen(backup_second) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU1_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo ");
			strncat(command, backup_third, strlen(backup_third) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU2_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_ENABLE);
			break;
		case INDEX_TEST_RESTORE_QUAD:
			strcpy(command, "echo ");
			strncat(command, backup_first, strlen(backup_first) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU0_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo ");
			strncat(command, backup_second, strlen(backup_second) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU1_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo ");
			strncat(command, backup_third, strlen(backup_third) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU2_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo ");
			strncat(command, backup_fourth, strlen(backup_fourth) - 1);
			strcat(command, " > ");
			strcat(command, FILE_CPU3_SCAL);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			strcpy(command, "echo 1 > ");
			strcat(command, FILE_HOTPLUG);
			XLOGD("INDEX_TEST_RESTORE_DUAL: %s", command);
			system(command);
			system(COMMAND_HOTPLUG_ENABLE);
			break;
		case INDEX_TEST_RESTORE_OCTA:
			XLOGD("INDEX_TEST_RESTORE_OCTA start");
			echo(backup_first, FILE_CPU0_SCAL);
			echo("1", FILE_HOTPLUG);
			system(COMMAND_HOTPLUG_ENABLE);
			XLOGD("INDEX_TEST_RESTORE_OCTA end");
			break;
		default:
			break;
	}
}

int ModuleCpuStress::BackupRestore(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int index = 0;
	if (paraNum != 1) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	int T = msgSender->ReadInt();
	if (T != PARAM_TYPE_INT) {
		return -1;
	}
	int L = msgSender->ReadInt();
	index = msgSender->ReadInt();
	switch (index) {
	case INDEX_TEST_BACKUP:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_BACKUP");
		break;
	case INDEX_TEST_BACKUP_TEST:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_BACKUP_TEST");
		break;
	case INDEX_TEST_BACKUP_SINGLE:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_BACKUP_SINGLE");
		break;
	case INDEX_TEST_BACKUP_DUAL:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_BACKUP_DUAL");
		break;
	case INDEX_TEST_BACKUP_TRIPLE:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_BACKUP_TRIPLE");
		break;
	case INDEX_TEST_BACKUP_QUAD:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_BACKUP_QUAD");
		break;
	case INDEX_TEST_BACKUP_OCTA:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_BACKUP_OCTA");
		break;
	case INDEX_TEST_RESTORE:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_RESTORE");
		break;
	case INDEX_TEST_RESTORE_TEST:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_RESTORE_TEST");
		break;
	case INDEX_TEST_RESTORE_SINGLE:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_RESTORE_SINGLE");
		break;
	case INDEX_TEST_RESTORE_DUAL:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_RESTORE_DUAL");
		break;
	case INDEX_TEST_RESTORE_TRIPLE:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_RESTORE_TRIPLE");
		break;
	case INDEX_TEST_RESTORE_QUAD:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_RESTORE_QUAD");
		break;
	case INDEX_TEST_RESTORE_OCTA:
		doBackupRestore(index);
		msgSender->PostMsg((char *)"INDEX_TEST_RESTORE_OCTA");
		break;
	default:
		XLOGE("BackupRestore unknown index: %d", index);
		msgSender->PostMsg((char *)"BackRestore unknown index");
		break;
	}
	return 0;
}


int ModuleCpuStress::ThermalUpdate(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int index = 0;
	if (paraNum != 1) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	int T = msgSender->ReadInt();
	if (T != PARAM_TYPE_INT) {
		return -1;
	}
	int L = msgSender->ReadInt();
	index = msgSender->ReadInt();
	switch(index) {
	case INDEX_THERMAL_DISABLE:
		system(THERMAL_DISABLE_COMMAND);
		XLOGD("disable thermal: %s", THERMAL_DISABLE_COMMAND);
		msgSender->PostMsg((char *)"INDEX_THERMAL_DISABLE");
		break;
	case INDEX_THERMAL_ENABLE:
		system(THERMAL_ENABLE_COMMAND);
		XLOGD("enable thermal: %s", THERMAL_ENABLE_COMMAND);
		msgSender->PostMsg((char *)"INDEX_THERMAL_ENABLE");
		break;
	default:
		break;
	}
	return 0;
}

ModuleCpuStress::ModuleCpuStress(void) {
	pthread_mutex_init(&lock, NULL);
}

ModuleCpuStress::~ModuleCpuStress(void) {
	pthread_mutex_destroy(&lock);
}

