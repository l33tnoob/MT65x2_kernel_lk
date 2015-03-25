package com.mediatek.ppl;

import java.util.Arrays;

import android.content.Context;
import android.util.Log;

import com.mediatek.telephony.TelephonyManagerEx;

/**
 * Utility class to track SIM states.
 */
public class SimTracker {
    private static final String TAG = "PPL/SimTracker";

    public final int slotNumber;
    private TelephonyManagerEx mTelephonyManager;
    private Context mContext;
    boolean inserted[];
    String serialNumbers[];
    int states[];
    private boolean mEnabled;

    public SimTracker(int number, Context context) {
        slotNumber = number;
        mContext = context;
        inserted = new boolean[slotNumber];
        serialNumbers = new String[slotNumber];
        states = new int[slotNumber];
        mTelephonyManager = new TelephonyManagerEx(context);
        if (mTelephonyManager == null) {
            Log.e(TAG, "mTelephonyManager is null");
            mEnabled = false;
        } else if (!isTelephonyManagerReady()) {
            Log.e(TAG, "mTelephonyManager is not ready");
        	mEnabled = false;
        } else {
        	mEnabled = true;
        }
    }

    /**
     * Take a snapshot of the current SIM information in system and store it in this object.
     */
    public synchronized void takeSnapshot() {
    	if (!mEnabled) {
    		return;
    	}

        for (int i = 0; i < slotNumber; ++i) {
            if (mTelephonyManager.hasIccCard(i)) {
                inserted[i] = true;
                serialNumbers[i] = mTelephonyManager.getSimSerialNumber(i);
                if ("".equals(serialNumbers[i])) {
                    serialNumbers[i] = null;
                }
                states[i] = mTelephonyManager.getSimState(i);
            } else {
                inserted[i] = false;
            }
        }
    }

    /**
     * Get a list of the IDs of current inserted SIM cards.
     * 
     * @return
     */
    public synchronized int[] getInsertedSim() {
        int[] result = new int[inserted.length];
        int count = 0;
        for (int i = 0; i < inserted.length; ++i) {
            if (inserted[i]) {
                result[count++] = i;
            }
        }
        return Arrays.copyOf(result, count);
    }

    /**
     * TelephonyManager may be unavailable in certain circumstance such as data encrypting process of the phone.
     * @return  Whether TelephonyManager is available.
     */
    @SuppressWarnings("unused")
    public boolean isTelephonyManagerReady() {
        try {
            boolean temp = mTelephonyManager.hasIccCard(0);
            return true;
        } catch (Throwable t) {
            return false;
        }
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("SimTracker: ");
        for (int i = 0; i < slotNumber; ++i) {
            sb.append("{")
              .append(inserted[i])
              .append(", ")
              .append(serialNumbers[i])
              .append(", ")
              .append(states[i])
              .append("}, ");
        }
        return sb.toString();
    }
}
