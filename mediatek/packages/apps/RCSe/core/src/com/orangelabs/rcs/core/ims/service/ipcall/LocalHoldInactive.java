package com.orangelabs.rcs.core.ims.service.ipcall;

import android.os.RemoteException;

import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoSdpBuilder;

public class LocalHoldInactive extends HoldManager {
	
	/**
	 * Constructor
	 */
	public LocalHoldInactive(IPCallStreamingSession session){
		super(session);
	}	
	
	/**
	 * Set Call Hold
	 * 
	 * @param callHoldAction  call hold action (true : call hold - false: call resume)
	 */
	public void setCallHold(boolean callHoldAction) {
		synchronized(this){
				//set HoldManager state
				HoldManager.state = (callHoldAction) ? HoldManager.HOLD_INPROGRESS : HoldManager.UNHOLD_INPROGRESS;
				
				// Build SDP
				String sdp = buildCallHoldSdpProposal(callHoldAction);

				// Set SDP proposal as the local SDP part in the dialog path
				session.getDialogPath().setLocalContent(sdp);

				// get feature tags
				String[] featureTags = null;
				if (session.isTagPresent(sdp, "m=video")) { // audio+ video
					featureTags = IPCallService.FEATURE_TAGS_IP_VIDEO_CALL;
				} else { // audio only
					featureTags = IPCallService.FEATURE_TAGS_IP_VOICE_CALL;
				}

				// Create re-INVITE
				SipRequest reInvite = session.getUpdateSessionManager().createReInvite(
						featureTags, sdp);

				// Send re-INVITE
				int requestType = (callHoldAction) ? IPCallStreamingSession.SET_ON_HOLD
						: IPCallStreamingSession.SET_ON_RESUME;
				session.getUpdateSessionManager().sendReInvite(reInvite, requestType);			
		}		
	}

