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


import android.app.ActionBar;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.StatFs;
import android.os.SystemClock;
import android.os.Trace;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnDismissListener;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.FMRadio.dialogs.NoAntennaDialog;
import com.mediatek.FMRadio.dialogs.SearchChannelsDialog;
import com.mediatek.FMRadio.ext.IProjectStringExt;
import com.mediatek.common.featureoption.FeatureOption;

import java.io.File;

/**
 * This class interact with user, provider FM basic function and FM recording
 * function
 *  
 */

public class FMRadioActivity extends Activity implements
        OnMenuItemClickListener, OnDismissListener,
        NoAntennaDialog.NoAntennaListener,
        SearchChannelsDialog.CancelSearchListener,
        FMRecordDialogFragment.OnRecordingDialogClickListener {
    
    public static final String TAG = "FmRx/Activity"; // log tag

    /*** dialog tags ***/
    // search dialog tag
    private static final String SEARCH = "Search";
    // save recording dialog tag
    private static final String SAVE_RECORDINGD = "SaveRecording";
    // RDS setting dialog tag
    private static final String RDS_SETTING = "RdsSetting";
    // no antenna dialog tag
    private static final String NO_ANTENNA = "NoAntenna";
    // use shared preference to store start recording time or start playback
    // time.
    private static final String REFS_NAME = "FMRecord";
    private static final String START_RECORD_TIME = "startRecordTime";
    private static final String START_PLAY_TIME = "startPlayTime";

    private static final int REQUEST_CODE_FAVORITE = 1;
    private int mPrevRecorderState = FMRecorder.STATE_INVALID;
    private int mCurrentStation = FMRadioUtils.DEFAULT_STATION;
    private int mRecordState = 0;
    
    // Timer delay 2 seconds.
    private static final long TOAST_TIMER_DELAY = 2000;
    private long mRecordStartTime = 0;
    private long mPlayStartTime = 0;

    private boolean mIsServiceStarted = false;
    private boolean mIsServiceBinded = false;
    private boolean mNeedTuneto = false;
    private boolean mIsNeedDisablePower = false;
    
    // When start, the radio is not playing.
    private boolean mIsPlaying = false;
    private boolean mIsInRecordingMode = false;
    private boolean mIsNeedShowRecordDlg = false;
    private boolean mIsNeedShowNoAntennaDlg = false;
    // ensure dialog show once
    private boolean mIsNeedShowSearchDlg = true;
    private boolean mIsActivityForeground = true;
    
    private FMRadioService mService = null;
    private Context mContext = null;
    private Toast mToast = null;
    private FragmentManager mFragmentManager = null;

    // station name text view
    private TextView mTextStationName = null;
    // station value text view
    private TextView mTextStationValue = null;
    // RDS text view
    private TextView mTextRDS = null;
    // Text view display "FM"
    private TextView mTextFM = null;
    // Text View display "MHZ"
    private TextView mTextMHz = null;
    
    private TextView mTxtRecInfoLeft = null;
    private TextView mTxtRecInfoRight = null;

    // decrease frequency button
    private ImageButton mButtonDecrease = null;
    // previous station button
    private ImageButton mButtonPrevStation = null;
    // next station button
    private ImageButton mButtonNextStation = null;
    // increase frequency button
    private ImageButton mButtonIncrease = null;
    // add to favorite button
    private ImageButton mButtonAddToFavorite = null;
    // FM record button
    private ImageButton mButtonRecord = null;
    // stop record and play record file button
    private ImageButton mButtonStop = null;
    // play FM record file button
    private ImageButton mButtonPlayback = null;

    // channel list menu item
    private MenuItem mMenuItemChannelList = null;
    // overflow menu item
    private MenuItem mMenuItemOverflow = null;
    // power menu
    private MenuItem mMenuItemPower = null;
    // popup menu
    private PopupMenu mPopupMenu = null;

    // layout display recording file information
    private RelativeLayout mRLRecordInfo = null;

    private Animation mAnimation = null;
    private ImageView mAnimImage = null;

    // Can not use float to record the station. Because there will be inaccuracy
    // when increase/decrease 0.1
    private AudioManager mAudioManager = null;
    private IProjectStringExt mProjectStringExt = null;

    private FMRadioListener mFMRadioListener = new FMRadioListener() {

        /**
         * call back method from service
         */
        public void onCallBack(Bundle bundle) {
            int flag = bundle.getInt(FMRadioListener.CALLBACK_FLAG);
            LogUtils.d(TAG, "call back method flag:" + flag);
            
            /*
             * if power down Fm, should remove all message first.
             */
            if (flag == FMRadioListener.MSGID_FM_EXIT) {
                mHandler.removeCallbacksAndMessages(null);
            }
            
            // remove tag message first, avoid too many same messages in queue.
            Message msg = mHandler.obtainMessage(flag);
            msg.setData(bundle);
            mHandler.removeMessages(flag);
            mHandler.sendMessage(msg);
        }
    };

    /**
     * list sd card unmount and mount when recording
     */
    private final BroadcastReceiver mSDListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            // porting 245343
            if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                LogUtils.d(TAG, "Sd card mounted");
                return;
            }

            // If not unmount recording sd card, do nothing;
            if (!isRecordingCardUnmount(intent)) {
                return;
            }

            if (Intent.ACTION_MEDIA_EJECT.equals(action)
                    || Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                FMRecordDialogFragment df = (FMRecordDialogFragment) mFragmentManager
                        .findFragmentByTag(SAVE_RECORDINGD);
                if ((null != df && df.getShowsDialog()) || mIsNeedShowRecordDlg) {
                    LogUtils.d(TAG, "recording sd card unmounted,dismiss save dialog");
                    dismissSaveRecordingDialog();
                    mIsNeedShowRecordDlg = false;
                    showToast(getString(R.string.toast_recording_lost_warning));
                }
                LogUtils.d(TAG, "Sd card Eject or unmounted");
            }
        }
    };

    /**
     * button click listeners on UI
     */
    private final View.OnClickListener mButtonClickListener = new View.OnClickListener() {

        /**
         * handle the event when view is clicked
         * 
         * @param v
         *            clicked view
         */
        public void onClick(View v) {
            switch (v.getId()) {
            case R.id.btn_record:
                // if power up, record FM, else toast prompt user to power up
                LogUtils.d(TAG, "btn record: CLICK!!");
//                mButtonRecord.setEnabled(false);
                if (null == mService) {
                    LogUtils.d(TAG, "mService is null");
                    return;
                }
                if (!mIsPlaying) {
                    showToast(getString(R.string.toast_powerup_before_record_warning));
                    return;
                }
                refreshRecordNotIdle();
                mRecordStartTime = SystemClock.elapsedRealtime();
                editSharedPreferences(START_RECORD_TIME, mRecordStartTime);
                mService.startRecordingAsync();
                mService.setModifiedRecordingName(null);
                break;

            case R.id.btn_stop:
                // stop record and stop play record file
                LogUtils.d(TAG, "btn stop: CLICK!!");
                if (null == mService) {
                    LogUtils.d(TAG, "mService is null");
                    return;
                }
                refreshRecordNotIdle();
                mService.stopRecordingAsync();
                mService.stopPlaybackAsync();
                break;

            case R.id.btn_playback:
                LogUtils.d(TAG, "btn playback: CLICK!!");
                if (null == mService) {
                    LogUtils.d(TAG, "mService is null");
                    return;
                }
                refreshRecordNotIdle();
                mPlayStartTime = SystemClock.elapsedRealtime();
                editSharedPreferences(START_PLAY_TIME, mPlayStartTime);
                mService.startPlaybackAsync();
                break;

            case R.id.button_add_to_favorite:
                LogUtils.d(TAG, "onClick AddToFavorite start");
                updateFavoriteStation();
                LogUtils.d(TAG, "onClick AddToFavorite end");
                break;

            case R.id.button_decrease:
                // for 100MHZ, decrease 0.1MHZ; for 50KHZ decrease 0.05MHZ
                tuneToStation(FMRadioUtils.computeDecreaseStation(mCurrentStation));
                break;

            case R.id.button_increase:
                // for 100MHZ, increase 0.1MHZ; for 50KHZ increase 0.05MHZ
                tuneToStation(FMRadioUtils.computeIncreaseStation(mCurrentStation));
                break;

            case R.id.button_prevstation:
                LogUtils.d(TAG, "onClick PrevStation");
                // Search for the previous station.
                seekStation(mCurrentStation, false); // false: previous station
                                                     // true: next station
                break;
            case R.id.button_nextstation:
                // Search for the next station.
                seekStation(mCurrentStation, true); // false: previous station
                                                    // true: next station
                break;

            default:
                LogUtils.d(TAG, "invalid view id");
                break;
            }
        }
    };
    
    /**
     * Update FM recording state with given state from FM service
     * 
     * @param recorderState
     *            recorder state
     * 
     */
    private void updateRecordingState(int recorderState) {
        mRecordState = recorderState;
        refreshRecordingStatus(recorderState);

        switch (recorderState) {
        case FMRecorder.STATE_RECORDING:
            showToast(getString(R.string.toast_start_recording));
            LogUtils.d(TAG, "updateRecordingState:startRecording");
            mHandler.sendEmptyMessage(FMRadioListener.MSGID_REFRESH);
            break;

        case FMRecorder.STATE_PLAYBACK:
            mHandler.sendEmptyMessage(FMRadioListener.MSGID_REFRESH);
            break;

        case FMRecorder.STATE_IDLE:
            LogUtils.d(TAG, "updateRecordingState:remove message");
            mHandler.removeMessages(FMRadioListener.MSGID_REFRESH);
            break;

            
        default:
            mHandler.removeMessages(FMRadioListener.MSGID_REFRESH);
            break;
        }
    }

    /**
     * Update FM recorder error with given error from FM service
     * 
     * @param errorType
     *            record error type
     * 
     */
    private void updateRecorderError(int errorType) {
        LogUtils.d(TAG, "updateRecorderError.errorType: " + errorType);
        String showString = null;
        // In FMRecorder.startRecording() error occurs,then we should set
        // mButtonRecord enable, because we set mButtonRecord disable ago
        refreshRecordIdle();
        switch (errorType) {
        case FMRecorder.ERROR_SDCARD_NOT_PRESENT:
            showString = getString(R.string.toast_sdcard_missing);
            break;

        case FMRecorder.ERROR_SDCARD_INSUFFICIENT_SPACE:
            showString = getString(R.string.toast_sdcard_insufficient_space);
            break;

        case FMRecorder.ERROR_RECORDER_INTERNAL:
            showString = getString(R.string.toast_recorder_internal_error);
            break;

        case FMRecorder.ERROR_PLAYER_INTERNAL:
            showString = getString(R.string.toast_player_internal_error);
            break;

        case FMRadioListener.NOT_AUDIO_FOCUS:
            showString = getString(R.string.not_available);
            if (isRecordFileExist()) {
                refreshPlaybackIdle(true);
            }
        default:
            LogUtils.d(TAG, "invalid recorder error");
            break;
        }

        showToast(showString);
    }

    /**
     * Update FM recorder mode with given mode from FM service
     * 
     * @param isInRecordingMode
     * 
     */
    private void exitRecordingMode(boolean isInRecordingMode) {
        refreshImageButton(mIsPlaying);
        refreshActionMenuItem(mIsPlaying);
        refreshPopupMenuItem(mIsPlaying);
        refreshActionMenuPower(true);
        if (!isInRecordingMode) {
            // Service has already set recording mode to false, need to modify
            // UI here
            mIsInRecordingMode = false;
            switchRecordLayout(isInRecordingMode);
        }
    }

    /**
     * Format the given time to be string by hour:minute:second
     * 
     * @param time
     *            time to be formated
     * @return string format from given time
     */
    private String getTimeString(int time) {
        final int oneHour = 3600;
        int hour = time / oneHour;
        final int minuteSecond = 60;
        int minute = (time / minuteSecond) % minuteSecond;
        int second = time % minuteSecond;
        String timeString = null;

        if (hour > 0) {
            final String timeFormatLong = "%02d:%02d:%02d";
            timeString = String.format(timeFormatLong, hour, minute, second);
        } else {
            final String timeFormatShort = "%02d:%02d";
            timeString = String.format(timeFormatShort, minute, second);
        }

        return timeString;
    }

    /**
    /**
     * update the favorite ui state
     * 
     */
    private void updateFavoriteStation() {
        String showString = null;
        // Judge the current output and switch between the devices.
        if (FMRadioStation.isFavoriteStation(mContext, mCurrentStation)) {
            // Need to delete this favorite channel.
            String stationName = FMRadioStation.getStationName(mContext,
                    mCurrentStation, FMRadioStation.STATION_TYPE_FAVORITE);
            FMRadioStation.updateStationToDB(mContext, stationName,
                    FMRadioStation.STATION_TYPE_SEARCHED, mCurrentStation);
            mButtonAddToFavorite
                    .setImageResource(R.drawable.btn_fm_favorite_off_selector);
            mTextStationName.setText("");
            showString = mProjectStringExt.getProjectString(mContext,
                    R.string.toast_channel_deleted,
                    R.string.toast_channel_deleted1);
        } else {
            // Add the station to favorite if the favorite list is not full.
            if (FMRadioStation.getStationCount(mContext,
                    FMRadioStation.STATION_TYPE_FAVORITE) >= FMRadioStation.MAX_FAVORITE_STATION_COUNT) {
                showString = mProjectStringExt.getProjectString(mContext,
                        R.string.toast_favorite_full,
                        R.string.toast_favorite_full1);
            } else {
                String stationName = FMRadioStation.getStationName(mContext,
                        mCurrentStation, FMRadioStation.STATION_TYPE_SEARCHED);
                if (FMRadioStation.isStationExist(mContext, mCurrentStation,
                        FMRadioStation.STATION_TYPE_SEARCHED)) {
                    FMRadioStation.updateStationToDB(mContext, stationName,
                            FMRadioStation.STATION_TYPE_FAVORITE,
                            mCurrentStation);
                } else {
                    FMRadioStation.insertStationToDB(mContext, stationName,
                            mCurrentStation,
                            FMRadioStation.STATION_TYPE_FAVORITE);
                }
                mButtonAddToFavorite
                        .setImageResource(R.drawable.btn_fm_favorite_on_selector);
                mTextStationName.setText(stationName);
                showString = mProjectStringExt.getProjectString(mContext,
                        R.string.toast_channel_added,
                        R.string.toast_channel_added1);
            }
        }
        showToast(showString);
        // When toast is showing, set the AddToFavorite button can not be click

//        mHandler.sendEmptyMessageDelayed(FMRadioListener.MSGID_SHOW_TOAST,
//                TOAST_TIMER_DELAY);
    }

    /**
     * edit value which saved in shared preference
     * 
     * @param key
     *            key
     * @param time
     *            value
     */
    private void editSharedPreferences(String key, long time) {
        SharedPreferences sharedPreferences = getSharedPreferences(REFS_NAME, 0);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putLong(key, time);
        editor.commit();
    }

    /**
     * when call bind service, it will call service connect. register call back
     * listener and initial device
     */
    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        /**
         * called by system when bind service
         * 
         * @param className
         *            component name
         * @param service
         *            service binder
         */
        public void onServiceConnected(ComponentName className, IBinder service) {
            LogUtils.d(TAG, "FMRadioActivity.onServiceConnected start");
            mService = ((FMRadioService.ServiceBinder) service).getService();
            if (null == mService) {
                LogUtils.e(TAG, "ServiceConnection: Error: can't get Service");
                finish();
                return;
            }

            mService.registerFMRadioListener(mFMRadioListener);
            if (mService.isServiceInit()) {
                LogUtils.d(TAG, "ServiceConnection: FM service is already init");
                if (mService.isDeviceOpen()) {
                    // tunetostation during changing language,we need to tune
                    // again when service bind success
                    if (mNeedTuneto) {
                        tuneToStation(mCurrentStation);
                        mNeedTuneto = false;
                    }
                    updateCurrentStation();
                    boolean isPlaying = mService.isPowerUp();
                    // back key destroy activity, mIsPlaying will be the default false.
                    // but it may be true. so the power button will be in wrong state.
                    mIsPlaying = isPlaying;
                    updateMenuStatus();
                    updateDialogStatus();

                    // check whether set play back button enable
                    if (!isRecordFileExist()) {
                        mButtonPlayback.setEnabled(false);
                    }
                    updateRds();

                    if (FeatureOption.MTK_FM_RECORDING_SUPPORT) {
                        restoreRecorderState();
                    }
                } else {
                    // Normal case will not come here
                    // ALPS01309754 Need to exit FM for this case
                    LogUtils.d(TAG, "ServiceConnection: service is exiting while start FM again, so exit again");
                    exitService();
                    finish();
                }
            } else {
                LogUtils.d(TAG, "ServiceConnection: FM service is not init");
                mService.initService(mCurrentStation);
                // Start init FM
                LogUtils.d(TAG, "ServiceConnection: call power up to service");
                powerUpFM();
            }
            
            LogUtils.d(TAG, "FMRadioActivity.onServiceConnected end");
        }

        /**
         * unbind service will call this method
         * 
         * @param className
         *            component name
         */
        public void onServiceDisconnected(ComponentName className) {
            LogUtils.d(TAG, "FMRadioActivity.onServiceDisconnected");
        }
    };

    /**
     * handler to update UI
     */
    private Handler mHandler = new Handler() {

        /**
         * handle the message of message queue, use to update UI
         * 
         * @param msg
         *            message in message queue
         */
        public void handleMessage(Message msg) {
            LogUtils.d(TAG, "mHandler.handleMessage: what = " + msg.what + ",hashcode:" + mHandler.hashCode());
            Bundle bundle;
            long endTime;
            switch (msg.what) {
            case FMRadioListener.MSGID_REFRESH:
                refreshTimeText();
                break;

            /**
             * hongen, call back method send message to handler
             */
            case FMRadioListener.MSGID_POWERUP_FINISHED:
                bundle = msg.getData();
                boolean isPowerup = bundle
                        .getBoolean(FMRadioListener.KEY_IS_POWER_UP);
                mIsPlaying = isPowerup;
                endTime = System.currentTimeMillis();
                Log.i(TAG, "[Performance test][FMRadio] power up end [" + endTime + "]");
                LogUtils.d(TAG, "updateFMState: FMRadio is powerup = " + isPowerup);
                Trace.traceCounter(Trace.TRACE_TAG_PERF, "AppUpdate", 1);
                stopAnimation();
                Trace.traceCounter(Trace.TRACE_TAG_PERF, "AppUpdate", 0);
                if (isPowerup) {
                    refreshImageButton(true);
                    refreshPopupMenuItem(true);
                    refreshActionMenuItem(true);
                } else {
                    showToast(getString(R.string.not_available));
                }
                Trace.traceCounter(Trace.TRACE_TAG_PERF, "AppUpdate", 1);
                // if not powerup success, refresh power to enable.
                refreshActionMenuPower(true);
                Trace.traceCounter(Trace.TRACE_TAG_PERF, "AppUpdate", 0);
                break;

            case FMRadioListener.MSGID_SWITCH_ANNTENNA:
                bundle = msg.getData();
                boolean isSwitch = bundle
                        .getBoolean(FMRadioListener.KEY_IS_SWITCH_ANNTENNA);
                LogUtils.d(TAG, "[FMRadioActivity.mHandler] swtich antenna: " + isSwitch);
                if (!isSwitch) {
                    if (mIsActivityForeground) {
                        dismissNoAntennaDialog();
                        showNoAntennaDialog();
                    } else {
                        LogUtils.d(TAG, "need show no antenna dialog after onResume:");
                        mIsNeedShowNoAntennaDlg = true;
                    }
                    stopAnimation();
                    // if not powerup success, refresh power to enable.
                    refreshActionMenuPower(true);
                } else {
                    mIsNeedShowNoAntennaDlg = false;
                    dismissNoAntennaDialog();
                }

                break;

            case FMRadioListener.MSGID_POWERDOWN_FINISHED:
                bundle = msg.getData();
                boolean isPowerdown = bundle
                        .getBoolean(FMRadioListener.KEY_IS_POWER_DOWN);
                mIsPlaying = !isPowerdown;
                endTime = System.currentTimeMillis();
                Log.i(TAG, "[Performance test][FMRadio] power down end [" + endTime + "]");
                refreshImageButton(false);
                refreshActionMenuItem(false);
                refreshPopupMenuItem(false);
                refreshActionMenuPower(true);

                break;
                
            case FMRadioListener.MSGID_TUNE_FINISHED:
                bundle = msg.getData();
                boolean tuneFinish = bundle.getBoolean(FMRadioListener.KEY_IS_TUNE);
                boolean isPowerUp = bundle.getBoolean(FMRadioListener.KEY_IS_POWER_UP);
                // when power down state, tune from channel list, will call back send mIsPowerup state.
                mIsPlaying = mIsPlaying ? mIsPlaying : isPowerUp;
                endTime = System.currentTimeMillis();
                Log.i(TAG, "[Performance test][FMRadio] increase frequency end [" + endTime + "]");
                Log.i(TAG, "[Performance test][FMRadio] decrease frequency end [" + endTime + "]");
                Log.i(TAG, "[Performance test][FMRadio] seek previous channel end [" + endTime + "]");
                Log.i(TAG, "[Performance test][FMRadio] seek next channel end [" + endTime + "]");
                // where?
                Log.i(TAG, "[Performance test][FMRadio] open channel end [" + endTime + "]");
                stopAnimation();
                // tune finished, should make poewer enable
                mIsNeedDisablePower = false;
                float frequency = bundle.getFloat(FMRadioListener.KEY_TUNE_TO_STATION);
                mCurrentStation = FMRadioUtils.computeStation(frequency);
                // After tune to station finished, refresh favorite button and
                // other button status.
                refreshStationUI(mCurrentStation);
                // tune fail,should resume button status
                if (!tuneFinish) {
                    LogUtils.d(TAG, "mHandler.tune: " + tuneFinish);
                    refreshActionMenuItem(mIsPlaying);
                    refreshImageButton(mIsPlaying);
                    refreshPopupMenuItem(mIsPlaying);
                    refreshActionMenuPower(true);
                    return;
                }
                refreshImageButton(true);
                refreshActionMenuItem(true);
                refreshPopupMenuItem(true);
                refreshActionMenuPower(true);
                
                break;

            case FMRadioListener.MSGID_SCAN_FINISHED:
                bundle = msg.getData();
                // cancel scan happen
                boolean isScan = bundle.getBoolean(FMRadioListener.KEY_IS_SCAN);
                int tuneToStation = bundle.getInt(FMRadioListener.KEY_TUNE_TO_STATION);
                int searchedNum = bundle.getInt(FMRadioListener.KEY_STATION_NUM);
                refreshActionMenuItem(mIsPlaying);
                refreshImageButton(mIsPlaying);
                refreshPopupMenuItem(mIsPlaying);
                // ebable action menu power items
                refreshActionMenuPower(true);

                if (!isScan) {
                    dismissSearchDialog();
                    LogUtils.d(TAG, "mHandler.scan canceled. not enter to channel list.");
                    return;
                }
                
                endTime = System.currentTimeMillis();
                Log.i(TAG, "[Performance test][FMRadio] scan channel end [" + endTime + "]");
                Log.i(TAG, "[Performance test][FMRadio] scan channel numbers [" + String.valueOf(searchedNum) + "]");
                
                mCurrentStation = tuneToStation;

                // After tune to station finished, refresh favorite button and
                // other button status.
                refreshStationUI(mCurrentStation);
                dismissSearchDialog();
                
                if (searchedNum == 0) {
                    showToast(getString(R.string.toast_cannot_search));
                    return;
                }

                enterChannelList();
                // Show toast to tell user how many stations have been searched
                showToast(getString(R.string.toast_channel_searched) + " " + String.valueOf(searchedNum));
                break;

            case FMRadioListener.MSGID_FM_EXIT:
                /*showToast("exit fm now.");*/
                finish();
                break;

            case FMRadioListener.LISTEN_RDSSTATION_CHANGED:
                bundle = msg.getData();
                int rdsStation = bundle.getInt(FMRadioListener.KEY_RDS_STATION);
                refreshStationUI(rdsStation);
                break;

            case FMRadioListener.LISTEN_PS_CHANGED:
            case FMRadioListener.LISTEN_RT_CHANGED:
                bundle = msg.getData();
                String text = "";
                String psString = bundle.getString(FMRadioListener.KEY_PS_INFO);
                String rtString = bundle.getString(FMRadioListener.KEY_RT_INFO);
                if ((null != psString) && (psString.length() > 0)) {
                    text += psString;
                }
                if ((null != rtString) && (rtString.length() > 0)) {
                    if (text.length() > 0) {
                        text += "  ";
                    }
                    text += rtString;
                }
                showRDS(text);
                break;

            /********* recording **********/
            case FMRadioListener.LISTEN_RECORDSTATE_CHANGED:
                bundle = msg.getData();
                int recorderState = bundle
                        .getInt(FMRadioListener.KEY_RECORDING_STATE);
                LogUtils.d(TAG, "FMRadioActivity.mHandler: recorderState = "
                        + recorderState);
                updateRecordingState(recorderState);
                break;

            case FMRadioListener.LISTEN_RECORDERROR:
                bundle = msg.getData();
                int errorState = bundle
                        .getInt(FMRadioListener.KEY_RECORDING_ERROR_TYPE);
                updateRecorderError(errorState);
                break;

            case FMRadioListener.LISTEN_RECORDMODE_CHANGED:
                bundle = msg.getData();
                boolean isInRecordingMode = bundle
                        .getBoolean(FMRadioListener.KEY_IS_RECORDING_MODE);
                exitRecordingMode(isInRecordingMode);
                break;
            default:
                LogUtils.d(TAG, "invalid message");
                break;
            }
            LogUtils.d(TAG, "handleMessage");
        }
    };

    /**
     * called when the activity is first created, initial variables
     * 
     * @param savedInstanceState
     *            saved bundle in onSaveInstanceState
     */
    public void onCreate(Bundle savedInstanceState) {
        long startTime = System.currentTimeMillis();
        Log.i(TAG, "[Performance test][FMRadio] onCreate start [" + startTime + "]");
        super.onCreate(savedInstanceState);
        LogUtils.i(TAG, "FMRadioActivity.onCreate start");

        mFragmentManager = getFragmentManager();
        // Bind the activity to FM audio stream.
        setVolumeControlStream(AudioManager.STREAM_FM);
        setContentView(R.layout.main);
        // Init UI Component,such as keeping button reference and set
        // textview--Kunpeng
        initUIComponent();
        // registerButtonClickListener--Kunpeng
        registerButtonClickListener();
        // ALPS01265381 fix this JE then move it to here
        // Because in record mode click back key will change to no
        registerSdcardReceiver();

        // put favorite button here since it might be used very early in
        // changing recording mode
        mCurrentStation = FMRadioStation.getCurrentStation(mContext);
        boolean isFavoriteStation = FMRadioStation.isFavoriteStation(mContext,
                mCurrentStation);
        // If the current station is in favorite, set its icon to favorite icon;
        // else, set to none favorite icon.
        if (isFavoriteStation) {
            mButtonAddToFavorite
                    .setImageResource(R.drawable.btn_fm_favorite_on_selector);
            mTextStationName.setText(FMRadioStation.getStationName(mContext,
                    mCurrentStation, FMRadioStation.STATION_TYPE_FAVORITE));
        } else {
            mButtonAddToFavorite
                    .setImageResource(R.drawable.btn_fm_favorite_off_selector);
        }

        mTextStationValue.setText(FMRadioUtils.formatStation(mCurrentStation));
        mAnimation = (Animation) AnimationUtils.loadAnimation(this,
                R.drawable.anim);
        mAnimImage = (ImageView) findViewById(R.id.iv_anim);
        mAnimImage.setVisibility(View.INVISIBLE);

        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

        // refreshButtonStatus(mIsPlaying);
        LogUtils.d(TAG, "FMRadioActivity.onCreate end");
        long endTime = System.currentTimeMillis();
        Log.i(TAG, "[Performance test][FMRadio] onCreate end [" + endTime + "]");
    }

    /**
     * go to channel list activity
     */
    private void enterChannelList() {
        LogUtils.d(TAG, "enterChannelList");
        if (mService == null) {
            LogUtils.d(TAG, "enterChannelList. mService is null");
        } else {
           // ALPS01250094, ams change the design for background start activity.
           // need check app is background in app code
           if (mService.isActivityForeGround()) {
               Intent intent = new Intent();
               intent.setClass(FMRadioActivity.this, FMRadioFavorite.class);
               startActivityForResult(intent, REQUEST_CODE_FAVORITE);
           } else {
               LogUtils.d(TAG, "enterChannelList. activity is background, not enter channel list.");
           }
        }
    }

    /**
     * Refresh the favorite button with the given station, if the station is
     * favorite station, show favorite icon, else show non-favorite icon.
     * 
     * @param station
     *            The station frequency
     */
    private void refreshStationUI(int station) {
        // Change the station frequency displayed.
        mTextStationValue.setText(FMRadioUtils.formatStation(station));
        // Show or hide the favorite icon
        if (FMRadioStation.isFavoriteStation(mContext, station)) {
            mButtonAddToFavorite
                    .setImageResource(R.drawable.btn_fm_favorite_on_selector);
            mTextStationName.setText(FMRadioStation.getStationName(mContext,
                    station, FMRadioStation.STATION_TYPE_FAVORITE));
        } else {
            mButtonAddToFavorite
                    .setImageResource(R.drawable.btn_fm_favorite_off_selector);
            mTextStationName.setText("");
        }
    }

    private void restoreConfiguration() {
        // after configuration change, need to reduction else the UI is abnormal
        if (null != getLastNonConfigurationInstance()) {
            LogUtils.d(TAG,
                    "Configration changes,activity restart,need to reset UI!");
            Bundle bundle = (Bundle) getLastNonConfigurationInstance();

            if (null == bundle) {
                return;
            }
            mPrevRecorderState = bundle.getInt("mPrevRecorderState");
            mRecordState = bundle.getInt("mRecordState");
            mIsNeedShowRecordDlg = bundle.getBoolean("mIsFreshRecordingStatus");
            mIsNeedShowNoAntennaDlg = bundle.getBoolean("mIsNeedShowNoAntennaDlg");
            mIsNeedShowSearchDlg = bundle.getBoolean("mIsNeedShowSearchDlg");
            // we doesn't get it from service because the service may be
            // null because not bind
            boolean isInRecordingMode = bundle.getBoolean("isInRecordingMode");
            mIsInRecordingMode = isInRecordingMode;
            LogUtils.d(TAG, "isInRecordingMode = " + isInRecordingMode
                    + ";mPrevRecorderState = " + mPrevRecorderState);
            mIsPlaying = bundle.getBoolean("mIsPlaying");
        }
    }

    /**
     * start and bind service, reduction variable values if configuration
     * changed
     */
    public void onStart() {
        super.onStart();
        LogUtils.d(TAG, "FMRadioActivity.onStart start");
        // Should start FM service first.

        if (null == startService(new Intent(FMRadioActivity.this,
                FMRadioService.class))) {
            LogUtils.e(TAG, "Error: Cannot start FM service");
            return;
        }

        mIsServiceStarted = true;
        mIsServiceBinded = bindService(new Intent(FMRadioActivity.this,
                FMRadioService.class), mServiceConnection,
                Context.BIND_AUTO_CREATE);

        if (!mIsServiceBinded) {
            LogUtils.e(TAG, "Error: Cannot bind FM service");
            finish();
            return;
        }

        restoreConfiguration();

        LogUtils.d(TAG, "FMRadioActivity.onStart end");
    }

    /**
     * refresh UI, when stop search, dismiss search dialog, pop up recording
     * dialog if FM stopped when recording in background
     */
    public void onResume() {
        super.onResume();
        LogUtils.d(TAG, "FMRadioActivity.onResume start");
        mIsActivityForeground = true;
        if (null == mService) {
            LogUtils.d(TAG, "service has not bind finished");
            return;
        }
        updateMenuStatus();
        updateDialogStatus();
        if (!isRecordFileExist()) {
            mButtonPlayback.setEnabled(false);
        }

        LogUtils.d(TAG, "FMRadioActivity.onResume end");
    }

    /**
     * when activity is paused call this method, indicate activity enter
     * background if press exit, power down FM
     */
    public void onPause() {
        LogUtils.d(TAG, "start FMRadioActivity.onPause");
        mIsActivityForeground = false;
        /**
         * should dismiss before call onSaveInstance, or it will resume automatic.
         * ALPS00529763
         */
        mIsNeedShowSearchDlg = true;
        dismissSearchDialog();

        /**
         * should dismiss before call onSaveInstance, or it will resume automatic.
         * ALPS01284803
         */
        FMRecordDialogFragment df = (FMRecordDialogFragment) mFragmentManager
                .findFragmentByTag(SAVE_RECORDINGD);
        if (null != df && df.getShowsDialog()) {
            LogUtils.d(TAG, "onPause.dismissSaveRecordingDialog()");
            if (mService != null) {
                mService.setModifiedRecordingName(df.getRecordingNameToSave());
            }
            dismissSaveRecordingDialog();
            mIsNeedShowRecordDlg = true;
        }

        LogUtils.d(TAG, "end FMRadioActivity.onPause");
        super.onPause();
    }

    /**
     * called when activity enter stopped state, unbind service, if exit
     * pressed, stop service
     */
    public void onStop() {
        LogUtils.d(TAG, "start FMRadioActivity.onStop");
        if (mIsServiceBinded) {
            unbindService(mServiceConnection);
            mIsServiceBinded = false;
        }
        // ALPS01197108 JE when start FM then quickly click home key(or phone's power key)
        if (mService != null) {
            /*
             * ALPS01006936
             * short antenna not support and earphone is not ready
             * HOME key or back key pressed. exit FM.
             */
            if (!FeatureOption.MTK_FM_SHORT_ANTENNA_SUPPORT 
                    && !mService.isAntennaAvailable() 
                    && mService.getRecorderState() != FMRecorder.STATE_PLAYBACK) {
                LogUtils.d(TAG, "onStop. short antenna not support,and earphone is not ok, exit FM.");
                exitService();
            }
        } else {
            LogUtils.e(TAG, "onStop. mService is not connected");
        }
        
        LogUtils.d(TAG, "end FMRadioActivity.onStop");
        super.onStop();
    }

    /**
     * when activity destroy, unregister broadcast receiver and remove handler
     * message
     */
    public void onDestroy() {
        LogUtils.d(TAG, "start FMRadioActivity.onDestroy");
        if (FeatureOption.MTK_FM_RECORDING_SUPPORT) {
            unregisterReceiver(mSDListener);
        }

        // need to call this function because if doesn't do this,after
        // configuration change will have many instance and recording time
        // or playing time will not refresh
        // Remove all the handle message
        mHandler.removeCallbacksAndMessages(null);
        // ALPS01197108 JE when start FM then quickly click home key(or phone's power key)
        if (mService != null) {
            mService.unregisterFMRadioListener(mFMRadioListener);
        } else {
            LogUtils.e(TAG, "onDestroy. mService is not connected");
        }
        // get the variable at last instance.
        // mService = null;
        mFMRadioListener = null;
        if (null != mPopupMenu) {
            mPopupMenu.dismiss();
            mPopupMenu = null;
        }
        LogUtils.d(TAG, "end FMRadioActivity.onDestroy");
        super.onDestroy();
    }

    /**
     * create options menu
     * 
     * @param menu
     *            option menu
     * @return true or false indicate need to handle other menu item
     */
    public boolean onCreateOptionsMenu(Menu menu) {
        LogUtils.d(TAG, "start FMRadioActivity.onCreateOptionsMenu");
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.fm_action_bar, menu);
        mMenuItemChannelList = menu.findItem(R.id.fm_channel_list);
        mMenuItemOverflow = menu.findItem(R.id.fm_menu);
        mMenuItemPower = menu.findItem(R.id.fm_power);

        LogUtils.d(TAG, "end FMRadioActivity.onCreateOptionsMenu");
        return true;
    }

    /**
     * prepare options menu
     * 
     * @param menu
     *            option menu
     * @return true or false indicate need to handle other menu item
     */
    public boolean onPrepareOptionsMenu(Menu menu) {
        LogUtils.d(TAG, "start FMRadioActivity.onPrepareOptionsMenu");

        if (!FeatureOption.MTK_FM_RECORDING_SUPPORT) {
            return true;
        }
        mMenuItemChannelList.setVisible(!mIsInRecordingMode);
        mMenuItemOverflow.setVisible(!mIsInRecordingMode);
        mMenuItemPower.setVisible(!mIsInRecordingMode);
        if (null == mService) {
            LogUtils.d(TAG, "mService is null");
            return !mIsInRecordingMode;
        }
        boolean isShortAntennaSupport = mService.isShortAntennaSupport();
        // if no short antenna support, refresh action item power enable.
        if (!isShortAntennaSupport) {
            LogUtils.d(TAG, "onPrepareOptionsMenu: no antenna support");
            refreshActionMenuPower(true);
            return true;
        }
        
        boolean isPlaying = mService.isPowerUp();
        boolean isMakePowerdown = mService.isMakePowerDown();
        boolean isSeeking = mService.isSeeking();
        boolean isScan = mService.isScanning();
        // if fm power down by other app, should enable power menu, make it to powerup.
        refreshActionMenuItem((isSeeking || isScan) ? false : isPlaying);
        refreshActionMenuPower((isScan || isSeeking)
                ? false
                : (isPlaying || (isMakePowerdown && !mIsNeedDisablePower)));

        // check whether show animation
        if (isSeeking) {
            LogUtils.d(TAG, "onPrepareOptionsMenu: it is seeking");
            startAnimation();
        }
        
        LogUtils.d(TAG, "end FMRadioActivity.onPrepareOptionsMenu");
        return !mIsInRecordingMode;
    }

    /**
     * handle event when option item selected
     * 
     * @param item
     *            clicked item
     * @return true or false indicate need to handle other menu item or not
     */
    public boolean onOptionsItemSelected(MenuItem item) {
        LogUtils.d(TAG, "start FMRadioActivity.onOptionsItemSelected");
        switch (item.getItemId()) {
        case android.R.id.home:
            onBackPressed();
            break;

        case R.id.fm_channel_list:
//            item.setEnabled(false);
            refreshImageButton(false);
            refreshActionMenuItem(false);
            refreshPopupMenuItem(false);
            refreshActionMenuPower(false);
            // Show favorite activity.
            enterChannelList();
            break;
            
        case R.id.fm_power:
            LogUtils.d(TAG, "click fm_power menu");
            if (mIsPlaying) {
                refreshImageButton(false);
                refreshActionMenuItem(false);
                refreshPopupMenuItem(false);
                refreshActionMenuPower(false);
                exitService();
                break;
            }
            powerUpFM();
            break;

        case R.id.fm_menu:
            item.setEnabled(false);
            
            mPopupMenu = new PopupMenu(mContext, findViewById(R.id.fm_menu));
            Menu menu = mPopupMenu.getMenu();
            mPopupMenu.getMenuInflater().inflate(R.menu.fm_menu, menu);
            mPopupMenu.setOnMenuItemClickListener(this);
            mPopupMenu.setOnDismissListener(this);
            // If record or RDS do not support,remove them from popup menu.
            boolean isFmViaBt = false;

            isFmViaBt = mService.isFmViaBt();
            if (!FeatureOption.MTK_FM_RECORDING_SUPPORT) {
                menu.findItem(R.id.fm_record).setVisible(false);
            }
            // When FM not playing or play over BT, prohibit recording

            boolean isPlaying = mService.isPowerUp();
            if (isPlaying && !isFmViaBt) {
                menu.findItem(R.id.fm_record).setEnabled(true);
                menu.findItem(R.id.fm_sound_mode).setEnabled(true);
                menu.findItem(R.id.fm_sound_mode).setTitle(
                        mService.isSpeakerUsed() ? R.string.optmenu_earphone 
                                : R.string.optmenu_speaker);
            }
            if (isPlaying) {
                menu.findItem(R.id.fm_search).setEnabled(true);
            }
            
            mPopupMenu.show();
            break;

        default:
            LogUtils.e(TAG, "Error: Invalid options menu item.");
            break;
        }
        LogUtils.d(TAG, "end FMRadioActivity.onOptionsItemSelected");
        return super.onOptionsItemSelected(item);
    }

    /**
     * whether antenna available
     * 
     * @return true or false indicate antenna available or not
     */
    private boolean isAntennaAvailable() {
        return FeatureOption.MTK_MT519X_FM_SUPPORT ? true : mAudioManager
                .isWiredHeadsetOn();
    }

    /**
     * on activity result, tune to station which is from channel list
     * 
     * @param requestCode
     *            request code
     * @param resultCode
     *            result code
     * @param data
     *            intent from channel list
     */
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (RESULT_OK == resultCode) {
            if (REQUEST_CODE_FAVORITE != requestCode) {
                LogUtils.e(TAG, "Error: Invalid requestcode.");
                return;
            }
            int iStation = data.getIntExtra(
                    FMRadioFavorite.ACTIVITY_RESULT, mCurrentStation);
            // Tune to this station.
            mCurrentStation = iStation;
            // if tune from channel list, we should disable power menu, especially for
            // power down state
            mIsNeedDisablePower = true;
            LogUtils.d(TAG, "onActivityForReult:" + mIsNeedDisablePower);
            if (null == mService) {
                LogUtils.d(TAG, "activity.onActivityResult mService is null");
                mNeedTuneto = true;
                return;
            }
            tuneToStation(iStation);
            return;
        }
        
        if (FMRadioStation.isFavoriteStation(mContext, mCurrentStation)) {
            mButtonAddToFavorite
                    .setImageResource(R.drawable.btn_fm_favorite_on_selector);
            mTextStationName.setText(FMRadioStation.getStationName(
                    mContext, mCurrentStation,
                    FMRadioStation.STATION_TYPE_FAVORITE));
        } else {
            mButtonAddToFavorite
                    .setImageResource(R.drawable.btn_fm_favorite_off_selector);
            mTextStationName.setText("");
        }

        // Do not handle other result.
        LogUtils.v(TAG, "The activity for requestcode " + requestCode
                + " does not return any data.");
    }

    /**
     * start animation
     */
    private void startAnimation() {
        mAnimImage.setAnimation(mAnimation);
        mAnimImage.setVisibility(View.VISIBLE);
        LogUtils.d(TAG, "FMRadioActivity.startAnimation end");
    }

    /**
     * stop animation
     */
    private void stopAnimation() {
        mAnimImage.setVisibility(View.INVISIBLE);
        mAnimImage.setAnimation(null);
    }

    /**
     * Restore recorder state from shared preference
     */
    private void restoreRecorderState() {
     // here should do some recorder related.
        mIsInRecordingMode = mService.getRecordingMode();
        mRecordState = mService.getRecorderState();
        RelativeLayout recInfoBar = (RelativeLayout) findViewById(R.id.rl_recinfo);
        // if recording or play backing state, should send message trigger refresh.
        if ((FMRecorder.STATE_RECORDING == mRecordState)
                || (FMRecorder.STATE_PLAYBACK == mRecordState)) {
            SharedPreferences sharedPreferences = getSharedPreferences(REFS_NAME, 0);
            mRecordStartTime = sharedPreferences.getLong(START_RECORD_TIME, 0);
            mPlayStartTime = sharedPreferences.getLong(START_PLAY_TIME, 0);
            recInfoBar.setVisibility(View.VISIBLE);
            LogUtils.d(TAG, "&&&sendemptyMessage:mRecoderStart:" + mRecordStartTime);
            mHandler.sendEmptyMessage(FMRadioListener.MSGID_REFRESH);
        } else {
            recInfoBar.setVisibility(View.GONE);
        }
        // if remove from app list, it will make recorder ui confused.
        switchRecordLayout(mIsInRecordingMode);
        changeRecordingMode(mIsInRecordingMode);
        if (mIsInRecordingMode) {
            refreshRecordingStatus(FMRecorder.STATE_INVALID);
        }
    }
    
    /**
     * play FM
     */
    private void powerUpFM() {
        LogUtils.v(TAG, "start powerUpFM");
        refreshImageButton(false);
        refreshActionMenuItem(false);
        refreshPopupMenuItem(false);
        refreshActionMenuPower(false);
        startAnimation();
        mService.powerUpAsync(FMRadioUtils.computeFrequency(mCurrentStation)); 
        LogUtils.v(TAG, "end powerUpFM");
    }

    private void setSpeakerPhoneOn(boolean isSpeaker) {
        if (isSpeaker) {
            LogUtils.v(TAG, "UseSpeaker");
            mService.setSpeakerPhoneOn(true);
            long endTime = System.currentTimeMillis();
            Log.i(TAG, "[Performance test][FMRadio] switch speaker end [" + endTime + "]");
        } else {
            LogUtils.v(TAG, "UseEarphone");
            mService.setSpeakerPhoneOn(false);
            long endTime = System.currentTimeMillis();
            Log.i(TAG, "[Performance test][FMRadio] switch earphone end [" + endTime + "]");
        }
        if (null != mPopupMenu) {
            Menu menu = mPopupMenu.getMenu();
            menu.findItem(R.id.fm_sound_mode).setTitle(
                    mService.isSpeakerUsed() ? R.string.optmenu_earphone : R.string.optmenu_speaker);
        }
    }

    /**
     * tune to station
     * 
     * @param station
     *            tune station
     */
    private void tuneToStation(final int station) {
        refreshImageButton(false);
        refreshActionMenuItem(false);
        refreshPopupMenuItem(false);
        refreshActionMenuPower(false);
        mService.tuneStationAsync(FMRadioUtils.computeFrequency(station));
        if (!mIsPlaying) {
            startAnimation();
        }
    }

    /**
     * seek station according current frequency and direction
     * 
     * @param station
     *            seek start station
     * @param direction
     *            seek direction
     */
    private void seekStation(final int station, boolean direction) {
        // If the seek AsyncTask has been executed and not canceled, cancel it
        // before start new.
        startAnimation();
        refreshImageButton(false);
        refreshActionMenuItem(false);
        refreshPopupMenuItem(false);
        refreshActionMenuPower(false);
        mService.seekStationAsync(FMRadioUtils.computeFrequency(station),
                direction);
    }
    
    /*****Fm Main UI******/
    private void refreshImageButton(boolean enabled) {
        mButtonDecrease.setEnabled(enabled);
        mButtonPrevStation.setEnabled(enabled);
        mButtonNextStation.setEnabled(enabled);
        mButtonIncrease.setEnabled(enabled);
    }
    
    // refresh action menu except power menu
    private void refreshActionMenuItem(boolean enabled) {
        // action menu
        if (null != mMenuItemChannelList) {
            // if power down by other app, should disable channelist list, over menu
            mMenuItemChannelList.setEnabled(enabled);
            mMenuItemOverflow.setEnabled(enabled);

        }
    }
    
    // refresh action menu only power menu
    private void refreshActionMenuPower(boolean enabled) {
        // action menu
        if (null != mMenuItemChannelList) {
            // if fm power down by other app, should enable this button to powerup.
            mMenuItemPower.setEnabled(enabled);
            mMenuItemPower.setIcon(mIsPlaying ? R.drawable.btn_fm_powerup_selector
                    : R.drawable.btn_fm_powerdown_selector);
        }
    }
    
    private void refreshPopupMenuItem(boolean enabled) {
        boolean isOverBT = false;
        if (null != mService) {
            isOverBT = mService.isFmViaBt();
        }
        if (null != mPopupMenu) {
            Menu menu = mPopupMenu.getMenu();
            menu.findItem(R.id.fm_record).setEnabled(!isOverBT && enabled);
            menu.findItem(R.id.fm_search).setEnabled(enabled);
            menu.findItem(R.id.fm_sound_mode).setEnabled(enabled);
        }
    }

    /********Fm Record UI*********/
    private void refreshRecordNotIdle() {
        mButtonRecord.setEnabled(false);
        mButtonPlayback.setEnabled(false);
        mButtonStop.setEnabled(false);
    }
    
    private void refreshRecordIdle() {
        mButtonRecord.setEnabled(true);
        mButtonPlayback.setEnabled(false);
        mButtonStop.setEnabled(false);
    }
    
    private void refreshPlaybackIdle(boolean btnPlayBack) {
        mButtonRecord.setEnabled(true);
        mButtonPlayback.setEnabled(btnPlayBack);
        mButtonStop.setEnabled(false);
    }
    
    private void refreshRecording() {
        mButtonRecord.setEnabled(false);
        mButtonPlayback.setEnabled(false);
        mButtonStop.setEnabled(true);
    }
    
    private void refreshPlaybacking() {
        mButtonRecord.setEnabled(false);
        mButtonPlayback.setEnabled(false);
        mButtonStop.setEnabled(true);
    }

    /**
     * called when back pressed
     */
    public void onBackPressed() {
        LogUtils.d(TAG, "begin FMRadioActivity.onBackPressed");

        if (FeatureOption.MTK_FM_RECORDING_SUPPORT && mIsInRecordingMode) {
            changeRecordingMode(false);
            if (null == mService) {
                LogUtils.d(TAG, "mService is null");
                return;
            }
            // no need consider power down and other situation
            boolean isPlaying = mService.isPowerUp();
            refreshImageButton(isPlaying);
            refreshPopupMenuItem(isPlaying);
            refreshActionMenuItem(isPlaying);
            refreshActionMenuPower(true);
            return;
        }

        // exit fm, disable all button
        if (!mIsPlaying && (null != mService) && !mService.isPowerUping()) {
            refreshImageButton(false);
            refreshActionMenuItem(false);
            refreshPopupMenuItem(false);
            refreshActionMenuPower(false);
            exitService();
            return;
        }
        
        super.onBackPressed();

        LogUtils.d(TAG, "end FMRadioActivity.onBackPressed");
    }

    private void showToast(CharSequence text) {
        if (null == mToast) {
            mToast = Toast.makeText(mContext, text, Toast.LENGTH_SHORT);
        }
        mToast.setText(text);
        mToast.show();
        LogUtils.v(TAG, "FMRadioActivity.showToast: toast = " + text);
    }

    private void showRDS(String text) {
        mTextRDS.setText(text);
        mTextRDS.setSelected(true);
        LogUtils.v(TAG, "FMRadioActivity.showRDS: RDS = " + text);
    }

    /**
     * change recording mode
     * 
     * @param recordingMode
     *            current recording mode
     */
    private void changeRecordingMode(boolean recordingMode) {
        LogUtils.d(TAG, "changeRecordingMode: " + recordingMode);
        if (mIsInRecordingMode == recordingMode) {
            LogUtils.e(TAG, "FM already " + (recordingMode ? "in" : "NOT in")
                    + "recording mode!");
            return;
        }
        mIsInRecordingMode = recordingMode;
        mService.setRecordingModeAsync(recordingMode);
        switchRecordLayout(recordingMode);
    }
    
    /**
     * switch to record layout, if in recorder mode.
     * @param recordingMode true in recorder mode, false not in recorder mode
     */
    private void switchRecordLayout(boolean recordingMode) {
        ActionBar actionBar = getActionBar();
        invalidateOptionsMenu();

        // Set the action bar on the right to be up navigation
        actionBar.setDisplayHomeAsUpEnabled(recordingMode);
        actionBar.setHomeButtonEnabled(recordingMode);
        actionBar.setTitle(recordingMode ? R.string.fm_recorder_name
                : R.string.app_name);

        LinearLayout recBar = (LinearLayout) findViewById(R.id.bottom_bar_recorder);
        LinearLayout bottomBar = (LinearLayout) findViewById(R.id.bottom_bar);

        bottomBar.setVisibility(recordingMode ? View.GONE : View.VISIBLE);
        recBar.setVisibility(recordingMode ? View.VISIBLE : View.GONE);
        mButtonAddToFavorite.setVisibility(recordingMode ? View.GONE
                : View.VISIBLE);
    }
    
    /**
     * update recording UI according record state
     * 
     * @param stateOverride
     *            recording state
     */
    private void refreshRecordingStatus(int stateOverride) {
        int recorderState = FMRecorder.STATE_INVALID;

        recorderState = (stateOverride == FMRecorder.STATE_INVALID ? mService
                .getRecorderState() : stateOverride);

        LogUtils.d(TAG, "refreshRecordingStatus: state=" + recorderState);
        switch (recorderState) {
        case FMRecorder.STATE_IDLE:
            long recordTime = mService.getRecordTime();
            if (recordTime > 0) {
                if (isRecordFileExist()) {
                    mButtonPlayback.setEnabled(true);
                }
                
                if (FMRecorder.STATE_RECORDING == mPrevRecorderState) {
                    LogUtils.d(TAG, "need show recorder dialog.mPrevRecorderState:" + mPrevRecorderState);
                    if (mIsActivityForeground) {
                        showSaveRecordingDialog();
                    } else {
                        mIsNeedShowRecordDlg = true;
                    }
                }
                
            } else {
                mButtonPlayback.setEnabled(false);
            }
            
            refreshPlaybackIdle((recordTime > 0) && isRecordFileExist());
            mRLRecordInfo.setVisibility(View.GONE);
            break;

        case FMRecorder.STATE_RECORDING:
            mTxtRecInfoLeft.setText("");
            mTxtRecInfoRight.setText("");
            mTxtRecInfoLeft.setSelected(false);
            refreshRecording();
            mRLRecordInfo.setVisibility(View.VISIBLE);
            break;

        case FMRecorder.STATE_PLAYBACK:
            String recordingName = mService.getRecordingName();
            if (null == recordingName) {
                recordingName = "";
            }
            mTxtRecInfoLeft.setText(recordingName);
            mTxtRecInfoRight.setText("");
            mTxtRecInfoLeft.setSelected(true);
            refreshPlaybacking();
            mRLRecordInfo.setVisibility(View.VISIBLE);
            break;

        case FMRecorder.STATE_INVALID:
            refreshRecordIdle();
            mRLRecordInfo.setVisibility(View.GONE);
            break;

        default:
            LogUtils.d(TAG, "invalid record status");
            break;
        }
        mPrevRecorderState = recorderState;
        LogUtils.d(TAG, "refreshRecordingStatus.mPrevRecorderState:" + mPrevRecorderState);
    }


    /*
     * Get the name in Save Dialog first then in Service
     * Because they may not the same for not click save
     */
    private String getRecordingShowName() {
        //Means the name showed in dialog
        String ret = null;
        String nameInDialog = mService.getModifiedRecordingName();
        String nameInService = mService.getRecordingName();
        ret = nameInDialog != null ? nameInDialog : nameInService;
        return ret;

    }

    /**
     * whether FM recording temporary file exist
     * 
     * @return true if FM recording temporary file exist, false not exist FM
     *         recording temporary file
     */
    private boolean isRecordFileExist() {
        String fileName = null;
        fileName = mService.getRecordingNameWithPath();
        // if recording file is delete by user, play button disabled
        File recordingFileToSave = new File(fileName + FMRecorder.RECORDING_FILE_EXTENSION);
        return recordingFileToSave.exists();
    }

    /**
     * whether have enough storage
     * 
     * @return true if have enough storage, false not have enough storage
     */
    private boolean isHaveAvailableStorage() {
        boolean ret = false;
        String sdcard = FMRadioService.getRecordingSdcard();
        try {
            StatFs fs = new StatFs(sdcard);
            long blocks = fs.getAvailableBlocks();
            long blockSize = fs.getBlockSize();
            long spaceLeft = blocks * blockSize;
            LogUtils.d(TAG, "checkRemainingStorage: available space=" + spaceLeft);
            ret = spaceLeft > FMRecorder.LOW_SPACE_THRESHOLD ? true : false;
        } catch (IllegalArgumentException e) {//ALPS01259807
            LogUtils.e(TAG, "sdcard may be unmounted:" + sdcard);
        }
        return ret;
    }

    // use onRetainNonConfigurationInstance because after configuration change,
    // activity will destroy and create
    // need use this function to save some important variables
    @Override
    public Object onRetainNonConfigurationInstance() {
        final int size = 5;
        Bundle bundle = new Bundle(size);
        boolean isInRecordingMode = false;
        bundle.putBoolean("isInRecordingMode", mIsInRecordingMode);
        bundle.putInt("mPrevRecorderState", mPrevRecorderState);
        bundle.putBoolean("mIsFreshRecordingStatus", mIsNeedShowRecordDlg);
        bundle.putBoolean("mIsNeedShowNoAntennaDlg", mIsNeedShowNoAntennaDlg);
        bundle.putBoolean("mIsNeedShowSearchDlg", mIsNeedShowSearchDlg);
        bundle.putInt("mRecordState", mRecordState);
        bundle.putBoolean("mIsPlaying", mIsPlaying);
        LogUtils.d(TAG, "onRetainNonConfigurationInstance() bundle:" + bundle);
        return bundle;
    }
    
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        LogUtils.d(TAG, "onSaveInstanceState");
        super.onSaveInstanceState(outState);
    }

    /**
     * handle event about pop up menu clicked
     * 
     * @param item
     *            pop up menu item
     * @return true or false indicate need to handle other menu item or not
     */
    public boolean onMenuItemClick(MenuItem item) {
        LogUtils.d(TAG, "onMenuItemClick:" + item.getItemId());
        switch (item.getItemId()) {

        case R.id.fm_search:
            mIsNeedShowSearchDlg = true;
            refreshImageButton(false);
            refreshActionMenuItem(false);
            refreshPopupMenuItem(false);
            refreshActionMenuPower(false);
            showSearchDialog();
            FMRadioStation.cleanSearchedStations(mContext);
            mService.startScanAsync();
            break;
           
        case R.id.fm_sound_mode:
            long startTime = System.currentTimeMillis();
            Log.i(TAG, "[Performance test][FMRadio] switch speaker start [" + startTime + "]");
            Log.i(TAG, "[Performance test][FMRadio] switch earphone start [" + startTime + "]");
            setSpeakerPhoneOn(!mService.isSpeakerUsed());
            break;
        case R.id.fm_record:
            if (FeatureOption.MTK_FM_RECORDING_SUPPORT) {
                changeRecordingMode(true);
                refreshRecordingStatus(FMRecorder.STATE_INVALID);
            }
            break;
        default:
            LogUtils.d(TAG, "invalid menu item");
            break;
        }
        return false;
    }

    /**
     * called when PopUp menu dismissed
     * 
     * @param PopUp
     *            menu which dismiss
     */
    public void onDismiss(PopupMenu menu) {
        LogUtils.d(TAG, "popmenu dismiss listener:" + menu);
        invalidateOptionsMenu();
    }

    /**
     * exit FM service
     */
    private void exitService() {
        LogUtils.i(TAG, "exitService");
        if (mIsServiceBinded) {
            unbindService(mServiceConnection);
            mIsServiceBinded = false;
        }

        if (mIsServiceStarted) {
            boolean isSuccess = stopService(new Intent(
                    FMRadioActivity.this, FMRadioService.class));
            if (!isSuccess) {
                LogUtils.e(TAG, "Error: Cannot stop the FM service.");
            }
            mIsServiceStarted = false;
        }
    }

    /**
     * show no antenna dialog
     */
    public void showNoAntennaDialog() {
        NoAntennaDialog newFragment = NoAntennaDialog.newInstance();
        newFragment.show(mFragmentManager, NO_ANTENNA);
        mFragmentManager.executePendingTransactions();

    }


    /**
     * show save recording dialog
     * 
     * @param name
     *            recording file name
     */
    public void showSaveRecordingDialog() {
        String sdcard = FMRadioService.getRecordingSdcard();
        String defaultName = mService.getRecordingName();
        String recordingName = mService.getModifiedRecordingName();
        FMRecordDialogFragment newFragment = new FMRecordDialogFragment(sdcard, defaultName, recordingName);
        newFragment.show(mFragmentManager, SAVE_RECORDINGD);
        mFragmentManager.executePendingTransactions();
    }

    /**
     * shou search dialog
     */
    public void showSearchDialog() {
        SearchChannelsDialog newFragment = SearchChannelsDialog.newInstance();
        newFragment.show(mFragmentManager, SEARCH);
        mFragmentManager.executePendingTransactions();
    }

    /**
     * dismiss search dialog
     */
    private void dismissSearchDialog() {
        SearchChannelsDialog newFragment = (SearchChannelsDialog) mFragmentManager
                .findFragmentByTag(SEARCH);
        if (null != newFragment) {
            newFragment.dismissAllowingStateLoss();
        }
    }

    /**
     * dismiss save recording dialog
     */
    private void dismissSaveRecordingDialog() {
        FragmentTransaction ft = mFragmentManager.beginTransaction();
        Fragment fragment = mFragmentManager.findFragmentByTag(SAVE_RECORDINGD);
        if (null != fragment) {
            ft.remove(fragment);
            ft.commitAllowingStateLoss();
        }
    }

    /**
     * whether recording card unmount
     * 
     * @param intent
     *            intent about sdcard
     * @return true or false indicate recording card unmount or not
     */
    public boolean isRecordingCardUnmount(Intent intent) {
        // ALPS01284803 start FM then unmount sdcard mService will be null
        String sdcard = FMRadioService.getRecordingSdcard();
        String unmountSDCard = intent.getData().toString();
        LogUtils.d(TAG, "unmount sd card file path: " + unmountSDCard);
        return unmountSDCard.equalsIgnoreCase("file://" + sdcard) ? true
                : false;
    }
    
    /**
     * dismiss no antenna dialog
     */
    private void dismissNoAntennaDialog() {
        NoAntennaDialog newFragment = (NoAntennaDialog) mFragmentManager
                .findFragmentByTag(NO_ANTENNA);
        if (null != newFragment) {
            newFragment.dismissAllowingStateLoss();
        }
    }

    /**
     * cancel search progress
     */
    public void cancelSearch() {
        LogUtils.d(TAG, "FMRadioActivity.cancelSearch");
        mService.stopScan();
    }

    /**
     * no antenna continue to operate
     */
    public void noAntennaContinue() {
        // We let user use the app if no antenna.
        // But we do not automatically start FM.
        LogUtils.d(TAG, " noAntennaContinue.onClick ok to continue");
        if (isAntennaAvailable()) {
            powerUpFM();
        } else {
            LogUtils.d(TAG, "noAntennaContinue.earphone is not ready");
            mService.switchAntennaAsync(1);
        }
    }

    /**
     * no antenna cancel to operate
     */
    public void noAntennaCancel() {
        LogUtils.d(TAG, " onClick Negative");
        exitService();
    }

    /**
     * recording dialog click
     * 
     * @param recordingName
     *            new recording name
     */
    public void onRecordingDialogClick(String recordingName) {
        mService.saveRecordingAsync(recordingName);
        mService.setModifiedRecordingName(null);
    }

    /**
     * update rds information
     */
    private void updateRds() {
        if (mIsPlaying) {
            Bundle bundle = new Bundle(2);
            bundle.putString(FMRadioListener.KEY_PS_INFO, mService.getPS());
            bundle.putString(FMRadioListener.KEY_RT_INFO, mService.getLRText());
            Message msg = mHandler
                    .obtainMessage(FMRadioListener.LISTEN_PS_CHANGED);
            msg.setData(bundle);
            mHandler.sendMessage(msg);
        }
    }

    /**
     * update current station according service station
     */
    private void updateCurrentStation() {
        // get the frequency from service, set frequency in activity, UI, database
        // same as the frequency in service
        int freq = mService.getFrequency();
        if (FMRadioUtils.isValidStation(freq)) {
            if (mCurrentStation != freq) {
                LogUtils.d(TAG,
                        "frequency in service isn't same as in database");
                mCurrentStation = freq;
                FMRadioStation.setCurrentStation(mContext, mCurrentStation);
                refreshStationUI(mCurrentStation);
            }
        }
    }
    
    /**
     * update button status, and dialog status
     */
    private void updateDialogStatus() {
        LogUtils.d(TAG, "updateDialogStatus.mIsNeedShowSearchDlg:" + mIsNeedShowSearchDlg);
        boolean isScan = mService.isScanning();
        // check whether show search dialog, because it may be dismissed onSaveInstance
        if (isScan && mIsNeedShowSearchDlg) {
            LogUtils.d(TAG, "updateDialogStatus: show search dialog. isScan is " + isScan);
            mIsNeedShowSearchDlg = false;
            showSearchDialog();
        }
        
        // check whether show recorder dialog, when activity is foreground
        if (mIsNeedShowRecordDlg) {
            LogUtils.d(TAG, "updateDialogStatus.reume recordDlg.mPrevRecorderState:" + mPrevRecorderState);
            showSaveRecordingDialog();
            mIsNeedShowRecordDlg = false;
        }
        
        // check whether show no antenna dialog, when activity is foreground
        if (mIsNeedShowNoAntennaDlg) {
            LogUtils.d(TAG, "updateDialogStatus.reume noAntennaDlg:");
            showNoAntennaDialog();
            refreshActionMenuPower(true);
            mIsNeedShowNoAntennaDlg = false;
        }
    }
    
    /**
     * update menu status, and animation
     */
    private void updateMenuStatus() {
        boolean isPlaying = mService.isPowerUp();
        boolean isPoweruping = mService.isPowerUping();
        boolean isSeeking = mService.isSeeking();
        boolean isScan = mService.isScanning();
        boolean isMakePowerdown = mService.isMakePowerDown();
        LogUtils.d(TAG, "updateMenuStatus.isSeeking:" + isSeeking);
        boolean fmStatus = (isScan || isSeeking || isPoweruping);
        // when seeking, all button should disabled,
        // else should update as origin status
        refreshImageButton(fmStatus ? false : isPlaying);
        refreshPopupMenuItem(fmStatus ? false : isPlaying);
        refreshActionMenuItem(fmStatus ? false : isPlaying);
        // if fm power down by other app, should enable power button
        // to powerup.
        LogUtils.d(TAG, "updateMenuStatus.mIsNeedDisablePower: " + mIsNeedDisablePower);
        refreshActionMenuPower(fmStatus ? false : (isPlaying || (isMakePowerdown && !mIsNeedDisablePower)));
   
        // check whether show animation
        if (isSeeking || isPoweruping) {
            LogUtils.d(TAG, "updateMenuStatus. it is seeking or poweruping");
            startAnimation();
        }
    }
    
    private void initUIComponent() {
        LogUtils.i(TAG, "initUIComponent");
        mContext = getApplicationContext();
        // Init FM database
        // Why need query data base in UI thread??--Kunpeng
        FMRadioStation.initFMDatabase(mContext);
        mProjectStringExt = ExtensionUtils.getExtension(mContext);
        mTextRDS = (TextView) findViewById(R.id.text_rds);
        mTextRDS.setText("");

        mTextFM = (TextView) findViewById(R.id.text_fm);
        mTextFM.setText("FM");

        mTextMHz = (TextView) findViewById(R.id.text_mhz);
        mTextMHz.setText("MHz");
        mTextStationValue = (TextView) findViewById(R.id.station_value);
        mTxtRecInfoLeft = (TextView) findViewById(R.id.txtRecInfoLeft);
        mTxtRecInfoRight = (TextView) findViewById(R.id.txtRecInfoRight);
        mRLRecordInfo = (RelativeLayout) findViewById(R.id.rl_recinfo);
        mButtonRecord = (ImageButton) findViewById(R.id.btn_record);
        mButtonStop = (ImageButton) findViewById(R.id.btn_stop);
        mButtonPlayback = (ImageButton) findViewById(R.id.btn_playback);
        mButtonAddToFavorite = (ImageButton) findViewById(R.id.button_add_to_favorite);
        mTextStationName = (TextView) findViewById(R.id.station_name);
        mButtonDecrease = (ImageButton) findViewById(R.id.button_decrease);
        mButtonIncrease = (ImageButton) findViewById(R.id.button_increase);
        mButtonPrevStation = (ImageButton) findViewById(R.id.button_prevstation);
        mButtonNextStation = (ImageButton) findViewById(R.id.button_nextstation);
        if (FeatureOption.MTK_FM_50KHZ_SUPPORT) {
            final int textSize = 50;
            mTextStationValue.setTextSize(textSize);
        }
        // initial mPopupMenu
        mPopupMenu = new PopupMenu(mContext, findViewById(R.id.fm_menu));
        Menu menu = mPopupMenu.getMenu();
        mPopupMenu.getMenuInflater().inflate(R.menu.fm_menu, menu);

    }

    private void registerButtonClickListener() {
        mButtonRecord.setOnClickListener(mButtonClickListener);
        mButtonStop.setOnClickListener(mButtonClickListener);
        mButtonPlayback.setOnClickListener(mButtonClickListener);
        mButtonAddToFavorite.setOnClickListener(mButtonClickListener);
        mButtonDecrease.setOnClickListener(mButtonClickListener);
        mButtonIncrease.setOnClickListener(mButtonClickListener);
        mButtonPrevStation.setOnClickListener(mButtonClickListener);
        mButtonNextStation.setOnClickListener(mButtonClickListener);
    }

    private void registerSdcardReceiver() {
        IntentFilter iFilter = new IntentFilter();
        iFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        iFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        iFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        iFilter.addDataScheme("file");
        registerReceiver(mSDListener, iFilter);
    }

    private void refreshTimeText() {
        LogUtils.d(TAG, "refreshTimeText:mRecordState:" + mRecordState);
        if (!mIsInRecordingMode) {
            LogUtils.d(TAG, "refreshTimeText:mIsInRecordingMode:" + mIsInRecordingMode);
            if (mRecordState == FMRecorder.STATE_RECORDING) {
                mService.stopRecordingAsync();
            } else if (mRecordState == FMRecorder.STATE_PLAYBACK) {
                mService.stopPlaybackAsync();
            }
            return;
        }
        
        final int oneSecond = 1000;
        switch (mRecordState) {
        case FMRecorder.STATE_RECORDING:
            int recordTime = (int) ((SystemClock.elapsedRealtime() - mRecordStartTime) / oneSecond);
            mTxtRecInfoLeft.setText(getTimeString(recordTime));
            LogUtils.d(TAG, "Recording time = " + mTxtRecInfoLeft.getText());
            if (!isHaveAvailableStorage()) {
                // Insufficient storage
                mService.stopRecordingAsync();
            }
            break;

        case FMRecorder.STATE_PLAYBACK:
            int playTime = (int) ((SystemClock.elapsedRealtime() - mPlayStartTime) / oneSecond);
            mTxtRecInfoRight.setText(getTimeString(playTime));
            LogUtils.d(TAG, "Playing time = " + mTxtRecInfoRight.getText());
            break;

        default:
            break;
        }
        mHandler.sendEmptyMessageDelayed(FMRadioListener.MSGID_REFRESH, oneSecond);
    }
}
