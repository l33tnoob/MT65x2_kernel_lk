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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>

#include "common.h"
#include "miniui.h"
#include "utils.h"
#include "ftm.h"

#include "hardware/ccci_intf.h"

#ifdef FEATURE_FTM_IDLE

#define TAG "[IDLE] "

#define mod_to_idle(p) (struct ftm_idle *)((char *)(p) + sizeof(struct ftm_module))

struct ftm_idle 
{
    struct ftm_module *mod;
    struct textview tv;
    text_t title;
};

typedef enum {
    FALSE = 0,
    TRUE,
} _BOOL;

#ifdef MTK_ENABLE_MD1
    int fd_atcmd;
#endif
#ifdef MTK_ENABLE_MD2
    int fd_atcmd2;
#endif
#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
    int fd_atcmd_dt;
#endif

#ifdef MTK_MD_SHUT_DOWN_NT
int fd_ioctl;
int fd_ioctlmd2;
extern int ExitFlightMode_PowerOffModem(int fd, int ioctl_fd, _BOOL bON);
#endif

extern int usb_com_port;
extern int usb_plug_in;

extern int ExitFlightMode(int fd, _BOOL bON);
extern int openDeviceWithDeviceName(char *deviceName);

static int idle_key_handler(int key, void *priv)
{
    return 0;
}

