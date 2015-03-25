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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <cutils/xlog.h>
#include <cutils/properties.h>
#include <utils/Log.h>

#include <charging_animation.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "BootLogoUpdater"
#endif


#define BOOT_MODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"
#define LCD_BACKLIGHT_PATH "/sys/class/leds/lcd-backlight/brightness"
#define BOOT_REASON_SYS_PROPERTY "sys.boot.reason"
#define BOOT_PACKAGE_SYS_PROPERTY "persist.sys.bootpackage"

    
/*
 * return value:
 * 0: normal
 * 1: Alarm boot
 * 2: Schedule power-on
 */
int update_boot_reason() {
    int fd;
    size_t s;
    char boot_mode;
    char boot_reason; // 0: normal boot, 1: alarm boot
    char propVal[PROPERTY_VALUE_MAX];
    int ret = 0;

    fd = open(BOOT_MODE_PATH, O_RDWR);
    if (fd < 0) {
        printf("[boot_logo_updater]fail to open: %s\n", BOOT_MODE_PATH);
        boot_reason = '0';
    } else {
        s = read(fd, (void *)&boot_mode, sizeof(boot_mode));
        close(fd);
            
        if(s <= 0) {
           printf("[boot_logo_updater]can't read the boot_mode\n");
           boot_reason = '0';
        } else {
            // ALARM_BOOT = 7 
            printf("[boot_logo_updater]boot_mode = %c\n", boot_mode);
            if ( boot_mode == '7' ) {
                //add for encrypt mode to avoid invoke the power-off alarm
                property_get("vold.decrypt", propVal, "");
                if (!strcmp(propVal, "") || !strcmp(propVal, "trigger_restart_framework")) {
                    boot_reason = '1';
                    ret = 1;
                } else {
                    boot_reason = '0';
                    ret = 2;
                }
            } else {
                // schedule on/off, normal boot
                property_get(BOOT_PACKAGE_SYS_PROPERTY, propVal, "0");
                int package = atoi(propVal);
                printf("[boot_logo_updater]boot package = %d\n", package);
                if ( package != 1 ) {
                    // it's not triggered by Desk Clock, change to normal boot
                    ret = 2;
                }
                boot_reason = '0';
            }
       }
    }
    sprintf(propVal, "%c", boot_reason);
    printf("[boot_logo_updater]update boot reason = %s, ret = %d\n", propVal, ret);
    property_set(BOOT_REASON_SYS_PROPERTY, propVal);
    return ret;
}


int write_to_file(const char* path, const char* buf, int size) {

    if (!path) {
        printf("[boot_logo_updater]path is null!\n");
        return 0;
    }

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        printf("[boot_logo_updater]Could not open '%s'\n", path);
        return 0;
    }

    int count = write(fd, buf, size); 
    if (count != size) {
        printf("[boot_logo_updater]write file (%s) fail, count: %d\n", path, count);
        return 0;
    }
    close(fd);
    return count;
}

void set_int_value(const char * path, const int value)
{
    char buf[32];
    sprintf(buf, "%d", value);

    write_to_file(path, buf, strlen(buf));
}


int main(void)
{
    
    printf("[boot_logo_updater %s %d]boot_logo_updater,\n",__FUNCTION__,__LINE__);
    int ret = update_boot_reason();
    if (ret == 1) {
        printf("[boot_logo_updater]skip the boot logo!\n");
        set_int_value(LCD_BACKLIGHT_PATH, 120);
        return 0;    
    } else if (ret == 2) {
        printf("[boot_logo_updater]schedule on\n");     
    }
    // set parameter before init
    set_draw_mode(DRAW_ANIM_MODE_FB);    
    anim_init();
    show_kernel_logo();
    anim_deinit();
    
    return 0;
}