	/**
	 * Build Call Hold SDP proposal
	 * 
	 * @param callHoldAction  call hold action (true : call hold - false: call resume)
	 */
	private String buildCallHoldSdpProposal(boolean action) {
		if (logger.isActivated()) {
			logger.info ("setCallHold - Inactive");
		}
		try {
			// Build SDP part
			String ntpTime = SipUtils.constructNTPtime(System
					.currentTimeMillis());
			String ipAddress = session.getDialogPath().getSipStack()
					.getLocalIpAddress();
			
			String aVar = (action) ? "a=inactive" : "a=sendrcv";

			String audioSdp = AudioSdpBuilder.buildSdpOffer(session
					.getAudioPlayer().getSupportedAudioCodecs(), session
					.getAudioPlayer().getLocalRtpPort())
					+ aVar + SipUtils.CRLF;

			String videoSdp = "";
			if ((session.getVideoContent() != null)
					&& (session.getVideoPlayer() != null)
					&& (session.getVideoRenderer() != null)) {
				videoSdp = VideoSdpBuilder.buildSdpOfferWithOrientation(session
						.getVideoPlayer().getSupportedVideoCodecs(), session
						.getVideoRenderer().getLocalRtpPort())
						+ aVar + SipUtils.CRLF;
			}

			String sdp = "v=0" + SipUtils.CRLF + "o=- " + ntpTime + " "
					+ ntpTime + " " + SdpUtils.formatAddressType(ipAddress)
					+ SipUtils.CRLF + "s=-" + SipUtils.CRLF + "c="
					+ SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF
					+ "t=0 0" + SipUtils.CRLF + audioSdp + videoSdp;

			return sdp;

		} catch (RemoteException e) {
			if (logger.isActivated()) {
				logger.error("build CallHold SdpProposal has failed", e);
			}

			// Unexpected error
			session.handleError(new IPCallError(
					IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
			return null;
		}
	}

	
	/**
	 * Start Media session when call is Resumed
	 */
	public void resumeMediaSession() {
		// Parse the remote SDP part
		SdpParser remoteParser = new SdpParser(session.getDialogPath()
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

		// Open the audio renderer
//		getAudioRenderer().open(remoteHost, audioRemotePort);
//		if (logger.isActivated()) {
//			logger.debug("Open audio renderer with remoteHost (" + remoteHost
//					+ ") and remotePort (" + audioRemotePort + ")");
//		}
//
//		// Open the audio player
//		getAudioPlayer().open(remoteHost, audioRemotePort);
//		if (logger.isActivated()) {
//			logger.debug("Open audio player on renderer RTP stream");
//		}

		// Open the Video Renderer and Player
		// always open the player after the renderer when the RTP stream is shared
		if ((session.getVideoRenderer() != null)
				&& (session.getVideoPlayer() != null)) {
			try {
				session.getVideoRenderer().open(remoteHost, videoRemotePort);

				session.getVideoPlayer().open(remoteHost, videoRemotePort);
				if (logger.isActivated()) {
					logger.debug("Open video renderer with remoteHost ("
							+ remoteHost + ") and remotePort ("
							+ videoRemotePort + ")");
					logger.debug("Open video player on renderer RTP stream");
				}
			} catch (RemoteException e) {
				if (logger.isActivated()) {
					logger.error("Open video player/renderer has failed", e);
				}

				// Unexpected error
				session.handleError(new IPCallError(
						IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
			}
		}
		
		//Start Audio Player/Renderer
		// getAudioPlayer().start();
		// getAudioRenderer().start();

		
		//Start Video Player/Renderer
		if ((session.getVideoPlayer() != null)
				&& (session.getVideoRenderer() != null)) {
			try {
				if (logger.isActivated()) {
					logger.debug("Start video player and renderer");
				}
				session.getVideoPlayer().start();
				session.getVideoRenderer().start();
			} catch (RemoteException e) {
				if (logger.isActivated()) {
					logger.error("start video player/renderer has failed", e);
				}
				// Unexpected error
				session.handleError(new IPCallError(
						IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
			}		
		} 
		
		//set ip call state
		HoldManager.state = HoldManager.IDLE ;
	}


	/**
	 * Close Media session when call is set On Hold
	 */
	public void holdMediaSession() {
		// if (audioRenderer != null) {
				// // Close the audio renderer
				// try {
				// audioRenderer.stop();
				// audioRenderer.close();
				// if (logger.isActivated()) {
				// logger.info("Stop and Close the audio renderer");
				// }
				// } catch (RemoteException e) {
				// if (logger.isActivated()) {
				// logger.error("Exception when closing the audio renderer", e);
				// }
				// }
				// }
				// if (audioPlayer != null) {
				// // Close the audio player
				// try {
				// audioPlayer.stop();
				// audioPlayer.close();
				// if (logger.isActivated()) {
				// logger.info("Stop and Close the audio player");
				// }
				// } catch (RemoteException e) {
				// if (logger.isActivated()) {
				// logger.error("Exception when closing the audio player", e);
				// }
				// }
				// }

				if (session.getVideoRenderer() != null) {
					// Close the video renderer
					try {
						session.getVideoRenderer().stop();
						session.getVideoRenderer().close();
						if (logger.isActivated()) {
							logger.info("Stop and close video renderer");
						}
					} catch (RemoteException e) {
						if (logger.isActivated()) {
							logger.error("Exception when closing the video renderer", e);
						}
					}
				}
				if (session.getVideoPlayer() != null) {
					// Close the video player
					try {
						session.getVideoPlayer().stop();
						session.getVideoPlayer().close();
						if (logger.isActivated()) {
							logger.info("stop and close video player");
						}
					} catch (Exception e) {
						if (logger.isActivated()) {
							logger.error("Exception when closing the video player", e);
						}
					}
				}
				
				HoldManager.state = HoldManager.HOLD;
	}

	
	public void setCallHold(boolean callHoldAction, SipRequest reInvite) {
		// Not used in IPCall_HoldInactive class
	}

	


}
