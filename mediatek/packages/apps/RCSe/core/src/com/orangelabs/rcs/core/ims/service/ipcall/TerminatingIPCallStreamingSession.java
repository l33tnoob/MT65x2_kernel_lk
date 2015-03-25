/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.service.ipcall;

import java.util.Vector;

import android.os.RemoteException;

import com.orangelabs.rcs.core.content.ContentManager;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.SessionTimerManager;
import com.orangelabs.rcs.core.ims.service.richcall.video.SdpOrientationExtension;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoCodecManager;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoSdpBuilder;
import com.orangelabs.rcs.service.api.client.media.audio.AudioCodec;
import com.orangelabs.rcs.service.api.client.media.video.VideoCodec;
import com.orangelabs.rcs.utils.CodecsUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Terminating IP call session
 *
 * @author opob7414
 */
public class TerminatingIPCallStreamingSession extends IPCallStreamingSession {
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     *
     * @param parent IMS service
     * @param invite Initial INVITE request
     */
    public TerminatingIPCallStreamingSession(ImsService parent, SipRequest invite) {
        super(parent, SipUtils.getAssertedIdentity(invite),
        		ContentManager.createLiveAudioContentFromSdp(invite.getContentBytes()),
        		ContentManager.createLiveVideoContentFromSdp(invite.getContentBytes()));

        // Create dialog path
        createTerminatingDialogPath(invite);
    }

    /**
     * Background processing
     */
	public void run() {
		try {
			if (logger.isActivated()) {
				logger.info("Initiate a new IP call session as terminating");
			}

			// Send a 180 Ringing response
			send180Ringing(getDialogPath().getInvite(), getDialogPath()
					.getLocalTag());

			// Notify listener
			getImsService().getImsModule().getCore().getListener()
					.handleIPCallInvitation(this);

			// Wait invitation answer
			int answer = waitInvitationAnswer();
			if (answer == ImsServiceSession.INVITATION_REJECTED) {
				if (logger.isActivated()) {
					logger.debug("Session has been rejected by user");
				}

				// Remove the current session
				getImsService().removeSession(this);

				// Notify listeners
				for (int i = 0; i < getListeners().size(); i++) {
					getListeners().get(i).handleSessionAborted(
							ImsServiceSession.TERMINATION_BY_USER);
				}
				return;
			} else if (answer == ImsServiceSession.INVITATION_NOT_ANSWERED) {
				if (logger.isActivated()) {
					logger.debug("Session has been rejected on timeout");
				}

				// Ringing period timeout
				send603Decline(getDialogPath().getInvite(), getDialogPath()
						.getLocalTag());

				// Remove the current session
				getImsService().removeSession(this);

				// Notify listeners
				for (int i = 0; i < getListeners().size(); i++) {
					getListeners().get(i).handleSessionAborted(
							ImsServiceSession.TERMINATION_BY_TIMEOUT);
				}
				return;
			} else if (answer == ImsServiceSession.INVITATION_CANCELED) {
				if (logger.isActivated()) {
					logger.debug("Session has been canceled");
				}
				return;
			}

			// Check if an audio renderer has been set
			if (getAudioRenderer() == null) {
				if (logger.isActivated()) {
					logger.debug("Audio renderer not initialized");
				}
				handleError(new IPCallError(
						IPCallError.AUDIO_RENDERER_NOT_INITIALIZED));
				return;
			}

			// Check if an audio player has been set
			if (getAudioPlayer() == null) {
				if (logger.isActivated()) {
					logger.debug("Audio player not initialized");
				}
				handleError(new IPCallError(
						IPCallError.AUDIO_PLAYER_NOT_INITIALIZED));
				return;
			}

			// user has accepted the call invitation
			String sdp = buildCallInitSdpResponse();
			if (logger.isActivated()) {
				logger.info("buildCallInitSdpResponse() - Done");
			}

			// Set the local SDP in the dialog path
			getDialogPath().setLocalContent(sdp);

			 // prepare media session			
			prepareMediaSession();
			
			// Create a 200 OK response
			SipResponse resp = null;
			if (getAudioPlayer() != null) {
				if (getVideoPlayer() != null) {
					// audio+video IP Call
					resp = SipMessageFactory.create200OkInviteResponse(
							getDialogPath(),
							IPCallService.FEATURE_TAGS_IP_VIDEO_CALL, sdp);
				} else {
					// audio IP Call
					resp = SipMessageFactory.create200OkInviteResponse(
							getDialogPath(),
							IPCallService.FEATURE_TAGS_IP_VOICE_CALL, sdp);
				}
			} else {
				handleError(new IPCallError(IPCallError.UNEXPECTED_EXCEPTION,
						"Audio player not initialized"));
			}

			// The signalisation is established
			getDialogPath().sigEstablished();

			if (logger.isActivated()) {
				logger.info("Send 200 OK");
			}
			// Send response
			SipTransactionContext ctx = getImsService().getImsModule()
					.getSipManager().sendSipMessageAndWait(resp);

			// Analyze the received response
			if (ctx.isSipAck()) {
				// ACK received
				if (logger.isActivated()) {
					logger.info("ACK request received");
				}

				// The session is established
				getDialogPath().sessionEstablished();

				// startmediaSession
				startMediaSession();
				

				// Start session timer
				if (getSessionTimerManager().isSessionTimerActivated(resp)) {
					getSessionTimerManager().start(
							SessionTimerManager.UAS_ROLE,
							getDialogPath().getSessionExpireTime());
				}

				// Notify listeners
				for (int i = 0; i < getListeners().size(); i++) {
					getListeners().get(i).handleSessionStarted();
				}
			} else {
				if (logger.isActivated()) {
					logger.debug("No ACK received for INVITE");
				}

				// No response received: timeout
				handleError(new IPCallError(
						IPCallError.SESSION_INITIATION_FAILED));
			}
		} catch (Exception e) {
			if (logger.isActivated()) {
				logger.error("Session initiation has failed", e);
			}

			// Unexpected error
			handleError(new IPCallError(IPCallError.UNEXPECTED_EXCEPTION,
					e.getMessage()));
		}
	}

