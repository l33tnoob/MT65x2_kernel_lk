/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth.map;

import android.net.Uri;

public class Email{
	public static final String  EMAIL_AUTHORITY = "com.android.email.provider";
   	public static final Uri CONTENT_URI = Uri.parse("content://" + EMAIL_AUTHORITY);
	public static final Uri MESSAGE_URI = Uri.parse(CONTENT_URI+"/message");
	public static final Uri ACCOUNT_URI = Uri.parse(CONTENT_URI+"/account");
	public static final Uri BODY_URI = Uri.parse(CONTENT_URI+"/body");
	public static final Uri ATTACHMENT_URI = Uri.parse(CONTENT_URI+"/attachment");
	public static final Uri MESSAGE_ID_URI = Uri.parse(CONTENT_URI + "/attachment/message");
	public static final Uri MAILBOX_URI = Uri.parse(CONTENT_URI + "/mailbox");

	/*************/
	/*Message provider flag_load definition*/
	public static final int FLAG_LOADED_UNLOADED = 0;
	public static final int FLAG_LOADED_COMPLETE = 1;
	public static final int FLAG_LOADED_PARTIAL = 2;
	public static final int FLAG_LOADED_DELETED = 3;
	/*************/

	/*************/
	// Values used in Flag_Read
    public static final int UNREAD = 0;
    public static final int READ = 1;
		/*************/

	public static final int isAccoutDefault = 1;

		
	public class AccountColumns{
		public static final String ID = "_id";
        // The display name of the account (user-settable)
        public static final String DISPLAY_NAME = "displayName";
        // The email address corresponding to this account
        public static final String EMAIL_ADDRESS = "emailAddress";
        // A server-based sync key on an account-wide basis (EAS needs this)
        public static final String SYNC_KEY = "syncKey";
        // The default sync lookback period for this account
        public static final String SYNC_LOOKBACK = "syncLookback";
        // The default sync frequency for this account, in minutes
        public static final String SYNC_INTERVAL = "syncInterval";
        // A foreign key into the account manager, having host, login, password, port, and ssl flags
        public static final String HOST_AUTH_KEY_RECV = "hostAuthKeyRecv";
        // (optional) A foreign key into the account manager, having host, login, password, port,
        // and ssl flags
        public static final String HOST_AUTH_KEY_SEND = "hostAuthKeySend";
        // Flags
        public static final String FLAGS = "flags";
        // Default account
        public static final String IS_DEFAULT = "isDefault";
        // Old-Style UUID for compatibility with previous versions
        public static final String COMPATIBILITY_UUID = "compatibilityUuid";
        // User name (for outgoing messages)
        public static final String SENDER_NAME = "senderName";
        // Ringtone
        public static final String RINGTONE_URI = "ringtoneUri";
        // Protocol version (arbitrary string, used by EAS currently)
        public static final String PROTOCOL_VERSION = "protocolVersion";
        // The number of new messages (reported by the sync/download engines
        public static final String NEW_MESSAGE_COUNT = "newMessageCount";
        // Flags defining security (provisioning) requirements of this account
        public static final String SECURITY_FLAGS = "securityFlags";
        // Server-based sync key for the security policies currently enforced
        public static final String SECURITY_SYNC_KEY = "securitySyncKey";
        // Signature to use with this account
        public static final String SIGNATURE = "signature";

	}

	public class MessageColumns {

		/*****************/
		/*message provider    */
		/*****************/
		public static final String ID = "_id";
		// Basic columns used in message list presentation
		// The name as shown to the user in a message list
		public static final String DISPLAY_NAME = "displayName";
		// The time (millis) as shown to the user in a message list [INDEX]
		public static final String TIMESTAMP = "timeStamp";
        // Message subject
        public static final String SUBJECT = "subject";
        // Boolean, unread = 0, read = 1 [INDEX]
        public static final String FLAG_READ = "flagRead";
        // Load state, see constants below (unloaded, partial, complete, deleted)
        public static final String FLAG_LOADED = "flagLoaded";
        // Boolean, unflagged = 0, flagged (favorite) = 1
        public static final String FLAG_FAVORITE = "flagFavorite";
        // Boolean, no attachment = 0, attachment = 1
        public static final String FLAG_ATTACHMENT = "flagAttachment";
        // Bit field for flags which we'll not be selecting on
        public static final String FLAGS = "flags";

        // Sync related identifiers
        // Any client-required identifier
        public static final String CLIENT_ID = "clientId";
        // The message-id in the message's header
        public static final String MESSAGE_ID = "messageId";

        // References to other Email objects in the database
        // Foreign key to the Mailbox holding this message [INDEX]
        public static final String MAILBOX_KEY = "mailboxKey";
        // Foreign key to the Account holding this message
        public static final String ACCOUNT_KEY = "accountKey";

        // Address lists, packed with Address.pack()
        public static final String FROM_LIST = "fromList";
        public static final String TO_LIST = "toList";
        public static final String CC_LIST = "ccList";
        public static final String BCC_LIST = "bccList";
        public static final String REPLY_TO_LIST = "replyToList";

