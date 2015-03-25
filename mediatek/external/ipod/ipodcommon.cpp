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
#include <utils/Log.h>
#include <fcntl.h>
#include "ipodmain.h"
#include <cutils/properties.h>
#include <linux/android_alarm.h>
#include <sys/reboot.h>
#include <errno.h>
#include <time.h>

#define RADIO_TIMEOUT_SEC 30
#define IPOD_POWER_DOWN_CAP "sys.ipo.pwrdncap"
#define IPOD_RADIO_OFF_STATE "ril.ipo.radiooff"
#define IPOD_RADIO_OFF_STATE2 "ril.ipo.radiooff.2"
#define IPOD_RADIO_WAKELOCK "IPOD_RADIO_WAKELOCK"
#define IPOD_SHUTDOWN_WAKELOCK "IPOD_SHUTDOWN_WAKELOCK"
#define IPOD_FINISH_SHUTDOWN_AWAKE_TIME (20)

#define WAKELOCK_ACQUIRE_PATH "/sys/power/wake_lock"
#define WAKELOCK_RELEASE_PATH "/sys/power/wake_unlock"

#define CHARGER_OVER_VOLTAGE 7000
#define CHARGER_VOLTAGE_PATH "/sys/class/power_supply/battery/ChargerVoltage"
#define WIRELESS_CHARGING "/sys/class/power_supply/wireless/online"
#define CHARGER_BATTNOTFY_PATH "/sys/devices/platform/mt-battery/BatteryNotify"

#define IPO_FAST_REBOOT "sys.ipo.fast_reboot"
#define IPO_BKL_ON_DELAY "sys.ipo.bklondelay"

/*
 * IPOD_RADIO_OFF_STATE
 * value:
 *       0: default value, dont bypass
 *       1: ShutdownThread ignores mountservice.shutdown only
 *       2: ShutdownThread ignores wait radio off. IPO will take over this job.
 *       3: ShutdownThread ignores both mountservice.shutdown and wait radio off. IPO will take over this job.
 */

// -- IPO-H
#define IPOH_MODE_ENABLE_PROP "ipo.ipoh.enable"
#define POWEROFF_ALARM_CHECK_TOLERANCE 60 //sec
#define POWEROFF_ALARM_TRIGGER_TOLERANCE 60 //sec
#define SYS_POWER_OFF_ALARM "sys.power_off_alarm"
#define IPO_POWER_OFF_TIME "ipo.power_off_time"
#define IPOH_PWROFFTIME "persist.ipoh.pot"
struct timespec ts_pwroff, ts_setOff;
int setOff = true;
// -- end of IPO-H

#define BOOTMODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"

int inCharging = false;
int showLowBattLogo = false;
int inExiting = false;
int radiooff_check_done = false;

#define VERBOSE_OUTPUT

/******/
// code comes from rild to judge if it is in DualTalkMode
#define PROPERTY_TELEPHONY_MODE "ril.telephony.mode"
static int getExternalModemSlot() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.external.md", property_value, "0");
    return atoi(property_value)-1;
}

static int isInternationalRoamingEnabled() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.evdo.irsupport", property_value, "0");
    return atoi(property_value);
}

static int getTelephonyMode() {
    char mode[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_TELEPHONY_MODE, mode, 0);
    if (strlen(mode) > 0)
        return atoi(mode);
    else
        return 0;
}

static int isEvdoOnDualtalkMode() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("mediatek.evdo.mode.dualtalk", property_value, "1");
    int mode = atoi(property_value);
    SXLOGI("evdoOnDualtalkMode mode: %d", mode);
    return mode;
}

static int isDualTalkMode() {
    if (isInternationalRoamingEnabled()) {
        return isEvdoOnDualtalkMode();
    }

    int telephonyMode = getTelephonyMode();
    if (telephonyMode == 0) {
        return (getExternalModemSlot() >= 0);
    } else if (telephonyMode >= 5) {
        return 1;
    } else {
        return 0;
    }
}
/******/


int write_to_file(const char* path, const char* buf, int size)
{
    if (!path) {
        SXLOGE("null path to write");
        return 0;
    }
#ifdef VERBOSE_OUTPUT
    SXLOGI("%s: path: %s, buf: %s, size: %d\n",__FUNCTION__, path ,buf, size);
#endif

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        SXLOGE("Could not open '%s'\n", path);
        return 0;
    }

    int count = write(fd, buf, size); 
    if (count != size) {
        SXLOGE("write file (%s) fail, count: %d\n", path, count);
        return 0;
    }

    close(fd);
    return count;
}


