package com.orangelabs.rcs.core.ims.service.ipcall;

import android.os.RemoteException;

import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoSdpBuilder;

public class RemoteHoldSendOnly extends HoldManager {
	/**
	 * Constructor
	 */
	public RemoteHoldSendOnly(IPCallStreamingSession session) {
		super(session);
	}


	/**
	 * Set Call Hold
	 * 
	 * @param callHoldAction  call hold action (true : call hold - false: call resume)
	 * @param reInvite  reInvite Request received
	 */
	@Override
	public void setCallHold(boolean callHoldAction, SipRequest reInvite) {
		if (logger.isActivated()) {
			logger.info("setCallHold - SendOnly");
		}

		synchronized (this) {
			// Set Hold Manager state
			HoldManager.state = (callHoldAction) ? HoldManager.REMOTE_HOLD_INPROGRESS
					: HoldManager.REMOTE_UNHOLD_INPROGRESS;
			// build sdp response
			String sdp = buildCallHoldSdpResponse(callHoldAction);
			
			if (sdp != null){
				// set sdp response as local content
				session.getDialogPath().setLocalContent(sdp);

				// get feature tags
				String[] featureTags = null;
				if (session.isTagPresent(reInvite.getContent(), "m=video")) { // audio+
																				// video
					featureTags = IPCallService.FEATURE_TAGS_IP_VIDEO_CALL;
				} else { // audio only
					featureTags = IPCallService.FEATURE_TAGS_IP_VOICE_CALL;
				}

				int requestType = (callHoldAction) ? IPCallStreamingSession.SET_ON_HOLD
						: IPCallStreamingSession.SET_ON_RESUME;

				// process user Answer and SIP response
				session.getUpdateSessionManager().send200OkReInviteResp(reInvite,
						featureTags, sdp, requestType);
			}
	
		}
	}

	
	/**
	 * Build Call Hold SDP response
	 * 
	 * @param callHoldAction  call hold action (true : call hold - false: call resume)
	 * @return String sdp or null if error
	 */
	private String buildCallHoldSdpResponse(boolean action) {
		try {
			// Build SDP part
			String ntpTime = SipUtils.constructNTPtime(System
					.currentTimeMillis());
			String ipAddress = session.getDialogPath().getSipStack()
					.getLocalIpAddress();
			String aVar = (action) ? "a=receiveonly" : "a=sendrcv";

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
				logger.error("build CallHold SdpResponse has failed", e);
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
		// TODO 	
		
		HoldManager.state = HoldManager.IDLE;
	}

	
	/**
	 * Close Media session when call is set On Hold
	 */
	public void holdMediaSession() {
		// TODO 
		
		HoldManager.state = HoldManager.REMOTE_HOLD;
	}
	
	
	public void setCallHold(boolean callHoldAction) {
		// not used in IPCall-RemoteHoldSendOnly class
	}




	
}
