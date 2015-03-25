package com.mediatek.voicecommand.voiceunlock;

import java.io.File;
import java.io.IOException;

import com.mediatek.xlog.Xlog;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.view.View;
import android.view.Window;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.SeekBar.OnSeekBarChangeListener;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.mgr.ConfigurationManager;

public class PswPreview extends Activity implements OnPreparedListener, OnErrorListener, OnCompletionListener{

    private PreviewPlayer mPlayer;
    private TextView mTitleView;
    private CharSequence mTitle;
    private SeekBar mSeekBar;
    private Handler mProgressRefresher;
    private boolean mSeeking = false;
    private int mDuration = -1;
    private Uri mUri;
    private AudioManager mAudioManager;
    private boolean mIsComplete = false;
    private boolean mPausedByTransientLossOfFocus;
    //check the activity status for power saving
    private boolean mPauseRefreshingProgressBar = false;
    static final String KEY_COMMAND_SUMMARY = "command_summary";
    static final String KEY_COMMAND_ID = "command_id";
    private String TAG = "VoiceUnlock";
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        
        Intent intent = getIntent();
        if (intent == null) {
            finish();
            return;
        }

        ConfigurationManager VoiceConfigMgr = ConfigurationManager.getInstance(this);
        if (VoiceConfigMgr == null) {
            Xlog.e(TAG, "ConfigurationManager is null");
            finish();
            return;
        }
        String path = VoiceConfigMgr.getPasswordFilePath();
        int commandId = intent.getIntExtra(KEY_COMMAND_ID, 0);
        mTitle = intent.getCharSequenceExtra(KEY_COMMAND_SUMMARY);
        String Filepath = path + commandId + ".dat";
        File file = new File(Filepath);
        mUri = Uri.fromFile(file);
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.pwd_preview);

        mTitleView = (TextView) findViewById(R.id.title);
        mSeekBar = (SeekBar) findViewById(R.id.progress);
        mProgressRefresher = new Handler();
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

        PreviewPlayer player = (PreviewPlayer) getLastNonConfigurationInstance();
        if (player == null) {
            mPlayer = new PreviewPlayer();
            mPlayer.setActivity(this);
            try {
                mPlayer.setDataSourceAndPrepare(mUri);
            } catch (IOException ex) {
                // catch generic Exception, since we may be called with a media
                // content URI, another content provider's URI, a file URI,
                // an http URI, and there are different exceptions associated
                // with failure to open each of those.
                log("Failed to open file: " + ex);
                Toast.makeText(this, R.string.playback_failed, Toast.LENGTH_SHORT).show();
                finish();
                return;
            }
        } else {
            mPlayer = player;
            mPlayer.setActivity(this);
            if (mPlayer.isPrepared()) {
                showPostPrepareUI();
            }
        }
    }
    
    public void playPauseClicked(View v) {
        if (mPlayer.isPlaying()) {
            mPlayer.pause();
        } else {
            start();
        }
        mIsComplete = false;
        updatePlayPause();
    }
    
    private void showPostPrepareUI() {
        
        mDuration = mPlayer.getDuration();
        
        if (mDuration != 0) {
            mSeekBar.setMax(mDuration);
            mSeekBar.setVisibility(View.VISIBLE);
        }
        mSeekBar.setOnSeekBarChangeListener(mSeekListener);
        //request focus when the seekbar is not in touch mode
        if (!mSeekBar.isInTouchMode()) {
            mSeekBar.requestFocus();
        }

        View v = findViewById(R.id.titleandbuttons);
        v.setVisibility(View.VISIBLE);
        mAudioManager.requestAudioFocus(mAudioFocusListener, AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        mProgressRefresher.postDelayed(new ProgressRefresher(), 200);
        updatePlayPause();
    }
    
    private static class PreviewPlayer extends MediaPlayer implements OnPreparedListener {
        PswPreview mActivity;
        boolean mIsPrepared = false;

        public void setActivity(PswPreview activity) {
            mActivity = activity;
            setOnPreparedListener(this);
            setOnErrorListener(mActivity);
            setOnCompletionListener(mActivity);
        }

        public void setDataSourceAndPrepare(Uri uri) throws IllegalArgumentException,
                        SecurityException, IllegalStateException, IOException {
            setDataSource(mActivity,uri);
            prepareAsync();
        }
        
        @Override
        public void onPrepared(MediaPlayer mp) {
            mIsPrepared = true;
            mActivity.onPrepared(mp);
        }

        boolean isPrepared() {
            return mIsPrepared;
        }
    }
    
    private OnSeekBarChangeListener mSeekListener = new OnSeekBarChangeListener() {
        public void onStartTrackingTouch(SeekBar bar) {
            mSeeking = true;
        }
        public void onProgressChanged(SeekBar bar, int progress, boolean fromuser) {
            if (!fromuser) {
                return;
            }
            // check if the mPlayer is not a null reference
            if (!mSeeking && (mPlayer != null)) {
                mPlayer.seekTo(progress);
            }
        }
        public void onStopTrackingTouch(SeekBar bar) {
            // check if the mPlayer is not a null reference 
            if ((mPlayer != null)) {
                mPlayer.seekTo(bar.getProgress());
            }
            mSeeking = false;
            mIsComplete = false;
        }
    };
    
    private OnAudioFocusChangeListener mAudioFocusListener = new OnAudioFocusChangeListener() {
        public void onAudioFocusChange(int focusChange) {
            if (mPlayer == null) {
                // this activity has handed its MediaPlayer off to the next activity
                // (e.g. portrait/landscape switch) and should abandon its focus
                mAudioManager.abandonAudioFocus(this);
                return;
            }
            switch (focusChange) {
                case AudioManager.AUDIOFOCUS_LOSS:
                    mPausedByTransientLossOfFocus = false;
                    mPlayer.pause();
                    break;
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                    if (mPlayer.isPlaying()) {
                        mPausedByTransientLossOfFocus = true;
                        mPlayer.pause();
                    }
                    break;
                case AudioManager.AUDIOFOCUS_GAIN:
                    if (mPausedByTransientLossOfFocus) {
                        mPausedByTransientLossOfFocus = false;
                        start();
                    }
                    break;
            }
            updatePlayPause();
        }
    };
    
    class ProgressRefresher implements Runnable {

        public void run() {
            if (mPlayer != null && !mSeeking && mDuration != 0) {
                //Remove dummy varible.
                int position = mPlayer.getCurrentPosition();
                log("ProgressRefresher Position:" + position);
                //if the media file is complete ,we set SeekBar to the end @{
                if (mIsComplete) {
                    position = mDuration;
                }
                
                mSeekBar.setProgress(position);
            }
            mProgressRefresher.removeCallbacksAndMessages(null);
            // check if the activity is pause for power saving 
            if (!mPauseRefreshingProgressBar) {
                mProgressRefresher.postDelayed(new ProgressRefresher(), 200);
            }
            
        }
    }
    

    @Override
    public void onPrepared(MediaPlayer mp) {
        if (isFinishing()) return;
        mPlayer = (PreviewPlayer) mp;
        setCommandTitle();
        mPlayer.start();
        showPostPrepareUI();
        
    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        Toast.makeText(this, R.string.playback_failed, Toast.LENGTH_SHORT).show();
        finish();
        return true;
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        updatePlayPause();
        //set the progress to end avoid when the file play complete but the 
        // the progress is not to the end of seekbar. 
        mSeekBar.setProgress(mSeekBar.getMax());
        mIsComplete = true;
        
    }
    
    @Override
    public Object onRetainNonConfigurationInstance() {
        PreviewPlayer player = mPlayer;
        mPlayer = null;
        return player;
    }
    
    @Override
    public void onDestroy() {
        stopPlayback();
        super.onDestroy();
    }
    
    @Override
    public void onUserLeaveHint() {
        stopPlayback();
        finish();
        super.onUserLeaveHint();
    }
    
    /**
     * stop to update the pregress bar when activity pausefor power saving
     */
    @Override
    public void onPause() {
        log("onPause for stop ProgressRefresher!");
        mPauseRefreshingProgressBar = true;
        mProgressRefresher.removeCallbacksAndMessages(null);
        super.onPause();
    }

    /**
     *  start the pregress bar update.
     */
    @Override
    public void onResume() {
        super.onResume();
        final int refreshTime = 200;
        log("onResume for start ProgressRefresher!");
        if (mPauseRefreshingProgressBar) {
            mPauseRefreshingProgressBar = false;
            mProgressRefresher.postDelayed(new ProgressRefresher(), refreshTime);
        }
    }

    /**
     *  monitor the current playing media file's duration update and reset the Maxprocess of SeekBar. 
     */
    public void onDurationUpdate(MediaPlayer mp, int duration) {
        if (duration > 0) {
            mDuration = duration;
            mSeekBar.setMax(mDuration);
        }
        log("onDurationUpdate(" + mDuration + ")");
    }
      
    public void setCommandTitle() {
        mTitleView.setText(mTitle);
      }
    
    private void stopPlayback() {
        if (mProgressRefresher != null) {
            mProgressRefresher.removeCallbacksAndMessages(null);
        }
        if (mPlayer != null) {
            mPlayer.release();
            mPlayer = null;
            mAudioManager.abandonAudioFocus(mAudioFocusListener);
        }
    }
    
    private void start() {
        mAudioManager.requestAudioFocus(mAudioFocusListener, AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        mPlayer.start();
        mProgressRefresher.postDelayed(new ProgressRefresher(), 200);
    }
    
    private void updatePlayPause() {
        ImageButton b = (ImageButton) findViewById(R.id.playpause);
        if (b != null) {
            if (mPlayer.isPlaying()) {
                b.setImageResource(R.drawable.btn_playback_ic_pause_small);
            } else {
                b.setImageResource(R.drawable.btn_playback_ic_play_small);
                mProgressRefresher.removeCallbacksAndMessages(null);
            }
        }
    }

    private void log(String msg) {
        Xlog.d(TAG, "PswPreview: " + msg);
    }
}
