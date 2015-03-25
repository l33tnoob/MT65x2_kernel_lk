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
import android.net.Uri;
import android.util.Log;

import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Locale;
import java.util.Date;


import com.mediatek.bluetooth.map.mime.MimeBase.MimeHeaders;
import com.mediatek.bluetooth.map.mime.MimeBase.MimeBody;
import com.mediatek.bluetooth.map.mime.MimeBase.MimeAttachment;
import com.mediatek.bluetooth.map.Address;
import com.mediatek.xlog.Xlog;

//a simple implement for email parser
public class MimeParser implements MimeListener{
	private static final String TAG = "MimeParser";

	private static final SimpleDateFormat DATE_FORMAT =
        new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss Z", Locale.US);
	
	private MimeBase mMime; 
	private MimeHeaders mHeaders;
	private MimeBody mBody;
	private MimeAttachment mAttachment;
	private MimeInputSeparator mSeparator;
	private String mBoundary;
	private int mSize = 0;

	private ArrayList<MimeAttachment> attachList;

	private boolean isMessageHeaderEnd = false;

	private StringBuilder bodyCache;

	public MimeParser(InputStream in, MimeBase mime) {
		mSeparator = new MimeInputSeparator(in, this);
		mMime = mime;
		mHeaders = mime.getHeader();
		mBody = mime.getBody();
		attachList = new ArrayList<MimeAttachment>();
	//	mAttachment = mime.getA
		bodyCache = new StringBuilder();
	}

	public void parse(){		
		mSeparator.separate();
	}

	public void onMessageStart(){
		log("onMessageStart");
	}
	public void onMessageEnd(){
		log("onMessageEnd");
		mMime.mAttachment = attachList.toArray(new MimeAttachment[0]);
	}
	public String onRequestBoundary(){
		log("onRequestBoundary");
		return mBoundary;
	}
	public int onRequestSize(){
		log("onRequestSize():"+mSize);
		return mSize;
	}


	public void onHeaderStart(){
		log("onHeaderStart");
	}
	public void onHeaderEnd(){
		log("onHeaderEnd");
		isMessageHeaderEnd = true;
	}
	public void onHeaderFail(){
		log("onHeaderFail");
	}
	public void onHeaderFieldAdd(String name, String value){
		log("onHeaderFieldAdd:"+ name+", "+value);
		if (name == null) {
			log("error, the header name is null");
			return;
		}
		if (name.equals(MimeContent.DATE)) {
			mHeaders.mTimeStamp = processTimeStamp(value);
		} else if (name.equals(MimeContent.SUBJECT)) {
			mHeaders.mSubject = value;
		} else if (name.equals(MimeContent.MESSAGE_ID)) {
			mHeaders.mMsgId = value;
		} else if (name.equals(MimeContent.FROM)) {
			mHeaders.mFrom = processAddress(value);
		} else if (name.equals(MimeContent.CC)) {
			mHeaders.mCc = processAddress(value);
		} else if (name.equals(MimeContent.TO)) {
			mHeaders.mTo = processAddress(value);
			log("value is "+value+", to is "+ mHeaders.mTo);
		} else if (name.equals(MimeContent.BCC)) {
			mHeaders.mBcc = processAddress(value);
		} else if (name.equals(MimeContent.REPLY_TO)) {
			mHeaders.mReplyTo = processAddress(value);
		} else if (name.equals(MimeContent.MIME_VERSION)) {
			mHeaders.mVersion = value;
		} else if (name.equals(MimeContent.CONTENT_TYPE)) {
			processContentType(value);
		} else if (name.equals(MimeContent.CONTENT_ID)) {
			processContentID(value);
		} else if (name.equals(MimeContent.CONTENT_DISPOSITION)) {
			processContentDisposition(value);
		} else if (name.equals(MimeContent.CONTENT_TRANSFER_ENCONDING)) {
			processContentEncoding(value);
		} else if (name.equals(MimeContent.CONTENT_LOCATION)) {
			processContentLocation(value);
		} else {
			log("unsupported header field:"+name);
		}		
	}
	
