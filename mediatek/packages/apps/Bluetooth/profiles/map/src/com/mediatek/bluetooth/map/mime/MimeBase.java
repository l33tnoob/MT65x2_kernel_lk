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

import java.io.InputStream;
import java.io.File;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.ByteArrayInputStream;
import android.net.Uri;
import android.content.Context;
import android.content.ContentResolver;
import android.content.res.AssetFileDescriptor;
import android.util.Log;

import com.google.android.mms.pdu.EncodedStringValue;

import com.mediatek.bluetooth.map.MAP;
import com.mediatek.xlog.Xlog;

public class MimeBase {
	protected static final String TAG = "MimeBase";
	
	public static int MSG_TYPE_EMAIL = MAP.MSG_TYPE_EMAIL;
	public static int MSG_TYPE_MMS = MAP.MSG_TYPE_MMS;

	public static String MULTIPART_MIX = "multipart/mixed";
	public static String MULTIPART_ALTERNATIVE = "multipart/alternative";
	public static String MULTIPART_RELATED = "multipart/related";

	public static final int FLAG_ICS_ALTERNATIVE_PART = 1<<0;	
	
	protected MimeHeaders mHeaders;
	protected MimeBody mBody;
	protected MimeAttachment[] mAttachment;
	protected int mMsgType;
	protected String mMultipartType;

	protected ContentResolver mContentResolver;
	
	public MimeBase(ContentResolver resolver, int type){
		mContentResolver = resolver;
		mMsgType = type;
		mHeaders = new MimeHeaders();
		mBody = new MimeBody();
		mAttachment = new MimeAttachment[0];
	}

	public class MimeHeaders{
		
		public long mId;
		public long mTimeStamp;
		public String mMsgId;
		public String mSubject;
		public String mFrom;
		public String mTo;
		public String mCc;
		public String mBcc;
		public String mReplyTo;
		public String mVersion;	
		public long mSize;

		public String mMiltipartType;
	}

	public class MimeBody{

		public String mTextContent;
		public String mHtmlContent;
		public String mTextReply;
		
		public String mHtmlReply;
		public String mIntroText;
		
	}

	public class MimeAttachment{
		public String mFileName;
		public String mName;
        public String mMimeType;
        public long mSize;
        public String mContentId;
        public Uri mContentUri;
        public long mMessageKey;
        public String mLocation;
        public String mEncoding;
        public String mContent; // Not currently used
        public int mFlags;
        public byte[] mContentBytes;
	public String mCharset;
		public String mContentLocation;
		
	public InputStream getContent(){
		InputStream inStream = null;
		long size;
    	AssetFileDescriptor fd;
		try {
		    if (mContentBytes != null) {
                	inStream = new ByteArrayInputStream(mContentBytes);
            	    } else {
				
					Uri fileUri = mContentUri;
					if (fileUri == null) {
						return null; 
					}
					fd = mContentResolver.openAssetFileDescriptor(fileUri, "rw"); 
					if(fd == null)
					{
						Log.e(TAG, "no file found for the Uri:"+ fileUri);
						return null;
					}

					size = fd.getLength();
					inStream = fd.createInputStream();
					if (size != mSize) {
						mSize = size;
					}
				} 
            } catch (IOException e) {
					e.printStackTrace();
					return null;
			}
			return inStream;
		}

		
	}

	public int getMsgType(){
		return mMsgType;
	}
	public MimeHeaders getHeader(){
		return mHeaders;
	}
	public MimeBody getBody(){
		return mBody;
	}
	public MimeAttachment[] getAttachment(){
		return mAttachment;
	}

	public MimeAttachment getAttachment(int index){
		if (index < mAttachment.length) {
			return mAttachment[index];
		} else {
			return null;
		}		
	}

	public void setAttachMent(MimeAttachment[] attachments) {
		mAttachment = attachments;
	}

	public boolean hasMultipart(){
		return mAttachment != null && mAttachment.length > 0;
	}
	//toDO: set mMiltipartType
	public String getMultipartType(){
		if ((mMsgType == MSG_TYPE_EMAIL) 
			&& (mAttachment != null) 
			&& (mAttachment.length == 1) 
			&& ((mAttachment[0].mFlags & FLAG_ICS_ALTERNATIVE_PART) != 0)) {
			return MULTIPART_ALTERNATIVE;
		} else {
			return MULTIPART_MIX;
		}
	}

	
}
