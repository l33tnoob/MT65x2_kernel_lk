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

package com.mediatek.atci.service;

import android.app.Service;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.media.AudioManager;
import android.media.MediaPlayer;

import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;

import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;

import java.io.IOException;
import java.lang.ref.WeakReference;

public class FMRadioService extends Service {
    public static final String TAG = "FMRadioService";

    // Broadcast messages from FM service to clients.
    public static final String ACTION_STATE_CHANGED = 
        "com.mediatek.FMRadio.FMRadioService.ACTION_STATE_CHANGED";
    public static final String EXTRA_FMRADIO_ISPOWERUP = "EXTRA_FMRADIO_ISPOWERUP"; // boolean
    // Broadcast messages from clients to FM service.
    public static final String ACTION_TOFMSERVICE_POWERDOWN = 
        "com.mediatek.FMRadio.FMRadioService.ACTION_TOFMSERVICE_POWERDOWN";
    // Broadcast messages to FM Tx service.
    //public static final String ACTION_TOFMTXSERVICE_POWERDOWN = 
    //   "com.mediatek.FMTransmitter.FMTransmitterService.ACTION_TOFMTXSERVICE_POWERDOWN";
    // Broadcast messages to mATV service.
    //public static final String ACTION_TOATVSERVICE_POWERDOWN = 
    //    "com.mediatek.app.mtv.ACTION_REQUEST_SHUTDOWN";

    private static WeakReference<FMRadioService> sFMService = null;
    
    private boolean mIsEarphoneUsed = true;
    private boolean mIsDeviceOpen = false;
    private boolean mIsPowerUp = false;
    private int mCurrentStation = AtciService.FIRST_FREQUENCY;
    private boolean mIsServiceInit = false;
    private AudioManager mAudioManager = null;
    private MediaPlayer mFMPlayer = null;
    private WakeLock mWakeLock = null;

    // Audio Manager parameters
    private static final String AUDIO_PATH_LOUDSPEAKER = "AudioSetForceToSpeaker=1";
    private static final String AUDIO_PATH_EARPHONE = "AudioSetForceToSpeaker=0";

    private static final int NUM_TEN = 10;

    private boolean mIsRecording = false;


