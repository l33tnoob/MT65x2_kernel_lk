package com.orangelabs.rcs.service.api.server.ipcall;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Vector;

import android.content.Intent;
import android.os.IBinder;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.content.AudioContent;
import com.orangelabs.rcs.core.content.VideoContent;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.service.ipcall.IPCallStreamingSession;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.ipcall.IPCall;
import com.orangelabs.rcs.provider.ipcall.IPCallData;
import com.orangelabs.rcs.service.api.client.ipcall.IIPCallApi;
import com.orangelabs.rcs.service.api.client.ipcall.IIPCallSession;
import com.orangelabs.rcs.service.api.client.ipcall.IPCallApiIntents;
import com.orangelabs.rcs.service.api.client.media.IAudioPlayer;
import com.orangelabs.rcs.service.api.client.media.IAudioRenderer;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.server.ServerApiException;
import com.orangelabs.rcs.service.api.server.ServerApiUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * IP call API service
 * 
 * @author owom5460
 */
public class IPCallApiService extends IIPCallApi.Stub {
	/**
	 * List of IP call sessions
	 */
    private static Hashtable<String, IIPCallSession> ipCallSessions = new Hashtable<String, IIPCallSession>();

	/**
	 * The logger
	 */
    private static Logger logger = Logger.getLogger(IPCallApiService.class.getName());

	/**
	 * Constructor
	 */
	public IPCallApiService() {
		if (logger.isActivated()) {
			logger.info("IP call API is loaded");
		}
	}

	/**
	 * Close API
	 */
	public void close() {
		// Clear lists of sessions
		ipCallSessions.clear();
	}

	/**
     * Add an IP Call session in the list
     * 
     * @param sessionApi Video sharing session
     */
	
	protected static void addIPCallSession(IPCallSession sessionApi){
		if (logger.isActivated()) {
			logger.debug("Add an IP Call session in the list (size=" + ipCallSessions.size() + ")");
		}
		
		ipCallSessions.put(sessionApi.getSessionID(), sessionApi);
	}

    /**
     * Remove an IP Call session from the list
     * 
     * @param sessionId Session ID
     */
	protected static void removeIPCallSession(String sessionId) {
		if (logger.isActivated()) {
			logger.debug("Remove an IP Call session from the list (size=" + ipCallSessions.size() + ")");
		}
		ipCallSessions.remove(sessionId);
	}

