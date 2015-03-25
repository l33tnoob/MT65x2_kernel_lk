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

package com.android.server.am;

import android.app.Activity;
import android.app.ActivityManagerNative;
import android.app.AppGlobals;
import android.app.IActivityManager;
import android.content.Context;
import android.content.IIntentReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.pm.ApplicationInfo;
import android.content.pm.ResolveInfo;
import android.media.AudioManager;
import android.net.wifi.IWifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Slog;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManagerImpl;
import android.view.WindowManagerPolicy;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.android.internal.app.ShutdownManager;
import com.android.internal.policy.PolicyManager;
import com.android.server.am.ActivityManagerService;
import com.android.server.am.BroadcastFilter;
import com.android.server.am.UserStartedState;

import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.mom.IMobileManagerService;

public final class ActivityManagerPlus {
    private static final String TAG = "ActivityManagerPlus";
    private static final boolean DEBUG_OOM = false;
    private static final boolean DEBUG_OOM_BOOST = false;

    final HandlerThread mHandlerThread = new HandlerThread("AMPlus",
    Process.THREAD_PRIORITY_FOREGROUND);
    final Handler mHandler;
    private Context mContext;
    private ActivityManagerService mService;

    private static View view;

    private ActivityRecord mTarget = null;

    /**
      *  only enable oom_adj when both the 3rdParty & in-house app lists are available.
      */
    private boolean mOomAdjEnabled = false;
    
    /**
      *  check if in house whitelist is available.
      */
    private boolean mHasInHouseWL = false;

    /**
      *  check if third party whitelist is available.
      */
    private boolean mHasThirdPartyWL = false;

    /**
      *  the highest priority of 3rd pary app which is the least recently used.
      */
    private String mThirdParyAppWinner = null; 

    private boolean mFlightModeOn = false;

    /**
      *  the time to identify which app is the least recently used.
      */
    private long mThirdParyAppWinnerTime = 0;

    static final int THIRD_PARTY_HIGHEST_SERVER_ADJ = ProcessList.SERVICE_ADJ + 1;
    static final int THIRD_PARTY_SERVER_ADJ = ProcessList.SERVICE_ADJ + 2;
    static final int OTHER_SERVER_ADJ = ProcessList.SERVICE_ADJ + 3;
    static final int BOOST_DOWNLOADING_ADJ = THIRD_PARTY_SERVER_ADJ;
    
    //List of supported 3rd party application list
    //final ArrayList<String> mThirdPartyAppWhiteList
    //          = new ArrayList<String>();

    //List of special boost applications
    final ArrayList<String> mBoostDownloadingAppList
                    = new ArrayList<String>();
        
    static final String[] mThirdPartyAppWhiteList = {
        "com.aol.mobile.aim",
        "com.facebook.katana", 
        "cn.com.fetion7",
        "com.fring",
        "android.process.hiyahoo",
        "android.process.msn.service",
        "com.nimbuzz",
        "mobi.qiss.plurq",
        "com.tencent.qq",
        "com.renren.mobile.android",
        "com.sina.weibo",
        "com.skype.raider",
        "com.twitter.android",
        "com.kaixin001.activity",
        "com.ebuddy.android",
        "com.google.android.talk"
        };

    //List of supported in house applications list
    //final ArrayList<String> mInHouseAppWhiteList
    //      = new ArrayList<String>();

    static final String[] mInHouseAppWhiteList = {
        "com.android.music",
        "android.process.media",
        "com.mediatek.FMRadio:remote",
        "com.mediatek.apst.target",
        "android.process.acore",
        "com.android.mms"
    };

    final ArrayList<String> mProcessWL = new ArrayList<String>();
    
    // default whitelist, need to be synced with ShutdownManager.java
    public static final String[] mProcessList = {
        "system",
        "com.mediatek.bluetooth",
        "android.process.acore",
        "com.android.wallpaper",
        "com.android.systemui",
        "com.mediatek.mobilelog",
    };

