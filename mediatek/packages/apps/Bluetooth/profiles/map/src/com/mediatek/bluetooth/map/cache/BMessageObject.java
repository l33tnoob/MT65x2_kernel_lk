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

import android.util.Log;
import android.content.Context;
import java.util.ArrayList;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.File;
import com.mediatek.bluetooth.map.MAP;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.lang.SecurityException;
import com.mediatek.xlog.Xlog;
public class BMessageObject {
	private final String TAG		= "BMessageObject";
	private String 					mFileName;  //store the bmessage
	private File 					mFile;
	private DataPool				mDataPool;

	private int 					mVersion = 1; // the field seems no use
	private int 					mReadStatus;
	private int 					mMsgType;
	private String					mFolderPath;

	private int						mOrignatorSize;
	private String					mOrignator;

	//envelope
	private ArrayList<Integer>		mRecipientSize;
	private ArrayList<String>		mRecipient;
	
	private int 					mPartId;

	private int						mEncoding;
	private int						mCharset;
	private int						mLanguage;

	private ArrayList<Integer>		mContentSize;
	private long					mWholeSize;

	private Context					mContext;
	private String					mName;

	private int 					mFracDeliver;  //more or last

	//only used in the case when message will be pushed
	private boolean					mTransparent;
	private boolean					mRetry;

	private long					mHandle; //valid when succeed to send message

	public class DataPool {		
		public final static int		FRAGMENT_THRESHOLD = 0x7FFF;
		private final String			FRAGMENT_FILE_SUFFIX = "_backup";	
		
		/*If message is too large and needs to fragment, the remain messag will be saved in backup file*/
		private String					mFileName;         //the data stored in the file will be saved to pool
		private String					mFileNameBackup; 
		private long 					mCurrentOffset;
		private long					mWholePoolSize;
		private boolean					isPoolEmpty = true;
		private Context 				mContext;

		public DataPool(Context context, String fileName){
			mContext = context;
			mFileName = fileName;
			mFileNameBackup = fileName + FRAGMENT_FILE_SUFFIX;
		}

		public boolean saveDataToPool() {
			log(" saveDataToPool isPoolEmpty is " + isPoolEmpty);
			File srcfile = mContext.getFileStreamPath(mFileName);
			File destfile = mContext.getFileStreamPath(mFileNameBackup);	
			mWholePoolSize = getContentSize();
			if (destfile.exists()) {
				destfile.delete();
			}
			mCurrentOffset = 0;
			if (srcfile.renameTo(destfile)) {
				isPoolEmpty = false;
				return true;
			} else {
			log("fail to renameTo ");
				return false;
			}
		}

		public boolean removeOneFragment() {
			log(" removeOneFragment isPoolEmpty is " + isPoolEmpty);
			byte[] data = new byte[FRAGMENT_THRESHOLD];
			int size ;
			boolean result = false;

			if (isPoolEmpty) {
				return false;
			}
			
			size = getContentAfterSkip(mFileNameBackup, data, 0, FRAGMENT_THRESHOLD, mCurrentOffset);			
			log("size is " + size);

  			if (size > 0 ) {
				result = setContentToFile(mFileName, data, 0, size);
				if (result) {
					mCurrentOffset += size;
				}
				if (mCurrentOffset >= mWholePoolSize) {
					resetPool();
				}
			} else {
				//no available data
				resetPool();
			}
			log("mCurrentOffset is " + mCurrentOffset);
			return result;
		}

		public boolean isPoolEmpty () {
			return isPoolEmpty;
		}

		public void resetPool() {
			if (isPoolEmpty) {
				return;
			}
			isPoolEmpty = true;
			mCurrentOffset = 0;
			File file = mContext.getFileStreamPath(mFileNameBackup);	
			if (file.exists()) {
				file.delete();
			}
		}
		
	}

	

	public BMessageObject(Context context, String fileName){
		mContext = context;
		try{
			FileOutputStream out = mContext.openFileOutput(fileName, Context.MODE_WORLD_READABLE);	
			out.close();
		} catch (FileNotFoundException e){
			log("fail to create file");
		} catch (IOException e) {
			log(e.toString());
		}
		mFile = mContext.getFileStreamPath(fileName);		
		mFileName = mFile.getAbsolutePath();		
		mName = fileName;
		initCache();	
		log("file path"+mFileName);

		mDataPool = new DataPool(mContext, mName);
	}
	private void initCache(){
		mRecipient = new ArrayList<String>();
		mRecipientSize = new ArrayList<Integer>();
		mContentSize = new ArrayList<Integer>();
	}

