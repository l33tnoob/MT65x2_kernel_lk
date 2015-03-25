package com.mediatek.notebook;

import android.content.Context;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.widget.ProgressBar;
import android.widget.TextView;

public class NoteWaitCursorView {
    private static final String TAG = "WaitCursorView";
    private TextView mLoadingText;
    private ProgressBar mProgress;
    private View mLoadingContainer;
    private Context mContext;
    private boolean mIsQuerying = false;
    
    public NoteWaitCursorView(Context context, View loadingContainer,
            ProgressBar progress, TextView loadingText) {
        mContext = context;
        mLoadingContainer = loadingContainer;
        mProgress = progress;
        mLoadingText = loadingText;
    }

    public void startWaitCursor() {
        mLoadingContainer.setVisibility(View.VISIBLE);
        mLoadingText.setVisibility(View.VISIBLE);
        mProgress.setVisibility(View.VISIBLE);
        mIsQuerying = true;
    }

    public void stopWaitCursor() {
        //mLoadingContainer.startAnimation(AnimationUtils.loadAnimation(mContext,
        //        android.R.anim.fade_out));
        mLoadingContainer.setVisibility(View.GONE);
        mLoadingText.setVisibility(View.GONE);
        mProgress.setVisibility(View.GONE);
        mIsQuerying = false;
    }

    public boolean getQueryStatus() {
        return mIsQuerying;
    }
}
