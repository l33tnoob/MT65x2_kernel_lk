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

#ifndef _IPODMAIN_H
#define _IPODMAIN_H


#include <cutils/xlog.h>
#include <cutils/properties.h>
#include <bootlogo.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "IPODMAIN"
#endif

/**********************************************************************
 * IPO-H mode                                                         *
 * set IPOH_MODE to :                                                 *
 * 0 : disable IPO-H mode                                             *
 * 1 : IPO-H mode with hibernation                                    *
 * 2 : IPO-H mode with normal shutdown (original power off feature)   *
 *************************** ******************************************/
#define IPOH_MODE 1
#define IPO_DEFAULT_POWEROFF_TIME (30*60) //sec

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

enum {
    CONTROL_UNKNOWN        = 0,
    CONTROL_ON             = 1,
    CONTROL_OFF            = 2,
};

// do not change the order
enum {
    LIGHTS_INIT        = 0,
    LIGHTS_CHGFULL     = 1,
    LIGHTS_CHGON       = 2,
    LIGHTS_CHGEXIT     = 3,
    LIGHTS_FUNC_NUM    = 4,
};

enum {
    EVENT_DUMMY        = 0,
    EVENT_PREBOOT_IPO  = 1,
    EVENT_BOOT_IPO     = 2,
    EVENT_ALARM_RTC    = 3,
    EVENT_EXIT_IPOD    = 4,
    EVENT_KEY_PRESS    = 5,
    EVENT_UEVENT_IN    = 6,
    EVENT_DRAW_CHARGING_ANIM = 7,
    EVENT_LOWBATT_FAIL_BOOT = 8,
    EVENT_RADIOOFF_CHECK = 9,
    EVENT_REQUEST_BLANK = 10,
    EVENT_REQUEST_UNBLANK = 11,
    EVENT_AMOUNT       = 12,

};

static const char *event_name[] = {
#define S(id) #id
    S(EVENT_DUMMY),             /* 0 */
    S(EVENT_PREBOOT_IPO),       /* 1 */
    S(EVENT_BOOT_IPO ),         /* 2 */
    S(EVENT_ALARM_RTC),         /* 3 */
    S(EVENT_EXIT_IPOD),         /* 4 */
    S(EVENT_KEY_PRESS),         /* 5 */
    S(EVENT_UEVENT_IN),         /* 6 */
    S(EVENT_DRAW_CHARGING_ANIM),/* 7 */
    S(EVENT_LOWBATT_FAIL_BOOT), /* 8 */
    S(EVENT_RADIOOFF_CHECK),    /* 9 */
    S(EVENT_REQUEST_BLANK),     /* 10 */
    S(EVENT_REQUEST_UNBLANK),   /* 11 */
};

enum {
    EXIT_POWER_UP         = 0,
    EXIT_REBOOT_UBOOT     = 1,
    EXIT_ERROR_SHUTDOWN   = 2,
    EXIT_DISABLE_IPOD     = 3,
    EXIT_DISABLE_IPOD_PROP = 4,
    EXIT_ALARM_BOOT		  = 5,
    EXIT_LOW_BATTERY	  = 6,
    EXIT_PM_FAIL          = 7,
    EXIT_CHARGING_MODE    = 8,
    EXIT_HIB_BOOT         = 9,
    EXIT_AMOUNT           =10,
};

enum {
    PARAM_IPO_VER        = 0, /*p0*/
    PARAM_BK_SKEW        = 1, /*p1*/
    PARAM_CHG_DUR        = 2, /*p2*/
    PARAM_CHG_CB_DURA    = 3, /*p3*/
    PARAM_BOOTTYPE_NOLOGO = 4, /*p4*/
    PARAM_NOLOGO          = 5, /*p5*/
    PARAM_BK_LEVEL		  = 6, /*p6*/
    PARAM_PWROFF_TIME     = 7, /*p7*/
    PARAM_BKL_ON_DELAY    = 8, /*p8*/
    PARAM_POWER_OFF_VOLTAGE = 9, /*9*/
    PARAM_POWER_ON_VOLTAGE = 10, /*10*/
    PARAM_TB_WIFI_ONLY = 11, /*11*/
    PARAM_CHARGING_MODE    = 12, /* 12 */
    PARAM_FAST_REBOOT = 13,
    PARAM_RESET_MODEM       =14,
    PARAM_IPOH_MODE         = 15,
    PARAM_KERN_LOGGING      = 16,
    PARAM_AMOUNTS         = 17,
};

enum {
    TRIGGER_ANIM_START       = 0,
    TRIGGER_ANIM_START_RESET = 1,
    TRIGGER_ANIM_STOP        = 2,
    TRIGGER_NORMAL_BOOT	     = 3,
    TRIGGER_ALARM_BOOT       = 4,
    TRIGGER_REBOOT           = 5,
    TRIGGER_ANIM_INIT        = 6,
    TRIGGER_ANIM_UEVT        = 7,
    TRIGGER_ANIM_KEY         = 8,
};


int lights_chgfull();
int lights_chgon();
int lights_chgexit();
void updateChargingMode();

void set_int_value(const char * path, const int value);
extern pthread_mutex_t mutex;
extern void (*ipod_trigger_chganim)(int);
extern int status_cb(int, int, int);
extern long params[];
extern int inCharging;
extern void radiooff_check();
extern void finish_shutdown();
extern int get_ov_status();
extern int get_battnotify_status();
extern void updateTbWifiOnlyMode();
extern int showLowBattLogo;
extern void updateFastRebootMode();
extern void setIPOBklOnDelay(void);
extern int is_wireless_charging();

// IPO-H
extern struct timespec ts_setOff;
// -- end of IPO-H

// for tablet customization
#if defined(MTK_IPO_POWERPATH_SUPPORT) || defined(MTK_BATLOWV_NO_PANEL_ON_EARLY)
#define AC_ONLINE_PATH "/sys/class/power_supply/ac/online"
#define BOOTMODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"
#define BATTERY_VOLTAGE_PATH "/sys/class/power_supply/battery/batt_vol"
#define USB_POWER_ON_VOLTAGE 3450

extern int get_int_value(const char * path);
extern void updateLogoBacklightVoltageMode(long params[]);
#endif

#ifndef MTK_RESET_MODEM
#define MTK_RESET_MODEM (0)
#endif

#endif
