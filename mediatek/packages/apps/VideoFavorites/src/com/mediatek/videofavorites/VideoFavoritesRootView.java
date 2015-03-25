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
 */

package com.mediatek.videofavorites;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.RemoteViews.RemoteView;

import com.mediatek.common.widget.IMtkWidget;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;


@RemoteView
public class VideoFavoritesRootView extends RelativeLayout implements IMtkWidget,
    View.OnClickListener {
    private static final String TAG = "VFRootView";

    private static final int MAX_VIDEO_CAN_PLAY = 2;
    // for optmization, only keep first 4 video views found in list
    // this value should be adjusted if we are going to support infinite number of videos
    private static final int MAX_VIDEO_VIEW = WidgetAdapter.LARGE_MAX_NUM_VIDEOS;
    private static final int MAX_ALLOWED_WIDGET_COUNT = 5;  // will be ignore by IMtkWidget now

    private static final int VIDEO_DELAY_TIME_MS = 200;
    private static final int VIDEO_DELAY_TIME_MS_RESUME = 300;
    private static final int VIDEO_DELAY_TIME_MS_SHORT = 50;
    private static final int VIDEO_DElAY_REFRESH_PLAY = 1000;   // delay of play after refresh

    private static final int MSG_START_RANDOM_PLAY = 1;
    private static final int MSG_START_NEXT_VIDEO_VIEW = 2;
    private static final int MSG_PAGE_SWITCH_OUT = 3;
    private static final int MSG_BROCAST_REFRESH = 4;
    private static final int MSG_ENABLE_DELETE_MODE = 5;

    private static final int DETACHED = 0;
    private static final int IDLE = 1;
    private static final int DRAG = 2;
    private static final int MOVING_OUT = 3;
    private static final int PAUSED = 4;

    private final VideoTextureView [] mVideoActive = new VideoTextureView[MAX_VIDEO_CAN_PLAY];

    private boolean mIsDeleteMode;
    private boolean mIsListeningIntent;
    // true if any drive has been mounted, false if any drive
    // started to ejected.
    private boolean mIsUnmounting;
    private int mDeleteModeViewId = View.NO_ID;
    private int mDeleteIconId = View.NO_ID;
    private int mScreenIndex;
    private int mState;
    private int mVideoActiveCount;
    private int mWidgetId;

    private AbsListView mListView;
    private Random mRandom;

    public static final String ACTION_DATA_UPDATE = "com.mediatek.videofavorites.DATA_UPDATE";

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_START_RANDOM_PLAY:
                Xlog.d(TAG, "rebuild video list");

                if (!buildVideoViewList()) {
                    // tricky, maybe still loading, so if num of img view ==0,
                    // we wait longer to retry again
                    startPlayVideoRandomly(mNumOfImgView == 0 ? 1000 : 200);
                    return;
                }
                showVideoAndRandomStrart();
                break;

            case MSG_START_NEXT_VIDEO_VIEW:
                showVideoAndRandomStrart();
                break;

            case MSG_PAGE_SWITCH_OUT:
                onPageSwitchOutInternal();
                break;

            case MSG_BROCAST_REFRESH:
                sendRefreshBroadcast();
                break;

            case MSG_ENABLE_DELETE_MODE:
                setDeleteMode(true);
                break;

            default:
                break;
            }
        }
    };

    private boolean mRefreshWhenBackToLauncher;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Xlog.d(TAG, "Received intent action=" + action);
            if (action.equals(AbsVideoFavoritesWidget.ACTION_REFRESH)) {
                removePlaybackMessages();
                stopAllVideoView();
                if (mIsUnmounting == false && mState == IDLE) {
                    // there's no callback for updating contents, so we give some time for reload.
                    startPlayVideoRandomly(VIDEO_DElAY_REFRESH_PLAY);
                }
                int favoriteCount = getFavoriteCount();
                setRecordButtonStatus(favoriteCount < WidgetAdapter.LARGE_MAX_NUM_VIDEOS);
                setEditButtonStatus(favoriteCount > 0);
            } else if (action.equals(ACTION_DATA_UPDATE)) {
                mRefreshWhenBackToLauncher = true;
            } else if (action.equals(Intent.ACTION_MEDIA_EJECT)) {
                mIsUnmounting = true;
                removePlaybackMessages();
                stopAllVideoView();
            } else if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {
                mIsUnmounting = false;
                sendRefreshBroadcast(1000);
            } else if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {
                mIsUnmounting = false;
                sendRefreshBroadcast(200);
            }
        }
    };

    public VideoFavoritesRootView(Context context) {
        super(context);
    }

    public VideoFavoritesRootView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public VideoFavoritesRootView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    private void sendRefreshBroadcast(int delay) {
        mHandler.removeMessages(MSG_BROCAST_REFRESH);
        if (delay == 0) {
            sendRefreshBroadcast();
        } else {
            mHandler.sendEmptyMessageDelayed(MSG_BROCAST_REFRESH, delay);
        }
    }

    private void removePlaybackMessages() {
        mHandler.removeMessages(MSG_START_NEXT_VIDEO_VIEW);
        mHandler.removeMessages(MSG_START_RANDOM_PLAY);
    }

    private void installIntentFilter() {
        Xlog.d(TAG, "installIntentFilter(): " + !mIsListeningIntent);
        if (!mIsListeningIntent) {
            IntentFilter intentFilter = new IntentFilter(Intent.ACTION_MEDIA_EJECT);
            intentFilter.addDataScheme("file");
            intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
            intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
            getContext().registerReceiver(mReceiver, intentFilter);

            IntentFilter intentRefresh = new IntentFilter(AbsVideoFavoritesWidget.ACTION_REFRESH);
            getContext().registerReceiver(mReceiver, intentRefresh);

            // data update from action activity
            IntentFilter intentUpdate = new IntentFilter(ACTION_DATA_UPDATE);
            getContext().registerReceiver(mReceiver, intentUpdate);

            mIsListeningIntent = true;
        }
    }

    private void uninstallIntentFilter() {
        Xlog.d(TAG, "uninstallIntentFilter(): " + mIsListeningIntent);
        if (mIsListeningIntent) {
            getContext().unregisterReceiver(mReceiver);
            mIsListeningIntent = false;
        }
    }

    private AbsListView findFirstAbsListView() {
        final int imax = getChildCount();
        Xlog.v(TAG, "findFirstAbsListView()");
        for (int i = 0; i < imax; i++) {
            View v = getChildAt(i);
            if (v instanceof AbsListView) {
                return (AbsListView)v;
            }
        }
        return null;
    }

    private AbsListView getListView() {
        if (mListView == null) {
            mListView = findFirstAbsListView();
        }
        return mListView;
    }

    // Count the number of text view to make sure we got correct count of video view items
    private int mNumOfImgView;
    private int mNumOfEmptyView;
    // find video views recursively
    private void addVideoViews(View v) {
        if (v instanceof ViewGroup) {
            final ViewGroup vg = (ViewGroup) v;
            if (v instanceof FrameLayout && v.getId() == R.id.empty_root) {
                // empty
                mNumOfEmptyView++;
                return;
            }
            final int count = vg.getChildCount();
            for (int i = 0; i < count; i++) {
                addVideoViews(vg.getChildAt(i));
            }
        } else if (v instanceof VideoTextureView) {
            mVideoViews.add((VideoTextureView)v);
        } else if (v instanceof ImageView) {
            mNumOfImgView++;
        }
    }

    private void setRecordButtonStatus(boolean enable) {
        Xlog.d(TAG, "setRecordButtonStatus():" + enable);
        View recordBtn = findViewById(R.id.btn_record);
        if (recordBtn.getVisibility() == View.VISIBLE) {
            recordBtn.setClickable(enable);
            recordBtn.setEnabled(enable);
        }
    }

    private void setEditButtonStatus(boolean enable) {
        Xlog.d(TAG, "seteditButtonsStatus: " + enable);
        View edit = findViewById(R.id.btn_edit);
        edit.setClickable(enable);
        edit.setEnabled(enable);
    }

    private final ArrayList<VideoTextureView> mVideoViews = new ArrayList<VideoTextureView>();

    // return "true" if the list is built successfully
    private boolean buildVideoViewList() {
        View rootView = (View)getListView();
        if (rootView == null) {
            Xlog.e(TAG, "rootview not found");
            return false;
        }
        mVideoViews.clear();
        mNumOfImgView = 0;
        mNumOfEmptyView = 0;
        addVideoViews(rootView);
        // we were unable to detect the # of videoview very correctly
        // sometimes there are 4 videos but we can only find 3.
        // so use number of image view and empty view to double verify.

        // 4 list_item or (n list_item + 1 list_item_new)
        Xlog.d(TAG, "numofImgView: " + mNumOfImgView);
        setEditButtonStatus(mNumOfEmptyView < (MAX_VIDEO_VIEW - 1));
        if (getListView() != null && (
                    getListView().getChildCount() * 2 == ((mNumOfImgView) + mNumOfEmptyView * 2)
                    || getListView().getChildCount() * 2
                    == ((mNumOfImgView + 1) + mNumOfEmptyView * 2))) {
            return true;
        }

        Xlog.d(TAG, "not all videoViews being found, Retry later");
        return false;
    }

    private void stopAllVideoViewAsync() {
        Xlog.v(TAG, "stopAllVideoView()");
        removePlaybackMessages();
        if (mVideoActiveCount == 0) {
            return;
        }
        VideoTextureView v;
        for (int i = 0; i < MAX_VIDEO_CAN_PLAY; i++) {
            v = mVideoActive[i];
            if (v == null) {
                continue;
            }
            v.pause();
            v.seekTo(0);
            v.cancelAnimation();
            v.prepareAnimation();
            v.setVisibility(View.INVISIBLE);
            mVideoActive[i] = null;
            v.stopPlaybackAsync();
        }
        mVideoActiveCount = 0;
    }

    private void stopAllVideoView() {
        Xlog.v(TAG, "stopAllVideoView()");
        removePlaybackMessages();
        if (mVideoActiveCount == 0) {
            return;
        }
        VideoTextureView v;
        for (int i = 0; i < MAX_VIDEO_CAN_PLAY; i++) {
            v = mVideoActive[i];
            if (v == null) {
                continue;
            }
            v.pause();
            v.seekTo(0);
            v.cancelAnimation();
            v.prepareAnimation();
            v.setVisibility(View.INVISIBLE);
            mVideoActive[i] = null;
            v.stopPlayback();
        }
        removePlaybackMessages();
        mVideoActiveCount = 0;
        Xlog.v(TAG, "stopAllVideoView() done");
    }

    private void pauseAllVideoView() {
        Xlog.v(TAG, "pauseAllVideoView()");
        for (int i = 0; i < MAX_VIDEO_CAN_PLAY; i++) {
            if (mVideoActive[i] == null) {
                continue;
            }
            mVideoActive[i].pause();
        }
        removePlaybackMessages();
    }

    private void resumeAllVideoView() {
        Xlog.v(TAG, "resumeAllVideoView()");
        for (int i = 0; i < MAX_VIDEO_CAN_PLAY; i++) {
            if (mVideoActive[i] == null) {
                continue;
            }
            mVideoActive[i].start();
        }
    }

    private boolean isInActiveList(RemoteVideoView v) {
        return Arrays.asList(mVideoActive).contains(v);
    }

    // insert into first free slot;
    private void addVideoToActiveList(RemoteVideoView v) {
        int index;
        for (index = 0; index < MAX_VIDEO_CAN_PLAY; index++) {
            if (mVideoActive[index] == null) {
                break;
            }
        }

        if (index >= MAX_VIDEO_CAN_PLAY) {
            Xlog.e(TAG, "unable to add Video into Active List");
            return;
        }
        mVideoActive[index] = v;
        mVideoActiveCount++;
    }

    RemoteVideoView getFirstPlayableVideoView(int availableVideos) {
        RemoteVideoView v;
        for (int i = 0; i < availableVideos; i++) {
            v = (RemoteVideoView) mVideoViews.get(i);
            if (v.isUriAvailable() && !isInActiveList(v)) {
                Xlog.d(TAG, "getFirstPlayableVideoView(), try#" + i + ", found.");
                return v;
            } else {
                // Add log since it's a fail safe function.
                Xlog.d(TAG, "getFirstPlayableVideoView(), try#" + i
                    + "isUrlAvailable(): " + v.isUriAvailable()
                    + "isInActiveList(): " + isInActiveList(v));
            }
        }
        return null;
    }

    private void showVideoAndRandomStrart() {
        final int availableVideos = mVideoViews.size();
        final int count = (availableVideos > MAX_VIDEO_CAN_PLAY
                           ? MAX_VIDEO_CAN_PLAY : availableVideos) - mVideoActiveCount;
        Xlog.i(TAG, "showVideoAndRandomStrart, available:"
               + availableVideos + " Active" + mVideoActiveCount + " count:" + count);

        if (count <= 0) {
            return;
        }

        int trial = 6;      // magic number
        RemoteVideoView v;
        do {
            int r = mRandom.nextInt(availableVideos);
            v = (RemoteVideoView) mVideoViews.get(r);
            trial--;
            if (trial == 0) {
                // Unable to choose a video view randomly, so get first available one.
                v = getFirstPlayableVideoView(availableVideos);
                if (v == null) {
                    Xlog.e(TAG, "Unable to find a valid VideoView");
                    return;
                }
                break;
            }
        } while (!v.isUriAvailable() || isInActiveList(v));
        String path = v.getUriPath();
        if (path != null && !("".equals(path))) {
            File f = new File(path);
            if (!f.exists()) {
                String scheme = v.getUriScheme();
                if (!ContentResolver.SCHEME_ANDROID_RESOURCE.equals(scheme)) {
                    Xlog.e(TAG, "file is not found");
                    sendRefreshBroadcast();
                    return;
                }
            }
        }

        v.setOnCompletionListener(mVideoCompleteListener);
        v.setOnErrorListener(mVideoErrorListener);
        if (!v.isInPlaybackState()) {
            Xlog.v(TAG, "openVideo()");
            v.openVideo();
        }

        Xlog.v(TAG, "start()");
        v.start();
        // error may occur during open / start
        if (v.isInErrorState()) {
            v.cancelAnimation();
            v.prepareAnimation();
            v.setVisibility(View.INVISIBLE);
        } else {
            v.setVisibility(View.VISIBLE);
            addVideoToActiveList(v);
        }

        if (count > 1) {
            mHandler.removeMessages(MSG_START_NEXT_VIDEO_VIEW);
            mHandler.sendEmptyMessageDelayed(MSG_START_NEXT_VIDEO_VIEW, 3000);
        }
    }

    private void sendRefreshBroadcast() {

        Intent i = new Intent(AbsVideoFavoritesWidget.ACTION_REFRESH);
        getContext().sendBroadcast(i);
    }

    // implmentations for IMtkWidget
    ////////////////////////////////

    /**
     * The count should be installed in launcher.
     */
    public int getPermittedCount() {
        return MAX_ALLOWED_WIDGET_COUNT;   // only 1 widget allowed.
    }

    public void setWidgetId(int widgetId) {
        mWidgetId = widgetId;
    }

    public int getWidgetId() {
        return mWidgetId;
    }

    public void setScreen(int screen) {
        mScreenIndex = screen;
    }

    public int getScreen() {
        return mScreenIndex;
    }

    public void setState(int newState) {
        Xlog.d(TAG, "setState:" + mState + " -> " + newState);
        mState = newState;
    }

    public void startDrag() {
        Xlog.d(TAG, "startDrag()");

        stopAllVideoView();
        setState(DRAG);
        if (mIsDeleteMode) {
            setDeleteMode(false);
        }
    }

    public void stopDrag() {
        Xlog.d(TAG, "stopDrag()");
        startPlayVideoRandomly(VIDEO_DELAY_TIME_MS_SHORT);
        setState(IDLE);
    }

    public void startCovered(int curScreen) {
        Xlog.d(TAG, "startCovered()");
        setState(MOVING_OUT);
        onPageSwitchOut();
        if (mIsDeleteMode) {
            setDeleteMode(false);
        }
    }

    public void stopCovered(int curScreen) {
        if (mState == PAUSED) {
            Xlog.d(TAG, "stopCovered() ignored");
            return;
        }
        Xlog.d(TAG, "stopCovered()");
        onPageSwitchIn(400);
    }


    public void onPauseWhenShown(int curScreen) {
        Xlog.d(TAG, "onPauseWhenShown()");
        stopAllVideoViewAsync();
        if (mIsDeleteMode) {
            setDeleteMode(false);
        }
        setState(PAUSED);
    }

    public void onResumeWhenShown(int curScreen) {
        Xlog.d(TAG, "onResumeWhenShown()");
        mDeleteIcons.clear();
        mIsDeleteMode = false;

        setState(IDLE);
        if (mRefreshWhenBackToLauncher) {
            // only update here since only we can only enter action when widget on screen
            Xlog.v(TAG, "enqueue sendRefreshBroadcast");
            sendRefreshBroadcast(200);
            mRefreshWhenBackToLauncher = false;
        } else {
            startPlayVideoRandomly(VIDEO_DELAY_TIME_MS_RESUME);
        }
    }

    private static final boolean STOP_VIDEO_ABRUPT_WHEN_PAGE_SWITCH = false;

    private void onPageSwitchOutInternal() {
        stopAllVideoViewAsync();
        if (mIsDeleteMode) {
            setDeleteMode(false);
        }
    }

    private void onPageSwitchOut() {
        removePlaybackMessages();
        mHandler.sendEmptyMessageDelayed(MSG_PAGE_SWITCH_OUT, 100);
        setState(MOVING_OUT);
    }

    private void onPageSwitchIn(int delay) {
        Xlog.d(TAG, "onPageSwitchIn");
        if (mState == IDLE || mState == PAUSED) {
            Xlog.e(TAG, "State is invalid: " + mState + ", ignore");
            return;
        }
        startPlayVideoRandomly(delay);
        mHandler.removeMessages(MSG_PAGE_SWITCH_OUT);
        setState(IDLE);
    }

    private void onPageSwitchIn() {
        onPageSwitchIn(VIDEO_DELAY_TIME_MS_SHORT);
    }


    public boolean moveOut(int curScreen) {
        if (STOP_VIDEO_ABRUPT_WHEN_PAGE_SWITCH) {
            onPageSwitchOut();
        }
        return true;
    }

    public void moveIn(int curScreen) {
        if (STOP_VIDEO_ABRUPT_WHEN_PAGE_SWITCH) {
            onPageSwitchIn();
        }
    }

    public void leaveAppwidgetScreen() {
        Xlog.d(TAG, "leaveAppwidgetScreen()");
        if (!STOP_VIDEO_ABRUPT_WHEN_PAGE_SWITCH) {
            onPageSwitchOut();
        }
    }

    public void enterAppwidgetScreen() {
        Xlog.d(TAG, "enterAppwidgetScreen()");
        if (!STOP_VIDEO_ABRUPT_WHEN_PAGE_SWITCH) {
            onPageSwitchIn();
        }
    }

    public void onSaveInstanceState(Bundle outSate) {
        // do nothing
    }

    public void onRestoreInstanceState(Bundle state) {
        // do nothing
    }

    private final ArrayList<View> mDeleteIcons = new ArrayList<View>();
    private void addViews(View v, int viewId, ArrayList<View> views) {
        if (v.getId() == viewId) {
            views.add(v);
        }

        if (v instanceof ViewGroup) {
            final ViewGroup vg = (ViewGroup) v;
            final int count = vg.getChildCount();
            for (int i = 0; i < count; i++) {
                addViews(vg.getChildAt(i), viewId, views);
            }
        }
    }

    // this method updates the videoViewList
    private void findAllViewsById(int viewId, ArrayList<View> views) {
        View rootView = getListView();

        if (rootView == null) {
            Xlog.e(TAG, "rootview not found");
            return;
        }
        views.clear();
        addViews(rootView, viewId, views);
    }

    private void switchDeleteMode() {
        setDeleteMode(!mIsDeleteMode);
        if (!mIsDeleteMode) {
            startPlayVideoRandomly(VIDEO_DELAY_TIME_MS);
        }
    }

    private void setDeleteMode(boolean isDelete) {
        Xlog.v(TAG, "setDeleteMode(): " + isDelete);
        stopAllVideoViewAsync();
        mIsDeleteMode = isDelete;
        mDeleteIcons.clear();
        findAllViewsById(mDeleteIconId, mDeleteIcons);

        int visibility = mIsDeleteMode ? View.VISIBLE : View.INVISIBLE;
        for (View v : mDeleteIcons) {
            v.setVisibility(visibility);
        }
    }

    @Override
    public void onClick(View v) {
        Xlog.v(TAG, "ViewId clicked: " + v.getId());
        if (v.getId() == mDeleteModeViewId) {
            switchDeleteMode();
        }
    }

    @android.view.RemotableViewMethod
    public void setDeleteModeViewId(int viewId) {
        mDeleteModeViewId = viewId;
        View child = findViewById(viewId);

        if (child == null) {
            Xlog.e(TAG, "delete view not found!");
        } else {
            child.setOnClickListener(this);
        }
    }

    @android.view.RemotableViewMethod
    public void setDeleteIconId(int viewId) {
        mDeleteIconId = viewId;
    }

    private void restartVideoViews() {
        stopAllVideoView();
        if (mState == IDLE) {
            startPlayVideoRandomly(VIDEO_DELAY_TIME_MS_RESUME);
        } else {
            Xlog.w(TAG, "mState: " + mState);
        }
    }

    private final OnHierarchyChangeListener mHierarchyChangeListener =
            new OnHierarchyChangeListener() {
                // The invoking order of onChildViewAdded and onChildViewRemoved() is not promised.
                // in ICS, will remove, then add,  but in JB it adds first, then remove.
                public void onChildViewAdded(View parent, View child) {
                    Xlog.d(TAG, "onChildViewAdded");
                    if (mIsDeleteMode) {
                        mHandler.sendEmptyMessage(MSG_ENABLE_DELETE_MODE);
                    } else {
                        restartVideoViews();
                    }
                }

                public void onChildViewRemoved(View parent, View child) {
                    Xlog.d(TAG, "onChildViewRemoved");
                    if (!mIsDeleteMode) {
                        restartVideoViews();
                    }
                }
            };

    @Override
    protected void onAttachedToWindow() {
        Xlog.v(TAG, "onAttachedToWindow():" + this);
        super.onAttachedToWindow();
        if (mState == PAUSED) {
            Xlog.v(TAG, "Attched again after onResume, remain in current State:" + mState);
        } else {
            setState(IDLE);
        }
        if (mRandom == null) {
            mRandom = new Random();
        }

        if (getListView() == null) {
            Xlog.e(TAG, "Failed to set onChildViewChangeListener");
        } else {
            getListView().setOnHierarchyChangeListener(mHierarchyChangeListener);
        }

        installIntentFilter();
        requestLayout();
        if (mState == IDLE) {
            sendRefreshBroadcast(1000);
        } else {
            // Detach then attached during launcher is paused
            // delay refresh when we back to launcher
            mRefreshWhenBackToLauncher = true;
        }
    }

    protected void onDetachedFromWindow() {
        Xlog.v(TAG, "onDetachedFromWindow():" + this);
        uninstallIntentFilter();
        stopAllVideoView();
        setState(DETACHED);

        if (mHandler.hasMessages(MSG_BROCAST_REFRESH)) {
            mHandler.removeMessages(MSG_BROCAST_REFRESH);
        }
    }

    @android.view.RemotableViewMethod
    public void startPlayVideoRandomly(int delay) {
        Xlog.v(TAG, "startvideoplay");
        removePlaybackMessages();

        if (mIsUnmounting) {
            Xlog.d(TAG, "some media is ejecting, return");
            return;
        }
        mHandler.sendEmptyMessageDelayed(MSG_START_RANDOM_PLAY, delay);
    }

    private final OnCompletionListener mVideoCompleteListener = new OnCompletionListener() {
        public void onCompletion(MediaPlayer mp) {
            Xlog.v(TAG, "onCompletion:" + mState);

            if (mState != IDLE) {
                Xlog.v(TAG, "onCompletion, ignore, state: " + mState);
                return;
            }
            int index = -1;
            VideoTextureView v;
            for (index = 0; index < MAX_VIDEO_CAN_PLAY; index++) {
                v = mVideoActive[index];
                if (v != null) {
                    if (v.isSameMediaPlayer(mp)) {
                        mp.seekTo(0);
                        v.cancelAnimation();
                        v.prepareAnimation();
                        v.setVisibility(View.INVISIBLE);
                        mVideoActive[index] = null;
                        mVideoActiveCount--;
                        break;
                    }
                }
            }

            if (index == MAX_VIDEO_CAN_PLAY) {
                Xlog.e(TAG, "not found");
            }
            startPlayVideoRandomly(1000);
        }
    };

    private final OnErrorListener mVideoErrorListener = new OnErrorListener() {
        public boolean onError(MediaPlayer mp, int what, int extra) {
            Xlog.e(TAG, "Error: " + what);
            VideoTextureView v;
            for (int index = 0; index < MAX_VIDEO_CAN_PLAY; index++) {
                v = mVideoActive[index];
                if (v != null && v.isSameMediaPlayer(mp)) {
                    v.cancelAnimation();
                    v.prepareAnimation();
                    v.setVisibility(View.INVISIBLE);
                    mVideoActive[index] = null;
                    mVideoActiveCount--;
                    break;
                }
            }
            sendRefreshBroadcast();
            return true;
        }
    };

    private float mLastY;   // NOPMD - LastY have to be keep with member variable in this case
    private static final int INTERCEPT_THRESHOLD = 10;
    // intercept scroll of gridview
    // return true to intercept
    @Override
    public boolean onInterceptTouchEvent(MotionEvent event) {
        final int action = event.getAction();
        switch (action & MotionEvent.ACTION_MASK) {
        case MotionEvent.ACTION_DOWN:
            mLastY = event.getY();
            break;

        case MotionEvent.ACTION_MOVE:
            if (Math.abs(event.getY() - mLastY) > INTERCEPT_THRESHOLD) {
                return true;
            }
            break;

        case MotionEvent.ACTION_SCROLL:
            return true;

        default:
            /* do nothing */
        }
        return false;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        final int action = event.getAction();
        switch (action & MotionEvent.ACTION_MASK) {
        case MotionEvent.ACTION_MOVE:
        case MotionEvent.ACTION_SCROLL:
            // touch Event goes here after interception
            // just return false and let launcher handle it.
            break;
        default:
            return super.onTouchEvent(event);
        }
        return false;
    }

    private static final String [] PROJ_COLS = new String[] {"count(*)"};
    private int getFavoriteCount() {
        Cursor cur = getContext().getContentResolver().query(
                         VideoFavoritesProviderValues.Columns.CONTENT_URI, PROJ_COLS,
                         null, null, null);
        cur.moveToFirst();
        final int cnt = cur.getInt(0);
        cur.close();
        Xlog.i(TAG, "getFavoriteCount():"  + cnt);
        return cnt;
    }

    /*
     *  for instrument test only
     */
    public boolean isDeleteMode() {
        return mIsDeleteMode;
    }

    public int getVideoCount() {
        return mVideoViews.size();
    }

    public static final void sendDataUpdateBroadcast(Context c) {
        Intent i = new Intent(ACTION_DATA_UPDATE);
        c.sendBroadcast(i);
    }
}
