package com.orangelabs.rcs.core.ims.service.ipcall;
import android.os.RemoteException;

import com.orangelabs.rcs.core.content.LiveAudioContent;
import com.orangelabs.rcs.core.content.LiveVideoContent;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceError;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.service.api.client.media.IAudioEventListener;
import com.orangelabs.rcs.service.api.client.media.IAudioPlayer;
import com.orangelabs.rcs.service.api.client.media.IAudioRenderer;
import com.orangelabs.rcs.service.api.client.media.IVideoEventListener;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.audio.AudioCodec;
import com.orangelabs.rcs.service.api.client.media.video.VideoCodec;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * IP call session
 * 
 * @author opob7414
 */
public abstract class IPCallStreamingSession extends ImsServiceSession {

	/**
	 * Constant values for session update request type
	 */
	public final static int ADD_VIDEO = 0;
	public final static int REMOVE_VIDEO = 1;
	public final static int SET_ON_HOLD = 2;
	public final static int SET_ON_RESUME = 3;

	/**
	 * Live video content to be shared
	 */
	private LiveVideoContent videoContent = null;

	/**
	 * Live audio content to be shared
	 */
	private LiveAudioContent audioContent = null;

	/**
	 * Audio renderer
	 */
	private IAudioRenderer audioRenderer = null;

	/**
	 * Audio player
	 */
	private IAudioPlayer audioPlayer = null;

	/**
	 * Video renderer
	 */
	private IVideoRenderer videoRenderer = null;

	/**
	 * Video player
	 */
	private IVideoPlayer videoPlayer = null;
	
	/**
	 * Call Hold Manager
	 */
	private HoldManager holdMgr;
	
	/**
	 * Add Video Manager
	 */
	private AddVideoManager addVideoMgr;
	
	/**
	 * Selected Audio Codec
	 */
	protected AudioCodec selectedAudioCodec = null;
	
	
	/**
	 * Selected Video Codec
	 */
	protected VideoCodec selectedVideoCodec = null;

	/**
	 * The logger
	 */
	private Logger logger = Logger.getLogger(this.getClass().getName());
	
	/**
	 * Constructor
	 * 
	 * @param parent IMS service
	 * @param contact Remote contact
	 * @param audioContent AudioContent
	 * @param videoContent VideoContent
	 */
	public IPCallStreamingSession(ImsService imsService, String contact, LiveAudioContent audioContent, LiveVideoContent videoContent) {
		super(imsService, contact);

		this.videoContent = videoContent;
		this.audioContent = audioContent;
		HoldManager.setState(HoldManager.IDLE) ;
		AddVideoManager.setState(AddVideoManager.IDLE);
	}

	/**
	 * Returns the video content
	 * 
	 * @return Live video content
	 */
	public LiveVideoContent getVideoContent() {
		return videoContent;
	}

	/**
	 * Set the video content
	 * 
	 * @param videoContent Live video content
	 */
	public void setVideoContent(LiveVideoContent videoContent) {
		this.videoContent = videoContent;
	}

	/**
	 * Returns the audio content
	 * 
	 * @return Live audio content
	 */
	public LiveAudioContent getAudioContent() {
		return audioContent;
	}

	/**
	 * Set the audio content
	 * 
	 * @param audioContent Live audio content
	 */
	public void setAudioContent(LiveAudioContent audioContent) {
		this.audioContent = audioContent;
	}

	/**
	 * Get the audio renderer
	 * 
	 * @return Audio renderer
	 */
	public IAudioRenderer getAudioRenderer() {
		return audioRenderer;
	}

	/**
	 * Set the audio renderer
	 * 
	 * @param audioRenderer Audio renderer
	 */
	public void setAudioRenderer(IAudioRenderer audioRenderer) {
		this.audioRenderer = audioRenderer;
	}

	/**
	 * Get the audio player
	 * 
	 * @return Audio player
	 */
	public IAudioPlayer getAudioPlayer() {
		return audioPlayer;
	}

	/**
	 * Set the audio player
	 * 
	 * @param audioPlayer Audio player
	 */
	public void setAudioPlayer(IAudioPlayer audioPlayer) {
		this.audioPlayer = audioPlayer;
	}

	/**
	 * Get the video renderer
	 * 
	 * @return Video renderer
	 */
	public IVideoRenderer getVideoRenderer() {
		return videoRenderer;
	}

