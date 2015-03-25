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

package com.mediatek.bluetooth.map.cache;
import java.util.ArrayList;
import java.util.Arrays;
import com.mediatek.bluetooth.map.util.*;

public class MessageListObject {
	private boolean mNewMessage ;
	private String mCurrentTime;
	private int mSize;
	private ArrayList<MessageItem> mMessageItems;
	private boolean isOccupied;

	public MessageListObject(){
		reset();
	}
	public synchronized void reset(){
		if (mMessageItems == null) {
			mMessageItems = new ArrayList<MessageItem>();
		} else {
			mMessageItems.clear();
		}
		
		mCurrentTime = UtcUtil.getCurrentTime();
		mSize = 0;
		isOccupied = false;
		mNewMessage = false;
	}

	public synchronized boolean addSize(int size) {		
			mSize += size;
			return true;
	}	
	public synchronized boolean setNewMessage() {
	//	if (!isOccupied) {
			if (!mNewMessage) {
				mNewMessage = true;
			}
			isOccupied = true;
		//	Time time = new Time();
	//		time.set(System.getCurrentTime());
	//		mCurrentTime = time.toString();
			return true;			
	//	} else {
//return false;
	//	}
	}
	public synchronized boolean addMessageItem(MessageItem item) {
	//	if (isOccupied) {
			if (item != null) {
				mMessageItems.add(item);
			//	mSize += 1;
			}
			return true;
	//	} else {
	//		return false;
	//	}
	}

	
	public synchronized MessageItem[] generateMessageItemArray(){
		return mMessageItems.toArray(new MessageItem[mMessageItems.size()]);
	}

	public int getCurrentSize(){
		return mSize;
	}
	public boolean isNewMessage(){
		return mNewMessage;
	}

	public boolean isAvailable() {
		return !isOccupied;
	}
	
}
