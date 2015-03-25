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
import java.io.OutputStream;
import java.io.PushbackInputStream;
import java.io.IOException;

import android.util.Log;
import com.mediatek.xlog.Xlog;

//for Email, \r\n is the line break, so the unit to read turn 
public class MimeInputStream {
	private static final String TAG = "MimeInputStream";
	private static final String LINE_BREAK = "\r\n"; 
//	public static final int MAXIUM_LINE_LENGTH = 80;
	public static final int MAXIUM_LINE_LENGTH = 998;

	
	private byte[] mCurrentLine;
//	private int lineSize = 0;
	private int currentPosition = 0;
	private PushbackInputStream mInput;
//	private StringBuilder mCurrentLine;
	
	MimeInputStream(InputStream in){
		mInput = new PushbackInputStream(in);		
		mCurrentLine = new byte[MAXIUM_LINE_LENGTH];
	}

	//read a line seperateb by \r\n
	public int readLine(byte[] cache){
		byte current = 0;
		byte previous = 0; 
		log("readLine()");
		currentPosition = -1;

		if (cache == null) {
			return 0;
		}
		try{
			current = previous = (byte)mInput.read();
		} catch (IOException e){
				//
		}
		while (current != -1 && currentPosition < MAXIUM_LINE_LENGTH){
			mCurrentLine[++currentPosition] = current;
			if (previous == '\r' && current == '\n') {
				//delete CRLF
				mCurrentLine[currentPosition--] = 0;
				mCurrentLine[currentPosition] = 0;
				break;
			} 
			previous = current;
			try{
				current = (byte)mInput.read();
			} catch (IOException e){
				//
				log("fail to read byte");
				return -1;
			}
		}
		if (currentPosition > 0){
			System.arraycopy(mCurrentLine, 0, cache, 0, currentPosition);
			log(new String(cache, 0, currentPosition));
		}
		return currentPosition;
	
	}

	public void unreadLine(){
		log("unreadLine():currentPosition is "+currentPosition);
		if (currentPosition <= 0){
			return;
		}
		try{
			mInput.unread(mCurrentLine, 0, currentPosition-1);
		} catch (IOException e) {
			log(e.toString());
		}
		currentPosition = 0;
		return;
	}
	public void unreadLine(byte[] cache, int offset, int len){
		log("unreadLine()"+"offset:"+offset+", len:"+len);
		if (cache == null){
			return;
		}
		try{
			mInput.unread(cache, offset, len);
		} catch (IOException e) {
			log(e.toString());
		}
		return;
	}

	public int readByteWithoutMark(){
		int value = 0;
		try{
			value = mInput.read();
			mInput.unread(value);
		} catch (IOException e){
				//
		}
		return value;
			
	}

	public boolean transferData(OutputStream out, int offset, int length){
		log("transferData():offset is"+offset+", length is"+ length);
		boolean ret = false;
		int index = -1;
		if (out == null || length == 0){
			ret = false;
			return ret;
		}
		try{
		do {
			index ++;
			int value = mInput.read();
			if (index < offset){
				continue;
			} else {
				if(value == -1){
					break;
				} else {
					out.write(value);
				}
			}			
		} while(index < length);
		} catch (IOException e) {
			log(e.toString());
		}
		return ret;
	}

	private void log(String info) {
		if (null != info){
			Xlog.v(TAG, info);
		}
	}
}
