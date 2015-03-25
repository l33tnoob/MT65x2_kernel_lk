
package com.mediatek.cellbroadcastreceiver;

import java.io.IOException;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.graphics.drawable.Drawable;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnErrorListener;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.ProgressBar;

public class AlertTonePreviewDialog extends DialogPreference {
    private static final String TAG = "AlertTonePreviewDialog";
    private static final float DISABLED_ALPHA = 0.4f;
    private ImageView mPlayBtn;
    private ProgressBar mProgressBar;
    private MediaPlayer mMediaPlayer;
    private ShowProgressThread thread;

    public AlertTonePreviewDialog(Context context, AttributeSet attrs) {
        super(context, attrs);

        setDialogLayoutResource(R.layout.preview_dialog);
        setPositiveButtonText(android.R.string.ok);
        setNegativeButtonText(null);
    }

    @Override
    protected void onBindDialogView(View view) {
        super.onBindDialogView(view);
        initControl(view);
    }

    @Override
    protected void onDialogClosed(boolean positiveResult) {
        super.onDialogClosed(positiveResult);
        if (mMediaPlayer != null) {
            Log.d(TAG, "onDialogClosed");
            mMediaPlayer.release();
            mMediaPlayer = null;
            thread.setFlag(true);
        }
    }

    public void initMediaPlay() {
        Log.d(TAG, "initMediaPlay");
        mMediaPlayer = null;
        mMediaPlayer = MediaPlayer.create(getContext(), R.raw.attention_signal);
        mMediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            public void onCompletion(MediaPlayer arg0) {
                Log.d(TAG, "onCompletion");
                thread.setFlag(true);
                mMediaPlayer.seekTo(0);
                mProgressBar.setProgress(0);
                setPlayBtnState(true);
            }
        });

        mMediaPlayer.setOnErrorListener(new OnErrorListener() {
            public boolean onError(MediaPlayer mp, int what, int extra) {
                Log.d(TAG, "Error occurred while playing audio.");
                mp.stop();
                mp.release();
                mMediaPlayer = null;
                return true;
            }
        });

        mProgressBar.setMax(mMediaPlayer.getDuration());
        try {
            mMediaPlayer.prepare();
        } catch (IOException e) {
            Log.e(TAG, "IOException");
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException");
        } catch (IllegalStateException e) {
            Log.e(TAG, "IllegalStateException");
        }
    }

    public void initControl(View view) {
        mPlayBtn = (ImageView) view.findViewById(R.id.play);
        mProgressBar = (ProgressBar) view.findViewById(R.id.progressbar);
        mPlayBtn.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                initMediaPlay();
                playTone();
            }
        });
        initMediaPlay();
        playTone();
    }

    public void playTone() {
        if (mMediaPlayer != null) {
            mMediaPlayer.start();
            thread = null;
            thread = new ShowProgressThread();
            thread.start();
            setPlayBtnState(false);
        }
    }

    public void setPlayBtnState(boolean flag) {
        mPlayBtn.setEnabled(flag);
        mPlayBtn.setClickable(flag);
        mPlayBtn.setFocusable(flag);
        if (!flag) {
            mPlayBtn.setAlpha(DISABLED_ALPHA);
        } else {
            mPlayBtn.setAlpha(1.0f);
        }
    }

    private class ShowProgressThread extends Thread {
        private boolean mStopFlag;

        public void setFlag(boolean flag) {
            mStopFlag = flag;
        }

        public void run() {
            try {
                while (!mStopFlag) {
                    if (mProgressBar.getProgress() < mMediaPlayer.getCurrentPosition()) {
                        mProgressBar.setProgress(mMediaPlayer.getCurrentPosition());
                        sleep(10);
                    }
                    if (mProgressBar.getProgress() == mMediaPlayer.getDuration()) {
                        Log.d(TAG, "reach max");
                        break;
                    }
                }
            } catch (Exception e) {
            }
        }
    }

}
