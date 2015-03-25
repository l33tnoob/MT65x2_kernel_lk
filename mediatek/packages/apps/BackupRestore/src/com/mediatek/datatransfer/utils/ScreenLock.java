package com.mediatek.datatransfer.utils;

import android.content.Context;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

public class ScreenLock {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/ScreenLock";
    private WakeLock mWakeLock;

    private ScreenLock() {

    }

    private static ScreenLock mIncetance = null;

    public static ScreenLock instance() {
        if (mIncetance == null) {
            mIncetance = new ScreenLock();
        }
        return mIncetance;
    }

    /**
     * keep screen always on
     * 
     * @param context
     */
    public void acquireWakeLock(Context context) {

        if (mWakeLock == null) {
            MyLogger.logD(CLASS_TAG, "Acquiring wake lock");
            PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, this.getClass()
                    .getCanonicalName());
        }
        mWakeLock.acquire();
    }

    /**
     * release screen always on
     */
    public void releaseWakeLock() {
        if (mWakeLock != null && mWakeLock.isHeld()) {
            mWakeLock.release();
            mWakeLock = null;
        }

    }

}
