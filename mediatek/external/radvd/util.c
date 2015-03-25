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

void
mdelay(double msecs)
{
	struct timeval tv;

	tv.tv_sec = (time_t)(msecs / 1000.0);
	tv.tv_usec = (suseconds_t)((msecs - tv.tv_sec * 1000.0) * 1000.0);

	select(0,(fd_set *)NULL,(fd_set *)NULL,(fd_set *)NULL, &tv);
}

double
rand_between(double lower, double upper)
{
	return ((upper - lower) / (RAND_MAX + 1.0) * rand() + lower);
}

void
print_addr(struct in6_addr *addr, char *str)
{
	const char *res;

	/* XXX: overflows 'str' if it isn't big enough */
	res = inet_ntop(AF_INET6, (void *)addr, str, INET6_ADDRSTRLEN);

	if (res == NULL)
	{
		flog(LOG_ERR, "print_addr: inet_ntop: %s", strerror(errno));
		strcpy(str, "[invalid address]");
	}
}

/* Check if an in6_addr exists in the rdnss list */
int
check_rdnss_presence(struct AdvRDNSS *rdnss, struct in6_addr *addr)
{
	while (rdnss) {
		if (    !memcmp(&rdnss->AdvRDNSSAddr1, addr, sizeof(struct in6_addr))
		     || !memcmp(&rdnss->AdvRDNSSAddr2, addr, sizeof(struct in6_addr))
		     || !memcmp(&rdnss->AdvRDNSSAddr3, addr, sizeof(struct in6_addr)) )
			break; /* rdnss address found in the list */
		else
			rdnss = rdnss->next; /* no match */
	}
	return (rdnss != NULL);
}

/* Check if a suffix exists in the dnssl list */
int
check_dnssl_presence(struct AdvDNSSL *dnssl, const char *suffix)
{
	int i;
	while (dnssl) {
		for (i = 0; i < dnssl->AdvDNSSLNumber; i++) {
			if (strcmp(dnssl->AdvDNSSLSuffixes[i], suffix) == 0)
				break; /* suffix found in the list */
		}
		if (i != dnssl->AdvDNSSLNumber)
			break;

		dnssl = dnssl->next; /* no match */
	}
	return (dnssl != NULL);
}

/* Like read(), but retries in case of partial read */
ssize_t
readn(int fd, void *buf, size_t count)
{
	size_t n = 0;
	while (count > 0) {
		int r = read(fd, buf, count);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return r;
		}
		if (r == 0)
			return n;
		buf = (char *)buf + r;
		count -= r;
		n += r;
	}
	return n;
}

/* Like write(), but retries in case of partial write */
ssize_t
writen(int fd, const void *buf, size_t count)
{
	size_t n = 0;
	while (count > 0) {
		int r = write(fd, buf, count);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return r;
		}
		if (r == 0)
			return n;
		buf = (const char *)buf + r;
		count -= r;
		n += r;
	}
	return n;
}
