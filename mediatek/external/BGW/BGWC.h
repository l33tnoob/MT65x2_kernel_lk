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

 #ifndef __BGWC_H__
 #define __BGWC_H__

#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sched.h>
#include <pthread.h>
#include <cutils/xlog.h> 
#include <errno.h>
#include "BGW_CCCI.h"

 
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#ifdef MSG
#undef MSG
#endif
#ifdef ERR
#undef ERR
#endif
 
#define LOG_TAG "BGWD"
#define MSG(fmt, arg ...) XLOGD("[BGW]%s: " fmt, __FUNCTION__ ,##arg)
#define ERR(fmt, arg ...) XLOGE("[BGW]%s: " fmt, __FUNCTION__ ,##arg)

#ifdef MAX_NL_MSG_LEN
#undef MAX_NL_MSG_LEN
#endif

#define MAX_NL_MSG_LEN 1024

#ifdef ON
#undef ON
#endif
#ifdef OFF
#undef OFF
#endif
#ifdef ACK
#undef ACK
#endif



#define ON 1
#define OFF 0
#define ACK 2

#ifdef NETLINK_TEST
#undef NETLINK_TEST
#endif
#define NETLINK_TEST 17
/*
struct req
{
	struct nlmsghdr nlh;
	char buf[MAX_NL_MSG_LEN];
};
*/
typedef enum
{
	INIT = 0,
	WAITING,
	RUNNING,
	UNKNOWN,
} STATE_MACHINE;


#define COMBO_NODE "/dev/stpwmt"
 

 struct req
 {
	 struct nlmsghdr nlh;
	 char buf[MAX_NL_MSG_LEN];
 };


 typedef struct BGWD_t	 //BT GPS WIFI Daemon tag
 {
	 int sock_id;	 /*kernel netlink socket id*/
	 pthread_t BGWM_id;/*thread pid*/
	 /*record for receiving msg*/
//	 struct nlmsghdr nlhdr;
	struct req r;
	 struct sockaddr_nl nladdr;
	 struct msghdr msg;
	 struct iovec iov;
	 STATE_MACHINE state_machine;
 }BGWD_T;

 
 #endif
