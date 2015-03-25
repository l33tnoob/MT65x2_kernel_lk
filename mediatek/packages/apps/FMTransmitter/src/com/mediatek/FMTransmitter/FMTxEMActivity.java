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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;

import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.graphics.Color;


import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.text.Editable;
import android.text.InputFilter;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Spinner;
import android.widget.Toast;


import com.mediatek.common.featureoption.FeatureOption;

import java.io.IOException;

public class FMTxEMActivity extends Activity implements OnClickListener {
    
        /**
         * global FMTxLogUtils string
         */
    private static final String TAG = "FMTxEMActivity";
    //private Context thisActivity = null;
    private static final String PREFERENCENAME = "userData";
    /**
     * current Tx device status 
     */
    public static enum TxDeviceStateEnum {
        TXOPENED,
        TXPOWERUP,
        TXCLOSED        
    };
    
    /**
     * current RDS status
     */
    public static enum RDSStateEnum {
        RDSENABLED,
        RDSDISABLED        
    };
    
    
    
    /**
     * search related messages
     */
    private static final int     SEARCH_IN_PROCESS = 1;
    private static final int     SEARCH_INTERRUPTED = 2;
    private static final int     SEARCH_FINISHED = 3;
    
    private static final int     PLAY_STATE_QUERY = 4;
    private static final int     PLAY_FINISHED = 5;
    private static final int      MONITOR_PRECISION = 2000;
    private static final String ASCIILIB = "[a - z A - Z]";
    
    ProgressDialog mSearchDialog = null;
        /**
         * all frequency must in this range (gdMinFrequency, gdMaxFrequency), 
         * all input frequency must be checked before used as a parameter for service request
         */
        private static final float MIN_FREQUENCY = 87.5f;
        private static final float MAX_FREQUENCY = 108.0f;
        /**
         * record target frequency, each time user click on the "power up", 
         * this frequency should be used to power tx
         * when system clicked one frequency in the listView,  
         * targetFrequency should be updated
         * each time user click on the "set" button, targetFrequency should be updated
         */
        private float mTargetFrequency = 87.9f;
        
        /**
         * max unoccupied channel frequencies to limit, this limit number is provided by driver
         */
        private static final int MAXCHANNELS = 10;
        private static int sChannelsToSearch = 0;
        private static short sPi = 0;
        private float[] mFrequencyList = null;
        private static final float SEARCH_GAP = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 0.05f : 0.1f;
        private static final int MAX_LENGTH = FeatureOption.MTK_FM_50KHZ_SUPPORT  ? 6 : 5;
        private static final int BASE_NUMBER = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 100 : 10;
        
        /**
         * text size on power up/down button
         */
        private static final int FONT_SIZE = 22;
        /**
        
         * constant Strings
         */
        private static final String MHZ = "MHz";
        private static final String SAVED_FREQUENCY = "lastTimeFrequency";
        
        
        /**
         * views in on the UI
         */
        EditText    mInputMaxChannels = null;
        Button         mButtonSearch = null;
        //ListView    mSpinnerList = null;
        Spinner     mSpinnerList = null;
        ImageButton    mButtonMinus = null;
        ImageButton    mButtonPlus = null;
        EditText    mInputFrequency = null;
        //Button     gCheckFrequency = null;
        Button         mButtonSetFrequency = null;
        EditText    mMp3Path = null;
        Button         mButtonPlay = null;
        EditText    mInputRdsText = null;
        CheckBox     mSwitchRds = null;
        Button        mButtonPowerOn = null;
        
        ArrayAdapter<String> mChannelsAdapter = null;
        
        private int mOriginalBackground = Color.LTGRAY;
        
        private SearchThread mSearchThread = null;
        
        /**
         * search unoccupied channels thread related handlers
         */
        Handler mMainHandler = null;
        Handler mSearchThreadHandler = null;
        
