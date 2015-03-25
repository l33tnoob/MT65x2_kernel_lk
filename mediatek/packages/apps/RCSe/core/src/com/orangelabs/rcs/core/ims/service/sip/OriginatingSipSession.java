/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.service.sip;

import java.util.Vector;

import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Originating SIP session
 * 
 * @author jexa7410
 */
public class OriginatingSipSession extends GenericSipSession {
	/**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

	/**
	 * Constructor
	 * 
	 * @param parent IMS service
	 * @param contact Remote contact
	 * @param featureTag Feature tag
	 * @param sdp SDP
	 */
	public OriginatingSipSession(ImsService parent, String contact, String featureTag, String sdp) {
		super(parent, contact, featureTag);
		
		// Create dialog path
		createOriginatingDialogPath();
		
		// Set the local SDP
		setLocalSdp(sdp);
	}

	/**
	 * Background processing
	 */
	public void run() {
		try {
	    	if (logger.isActivated()) {
	    		logger.info("Initiate a new session as originating");
	    	}
	    	/**
	    	 * M: Modified to resolve rich call 403 error. @{
	    	 */
	    	SipRequest invite = createSipInvite();
	    	/**
	    	 * @}
	    	 */
			
			// Set the local SDP part in the dialog path
	        getDialogPath().setLocalContent(getLocalSdp());

	        // Create an INVITE request
	        if (logger.isActivated()) {
	        	logger.info("Send INVITE");
	        }
	         invite = SipMessageFactory.createInvite(getDialogPath(),
	        		new String [] { getFeatureTag() },
	        		getLocalSdp());

	        // Set the Authorization header
	        getAuthenticationAgent().setAuthorizationHeader(invite);
	        
	        // Set initial request in the dialog path
	        getDialogPath().setInvite(invite);
	    
	        // Send INVITE request
	        sendInvite(invite);	        
		} catch(Exception e) {
        	if (logger.isActivated()) {
        		logger.error("Session initiation has failed", e);
        	}

        	// Unexpected error
			handleError(new SipSessionError(SipSessionError.UNEXPECTED_EXCEPTION,
					e.getMessage()));
		}
	}
	
        	/**
     * M: Modified to resolve rich call 403 error. @{
     */
    /**
     * @return A sip invite request
     */
    @Override
    protected SipRequest createSipInvite(String callId) {
        logger.debug("createSipInvite(), callId = " + callId);
        createOriginatingDialogPath(callId);
        return createSipInvite();
    }

    private SipRequest createSipInvite() {
        logger.debug("createSipInvite()");
        // Set the local SDP part in the dialog path
        getDialogPath().setLocalContent(getLocalSdp());

        // Create an INVITE request
        if (logger.isActivated()) {
            logger.info("Send INVITE");
        }
        try {
            SipRequest invite;
            invite = SipMessageFactory.createInvite(getDialogPath(), new String[] {
                getFeatureTag()
            }, getLocalSdp());
            // Set initial request in the dialog path
            getDialogPath().setInvite(invite);
            return invite;
        } catch (SipException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        logger.error("Create sip invite failed, return null.");
        return null;
    }
    /**
     * @}
     */
    /**
	 * Close media session
	 */
	public void startMediaSession() {
	}
    /**
	 * Close media session
	 */
	public void prepareMediaSession() {
	}
    /**
	 * Close media session
	 */
	public void closeMediaSession() {
	}
}
