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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   emd_d.c
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 *   Dual Talk
 *
 * Author:
 * -------
 *   Chao Song
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <android/log.h>
#include <sys/ioctl.h>

#define EXT_MD_MONITOR_DEV "/dev/ext_md_ctl0"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "emd_d",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "emd_d",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "emd_d",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "emd_d",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "emd_d",__VA_ARGS__)

/* MD Message, this is for user space deamon use */
enum {
	EXT_MD_MSG_READY = 0xF0A50000,
	EXT_MD_MSG_REQUEST_RST,
	EXT_MD_MSG_WAIT_DONE,
};

/* MD Status, this is for user space deamon use */
enum {
	EXT_MD_STA_RST = 0,
	EXT_MD_STA_RDY = 1,
	EXT_MD_STA_WAIT = 2,
};

#define EXT_MD_IOC_MAGIC		'E'
#define EXT_MD_IOCTL_LET_MD_GO		_IO(EXT_MD_IOC_MAGIC, 1)
#define EXT_MD_IOCTL_REQUEST_RESET	_IO(EXT_MD_IOC_MAGIC, 2)
#define EXT_MD_IOCTL_POWER_ON_HOLD	_IO(EXT_MD_IOC_MAGIC, 3)

static int curr_md_sta = EXT_MD_STA_RST;

static void stop_ext_md_services(void)
{
	LOGD("stop_ext_md_services\n");

	property_set("ctl.stop", "gsm0710muxd3");
	property_set("ctl.stop", "ril3-daemon");
}

static void start_ext_md_services(void)
{
	LOGD("start_ext_md_services\n");

	property_set("ctl.start", "gsm0710muxd3");
	//property_set("ctl.start", "ril3-daemon");
}

static void ready_state_handler(int message)
{
	switch(message)
	{
	case EXT_MD_MSG_REQUEST_RST:
		LOGD("Got reset request mesage @ready\n");
		curr_md_sta = EXT_MD_STA_WAIT;
		stop_ext_md_services();
		break;
	default:
		LOGD("Default @ready, msg=%08x\n", message);
		break;
	}
}

static void reset_state_handler(int message)
{
	switch(message)
	{
	case EXT_MD_MSG_READY:
		LOGD("Got md ready mesage @reset\n");
		curr_md_sta = EXT_MD_STA_RDY;
		break;
	default:
		LOGD(" Default @ reset, msg=%08x\n", message);
		break;
	}
}

static void wait_state_handler(int message)
{
	switch(message)
	{
	case EXT_MD_MSG_WAIT_DONE:
		LOGD("Got waitdone mesage @wait\n");
		curr_md_sta = EXT_MD_STA_RST;
		start_ext_md_services();
		break;
	default:
		LOGD(" Default @ wait, msg=%08x\n", message);
		break;
	}
}

int main(int argc, char **argv)
{
	int ret, fd, message;
	ssize_t s;

	LOGD("emd_daemon ver:0.02");

	fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
	if (fd < 0) {
		LOGD("fail to open %s: ", EXT_MD_MONITOR_DEV);
		perror("");
		return -1;
	}

	ret = ioctl(fd, EXT_MD_IOCTL_POWER_ON_HOLD, NULL);
	if (ret < 0) {
		LOGD("power on modem failed!\n");
		return ret;
	}

	start_ext_md_services();

	do {
		s = read(fd, (void *)&message, sizeof(int));
		if (s<=0) {
			LOGD("read error ret=%ld.\n",s); 
			continue;
		} else if (s!= sizeof(int)) {
			LOGD("read ext md message with unexpected size\n");
			LOGD("s = %d\n", (int)s);
			continue;
		} 

		LOGD("message = 0x%08x\n", message);

		switch(curr_md_sta)
		{
		case EXT_MD_STA_RDY:
			ready_state_handler(message);
			break;
		case EXT_MD_STA_WAIT:
			wait_state_handler(message);
			break;
		case EXT_MD_STA_RST:
			reset_state_handler(message);
			break;
		default:
			LOGD("Invalid state, should not enter here!!\n");
			break;
		}

	} while (1);

	return 0;
}

