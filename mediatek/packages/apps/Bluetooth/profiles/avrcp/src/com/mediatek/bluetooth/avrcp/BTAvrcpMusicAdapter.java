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

package com.mediatek.bluetooth.avrcp;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.media.AudioManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import com.android.music.IMediaPlaybackService;

import java.lang.ref.WeakReference;
import java.util.BitSet;

/**
 * @brief Avrcp-Music Adpater for the native Android Music player Note: bind Music service will be disconnected by framework
 *        if the service is idle around 1mins
 */
public class BTAvrcpMusicAdapter extends Thread {
    public static final String TAG = "MMI_AVRCP";

    private BluetoothAvrcpService mAvrcpSrv = null;

    private AudioManager mAudioMgr = null;

    private int mAudioMax = 100;

    private byte mCapabilities[]; // suppport event list. from 0x01~0x0d

    private byte mAttrs[]; // save the attr_id

    private byte mValueNum[]; // save the num of values which is regarding to

    // attr_id

    private byte mCurValue[]; // save the setting value to attr_id

    private byte mValuesEqualizer[];

    private byte mValuesRepeat[];

    private byte mValuesShuffle[];

    private byte mValuesScan[];

    private byte mPlayerStatus = 2;

    private byte mVolume = 0x12; // 0x00~0x7f(0%~100%)

    public byte mCurEqualSetting = 1;

    public byte mCurRepeatSetting = 1;

    public byte mCurShuffleSetting = 1;

    public byte mCurScanSetting = 1;

    private Context mContext;

    private long[] mAddToNowList;

    private final int mActionKey = 0x11;

    private final int mActionSetSetting = 0x12;

    private final int mActionKeyInfo = 0x21;

    private final int mActionRegNotify = 0x22; /* registered event */

    private boolean mStartBind = false;

    private volatile Looper mServiceLooper = null;

    private BitSet mRegBit;

    private BitSet mPendingRegBit; /* register when player service is ready ! */

    private long mNotifySongId = 0;

    private static boolean mStartReceiver = false;

    // Music intent
    public static final String PLAYSTATE_CHANGED = "com.android.music.playstatechanged";

    public static final String META_CHANGED = "com.android.music.metachanged";

    public static final String QUEUE_CHANGED = "com.android.music.queuechanged";

    public static final String QUIT_PLAYBACK = "com.android.music.quitplayback";

    public static final String PLAYBACK_COMPLETE = "com.android.music.playbackcomplete";

    public static final int NOW = 1; /* assign as now item */

    public static final int NEXT = 2; /* append to the next */

    public static final int LAST = 3;

    // native android music interface
    private boolean mPlayStartBind = false;

    private IMediaPlaybackService mPlayService = null;

    public static final int AVRCP_PLAY_STOP = 0;

    public static final int AVRCP_PLAY_PLAYING = 1;

    public static final int AVRCP_PLAY_PAUSE = 2;

    // Receive intent to update the song's information
    private static String sMusicArtist = null;

    private static String sMusicAlbum = null;

    private static String sMusicTrack = null;

    private static long sMusicId = -1;

    private static boolean sMusicPlaying = false;

    private static boolean mExtraAttribute = true;

    // if cannot find music serivce, turn it off
    private static boolean sPlayServiceInterface = true;


    // Support the FF_SEEK playstatus
    private static final int STATUS_STOPPED = 0x00;

    private static final int STATUS_PLAYING = 0x01;

    private static final int STATUS_PAUSED = 0x02;

    private static final int STATUS_FWD_SEEK = 0x03;

    private static final int STATUS_REV_SEEK = 0x04;

    private static int mPlayStatus = 0x00;

    private static int mPreviousPlayStatus = 0x00;

    public static final String DEFAULT_METADATA_STRING = " "; // avoid the empty string

    private boolean mConnected = false;     // avrcp is connected or not

    private boolean mUpdateSending = false; // avoid sending update during updating

    private static final int PLAY_UPDATE_PERIOD = 1000; //millis

    private static int mPreviousFFPlayStatus = STATUS_PLAYING; // keep the status before FF/REW

    private boolean mInitCapability = false;

    BTAvrcpMusicAdapter(Context context, BluetoothAvrcpService server) {
        mContext = context;
        mAvrcpSrv = server;
        mAudioMgr = (AudioManager) context.getSystemService(context.AUDIO_SERVICE);
        byte i;

        setName("BTAvrcpMusicAdapterThread ");
        if (mAudioMgr != null) {
            mAudioMax = mAudioMgr.getStreamMaxVolume(AudioManager.STREAM_MUSIC); // 0x7f(100%)
            mVolume = convertToAbosoluteVolume(mAudioMgr.getStreamVolume(AudioManager.STREAM_MUSIC));
        } else {
            mAudioMax = 0;
            mVolume = 0;
        }

        convertToMgrVolume((byte) (0x7f / 2));
        convertToMgrVolume((byte) 0x7f);
        convertToAbosoluteVolume((int) (mAudioMax / 2));
        convertToAbosoluteVolume((int) (mAudioMax));

        // setup the capability 
        Log.i(TAG, "[BT][AVRCP] BTAvrcpMusicAdapter construct");
        checkCapability();

        // init flag
        mStartReceiver = false;
    }
	  
	  
    private void checkCapability(){
        byte version = 10;
        byte i = 0;
        version = getSupportVersion();
	  	  
        if( true == mInitCapability ){
            Log.i(TAG, "[BT][AVRCP] version:" + Byte.toString(version) );
            return;
        }
								
        Log.i(TAG, "[BT][AVRCP] init capability version:" + Byte.toString(version) );
        mInitCapability = true; 
	
        mAttrs = new byte[2]; // BTAvrcpProfile.AVRCP_MAX_ATTRIBUTE_NUM];
        mValueNum = new byte[2];// BTAvrcpProfile.AVRCP_MAX_ATTRIBUTE_NUM];
        mCurValue = new byte[BTAvrcpProfile.AVRCP_MAX_ATTRIBUTE_NUM];

        // setup the Android default music player's capability
        i = 0;
        if( version == 14 ){
                mCapabilities = new byte[0x05];
      	}else{
      		mCapabilities = new byte[0x02];
      	}
      	
        mCapabilities[i++] = BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED;
        mCapabilities[i++] = BTAvrcpProfile.EVENT_TRACK_CHANGED;
        // mCapabilities[i++] = BTAvrcpProfile.EVENT_TRACK_REACHED_END ;
        // mCapabilities[i++] = BTAvrcpProfile.EVENT_TRACK_REACHED_START ;
        // mCapabilities[i++] = BTAvrcpProfile.EVENT_PLAYBACK_POS_CHANGED ;
        // mCapabilities[i++] = BTAvrcpProfile.EVENT_BATT_STATUS_CHANGED ;
        // mCapabilities[i++] = BTAvrcpProfile.EVENT_SYSTEM_STATUS_CHANGED ;
        // mCapabilities[i++] =
        // BTAvrcpProfile.EVENT_PLAYER_APPLICATION_SETTING_CHANGED ;
        if( version == 14 ){
        mCapabilities[i++] = BTAvrcpProfile.EVENT_NOW_PLAYING_CONTENT_CHANGED;
        mCapabilities[i++] = BTAvrcpProfile.EVENT_AVAILABLE_PLAYERS_CHANGED;
        mCapabilities[i++] = BTAvrcpProfile.EVENT_ADDRESSED_PLAYER_CHANGED;
        // mCapabilities[i++] = BTAvrcpProfile.EVENT_UIDS_CHANGED ;
        // mCapabilities[i++] = BTAvrcpProfile.EVENT_VOLUME_CHANGED ;
        }

        i = 0;// page 143 0x01~0x04
        // mAttrs[i++] = BTAvrcpProfile.APP_SETTING_EQUALIZER;
        mAttrs[i++] = BTAvrcpProfile.APP_SETTING_REPEAT_MODE;
        mAttrs[i++] = BTAvrcpProfile.APP_SETTING_SHUFFLE;
        // mAttrs[i++] = BTAvrcpProfile.APP_SETTING_SCAN;

        // page 143, number of Values
        i = 0;
        mValueNum[i++] = 2;
        mValueNum[i++] = 2;
        // mValueNum[i++] = 2;
        // mValueNum[i++] = 1;

        // page 143, sample
        mCurValue[0] = 1; // OFF
        mCurValue[1] = 1;
        // mCurValue[2] = 1;
        // mCurValue[3] = 1;

        // mValuesEqualizer = new byte[2];
        mValuesRepeat = new byte[3];
        mValuesShuffle = new byte[2];
        // mValuesScan = new byte[2];

        mValuesRepeat[0] = BTAvrcpProfile.REPEAT_MODE_OFF;
        mValuesRepeat[1] = BTAvrcpProfile.REPEAT_MODE_SINGLE_TRACK;
        mValuesRepeat[2] = BTAvrcpProfile.REPEAT_MODE_ALL_TRACK;

        mValuesShuffle[0] = BTAvrcpProfile.SHUFFLE_OFF;
        mValuesShuffle[1] = BTAvrcpProfile.SHUFFLE_ALL_TRACK;

        mRegBit = new BitSet(16);
        mRegBit.clear();

        mPendingRegBit = new BitSet(16);
        mPendingRegBit.clear();

        if (BluetoothAvrcpService.sSupportMusicUI) {
            this.start(); // start to run a thread
        } else {
            Log.i(TAG, "[BT][AVRCP] No AvrcpMusic debug looper");
        }
    }