    // accurate service restart in-house service list
    public static final String[] mAccurateSvcRestartList = {
        "com.android.calendar/.widget.CalendarAppWidgetService",
        "com.android.contacts/.util.EmptyService",
        "com.android.deskclock/com.android.alarmclock.DigitalAppWidgetService",
        "com.android.gallery3d/.gadget.WidgetService",
        "com.android.inputmethod.latin/.LatinIME",
        "com.android.mms/.widget.MmsWidgetService",
        "com.mediatek.appwidget.weather/.UpdateService",
    };

    public ActivityManagerPlus(Context context, ActivityManagerService service) {
        Slog.i(TAG, "start ActivityManagerPlus");

        mContext = context;
        mService = service;
        mHasThirdPartyWL = true;
        mHasInHouseWL = true;
        mOomAdjEnabled = true;
        Slog.i(TAG, "support wl!");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());

        startHandler();
        
        for ( int index = 0 ; index < mProcessList.length ; index++ ) {
            mProcessWL.add(mProcessList[index]);
        }
        
        // read the processs list from system property - ipo.process.wl
        // It's for customization
        String processList = SystemProperties.get("persist.ipo.shutdown.process.wl", null);
        if ( processList != null ) {
            Slog.i(TAG,"Process whitelist: " + processList);
            ArrayList<String> processArrayList = new ArrayList<String>();
            parseStringIntoArrary("/", processList, processArrayList);

            for(int i = 0; i < processArrayList.size(); ++i) {
                String item = processArrayList.get(i);
                if(item.length() > 0){
                    if(item.startsWith("!") && item.length() > 1 && mProcessWL.contains(item.substring(1))){
                        mProcessWL.remove(item.substring(1));
                    }else if(!item.startsWith("!") && !mProcessWL.contains(item)){
                        mProcessWL.add(item);
                    }
                }
            }
        }
        