        // Meeting invitation related information (for now, start time in ms)
        public static final String MEETING_INFO = "meetingInfo";
		
        public static final String SIZE = "size";
		/*************/

	}
		
		/*****************/
		/*attachment provider*/
		/*****************/
	public class AttachmentColumns {
		public static final String ID = "_id";
        // The display name of the attachment
    	public static final String FILENAME = "fileName";
		// The mime type of the attachment
		public static final String MIME_TYPE = "mimeType";
		// The size of the attachment in bytes
		public static final String SIZE = "size";
		// The (internal) contentId of the attachment (inline attachments will have these)
		public static final String CONTENT_ID = "contentId";
		// The location of the loaded attachment (probably a file)
		public static final String CONTENT_URI = "contentUri";
		// A foreign key into the Message table (the message owning this attachment)
		public static final String MESSAGE_KEY = "messageKey";
		// The location of the attachment on the server side
		// For IMAP, this is a part number (e.g. 2.1); for EAS, it's the internal file name
		public static final String LOCATION = "location";
		// The transfer encoding of the attachment
		public static final String ENCODING = "encoding";
		// Not currently used
		public static final String CONTENT = "content";
		// Flags
		public static final String FLAGS = "flags";
		// Content that is actually contained in the Attachment row		
		public static final String CONTENT_BYTES = "content_bytes";
		/*attachment provider end*/
	}

		
	
	public interface BodyColumns {
        public static final String ID = "_id";
        // Foreign key to the message corresponding to this body
        public static final String MESSAGE_KEY = "messageKey";
        // The html content itself
        public static final String HTML_CONTENT = "htmlContent";
        // The plain text content itself
        public static final String TEXT_CONTENT = "textContent";
        // Replied-to or forwarded body (in html form)
        public static final String HTML_REPLY = "htmlReply";
        // Replied-to or forwarded body (in text form)
        public static final String TEXT_REPLY = "textReply";
        // A reference to a message's unique id used in reply/forward.
        // Protocol code can be expected to use this column in determining whether a message can be
        // deleted safely (i.e. isn't referenced by other messages)
        public static final String SOURCE_MESSAGE_KEY = "sourceMessageKey";
        // The text to be placed between a reply/forward response and the original message
        public static final String INTRO_TEXT = "introText";
    }

	 public interface MailboxColumns {
        public static final String ID = "_id";
        // The display name of this mailbox [INDEX]
        static final String DISPLAY_NAME = "displayName";
        // The server's identifier for this mailbox
        public static final String SERVER_ID = "serverId";
        // The server's identifier for the parent of this mailbox (null = top-level)
        public static final String PARENT_SERVER_ID = "parentServerId";
        // A foreign key to the Account that owns this mailbox
        public static final String ACCOUNT_KEY = "accountKey";
        // The type (role) of this mailbox
        public static final String TYPE = "type";
        // The hierarchy separator character
        public static final String DELIMITER = "delimiter";
        // Server-based sync key or validity marker (e.g. "SyncKey" for EAS, "uidvalidity" for IMAP)
        public static final String SYNC_KEY = "syncKey";
        // The sync lookback period for this mailbox (or null if using the account default)
        public static final String SYNC_LOOKBACK = "syncLookback";
        // The sync frequency for this mailbox (or null if using the account default)
        public static final String SYNC_INTERVAL = "syncInterval";
        // The time of last successful sync completion (millis)
        public static final String SYNC_TIME = "syncTime";
        // Cached unread count
        public static final String UNREAD_COUNT = "unreadCount";
        // Visibility of this folder in a list of folders [INDEX]
        public static final String FLAG_VISIBLE = "flagVisible";
        // Other states, as a bit field, e.g. CHILDREN_VISIBLE, HAS_CHILDREN
        public static final String FLAGS = "flags";
        // Backward compatible
        public static final String VISIBLE_LIMIT = "visibleLimit";
        // Sync status (can be used as desired by sync services)
        public static final String SYNC_STATUS = "syncStatus";
    }

	//mailbox 
		public static final int INBOX = 0;
		// Types of mailboxes
		// Holds mail (generic)
		public static final int MAIL = 1;
		// Parent-only mailbox; holds no mail
		public static final int PARENT = 2;
		// Holds drafts
		public static final int DRAFTS = 3;
		// The local outbox associated with the Account
		public static final int OUTBOX = 4;
		// Holds sent mail
		public static final int SENT = 5;
		// Holds deleted mail
		public static final int TRASH = 6;
		// Holds junk mail
		public static final int JUNK = 7;

	/**********/
	/** event to notify  map service**/
	public final int ACCOUNT_DELETED = 0;
	/**********/
	
	public static final int SEND_AND_SAVE = 0;
	public static final int SEND_NO_SAVE = 1;	
	public static final int NO_SEND_AND_SAVE = 2;
	
	/*query order */
	public static final String KEY_TIMESTAMP_DESC = MessageColumns.TIMESTAMP + " desc";
	/*end */
};