int idle_entry(struct ftm_param *param, void *priv)
{
    struct ftm_idle *idle = (struct ftm_idle *)priv;
    struct textview *tv = &idle->tv;

    int fd_suspend = -1, fd_backlight = -1, fd_mdm = -1;
    int ret = 0, key = 0, i = 0;

    char *s_state_mem = "mem";
    char *s_state_on = "on";
    char *s_backlight_on = "102";
    char *s_backlight_off = "0";
    char *s_mdm_txpwr_disable = "0";
    char *close_com_port = "close com port!";

    #ifdef MTK_ENABLE_MD1
    char ccci_md1_dev[32];
    #endif

    #ifdef MTK_ENABLE_MD2
    char ccci_md2_dev[32];
    #endif

    #ifdef MTK_MD_SHUT_DOWN_NT
    char ccci_md1_power_ioctl_dev[32];
    char ccci_md2_power_ioctl_dev[32];
    #endif

    const char *atcmd_ret;

    LOGD(TAG "%s: idle_entry\n", __FUNCTION__);

    init_text(&idle->title, param->name, COLOR_YELLOW);

    ui_init_textview(tv, idle_key_handler, (void*)idle);
    tv->set_title(tv, &idle->title);

    /* Make MD into flight mode */
    #ifdef MTK_ENABLE_MD1
        snprintf(ccci_md1_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE, MD_SYS1));
        LOGD(TAG "Open CCCI MD1 dev node: %s\n", ccci_md1_dev);
        fd_atcmd = openDeviceWithDeviceName(ccci_md1_dev);
        if (-1 == fd_atcmd)
        {
            LOGD(TAG "Fail to open CCCI interface\n");
            return 0;
        }
        for (i = 0; i < 30; i++) usleep(50000); // sleep 1s wait for modem bootup

        #ifdef MTK_MD_SHUT_DOWN_NT
            snprintf(ccci_md1_power_ioctl_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE_IOCTL, MD_SYS1));
            LOGD(TAG "Open CCCI MD1 Power IOCTL dev node: %s\n", ccci_md1_power_ioctl_dev);
            fd_ioctl = openDeviceWithDeviceName(ccci_md1_power_ioctl_dev);
            if (-1 == fd_ioctl)
            {
                LOGD(TAG "Fail to open CCCI IOCTL interface\n");
                return 0;
            }

            ExitFlightMode_PowerOffModem(fd_atcmd,fd_ioctl,FALSE);
        #else
            ExitFlightMode(fd_atcmd, FALSE);
        #endif
    #endif
    #ifdef MTK_ENABLE_MD2
        snprintf(ccci_md2_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE, MD_SYS2));
        LOGD(TAG "Open CCCI MD2 dev node: %s\n", ccci_md2_dev);
        fd_atcmd2 = openDeviceWithDeviceName(ccci_md2_dev);
        if (-1 == fd_atcmd2)
        {
            LOGD(TAG "Fail to open MD2 CCCI interface\n");
            return 0;
        }
        for (i = 0; i < 30; i++) usleep(50000); // sleep 1s wait for modem bootup

        #ifdef MTK_MD_SHUT_DOWN_NT
            snprintf(ccci_md2_power_ioctl_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE_IOCTL, MD_SYS2));
            LOGD(TAG "Open CCCI MD2 Power IOCTL dev node: %s\n", ccci_md2_power_ioctl_dev);
            fd_ioctlmd2 = openDeviceWithDeviceName(ccci_md2_power_ioctl_dev);
            if (-1 == fd_ioctlmd2)
            {
                LOGD(TAG "Fail to open CCCI MD2 IOCTL interface\n");
                return 0;
            }
            ExitFlightMode_PowerOffModem(fd_atcmd2,fd_ioctlmd2,FALSE);
        #else
            ExitFlightMode(fd_atcmd2, FALSE);
        #endif
    #endif
    #if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
        #if defined(PURE_AP_USE_EXTERNAL_MODEM)
            fd_atcmd_dt = openDeviceWithDeviceName(CCCI_MODEM_MT8135);
        #else
            fd_atcmd_dt= openDeviceWithDeviceName(CCCI_MODEM_MT6252);
        #endif

        if (-1 == fd_atcmd_dt)
        {
            LOGD(TAG "Fail to open CCCI interface\n");
            return 0;
        }
		
        #if defined(PURE_AP_USE_EXTERNAL_MODEM)
            initTermIO(fd_atcmd_dt, 5);
        #endif
		
        for (i = 0; i < 30; i++) usleep(50000); // sleep 1s wait for modem bootup

        ExitFlightMode(fd_atcmd_dt, FALSE);
    #endif

    /* Turn off thermal query MD TXPWR function */
    fd_mdm = open("/proc/mtk_mdm_txpwr/txpwr_sw", O_RDWR, 0);
    if (fd_mdm == -1) 
    {
        idle->mod->test_result = FTM_TEST_FAIL;
        LOGD(TAG "%s: cannot open /proc/mtk_mdm_txpwr/txpwr_sw, not support\n", __FUNCTION__);
    }
    else
    {
        ret = write(fd_mdm, s_mdm_txpwr_disable, strlen(s_mdm_txpwr_disable));
    }

    /* Turn off backlight */
    fd_backlight = open("/sys/class/leds/lcd-backlight/brightness", O_RDWR, 0);
    if (fd_backlight == -1) 
    {
        idle->mod->test_result = FTM_TEST_FAIL;
        LOGD(TAG "%s: cannot open /sys/class/leds/lcd-backlight/brightness\n", __FUNCTION__);
        return -1;
    }
    ret = write(fd_backlight, s_backlight_off, strlen(s_backlight_off));

    /* Make AP enter sleep mode */
    fd_suspend = open("/sys/power/state", O_RDWR, 0);
    if (fd_suspend == -1) 
    {
        idle->mod->test_result = FTM_TEST_FAIL;
        LOGD(TAG "%s: cannot open /sys/power/state\n", __FUNCTION__);
        return -1;
    }
    ret = write(fd_suspend, s_state_mem, strlen(s_state_mem));

    if(!get_is_ata())
    {
        while (1)
        {
            key = ui_wait_phisical_key();
            LOGD(TAG "%s: %d\n", __FUNCTION__, key);

            ret = write(fd_suspend, s_state_on, strlen(s_state_on));
            close(fd_suspend);
            LOGD(TAG "%s: exit from suspend\n", __FUNCTION__);

            break;
        }
    }
    else
    {
        int write_len = write(usb_com_port, close_com_port, strlen(close_com_port));
        LOGD(TAG "after write data to pc\n");
        if(write_len != strlen(close_com_port))
        {
            LOGD(TAG "write data to pc fail\n");
        }

        close_usb();

        while(is_USB_State_PlugIn())
        {
            sleep(1);
        }

        sleep(2);

        while(!is_USB_State_PlugIn())
        {
            sleep(1);
        }

        ret = write(fd_suspend, s_state_on, strlen(s_state_on));
        close(fd_suspend);
        LOGD(TAG "%s: exit from suspend\n", __FUNCTION__);

        usb_plug_in = 1;
    }

    close(fd_mdm);

    /* Turn on backlight */
    ret = write(fd_backlight, s_backlight_on, strlen(s_backlight_on));
    close(fd_backlight);

    #ifdef MTK_ENABLE_MD1
        #ifdef MTK_MD_SHUT_DOWN_NT
            ExitFlightMode_PowerOffModem(fd_atcmd,fd_ioctl,TRUE);
            closeDevice(fd_ioctl);
        #endif
        closeDevice(fd_atcmd);
    #endif
    #ifdef MTK_ENABLE_MD2
        #ifdef MTK_MD_SHUT_DOWN_NT
            ExitFlightMode_PowerOffModem(fd_atcmd2,fd_ioctlmd2,TRUE);
            closeDevice(fd_ioctlmd2);
        #endif
        closeDevice(fd_atcmd2);
    #endif
    #if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
        closeDevice(fd_atcmd_dt);
    #endif

    idle->mod->test_result = FTM_TEST_PASS;
    return 0;
}

int idle_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct ftm_idle *idle;

    LOGD(TAG "idle_init\n");

    mod = ftm_alloc(ITEM_IDLE, sizeof(struct ftm_idle));
    if (!mod)
        return -ENOMEM;

    idle = mod_to_idle(mod);
    idle->mod = mod;

    ret = ftm_register(mod, idle_entry, (void*) idle);
    if (ret) {
        LOGD(TAG "register IDLE failed (%d)\n", ret);
        return ret;
    }

    return 0;
}

#endif