        for (String target : mProcessWL) {
            Slog.v(TAG, "app = " + target);
        }

/*
        if ("1".equals(SystemProperties.get("persist.sys.ipo.wifi", null))) {
            Slog.i(TAG,"enable wifi due to ipo");
            SystemProperties.set("persist.sys.ipo.wifi", "");
            IWifiManager wifiMgr
                    = IWifiManager.Stub.asInterface(ServiceManager.getService(Context.WIFI_SERVICE));
            try {                
                if (wifiMgr != null) {
                    wifiMgr.setWifiEnabled(true);
                } else {
                    Slog.i(TAG," can not get the IWifiManager binder");
                }
            } catch (RemoteException e) {
                Slog.i(TAG,"Wi-Fi operation failed: " + e);
            }
        }
*/
    }

    final void startHandler() {
        IntentFilter itFilter = new IntentFilter();
        itFilter.addAction("android.intent.action.BOOST_DOWNLOADING");
        itFilter.addAction("android.intent.action.ACTION_BOOT_IPO");
        itFilter.addAction("android.intent.action.ACTION_PREBOOT_IPO");
        itFilter.addAction("android.intent.action.ACTION_SHUTDOWN_IPO");
        itFilter.addAction("android.intent.action.black.mode");
        itFilter.setPriority(1000);

        // it's the workaround to record the mute/unmute state
        itFilter.addAction(AudioManager.RINGER_MODE_CHANGED_ACTION);

        Slog.i(TAG, "startHandler!");

        mContext.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Slog.i(TAG, "Receive: " + intent);
                if ("android.intent.action.BOOST_DOWNLOADING".equals(intent.getAction())) {
                    Bundle bundle = intent.getExtras();
                    String pkgName;
                    Boolean enabled;
                    if (bundle == null)
                        return;
                    pkgName = bundle.getString("package_name");
                    if (pkgName == null)
                        return;                    
                    enabled = bundle.getBoolean("enabled", false);
                    final int count = mBoostDownloadingAppList.size();
                    int i = count - 1;
                    Boolean alreadyInList = false;

                    if(count != 0) {
                        while (i>=0 && !mBoostDownloadingAppList.get(i).equals(pkgName)) {
                            i--;
                        }
                        if(i<0)
                            alreadyInList = false;
                        else
                            alreadyInList = true;
                    }

                    if(enabled && !alreadyInList)
                        mBoostDownloadingAppList.add(pkgName);
                    else if (!enabled && alreadyInList){
                        mBoostDownloadingAppList.remove(i);
                    }
                } else if ("android.intent.action.ACTION_PREBOOT_IPO".equals(intent.getAction())) {
                    Slog.i(TAG, "ipo PREBOOT_IPO");
                    final ShutdownManager stMgr = new ShutdownManager();
                    stMgr.preRestoreStates(mContext);
                    //stMgr.forceStopKillPackages(mContext); // forceStopPackage & kill again
                    //stMgr.prebootKillProcess(mContext); // forceStopPackage & kill again
                    
                    if(ShutdownManager.prebootKillProcessListSize() != 0)
                    mHandler.postDelayed(new Runnable(){
                        public void run(){
                            long start_time = SystemClock.uptimeMillis();
                            long current_time  = start_time; 

                            Slog.i(TAG, "start waiting for ril.ipo.radiooff & ril.ipo.radiooff.2");
                            do{
                                boolean DualTalkMode = PhoneFactory.isDualTalkMode();
                                String radiooff = SystemProperties.get("ril.ipo.radiooff", null);
                                String radiooff2 = SystemProperties.get("ril.ipo.radiooff.2", null);

                                Slog.i(TAG,
                                    "DualTalkMode = " + DualTalkMode +
                                    ", ril.ipo.radiooff = " + radiooff +
                                    ", ril.ipo.radiooff.2 = " + radiooff2);

                                if ("0".equals(radiooff) && (!DualTalkMode || (
                                        DualTalkMode && "0".equals(radiooff2)))
                                    ){
                                    Slog.i(TAG, "break waiting radiooff");
                                    break;
                                }else{
                                    Slog.i(TAG, "keep waiting radiooff");
                                    try{
                                        Thread.sleep(100);
                                    } catch (InterruptedException e) {}
                                }
                                current_time = SystemClock.uptimeMillis();
                            }while(current_time < (start_time + 5 * 1000));

                            if(start_time + 5 * 1000 <= current_time)
                                Slog.i(TAG, "wait radiooff timeout...");

                            Slog.i(TAG, "prebootKillProcess");
                    stMgr.prebootKillProcess(mContext); // forceStopPackage & kill again
                  
                        }
                    }, 500);
                    else
                        Slog.i(TAG, "prebootKillProcess list empty, don't need to perform kill");

                    removeAllTasks(false);
                    Slog.i(TAG, "finished");

                    // it's to close system dialog when IPO shutdown. For example, MMI error dialog
                    mService.mWindowManager.closeSystemDialogs();

                    if (PowerOffAlarmUtility.isAlarmBoot()) {
                        mFlightModeOn = Settings.Global.getInt(
                                mContext.getContentResolver(),
                                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
                        if (!mFlightModeOn) {
                            Slog.i(TAG, "ActivityManagerPlus turn on flight mode for powerOffAlarm");
                            Settings.Global.putInt(mContext.getContentResolver(),
                                    Settings.Global.AIRPLANE_MODE_ON, 1);
                        }
                    }

                } else if ("android.intent.action.ACTION_BOOT_IPO".equals(intent.getAction())) {
                    Slog.i(TAG, "ipo BOOT_IPO");

                    ShutdownManager stMgr = new ShutdownManager();
                    stMgr.restoreStates(mContext);
                    /// M: launch power-off alarm when IPO shutdown device @{
                    if (PowerOffAlarmUtility.isAlarmBoot()) {
                        Slog.v(TAG, "power off alarm enabled");
                        mHandler.postDelayed(new Runnable(){
                            public void run(){
                                mService.mPowerOffAlarmUtility.launchPowrOffAlarmIPO(!mFlightModeOn);
                            }
                        }, 500);
                    } else {
                        if ("unencrypted".equals(SystemProperties.get("ro.crypto.state", null))) {
                            Slog.i(TAG, "ipo BOOT_IPO: removeIPOWin");
                            mService.removeIPOWin(context);
                            synchronized (mService){
                                IPOBootCompletedLocked();
                            }
                        }
                    }
                    /// @}
                } else if ("android.intent.action.ACTION_SHUTDOWN_IPO".equals(intent.getAction())) {
                    /// M: power-off alarm feature
                    mService.mPowerOffAlarmUtility.mFirstBoot = false;
                    removeAllTasks(false);                    
                    /// M: Mobile Management Feature @{
                    monitorBootReceiver(true, "IPO Bootup Start");
                    /// @}
                    Slog.i(TAG, "handling SHUTDOWN_IPO finished");

                } else if ("android.intent.action.black.mode".equals(intent.getAction())) {
                    boolean mode = intent.getBooleanExtra("_black_mode", false);
                    if ( mode == true ) {
                        mService.createIPOWin(context);
                    }
                } else if (AudioManager.RINGER_MODE_CHANGED_ACTION.equals(intent.getAction())) {
                    int ringerMode = intent.getIntExtra(AudioManager.EXTRA_RINGER_MODE, -1);
                    String state = Integer.toString(ringerMode);
                    SystemProperties.set("persist.sys.mute.state", state);
                }
            }
        }, itFilter);
    }

    private void parseStringIntoArrary(String split, String strings, ArrayList<String> arrayList) {
        String[] str = strings.split(split);
        int length = str.length;
        for ( int i = 0 ; i< length ; i++ ) {
            arrayList.add(str[i]);
        }
    }


    public void updateRegisterReceivers(List<BroadcastFilter> receivers, Intent intent) {

        if ( true == FeatureOption.MTK_IPO_SUPPORT && Intent.ACTION_SHUTDOWN.equals(intent.getAction()) ) {
            // 0 -> normal shutdown
            // 1 -> ipo shutdown
            if ( 0 == intent.getIntExtra("_mode", 0) ) {
                Slog.v(TAG, "normal shutdown");
                return;
            }
            
            int size = receivers != null ? receivers.size() : 0;
            for ( int i = 0 ; i < size ; i++ ) {
                BroadcastFilter curr = receivers.get(i);
                for ( String target : mProcessWL ) {
                    if ( curr.receiverList.app.processName.equals(target) ) {
                        receivers.remove(i);
                        size--;
                        i--;
                        break;
                    }
                }
            }
        } else {
            return;
        }
    }
    
    public int updateOomAdjPlus(ProcessRecord app) {

        // not enabled, return the curAdj.
        if (!mOomAdjEnabled)
            return app.curAdj;

        // Boost enabled, check the adj now
        if (mBoostDownloadingAppList.size()>0) {
            for (String  boostApp : mBoostDownloadingAppList) {
                if (DEBUG_OOM_BOOST) Slog.i(TAG, "mBoostDownloadingAppList: " + boostApp + ", app:" + app.processName + ", adj:" + app.curAdj);
                if (app.processName.equals(boostApp)) {
                    if (app.curAdj > BOOST_DOWNLOADING_ADJ) {
                        app.curAdj = BOOST_DOWNLOADING_ADJ;
                        return BOOST_DOWNLOADING_ADJ;
                    }
                    return app.curAdj;
                }
            }
        }


        //  LCA special case to handle the service adj only.
        if (app.curAdj == ProcessList.SERVICE_ADJ) {
            
            // first consider the in-house applications.
            if (mHasInHouseWL) {
                for (String  inHouseApp : mInHouseAppWhiteList) {
                    if (DEBUG_OOM) Slog.i(TAG, "inHouseApp whitelist:" + inHouseApp + ", app:" + app.processName);
                    if (app.processName.equals(inHouseApp))
                        return app.curAdj;
                }
            }

            if (mHasThirdPartyWL) {
                // second consider the third party applications.
                for (String  thirdPartyApp : mThirdPartyAppWhiteList) {
                    if (DEBUG_OOM) Slog.i(TAG, "thirdPartyApp whitelist:" + thirdPartyApp + ", app:" + app.processName);
                    if (app.processName.equals(thirdPartyApp)) {
                        if (DEBUG_OOM) Slog.i(TAG, "mThirdParyAppWinner:" + mThirdParyAppWinner + 
                            ", mThirdParyAppWinnerTime:" + mThirdParyAppWinnerTime
                            + ", app.lastActivityTime:" + app.lastActivityTime );
                        //app is in white list, now decide who is the highest one by lru.
                        if (mThirdParyAppWinner == null) {
                            mThirdParyAppWinner = app.processName;
                            mThirdParyAppWinnerTime = app.lastActivityTime;
                        } else {
                            if (app.processName.equals(mThirdParyAppWinner)) {
                                // same process, update the time.
                                mThirdParyAppWinnerTime = app.lastActivityTime;
                            } else if (app.lastActivityTime > mThirdParyAppWinnerTime) {
                                // new winner.
                                mThirdParyAppWinner = app.processName;
                                mThirdParyAppWinnerTime = app.lastActivityTime;
                            } else {
                                // other 3rd party app in the white list.
                                app.curAdj = THIRD_PARTY_SERVER_ADJ;
                                return THIRD_PARTY_SERVER_ADJ;
                            }
                        }
                        app.curAdj = THIRD_PARTY_HIGHEST_SERVER_ADJ;
                        return THIRD_PARTY_HIGHEST_SERVER_ADJ;
                    }
                }
            }

            // others services.
            if (DEBUG_OOM) Slog.i(TAG, "other services: " + app.processName);
            app.curAdj = OTHER_SERVER_ADJ;
            return OTHER_SERVER_ADJ;
        }

        // app not belongs to SERVER_ADJ.
        return app.curAdj;
    }

    public void IPOBootCompletedLocked() {
        Slog.v(TAG, "IPOBootCompletedLocked");

        if (!SystemProperties.get("sys.boot_completed").equals("1")) {
            Slog.i(TAG, "sys.boot_completed is not set");
            SystemProperties.set("sys.boot_completed", "1");
            mService.sendFullBootCompletedIntentLocked();
            return;
        }

        mHandler.post(new Runnable() {
            public void run() {
                mHandler.removeCallbacks(this);
                List<BroadcastFilter> registeredReceivers = null;
                List<ResolveInfo> receivers = null;
                Intent intent = new Intent(Intent.ACTION_BOOT_COMPLETED, null);
                intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);

                try {
                    receivers = AppGlobals.getPackageManager().
                            queryIntentReceivers(intent, null, mService.STOCK_PM_FLAGS, 0);
                } catch (RemoteException e) {
                }

                registeredReceivers = mService.mReceiverResolver.queryIntent(intent, null, false, 0);

                int sizeOfRegisteredReceivers= registeredReceivers.size();
                for ( int i = 0 ; i < sizeOfRegisteredReceivers ; i++ ) {
                    BroadcastFilter curr = registeredReceivers.get(i);
                    for ( String target : mProcessWL ) {
                        if ( curr.receiverList.app.processName.equals(target) ) {
                            registeredReceivers.remove(i);
                            sizeOfRegisteredReceivers--;
                            i--;
                            break;
                        }
                    }                    
                }

                int sizeOfReceivers = receivers != null ? receivers.size() : 0;
                for ( int i = 0 ; i < sizeOfReceivers ; i++ ) {                                
                    ResolveInfo curt = (ResolveInfo)receivers.get(i);
                    //Slog.v(TAG, "curt.activityInfo.name = " + curt.activityInfo.name);
                    for (String target : mProcessWL) {
                        if (curt.activityInfo.processName.equals(target)) {
                            receivers.remove(i);
                            sizeOfReceivers--;
                            i--;
                            break;
                         }
                    }   
                }
                synchronized(mService) {                                           

                    for (int i=0; i<mService.mStartedUsers.size(); i++) {
                        final int userId = mService.mStartedUsers.keyAt(i);
                        intent.putExtra(Intent.EXTRA_USER_HANDLE, userId);
                        mService.broadcastSpecificIntentLocked(null, null, intent, null,
                                new BootEndIntentReceiver(mService.mAmPlus, "IPO Bootup End"),
                                0, null, null,
                                android.Manifest.permission.RECEIVE_BOOT_COMPLETED,
                                receivers, registeredReceivers, userId);
                    }
                }
                
            }
        });
    }

    /// M: add for power-off alarm @{
    public void setBootingVal(boolean val) {
        mService.mBooting = val;
    }
    /// @}
    
    public boolean checkNeedAccurateRestartService(ProcessRecord app) {
        if ((app.info.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
            return true;
        } else {
            Iterator<ServiceRecord> it = app.services.iterator();
            while (it.hasNext()) {
                ServiceRecord sr = it.next();
                for (String serviceName : mAccurateSvcRestartList) {
                    if (sr.shortName.equals(serviceName)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    
    private void removeAllTasks(boolean removeHomeTask) {
        synchronized(mService){
            ArrayList<ActivityStack> stacks = mService.mStackSupervisor.getStacks();
            for (ActivityStack stack : stacks) {
                ArrayList<TaskRecord> tasks = stack.getAllTasks();
                for (TaskRecord task : tasks) {
                    if (!task.isHomeTask() || removeHomeTask) {
                        mService.removeTask(task.taskId, 0);
                    }
                }
            }
        }
    }

    static class BootEndIntentReceiver extends IIntentReceiver.Stub {
        private ActivityManagerPlus mAmPlus = null;
        private String mCause = null;

        public BootEndIntentReceiver(ActivityManagerPlus amPlus, String cause) {
            mAmPlus = amPlus;
            mCause = cause;
        }

        @Override
        public void performReceive(Intent intent, int resultCode,
                String data, Bundle extras, boolean ordered,
                boolean sticky, int sendingUser) {
            /// M: Mobile Management Feature @{
            mAmPlus.monitorBootReceiver(false, mCause);
            /// @}
        }
    }

    /**
    * M: Mobile Management Feature: Auto-boot Control
    * This function will filiter out receivers receives boot related intent
    * according to the config in MoMS, and the intent can be customized
    * by modifing package com.mediatek.common.mom.BootReceiverPolicy.
    * 
    * @param intent The intent for broadcasting.
    * @param resloveList The receiver list to be filtered.
    * @param userId User ID for broadcasting.
    */
    void filterReceiver(Intent intent, List<ResolveInfo> resolveList, int userId) {
        if (FeatureOption.MTK_MOBILE_MANAGEMENT) {
            IBinder binder = ServiceManager.getService(Context.MOBILE_SERVICE);
            IMobileManagerService moms = IMobileManagerService.Stub.asInterface(binder);
            try {
                if (moms != null) {
                    moms.filterReceiver(intent, resolveList, userId);
                }
            } catch (RemoteException e) {
                // Should not happened
                Log.e(TAG, "filterReceiver() failed", e);
            }
        }
    }

    /**
    * M: Mobile Management Feature: Auto-boot Control
    * Decide the start and end point of time to filter the receivers
    * 
    * @param start start or stop to monitor.
    * @param cause the reason to start or stop moniting.
    */
    void monitorBootReceiver(boolean start, String cause) {
        if (FeatureOption.MTK_MOBILE_MANAGEMENT) {
            IBinder binder = ServiceManager.getService(Context.MOBILE_SERVICE);
            IMobileManagerService moms = IMobileManagerService.Stub.asInterface(binder);
            try {
                if (moms != null) {
                    if (start) {
                        moms.startMonitorBootReceiver(cause);
                    } else {
                        moms.stopMonitorBootReceiver(cause);
                    }
                }
            } catch (RemoteException e) {
                // Should not happened
                Log.e(TAG, "monitorBootReceiver() failed", e);
            }
        }
    }

}
