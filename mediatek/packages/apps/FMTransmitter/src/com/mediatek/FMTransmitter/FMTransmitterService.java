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

package com.mediatek.FMTransmitter;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.media.AudioManager;
import android.os.IBinder;
import android.os.RemoteException;

import com.mediatek.common.featureoption.FeatureOption;

import java.util.Timer;
import java.util.TimerTask;

public class FMTransmitterService  extends Service {

    /**
     * FMTxLogUtils string
     */
    private static final String TAG = "FMTx/FMTxService";
    /**
     * audio path for set/get FM Tx state String
     */
    private static final String GETTXPATHENABLESTATE = "GetFmTxEnable";
    private static final String TXPATHENABLESTATE = "GetFmTxEnable=true";
    private static final String TXPATHDISABLESTATE = "GetFmTxEnable=false";
    private static final String SETTXPATHENABLE = "SetFmTxEnable=1";
    private static final String SETTXPATHDISENABLE = "SetFmTxEnable=0";
    //inform FM Rx to powerdown
    public static final String ACTION_TOFMSERVICE_POWERDOWN = 
        "com.mediatek.FMRadio.FMRadioService.ACTION_TOFMSERVICE_POWERDOWN";
    // Broadcast messages from FM Rx service to FM  Tx service.
    public static final String ACTION_TOFMTXSERVICE_POWERDOWN = 
        "com.mediatek.FMTransmitter.FMTransmitterService.ACTION_TOFMTXSERVICE_POWERDOWN";
    
    public static final String ACTION_MTV_POWERON = "com.mediatek.app.mtv.POWER_ON";
    //infrom app to update UI status
    public static final String EXTRA_FMTX_ISPOWERUP = "EXTRA_FMTX_ISPOWERUP";
    public static final String ACTION_STATE_CHANGED = 
        "com.mediatek.FMTransmitter.FMTransmitterService.ACTION_STATE_CHANGED";
    /**
     * all frequency must in this range (gdMinFrequency, gdMaxFrequency)
     * all input frequency must be checked before used as a parameter for service request
     */
    private static float sCurFrequency = 100.0f;
    private AudioManager mAudioManager = null;
    private static boolean sIsDeviceOpen = false;
    private static boolean sIsDevicePowerOn = false;
    private static boolean sIsRDSState = false;
    private static boolean sInitFlag = false;
    private static boolean sIsSearching = false;
    private static boolean sPowerDownForEarphone = false;
    private static final int NOTIFICATION_ID = 1;
    private static final float BASE_NUMBER = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 100.0f : 10.0f;
    private static final int POWER_DOWN_TIME = 5;//magic number
    private static final int MILLISECONEND = 1000;//magic number
    private static final int TIME_DELAY = 10; // magic number

    private FMTxServiceBroadcastReceiver mBroadcastReceiver = null;
    private HeadsetConnectionReceiver mHeadsetConnectionReceiver = null;
    private ListenMusicStatus mStatusMusicListener = new ListenMusicStatus();
    
    private class ListenMusicStatus {
        private int mCounter = 0;
        Timer mListenMusicStatusTimer = null;
        TimerTask mListenMusicStatusTask = null;
        private TimerTask getNewTask() {
            return new TimerTask() {
                public void run() {
                    if (stubs()) {
                        mCounter = 0;
                        FMTxLogUtils.e(TAG, "Music ON");
                    } else {
                        mCounter++;
                        FMTxLogUtils.e(TAG, "Music OFF for " + mCounter);
                        if (mCounter > POWER_DOWN_TIME) {
                            FMTxLogUtils.e(TAG, "Time out! powerdown it.");
                            try {
                                mBinder.powerDownTx();
                                mBinder.closeTxDevice();
                                FMTxLogUtils.i(TAG, 
                                        "music off for 5, power down device and close device.");
                            } catch (RemoteException e) {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }
                            mCounter = 0;                        
                        }
                    }
                }
            };
        }
        public void startListen() {
            if (null == mListenMusicStatusTimer) {
                mCounter = 0;
                mListenMusicStatusTimer = new Timer();
                mListenMusicStatusTask = getNewTask();
                mListenMusicStatusTimer.schedule(mListenMusicStatusTask, 0, 60 * MILLISECONEND);//60sec
            }
        }
        
