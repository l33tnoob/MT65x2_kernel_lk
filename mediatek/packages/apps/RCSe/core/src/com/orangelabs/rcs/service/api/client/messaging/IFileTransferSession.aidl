package com.orangelabs.rcs.service.api.client.messaging;

import com.orangelabs.rcs.service.api.client.messaging.IFileTransferEventListener;

/**
 * File transfer session interface
 */
interface IFileTransferSession {
	// Get session ID
	String getSessionID();

	// Get remote contact
	String getRemoteContact();

	// Get list of contacts (only for group transfer)
	List<String> getContacts();

	// Is group transfer
	boolean isGroupTransfer();

	// Is HTTP transfer
	boolean isHttpTransfer();

	// Get chat ID (ie. Contribution ID) used to send file transfer URL via chat
	String getChatID();

	// Get session ID of the chat used to send file transfer URL via chat
	String getChatSessionID();

	// Get session direction
	int getSessionDirection();

	// Get session state
	int getSessionState();
	
	
	String  getFileThumbUrl();

	// Get filename
	String getFilename();
	
	// Get file size
	long getFilesize();

	// Get file thumbnail
	byte[] getFileThumbnail();

	// Accept the session invitation
	void acceptSession();

	// Reject the session invitation
	void rejectSession();

	// Cancel the session
	void cancelSession();

	// Pause the session (only for HTTP transfer)
	void pauseSession();

	// Resume the session (only for HTTP transfer)
	void resumeSession();

	 boolean isSessionPaused();

	// Add session listener
	void addSessionListener(in IFileTransferEventListener listener);

	// Remove session listener
	void removeSessionListener(in IFileTransferEventListener listener);
}
