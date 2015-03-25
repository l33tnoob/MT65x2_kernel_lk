package com.mediatek.rcse.ipcall;

import java.lang.reflect.Method;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.view.Display;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TextView;

import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.H264Config;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.video.CameraOptions;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.video.Orientation;
import com.orangelabs.rcs.core.ims.service.ipcall.IPCallError;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.ipcall.IIPCallEventListener;
import com.orangelabs.rcs.service.api.client.ipcall.IIPCallSession;
import com.orangelabs.rcs.service.api.client.media.MediaCodec;
import com.orangelabs.rcs.service.api.client.media.audio.AudioRenderer;
import com.orangelabs.rcs.service.api.client.media.audio.LiveAudioPlayer;
import com.orangelabs.rcs.service.api.client.media.video.LiveVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.video.VideoRenderer;
import com.orangelabs.rcs.service.api.client.media.video.VideoSurfaceView;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * IP call view
 */
public class IPCallView extends Activity implements SurfaceHolder.Callback {

	/**
	 * UI handler
	 */
	public static final  Handler handler = new Handler();

	/**
	 * Activity is in state onResume
	 */ 
	private boolean  isVisible = false;
	
	/**
	 * current session
	 */
	private IIPCallSession session = null;

	/**
	 *  current session ID
	 */
	private String sessionId = null;

	/**
	 * current RemoteContact
	 */
	public String remoteContact;

	/**
	 * direction ("outgoing" "incoming") of current session
	 */
	public String direction  = "";
	
	/**
	 * video requested
	 */
	private boolean videoSelected  = false;
	
	/**
	 * video connected
	 */
	private boolean videoConnected  = false;
	
	/**
	 * Progress dialog
	 */
	private static Dialog outgoingProgressDialog = null;
	
	/**
	 * Add Video Invitation dialog
	 */
	private AlertDialog addVideoInvitationDialog;
	
	/**
	 * incoming call Invitation dialog
	 */
	private AlertDialog incomingCallInvitationDialog;

	/**
	 * HangUp button
	 */
	private static Button hangUpBtn = null;

	/**
	 * Set On Hold button
	 */
	private static Button setOnHoldBtn = null;

	/**
	 * Add Video button
	 */
	private static Button addVideoBtn = null;
	
	/**
	 * Switch camera button 
	 */
    private static Button switchCamBtn = null;

	/**
	 * Audio player
	 */
	private LiveAudioPlayer outgoingAudioPlayer = null;

	/**
	 * Audio renderer
	 */
	private AudioRenderer incomingAudioRenderer = null;

	/**
	 * Video player
	 */
	private LiveVideoPlayer outgoingVideoPlayer = null;
	
	/**
	 * Video renderer
	 */
	private VideoRenderer incomingVideoRenderer = null;
	
    /** Camera */
    private Camera camera = null;

    /** Opened camera id */
    private CameraOptions openedCameraId = CameraOptions.FRONT;

    /** Camera preview started flag */
    private boolean cameraPreviewRunning = false;

    /** Number of cameras */
    private int numberOfCameras = 1;
    
    /** Incoming Video preview */
    private VideoSurfaceView incomingVideoView = null;
    
    /** Incoming Video width */
    private int incomingWidth = 0;

    /** Incoming Video height */
    private int incomingHeight = 0;
    
    /** Outgoing Video preview */
    private VideoSurfaceView outgoingVideoView = null;
    
    /** Outgoing Video width */
    private int outgoingWidth = H264Config.QCIF_WIDTH;

    /** Outgoing Video height */
    private int outgoingHeight = H264Config.QCIF_HEIGHT;

    /** Outgoing Video surface holder */
    private SurfaceHolder surface;
    
    /** Preview surface view is created */
    private boolean isSurfaceCreated = false;
    
    /** wait surface creation to launch outgoing session */
    private boolean startOutgoingSessionWhenSurfaceCreated = false;
    
    /** wait surface creation to accept incoming session */
    private boolean acceptIncomingSessionWhenSurfaceCreated = false;
    
    /**  (session cancellation requested) stop session as soon as initiated */
    private boolean cancelOutgoingSessionWhenCreated = false ;

	/**
	 * Logger
	 */
	private Logger logger = Logger.getLogger(IPCallView.class.getName());
	
    /* *****************************************
     *                Activity
     ***************************************** */

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if (logger.isActivated()) {
			logger.info("onCreate()");
		}
		
		// Set layout
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
		setContentView(R.layout.ipcall_session2);

		// Set title
		setTitle(R.string.title_ipcall);

		// set buttons
		setOnHoldBtn = (Button) findViewById(R.id.set_onhold_btn);
		setOnHoldBtn.setOnClickListener(btnOnHoldListener);

		hangUpBtn = (Button) findViewById(R.id.hangup_btn);
		hangUpBtn.setOnClickListener(btnHangUpListener);
		hangUpBtn.setEnabled(false);

		addVideoBtn = (Button) findViewById(R.id.add_video_btn);
		addVideoBtn.setOnClickListener(btnAddVideoListener);
		addVideoBtn.setEnabled(false);

		switchCamBtn = (Button) findViewById(R.id.switch_cam_btn);
		switchCamBtn.setVisibility(View.GONE);

		outgoingVideoView = (VideoSurfaceView) findViewById(R.id.outgoing_ipcall_video_preview);
		outgoingVideoView.setVisibility(View.GONE);

		incomingVideoView = (VideoSurfaceView) findViewById(R.id.incoming_ipcall_video_view);
		incomingVideoView.setVisibility(View.GONE);
		
		String action = getIntent().getAction();
		if (logger.isActivated()) {
			logger.info("action =" + action);
		}
		
