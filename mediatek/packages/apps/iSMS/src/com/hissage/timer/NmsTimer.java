package com.hissage.timer;

//import packages.
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.os.SystemClock;

import com.hissage.hpe.SDK;
import com.hissage.jni.engineadapter;
import com.hissage.pn.hpnsReceiver;
import com.hissage.service.NmsService;
import com.hissage.upgrade.NmsUpgradeManager;
import com.hissage.util.data.NmsConverter;
import com.hissage.util.log.NmsLog;
//M: Activation Statistics
import com.hissage.util.statistics.NmsStatistics;
import com.hissage.vcard.NmsVcardUtils;

public final class NmsTimer {
    private static final String TAG = "NmsTimer";

    public static class NmsTimerObj {
        public Intent intent;
        public PendingIntent sender;
        public boolean flag = false;
    }

    public static final int NMS_TIMERID_REGISTRATION = 0;
    public static final int NMS_TIMERID_CONNECTION = 1;
    public static final int NMS_TIMERID_TRANSACTION = 2;
    public static final int NMS_TIMERID_HEART_BEAT = 3;
    public static final int NMS_TIMERID_CFG_SET = 4;
    public static final int NMS_TIMERID_FETCH = 5;
    public static final int NMS_TIMERID_VCARD = 6;
    public static final int NMS_TIMERID_REG_SMS = 7;
    public static final int NMS_TIMERID_TCP_1 = 8;
    public static final int NMS_TIMERID_TCP_2 = 9;
    public static final int NMS_TIMERID_TCP_3 = 10;
    public static final int NMS_TIMERID_CONTACT_RESET_1 = 11;
    public static final int NMS_TIMERID_CONTACT_RESET_2 = 12;
    public static final int NMS_TIMERID_CONTACT_RESET_3 = 13;
    public static final int NMS_TIMERID_CONTACT_RESET_4 = 14;
    public static final int NMS_TIMERID_CONTACT_RESET_5 = 15;
    public static final int NMS_TIMERID_CONTACT_RESET_6 = 16;
    public static final int NMS_TIMERID_CONTACT_RESET_7 = 17;
    public static final int NMS_TIMERID_CONTACT_RESET_8 = 18;
    public static final int NMS_TIMERID_CONTACT_RESET_9 = 19;
    public static final int NMS_TIMERID_CONTACT_RESET_10 = 20;
    public static final int NMS_TIMERID_MAX = 21;

    public static final int NMS_TIMERID_CONTACT = 22;
    public static final int NMS_TIMERID_UPGRADE = 23;
    public static final int NMS_TIMERID_PN_REGISTER = 24;
	//M: Activation Statistics
    public static final int NMS_TIMERID_STATISTICS = 25;
    public static final int NMS_PRIVATE_UPGRADE = 26;
    public static final int NMS_TIMERID_ALL_MAX = 27;
	//M: Activation Statistics
    public static final String nmsTimerName[] = { "REGISTRATION", "CONNECTION", "TRANSACTION",
            "HEART_BEAT", "CFG_SET", "FETCH", "VCARD", "NMS_TIMERID_REG_SMS", "TCP_0", "TCP_1",
            "TCP_2", "RESET_1", "RESET_2", "RESET_3", "RESET_4", "RESET_5", "RESET_6", "RESET_7",
            "RESET_8", "RESET_9", "RESET_10", "NMS_TIMERID_ENGINE_MAX", "NMS_TIMERID_CONTACT",
            "NMS_TIMERID_UPGRADE", "NMS_TIMERID_PN_REGISTER", "NMS_TIMERID_STATISTICS", "NMS_TIMERID_ALL_MAX" };

    public static class AlarmRecevier extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String category = intent.getAction();
            int timerId = NmsConverter.string2Int(category);

