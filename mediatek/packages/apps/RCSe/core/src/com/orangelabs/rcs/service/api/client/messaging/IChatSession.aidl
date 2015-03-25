package com.orangelabs.rcs.service.api.client.messaging;

import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;

/**
 * Chat session interface
 */
interface IChatSession {
	// Get session ID
	String getSessionID();

	// Get chat ID
	String getChatID();

	// Get remote contact
	String getRemoteContact();
	
	// Get session direction
	int getSessionDirection();

	// Get session state
	int getSessionState();

	// Is session idle.
    boolean isSessionIdle();

	// Is group chat
	boolean isGroupChat();
	
	// Is Store & Forward
	boolean isStoreAndForward();

	// Get first message exchanged during the session
	InstantMessage getFirstMessage();

	// Get subject associated to the session
	String getSubject();

	// Accept the session invitation
	void acceptSession();

	// Reject the session invitation
	void rejectSession();

	// Cancel the session
	void cancelSession();

	// Get list of participants in the session
	List<String> getParticipants();

	// Get max number of participants in the session
	int getMaxParticipants();

	// Get max number of participants which can be added to the conference
	int getMaxParticipantsToBeAdded();

	// Add a participant to the session
	void addParticipant(in String participant);

	// Add a list of participants to the session
	void addParticipants(in List<String> participants);

	// Send a text message
	String sendMessage(in String text);

	//TODO add mediatek marker
	 void sendMessageWithMsgId(in String text,in String msgid) ;

	// Is geoloc supported
	boolean isGeolocSupported();

	// Send a geoloc message
	String sendGeoloc(in GeolocPush geoloc);

	// Is file transfer supported
	boolean isFileTransferSupported();

	// Send a file to participants of the group chat
	IFileTransferSession sendFile(in String file, in boolean thumbnail);

	// Set is composing status
	void setIsComposingStatus(in boolean status);

	// Set message delivery status
	void setMessageDeliveryStatus(in String msgId, in String status);

	// Add session listener
	void addSessionListener(in IChatEventListener listener);

	// Remove session listener
	void removeSessionListener(in IChatEventListener listener);
	
	// Set message displayed status by sip message
    void setMessageDisplayedStatusBySipMessage(in String contact, in String msgId, in String status);
    
    //Get the Referred-By header
    String getReferredByHeader();
    
    //Get participants no matter he has accepted the session.
    List<String> getInivtedParticipants();


    /**
     * M: Add this method to define whether this session is an incoming one @{
     */
    //Whether is an incoming chat session
    boolean isInComing();
    /**
     * @}
     */
	
}
