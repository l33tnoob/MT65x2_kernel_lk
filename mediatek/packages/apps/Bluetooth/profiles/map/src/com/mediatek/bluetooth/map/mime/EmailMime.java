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

package com.mediatek.bluetooth.map.mime;
import com.mediatek.bluetooth.map.Email;
import com.mediatek.bluetooth.map.mime.MimeBase.MimeHeaders;
import com.mediatek.bluetooth.map.mime.MimeBase.MimeBody;
import com.mediatek.bluetooth.map.mime.MimeBase.MimeAttachment;
import com.mediatek.bluetooth.map.MAP;
import java.util.ArrayList;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ContentUris;
import android.util.Log;
import android.database.Cursor;
import android.net.Uri;
import android.database.sqlite.SQLiteException;
import com.mediatek.xlog.Xlog;
public class EmailMime extends MimeBase {
	
	private static final String[] MESSAGE_PROJECTION = new String[] {
			Email.MessageColumns.ID,			//0
			Email.MessageColumns.SUBJECT,		// 1 
			Email.MessageColumns.TIMESTAMP,		// 2
			Email.MessageColumns.FROM_LIST,    // 3
			Email.MessageColumns.REPLY_TO_LIST,	//  4 
			Email.MessageColumns.TO_LIST,		// 5
			Email.MessageColumns.FLAG_ATTACHMENT,// 6
			Email.MessageColumns.FLAG_LOADED,//7
			Email.MessageColumns.FLAG_READ,//8
			Email.MessageColumns.MESSAGE_ID,//9			
			Email.MessageColumns.BCC_LIST,// 10
			Email.MessageColumns.CC_LIST// 11
		//	Email.MessageColumns.SIZE,// 12
	};
		
		
	private final int COLUMN_ID = 0;
	private final int COLUMN_SUBJECT		= 1;
	private final int COLUMN_TIMESTATP		= 2;
	private final int COLUMN_FROM			= 3;
	private final int COLUMN_REPLAY			= 4;
	private final int COLUMN_TO				= 5;
	private final int COLUMN_FLAG_ATTACHMENT		= 6;
	private final int COLUMN_LOADED			= 7;
	private final int COLUMN_READ			= 8;
	private final int COLUMN_MESSAGE_ID		= 9;
	private final int COLUMN_BCC			= 10;
	private final int COLUMN_CC				= 11;

	private static final String[] BODY_PROJECTION = new String[]{
        Email.BodyColumns.HTML_CONTENT,
        Email.BodyColumns.TEXT_CONTENT,
        Email.BodyColumns.HTML_REPLY,
        Email.BodyColumns.TEXT_REPLY,    
        Email.BodyColumns.INTRO_TEXT,
	};
	private static final int BODY_HTML_CONTENT_COLUMN		= 0;
	private static final int BODY_TEXT_CONTENT_COLUMN		= 1;
	private static final int BODY_HTML_REPLY_COLUMN			= 2;
	private static final int BODY_TEXT_REPLY_COLUMN			= 3;
	private static final int BODY_INTRO_COLUMN				= 4;
		
	private static final String[] ATTACHMENT_PROJECTION = new String[] {
      Email.AttachmentColumns.ID, 
			Email.AttachmentColumns.FILENAME,
			Email.AttachmentColumns.MIME_TYPE,
			Email.AttachmentColumns.SIZE, 
			      Email.AttachmentColumns.CONTENT_ID, 
            Email.AttachmentColumns.CONTENT_URI,
            Email.AttachmentColumns.MESSAGE_KEY, 
            Email.AttachmentColumns.LOCATION, 
            Email.AttachmentColumns.ENCODING,
            Email.AttachmentColumns.CONTENT, 
            Email.AttachmentColumns.FLAGS, 
            Email.AttachmentColumns.CONTENT_BYTES
    };
	public static final int CONTENT_ID_COLUMN = 0;
	public static final int CONTENT_FILENAME_COLUMN = 1;
	public static final int CONTENT_MIME_TYPE_COLUMN = 2;
	public static final int CONTENT_SIZE_COLUMN = 3;
	public static final int CONTENT_CONTENT_ID_COLUMN = 4;
	public static final int CONTENT_CONTENT_URI_COLUMN = 5;
	public static final int CONTENT_MESSAGE_ID_COLUMN = 6;
	public static final int CONTENT_LOCATION_COLUMN = 7;
	public static final int CONTENT_ENCODING_COLUMN = 8;
	public static final int CONTENT_CONTENT_COLUMN = 9; // Not currently used
	public static final int CONTENT_FLAGS_COLUMN = 10;
	public static final int CONTENT_CONTENT_BYTES_COLUMN = 11;
	
	private long mId;
	
