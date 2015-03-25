package com.orangelabs.rcs.service.api.client.richcall;

import com.orangelabs.rcs.service.api.client.richcall.IVideoSharingEventListener;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;


/**
 * Video sharing session interface
 */
interface IVideoSharingSession {
	// Get session ID
	String getSessionID();

	// Get remote contact
	String getRemoteContact();
	
	// Get session direction
	int getSessionDirection();

	// Get session state
	int getSessionState();

	// Accept the session invitation
	void acceptSession();

	// Reject the session invitation
	void rejectSession();

	// Cancel the session
	void cancelSession();

	// Set the video renderer
	void setVideoRenderer(in IVideoRenderer renderer);

    // Get the video renderer
    IVideoRenderer getVideoRenderer();

    // Set the video player
    void setVideoPlayer(in IVideoPlayer player);

    // Get the video player
    IVideoPlayer getVideoPlayer();

	// Add session listener
	void addSessionListener(in IVideoSharingEventListener listener);

	// Remove session listener
	void removeSessionListener(in IVideoSharingEventListener listener);
}
