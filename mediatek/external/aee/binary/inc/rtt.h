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


#ifndef __rtt_h
#define __rtt_h

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <unwind.h>

#ifdef __cplusplus
extern "C" {
#endif 

// RTT Exported Functions
extern int rtt_is_ready(int block);
extern int rtt_dump_backtrace(pid_t pid, pid_t tid, const char* file_path,int fd);
extern int rtt_dump_all_backtrace(pid_t pid, const char* file_path,int fd);
extern int rtt_dump_all_backtrace_by_name(const char *ps_name, const char *file_path,int fd);
extern int rtt_dump_process_name_to_pid(const char *proc_name);


/*
 * Get the caller backtrace
 *   max_cnt: call stack frames maximum count
 *   paddrs : arrays to save the frame address
 * Return
 *   total stack frame count could unwinded by the interface
 */
extern unsigned rtt_get_caller_backtrace(unsigned max_cnt, intptr_t* paddrs);

/*
 * Print the caller backtrace
 *   tag: use the logcat tag to dump the result 
 */
extern void rtt_dump_caller_backtrace(const char* tag);

extern int rtt_is_java_process(pid_t pid);

/*
 * Get the system average load
 *   min_1  : last 1 minutes system average load
 *   min_5  : last 5 minutes system average load
 *   min_15 : last 15 minutes system average load
 * Return
 *   0: success
 *   fail otherwise
 */
extern int rtt_get_loadavg(double* min_1, double* min_5, double* min_15);

/*
 * Get the system uptime from booting to now
 * Return
 *   seconds from boot time to now
 */
extern double rtt_get_uptime();

/*
 * Get the system time. Include RTC and uptime
 *   to_screen : whether dump to Android log (0: disable)
 *   uptime    : whether to get the up time (0: disable)
 *   out       : whether write the result to the buffer (NULL: disable)
 *   len       : lenght of the out buffer
 * Return
 *   0: success
 *   fail otherwise
 */
extern int rtt_get_system_time(int to_screen, int uptime, char* out, 
    unsigned int len);

#ifdef __cplusplus
}
#endif 

#endif
