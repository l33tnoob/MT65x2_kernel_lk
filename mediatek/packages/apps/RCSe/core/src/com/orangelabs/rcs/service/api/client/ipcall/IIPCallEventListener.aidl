package com.orangelabs.rcs.service.api.client.ipcall;

/**
 * IP call event listener interface
 *
 * @author owom5460
 */
interface IIPCallEventListener {

	// Session is started
	void handleSessionStarted();

	// Session has been aborted
	void handleSessionAborted(in int reason);
    
	// Session has been terminated by remote
	void handleSessionTerminatedByRemote();
	
	// Add video invitation
	void handleAddVideoInvitation(in String videoEncoding, in int videoWidth, in int videoHeight);
	
	// Remove video invitation
	void handleRemoveVideo();
	
	// Add video has been accepted by user 
	void handleAddVideoAccepted();
	
	// Remove video has been accepted 
	void handleRemoveVideoAccepted();
	
	// Add video has been aborted
	void handleAddVideoAborted(in int reason);

	// Remove video aborted 
	void handleRemoveVideoAborted(in int reason);

	// Call Hold invitation
	void handleCallHold();
	
	// Call Resume invitation
	void handleCallResume();
	
	// Call Hold has been accepted  
	void handleCallHoldAccepted();
	
	// Call Resume has been accepted 
	void handleCallResumeAccepted();
	
	// Call Hold has been aborted
	void handleCallHoldAborted(in int reason);

	// Call Resume aborted 
	void handleCallResumeAborted(in int reason);
	
	// IP Call error
	void handleCallError(in int error);
	
	// Called user is Busy
	void handle486Busy();

    // Video stream has been resized
    void handleVideoResized(in int width, in int height);
}
