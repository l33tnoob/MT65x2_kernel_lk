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

package com.mediatek.schpwronoff;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Parcel;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;

import com.mediatek.xlog.Xlog;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Glue class: connects AlarmAlert IntentReceiver to AlarmAlert activity. Passes
 * through Alarm ID.
 */
public class SchPwrOffReceiver extends BroadcastReceiver {
    private static final String TAG = "SchPwrOffReceiver";
    /**
     * If the alarm is older than STALE_WINDOW seconds, ignore. It is probably
     * the result of a time or timezone change
     */
    private static final int STALE_WINDOW = 60 * 30;
    private static final String SHUTDOWN_IPO = "android.intent.action.ACTION_SHUTDOWN_IPO";
    private static final String ACTION_SHUTDOWN = "android.intent.action.ACTION_SHUTDOWN";
    Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        Xlog.d(TAG, "intent action " + String.valueOf(intent.getAction()));
        if (SHUTDOWN_IPO.equals(intent.getAction())) {
            if (ShutdownActivity.sCountDownTimer != null) {
                Xlog.d(TAG, "ShutdownActivity.sCountDownTimer != null");
                ShutdownActivity.sCountDownTimer.cancel();
                ShutdownActivity.sCountDownTimer = null;
            }
            return;
        // ALPS00881041 hold a cpu wake lock, if Shutdown thread received the shutdown request, release the lock
        } else if (ACTION_SHUTDOWN.equals(intent.getAction())) {
            SchPwrWakeLock.releaseCpuWakeLock();
            return;
        }

        mContext = context;
        Alarm alarm = null;
        // Grab the alarm from the intent. Since the remote AlarmManagerService
        // fills in the Intent to add some extra data, it must unparcel the
        // Alarm object. It throws a ClassNotFoundException when unparcelling.
        // To avoid this, do the marshalling ourselves.
        final byte[] data = intent.getByteArrayExtra(Alarms.ALARM_RAW_DATA);
        if (data != null) {
            Parcel in = Parcel.obtain();
            in.unmarshall(data, 0, data.length);
            in.setDataPosition(0);
            alarm = Alarm.CREATOR.createFromParcel(in);
        }

        if (alarm == null) {
            Xlog.d(TAG, "SchPwrOffReceiver failed to parse the alarm from the intent");
            return;
        }

        final int millisInSeconds = 1000;
        // Intentionally verbose: always log the alarm time to provide useful
        // information in bug reports.
        long now = System.currentTimeMillis();
        SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss.SSS aaa", Locale.US);
        Xlog.d(TAG, "SchPwrOffReceiver.onReceive() id " + alarm.mId + " setFor " + format.format(new Date(alarm.mTime)));

        if (now > alarm.mTime + STALE_WINDOW * millisInSeconds) {
            Xlog.d(TAG, "SchPwrOffReceiver ignoring stale alarm");
            Xlog.d(TAG, "now = " + now);
            Xlog.d(TAG, "stale time = " + (alarm.mTime + STALE_WINDOW * millisInSeconds));
            return;
        }

        // Maintain a cpu wake lock until the AlarmAlert and AlarmKlaxon can
        // pick it up.
        // AlarmAlertWakeLock.acquireCpuWakeLock(context);

        /* Close dialogs and window shade */
        Intent closeDialogs = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        context.sendBroadcast(closeDialogs);
        final int schduleTimeOff = 900;
        // Decide which activity to start based on the state of the keyguard.

        if (alarm.mId == 1) {
            Xlog.d(TAG, "SchPwrOffReceiver.onReceive() id " + alarm.mId + " get power on time out ");
        } else if (alarm.mId == 2) {
            boolean isInCall = false;
            TelephonyManager telephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            isInCall = telephonyManager.getCallState() != TelephonyManager.CALL_STATE_IDLE;
            Xlog.d(TAG, "SchPwrOffReceiver.onReceive() id " + alarm.mId + " in call " + isInCall);

            if (isInCall || isAlarmBoot()) {
                Xlog.d(TAG, "SchPwrOffReceiver.onReceive() id " + alarm.mId + " isAlarmboot= " + isAlarmBoot());
            } else {
                Intent i = new Intent(context, ShutdownActivity.class);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                PendingIntent pendingIntent = PendingIntent.getActivity(context, getResultCode(), i,
                        PendingIntent.FLAG_ONE_SHOT);
                AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
                am.setExact(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + schduleTimeOff, pendingIntent);
            }

            // Disable this alarm if it does not repeat.
            if (alarm.mDaysOfWeek.isRepeatSet()) {
                // Enable the next alert if there is one. The above call to
                // enableAlarm will call setNextAlert so avoid calling it twice.
                Xlog.d(TAG, "SchPwrOffReceiver.onReceive(): not isRepeatSet()");
                Alarms.setNextAlertPowerOff(context);
            } else {
                Xlog.d(TAG, "SchPwrOffReceiver.onReceive(): isRepeatSet() ");
                Alarms.enableAlarm(context, alarm.mId, false);
            }
        }
    }

    /**
     * check if is alarm boot; if it alarm boot, we don't fire the
     * shutdownactivity pop dialog.
     * 
     * @return boolean true or false
     */
    private static boolean isAlarmBoot() {
        String bootReason = SystemProperties.get("sys.boot.reason");
        boolean ret = (bootReason != null && bootReason.equals("1")) ? true : false;
        return ret;
    }
}
