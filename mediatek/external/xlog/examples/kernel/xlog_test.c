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

#include <linux/module.h>
#include <linux/xlog.h>
#include <linux/proc_fs.h>
#include <linux/time.h>

static int xlog_test_call(char *buf, char **start, off_t off, 
			  int count, int *eof, void *data)
{
	int len = 0;

	xlog_printk(ANDROID_LOG_VERBOSE, "xlogtest/kernel", "verbose hello world\n");
	xlog_printk(ANDROID_LOG_DEBUG, "xlogtest/kernel", "debug hello world\n");
	xlog_printk(ANDROID_LOG_INFO, "xlogtest/kernel", "info hello world\n");
	xlog_printk(ANDROID_LOG_WARN, "xlogtest/kernel", "warning hello world\n");
	xlog_printk(ANDROID_LOG_ERROR, "xlogtest/kernel", "error hello world\n");

	xlog_ksystem_printk(ANDROID_LOG_ERROR, "xlogtest/kernel", "Error in ksystem buffer");

	len = sprintf(buf, "%s", "hello world\n");
	return len;
}

static int xlog_test_perf_call(char *buf, char **start, off_t off, 
			       int count, int *eof, void *data)
{
	int len = 0;
	struct timeval tv_start, tv_end, tv_diff;

	do_gettimeofday(&tv_start);
	xlog_printk(ANDROID_LOG_VERBOSE, "xlog/test", "verbose hello world\n");
	do_gettimeofday(&tv_end);
	tv_diff.tv_sec = tv_end.tv_sec - tv_start.tv_sec;
	tv_diff.tv_usec = tv_end.tv_usec - tv_start.tv_usec;
	if (tv_diff.tv_usec < 0) {
		++tv_diff.tv_sec;
		tv_diff.tv_usec += 1000000;
	}
	len = sprintf(buf, "Time diff %d(s)%d(us)\n", tv_diff.tv_sec, tv_diff.tv_usec);
	return len;
}

static int __init xlog_test_init(void)
{
	struct proc_dir_entry *entry;

        entry = create_proc_read_entry("xlog_test", 0, NULL, xlog_test_call, NULL);
        if (!entry)
        {
                goto create_err;
        }

        entry = create_proc_read_entry("xlog_test_perf", 0, NULL, xlog_test_perf_call, NULL);
        if (!entry)
        {
                goto create_err;
        }

        return 0;
create_err:
        return -1;
}

static void __exit xlog_test_exit(void)
{
	remove_proc_entry("xlog_test", NULL);
	remove_proc_entry("xlog_test_perf", NULL);
}

module_init(xlog_test_init);
module_exit(xlog_test_exit);
