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

package com.mediatek.common.thermal;

import java.io.File;
import java.io.OutputStreamWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.FileInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.util.*;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.util.Log;
import static junit.framework.Assert.*;

import com.mediatek.common.thermal.MtkThermalSwitchConfig;

public class MtkThermalSwitchManager {
    private static final String TAG = "ThermalSwitchManager";
    private HandlerThread mHandlerThread;
    private ThermalThreadHandler mHandler;
    private Context mContext;
    private int mState;

    public enum AppState {
        Paused,
        Resumed,
        Destroyed,
        Dead
    }

    class ThermalSwitchState {
        public static final int Init = 0;
        public static final int Ready = 1;		// May be TP or DTM
        public static final int Disabling = 2;	// Switching to TP
        public static final int Disabled= 3;	// Switched to TP 
        public static final int Enabling = 4;	// Switching to DTM
    }
	
    public MtkThermalSwitchManager(Context context) {
        super();
        mContext = context;
        mState = ThermalSwitchState.Init;
        mHandlerThread = new HandlerThread("ThermalSwitchManager", Process.THREAD_PRIORITY_FOREGROUND);
        mHandlerThread.start();
        mHandler = new ThermalThreadHandler(mHandlerThread.getLooper());
        Log.i(TAG, "Created and started thermal switch thread");
    }

    public void systemReady() {
        Log.i(TAG, "systemReady, register ACTION_BOOT_COMPLETED");
        mContext.registerReceiver(mBroadcastReceiver, new IntentFilter(Intent.ACTION_BOOT_COMPLETED), null, mHandler);
    }
 
    public void notifyAppState(String appPackage, AppState appState) {
        //Log.i(TAG, "notify app state, app: " + appPackage + ", state: " + appState + ", thermal state: " + mState);

        // Check whether appPackage is a benchmark app and whether appState is "Resume"
        if (MtkThermalSwitchConfig.appConfig.containsKey(appPackage) &&
            appState == AppState.Resumed &&
            mState != ThermalSwitchState.Disabling &&
            mState != ThermalSwitchState.Enabling) {
            Message msg = mHandler.obtainMessage();
            msg.what = ThermalThreadHandler.MESSAGE_APP_RESUMED;
            msg.arg1 = MtkThermalSwitchConfig.appConfig.get(appPackage);
            msg.sendToTarget();
        }
    }

    private void execShellCommand(String shellCommand) {
        java.lang.Process proc = null;
        OutputStreamWriter osw = null;

        try {
            proc = Runtime.getRuntime().exec(shellCommand);
            osw = new OutputStreamWriter(proc.getOutputStream());
            if (null != osw) {
                osw.write(shellCommand);
                osw.write("\n");
                osw.write("exit\n");
                osw.flush();
                osw.close();
            }
        }
        catch (IOException ex) {
            Log.e(TAG, "execCommandLine() IO Exception");
            return;
        }
        finally	{
            if (null != osw) {
                try	{
                    osw.close();
                }
                catch (IOException e) {}
            }
        }

        try {
            proc.waitFor();
        }
        catch (InterruptedException e) {}

        if (proc.exitValue() != 0) {
            Log.e(TAG, "execCommandLine() Err exit code: " + proc.exitValue());
        }
    }

    private void changeToThermalProtection()
    {
        execShellCommand("/system/bin/thermal_manager /etc/.tp/thermal.off.conf");
    }

    private void changeToDynamicThermalManagement()
    {
        execShellCommand("/system/bin/thermal_manager /etc/.tp/thermal.conf");
    }
 
    private class ThermalThreadHandler extends Handler {
        private static final int MESSAGE_APP_RESUMED = 0;
        private static final int MESSAGE_TIMER = 1;