	public void onBodyStart(){
		log("onBodyStart");
		mAttachment = mMime.new MimeAttachment();
		mSize = 0;
	}
	public void onBodyEnd(){
		log("onBodyEnd");
		attachList.add(mAttachment);
		mAttachment = null;
	}
	public void onBodyFail(){
	}
	public void onContentAdd(byte[] value){
		log("onContentAdd:"+ (value == null ? 0: value.length));
		mAttachment.mContentBytes = value;		
	}
	

	private long processTimeStamp(String time) {
		if (time == null){
			return 0;
		}
		try{
			Date date = DATE_FORMAT.parse(time);
			return date.getTime();
		} catch (IllegalArgumentException e) {
			log (e.toString());
		} catch (ParseException e) {
			log (e.toString());
		}
		return 0;
	}

	private String processAddress(String value){
		return value == null ? null : Address.getFormatAddress(value);
	}
	private void processContentType(String value){
		String contentType = null;
		String type = null;
		String name = null;
		String charset = null;
		if (value == null) {
			return;
		}
		String[] elements = value.split(";");
		contentType = elements[0];
		for (int elementsindex = 1; elementsindex < (elements.length); elementsindex ++) {
	//	if (elements.length > 1) {
			String[] boundary = elements[elementsindex].split("=");
			for (int index = 0; boundary != null && index < boundary.length;index ++,index++){
				if (boundary[index].trim().indexOf(MimeContent.BOUNDARY) != -1){
					//remove qoute
					String bnd = boundary[index+1];
					if (bnd.length()>2){
						mBoundary = bnd.substring(1, bnd.length()-1).trim();
					}
				} else if (boundary[index].trim().indexOf(MimeContent.NAME)!= -1){
					name =  boundary[index+1].trim();
				} else if (boundary[index].trim().toLowerCase().indexOf(MimeContent.CHARSET)!= -1){
					charset = boundary[index+1].trim();
				} else {
					log("unknown type:"+boundary[index]);
				}
			}
			
		}		

		if(!isMessageHeaderEnd){
			mHeaders.mMiltipartType = contentType;
		} else {
			mAttachment.mMimeType = contentType;
			mAttachment.mName = name;
			mAttachment.mCharset = charset;
		}
	}

	private void processContentID(String id){
		if (mAttachment == null) {
			return;
		}
		if(isMessageHeaderEnd){
			mAttachment.mContentId = id;
		}
	}

	private void processContentDisposition (String value){
		String disposition = null;
		String fileName = null;
		if (mAttachment == null) {
			return;
		}

		String[] elements = value.split(";");
		disposition = elements[0].trim();
		for (int elementsindex = 1; elementsindex < (elements.length); elementsindex ++) {
	//	if (elements.length > 1) {
			String[] sub = elements[elementsindex].split("=");
			for (int index = 0; sub != null && index < sub.length;index ++,index++){
				if (sub[index].trim().toLowerCase().indexOf(MimeContent.FILE_NAME) != -1){					
					fileName = sub[index+1].trim();
				} else if (sub[index].trim().toLowerCase().indexOf(MimeContent.SIZE) != -1){
					mSize = Integer.parseInt(sub[index+1].trim());
					log("mSize is "+mSize);
				}
			}			
		}
		
		
		if(isMessageHeaderEnd){
		} else {
			mAttachment.mLocation = disposition;
			mAttachment.mFileName = fileName;
		} 

	}
	private void processContentEncoding(String encoding){
		if (mAttachment == null) {
			return;
		}
		if(isMessageHeaderEnd){
		    mAttachment.mEncoding = encoding;
		} 
	}

	private void processContentLocation(String location){
		if (mAttachment == null) {
			return;
		}
		if(isMessageHeaderEnd){	
			mAttachment.mContentLocation = location;
		}
	}
	private void log(String info) {
		if (null != info){
			Xlog.v(TAG, info);
		}
	}
}
