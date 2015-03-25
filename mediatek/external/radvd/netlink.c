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

/*
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>	 
 *    Reuben Hawkins		<reubenhwk@gmail.com>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <pekkas@netcore.fi>.
 *
 */

#include "config.h"
#include "radvd.h"
#include "log.h"
#include "netlink.h"

#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static int set_last_prefix(const char * ifname){
	char prefix_prop_name[PROPERTY_KEY_MAX];
	char plen_prop_name[PROPERTY_KEY_MAX];
	char prop_value[PROPERTY_VALUE_MAX] = {'\0'};

	if (property_get("net.ipv6.tether", prop_value, NULL)) {
		if(0 == strcmp(prop_value, ifname)){
			dlog(LOG_DEBUG, 3, "set last prefix for %s", ifname);
		} else{
		    dlog(LOG_DEBUG, 3, "%s is not tether interface", ifname);
			return 0;
		}
	}
#ifdef MTK_IPV6_TETHER_PD_MODE
	snprintf(prefix_prop_name, sizeof(prefix_prop_name), 
		"net.pd.%s.prefix", ifname);
	if (property_get(prefix_prop_name, prop_value, NULL)) {
			property_set(prefix_prop_name, "");
			//set last prefix
			property_set("net.pd.lastprefix", prop_value);
	}
	snprintf(plen_prop_name, sizeof(plen_prop_name), 
		"net.pd.%s.plen", ifname);
	if (property_get(plen_prop_name, prop_value, NULL)) {
			property_set(plen_prop_name, "");
			property_set("net.pd.lastplen", prop_value);
			
	}

#else	
	snprintf(prefix_prop_name, sizeof(prefix_prop_name), 
		"net.ipv6.%s.prefix", ifname);
	if (property_get(prefix_prop_name, prop_value, NULL)) {
			property_set(prefix_prop_name, "");
			//set last prefix
			property_set("net.ipv6.lastprefix", prop_value);
	}
	snprintf(plen_prop_name, sizeof(plen_prop_name), 
		"net.ipv6.%s.plen", ifname);
	if (property_get(plen_prop_name, prop_value, NULL)) {
			property_set(plen_prop_name, "");
	}
#endif
	
	return 0;
}

void process_netlink_msg(int sock)
{
	int len;
	char buf[4096];
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl sa;
	struct msghdr msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
	struct nlmsghdr *nh;
	struct ifinfomsg * ifinfo;
	char ifname[IF_NAMESIZE] = {""};

	len = recvmsg (sock, &msg, 0);
	if (len == -1) {
		flog(LOG_ERR, "recvmsg failed: %s", strerror(errno));
	}

	for (nh = (struct nlmsghdr *) buf; NLMSG_OK (nh, len); nh = NLMSG_NEXT (nh, len)) {
		/* The end of multipart message. */
		if (nh->nlmsg_type == NLMSG_DONE)
			return;

		if (nh->nlmsg_type == NLMSG_ERROR) {
			flog(LOG_ERR, "%s:%d Some type of netlink error.\n", __FILE__, __LINE__);
			abort();
		}
/*
		if (nh->nlmsg_type == RTM_NEWPREFIX) {
			ifinfo = NLMSG_DATA(nh);
			if_indextoname(ifinfo->ifi_index, ifname);
			dlog(LOG_DEBUG, 3, "%s receive new prefix.\n", ifname);

			char tether_interface[PROPERTY_VALUE_MAX] = {'\0'};
			
			if (!property_get("net.ipv6.tether", tether_interface, NULL)) {
				flog(LOG_ERR, "netlink get tether interface failed!");
				abort();
			}
			if(!strncmp(tether_interface, ifname, strlen(ifname))){
				dlog(LOG_DEBUG, 2, "reload config for new prefix.\n");
				reload_config();
			}
			
			return ;
		}
*/

		/* Continue with parsing payload. */
		ifinfo = NLMSG_DATA(nh);
		if_indextoname(ifinfo->ifi_index, ifname);
		if (ifinfo->ifi_flags & IFF_RUNNING) {
			dlog(LOG_DEBUG, 3, "%s, ifindex %d, flags is running", ifname, ifinfo->ifi_index);
		}
		else {
			dlog(LOG_DEBUG, 3, "%s, ifindex %d, flags is *NOT* running", ifname, ifinfo->ifi_index);
			set_last_prefix(ifname);
		}
		reload_config();
	}
}

int netlink_socket(void)
{
	int rc, sock;
	struct sockaddr_nl snl;

	sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock == -1) {
		flog(LOG_ERR, "Unable to open netlink socket: %s", strerror(errno));
	}

	memset(&snl, 0, sizeof(snl));
	snl.nl_family = AF_NETLINK;
	snl.nl_groups = RTMGRP_LINK;

	rc = bind(sock, (struct sockaddr*)&snl, sizeof(snl));
	if (rc == -1) {
		flog(LOG_ERR, "Unable to bind netlink socket: %s", strerror(errno));
		close(sock);
		sock = -1;
	}

	return sock;
}

