/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.mediatek.oobe.qsg;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.os.Bundle;
import android.os.SystemProperties;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.oobe.R;
import com.mediatek.oobe.utils.PDebug;
import com.mediatek.oobe.utils.Utils;
import com.mediatek.xlog.Xlog;

import java.io.IOException;

public class QuickStartGuideMain extends Activity implements OnCompletionListener, 
        OnPreparedListener,SurfaceHolder.Callback, View.OnClickListener {
    private static final String TAG = "QuickStartGuideMain";

    private Button mLeftBtn;
    private Button mRightBtn;
    private LinearLayout mProgressBar;
    private TextView mTitle;
    private TextView mSummary;

    private MediaPlayer mMediaPlayer;
    // Key and value for enable ClearMotion
    private static final int CLEAR_MOTION_KEY = 1700;
    private static final int CLEAR_MOTION_DISABLE = 1;
	
    private SurfaceView mPreview;
    private SurfaceHolder mHolder;

    private int mCurrentStep = 0;
    private boolean mHideVideo = true;

    private boolean mIsPrepared = false;

    private boolean mIsFirstRun = false;
    private boolean mSetScreen = false;

    private static final int TITLE[] = { 
            R.string.qsg_title2_home_screen, 
            R.string.qsg_title3_choose_widget,
            R.string.qsg_title4_launch_page, 
            R.string.qsg_title5_view_notification, 
            R.string.qsg_app_name };
    private static final int SUMMARY[] = { 
            R.string.qsg_summary2_home_screen, 
            R.string.qsg_summary3_choose_widget,
            R.string.qsg_summary4_launch_page, 
            R.string.qsg_summary5_view_notification, 
            R.string.qsg_summary_end };

    private String[] mVideoTips = new String[] { 
            "JB_01View_Home_screen.mp4", 
            "JB_02Choose_some_widgets.mp4",
            "JB_03Launch_detail_page.mp4", 
            "JB_04ViewNotifications.mp4" };

    private OnErrorListener mErrorListener = new OnErrorListener() {

        @Override
        public boolean onError(MediaPlayer mp, int what, int extra) {
            Xlog.d(TAG, "play error: " + what);
            if (mMediaPlayer != null) {
                mMediaPlayer.reset();
            }
            return false;
        }
    };

    @Override
    protected void onCreate(Bundle icicle) {
        PDebug.Start("QuickStartGuideMain.onCreate");
        super.onCreate(icicle);
        Xlog.d(TAG, "onCreate() ");
        PDebug.Start("setContentView");
        setContentView(R.layout.videoview_layout);
        PDebug.End("setContentView");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        initLayout();
        initMedia();
        PDebug.End("QuickStartGuideMain.onCreate");
    }

    @Override
    protected void onDestroy() {
        releaseMediaPlayer();
        super.onDestroy();
    }

    @Override
    public void onCompletion(MediaPlayer arg0) {
        Xlog.d(TAG, "onCompletion called");
        updateTitleSummary(mCurrentStep, false);
        mLeftBtn.setVisibility(View.VISIBLE);
        mLeftBtn.setText(R.string.play_again);
    }

    @Override
    public void onPrepared(MediaPlayer mediaplayer) {
        PDebug.Start("onPrepared");
        Xlog.d(TAG, "onPrepared called");
        mIsPrepared = true;
        startPlay();
        PDebug.End("onPrepared");
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceholder, int i, int j, int k) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceholder) {
        Xlog.d(TAG, "surfaceDestroyed called");
        if (mMediaPlayer != null && mMediaPlayer.isPlaying()) {
            mMediaPlayer.pause();
            mMediaPlayer.release();
            mMediaPlayer = null;
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        PDebug.Start("surfaceCreated");
        Xlog.d(TAG, "surfaceCreated called");
        if (mMediaPlayer == null) {
            initMedia();
        }
        this.mHolder = holder;
        mMediaPlayer.setDisplay(mHolder);
        prepareVideo(mCurrentStep);
        PDebug.End("surfaceCreated");
    }

    public void onClick(View v) {
        if (v == mLeftBtn) {
            String text = (String)mLeftBtn.getText();
            if (getString(R.string.oobe_btn_text_skip).equals(text)) {
                finishQsg();
            } else if (getString(R.string.play_again).equals(text)) {
                // play again video from start
                if (mCurrentStep == mVideoTips.length) {
                    mCurrentStep = 0;
                    mHideVideo = false;
                    mProgressBar.setVisibility(View.VISIBLE);
                    mRightBtn.setText(R.string.oobe_btn_text_next);
                }
                updateTitleSummary(0, true);
                prepareVideo(mCurrentStep);
            }
        } else if (v == mRightBtn) {
            String text = (String)mRightBtn.getText();
            if (getString(R.string.ok_button).equals(text)) {
                mHideVideo = false;
                startPlay();
                mProgressBar.setVisibility(View.VISIBLE);
                updateProgress(mCurrentStep);
                mRightBtn.setText(R.string.oobe_btn_text_next);
            } else if (getString(R.string.oobe_btn_text_next).equals(text)) {
                nextStep();
            } else if (getString(R.string.oobe_btn_text_finish).equals(text)) {
                finishQsg();
            }
        }
    }

    /*
    * go to next step
    */
    private void nextStep() {
        if (mCurrentStep == mVideoTips.length - 1) {
            Xlog.d(TAG, "go to last QSG page");
            updateTitleSummary(mVideoTips.length, false);
            mRightBtn.setText(R.string.oobe_btn_text_finish);
            mLeftBtn.setText(R.string.play_again);
            mLeftBtn.setVisibility(View.VISIBLE);
            mHideVideo = true;
            mCurrentStep++;
            mPreview.setBackgroundResource(R.drawable.wallpaper);
            mProgressBar.setVisibility(View.GONE);
        } else {
            mCurrentStep++;
            updateTitleSummary(0, true);
            prepareVideo(mCurrentStep);
        }
    }

    /*
    * init layout
    */
    private void initLayout() {
        PDebug.Start("initLayout");
        // need full screen
        if ((getWindow().getAttributes().flags & WindowManager.LayoutParams.FLAG_FULLSCREEN) 
                != WindowManager.LayoutParams.FLAG_FULLSCREEN) {
            Xlog.d(TAG, " fullscreen = false");
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }

        mTitle = (TextView) findViewById(R.id.quickstartguide_title);
        mSummary = (TextView) findViewById(R.id.quickstartguide_summary);
        mProgressBar = (LinearLayout) findViewById(R.id.progressbar_layout);

        mLeftBtn = (Button) findViewById(R.id.left_btn);
        mLeftBtn.setOnClickListener(this);
        mRightBtn = (Button) findViewById(R.id.right_btn);
        mRightBtn.setOnClickListener(this);

        mIsFirstRun = getIntent().getBooleanExtra("mIsFirstRun", false);
        Xlog.d(TAG, "mIsFirstRun = " + mIsFirstRun);
        if (mIsFirstRun) {
            mLeftBtn.setText(R.string.oobe_btn_text_skip);
        } else {
            mLeftBtn.setVisibility(View.INVISIBLE);
        }
        mPreview = (SurfaceView) findViewById(R.id.surface);
        PDebug.Start("mPreview.getHolder");
        mHolder = mPreview.getHolder();
        PDebug.EndAndStart("mPreview.getHolder", "mHolder.addCallBack");
        mHolder.addCallback(this);
        PDebug.EndAndStart("mHolder.addCallBack", "takeSurface");
        mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        PDebug.End("takeSurface");
        PDebug.End("initLayout");
    }

    /*
    * init media player
    */
    private void initMedia() {
        PDebug.Start("initMediaPlayer");
        mMediaPlayer = new MediaPlayer();
        mMediaPlayer.setOnCompletionListener(this);
        mMediaPlayer.setOnPreparedListener(this);
        mMediaPlayer.setOnErrorListener(mErrorListener);
        PDebug.End("initMediaPlayer");
    }

    /*
    * start prepare video
    */
    private void prepareVideo(int step) {
        PDebug.Start("prepareVideo");
        Xlog.d(TAG, "prepareVideo step = " + step);
        if (step > mVideoTips.length - 1) {
            return;
        }

        updateProgress(step);
        mIsPrepared = false;
        try {
            String path = mVideoTips[step];
            if (mMediaPlayer != null) {
                mMediaPlayer.reset();
                AssetFileDescriptor afd = getAssets().openFd(path);
                PDebug.Start("setDataSource");
                mMediaPlayer.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
                PDebug.EndAndStart("setDataSource", "prepare");
                afd.close();
                // Disable ClearMotion
                mMediaPlayer.setParameter(CLEAR_MOTION_KEY, CLEAR_MOTION_DISABLE);				
                mMediaPlayer.prepare();
                PDebug.End("prepare");
                resizeSurfaceView();
            }
        } catch (IOException e) {
            Xlog.e(TAG, "IOException; e: " + e.getMessage(), e);
            releaseMediaPlayer();
        } catch (IllegalStateException e) {
            Xlog.e(TAG, "IllegalStateException; e: " + e.getMessage(), e);
            releaseMediaPlayer();
        }
        PDebug.End("prepareVideo");
    }

    /*
    * start play video
    */
    private void startPlay() {
        if (!mHideVideo && mIsPrepared) {
            updateTitleSummary(0, true);
            mMediaPlayer.start();
            mPreview.setBackgroundColor(android.R.color.transparent);
        }
    }

    /*
    * update progress bar
    * @param step current step
    */
    protected void updateProgress(int step) {
        if (mHideVideo) {
            return;
        }
        ImageView image;
        for (int i = 0; i < mVideoTips.length; i++) {
            image = (ImageView) mProgressBar.getChildAt(i);
            if (i == step) {
                image.setImageResource(R.drawable.progress_radio_on);
            } else {
                image.setImageResource(R.drawable.progress_radio_off);
            }
            image.setVisibility(View.VISIBLE);
        }
    }

    /*
    * update title and summary
    * @param index current step 
    * @param isPlaying whether media player is playing
    */
    protected void updateTitleSummary(int index, boolean isPlaying) {
        if (isPlaying) {
            mTitle.setText("");
            mSummary.setText("");
            mLeftBtn.setVisibility(View.INVISIBLE);
        } else {
            mTitle.setText(TITLE[index]);
            mSummary.setText(SUMMARY[index]);
        }
    }

    /*
    * release media player
    */
    private void releaseMediaPlayer() {
        if (mMediaPlayer != null) {
            mMediaPlayer.release();
            mMediaPlayer = null;
        }
    }

    /*
    * reset surface size
    */
    public void resizeSurfaceView() {
        Xlog.d(TAG, "resizeSurfaceView()");
        if (mSetScreen) {
            return;
        } else {
            mSetScreen = true;
        }
        int videoW = mMediaPlayer.getVideoWidth();
        int videoH = mMediaPlayer.getVideoHeight();
        int screenW = getWindowManager().getDefaultDisplay().getWidth();
        int screenH = getWindowManager().getDefaultDisplay().getHeight();
        android.view.ViewGroup.LayoutParams lp = mPreview.getLayoutParams();

        float videoScale = (float)videoH / (float)videoW;
        float screenScale = (float)screenH / (float)screenW;
        if (screenScale > videoScale) {
            lp.width = screenW;
            lp.height = (int)(videoScale * (float)screenW);
            Xlog.d(TAG, "screenScale > videoScale");
        } else {
            lp.height = screenH;
            lp.width = (int)((float)screenH / videoScale);
            Xlog.d(TAG, "screenScale < videoScale");
        }
        mPreview.setLayoutParams(lp);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mIsFirstRun) {
                // do not handle back key for the first time
                return true;
            } else {
                releaseMediaPlayer();
            }
        }
        return super.onKeyDown(keyCode, event);
    }

    private void finishQsg() {
        if (mIsFirstRun) {
            Utils.startLauncher(this);
        }
        finish();
    }
}
