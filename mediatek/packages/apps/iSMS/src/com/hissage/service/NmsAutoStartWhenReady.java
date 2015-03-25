package com.hissage.service;

import android.content.Context;
import android.content.Intent;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.hissage.timer.NmsWakeLock;

public class NmsAutoStartWhenReady {
    private static final String TAG = "NmsAutoStartWhenReady";
    private static NmsAutoStartWhenReady mAutoStart = null;
    private static Context mContext = null;
    private static boolean mListenFlag = false;
    private static Intent mIntent = null;

    private NmsAutoStartWhenReady() {
        return;
    }

    public static NmsAutoStartWhenReady getInstance(Context context, Intent i) {
        if (null == mAutoStart) {
            mAutoStart = new NmsAutoStartWhenReady();
            mContext = context;
            mIntent = i;
        }

        return mAutoStart;
    }

    public void startEngineWhenSimCardReady() {
        if (mListenFlag) {
            Log.e(TAG, "some one create NmsAutoStartWhenReady instance already, so ingnore this req.");
            return;
        }
        mListenFlag = true;

        new Thread(new Runnable() {
            @Override
            public void run() {
                NmsWakeLock.NmsSetWakeupLock(mContext, "NmsAutoStartWhenReady");
                int i = 0;
                // break it 1min later
                while (i < 30) {
                    TelephonyManager telManager = (TelephonyManager) mContext
                            .getSystemService(Context.TELEPHONY_SERVICE);

                    if (null != telManager) {
                        int stat = telManager.getSimState();
                        if (TelephonyManager.SIM_STATE_READY == stat) {
                            break;
                        } else {
                            Log.e(TAG, "sim card not ready, retry..." + i);
                        }
                    }

                    try {
                        Thread.sleep(2000);
                        i++;
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        i++;
                    }

                }
                if (!NmsService.bCEngineRunflag) {
                    mContext.startService(mIntent);
                }

                NmsWakeLock.NmsReleaseWakeupLock("NmsAutoStartWhenReady");
            }
        }).start();
    }

}
