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

package com.mediatek.FMRadio;

import android.app.ActivityManager;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
//import android.bluetooth.BluetoothA2dp;
//import android.bluetooth.IBluetoothA2dp;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentProviderOperation;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.OperationApplicationException;
import android.content.res.Configuration;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.media.MediaPlayer;
import android.media.AudioSystem;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
//import android.server.BluetoothA2dpService;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.FMRadio.FMRadioStation.Station;
import com.mediatek.common.featureoption.FeatureOption;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

public class FMRadioService extends Service implements FMRecorder.OnRecorderStateChangedListener {
    public static final String TAG = "FmRx/Service";

    private static final String ACTION_SHUTDOWN_IPO = "android.intent.action.ACTION_SHUTDOWN_IPO";
    // Broadcast messages from clients to FM service.
    public static final String ACTION_TOFMSERVICE_POWERDOWN
                               = "com.mediatek.FMRadio.FMRadioService.ACTION_TOFMSERVICE_POWERDOWN";
    // Broadcast messages to FM Tx service.
    public static final String ACTION_TOFMTXSERVICE_POWERDOWN
                               = "com.mediatek.FMTransmitter.FMTransmitterService.ACTION_TOFMTXSERVICE_POWERDOWN";
    // Broadcast messages to mATV service.
    public static final String ACTION_TOATVSERVICE_POWERDOWN = "com.mediatek.app.mtv.ACTION_REQUEST_SHUTDOWN";
    // Broadcast messages to music service.
    public static final String ACTION_TOMUSICSERVICE_POWERDOWN = "com.android.music.musicservicecommand.pause";
    // Broadcast messages from mATV service.
    public static final String ACTION_FROMATVSERVICE_POWERUP = "com.mediatek.app.mtv.POWER_ON";
    // Broadcast to tell A2DP that FM has powered up / powered down
    public static final String FM_POWER_UP_MSG = "com.mediatek.FMRadio.FMRadioService.ACTION_TOA2DP_FM_POWERUP";
    public static final String FM_POWER_DOWN_MSG = "com.mediatek.FMRadio.FMRadioService.ACTION_TOA2DP_FM_POWERDOWN";
    // Broadcast messages from other sounder APP to FM service
    public static final String SOUND_POWER_DOWN_MSG = "com.android.music.musicservicecommand";
    public static final String CMDPAUSE = "pause";
    // message for HandlerThread
    private static final String FM_FREQUENCY = "frequency";
    private static final String OPTION = "option";
    private static final String RECODING_FILE_NAME = "name";

    // RDS events
    public static final int RDS_EVENT_PROGRAMNAME = 0x0008; // PS
    public static final int RDS_EVENT_LAST_RADIOTEXT = 0x0040; // RT
    public static final int RDS_EVENT_AF = 0x0080; // AF
    // Headset plug in
    private static final int HEADSET_PLUG_IN = 1;
    // notification id
    private static final int NOTIFICATION_ID = 1;
    
    //must check AudioSystem.usage when google upgrade
    private static final int FOR_PROPRIETARY = 5;
    

    private int mForcedUseForMedia;

    // get BluetoothA2dp instance to get bluetooth state
//    private IBluetoothA2dp mA2dpService = null;
    // record whether FM is over BT controller
    private boolean mUsingFMViaBTController = false;
    
    // BT properties use to judge chip support over bt or not
    private static final String BT_PROPERTIES = "bt.fmoverbt";
    private static final boolean SHORT_ANNTENNA_SUPPORT = 
                                 FeatureOption.MTK_FM_SHORT_ANTENNA_SUPPORT;

    // Binder use BT to link, if this binder died,BluetoothA2dpService will
    // release channel
    private IBinder mICallBack = new Binder();
    // FM recorder
    FMRecorder mFMRecorder = null;
    // monitor the SD card whether mount or unmount
    private BroadcastReceiver mSDListener = null;
    // record FMRecord sate
    private int mRecordState = FMRecorder.STATE_INVALID;
    // record the error type occur in FMRecorder
    private int mErrorType = -1;
    // ALPS01226747 if eject record sdcard, should set Value false to not record.
    // Key is sdcard path(like "/storage/sdcard0"), V is to enable record or not.
    private HashMap<String, Boolean> mSdcardStateMap = new HashMap<String, Boolean>();

    // record the listener list, will notify all listener in list
    private ArrayList<Record> mRecords = new ArrayList<Record>();

    // broadcast to receive the external event
    private FMServiceBroadcastReceiver mBroadcastReceiver = null;

    // RDS Strings.
    private String mPSString = ""; // PS String
    private String mLRTextString = ""; // RT String

    // RDS settings
    // record whether PS RT enabled or not
    private boolean mIsPSRTEnabled = false;
    // record whether AF enabled or not
    private boolean mIsAFEnabled = false;
    // record whether FM is in native scan state
    private boolean mIsNativeScanning = false;
    // record whether FM is in scan thread
    private boolean mIsScanning = false;
    // record whether FM is in seeking state
    private boolean mIsNativeSeeking = false;
    // record whether FM is in native seek
    private boolean mIsSeeking = false;
    // record whether searching progress is canceled
    private boolean mIsStopScanCalled = false;
    // RDS thread use to receive the information send by station
    private Thread mRDSThread = null;
    // record whether RDS thread exit
    private boolean mIsRDSThreadExit = false;
    // record whether is speaker used
    private boolean mIsSpeakerUsed = false;
    // record whether device is open
    private boolean mIsDeviceOpen = false;
    // record whether FM is power up
    private boolean mIsPowerUp = false;
    // check whether is power uping, if so, should judge in activity back key.
    private boolean mIsPowerUping = false;
    // record whether service is init
    private boolean mIsServiceInit = false;
    // fm power down by loss audio focus,should make power down menu item can
    // click
    private boolean mIsMakePowerDown = false;
    // check whether no short antenna support
    private boolean mIsShortAntennaSupport = true;
    
    // context
    private Context mContext = null;
    // record current station frequency
    private int mCurrentStation = FMRadioUtils.DEFAULT_STATION;
    // record is headset plug state
    private int mValueHeadSetPlug = 1;
    // audio manager instance use to set volume stream type and use audio focus
    private AudioManager mAudioManager = null;
    // Activity manager service to check activity state
    private ActivityManager mActivityManager = null;
    // FM player use to play the voice data receive from station
    private MediaPlayer mFMPlayer = null;
    // wake lock use when FM play not let CPU enter sleep mode,
    // need to adjust volume when screen off
    private WakeLock mWakeLock = null;
    // record FM whether in recording mode
    private boolean mIsInRecordingMode = false;
    // record sd card path when start recording
    private static String sRecordingSdcard = FMRadioUtils.getDefaultStoragePath();
    // binder use to return the service instance to activity
    private final IBinder mBinder = new ServiceBinder();

    private boolean mIsAudioFocusHeld = false;

    private boolean mPausedByTransientLossOfFocus = false;

    private FmRadioServiceHandler mFmServiceHandler;
    // ALPS01077955 Lock for lose audio focus and receive SOUND_POWER_DOWN_MSG at the same time
    // while recording call stop recording not finished(status is still RECORDING), but
    // SOUND_POWER_DOWN_MSG will exitFM(), if it is RECORDING will discard the record.
    // 1. lose audio focus -> stop recording(lock) -> set to IDLE and show save dialog
    // 2. exitFM() -> check the record status, discard it if it is recording status(lock)
    // Add this lock the exitFM() while stopRecording()
    private Object mStopRecordingLock = new Object();

    @Override
    public IBinder onBind(Intent intent) {
        LogUtils.d(TAG, "FMRadioService.onBind: " + intent);
        /*
         * do power up in onBind method, when ap bind service, it will be power up.
         * ALPS00527321
         * ALPS00542386
         */
        int iCurrentStation = FMRadioStation.getCurrentStation(mContext);
        powerUpAsync(FMRadioUtils.computeFrequency(iCurrentStation));
        return mBinder;
    }

    /**
     * class use to return service instance
     * 
     */
    public class ServiceBinder extends Binder {
        /**
         * get FM service instance
         * 
         * @return service instance
         */
        FMRadioService getService() {
            return FMRadioService.this;
        }
    }