		if (action != null){
			// API or IMS disconnection
			if (action.equals("ExitActivity")) {
				exitActivity(getIntent().getStringExtra("messages"));	
			}
			else {
				// instantiate Audio Player and Renderer
				incomingAudioRenderer = new AudioRenderer(this);
				outgoingAudioPlayer = new LiveAudioPlayer(incomingAudioRenderer);
				
				// IP call already established - recover existing session
				if (action.equals("recover")) {
					// get sessionId from Intent
					sessionId = getIntent().getStringExtra("sessionId");	
					if (sessionId == null) {
						exitActivity("sessionId is null");
					} else {
						recoverSessions();
					}
					
				} 
				else if ((action.equals("incoming")||(action.equals("outgoing")))) {
					// Get remote contact from intent
					remoteContact = getIntent().getStringExtra("contact");

					// set direction of call
					direction = action ;

					// reset session and sessionid 
					session = null;
					sessionId = "";
					
					// incoming ip call
					if (action.equals("incoming")){
						// Get intent info
						sessionId = getIntent().getStringExtra("sessionId");
						videoSelected =  (getIntent().getStringExtra("videotype")!=null)? true: false;
						if (logger.isActivated()) {
							logger.info("video =" + videoSelected);
						}
						
						// remove notification
						if (!sessionId.equals(null)) {
							InitiateIPCall.removeIPCallNotification(
									getApplicationContext(), sessionId);
						}
						getIncomingSession();
					}
					// initiate an outgoing ip call
					else if (action.equals("outgoing")) {					
						// Get intent info
						videoSelected = getIntent().getBooleanExtra("video", false);
						if (logger.isActivated()) {
							logger.info("video =" + videoSelected);
						}
						
						InitiateIPCall.initCallProgressDialog.dismiss();
						// display loader
						displayProgressDialog();
						
						// prepare video session (camera surface View ...)
						prepareVideoSession() ;
						
						// set video state - layout and buttons for video
						if (videoSelected){
							addVideoBtn.setEnabled(false);
							addVideoBtn.setText(R.string.label_remove_video_btn);
							outgoingVideoView.setVisibility(View.VISIBLE);
							incomingVideoView.setVisibility(View.VISIBLE);
							switchCamBtn.setVisibility(View.VISIBLE);
							switchCamBtn.setEnabled(false);
						}
						
						if ((!videoSelected)||((videoSelected)&& isSurfaceCreated)){
							startOutgoingSession();	
						}
						else {
							startOutgoingSessionWhenSurfaceCreated = true ;
						}										
					}			
				}
			}

		} 
		else {exitActivity(null);}
	}
	

	public void onResume() {
		super.onResume();
		if (logger.isActivated()) {
			logger.info("onResume()");
		}
		// activity is displayed
		isVisible = true; 
		
		// set contact number
		TextView fromTxt = (TextView) findViewById(R.id.ipcall_with_txt);
		fromTxt.setText(getString(R.string.label_ipcall_with, remoteContact));
	}

	@Override
	public void onPause() {
		super.onPause();
		if (logger.isActivated()) {
			logger.info("onPause()");
		}
		
		isVisible = false; 
	}
	
	public void onFinish(){
		super.finish();
		isVisible = false ;
	}

	@Override
	protected void onNewIntent(Intent intent) {
		super.onNewIntent(intent);
		if (logger.isActivated()) {
			logger.info("onNewIntent()");
		}
		if (intent.getAction().equals("ExitActivity")) {
			exitActivity(intent.getExtras().getString("messages")) ;
		}
		// other actions
		else {setIntent(intent);}
	}

	 @Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (logger.isActivated()) {
			logger.info("onKeyDown()");
		}
		switch (keyCode) {
		case KeyEvent.KEYCODE_BACK:
			// save current session data
			if (logger.isActivated()) {
				logger.info("onKeyBack()");
			}
			if (videoConnected) {
				removeVideo();
			}

			try {
				session.removeSessionListener(sessionEventListener);
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			this.finish();
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
	
	@Override
	public void onDestroy() {
		super.onDestroy();
		if (logger.isActivated()) {
			logger.info("onDestroy()");
		}
	}

	
	/* *****************************************
     *              Button listeners
     ***************************************** */

	/**
	 * Accept button listener
	 */
	private OnClickListener acceptBtnListener = new OnClickListener() {
		public void onClick(DialogInterface dialog, int which) {
			if (logger.isActivated()) {
				logger.info("onClick()");
			}

			// remove notif
			InitiateIPCall.removeIPCallNotification(getApplicationContext(), sessionId);
			
			// display loader
			displayProgressDialog();	
					
			// prepare video session (camera surface View ...)
			prepareVideoSession() ;
			
			// set video state - layout and buttons for video
			if (videoSelected){
				addVideoBtn.setEnabled(false);
				addVideoBtn.setText(R.string.label_remove_video_btn);
				outgoingVideoView.setVisibility(View.VISIBLE);
				incomingVideoView.setVisibility(View.VISIBLE);
				switchCamBtn.setVisibility(View.VISIBLE);
				switchCamBtn.setEnabled(false);
			}
			 
			if (!videoSelected){
				acceptIncomingSession();
			}
			else if ((videoSelected)&& isSurfaceCreated){	
				acceptIncomingSession();
			}
			else {//videoOption And surface Not created
				acceptIncomingSessionWhenSurfaceCreated = true ;
			}
		}
	};

	/**
	 * Reject button listener
	 */
	private OnClickListener declineBtnListener = new OnClickListener() {
		public void onClick(DialogInterface dialog, int which) {
			InitiateIPCall.removeIPCallNotification(getApplicationContext(), sessionId);
			declineIncomingSession();
		}
	};

	/**
	 * Set On Hold button listener
	 */
	private View.OnClickListener btnOnHoldListener = new View.OnClickListener() {
		public void onClick(View v) {
			Utils.showMessage(IPCallView.this, getString(R.string.label_not_implemented));
			// TODO
		}
	};

	/**
	 * Hang Up button listener
	 */
	private View.OnClickListener btnHangUpListener = new View.OnClickListener() {
		public void onClick(View v) {
			Thread thread = new Thread() {
				public void run() {
					releaseCamera() ;
					stopSession();
					exitActivity(null);
				}
			};
			thread.start();
		}
	};

	/**
	 * Add Video button listener
	 */
	private View.OnClickListener btnAddVideoListener = new View.OnClickListener() {
		public void onClick(View v) {
			if (logger.isActivated()) {
				logger.info("btnAddVideoListener.onClick()");			
			}
			
			if (!videoConnected) {
				addVideoBtn.setEnabled(false);
				addVideoBtn.setText(R.string.label_remove_video_btn);
				outgoingVideoView.setVisibility(View.VISIBLE);
				incomingVideoView.setVisibility(View.VISIBLE);
				switchCamBtn.setVisibility(View.VISIBLE);
				switchCamBtn.setEnabled(false);
				addVideo();
			} else if (videoConnected) {
				addVideoBtn.setEnabled(false);
				addVideoBtn.setText(R.string.label_add_video_btn);
				switchCamBtn.setEnabled(false);
				videoConnected = false;
				removeVideo();
			}
		}
	};

	/**
     * Switch camera button listener
     */
    private View.OnClickListener btnSwitchCamListener = new View.OnClickListener() {
        public void onClick(View v) {
            // Release camera
            releaseCamera();

            // Open the other camera
            if (openedCameraId.getValue() == CameraOptions.BACK.getValue()) {
                OpenCamera(CameraOptions.FRONT);
            } else {
                OpenCamera(CameraOptions.BACK);
            }

            // Restart the preview
            camera.setPreviewCallback(outgoingVideoPlayer);
            startCameraPreview();
        }
    };
	
	/* *****************************************
     *              session events listener
     ***************************************** */
	
	/**
	 * IP call session event listener
	 */
	private IIPCallEventListener sessionEventListener = new IIPCallEventListener.Stub() {
		// Session is started
		public void handleSessionStarted() {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleSessionStarted()");
			}
			handler.post(new Runnable() {
				public void run() {
					hideProgressDialog();
					hangUpBtn.setEnabled(true);
					//setOnHoldBtn.setEnabled(true);
					addVideoBtn.setEnabled(true);
					// video
					if (videoSelected){
						try {
							boolean video = ((session.getVideoPlayer()== null)||(session.getVideoRenderer()== null)) ? false : true ;
							if (video) {
								videoConnected = true;
								addVideoBtn.setEnabled(true);
								switchCamBtn.setEnabled(true);
								// Update Camera
			                    outgoingHeight = outgoingVideoPlayer.getVideoCodecHeight();
			                    outgoingWidth = outgoingVideoPlayer.getVideoCodecWidth();
			                    if (logger.isActivated()) {
			        				logger.info("Update Camera - outGoingHeight ="+outgoingHeight+"- outGoingWidth = "+outgoingWidth);
			        			}
			                    reStartCamera();
							}
							else { // video declined
								videoConnected = false;
								addVideoBtn.setText(R.string.label_add_video_btn);
								switchCamBtn.setEnabled(false);
								outgoingVideoView.setVisibility(View.GONE);
								incomingVideoView.setVisibility(View.GONE);
								switchCamBtn.setVisibility(View.GONE);
							}
								
						} catch (RemoteException e) {
							releaseCamera();
							stopSession();
							exitActivity(getString(R.string.label_ipcall_failed)+"-"+ e.getMessage());
							
						} 
						
					}

					
				}
			});
		}

		// Session has been aborted
		public void handleSessionAborted(int reason) {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleSessionAborted()");
			}
			Thread thread = new Thread() {
				public void run() {
					stopSession();
				}
			};
			thread.start();
			handler.post(new Runnable() {
				public void run() {
					releaseCamera() ;
					//setOnHoldBtn.setEnabled(false);
					hangUpBtn.setEnabled(false);
					if (direction.equals("outgoing")){
						exitActivity(getString(R.string.label_outgoing_ipcall_aborted));
					}
					else if (direction.equals("incoming")){
						exitActivity(getString(R.string.label_incoming_ipcall_aborted));
					}
					else {exitActivity(getString(R.string.label_ipcall_failed));}
				}
			});
		}

		// Session has been terminated by remote
		public void handleSessionTerminatedByRemote() {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleSessionTerminatedByRemote()");
			}
			Thread thread = new Thread() {
				public void run() {
					stopSession();
				}
			};
			thread.start();
			handler.post(new Runnable() {
				public void run() {
					releaseCamera() ;
					//setOnHoldBtn.setEnabled(false);
					hangUpBtn.setEnabled(false);
//					stopSession();
					if ((direction != null) && (direction.equals("outgoing"))){
						exitActivity(getString(R.string.label_outgoing_ipcall_terminated_by_remote));
					}
					else if ((direction != null) && (direction.equals("incoming"))){
						exitActivity(getString(R.string.label_incoming_ipcall_terminated_by_remote));
					}	
					else { exitActivity(null);}
				}
			});
		}

		// IP call error
		public void handleCallError(final int error) {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleCallError()");
			}		
			Thread thread = new Thread() {
				public void run() {
					stopSession();
				}
			};
			thread.start();
			handler.post(new Runnable() {
				public void run() {
					releaseCamera() ;
					//setOnHoldBtn.setEnabled(false);
					hangUpBtn.setEnabled(false);
					addVideoBtn.setEnabled(false);

					if (error == IPCallError.SESSION_INITIATION_DECLINED) {
						exitActivity(getString(R.string.label_ipcall_invitation_declined));
					}
					else {
						exitActivity(getString(R.string.label_ipcall_failed));
					}
				}
			});	
		}

		@Override
		public void handleVideoResized(final int width, final int height)
				throws RemoteException {
			if (logger.isActivated()) {
				logger.info("handleMediaResized - width =" + width
						+ " - height =" + height);
			}

			incomingWidth = width;
			incomingHeight = height;
			handler.post(new Runnable() {
				public void run() {
					incomingVideoView.setAspectRatio(incomingWidth,
							incomingHeight);
				}
			});

		}

		@Override
		public void handle486Busy() throws RemoteException {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handle486Busy()");
			}
			Thread thread = new Thread() {
				public void run(){
					stopSession();
				}
			};
			thread.start();
			handler.post(new Runnable() {
				public void run() {
					//setOnHoldBtn.setEnabled(false);
					hangUpBtn.setEnabled(false);			
					exitActivity(getString(R.string.label_ipcall_called_is_busy));
				}
			});
		}

		@Override
		public void handleAddVideoInvitation(String arg0, int arg1, int arg2)
				throws RemoteException {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleAddVideoInvitation()");
			}
			handler.post(new Runnable() {
				public void run() {
					if ((IPCallView.this != null) && (isVisible)) {
						getAddVideoInvitation();
					} else {
						rejectAddVideo();
					}
				}
			});
		}

		@Override
		public void handleAddVideoAborted(int arg0) throws RemoteException {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleAddVideoAborted()");
			}
			handler.post(new Runnable() {
				public void run() {
					releaseCamera() ;
					recreateVideoPlayer();
					videoConnected = false;
					addVideoBtn.setText(R.string.label_add_video_btn);
					addVideoBtn.setEnabled(true);
					outgoingVideoView.setVisibility(View.GONE);
					incomingVideoView.setVisibility(View.GONE);
					switchCamBtn.setVisibility(View.GONE);
					if (addVideoInvitationDialog != null){
						addVideoInvitationDialog.dismiss();
						
					}
				}
			});
		}

		@Override
		public void handleAddVideoAccepted() throws RemoteException {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleAddVideoAccepted()");
			}
			handler.post(new Runnable() {
				public void run() {
					videoConnected= true;
					addVideoBtn.setText(R.string.label_remove_video_btn);
					addVideoBtn.setEnabled(true);
					switchCamBtn.setEnabled(true);
					
					// Update Camera
                    outgoingHeight = outgoingVideoPlayer.getVideoCodecHeight();
                    outgoingWidth = outgoingVideoPlayer.getVideoCodecWidth();
                    if (logger.isActivated()) {
        				logger.info("Update Camera - outGoingHeight ="+outgoingHeight+"- outGoingWidth = "+outgoingWidth);
        			}
                    reStartCamera();
				}
			});
		}

		@Override
		public void handleRemoveVideo() throws RemoteException {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleRemoveVideoInvitation()");
			}
			handler.post(new Runnable() {
				public void run() {
					videoConnected = false;
					addVideoBtn.setText(R.string.label_add_video_btn);
					addVideoBtn.setEnabled(false);
				}
			});
		}

		public void handleRemoveVideoAccepted() throws RemoteException {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleRemoveVideoAccepted()");
			}
			handler.post(new Runnable() {
				public void run() {
					releaseCamera();
					recreateVideoPlayer();
					videoConnected= false;
					addVideoBtn.setText(R.string.label_add_video_btn);
					addVideoBtn.setEnabled(true);
					outgoingVideoView.setVisibility(View.GONE);
					incomingVideoView.setVisibility(View.GONE);
					switchCamBtn.setVisibility(View.GONE);
				}
			});
		}

		public void handleRemoveVideoAborted(int arg0) throws RemoteException {
			if (logger.isActivated()) {
				logger.info("sessionEventListener - handleRemoveVideoAborted()");
			}
			handler.post(new Runnable() {
				public void run() {
					videoConnected = true;
					addVideoBtn.setText(R.string.label_remove_video_btn);
					addVideoBtn.setEnabled(true);
					
				}
			});
			
		}

		@Override
		public void handleCallHold() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void handleCallResume() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void handleCallHoldAccepted() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void handleCallResumeAccepted() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void handleCallHoldAborted(int reason) throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void handleCallResumeAborted(int reason) throws RemoteException {
			// TODO Auto-generated method stub
			
		}
	};


	
	/* *****************************************
     *              session methods
     ***************************************** */
	/**
	 * Get incoming session.
	 */
	private void getIncomingSession() {
		if (logger.isActivated()) {
			logger.debug("getIncomingSession()");
		}

		handler.post(new Runnable() {
			public void run() {
				try {
					session = IPCallSessionsData.getInstance().callApi.getSession(sessionId);
					session.addSessionListener(sessionEventListener);
					
					// construct AlertDialog to accept/reject call if incoming call
					View checkBoxView = View.inflate(IPCallView.this, R.layout.ipcall_incoming_session_dialogbox_checkbox, null);
					final CheckBox checkBox = (CheckBox) checkBoxView.findViewById(R.id.checkBox1);
					if (videoSelected){
						checkBox.setChecked(true);
						checkBox.setClickable(true);
						
						checkBox.setOnClickListener(new View.OnClickListener() {
							@Override
							public void onClick(View arg0) {
								videoSelected = checkBox.isChecked();						
							}
						});
					}
					else {checkBox.setClickable(false);}					
					
					// construct AlertDialog
					AlertDialog.Builder builder = new AlertDialog.Builder(IPCallView.this);
					builder.setTitle(R.string.title_recv_ipcall);
					builder.setMessage(getString(R.string.label_from) + " " + remoteContact);
					builder.setView(checkBoxView);
					builder.setCancelable(false);
					builder.setIcon(R.drawable.ri_notif_ipcall_icon);
					builder.setPositiveButton(getString(R.string.label_accept),
							acceptBtnListener);
					builder.setNegativeButton(getString(R.string.label_decline),
							declineBtnListener);
					incomingCallInvitationDialog = builder.create();

					incomingCallInvitationDialog.show();					
				} catch (final Exception e) {
					handler.post(new Runnable() {
						public void run() {
							exitActivity(getString(R.string.label_incoming_ipcall_failed)+" - error:"+e.getMessage());
						}
					});
				}
			}
		});


	}
	
	
	private void startOutgoingSession() {

		if (logger.isActivated()) {
			logger.debug("startOutgoingSession(" + videoSelected + ")" );
		}
	
		Thread thread = new Thread() {
			public void run() {
				try {
					// Initiate IP call outgoing session
					if (videoSelected) {						
						instantiateIncomingVideoRenderer();
						instantiateOutgoingVideoPlayer();

						session = IPCallSessionsData.getInstance().callApi.initiateCall(
								remoteContact, outgoingAudioPlayer,
								incomingAudioRenderer, outgoingVideoPlayer,
								incomingVideoRenderer);
					} else {
						session = IPCallSessionsData.getInstance().callApi.initiateCall(
								remoteContact, outgoingAudioPlayer,
								incomingAudioRenderer);
					}
					session.addSessionListener(sessionEventListener);
					sessionId = session.getSessionID();
					// cancel session requested by user
					if (cancelOutgoingSessionWhenCreated) {
						stopSession();
						cancelOutgoingSessionWhenCreated = false;}
				} catch (final Exception e) {
					exitActivity(getString(R.string.label_outgoing_ipcall_failed)
							+ " - error:" + e.getMessage());
				}
			}
		};
		thread.start();
	}

	
	/**
	 * Recover incoming/outgoing sessions.
	 */
	private void recoverSessions() {
		if (logger.isActivated()) {
			logger.debug("recoverSessions()");
		}

		// recover session and session data
		try {
			session = IPCallSessionsData.getInstance().callApi
					.getSession(sessionId);
		} catch (ClientApiException e) {
			exitActivity("Call Api exception - unable to get session");
		}

		if (session != null) {
			Thread thread = new Thread() {
				public void run() {
					try {

						session.addSessionListener(sessionEventListener);
						try {
							String contact = session.getRemoteContact() ;
							int beginIdx = contact.indexOf("tel");
							int endIdx = contact.indexOf(">", beginIdx);
							if (endIdx == -1) {endIdx = contact.length();}
							remoteContact = contact.substring(beginIdx, endIdx);
							
							direction = (session.getSessionDirection() == IPCallSessionsData.TYPE_OUTGOING_IPCALL) ? "outgoing"
									: "incoming";
						} catch (RemoteException e1) {
							exitActivity("Call Api exception - unable to get session");
						}

					} catch (final Exception e) {
						exitActivity("Recover sessions failed - error:"
								+ e.getMessage());
					}
				}
			};
			thread.start();
		} else {
			exitActivity("no session established");
		}

		// enables buttons
		// setOnHoldBtn.setText(R.string.label_set_onhold_btn);
		hangUpBtn.setEnabled(true);
		// setOnHoldBtn.setEnabled(true);
		addVideoBtn.setEnabled(true);

		// prepare video session (camera surface View ...)
		prepareVideoSession();

		// video is in IDDLE State (video always stopped when user exit activity
		// by "back" button )
		addVideoBtn.setText(R.string.label_add_video_btn);

	}


	/**
	 * Accept incoming session.
	 */
	private void acceptIncomingSession() {
		if (logger.isActivated()) {
			logger.debug("acceptIncomingSession()");
			logger.debug("videoSelected :"+videoSelected);
		}
		
		// Accept the session in background
		Thread thread = new Thread() {
			public void run() {
				try {
					// Accept the invitation
					session.setAudioPlayer(outgoingAudioPlayer);
					session.setAudioRenderer(incomingAudioRenderer);
					//if video selected instantiate and set on session video player/renderer 
					if (videoSelected) {
						instantiateIncomingVideoRenderer();
						instantiateOutgoingVideoPlayer();
						session.setVideoPlayer(outgoingVideoPlayer);
						session.setVideoRenderer(incomingVideoRenderer);
					}
					session.acceptSession(videoSelected);
				} catch (final Exception e) {
					exitActivity(getString(R.string.label_invitation_failed)
							+ " - error:" + e.getMessage());
				}

			}
		};
		thread.start();	
		
		
	}

	/**
	 * Decline incoming session.
	 */
	private void declineIncomingSession() {
		Thread thread = new Thread() {
			public void run() {
				try {
					// Reject the invitation
					session.removeSessionListener(sessionEventListener);
					session.rejectSession();
					session = null;
					videoConnected = false;

					// Exit activity
					exitActivity(null);
				} catch (final Exception e) {
					exitActivity(getString(R.string.label_invitation_failed)
							+ " - error:" + e.getMessage());
				}
			}
		};
		thread.start();
	}
	
	/**
	 * Stop the outgoing session
	 */
	private void stopSession() {
		if (logger.isActivated()) {
			logger.debug("stopSession()");
		}

		if (session != null) {
			try {
				session.removeSessionListener(sessionEventListener);
				session.cancelSession();
				releaseCamera();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	
	
	/**
	 * Add video to Audio session
	 */
	private void addVideo() {
		if (logger.isActivated()) {
			logger.debug("addVideo()");
		}

		Thread thread = new Thread() {
			public void run() {

				// wait api connection & video surface creation to instantiate
				// video player
				while (!isSurfaceCreated) {
					if (logger.isActivated()) {
						logger.info("wait isSurfaceCreated");
					}
					synchronized(this){
						try {
							wait(500);
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}
					
					if (logger.isActivated()) {
						logger.info("sortie wait");
					}
				}
				if (logger.isActivated()) {
					logger.info("call api connected and isSurfaceCreated - instantiate video Renderer + instantiate video Player ");
				}
				instantiateIncomingVideoRenderer();			
				instantiateOutgoingVideoPlayer();

				if (session != null) {
					try {
						session.addVideo(
								outgoingVideoPlayer, incomingVideoRenderer);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}
		};
		thread.start();
	}

	/**
	 * remove video from session.
	 */
	private void removeVideo() {
		if (logger.isActivated()) {
			logger.debug("removeVideo()");
		}
		
		if (session != null) {
			releaseCamera();			
			try {
				session.removeVideo();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	/**
	 * get Add Video Invitation and display Dialog Box
	 */
	private void getAddVideoInvitation() {
		if (logger.isActivated()) {
			logger.debug("getAddVideoInvitation()");
		}

		AlertDialog.Builder builder = new AlertDialog.Builder(
				IPCallView.this).setTitle("Add Video Invitation");
		if (logger.isActivated()) {
			logger.debug("IPCallSessionActivity.this :"
					+ IPCallView.this);
		}

		// Add the buttons
		builder.setPositiveButton(R.string.label_accept,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int id) {
						outgoingVideoView.setVisibility(View.VISIBLE);
						incomingVideoView.setVisibility(View.VISIBLE);
						switchCamBtn.setVisibility(View.VISIBLE);	
						switchCamBtn.setEnabled(false);

						
						acceptAddVideo();
						addVideoBtn.setText(R.string.label_remove_video_btn);
						addVideoBtn.setEnabled(true);
					}
				});
		builder.setNegativeButton(R.string.label_decline,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int id) {
						rejectAddVideo();
						addVideoBtn.setText(R.string.label_add_video_btn);
						addVideoBtn.setEnabled(true);
					}
				});

		addVideoInvitationDialog = builder.create();
		addVideoInvitationDialog.setCancelable(false);
		addVideoInvitationDialog.show();

	}
	
	/**
	 * Accept Add Video Invitation 
	 */
	private void acceptAddVideo() {
		if (logger.isActivated()) {
			logger.info("acceptAddVideo()");
		}
		Thread thread = new Thread() {
			public void run() {
				// wait api connection & video surface creation to instantiate
				// video player
				while (!isSurfaceCreated) {
					if (logger.isActivated()) {
						logger.info("wait isSurfaceCreated");
					}
					synchronized(this){
						try {
							wait(500);
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}
					
					if (logger.isActivated()) {
						logger.info("sortie wait");
					}
				}
				if (logger.isActivated()) {
					logger.info("isSurfaceCreated - instantiate video Renderer + instantiate video Player ");
				}
				instantiateIncomingVideoRenderer();
				instantiateOutgoingVideoPlayer();

				try {
					session
							.setVideoPlayer(outgoingVideoPlayer);
					session
							.setVideoRenderer(incomingVideoRenderer);
					session.acceptAddVideo();
					videoConnected = true;
					
					
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		};
		thread.start();

	}
	
	/**
	 * Reject Add Video Invitation 
	 */
	private void rejectAddVideo() {
		if (logger.isActivated()) {
			logger.info("rejectAddVideo()");
		}
		try {
			session.rejectAddVideo();
			videoConnected = false;
			
		} catch (RemoteException e) {
			e.printStackTrace();
		}		
	}
	
	/**
     * Runnable to Receive incoming session
     */
    private void instantiateIncomingVideoRenderer() {
        	if (logger.isActivated()){
                logger.error("instantiateIncomingVideoRenderer");
                logger.info("isCallApiConnected : "+IPCallSessionsData.getInstance().isCallApiConnected);
            }

                try {
                    if (VideoSettings.isCodecsManagedByStack(getApplicationContext())) {
                        incomingVideoRenderer = new VideoRenderer();
                    } else {
                        incomingVideoRenderer = new VideoRenderer(createSupportedCodecList(VideoSettings.getCodecsList(getApplicationContext())));
                    }
                    //TODO Fix Error
			// incomingVideoRenderer.setVideoSurface(incomingVideoView);
 
                } catch (Exception e) {
                	exitActivity(getString(R.string.label_videorenderer_start_failed)
        					+ " - error:" + e.getMessage());
                }
    }
	
    /**
     * Runnable to start outgoing session
     */
    private void  instantiateOutgoingVideoPlayer() {
        	if (logger.isActivated()){
                logger.error("instantiateOutgoingVideoPlayer");
                logger.info("isCallApiConnected : "+IPCallSessionsData.getInstance().isCallApiConnected);
                logger.info("isSurfaceCreated : "+isSurfaceCreated);
            }

                if (VideoSettings.isCodecsManagedByStack(getApplicationContext())) {
                    outgoingVideoPlayer = new LiveVideoPlayer(incomingVideoRenderer);
                    if (logger.isActivated()){ logger.info("Codecs Managed By Stack");}
                } else {
                    outgoingVideoPlayer = new LiveVideoPlayer(createSupportedCodecList(VideoSettings.getCodecsList(getApplicationContext())), incomingVideoRenderer);
                }
         
                // Start camera
                startCamera();
    };

    /**
     * Recreate the video player
     */
    private void recreateVideoPlayer(){
        // Create the live video player
        if (VideoSettings.isCodecsManagedByStack(getApplicationContext())) {
            outgoingVideoPlayer = new LiveVideoPlayer(incomingVideoRenderer);
        } else {
            outgoingVideoPlayer = new LiveVideoPlayer(createSupportedCodecList(VideoSettings.getCodecsList(getApplicationContext())), incomingVideoRenderer);
        }
        if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE){
            outgoingVideoView.setAspectRatio(outgoingWidth, outgoingHeight);
        } else {
            outgoingVideoView.setAspectRatio(outgoingHeight, outgoingWidth);
        }
        surface = outgoingVideoView.getHolder();
        surface.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        surface.addCallback(this);
        if (camera != null) {
            camera.setPreviewCallback(outgoingVideoPlayer);
        }
    }
    
    /**
     * Create a list of supported video codecs
     *
     * @return codecs list
     */
    private MediaCodec[] createSupportedCodecList(boolean[] codecs) {
        // Set number of codecs
        int size = 0;
        for (int i = 0; i < VideoSettings.CODECS_SIZE + 1; i++) {
            size += ((codecs[i]) ? 1 : 0);
        }
        if (size == 0) {
            return null;
        }

        // Add codecs settings (preferred in first)
        MediaCodec[] supportedMediaCodecs = new MediaCodec[size];
        for (int i = 0; i < VideoSettings.CODECS_SIZE; i++) {
            if (codecs[i]) {
                supportedMediaCodecs[--size] = VideoSettings.CODECS[i];
            }
        }
        if (codecs[VideoSettings.CODECS_SIZE]) {
            supportedMediaCodecs[--size] = VideoSettings.getCustomCodec(getApplicationContext());
        }
        return supportedMediaCodecs;
    }
    
	/**
	 * Exit activity if all sessions are stopped.
	 * 
	 * @param message
	 *            the message to display. Can be null for no message.
	 */
	private void exitActivity(String message) {
		if (logger.isActivated()) {
			logger.info("exitActivity()");
		}

		hideProgressDialog();
		videoConnected = false;
		//IPCallSessionsData.getInstance().removeSessionData(sessionId);
		session = null;
		sessionId = null;

		if (message == null) {
			this.finish();
		} else {
			Utils.showMessageAndExit(IPCallView.this, message);
		}
	}

	/**
	 * Display progress dialog
	 */
	private void displayProgressDialog() {
		if (logger.isActivated()) {
			logger.debug("displayProgressDialog()");
			logger.debug("outgoingProgressDialog= "+outgoingProgressDialog);
		}
		// Display a progress dialog
		outgoingProgressDialog = Utils.showProgressDialog(
				IPCallView.this,
				getString(R.string.label_command_in_progress));
		outgoingProgressDialog.setCancelable(false);
		outgoingProgressDialog.setOnKeyListener(new Dialog.OnKeyListener(){

			@Override
            public boolean onKey(DialogInterface arg0, int keyCode,
                    KeyEvent event) {
				if (logger.isActivated()) {
					logger.debug("onKey()");
				}
                // TODO Auto-generated method stub
                if (keyCode == KeyEvent.KEYCODE_BACK) {
                	
                if (session != null){stopSession();}
                else {cancelOutgoingSessionWhenCreated = true; }
                exitActivity(null);
                }
                return true;
            }
			
		});
	}

	/**
	 * Hide progress dialog
	 */
	private void hideProgressDialog() {
		if (logger.isActivated()) {
			logger.debug("hideProgressDialog()");
			logger.debug("outgoingProgressDialog= "+outgoingProgressDialog);
		}
		if (outgoingProgressDialog != null
				&& outgoingProgressDialog.isShowing()) {
			outgoingProgressDialog.dismiss();
			outgoingProgressDialog = null;
		}
	}

	

    /* *****************************************
     *                Camera
     ***************************************** */

    /**
     * Get Camera "open" Method
     *
     * @return Method
     */
    private Method getCameraOpenMethod() {
        ClassLoader classLoader = VisioSharing.class.getClassLoader();
        Class cameraClass = null;
        try {
            cameraClass = classLoader.loadClass("android.hardware.Camera");
            try {
                return cameraClass.getMethod("open", new Class[] {
                    int.class
                });
            } catch (NoSuchMethodException e) {
                return null;
            }
        } catch (ClassNotFoundException e) {
            return null;
        }
    }

    /**
     * Get Camera "numberOfCameras" Method
     *
     * @return Method
     */
    private Method getCameraNumberOfCamerasMethod() {
        ClassLoader classLoader = VisioSharing.class.getClassLoader();
        Class cameraClass = null;
        try {
            cameraClass = classLoader.loadClass("android.hardware.Camera");
            try {
                return cameraClass.getMethod("getNumberOfCameras", (Class[])null);
            } catch (NoSuchMethodException e) {
                return null;
            }
        } catch (ClassNotFoundException e) {
            return null;
        }
    }

    /**
     * Get number of cameras
     *
     * @return number of cameras
     */
    private int getNumberOfCameras() {
        Method method = getCameraNumberOfCamerasMethod();
        if (method != null) {
            try {
                Integer ret = (Integer)method.invoke(null, (Object[])null);
                return ret.intValue();
            } catch (Exception e) {
                return 1;
            }
        } else {
            return 1;
        }
    }

    /**
     * Open a camera
     *
     * @param cameraId
     */
    private void OpenCamera(CameraOptions cameraId) {
        Method method = getCameraOpenMethod();
        if (numberOfCameras > 1 && method != null) {
            try {
                camera = (Camera)method.invoke(camera, new Object[] {
                    cameraId.getValue()
                });
                openedCameraId = cameraId;
            } catch (Exception e) {
                camera = Camera.open();
                openedCameraId = CameraOptions.BACK;
            }
        } else {
            camera = Camera.open();
            openedCameraId = CameraOptions.BACK;
        }
        if (outgoingVideoPlayer != null) {
            outgoingVideoPlayer.setCameraId(openedCameraId.getValue());
        }
    }

    /**
     * Check if good camera sizes are available for encoder.
     * Must be used only before open camera.
     * 
     * @param cameraId
     * @return false if the camera don't have the good preview size for the encoder
     */
    private boolean checkCameraSize(CameraOptions cameraId) {
        boolean sizeAvailable = false;

        // Open the camera
        OpenCamera(cameraId);

        // Check common sizes
        Parameters param = camera.getParameters();
        List<Camera.Size> sizes = param.getSupportedPreviewSizes();
        for (Camera.Size size:sizes) {
            if (    (size.width == H264Config.QVGA_WIDTH && size.height == H264Config.QVGA_HEIGHT) ||
                    (size.width == H264Config.CIF_WIDTH && size.height == H264Config.CIF_HEIGHT) ||
                    (size.width == H264Config.VGA_WIDTH && size.height == H264Config.VGA_HEIGHT)) {
                sizeAvailable = true;
                break;
            }
        }

        // Release camera
        releaseCamera();

        return sizeAvailable;
    }

    /**
     * Start the camera preview
     */
    @SuppressLint("NewApi")
	private void startCameraPreview() {
		if (logger.isActivated()) {
            logger.debug("startCameraPreview()");  
            logger.debug("openedCameraId ="+openedCameraId);
            
        }
		
        if (camera != null) {
            // Camera settings
            Camera.Parameters p = camera.getParameters();
            p.setPreviewFormat(PixelFormat.YCbCr_420_SP); //ImageFormat.NV21);

            // Orientation
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.FROYO) {
                Display display = ((WindowManager)getSystemService(WINDOW_SERVICE)).getDefaultDisplay();
                switch (display.getRotation()) {
                    case Surface.ROTATION_0:
                        if (logger.isActivated()) {
                            logger.debug("ROTATION_0");
                        }
                        if (openedCameraId == CameraOptions.FRONT) {
                            outgoingVideoPlayer.setOrientation(Orientation.ROTATE_90_CCW);
                        } else {
                            outgoingVideoPlayer.setOrientation(Orientation.ROTATE_90_CW);
                        }
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.FROYO) {
                            camera.setDisplayOrientation(90);
                        } else {
                            p.setRotation(90);
                        }
                        break;
                    case Surface.ROTATION_90:
                        if (logger.isActivated()) {
                            logger.debug("ROTATION_90");
                        }
                        outgoingVideoPlayer.setOrientation(Orientation.NONE);
                        break;
                    case Surface.ROTATION_180:
                        if (logger.isActivated()) {
                            logger.debug("ROTATION_180");
                        }
                        if (openedCameraId == CameraOptions.FRONT) {
                            outgoingVideoPlayer.setOrientation(Orientation.ROTATE_90_CW);
                        } else {
                            outgoingVideoPlayer.setOrientation(Orientation.ROTATE_90_CCW);
                        }
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.FROYO) {
                            camera.setDisplayOrientation(270);
                        } else {
                            p.setRotation(270);
                        }
                        break;
                    case Surface.ROTATION_270:
                        if (logger.isActivated()) {
                            logger.debug("ROTATION_270");
                        }
                        if (openedCameraId == CameraOptions.FRONT) {
                            outgoingVideoPlayer.setOrientation(Orientation.ROTATE_180);
                        } else {
                            outgoingVideoPlayer.setOrientation(Orientation.ROTATE_180);
                        }
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.FROYO) {
                            camera.setDisplayOrientation(180);
                        } else {
                            p.setRotation(180);
                        }
                        break;
                }
            } else {
                // getRotation not managed under Froyo
                outgoingVideoPlayer.setOrientation(Orientation.NONE);
            }

            // Camera size
            List<Camera.Size> sizes = p.getSupportedPreviewSizes();
            if (sizeContains(sizes, outgoingWidth, outgoingHeight)) {
                // Use the existing size without resizing
                p.setPreviewSize(outgoingWidth, outgoingHeight);
                outgoingVideoPlayer.activateResizing(outgoingWidth, outgoingHeight); // same size = no resizing
                if (logger.isActivated()) {
                	logger.info("Use the existing size without resizing");
                    logger.info("Camera preview initialized with size " + outgoingWidth + "x" + outgoingHeight);
                }
            } else {
                // Check if can use a other known size (QVGA, CIF or VGA)
                int w = 0;
                int h = 0;
                for (Camera.Size size:sizes) {
                    w = size.width;
                    h = size.height;
                    if (    (w == H264Config.QVGA_WIDTH && h == H264Config.QVGA_HEIGHT) ||
                            (w == H264Config.CIF_WIDTH && h == H264Config.CIF_HEIGHT) ||
                            (w == H264Config.VGA_WIDTH && h == H264Config.VGA_HEIGHT)) {
                        break;
                    }
                }

                if (w != 0) {
                    p.setPreviewSize(w, h);
                    outgoingVideoPlayer.activateResizing(w, h);
                    if (logger.isActivated()) {
                        logger.info("Camera preview initialized with size " + w + "x" + h + " with a resizing to " + outgoingWidth + "x" + outgoingHeight);
                        logger.info("outGoingVideoView size ="+outgoingVideoView.getWidth()+"x"+outgoingVideoView.getHeight());
                    }
                } else {
                    // The camera don't have known size, we can't use it
                    if (logger.isActivated()) {
                        logger.warn("Camera preview can't be initialized with size " + outgoingWidth + "x" + outgoingHeight);
                    }
                    camera = null;
                    return;
                }
            }
         
            camera.setParameters(p);
            try {
                camera.setPreviewDisplay(surface);
                camera.startPreview();
                cameraPreviewRunning = true;
            } catch (Exception e) {
                camera = null;
            }
        }
    }

    /**
     * Release the camera
     */
    private void releaseCamera() {
        if (camera != null) {
            camera.setPreviewCallback(null);
            if (cameraPreviewRunning) {
                cameraPreviewRunning = false;
                camera.stopPreview();
            }
            camera.release();
            camera = null;
        }
    }

    /**
     * Test if size is in list.
     * Can't use List.contains because it doesn't work with some devices.
     *
     * @param list
     * @param width
     * @param height
     * @return boolean
     */
    private boolean sizeContains(List<Camera.Size> list, int width, int height) {
        for (int i = 0; i < list.size(); i++) {
            if (list.get(i).width == width && list.get(i).height == height) {
                return true;
            }
        }
        return false;
    }

    /**
     * Start the camera
     */
    private void startCamera() {
    	if (logger.isActivated()) {
            logger.error("startCamera()");
        }
        if (camera == null) {
            // Open camera
            OpenCamera(openedCameraId);
            if (logger.isActivated()) {
                logger.error("camera is null - openedCameraId ="+openedCameraId);
            }
            if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE){
                outgoingVideoView.setAspectRatio(outgoingWidth, outgoingHeight);
                if (logger.isActivated()) {
                    logger.info("Landscape mode - "+outgoingWidth+"x"+outgoingHeight);
                }
            } else {
                outgoingVideoView.setAspectRatio(outgoingHeight, outgoingWidth);
                if (logger.isActivated()) {
                    logger.info("Portrait mode - outGoingWidth ="+outgoingWidth+"- outGoingHeith ="+outgoingHeight);
                    logger.info("outGoingVideoView size ="+outgoingVideoView.getWidth()+"x"+outgoingVideoView.getHeight());
                }
            }
            // Start camera
            camera.setPreviewCallback(outgoingVideoPlayer);
            startCameraPreview();
        } else {
            if (logger.isActivated()) {
                logger.error("Camera is not null");
            }
        }
    }

    /**
     * ReStart the camera
     */
    private void reStartCamera() {
        if (camera != null) {
            releaseCamera();
        }
        startCamera();
    }

    /* *****************************************
     *          SurfaceHolder.Callback
     ***************************************** */

    @Override
    public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {
    	if (logger.isActivated()) {
			logger.info("surfaceChanged()");
			logger.info("width ="+arg2);
			logger.info("height ="+arg3);
			
		}
        isSurfaceCreated = true;
        
    }

    @Override
    public void surfaceCreated(SurfaceHolder arg0) {
    	if (logger.isActivated()) {
			logger.info("surfaceCreated()");
		}
        isSurfaceCreated = true;
        
        if (startOutgoingSessionWhenSurfaceCreated){
        	startOutgoingSession();
        	startOutgoingSessionWhenSurfaceCreated = false ;
        }
        
        if (acceptIncomingSessionWhenSurfaceCreated) {
        	acceptIncomingSession();
        	acceptIncomingSessionWhenSurfaceCreated = false;
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder arg0) {
    	if (logger.isActivated()) {
			logger.info("surfaceDestroyed()");
		}
        isSurfaceCreated = false;
    }
    
    
    /* *****************************************
    * Video session methods
    ***************************************** */
    
    public void prepareVideoSession(){
    	if (logger.isActivated()) {
			logger.info("prepareVideoSession()");
		}
    	
    	numberOfCameras = getNumberOfCameras();
    	if (logger.isActivated()) {
			logger.info("number of cameras ="+numberOfCameras);
		}
        if (numberOfCameras > 1) {
            boolean backAvailable = checkCameraSize(CameraOptions.BACK);
            boolean frontAvailable = checkCameraSize(CameraOptions.FRONT);
            if (frontAvailable && backAvailable) {
                switchCamBtn.setOnClickListener(btnSwitchCamListener);
            } else if (frontAvailable) {
                openedCameraId = CameraOptions.FRONT;
                switchCamBtn.setVisibility(View.INVISIBLE);
            } else if (backAvailable) {
                openedCameraId = CameraOptions.BACK;
                switchCamBtn.setVisibility(View.INVISIBLE);
            } else {
                // TODO: Error - no camera available for encoding
            }
        } else {
            if (checkCameraSize(CameraOptions.FRONT)) {
                switchCamBtn.setVisibility(View.INVISIBLE);
            } else {
                // TODO: Error - no camera available for encoding
            }
        }
        
        // Set incoming video view
        incomingVideoView = (VideoSurfaceView)findViewById(R.id.incoming_ipcall_video_view);
        if (incomingWidth != 0 && incomingHeight != 0) {
            incomingVideoView.setAspectRatio(incomingWidth, incomingHeight);
        }
        
        // set the outgoing video preview 
        if (logger.isActivated()) {logger.info("set outgoing video preview");}
        outgoingVideoView = (VideoSurfaceView)findViewById(R.id.outgoing_ipcall_video_preview);
        if (outgoingWidth == 0 || outgoingHeight == 0) {
            if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE){
                outgoingVideoView.setAspectRatio(H264Config.QCIF_WIDTH, H264Config.QCIF_HEIGHT);
            } else {
                outgoingVideoView.setAspectRatio(H264Config.QCIF_HEIGHT, H264Config.QCIF_WIDTH);
            }
        } else {
            if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE){
                outgoingVideoView.setAspectRatio(outgoingWidth, outgoingHeight);
            } else {
                outgoingVideoView.setAspectRatio(outgoingHeight, outgoingWidth);
                if (logger.isActivated()) {
        			logger.info("Portrait mode - height ="+outgoingHeight+" - width ="+outgoingWidth);
        			logger.info("outGoingVideoView size ="+outgoingVideoView.getWidth()+"x"+outgoingVideoView.getHeight());
        		}
            }
        }
        surface = outgoingVideoView.getHolder();
        surface.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        surface.setKeepScreenOn(true);
        surface.addCallback(this);
        
    }
}
