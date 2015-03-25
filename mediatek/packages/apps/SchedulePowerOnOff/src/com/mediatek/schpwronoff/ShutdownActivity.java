/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.PowerManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;

import com.mediatek.schpwronoff.R;
import com.mediatek.xlog.Xlog;

public class ShutdownActivity extends Activity {
    private static final String TAG = "ShutdownActivity";
    public static CountDownTimer sCountDownTimer = null;
    private String mMessage;
    private int mSecondsCountdown;
    private TelephonyManager mTelephonyManager;
    private static final int DIALOG = 1;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SchPwrWakeLock.acquireCpuWakeLock(this);
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        Log.d(TAG, "screen is on ? ----- " + pm.isScreenOn());

        mTelephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        final Window win = getWindow();
        win.addFlags(WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD | WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
        
        final int countSeconds = 11;
        final int millisSeconds = 1000;
        if (savedInstanceState == null) {
            mSecondsCountdown = countSeconds;
        } else {
            mSecondsCountdown = savedInstanceState.getInt("lefttime");
            mMessage = savedInstanceState.getString("message");
        }
        sCountDownTimer = new CountDownTimer(mSecondsCountdown * millisSeconds, millisSeconds) {
            @Override
            public void onTick(long millisUntilFinished) {
                mSecondsCountdown = (int) (millisUntilFinished / millisSeconds);
                if (mSecondsCountdown > 1) {
                    mMessage = getString(R.string.schpwr_shutdown_message, mSecondsCountdown);
                } else {
                    mMessage = getString(R.string.schpwr_shutdown_message_second, mSecondsCountdown);
                }
                Xlog.d(TAG, "showDialog time = " + millisUntilFinished/millisSeconds);
                showDialog(DIALOG);
            }

            @Override
            public void onFinish() {
                if (mTelephonyManager.getCallState() != TelephonyManager.CALL_STATE_IDLE) {
                    Xlog.d(TAG, "phone is incall, countdown end");
                    SchPwrWakeLock.releaseCpuWakeLock();
                    finish();
                } else {
                    Xlog.d(TAG, "count down timer arrived, shutdown phone");
                    fireShutDown();
                    sCountDownTimer = null;
                }
            }
        };

        Xlog.d(TAG, "ShutdonwActivity onCreate");
        if (sCountDownTimer == null) {
            SchPwrWakeLock.releaseCpuWakeLock();
            finish();
        } else {
            sCountDownTimer.start();
        }
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt("lefttime", mSecondsCountdown);
        outState.putString("message", mMessage);
    }

    private void cancelCountDownTimer() {
        if (sCountDownTimer != null) {
            Xlog.d(TAG, "cancel sCountDownTimer");
            sCountDownTimer.cancel();
            sCountDownTimer = null;
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Xlog.d(TAG, "onCreateDialog");
        AlertDialog dialog = new AlertDialog.Builder(this)
                .setCancelable(false).setIcon(android.R.drawable.ic_dialog_alert)
                .setTitle(com.android.internal.R.string.power_off).setMessage(mMessage)
                .setPositiveButton(com.android.internal.R.string.yes, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        cancelCountDownTimer();
                        fireShutDown();
                    }
                }).setNegativeButton(com.android.internal.R.string.no, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        cancelCountDownTimer();
                        SchPwrWakeLock.releaseCpuWakeLock();
                        finish();
                    }
                }).create();
        if (!getResources().getBoolean(com.android.internal.R.bool.config_sf_slowBlur)) {
            dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
        }
        Window win = dialog.getWindow();
        WindowManager.LayoutParams winParams = win.getAttributes();
        winParams.flags |= WindowManager.LayoutParams.FLAG_HOMEKEY_DISPATCHED;
        win.setAttributes(winParams);
        return dialog;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
        ((AlertDialog) dialog).setMessage(mMessage);
    }

    private void fireShutDown() {
        Intent intent = new Intent(Intent.ACTION_REQUEST_SHUTDOWN);
        intent.putExtra(Intent.EXTRA_KEY_CONFIRM, false);
        intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }
}