	public EmailMime(ContentResolver resolver, long id){
		super(resolver, MSG_TYPE_EMAIL);
		mId = id;
		loadHeaders();
		loadBody();
		loadAttachment();
	}
	
	private void loadHeaders(){
		Uri uri = ContentUris.withAppendedId(Email.MESSAGE_URI, mId);
		Cursor messageCursor;
		if (mContentResolver == null) {
			log("mContentResolver is null");
			return;
		}
		try{
			messageCursor = mContentResolver.query(uri,
						MESSAGE_PROJECTION, null, null, null);
		} catch (SQLiteException e) {
				e.printStackTrace();
				return;
		}
		if (messageCursor == null || !messageCursor.moveToFirst()) {
			log("no email message record for the id:" + mId);
			return;
		}
		mHeaders.mId = mId;
		mHeaders.mBcc = messageCursor.getString(COLUMN_BCC);
		mHeaders.mCc = messageCursor.getString(COLUMN_CC);
		mHeaders.mSubject = messageCursor.getString(COLUMN_SUBJECT);
		mHeaders.mTo = messageCursor.getString(COLUMN_TO);
		mHeaders.mTimeStamp = messageCursor.getLong(COLUMN_TIMESTATP);
		mHeaders.mReplyTo = messageCursor.getString(COLUMN_REPLAY);
		mHeaders.mFrom = messageCursor.getString(COLUMN_FROM);
		mHeaders.mVersion = "1.0";
		mHeaders.mMsgId = Integer.toString(messageCursor.getInt(COLUMN_MESSAGE_ID));
		messageCursor.close();
	}

	
	private void loadBody(){
		Uri uri = ContentUris.withAppendedId(Email.BODY_URI, mId);
		Cursor bodyCursor;
		try{
			bodyCursor = mContentResolver.query(uri,
								BODY_PROJECTION, null, null, null);
		} catch (SQLiteException e) {
				e.printStackTrace();
				return;
		}
		if (bodyCursor == null || !bodyCursor.moveToFirst()) {
			log("no email body record for the id:" + mId);
			return;
		}

		mBody.mTextContent 	= bodyCursor.getString(BODY_TEXT_CONTENT_COLUMN);
		mBody.mHtmlContent 	= bodyCursor.getString(BODY_HTML_CONTENT_COLUMN);
		mBody.mTextReply		= bodyCursor.getString(BODY_TEXT_REPLY_COLUMN);
		mBody.mHtmlReply		= bodyCursor.getString(BODY_HTML_REPLY_COLUMN);
		mBody.mIntroText		= bodyCursor.getString(BODY_INTRO_COLUMN);
		bodyCursor.close();
	}

	private void loadAttachment() {
		ArrayList<MimeAttachment> attaches = new ArrayList<MimeAttachment>();
		Uri uri = ContentUris.withAppendedId(Email.ATTACHMENT_URI, mId);
		Cursor cursor;
        try {
			cursor = mContentResolver.query(uri, ATTACHMENT_PROJECTION,
                    null, null, null);
		} catch (SQLiteException e) {
				e.printStackTrace();
				return;
		}	
		if (cursor == null) {
			log("no email attachment record for the id" + mId);
			return;
		}
		while(cursor.moveToNext()) {
				MimeAttachment attach = new MimeAttachment();
				attach.mFileName= cursor.getString(CONTENT_FILENAME_COLUMN);
				attach.mName= attach.mFileName;
				attach.mMimeType = cursor.getString(CONTENT_MIME_TYPE_COLUMN);
				attach.mSize = cursor.getLong(CONTENT_SIZE_COLUMN);
				attach.mContentId = cursor.getString(CONTENT_CONTENT_ID_COLUMN);
				String uriStr = cursor.getString(CONTENT_CONTENT_URI_COLUMN);
				if (uriStr != null) {
					attach.mContentUri = Uri.parse(uriStr);
				} else {
					attach.mContentUri = null;
				}
				attach.mMessageKey = cursor.getLong(CONTENT_MESSAGE_ID_COLUMN);
				//set "attachment" defaultly
		//		attach.mLocation = cursor.getString(CONTENT_LOCATION_COLUMN);
				attach.mEncoding = cursor.getString(CONTENT_ENCODING_COLUMN);
				attach.mContent = cursor.getString(CONTENT_CONTENT_COLUMN);
				attach.mFlags = cursor.getInt(CONTENT_FLAGS_COLUMN);
				attach.mContentBytes = cursor.getBlob(CONTENT_CONTENT_BYTES_COLUMN);	
				attaches.add(attach);
		}
		cursor.close();
		mAttachment = attaches.toArray(new MimeBase.MimeAttachment[attaches.size()]);
   }
	private void log(String info){
		if (null != info){
			Xlog.v(TAG, info);
		}
	}

}