/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.plugin.phone;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothHeadset;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.view.OrientationEventListener;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.mediatek.rcse.api.Logger;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.video.Orientation;
import com.orangelabs.rcs.core.ims.service.ImsServiceError;
import com.orangelabs.rcs.core.ims.service.richcall.ContentSharingError;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.media.video.LiveVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.video.VideoRenderer;
import com.orangelabs.rcs.service.api.client.media.video.VideoSurfaceView;
import com.orangelabs.rcs.service.api.client.richcall.IVideoSharingEventListener;
import com.orangelabs.rcs.service.api.client.richcall.IVideoSharingSession;
import com.orangelabs.rcs.utils.PhoneUtils;

/**
 * This class defined to implement the function interface of ICallScreenPlugIn,
 * and acheive  the main function here.When user starts a video share the method
 * {@link #start(String)} will be called. If client receive a video share, it
 * will go to {@link VideoSharingInvitationReceiver#onReceive(Context, Intent)}.
 */
public class VideoSharingPlugin extends SharingPlugin implements SurfaceHolder.Callback, OnClickListener,
DialogInterface.OnClickListener, SensorEventListener{
	private static final String TAG = "VideoSharingPlugin";
	/* package */static final String VIDEO_SHARING_INVITATION_ACTION =
			"com.orangelabs.rcs.richcall.VIDEO_SHARING_INVITATION";
	/* package */static final String VIDEO_SHARING_ACCEPT_ACTION =
			"com.mediatek.phone.plugin.VIDEO_SHARING_ACCEPT_ACTION";
	/* package */static final String VIDEO_SHARING_DECLINE_ACTION =
			"com.mediatek.phone.plugin.VIDEO_SHARING_DECLINE_ACTION";

	/* package */static final String SERVICE_STATUS = "com.orangelabs.rcs.SERVICE_STATUS";
	/* package */static final String SERVICE_REGISTRATION =
			"com.orangelabs.rcs.SERVICE_REGISTRATION";
	/* package */static final String VIDEO_NAME = "videoName";
	/* package */static final String VIDEO_DURATION = "videoDuration";
	/* package */static final String VIDEO_ENCODING = "videoEncoding";
	/* package */static final String VIDEO_WIDTH = "videoWidth";
	/* package */static final String VIDEO_HEIGHT = "videoHeight";
	private static final String QVGA = "QVGA";
	private static final String TEL_URI_SCHEMA = "tel:";
	private static final String CAMERA_ID = "camera-id";
	private static final int QVGA_WIDTH = 480;
	private static final int QVGA_HEIGHT = 320;
	private static final int QCIF_WIDTH = 176;
	private static final int QCIF_HEIGHT = 144;
	private static final int MIN_WINDOW_WIDTH = 83;
	private static final int MIN_WINDOW_HEIGHT = 133;
	private static final int SWITCH_BUTTON_WIDTH = 70;
	private static final int SWITCH_BUTTON_HEIGHT = 70;
	private static final int TERMINATE_BUTTON_WIDTH = 32;
	private static final int TERMINATE_BUTTON_HEIGHT = 32;
	private static final int SWITCH_BUTTON_MARGIN_TOP = 20;
	private static final int SWITCH_BUTTON_MARGIN_BOTTOM = 0;
	private static final int SWITCH_BUTTON_MARGIN_LEFT = 0;
	private static final int SWITCH_BUTTON_MARGIN_RIGHT = 20;
	private static final int TERMINATE_BUTTON_MARGIN_LEFT = 20;
	private static final int TERMINATE_BUTTON_MARGIN_RIGHT = 20;
	private static final int LOCAL_CAMERA_MARGIN_TOP = 10;
	private static final int LOCAL_CAMERA_MARGIN_BOTTOM = 0;
	private static final int LOCAL_CAMERA_MARGIN_LEFT = 0;
	private static final int LOCAL_CAMERA_MARGIN_RIGHT = 10;
	private static final int DELAY_TIME = 500;
	private static final String CMA_MODE = "cam-mode";
	private static final int ROTATION_BY_HW = 3;
	private static final int SINGLE_CAMERA = 1;
	private static final int ROTATION_0 = 0;
	private static final int ROTATION_90 = 90;
	private static final int ROTATION_180 = 180;
	private static final int ROTATION_270 = 270;
	private int mShareStatus = ShareStatus.UNKNOWN;
    private OrientationEventListener mOrientationListener;
    private int mOrient = 0;
    boolean mVideoRotated = false;

	// Indicate whether click start button
	private final AtomicBoolean mIsStarted = new AtomicBoolean(false);
	private final AtomicInteger mVideoSharingState =
			new AtomicInteger(Constants.SHARE_VIDEO_STATE_IDLE);
	private RelativeLayout mOutgoingDisplayArea = null;
	private ViewGroup mIncomingDisplayArea = null;
	// In current phase ,just care live video share, as a result it's value
	// never false.
	// Surface holder for video preview
	private SurfaceHolder mPreviewSurfaceHolder = null;
	private Camera mCamera = null;
	private final Object mLock = new Object();
	// Camera preview started flag
	private boolean mCameraPreviewRunning = false;
	private IVideoSharingSession mOutgoingVideoSharingSession = null;
	private LiveVideoPlayer mOutgoingVideoPlayer = null;
	private VideoSurfaceView mOutgoingLocalVideoSurfaceView = null;
	private VideoSurfaceView mOutgoingRemoteVideoSurfaceView = null;
	private VideoRenderer mOutgoingRemoteVideoRenderer = null;
	private String mOutgoingVideoFormat = null;
	private int mVideoWidth = 176;
	private int mVideoHeight = 144;
	// Video surface holder
	private SurfaceHolder mSurfaceHolder;
	private final AtomicBoolean mIsVideoSharingSender = new AtomicBoolean(false);
	private final AtomicBoolean mIsVideoSharingReceiver = new AtomicBoolean(false);


	// For incoming video share information
	private volatile IVideoSharingSession mIncomingVideoSharingSession = null;
	private VideoSurfaceView mIncomingLocalVideoSurfaceView = null;
	private VideoSurfaceView mIncomingRemoteVideoSurfaceView = null;
	private VideoRenderer mIncomingRemoteVideoRenderer = null;
	private int mCamerNumber = 0;
	private int mOpenedCameraId = 0;
	String mIncomingSessionId = null;
	String mIncomingVideoFormat = null;
	private ImageButton mAudioButton = null;

	private ImageView mSwitchCamerImageView = null;
	private ImageView mEndSenderSessionImageView = null;
	private ImageView mEndReceiverSessionImageView = null;
	private final AtomicBoolean mGetIncomingSessionWhenApiConnected = new AtomicBoolean(false);
	private final AtomicBoolean mStartOutgoingSessionWhenApiConnected = new AtomicBoolean(false);
	// Save a set of number to be listened, the number was passed from phone
	private final CopyOnWriteArraySet<String> mNumbersToBeListened =
			new CopyOnWriteArraySet<String>();
	private final CopyOnWriteArraySet<CallScreenDialog> mCallScreenDialogSet =
			new CopyOnWriteArraySet<CallScreenDialog>();
	private List<CallScreenDialog> mCallScreenSavedDialogSet =
			new ArrayList<CallScreenDialog>();
	private WaitingProgressDialog mWaitingProgressDialog = null;
	private VideoSharingDialogManager mVideoSharingDialogManager = null;

	private long mVideoDuration;
	public static final String VIDEO_LIVE = "videolive";
	private static final String AUDIO_BUTTON = "audioButton";
	private static final String VIEW_ID = "id";
	private boolean mHeadsetConnected = false;



	private VideoSharingInvitationReceiver mVideoSharingInvitationReceiver = new VideoSharingInvitationReceiver();

	private ISharingPlugin mVideoSharingCallBack = new ISharingPlugin() {

		@Override
		public void onApiConnected() {
			Logger.d(TAG, "onApiConnected() entry!");
			mCountDownLatch.countDown();
			AsyncTask.execute(new Runnable() {
				@Override
				public void run() {
					// there may be an case that content sharing plugin starts
					// earlier than core service, so we need to refresh data
					// that may be modified by auto-configuration
					PhoneUtils.initialize(mContext);
				}
			});
			if (mGetIncomingSessionWhenApiConnected.compareAndSet(true, false)) {
				Logger.v(TAG, "onApiConnected(), " +
						"Richcall api connected, and need to get incoming video share session");
				getIncomingVideoSharingSession();
			} else {
				Logger.v(TAG, "onApiConnected(), " +
						"Richcall api connected, but need not to get incoming video share session");
			}
			if (mStartOutgoingSessionWhenApiConnected.compareAndSet(true, false)) {
				Logger.v(TAG, "onApiConnected(), " +
						"Richcall api connected, and need to start outgoing video share session");
				startOutgoingVideoShareSession();
			} else {
				Logger.v(TAG, "onApiConnected(), " +
						"Richcall api connected, but need not to start outgoing video share session");
			}
		}

		@Override
		public void onFinishSharing() {
			destroy();
		}

	};

	/**
	 * Constructor
	 * 
	 * @param ctx Application context
	 */
	public VideoSharingPlugin(Context context) {
		super(context);
		Logger.v(TAG, "VideoSharingPlugin constructor. context = " + context);
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(VIDEO_SHARING_INVITATION_ACTION);
		intentFilter.addAction(VIDEO_SHARING_ACCEPT_ACTION);
		intentFilter.addAction(VIDEO_SHARING_DECLINE_ACTION);
		intentFilter.addAction(Intent.ACTION_HEADSET_PLUG);
		intentFilter.addAction(BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED);
		mContext.registerReceiver(mVideoSharingInvitationReceiver, intentFilter);
		new AsyncTask<Void, Void, Void>() {

			@Override
			protected Void doInBackground(Void... params) {
				Logger.v(TAG, "CallScreenPlugin constructor. Thread start run.");
				if (RcsSettings.getInstance() != null) {
					mOutgoingVideoFormat = RcsSettings.getInstance().getCShVideoFormat();
					String cshVideoSize = RcsSettings.getInstance().getCShVideoSize();
					Logger.v(TAG, "cshVideoSize = " + cshVideoSize);
					if (QVGA.equals(cshVideoSize)) {
						// QVGA
						mVideoWidth = QVGA_WIDTH;
						mVideoHeight = QVGA_HEIGHT;
					} else {
						// QCIF
						mVideoWidth = QCIF_WIDTH;
						mVideoHeight = QCIF_HEIGHT;
					}
				} else {
					Logger.e(TAG, "RcsSettings.getInstance() return null");
				}
				Logger.v(TAG, "mOutgoingVideoFormat = " + mOutgoingVideoFormat);
				//mOutgoingVideoFormat = "h264";
				Logger.v(TAG, "mOutgoingVideoFormat = " + mOutgoingVideoFormat);
				AndroidFactory.setApplicationContext(mContext);
				// Get the number of the camera
				mCamerNumber = Utils.getCameraNums();
				Logger.v(TAG, "mCamerNumber = " + mCamerNumber);
				return null;
			}

			@Override
			protected void onPostExecute(Void result) {
				Logger.v(TAG, "onPostExecute() ");
				Logger.v(TAG, "After register mVideoSharingInvitationReceiver");
				if (mCallScreenHost != null) {
					boolean isSupportVideoShare = getCapability(mNumber);
					mCallScreenHost.onCapabilityChange(mNumber, isSupportVideoShare);
				}
			}

		}.execute();

		mOrientationListener = new OrientationEventListener(context,
				SensorManager.SENSOR_DELAY_NORMAL) {
			public void onOrientationChanged(int orientation) {
				
				 final int iLookup[] = {0,   0, 0,90,90, 90,90, 90,  90, 180, 180, 180, 180, 180, 180, 270, 270, 270, 270, 270, 270, 0, 0, 0}; // 15-degree increments 
		            if (orientation != ORIENTATION_UNKNOWN)
		            {		            	
		                int iNewOrientation = iLookup[orientation / 15];
		                if (mOrient != iNewOrientation)
		                {		                	
		                	mOrient = iNewOrientation;
		                	if(mCameraPreviewRunning)
							{
		                		Logger.d(TAG, "onOrientationChanged mOrient = " + mOrient);
		                		cameraRotation();
							}									                   
					}
				}

			}
		};
		// To display if orientation detection will work and enable it
	    if (mOrientationListener.canDetectOrientation())
	    {
	    	Logger.d(TAG, "onOrientationChanged Enabled");
	    	mOrientationListener.enable();
				}
	    else
	    {
	    	Logger.d(TAG, "onOrientationChanged Cannot detect");
			}

		mSwitchCamerImageView = new ImageView(mContext);
		mSwitchCamerImageView.setImageResource(R.drawable.ic_rotate_camera_disabled_holo_dark);
		mSwitchCamerImageView.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				switchCamera();
			}
		});
		mEndSenderSessionImageView = new ImageView(mContext);
		mEndSenderSessionImageView.setImageResource(R.drawable.btn_terminate_video_share_pre_sender);
		mEndReceiverSessionImageView = new ImageView(mContext);
		mEndReceiverSessionImageView.setImageResource(R.drawable.btn_terminate_video_share_pre_receiver);
		mEndSenderSessionImageView.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {

				Logger.v(TAG, "onClick() listener Sender Button share status =" + mShareStatus);

				if (mIsVideoSharingReceiver.get()){
					Logger.v(TAG, "onClick() listener Sender Button LIVE_TWOWAY");
					finishSenderTwoWaylocal();
					mShareStatus = ShareStatus.LIVE_IN;
				}
				else
				{
					Logger.v(TAG, "onClick()listener receiver Button LIVE_OUT");
					VideoSharingPlugin.this.stop();
					mShareStatus = ShareStatus.UNKNOWN;
				}
				mIsVideoSharingSender.set(false);
				
			}
		});
		mEndReceiverSessionImageView.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Logger.v(TAG, "onClick listener() receiver Button status = " + mShareStatus );
				
				if (mIsVideoSharingSender.get()){
					Logger.v(TAG, "onClick() listener receiver Button LIVE_TWOWAY");
					finishReceiverTwoWayLocal();
					mShareStatus = ShareStatus.LIVE_OUT;
				}
				else
				{
					Logger.v(TAG, "onClick() listener receiver Button LIVE_IN");
					VideoSharingPlugin.this.stop();
					mShareStatus = ShareStatus.UNKNOWN;
				}
				mIsVideoSharingReceiver.set(false);
				
			}
		});

		mVideoSharingDialogManager = new VideoSharingDialogManager();
		mInterface = mVideoSharingCallBack;
	}

	private void initAudioButton() {
		Logger.d(TAG, "initAudioButton entry, mAudioButton: " + mAudioButton + " mCallScreenHost: "
				+ mCallScreenHost);
		if (mAudioButton == null) {
			if (mCallScreenHost != null) {
				Activity inCallScreen = mCallScreenHost.getCallScreenActivity();
				String packageName = inCallScreen.getPackageName();
				Resources resource = inCallScreen.getResources();
				mAudioButton = (ImageButton) inCallScreen.findViewById(resource.getIdentifier(
						AUDIO_BUTTON, VIEW_ID, packageName));
			} else {
				Logger.d(TAG, "initAudioButton mCallScreenHost is null");
			}
		}
		Logger.d(TAG, "initAudioButton exit: " + mAudioButton);
	}




	// Implements ICallScreenPlugIn
	@Override
	public void start(String number) {
		Logger.d(TAG, "start() entry, number: " + number + " mShareStatus: " + mShareStatus);
		super.start(number);
		// Image share is ongoing.
		if (Utils.isInImageSharing()) {
			alreadyOnGoing();
			return;
		}
		// Video share is of full status.
		if ( mShareStatus == ShareStatus.LIVE_OUT || mShareStatus == ShareStatus.LIVE_TWOWAY) {
			alreadyOnGoing();
			return;
		}
		if (!isVideoShareSupported(number)) {
			videoShareNotSupported();
			return;
		}
		startVideoShare();

	}

	@Override
	public void stop() {
		Logger.v(TAG, "stop button is clicked");
		destroy();
	}

	@Override
	public int getState() {
		Logger.v(TAG, "getState(), getState() = " + mVideoSharingState.get());
		return mVideoSharingState.get();
	}


	/**
	 * Start a live video sharing based on current status.
	 * Start sharing may not execute.
	 * @param isLive True if start a live sharing
	 */
	private void startVideoShare() {
		Logger.v(TAG, "startVideoShare(), number = " + mNumber
				+ ", then request a view group");
		if (mNumber == null) {
			Logger.e(TAG, "startVideoShare() number is null");
			return;
		}
		if (allowOutgoingLiveSharing()) {
			int networkType = Utils.getNetworkType(mContext);
			startByNetwork(networkType);
		} else {
			Logger.w(TAG, "startVideoShare() Start has been clicked,"
					+ " please wait until the session failed");
			alreadyOnGoing();
		}
		
	}

	/**
	 * Start a video sharing( according to current network
	 * type. First set sharing status and then check network type, if can not
	 * start a video sharing, then restore the sharing status to the previous.
	 * 
	 * @param networkType network type
	 * @param isLive True if start a live sharing
	 *            sharing.
	 */
	private void startByNetwork(int networkType) {

		Logger.d(TAG, "start a live sharing, modify status");
		if (mShareStatus == ShareStatus.LIVE_IN) {
			mShareStatus = ShareStatus.LIVE_TWOWAY;
		} else {
			mShareStatus = ShareStatus.LIVE_OUT;
		}
		// 2G & 2.5
		if (networkType == Utils.NETWORK_TYPE_GSM
				|| networkType == Utils.NETWORK_TYPE_GPRS
				|| networkType == Utils.NETWORK_TYPE_EDGE) {
			Logger.v(TAG,
					"startVideoShare()-2G or 2.5G mobile network, fibbiden video share");
			Resources resources = mContext.getResources();
			String message = resources
					.getString(R.string.now_allowed_video_share_by_network);
			showToast(message);
			setSharingStatusToPreviousStatus();
			return;
		} else if (networkType == Utils.NETWORK_TYPE_UMTS
				|| networkType == Utils.NETWORK_TYPE_HSUPA
				|| networkType == Utils.NETWORK_TYPE_HSDPA
				|| networkType == Utils.NETWORK_TYPE_1XRTT
				|| networkType == Utils.NETWORK_TYPE_EHRPD) { // 2.75G
			// or
			// 3G
			Logger.v(TAG, "startVideoShare()-2.75G or 3G mobile network, "
					+ "allow single line video share");
			if (mIncomingVideoSharingSession != null) {
				try {
					if (mIncomingVideoSharingSession.getSessionState() == SessionState.ESTABLISHED
							|| mIncomingVideoSharingSession.getSessionState() == SessionState.PENDING) {
						Resources resources = mContext.getResources();
						String message = resources
								.getString(R.string.now_allowed_video_share_by_network);
						showToast(message);
						setSharingStatusToPreviousStatus();
						return;
					}
				} catch (RemoteException e) {
					Logger.e(TAG, e.toString());
				}
			} else {
				Logger.d(TAG,
						"startByNetwork(), mIncomingVideoSharingSession is null");
			}
		} else if (networkType == Utils.NETWORK_TYPE_HSPA
				|| networkType == Utils.NETWORK_TYPE_LTE
				|| networkType == Utils.NETWORK_TYPE_UMB) { // 4G
			Logger.v(TAG,
					"startVideoShare()-4G mobile network, allow two-way video share");
		} else if (networkType == Utils.NETWORK_TYPE_WIFI) { // WI-FI
			Logger.v(TAG,
					"startVideoShare()-WI-FI network, allow two-way video share");
		} else { // Unknown
			Logger.v(TAG,
					"startVideoShare()-Unknown network, default to allow two-way video share");
		}
		if (!mWakeLock.isHeld()) {
			mWakeLock.acquire();
			Logger.v(TAG, "startVideoShare() when start, acquire a wake lock");
		} else {
			Logger.v(TAG,
					"startVideoShare() when start, the wake lock has been acquired,"
							+ " so do not acquire");
		}
		mNumbersToBeListened.add(mNumber);

		if (mShareStatus == ShareStatus.LIVE_OUT
				|| mShareStatus == ShareStatus.LIVE_TWOWAY) {
			// Create the live video player
			Logger.v(TAG,
					"startVideoShare() start createLiveVideoPlayer mOutgoingVideoFormat is: "
							+ mOutgoingVideoFormat);

			mOutgoingVideoPlayer = new LiveVideoPlayer();

		} 
		// Tell that it is the vs sender if start vs before receive vs
		// invitation
		
		mIsVideoSharingSender.set(true);
		
		Logger.w(TAG, "startVideoShare() mCallScreenHost = " + mCallScreenHost);
		if (mCallScreenHost != null) {
			mOutgoingDisplayArea = (RelativeLayout) mCallScreenHost
					.requestAreaForDisplay();
		}
		showLocalView();
	}

	/**
	 * Set Sharing status to previous status when the client failed to start a
	 * video sharing.
	 */
	private void setSharingStatusToPreviousStatus() {
		Logger.d(TAG, "setSharingStatusToPreviousStatus");

		if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
			mShareStatus = ShareStatus.LIVE_IN;
		} else {
			mShareStatus = ShareStatus.UNKNOWN;
		}
	}

	/**
	 * Check whether allow a outgoing live sharing
	 * @return True if allow a outgoing live sharing
	 */
	private boolean allowOutgoingLiveSharing(){
		boolean allow = false;
		if ( mShareStatus == ShareStatus.LIVE_OUT
				|| mShareStatus == ShareStatus.LIVE_TWOWAY) {
			allow = false;
		} else {
			allow = true;
		}
		Logger.d(TAG, "allowOutgoingLiveSharing(): return " + allow);
		return allow;
	}



	private void showLocalView() {
		Logger.d(TAG, "showLocalView(), mShareStatus: " + mShareStatus + " mIsVideoSharingSender: "
				+ mIsVideoSharingSender);
		if (mShareStatus != ShareStatus.UNKNOWN) {
		if (mIsVideoSharingReceiver.get()) {
				Logger.v(TAG,
						"startVideoShare() After recevie vs invitation, then send vs invitation");
				showReceiverLocalView();
				showWaitRemoteAcceptMessage();
			}
		else if (mIsVideoSharingSender.get()) {
				Logger.v(TAG, "startVideoShare() First send vs invitation");
				showSenderLocalView();
				showWaitRemoteAcceptMessage();
			} 
		} 
		mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
		Utils.setInVideoSharing(true);
		if (mRichCallStatus == RichCallStatus.CONNECTED) {
			Logger.v(TAG, "startVideoShare(), then call startOutgoingVideoShareSession()");
			startOutgoingVideoShareSession();
		} else {
			Logger.v(
					TAG,
					"startVideoShare(), call startOutgoingVideoShareSession() when richcall api connected.");
			mStartOutgoingSessionWhenApiConnected.set(true);
			resetVideoShaingState();
		}
	}

	private void alreadyOnGoing() {
		Logger.v(TAG, "alreadyOnGoing entry");
		if (Utils.isInImageSharing() && RCSeInCallUIExtension.getInstance().getmShareFilePlugIn().getCurrentState() == Constants.SHARE_FILE_OUTGOING) {
			Logger.v(TAG, "alreadyOnGoing isInImageSharing outgoing file true");
			mVideoSharingDialogManager.alreadyOnGoingImageShare();
		}else if (Utils.isInImageSharing() && RCSeInCallUIExtension.getInstance().getmShareFilePlugIn().getCurrentState() == Constants.SHARE_FILE_INCOMING) {
			Logger.v(TAG, "alreadyOnGoing isInImageSharing Incmoing file true");
	        startVideoShare();
		} 
		else {
			mMainHandler.post(new Runnable() {
				@Override
				public void run() {
					Logger.v(TAG, "alreadyOnGoing isInImageSharing false");
					String message = mContext.getResources().getString(
							R.string.video_sharing_is_on_going);

					showToast(message);
				}
			});

		}
		Logger.v(TAG, "alreadyOnGoing exit");
	}

	public void saveAlertDialogs()
	{
		Logger.v(TAG, "saveAlertDialogs() Video entry");
		mCallScreenSavedDialogSet.clear();
		mCallScreenSavedDialogSet.addAll(mCallScreenDialogSet);
		Logger.v(TAG, "saveAlertDialogs() Video exit");
	}

	public void showAlertDialogs()
	{
		Logger.v(TAG, "showAlertDialogs() Video entry");
		mMainHandler.post(new Runnable() {
			@Override
			public void run() {
				if(mCallScreenSavedDialogSet.size() > 1){
					mCallScreenDialogSet.add(mCallScreenSavedDialogSet.get(mCallScreenSavedDialogSet.size() - 1));
					mCallScreenSavedDialogSet.get(mCallScreenSavedDialogSet.size() - 1).show();
				}
				else if(mCallScreenSavedDialogSet.size() == 1){
					mCallScreenDialogSet.add(mCallScreenSavedDialogSet.get(0));
					mCallScreenSavedDialogSet.get(0).show();
				}
				mCallScreenSavedDialogSet.clear();
			}
		});
		Logger.v(TAG, "showAlertDialogs() Video exit");
	}

	public void clearSavedDialogs(){
		Logger.v(TAG, "clearSavedDialogs() entry mCallScreenSavedDialogSet size is "+mCallScreenSavedDialogSet.size());
		mCallScreenSavedDialogSet.clear();
		Logger.v(TAG, "clearSavedDialogs() exit mCallScreenSavedDialogSet size is "+mCallScreenSavedDialogSet.size());
	}

	public boolean dismissDialog() {
		Logger.v(TAG, "dismissDialog()");
		dismissAllDialogs();
		return false;
	}

	private void dismissAllDialogs() {
		Logger.v(TAG, "dismissAllDialog()");
		for (CallScreenDialog dialog : mCallScreenDialogSet) {
			dialog.dismissDialog();
			mCallScreenDialogSet.remove(dialog);
			Logger.v(TAG, "have dismissed a dialog: " + dialog);
		}
		mVideoSharingDialogManager.dismissWaitingInitializeConextProgressDialog();
	}

	/**
	 * Start the outgoing session
	 */
	private void startOutgoingVideoShareSession() {
		Logger.v(TAG, "startOutgoingVideoShareSession");
		AsyncTask.execute(new Runnable() {
			@Override
			public void run() {
				try {
					// Initiate sharing
					Logger.d(TAG, "startOutgoingVideoShareSession(), mShareStatus: " + mShareStatus);
					if (mShareStatus == ShareStatus.LIVE_OUT
							|| mShareStatus == ShareStatus.LIVE_TWOWAY) {
						String vodafoneAccount = getVodafoneAccount(mNumber);
						if (vodafoneAccount != null) {
							mOutgoingVideoSharingSession =
									mRichCallApi.initiateLiveVideoSharing(TEL_URI_SCHEMA
											+ getVodafoneAccount(mNumber), mOutgoingVideoPlayer);
						} else {
							Logger.w(TAG, "vodafoneAccount is null");
						}
					} 
					if (mOutgoingVideoSharingSession != null) {
						mOutgoingVideoSharingSession
						.addSessionListener(mOutgoingSessionEventListener);
					} else {
						Logger.w(TAG, "mOutgoingVideoSharingSession is null.");
						handleStartVideoSharingFailed();
					}
				} catch (ClientApiException e) {
					e.printStackTrace();
					handleStartVideoSharingFailed();
				} catch (RemoteException e) {
					e.printStackTrace();
					handleStartVideoSharingFailed();
				}
			}
		});
	}

	private void handleStartVideoSharingFailed() {
		Logger.d(TAG, "handleStartVideoSharingFailed entry");
		if (mIsVideoSharingSender.get()) {
			Logger.v(TAG, "handleStartVideoSharingFailed sender fail");
			destroy();
		} else {
			Logger.v(TAG, "handleStartVideoSharingFailed receiver fail");
			if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
				mShareStatus = ShareStatus.LIVE_IN;
			} else {
				mShareStatus = ShareStatus.UNKNOWN;
			}
			destroyOutgoingViewOnly();
		}
		Logger.d(TAG, "handleStartVideoSharingFailed exit");
	}

	/**
	 * Outgoing video sharing session event listener
	 */
	private IVideoSharingEventListener mOutgoingSessionEventListener = new IVideoSharingEventListener.Stub() {
		private static final String TAG = "OutgoingSessionEventListener";

		// Session is started
		public void handleSessionStarted() {
			Logger.v(TAG, "handleSessionStarted(), mShareStatus: " + mShareStatus
					+ " mHeadsetConnected: " + mHeadsetConnected);
			try {
				if (mOutgoingVideoSharingSession != null) {
					if (is3GMobileNetwork()) {
						if (mIncomingVideoSharingSession != null) {
							Logger.d(TAG,"Reject the incoming session because the device is upder " +
									"3G network and the outgoing video share session is established");
							mIncomingVideoSharingSession.cancelSession();
						}else {
							Logger.v(TAG, "handleSessionStarted(),Entered in else of mIncomingVideoSharingSession");
							// Tell the host to update
							mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
							mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
							showRemoteAcceptMessage();
						}
					} else {
						Logger.v(TAG, "handleSessionStarted(),Entered in else of 3g");
						// Tell the host to update
						mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
						mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
						showRemoteAcceptMessage();
					}
				} else {
					Logger.w(TAG, "mOutgoingVideoSharingSession is null");
				}
			} catch (RemoteException e) {
				e.printStackTrace();
			}
			mMainHandler.post(new Runnable() {
				@Override
				public void run() {}
			});
		}
		// Video stream has been resized    
		public void handleVideoResized( int width,  int height)
		{
		}

		// Session has been aborted
		public void handleSessionAborted(int reason) {
			Logger.v(TAG, "handleSessionAborted()");
			mIsStarted.set(false);
			if (mIsVideoSharingReceiver.get()) {
				// At this time we are receiving vs from remote, so
				// should only remove outgoing view
				mMainHandler.post(new Runnable() {
					@Override
					public void run() {
						removeOutgoingVideoSharingVews();						
					}
				});
			} else {
				RCSeInCallUIExtension.getInstance().resetDisplayArea();
				mVideoSharingDialogManager.showTerminatedByNetworkDialog();
			}
		}

		// Session has been terminated by remote
		public void handleSessionTerminatedByRemote() {
			Logger.v(TAG, "handleSessionTerminatedByRemote()");			
				// At this time we are receiving vs from remote, so
				// should only remove outgoing view
				mMainHandler.post(new Runnable() {
					@Override
					public void run() {
						if (mIsVideoSharingReceiver.get())
						{
                    	removeVeiwsAtSenderTerminatedByRemote();
					}
						else
						{
							RCSeInCallUIExtension.getInstance().resetDisplayArea();
				mVideoSharingDialogManager.showTerminatedByRemoteDialog();
			}
							
					}
				});	
										
		}

		// Content sharing error
		public void handleSharingError(final int error) {
			Logger.v(TAG, "handleSharingError(), error = " + error);
			switch (error) {
			case ImsServiceError.SESSION_INITIATION_CANCELLED:
				Logger.v(TAG, "SESSION_INITIATION_CANCELLED");
				break;
			case ImsServiceError.SESSION_INITIATION_DECLINED:
				Logger.v(TAG, "SESSION_INITIATION_DECLINED");
				mVideoSharingDialogManager.showRejectedByRemoteDialog();
				return;
			case ImsServiceError.SESSION_INITIATION_FAILED:
				Logger.v(TAG,
						"SESSION_INITIATION_FAILED, at most case it is a 408 error(time out)");
				if (mIsVideoSharingReceiver.get()) {
					// At this time we are receiving vs from remote,
					// so should only remove outgoing view
					mMainHandler.post(new Runnable() {
						@Override
						public void run() {
							removeOutgoingVideoSharingVews();
							mVideoSharingDialogManager.showTerminatedByNetworkDialog();
						}
					});
				} else {
					RCSeInCallUIExtension.getInstance().resetDisplayArea();
					mVideoSharingDialogManager.showTerminatedByNetworkDialog();
				}
				return;
			case ContentSharingError.SESSION_INITIATION_TIMEOUT:
				Logger.v(TAG, "SESSION_INITIATION_TIMEOUT)");
				// status can be LIVE_OUT, LIVE_TWOWAY
				if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
					mShareStatus = ShareStatus.LIVE_IN;
				} else {
					mShareStatus = ShareStatus.UNKNOWN;
				}
				if (mIsVideoSharingReceiver.get()) {
					// At this time we are receiving vs from remote,
					// so should only remove outgoing view
					mMainHandler.post(new Runnable() {
						@Override
						public void run() {
							removeOutgoingVideoSharingVews();
							mVideoSharingDialogManager.showTimeOutDialog();
						}
					});
				} else {
					mVideoSharingDialogManager.showTimeOutDialog();
				}
				return;
			default:
				break;
			}
			if (mIsVideoSharingReceiver.get()) {
				// At this time we are receiving vs from remote, so
				// should only remove outgoing view
				mMainHandler.post(new Runnable() {
					@Override
					public void run() {
						removeOutgoingVideoSharingVews();
					}
				});
			} else {
				mVideoSharingDialogManager.showTerminatedByRemoteDialog();
			}
		}
	};

    private void removeVeiwsAtReceiverTerminatedByRemote() {
        Logger.v(TAG, "removeVeiwsAtReceiverTerminatedByRemote");
    	 mMainHandler.post(new Runnable() {
 			@Override
 			public void run() {
        if (mIncomingDisplayArea != null) {
    		Logger.v(TAG, "removeVeiwsAtReceiverTerminatedByRemote() mIncomingDisplayArea Entry");
            mIncomingDisplayArea.removeView(mIncomingLocalVideoSurfaceView);
            mIncomingDisplayArea.removeView(mIncomingRemoteVideoSurfaceView);
            mIncomingDisplayArea.removeView(mSwitchCamerImageView);
            mIncomingDisplayArea.removeView(mEndReceiverSessionImageView);
            mIncomingDisplayArea.removeView(mEndSenderSessionImageView);                           
            
        } else {
            Logger.w(TAG, " removeVeiwsAtReceiverTerminatedByRemote mIncomingDisplayArea is null");
        }   
    	
    	 if (mOutgoingDisplayArea != null) {
             mOutgoingDisplayArea.removeView(mOutgoingLocalVideoSurfaceView);
             mOutgoingDisplayArea.removeView(mOutgoingRemoteVideoSurfaceView);
             mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
             mOutgoingDisplayArea.removeView(mEndSenderSessionImageView);
             mOutgoingDisplayArea.removeView(mEndReceiverSessionImageView);

         } else {
             Logger.w(TAG, " removeVeiwsAtReceiverTerminatedByRemote mOutgoingDisplayArea is null");
         }
    	 mIsVideoSharingReceiver.set(false); 
    	 if (mIsVideoSharingSender.get())
    	 {
    		 Logger.w(TAG, " removeVeiwsAtReceiverTerminatedByRemote sender not null");
    		 showSenderLocalView();
    		 mShareStatus = ShareStatus.LIVE_OUT;
    	 }
    	 else
    	 {
    		 Logger.w(TAG, " removeVeiwsAtReceiverTerminatedByRemote sender is null");
    		 RCSeInCallUIExtension.getInstance().resetDisplayArea();
    		 resetVideoShaingState();
    }
    }
 		});
    	 
    }

	/**
	 * Incoming video sharing session event listener
	 */
	private IVideoSharingEventListener mIncomingSessionEventListener = new IVideoSharingEventListener.Stub() {
		private static final String TAG = "IncomingSessionEventListener";

		// Session is started
		public void handleSessionStarted() {
			Logger.v(TAG, "handleSessionStarted(), mShareStatus: " + mShareStatus
					+ " mHeadsetConnected: " + mHeadsetConnected);
			mMainHandler.post(new Runnable() {
				@Override
				public void run() {
					
							Logger.d(TAG, "onFirstTimeRecevie(), mShareStatus: " + mShareStatus);
							mVideoSharingDialogManager
							.dismissWaitingInitializeConextProgressDialog();

							Logger.d(TAG, "onFirstTimeRecevie(), is incoming live video share!");

				}
			});
		}

		// Video stream has been resized    
		public void handleVideoResized( int width,  int height)
		{
		}

		// Session has been aborted
		public void handleSessionAborted(int reason) {
			Logger.v(TAG, "handleSessionAborted()");
			// status can be LIVE_IN, LIVE_TWOWAY
			if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
				mShareStatus = ShareStatus.LIVE_OUT;
			} else {
				RCSeInCallUIExtension.getInstance().resetDisplayArea();
				mShareStatus = ShareStatus.UNKNOWN;
			}
			mVideoSharingDialogManager.showTerminatedByRemoteDialog();
		}

		// Session has been terminated by remote
		public void handleSessionTerminatedByRemote() {
			Logger.v(TAG, "handleSessionTerminatedByRemote()");
            removeVeiwsAtReceiverTerminatedByRemote();
			mVideoSharingDialogManager.showTerminatedByRemoteDialog();
		}

		// Sharing error
		public void handleSharingError(final int error) {
			Logger.v(TAG, "handleSharingError(), error = " + error);
			switch (error) {
			case ImsServiceError.SESSION_INITIATION_CANCELLED:
				Logger.v(TAG, "SESSION_INITIATION_CANCELLED");
				break;
			case ImsServiceError.SESSION_INITIATION_DECLINED:
				Logger.v(TAG, "SESSION_INITIATION_DECLINED");
				break;
			case ImsServiceError.SESSION_INITIATION_FAILED:
				Logger.v(TAG, "SESSION_INITIATION_FAILED");
				break;
			default:
				break;
			}
			mVideoSharingDialogManager.showTerminatedByRemoteDialog();
		}
	};

	// Implements SurfaceHolder.Callback
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		synchronized (mLock) {
			boolean isLive = (mShareStatus == ShareStatus.LIVE_IN
					|| mShareStatus == ShareStatus.LIVE_OUT || mShareStatus == ShareStatus.LIVE_TWOWAY);
			Logger.d(TAG, "surfaceCreated(), mCamerNumber: " + mCamerNumber + ", mCamera: "
					+ mCamera + ", mShareStatus: " + mShareStatus);
			if (mCamera == null && isLive) {
				// Start camera preview
				if (mCamerNumber > SINGLE_CAMERA) {
					// Try to open the front camera
					Logger.v(TAG, "surfaceCreated(), try to open the front camera");
					mCamera = Camera.open(CameraInfo.CAMERA_FACING_FRONT);
					mOpenedCameraId = CameraInfo.CAMERA_FACING_FRONT;
				} else {
					Logger.v(TAG, "surfaceCreated(), try to open the front camera");
					openDefaultCamera();
				}
				addPreviewCallback();
			}
		}
	}

	private void addPreviewCallback() {
		Logger.v(TAG, "addPreviewCallback entry");
		if (mCamera == null) {
			Logger.v(TAG, "addPreviewCallback mCamera is null");
			return;
		}
		mCamera.setPreviewCallback(new Camera.PreviewCallback() {

			@Override
			public void onPreviewFrame(byte[] data, Camera camera) {
				onCameraPreviewFrame(data, camera);
			}
		});
		Logger.v(TAG, "addPreviewCallback exit");
	}

	private void onCameraPreviewFrame(byte[] data, Camera camera) {

		if (mOutgoingVideoPlayer != null) {
			mOutgoingVideoPlayer.onPreviewFrame(data,camera);
		} else {
			Logger.d(TAG, "onPreviewFrame(), addPreviewCallback mOutgoingLiveVideoPlayer is null");
		}


	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		Logger.v(TAG, "surfaceChanged(), mShareStatus: " + mShareStatus);
		mPreviewSurfaceHolder = holder;
		synchronized (mLock) {
			if (mCamera != null) {
				if (mCameraPreviewRunning) {
					mCameraPreviewRunning = false;
					mCamera.stopPreview();
				}
			} else {
				Logger.w(TAG, "mCamera is null");
			}
		}
		boolean isLive = (mShareStatus == ShareStatus.LIVE_IN
				|| mShareStatus == ShareStatus.LIVE_OUT || mShareStatus == ShareStatus.LIVE_TWOWAY);
		if (isLive) {
			startCameraPreview();
		}
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Logger.v(TAG, "surfaceDestroyed(), holder = " + holder + ",set holder to null");
	}

	/**
	 * Start the camera preview
	 */
	private void startCameraPreview() {
		Logger.v(TAG, "startCameraPreview()");
		synchronized (mLock) {
			if (mCamera != null) {
				Camera.Parameters p = mCamera.getParameters();
				// Init Camera
				p.setPreviewSize(mVideoWidth, mVideoHeight);
				p.setPreviewFormat(PixelFormat.YCbCr_420_SP);
				// Try to set front camera if back camera doesn't support size
				List<Camera.Size> sizes = p.getSupportedPreviewSizes();
				if (sizes != null && !sizes.contains(mCamera.new Size(mVideoWidth, mVideoHeight))) {
					Logger.v(TAG, "Does not contain");
					String camId = p.get(CAMERA_ID);
					if (camId != null) {
						p.set(CAMERA_ID, 2);
						p.set(CMA_MODE, ROTATION_BY_HW);
					} else {
						Logger.v(TAG, "cam_id is null");
					}
				} else {
					Logger.v(TAG, "startCameraPreview(), sizes object = " + sizes + ". contains the size object. "
							+ "mOpenedCameraId = " + mOpenedCameraId);
					p.set(CMA_MODE, ROTATION_BY_HW);
				}
		
				android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();

				android.hardware.Camera.getCameraInfo(mOpenedCameraId, info);

				if (info != null) {
					switch (mOrient) {
					case ROTATION_0:
						Logger.e(TAG, "ROTATION_0");					
						if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
							mOutgoingVideoPlayer
									.setOrientation(Orientation.ROTATE_90_CCW);
						} else {
							mOutgoingVideoPlayer
									.setOrientation(Orientation.ROTATE_90_CW);
						}						
						mCamera.setDisplayOrientation(90);
						mSwitchCamerImageView.setRotation(0);

						break;
					case ROTATION_90:
						Logger.e(TAG, "ROTATION_90");						
						mCamera.setDisplayOrientation(90);
						mOutgoingVideoPlayer.setOrientation(Orientation.NONE);
						mSwitchCamerImageView.setRotation(270);
						break;

					case ROTATION_180:
						Logger.e(TAG, "ROTATION_180");						
						if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
							mOutgoingVideoPlayer
									.setOrientation(Orientation.ROTATE_90_CW);
						} else {
							mOutgoingVideoPlayer
									.setOrientation(Orientation.ROTATE_90_CCW);
						}
						mCamera.setDisplayOrientation(90);
						mSwitchCamerImageView.setRotation(180);

						break;
					case ROTATION_270:
						Logger.e(TAG, "ROTATION_270");
						mCamera.setDisplayOrientation(90);						
						if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
							mOutgoingVideoPlayer
									.setOrientation(Orientation.ROTATE_180);
						} else {
							mOutgoingVideoPlayer
									.setOrientation(Orientation.ROTATE_180);
						}
						mSwitchCamerImageView.setRotation(90);
						break;
					}
				}				
				mCamera.setParameters(p);
			//	mOutgoingVideoPlayer.setCameraId(mOpenedCameraId);
				try {
					mCamera.setPreviewDisplay(mPreviewSurfaceHolder);
					mCamera.startPreview();
					mCameraPreviewRunning = true;
				} catch (IOException e) {
					e.printStackTrace();
					mCamera = null;
				}
			} else {
				Logger.e(TAG, "mCamera is null");
			}
		}
	}
	
	private void cameraRotation()
	{
		Logger.e(TAG, "cameraRotation entry");
		android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
		android.hardware.Camera.getCameraInfo(mOpenedCameraId, info);		
		if (info != null) {
			switch (mOrient) {
			case ROTATION_0:
				Logger.e(TAG, "ROTATION_0");					
				if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
					mOutgoingVideoPlayer
							.setOrientation(Orientation.ROTATE_90_CCW);
				} else {
					mOutgoingVideoPlayer
							.setOrientation(Orientation.ROTATE_90_CW);
				}										
				mSwitchCamerImageView.setRotation(0);

				break;
			case ROTATION_90:
				Logger.e(TAG, "ROTATION_90");						
				
				if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
					mOutgoingVideoPlayer
							.setOrientation(Orientation.ROTATE_90_CCW);
				} else {
					mOutgoingVideoPlayer
							.setOrientation(Orientation.ROTATE_90_CW);
				}
				mSwitchCamerImageView.setRotation(270);
				break;

			case ROTATION_180:
				Logger.e(TAG, "ROTATION_180");						
				if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
					mOutgoingVideoPlayer
							.setOrientation(Orientation.ROTATE_90_CCW);
				} else {
					mOutgoingVideoPlayer
							.setOrientation(Orientation.ROTATE_90_CW);
				}
				
				mSwitchCamerImageView.setRotation(180);

				break;
			case ROTATION_270:
				Logger.e(TAG, "ROTATION_270");								
				if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
					mOutgoingVideoPlayer
							.setOrientation(Orientation.ROTATE_90_CCW);
				} else {
					mOutgoingVideoPlayer
							.setOrientation(Orientation.ROTATE_90_CW);
				}
				mSwitchCamerImageView.setRotation(90);
				break;
			}
		}			
	}
	

	private void stopCameraPreview() {
		Logger.v(TAG, "stopCameraPreview()");
		// Release the camera
		synchronized (mLock) {
			if (mCamera != null) {
				mCamera.setPreviewCallback(null);
				mCamera.stopPreview();
				mCamera.release();
				mCamera = null;
				mCameraPreviewRunning = false;
			} else {
				Logger.v(TAG, "mCamera is null, so do not release");
			}
		}
	}

	private void removeIncomingVideoSharingVews() {
		Logger.v(TAG, "removeIncomingVideoSharingVews");
		if (mIncomingDisplayArea != null) {
			mIncomingDisplayArea.removeView(mIncomingRemoteVideoSurfaceView);
			mIncomingDisplayArea.removeView(mIncomingLocalVideoSurfaceView);
			mIncomingDisplayArea.removeView(mSwitchCamerImageView);

		} else {
			Logger.w(TAG, "mIncomingDisplayArea is null");
		}
	}

	private void removeIncomingVideoSharingRemoteVews() {
		Logger.v(TAG, "removeIncomingVideoSharingRemoteVews");
		if (mIncomingDisplayArea != null) {
			mIncomingDisplayArea.removeView(mIncomingRemoteVideoSurfaceView);
			mIncomingDisplayArea.removeView(mEndReceiverSessionImageView);
		} else {
			Logger.w(TAG, "mIncomingDisplayArea is null");
		}
	}

	private void removeIncomingVideoSharingLocalVews() {
		Logger.v(TAG, "removeIncomingVideoSharingLocalVews");
		if (mIncomingDisplayArea != null) {
			mIncomingDisplayArea.removeView(mIncomingLocalVideoSurfaceView);
			mIncomingDisplayArea.removeView(mSwitchCamerImageView);
			mIncomingDisplayArea.removeView(mEndReceiverSessionImageView);

		} else {
			Logger.w(TAG, "mIncomingDisplayArea is null");
		}
	}

    private void removeVeiwsAtSenderTerminatedByRemote() {
        Logger.v(TAG, "removeVeiwsAtSenderTerminatedByRemote");
        
        mMainHandler.post(new Runnable() {
			@Override
			public void run() {
        if (mOutgoingDisplayArea != null) {
        	Logger.v(TAG, "removeVeiwsAtSenderTerminatedByRemote, outgoing is not null");
            mOutgoingDisplayArea.removeView(mOutgoingLocalVideoSurfaceView);          
            mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
            mOutgoingDisplayArea.removeView(mEndSenderSessionImageView);            
        } else {
            Logger.w(TAG, "removeVeiwsAtSenderTerminatedByRemote, mOutgoingDisplayArea is null");
        }
        if (mIncomingDisplayArea != null) {            
        	Logger.v(TAG, "removeVeiwsAtSenderTerminatedByRemote, mincomingarea not null");
            mIncomingDisplayArea.removeView(mIncomingLocalVideoSurfaceView);
            mIncomingDisplayArea.removeView(mSwitchCamerImageView);
            mIncomingDisplayArea.removeView(mEndSenderSessionImageView);
            
        } else {
            Logger.w(TAG, "removeVeiwsAtSenderTerminatedByRemote, mIncomingDisplayArea is null");
        }
        
        mIsVideoSharingSender.set(false);
		        
		        	mShareStatus = ShareStatus.LIVE_IN;
		        	mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
        mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
    		        
			}
		});
        
        
    }

	private void removeOutgoingVideoSharingVews() {
		Logger.v(TAG, "removeOutgoingVideoSharingVews");
		if (mOutgoingDisplayArea != null) {
			mOutgoingDisplayArea.removeView(mOutgoingLocalVideoSurfaceView);
			mOutgoingDisplayArea.removeView(mOutgoingRemoteVideoSurfaceView);
			mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
			mOutgoingDisplayArea.removeView(mEndSenderSessionImageView);
						

		} else {
			Logger.w(TAG, "mOutgoingDisplayArea is null");
		}
	}

	// This is only called by the first sender.
	private void showSenderLocalView() {
		Logger.v(TAG, "showSenderLocalView() entry, mOutgoingVideoFormat is: "
				+ mOutgoingVideoFormat);
		removeOutgoingVideoSharingVews();
		mOutgoingLocalVideoSurfaceView = new VideoSurfaceView(mContext);
		int width = mOutgoingDisplayArea.getWidth();
		int height = mOutgoingDisplayArea.getHeight();
		Logger.v(TAG, "showSenderLocalView(), width = " + width + ",height = " + height
				+ "mVideoWidth = " + mVideoWidth + ",mVideoHeight = " + mVideoHeight);
		LayoutParams params =
				new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
						ViewGroup.LayoutParams.MATCH_PARENT);
		mOutgoingLocalVideoSurfaceView.setLayoutParams(params);
		mOutgoingLocalVideoSurfaceView.setAspectRatio(mVideoHeight, mVideoWidth);

		mSurfaceHolder = mOutgoingLocalVideoSurfaceView.getHolder();
		mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		mSurfaceHolder.addCallback(VideoSharingPlugin.this);
		Logger.w(TAG, "showSenderLocalView(), mOutgoingVideoFormat "+ mOutgoingVideoFormat);

		

		RelativeLayout.LayoutParams layoutParams =
				new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
						ViewGroup.LayoutParams.MATCH_PARENT);
		mOutgoingDisplayArea.addView(mOutgoingLocalVideoSurfaceView, layoutParams);

		layoutParams =
				new RelativeLayout.LayoutParams(Utils.dip2px(mContext, SWITCH_BUTTON_WIDTH), Utils
						.dip2px(mContext, SWITCH_BUTTON_HEIGHT));
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
		layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
				SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
				SWITCH_BUTTON_MARGIN_BOTTOM);
		mOutgoingDisplayArea.addView(mSwitchCamerImageView, layoutParams);

		//Adding terminate first sender video button
		layoutParams =
				new RelativeLayout.LayoutParams(Utils.dip2px(mContext, TERMINATE_BUTTON_WIDTH), Utils
						.dip2px(mContext, TERMINATE_BUTTON_HEIGHT));
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
		layoutParams.setMargins(TERMINATE_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
				SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_LEFT),
				SWITCH_BUTTON_MARGIN_BOTTOM);
		mOutgoingDisplayArea.addView(mEndSenderSessionImageView, layoutParams);

		mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
	}


	// When first sender receive invitation
	private void updateFirstSenderView() {
		Logger.v(TAG, "updateFirstSenderView() entry, mOutgoingVideoFormat is: "
				+ mOutgoingVideoFormat);
		removeOutgoingVideoSharingVews();
		mOutgoingRemoteVideoSurfaceView = new VideoSurfaceView(mContext);
		LayoutParams params =
				new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
						ViewGroup.LayoutParams.MATCH_PARENT);
		mOutgoingRemoteVideoSurfaceView.setLayoutParams(params);
		Logger.v(TAG, "FirstSenderView ,mOutgoingVideoFormat" +mOutgoingVideoFormat);

		mOutgoingRemoteVideoRenderer = new VideoRenderer();
        mOutgoingRemoteVideoRenderer.setVideoSurface(mOutgoingRemoteVideoSurfaceView);


		// Remote video view
		RelativeLayout.LayoutParams layoutParams =
				new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
						ViewGroup.LayoutParams.MATCH_PARENT);
		mOutgoingDisplayArea.addView(mOutgoingRemoteVideoSurfaceView, layoutParams);
		Logger.v(TAG, "mOutgoingRemoteVideoSurfaceView = " + mOutgoingRemoteVideoSurfaceView
				+ ", mOutgoingRemoteVideoSurfaceView.height = "
				+ mOutgoingRemoteVideoSurfaceView.getHeight()
				+ ", mOutgoingRemoteVideoSurfaceView.width = "
				+ mOutgoingRemoteVideoSurfaceView.getWidth());
		// Local video view
		 mOutgoingLocalVideoSurfaceView.setAspectRatio(mVideoWidth,
		 mVideoHeight);
		mSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
		mOutgoingLocalVideoSurfaceView.setZOrderMediaOverlay(true);
		layoutParams =
				new RelativeLayout.LayoutParams(Utils.dip2px(mContext, MIN_WINDOW_WIDTH), Utils
						.dip2px(mContext, MIN_WINDOW_HEIGHT));
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		layoutParams.setMargins(LOCAL_CAMERA_MARGIN_LEFT, Utils.dip2px(mContext,
				LOCAL_CAMERA_MARGIN_TOP), Utils.dip2px(mContext, LOCAL_CAMERA_MARGIN_RIGHT),
				LOCAL_CAMERA_MARGIN_BOTTOM);
		mOutgoingDisplayArea.addView(mOutgoingLocalVideoSurfaceView, layoutParams);

		//Adding terminate sender video button
		layoutParams =
				new RelativeLayout.LayoutParams(Utils.dip2px(mContext, TERMINATE_BUTTON_WIDTH), Utils
						.dip2px(mContext, TERMINATE_BUTTON_HEIGHT));
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
				SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
				SWITCH_BUTTON_MARGIN_BOTTOM);
		mOutgoingDisplayArea.addView(mEndSenderSessionImageView, layoutParams);

		//Adding terminate receiver video button
		layoutParams =
				new RelativeLayout.LayoutParams(Utils.dip2px(mContext, TERMINATE_BUTTON_WIDTH), Utils
						.dip2px(mContext, TERMINATE_BUTTON_HEIGHT));
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
		layoutParams.setMargins(TERMINATE_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
				SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_LEFT),
				SWITCH_BUTTON_MARGIN_BOTTOM);
		mOutgoingDisplayArea.addView(mEndReceiverSessionImageView, layoutParams);

		// Switch button
		layoutParams = new RelativeLayout.LayoutParams(SWITCH_BUTTON_WIDTH, SWITCH_BUTTON_HEIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
		layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
				SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
				SWITCH_BUTTON_MARGIN_BOTTOM);
		mOutgoingDisplayArea.addView(mSwitchCamerImageView, layoutParams);

		mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
		mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
	}

	private void initializeReceiverRemoteView() {
		Logger.v(TAG, "initializeReceiverRemoteView() entry, mIncomingVideoFormat: "
				+ mIncomingVideoFormat + "mShareStatus: " + mShareStatus);
		removeIncomingVideoSharingRemoteVews();
		Logger.v(TAG, "initializeReceiverRemoteView ,mIncomingVideoFormat" +mIncomingVideoFormat);
		if (mIncomingDisplayArea != null) {

			mIncomingRemoteVideoRenderer = new VideoRenderer();


			mIncomingRemoteVideoSurfaceView = new VideoSurfaceView(mContext);
            mIncomingRemoteVideoRenderer.setVideoSurface(mIncomingRemoteVideoSurfaceView);
			mIncomingRemoteVideoSurfaceView.setVisibility(View.VISIBLE);
			LayoutParams params =
					new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
							ViewGroup.LayoutParams.MATCH_PARENT);
			mIncomingRemoteVideoSurfaceView.setLayoutParams(params);
			mIncomingRemoteVideoSurfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);


		} else {
			Logger.w(TAG, "mIncomingDisplayArea is null");
		}
		if (mOutgoingDisplayArea != null) {
			mOutgoingDisplayArea.removeView(mOutgoingLocalVideoSurfaceView);
		} else {
			Logger.w(TAG, "mOutgoingDisplayArea is null");
		}
	}

	// When receive video share invitation
	private void showReceiverRemoteView() {
		Logger.v(TAG, "showReceiverRemoteView(), mShareStatus: " + mShareStatus);
		if (mIncomingDisplayArea != null) {
			RelativeLayout.LayoutParams layoutParams = null;

			Logger.d(TAG, "showReceiverRemoteView, is live show!");
			layoutParams =
					new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
							ViewGroup.LayoutParams.MATCH_PARENT);

			mIncomingDisplayArea.addView(mIncomingRemoteVideoSurfaceView, layoutParams);
			mIncomingRemoteVideoSurfaceView.setAspectRatio(mVideoHeight, mVideoWidth);
			//Adding terminate button to first receiver of video 
			layoutParams =
					new RelativeLayout.LayoutParams(Utils.dip2px(mContext, TERMINATE_BUTTON_WIDTH), Utils
							.dip2px(mContext, TERMINATE_BUTTON_HEIGHT));
			layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
			layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
			layoutParams.setMargins(TERMINATE_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
					SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_LEFT),
					SWITCH_BUTTON_MARGIN_BOTTOM);
			mIncomingDisplayArea.addView(mEndReceiverSessionImageView, layoutParams);

			Logger.v(TAG, "mIncomingRemoteVideoSurfaceView = " + mIncomingRemoteVideoSurfaceView
					+ ", mIncomingRemoteVideoSurfaceView.height = "
					+ mIncomingRemoteVideoSurfaceView.getHeight()
					+ ", mIncomingRemoteVideoSurfaceView.width = "
					+ mIncomingRemoteVideoSurfaceView.getWidth());
			mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
			mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
		} else {
			Logger.w(TAG, "mIncomingDisplayArea is null");
		}

	}

	// When the receiver send invitation, this method will be called
	private void showReceiverLocalView() {
		Logger.v(TAG, "showReceiverLocalView() entry, mIncomingVideoFormat is: "
				+ mIncomingVideoFormat);
		removeIncomingVideoSharingLocalVews();
		Logger.v(TAG, "showReceiverLocalView ,mIncomingVideoFormat" +mIncomingVideoFormat);

		mIncomingLocalVideoSurfaceView = new VideoSurfaceView(mContext);
		mIncomingLocalVideoSurfaceView.setAspectRatio(mVideoWidth,
		 mVideoHeight);
		mSurfaceHolder = mIncomingLocalVideoSurfaceView.getHolder();
		mSurfaceHolder.addCallback(VideoSharingPlugin.this);
		mSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
		mIncomingLocalVideoSurfaceView.setZOrderMediaOverlay(true);

		RelativeLayout.LayoutParams layoutParams = null;
		// Local video view
		layoutParams =
				new RelativeLayout.LayoutParams(Utils.dip2px(mContext, MIN_WINDOW_WIDTH), Utils
						.dip2px(mContext, MIN_WINDOW_HEIGHT));
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		layoutParams.setMargins(LOCAL_CAMERA_MARGIN_LEFT, Utils.dip2px(mContext,
				LOCAL_CAMERA_MARGIN_TOP), Utils.dip2px(mContext, LOCAL_CAMERA_MARGIN_RIGHT),
				LOCAL_CAMERA_MARGIN_BOTTOM);
		mIncomingDisplayArea.addView(mIncomingLocalVideoSurfaceView, layoutParams);

		//Adding terminate button to receiver when sending the video, will end send video
		layoutParams =
				new RelativeLayout.LayoutParams(Utils.dip2px(mContext, TERMINATE_BUTTON_WIDTH), Utils
						.dip2px(mContext, TERMINATE_BUTTON_HEIGHT));
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
				SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
				SWITCH_BUTTON_MARGIN_BOTTOM);
		mIncomingDisplayArea.addView(mEndSenderSessionImageView, layoutParams);

		//Adding terminate button to receiver when receiving the video, will end send video
				layoutParams =
						new RelativeLayout.LayoutParams(Utils.dip2px(mContext, TERMINATE_BUTTON_WIDTH), Utils
								.dip2px(mContext, TERMINATE_BUTTON_HEIGHT));
				layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
				layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
				layoutParams.setMargins(TERMINATE_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
						SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_LEFT),
						SWITCH_BUTTON_MARGIN_BOTTOM);
				mIncomingDisplayArea.addView(mEndReceiverSessionImageView, layoutParams);

		// switch button
		layoutParams =
				new RelativeLayout.LayoutParams(SWITCH_BUTTON_WIDTH, SWITCH_BUTTON_HEIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
		layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT, Utils.dip2px(mContext,
				SWITCH_BUTTON_MARGIN_TOP), Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
				SWITCH_BUTTON_MARGIN_BOTTOM);
		mIncomingDisplayArea.addView(mSwitchCamerImageView, layoutParams);


		mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
		mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);

		if (mOutgoingDisplayArea != null) {
			mOutgoingDisplayArea.removeView(mOutgoingLocalVideoSurfaceView);
		} else {
			Logger.w(TAG, "mOutgoingDisplayArea is null");
		}
	}

	private void showRemoteAcceptMessage() {
		Logger.v(TAG, "showRemoteAcceptMessage()");
		mMainHandler.post(new Runnable() {
			@Override
			public void run() {
				String message = mContext.getResources().getString(R.string.remote_has_accepted);
				showToast(message);

				Logger.d(TAG, "showRemoteAcceptMessage(), is live video share.");

			}
		});
	}


	private void showWaitRemoteAcceptMessage() {
		Logger.v(TAG, "showWaitRemoteAcceptMessage()");
		Runnable runnable = new Runnable() {
			@Override
			public void run() {
				String message = mContext.getResources().getString(R.string.wait_remote_to_accept);
				showToast(message);
			}
		};
		mMainHandler.postDelayed(runnable, DELAY_TIME);
	}

	private void resetAtomicBolean() {
		Logger.v(TAG, "resetAtomicBolean()");
		mIsVideoSharingSender.set(false);
		mIsVideoSharingReceiver.set(false);
		mIsStarted.set(false);
	}

	/**
	 * When receiving stream is terminated by receiver in two way , from the local End 
	 */
	private void finishReceiverTwoWayLocal()
	{
		Logger.v(TAG, "finishReceiverTwoWayLocal() entry");
		new AsyncTask<Void, Void, Void>() {
			@Override
			protected Void doInBackground(Void... params) {
				Logger.v(TAG, "destroy in a background thread.");
				if (mIncomingVideoSharingSession != null) {
					try {
						mIncomingVideoSharingSession.removeSessionListener(mIncomingSessionEventListener);
						mIncomingVideoSharingSession.cancelSession();
					} catch (RemoteException e) {
						e.printStackTrace();
					} finally {
						mIncomingVideoSharingSession = null;
					}
				}         
				return null;            }

		}.execute();

		if (mIncomingDisplayArea != null) {
			Logger.v(TAG, "finishReceiverTwoWayLocal() mIncomingDisplayArea Entry");
			mIncomingDisplayArea.removeView(mIncomingLocalVideoSurfaceView);
			mIncomingDisplayArea.removeView(mIncomingRemoteVideoSurfaceView);
			mIncomingDisplayArea.removeView(mSwitchCamerImageView);
			mIncomingDisplayArea.removeView(mEndReceiverSessionImageView);
			mIncomingDisplayArea.removeView(mEndSenderSessionImageView);                      


		} else {
			Logger.w(TAG, " finishReceiverTwoWayLocal mIncomingDisplayArea is null");
		}   

		if (mOutgoingDisplayArea != null) {
			Logger.w(TAG, " finishReceiverTwoWayLocal mOutgoingDisplayArea entry");
			mOutgoingDisplayArea.removeView(mOutgoingLocalVideoSurfaceView);
			mOutgoingDisplayArea.removeView(mOutgoingRemoteVideoSurfaceView);
			mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
			mOutgoingDisplayArea.removeView(mEndSenderSessionImageView);
			mOutgoingDisplayArea.removeView(mEndReceiverSessionImageView);

		} else {
			Logger.w(TAG, " finishReceiverTwoWayLocal mOutgoingDisplayArea is null");
		}
		mIsVideoSharingReceiver.set(false); 
		showSenderLocalView();

	}

	public void finishSenderTwoWaylocal()
	{

		Logger.v(TAG, "finishSenderTwoWaylocal() entry");
		new AsyncTask<Void, Void, Void>() {
			@Override
			protected Void doInBackground(Void... params) {
				Logger.v(TAG, "destroy in a background thread.");              

				stopCameraPreview();

				if (mOutgoingVideoPlayer != null) {
					mOutgoingVideoPlayer.stop();
					mOutgoingVideoPlayer = null;
				}

				if (mOutgoingVideoSharingSession != null) {
					try {
						mOutgoingVideoSharingSession.removeSessionListener(mOutgoingSessionEventListener);
						mOutgoingVideoSharingSession.cancelSession();
					} catch (RemoteException e) {
						e.printStackTrace();
					} finally {
						mOutgoingVideoSharingSession = null;
					}
				}                
				return null;
			}

		}.execute();

		if (mIncomingDisplayArea != null) {
			Logger.v(TAG, "finishSenderTwoWaylocal() mIncomingDisplayArea Entry");
			mIncomingDisplayArea.removeView(mIncomingLocalVideoSurfaceView);            
			mIncomingDisplayArea.removeView(mSwitchCamerImageView);            
			mIncomingDisplayArea.removeView(mEndSenderSessionImageView);                 


		} else {
			Logger.w(TAG, " finishSenderTwoWaylocal mIncomingDisplayArea is null");
		}   

		if (mOutgoingDisplayArea != null) {
			Logger.v(TAG, "finishSenderTwoWaylocal() mOutgoingDisplayArea Entry");
			mOutgoingDisplayArea.removeView(mOutgoingLocalVideoSurfaceView);             
			mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
			mOutgoingDisplayArea.removeView(mEndSenderSessionImageView);           

		} else {
			Logger.w(TAG, " finishSenderTwoWaylocal mOutgoingDisplayArea is null");
		}
		mIsVideoSharingSender.set(false); 
		mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);

	}
	/**
	 * Release resource. Please call me at any thread including UI thread
	 */
	private void destroy() {
		Logger.v(TAG, "destroy() entry");
		new AsyncTask<Void, Void, Void>() {
			@Override
			protected Void doInBackground(Void... params) {
				Logger.v(TAG, "destroy in a background thread.");
				resetAtomicBolean();
				// fix NE
				stopCameraPreview();

				if (mOutgoingVideoPlayer != null) {
					mOutgoingVideoPlayer.stop();
					mOutgoingVideoPlayer = null;
				}

				cancelSession();
				return null;
			}

		}.execute();
		mMainHandler.post(new Runnable() {
			@Override
			public void run() {
				removeIncomingVideoSharingVews();
				removeOutgoingVideoSharingVews();
				dismissAllDialogs();
				clearSavedDialogs();
				RCSeInCallUIExtension.getInstance().resetDisplayArea();
				if (mSurfaceHolder != null) {
					mSurfaceHolder.removeCallback(VideoSharingPlugin.this);
				} else {
					Logger.w(TAG, "mSurfaceHolder is null");
				}
			}
		});
		if (mWakeLock.isHeld()) {
			mWakeLock.release();
			Logger.v(TAG, "when destroy, release the wake lock");
		}
		resetVideoShaingState();
		Logger.d(TAG, "destroy() exit");
	}

	private boolean cancelSession() {
		// Should make sure cancelSession run a different thread with
		// RemoteCallback
		Logger.d(TAG, "cancelSession()");
		if (mOutgoingVideoSharingSession != null) {
			try {
				mOutgoingVideoSharingSession.removeSessionListener(mOutgoingSessionEventListener);
				mOutgoingVideoSharingSession.cancelSession();
			} catch (RemoteException e) {
				e.printStackTrace();
			} finally {
				mOutgoingVideoSharingSession = null;
			}
		}
		if (mIncomingVideoSharingSession != null) {
			try {
				mIncomingVideoSharingSession.removeSessionListener(mIncomingSessionEventListener);
				mIncomingVideoSharingSession.cancelSession();
			} catch (RemoteException e) {
				e.printStackTrace();
			} finally {
				mIncomingVideoSharingSession = null;
			}
		}
		return true;
	}


	private void destroyIncomingSessionOnly() {
		Logger.v(TAG, "destroyIncomingSession()");
		if (mIncomingVideoSharingSession != null) {
			AsyncTask.execute(new Runnable() {
				@Override
				public void run() {
					try {
						Logger.v(TAG, "reject session in backgroun thread.");
						mIncomingVideoSharingSession
						.removeSessionListener(mIncomingSessionEventListener);
						mIncomingVideoSharingSession.rejectSession();
					} catch (RemoteException e) {
						e.printStackTrace();
					} finally {
						mIncomingVideoSharingSession = null;
						mIsVideoSharingReceiver.set(false);
					}
				}
			});
		}
	}

	// Sometime a receiver send a viedo sharing invitation may be failed.
	// This method run on ui thread, so you can call it in any thread
	private void destroyOutgoingViewOnly() {
		Logger.v(TAG, "destroyOutgoingViewOnly()");
		mMainHandler.post(new Runnable() {
			@Override
			public void run() {
				if (mIncomingDisplayArea != null) {
					mIncomingDisplayArea.removeView(mIncomingLocalVideoSurfaceView);
					mIncomingDisplayArea.removeView(mSwitchCamerImageView);
				} else {
					Logger.w(TAG, "mIncomingDisplayArea is null.");
				}
			}
		});
	}

	private void resetVideoShaingState() {
		 mMainHandler.post(new Runnable() {
	 			@Override
	 			public void run() {
		Logger.v(TAG, "resetVideoShaingState");
		mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_IDLE);
		Utils.setInVideoSharing(false);
		mShareStatus = ShareStatus.UNKNOWN;
		if (mCallScreenHost != null) {
			mCallScreenHost.onStateChange(Constants.SHARE_VIDEO_STATE_IDLE);
		} else {
			Logger.w(TAG, "mCallScreenHost is null");
		}

	 			}
	 		});
	}

	private void rotateCamera() {
		Logger.v(TAG, "rotateCamera");		
		// release camera
		stopCameraPreview();
		// open the other camera
		if (mOpenedCameraId == CameraInfo.CAMERA_FACING_FRONT) {			
			mCamera = Camera.open(CameraInfo.CAMERA_FACING_FRONT);
		} else {
			mCamera = Camera.open(CameraInfo.CAMERA_FACING_BACK);			
		}
		// restart the preview
		startCameraPreview();
		addPreviewCallback();
	}

	private void switchCamera() {
		Logger.v(TAG, "switchCamera");
		if (mCamerNumber == SINGLE_CAMERA) {
			Logger.w(TAG, "The device only has one camera, so can not switch");
			return;
		}
		// release camera
		stopCameraPreview();
		// open the other camera
		if (mOpenedCameraId == CameraInfo.CAMERA_FACING_BACK) {
			mCamera = Camera.open(CameraInfo.CAMERA_FACING_FRONT);
			mOpenedCameraId = CameraInfo.CAMERA_FACING_FRONT;
		} else {
			mCamera = Camera.open(CameraInfo.CAMERA_FACING_BACK);
			mOpenedCameraId = CameraInfo.CAMERA_FACING_BACK;
		}
		// restart the preview
		
	//	mOutgoingVideoPlayer.setCameraId(mOpenedCameraId);
		startCameraPreview();
		addPreviewCallback();
	}

	private void openDefaultCamera() {
		Logger.v(TAG, "openDefaultCamera");
		mCamera = Camera.open();
		mOpenedCameraId = CameraInfo.CAMERA_FACING_BACK;
	}

	/**
	 * Video sharing invitation receiver.
	 */
	private class VideoSharingInvitationReceiver extends BroadcastReceiver {
		private static final String TAG = "VideoSharingInvitationReceiver";

		@Override
		public void onReceive(Context context, Intent intent) {
			Logger.v(TAG, "onReceive(), context = " + context + ", intent = " + intent);
			if (intent != null) {
				String action = intent.getAction();
				Logger.v(TAG, "action = " + action);
				if (VIDEO_SHARING_INVITATION_ACTION.equals(action)) {
					initAudioButton();
					handleVideoSharingInvitation(context, intent);
				}  else if (intent.ACTION_HEADSET_PLUG.equals(action)) {
					mHeadsetConnected = (intent.getIntExtra("state", 0) == 1);
					Logger.d(TAG, "onReceive() ACTION_HEADSET_PLUG: " + mHeadsetConnected);
				} else if (BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED.equals(action)) {
					int state = intent.getIntExtra(BluetoothHeadset.EXTRA_STATE,
							BluetoothHeadset.STATE_DISCONNECTED);
					Logger.d(TAG, "onReceive() ACTION_CONNECTION_STATE_CHANGED: " + state);
				} else {
					Logger.w(TAG, "Unknown action");
				}
			} else {
				Logger.w(TAG, "intent is null");
			}
		}
	}// end VideoSharingInvitationReceiver

	private void handleVideoSharingInvitation(Context context, Intent intent) {
		Logger.v(TAG, "handleVideoSharingInvitation()");
		// Display invitation dialog
		mIncomingSessionId = intent.getStringExtra(RichcallProxyActivity.SESSION_ID);
		mIncomingVideoFormat = intent.getStringExtra(RichcallProxyActivity.VIDEO_TYPE);
		mVideoDuration = intent.getLongExtra("videosize", -1);
		Logger.v(TAG, "handleVideoSharingInvitation Received Duration is()"+ mVideoDuration);
		String mediaType = intent.getStringExtra(RichcallProxyActivity.MEDIA_TYPE);
		// Video share is of full status.
		boolean canShare = (mShareStatus == ShareStatus.UNKNOWN || mShareStatus == ShareStatus.LIVE_OUT);
		boolean supported = getCapability(mNumber);
		Logger.d(TAG, "handleVideoSharingInvitation() mShareStatus: " + mShareStatus
				+ " supported: " + supported);
		if (!canShare || !supported) {
			try {
				IVideoSharingSession sharingSession = mRichCallApi
						.getVideoSharingSession(mIncomingSessionId);
				if (sharingSession != null) {
					try {
						sharingSession.rejectSession();
					} catch (RemoteException e) {
						e.printStackTrace();
					}
				}
			} catch (ClientApiException e) {
				e.printStackTrace();
			}
			return;
		}
		if (VIDEO_LIVE.equals(mediaType)) {
			if (mShareStatus == ShareStatus.LIVE_OUT) {
				if (is3GMobileNetwork() && mOutgoingVideoSharingSession != null) {
					try {
						if (mOutgoingVideoSharingSession.getSessionState() == SessionState.ESTABLISHED
								|| mOutgoingVideoSharingSession.getSessionState() == SessionState.PENDING) {
							Logger.d(TAG,"Reject the incoming session because the device " +
									"is upder 3G network and the outgoing video share " +
									"session is established or on pending");
							try {
								IVideoSharingSession sharingSession = mRichCallApi
										.getVideoSharingSession(mIncomingSessionId);
								if (sharingSession != null) {
									try {
										sharingSession.rejectSession();
									} catch (RemoteException e) {
										e.printStackTrace();
									}
								}
							} catch (ClientApiException e) {
								e.printStackTrace();
							}
							return;
						}
					} catch (RemoteException e) {
						Logger.e(TAG, e.toString());
					}
				} else {
					mShareStatus = ShareStatus.LIVE_TWOWAY;
				}
			} else {
				mShareStatus = ShareStatus.LIVE_IN;
			}
		} 
		// Tell that it is vs receiver if recevie a vs invitation before
		// start vs
		if (!mWakeLock.isHeld()) {
			mWakeLock.acquire();
			Logger.v(TAG, "when start, acquire a wake lock");
		} else {
			Logger.v(TAG, "when start, the wake lock has been acquired, so do not acquire");
		}
		
			mIsVideoSharingReceiver.set(true);
	
		if (mCallScreenHost != null) {
			mIncomingDisplayArea = mCallScreenHost.requestAreaForDisplay();
		} else {
			Logger.w(TAG, "mCallScreenHost is null");
		}
		// Set application context
		AndroidFactory.setApplicationContext(context);
		if (mRichCallStatus == RichCallStatus.CONNECTED) {
			// Initialize a vs session
			mGetIncomingSessionWhenApiConnected.set(false);
			getIncomingVideoSharingSession();
		} else {
			Logger.v(TAG, "Richcall api not connected, and then connect api");
			mGetIncomingSessionWhenApiConnected.set(true);
			mRichCallApi.connectApi();
		}
	}

	protected boolean is3GMobileNetwork() {
		int networkType = Utils.getNetworkType(mContext);
		boolean is3G = false;
		if (networkType == Utils.NETWORK_TYPE_UMTS || networkType == Utils.NETWORK_TYPE_EDGE
				|| networkType == Utils.NETWORK_TYPE_HSUPA
				|| networkType == Utils.NETWORK_TYPE_HSDPA
				|| networkType == Utils.NETWORK_TYPE_1XRTT
				|| networkType == Utils.NETWORK_TYPE_EHRPD) {
			is3G = true;
		}
		return is3G;
	}

	private void handleUserAcceptVideoSharing() {
		Logger.v(TAG, "handleUserAcceptVideoSharing()");
		if (mIncomingVideoSharingSession != null) {
			Utils.setInVideoSharing(true);
		    if (mIsVideoSharingSender.get()) {
				// Receive vs invitation after invitation other
				Logger.v(TAG, "Receive vs invitation after invitation other");
				handleSecondReceiveInvitation();
			}
			else if (mIsVideoSharingReceiver.get()) {
				// First receive vs invitation and accept
				Logger.v(TAG, "First receive vs invitation and accept");
				handleFirstReceiveInvitation();
			} 
		} else {
			Logger.w(TAG, "mIncomingVideoSharingSession is null");
		}
	}

	private void handleUserDeclineVideoSharing() {
		Logger.v(TAG, "handleUserDeclineVideoSharing() mShareStatus: " + mShareStatus);
		if (mWakeLock.isHeld()) {
			mWakeLock.release();
			Logger.v(TAG, "handleUserDeclineVideoSharing() release wake lock");
		} else {
			Logger.v(TAG, "handleUserDeclineVideoSharing() no need to release wake lock");
		}
		// status here can be: LIVE_TWOWAY, LIVE_IN.
		if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
			mShareStatus = ShareStatus.LIVE_OUT;
		} else {
			mShareStatus = ShareStatus.UNKNOWN;
		}
		if (mIncomingVideoSharingSession != null) {
			// In such case, only need remove incoming ui
			destroyIncomingSessionOnly();
		} else {
			Logger.w(TAG, "vmIncomingVideoSharingSession is null");
		}
	}

	// Receive invitaiton before send invitation
	private void handleFirstReceiveInvitation() {
		Logger.v(TAG, "handleFirstReceiveInvitation");
		try {
			initializeReceiverRemoteView();
			mIncomingVideoSharingSession.setVideoRenderer(mIncomingRemoteVideoRenderer);
			mIncomingVideoSharingSession.acceptSession();
			mVideoSharingDialogManager.showWaitingInitializeConextProgressDialog();
			showReceiverRemoteView();
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}

	// Receive invitation after send invitation
	private void handleSecondReceiveInvitation() {
		Logger.v(TAG, "handleSecondReceiveInvitation");
		try {
			Logger.v(TAG, "Receive invitation after send invitation");
			mVideoSharingDialogManager.showWaitingInitializeConextProgressDialog();
			updateFirstSenderView();
			mIncomingVideoSharingSession.setVideoRenderer(mOutgoingRemoteVideoRenderer);
			mIncomingVideoSharingSession.acceptSession();
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onClick(DialogInterface dialog, int which) {
		Logger.v(TAG, "onClick(), which = " + which);
		switch (which) {
		case DialogInterface.BUTTON_POSITIVE:
			handleUserAcceptVideoSharing();
			break;
		case DialogInterface.BUTTON_NEGATIVE:
			handleUserDeclineVideoSharing();
			break;
		default:
			Logger.v(TAG, "Unknown option");
			break;
		}
	}

	private void getIncomingVideoSharingSession() {
		Logger.v(TAG, "getIncomingVideoSharingSession()");
		try {
			mIncomingVideoSharingSession = mRichCallApi.getVideoSharingSession(mIncomingSessionId);
			if (mIncomingVideoSharingSession != null) {
				mIncomingVideoSharingSession.addSessionListener(mIncomingSessionEventListener);
			} else {
				Logger.w(TAG, "mIncomingVideoSharingSession is null");
			}
		} catch (ClientApiException e) {
			e.printStackTrace();
		} catch (RemoteException e) {
			e.printStackTrace();
		}
		mVideoSharingDialogManager.showInvitationDialog();
	}

	/**
	 * Wait to receive live video from remote Progress dialog
	 */
	public static class WaitingProgressDialog extends DialogFragment {
		public static final String TAG = "WaitingProgressDialog";
		private static Context sContext = null;
		private static Activity sActivity = null;

		public static WaitingProgressDialog newInstance(Context context, Activity activity) {
			WaitingProgressDialog f = new WaitingProgressDialog();
			sContext = context;
			sActivity = activity;
			return f;
		}

		@Override
		public Dialog onCreateDialog(Bundle savedInstanceState) {
			Logger.v(TAG, "onCreateDialog()");
			ProgressDialog dialog = new ProgressDialog(sActivity);
			dialog.setIndeterminate(true);
			dialog.setMessage(sContext.getString(R.string.wait_for_video));
			dialog.setCanceledOnTouchOutside(false);
			return dialog;
		}

		@Override
		public void onDismiss(DialogInterface dialog) {
			super.onDismiss(dialog);
		}
	}

	private class VideoSharingDialogManager {
		private static final String TAG = "VideoSharingDialogManager";
		private DialogInterface.OnClickListener mOnClickListener = new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				Logger.v(TAG, "onClick(), which = " + which);
				switch (which) {
				case DialogInterface.BUTTON_POSITIVE:
					Logger.v(TAG, "onClick() stop video sharing");
					destroy();
					break;
				case DialogInterface.BUTTON_NEGATIVE:
					Logger.v(TAG, "onClick() Do nothing");
					break;
				default:
					Logger.v(TAG, "onClick() Unknown option");
					break;
				}
				dismissDialog();
				clearSavedDialogs();
			}
		};

		public void showTerminatedByRemoteDialog() {
			Logger.v(TAG, "showTerminatedByRemoteDialog() entry");
			destroy();
			mMainHandler.post(new Runnable() {
				@Override
				public void run() {
					dismissAllDialogs();
					clearSavedDialogs();
					String msg = mContext.getString(R.string.remote_terminated);
					Logger.v(TAG, "showTerminatedByRemoteDialog msg = " + msg);
					createAndShowAlertDialog(msg, mTerminatedDialogListener);
				}
			});
			Logger.v(TAG, "showTerminatedByRemoteDialog() exit");
		}
		DialogInterface.OnClickListener mTerminatedDialogListener = new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				Logger.v(TAG, "mTerminatedDialogListener, which = " + which);
				switch (which) {
				case DialogInterface.BUTTON_POSITIVE:
					Logger.v(TAG, "remvoe all Dialogs");                   
					break;
				case DialogInterface.BUTTON_NEGATIVE:
					Logger.v(TAG, "Do nothing");
					break;
				default:
					Logger.v(TAG, "Unknown option");
					break;
				}
				dismissDialog();
				clearSavedDialogs();
			}
		};

		private void showTerminatedByNetworkDialog() {
			Logger.d(TAG, "showTerminatedByNetworkDialog() entry");
			destroy();
			mMainHandler.post(new Runnable() {
				@Override
				public void run() {
					dismissAllDialogs();
					clearSavedDialogs();
					String msg =
							mContext.getString(R.string.video_sharing_terminated_due_to_network);
					Logger.v(TAG, "showTerminatedByNetworkDialog msg = " + msg);
					createAndShowAlertDialog(msg, mOnClickListener);
				}
			});
			Logger.d(TAG, "showTerminatedByNetworkDialog() exit");
		}

		public void showWaitingInitializeConextProgressDialog() {
			Logger.d(TAG, "showWaitingInitializeConextProgressDialog() entry");
			if (mCallScreenHost != null) {
				dismissAllDialogs();
				clearSavedDialogs();
				Activity activity = mCallScreenHost.getCallScreenActivity();
				mWaitingProgressDialog =
						WaitingProgressDialog.newInstance(mContext, mCallScreenHost
								.getCallScreenActivity());
				mWaitingProgressDialog.show(activity.getFragmentManager(), TAG);
			} else {
				Logger.d(TAG, "mCallScreenHost is null");
			}
			Logger.d(TAG, "showWaitingInitializeConextProgressDialog() exit");
		}

		public void dismissWaitingInitializeConextProgressDialog() {
			Logger.d(TAG, "dismissWaitingInitializeConextProgressDialog");
			if (mWaitingProgressDialog != null) {
				mWaitingProgressDialog.dismiss();
			} else {
				Logger.d(TAG, "mWaitingProgressDialog is null");
			}
		}

		public void showRejectedByRemoteDialog() {
			Logger.v(TAG, "showRejectedByRemoteDialog() mShareStatus: " + mShareStatus);
			if (mIsVideoSharingSender.get()) {
				Logger.v(TAG, "showRejectedByRemoteDialog(), sender is rejected by remote");
				destroy();
			} else {
				Logger.v(TAG, "showRejectedByRemoteDialog(), receiver is rejected by remote");
				// share status here can be: LIVE_TWOWAY LIVE_OUT 
				if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
					mShareStatus = ShareStatus.LIVE_IN;
				} else {
					mShareStatus = ShareStatus.UNKNOWN;
				}
				mIsStarted.set(false);
			}
			mMainHandler.post(new Runnable() {
				@Override
				public void run() {
					dismissAllDialogs();
					clearSavedDialogs();
					String msg = mContext.getString(R.string.remote_reject);
					Logger.v(TAG, "showRejectedByRemoteDialog msg = " + msg);
					createAndShowAlertDialog(msg, mRejectDialogListener);
				}
			});
		}

		DialogInterface.OnClickListener mRejectDialogListener = new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				Logger.v(TAG, "onClick(), which = " + which);
				switch (which) {
				case DialogInterface.BUTTON_POSITIVE:
					Logger.v(TAG, "remvoe local video view");
					removeIncomingVideoSharingLocalVews();
					break;
				case DialogInterface.BUTTON_NEGATIVE:
					Logger.v(TAG, "Do nothing");
					break;
				default:
					Logger.v(TAG, "Unknown option");
					break;
				}
				dismissDialog();
				clearSavedDialogs();
			}
		};

		private CallScreenDialog createAndShowAlreadyGoingDialog(String msg,
				DialogInterface.OnClickListener DialogListener) {
			Logger.v(TAG, "createAlertDialog(), msg = " + msg);
			if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
				Logger.v(TAG, "createAndShowAlertDialog(), call screen host entry!");
				Activity activity = mCallScreenHost.getCallScreenActivity();
				final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
				callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
				callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
				callScreenDialog.setMessage(msg);
				callScreenDialog.setPositiveButton(mContext
						.getString(R.string.rcs_dialog_positive_button), DialogListener);	
				callScreenDialog.setNegativeButton(mContext
						.getString(R.string.rcs_dialog_negative_button), DialogListener);				
				callScreenDialog.setCancelListener(
						new DialogInterface.OnCancelListener() {
							@Override
							public void onCancel(DialogInterface dialog) {
								mCallScreenDialogSet.remove(callScreenDialog);
								clearSavedDialogs();
							}
						});
				clearSavedDialogs();
				callScreenDialog.show();
				mCallScreenDialogSet.add(callScreenDialog);
				saveAlertDialogs();
				return callScreenDialog;
			} else {
				Logger.w(TAG, "mCallScreenHost is null or activity is null.");
				return null;
			}
		}

		public void alreadyOnGoingImageShare() {
			Logger.v(TAG, "alreadyOnGoingImageShare entry");      

			mMainHandler.post(new Runnable() {
				@Override
				public void run() {
					String message =
							mContext.getResources().getString(R.string.image_share_ongoing_video_start);                
					Logger.v(TAG, "alreadyOnGoingImageShare msg = " + message);
					createAndShowAlreadyGoingDialog(message,mAlreadyOnGoingShareListener );

				}
			});
			Logger.v(TAG, "alreadyOnGoing exit");
		}

		DialogInterface.OnClickListener mAlreadyOnGoingShareListener = new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				Logger.v(TAG, "onClick(), which = " + which);
				switch (which) {
				case DialogInterface.BUTTON_POSITIVE:
					Logger.v(TAG, "stop image share first");
					RCSeInCallUIExtension.getInstance().getmShareFilePlugIn().stop();
					startVideoShare();
					break;
				case DialogInterface.BUTTON_NEGATIVE:
					Logger.v(TAG, "Do nothing");
					break;
				default:
					Logger.v(TAG, "Unknown option");
					break;
				}
				dismissDialog();
				clearSavedDialogs();
			}
		};


		public void showTimeOutDialog() {
			Logger.d(TAG, "showTimeOutDialog entry");
			dismissAllDialogs();
			clearSavedDialogs();
			mMainHandler.post(new Runnable() {
				@Override
				public void run() {
					Logger.d(TAG, "showTimeOutDialog() create time out dialog!");
					String msg = mContext.getString(R.string.video_sharing_invitation_time_out);
					createAndShowAlertDialog(msg, mOnClickListener);
				}
			});
			Logger.d(TAG, "showTimeOutDialog exit");
		}


		private CallScreenDialog createAndShowAlertDialog(String msg,
				DialogInterface.OnClickListener posiviteListener) {
			Logger.v(TAG, "createAlertDialog(), msg = " + msg);
			if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
				Logger.v(TAG, "createAndShowAlertDialog(), call screen host entry!");
				Activity activity = mCallScreenHost.getCallScreenActivity();
				final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
				callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
				callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
				callScreenDialog.setMessage(msg);
				callScreenDialog.setPositiveButton(mContext
						.getString(R.string.rcs_dialog_positive_button), posiviteListener);
				callScreenDialog.setCancelListener(
						new DialogInterface.OnCancelListener() {
							@Override
							public void onCancel(DialogInterface dialog) {
								mCallScreenDialogSet.remove(callScreenDialog);
								clearSavedDialogs();
							}
						});
				clearSavedDialogs();
				callScreenDialog.show();
				mCallScreenDialogSet.add(callScreenDialog);
				saveAlertDialogs();
				return callScreenDialog;
			} else {
				Logger.w(TAG, "mCallScreenHost is null or activity is null.");
				return null;
			}
		}
		/**
		 * Show the video sharing invitation dialog.
		 * 
		 * @return True if show dialog successfully, otherwise return false.
		 */
		private boolean showInvitationDialog() {
			Logger.v(TAG, "showInvitationDialog()");
			dismissAllDialogs();
			clearSavedDialogs();
			// Vibrate
			Utils.vibrate(mContext, Utils.MIN_VIBRATING_TIME);
			CallScreenDialog callScreenDialog = createInvitationDialog();
			if (callScreenDialog != null) {
				callScreenDialog.show();
				saveAlertDialogs();
				return true;
			}
			return false;
		}

		/**
		 * Create a video sharing invitation dialog, and add it to the set
		 * 
		 * @return The dialog created.
		 */
		private CallScreenDialog createInvitationDialog() {
			Logger.v(TAG, "createInvitationDialog()");
			CallScreenDialog callScreenDialog = null;
			if (mCallScreenHost != null) {
				callScreenDialog = new CallScreenDialog(mCallScreenHost.getCallScreenActivity());
				callScreenDialog.setPositiveButton(
						mContext.getString(R.string.rcs_dialog_positive_button),
						VideoSharingPlugin.this);
				callScreenDialog.setNegativeButton(
						mContext.getString(R.string.rcs_dialog_negative_button),
						VideoSharingPlugin.this);
				callScreenDialog.setTitle(mContext
						.getString(R.string.video_sharing_invitation_dialog_title));
				callScreenDialog.setMessage(mContext
						.getString(R.string.video_sharing_invitation_dialog_content));
				callScreenDialog.setCancelable(false);
				mCallScreenDialogSet.add(callScreenDialog);
			}
			Logger.d(TAG, "createInvitationDialog() exit, callScreenDialog = " + callScreenDialog);
			return callScreenDialog;
		}
	}



	/**
	 * Share status.
	 */
	private static final class ShareStatus {
		public static final int UNKNOWN = 0;
		public static final int LIVE_OUT = 1;
		public static final int LIVE_IN = 2;
		public static final int LIVE_TWOWAY = 3;
	}

	@Override
	public void onClick(View v) {
		Logger.v(TAG, "onClick()");
		switch (v.getId()) {
		case R.drawable.btn_terminate_video_share_pre_receiver: {
			Logger.v(TAG, "onClick() receiver Button");
			if (mShareStatus == ShareStatus.LIVE_IN) {
				Logger.v(TAG, "onClick() receiver Button LIVE_IN");
				VideoSharingPlugin.this.stop();
			}
			else if(mShareStatus == ShareStatus.LIVE_TWOWAY)
			{
				Logger.v(TAG, "onClick() receiver Button LIVE_TWOWAY");
				finishReceiverTwoWayLocal();
				mShareStatus = ShareStatus.LIVE_OUT;
			}

		}
		break;
		case R.drawable.btn_terminate_video_share_pre_sender: {
			Logger.v(TAG, "onClick() Sender Button");
			if (mShareStatus == ShareStatus.LIVE_OUT) {

				Logger.v(TAG, "onClick() receiver Button LIVE_OUT");
				VideoSharingPlugin.this.stop();
			}
			else if(mShareStatus == ShareStatus.LIVE_TWOWAY)
			{
				Logger.v(TAG, "onClick() Sender Button LIVE_TWOWAY");
				finishSenderTwoWaylocal();
				mShareStatus = ShareStatus.LIVE_IN;
			}
		}
		break;
		}

	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		Logger.v(TAG, "onSensorChanged() " + event);
		synchronized (mLock) {
            if (mCamera != null) {
                if (mCameraPreviewRunning) {
                    mCameraPreviewRunning = false;
                    mCamera.stopPreview();
                }
            } else {
                Logger.w(TAG, "mCamera is null");
            }
        }
        boolean isLive = (mShareStatus == ShareStatus.LIVE_IN
                || mShareStatus == ShareStatus.LIVE_OUT || mShareStatus == ShareStatus.LIVE_TWOWAY);
        if (isLive) {
            startCameraPreview();
        }
		
	}
}