	/**
	 * Set the video renderer
	 * 
	 * @param videoRenderer Video renderer
	 */
	public void setVideoRenderer(IVideoRenderer videoRenderer) {
		this.videoRenderer = videoRenderer;
	}

	/**
	 * Get the video player
	 * 
	 * @return Video player
	 */
	public IVideoPlayer getVideoPlayer() {
		return videoPlayer;
	}

	/**
	 * Set the video player
	 * 
	 * @param videoPlayer Video player
	 */
	public void setVideoPlayer(IVideoPlayer videoPlayer) {
		this.videoPlayer = videoPlayer;
	}

	/**
	 * Receive BYE request
	 * 
	 * @param bye BYE request
	 */
	public void receiveBye(SipRequest bye) {
		super.receiveBye(bye);

		// Request capabilities to the remote
		getImsService().getImsModule().getCapabilityService()
				.requestContactCapabilities(getDialogPath().getRemoteParty());
	}

	/**
	 * Create an INVITE request
	 * 
	 * @return the INVITE request
	 * @throws SipException
	 */
	public SipRequest createInvite() throws SipException {
		//return SipMessageFactory.createInvite(getDialogPath(), null,
				//getDialogPath().getLocalContent());
		
		if (getVideoContent() == null) {
        	// Voice call
        	return SipMessageFactory.createInvite(getDialogPath(), IPCallService.FEATURE_TAGS_IP_VOICE_CALL, getDialogPath().getLocalContent());
        } else {
        	// Video call
        	return SipMessageFactory.createInvite(getDialogPath(), IPCallService.FEATURE_TAGS_IP_VIDEO_CALL, getDialogPath().getLocalContent());
        } 
	}
	
	
	/**
	 * Receive re-INVITE request
	 * 
	 * @param reInvite
	 *            re-INVITE received request
	 */
	public void receiveReInvite(SipRequest reInvite) {
		if (logger.isActivated()) {
			logger.info("receiveReInvite");
		}

		if (reInvite.getSdpContent() == null) {
			// "Keep Alive" ReInvite
			getSessionTimerManager().receiveReInvite(reInvite);
		} else {// "SessionUpdate" ReInvite		
			String content = reInvite.getSdpContent();
			int requestType = -1;
			
			// Analyze sdp to dispatch according to sdp content
			if (isTagPresent(content, "a=sendonly")){ // Set On Hold "SendOnly"
					if ((AddVideoManager.getState()==AddVideoManager.IDLE)&&(HoldManager.getState()==HoldManager.IDLE)) {
						requestType = 3;
					}	
			} else if (isTagPresent(content, "a=inactive")) {// Set On Hold "Inactive"
					if ((AddVideoManager.getState()==AddVideoManager.IDLE)&&(HoldManager.getState()==HoldManager.IDLE)) {
						requestType = 2;
					}
			} else if  (isTagPresent(content, "a=sendrcv")&&(getVideoContent()==null)&&(isTagPresent(content, "m=video"))){
				if ((AddVideoManager.getState()==AddVideoManager.IDLE)&&(HoldManager.getState()==HoldManager.IDLE)) {
					requestType = 0; // Add Video	
				} 
			} else if  (isTagPresent(content, "a=sendrcv")&&(getVideoContent()!=null)&&(!isTagPresent(content, "m=video"))){
				 if ((AddVideoManager.getState()==AddVideoManager.IDLE)&&((HoldManager.getState()==HoldManager.IDLE)||(HoldManager.getState()==HoldManager.HOLD)||(HoldManager.getState()==HoldManager.REMOTE_HOLD))) {
					requestType = 1;// Remove Video
				}
			} else if (isTagPresent(content, "a=sendrcv")) {				
				if ((AddVideoManager.getState()==AddVideoManager.IDLE)&&(HoldManager.getState() == HoldManager.REMOTE_HOLD)){
					requestType = 5;// Set on Resume
				}			
			} else {
				// send error to remote client
				sendErrorResponse(reInvite, getDialogPath().getLocalTag(), 603);
			}
			
			 if (requestType != -1) {
				 // set received sdp proposal as remote sdp content in dialogPath
				 getDialogPath().setRemoteContent(content);
			 }
					

			switch (requestType) {
			case (0): { // Case Add Video

				// instantiate Manager and requests addVideo
				addVideoMgr = new RemoteAddVideo(this);
				LiveVideoContent videocontent = addVideoMgr.addVideo(reInvite);

				// get video Encoding , video Width and video Height
				String videoEncoding = (videocontent == null) ? ""
						: videocontent.getEncoding();
				int videoWidth = (videocontent == null) ? 0 : videocontent
						.getWidth();
				int videoHeight = (videocontent == null) ? 0 : videocontent
						.getHeight();

				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleAddVideoInvitation(videoEncoding,
										videoWidth, videoHeight);
					}
				}
			}
				break;
			case (1): { // Case Remove Video
				// instantiate Manager and requests removeVideo
				addVideoMgr = new RemoteAddVideo(this);				
				addVideoMgr.removeVideo(reInvite);

				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleRemoveVideo();
					}
				}

			}
				break;
			case (2): { // Case Set "Inactive" On Hold
				// instantiate Hold Manager
				holdMgr = new RemoteHoldInactive(this);

				// launch callHold
				holdMgr.setCallHold(true, reInvite);

				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleCallHold();
					}
				}
			}
				break;
			case (5): { // Case Set On Resume
				// instantiate Hold Manager
				holdMgr = new RemoteHoldInactive(this);

				// launch callHold
				holdMgr.setCallHold(false, reInvite);

				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleCallResume();
					}
				}
			}
				break;
			}
		}
	}

	
	/**
	 * Add video in the current call
	 * 
	 * @param videoPlayer Video player
	 * @param videoRenderer Video renderer
	 */
	public void addVideo(IVideoPlayer videoPlayer, IVideoRenderer videoRenderer) {
		if (logger.isActivated()) {
			logger.info("Add video");
		}

		if ((getVideoContent()== null)&&(AddVideoManager.getState() == AddVideoManager.IDLE)&&(HoldManager.getState() == HoldManager.IDLE)) {			
			addVideoMgr = new LocalAddVideo(this);

			// launch addVideo
			addVideoMgr.addVideo(videoPlayer, videoRenderer);
		} else {
			if (!isInterrupted()) {
				for (int i = 0; i < getListeners().size(); i++) {
					((IPCallStreamingSessionListener) getListeners().get(i))
							.handleAddVideoAborted(IPCallError.INVALID_COMMAND);
				}
			}
		}
	}
		

	
	/**
	 * Remove video from the current call
	 */
	public void removeVideo() {
		if (logger.isActivated()) {
			logger.info("Remove video");
		}
		
		if ((getVideoContent()!= null)&&(AddVideoManager.getState()== AddVideoManager.IDLE)&&((HoldManager.getState() == HoldManager.IDLE)||(HoldManager.getState() == HoldManager.HOLD)||(HoldManager.getState() == HoldManager.REMOTE_HOLD))){
			// instantiate Add Video Manager and requests add video
			addVideoMgr = new LocalAddVideo(this);

			// launch removeVideo
			addVideoMgr.removeVideo();
		}
		else {
			if (!isInterrupted()) {
				for (int i = 0; i < getListeners().size(); i++) {
					((IPCallStreamingSessionListener) getListeners().get(i))
							.handleRemoveVideoAborted(IPCallError.INVALID_COMMAND);
				}
			}
		}
	}

	
	/**
	 * Set On Hold/on Resume the current call
	 * 
	 * @param callHoldAction boolean defining call hold action 
	 */
	public void setOnHold(boolean callHoldAction){
		if (logger.isActivated()) {
			logger.info("setOnHold");
		}
		
		if (((callHoldAction)&&(AddVideoManager.getState()==AddVideoManager.IDLE)&&(HoldManager.getState()==HoldManager.IDLE))
				||((!callHoldAction)&& (AddVideoManager.getState() == AddVideoManager.IDLE)&&(HoldManager.getState() == HoldManager.HOLD))){
			// instanciate Hold Manager
			holdMgr = new LocalHoldInactive(this);
			
			// launch callHold/callResume (depending on boolean value)
			holdMgr.setCallHold(callHoldAction);	
		}
		else {
			if (!isInterrupted()) {
				for (int i = 0; i < getListeners().size(); i++) {
					if (callHoldAction){
						((IPCallStreamingSessionListener) getListeners().get(i))
						.handleCallHoldAborted(IPCallError.INVALID_COMMAND);
					}
					else {
						((IPCallStreamingSessionListener) getListeners().get(i))
						.handleCallResumeAborted(IPCallError.INVALID_COMMAND);
					}					
				}
			}
		}
	}
	
	
	/**
	 * Handle Sip Response to ReInvite / originating side 
	 * 
	 * @param int code response code
	 * @param response
	 *            Sip response to sent ReInvite
	 * @param requestType
	 *            Type type of request (addVideo/RemoveVideo/Set on Hold/Set on
	 *            Resume)
	 */
	public void handleReInviteResponse(int code, SipResponse response,
			int requestType) {
		if (logger.isActivated()) {
			logger.info("handleReInviteResponse: " + code);
		}

		// case Add video
		if (requestType == IPCallStreamingSession.ADD_VIDEO) {
			if (code == 200) { // 200 OK response
				//prepare video session
				addVideoMgr.prepareVideoSession();
				
				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleAddVideoAccepted();
					}
				}
				//start video session
				addVideoMgr.startVideoSession();
				
			} else if ((code == ImsServiceSession.INVITATION_REJECTED)
					|| (code == ImsServiceSession.TERMINATION_BY_TIMEOUT)) {	

				//reset add video manager - set video content to null
				AddVideoManager.setState(AddVideoManager.IDLE);
				setVideoContent(null);	
				
				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleAddVideoAborted(code);
					}
				}	
			}
			
			// release add video manager
			addVideoMgr = null;
			
		// case Remove Video
		} else if (requestType == IPCallStreamingSession.REMOVE_VIDEO) {
			if (code == 200) { // 200 OK response
				// close video media session
				addVideoMgr.closeVideoSession();				

				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleRemoveVideoAccepted();
					}
				}
			} else if (code == ImsServiceSession.TERMINATION_BY_TIMEOUT) { // No answer or 
																		//408 TimeOut response
				//reset add video manager state to "idle"
				AddVideoManager.setState(AddVideoManager.IDLE);
				
				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleRemoveVideoAborted(code);
					}
				}
			}
			// release add video manager
			addVideoMgr = null;
			
		// case Set On Hold
		} else if (requestType == IPCallStreamingSession.SET_ON_HOLD) {
			if (code == 200) { // 200 OK response
				// prepare media session
				holdMgr.holdMediaSession();

				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleCallHoldAccepted();
					}
				}
			} else if (code == ImsServiceSession.TERMINATION_BY_TIMEOUT) { //No answer or
																		//408 TimeOut response
				//reset hold manager state to "idle"
				HoldManager.setState(HoldManager.IDLE);
				
				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleCallHoldAborted(code);
					}
				}
				
			}
			// release hold manager
			holdMgr = null;
			
		//case Set On Resume
		} else if (requestType == IPCallStreamingSession.SET_ON_RESUME) {
			if (code == 200) { // 200 OK response
				// prepare media session
				holdMgr.resumeMediaSession();
				
				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleCallResumeAccepted();
					}
				}
			} else if (code == ImsServiceSession.TERMINATION_BY_TIMEOUT) { // No answer or
																		//408 TimeOut response
				//reset hold manager state to "Hold"
				HoldManager.setState(HoldManager.HOLD);
				
				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleCallResumeAborted(code);
					}
				}
			}
			
			// release hold manager
			holdMgr = null;
		}
	}
	

	/**
	 * Handle User Response to ReInvite/ terminating side
	 * 
	 * @param int code response code
	 * @param requestType
	 *            Type type of request (addVideo/RemoveVideo/Set on Hold/Set on
	 *            Resume)
	 */
	public void handleReInviteUserAnswer(int code, int requestType) {
		if (logger.isActivated()) {
			logger.info("handleReInviteUserAnswer: " + code);
		}

		// case Add video
		if (requestType == IPCallStreamingSession.ADD_VIDEO) {// Invitation accepted			
			if (code == ImsServiceSession.INVITATION_ACCEPTED) {
				// prepare Video media session
				addVideoMgr.prepareVideoSession();
	
			} else if ((code == ImsServiceSession.INVITATION_NOT_ANSWERED)||(code == ImsServiceSession.INVITATION_REJECTED)) {// Invitation declined or not answered
				//reset add video manager - set video content to null
				AddVideoManager.setState(AddVideoManager.IDLE);
				setVideoContent(null);				
				
				// Notify listeners
				if (!isInterrupted()) {
					for (int i = 0; i < getListeners().size(); i++) {
						((IPCallStreamingSessionListener) getListeners().get(i))
								.handleAddVideoAborted(code);
					}
				}
			}
		}
	}
	
	
	/**
	 * Handle Sip Response to ReInvite ACK / terminating side
	 * 
	 * @param int code response code
	 * @param requestType
	 *            Type type of request (addVideo/RemoveVideo/Set on Hold/Set on
	 *            Resume)
	 */
	public void handleReInviteAck(int code, int requestType) {
		if (logger.isActivated()) {
			logger.info("handleReInviteAckResponse: " + code);
		}

		// case Add video
		if ((requestType == IPCallStreamingSession.ADD_VIDEO) && (code == 200)) {
			// start Video media session
			addVideoMgr.startVideoSession();

			// Notify listeners
			if (!isInterrupted()) {
				for (int i = 0; i < getListeners().size(); i++) {
					((IPCallStreamingSessionListener) getListeners().get(i))
							.handleAddVideoAccepted();
				}
			}
			// release add video manager
			addVideoMgr  = null;
						
		// case Remove Video	
		} else if ((requestType == IPCallStreamingSession.REMOVE_VIDEO)&& (code == 200)) {						
			// close video media session
			addVideoMgr.closeVideoSession();

			// Notify listeners
			if (!isInterrupted()) {
				for (int i = 0; i < getListeners().size(); i++) {
					((IPCallStreamingSessionListener) getListeners().get(i))
							.handleRemoveVideoAccepted();
				}
			}
			// release add video manager
			addVideoMgr  = null;
			
		// case Set On Hold
		} else if ((requestType == IPCallStreamingSession.SET_ON_HOLD)&& (code == 200)) {						
			//prepare media session
			holdMgr.holdMediaSession();
			
			// Notify listeners
			if (!isInterrupted()) {
				for (int i = 0; i < getListeners().size(); i++) {
					((IPCallStreamingSessionListener) getListeners().get(i))
							.handleCallHoldAccepted();
				}
			}
			// release hold manager
			holdMgr = null;
			
		// case Set On Resume	
		} else if ((requestType == IPCallStreamingSession.SET_ON_RESUME)
				&& (code == 200)) {// case On Resume
			// prepare media session
			holdMgr.resumeMediaSession();
			
			// Notify listeners
			if (!isInterrupted()) {
				for (int i = 0; i < getListeners().size(); i++) {
					((IPCallStreamingSessionListener) getListeners().get(i))
							.handleCallResumeAccepted();
				}
			}
			
			// release hold manager
			holdMgr = null;
		}
	}
	
	
	/**
	 * Handle 486 Busy
	 * 
	 * @param resp SipResponse
	 *            
	 */
	public void handle486Busy(SipResponse resp) {
		if (logger.isActivated()) {
			logger.info("486 Busy");
		}

		// Close audio and video session
		closeMediaSession();

		// Remove the current session
		getImsService().removeSession(this);

		// Request capabilities to the remote
		getImsService().getImsModule().getCapabilityService()
				.requestContactCapabilities(getDialogPath().getRemoteParty());

		// Notify listeners
		if (!isInterrupted()) {
			for (int i = 0; i < getListeners().size(); i++) {
				((IPCallStreamingSessionListener) getListeners().get(i))
						.handle486Busy();
			}
		}
	}


	/**
	 * Handle 407 Proxy authent error
	 * 
	 * @param resp SipResponse
	 * @param requestType type of request (addVideo/RemoveVideo/Set on Hold/Set on
	 *            Resume)            
	 */
	public void handleReInvite407ProxyAuthent(SipResponse response,
			int requestType) {

		// // Set the remote tag
		getDialogPath().setRemoteTag(response.getToTag());

		// Update the authentication agent
		getAuthenticationAgent().readProxyAuthenticateHeader(response);

		// get sdp content
		String content = getDialogPath().getLocalContent();

		SipRequest reInvite = null;
		// create reInvite request
		if (requestType == IPCallStreamingSession.ADD_VIDEO) {
			reInvite = getUpdateSessionManager().createReInvite(
					IPCallService.FEATURE_TAGS_IP_VIDEO_CALL, content);
		} else if (requestType == IPCallStreamingSession.REMOVE_VIDEO) {
			reInvite = getUpdateSessionManager().createReInvite(
					IPCallService.FEATURE_TAGS_IP_VOICE_CALL, content);
		} else {
			// TODO for set On Hold
			reInvite = getUpdateSessionManager().createReInvite(
					IPCallService.FEATURE_TAGS_IP_VIDEO_CALL, content);
		}

		// send reInvite request
		getUpdateSessionManager().sendReInvite(reInvite, requestType);
	}

	
	/**
	 * Handle error
	 * 
	 * @param error Error
	 */
	public void handleError(ImsServiceError error) {
		if (logger.isActivated()) {
			logger.info("Session error: " + error.getErrorCode() + ", reason="
					+ error.getMessage());
		}

		// Close Audio and Video session
		closeMediaSession();

		// Remove the current session
		getImsService().removeSession(this);

		// Request capabilities to the remote
		getImsService().getImsModule().getCapabilityService()
				.requestContactCapabilities(getDialogPath().getRemoteParty());

		// Notify listeners
		if (!isInterrupted()) {
			for (int i = 0; i < getListeners().size(); i++) {
				((IPCallStreamingSessionListener) getListeners().get(i))
						.handleCallError(new IPCallError(error));
			}
		}
	}
	
	/**
	 * Is tag present in SDP
	 * 
	 * @param sdp SDP
	 * @param tag Tag to be searched
	 * @return Boolean
	 */
	public boolean isTagPresent(String sdp, String tag) {
		if ((sdp != null) && (sdp.toLowerCase().indexOf(tag) != -1)) {
			return true;
		} else {
			return false;
		}
	}
	
	
	/**
	 * select sdp builder method (to build sdp response) according to serviceContext - build and return sdp
	 * 
	 * @param reInvite
	 *            reInvite received request
	 * @param serviceContext
	 *            context of service (Add Video, Remove Video ...)
	 * @return sdp 
	 */
	public String buildReInviteSdpResponse(SipRequest reInvite, int serviceContext) {
		String localSdp = "";
		if (serviceContext == IPCallStreamingSession.ADD_VIDEO) {
				localSdp = ((RemoteAddVideo) addVideoMgr).buildAddVideoSdpResponse(reInvite);
		}

		return localSdp;
	}

	
	
	//******************************************************************************
	//******************************************************************************
	//******************      Media Session Management Methods      ****************
	//******************************************************************************
	//******************************************************************************

	/**
     * Close media session
     */
	public void closeMediaSession() {
		if (logger.isActivated()) {
			logger.info("Close media session");
		}

//		if (audioRenderer != null) {
//			// Close the audio renderer
//			try {
//				audioRenderer.stop();
//				audioRenderer.close();
//				if (logger.isActivated()) {
//					logger.info("Stop and Close the audio renderer");
//				}
//			} catch (RemoteException e) {
//				if (logger.isActivated()) {
//					logger.error("Exception when closing the audio renderer", e);
//				}
//			}
//		}
//		if (audioPlayer != null) {
//			// Close the audio player
//			try {
//				audioPlayer.stop();
//				audioPlayer.close();
//				if (logger.isActivated()) {
//					logger.info("Stop and Close the audio player");
//				}
//			} catch (RemoteException e) {
//				if (logger.isActivated()) {
//					logger.error("Exception when closing the audio player", e);
//				}
//			}
//		}
		
		if (videoRenderer != null) {
			// Close the video renderer
			try {
				videoRenderer.stop();
				videoRenderer.close();
				if (logger.isActivated()) {
					logger.info("Stop and close video renderer");
				}
			} catch (RemoteException e) {
				if (logger.isActivated()) {
					logger.error("Exception when closing the video renderer", e);
				}
			}
		}
		if (videoPlayer != null) {
			// Close the video player
			try {
				videoPlayer.stop();
				videoPlayer.close();
				if (logger.isActivated()) {
					logger.info("stop and close video player");
				}
			} catch (Exception e) {
				if (logger.isActivated()) {
					logger.error("Exception when closing the video player", e);
				}
			}
		}
		setAudioPlayer(null);
		setAudioRenderer(null);
		setVideoPlayer(null);
		setVideoRenderer(null);
	}
	

	//******************************************************************************
	//******************************************************************************
	//*************************       Media Listeners       ************************
	//******************************************************************************
	//******************************************************************************
	
	/**
	 * Audio player event listener
	 */
	protected class AudioPlayerEventListener extends IAudioEventListener.Stub {
		/**
		 * Streaming session
		 */
		private IPCallStreamingSession session;

		/**
		 * Constructor
		 * 
		 * @param session
		 *            Streaming session
		 */
		public AudioPlayerEventListener(IPCallStreamingSession session) {
			this.session = session;
		}

		/**
		 * Audio player is opened
		 */
		public void audioOpened() {
			if (logger.isActivated()) {
				logger.debug("Audio player is opened");
			}
		}

		/**
		 * Audio player is closed
		 */
		public void audioClosed() {
			if (logger.isActivated()) {
				logger.debug("Audio player is closed");
			}
		}

		/**
		 * Audio player is started
		 */
		public void audioStarted() {
			if (logger.isActivated()) {
				logger.debug("Audio player is started");
			}
		}

		/**
		 * Audio player is stopped
		 */
		public void audioStopped() {
			if (logger.isActivated()) {
				logger.debug("Audio player is stopped");
			}
		}

		/**
		 * Audio player has failed
		 * 
		 * @param error Error
		 */
		public void audioError(String error) {
			if (logger.isActivated()) {
				logger.error("Audio player has failed: " + error);
			}

			// Close the media (audio, video) session
			closeMediaSession();

			// Terminate session
			terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);

			// Remove the current session
			getImsService().removeSession(session);

			// Notify listeners
			if (!isInterrupted()) {
				for (int i = 0; i < getListeners().size(); i++) {
					((IPCallStreamingSessionListener) getListeners().get(i))
							.handleCallError(new IPCallError(
									IPCallError.AUDIO_STREAMING_FAILED,
									error));
				}
			}

			// Request capabilities to the remote
			getImsService()
					.getImsModule()
					.getCapabilityService()
					.requestContactCapabilities(
							getDialogPath().getRemoteParty());
		}
	}
	
	
    /**
     * Video player event listener
     */
    protected class VideoPlayerEventListener extends IVideoEventListener.Stub {
        /**
         * Streaming session
         */
        private IPCallStreamingSession session;

        /**
         * Constructor
         *
         * @param session Streaming session
         */
        public VideoPlayerEventListener(IPCallStreamingSession session) {
            this.session = session;
        }

        /**
         * Media player is opened
         */
        public void mediaOpened() {
            if (logger.isActivated()) {
                logger.debug("Media renderer is opened");
            }
        }

        /**
         * Video stream has been resized
         *
         * @param width Video width
         * @param height Video height
         */
        public void mediaResized(int width, int height) {
            if (logger.isActivated()) {
                logger.debug("The size of media has changed " + width + "x" + height);
            }
            // Notify listeners
            if (!isInterrupted()) {
                for (int i = 0; i < getListeners().size(); i++) {
                    ((IPCallStreamingSessionListener) getListeners().get(i))
                            .handleVideoResized(width, height);
                }
            }
        }

        /**
         * Media player is closed
         */
        public void mediaClosed() {
            if (logger.isActivated()) {
                logger.debug("Media renderer is closed");
            }
        }

        /**
         * Media player is started
         */
        public void mediaStarted() {
            if (logger.isActivated()) {
                logger.debug("Media renderer is started");
            }
        }

        /**
         * Media player is stopped
         */
        public void mediaStopped() {
            if (logger.isActivated()) {
                logger.debug("Media renderer is stopped");
            }
        }

        /**
         * Media player has failed
         *
         * @param error Error
         */
        public void mediaError(String error) {
            if (logger.isActivated()) {
                logger.error("Media renderer has failed: " + error);
            }

            // Close the audio and video session
            closeMediaSession();

            // Terminate session
            terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);

            // Remove the current session
            getImsService().removeSession(session);

            // Notify listeners
            if (!isInterrupted()) {
                for(int i=0; i < getListeners().size(); i++) {
                    ((IPCallStreamingSessionListener)getListeners().get(i)).handleCallError(new IPCallError(IPCallError.VIDEO_STREAMING_FAILED, error));
                }
            }

            // Request capabilities to the remote
            getImsService().getImsModule().getCapabilityService().requestContactCapabilities(getDialogPath().getRemoteParty());
        }
    }
}