    public void init() {
        // startToBind();
        // Can NOT start music service. it will take too long time~~~
        // startToBindPlayService();
        startReceiver();
    }

    public byte getSupportVersion() {
        // 
        checkAndBindPlayService(false);

        if (sPlayServiceInterface) {
            return (byte) 14; // have aidl interface
        }
        return (byte) 13; // avrcp v1.3
    }

    public void deinit() {
        
            Log.i(TAG, "[BT][AVRCP] Adapter deinit");

        if (null != mPlayService) {
            mAvrcpSrv.unbindService(mPlayConnection);
            mPlayService = null;
        }
        if (null != mServiceLooper) {
            mServiceLooper.quit();
            mServiceLooper = null;
        }
        stopReceiver();

        if (null != mHandler) {
            Log.i(TAG, "[BT][AVRCP] BTAvrcpMusicAdapter mHandler join 2");
            this.interrupt();

            try {
                this.join(100);
            } catch (InterruptedException ex) {
                Log.i(TAG, "[BT][AVRCP] join fail");
            }
        }
    }

    public void onConnect() {
        Log.i(TAG, "[BT][AVRCP] Adapter onConnect");
        startToBind();
        Log.i(TAG, "[BT][AVRCP] mbPlayServiceInterface is " + Boolean.toString(sPlayServiceInterface));

            synchronized (mRegBit) {
            mRegBit.clear();
            }
        if( null != mPendingRegBit ){
        synchronized (mPendingRegBit) {
            mPendingRegBit.clear();
        }
        }
	
        mConnected = true;
    }

    public void onDisconnect() {
            Log.i(TAG, "[BT][AVRCP] Adapter onDisconnect");

        synchronized (mRegBit) {
            mRegBit.clear();
        }
        synchronized (mPendingRegBit) {
            mPendingRegBit.clear();
        }

        mConnected = false;
        mUpdateSending = false;
    }

    public byte[] playerAppCapabilities() {
        return mCapabilities;
    }

    public byte[] listPlayerAppAttribute() {
        return mAttrs;
    }

    public byte[] listPlayerAppValue(byte attrId) {
        byte a[];
        // search the attr id

        switch (attrId) {
            case BTAvrcpProfile.APP_SETTING_EQUALIZER:
                return mValuesEqualizer;
                // break;
            case BTAvrcpProfile.APP_SETTING_REPEAT_MODE:
                return mValuesRepeat;
                // break;
            case BTAvrcpProfile.APP_SETTING_SHUFFLE:
                return mValuesShuffle;
                // break;
            case BTAvrcpProfile.APP_SETTING_SCAN:
                return mValuesScan;
                // break;
            default:
                break;
        }
        Log.w(TAG, String.format("[BT][AVRCP] listPlayerAppValue attr_id:%d", attrId));
        return null;
    }

    /**
    * @brief Get Player Setting from applicatoin
    * @return 0 is invalid attrId
    */
    public byte getCurPlayerAppValue(byte attrId) {
        int value = 0;

        checkAndBindMusicService();
        Log.i(TAG, String.format("[BT][AVRCP] getCurPlayerAppValue attrId:%d", (int)attrId));
        if( null != mPlayService ){
        switch (attrId) {
            case BTAvrcpProfile.APP_SETTING_REPEAT_MODE:
                try {
                    value = mPlayService.getRepeatMode();
                    Log.i(TAG, String.format("[BT][AVRCP] getRepeatMode ret %d", value));
                } catch (RemoteException ex) {
                    Log.w(TAG, String.format("[BT][AVRCP] Exception ! Fail to getRepeatMode %d %s", value, ex.toString()));
                }
                return (byte)(value+1);
                // break;
            case BTAvrcpProfile.APP_SETTING_SHUFFLE:
                try {
                    value = mPlayService.getShuffleMode();
                    Log.i(TAG, String.format("[BT][AVRCP] getShuffleMode ret %d", value));
                } catch (RemoteException ex) {
                    Log.w(TAG, String.format("[BT][AVRCP] Exception ! Fail to getShuffleMode %d %s", value, ex.toString()));
                }
                return (byte)(value+1);
                // break;
            default:
                break;
        }
        // Pass to player
        }else{
        	Log.i(TAG, String.format("[BT][AVRCP] getCurPlayerAppValue no mPlayService"));
        }

        Log.w(TAG, String.format("[BT][AVRCP] attr_id is not find attr_id:%d", attrId));
        return (byte)0; // 0 is invalid value
    }