        public ThermalThreadHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            try {
                switch (msg.what) {
                    case MESSAGE_APP_RESUMED:
                    {
                        // Get current thermal policy. Switch to TP only when current policy is DTM.
                        boolean dtm = checkIsDTM();
                        Log.d(TAG, "handleMessage " + msg.what + ", thermal state: " + mState + ", dtm: " + dtm);

                        if (mState == ThermalSwitchState.Ready) {
                            if (true == dtm) {
                                // DTM: Change state to Disabling -> Switch to TP -> Start timer -> Change state to Disabled
                                mState = ThermalSwitchState.Disabling;
                                changeToThermalProtection();
                                stopTimer();
                                startTimer(msg.arg1);
                                mState = ThermalSwitchState.Disabled;
                            }
                        }
                        else if (mState == ThermalSwitchState.Disabled) {
                            if (true == dtm) {
                                // DTM: Change state to Disabling -> Stop timer -> Switch to TP -> Start timer -> Change state to Disabled
                                mState = ThermalSwitchState.Disabling;
                                stopTimer();
                                changeToThermalProtection();
                                startTimer(msg.arg1);
                                mState = ThermalSwitchState.Disabled;
                            }
                            else {
                                // TP: reset timer
                                stopTimer();
                                startTimer(msg.arg1);
                            }
                        }
                        break;
                    }

                    case MESSAGE_TIMER:
                    {
                        // Get current thermal policy. Switch to TP only when current policy is DTM.
                        boolean dtm = checkIsDTM();
                        Log.d(TAG, "handleMessage " + msg.what + ", thermal state: " + mState + ", dtm: " + dtm);

                        // Handle timeout message only when state is Disabled
                        if (mState == ThermalSwitchState.Disabled) {
                            if (true == dtm) {
                                // DTM: Change state to Ready
                                mState = ThermalSwitchState.Ready;
                            }
                            else {
                                // TP: Change state to Enabling -> Switch to DTM -> Change state to Ready
                                mState = ThermalSwitchState.Enabling;
                                changeToDynamicThermalManagement();
                                mState = ThermalSwitchState.Ready;
                            }
                        }
                        break;
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "Exception in ThermalThreadHandler.handleMessage: ", e);
            }
        }

        private void startTimer(int delay) {
            Message msg = this.obtainMessage(MESSAGE_TIMER);
            this.sendMessageDelayed(msg, delay * 1000);
        }

        private void stopTimer() {
            this.removeMessages(MESSAGE_TIMER);
        }

        private boolean checkIsDTM() {
            boolean ret = false;
            File f = new File("/data/.tp.settings");

            if(f.exists() == false) {
                // File not found -> apply one thermal policy and thermal will create the file
                execShellCommand("/system/bin/thermal_manager /etc/.tp/thermal.conf");
            }

            f = new File("/data/.tp.settings");
            if (f.exists()) {
                try {
                    // Open the file for reading
                    InputStream instream = new FileInputStream(f);

                    // If file is available for reading
                    if (instream != null) {
                        // Prepare the file for reading
                        InputStreamReader inputReader = new InputStreamReader(instream);
                        BufferedReader buffReader = new BufferedReader(inputReader);
                        String line;
                
                        if ((line = buffReader.readLine()) != null) {
                            if (line.equals("/etc/.tp/thermal.conf")) {
                                ret = true;
                            }
                        }
                    }
                
                    // Close the file
                    instream.close();
                } catch (IOException e) {}
            }

            return ret;
        }
    }

    BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "received intent " + action);

            // Check if app timeout is smaller than thermal reset time
            if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
                int appTimeout = Integer.MAX_VALUE;
                int resetTime = 60; // Default thermal reset time is 60s if can not get the value
                File f = new File("/data/.tp.settings");

                // Get the smallest app timeout                
                for (Object o : MtkThermalSwitchConfig.appConfig.keySet()) {
                    if (MtkThermalSwitchConfig.appConfig.get(o) < appTimeout) {
                        appTimeout = MtkThermalSwitchConfig.appConfig.get(o);
                    }
                }
                Log.d(TAG, "smallest app timeout: " + appTimeout + " seconds");

                // Get thermal reset time
                if(f.exists() == false) {
                    // File not found -> apply one thermal policy and thermal will create the file
                    execShellCommand("/system/bin/thermal_manager /etc/.tp/thermal.conf");
                }

                f = new File("/data/.tp.settings");
                if (f.exists()) {
                    try {
                        // Open the file for reading
                        InputStream instream = new FileInputStream(f);

                        // If file is available for reading
                        if (instream != null) {
                            // Prepare the file for reading
                            InputStreamReader inputReader = new InputStreamReader(instream);
                            BufferedReader buffReader = new BufferedReader(inputReader);
                            String line;

                            if (buffReader.readLine() != null) {
                                if ((line = buffReader.readLine()) != null) {
                                    resetTime = Integer.parseInt(line.trim());
                                }
                            }
                        }

                        // Close the file
                        instream.close();
                    } catch (IOException e) {}
                }
  
                Log.d(TAG, "thermal reset time: " + resetTime);

                // Compare app timeout with thermal reset time
                assertFalse(appTimeout < resetTime);

                mState = ThermalSwitchState.Ready;
            }
        }
    };
}
