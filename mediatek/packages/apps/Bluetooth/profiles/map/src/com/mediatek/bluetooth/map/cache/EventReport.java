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

import com.mediatek.bluetooth.map.MAP;
import com.mediatek.bluetooth.map.util.HandleUtil;
import com.mediatek.xlog.Xlog;

//the class will store the event message

// the basic rule to access the object:
// 1. check if set value to the object 
// 2. access directly if get value from the object 
public class EventReport{
	private int mMasId;
	private int mEventType;
	private long mHandle;
	private String mFolder;
	private String mOldFolder;
	private int mMsgType;

	public EventReport(int masId) {
		mMasId = masId;
	}
	public boolean match(int masId)
	{
		return (mMasId == masId);
	}
	public boolean notifyNewMessageEvent(long id, int type, String folder){
		//fisrt check the valid value
		if (folder == null || !MAP.isMessageTypeValid(type)) {
			return false;
		}
		mEventType = MAP.EVENT_NEW_MESSAGE;
		mHandle = HandleUtil.getHandle(type, id);
		mMsgType = type;
		mFolder = folder;
		return true;
	}

	public boolean notifySendResult(long id, int type, String folder, int result){
		if (folder == null || !MAP.isMessageTypeValid(type)) {
			return false;
		}
		if (result == MAP.RESULT_OK) {
			mEventType = MAP.EVENT_SEND_SUCCESS;
		} else {
			mEventType = MAP.EVENT_SEND_FAILURE;
		}

		mHandle = HandleUtil.getHandle(type, id);
		mMsgType= type;
		mFolder = folder;
		return true;
	}
	public boolean notifyDeliverResult(long id, int type, String folder, int result){
		if (folder == null || !MAP.isMessageTypeValid(type)) {
			return false;
		}
		if (result == MAP.RESULT_OK) {
			mEventType = MAP.EVENT_DELIVERY_SUCCESS;
		} else {
			mEventType = MAP.EVENT_DELIVERY_FAILURE;
		}

		mHandle = HandleUtil.getHandle(type, id);
		mMsgType= type;
		mFolder = folder;
		return true;
	}
	public boolean notifyMessageDeleted(long id, int type, String folder){
		if (!MAP.isMessageTypeValid(type)) {
			return false;
		}
		mEventType = MAP.EVENT_MESSAGE_DELETED;
		mHandle = HandleUtil.getHandle(type, id);
		mMsgType = type;
		mFolder = folder;
		return true;
	}

	public boolean notifyMessageShifted(long id, int type, String oldFolder, String newFolder){
		if (!MAP.isMessageTypeValid(type)) {
			return false;
		} 
		mEventType = MAP.EVENT_MESSAGE_SHIFT;
		mMsgType = type;
		mFolder = newFolder;
		mOldFolder = oldFolder;
		mHandle = HandleUtil.getHandle(type, id);
		return true;
	}
	public boolean notifyMemoryStatus(int state){
		if (state == MAP.RESULT_ERROR) {
			mEventType = MAP.EVENT_MEMORY_FULL;
		} else {
			mEventType = MAP.EVENT_MEMORY_AVAILABLE;
		}
		return true;
	}

}