	/**
	* @brief Set the player setting to AVRCP App Value
	* Android default definition is not the same as AVRCP App Value in spec.
	*/
    public boolean setPlayerAppValue(byte attrId, byte value) {
        boolean lDone = false;
        int playerMode = 0; // ex. MediaPlaybackService.REPEAT_NONE is 0
        int targetMode = 0; // ex. BTAvrcpProfile.REPEAT_MODE_OFF is 1

        checkAndBindMusicService();
        if( null != mPlayService ){
        if (attrId == BTAvrcpProfile.APP_SETTING_REPEAT_MODE) {
           
            lDone = true;
            switch (value) {
                case BTAvrcpProfile.REPEAT_MODE_OFF:
                	playerMode = 0; // REPEAT_NONE in player
                	break;
                case BTAvrcpProfile.REPEAT_MODE_SINGLE_TRACK:
                	playerMode = 1; // REPEAT_CURRENT
                	break;
                case BTAvrcpProfile.REPEAT_MODE_ALL_TRACK:
                	playerMode = 2; // REPEAT_ALL
                	break;
                default:
                	lDone = false; // PTS TC_TG_PAS_BI_05_C need to test reject !
                break;
            }
                    try {
                        if (lDone) {
                    mPlayService.setRepeatMode(playerMode);
                            Log.i(TAG, String.format("[BT][AVRCP] setRepeatMode ret %s", Boolean.toString(lDone)));
                        }
                    } catch (RemoteException ex) {
                        Log.w(TAG, String
                                .format("[BT][AVRCP] Exception ! Fail to setRepeatMode %d %s", value, ex.toString()));
                    }
        }
        if (attrId == BTAvrcpProfile.APP_SETTING_SHUFFLE) {
            lDone = true;
            switch (value) {
                case BTAvrcpProfile.SHUFFLE_OFF:
                	playerMode = 0; // SHUFFLE_NONE in player
                case BTAvrcpProfile.SHUFFLE_ALL_TRACK:
                	playerMode = 1; // SHUFFLE_NORMAL
                	break;
                case BTAvrcpProfile.REPEAT_MODE_ALL_TRACK:
                	playerMode = 2; // SHUFFLE_AUTO
                    break;
                default:
                	lDone = false; // PTS TC_TG_PAS_BI_05_C need to test reject !
                    break;
            }
                    try {
                        if (lDone) {
                    mPlayService.setShuffleMode(playerMode);
                            Log.i(TAG, String.format("[BT][AVRCP] setShuffleMode ret %s", Boolean.toString(lDone)));
                        }
                    } catch (RemoteException ex) {
                        Log.w(TAG, String.format("[BT][AVRCP] Exception ! Fail to setShuffleMode %d %s", value, ex
                                .toString()));
                        Log.w(TAG, ex.toString());
                    }
	           
            }
        }
        if (!lDone) {
            Log.w(TAG, String.format("[BT][AVRCP] fail to set attr_id:%d to value:%d", attrId, value));
        }
        return lDone;
    }

    public String getPlayerAppAttrText(byte attrId) {

        switch (attrId) {
            case BTAvrcpProfile.APP_SETTING_EQUALIZER:
                return "Equalizer Setting";
            case BTAvrcpProfile.APP_SETTING_REPEAT_MODE:
                return "RepeatMode Setting";
            case BTAvrcpProfile.APP_SETTING_SHUFFLE:
                return "Shuffle Setting";
            case BTAvrcpProfile.APP_SETTING_SCAN:
                return "Scan Setting";
            default:
                Log.w(TAG, String.format("[BT][AVRCP] getPlayerAppAttrText unknow id:%d", attrId));
                return null;
        }
    }

    public String getPlayerAppValueText(byte attrId, byte valueId) {
        switch (attrId) {
            case BTAvrcpProfile.APP_SETTING_EQUALIZER:
                switch (valueId) {
                    case BTAvrcpProfile.EQUALIZER_OFF:
                        return "Equal Off";
                    case BTAvrcpProfile.EQUALIZER_ON:
                        return "Equal On";
                    default:
                        return null;
                }

            case BTAvrcpProfile.APP_SETTING_REPEAT_MODE:
                switch (valueId) {
                    case BTAvrcpProfile.REPEAT_MODE_OFF:
                        return "Repeat Off";
                    case BTAvrcpProfile.REPEAT_MODE_SINGLE_TRACK:
                        return "Repeat Single";
                    case BTAvrcpProfile.REPEAT_MODE_ALL_TRACK:
                        return "Repeat All";
                    default:
                        return null;
                }

            case BTAvrcpProfile.APP_SETTING_SHUFFLE:
                switch (valueId) {
                    case BTAvrcpProfile.SHUFFLE_OFF:
                        return "Shuffle Off";
                    case BTAvrcpProfile.SHUFFLE_ALL_TRACK:
                        return "Shuffle All";
                    default:
                        return null;
                }

            case BTAvrcpProfile.APP_SETTING_SCAN:
                switch (valueId) {
                    case BTAvrcpProfile.SCAN_OFF:
                        return "Equal Off";
                    case BTAvrcpProfile.SCAN_ALL_TRACK:
                        return "Equal On";
                    default:
                        return null;
                }

            default:
                return null;
        }
    }

    public void informBatteryStatus(byte status) {
        Log.i(TAG, String.format("[BT][AVRCP] informBatteryStatus status:%d", status));
    }

    public boolean informDisplayCharset(byte count, short charsets[]) {
        // go through all charsets. if not support any one, reject it

        for (byte i = 0; i < charsets.length && i < count; i++) {
            Log.w(TAG, String.format("[BT][AVRCP] charset i:%d value:%d", i, charsets[i]));
            if (charsets[i] == 0x6a) {
                return true;
            }
        }
        // no support charset in list.
        return false;
    }

    public void notificationBatteryStatusChanged(byte error, byte isinterim, byte status) {
        if (null != mAvrcpSrv) {
            mAvrcpSrv.notificationBatteryStatusChanged(error, isinterim, status);
        }
    }

    public void notificationSystemStatusChanged(byte error, byte isinterim, byte status) {
        if (null != mAvrcpSrv) {
            mAvrcpSrv.notificationSystemStatusChanged(error, isinterim, status);
        }
    }

    public void notificationVolumeChanged(byte error, byte isinterim, byte volume) {
        if (null != mAvrcpSrv) {
            mAvrcpSrv.notificationVolumeChanged(error, isinterim, volume);
        }
    }