        public void stopListen() {
            if (null != mListenMusicStatusTimer) {
                mListenMusicStatusTask.cancel();
                mListenMusicStatusTimer.cancel();
                mListenMusicStatusTimer.purge();
                mListenMusicStatusTask = null;
                mListenMusicStatusTimer = null;
            }
        }
        
        private boolean stubs() {
            //return false;
            return mAudioManager.isMusicActive();
        }
    }
    
    
    @Override
    public IBinder onBind(Intent arg0) {
        return mBinder;
    }
    private class FMTxServiceBroadcastReceiver extends BroadcastReceiver {
        
        public void onReceive(Context context, Intent intent) {
            FMTxLogUtils.d(TAG, "FMTxServiceBroadcastReceiver.onReceive");
            String action = intent.getAction();
            FMTxLogUtils.v(TAG, "Context: " + context);
            FMTxLogUtils.v(TAG, "Action: " + action);
            if (action.equals(ACTION_TOFMTXSERVICE_POWERDOWN) || action.equals(ACTION_MTV_POWERON)) {
                // set cmd to audio driver to notify FM Rx force FM Tx close.
                FMTxLogUtils.v(TAG, "Send CMD down to audio driver to notify Rx force close TX");
                mAudioManager.setParameters("FMRXForceDisableFMTX=1");
                try {
                    mBinder.powerDownTx();
                } catch (RemoteException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, "Exception: Cannot call binder function.");
                }
            } else if (action.equals(Intent.ACTION_SHUTDOWN)) {
                try {
                    mBinder.closeTxDevice();
                } catch (RemoteException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, "Exception: Cannot call binder function.");
                }
            }
        }
    }
    
    private class HeadsetConnectionReceiver extends BroadcastReceiver {
         public void onReceive(Context context, Intent intent) {
             if (intent.hasExtra("state")) {    
                 if (intent.getIntExtra("state", 0) == 0) { //unpluged
                     FMTxLogUtils.e(TAG, "Ear phone is removed. ");
                     sPowerDownForEarphone = false;//the variable is just set for search
                 } else if (intent.getIntExtra("state", 0) == 1) { //pluged    
                         FMTxLogUtils.e(TAG, "Ear phone is inserted. ");
                         if (sIsSearching) {
                             sPowerDownForEarphone = true;
                             FMTxLogUtils.w(TAG, "when searching ,the ear phone is inserted!");
                          } else {
                            if (sIsDevicePowerOn) {
                            try {
                                FMTxLogUtils.e(TAG, "Power down Tx ");
                                mBinder.powerDownTx();
                                mBinder.closeTxDevice();
                            } catch (RemoteException e) {
                                e.printStackTrace();
                                FMTxLogUtils.e(TAG, "Exception: powerDownTx() in HeadsetConnectionReceiver.");
                            }
                    }
                }
            }
                 
        }  
        }  
    };     
    
    public void onCreate() {
            super.onCreate();
            // Register broadcast receiver.
            IntentFilter filter = new IntentFilter();
            filter.addAction(ACTION_TOFMTXSERVICE_POWERDOWN);
            filter.addAction(ACTION_MTV_POWERON);
            filter.addAction(Intent.ACTION_SHUTDOWN);
            if (mBroadcastReceiver == null) {
                mBroadcastReceiver = new FMTxServiceBroadcastReceiver();
                FMTxLogUtils.d(TAG, "Register tx power down action broadcast receiver.");
                registerReceiver(mBroadcastReceiver, filter);
            }
            IntentFilter filterHeadset = new IntentFilter();
            filterHeadset.addAction(Intent.ACTION_HEADSET_PLUG);
            mHeadsetConnectionReceiver = new HeadsetConnectionReceiver();
            FMTxLogUtils.d(TAG, "Register HeadsetConnectionReceiver");
            registerReceiver(mHeadsetConnectionReceiver, filterHeadset);
            mAudioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
            FMTxLogUtils.i(TAG, "FMTransmitterService.onCreate");
            
            try {
                if (!mBinder.isFMTxSupport()) {
                    getApplicationContext()
                        .getPackageManager()
                        .setApplicationEnabledSetting(
                            "com.mediatek.FMTransmitter",
                            PackageManager.COMPONENT_ENABLED_STATE_DISABLED_USER,
                            0);
                } else {
                    getApplicationContext()
                        .getPackageManager()
                        .setApplicationEnabledSetting(
                            "com.mediatek.FMTransmitter",
                            PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                            0);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
                FMTxLogUtils.e(TAG, "Exception: Cannot call binder function.");
            }         
        }
        
        public void onDestroy() {
            // Unregister the broadcast receiver.
            if (null != mBroadcastReceiver) {
                FMTxLogUtils.d(TAG, "Unregister fm tx powerdown broadcast receiver.");
                unregisterReceiver(mBroadcastReceiver);
                mBroadcastReceiver = null;
            }
            // Unregister the headset connection receiver
            if (null != mHeadsetConnectionReceiver) {
                unregisterReceiver(mHeadsetConnectionReceiver);
                mHeadsetConnectionReceiver = null;
            }
            sIsSearching = false;
            sPowerDownForEarphone = false;        
            try {
                mBinder.closeTxDevice();
            } catch (RemoteException e) {
                e.printStackTrace();
                FMTxLogUtils.e(TAG, "Exception: Cannot call binder function.");
            }
            FMTxLogUtils.d(TAG, "FMTransmitterService.onDestroy");
            super.onDestroy();
        }
    
          // add for ICS to show Notification when Tx is in use
        private void showNotification() {
            Intent notificationIntent = new Intent();
            notificationIntent.setClassName(getPackageName(), FMTransmitterActivity.class.getName());
            PendingIntent pendingIntent = PendingIntent.getActivity(getApplicationContext(), 0,
                    notificationIntent, 0);
            Notification notification = new Notification(R.drawable.fmtx_title_icon, null,
                    System.currentTimeMillis());
            notification.flags |= Notification.FLAG_ONGOING_EVENT | Notification.FLAG_NO_CLEAR;
            String text = (float) sCurFrequency + " MHz";
            notification.setLatestEventInfo(getApplicationContext(),
                    getResources().getString(R.string.main_title), text, pendingIntent);
            FMTxLogUtils.d(TAG, "Add notification to the title bar.");
            startForeground(NOTIFICATION_ID, notification);
            FMTxLogUtils.d(TAG, "FMTransmitterService.showNotification");
        }
        
        private void removeNotification() {
            stopForeground(true);
            FMTxLogUtils.d(TAG, "FMTransmitterService.removeNotification");
        }

        private void updateNotification() {
            if (sIsDevicePowerOn) {
                showNotification();
            } else {
                FMTxLogUtils.w(TAG, "FM is not power up.");
            }
            FMTxLogUtils.d(TAG, "FMTransmitterService.updateNotification");
        }

        public void onConfigurationChanged(Configuration newConfig) {
            super.onConfigurationChanged(newConfig);
            // Change the notification string.
            if (sIsDevicePowerOn) {
                showNotification();
            }
        }
    //public class FMTransmitterServiceimpl extends IFMTransmitterService.Stub {
    private  IFMTransmitterService.Stub mBinder = new IFMTransmitterService.Stub() {
        /**
        *record the Init Flag and init station frequency
        *@return 
        *    
        */
        public boolean initService(float iCurrentFrequency) {
            sInitFlag = true;
            sCurFrequency = iCurrentFrequency;
            FMTxLogUtils.d(TAG, "initService");
            return true;
        }
        
        /**
        *return service's initialization state
        *@return 
        *    current service state
        */
        public boolean isServiceInit() {
            FMTxLogUtils.d(TAG, "isServiceInit : " + sInitFlag);
            return sInitFlag;
        }
        
        /**
        *return the last successful tx frequency
        *@return 
        *    current frequency
        */
        public float getCurFrequency() {
            FMTxLogUtils.d(TAG, "getCurFrequency : " + sCurFrequency);
            return sCurFrequency;
        }
        
        /**
         * open FM Tx device
         * @return 
         *         true if Tx device is opened successfully
         *         false otherwise
         */
        public boolean openTxDevice() {
            if (!sIsDeviceOpen) {
                if (FMTransmitterNative.opendev()) {
                    sIsDeviceOpen = true;
                }                
            }
            FMTxLogUtils.v(TAG, "openTxDevice, sIsDeviceOpen = " + sIsDeviceOpen);
            return sIsDeviceOpen;
        }
        
        /**
         * close FM Tx device
         * @return 
         *         true if Tx device is closed successfully
         *         false otherwise
         */
        public boolean closeTxDevice() {
            if (sIsDevicePowerOn) {
                powerDownTx();
            }
            if (sIsDeviceOpen) {
                if (FMTransmitterNative.closedev()) {
                    sIsDeviceOpen = false;
                }
            }
            FMTxLogUtils.d(TAG, "closeTxDevice");
            return !sIsDeviceOpen;
        }
        
        /**
         * check if Tx device is opened or not
         * @return 
         *         true if Tx device has been opened successfully
         *         false otherwise
         */
        public boolean isTxDeviceOpen() {
            FMTxLogUtils.d(TAG, "isTxDeviceOpen:DeviceOpenState = " + sIsDeviceOpen);
            
            return sIsDeviceOpen;
        }
        
        /**
         * Power up FM Tx at target frequency
         * @return 
         *         true if power up Tx at target frequency successfully
         *         false otherwise
         */
        public boolean powerUpTx(float frequency) {
            FMTxLogUtils.d(TAG, ">>>powerUpTx");
            if (!sIsSearching) {
            sendPowerdownFMRxMsg();
                try {
                    Thread.sleep(TIME_DELAY);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, "Exception: Cannot call binder function.");
                }
            }            
           
            //only if openTxDevice has been called successfully, can this function be called
            //otherwise return false;
            if (!sIsDeviceOpen) {
                FMTxLogUtils.d(TAG, "powerUpTx:Device is not open");
                return false;
            }
            //judge if Tx is power up or not first
            if (!sIsDevicePowerOn) {
                if (FMTransmitterNative.powerupTX(frequency)) {
                    sIsDevicePowerOn = true;
                    turnToFrequency(frequency);
                    if (!setAudioPathToFMTx()) {
                        FMTxLogUtils.e(TAG, "failed to set audio path to FM Tx");
                    }
                    mStatusMusicListener.startListen();
                    //add notification
                    showNotification();
                }
            }
            FMTxLogUtils.d(TAG, "<<<powerUpTx");
            
            return sIsDevicePowerOn;
        }

        /**
         * Power down FM Tx 
         * @return 
         *         true if power down Tx successfully 
         *         false otherwise
         */
        public boolean powerDownTx() {
            FMTxLogUtils.d(TAG, ">>> powerDownTx");
            //only if openTxDevice has been called successfully, can this function be called
            //otherwise return false;
            sIsRDSState = false;
            if (!sIsDeviceOpen) {
                FMTxLogUtils.d(TAG, "powerDownTx:Device is not open");
                return false;
            }    
            if (sIsDevicePowerOn) {
                if (!setAudioPathOutofFMTx()) {
                    FMTxLogUtils.e(TAG, "failed to set audio path out of FM Tx");
                }
                if (FMTransmitterNative.powerdown(1)) { 
                    //0, FM_RX; 1, FM_TX
                    sIsDevicePowerOn = false;
                    mStatusMusicListener.stopListen();
                    sPowerDownForEarphone = false;//receive intent when searching
                    FMTxLogUtils.d(TAG, "success to power down FM Tx");
                }
            }
            // remove Tx Notification
            removeNotification();
            // Broadcast message to applications.
            Intent intent = new Intent(ACTION_STATE_CHANGED);
            intent.putExtra(EXTRA_FMTX_ISPOWERUP, sIsDevicePowerOn);
            sendBroadcast(intent);
            FMTxLogUtils.d(TAG, "<<< powerDownTx");
            return !sIsDevicePowerOn;
        }
        
        public boolean repowerTx(float frequency) {
            if (!sIsDevicePowerOn) {
                return true;
            }
            //0, FM_RX; 1, FM_TX
            if (!FMTransmitterNative.powerdown(1)) {
                FMTxLogUtils.e(TAG, "failed to power down FM Tx");
                return false;
            }
            if (FMTransmitterNative.powerupTX(frequency)) {                
                turnToFrequency(frequency);                
            } else {
                FMTxLogUtils.e(TAG, "failed turnToFrequency");
                return false;
            }
            return true;
        }
        /**
         * check if Tx is in power up state or not
         * @return 
         *         true if Tx is in  power up status
         *         false otherwise
         */
        public boolean isTxPowerUp() {
            FMTxLogUtils.d(TAG, "isTxPowerUp:DevicePowerState = " + sIsDevicePowerOn);    
            return sIsDevicePowerOn;
        }
        
        public boolean isSearching() {
            FMTxLogUtils.d(TAG, "sIsSearching: " + sIsSearching);    
            return sIsSearching;
        }
        
        public boolean turnToFrequency(float frequency) { 
            FMTxLogUtils.d(TAG, ">>> turnToFrequency " + frequency + "result:");
            boolean result = FMTransmitterNative.tuneTX(frequency);
            if (result) {
                sCurFrequency = frequency;
            }
            updateNotification();
            FMTxLogUtils.d(TAG, "<<< turnToFrequency " + frequency + "result:" + result);   
            return result;
        }
        
        public float[] searchChannelsForTx(float frequency, int direction, int number)
        {
            FMTxLogUtils.d(TAG, ">>> searchChannelsForTx begin at "
                    + frequency + " direction = " + direction + " number = " + number);
            sIsSearching = true;
            float[] channelList = null;
            boolean txOriPowerState =  sIsDevicePowerOn;
            //open device
            if (!openTxDevice()) {
                FMTxLogUtils.e(TAG, "<<< searchChannelsForTx, opendevice fm failed");
                return null;
            }
            if (sIsDevicePowerOn) {
                //if device is in FM Tx power up state, we need to power down tx first
                FMTxLogUtils.d(TAG, "fm tx should be powered down before search channels");
                //0, FM_RX; 1, FM_TX
                if (FMTransmitterNative.powerdown(1)) {
                    sIsDevicePowerOn = false;
                }
            }
            
            if (FMTransmitterNative.powerup(frequency)) {
                    long startSeekTime = System.currentTimeMillis();
                    FMTxLogUtils.i(TAG,
                            "[Performance test][FMTransmitter] Test FM Tx Native seek time start [" + startSeekTime + "]");
                    short[] availableFre = FMTransmitterNative.getTXFreqList(frequency, (short)direction, number);
                    long endSeekTime = System.currentTimeMillis();
                    FMTxLogUtils.i(TAG,
                            "[Performance test][FMTransmitter] Test FM Tx Native seek time end [" + endSeekTime + "]");
                    //if(availableFre.length > 0)
                    if (availableFre != null) {    
                        channelList = new float[availableFre.length];
                        for (int i = 0 ;i < availableFre.length; i++) {
                            channelList[i] = availableFre[i] / BASE_NUMBER;
                        }
                    }
                    //0, FM_RX; 1, FM_TX
                    if (!FMTransmitterNative.powerdown(1)) {
                        FMTxLogUtils.e(TAG, "power down rx failed");
                    }
                    FMTxLogUtils.e(TAG, "power up rx successfully");
                } 
            if (txOriPowerState) {
                powerUpTx(sCurFrequency);
            }
            
            if (sPowerDownForEarphone) {
                powerDownTx();
                closeTxDevice();
                sPowerDownForEarphone = false;
            }
            
            FMTxLogUtils.d(TAG, "<<< searchChannelsForTx");
            sIsSearching = false;
            return channelList;
            //return FMTransmitterNative.getTXFreqList(frequency, number);
        }

        public boolean isRDSOn() throws RemoteException {
            FMTxLogUtils.d(TAG, "isRDSOn: RDSState = " + sIsRDSState);    
            return sIsRDSState;
        }

        public boolean isFMTxSupport() throws RemoteException {
            // TODO Auto-generated method stub
            return FMTransmitterNative.isTXSupport() == 1;
        }
        

        public boolean isRDSTxSupport() throws RemoteException {
            // TODO Auto-generated method stub
            return FMTransmitterNative.isRDSTXSupport() == 1;
        }

        public boolean setRDSTxEnabled(boolean state) throws RemoteException {
            FMTxLogUtils.d(TAG, ">>> setRDSTx");
            // TODO call setRDSEnable from jni
            boolean bRet = false;
            if (state != sIsRDSState) {
                bRet = FMTransmitterNative.setRDSTXEnabled(state);
                if (bRet) {
                    sIsRDSState = state;
                }
            }
            FMTxLogUtils.d(TAG, "setRDSTxEnabled:" + bRet);    
            FMTxLogUtils.d(TAG, "<<< setRDSTx");
            return bRet;

        }

        public boolean setRDSText(int pi, char[] ps, int[] rds, int rdsCnt) throws RemoteException {
            // TODO set RDS Tx content
            FMTxLogUtils.d(TAG, "setRDSText, pi = " + pi + " ps = " + new String(ps));
            
            int i = 0;
            short[] rdsText = null;
            char[] rdsPs = new char[4];
            boolean result = false;
            if (ps == null) {
                FMTxLogUtils.e(TAG, "Error: ps = null");
                return false;
            }
            if (ps.length != 8) {
                FMTxLogUtils.e(TAG, "Error: ps.length != 8");
                return false;
            }
            
            for (i = 0; i < 4; i++) {
                rdsPs[i] = 0x0;
                rdsPs[i] = ps[2 * i];
                rdsPs[i] += (ps[2 * i + 1] << 8);
            }
            
            rdsText = new short[rdsCnt];
            for (i = 0; i < rdsCnt; i++) {
                rdsText[i] = (short)rds[i];
            }
            //need to do some transfer job to 
            result = FMTransmitterNative.setRDSTX((short)pi, rdsPs, rdsText, rdsCnt);
            FMTxLogUtils.d(TAG, "<--setRDSText");
            return result;
        }
        
        /**
         * set current AudioPath to FMTx 
         * @return 
         *         true if if operation succeed
         *         false otherwise
         */
        public boolean setAudioPathToFMTx() {
            FMTxLogUtils.d(TAG, "setAudioPathToFMTx");
            String state  = null;
            if (null == mAudioManager) {
                mAudioManager = (AudioManager)getSystemService(AUDIO_SERVICE);
            }
            
            if (mAudioManager != null) {
                //check audio path first
                state = mAudioManager.getParameters(GETTXPATHENABLESTATE);
                if (state.equals(TXPATHENABLESTATE)) {
                    FMTxLogUtils.d(TAG, "set audio path to FM Tx succeed, audio state =\" " + state + "\"");
                    return true;
                }
                if (mAudioManager.setAudioPathToFMTx()) {
                    FMTxLogUtils.d(TAG, "set audio path to FM Tx succeed, audio state =\" " + state + "\"");
                    return true;
                }
            }
            return false;
        }
        
        /**
         * set current AudioPath out of FMTx 
         * @return 
         *         true if if operation succeed
         *         false otherwise
         */
        public boolean setAudioPathOutofFMTx() {
            FMTxLogUtils.d(TAG, "setAudioPathOutofFMTx");
            String state = null;
            if (null == mAudioManager) {
                mAudioManager = (AudioManager)getSystemService(AUDIO_SERVICE);
            }
            if (mAudioManager != null) {
                state = mAudioManager.getParameters(GETTXPATHENABLESTATE);
                if (!state.equals(TXPATHENABLESTATE)) {
                    //if audio path is not in fm tx, just return true
                    FMTxLogUtils.d(TAG, "set audio path out of FM Tx succeed, state = \"" + state + "\"");
                    return true;
                }
                //mAudioManager.setParameters(SETTXPATHDISENABLE);
                //state = mAudioManager.getParameters(GETTXPATHENABLESTATE);
                //if(state.equals(TXPATHDISABLESTATE))
                if (mAudioManager.setAudioPathOutofFMTx()) {
                    FMTxLogUtils.d(TAG, "set audio path out of FM Tx succeed, state = \"" + state + "\"");
                    return true;
                }
            }
            return false;
        }
        
        /**
         * set current AudioPath out of FMTx 
         * @return 
         *         true if if operation succeed
         *         false otherwise
         */
        public void sendPowerdownFMRxMsg() {
            FMTxLogUtils.d(TAG, "-->sendPowerdownFMRxMsg");
            Intent intent = new Intent();
            intent.setAction(ACTION_TOFMSERVICE_POWERDOWN);
            sendBroadcast(intent);
        }
        
        public boolean isEarphonePluged() {
            //return false;
            return mAudioManager.isWiredHeadsetOn();            
        }



    };
}
