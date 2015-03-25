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
import com.orangelabs.rcs.core.ims.service.ipcall.IPCallStreamingSession.VideoPlayerEventListener;
import com.orangelabs.rcs.core.ims.service.richcall.video.SdpOrientationExtension;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoCodecManager;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoSdpBuilder;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.video.VideoCodec;
import com.orangelabs.rcs.utils.CodecsUtils;

public class RemoteAddVideo extends AddVideoManager {

	/**
	 * Constructor
	 */
	public RemoteAddVideo(IPCallStreamingSession session) {
		super(session);
	}

	/**
	 * Add Video
	 * 
	 * @param reInvite	reInvite SIP Request received
	 * @return LiveVideoContent
	 */
	public LiveVideoContent addVideo(SipRequest reInvite) {
		if (logger.isActivated()) {
			logger.info("addVideo() - RemoteAddVideo");
		}
		synchronized (this) {
			// set AddVideoManager state
			state = AddVideoManager.ADD_VIDEO_INPROGRESS;

			// create Video Content and set it on session
			LiveVideoContent videocontent = ContentManager
					.createLiveVideoContentFromSdp(reInvite.getContentBytes());
			session.setVideoContent(videocontent);

			// processes user Answer and SIP response
			session.getUpdateSessionManager().waitUserAckAndSendReInviteResp(
					reInvite, IPCallService.FEATURE_TAGS_IP_VIDEO_CALL,
					IPCallStreamingSession.ADD_VIDEO);

			return videocontent;
		}
	}

	/**
	 * Remove Video
	 * 
	 * @param reInvite	reInvite SIP Request received
	 */
	public void removeVideo(SipRequest reInvite) {
		if (logger.isActivated()) {
			logger.info("removeVideo() - RemoteAddVideo");
		}
		synchronized (this) {
			// set AddVideoManager state
			state = AddVideoManager.REMOVE_VIDEO_INPROGRESS;

			// build sdp response
			String sdp = buildRemoveVideoSdpResponse();
			
			if (sdp != null) {
				// set sdp response as local content
				session.getDialogPath().setLocalContent(sdp);

				// process user Answer and SIP response
				session.getUpdateSessionManager().send200OkReInviteResp(reInvite,
						IPCallService.FEATURE_TAGS_IP_VOICE_CALL, sdp,
						IPCallStreamingSession.REMOVE_VIDEO);
			}

		}
	}