    public byte getPlayerstatus() {
        byte status = (byte) 0xff; // got from service
        boolean bPlay = false;
        long id = 0;

        if (sMusicPlaying) {
            status = AVRCP_PLAY_PLAYING;// playing 1
        } else {
            if ((long) -1 != sMusicId) {
                status = AVRCP_PLAY_PAUSE;// pause is 2
            } else {
                status = AVRCP_PLAY_STOP;// stop is 0
            }
        }
        mPlayerStatus = status;

        return (byte) status;
    }

    public int getPlayerstatusSongLength() {
        int duration = 0;

        checkAndBindPlayService(true);
        if (null != mPlayService) {
            try {
                duration = (int) mPlayService.duration();
            } catch (RemoteException ex) {
                duration = 0;
            }
            return duration;
        }

        return 0x0;
    }

    public int getPlayerstatusSongPos() {
        int position = 0;

        checkAndBindPlayService(true);
        if (null != mPlayService) {
            try {
                position = (int) mPlayService.position();
            } catch (RemoteException ex) {
                position = 0;
            }
            return position;
        }
        return 0x0;
    }

    /*
     * registerNotification Interim
     */
    public boolean registerNotification(byte eventId, int interval) {
        boolean bReg = false;
        byte status;
        long lvalue;
        // / register the notification event to Music service and return the
        // interim response
        switch (eventId) {
            case BTAvrcpProfile.EVENT_TRACK_REACHED_END:
            case BTAvrcpProfile.EVENT_TRACK_REACHED_START:
            case BTAvrcpProfile.EVENT_PLAYBACK_POS_CHANGED:
            case BTAvrcpProfile.EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
                Log.i(TAG, String.format("[BT][AVRCP] MusicAdapter blocks support register event:%d", eventId));
                bReg = false;
                break;
            case BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED:
            case BTAvrcpProfile.EVENT_TRACK_CHANGED:
            case BTAvrcpProfile.EVENT_NOW_PLAYING_CONTENT_CHANGED:
                /* Need to response immediate response now */

                switch (eventId) {
                    case BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED:
                        /* assume stop status */
                        status = getPlayerstatus();
                        mAvrcpSrv.notificationPlayStatusChangedNative((byte) 0, (byte) 1, (byte) status);

                        /* player need notify this whether register or not */
                        bReg = true;

                        break;
                    case BTAvrcpProfile.EVENT_TRACK_CHANGED:
                        status = getPlayerstatus();
                        switch (status) {
                            case AVRCP_PLAY_PLAYING:
                            case AVRCP_PLAY_PAUSE:
                                lvalue = sMusicId;
                                break;
                            default: // AVRCP_PLAY_STOP
                                lvalue = 0xFFFFFFFF;
                                break;
                        }
                        mAvrcpSrv.notificationTrackChangedNative((byte) 0x0, (byte) 1, (long) lvalue);
                        bReg = true;
                        break;
                    case BTAvrcpProfile.EVENT_NOW_PLAYING_CONTENT_CHANGED:
                        mAvrcpSrv.notificationNowPlayingChangedNative((byte) 0x0, (byte) 1);
                        bReg = true;
                        break;
                    default:
                        break;
                }
                Log.w(TAG, "[BT][AVRCP] registerNotification " + Integer.toString(eventId));
                break;

            case BTAvrcpProfile.EVENT_BATT_STATUS_CHANGED:
            case BTAvrcpProfile.EVENT_SYSTEM_STATUS_CHANGED:
                if (null != mSystemListener) {
                    bReg = mSystemListener.regNotificationEvent(eventId, interval);
                }
                break;

            case BTAvrcpProfile.EVENT_VOLUME_CHANGED:
                Log.i(TAG, String.format("[BT][AVRCP] MusicAdapter blocks support register event:%d", eventId));
                bReg = false;
                break;
            default:
                break;
        }

        if (bReg) {

            synchronized (mRegBit) {
                mRegBit.set(eventId);
                bReg = mRegBit.get(eventId);
                Log.i(TAG, String.format("[BT][AVRCP] mRegBit set %d Reg:%b cardinality:%d", eventId, bReg, mRegBit
                        .cardinality()));
            }
        }

        return bReg;
    }

    public void abortContinueInd() {
        // / show a tost
        Log.w(TAG, "Receive an abort indication !");
    }

    public boolean setAbsoluteVolume(byte volume) {
        int adjVolume;
        int getVolume;
        int compare;
        if (mAudioMgr == null) {
            return false;
        }

        // if(volume > 0x7f){
        // return false;
        // }

        // 0(0%) 0x7f(100%) to 0 - mAudioMax
        adjVolume = convertToMgrVolume(volume);
        if (mAudioMgr != null) {
            getVolume = mAudioMgr.getStreamVolume(AudioManager.STREAM_MUSIC);
            mAudioMgr.setStreamVolume(AudioManager.STREAM_MUSIC, adjVolume, AudioManager.FLAG_PLAY_SOUND);
            compare = mAudioMgr.getStreamVolume(AudioManager.STREAM_MUSIC);
            Log.i(TAG, String.format("[BT][AVRCP] Adapter before:%d to-set:%d after:%d", getVolume, adjVolume, compare));
            if (compare == adjVolume) {
                mVolume = volume;
                return true;
            }
        }
        return false;
    }

    public byte getAbsoluteVolume() {
        return mVolume;
    }

    private byte convertToAbosoluteVolume(int iMgrVolume) {
        byte ret = 0;
        ret = (byte) (((float) iMgrVolume / mAudioMax) * 0x7f);
        Log.i(TAG, String.format("[BT][AVRCP] Adapter convertToAbosoluteVolume Mgr(%d) to abs(%d) MaxMgr(%d)", iMgrVolume,
                ret, mAudioMax));
        return ret;
    }

    private int convertToMgrVolume(byte absolute) {
        int ret = 0;
        ret = (int) (((float) absolute / 0x7f) * mAudioMax);
        Log.i(TAG, String.format("[BT][AVRCP] Adapter convertToMgrVolume absolute(%d) to Mgr(%d) MaxMgr(%d)", absolute, ret,
                mAudioMax));
        return ret;
    }

    /**
     * When using bindService with default param, the bindService will onDisconnect state if idle 1 mins. need to bind again.
     */
    public void checkAndBindMusicService() {
         // @deprecated use checkAndBindPlayService()
    }

    /**
     * When service is initializing, the check of bindService should not be used to pending
     */
    public void checkAndBindPlayService(boolean wait) {

        if (sPlayServiceInterface) {
            if (null == mPlayService) {
                try {
                    startToBindPlayService();
                    if( true == wait ){
                    	//when testing PTS, it maybe lost the bind connection
                        sleep(2000);
                        //ALPS00415905 cannot wait when using 
                    	Log.w(TAG, "[BT][AVRCP] sleep 2000 to wait for binding "); 
                    }
                } catch (Exception ex) {

                }
            }
        } else {
            Log.w(TAG, "[BT][AVRCP] ignore the mMusic playService");
        }

    }

