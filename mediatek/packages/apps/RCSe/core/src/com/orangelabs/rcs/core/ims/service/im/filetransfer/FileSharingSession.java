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
package com.orangelabs.rcs.core.ims.service.im.filetransfer;

import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.im.chat.ListOfParticipant;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.SessionState;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;

/**
 * Abstract file sharing session 
 * 
 * @author jexa7410
 */
public abstract class FileSharingSession extends ImsServiceSession {
    /**
     * Contribution ID
     */
    private String contributionId = null;	

   /**
	 * Default SO_TIMEOUT value (in seconds)
	 */
	public final static int DEFAULT_SO_TIMEOUT = 30;
    
    /**
	 * Content to be shared
	 */
	protected MmContent content;
	
	private String thumbUrl = null;
	
	/**
	 * File transfered
	 */
	private boolean fileTransfered = false;

    /**
     * List of participants
     */
    protected ListOfParticipant participants = new ListOfParticipant();

    /**
	 * Thumbnail
	 */
	private byte[] thumbnail = null;
	
    /**
	 * File transfer paused
	 */
	private boolean fileTransferPaused = false;

    /**
	 * Constructor
	 * 
	 * @param parent IMS service
	 * @param content Content to be shared
	 * @param contact Remote contact
	 * @param thumbnail Thumbnail
	 */
	public FileSharingSession(ImsService parent, MmContent content, String contact, byte[] thumbnail) {
		super(parent, contact);
		
		this.content = content;
		this.thumbnail = thumbnail;
	}
	
	/**
	 * Return the contribution ID
	 * 
	 * @return Contribution ID
	 */
	public String getContributionID() {
		return contributionId;
	}	
	
	/**
	 * Set the contribution ID
	 * 
	 * @param id Contribution ID
	 */
	public void setContributionID(String id) {
		this.contributionId = id;
	}
	
	/**
	 * Returns the content
	 * 
	 * @return Content 
	 */
	public MmContent getContent() {
		return content;
	}
	
	/**
	 * Returns the list of participants involved in the transfer
	 * 
	 * @return List of participants 
	 */
	public ListOfParticipant getParticipants() {
		return participants;
	}
	
	/**
	 * Set the content
	 * 
	 * @param content Content  
	 */
	public void setContent(MmContent content) {
		this.content = content;
	}	
	
	/**
	 * Returns the "file-transfer-id" attribute
	 * 
	 * @return String
	 */
	public String getFileTransferId() {
		return "" + System.currentTimeMillis();
	}	
	
	/**
	 * File has been transfered
	 */
	public void fileTransfered() {
		this.fileTransfered = true;
		
	}
	
	/**
	 * Is file transfered
	 * 
	 * @return Boolean
	 */
	public boolean isFileTransfered() {
		return fileTransfered; 
	}
	
	/**
	 * File has been paused
	 */
	public void fileTransferPaused() {
		this.fileTransferPaused = true;
	}
	
	/**
	 * File is resuming
	 */
	public void fileTransferResumed() {
		this.fileTransferPaused = false;
	}
	
	/**
	 * Is file transfer paused
	 * 
	 * @return fileTransferPaused
	 */
	public boolean isFileTransferPaused() {
		return fileTransferPaused; 
	}

	/**
	 * Returns max file sharing size
	 * 
	 * @return Size in bytes
	 */
	public static int getMaxFileSharingSize() {
		return RcsSettings.getInstance().getMaxFileTransferSize()*1024;
	}

    /** M: ftAutAccept @{ */
    public boolean shouldAutoAccept() {
        return RcsSettings.getInstance().isFileTransferAutoAccepted();
    }

    /** @} */

    /**
     * Returns the thumbnail
     * 
     * @return Thumbnail
     */
    public byte[] getThumbnail() {
    	return thumbnail;
    }

    /**
     * Set the thumbnail
     *
     * @param Thumbnail
     */
    public void setThumbnail(byte[] thumbnail) {
        this.thumbnail = thumbnail;
    }

    /**
     * Returns the thumbnail
     * 
     * @return Thumbnail
     */
    public String getThumbUrl() {
    	return thumbUrl;
    }

    /**
     * Set the thumbnail
     *
     * @param Thumbnail
     */
    public void setThumbUrl(String thumbUrl) {
        this.thumbUrl = thumbUrl;
    }

    /**
     * Get session state
     *
     * @return State 
     * @see SessionState
     */
    public abstract int getSessionState();
}