void set_int_value(const char * path, const int value)
{
    char buf[32];
    sprintf(buf, "%d", value);
#ifdef VERBOSE_OUTPUT
    SXLOGI("%s: %s, %s \n",__FUNCTION__, path ,buf);
#endif
    write_to_file(path, buf, strlen(buf));
}

/*   return value:
 *         0, error or read nothing
 *        !0, read counts
 */
int read_from_file(const char* path, char* buf, int size)
{
    if (!path) {	
        return 0;
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return 0;
    }

    int count = read(fd, buf, size); 
    if (count > 0) {
        count = (count < size) ? count : size - 1;
        while (count > 0 && buf[count-1] == '\n') count--;
        buf[count] = '\0';
    } else {
        buf[0] = '\0';
    }

    close(fd);
    return count;
}

int get_int_value(const char * path)
{
    int size = 32;
    char buf[size];
    if(!read_from_file(path, buf, size))
        return 0;
    return atoi(buf);
}

void set_str_value(const char * path, const char * str)
{
    write_to_file(path, str, strlen(str));
}

void acquire_wakelock(const char *str)
{

    set_str_value(WAKELOCK_ACQUIRE_PATH, str);
}

void release_wakelock(const char *str)
{
    set_str_value(WAKELOCK_RELEASE_PATH, str);
}

static void* test_thread_routine(void *arg)
{	
    while (1) 
    {
        if (ipod_trigger_chganim == NULL )
        {
            SXLOGE("ipod_trigger_chganim is NULL, test abort");
            return 0;
        }

        SXLOGI(" trigger charge animation without reset timer after 4 sec");
        usleep(4*1000*1000);
        SXLOGI(" trigger charge animation without reset timer");
        ipod_trigger_chganim(0);
        usleep(20*1000*1000);

        SXLOGI(" trigger charge animation with reset timer after 4 sec");
        usleep(4*1000*1000);
        SXLOGI(" trigger charge animation with reset timer");
        ipod_trigger_chganim(1);
        usleep(20*1000*1000);
    }
    return 0;
}

void test_trigger()
{
    int ret = 0;
    pthread_attr_t attr;
    pthread_t test_thread;

    pthread_attr_init(&attr);

    ret = pthread_create(&test_thread, &attr, test_thread_routine, NULL);
    if (ret != 0) 
    {
        SXLOGE("create test pthread failed.\n");
    }
}

// -- IPO-H
#define IPO_MIN_POWEROFF_TIME (RADIO_TIMEOUT_SEC+20) //sec
long getIPOPowerOffTime(void)
{
    char buf[PROPERTY_VALUE_MAX];
    unsigned long time = 0;
    unsigned long ptime = 0;

    if(property_get(IPO_POWER_OFF_TIME, buf, "0")) {
        time = strtoul(buf, NULL, 10);
        if (time > 0) {
            SXLOGI("found ipo.power_off_time = %lu\n", time);
            time = (time > IPO_MIN_POWEROFF_TIME) ? time : IPO_MIN_POWEROFF_TIME;
            sprintf(buf, "%lu", time);
            property_set(IPOH_PWROFFTIME, buf);
        }
    }

    if (property_get(IPOH_PWROFFTIME, buf, "0")) {
        ptime = strtoul(buf, NULL, 10);
        if (ptime > 0)
            SXLOGI("found persist.ipoh.pot = %lu \n", ptime);
        ptime = (ptime > 0) ? ptime : IPO_DEFAULT_POWEROFF_TIME;
    }

    return (ptime < IPO_MIN_POWEROFF_TIME ? IPO_MIN_POWEROFF_TIME : ptime);
}

int getPowerOffAlarm()
{
    char buf[PROPERTY_VALUE_MAX];
    unsigned long time = 0;
    if(property_get(SYS_POWER_OFF_ALARM,buf,"0")) {
        time = strtoul(buf, NULL, 10);
        if(time > 0) {
            ts_pwroff.tv_sec = time;
            ts_pwroff.tv_nsec = 0;
            SXLOGI("found power off alarm: %lu \n",time);
            return true;
        }
    }
    return false;
}

int clearPowerOffAlarmProperty()
{
    char buf[PROPERTY_VALUE_MAX];
    unsigned long time = 0;
    if(property_get(SYS_POWER_OFF_ALARM,buf,"0")) {
        time = strtoul(buf, NULL, 10);
        if(time > 0) {
            SXLOGI("reset power off alarm systemproperty\n");
            property_set(SYS_POWER_OFF_ALARM,"0");
            return true;
        }
    }
    return false;
}