    /**
     * Broadcast monitor external event, Other app want FM stop, Phone shut
     * down, screen state, headset state, FM over bt state
     * 
     */
    private class FMServiceBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            LogUtils.d(TAG, ">>> FMRadioService.onReceive");
            String action = intent.getAction();
            String command = intent.getStringExtra("command");
            LogUtils.d(TAG, "Action/Command: " + action + " / " + command);
            // other app want FM stop, stop FM
            if (ACTION_TOFMSERVICE_POWERDOWN.equals(action) || ACTION_FROMATVSERVICE_POWERUP.equals(action)
                    || (SOUND_POWER_DOWN_MSG.equals(action) && CMDPAUSE.equals(command))) {
                // need remove all messages, make power down will be execute
                mFmServiceHandler.removeCallbacksAndMessages(null);
                /*
                 * ALPS01006936
                 * if camera record, FM exit
                 */
                //if (SOUND_POWER_DOWN_MSG.equals(action)) {
                    LogUtils.d(TAG, "onReceive.SOUND_POWER_DOWN_MSG. exit FM");
                    exitFM();
                    stopSelf();
                ///}
                
                //stopFMFocusLoss(AudioManager.AUDIOFOCUS_LOSS);
                // stop play FM recording file
                /*if (FeatureOption.MTK_FM_RECORDING_SUPPORT && mFMRecorder != null) {
                    if (mFMRecorder.getState() == FMRecorder.STATE_PLAYBACK) {
                        // stop play back, should remove play back messages in
                        // thread handler.
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTPLAYBACK_FINISHED);
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPPLAYBACK_FINISHED);
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTRECORDING_FINISHED);
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPRECORDING_FINISHED);
                        mFMRecorder.stopPlayback();
                    }
                }*/
                // phone shut down, so exit FM
            } else if (Intent.ACTION_SHUTDOWN.equals(action) || ACTION_SHUTDOWN_IPO.equals(action)) {
                /**
                 * 
                 * here exitFM, system will send broadcast, system will shut
                 * down, so fm does not need call back to activity
                 */
                mFmServiceHandler.removeCallbacksAndMessages(null);
                exitFM();
                // screen on, if FM play, open rds
            } else if (Intent.ACTION_SCREEN_ON.equals(action)) {

                setRDSAsync(true);
                // screen off, if FM play, close rds
            } else if (Intent.ACTION_SCREEN_OFF.equals(action)) {
                // mFmServiceHandler.removeMessages(SET_RDS_MSG);
                setRDSAsync(false);
                // switch antenna when headset plug in or plug out
            } else if (Intent.ACTION_HEADSET_PLUG.equals(action)) {
                // switch antenna should not impact audio focus status
                mValueHeadSetPlug = (intent.getIntExtra("state", -1) == HEADSET_PLUG_IN) ? 0 : 1;
                switchAntennaAsync(mValueHeadSetPlug);
                if (SHORT_ANNTENNA_SUPPORT) {
                    boolean isSwitch = (switchAntenna(mValueHeadSetPlug) == 0) ? true : false;
                    LogUtils.d(TAG, "onReceive.switch anntenna:isWitch:" + isSwitch);
                } else {
                    // ALPS01006910: Avoid Service is killed, and receive headset plug in broadcast again
                    if (!mIsServiceInit) {
                        LogUtils.d(TAG, "onReceive.switch anntenna:service is not init");
                        return;
                    }
                    /* ALPS01006939.
                     * if ear phone insert and activity is foreground.
                     * power up FM automatic
                     * */
                    if ((0 == mValueHeadSetPlug) && isActivityForeGround()) {
                        LogUtils.d(TAG, "onReceive.switch anntenna:need auto power up");
                        powerUpAsync(FMRadioUtils.computeFrequency(mCurrentStation));
                    } else if (1 == mValueHeadSetPlug){
                        if (mRecordState == FMRecorder.STATE_PLAYBACK || isActivityForeGround()) {
                            LogUtils.d(TAG, "onReceive.switch anntenna:playback or forground need to stop fm");
                            mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SCAN_FINISHED);
                            mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SEEK_FINISHED);
                            mFmServiceHandler.removeMessages(FMRadioListener.MSGID_TUNE_FINISHED);
                            mFmServiceHandler.removeMessages(FMRadioListener.MSGID_POWERDOWN_FINISHED);
                            mFmServiceHandler.removeMessages(FMRadioListener.MSGID_POWERUP_FINISHED);
                            stopFMFocusLoss(AudioManager.AUDIOFOCUS_LOSS);
                        } else {
                            if (!isActivityForeGround()) {
                                LogUtils.d(TAG, "onReceive.switch anntenna:background need to exit fm");
                                mFmServiceHandler.removeCallbacksAndMessages(null);
                                exitFM();
                                stopSelf();
                            }
                        }
                    }
                }
            } /*else if (BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED.equals(action)) {
                // Change FM chip status according to A2dp connect state and
                // current FM chip state
                int connectState = intent.getIntExtra(BluetoothA2dp.EXTRA_STATE, 0);
                LogUtils.d(TAG, "ACTION_CONNECTION_STATE_CHANGED: connectState=" + connectState + ", ispowerup="
                        + mIsPowerUp);
                handleBtConnectState(connectState);

            } else if (BluetoothA2dpService.ACTION_FM_OVER_BT_CONTROLLER.equals(action)) {
                // change to controller if necessary
                // stop FM player btw.
                int fmOverBTState = intent.getIntExtra(BluetoothA2dpService.EXTRA_RESULT_STATE,
                        BluetoothA2dpService.FMSTART_SUCCESS);
                LogUtils.d(TAG, "handling ACTION_FM_OVER_BT_CONTROLLER: " + fmOverBTState);
                fmOverBtController(fmOverBTState);
            } else if (action.equals(BluetoothA2dpService.ACTION_FM_OVER_BT_HOST)) {
                // change back to host if necessary
                // re-start FM player btw.
                LogUtils.d(TAG, "ACTION_FM_OVER_BT_HOST");
                if (!FMRadioNative.setFMViaBTController(false)) {
                    LogUtils.e(TAG, "failed to set FM over BT via Host!!");
                    return;
                }
                LogUtils.d(TAG, "setFMViaBTController(false) succeeded!!");
                mUsingFMViaBTController = false;
                enableFMAudio(true);
            } */else {
                LogUtils.w(TAG, "Error: undefined action.");
            }
            LogUtils.d(TAG, "<<< FMRadioService.onReceive");
        }
    }

    /**
     * whether antenna available
     * 
     * @return true, antenna available; false, antenna not available
     */
    public boolean isAntennaAvailable() {
        return FeatureOption.MTK_MT519X_FM_SUPPORT ? true : mAudioManager.isWiredHeadsetOn();
    }
    
    /**
     * whether short antenna support
     * @return true, support; false, not support
     */
    public boolean isShortAntennaSupport() {
        return mIsShortAntennaSupport;
    }
    
    public void setSpeakerPhoneOn(boolean isSpeaker) {
        LogUtils.d(TAG, ">>> FMRadioService.useSpeaker: " + isSpeaker);
        mForcedUseForMedia = isSpeaker ? AudioSystem.FORCE_SPEAKER : AudioSystem.FORCE_NONE;
        AudioSystem.setForceUse(FOR_PROPRIETARY, mForcedUseForMedia);
        mIsSpeakerUsed = isSpeaker;
        LogUtils.d(TAG, "<<< FMRadioService.useSpeaker");
    }
    
    
    private boolean isSpeakerPhoneOn() {
        return (mForcedUseForMedia == AudioSystem.FORCE_SPEAKER);
    }

    /**
     * open FM device, should be call before power up
     * 
     * @return true if FM device open, false FM device not open
     */

    private boolean openDevice() {
        LogUtils.d(TAG, ">>> FMRadioService.openDevice");
        if (!mIsDeviceOpen) {
            mIsDeviceOpen = FMRadioNative.opendev();
        }
        LogUtils.d(TAG, "<<< FMRadioService.openDevice: " + mIsDeviceOpen);
        return mIsDeviceOpen;
    }

    /**
     * close FM device
     * 
     * @return true if close FM device success, false close FM device failed
     */
    private boolean closeDevice() {
        LogUtils.d(TAG, ">>> FMRadioService.closeDevice");
        boolean isDeviceClose = false;
        if (mIsDeviceOpen) {
            isDeviceClose = FMRadioNative.closedev();
            mIsDeviceOpen = !isDeviceClose;
        }
        LogUtils.d(TAG, "<<< FMRadioService.closeDevice: " + isDeviceClose);
        
        // quit looper
        mFmServiceHandler.getLooper().quit();
        return isDeviceClose;
    }

    /**
     * get FM device opened or not
     * 
     * @return true FM device opened, false FM device closed
     */
    public boolean isDeviceOpen() {
        LogUtils.d(TAG, "FMRadioService.isDeviceOpen: " + mIsDeviceOpen);
        return mIsDeviceOpen;
    }

    /*private boolean setBtStatus(boolean isPowerUp) {
        if (!FeatureOption.MTK_BT_FM_OVER_BT_VIA_CONTROLLER) {
            return false;
        }
        
        if (!chipSupportOverBt()) {
            return false;
        }

        if (mA2dpService == null) {
            IBinder b = ServiceManager.getService(BluetoothA2dpService.BLUETOOTH_A2DP_SERVICE);
            if (b != null) {
                mA2dpService = IBluetoothA2dp.Stub.asInterface(b);
            }
        }

        if (mA2dpService != null) {
            final String msg;
            int a2dpState = -1;
            try {
                a2dpState = mA2dpService.getState();
            } catch (RemoteException re) {
                LogUtils.e(TAG, "binder error!!");
            }

            if (isPowerUp) {
                // Notify A2dp about power up event
                try {
                    mA2dpService.setAudioPathToAudioTrack(mICallBack);
                } catch (RemoteException re) {
                    LogUtils.e(TAG, "binder error!!");
                }
                LogUtils.d(TAG, "powerup: mA2dpService=" + mA2dpService);
                LogUtils.d(TAG, "powerup: mA2dpService.getState()=" + a2dpState);
                msg = FM_POWER_UP_MSG;
                if (a2dpState == BluetoothA2dp.STATE_NOT_PLAYING) {
                    // A2dp connected, so send out
                    // FM_POWER_UP_MSG
                    LogUtils.d(TAG, FM_POWER_UP_MSG + " sent to A2dp service!!");
                    sendBroadcast(new Intent(msg));
                    return true;
                }

            } else {
                // Notify A2dp about power down event
                LogUtils.d(TAG, "powerdown: mA2dpService != null");
                LogUtils.d(TAG, "powerdown: mA2dpService.getState()=" + a2dpState);
                msg = FM_POWER_DOWN_MSG;
            }

            if (a2dpState == BluetoothA2dp.STATE_CONNECTED || a2dpState == BluetoothA2dp.STATE_PLAYING) {
                sendBroadcast(new Intent(msg));
                return true;
            }

        }
        return false;
    }*/

    /**
     * power up FM, and make FM voice output from earphone or BT
     * 
     * @param frequency
     * @return
     */
    public void powerUpAsync(float frequency) {
        mIsPowerUping = true;
        final int bundleSize = 1;
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_POWERUP_FINISHED);
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_POWERDOWN_FINISHED);
        Bundle bundle = new Bundle(bundleSize);
        bundle.putFloat(FM_FREQUENCY, frequency);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_POWERUP_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private boolean powerUpFM(float frequency) {
        // after discuss with driver, not need to wait tx power down,
        // driver will do this

        LogUtils.d(TAG, ">>> FMRadioService.powerUp: " + frequency);

        if (mIsPowerUp) {
            LogUtils.d(TAG, "<<< FMRadioService.powerUp: already power up:" + mIsPowerUp);
            return true;
        }
        sendBroadcastToStopOtherAPP();

        if (!requestAudioFocus()) {
            // activity used for update powerdown menu
            mIsMakePowerDown = true;
            LogUtils.d(TAG, "FM can't get audio focus when power up");
            return false;
        }
        
        // if device open fail when chip reset, it need open device again before power up
        if (!mIsDeviceOpen) {
            openDevice();
        }
            
        long time = System.currentTimeMillis();
        Log.d(TAG, "performance test. service native power up start:" + time);
        if (!FMRadioNative.powerup(frequency)) {
            LogUtils.e(TAG, "Error: powerup failed.");
            return false;
        }
        time = System.currentTimeMillis();
        Log.d(TAG, "performance test. service native power up end:" + time);
        mIsPowerUp = true;
        // need mute after power up
        setMute(true);

        // activity used for update powerdown menu
        mIsMakePowerDown = false;
        LogUtils.d(TAG, "<<< FMRadioService.powerUp: " + mIsPowerUp);
        return mIsPowerUp;
    }

    private boolean initDevice(float frequency) {
        LogUtils.d(TAG, ">>> FMRadioService.initDevice: " + frequency);

        mCurrentStation = FMRadioUtils.computeStation(frequency);
        FMRadioStation.setCurrentStation(mContext, mCurrentStation);
        // Add notification to the title bar.
        showNotification();

        // Start the RDS thread if RDS is supported.
        if (isRDSSupported()) {
            LogUtils.d(TAG, "RDS is supported. Start the RDS thread.");
            startRDSThread();
        }

        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
            LogUtils.d(TAG, "acquire wake lock");
        }
        // setMute(true);
        //if (!setBtStatus(true)) {
            if (mIsSpeakerUsed != isSpeakerPhoneOn()) {
                setSpeakerPhoneOn(mIsSpeakerUsed);
            }
            if (mRecordState != FMRecorder.STATE_PLAYBACK) {
                enableFMAudio(true);
            }
//        }
        if (!isAntennaAvailable()) {
            // Antenna not ready, try short antenna
            if (switchAntenna(1) != 0) {
                LogUtils.e(TAG, "Error while trying to switch to short antenna: ");
            }
            // add tune because FMTx has power up, antenna has switch to FMTx,
            // call tune to make switch antenna effective
            FMRadioNative.tune(FMRadioUtils.computeFrequency(mCurrentStation));
        }
        setRDS(true);
        setMute(false);

        LogUtils.d(TAG, "<<< FMRadioService.initDevice: " + mIsPowerUp);
        return mIsPowerUp;
    }

    /**
     * send broadcast to connect bt device
     */
    private void connectBtDevice() {
        // A2dp connected, so send out
        // FM_POWER_UP_MSG
        Intent i = new Intent(FM_POWER_UP_MSG);
        LogUtils.d(TAG, FM_POWER_UP_MSG + " sent to A2dp service!!");
        sendBroadcast(i);
    }

    /**
     * send broadcast to stop other application, such as music, MATV,
     * FMTransmitter
     */
    private void sendBroadcastToStopOtherAPP() {
        Intent intentToMusic = new Intent(ACTION_TOMUSICSERVICE_POWERDOWN);
        sendBroadcast(intentToMusic);
        if (FeatureOption.MTK_MT519X_FM_SUPPORT) {
            Intent intentToAtv = new Intent(ACTION_TOATVSERVICE_POWERDOWN);
            sendBroadcast(intentToAtv);
        }
        if (!FeatureOption.MTK_MT519X_FM_SUPPORT) {
            Intent intentToFMTx = new Intent(ACTION_TOFMTXSERVICE_POWERDOWN);
            sendBroadcast(intentToFMTx);
        }
    }

    /**
     * power down FM
     * 
     * @return
     */
    public void powerDownAsync() {
        // if power down Fm, should remove message first.
        // not remove all messages, because such as recorder message need
        // to execute after or before power down
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SCAN_FINISHED);
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SEEK_FINISHED);
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_TUNE_FINISHED);
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_POWERDOWN_FINISHED);
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_POWERUP_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FMRadioListener.MSGID_POWERDOWN_FINISHED);
    }

    public boolean powerDown() {
        LogUtils.d(TAG, ">>> FMRadioService.powerDown");

        if (!mIsPowerUp) {
            LogUtils.w(TAG, "Error: device is already power down.");
            return true;
        }

        setMute(true);
        setRDS(false);
        enableFMAudio(false);

        if (!FMRadioNative.powerdown(0)) {
            LogUtils.e(TAG, "Error: powerdown failed.");
         // activity used for update powerdown menu
            mIsMakePowerDown = true;
            mUsingFMViaBTController = false;

            if (isRDSSupported()) {
                LogUtils.d(TAG, "RDS is supported. Stop the RDS thread.");
                stopRDSThread();
            }
            
            mIsPowerUp = false;
            
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
                LogUtils.d(TAG, "release wake lock");
            }

//            setBtStatus(false);

            // Remove the notification in the title bar.
            removeNotification();
            LogUtils.d(TAG, "powerdown failed.release some resource.");
            return false;
        }
        // activity used for update powerdown menu
        mIsMakePowerDown = true;
        mUsingFMViaBTController = false;

        if (isRDSSupported()) {
            LogUtils.d(TAG, "RDS is supported. Stop the RDS thread.");
            stopRDSThread();
        }
        
        mIsPowerUp = false;
        
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
            LogUtils.d(TAG, "release wake lock");
        }

