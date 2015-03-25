 package com.orangelabs.rcs.service.api.client.ipcall;

import com.orangelabs.rcs.service.api.client.ipcall.IIPCallEventListener;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IAudioRenderer;
import com.orangelabs.rcs.service.api.client.media.IAudioPlayer;

/**
 * IP call session interface
 * 
 * @author owom5460
 */
interface IIPCallSession {
	// Get session ID
	String getSessionID();

	// Get remote contact
	String getRemoteContact();
	
	// Get session state
	int getSessionState();

	// Get session direction
	int getSessionDirection();

	// Accept the session invitation
	void acceptSession(in boolean video);

	// Reject the session invitation
	void rejectSession();

	// Cancel the session
	void cancelSession();
	
	// Add video to the session
	void addVideo(in IVideoPlayer videoPlayer, in IVideoRenderer videoRenderer);
	
	// Accept invitation to add video
	void acceptAddVideo();
	
	// Reject invitation to add video
	void rejectAddVideo();
	
	// Remove video from the session
	void removeVideo();
	
	// Set call hold
	void setCallHold(in boolean flag);

	// Set the video renderer
	void setVideoRenderer(in IVideoRenderer renderer);

    // Get the video renderer
    IVideoRenderer getVideoRenderer();

    // Set the video player
    void setVideoPlayer(in IVideoPlayer player);

    // Get the video player
    IVideoPlayer getVideoPlayer();
    
    // Set the audio renderer
	void setAudioRenderer(in IAudioRenderer renderer);

    // Get the audio renderer
    IAudioRenderer getAudioRenderer();

    // Set the audio player
    void setAudioPlayer(in IAudioPlayer player);

    // Get the audio player
    IAudioPlayer getAudioPlayer();

	// Add session listener
	void addSessionListener(in IIPCallEventListener listener);

	// Remove session listener
	void removeSessionListener(in IIPCallEventListener listener);
}

