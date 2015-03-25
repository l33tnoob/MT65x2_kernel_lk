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
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ByteArrayOutputStream;
import android.util.Log;
import com.mediatek.xlog.Xlog;

import com.mediatek.bluetooth.map.mime.MimeInputStream;
//a simple implement for email parser
public class MimeInputSeparator {
	private final static String TAG = "MimeInputSeparator";

	private final static int BODY_START		= 0;
	private final static int BODY_CONTINUE  = 1;
	private final static int BODY_END		= 2;

	private final static int BOUNDARY_NO_MATCH		= 0;
	private final static int BOUNDARY_START  		= 1;
	private final static int BOUNDARY_CONTINUE  	= 2;
	private final static int BOUNDARY_FINAL			= 3;
	
	private MimeListener mListener; 
	private MimeInputStream mStream;
	private String mBoundary;
	private byte[] mCache;

	private int mBodyCurrentStatus;

	public MimeInputSeparator(InputStream in, MimeListener listener) {
		mStream = new MimeInputStream(in);
		mListener = listener;
		mCache = new byte[MimeInputStream.MAXIUM_LINE_LENGTH];
		mBodyCurrentStatus = -1;
	}

	public void separate(){
		mListener.onMessageStart();
		separateHeader();
		mBodyCurrentStatus = BODY_START;
		mBoundary = mListener.onRequestBoundary();	
		if (mBoundary != null){
			mBodyCurrentStatus = BOUNDARY_START;
		}
		while (!separateBody());
		mListener.onMessageEnd();
	}

	

	public void separateHeader(){
		String headerField;
		String key;
		String value; 
		mListener.onHeaderStart();

		
		while(true) {
			headerField = getUnfoldLine();
			if (!isValidHeaderLine(headerField)) {
				break;
			}
			ignoreComment(headerField);
			key = getHeaderName(headerField);
			value = getHeaderValue(headerField);
			mListener.onHeaderFieldAdd(key, value);
		} 
		mListener.onHeaderEnd();	
		
	}

	public boolean separateBody(){
		int position;
		String value;
		boolean isEOF = false;
		boolean isHeaderEnd = false;
		boolean isBodyStart = false; //set as true when find boundary or bounary is null
		int size = 0;
		ByteArrayOutputStream content = new ByteArrayOutputStream();
		mListener.onBodyStart();
		log("mBoundary is "+mBoundary);
		if (mBodyCurrentStatus == BOUNDARY_START){
			mBodyCurrentStatus = findBoundary();		
			
			if (mBodyCurrentStatus == BOUNDARY_FINAL) {
				mListener.onBodyEnd();
				return isEOF;
			}
		}	
		if (mBoundary != null){			
			separateHeader();
			size = mListener.onRequestSize();
		}

		while(true) {
				if (size > 0) {
					mStream.transferData(content, 0, size);		
					size = 0;
				}
				position = getLine(mCache);
				if (position == -1){
					isEOF = true;
					break;
				}
				int result = matchBoundary(mCache);
				if (result == BOUNDARY_CONTINUE || result == BOUNDARY_FINAL){
					isEOF = (result == BOUNDARY_FINAL);
					break;
				} 						
			
				if (position > 0){
					content.write(mCache, 0, position);			
				} 
			
		}

		//find boundary
	/*	do{
			if (mBoundary == null){
				break;
			}
			position = getLine(mCache);
			if (position == -1){
				isEOF = true;
				break;
			}
			value = new String (mCache, 0, position);
			log("value is "+value);
		} while(!isBoundaryBegin(value));

		if (isHeaderExist()) {
			separateHeader();
			isHeaderEnd = true;
		}
		
		//add content
		do {
			position = getLine(mCache);
			if (position == -1){
				isEOF = true;
				break;
			}
			value = new String (mCache, 0, position);
			log("value is "+value);
			
			if (position > 0){
				content.write(mCache, 0, position);			
			} else {
				log("position is 0");
			}
		} while(isBoundaryEnd(value));
		*/
		mListener.onContentAdd(content.toByteArray());
		mListener.onBodyEnd();
		return isEOF; 
	}
	

	private String getUnfoldLine(){
		StringBuilder sb = new StringBuilder(); 			
		int value;
		int position = 0;
		do {
			position = mStream.readLine(mCache);
			if (position <= 0) {
				break;
			}
			sb.append(new String (mCache, 0, position));
			value = mStream.readByteWithoutMark();
		} while (value != -1 && (value == ' ' || value == '\t' ));

		if (position == -1) {
			return null;
		} else {
			return sb.toString();	
		}
	}

	private int getLine(byte[] cache){		
		if (cache == null){
			return 0;
		}
		return mStream.readLine(cache);		
	}

	private int findBoundary(){
		int position;
		String value;
		int state = BOUNDARY_NO_MATCH;
		while (true){
			position = getLine(mCache);
			if (position == -1){
				state = BOUNDARY_FINAL;
				break;
			}

			state = matchBoundary(mCache);	
			if (state == BOUNDARY_CONTINUE || state == BOUNDARY_FINAL) {
				break;
			}
		}
		return state;
	}

	private int matchBoundary(byte[] cache){
		byte[] bnd = ("--"+mBoundary).getBytes();
		if (mBoundary == null || cache == null || bnd.length > cache.length){
			return BOUNDARY_NO_MATCH;
		}
		int index;
		for (index = 0; index < bnd.length; index++){
			if (bnd[index] != cache[index]){
				break;
			}
		}
		if (index < bnd.length) {
			return BOUNDARY_NO_MATCH;
		} else {
			if (cache.length >= bnd.length + 2 && cache[index] == '-' &&
				cache[index+1] == '-'){
				return BOUNDARY_FINAL;
			} else {
				return BOUNDARY_CONTINUE;
			}
		}
		
	}

	private boolean isBoundaryBegin(String boundary){
		
		return (boundary != null && boundary.indexOf("--"+mBoundary) != -1);
			
		
	}
	private boolean isBoundaryEnd(String boundary){
		return (boundary != null && boundary.indexOf("--"+mBoundary+"--") != -1);		
	}

	private boolean isValidHeaderLine(String line){
		return line != null && line.indexOf(":") > 0;			
	}

	private boolean isHeaderExist(){
		log("isHeaderExist()");
		boolean exist = false;
		int position = mStream.readLine(mCache);
		if (position == -1) {
			exist = false;
		} else if (position == 0) {
			exist = isHeaderExist();
		} else {
			String value = new String(mCache, 0, position);
			if (!isValidHeaderLine(value)) {
				exist = false;
			} else {
				exist = true;
			}
			mStream.unreadLine();
		}
		return exist;
		
	}

	private String getHeaderName(String field){
		if (field == null) {
			//throw exception
			return null;
		}
		int index = field.indexOf(":");
		if (index == -1 || index == 0) {
			return null;
		}
		return field.substring(0, index).trim();
		
	}
	private String getHeaderValue(String field){
		if (field == null) {
			//throw exception
			return null;
		}
		int index = field.indexOf(":");
		if (index == -1) {
			return null;
		}
		return field.substring(index+1).trim();
	}

	//ignore comments in headers 
	private String ignoreComment(String field){
		return field;
	}
	private void log(String info){
		if (null != info){
			Xlog.v(TAG, info);
		}
	}

	
	
}
