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
import com.mediatek.bluetooth.map.util.*;
import com.mediatek.bluetooth.map.MAP;
public class MessageListRequest {
		private int mMasId;
		private String mAddr;
		private String mChildFolder; // this field should be removed
		private int mListSize;
		private int mListOffset;
		private int mMaxSubjectLen;

		//the mask seems not usefull
		//even client reset a mask to 0,
		//MSE can still send the relevant field
		private int mMask;
		
		private	int mType;                  //message type
		private String mStartTime;
		private String mEndTime;
		private int mReadStatus;
		private String mRecipient;
		private String mOrignator;	
		private int mPriority;  // not supported

		private String mFolder;

		private boolean isOccupied = false;

		public MessageListRequest(int id){
			mMasId = id;
		}
		public void setMask(int value) {
			mMask = value;				
		}

		public void setDefaultMask() {
			mMask=0;
		}

		public void setMessageType(int type){
			mType = type;
		}
		public void setSize(int size){
			mListSize = size;
		}

		//the message list object can not be avaible until previous request has been processed
		//so when get the message list, pls first to the check the object is being occupied
		public boolean isOccupied() {
			return isOccupied;
		}

		public void declineListOffset(int offset){
			mListOffset -= offset;
			if (mListOffset < 0) {
				mListOffset = 0;
			}
		}/*
		public void declineListSize(int size){
			mListSize -= size;
			if (mListSize < 0) {
				mListSize = 0;
			}
		}
		*/

		public synchronized boolean setOccupied(boolean occupied) {
			/*when the object is not occupied, accept the opration*/
			/*but when object is occupied, only the release opration is accepted.*/
			if (!isOccupied) {
				isOccupied = occupied;
			} else if (!occupied) {
				isOccupied = occupied;
			} else {
				return false;
			}
			return true;
		}

		/*Message type filter definition in spec:		*/
		/*			if 1 is set, filter out this type	*/
		/*			if 0 is set, No filter, get this type	*/
		/*So transit the filters to supported types	*/
		public int getMessageType () {			
			return (~mType) & 0x00FF;
		}
		public int geMask () {
			return mMask;
		}

		public long getStartTime() {			
			return UtcUtil.revertUtcToMillis(mStartTime);
		}
		
		public long getEndTime() {
			return UtcUtil.revertUtcToMillis(mEndTime);
		}

		public String getRecipient(){
			return mRecipient;
		}
		public String getOrignator(){
			return mOrignator;
		}

		//return last level folder name
		//TODO: check neccessary to tranfer string or int
		public String getFolder (){
			if (mFolder == null) {
				return null;
			}
			
			int lastIndex = mFolder.lastIndexOf("/");
		//	lastIndex = (lastIndex < 0)? 0:lastIndex;
			return mFolder.substring(lastIndex+1);
		}
		public int getReadStatus() {
			return mReadStatus;
		}

		public int getMaxSubjectLen(){
			if (mListSize == 0) {
				mMaxSubjectLen = MAP.MAX_SUBJECT_LEN;
			}
			return mMaxSubjectLen;
		}

		public int getListSize(){
			return mListSize;
		}

		public int getListOffset(){
			return mListOffset;
		}

		public String getAddress(){
			return mAddr;
		}
		
		public int getPriority() {
			return mPriority;
		}
		
		
	}