	public void reset() {
		mReadStatus = -1;
		mMsgType = -1;
		mFolderPath = null;

		mOrignatorSize = 0;
		mOrignator = null;
		
		mRecipientSize.clear();
		mRecipient.clear();
	
		mPartId = 0;

		mEncoding = 0;
		mCharset = 0;
		mLanguage = 0;

		mContentSize.clear();
		mWholeSize = 0;

		mFracDeliver = MAP.FRACTION_DELIVER_NO;

		clearMessagebuffer();
		mDataPool.resetPool();

		mHandle = -1;
	}

	private void clearMessagebuffer(){
		//empty file
		try {
			FileOutputStream out = new FileOutputStream(mContext.getFileStreamPath(mName), false);
			out.close();
		} catch (IOException e) {						
		   log(e.toString());
		}
	}

	public boolean setFolderPath(String folder) {
		mFolderPath = folder;
		return true;
	}

	

	public boolean setOrignator(String orignator){
		mOrignator = orignator;
		if (orignator == null) {
			mOrignatorSize = 0;
		} else {
			mOrignatorSize = orignator.length();
		}
		return true;
	}

	//note: the input is nest vcards
	public boolean addRecipient(String recipient) {
		if (recipient == null) {
			return true;
		}
		mRecipientSize.add(recipient.length());
		mRecipient.add(recipient);
		return true;
	}

	public boolean setPartId(int partId) {
		if (partId >= 0) {
			mPartId = partId;
			return true;
		}
		return false;
	}
	public boolean setMessageType(int type){
		mMsgType = type;
		switch(mMsgType) {
			case MAP.MSG_TYPE_EMAIL:
			case MAP.MSG_TYPE_MMS:
			case MAP.MSG_TYPE_SMS_CDMA:
			case MAP.MSG_TYPE_SMS_GSM:
				mMsgType = type;
				return true;
			default:
				log("error, invalid message type:" + type);				
		}
		return false;
	}

	public boolean setContentSize(int size){
		mWholeSize = size;
		mContentSize.add(size);
		return true;
	}
	public boolean setContentSize(File file){
		if (file == null){
			return false;
		}
		try {
			FileInputStream stream = new FileInputStream(file);
			int size = stream.available();
			mWholeSize = size; 
			mContentSize.add(size);
			stream.close();
			return true;
		} catch (IOException e) {			
           log(e.toString());
		}
		return true;
	}
	public boolean setContent (byte[] content){
		return setContentToFile(mName, content, 0, content.length);
	}

	public boolean setContentToFile (String fileName, byte[] content, int offset, int length){
		FileOutputStream stream;
		try {
			if (fileName != null){
				stream = mContext.openFileOutput(fileName, Context.MODE_WORLD_READABLE);			
			} else {
				log("fail to get content");
				return false;
			}		
			stream.write(content, offset, length);
			stream.close();
			mContentSize.clear();
			mContentSize.add(length);			
			mWholeSize = length; 	
		}catch (FileNotFoundException e) {
			log(e.toString());
			return false;
		} catch (IOException e) {			
          // throw e;		
           log(e.toString());
		}
		return true;
	}
	public boolean addContent(byte[] content){
		FileOutputStream stream;
		if (content == null || content.length == 0){
			return true;
		}
		try {
			if (mContext == null) {
				stream = new FileOutputStream(mFile);
			
			} else if (mFileName != null){
				stream = mContext.openFileOutput(mName, Context.MODE_WORLD_READABLE | Context.MODE_APPEND);			
			
			} else {
				log("fail to get content");
				return false;
			}
					
			stream.write(content);
			stream.close();
			mContentSize.add(content.length);
			mWholeSize +=content.length; 
			return true;		
		} catch (FileNotFoundException e) {		
           log(e.toString());			
		} catch (IOException e) {			
          // throw e;		
           log(e.toString());
		}
		return true;
	}

	//
	public boolean setEncoding(int encoding) {
		 mEncoding = encoding;
		 return true;
	}

	public boolean setCharset(int charset) {
		switch(charset) {
			case MAP.CHARSET_NATIVE:
			case MAP.CHARSET_UTF8:
				mCharset = charset;
				break;
			default:
				log("error, invalid charset");
				mCharset = MAP.CHARSET_NATIVE; 
		}
	//	return mEncoding == encoding;
		return true;
	}

