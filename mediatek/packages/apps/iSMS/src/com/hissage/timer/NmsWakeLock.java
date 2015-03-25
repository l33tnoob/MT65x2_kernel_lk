package com.hissage.timer;

import com.hissage.util.log.NmsLog;

import android.content.Context;
import android.os.PowerManager;

public class NmsWakeLock {

    private PowerManager mPM;
    private static PowerManager.WakeLock mPmWl;
    private static NmsWakeLock mWakelock = null;

    // constructor.
    public NmsWakeLock(Context context, String tag) {
        mPM = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mPmWl = mPM.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, tag);
    }

    public static void NmsSetWakeupLock(Context context, String tag) {
        // NmsUtils.trace(HesineTag.stm,
        // tag+"--set wakeup lock, phone wakeup.");
        if (null == mWakelock) {
            mWakelock = new NmsWakeLock(context, tag);
        }
        mPmWl.acquire();
    }

    public static void NmsReleaseWakeupLock(String tag) {
        // NmsUtils.trace(HesineTag.stm, tag+"--release wakeup lock.");
        mPmWl.release();
    }

    public static void NmsSetWakeupLock(int time, Context context, String tag) {
        NmsWakeLockObj wakelock = new NmsWakeLockObj(time, context, tag);
        wakelock.start();
    }
    
    public static class NmsWakeLockObj extends Thread {
        private static final String TAG = "NmsWakeLockObj";
        private PowerManager mPM;
        private PowerManager.WakeLock mWakelock;
        private int mWakeupTime;

        // constructor.
        public NmsWakeLockObj(int time, Context context, String tag) {
            super();
            mPM = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            mWakelock = mPM.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, tag);
            mWakeupTime = time;
        }

        public void run() {
            mWakelock.acquire();
            // NmsUtils.trace(HesineTag.stm,
            // mTag+"--set wakeup lock, phone wakeup "+wakeupTime+"s.");
            try {
                while (mWakeupTime > 0) {
                    Thread.sleep(mWakeupTime * 1000);
                    mWakeupTime = 0;
                }
            } catch (InterruptedException e) {
                NmsLog.error(TAG, "wakeup lock exception: " + NmsLog.nmsGetStactTrace(e));
            }
            // NmsUtils.trace(HesineTag.stm, mTag+"--release wakeup lock.");
            mWakelock.release();
        }
    }
    
}