package com.orangelabs.rcs.service.api.server.ipcall;

import android.os.RemoteCallbackList;

import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ipcall.IPCallError;
import com.orangelabs.rcs.core.ims.service.ipcall.IPCallStreamingSession;
import com.orangelabs.rcs.core.ims.service.ipcall.IPCallStreamingSessionListener;
import com.orangelabs.rcs.core.ims.service.ipcall.OriginatingIPCallStreamingSession;
import com.orangelabs.rcs.provider.ipcall.IPCall;
import com.orangelabs.rcs.provider.ipcall.IPCallData;
import com.orangelabs.rcs.service.api.client.SessionDirection;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.ipcall.IIPCallEventListener;
import com.orangelabs.rcs.service.api.client.ipcall.IIPCallSession;
import com.orangelabs.rcs.service.api.client.media.IAudioPlayer;
import com.orangelabs.rcs.service.api.client.media.IAudioRenderer;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.server.ServerApiUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * IP call session
 * 
 * * @author owom5460
 */
public class IPCallSession extends IIPCallSession.Stub implements IPCallStreamingSessionListener{ 

	/**
	 * Core session
	 */
	private IPCallStreamingSession session;

	/**
	 * List of listeners
	 */
	private RemoteCallbackList<IIPCallEventListener> listeners = new RemoteCallbackList<IIPCallEventListener>();

	/**
	 * Lock used for synchronisation
	 */
	private Object lock = new Object();

	/**
	 * The logger
	 */
	private Logger logger = Logger.getLogger(this.getClass().getName());

	/**
	 * Constructor
	 * 
	 * @param session Session
	 */
	public IPCallSession(IPCallStreamingSession session) {
		this.session = session;
		session.addListener(this);
	}

	/**
	 * Get session ID
	 * 
	 * @return Session ID
	 */
	public String getSessionID() {
		return session.getSessionID();
	}

	/**
	 * Get remote contact
	 * 
	 * @return Contact
	 */
	public String getRemoteContact() {
		return session.getRemoteContact();
	}

	/**
	 * Get session state
	 * 
	 * @return State
	 * @see SessionState
	 */
	public int getSessionState() {
		return ServerApiUtils.getSessionState(session);
	}

	/**
	 * Get session direction
	 * 
	 * @return Direction
	 * @see SessionDirection
	 */
	public int getSessionDirection() {
		if (session instanceof OriginatingIPCallStreamingSession) {
			return SessionDirection.OUTGOING;
		} else {
			return SessionDirection.INCOMING;
		}
	}	
	
	/**
	 * Accept the session invitation
	 * 
	 * @param video Activate video
	 */
	public void acceptSession(boolean video) {
		if (logger.isActivated()) {
			logger.info("Accept session invitation");
		}

		if (video == false) {
			session.setVideoPlayer(null);
			session.setVideoRenderer(null);
			session.setVideoContent(null);
			
		}

		// Accept invitation
		session.acceptSession();
	}

	/**
	 * Reject the session invitation
	 */
	public void rejectSession() {
		if (logger.isActivated()) {
			logger.info("Reject session invitation");
		}

		// Update IP call history
		IPCall.getInstance().setStatus(session.getSessionID(),
				IPCallData.STATUS_CANCELED); 

		// Reject invitation
		session.rejectSession(603);
	}

	/**
	 * Cancel the session
	 */
	public void cancelSession() {
		if (logger.isActivated()) {
			logger.info("Cancel session");
		}

		// Abort the session
		session.abortSession(ImsServiceSession.TERMINATION_BY_USER);
	}

	/**
	 * Add video to the session
	 * 
	 * @param videoPlayer Video player
	 * @param videoRenderer Video renderer
	 */
	public void addVideo(IVideoPlayer videoPlayer, IVideoRenderer videoRenderer) {
		if (logger.isActivated()) {
			logger.info("Add video");
		}

		// Add video to session
		session.addVideo(videoPlayer, videoRenderer);		
	}

	/**
	 * Remove video from the session
	 */
	public void removeVideo() {
		if (logger.isActivated()) {
			logger.info("Remove video");
		}

		// Remove video from session
		session.removeVideo();		
	}