    private IFMRadioService.Stub mBinder = new IFMRadioService.Stub() {
        public boolean openDevice() {
            Log.i(TAG, ">>> FMRadioService.openDevice");
            boolean bRet = false;
            if (mIsDeviceOpen) {
                Log.e(TAG, "Error: device is already open.");
                bRet = true;
            } else {
                bRet = AtciFMRadioNative.opendev();
                if (bRet) {
                    mIsDeviceOpen = true;

                } else {
                    Log.e(TAG, "Error: opendev failed.");
                }
            }
            Log.i(TAG, "<<< FMRadioService.openDevice: " + bRet);
            return bRet;
        }

        public boolean closeDevice() {
            Log.i(TAG, ">>> FMRadioService.closeDevice");
            boolean bRet = false;
            if (mIsDeviceOpen) {
                bRet = AtciFMRadioNative.closedev();
                if (bRet) {
                    mIsDeviceOpen = false;
                } else {
                    Log.e(TAG, "Error: closedev failed.");
                }
            } else {
                Log.e(TAG, "Error: device is already closed.");
                bRet = true;
            }
            Log.i(TAG, "<<< FMRadioService.closeDevice: " + bRet);
            return bRet;
        }

        public boolean isDeviceOpen() {
            Log.i(TAG, ">>> FMRadioService.isDeviceOpen");
            Log.i(TAG, "<<< FMRadioService.isDeviceOpen: " + mIsDeviceOpen);
            return mIsDeviceOpen;
        }

        public boolean powerUp(float frequency) {
            Log.i(TAG, ">>> FMRadioService.powerUp: " + frequency);
            boolean bRet = false;
            mCurrentStation = (int) (frequency * NUM_TEN);
            /*if (FeatureOption.MTK_MT519X_FM_SUPPORT) {
                Intent intentToAtv = new Intent(ACTION_TOATVSERVICE_POWERDOWN);
                sendBroadcast(intentToAtv);
            }
            if (!FeatureOption.MTK_MT519X_FM_SUPPORT) {
                Intent intentToFMTx = new Intent(ACTION_TOFMTXSERVICE_POWERDOWN);
                sendBroadcast(intentToFMTx);
            }*/
                if (mIsPowerUp) {
                    Log.e(TAG, "Error: device is already power up.");
                    bRet = true;
                } else {
                    if (!FeatureOption.MTK_MT519X_FM_SUPPORT) {
                        // Sleep to wait for FM Tx power down.
                        try {
                            Thread.sleep(NUM_TEN);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                            Log.e(TAG, "Exception: Thread sleep.");
                        }
                    }
                    bRet = AtciFMRadioNative.powerup(frequency);
                    if (FeatureOption.MTK_MT519X_FM_SUPPORT 
                        && AtciFMRadioNative.isFMPoweredUp() == 0) {
                        // FM is actually NOT powered up
                        // due to mATV power down FM before FM.powerup returns
                        Log.e(TAG, "powerup: NOT powered up after calling powerup()!");
                        return false;
                    }
                    if (bRet) {
                        // Add notification to the title bar.

                        if (!FeatureOption.MTK_BT_FM_OVER_BT_VIA_CONTROLLER) {
                            enableFMAudio(true);
                        }
                        Intent itenMusic = new Intent("com.android.music.musicservicecommand.pause");
                        sendBroadcast(itenMusic);

                        mIsPowerUp = true;

                        // Broadcast message to applications.
                        Intent intent = new Intent(ACTION_STATE_CHANGED);
                        intent.putExtra(EXTRA_FMRADIO_ISPOWERUP, mIsPowerUp);
                        sendBroadcast(intent);

                        if (FeatureOption.MTK_MT519X_FM_SUPPORT) {
                            mWakeLock.acquire();
                        }
                    } else {
                        Log.e(TAG, "Error: powerup failed.");
                    }
                }

            Log.i(TAG, "<<< FMRadioService.powerUp: " + bRet);
            return bRet;
        }

        public boolean powerDown() {
            Log.i(TAG, ">>> FMRadioService.powerDown");
            boolean bRet = false;
            if (mIsPowerUp) {
                enableFMAudio(false);
                bRet = AtciFMRadioNative.powerdown(0); // 0, FM_RX; 1, FM_TX
                if (bRet) {
                    mIsPowerUp = false;
                    // Broadcast message to applications.
                    Intent intent = new Intent(ACTION_STATE_CHANGED);
                    intent.putExtra(EXTRA_FMRADIO_ISPOWERUP, mIsPowerUp);
                    sendBroadcast(intent);

                    if (FeatureOption.MTK_MT519X_FM_SUPPORT) {
                        mWakeLock.release();
                    }
                } else {
                    Log.e(TAG, "Error: powerdown failed.");
                }
            } else {
                Log.e(TAG, "Error: device is already power down.");
                bRet = true;
            }
            Log.i(TAG, "<<< FMRadioService.powerDown: " + bRet);
            return bRet;
        }

        public boolean isPowerUp() {
            Log.i(TAG, ">>> FMRadioService.isPowerUp");
            Log.i(TAG, "<<< FMRadioService.isPowerUp: " + mIsPowerUp);
            return mIsPowerUp;
        }

        public boolean tune(float frequency) {
            Log.i(TAG, ">>> FMRadioService.tune: " + frequency);
            boolean bRet = AtciFMRadioNative.tune(frequency);
            if (bRet) {
                mCurrentStation = (int) (frequency * NUM_TEN);
            }
            Log.i(TAG, "<<< FMRadioService.tune: " + bRet);
            return bRet;
        }

        public float seek(float frequency, boolean isUp) {
            Log.i(TAG, ">>> FMRadioService.seek: " + frequency + " " + isUp);
            float fRet = AtciFMRadioNative.seek(frequency, isUp);
            Log.i(TAG, "<<< FMRadioService.seek: " + fRet);
            return fRet;
        }

        public int setMute(boolean mute) {
            Log.i(TAG, ">>> FMRadioService.setMute: " + mute);
            int iRet = AtciFMRadioNative.setmute(mute);
            Log.i(TAG, "<<< FMRadioService.setMute: " + iRet);
            return iRet;
        }
        
        public void useEarphone(boolean use) {
            Log.i(TAG, ">>> FMRadioService.useEarphone: " + use);
            if (use) {
                mAudioManager.setParameters(AUDIO_PATH_EARPHONE);
                mIsEarphoneUsed = true;
            } else {
                mAudioManager.setParameters(AUDIO_PATH_LOUDSPEAKER);
                mIsEarphoneUsed = false;
            }
            Log.i(TAG, "<<< FMRadioService.useEarphone");
        }

        public boolean isEarphoneUsed() {
            Log.i(TAG, ">>> FMRadioService.isEarphoneUsed");
            Log.i(TAG, "<<< FMRadioService.isEarphoneUsed: " + mIsEarphoneUsed);
            return mIsEarphoneUsed;
        }

        public void initService(int iCurrentStation) {
            Log.i(TAG, ">>> FMRadioService.initService: " + iCurrentStation);
            mIsServiceInit = true;
            mCurrentStation = iCurrentStation;
            Log.i(TAG, "<<< FMRadioService.initService");
        }

        public boolean isServiceInit() {
            Log.i(TAG, ">>> FMRadioService.isServiceInit");
            Log.i(TAG, "<<< FMRadioService.isServiceInit: " + mIsServiceInit);
            return mIsServiceInit;
        }

        public int getFrequency() {
            Log.i(TAG, ">>> FMRadioService.getFrequency");
            Log.i(TAG, "<<< FMRadioService.getFrequency: " + mCurrentStation);
            return mCurrentStation;
        }

        public void resumeFMAudio() {
            Log.i(TAG, ">>> FMRadioService.resumeFMAudio");
            enableFMAudio(true);
            Log.i(TAG, "<<< FMRadioService.resumeFMAudio");
        }

        public int switchAntenna(int antenna) {
            Log.i(TAG, ">>> FMRadioService.switchAntenna");
            int ret = AtciFMRadioNative.switchAntenna(antenna);
            Log.i(TAG, "<<< FMRadioService.switchAntenna: " + ret);
            return ret;
        } // LXO add.
    };

