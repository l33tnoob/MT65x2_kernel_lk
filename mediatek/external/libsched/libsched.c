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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/mt_sched.h>

#define DEVFILE "/dev/mtk_sched"

int mt_sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask)
{
	int fd, ret;
	struct ioctl_arg cmd;
       
	fd = open(DEVFILE, O_RDWR);
	if (fd == -1)
		return sched_setaffinity(pid, cpusetsize, mask);

	cmd.pid = pid;
	cmd.len = cpusetsize;
	cmd.mask = (unsigned long *) mask;
	ret = ioctl(fd, IOCTL_SETAFFINITY, &cmd);
	
        if (close(fd) != 0)
                return -EIO;

        return ret;	
}

/* 
 * input:
 *	mask: The same as sched_get_affinity
 * 	mt_mask: backup affinity setting
 * Return: 
 *	On Success: return the length of return mask
 * 	On error: return 0 or error code. Error code is negative value. 
 */
int mt_sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask, cpu_set_t *mt_mask)
{
	int fd, ret;
	struct ioctl_arg cmd;
       
	fd = open(DEVFILE, O_RDWR);
	if (fd == -1){
		return sched_getaffinity(pid, cpusetsize, mask);
	}

	cmd.pid = pid;
	cmd.len = cpusetsize;
	cmd.mask = (unsigned long *) mask;
	cmd.mt_mask = (unsigned long *) mt_mask;	
	ret = ioctl(fd, IOCTL_GETAFFINITY, &cmd);

        if (close(fd) != 0)
                return -EIO;	

	return ret;
}

int mt_sched_exitaffinity(pid_t pid)
{
	int fd, ret;
	       
	fd = open(DEVFILE, O_RDWR);
	if (fd == -1){
		return 0;
		//return -ENOENT;
	}

	ret = ioctl(fd, IOCTL_EXITAFFINITY, &pid);

        if (close(fd) != 0)
                return -EIO;

	return ret;
}

