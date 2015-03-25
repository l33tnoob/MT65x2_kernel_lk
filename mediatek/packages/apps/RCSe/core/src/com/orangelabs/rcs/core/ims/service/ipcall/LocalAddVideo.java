package com.orangelabs.rcs.core.ims.service.ipcall;

import java.util.Vector;

import android.os.RemoteException;

import com.orangelabs.rcs.core.content.ContentManager;
import com.orangelabs.rcs.core.content.LiveVideoContent;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.richcall.video.SdpOrientationExtension;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoCodecManager;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoSdpBuilder;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.video.VideoCodec;
import com.orangelabs.rcs.utils.logger.Logger;

public class LocalAddVideo extends AddVideoManager {

	/**
	 * Constructor
	 */
	public LocalAddVideo(IPCallStreamingSession session) {
		super(session);
	}
	
	
	/**
	 * Add video to the session
	 *  
	 * @param videoPlayer video player instance
	 * @param videoRenderer video renderer instance
	 */
	public void addVideo(IVideoPlayer videoPlayer, IVideoRenderer videoRenderer) {
		if (logger.isActivated()) {
			logger.info("addVideo() - LocalAddVideo");
		}
		synchronized (this) {
			state = AddVideoManager.ADD_VIDEO_INPROGRESS;

			// Set video player/render
			session.setVideoRenderer(videoRenderer);
			session.setVideoPlayer(videoPlayer);

			// create and set live Video Content
			LiveVideoContent liveVideoContent = (videoPlayer == null) ? null
					: ContentManager.createGenericLiveVideoContent();
			session.setVideoContent(liveVideoContent);

			// Build SDP
			String sdp = buildAddVideoSdpProposal();
			
			if (sdp != null) {
				// Set SDP proposal as the local SDP part in the dialog path
				session.getDialogPath().setLocalContent(sdp);

				// Create re-INVITE
				SipRequest reInvite = session.getUpdateSessionManager()
						.createReInvite(IPCallService.FEATURE_TAGS_IP_VIDEO_CALL,
								sdp);

				// Send re-INVITE
				session.getUpdateSessionManager().sendReInvite(reInvite,
						IPCallStreamingSession.ADD_VIDEO);
			}

			
		}
	}
	
	/**
	 * Remove video from the current call
	 */
	public void removeVideo() {
		if (logger.isActivated()) {
			logger.info("removeVideo() - LocalAddVideo");
			logger.info("video status =" + session.getVideoContent());
		}

		synchronized (this) {
			state = AddVideoManager.REMOVE_VIDEO_INPROGRESS;

			// Build SDP
			String sdp = buildRemoveVideoSdpProposal();

			if (sdp != null) {
				// Set the SDP proposal as local SDP content in the dialog path
				session.getDialogPath().setLocalContent(sdp);

				// Create re-INVITE
				SipRequest reInvite = session.getUpdateSessionManager()
						.createReInvite(IPCallService.FEATURE_TAGS_IP_VOICE_CALL,
								sdp);

				// Send re-INVITE
				session.getUpdateSessionManager().sendReInvite(reInvite,
						IPCallStreamingSession.REMOVE_VIDEO);
			}
			
		}
	}
	
	
	/**
	 * Build SDP proposal for audio+ video session (call init or addVideo)
	 * 
	 * @return String (SDP content) or null in case of error
	 */
	private String buildAddVideoSdpProposal() {
		if (logger.isActivated()) {
			logger.debug("Build SDP proposal to add video stream in the session");
		}

		try {
			// Build SDP part
			String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
			String ipAddress = session.getDialogPath().getSipStack().getLocalIpAddress();
			
			String audioSdp = AudioSdpBuilder.buildSdpOffer(session.getAudioPlayer().getSupportedAudioCodecs(), 
					session.getAudioPlayer().getLocalRtpPort());
			
			String videoSdp = "";
	        if ((session.getVideoContent()!= null)&&(session.getVideoPlayer()!= null)&&(session.getVideoRenderer()!= null)) {	        	
					videoSdp = VideoSdpBuilder.buildSdpOfferWithOrientation(
							session.getVideoPlayer().getSupportedVideoCodecs(),
							session.getVideoRenderer().getLocalRtpPort());		
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
				logger.error("Add video has failed", e);
			}

			// Unexpected error
			session.handleError(new IPCallError(IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
			return null;
		}
	}

	
	/**
	 * Build SDP proposal to remove video stream from the session
	 * 
	 * @return String (SDP content) or null in case of error
	 */
	private String buildRemoveVideoSdpProposal() {
		if (logger.isActivated()) {
			logger.debug("Build SDP proposal to remove video stream from the session");
		}

		try {
			// Build SDP part
			String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
			String ipAddress = session.getDialogPath().getSipStack().getLocalIpAddress();

			session.getAudioPlayer().getLocalRtpPort();			
			String audioSdp = AudioSdpBuilder.buildSdpOffer(session.getAudioPlayer().getSupportedAudioCodecs(), session.getAudioPlayer().getLocalRtpPort());
			
			return "v=0" + SipUtils.CRLF +
					"o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
					"s=-" + SipUtils.CRLF +
					"c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
					"t=0 0" + SipUtils.CRLF + audioSdp +
					"a=sendrcv"	+ SipUtils.CRLF;
		} catch (RemoteException e) {
			if (logger.isActivated()) {
				logger.error("Remove video has failed", e);
			}

			// Unexpected error
			session.handleError(new IPCallError(IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
			return null;
		}
	}
	



	@Override
	public LiveVideoContent addVideo(SipRequest reInvite) {
		return null;
		// Not used in Local Add Video Manager

	}


	@Override
	public void removeVideo(SipRequest reInvite) {
		// Not used in Local Add Video Manager
		
	}
}
		
		
