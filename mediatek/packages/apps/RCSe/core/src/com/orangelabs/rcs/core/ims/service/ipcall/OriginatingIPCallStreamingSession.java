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

import com.orangelabs.rcs.core.content.LiveAudioContent;
import com.orangelabs.rcs.core.content.LiveVideoContent;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.richcall.video.SdpOrientationExtension;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoCodecManager;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoSdpBuilder;
import com.orangelabs.rcs.service.api.client.media.IAudioPlayer;
import com.orangelabs.rcs.service.api.client.media.IAudioRenderer;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.audio.AudioCodec;
import com.orangelabs.rcs.service.api.client.media.video.VideoCodec;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Originating IP call session
 *
 * @author opob7414
 */
public class OriginatingIPCallStreamingSession extends IPCallStreamingSession {
	
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    /**
     * Constructor
     *
     * @param parent IMS service
     * @param contact Remote contact
     * @param audioContent Audio content
     * @param audioPlayer Audio player
     * @param audioRenderer Audio renderer
     * @param videoContent Video content
     * @param videoPlayer Video player
     * @param videoRenderer Video renderer
     */
    public OriginatingIPCallStreamingSession(ImsService parent, String contact,
    		LiveAudioContent audioContent, IAudioPlayer audioPlayer, IAudioRenderer audioRenderer,
    		LiveVideoContent videoContent, IVideoPlayer videoPlayer, IVideoRenderer videoRenderer) {
    	super(parent, contact, audioContent, videoContent);
    	
        // Create dialog path
        createOriginatingDialogPath();
        
        // Set the video player
        setVideoPlayer(videoPlayer);
        
        // Set the video renderer
        setVideoRenderer(videoRenderer);

        // Set the audio player
        setAudioPlayer(audioPlayer);
        
        // Set the audio renderer
        setAudioRenderer(audioRenderer);
        
    }
    
    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("Initiate a new IP call session as originating");
            }

            // Check audio parameters 
            if (getAudioContent() == null) {
                handleError(new IPCallError(IPCallError.UNSUPPORTED_AUDIO_TYPE, "Audio codec not supported"));
                return;
            }
            if ((getAudioPlayer() == null) || (getAudioPlayer().getAudioCodec() == null)) {
                handleError(new IPCallError(IPCallError.UNSUPPORTED_AUDIO_TYPE,
                		"Audio codec not selected"));
                return;
            }
            if ((getAudioRenderer() == null) || (getAudioRenderer().getAudioCodec() == null)) {
                handleError(new IPCallError(IPCallError.UNSUPPORTED_AUDIO_TYPE,
                		"Audio codec not selected"));
                return;
            }
            
            // build SDP proposal
            String sdp = buildCallInitSdpProposal();

            // Set the local SDP part in the dialog path
            getDialogPath().setLocalContent(sdp); 

            // Create an INVITE request
            if (logger.isActivated()) {
                logger.info("Send INVITE");
            }
            SipRequest invite;  
            if (getVideoContent() == null) {
            	// Voice call
            	invite = SipMessageFactory.createInvite(getDialogPath(), IPCallService.FEATURE_TAGS_IP_VOICE_CALL, sdp);
            } else {
            	// Video call
            	invite = SipMessageFactory.createInvite(getDialogPath(), IPCallService.FEATURE_TAGS_IP_VIDEO_CALL, sdp);
            } 

	        // Set the Authorization header
	        getAuthenticationAgent().setAuthorizationHeader(invite);

	        // Set initial request in the dialog path
            getDialogPath().setInvite(invite);

            // Send INVITE request
            sendInvite(invite);
            
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
	 * Build SDP proposal for audio+ video session (call init or addVideo)
	 * 
	 * @return SDP content or null in case of error
	 */
	private String buildCallInitSdpProposal() {
		if (logger.isActivated()) {
			logger.debug("Build SDP proposal for call init");
		}

		try {
			// Build SDP part
			String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
			String ipAddress = getDialogPath().getSipStack().getLocalIpAddress();
			
			String audioSdp = AudioSdpBuilder.buildSdpOffer(getAudioPlayer().getSupportedAudioCodecs(), 
					getAudioPlayer().getLocalRtpPort());
			
			String videoSdp = "";
	        if ((getVideoContent()!= null)&&(getVideoPlayer()!= null)&&(getVideoRenderer()!= null)) {	        	
					videoSdp = VideoSdpBuilder.buildSdpOfferWithOrientation(
							getVideoPlayer().getSupportedVideoCodecs(),
							getVideoRenderer().getLocalRtpPort());		
	        }
			
	        String  sdp =
	            	"v=0" + SipUtils.CRLF +
	            	"o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
	            	"s=-" + SipUtils.CRLF +
	            	"c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
	            	"t=0 0" + SipUtils.CRLF +
	            	audioSdp + "a=sendrcv" + SipUtils.CRLF +
	            	videoSdp + "a=sendrcv" + SipUtils.CRLF;

	        	return sdp;

		} catch (RemoteException e) {
			if (logger.isActivated()) {
				logger.error("Session initiation has failed", e);
			}

			// Unexpected error
			handleError(new IPCallError(IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
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
		SdpParser parser = new SdpParser(getDialogPath().getRemoteContent()
				.getBytes());

		// Extract the remote host (same between audio and video)
		String remoteHost = SdpUtils
				.extractRemoteHost(parser.sessionDescription.connectionInfo);

		// Extract media ports
		MediaDescription mediaAudio = parser.getMediaDescription("audio");
		int audioRemotePort = mediaAudio.port;
		MediaDescription mediaVideo = parser.getMediaDescription("video");
		int videoRemotePort = -1;

		if (mediaVideo != null) {
			videoRemotePort = mediaVideo.port;
		} else { // no video in sdp
			setVideoPlayer(null);
			setVideoRenderer(null);
			setVideoContent(null);
		}

		// Extract audio codecs from SDP
		Vector<MediaDescription> audio = parser.getMediaDescriptions("audio");
		Vector<AudioCodec> proposedAudioCodecs = AudioCodecManager
				.extractAudioCodecsFromSdp(audio);

		// Extract video codecs from SDP
		Vector<MediaDescription> video = parser.getMediaDescriptions("video");
		Vector<VideoCodec> proposedVideoCodecs = VideoCodecManager
				.extractVideoCodecsFromSdp(video);

		// Audio codec negotiation
		selectedAudioCodec = AudioCodecManager
				.negociateAudioCodec(
						getAudioPlayer().getSupportedAudioCodecs(),
						proposedAudioCodecs);
		if (selectedAudioCodec == null) {
			if (logger.isActivated()) {
				logger.debug("Proposed audio codecs are not supported");
			}

			// Terminate session
			terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);

			// Report error
			handleError(new IPCallError(IPCallError.UNSUPPORTED_AUDIO_TYPE));
			return;
		}

		// Video codec negotiation
		if ((mediaVideo != null) && (getVideoPlayer() != null)) {
			selectedVideoCodec = VideoCodecManager.negociateVideoCodec(
					getVideoPlayer().getSupportedVideoCodecs(),
					proposedVideoCodecs);
			if (selectedVideoCodec == null) {
				if (logger.isActivated()) {
					logger.debug("Proposed video codecs are not supported");
				}

				// Terminate session
				terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);

				// Report error
				handleError(new IPCallError(IPCallError.UNSUPPORTED_VIDEO_TYPE));
				return;
			}
		}

		// Set the audio codec in audio player
		getAudioPlayer().setAudioCodec(selectedAudioCodec.getMediaCodec());
		getAudioPlayer().addListener(new AudioPlayerEventListener(this));

		// Set the audio codec in audio renderer
		getAudioRenderer().setAudioCodec(selectedAudioCodec.getMediaCodec());
		getAudioRenderer().addListener(new AudioPlayerEventListener(this));

		// Set the video codec in video renderer and player
		if ((getVideoRenderer()!= null)&&(getVideoPlayer()!= null)&&(selectedVideoCodec!= null)) {
			getVideoRenderer().setVideoCodec(selectedVideoCodec.getMediaCodec());		
			getVideoPlayer().setVideoCodec(selectedVideoCodec.getMediaCodec());	
			if (logger.isActivated()) {
				logger.debug("Set video codec in the video renderer: "+ selectedVideoCodec.getMediaCodec().getCodecName());
				logger.debug("Set video codec in the video player: "+ selectedVideoCodec.getMediaCodec().getCodecName());
			}
	
		}
		
		// add listeneres on video renderer and player
		if ((getVideoRenderer()!= null)&&(getVideoPlayer()!= null)) {
			getVideoRenderer().addListener(new VideoPlayerEventListener(this));
			getVideoPlayer().addListener(new VideoPlayerEventListener(this));
		}

		// Set the OrientationHeaderID
		if (mediaVideo!= null) {
			SdpOrientationExtension extensionHeader = SdpOrientationExtension
					.create(mediaVideo);
			if ((getVideoRenderer()!= null)&&(getVideoPlayer()!= null)&&(extensionHeader != null)) {
				getVideoRenderer().setOrientationHeaderId(
						extensionHeader.getExtensionId());
				getVideoPlayer().setOrientationHeaderId(
						extensionHeader.getExtensionId());
			}
		}
		

		// Open the audio renderer
//		getAudioRenderer().open(remoteHost, audioRemotePort);
		
		// Open the audio player - always open the player after the renderer when the RTP stream is shared
//		getAudioPlayer().open(remoteHost, audioRemotePort); 

		// Open the video player/renderer
		if ((getVideoRenderer()!= null)&&(getVideoPlayer()!= null)&&(selectedVideoCodec!= null)) {
			getVideoRenderer().open(remoteHost, videoRemotePort);
			// always open the player after the renderer when the RTP stream is shared
			getVideoPlayer().open(remoteHost, videoRemotePort); 
			if (logger.isActivated()) {
				logger.debug("Open video renderer with remoteHost ("
						+ remoteHost + ") and remotePort (" + videoRemotePort
						+ ")");
				logger.debug("Open video player on renderer RTP stream");
			}
		}
	}

	@Override
	public void startMediaSession() throws Exception {
//		getAudioPlayer().start();	
//		getAudioRenderer().start();
		
		if ((getVideoPlayer()!= null)&&(getVideoRenderer()!= null) ){
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