    public boolean playItems(long id) {
        boolean ret = false;
        boolean hasExit = false;
        long curList[];
        int i = 0;

        // add this to now playing list
        if (id == 0) {
            Log.v(TAG, "[BT][AVRCP] Wrong id 0");
            return true;
        }

        checkAndBindPlayService(true);
        if (null != mPlayService) {
            try {
                curList = new long[1];
                curList[0] = id;
                mPlayService.enqueue(curList, NOW);
                Log.i(TAG, String.format("[BT][AVRCP] enqueu %d", curList[0]));
                ret = true;
            } catch (RemoteException ex) {
                Log.w(TAG, "[BT][AVRCP] BTAvrcpMusicAdapter playItem enqueue exception:" + ex.getMessage());
            }
        }
        return ret;
    }

    public boolean addToNowPlaying(long id) { /* MCN_NP_BV_04_C */
        boolean ret = false;
        boolean hasExit = false;
        long curList[];
        int i = 0;

        checkAndBindPlayService(true);
        if (null != mPlayService) {
            try {
                curList = new long[1];
                curList[0] = id;
                mPlayService.enqueue(curList, LAST);
                Log.i(TAG, String.format("[BT][AVRCP] enqueu %d", curList[0]));
                ret = true;
            } catch (RemoteException ex) {
                Log.w(TAG, "[BT][AVRCP] BTAvrcpMusicAdapter addToNowPlaying enqueue exception:" + ex.getMessage());
            }
        }

        return ret;
    }

    public long[] getNowPlaying() {
        long playing[] = null;

        checkAndBindPlayService(true);
        if (null == mPlayService) {
            Log.i(TAG, "[AVRCP] no mPlayService for getNowPlaying");
            return null;
        }
        try {
            playing = mPlayService.getQueue();
            Log.i(TAG, "[AVRCP] getQueue from mPlayService");
            if( null != playing ){
               Log.i(TAG, "[AVRCP] getQueue from mPlayService length:" + Integer.toString(playing.length));
            }
        } catch (RemoteException ex) {
            playing = null;
        }
        return playing;
    }

    public void sendAvrcpKeyEvent(int keyvalue, byte isPress) {
        Message msg;
        int apKey = 0;
        String sMsg;
        sMsg = String.format("[BT][AVRCP] Receive a Avrcpkey:%d (APKey:%d)", keyvalue, apKey);

        Log.v(TAG, sMsg);

        if (null != mContext && isPress == 1 && null != mHandler) {
            // Toast.makeText( mContext, sMsg, Toast.LENGTH_SHORT);
            // send a message to itself
            msg = mHandler.obtainMessage();
            msg.what = mActionKey;

            msg.arg1 = keyvalue;
            msg.arg2 = isPress;

            mHandler.sendMessage(msg);

        }

        // convert AvrcpKey to MMI key
    }

    /*
     * Only show the indication
     */
    public void passThroughKeyInd(int keyvalue, byte isPress) {
        Message msg;
        int apKey = 0;
        String sMsg;
        sMsg = String.format("[BT][AVRCP] Receive a Avrcpkey:%d (APKey:%d)", keyvalue, apKey);

        Log.v(TAG, sMsg);

        if (null != mContext && isPress == 1 && BluetoothAvrcpService.sSupportMusicUI) {
            // Toast.makeText( mContext, sMsg, Toast.LENGTH_SHORT);
            // send a message to itself
            if (null != mHandler) {
                msg = mHandler.obtainMessage();
                msg.what = mActionKeyInfo;

                msg.arg1 = keyvalue;
                msg.arg2 = isPress;

                mHandler.sendMessage(msg);
            }
        }

        // convert AvrcpKey to MMI key
    }

    public boolean passNotifyMsg(int event, int interval) {
        Message msg;
        if (null != mHandler) {
            msg = mHandler.obtainMessage();
            msg.what = mActionRegNotify;

            msg.arg1 = event;
            msg.arg2 = interval;

            mHandler.sendMessage(msg);
            return true;
        }
        return false;
    }

    public String getElementAttribute(long identifier, int attrId) {
        String s = null;
        int lsplit = 0;
        long id = 0;

        // if( identifier != 0 ){
        // if( null != mPlayService ){
        // try {
        // id = mPlayService.getAudioId();
        // }catch(Exception ex){
        // }
        // }
        // spec 58 all other values other than 0x0 are currently reserved
        // Log.w( TAG,
        // String.format("[BT][AVRCP] AVRCP getElementAttribute identifider:%d %d",
        // identifier, id) );
        // return null;
        // }

        Log.w(TAG, String.format("[BT][AVRCP] AVRCP getElementAttribute %b", (null != mPlayService)));

        //try {
            switch (attrId) {
                case BTAvrcpProfile.MEIDA_ATTR_TITLE:
                    s = sMusicTrack;
                    if (s == null) {
                        s = "";
                    }
                    break;
                case BTAvrcpProfile.MEIDA_ATTR_ARTIST:
                    s = sMusicArtist;
                    if (s == null) {
                        s = "";
                    }
                    break;
                case BTAvrcpProfile.MEIDA_ATTR_ALBUM:
                    s = sMusicAlbum;
                    if (s == null) {
                        s = "";
                    }
                    break;
                case BTAvrcpProfile.MEIDA_ATTR_NUM_OF_ALBUM:

                    break;
                case BTAvrcpProfile.MEIDA_ATTR_TOTAL_NUM:

                    break;
                case BTAvrcpProfile.MEIDA_ATTR_GENRE:

                    break;

                case BTAvrcpProfile.MEIDA_ATTR_PLAYING_TIME_MS:
                    break;
                default:
                    return s;
            }
            // Support the attr_id but cannot get it from player
            if (s == null) {
                s = "";
            }

//        }
//        catch (Exception ex) {
//            s = "";
//        }

        return s;
    }

    private Handler mHandler;

