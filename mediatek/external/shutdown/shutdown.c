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
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>

#include <signal.h>
#include <dirent.h>
#include <cutils/properties.h>
#include <utils/Log.h>

#define BKL_LCD_PATH "/sys/class/leds/lcd-backlight/brightness"

extern void kill_process(const char *path);

char *service_name[]={"runtime", "zygote", "keystore", "adbd","pppd",
			"wpa_supplicant", "p2p_supplicant1", "p2p_supplicant0",
			"nvram_daemon","NvRAMAgent","bluetoothd","racoon",
			"mtpd","systemkeys","ipod", "bootlogoupdater",
			"ueventd","pvrsrvinit","mtpd","mnld",
			"netd","netdiag", "mobile_log_d","debuggerd",
			"media","bootanim","dbus", "6620_launcher","mtkbt",
			"ccci_fsd", "ccci_mdinit", "pppd_gprs","gsm0710muxd",
			"muxreport-daemon", "ril-daemon", "atci-daemon",
			"audio-daemon", "installd", "wlaninit", "dhcpcd",
			"agpsd", "emsvr", "afmsvr", "mdlogger","" };

void turn_off_backlight()
{
    int fd = open(BKL_LCD_PATH, O_RDWR);
    if (fd == -1) {
		reboot(RB_POWER_OFF);
    }
    write(fd, "0", 1);	
    close(fd);
}

//#define FSCK_TUNE
#ifdef MTK_FSCK_TUNE
static char shutdown_flag[]="shutdown_flag";
static char shutdown_value_normal[]="normal";
static char shutdown_value_exception[]="exception";
#define FLAG_SHUTDOWN 0x1
#define FLAG_POWER_CUT 0x0
#define MAX_SHUTDOWN_VALUE_LEN (sizeof(shutdown_value_exception)>sizeof(shutdown_value_normal)? sizeof(shutdown_value_exception):sizeof(shutdown_value_normal) )
#define SHUTDOWN_FLAG_LEN (sizeof(shutdown_flag))
#define DATA_PATH "/emmc@usrdata" 
struct env_ioctl
{
	char *name;
	int name_len;
	char *value;
	int value_len;	
};

#define ENV_MAGIC	 'e'
#define ENV_READ		_IOW(ENV_MAGIC, 1, int)
#define ENV_WRITE 		_IOW(ENV_MAGIC, 2, int)

#endif
#ifdef MTK_FSCK_TUNE
void mark_shutdown()
{
	struct env_ioctl shutdown;
	int env_fd;
	int ret;

	if((env_fd = open("/proc/lk_env", O_RDWR)) < 0) {
		SLOGE("Open env for format check fail.\n");
		goto FAIL_RUTURN;
	}
	if(!(shutdown.name = calloc(SHUTDOWN_FLAG_LEN, sizeof(char)))) {
		SLOGE("Allocate Memory for env name fail.\n");
		goto FREE_FD;
	}
	if(!(shutdown.value = calloc(MAX_SHUTDOWN_VALUE_LEN, sizeof(char)))) {
		SLOGE("Allocate Memory for env value fail.\n");
		goto FREE_ALLOCATE_NAME;
	}
	shutdown.name_len = SHUTDOWN_FLAG_LEN;
	shutdown.value_len = MAX_SHUTDOWN_VALUE_LEN;
	memcpy(shutdown.name, shutdown_flag, sizeof(shutdown_flag));
	memcpy(shutdown.value, shutdown_value_normal, sizeof(shutdown_value_normal));
	if(ret = ioctl(env_fd, ENV_WRITE, &shutdown)) {
		SLOGE("write env for shutdown flag fail.ret = %d, errno = %d\n", ret, errno);
		goto FREE_ALLOCATE_VALUE;
	}

	SLOGE("Successfully clear shut down flag.\n");
	free(shutdown.name);
	free(shutdown.value);
	close(env_fd);
	return 0; 
FREE_ALLOCATE_VALUE:
	free(shutdown.value);
FREE_ALLOCATE_NAME:
	free(shutdown.name);
FREE_FD:
	close(env_fd);
FAIL_RUTURN:
	return 1;
}
#endif
int main(int argc, char **argv)
{
    int i = 0;
    
    turn_off_backlight();
	
    while ( strcmp (service_name[i],"") ) {
        property_set("ctl.stop", service_name[i]);
	i ++;
    }
        
    sleep(1);

    kill_process("/data");    
    sleep(1);
    sync();
    umount2("/data",2);
    umount2("/cache",2);
#ifdef MTK_FSCK_TUNE
	mark_shutdown();
#endif

    reboot(RB_POWER_OFF);
    return 0;
}

