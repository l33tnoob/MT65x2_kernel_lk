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

import android.database.ContentObserver;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import android.os.Handler;
import com.mediatek.xlog.Xlog;	
public class MessageObserver extends ContentObserver{
	private static final String TAG = "MessageObserver";

	public interface ControllerHelper{
		void queryMessage(HashMap<Long, Integer> info);
		String revertMailboxType(int mailbox);
	}
	//message id(long)  <-->  message type(int)
	HashMap<Long, Integer> previousMessage;
	Object					mLock;

	private ControllerListener mListener;
	private ControllerHelper mHelper;
	private int mType;
	
	public MessageObserver(ControllerHelper helper, ControllerListener listener, int type) {
		super(new Handler());
		previousMessage = new HashMap<Long, Integer>();
		mListener = listener;
		mHelper = helper;
		mType = type;
		mLock = new Object();
		new DatabaseMonitor(DatabaseMonitor.MONITER_TYPE_ONLY_QUERY).start();		
	}
	
	
	
	@Override
	public void onChange(boolean onSelf) {	
		super.onChange(onSelf);
		new DatabaseMonitor(DatabaseMonitor.MONITER_TYPE_QUERY_AND_NOTIFY).start();			
	}


	private void log(String info){
		if (null != info){
			Xlog.v(TAG, info);
		}
	}

	public class DatabaseMonitor extends Thread{
		public final static int MONITER_TYPE_ONLY_QUERY = 0;
		public final static int MONITER_TYPE_QUERY_AND_NOTIFY = 1;

		private int mQueryType = 0;

		public DatabaseMonitor(int type) {
			mQueryType = type;
		}

		public void run(){
			if (MONITER_TYPE_ONLY_QUERY == mQueryType) {
				query();
			} else if (MONITER_TYPE_QUERY_AND_NOTIFY == mQueryType) {
				queryAndNotify();
			} else {
				//do nothing
				log("invalid motinor type:"+mQueryType);
			}
		}
		
		private void query() {
			synchronized(mLock) {
				mHelper.queryMessage(previousMessage);	
				log("query: size->"+previousMessage.size());
			}
		}
		private void queryAndNotify() {
			HashMap<Long, Integer> currentMessage = new HashMap<Long, Integer>();
			Iterator iterator;
			String newFolder;
			String oldFolder;
			
			if(mListener == null) {
				return;
			}
			synchronized(mLock) {
				mHelper.queryMessage(currentMessage);
					
				log("database has been changed, mType is "+mType+" previous size is "+previousMessage.size()+
						"current size is "+currentMessage.size());
    		
				
    		
				//if previous message is smaller than current, new message is received
				if (previousMessage.size() < currentMessage.size()){
					//find the new message
					iterator = currentMessage.entrySet().iterator();
					while (iterator.hasNext()) {
						Map.Entry entry = (Map.Entry)iterator.next();
						Long key = (Long)entry.getKey();
						//messasge is not in previous messages and the type is 
    		
						String folder = mHelper.revertMailboxType(currentMessage.get(key));
						if (!previousMessage.containsKey(key) && 
							folder != null &&
							folder.equals(MAP.Mailbox.INBOX)){
							mListener.onNewMessage(key, mType);						
						}
					}
				} else {
					iterator = previousMessage.entrySet().iterator();
					while (iterator.hasNext()) {
						Map.Entry entry = (Map.Entry)iterator.next();
						Long key = (Long)entry.getKey();
						//messasge is not in previous messages and the type is 
    						if (!currentMessage.containsKey(key)){
							oldFolder = mHelper.revertMailboxType((previousMessage.get(key)));
							mListener.onMessageDeleted(key, mType, oldFolder);					
						} else {
							oldFolder = mHelper.revertMailboxType((Integer)entry.getValue());
							newFolder = mHelper.revertMailboxType((currentMessage.get(key)));
													
    					//	log("id " + key +"oldFolder is " + oldFolder + "new folder is " + newFolder);
							
							if(newFolder == null || oldFolder == null || oldFolder.equals(newFolder)) {
									continue;
							}
							//check to determin message to be deleted or shifted
							
							if(newFolder.equals(MAP.Mailbox.DELETED)) {
								mListener.onMessageDeleted(key, mType, oldFolder);
							} else {
								mListener.onMessageShifted(key,mType, oldFolder,newFolder);
							}						
						}
					}
				}			
				previousMessage = currentMessage;
			}
		}
	}
}