    /**
     * @brief MusicAdapter as a Looper
     */
    public void run() {

        Looper.prepare();
        mServiceLooper = Looper.myLooper();
        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                // process incoming messages here
                passToHandleMessage(msg);
            }
        };

        Looper.loop();
        mHandler = null;
    }

    public void passToHandleMessage(Message msg) {
        switch (msg.what) {
            case mActionSetSetting:
                handleSettingMessage(msg);
                break;
            case mActionKey:
            default:
                handleKeyMessage(msg);
                break;
        }
    }

    private void handleSettingMessage(Message msg) {

    }

    private void handleKeyMessage(Message msg) {
        int apKey = 0;
        String sMsg;
        long id = 0;
        int eventId = 0;

        switch (msg.what) {
            case mActionKey:
                sMsg = String.format("[BT][AVRCP] Receive a Avrcpkey:%d ", msg.arg1);
                Log.i(TAG, String.format("[BT][AVRCP] ACTION_KEY msg.what:%d arg1:%d arg2:%d", msg.what, msg.arg1,
                                msg.arg2));
                Toast.makeText(mContext, sMsg, Toast.LENGTH_SHORT).show();

                handleKeyMessageKeyEvent(msg);
                break;

            case mActionKeyInfo:

                Log.i(TAG, String.format("[BT][AVRCP] KEY_INFO msg.what:%d arg1:%d arg2:%d", msg.what, msg.arg1, msg.arg2));
                switch (msg.arg1) {
                    case BTAvrcpProfile.AVRCP_POP_POWER:
                        sMsg = "POWER Key";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_VOLUME_UP:
                        sMsg = "VOLUME UP";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_VOLUME_DOWN:
                        sMsg = "VOLUME DOWN";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_MUTE:
                        sMsg = "MUTE";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_PLAY:
                        sMsg = "PLAY";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_STOP:
                        sMsg = "STOP";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_PAUSE:
                        sMsg = "PAUSE";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_RECORD:
                        sMsg = "RECORD";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_REWIND:
                        sMsg = "REWIND";
			if( 0 == msg.arg2 ){ //release
				if( mPlayStatus == STATUS_REV_SEEK ){
				Log.i( TAG, "[AVRCP] back to playing status from rev_seek");
				mPlayStatus = mPreviousFFPlayStatus;
				Log.v( TAG, "[BT][AVRCP] update-info back mPlayStatus:" + Integer.toString(mPlayStatus));
				}
			}else{
				// before press the FF key, keep the old status
				if( mPlayStatus == STATUS_REV_SEEK || mPlayStatus == STATUS_FWD_SEEK ){
					mPreviousFFPlayStatus = STATUS_PLAYING;
				}else{
					mPreviousFFPlayStatus = mPlayStatus;
				}
				
				// the rew is pressed change to rev_seek
				mPlayStatus = STATUS_REV_SEEK;
			}
			checkPlayStatusChange(); // add this when player doesn't notify REW release
                        break;
                    case BTAvrcpProfile.AVRCP_POP_FAST_FORWARD:
                        sMsg = "FAST FORWARD";
			if( 0 == msg.arg2 ){ //release
				if( mPlayStatus == STATUS_FWD_SEEK ){
					Log.i( TAG, "[AVRCP] back to playing status from fwd_seek");
					mPlayStatus = mPreviousFFPlayStatus;
					Log.v( TAG, "[BT][AVRCP] update-info back mPlayStatus:" + Integer.toString(mPlayStatus));
				}
			}else{
				// before press the REW key, keep the old status
				if( mPlayStatus == STATUS_REV_SEEK || mPlayStatus == STATUS_FWD_SEEK ){
					mPreviousFFPlayStatus = STATUS_PLAYING;
				}else{
					mPreviousFFPlayStatus = mPlayStatus;
				}
			
				// the rew is pressed change to rev_seek
				mPlayStatus = STATUS_FWD_SEEK;
			}
			checkPlayStatusChange(); // add this when player doesn't notify FF release
                        break;
                    case BTAvrcpProfile.AVRCP_POP_EJECT:
                        sMsg = "EJECT";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_FORWARD:
                        sMsg = "FORWARD";
                        break;
                    case BTAvrcpProfile.AVRCP_POP_BACKWARD:
                        sMsg = "BACKWARD";
                        break;
                    default:
                        sMsg = String.format("KeyCode:%d", msg.arg1);
                        break;
                }
                // Toast.makeText( mContext, sMsg, Toast.LENGTH_SHORT).show();
            
                break;

            case mActionRegNotify:
                Log.i(TAG, String.format(
                        "[BT][AVRCP] ACTION_REG_NOTIFY for notifyChange msg.what:%d arg1:%d arg2:%d cardinality:%d",
                        msg.what, msg.arg1, msg.arg2, mRegBit.cardinality()));

                // bind the play service again
                // AVRCP V13 TC_TG_NFY

                synchronized (mRegBit) { // protect the RegBit
                    switch (msg.arg1) {
                        case BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED:
                            eventId = BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED;
                            if (mRegBit.get(eventId)) {
                                mAvrcpSrv.notificationPlayStatusChangedNative((byte) 0, (byte) 0, (byte) msg.arg1);
                                mRegBit.clear(eventId);
                            }
                            break;
                        case BTAvrcpProfile.EVENT_TRACK_CHANGED:
                            eventId = BTAvrcpProfile.EVENT_TRACK_CHANGED;
                            if (mRegBit.get(eventId)) {
                                mNotifySongId = sMusicId;
                                Log.i(TAG, "[BT][AVRCP] songid:" + mNotifySongId);
                                mAvrcpSrv.notificationTrackChangedNative((byte) 0, (byte) 0, mNotifySongId);
                                mRegBit.clear(eventId);
                            }

                            break;
                        case BTAvrcpProfile.EVENT_NOW_PLAYING_CONTENT_CHANGED:
                            eventId = BTAvrcpProfile.EVENT_NOW_PLAYING_CONTENT_CHANGED;
                            if (mRegBit.get(eventId)) {
                                mAvrcpSrv.notificationNowPlayingChangedNative((byte) 0, (byte) 0);
                                mRegBit.clear(eventId);
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;
            default:
                break;

        }
    }

    private void handleKeyMessageKeyEvent(Message msg) {
        if (null == mPlayService) {
            return;
        }

        try {

            switch (msg.arg1) {
                case BTAvrcpProfile.AVRCP_POP_PLAY:
                    
                        mPlayService.play();
                    
                    break;
                case BTAvrcpProfile.AVRCP_POP_STOP:
                    
                        mPlayService.stop();
                    
                    break;
                case BTAvrcpProfile.AVRCP_POP_PAUSE:
                    
                        mPlayService.pause();
                    
                    break;
                case BTAvrcpProfile.AVRCP_POP_FORWARD:
                    mPlayService.next();
                    break;
                case BTAvrcpProfile.AVRCP_POP_BACKWARD:
                    mPlayService.prev();
                    break;
                default:
                    Log.i(TAG, String.format("[BT][AVRCP] mPlayService Unhandle AvrcpKey:%d", msg.what));
                    break;
            }
        } catch (RemoteException ex) {
            Log.i(TAG, String.format("[BT][AVRCP] AVRCP fail to passToHandleMessage what:%d", msg.what));
        }
    }

    private BTAvrcpSystemListener mSystemListener = new BTAvrcpSystemListener(this) {
        @Override
        public void onBatteryStatusChange(int status) {
            Log.i(TAG, String.format("[BT][AVRCP] onBatteryStatusChange status:%d", status));
        }

        @Override
        public void onSystemStatusChange(int status) {
            Log.i(TAG, String.format("[BT][AVRCP] onSystemStatusChange status:%d", status));
        }

        public void onVolumeStatusChange(int volume) {
            Log.i(TAG, String.format("[BT][AVRCP] onSystemStatusChange volumn:%d", volume));
        }
    };

    public void startToBind() {
        // Log.i( TAG, "[BT][AVRCP][b] AVRCPMusicAdapter startToBind 4(a) " +
        // mbStartBind );
        // change to use startToBindPlayService, android native API
    }

    public void startToBindPlayService() {
        boolean bBindRet = false;
        if (null == mPlayService) {
            if (!sPlayServiceInterface) {
                return;
            }
            // when no permission to access the service. it may jump to catch{}
            sPlayServiceInterface = false;

            try {
                mAvrcpSrv.startService(new Intent("com.android.music.MediaPlaybackService"));
                bBindRet = mAvrcpSrv.bindService(new Intent("com.android.music.MediaPlaybackService"), mPlayConnection, 0);
                Log.i(TAG, String.format("[BT][AVRCP][b] startPlaybackService bBindRet:%b", bBindRet));
                sPlayServiceInterface = bBindRet;
                if (!bBindRet) {
                    Log.i(TAG, "[BT][AVRCP] mPlayService does not have play interface ");
                }
                mPlayStartBind = true;
            } catch (SecurityException ex) {
                sPlayServiceInterface = false; // no permission to bind
            }

            Log.i(TAG, "[BT][AVRCP] mbPlayServiceInterface is " + Boolean.toString(sPlayServiceInterface));
        }
    }

    public void startReceiver() {
        Log.i(TAG, "[BT][AVRCP][b] startReceiver");
        if (mStartReceiver) {
            Log.i(TAG, "[BT][AVRCP][b] startReceiver ignore");
            return;
        }

        Log.i(TAG, "[BT][AVRCP][b] startReceiver music intent");
        IntentFilter f = new IntentFilter();
        f.addAction(this.PLAYSTATE_CHANGED);
        f.addAction(this.META_CHANGED);
        f.addAction(this.QUIT_PLAYBACK);
        f.addAction(this.QUEUE_CHANGED);
        f.addAction(Intent.ACTION_SCREEN_ON);
        f.addAction(Intent.ACTION_SCREEN_OFF);
        mAvrcpSrv.registerReceiver(mStatusListener, new IntentFilter(f));
        mStartReceiver = true;
    }

    public void stopReceiver() {
        if (!mStartReceiver) {
            Log.i(TAG, "[BT][AVRCP][b] stopReceiver ignore");
            return;
        }
        Log.i(TAG, "[BT][AVRCP] startReceiver stop ");
        mAvrcpSrv.unregisterReceiver(mStatusListener);

        mStartReceiver = false;
    }

    public void stopToBind() {
        Log.i(TAG, "[BT][AVRCP][b] stopToBind");
        
        mStartBind = false;
        // Don't stop service. background music playing will stop if anyone
        // invoke stopService
        // avrcpSrv.stopService(new Intent(IBTAvrcpMusic.class.getName()));
        Log.i(TAG, "[BT][AVRCP][b] PlayService stopToBind");
        mAvrcpSrv.unbindService(mPlayConnection);
        mPlayStartBind = false;
    }

    private boolean mDebug = false;

	public static boolean hasStartReceiver(){
             return mStartReceiver;
	}

	private boolean sendHandlerMessageDelayed(int action, int arg1, int arg2, long delayMillis){
   	Message msg;
		//Log.i( TAG, "[BT][AVRCP] updatePlayerstatus send intent ");
		// send the intent to update the music metadata
		boolean result = false; 
		if( null != mContext  ){
			// send a message to itself
			if( null != mHandler ){
			msg = mHandler.obtainMessage();
			msg.what = action;

			msg.arg1 = arg1;
			msg.arg2 = arg2;

			result = mHandler.sendMessageDelayed(msg, delayMillis);
			if( false == result ){
				Log.e( TAG, "[BT][AVRCP] sendMessageDelayed fail ! ");
			}
			return result;
	   }
	  }
	  return result;
	}
	
	/**
	 * @brief update the playstatus from intent
	 * @param newPLayStatus got from player. its FWD_SEEK/REV_SEEK is not right
         * Search keyword is 'update-info'
	 */
	private static void updateNewPlayStatus(int newPlayStatus){
	    int before = mPlayStatus;
		  
	    Log.v( TAG, "[BT][AVRCP] updateNewPlayStatuso status:" + Integer.toString(mPlayStatus) + " newStatus:" + Integer.toString(newPlayStatus));
	    // since the intent maybe queued, the FF/REW status is detemined by AVRCP service
	    if( STATUS_FWD_SEEK == newPlayStatus || STATUS_REV_SEEK == newPlayStatus || STATUS_PLAYING == newPlayStatus){
    	        newPlayStatus = STATUS_PLAYING;
    		 
    	        // we only know the player is playing(playing/ff/rew status), only update it when not in special status (ff/rew)
    	        if( STATUS_FWD_SEEK != mPlayStatus && STATUS_REV_SEEK != newPlayStatus){
    	            mPlayStatus = newPlayStatus ;
    	        }
    	    }else{
    	        // pause or stop
    	        mPlayStatus = newPlayStatus ;
    	    }
    	
    	    if( before != mPlayStatus ){
    	        Log.v( TAG, "[BT][AVRCP] update-info new mPlayStatus:" + Integer.toString(mPlayStatus));
    	    }
	}
	
	private void checkPlayStatusChange(){
	    int eventId = BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED;
	    int status = 0;
    
	    status = getPlayerstatus();
	    if( mPreviousPlayStatus != status ){ // continusly update, notify only when change
                mPreviousPlayStatus = status;	                
                // force SE mw600 to show the song title
                eventId = BTAvrcpProfile.EVENT_TRACK_CHANGED;
                if( true != passNotifyMsg( eventId, status ) ){
                    Log.i( TAG, "[BT][AVRCP] onReceive EVENT_TRACK_CHANGED fail" );
                }

                eventId = BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED;
                if( true != passNotifyMsg( eventId, mPlayStatus ) ){
                    Log.i( TAG, "[BT][AVRCP] onReceive EVENT_PLAYBACK_STATUS_CHANGED fail" );
                }
            }		
        }

    public static void updateMusicTrackInfo(Intent intent) {
        String lsPlay;
        Boolean b1;
        Boolean b2;
        if (intent != null) {
            if (intent.getAction().equals(BTAvrcpMusicAdapter.META_CHANGED)) {
                sMusicArtist = intent.getStringExtra("artist");
                sMusicAlbum = intent.getStringExtra("album");
                sMusicTrack = intent.getStringExtra("track");

                sMusicId = intent.getLongExtra("id", (long) -1);
                Log.i(TAG, "[BT][AVRCP] update-info id:" + String.format("%d", sMusicId));
            }

            // only playstate change has the playing information
            if (intent.getAction().equals(BTAvrcpMusicAdapter.PLAYSTATE_CHANGED)) {
                // android music player (default)
                b1 = intent.getBooleanExtra("playing", false);
                // google music player (market)
                b2 = intent.getBooleanExtra("playstate", false);
                sMusicPlaying = b1 || b2;
                Log.i(TAG, "[BT][AVRCP] update-info playing:" + Boolean.toString(b1) + " " + Boolean.toString(b2));
            }

        }

        if (null != sMusicArtist) {
            Log.i(TAG, "[BT][AVRCP] track-info artist:" + sMusicArtist + " isPlaying:" + String.format("%b", sMusicPlaying)
                    + " id:" + Long.toString(sMusicId));
        } else {
            Log.i(TAG, "[BT][AVRCP] track-info isPlaying:" + String.format("%b", sMusicPlaying) + " id:"
                    + Long.toString(sMusicId));
        }

    }

    private BroadcastReceiver mStatusListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            int eventId = BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED;
            int status = 0;
            String action = intent.getAction();
            String mMuiscCmd;

            Log.i(TAG, "[BT][AVRCP] onReceive mStatusListener: " + action);
            if (action.equals(BTAvrcpMusicAdapter.META_CHANGED)) {
                // change song. redraw the artist/title info and
                updateMusicTrackInfo(intent);
                eventId = BTAvrcpProfile.EVENT_TRACK_CHANGED;

                if (!passNotifyMsg(eventId, status)) {
                    Log.i(TAG, "[BT][AVRCP] onReceive EVENT_TRACK_CHANGED fail");
                }
					
					// when using JB native music player and playing, clear all playlist. only has meta change event
					eventId = BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED;
					status = getPlayerstatus();
					if( true != passNotifyMsg( eventId, status ) ){
					    Log.i( TAG, "[BT][AVRCP] onReceive EVENT_PLAYBACK_STATUS_CHANGED fail" );
					}
            } else if (action.equals(BTAvrcpMusicAdapter.PLAYSTATE_CHANGED)) {
                updateMusicTrackInfo(intent);
	            
	                status = getPlayerstatus();
	                if( mPreviousPlayStatus != status ){
		                mPreviousPlayStatus = status;
                eventId = BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED;
                if (!passNotifyMsg(eventId, status)) {
                    Log.i(TAG, "[BT][AVRCP] onReceive EVENT_PLAYBACK_STATUS_CHANGED fail");
                }
                // SE mw600 need this to update the song name
                eventId = BTAvrcpProfile.EVENT_TRACK_CHANGED;
                if (!passNotifyMsg(eventId, status)) {
                    Log.i(TAG, "[BT][AVRCP] onReceive EVENT_TRACK_CHANGED fail");
                }
	                }

            } else if (action.equals(BTAvrcpMusicAdapter.QUIT_PLAYBACK)) {
                Log.i(TAG, "[BT][AVRCP] action equals BTAvrcpMusicAdapter.QUIT_PLAYBACK");
            } else if (action.equals(BTAvrcpMusicAdapter.QUEUE_CHANGED)) {
                updateMusicTrackInfo(intent);
                eventId = BTAvrcpProfile.EVENT_NOW_PLAYING_CONTENT_CHANGED;
                if (!passNotifyMsg(eventId, status)) {
                    Log.i(TAG, "[BT][AVRCP] onReceive EVENT_NOW_PLAYING_CONTENT_CHANGED fail");
                }
            } else if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                Log.i(TAG, "[BT][AVRCP] onReceive action equals ACTION_SCREEN_OFF");
            } else if (action.equals(Intent.ACTION_SCREEN_ON)) {
                Log.i(TAG, "[BT][AVRCP] onReceive action equals ACTION_SCREEN_ON");
            }
        }
    };

    /* connection */
    private ServiceConnection mPlayConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            int eventId;
            int interval = 0; /* use 0 to as pending event's interval value */
            boolean bReg = false;

            Log.i(TAG, String.format("[BT][AVRCP][b] PlayService onServiceConnected className:%s", className
                            .getClassName()));
            mPlayService = IMediaPlaybackService.Stub.asInterface(service);
        }

        public void onServiceDisconnected(ComponentName className) {
            Log.i(TAG, String.format("[BT][AVRCP][b] PlayService onServiceDisconnected className:%s", className
                    .getClassName()));
            if (mPlayService != null) {

                Log.i(TAG, String.format("[BT][AVRCP][b] unregistercallback "));
            }
            mPlayService = null;

        }
    };


    private AvrcpMusicAdapterStub mAdapterCallback = new AvrcpMusicAdapterStub(this);

    class AvrcpMusicAdapterStub extends IBTAvrcpMusicCallback.Stub {
        WeakReference<BTAvrcpMusicAdapter> mAdapter;

        AvrcpMusicAdapterStub(BTAvrcpMusicAdapter adapter) {
            mAdapter = new WeakReference<BTAvrcpMusicAdapter>(adapter);
        }

        public void notifyPlaybackStatus(byte status) {
            // use broadcase to handle this. android native aidl api
        }

        public void notifyTrackChanged(long id) {
            // use broadcase to handle this. android native aidl api
        }

        public void notifyTrackReachStart() {
            int eventId = BTAvrcpProfile.EVENT_TRACK_REACHED_START;
            synchronized (mRegBit) {
                if (!mRegBit.get(eventId)) {
                    return;
                }
                mRegBit.clear(eventId);
            }
            Log.i(TAG, String.format("[BT][AVRCP] callback notifyTrackReachStart "));
        }

        public void notifyTrackReachEnd() {
            int eventId = BTAvrcpProfile.EVENT_TRACK_REACHED_END;
            if (!mRegBit.get(eventId)) {
                return;
            }
            mRegBit.clear(eventId);
            Log.i(TAG, String.format("[BT][AVRCP] callback notifyTrackReachEnd "));
        }

        public void notifyPlaybackPosChanged() {
            int eventId = BTAvrcpProfile.EVENT_PLAYBACK_POS_CHANGED;
            if (!mRegBit.get(eventId)) {
                return;
            }
            mRegBit.clear(eventId);
            Log.i(TAG, String.format("[BT][AVRCP] callback notifyPlaybackPosChanged "));
        }

        public void notifyAppSettingChanged() {
            int eventId = BTAvrcpProfile.EVENT_PLAYER_APPLICATION_SETTING_CHANGED;
            if (!mRegBit.get(eventId)) {
                return;
            }
            mRegBit.clear(eventId);
            Log.i(TAG, String.format("[BT][AVRCP] callback notifyAppSettingChanged "));
        }

        public void notifyNowPlayingContentChanged() {
            // use broadcase to handle this. android native aidl api
        }

        public void notifyVolumehanged(byte volume) {
            int eventId = BTAvrcpProfile.EVENT_VOLUME_CHANGED;
            if (!mRegBit.get(eventId)) {
                return;
            }
            mRegBit.clear(eventId);
            Log.i(TAG, String.format("[BT][AVRCP] callback notifyVolumehanged "));
            mAvrcpSrv.notificationVolumeChangedNative((byte) 0, (byte) 0, volume);
        }

    }

}
