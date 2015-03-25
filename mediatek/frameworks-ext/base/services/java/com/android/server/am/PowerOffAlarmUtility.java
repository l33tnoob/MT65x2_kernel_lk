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

package com.android.server.am;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.provider.Settings;
import android.util.Log;
import android.os.SystemProperties;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import com.android.internal.policy.PolicyManager;
import com.android.server.wm.WindowManagerService;

/**
 * add for power-off alarm
 *
 * @author mtk54296
 */
public class PowerOffAlarmUtility {
    private final static String         TAG           = "PowerOffAlarm";
    private Context                     mContext;
    private final static String         REMOVE_IPOWIN = "alarm.boot.remove.ipowin";
    private final static String         ALARM_BOOT_DONE = "android.intent.action.normal.boot.done";
    private boolean                     mRollback     = false;
    public boolean                      mFirstBoot    = false;
    private ActivityManagerPlus         mAmPlus;
    private ActivityStack               mStack;
    private static PowerOffAlarmUtility mInstance;

    /**
     * return the singleton instance of this class called by systemReady() in
     * AMS to create a instance.
     *
     * @param ctx
     * @param aStack
     * @param amPlus
     * @return
     */
    public static PowerOffAlarmUtility getInstance(Context ctx,
            ActivityStack aStack, ActivityManagerPlus amPlus) {
        if (mInstance != null) {
            return mInstance;
        }
        if (ctx != null && aStack != null && amPlus != null) {
            mInstance = new PowerOffAlarmUtility(ctx, aStack, amPlus);
        }
        return mInstance;
    }

    /**
     * constructor
     *
     * @param ctx
     * @param aStack
     * @param amPlus
     */
    private PowerOffAlarmUtility(Context ctx, ActivityStack aStack,
            ActivityManagerPlus amPlus) {
        mContext = ctx;
        mAmPlus = amPlus;
        mStack = aStack;
        registerNormalBootReceiver(mContext);
        boolean recover = SystemProperties.getBoolean(
                "persist.sys.ams.recover", false);
        if (recover) {
            checkFlightMode(true, false);
        }
    }

    /**
     * launch power off alarm
     *
     * @param isAlarmBoot
     * @param recover
     * @param shutdown
     */

    public void launchPowrOffAlarm(Boolean recover, Boolean shutdown) {
        if (recover != null && shutdown != null) {
            checkFlightMode(recover, shutdown);
        }
        mContext.sendBroadcast(new Intent(
                "android.intent.action.LAUNCH_POWEROFF_ALARM"));
    }

    public void launchPowrOffAlarmIPO(Boolean mFlightModeOn) {
        if (mFlightModeOn) {
            mRollback = true;
        }
        mContext.sendBroadcast(new Intent(
                "android.intent.action.LAUNCH_POWEROFF_ALARM"));
    }
    /**
     * to check if it is alarm boot
     *
     * @return
     */
    public static boolean isAlarmBoot() {
        String bootReason = SystemProperties.get("sys.boot.reason");
        boolean ret = (bootReason != null && bootReason.equals("1")) ? true
                : false;
        return ret;
    }

    /**
     * Power off Alarm feature: When receiving the
     * android.intent.action.normal.boot intent, AMS will resume the boot
     * process (broadcast BOOT_COMPLETED intent)
     */
    private final void registerNormalBootReceiver(Context context) {
        IntentFilter filter = new IntentFilter();
        filter.addAction("android.intent.action.normal.boot");
        filter.addAction("android.intent.action.normal.shutdown");
        filter.addAction(ALARM_BOOT_DONE);
        filter.addAction(REMOVE_IPOWIN);
        mFirstBoot = true;

        context.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (intent == null) {
                    return;
                }
                String action = intent.getAction();
                if ("android.intent.action.normal.boot".equals(action)) {
                    Log.i(TAG, "DeskClock normally boots-up device");
                    if (mRollback) {
                        checkFlightMode(false, false);
                    }
                    if (mFirstBoot) {
                        // set mBooting
                        synchronized (mStack.mService) {
                            mAmPlus.setBootingVal(true);
                            mStack.resumeTopActivityLocked(null);
                        }
                    } else {
                        synchronized (mStack.mService) {
                            mAmPlus.IPOBootCompletedLocked();
                        }
                    }
                } else if ("android.intent.action.normal.shutdown"
                        .equals(action)) {
                    Log.v(TAG, "DeskClock normally shutdowns device");
                    mStack.mService.createIPOWin(mContext);
                    if (mRollback) {
                        checkFlightMode(false, true);
                    }
                } else if (ALARM_BOOT_DONE.equals(action)) {
                    Log.w(TAG, "ALARM_BOOT_DONE normally shutdowns device");
                    // do we need to remove the synchronized ?
                    synchronized (mStack.mService) {
                        mStack.resumeTopActivityLocked(null);
                    }
                } else if (REMOVE_IPOWIN.equals(action)) {
                    mStack.mService.removeIPOWin(mContext);
                }
            }
        }, filter);
    }

    /**
     * Power Off Alarm feature: Update the flight mode status when Power Off
     * Alarm is triggered
     */
    public void checkFlightMode(boolean recover, boolean shutdown) {
        Log.v(TAG, "mRollback = " + mRollback + ", recover = " + recover);
        
        if (recover) {
            Log.v(TAG, "since system crash, switch flight mode to off");
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0);
            SystemProperties.set("persist.sys.ams.recover", "false");
            return;
        }

        if (mRollback) {
            mRollback = false;
            SystemProperties.set("persist.sys.ams.recover", "false");
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0);

            if (!shutdown) {
                Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
                intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
                intent.putExtra("state", false);
                mContext.sendBroadcast(intent);
            }
        } else {
            boolean mode = Settings.Global.getInt(
                    mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0) != 0;

            if (!mode) {
                Log.v(TAG, "turn on flight mode");
                SystemProperties.set("persist.sys.ams.recover", "true");
                mRollback = true;
                Settings.Global.putInt(mContext.getContentResolver(),
                        Settings.Global.AIRPLANE_MODE_ON, 1);
            }
        }
    }
}