int getTime(struct timespec *ts)
{
    time_t t;

    time(&t);
    ts->tv_sec = t;

    SXLOGI("getTime: %ld", ts->tv_sec);
    return true;
}

void enablePowerOff(long offTimeSec)
{
    setOff = true;

    if (getTime(&ts_setOff)) {
        if (getPowerOffAlarm()) {
            // we have power off alarm set from AlarmManagerService.
            if ((ts_pwroff.tv_sec - ts_setOff.tv_sec) < (offTimeSec + POWEROFF_ALARM_CHECK_TOLERANCE)) {
                // If the power on alarm is set before the offTimeSec-60sec,
                // no need to set extra alarm to power off device because we will get power on
                // before we want to real power off in IPO.
                // The 60sec is the tolerance time. Suggest >= 60sec.
                setOff = false;
                ts_setOff.tv_sec = 0;
            }
        }
        if (setOff) {
            ts_setOff.tv_sec += offTimeSec;
            params[PARAM_PWROFF_TIME] = ts_setOff.tv_sec;
            SXLOGI("set power off time: %ld", params[PARAM_PWROFF_TIME]);
        }
    }
}

void checkPowerOff()
{
    struct timespec ts;
    if (setOff) {
        if (getTime(&ts)) {
            SXLOGI("checkPowerOff, now: %ld, set %ld", ts.tv_sec, ts_setOff.tv_sec);
            if (labs(ts.tv_sec - ts_setOff.tv_sec) < POWEROFF_ALARM_TRIGGER_TOLERANCE) {
                // If alarm is triggered and the trigger time is +-POWEROFF_ALARM_CHECK_TOLERANCE 
                // sec to the expected real power off time, do the power off procedure.
                SXLOGI("IPO-H shutdown device...");
                //reboot(RB_POWER_OFF);
                sleep(3); // delay to collect log before shutting down
                property_set("ctl.start", "shutdown"); // use shutdown thead to power down.
                while(1) usleep(1000*1000);
            }
        }
    }
}

void checkIPOHMode(void)
{
    char buf[PROPERTY_VALUE_MAX];

    // runtime setting first
    if (!property_get(IPOH_MODE_ENABLE_PROP, buf, NULL)) {
        sprintf(buf, "%d", IPOH_MODE);
    }

#if !defined(MTK_IPOH_SUPPORT)
    // IPO-H feature disabled, force to disable mode
    sprintf(buf, "%d", 0);
#endif

    property_set(IPOH_MODE_ENABLE_PROP, buf);
    params[PARAM_IPOH_MODE] = (long) atoi(buf); // passing to libipod

    SXLOGI("IPO-H: mode (%d)", atoi(buf));

    // set power off time, if ipoh is enable
    if (isdigit(buf[0]) && atoi(buf) != 0) {
        enablePowerOff(getIPOPowerOffTime());
    }
}

int getIPOHMode(void)
{
    char buf[PROPERTY_VALUE_MAX];
    int mode;

    // re-confirm the value consistence
    property_get(IPOH_MODE_ENABLE_PROP, buf, "0");
    mode = isdigit(buf[0]) ? atoi(buf) : 0;
    if (mode != params[PARAM_IPOH_MODE]) {
        SXLOGI("IPO-H: sys. property value (%d), params[PARAM_IPOH_MODE] (%d) is in-consistent!!\n",
               mode, params[PARAM_IPOH_MODE]);
    }

    return (int) params[PARAM_IPOH_MODE];
}
// -- end of IPO-H

void updateTbWifiOnlyMode()
{
    params[PARAM_TB_WIFI_ONLY] = 1;
}

void updateChargingMode()
{
    int val = get_int_value(BOOTMODE_PATH);

    if (val==8 || val==9) {
        params[PARAM_CHARGING_MODE] = 1;
        SXLOGI("IPOD under charging mode!");
        // enable logging into kernel log 
        // for KPOC issues debugging
        params[PARAM_KERN_LOGGING] = 1;
        set_draw_anim_mode(1);
    }
}

void setIPOBklOnDelay(void)
{
    char buf[PROPERTY_VALUE_MAX];
    int delay = params[PARAM_BKL_ON_DELAY];

    if (property_get(IPO_BKL_ON_DELAY, buf, "0"))
    {
        delay = atoi(buf);
        if (delay)
        {
            params[PARAM_BKL_ON_DELAY]= delay;
            SXLOGI("update BLK_ON_DELAY to %d", delay);
        }
    }
}

