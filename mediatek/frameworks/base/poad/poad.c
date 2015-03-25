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



#include <fcntl.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "POAD"
#endif

#define BOOT_MODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"
#define BOOT_REASON_SYS_PROPERTY "sys.boot.reason"
#define BOOT_PACKAGE_SYS_PROPERTY "persist.sys.bootpackage"

/*
 * return value:
 * 0: normal
 * 1: Alarm boot
 * 2: Schedule power-on
 */
void updateBootReason() {
    int fd;
    size_t s;
    char boot_mode;
    char boot_reason; // 0: normal boot, 1: alarm boot
    char propVal[PROPERTY_VALUE_MAX];

    fd = open(BOOT_MODE_PATH, O_RDONLY);
    if (fd < 0) {
        SLOGE("updateBootReason() fail to open: %s\n", BOOT_MODE_PATH);
        boot_reason = '0';
    } else {
        s = read(fd, (void *)&boot_mode, sizeof(boot_mode));
        close(fd);

        if(s <= 0) {
            SLOGE("can't read the boot_mode");
            boot_reason = '0';
        } else {
            // ALARM_BOOT = 7 
            SLOGE("boot_mode = %c", boot_mode);
            if ( boot_mode == '7' ) {
                boot_reason = '1';
                property_get(BOOT_PACKAGE_SYS_PROPERTY, propVal, "0");
                int package = atoi(propVal);
                SLOGE("boot package = %d", package);
                if ( package != 1 ) {
                    // it's not triggered by Desk Clock, change to normal boot
                    boot_reason = '0';
                }
            } else {
                boot_reason = '0';
            }
        }
    }
    sprintf(propVal, "%c", boot_reason);
    SLOGE("update boot reason = %s", propVal);
    property_set(BOOT_REASON_SYS_PROPERTY, propVal);
}

void main(void)
{
    printf("start poad service ....");
    updateBootReason();
}
