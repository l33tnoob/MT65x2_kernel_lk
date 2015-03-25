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


#ifndef __aee_h
#define __aee_h

#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>

#include <sys/ptrace.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
//#include <cutils/log.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef enum {
    AE_KE = 0, /* Kernel panic, HWT-REBOOT  */
    AE_NE,     /* Process received signal and terminate abnormally */
    AE_JE,
    AE_SWT,
    AE_EE, 
    AE_EXP_ERR_END,
    AE_ANR, /* Error or Warning or Defect */
    AE_RESMON,  
    AE_MODEM_WARNING,
    AE_WRN_ERR_END,
    AE_MANUAL, /* Manual Raise */
    AE_EXP_CLASS_END,

    AE_KERNEL_DEFECT = 1000, /* Kernel or driver report abnormal event */
    AE_SYSTEM_JAVA_DEFECT,
    AE_SYSTEM_NATIVE_DEFECT,
} AE_EXP_CLASS; /* General Program Exception Class */

typedef enum {
    AE_DEFECT_FATAL,
    AE_DEFECT_EXCEPTION,
    AE_DEFECT_WARNING,
    AE_DEFECT_REMINDING,
    AE_DEFECT_ATTR_END
} AE_DEFECT_ATTR;

#define AE_INVALID              0xAEEFF000
#define AE_NOT_AVAILABLE        0xAEE00000
#define AE_DEFAULT              0xAEE00001
#define SDCARD_RESMON_CASE      0xAEE00002

/* powerkey press,modules use bits */

#define WDT_SETBY_DEFAULT               	(0)
#define WDT_SETBY_Backlight             	(1<<0)
#define WDT_SETBY_Display              		(1<<1)
#define WDT_SETBY_SF            			(1<<2)
#define WDT_SETBY_PM            			(1<<3)
#define WDT_SETBY_WMS_DISABLE_PWK_MONITOR 	(0xAEEAEE00)
#define WDT_SETBY_WMS_ENABLE_PWK_MONITOR  	(0xAEEAEE01)
#define WDT_PWK_HANG_FORCE_HWT  		 	(0xAEE0FFFF)




// QHQ RT Monitor begin
#define AEEIOCTL_WDT_KICK_POWERKEY _IOR('p', 0x09, int)
#define AEEIOCTL_RT_MON_Kick _IOR('p', 0x0A, int)
#define AE_WDT_DEVICE_PATH      "/dev/RT_Monitor"
#define AE_WDT_POWERKEY_DEVICE_PATH     "/dev/kick_powerkey"
// QHQ RT Monitor    end



/* DB dump option bits, set relative bit to 1 to include related file in db */
#define DB_OPT_DEFAULT                  (0)
#define DB_OPT_FTRACE                   (1<<0)
#define DB_OPT_STORAGE_LOG              (1<<1)
#define DB_OPT_NE_JBT_TRACES            (1<<2)
#define DB_OPT_SWT_JBT_TRACES           (1<<3)
#define DB_OPT_VM_TRACES                (1<<4)
#define DB_OPT_DUMPSYS_ACTIVITY         (1<<5)
#define DB_OPT_DUMPSYS_WINDOW           (1<<6)
#define DB_OPT_DUMPSYS_GFXINFO          (1<<7)
#define DB_OPT_DUMPSYS_SURFACEFLINGER   (1<<8)
#define DB_OPT_DISPLAY_HANG_DUMP        (1<<9)
#define DB_OPT_LOW_MEMORY_KILLER        (1<<10)
#define DB_OPT_PROC_MEM                 (1<<11)
#define DB_OPT_FS_IO_LOG                (1<<12)
#define DB_OPT_PROCESS_COREDUMP         (1<<13)
#define DB_OPT_VM_HPROF                 (1<<14)
#define DB_OPT_PROCMEM                  (1<<15)
#define DB_OPT_DUMPSYS_INPUT            (1<<16)
#define DB_OPT_MMPROFILE_BUFFER         (1<<17)
#define DB_OPT_BINDER_INFO              (1<<18)
#define DB_OPT_WCN_ISSUE_INFO           (1<<19)
#define DB_OPT_DUMMY_DUMP               (1<<20)
#define DB_OPT_PID_MEMORY_INFO          (1<<21)
#define DB_OPT_VM_OOME_HPROF            (1<<22)
#define DB_OPT_PID_SMAPS                (1<<23)
#define DB_OPT_PROC_CMDQ_INFO           (1<<24)
#define DB_OPT_PROC_USKTRK              (1<<25)
#define DB_OPT_SF_RTT_DUMP              (1<<26)
#define DB_OPT_PAGETYPE_INFO            (1<<27)

struct aee_exception_entry {
    struct timeval time;
    pid_t worker_pid;
    pid_t pid;
    AE_EXP_CLASS clasz;
};

#define AEE_WORKER_MAX 4

// AED Exported Functions

int aee_get_mode(void);

int aee_load_is_eng_built(void);

int aee_load_is_customer_built(void);

int aee_exception_running(struct aee_exception_entry rec[AEE_WORKER_MAX]);

/**
 * Check if aee is running
 * 
 * Return TRUE if system running AEE
 **/
extern int aee_aed_is_ready();

/**
 * Raise aa AEE dump
 **/
extern int aee_aed_raise_exception(AE_EXP_CLASS cls,
                                   pid_t pid,
                                   pid_t tid,
                                   const char* type,
                                   const char* process,
                                   const char* module,
                                   const char* backtrace,
                                   const char* detail_mem,
                                   const char* detail_file);

/**
 * Raise an AEE dump, this is simplify version of "aee_aed_raise_exception"
 *
 * @att: Specified defect level
 * @cls: 
 * @path: A file content which will include in db.xx(_exp_detail.txt), must not be NULL 
 * @suspect: Message describe defect reason
 * @pid: Process ID whose generate detect
 *
 * Return TRUE if raise detect successfully
 **/
extern int aee_aed_raise_defect(AE_DEFECT_ATTR att, 
                                AE_EXP_CLASS cls, 
                                const char* path, 
                                const char* suspect,
                                pid_t pid);

int aee_system_warning(const char *module, const char* path, unsigned int flags, const char *msg,...);
int aee_system_exception(const char *module, const char* path, unsigned int flags, const char *msg,...);
int aee_modem_warning(const char *module,const char* path,unsigned int flags, const char *modem_warning,  const char *modem_version);


/**
 * Raise an AEE system report
 * DONTUSE, unless used in restricted environment like dynamic linker
 */
int aee_system_report_JNI(AE_DEFECT_ATTR attr, const char *module, const char* backtrace, const char* path, const char *msg, unsigned int flags);

bool aee_ioctl_get_reg(pid_t tid, struct pt_regs *regs);
bool aee_try_get_word(pid_t pid, uintptr_t ptr, uint32_t *value);

#ifdef __cplusplus
}
#endif 

#endif
