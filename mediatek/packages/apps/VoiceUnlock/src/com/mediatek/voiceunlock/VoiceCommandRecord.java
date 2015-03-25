/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.voiceunlock;

import java.text.NumberFormat;
import java.util.ArrayList;

import com.android.internal.telephony.ITelephony;
import com.android.internal.widget.LockPatternUtils;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.StatusBarManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.content.ServiceConnection;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.media.SoundPool;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.provider.Settings;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.android.internal.widget.LockPatternUtils;

import com.mediatek.voiceunlock.R;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandManagerService;

import com.mediatek.xlog.Xlog;
import com.mediatek.voiceunlock.VoiceUnlock.VoiceUnlockFragment;

public class VoiceCommandRecord extends PreferenceActivity {
    // required constructor for fragments
    public VoiceCommandRecord() {

    }

    @Override
    public Intent getIntent() {
        Intent modIntent = new Intent(super.getIntent());
        modIntent.putExtra(EXTRA_SHOW_FRAGMENT, VoiceCommandRecordFragment.class.getName());
        modIntent.putExtra(EXTRA_NO_HEADERS, true);
        return modIntent;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        CharSequence msg = getText(R.string.voice_unlock_setup_intro_header);
        showBreadCrumbs(msg, msg);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        if (VoiceCommandRecordFragment.class.getName().equals(fragmentName)) return true;
        return false;
    }