            if (timerId >= NMS_TIMERID_REGISTRATION && timerId < NMS_TIMERID_MAX) {
                // NmsUtils.trace(HissageTag.stm, "timer " +
                // nmsTimerName[timerId] + " is expired");

                NmsWakeLock.NmsSetWakeupLock(NmsService.getInstance(), nmsTimerName[timerId]);
                engineadapter.get().nmsSendTimerMsgToEngine(timerId);
                nmsTimerObj[timerId].flag = false;
                NmsWakeLock.NmsReleaseWakeupLock(nmsTimerName[timerId]);
            } else {
                // NmsUtils.trace(HissageTag.stm, "java platform timer ID: " +
                // timerId);
                if (timerId == NMS_TIMERID_CONTACT) {
                    NmsVcardUtils.notifyEngineContactChanged();
                } else if (timerId == NMS_TIMERID_UPGRADE) {
                    NmsUpgradeManager.handleTimerEvent();
                } else if (timerId == NMS_TIMERID_PN_REGISTER) {
                    nmsTimerObj[timerId].flag = false;
                    SDK.onRegister(context);
				//M: Activation Statistics
                } else if (timerId == NMS_TIMERID_STATISTICS) {
                    NmsStatistics.handleTimerEvent() ;
                }else if (timerId == NMS_PRIVATE_UPGRADE){
                    NmsUpgradeManager.handlePrivateUpgradeEvent();
                }
            }
        }
    }

    public static final int NMS_TIMER_RESULT_ERROR = -2;
    public static final int NMS_TIMER_RESULT_EXIST = -1;
    public static final int NMS_TIMER_RESULT_OK = 0;

    public static AlarmRecevier mReceiver = new AlarmRecevier();
    public static AlarmManager am;
    public static NmsTimerObj[] nmsTimerObj = null;

    public static NmsTimerObj getNmsTimerObj(int id) {
        if (nmsTimerObj == null || nmsTimerObj.length == 0) {
            nmsTimerObj = new NmsTimerObj[NMS_TIMERID_ALL_MAX];
            for (int i = 0; i < NMS_TIMERID_ALL_MAX; ++i) {
                nmsTimerObj[i] = new NmsTimerObj();
                nmsTimerObj[i].flag = false;
            }
        }
        return nmsTimerObj[id];
    }

    public static int NmsCreateTimer(int timerId, int seconds) {
        if ((timerId >= NMS_TIMERID_ALL_MAX || timerId < 0)) {
            NmsLog.trace(TAG, "Exception: create timer id outof TIMERID_MAX, id is " + timerId);
            return NMS_TIMER_RESULT_ERROR;
        }
        if (true == getNmsTimerObj(timerId).flag)
            return NMS_TIMER_RESULT_EXIST;
        getNmsTimerObj(timerId).flag = true;
        String timerName = NmsConverter.int2String(timerId);

        NmsService.getInstance().registerReceiver(mReceiver, new IntentFilter(timerName));

        getNmsTimerObj(timerId).intent = new Intent(timerName);
        getNmsTimerObj(timerId).sender = PendingIntent.getBroadcast(NmsService.getInstance(), 0,
                getNmsTimerObj(timerId).intent, 0);

        long firstTime = SystemClock.elapsedRealtime();
        firstTime += seconds * 1000;

        am = (AlarmManager) NmsService.getInstance().getSystemService(Context.ALARM_SERVICE);
        am.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, firstTime, getNmsTimerObj(timerId).sender);

        return NMS_TIMER_RESULT_OK;
    }

    public static int NmsRemoveTimer(int timerId) {
        if (timerId >= NMS_TIMERID_ALL_MAX || timerId < 0) {
            NmsLog.trace(TAG, "Exception: kill timer id outof TIMERID_MAX, id is " + timerId);
            return NMS_TIMER_RESULT_ERROR;
        }

        if (true != getNmsTimerObj(timerId).flag)
            return NMS_TIMER_RESULT_EXIST;
        // NmsUtils.trace(HissageTag.stm, "timer " + nmsTimerName[timerId] +
        // " is killed");
        am.cancel(getNmsTimerObj(timerId).sender);
        getNmsTimerObj(timerId).flag = false;

        return NMS_TIMER_RESULT_OK;
    }

    public static int NmsSetTimer(int id, long seconds) {
        try {
            // NmsUtils.trace(HissageTag.stm, "timer " + nmsTimerName[id] +
            // " is set to: " + seconds + " seconds");
            if (seconds > 0) {
                return NmsCreateTimer(id, (int) seconds);
            } else {
                // This timer can't support 0 seconds, if set 0 then throw the
                // exception
                return NmsCreateTimer(id, 0);
            }
        } catch (Exception e) {
            NmsLog.error(TAG, "create timer id:" + id + "\texception:" + NmsLog.nmsGetStactTrace(e));
            return NMS_TIMER_RESULT_ERROR;
        }
    }

    public static int NmsKillTimer(int id) {
        if (id < NMS_TIMERID_ALL_MAX && id >= 0) {
            return NmsRemoveTimer(id);
        } else {
            NmsLog.trace(TAG, "Timer ID error, ID is :" + id);
            return NMS_TIMER_RESULT_ERROR;
        }
    }

    // retrieve current system time in seconds.
    public static int NmsGetSystemTime() {
        long millSeconds = System.currentTimeMillis();
        return (int) (millSeconds / 1000);
    }
}
