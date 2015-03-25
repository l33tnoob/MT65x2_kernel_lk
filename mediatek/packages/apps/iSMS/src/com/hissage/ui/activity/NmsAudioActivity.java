package com.hissage.ui.activity;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.provider.MediaStore;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.Recorder;
import com.hissage.util.message.RemainingTimeCalculator;

public class NmsAudioActivity extends Activity implements Recorder.OnStateChangedListener {

    public String mAudioTempPath = "";
    public String mDstPath = "";

    private ProgressBar mStateProgressBar;
    private ImageButton mRecordButton;
    private ImageButton mPlayButton;
    private ImageButton mStopButton;
    private TextView mLeftTime;
    private TextView mRightTime;
    private LinearLayout mTimeZone;
    private TextView mStartRecordTip;
    private Button mSendButton;
    private Button mCancelButton;

    private RemainingTimeCalculator mRemainingTimeCalculator;
    private Recorder mRecorder;
    private Context mContext;

    static final String STATE_FILE_NAME = "soundrecorder.state";
    static final String RECORDER_STATE_KEY = "recorder_state";
    static final String SAMPLE_INTERRUPTED_KEY = "sample_interrupted";
    public static final String MAX_FILE_SIZE_KEY = "max_file_size";
    public static final String MAX_AUDIO_DURATION = "max_audio_duration";

    static final String AUDIO_3GPP = "audio/3gpp";
    static final String AUDIO_AMR = "audio/amr";
    static final String AUDIO_ANY = "audio/*";
    static final String ANY_ANY = "*/*";

    static final int BITRATE_AMR = 5900; // bits/sec
    static final int BITRATE_3GPP = 5900;

    WakeLock mWakeLock;
    String mRequestedType = AUDIO_AMR;
    boolean mSampleInterrupted = false;
    String mErrorUiMessage = null;

    long mMaxFileSize = -1; // can be specified in the intent

    long mDurationLimit = NmsCustomUIConfig.AUDIO_MAX_DURATION;

    boolean mWithDuration = false;

    private BroadcastReceiver mSDCardMountEventReceiver = null;

    String mTimerFormat;
    Runnable mUpdateTimer = new Runnable() {
        public void run() {
            updateTimerView();
        }
    };

    private final static String TAG = "NmsAudioActivity";

    public Handler mHandler = new Handler();

    private void updateTimerView() {
        int state = mRecorder.state();

        boolean ongoing = state == Recorder.RECORDING_STATE || state == Recorder.PLAYING_STATE;

        if (ongoing) {
            mTimeZone.setVisibility(View.VISIBLE);
        } else {
            mTimeZone.setVisibility(View.GONE);
        }

        long lefttime = 0;
        int file = (int) mMaxFileSize / 1024;
        mRightTime.setText("" + file + "K");
        if (state == Recorder.PLAYING_STATE) {
            lefttime = mRecorder.progress();
            String timeStr = String.format(mTimerFormat, lefttime / 60, lefttime % 60);
            mLeftTime.setText(timeStr);

            long t = mRecorder.sampleLength();
            String timeStr1 = String.format(mTimerFormat, t / 60, t % 60);
            mRightTime.setText(timeStr1);
            mStateProgressBar.setProgress((int) (100 * lefttime / mRecorder.sampleLength()));
        } else if (state == Recorder.RECORDING_STATE) {
            updateFileSizeRemaining();
        }

        if (ongoing)
            mHandler.postDelayed(mUpdateTimer, 0);
    }

    private void updateTimeRemaining() {
        long t = 0;
        if (mWithDuration) {
            t = mDurationLimit - mRecorder.progress();
        } else {
            t = mRemainingTimeCalculator.timeRemaining();
        }
        if (t <= 0) {
            mSampleInterrupted = true;

            int limit = mRemainingTimeCalculator.currentLowerLimit();
            switch (limit) {
            case RemainingTimeCalculator.DISK_SPACE_LIMIT:
                Toast.makeText(mContext, "Storage is full", Toast.LENGTH_SHORT).show();
                break;
            case RemainingTimeCalculator.FILE_SIZE_LIMIT:
                break;
            default:
                mErrorUiMessage = null;
                break;
            }

            mRecorder.stop();
            return;
        }

        long min = t / 60 > 6 ? 7 : t / 60;
        String timeStr = String.format(mTimerFormat, min, t % 60);
        mRightTime.setText(timeStr);
    }