    /** {@inheritDoc} */
    @Override
    public void onCreate() {
        Log.i(TAG, ">>> FMRadioService.onCreate");
        super.onCreate();
        sFMService = new WeakReference<FMRadioService>(this);
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mFMPlayer = new MediaPlayer();
        try {
            mFMPlayer.setDataSource("MEDIATEK://MEDIAPLAYER_PLAYERTYPE_FM");
        } catch (IOException ex) {
            //notify the user why the file couldn't be opened
            Log.e(TAG, "setDataSource: " + ex);
            return;
        } catch (IllegalArgumentException ex) {
            // notify the user why the file couldn't be opened
            Log.e(TAG, "setDataSource: " + ex);
            return;
        } catch (IllegalStateException ex) {
            Log.e(TAG, "setDataSource: " + ex);
            return;
        }
        mFMPlayer.setAudioStreamType(AudioManager.STREAM_FM);
        if (FeatureOption.MTK_MT519X_FM_SUPPORT) {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
            mWakeLock.setReferenceCounted(false);
        }

        // Register broadcast receiver.
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_TOFMSERVICE_POWERDOWN);
        filter.addAction(Intent.ACTION_SHUTDOWN);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_HEADSET_PLUG);
        Log.i(TAG, "<<< FMRadioService.onCreate");
    }

    /** {@inheritDoc} */
    @Override
    public void onDestroy() {
        sFMService = null;
        Log.i(TAG, ">>> FMRadioService.onDestroy");
        // When exit, we set the audio path back to earphone.
        try {
            if (!mIsEarphoneUsed) {
                mBinder.useEarphone(true);
            }
            if (mIsPowerUp) {
                mBinder.powerDown();
            }
            if (mIsDeviceOpen) {
                mBinder.closeDevice();
            }
        } catch (RemoteException e) {
            Log.e(TAG, "Exception: Cannot call binder function.");
        }
        
        // Release FM player upon exit
        if (mFMPlayer != null) {
            mFMPlayer.release();
        }
        super.onDestroy();
        Log.i(TAG, "<<< FMRadioService.onDestroy");
    }

    public IBinder onBind(Intent intent) {
        Log.i(TAG, ">>> FMRadioService.onBind");
        Log.i(TAG, "<<< FMRadioService.onBind: " + mBinder);
        return mBinder;
    }

    /** {@inheritDoc} */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, ">>> FMRadioService.onStartCommand intent: " + intent + " startId: " + startId);
        int iRet = super.onStartCommand(intent, flags, startId);
        Log.i(TAG, "<<< FMRadioService.onStartCommand: " + iRet);
        return iRet;
    }

    private void enableFMAudio(boolean bEnable) {
        Log.i(TAG, ">>> FMRadioService.enableFMAudio: " + bEnable);
        if (bEnable) {
            if (mFMPlayer.isPlaying()) {
                Log.i(TAG, "Error: FM audio is already enabled.");
            } else {
                try {
                    mFMPlayer.prepare();
                } catch (IOException e) {
                    Log.e(TAG, "Exception: Cannot call MediaPlayer prepare.", e);
                } catch (IllegalStateException e1) {
                    Log.e(TAG, "Exception: Cannot call MediaPlayer prepare.", e1);
                }
                mFMPlayer.start();
            }
        } else {
            if (mFMPlayer.isPlaying()) {
                mFMPlayer.stop();
            } else {
                Log.i(TAG, "Error: FM audio is already disabled.");
            }
        }
        Log.i(TAG, "<<< FMRadioService.enableFMAudio");
    }

    public static void onStateChanged(int state) {
        Log.d(TAG, ">>> onStateChanged");
        FMRadioService fmradioService = (FMRadioService) sFMService.get();
        if (fmradioService == null) {
            Log.d(TAG, "onStateChanged: service ref is null!!");
            return;
        }
        if (state == 0) {
            // FM has powered down from lower layers
            Log.d(TAG, "onStateChanged: FM has been powered down");
            if (fmradioService.mIsPowerUp) {
                // DO WE STILL NEED TO SETMUTE/RDSSET?????
                fmradioService.enableFMAudio(false);
                fmradioService.mIsPowerUp = false;

                // Broadcast to FM activity
                Intent intent = new Intent(ACTION_STATE_CHANGED);
                intent.putExtra(EXTRA_FMRADIO_ISPOWERUP, fmradioService.mIsPowerUp);
                fmradioService.sendBroadcast(intent);
            }
        }
        Log.d(TAG, "<<< onStateChanged: " + state);
    }

}
