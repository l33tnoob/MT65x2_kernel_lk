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

package com.mediatek.engineermode.nonsleep;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.PowerManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.mediatek.engineermode.EngineerMode;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.List;
/**
 * Non sleep mode : keep display on when activity is up
 * @author mtk54040
 *
 */
public class NonSleepMode extends Activity implements OnClickListener, ServiceConnection {

    private static final String TAG = "EM/NonSleep";
    private Button mSetButton;
    private EMWakeLockService mWakeLockServ = null;
    
    private static boolean isServiceRunning(Context context, Class<? extends Service> clazz) {
        boolean isRunning = false;
        ActivityManager am = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);
        int maxCount = 100;
        List<ActivityManager.RunningServiceInfo> runningServices = am.getRunningServices(maxCount);
        while (runningServices.size() == maxCount) {
            maxCount += 50;
            runningServices = am.getRunningServices(maxCount);
        }
        
        for (int i = 0; i < runningServices.size(); i++) {
            ActivityManager.RunningServiceInfo info = runningServices.get(i);
            if (info.service.getClass().equals(clazz)) {
                isRunning = true;
                break;
            }
        }
        return isRunning;
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.non_sleep_mode);
        mSetButton = (Button) findViewById(R.id.non_sleep_switch);

        mSetButton.setOnClickListener(this);
        if (!isServiceRunning(this, EMWakeLockService.class)) {
            startService(new Intent(this, EMWakeLockService.class));
        }
    }
    
    @Override
    protected void onStart() {
        super.onStart();
        mSetButton.setEnabled(false);
        Intent intent = new Intent(this, EMWakeLockService.class);
        bindService(intent, this, Context.BIND_AUTO_CREATE);
    }
    
    @Override
    protected void onStop() {
        unbindService(this);
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        if (isServiceRunning(this, EMWakeLockService.class) && 
                mWakeLockServ != null && !mWakeLockServ.isHeld()) {
            stopService(new Intent(this, EMWakeLockService.class));
        }
        super.onDestroy();
    }

    /**
     * set flag value
     * */
    public void onClick(final View arg0) {

        if (arg0 == mSetButton) {
            if ((getString(R.string.non_sleep_enable)).equals(
                    mSetButton.getText())) {
                mSetButton.setText(R.string.non_sleep_disable);
                if (!mWakeLockServ.isHeld()) {
                    mWakeLockServ.acquire(NonSleepMode.class);
                }
            } else {
                mSetButton.setText(R.string.non_sleep_enable);
                if (mWakeLockServ.isHeld()) {
                    mWakeLockServ.release();
                }
            }
        }
    }

    @Override
    public void onServiceConnected(ComponentName className, IBinder service) {
        EMWakeLockService.LocalBinder binder = (EMWakeLockService.LocalBinder)service;
        mWakeLockServ = binder.getService();
        mSetButton.setEnabled(true);
        if (mWakeLockServ.isHeld()) {
            mSetButton.setText(R.string.non_sleep_disable);
        } else {
            mSetButton.setText(R.string.non_sleep_enable);
        }
    }

    @Override
    public void onServiceDisconnected(ComponentName className) {
        Xlog.d(TAG, "onServiceDisconnected");
        
    }
}