    private void updateFileSizeRemaining() {
        long filesize = mRemainingTimeCalculator.getCurrentFileSize();
        int t = (300 - 1) * 1024 -  (int)filesize;// To prevent delays
        if (t <= 0) {
            mSampleInterrupted = true;
            int limit = mRemainingTimeCalculator.currentLowerLimit();
            switch (limit) {
            case RemainingTimeCalculator.DISK_SPACE_LIMIT:
                Toast.makeText(mContext, "Storage is full", Toast.LENGTH_SHORT).show();
                break;
            case RemainingTimeCalculator.FILE_SIZE_LIMIT:
                break;
            default:
                mErrorUiMessage = null;
                break;
            }

            mRecorder.stop();
            return;
        } else {
            mStateProgressBar.setProgress((int) (100 * filesize / (300 * 1024)));
        }

        long size = filesize / 1024;
        String fileStr = "" + size + "K";
        mLeftTime.setText(fileStr);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.record_audio_layout);
        setFinishOnTouchOutside(false);

        Intent i = getIntent();
        if (i != null) {
            String s = i.getType();
            if (AUDIO_AMR.equals(s) || AUDIO_3GPP.equals(s) || AUDIO_ANY.equals(s)
                    || ANY_ANY.equals(s)) {
                mRequestedType = s;
            } else if (s != null) {
                setResult(RESULT_CANCELED);
                finish();
                return;
            }
            mDurationLimit = i.getLongExtra(MAX_AUDIO_DURATION, -1);
            mMaxFileSize = i.getLongExtra(MAX_FILE_SIZE_KEY, -1);
            if (mDurationLimit == -1) {
                mDurationLimit = NmsCustomUIConfig.AUDIO_MAX_DURATION;
            }
            if (mMaxFileSize == -1) {
                mWithDuration = true;
            } else {
                mWithDuration = false;
            }
        }
        mContext = this;
        initialize();
        registerExternalStorageListener();
        if (savedInstanceState != null) {
            Bundle recorderState = savedInstanceState.getBundle(RECORDER_STATE_KEY);
            if (recorderState != null) {
                mRecorder.restoreState(recorderState);
                mSampleInterrupted = recorderState.getBoolean(SAMPLE_INTERRUPTED_KEY, false);
                mMaxFileSize = recorderState.getLong(MAX_FILE_SIZE_KEY, -1);
                mDurationLimit = recorderState.getLong(MAX_AUDIO_DURATION, -1);
                if (mMaxFileSize == -1) {
                    mWithDuration = true;
                } else {
                    mWithDuration = false;
                }
            }
        }

        updateUi();

