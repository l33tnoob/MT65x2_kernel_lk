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
import java.io.UnsupportedEncodingException;
import java.text.CharacterIterator;
import java.text.StringCharacterIterator;

//use to compose a message list response
public class MessageItem {
		private long 	MsgHandle;
		private String Subject;		// [M] Title, the first words of the message, or "" (This length shall be used according to the requested value in GetMessagesListing)
		private String DateTime;			// [M] The sending time or the reception time in format "YYYYMMDDTHHMMSS"
		private String SenderName;	// [C]
		private String SenderAddr;	// [C] The senders email address or phone number
		private String ReplyToAddr;	// [C] This shall be used only for emails to deliver the sender's reply-to email address.
		private String RecipientName;	// [C] The recipient's email address, a list of email addresses, or phone number
		private String RecipientAddr;	// [M] If the recipient is not known this may be left empty.
		private int MsgType;		// [M]
		private int OrignalMsgSize;	// [M] [MAP_CHECK] The overall size in bytes of the original message as received from network (using UINT16 in BRCM)
		private boolean bText;			// (default 'no') (The message includes textual content or not)
		private int RecipientStatus;	// [M]
		private int AttachSize;		// [M] [MAP_CHECK] (using UINT16 in BRCM)
		private boolean bPriority;		// (default 'no') The message is of high priority or not.
		private int read;			// (default 'no') The message has already been read on the MSE or not.
		private boolean bSent = true;			// (default 'no') The message has already been sent to the recipient or not.
		private boolean bProtected;

		private int mMaxSubjectLength = MAP.MAX_SUBJECT_LEN;

		public MessageItem(){
			
		}
		public void setSubjectLength(int len){
			mMaxSubjectLength = len;
		}

		public void resetSubjectLength () {
			mMaxSubjectLength = MAP.MAX_SUBJECT_LEN;
		}

		public synchronized void set( String subject,
						 long time, String senderAddr,
						 String sendName, String reply,
						 String recepientName, String recepientAddr,
						 int msgType, int origSize,
						 boolean bText, int recepientStatus,
						 int AttachSize, int read, boolean protect) {
			setSubject(subject);
			setDatetime(time);
			SenderAddr = senderAddr;
			SenderName = sendName;
			ReplyToAddr = reply;
			RecipientName = recepientName;
			RecipientAddr = recepientAddr; 
			RecipientStatus = recepientStatus; // to do 
			MsgType = msgType;        // todo 
			OrignalMsgSize = origSize;
			this.bText = bText;
			AttachSize = AttachSize;
			bPriority = false;
			this.read = read;
			bProtected = protect;
			
		}
		public void setSubject(String sub) {
			if(sub == null) {
				return;
			}
			sub = encode(sub);
			byte[] databytes = sub.getBytes();
			int length = databytes.length; 
			if (length > MAP.MAX_SUBJECT_LEN) {
				try {
					Subject = new String(databytes, 0, (MAP.MAX_SUBJECT_LEN-1), "utf-8");
				} catch (UnsupportedEncodingException e) {
				}
			} else {
				Subject = sub;
			}
		}
		public void setHandle(long handle){
			MsgHandle = handle;
		}
		public void setDatetime(long millis) {
			DateTime = UtcUtil.convertMillisToUtc(millis);
		}
		public void setSenderName(String name){
			SenderName = encode(name);
		}
		public void setSenderAddr(String addr) {
			SenderAddr = addr;
		}
		public void setReplyAddr(String addr) {
			ReplyToAddr = addr;
		}
		public void setRecipientName(String name) {
			RecipientName = encode(name);
		}
		public void setRecipientAddr(String addr) {
			RecipientAddr= addr;
		}
		public void setRecipientStatus(int status) {
			RecipientStatus = status;
		}
		public void setMsgType(int type) {
			MsgType = type;
		}
		public void setSize(int size) {
			OrignalMsgSize = size;
		}
		public void setText(boolean text) {
			bText= text;
		}
		public void setAttachSize(int size) {
			AttachSize = size;
		}
		public void setPriority(boolean priority) {
			bPriority = priority;
		}
		public void setReadStatus(int read) {
			this.read = read;
		}
		public void setProtected(boolean isProtected) {
			bProtected = isProtected;
		}
		
		/* Xml 1.0 predefined escape characters*/
		/* description				Character	name	Unicode*/
		/* double quotion mark		"			quot		U+022(34)*/
		/* ampersand				&			amp		U+026(38)*/
		/* apostroghe			'			apos		U+027(39)*/
		/* less-than sign			<			lt		U+03C(60)*/
		/* great-than sign			>			gt		U+03E(62)*/
		private String encode (String rawData) {
		if (rawData == null) {
            return null;
        }		
		StringBuilder result = new StringBuilder();
        final StringCharacterIterator iterator = new StringCharacterIterator(rawData);
        char character =  iterator.current();
        while (character != CharacterIterator.DONE ){
            if (character == '<') {
                result.append("&lt;");
            }
            else if (character == '>') {
                result.append("&gt;");
            }
            else if (character == '\"') {
                result.append("&quot;");
            }
            else if (character == '\'') {
				result.append("&apos;");
            }
            else if (character == '&') {
                result.append("&amp;");
            }
            else {
                // The char is not a special one, add it to the result as is
                result.append(character);
            }
            character = iterator.next();
        }
		return result.toString();
	}
	}
