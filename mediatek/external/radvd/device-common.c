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
#include "includes.h"
#include "radvd.h"
#include "defaults.h"

int
check_device(struct Interface *iface)
{
	struct ifreq	ifr;

	strncpy(ifr.ifr_name, iface->Name, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
	{
		if (!iface->IgnoreIfMissing)
			flog(LOG_ERR, "ioctl(SIOCGIFFLAGS) failed for %s: %s",
				iface->Name, strerror(errno));
		return (-1);
	}

	if (!(ifr.ifr_flags & IFF_UP))
	{
		if (!iface->IgnoreIfMissing)
                	flog(LOG_ERR, "interface %s is not UP", iface->Name);
		return (-1);
	}
	if (!(ifr.ifr_flags & IFF_RUNNING))
	{
		if (!iface->IgnoreIfMissing)
                	flog(LOG_ERR, "interface %s is not RUNNING", iface->Name);
		return (-1);
	}

	if (! iface->UnicastOnly && !(ifr.ifr_flags & IFF_MULTICAST))
	{
		flog(LOG_WARNING, "interface %s does not support multicast",
			iface->Name);
		flog(LOG_WARNING, "   do you need to add the UnicastOnly flag?");
	}

	if (! iface->UnicastOnly && !(ifr.ifr_flags & IFF_BROADCAST))
	{
		flog(LOG_WARNING, "interface %s does not support broadcast",
			iface->Name);
		flog(LOG_WARNING, "   do you need to add the UnicastOnly flag?");
	}

	return 0;
}

int
get_v4addr(const char *ifn, unsigned int *dst)
{
        struct ifreq    ifr;
        struct sockaddr_in *addr;
        int fd;

        if( ( fd = socket(AF_INET,SOCK_DGRAM,0) ) < 0 )
        {
                flog(LOG_ERR, "create socket for IPv4 ioctl failed for %s: %s",
                        ifn, strerror(errno));
                return (-1);
        }

        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, ifn, IFNAMSIZ-1);
        ifr.ifr_name[IFNAMSIZ-1] = '\0';
        ifr.ifr_addr.sa_family = AF_INET;

        if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
        {
                flog(LOG_ERR, "ioctl(SIOCGIFADDR) failed for %s: %s",
                        ifn, strerror(errno));
                close( fd );
                return (-1);
        }

        addr = (struct sockaddr_in *)(&ifr.ifr_addr);

        dlog(LOG_DEBUG, 3, "IPv4 address for %s is %s", ifn,
                inet_ntoa( addr->sin_addr ) );

        *dst = addr->sin_addr.s_addr;

        close( fd );

        return 0;
}
