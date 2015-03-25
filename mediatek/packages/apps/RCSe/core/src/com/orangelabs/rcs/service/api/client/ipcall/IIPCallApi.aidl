 package com.orangelabs.rcs.service.api.client.ipcall;

import com.orangelabs.rcs.service.api.client.ipcall.IIPCallSession;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.IAudioPlayer;
import com.orangelabs.rcs.service.api.client.media.IAudioRenderer;

/**
 * IP call API
 * 
 * @author owom5460
 */
interface IIPCallApi {
	// Initiate a call session
	IIPCallSession initiateCall(in String contact, in IAudioPlayer audioPlayer, in IAudioRenderer audioRenderer, in IVideoPlayer videoPlayer, in IVideoRenderer videoRenderer);

	// Get current call session from its session ID
	IIPCallSession getSession(in String id);

	// Get list of current call sessions
	List<IBinder> getSessions();
}