        if (NmsCommonUtils.getSDcardAvailableSpace() < NmsCustomUIConfig.MIN_SEND_ATTACH_SD_CARD_SIZE) {
            NmsLog.error("audio_activity", "sd card available space is not enough: "
                    + NmsCommonUtils.getSDcardAvailableSpace());
            Toast.makeText(this, R.string.STR_NMS_SD_CARD_FULL, Toast.LENGTH_SHORT).show();
            setResult(RESULT_CANCELED);
            finish();
            return;
        }
    }

    @Override
    public void onStop() {
        mRecorder.stop();
        super.onStop();
    }

    @Override
    protected void onPause() {
        mSampleInterrupted = mRecorder.state() == Recorder.RECORDING_STATE;
        mRecorder.stop();
        if (mWakeLock != null && mWakeLock.isHeld())
            mWakeLock.release();
        super.onPause();
    }

    @Override
    public void onDestroy() {
        if (mSDCardMountEventReceiver != null) {
            unregisterReceiver(mSDCardMountEventReceiver);
            mSDCardMountEventReceiver = null;
        }
        if (mWakeLock != null && mWakeLock.isHeld())
            mWakeLock.release();
        super.onDestroy();
    }

    private void registerExternalStorageListener() {
        if (mSDCardMountEventReceiver == null) {
            mSDCardMountEventReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    String action = intent.getAction();
                    if (action.equals(Intent.ACTION_MEDIA_EJECT)) {
                        mRecorder.delete();
                    } else if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {
                        mSampleInterrupted = false;
                        updateUi();
                    }
                }
            };
            IntentFilter iFilter = new IntentFilter();
            iFilter.addAction(Intent.ACTION_MEDIA_EJECT);
            iFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
            iFilter.addDataScheme("file");
            registerReceiver(mSDCardMountEventReceiver, iFilter);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            setResult(RESULT_CANCELED);
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        if (mRecorder.sampleLength() == 0)
            return;

        Bundle recorderState = new Bundle();

        mRecorder.saveState(recorderState);
        recorderState.putBoolean(SAMPLE_INTERRUPTED_KEY, mSampleInterrupted);
        recorderState.putLong(MAX_FILE_SIZE_KEY, mMaxFileSize);
        recorderState.putLong(MAX_AUDIO_DURATION, -1);

        outState.putBundle(RECORDER_STATE_KEY, recorderState);
    }

    private void initialize() {

        if (NmsCommonUtils.getSDCardStatus()) {
            mAudioTempPath = NmsCommonUtils.getSDCardPath(this) + File.separator
                    + NmsCustomUIConfig.ROOTDIRECTORY + File.separator + "audio";
            File f = new File(mAudioTempPath);
            if (!f.exists()) {
                f.mkdirs();
            }
        }

        mSendButton = (Button) findViewById(R.id.send);
        mCancelButton = (Button) findViewById(R.id.cancel);
        mSendButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mRecorder.stop();
                mDstPath = mRecorder.sampleFile().getAbsolutePath();
                setResult();
            }
        });
        mCancelButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mRecorder.clear();
                mRecorder.delete();
                setResult(RESULT_CANCELED);
                finish();
            }
        });

        mRecordButton = (ImageButton) findViewById(R.id.ib_start);
        mPlayButton = (ImageButton) findViewById(R.id.ib_play);
        mStopButton = (ImageButton) findViewById(R.id.ib_stop);
        mLeftTime = (TextView) findViewById(R.id.tv_left_time);
        mRightTime = (TextView) findViewById(R.id.tv_right_time);
        mStateProgressBar = (ProgressBar) findViewById(R.id.pb_audio_progress);
        mTimeZone = (LinearLayout) findViewById(R.id.ll_time_zone);
        mStartRecordTip = (TextView) findViewById(R.id.tv_start_audio_tip);

        mPlayButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mRecorder.startPlayback();
            }
        });

        mStopButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mRecorder.stop();
            }
        });

        mRecordButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mRecorder.clear();
                mRemainingTimeCalculator.reset();
                if (!Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
                    mSampleInterrupted = true;
                    updateUi();
                } else if (!mRemainingTimeCalculator.diskSpaceAvailable()) {
                    mSampleInterrupted = true;
                    updateUi();
                } else {
                    stopAudioPlayback();

                    if (AUDIO_AMR.equals(mRequestedType)) {
                        mRemainingTimeCalculator.setBitRate(BITRATE_AMR);
                        mRecorder.startRecording(MediaRecorder.OutputFormat.AMR_NB, ".amr",
                                mContext);
                    } else if (AUDIO_3GPP.equals(mRequestedType)) {
                        mRemainingTimeCalculator.setBitRate(BITRATE_3GPP);
                        mRecorder.startRecording(MediaRecorder.OutputFormat.THREE_GPP, ".3gpp",
                                mContext);
                    } else {
                        throw new IllegalArgumentException("Invalid output file type requested");
                    }

                    if (mMaxFileSize != -1) {
                        mRemainingTimeCalculator.setFileSizeLimit(mRecorder.sampleFile(),
                                mMaxFileSize);
                    }
                }
            }
        });

        mRecorder = new Recorder();
        mRecorder.setOnStateChangedListener(this);
        mRemainingTimeCalculator = new RemainingTimeCalculator();
        mTimerFormat = getString(R.string.STR_NMS_AUDIO_TIMER_FORMAT);

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "SoundRecorder");
    }

    private void setResult() {
        Intent intent = new Intent();
        Uri uri = null;
        try {
            uri = this.addToMediaDB(mRecorder.sampleFile());
        } catch (UnsupportedOperationException ex) { // Database manipulation
                                                     // failure
            return;
        }
        if (uri == null) {
            return;
        }
        intent.setData(uri);
        setResult(RESULT_OK, intent);
        finish();
    }

    private void stopAudioPlayback() {
        Intent i = new Intent("com.android.music.musicservicecommand");
        i.putExtra("command", "pause");

        sendBroadcast(i);
    }

    @Override
    public void onStateChanged(int state) {
        if (state == Recorder.PLAYING_STATE || state == Recorder.RECORDING_STATE) {
            mSampleInterrupted = false;
            mErrorUiMessage = null;
            mWakeLock.acquire(); // we don't want to go to sleep while recording
                                 // or playing
        } else {
            if (mWakeLock.isHeld())
                mWakeLock.release();
        }

        updateUi();
    }

    @Override
    public void onError(int error) {
        // TODO Auto-generated method stub

    }

    private void updateUi() {

        switch (mRecorder.state()) {
        case Recorder.IDLE_STATE:
            if (mRecorder.sampleLength() == 0) {
                mRecordButton.setEnabled(true);
                mRecordButton.setFocusable(true);
                mPlayButton.setEnabled(false);
                mPlayButton.setFocusable(false);
                mStopButton.setEnabled(false);
                mStopButton.setFocusable(false);
                mRecordButton.requestFocus();
                mRecordButton.setVisibility(View.VISIBLE);
                mPlayButton.setVisibility(View.GONE);
                mStopButton.setVisibility(View.GONE);

                mStartRecordTip.setText(R.string.STR_NMS_RECORD_AUDIO_CONT);
                mSendButton.setEnabled(false);
                mSendButton.setTextColor(getResources().getColor(R.color.grey));
                mStateProgressBar.setVisibility(View.GONE);
                mStartRecordTip.setVisibility(View.VISIBLE);

            } else {
                mRecordButton.setEnabled(true);
                mRecordButton.setFocusable(true);
                mPlayButton.setEnabled(true);
                mPlayButton.setFocusable(true);
                mStopButton.setEnabled(false);
                mStopButton.setFocusable(false);
                mRecordButton.setVisibility(View.GONE);
                mPlayButton.setVisibility(View.VISIBLE);
                mStopButton.setVisibility(View.GONE);

                mStartRecordTip.setText(R.string.STR_NMS_PLAY_AUDIO_CONT);
                mSendButton.setEnabled(true);
                mSendButton.setTextColor(getResources().getColor(R.color.white));
                mStateProgressBar.setVisibility(View.GONE);
                mStartRecordTip.setVisibility(View.VISIBLE);
            }

            if (mSampleInterrupted) {
            }

            if (mErrorUiMessage != null) {
            }

            break;
        case Recorder.RECORDING_STATE:
            mRecordButton.setEnabled(false);
            mRecordButton.setFocusable(false);
            mPlayButton.setEnabled(false);
            mPlayButton.setFocusable(false);
            mStopButton.setEnabled(true);
            mStopButton.setFocusable(true);

            mRecordButton.setVisibility(View.GONE);
            mPlayButton.setVisibility(View.GONE);
            mStopButton.setVisibility(View.VISIBLE);

            mStateProgressBar.setVisibility(View.VISIBLE);
            mStartRecordTip.setVisibility(View.GONE);

            break;

        case Recorder.PLAYING_STATE:
            mRecordButton.setEnabled(true);
            mRecordButton.setFocusable(true);
            mPlayButton.setEnabled(false);
            mPlayButton.setFocusable(false);
            mStopButton.setEnabled(true);
            mStopButton.setFocusable(true);

            mRecordButton.setVisibility(View.GONE);
            mPlayButton.setVisibility(View.GONE);
            mStopButton.setVisibility(View.VISIBLE);

            mStateProgressBar.setVisibility(View.VISIBLE);
            mStartRecordTip.setVisibility(View.GONE);

            break;
        }

        updateTimerView();
    }

    private Uri addToMediaDB(File file) {
        Resources res = getResources();
        ContentValues cv = new ContentValues();
        long current = System.currentTimeMillis();
        long modDate = file.lastModified();
        Date date = new Date(current);
        SimpleDateFormat formatter = new SimpleDateFormat(
                res.getString(R.string.audio_db_title_format));
        String title = formatter.format(date);
        long sampleLengthMillis = mRecorder.sampleLength() * 1000L;

        cv.put(MediaStore.Audio.Media.IS_MUSIC, "0");

        cv.put(MediaStore.Audio.Media.TITLE, title);
        cv.put(MediaStore.Audio.Media.DATA, file.getAbsolutePath());
        cv.put(MediaStore.Audio.Media.DATE_ADDED, (int) (current / 1000));
        cv.put(MediaStore.Audio.Media.DATE_MODIFIED, (int) (modDate / 1000));
        cv.put(MediaStore.Audio.Media.DURATION, sampleLengthMillis);
        cv.put(MediaStore.Audio.Media.MIME_TYPE, mRequestedType);
        cv.put(MediaStore.Audio.Media.ARTIST, res.getString(R.string.audio_db_artist_name));
        cv.put(MediaStore.Audio.Media.ALBUM, res.getString(R.string.audio_db_album_name));
        Log.d(TAG, "Inserting audio record: " + cv.toString());
        ContentResolver resolver = getContentResolver();
        Uri base = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        Log.d(TAG, "ContentURI: " + base);
        Uri result = resolver.insert(base, cv);
        sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, result));
        return result;
    }
}