    public static class VoiceCommandRecordFragment extends SettingsPreferenceFragment
            implements View.OnClickListener {
        private static final int RED_PROGRESS_THRESHOLD = 40;
        private static final int YELLOW_PROGRESS_THRESHOLD = 100;

        private static final int MAX_PROGRESS = 100;

        private static final int START_ERROR_DIALOG_ID = 0;
        private static final int PLAY_BACK_DIALOG_ID = 1;
        private static final int TIMEOUT_DIALOG_ID = 2;
        private static final int DURING_CALL_DIALOG_ID = 3;

        private static final int COMMAND_COUNT = 4;

        private static final int TRAINING_RESULT_ENOUGH = 0;
        private static final int TRAINING_RESULT_NOT_ENOUGH = 1;
        private static final int TRAINING_RESULT_NOISY = 2;
        private static final int TRAINING_RESULT_WEAK = 3;
        private static final int TRAINING_RESULT_DIFF = 4;
        private static final int TRAINING_RESULT_EXIST = 5;
        private static final int TRAINING_RESULT_TIMEOUT = 6;
        private static final int TRAINING_RESULT_HEADSET_SWAP = 100;

        private static final int MSG_START_TRAINING = 0;
        private static final int MSG_UPDATE_INTENSITY = 1;
        private static final int MSG_UPDATE_NOTIFY = 2;
        private static final int MSG_SERVICE_ERROR = 3;

        private static final int MSG_SEND_INTENSITY_COMMAND = 0;

        private static final int MSG_PLAY_INDICATION = 0;

        private static final int INTENSITY_ANIMATION_INTERVAL = 90;

        private static final int RECORD_INTERVAL = 1000;

        private SoundPool mSounds;
        private int mSoundId;
        private int mSoundStreamId;
        private AudioManager mAudioManager;
        private int mMasterStreamType;

        private StatusBarManager mStatusBarManager;

        private TextView mCommandDescription;
        private ImageView mWave;
        private TextView mProgressText;
        private ProgressBar mProgressBar;
        private TextView mPrompt;
        private Button mFooterLeftButton;
        private Button mFooterRightButton;

        private ForegroundColorSpan mColorSpan;

        private Stage mUiStage = Stage.Introduction;
        private int mProgress = 0;
        private LockPatternUtils mLockPatternUtils;

        private String mCommandKey;
        private String mCommandValue;

        private NumberFormat mProgressPercentFormat;

        private IVoiceCommandManagerService mVCmdMgrService;
        private boolean isRegistered = false;

        private String mErrorMsg;

        private Handler mHandler;
        private Handler mIndicationHandler;
        private Handler mIntensityHandler;
        private Runnable mIntensityRunnable;

        private boolean mTraining;  //training is ongoing
        private boolean mCanceled;  //training is canceled
        private boolean mSkipPause;  //skip on pause when screen rotated
        private boolean mBindToService;

        private int mCurOrientation;

        private PowerManager mPM;
        private PowerManager.WakeLock mWakeLock;
        private Context mContext;
        private String mPkgName;

        private static final int FALLBACK_REQUEST = 101;//must be the same value with ChooseLockGeneric
        private static final String CONFIRM_CREDENTIALS = "confirm_credentials";//must be the same value with ChooseLockGeneric

        private IVoiceCommandListener mVoiceCallback = new IVoiceCommandListener.Stub(){
            public void onVoiceCommandNotified(int mainAction, int subAction, Bundle extraData)
                            throws RemoteException {
                            Message.obtain(mVoiceCommandHandler, mainAction, subAction, 0, extraData).sendToTarget();
             }
        };
        private Handler mVoiceCommandHandler = new Handler() {
            public void handleMessage(Message msg) {
                handleVoiceCommandNotified(msg.what, msg.arg1, (Bundle) msg.obj);
             }
        };
        public void handleVoiceCommandNotified(int mainAction, int subAction, Bundle extraData) {
            int result = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT);
            log("onNotified result=" + result + " mainAction = " + mainAction + " subAction = " + subAction);
            if (result == VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS) {
                switch (subAction) {
                case VoiceCommandListener.ACTION_VOICE_TRAINING_START:
                    log("onNotified TRAINING_START");
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TRAINING));
                    break;
                case VoiceCommandListener.ACTION_VOICE_TRAINING_INTENSITY:
                    int intensity = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                    log("onNotified TRAINING_INTENSITY intensity = " + intensity);
                    mHandler.removeMessages(MSG_UPDATE_INTENSITY);
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_UPDATE_INTENSITY, intensity, 0));
                    break;
                case VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY:
                    int resultId = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                    int progress = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                    log("onNotified TRAINING_NOTIFY progress = " + progress + " resultId = " + resultId);
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_UPDATE_NOTIFY, resultId, progress));
                    break;
                default:
                    break;
                }
            } else if (result == VoiceCommandListener.ACTION_EXTRA_RESULT_ERROR) {
                String errorMsg = extraData.getString(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                log("onNotified RESULT_ERROR errorMsg = " + errorMsg);
                mHandler.sendMessage(mHandler.obtainMessage(MSG_SERVICE_ERROR, errorMsg));
            }
        }

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            mLockPatternUtils = new LockPatternUtils(getActivity());
            mCommandKey = getActivity().getIntent().
                getStringExtra(LockPatternUtils.SETTINGS_COMMAND_KEY);
            mCommandValue = getActivity().getIntent()
                .getStringExtra(LockPatternUtils.SETTINGS_COMMAND_VALUE);

            mStatusBarManager = (StatusBarManager) getSystemService(Context.STATUS_BAR_SERVICE);

            mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
            mSounds = new SoundPool(1, AudioManager.STREAM_NOTIFICATION, 0);
            mSoundId = mSounds.load(getActivity(), R.raw.dock, 0);

            mPM = (PowerManager) getSystemService(Context.POWER_SERVICE);
            mWakeLock = mPM.newWakeLock(
                    PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
                    "VoiceUnlock");
            mWakeLock.setReferenceCounted(false);

            mCurOrientation = getResources().getConfiguration().orientation;
            mContext = getActivity().getBaseContext();
            mPkgName =  mContext.getPackageName();

            mHandler = new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    switch (msg.what) {
                        case MSG_START_TRAINING:
                            mBindToService = true;
                            playIndication(true);
                            break;
                        case MSG_UPDATE_INTENSITY:
                            updateIntensity(msg.arg1);
                            break;
                        case MSG_UPDATE_NOTIFY:
                            handleUpdateNotify(msg.arg1, msg.arg2);
                            break;
                        case MSG_SERVICE_ERROR:
                            mErrorMsg = (String) msg.obj;
                            showDialog(START_ERROR_DIALOG_ID);
                            break;
                        default:
                            break;
                    }
                }
            };

            mIntensityHandler = new Handler();
            mIndicationHandler = new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    switch (msg.what) {
                        case MSG_PLAY_INDICATION:
                            boolean first = msg.arg1 == 1 ? true : false;
                            playIndication(first);
                            break;

                        default:
                            break;
                    }
                }
            };

            mIntensityRunnable = new Runnable() {
                @Override
                public void run() {
                    if (mVCmdMgrService != null) {
                        log("sendCommand TRAINING_INTENSITY");
                        sendVoiceCommand(mPkgName,
                              VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                              VoiceCommandListener.ACTION_VOICE_TRAINING_INTENSITY, null);
                    }
                    mIntensityHandler.postDelayed(this, INTENSITY_ANIMATION_INTERVAL);
                }
            };

            mProgressPercentFormat = NumberFormat.getPercentInstance();
            mProgressPercentFormat.setMaximumFractionDigits(0);
            mColorSpan = new ForegroundColorSpan(getResources().getColor(android.R.color.holo_blue_light));

            log("onCreate mCommandKey = " + mCommandKey + " mCommandValue = " + mCommandValue);
        }

        @Override
        public void onPause() {
            super.onPause();
            log("onPause() mSkipPause = " + mSkipPause + " mTraining = " +
                    mTraining + " mCanceled = " + mCanceled);
            if (!mSkipPause) {
                stopVoiceCommandService();

                if (mTraining && !mCanceled) {
                    updateStage(Stage.Introduction);
                }
                stopUpdateIntensity();
                mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);
            } else {
                mSkipPause = false;
            }

        }

        @Override
        public void onResume() {
            super.onResume();
            log("onResume()");
            mSkipPause = false;
        }

        @Override
        public void onConfigurationChanged(Configuration newConfig) {
            super.onConfigurationChanged(newConfig);
            log("onConfigurationChanged newConfig.orientation = " + newConfig.orientation + " mCurOrientation = " + mCurOrientation);
            if (mCurOrientation != newConfig.orientation) {
                //donnot update stage to Introduction when orientation change,
                //let user continue recording when screen rotated
                log("onConfigurationChanged mSkipPause = true");
                mSkipPause = true;
                //detach and attach this fragment to reload its layout
                getFragmentManager().beginTransaction()
                    .detach(this)
                    .attach(this)
                    .commitAllowingStateLoss();
            }
            mCurOrientation = newConfig.orientation;
            log("onConfigurationChanged mCurOrientation = " + mCurOrientation);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            log("onCreateView");
            View view = inflater.inflate(R.layout.voice_command_record, null);
            mCommandDescription = (TextView)view.findViewById(R.id.command_description);
            mWave = (ImageView)view.findViewById(R.id.wave);
            mWave.setImageResource(com.mediatek.internal.R.drawable.voice_wave);
            mProgressText = (TextView)view.findViewById(R.id.progress_text);
            mProgressBar = (ProgressBar)view.findViewById(R.id.progress_bar);
            mPrompt = (TextView)view.findViewById(R.id.prompt);
            mFooterLeftButton = (Button)view.findViewById(R.id.footer_left_button);
            mFooterLeftButton.setOnClickListener(this);
            mFooterRightButton = (Button)view.findViewById(R.id.footer_right_button);
            mFooterRightButton.setOnClickListener(this);

            setCommandDescription();

            updateStage(mUiStage);
            setTrainingProgress(mProgress);
            return view;
        }

        @Override
        public Dialog onCreateDialog(int dialogId) {
            switch (dialogId) {
            case START_ERROR_DIALOG_ID:
                return new AlertDialog.Builder(getActivity())
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(R.string.voice_service_error_title)
                    .setCancelable(false)
                    .setMessage(mErrorMsg)
                    .setPositiveButton(R.string.voice_unlock_ok_label, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            getActivity().finish();
                        }
                    })
                    .create();
            case PLAY_BACK_DIALOG_ID:
                return new AlertDialog.Builder(getActivity())
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle(R.string.stop_playing_title)
                .setMessage(R.string.stop_playing_message)
                .setPositiveButton(R.string.voice_unlock_ok_label, null)
                .create();
            case TIMEOUT_DIALOG_ID:
                return new AlertDialog.Builder(getActivity())
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle(R.string.time_out_title)
                .setCancelable(false)
                .setMessage(R.string.time_out_message)
                .setPositiveButton(R.string.voice_unlock_ok_label, new AlertDialog.OnClickListener() {
                    public void onClick(DialogInterface arg0, int arg1) {
                        updateStage(Stage.Introduction);
                    }
                }).create();
            case DURING_CALL_DIALOG_ID:
                return new AlertDialog.Builder(getActivity())
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle(R.string.during_call_title)
                .setMessage(R.string.during_call_message)
                .setPositiveButton(R.string.voice_unlock_ok_label, null)
                .create();
            default :
                return super.onCreateDialog(dialogId);
            }
        }

        private void handleUpdateNotify(int resultId, int progress) {
            switch (resultId) {
            case TRAINING_RESULT_ENOUGH:
                updateStage(Stage.RecordingOK);
                break;
            case TRAINING_RESULT_NOT_ENOUGH:
                updateStage(Stage.OneRoundOK);
                break;
            case TRAINING_RESULT_NOISY:
                updateStage(Stage.OneRoundNoisy);
                break;
            case TRAINING_RESULT_WEAK:
                updateStage(Stage.OneRoundWeak);
                break;
            case TRAINING_RESULT_DIFF:
                updateStage(Stage.OneRoundDiff);
                break;
            case TRAINING_RESULT_EXIST:
                updateStage(Stage.OneRoundExist);
                break;
            case TRAINING_RESULT_TIMEOUT:
                stopUpdateIntensity();
                stopVoiceCommandService();
                showDialog(TIMEOUT_DIALOG_ID);
                break;
            case TRAINING_RESULT_HEADSET_SWAP:
                getActivity().finish();
                break;
            default:
                break;
            }
            setTrainingProgress(progress);
        }


        private void setCommandDescription() {
            String commandKey = getActivity().getIntent().getStringExtra(LockPatternUtils.SETTINGS_COMMAND_KEY);
            if (commandKey != null) {
                if (commandKey.equals(Settings.System.VOICE_UNLOCK_SCREEN)) {
                    mCommandDescription.setText(R.string.voice_command_record_description_unlock_screen);
                } else {
                    ComponentName cn = ComponentName.unflattenFromString(mCommandValue);
                    ActivityInfo info;
                    CharSequence name = "";
                    try {
                        info = getPackageManager().getActivityInfo(cn,
                                PackageManager.GET_SHARED_LIBRARY_FILES);
                        name = info.loadLabel(getPackageManager());
                    } catch (NameNotFoundException e) {
                        log("Cann't get app activityInfo via mCommandValue");
                    }
                    mCommandDescription.setText(getActivity().getString(R.string.voice_command_record_description_command, name));
                }
            }
        }

        private void updateIntensity(int intensity) {
            log("updateIntensity intensity = " + intensity);
            intensity -= 200;  //we don't want voice wave too sensitive
            if (intensity < 128) {
                log("updateIntensity 0");
                mWave.setImageLevel(0);
            } else if (intensity < 256) {
                log("updateIntensity 1");
                mWave.setImageLevel(1);
            } else if (intensity < 512) {
                log("updateIntensity 2");
                mWave.setImageLevel(2);
            } else if (intensity < 1024) {
                log("updateIntensity 3");
                mWave.setImageLevel(3);
            } else if (intensity < 2048) {
                log("updateIntensity 4");
                mWave.setImageLevel(4);
            }
        }

        private int getCommandId() {
            if (mCommandKey.equals(Settings.System.VOICE_UNLOCK_AND_LAUNCH1)) {
                return 1;
            } else if (mCommandKey.equals(Settings.System.VOICE_UNLOCK_AND_LAUNCH2)) {
                return 2;
            } else if (mCommandKey.equals(Settings.System.VOICE_UNLOCK_AND_LAUNCH3)) {
                return 3;
            } else {
                return 0;
            }
        }

        private int getAvailableCommand() {
            int cmdSet = 0;

            String voice_command3_app = Settings.System.getString(getContentResolver(),
                    Settings.System.VOICE_UNLOCK_AND_LAUNCH3);
            if (voice_command3_app != null) {
                cmdSet += 1;
            }
            cmdSet = cmdSet << 1;

            String voice_command2_app = Settings.System.getString(getContentResolver(),
                    Settings.System.VOICE_UNLOCK_AND_LAUNCH2);
            if (voice_command2_app != null) {
                cmdSet += 1;
            }
            cmdSet = cmdSet << 1;

            String voice_command1_app = Settings.System.getString(getContentResolver(),
                    Settings.System.VOICE_UNLOCK_AND_LAUNCH1);
            if (voice_command1_app != null) {
                cmdSet += 1;
            }
            cmdSet = cmdSet << 1;

            String voice_unlock_screen = Settings.System.getString(getContentResolver(),
                    Settings.System.VOICE_UNLOCK_SCREEN);
            if (voice_unlock_screen != null) {
                cmdSet += 1;
            }
            return cmdSet;
        }

        private void playIndication(final boolean first) {
            log("playIndication first = " + first + " mBindToService = " + mBindToService);
            if (mBindToService) { //To fix ALPS00441146, don't post any runnable after we unbind from service.
                if (first) {
                    updateStage(Stage.FirstRecording);
                } else {
                    updateStage(Stage.NonFirstRecording);
                }
                log("start to update Intensity");
                mIntensityHandler.postDelayed(mIntensityRunnable, 1200);
                playSound();
            }
        }

        private void stopUpdateIntensity() {
            mIntensityHandler.removeCallbacks(mIntensityRunnable);
            mWave.setImageLevel(0);
        }

        private void playSound() {
            mSounds.stop(mSoundStreamId);

            if (mAudioManager != null) {
                mMasterStreamType = mAudioManager.getMasterStreamType();
            }
            // If the stream is muted, don't play the sound
            if (mAudioManager.isStreamMute(mMasterStreamType)) return;

            mSoundStreamId = mSounds.play(mSoundId,
                    1, 1, 1/*priortiy*/, 0/*loop*/, 1.0f/*rate*/);
        }

        private boolean checkPlayback() {
            boolean isPlaying = AudioSystem.isStreamActive(AudioSystem.STREAM_MUSIC, 0)
                                || AudioSystem.isStreamActive(AudioSystem.STREAM_FM, 0);
            if (isPlaying) {
                showDialog(PLAY_BACK_DIALOG_ID);
            }
            return isPlaying;
        }

        private boolean phoneIsInUse() {
            boolean phoneInUse = true;
            try {
                ITelephony phone = ITelephony.Stub.asInterface(ServiceManager.checkService("phone"));
                if (phone != null) {
                    phoneInUse = !phone.isIdle();
                }
            } catch (RemoteException e) {
                Log.w(VoiceUnlock.TAG, "phone.isIdle() failed", e);
            }
            if (phoneInUse) {
                showDialog(DURING_CALL_DIALOG_ID);
            }
            return phoneInUse;
        }

        private void disableNotification(boolean disabled) {
            log("disableNotification disabled = " + disabled);
            int flags = StatusBarManager.DISABLE_NONE;
            if (disabled) {
                flags |= StatusBarManager.DISABLE_NOTIFICATION_ALERTS;
            }
            mStatusBarManager.disable(flags);
        }

        private void stopVoiceCommandService() {
            if (mVCmdMgrService != null) {
                log("sendCommand TRAINING_STOP");
                sendVoiceCommand(mPkgName,
                        VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                        VoiceCommandListener.ACTION_VOICE_TRAINING_STOP, null);

                log("unregister to service");
                unregisterVoicecommand(mPkgName);
                mBindToService = false;
            }
            disableNotification(false);
            mWakeLock.release();
        }

        private void startVoiceCommandService() {
            log("register to service");
            if (mVCmdMgrService == null) {
                bindVoiceService(mContext);
            } else {
                registerVoiceCommand(mPkgName);
            }
        }

        private void voiceTrainingStart() {
            Bundle extra = new Bundle();
            int commandId = getCommandId();
            int availableCmd = getAvailableCommand();
            int[] cmds = {availableCmd, COMMAND_COUNT};
            log("sendCommand TRAINING_START commandId = " + commandId
                    + " availableCmd = "  + availableCmd + " COMMAND_COUNT = "
                    + COMMAND_COUNT);
            extra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, commandId);
            extra.putIntArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, cmds);
            sendVoiceCommand(mPkgName,
                    VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                    VoiceCommandListener.ACTION_VOICE_TRAINING_START, extra);

            disableNotification(true);
            mWakeLock.acquire();
        }

        public void onClick(View v) {
            if (v == mFooterLeftButton) {
                if (mUiStage.leftMode.equals(LeftButtonMode.Cancel)) {
                    // Canceling, so finish all
                    mCanceled = true;
                  getActivity().finish();
                }

            } else if (v == mFooterRightButton) {
                log("click on right button ");
                if (mUiStage.rightMode.equals(RightButtonMode.Record)) {
                    if (!phoneIsInUse() && !checkPlayback()) {
                        updateStage(Stage.Prepare);
                    }
                } else if (mUiStage.rightMode.equals(RightButtonMode.Retry)) {
                    stopUpdateIntensity();
                    stopVoiceCommandService();
                    updateStage(Stage.Introduction);
                } else if (mUiStage.rightMode.equals(RightButtonMode.Continue)) {
                    log("mode = Continue mLockPatternUtils.getVoiceUnlockFallbackSet() = "
                            + mLockPatternUtils.getVoiceUnlockFallbackSet());
                    if (!mLockPatternUtils.getVoiceUnlockFallbackSet()) {
                        Intent intent = new Intent();
                        intent.setClassName("com.android.settings", "com.android.settings.ChooseLockGeneric");
                        intent.putExtra(CONFIRM_CREDENTIALS, false);
                        intent.putExtra(LockPatternUtils.LOCKSCREEN_WEAK_FALLBACK, true);
                        intent.putExtra(LockPatternUtils.LOCKSCREEN_WEAK_FALLBACK_FOR, LockPatternUtils.VOICE_UNLOCK);
                        intent.putExtra(LockPatternUtils.SETTINGS_COMMAND_KEY, mCommandKey);
                        intent.putExtra(LockPatternUtils.SETTINGS_COMMAND_VALUE, mCommandValue);
                        startActivityForResult(intent, FALLBACK_REQUEST);
                    } else {
                        if (mCommandKey != null) {
                            Settings.System.putString(getContentResolver(), mCommandKey, mCommandValue);
                        }
                    }
                    getActivity().finish();
                }
            }
        }

        private void setTrainingProgress(int progress) {
            mProgress = progress;
            float p = ((float)progress) / MAX_PROGRESS;
            mProgressText.setText(mProgressPercentFormat.format(p));

            if (progress < RED_PROGRESS_THRESHOLD) {
                mProgressBar.setProgressDrawable(getResources().getDrawable(R.drawable.voice_training_progress_red));
            } else if (progress >= RED_PROGRESS_THRESHOLD
                    && progress < YELLOW_PROGRESS_THRESHOLD) {
                mProgressBar.setProgressDrawable(getResources().getDrawable(R.drawable.voice_training_progress_yellow));
            } else {
                mProgressBar.setProgressDrawable(getResources().getDrawable(R.drawable.voice_training_progress_green));
            }
            mProgressBar.setProgress(progress);
        }

        private void updateStage(Stage stage) {
            log("updateStage stage = " + stage.toString());
            mUiStage = stage;
            mFooterLeftButton.setEnabled(stage.leftMode.enabled);
            mFooterLeftButton.setText(stage.leftMode.text);
            mFooterRightButton.setEnabled(stage.rightMode.enabled);
            mFooterRightButton.setText(stage.rightMode.text);
            switch (mUiStage) {
            case Introduction:
                mPrompt.setTextColor(getResources().getColor(android.R.color.primary_text_dark));
                setTrainingProgress(0);
                stopUpdateIntensity();
              //TODO stop recording service if exist
                break;
            case Prepare :
                mPrompt.setTextColor(getResources().getColor(android.R.color.primary_text_dark));
                startVoiceCommandService();
                break;

            case FirstRecording:
            case NonFirstRecording:
                mTraining = true;
                mPrompt.setTextColor(getResources().getColor(android.R.color.primary_text_dark));
                break;
            case RecordingOK:
                mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);
                mPrompt.setTextColor(getResources().getColor(android.R.color.primary_text_dark));
                mTraining = false;
                stopUpdateIntensity();
                break;
            case OneRoundExist:
                mPrompt.setTextColor(getResources().getColor(android.R.color.holo_red_dark));
                mTraining = false;
                stopUpdateIntensity();
                break;
            case OneRoundOK:
                mPrompt.setTextColor(getResources().getColor(android.R.color.holo_blue_dark));
                mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);
                mIndicationHandler.sendMessageDelayed(mIndicationHandler.obtainMessage(MSG_PLAY_INDICATION, 0, 0), RECORD_INTERVAL);
                stopUpdateIntensity();
                break;
            case OneRoundNoisy:
            case OneRoundWeak:
            case OneRoundDiff:
                mPrompt.setTextColor(getResources().getColor(android.R.color.holo_red_dark));
                mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);
                mIndicationHandler.sendMessageDelayed(mIndicationHandler.obtainMessage(MSG_PLAY_INDICATION, 0, 0), RECORD_INTERVAL);
                stopUpdateIntensity();
                break;
            default :
                break;
            }
            log("promptMessage = " + getResources().getText(stage.promptMessage));
            mPrompt.setText(stage.promptMessage);
        }

        enum RightButtonMode {
            Continue(R.string.voice_unlock_continue_label, true),
            Retry(R.string.voice_unlock_retry_label, true),
            Record(R.string.voice_unlock_record_label, true);

            /**
             * @param text The displayed text for this mode.
             * @param enabled Whether the button should be enabled.
             */
            RightButtonMode(int text, boolean enabled) {
                this.text = text;
                this.enabled = enabled;
            }

            final int text;
            final boolean enabled;
        }

        enum LeftButtonMode {
            Cancel(R.string.voice_unlock_cancel_label, true);

            /**
             * @param text The displayed text for this mode.
             * @param enabled Whether the button should be enabled.
             */
            LeftButtonMode(int text, boolean enabled) {
                this.text = text;
                this.enabled = enabled;
            }

            final int text;
            final boolean enabled;

        }

         enum Stage {

             Introduction(
                     R.string.voice_command_record_introdution,
                     LeftButtonMode.Cancel, RightButtonMode.Record),
             Prepare(
                     R.string.voice_command_record_prepare,
                     LeftButtonMode.Cancel, RightButtonMode.Retry),
             FirstRecording(
                     R.string.voice_command_record_first_recording,
                     LeftButtonMode.Cancel, RightButtonMode.Retry),
             NonFirstRecording(
                     R.string.voice_command_record_non_first_recording,
                     LeftButtonMode.Cancel, RightButtonMode.Retry),
             OneRoundOK(
                     R.string.voice_command_record_one_round_ok,
                     LeftButtonMode.Cancel, RightButtonMode.Retry),
             OneRoundNoisy(
                     R.string.voice_command_record_one_round_noisy,
                     LeftButtonMode.Cancel, RightButtonMode.Retry),
             OneRoundWeak(
                     R.string.voice_command_record_one_round_weak,
                     LeftButtonMode.Cancel, RightButtonMode.Retry),
             OneRoundDiff(
                     R.string.voice_command_record_one_round_diff,
                     LeftButtonMode.Cancel, RightButtonMode.Retry),
             OneRoundExist(
                     R.string.voice_command_record_one_round_exist,
                     LeftButtonMode.Cancel, RightButtonMode.Retry),
             RecordingOK(
                     R.string.voice_command_record_recording_ok,
                     LeftButtonMode.Cancel, RightButtonMode.Continue);

            /**
             * @param headerMessage The message displayed at the top.
             * @param leftMode The mode of the left button.
             * @param rightMode The mode of the right button.
             * @param footerMessage The footer message.
             * @param patternEnabled Whether the pattern widget is enabled.
             */
            Stage(int promptMessage,
                    LeftButtonMode leftMode,
                    RightButtonMode rightMode) {
                this.promptMessage = promptMessage;
                this.leftMode = leftMode;
                this.rightMode = rightMode;
            }

            final int promptMessage;
            final LeftButtonMode leftMode;
            final RightButtonMode rightMode;
        }
         private void log(String msg) {
             if (VoiceUnlock.DEBUG) {
                 Xlog.d(VoiceUnlock.TAG, "VoiceCommandRecord: " + msg);
             }
         }

    private void registerVoiceCommand(String pkgName) {
         if(!isRegistered) {
             try {
                 int errorid = mVCmdMgrService.registerListener(pkgName, mVoiceCallback);
                 if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                     isRegistered = true;
                 } else {
                     log("register voiceCommand fail " );
                 }
             } catch (RemoteException e){
                 isRegistered = false;
                 mVCmdMgrService = null;
                 log("register voiceCommand RemoteException =  " + e.getMessage() );
             }
         } else {
             log("register voiceCommand success " );
         }
         log("register voiceCommand end " );
     }

    private void unregisterVoicecommand(String pkgName) {
        if (mVCmdMgrService != null) {
            try {
                int errorid = mVCmdMgrService.unregisterListener(pkgName, mVoiceCallback);
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    isRegistered = false;
                }
            } catch (RemoteException e) {
                log("unregisteVoiceCmd voiceCommand RemoteException = " + e.getMessage() );
                isRegistered = false;
                mVCmdMgrService = null;
            }
            log("unregisteVoiceCmd end " );
            mContext.unbindService(mVoiceSerConnection);
            mVCmdMgrService = null;
            isRegistered = false;
        }
    }

     private void sendVoiceCommand(String pkgName, int mainAction, int subAction, Bundle extraData) {
         if(isRegistered) {
             try{
                 int errorid = mVCmdMgrService.sendCommand(pkgName, mainAction, subAction, extraData);
                 if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
                     log("send voice Command fail " );
                 } else {
                     log("send voice Command success " );
                 }
             }catch (RemoteException e){
                 isRegistered = false;
                 mVCmdMgrService = null;
                 log("send voice Command RemoteException =  " + e.getMessage() );
             }
         } else {
             log("didn't register , can not send voice Command  " );
         }
     }

     private void bindVoiceService(Context context){
         log("bindVoiceService begin  " );
         Intent mVoiceServiceIntent = new Intent();
         mVoiceServiceIntent.setAction(VoiceCommandListener.VOICE_SERVICE_ACTION);
         mVoiceServiceIntent.addCategory(VoiceCommandListener.VOICE_SERVICE_CATEGORY);
         context.bindService(mVoiceServiceIntent, mVoiceSerConnection, Context.BIND_AUTO_CREATE);
     }

     private ServiceConnection mVoiceSerConnection = new ServiceConnection() {
          @Override
         public void onServiceConnected(ComponentName name, IBinder service){
             mVCmdMgrService = IVoiceCommandManagerService.Stub.asInterface(service);
             registerVoiceCommand(mPkgName);
             log("onServiceConnected   " );

             voiceTrainingStart();
         }
         public void onServiceDisconnected(ComponentName name) {
             log("onServiceDisconnected   " );
             isRegistered = false;
             mVCmdMgrService = null;
         }

     };

    }
}