	public void setReadStatus(int state) {
		switch(state) {
			case MAP.READ_STATUS:
			case MAP.UNREAD_STATUS:
				mReadStatus = state;
				break;
			default:
				log("error, invalid read status: "+ state);
				mReadStatus = MAP.READ_STATUS; 
		}
	}
	public boolean setLang(int lang) {
	/*	switch(lang){
			case MAP.LANG_ENGLISH:
			case MAP.LANG_CHINESE:
			case MAP.LANG_FRENCH:
			case MAP.LANG_HEBREW:
			case MAP.LANG_JAPANESE:
			case MAP.LANG_KOREAN:
			case MAP.LANG_PORTUGUESE:
			case MAP.LANG_SPANISH:
			case MAP.LANG_TURKISH:
			case MAP.LANG_UNKNOWN:
				mLanguage = lang;
				return true;
			default:
				log("invalid Language,"+lang);
				return false;
		}
		*/
		mLanguage = lang;
		return true;
	}

	/*Some MCE can not correctly process BMessage if message is too large*/
	/*So the large message can be fragmented if neccessary*/
	public void fragmentIfNeccessary() {
		if (getContentSize() <= DataPool.FRAGMENT_THRESHOLD) {
			mFracDeliver = MAP.FRACTION_DELIVER_NO;	
		} else if (mDataPool.saveDataToPool() ) {
			if (mDataPool.removeOneFragment()&& !mDataPool.isPoolEmpty()) {
				mFracDeliver = MAP.FRACTION_DELIVER_MORE;
			} else {
				//if failed in the fisrt time or pool has been empty, set no fraction deliver
				mFracDeliver = MAP.FRACTION_DELIVER_NO;					
			}
		} else {
			mFracDeliver = MAP.FRACTION_DELIVER_NO;	
		}
 
	}

	public void retrieveNextPartion() {
		boolean result;

		clearMessagebuffer();
		result = mDataPool.removeOneFragment();

		if (result) {
			mPartId += 1;
		}
		if (result && !mDataPool.isPoolEmpty()) {
			mFracDeliver = MAP.FRACTION_DELIVER_MORE;
		} else {
			mFracDeliver = MAP.FRACTION_DELIVER_LAST;	
		}
	}

	public int getMessageType() {
		return mMsgType;
	}

	public int getContent(byte[] content){
		return getContentFromFile(mName, content, 0, content.length);
	}

	/*Notes: no seperator can be included in filename*/
	public int getContentFromFile(String filename, byte[] content, int offset, int length){
		FileInputStream stream;
		int size = 0; 
		
		try{
			if (filename != null){			
				stream = mContext.openFileInput(filename);					
			} else {
				log("fail to get content");
				return size;
			}				
			size = stream.read(content, offset, length);
			stream.close();
		} catch (FileNotFoundException e){
			log("fail to find file");
		} catch (IOException e) {	           			
           log(e.toString());
		} 
			return size;
		}
	/*Notes: no seperator can be included in filename*/
	public int getContentAfterSkip(String filename, byte[] content, int offset, int length, long skipoffset){
			FileInputStream stream;
			int size = 0; 
			
			try{
				if (filename != null){			
					stream = mContext.openFileInput(filename);					
				} else {
					log("fail to get content");
					return size;
				}	
				long skipnum = stream.skip(skipoffset);
				size = stream.read(content, offset, length);
				stream.close();
			} catch (FileNotFoundException e){
				log("fail to find file");
			} catch (IOException e) {						
			   log(e.toString());
			} 

				return size;		
	}

	public long getContentSize(){
		return mWholeSize;
	}
	public int getContentSize(int i){
		if (i < mContentSize.size()){
			return mContentSize.get(i).intValue();
		} else {
			return 0;
		}
	}	

	public String getOrignator(){
		return mOrignator;
	}

	public String getFinalRecipient(){
		if (mRecipient.size() > 0) {
			return mRecipient.get(mRecipient.size() - 1);
		} else {
			return null;
		}
	}

	public ArrayList<String> getRecipient() {
		return mRecipient;
	}

	public File getFile(){
		return mFile;
	}

	public String getFolder(){
		if (mFolderPath == null) {
			return null;
		}
		mFolderPath = mFolderPath.toLowerCase();
			
		int lastIndex = mFolderPath.lastIndexOf("/");
		//	lastIndex = (lastIndex < 0)? 0:lastIndex;
		return mFolderPath.substring(lastIndex+1);
	}

	public int getReadStatus(){
		return mReadStatus;
	}
	public int getCharset(){
		return mCharset;
	}

	public boolean releaseResource(){
		if (mFile != null && mFile.exists()){
			 mFile.delete();
		}
		return true;
	}
	//return:
	//@false: do not save message
	//@true: save message
	public boolean isTransparent(){
		return mTransparent;
	}
	
	public boolean isRetry(){
		return mRetry;
	}

	public void setHandle(long id){
		mHandle = id;
	}
	public long getHandle()
	{
		return mHandle;
	}

	
	private void log(String info){
		Xlog.v(TAG,info);
	}
	
}