        /**
         * system service related
         */
        private boolean mIsPlaying = false;
        private MediaPlayer mMediaPlayer = null;
        PlayerStateMonitorThread mMediaThread = null;
        /**
         * service related
         */
        private IFMTransmitterService mFMTxService = null;
        private ServiceConnection mServiceConnection = new ServiceConnection() {
            
            //@Override
            public void onServiceDisconnected(ComponentName name) {
                // TODO Auto-generated method stub
                FMTxLogUtils.d(TAG, "mServiceConnection - onServiceDisconnected called.");
                mFMTxService = null;
                
            }

            //@Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                // TODO Auto-generated method stub
                mFMTxService = IFMTransmitterService.Stub.asInterface(service);
                FMTxLogUtils.d(TAG, "mServiceConnection - onServiceConnected called.");
                FMTxLogUtils.v(TAG, "mServiceConnection - myService = " + mFMTxService);
                if (mFMTxService != null) {
                    /*added @ 2010-12-20*/
                try {
                    if (mFMTxService.isServiceInit()) {
                        mTargetFrequency = mFMTxService.getCurFrequency();
                    } else {
                        mFMTxService.initService(mTargetFrequency);
                    }
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, "service call  failed.");
                }
                    enableAllView();
                    updatePowerBtnView();
                }
            }
        };
        
        private class FMTxEMBroadcastReceiver extends BroadcastReceiver {
            public void onReceive(Context context, Intent intent) {
                FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.onReceive");
                String action = intent.getAction();                
                if (action.equals(FMTransmitterService.ACTION_STATE_CHANGED)) {
                    updatePowerBtnView();
                } else {
                    FMTxLogUtils.e(TAG, "Error: undefined Tx action.");
                }
                FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.onReceive");
            }
        }

        private FMTxEMBroadcastReceiver mBroadcastReceiver = null;
        /** Called when the activity is first created. */
        //@Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            FMTxLogUtils.d(TAG, "-->onCreate");
            setContentView(R.layout.fm_tx_em);
            //thisActivity = this; 
            SharedPreferences preferences = getSharedPreferences(PREFERENCENAME, MODE_PRIVATE);
            mTargetFrequency = preferences.getFloat(SAVED_FREQUENCY, MIN_FREQUENCY);
                
            mInputMaxChannels = (EditText) findViewById(R.id.inputMaxChannels);    
            mButtonSearch = (Button) findViewById(R.id.searchUnoccupiedChannels);            
            mSpinnerList = (Spinner)findViewById(R.id.channelsList);
            mButtonMinus = (ImageButton) findViewById(R.id.minusFrequency);
            mButtonPlus = (ImageButton) findViewById(R.id.plusFrequency);
            mInputFrequency = (EditText) findViewById(R.id.inputFrequency);
            mInputFrequency.setFilters(new InputFilter[]{new InputFilter.LengthFilter(MAX_LENGTH)});
            /*
            gCheckFrequency = (Button) findViewById(R.id.checkFrequency);
            gCheckFrequency.setOnClickListener(this);
            */
            mButtonSetFrequency = (Button) findViewById(R.id.setFrequency);
            mMp3Path = (EditText) findViewById(R.id.inputMp3Path);
            //mMp3Path.setEnabled(false);            
            mButtonPlay = (Button) findViewById(R.id.musicAction);
            mInputRdsText = (EditText) findViewById(R.id.inputRdsText);
            //mInputRdsText.setEnabled(false);            
            mSwitchRds = (CheckBox) findViewById(R.id.switchRds);
            mButtonPowerOn = (Button) findViewById(R.id.powerFMTx);
            
            if (mInputMaxChannels == null
                    || mButtonSearch == null
                    || mSpinnerList == null
                    || mButtonMinus == null
                    || mButtonPlus == null
                    || mInputFrequency == null
                    || mButtonSetFrequency == null
                    || mMp3Path == null
                    || mButtonPlay == null
                    || mInputRdsText == null
                    || mSwitchRds == null
                    || mButtonPowerOn == null
                    ) {
                FMTxLogUtils.e(TAG, "clocwork worked...");    
                //not return and let exception happened.
            }
            
            mMainHandler = new Handler() {
                //@Override
                public void handleMessage(Message msg) {
                    
                    FMTxLogUtils.d(TAG, "-->main Handler - handleMessage");
                    switch (msg.what) {
                    case SEARCH_IN_PROCESS:
                        if (mSearchDialog != null) {
                            mSearchDialog.dismiss();
                            mSearchDialog = null;
                        }
                        mSearchDialog = new ProgressDialog(FMTxEMActivity.this);
                        mSearchDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                        mSearchDialog.setTitle("Search");
                        mSearchDialog.setMessage("Search for unoccupied channel, please wait ...");
                        mSearchDialog.setCancelable(false);
                        mSearchDialog.show();
                        break;
                    case SEARCH_FINISHED:
                        mSearchThreadHandler.getLooper().quit();
                        mSearchThread = null;
                        mSearchThreadHandler = null;
                        if (null != mFrequencyList) {
                            FMTxLogUtils.v(TAG, "search finished, need to update channels list");
                            mChannelsAdapter.clear();
                            if (0 < mFrequencyList.length) {
                                for (int i = 0; i < mFrequencyList.length; i++) {
                                    FMTxLogUtils.v(TAG, "mFrequencyList[" + i + "] = " + mFrequencyList[i]);
                                    mChannelsAdapter.add("" + mFrequencyList[i]);
                                }                                
                                mSpinnerList.setEnabled(true);
                                mSpinnerList.setSelection(0);
                                //set to target frequency
                                setTarFrequency(mFrequencyList[0]);
                            } else {
                                mChannelsAdapter.clear();
                                showIndication("Cannot search out available channel", true);
                                mSpinnerList.setEnabled(false);
                            }
                        } else {
                            showIndication("Cannot search out available channel", true);
                            mChannelsAdapter.clear();                            
                        }
                        if (mSearchDialog != null) {
                            mSearchDialog.dismiss();
                            mSearchDialog = null;
                        }
                        break;
                    case SEARCH_INTERRUPTED:
                        if (mSearchDialog != null) {
                            mSearchDialog.dismiss();
                            mSearchDialog = null;
                        }
                        break;
                    case PLAY_FINISHED:
                        mMediaThread = null;
                        //music is playing, stop it
                        updatePlayerUI(false);
                        break;
                    default:
                        break;
                    }
                }
            };
                
            mButtonMinus.setOnClickListener(this);
            mButtonPlus.setOnClickListener(this);
            mInputFrequency.setText(new Float(mTargetFrequency).toString());
            mButtonSetFrequency.setOnClickListener(this);
            mButtonPlay.setOnClickListener(this);
            mSwitchRds.setOnClickListener(this);
            mButtonPowerOn.setOnClickListener(this);
            mButtonSearch.setOnClickListener(this);

            mChannelsAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
            mChannelsAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            
            mSpinnerList.setAdapter(mChannelsAdapter);
            mSpinnerList.setEnabled(false);
            mSpinnerList.setOnItemSelectedListener(new OnItemSelectedListener() {

                public void onItemSelected(AdapterView<?> arg0, View arg1,
                        int arg2, long arg3) {
                    FMTxLogUtils.d(TAG, "selected index = " + arg2);
                    //parameter validity check
                    if (null != mFrequencyList && arg2 < mFrequencyList.length) {
                        //set frequency edittext to this frequency 
                        mInputFrequency.setText("" + mFrequencyList[arg2]);
                        //turn to selected frequency and update current frequency state
                        setTarFrequency(mFrequencyList[arg2]);
                    }
                }

                public void onNothingSelected(AdapterView<?> arg0) {
                    FMTxLogUtils.d(TAG, "onNothingSelected");
                }
            });
            mButtonPowerOn.setText("");
            disableAllView();
            bindFMTransmitterService();
            
            mOriginalBackground = Color.LTGRAY;//mButtonPowerOn.g;
            // Register broadcast receiver.
            IntentFilter filter = new IntentFilter();
            filter.addAction(FMTransmitterService.ACTION_STATE_CHANGED);
            mBroadcastReceiver = new FMTxEMBroadcastReceiver();
            FMTxLogUtils.d(TAG, "Register Tx broadcast receiver.");
            registerReceiver(mBroadcastReceiver, filter);
        }

        @Override
        protected void onResume() {
            FMTxLogUtils.d(TAG, "--> onResume");
            super.onResume();
                if (null != mFMTxService) {
                    updatePowerBtnView();
                }
                FMTxLogUtils.d(TAG, "<-- onResume");
        }
        private void disableAllView() {
            mInputMaxChannels.setEnabled(true);
            mButtonSearch.setEnabled(true);
            mButtonMinus.setEnabled(false);
            mInputFrequency.setEnabled(false);
            mButtonPlus.setEnabled(false);
            //gCheckFrequency.setEnabled(false);
            mButtonSetFrequency.setEnabled(false);
            mMp3Path.setEnabled(false);
            mButtonPlay.setEnabled(false);
            mInputRdsText.setEnabled(false);
            mSwitchRds.setEnabled(false);
            mButtonPowerOn.setEnabled(false);
        }
        
        private void enableAllView() {
            mInputMaxChannels.setEnabled(false);
            mButtonSearch.setEnabled(false);
            mButtonMinus.setEnabled(true);
            mInputFrequency.setEnabled(true);
            mButtonPlus.setEnabled(true);
            //gCheckFrequency.setEnabled(true);
            mButtonSetFrequency.setEnabled(true);
            mMp3Path.setEnabled(true);
            mButtonPlay.setEnabled(true);
            //mInputRdsText.setEnabled(true);
            //this should not be enabled here, 
            //this EditText can and should only be enabled when RDS is turned off
            mSwitchRds.setEnabled(true);
            if (mSwitchRds.isChecked()) {
                mInputRdsText.setEnabled(false);
            } else {
                mInputRdsText.setEnabled(true);
            }
            mButtonPowerOn.setEnabled(true);
        }
        
        @Override
        protected void onSaveInstanceState(Bundle outState) {
            FMTxLogUtils.d(TAG, "-->onSaveInstanceState, mTargetFrequency = " + mTargetFrequency);
            super.onSaveInstanceState(outState);
            outState.putFloat(SAVED_FREQUENCY, mTargetFrequency);
        }
        @Override
        protected void onStop() {
            FMTxLogUtils.d(TAG, "-->onStop");
            super.onStop();
        }
        protected void onDestroy() {
            FMTxLogUtils.d(TAG, "-->onDestroy");
            if (mMediaThread != null) {
                mMediaThread.interrupt();
                mMediaThread = null; 
            }
            if (null != mMediaPlayer) {
                //mMediaPlayer.stop();
                mMediaPlayer.reset();
                mMediaPlayer.release();
                mMediaPlayer = null;
            }
            if (mFMTxService != null) {
                //close Tx device
                switch(getTxStatus()) {
                case TXOPENED:
                    try {
                        mFMTxService.closeTxDevice();
                    } catch (RemoteException e) {
                        e.printStackTrace();
                        FMTxLogUtils.e(TAG, "service call - closeTx(xxx) failed.");
                    }
                    break;
                case TXPOWERUP:
                    if (getRDSTxStatus() == RDSStateEnum.RDSENABLED) {
                        try {
                            if (mFMTxService.setRDSTxEnabled(false)) {
                                FMTxLogUtils.v(TAG, 
                                        "service call - setRDSEnabled(false) return true.");
                            } else {
                                FMTxLogUtils.e(TAG, 
                                        "service call - setRDSEnabled(false) return false.");
                            }
                        } catch (RemoteException e) {
                            e.printStackTrace();
                            FMTxLogUtils.e(TAG, 
                                    "service call - setRDSEnabled(false) failed.");
                        }
                    }
                    try {
                        mFMTxService.powerDownTx();
                        mFMTxService.closeTxDevice();
                        } catch (RemoteException e) {
                            e.printStackTrace();
                            FMTxLogUtils.e(TAG, "close FM Tx failed.");
                        }
                    break;
                case TXCLOSED:
                    FMTxLogUtils.v(TAG, "FM Tx in close state.");
                    break;
                default:
                    break;
                        
                }
            }
            unbindService(mServiceConnection);
            mSearchDialog = null;
            //thisActivity = null;
            
            SharedPreferences preferences = 
                getSharedPreferences(PREFERENCENAME, MODE_WORLD_READABLE);
            Editor edit = preferences.edit();
            edit.putFloat(SAVED_FREQUENCY, mTargetFrequency);
            edit.commit();
            super.onDestroy();
        }
        private boolean bindFMTransmitterService() {
            // TODO Auto-generated method stub
            if (bindService(new Intent("com.mediatek.FMTransmitter.IFMTransmitterService"),
                    mServiceConnection, Context.BIND_AUTO_CREATE)) {
                showIndication("bindService call succeed" , false);
                return true;
            } else {
                showIndication("bindService call failed" , true);
                //service call finished, end this activity
                return false;
            }
        }

        @Override
        public void onClick(View v) {
            if (v.equals(mButtonSearch)) {
                //search unoccupied channnels button is clicked
                handleSearchBtn();
            } else if (v.equals(mButtonMinus)) {
                //minus frequency button is clicked
                handleMinusBtn();
            } else if (v.equals(mButtonPlus)) {
                //plus frequency button is clicked
                handlePlusBtn();
            } else if (v.equals(mButtonSetFrequency)) {
                //set frequency button is clicked
                handleSetBtn();
            } else if (v.equals(mButtonPlay)) {
                //play/stop play music button is clicked
                disableAllView();
                handlePlayBtn();
                updatePowerBtnView();
                updatePlayerUI(false);
            } else if (v.equals(mSwitchRds)) {
                //RDS on/off checkboc is clicked
                mSwitchRds.setEnabled(false);
                mInputRdsText.setEnabled(false);    
                handleRdsSwitch();
                switch (getRDSTxStatus()) {
                case RDSENABLED:
                    mSwitchRds.setChecked(true);
                    mInputRdsText.setEnabled(false);
                    break;
                case RDSDISABLED:
                    mSwitchRds.setChecked(false);
                    mInputRdsText.setEnabled(true);
                    break;
                default:
                    break;
                }
                mSwitchRds.setEnabled(true);
            } else if (v.equals(mButtonPowerOn)) {
                //Power on/off FM Tx button is clicked
                handlePowerBtn();
                updatePowerBtnView();
            }
        }
        
        /**
         * handle user's click on "Search" button
         */
        public void handleSearchBtn() {
            //check whether max channel is ok or not first
            FMTxLogUtils.d(TAG, "-->handleSearchBtn");
            try {
                mFMTxService.sendPowerdownFMRxMsg();
            } catch (RemoteException e) {
                e.printStackTrace();
                FMTxLogUtils.e(TAG,
                        "mFMTxService.sendPowerdownFMRxMsg expception in handleSearchBtn()");
            }
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
                
            }
            String s = mInputMaxChannels.getText().toString();
            if (s.length() == 0) {
                showIndication("please input a maximum frequencies number", true);
                return;
            }
            int channels = new Integer(s); 
            if (channels > MAXCHANNELS || channels < 1) {
                 showIndication("invalid parameter" + "\nmin channel numbers = 1" 
                         + "\nmax channel numbers = " + MAXCHANNELS , true);
             } else {
                 sChannelsToSearch = channels; 
                 if (null == mSearchThread) {
                     mSearchThread = new SearchThread();
                     mSearchThread.start();
                 }
             }
        }
        
        /**
         * handle user's click on "Minus" button
         */
        public void handleMinusBtn() {
            //check whether is in the specified range of FM frequency first
            FMTxLogUtils.d(TAG, "-->handleMinusBtn");
            float fre = new Float(mInputFrequency.getText().toString());
            fre = fre - SEARCH_GAP;
            fre = Math.round(fre * BASE_NUMBER);
            fre = fre / BASE_NUMBER;
            if (fre >= MIN_FREQUENCY && fre <= MAX_FREQUENCY) {
                mInputFrequency.setText(new Float(fre).toString());
            } else {
                mInputFrequency.setText(new Float(MAX_FREQUENCY).toString());
            }
        }
    
        /**
         * handle user's click on "Minus" button
         */
        public void handlePlusBtn() {
            //check whether is in the specified range of FM frequency first
            FMTxLogUtils.d(TAG, "-->handlePlusBtn");    
            float fre = new Float(mInputFrequency.getText().toString());
               
            fre = fre + SEARCH_GAP;
            fre = Math.round(fre * BASE_NUMBER);
            fre = fre / BASE_NUMBER;
            if (fre > MIN_FREQUENCY && fre < MAX_FREQUENCY) {
                FMTxLogUtils.v(TAG, "fre ok.");    
                mInputFrequency.setText(new Float(fre).toString());
            } else {
                FMTxLogUtils.w(TAG, "fre out of range.");    
                mInputFrequency.setText(new Float(MIN_FREQUENCY).toString());
            }
        }
        
        /**
         * handle user's click on "Check" button
         */
        public void handleCheckBtn() {
            //check whether is in the specified range of FM frequency first
            FMTxLogUtils.d(TAG, "-->handleCheckBtn");
            serviceValidCheck();
            String s = mInputFrequency.getText().toString();
            if (s.length() == 0) {
                showIndication("please input a frequency value", true);
                return;
            }
            float frequency = new Float(s);
            //check frequency
            if (frequency > MAX_FREQUENCY || frequency < MIN_FREQUENCY) {
                showIndication("Please input an Frequency between "
                        + MIN_FREQUENCY + " and " + MAX_FREQUENCY, true);
                return;
            }
        }
        
        /**
         * handle user's click on "Set" button
         * @throws RemoteException 
         */
        public void handleSetBtn() {
            //check whether is in the specified range of FM frequency first
            FMTxLogUtils.d(TAG, "-->handleSetBtn");
            String s = mInputFrequency.getText().toString();
            if (s.length() == 0) {
                showIndication("please input a frequency value", true);
                return;
            }
            float fre = new Float(s);
            setTarFrequency(fre);
        }
        
        /**
         * handle user's click on "Play/Stop" button
         */
        public void handlePlayBtn() {
            //check whether target mp3 file exist or not
            updatePlayerUI(true);
            FMTxLogUtils.d(TAG, "-->handlePlayBtn");    
            String status = Environment.getExternalStorageState();
            FMTxLogUtils.v(TAG, "SD card status: " + status);
            if (mMp3Path.getText().toString().length() == 0) {
                showIndication("input a valid music name" , true);
                updatePlayerUI(false);
                return;
            }
            if (mIsPlaying) {
                //music is playing, stop it
                if (null != mMediaPlayer) {
                    mMediaPlayer.stop();
                }
                if (mMediaThread != null) {
                    mMediaThread.interrupt();
                    mMediaThread = null; 
                }
                mIsPlaying = false;
                
            } else {
                //start to play the music
                if (null == mMediaPlayer) {
                    mMediaPlayer = new MediaPlayer();
                    if (null == mMediaPlayer) {
                        showIndication("MediaPlayer creatation failed" , true);
                        updatePlayerUI(false);
                        return;
                    }
                }
                mMediaPlayer.reset();
                mMediaPlayer.setLooping(true);
                try {
                    mMediaPlayer.setDataSource("/sdcard/" 
                            + mMp3Path.getText().toString());
                    mMediaPlayer.prepare();
                    mMediaPlayer.start();
                    //mMediaPlayer.setLooping(true)
                    //here we make the player to play without stop 
                    mIsPlaying = true;    
                    if (null == mMediaThread) {
                        mMediaThread = new PlayerStateMonitorThread();
                    }
                    if (null != mMediaThread) {
                        mMediaThread.start();
                        FMTxLogUtils.d(TAG, "PlayerStateMonitorThread creation succeed");
                    }
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, "IllegalArgumentException");
                } catch (IllegalStateException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, "IllegalStateException");
                } catch (IOException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, "IOException - " + e.getMessage()) ;
                    showIndication("please make sure file /sdcard/" + 
                            mMp3Path.getText().toString() + " exist" , true);
                } 
            }
            updatePlayerUI(false);
        } 
        
        private void updatePlayerUI(boolean state) {
            if (state) {
                mButtonPlay.setEnabled(false);
                mMp3Path.setEnabled(false);
            } else {
                mButtonPlay.setEnabled(true);
                if (mIsPlaying) {
                mButtonPlay.setText("Stop");
                    mMp3Path.setEnabled(false);
                } else {
                    mButtonPlay.setText("Play");
                    mMp3Path.setEnabled(true);
                }
            }
        }
        
        /**
         * handle user's click on RDS CheckBox
         */
        public void handleRdsSwitch() {
            //check whether target RDS text in the range of
            FMTxLogUtils.d(TAG, "-->handleRdsSwitch");    
            serviceValidCheck();
            //check if RDS is supported or not
            try {
                if (!mFMTxService.isRDSTxSupport()) {
                    showIndication("RDS is not supported" , true);
                    mSwitchRds.setEnabled(false);
                    return;
                }
            } catch (RemoteException e1) {
                e1.printStackTrace();
                FMTxLogUtils.e(TAG, 
                        "service call - mFMTxService.isRDSSupport() failed.");
            }
            switch (getRDSTxStatus()) {
            case RDSENABLED:
                try {
                    mFMTxService.setRDSTxEnabled(false);
                } catch (RemoteException eEnable) {
                    eEnable.printStackTrace();
                    showIndication("turn off RDS failed" , true);
                    FMTxLogUtils.e(TAG, 
                            "service call - setRDSEnabled(true) failed.");
                }
                break;
            case RDSDISABLED:
                String sRdsText = mInputRdsText.getText().toString();
                
                if (sRdsText.length() == 0 || sRdsText.length() > 8) {
                    showIndication("invalid RDS text length," 
                            + " please input an ASCII string whose length is between 1 ~ 8" , false);
                    return;
                }
                Editable e = mInputRdsText.getText();
                char[] ps = new char[8];
                e.getChars(0, e.length(), ps, 0);
                for (int i = 0 ; i < e.length() ; i++) {
                    if (!(ps[i] >= 0 && ps[i] <= 0x7F)) {
                        FMTxLogUtils.e(TAG, "error code " + new String(ps));
                        showIndication("invalid RDS text content," 
                                + " please input an ASCII string" , false);
                        return;
                    }
                }
                
                if (e.length() < 8) {
                    //fill unused positon in 0x20
                    for (int i = e.length(); i < 8; i++) {
                        ps[i] = 0x20;
                    }
                }
                FMTxLogUtils.v(TAG, "ps = " + new String(ps));
                try {
                    mFMTxService.setRDSTxEnabled(true);
                    sPi += 1;
                    mFMTxService.setRDSText(sPi, ps, null, 0);
                } catch (RemoteException eDisable) {
                    eDisable.printStackTrace();
                    showIndication("turn on RDS failed" , true);
                    FMTxLogUtils.e(TAG, 
                            "service call setRDSEnabled(false) failed.");
                }
                break;
            default:
                FMTxLogUtils.e(TAG, "this should never be seen");
                break;
            }
            
        }  
        
        /**
         * handle user's click on "Turn on/off" button
         * @throws RemoteException 
         */
        public void handlePowerBtn() {
            //check current power on/off status first    
            FMTxLogUtils.d(TAG, "-->handlePowerBtn");
            serviceValidCheck();
            mButtonPowerOn.setEnabled(false);
            switch(getTxStatus()) {
            case TXOPENED:
                boolean earphone = false;
                try {
                    earphone = mFMTxService.isEarphonePluged();
                } catch (RemoteException e) {
                    FMTxLogUtils.e(TAG, "isEarphonePluged call failed.");
                    earphone = false;
                    e.printStackTrace();
                }
                if (earphone) {
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("Attention");
                    builder.setMessage("Please remove earphone.");
                    builder.setPositiveButton("OK" , null);
                    builder.create().show();
                    break;
                }
                try {
                    mFMTxService.powerUpTx(mTargetFrequency);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG,
                            "service call - powerUpTx(xxx) failed.");
                }
                
                break;
            case TXPOWERUP:
                if (getRDSTxStatus() == RDSStateEnum.RDSENABLED) {
                    try {
                        if (mFMTxService.setRDSTxEnabled(false)) {
                            mSwitchRds.setChecked(false);
                        }
                    } catch (RemoteException e) {
                        e.printStackTrace();
                        FMTxLogUtils.e(TAG,
                                "service call - setRDSEnabled(false) failed.");
                    }
                }
                try {
                    mFMTxService.powerDownTx();
                } catch (RemoteException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG,
                            "service call - powerDownTx() failed.");
                }
                break;
            case TXCLOSED:
                earphone = false;
                try {
                    earphone = mFMTxService.isEarphonePluged();
                } catch (RemoteException e) {
                    FMTxLogUtils.e(TAG, 
                            "isEarphonePluged call failed.");
                    earphone = false;
                    e.printStackTrace();
                }
                if (earphone) {
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("Attention");
                    builder.setMessage("Please remove earphone.");
                    builder.setPositiveButton("OK" , null);
                    builder.create().show();
                    break;
                }
                try {
                    if (mFMTxService.openTxDevice()) {
                        if (!mFMTxService.isFMTxSupport()) {
                            FMTxLogUtils.e(TAG, 
                                    "mServiceConnection - Tx is not supported in this device");
                            mFMTxService.closeTxDevice();
                        }
                        mFMTxService.powerUpTx(mTargetFrequency);
                    }
                } catch (RemoteException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, 
                            "service call - openTxDevice()/powerUpTx(xxx) failed.");
                }
                
                break;
            default:
                break;
                    
            }
            //restore current button's status
            mButtonPowerOn.setEnabled(true);
        }
        /**
         * get current Tx device's state
         * @return TxDeviceStateEnum
         */
        public TxDeviceStateEnum getTxStatus() {
          serviceValidCheck();
            try {
                if (mFMTxService.isTxPowerUp()) {
                    FMTxLogUtils.i(TAG, "tx state -> power up.");    
                    return TxDeviceStateEnum.TXPOWERUP;
                } else if (mFMTxService.isTxDeviceOpen()) {
                    FMTxLogUtils.i(TAG, "tx state -> open.");    
                    return TxDeviceStateEnum.TXOPENED;
                } else {
                    FMTxLogUtils.i(TAG, "tx state -> close.");    
                    return TxDeviceStateEnum.TXCLOSED;
                }
            } catch (RemoteException e) {
                e.printStackTrace();
                FMTxLogUtils.e(TAG,
                        "service call - isTxPowerUp()failed.");
            }
            return TxDeviceStateEnum.TXCLOSED;
        }
        
        
        public RDSStateEnum getRDSTxStatus() {
             serviceValidCheck();
             try {
                 return mFMTxService.isRDSOn() ? RDSStateEnum.RDSENABLED : RDSStateEnum.RDSDISABLED;
             } catch (RemoteException e) {
                 e.printStackTrace();
                 FMTxLogUtils.e(TAG, 
                         "service call - FMTxService.isRDSOn()failed.");
             }
            return RDSStateEnum.RDSDISABLED;
        }
        
        /**
         * set current frequency to mTargetFrequency
         */
        private boolean setTarFrequency(float frequency) {
            if (frequency >= MIN_FREQUENCY && frequency <= MAX_FREQUENCY) {
                //if current frequency is not equal to the target frequency, 
                //set current frequency to  target frequency  
                //and then update the power up/downbutton 
                if (mTargetFrequency != frequency) {
                    mTargetFrequency = frequency;
                    turnTogfTargetFrequency();
                    if (getTxStatus() == TxDeviceStateEnum.TXPOWERUP) {
                        boolean shouldRDSon = false;
                        if (getRDSTxStatus() == RDSStateEnum.RDSENABLED) {
                            shouldRDSon = true;
                        } else {
                            shouldRDSon = false;
                        }
                        //should set mute
                        //handlePowerBtn(); //close.
                        //handlePowerBtn();  //reopen.
                        try {
                            mFMTxService.repowerTx(mTargetFrequency);
                        } catch (RemoteException e) {
                            FMTxLogUtils.e(TAG,
                                    "service call - repowerTx(xxx)failed.");
                        }
                        FMTxLogUtils.e(TAG, "Switch Power again.XX");
                        if (shouldRDSon) {                            
                            onClick(mSwitchRds);
                            FMTxLogUtils.e(TAG, "Switch RDS again.");
                        }
                    }
                    updatePowerBtnView();
                }
                return true;
            } else {
                showIndication("Please input an Frequency between "
                        + MIN_FREQUENCY + " and " + MAX_FREQUENCY, true);
                FMTxLogUtils.e(TAG, "target frequency "
                        + frequency + "out of range supported.");
                return false;
            }
        }
        
        /**
         * get current mTargetFrequency
         */
        private float getTarFrequency() {
            return mTargetFrequency;
        }
        
        private void turnTogfTargetFrequency() {
            //turn to mTargetFrequency
            serviceValidCheck();
            try {
                if (mFMTxService.isTxPowerUp()) {
                    if (mFMTxService.turnToFrequency(mTargetFrequency)) {
                        FMTxLogUtils.i(TAG, 
                                "turnTogfTargetFrequency succeed.");
                    } else {
                        showIndication("Set current frequency to "
                                + mTargetFrequency + " MHz failed" , true);
                        FMTxLogUtils.e(TAG, 
                                "turnTogfTargetFrequency failed.");
                    }
                }
            } catch (RemoteException e) {
                e.printStackTrace();
                FMTxLogUtils.e(TAG, 
                        "service call - turnToFrequency(xxx)failed.");
            }
        }
        
        private void showIndication(CharSequence text, boolean isLongNeeded) {
            Toast.makeText(this, text, isLongNeeded ? Toast.LENGTH_LONG : Toast.LENGTH_SHORT).show();
        }
        
        /**
         * update the power up/down button view status, set to target frequency
         */
        private void updatePowerBtnView() {
                switch(getTxStatus()) {
                case TXPOWERUP:
                    setPowerBtnText(mTargetFrequency, true);
                    if (mChannelsAdapter.getCount() > 0) {
                        mSpinnerList.setEnabled(true);
                    }
                    mButtonSearch.setEnabled(false);
                    mInputMaxChannels.setEnabled(false);
                    mButtonSetFrequency.setEnabled(true);
                    mSwitchRds.setEnabled(true);
                    mInputFrequency.setEnabled(true);
                    mButtonMinus.setEnabled(true);
                    mButtonPlus.setEnabled(true);
                    //set background to special color
                    break;
                default:
                    setPowerBtnText(mTargetFrequency, false);
                    if (mChannelsAdapter.getCount() > 0) {
                        mSpinnerList.setEnabled(true);
                    }
                    mButtonSearch.setEnabled(true);
                    mInputMaxChannels.setEnabled(true);
                    mButtonSetFrequency.setEnabled(false);
                    mSwitchRds.setEnabled(false);
                    mInputFrequency.setEnabled(false);
                    mButtonMinus.setEnabled(false);
                    mButtonPlus.setEnabled(false);
                    //set background to grey color
                    break;
                }
                if (getRDSTxStatus() == RDSStateEnum.RDSENABLED) {
                    mSwitchRds.setChecked(true);
                    mInputRdsText.setEnabled(false);
                } else {
                    mSwitchRds.setChecked(false);
                    mInputRdsText.setEnabled(true);
                }
        }

        /**
         * check if service is available or not , if 
         */
        private void serviceValidCheck() {
            if (mFMTxService == null) {
                FMTxLogUtils.e(TAG,
                        "service not connected yet, activity will exit.");
                finish();
            }
        }
        
        private void setPowerBtnText(float frequency, boolean onoff) {
            mButtonPowerOn.setTextSize(FONT_SIZE);
            mButtonPowerOn.setText("" + frequency + "    Turn " + (onoff ? "off" : "on"));
            mButtonPowerOn.setBackgroundColor(onoff ? Color.GREEN : mOriginalBackground);
            mButtonPowerOn.setEnabled(true);
        }
        
        private class SearchThread extends Thread {
            public void run() {
                FMTxLogUtils.d(TAG, "SearchThread begins");
                Looper.prepare();
                mSearchThreadHandler = new Handler() {
                    @Override
                    public void handleMessage(Message msg) {
                        switch(msg.what) {
                            case SEARCH_INTERRUPTED:
                                FMTxLogUtils.e(TAG,
                                        "SearchThread - receive the interrupted message.");
                                mSearchThreadHandler = null;
                                break;
                            default:
                                break;
                        }
                    }
                };
                serviceValidCheck();
                mMainHandler.sendEmptyMessage(SEARCH_IN_PROCESS);
                FMTxLogUtils.i(TAG, "-->SearchThread.run");
                try {
                    int direction = 0;
                    if (mTargetFrequency  < MIN_FREQUENCY + 1.0f) {
                        direction = 0;
                    } else if (mTargetFrequency  > MAX_FREQUENCY - 1.0f) {
                        direction = 1;
                    }
                    mFrequencyList =  mFMTxService.searchChannelsForTx(mTargetFrequency,
                            direction, sChannelsToSearch);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    FMTxLogUtils.e(TAG, 
                            "service call - searchChannelsForTx(xxx)failed.");
                    mFrequencyList = null;
                }
                //after search process finished, send SEARCH_FINISHED msg.
                mMainHandler.sendEmptyMessage(SEARCH_FINISHED);
                Looper.loop();
                FMTxLogUtils.d(TAG, "SearchThread exits");
            }
        }
        
        
        private class PlayerStateMonitorThread extends Thread {
            public void run() {
                FMTxLogUtils.d(TAG, "i am running, my Id = "
                        + getId() + "myName = " + getName());
                super.run();
                try { 
                    do { 
                        FMTxLogUtils.d(TAG,
                                "PlayerStateMonitorThread - receive PLAY_STATE_QUERY message.");
                        Thread.sleep(MONITOR_PRECISION);
                        if (mMediaPlayer != null) {
                            if (mMediaPlayer.isPlaying()) {
                                FMTxLogUtils.d(TAG,
                                        "MediaPlayer is playing.");
                            } else {
                                Message m = new Message();
                                m.what = FMTxEMActivity.PLAY_FINISHED;
                                mMainHandler.sendMessage(m);
                                FMTxLogUtils.d(TAG, "MediaPlayer is stopped.");
                                mIsPlaying = false;
                                break;
                            }
                        } else {
                            break;
                        }
                    } while (!FMTxEMActivity.PlayerStateMonitorThread.interrupted());
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                FMTxLogUtils.d(TAG, "bye bye, my Id = " + getId() + "myName = " + getName());
            }
    }
}