//        setBtStatus(false);

        // Remove the notification in the title bar.
        removeNotification();
        LogUtils.d(TAG, "<<< FMRadioService.powerDown: true");
        return true;
    }

    /**
     * whether FM is power up
     * 
     * @return true, power up; false, power down.
     */
    public boolean isPowerUp() {
        LogUtils.d(TAG, "FMRadioService.isPowerUp: " + mIsPowerUp);
        return mIsPowerUp;
    }

    /**
     * whether FM is power uping. if power uping, activity should call
     * super.onBackPressed, avoid not execute power down method.
     * 
     * @return true, power up; false, power down.
     */
    public boolean isPowerUping() {
        LogUtils.d(TAG, "FMRadioService.isPowerUping: " + mIsPowerUping);
        return mIsPowerUping;
    }

    /**
     * whether FM is power down by other app.
     * 
     * @return true, power down; true.
     */
    public boolean isMakePowerDown() {
        LogUtils.d(TAG, "FMRadioService.mIsMakePowerDown: " + mIsMakePowerDown);
        return mIsMakePowerDown;
    }

    /**
     * tune to station
     * 
     * @param frequency
     *            the frequency to tune
     * @return true, success; false, fail.
     */
    public void tuneStationAsync(float frequency) {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_TUNE_FINISHED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putFloat(FM_FREQUENCY, frequency);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_TUNE_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private boolean tuneStation(float frequency) {
        LogUtils.d(TAG, ">>> FMRadioService.tune: " + frequency);
        if (mIsPowerUp) {
            setRDS(false);
            LogUtils.d(TAG, "FMRadioService.native tune start");
            boolean bRet = FMRadioNative.tune(frequency);
            LogUtils.d(TAG, "FMRadioService.native tune end");
            if (bRet) {
                setRDS(true);
                mCurrentStation = FMRadioUtils.computeStation(frequency);
                FMRadioStation.setCurrentStation(mContext, mCurrentStation);
                updateNotification();
            }
            setMute(false);
            LogUtils.d(TAG, "<<< FMRadioService.tune: " + bRet);
            return bRet;
        }
        // if not support short Antenna and earphone is not insert, not power up
        if (!isAntennaAvailable() && !SHORT_ANNTENNA_SUPPORT) {
            LogUtils.d(TAG, "<<< FMRadioService.tune: earphone is not insert and short antenna not support");
            return false;
        }
        
        // if not power up yet, should powerup first
        LogUtils.w(TAG, "FM is not powered up");
        mIsPowerUping = true;
        boolean tune = false;
        if (powerUpFM(frequency)) {
            // need switch antenna and other function
            tune = initDevice(frequency);
        }
        mIsPowerUping = false;
        LogUtils.d(TAG, "<<< FMRadioService.tune: mIsPowerup:" + tune);
        return tune;
    }

    /**
     * seek station according frequency and direction
     * 
     * @param frequency
     *            start frequency(50KHZ, 87.55; 100KHZ, 87.5)
     * @param isUp
     *            direction(true, next station; false, previous station)
     * @return the frequency after seek
     */
    public void seekStationAsync(float frequency, boolean isUp) {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SEEK_FINISHED);
        final int bundleSize = 2;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putFloat(FM_FREQUENCY, frequency);
        bundle.putBoolean(OPTION, isUp);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_SEEK_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    // need discussion
    private float seekStation(float frequency, boolean isUp) {

        LogUtils.d(TAG, ">>> FMRadioService.seek: " + frequency + " " + isUp);

        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return -1;
        }

        setRDS(false);
        mIsNativeSeeking = true;
        long startSeekTime = System.currentTimeMillis();
        LogUtils.i(TAG, "[Performance test][FMRadio] Test FMRadio Native seek time start [" + startSeekTime + "]");
        float fRet = FMRadioNative.seek(frequency, isUp);
        long endSeekTime = System.currentTimeMillis();
        LogUtils.i(TAG, "[Performance test][FMRadio] Test FMRadio Native seek time end [" + endSeekTime + "]");
        mIsNativeSeeking = false;

        // make mIsStopScanCalled false, avoid stop scan make this true,
        // when start scan, it will return null.
        mIsStopScanCalled = false;
        LogUtils.d(TAG, "<<< FMRadioService.seek: " + fRet);
        return fRet;
    }

    /**
     * scan stations
     * 
     * @return scanned stations
     */
    public void startScanAsync() {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SCAN_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FMRadioListener.MSGID_SCAN_FINISHED);
    }

    private int[] startScan() {
        LogUtils.d(TAG, ">>> FMRadioService.startScan");
        int[] iChannels = null;

        setRDS(false);
        setMute(true);
        short[] shortChannels = null;
        if (!mIsStopScanCalled) {
            mIsNativeScanning = true;
            LogUtils.d(TAG, "startScan native method:start");
            shortChannels = FMRadioNative.autoscan();
            LogUtils.d(TAG, "startScan native method:end " + Arrays.toString(shortChannels));
            mIsNativeScanning = false;
        }

        setRDS(true);
        if (mIsStopScanCalled) {
            // Received a message to power down FM, or interrupted by a phone
            // call. Do not return any stations. shortChannels = null;
            // if cancel scan, return invalid station -100
            shortChannels = new short[] { -100 };
            mIsStopScanCalled = false;
        } else {
            // We do not enable audio after activity has tuneToStation
            LogUtils.d(TAG, "startScan: scan complete, but don't enable audio yet!");
        }

        if (null != shortChannels) {
            int size = shortChannels.length;
            iChannels = new int[size];
            for (int i = 0; i < size; i++) {
                iChannels[i] = shortChannels[i];
            }
        }
        LogUtils.d(TAG, "<<< FMRadioService.startScan: " + Arrays.toString(iChannels));
        return iChannels;
    }

    /**
     * FM Radio is in scan progress or not
     * 
     * @return if in scan progress return true, otherwise return false.
     */
    public boolean isScanning() {
        return mIsScanning;
    }

    /**
     * stop scan progress
     * 
     * @return if can stop scan return ture, otherwise return false.
     */
    public boolean stopScan() {
        LogUtils.d(TAG, ">>> FMRadioService.stopScan");

        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return false;
        }

        boolean bRet = false;

        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SCAN_FINISHED);
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SEEK_FINISHED);

        if (mIsNativeScanning || mIsNativeSeeking) {
            mIsStopScanCalled = true;
            LogUtils.d(TAG, "native stop scan:start");
            bRet = FMRadioNative.stopscan();
            LogUtils.d(TAG, "native stop scan:end --" + bRet);
        }
        LogUtils.d(TAG, "<<< FMRadioService.stopScan: " + bRet);
        return bRet;
    }

    /**
     * FM Radio is in seek progress or not
     * 
     * @return if in seek progress return true, otherwise return false.
     */
    public boolean isSeeking() {
        return mIsNativeSeeking;
    }

    /**
     * set RDS
     * 
     * @param on
     *            true, enable RDS; false, disable RDS.
     * @return
     */
    public void setRDSAsync(boolean on) {
        final int bundleSize = 1;
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SET_RDS_FINISHED);
        Bundle bundle = new Bundle(bundleSize);
        bundle.putBoolean(OPTION, on);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_SET_RDS_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private int setRDS(boolean on) {
        if (!mIsPowerUp) {
            return -1;
        }
        LogUtils.d(TAG, ">>> FMRadioService.setRDS: " + on);
        int ret = -1;
        if (isRDSSupported()) {
            ret = FMRadioNative.rdsset(on);
        }

        setPS("");
        setLRText("");
        LogUtils.d(TAG, "<<< FMRadioService.setRDS: " + ret);
        return ret;
    }

    /*public int getRDS() {
        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return -1;
        }
        int event = FMRadioNative.readrds();
        LogUtils.d(TAG, "FMRadioService.readRDS: " + event);
        return event;
    }*/

    /**
     * get PS information
     * 
     * @return PS information
     */
    public String getPS() {
        LogUtils.d(TAG, "FMRadioService.getPS: " + mPSString);
        return mPSString;
    }

    /**
     * get RT information
     * 
     * @return RT information
     */
    public String getLRText() {
        LogUtils.d(TAG, "FMRadioService.getLRText: " + mLRTextString);
        return mLRTextString;
    }

    /**
     * get AF frequency
     * 
     * @return AF frequency
     */
    public void activeAFAsync() {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_ACTIVE_AF_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FMRadioListener.MSGID_ACTIVE_AF_FINISHED);
    }

    private int activeAF() {
        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return -1;
        }

        int frequency = FMRadioNative.activeAF();
        LogUtils.d(TAG, "FMRadioService.activeAF: " + frequency);
        return frequency;
    }

    /**
     * mute or unmute FM voice
     * 
     * @param mute
     *            (true, mute; false, unmute)
     * @return (true, success; false, failed)
     */
    public void setMuteAsync(boolean mute) {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SET_MUTE_FINISHED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putBoolean(OPTION, mute);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_SET_MUTE_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private int setMute(boolean mute) {
        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return -1;
        }

        LogUtils.d(TAG, ">>> FMRadioService.setMute: " + mute);
        int iRet = FMRadioNative.setmute(mute);
        LogUtils.d(TAG, "<<< FMRadioService.setMute: " + iRet);
        return iRet;
    }

    /**
     * Inquiry if RDS is support in driver
     * 
     * @return (true, support; false, not support)
     */
    public boolean isRDSSupported() {
        boolean isRDSSupported = (FMRadioNative.isRDSsupport() == 1);
        LogUtils.d(TAG, "FMRadioService.isRDSSupported: " + isRDSSupported);
        return isRDSSupported;
    }

    /**
     * inquiry earphone used or not
     * 
     * @return if use earphone return true, otherwise return false
     */
    public boolean isSpeakerUsed() {
        LogUtils.d(TAG, "FMRadioService.isEarphoneUsed: " + mIsSpeakerUsed);
        return mIsSpeakerUsed;
    }

    /**
     * initial service and current station
     * 
     * @param iCurrentStation
     *            current station frequency
     */
    public void initService(int iCurrentStation) {
        LogUtils.d(TAG, "FMRadioService.initService: " + iCurrentStation);
        mIsServiceInit = true;
        mCurrentStation = iCurrentStation;
    }

    /**
     * inquiry service is initialed or not
     * 
     * @return if initialed return true, otherwise return false
     */
    public boolean isServiceInit() {
        LogUtils.d(TAG, "FMRadioService.isServiceInit: " + mIsServiceInit);
        return mIsServiceInit;
    }

    /**
     * get FM service current station frequency
     * 
     * @return current station frequency
     */
    public int getFrequency() {
        LogUtils.d(TAG, "FMRadioService.getFrequency: " + mCurrentStation);
        return mCurrentStation;
    }

    /**
     * set FM service station frequency
     * 
     * @param station
     *            current station
     */
    public void setFrequency(int station) {
        mCurrentStation = station;
    }

    /**
     * resume FM audio
     */
    public void resumeFMAudio() {
        LogUtils.d(TAG, "FMRadioService.resumeFMAudio");
        // If not check mIsAudioFocusHeld && mIsPowerup, when scan canceled,
        // this will be resume first, then execute power down. it will cause nosise.
        if (!mUsingFMViaBTController && mIsAudioFocusHeld && mIsPowerUp) {
            enableFMAudio(true);
        }
    }

    /**
     * switch antenna
     * 
     * @param antenna
     *            antenna (0, long antenna, 1 short antenna)
     * @return (0, success; 1 failed; 2 not support)
     */
    public void switchAntennaAsync(int antenna) {
        final int bundleSize = 1;
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SWITCH_ANNTENNA);

        Bundle bundle = new Bundle(bundleSize);
        bundle.putInt(FMRadioListener.SWITCH_ANNTENNA_VALUE, antenna);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_SWITCH_ANNTENNA);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    /**
     * need native support whether antenna support interface. 
     * @param antenna
     *            antenna (0, long antenna, 1 short antenna)
     * @return (0, success; 1 failed; 2 not support)
     * 
     */
    private int switchAntenna(int antenna) {
        LogUtils.d(TAG, ">>> FMRadioService.switchAntenna:" + antenna);
        /*if (!mIsPowerUp) {
            // If not even powered up, just return and do nothing
            LogUtils.w(TAG, "ACTION_HEADSET_PLUG: FM is not powerup!!");
            return 0;
        }*/

        // if fm not powerup, switchAntenna will flag whether has earphone
        int ret = FMRadioNative.switchAntenna(antenna);
        LogUtils.d(TAG, "<<< FMRadioService.switchAntenna: " + ret);
        return ret;
    }

    /**
     * Read cap array method not need async
     */
    public int getCapArray() {
        LogUtils.d(TAG, "FMRadioService.readCapArray");
        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return -1;
        }
        return FMRadioNative.readCapArray();
    }

    /**
     * Get rssi not need async
     */
    public int getRssi() {
        LogUtils.d(TAG, "FMRadioService.readRssi");
        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return -1;
        }
        return FMRadioNative.readRssi();
    }

    public boolean getAudioChannelSetting() {
        LogUtils.d(TAG, "FMRadioService.getStereoMono");
        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return false;
        }
        return FMRadioNative.stereoMono();
    }

    /**
     * Force set to stero/mono mode
     * 
     * @param isMono
     *            (true, mono; false, stereo)
     * @return (true, success; false, failed)
     */
    public void setAudioChannelAsync(boolean isMono) {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SET_CHANNEL_FINISHED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putBoolean(OPTION, isMono);
        // Message msg = mFmServiceHandler.obtainMessage(SET_CHANNEL_MSG);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_SET_CHANNEL_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private boolean setAudioChannel(boolean isMono) {
        LogUtils.d(TAG, "FMRadioService.setStereoMono: isMono=" + isMono);
        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return false;
        }
        return FMRadioNative.setStereoMono(isMono);
    }

    /**
     * read rds bler not need async
     */
    public int getRdsBler() {
        LogUtils.d(TAG, "FMRadioService.readRdsBler");
        if (!mIsPowerUp) {
            LogUtils.w(TAG, "FM is not powered up");
            return -1;
        }
        return FMRadioNative.readRdsBler();
    }

    /**
     * Get hardware version not need async
     */
    public int[] getHardwareVersion() {
        return FMRadioNative.getHardwareVersion();
    }
    
    
    private boolean chipSupportOverBt() {
        int result = android.os.SystemProperties.getInt(BT_PROPERTIES, 0);
        LogUtils.e(TAG, "bt.fmoverbt: " + result);
        return (1 == result);
    }

    /**
     * start recording
     */
    public void startRecordingAsync() {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTRECORDING_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FMRadioListener.MSGID_STARTRECORDING_FINISHED);
    }

    private void startRecording() {
        LogUtils.d(TAG, ">>> startRecording");
        sRecordingSdcard = FMRadioUtils.getDefaultStoragePath();
        LogUtils.d(TAG, "default sd card file path: " + sRecordingSdcard);
        if (sRecordingSdcard == null || sRecordingSdcard.isEmpty()) {
            LogUtils.d(TAG, "startRecording: may be no sdcard");
            onRecorderError(FMRecorder.ERROR_SDCARD_NOT_PRESENT);
            return;
        }

        if (mFMRecorder == null) {
            mFMRecorder = new FMRecorder();
            mFMRecorder.registerRecorderStateListener(FMRadioService.this);
        }

        if (isSdcardReady(sRecordingSdcard)) {
            mFMRecorder.startRecording(getApplicationContext());
        } else {
            LogUtils.d(TAG, "Cannot record because sdcard is not ready!!");
            onRecorderError(FMRecorder.ERROR_SDCARD_NOT_PRESENT);
        }
        LogUtils.d(TAG, "<<< startRecording");
    }

    private boolean isSdcardReady(String sdcardPath) {
        LogUtils.d(TAG, ">>> isSdcardReady: sdcardPath is " + sdcardPath + ", mSdcardStateMap is " + mSdcardStateMap);
        if (!mSdcardStateMap.isEmpty()) {
            if (mSdcardStateMap.get(sdcardPath) != null && mSdcardStateMap.get(sdcardPath) == false) {
                LogUtils.d(TAG, "<<< isSdcardReady: return false");
                return false;
            }
        }
        LogUtils.d(TAG, "<<< isSdcardReady: mSdcardStateMap:" + mSdcardStateMap);
        return true;
    }

    /**
     * stop recording
     */
    public void stopRecordingAsync() {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPRECORDING_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FMRadioListener.MSGID_STOPRECORDING_FINISHED);
    }

    private boolean stopRecording() {
        LogUtils.d(TAG, ">>> stopRecording");
        if (mFMRecorder == null) {
            LogUtils.e(TAG, "stopRecording called without a valid recorder!!");
            return false;
        }
        synchronized (mStopRecordingLock) {
            mFMRecorder.stopRecording();
            LogUtils.d(TAG, "<<< stopRecording");
        }
        return true;
    }

    /**
     * start play recording file
     */

    public void startPlaybackAsync() {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTPLAYBACK_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FMRadioListener.MSGID_STARTPLAYBACK_FINISHED);
    }

    private boolean startPlayback() {
        LogUtils.d(TAG, ">>> startPlayback");

        if (!requestAudioFocus()) {
            LogUtils.d(TAG, "can't get audio focus when play recording file");
            return false;
        }

        if (mFMRecorder == null) {
            LogUtils.e(TAG, "FMRecorder is null !!");
            return false;
        }

        // set pre stop before start playback
        mAudioManager.setParameters("AudioFmPreStop=1");
        enableFMAudio(false);

        mFMRecorder.startPlayback();

        LogUtils.d(TAG, "<<< startPlayback");

        return true;
    }

    /**
     * stop play recording file
     */
    public void stopPlaybackAsync() {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPPLAYBACK_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FMRadioListener.MSGID_STOPPLAYBACK_FINISHED);
    }

    private void stopPlayback() {
        LogUtils.d(TAG, ">>> stopPlayback");

        if (mFMRecorder != null) {
            mFMRecorder.stopPlayback();
//            enableFMAudio(true);
            checkAfterPlayback();
        }

        LogUtils.d(TAG, "<<< stopPlayback");
    }

    /**
     * save recording file according name or discard recording file if name is
     * null
     * 
     * @param newName
     *            new recording file name
     */
    public void saveRecordingAsync(String newName) {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_SAVERECORDING_FINISHED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putString(RECODING_FILE_NAME, newName);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_SAVERECORDING_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private void saveRecording(String newName) {
        LogUtils.d(TAG, ">>> saveRecording");
        if (mFMRecorder != null) {
            if (newName != null) {
                mFMRecorder.saveRecording(FMRadioService.this, newName);
                LogUtils.d(TAG, "<<< saveRecording");
                return;
            }
            mFMRecorder.discardRecording();
        }
        LogUtils.d(TAG, "<<< saveRecording");
    }

    /**
     * get record time
     * 
     * @return record time
     */
    public long getRecordTime() {
        if (mFMRecorder != null) {
            return mFMRecorder.recordTime();
        }
        LogUtils.e(TAG, "FMRecorder is null !!");
        return 0;
    }

    /**
     * set recording mode
     * 
     * @param isRecording
     *            true, enter recoding mode; false, exit recording mode
     */

    public void setRecordingModeAsync(boolean isRecording) {
        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_RECORD_MODE_CHANED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putBoolean(OPTION, isRecording);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_RECORD_MODE_CHANED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private void setRecordingMode(boolean isRecording) {
        LogUtils.d(TAG, ">>> setRecordingMode: isRecording=" + isRecording);
        mIsInRecordingMode = isRecording;
        if (mFMRecorder != null) {
            if (!isRecording) {
                if (mFMRecorder.getState() != FMRecorder.STATE_IDLE) {
                    mFMRecorder.stopRecording();
                    mFMRecorder.stopPlayback();
                }
                resumeFMAudio();
                LogUtils.d(TAG, "<<< setRecordingMode");
                return;
            }
            // reset recorder to unused status
            mFMRecorder.resetRecorder();
        }
        LogUtils.d(TAG, "<<< setRecordingMode");
    }

    /**
     * get current recording mode
     * 
     * @return if in recording mode return true, otherwise return false;
     */
    public boolean getRecordingMode() {
        return mIsInRecordingMode;
    }

    /**
     * get record state
     * 
     * @return record state
     */
    public int getRecorderState() {
        if (null != mFMRecorder) {
            return mFMRecorder.getState();
        }
        return FMRecorder.STATE_INVALID;
    }

    /**
     * get recording file name
     * 
     * @return recording file name
     */
    public String getRecordingName() {
        if (null != mFMRecorder) {
            return mFMRecorder.getRecordingName();
        }
        return null;
    }

    /**
     * get current recording file name with full path
     * @return
     */
    public String getRecordingNameWithPath() {
        if (null != mFMRecorder) {
            return mFMRecorder.getRecordingNameWithPath();
        }
        return null;
    }

    /**
     * whether FM over BT
     * 
     * @return if FM is over BT return true, otherwise return false
     */
    public boolean isFmViaBt() {
        return mUsingFMViaBTController;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        LogUtils.d(TAG, ">>> FMRadioService.onCreate");
        LogUtils.d(TAG, "short antenna support: " + SHORT_ANNTENNA_SUPPORT);
        mContext = getApplicationContext();
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mActivityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        PowerManager powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        mWakeLock.setReferenceCounted(false);
        mFMPlayer = new MediaPlayer();
        mFMPlayer.setWakeMode(FMRadioService.this, PowerManager.PARTIAL_WAKE_LOCK);
        mFMPlayer.setOnErrorListener(mPlayerErrorListener);
        sRecordingSdcard = FMRadioUtils.getDefaultStoragePath();
        try {
            mFMPlayer.setDataSource("MEDIATEK://MEDIAPLAYER_PLAYERTYPE_FM");
        } catch (IOException ex) {
            // notify the user why the file couldn't be opened
            LogUtils.e(TAG, "setDataSource: " + ex);
            return;
        } catch (IllegalArgumentException ex) {
            // notify the user why the file couldn't be opened
            LogUtils.e(TAG, "setDataSource: " + ex);
            return;
        } catch (SecurityException ex) {
            LogUtils.e(TAG, "setDataSource: " + ex);
            return;
        } catch (IllegalStateException ex) {
            LogUtils.e(TAG, "setDataSource: " + ex);
            return;
        }
        mFMPlayer.setAudioStreamType(AudioManager.STREAM_FM);

        HandlerThread handlerThread = new HandlerThread("FmRadioServiceThread");
        handlerThread.start();
        mFmServiceHandler = new FmRadioServiceHandler(handlerThread.getLooper());

        // Register broadcast receiver.
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_TOFMSERVICE_POWERDOWN);
        filter.addAction(SOUND_POWER_DOWN_MSG);
        filter.addAction(Intent.ACTION_SHUTDOWN);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);

        if (FeatureOption.MTK_MT519X_FM_SUPPORT) {
            filter.addAction(ACTION_FROMATVSERVICE_POWERUP);
        }

        /*if (FeatureOption.MTK_BT_FM_OVER_BT_VIA_CONTROLLER) {
            filter.addAction(BluetoothA2dpService.ACTION_FM_OVER_BT_CONTROLLER);
            filter.addAction(BluetoothA2dpService.ACTION_FM_OVER_BT_HOST);
            filter.addAction(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED);
        }*/
        filter.addAction(Intent.ACTION_HEADSET_PLUG);
        mBroadcastReceiver = new FMServiceBroadcastReceiver();
        LogUtils.i(TAG, "Register broadcast receiver.");
        registerReceiver(mBroadcastReceiver, filter);

        // register FM recorder related listener/broadcast receiver
        if (FeatureOption.MTK_FM_RECORDING_SUPPORT) {
            registerSDListener();
        }
        // open device when service create.
        openDevice();
//        int iCurrentStation = FMRadioStation.getCurrentStation(mContext);
//        initService(iCurrentStation);
        /*
         * set speaker to default status, avoid setting->clear data.
         * ALPS00445776
         */
        setSpeakerPhoneOn(mIsSpeakerUsed);
        LogUtils.d(TAG, "<<< FMRadioService.onCreate");
    }

    @Override
    public void onDestroy() {
        LogUtils.d(TAG, ">>> FMRadioService.onDestroy");
        // stop rds first, avoid blocking other native method
        if (isRDSSupported()) {
            LogUtils.d(TAG, "RDS is supported. Stop the RDS thread.");
            stopRDSThread();
        }
        // Unregister the broadcast receiver.
        if (null != mBroadcastReceiver) {
            LogUtils.i(TAG, "Unregister broadcast receiver.");
            unregisterReceiver(mBroadcastReceiver);
            mBroadcastReceiver = null;
        }
        abandonAudioFocus();
        exitFM();
        // release FMRecorder & unregister SD event receiver
        if (FeatureOption.MTK_FM_RECORDING_SUPPORT) {
            if (null != mFMRecorder) {
                mFMRecorder = null;
            }
            if (null != mSDListener) {
                unregisterReceiver(mSDListener);
            }
        }
        super.onDestroy();
    }

    /**
     * exit FM Radio application
     */
    private void exitFM() {
        LogUtils.d(TAG, "service.exitFM start");
        mIsAudioFocusHeld = false;
        // Stop FM recorder if it is working
        if (FeatureOption.MTK_FM_RECORDING_SUPPORT && null != mFMRecorder) {
            synchronized (mStopRecordingLock) {
                int fmState = mFMRecorder.getState();
                if (FMRecorder.STATE_PLAYBACK == fmState) {
                    mFMRecorder.stopPlayback();
                    LogUtils.d(TAG, "Stop playback FMRecorder.");
                } else if (FMRecorder.STATE_RECORDING == fmState) {
                    mFMRecorder.discardRecording();
                    LogUtils.d(TAG, "Discard Recording.");
                }
            }
        }

        // When exit, we set the audio path back to earphone.
        if (mIsNativeScanning || mIsNativeSeeking) {
            stopScan();
        }

        /*
         * if exit Fm, should remove all message first.
         */
        mFmServiceHandler.removeCallbacksAndMessages(null);

        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_FM_EXIT);
        mFmServiceHandler.sendEmptyMessage(FMRadioListener.MSGID_FM_EXIT);

        LogUtils.d(TAG, "service.exitFM end");
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Change the notification string.
        if (mIsPowerUp) {
            showNotification();
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        LogUtils.d(TAG, ">>> FMRadioService.onStartCommand intent: " + intent + " startId: " + startId);
        int iRet = super.onStartCommand(intent, flags, startId);
        LogUtils.d(TAG, "<<< FMRadioService.onStartCommand: " + iRet);
        return START_NOT_STICKY;
    }

    /**
     * start RDS thread to update RDS information
     */
    private void startRDSThread() {
        LogUtils.d(TAG, ">>> FMRadioService.startRDSThread");
        mIsRDSThreadExit = false;
        if (null != mRDSThread) {
            return;
        }
        mRDSThread = new Thread() {
            public void run() {
                LogUtils.d(TAG, ">>> RDS Thread run()");
                while (true) {
                    if (mIsRDSThreadExit) {
                        break;
                    }
                    
                    int iRDSEvents = FMRadioNative.readrds();
                    if (iRDSEvents != 0) {
                        LogUtils.d(TAG, "FMRadioNative.readrds events: " + iRDSEvents);
                    }

                    if (RDS_EVENT_PROGRAMNAME == (RDS_EVENT_PROGRAMNAME & iRDSEvents)) {
                        LogUtils.d(TAG, "RDS_EVENT_PROGRAMNAME");
                        byte[] bytePS = FMRadioNative.getPS();
                        if (null != bytePS) {
                            setPS(new String(bytePS).trim());
                        }
                    }

                    if (RDS_EVENT_LAST_RADIOTEXT == (RDS_EVENT_LAST_RADIOTEXT & iRDSEvents)) {
                        LogUtils.d(TAG, "RDS_EVENT_LAST_RADIOTEXT");
                        byte[] byteLRText = FMRadioNative.getLRText();
                        if (null != byteLRText) {
                            setLRText(new String(byteLRText).trim());
                        }
                    }

                    if (RDS_EVENT_AF == (RDS_EVENT_AF & iRDSEvents)) {
                        LogUtils.d(TAG, "RDS_EVENT_AF");
                        /*
                         * add for rds AF
                         * ALPS00594894
                         */
                        if (mIsScanning || mIsSeeking) {
                            LogUtils.d(TAG, "RDSThread. seek or scan going, no need to tune here");
                        } else if (!mIsPowerUp) {
                            LogUtils.d(TAG, "RDSThread. fm is power down, do nothing.");
                        } else {
                            int iFreq = FMRadioNative.activeAF();
                            if (FMRadioUtils.isValidStation(iFreq)) {
                                // if the new frequency is not equal to current frequency.
                                if (mCurrentStation == iFreq) {
                                    LogUtils.w(TAG, "RDSThread. the new frequency is the same as current.");
                                } else {
                                    setPS("");
                                    setLRText("");
                                    if (!mIsScanning && !mIsSeeking) {
                                        LogUtils.d(TAG, "RDSThread. seek or scan not going, need to tune here");
                                        tuneStationAsync(FMRadioUtils.computeFrequency(iFreq));
                                    }
                                }
                            }
                        }
                    }
                    // Do not handle other events.
                    // Sleep 500ms to reduce inquiry frequency
                    try {
                        final int hundredMillisecond = 500;
                        Thread.sleep(hundredMillisecond);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                LogUtils.d(TAG, "<<< RDS Thread run()");
            }
        };

        LogUtils.d(TAG, "Start RDS Thread.");
        mRDSThread.start();
        LogUtils.d(TAG, "<<< FMRadioService.startRDSThread");
    }

    /**
     * stop RDS thread to stop listen station RDS change
     */
    private void stopRDSThread() {
        LogUtils.d(TAG, ">>> FMRadioService.stopRDSThread");
        if (null != mRDSThread) {
            // Must call closedev after stopRDSThread.
            mIsRDSThreadExit = true;
            mRDSThread = null;
        }
        LogUtils.d(TAG, "<<< FMRadioService.stopRDSThread");
    }

    /**
     * set PS information
     * 
     * @param ps
     *            update ps information
     */
    private void setPS(String ps) {
        LogUtils.d(TAG, "FMRadioService.setPS: " + ps + " ,current: " + mPSString);
        if (0 != mPSString.compareTo(ps)) {
            mPSString = ps;
            Bundle bundle = new Bundle(3);
            bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.LISTEN_PS_CHANGED);
            bundle.putString(FMRadioListener.KEY_PS_INFO, mPSString);
            bundle.putString(FMRadioListener.KEY_RT_INFO, mLRTextString);
            notifyActivityStateChanged(bundle);
        } // else New PS is the same as current
    }

    /**
     * set RT information
     * 
     * @param lrtText
     *            RT information
     */
    private void setLRText(String lrtText) {
        LogUtils.d(TAG, "FMRadioService.setLRText: " + lrtText + " ,current: " + mLRTextString);
        if (0 != mLRTextString.compareTo(lrtText)) {
            mLRTextString = lrtText;
            Bundle bundle = new Bundle(3);
            bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.LISTEN_RT_CHANGED);
            bundle.putString(FMRadioListener.KEY_PS_INFO, mPSString);
            bundle.putString(FMRadioListener.KEY_RT_INFO, mLRTextString);
            notifyActivityStateChanged(bundle);
        } // else New RT is the same as current
    }

    /**
     * Inquiry if fm stereo mono(true, stereo; false mono)
     * 
     * @return (true, stereo; false, mono)
     */
    public boolean getStereoMono() {
        LogUtils.d(TAG, "FMRadioService.getStereoMono");
        return FMRadioNative.stereoMono();
    }

    /**
     * Force set to stero/mono mode
     * 
     * @param isMono
     *            (true, mono; false, stereo)
     * @return (true, success; false, failed)
     */
    public boolean setStereoMono(boolean isMono) {
        LogUtils.d(TAG, "FMRadioService.setStereoMono: isMono=" + isMono);
        return FMRadioNative.setStereoMono(isMono);
    }
    
    /**
     * set RSSI, desense RSSI, mute gain soft
     * @param index flag which will execute
     * (0:rssi threshold,1:desense rssi threshold,2: SGM threshold)
     * @param value send to native
     * @return execute ok or not
     */
    public boolean setEmth(int index, int value) {
        LogUtils.d(TAG, ">>> FMRadioService.setEmth: index=" + index + ",value=" + value);
        boolean isOk = FMRadioNative.emsetth(index, value);
        LogUtils.d(TAG, "<<< FMRadioService.setEmth: isOk=" + isOk);
        return isOk;
    }
    
    /**
     * send variables to native, and get some variables return.
     * @param val send to native
     * @return get value from native
     */
    public short[] emcmd(short[] val) {
        LogUtils.d(TAG, ">>FMRadioService.emcmd: val=" + val);
        short[] shortCmds = null;
        shortCmds = FMRadioNative.emcmd(val);
        LogUtils.d(TAG, "<<FMRadioService.emcmd:" + shortCmds);
        return shortCmds;
    }

    /**
     * open or close FM Radio audio
     * 
     * @param enable
     *            true, open FM audio; false, close FM audio;
     */
    private void enableFMAudio(boolean enable) {
        LogUtils.d(TAG, ">>> FMRadioService.enableFMAudio: " + enable);
        if ((mFMPlayer == null) || !mIsPowerUp) {
            LogUtils.w(TAG, "mFMPlayer is null in Service.enableFMAudio");
            return;
        }

        try {
            if (!enable) {
                if (!mFMPlayer.isPlaying()) {
                    LogUtils.d(TAG, "warning: FM audio is already disabled.");
                    return;
                }

                mFMPlayer.stop();
                LogUtils.d(TAG, "stop FM audio.");
                return;
            }

            if (mFMPlayer.isPlaying()) {
                LogUtils.d(TAG, "warning: FM audio is already enabled.");
                return;
            }
        } catch (IllegalStateException e) {
            LogUtils.e(TAG, "Exception: Cannot call MediaPlayer isPlaying.", e);
        }

        try {
            mFMPlayer.prepare();
            mFMPlayer.start();
        } catch (IOException e) {
            LogUtils.e(TAG, "Exception: Cannot call MediaPlayer prepare.", e);
        } catch (IllegalStateException e) {
            LogUtils.e(TAG, "Exception: Cannot call MediaPlayer prepare.", e);
        }

        LogUtils.d(TAG, "Start FM audio.");
        LogUtils.d(TAG, "<<< FMRadioService.enableFMAudio");
    }

    /**
     * show notification
     */
    private void showNotification() {
        LogUtils.d(TAG, "FMRadioService.showNotification");
        Intent notificationIntent = new Intent();
        notificationIntent.setClassName(getPackageName(), FMRadioActivity.class.getName());
        PendingIntent pendingIntent = PendingIntent.getActivity(getApplicationContext(), 0, notificationIntent, 0);
        Notification notification = new Notification(R.drawable.fm_title_icon, null, System.currentTimeMillis());
        notification.flags |= Notification.FLAG_ONGOING_EVENT | Notification.FLAG_NO_CLEAR;
        String text = FMRadioUtils.formatStation(mCurrentStation) + " MHz";
        notification.setLatestEventInfo(getApplicationContext(), getResources().getString(R.string.app_name), text,
                pendingIntent);
        LogUtils.d(TAG, "Add notification to the title bar.");
        startForeground(NOTIFICATION_ID, notification);
    }

    /**
     * remove notification
     */
    private void removeNotification() {
        LogUtils.d(TAG, "FMRadioService.removeNotification");
        stopForeground(true);
    }

    /**
     * update notification
     */
    private void updateNotification() {
        LogUtils.d(TAG, "FMRadioService.updateNotification");
        if (mIsPowerUp) {
            showNotification();
        }
    }

    /**
     * register sd card listener
     */
    private void registerSDListener() {
        LogUtils.v(TAG, "registerSDListener >>> ");
        if (mSDListener == null) {
            mSDListener = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    // ALPS01226747 if eject record sdcard, should set this false to not record.
                    updateSdcardStateMap(intent);

                    if (mFMRecorder == null) {
                        LogUtils.w(TAG, "SD receiver: FMRecorder is not present!!");
                        return;
                    }

                    String action = intent.getAction();

                    // porting 245343
                    /*if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                        LogUtils.v(TAG, "MEDIA_MOUNTED");
                        mFMRecorder.onSDInserted();
                        return;
                    }*/

                    if (Intent.ACTION_MEDIA_EJECT.equals(action) || Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                        // If not unmount recording sd card, do nothing;
                        if (isRecordingCardUnmount(intent)) {
                            LogUtils.v(TAG, "MEDIA_EJECT");
                            if (mFMRecorder.getState() == FMRecorder.STATE_RECORDING) {
                                LogUtils.d(TAG, "old state is recording");
                                onRecorderError(FMRecorder.ERROR_SDCARD_NOT_PRESENT);
                                mFMRecorder.discardRecording();
                            } else {
                                Bundle bundle = new Bundle(2);
                                bundle.putInt(FMRadioListener.CALLBACK_FLAG,
                                        FMRadioListener.LISTEN_RECORDSTATE_CHANGED);
                                bundle.putInt(FMRadioListener.KEY_RECORDING_STATE,
                                        FMRecorder.STATE_IDLE);
                                notifyActivityStateChanged(bundle);
                            }
                        }
                        return;
                    }
                }
            };
        }
        IntentFilter filter = new IntentFilter();
        filter.addDataScheme("file");
        filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        filter.addAction(Intent.ACTION_MEDIA_EJECT);
        registerReceiver(mSDListener, filter);
        LogUtils.v(TAG, "registerSDListener <<< ");
    }

    // ALPS01226747 update the mSdcardStateMap for record
    private void updateSdcardStateMap(Intent intent) {
        String action = intent.getAction();
        String sdcardPath = null;
        Uri mountPointUri = intent.getData();
        if (mountPointUri != null) {
            sdcardPath = mountPointUri.getPath();
            if (sdcardPath != null) {
                if (Intent.ACTION_MEDIA_EJECT.equals(action)) {
                    LogUtils.d(TAG, "updateSdcardStateMap: ENJECT " + sdcardPath);
                    mSdcardStateMap.put(sdcardPath, false);
                } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                    LogUtils.d(TAG, "updateSdcardStateMap: UNMOUNTED " + sdcardPath);
                    mSdcardStateMap.put(sdcardPath, false);
                } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                    LogUtils.d(TAG, "updateSdcardStateMap: MOUNTED " + sdcardPath);
                    mSdcardStateMap.put(sdcardPath, true);
                }
            }
        }
    }

    /**
     * notify FM recorder state
     * 
     * @param state
     *            current FM recorder state
     */
    public void onRecorderStateChanged(int state) {
        LogUtils.d(TAG, "onRecorderStateChanged: " + state);
        mRecordState = state;
        Bundle bundle = new Bundle(2);
        bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.LISTEN_RECORDSTATE_CHANGED);
        bundle.putInt(FMRadioListener.KEY_RECORDING_STATE, state);
        notifyActivityStateChanged(bundle);
    }

    /**
     * notify FM recorder error message
     * 
     * @param error
     *            error type
     */
    public void onRecorderError(int error) {
        LogUtils.d(TAG, "onRecorderError: " + error);
        // if media server die, will not enable FM audio, and convert to
        // ERROR_PLAYER_INATERNAL, call back
        // to activity showing toast.
        mErrorType = (MediaPlayer.MEDIA_ERROR_SERVER_DIED == error) ? FMRecorder.ERROR_PLAYER_INTERNAL : error;

        Bundle bundle = new Bundle(2);
        bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.LISTEN_RECORDERROR);
        bundle.putInt(FMRadioListener.KEY_RECORDING_ERROR_TYPE, mErrorType);
        notifyActivityStateChanged(bundle);

        // if media server die, should not enable fm, otherwise je will occur.
        if (FMRecorder.ERROR_PLAYER_INTERNAL == error) {
            resumeFMAudio();
        }
    }

    /**
     * notify play FM record file complete
     */
    public void onPlayRecordFileComplete() {
        LogUtils.d(TAG, "service.onPlayRecordFileComplete");
        checkAfterPlayback();
    }

    /**
     * Check and go next(play or exit or show dialog) after recorder file play back finish.
     * Four cases:
     * 1. With headset + forground -> play FM
     * 2. With headset + background -> play FM
     * 3. Without headset + forground -> show no anntenna dialog if not support SHORT_ANNTENNA
     * 4. Without headset + background-> exit FM if not in STATE_PLAYBACK
     */
    private void checkAfterPlayback() {
        if (isHeadSetIn()) {//with headset
            LogUtils.d(TAG, "checkAfterPlayback:eaphone is in,need resume fm");
            if (mIsPowerUp) {
                resumeFMAudio();
            } else {
                powerUpAsync(FMRadioUtils.computeFrequency(mCurrentStation));
            }
        } else {// without headset, should check background or forground
            if (isActivityForeGround()) {
                // if not support short anntenna, show dialog when play back finished
                if (!SHORT_ANNTENNA_SUPPORT) {
                    LogUtils.d(TAG, "checkAfterPlayback:earphone is out,foreground need show dialog");
                    switchAntennaAsync(mValueHeadSetPlug);
                } else {
                    //need to check here
                }
            } else if (mRecordState != FMRecorder.STATE_PLAYBACK) {
                    LogUtils.d(TAG, "checkAfterPlayback:earphone is out,background need to exit fm");
                    mFmServiceHandler.removeCallbacksAndMessages(null);
                    exitFM();
                    stopSelf();
            }
        }
    }

    /**
     * Check the headset is plug in or plug out
     * @return true for plug in; false for plug out
     */
    private boolean isHeadSetIn() {
        return (0 == mValueHeadSetPlug);
    }

    /**
     * stop FM
     */
    private void stopFMFocusLoss(int focusState) {
        mIsAudioFocusHeld = false;
        if (mIsNativeScanning || mIsNativeSeeking) {
            // make stop scan from activity call to service.
            // notifyActivityStateChanged(FMRadioListener.LISTEN_SCAN_CANCELED);
            stopScan();
            LogUtils.d(TAG, "need to stop FM, so stop scan channel.");
        }

        // using handler thread to update audio focus state
        updateAudioFocusAync(focusState);
        LogUtils.d(TAG, "need to stop FM, so powerdown FM.");
        
    }

    /**
     * handle FM Player error
     */
    private final MediaPlayer.OnErrorListener mPlayerErrorListener = new MediaPlayer.OnErrorListener() {
        /**
         * handle error message
         * 
         * @param mp
         *            occurred error media player
         * @param what
         *            error message
         * @param extra
         *            error message extra
         * @return handle error message or not
         */
        public boolean onError(MediaPlayer mp, int what, int extra) {

            if (MediaPlayer.MEDIA_ERROR_SERVER_DIED == what) {
                LogUtils.d(TAG, "onError: MEDIA_SERVER_DIED");
                if (null != mFMPlayer) {
                    mFMPlayer.release();
                    mFMPlayer = null;
                }
                mFMPlayer = new MediaPlayer();
                mFMPlayer.setWakeMode(FMRadioService.this, PowerManager.PARTIAL_WAKE_LOCK);
                mFMPlayer.setOnErrorListener(mPlayerErrorListener);
                try {
                    mFMPlayer.setDataSource("MEDIATEK://MEDIAPLAYER_PLAYERTYPE_FM");
                    mFMPlayer.setAudioStreamType(AudioManager.STREAM_FM);
                    if (mIsPowerUp) {
                        // set speaker mode according to AP
                        setSpeakerPhoneOn(mIsSpeakerUsed);
                        mFMPlayer.prepare();
                        mFMPlayer.start();
                    }
                } catch (IOException ex) {
                    LogUtils.e(TAG, "setDataSource: " + ex);
                    return false;
                } catch (IllegalArgumentException ex) {
                    LogUtils.e(TAG, "setDataSource: " + ex);
                    return false;
                } catch (IllegalStateException ex) {
                    LogUtils.e(TAG, "setDataSource: " + ex);
                    return false;
                }
            }

            return true;
        }
    };

    /**
     * request audio focus
     * 
     * @return true, success; false, fail;
     */
    public boolean requestAudioFocus() {
        if (mIsAudioFocusHeld) {
            return true;
        }

        int audioFocus = mAudioManager.requestAudioFocus(mAudioFocusChangeListener, AudioManager.STREAM_FM,
                AudioManager.AUDIOFOCUS_GAIN);
        mIsAudioFocusHeld = (AudioManager.AUDIOFOCUS_REQUEST_GRANTED == audioFocus);
        return mIsAudioFocusHeld;
    }

    /**
     * abandon audio focus
     */
    public void abandonAudioFocus() {
        mAudioManager.abandonAudioFocus(mAudioFocusChangeListener);
        mIsAudioFocusHeld = false;
    }

    /**
     * use to interact with other voice related app
     */
    private final OnAudioFocusChangeListener mAudioFocusChangeListener = new OnAudioFocusChangeListener() {
        /**
         * handle audio focus change
         * ensure message FIFO
         * @param focusChange
         *            audio focus change state
         */
        public void onAudioFocusChange(int focusChange) {
            LogUtils.d(TAG, "onAudioFocusChange: " + focusChange);
            switch (focusChange) {
            case AudioManager.AUDIOFOCUS_LOSS:
                synchronized (this) {
                    LogUtils.d(TAG, "AudioFocus: received AUDIOFOCUS_LOSS");
                    /*
                     * ALPS00727310, audio driver work around, this method
                     * will make audio mute first.
                     */
                    mAudioManager.setParameters("AudioFmPreStop=1");
                    LogUtils.d(TAG, "onAudioFocusChange.setParameters end");
                    exitFM();//ALPS01265351
                    stopSelf();
                }
                break;

            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                synchronized (this) {
                    // ALPS01288327 Prestop for FileManager play music while FM playing
                    mAudioManager.setParameters("AudioFmPreStop=1");
                    LogUtils.d(TAG, "AudioFocus: received AUDIOFOCUS_LOSS_TRANSIENT");
                    stopFMFocusLoss(AudioManager.AUDIOFOCUS_LOSS_TRANSIENT);
                }
                break;

            case AudioManager.AUDIOFOCUS_GAIN:
                synchronized (this) {
                    LogUtils.d(TAG, "AudioFocus: received AUDIOFOCUS_GAIN");
                    updateAudioFocusAync(AudioManager.AUDIOFOCUS_GAIN);
                }
                break;

            default:
                LogUtils.d(TAG, "AudioFocus: Audio focus change, but not need handle");
                break;
            }
        }
    };

    /**
     * audio focus changed, will send message to handler thread.
     * synchronized to ensure one message can go in this method.
     * @param focusState AudioManager state
     */
    private synchronized void updateAudioFocusAync(int focusState) {
        LogUtils.d(TAG, "updateAudioFocusAync: focusState = " + focusState);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putInt(FMRadioListener.KEY_AUDIOFOCUS_CHANGED, focusState);
        Message msg = mFmServiceHandler.obtainMessage(FMRadioListener.MSGID_AUDIOFOCUS_CHANGED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }
    
    /**
     * audio focus changed, update FM focus state.
     * @param focusState AudioManager state
     */
    private void updateAudioFocus(int focusState) { 
        LogUtils.d(TAG, "FMRadioService.updateAudioFocus");
        switch (focusState) {
            case AudioManager.AUDIOFOCUS_LOSS :
                mPausedByTransientLossOfFocus = false;
                LogUtils.d(TAG, "AUDIOFOCUS_LOSS: mPausedByTransientLossOfFocus:" + mPausedByTransientLossOfFocus);
                //ALPS00818455, play back audio will output with music audio
                //May be affect other recorder app, but the flow can not be execute earlier,
                //It should ensure execute after start/stop record.
                if (FeatureOption.MTK_FM_RECORDING_SUPPORT && mFMRecorder != null) {
                    int fmState = mFMRecorder.getState();
                    LogUtils.d(TAG, "stopFMFocusLoss.recorder state=" + fmState);
                    // only handle recorder state, not handle playback state
                    if (fmState == FMRecorder.STATE_RECORDING) {
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTRECORDING_FINISHED);
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPRECORDING_FINISHED);
                        stopRecording();
                    }
                    /*if (fmState == FMRecorder.STATE_PLAYBACK) {
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTPLAYBACK_FINISHED);
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPPLAYBACK_FINISHED);
                        stopPlayback();
                    }*/
                }
                handlePowernDown();
                break;

            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT :
                if (mIsPowerUp) {
                    mPausedByTransientLossOfFocus = true;
                }
                LogUtils.d(TAG, "AUDIOFOCUS_LOSS_TRANSIENT: mPausedByTransientLossOfFocus:" + mPausedByTransientLossOfFocus);
                //ALPS00818455, play back audio will output with music audio
                //May be affect other recorder app, but the flow can not be execute earlier,
                //It should ensure execute after start/stop record.
                if (FeatureOption.MTK_FM_RECORDING_SUPPORT && mFMRecorder != null) {
                    int fmState = mFMRecorder.getState();
                    LogUtils.d(TAG, "stopFMFocusLoss.recorder state=" + fmState);
                    if (fmState == FMRecorder.STATE_RECORDING) {
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTRECORDING_FINISHED);
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPRECORDING_FINISHED);
                        stopRecording();
                    }
                    if (fmState == FMRecorder.STATE_PLAYBACK) {
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTPLAYBACK_FINISHED);
                        mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPPLAYBACK_FINISHED);
                        stopPlayback();
                    }
                }
                handlePowernDown();
                break;

            case AudioManager.AUDIOFOCUS_GAIN :
                LogUtils.d(TAG, "AUDIOFOCUS_GAIN: mPausedByTransientLossOfFocus:" + mPausedByTransientLossOfFocus);
                if (!mIsPowerUp && mPausedByTransientLossOfFocus) {
                    mIsPowerUping = true;
                    final int bundleSize = 1;
                    mFmServiceHandler.removeMessages(FMRadioListener.MSGID_POWERUP_FINISHED);
                    mFmServiceHandler.removeMessages(FMRadioListener.MSGID_POWERDOWN_FINISHED);
                    Bundle bundle = new Bundle(bundleSize);
                    bundle.putFloat(FM_FREQUENCY, FMRadioUtils.computeFrequency(mCurrentStation));
                    handlePowerUp(bundle);
                }
                break;

            default :
                break;
        }
    }
    
    /**
     * FM Radio listener record
     * 
     */
    private static class Record {
        int mHashCode; // hash code
        FMRadioListener mCallback; // call back
    }

    /**
     * register FM Radio listener, activity get service state should call this
     * method register FM Radio listener
     * 
     * @param callback
     *            FM Radio listener
     */
    public void registerFMRadioListener(FMRadioListener callback) {
        synchronized (mRecords) {
            // register callback in AudioProfileService, if the callback is
            // exist, just replace the event.
            Record record = null;
            int hashCode = callback.hashCode();
            final int n = mRecords.size();
            for (int i = 0; i < n; i++) {
                record = mRecords.get(i);
                if (hashCode == record.mHashCode) {
                    return;
                }
            }
            record = new Record();
            record.mHashCode = hashCode;
            record.mCallback = callback;
            mRecords.add(record);
        }
    }

    /**
     * hongen, call back from service to activity
     * 
     * @param bundle
     */
    private void notifyActivityStateChanged(Bundle bundle) {
        if (!mRecords.isEmpty()) {
            LogUtils.d(TAG, "notifyActivityStatusChanged:clients = " + mRecords.size());
            synchronized (mRecords) {
                Iterator<Record> iterator = mRecords.iterator();
                while (iterator.hasNext()) {
                    Record record = (Record) iterator.next();

                    FMRadioListener listener = record.mCallback;

                    if (listener == null) {
                        iterator.remove();
                        return;
                    }

                    listener.onCallBack(bundle);
                }
            }
        } /*else if (isServiceInit() && !isDeviceOpen()) {
            
            LogUtils.d(TAG, "service show exit toast to user");
            // back key exit activity, so when service exit fm, activity will not receive
            // call back from service.
            Handler handler = new Handler(getMainLooper());
            handler.post(new Runnable() {
                
                @Override
                public void run() {
                    Toast.makeText(mContext, "exit fm now.", Toast.LENGTH_SHORT).show();
                }
            });
        }*/
    }

    /**
     * unregister FM Radio listener
     * 
     * @param callback
     *            FM Radio listener
     */
    public void unregisterFMRadioListener(FMRadioListener callback) {
        remove(callback.hashCode());
    }

    /**
     * remove call back according hash code
     * 
     * @param hashCode
     *            call back hash code
     */
    private void remove(int hashCode) {
        synchronized (mRecords) {
            Iterator<Record> iterator = mRecords.iterator();
            while (iterator.hasNext()) {
                Record record = (Record) iterator.next();
                if (record.mHashCode == hashCode) {
                    iterator.remove();
                }
            }
        }
    }

    /**
     * check recording sd card is unmount
     * 
     * @param intent
     *            unmount sd card intent
     * @return true or false indicate whether current recording sd card is
     *         unmount or not
     */
    public boolean isRecordingCardUnmount(Intent intent) {
        String unmountSDCard = intent.getData().toString();
        LogUtils.d(TAG, "unmount sd card file path: " + unmountSDCard);
        return unmountSDCard.equalsIgnoreCase("file://" + sRecordingSdcard) ? true : false;
    }

    /**
     * handle FM over BT connect state
     * 
     * @param connectState
     *            FM over BT connect state
     */
    /*private void handleBtConnectState(int connectState) {
        if (!mIsPowerUp) {
            return;
        }

        switch (connectState) {
        case BluetoothA2dp.STATE_CONNECTED:
        case BluetoothA2dp.STATE_PLAYING:
            if (mUsingFMViaBTController) {
                LogUtils.d(TAG, "ACTION_CONNECTION_STATE_CHANGED: FM over BT already enabled, ignore this message");
                break;
            }
            if (!chipSupportOverBt()) {
                LogUtils.d(TAG, "chip not support fm over bt, ignore this message");
                break;
            }
            LogUtils.d(TAG, "ACTION_CONNECTION_STATE_CHANGED: disable FM audio first to avoid I2S noise!!");
            enableFMAudio(false);
            connectBtDevice();
            break;

        case BluetoothA2dp.STATE_DISCONNECTED:
        case BluetoothA2dp.STATE_DISCONNECTING:
            if (!FMRadioNative.setFMViaBTController(false)) {
                LogUtils.e(TAG, "failed to set FM over BT via Host!!");
                break;
            }
            LogUtils.d(TAG, "setFMViaBTController(false) succeeded!!");
            mUsingFMViaBTController = false;
            enableFMAudio(true);
            // need to update UI
            Bundle bundle = new Bundle(2);
            bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.LISTEN_RECORDMODE_CHANGED);
            bundle.putBoolean(FMRadioListener.KEY_IS_RECORDING_MODE, mIsInRecordingMode);
            notifyActivityStateChanged(bundle);
            break;

        default:
            LogUtils.d(TAG, "invalid fm over bt connect state");
            break;
        }
    }*/

    /**
     * handle FM over BT controller
     * 
     * @param fmOverBTState
     *            FM over BT controller state
     */
    /*private void fmOverBtController(int fmOverBTState) {
        switch (fmOverBTState) {
        case BluetoothA2dpService.FMSTART_SUCCESS:
            if (!FMRadioNative.setFMViaBTController(true)) {
                LogUtils.e(TAG, "failed to set FM over BT via Controller!!");
                break;
            }
            LogUtils.d(TAG, "setFMViaBTController(true) succeeded!!");
            mUsingFMViaBTController = true;
            // quit recording mode
            if (FeatureOption.MTK_FM_RECORDING_SUPPORT) {
                mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTPLAYBACK_FINISHED);
                mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPPLAYBACK_FINISHED);
                mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STARTRECORDING_FINISHED);
                mFmServiceHandler.removeMessages(FMRadioListener.MSGID_STOPRECORDING_FINISHED);
                setRecordingMode(false);
                Bundle bundle = new Bundle(2);
                bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.LISTEN_RECORDMODE_CHANGED);
                bundle.putBoolean(FMRadioListener.KEY_IS_RECORDING_MODE, mIsInRecordingMode);
                notifyActivityStateChanged(bundle);
            }
            enableFMAudio(false);
            break;

        case BluetoothA2dpService.FMSTART_FAILED:
            enableFMAudio(true);
            break;

        case BluetoothA2dpService.FMSTART_ALREADY:
            LogUtils.d(TAG, "ACTION_FM_OVER_BT_CONTROLLER: FM over BT already on-going!");
            break;

        default:
            LogUtils.d(TAG, "invalid fm over bt state");
            break;
        }
    }*/

    private int[] insertSearchedStation(int[] channels) {
        LogUtils.d(TAG, "insertSearchedStation.firstValidChannel:" + Arrays.toString(channels));
        int firstValidChannel = mCurrentStation;
        int channelNum = 0;
        if (null != channels) {
            Arrays.sort(channels);
            int size = channels.length;
            // Save searched stations into database by batch
            ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
            String defaultStationName = getString(R.string.default_station_name);
            for (int i = 0; i < size; i++) {
                if (FMRadioUtils.isValidStation(channels[i])) {
                    if (0 == channelNum) {
                        firstValidChannel = channels[i];
                    }

                    if (!FMRadioStation.isFavoriteStation(mContext, channels[i])) {
                        ops.add(ContentProviderOperation.newInsert(Station.CONTENT_URI)
                                .withValue(Station.COLUMN_STATION_NAME, defaultStationName)
                                .withValue(Station.COLUMN_STATION_FREQ, channels[i])
                                .withValue(Station.COLUMN_STATION_TYPE, FMRadioStation.STATION_TYPE_SEARCHED)
                                .build());
                    }
                    channelNum++;
                }
            }
            // Save search stations to database by batch
            try {
                mContext.getContentResolver().applyBatch(FMRadioStation.AUTHORITY, ops);
            } catch (RemoteException e) {
                LogUtils.d(TAG, "Exception when applyBatch searched stations " + e);
            } catch (OperationApplicationException e) {
                LogUtils.d(TAG, "Exception when applyBatch searched stations " + e);
            }
        }
        LogUtils.d(TAG, "insertSearchedStation.firstValidChannel:" + firstValidChannel + ",channelNum:" + channelNum);
        return (new int[] { firstValidChannel, channelNum });
    }

    class FmRadioServiceHandler extends Handler {
        public FmRadioServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Bundle bundle;
            boolean isPowerup = false;
            boolean isSwitch = true;

            switch (msg.what) {

            // power up
            case FMRadioListener.MSGID_POWERUP_FINISHED:
                bundle = msg.getData();
                handlePowerUp(bundle);
                break;

            // power down
            case FMRadioListener.MSGID_POWERDOWN_FINISHED:
                handlePowernDown();
                break;

            // fm exit
            case FMRadioListener.MSGID_FM_EXIT:
                if (mIsSpeakerUsed) {
                    setSpeakerPhoneOn(false);
                }
                powerDown();
                closeDevice();
                // Release FM player upon exit
                if (null != mFMPlayer) {
                    mFMPlayer.release();
                    mFMPlayer = null;
                }

                bundle = new Bundle(1);
                bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_FM_EXIT);
                notifyActivityStateChanged(bundle);
                // ALPS01270783 Finish favorite when exit FM
                if (mExitListener != null) {
                    mExitListener.onExit();
                }
                break;

            // switch antenna
            case FMRadioListener.MSGID_SWITCH_ANNTENNA:
                bundle = msg.getData();
                int value = bundle.getInt(FMRadioListener.SWITCH_ANNTENNA_VALUE);
                
                // if not support short antenna, just notify, not need to switch antenna.
                if (SHORT_ANNTENNA_SUPPORT) {
                    isSwitch = (switchAntenna(value) == 0) ? true : false;
                    LogUtils.d(TAG, "FmServiceHandler.switch anntenna:isWitch:" + isSwitch);
                } else {
                    // if ear phone insert, need dismiss plugin earphone dialog
                    // if earphone plug out and it is not play recorder state, show plug dialog.
                    if (0 == value) {
                        LogUtils.d(TAG, "FmServiceHandler.switch anntenna:dismiss dialog");
                        //powerUpAsync(FMRadioUtils.computeFrequency(mCurrentStation));
                        bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_SWITCH_ANNTENNA);
                        bundle.putBoolean(FMRadioListener.KEY_IS_SWITCH_ANNTENNA, true);
                        notifyActivityStateChanged(bundle);
                    } else {
                        // ear phone plug out, and recorder state is not play recorder state, show dialog.
                        if (mRecordState != FMRecorder.STATE_PLAYBACK) {
                            LogUtils.d(TAG, "FmServiceHandler.switch anntenna:show dialog");
                            bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_SWITCH_ANNTENNA);
                            bundle.putBoolean(FMRadioListener.KEY_IS_SWITCH_ANNTENNA, false);
                            notifyActivityStateChanged(bundle);
                        }
                    }
                }

                break;

            // tune to station
            case FMRadioListener.MSGID_TUNE_FINISHED:
                bundle = msg.getData();
                float tuneStation = bundle.getFloat(FM_FREQUENCY);
                boolean isTune = tuneStation(tuneStation);
                // if tune fail, pass current station to update ui
                if (!isTune) {
                    tuneStation = FMRadioUtils.computeFrequency(mCurrentStation);
                }
                bundle = new Bundle(4);
                bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_TUNE_FINISHED);
                bundle.putBoolean(FMRadioListener.KEY_IS_TUNE, isTune);
                bundle.putFloat(FMRadioListener.KEY_TUNE_TO_STATION, tuneStation);
                bundle.putBoolean(FMRadioListener.KEY_IS_POWER_UP, mIsPowerUp);
                notifyActivityStateChanged(bundle);
                break;

            // seek to station
            case FMRadioListener.MSGID_SEEK_FINISHED:
                bundle = msg.getData();
                mIsSeeking = true;
                float seekStation = seekStation(bundle.getFloat(FM_FREQUENCY), bundle.getBoolean(OPTION));
                boolean isSeekTune = false;
                int station = FMRadioUtils.computeStation(seekStation);
                if (FMRadioUtils.isValidStation(station)) {
                    isSeekTune = tuneStation(seekStation);
                }
                // if tune fail, pass current station to update ui
                if (!isSeekTune) {
                    seekStation = FMRadioUtils.computeFrequency(mCurrentStation);
                }
                bundle = new Bundle(2);
                bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_TUNE_FINISHED);
                bundle.putBoolean(FMRadioListener.KEY_IS_TUNE, isSeekTune);
                bundle.putFloat(FMRadioListener.KEY_TUNE_TO_STATION, seekStation);
                notifyActivityStateChanged(bundle);
                mIsSeeking = false;
                break;

            // start scan
            case FMRadioListener.MSGID_SCAN_FINISHED:
                int[] channels = null;
                int[] result = null;
                int scanTuneStation = 0;
                boolean isScan = true;
                mIsScanning = true;
                if (powerUpFM(FMRadioUtils.DEFAULT_STATION_FLOAT)) {
                    channels = startScan();
                }

                // check whether cancel scan
                if ((null != channels) && channels[0] == -100) {
                    LogUtils.d(TAG, "user canceled scan:channels[0]=" + channels[0]);
                    isScan = false;
                    result = new int[] { -1, 0 };
                } else {
                    result = insertSearchedStation(channels);
                    scanTuneStation = result[0];
                    isTune = tuneStation(FMRadioUtils.computeFrequency(scanTuneStation));
                    scanTuneStation = isTune ? scanTuneStation : mCurrentStation;
                }
                
                /*
                 * if there is stop command when scan, so it needs to mute fm avoid
                 * fm sound come out.
                 */
                if (mIsAudioFocusHeld) {
                    LogUtils.d(TAG, "there is not power down command.set mute false");
                    setMute(false);
                }
                bundle = new Bundle(4);
                bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_SCAN_FINISHED);
                bundle.putInt(FMRadioListener.KEY_TUNE_TO_STATION, scanTuneStation);
                bundle.putInt(FMRadioListener.KEY_STATION_NUM, result[1]);
                bundle.putBoolean(FMRadioListener.KEY_IS_SCAN, isScan);
                notifyActivityStateChanged(bundle);
                mIsScanning = false;
                break;
                
            // audio focus changed
            case FMRadioListener.MSGID_AUDIOFOCUS_CHANGED:
                bundle = msg.getData();
                int focusState = bundle.getInt(FMRadioListener.KEY_AUDIOFOCUS_CHANGED);
                updateAudioFocus(focusState);
                break;

            case FMRadioListener.MSGID_SET_RDS_FINISHED:
                bundle = msg.getData();
                setRDS(bundle.getBoolean(OPTION));
                break;

            case FMRadioListener.MSGID_SET_CHANNEL_FINISHED:
                bundle = msg.getData();
                setAudioChannel(bundle.getBoolean(OPTION));
                break;

            case FMRadioListener.MSGID_SET_MUTE_FINISHED:
                bundle = msg.getData();
                setMute(bundle.getBoolean(OPTION));
                break;

            case FMRadioListener.MSGID_ACTIVE_AF_FINISHED:
                activeAF();
                break;

            /********** recording **********/
            case FMRadioListener.MSGID_STARTRECORDING_FINISHED:
                startRecording();
                break;

            case FMRadioListener.MSGID_STOPRECORDING_FINISHED:
                stopRecording();
                break;

            case FMRadioListener.MSGID_STARTPLAYBACK_FINISHED:
                boolean isStart = startPlayback();
                // Can not start play back, call back to activity.
                if (!isStart) {
                    bundle = new Bundle(2);
                    bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.LISTEN_RECORDERROR);
                    bundle.putInt(FMRadioListener.KEY_RECORDING_ERROR_TYPE, FMRadioListener.NOT_AUDIO_FOCUS);
                    notifyActivityStateChanged(bundle);
                }
                break;

            case FMRadioListener.MSGID_STOPPLAYBACK_FINISHED:
                stopPlayback();
                break;

            case FMRadioListener.MSGID_RECORD_MODE_CHANED:
                bundle = msg.getData();
                setRecordingMode(bundle.getBoolean(OPTION));
                break;

            case FMRadioListener.MSGID_SAVERECORDING_FINISHED:
                bundle = msg.getData();
                saveRecording(bundle.getString(RECODING_FILE_NAME));
                break;

            default:
                break;
            }
        }

    }
    /**
     * handle power down, execute power down and call back to activity.
     */
    private void handlePowernDown() {
        Bundle bundle;
        boolean isPowerdown = powerDown();
        bundle = new Bundle(2);
        bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_POWERDOWN_FINISHED);
        bundle.putBoolean(FMRadioListener.KEY_IS_POWER_DOWN, isPowerdown);
        notifyActivityStateChanged(bundle);
    }

    /**
     * handle power up, execute power up and call back to activity.
     * @param bundle power up frequency
     */
    private void handlePowerUp(Bundle bundle) {
        boolean isPowerup = false;
        boolean isSwitch = true;
        long time = System.currentTimeMillis();
        Log.d(TAG, "performance test. service handler power up start:" + time);
        float curFrequency = bundle.getFloat(FM_FREQUENCY);

        // If not support antenna, return to show dialog
        if (!SHORT_ANNTENNA_SUPPORT && !isAntennaAvailable()) {
            Log.d(TAG, "call back to activity, earphone is not ready");
            mIsShortAntennaSupport = false;
            mIsPowerUping = false;
            bundle = new Bundle(2);
            bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_SWITCH_ANNTENNA);
            bundle.putBoolean(FMRadioListener.KEY_IS_SWITCH_ANNTENNA, false);
            notifyActivityStateChanged(bundle);
            return;
        }

        if (powerUpFM(curFrequency)) {
            isPowerup = initDevice(curFrequency);
            mPausedByTransientLossOfFocus = false;
        }
        mIsPowerUping = false;
        mIsShortAntennaSupport = true;
        bundle = new Bundle(2);
        bundle.putInt(FMRadioListener.CALLBACK_FLAG, FMRadioListener.MSGID_POWERUP_FINISHED);
        bundle.putBoolean(FMRadioListener.KEY_IS_POWER_UP, isPowerup);
        notifyActivityStateChanged(bundle);
        time = System.currentTimeMillis();
        Log.d(TAG, "performance test. service handler power up end:" + time);
    }
    
    /**
     * check activity is foreground or background
     */
    public boolean isActivityForeGround() {
        /* ALPS01006939.
         * get activity status, is foreground or background.
         * */
        List<ActivityManager.RunningTaskInfo> taskInfo = mActivityManager.getRunningTasks(1);
        ComponentName componentInfo = taskInfo.get(0).topActivity;
        LogUtils.d(TAG, "isActivityForeGround:" + componentInfo.getPackageName());
        return "com.mediatek.FMRadio".equals(componentInfo.getPackageName().toString());
    }

    /**
     * Get the recording sdcard path when staring record
     * @return sdcard path like "/storage/sdcard0"
     */
    public static String getRecordingSdcard() {
        return sRecordingSdcard;
    }

    /// ALPS01270783 Finish favorite when exit FM @{
    private static OnExitListener mExitListener = null;

    public interface OnExitListener {
        /**
         * When Service finish, should notify FmRadioFavorite to finish
         */
        void onExit();
    }

    public static void registerExitListener(OnExitListener listener) {
        mExitListener = listener;
    }

    public static void unregisterExitListener(OnExitListener listener) {
        mExitListener = null;
    }
    /// @}

    /// ALPS01293092 The show name in save dialog but saved in service @{
    // If modify the save title it will be not null, otherwise it will be null
    private String mModifiedRecordingName = null;

    public String getModifiedRecordingName() {
        LogUtils.d(TAG, "getRecordingNameInDialog:" + mModifiedRecordingName);
        return mModifiedRecordingName;
    }

    public void setModifiedRecordingName(String name) {
        LogUtils.d(TAG, "setRecordingNameInDialog:" + name);
        mModifiedRecordingName = name;
    }
    /// @}
}