int status_cb(int event, int data1, int data2)
{
    /*
     * DO NOT BLOCK THIS FUNCTION!
     */

    int val = 0;
	SXLOGI("status_cb: %s(%d), %d, %d", 
            (event > EVENT_DUMMY && event < EVENT_AMOUNT)? event_name[event] : "unknown_event",
            event, data1, data2); 

    switch (event) {
        case EVENT_PREBOOT_IPO:
            if (getIPOHMode() != 0) {
                if (data1 == 1) //after preboot_ipo intent is sent.
                    clearPowerOffAlarmProperty();
            }
            break;

        case EVENT_BOOT_IPO:
            break;

        case EVENT_ALARM_RTC:
            if (getIPOHMode() != 0) {
                checkPowerOff();
            }
            break;

        case EVENT_DRAW_CHARGING_ANIM:
            inCharging = data1; // 1: in charging
            if (data1 == 0)
                showLowBattLogo = 0;
            break;

        case EVENT_LOWBATT_FAIL_BOOT:
            // libipod indicate that user long press power key in low power state

#ifdef MTK_KERNEL_POWER_OFF_CHARGING 			
#if defined(MTK_IPO_POWERPATH_SUPPORT) || defined(MTK_BATLOWV_NO_PANEL_ON_EARLY)
            if (!get_int_value(AC_ONLINE_PATH)) {
                SXLOGI("NOT AC charger"); 
                return 0; // NO any display
            }
#endif           
#endif            
            showLowBattLogo = 1;
            ipod_trigger_chganim(0);
            break;

            case EVENT_KEY_PRESS:
#if 0
            if (data1 == KEY_HP && 
                    data2 == 1 &&
                    inCharging)
                ipod_trigger_chganim(TRIGGER_ANIM_STOP);
#endif
            break;

        case EVENT_UEVENT_IN:
#if 0
            // example when USB cable is, boot up the device
#define USB_ONLINE_PATH "/sys/class/power_supply/usb/online"
            if (get_int_value(USB_ONLINE_PATH)) {
                ipod_trigger_chganim(TRIGGER_NORMAL_BOOT);
            }
#endif
            break;

        case EVENT_EXIT_IPOD:
            inExiting = true;
            // in case the releasing wakelock thread has no chance to execute
            release_wakelock(IPOD_SHUTDOWN_WAKELOCK);
            if(params[PARAM_FAST_REBOOT])
                val = property_set(IPO_FAST_REBOOT, "0");
            break;

        case EVENT_RADIOOFF_CHECK:
            val = radiooff_check_done;
            break;

        default:
            break;
    }
    return val;
}

int getPowerDownCap()
{
    char buf[PROPERTY_VALUE_MAX];
    unsigned int value = 0;
    if(property_get(IPOD_POWER_DOWN_CAP, buf, "0")) {
        value = atoi(buf);
        if(value == 2 || value == 3) {
            SXLOGI("radio off check is on (%d) \n",value);
            return true;
        }
    }
    SXLOGI("radio off check is off (%d) \n",value);
    return false;
}

int getRadioOffState()
{
    char buf[PROPERTY_VALUE_MAX];
    unsigned int value = 0;
    if(property_get(IPOD_RADIO_OFF_STATE, buf, "0")) {
        value = atoi(buf);
        return value;
    }
    return false;
}

int getDualTalkRadioOffState(){
    char buf[PROPERTY_VALUE_MAX];
    unsigned int radiooff = 0;
    unsigned int radiooff2 = 0;

    if(property_get(IPOD_RADIO_OFF_STATE, buf, "0")) {
        radiooff = atoi(buf);
    }
    if(property_get(IPOD_RADIO_OFF_STATE2, buf, "0")) {
        radiooff2 = atoi(buf);
    }
    if(radiooff && radiooff2)
        return true;
    else 
        return false;
}

static void* radiooff_check_routine(void *arg)
{	
    int i=0;
    do {
        sleep(1);
        int DualTalkMode = isDualTalkMode();
        if ((DualTalkMode && getDualTalkRadioOffState()) ||
                (!DualTalkMode && getRadioOffState())){
            SXLOGI("In %s, radio off done (%d sec).\n",
                    DualTalkMode?"DualTalkMode":"non-DualTalkMode",
                    i);
            radiooff_check_done = 1;
            release_wakelock(IPOD_RADIO_WAKELOCK);
            pthread_exit(NULL);
        }
    }while(++i < RADIO_TIMEOUT_SEC);

    if(0 != params[PARAM_FAST_REBOOT]){
        SXLOGE("radio off timeout (%d sec), reboot for fast_reboot mode\n", RADIO_TIMEOUT_SEC);
        reboot(LINUX_REBOOT_CMD_RESTART);
    }else if(!inExiting){
        SXLOGE("radio off timeout (%d sec), shutdown\n", RADIO_TIMEOUT_SEC);
        sleep(3);
        property_set("ctl.start", "shutdown");
    }else
        SXLOGE("radio off timeout but exiting.\n");

    return 0;
}