	/**
	 * Build sdp response for addVideo
	 * 
	 * @param reInvite	reInvite SIP Request received
	 * @return String (sdp content)
	 */
	public String buildAddVideoSdpResponse(SipRequest reInvite) {
		if (logger.isActivated()) {
			logger.info("buildAddVideoSdpResponse()");
		}

		String sdp = "";

		// Parse the remote SDP part
		SdpParser parser = new SdpParser(reInvite.getSdpContent().getBytes());
		MediaDescription mediaVideo = parser.getMediaDescription("video");

		// Extract video codecs from SDP
		Vector<MediaDescription> medias = parser.getMediaDescriptions("video");
		Vector<VideoCodec> proposedVideoCodecs = VideoCodecManager
				.extractVideoCodecsFromSdp(medias);
		try {		
			// Check that a video player and renderer has been set
			if (session.getVideoPlayer() == null) {
				session.handleError(new IPCallError(
						IPCallError.UNSUPPORTED_VIDEO_TYPE,
						"Video player null or Video codec not selected"));
				return null;
			}

			if ((session.getVideoRenderer() == null)
					|| (session.getVideoRenderer().getVideoCodec() == null)) {
				session.handleError(new IPCallError(
						IPCallError.UNSUPPORTED_VIDEO_TYPE,
						"Video renderer null or Video codec not selected"));
				return null;
			}

			// Codec negotiation
			session.selectedVideoCodec = VideoCodecManager.negociateVideoCodec(
					session.getVideoRenderer().getSupportedVideoCodecs(),
					proposedVideoCodecs);

			if (session.selectedVideoCodec == null) {
				// Support of video codec of profile 1B is compulsory. Even if
				// not proposed explicitly it shall be selected.
				session.selectedVideoCodec = CodecsUtils
						.getVideoCodecProfile1b(session.getVideoRenderer()
								.getSupportedVideoCodecs());
				if (session.selectedVideoCodec != null) {
					if (logger.isActivated())
						logger.info("Video codec profile 1B is selected by default");
				} else {
					if (logger.isActivated()) {
						logger.debug("Proposed codecs are not supported");
					}

					// Send a 415 Unsupported media type response
					session.send415Error(reInvite);

					// Unsupported media type
					session.handleError(new IPCallError(
							IPCallError.UNSUPPORTED_VIDEO_TYPE));
					return null;
				}
			}

			// Build SDP part for response
			String ntpTime = SipUtils.constructNTPtime(System
					.currentTimeMillis());
			String ipAddress = session.getDialogPath().getSipStack()
					.getLocalIpAddress();
			String videoSdp = VideoSdpBuilder.buildSdpAnswer(
					session.selectedVideoCodec.getMediaCodec(), session
							.getVideoRenderer().getLocalRtpPort(), mediaVideo);
			String audioSdp = AudioSdpBuilder.buildSdpAnswer(session
					.getAudioPlayer().getAudioCodec(), session
					.getAudioRenderer().getLocalRtpPort());
			sdp = "v=0" + SipUtils.CRLF + "o=- " + ntpTime + " " + ntpTime
					+ " " + SdpUtils.formatAddressType(ipAddress)
					+ SipUtils.CRLF + "s=-" + SipUtils.CRLF + "c="
					+ SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF
					+ "t=0 0" + SipUtils.CRLF + audioSdp + "a=sendrcv"
					+ SipUtils.CRLF + videoSdp + "a=sendrcv" + SipUtils.CRLF;
			return sdp;
		} catch (RemoteException e) {
			if (logger.isActivated()) {
				logger.error("Remove Video has failed", e);
			}

			// Unexpected error
			session.handleError(new IPCallError(
					IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
			return null;
		}


	}

	/**
	 * Build sdp response for removeVideo
	 * 
	 * @return String sdp content
	 */
	private String buildRemoveVideoSdpResponse() {
		if (logger.isActivated()) {
			logger.info("buildRemoveVideoSdpResponse()");
		}

		// Build SDP part
		String sdp = "";
		String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
		String ipAddress = session.getDialogPath().getSipStack()
				.getLocalIpAddress();

		try {
			logger.warn("Build audio sdp");
			session.getAudioPlayer().getLocalRtpPort();
			String audioSdp = AudioSdpBuilder.buildSdpAnswer(session
					.getAudioPlayer().getAudioCodec(), session.getAudioPlayer()
					.getLocalRtpPort());

			sdp = "v=0" + SipUtils.CRLF + "o=- " + ntpTime + " " + ntpTime
					+ " " + SdpUtils.formatAddressType(ipAddress)
					+ SipUtils.CRLF + "s=-" + SipUtils.CRLF + "c="
					+ SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF
					+ "t=0 0" + SipUtils.CRLF + audioSdp + "a=sendrcv"
					+ SipUtils.CRLF;
			return sdp;
		} catch (RemoteException e) {
			if (logger.isActivated()) {
				logger.error("Remove Video has failed", e);
			}

			// Unexpected error
			session.handleError(new IPCallError(
					IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
			return null;
		}
		
	}

	public void addVideo(IVideoPlayer videoPlayer, IVideoRenderer videoRenderer) {
		// Not used in Remote Add Video Manager
	}

	public void removeVideo() {
		// Not used in Remote Add Video Manager
	}

	
	/**
	 * Prepare video session (set codec, get remote Host and port ...) 
	 */
	public void prepareVideoSession() {
		if (logger.isActivated()) {
			logger.info("prepareVideoSession()");
		}
		// Parse the remote SDP part
		SdpParser parser = new SdpParser(session.getDialogPath()
				.getRemoteContent().getBytes());
		MediaDescription mediaVideo = parser.getMediaDescription("video");
		String remoteHost = SdpUtils
				.extractRemoteHost(parser.sessionDescription.connectionInfo);

		int remotePort = mediaVideo.port;

		try {
//			// Extract video codecs from SDP
//			Vector<MediaDescription> medias = parser
//					.getMediaDescriptions("video");
//			Vector<VideoCodec> proposedCodecs = VideoCodecManager
//					.extractVideoCodecsFromSdp(medias);

//			// Codec negotiation
//			session.selectedVideoCodec = VideoCodecManager
//					.negociateVideoCodec(session.getVideoPlayer()
//							.getSupportedVideoCodecs(), proposedCodecs);
//
//			if (session.selectedVideoCodec == null) {
//				if (logger.isActivated()) {
//					logger.debug("Proposed codecs are not supported");
//				}
//
//				// Terminate session
//				session.terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
//
//				// Report error Unsupported video type
//				session.handleError(new IPCallError(
//						IPCallError.UNSUPPORTED_VIDEO_TYPE));
//				return;
//			}

			// Set the selected video codec
			session.getVideoPlayer().setVideoCodec(
					session.selectedVideoCodec.getMediaCodec());
			session.getVideoRenderer().setVideoCodec(
					session.selectedVideoCodec.getMediaCodec());

			// Set the OrientationHeaderID
			SdpOrientationExtension extensionHeader = SdpOrientationExtension
					.create(mediaVideo);
			if (extensionHeader != null) {
				session.getVideoRenderer().setOrientationHeaderId(
						extensionHeader.getExtensionId());
				session.getVideoPlayer().setOrientationHeaderId(
						extensionHeader.getExtensionId());
			}

			// Set video player and renderer event listeners
			session.getVideoRenderer().addListener(
					session.new VideoPlayerEventListener(session));
			session.getVideoPlayer().addListener(
					session.new VideoPlayerEventListener(session));

			// Open the video renderer and player
			session.getVideoRenderer().open(remoteHost, remotePort);
			session.getVideoPlayer().open(remoteHost, remotePort);

		} catch (RemoteException e) {
			if (logger.isActivated()) {
                logger.error("Prepare Video session has failed", e);
            }
			// Report error
			session.handleError(new IPCallError(
					IPCallError.UNEXPECTED_EXCEPTION));

		}
	}
}
