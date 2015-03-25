package com.orangelabs.rcs.core.ims.service.ipcall;

import java.util.Vector;

import android.os.RemoteException;

import com.orangelabs.rcs.core.content.LiveVideoContent;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ImsSessionBasedServiceError;
import com.orangelabs.rcs.core.ims.service.ipcall.IPCallStreamingSession.VideoPlayerEventListener;
import com.orangelabs.rcs.core.ims.service.richcall.video.SdpOrientationExtension;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoCodecManager;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.video.VideoCodec;
import com.orangelabs.rcs.utils.logger.Logger;


/**
 * Super class for IP Call Add Video Manager 
 * 
 * @author O. Magnon
 */
public abstract class AddVideoManager {

	/**
	 * Constant values for IPCall Hold states
	 */
	public static final int IDLE = 0; 
	public static final int ADD_VIDEO_INPROGRESS = 1;
	public static final int REMOVE_VIDEO_INPROGRESS = 2;
	
	/**
	 * Add Video state
	 */
	protected static int state  ;
	
	/**
	 * session handled by AddVideoManager
	 */
	IPCallStreamingSession session ; 	
	
	/**
	 * The logger
	 */
	protected Logger logger = Logger.getLogger(this.getClass().getName());	
	
	/**
	 * constructor
	 */
	public AddVideoManager(IPCallStreamingSession session){
		if (logger.isActivated()){
			logger.info("AddVideoManager()");
		}
		this.state = AddVideoManager.IDLE;
		this.session = session;
	}	
	
	/**
	 * get AddVideoManager state
	 * 
	 * @return int state
	 */
	public static int getState(){
		return state;
	}	
	
	/**
	 * set AddVideoManager state
	 */
	public static void setState(int val){
		state = val;
	}
	
	
	/**
	 * add Video to session (case local AddVideoManager)
	 * 
	 * @param videoPlayer video player instance
	 * @param videoRenderer video renderer instance
	 */
	public abstract void addVideo(IVideoPlayer videoPlayer, IVideoRenderer videoRenderer);
	
	
	/**
	 * add Video to session (case remote AddVideoManager)
	 * 
	 * @param reInvite reInvite SIP request received
	 */
	public abstract LiveVideoContent addVideo(SipRequest reInvite);
	
	
	/**
	 * remove Video from session (case local AddVideoManager)
	 */
	public abstract void removeVideo();
	
	
	/**
	 * remove Video from session (case remote AddVideoManager)
	 * 
	 * @param reInvite reInvite SIP request received
	 */
	public abstract void removeVideo(SipRequest reInvite);
		
	
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
			// Extract video codecs from SDP
			Vector<MediaDescription> medias = parser
					.getMediaDescriptions("video");
			Vector<VideoCodec> proposedCodecs = VideoCodecManager
					.extractVideoCodecsFromSdp(medias);

			// Codec negotiation
			session.selectedVideoCodec = VideoCodecManager
					.negociateVideoCodec(session.getVideoPlayer()
							.getSupportedVideoCodecs(), proposedCodecs);

			if (session.selectedVideoCodec == null) {
				if (logger.isActivated()) {
					logger.debug("Proposed codecs are not supported");
				}

				// Terminate session
				session.terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);

				// Report error Unsupported video type
				session.handleError(new IPCallError(
						IPCallError.UNSUPPORTED_VIDEO_TYPE));
				return;
			}

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
	
	/**
     * Start video session
     */
	public void startVideoSession() {
		if (logger.isActivated()) {
			logger.info("startVideoSession()");
		}

		try {
			// Start the video player and renderer
			session.getVideoPlayer().start();			
			session.getVideoRenderer().start();
		} catch (RemoteException e) {
			if (logger.isActivated()) {
				logger.error("Start Video session has failed", e);
			}
			session.handleError(new IPCallError(
					IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
		}
		// set AddVideoManager state
		AddVideoManager.state = AddVideoManager.IDLE ;

	}
	
	
	/**
	 * Close video session  
	 */
	public void closeVideoSession() {
		if (logger.isActivated()) {
			logger.info("closeVideoSession()");
		}

		try {
			if (session.getVideoPlayer() != null) {
				// Close the video player

				session.getVideoPlayer().stop();
				session.getVideoPlayer().close();
				if (logger.isActivated()) {
					logger.info("stop and close Video player");
				}
			}
			if (session.getVideoRenderer() != null) {
				// Close the video renderer

				session.getVideoRenderer().stop();
				session.getVideoRenderer().close();
				if (logger.isActivated()) {
					logger.info("stop and close Video renderer");
				}
			}
		} catch (Exception e) {
			if (logger.isActivated()) {
				logger.error(
						"Exception when closing the video player or video renderer",
						e);
			}
			session.handleError(new IPCallError(
					IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
		}

		// set player/renderer and videoContent to null
		session.setVideoPlayer(null);
		session.setVideoRenderer(null);
		session.setVideoContent(null);

		// set AddVideoManager state
		AddVideoManager.state = AddVideoManager.IDLE;
	}
	
	
}