	/**
	 * Accept invitation to add video
	 */
	public void acceptAddVideo() {
		if (logger.isActivated()) {
			logger.info("Accept invitation to add video");
			
		}
		
		// Accept to add video
		session.getUpdateSessionManager().acceptReInvite();
	}

	/**
	 * Reject invitation to add video
	 */
	public void rejectAddVideo() {
		if (logger.isActivated()) {
			logger.info("Reject invitation to add video");
		}
		//set video content to null
		session.setVideoContent(null);
		
		// Reject add video
		session.getUpdateSessionManager().rejectReInvite(603);
	}

	/**
	 * Set call hold
	 * 
	 * @param state State
	 */
	public void setCallHold(boolean flag) {
		if (logger.isActivated()) {
			logger.info("Set call hold to " + flag);
		}

		session.setOnHold(flag);
	}

	/**
	 * Set the video renderer
	 * 
	 * @param renderer Video Renderer
	 */
	public void setVideoRenderer(IVideoRenderer renderer) {
		if (logger.isActivated()) {
			logger.info("Set a video renderer");
		}

		session.setVideoRenderer(renderer);
	}

	/**
	 * Get the video renderer
	 * 
	 * @return Video renderer
	 */
	public IVideoRenderer getVideoRenderer() {
		if (logger.isActivated()) {
			logger.info("Get video renderer");
		}
		
		return session.getVideoRenderer();
	}

	/**
	 * Set the video player
	 * 
	 * @param player Video player
	 */
	public void setVideoPlayer(IVideoPlayer player) {
		if (logger.isActivated()) {
			logger.info("Set video player");
		}

		session.setVideoPlayer(player);
	}

	/**
	 * Get the video player
	 * 
	 * @return Video player
	 */
	public IVideoPlayer getVideoPlayer() {
		if (logger.isActivated()) {
			logger.info("Get video player");
		}
		
		return session.getVideoPlayer();
	}

	/**
	 * Set the audio renderer
	 * 
	 * @param renderer Audio renderer
	 */
	public void setAudioRenderer(IAudioRenderer renderer) {
		if (logger.isActivated()) {
			logger.info("Set audio renderer");
		}
		
		session.setAudioRenderer(renderer);
	}

	/**
	 * Get the audio renderer
	 * 
	 * @return Audio renderer
	 */
	public IAudioRenderer getAudioRenderer() {
		if (logger.isActivated()) {
			logger.info("Get audio renderer");
		}
		
		return session.getAudioRenderer();
	}

	/**
	 * Set the audio player
	 * 
	 * @param player Audio pPlayer
	 */
	public void setAudioPlayer(IAudioPlayer player) {
		if (logger.isActivated()) {
			logger.info("Set audio player");
		}

		session.setAudioPlayer(player);
	}

	/**
	 * Get the audio player
	 * 
	 * @return Audio player
	 */
	public IAudioPlayer getAudioPlayer() {
		if (logger.isActivated()) {
			logger.info("Get audio player");
		}
		
		return session.getAudioPlayer();
	}

	/**
	 * Add session listener
	 * 
	 * @param listener Listener
	 */
	public void addSessionListener(IIPCallEventListener listener) {
		if (logger.isActivated()) {
			logger.info("Add an event listener");
		}

		synchronized (lock) {
			listeners.register(listener);
		}
	}

	/**
	 * Remove session listener
	 * 
	 * @param listener Listener
	 */
	public void removeSessionListener(IIPCallEventListener listener) {
		if (logger.isActivated()) {
			logger.info("Remove an event listener");
		}

		synchronized (lock) {
			listeners.unregister(listener);
		}
	}

	/**
	 * Session is started
	 */
	public void handleSessionStarted() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Session started");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleSessionStarted();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}

	/**
	 * Session has been aborted
	 * 
	 * @param reason Termination reason
	 */
	public void handleSessionAborted(int reason) {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Session aborted (reason " + reason + ")");
			}

			// Update IP call history
			IPCall.getInstance().setStatus(session.getSessionID(),
					IPCallData.STATUS_CANCELED); 

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleSessionAborted(reason);
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();