    /**
     * Handle error
     *
     * @param error Error
     */
    public void handleError(IPCallError error) {
        // Error
        if (logger.isActivated()) {
            logger.info("Session error: " + error.getErrorCode() + ", reason=" + error.getMessage());
        }

        // Close media (audio, video) session
        closeMediaSession();

        // Remove the current session
        getImsService().removeSession(this);
        
        // Notify listener
        if (!isInterrupted()) {
            for(int i=0; i < getListeners().size(); i++) {
                ((IPCallStreamingSessionListener)getListeners().get(i)).handleCallError(error);
            }
        }
    }  
    

	/**
	 * Build sdp response for call init
	 * 
	 * @param reInvite  reInvite Request received
	 */
	private String buildCallInitSdpResponse() {
		if (logger.isActivated()) {
			logger.debug("Build SDP proposal for call init");
		}
		
		// Parse the remote SDP part
        SdpParser parser = new SdpParser(getDialogPath().getRemoteContent().getBytes());
        
        // Extract the video description
        MediaDescription mediaVideo = parser.getMediaDescription("video");


        // Extract the audio codecs from SDP
        Vector<MediaDescription> audio = parser.getMediaDescriptions("audio");
        Vector<AudioCodec> proposedAudioCodecs = AudioCodecManager.extractAudioCodecsFromSdp(audio);

        // Extract video codecs from SDP            
        Vector<MediaDescription> video = parser.getMediaDescriptions("video");
        Vector<VideoCodec> proposedVideoCodecs = null;
        if (mediaVideo != null) {
        	proposedVideoCodecs = VideoCodecManager.extractVideoCodecsFromSdp(video);
        }
        
        
//        // Audio codec negotiation
		try {
			selectedAudioCodec = AudioCodecManager.negociateAudioCodec(getAudioRenderer().getSupportedAudioCodecs(), proposedAudioCodecs);
			if (selectedAudioCodec == null) {
				if (logger.isActivated()) {
					logger.debug("Proposed audio codecs are not supported");
				}

				// Send a 415 Unsupported media type response
				send415Error(getDialogPath().getInvite());

				// Unsupported media type
				handleError(new IPCallError(IPCallError.UNSUPPORTED_AUDIO_TYPE));
				return null;
			}
			
	        // Video codec negotiation
			if ((mediaVideo != null) && (getVideoPlayer() != null)) {
				selectedVideoCodec = VideoCodecManager.negociateVideoCodec(getVideoPlayer().getSupportedVideoCodecs(), proposedVideoCodecs);
				if (selectedVideoCodec == null) {
					// Support of video codec of profile 1B is compulsory. Even if not proposed explicitly it shall be selected.
					selectedVideoCodec = CodecsUtils.getVideoCodecProfile1b(getVideoRenderer().getSupportedVideoCodecs());
					if (selectedVideoCodec != null) {
						if (logger.isActivated())
							logger.info("Video codec profile 1B is selected by default");
					} else {
						if (logger.isActivated())
							logger.debug("Proposed video codecs are not supported");

						// Send a 415 Unsupported media type response
						send415Error(getDialogPath().getInvite());

						// Report error
						handleError(new IPCallError(IPCallError.UNSUPPORTED_VIDEO_TYPE));
						return null;
					}
				}
			}
			  	
	    	String audioSdp = AudioSdpBuilder.buildSdpAnswer(selectedAudioCodec.getMediaCodec(),
	    			getAudioPlayer().getLocalRtpPort());
	        
	    	String videoSdp = "";
	        if ((getVideoContent() != null) && (getVideoRenderer() != null)){
	        	if (selectedVideoCodec != null) {
	            	videoSdp = VideoSdpBuilder.buildSdpAnswer(selectedVideoCodec.getMediaCodec(),
	            			getVideoRenderer().getLocalRtpPort(), mediaVideo);
	            }	
	        }
	        
	     // Build audioSdp and videoSdp part
	        String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
	    	String ipAddress = getDialogPath().getSipStack().getLocalIpAddress();

	        // Build SDP for response
	        String sdp =
	        	"v=0" + SipUtils.CRLF +
	        	"o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
	        	"s=-" + SipUtils.CRLF +
	        	"c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
	            "t=0 0" + SipUtils.CRLF +
	            audioSdp +
	            videoSdp +
	            "a=sendrcv" + SipUtils.CRLF;
	        
			return sdp;
	        
		} catch (RemoteException e) {
			if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }
            
            // Unexpected error
            handleError(new IPCallError(IPCallError.UNEXPECTED_EXCEPTION,
                    e.getMessage()));
            
            return null;
		}
	}
	
    

    /**
     * Prepare media session
     * 
     * @throws Exception 
     */
	public void prepareMediaSession() throws Exception {
	
		// Parse the remote SDP part
		SdpParser remoteParser = new SdpParser(getDialogPath()
				.getRemoteContent().getBytes());

		// Extract the remote host (same between audio and video)
		String remoteHost = SdpUtils
				.extractRemoteHost(remoteParser.sessionDescription.connectionInfo);

		// Extract the audio port
		MediaDescription mediaAudio = remoteParser.getMediaDescription("audio");
		int audioRemotePort = mediaAudio.port;

		// Extract the video port
		MediaDescription mediaVideo = remoteParser.getMediaDescription("video");
		// int videoRemotePort = mediaVideo.port;
		int videoRemotePort = -1;
		if (mediaVideo != null) {
			videoRemotePort = mediaVideo.port;
		}

		if (logger.isActivated()) {
			logger.info("Extract Audio/Video ports - Done");
		}


		// Set the audio codec and listener in Audio Renderer
		getAudioRenderer().setAudioCodec(selectedAudioCodec.getMediaCodec());
		getAudioRenderer().addListener(new AudioPlayerEventListener(this));
		if (logger.isActivated()) {
			logger.debug("Set audio codec in the audio renderer: "
					+ selectedAudioCodec.getMediaCodec().getCodecName());
		}

		// Set the audio codec and listener in Audio Player
		getAudioPlayer().setAudioCodec(selectedAudioCodec.getMediaCodec());
		getAudioPlayer().addListener(new AudioPlayerEventListener(this));
		if (logger.isActivated()) {
			logger.debug("Set audio codec in the audio player: "
					+ selectedAudioCodec.getMediaCodec().getCodecName());
		}

		// // Open the audio renderer
		// getAudioRenderer().open(remoteHost, audioRemotePort);
		// if (logger.isActivated()) {
		// logger.debug("Open audio renderer with remoteHost ("+remoteHost+") and remotePort ("+audioRemotePort+")");
		// }
		//
		// // Open the audio player
		// getAudioPlayer().open(remoteHost, audioRemotePort);
		// if (logger.isActivated()) {
		// logger.debug("Open audio player on renderer RTP stream");
		// }

		// Set the video codec in Video Player/Renderer
		if ((getVideoRenderer() != null) && (getVideoPlayer() != null)
				&& (selectedVideoCodec != null)) {
			getVideoRenderer()
					.setVideoCodec(selectedVideoCodec.getMediaCodec());
			getVideoPlayer().setVideoCodec(selectedVideoCodec.getMediaCodec());
			if (logger.isActivated()) {
				logger.debug("Set video codec in the video renderer: "
						+ selectedVideoCodec.getMediaCodec().getCodecName());
				logger.debug("Set video codec in the video player: "
						+ selectedVideoCodec.getMediaCodec().getCodecName());
			}
		}

		// Set the listeners on video player and renderer
		if ((getVideoRenderer() != null) && (getVideoPlayer() != null)) {
			getVideoRenderer().addListener(new VideoPlayerEventListener(this));
			getVideoPlayer().addListener(new VideoPlayerEventListener(this));
		}

		// Set the OrientationHeaderID in renderer and player
		if (mediaVideo != null) {
			SdpOrientationExtension extensionHeader = SdpOrientationExtension
					.create(mediaVideo);
			if ((getVideoRenderer() != null) && (getVideoPlayer() != null)
					&& (extensionHeader != null)) {
				getVideoRenderer().setOrientationHeaderId(
						extensionHeader.getExtensionId());
				getVideoPlayer().setOrientationHeaderId(
						extensionHeader.getExtensionId());
			}
		}

		// Open the Video Renderer and Player
		// always open the player after the renderer when the RTP stream is shared
		if ((getVideoRenderer() != null) && (getVideoPlayer() != null)) {
			getVideoRenderer().open(remoteHost, videoRemotePort);
			getVideoPlayer().open(remoteHost, videoRemotePort); 	
			if (logger.isActivated()) {
				logger.debug("Open video renderer with remoteHost ("
						+ remoteHost + ") and remotePort (" + videoRemotePort
						+ ")");
				logger.debug("Open video player on renderer RTP stream");
			}
		}

	}

    /**
     * Start media session
     * 
     * @throws Exception 
     */
    public void startMediaSession() throws Exception {
        // Already done in run() method
    	
    	// Start the audio renderer
		// getAudioRenderer().start();

		// Start the audio player
		// getAudioPlayer().start();


		// Start the video renderer and video player
		if ((getVideoPlayer() != null) && (getVideoRenderer() != null)) {
			getVideoPlayer().start();
			if (logger.isActivated()) {
				logger.debug("Start video player");
			}
			getVideoRenderer().start();
			if (logger.isActivated()) {
				logger.debug("Start video renderer");
			}
		}
    }
}