	/**
     * Initiate an IP call session with Audio + Video
     * 
     * @param contact Contact
     * @param audioPlayer Media player for audio
     * @param audioRenderer Media renderer for audio
     * @param videoPlayer Media player for video
     * @param videoRenderer Media renderer for video
     * @throws ServerApiException
     */	
	public IIPCallSession initiateCall(String contact,
			IAudioPlayer audioPlayer, IAudioRenderer audioRenderer,
			IVideoPlayer videoPlayer, IVideoRenderer videoRenderer)	throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Initiate an IP call session with " + contact);
		}

		// Check permission
		ServerApiUtils.testPermission();

		// Test IMS connection
		ServerApiUtils.testIms();

		// Test if at least the audio media is configured
		if ((audioPlayer == null) || (audioRenderer == null)) {
			throw new ServerApiException("Missing audio player or renderer");
		}
		
		try {
			// Initiate a new session
			IPCallStreamingSession session = Core.getInstance().getIPCallService().initiateIPCallSession(contact,
					audioPlayer, audioRenderer, videoPlayer, videoRenderer);

			// Update IP call history
			IPCall.getInstance().addCall(contact,
					session.getSessionID(),
					IPCallData.EVENT_OUTGOING,
					session.getAudioContent(),
					session.getVideoContent(),
					IPCallData.STATUS_STARTED);

			// Add session in the list
			IPCallSession sessionApi = new IPCallSession(session);
			IPCallApiService.addIPCallSession(sessionApi);
			return sessionApi;
		} catch (Exception e) {
			throw new ServerApiException(e.getMessage());
		}
	} 
		
	/**
     * Receive a new IP call invitation
     * 
     * @param session IP call session
	 * @throws ServerApiException 
     */
	public void receiveIPCallInvitation(IPCallStreamingSession session) {
		if (logger.isActivated()) {
			logger.info("Receive IP call invitation from " + session.getRemoteContact());
		}

		// Extract number from contact
		String number = PhoneUtils.extractNumberFromUri(session.getRemoteContact());

		// Get audio encoding
		AudioContent audiocontent = (AudioContent)session.getAudioContent();
		String audioEncoding = null;
		if (audiocontent != null) {
			audioEncoding = audiocontent.getEncoding();
		}
		// TODO: if no audio then rejects the call
		
		// Get video encoding
		VideoContent videocontent  = (VideoContent)session.getVideoContent();
		String videoEncoding = null;
		int videoWidth = -1;
		int videoHeight = -1;
		if (videocontent != null) {
			videoEncoding = videocontent.getEncoding();
			videoWidth = videocontent.getWidth();
			videoHeight = videocontent.getHeight();
		}				

		// Add session in the list
		IPCallSession sessionApi = new IPCallSession(session);
		IPCallApiService.addIPCallSession(sessionApi);

		// Update IP call history
		IPCall.getInstance().addCall(number, session.getSessionID(),
				IPCallData.EVENT_INCOMING, audiocontent,
				videocontent, IPCallData.STATUS_STARTED);

		// Broadcast intent related to the received invitation
		Intent intent = new Intent(IPCallApiIntents.IPCALL_INVITATION);
		intent.putExtra("contact", number);
		intent.putExtra("contactDisplayname",
				session.getRemoteDisplayName());
		intent.putExtra("sessionId", session.getSessionID());
		intent.putExtra("audiotype", audioEncoding);
		if (videocontent != null) {
			intent.putExtra("audiotype", audioEncoding);
			intent.putExtra("videotype", videoEncoding);
			intent.putExtra("videowidth", videoWidth);
			intent.putExtra("videoheight", videoHeight);
		}
		
		if (logger.isActivated()) {
			logger.info("IPCallInvitation Intent ");
			logger.info("audiotype :"+audioEncoding);
			logger.info("videotype :"+videoEncoding);
			logger.info("videowidth :"+videoWidth);
			logger.info("videoheight :"+videoHeight);
		}
        AndroidFactory.getApplicationContext().sendBroadcast(intent);
        
        
	}
	
	/**
	 * Get a current IP call session from its session ID
	 *
	 * @param id Session ID
	 * @return Session
	 * @throws ServerApiException
	 */
	public IIPCallSession getSession(String id) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Get IP call session " + id);
		}

		// Check permission
		ServerApiUtils.testPermission();

		// Test core availability
		ServerApiUtils.testCore();

		// Return a session instance
		return ipCallSessions.get(id);
	}
	
	/**
	 * Get list of current IP call sessions
	 * 
	 * @return List of sessions
	 * @throws ServerApiException
	 */
	public List<IBinder> getSessions() throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Get IP call sessions");
		}

		// Check permission
		ServerApiUtils.testPermission();

		// Test core availability
		ServerApiUtils.testCore();
		
		try {
			Vector<IPCallStreamingSession> list = Core.getInstance().getIPCallService().getIPCallSessions();
			ArrayList<IBinder> result = new ArrayList<IBinder>(list.size());
			for(int i=0; i < list.size(); i++) {
				IPCallStreamingSession session = list.elementAt(i);
				SipDialogPath dialog = session.getDialogPath();
				if ((dialog != null) && (dialog.isSigEstablished())) {
					// Returns only sessions which are established
					IIPCallSession sessionApi = ipCallSessions.get(session.getSessionID());
					if (sessionApi != null) {
						result.add(sessionApi.asBinder());
					}
				}
			}
			return result;
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
	}	
}