			// Remove session from the list
			IPCallApiService.removeIPCallSession(session.getSessionID());
		}
	}

	/**
	 * Session has been terminated by remote
	 */
	public void handleSessionTerminatedByRemote() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Session terminated by remote");
			}

			// Update IP call history
			IPCall.getInstance().setStatus(session.getSessionID(),
					IPCallData.STATUS_TERMINATED); 

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i)
							.handleSessionTerminatedByRemote();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();

			// Remove session from the list
			IPCallApiService.removeIPCallSession(session.getSessionID());
		}
	}

	/**
	 * Add video invitation
	 * 
	 * @param videoEncoding Video encoding
     * @param width Video width
     * @param height Video height
	 */
	public void handleAddVideoInvitation(String videoEncoding, int videoWidth, int videoHeight) {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Add video invitation");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleAddVideoInvitation(
							videoEncoding, videoWidth, videoHeight);
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}
	
	/**
	 * Remove video invitation
	 */
	public void handleRemoveVideo() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Remove video invitation");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleRemoveVideo();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}

	/**
	 * Add video has been accepted by user 
	 */
	public void handleAddVideoAccepted() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Add video accepted");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleAddVideoAccepted();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}

	/**
	 * Remove video has been accepted by user 
	 */
	public void handleRemoveVideoAccepted() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Remove video accepted");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleRemoveVideoAccepted();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}
	
	/**
	 * Add video has been aborted
	 * 
	 * @param reason Termination reason
	 */
	public void handleAddVideoAborted(int reason) {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Add video aborted (reason " + reason + ")");
			}

	        // Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleAddVideoAborted(reason);
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}
	
	/**
	 * Remove video has been aborted
	 * 
	 * @param reason Termination reason
	 */
	public void handleRemoveVideoAborted(int reason) {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Remove video aborted (reason " + reason + ")");
			}

	        // Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleRemoveVideoAborted(reason);
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}		
	}
	
	/**
	 * Call Hold invitation
	 * 
	 */
	public void handleCallHold() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Call Hold invitation");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleCallHold();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}

	/**
	 * Call Resume invitation
	 * 
	 */
	public void handleCallResume() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Call Resume invitation");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleCallResume();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}

	/**
	 * Call Hold has been accepted
	 * 
	 */
	public void handleCallHoldAccepted() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Call Hold accepted");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleCallHoldAccepted();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}

	/**
	 * Call Hold has been aborted
	 * 
	 * @param reason Termination reason
	 */
	public void handleCallHoldAborted(int errorCode) {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Call Hold aborted (reason " + errorCode + ")");
			}

	        // Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleCallHoldAborted(errorCode);
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}		
	}

	/**
	 * Call Resume has been accepted
	 * 
	 */
	public void handleCallResumeAccepted() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Call Resume accepted");
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleCallResumeAccepted();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}

	/**
	 * Call Resume has been aborted
	 * 
	 * @param reason Termination reason
	 */
	public void handleCallResumeAborted(int code) {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Call Resume aborted (reason " + code + ")");
			}

	        // Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleCallResumeAborted(code);
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}		
	}


	/**
	 * IP Call error
	 * 
	 * @param error Error
	 */
	public void handleCallError(IPCallError error) {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Session error");
			}

			// Update IP call history
			IPCall.getInstance().setStatus(session.getSessionID(),
					IPCallData.STATUS_FAILED); 

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleCallError(
							error.getErrorCode());
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();

			// Remove session from the list
			IPCallApiService.removeIPCallSession(session.getSessionID());
		}
	}

	/**
	 * Called user is Busy
	 */
	public void handle486Busy() {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("486 Busy");
			}

			// Update IP call history
			IPCall.getInstance().setStatus(session.getSessionID(),
					IPCallData.STATUS_FAILED);

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handle486Busy();
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();

			// Remove session from the list
			IPCallApiService.removeIPCallSession(session.getSessionID());
		}
	}

    /**
     * Video stream has been resized
     *
     * @param width Video width
     * @param height Video height
     */
	public void handleVideoResized(int width, int height) {
		synchronized (lock) {
			if (logger.isActivated()) {
				logger.info("Video resized to " + width + "x" + height);
			}

			// Notify event listeners
			final int N = listeners.beginBroadcast();
			for (int i = 0; i < N; i++) {
				try {
					listeners.getBroadcastItem(i).handleVideoResized(width,
							height);
				} catch (Exception e) {
					if (logger.isActivated()) {
						logger.error("Can't notify listener", e);
					}
				}
			}
			listeners.finishBroadcast();
		}
	}

	

}

