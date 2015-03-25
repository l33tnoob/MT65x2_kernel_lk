/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.mediatek.cellbroadcastreceiver;

import android.R.integer;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.AssetFileDescriptor;
import android.content.res.Resources;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnErrorListener;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Vibrator;
import android.speech.tts.TextToSpeech;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;
import java.util.ArrayList;
import java.util.Locale;

import static com.mediatek.cellbroadcastreceiver.CellBroadcastReceiver.DBG;

/**
 * Manages alert audio and vibration and text-to-speech. Runs as a service so that
 * it can continue to play if another activity overrides the CellBroadcastListActivity.
 */
public class CellBroadcastAlertAudio extends Service implements TextToSpeech.OnInitListener,
        TextToSpeech.OnUtteranceCompletedListener {
    private static final String TAG = "CellBroadcastAlertAudio";

    /** Action to start playing alert audio/vibration/speech. */
    static final String ACTION_START_ALERT_AUDIO = "ACTION_START_ALERT_AUDIO";

    /** Extra for alert audio duration (from settings). */
    public static final String ALERT_AUDIO_DURATION_EXTRA =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_DURATION";

    /** Extra for message body to speak (if speech enabled in settings). */
    public static final String ALERT_AUDIO_MESSAGE_BODY =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_MESSAGE_BODY";

    /** Extra for text-to-speech language (if speech enabled in settings). */
    public static final String ALERT_AUDIO_MESSAGE_LANGUAGE =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_MESSAGE_LANGUAGE";

    /** Extra for alert audio vibration enabled (from settings). */
    public static final String ALERT_AUDIO_VIBRATE_EXTRA =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_VIBRATE";

    /** Extra for alert audio ETWS behavior (always vibrate, even in silent mode). */
    public static final String ALERT_AUDIO_ETWS_VIBRATE_EXTRA =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_ETWS_VIBRATE";

    /** Pause duration between alert sound and alert speech. */
    private static final int PAUSE_DURATION_BEFORE_SPEAKING_MSEC = 1000;

    //volume change action
    private static final String VOLUME_CHANGED_ACTION = "android.media.VOLUME_CHANGED_ACTION";

    /** Vibration uses the same on/off pattern as the CMAS alert tone */
    private static final long[] sVibratePattern = { 0, 2000, 500, 1000, 500, 1000, 500,
            2000, 500, 1000, 500, 1000};

    private static final int STATE_IDLE = 0;
    private static final int STATE_ALERTING = 1;
    private static final int STATE_PAUSING = 2;
    private static final int STATE_SPEAKING = 3;

    // Internal messages
    private static final int ALERT_SOUND_FINISHED = 1000;
    private static final int ALERT_PAUSE_FINISHED = 1001;
    /// M: add for repeat alert feature. @{
    private static final String PREF_NAME = "com.mediatek.cellbroadcastreceiver_preferences";
    private static final int REPEAT_ALERT_START = 1002;
    private static final int[] sRepeatTimes = { 60000, 60000 * 3, 60000 * 5 };
    private boolean mEnableRepeat = true;
    private int mDuration;
    private ArrayList<Integer> mRepeatPattern;
    /// @}

    private int mState;

    private TextToSpeech mTts;
    private boolean mTtsEngineReady;

    private String mMessageBody;
    private String mMessageLanguage;
    private boolean mTtsLanguageSupported;
    private boolean mEnableVibrate;
    private boolean mEnableAudio;
    private boolean mRegisteredVolumeReceiver = false;

    private Vibrator mVibrator;
    private MediaPlayer mMediaPlayer;
    private AudioManager mAudioManager;
    private TelephonyManager mTelephonyManager;
    private int mInitialCallState;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case ALERT_SOUND_FINISHED:
                    if (DBG) log("ALERT_SOUND_FINISHED "+ mRepeatPattern.size());
                    /// @}
                    stop();     // stop alert sound
                    // if we can speak the message text
                    if (mMessageBody != null && mTtsEngineReady && mTtsLanguageSupported) {
                        mHandler.sendMessageDelayed(mHandler.obtainMessage(ALERT_PAUSE_FINISHED),
                                PAUSE_DURATION_BEFORE_SPEAKING_MSEC);
                        mState = STATE_PAUSING;
                    } else if (!mEnableRepeat || mRepeatPattern.isEmpty()) { ///M:
                        // add for repeat alert feature. do not stop service until end repeat.
                        stopSelf();
                        mState = STATE_IDLE;
                    }
                    break;

                case ALERT_PAUSE_FINISHED:
                    if (DBG) log("ALERT_PAUSE_FINISHED");
                    if (mMessageBody != null && mTtsEngineReady && mTtsLanguageSupported) {
                        if (DBG) log("Speaking broadcast text: " + mMessageBody);
                        mTts.speak(mMessageBody, TextToSpeech.QUEUE_FLUSH, null);
                        mState = STATE_SPEAKING;
                    } else if (!mEnableRepeat || mRepeatPattern.isEmpty()) { ///M: 
                        // add for repeat alert feature. do not stop service until end repeat.
                        Log.w(TAG, "TTS engine not ready or language not supported");
                        stopSelf();
                        mState = STATE_IDLE;
                    }
                    break;

                ///M: add for repeat alert feature. @{
                case REPEAT_ALERT_START:
                    if (DBG) log("REPEAT_ALERT_START");
                    if (!mRepeatPattern.isEmpty()) {
                        mRepeatPattern.remove(0);
                        play();
                    }
                    break;
                /// @}
                default:
                    Log.e(TAG, "Handler received unknown message, what=" + msg.what);
            }
        }
    };

    private final PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String ignored) {
            // Stop the alert sound and speech if the call state changes.
            if (state != TelephonyManager.CALL_STATE_IDLE
                    && state != mInitialCallState) {
                stopSelf();
            }
        }
    };

    private BroadcastReceiver volumeChangeReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context arg0, Intent arg1) {
            Log.d(TAG, "receive volume change broadcast, stop sound and vibration alert");
            stop();
        }

    };

    /**
     * Callback from TTS engine after initialization.
     * @param status {@link TextToSpeech#SUCCESS} or {@link TextToSpeech#ERROR}.
     */
    @Override
    public void onInit(int status) {
        if (DBG) log("onInit() TTS engine status: " + status);
        if (status == TextToSpeech.SUCCESS) {
            mTtsEngineReady = true;
            // try to set the TTS language to match the broadcast
            setTtsLanguage();
        } else {
            mTtsEngineReady = false;
            mTts = null;
            Log.e(TAG, "onInit() TTS engine error: " + status);
        }
    }

    /**
     * Try to set the TTS engine language to the value of mMessageLanguage.
     * mTtsLanguageSupported will be updated based on the response.
     */
    private void setTtsLanguage() {
        if (mMessageLanguage != null) {
            if (DBG) log("Setting TTS language to '" + mMessageLanguage + '\'');
            int result = mTts.setLanguage(new Locale(mMessageLanguage));
            // success values are >= 0, failure returns negative value
            if (DBG) log("TTS setLanguage() returned: " + result);
            mTtsLanguageSupported = result >= 0;
        } else {
            // try to use the default TTS language for broadcasts with no language specified
            if (DBG) log("No language specified in broadcast: using default");
            mTtsLanguageSupported = true;
        }
    }

    /**
     * Callback from TTS engine.
     * @param utteranceId the identifier of the utterance.
     */
    @Override
    public void onUtteranceCompleted(String utteranceId) {
        ///M: if support repeat alert feature, do not stop service until repeat over.
        if (!mEnableRepeat || mRepeatPattern.isEmpty()) {
            stopSelf();
        }
    }

    @Override
    public void onCreate() {
        mVibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        // Listen for incoming calls to kill the alarm.
        mTelephonyManager =
                (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManager.listen(
                mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
    }

    @Override
    public void onDestroy() {
        //unregister volume register
        if (mRegisteredVolumeReceiver) {
            mRegisteredVolumeReceiver = false;
            unregisterReceiver(volumeChangeReceiver);
        }

        /// M: add for repeat alert feature. cancel the next schedule when destroy. @{
        if (mEnableRepeat && !mRepeatPattern.isEmpty()) {
            cancelRepeat();
        }
        /// @}

        // stop audio, vibration and TTS
        stop();
        // Stop listening for incoming calls.
        mTelephonyManager.listen(mPhoneStateListener, 0);
        // shutdown TTS engine
        if (mTts != null) {
            try {
                mTts.shutdown();
            } catch (IllegalStateException e) {
                // catch "Unable to retrieve AudioTrack pointer for stop()" exception
                Log.e(TAG, "exception trying to shutdown text-to-speech");
            }
        }
        // release CPU wake lock acquired by CellBroadcastAlertService
        CellBroadcastAlertWakeLock.releaseCpuLock();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // No intent, tell the system not to restart us.
        if (intent == null) {
            stopSelf();
            return START_NOT_STICKY;
        }

        ///M: add for repeat alert feature.
        cancelRepeat();
        initAlertSequence();

        // This extra should always be provided by CellBroadcastAlertService,
        // but default to 10.5 seconds just to be safe (CMAS requirement).
        int duration = intent.getIntExtra(ALERT_AUDIO_DURATION_EXTRA, 10500);

        // Get text to speak (if enabled by user)
        mMessageBody = intent.getStringExtra(ALERT_AUDIO_MESSAGE_BODY);
        mMessageLanguage = intent.getStringExtra(ALERT_AUDIO_MESSAGE_LANGUAGE);

        mEnableVibrate = intent.getBooleanExtra(ALERT_AUDIO_VIBRATE_EXTRA, true);
        boolean forceVibrate = intent.getBooleanExtra(ALERT_AUDIO_ETWS_VIBRATE_EXTRA, false);

        switch (mAudioManager.getRingerMode()) {
            case AudioManager.RINGER_MODE_SILENT:
                if (DBG) log("Ringer mode: silent");
                mEnableVibrate = forceVibrate;
                mEnableAudio = false;
                break;

            case AudioManager.RINGER_MODE_VIBRATE:
                if (DBG) log("Ringer mode: vibrate");
                mEnableAudio = false;
                break;

            case AudioManager.RINGER_MODE_NORMAL:
            default:
                if (DBG) log("Ringer mode: normal");
                /// M: add to distinguish general and outdoor mode. Do not vibrate
                // in general mode. @{
                if (!mAudioManager.shouldVibrate(AudioManager.VIBRATE_TYPE_NOTIFICATION)) {
                    mEnableVibrate = forceVibrate;
                }
                /// @}
                mEnableAudio = true;
                break;
        }

        if (mMessageBody != null && mEnableAudio) {
            if (mTts == null) {
                mTts = new TextToSpeech(this, this);
            } else if (mTtsEngineReady) {
                setTtsLanguage();
            }
        }
        ///M: add for repeat alert. @{
        mDuration = duration;
        schedulePlay();
        /// @}
        //register volume change receiver
        if (!mRegisteredVolumeReceiver) {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(VOLUME_CHANGED_ACTION);
            registerReceiver(volumeChangeReceiver, intentFilter);
            mRegisteredVolumeReceiver = true;
        }

        // Record the initial call state here so that the new alarm has the
        // newest state.
        mInitialCallState = mTelephonyManager.getCallState();

        return START_STICKY;
    }

    // Volume suggested by media team for in-call alarms.
    private static final float IN_CALL_VOLUME = 0.125f;

    /**
     * Start playing the alert sound, and send delayed message when it's time to stop.
     * @param duration the alert sound duration in milliseconds
     */
    private void play() {
        // stop() checks to see if we are already playing.
        stop();

        if (DBG) log("play()");

        // Start the vibration first.
        if (mEnableVibrate || forceAlert()) {
            mVibrator.vibrate(sVibratePattern, -1);
        }

        if (mEnableAudio || forceAlert()) {
            // future optimization: reuse media player object
            mMediaPlayer = new MediaPlayer();
            mMediaPlayer.setOnErrorListener(new OnErrorListener() {
                public boolean onError(MediaPlayer mp, int what, int extra) {
                    Log.e(TAG, "Error occurred while playing audio.");
                    mp.stop();
                    mp.release();
                    mMediaPlayer = null;
                    return true;
                }
            });

            try {
                // Check if we are in a call. If we are, play the alert
                // sound at a low volume to not disrupt the call.
                if (mTelephonyManager.getCallState()
                        != TelephonyManager.CALL_STATE_IDLE) {
                    Log.v(TAG, "in call: reducing volume");
                    mMediaPlayer.setVolume(IN_CALL_VOLUME, IN_CALL_VOLUME);
                }

                // start playing alert audio (unless master volume is vibrate only or silent).
                setDataSourceFromResource(getResources(), mMediaPlayer,
                        R.raw.attention_signal);
                int streamType = AudioManager.STREAM_NOTIFICATION;
                if (forceAlert()) {
                    streamType = AudioManager.STREAM_ALARM;
                }
                mAudioManager.requestAudioFocus(null, streamType,
                        AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
                startAlarm(mMediaPlayer);
            } catch (Exception ex) {
                Log.e(TAG, "Failed to play alert sound", ex);
            }
        }

        // stop alert after the specified duration
        mHandler.sendMessageDelayed(mHandler.obtainMessage(ALERT_SOUND_FINISHED), mDuration);
        mState = STATE_ALERTING;
    }

    // Do the common stuff when starting the alarm.
    private void startAlarm(MediaPlayer player)
            throws java.io.IOException, IllegalArgumentException, IllegalStateException {
        int streamType = AudioManager.STREAM_NOTIFICATION;
        if (forceAlert()) {
            streamType = AudioManager.STREAM_ALARM;
        }
        player.setAudioStreamType(streamType);
        player.setLooping(true);
        player.prepare();
        player.start();
    }

    private static void setDataSourceFromResource(Resources resources,
            MediaPlayer player, int res) throws java.io.IOException {
        AssetFileDescriptor afd = resources.openRawResourceFd(res);
        if (afd != null) {
            player.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(),
                    afd.getLength());
            afd.close();
        }
    }

    /**
     * Stops alert audio and speech.
     */
    public void stop() {
        if (DBG) log("stop()");

        mHandler.removeMessages(ALERT_SOUND_FINISHED);
        mHandler.removeMessages(ALERT_PAUSE_FINISHED);

        if (mState == STATE_ALERTING) {
            // Stop audio playing
            if (mMediaPlayer != null) {
                try {
                    mMediaPlayer.stop();
                    mMediaPlayer.release();
                } catch (IllegalStateException e) {
                    // catch "Unable to retrieve AudioTrack pointer for stop()" exception
                    Log.e(TAG, "exception trying to stop media player");
                }
                mMediaPlayer = null;
            }

            // Stop vibrator
            mVibrator.cancel();
        } else if (mState == STATE_SPEAKING && mTts != null) {
            try {
                mTts.stop();
            } catch (IllegalStateException e) {
                // catch "Unable to retrieve AudioTrack pointer for stop()" exception
                Log.e(TAG, "exception trying to stop text-to-speech");
            }
        }
        mAudioManager.abandonAudioFocus(null);
        mState = STATE_IDLE;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    /**
     * M: add for repeat alert feature.
     */
    private void initAlertSequence() {
        SharedPreferences pre = getSharedPreferences(PREF_NAME, 0);
        mEnableRepeat = pre.getBoolean(CheckBoxAndSettingsPreference.KEY_ENABLE_REPEAT_ALERT, true);
        mRepeatPattern = new ArrayList<Integer>();
        if (mEnableRepeat) {
            for (int i : sRepeatTimes) {
                mRepeatPattern.add(i);
            }
        }
    }

    /**
     * M: add for repeat alert feature. 
     * @param duration the duration of one alert time.
     */
    private void schedulePlay() {
        if (mAudioManager.getRingerMode() != AudioManager.RINGER_MODE_SILENT) {
            play();
        }
        if (mEnableRepeat) {
            for (int i = 0; i < mRepeatPattern.size(); i++) {
                mHandler.sendMessageDelayed(mHandler.obtainMessage(REPEAT_ALERT_START),
                        mRepeatPattern.get(i));
            }
        }
    }

    /**
     * M: add for repeat alert feature.
     */
    public void cancelRepeat() {
        mHandler.removeMessages(REPEAT_ALERT_START);
    }

    /**
     * M: force alarm (except first alert) in silent mode.
     * @return
     */
    private boolean forceAlert() {
        boolean isSilentMode = mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_SILENT;
        boolean isRepeating = mEnableRepeat && (mRepeatPattern.size() < sRepeatTimes.length);
        return isSilentMode && isRepeating;
    }
}