void radiooff_check()
{
    int ret = 0;
    pthread_attr_t attr;
    pthread_t checkradiooff_thread;

    if (!getPowerDownCap())
        return;

    acquire_wakelock(IPOD_RADIO_WAKELOCK);
    pthread_attr_init(&attr);

    ret = pthread_create(&checkradiooff_thread, &attr, radiooff_check_routine, NULL);
    if (ret != 0) 
    {
        SXLOGE("create radio check pthread failed.\n");
        sleep(3); // delay to collect log before shutting down
        property_set("ctl.start", "shutdown");
    }
}

static void* release_wakelock_routine(void *arg){
    unsigned int i = 0;

    do{
        i++;
        sleep(1);
    }while(!inExiting && i < IPOD_FINISH_SHUTDOWN_AWAKE_TIME);

    SXLOGI("release %s in %u s...\n", IPOD_SHUTDOWN_WAKELOCK, i);
    release_wakelock(IPOD_SHUTDOWN_WAKELOCK);
    return 0;
}

void finish_shutdown(){
    int ret = 0;
    pthread_attr_t attr;
    pthread_t release_wakelock_thread;

    acquire_wakelock(IPOD_SHUTDOWN_WAKELOCK);
    ret = pthread_create(&release_wakelock_thread, &attr, release_wakelock_routine, NULL);
    if (ret != 0) 
    {
        SXLOGE("create finish shutdown pthread failed.\n");
        sleep(3); // delay to collect log before shutting down
        property_set("ctl.start", "shutdown");
    }
}


/*
 * return value:
 *     1: over voltage
 *     0: normal voltage
 */
int get_ov_status()
{
    int voltage = get_int_value(CHARGER_VOLTAGE_PATH);
    SXLOGI("charger voltage: %d\n",voltage);

    if (voltage >= CHARGER_OVER_VOLTAGE) {
        return 1;
    }
    return 0;
}

/*
 * return value:
 *     1: abnormal status
 *     0: normal status
 */
int get_battnotify_status()
{
	int battStatus = get_int_value(CHARGER_BATTNOTFY_PATH);
	SXLOGI("charger battStatus: %d\n",battStatus);
	if (battStatus != 0) {
		return 1;
	}
	return 0;
}


void updateFastRebootMode(){
    char buf[PROPERTY_VALUE_MAX];

    if(property_get(IPO_FAST_REBOOT, buf, "0")){
        if(isdigit(buf[0]) && (buf[0]!='0')){
            SXLOGI("Fast reboot is on\n");
            params[PARAM_FAST_REBOOT] = atoi(buf);
        }else
            SXLOGI("Fast reboot is off\n");
    }
}

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#if defined(MTK_IPO_POWERPATH_SUPPORT) || defined(MTK_BATLOWV_NO_PANEL_ON_EARLY)
void updateLogoBacklightVoltageMode(long params[])
{
	int val_bootmode = get_int_value(BOOTMODE_PATH);
	int val_ac = get_int_value(AC_ONLINE_PATH);

    SXLOGI("val_bootmode = %d, val_ac = %d\n", val_bootmode, val_ac);

	if ((val_bootmode == 9) && (val_ac == 0)) {
		params[PARAM_NOLOGO] = 7;
		params[PARAM_BK_LEVEL] = 20;
		params[PARAM_POWER_ON_VOLTAGE] = USB_POWER_ON_VOLTAGE;
		SXLOGI("IPOD disable logo/backlight for tablet w/low batt and USB charger!");
	}
#ifdef MTK_IPO_POWERPATH_SUPPORT	
	else
	{
	    params[PARAM_POWER_ON_VOLTAGE] = 3000; //keep powerpath working
	    //params[PARAM_POWER_OFF_VOLTAGE] = 3000;
	}
#endif	
}
#endif
#endif

int is_wireless_charging(){
    int wireless_charging = get_int_value(WIRELESS_CHARGING);
    SXLOGI("wireless_charging: %d\n", wireless_charging);
    return wireless_charging;
}
